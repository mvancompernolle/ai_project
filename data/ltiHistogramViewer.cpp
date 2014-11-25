/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiHistogramViewer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.6.2002
 * revisions ..: $Id: ltiHistogramViewer.cpp,v 1.11 2006/09/05 10:43:19 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiHistogramViewer.h"

#ifdef HAVE_GTK

#include "ltiGtkServer.h"
#include "ltiBMPFunctor.h"
#include "ltiViewerFunctor.h"
#include <limits>

#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // viewerBase3D::parameters
  // --------------------------------------------------

  // default constructor
  histogramViewer::parameters::parameters()
    : viewerBase3D::parameters() {

    title = "Histogram";
    cells = tpoint3D<int>(32,32,32);
    useBoxes = true;
    greyEntries = false;
    useLines = false;
    binThreshold = 0.0;
    infoMaxEntry = -1.0;
    infoNumEntries = -1.0;
    infoCells.clear();
  };

  // copy constructor
  histogramViewer::parameters::parameters(const parameters& other)
    : viewerBase3D::parameters()  {
    copy(other);
  }

  // destructor
  histogramViewer::parameters::~parameters() {
  }

  // get type name
  const char* histogramViewer::parameters::getTypeName() const {
    return "histogramViewer::parameters";
  }

  // copy member

  histogramViewer::parameters&
    histogramViewer::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    viewerBase3D::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    viewerBase3D::parameters& (viewerBase3D::parameters::* p_copy)
      (const viewerBase3D::parameters&) =
      viewerBase3D::parameters::copy;
    (this->*p_copy)(other);
# endif

    cells = other.cells;
    useBoxes = other.useBoxes;
    greyEntries = other.greyEntries;
    useLines = other.useLines;
    binThreshold = other.binThreshold;
    infoMaxEntry = other.infoMaxEntry;
    infoNumEntries = other.infoNumEntries;
    infoCells = other.infoCells;

    return *this;
  }

  // clone member
  functor::parameters* histogramViewer::parameters::clone() const {
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
  bool histogramViewer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool histogramViewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"cells",cells);
      lti::write(handler,"useBoxes",useBoxes);
      lti::write(handler,"greyEntries",greyEntries);
      lti::write(handler,"useLines",useLines);
      lti::write(handler,"binThreshold",binThreshold);
      lti::write(handler,"infoMaxEntry",infoMaxEntry);
      lti::write(handler,"infoNumEntries",infoNumEntries);
      lti::write(handler,"infoCells",infoCells);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase3D::parameters::write(handler,false);
# else
    bool (viewerBase3D::parameters::* p_writeMS)(ioHandler&,const bool) const =
      viewerBase3D::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool histogramViewer::parameters::write(ioHandler& handler,
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
  bool histogramViewer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool histogramViewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"cells",cells);
      lti::read(handler,"useBoxes",useBoxes);
      lti::read(handler,"greyEntries",greyEntries);
      lti::read(handler,"useLines",useLines);
      lti::read(handler,"binThreshold",binThreshold);
      lti::read(handler,"infoMaxEntry",infoMaxEntry);
      lti::read(handler,"infoNumEntries",infoNumEntries);
      lti::read(handler,"infoCells",infoCells);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase3D::parameters::read(handler,false);
# else
    bool (viewerBase3D::parameters::* p_readMS)(ioHandler&,const bool) =
      viewerBase3D::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool histogramViewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  };
