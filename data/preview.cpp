/**************************************************************

preview.cpp (C-Munipack project)
Preview widget
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

#include "preview.h"
#include "utils.h"
#include "main.h"
#include "configuration.h"
#include "configuration.h"
#include "ccdfile_dlg.h"
#include "phtfile_dlg.h"
#include "table_dlg.h"
#include "catfile_class.h"
#include "image_class.h"

//
// Constructor
//
CPreview::CPreview():m_Path(NULL), m_Data(NULL), m_FileType(TYPE_UNKNOWN), 
	m_MinX(0.0), m_MaxX(0.0), m_EpsX(0.0), m_MinY(0.0), m_MaxY(0.0), m_EpsY(0.0),
	m_ReverseY(FALSE)
{
	GtkWidget *vbox;

	m_Buffer = gtk_text_buffer_new(NULL);

	// Frame
	m_Widget = gtk_frame_new(NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(m_Widget), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Image area
	m_Preview = cmpack_preview_new();
	gtk_widget_set_size_request(m_Preview, 160, 160);
	gtk_box_pack_start(GTK_BOX(vbox), m_Preview, FALSE, TRUE, 0);
	gtk_widget_set_events(m_Preview, GDK_BUTTON_PRESS_MASK);
	gtk_widget_set_tooltip_text(m_Preview, "Double click to enlarge the preview");
	g_signal_connect(G_OBJECT(m_Preview), "button_press_event", G_CALLBACK(button_press_event), this);

	// Text area
	m_View = gtk_text_view_new_with_buffer(m_Buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_View), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_View), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_View), false);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_View), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), m_View, TRUE, TRUE, 0);

	gtk_widget_show_all(m_Widget);
}

//
// Destructor
//
CPreview::~CPreview()
{
	if (m_Data)
		g_object_unref(m_Data);
	g_free(m_Path);
	gtk_widget_destroy(m_Widget);
	g_object_unref(m_Buffer);
}


//
// Clear all
//
void CPreview::Clear(void)
{
	cmpack_preview_set_model(CMPACK_PREVIEW(m_Preview), NULL);
	cmpack_preview_set_text(CMPACK_PREVIEW(m_Preview), NULL);
	if (m_Data) {
		g_object_unref(m_Data);
		m_Data = NULL;
	}
	g_free(m_Path);
	m_Path = NULL;
	m_FileType = TYPE_UNKNOWN;
	m_MinX = m_MaxX = m_EpsX = m_MinY = m_MaxY = m_EpsY = 0.0;
	m_ReverseY = FALSE;
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_View)), "", -1);
}

//
// Set file path
//
void CPreview::SetPath(const char *path)
{
	if (path) {
		int len = (int)strlen(path);
		m_Path = (char*)g_realloc(m_Path, (len+1)*sizeof(char));
		strcpy(m_Path, path);
	} else {
		g_free(m_Path);
		m_Path = NULL;
	}
}


//
// Load image file
//
void CPreview::LoadImage(const char *filename)
{
	bool image_invert, rows_upward;
	double exptime, ccdtemp;
	CmpackDateTime dt;
	char msg[1024];
	const char *filter, *object, *observer;
	GtkTextIter ptr;
	CCCDFile ccd;
	CImage img;

	if (!ccd.Open(filename, CMPACK_OPEN_READONLY) || !ccd.GetImageData(img))
		return;

	// Update image
	image_invert = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	cmpack_preview_set_negative(CMPACK_PREVIEW(m_Preview), image_invert);
	rows_upward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	cmpack_preview_set_image_orientation(CMPACK_PREVIEW(m_Preview), rows_upward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	m_Data = G_OBJECT(img.ToImageData(image_invert, false, false, rows_upward));
	m_FileType = TYPE_IMAGE;
	// Update info
	gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
	if (ccd.DateTime(&dt)) {
		sprintf(msg, "Date: %04d-%02d-%02d\nTime: %d:%02d:%02d",
			dt.date.year, dt.date.month, dt.date.day, 
			dt.time.hour, dt.time.minute, dt.time.second);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	} else {
		gtk_text_buffer_insert(m_Buffer, &ptr, "Invalid date or time", -1);
	}
	filter = ccd.Filter();
	if (filter) {
		sprintf(msg, "\nFilter: %s", filter);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
	exptime = ccd.ExposureDuration();
	if (exptime>0) {
		sprintf(msg, "\nExp. duration: %.3f s", exptime);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
	ccdtemp = ccd.CCDTemperature();
	if (ccdtemp > -999 && ccdtemp < 999) {
		sprintf(msg, "\nCCD temperature: %.1f \xC2\xB0""C", ccdtemp);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
	object = ccd.Object()->Name();
	if (object) {
		sprintf(msg, "\nObject: %s", object);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
	observer = ccd.Location()->Name();
	if (observer) {
		sprintf(msg, "\nObserver: %s", observer);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
	}
}

//
// Load photometry file
//
void CPreview::LoadChart(const char *filename)
{
	CmpackDateTime dt;
	char msg[1024];
	GtkTextIter ptr;

	CPhot pht;
	if (pht.Load(filename) && pht.ApertureCount()>0) {
		bool chart_invert = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
		cmpack_preview_set_negative(CMPACK_PREVIEW(m_Preview), chart_invert);
		bool rows_upward = CConfig::GetBool(CConfig::ROWS_UPWARD);
		cmpack_preview_set_image_orientation(CMPACK_PREVIEW(m_Preview), rows_upward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
		pht.SelectAperture(0);
		// Aperture 
		CmpackChartData *data = pht.ToChartData(0, pht.Matched());
		if (data) {
			m_Data = G_OBJECT(data);
			m_FileType = TYPE_PHOT;
			// Update info
			gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
			if (pht.JulianDate()>0) {
				cmpack_decodejd(pht.JulianDate(), &dt);
				sprintf(msg, "Date: %04d-%02d-%02d\nTime: %d:%02d:%02d",
					dt.date.year, dt.date.month, dt.date.day, 
					dt.time.hour, dt.time.minute, dt.time.second);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			} else {
				gtk_text_buffer_insert(m_Buffer, &ptr, "Invalid date or time", -1);
			}
			if (pht.Filter()) {
				sprintf(msg, "\nFilter: %s", pht.Filter());
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
			if (pht.ExposureDuration()>=0) {
				sprintf(msg, "\nExp. duration: %.3f s", pht.ExposureDuration());
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
			if (pht.ObjectCount()>0) {
				sprintf(msg, "\nNumber of stars: %d", pht.ObjectCount());
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
		}
	}
}

//
// Load catalog file
//
void CPreview::LoadCatalog(const char *filename)
{
	int stars;
	double jd, exptime;
	CmpackDateTime dt;
	char msg[1024];
	const char *filter;
	GtkTextIter ptr;
	CCatalog cat;

	if (cat.Load(filename)) {
		bool chart_invert = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
		cmpack_preview_set_negative(CMPACK_PREVIEW(m_Preview), chart_invert);
		bool rows_upward = CConfig::GetBool(CConfig::ROWS_UPWARD);
		cmpack_preview_set_image_orientation(CMPACK_PREVIEW(m_Preview), rows_upward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
		cat.SelectSelection(0);
		CmpackChartData *data = cat.ToChartData(true, true, false);
		if (data) {
			m_Data = G_OBJECT(data);
			m_FileType = TYPE_CAT;
			// Update info
			gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
			jd = cat.JulianDate();
			if (jd>0) {
				cmpack_decodejd(jd, &dt);
				sprintf(msg, "Date: %04d-%02d-%02d\nTime: %d:%02d:%02d",
					dt.date.year, dt.date.month, dt.date.day, 
					dt.time.hour, dt.time.minute, dt.time.second);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			} else {
				gtk_text_buffer_insert(m_Buffer, &ptr, "Invalid date or time", -1);
			}
			filter = cat.Filter();
			if (filter && *filter!='\0') {
				sprintf(msg, "\nFilter: %s", filter);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
			exptime = cat.ExposureDuration();
			if (exptime>0) {
				sprintf(msg, "\nExp. duration: %.3f s", exptime);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
			stars = cat.ObjectCount();
			if (stars>0) {
				sprintf(msg, "\nNumber of stars: %d", stars);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
		}
	}
}

//
// Load table
//
void CPreview::LoadTable(const char *filename)
{
	CTable tab;
	int x, y, aperture;
	char msg[1024];
	const char *filter;
	GtkTextIter ptr;

	if (tab.Load(filename)) {
		switch (tab.Type())
		{
		case CMPACK_TABLE_LCURVE_DIFF:
		case CMPACK_TABLE_LCURVE_INST:
			x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD);
			if (x<0)
				x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD_HEL);
			y = tab.ChannelsY()->FindFirst(CChannel::DATA_MAGNITUDE);
			m_EpsX = 0.01;
			m_EpsY = 0.05;
			break;

		case CMPACK_TABLE_TRACKLIST:
			x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD);
			if (x<0)
				x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD_HEL);
			y = tab.ChannelsY()->FindFirst(CChannel::DATA_OFFSET);
			m_EpsX = 0.01;
			m_EpsY = 1.0;
			break;

		case CMPACK_TABLE_AIRMASS:
			x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD);
			if (x<0)
				x = tab.ChannelsX()->FindFirst(CChannel::DATA_JD_HEL);
			y = tab.ChannelsY()->FindFirst(CChannel::DATA_AIRMASS);
			if (y<0)
				y = tab.ChannelsY()->FindFirst(CChannel::DATA_ALTITUDE);
			m_EpsX = 0.01;
			m_EpsY = 0.5;
			break;

		case CMPACK_TABLE_APERTURES:
			x = tab.ChannelsX()->FindFirst(CChannel::DATA_APERTURE_ID);
			y = tab.ChannelsY()->FindFirst(CChannel::DATA_DEVIATION);
			m_EpsX = 1.0;
			m_EpsY = 0.05;
			break;

		case CMPACK_TABLE_MAGDEV:
			x = tab.ChannelsX()->FindFirst(CChannel::DATA_MAGNITUDE);
			y = tab.ChannelsY()->FindFirst(CChannel::DATA_DEVIATION);
			m_EpsX = 0.05;
			m_EpsY = 0.05;
			break;

		default:
			x = y = 0;
			m_EpsX = 0.05;
			m_EpsY = 0.05;
			break;
		}
		m_MinX = tab.ChannelsX()->GetMin(x);
		m_MaxX = tab.ChannelsX()->GetMax(x);
		m_MinY = tab.ChannelsY()->GetMin(y);
		m_MaxY = tab.ChannelsY()->GetMax(y);
		m_ReverseY = (tab.Type() == CMPACK_TABLE_LCURVE_DIFF || tab.Type() == CMPACK_TABLE_LCURVE_INST);
		cmpack_preview_set_x_axis(CMPACK_PREVIEW(m_Preview), FALSE, FALSE, m_MinX, m_MaxX, m_EpsX);
		cmpack_preview_set_y_axis(CMPACK_PREVIEW(m_Preview), FALSE, m_ReverseY, m_MinY, m_MaxY, m_EpsY);

		m_Data = G_OBJECT(tab.ToGraphData(x, y));
		m_FileType = TYPE_TABLE;
		// Update info
		gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
		switch (tab.Type())
		{
		case CMPACK_TABLE_LCURVE_DIFF:
		case CMPACK_TABLE_LCURVE_INST:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Light curve", -1);
			break;
		case CMPACK_TABLE_TRACKLIST:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Track curve", -1);
			break;
		case CMPACK_TABLE_APERTURES:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Apertures", -1);
			break;
		case CMPACK_TABLE_MAGDEV:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Mag-dev curve", -1);
			break;
		case CMPACK_TABLE_AIRMASS:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Air mass curve", -1);
			break;
		default:
			gtk_text_buffer_insert(m_Buffer, &ptr, "Table", -1);
			break;
		}
		filter = tab.Filter();
		if (filter && *filter!='\0') {
			sprintf(msg, "\nFilter: %s", filter);
			gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		}
		aperture = tab.Aperture();
		if (aperture>0) {
			sprintf(msg, "\nAperture: %d", aperture);
			gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		}
	}
}

//
// Load Munifind table
//
void CPreview::LoadVarFind(const char *filename)
{
	gtk_text_buffer_set_text(m_Buffer, "Varfind data file\n(no preview available)", -1);
	m_FileType = TYPE_PROFILE;
}

//
// Load project file
//
void CPreview::LoadProject(const char *filename)
{
	char buf[1024];
	CProject prj;
	CmpackDateTime dt;
	GtkTextIter ptr;
	CImage img;

	if (prj.Open(filename, CProject::OPEN_READONLY, NULL, NULL)) {
		gtk_text_buffer_set_text(m_Buffer, "C-Munipack project", -1);
		gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
		const char *typestr = ProjectTypeCaption(prj.ProjectType());
		if (typestr) {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\nType: ", -1);
			gtk_text_buffer_insert(m_Buffer, &ptr, typestr, -1);
		}
		CObjectCoords obj = *prj.ObjectCoords();
		if (obj.Name()) {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\nObject: ", -1);
			gtk_text_buffer_insert(m_Buffer, &ptr, obj.Name(), -1);
		}
		CLocation loc = *prj.Location();
		if (loc.Name()) {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\nObject: ", -1);
			gtk_text_buffer_insert(m_Buffer, &ptr, loc.Name(), -1);
		}
		int nframes = prj.GetFileCount();
		if (nframes>0) {
			sprintf(buf, "%d", nframes);
			gtk_text_buffer_insert(m_Buffer, &ptr, "\nFrames: ", -1);
			gtk_text_buffer_insert(m_Buffer, &ptr, buf, -1);
		} else {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\nEmpty project", -1);
		}
		GtkTreePath *first = prj.GetFirstFile();
		if (first) {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\n\nFirst frame: ", -1);
			double jd = prj.GetJulDate(first);
			if (jd>0 && cmpack_decodejd(jd, &dt)==0) {
				sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
							dt.time.hour, dt.time.minute, dt.time.second);
				gtk_text_buffer_insert(m_Buffer, &ptr, "\nDate & time: ", -1);
				gtk_text_buffer_insert(m_Buffer, &ptr, buf, -1);
			}
			gchar *filter = prj.GetColorFilter(first);
			if (filter && filter[0]!='\0') {
				gtk_text_buffer_insert(m_Buffer, &ptr, "\nFilter: ", -1);
				gtk_text_buffer_insert(m_Buffer, &ptr, filter, -1);
			}
			g_free(filter);
			double exposure = prj.GetExposure(first);
			if (exposure>=0) {
				sprintf(buf, "%.3f s", exposure);
				gtk_text_buffer_insert(m_Buffer, &ptr, "\nExposure: ", -1);
				gtk_text_buffer_insert(m_Buffer, &ptr, buf, -1);
			}
			// Show the first frame in the preview
			bool image_invert = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
			cmpack_preview_set_negative(CMPACK_PREVIEW(m_Preview), image_invert);
			bool rows_upward = CConfig::GetBool(CConfig::ROWS_UPWARD);
			cmpack_preview_set_image_orientation(CMPACK_PREVIEW(m_Preview), rows_upward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
			gchar *ftemp = prj.GetImageFile(first);
			if (ftemp && img.Load(ftemp)) {
				m_Data = G_OBJECT(img.ToImageData(image_invert, false, false, rows_upward));
			} else {
				gchar *forig = prj.GetSourceFile(first);
				if (forig && img.Load(forig))
					m_Data = G_OBJECT(img.ToImageData(image_invert, false, false, rows_upward));
				g_free(forig);
			}
			g_free(ftemp);
		} else {
			gtk_text_buffer_insert(m_Buffer, &ptr, "\n(no preview available)", -1);
		}
		m_FileType = TYPE_PROJECT;
	}
}

//
// Profile file
//
void CPreview::LoadProfile(const char *filename)
{
	gtk_text_buffer_set_text(m_Buffer, "C-Munipack profile\n(no preview available)", -1);
	m_FileType = TYPE_PROFILE;
}

void CPreview::Update(const char *fpath)
{
	cmpack_preview_set_model(CMPACK_PREVIEW(m_Preview), NULL);
	cmpack_preview_set_text(CMPACK_PREVIEW(m_Preview), NULL);
	gtk_text_buffer_set_text(m_Buffer, "", -1);
	if (m_Data) {
		g_object_unref(m_Data);
		m_Data = NULL;
	}
	m_FileType = TYPE_UNKNOWN;
	if (fpath) {
		SetPath(fpath);
		switch (FileType(fpath))
		{
		case TYPE_IMAGE:
			LoadImage(fpath);
			break;
		case TYPE_PHOT:
			LoadChart(fpath);
			break;
		case TYPE_CAT:
			LoadCatalog(fpath);
			break;
		case TYPE_TABLE:
			LoadTable(fpath);
			break;
		case TYPE_VARFIND:
			LoadVarFind(fpath);
			break;
		case TYPE_PROJECT:
			LoadProject(fpath);
			break;
		case TYPE_PROFILE:
			LoadProfile(fpath);
			break;
		default:
			break;
		}
		if (m_Data)
			cmpack_preview_set_model(CMPACK_PREVIEW(m_Preview), m_Data);
	}
}

gint CPreview::button_press_event(GtkWidget *widget, GdkEventButton *event, CPreview *pMe)
{
	pMe->OnButtonPress(widget, event);
	return true;
}

void CPreview::OnButtonPress(GtkWidget *widget, GdkEventButton *event)
{
	if (event->type==GDK_2BUTTON_PRESS && event->button==1 && m_Path) {
		// Left double click
		ShowFileModal();
	}
}

//
// Get top level widget
//
GtkWidget *CPreview::GetTopLevel(void)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(m_Widget);
	if (GTK_WIDGET_TOPLEVEL(toplevel)) 
		return toplevel;
	return NULL;		
}

//
// Open preview in standalone window
//
void CPreview::ShowFileModal(void)
{
	GdkRectangle rc;
	char buf[FILENAME_MAX+128];
	GtkWindow *pParent = GTK_WINDOW(GetTopLevel());

	if (m_FileType!=TYPE_IMAGE && m_FileType!=TYPE_CAT && m_FileType!=TYPE_PHOT && m_FileType!=TYPE_TABLE)
		return;

	// Dialog with buttons
	GtkWidget *pDlg = gtk_dialog_new_with_buttons("", pParent,
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(pDlg));

	// Dialog size
	GdkScreen *scr = gtk_window_get_screen(pParent);
	int mon = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(pParent)->window);
	gdk_screen_get_monitor_geometry(scr, mon, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(pDlg), RoundToInt(0.7*rc.width), RoundToInt(0.7*rc.height));

	// Dialog icon
	char *icon = get_icon_file("preview");
	gtk_window_set_icon(GTK_WINDOW(pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Preview widget
	GtkWidget *preview = cmpack_preview_new_with_model(m_Data);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pDlg)->vbox), preview, TRUE, TRUE, 0);
	if (m_FileType == TYPE_TABLE) {
		cmpack_preview_set_x_axis(CMPACK_PREVIEW(preview), FALSE, FALSE, m_MinX, m_MaxX, m_EpsX);
		cmpack_preview_set_y_axis(CMPACK_PREVIEW(preview), FALSE, m_ReverseY, m_MinY, m_MaxY, m_EpsY);
	}
	bool chart_invert = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	cmpack_preview_set_negative(CMPACK_PREVIEW(preview), chart_invert);
	bool rows_upward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	cmpack_preview_set_image_orientation(CMPACK_PREVIEW(preview), rows_upward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);

	// Display the preview dialog
	gchar *basename = g_path_get_basename(m_Path);
	sprintf(buf, "%s - %s", basename, g_AppTitle);
	g_free(basename);
	gtk_window_set_title(GTK_WINDOW(pDlg), buf);
	gtk_widget_show_all(pDlg);
	gtk_dialog_run(GTK_DIALOG(pDlg));

	gtk_widget_destroy(pDlg);
}
