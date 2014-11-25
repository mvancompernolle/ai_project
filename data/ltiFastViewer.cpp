/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiViewer.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.4.2000
 * revisions ..: $Id: ltiFastViewer.cpp,v 1.13 2010/04/10 01:34:47 alvarado Exp $
 */


#include "ltiObject.h"

#include "ltiFastViewer.h"
#ifndef _LTI_WIN32

// required to compile with gcc 2.95.3 and the new unistd
#define __USE_BSD 1
// The constants which we use are defined in a Motif header file, but we
// do not want Motif stuff in here
// include <Xm/MwmUtil.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <X11/Xatom.h>

// These are the required definitions from MwmUtil.h, as taken from
// the lesstif package:
#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_FUNC_ALL            (1L << 0)
#define _XA_MOTIF_WM_HINTS      "_MOTIF_WM_HINTS"
// end of definitions

#include <iostream>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // --------------------------------------------------
  // fastViewer::parameters
  // --------------------------------------------------

  // default constructor
  fastViewer::parameters::parameters() : viewerBase::parameters() {
    topleft=point(50,50);
    noBorder=false;
  }

  // copy constructor
  fastViewer::parameters::parameters(const parameters& other)
  : viewerBase::parameters() {
    copy(other);
  }

  // destructor
  fastViewer::parameters::~parameters() {
  }

  // get type name
  const char* fastViewer::parameters::getTypeName() const {
    return "fastViewer::parameters";
  }

  // copy member

  fastViewer::parameters&
  fastViewer::parameters::copy(const parameters& other) {

    topleft.copy(other.topleft);
    noBorder=other.noBorder;
    return *this;
  }

  fastViewer::parameters&
  fastViewer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fastViewer::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool fastViewer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fastViewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler, "topleft", topleft);
      lti::write(handler, "noBorder", noBorder);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase::parameters::write(handler,false);
# else
    bool (viewerBase::parameters::* p_writeMS)(ioHandler&,const bool) const =
      viewerBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fastViewer::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool fastViewer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fastViewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler, "topleft" ,topleft);
      lti::read(handler, "noBorder" ,noBorder);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase::parameters::read(handler,false);