# endif

  // --------------------------------------------------
  // histogramViewer
  // --------------------------------------------------

  // default constructor
  histogramViewer::histogramViewer(bool createDefaultParameters)
    :  viewerBase3D(false) {

    if (createDefaultParameters) {
      parameters defaultParameters;
      setParameters(defaultParameters);
    }
  };

  // copy constructor
  histogramViewer::histogramViewer(const histogramViewer& other)
    : viewerBase3D(false) {

    copy(other);
  };

  // default constructor
  histogramViewer::histogramViewer(const std::string& title)
    : viewerBase3D(false) {

    parameters defaultParameters;
    defaultParameters.title = title;
    setParameters(defaultParameters);
  }

  // default constructor
  histogramViewer::histogramViewer(const char* title)
    : viewerBase3D(false) {

    parameters defaultParameters;
    defaultParameters.title = title;
    setParameters(defaultParameters);
  }


  // destructor
  histogramViewer::~histogramViewer() {
  }

  // returns the name of this type
  const char* histogramViewer::getTypeName() const {
    return "histogramViewer";
  }

  // copy member
  histogramViewer&
    histogramViewer::copy(const histogramViewer& other) {
    viewerBase3D::copy(other);

    theHistogram.copy(other.theHistogram);
    return (*this);  }

  // clone member
  viewerBase* histogramViewer::clone() const {
    return new histogramViewer(*this);
  }

  // return parameters
  const histogramViewer::parameters&
    histogramViewer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&viewerBase::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The show-methods!
  // -------------------------------------------------------------------

  /**
   * shows a color image.
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const thistogram<double>& data) {
    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        // copy data into the local histogram
        theHistogram.copy(data);

        // transfer data to wnd
        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /**
   * shows a color image.
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const image& data) {
    const parameters& param = getParameters();

    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();


        ivector dims(3),idx(3);
        dims.at(0) = param.cells.x;
        dims.at(1) = param.cells.y;
        dims.at(2) = param.cells.z;
        // extract the histogram of the image
        theHistogram.resize(3,dims);

        vector<image::value_type>::const_iterator it,eit;
        rgbPixel p;
        dims = theHistogram.getLastCell();
        int j;

        for (j=0;j<data.rows();++j) {
          const vector<image::value_type>& vct = data.getRow(j);
          for (it=vct.begin(),eit=vct.end();it != eit;++it) {
            p = (*it);
            idx.at(0) = (p.getRed()*param.cells.x/256);
            idx.at(1) = (p.getGreen()*param.cells.y/256);
            idx.at(2) = (p.getBlue()*param.cells.z/256);
            
            theHistogram.put(idx);
          }
        }

        // transfer data to wnd
        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /**
   * shows a 8-bit channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const channel8& data) {
    const parameters& param = getParameters();

    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        ivector dims(1),idx(1);
        dims.at(0) = param.cells.x;
        // extract the histogram of the image
        theHistogram.resize(1,dims);

        vector<channel8::value_type>::const_iterator it,eit;
        const int dim = param.cells.x;
        int j;
        
        for (j=0;j<data.rows();++j) {
          const vector<channel8::value_type>& vct = data.getRow(j);
          for (it=vct.begin(),eit=vct.end();it != eit;++it) {
            idx.at(0) = ((*it)*dim/256);
            theHistogram.put(idx);
          }
        }

        // transfer data to wnd
        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /*
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const channel& data) {
    return show(static_cast<const matrix<float> >(data));
  }

  /**
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const matrix<float>& data) {
    const parameters& param = getParameters();

    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        // compute a linear mapping from the data value range to 
        // (0 ... param.cells.x-1)
        float theMin,theMax;
        data.getExtremes(theMin,theMax);
        
        // compute epsilon value to avoid later overflow
        // (std::epsilon() is the smallest value that can be added to 0)
        const float eps=std::numeric_limits<float>::epsilon()*(theMax-theMin);
        
        // enhance theMax, to avoid reaching the next highest value
        theMax+=eps;
        
        const float m=float(param.cells.x)/float(theMax-theMin);
        const float b=-m*theMin;
        
        // transfer data to wnd        
        ivector dims(1),idx(1);
        dims.at(0) = param.cells.x;
        // extract the histogram of the image
        theHistogram.resize(1,dims);

        vector<float>::const_iterator it,eit;
        int j;

        for (j=0;j<data.rows();++j) {
          const vector<float>& vct=data.getRow(j);
          for (it=vct.begin(),eit=vct.end();it != eit;++it) {
            // floor of m*x+b
            idx.at(0) = static_cast<int>((*it)*m + b);
            theHistogram.put(idx);
          }
        }

        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /**
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const matrix<double>& data) {
    const parameters& param = getParameters();

    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        // compute a linear mapping from the data value range to 
        // (0 ... param.cells.x-1)
        double theMin,theMax;
        data.getExtremes(theMin,theMax);
        
        // compute epsilon value to avoid later overflow
        // (std::epsilon() is the smallest value that can be added to 0)
        const double eps=std::numeric_limits<double>::epsilon()*(theMax-theMin);
        
        // enhance theMax, to avoid reaching the next highest value
        theMax+=eps;
        
        const double m=double(param.cells.x)/double(theMax-theMin);
        const double b=-m*theMin;
        
        // transfer data to wnd        
        ivector dims(1),idx(1);
        dims.at(0) = param.cells.x;
        // extract the histogram of the image
        theHistogram.resize(1,dims);

        vector<double>::const_iterator it,eit;
        int j;

        for (j=0;j<data.rows();++j) {
          const vector<double>& vct=data.getRow(j);
          for (it=vct.begin(),eit=vct.end();it != eit;++it) {
            // floor of m*x+b
            idx.at(0) = static_cast<int>((*it)*m + b);
            theHistogram.put(idx);
          }
        }

        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /**
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const matrix<int>& data) {
    const parameters& param = getParameters();

    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        // compute a linear mapping from the data value range to 
        // (0 ... param.cells.x-1)
        int theMin,theMax;
        data.getExtremes(theMin,theMax);
        
        // enhance theMax, to avoid reaching the next highest value
        theMax++;
        
        const double m=double(param.cells.x)/double(theMax-theMin);
        const double b=-m*theMin;
        
        // transfer data to wnd        
        ivector dims(1),idx(1);
        dims.at(0) = param.cells.x;
        // extract the histogram of the image
        theHistogram.resize(1,dims);

        vector<int>::const_iterator it,eit;
        int j;

        for (j=0;j<data.rows();++j) {
          const vector<int>& vct=data.getRow(j);
          for (it=vct.begin(),eit=vct.end();it != eit;++it) {
            // floor of m*x+b
            idx.at(0) = static_cast<int>((*it)*m + b);
            theHistogram.put(idx);
          }
        }

        window->putData(theHistogram);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const vector<double>& data) {
    matrix<double> tmp;
    tmp.useExternData(1,data.size(),const_cast<double*>(&data.at(0)));
    return show(tmp);
  };
  
  /*
   * shows a vector of floats
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const vector<float>& data) {
    matrix<float> tmp;
    tmp.useExternData(1,data.size(),const_cast<float*>(&data.at(0)));
    return show(tmp);
  };

  /*
   * shows a vector of integers
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool histogramViewer::show(const vector<int>& data) {
    matrix<int> tmp;
    tmp.useExternData(1,data.size(),const_cast<int*>(&data.at(0)));
    return show(tmp);
  };

  /*
   *  get new instance of main window class
   */
  viewerBase3D::mainWindow* histogramViewer::getNewMainWindow() const {
    return new mainWindow();
  }


  // -------------------------------------------------------------------
  // Main Window
  // -------------------------------------------------------------------

  histogramViewer::mainWindow::mainWindow()
    : viewerBase3D::mainWindow(false) {

    gdk_threads_enter();
    options = new configDialog();
    options->buildDialog();
    gdk_threads_leave();

    param = new parameters();
  };

  histogramViewer::mainWindow::mainWindow(const mainWindow& other)
    : viewerBase3D::mainWindow(false) {

    gdk_threads_enter();
    options = new configDialog();
    options->buildDialog();
    gdk_threads_leave();

    param = new parameters();

    copy(other);
  };

  histogramViewer::mainWindow::~mainWindow() {
  }

  void histogramViewer::mainWindow::putData(const thistogram<double>& hist) {
    theHistogram = &hist;
    histMaximum = theHistogram->maximum();
  };

  bool histogramViewer::mainWindow::validData() {
    return notNull(theHistogram);
  }

  void histogramViewer::mainWindow::indicateDataHide() {
    theHistogram = 0;
  }

  // return parameters
  const histogramViewer::parameters&
    histogramViewer::mainWindow::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(param);

    if (isNull(par)) {
      throw invalidParametersException(getTypeName());
    }

    return *par;
  }


  // ----------------------------------------------------
  //        generate image from histogram
  // ----------------------------------------------------
  void histogramViewer::mainWindow::dataToImage() {

    if (theHistogram->dimensions() == 3) {
      // three dimensional histograms
      his3DToImage();

    } else if (theHistogram->dimensions() == 2) {
      // two dimensional histograms
      his2DToImage();

    } else if (theHistogram->dimensions() == 1) {
      // one dimensional histograms
      his1DToImage();

    } else {
      // don't know how to show this...
      return;
    }

  }

  void histogramViewer::mainWindow::his3DToImage() {

    const parameters& par = getParameters();

    // draw axis (parent class method)
    draw3DAxis(255);

    int r,g,b;

    const int maxR = theHistogram->getLastCell().at(0);
    const int maxG = theHistogram->getLastCell().at(1);
    const int maxB = theHistogram->getLastCell().at(2);
    thistogram<double>::const_iterator it;

    const double thresh=par.binThreshold*histMaximum;

    it = theHistogram->begin();

    for (b=0;b<=maxB;++b) {
      for (g=0;g<=maxG;++g) {
        for (r=0;r<=maxR;++r) {

          if ((*it) > thresh) {
            dpoint3D pos(r*255/maxR,g*255/maxG,b*255/maxB);

            if (par.greyEntries) {
              ubyte c = static_cast<ubyte>((*it)*255/histMaximum);
              drawTool.setColor(rgbPixel(c,c,c));
            } else {
              drawTool.setColor(rgbPixel(static_cast<ubyte>(pos.x),
                                       static_cast<ubyte>(pos.y),
                                       static_cast<ubyte>(pos.z)));
            }

            if (mouseButtonPressed || !par.useBoxes) {
              // show as points
              drawTool.set3D(pos);
            } else  {
              // show as box
              dpoint3D epos((r+1)*255/maxR,(g+1)*255/maxG,(b+1)*255/maxB);
              if (par.useLines) {
                drawTool.box(pos,epos,param->axisColor);
              } else {
                drawTool.box(pos,epos,true);
              }
            }
          }

          ++it;
        }
      }
    }

  };

  void
  histogramViewer::mainWindow::his2DToImage() {
    const parameters& par = getParameters();
    const thistogram<double>& hist = *theHistogram;
    // 1st convert hist to a channel
    channel chnl;
    chnl.resize(hist.cellsInDimension(0),
                hist.cellsInDimension(1),0.0,false,false);
    channel::iterator cit;
    thistogram<double>::const_iterator it,eit;
    float maxVal = 0;
    for (it=hist.begin(),eit=hist.end(),cit=chnl.begin();
         it!=eit;
         ++it,++cit) {
      *cit = static_cast<float>(*it);
      maxVal = max(maxVal,*cit);
    }

    chnl.divide(maxVal);

    double height = max(chnl.columns(),chnl.rows())*6;
    // draw axis:
    if (par.axisColorFixed) {
      // fixed
      drawTool.setColor(par.axisColor);
      drawTool.line3D(0,0,0,chnl.columns()*8,0,0);
      drawTool.line3D(0,0,0,0,chnl.rows()*8,0);
      drawTool.line3D(0,0,0,0,0,height);
    } else {
      // gradient
      int i;
      const int ax = static_cast<int>(0.5+height*8.0/6.0);
      for (i=0;i<ax;++i) {
        drawTool.setColor(rgbPixel(i,0,0));
        drawTool.set3D(i,0,0);
        drawTool.setColor(rgbPixel(0,i,0));
        drawTool.set3D(0,i,0);
        drawTool.setColor(rgbPixel(0,0,i));
        drawTool.set3D(0,0,i);
      }
    }

    drawTool.set3D(chnl,height,
                 point(8,8), // grid size
                 point(1,1), // sample rate
                 !par.useBoxes, // only points
                 par.useBoxes, // useBoxes
                 par.greyEntries, // height color
                 par.useLines, // draw lines
                 par.axisColor, // line color
                 false, // draw contour
                 White);

  }

  void
  histogramViewer::mainWindow::his1DToImage() {
    const parameters& par = getParameters();
    const thistogram<double>& hist = *theHistogram;
    // 1st convert hist to a vector<double>
    dvector tmp;
    tmp.resize(hist.cellsInDimension(0),0,false,false);
    tmp.fill(&hist.at(hist.getFirstCell()));

    // create a viewerFunctor, which can paint this
    viewerFunctor vf;
    viewerFunctor::parameters vfp;
    vfp.useBoxes = par.useBoxes;
    vfp.useLines = par.useLines;
    
    // from viewerBase3D::parameters
    vfp.backgroundColor = par.backgroundColor;
    vfp.lineColor       = par.axisColor;
    
    vf.setParameters(vfp);
    vf.apply(tmp,*dynamic_cast<image*>(&drawTool.getCanvas()),true);
  };

  void histogramViewer::mainWindow::prepareParameters(void){
    parameters* param = dynamic_cast<parameters*>(this->param);

    if (notNull(param)) {
      param->infoMaxEntry   = histMaximum;
      param->infoNumEntries = theHistogram->getNumberOfEntries();
      param->infoCells      = theHistogram->cellsPerDimension();
    }
  };


