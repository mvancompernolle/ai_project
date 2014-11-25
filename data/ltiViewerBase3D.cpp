/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiViewerBase3D.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 1.12.2001
 * revisions ..: $Id: ltiViewerBase3D.cpp,v 1.10 2006/09/05 10:44:05 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiViewerBase3D.h"

#ifdef HAVE_GTK

#include "ltiGtkServer.h"
#include "ltiALLFunctor.h"
#include <cstdio>

// debug levels: 4. everything
//               3. more messages but not all
//               2. a few messages
//               1. nothing at all


#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // viewerBase3D::parameters
  // --------------------------------------------------

  // default constructor
  viewerBase3D::parameters::parameters()
    : viewerBase::parameters() {

    saveName = "image.png";
    backgroundColor = lti::Black;
    axisColor = lti::White;
    windowSize = point(256,256);
    axisColorFixed = true;

    camTarget = dpoint3D(128,128,128);
    camRadius = 200;

    double e,a;
    e = 60*Pi/180.0;
    a = 45*Pi/180.0;

    dpoint3D camPos(camTarget.x+camRadius*sin(e)*cos(a),
                    camTarget.y+camRadius*sin(e)*sin(a),
                    camTarget.z+camRadius*cos(e));

    camParameters.camPos = camPos;
    camParameters.elevation = e;
    camParameters.azimuth = a;
    camParameters.center = dpoint(0.5,0.5);
    camParameters.zoom = 0.5;
    camParameters.perspective = 0.002;
  }

  // copy constructor
  viewerBase3D::parameters::parameters(const parameters& other)
    : viewerBase::parameters()  {
    copy(other);
  }

  // destructor
  viewerBase3D::parameters::~parameters() {
  }

  // get type name
  const char* viewerBase3D::parameters::getTypeName() const {
    return "viewerBase3D::parameters";
  }

  // copy member

  viewerBase3D::parameters&
    viewerBase3D::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    viewerBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    viewerBase::parameters& (viewerBase::parameters::* p_copy)
      (const viewerBase::parameters&) =
      viewerBase::parameters::copy;
    (this->*p_copy)(other);
