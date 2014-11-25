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
 * file .......: ltiGtkWidget.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.05.2000
 * revisions ..: $Id: ltiGtkWidget.cpp,v 1.9 2006/02/07 20:51:00 ltilib Exp $
 */

#include "ltiGtkServer.h"

#ifdef HAVE_GTK

#include "ltiObject.h"
#include "ltiRGBPixel.h"
#include "ltiGtkWidget.h"
#include "ltiImage.h"
#include "ltiBMPFunctor.h"
#include "ltiPNGFunctor.h"
#include "ltiJPEGFunctor.h"
#include "ltiDownsampling.h"
#include "ltiALLFunctor.h"
#include "ltiDraw.h"
#include <cstdio>
#include <cstring>
#include <string>

// debug levels: 1. everything
//               2. more messages but not all
//               3. a few messages
//               4. nothing at all

#undef _LTI_DEBUG
//#define _LTI_DEBUG 4
#include "ltiDebug.h"

namespace lti {
  // -------------------------------------------------------------------
  // Main Window
  // -------------------------------------------------------------------

  // static members
  const point mainGTKWindow::borderWidth(21,36);

  mainGTKWindow::mainGTKWindow(bool setConfigDlg)
    : object(), options(0), syncEnd(0), window(0) {

    init();

    if (setConfigDlg) {
      gdk_threads_enter();
      options = new configGTKDialog();
      options->buildDialog();
      gdk_threads_leave();
    }
    param = 0;

  };

  mainGTKWindow::mainGTKWindow(const mainGTKWindow& other)
    : object(), options(0), syncEnd(0), window(0) {

    init();

    if (notNull(other.options)) {
      gdk_threads_enter();
      options = other.options->newInstance();
      options->buildDialog();
      gdk_threads_leave();
    }

    param = 0;

    copy(other);
  };

  void mainGTKWindow::init() {

    theLock.lock();

    winSize = point(0,0);

    xbuffer = 0;
    xbuffer_rows = 0;
    xbuffer_cols = 0;

    mouseButtonPressed = 0;
    busy=false;
    dataChanged = true;
    timerTag = 0;

    theLock.unlock();
  };

  mainGTKWindow::~mainGTKWindow() {

    if (gtkServer::isAlive()) {
      gdk_threads_enter();
      gtk_timeout_add(1,destroyWndTimeout,(gpointer)this);
      gdk_threads_leave();

      syncEnd.wait(); // wait until widgets destroyed!
    }

    theLock.lock();
    delete[] xbuffer;
    xbuffer = NULL;
    delete options;
    options = NULL;
    param = 0;

    theLock.unlock();
  }

  bool mainGTKWindow::useParameters(viewerBase::parameters& theParam) {
    if (notNull(dynamic_cast<viewerBase::parameters*>(&theParam))) {
      param = &theParam;
      return true;
    }
    param = 0;
    return false;
  };

  void  mainGTKWindow::changeGeometryRequest(const bool request) {
    geometryChangeRq = request;
  }

  // copy member
  mainGTKWindow& mainGTKWindow::copy(const mainGTKWindow& other) {

    useParameters(*other.param);

    if (isNull(window) && notNull(other.window)) {
      createWindow();
    }

    // copy window name and size
    winName = other.winName;
    winSize = other.winSize;

    // erase actual data
    theLock.lock();
    theImage.copy(other.theImage);
    theLock.unlock();

    return *this;
  }

  mainGTKWindow* mainGTKWindow::clone() const {
    return new mainGTKWindow(*this);
  }

  void mainGTKWindow::lock() {
    theLock.lock();
  }

  void mainGTKWindow::unlock() {
    theLock.unlock();
  }

  void mainGTKWindow::createWindow() {
    gdk_threads_enter();

    //createWindowLocal();
    gtk_timeout_add(1,createWindowTimeout,(gpointer)this);

    gdk_threads_leave();
  }

  /*
   * set position of the window
   */
  void mainGTKWindow::setPosition(const point& p,const bool withinGtkThread) {
    winPosition = p;
    geometryChangeRq = true;

    if (notNull(window)) {
      if (!withinGtkThread) {
        gdk_threads_enter();
        gtk_timeout_add(1,show,(gpointer)this);
        gdk_threads_leave();
      }
      gdk_window_move(window->window,p.x,p.y);
    }
  }

