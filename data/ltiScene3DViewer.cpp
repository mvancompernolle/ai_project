/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiScene3DViewer.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 17.1.2003
 * revisions ..: $Id: ltiScene3DViewer.cpp,v 1.11 2006/09/05 10:43:44 ltilib Exp $
 */

#include "ltiScene3DViewer.h"

#ifdef HAVE_GTK

#include <cstdio>

namespace lti {


  // --------------------------------------------------
  // viewerBase3D::parameters
  // --------------------------------------------------

  // default constructor
  scene3DViewer::parameters::parameters() 
    : viewerBase3D::parameters() {    
    
    ranges.resize(3,2);
  };

  // copy constructor
  scene3DViewer::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  scene3DViewer::parameters::~parameters() {    
  }

  // get type name
  const char* scene3DViewer::parameters::getTypeName() const {
    return "scene3DViewer::parameters";
  }
  
  // copy member

  scene3DViewer::parameters& 
    scene3DViewer::parameters::copy(const parameters& other) {
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
    
    ranges = other.ranges;

    return *this;
  }

  // clone member
  functor::parameters* scene3DViewer::parameters::clone() const {
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
  bool scene3DViewer::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool scene3DViewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"ranges",ranges);
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
  bool scene3DViewer::parameters::write(ioHandler& handler,
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
  bool scene3DViewer::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool scene3DViewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"ranges",ranges);
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
  bool scene3DViewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  };