# endif

    saveName = other.saveName;
    backgroundColor = other.backgroundColor;
    axisColor = other.axisColor;
    windowSize = other.windowSize;
    axisColorFixed = other.axisColorFixed;
    camParameters.copy(other.camParameters);

    return *this;
  }

  // alias for copy member
  viewerBase3D::parameters&
    viewerBase3D::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* viewerBase3D::parameters::clone() const {
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
  bool viewerBase3D::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool viewerBase3D::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"saveName",saveName);
      lti::write(handler,"backgroundColor",backgroundColor);
      lti::write(handler,"axisColor",axisColor);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"axisColorFixed",axisColorFixed);
      lti::write(handler,"camParameters",camParameters);
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
  bool viewerBase3D::parameters::write(ioHandler& handler,
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
  bool viewerBase3D::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool viewerBase3D::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"saveName",saveName);
      lti::read(handler,"backgroundColor",backgroundColor);
      lti::read(handler,"axisColor",axisColor);
      lti::read(handler,"windowSize",windowSize);
      lti::read(handler,"axisColorFixed",axisColorFixed);
      lti::read(handler,"camParameters",camParameters);
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
  bool viewerBase3D::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // viewerBase3D
  // --------------------------------------------------

  // default constructor
  viewerBase3D::viewerBase3D(bool createDefaultParams)
    : viewerBase() ,wnd(0) {

    if (createDefaultParams) {
      // create an instance of the parameters with the default values
      parameters defaultParameters;

      // set the default parameters
      setParameters(defaultParameters);
    }

    gtkServer server;
    server.start();
  };

  // copy constructor
  viewerBase3D::viewerBase3D(const viewerBase3D& other)
  : viewerBase() ,wnd(0) {

    copy(other);

    gtkServer server;
    server.start();
  };

  // default constructor
  viewerBase3D::viewerBase3D(const std::string& title)
    : viewerBase() ,wnd(0) {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.title = title;

    // set the default parameters
    setParameters(defaultParameters);

    gtkServer server;
    server.start();
  };


  // destructor
  viewerBase3D::~viewerBase3D() {
    delete wnd;
    wnd = 0;
  };

  // returns the name of this type
  const char* viewerBase3D::getTypeName() const {
    return "viewerBase3D";
  }

  // copy member
  viewerBase3D& viewerBase3D::copy(const viewerBase3D& other) {
    viewerBase::copy(other);

    return (*this);
  }

  // alias for copy member
  viewerBase3D&
    viewerBase3D::operator=(const viewerBase3D& other) {
    return (copy(other));
  }


  // clone member
  viewerBase* viewerBase3D::clone() const {
    return new viewerBase3D(*this);
  }

  // return parameters
  const viewerBase3D::parameters&
    viewerBase3D::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&viewerBase::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  viewerBase3D::parameters& viewerBase3D::getParameters() {

    parameters* par =
      dynamic_cast<parameters*>(&viewerBase::getParameters());

    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }

    return *par;
  }

  bool viewerBase3D::setParameters(const viewerBase::parameters& par) {
    if (viewerBase::setParameters(par)) {
      if (notNull(wnd)) { // the data need to be redrawn
        wnd->useParameters(getParameters());
        wnd->redraw();
      }
      return true;
    }

    return false;
  }

  /*
   * shows a color image.
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase3D::show() {

    if (ensureMainWindowInstance()) {
      wnd->drawData();
    } else {
      return false;
    }

    return true;
  }

  /*
   *  get new instance of main window class
   */
  viewerBase3D::mainWindow* viewerBase3D::getNewMainWindow() const {
    return new mainWindow();
  }

  /*
   *  get new instance of main window class
   */
  bool viewerBase3D::ensureMainWindowInstance() {

    if (isNull(wnd)) {
      _lti_debug2("viewerBase3D::ensureMainWindow(): creating mainWindow\n");

      wnd = getNewMainWindow();

      if (isNull(wnd)) {
        // if still null, something went really wrong here!
        return false;
      }

      wnd->useParameters(getParameters());
      wnd->createWindow();
    }

    return true;
  }

  /*
   * take a snapshot of the current visualized image.
   *
   * The image being shown will be saved in a file with the given name.
   * All data types supported by lti::saveImage functor will be accepted.
   */
  bool viewerBase3D::snapshot(const std::string& filename) const {
    if (notNull(wnd)) {
      wnd->lock();
      image img(wnd->getDisplayedData());
      wnd->unlock();

      if (!img.empty()) {
        saveImage saver;
        if (saver.save(filename,img)) {
          return true;
        } else {
          setStatusString(saver.getStatusString());
          return false;
        }
      }
    }

    return false;
  }

  // -------------------------------------------------------------------
  // Main Window
  // -------------------------------------------------------------------

  // static members
  const point viewerBase3D::mainWindow::borderWidth(21,36);

  viewerBase3D::mainWindow::mainWindow(bool setClassParam)
    : object(), options(0), syncEnd(0)  {

    init();

    if (setClassParam) {
      gdk_threads_enter();
      options = new configDialog();
      options->buildDialog();
      gdk_threads_leave();
    }
    param = 0;
  };

  viewerBase3D::mainWindow::mainWindow(const mainWindow& other)
    : object(), syncEnd(0) {

    init();

    gdk_threads_enter();
    options = new configDialog();
    options->buildDialog();
    gdk_threads_leave();

    param = 0;

    copy(other);
  };

  void viewerBase3D::mainWindow::init(void) {
    showingBuffer = false;
    winSize = point(0,0);

    xbuffer = 0;
    xbuffer_rows = 0;
    xbuffer_cols = 0;

    mouseButtonPressed = 0;
    busy=false;
    dataChanged = true;

    theLock.lock();
    timerTag = 0;
    theLock.unlock();
  };

  viewerBase3D::mainWindow::~mainWindow() {

    if (gtkServer::isAlive()) {
      gdk_threads_enter();
      gtk_timeout_add(1,destroyWndTimeout,(gpointer)this);
      gdk_threads_leave();

      syncEnd.wait(); // wait until widgets destroyed!
    }

    theLock.lock();
    delete[] xbuffer;
    xbuffer = NULL;
    theLock.unlock();

    delete options;
    options = NULL;
  }

  void viewerBase3D::mainWindow::drawData() {

    gdk_threads_enter();
    if (!showingBuffer) {
      showingBuffer = true;
      timerTag = gtk_timeout_add(1,show,(gpointer)this);
    }
    gdk_threads_leave();
  };


  bool viewerBase3D::mainWindow::useParameters(parameters& theParam) {
    if (dynamic_cast<parameters*>(&theParam) != 0) {
      param = &theParam;
      return true;
    }
    param = 0;
    return false;
  };

  // copy member
  viewerBase3D::mainWindow&
  viewerBase3D::mainWindow::copy(const viewerBase3D::mainWindow& other) {
    useParameters(*other.param);

    // copy window name
    winName = other.winName;
    winSize = other.winSize;

    // erase actual data
    theLock.lock();
    theImage.copy(other.theImage);
    theLock.unlock();

    return *this;
  }

  viewerBase3D::mainWindow* viewerBase3D::mainWindow::clone() const {
    return new mainWindow(*this);
  }

  void viewerBase3D::mainWindow::lock() {
    theLock.lock();
  }

  void viewerBase3D::mainWindow::unlock() {
    theLock.unlock();
  }

  void viewerBase3D::mainWindow::createWindow() {
    gdk_threads_enter();
    gtk_timeout_add(1,createWindowTimeout,(gpointer)this);
    gdk_threads_leave();
  }

  // this timeout will be executed in the correct thread!
  gint viewerBase3D::mainWindow::createWindowTimeout(gpointer data ) {

    mainWindow* me = (mainWindow*)(data);
    me->createWindowLocal();

    return 0; // destroy timer!
  }

  // create new window
  void viewerBase3D::mainWindow::createWindowLocal() {

    /* create a new window */
    window = gtk_dialog_new ();

    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
        GTK_SIGNAL_FUNC (delete_event), this);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    //  gtk_signal_connect (GTK_OBJECT (window), "destroy",
    //      GTK_SIGNAL_FUNC (destroy), this);

    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window),0);

    /* create a new scrolled window. */
    scrolledWindow = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_set_border_width (GTK_CONTAINER (scrolledWindow), 0);

    /* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
     * GTK_POLICY_AUTOMATIC will automatically decide whether you need
     * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
     * there.  The first one is the horizontal scrollbar, the second,
     * the vertical. */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledWindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    /* The dialog window is created with a vbox packed into it. */
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(window)->vbox),
                        scrolledWindow,
      TRUE, TRUE, 0);

    /* Status bar */
    statusBar = gtk_statusbar_new();

    gtk_container_add(GTK_CONTAINER (GTK_DIALOG(window)->action_area),
                      statusBar);
    statusBarContextId =
      gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar),
                                   "ltiViewerBase3D_StatusBar");

    /* Sets the border width of the status bar. */
    gtk_container_set_border_width(
                      GTK_CONTAINER(GTK_DIALOG(window)->action_area),0);


    /* drawing area */
    darea = gtk_drawing_area_new();

    gtk_drawing_area_size(GTK_DRAWING_AREA(darea), 256, 256);

    gtk_signal_connect (GTK_OBJECT (darea), "expose-event",
                        GTK_SIGNAL_FUNC (dareaExpose), this);

    gtk_widget_set_usize(window, 64, 64);

    /* event box */
    eventBox = gtk_event_box_new();

    gtk_scrolled_window_add_with_viewport(
                      GTK_SCROLLED_WINDOW(scrolledWindow),eventBox);
    gtk_container_add(GTK_CONTAINER (eventBox),darea);
    gtk_widget_set_events(eventBox,GDK_MOTION_NOTIFY);
    gtk_signal_connect (GTK_OBJECT (eventBox), "motion_notify_event",
                        GTK_SIGNAL_FUNC (mouseMovedEvent), this);
    gtk_signal_connect (GTK_OBJECT (eventBox), "button_press_event",
                          GTK_SIGNAL_FUNC (buttonPressedEvent), this);
    gtk_signal_connect (GTK_OBJECT (eventBox), "button_release_event",
                          GTK_SIGNAL_FUNC (buttonPressedEvent), this);

    gtk_widget_realize (eventBox);
    gdk_window_set_cursor (eventBox->window, gdk_cursor_new (GDK_CROSS));
  }

  // destroy window
  void viewerBase3D::mainWindow::destroyWindow() {

    _lti_debug2("hiding histogram viewer...");

    gtk_widget_hide_all(window);
    gtk_widget_destroy(GTK_WIDGET(window));

    _lti_debug2("done\n");

    // report end of destruction
    syncEnd.post();
  }


  // this will be called in the right thread to destroy the actual window!
  gint viewerBase3D::mainWindow::destroyWndTimeout(gpointer data) {
    mainWindow* me = (mainWindow*)(data);
    me->destroyWindow();

    return 0; // destroy timer!
  }

  gint viewerBase3D::mainWindow::delete_event(GtkWidget *widget,
                                                 GdkEvent  *event,
                                                 gpointer   data ) {
    /*
     * If you return FALSE in the "delete_event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs.
     */

    gtk_widget_hide_all(widget);

    /*
     * Change TRUE to FALSE and the main window will be destroyed with
     * a "delete_event".
     */
    return(TRUE);
  }

  const image& viewerBase3D::mainWindow::getDisplayedData() const {
    return theImage;
  }

  void viewerBase3D::mainWindow::hideData() {
    gdk_threads_enter();
    gtk_timeout_add(1,hide,(gpointer)this);
    gdk_threads_leave();
  }

  void viewerBase3D::mainWindow::redraw() {
    busy = true;
    gtk_timeout_add(1,show,(gpointer)this);
  }

  // --------------------------------------------------------
  //  show method
  // --------------------------------------------------------

  gint viewerBase3D::mainWindow::show(gpointer data) {

    mainWindow* me = (mainWindow*)(data);

    me->lock();
    me->showingBuffer = me->validData();
    me->unlock();
    me->dataChanged = true;

    if (notNull(me->param)) {
      gtk_window_set_title(GTK_WINDOW(me->window),me->param->title.c_str());
    }

    gtk_widget_show_all(me->window);

    static GdkEventExpose event;
    event.area.x = event.area.y = 0;
    dareaExpose (GTK_WIDGET(me->darea),&event,me);

    return 0;
  };

  bool viewerBase3D::mainWindow::validData() {
    return true;
  };

  // hide method!
  gint viewerBase3D::mainWindow::hide(gpointer data) {

    mainWindow* me = (mainWindow*)(data);
    gtk_widget_hide_all(me->window);
    me->indicateDataHide();

    return 0; // stop timer
  };

  void viewerBase3D::mainWindow::indicateDataHide() {
  };

  // ----------------------------------------------------------
  // redraw the window
  // ----------------------------------------------------------

  gboolean viewerBase3D::mainWindow::dareaExpose (GtkWidget *widget,
              GdkEventExpose *event,
              gpointer data) {

    mainWindow* me = (mainWindow*)(data);
    return (me->dareaExposeLocal(widget,event));
  }

  gboolean
  viewerBase3D::mainWindow::dareaExposeLocal(GtkWidget *widget,
                                             GdkEventExpose *event) {

    theLock.lock();
    busy = true;
    if (notNull(dynamic_cast<parameters*>(param))) {
      // initialize the drawing tools and canvas image
      drawTool.setParameters(param->camParameters);
      // our image must be resized and cleared with the background color
      theImage.resize(param->windowSize,param->backgroundColor,false,true);
      // tell the drawTool to draw in the standard canvas image
      drawTool.use(theImage);
      // initialized the z-buffer
      drawTool.resetDeepCtrl(true);
      // and call the dataToImage member, which will draw all data.
      dataToImage();
      // convert the image into a buffer GTK
      generateXbuffer(theImage,widget,event);
    }
    busy = false;
    showingBuffer = false;
    theLock.unlock();

    return TRUE;
  }

  void viewerBase3D::mainWindow::dataToImage() {
    draw3DAxis(255);
  };

  void viewerBase3D::mainWindow::draw3DAxis(const double ax,
                                            const bool auxBox) {

    // as default implementation show the axis

    if (isNull(dynamic_cast<parameters*>(param))) {
      return;
    }

    int r,g,b;

    if (auxBox) {
      r = abs(int(param->axisColor.getRed()) +
              int(param->backgroundColor.getRed()))/2;

      g = abs(int(param->axisColor.getGreen()) +
              int(param->backgroundColor.getGreen()))/2;

      b = abs(int(param->axisColor.getBlue()) +
              int(param->backgroundColor.getBlue()))/2;

      // auxiliar lines
      drawTool.setColor(rgbPixel(r,g,b));

      drawTool.line3D(  ax,ax,ax,
                      0, ax,ax);
      drawTool.line3D(  ax,0, ax,
                      0, 0, ax);

      drawTool.line3DTo(0, ax,ax);
      drawTool.line3DTo(0 ,ax,0 );
      drawTool.line3DTo(ax,ax,0 );
      drawTool.line3DTo(ax,0 ,0 );
      drawTool.line3DTo(ax,0 ,ax);
      drawTool.line3DTo(ax,ax,ax);
      drawTool.line3DTo(ax,ax,0 );
    }

    // draw axis:
    if (param->axisColorFixed) {
      // fixed
      const double axl = (1.02*ax);
      drawTool.setColor(param->axisColor);
      drawTool.line3D(0,0,0,axl,0,0);
      drawTool.line3D(0,0,0,0,axl,0);
      drawTool.line3D(0,0,0,0,0,axl);
    } else {
      // gradient
      int i;
      const double s = ax/255.0;

      // red axis
      drawTool.setColor(Black);
      drawTool.set3D(0,0,0);
      for (i=0;i<=255;++i) {
        drawTool.setColor(rgbPixel(i,0,0));
        drawTool.line3DTo(i*s,0,0);
      }

      // green axis
      drawTool.set3D(0,0,0);
      for (i=0;i<=255;++i) {
        drawTool.setColor(rgbPixel(0,i,0));
        drawTool.line3DTo(0,i*s,0);
      }

      // blue axis
      drawTool.set3D(0,0,0);
      for (i=0;i<=255;++i) {
        drawTool.setColor(rgbPixel(0,0,i));
        drawTool.line3DTo(0,0,i*s);
      }
    }
  }


  void viewerBase3D::mainWindow::prepareParameters() {
  }

  void
  viewerBase3D::mainWindow::buttonPressedEventLocal(GtkWidget* widget,
                                                    GdkEventButton*  event) {

    const point pos(iround(event->x),iround(event->y));
    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl   = (event->state & GDK_CONTROL_MASK) != 0;

    lastMousePos = pos;

    if (event->button == 3) {
      // right mouse button
      if (!shift && !ctrl) {
        prepareParameters();
        // options window
        options->whereToCallBack(this);
        options->useParameters(*param);
        options->show();
      }
    }

    mouseButtonPressed = event->button;

    if (event->type == GDK_BUTTON_RELEASE) {
      mouseButtonPressed = 0;
      redraw();
    }

    if (!busy && notNull(dynamic_cast<parameters*>(param))) {
      if (event->button == 4) {
        param->camParameters.zoom*=1.1;
        redraw();
      } else if (event->button == 5) {
        param->camParameters.zoom/=1.1;
        redraw();
      }

      showCameraPosition();
    }
  };

  void viewerBase3D::mainWindow::mouseMovedEventLocal(GtkWidget* widget,
                                                      GdkEventMotion*  event) {
    const point pos(iround(event->x),iround(event->y));
    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl   = (event->state & GDK_CONTROL_MASK) != 0;

    if (isNull(dynamic_cast<parameters*>(param))) {
      return;
    }

    if (busy)
      return;

    mouseMovedHandler(mouseButtonPressed,pos,shift,ctrl);

    lastMousePos = pos;

  };