  /*
   * get position of the window
   */
  point mainGTKWindow::getPosition() const {
    return winPosition;
  }

  /*
   * set size of the window
   */
  void mainGTKWindow::setSize(const point& p,const bool withinGtkThread) {
    winSize = p;
    geometryChangeRq = true;

    if (notNull(window)) {
      if (!withinGtkThread) {
        gdk_threads_enter();
        gtk_timeout_add(1,show,(gpointer)this);
        gdk_threads_leave();
      }
      gdk_window_resize(window->window,p.x,p.y);
    }
  }

  /*
   * get size of the window
   */
  point mainGTKWindow::getSize() const {
    return winSize; // invalid size
  }

  gint mainGTKWindow::createWindowTimeout(gpointer data) {
    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->createWindowLocal();
    return 0; // destroy timer
  }

  // create new window
  void mainGTKWindow::createWindowLocal() {
    lock();

    /* create a new window */
    window = gtk_dialog_new ();

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
                                   "ltiStatusBar");

    /* Sets the border width of the status bar. */
    gtk_container_set_border_width(
                      GTK_CONTAINER(GTK_DIALOG(window)->action_area),0);


    /* drawing area */
    darea = gtk_drawing_area_new();

    gtk_drawing_area_size(GTK_DRAWING_AREA(darea), 256, 256);
    gtk_widget_set_usize(window, 64, 64);

    /* event box */
    eventBox = gtk_event_box_new();

    gtk_scrolled_window_add_with_viewport(
                      GTK_SCROLLED_WINDOW(scrolledWindow),eventBox);
    gtk_container_add(GTK_CONTAINER (eventBox),darea);
    gtk_widget_set_events(eventBox,  ( GDK_EXPOSURE_MASK        |
                                       GDK_POINTER_MOTION_MASK  |
                                       GDK_BUTTON_MOTION_MASK	|
                                       GDK_BUTTON1_MOTION_MASK	|
                                       GDK_BUTTON2_MOTION_MASK	|
                                       GDK_BUTTON3_MOTION_MASK	|
                                       GDK_BUTTON_PRESS_MASK	|
                                       GDK_BUTTON_RELEASE_MASK	|
                                       GDK_FOCUS_CHANGE_MASK    |
                                       GDK_VISIBILITY_NOTIFY_MASK ) );


