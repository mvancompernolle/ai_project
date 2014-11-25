/**************************************************************

file_dlg.cpp (C-Munipack project)
The preview dialog
Copyright (C) 2008 David Motl, dmotl@volny.cz

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

#include <stdlib.h>
#include <string.h>

#include "ccdfile_dlg.h"
#include "configuration.h"
#include "configuration.h"
#include "showheader_dlg.h"
#include "export_dlgs.h"
#include "main.h"
#include "utils.h"
#include "ctxhelp.h"

enum tMenuId
{
	MENU_FILE = 1,
	MENU_VIEW,
	MENU_IMAGE,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_OPEN = 100,
	CMD_SAVE_AS,
	CMD_CLOSE,
	CMD_PRINT,
	CMD_EXPORT,
	CMD_PROPERTIES,
	CMD_RULERS,
	CMD_PSEUDOCOLORS,
	CMD_PHOTOMETRY,
	CMD_GRAYSCALE,
	CMD_PROFILE,
	CMD_HISTOGRAM,
	CMD_SHOW_HELP,
	CMD_FLIP_H,
	CMD_FLIP_V
};

static const CMenuBar::tMenuItem FileMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_OPEN,		"_Open" },
	{ CMenuBar::MB_ITEM,	CMD_SAVE_AS,	"_Save as ..." },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT,		"_Export" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_PROPERTIES,	"Show _Properties" },
	//{ CMenuBar::MB_SEPARATOR },
	//{ CMenuBar::MB_ITEM, CMD_PRINT,		"P_rint" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_CLOSE,		"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_CHECKBTN, CMD_RULERS,		"_Rulers" },
	{ CMenuBar::MB_CHECKBTN, CMD_PSEUDOCOLORS,	"_Pseudo-color image" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ImageMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_FLIP_H,		"_Horizontal flip" },
	{ CMenuBar::MB_ITEM,	CMD_FLIP_V,		"_Vertical flip" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN, CMD_GRAYSCALE,		"_Gray scale" },
	{ CMenuBar::MB_CHECKBTN, CMD_PHOTOMETRY,	"Quick _photometry" },
	{ CMenuBar::MB_RADIOBTN, CMD_PROFILE,		"Pro_file" },
	{ CMenuBar::MB_RADIOBTN, CMD_HISTOGRAM,		"_Histogram" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_SHOW_HELP,		"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu Menu[] = {
	{ "_File",	MENU_FILE,	FileMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Image", MENU_IMAGE, ImageMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   MENU FOR PREVIEW WINDOW   ---------------------------

//
// Constructor
//
CCCDFileDlg::CCCDFileDlg(void):m_ImageData(NULL), m_Updating(false), m_UpdatePos(true),
	m_UpdateZoom(true), m_MouseOnProfile(false), m_MouseOnHistogram(false), 
	m_LastPosX(-1), m_LastPosY(-1), m_InfoMode(INFO_NONE)
{
	GtkWidget *tbox, *hbox, *scrwnd;

	m_Pseudocolors = g_Project->GetBool("PreviewDlg", "Pseudocolors");
	m_Rulers = g_Project->GetBool("PreviewDlg", "Rulers");
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Menu bar
	m_Menu.Create(Menu, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), false, false, 0);

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(m_MainBox), tbox, false, false, 0);

	toolbar_new_label(tbox, "Zoom");
	m_ZoomFit = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_FIT, "Fit the frame to the window");
	g_signal_connect(G_OBJECT(m_ZoomFit), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomOut = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_OUT, "Zoom out");
	g_signal_connect(G_OBJECT(m_ZoomOut), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomIn = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_IN, "Zoom in");
	g_signal_connect(G_OBJECT(m_ZoomIn), "clicked", G_CALLBACK(button_clicked), this);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_MainBox), hbox, TRUE, TRUE, 0);

	// Chart
	m_Chart = cmpack_chart_view_new();
	cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_Chart), m_Rulers);
	cmpack_chart_view_set_mouse_control(CMPACK_CHART_VIEW(m_Chart), TRUE);
	g_signal_connect(G_OBJECT(m_Chart), "zoom-changed", G_CALLBACK(zoom_changed), this);
	g_signal_connect(G_OBJECT(m_Chart), "mouse-moved", G_CALLBACK(mouse_moved), this);
	g_signal_connect(G_OBJECT(m_Chart), "button-press-event", G_CALLBACK(button_press_event), this);
	g_signal_connect(G_OBJECT(m_Chart), "profile-changed", G_CALLBACK(profile_changed), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Chart);
	gtk_box_pack_start(GTK_BOX(hbox), scrwnd, TRUE, TRUE, 0);

	// Quick photometry
	m_QPhot.RegisterCallback(InfoBoxCallback, this);
	m_QPhot.SetChart(CMPACK_CHART_VIEW(m_Chart));
	gtk_box_pack_start(GTK_BOX(hbox), m_QPhot.Handle(), FALSE, FALSE, 0);

	// Scale
	m_Scale.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_Scale.Handle(), FALSE, FALSE, 0);
		
	// Profile
	m_Profile.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Profile.Handle(), FALSE, FALSE, 0);

	// Histogram
	m_Histogram.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Histogram.Handle(), FALSE, FALSE, 0);

	// Timers
	m_TimerId = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 100, GSourceFunc(timer_cb), this, NULL);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	SetInfoMode(INFO_NONE);
}

//
// Destructor
//
CCCDFileDlg::~CCCDFileDlg()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_Chart), (gpointer)zoom_changed, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_Chart), (gpointer)mouse_moved, this);
	g_source_remove(m_TimerId);
	if (m_ImageData)
		g_object_unref(m_ImageData);
}

bool CCCDFileDlg::LoadFile(const char *fpath, GError **error)
{
	SetStatus(NULL);
	SetInfoMode(INFO_NONE);
	m_Histogram.SetData(NULL);
	m_File.Close();

	CCCDFile file;
	if (file.Open(fpath, CMPACK_OPEN_READONLY, error)) {
		if (file.GetImageData(m_Image, CMPACK_BITPIX_AUTO, error)) {
			m_File.MakeCopy(file);
			m_QPhot.SetParams(m_File);
			if (m_InfoMode == INFO_HISTOGRAM)
				m_Histogram.SetData(&m_Image);
			UpdateImage();
			UpdateControls();
			return true;
		}
	}
	return false;
}

// Environment changed, reload settings
void CCCDFileDlg::EnvironmentChanged(void)
{
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	UpdateImage();
	UpdateControls();
}

void CCCDFileDlg::UpdateControls(void)
{
	m_Menu.Enable(CMD_SAVE_AS, m_File.Valid());
	m_Menu.Enable(CMD_EXPORT, m_File.Valid() && m_ImageData);
	m_Menu.Enable(CMD_PROPERTIES, m_File.Valid());

	m_Menu.Check(CMD_RULERS, m_Rulers);
	m_Menu.Check(CMD_PSEUDOCOLORS, m_Pseudocolors);
	m_Menu.Check(CMD_PHOTOMETRY, m_InfoMode == INFO_PHOTOMETRY);
	m_Menu.Check(CMD_GRAYSCALE, m_InfoMode == INFO_GRAYSCALE);
	m_Menu.Check(CMD_PROFILE, m_InfoMode == INFO_PROFILE);
	m_Menu.Check(CMD_HISTOGRAM, m_InfoMode == INFO_HISTOGRAM);
}

void CCCDFileDlg::UpdateZoom(void)
{
	double zoom, min, max;

	zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_Chart));
	cmpack_chart_view_get_zoom_limits(CMPACK_CHART_VIEW(m_Chart), &min, &max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), zoom < max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), zoom > min);
}

void CCCDFileDlg::UpdateImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ImageData) 
		g_object_unref(m_ImageData);
	m_ImageData = m_Image.ToImageData(m_Negative, m_Pseudocolors, true, m_RowsUpward);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Chart), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
}

void CCCDFileDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CCCDFileDlg *pMe = (CCCDFileDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CCCDFileDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_OPEN:
		Open();
		break;
	case CMD_SAVE_AS:
		SaveAs("FITS files", "*.fts");
		break;
	case CMD_CLOSE:
		Close();
		break;
	case CMD_PROPERTIES:
		ShowProperties();
		break;
	case CMD_EXPORT:
		ExportImage();
		break;

	// View menu
	case CMD_PSEUDOCOLORS:
		m_Pseudocolors = m_Menu.IsChecked(CMD_PSEUDOCOLORS);
		g_Project->SetBool("PreviewDlg", "Pseudocolors", m_Pseudocolors);
		UpdateImage();
		m_Scale.SetScaleParams(m_Image.BlackLevel(), m_Image.BlackLevel() + m_Image.Range(), 
			m_Pseudocolors, m_Negative);
		break;
	case CMD_RULERS:
		m_Rulers = m_Menu.IsChecked(CMD_RULERS);
		g_Project->SetBool("PreviewDlg", "Rulers", m_Rulers);
		cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_Chart), m_Rulers);
		break;

	// Edit menu
	case CMD_FLIP_H:
		HorizontalFlip();
		break;
	case CMD_FLIP_V:
		VerticalFlip();
		break;
		
	// Tools menu
	case CMD_GRAYSCALE:
		if (m_Menu.IsChecked(CMD_GRAYSCALE)) 
			SetInfoMode(INFO_GRAYSCALE);
		else if (m_InfoMode == INFO_GRAYSCALE)
			SetInfoMode(INFO_NONE);
		break;
	case CMD_PHOTOMETRY:
		if (m_Menu.IsChecked(CMD_PHOTOMETRY)) 
			SetInfoMode(INFO_PHOTOMETRY);
		else if (m_InfoMode == INFO_PHOTOMETRY) 
			SetInfoMode(INFO_NONE);
		break;
	case CMD_PROFILE:
		if (m_Menu.IsChecked(CMD_PROFILE))
			SetInfoMode(INFO_PROFILE);
		else if (m_InfoMode == INFO_PROFILE)
			SetInfoMode(INFO_NONE);
		break;
	case CMD_HISTOGRAM:
		if (m_Menu.IsChecked(CMD_HISTOGRAM))
			SetInfoMode(INFO_HISTOGRAM);
		else if (m_InfoMode == INFO_HISTOGRAM)
			SetInfoMode(INFO_NONE);
		break;

	// Help menu
	case CMD_SHOW_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_IMAGE_FILE_WINDOW);
		break;
	}
}

void CCCDFileDlg::ShowProperties(void)
{
	CCCDFileInfoDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.ShowModal(&m_File, m_Name, m_Path);
}

void CCCDFileDlg::button_clicked(GtkWidget *button, CCCDFileDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CCCDFileDlg::OnButtonClicked(GtkWidget *pBtn)
{
	double zoom;

	if (pBtn==GTK_WIDGET(m_ZoomIn)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_Chart));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_Chart), zoom + 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomOut)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_Chart));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_Chart), zoom - 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomFit)) {
		cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Chart), TRUE);
	}
}

void CCCDFileDlg::profile_changed(GtkWidget *pChart, CCCDFileDlg *pDlg)
{
	pDlg->OnProfileChanged();
}

//
// Profile changed
//
void CCCDFileDlg::OnProfileChanged(void)
{
	gint x0, y0, x1, y1;

	if (m_InfoMode == INFO_PROFILE) {
		if (cmpack_chart_view_profile_get_pos(CMPACK_CHART_VIEW(m_Chart), CHART_PROFILE_START, &x0, &y0) &&
			cmpack_chart_view_profile_get_pos(CMPACK_CHART_VIEW(m_Chart), CHART_PROFILE_END, &x1, &y1)) {
				m_Profile.SetProfile(m_Image, x0, y0, x1, y1);
		} else
			m_Profile.Clear();
	}
}

void CCCDFileDlg::mouse_moved(GtkWidget *button, CCCDFileDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
	pDlg->m_MouseOnProfile = false;
	pDlg->m_MouseOnHistogram = false;
}

void CCCDFileDlg::zoom_changed(GtkWidget *button, CCCDFileDlg *pDlg)
{
	pDlg->m_UpdateZoom = true;
}

gboolean CCCDFileDlg::timer_cb(CCCDFileDlg *pDlg)
{
	if (pDlg->m_UpdatePos) {
		pDlg->m_UpdatePos = false;
		pDlg->UpdateStatus();
	}
	if (pDlg->m_UpdateZoom) {
		pDlg->m_UpdateZoom = false;
		pDlg->UpdateZoom();
	}
	return TRUE;
}

void CCCDFileDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CCCDFileDlg *pMe = (CCCDFileDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed((CInfoBox*)sender);
		break;
	case CInfoBox::CB_MOUSE_MOVED:
		pMe->m_UpdatePos = true;
		pMe->m_MouseOnProfile = (sender==&pMe->m_Profile);
		pMe->m_MouseOnHistogram = (sender==&pMe->m_Histogram);
		break;
	}
}

void CCCDFileDlg::OnInfoBoxClosed(CInfoBox *pBox)
{
	m_InfoMode = INFO_NONE;
	cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_Chart));
	cmpack_chart_view_profile_enable(CMPACK_CHART_VIEW(m_Chart), FALSE);
	UpdateControls();
}

void CCCDFileDlg::UpdateStatus(void)
{
	bool	ok;
	char	buf[256];
	int		x = -1, y = -1;
	double	pos_x;

	if (m_MouseOnProfile) 
		ok = m_Profile.GetMousePos(&x, &y);
	else if (m_MouseOnHistogram) {
		ok = m_Histogram.GetMousePos(&pos_x);
		x = RoundToInt(pos_x);
	} else
		ok = cmpack_chart_view_mouse_pos(CMPACK_CHART_VIEW(m_Chart), &x, &y)!=0;
	if (ok) {
		if (x!=m_LastPosX || y!=m_LastPosY) {
			m_LastPosX = x;
			m_LastPosY = y;
			if (m_MouseOnHistogram)
				sprintf(buf, "Cursor: Pixel value = %d ADU", x);
			else 
				sprintf(buf, "Cursor: X = %d, Y = %d, Pixel value = %.1f ADU", x, y, m_Image.getPixel(x, y));
			SetStatus(buf);
		}
	} else {
		if (m_LastPosX!=-1 || m_LastPosY!=-1) {
			m_LastPosX = m_LastPosY = -1;
			SetStatus(NULL);
		}
	}
}

gboolean CCCDFileDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CCCDFileDlg *pMe)
{
	if (event->button == 1 && pMe->m_InfoMode==INFO_PHOTOMETRY && 
		(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==0) {
		pMe->OnLButtonClick(event->time);
		return TRUE;
	}
	return FALSE;
}

//
// Left button click
//
void CCCDFileDlg::OnLButtonClick(guint32 timestamp)
{
	if (m_InfoMode==INFO_PHOTOMETRY) 
		m_QPhot.Update(&m_Image, m_LastPosX, m_LastPosY);
}

void CCCDFileDlg::SetInfoMode(tInfoMode mode)
{
	if (mode!=INFO_PROFILE) {
		cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_Chart));
		cmpack_chart_view_profile_enable(CMPACK_CHART_VIEW(m_Chart), FALSE);
	}
	if (mode==INFO_GRAYSCALE) {
		m_Scale.SetScaleParams(m_Image.BlackLevel(), m_Image.BlackLevel() + m_Image.Range(), 
			m_Pseudocolors, m_Negative);
	}
	if (mode==INFO_HISTOGRAM)
		m_Histogram.SetData(&m_Image);

	m_InfoMode = mode;
	switch (m_InfoMode) 
	{
	case INFO_GRAYSCALE:
		m_QPhot.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
		m_Scale.Show(true);
		break;
	case INFO_PHOTOMETRY:
		m_Scale.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
		m_QPhot.Show(true);
		break;
	case INFO_PROFILE:
		m_Scale.Show(false);
		m_QPhot.Show(false);
		m_Histogram.Show(false);
		m_Profile.Show(true);
		break;
	case INFO_HISTOGRAM:
		m_Scale.Show(false);
		m_QPhot.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(true);
		break;
	default:
		m_Scale.Show(false);
		m_QPhot.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
	}

	cmpack_chart_view_profile_enable(CMPACK_CHART_VIEW(m_Chart), m_InfoMode==INFO_PROFILE);
	UpdateControls();
}

void CCCDFileDlg::ExportImage(void)
{
	CChartExportDlg dlg(GTK_WINDOW(m_pDlg));
	gchar *basename = g_path_get_basename(m_Path);
	dlg.Execute(NULL, m_ImageData, basename, m_Negative, m_RowsUpward);
	g_free(basename);
}

bool CCCDFileDlg::SaveFile(const char *fpath, GError **error)
{
	CCCDFile dst;
	return dst.MakeCopy(m_File, error)
		&& dst.SetImageData(m_Image, error)
		&& dst.SaveAs(fpath, error);
}

void CCCDFileDlg::HorizontalFlip(void)
{
	m_Image.Transpose(true, false);
	UpdateImage();
	if (!m_NotSaved) {
		m_NotSaved = true;
		UpdateTitle();
		UpdateControls();
	}
}

void CCCDFileDlg::VerticalFlip(void)
{
	m_Image.Transpose(false, true);
	UpdateImage();
	if (!m_NotSaved) {
		m_NotSaved = true;
		UpdateTitle();
		UpdateControls();
	}
}

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

enum tFieldId
{
	FIELD_FILE,
	FIELD_FRAME,
	FIELD_FILENAME,
	FIELD_DIRPATH,
	FIELD_MAGIC,
	FIELD_DATETIME,
	FIELD_JULDAT,
	FIELD_FILTER,
	FIELD_EXPTIME,
	FIELD_CCDTEMP,
	FIELD_OBJECT,
	FIELD_OBJRA,
	FIELD_OBJDEC,
	FIELD_OBSERVATORY,
	FIELD_LONGITUDE,
	FIELD_LATITUDE,
	FIELD_OBSERVER,
	FIELD_TELESCOPE,
	FIELD_INSTRUMENT,
	FIELD_AVGFRAMES,
	FIELD_SUMFRAMES
};

//
// Constructor
//
CCCDFileInfoDlg::CCCDFileInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0),
	m_Name(NULL)
{
	GtkWidget *bbox;

	AddHeading(FIELD_FILE, 0, 0, "File");
	AddField(FIELD_FILENAME, 0, 1, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_DIRPATH, 0, 2, "Location", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_MAGIC, 0, 3, "Format");
	AddSeparator(0, 4);
	AddHeading(FIELD_FRAME, 0, 5, "Frame");
	AddField(FIELD_DATETIME, 0, 6, "Date and time (UTC)");
	AddField(FIELD_JULDAT, 0, 7, "Julian date");
	AddField(FIELD_FILTER, 0, 8, "Optical filter", PANGO_ELLIPSIZE_END);
	AddField(FIELD_EXPTIME, 0, 9, "Exposure duration");
	AddField(FIELD_CCDTEMP, 0, 10, "CCD temperature");
	AddField(FIELD_AVGFRAMES, 0, 11, "Subframes averaged");
	AddField(FIELD_SUMFRAMES, 0, 12, "Subframes summed");
	AddField(FIELD_OBJECT, 0, 13, "Object", PANGO_ELLIPSIZE_END);
	AddField(FIELD_OBJRA, 0, 14, "Right ascension", PANGO_ELLIPSIZE_NONE);
	AddField(FIELD_OBJDEC, 0, 15, "Declination", PANGO_ELLIPSIZE_NONE);
	AddField(FIELD_OBSERVATORY, 0, 16, "Observatory", PANGO_ELLIPSIZE_END);
	AddField(FIELD_LONGITUDE, 0, 17, "Longitude", PANGO_ELLIPSIZE_NONE);
	AddField(FIELD_LATITUDE, 0, 18, "Latitude", PANGO_ELLIPSIZE_NONE);
	AddField(FIELD_OBSERVER, 0, 19, "Observer", PANGO_ELLIPSIZE_END);
	AddField(FIELD_TELESCOPE, 0, 20, "Telescope", PANGO_ELLIPSIZE_END);
	AddField(FIELD_INSTRUMENT, 0, 21, "Instrument", PANGO_ELLIPSIZE_END);
	gtk_widget_show_all(m_Tab);

	// Separator
	gtk_box_pack_start(GTK_BOX(m_Box), gtk_label_new(NULL), FALSE, TRUE, 0);

	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_box_pack_start(GTK_BOX(m_Box), bbox, FALSE, TRUE, 0);
	m_HdrBtn = gtk_button_new_with_label("Show header");
	g_signal_connect(G_OBJECT(m_HdrBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_HdrBtn, FALSE, TRUE, 0);
	
	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

//
// Show dialog
//
void CCCDFileInfoDlg::ShowModal(CCCDFile *img, const gchar *name, const gchar *path)
{
	char buf[512];
	CmpackDateTime dt;

	m_Image = img;
	m_Name = name;

	// Window caption
	sprintf(buf, "%s - %s - %s", name, "properties", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Update properties
	gchar *basename = g_path_get_basename(path);
	SetField(FIELD_FILENAME, basename);
	g_free(basename);
	gchar *dirpath = g_path_get_dirname(path);
	SetField(FIELD_DIRPATH, dirpath);
	g_free(dirpath);

	SetField(FIELD_MAGIC, img->Magic());
	if (img->JulianDate()>0) {
		cmpack_decodejd(img->JulianDate(), &dt);
		sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
		SetField(FIELD_DATETIME, buf);
		SetField(FIELD_JULDAT, img->JulianDate(), JD_PREC);
	} else {
		SetField(FIELD_DATETIME, "");
		SetField(FIELD_JULDAT, "");
	}
	if (img->Filter())
		SetField(FIELD_FILTER, img->Filter());
	else
		SetField(FIELD_FILTER, "");
	if (img->ExposureDuration()>0)
		SetField(FIELD_EXPTIME, img->ExposureDuration(), 3, "second(s)");
	else
		SetField(FIELD_EXPTIME, "");
	if (img->CCDTemperature()>-999 && img->CCDTemperature()<999)
		SetField(FIELD_CCDTEMP, img->CCDTemperature(), 2, "\xC2\xB0""C");
	else
		SetField(FIELD_CCDTEMP, "");
	if (img->AvgFrames()>0)
		SetField(FIELD_AVGFRAMES, img->AvgFrames(), "frame(s)");
	else
		SetField(FIELD_AVGFRAMES, "");
	if (img->SumFrames()>0)
		SetField(FIELD_AVGFRAMES, img->SumFrames(), "frame(s)");
	else
		SetField(FIELD_AVGFRAMES, "");
	if (img->Object()->Name())
		SetField(FIELD_OBJECT, img->Object()->Name());
	else
		SetField(FIELD_OBJECT, "");
	if (img->Object()->RA())
		SetField(FIELD_OBJRA, img->Object()->RA());
	else
		SetField(FIELD_OBJRA, "");
	if (img->Object()->Dec())
		SetField(FIELD_OBJDEC, img->Object()->Dec());
	else
		SetField(FIELD_OBJDEC, "");
	if (img->Location()->Name())
		SetField(FIELD_OBSERVATORY, img->Location()->Name());
	else
		SetField(FIELD_OBSERVATORY, "");
	if (img->Location()->Lon())
		SetField(FIELD_LONGITUDE, img->Location()->Lon());
	else
		SetField(FIELD_LONGITUDE, "");
	if (img->Location()->Lat())
		SetField(FIELD_LATITUDE, img->Location()->Lat());
	else
		SetField(FIELD_LATITUDE, "");
	if (img->Observer())
		SetField(FIELD_OBSERVER, img->Observer());
	else
		SetField(FIELD_OBSERVER, "");
	if (img->Telescope())
		SetField(FIELD_TELESCOPE, img->Telescope());
	else
		SetField(FIELD_TELESCOPE, "");
	if (img->Instrument())
		SetField(FIELD_INSTRUMENT, img->Instrument());
	else
		SetField(FIELD_INSTRUMENT, "");

	CInfoDlg::ShowModal();
}

void CCCDFileInfoDlg::button_clicked(GtkWidget *button, CCCDFileInfoDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CCCDFileInfoDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_HdrBtn))
		ShowHeader();
}

void CCCDFileInfoDlg::ShowHeader(void)
{
	CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(m_Image, m_Name);
}