# ifndef _LTI_MSC_6
  void viewerBase3D::mainWindow::mouseMovedHandler(const int& button,
                                                   const point& pos,
                                                   const bool shift,
                                                   const bool ctrl)

# else
  void viewerBase3D::mainWindow::mouseMovedHandlerMS(const int& button,
                                                     const point& pos,
                                                     const bool shift,
                                                     const bool ctrl)
# endif
  {
    draw3D<rgbPixel>::parameters& par = param->camParameters;

    if (button == 1) {
      if (!shift && !ctrl) {

        // rotate the camera axis
        double& e = par.elevation;
        const double deltaElevation =
          -double(pos.y - lastMousePos.y)*Pi/180;
        e += deltaElevation;
        if (e<0) e = 0;
        if (e>Pi) e = Pi;

        double& a = par.azimuth;
        const double deltaAzimuth =
          -double(pos.x -lastMousePos.x)*Pi/180;
        a += deltaAzimuth;
        while (a>2*Pi) a -= 2*Pi;
        while (a<0) a += 2*Pi;

        const double camR = param->camRadius;
        par.camPos = dpoint3D(param->camTarget.x+camR*sin(e)*cos(a),
                              param->camTarget.y+camR*sin(e)*sin(a),
                              param->camTarget.z+camR*cos(e));

      } else if (shift) {
        double deltaZoom = double(lastMousePos.y - pos.y)/100.0;
        deltaZoom = 2.0 / (1.0+exp(deltaZoom));

        par.zoom *= deltaZoom;
      } else if (ctrl) {
        if (!theImage.empty()) {
          par.center.x = double(pos.x)/theImage.columns();
          par.center.y = double(pos.y)/theImage.rows();
        }
      }
    }

    redraw();
    showCameraPosition();
  }