# endif

  // --------------------------------------------------
  // scene3DViewer
  // --------------------------------------------------

  // default constructor
  scene3DViewer::scene3DViewer(bool createDefaultParameters)
    :  viewerBase3D(false) {

    if (createDefaultParameters) {
      parameters defaultParameters;
      setParameters(defaultParameters);
    }

  };

  // copy constructor
  scene3DViewer::scene3DViewer(const scene3DViewer& other)
    : viewerBase3D(false) {

    copy(other);
  };

  // default constructor
  scene3DViewer::scene3DViewer(const std::string& title)
    : viewerBase3D(false) {

    parameters defaultParameters;
    defaultParameters.title = title;
    setParameters(defaultParameters);
  }

  // default constructor
  scene3DViewer::scene3DViewer(const char* title)
    : viewerBase3D(false) {

    parameters defaultParameters;
    defaultParameters.title = title;
    setParameters(defaultParameters);
  }


  // destructor
  scene3DViewer::~scene3DViewer() {
  }

  // returns the name of this type
  const char* scene3DViewer::getTypeName() const {
    return "scene3DViewer";
  }

  // copy member
  scene3DViewer&
    scene3DViewer::copy(const scene3DViewer& other) {
    viewerBase3D::copy(other);

    return (*this);  }
    
  // operator=
  scene3DViewer&
    scene3DViewer::operator=(const scene3DViewer& other) {
    viewerBase3D::copy(other);

    return (*this);  }

  // clone member
  viewerBase* scene3DViewer::clone() const {
    return new scene3DViewer(*this);
  }

  // return parameters
  const scene3DViewer::parameters&
    scene3DViewer::getParameters() const {
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

  bool scene3DViewer::show(scene3D<rgbPixel>& scene) {
    if (ensureMainWindowInstance()) {

      mainWindow* window = dynamic_cast<mainWindow*>(wnd);

      if (notNull(window)) {
        window->lock();

        // transfer data to wnd
        window->putData(scene);

        window->unlock();
        return viewerBase3D::show();
      }
    }
    return false;
  }

  viewerBase3D::mainWindow* scene3DViewer::getNewMainWindow() const {
    return new mainWindow();
  }


  // -------------------------------------------------------------------
  // Main Window
  // -------------------------------------------------------------------

  scene3DViewer::mainWindow::mainWindow()
    : viewerBase3D::mainWindow(false) {

    gdk_threads_enter();
    options = new configDialog();
    options->buildDialog();
    gdk_threads_leave();

    param = new parameters();
  };

  scene3DViewer::mainWindow::mainWindow(const mainWindow& other)
    : viewerBase3D::mainWindow(false) {

    gdk_threads_enter();
    options = new configDialog();
    options->buildDialog();
    gdk_threads_leave();

    param = new parameters();

    copy(other);

  };

  scene3DViewer::mainWindow::~mainWindow() {
  }

  void scene3DViewer::mainWindow::putData(scene3D<rgbPixel>& scene) {
      ptScene=&scene;
  };

  // return parameters
  const scene3DViewer::parameters&
    scene3DViewer::mainWindow::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(param);

    if (isNull(par)) {
      throw invalidParametersException(getTypeName());
    }

    return *par;
  }

  void scene3DViewer::mainWindow::prepareParameters(void){
    parameters* param = dynamic_cast<parameters*>(this->param);

    if (notNull(param)) {
      param->ranges = ptScene->getRange();
    }
  };


  // ----------------------------------------------------
  //        generate image from scene
  // ----------------------------------------------------
  void scene3DViewer::mainWindow::dataToImage() {
      ptScene->setZoom(param->camParameters.zoom);
      param->camParameters.setCamera(dpoint3D(0.,0.,0.),
                                     param->camParameters.elevation,
                                     param->camParameters.azimuth);
      ptScene->flush(param->camParameters,theImage);

	  // TODO: Drawing the axis is necessary, but more configuration
	  //       options in the dialog need to be added.
#ifndef _LTI_MSC_6

//      viewerBase3D::mainWindow::dataToImage();
#else
//      void (viewerBase3D::mainWindow::* p_dataToImage)() = 
//        viewerBase3D::mainWindow::dataToImage;
//      (this->*p_dataToImage)();
#endif
  }

  // -------------------------------------------------------------------
  // Config Dialog
  // -------------------------------------------------------------------

  scene3DViewer::configDialog::configDialog() 
    : viewerBase3D::configDialog() {
  };

  scene3DViewer::configDialog::~configDialog() {
  }

  void scene3DViewer::configDialog::insertPages() {
    appendPage(buildCommonPage());
    appendPage(buildRangePage());
  }

  scene3DViewer::configDialog::pageWidget 
  scene3DViewer::configDialog::buildRangePage() {

  pageWidget pw;
  pw.label = "axes ranges";

  GtkWidget* theWidget = gtk_vbox_new (FALSE, 0);
  pw.widget=theWidget;

  gtk_widget_ref (theWidget);
  gtk_object_set_data_full (GTK_OBJECT (settings), "theWidget", theWidget,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (theWidget);

  GtkWidget* hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (theWidget), hbox1, TRUE, FALSE, 0);


//  GtkWidget* table1 = gtk_table_new (4, 3, FALSE);
  table1 = gtk_table_new (4, 3, FALSE);

  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (hbox1), table1, TRUE, TRUE, 0);


  GtkWidget* label1 = gtk_label_new (" x-Axis ");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);


  GtkWidget* label2 = gtk_label_new (" y-Axis ");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);


  GtkWidget* label3 = gtk_label_new (" z-Axis ");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);


  GtkWidget* label4 = gtk_label_new ("   min   ");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);


  GtkWidget* label5 = gtk_label_new ("   max   ");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);
  
  return pw;
  }

  // transfer the data in the param object into the gtk widgets axel
# ifndef _LTI_MSC_6
  bool scene3DViewer::configDialog::setDialogData() 
# else  
  bool scene3DViewer::configDialog::setDialogDataMS() 
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

 GtkWidget* label9 = gtk_label_new (toString(param->ranges.at(0,1),2).c_str());
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table1), label9, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  GtkWidget* label6 = gtk_label_new (toString(param->ranges.at(0,0),2).c_str());
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  GtkWidget* label7 = gtk_label_new(toString(param->ranges.at(1,0),2).c_str());
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);


  GtkWidget* label8=gtk_label_new(toString(param->ranges.at(2,0),2).c_str());
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  GtkWidget* label10=gtk_label_new(toString(param->ranges.at(1,1),2).c_str());
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table1), label10, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);


  GtkWidget* label11=gtk_label_new(toString(param->ranges.at(2,1),2).c_str());
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (settings), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table1), label11, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

    return true;
  }

# ifndef _LTI_MSC_6
  bool scene3DViewer::configDialog::getDialogData() 
# else  
  bool scene3DViewer::configDialog::getDialogDataMS() 
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

    
    return true;
  };

# ifdef _LTI_MSC_6
  bool scene3DViewer::configDialog::getDialogData() {
    return getDialogDataMS();
  }

  bool scene3DViewer::configDialog::setDialogData() {
    return setDialogDataMS();
  }
# endif

}

#endif