    // the signals used here
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                        GTK_SIGNAL_FUNC (delete_event), this);
    gtk_signal_connect (GTK_OBJECT (window), "configure-event",
                        GTK_SIGNAL_FUNC (geometryChanged), this);
    gtk_signal_connect (GTK_OBJECT (darea), "expose-event",
                        GTK_SIGNAL_FUNC (dareaExpose), this);
    gtk_signal_connect (GTK_OBJECT (eventBox), "motion_notify_event",
                        GTK_SIGNAL_FUNC (mouseMovedEvent), this);
    gtk_signal_connect (GTK_OBJECT (eventBox), "button_press_event",
                        GTK_SIGNAL_FUNC (buttonPressedEvent), this);
    gtk_signal_connect (GTK_OBJECT (eventBox), "button_release_event",
                        GTK_SIGNAL_FUNC (buttonPressedEvent), this);
    // end of signals

    gtk_widget_realize (eventBox);

    // possible cursors
    // GDK_X_CURSOR   (X Cross, but relativ thick)
    // GDK_ARROW      (Arrow pointing up + right)
    // GDK_CENTER_PTR (Arrow pointing up!)
    // GDK_CIRCLE     (black circle, the pointer is in its middle)
    // GDK_CROSS      (A + similar cross, the traditional one)
    // GDK_HAND1      (Black right hand pointing up + right)
    // GDK_HAND2      (White right hand pointing up + left)
    // GDK_DOTBOX
    gdk_window_set_cursor(eventBox->window, gdk_cursor_new (GDK_DOTBOX));




    // activate key detection
    gtk_widget_add_events(window,  ( GDK_KEY_PRESS_MASK |
                                     GDK_KEY_RELEASE_MASK ) );

    gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
                        GTK_SIGNAL_FUNC (keyPressedEvent), this);

    gtk_signal_connect (GTK_OBJECT (window), "key_release_event",
                        GTK_SIGNAL_FUNC (keyReleasedEvent), this);

    // force resize and position with user data
    geometryChangeRq = true;

    unlock();
  }

  // destroy window
  void mainGTKWindow::destroyWindow() {

    _lti_debug2("hiding window...")

    gtk_widget_hide_all(window);
    gtk_widget_destroy(GTK_WIDGET(window));
    window = 0;

    _lti_debug2("done\n")

    // report end of destruction
    syncEnd.post();
  }

  // this will be called in the right thread to destroy the actual window!
  gint mainGTKWindow::destroyWndTimeout(gpointer data) {
    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->destroyWindow();

    return 0; // destroy timer!
  }

  gint mainGTKWindow::delete_event(GtkWidget *widget,
                                             GdkEvent  *event,
                                             gpointer   data ) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->windowDestructionHandler();

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

  const image& mainGTKWindow::getDisplayedData() const {
    return theImage;
  }

  void mainGTKWindow::hideData() {
    gdk_threads_enter();
    gtk_timeout_add(1,hide,(gpointer)this);
    gdk_threads_leave();
  }

  bool mainGTKWindow::drawData() {
    gdk_threads_enter();
    const bool res = redraw();
    gdk_threads_leave();
    return res;
  };

  bool mainGTKWindow::redraw() {
    bool res = true;
    theLock.lock();
    if (!busy) {
      busy = true;
      dataChanged = true;
      theLock.unlock();

      timerTag = gtk_timeout_add(1,show,(gpointer)this);
    } else {
      // data dropped (it was busy!)
      res = false;
      theLock.unlock();
    }
    return res;
  }

  void mainGTKWindow::drawReady() {
  }

  // --------------------------------------------------------
  //  show method
  // --------------------------------------------------------

  // this method is called each time you need to update the window
  gint mainGTKWindow::show(gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);

    // update the window title if necessary
    if (notNull(me->param) && (me->winName != me->param->title)) {
      me->winName = me->param->title;
      gtk_window_set_title(GTK_WINDOW(me->window),me->winName.c_str());
    }

    gtk_widget_show_all(me->window);

    static GdkEventExpose event;
    event.area.x = event.area.y = 0;
    dareaExpose (GTK_WIDGET(me->darea),&event,me);

    return 0;
  };

  // ----------------------------------------------------------
  // redraw the window
  // ----------------------------------------------------------

  gboolean mainGTKWindow::dareaExpose (GtkWidget *widget,
                                       GdkEventExpose *event,
                                       gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    return (me->dareaExposeLocal(widget,event));
  }

  gboolean
  mainGTKWindow::dareaExposeLocal(GtkWidget *widget,
                                  GdkEventExpose *event) {

    theLock.lock();

    if (dataChanged && notNull(dynamic_cast<viewerBase::parameters*>(param))) {
      _lti_debug4("dareaExposeLocal -> regenerating x-buffer" << endl)
      // generate in the respective viewer instance the visualization data
      dataToImage(theImage);
      // convert the image into a buffer GTK
      generateXbuffer(theImage,widget,event);
      // "data generated"-flag
      dataChanged = false;
    }
    showXbuffer(widget);

    busy = false;
    theLock.unlock();

    // allow inherited applications to receive the message, that everything
    // is done
    drawReady();

    return TRUE;
  }

  // ----------------------------------------------------
  //        generate the xbuffer from an image
  // ----------------------------------------------------
  void mainGTKWindow::generateXbuffer(const image& img,
                                      GtkWidget *widget,
                                      GdkEventExpose *event) {

    bool sizeChanged = (img.size() != point(xbuffer_cols,xbuffer_rows));

    if (sizeChanged) {
      xbuffer_cols = img.columns();
      xbuffer_rows = img.rows();
    }

    if (isNull(xbuffer) || sizeChanged) {

      // free old data
      delete[] xbuffer;
      xbuffer = 0;

      const int size = xbuffer_cols*xbuffer_rows*3;
      if (size>0) {
        xbuffer = new guchar[size];
      } else {
        return;
      }

    }

    guchar* ptr = xbuffer;

    // fill buffer with the image
    vector<rgbPixel>::const_iterator it,e;
    int py;
    for (py=0;py<img.rows();++py) {
      const vector<rgbPixel>& vct = img.getRow(py);
      for (it = vct.begin(),e=vct.end();it!=e;++it) {
        *ptr++ = static_cast<guchar>((*it).getRed());
        *ptr++ = static_cast<guchar>((*it).getGreen());
        *ptr++ = static_cast<guchar>((*it).getBlue());
      }
    }

    dataChanged = false;

    // maximal valid window size
    point wsize(xbuffer_cols+borderWidth.x+1,xbuffer_rows+borderWidth.y);

    if (sizeChanged && ((winSize.x > wsize.x) || (winSize.y > wsize.y))) {
        // clear unused part of the window!
      gdk_window_clear_area (widget->window,
                             event->area.x, event->area.y,
                             winSize.x, winSize.y);
    }

    if (winSize != wsize) {
      winSize = wsize;
    }

    if (sizeChanged) {
      gtk_drawing_area_size(GTK_DRAWING_AREA(darea),
                            xbuffer_cols,
                            xbuffer_rows);
    }

  }

  /*
   * generate xbuffer from an image
   */
  void mainGTKWindow::showXbuffer(GtkWidget *widget) {
    if (notNull(xbuffer)) {
      gdk_draw_rgb_image (widget->window,
                          widget->style->fg_gc[GTK_STATE_NORMAL],
                          0, 0, xbuffer_cols, xbuffer_rows,
                          GDK_RGB_DITHER_NORMAL, xbuffer,
                          xbuffer_cols*3);

      if (geometryChangeRq) {
        // change position and size
        gdk_window_move(window->window,
                        min(winPosition.x,gdk_screen_width()),
                        min(winPosition.y,gdk_screen_height()));

        // change size
        gdk_window_resize(window->window,
                          min(winSize.x,gdk_screen_width()),
                          min(winSize.y,gdk_screen_height()));

        geometryChangeRq = false;
      }
    }

  }

  /*
   * validData
   */
  bool mainGTKWindow::validData() {
    return true;
  };

  // hide method!
  gint mainGTKWindow::hide(gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    if (notNull(me->options)) {
      me->options->hide();
    }
    gtk_widget_hide_all(me->window);
    me->indicateDataHide();

    return 0; // stop timer
  };

  void mainGTKWindow::indicateDataHide() {
  };


  void mainGTKWindow::setStatusBar(const char* str) {
    gtk_statusbar_pop(GTK_STATUSBAR(statusBar),statusBarContextId);
    gtk_statusbar_push( GTK_STATUSBAR(statusBar),
                        statusBarContextId,
                        str);

  }

  void mainGTKWindow::dataToImage(image& img) {
  };


  void mainGTKWindow::prepareParameters() {
  }

  gint mainGTKWindow::keyPressedEvent(GtkWidget* widget,
                                      GdkEventKey* event,
                                      gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->keyPressedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  gint mainGTKWindow::keyReleasedEvent(GtkWidget* widget,
                                      GdkEventKey* event,
                                      gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->keyReleasedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  void mainGTKWindow::windowDestructionHandler() {
  }

  void mainGTKWindow::keyPressedEventLocal(GtkWidget* widget,
                                            GdkEventKey*  event) {

    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl   = (event->state & GDK_CONTROL_MASK) != 0;

    keyPressedHandler(shift,ctrl,event->keyval);
  };

  void mainGTKWindow::keyReleasedEventLocal(GtkWidget* widget,
                                            GdkEventKey*  event) {

    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl   = (event->state & GDK_CONTROL_MASK) != 0;

    keyReleasedHandler(shift,ctrl,event->keyval);
  };

  void mainGTKWindow::keyPressedHandler(const bool shift,
                                        const bool ctrl,
                                        const int keys) {
  }

  void mainGTKWindow::keyReleasedHandler(const bool shift,
                                        const bool ctrl,
                                        const int keys) {
  }

  gint mainGTKWindow::buttonPressedEvent(GtkWidget* widget,
                                         GdkEventButton* event,
                                         gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->buttonPressedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  void
  mainGTKWindow::buttonPressedEventLocal(GtkWidget* widget,
                                         GdkEventButton*  event) {

    const point pos(iround(event->x),iround(event->y));
    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl   = (event->state & GDK_CONTROL_MASK) != 0;

    lastMousePos = pos;
    mouseButtonHandler(event->button,pos,shift,ctrl,
                       event->type == GDK_BUTTON_PRESS);

    if (event->type == GDK_BUTTON_RELEASE) {
      mouseButtonPressed=0;
    } else {
      mouseButtonPressed = event->button;
    }

  };

  void mainGTKWindow::mouseButtonHandler(const int& button,
                                         const point& pos,
                                         const bool shift,
                                         const bool ctrl,
                                         const bool pressed) {
    if (button==3) {
      // right mouse button
      if (!shift && !ctrl) {
        prepareParameters();
        // options window
        options->whereToCallBack(this);
        options->useParameters(*param);
        options->show();
      }
    }
  }

  // mouse-moved-event call back function
  gint mainGTKWindow::mouseMovedEvent(GtkWidget* widget,
                                      GdkEventMotion* event,
                                      gpointer data) {

    mainGTKWindow* me = (mainGTKWindow*)(data);
    me->mouseMovedEventLocal(widget,event);

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  void mainGTKWindow::mouseMovedEventLocal(GtkWidget* widget,
                                           GdkEventMotion* event) {
    const point pos(iround(event->x),iround(event->y));
    const bool shift = (event->state & GDK_SHIFT_MASK) != 0;
    const bool ctrl  = (event->state & GDK_CONTROL_MASK) != 0;

    if (isNull(dynamic_cast<viewerBase::parameters*>(param))) {
      return;
    }

    if (busy) {
      return;
    }

    mouseMovedHandler(mouseButtonPressed,pos,shift,ctrl);

    lastMousePos = pos;
  };


  void mainGTKWindow::mouseMovedHandler(const int& button,
                                        const point& pos,
                                        const bool shift,
                                        const bool ctrl) {
  }

  gint mainGTKWindow::geometryChanged(GtkWidget* widget,
                                      GdkEventConfigure* event,
                                      gpointer data) {
    mainGTKWindow* me = (mainGTKWindow*)(data);

    me->winSize = point(event->width,event->height); //size;
    me->winPosition = point(event->x,event->y); //pos;

    return (FALSE); // continue normal event handling
                    // TRUE to stop normal event handling
  }

  // -----------------------------------------------------------------------
  // Configuration Dialog
  // -----------------------------------------------------------------------

  configGTKDialog::configGTKDialog() {

    // Color selector
    colorSelector = new colorDialog();

    // File Selector
    filew = gtk_file_selection_new ("File selection");

    // Connect the ok_button to file_ok_sel function
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
                        "clicked", (GtkSignalFunc) on_fileselector_ok,
                        this);

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

    tooltips = gtk_tooltips_new ();
    configGTKWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  }

  configGTKDialog::~configGTKDialog() {
    hide();

    gtk_widget_destroy(configGTKWidget);
    gtk_widget_destroy(filew);

    delete colorSelector;
    colorSelector = 0;

  }

  bool configGTKDialog::setDialogData() {
    viewerBase::parameters* param =
      dynamic_cast<viewerBase::parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }
    return true;
  }

  bool configGTKDialog::getDialogData() {
    viewerBase::parameters* param =
      dynamic_cast<viewerBase::parameters*>(this->param);
    if (isNull(param)) {
      return false;
    }

    return true;
  }

  configGTKDialog* configGTKDialog::newInstance() const {
    return new configGTKDialog;
  }

  bool
  configGTKDialog::useParameters(viewerBase::parameters& theParam) {
    if (dynamic_cast<viewerBase::parameters*>(&theParam) != 0) {
      param = &theParam;
      return true;
    }
    param = 0;
    return false;
  }

  void configGTKDialog::show() {
    setDialogData();
    gtk_widget_show(configGTKWidget);
    gdk_window_raise(configGTKWidget->window);
  }

  void configGTKDialog::hide() {
    colorSelector->hide();
    gtk_widget_hide(filew);
    gtk_widget_hide(configGTKWidget);
  }

  void configGTKDialog::appendFrame(GtkWidget* widget) {
    if (widget != 0) {
      gtk_box_pack_start (GTK_BOX (ConfigFrame), widget, FALSE, FALSE, 2);
    }
  }

  // build common page
  GtkWidget* configGTKDialog::buildCommonFrame() {
    // the widget object that will be returned
    return 0;
  };

  void configGTKDialog::insertFrames() {
    appendFrame(buildCommonFrame());
  }

  void configGTKDialog::buildDialog() {

    gtk_object_set_data (GTK_OBJECT (configGTKWidget),
                         "configGTKWidget", configGTKWidget);
    gtk_window_set_title (GTK_WINDOW (configGTKWidget),
                          _("Configuration Dialog"));
    gtk_window_set_policy (GTK_WINDOW (configGTKWidget), TRUE, TRUE, FALSE);

    dialogBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (dialogBox);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "dialogBox",
                              dialogBox,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (dialogBox);
    gtk_container_add (GTK_CONTAINER (configGTKWidget), dialogBox);

    ConfigFrame = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (ConfigFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ConfigFrame",
                              ConfigFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ConfigFrame);
    gtk_box_pack_start (GTK_BOX (dialogBox), ConfigFrame, TRUE, TRUE, 0);

    insertFrames();

    savingFrame = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (savingFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "savingFrame",
                              savingFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (savingFrame);
    gtk_box_pack_start (GTK_BOX (dialogBox), savingFrame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (savingFrame), 5);

    filename = gtk_entry_new ();
    gtk_widget_ref (filename);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "filename",
                              filename,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (filename);
    gtk_box_pack_start (GTK_BOX (savingFrame), filename, TRUE, TRUE, 0);
    gtk_tooltips_set_tip (tooltips, filename, _("Filename for the displayed image (if you decide to save it)"), NULL);
    gtk_entry_set_text (GTK_ENTRY (filename), _("penguin.bmp"));

    browse = gtk_button_new_with_label (_("..."));
    gtk_widget_ref (browse);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "browse",
                              browse,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (browse);
    gtk_box_pack_start (GTK_BOX (savingFrame), browse, FALSE, FALSE, 5);
    gtk_tooltips_set_tip (tooltips, browse, _("Browse for a file name."),
                          NULL);

    save_button = gtk_button_new_with_label (_("Save"));
    gtk_widget_ref (save_button);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "save_button",
                              save_button,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (save_button);
    gtk_box_pack_start (GTK_BOX (savingFrame), save_button, FALSE, FALSE, 5);
    gtk_tooltips_set_tip (tooltips, save_button, _("Save currently displayed data in the given file."), NULL);

    buttonsFrame = gtk_hbox_new (TRUE, 0);
    gtk_widget_ref (buttonsFrame);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "buttonsFrame",
                              buttonsFrame,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (buttonsFrame);
    gtk_box_pack_start (GTK_BOX (dialogBox), buttonsFrame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (buttonsFrame), 5);

    ok_button = gtk_button_new_with_label (_("OK"));
    gtk_widget_ref (ok_button);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "ok_button",
                              ok_button,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (ok_button);
    gtk_box_pack_start (GTK_BOX (buttonsFrame), ok_button, FALSE, FALSE, 12);
    gtk_tooltips_set_tip (tooltips, ok_button, _("Apply the parameters you have chosen and close the configuration window"), NULL);

    apply_button = gtk_button_new_with_label (_("Apply"));
    gtk_widget_ref (apply_button);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "apply_button", apply_button,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (apply_button);
    gtk_box_pack_start (GTK_BOX (buttonsFrame), apply_button, FALSE, FALSE, 12);
    gtk_tooltips_set_tip (tooltips, apply_button, _("Apply the parameters you have chosen."), NULL);

    cancel_button = gtk_button_new_with_label (_("Cancel"));
    gtk_widget_ref (cancel_button);
    gtk_object_set_data_full (GTK_OBJECT (configGTKWidget), "cancel_button", cancel_button,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (cancel_button);
    gtk_box_pack_start (GTK_BOX (buttonsFrame), cancel_button, FALSE, FALSE, 12);
    gtk_tooltips_set_tip (tooltips, cancel_button, _("Close the configuration window and ignore the changes you have made."), NULL);

    gtk_signal_connect (GTK_OBJECT (browse), "clicked",
                        GTK_SIGNAL_FUNC (on_browse_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (save_button), "clicked",
                        GTK_SIGNAL_FUNC (on_save_button_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
                        GTK_SIGNAL_FUNC (on_ok_button_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (apply_button), "clicked",
                        GTK_SIGNAL_FUNC (on_apply_button_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                        GTK_SIGNAL_FUNC (on_cancel_button_clicked),
                        this);
    gtk_signal_connect (GTK_OBJECT (configGTKWidget), "delete_event",
                        GTK_SIGNAL_FUNC (delete_event),
                        this);

    gtk_object_set_data (GTK_OBJECT (configGTKWidget), "tooltips", tooltips);

  }

  void configGTKDialog::newStrCopy(char* &dest, const char* src) {
    if(dest)
      delete[] dest;
    const int l = strlen(src)+1;
    dest = new char[l];
    strcpy(dest,src);
  };

  std::string configGTKDialog::toString(const double& value,
                                        const int& dec) {
    char phrase[8];
    sprintf(phrase,"%s%i%s","%.",dec,"f");
    char tmpBuf[64];
    sprintf(tmpBuf,phrase,value); //"%.4f"
    return std::string(tmpBuf);
  };

  std::string configGTKDialog::toString(const trgbPixel<float>& px,
                                        const int dec) {
    std::string str;
    str = toString(px.getRed(),dec) + ","
      + toString(px.getGreen(),dec) + ","
      + toString(px.getBlue(),dec);
    return str;
  }

  std::string configGTKDialog::toString(const rgbPixel& px) {
    std::string str;
    str = toString(px.getRed(),0) + ","
      + toString(px.getGreen(),0) + ","
      + toString(px.getBlue(),0);
    return str;
  }

  std::string configGTKDialog::toString(const point& pt) {
    std::string str;
    str = toString(pt.x,0) + " x " + toString(pt.y,0);
    return str;
  }

  double configGTKDialog::fromString(const char* value) {
    return atof(value);
  };

  /*
   * set the parent of this dialog to tell it when everything is done!
   */
  void configGTKDialog::whereToCallBack(mainGTKWindow* obj) {
    theMainWindow = obj;
  }

  // --------------------------------------------------------------------------
  // Static callbacks
  // --------------------------------------------------------------------------

  void configGTKDialog::redrawMainWindow() {
    theMainWindow->useParameters(*param);
    theMainWindow->changeGeometryRequest();
    theMainWindow->redraw();
  };

  void configGTKDialog::on_ok_button_clicked(GtkButton *button,
                                             gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;
    mee->getDialogData();
    mee->redrawMainWindow();
    mee->hide();
  }

  void configGTKDialog::on_apply_button_clicked(GtkButton *button,
                                                gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;
    mee->getDialogData();
    mee->redrawMainWindow();
  };

  void configGTKDialog::on_cancel_button_clicked(GtkButton *button,
                                                 gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;
    mee->hide();
  }

  gint configGTKDialog::delete_event(GtkWidget *widget,
                                     GdkEvent  *event,
                                     gpointer   me) {

    configGTKDialog* mee = (configGTKDialog*)me;

    mee->hide();

    /*
     * Change TRUE to FALSE and the main window will be destroyed with
     * a "delete_event".
     */
    return(TRUE);
  }


  void configGTKDialog::on_browse_clicked(GtkButton *button,
                                          gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;

    std::string tString =
      gtk_entry_get_text(GTK_ENTRY(mee->filename));

    gtk_file_selection_set_filename (GTK_FILE_SELECTION(mee->filew),
                                     tString.c_str());

    gtk_widget_show(mee->filew);
  }

  void configGTKDialog::on_fileselector_ok(GtkButton *widget,
                                           gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;

    std::string tString =
      gtk_file_selection_get_filename (GTK_FILE_SELECTION (mee->filew));

    gtk_entry_set_text(GTK_ENTRY(mee->filename),tString.c_str());

    gtk_widget_hide(mee->filew);
  };

  void configGTKDialog::on_save_button_clicked(GtkButton *button,
                                               gpointer me) {
    configGTKDialog* mee = (configGTKDialog*)me;
    gtk_widget_hide(mee->filew);

    std::string filename(gtk_entry_get_text(GTK_ENTRY(mee->filename)));

    const lti::image& viewed =
      (mee->theMainWindow)->getDisplayedData();

    lti::saveImage saver;
    saver.save(filename,viewed);
  };

}

#endif
