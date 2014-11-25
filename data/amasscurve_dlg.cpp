/**************************************************************

amasslist_dlg.cpp (C-Munipack project)
The 'Plot deviations' dialog
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
#include "amasscurve_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "frameinfo_dlg.h"
#include "ctxhelp.h"
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
	CMD_CLOSE = 100,
	CMD_EXPORT,
	CMD_REBUILD,
	CMD_PROPERTIES,
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

static const CMenuBar::tMenuItem EditMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_PROPERTIES,	"Edit _properties" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN,	CMD_GRAPH,		"_Graph" },
	{ CMenuBar::MB_RADIOBTN,	CMD_TABLE,		"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN,	CMD_GRID,		"_Grid" },
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

static const CMenuBar::tMenu AMassCurveMenu[] = {
	{ "_File",	MENU_FILE,	FileMenu },
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

//-------------------------   AIR MASS CURVE DIALOG   --------------------------------

CAMassCurveDlg::CAMassCurveDlg(void):m_UpdatePos(true), m_LastPosValid(false), 
	m_LastPosX(0), m_LastPosY(0), m_LastFocus(-1), m_InFiles(0), m_OutFiles(0), 
	m_ChannelX(-1), m_ChannelY(-1), m_Table(NULL), m_InfoMode(INFO_NONE), 
	m_GraphData(NULL), m_TableData(NULL)
{
	gchar buf[512];
	GtkWidget *tbox;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	m_RawFiles = true;

	m_ShowGrid   = g_Project->GetBool("AirMass", "Grid", false);
	m_DateFormat = (tDateFormat)g_Project->GetInt("Display", "DateFormat", JULIAN_DATE, 0, GREGORIAN_DATE);
	m_DispMode   = (tDisplayMode)g_Project->GetInt("AirMass", "Display", DISPLAY_GRAPH, 0, DISPLAY_TABLE);

	// Dialog caption
	sprintf(buf, "%s - %s", "Air mass curve", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Menu bar
	m_Menu.Create(AMassCurveMenu, false);
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

	// Graph data
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
	g_signal_connect(G_OBJECT(m_TableView), "button_press_event", G_CALLBACK(button_press_event), this);
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
	SetInfoMode(INFO_NONE);
}

CAMassCurveDlg::~CAMassCurveDlg()
{
	// Disconnect graph signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)mouse_moved, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)button_press_event, this);

	// Release objects, free allocated memory
	g_source_remove(m_TimerId);
	g_object_unref(m_DateFormats);
	g_object_unref(m_YChannels);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	if (m_TableData)
		g_object_unref(m_TableData);
	delete m_Table;
}

bool CAMassCurveDlg::Make(GtkWindow *parent, bool selected_files, 
	const CObjectCoords &obj, const CLocation &loc)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;
	m_FrameSet.Clear();
	m_ChannelX = m_ChannelY = -1;
	m_DataSaved = false;
	m_ObjCoords = obj;
	m_Location = loc;

	if (!InitFileList(parent, selected_files))
		return false;

	if (!RebuildData(parent))
		return false;

	UpdateAirMassCurve();
	UpdateChannels();
	UpdateGraphTable(TRUE, TRUE);
	UpdateTools();
	SetInfoMode(INFO_NONE);
	SetStatus(NULL);
	UpdateControls();
	return true;
}

void CAMassCurveDlg::OnFrameSetChanged(void)
{
	UpdateAirMassCurve();
	UpdateGraphTable(TRUE, TRUE);
	UpdateStatus();
	UpdateTools();
	UpdateControls();
}

void CAMassCurveDlg::UpdateAirMassCurve(void)
{
	m_MeasBox.SetTable(NULL);
	delete m_Table;
	m_Table = NULL;
	if (CmpackAirMassCurve(NULL, &m_Table, m_FrameSet, m_ObjCoords, m_Location)==0) 
		m_MeasBox.SetTable(m_Table);
}

void CAMassCurveDlg::UpdateControls()
{
	gtk_widget_set_sensitive(m_XLabel, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_DateFormats), NULL)>1 &&
		m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(m_DCombo, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_DateFormats), NULL)>1 &&
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

void CAMassCurveDlg::UpdateGraphTable(gboolean autozoom_x, gboolean autozoom_y)
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

void CAMassCurveDlg::UpdateChannels(void)
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

void CAMassCurveDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CAMassCurveDlg *pMe = (CAMassCurveDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CAMassCurveDlg::OnCommand(int cmd_id)
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
		g_Project->SetBool("AirMass", "Grid", m_ShowGrid);
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_AIR_MASS_CURVE);
		break;
	}
}

void CAMassCurveDlg::entry_changed(GtkWidget *pBtn, CAMassCurveDlg *pMe)
{
	pMe->OnEntryChanged(pBtn);
}

void CAMassCurveDlg::OnEntryChanged(GtkWidget *pBtn)
{
	if (pBtn==m_DCombo) {
		int format = SelectedItem(GTK_COMBO_BOX(m_DCombo));
		if (format>=0 && format!=m_DateFormat) {
			m_DateFormat = (tDateFormat)format;
			g_Project->SetInt("Display", "DateFormat", m_DateFormat);
			UpdateGraphTable(FALSE, FALSE);
			UpdateTools();
			UpdateControls();
		}
	} else if (pBtn == m_YCombo) {
		int channel = SelectedItem(GTK_COMBO_BOX(m_YCombo));
		if (channel>=0 && channel!=m_ChannelY) {
			m_ChannelY = channel;
			UpdateGraphTable(FALSE, TRUE);
			UpdateTools();
			UpdateControls();
		}
	}
}

void CAMassCurveDlg::mouse_moved(GtkWidget *button, CAMassCurveDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

//
// Mouse button handler
//
gint CAMassCurveDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CAMassCurveDlg *pMe)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		if (widget==pMe->m_GraphView) {
			int focused = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(widget));
			if (focused>=0 && !cmpack_graph_view_is_selected(CMPACK_GRAPH_VIEW(widget), focused)) {
				cmpack_graph_view_unselect_all(CMPACK_GRAPH_VIEW(widget));
				cmpack_graph_view_select(CMPACK_GRAPH_VIEW(widget), focused);
			}
		} else
		if (widget==pMe->m_TableView) {
			GtkTreePath *path;
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) {
				gtk_widget_grab_focus(widget);
				GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
				if (gtk_tree_selection_count_selected_rows(sel)<=1)
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, NULL, FALSE);
				gtk_tree_path_free(path);
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
void CAMassCurveDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int res, selected;

	if (m_DispMode == DISPLAY_GRAPH) {
		selected = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_GraphView));
	} else {
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView));
		selected = gtk_tree_selection_count_selected_rows(selection);
	}
	if (selected>0) {
		m_GraphMenu.Enable(CMD_DELETE, true);
		m_GraphMenu.Enable(CMD_HIDE, true);
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

gboolean CAMassCurveDlg::timer_cb(CAMassCurveDlg *pDlg)
{
	if (pDlg->m_UpdatePos) {
		pDlg->m_UpdatePos = false;
		pDlg->UpdateStatus();
	}
	return TRUE;
}

void CAMassCurveDlg::selection_changed(GtkWidget *pChart, CAMassCurveDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

void CAMassCurveDlg::OnSelectionChanged(void)
{
	UpdateTools();
	UpdateStatus();
}

void CAMassCurveDlg::UpdateStatus(void)
{
	char	msg[256], buf1[256], buf2[256];
	gdouble	dx, dy;
	int		frame_id;

	if (m_DispMode == DISPLAY_GRAPH) {
		int item = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(m_GraphView));
		if (item>=0) {
			if (m_LastFocus!=item && m_GraphData && m_Table) {
				m_LastFocus = item;
				int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, item),
					xcol = m_Table->ChannelsX()->GetColumn(m_ChannelX),
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
		GList *list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView)), NULL);
		if (list && m_TableData) {
			int count = g_list_length(list);
			if (count>1) {
				sprintf(msg, "%d rows selected", count);
				SetStatus(msg);
			} else {
				GtkTreeIter iter;
				if (gtk_tree_model_get_iter(m_TableData, &iter, (GtkTreePath*)list->data)) {
					gtk_tree_model_get(m_TableData, &iter, 0, &frame_id, -1);
					sprintf(msg, "Frame #%d", frame_id);
					SetStatus(msg);
				} else
					SetStatus(NULL);
			}
		} else 
			SetStatus(NULL);
		g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (list);
	}
}

void CAMassCurveDlg::SetDisplayMode(tDisplayMode mode)
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
	g_Project->SetInt("AirMass", "Display", m_DispMode);
	UpdateControls();
}

void CAMassCurveDlg::PrintValue(char *buf, double value, const CChannel *channel)
{
	CChannel::tChannelInfo info = channel->Info();
	if ((info == CChannel::DATA_JD || (info == CChannel::DATA_JD_HEL)) && m_DateFormat != JULIAN_DATE) {
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

void CAMassCurveDlg::PrintKeyValue(char *buf, double value, const CChannel *channel)
{
	CChannel::tChannelInfo info = channel->Info();
	if ((info == CChannel::DATA_JD || (info == CChannel::DATA_JD_HEL)) && m_DateFormat != JULIAN_DATE) {
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

void CAMassCurveDlg::UpdateTools(void)
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
				sprintf(msg, "Data %s:", m_Table->ChannelsY()->GetName(m_ChannelY));
				m_InfoBox.AddTitle(1, msg);
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

void CAMassCurveDlg::SetInfoMode(tInfoMode mode)
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

void CAMassCurveDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CAMassCurveDlg *pMe = (CAMassCurveDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CAMassCurveDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateControls();
}

void CAMassCurveDlg::button_clicked(GtkWidget *button, CAMassCurveDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CAMassCurveDlg::OnButtonClicked(GtkWidget *pBtn)
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

void CAMassCurveDlg::EditProperties(void)
{
	CAMassCurveOptionsDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(m_ObjCoords, m_Location)) {
		m_DataSaved = false;
		g_Project->SetObjectCoords(m_ObjCoords);
		CConfig::SetLastObject(m_ObjCoords);
		g_Project->SetLocation(m_Location);
		CConfig::SetLastLocation(m_Location);
		UpdateAirMassCurve();
		UpdateChannels();
		UpdateGraphTable(TRUE, TRUE);
		UpdateTools();
		SetStatus(NULL);
		UpdateControls();
	}
}

void CAMassCurveDlg::SaveData(void)
{
	CSaveAirMassCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	if (pDlg.Execute(*m_Table, m_ChannelY)) {
		m_DataSaved = true;
		UpdateControls();
	}
}

void CAMassCurveDlg::Export(void)
{
	CGraphExportDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(CMPACK_GRAPH_VIEW(m_GraphView), "airmass");
}

GList *CAMassCurveDlg::GetSelectedFrames(void)
{
	GList *frames = NULL;

	if (m_DispMode == DISPLAY_GRAPH) {
		GList *rows = cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_GraphView));
		if (rows && m_GraphData) {
			for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
				int frame = (gint)cmpack_graph_data_get_param(m_GraphData, (gint)(intptr_t)ptr->data);
				frames = g_list_prepend(frames, g_Project->GetFilePath(frame));
			}
			g_list_free(rows);
		}
	} else {
		GList *rows = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView)), NULL);
		if (rows && m_TableData) {
			GtkTreeIter iter;
			if (gtk_tree_model_get_iter(m_TableData, &iter, (GtkTreePath*)rows->data)) {
				int frame_id;
				gtk_tree_model_get(m_TableData, &iter, 0, &frame_id, -1);
				frames = g_list_prepend(frames, g_Project->GetFilePath(frame_id));
			}
		}
		g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (rows);
	}
	return g_list_reverse(frames);
}

GtkTreePath *CAMassCurveDlg::GetSelectedFrame(void)
{
	GtkTreePath *retval = NULL;
	if (m_DispMode == DISPLAY_GRAPH) {
		GList *rows = cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_GraphView));
		if (rows && m_GraphData) {
			int frame = (gint)cmpack_graph_data_get_param(m_GraphData, (gint)(intptr_t)rows->data);
			retval = g_Project->GetFilePath(frame);
		}
		g_list_free(rows);
	} else {
		GList *rows = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView)), NULL);
		if (rows && m_TableData) {
			GtkTreeIter iter;
			if (gtk_tree_model_get_iter(m_TableData, &iter, (GtkTreePath*)rows->data)) {
				int frame_id;
				gtk_tree_model_get(m_TableData, &iter, 0, &frame_id, -1);
				retval = g_Project->GetFilePath(frame_id);
			}
		}
		g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (rows);
	}
	return retval;
}

//------------------------   AIR MASS CURVE OPTIONS   ---------------------------------

CAMassCurveOptionsDlg::CAMassCurveOptionsDlg(GtkWindow *pParent)
{
	GtkWidget *tbox;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Air mass curve options", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("airmasscurve");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	tbox = gtk_table_new(6, 4, FALSE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 12);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);

	// Object coordinates
	m_ObjLabel = gtk_label_new("Object - designation");
	gtk_misc_set_alignment(GTK_MISC(m_ObjLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjLabel, 0, 1, 0, 1);
	m_ObjName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_ObjName, "Object's designation, e.g. RT And");
	gtk_entry_set_max_length(GTK_ENTRY(m_ObjName), MAX_OBJECT_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjName, 1, 2, 0, 1);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 2, 3, 0, 1);
	m_RALabel = gtk_label_new("- right ascension");
	gtk_misc_set_alignment(GTK_MISC(m_RALabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RALabel, 0, 1, 1, 2);
	m_RA = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_RA), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 1, 2);
	m_RAUnit = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(m_RAUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RAUnit, 2, 3, 1, 2);
	m_DecLabel = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(m_DecLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecLabel, 0, 1, 2, 3);
	m_Dec = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_Dec), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 2, 3);
	m_DecUnit = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(m_DecUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecUnit, 2, 3, 2, 3);

	// Location
	m_LocLabel = gtk_label_new("Observatory - name");
	gtk_misc_set_alignment(GTK_MISC(m_LocLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocLabel, 0, 1, 3, 4);
	m_LocName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_LocName, "Observer's location designation, e.g. Brno, Czech Republic");
	gtk_entry_set_max_length(GTK_ENTRY(m_LocName), MAX_LOCATION_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocName, 1, 2, 3, 4);
	m_LocBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_LocBtn, "Load observer's coordinates from presets");
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocBtn, 2, 3, 3, 4);
	m_LonLabel = gtk_label_new("- longitude");
	gtk_misc_set_alignment(GTK_MISC(m_LonLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonLabel, 0, 1, 4, 5);
	m_Lon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lon, 1, 2, 4, 5);
	m_LonUnit = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(m_LonUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonUnit, 2, 3, 4, 5);
	m_LatLabel = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(m_LatLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatLabel, 0, 1, 5, 6);
	m_Lat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lat, 1, 2, 5, 6);
	m_LatUnit = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(m_LatUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatUnit, 2, 3, 5, 6);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CAMassCurveOptionsDlg::~CAMassCurveOptionsDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CAMassCurveOptionsDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAMassCurveOptionsDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAMassCurveOptionsDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_AIR_MASS_CURVE);
		return false;
	}
	return true;
}

bool CAMassCurveOptionsDlg::Execute(CObjectCoords &obj, CLocation &loc)
{
	m_ObjCoords = obj;
	m_Location = loc;

	UpdateObjectCoords();
	UpdateLocation();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT) 
		return false;

	obj = m_ObjCoords;
	loc = m_Location;
	return true;
}

//
// Set parameters
//
void CAMassCurveOptionsDlg::UpdateObjectCoords(void)
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

void CAMassCurveOptionsDlg::UpdateLocation(void)
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

bool CAMassCurveOptionsDlg::OnCloseQuery()
{
	const gchar *ra, *dec, *lon, *lat;
	gchar buf[64], *name;
	gdouble x, y;

	name = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	gtk_entry_set_text(GTK_ENTRY(m_ObjName), g_strstrip(name));
	m_ObjCoords.SetName(name);
	g_free(name);

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
	gtk_entry_set_text(GTK_ENTRY(m_RA), g_strstrip(buf));
	m_ObjCoords.SetRA(buf);
		
	if (cmpack_strtodec(dec, &y)!=0) {
		ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the declination.");
		return false;
	}
	cmpack_dectostr(y, buf, 64);
	gtk_entry_set_text(GTK_ENTRY(m_Dec), g_strstrip(buf));
	m_ObjCoords.SetDec(buf);

	name = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	gtk_entry_set_text(GTK_ENTRY(m_LocName), g_strstrip(name));
	m_Location.SetName(name);
	g_free(name);

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
	gtk_entry_set_text(GTK_ENTRY(m_Lon), g_strstrip(buf));
	m_Location.SetLon(buf);
		
	if (cmpack_strtolat(lat, &y)!=0) {
		ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the latitude.");
		return false;
	}
	cmpack_lattostr(y, buf, 64);
	gtk_entry_set_text(GTK_ENTRY(m_Lat), g_strstrip(buf));
	m_Location.SetLat(buf);

	return true;
}

void CAMassCurveOptionsDlg::button_clicked(GtkWidget *pButton, CAMassCurveOptionsDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CAMassCurveOptionsDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
}

void CAMassCurveOptionsDlg::EditObjectCoords(void)
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
	if (dlg.Execute(&m_ObjCoords)) 
		UpdateObjectCoords();
}

void CAMassCurveOptionsDlg::EditLocation(void)
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
	if (dlg.Execute(&m_Location)) 
		UpdateLocation();
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
	return NULL;
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
CSaveAirMassCurveDlg::CSaveAirMassCurveDlg(GtkWindow *pParent):m_pParent(pParent),
	m_Updating(false), m_FileType(TYPE_MUNIPACK), m_SelectedY(-1)
{
	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));
	m_Options[TYPE_MUNIPACK].header = m_Options[TYPE_MUNIPACK].all_values = true;
	
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Save air mass curve", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("airmasscurve");
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

CSaveAirMassCurveDlg::~CSaveAirMassCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_FileTypes);
	g_object_unref(m_Channels);
}

CSaveAirMassCurveDlg::tFileType CSaveAirMassCurveDlg::StrToFileType(const gchar *str)
{
	if (str) {
		for (gint i=0; FileFormats[i].Id!=NULL; i++) {
			if (strcmp(FileFormats[i].Id, str)==0)
				return (tFileType)i;
		}
	}
	return TYPE_MUNIPACK;
}

const gchar *CSaveAirMassCurveDlg::FileTypeToStr(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].Id;
	return "";
}

bool CSaveAirMassCurveDlg::Execute(const CTable &table, int ychannel)
{
	m_Table.MakeCopy(table);
	m_SelectedY = ychannel;

	gchar *aux = g_Project->GetStr("AirMassCurve", "FileType");
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

	gchar *filename = g_Project->GetStr("AirMassCurve", "File", "airmass.txt");
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
	m_Options[TYPE_CSV].frame_id = g_Project->GetBool("AirMassCurve", "CSV_FRAME_ID");
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("AirMassCurve", "CSV_SKIP_INVALID", true);
	m_Options[TYPE_CSV].header = g_Project->GetBool("AirMassCurve", "CSV_HEADER", true);
	m_Options[TYPE_CSV].all_values = g_Project->GetBool("AirMassCurve", "CSV_ALL_VALUES");
	m_Options[TYPE_TEXT].frame_id = g_Project->GetBool("AirMassCurve", "TEXT_FRAME_ID");
	m_Options[TYPE_TEXT].skip_invalid = g_Project->GetBool("AirMassCurve", "TEXT_SKIP_INVALID", true);
	m_Options[TYPE_TEXT].header = g_Project->GetBool("AirMassCurve", "TEXT_HEADER", true);
	m_Options[TYPE_TEXT].all_values = g_Project->GetBool("AirMassCurve", "TEXT_ALL_VALUES");

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	// Output file path
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	dirpath = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);
	basename = g_path_get_basename(filename);
	g_Project->SetStr("AirMassCurve", "File", basename);
	g_free(basename);

	// Save settings
	g_Project->SetStr("AirMassCurve", "FileType", FileTypeToStr(m_FileType));
	g_Project->SetBool("AirMassCurve", "CSV_FRAME_ID", m_Options[TYPE_CSV].frame_id);
	g_Project->SetBool("AirMassCurve", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("AirMassCurve", "CSV_HEADER", m_Options[TYPE_CSV].header);
	g_Project->SetBool("AirMassCurve", "CSV_ALL_VALUES", m_Options[TYPE_CSV].all_values);
	g_Project->SetBool("AirMassCurve", "TEXT_FRAME_ID", m_Options[TYPE_TEXT].frame_id);
	g_Project->SetBool("AirMassCurve", "TEXT_SKIP_INVALID", m_Options[TYPE_TEXT].skip_invalid);
	g_Project->SetBool("AirMassCurve", "TEXT_HEADER", m_Options[TYPE_TEXT].header);
	g_Project->SetBool("AirMassCurve", "TEXT_ALL_VALUES", m_Options[TYPE_TEXT].all_values);

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
		case CChannel::DATA_AIRMASS:
		case CChannel::DATA_ALTITUDE:
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
	return res==0;
}

void CSaveAirMassCurveDlg::selection_changed(GtkComboBox *pWidget, CSaveAirMassCurveDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CSaveAirMassCurveDlg::OnSelectionChanged(GtkComboBox *pWidget)
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
			if (m_SelectedY!=ch) {
				m_SelectedY = ch;
				UpdateControls();
			}
		}
	}
}

void CSaveAirMassCurveDlg::UpdateControls(void)
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

void CSaveAirMassCurveDlg::OnTypeChanged(void)
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

	// Y columns
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), NULL);
	gtk_list_store_clear(m_Channels);
	if (m_FileType == TYPE_MUNIPACK) {
		// Show [All values] only
		for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
			CChannel *ch = m_Table.ChannelsY()->Get(i);
			if (ch && (ch->Info() == CChannel::DATA_AIRMASS || ch->Info() == CChannel::DATA_ALTITUDE)) {
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
			if (ch && (ch->Info() == CChannel::DATA_AIRMASS || ch->Info() == CChannel::DATA_ALTITUDE)) {
				GtkTreeIter iter;
				gtk_list_store_append(m_Channels, &iter);
				gtk_list_store_set(m_Channels, &iter, 0, i, 1, ch->Name(), -1);
			}
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), GTK_TREE_MODEL(m_Channels));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_VCCombo), m_SelectedY);
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

void CSaveAirMassCurveDlg::button_toggled(GtkToggleButton *pWidget, CSaveAirMassCurveDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CSaveAirMassCurveDlg::OnButtonToggled(GtkToggleButton *pWidget)
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

void CSaveAirMassCurveDlg::response_dialog(GtkWidget *pDlg, gint response_id, CSaveAirMassCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CSaveAirMassCurveDlg::OnResponseDialog(gint response_id)
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