# else
    bool (viewerBase::parameters::* p_readMS)(ioHandler&,const bool) =
      viewerBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fastViewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // -------------------------------------
  // fastViewer
  // -------------------------------------

  /*
   * default constructor
   */
  fastViewer::fastViewer(const char* winName)
    : viewerBase(),title(winName),remoteData(0) {

    if((display_info.display = XOpenDisplay(NULL)) == NULL){
      throw exception("Error: Cannot open display:");
    }

    parameters p;
    p.title = winName;
    setParameters(p);

    wm_info.motif_wm_hints = XInternAtom(display_info.display,
                                         _XA_MOTIF_WM_HINTS,0);
    _lti_debug("WMinfo is " << wm_info.motif_wm_hints << "\n");

    std::string displayStr,dispHost,realHost;
    std::string::size_type pos;
    char buffer[256];

    useShareMemory = true;

    displayStr = getenv("DISPLAY");
    pos = displayStr.find(':');

    if (pos == 0) {
      return;
    }

    dispHost = displayStr.substr(0,pos);

    if (dispHost == "localhost") {
      return;
    }

    bool error = false;

    if (dispHost.find('.') == std::string::npos) {
      error = error || (::getdomainname(buffer,256) != 0);
      dispHost += ".";
      dispHost += buffer;
    }


    error = error || (::gethostname(buffer,256) != 0);
    realHost = buffer;
    if (realHost.find('.') == std::string::npos) {
      error = error || (::getdomainname(buffer,256) != 0);
      realHost += ".";
      realHost += buffer;
    }

    useShareMemory = (dispHost.find(realHost) == 0);

  }

  /*
   * copy constructor
   * @param other the object to be copied
   */
  fastViewer::fastViewer(const fastViewer& other)
    : viewerBase(),remoteData(0) {
    if((display_info.display = XOpenDisplay(NULL)) == NULL){
      throw exception("Error: Cannot open display:");
    }
    title = other.title;
    useShareMemory = other.useShareMemory;
  }

  /*
   * destructor
   */
  fastViewer::~fastViewer() {
    destroyWindow();
    if (notNull(display_info.display)) {
      XCloseDisplay(display_info.display);
      display_info.display = 0;
    }
  }


  void fastViewer::setPosition(const point& pos) {
    parameters p=getParameters();
    p.topleft=pos;
    setParameters(p);
    if (display_info.win != 0) {
      XMoveWindow(display_info.display,display_info.win,
                  pos.x, pos.y);
    }
  }

  /*
   * returns the name of this type ("fastViewer")
   */
  const char* fastViewer::getTypeName() const {
    return "fastViewer";
  }

  /*
   * shows an lti::mathObject
   * @param data the object to be shown.
   */
  bool fastViewer::show(const image& img) {
    // Draw screen onto display

    if (img.rows()>0 && img.columns()>0) {
      if (data.size() == img.size()) {
        data.fill(img);
      } else {
        destroyImage();
        createImage(img);
      }
    } else {
      setStatusString("empty image");
      return false;
    }

    if (useShareMemory) {
      XShmPutImage(display_info.display,
                   display_info.win,
                   display_info.gc,
                   display_info.shmimage,
                   0, 0, 0, 0,
                   display_info.width,
                   display_info.height, false);
    } else {
      XPutImage(display_info.display,
                display_info.win,
                display_info.gc,
                display_info.shmimage,
                0, 0, 0, 0,
                display_info.width,
                display_info.height);
    }

    XSync(display_info.display,0);

    return true;
  }

  /*
   * shows a 8-bit channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const channel8& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const channel& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const matrix<float>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const vector<double>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const vector<float>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const vector<int>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a matrix of doubles as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const matrix<double>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a matrix of integers as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool fastViewer::show(const matrix<int>& data) {
    return viewerBase::show(data);
  }

  /*
   * hides the display window
   */
  bool fastViewer::hide() {
    destroyWindow();
    return true;
  }

  /*
   * copy data of "other" functor.
   */
  fastViewer& fastViewer::copy(const fastViewer& other) {
    viewerBase::copy(other);
    title = other.title;
    return *this;
  }

  /*
   * copy data of "other" functor.
   */
  fastViewer& fastViewer::operator=(const fastViewer& other) {
    return copy(other);
  }

  /*
   * returns a pointer to a clone of the functor.
   */
  viewerBase* fastViewer::clone() const {
    return new fastViewer(*this);
  }

  /*
   * initialize window
   */
  void fastViewer::createWindow() {
    XGCValues gc_values;
    display_info_s& di = display_info;

    //
    // Window Creation and Setup
    //
    const parameters& p=getParameters();
    title = p.title;

    _lti_debug("Creating window at " << p.topleft.x << "," << p.topleft.y << "\n");


    di.win = XCreateSimpleWindow(di.display, DefaultRootWindow(di.display),
                          p.topleft.x, p.topleft.y, di.width, di.height, 0,
                          BlackPixel(di.display, DefaultScreen(di.display)),
        BlackPixel(di.display, DefaultScreen(di.display)));

    if(!di.win) {
      throw exception("Error: Cannot create window:");
    }

    XMapWindow(di.display, di.win);

    gc_values.graphics_exposures = false;
    di.gc = XCreateGC(di.display, di.win, GCGraphicsExposures, &gc_values);

    XStoreName(di.display,di.win,title.c_str());

    // this lines tell the X-Server to redraw automatically
    // the contents of the image if required...
    XSetWindowAttributes xattrib;
    xattrib.backing_store = Always;
    XChangeWindowAttributes(di.display,di.win,CWBackingStore,&xattrib);
    XSelectInput(di.display, di.win, (KeyPressMask |
                                      KeyReleaseMask));
    // Move window to override window manager
    XMoveWindow(di.display,di.win,p.topleft.x,p.topleft.y);

    // Change the protocol to impede the wm to close this window and to kill
    // the application (which is done somewhere else...)
    Atom wmProtocol[1];
    wmProtocol[0] = XInternAtom(di.display,"WM_DELETE_WINDOW",False);
    XSetWMProtocols(di.display,di.win,wmProtocol,1);

    // Now set properties:
    // MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS, MWM_FUNC_ALL, no decorations
    if (p.noBorder) {
      int32 prop[5]={ MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS,
                      MWM_FUNC_ALL, 0, 0, 0};
      _lti_debug("WMinfo2 is " << wm_info.motif_wm_hints << "\n");
      XChangeProperty(di.display, di.win,
                      wm_info.motif_wm_hints, wm_info.motif_wm_hints,
                      32, PropModeReplace, (unsigned char *)prop, 5);
    }
  }

  /*
   * destroy window
   */
  void fastViewer::destroyWindow() {
    destroyImage();
    if (display_info.win != 0) {
      XFreeGC(display_info.display,display_info.gc);
      XDestroyWindow(display_info.display,display_info.win);
    }
    display_info.win = 0;
  }

  /*
   * create XImage
   */
  void fastViewer::createImage(const image& img) {
    int screen;
    display_info_s& di = display_info;
    XWindowAttributes win_attributes;
    bool resizeWin = false;

    if ((di.height != img.rows()) ||
        (di.width != img.columns())) {
      resizeWin = true;
    }

    di.height = img.rows();
    di.width = img.columns();

    if (di.win == 0) {
      createWindow();
    }

    if (resizeWin) {
      XResizeWindow(di.display,di.win,di.width,di.height);
    }

    screen = DefaultScreen(di.display);
    XGetWindowAttributes(di.display, di.win, &win_attributes);
    di.depth  = win_attributes.depth;

    // TODO: maybe screen->root_depth is more precise than win_attr.depth
    //       We should try it.

    // check if the X-Server has a 32 bit interface
    if (di.depth < 24) {
      throw exception("Error: Fast Viewer works only with 32 bit depth!");
    }

    if (useShareMemory) {
      //
      // Shared Memory Setup
      //

      di.shmimage = XShmCreateImage(di.display,
                                    DefaultVisual(di.display, screen),
                                    di.depth, ZPixmap, NULL, &shminfo,
                                    di.width,
                                    di.height);

      if(isNull(di.shmimage)) {
        throw exception("fastViewer::shmimage == NULL:");
      }

      int sharedMemSize = di.shmimage->bytes_per_line * di.shmimage->height;

      shminfo.shmid = shmget(IPC_PRIVATE,
                             sharedMemSize,
                             IPC_CREAT | 0777);

      if(shminfo.shmid < 0) {
        std::string str;
        str = "fastViewer::shmget failed:";
        str += strerror(errno);

        throw exception(str);
      }

      shminfo.shmaddr = (char *) shmat(shminfo.shmid, (void *) 0, 0);
      if (shminfo.shmaddr == 0) {
        std::string str;
        str = "fastViewer::shmmat failed:";
        str += std::strerror(errno);

        throw exception(str);
      }

      di.shmimage->data = shminfo.shmaddr;

      XShmAttach(di.display, &shminfo);
      data.useExternData(di.height,di.width,(rgbPixel*)di.shmimage->data);
      data.fill(img);
    } else {
      //
      // without shared memory
      //
      const int blockSize = img.rows()*img.columns()*4;
      remoteData = new char[blockSize];
      data.useExternData(di.height,di.width,(rgbPixel*)remoteData);
      data.fill(img);

      di.shmimage = XCreateImage(di.display,
                                 DefaultVisual(di.display, screen),
                                 di.depth, ZPixmap, 0,
                                 remoteData,
                                 di.width,
                                 di.height,8,0);

      if(isNull(di.shmimage)) {
        throw exception("fastViewer::shmimage == NULL:");
      }
    }
  }

  /*
   * destroy XImage
   */
  void fastViewer::destroyImage() {
    if (notNull(display_info.shmimage)) {

      if (useShareMemory) {
        XShmDetach(display_info.display,&shminfo);
      }
      XDestroyImage(display_info.shmimage);
      display_info.shmimage = 0;

      if (useShareMemory) {
        // delete the shared memory block!
        shmid_ds shmStruct;

        shmStruct.shm_nattch = 0;
        shmdt(shminfo.shmaddr);
        if (shmctl(shminfo.shmid,IPC_RMID,&shmStruct) < 0) {
          std::string str;
          str = "fastViewer::shmctl failed:";
          str += std::strerror(errno);

          throw exception(str);
        }

        shminfo.shmid = -1;
      }
    }
  }

  // return parameters
  const fastViewer::parameters& fastViewer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&viewerBase::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  bool fastViewer::setParameters(const viewerBase::parameters& param) {
    if (viewerBase::setParameters(param)) {
      // change the window name if necessary
      if (display_info.win != 0) {
        const parameters& p = getParameters();
        if (title != p.title) {
          title = p.title;
          XStoreName(display_info.display,display_info.win,title.c_str());
        }
      }
      return true;
    }
    return false;
  }

  /*
   * constructor
   */
  fastViewer::display_info_s::display_info_s() {
    win = 0;
    display = 0;
    width = 0;
    height = 0;
    depth = 0;
    shmimage = 0;
  }

  /*
   * constructor
   */
  fastViewer::winman_info_s::winman_info_s() {
    motif_wm_hints=0;
  }


}

#endif
