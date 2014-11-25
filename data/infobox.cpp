/**************************************************************

starinfo.cpp (C-Munipack project)
The toolbox window that shows the information about a star
Copyright (C) 2009 David Motl, dmotl@volny.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "infobox.h"
#include "cmpack_widgets.h"
#include "utils.h"
#include "configuration.h"
#include "main.h"

//-------------------------   INFO BOX CONTAINER   --------------------------------

CInfoBox::CInfoBox(const gchar *caption):m_Title(NULL), m_Pixmap(NULL), m_PixWidth(0), m_PixHeight(0), 
	m_Visible(false), m_NeedUpdate(true)
{
	if (caption) 
		m_Title = g_strdup(caption);

	// Caption
	m_Caption = gtk_drawing_area_new();
	gtk_widget_set_size_request(m_Caption, -1, 16);
	gtk_widget_set_events(m_Caption, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(m_Caption), "expose_event", G_CALLBACK(expose_event), this);
	g_signal_connect(G_OBJECT(m_Caption), "configure_event", G_CALLBACK(configure_event), this);
	g_signal_connect(G_OBJECT(m_Caption), "button_press_event", G_CALLBACK(button_press_event), this);

	// Frame
	m_Frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(m_Frame), GTK_SHADOW_IN);

	// Box
	m_Box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_Frame), GTK_WIDGET(m_Box));
	gtk_box_pack_start(GTK_BOX(m_Box), m_Caption, FALSE, TRUE, 0);

	gtk_widget_hide(m_Frame);
}

CInfoBox::~CInfoBox()
{
	if (m_Pixmap)
		g_object_unref(m_Pixmap);
	g_free(m_Title);
}

// Show/hide
void CInfoBox::Show(bool show)
{
	m_Visible = show;
	if (m_Visible) {
		OnShow();
		gtk_widget_show(m_Frame);
	} else {
		gtk_widget_hide(m_Frame);
		OnHide();
	}
}

void CInfoBox::SetCaption(const gchar *caption)
{
	g_free(m_Title);
	m_Title = NULL;
	if (caption) 
		m_Title = g_strdup(caption);
	m_NeedUpdate = true;
	if (m_Caption)
		gtk_widget_queue_draw(m_Caption);
}

gint CInfoBox::configure_event(GtkWidget *widget, GdkEventConfigure *event, CInfoBox *pMe)
{
	pMe->OnConfigure(widget, event);
	return TRUE;
}

void CInfoBox::OnConfigure(GtkWidget *widget, GdkEventConfigure *event)
{
	int width, height;

	width = widget->allocation.width;
	height = widget->allocation.height;
	if (width!=m_PixWidth || height!=m_PixHeight) {
		if (m_Pixmap) {
			g_object_unref(m_Pixmap);
			m_Pixmap = NULL;
		}
		if (width>0 && height>0) {
			m_Pixmap = gdk_pixmap_new(widget->window, width, height, -1);
			m_PixWidth = width;
			m_PixHeight = height;
		} else {
			m_PixHeight = m_PixWidth = 0;
		}
		m_NeedUpdate = true;
		gtk_widget_queue_draw(m_Caption);
	}
}

// Redraw the screen from the backing pixmap 
gint CInfoBox::expose_event(GtkWidget *widget, GdkEventExpose *event, CInfoBox *pMe)
{
	pMe->OnExposeEvent(widget, event);
	return FALSE;
}

void CInfoBox::OnExposeEvent(GtkWidget *widget, GdkEventExpose *event)
{
	if (m_NeedUpdate) {
		m_NeedUpdate = false;
		RedrawCaption();
	}
	gdk_draw_drawable(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		m_Pixmap, event->area.x, event->area.y, event->area.x, event->area.y,
		event->area.width, event->area.height);
}

void CInfoBox::RedrawCaption()
{
	int dx, dy;
	cairo_text_extents_t extents;

	if (m_Pixmap && m_PixWidth>0 && m_PixHeight>0) {
		cairo_t *cr = gdk_cairo_create(m_Pixmap);
		// Background
		cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
		cairo_paint(cr);
		// Caption
		if (m_Title) {
			cairo_set_font_size(cr, 12.0);
			cairo_select_font_face(cr, "", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
			cairo_text_extents(cr, "X", &extents);
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_move_to(cr, 4, (m_PixHeight+extents.height)/2);
			cairo_show_text(cr, m_Title);
		}
		// Close icon
		cairo_set_line_width(cr, 2);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_set_source_rgb(cr, 0, 0, 0);
		dx = m_PixWidth-12; dy = (m_PixHeight-8)/2;
		cairo_move_to(cr, dx, dy);
		cairo_line_to(cr, dx+8, dy+8);
		cairo_move_to(cr, dx+8, dy);
		cairo_line_to(cr, dx, dy+8);
		cairo_stroke(cr);
		cairo_destroy(cr);
	}
}

gint CInfoBox::button_press_event(GtkWidget *widget, GdkEventButton *event, CInfoBox *pMe)
{
	pMe->OnButtonPress(widget, event);
	return true;
}

void CInfoBox::OnButtonPress(GtkWidget *widget, GdkEventButton *event)
{
	if (widget==m_Caption && event->type==GDK_BUTTON_PRESS && event->button==1) {
		if (event->x >= m_PixWidth-16) {
			if (OnCloseQuery()) {
				Show(false);
				Callback(CB_CLOSED);
			}
		}
	}
}

int CInfoBox::text_width(GtkWidget *widget, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		g_object_unref (layout);
		return logical_rect.width;
	}
	return 0;
}

//-------------------------   INFO BOX FOR TEXT   --------------------------------

CTextBox::CTextBox(void):CInfoBox(NULL)
{
	// Data buffer
	m_Buffer = gtk_text_buffer_new(NULL);

	// Frame info
	m_TextView = gtk_text_view_new_with_buffer(m_Buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_TextView), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_TextView), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_TextView), false);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_TextView), GTK_JUSTIFY_CENTER);
	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(m_TextView), 4);
	gtk_box_pack_start(GTK_BOX(m_Box), m_TextView, TRUE, TRUE, 0);
	
	int w = text_width(m_TextView, "Brightness = 99.9999 mag");
	gtk_widget_set_size_request(m_TextView, w+8, -1);

	// Text tags
	gtk_text_buffer_create_tag(m_Buffer, "title1", "weight", PANGO_WEIGHT_BOLD, 
		"underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(m_Buffer, "title2", "weight", PANGO_WEIGHT_NORMAL, 
		"underline", PANGO_UNDERLINE_SINGLE, NULL);
}

CTextBox::~CTextBox()
{
	g_object_unref(m_Buffer);
}

void CTextBox::Clear(void)
{
	gtk_text_buffer_set_text(m_Buffer, "", 0);
}

void CTextBox::AddTitle(int level, const char *caption)
{
	GtkTextIter from, to;
	GtkTextMark *mark;

	gtk_text_buffer_get_end_iter(m_Buffer, &from);
	mark = gtk_text_buffer_create_mark(m_Buffer, NULL, &from, TRUE);
	gtk_text_buffer_insert(m_Buffer, &from, caption, -1);
	gtk_text_buffer_get_end_iter(m_Buffer, &to);
	gtk_text_buffer_get_iter_at_mark(m_Buffer, &from, mark);
	if (level==1)
		gtk_text_buffer_apply_tag_by_name(m_Buffer, "title1", &from, &to);
	else
		gtk_text_buffer_apply_tag_by_name(m_Buffer, "title2", &from, &to);
	gtk_text_buffer_delete_mark(m_Buffer, mark);
	gtk_text_buffer_get_end_iter(m_Buffer, &to);
	gtk_text_buffer_insert(m_Buffer, &to, "\n", -1);
}

void CTextBox::AddText(const char *caption)
{
	GtkTextIter iter;

	gtk_text_buffer_get_end_iter(m_Buffer, &iter);
	gtk_text_buffer_insert(m_Buffer, &iter, caption, -1);
	gtk_text_buffer_insert(m_Buffer, &iter, "\n", -1);
}

void CTextBox::BeginUpdate(void)
{
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_TextView), NULL);
}

void CTextBox::EndUpdate(void)
{
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_TextView), m_Buffer);
}

//-------------------------   GRAY SCALE   --------------------------------

CScaleBox::CScaleBox():CInfoBox("Gray scale")
{
	gtk_widget_set_size_request(m_Box, 128, -1);

	m_Scale = cmpack_scale_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start(GTK_BOX(m_Box), m_Scale, TRUE, TRUE, 0);
}

void CScaleBox::SetScaleParams(double min, double max, bool pseudocolors, bool invert)
{
	cmpack_scale_set_range(CMPACK_SCALE(m_Scale), min, max);
	cmpack_scale_set_pseudocolor(CMPACK_SCALE(m_Scale), pseudocolors);
	cmpack_scale_set_invert(CMPACK_SCALE(m_Scale), invert);
}

//-------------------------   PROFILE  --------------------------------

// Constructor
CProfileBox::CProfileBox():CInfoBox("Profile"), m_Style(0), m_X0(0), m_Y0(0), 
	m_X1(0), m_Y1(0), m_Count(0), m_YMin(0), m_YMax(0), m_Mean(0), m_StdDev(0)
{
	int w;
	GtkWidget *scrwnd, *hbox, *vbox, *frame;

	gtk_widget_set_size_request(m_Box, -1, 280);

	m_Data = cmpack_graph_data_new();
	m_Buffer = gtk_text_buffer_new(NULL);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_Box), hbox, TRUE, TRUE, 0);

	// Graph view
	m_Plot = cmpack_curve_plot_new_with_model(m_Data);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Plot);
	gtk_box_pack_start(GTK_BOX(hbox), scrwnd, TRUE, TRUE, 0);
	cmpack_curve_plot_set_scales(CMPACK_CURVE_PLOT(m_Plot), TRUE, TRUE);
	cmpack_curve_plot_set_mouse_control(CMPACK_CURVE_PLOT(m_Plot), TRUE);

	// Right tools
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_widget_set_size_request(vbox, 120, -1);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	
	m_StyleBox = gtk_combo_box_new_text();
	gtk_widget_set_tooltip_text(m_StyleBox, "Display style");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_StyleBox), "Lines");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_StyleBox), "Steps");
	gtk_box_pack_start(GTK_BOX(vbox), m_StyleBox, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_StyleBox), "changed", G_CALLBACK(select_changed), this);

	// Text area
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	m_View = gtk_text_view_new_with_buffer(m_Buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_View), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_View), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_View), FALSE);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_View), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(frame), m_View);
	w = text_width(m_View, "St.dev: 99999.9 ADU");
	gtk_widget_set_size_request(m_View, w+24, -1);

	g_signal_connect(G_OBJECT(m_Plot), "mouse-moved", G_CALLBACK(mouse_moved), this);
}

// Destructor
CProfileBox::~CProfileBox()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_Plot), (gpointer)mouse_moved, this);
	g_object_unref(m_Data);
	g_object_unref(m_Buffer);
}

// Set scale parameters
void CProfileBox::SetProfile(const CImage &image, int x0, int y0, int x1, int y1)
{
	gdouble d;

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_View), NULL);
	cmpack_curve_plot_set_model(CMPACK_CURVE_PLOT(m_Plot), NULL);

	image.Profile(m_Data, x0, y0, x1, y1, &m_YMin, &m_YMax, &m_Mean, &m_StdDev);

	m_Count = cmpack_graph_data_nrows(m_Data);
	if (m_Count>0) {
		// Update plot
		m_X0 = x0; m_X1 = x1; m_Y0 = y0; m_Y1 = y1;
		d = sqrt((double)(x1-x0)*(x1-x0)+(double)(y1-y0)*(y1-y0));
		cmpack_curve_plot_set_x_axis(CMPACK_CURVE_PLOT(m_Plot), d/m_Count, 0, 
			1.0, PLOT_INT, 0, 0, "Pixel");
		cmpack_curve_plot_set_y_axis(CMPACK_CURVE_PLOT(m_Plot), FALSE, FALSE, 
			m_YMin, m_YMax, m_YMax, PLOT_INT, 0, 1, "ADU");
	} else {
		m_X0 = m_X1 = m_Y0 = m_Y1 = 0;
		cmpack_curve_plot_set_x_axis(CMPACK_CURVE_PLOT(m_Plot), 0, 0, 
			1.0, PLOT_INT, 0, 0, "Pixel");
		cmpack_curve_plot_set_y_axis(CMPACK_CURVE_PLOT(m_Plot), FALSE, FALSE, 
			0.0, 0.0, 64.0, PLOT_INT, 0, 1, "ADU");
	}
	cmpack_curve_plot_set_model(CMPACK_CURVE_PLOT(m_Plot), m_Data);
	cmpack_curve_plot_auto_zoom(CMPACK_CURVE_PLOT(m_Plot), TRUE, TRUE);

	UpdateText();
}

// Set scale parameters
void CProfileBox::UpdateText(void)
{
	gchar msg[256];
	GtkTextIter ptr;

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_View), NULL);
	gtk_text_buffer_set_text(m_Buffer, "", -1);
	gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
	m_Count = cmpack_graph_data_nrows(m_Data);
	if (m_Count>0) {
		sprintf(msg, "Min.: %.1f ADU\n", m_YMin);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Max.: %.1f ADU\n", m_YMax);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Mean: %.1f ADU\n", m_Mean);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "St.dev.: %.1f ADU\n", m_StdDev);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	} else {
		gtk_text_buffer_insert(m_Buffer, &ptr, "To show a profile, move the mouse to the chart, press and hold the left button and draw a line.", -1);
	}
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_View), m_Buffer);
}

// Set scale parameters
void CProfileBox::Clear(void)
{
	cmpack_graph_data_clear(m_Data);
}

// Initialization before the tool is shown
void CProfileBox::OnShow(void) 
{
	m_Style = g_Project->GetInt("Profile", "Style", 0, 0, 1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_StyleBox), m_Style);
	CmpackPlotStyle s = (m_Style ? PLOT_STEPS : PLOT_LINES);
	cmpack_curve_plot_set_style(CMPACK_CURVE_PLOT(m_Plot), s);
	UpdateText();
}

// Change display style
void CProfileBox::SetDisplayStyle(int style)
{
	if (style!=m_Style) {
		m_Style = style;
		CmpackPlotStyle s = (m_Style ? PLOT_STEPS : PLOT_LINES);
		cmpack_curve_plot_set_style(CMPACK_CURVE_PLOT(m_Plot), s);
		g_Project->SetInt("Profile", "Style", m_Style);
	}
}

// Clean up after the tool is hidden
void CProfileBox::OnHide(void) 
{
	Clear();
}

// Tell parent that the mouse has been moved
void CProfileBox::mouse_moved(GtkWidget *pChart, CProfileBox *pMe)
{
	pMe->Callback(CB_MOUSE_MOVED);
}

// Get pixel coordinates
bool CProfileBox::GetMousePos(int *x, int *y)
{
	gint index = cmpack_curve_plot_mouse_pos(CMPACK_CURVE_PLOT(m_Plot));
	if (index>=0) {
		if (m_Count>1) {
			if (x)
				*x = m_X0+(int)((double)index*(m_X1-m_X0)/(m_Count-1));
			if (y)
				*y = m_Y0+(int)((double)index*(m_Y1-m_Y0)/(m_Count-1));
		} else {
			if (x)
				*x = m_X0;
			if (y)
				*y = m_Y0;
		}
		return TRUE;
	}
	return FALSE;
}

void CProfileBox::select_changed(GtkComboBox *pWidget, CProfileBox *pMe)
{
	pMe->SetDisplayStyle(gtk_combo_box_get_active(pWidget));
}

//-------------------------   HISTOGRAM  --------------------------------

// Constructor
CHistogramBox::CHistogramBox():CInfoBox("Histogram"), m_pImage(NULL), m_logScale(false),
	m_XMin(0), m_XMax(0), m_Count(0)
{
	int w;
	GtkWidget *scrwnd, *hbox, *vbox, *frame;

	gtk_widget_set_size_request(m_Box, -1, 280);

	m_Data = cmpack_graph_data_new();
	m_Buffer = gtk_text_buffer_new(NULL);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_Box), hbox, TRUE, TRUE, 0);

	// Graph view
	m_Plot = cmpack_curve_plot_new_with_model(m_Data);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Plot);
	gtk_box_pack_start(GTK_BOX(hbox), scrwnd, TRUE, TRUE, 0);
	cmpack_curve_plot_set_scales(CMPACK_CURVE_PLOT(m_Plot), TRUE, FALSE);
	cmpack_curve_plot_set_style(CMPACK_CURVE_PLOT(m_Plot), PLOT_HISTOGRAM);
	cmpack_curve_plot_set_mouse_control(CMPACK_CURVE_PLOT(m_Plot), TRUE);

	// Right tools
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_widget_set_size_request(vbox, 120, -1);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	m_LogScaleChk = gtk_check_button_new_with_label("Log. Y scale");
	gtk_widget_set_tooltip_text(m_LogScaleChk, "Logarithmic Y scale");
	gtk_box_pack_start(GTK_BOX(vbox), m_LogScaleChk, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_LogScaleChk), "toggled", G_CALLBACK(button_toggled), this);

	// Text area
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	m_View = gtk_text_view_new_with_buffer(m_Buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_View), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_View), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_View), FALSE);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_View), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(frame), m_View);
	w = text_width(m_View, "St.dev: 99999.9 ADU");
	gtk_widget_set_size_request(m_View, w+24, -1);

	g_signal_connect(G_OBJECT(m_Plot), "mouse-moved", G_CALLBACK(mouse_moved), this);
}

// Destructor
CHistogramBox::~CHistogramBox()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_Plot), (gpointer)mouse_moved, this);
	g_object_unref(m_Data);
	g_object_unref(m_Buffer);
}

// Set data container
void CHistogramBox::SetData(CImage *image)
{
	m_pImage = image;
	UpdateData();
}

// Set scale parameters
void CHistogramBox::UpdateData(void)
{
	static const int length = 1024;

	double ymax, ch_width, offset, mean, stddev;
	gchar msg[256];
	GtkTextIter ptr;

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_View), NULL);
	cmpack_curve_plot_set_model(CMPACK_CURVE_PLOT(m_Plot), NULL);
	gtk_text_buffer_set_text(m_Buffer, "", -1);

	cmpack_curve_plot_set_model(CMPACK_CURVE_PLOT(m_Plot), NULL);
	cmpack_graph_data_clear(m_Data);
	if (m_pImage && m_pImage->MinMax(&m_XMin, &m_XMax)) {
		ch_width = (m_XMax - m_XMin)/(length-1);
		offset = m_XMin - 0.5*ch_width;
		m_pImage->Histogram(m_Data, length, ch_width, offset, &ymax, m_logScale);
	} else {
		ch_width = offset = 0.0;
	}
	m_Count = cmpack_graph_data_nrows(m_Data);
	if (m_Count>0) {
		cmpack_curve_plot_set_x_axis(CMPACK_CURVE_PLOT(m_Plot), ch_width, offset, 
			1.0, PLOT_INT, 0, 0, "ADU");
		cmpack_curve_plot_set_y_axis(CMPACK_CURVE_PLOT(m_Plot), FALSE, FALSE, 
			0.0, ymax, ymax, PLOT_INT, 0, 1, "");
		gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
		sprintf(msg, "Min.: %.1f ADU\n", m_XMin);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Max.: %.1f ADU\n", m_XMax);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	} else {
		m_Count = 0;
		cmpack_curve_plot_set_x_axis(CMPACK_CURVE_PLOT(m_Plot), 0, 0, 
			1.0, PLOT_INT, 0, 0, "ADU");
		cmpack_curve_plot_set_y_axis(CMPACK_CURVE_PLOT(m_Plot), FALSE, FALSE, 
			0.0, 0.0, 1.0, PLOT_INT, 0, 1, "");
	}
	if (m_pImage && m_pImage->MeanDev(&mean, &stddev)) {
		gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
		sprintf(msg, "Mean: %.1f ADU\n", mean);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "St.dev: %.1f ADU\n", stddev);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
	cmpack_curve_plot_set_model(CMPACK_CURVE_PLOT(m_Plot), m_Data);
	cmpack_curve_plot_auto_zoom(CMPACK_CURVE_PLOT(m_Plot), TRUE, TRUE);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_View), m_Buffer);
}

// Set scale parameters
void CHistogramBox::Clear(void)
{
	cmpack_graph_data_clear(m_Data);
}

// Initialization before the tool is shown
void CHistogramBox::OnShow(void)
{
	m_logScale = g_Project->GetBool("Histogram", "LogScale");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_LogScaleChk), m_logScale);
	UpdateData();
}

// Clean up after the tool is hidden
void CHistogramBox::OnHide(void) 
{
	Clear();
}

// Change display style
void CHistogramBox::SetLogScale(bool enable)
{
	if (enable!=m_logScale) {
		m_logScale = enable;
		UpdateData();
		g_Project->SetBool("Histogram", "LogScale", m_logScale);
	}
}

// Tell parent that the mouse has been moved
void CHistogramBox::mouse_moved(GtkWidget *pChart, CHistogramBox *pMe)
{
	pMe->Callback(CB_MOUSE_MOVED);
}

// Get pixel coordinates
bool CHistogramBox::GetMousePos(double *pos)
{
	gint index = cmpack_curve_plot_mouse_pos(CMPACK_CURVE_PLOT(m_Plot));
	if (index>=0) {
		if (m_Count>1) {
			if (pos)
				*pos = m_XMin+((double)index*(m_XMax-m_XMin)/(m_Count-1));
		} else {
			if (pos)
				*pos = m_XMin;
		}
		return TRUE;
	}
	return FALSE;
}

// Check box toggled
void CHistogramBox::button_toggled(GtkToggleButton *pButton, CHistogramBox *pMe)
{
	pMe->SetLogScale(gtk_toggle_button_get_active(pButton)!=FALSE);
}
