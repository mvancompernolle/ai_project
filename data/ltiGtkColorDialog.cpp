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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiGtkColorDialog.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 10.12.2001
 * revisions ..: $Id: ltiGtkColorDialog.cpp,v 1.3 2006/02/07 20:50:26 ltilib Exp $
 */


#include "ltiObject.h"
#include "ltiGtkColorDialog.h"

#ifdef HAVE_GTK

namespace lti {


  // -------------------------------------------------------------
  //                    COLOR SELECTION DIALOG
  // -------------------------------------------------------------

  colorDialog::colorDialog() {
    colorselectiondialog = gtk_color_selection_dialog_new ("Select Color");
    gtk_object_set_data (GTK_OBJECT (colorselectiondialog),
                         "colorselectiondialog", colorselectiondialog);
    gtk_container_set_border_width (GTK_CONTAINER (colorselectiondialog), 10);

    GtkWidget* colorsel;
    colorsel = GTK_COLOR_SELECTION_DIALOG(colorselectiondialog)->colorsel;

    gtk_signal_connect (GTK_OBJECT (colorsel), "color_changed",
			GTK_SIGNAL_FUNC (colorChanged_callback), this);

    ok_button1 = GTK_COLOR_SELECTION_DIALOG (colorselectiondialog)->ok_button;
    gtk_object_set_data (GTK_OBJECT (colorselectiondialog), "ok_button1",
                         ok_button1);
    gtk_widget_show (ok_button1);
    GTK_WIDGET_SET_FLAGS (ok_button1, GTK_CAN_DEFAULT);

    gtk_signal_connect (GTK_OBJECT (ok_button1), "clicked",
			GTK_SIGNAL_FUNC (ok_callback), this);

    cancel_button1 = GTK_COLOR_SELECTION_DIALOG (colorselectiondialog)->cancel_button;
    gtk_object_set_data (GTK_OBJECT (colorselectiondialog),
                         "cancel_button1", cancel_button1);
    gtk_widget_show (cancel_button1);
    GTK_WIDGET_SET_FLAGS (cancel_button1, GTK_CAN_DEFAULT);

    gtk_signal_connect (GTK_OBJECT (cancel_button1), "clicked",
			GTK_SIGNAL_FUNC (cancel_callback), this);

    help_button1 = GTK_COLOR_SELECTION_DIALOG (colorselectiondialog)->help_button;
    gtk_object_set_data (GTK_OBJECT (colorselectiondialog),
                         "help_button1", help_button1);
    gtk_widget_show (help_button1);
    GTK_WIDGET_SET_FLAGS (help_button1, GTK_CAN_DEFAULT);
  }

  colorDialog::~colorDialog() {
    hide();
    gtk_widget_destroy (colorselectiondialog);
  }

  void colorDialog::show() {
    gtk_widget_show(colorselectiondialog);
  }

  void colorDialog::hide() {
    gtk_widget_hide(colorselectiondialog);
  }

  void colorDialog::useColor(rgbPixel& pixel) {
    extColor = &pixel;
    color.copy(pixel);

    gdouble clr[4];
    clr[0] = gdouble(color.getRed())/255.0;
    clr[1] = gdouble(color.getGreen())/255.0;
    clr[2] = gdouble(color.getBlue())/255.0;
    clr[3] = 0;

    GtkWidget* colorsel;
    colorsel = GTK_COLOR_SELECTION_DIALOG(colorselectiondialog)->colorsel;

    gtk_color_selection_set_color(GTK_COLOR_SELECTION(colorsel),clr);

  }

  void colorDialog::colorChanged_callback(GtkColorSelection* widget,
                                          gpointer data) {
    colorDialog* me = (colorDialog*)data;

    me->colorChanged_local(widget);
  }

  void colorDialog::colorChanged_local(GtkColorSelection* widget) {
    gdouble clr[4];

    gtk_color_selection_get_color(widget,clr);
    color.setRed(static_cast<ubyte>(255*clr[0]));
    color.setGreen(static_cast<ubyte>(255*clr[1]));
    color.setBlue(static_cast<ubyte>(255*clr[2]));
  }

  void colorDialog::ok_callback(GtkWidget *widget,
                                    gpointer data) {
    colorDialog* me = (colorDialog*)data;

    me->ok_local(widget);
  }

  void colorDialog::ok_local(GtkWidget *widget) {
    extColor->copy(color);
    hide();
  }

  void colorDialog::cancel_callback(GtkWidget *widget,
                               gpointer data) {

    colorDialog* me = (colorDialog*)data;

    me->cancel_local(widget);
  }

  void colorDialog::cancel_local(GtkWidget *widget) {
    hide();
  }
}

#endif
