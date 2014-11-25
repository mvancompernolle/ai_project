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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiViewer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.4.2000
 * revisions ..: $Id: ltiViewer.cpp,v 1.18 2006/09/05 10:43:54 ltilib Exp $
 */

#include "ltiViewer.h"

#ifdef HAVE_GTK
#include "ltiGtkServer.h"
#include "ltiGtkWidget.h"
#endif

#include "ltiLabelAdjacencyMap.h"
#include "ltiSerialStatsExtFunctor.h"
#include "ltiLabelAdjacencyMap.h"
#include "ltiViewerFunctor.h"

#include <cstdio>
#include <limits>

namespace lti {

  // --------------------------------------------------
  // viewer::parameters
  // --------------------------------------------------

  // default constructor
  viewer::parameters::parameters() : viewerBase::parameters() {

    colors.copy(labelAdjacencyMap::parameters::defaultPalette);

    whichPalette = Gray;

    overflowColor = rgbPixel(255,255,128); // yellow
    underflowColor = rgbPixel(128,128,255); // blue
    zoomFactor = int(0);
    showValue = true;
    showHex = false;
    contrast = 1.0f;
    brightness =  0.0f;
    title = "lti::viewer";
    labelAdjacencyMap = false;
    neighborhood8 = false;
    useFewColors = false;
    vectorHeight = 256;
    pixelsPerElement = 1;
    useBoxes = true;
    useLines = false;
    backgroundColor = Black;
    lineColor = White;
    drawHorizontalAxis = true;
    drawMeanValues = false;

    minI = maxI = averageI = 0;
    minRGB = maxRGB = Black;
    averageRGB = diagCovariance = Black;
  }

  // copy constructor
  viewer::parameters::parameters(const parameters& other)
  : viewerBase::parameters() {
    copy(other);
  }

  // destructor
  viewer::parameters::~parameters() {
  }

  // get type name
  const char* viewer::parameters::getTypeName() const {
    return "viewer::parameters";
  }

  // copy member

  viewer::parameters&
  viewer::parameters::copy(const parameters& other) {

    // user parameters
    whichPalette = other.whichPalette;
    colors.copy(other.colors);
    overflowColor.copy(other.overflowColor);
    underflowColor.copy(other.underflowColor);
    zoomFactor = other.zoomFactor;
    showValue = other.showValue;
    showHex = other.showHex;
    contrast = other.contrast;
    brightness = other.brightness;
    labelAdjacencyMap = other.labelAdjacencyMap;
    neighborhood8 = other.neighborhood8;
    useFewColors = other.useFewColors;
    vectorHeight = other.vectorHeight;
    pixelsPerElement = other.pixelsPerElement;
    useBoxes = other.useBoxes;
    useLines = other.useLines;
    backgroundColor = other.backgroundColor;
    lineColor = other.lineColor;
    drawHorizontalAxis = other.drawHorizontalAxis;
    drawMeanValues = other.drawMeanValues;

    // info parameters
    size = other.size;
    minI = other.minI;
    maxI = other.maxI;
    averageI = other.averageI;
    stdDeviation = other.stdDeviation;
    minRGB = other.minRGB;
    maxRGB = other.maxRGB;
    averageRGB = other.averageRGB;
    diagCovariance = other.diagCovariance;

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

    return *this;
  }

  viewer::parameters&
  viewer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* viewer::parameters::clone() const {
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
  bool viewer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool viewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch (whichPalette) {
        case Gray:
          lti::write(handler,"whichPalette",Gray);
          break;
        case Hue:
          lti::write(handler,"whichPalette",Hue);
          break;
        case Red:
          lti::write(handler,"whichPalette",Red);
          break;
        case Green:
          lti::write(handler,"whichPalette",Green);
          break;
        case Blue:
          lti::write(handler,"whichPalette",Blue);
          break;
        case UserDefined:
          lti::write(handler,"whichPalette",UserDefined);
          break;
        default:
          lti::write(handler,"whichPalette",Gray);
          break;
      }

      lti::write(handler,"colors",colors);
      lti::write(handler,"overflowColor",overflowColor);
      lti::write(handler,"underflowColor",underflowColor);
      lti::write(handler,"zoomFactor",zoomFactor);
      lti::write(handler,"showValue",showValue);
      lti::write(handler,"showHex",showHex);
      lti::write(handler,"contrast",contrast);
      lti::write(handler,"brightness",brightness);
      lti::write(handler,"labelAdjacencyMap",labelAdjacencyMap);
      lti::write(handler,"neighborhood8",neighborhood8);
      lti::write(handler,"useFewColors",useFewColors);
      lti::write(handler,"vectorHeight",vectorHeight);
      lti::write(handler,"pixelsPerElement",pixelsPerElement);
      lti::write(handler,"useBoxes",useBoxes);
      lti::write(handler,"useLines",useLines);
      lti::write(handler,"backgroundColor",backgroundColor);
      lti::write(handler,"lineColor",lineColor);
      lti::write(handler,"drawHorizontalAxis",drawHorizontalAxis);
      lti::write(handler,"drawMeanValues",drawMeanValues);

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
  bool viewer::parameters::write(ioHandler& handler,
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
  bool viewer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool viewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"whichPalette",str);
      if (str == "Gray") {
        whichPalette = Gray;
      } else if (str == "Hue") {
        whichPalette = Hue;
      } else if (str == "Red") {
        whichPalette = Red;
      } else if (str == "Green") {
        whichPalette = Green;
      } else if (str == "Blue") {
        whichPalette = Blue;
      } else if (str == "UserDefined") {
        whichPalette = UserDefined;
      } else  {
        whichPalette = Gray;
      }

      lti::read(handler,"colors",colors);
      lti::read(handler,"overflowColor",overflowColor);
      lti::read(handler,"underflowColor",underflowColor);
      lti::read(handler,"zoomFactor",zoomFactor);
      lti::read(handler,"showValue",showValue);
      lti::read(handler,"showHex",showHex);
      lti::read(handler,"contrast",contrast);
      lti::read(handler,"brightness",brightness);
      lti::read(handler,"labelAdjacencyMap",labelAdjacencyMap);
      lti::read(handler,"neighborhood8",neighborhood8);
      lti::read(handler,"useFewColors",useFewColors);
      lti::read(handler,"vectorHeight",vectorHeight);
      lti::read(handler,"pixelsPerElement",pixelsPerElement);
      lti::read(handler,"useBoxes",useBoxes);
      lti::read(handler,"useLines",useLines);
      lti::read(handler,"backgroundColor",backgroundColor);
      lti::read(handler,"lineColor",lineColor);
      lti::read(handler,"drawHorizontalAxis",drawHorizontalAxis);
      lti::read(handler,"drawMeanValues",drawMeanValues);
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
  bool viewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // viewer
  // --------------------------------------------------
#ifdef HAVE_GTK

  // static members
  const int viewer::winTypes = 4;

  // default constructor
  viewer::viewer(const std::string& title, 
                 const float contrast,
                 const float brightness,
                 const int zoom,
                 const bool labelMap)
    : viewerBase(),wnd(-1) {

    master = new viewerFunctor;

    for (int i=0;i<winTypes;++i) {
      wnds[i]=0;
    }

    parameters defaultParam;

    defaultParam.title = title;
    defaultParam.contrast = contrast;
    defaultParam.brightness = brightness;
    defaultParam.zoomFactor = zoom;
    defaultParam.labelAdjacencyMap = labelMap;

    setParameters(defaultParam);

    gtkServer server;
    server.start();
  }

  // default constructor
  viewer::viewer(const parameters& par)
    : viewerBase(),wnd(-1) {
    master = new viewerFunctor;

    for (int i=0;i<winTypes;++i) {
      wnds[i]=0;
    }

    setParameters(par);

    gtkServer server;
    server.start();
  }


  // copy constructor
  viewer::viewer(const viewer& other)
    : viewerBase(),wnd(-1) {

    master = dynamic_cast<viewerFunctor*>(other.master->clone());

    for (int i=0;i<winTypes;++i) {
      wnds[i]=0;
    }

    copy(other);
  }

  // destructor
  viewer::~viewer() {
    delete master;
    master = 0;

    wnd = -1;
    for (int i=0;i<winTypes;++i) {
      delete wnds[i];
      wnds[i]=0;
    }
  }

  // returns the name of this type
  const char* viewer::getTypeName() const {
    return "viewer";
  }

  // copy member
  viewer& viewer::copy(const viewer& other) {
    viewerBase::copy(other);

    wnd = other.wnd;

    for (int i=0;i<winTypes;++i) {
      delete wnds[i];
      wnds[i]=0;
      if (notNull(other.wnds[i])) {
        wnds[i]=dynamic_cast<mainWindow*>(other.wnds[i]->clone());
        wnds[i]->useFunctor(*master);
        if (i==wnd) {
          wnds[i]->drawData();
        }
      }
    }

    return (*this);
  }

  // clone member
  viewerBase* viewer::clone() const {
    return new viewer(*this);
  }

  // return parameters
  const viewer::parameters& viewer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&master->getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  viewer::parameters& viewer::getParameters() {
    parameters* par = dynamic_cast<parameters*>(&master->getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  bool viewer::setParameters(const viewerBase::parameters& p) {
    master->setParameters(p);
    parameters& par = master->getParameters();
    viewerBase::useParameters(par);

    for (int i=0;i<winTypes;++i) {
      if (notNull(wnds[i])) {
        wnds[i]->useFunctor(*master);
        wnds[i]->useParameters(par);
      }
    }

    return true;
  }

  // -------------------------------------------------------------------
  // The show-methods!
  // -------------------------------------------------------------------

  void viewer::ensureWindow(const int newWnd) {
    // wins[0] corresponds to channel8
    // wins[1] corresponds to channel
    // wins[2] corresponds to images
    // wins[3] corresponds to vectors

    mainWindow*& myWnd = wnds[newWnd];
    parameters& par = master->getParameters();

    if (isNull(myWnd)) {
      switch(newWnd) {
        case 0:
          myWnd=new mainWndChannelFixed;
          break;
        case 1:
          myWnd=new mainWndChannelFloat;
          break;
        case 2:
          myWnd=new mainWndImage;
          break;
        case 3:
          myWnd=new mainWndVector;
          break;
      }
      myWnd->useFunctor(*master);
      myWnd->useParameters(par);

      // create first the required window
      myWnd->createWindow();

    } else {
      myWnd->useParameters(par);
      myWnd->useFunctor(*master);
    }

    if ((wnd >= 0) && (wnd < winTypes) &&
        (wnd != newWnd) && notNull(wnds[wnd])) {
      myWnd->setPosition(wnds[wnd]->getPosition(),false);
      myWnd->setSize(wnds[wnd]->getSize(),false);
      wnds[wnd]->hideData();
    }

    wnd = newWnd;
  }

  /*
   * shows a color image.
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewer::show(const image& data) {
    ensureWindow(2);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };


  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewer::show(const matrix<ubyte>& data) {
    ensureWindow(0);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::show(const matrix<int>& data) {
    ensureWindow(0);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewer::show(const matrix<float>& data) {
    ensureWindow(1);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::show(const matrix<double>& data) {
    ensureWindow(1);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewer::show(const vector<double>& data) {
    ensureWindow(3);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::show(const histogram1D& data) {
    // use the vector representation
    const int size = data.getLastCell() - data.getFirstCell() + 1;
    if (size == 0) {
      return true;
    }

    dvector tmp;
    tmp.useExternData(size,const_cast<double*>(&data.at(data.getFirstCell())));
    return show(tmp);
  };

  bool viewer::show(const histogram2D& data) {
    // use the vector representation
    const point size = (data.getLastCell() - data.getFirstCell() + point(1,1));
    if (size.x*size.y == 0) {
      return true;
    }

    dmatrix tmp;
    tmp.useExternData(size.y,size.x,
                      const_cast<double*>(&data.at(data.getFirstCell())));
    return show(tmp);
  };

  bool viewer::show(const vector<float>& data) {
    ensureWindow(3);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::show(const vector<int>& data) {
    ensureWindow(3);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::show(const vector<ubyte>& data) {
    ensureWindow(3);

    wnds[wnd]->setData(data);
    return wnds[wnd]->drawData();
  };

  bool viewer::hide() {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      wnds[wnd]->hideData();
    }

    return true;
  };

  /*
   * set position of the window
   */
  void viewer::setPosition(const point& p) {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      wnds[wnd]->setPosition(p,false);
    }
  }

  /*
   * get position of the window
   */
  point viewer::getPosition() const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->getPosition();
    }
    return point(0,0); // default position
  }

