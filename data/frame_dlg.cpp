/**************************************************************

preview_dlg.cpp (C-Munipack project)
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
#include <math.h>

#include "project.h"
#include "frame_dlg.h"
#include "frameinfo_dlg.h"
#include "main.h"
#include "utils.h"
#include "configuration.h"
#include "configuration.h"
#include "profile.h"
#include "showheader_dlg.h"
#include "export_dlgs.h"
#include "ctxhelp.h"

enum tMenuId
{
	MENU_FRAME = 1,
	MENU_VIEW,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_CLOSE = 100,
	CMD_PRINT,
	CMD_EXPORT,
	CMD_PROPERTIES,
	CMD_REMOVE,
	CMD_GOTO_PREV,
	CMD_GOTO_NEXT,
	CMD_PSEUDOCOLORS,
	CMD_CALIBRATED,
	CMD_ORIGINAL,
	CMD_IMAGE,
	CMD_CHART,
	CMD_TABLE,
	CMD_RULERS,
	CMD_IMAGE_CHART,
	CMD_GRAYSCALE,
	CMD_PROFILE,
	CMD_HISTOGRAM,
	CMD_INSPECTOR,
	CMD_PHOTOMETRY,
	CMD_SHOW_HELP
};

//-------------------------   MENU FOR PREVIEW WINDOW   ---------------------------

static const CMenuBar::tMenuItem FrameMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_PROPERTIES,		"Show _Properties" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_REMOVE,			"_Remove from project" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT,			"_Export" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_CLOSE,			"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN, CMD_IMAGE,			"_Image only" },
	{ CMenuBar::MB_RADIOBTN, CMD_CHART,			"_Chart only" },
	{ CMenuBar::MB_RADIOBTN, CMD_IMAGE_CHART,	"I_mage and chart" },
	{ CMenuBar::MB_RADIOBTN, CMD_TABLE,			"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_RADIOBTN, CMD_CALIBRATED,	"C_alibrated image" },
	{ CMenuBar::MB_RADIOBTN, CMD_ORIGINAL,		"_Original image" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN, CMD_PSEUDOCOLORS,	"_Pseudo-color image" },
	{ CMenuBar::MB_CHECKBTN, CMD_RULERS,		"_Rulers" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_RADIOBTN, CMD_GRAYSCALE,		"_Gray scale" },
	{ CMenuBar::MB_RADIOBTN, CMD_INSPECTOR,		"_Object inspector" },
	{ CMenuBar::MB_RADIOBTN, CMD_PHOTOMETRY,	"Quick _photometry" },
	{ CMenuBar::MB_RADIOBTN, CMD_PROFILE,		"Pro_file" },
	{ CMenuBar::MB_RADIOBTN, CMD_HISTOGRAM,		"_Histogram" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_SHOW_HELP,		"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu PreviewMenu[] = {
	{ "_Frame", MENU_FRAME,	FrameMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   PREVIEW DIALOG   --------------------------------

CFrameDlg::CFrameDlg(void):m_ChartData(NULL), m_ImageData(NULL), m_TableData(NULL),
	m_pFile(NULL), m_InfoMode(INFO_NONE), m_SortColumnId(-1), m_SortType(GTK_SORT_ASCENDING), 
	m_SortCol(NULL), m_ApertureIndex(-1), m_SelectedRow(-1), m_SelectedObjId(-1), 
	m_SelectedRefId(-1), m_DontClose(false), m_Updating(false), m_UpdatePos(true), 
	m_MouseOnProfile(false), m_MouseOnHistogram(false), m_UpdateZoom(true), 
	m_LastPosX(-1), m_LastPosY(-1), m_LastFocus(-1), m_FrameState(0), m_StatusCtx(-1), 
	m_StatusMsg(-1), m_LayerId(-1), m_SelectedPath(NULL)
{
	GtkWidget *tbox, *vbox, *hbox;
	GdkRectangle rc;

	g_MainWnd->RegisterFrameDlg(this);

	m_DispMode = (tDispMode)g_Project->GetInt("PreviewDlg", "Mode", DISP_FULL, 0, DISP_MODE_COUNT-1);
	m_Pseudocolors = g_Project->GetBool("PreviewDlg", "Pseudocolors");
	m_ShowOrig = g_Project->GetBool("PreviewDlg", "Original");
	m_Rulers = g_Project->GetBool("PreviewDlg", "Rulers");
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(m_pDlg), "destroy", G_CALLBACK(destroy), this);

	// Window size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.6*rc.width), RoundToInt(0.6*rc.height));
	gtk_window_set_position(GTK_WINDOW(m_pDlg), GTK_WIN_POS_CENTER);

	// Dialog icon
	char *icon = get_icon_file("preview");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Window layout
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_pDlg), vbox);

	// Menu bar
	m_Menu.Create(PreviewMenu, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(vbox), m_Menu.Handle(), false, false, 0);

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, false, false, 0);

	toolbar_new_label(tbox, "Frame");
	m_PrevBtn = toolbar_new_button_from_stock(tbox, GTK_STOCK_GO_BACK, "Go to the previous frame in the project");
	g_signal_connect(G_OBJECT(m_PrevBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_FrameEntry = toolbar_new_entry(tbox, "Current frame / number of frames", false);
	m_NextBtn = toolbar_new_button_from_stock(tbox, GTK_STOCK_GO_FORWARD, "Go to the next frame in the project");
	g_signal_connect(G_OBJECT(m_NextBtn), "clicked", G_CALLBACK(button_clicked), this);

	toolbar_new_separator(tbox);

	toolbar_new_label(tbox, "Zoom");
	m_ZoomFit = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_FIT, "Fit the frame to the window");
	g_signal_connect(G_OBJECT(m_ZoomFit), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomOut = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_OUT, "Zoom out");
	g_signal_connect(G_OBJECT(m_ZoomOut), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomIn = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_IN, "Zoom in");
	g_signal_connect(G_OBJECT(m_ZoomIn), "clicked", G_CALLBACK(button_clicked), this);

	m_AperSeparator = toolbar_new_separator(tbox);

	m_AperLabel = toolbar_new_label(tbox, "Aperture");
	m_AperCombo = toolbar_new_combo(tbox, "Aperture used to get brightness of the objects");
	g_signal_connect(G_OBJECT(m_AperCombo), "changed", G_CALLBACK(button_clicked), this);
	m_Apertures = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_AperCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_AperCombo), renderer, "text", 1);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	// Chart
	m_ChartView = cmpack_chart_view_new();
	cmpack_chart_view_set_mouse_control(CMPACK_CHART_VIEW(m_ChartView), TRUE);
	cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_SINGLE);
	cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DBLCLICK);
	cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_ChartView), m_Rulers);
	g_signal_connect(G_OBJECT(m_ChartView), "zoom-changed", G_CALLBACK(zoom_changed), this);
	g_signal_connect(G_OBJECT(m_ChartView), "mouse-moved", G_CALLBACK(mouse_moved), this);
	g_signal_connect(G_OBJECT(m_ChartView), "selection-changed", G_CALLBACK(selection_changed), this);
	g_signal_connect(G_OBJECT(m_ChartView), "profile-changed", G_CALLBACK(profile_changed), this);
	g_signal_connect(G_OBJECT(m_ChartView), "item-activated", G_CALLBACK(item_activated), this);
	g_signal_connect(G_OBJECT(m_ChartView), "button-press-event", G_CALLBACK(button_press_event), this);
	m_ChartScrWnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_ChartScrWnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_ChartScrWnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_ChartScrWnd), m_ChartView);
	gtk_box_pack_start(GTK_BOX(hbox), m_ChartScrWnd, TRUE, TRUE, 0);

	// Table
	m_TableView = gtk_tree_view_new_with_model(NULL);
	m_TableScrWnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_TableScrWnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_TableScrWnd), GTK_SHADOW_ETCHED_IN);
	g_signal_connect(G_OBJECT(m_TableView), "row-activated", G_CALLBACK(table_row_activated), this);
	gtk_container_add(GTK_CONTAINER(m_TableScrWnd), m_TableView);
	gtk_box_pack_start(GTK_BOX(hbox), m_TableScrWnd, TRUE, TRUE, 0);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(m_TableView), TRUE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_TableView), TRUE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Object inspector
	m_InfoBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_InfoBox.Handle(), FALSE, FALSE, 0);
	m_InfoBox.SetCaption("Object inspector");

	// Quick photometry
	m_QPhot.RegisterCallback(InfoBoxCallback, this);
	m_QPhot.SetChart(CMPACK_CHART_VIEW(m_ChartView));
	gtk_box_pack_start(GTK_BOX(hbox), m_QPhot.Handle(), FALSE, FALSE, 0);

	// Scale
	m_Scale.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_Scale.Handle(), FALSE, FALSE, 0);

	// Profile
	m_Profile.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(vbox), m_Profile.Handle(), FALSE, FALSE, 0);

	// Histogram
	m_Histogram.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(vbox), m_Histogram.Handle(), FALSE, FALSE, 0);

	// Status bar
	m_Status = gtk_statusbar_new();
	gtk_box_pack_end(GTK_BOX(vbox), m_Status, FALSE, FALSE, 0);
	m_StatusCtx = gtk_statusbar_get_context_id(GTK_STATUSBAR(m_Status), "Main");

	// Timers
	m_TimerId = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 100, GSourceFunc(timer_cb), this, NULL);

	// Register callbacks for the model
	GtkTreeModel *pModel = g_Project->FileList();
	g_signal_connect(G_OBJECT(pModel), "row-deleted", G_CALLBACK(row_deleted), this);

	gtk_widget_show_all(vbox);
	SetDisplayMode(m_DispMode);
	SetInfoMode(INFO_NONE);
}

CFrameDlg::~CFrameDlg()
{
	// Unregister window in the main frame
	g_MainWnd->FrameDlgClosed(this);

	// Disconnect chart signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)zoom_changed, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)mouse_moved, this);

	// Disconnect project signals
	GtkTreeModel *pModel = g_Project->FileList();
	g_signal_handlers_disconnect_by_func(G_OBJECT(pModel), (gpointer)row_deleted, this);

	// Release objects, free allocated memory
	if (m_pFile)
		gtk_tree_row_reference_free(m_pFile);
	g_source_remove(m_TimerId);
	if (m_SelectedPath)
		gtk_tree_path_free(m_SelectedPath);
	if (m_Apertures)
		g_object_unref(m_Apertures);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_TableData)
		g_object_unref(m_TableData);
	if (m_ImageData)
		g_object_unref(m_ImageData);
}

bool CFrameDlg::LoadFrame(GtkTreePath *pPath)
{
	int id;
	char txt[128];
	gint *indices, count;
	GtkTreePath *pPrev, *pNext;

	if (m_pFile) {
		gtk_tree_row_reference_free(m_pFile);
		m_pFile = NULL;
		m_FrameState = 0;
		// Window caption
		sprintf(txt, "No frame - %s", g_AppTitle);
		gtk_window_set_title(GTK_WINDOW(m_pDlg), txt);
		// Frame indicator
		gtk_entry_set_text(GTK_ENTRY(m_FrameEntry), "");
		gtk_widget_set_sensitive(GTK_WIDGET(m_PrevBtn), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(m_NextBtn), FALSE);
	}
	if (pPath) {
		m_pFile = gtk_tree_row_reference_new(g_Project->FileList(), pPath);
		m_FrameState = g_Project->GetState(pPath);
		// Window caption
		id = g_Project->GetFrameID(pPath);
		sprintf(txt, "Frame #%d - %s", id, g_AppTitle);
		gtk_window_set_title(GTK_WINDOW(m_pDlg), txt);
		// Frame indicator
		pPath = gtk_tree_row_reference_get_path(m_pFile);
		if (pPath) {
			indices = gtk_tree_path_get_indices(pPath);
			count = g_Project->GetFileCount();
			sprintf(txt, "%d/%d", indices[0]+1, count);
			gtk_entry_set_text(GTK_ENTRY(m_FrameEntry), txt);
			gtk_widget_set_sensitive(GTK_WIDGET(m_FrameEntry), true);
			pPrev = g_Project->GetPreviousFile(pPath);
			gtk_widget_set_sensitive(GTK_WIDGET(m_PrevBtn), pPrev!=NULL);
			gtk_tree_path_free(pPrev);
			pNext = g_Project->GetNextFile(pPath);
			gtk_widget_set_sensitive(GTK_WIDGET(m_NextBtn), pNext!=NULL);
			gtk_tree_path_free(pNext);
			gtk_tree_path_free(pPath);
		}
	}
	ReloadImage();
	ReloadChart();
	if (!m_Image.Valid()) {
		if (m_DispMode==DISP_FULL || m_DispMode==DISP_IMAGE)
			SetDisplayMode(m_Phot.Valid() ? DISP_CHART : DISP_EMPTY);
	} else {
		if (m_DispMode==DISP_EMPTY)
			SetDisplayMode(DISP_IMAGE);
	}
	if (!m_Phot.Valid()) {
		if (m_DispMode==DISP_FULL || m_DispMode==DISP_CHART || m_DispMode==DISP_TABLE)
			SetDisplayMode(m_Image.Valid() ? DISP_IMAGE : DISP_EMPTY);
	} else {
		if (m_DispMode==DISP_EMPTY)
			SetDisplayMode(DISP_CHART);
	}
	if (!m_ShowOrig && (m_FrameState & CFILE_CONVERSION)==0) 
		m_ShowOrig = true;
	UpdateInfoBox();
	UpdateStatus();
	UpdateZoom();
	UpdateControls();
	return m_pFile!=NULL;
}

void CFrameDlg::ReloadImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), NULL);
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	m_Image.Clear();
	if (m_pFile) {
		GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
		if (pPath) {
			if (m_ShowOrig || (m_FrameState & CFILE_CONVERSION)==0) {
				// Original source file
				gchar *fpath = g_Project->GetSourceFile(pPath);
				if (fpath) {
					CCCDFile file;
					if (file.Open(fpath, CMPACK_OPEN_READONLY)) {
						file.GetImageData(m_Image, CMPACK_BITPIX_AUTO);
						m_Image.TransformOrigFrame();
						m_QPhot.Clear();
						m_QPhot.SetParams(file);
						m_Profile.Clear();
						cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_ChartView));
						SetInfoMode(m_InfoMode);
						UpdateApertures();
						UpdateImage();
						UpdateStatus();
						UpdateInfoBox();
					}
					g_free(fpath);
				}
			} else {
				// Calibrated frame
				gchar *fpath = g_Project->GetImageFile(pPath);
				if (fpath) {
					CCCDFile file;
					if (file.Open(fpath, CMPACK_OPEN_READONLY)) {
						file.GetImageData(m_Image, CMPACK_BITPIX_AUTO);
						m_QPhot.Clear();
						m_QPhot.SetParams(file);
						m_Profile.Clear();
						cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_ChartView));
						SetInfoMode(m_InfoMode);
						UpdateApertures();
						UpdateImage();
						UpdateStatus();
						UpdateInfoBox();
					}
					g_free(fpath);
				}
			}
			gtk_tree_path_free(pPath);
		}
	}
}

void CFrameDlg::ReloadChart(void)
{
	m_Updating = true;

	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), NULL);
	if (m_TableData) {
		g_object_unref(m_TableData);
		m_TableData = NULL;
	}
	if (m_SelectedPath) {
		gtk_tree_path_free(m_SelectedPath);
		m_SelectedPath = NULL;
	}
	m_Phot.Clear();
	m_SelectedRow = m_SelectedObjId = -1;

	if (m_pFile && (m_FrameState & CFILE_PHOTOMETRY)) {
		GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
		if (pPath) {
			gchar *fpath = g_Project->GetPhotFile(pPath);
			if (fpath && m_Phot.Load(fpath)) {
				if (m_SelectedRefId>=0) {
					int i = m_Phot.FindObjectRefID(m_SelectedRefId);
					m_SelectedObjId = m_Phot.GetObjectID(i);
				}
				UpdateTableHeader();
				UpdateApertures();
				UpdateChart();
				UpdateStatus();
				UpdateInfoBox();
			}
			g_free(fpath);
			gtk_tree_path_free(pPath);
		}
	}

	m_Updating = false;
}

// Update list of apertures
void CFrameDlg::UpdateApertures(void)
{
	char txt[128];
	GtkTreeIter iter;

	if (m_FrameState & CFILE_PHOTOMETRY)
		m_Aper = *m_Phot.Apertures();
	else
		m_Aper = g_Project->Profile()->Apertures();

	gtk_combo_box_set_model(GTK_COMBO_BOX(m_AperCombo), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_Aper.Count(); i++) {
		const CAperture *aper = m_Aper.Get(i);
		sprintf(txt, "#%d (%.2f)", aper->Id(), aper->Radius());
		gtk_list_store_append(m_Apertures, &iter);
		gtk_list_store_set(m_Apertures, &iter, 0, i, 1, txt, -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_AperCombo), GTK_TREE_MODEL(m_Apertures));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_AperCombo), m_ApertureIndex);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_AperCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_AperCombo), 0);
			m_ApertureIndex = SelectedItem(GTK_COMBO_BOX(m_AperCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_AperCombo), -1);
		m_ApertureIndex = -1;
	}
	m_Phot.SelectAperture(m_ApertureIndex);
}

void CFrameDlg::UpdateControls(void)
{
	// Menu View
	m_Menu.Enable(CMD_CALIBRATED, (m_FrameState & CFILE_CONVERSION)!=0 && (m_DispMode==DISP_IMAGE || m_DispMode==DISP_FULL));
	m_Menu.Check(CMD_CALIBRATED, !m_ShowOrig);
	m_Menu.Enable(CMD_ORIGINAL, (m_FrameState & CFILE_CONVERSION)!=0 && (m_DispMode==DISP_IMAGE || m_DispMode==DISP_FULL));
	m_Menu.Check(CMD_ORIGINAL, m_ShowOrig);
	m_Menu.Enable(CMD_IMAGE, m_Image.Width()>0 && m_Image.Height()>0);
	m_Menu.Check(CMD_IMAGE, m_DispMode==DISP_IMAGE);
	m_Menu.Enable(CMD_CHART, m_Phot.Valid());
	m_Menu.Check(CMD_CHART, m_DispMode==DISP_CHART);
	m_Menu.Enable(CMD_TABLE, m_Phot.Valid());
	m_Menu.Check(CMD_TABLE, m_DispMode==DISP_TABLE);
	m_Menu.Enable(CMD_IMAGE_CHART, m_Image.Width()>0 && m_Image.Height()>0 && m_Phot.Valid());
	m_Menu.Check(CMD_IMAGE_CHART, m_DispMode==DISP_FULL);
	m_Menu.Enable(CMD_PSEUDOCOLORS, m_Image.Width()>0 && m_Image.Height()>0 && (m_DispMode==DISP_IMAGE || m_DispMode==DISP_FULL));
	m_Menu.Check(CMD_PSEUDOCOLORS, m_Pseudocolors);
	m_Menu.Check(CMD_RULERS, m_DispMode==DISP_IMAGE || m_DispMode==DISP_CHART || m_DispMode==DISP_FULL);
	m_Menu.Check(CMD_RULERS, m_Rulers);

	// Menu Tools
	m_Menu.Enable(CMD_INSPECTOR, m_Phot.Valid());
	m_Menu.Check(CMD_INSPECTOR, m_InfoMode == INFO_OBJECT);
	m_Menu.Enable(CMD_PHOTOMETRY, m_Image.Width()>0 && m_Image.Height()>0);
	m_Menu.Check(CMD_PHOTOMETRY, m_InfoMode == INFO_PHOTOMETRY);
	m_Menu.Enable(CMD_GRAYSCALE, m_Image.Width()>0 && m_Image.Height()>0);
	m_Menu.Check(CMD_GRAYSCALE, m_InfoMode == INFO_GRAYSCALE);
	m_Menu.Enable(CMD_PROFILE, m_Image.Width()>0 && m_Image.Height()>0);
	m_Menu.Check(CMD_PROFILE, m_InfoMode == INFO_PROFILE);
	m_Menu.Enable(CMD_HISTOGRAM, m_Image.Width()>0 && m_Image.Height()>0);
	m_Menu.Check(CMD_HISTOGRAM, m_InfoMode == INFO_HISTOGRAM);

	// Apertures
	bool ok = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1;
	gtk_widget_set_sensitive(m_AperCombo, m_DispMode!=DISP_IMAGE && ok);
	gtk_widget_set_sensitive(m_AperLabel, m_DispMode!=DISP_IMAGE && ok);

	// Zoom
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), m_DispMode==DISP_CHART || m_DispMode==DISP_FULL);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), m_DispMode==DISP_CHART || m_DispMode==DISP_FULL);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), m_DispMode==DISP_CHART || m_DispMode==DISP_FULL);
}

void CFrameDlg::UpdateImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), NULL);
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	if (m_DispMode == DISP_IMAGE || m_DispMode == DISP_FULL) {
		m_ImageData = m_Image.ToImageData(m_Negative, m_Pseudocolors, true, m_RowsUpward);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), m_ImageData);
	}
}

void CFrameDlg::UpdateTableHeader(void)
{
	m_SortCol = NULL;

	m_Phot.SetView(GTK_TREE_VIEW(m_TableView));

	// Enable sorting
	GList *list = gtk_tree_view_get_columns(GTK_TREE_VIEW(m_TableView));
	for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) {
		if (ptr->next) {		// Except the last empty column
			GtkTreeViewColumn *col = GTK_TREE_VIEW_COLUMN(ptr->data);
			gtk_tree_view_column_set_clickable(col, TRUE);
			g_signal_connect(G_OBJECT(col), "clicked", G_CALLBACK(table_column_clicked), this);
			if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(col), "sort-column-id")) == m_SortColumnId) {
				m_SortCol = col;
				gtk_tree_view_column_set_sort_order(col, m_SortType);
				gtk_tree_view_column_set_sort_indicator(col, TRUE);
			}
		}
	}
	g_list_free(list);
}

void CFrameDlg::UpdateChart(void)
{
	bool matched = (m_FrameState & CFILE_MATCHING)!=0;

	m_Updating = true;

	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), NULL);
	if (m_TableData) {
		g_object_unref(m_TableData);
		m_TableData = NULL;
	}
	if (m_SelectedPath) {
		gtk_tree_path_free(m_SelectedPath);
		m_SelectedPath = NULL;
	}
	m_SelectedRow = -1;

	if (m_DispMode != DISP_TABLE) {
		cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_ChartView), m_Negative);
		cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_ChartView), (m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS));
		if (m_DispMode == DISP_CHART) {
			m_ChartData = m_Phot.ToChartData(matched, m_DispMode==DISP_IMAGE);
			if (m_ChartData) 
				m_SelectedRow = cmpack_chart_data_find_item(m_ChartData, m_SelectedObjId);
			cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), m_ChartData);
			cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_SINGLE);
			cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DBLCLICK);
			cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), NULL);
			if (m_SelectedRow>=0)
				cmpack_chart_view_select(CMPACK_CHART_VIEW(m_ChartView), m_SelectedRow);
		} else
		if (m_DispMode == DISP_IMAGE) {
			cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_NONE);
			cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DISABLED);
			cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), m_ImageData);
		} else {
			m_ChartData = m_Phot.ToChartData(matched, m_DispMode==DISP_IMAGE);
			if (m_ChartData) 
				m_SelectedRow = cmpack_chart_data_find_item(m_ChartData, m_SelectedObjId);
			cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), m_ChartData);
			cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_SINGLE);
			cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DBLCLICK);
			cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_ChartView), m_ImageData);
			if (m_SelectedRow>=0)
				cmpack_chart_view_select(CMPACK_CHART_VIEW(m_ChartView), m_SelectedRow);
		}
	} else {
		m_TableData = m_Phot.ToTreeModel();
		if (m_TableData) {
			// Set current sorting rule
			if (m_SortColumnId>=0) 
				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(m_TableData), m_SortColumnId, m_SortType);
			// Set selected object
			if (m_SelectedObjId>=0) {
				GtkTreeIter iter;
				gboolean ok = gtk_tree_model_get_iter_first(m_TableData, &iter);
				while (ok) {
					int obj_id;
					gtk_tree_model_get(m_TableData, &iter, CPhot::COL_ID, &obj_id, -1);
					if (obj_id == m_SelectedObjId) {
						m_SelectedPath = gtk_tree_model_get_path(m_TableData, &iter);
						break;
					}
					ok = gtk_tree_model_iter_next(m_TableData, &iter);
				}
			}
			// Make the selected object visible
			gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), m_TableData);
			if (m_SelectedPath) {
				GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
				gtk_tree_selection_select_path(selection, m_SelectedPath);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_TableView), m_SelectedPath, NULL, 0, 0, 0);
			}
		}
	}

	m_Updating = false;
}

void CFrameDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CFrameDlg *pMe = (CFrameDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CFrameDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// Frame menu
	case CMD_CLOSE:
		gtk_widget_destroy(GTK_WIDGET(m_pDlg));
		break;
	case CMD_REMOVE:
		RemoveFromProject();
		break;
	case CMD_PROPERTIES:
		ShowProperties();
		break;
	case CMD_EXPORT:
		Export();
		break;

	// Navigate menu
	case CMD_GOTO_PREV:
		GoToPreviousFrame();
		break;
	case CMD_GOTO_NEXT:
		GoToNextFrame();
		break;

	// View menu
	case CMD_IMAGE:
		SetDisplayMode(DISP_IMAGE);
		break;
	case CMD_CHART:
		SetDisplayMode(DISP_CHART);
		break;
	case CMD_TABLE:
		SetDisplayMode(DISP_TABLE);
		break;
	case CMD_IMAGE_CHART:
		SetDisplayMode(DISP_FULL);
		break;
	case CMD_PSEUDOCOLORS:
		m_Pseudocolors = m_Menu.IsChecked(CMD_PSEUDOCOLORS);
		UpdateImage();
		g_Project->SetBool("PreviewDlg", "Pseudocolors", m_Pseudocolors);
		break;
	case CMD_ORIGINAL:
		m_ShowOrig = m_Menu.IsChecked(CMD_ORIGINAL);
		ReloadImage();
		g_Project->SetBool("PreviewDlg", "Original", m_ShowOrig);
		break;
	case CMD_CALIBRATED:
		m_ShowOrig = !m_Menu.IsChecked(CMD_CALIBRATED);
		ReloadImage();
		g_Project->SetBool("PreviewDlg", "Original", m_ShowOrig);
		break;
	case CMD_RULERS:
		m_Rulers = m_Menu.IsChecked(CMD_RULERS);
		cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_ChartView), m_Rulers);
		g_Project->SetBool("PreviewDlg", "Rulers", m_Rulers);
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
	case CMD_INSPECTOR:
		if (m_Menu.IsChecked(CMD_INSPECTOR))
			SetInfoMode(INFO_OBJECT);
		else if (m_InfoMode == INFO_OBJECT)
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_FRAME_PREVIEW);
		break;
	}
}

void CFrameDlg::ShowProperties(void)
{
	if (!m_pFile)
		return;

	GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		CFrameInfoDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Show(pPath);
		gtk_tree_path_free(pPath);
	}
}

void CFrameDlg::button_clicked(GtkWidget *button, CFrameDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CFrameDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_NextBtn)) {
		GoToNextFrame();
	} else 
	if (pBtn==GTK_WIDGET(m_PrevBtn)) {
		GoToPreviousFrame();
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomIn)) {
		double zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_ChartView), zoom + 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomOut)) {
		double zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_ChartView), zoom - 5.0);
	} else
	if (pBtn==GTK_WIDGET(m_ZoomFit)) {
		cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_ChartView), TRUE);
	} else 
	if (pBtn==m_AperCombo) {
		int index = SelectedItem(GTK_COMBO_BOX(m_AperCombo));
		if (index>=0 && index!=m_ApertureIndex) {
			m_ApertureIndex = index;
			m_Phot.SelectAperture(m_ApertureIndex);
			UpdateChart();
			if (m_InfoMode==INFO_OBJECT && m_SelectedObjId>=0)
				UpdateInfoBox();
		}
	}
}

void CFrameDlg::selection_changed(GtkWidget *pChart, CFrameDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

//
// Left button click
//
void CFrameDlg::OnSelectionChanged(void)
{
	if (!m_Updating) {
		if (m_DispMode != DISP_TABLE) {
			int row = cmpack_chart_view_get_selected(CMPACK_CHART_VIEW(m_ChartView));
			if (row>=0 && m_ChartData) {
				if (row != m_SelectedRow) {
					m_SelectedRow = row;
					m_SelectedObjId = (int)cmpack_chart_data_get_param(m_ChartData, m_SelectedRow);
					m_SelectedRefId = m_Phot.GetObjectRefID(m_Phot.FindObject(m_SelectedObjId));
					UpdateInfoBox();
				}
			} else {
				if (m_SelectedRow>=0 || m_SelectedObjId>=0 || m_SelectedRefId>=0) {
					m_SelectedRow = m_SelectedObjId = m_SelectedRefId = -1;
					UpdateInfoBox();
				}
			}
		} else {
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
			GList *list = gtk_tree_selection_get_selected_rows(selection, NULL);
			if (list && m_TableData) {
				GtkTreePath *path = (GtkTreePath*)(list->data);
				if (!m_SelectedPath || gtk_tree_path_compare(path, m_SelectedPath)) {
					if (m_SelectedPath)
						gtk_tree_path_free(m_SelectedPath);
					m_SelectedPath = gtk_tree_path_copy(path);
					GtkTreeIter iter;
					gtk_tree_model_get_iter(m_TableData, &iter, path);
					gtk_tree_model_get(m_TableData, &iter, CPhot::COL_ID, &m_SelectedObjId, CPhot::COL_REF_ID, &m_SelectedRefId, -1);
					gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_TableView), m_SelectedPath, 0, 0, 0, 0);
					UpdateInfoBox();
				}
			} else {
				if (m_SelectedPath!=NULL || m_SelectedObjId>=0 || m_SelectedRefId>=0) {
					if (m_SelectedPath)
						gtk_tree_path_free(m_SelectedPath);
					m_SelectedPath = NULL;
					m_SelectedObjId = m_SelectedRefId = -1;
					UpdateInfoBox();
				}
			}
			g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
			g_list_free (list);
		}
	}
}

void CFrameDlg::profile_changed(GtkWidget *pChart, CFrameDlg *pDlg)
{
	pDlg->OnProfileChanged();
}

//
// Profile changed
//
void CFrameDlg::OnProfileChanged(void)
{
	gint x0, y0, x1, y1;

	if (m_InfoMode == INFO_PROFILE) {
		if (cmpack_chart_view_profile_get_pos(CMPACK_CHART_VIEW(m_ChartView), CHART_PROFILE_START, &x0, &y0) &&
			cmpack_chart_view_profile_get_pos(CMPACK_CHART_VIEW(m_ChartView), CHART_PROFILE_END, &x1, &y1)) {
				m_Profile.SetProfile(m_Image, x0, y0, x1, y1);
		} else
			m_Profile.Clear();
	}
}

gboolean CFrameDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CFrameDlg *pMe)
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
void CFrameDlg::OnLButtonClick(guint32 timestamp)
{
	if (m_InfoMode==INFO_PHOTOMETRY) 
		m_QPhot.Update(&m_Image, m_LastPosX, m_LastPosY);
}

void CFrameDlg::item_activated(GtkWidget *button, gint row, CFrameDlg *pDlg)
{
	pDlg->OnItemActivated(row);
}

void CFrameDlg::OnItemActivated(gint row)
{
	if (m_InfoMode != INFO_OBJECT)
		SetInfoMode(INFO_OBJECT);
	OnSelectionChanged();
}

void CFrameDlg::mouse_moved(GtkWidget *button, CFrameDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
	pDlg->m_MouseOnProfile = false;
	pDlg->m_MouseOnHistogram = false;
}

void CFrameDlg::zoom_changed(GtkWidget *button, CFrameDlg *pDlg)
{
	pDlg->m_UpdateZoom = true;
}

gboolean CFrameDlg::timer_cb(CFrameDlg *pDlg)
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

void CFrameDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CFrameDlg *pMe = (CFrameDlg*)cb_data;

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

void CFrameDlg::OnInfoBoxClosed(CInfoBox *pBox)
{
	m_InfoMode = INFO_NONE;
	cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_ChartView));
	cmpack_chart_view_profile_enable(CMPACK_CHART_VIEW(m_ChartView), FALSE);
	HideOverlays();
	UpdateStatus();
	UpdateControls();
}

void CFrameDlg::UpdateZoom(void)
{
	double zoom, min, max;

	zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
	cmpack_chart_view_get_zoom_limits(CMPACK_CHART_VIEW(m_ChartView), &min, &max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), zoom < max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), zoom > min);
}

void CFrameDlg::UpdateStatus(void)
{
	gchar		buf[256];
	gdouble		pos_x, pos_y;
	gint		x = -1, y = -1, obj_id, ref_id, index;

	if (m_MouseOnProfile) {
		m_LastFocus = -1;
		if (m_Profile.GetMousePos(&x, &y)) {
			if (x!=m_LastPosX || y!=m_LastPosY) {
				m_LastPosX = x;
				m_LastPosY = y;
				sprintf(buf, "Cursor: X = %d, Y = %d, Pixel value = %.1f ADU", x, y, m_Image.getPixel(x, y));
				SetStatus(buf);
			}
		} else {
			if (m_LastPosX!=-1 || m_LastPosY!=-1) {
				m_LastPosX = m_LastPosY = -1;
				SetStatus(NULL);
			}
		}
	} else
	if (m_MouseOnHistogram) {
		m_LastFocus = -1;
		if (m_Histogram.GetMousePos(&pos_x)) {
			gint x = RoundToInt(pos_x);
			if (x!=m_LastPosX) {
				m_LastPosX = x;
				sprintf(buf, "Cursor: Pixel value = %d ADU", x);
				SetStatus(buf);
			}
		} else {
			if (m_LastPosX!=-1) {
				m_LastPosX = -1;
				SetStatus(NULL);
			}
		}
	} else
	if (m_DispMode==DISP_IMAGE) {
		m_LastFocus = -1;
		if (cmpack_chart_view_mouse_pos(CMPACK_CHART_VIEW(m_ChartView), &x, &y)) {
			if (x!=m_LastPosX || y!=m_LastPosY) {
				m_LastPosX = x;
				m_LastPosY = y;
				sprintf(buf, "Cursor: X = %d, Y = %d, Pixel value = %.1f ADU", x, y, m_Image.getPixel(x, y));
				SetStatus(buf);
			}
		} else {
			if (m_LastPosX!=-1 || m_LastPosY!=-1) {
				m_LastPosX = m_LastPosY = -1;
				SetStatus(NULL);
			}
		}
	} else
	if (m_DispMode==DISP_FULL) {
		int item = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(m_ChartView));
		if (item>=0 && m_ChartData) {
			if (m_LastFocus!=item) {
				m_LastFocus = item;
				obj_id = (int)cmpack_chart_data_get_param(m_ChartData, item);
				index = m_Phot.FindObject(obj_id);
				ref_id = (index>=0 ? m_Phot.GetObjectRefID(index) : -1);
				m_Phot.GetObjectPos(index, &pos_x, &pos_y);
				if (ref_id>=0) 
					sprintf(buf, "Object #%d (ref. #%d): X = %.1f, Y = %.1f", obj_id, ref_id, pos_x, pos_y);
				else
					sprintf(buf, "Object #%d (not matched): X = %.1f, Y = %.1f", obj_id, pos_x, pos_y);
				SetStatus(buf);
			}
		} else {
			m_LastFocus = -1;
			if (cmpack_chart_view_mouse_pos(CMPACK_CHART_VIEW(m_ChartView), &x, &y)) {
				if (x!=m_LastPosX || y!=m_LastPosY) {
					m_LastPosX = x;
					m_LastPosY = y;
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
	} else
	if (m_DispMode==DISP_CHART) {
		int item = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(m_ChartView));
		if (item>=0 && m_ChartData) {
			if (m_LastFocus!=item) {
				m_LastFocus = item;
				obj_id = (int)cmpack_chart_data_get_param(m_ChartData, item);
				index = m_Phot.FindObject(obj_id);
				ref_id = (index>=0 ? m_Phot.GetObjectRefID(index) : -1);
				gdouble pos_x, pos_y;
				m_Phot.GetObjectPos(index, &pos_x, &pos_y);
				if (ref_id>=0) 
					sprintf(buf, "Object #%d (ref. #%d): X = %.1f, Y = %.1f", obj_id, ref_id, pos_x, pos_y);
				else
					sprintf(buf, "Object #%d (not matched): X = %.1f, Y = %.1f", obj_id, pos_x, pos_y);
				SetStatus(buf);
			}
		} else {
			m_LastFocus = -1;
			if (cmpack_chart_view_mouse_pos(CMPACK_CHART_VIEW(m_ChartView), &x, &y)) {
				if (x!=m_LastPosX || y!=m_LastPosY) {
					m_LastPosX = x;
					m_LastPosY = y;
					sprintf(buf, "Cursor: X = %d, Y = %d", x, y);
					SetStatus(buf);
				}
			} else {
				if (m_LastPosX!=-1 || m_LastPosY!=-1) {
					m_LastPosX = m_LastPosY = -1;
					SetStatus(NULL);
				}
			}
		}
	} else {
		SetStatus(NULL);
	}
}

void CFrameDlg::GoToFirstFrame(void)
{
	GtkTreePath *pFirst = g_Project->GetFirstFile();
	if (pFirst) {
		LoadFrame(pFirst);
		gtk_tree_path_free(pFirst);
	}
}

void CFrameDlg::GoToLastFrame(void)
{
	GtkTreePath *pLast = g_Project->GetLastFile();
	if (pLast) {
		LoadFrame(pLast);
		gtk_tree_path_free(pLast);
	}
}

void CFrameDlg::GoToNextFrame(void)
{
	GtkTreePath *pPath, *pNext;

	pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		pNext = g_Project->GetNextFile(pPath);
		if (pNext) {
			LoadFrame(pNext);
			gtk_tree_path_free(pNext);
		}
		gtk_tree_path_free(pPath);
	}
}

void CFrameDlg::GoToPreviousFrame(void)
{
	GtkTreePath *pPath, *pPrev;

	pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		pPrev = g_Project->GetPreviousFile(pPath);
		if (pPrev) {
			LoadFrame(pPrev);
			gtk_tree_path_free(pPrev);
		}
		gtk_tree_path_free(pPath);
	}
}

void CFrameDlg::SetDisplayMode(tDispMode mode)
{
	switch (mode) 
	{
	case DISP_IMAGE:
		gtk_widget_hide(m_TableScrWnd);
		gtk_widget_show(m_ChartScrWnd);
		if (m_InfoMode == INFO_OBJECT) 
			SetInfoMode(INFO_NONE);
		gtk_widget_grab_focus(m_ChartView);
		break;

	case DISP_CHART:
		gtk_widget_hide(m_TableScrWnd);
		gtk_widget_show(m_ChartScrWnd);
		if (m_InfoMode != INFO_NONE && m_InfoMode != INFO_OBJECT)
			SetInfoMode(INFO_NONE);
		gtk_widget_grab_focus(m_ChartView);
		break;

	case DISP_TABLE:
		gtk_widget_hide(m_ChartScrWnd);
		gtk_widget_show(m_TableScrWnd);
		if (m_InfoMode != INFO_NONE && m_InfoMode != INFO_OBJECT)
			SetInfoMode(INFO_NONE);
		gtk_widget_grab_focus(m_TableView);
		break;

	case DISP_FULL:
		gtk_widget_hide(m_TableScrWnd);
		gtk_widget_show(m_ChartScrWnd);
		gtk_widget_grab_focus(m_ChartView);
		if (m_InfoMode == INFO_PHOTOMETRY)
			SetInfoMode(INFO_NONE);
		break;

	default:
		gtk_widget_hide(m_TableScrWnd);
		gtk_widget_show(m_ChartScrWnd);
		SetInfoMode(INFO_NONE);
		break;
	}
	m_DispMode = mode;
	g_Project->SetInt("PreviewDlg", "Mode", m_DispMode);
	UpdateImage();
	UpdateChart();
	UpdateInfoBox();
	UpdateStatus();
	UpdateControls();
}

// Environment changed, reload settings
void CFrameDlg::EnvironmentChanged(void)
{
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	UpdateImage();
	UpdateChart();
	UpdateInfoBox();
	UpdateStatus();
	UpdateControls();
}

void CFrameDlg::SetInfoMode(tInfoMode mode)
{
	if (mode!=INFO_PROFILE) 
		cmpack_chart_view_profile_clear(CMPACK_CHART_VIEW(m_ChartView));
	if (mode==INFO_GRAYSCALE) {
		m_Scale.SetScaleParams(m_Image.BlackLevel(), m_Image.BlackLevel() + m_Image.Range(), 
			m_Pseudocolors, m_Negative);
	}
	if (mode==INFO_HISTOGRAM)
		m_Histogram.SetData(&m_Image);
	
	m_InfoMode = mode;
	switch (mode) 
	{
	case INFO_GRAYSCALE:
		if (m_DispMode == DISP_CHART)
			SetDisplayMode(DISP_FULL);
		else if (m_DispMode == DISP_TABLE)
			SetDisplayMode(DISP_IMAGE);
		m_QPhot.Show(false);
		m_InfoBox.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
		m_Scale.Show(true);
		break;
	case INFO_PHOTOMETRY:
		if (m_DispMode != DISP_IMAGE)
			SetDisplayMode(DISP_IMAGE);
		m_Scale.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
		m_InfoBox.Show(false);
		m_QPhot.Show(true);
		break;
	case INFO_OBJECT:
		if (m_DispMode == DISP_IMAGE)
			SetDisplayMode(DISP_FULL);
		m_Scale.Show(false);
		m_Profile.Show(false);
		m_Histogram.Show(false);
		m_QPhot.Show(false);
		m_InfoBox.Show(true);
		break;
	case INFO_PROFILE:
		if (m_DispMode != DISP_IMAGE)
			SetDisplayMode(DISP_IMAGE);
		m_Scale.Show(false);
		m_InfoBox.Show(false);
		m_Histogram.Show(false);
		m_QPhot.Show(false);
		m_Profile.Show(true);
		break;
	case INFO_HISTOGRAM:
		if (m_DispMode != DISP_IMAGE)
			SetDisplayMode(DISP_IMAGE);
		m_Scale.Show(false);
		m_InfoBox.Show(false);
		m_Profile.Show(false);
		m_QPhot.Show(false);
		m_Histogram.Show(true);
		break;
	default:
		m_Scale.Show(false);
		m_InfoBox.Show(false);
		m_Profile.Show(false);
		m_QPhot.Show(false);
		m_Histogram.Show(false);
		break;
	}

	cmpack_chart_view_profile_enable(CMPACK_CHART_VIEW(m_ChartView), m_InfoMode==INFO_PROFILE);
	UpdateInfoBox();
	UpdateControls();
}

void CFrameDlg::UpdateInfoBox(void)
{
	int		index;
	double	sky_in, sky_out, aperture;
	char	buf[256];
	CmpackPhtData data;
	CmpackError code;

	if (m_InfoMode == INFO_OBJECT) {
		// Show information about selected star
		m_InfoBox.BeginUpdate();
		m_InfoBox.Clear();
		if (m_SelectedObjId>=0) {
			CmpackPhtObject obj;
			sprintf(buf, "Object #%d", m_SelectedObjId);
			m_InfoBox.AddTitle(1, buf);
			index = m_Phot.FindObject(m_SelectedObjId);
			m_Phot.GetObjectParams(index, CMPACK_PO_REF_ID | CMPACK_PO_CENTER | CMPACK_PO_SKY | CMPACK_PO_FWHM, &obj);
			code = CMPACK_ERR_OK;
			data.mag_valid = false;
			m_Phot.GetMagnitudeAndCode(index, data, code);
			if (obj.ref_id>=0) {
				sprintf(buf, "Reference ID: %d", obj.ref_id);
				m_InfoBox.AddText(buf);
			}
			sprintf(buf, "Center(X) = %.2f", obj.x);
			m_InfoBox.AddText(buf);
			sprintf(buf, "Center(Y) = %.2f", obj.y);
			m_InfoBox.AddText(buf);
			if (obj.fwhm>0) 
				sprintf(buf, "FWHM = %.2f pxl", obj.fwhm);
			else
				sprintf(buf, "FWHM = ---- pxl");
			m_InfoBox.AddText(buf);
			sprintf(buf, "Sky = %.2f ADU", obj.skymed);
			m_InfoBox.AddText(buf);
			sprintf(buf, "Sky dev. = %.2f ADU", obj.skysig);
			m_InfoBox.AddText(buf);
			if (data.mag_valid) {
				double adu = pow(10.0, (25.0-data.magnitude)/2.5), stdev = adu*data.mag_error/1.0857;
				sprintf(buf, "Net intensity = %.1f ADU", adu);
				m_InfoBox.AddText(buf);
				sprintf(buf, "Noise = %.1f ADU", stdev);
				m_InfoBox.AddText(buf);
				if (data.mag_error>0) 
					sprintf(buf, "S/N ratio = %.1f dB", -10.0*log10(1.0857/data.mag_error));
				else
					sprintf(buf, "S/N ratio = ---- dB");
				m_InfoBox.AddText(buf);
				sprintf(buf, "Brightness = %.4f mag", data.magnitude);
				m_InfoBox.AddText(buf);
				if (data.mag_error>0)
					sprintf(buf, "Error = %.4f mag", data.mag_error);
				else
					sprintf(buf, "Error = ---- mag");
				m_InfoBox.AddText(buf);
			} else {
				sprintf(buf, "Net intensity = ---- ADU");
				m_InfoBox.AddText(buf);
				sprintf(buf, "Noise = ---- ADU");
				m_InfoBox.AddText(buf);
				sprintf(buf, "S/N ratio = ---- dB");
				m_InfoBox.AddText(buf);
				sprintf(buf, "Brightness = ---- mag");
				m_InfoBox.AddText(buf);
				sprintf(buf, "Error = ---- mag");
				m_InfoBox.AddText(buf);
			}
			if (code != CMPACK_ERR_OK) {
				char *msg = cmpack_formaterror(code);
				m_InfoBox.AddText(msg);
				cmpack_free(msg);
			}
			if (m_DispMode != DISP_TABLE) {
				m_InfoBox.AddText("");
				m_InfoBox.AddTitle(1, "Legend");
				m_InfoBox.AddText("Aperture: green circle");
				m_InfoBox.AddText("Sky: blue annulus");
			}
		} else {
			if (m_DispMode != DISP_TABLE) {
				m_InfoBox.AddText("Click on the object");
				m_InfoBox.AddText("to show its properties");
			} else {
				m_InfoBox.AddText("Select a row in the table");
				m_InfoBox.AddText("to show its properties");
			}
		}
		m_InfoBox.EndUpdate();
	}
	if (m_InfoMode == INFO_OBJECT && m_DispMode != DISP_TABLE && m_ChartData && m_SelectedRow>=0) {
		double x, y;
		cmpack_chart_data_get_center(m_ChartData, m_SelectedRow, &x, &y);
		aperture = m_Aper.Get(m_ApertureIndex)->Radius();
		sky_in = g_Project->Profile()->GetDbl(CProfile::SKY_INNER_RADIUS);
		sky_out = g_Project->Profile()->GetDbl(CProfile::SKY_OUTER_RADIUS);
		ShowOverlays(x, y, aperture, 0.0, sky_in, sky_out);
	} else {
		HideOverlays();
	}
}


void CFrameDlg::ShowOverlays(double x, double y, double aperture, double fwhm,
		double sky_in, double sky_out)
{
	double r;

	if (m_LayerId<0) {
		m_LayerId = cmpack_chart_view_add_layer(CMPACK_CHART_VIEW(m_ChartView));
		r = aperture;
		m_ObjectId[0] = cmpack_chart_view_add_circle(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, 
			x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_GREEN, FALSE);
		r = 0.5 * fwhm;
		m_ObjectId[1] = cmpack_chart_view_add_circle(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, 
			x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_RED, FALSE);
		r = sky_in;
		m_ObjectId[2] = cmpack_chart_view_add_circle(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, 
			x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_BLUE, FALSE);
		r = sky_out;
		m_ObjectId[3] = cmpack_chart_view_add_circle(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, 
			x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_BLUE, FALSE);
	} else {
		r = aperture;
		cmpack_chart_view_move_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[0], x - r, y - r);
		cmpack_chart_view_resize_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[0], 2.0*r, 2.0*r);
		r = 0.5 * fwhm;
		cmpack_chart_view_move_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[1], x - r, y - r);
		cmpack_chart_view_resize_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[1], 2.0*r, 2.0*r);
		r = sky_in;
		cmpack_chart_view_move_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[2], x - r, y - r);
		cmpack_chart_view_resize_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[2],  2.0*r, 2.0*r);
		r = sky_out;
		cmpack_chart_view_move_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[3], x - r, y - r);
		cmpack_chart_view_resize_object(CMPACK_CHART_VIEW(m_ChartView), m_ObjectId[3], 2.0*r, 2.0*r);
		cmpack_chart_view_show_layer(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, TRUE);
	}
}

void CFrameDlg::HideOverlays()
{
	if (m_LayerId>=0) 
		cmpack_chart_view_show_layer(CMPACK_CHART_VIEW(m_ChartView), m_LayerId, FALSE);
}

//
// Show dialog
//
void CFrameDlg::Show(void)
{
	gtk_window_present(GTK_WINDOW(m_pDlg));
}


//
// Window destroyed
//
void CFrameDlg::destroy(GtkObject *pWnd, CFrameDlg *pDlg)
{
	delete pDlg;
}


//
// Frame deleted
//
void CFrameDlg::row_deleted(GtkTreeModel *tree_model, GtkTreePath *path, CFrameDlg *pDlg)
{
	pDlg->OnRowDeleted(tree_model, path);
}
void CFrameDlg::OnRowDeleted(GtkTreeModel *tree_model, GtkTreePath *path)
{
	if (m_pFile && !m_DontClose) {
		GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
		if (!pPath)
			Close();
		else 
			UpdateControls();
		gtk_tree_path_free(pPath);
	}
}


//
// Close the window
//
void CFrameDlg::Close(void)
{
	gtk_widget_destroy(m_pDlg);
}


//
// Remove frame from the list
//
void CFrameDlg::RemoveFromProject(void)
{
	bool ok, close_output;
	GtkTreePath *pPath, *pNext;
	GtkTreeRowReference *pNextRef;

	if (!m_pFile)
		return;

	pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		if (g_Project->IsReferenceFrame(pPath)) {
			ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "This frame is a reference frame. This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
			close_output = true;
		} else {
			ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the frame from the current project?");
			close_output = false;
		}
		if (ok) {
			if (close_output) {
				g_MainWnd->CloseOutputDlgs();
				g_MainWnd->CloseNewFiles();
			}
			// Get reference to a row that will be shown when the current item is deleted
			pNext = g_Project->GetNextFile(pPath);
			if (!pNext)
				pNext = g_Project->GetPreviousFile(pPath);
			if (pNext)
				pNextRef = gtk_tree_row_reference_new(g_Project->FileList(), pNext);
			else
				pNextRef = NULL;
			gtk_tree_path_free(pNext);
			// Delete the frame
			m_DontClose = true;
			g_Project->RemovePath(pPath);
			m_DontClose = false;
			gtk_tree_path_free(pPath);
			// Show the next frame
			if (pNextRef) {
				GtkTreePath *pNextPath = gtk_tree_row_reference_get_path(pNextRef);
				if (pNextPath) {
					LoadFrame(pNextPath);
					gtk_tree_path_free(pNextPath);
				}
				gtk_tree_row_reference_free(pNextRef);
			} else {
				// If this was the last frame in the project, close
				// the window
				Close();
			}
		}
	}
}

void CFrameDlg::SetStatus(const char *text)
{
	if (m_StatusMsg>=0) {
		gtk_statusbar_pop(GTK_STATUSBAR(m_Status), m_StatusCtx);
		m_StatusMsg = -1;
	}
	if (text && strlen(text)>0) 
		m_StatusMsg = gtk_statusbar_push(GTK_STATUSBAR(m_Status), m_StatusCtx, text);
}

void CFrameDlg::Export(void)
{
	GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		unsigned status = g_Project->GetState(pPath);
		gchar filename[256];
		sprintf(filename, "frame%d", g_Project->GetFrameID(pPath));
		if (m_DispMode != DISP_TABLE) {
			// Export chart
			CChartExportDlg dlg(GTK_WINDOW(m_pDlg));
			if (m_DispMode==DISP_CHART)
				dlg.Execute(m_ChartData, NULL, filename, m_Negative, m_RowsUpward);
			else if (m_DispMode==DISP_IMAGE)
				dlg.Execute(NULL, m_ImageData, filename, m_Negative, m_RowsUpward);
			else
				dlg.Execute(m_ChartData, m_ImageData, filename, m_Negative, m_RowsUpward);
		} else 
		if (status & CFILE_PHOTOMETRY) {
			// Export table of objects
			CFrameExportDlg dlg(GTK_WINDOW(m_pDlg));
			dlg.Execute(m_Phot, (status & CFILE_MATCHING)!=0, filename, m_SortColumnId, m_SortType);
		}
		gtk_tree_path_free(pPath);
	}
}

void CFrameDlg::table_column_clicked(GtkTreeViewColumn *treeviewcolumn, CFrameDlg *pDlg)
{
	pDlg->OnTableColumnClicked(treeviewcolumn);
}

void CFrameDlg::OnTableColumnClicked(GtkTreeViewColumn *pCol)
{
	if (m_SelectedPath) {
		gtk_tree_path_free(m_SelectedPath);
		m_SelectedPath = NULL;
	}

	if (m_TableData) {
		if (pCol == m_SortCol) {
			// Swap the sort type (order)
			m_SortType = (GtkSortType)(1-m_SortType);
		} else {
			// Change the sort column, keep sort type
			if (m_SortCol)
				gtk_tree_view_column_set_sort_indicator(m_SortCol, FALSE);
			m_SortCol = pCol;
			m_SortColumnId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pCol), "sort-column-id"));
			gtk_tree_view_column_set_sort_indicator(pCol, TRUE);
		}
		gtk_tree_view_column_set_sort_order(pCol, m_SortType);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(m_TableData), m_SortColumnId, m_SortType);
		OnSelectionChanged();
		gtk_widget_grab_focus(m_TableView);
	}
}

void CFrameDlg::table_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, CFrameDlg *pMe)
{
	pMe->OnTableRowActivated(treeview, path);
}

void CFrameDlg::OnTableRowActivated(GtkTreeView *treeview, GtkTreePath *path)
{
	if (m_InfoMode != INFO_OBJECT)
		SetInfoMode(INFO_OBJECT);
	OnSelectionChanged();
}
