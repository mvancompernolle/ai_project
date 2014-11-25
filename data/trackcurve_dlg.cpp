/**************************************************************

tracklist_dlg.cpp (C-Munipack project)
The 'Plot track-list' dialog
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
#include "trackcurve_dlg.h"
#include "frameinfo_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"
#include "export_dlgs.h"

//-------------------------   LOCAL MENU   --------------------------------

enum tMenuId
{
	MENU_FILE = 1,
	MENU_VIEW,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_CLOSE = 100,
	CMD_EXPORT,
	CMD_REBUILD,
	CMD_PRINT,
	CMD_SAVE,
	CMD_HELP,
	CMD_HIDE,
	CMD_DELETE,
	CMD_PREVIEW,
	CMD_FRAMEINFO,
	CMD_GRAPH,
	CMD_TABLE,
	CMD_GRID,
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

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN,	CMD_GRAPH,		"_Graph" },
	{ CMenuBar::MB_RADIOBTN,	CMD_TABLE,		"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN,	CMD_GRID,		"Gri_d" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN,	CMD_STATISTICS,	"_Statistics" },
	{ CMenuBar::MB_CHECKBTN,	CMD_MEASUREMENT, "_Measurement" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_HELP,		"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu TrackListMenu[] = {
	{ "_File",	MENU_FILE,	FileMenu },
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

//-------------------------   TRACK CURVE WINDOW   --------------------------------

CTrackCurveDlg::CTrackCurveDlg(void):m_UpdatePos(true), m_LastPosValid(false), 
	m_LastPosX(0), m_LastPosY(0), m_LastFocus(-1), m_InFiles(0), m_OutFiles(0), 
	m_Column(-1), m_ChannelX(-1), m_ChannelY(-1), m_Table(NULL), m_InfoMode(INFO_NONE), 
	m_GraphData(NULL), m_TableData(NULL)
{
	gchar buf[512];
	GtkWidget *tbox;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	m_ShowGrid	 = g_Project->GetBool("TrackCurve", "Grid", false);
	m_DateFormat = (tDateFormat)g_Project->GetInt("Display", "DateFormat", JULIAN_DATE, 0, GREGORIAN_DATE);
	m_DispMode	 = (tDisplayMode)g_Project->GetInt("TrackCurve", "Display", DISPLAY_GRAPH, 0, DISPLAY_TABLE);

	// Dialog caption
	sprintf(buf, "%s - %s", "Track curve", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Menu bar
	m_Menu.Create(TrackListMenu, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), FALSE, FALSE, 0);

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(m_MainBox), tbox, false, false, 0);

	m_XLabel = toolbar_new_label(tbox, "X axis");
	m_XChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_list_store_append(m_XChannels, &iter);
	gtk_list_store_set(m_XChannels, &iter, 0, JULIAN_DATE, 1, "JD", -1);
	gtk_list_store_append(m_XChannels, &iter);
	gtk_list_store_set(m_XChannels, &iter, 0, GREGORIAN_DATE, 1, "UTC", -1);
	m_XCombo = toolbar_new_combo(tbox, "Column shown on the horizontal axis of the graph");
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_XCombo), GTK_TREE_MODEL(m_XChannels));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_XCombo), m_DateFormat);
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

	toolbar_new_separator(tbox);

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
	
	// Info box
	m_InfoBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_InfoBox.Handle(), FALSE, FALSE, 0);
	m_InfoBox.SetCaption("Statistics");

	// Measurement tool
	m_MeasBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_MeasBox.Handle(), FALSE, FALSE, 0);
	m_MeasBox.SetGraphView(CMPACK_GRAPH_VIEW(m_GraphView));

	// Timers
	m_TimerId = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 100, GSourceFunc(timer_cb), this, NULL);

	// Popup menu
	m_GraphMenu.Create(GraphContextMenu);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	SetDisplayMode(m_DispMode);
	SetInfoMode(m_InfoMode);
}

CTrackCurveDlg::~CTrackCurveDlg()
{
	// Disconnect graph signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)mouse_moved, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)button_press_event, this);

	// Release objects, free allocated memory
	g_source_remove(m_TimerId);
	g_object_unref(m_XChannels);
	g_object_unref(m_YChannels);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	if (m_TableData)
		g_object_unref(m_TableData);
	delete m_Table;
}

bool CTrackCurveDlg::Make(GtkWindow *parent, bool selected_files)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;
	m_FrameSet.Clear();
	m_ChannelX = m_ChannelY = -1;
	m_DataSaved = false;

	if (!InitFileList(parent, selected_files))
		return false;

	if (!RebuildData(parent))
		return false;

	SetInfoMode(INFO_NONE);
	UpdateTrackCurve();
	UpdateChannels();
	UpdateGraphTable(TRUE, TRUE);
	UpdateTools();
	SetStatus(NULL);
	UpdateControls();
	return true;
}

void CTrackCurveDlg::OnFrameSetChanged(void)
{
	UpdateTrackCurve();
	UpdateGraphTable(TRUE, TRUE);
	UpdateStatus();
	UpdateTools();
	UpdateControls();
}

void CTrackCurveDlg::UpdateTrackCurve(void)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;
	if (CmpackTrackCurve(NULL, &m_Table, m_FrameSet)==0) 
		m_MeasBox.SetTable(m_Table);
}

void CTrackCurveDlg::UpdateControls()
{
	gtk_widget_set_sensitive(m_XLabel, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_XChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_XCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_XChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_YLabel, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_YChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_YCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_YChannels), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_ZoomLabel, m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_GRAPH, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_TABLE, m_DispMode==DISPLAY_TABLE);
	m_Menu.Enable(CMD_GRID, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Enable(CMD_EXPORT, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_GRID, m_ShowGrid);
	m_Menu.Enable(CMD_STATISTICS, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_STATISTICS, m_InfoMode == INFO_STATISTICS);
	m_Menu.Enable(CMD_MEASUREMENT, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_MEASUREMENT, m_InfoMode == INFO_MEASUREMENT);
}

void CTrackCurveDlg::UpdateGraphTable(gboolean autozoom_x, gboolean autozoom_y)
{
	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), NULL);
	if (m_GraphData) {
		g_object_unref(m_GraphData);
		m_GraphData = NULL;
	}
	if (m_Table && m_ChannelX>=0 && m_ChannelY>=0) {
		m_GraphData = m_Table->ToGraphData(m_ChannelX, m_ChannelY);
		m_Table->SetView(CMPACK_GRAPH_VIEW(m_GraphView), m_ChannelX, m_ChannelY, false, NULL, NULL, m_DateFormat);
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

void CTrackCurveDlg::UpdateChannels(void)
{
	// First JD column is on horizontal axis
	CChannels *cx = m_Table->ChannelsX();
	for (int i=0; i<cx->Count(); i++) {
		if (cx->GetInfo(i) == CChannel::DATA_JD) {
			m_ChannelX = i;
			break;
		}
	}
	
	// Update list of dependent channels
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
}

void CTrackCurveDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CTrackCurveDlg *pMe = (CTrackCurveDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CTrackCurveDlg::OnCommand(int cmd_id)
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

	// View menu
	case CMD_GRAPH:
		SetDisplayMode(DISPLAY_GRAPH);
		break;
	case CMD_TABLE:
		SetDisplayMode(DISPLAY_TABLE);
		break;
	case CMD_GRID:
		m_ShowGrid = m_Menu.IsChecked(CMD_GRID);
		cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), m_ShowGrid, m_ShowGrid);
		g_Project->SetBool("TrackCurve", "Grid", m_ShowGrid);
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_TRACK_CURVE);
		break;
	}
}

void CTrackCurveDlg::entry_changed(GtkWidget *pBtn, CTrackCurveDlg *pMe)
{
	pMe->OnEntryChanged(pBtn);
}

void CTrackCurveDlg::OnEntryChanged(GtkWidget *pBtn)
{
	if (pBtn==m_XCombo) {
		int ch = SelectedItem(GTK_COMBO_BOX(m_XCombo));
		if (ch>=0 && ch!=m_DateFormat) {
			m_DateFormat = (tDateFormat)ch;
			g_Project->SetInt("Display", "DateFormat", m_DateFormat);
			UpdateGraphTable(FALSE, FALSE);
			UpdateTools();
			UpdateControls();
		}
	} else if (pBtn==m_YCombo) {
		int ch = SelectedItem(GTK_COMBO_BOX(m_YCombo));
		if (ch>=0 && ch!=m_ChannelY) {
			m_ChannelY = ch;
			UpdateGraphTable(FALSE, TRUE);
			UpdateTools();
			UpdateControls();
		}
	}
}

void CTrackCurveDlg::button_clicked(GtkWidget *button, CTrackCurveDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CTrackCurveDlg::OnButtonClicked(GtkWidget *pBtn)
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

void CTrackCurveDlg::mouse_moved(GtkWidget *button, CTrackCurveDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

//
// Mouse button handler
//
gint CTrackCurveDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CTrackCurveDlg *pMe)
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

//
// Show context menu
//
void CTrackCurveDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
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

gboolean CTrackCurveDlg::timer_cb(CTrackCurveDlg *pDlg)
{
	if (pDlg->m_UpdatePos) {
		pDlg->m_UpdatePos = false;
		pDlg->UpdateStatus();
	}
	return TRUE;
}

void CTrackCurveDlg::selection_changed(GtkWidget *pChart, CTrackCurveDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

//
// Left button click
//
void CTrackCurveDlg::OnSelectionChanged(void)
{
	UpdateTools();
	UpdateStatus();
}

void CTrackCurveDlg::UpdateStatus(void)
{
	char	msg[256], buf1[256], buf2[256];
	int		frame_id, xcol, ycol;
	gdouble	dx, dy;
	GtkTreeIter iter;

	if (!m_GraphData)
		return;

	if (m_DispMode == DISPLAY_GRAPH) {
		int item = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(m_GraphView));
		if (item>=0) {
			if (m_LastFocus!=item && m_Table) {
				m_LastFocus = item;
				frame_id = (int)cmpack_graph_data_get_param(m_GraphData, item);
				xcol = m_Table->ChannelsX()->GetColumn(m_ChannelX);
				ycol = m_Table->ChannelsY()->GetColumn(m_ChannelY);
				if (m_Table->Find(frame_id) && m_Table->GetDbl(xcol, &dx) && m_Table->GetDbl(ycol, &dy)) {
					PrintKeyValue(buf1, dx, m_Table->ChannelsX()->Get(m_ChannelX));
					PrintKeyValue(buf2, dy, m_Table->ChannelsY()->Get(m_ChannelY));
					sprintf(msg, "Frame #%d: %s, %s", frame_id, buf1, buf2);
				} else {
					sprintf(msg, "Frame #%d", frame_id);
				}
				SetStatus(msg);
			}
		} else {
			m_LastFocus = -1;
			if (m_Table && m_ChannelY>=0 && m_ChannelY<m_Table->ChannelsY()->Count() &&
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
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
		int count = gtk_tree_selection_count_selected_rows(selection);
		if (count>1) {
			sprintf(msg, "%d rows selected", count);
			SetStatus(msg);
		} else 
		if (count==1) {
			GList *list = gtk_tree_selection_get_selected_rows(selection, NULL);
			gtk_tree_model_get_iter(m_TableData, &iter, (GtkTreePath*)list->data);
			gtk_tree_model_get(m_TableData, &iter, 0, &frame_id, -1);
			sprintf(msg, "Frame #%d", frame_id);
			SetStatus(msg);
			g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
			g_list_free (list);
		} else {
			SetStatus(NULL);
		}
	}
}

void CTrackCurveDlg::SetDisplayMode(tDisplayMode mode)
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
	g_Project->SetInt("TrackCurve", "Display", m_DispMode);
	UpdateControls();
}

void CTrackCurveDlg::PrintValue(char *buf, double value, const CChannel *channel)
{
	const gchar *name = channel->Name();

	if (strcmp(name, "JD")==0 && m_DateFormat != JULIAN_DATE) {
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

void CTrackCurveDlg::PrintKeyValue(char *buf, double value, const CChannel *channel)
{
	const gchar *name = channel->Name();

	if (strcmp(name, "JD")==0 && m_DateFormat != JULIAN_DATE) {
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

void CTrackCurveDlg::UpdateTools(void)
{
	char	buf[256], msg[256];
	double	dx, dy, dmin, dmax;
	int		xcol, ycol;

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
				sprintf(msg, "Data %s:", (m_DateFormat==JULIAN_DATE ? "JD" : "UTC"));
				m_InfoBox.AddTitle(1, msg);
				PrintValue(buf, dmin, m_Table->ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_Table->ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Max.", buf);
				m_InfoBox.AddText(msg);
			}
			if (ComputeMinMax(count, y, &dmin, &dmax)) {
				m_InfoBox.AddText("");
				sprintf(buf, "Data %s:", m_Table->ChannelsY()->GetName(m_ChannelY));
				m_InfoBox.AddTitle(1, buf);
				PrintValue(buf, dmin, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_Table->ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Max.", buf);
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

void CTrackCurveDlg::SetInfoMode(tInfoMode mode)
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

void CTrackCurveDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CTrackCurveDlg *pMe = (CTrackCurveDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CTrackCurveDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateControls();
}

void CTrackCurveDlg::SaveData(void)
{
	CSaveTrackCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	if (pDlg.Execute(*m_Table, m_ChannelY)) {
		m_DataSaved = true;
		UpdateControls();
	}
}

void CTrackCurveDlg::Export(void)
{
	CGraphExportDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(CMPACK_GRAPH_VIEW(m_GraphView), "trackcurve");
}

GList *CTrackCurveDlg::GetSelectedFrames(void)
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

GtkTreePath *CTrackCurveDlg::GetSelectedFrame(void)
{
	int row = cmpack_graph_view_get_selected(CMPACK_GRAPH_VIEW(m_GraphView));
	if (row>=0 && m_GraphData) {
		int frame = (gint)cmpack_graph_data_get_param(m_GraphData, row);
		return g_Project->GetFilePath(frame);
	}
	return NULL;
}

//------------------------   SAVE AIR MASS CURVE   ---------------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
} FileFormats[] = {
	{ "CMPACK",	"C-Munipack (default)",				"txt",	NULL,			"C-Munipack files", "*.txt" },
	{ "TEXT",	"Text (space separated values)",	"txt",	"text/plain",	"Text files",		"*.txt" },
	{ "CSV",	"CSV (comma separated values)",		"csv",	"text/csv",		"CSV files",		"*.csv" },
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

//
// Constructor
//
CSaveTrackCurveDlg::CSaveTrackCurveDlg(GtkWindow *pParent):m_pParent(pParent),
	m_Updating(false), m_FileType(TYPE_MUNIPACK), m_SelectedY(-1)
{
	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));
	m_Options[TYPE_MUNIPACK].header = m_Options[TYPE_MUNIPACK].all_values = true;
	
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Save track curve", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("trackcurve");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Options
	GtkWidget *frame = gtk_frame_new("Export options");
	GtkWidget *hbox = gtk_hbox_new(TRUE, 8);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);

	// File format
	GtkWidget *lbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), lbox, TRUE, TRUE, 0);
	m_FileTypes = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	GtkWidget *label = gtk_label_new("File type");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(lbox), label, FALSE, TRUE, 0);
	m_TypeCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_FileTypes));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
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
	m_AllValues = gtk_check_button_new_with_label("Export all values");
	gtk_box_pack_start(GTK_BOX(rbox), m_AllValues, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_AllValues), "toggled", G_CALLBACK(button_toggled), this);
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CSaveTrackCurveDlg::~CSaveTrackCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
	if (m_Channels)
	g_object_unref(m_Channels);
}

CSaveTrackCurveDlg::tFileType CSaveTrackCurveDlg::StrToFileType(const gchar *str)
{
	if (str) {
		for (gint i=0; FileFormats[i].Id!=NULL; i++) {
			if (strcmp(FileFormats[i].Id, str)==0)
				return (tFileType)i;
		}
	}
	return TYPE_MUNIPACK;
}

const gchar *CSaveTrackCurveDlg::FileTypeToStr(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].Id;
	return "";
}

bool CSaveTrackCurveDlg::Execute(const CTable &table, int ychannel)
{
	m_Table.MakeCopy(table);
	m_SelectedY = ychannel;

	char *aux = g_Project->GetStr("TrackCurve", "FileType");
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
	gchar *filename = g_Project->GetStr("TrackCurve", "File", "track.txt");
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

	// File types
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), NULL);
	gtk_list_store_clear(m_FileTypes);
	for (int i=0; FileFormats[i].Caption; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(m_FileTypes, &iter);
		gtk_list_store_set(m_FileTypes, &iter, 0, i, 1, FileFormats[i].Caption, -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), GTK_TREE_MODEL(m_FileTypes));
	SelectItem(GTK_COMBO_BOX(m_TypeCombo), m_FileType);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_TypeCombo))<0) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), 0);
		m_FileType = (tFileType)SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
	}
	gtk_widget_set_sensitive(m_TypeCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_FileTypes), NULL)>1);

	// Restore options (format dependent customizable options)
	m_Options[TYPE_CSV].frame_id = g_Project->GetBool("TrackCurve", "CSV_FRAME_ID", true);
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("TrackCurve", "CSV_SKIP_INVALID", true);
	m_Options[TYPE_CSV].header = g_Project->GetBool("TrackCurve", "CSV_HEADER", true);
	m_Options[TYPE_CSV].all_values = g_Project->GetBool("TrackCurve", "CSV_ALL_VALUES");
	m_Options[TYPE_TEXT].frame_id = g_Project->GetBool("TrackCurve", "TEXT_FRAME_ID", true);
	m_Options[TYPE_TEXT].skip_invalid = g_Project->GetBool("TrackCurve", "TEXT_SKIP_INVALID", true);
	m_Options[TYPE_TEXT].header = g_Project->GetBool("TrackCurve", "TEXT_HEADER", true);
	m_Options[TYPE_TEXT].all_values = g_Project->GetBool("TrackCurve", "TEXT_ALL_VALUES");

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
	g_Project->SetStr("TrackCurve", "File", basename);
	g_free(basename);

	// Save settings
	g_Project->SetStr("TrackCurve", "FileType", FileTypeToStr(m_FileType));
	g_Project->SetBool("TrackCurve", "CSV_FRAME_ID", m_Options[TYPE_CSV].frame_id);
	g_Project->SetBool("TrackCurve", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("TrackCurve", "CSV_HEADER", m_Options[TYPE_CSV].header);
	g_Project->SetBool("TrackCurve", "CSV_ALL_VALUES", m_Options[TYPE_CSV].all_values);
	g_Project->SetBool("TrackCurve", "TEXT_FRAME_ID", m_Options[TYPE_TEXT].frame_id);
	g_Project->SetBool("TrackCurve", "TEXT_SKIP_INVALID", m_Options[TYPE_TEXT].skip_invalid);
	g_Project->SetBool("TrackCurve", "TEXT_HEADER", m_Options[TYPE_TEXT].header);
	g_Project->SetBool("TrackCurve", "TEXT_ALL_VALUES", m_Options[TYPE_TEXT].all_values);

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
			cx->Get(i)->SetExportFlags(0);
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
		case CChannel::DATA_OFFSET:
			if (opt->all_values || i==m_SelectedY) 
				ch->SetExportFlags(0);
			else 
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		default:
			ch->SetExportFlags(CChannel::EXPORT_SKIP);
		}
	}

	m_Table.SetParam("JD", "geocentric");

	int res = 0;
	if (m_FileType == TYPE_MUNIPACK) {
		GError *error = NULL;
		if (!m_Table.Save(filename, &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
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
		else
			flags |= CTable::EXPORT_NULVAL_ZERO;
		GError *error = NULL;
		if (!m_Table.ExportTable(filename, FileMimeType(m_FileType), flags, &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
			}
			res = -1;
		}
	}
	g_free(filename);
	return (res==0);
}

void CSaveTrackCurveDlg::selection_changed(GtkComboBox *pWidget, CSaveTrackCurveDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CSaveTrackCurveDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
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

void CSaveTrackCurveDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_VCCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1 &&
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllValues)));
	gtk_widget_set_sensitive(m_AllValues, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT) &&
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1);
	gtk_widget_set_sensitive(m_FrameIds, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_Header, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_SkipInvalid, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
}

void CSaveTrackCurveDlg::OnTypeChanged(void)
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

	// Set file's extension
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

	// Y columns
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), NULL);
	gtk_list_store_clear(m_Channels);
	if (m_FileType == TYPE_MUNIPACK) {
		// Show [All values] only
		for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
			CChannel *ch = m_Table.ChannelsY()->Get(i);
			if (ch && (ch->Info() == CChannel::DATA_OFFSET)) {
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
			if (ch && (ch->Info() == CChannel::DATA_OFFSET)) {
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

	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FrameIds), opt->frame_id);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllValues), opt->all_values);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);

	m_Updating = false;

	UpdateControls();
}

void CSaveTrackCurveDlg::button_toggled(GtkToggleButton *pWidget, CSaveTrackCurveDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CSaveTrackCurveDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
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

void CSaveTrackCurveDlg::response_dialog(GtkWidget *pDlg, gint response_id, CSaveTrackCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CSaveTrackCurveDlg::OnResponseDialog(gint response_id)
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