  /*
   * set size of the window
   */
  void viewer::setSize(const point& p) {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      wnds[wnd]->setSize(p,false);
    }
  }

  /*
   * get size of the window
   */
  point viewer::getSize() const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->getSize();
    }
    return point(-1,-1); // invalid size
  }

  bool viewer::lastClickedPosition(point& pos) const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->lastClickedPosition(pos);
    }
    return false; // invalid size
  }

  point viewer::waitButtonPressed(const bool onlyValidPos) const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->waitButtonPressed(onlyValidPos);
    }
    return point(-1,-1); // invalid size
  }

  int viewer::lastKey() const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->lastKey();
    }
    return -1; // invalid key
  }

  int viewer::waitKey() const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->waitKey();
    }
    return -1; // invalid key
  }

  point viewer::waitKeyOrButton(bool& key,bool& button) const {
    if ((wnd>=0) && (wnd<winTypes) && notNull(wnds[wnd])) {
      return wnds[wnd]->waitKeyOrButton(key,button);
    }
    return point(-1,-1); // invalid key
  }
  

  // -----------------------------------------------------------------------
  //  configuration dialog
  // -----------------------------------------------------------------------

  viewer::configDialog::configDialog() : configGTKDialog() {
    master=0;
  }

  void viewer::configDialog::insertFrames() {
    clearAllWidgets();
  }


  configGTKDialog* viewer::configDialog::newInstance() const {
    return new configDialog;
  }


  bool viewer::configDialog::getDialogData() {
    parameters* par = dynamic_cast<parameters*>(param);

    if (par == 0) {
      return false;
    }

    // copy the temporal parameters in the real instance
    par->copy(tmpParam);

    return true;
  }

  bool viewer::configDialog::setDialogData() {
    return setCommonDialogData();
  }

  void viewer::configDialog::useFunctor(viewerFunctor& fct) {
    master=&fct;
  }


  bool viewer::configDialog::setCommonDialogData() {
    parameters* par = dynamic_cast<parameters*>(param);
    if (par == 0) {
      return false;
    }

    tmpParam.copy(*par);

    if (notNull(HueButton)) {
      switch(tmpParam.whichPalette) {
        case viewer::parameters::Hue:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(HueButton),TRUE);
          break;
        case viewer::parameters::Gray:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GrayButton),TRUE);
          break;
        case viewer::parameters::UserDefined:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(UserDefinedButton),
                                       TRUE);
          break;
        case viewer::parameters::Red:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(RedButton),TRUE);
          break;
        case viewer::parameters::Green:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GreenButton),TRUE);
          break;
        case viewer::parameters::Blue:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(BlueButton),TRUE);
          break;
        default:
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GrayButton),TRUE);
      }
    }

    if (notNull(ZoomAdjustment)) {
      gtk_adjustment_set_value(GTK_ADJUSTMENT(ZoomAdjustment),
                               tmpParam.zoomFactor);
      gtk_signal_emit_by_name(GTK_OBJECT(ZoomAdjustment),"value_changed");
    }

    if (notNull(ContrastAdjustment)) {
      gtk_adjustment_set_value(GTK_ADJUSTMENT(ContrastAdjustment),
                               tmpParam.contrast);
      gtk_signal_emit_by_name(GTK_OBJECT(ContrastAdjustment),"value_changed");
    }

    if (notNull(BrightnessAdjustment)) {
      gtk_adjustment_set_value(GTK_ADJUSTMENT(BrightnessAdjustment),
                               tmpParam.brightness);
      gtk_signal_emit_by_name(GTK_OBJECT(BrightnessAdjustment),"value_changed");
    }


    if (notNull(LabelAdjacencyButton)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(LabelAdjacencyButton),
                                   tmpParam.labelAdjacencyMap?TRUE:FALSE);

      gtk_signal_emit_by_name(GTK_OBJECT(LabelAdjacencyButton),"toggled");

      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Neigh8),
                                   tmpParam.neighborhood8?TRUE:FALSE);

      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(MinColorsButton),
                                   tmpParam.useFewColors?TRUE:FALSE);

    }

    if (notNull(hexDisplay)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hexDisplay),
                                   tmpParam.showHex?TRUE:FALSE);
    }

    if (notNull(VectorOptionsFrame)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useBoxes),
                                   tmpParam.useBoxes?TRUE:FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useLines),
                                   tmpParam.useLines?TRUE:FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(horizontalAxis),
                                   tmpParam.drawHorizontalAxis?TRUE:FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(drawMeanValues),
                                   tmpParam.drawMeanValues?TRUE:FALSE);

      gtk_entry_set_text(GTK_ENTRY(vctHeight),
                         toString(tmpParam.vectorHeight,0).c_str());
      
      gtk_entry_set_text(GTK_ENTRY(vctPixPerElem),
                         toString(tmpParam.pixelsPerElement,0).c_str());

    }

    return true;
  }

  void viewer::configDialog::clearAllWidgets() {
    // Statistics Frame
    StatisticsFrame=0;
    StatisticsBox=0;
    StatisticsSubBox=0;
    MinLabel=0;
    MaxLabel=0;
    AverageLabel=0;
    StdDevLabel=0;
    SizeEntry=0;
    MinEntry=0;
    MaxEntry=0;
    AverageEntry=0;
    StdDevEntry=0;
    ScaleMinimumButton=0;
    ScaleMaximumButton=0;
    ScaleMinMaxButton=0;
    Size=0;
    AvrgStdDevScalingBox=0;
    ScaleStdDevButton=0;
    ScalingFactorEntry=0;
    ColorsBox=0;
    UnderflowColorButton=0;
    OverflowColorButton=0;

    // Intensity Control Frame
    IntensityControlFrame=0;
    table1=0;
    ZoomLabel=0;
    ContrastLabel=0;
    BrightnessLabel=0;
    ZoomScale=0;
    ZoomAdjustment=0;
    BrightnessAdjustment=0;
    ContrastAdjustment=0;
    BrightnessEntry=0;
    ContrastEntry=0;
    ZoomEntry=0;
    ContrastScale=0;
    BrightnessScale=0;

    // Palette Control Frame
    PaletteControlFrame=0;
    PaletteOptionsBox=0;
    UsualPalettes=0;
    _1_group=0;
    GrayButton=0;
    RedButton=0;
    GreenButton=0;
    BlueButton=0;
    HueButton=0;
    UserDefinedButton=0;

    // Label Adjacency
    LabelAdjacencyButton=0;
    hbox2=0;
    _2_group=0;
    Neigh4Button=0;
    Neigh8=0;
    MinColorsButton=0;

      // Hexadecimal Display
    hexDisplay=0;

      // Vector Options Frame
    VectorOptionsFrame=0;
    vctSubframe=0;
    vctbox1=0;
    vctSizes=0;
    vctHeightLabel=0;
    vctHeight=0;
    vctPixPerElemLabel=0;
    vctPixPerElem=0;
    vctBools=0;
    useBoxes=0;
    useLines=0;
    horizontalAxis=0;
    drawMeanValues=0;
    vctColors=0;
    backgroundColor=0;
    lineColor=0;
  }

  // Intensity Control Frame
  void viewer::configDialog::buildIntensityControlFrame() {

    IntensityControlFrame = gtk_frame_new (_("Intensity Control"));
    gtk_widget_ref (IntensityControlFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "IntensityControlFrame", IntensityControlFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (IntensityControlFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), IntensityControlFrame, FALSE, FALSE, 0);

    table1 = gtk_table_new (3, 3, FALSE);
    gtk_widget_ref (table1);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "table1", table1,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table1);
    gtk_container_add (GTK_CONTAINER (IntensityControlFrame), table1);

    ZoomLabel = gtk_label_new (_("Zoom"));
    gtk_widget_ref (ZoomLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomLabel", ZoomLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomLabel);
    gtk_table_attach (GTK_TABLE (table1), ZoomLabel, 0, 1, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (ZoomLabel), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (ZoomLabel), 0, 0.5);

    ContrastLabel = gtk_label_new (_("Contrast"));
    gtk_widget_ref (ContrastLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ContrastLabel", ContrastLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ContrastLabel);
    gtk_table_attach (GTK_TABLE (table1), ContrastLabel, 0, 1, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (ContrastLabel), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (ContrastLabel), 0, 0.5);

    BrightnessLabel = gtk_label_new (_("Brightness"));
    gtk_widget_ref (BrightnessLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "BrightnessLabel", BrightnessLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (BrightnessLabel);
    gtk_table_attach (GTK_TABLE (table1), BrightnessLabel, 0, 1, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (BrightnessLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (BrightnessLabel), 0, 0.5);

    ZoomScale = gtk_hscale_new (GTK_ADJUSTMENT (ZoomAdjustment = gtk_adjustment_new (0,-4, 4 + 1, 1, 1, 1)));
    gtk_widget_ref (ZoomScale);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomScale", ZoomScale,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomScale);
    gtk_table_attach (GTK_TABLE (table1), ZoomScale, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (ZoomScale), FALSE);

    BrightnessEntry = gtk_entry_new_with_max_length (10);
    gtk_widget_ref (BrightnessEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "BrightnessEntry", BrightnessEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (BrightnessEntry);
    gtk_table_attach (GTK_TABLE (table1), BrightnessEntry, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_text (GTK_ENTRY (BrightnessEntry), _("0"));

    ContrastEntry = gtk_entry_new_with_max_length (10);
    gtk_widget_ref (ContrastEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ContrastEntry", ContrastEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ContrastEntry);
    gtk_table_attach (GTK_TABLE (table1), ContrastEntry, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_text (GTK_ENTRY (ContrastEntry), _("1"));

    ZoomEntry = gtk_entry_new_with_max_length (10);
    gtk_widget_ref (ZoomEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomEntry", ZoomEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomEntry);
    gtk_table_attach (GTK_TABLE (table1), ZoomEntry, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_text (GTK_ENTRY (ZoomEntry), _("1"));

    ContrastScale = gtk_hscale_new (GTK_ADJUSTMENT (ContrastAdjustment = gtk_adjustment_new (1, -2, 4 + 0.01, 0.01, 0.1, 0.01)));
    gtk_widget_ref (ContrastScale);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ContrastScale", ContrastScale,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ContrastScale);
    gtk_table_attach (GTK_TABLE (table1), ContrastScale, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (ContrastScale), FALSE);

    BrightnessScale = gtk_hscale_new (GTK_ADJUSTMENT (BrightnessAdjustment = gtk_adjustment_new (0, -3, 3 + 0.01, 0.01, 0.1, 0.01)));
    gtk_widget_ref (BrightnessScale);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "BrightnessScale", BrightnessScale,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (BrightnessScale);
    gtk_table_attach (GTK_TABLE (table1), BrightnessScale, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (BrightnessScale), FALSE);

    gtk_signal_connect (GTK_OBJECT (ZoomAdjustment), "value_changed",
                        GTK_SIGNAL_FUNC (on_zoom_value_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (ContrastAdjustment), "value_changed",
                        GTK_SIGNAL_FUNC (on_contrast_value_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (BrightnessAdjustment), "value_changed",
                        GTK_SIGNAL_FUNC (on_brightness_value_changed),
                        this);

    gtk_signal_connect (GTK_OBJECT (ZoomEntry), "activate",
                        GTK_SIGNAL_FUNC (on_ZoomEntry_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (BrightnessEntry), "activate",
                        GTK_SIGNAL_FUNC (on_BrightnessEntry_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (ContrastEntry), "activate",
                        GTK_SIGNAL_FUNC (on_ContrastEntry_changed),
                        this);

  }

  // Intensity Control Frame
  void viewer::configDialog::buildZoomFrame() {

    IntensityControlFrame = gtk_frame_new (_("Zoom Control"));

    gtk_widget_ref (IntensityControlFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "IntensityControlFrame", IntensityControlFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (IntensityControlFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), IntensityControlFrame, FALSE, FALSE, 0);

    table1 = gtk_table_new (3, 3, FALSE);
    gtk_widget_ref (table1);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "table1", table1,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table1);
    gtk_container_add (GTK_CONTAINER (IntensityControlFrame), table1);

    ZoomLabel = gtk_label_new (_("Zoom"));
    gtk_widget_ref (ZoomLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomLabel", ZoomLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomLabel);
    gtk_table_attach (GTK_TABLE (table1), ZoomLabel, 0, 1, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (ZoomLabel), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (ZoomLabel), 0, 0.5);

    ZoomScale = gtk_hscale_new (GTK_ADJUSTMENT (ZoomAdjustment = gtk_adjustment_new (0, -4, 4 + 1, 1, 1, 1)));
    gtk_widget_ref (ZoomScale);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomScale", ZoomScale,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomScale);
    gtk_table_attach (GTK_TABLE (table1), ZoomScale, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (ZoomScale), FALSE);

    ZoomEntry = gtk_entry_new_with_max_length (10);
    gtk_widget_ref (ZoomEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ZoomEntry", ZoomEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ZoomEntry);
    gtk_table_attach (GTK_TABLE (table1), ZoomEntry, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_text (GTK_ENTRY (ZoomEntry), _("1"));

    gtk_signal_connect (GTK_OBJECT (ZoomAdjustment), "value_changed",
                        GTK_SIGNAL_FUNC (on_zoom_value_changed),
                        this);

    gtk_signal_connect (GTK_OBJECT (ZoomEntry), "activate",
                        GTK_SIGNAL_FUNC (on_ZoomEntry_changed),
                        this);
  }


  // Statistics Frame
  void viewer::configDialog::buildStatisticsFrame() {

    StatisticsFrame = gtk_frame_new (_("Image Statistics"));
    gtk_widget_ref (StatisticsFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "StatisticsFrame", StatisticsFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (StatisticsFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), StatisticsFrame, TRUE, TRUE, 0);

    StatisticsBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (StatisticsBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "StatisticsBox", StatisticsBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (StatisticsBox);
    gtk_container_add (GTK_CONTAINER (StatisticsFrame), StatisticsBox);

    StatisticsSubBox = gtk_table_new (5, 3, FALSE);
    gtk_widget_ref (StatisticsSubBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "StatisticsSubBox", StatisticsSubBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (StatisticsSubBox);
    gtk_box_pack_start (GTK_BOX (StatisticsBox), StatisticsSubBox, TRUE, TRUE, 0);

    MinLabel = gtk_label_new (_("MinimumValue"));
    gtk_widget_ref (MinLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "MinLabel", MinLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (MinLabel);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), MinLabel, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (MinLabel), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (MinLabel), 4, 0);

    MaxLabel = gtk_label_new (_("Maximum Value"));
    gtk_widget_ref (MaxLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "MaxLabel", MaxLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (MaxLabel);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), MaxLabel, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (MaxLabel), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (MaxLabel), 4, 0);

    AverageLabel = gtk_label_new (_("Average Value"));
    gtk_widget_ref (AverageLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "AverageLabel", AverageLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (AverageLabel);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), AverageLabel, 0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (AverageLabel), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (AverageLabel), 4, 0);

    StdDevLabel = gtk_label_new (_("Standard Deviation"));
    gtk_widget_ref (StdDevLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "StdDevLabel", StdDevLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (StdDevLabel);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), StdDevLabel, 0, 1, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (StdDevLabel), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (StdDevLabel), 4, 0);

    SizeEntry = gtk_entry_new_with_max_length (32);
    gtk_widget_ref (SizeEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "SizeEntry", SizeEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (SizeEntry);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), SizeEntry, 1, 3, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_editable (GTK_ENTRY (SizeEntry), FALSE);
    gtk_entry_set_text (GTK_ENTRY (SizeEntry), _("256x256"));

    MinEntry = gtk_entry_new_with_max_length (32);
    gtk_widget_ref (MinEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "MinEntry", MinEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (MinEntry);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), MinEntry, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_editable (GTK_ENTRY (MinEntry), FALSE);
    gtk_entry_set_text (GTK_ENTRY (MinEntry), _("0"));

    MaxEntry = gtk_entry_new_with_max_length (32);
    gtk_widget_ref (MaxEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "MaxEntry", MaxEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (MaxEntry);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), MaxEntry, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_editable (GTK_ENTRY (MaxEntry), FALSE);
    gtk_entry_set_text (GTK_ENTRY (MaxEntry), _("0"));

    AverageEntry = gtk_entry_new_with_max_length (32);
    gtk_widget_ref (AverageEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "AverageEntry", AverageEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (AverageEntry);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), AverageEntry, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_editable (GTK_ENTRY (AverageEntry), FALSE);
    gtk_entry_set_text (GTK_ENTRY (AverageEntry), _("0"));

    StdDevEntry = gtk_entry_new_with_max_length (32);
    gtk_widget_ref (StdDevEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "StdDevEntry", StdDevEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (StdDevEntry);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), StdDevEntry, 1, 2, 4, 5,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_editable (GTK_ENTRY (StdDevEntry), FALSE);
    gtk_entry_set_text (GTK_ENTRY (StdDevEntry), _("0"));

    ScaleMinimumButton = gtk_button_new_with_label (_("Scale Minimum"));
    gtk_widget_ref (ScaleMinimumButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ScaleMinimumButton", ScaleMinimumButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ScaleMinimumButton);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), ScaleMinimumButton, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 4, 4);

    ScaleMaximumButton = gtk_button_new_with_label (_("Scale Maximum"));
    gtk_widget_ref (ScaleMaximumButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ScaleMaximumButton", ScaleMaximumButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ScaleMaximumButton);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), ScaleMaximumButton, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 4, 4);

    ScaleMinMaxButton = gtk_button_new_with_label (_("Scale Min-Max"));
    gtk_widget_ref (ScaleMinMaxButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ScaleMinMaxButton", ScaleMinMaxButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ScaleMinMaxButton);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), ScaleMinMaxButton, 2, 3, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 4, 4);

    Size = gtk_label_new (_("Image Size"));
    gtk_widget_ref (Size);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "Size", Size,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (Size);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), Size, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_label_set_justify (GTK_LABEL (Size), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (Size), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (Size), 4, 0);

    AvrgStdDevScalingBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (AvrgStdDevScalingBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "AvrgStdDevScalingBox", AvrgStdDevScalingBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (AvrgStdDevScalingBox);
    gtk_table_attach (GTK_TABLE (StatisticsSubBox), AvrgStdDevScalingBox, 2, 3, 4, 5,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    ScaleStdDevButton = gtk_button_new_with_label (_("Avrg. + StdDev*"));
    gtk_widget_ref (ScaleStdDevButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ScaleStdDevButton", ScaleStdDevButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ScaleStdDevButton);
    gtk_box_pack_start (GTK_BOX (AvrgStdDevScalingBox), ScaleStdDevButton, TRUE, TRUE, 4);

    ScalingFactorEntry = gtk_entry_new_with_max_length (10);
    gtk_widget_ref (ScalingFactorEntry);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ScalingFactorEntry", ScalingFactorEntry,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ScalingFactorEntry);
    gtk_box_pack_start (GTK_BOX (AvrgStdDevScalingBox), ScalingFactorEntry, FALSE, TRUE, 4);
    gtk_widget_set_usize (ScalingFactorEntry, 48, -2);
    gtk_entry_set_text (GTK_ENTRY (ScalingFactorEntry), _("3"));

    ColorsBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (ColorsBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ColorsBox", ColorsBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ColorsBox);
    gtk_box_pack_start (GTK_BOX (StatisticsBox), ColorsBox, TRUE, TRUE, 0);

    // -----------------------------
    // underflow and overflow colors
    
    UnderflowColorLabel = gtk_label_new (_("Underflow Color"));
    gtk_widget_ref (UnderflowColorLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget),
                              "UnderflowColorLabel",UnderflowColorLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (UnderflowColorLabel);

    UnderflowColorButton = gtk_button_new();
    gtk_widget_ref (UnderflowColorButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), 
                              "UnderflowColorButton", UnderflowColorButton,
                              (GtkDestroyNotify) gtk_widget_unref);

    gtk_container_add (GTK_CONTAINER (UnderflowColorButton),
                       UnderflowColorLabel);

    gtk_widget_show (UnderflowColorButton);
    gtk_box_pack_start (GTK_BOX (ColorsBox), UnderflowColorButton, 
                        TRUE, TRUE, 4);

    OverflowColorLabel = gtk_label_new (_("Overflow Color"));
    gtk_widget_ref (OverflowColorLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget),
                              "OverflowColorLabel",OverflowColorLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (OverflowColorLabel);

    OverflowColorButton = gtk_button_new();
    gtk_widget_ref (OverflowColorButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget),
                              "OverflowColorButton", OverflowColorButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_container_add (GTK_CONTAINER (OverflowColorButton),
                       OverflowColorLabel);
    gtk_widget_show (OverflowColorButton);
    gtk_box_pack_start (GTK_BOX (ColorsBox), OverflowColorButton, TRUE, TRUE, 4);

    gtk_signal_connect (GTK_OBJECT (ScaleMinimumButton), "clicked",
                        GTK_SIGNAL_FUNC (on_ScaleMinimumButton_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (ScaleMaximumButton), "clicked",
                        GTK_SIGNAL_FUNC (on_ScaleMaximumButton_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (ScaleMinMaxButton), "clicked",
                        GTK_SIGNAL_FUNC (on_ScaleMinMaxButton_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (ScaleStdDevButton), "clicked",
                        GTK_SIGNAL_FUNC (on_ScaleStdDevButton_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (UnderflowColorButton), "clicked",
                        GTK_SIGNAL_FUNC (on_UnderflowColorButton_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (OverflowColorButton), "clicked",
                        GTK_SIGNAL_FUNC (on_OverflowColorButton_clicked),
                        this);
  }

  // Palette Frame
  void viewer::configDialog::buildPaletteFrame() {
    _1_group = 0;
    _2_group = 0;
    PaletteControlFrame = gtk_frame_new (_("Palette Control"));
    gtk_widget_ref (PaletteControlFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "PaletteControlFrame", PaletteControlFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (PaletteControlFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), PaletteControlFrame, TRUE, TRUE, 0);

    PaletteOptionsBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (PaletteOptionsBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "PaletteOptionsBox", PaletteOptionsBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (PaletteOptionsBox);
    gtk_container_add (GTK_CONTAINER (PaletteControlFrame), PaletteOptionsBox);

    UsualPalettes = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (UsualPalettes);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "UsualPalettes", UsualPalettes,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (UsualPalettes);
    gtk_box_pack_start (GTK_BOX (PaletteOptionsBox), UsualPalettes, TRUE, TRUE, 0);

    GrayButton = gtk_radio_button_new_with_label (_1_group, _("Gray"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (GrayButton));
    gtk_widget_ref (GrayButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "GrayButton", GrayButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (GrayButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), GrayButton, TRUE, TRUE, 0);

    RedButton = gtk_radio_button_new_with_label (_1_group, _("Red"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (RedButton));
    gtk_widget_ref (RedButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "RedButton", RedButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (RedButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), RedButton, TRUE, TRUE, 0);

    GreenButton = gtk_radio_button_new_with_label (_1_group, _("Green"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (GreenButton));
    gtk_widget_ref (GreenButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "GreenButton", GreenButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (GreenButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), GreenButton, TRUE, TRUE, 0);

    BlueButton = gtk_radio_button_new_with_label (_1_group, _("Blue"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (BlueButton));
    gtk_widget_ref (BlueButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "BlueButton", BlueButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (BlueButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), BlueButton, TRUE, TRUE, 0);

    HueButton = gtk_radio_button_new_with_label (_1_group, _("Hue"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (HueButton));
    gtk_widget_ref (HueButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "HueButton", HueButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (HueButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), HueButton, TRUE, TRUE, 0);

    UserDefinedButton = gtk_radio_button_new_with_label (_1_group, _("User Defined"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (UserDefinedButton));
    gtk_widget_ref (UserDefinedButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "UserDefinedButton", UserDefinedButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (UserDefinedButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), UserDefinedButton, TRUE, TRUE, 0);

    // Signals
    gtk_signal_connect (GTK_OBJECT (GrayButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (RedButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (GreenButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (BlueButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (HueButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (UserDefinedButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);

  }

  // Palette options with label frame capabilities
  void viewer::configDialog::buildPaletteAndLabelFrame() {
    _1_group = 0;
    _2_group = 0;

    PaletteControlFrame = gtk_frame_new (_("Palette Control"));
    gtk_widget_ref (PaletteControlFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "PaletteControlFrame", PaletteControlFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (PaletteControlFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), PaletteControlFrame, TRUE, TRUE, 0);

    PaletteOptionsBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (PaletteOptionsBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "PaletteOptionsBox", PaletteOptionsBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (PaletteOptionsBox);
    gtk_container_add (GTK_CONTAINER (PaletteControlFrame), PaletteOptionsBox);

    UsualPalettes = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (UsualPalettes);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "UsualPalettes", UsualPalettes,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (UsualPalettes);
    gtk_box_pack_start (GTK_BOX (PaletteOptionsBox), UsualPalettes, TRUE, TRUE, 0);

    GrayButton = gtk_radio_button_new_with_label (_1_group, _("Gray"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (GrayButton));
    gtk_widget_ref (GrayButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "GrayButton", GrayButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (GrayButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), GrayButton, TRUE, TRUE, 0);

    RedButton = gtk_radio_button_new_with_label (_1_group, _("Red"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (RedButton));
    gtk_widget_ref (RedButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "RedButton", RedButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (RedButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), RedButton, TRUE, TRUE, 0);

    GreenButton = gtk_radio_button_new_with_label (_1_group, _("Green"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (GreenButton));
    gtk_widget_ref (GreenButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "GreenButton", GreenButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (GreenButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), GreenButton, TRUE, TRUE, 0);

    BlueButton = gtk_radio_button_new_with_label (_1_group, _("Blue"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (BlueButton));
    gtk_widget_ref (BlueButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "BlueButton", BlueButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (BlueButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), BlueButton, TRUE, TRUE, 0);

    HueButton = gtk_radio_button_new_with_label (_1_group, _("Hue"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (HueButton));
    gtk_widget_ref (HueButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "HueButton", HueButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (HueButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), HueButton, TRUE, TRUE, 0);

    UserDefinedButton = gtk_radio_button_new_with_label (_1_group, _("User Defined"));
    _1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (UserDefinedButton));
    gtk_widget_ref (UserDefinedButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "UserDefinedButton", UserDefinedButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (UserDefinedButton);
    gtk_box_pack_start (GTK_BOX (UsualPalettes), UserDefinedButton, TRUE, TRUE, 0);

    LabelAdjacencyButton = gtk_check_button_new_with_label (_("Label Adjacency Analysis"));
    gtk_widget_ref (LabelAdjacencyButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "LabelAdjacencyButton", LabelAdjacencyButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (LabelAdjacencyButton);
    gtk_box_pack_start (GTK_BOX (PaletteOptionsBox), LabelAdjacencyButton, FALSE, FALSE, 0);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (hbox2);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "hbox2", hbox2,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (PaletteOptionsBox), hbox2, TRUE, TRUE, 0);

    Neigh4Button = gtk_radio_button_new_with_label (_2_group, _("4-Neighborhood"));
    _2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (Neigh4Button));
    gtk_widget_ref (Neigh4Button);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "Neigh4Button", Neigh4Button,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (Neigh4Button);
    gtk_box_pack_start (GTK_BOX (hbox2), Neigh4Button, TRUE, TRUE, 0);

    Neigh8 = gtk_radio_button_new_with_label (_2_group, _("8-Neighborhood"));
    _2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (Neigh8));
    gtk_widget_ref (Neigh8);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "Neigh8", Neigh8,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (Neigh8);
    gtk_box_pack_start (GTK_BOX (hbox2), Neigh8, TRUE, TRUE, 0);

    MinColorsButton = gtk_check_button_new_with_label (_("Use minimum number of colors"));
    gtk_widget_ref (MinColorsButton);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "MinColorsButton", MinColorsButton,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (MinColorsButton);
    gtk_box_pack_start (GTK_BOX (hbox2), MinColorsButton, TRUE, TRUE, 0);


    // signals
    gtk_signal_connect (GTK_OBJECT (GrayButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (RedButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (GreenButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (BlueButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (HueButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (UserDefinedButton), "toggled",
                        GTK_SIGNAL_FUNC (on_palette_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (LabelAdjacencyButton), "toggled",
                        GTK_SIGNAL_FUNC (on_LabelAdjacencyButton_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (Neigh4Button), "toggled",
                        GTK_SIGNAL_FUNC (on_neighborhood_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (Neigh8), "toggled",
                        GTK_SIGNAL_FUNC (on_neighborhood_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (MinColorsButton), "toggled",
                        GTK_SIGNAL_FUNC (on_MinColorsButton_toggled),
                        this);

  }

  // build hexadecimal display
  void viewer::configDialog::buildHexadecimalDisplayFrame() {
    hexDisplay = gtk_check_button_new_with_label (_("Show pixel information in hexadecimal"));
    gtk_widget_ref (hexDisplay);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "hexDisplay",
                              hexDisplay,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (hexDisplay);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), hexDisplay, FALSE, FALSE, 0);

    gtk_signal_connect (GTK_OBJECT (hexDisplay), "toggled",
                        GTK_SIGNAL_FUNC (on_hexDisplay_toggled),
                        this);

  }

  void viewer::configDialog::buildVectorFrame() {

    VectorOptionsFrame = gtk_frame_new (_("Vector Options"));
    gtk_widget_ref (VectorOptionsFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "VectorOptionsFrame", VectorOptionsFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (VectorOptionsFrame);
    gtk_box_pack_start (GTK_BOX (ConfigFrame), VectorOptionsFrame, TRUE, TRUE, 0);
    
    vctSubframe = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (vctSubframe);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctSubframe", vctSubframe,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctSubframe);
    gtk_container_add (GTK_CONTAINER (VectorOptionsFrame), vctSubframe);
    
    vctbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (vctbox1);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctbox1", vctbox1,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctbox1);
    gtk_box_pack_start (GTK_BOX (vctSubframe), vctbox1, TRUE, TRUE, 0);
    
    vctSizes = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (vctSizes);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctSizes", vctSizes,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctSizes);
    gtk_box_pack_start (GTK_BOX (vctbox1), vctSizes, TRUE, TRUE, 0);
    
    vctHeightLabel = gtk_label_new (_("Vector height:"));
    gtk_widget_ref (vctHeightLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctHeightLabel", vctHeightLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctHeightLabel);
    gtk_box_pack_start (GTK_BOX (vctSizes), vctHeightLabel, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (vctHeightLabel), 4, 0);
    
    vctHeight = gtk_entry_new ();
    gtk_widget_ref (vctHeight);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctHeight", vctHeight,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctHeight);
    gtk_box_pack_start (GTK_BOX (vctSizes), vctHeight, TRUE, TRUE, 0);
    gtk_tooltips_set_tip (tooltips, vctHeight, _("Height of the image used to display the vector"), NULL);
    gtk_entry_set_text (GTK_ENTRY (vctHeight), _("256"));
    
    vctPixPerElemLabel = gtk_label_new (_("Pixels per element:"));
    gtk_widget_ref (vctPixPerElemLabel);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctPixPerElemLabel", vctPixPerElemLabel,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctPixPerElemLabel);
    gtk_box_pack_start (GTK_BOX (vctSizes), vctPixPerElemLabel, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (vctPixPerElemLabel), 4, 0);
    
    vctPixPerElem = gtk_entry_new ();
    gtk_widget_ref (vctPixPerElem);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctPixPerElem", vctPixPerElem,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctPixPerElem);
    gtk_box_pack_start (GTK_BOX (vctSizes), vctPixPerElem, TRUE, TRUE, 0);
    gtk_tooltips_set_tip (tooltips, vctPixPerElem, _("Number of pixels (width) representing one vector element or histogram bin entry."), NULL);
    gtk_entry_set_text (GTK_ENTRY (vctPixPerElem), _("1"));
    
    vctBools = gtk_hbox_new (TRUE, 0);
    gtk_widget_ref (vctBools);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctBools", vctBools,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctBools);
    gtk_box_pack_start (GTK_BOX (vctbox1), vctBools, TRUE, TRUE, 0);
    
    useBoxes = gtk_check_button_new_with_label (_("Use boxes"));
    gtk_widget_ref (useBoxes);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "useBoxes", useBoxes,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (useBoxes);
    gtk_box_pack_start (GTK_BOX (vctBools), useBoxes, FALSE, FALSE, 0);
    
    useLines = gtk_check_button_new_with_label (_("Use lines"));
    gtk_widget_ref (useLines);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "useLines", useLines,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (useLines);
    gtk_box_pack_start (GTK_BOX (vctBools), useLines, FALSE, FALSE, 0);
    
    horizontalAxis = gtk_check_button_new_with_label (_("Horizontal axis"));
    gtk_widget_ref (horizontalAxis);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "horizontalAxis", horizontalAxis,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (horizontalAxis);
    gtk_box_pack_start (GTK_BOX (vctBools), horizontalAxis, FALSE, FALSE, 0);
    
    drawMeanValues = gtk_check_button_new_with_label (_("Draw mean values"));
    gtk_widget_ref (drawMeanValues);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "drawMeanValues", drawMeanValues,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (drawMeanValues);
    gtk_box_pack_start (GTK_BOX (vctBools), drawMeanValues, FALSE, FALSE, 0);
    
    vctColors = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (vctColors);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "vctColors", vctColors,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vctColors);
    gtk_box_pack_start (GTK_BOX (vctSubframe), vctColors, TRUE, TRUE, 0);
    
    backgroundColor = gtk_button_new_with_label (_("Background Color"));
    gtk_widget_ref (backgroundColor);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "backgroundColor", backgroundColor,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (backgroundColor);
    gtk_box_pack_start (GTK_BOX (vctColors), backgroundColor, FALSE, FALSE, 0);
    
    lineColor = gtk_button_new_with_label (_("Line Color"));
    gtk_widget_ref (lineColor);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "lineColor", lineColor,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (lineColor);
    gtk_box_pack_start (GTK_BOX (vctColors), lineColor, FALSE, FALSE, 0);
    
    gtk_signal_connect (GTK_OBJECT (vctHeight), "changed",
                        GTK_SIGNAL_FUNC (on_vctHeight_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (vctPixPerElem), "changed",
                        GTK_SIGNAL_FUNC (on_vctPixPerElem_changed),
                        this);
    gtk_signal_connect (GTK_OBJECT (useBoxes), "toggled",
                        GTK_SIGNAL_FUNC (on_useBoxes_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (useLines), "toggled",
                        GTK_SIGNAL_FUNC (on_useLines_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (horizontalAxis), "toggled",
                        GTK_SIGNAL_FUNC (on_horizontalAxis_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (drawMeanValues), "toggled",
                        GTK_SIGNAL_FUNC (on_drawMeanValues_toggled),
                        this);
    gtk_signal_connect (GTK_OBJECT (backgroundColor), "clicked",
                        GTK_SIGNAL_FUNC (on_backgroundColor_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (lineColor), "clicked",
                        GTK_SIGNAL_FUNC (on_lineColor_clicked),
                        this);

  }

  // -------------------
  // call-back functions
  // -------------------
  void viewer::configDialog::on_zoom_value_changed(GtkAdjustment* widget,
                                                   gpointer value) {
    configDialog* me = (configDialog*)(value);

    me->tmpParam.zoomFactor = iround(widget->value);
    gfloat zoom = float(pow(2.0,int( me->tmpParam.zoomFactor)));

    gtk_entry_set_text(GTK_ENTRY(me->ZoomEntry),
                       me->toString(zoom).c_str());

    gtk_signal_emit_by_name (GTK_OBJECT (widget), "changed");
  }

  void viewer::configDialog::on_contrast_value_changed(GtkAdjustment* widget,
                                                       gpointer value) {
    configDialog* me = (configDialog*)(value);

    gfloat val = widget->value;
    me->tmpParam.contrast = val;

    gtk_entry_set_text(GTK_ENTRY(me->ContrastEntry),
                       me->toString(val).c_str());

    gtk_signal_emit_by_name (GTK_OBJECT (widget), "changed");
  }

  void viewer::configDialog::on_brightness_value_changed(GtkAdjustment* widget,
                                                       gpointer value) {
    configDialog* me = (configDialog*)(value);

    gfloat val = widget->value;
    me->tmpParam.brightness = val;

    gtk_entry_set_text(GTK_ENTRY(me->BrightnessEntry),
                       me->toString(val).c_str());

    gtk_signal_emit_by_name (GTK_OBJECT (widget), "changed");

  }

  void viewer::configDialog::on_BrightnessEntry_changed(GtkEditable *editable,
                                                        gpointer user_data){
    configDialog* me = (configDialog*)(user_data);

    gfloat value;
    value = me->fromString(gtk_entry_get_text(GTK_ENTRY(me->BrightnessEntry)));
    me->tmpParam.brightness = value;

    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->BrightnessAdjustment),value);

    gtk_signal_emit_by_name(GTK_OBJECT (me->BrightnessAdjustment),
                            "value_changed");

  }

  void viewer::configDialog::on_ContrastEntry_changed(GtkEditable *editable,
                                                      gpointer user_data){
    configDialog* me = (configDialog*)(user_data);

    gfloat value;
    value = me->fromString(gtk_entry_get_text(GTK_ENTRY(me->ContrastEntry)));
    me->tmpParam.contrast = value;

    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ContrastAdjustment),value);

    gtk_signal_emit_by_name(GTK_OBJECT (me->ContrastAdjustment),
                            "value_changed");

   }

  void viewer::configDialog::on_ZoomEntry_changed(GtkEditable *editable,
                                                  gpointer user_data){
    configDialog* me = (configDialog*)(user_data);

    gfloat value;
    value = me->fromString(gtk_entry_get_text(GTK_ENTRY(me->ZoomEntry)));
    int exponent = iround(log(value)/log(2.0));
    me->tmpParam.zoomFactor = exponent;

    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ZoomAdjustment),exponent);

    gtk_signal_emit_by_name(GTK_OBJECT (me->ZoomAdjustment),
                            "value_changed");
  }

  void
  viewer::configDialog::on_ScaleMinimumButton_clicked(GtkButton *button,
                                                      gpointer user_data) {

    configDialog* me = (configDialog*)(user_data);
    float contr,bright;
    me->minScaling(contr,bright);
    me->tmpParam.brightness = bright;
    me->tmpParam.contrast = contr;
    gfloat value = contr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ContrastAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->ContrastEntry),
                       me->toString(value).c_str());

    value = bright;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->BrightnessAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->BrightnessEntry),
                       me->toString(value).c_str());


    gtk_signal_emit_by_name(GTK_OBJECT (me->ContrastAdjustment),
                            "value_changed");
    gtk_signal_emit_by_name(GTK_OBJECT (me->BrightnessAdjustment),
                            "value_changed");
  }

  void
  viewer::configDialog::on_ScaleMaximumButton_clicked(GtkButton *button,
                                                      gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    float contr,bright;
    me->maxScaling(contr,bright);
    me->tmpParam.brightness = bright;
    me->tmpParam.contrast = contr;
    gfloat value = contr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ContrastAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->ContrastEntry),
                       me->toString(value).c_str());

    value = bright;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->BrightnessAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->BrightnessEntry),
                       me->toString(value).c_str());


    gtk_signal_emit_by_name(GTK_OBJECT (me->ContrastAdjustment),
                            "value_changed");
    gtk_signal_emit_by_name(GTK_OBJECT (me->BrightnessAdjustment),
                            "value_changed");
  }

  void
  viewer::configDialog::on_ScaleMinMaxButton_clicked(GtkButton *button,
                                                     gpointer user_data) {

    configDialog* me = (configDialog*)(user_data);
    float contr(2),bright(0);
    me->tmpParam.brightness = bright;
    me->tmpParam.contrast = contr;
    gfloat value = contr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ContrastAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->ContrastEntry),
                       me->toString(value).c_str());

    value = bright;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->BrightnessAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->BrightnessEntry),
                       me->toString(value).c_str());
  }

  void
  viewer::configDialog::on_ScaleStdDevButton_clicked(GtkButton *button,
                                                     gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    float contr,bright;
    me->deltaScaling(contr,bright);
    me->tmpParam.brightness = bright;
    me->tmpParam.contrast = contr;

    gfloat value = contr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->ContrastAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->ContrastEntry),
                       me->toString(value).c_str());

    value = bright;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(me->BrightnessAdjustment),value);
    gtk_entry_set_text(GTK_ENTRY(me->BrightnessEntry),
                       me->toString(value).c_str());

    gtk_signal_emit_by_name(GTK_OBJECT (me->ContrastAdjustment),
                            "value_changed");
    gtk_signal_emit_by_name(GTK_OBJECT (me->BrightnessAdjustment),
                            "value_changed");
  }

  void
  viewer::configDialog::on_UnderflowColorButton_clicked(GtkButton *button,
                                                        gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    me->colorSelector->useColor(me->tmpParam.underflowColor);
    me->colorSelector->show();
  }

  void
  viewer::configDialog::on_OverflowColorButton_clicked(GtkButton *button,
                                                       gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    me->colorSelector->useColor(me->tmpParam.overflowColor);
    me->colorSelector->show();
  }

  void viewer::configDialog::on_palette_toggled(GtkToggleButton *togglebutton,
                                     gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    if (GTK_TOGGLE_BUTTON (togglebutton)->active) {
      GtkWidget* button = (GtkWidget*)(togglebutton);
      if (button == me->UserDefinedButton) {
        me->tmpParam.whichPalette = viewer::parameters::UserDefined;
      } else if (button == me->GrayButton) {
        me->tmpParam.whichPalette = viewer::parameters::Gray;
      } else if (button == me->RedButton) {
        me->tmpParam.whichPalette = viewer::parameters::Red;
      } else if (button == me->GreenButton) {
        me->tmpParam.whichPalette = viewer::parameters::Green;
      } else if (button == me->BlueButton) {
        me->tmpParam.whichPalette = viewer::parameters::Blue;
      } else if (button == me->HueButton) {
        me->tmpParam.whichPalette = viewer::parameters::Hue;
      } else {
        me->tmpParam.whichPalette = viewer::parameters::Gray;
      }
    }
  }

  void viewer::configDialog::on_LabelAdjacencyButton_toggled(GtkToggleButton *togglebutton,
                                                  gpointer user_data){

    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.labelAdjacencyMap =
      (GTK_TOGGLE_BUTTON (togglebutton)->active);

    GtkStateType state,nostate;
    gboolean sensitivity,nosense;

    if (me->tmpParam.labelAdjacencyMap) {
      state = GTK_STATE_NORMAL;
      sensitivity = TRUE;
      nostate = GTK_STATE_INSENSITIVE;
      nosense = FALSE;
    } else {
      state = GTK_STATE_INSENSITIVE;
      sensitivity = FALSE;
      nostate = GTK_STATE_NORMAL;
      nosense = TRUE;
    }

    // label options
    gtk_widget_set_state(GTK_WIDGET(me->Neigh4Button),state);
    gtk_widget_set_sensitive(GTK_WIDGET(me->Neigh4Button),sensitivity);

    gtk_widget_set_state(GTK_WIDGET(me->Neigh8),state);
    gtk_widget_set_sensitive(GTK_WIDGET(me->Neigh8),sensitivity);

    gtk_widget_set_state(GTK_WIDGET(me->MinColorsButton),state);
    gtk_widget_set_sensitive(GTK_WIDGET(me->MinColorsButton),sensitivity);

    // contrast / brightness
    gtk_widget_set_state(GTK_WIDGET(me->ContrastScale),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ContrastScale),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->BrightnessScale),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->BrightnessScale),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->ContrastEntry),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ContrastEntry),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->BrightnessEntry),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->BrightnessEntry),nosense);


    // palette control
    gtk_widget_set_state(GTK_WIDGET(me->GrayButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->GrayButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->RedButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->RedButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->GreenButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->GreenButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->BlueButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->BlueButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->HueButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->HueButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->UserDefinedButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->UserDefinedButton),nosense);

    // scale buttons
    gtk_widget_set_state(GTK_WIDGET(me->ScaleMinimumButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ScaleMinimumButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->ScaleMaximumButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ScaleMaximumButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->ScaleMinMaxButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ScaleMinMaxButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->ScaleStdDevButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ScaleStdDevButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->ScalingFactorEntry),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->ScalingFactorEntry),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->UnderflowColorButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->UnderflowColorButton),nosense);

    gtk_widget_set_state(GTK_WIDGET(me->OverflowColorButton),nostate);
    gtk_widget_set_sensitive(GTK_WIDGET(me->OverflowColorButton),nosense);
  }

  void viewer::configDialog::on_neighborhood_toggled(GtkToggleButton *widget,
                                                     gpointer user_data){
    configDialog* me = (configDialog*)(user_data);
    if (GTK_TOGGLE_BUTTON (widget)->active) {
      if (((GtkWidget*)widget) == me->Neigh8) {
        me->tmpParam.neighborhood8 = true;
      } else {
        me->tmpParam.neighborhood8 = false;
      }
    }
  }

  void viewer::configDialog::on_MinColorsButton_toggled(GtkToggleButton *wid,
                                                        gpointer user_data){

    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.useFewColors = (GTK_TOGGLE_BUTTON (wid)->active);
  }

  void viewer::configDialog::on_hexDisplay_toggled(GtkToggleButton *wid,
                                                   gpointer user_data){

    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.showHex = (GTK_TOGGLE_BUTTON (wid)->active);
  }

  void viewer::configDialog::on_vctHeight_changed(GtkEditable *editable,
                                                  gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);

    gfloat value,tvalue;
    value = me->fromString(gtk_entry_get_text(GTK_ENTRY(me->vctHeight)));
    tvalue = min(2048.0f,max(1.0f,value));
    me->tmpParam.vectorHeight = static_cast<int>(tvalue);
  }
  
  void viewer::configDialog::on_vctPixPerElem_changed(GtkEditable *editable,
                                                      gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);

    gfloat value,tvalue;
    value = me->fromString(gtk_entry_get_text(GTK_ENTRY(me->vctPixPerElem)));
    tvalue = min(256.0f,max(1.0f,value));
    me->tmpParam.pixelsPerElement = static_cast<int>(tvalue);
  }
  
  void viewer::configDialog::on_useBoxes_toggled(GtkToggleButton *togglebutton,
                                                 gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.useBoxes = (GTK_TOGGLE_BUTTON (me->useBoxes)->active);
  }
  
  void viewer::configDialog::on_useLines_toggled(GtkToggleButton *togglebutton,
                                                 gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.useLines = (GTK_TOGGLE_BUTTON (me->useLines)->active);
  }
  
  void viewer::configDialog::on_horizontalAxis_toggled(GtkToggleButton *togglebutton,
                                                       gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.drawHorizontalAxis= (GTK_TOGGLE_BUTTON(me->horizontalAxis)->active);
  }
  
  void viewer::configDialog::on_drawMeanValues_toggled(GtkToggleButton *togglebutton,
                                                       gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->tmpParam.drawMeanValues = (GTK_TOGGLE_BUTTON (me->drawMeanValues)->active);
  }
  
  void viewer::configDialog::on_backgroundColor_clicked(GtkButton *button,
                                                        gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->colorSelector->useColor(me->tmpParam.backgroundColor);
    me->colorSelector->show();

  }
  
  void viewer::configDialog::on_lineColor_clicked(GtkButton *button,
                                                  gpointer user_data) {
    configDialog* me = (configDialog*)(user_data);
    me->colorSelector->useColor(me->tmpParam.lineColor);
    me->colorSelector->show();
  }



  void viewer::configDialog::minScaling(float& contr, float& bright) {
    // get and normalize the values from the parameters
    const viewer::parameters* par = dynamic_cast<viewer::parameters*>(param);

    if (isNull(par)) {
      return;
    }

    master->computeContrastBrightness(0,par->maxI,norm,contr,bright);

  }

  void viewer::configDialog::maxScaling(float& contr, float& bright) {
    // get and normalize the values from the parameters
    const viewer::parameters* par = dynamic_cast<viewer::parameters*>(param);

    if (isNull(par)) {
      return;
    }

    master->computeContrastBrightness(par->minI,norm,norm,contr,bright);
  }

  void viewer::configDialog::deltaScaling(float& contr, float& bright) {
    const viewer::parameters* par = dynamic_cast<viewer::parameters*>(param);

    if (isNull(par)) {
      return;
    }

    float f=fromString(gtk_entry_get_text(GTK_ENTRY(ScalingFactorEntry)));
    const float p1 = par->averageI - f*par->stdDeviation;
    const float p2 = par->averageI + f*par->stdDeviation;
    float m,b;
    m = norm/(p2-p1);
    b = norm - m*p2;

    master->computeContrastBrightness(m*par->minI + b,
                                      m*par->maxI + b,
                                      norm,
                                      contr,bright);
  }

  // ---------------------------------------------------------
  // Configuration Options for Fixed-Point-Arithmetic Channels
  // ---------------------------------------------------------

  viewer::configChannelFixed::configChannelFixed()
    : configDialog() {
       norm = 255;
  }

  /*
   * Inserts all pages in the main settings dialog.
   *
   */
  void viewer::configChannelFixed::insertFrames() {
    clearAllWidgets();
    buildIntensityControlFrame();
    buildStatisticsFrame();
    buildPaletteAndLabelFrame();
    buildHexadecimalDisplayFrame();
  }

  configGTKDialog* viewer::configChannelFixed::newInstance() const {
    return new configChannelFixed;
  }

  bool viewer::configChannelFixed::setDialogData() {
    if (!setCommonDialogData()) {
      return false;
    }

    // set informational text boxes

    // set the size
    gtk_entry_set_text(GTK_ENTRY(SizeEntry),
                       toString(tmpParam.size).c_str());

    // set statistics

    gtk_entry_set_text(GTK_ENTRY(MinEntry),
                       toString(tmpParam.minI).c_str());

    gtk_entry_set_text(GTK_ENTRY(MaxEntry),
                       toString(tmpParam.maxI).c_str());

    gtk_entry_set_text(GTK_ENTRY(AverageEntry),
                       toString(tmpParam.averageI).c_str());

    gtk_entry_set_text(GTK_ENTRY(StdDevEntry),
                       toString(tmpParam.stdDeviation).c_str());

    return true;
  }

  // ---------------------------------------------------------
  // Configuration Options for Floating-Point-Arithmetic Channels
  // ---------------------------------------------------------
  viewer::configChannelFloat::configChannelFloat()
    : configDialog() {
    norm = 1;
  }

  configGTKDialog* viewer::configChannelFloat::newInstance() const {
    return new configChannelFloat;
  }

  /*
   * Inserts all pages in the main settings dialog.
   *
   */
  void viewer::configChannelFloat::insertFrames() {
    clearAllWidgets();
    buildIntensityControlFrame();
    buildStatisticsFrame();
    buildPaletteFrame();
    buildHexadecimalDisplayFrame();
  }

  bool viewer::configChannelFloat::setDialogData() {
    if (!setCommonDialogData()) {
      return false;
    }

    // set informational text boxes

    // set the size
    gtk_entry_set_text(GTK_ENTRY(SizeEntry),
                       toString(tmpParam.size).c_str());

    // set statistics

    gtk_entry_set_text(GTK_ENTRY(MinEntry),
                       toString(tmpParam.minI,8).c_str());

    gtk_entry_set_text(GTK_ENTRY(MaxEntry),
                       toString(tmpParam.maxI,8).c_str());

    gtk_entry_set_text(GTK_ENTRY(AverageEntry),
                       toString(tmpParam.averageI,8).c_str());

    gtk_entry_set_text(GTK_ENTRY(StdDevEntry),
                       toString(tmpParam.stdDeviation,8).c_str());

    return true;
  }

  // ---------------------------------------------------------
  // Configuration Options for RGB Images
  // ---------------------------------------------------------

  viewer::configImage::configImage()
    : configDialog() {
    norm = 255;
  }

  configGTKDialog* viewer::configImage::newInstance() const {
    return new configImage;
  }

  /*
   * Inserts all pages in the main settings dialog.
   *
   */
  void viewer::configImage::insertFrames() {
    clearAllWidgets();
    buildIntensityControlFrame();
    buildStatisticsFrame();
    buildHexadecimalDisplayFrame();
  }

  bool viewer::configImage::setDialogData() {
    if (!setCommonDialogData()) {
      return false;
    }

    // set informational text boxes

    // set the size
    gtk_entry_set_text(GTK_ENTRY(SizeEntry),
                       toString(tmpParam.size).c_str());

    // set statistics
    gtk_entry_set_text(GTK_ENTRY(MinEntry),
                       toString(tmpParam.minRGB).c_str());

    gtk_entry_set_text(GTK_ENTRY(MaxEntry),
                       toString(tmpParam.maxRGB).c_str());

    gtk_entry_set_text(GTK_ENTRY(AverageEntry),
                       toString(tmpParam.averageRGB).c_str());

    gtk_entry_set_text(GTK_ENTRY(StdDevEntry),
                       toString(tmpParam.diagCovariance).c_str());

    return true;
  }

  // ---------------------------------------------------------
  // Configuration Options for Vector Visualization
  // ---------------------------------------------------------
  viewer::configVector::configVector()
    : configDialog() {
    norm = 1;
  }

  configGTKDialog* viewer::configVector::newInstance() const {
    return new configVector;
  }

  /*
   * Inserts all pages in the main settings dialog.
   *
   */
  void viewer::configVector::insertFrames() {
    clearAllWidgets();
    buildZoomFrame();
    buildStatisticsFrame();
    buildVectorFrame();
    buildHexadecimalDisplayFrame();

    // deactivate unnecessary widgets
    static const GtkStateType nostate = GTK_STATE_INSENSITIVE;
    static const gboolean     nosense = FALSE;

    gtk_widget_set_state(GTK_WIDGET(ScaleMinimumButton),nostate);
    gtk_widget_set_state(GTK_WIDGET(ScaleMaximumButton),nostate);
    gtk_widget_set_state(GTK_WIDGET(ScaleMinMaxButton),nostate);
    gtk_widget_set_state(GTK_WIDGET(ScaleStdDevButton),nostate);
    gtk_widget_set_state(GTK_WIDGET(ScalingFactorEntry),nostate);

    gtk_widget_set_sensitive(GTK_WIDGET(ScaleMaximumButton),nosense);
    gtk_widget_set_sensitive(GTK_WIDGET(ScaleMinimumButton),nosense);
    gtk_widget_set_sensitive(GTK_WIDGET(ScaleMinMaxButton),nosense);
    gtk_widget_set_sensitive(GTK_WIDGET(ScaleStdDevButton),nosense);
    gtk_widget_set_sensitive(GTK_WIDGET(ScalingFactorEntry),nosense);

    gtk_label_set_text (GTK_LABEL (Size), "Vector Size");
    gtk_label_set_text (GTK_LABEL (UnderflowColorLabel), "Axis Color");
    gtk_label_set_text (GTK_LABEL (OverflowColorLabel), "Mean Values Color");

  }

  bool viewer::configVector::setDialogData() {
    if (!setCommonDialogData()) {
      return false;
    }

    // set informational text boxes

    // set the size
    gtk_entry_set_text(GTK_ENTRY(SizeEntry),
                       toString(tmpParam.size.y,0).c_str());

    // set statistics

    gtk_entry_set_text(GTK_ENTRY(MinEntry),
                       toString(tmpParam.minI).c_str());

    gtk_entry_set_text(GTK_ENTRY(MaxEntry),
                       toString(tmpParam.maxI).c_str());

    gtk_entry_set_text(GTK_ENTRY(AverageEntry),
                       toString(tmpParam.averageI).c_str());

    gtk_entry_set_text(GTK_ENTRY(StdDevEntry),
                       toString(tmpParam.stdDeviation).c_str());

    return true;
  }

  // --------------------------------------------------------------------------
  //             Main Windows
  // --------------------------------------------------------------------------

  viewer::mainWindow::mainWindow()
    : mainGTKWindow(false),dataType(Invalid),theData(0),
      master(0),norm(1),newDataFlag(false),drawRequestSem(1),
      lastPosition(0,0),validLastPosition(false),
      waitInteraction(0),waitingForClick(false),
      lastKeyCode(-1),waitingForKey(false) {
  }

  viewer::mainWindow::mainWindow(const viewer::mainWindow& other)
    : mainGTKWindow(other),drawRequestSem(1),
      lastPosition(0,0),validLastPosition(false),
      waitInteraction(0),waitingForClick(false),
      lastKeyCode(-1),waitingForKey(false) {
    dataType = other.dataType;
    theData = other.theData->clone();
    master=0;
    norm = other.norm;
    newDataFlag = true;
  }

  viewer::mainWindow::~mainWindow() {
    dataLock.lock();
    delete theData;
    theData = 0;
    dataType = Invalid;
    dataLock.unlock();

    // in case someone is still waiting for...
    waitingForClick=false;
    waitingForKey=false;
    waitInteraction.post(); // once for click
    waitInteraction.post(); // once for key
  }

  void viewer::mainWindow::setData(const matrix<ubyte>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = MatrixUByte;
    newDataFlag = true;
    norm = 255;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const matrix<int>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = MatrixInt;
    newDataFlag = true;
    norm = 255;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const matrix<float>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = MatrixFloat;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const matrix<double>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = MatrixDouble;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const matrix<rgbPixel>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = Image;
    newDataFlag = true;
    norm = 255;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const vector<ubyte>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = VectorUByte;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const vector<int>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = VectorInt;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const vector<float>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = VectorFloat;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  void viewer::mainWindow::setData(const vector<double>& data) {
    dataLock.lock();
    delete theData;
    theData = data.clone();
    dataType = VectorDouble;
    newDataFlag = true;
    norm = 1;
    dataLock.unlock();
  }

  bool viewer::mainWindow::drawData() {
    // wait until the previous draw is ready
    drawRequestSem.wait();
    return mainGTKWindow::drawData();
  }

  void viewer::mainWindow::drawReady() {
    drawRequestSem.post();
  }

  void viewer::mainWindow::useFunctor(viewerFunctor& fct) {
    master=&fct;
  }

  bool viewer::mainWindow::lastClickedPosition(point& pos) const {
    dataLock.lock();
    pos.copy(lastPosition);
    dataLock.unlock();

    return validLastPosition;
  }

  point viewer::mainWindow::waitButtonPressed(const bool onlyValidPos) const {

    point p;
    do {
      dataLock.lock();
      waitingForClick=true;
      dataLock.unlock();

      // block until the user clicks somewhere...
      do {
        waitInteraction.wait();
      } while(waitingForClick);

    } while(!lastClickedPosition(p) && onlyValidPos);
    return p;
  }

  int viewer::mainWindow::lastKey() const {
    dataLock.lock();
    const int p(lastKeyCode);
    dataLock.unlock();

    return p;
  }

  int viewer::mainWindow::waitKey() const {
    dataLock.lock();
    waitingForKey=true;
    dataLock.unlock();

    // block until the user clicks somewhere...
    do {
      waitInteraction.wait();
    } while(waitingForKey);

    return lastKey();
  }

  point viewer::mainWindow::waitKeyOrButton(bool& key,bool& button) const {
    dataLock.lock();
    waitingForClick=true;
    waitingForKey=true;
    dataLock.unlock();

    // block until the user clicks somewhere...
    waitInteraction.wait();

    key = !waitingForKey;
    button = !waitingForClick;

    point p(-1,-1);

    if (key) {
      p.x = p.y = lastKey();
    }

    if (button) {
      lastClickedPosition(p);
    }

    return p;
  }

  bool viewer::mainWindow::validData() {
    dataLock.lock();
    const bool res = notNull(theData);
    dataLock.unlock();

    return res;
  }

  void viewer::mainWindow::indicateDataHide(void) {
    dataLock.lock();
    delete theData;
    theData = 0;
    dataType = Invalid;
    theImage.clear();
    dataLock.unlock();
  }

  void viewer::mainWindow::windowDestructionHandler() {
    dataLock.lock();
 
    if (waitingForKey) {
      lastKeyCode = -1;
      waitingForKey=false;
      waitInteraction.post();  // continue execution on the other thread
    }

    if (waitingForClick) {
      waitingForClick=false;
      waitInteraction.post();  // continue execution on the other thread
    }

    dataLock.unlock();
  }

  void viewer::mainWindow::keyPressedHandler(const bool shift,
                                             const bool ctrl,
                                             const int key) {
    dataLock.lock();
    lastKeyCode = key;

    if (waitingForKey) {
      waitingForKey=false;
      waitInteraction.post();  // continue execution on the other thread
    }
    dataLock.unlock();
  }

  void viewer::mainWindow::keyReleasedHandler(const bool shift,
                                              const bool ctrl,
                                              const int key) {
    dataLock.lock();
    if (!waitingForKey) {
      lastKeyCode = -1;
    }
    dataLock.unlock();
  }

  void viewer::mainWindow::mouseMovedHandler(const int& button,
                                             const point& pos,
                                             const bool shift,
                                             const bool ctrl) {
    if (button>0) {
      updateStatusBar(pos);

      dataLock.lock();
      if (waitingForClick) {
        waitingForClick=false;
        waitInteraction.post();  // continue execution on the other thread
      }
      dataLock.unlock();
    }
  }

  void viewer::mainWindow::mouseButtonHandler(const int& button,
                                              const point& pos,
                                              const bool shift,
                                              const bool ctrl,
                                              const bool pressed) {

    // check default behavious for buttons first
    mainGTKWindow::mouseButtonHandler(button,pos,shift,ctrl,pressed);

    // and after that our special behaviour:
    updateStatusBar(pos);

    if (pressed && (button != 3)) {
      dataLock.lock();
      if (waitingForClick) {
        waitingForClick=false;
        waitInteraction.post();  // continue execution on the other thread
      }
      dataLock.unlock();
    }
  }



  // --------------------------------------------------------------------------
  //             Main Windows for Channel8 and similar
  // --------------------------------------------------------------------------

  /*
   * Constructor
   */
  viewer::mainWndChannelFixed::mainWndChannelFixed()
    : mainWindow(),chnl8(0),chnl32(0) {
    gdk_threads_enter();
    options = new configChannelFixed();
    options->buildDialog();
    gdk_threads_leave();
  }

  /*
   * Copy constructor
   */
  viewer::mainWndChannelFixed::mainWndChannelFixed(const mainWndChannelFixed& other)
    : mainWindow(other),chnl8(0),chnl32(0)  {

    dataLock.lock();
    chnl8 = dynamic_cast<const matrix<ubyte>*>(theData);
    chnl32 = dynamic_cast<const matrix<int>*>(theData);
    dataLock.unlock();
  }

  /*
   * Destructor
   */
  viewer::mainWndChannelFixed::~mainWndChannelFixed() {
  }

  /*
   * clone method
   */
  mainGTKWindow* viewer::mainWndChannelFixed::clone() const {
    return new mainWndChannelFixed(*this);
  }

  /*
   * This method is the one where your data should be drawn.
   * You just need to draw on the "theImage" attribute image.
   */
  void viewer::mainWndChannelFixed::dataToImage(image& img) {
    dataLock.lock();

    if (isNull(theData)) {
      chnl8 =0;
      chnl32=0;
      img.clear();
      dataLock.unlock();
      return;
    }

    // try first the chnl8, the most usual one
    chnl8 = dynamic_cast<const matrix<ubyte>*>(theData);
    if (notNull(chnl8)) {
      master->apply(*chnl8,img,newDataFlag);
    } else {

      // bad luck! not a channel 8!  Maybe a chnl32?

      chnl32 = dynamic_cast<const matrix<int>*>(theData);

      if (notNull(chnl32)) {
        master->apply(*chnl32,img,newDataFlag);
      }

    }
    newDataFlag = false;
    dataLock.unlock();
  }


  /*
   * Prepare the parameters before the configuration dialog is started.
   *
   * This member gets some information of the displayed data into
   * the informational attributes of the parameters instance.
   * It is called just before opening the settings dialog, and is
   * the proper place to update the mentioned informational data.
   */
  void viewer::mainWndChannelFixed::prepareParameters() {
    // just use the ones in the master painter!  They are all right!
    configDialog* dlg = dynamic_cast<configDialog*>(options);
    if (notNull(dlg)) {
      dlg->useFunctor(*master);
    }
  }

  /*
   * called when the status bar information needs to be updated
   */
  void viewer::mainWndChannelFixed::updateStatusBar(const point& pos) {
    parameters* par = dynamic_cast<parameters*>(param);
    if (notNull(par)) {
      // lets assume, that if the image is already displayed, then the
      // corresponding aliases chnl8 and chnl32 are also all right!
      char strbuffer[256];
      double s = pow(2.0,par->zoomFactor);
      point ip(static_cast<int>(pos.x/s),static_cast<int>(pos.y/s));

      // remember this position
      dataLock.lock();

      lastPosition = ip;

      int val=0;
      bool undef = true;

      if ((ip.x>=0) && (ip.y>=0)) {
        if ((dataType == MatrixUByte) && (notNull(chnl8))) {
          if ((ip.x < chnl8->columns()) &&
              (ip.y < chnl8->rows())) {
            val = chnl8->at(ip);
            undef=false;
          }
        } else if ((dataType == MatrixInt) && (notNull(chnl32))) {
          if ((ip.x < chnl32->columns()) &&
              (ip.y < chnl32->rows())) {
            val = chnl32->at(ip);
            undef=false;
          }
        }
      }

      validLastPosition = !undef;

      if (par->showValue) {
        if (undef) {
          sprintf(strbuffer,"  at(%d,%d) = undefined",ip.x,ip.y);
        } else {
          if (par->whichPalette == viewer::parameters::Hue) {
            if (par->showHex) {
              sprintf(strbuffer,"  at(%d,%d) =%02X (%f deg)",ip.x,ip.y,
                      val,360.0f*val/255);
            } else {
              sprintf(strbuffer,"  at(%d,%d) =%d (%f deg)",ip.x,ip.y,
                      val,360.0f*val/255);
            }
          } else {
            if (par->showHex) {
              sprintf(strbuffer,"  at(%d,%d) =%02X",ip.x,ip.y,val);
            } else {
              sprintf(strbuffer,"  at(%d,%d) =%d",ip.x,ip.y,val);
            }
          }
        }

        setStatusBar(strbuffer);
      }

      dataLock.unlock();
    }
  }

  // --------------------------------------------------------------------------
  //             Main Windows for channels and similar
  // --------------------------------------------------------------------------

  /*
   * Constructor
   */
  viewer::mainWndChannelFloat::mainWndChannelFloat()
    : mainWindow(),chnl(0),dchnl(0) {
    gdk_threads_enter();
    options = new configChannelFloat();
    options->buildDialog();
    gdk_threads_leave();
  }

  /*
   * Copy constructor
   */
  viewer::mainWndChannelFloat::mainWndChannelFloat(const mainWndChannelFloat& other)
    : mainWindow(other),chnl(0),dchnl(0)  {

    dataLock.lock();
    chnl = dynamic_cast<const matrix<float>*>(theData);
    dchnl = dynamic_cast<const matrix<double>*>(theData);
    dataLock.unlock();

  }

  /*
   * Destructor
   */
  viewer::mainWndChannelFloat::~mainWndChannelFloat() {
  }


  /*
   * clone method
   */
  mainGTKWindow* viewer::mainWndChannelFloat::clone() const {
    return new mainWndChannelFloat(*this);
  }

  /*
   * This method is the one where your data should be drawn.
   * You just need to draw on the "theImage" attribute image.
   */
  void viewer::mainWndChannelFloat::dataToImage(image& img) {

    dataLock.lock();
    if (isNull(theData)) {
      chnl =0;
      dchnl=0;
      img.clear();
      dataLock.unlock();
      return;
    }

    // try first the chnl, the most usual one
    chnl = dynamic_cast<const matrix<float>*>(theData);
    if (notNull(chnl)) {
      master->apply(*chnl,img,newDataFlag);
    } else {

      // bad luck! not a channel!  Maybe a dmatrix?

      dchnl = dynamic_cast<const matrix<double>*>(theData);

      if (notNull(dchnl)) {
        master->apply(*dchnl,img,newDataFlag);
      }

    }
    newDataFlag = false;
    dataLock.unlock();
  }


  /*
   * Prepare the parameters before the configuration dialog is started.
   *
   * This member gets some information of the displayed data into
   * the informational attributes of the parameters instance.
   * It is called just before opening the settings dialog, and is
   * the proper place to update the mentioned informational data.
   */
  void viewer::mainWndChannelFloat::prepareParameters() {
    // just use the ones in the master painter!  They are all right!
    configDialog* dlg = dynamic_cast<configDialog*>(options);
    if (notNull(dlg)) {
      dlg->useFunctor(*master);
    }
  }

  /*
   * called when the status bar information needs to be updated
   */
  void viewer::mainWndChannelFloat::updateStatusBar(const point& pos) {
    parameters* par = dynamic_cast<parameters*>(param);
    if (notNull(par)) {
      // lets assume, that if the image is already displayed, then the
      // corresponding aliases chnl and chnl32 are also all right!
      char strbuffer[256];
      double s = pow(2.0,par->zoomFactor);
      point ip(static_cast<int>(pos.x/s),static_cast<int>(pos.y/s));

      // remember this position
      dataLock.lock();
      lastPosition = ip;

      double val=0;
      bool undef = true;

      if ((ip.x>=0) && (ip.y>=0)) {
        if ((dataType == MatrixFloat) && (notNull(chnl))) {
          if ((ip.x < chnl->columns()) &&
              (ip.y < chnl->rows())) {
            val = chnl->at(ip);
            undef=false;
          }
        } else if ((dataType == MatrixDouble) && (notNull(dchnl))) {
          if ((ip.x < dchnl->columns()) &&
              (ip.y < dchnl->rows())) {
            val = dchnl->at(ip);
            undef=false;
          }
        }
      }

      validLastPosition = !undef;

      if (par->showValue) {
        if (undef) {
          sprintf(strbuffer,"  at(%d,%d) = undefined",ip.x,ip.y);
        } else {
          if (par->whichPalette == viewer::parameters::Hue) {
            sprintf(strbuffer,"  at(%d,%d) =%f (%f deg)",ip.x,ip.y,
                    val,180.0f*val/Pi);
          } else {
            sprintf(strbuffer,"  at(%d,%d) =%f",ip.x,ip.y,val);
          }
        }

        setStatusBar(strbuffer);
      }
      dataLock.unlock();
    }
  }


  // --------------------------------------------------------------------------
  //             Main Windows for images
  // --------------------------------------------------------------------------

  /*
   * Constructor
   */
  viewer::mainWndImage::mainWndImage()
    : mainWindow(),chnl(0) {
    gdk_threads_enter();
    options = new configImage();
    options->buildDialog();
    gdk_threads_leave();
  }

  /*
   * Copy constructor
   */
  viewer::mainWndImage::mainWndImage(const mainWndImage& other)
    : mainWindow(other),chnl(0)  {

    dataLock.lock();
    chnl = dynamic_cast<const image*>(theData);
    dataLock.unlock();

  }

  /*
   * Destructor
   */
  viewer::mainWndImage::~mainWndImage() {
  }



  /*
   * clone method
   */
  mainGTKWindow* viewer::mainWndImage::clone() const {
    return new mainWndImage(*this);
  }

  /*
   * This method is the one where your data should be drawn.
   * You just need to draw on the "theImage" attribute image.
   */
  void viewer::mainWndImage::dataToImage(image& img) {
    dataLock.lock();

    if (isNull(theData)) {
      chnl =0;
      img.clear();
      dataLock.unlock();
      return;
    }

    // try first the chnl, the most usual one
    chnl = dynamic_cast<const image*>(theData);
    if (notNull(chnl)) {
      master->apply(*chnl,img,newDataFlag);
    }
    newDataFlag = false;

    dataLock.unlock();

  }


  /*
   * Prepare the parameters before the configuration dialog is started.
   *
   * This member gets some information of the displayed data into
   * the informational attributes of the parameters instance.
   * It is called just before opening the settings dialog, and is
   * the proper place to update the mentioned informational data.
   */
  void viewer::mainWndImage::prepareParameters() {
    // just use the ones in the master painter!  They are all right!
    configDialog* dlg = dynamic_cast<configDialog*>(options);
    if (notNull(dlg)) {
      dlg->useFunctor(*master);
    }
  }

  /*
   * called when the status bar information needs to be updated
   */
  void viewer::mainWndImage::updateStatusBar(const point& pos) {
    parameters* par = dynamic_cast<parameters*>(param);
    if (notNull(par)) {
      // lets assume, that if the image is already displayed, then the
      // corresponding aliases chnl and chnl32 are also all right!
      char strbuffer[256];
      double s = pow(2.0,par->zoomFactor);
      point ip(static_cast<int>(pos.x/s),static_cast<int>(pos.y/s));

      // remember this position
      dataLock.lock();
      lastPosition = ip;

      rgbPixel val;
      bool undef = true;

      if ((ip.x>=0) && (ip.y>=0)) {
        if ((dataType == Image) && (notNull(chnl))) {
          if ((ip.x < chnl->columns()) &&
              (ip.y < chnl->rows())) {
            val = chnl->at(ip);
            undef=false;
          }
        }
      }

      validLastPosition = !undef;

      if (par->showValue) {
        if (undef) {
          sprintf(strbuffer,"  at(%d,%d) = undefined",ip.x,ip.y);
        } else {
          if (par->showHex) {
            sprintf(strbuffer,"  at(%d,%d) = (%02X,%02X,%02X)",ip.x,ip.y,
                    val.getRed(),val.getGreen(),val.getBlue());
          } else {
            sprintf(strbuffer,"  at(%d,%d) = (%d,%d,%d)",ip.x,ip.y,
                    val.getRed(),val.getGreen(),val.getBlue());
          }
        }
        setStatusBar(strbuffer);
      }
      dataLock.unlock();
    }
  }

  // --------------------------------------------------------------------------
  //             Main Windows for vectors
  // --------------------------------------------------------------------------

  /*
   * Constructor
   */
  viewer::mainWndVector::mainWndVector()
    : mainWindow(),dvct(0),fvct(0),ivct(0),uvct(0) {
    gdk_threads_enter();
    options = new configVector();
    options->buildDialog();
    gdk_threads_leave();
  }

  /*
   * Copy constructor
   */
  viewer::mainWndVector::mainWndVector(const mainWndVector& other)
    : mainWindow(other),dvct(0),fvct(0),ivct(0),uvct(0)  {

    dataLock.lock();
    dvct = dynamic_cast<const vector<double>*>(theData);
    fvct = dynamic_cast<const vector<float>*>(theData);
    ivct = dynamic_cast<const vector<int>*>(theData);
    uvct = dynamic_cast<const vector<ubyte>*>(theData);
    dataLock.unlock();
  }

  /*
   * Destructor
   */
  viewer::mainWndVector::~mainWndVector() {
  }


  /*
   * clone method
   */
  mainGTKWindow* viewer::mainWndVector::clone() const {
    return new mainWndVector(*this);
  }

  /*
   * This method is the one where your data should be drawn.
   * You just need to draw on the "theImage" attribute image.
   */
  void viewer::mainWndVector::dataToImage(image& img) {
    dataLock.lock();

    if (isNull(theData)) {
      dvct=0;
      fvct=0;
      ivct=0;
      uvct=0;
      img.clear();
      dataLock.unlock();
      return;
    }

    // try first the a dvector, the most usual one
    dvct = dynamic_cast<const vector<double>*>(theData);
    if (notNull(dvct)) {
      master->apply(*dvct,img,newDataFlag);
    } else {
      // bad luck! not a dvector!  Maybe a ivector?
      ivct = dynamic_cast<const vector<int>*>(theData);
      if (notNull(ivct)) {
        master->apply(*ivct,img,newDataFlag);
      } else {
        // bad luck! not a ivector!  Maybe a fvector?
        fvct = dynamic_cast<const vector<float>*>(theData);
        if (notNull(fvct)) {
          master->apply(*fvct,img,newDataFlag);
        } else {
          uvct = dynamic_cast<const vector<ubyte>*>(theData);
          if (notNull(uvct)) {
            master->apply(*uvct,img,newDataFlag);
          }
        }
      }
    }

    newDataFlag = false;
    dataLock.unlock();
  }


  /*
   * Prepare the parameters before the configuration dialog is started.
   *
   * This member gets some information of the displayed data into
   * the informational attributes of the parameters instance.
   * It is called just before opening the settings dialog, and is
   * the proper place to update the mentioned informational data.
   */
  void viewer::mainWndVector::prepareParameters() {
    // just use the ones in the master painter!  They are all right!
    configDialog* dlg = dynamic_cast<configDialog*>(options);
    if (notNull(dlg)) {
      dlg->useFunctor(*master);
    }
  }

  /*
   * called when the status bar information needs to be updated
   */
  void viewer::mainWndVector::updateStatusBar(const point& pos) {
    parameters* par = dynamic_cast<parameters*>(param);
    if (notNull(par)) {
      // lets assume, that if the image is already displayed, then the
      // corresponding aliases dvct, fvct, ivct or uvct are also all right!
      char strbuffer[256];
      double s = pow(2.0,par->zoomFactor)*par->pixelsPerElement;
      point ip(static_cast<int>(pos.x/s),pos.y);

      // remember this position
      dataLock.lock();
      lastPosition = ip;

      double val=0.0;
      bool undef = true;

      if (ip.x>=0) {
        if ((dataType == VectorUByte) && (notNull(uvct))) {
          if (ip.x < uvct->size()) {
            val = uvct->at(ip.x);
            undef=false;
          }
        } else if ((dataType == VectorInt) && (notNull(ivct))) {
          if (ip.x < ivct->size()) {
            val = ivct->at(ip.x);
            undef=false;
          }
        } else if ((dataType == VectorFloat) && (notNull(fvct))) {
          if (ip.x < fvct->size()) {
            val = fvct->at(ip.x);
            undef=false;
          }
        } else if ((dataType == VectorDouble) && (notNull(dvct))) {
          if (ip.x < dvct->size()) {
            val = dvct->at(ip.x);
            undef=false;
          }
        }
      }

      validLastPosition = !undef;

      if (par->showValue) {
        if (undef) {
          sprintf(strbuffer,"  at(%d) = undefined",ip.x);
        } else {
          const double m = double(par->maxI-par->minI)/(1.0-par->vectorHeight);
          const double b = par->maxI;
          if (notNull(ivct) || notNull(uvct)) {
            const int y = static_cast<int>(m*pos.y+b);
            if (par->showHex) {
              sprintf(strbuffer,"  at(%d) =%02X   y=%02X",ip.x,iround(val),y);
            } else {
              sprintf(strbuffer,"  at(%d) =%d   y=%d",ip.x,iround(val),y);
            }
          } else {
            const double y =  m*pos.y+b;            
            sprintf(strbuffer,"  at(%d) =%f   y=%f",ip.x,val,y);
          }
        }
        setStatusBar(strbuffer);
      }
      dataLock.unlock();
    }
  }

#else 

  // ----------------------------
  // NO GTK: dummy implementation
  // ----------------------------

  // default constructor
  viewer::viewer(const std::string& title, 
                 const float contrast,
                 const float brightness,
                 const int zoom,
                 const bool labelMap)
    : viewerBase() {

    parameters par;
    par.title = title;
    par.contrast = contrast;
    par.brightness = brightness;
    par.zoomFactor = zoom;
    par.labelAdjacencyMap = labelMap;
    
    setParameters(par);
  }
  
  viewer::viewer(const parameters& par) 
    : viewerBase() {
    setParameters(par);
  }

  // copy constructor
  viewer::viewer(const viewer& other)
    : viewerBase()  {

    copy(other);
  }

  // destructor
  viewer::~viewer() {
  }

  // returns the name of this type
  const char* viewer::getTypeName() const {
    return "viewer";
  }

  // copy member
  viewer& viewer::copy(const viewer& other) {
    viewerBase::copy(other);
    return (*this);
  }

  // clone member
  viewerBase* viewer::clone() const {
    return new viewer(*this);
  }

  // return parameters
  const viewer::parameters& viewer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&viewerBase::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool viewer::setParameters(const viewerBase::parameters& p) {
    viewerBase::setParameters(p);
    return true;
  }

  // -------------------------------------------------------------------
  // The show-methods!
  // -------------------------------------------------------------------

  bool viewer::show(const image& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const matrix<ubyte>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const matrix<int>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const matrix<float>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const matrix<double>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const vector<double>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const vector<float>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const vector<int>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::show(const vector<ubyte>& data) {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  bool viewer::hide() {
    setStatusString("No implementation found. GTK installed?");
    return false;
  };

  void viewer::setPosition(const point& p) {
  }

  point viewer::getPosition() const {
    return point(0,0); // default position
  }

  void viewer::setSize(const point& p) {
  }

  point viewer::getSize() const {
    return point(-1,-1); // invalid size
  }

  bool viewer::lastClickedPosition(point& pos) const {
    return false; // invalid size
  }

  point viewer::waitButtonPressed(const bool onlyValidPos) const {
    return point(-1,-1); // invalid size
  }

  int viewer::lastKey() const {
    return -1; // invalid key
  }

  int viewer::waitKey() const {
    return -1; // invalid key
  }
#endif
}
