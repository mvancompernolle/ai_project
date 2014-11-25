/**************************************************************

table_dlg.cpp (C-Munipack project)
The preview dialog for tables
Copyright (C) 2010 David Motl, dmotl@volny.cz

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

#include "table_dlg.h"
#include "configuration.h"
#include "showheader_dlg.h"
#include "export_dlgs.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"

enum tMenuId
{
	MENU_FILE = 1,
	MENU_VIEW,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_OPEN = 100,
	CMD_SAVE,
	CMD_CLOSE,
	CMD_PRINT,
	CMD_EXPORT,
	CMD_PROPERTIES,
	CMD_ERRORS,
	CMD_GRAPH,
	CMD_TABLE,
	CMD_GRID,
	CMD_STATISTICS,
	CMD_MEASUREMENT,
	CMD_SHOW_HELP
};

enum tFieldId
{
	FIELD_FILE,
	FIELD_FILENAME,
	FIELD_DIRPATH,
	FIELD_FILTER,
	FIELD_APERTURE
};

//-------------------------   MENU FOR PREVIEW WINDOW   ---------------------------

static const CMenuBar::tMenuItem FileMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_OPEN,			"_Open" },
	//{ CMenuBar::MB_ITEM,	CMD_SAVE,			"Save _as ..." },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT,			"Export" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_PROPERTIES,		"Show _Properties" },
	//{ CMenuBar::MB_SEPARATOR },
	//{ CMenuBar::MB_ITEM,	CMD_PRINT,			"P_rint" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_CLOSE,			"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN,	CMD_GRAPH,		"_Graph" },
	{ CMenuBar::MB_RADIOBTN,	CMD_TABLE,		"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN,	CMD_ERRORS,		"_Error bars" },
	{ CMenuBar::MB_CHECKBTN,	CMD_GRID,		"_Grid" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN,	CMD_STATISTICS,	"_Statistics" },
	{ CMenuBar::MB_CHECKBTN,	CMD_MEASUREMENT,"_Measurement" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,		CMD_SHOW_HELP,	"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu PreviewMenu[] = {
	{ "_File", MENU_FILE,	FileMenu },
	{ "_View", MENU_VIEW,	ViewMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   PREVIEW DIALOG   --------------------------------

CTableDlg::CTableDlg(void):m_Updating(false), m_UpdatePos(true), m_LastPosValid(false), 
	m_ChannelX(0), m_ChannelY(0), m_LastPosX(0), m_LastPosY(0), m_LastFocus(-1), 
	m_InfoMode(INFO_NONE), m_GraphData(NULL), m_TableData(NULL)
{
	GtkWidget *tbox;
	GtkCellRenderer *renderer;

	m_ShowErrors = g_Project->GetBool("PreviewDlg", "Errors", true);
	m_ShowGrid   = g_Project->GetBool("PreviewDlg", "Grid", false);
	m_DispMode   = (tDisplayMode)g_Project->GetInt("PreviewDlg", "Display", DISPLAY_GRAPH, 0, DISPLAY_TABLE);

	// Menu bar
	m_Menu.Create(PreviewMenu, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), false, false, 0);

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(m_MainBox), tbox, false, false, 0);

	m_XLabel = toolbar_new_label(tbox, "X axis");
	m_XChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_XCombo = toolbar_new_combo(tbox, "Column shown on the horizontal axis of the graph");
	g_signal_connect(G_OBJECT(m_XCombo), "changed", G_CALLBACK(combo_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_XCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_XCombo), renderer, "text", 1);

	toolbar_new_separator(tbox);

	m_YLabel = toolbar_new_label(tbox, "Y axis");
	m_YChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_YCombo = toolbar_new_combo(tbox, "Column shown on the vertical axis of the graph");
	g_signal_connect(G_OBJECT(m_YCombo), "changed", G_CALLBACK(combo_changed), this);
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
	g_signal_connect(G_OBJECT(m_GraphView), "selection-changed", G_CALLBACK(selection_changed), this);
	gtk_widget_set_size_request(m_GraphView, 640, 200);
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

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	SetDisplayMode(m_DispMode);
	SetInfoMode(INFO_NONE);
}

CTableDlg::~CTableDlg(void)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_GraphView), (gpointer)mouse_moved, this);
	g_source_remove(m_TimerId);
	g_object_unref(m_XChannels);
	g_object_unref(m_YChannels);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	if (m_TableData)
		g_object_unref(m_TableData);
}

bool CTableDlg::LoadFile(const char *filename, GError **error)
{
	m_MeasBox.SetTable(NULL);
	SetStatus(NULL);
	SetInfoMode(INFO_NONE);

	if (!m_File.Load(filename, error)) 
		return false;

	m_MeasBox.SetTable(&m_File);
	UpdateChannels();
	UpdateGraph(TRUE, TRUE);
	UpdateTools();
	UpdateControls();
	return true;
}

void CTableDlg::UpdateControls(void)
{
	// Menu File
	m_Menu.Enable(CMD_SAVE, m_File.Valid());
	m_Menu.Enable(CMD_EXPORT, m_File.Valid());
	m_Menu.Enable(CMD_PROPERTIES, m_File.Valid());

	// Menu View
	m_Menu.Enable(CMD_GRAPH, m_File.Valid());
	m_Menu.Check(CMD_GRAPH, m_DispMode==DISPLAY_GRAPH);
	m_Menu.Enable(CMD_TABLE, m_File.Valid());
	m_Menu.Check(CMD_TABLE, m_DispMode==DISPLAY_TABLE);
	m_Menu.Enable(CMD_ERRORS, m_File.Valid() && m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_ERRORS, m_ShowErrors);
	m_Menu.Enable(CMD_GRID, m_File.Valid() && m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_GRID, m_ShowGrid);

	// Menu Tools
	m_Menu.Enable(CMD_STATISTICS, m_File.Valid() && m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_STATISTICS, m_InfoMode == INFO_STATISTICS);
	m_Menu.Enable(CMD_MEASUREMENT, m_File.Valid() && m_DispMode==DISPLAY_GRAPH);
	m_Menu.Check(CMD_MEASUREMENT, m_InfoMode == INFO_MEASUREMENT);

	// Channels
	bool ok = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_XChannels), NULL)>1;
	gtk_widget_set_sensitive(m_XLabel, m_DispMode==DISPLAY_GRAPH && ok);
	gtk_widget_set_sensitive(m_XCombo, m_DispMode==DISPLAY_GRAPH && ok);
	ok = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_YChannels), NULL)>1;
	gtk_widget_set_sensitive(m_YLabel, m_DispMode==DISPLAY_GRAPH && ok);
	gtk_widget_set_sensitive(m_YCombo, m_DispMode==DISPLAY_GRAPH && ok);

	// Zoom
	gtk_widget_set_sensitive(m_ZoomLabel, m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), m_DispMode==DISPLAY_GRAPH);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), m_DispMode==DISPLAY_GRAPH);
}

void CTableDlg::UpdateGraph(gboolean autozoom_x, gboolean autozoom_y)
{
	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), NULL);
	if (m_GraphData) {
		g_object_unref(m_GraphData);
		m_GraphData = NULL;
	}
	if (m_ChannelX>=0 && m_ChannelY>=0) {
		m_GraphData = m_File.ToGraphData(m_ChannelX, m_ChannelY);
		m_File.SetView(CMPACK_GRAPH_VIEW(m_GraphView), m_ChannelX, m_ChannelY, !m_ShowErrors);
		cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), m_ShowGrid, m_ShowGrid);
		cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), m_GraphData);
		cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_GraphView), autozoom_x, autozoom_y);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), NULL);
	if (m_TableData) {
		g_object_unref(m_TableData);
		m_TableData = NULL;
	}
	if (m_ChannelX>=0 && m_ChannelY>=0) {
		m_TableData = m_File.ToTreeModel();
		m_File.SetView(GTK_TREE_VIEW(m_TableView));
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_TableView), m_TableData);
	}
}

void CTableDlg::UpdateChannels(void)
{
	// Update list of independent channels
	CChannels *cx = m_File.ChannelsX();
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_XCombo), NULL);
	gtk_list_store_clear(m_XChannels);
	for (int i=0; i<cx->Count(); i++) {
		if (cx->GetInfo(i) == CChannel::DATA_JD || 
			(m_File.Type() == CMPACK_TABLE_MAGDEV && cx->GetInfo(i) == CChannel::DATA_MAGNITUDE) || 
			(m_File.Type() == CMPACK_TABLE_APERTURES && cx->GetInfo(i) == CChannel::DATA_APERTURE_ID)) {
				GtkTreeIter iter;
				gtk_list_store_append(m_XChannels, &iter);
				gtk_list_store_set(m_XChannels, &iter, 0, i, 1, cx->GetName(i), -1);
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
	CChannels *cy = m_File.ChannelsY();
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_YCombo), NULL);
	gtk_list_store_clear(m_YChannels);
	for (int i=0; i<cy->Count(); i++) {
		if (cy->GetInfo(i) == CChannel::DATA_MAGNITUDE || cy->GetInfo(i) == CChannel::DATA_AIRMASS || 
			cy->GetInfo(i) == CChannel::DATA_HELCOR || cy->GetInfo(i) == CChannel::DATA_ALTITUDE ||
			cy->GetInfo(i) == CChannel::DATA_OFFSET || cy->GetInfo(i) == CChannel::DATA_DEVIATION) {
				GtkTreeIter iter;
				gtk_list_store_append(m_YChannels, &iter);
				gtk_list_store_set(m_YChannels, &iter, 0, i, 1, cy->GetName(i), -1);
		}
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

void CTableDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CTableDlg *pMe = (CTableDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CTableDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_OPEN:
		Open();
		break;
	case CMD_SAVE:
		SaveAs("C-Munipack table files", "*.txt");
		break;
	case CMD_CLOSE:
		Close();
		break;
	case CMD_EXPORT:
		Export();
		break;
	case CMD_PROPERTIES:
		ShowProperties();
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
		m_File.SetView(CMPACK_GRAPH_VIEW(m_GraphView), m_ChannelX, m_ChannelY, !m_ShowErrors);
		g_Project->SetBool("PreviewDlg", "Errors", m_ShowErrors);
		break;
	case CMD_GRID:
		m_ShowGrid = m_Menu.IsChecked(CMD_GRID);
		cmpack_graph_view_set_grid(CMPACK_GRAPH_VIEW(m_GraphView), m_ShowGrid, m_ShowGrid);
		g_Project->SetBool("PreviewDlg", "Grid", m_ShowGrid);
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
	case CMD_SHOW_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_GRAPH_WINDOW);
		break;
	}
}

void CTableDlg::ShowProperties(void)
{
	CTableInfoDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.ShowModal(&m_File, m_Name, m_Path);
}

void CTableDlg::combo_changed(GtkComboBox *button, CTableDlg *pDlg)
{
	pDlg->OnComboChanged(button);
}

void CTableDlg::OnComboChanged(GtkComboBox *pWidget)
{
	if (pWidget== GTK_COMBO_BOX(m_XCombo)) {
		int ch = SelectedItem(pWidget);
		if (ch>=0 && ch!=m_ChannelX) {
			m_ChannelX = ch;
			UpdateGraph(TRUE, FALSE);
			UpdateTools();
		}
	} else if (pWidget== GTK_COMBO_BOX(m_YCombo)) {
		int ch = SelectedItem(pWidget);
		if (ch>=0 && ch!=m_ChannelY) {
			m_ChannelY = ch;
			UpdateGraph(FALSE, TRUE);
			UpdateTools();
		}
	}
}

void CTableDlg::button_clicked(GtkWidget *button, CTableDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CTableDlg::OnButtonClicked(GtkWidget *pBtn)
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

void CTableDlg::mouse_moved(GtkWidget *button, CTableDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

gboolean CTableDlg::timer_cb(CTableDlg *pDlg)
{
	if (pDlg->m_UpdatePos) {
		pDlg->m_UpdatePos = false;
		pDlg->UpdateStatus();
	}
	return TRUE;
}

void CTableDlg::selection_changed(GtkWidget *pChart, CTableDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

void CTableDlg::OnSelectionChanged(void)
{
	UpdateTools();
	UpdateStatus();
}

void CTableDlg::UpdateStatus(void)
{
	char	msg[256], buf1[256], buf2[256];
	gdouble	dx, dy, sdev;

	if (m_DispMode == DISPLAY_GRAPH) {
		int item = cmpack_graph_view_get_focused(CMPACK_GRAPH_VIEW(m_GraphView));
		if (item>=0) {
			if (m_LastFocus!=item && m_GraphData) {
				m_LastFocus = item;
				int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, item),
					xcol = m_File.ChannelsX()->GetColumn(m_ChannelX),
					ycol = m_File.ChannelsY()->GetColumn(m_ChannelY),
					ucol = m_File.ChannelsY()->GetColumnU(m_ChannelY);
				if (m_File.Find(frame_id) && m_File.GetDbl(xcol, &dx) && m_File.GetDbl(ycol, &dy)) {
					PrintKeyValue(buf1, dx, m_File.ChannelsX()->Get(m_ChannelX));
					if (ucol>=0 && m_File.GetDbl(ucol, &sdev))
						PrintKeyValueU(buf2, dy, sdev, m_File.ChannelsY()->Get(m_ChannelY));
					else 
						PrintKeyValue(buf2, dy, m_File.ChannelsY()->Get(m_ChannelY));
					sprintf(msg, "Point #%d: %s, %s", frame_id, buf1, buf2);
				} else {
					sprintf(msg, "Point #%d", frame_id);
				}
				SetStatus(msg);
			}
		} else {
			m_LastFocus = -1;
			if (m_ChannelX>=0 && m_ChannelX<m_File.ChannelsX()->Count() && 
				m_ChannelY>=0 && m_ChannelY<m_File.ChannelsY()->Count() &&
				cmpack_graph_view_mouse_pos(CMPACK_GRAPH_VIEW(m_GraphView), &dx, &dy)) {
					if (!m_LastPosValid || dx!=m_LastPosX || dy!=m_LastPosY) {
						m_LastPosValid = true;
						m_LastPosX = dx;
						m_LastPosY = dy;
						PrintKeyValue(buf1, dx, m_File.ChannelsX()->Get(m_ChannelX));
						PrintKeyValue(buf2, dy, m_File.ChannelsY()->Get(m_ChannelY));
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
		} else if (count==1) 
			SetStatus("1 row selected");
		else
			SetStatus(NULL);
	}
}

void CTableDlg::SetDisplayMode(tDisplayMode mode)
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
	g_Project->SetInt("PreviewDlg", "Display", m_DispMode);
	UpdateControls();
}

void CTableDlg::PrintValue(char *buf, double value, const CChannel *channel)
{
	const gchar *unit = channel->Unit();
	int prec = channel->Precision();

	if (unit)
		sprintf(buf, "%.*f %s", prec, value, unit);
	else
		sprintf(buf, "%.*f", prec, value);
}

void CTableDlg::PrintKeyValue(char *buf, double value, const CChannel *channel)
{
	const gchar *name = channel->Name(), *unit = channel->Unit();
	int prec = channel->Precision();

	if (unit)
		sprintf(buf, "%s = %.*f %s", name, prec, value, unit);
	else
		sprintf(buf, "%s = %.*f", name, prec, value);
}

void CTableDlg::PrintKeyValueU(char *buf, double value, double sdev, const CChannel *channel)
{
	const gchar *name = channel->Name(), *unit = channel->Unit();
	int prec = channel->Precision();

	if (unit)
		sprintf(buf, "%s = %.*f +- %.*f %s", name, prec, value, prec, sdev, unit);
	else
		sprintf(buf, "%s = %.*f +- %.*f", name, prec, value, prec, sdev);
}

void CTableDlg::UpdateTools(void)
{
	char	buf[256], msg[256];
	double	dx, dy, dmin, dmax, mean, sdev;
	int		xcol, ycol, ucol;

	if (m_InfoMode == INFO_MEASUREMENT) {
		// Update measurement tool
		m_MeasBox.SetChannel(CMPACK_AXIS_X, m_ChannelX);
		m_MeasBox.SetChannel(CMPACK_AXIS_Y, m_ChannelY);
	} else 
	if (m_InfoMode == INFO_STATISTICS && m_GraphData) {
		m_InfoBox.BeginUpdate();
		m_InfoBox.Clear();
		// Show information about selected star
		xcol = m_File.ChannelsX()->GetColumn(m_ChannelX);
		ycol = m_File.ChannelsY()->GetColumn(m_ChannelY);
		ucol = m_File.ChannelsY()->GetColumnU(m_ChannelY);
		int selected_count = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_GraphView));
		if (selected_count==1) {
			int row = cmpack_graph_view_get_selected(CMPACK_GRAPH_VIEW(m_GraphView));
			int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, row);
			if (frame_id>=0) {
				sprintf(buf, "Point #%d", frame_id+1);
				m_InfoBox.AddTitle(1, buf);
				if (m_File.Find(frame_id) && m_File.GetDbl(xcol, &dx) && m_File.GetDbl(ycol, &dy)) {
					PrintKeyValue(buf, dx, m_File.ChannelsX()->Get(m_ChannelX));
					m_InfoBox.AddText(buf);
					if (ucol>=0 && m_File.GetDbl(ucol, &sdev)) 
						PrintKeyValueU(buf, dy, sdev, m_File.ChannelsY()->Get(m_ChannelY));
					else
						PrintKeyValue(buf, dy, m_File.ChannelsY()->Get(m_ChannelY));
					m_InfoBox.AddText(buf);
				}
			}
		} else {
			// Compute statistics
			int count = 0, length = (selected_count>0 ? selected_count : m_File.Rows());
			double *x = new double[length], *y = new double[length];
			if (selected_count>0) {
				GList *list = cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_GraphView));
				for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) {
					int frame_id = (int)cmpack_graph_data_get_param(m_GraphData, (gint)(intptr_t)ptr->data);
					if (m_File.Find(frame_id) && m_File.GetDbl(xcol, x+count) && m_File.GetDbl(ycol, y+count))
						count++;
				}
				g_list_free(list);
			} else {
				bool ok = m_File.Rewind();
				while (ok) {
					if (m_File.GetDbl(xcol, x+count) && m_File.GetDbl(ycol, y+count))
						count++;
					ok = m_File.Next();
				}
			}
			if (selected_count>0)
				sprintf(buf, "There are %d selected points.", count);
			else
				sprintf(buf, "There are %d points in total.", count);
			m_InfoBox.AddText(buf);
			if (ComputeMinMax(count, x, &dmin, &dmax)) {
				m_InfoBox.AddText("");
				sprintf(msg, "Data %s:", m_File.ChannelsX()->GetName(m_ChannelX));
				m_InfoBox.AddTitle(1, msg);
				PrintValue(buf, dmin, m_File.ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_File.ChannelsX()->Get(m_ChannelX));
				sprintf(msg, "%s: %s", "Max.", buf);
				m_InfoBox.AddText(msg);
			}
			if (ComputeMinMax(count, y, &dmin, &dmax)) {
				cmpack_robustmean(count, y, &mean, &sdev);
				m_InfoBox.AddText("");
				sprintf(buf, "Data %s:", m_File.ChannelsY()->GetName(m_ChannelY));
				m_InfoBox.AddTitle(1, buf);
				PrintValue(buf, dmin, m_File.ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Min.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, dmax, m_File.ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Max.", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, mean, m_File.ChannelsY()->Get(m_ChannelY));
				sprintf(msg, "%s: %s", "Mean", buf);
				m_InfoBox.AddText(msg);
				PrintValue(buf, sdev, m_File.ChannelsY()->Get(m_ChannelY));
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

void CTableDlg::SetInfoMode(tInfoMode mode)
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

void CTableDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CTableDlg *pMe = (CTableDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CTableDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateControls();
}

void CTableDlg::Export()
{
	gchar *basename = g_path_get_basename(m_Path);
	if (m_DispMode == DISPLAY_GRAPH) {
		CGraphExportDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Execute(CMPACK_GRAPH_VIEW(m_GraphView), basename);
	} else {
		CExportTableFileDlg pDlg(GTK_WINDOW(m_pDlg), m_File.Type());
		if (pDlg.Execute(m_File, basename, m_ChannelX, m_ChannelY)) {
			m_NotSaved = false;
			UpdateControls();
		}
	}
	g_free(basename);
}

//---------------------------   INFO DIALOG   --------------------------------

//
// Constructor
//
CTableInfoDlg::CTableInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0),
	m_Name(NULL)
{
	GtkWidget *bbox;

	AddHeading(FIELD_FILE, 0, 0, "File");
	AddField(FIELD_FILENAME, 0, 1, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_DIRPATH, 0, 2, "Location", PANGO_ELLIPSIZE_MIDDLE);
	AddSeparator(0, 4);
	AddField(FIELD_FILTER, 0, 5, "Optical filter", PANGO_ELLIPSIZE_END);
	AddField(FIELD_APERTURE, 0, 6, "Aperture");
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
void CTableInfoDlg::ShowModal(const CTable *tab, const gchar *name, const gchar *path)
{
	char buf[512];

	m_File = tab;
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

	if (tab->Filter())
		SetField(FIELD_FILTER, tab->Filter());
	else
		SetField(FIELD_FILTER, "");
	if (tab->Aperture()>0)
		SetField(FIELD_APERTURE, tab->Aperture());
	else
		SetField(FIELD_APERTURE, "");

	CInfoDlg::ShowModal();
}

void CTableInfoDlg::button_clicked(GtkWidget *button, CTableInfoDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CTableInfoDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_HdrBtn))
		ShowHeader();
}


void CTableInfoDlg::ShowHeader(void)
{
	CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(m_File, m_Name);
}

//-------------------------   SAVE LIGHT CURVE   --------------------------------

//-------------------------   FILE FORMATS   ------------------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
	bool DiffMag, InstMag, Track, AirMass, MagDev;
} FileFormats[] = {
	{ "AVE",	"AVE compatible",					"djm", "text/plain",	"AVE files",		"*.djm",	
		true,	false,	false,	false,	false },
	{ "MCV",	"MCV compatible",					"dat", "text/plain",	"MCV files",		"*.mcv",	
		false,	true,	false,	false,	false },
	{ "TEXT",	"Text (space separated values)",	"txt", "text/plain",	"Text files",		"*.txt",	
		true,	true,	true,	true,	true },
	{ "CSV",	"CSV (comma separated values)",		"csv", "text/csv",		"CSV files",		"*.csv",	
		true,	true,	true,	true,	true },
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

//
// Constructor
//
CExportTableFileDlg::CExportTableFileDlg(GtkWindow *parent, CmpackTableType type):m_pParent(parent),
	m_JDCombo(NULL), m_VCCombo(NULL), m_Errors(NULL), m_SkipInvalid(NULL), m_ZeroInvalid(NULL), 
	m_AllValues(NULL), m_HelCor(NULL), m_AirMass(NULL), m_Altitude(NULL), m_JDTypes(NULL), 
	m_Channels(NULL), m_Type(type), m_Updating(false), m_FileType(TYPE_CSV), 
	m_JDType(DATE_JD_GEOCENTRIC), m_SelectedY(-1), m_HaveHelCor(false), m_HaveAirMass(false), 
	m_HaveAltitude(false)
{
	GtkWidget *frame, *hbox, *label;
	GtkCellRenderer *renderer;

	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));
	m_Options[TYPE_AVE].skip_invalid = m_Options[TYPE_MCV].zero_invalid = true;
	
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export table", m_pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
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
	m_SkipInvalid = gtk_check_button_new_with_label("Discard rows with invalid values");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipInvalid), "toggled", G_CALLBACK(button_toggled), this);
	if (m_Type==CMPACK_TABLE_LCURVE_INST) {
		m_ZeroInvalid = gtk_check_button_new_with_label("Replace invalid values with zeros");
		gtk_box_pack_start(GTK_BOX(lbox), m_ZeroInvalid, FALSE, TRUE, 0);
		g_signal_connect(G_OBJECT(m_ZeroInvalid), "toggled", G_CALLBACK(button_toggled), this);
	}
	
	// JD/UTC selection
	if (m_Type==CMPACK_TABLE_LCURVE_INST || m_Type==CMPACK_TABLE_LCURVE_DIFF) {
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
	}

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
	if (m_Type==CMPACK_TABLE_LCURVE_INST || m_Type==CMPACK_TABLE_LCURVE_DIFF) {
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
	} else {
		m_AllValues = gtk_check_button_new_with_label("Export all values");
		gtk_box_pack_start(GTK_BOX(rbox), m_AllValues, FALSE, TRUE, 0);
		g_signal_connect(G_OBJECT(m_AllValues), "toggled", G_CALLBACK(button_toggled), this);
	}
		
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CExportTableFileDlg::~CExportTableFileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
	if (m_JDTypes)
		g_object_unref(m_JDTypes);
	if (m_Channels)
		g_object_unref(m_Channels);
}

CExportTableFileDlg::tFileType CExportTableFileDlg::StrToFileType(const gchar *str)
{
	if (str) {
		for (gint i=0; FileFormats[i].Id!=NULL; i++) {
			if (strcmp(FileFormats[i].Id, str)==0)
				return (tFileType)i;
		}
	}
	return TYPE_CSV;
}

const gchar *CExportTableFileDlg::FileTypeToStr(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].Id;
	return "";
}

bool CExportTableFileDlg::Execute(const CTable &table, const gchar *current_path, int xchannel, int ychannel)
{
	m_Table.MakeCopy(table);
	m_SelectedY = ychannel;

	m_JDType = (m_Table.ChannelsX()->GetInfo(xchannel)==CChannel::DATA_JD_HEL ? DATE_JD_HELIOCENTRIC : DATE_JD_GEOCENTRIC);
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

	CmpackTableType type = m_Table.Type();
	gchar key[256];
	sprintf(key, "%s_FileType", TableTypeToStr(type));
	gchar *aux = g_Project->GetStr("TableDlg", key);
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

	gchar *filename = g_path_get_basename(current_path);
	if (filename) {
		gchar *newname = SetFileExtension(filename, FileExtension(m_FileType));
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
		bool ok;
		switch (type)
		{
		case CMPACK_TABLE_LCURVE_DIFF:
			ok = FileFormats[i].DiffMag;
			break;
		case CMPACK_TABLE_LCURVE_INST:
			ok = FileFormats[i].InstMag;
			break;
		case CMPACK_TABLE_MAGDEV:
			ok = FileFormats[i].MagDev;
			break;
		case CMPACK_TABLE_TRACKLIST:
			ok = FileFormats[i].Track;
			break;
		case CMPACK_TABLE_AIRMASS:
			ok = FileFormats[i].AirMass;
			break;
		default:
			ok = false;
		}
		if (ok) {
			GtkTreeIter iter;
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
	if (m_JDCombo && m_JDTypes) {
		gtk_combo_box_set_model(GTK_COMBO_BOX(m_JDCombo), NULL);
		gtk_list_store_clear(m_JDTypes);
		for (int i=0; DateFormats[i].Caption; i++) {
			if (i==DATE_JD_GEOCENTRIC || m_Table.ChannelsX()->FindFirst("JDHEL")>=0) {
				GtkTreeIter iter;
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
	}

	// Y columns
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_VCCombo), NULL);
	gtk_list_store_clear(m_Channels);
	for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
		CChannel *ch = m_Table.ChannelsY()->Get(i);
		if (ch) {
			if (m_Type == CMPACK_TABLE_LCURVE_INST || m_Type == CMPACK_TABLE_LCURVE_DIFF) {
				if (ch && ch->Info() == CChannel::DATA_MAGNITUDE) {
					GtkTreeIter iter;
					gtk_list_store_append(m_Channels, &iter);
					gtk_list_store_set(m_Channels, &iter, 0, i, 1, ch->Name(), -1);
				}
			} else {
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
			m_SelectedY = SelectedItem(GTK_COMBO_BOX(m_VCCombo));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_VCCombo), -1);
		m_SelectedY = -1;
	}
	
	// Restore options (format dependent customizable options)
	m_Options[TYPE_CSV].helcor = m_HaveHelCor && g_Project->GetBool("TableDlg", "CSV_HELCOR");
	m_Options[TYPE_CSV].airmass = m_HaveAirMass && g_Project->GetBool("TableDlg", "CSV_AIRMASS");
	m_Options[TYPE_CSV].altitude = m_HaveAltitude && g_Project->GetBool("TableDlg", "CSV_ALTITUDE");
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("TableDlg", "CSV_SKIP_INVALID", true);
	m_Options[TYPE_CSV].zero_invalid = g_Project->GetBool("TableDlg", "CSV_ZERO_INVALID");
	m_Options[TYPE_CSV].errors = g_Project->GetBool("TableDlg", "CSV_ERRORS", true);
	m_Options[TYPE_CSV].header = g_Project->GetBool("TableDlg", "CSV_HEADER", true);
	m_Options[TYPE_CSV].all_values = g_Project->GetBool("TableDlg", "CSV_ALL_VALUES");
	m_Options[TYPE_TEXT].helcor = m_HaveHelCor && g_Project->GetBool("TableDlg", "TEXT_HELCOR");
	m_Options[TYPE_TEXT].airmass = m_HaveAirMass && g_Project->GetBool("TableDlg", "TEXT_AIRMASS");
	m_Options[TYPE_TEXT].altitude = m_HaveAltitude && g_Project->GetBool("TableDlg", "TEXT_ALTITUDE");
	m_Options[TYPE_TEXT].skip_invalid = g_Project->GetBool("TableDlg", "TEXT_SKIP_INVALID", true);
	m_Options[TYPE_TEXT].zero_invalid = g_Project->GetBool("TableDlg", "TEXT_ZERO_INVALID");
	m_Options[TYPE_TEXT].errors = g_Project->GetBool("TableDlg", "TEXT_ERRORS", true);
	m_Options[TYPE_TEXT].header = g_Project->GetBool("TableDlg", "TEXT_HEADER", true);
	m_Options[TYPE_TEXT].all_values = g_Project->GetBool("TableDlg", "TEXT_ALL_VALUES");

	m_Updating = false;

	OnTypeChanged();
	OnSelectionChanged(GTK_COMBO_BOX(m_VCCombo));
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	dirpath = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);
	basename = g_path_get_basename(filename);
	g_Project->SetStr("TableDlg", "File", basename);
	g_free(basename);

	// Save settings
	sprintf(key, "%s_FileType", TableTypeToStr(type));
	g_Project->SetStr("TableDlg", key, FileTypeToStr(m_FileType));
	if (m_HaveHelCor) {
		g_Project->SetBool("TableDlg", "CSV_HELCOR", m_Options[TYPE_CSV].helcor);
		g_Project->SetBool("TableDlg", "TEXT_HELCOR", m_Options[TYPE_TEXT].helcor);
	}
	if (m_HaveAirMass) {
		g_Project->SetBool("TableDlg", "CSV_AIRMASS", m_Options[TYPE_CSV].airmass);
		g_Project->SetBool("TableDlg", "TEXT_AIRMASS", m_Options[TYPE_TEXT].airmass);
	}
	if (m_HaveAltitude) {
		g_Project->SetBool("TableDlg", "CSV_ALTITUDE", m_Options[TYPE_CSV].altitude);
		g_Project->SetBool("TableDlg", "TEXT_ALTITUDE", m_Options[TYPE_TEXT].altitude);
	}
	g_Project->SetBool("TableDlg", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("TableDlg", "CSV_ZERO_INVALID", m_Options[TYPE_CSV].zero_invalid);
	g_Project->SetBool("TableDlg", "CSV_ERRORS", m_Options[TYPE_CSV].errors);
	g_Project->SetBool("TableDlg", "CSV_HEADER", m_Options[TYPE_CSV].header);
	g_Project->SetBool("TableDlg", "CSV_ALL_VALUES", m_Options[TYPE_CSV].all_values);
	g_Project->SetBool("TableDlg", "TEXT_SKIP_INVALID", m_Options[TYPE_TEXT].skip_invalid);
	g_Project->SetBool("TableDlg", "TEXT_ZERO_INVALID", m_Options[TYPE_TEXT].zero_invalid);
	g_Project->SetBool("TableDlg", "TEXT_ERRORS", m_Options[TYPE_TEXT].errors);
	g_Project->SetBool("TableDlg", "TEXT_HEADER", m_Options[TYPE_TEXT].header);
	g_Project->SetBool("TableDlg", "TEXT_ALL_VALUES", m_Options[TYPE_TEXT].all_values);

	const tOptions *opt = m_Options+m_FileType;

	CChannels *cx = m_Table.ChannelsX();
	for (int i=0; i<cx->Count(); i++) 
		cx->Get(i)->SetExportFlags(0);

	CChannels *cy = m_Table.ChannelsY();
	if (m_Type == CMPACK_TABLE_LCURVE_INST || m_Type == CMPACK_TABLE_LCURVE_DIFF) {
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
	} else {
		for (int i=0; i<cy->Count(); i++) {
			CChannel *ch = m_Table.ChannelsY()->Get(i);
			if (opt->all_values || i==m_SelectedY) {
				if (opt->errors) 
					ch->SetExportFlags(0);
				else
					ch->SetExportFlags(CChannel::EXPORT_VALUE_ONLY);
			} else 
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
		}			
	}

	int res = 0, flags = 0;
	if (!opt->header)
		flags |= CTable::EXPORT_NO_HEADER;
	if (opt->skip_invalid)
		flags |= CTable::EXPORT_NULVAL_SKIP_ROW;
	if (opt->zero_invalid)
		flags |= CTable::EXPORT_NULVAL_ZERO;
	GError *error = NULL;
	if (!m_Table.ExportTable(filename, FileMimeType(m_FileType), flags, &error)) {
		if (error) {
			ShowError(m_pParent, error->message);
			g_error_free(error);
		}
		res = -1;
	}
	g_free(filename);
	return res==0;
}

void CExportTableFileDlg::selection_changed(GtkComboBox *pWidget, CExportTableFileDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportTableFileDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft>=0 && ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
			}
		} else
		if (GTK_WIDGET(pWidget) == m_JDCombo) {
			int dt = SelectedItem(pWidget);
			if (dt>=0 && dt!=m_JDType) {
				m_JDType = (tDateType)dt;
				UpdateControls();
			}
		} else
		if (GTK_WIDGET(pWidget) == m_VCCombo) {
			int ch = SelectedItem(pWidget);
			if (ch>=0 && ch!=m_SelectedY) {
				m_SelectedY = ch;
				UpdateControls();
			}
		}
	}
}

void CExportTableFileDlg::UpdateControls(void)
{
	if (m_VCCombo) {
		int count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL);
		gtk_widget_set_sensitive(m_VCCombo, 
			count>1 && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllValues)));
		gtk_widget_set_sensitive(m_AllValues, 
			(m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT) && count>1);
	}
	if (m_JDCombo) {
		gtk_widget_set_sensitive(m_JDCombo, 
			gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_JDTypes), NULL)>1);
	}
	if (m_Errors) 
		gtk_widget_set_sensitive(m_Errors, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_Header, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	gtk_widget_set_sensitive(m_SkipInvalid, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	if (m_ZeroInvalid) {
		gtk_widget_set_sensitive(m_ZeroInvalid, (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT) && 
			!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SkipInvalid)));
	}
	if (m_Altitude)
		gtk_widget_set_sensitive(m_Altitude, m_HaveAltitude && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	if (m_AirMass)
		gtk_widget_set_sensitive(m_AirMass, m_HaveAirMass && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
	if (m_HelCor)
		gtk_widget_set_sensitive(m_HelCor, m_HaveHelCor && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT));
}

void CExportTableFileDlg::OnTypeChanged(void)
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

	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllValues), opt->all_values);
	if (m_Errors)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Errors), opt->errors);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);
	if (m_ZeroInvalid)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ZeroInvalid), opt->zero_invalid);
	if (m_HelCor)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_HelCor), m_HaveHelCor && opt->helcor);
	if (m_AirMass)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AirMass), m_HaveAirMass && opt->airmass);
	if (m_Altitude)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Altitude), m_HaveAltitude && opt->altitude);

	m_Updating = false;

	UpdateControls();
}

void CExportTableFileDlg::button_toggled(GtkToggleButton *pWidget, CExportTableFileDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CExportTableFileDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_HelCor) {
		if (m_HaveHelCor && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT))
			m_Options[m_FileType].helcor = gtk_toggle_button_get_active(pWidget)!=0;
	} else 
	if (GTK_WIDGET(pWidget) == m_AirMass) {
		if (m_HaveAirMass && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT))
			m_Options[m_FileType].airmass = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Altitude) {
		if (m_HaveAltitude && (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT))
			m_Options[m_FileType].altitude = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_ZeroInvalid) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
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
	if (GTK_WIDGET(pWidget) == m_AllValues) {
		if (m_FileType==TYPE_CSV || m_FileType==TYPE_TEXT)
			m_Options[m_FileType].all_values = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CExportTableFileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CExportTableFileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExportTableFileDlg::OnResponseDialog(gint response_id)
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