# ifdef _LTI_MSC_6
  void viewerBase3D::mainWindow::mouseMovedHandler(const int& button,
                                                   const point& pos,
                                                   const bool shift,
                                                   const bool ctrl) {
    // ...we need this workaround to cope with another really awful MSVC bug.
    mouseMovedHandlerMS(button,pos,shift,ctrl);
  }
# endif


  // ----------------------------------------------------
  //        generate the xbuffer from an image
  // ----------------------------------------------------

  void viewerBase3D::mainWindow::generateXbuffer(const image& img,
             GtkWidget *widget,
             GdkEventExpose *event) {

    bool newData = (img.size() != point(xbuffer_cols,xbuffer_rows));

   if (newData) {
      xbuffer_cols = img.columns();
      xbuffer_rows = img.rows();
    }

    // xbuffer_cols and rows were set the last time, when xbuffer was set!
    point wsize(xbuffer_cols+borderWidth.x+1,xbuffer_rows+borderWidth.y);

    if (isNull(xbuffer) || dataChanged) {
      // free old data
      delete[] xbuffer;
      xbuffer = 0;

      const int size = xbuffer_cols*xbuffer_rows*3;
      xbuffer = new guchar[size];
      guchar* ptr = xbuffer;
      vector<rgbPixel>::const_iterator it,e;

      // create image
      point p;
      for (p.y=0;p.y<img.rows();++p.y) {
        const vector<rgbPixel>& vct = img.getRow(p.y);
        for (it = vct.begin(),e=vct.end();it!=e;++it) {
          *ptr++ = static_cast<guchar>((*it).getRed());
          *ptr++ = static_cast<guchar>((*it).getGreen());
          *ptr++ = static_cast<guchar>((*it).getBlue());
        }
      }

      dataChanged = false;
    }

    if (winSize == point(0,0)) {
      if ((winSize.x > wsize.x) ||
          (winSize.y > wsize.y)) {
        // clear unused part of the window!
        gdk_window_clear_area (widget->window,
                               event->area.x, event->area.y,
                               winSize.x, winSize.y);
      }

      if (winSize != wsize) {
        winSize = wsize;
        gtk_window_set_default_size(GTK_WINDOW(window),
                                    wsize.x,
                                    wsize.y);

        gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_MOUSE);
      }

    }

    if (newData) {
      gtk_drawing_area_size(GTK_DRAWING_AREA(darea),
                            xbuffer_cols,
                            xbuffer_rows);
    }

    gdk_draw_rgb_image (widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, 0, xbuffer_cols, xbuffer_rows,
                        GDK_RGB_DITHER_NORMAL, xbuffer,
                        xbuffer_cols*3);
  }

  void viewerBase3D::mainWindow::showCameraPosition() {

    if (isNull(dynamic_cast<parameters*>(param))) {
      return;
    }

    // set camParameters to valid values
    draw3D<rgbPixel>::parameters& par = param->camParameters;
    drawTool.setParameters(par);
    par = drawTool.getParameters();

    int e = static_cast<int>(0.5+par.elevation*180/Pi);
    int a = static_cast<int>(0.5+par.azimuth*180/Pi);
    float z = par.zoom;

    static char strbuffer[256];

    gtk_statusbar_pop(GTK_STATUSBAR(statusBar),statusBarContextId);

    sprintf(strbuffer,"  E: %d  A: %d  Z: %.3f ",e,a,z);
    gtk_statusbar_push( GTK_STATUSBAR(statusBar),
                        statusBarContextId,
                        strbuffer);
  }

  gint viewerBase3D::mainWindow::mouseMovedEvent(GtkWidget* widget,
                                                    GdkEventMotion* event,
                                                    gpointer data) {

    mainWindow* me = (mainWindow*)(data);
    me->mouseMovedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  gint viewerBase3D::mainWindow::buttonPressedEvent(GtkWidget* widget,
                                        GdkEventButton* event,
                                        gpointer data) {

    mainWindow* me = (mainWindow*)(data);
    me->buttonPressedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }


  // -----------------------------------------------------------------------
  // Configuration Dialog
  // -----------------------------------------------------------------------

  viewerBase3D::configDialog::configDialog() {

    // Color selector
    colorSelector = new colorDialog();

    userdefBackcolor = lti::Black;

    // File Selector
    filew = gtk_file_selection_new ("File selection");

    // Connect the ok_button to file_ok_sel function
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
      "clicked", (GtkSignalFunc) chooseFilename_callback,
                        (gpointer)this);

    // Connect the cancel_button to destroy the widget
    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
                                            (filew)->cancel_button),
             "clicked", (GtkSignalFunc) gtk_widget_hide,
             GTK_OBJECT (filew));

    // Lets set the filename, as if this were a save dialog, and we
    // are giving a default filename
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew),
             "*.bmp");

    // parameters not set yet!
    param = 0;

    settings = gtk_dialog_new();
    tooltips = gtk_tooltips_new ();
  }

  viewerBase3D::configDialog::~configDialog() {
    hide();

    gtk_widget_destroy(settings);
    gtk_widget_destroy(filew);

    //destroy combo entries
    GList* delP = g_list_first (comboNamelist);
    while(delP != NULL) {
      delete[] (char*)delP->data;
      delP = g_list_next(delP);
    }
    g_list_free(comboNamelist);

    delete colorSelector;
    colorSelector = 0;

  }

# ifndef _LTI_MSC_6
  bool viewerBase3D::configDialog::setDialogData()
# else
  bool viewerBase3D::configDialog::setDialogDataMS()
# endif
  {
    parameters* param = dynamic_cast<parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }

    if (param->backgroundColor == lti::Black) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(backBlack),TRUE);
    } else if (param->backgroundColor == lti::rgbPixel(128,128,128)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(backGrey),TRUE);
    } else if (param->backgroundColor == lti::White) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(backWhite),TRUE);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(backUserdef),TRUE);
    }

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(sizex),param->windowSize.x);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(sizey),param->windowSize.y);

    if (param->axisColorFixed) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(axisFixed),TRUE);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(axisGradient),TRUE);
    }

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(elevation),
                               int(180.0*param->camParameters.elevation/Pi+0.5));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(azimuth),
                               int(180.0*param->camParameters.azimuth/Pi+0.5));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(zoom),
             param->camParameters.zoom);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(perspective),
             param->camParameters.perspective*1000);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(centerx),
             param->camParameters.center.x);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(centery),
             param->camParameters.center.y);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(camx),
             param->camTarget.x);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(camy),
             param->camTarget.y);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(camz),
             param->camTarget.z);

    return true;
  }