# ifndef _LTI_MSC_6
  void histogramViewer::mainWindow::mouseMovedHandler(const int& button,
                                                   const point& pos,
                                                   const bool shift,
                                                   const bool ctrl)

# else
  void histogramViewer::mainWindow::mouseMovedHandlerMS(const int& button,
                                                     const point& pos,
                                                     const bool shift,
                                                     const bool ctrl)
# endif
  {

    parameters* p = dynamic_cast<parameters*>(param);

    if (isNull(p)) {
      return;
    }

    if ((button == 3) && !shift && ctrl) {
      // threshold
      double deltaThresh = double(pos.y - lastMousePos.y)/200.0;
      p->binThreshold += deltaThresh;
      if (p->binThreshold < 0) {
        p->binThreshold = 0.0;
      } else if (p->binThreshold >= 1.0) {
        p->binThreshold = 1.0;
      }
    }

    // call parent method to set common param's and redraw()

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    viewerBase3D::mainWindow::mouseMovedHandler(button,pos,shift,ctrl);
# else
    void (viewerBase3D::mainWindow::* p_mmh)(const int&,const point&,
                                             const bool,const bool) =
      viewerBase3D::mainWindow::mouseMovedHandlerMS;
    (this->*p_mmh)(button,pos,shift,ctrl);
# endif
  };

