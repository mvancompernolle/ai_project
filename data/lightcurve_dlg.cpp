/**************************************************************

lightcurve_dlg.cpp (C-Munipack project)
The 'Light curve' window
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

#include "project.h"
#include "configuration.h"
#include "progress_dlg.h"
#include "lightcurve_dlg.h"
#include "makelightcurve_dlg.h"
#include "choosestars_dlg.h"
#include "chooseaperture_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"
#include "chart_dlg.h"
#include "frameinfo_dlg.h"
#include "object_dlg.h"
#include "observer_dlg.h"
#include "export_dlgs.h"

//-------------------------   LOCAL MENU   --------------------------------

enum tMenuId
{
	MENU_FILE = 1,
	MENU_EDIT,
	MENU_VIEW,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_SAVE = 100,
	CMD_EXPORT,
	CMD_REBUILD,
	CMD_PRINT,
	CMD_CLOSE,
	CMD_PROPERTIES,
	CMD_STARS,
	CMD_APERTURE,
	CMD_ERRORS,
	CMD_GRAPH,
	CMD_TABLE,
	CMD_GRID,
	CMD_CHART,
	CMD_HELP,
	CMD_HIDE,
	CMD_DELETE,
	CMD_PREVIEW,
	CMD_FRAMEINFO,
	CMD_STATISTICS,
	CMD_MEASUREMENT
};

static const CMenuBar::tMenuItem FileMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_SAVE,		"_Save" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,		CMD_EXPORT,		"Export _image" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,		CMD_REBUILD,	"_Rebuild" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,		CMD_CLOSE,		"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem EditMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_PROPERTIES,	"Edit _properties" },
	{ CMenuBar::MB_ITEM,		CMD_STARS,		"Change selected _stars" },
	{ CMenuBar::MB_ITEM,		CMD_APERTURE,	"Change _aperture" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN,	CMD_GRAPH,		"_Graph" },
	{ CMenuBar::MB_RADIOBTN,	CMD_TABLE,		"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN,	CMD_ERRORS,		"_Error bars" },
	{ CMenuBar::MB_CHECKBTN,	CMD_GRID,		"_Grid" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,		CMD_CHART,		"_Chart" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN,	CMD_STATISTICS,	"_Statistics" },
	{ CMenuBar::MB_CHECKBTN,	CMD_MEASUREMENT,	"_Measurement" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_HELP,		"_Show help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu MainMenu[] = {
	{ "_File", MENU_FILE,	FileMenu },
	{ "_Edit",	MENU_EDIT,	EditMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

static const CPopupMenu::tPopupMenuItem GraphContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_PREVIEW,		"_Show frame" },
	{ CPopupMenu::MB_ITEM, CMD_FRAMEINFO,	"_Show properties" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_HIDE,		"_Delete from data set" },
	{ CPopupMenu::MB_ITEM, CMD_DELETE,		"_Remove from project" },
	{ CPopupMenu::MB_END }
};

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CLightCurveDlg::CLightCurveDlg(void):m_UpdatePos(true), m_LastPosValid(false), 
	m_ApertureIndex(-1), m_ChannelX(-1), m_ChannelY(-1), m_LastPosX(-1), 
	m_LastPosY(-1), m_LastFocus(-1), m_Table(NULL), m_TimerId(0), m_InfoMode(INFO_NONE),
	m_GraphData(NULL), m_TableData(NULL)
{
	gchar buf[512];
	GtkWidget *tbox;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	m_ShowErrors = g_Project->GetBool("LightCurve", "Errors", true);
	m_ShowGrid   = g_Project->GetBool("LightCurve", "Grid", false);
	m_DateFormat = (tDateFormat)g_Project->GetInt("Display", "DateFormat", JULIAN_DATE, 0, GREGORIAN_DATE);
	m_DispMode	 = (tDisplayMode)g_Project->GetInt("LightCurve", "Display", DISPLAY_GRAPH, 0, DISPLAY_TABLE);

	// Dialog caption
	sprintf(buf, "%s - %s", "Light curve", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Menu bar
	m_Menu.Create(MainMenu, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), FALSE, FALSE, 0);

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(m_MainBox), tbox, false, false, 0);

	m_XLabel = toolbar_new_label(tbox, "X axis");
	m_DateFormats = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_list_store_append(m_DateFormats, &iter);
	gtk_list_store_set(m_DateFormats, &iter, 0, JULIAN_DATE, 1, "JD", -1);
	gtk_list_store_append(m_DateFormats, &iter);
	gtk_list_store_set(m_DateFormats, &iter, 0, GREGORIAN_DATE, 1, "UTC", -1);
	m_DCombo = toolbar_new_combo(tbox, "Column shown on the horizontal axis of the graph");
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_DCombo), GTK_TREE_MODEL(m_DateFormats));
	g_signal_connect(G_OBJECT(m_DCombo), "changed", G_CALLBACK(entry_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_DCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_DCombo), renderer, "text", 1);
	SelectItem(GTK_COMBO_BOX(m_DCombo), m_DateFormat);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_DCombo))<0) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_DCombo), 0);
		m_DateFormat = (tDateFormat)SelectedItem(GTK_COMBO_BOX(m_DCombo));
	}

	m_XChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_XCombo = toolbar_new_combo(tbox, "Column shown on the horizontal axis of the graph");
	g_signal_connect(G_OBJECT(m_XCombo), "changed", G_CALLBACK(entry_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_XCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_XCombo), renderer, "text", 1);

	m_YLabel = toolbar_new_label(tbox, "Y axis");
	m_YChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_YCombo = toolbar_new_combo(tbox, "Column shown on the vertical axis of the graph");
	g_signal_connect(G_OBJECT(m_YCombo), "changed", G_CALLBACK(entry_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_YCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_YCombo), renderer, "text", 1);

	m_ALabel = toolbar_new_label(tbox, "Aperture");
	m_Apertures = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_ACombo = toolbar_new_combo(tbox, "Aperture used to get brightness of the objects");
	g_signal_connect(G_OBJECT(m_ACombo), "changed", G_CALLBACK(entry_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_ACombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_ACombo), renderer, "text", 1);

	m_ZoomLabel = toolbar_new_label(tbox, "Zoom");
	m_ZoomFit = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_FIT, "Zoom to all points");
	g_signal_connect(G_OBJECT(m_ZoomFit), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomOut = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_OUT, "Zoom out");
	g_signal_connect(G_OBJECT(m_ZoomOut), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomIn = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_IN, "Zoom in");
	g_signal_connect(G_OBJECT(m_ZoomIn), "clicked", G_CALLBACK(button_clicked), this);

	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_MainBox), hbox, TRUE, TRUE, 0);

	// Graph
	m_GraphView = cmpack_graph_view_new_with_model(NULL);
	cmpack_graph_view_set_mouse_control(CMPACK_GRAPH_VIEW(m_GraphView), TRUE);
	cmpack_graph_view_set_selection_mode(CMPACK_GRAPH_VIEW(m_GraphView), GTK_SELECTION_MULTIPLE);
	cmpack_graph_view_set_scales(CMPACK_GRAPH_VIEW(m_GraphView), TRUE, TRUE);
	cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), TRUE, TRUE);
	g_signal_connect(G_OBJECT(m_GraphView), "mouse-moved", G_CALLBACK(mouse_moved), this);
	g_signal_connect(G_OBJECT(m_GraphView), "button_press_event", G_CALLBACK(button_press_event), this);
	g_signal_connect(G_OBJECT(m_GraphView), "selection-changed", G_CALLBACK(selection_changed), this);
	gtk_widget_set_size_request(m_GraphView, 300, 200);
	m_GraphScrWnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_GraphScrWnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_GraphScrWnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_GraphScrWnd), m_GraphView);
	gtk_box_pack_start(GTK_BOX(hbox), m_GraphScrWnd, TRUE, TRUE, 0);

	// Table
	m_TableView = gtk_tree_view_new_with_model(NULL);
	m_TableScrWnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_TableScrWnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_TableScrWnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_TableScrWnd), m_TableView);
	gtk_box_pack_start(GTK_BOX(hbox), m_TableScrWnd, TRUE, TRUE, 0);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_TableView), TRUE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Statistics tool
	m_InfoBox.SetCaption("Statistics");
	m_InfoBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_InfoBox.Handle(), FALSE, FALSE, 0);

	// Measurement tool
	m_MeasBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_MeasBox.Handle(), FALSE, FALSE, 0);
	m_MeasBox.SetGraphView(CMPACK_GRAPH_VIEW(m_GraphView));

	// Timers
	m_TimerId = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 100, GSourceFunc(update_status_timer), this, NULL);

	// Popup menu
	m_GraphMenu.Create(GraphContextMenu);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	SetDisplayMode(m_DispMode);
	SetInfoMode(INFO_NONE);
}

CLightCurveDlg::~CLightCurveDlg()
{
	// Disconnect graph signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)mouse_moved, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)button_press_event, this);

	// Release objects, free allocated memory
	g_source_remove(m_TimerId);
	g_object_unref(m_DateFormats);
	g_object_unref(m_XChannels);
	g_object_unref(m_YChannels);
	g_object_unref(m_Apertures);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	if (m_TableData)
		g_object_unref(m_TableData);
	delete m_Table;
}

bool CLightCurveDlg::Make(GtkWindow *parent, bool selected_files, const tParamsRec &rec, 
	const CObjectCoords &obj, const CLocation &loc)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;
	m_FrameSet.Clear();
	m_AperList.Clear();
	m_ChannelX = m_ChannelY = -1;
	m_DataSaved = false;
	m_Params = rec;
	m_ObjCoords = obj;
	m_Location = loc;

	// Create list of files
	if (!InitFileList(parent, selected_files))
		return false;
	
	// Apertures
	m_AperList = *g_Project->Apertures();
	if (m_AperList.Count()==0) {
		ShowError(parent, "No aperture available.\nCheck the configuration and execute the photometry and the matching.");
		return false;
	}

	// Choose stars
	if (g_Project->LastSelection()->Count()>0) 
		m_Selection = *g_Project->LastSelection();
	else if (g_Project->SelectionList()->Count()>0) 
		m_Selection = g_Project->SelectionList()->At(0);
	else
		m_Selection.Clear();

	CChooseStarsDlg dlg(parent);
	if (!dlg.Execute(m_Selection, m_Params.instmag))
		return false;
	g_Project->SetLastSelection(m_Selection);
	RebuildData(parent);

	// Choose aperture
	m_ApertureIndex = -1;
	if (m_AperList.Count()>0) {
		int index = m_AperList.Find(g_Project->GetInt("LightCurve", "Aperture", 0));
		if (m_Selection.CountStars(CMPACK_SELECT_CHECK)>0 && m_Selection.CountStars(CMPACK_SELECT_COMP)>0) {
			CChooseApertureDlg dlg(parent);
			if (!dlg.Execute(m_FrameSet, m_AperList, m_Selection, &index))
				return false;
		} else {
			CChooseApertureSimpleDlg dlg(parent);
			if (!dlg.Execute(m_FrameSet, m_AperList, m_Selection, &index))
				return false;
		}
		g_Project->SetInt("LightCurve", "Aperture", m_AperList.GetId(index));
		m_ApertureIndex = index;
	}

	UpdateApertures();
	UpdateLightCurve();
	UpdateChannels();
	UpdateGraphTable(TRUE, TRUE);
	UpdateTools();
	SetInfoMode(INFO_NONE);
	SetStatus(NULL);
	UpdateControls();
	return true;
}

void CLightCurveDlg::OnFrameSetChanged(void)
{
	UpdateLightCurve();
	UpdateGraphTable(TRUE, TRUE);
	UpdateTools();
	UpdateStatus();
}

void CLightCurveDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_XLabel, m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_DCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_DateFormats), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_XCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_XChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_YLabel, m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_YCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_YChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_ACombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1);
	gtk_widget_set_sensitive(m_ALabel, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1);
	gtk_widget_set_sensitive(m_ZoomLabel, m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_GRAPH, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_TABLE, m_DispMode==DISPLAY_TABLE);
	m_Menu.Enable(CMD_ERRORS, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Enable(CMD_EXPORT, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_ERRORS, m_ShowErrors);
	m_Menu.Enable(CMD_GRID, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_GRID, m_ShowGrid);
	m_Menu.Enable(CMD_STATISTICS, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_STATISTICS, m_InfoMode == INFO_STATISTICS);
	m_Menu.Enable(CMD_MEASUREMENT, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_MEASUREMENT, m_InfoMode == INFO_MEASUREMENT);
}

void CLightCurveDlg::UpdateLightCurve(void)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;

	int res = 0;
	if (m_Params.amass) 
		res = CmpackAirMass(NULL, m_FrameSet, m_ObjCoords, m_Location);
	if (res==0 && m_Params.helcor)
		res = CmpackHelCorr(NULL, m_FrameSet, m_ObjCoords);
	if (res==0) {
		res = CmpackLightCurve(NULL, &m_Table, m_Selection, m_FrameSet, 
			m_AperList.GetId(m_ApertureIndex), (CmpackLCurveFlags)(
			CMPACK_LCURVE_FRAME_IDS |
			(m_Params.helcor ? CMPACK_LCURVE_HELCORR | CMPACK_LCURVE_HJD : 0) |
			(m_Params.amass ? CMPACK_LCURVE_AIRMASS | CMPACK_LCURVE_ALTITUDE : 0) |
			(m_Params.instmag ? CMPACK_LCURVE_INSTMAG : 0)));
	}
	if (m_Table && res==0)
		m_MeasBox.SetTable(m_Table);
}

void CLightCurveDlg::UpdateGraphTable(gboolean autozoom_x, gboolean autozoom_y)
{
	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), NULL);
	if (m_GraphData) {
		g_object_unref(m_GraphData);
		m_GraphData = NULL;
	}
	if (m_Table && m_ChannelX>=0 && m_ChannelY>=0) {
		m_GraphData = m_Table->ToGraphData(m_ChannelX, m_ChannelY);
		m_Table->SetView(CMPACK_GRAPH_VIEW(m_GraphView), m_ChannelX, m_ChannelY, !m_ShowErrors, 
			NULL, NULL, m_DateFormat);
		cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), m_ShowGrid, m_ShowGrid);
		cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), m_GraphData);
		cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_GraphView), autozoom_x, autozoom_y);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), NULL);
	if (m_TableData) {
		g_object_unref(m_TableData);
		m_TableData = NULL;
	}
	if (m_Table && m_ChannelX>=0 && m_ChannelY>=0) {
		m_TableData = m_Table->ToTreeModel();
		m_Table->SetView(GTK_TREE_VIEW(m_TableView));
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), m_TableData);
	}
}

void CLightCurveDlg::UpdateChannels(void)
{
	m_Updating = true;

	// Update list of independent channels
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_XCombo), NULL);
	gtk_list_store_clear(m_XChannels);
	CChannels *cx = m_Table->ChannelsX();
	for (int i=0; i<cx->Count(); i++) {
		if (cx->GetInfo(i) == CChannel::DATA_JD) {
			GtkTreeIter iter;
			gtk_list_store_append(m_XChannels, &iter);
			if (m_DateFormat == JULIAN_DATE)
				gtk_list_store_set(m_XChannels, &iter, 0, i, 1, "Geocentric JD", -1);
			else
				gtk_list_store_set(m_XChannels, &iter, 0, i, 1, "Geocentric UTC", -1);
			if (m_ChannelX<0 && !m_Params.helcor)
				m_ChannelX = i;
		}
		else if (cx->GetInfo(i) == CChannel::DATA_JD_HEL) {
			GtkTreeIter iter;
			gtk_list_store_append(m_XChannels, &iter);
			if (m_DateFormat == JULIAN_DATE)
				gtk_list_store_set(m_XChannels, &iter, 0, i, 1, "Heliocentric JD", -1);
			else
				gtk_list_store_set(m_XChannels, &iter, 0, i, 1, "Heliocentric UTC", -1);
			if (m_ChannelX<0 && m_Params.helcor)
				m_ChannelX = i;
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_XCombo), GTK_TREE_MODEL(m_XChannels));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_XChannels), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_XCombo), m_ChannelX);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_XCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_XCombo), 0);
			m_ChannelX = SelectedItem(GTK_COMBO_BOX(m_XCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_XCombo), -1);
		m_ChannelX = -1;
	}

	// Update list of independent channels
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_YCombo), NULL);
	gtk_list_store_clear(m_YChannels);
	CChannels *cy = m_Table->ChannelsY();
	for (int i=0; i<cy->Count(); i++) {
		GtkTreeIter iter;
		gtk_list_store_append(m_YChannels, &iter);
		gtk_list_store_set(m_YChannels, &iter, 0, i, 1, cy->GetName(i), -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_YCombo), GTK_TREE_MODEL(m_YChannels));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_YChannels), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_YCombo), m_ChannelY);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_YCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_YCombo), 0);
			m_ChannelY = SelectedItem(GTK_COMBO_BOX(m_YCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_YCombo), -1);
		m_ChannelY = -1;
	}

	m_Updating = false;
}

void CLightCurveDlg::UpdateApertures(void)
{
	char txt[256];
	GtkTreeIter iter;

	m_Updating = true;

	// Update list of independent channels
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_ACombo), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_AperList.Count(); i++) {
		const CAperture *aper = m_AperList.Get(i);
		sprintf(txt, "#%d (%.2f)", aper->Id(), aper->Radius());
		gtk_list_store_append(m_Apertures, &iter);
		gtk_list_store_set(m_Apertures, &iter, 0, i, 1, txt, -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_ACombo), GTK_TREE_MODEL(m_Apertures));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_ACombo), m_ApertureIndex);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_ACombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_ACombo), 0);
			m_ApertureIndex = SelectedItem(GTK_COMBO_BOX(m_ACombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_ACombo), -1);
		m_ApertureIndex = -1;
	}

	m_Updating = false;
}

void CLightCurveDlg::MenuCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CLightCurveDlg *pMe = (CLightCurveDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		// Menu bar command
		pMe->OnCommand(wparam);
		break;
	}
}

void CLightCurveDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_CLOSE:
		Close();
		break;
	case CMD_SAVE:
		SaveData();
		break;
	case CMD_EXPORT:
		Export();
		break;
	case CMD_REBUILD:
		RebuildData(GTK_WINDOW(m_pDlg));
		break;

	// Edit menu
	case CMD_PROPERTIES:
		EditProperties();
		break;
	case CMD_STARS:
		EditSelection();
		break;
	case CMD_APERTURE:
		ChangeAperture();
		break;

	// View menu
	case CMD_GRAPH:
		SetDisplayMode(DISPLAY_GRAPH);
		break;
	case CMD_TABLE:
		SetDisplayMode(DISPLAY_TABLE);
		break;
	case CMD_ERRORS:
		m_ShowErrors = m_Menu.IsChecked(CMD_ERRORS);
		m_Table->SetView(CMPACK_GRAPH_VIEW(m_GraphView), m_ChannelX, m_ChannelY, !m_ShowErrors);
		g_Project->SetBool("LightCurve", "Errors", m_ShowErrors);
		break;
	case CMD_GRID:
		m_ShowGrid = m_Menu.IsChecked(CMD_GRID);
		cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), m_ShowGrid, m_ShowGrid);
		g_Project->SetBool("LightCurve", "Grid", m_ShowGrid);
		break;
	case CMD_CHART:
		ShowChart();
		break;

	// Tools menu
	case CMD_STATISTICS:
		if (m_Menu.IsChecked(CMD_STATISTICS)) 
			SetInfoMode(INFO_STATISTICS);
		else if (m_InfoMode == INFO_STATISTICS)
			SetInfoMode(INFO_NONE);
		break;
	case CMD_MEASUREMENT:
		if (m_Menu.IsChecked(CMD_MEASUREMENT)) 
			SetInfoMode(INFO_MEASUREMENT);
		else if (m_InfoMode == INFO_MEASUREMENT)
			SetInfoMode(INFO_NONE);
		break;

	// Help menu
	case CMD_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_LIGHT_CURVE);
		break;
	}
}

void CLightCurveDlg::entry_changed(GtkWidget *button, CLightCurveDlg *pDlg)
{
	pDlg->OnEntryChanged(button);
}

void CLightCurveDlg::OnEntryChanged(GtkWidget *pBtn)
{
	if (!m_Updating) {
		if (pBtn==m_DCombo) {
			int format = SelectedItem(GTK_COMBO_BOX(m_DCombo));
			if (format>=0 && format!=m_DateFormat) {
				m_DateFormat = (tDateFormat)format;
				g_Project->SetInt("Display", "DateFormat", m_DateFormat);
				UpdateChannels();
				UpdateGraphTable(TRUE, FALSE);
				UpdateTools();
				UpdateControls();
			}
		} else if (pBtn==m_XCombo) {
			int channel = SelectedItem(GTK_COMBO_BOX(m_XCombo));
			if (channel>=0 && channel!=m_ChannelX) {
				m_ChannelX = channel;
				UpdateGraphTable(FALSE, TRUE);
				UpdateTools();
				UpdateControls();
			}
		} else if (pBtn==m_YCombo) {
			int channel = SelectedItem(GTK_COMBO_BOX(m_YCombo));
			if (channel>=0 && channel!=m_ChannelY) {
				m_ChannelY = channel;
				UpdateGraphTable(FALSE, TRUE);
				UpdateTools();
				UpdateControls();
			}
		} else if (pBtn==m_ACombo) {
			int index = SelectedItem(GTK_COMBO_BOX(m_ACombo));
			if (index>=0 && index!=m_ApertureIndex) {
				m_ApertureIndex = index;
				g_Project->SetInt("LightCurve", "Aperture", m_AperList.GetId(m_ApertureIndex));
				m_DataSaved = false;
				UpdateLightCurve();
				UpdateGraphTable(FALSE, FALSE);
				UpdateTools();
				UpdateControls();
			}
		}
	}
}

void CLightCurveDlg::button_clicked(GtkWidget *button, CLightCurveDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CLightCurveDlg::OnButtonClicked(GtkWidget *pBtn)
{
	double zoom;

	if (pBtn==GTK_WIDGET(m_ZoomIn)) {
		zoom = cmpack_graph_view_get_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_X);
		cmpack_graph_view_set_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_X, zoom + 5.0);
		zoom = cmpack_graph_view_get_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_Y);
		cmpack_graph_view_set_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_Y, zoom + 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomOut)) {
		zoom = cmpack_graph_view_get_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_X);
		cmpack_graph_view_set_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_X, zoom - 5.0);
		zoom = cmpack_graph_view_get_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_Y);
		cmpack_graph_view_set_zoom(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_Y, zoom - 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomFit)) {
		cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_GraphView), TRUE, TRUE);
	}
}

void CLightCurveDlg::mouse_moved(GtkWidget *button, CLightCurveDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

//
// Mouse button handler
//
gint CLightCurveDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CLightCurveDlg *pMe)
{
	int focused;

	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		if (widget==pMe->m_GraphView) {
			focused = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(widget));
			if (focused>=0 && !cmpack_graph_view_is_selected(CMPACK_GRAPH_VIEW(widget), focused)) {
				cmpack_graph_view_unselect_all(CMPACK_GRAPH_VIEW(widget));
				cmpack_graph_view_select(CMPACK_GRAPH_VIEW(widget), focused);
			}
		}
		pMe->OnContextMenu(widget, event);
		return TRUE;
	}
	return FALSE;
}

void CLightCurveDlg::selection_changed(GtkWidget *pChart, CLightCurveDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

//
// Left button click
//
void CLightCurveDlg::OnSelectionChanged(void)
{
	UpdateTools();
	UpdateStatus();
}

//
// Show context menu
//
void CLightCurveDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int res, selected;

	if (widget==m_GraphView) {
		selected = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_GraphView));
		m_GraphMenu.Enable(CMD_DELETE, selected>0);
		m_GraphMenu.Enable(CMD_HIDE, selected>0);
		m_GraphMenu.Enable(CMD_FRAMEINFO, selected==1);
		m_GraphMenu.Enable(CMD_PREVIEW, selected==1);
		res = m_GraphMenu.Execute(event);
		switch (res) 
		{
		case CMD_PREVIEW:
			// Show frame preview
			ShowFramePreview();
			break;
		case CMD_FRAMEINFO:
			// Show frame properties
			ShowFrameInfo();
			break;
		case CMD_HIDE:
			// Remove selected points from data set
			RemoveFromDataSet();
			break;
		case CMD_DELETE:
			// Delete selected points from project
			DeleteFromProject();
			break;
		}
	}
}

// 100 ms timer
gboolean CLightCurveDlg::update_status_timer(CLightCurveDlg *pDlg)
{
	gdk_threads_lock();
	if (pDlg->m_UpdatePos) {
		pDlg->m_UpdatePos = false;
		pDlg->UpdateStatus();
	}
	gdk_threads_unlock();
	return TRUE;
}

void CLightCurveDlg::UpdateStatus(void)
{
	char	msg[256], buf1[256], buf2[256];
	gdouble	dx, dy, sdev;
	int		frame_id;

	if (m_DispMode == DISPLAY_GRAPH) {
		int item = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(m_GraphView));
		if (item>=0) {
			if (m_LastFocus!=item && m_GraphData && m_Table) {
				m_LastFocus = item;
				int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, item),
					xcol = m_Table->ChannelsX()->GetColumn(m_ChannelX),
					ycol = m_Table->ChannelsY()->GetColumn(m_ChannelY),
					ucol = m_Table->ChannelsY()->GetColumnU(m_ChannelY);
				if (m_Table->Find(frame_id) && m_Table->GetDbl(xcol, &dx) && m_Table->GetDbl(ycol, &dy)) {
					PrintKeyValue(buf1, dx, m_Table->ChannelsX()->Get(m_ChannelX));
					if (ucol>=0 && m_Table->GetDbl(ucol, &sdev))
						PrintKeyValueU(buf2, dy, sdev, m_Table->ChannelsY()->Get(m_ChannelY));
					else 
						PrintKeyValue(buf2, dy, m_Table->ChannelsY()->Get(m_ChannelY));
					sprintf(msg, "Frame #%d: %s, %s", frame_id, buf1, buf2);
				} else {
					sprintf(msg, "Frame #%d", frame_id);
				}
				SetStatus(msg);
			}
		} else {
			m_LastFocus = -1;
			if (m_ChannelX>=0 && m_ChannelX<m_Table->ChannelsX()->Count() &&
				m_ChannelY>=0 && m_ChannelY<m_Table->ChannelsY()->Count() &&
				cmpack_graph_view_mouse_pos(CMPACK_GRAPH_VIEW(m_GraphView), &dx, &dy)) {
					if (!m_LastPosValid || dx!=m_LastPosX || dy!=m_LastPosY) {
						m_LastPosValid = true;
						m_LastPosX = dx;
						m_LastPosY = dy;
						PrintKeyValue(buf1, dx, m_Table->ChannelsX()->Get(m_ChannelX));
						PrintKeyValue(buf2, dy, m_Table->ChannelsY()->Get(m_ChannelY));
						sprintf(msg, "Cursor: %s, %s", buf1, buf2);
						SetStatus(msg);
					}
			} else {
				if (m_LastPosValid) {
					m_LastPosValid = false;
					SetStatus(NULL);
				}
			}
		}
	} else {
		GList *rows = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView)), NULL);
		if (rows && m_TableData) {
			int count = g_list_length(rows);
			if (count>1) {
				sprintf(msg, "%d rows selected", count);
				SetStatus(msg);
			} else {
				GtkTreeIter iter;
				if (gtk_tree_model_get_iter(m_TableData, &iter, (GtkTreePath*)rows->data)) {
					gtk_tree_model_get(m_TableData, &iter, 0, &frame_id, -1);
					sprintf(msg, "Frame #%d", frame_id);
					SetStatus(msg);
				} else
					SetStatus(NULL);
			}
		} else
			SetStatus(NULL);
		g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (rows);
	}
}

void CLightCurveDlg::SetDisplayMode(tDisplayMode mode)
{
	switch (mode) 
	{
	case DISPLAY_TABLE:
		gtk_widget_hide(m_GraphScrWnd);
		gtk_widget_show(m_TableScrWnd);
		SetInfoMode(INFO_NONE);
		break;

	case DISPLAY_GRAPH:
		gtk_widget_show(m_GraphScrWnd);
		gtk_widget_hide(m_TableScrWnd);
		m_LastFocus = -1;
		m_LastPosValid = false;
		break;
	}
	m_DispMode = mode;
	g_Project->SetInt("LightCurve", "Display", m_DispMode);
	UpdateControls();
}

void CLightCurveDlg::PrintValue(char *buf, double value, const CChannel *channel)
{
	const gchar *name = channel->Name();

	if ((strcmp(name, "JD")==0 || strcmp(name, "JDHEL")==0) && m_DateFormat != JULIAN_DATE) {
		CmpackDateTime dt;
		cmpack_decodejd(value, &dt);
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second);
	} else {
		const gchar *unit = channel->Unit();
		if (unit)
			sprintf(buf, "%.*f %s", channel->Precision(), value, unit);
		else
			sprintf(buf, "%.*f", channel->Precision(), value);
	}
}

void CLightCurveDlg::PrintKeyValue(char *buf, double value, const CChannel *channel)
{
	const gchar *name = channel->Name();

	if ((strcmp(name, "JD")==0 || strcmp(name, "JDHEL")==0) && m_DateFormat != JULIAN_DATE) {
		CmpackDateTime dt;
		cmpack_decodejd(value, &dt);
		sprintf(buf, "%s = %04d-%02d-%02d %02d:%02d:%02d", "UTC", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second);
	} else {
		const gchar *unit = channel->Unit();
		if (unit)
			sprintf(buf, "%s = %.*f %s", channel->Name(), channel->Precision(), value, unit);
		else
			sprintf(buf, "%s = %.*f", channel->Name(), channel->Precision(), value);
	}
}

void CLightCurveDlg::PrintKeyValueU(char *buf, double value, double sdev, const CChannel *channel)
{
	const gchar *name = channel->Name(), *unit = channel->Unit();
	int prec = channel->Precision();

	if (unit)
		sprintf(buf, "%s = %.*f +- %.*f %s", name, prec, value, prec, sdev, unit);
	else
		sprintf(buf, "%s = %.*f +- %.*f", name, prec, value, prec, sdev);
}

void CLightCurveDlg::UpdateTools(void)
{
	char	buf[256], msg[256];
	double	dx, dy, dmin, dmax, mean, sdev;
	int		xcol, ycol, ucol;

	if (m_InfoMode == INFO_MEASUREMENT) {
		// Update measurement tool
		m_MeasBox.SetDateFormat(m_DateFormat);
		m_MeasBox.SetChannel(CMPACK_AXIS_X, m_ChannelX);
		m_MeasBox.SetChannel(CMPACK_AXIS_Y, m_ChannelY);
	} else 
	if (m_InfoMode == INFO_STATISTICS && m_GraphData && m_Table) {
		m_InfoBox.BeginUpdate();
		m_InfoBox.Clear();
		// Show information about selected star
		xcol = m_Table->ChannelsX()->GetColumn(m_ChannelX);
		ycol = m_Table->ChannelsY()->GetColumn(m_ChannelY);
		ucol = m_Table->ChannelsY()->GetColumnU(m_ChannelY);
		int selected_count = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_GraphView));
		if (selected_count==1) {
			int row = cmpack_graph_view_get_selected(CMPACK_GRAPH_VIEW(m_GraphView));
			int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, row);
			if (frame_id>=0) {
				sprintf(buf, "Frame #%d", frame_id);
				m_InfoBox.AddTitle(1, buf);
				if (m_Table->Find(frame_id) && m_Table->GetDbl(xcol, &dx) && m_Table->GetDbl(ycol, &dy)) {
					PrintKeyValue(buf, dx, m_Table->ChannelsX()->Get(m_ChannelX));
					m_InfoBox.AddText(buf);
					if (ucol>=0 && m_Table->GetDbl(ucol, &sdev)) 
						PrintKeyValueU(buf, dy, sdev, m_Table->ChannelsY()->Get(m_ChannelY));
					else
						PrintKeyValue(buf, dy, m_Table->ChannelsY()->Get(m_ChannelY));
					m_InfoBox.AddText(buf);
				}
			}
		} else {
			// Compute statistics
			int count = 0, length = (selected_count>0 ? selected_count : m_Table->Rows());
			double *x = new double[length], *y = new double[length];
			if (selected_count>0) {
				GList *list = cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_GraphView));
				for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) {
					int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, (gint)(intptr_t)ptr->data);
					if (m_Table->Find(frame_id) && m_Table->GetDbl(xcol, x+count) && m_Table->GetDbl(ycol, y+count))
						count++;
				}
				g_list_free(list);
			} else {
				bool ok = m_Table->Rewind();
				while (ok) {
					if (m_Table->GetDbl(xcol, x+count) && m_Table->GetDbl(ycol, y+count))
						count++;
					ok = m_Table->Next();
				}
			}
			if (selected_count>0)
				sprintf(buf, "There are %d selected points.", count);
			else
				sprintf(buf, "There are %d points in total.", count);
			m_InfoBox.AddText(buf);
			if (ComputeMinMax(count, x, &dmin, &dmax)) {
				m_InfoBox.AddText("");
				bool jdhel = m_Table->ChannelsX()->GetInfo(m_ChannelX) == CChannel::DATA_JD_HEL;
				sprintf(msg, "Data %s:", (m_DateFormat==JULIAN_DATE ? (!jdhel ? "Geocentric JD" : "Heliocentric JD") : (!jdhel ? "Geocentric UTC" : "Heliocentric UTC")));
				m_InfoBox.AddTitle(1, msg);
				PrintValue(buf, dmin, m_Table->ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_Table->ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Max.", buf);
				m_InfoBox.AddText(msg);
			}
			if (ComputeMinMax(count, y, &dmin, &dmax)) {
				cmpack_robustmean(count, y, &mean, &sdev);
				m_InfoBox.AddText("");
				sprintf(msg, "Data %s:", m_Table->ChannelsY()->GetName(m_ChannelY));
				m_InfoBox.AddTitle(1, msg);
				PrintValue(buf, dmin, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Max.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, mean, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Mean", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, sdev, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "St.dev.", buf);
				m_InfoBox.AddText(msg);
			}
			delete[] x;
			delete[] y;
		}
		m_InfoBox.EndUpdate();
	} else {
		// No tool is open
		cmpack_graph_view_set_cursors(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_X, 0);
		cmpack_graph_view_set_cursors(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_AXIS_Y, 0);
	}
}

void CLightCurveDlg::SetInfoMode(tInfoMode mode)
{
	m_InfoMode = mode;
	switch (m_InfoMode) 
	{
	case INFO_STATISTICS:
		m_MeasBox.Show(false);
		m_InfoBox.Show(true);
		break;
	case INFO_MEASUREMENT:
		m_InfoBox.Show(false);
		m_MeasBox.Show(true);
		break;
	default:
		m_MeasBox.Show(false);
		m_InfoBox.Show(false);
		break;
	}
	UpdateTools();
	UpdateControls();
}

void CLightCurveDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CLightCurveDlg *pMe = (CLightCurveDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CLightCurveDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateControls();
}

void CLightCurveDlg::SaveData(void)
{
	CSaveLightCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	if (pDlg.Execute(*m_Table, m_ChannelX, m_ChannelY)) {
		m_DataSaved = true;
		UpdateControls();
	}
}

void CLightCurveDlg::Export(void)
{
	CGraphExportDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(CMPACK_GRAPH_VIEW(m_GraphView), g_Project->Name());
}

void CLightCurveDlg::EditProperties(void)
{
	CLightCurveOptionsDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(m_Params, m_ObjCoords, m_Location)) {
		g_Project->SetBool("LightCurve", "HelCorr", m_Params.helcor);
		g_Project->SetBool("LightCurve", "AirMass", m_Params.amass);
		g_Project->SetBool("LightCurve", "InstMag", m_Params.instmag);
		g_Project->SetObjectCoords(m_ObjCoords);
		CConfig::SetLastObject(m_ObjCoords);
		g_Project->SetLocation(m_Location);
		CConfig::SetLastLocation(m_Location);
		m_DataSaved = false;
		UpdateLightCurve();
		UpdateChannels();
		UpdateGraphTable(TRUE, TRUE);
		UpdateTools();
		SetStatus(NULL);
		UpdateControls();
	}
}

void CLightCurveDlg::EditSelection(void)
{
	CChooseStarsDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(m_Selection, m_Params.instmag)) {
		m_DataSaved = false;
		g_Project->SetLastSelection(m_Selection);
		RebuildData(GTK_WINDOW(m_pDlg));
		UpdateLightCurve();
		UpdateChannels();
		UpdateGraphTable(TRUE, TRUE);
		UpdateTools();
		SetStatus(NULL);
		UpdateControls();
	}
}

void CLightCurveDlg::ChangeAperture(void)
{
	bool ok;

	int index = m_ApertureIndex;
	if (m_Selection.CountStars(CMPACK_SELECT_CHECK)>0 && m_Selection.CountStars(CMPACK_SELECT_COMP)>0) {
		CChooseApertureDlg dlg(GTK_WINDOW(m_pDlg));
		ok = dlg.Execute(m_FrameSet, m_AperList, m_Selection, &index);
	} else {
		CChooseApertureSimpleDlg dlg(GTK_WINDOW(m_pDlg));
		ok = dlg.Execute(m_FrameSet, m_AperList, m_Selection, &index);
	}
	if (ok) {
		if (index!=m_ApertureIndex) {
			m_ApertureIndex = index;
			SelectItem(GTK_COMBO_BOX(m_ACombo), m_ApertureIndex);
			if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_ACombo))<0) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(m_ACombo), 0);
				m_ApertureIndex = SelectedItem(GTK_COMBO_BOX(m_ACombo));
			}
			g_Project->SetInt("LightCurve", "Aperture", m_AperList.GetId(m_ApertureIndex));
			m_DataSaved = false;
			UpdateLightCurve();
			UpdateGraphTable(TRUE, TRUE);
			UpdateTools();
			SetStatus(NULL);
			UpdateControls();
		}
	}
}

void CLightCurveDlg::ShowChart(void)
{
	CChartDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(m_Selection, m_AperList.GetId(m_ApertureIndex));
}

GList *CLightCurveDlg::GetSelectedFrames(void)
{
	GList *frames = NULL;

	GList *rows = cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_GraphView));
	if (rows && m_GraphData) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			int frame = cmpack_graph_data_get_param(m_GraphData, (intptr_t)ptr->data);
			GtkTreePath *path = g_Project->GetFilePath(frame);
			if (path)
				frames = g_list_prepend(frames, path);
		}
		g_list_free(rows);
	}
	return g_list_reverse(frames);
}

GtkTreePath *CLightCurveDlg::GetSelectedFrame(void)
{
	int row = cmpack_graph_view_get_selected(CMPACK_GRAPH_VIEW(m_GraphView));
	if (row>=0 && m_GraphData) {
		int frame = cmpack_graph_data_get_param(m_GraphData, row);
		return g_Project->GetFilePath(frame);
	}
	return NULL;
}

//----------------------------   LIGHT CURVE OPTIONS ---------------------------------

CLightCurveOptionsDlg::CLightCurveOptionsDlg(GtkWindow *pParent)
{
	GtkWidget *tbox;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Light curve options", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("lightcurve");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	tbox = gtk_table_new(12, 4, FALSE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 3, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 6, 12);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);

	// Light curve options
	m_HelCor = gtk_check_button_new_with_label("Compute heliocentric correction");
	g_signal_connect(G_OBJECT(m_HelCor), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_HelCor, 0, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_AirMass = gtk_check_button_new_with_label("Compute air mass coefficients");
	g_signal_connect(G_OBJECT(m_AirMass), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_AirMass, 0, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_InstMag = gtk_check_button_new_with_label("Show raw instrumental magnitudes");
	g_signal_connect(G_OBJECT(m_InstMag), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_InstMag, 0, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

	// Object coordinates
	m_ObjLabel = gtk_label_new("Object - designation");
	gtk_misc_set_alignment(GTK_MISC(m_ObjLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjLabel, 0, 1, 4, 5);
	m_ObjName = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(m_ObjName), MAX_OBJECT_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjName, 1, 2, 4, 5);
	m_ObjBtn = gtk_button_new_with_label("More");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 2, 3, 4, 5);
	m_RALabel = gtk_label_new("- right ascension");
	gtk_misc_set_alignment(GTK_MISC(m_RALabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RALabel, 0, 1, 5, 6);
	m_RA = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(m_RA), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 5, 6);
	m_RAUnit = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(m_RAUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RAUnit, 2, 3, 5, 6);
	m_DecLabel = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(m_DecLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecLabel, 0, 1, 6, 7);
	m_Dec = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(m_Dec), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 6, 7);
	m_DecUnit = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(m_DecUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecUnit, 2, 3, 6, 7);

	// Location
	m_LocLabel = gtk_label_new("Observatory - name");
	gtk_misc_set_alignment(GTK_MISC(m_LocLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocLabel, 0, 1, 7, 8);
	m_LocName = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(m_LocName), MAX_LOCATION_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocName, 1, 2, 7, 8);
	m_LocBtn = gtk_button_new_with_label("More");
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocBtn, 2, 3, 7, 8);
	m_LonLabel = gtk_label_new("- longitude");
	gtk_misc_set_alignment(GTK_MISC(m_LonLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonLabel, 0, 1, 8, 9);
	m_Lon = gtk_entry_new_with_max_length(32);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lon, 1, 2, 8, 9);
	m_LonUnit = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(m_LonUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonUnit, 2, 3, 8, 9);
	m_LatLabel = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(m_LatLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatLabel, 0, 1, 9, 10);
	m_Lat = gtk_entry_new_with_max_length(32);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lat, 1, 2, 9, 10);
	m_LatUnit = gtk_label_new("[ d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(m_LatUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatUnit, 2, 3, 9, 10);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CLightCurveOptionsDlg::~CLightCurveOptionsDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CLightCurveOptionsDlg::response_dialog(GtkDialog *pDlg, gint response_id, CLightCurveOptionsDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CLightCurveOptionsDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_LIGHT_CURVE);
		return false;
	}
	return true;
}

bool CLightCurveOptionsDlg::Execute(CLightCurveDlg::tParamsRec &params, 
	CObjectCoords &obj, CLocation &loc)
{
	m_ComputeHC = params.helcor;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_HelCor), m_ComputeHC);
	m_ComputeAM = params.amass;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AirMass), m_ComputeAM);
	m_ShowInstMag = params.instmag;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_InstMag), m_ShowInstMag);
	m_ObjCoords = obj;
	m_Location = loc;

	UpdateObjectCoords();
	UpdateLocation();
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT) 
		return false;

	params.amass = m_ComputeAM;
	params.helcor = m_ComputeHC;
	params.instmag = m_ShowInstMag;
	obj = m_ObjCoords;
	loc = m_Location;
	return true;
}

//
// Set parameters
//
void CLightCurveOptionsDlg::UpdateObjectCoords(void)
{
	const CObjectCoords *obj = &m_ObjCoords;
	if (obj->Name())
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), obj->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), "");
	if (obj->RA())
		gtk_entry_set_text(GTK_ENTRY(m_RA), obj->RA());
	else
		gtk_entry_set_text(GTK_ENTRY(m_RA), "");
	if (obj->Dec())
		gtk_entry_set_text(GTK_ENTRY(m_Dec), obj->Dec());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Dec), "");
}

void CLightCurveOptionsDlg::UpdateLocation(void)
{
	const CLocation *obs = &m_Location;
	if (obs->Name())
		gtk_entry_set_text(GTK_ENTRY(m_LocName), obs->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
	if (obs->Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), obs->Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (obs->Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), obs->Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");
}

void CLightCurveOptionsDlg::UpdateControls()
{
	gboolean ok;

	ok = m_ComputeHC || m_ComputeAM;
	gtk_widget_set_sensitive(m_ObjBtn, ok);
	gtk_widget_set_sensitive(m_ObjName, ok);
	gtk_widget_set_sensitive(m_ObjLabel, ok);
	gtk_widget_set_sensitive(m_RA, ok);
	gtk_widget_set_sensitive(m_RALabel, ok);
	gtk_widget_set_sensitive(m_RAUnit, ok);
	gtk_widget_set_sensitive(m_Dec, ok);
	gtk_widget_set_sensitive(m_DecLabel, ok);
	gtk_widget_set_sensitive(m_DecUnit, ok);

	ok = m_ComputeAM;
	gtk_widget_set_sensitive(m_LocBtn, ok);
	gtk_widget_set_sensitive(m_LocName, ok);
	gtk_widget_set_sensitive(m_LocLabel, ok);
	gtk_widget_set_sensitive(m_Lat, ok);
	gtk_widget_set_sensitive(m_LatLabel, ok);
	gtk_widget_set_sensitive(m_LatUnit, ok);
	gtk_widget_set_sensitive(m_Lon, ok);
	gtk_widget_set_sensitive(m_LonLabel, ok);
	gtk_widget_set_sensitive(m_LonUnit, ok);
}

bool CLightCurveOptionsDlg::OnCloseQuery()
{
	const gchar *ra, *dec, *lon, *lat;
	gchar buf[64];
	gdouble x, y;

	m_ComputeHC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_HelCor))!=0;
	m_ComputeAM = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AirMass))!=0;
	m_ShowInstMag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_InstMag))!=0;
	
	if (m_ComputeHC || m_ComputeAM) {
		ra = gtk_entry_get_text(GTK_ENTRY(m_RA));
		dec = gtk_entry_get_text(GTK_ENTRY(m_Dec));
		if (*ra=='\0' || *dec=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter object coordinates.");
			return false;
		}
		if (cmpack_strtora(ra, &x)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the right ascension.");
			return false;
		}
		cmpack_ratostr(x, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_RA), buf);
		
		if (cmpack_strtodec(dec, &y)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the declination.");
			return false;
		}
		cmpack_dectostr(y, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Dec), buf);
	}

	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	m_ObjCoords.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	m_ObjCoords.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	m_ObjCoords.SetDec(g_strstrip(aux));
	g_free(aux);

	if (m_ComputeAM) {
		lon = gtk_entry_get_text(GTK_ENTRY(m_Lon));
		lat = gtk_entry_get_text(GTK_ENTRY(m_Lat));
		if (*lon=='\0' || *lat=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter the geographic coordinates.");
			return false;
		}
		if (cmpack_strtolon(lon, &x)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the longitude.");
			return false;
		}
		cmpack_lontostr(x, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Lon), buf);
		
		if (cmpack_strtolat(lat, &y)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the latitude.");
			return false;
		}
		cmpack_lattostr(y, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Lat), buf);
	}

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	m_Location.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	m_Location.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	m_Location.SetLat(g_strstrip(aux));
	g_free(aux);

	return true;
}

void CLightCurveOptionsDlg::button_clicked(GtkWidget *pButton, CLightCurveOptionsDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CLightCurveOptionsDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
	UpdateControls();
}

void CLightCurveOptionsDlg::button_toggled(GtkWidget *pButton, CLightCurveOptionsDlg *pMe)
{
	pMe->OnButtonToggled(pButton);
}

void CLightCurveOptionsDlg::OnButtonToggled(GtkWidget *pButton)
{
	m_ComputeHC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_HelCor))!=0;
	m_ComputeAM = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AirMass))!=0;
	UpdateControls();
}

void CLightCurveOptionsDlg::EditObjectCoords(void)
{
	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	m_ObjCoords.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	m_ObjCoords.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	m_ObjCoords.SetDec(g_strstrip(aux));
	g_free(aux);

	CObjectDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_ObjCoords)) {
		UpdateObjectCoords();
		UpdateControls();
	}
}

void CLightCurveOptionsDlg::EditLocation(void)
{
	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	m_Location.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	m_Location.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	m_Location.SetLat(g_strstrip(aux));
	g_free(aux);

	CLocationDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_Location)) {
		UpdateLocation();
		UpdateControls();
	}
}

//-------------------------   SAVE LIGHT CURVE   --------------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
	bool DiffMag, InstMag;
} FileFormats[] = {
	{ "CMPACK",	"C-Munipack (default)",				"txt", NULL,			"C-Munipack files", "*.txt",		
		true,	true },
	{ "AVE",	"AVE compatible",					"djm", "text/plain",	"AVE files",		"*.djm",	
		true,	false },
	{ "MCV",	"MCV compatible",					"dat", "text/plain",	"MCV files",		"*.mcv",	
		false,	true },
	{ "TEXT",	"Text (space separated values)",	"txt", "text/plain",	"Text files",		"*.txt",	
		true,	true },
	{ "CSV",	"CSV (comma separated values)",		"csv", "text/csv",		"CSV files",		"*.csv",	
		true,	true },
	{ NULL }
};

static const struct {
	const gchar *Id, *Caption;
} DateFormats[] = {
	{ "JDGEO", "Julian date geocentric" },
	{ "JDHEL", "Julian date heliocentric" },
	{ NULL }
};

static const gchar *FileExtension(gint type)
{
	if (type>=0)
		return FileFormats[type].Extension;
	return "";
}

static const gchar *FileMimeType(gint type)
{
	if (type>=0)
		return FileFormats[type].MimeType;
	return "";
}

static GtkFileFilter *FileFilter(gint type)
{
	if (type>=0 && FileFormats[type].FilterName && FileFormats[type].FilterPattern) {
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, FileFormats[type].FilterPattern);
		gtk_file_filter_set_name(filter, FileFormats[type].FilterName);
		return filter;
	}
	return NULL;
}

CSaveLightCurveDlg::CSaveLightCurveDlg(GtkWindow *pParent):m_pParent(pParent),
	m_Updating(false), m_FileType(TYPE_MUNIPACK), m_JDType(DATE_JD_GEOCENTRIC), 
	m_SelectedY(-1), m_InstMag(false), m_HaveHelCor(false), m_HaveAirMass(false), 
	m_HaveAltitude(false)
{
	GtkWidget *frame, *hbox, *label;
	GtkCellRenderer *renderer;

	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));
	m_Options[TYPE_MUNIPACK].header = m_Options[TYPE_MUNIPACK].errors = true;
	m_Options[TYPE_MUNIPACK].all_values = true;
	m_Options[TYPE_AVE].skip_invalid = m_Options[TYPE_MCV].zero_invalid = true;
	
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Save light curve", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("lightcurve");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Options
	frame = gtk_frame_new("Export options");
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);

	// File format
	GtkWidget *lbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), lbox, TRUE, TRUE, 0);
	m_FileTypes = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	label = gtk_label_new("File type");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(lbox), label, FALSE, TRUE, 0);
	m_TypeCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_FileTypes));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_TypeCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_TypeCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(lbox), m_TypeCombo, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_TypeCombo), "changed", G_CALLBACK(selection_changed), this);
	m_Header = gtk_check_button_new_with_label("Include column names");
	gtk_box_pack_start(GTK_BOX(lbox), m_Header, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Header), "toggled", G_CALLBACK(button_toggled), this);
	m_FrameIds = gtk_check_button_new_with_label("Include frame #");
	gtk_box_pack_start(GTK_BOX(lbox), m_FrameIds, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_FrameIds), "toggled", G_CALLBACK(button_toggled), this);
	m_SkipInvalid = gtk_check_button_new_with_label("Discard rows with invalid values");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipInvalid), "toggled", G_CALLBACK(button_toggled), this);
	m_ZeroInvalid = gtk_check_button_new_with_label("Replace invalid values with zeros");
	gtk_box_pack_start(GTK_BOX(lbox), m_ZeroInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_ZeroInvalid), "toggled", G_CALLBACK(button_toggled), this);
	
	// JD/UTC selection
	GtkWidget *mbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), mbox, TRUE, TRUE, 0);
	m_JDTypes = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	label = gtk_label_new("Date and time");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(mbox), label, FALSE, TRUE, 0);
	m_JDCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_JDTypes));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_JDCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_JDCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(mbox), m_JDCombo, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_JDCombo), "changed", G_CALLBACK(selection_changed), this);

	// Column selection
	GtkWidget *rbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), rbox, TRUE, TRUE, 0);
	m_Channels = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	label = gtk_label_new("Values");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(rbox), label, FALSE, TRUE, 0);
	m_VCCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_Channels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_VCCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_VCCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(rbox), m_VCCombo, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_VCCombo), "changed", G_CALLBACK(selection_changed), this);
	m_AllValues = gtk_check_button_new_with_label("Export all magnitudes");
	gtk_box_pack_start(GTK_BOX(rbox), m_AllValues, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_AllValues), "toggled", G_CALLBACK(button_toggled), this);
	m_Errors = gtk_check_button_new_with_label("Include error estimation");
	gtk_box_pack_start(GTK_BOX(rbox), m_Errors, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Errors), "toggled", G_CALLBACK(button_toggled), this);
	m_HelCor = gtk_check_button_new_with_label("Include heliocentric correction (days)");
	gtk_box_pack_start(GTK_BOX(rbox), m_HelCor, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_HelCor), "toggled", G_CALLBACK(button_toggled), this);
	m_AirMass = gtk_check_button_new_with_label("Include air mass coefficient");
	gtk_box_pack_start(GTK_BOX(rbox), m_AirMass, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_AirMass), "toggled", G_CALLBACK(button_toggled), this);
	m_Altitude = gtk_check_button_new_with_label("Include apparent altitude (degrees)");
	gtk_box_pack_start(GTK_BOX(rbox), m_Altitude, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Altitude), "toggled", G_CALLBACK(button_toggled), this);
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CSaveLightCurveDlg::~CSaveLightCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_FileTypes);
	g_object_unref(m_JDTypes);
	g_object_unref(m_Channels);
}

CSaveLightCurveDlg::tFileType CSaveLightCurveDlg::StrToFileType(const gchar *str)
{
	if (str) {
		for (gint i=0; FileFormats[i].Id!=NULL; i++) {
			if (strcmp(FileFormats[i].Id, str)==0)
				return (tFileType)i;
		}
	}
	return TYPE_MUNIPACK;
}

const gchar *CSaveLightCurveDlg::FileTypeToStr(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].Id;
	return "";
}

bool CSaveLightCurveDlg::Execute(const CTable &table, int xchannel, int ychannel)
{
	GtkTreeIter iter;

	m_Table.MakeCopy(table);
	m_SelectedY = ychannel;
	bool jdhel = m_Table.ChannelsX()->GetInfo(xchannel) == CChannel::DATA_JD_HEL;

	m_JDType = (jdhel ? DATE_JD_HELIOCENTRIC : DATE_JD_GEOCENTRIC);
	m_HaveHelCor = m_HaveAirMass = m_HaveAltitude = false;
	for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
		CChannel *ch = m_Table.ChannelsY()->Get(i);
		if (ch) {
			switch (ch->Info())
			{
			case CChannel::DATA_ALTITUDE:
				m_HaveAltitude = true;
				break;
			case CChannel::DATA_AIRMASS:
				m_HaveAirMass = true;
				break;
			case CChannel::DATA_HELCOR:
				m_HaveHelCor = true;
				break;
			default:
				break;
			}
		}
	}

	m_InstMag = m_Table.Type() == CMPACK_TABLE_LCURVE_INST;
	gchar *aux = g_Project->GetStr("LightCurve", (!m_InstMag ? "FileType" : "FileType2"));
	m_FileType = StrToFileType(aux);
	g_free(aux);

	m_Updating = true;

	// Restore last folder and file name
	gchar *dirpath = g_Project->GetStr("Output", "Folder", NULL);
	if (!dirpath)
		dirpath = g_path_get_dirname(g_Project->Path());
	if (dirpath && g_file_test(dirpath, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), dirpath);
	g_free(dirpath);
	gchar *defaultname = AddFileExtension(g_Project->Name(), "txt");
	gchar *filename = g_Project->GetStr("LightCurve", "File", defaultname);
	const gchar *defext = FileExtension(m_FileType);
	if (defext && filename) {
		gchar *newname = SetFileExtension(filename, defext);
		g_free(filename);
		filename = newname;
	}
	gchar *basename = g_path_get_basename(filename);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), basename);
	g_free(basename);
	g_free(filename);
	g_free(defaultname);

	// File types
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), NULL);
	gtk_list_store_clear(m_FileTypes);
	for (int i=0; FileFormats[i].Caption; i++) {
		if ((!m_InstMag && FileFormats[i].DiffMag) || (m_InstMag && FileFormats[i].InstMag)) {
			gtk_list_store_append(m_FileTypes, &iter);
			gtk_list_store_set(m_FileTypes, &iter, 0, i, 1, FileFormats[i].Caption, -1);
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), GTK_TREE_MODEL(m_FileTypes));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_FileTypes), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_TypeCombo), m_FileType);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_TypeCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), 0);
			m_FileType = (tFileType)SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), -1);
		m_FileType = TYPE_INVALID;
	}
	gtk_widget_set_sensitive(m_TypeCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_FileTypes), NULL)>1);

	// JD columns
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_JDCombo), NULL);
	gtk_list_store_clear(m_JDTypes);
	for (int i=0; DateFormats[i].Caption; i++) {
		if (i==DATE_JD_GEOCENTRIC || m_Table.ChannelsX()->FindFirst("JDHEL")>=0) {
			gtk_list_store_append(m_JDTypes, &iter);
			gtk_list_store_set(m_JDTypes, &iter, 0, i, 1, DateFormats[i].Caption, -1);
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_JDCombo), GTK_TREE_MODEL(m_JDTypes));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_JDTypes), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_JDCombo), m_JDType);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_JDCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_JDCombo), 0);
			m_JDType = (tDateType)SelectedItem(GTK_COMBO_BOX(m_JDCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_JDCombo), -1);
		m_JDType = DATE_INVALID;
	}
	gtk_widget_set_sensitive(m_JDCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_JDTypes), NULL)>1);

	// Restore options (format dependent customizable options)
	m_Options[TYPE_MUNIPACK].helcor = m_HaveHelCor && g_Project->GetBool("LightCurve", "MUNIPACK_HELCOR");
	m_Options[TYPE_MUNIPACK].airmass = m_HaveAirMass && g_Project->GetBool("LightCurve", "MUNIPACK_AIRMASS");
	m_Options[TYPE_MUNIPACK].altitude = m_HaveAltitude && g_Project->GetBool("LightCurve", "MUNIPACK_ALTITUDE");
	m_Options[TYPE_CSV].frame_id = g_Project->GetBool("LightCurve", "CSV_FRAME_ID");
	m_Options[TYPE_CSV].helcor = m_HaveHelCor && g_Project->GetBool("LightCurve", "CSV_HELCOR");
	m_Options[TYPE_CSV].airmass = m_HaveAirMass && g_Project->GetBool("LightCurve", "CSV_AIRMASS");
	m_Options[TYPE_CSV].altitude = m_HaveAltitude && g_Project->GetBool("LightCurve", "CSV_ALTITUDE");
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("LightCurve", "CSV_SKIP_INVALID", true);
	m_Options[TYPE_CSV].zero_invalid = m_InstMag && g_Project->GetBool("LightCurve", "CSV_ZERO_INVALID");
	m_Options[TYPE_CSV].errors = g_Project->GetBool("LightCurve", "CSV_ERRORS", true);
	m_Options[TYPE_CSV].header = g_Project->GetBool("LightCurve", "CSV_HEADER", true);
	m_Options[TYPE_CSV].all_values = g_Project->GetBool("LightCurve", "CSV_ALL_VALUES");
	m_Options[TYPE_TEXT].frame_id = g_Project->GetBool("LightCurve", "TEXT_FRAME_ID");
	m_Options[TYPE_TEXT].helcor = m_HaveHelCor && g_Project->GetBool("LightCurve", "TEXT_HELCOR");
	m_Options[TYPE_TEXT].airmass = m_HaveAirMass && g_Project->GetBool("LightCurve", "TEXT_AIRMASS");
	m_Options[TYPE_TEXT].altitude = m_HaveAltitude && g_Project->GetBool("LightCurve", "TEXT_ALTITUDE");
	m_Options[TYPE_TEXT].skip_invalid = g_Project->GetBool("LightCurve", "TEXT_SKIP_INVALID", true);
	m_Options[TYPE_TEXT].zero_invalid = m_InstMag && g_Project->GetBool("LightCurve", "TEXT_ZERO_INVALID");
	m_Options[TYPE_TEXT].errors = g_Project->GetBool("LightCurve", "TEXT_ERRORS", true);
	m_Options[TYPE_TEXT].header = g_Project->GetBool("LightCurve", "TEXT_HEADER", true);
	m_Options[TYPE_TEXT].all_values = g_Project->GetBool("LightCurve", "TEXT_ALL_VALUES");

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	dirpath = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);
	basename = g_path_get_basename(filename);
	g_Project->SetStr("LightCurve", "File", basename);
	g_free(basename);

	// Save settings
	g_Project->SetStr("LightCurve", (!m_InstMag ? "FileType" : "FileType2"), FileTypeToStr(m_FileType));
	if (m_HaveHelCor) {
		g_Project->SetBool("LightCurve", "MUNIPACK_HELCOR", m_Options[TYPE_MUNIPACK].helcor);
		g_Project->SetBool("LightCurve", "CSV_HELCOR", m_Options[TYPE_CSV].helcor);
		g_Project->SetBool("LightCurve", "TEXT_HELCOR", m_Options[TYPE_TEXT].helcor);
	}
	if (m_HaveAirMass) {
		g_Project->SetBool("LightCurve", "MUNIPACK_AIRMASS", m_Options[TYPE_MUNIPACK].airmass);
		g_Project->SetBool("LightCurve", "CSV_AIRMASS", m_Options[TYPE_CSV].airmass);
		g_Project->SetBool("LightCurve", "TEXT_AIRMASS", m_Options[TYPE_TEXT].airmass);
	}
	if (m_HaveAltitude) {
		g_Project->SetBool("LightCurve", "MUNIPACK_ALTITUDE", m_Options[TYPE_MUNIPACK].altitude);
		g_Project->SetBool("LightCurve", "CSV_ALTITUDE", m_Options[TYPE_CSV].altitude);
		g_Project->SetBool("LightCurve", "TEXT_ALTITUDE", m_Options[TYPE_TEXT].altitude);
	}
	if (m_InstMag) {
		g_Project->SetBool("LightCurve", "CSV_ZERO_INVALID", m_Options[TYPE_CSV].zero_invalid);
		g_Project->SetBool("LightCurve", "TEXT_ZERO_INVALID", m_Options[TYPE_TEXT].zero_invalid);
	}
	g_Project->SetBool("LightCurve", "CSV_FRAME_ID", m_Options[TYPE_CSV].frame_id);
	g_Project->SetBool("LightCurve", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("LightCurve", "CSV_ERRORS", m_Options[TYPE_CSV].errors);
	g_Project->SetBool("LightCurve", "CSV_HEADER", m_Options[TYPE_CSV].header);
	g_Project->SetBool("LightCurve", "CSV_ALL_VALUES", m_Options[TYPE_CSV].all_values);
	g_Project->SetBool("LightCurve", "TEXT_FRAME_ID", m_Options[TYPE_TEXT].frame_id);
	g_Project->SetBool("LightCurve", "TEXT_SKIP_INVALID", m_Options[TYPE_TEXT].skip_invalid);
	g_Project->SetBool("LightCurve", "TEXT_ERRORS", m_Options[TYPE_TEXT].errors);
	g_Project->SetBool("LightCurve", "TEXT_HEADER", m_Options[TYPE_TEXT].header);
	g_Project->SetBool("LightCurve", "TEXT_ALL_VALUES", m_Options[TYPE_TEXT].all_values);

	const tOptions *opt = m_Options+m_FileType;

	CChannels *cx = m_Table.ChannelsX();
	for (int i=0; i<cx->Count(); i++) {
		CChannel *ch = m_Table.ChannelsX()->Get(i);
		switch (ch->Info())
		{
		case CChannel::DATA_FRAME_ID:
			if (opt->frame_id)
				cx->Get(i)->SetExportFlags(0);
			else
				cx->Get(i)->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		case CChannel::DATA_JD:
			if (m_JDType==DATE_JD_GEOCENTRIC)
				cx->Get(i)->SetExportFlags(0);
			else
				cx->Get(i)->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		case CChannel::DATA_JD_HEL:
			if (m_JDType==DATE_JD_HELIOCENTRIC)
				cx->Get(i)->SetExportFlags(0);
			else
				cx->Get(i)->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		default:
			ch->SetExportFlags(CChannel::EXPORT_SKIP);
		}
	}

	CChannels *cy = m_Table.ChannelsY();
	for (int i=0; i<cy->Count(); i++) {
		CChannel *ch = m_Table.ChannelsY()->Get(i);
		switch (ch->Info())
		{
		case CChannel::DATA_MAGNITUDE:
			if (opt->all_values || i==m_SelectedY) {
				if (opt->errors) 
					ch->SetExportFlags(0);
				else
					ch->SetExportFlags(CChannel::EXPORT_VALUE_ONLY);
			} else 
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		case CChannel::DATA_HELCOR:
			if (opt->helcor)
				ch->SetExportFlags(0);
			else
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		case CChannel::DATA_AIRMASS:
			if (opt->airmass)
				ch->SetExportFlags(0);
			else
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		case CChannel::DATA_ALTITUDE:
			if (opt->altitude)
				ch->SetExportFlags(0);
			else
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		default:
			ch->SetExportFlags(CChannel::EXPORT_SKIP);
		}
	}

	if (m_JDType==DATE_JD_GEOCENTRIC)
		m_Table.SetParam("JD", "geocentric");
	else
		m_Table.SetParam("JD", "heliocentric");

	int res = 0;
	if (m_FileType == TYPE_MUNIPACK) {
		GError *error = NULL;
		if (!m_Table.Save(filename, &error)) {
			if (error) {
				ShowError(m_pParent, error->message, true);
				g_error_free(error);
			}
			res = -1;
		}
	} else {
		unsigned flags = 0;
		if (!opt->header)
			flags |= CTable::EXPORT_NO_HEADER;
		if (opt->skip_invalid)
			flags |= CTable::EXPORT_NULVAL_SKIP_ROW;
		if (opt->zero_invalid)
			flags |= CTable::EXPORT_NULVAL_ZERO;
		GError *error = NULL;
		if (!m_Table.ExportTable(filename, FileMimeType(m_FileType), flags, &error)) {
			if (error) {
				ShowError(m_pParent, error->message, true);
				g_error_free(error);
			}
			res = -1;
		}
	}
	g_free(filename);
	return res==0;
}

void CSaveLightCurveDlg::selection_changed(GtkComboBox *pWidget, CSaveLightCurveDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CSaveLightCurveDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
			}
		} else
		if (GTK_WIDGET(pWidget) == m_JDCombo) {
			int dt = SelectedItem(pWidget);
			if (dt!=m_JDType) {
				m_JDType = (tDateType)dt;
				UpdateControls();
			}
		} else
		if (GTK_WIDGET(pWidget) == m_VCCombo) {
			int ch = SelectedItem(pWidget);
			if (ch!=m_SelectedY) {
				m_SelectedY = ch;
				UpdateControls();
			}
		}
	}
}

void CSaveLightCurveDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_VCCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1 &&
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllValues)));
	gtk_widget_set_sensitive(m_JDCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_JDTypes), NULL)>1);
	gtk_widget_set_sensitive(m_AllValues, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT) &&
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1);
	gtk_widget_set_sensitive(m_FrameIds, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_Errors, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_Header, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_SkipInvalid, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_ZeroInvalid, m_InstMag && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT) && 
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SkipInvalid)));
	gtk_widget_set_sensitive(m_Altitude, m_HaveAltitude && 
		(m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK));
	gtk_widget_set_sensitive(m_AirMass, m_HaveAirMass && 
		(m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK));
	gtk_widget_set_sensitive(m_HelCor, m_HaveHelCor && 
		(m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK));
}

void CSaveLightCurveDlg::OnTypeChanged(void)
{
	// Change file filters
	GSList *list = gtk_file_chooser_list_filters(GTK_FILE_CHOOSER(m_pDlg));
	for (GSList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(m_pDlg), (GtkFileFilter*)ptr->data);
	g_slist_free(list);
	GtkFileFilter *type_filter = FileFilter(m_FileType);
	if (type_filter)
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), type_filter);
	GtkFileFilter *all_files = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_files, "*");
	gtk_file_filter_set_name(all_files, "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), all_files);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), (type_filter ? type_filter : all_files));
	
	// Change file's extension
	const gchar *ext = FileExtension(m_FileType);
	if (ext) {
		gchar *oldname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		if (oldname) {
			gchar *newname = SetFileExtension(oldname, ext);
			gchar *basename = g_path_get_basename(newname);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), basename);
			g_free(basename);
			g_free(newname);
			g_free(oldname);
		}
	}

	m_Updating = true;

	// Column selection
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), NULL);
	gtk_list_store_clear(m_Channels);
	if (m_FileType == TYPE_MUNIPACK) {
		// Show [All values] only
		for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
			CChannel *ch = m_Table.ChannelsY()->Get(i);
			if (ch && ch->Info() == CChannel::DATA_MAGNITUDE) {
				GtkTreeIter iter;
				gtk_list_store_append(m_Channels, &iter);
				gtk_list_store_set(m_Channels, &iter, 0, -1, 1, "[All values]", -1);
				break;
			}
		}
	} else {
		// Show list of available channels
		for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
			CChannel *ch = m_Table.ChannelsY()->Get(i);
			if (ch && ch->Info() == CChannel::DATA_MAGNITUDE) {
				GtkTreeIter iter;
				gtk_list_store_append(m_Channels, &iter);
				gtk_list_store_set(m_Channels, &iter, 0, i, 1, ch->Name(), -1);
			}
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), GTK_TREE_MODEL(m_Channels));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>0) {
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_VCCombo))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_VCCombo), 0);
			if (m_FileType == TYPE_MUNIPACK)
				m_SelectedY = SelectedItem(GTK_COMBO_BOX(m_VCCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_VCCombo), -1);
		if (m_FileType == TYPE_MUNIPACK)
			m_SelectedY = -1;
	}
	OnSelectionChanged(GTK_COMBO_BOX(m_VCCombo));

	// Options
	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FrameIds), opt->frame_id);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllValues), opt->all_values);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Errors), opt->errors);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ZeroInvalid), m_InstMag && opt->zero_invalid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_HelCor), m_HaveHelCor && opt->helcor);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AirMass), m_HaveAirMass && opt->airmass);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Altitude), m_HaveAltitude && opt->altitude);

	m_Updating = false;

	UpdateControls();
}

void CSaveLightCurveDlg::button_toggled(GtkToggleButton *pWidget, CSaveLightCurveDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CSaveLightCurveDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_HelCor) {
		if (m_HaveHelCor && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK))
			m_Options[m_FileType].helcor = gtk_toggle_button_get_active(pWidget)!=0;
	} else 
	if (GTK_WIDGET(pWidget) == m_AirMass) {
		if (m_HaveAirMass && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK))
			m_Options[m_FileType].airmass = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Altitude) {
		if (m_HaveAltitude && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT || m_FileType==TYPE_MUNIPACK))
			m_Options[m_FileType].altitude = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_ZeroInvalid) {
		if (m_InstMag && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT))
			m_Options[m_FileType].zero_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Errors) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].errors = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_FrameIds) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].frame_id = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_AllValues) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].all_values = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CSaveLightCurveDlg::response_dialog(GtkWidget *pDlg, gint response_id, CSaveLightCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CSaveLightCurveDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_TABLE);
		return false;
	}
	return true;
}