# ifndef _LTI_MSC_6
  bool viewerBase3D::configDialog::getDialogData()
# else
  bool viewerBase3D::configDialog::getDialogDataMS()
#endif
  {
    parameters* param = dynamic_cast<parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(backBlack)) == TRUE)
      param->backgroundColor = lti::Black;
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(backGrey)) == TRUE)
      param->backgroundColor = rgbPixel(128,128,128);
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(backWhite)) == TRUE)
      param->backgroundColor = lti::White;
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(backUserdef)) == TRUE)
      param->backgroundColor = userdefBackcolor;

    param->windowSize.x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sizex));
    param->windowSize.y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sizey));

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(axisFixed)) ==TRUE)
      param->axisColorFixed = true;
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(axisGradient)) == TRUE)
      param->axisColorFixed = false;

    param->camParameters.elevation =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(elevation))*Pi/180.0;
    param->camParameters.azimuth =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(azimuth))*Pi/180.0;
    param->camParameters.zoom =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(zoom));
    param->camParameters.perspective =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(perspective))/1000;

    param->camParameters.center.x =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(centerx));
    param->camParameters.center.y =
      gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(centery));

    param->camTarget.x =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(camx));
    param->camTarget.y =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(camy));
    param->camTarget.z =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(camz));

    const double& e=param->camParameters.elevation;
    const double& a=param->camParameters.azimuth;
    const double& camR = param->camRadius;

    param->camParameters.camPos =
      dpoint3D(param->camTarget.x+camR*sin(e)*cos(a),
               param->camTarget.y+camR*sin(e)*sin(a),
               param->camTarget.z+camR*cos(e));

    return true;
  }

# ifdef _LTI_MSC_6
  bool viewerBase3D::configDialog::getDialogData() {
    return getDialogDataMS();
  }

  bool viewerBase3D::configDialog::setDialogData() {
    return setDialogDataMS();
  }