# ifdef _LTI_MSC_6
  void histogramViewer::mainWindow::mouseMovedHandler(const int& button,
                                                   const point& pos,
                                                   const bool shift,
                                                   const bool ctrl) {
    // ...we need this workaround to cope with another really awful MSVC bug.
    mouseMovedHandlerMS(button,pos,shift,ctrl);
  }
# endif


  // -----------------------------------------------------------------------
  // Configuration Dialog
  // -----------------------------------------------------------------------
  histogramViewer::configDialog::configDialog()
    : viewerBase3D::configDialog() {
  };

  histogramViewer::configDialog::~configDialog() {
  }

  void histogramViewer::configDialog::insertPages() {
    appendPage(buildCommonPage());
    appendPage(buildHistoPage());
  }

  histogramViewer::configDialog::pageWidget
  histogramViewer::configDialog::buildHistoPage() {

  pageWidget pw;
  pw.label = "Histogram";

  GtkWidget* vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);

  GtkWidget* hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  GtkWidget* vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, FALSE, 0);

  GtkWidget *labelColor = gtk_frame_new ("Cell Color");
  gtk_widget_ref (labelColor);
  gtk_object_set_data_full (GTK_OBJECT (settings), "labelColor", labelColor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (labelColor);
  gtk_box_pack_start (GTK_BOX (vbox2), labelColor, TRUE, TRUE, 0);

  GtkWidget* vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox4);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox4", vbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (labelColor), vbox4);

  GSList *color_group = NULL;
  cellcolorGrey = gtk_radio_button_new_with_label (color_group, "Grey all values");
  color_group = gtk_radio_button_group (GTK_RADIO_BUTTON (cellcolorGrey));
  gtk_widget_ref (cellcolorGrey);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellcolorGrey", cellcolorGrey,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellcolorGrey);
  gtk_box_pack_start (GTK_BOX (vbox4), cellcolorGrey, TRUE, FALSE, 0);

  cellcolorRGB = gtk_radio_button_new_with_label (color_group, "RGB cell position");
  color_group = gtk_radio_button_group (GTK_RADIO_BUTTON (cellcolorRGB));
  gtk_widget_ref (cellcolorRGB);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellcolorRGB", cellcolorRGB,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellcolorRGB);
  gtk_box_pack_start (GTK_BOX (vbox4), cellcolorRGB, TRUE, FALSE, 0);

  GtkWidget* labelForm = gtk_frame_new ("Cell Form");
  gtk_widget_ref (labelForm);
  gtk_object_set_data_full (GTK_OBJECT (settings), "labelForm", labelForm,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (labelForm);
  gtk_box_pack_start (GTK_BOX (vbox2), labelForm, TRUE, TRUE, 0);

  GtkWidget* vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (labelForm), vbox3);

  GSList *form_group = NULL;
  cellformPoint = gtk_radio_button_new_with_label (form_group, "points");
  form_group = gtk_radio_button_group (GTK_RADIO_BUTTON (cellformPoint));
  gtk_widget_ref (cellformPoint);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellformPoint", cellformPoint,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellformPoint);
  gtk_box_pack_start (GTK_BOX (vbox3), cellformPoint, TRUE, FALSE, 0);

  GtkWidget* hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox2);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox2", hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, TRUE, TRUE, 0);

  cellformBox = gtk_radio_button_new_with_label (form_group, "Boxes (");
  form_group = gtk_radio_button_group (GTK_RADIO_BUTTON (cellformBox));
  gtk_widget_ref (cellformBox);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellformBox", cellformBox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellformBox);
  gtk_box_pack_start (GTK_BOX (hbox2), cellformBox, FALSE, FALSE, 0);

  cellformLines = gtk_check_button_new_with_label ("show cell lines");
  gtk_widget_ref (cellformLines);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellformLines", cellformLines,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellformLines);
  gtk_box_pack_start (GTK_BOX (hbox2), cellformLines, FALSE, FALSE, 0);

  GtkWidget* label1 = gtk_label_new (" )");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox2), label1, FALSE, FALSE, 0);

  GtkWidget* vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (settings), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox5, TRUE, TRUE, 0);

  GtkWidget* labelBins = gtk_frame_new ("Histogram Bins");
  gtk_widget_ref (labelBins);
  gtk_object_set_data_full (GTK_OBJECT (settings), "labelBins", labelBins,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (labelBins);
  gtk_box_pack_start (GTK_BOX (vbox5), labelBins, TRUE, TRUE, 0);

  GtkWidget* table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (table2);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table2", table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (labelBins), table2);

  GtkWidget* label6 = gtk_label_new ("X1,R: ");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table2), label6, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  GtkWidget* label7 = gtk_label_new ("X2,G: ");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  GtkWidget* label8 = gtk_label_new ("X3,B: ");
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table2), label8, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  GtkObject* binr_adj = gtk_adjustment_new (2, 2, 256, 1, 10, 10);
  binr = gtk_spin_button_new (GTK_ADJUSTMENT (binr_adj), 1, 0);
  gtk_widget_ref (binr);
  gtk_object_set_data_full (GTK_OBJECT (settings), "binr", binr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (binr);
  gtk_table_attach (GTK_TABLE (table2), binr, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  GtkObject* bing_adj = gtk_adjustment_new (2, 2, 256, 1, 10, 10);
  bing = gtk_spin_button_new (GTK_ADJUSTMENT (bing_adj), 1, 0);
  gtk_widget_ref (bing);
  gtk_object_set_data_full (GTK_OBJECT (settings), "bing", bing,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bing);
  gtk_table_attach (GTK_TABLE (table2), bing, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  GtkObject* binb_adj = gtk_adjustment_new (2, 2, 256, 1, 10, 10);
  binb = gtk_spin_button_new (GTK_ADJUSTMENT (binb_adj), 1, 0);
  gtk_widget_ref (binb);
  gtk_object_set_data_full (GTK_OBJECT (settings), "binb", binb,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (binb);
  gtk_table_attach (GTK_TABLE (table2), binb, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  GtkWidget* labelThr = gtk_frame_new ("Cell threshold");
  gtk_widget_ref (labelThr);
  gtk_object_set_data_full (GTK_OBJECT (settings), "labelThr", labelThr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (labelThr);
  gtk_box_pack_start (GTK_BOX (vbox5), labelThr, TRUE, TRUE, 0);

  cellThreshold = gtk_entry_new ();
  gtk_widget_ref (cellThreshold);
  gtk_object_set_data_full (GTK_OBJECT (settings), "cellThreshold", cellThreshold,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cellThreshold);
  gtk_container_add (GTK_CONTAINER (labelThr), cellThreshold);
  gtk_entry_set_text (GTK_ENTRY (cellThreshold), "0.0");
  gtk_tooltips_set_tip (tooltips, cellThreshold, "Relative threshold (between 0 and 1) of the maximal entry value [Ctrl + Mouse Right Button + Up/Down]", NULL);

  GtkWidget* labelStatistic = gtk_frame_new ("Actual Statistic");
  gtk_widget_ref (labelStatistic);
  gtk_object_set_data_full (GTK_OBJECT (settings), "labelStatistic", labelStatistic,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (labelStatistic);
  gtk_box_pack_start (GTK_BOX (vbox1), labelStatistic, TRUE, TRUE, 0);

  GtkWidget* table3 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (table3);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table3", table3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (labelStatistic), table3);

  GtkWidget* label9 = gtk_label_new ("Max cell value: ");
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table3), label9, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  GtkWidget* label10 = gtk_label_new ("Number of Entries: ");
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table3), label10, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  GtkWidget* label11 = gtk_label_new ("Bins: ");
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table3), label11, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  maxcellvalue = gtk_entry_new ();
  gtk_widget_ref (maxcellvalue);
  gtk_object_set_data_full (GTK_OBJECT (settings), "maxcellvalue", maxcellvalue,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (maxcellvalue);
  gtk_table_attach (GTK_TABLE (table3), maxcellvalue, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (maxcellvalue), FALSE);

  numofentry = gtk_entry_new ();
  gtk_widget_ref (numofentry);
  gtk_object_set_data_full (GTK_OBJECT (settings), "numofentry", numofentry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (numofentry);
  gtk_table_attach (GTK_TABLE (table3), numofentry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (numofentry), FALSE);

  bins = gtk_entry_new ();
  gtk_widget_ref (bins);
  gtk_object_set_data_full (GTK_OBJECT (settings), "bins", bins,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bins);
  gtk_table_attach (GTK_TABLE (table3), bins, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (bins), FALSE);

  //instead: gtk_container_add (GTK_CONTAINER (xxx), vbox6);
  //put infos about widget in a list
  pw.widget = vbox1;
  return pw;

  };

  // transfer the data in the param object into the gtk widgets axel
# ifndef _LTI_MSC_6
  bool histogramViewer::configDialog::setDialogData()
# else
  bool histogramViewer::configDialog::setDialogDataMS()
#endif
  {

    parameters* param = dynamic_cast<parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    if (!viewerBase3D::configDialog::setDialogData())
# else
    bool (viewerBase3D::configDialog::* p_gdd)(void) =
      viewerBase3D::configDialog::setDialogDataMS;
    if (!(this->*p_gdd)())
# endif
    {
      return false;
    };

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(binr),param->cells.x);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(bing),param->cells.y);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(binb),param->cells.z);

    if (param->useBoxes)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellformBox),TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellformPoint),TRUE);

    if (param->useLines)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellformLines),TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellformLines),FALSE);

    if (param->greyEntries)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellcolorGrey),TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cellcolorRGB),TRUE);

    gtk_entry_set_text(GTK_ENTRY(cellThreshold),
                       toString(param->binThreshold).c_str());

    gtk_entry_set_text(GTK_ENTRY(maxcellvalue),
                       toString(param->infoMaxEntry,0).c_str());

    gtk_entry_set_text(GTK_ENTRY(numofentry),
           toString(param->infoNumEntries,0).c_str());

    char buffer[256];
    ivector& ic = param->infoCells;
    if (ic.size() == 1) {
      sprintf(buffer,"%d",ic.at(0));
    } else if (ic.size() == 2) {
      sprintf(buffer,"%d x %d",ic.at(0),ic.at(1));
    } else if (ic.size() == 3) {
      sprintf(buffer,"%d x %d x %d",ic.at(0),ic.at(1),ic.at(2));
    } else {
      sprintf(buffer,"unsupported");
    }
    gtk_entry_set_text(GTK_ENTRY(bins),buffer);

    return true;
  }