# endif

  bool
  viewerBase3D::configDialog::useParameters(parameters& theParam) {
    if (dynamic_cast<parameters*>(&theParam) != 0) {
      param = &theParam;
      return true;
    }
    param = 0;
    return false;
  }

  void viewerBase3D::configDialog::show() {
    setDialogData();
    gtk_widget_show(settings);
  }

  void viewerBase3D::configDialog::hide() {
    colorSelector->hide();
    gtk_widget_hide(filew);
    gtk_widget_hide(settings);
  }

  void viewerBase3D::configDialog::appendPage(pageWidget pw) {
    pageList.push_back(pw);
  }

  viewerBase3D::configDialog::pageWidget
  viewerBase3D::configDialog::buildCommonPage() {

  // the widget object that will be returned
  pageWidget pw;
  pw.label = "Common";

  // create the GTK widget
  GtkWidget* vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox6);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox6", vbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox6);

  GtkWidget* hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox5);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox5", hbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox6), hbox5, TRUE, TRUE, 0);

  GtkWidget* frame4 = gtk_frame_new ("Background Color");
  gtk_widget_ref (frame4);
  gtk_object_set_data_full (GTK_OBJECT (settings), "frame4", frame4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (hbox5), frame4, TRUE, TRUE, 0);
  gtk_widget_set_usize (frame4, -2, 113);

  GtkWidget* vbox7 = gtk_vbox_new (TRUE, 0);
  gtk_widget_ref (vbox7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox7", vbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (frame4), vbox7);

  GSList *back_group = NULL;
  backBlack = gtk_radio_button_new_with_label (back_group, "Black");
  back_group = gtk_radio_button_group (GTK_RADIO_BUTTON (backBlack));
  gtk_widget_ref (backBlack);
  gtk_object_set_data_full (GTK_OBJECT (settings), "backBlack", backBlack,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (backBlack);
  gtk_box_pack_start (GTK_BOX (vbox7), backBlack, FALSE, FALSE, 0);
  gtk_widget_set_usize (backBlack, 92, 20);

  backGrey = gtk_radio_button_new_with_label (back_group, "Grey");
  back_group = gtk_radio_button_group (GTK_RADIO_BUTTON (backGrey));
  gtk_widget_ref (backGrey);
  gtk_object_set_data_full (GTK_OBJECT (settings), "backGrey", backGrey,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (backGrey);
  gtk_box_pack_start (GTK_BOX (vbox7), backGrey, FALSE, FALSE, 0);

  backWhite = gtk_radio_button_new_with_label (back_group, "White");
  back_group = gtk_radio_button_group (GTK_RADIO_BUTTON (backWhite));
  gtk_widget_ref (backWhite);
  gtk_object_set_data_full (GTK_OBJECT (settings), "backWhite", backWhite,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (backWhite);
  gtk_box_pack_start (GTK_BOX (vbox7), backWhite, FALSE, FALSE, 0);

  GtkWidget* hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox6);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox6", hbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (vbox7), hbox6, FALSE, FALSE, 0);

  //pointer in header because need of control
  backUserdef = gtk_radio_button_new_with_label (back_group, "User Defined");
  back_group = gtk_radio_button_group (GTK_RADIO_BUTTON (backUserdef));
  gtk_widget_ref (backUserdef);
  gtk_object_set_data_full (GTK_OBJECT (settings), "backUserdef", backUserdef,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (backUserdef);
  gtk_box_pack_start (GTK_BOX (hbox6), backUserdef, FALSE, FALSE, 0);

  GtkWidget* backChoose = gtk_button_new_with_label ("   ...   ");
  gtk_widget_ref (backChoose);
  gtk_object_set_data_full (GTK_OBJECT (settings), "backChoose", backChoose,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (backChoose);
  gtk_box_pack_end (GTK_BOX (hbox6), backChoose, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, backChoose, "Choose background color", NULL);

  GtkWidget* vbox8 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox8);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox8", vbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox8);
  gtk_box_pack_start (GTK_BOX (hbox5), vbox8, TRUE, TRUE, 0);

  GtkWidget* axisColor = gtk_frame_new ("Axis Color");
  gtk_widget_ref (axisColor);
  gtk_object_set_data_full (GTK_OBJECT (settings), "axisColor", axisColor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (axisColor);
  gtk_box_pack_start (GTK_BOX (vbox8), axisColor, TRUE, TRUE, 0);

  GtkWidget* vbox9 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox9", vbox9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox9);
  gtk_container_add (GTK_CONTAINER (axisColor), vbox9);

  GSList *axis_group = NULL;
  axisGradient = gtk_radio_button_new_with_label (axis_group, "Gradient");
  axis_group = gtk_radio_button_group (GTK_RADIO_BUTTON (axisGradient));
  gtk_widget_ref (axisGradient);
  gtk_object_set_data_full (GTK_OBJECT (settings), "axisGradient", axisGradient,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (axisGradient);
  gtk_box_pack_start (GTK_BOX (vbox9), axisGradient, TRUE, FALSE, 0);

  GtkWidget* hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox7", hbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox7, TRUE, FALSE, 0);

  axisFixed = gtk_radio_button_new_with_label (axis_group, "Fixed");
  axis_group = gtk_radio_button_group (GTK_RADIO_BUTTON (axisFixed));
  gtk_widget_ref (axisFixed);
  gtk_object_set_data_full (GTK_OBJECT (settings), "axisFixed", axisFixed,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (axisFixed);
  gtk_box_pack_start (GTK_BOX (hbox7), axisFixed, FALSE, FALSE, 0);

  GtkWidget* axisChoose = gtk_button_new_with_label ("   ...   ");
  gtk_widget_ref (axisChoose);
  gtk_object_set_data_full (GTK_OBJECT (settings), "axisChoose", axisChoose,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (axisChoose);
  gtk_box_pack_end (GTK_BOX (hbox7), axisChoose, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, axisChoose, "Choose axis color", NULL);

  GtkWidget* winSize = gtk_frame_new ("Window Size");
  gtk_widget_ref (winSize);
  gtk_object_set_data_full (GTK_OBJECT (settings), "winSize", winSize,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (winSize);
  gtk_box_pack_start (GTK_BOX (vbox8), winSize, TRUE, TRUE, 0);

  GtkWidget* vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox10", vbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox10);
  gtk_container_add (GTK_CONTAINER (winSize), vbox10);

  GtkWidget* hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox8);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox8", hbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox8, TRUE, FALSE, 0);

  GtkWidget* label3 = gtk_label_new (" X: ");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox8), label3, FALSE, FALSE, 0);

  GtkObject* sizex_adj = gtk_adjustment_new (100, 1, 1000, 1, 10, 10);
  sizex = gtk_spin_button_new (GTK_ADJUSTMENT (sizex_adj), 1, 0);
  gtk_widget_ref (sizex);
  gtk_object_set_data_full (GTK_OBJECT (settings), "sizex", sizex,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (sizex);

  gtk_box_pack_start (GTK_BOX (hbox8), sizex, TRUE, TRUE, 0);

  GtkWidget* hbox9 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox9", hbox9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox9);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox9, TRUE, FALSE, 0);

  GtkWidget* label4 = gtk_label_new (" Y: ");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox9), label4, FALSE, FALSE, 0);

  GtkObject* sizey_adj = gtk_adjustment_new (100, 1, 1000, 1, 10, 10);
  sizey = gtk_spin_button_new (GTK_ADJUSTMENT (sizey_adj), 1, 0);
  gtk_widget_ref (sizey);
  gtk_object_set_data_full (GTK_OBJECT (settings), "sizey", sizey,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (sizey);
  gtk_box_pack_start (GTK_BOX (hbox9), sizey, TRUE, TRUE, 0);

  GtkWidget* frame7 = gtk_frame_new ("3D Camera");
  gtk_widget_ref (frame7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "frame7", frame7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame7);
  gtk_box_pack_start (GTK_BOX (vbox6), frame7, TRUE, TRUE, 0);

  GtkWidget* hbox10 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox10", hbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox10);
  gtk_container_add (GTK_CONTAINER (frame7), hbox10);

  GtkWidget* table1 = gtk_table_new (4, 2, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (hbox10), table1, TRUE, TRUE, 0);

  GtkWidget* label5 = gtk_label_new (" Elevation: ");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  GtkWidget* label6 = gtk_label_new (" Azimuth: ");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  GtkWidget* label7 = gtk_label_new (" Zoom: ");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  GtkWidget* label8 = gtk_label_new (" Perspective (:1000) ");
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  GtkObject* elevation_adj = gtk_adjustment_new (0, 0, 360, 1, 10, 10);
  elevation = gtk_spin_button_new (GTK_ADJUSTMENT (elevation_adj), 1, 0);
  gtk_widget_ref (elevation);
  gtk_object_set_data_full (GTK_OBJECT (settings), "elevation", elevation,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (elevation);
  gtk_table_attach (GTK_TABLE (table1), elevation, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip(tooltips, elevation,
           "Angle between z-axis and camera axis [Mouse Left Button + Up/Down]", NULL);

  GtkObject* azimuth_adj = gtk_adjustment_new (0, 0, 360, 1, 10, 10);
  azimuth = gtk_spin_button_new (GTK_ADJUSTMENT (azimuth_adj), 1, 0);
  gtk_widget_ref (azimuth);
  gtk_object_set_data_full (GTK_OBJECT (settings), "azimuth", azimuth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (azimuth);
  gtk_table_attach (GTK_TABLE (table1), azimuth, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, azimuth,
      "Angle between x-axis and projection of camera axis on xy-plane [Mouse Left Button + Left/Right]", NULL);

  GtkObject* zoom_adj = gtk_adjustment_new (0.5, 0.1, 100, 0.1, 10, 10);
  zoom = gtk_spin_button_new (GTK_ADJUSTMENT (zoom_adj), 0.1, 1);
  gtk_widget_ref (zoom);
  gtk_object_set_data_full (GTK_OBJECT (settings), "zoom", zoom,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom);
  gtk_table_attach (GTK_TABLE (table1), zoom, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, zoom,
      "Zoom factor [Shift + Mouse Left Button + Up/Down, or Wheel]", NULL);

  GtkObject* perspective_adj = gtk_adjustment_new (1, 0, 100, 0.1, 10, 10);
  perspective = gtk_spin_button_new (GTK_ADJUSTMENT (perspective_adj), 0.1, 1);
  gtk_widget_ref (perspective);
  gtk_object_set_data_full (GTK_OBJECT (settings), "perspective", perspective,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (perspective);
  gtk_table_attach (GTK_TABLE (table1), perspective, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, perspective,
      "Use 0 for othographic projection or greater for perspective.", NULL);

  GtkWidget* frame10 = gtk_frame_new ("Center Image Coordinates");
  gtk_widget_ref (frame10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "frame10", frame10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame10);
  gtk_box_pack_start (GTK_BOX (hbox10), frame10, TRUE, TRUE, 0);

  GtkWidget* table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (table2);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table2", table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (frame10), table2);

  GtkWidget* label9 = gtk_label_new (" X: ");
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table2), label9, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  GtkWidget* label10 = gtk_label_new (" Y: ");
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  GtkWidget* label11 = gtk_label_new ("");
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table2), label11, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  GtkObject* centerx_adj = gtk_adjustment_new (0.5, 0, 1, 0.01, 10, 10);
  centerx = gtk_spin_button_new (GTK_ADJUSTMENT (centerx_adj), 0.01, 2);
  gtk_widget_ref (centerx);
  gtk_object_set_data_full (GTK_OBJECT (settings), "centerx", centerx,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (centerx);
  gtk_table_attach (GTK_TABLE (table2), centerx, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, centerx,
      "Relative x-coordinate for the center (between 0 and 1)", NULL);

  GtkObject* centery_adj = gtk_adjustment_new (0.5, 0, 1, 0.01, 10, 10);
  centery = gtk_spin_button_new (GTK_ADJUSTMENT (centery_adj), 0.01, 2);
  gtk_widget_ref (centery);
  gtk_object_set_data_full (GTK_OBJECT (settings), "centery", centery,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (centery);
  gtk_table_attach (GTK_TABLE (table2), centery, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, centery,
      "Relative y-coordinate for the center (between 0 and 1)", NULL);

  GtkWidget* frame9 = gtk_frame_new ("Camera Target (3D Space)");
  gtk_widget_ref (frame9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "frame9", frame9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame9);
  gtk_box_pack_start (GTK_BOX (hbox10), frame9, TRUE, TRUE, 0);

  GtkWidget* table3 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (table3);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table3", table3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (frame9), table3);

  GtkWidget* label12 = gtk_label_new (" X: ");
  gtk_widget_ref (label12);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label12", label12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table3), label12, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  GtkWidget* label13 = gtk_label_new (" Y: ");
  gtk_widget_ref (label13);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label13", label13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label13);
  gtk_table_attach (GTK_TABLE (table3), label13, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);

  GtkWidget* label14 = gtk_label_new (" Z: ");
  gtk_widget_ref (label14);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label14", label14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label14);
  gtk_table_attach (GTK_TABLE (table3), label14, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);

  GtkObject* camx_adj = gtk_adjustment_new (128, -1000, 1000, 1, 10, 10);
  camx = gtk_spin_button_new (GTK_ADJUSTMENT (camx_adj), 1, 0);
  gtk_widget_ref (camx);
  gtk_object_set_data_full (GTK_OBJECT (settings), "camx", camx,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camx);
  gtk_table_attach (GTK_TABLE (table3), camx, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  GtkObject* camy_adj = gtk_adjustment_new (128, -1000, 1000, 1, 10, 10);
  camy = gtk_spin_button_new (GTK_ADJUSTMENT (camy_adj), 1, 0);
  gtk_widget_ref (camy);
  gtk_object_set_data_full (GTK_OBJECT (settings), "camy", camy,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camy);
  gtk_table_attach (GTK_TABLE (table3), camy, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  GtkObject* camz_adj = gtk_adjustment_new (128, -1000, 1000, 1, 10, 10);
  camz = gtk_spin_button_new (GTK_ADJUSTMENT (camz_adj), 1, 0);
  gtk_widget_ref (camz);
  gtk_object_set_data_full (GTK_OBJECT (settings), "camz", camz,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camz);
  gtk_table_attach (GTK_TABLE (table3), camz, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  //signals

  gtk_signal_connect (GTK_OBJECT (backChoose), "clicked",
          GTK_SIGNAL_FUNC (on_backChoose_clicked),
          this);
  gtk_signal_connect (GTK_OBJECT (axisChoose), "clicked",
          GTK_SIGNAL_FUNC (on_axisChoose_clicked),
          this);

  //instead: gtk_container_add (GTK_CONTAINER (xxx), vbox6);
  //just notify the widget to the external program using the returned object.
  pw.widget = vbox6;

  return pw;

  };

  void viewerBase3D::configDialog::insertPages() {
    appendPage(buildCommonPage());
  }

  void viewerBase3D::configDialog::buildDialog() {
    insertPages();

    //settings = gtk_dialog_new (); move in constructor
    gtk_widget_set_name (settings, "settings");
    gtk_object_set_data (GTK_OBJECT (settings), "settings", settings);
    gtk_window_set_title (GTK_WINDOW (settings), "Settings");
    GTK_WINDOW (settings)->type = GTK_WINDOW_DIALOG;
    gtk_window_set_policy (GTK_WINDOW (settings), TRUE, TRUE, FALSE);

    GtkWidget* vboxDialog = GTK_DIALOG (settings)->vbox;
    gtk_widget_set_name (vboxDialog, "vboxDialog");
    gtk_object_set_data (GTK_OBJECT (settings), "vboxDialog", vboxDialog);
    gtk_widget_show (vboxDialog);

    GtkWidget* notebook = gtk_notebook_new ();
    gtk_widget_set_name (notebook, "notebook");
    gtk_widget_ref (notebook);
    gtk_object_set_data_full (GTK_OBJECT (settings), "notebook", notebook,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (notebook);
    gtk_box_pack_start (GTK_BOX (vboxDialog), notebook, TRUE, TRUE, 0);

    // paste page widgets
    int i = 0;
    std::list<pageWidget>::iterator it = pageList.begin();
    while(it != pageList.end()) {

      gtk_container_add (GTK_CONTAINER (notebook), (*it).widget);

      // label page of special widget
      GtkWidget* label = gtk_label_new ((*it).label.c_str());
      gtk_widget_set_name (label, (*it).label.c_str());
      gtk_widget_ref (label);
      gtk_object_set_data_full (GTK_OBJECT (settings), (*it).label.c_str(), label,
        (GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (label);
      gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
          gtk_notebook_get_nth_page(GTK_NOTEBOOK (notebook),i), label);
      i++;
      it++;
    }

    GtkWidget* hboxSave = gtk_hbox_new (FALSE, 0);
    gtk_widget_set_name (hboxSave, "hboxSave");
    gtk_widget_ref (hboxSave);
    gtk_object_set_data_full (GTK_OBJECT (settings), "hboxSave", hboxSave,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (hboxSave);
    gtk_box_pack_start (GTK_BOX (vboxDialog), hboxSave, FALSE, TRUE, 0);

    GtkWidget* chooseFilename = gtk_button_new_with_label ("   ...   ");
    gtk_widget_set_name (chooseFilename, "chooseFilename");
    gtk_widget_ref (chooseFilename);
    gtk_object_set_data_full (GTK_OBJECT (settings), "chooseFilename", chooseFilename,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (chooseFilename);
    gtk_box_pack_start (GTK_BOX (hboxSave), chooseFilename, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (chooseFilename), 5);

    comboFilename = gtk_combo_new ();
    gtk_widget_set_name (comboFilename, "comboFilename");
    gtk_widget_ref (comboFilename);
    gtk_object_set_data_full (GTK_OBJECT (settings), "comboFilename", comboFilename,
            (GtkDestroyNotify) gtk_widget_unref);
    comboNamelist = NULL;

    char* fn = NULL;
    newStrCopy(fn,"image.png");
    if (notNull(dynamic_cast<parameters*>(param))) {
      newStrCopy(fn,param->saveName.c_str());
    }
    comboNamelist = g_list_append(comboNamelist, (gpointer*)fn);
    gtk_combo_set_popdown_strings(GTK_COMBO (comboFilename), comboNamelist);
    gtk_widget_show (comboFilename);
    gtk_box_pack_start (GTK_BOX (hboxSave), comboFilename, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (comboFilename), 5);

    GtkWidget* entryFilename = GTK_COMBO (comboFilename)->entry;
    gtk_widget_set_name (entryFilename, "entryFilename");
    gtk_widget_ref (entryFilename);
    gtk_object_set_data_full (GTK_OBJECT (settings), "entryFilename", entryFilename,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (entryFilename);

    GtkWidget* save = gtk_button_new_with_label (" Save ");
    gtk_widget_set_name (save, "save");
    gtk_widget_ref (save);
    gtk_object_set_data_full (GTK_OBJECT (settings), "save", save,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (save);
    gtk_box_pack_start (GTK_BOX (hboxSave), save, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (save), 5);

    GtkWidget* dialog_action_area = GTK_DIALOG (settings)->action_area;
    gtk_widget_set_name (dialog_action_area, "dialog_action_area");
    gtk_object_set_data (GTK_OBJECT (settings), "dialog_action_area", dialog_action_area);
    gtk_widget_show (dialog_action_area);
    gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 10);

    GtkWidget* ok =  gtk_button_new_with_label ("   OK   ");
    gtk_widget_set_name (ok, "ok");
    gtk_widget_ref (ok);
    gtk_object_set_data_full (GTK_OBJECT (settings), "ok", ok,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ok);
    gtk_box_pack_start (GTK_BOX (dialog_action_area), ok, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (ok), 5);

    GtkWidget* cancel = gtk_button_new_with_label (" Cancel ");
    gtk_widget_set_name (cancel, "cancel");
    gtk_widget_ref (cancel);
    gtk_object_set_data_full (GTK_OBJECT (settings), "cancel", cancel,
            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (cancel);
    gtk_box_pack_start (GTK_BOX (dialog_action_area), cancel, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (cancel), 5);

    GtkWidget* apply = gtk_button_new_with_label (" Apply ");
    gtk_widget_set_name (apply, "apply");
    gtk_widget_ref (apply);
    gtk_object_set_data_full (GTK_OBJECT (settings), "apply", apply,
                            (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (apply);
    gtk_box_pack_start (GTK_BOX (dialog_action_area), apply, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (apply), 5);

    gtk_signal_connect (GTK_OBJECT (ok), "clicked",
                        GTK_SIGNAL_FUNC (on_ok_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (cancel), "clicked",
                        GTK_SIGNAL_FUNC (on_cancel_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (apply), "clicked",
                        GTK_SIGNAL_FUNC (on_apply_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (chooseFilename), "clicked",
                        GTK_SIGNAL_FUNC (on_chooseFilename_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (save), "clicked",
                        GTK_SIGNAL_FUNC (on_save_clicked),
                        this);

//      gtk_signal_connect (GTK_OBJECT (settings), "delete_event",
//                          GTK_SIGNAL_FUNC (on_close_clicked),
//                          this);

    gtk_object_set_data (GTK_OBJECT (settings), "tooltips", tooltips);

  }

  void viewerBase3D::configDialog::newStrCopy(char* &dest, const char* src) {
    if(dest)
      delete[] dest;
    const int l = strlen(src)+1;
    dest = new char[l];
    strcpy(dest,src);
  };

  std::string viewerBase3D::configDialog::toString(const double& value,
               const int& dec) {
    char phrase[8];
    sprintf(phrase,"%s%i%s","%.",dec,"f");
    char tmpBuf[64];
    sprintf(tmpBuf,phrase,value); //"%.4f"
    return std::string(tmpBuf);
  };

  double viewerBase3D::configDialog::fromString(const char* value) {
    return atof(value);
  };

  /*
   * set the parent of this dialog to tell it when everything is done!
   */
  void viewerBase3D::configDialog::whereToCallBack(mainWindow* obj) {
    theMainWindow = obj;
  }

  // --------------------------------------------------------------------------
  // Static callbacks
  // --------------------------------------------------------------------------

  void viewerBase3D::configDialog::on_backChoose_clicked(GtkButton *button,
                                                         gpointer me) {
    configDialog* mee = (configDialog*)me;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (mee->backUserdef),TRUE);

    mee->colorSelector->useColor(mee->userdefBackcolor); //set color per reference
    mee->colorSelector->show();
  };

  void viewerBase3D::configDialog::on_axisChoose_clicked(GtkButton *button,
                                                         gpointer me) {

    configDialog* mee = (configDialog*)me;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (mee->axisFixed),TRUE);

    if (notNull(dynamic_cast<parameters*>(mee->param))) {
      mee->colorSelector->useColor(mee->param->axisColor);
    }

    mee->colorSelector->show();
  };

  void viewerBase3D::configDialog::on_ok_clicked(GtkButton *button,
             gpointer me) {
    configDialog* mee = (configDialog*)me;
    mee->getDialogData();
    mee->redrawMainWindow();
    mee->hide();
  }

  void viewerBase3D::configDialog::on_apply_clicked(GtkButton *button,
                                                    gpointer me) {
    configDialog* mee = (configDialog*)me;
    mee->getDialogData();
    mee->redrawMainWindow();
  };

  void viewerBase3D::configDialog::redrawMainWindow() {
    theMainWindow->useParameters(*param);
    theMainWindow->redraw();
  };

  void viewerBase3D::configDialog::on_cancel_clicked(GtkButton *button,
                 gpointer me) {
    configDialog* mee = (configDialog*)me;
    mee->hide();
  }

//    gint viewerBase3D::configDialog::on_close_clicked(GtkButton *button,
//                  gpointer me) {
//      configDialog* mee = (configDialog*)me;
//      mee->hide();
//      return TRUE;
//    }

  void viewerBase3D::configDialog::on_chooseFilename_clicked(GtkButton *button,
                   gpointer me) {
    configDialog* mee = (configDialog*)me;
    gtk_widget_show(mee->filew);
  }

  void viewerBase3D::configDialog::chooseFilename_callback(GtkWidget *widget,
                 gpointer me) {
    configDialog* mee = (configDialog*)me;
    gtk_widget_hide(mee->filew);

    std::string tString =
      gtk_file_selection_get_filename (GTK_FILE_SELECTION (mee->filew));

    //check for double entries
    if(mee->comboNamelist != NULL) {
      GList* pList = g_list_first(mee->comboNamelist);
      do {
        if(tString == std::string((char*)pList->data)) {
          return;
        }
        pList = pList->next;
      } while(pList != NULL);
    }

    //new entry -> set choosen filename in combobox
    char* fn = NULL;
    mee->newStrCopy(fn,tString.c_str());
    mee->comboNamelist = g_list_prepend (mee->comboNamelist,(gpointer*)fn);
#ifndef _LTI_MSC_6
    gtk_combo_set_popdown_strings (GTK_COMBO(mee->comboFilename),
           mee->comboNamelist);
#endif
  };

  void viewerBase3D::configDialog::on_save_clicked(GtkButton *button,
               gpointer me) {
    configDialog* mee = (configDialog*)me;

    std::string filename(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(mee->comboFilename)->entry)));
    const lti::image& viewed =
      (mee->theMainWindow)->getDisplayedData();
    lti::saveImage saver;
    saver.save(filename,viewed);

    if(mee->comboNamelist != NULL) {
      //further: if entry is new ? save it in comboNamelist
      GList* pList = g_list_first(mee->comboNamelist);
      do {
        if(filename == std::string((char*)pList->data)) {
          return;
        }
        pList = pList->next;
      } while(pList != NULL);
    }

    //new entry -> set choosen filename in combobox
    char *fn = NULL;
    mee->newStrCopy(fn,filename.c_str());
    mee->comboNamelist = g_list_prepend (mee->comboNamelist,
					 (gpointer*)fn);
#ifndef _LTI_MSC_6
    gtk_combo_set_popdown_strings (GTK_COMBO(mee->comboFilename),
           mee->comboNamelist);
#endif
  };

}
#endif