# ifndef _LTI_MSC_6
  bool histogramViewer::configDialog::getDialogData()
# else
  bool histogramViewer::configDialog::getDialogDataMS()
#endif
  {
    parameters* param = dynamic_cast<parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    if (!viewerBase3D::configDialog::getDialogData())
# else
    bool (viewerBase3D::configDialog::* p_gdd)(void) =
      viewerBase3D::configDialog::getDialogDataMS;
    if (!(this->*p_gdd)())
# endif
    {
      return false;
    };

    param->cells.x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(binr));
    param->cells.y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(bing));
    param->cells.z = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(binb));

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cellcolorGrey)))
      param->greyEntries = true;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cellcolorRGB)))
      param->greyEntries = false;

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cellformPoint)))
      param->useBoxes = false;
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cellformBox)))
      param->useBoxes = true;

    param->useLines =
      (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cellformLines)) == TRUE)
      ? true : false;

    param->binThreshold =
      fromString(gtk_entry_get_text(GTK_ENTRY(cellThreshold)));

    return true;
  };

# ifdef _LTI_MSC_6
  bool histogramViewer::configDialog::getDialogData() {
    return getDialogDataMS();
  }

  bool histogramViewer::configDialog::setDialogData() {
    return setDialogDataMS();
  }
# endif

}

#endif
