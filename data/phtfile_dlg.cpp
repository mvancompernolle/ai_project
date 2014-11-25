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

#include "phtfile_dlg.h"
#include "configuration.h"
#include "configuration.h"
#include "showheader_dlg.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"
#include "export_dlgs.h"

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
	CMD_SAVE_AS,
	CMD_CLOSE,
	CMD_PRINT,
	CMD_EXPORT,
	CMD_PROPERTIES,
	CMD_CHART,
	CMD_TABLE,
	CMD_RULERS,
	CMD_INSPECTOR,
	CMD_SHOW_HELP
};

enum tFieldId
{
	FIELD_FILE,
	FIELD_FILENAME,
	FIELD_DIRPATH,
	FIELD_FRAME,
	FIELD_DATETIME,
	FIELD_JULDAT,
	FIELD_FILTER,
	FIELD_EXPTIME,
	FIELD_PHOTOMETRY,
	FIELD_STARS
};

//-------------------------   MENU FOR PREVIEW WINDOW   ---------------------------

static const CMenuBar::tMenuItem FileMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_OPEN,		"_Open" },
	//{ CMenuBar::MB_ITEM,	CMD_SAVE_AS,	"_Save as ..." },
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
	{ CMenuBar::MB_RADIOBTN, CMD_CHART,			"_Chart" },
	{ CMenuBar::MB_RADIOBTN, CMD_TABLE,			"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN, CMD_RULERS,		"_Rulers" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN, CMD_INSPECTOR,		"_Object inspector" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_SHOW_HELP,		"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu PreviewMenu[] = {
	{ "_File",	MENU_FILE,	FileMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   PREVIEW DIALOG   --------------------------------

CPhtFileDlg::CPhtFileDlg(void):m_File(NULL), m_ChartData(NULL), m_TableData(NULL), m_Updating(false), 
	m_UpdatePos(true), m_UpdateZoom(true), m_ApertureIndex(-1), m_SelectedObjId(-1), 
	m_SelectedRow(-1), m_LastPosX(-1), m_LastPosY(-1), m_LastFocus(-1),
	m_InfoMode(INFO_NONE), m_SortColumnId(-1), m_SortType(GTK_SORT_ASCENDING), 
	m_SortCol(NULL), m_SelectedPath(NULL)
{
	GtkWidget *tbox, *hbox;

	m_DispMode = (tDispMode)g_Project->GetInt("PhtFileDlg", "Mode", DISP_CHART, 0, DISP_MODE_COUNT-1);
	m_Rulers = g_Project->GetBool("PreviewDlg", "Rulers");
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Menu bar
	m_Menu.Create(PreviewMenu, false);
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

	toolbar_new_separator(tbox);

	m_Apertures = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	m_AperLabel = toolbar_new_label(tbox, "Aperture");
	m_AperCombo = toolbar_new_combo(tbox, "Aperture used to get brightness of the objects");
	g_signal_connect(G_OBJECT(m_AperCombo), "changed", G_CALLBACK(button_clicked), this);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_AperCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_AperCombo), renderer, "text", 1);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_MainBox), hbox, TRUE, TRUE, 0);

	// Chart
	m_ChartView = cmpack_chart_view_new();
	cmpack_chart_view_set_mouse_control(CMPACK_CHART_VIEW(m_ChartView), TRUE);
	cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_SINGLE);
	cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DBLCLICK);
	cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_ChartView), m_Rulers);
	cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_ChartView), TRUE);
	g_signal_connect(G_OBJECT(m_ChartView), "zoom-changed", G_CALLBACK(zoom_changed), this);
	g_signal_connect(G_OBJECT(m_ChartView), "mouse-moved", G_CALLBACK(mouse_moved), this);
	g_signal_connect(G_OBJECT(m_ChartView), "selection-changed", G_CALLBACK(selection_changed), this);
	g_signal_connect(G_OBJECT(m_ChartView), "item-activated", G_CALLBACK(item_activated), this);
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

	// Info box
	m_InfoBox.RegisterCallback(InfoBoxCallback, this);
	gtk_box_pack_start(GTK_BOX(hbox), m_InfoBox.Handle(), FALSE, FALSE, 0);
	m_InfoBox.SetCaption("Object inspector");
	
	// Timers
	m_TimerId = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 100, GSourceFunc(timer_cb), this, NULL);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	SetDisplayMode(m_DispMode);
	SetInfoMode(INFO_NONE);
}

//
// Destructor
//
CPhtFileDlg::~CPhtFileDlg(void)
{
	// Disconnect chart signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)zoom_changed, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)mouse_moved, this);

	// Release objects, free allocated memory
	g_source_remove(m_TimerId);
	if (m_SelectedPath)
		gtk_tree_path_free(m_SelectedPath);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_TableData)
		g_object_unref(m_TableData);
	if (m_Apertures)
		g_object_unref(m_Apertures);
	delete m_File;
}

bool CPhtFileDlg::LoadFile(const char *fpath, GError **error)
{
	CPhot *file = new CPhot();
	if (!file->Load(fpath, error)) {
		delete file;
		return false;
	}

	SetStatus(NULL);
	SetInfoMode(INFO_NONE);
	delete m_File;
	m_File = file;
	if (m_SelectedPath) {
		gtk_tree_path_free(m_SelectedPath);
		m_SelectedPath = NULL;
	}
	m_SelectedObjId = m_SelectedRow = -1;

	m_File->SelectAperture(m_ApertureIndex);
	if (m_DispMode==DISP_EMPTY)
		SetDisplayMode(DISP_CHART);
	UpdateApertures();
	UpdateTableHeader();
	UpdateChart();
	UpdateControls();
	UpdateZoom();
	return true;
}

// Environment changed, reload settings
void CPhtFileDlg::EnvironmentChanged(void)
{
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	UpdateChart();
	UpdateControls();
}

void CPhtFileDlg::UpdateControls(void)
{
	bool ok = m_File && m_File->Valid();

	// Menu File
	m_Menu.Enable(CMD_SAVE_AS, ok);
	m_Menu.Enable(CMD_EXPORT, ok);
	m_Menu.Enable(CMD_PROPERTIES, ok);

	// Menu View
	m_Menu.Enable(CMD_CHART, ok);
	m_Menu.Check(CMD_CHART, m_DispMode==DISP_CHART);
	m_Menu.Enable(CMD_TABLE, ok);
	m_Menu.Check(CMD_TABLE, m_DispMode==DISP_TABLE);
	m_Menu.Enable(CMD_RULERS, ok && m_DispMode==DISP_CHART);
	m_Menu.Check(CMD_RULERS, m_Rulers);

	// Menu Tools
	m_Menu.Check(CMD_INSPECTOR, m_InfoMode == INFO_OBJECT);

	// Apertures
	ok = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1;
	gtk_widget_set_sensitive(m_AperCombo, ok);
	gtk_widget_set_sensitive(m_AperLabel, ok);

	// Zoom
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), m_DispMode==DISP_CHART);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), m_DispMode==DISP_CHART);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), m_DispMode==DISP_CHART);
}

void CPhtFileDlg::UpdateZoom(void)
{
	double zoom, min, max;

	zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
	cmpack_chart_view_get_zoom_limits(CMPACK_CHART_VIEW(m_ChartView), &min, &max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), zoom < max);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), zoom > min);
}

void CPhtFileDlg::UpdateTableHeader(void)
{
	m_SortCol = NULL;

	if (m_File)
		m_File->SetView(GTK_TREE_VIEW(m_TableView));

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

void CPhtFileDlg::UpdateChart(void)
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
	m_SelectedRow = -1;

	if (m_File) {
		if (m_DispMode != DISP_TABLE) {
			m_ChartData = m_File->ToChartData(m_File->Matched());
			if (m_ChartData) 
				m_SelectedRow = cmpack_chart_data_find_item(m_ChartData, m_SelectedObjId);
			cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_ChartView), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
			cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_ChartView), m_Negative);
			cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), m_ChartData);
			if (m_SelectedRow>=0) 
				cmpack_chart_view_select(CMPACK_CHART_VIEW(m_ChartView), m_SelectedRow);
		} else {
			m_TableData = m_File->ToTreeModel();
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
	}
	m_Updating = false;
}

void CPhtFileDlg::UpdateApertures(void)
{
	char txt[128];
	GtkTreeIter iter;

	if (m_File)
		m_Aper = *m_File->Apertures();
	else
		m_Aper.Clear();

	// Update list of apertures
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_AperCombo), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_Aper.Count(); i++) {
		const CAperture *aper = m_Aper.Get(i);
		if (aper->Radius()>0)
			sprintf(txt, "#%d (%.2f)", aper->Id(), aper->Radius());
		else
			sprintf(txt, "#%d", aper->Id());
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
	if (m_File)
		m_File->SelectAperture(m_ApertureIndex);
}

void CPhtFileDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CPhtFileDlg *pMe = (CPhtFileDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CPhtFileDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_OPEN:
		Open();
		break;
	case CMD_SAVE_AS:
		SaveAs("C-Munipack photometry files", "*.pht");
		break;
	case CMD_CLOSE:
		Close();
		break;
	case CMD_PROPERTIES:
		ShowProperties();
		break;
	case CMD_EXPORT:
		Export();
		break;

	// View menu
	case CMD_CHART:
		SetDisplayMode(DISP_CHART);
		break;
	case CMD_TABLE:
		SetDisplayMode(DISP_TABLE);
		break;
	case CMD_RULERS:
		m_Rulers = m_Menu.IsChecked(CMD_RULERS);
		cmpack_chart_view_show_scales(CMPACK_CHART_VIEW(m_ChartView), m_Rulers);
		g_Project->SetBool("PreviewDlg", "Rulers", m_Rulers);
		break;

	// Tools menu
	case CMD_INSPECTOR:
		if (m_Menu.IsChecked(CMD_INSPECTOR)) 
			SetInfoMode(INFO_OBJECT);
		else if (m_InfoMode == INFO_OBJECT)
			SetInfoMode(INFO_NONE);
		break;

	// Help menu
	case CMD_SHOW_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_PHT_FILE_WINDOW);
		break;
	}
}

void CPhtFileDlg::ShowProperties(void)
{
	CPhtFileInfoDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.ShowModal(m_File, m_Name, m_Path);
}

void CPhtFileDlg::button_clicked(GtkWidget *button, CPhtFileDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CPhtFileDlg::OnButtonClicked(GtkWidget *pBtn)
{
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
		if (index>=0 && index!=m_ApertureIndex && m_File) {
			m_ApertureIndex = index;
			m_File->SelectAperture(m_ApertureIndex);
			UpdateChart();
			if (m_InfoMode==INFO_OBJECT && m_SelectedObjId>=0)
				UpdateInfoBox();
		}
	}
}

void CPhtFileDlg::selection_changed(GtkWidget *pChart, CPhtFileDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

//
// Left button click
//
void CPhtFileDlg::OnSelectionChanged(void)
{
	if (!m_Updating) {
		if (m_DispMode != DISP_TABLE) {
			int row = cmpack_chart_view_get_selected(CMPACK_CHART_VIEW(m_ChartView));
			if (row>=0 && m_ChartData) {
				if (row != m_SelectedRow) {
					m_SelectedRow = row;
					m_SelectedObjId = (int)cmpack_chart_data_get_param(m_ChartData, m_SelectedRow);
					UpdateInfoBox();
				}
			} else {
				if (m_SelectedRow>=0 || m_SelectedObjId>=0) {
					m_SelectedRow = m_SelectedObjId = -1;
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
					gtk_tree_model_get(m_TableData, &iter, CPhot::COL_ID, &m_SelectedObjId, -1);
					gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_TableView), m_SelectedPath, 0, 0, 0, 0);
					UpdateInfoBox();
				}
			} else {
				if (m_SelectedPath!=NULL || m_SelectedObjId>=0) {
					if (m_SelectedPath)
						gtk_tree_path_free(m_SelectedPath);
					m_SelectedPath = NULL;
					m_SelectedObjId = -1;
					UpdateInfoBox();
				}
			}
			g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
			g_list_free (list);
		}
	}
}

void CPhtFileDlg::item_activated(GtkWidget *button, gint row, CPhtFileDlg *pDlg)
{
	pDlg->OnItemActivated(row);
}

void CPhtFileDlg::OnItemActivated(gint row)
{
	if (m_InfoMode != INFO_OBJECT)
		SetInfoMode(INFO_OBJECT);
	OnSelectionChanged();
}

void CPhtFileDlg::mouse_moved(GtkWidget *button, CPhtFileDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

void CPhtFileDlg::zoom_changed(GtkWidget *button, CPhtFileDlg *pDlg)
{
	pDlg->m_UpdateZoom = true;
}

gboolean CPhtFileDlg::timer_cb(CPhtFileDlg *pDlg)
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

void CPhtFileDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CPhtFileDlg *pMe = (CPhtFileDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CPhtFileDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateControls();
}

void CPhtFileDlg::UpdateStatus(void)
{
	char	buf[256], aux[256];
	double	pos_x, pos_y;
	int		x, y, ref_id, index;

	if (m_DispMode==DISP_CHART) {
		int item = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(m_ChartView));
		if (item>=0 && m_ChartData && m_File) {
			if (m_LastFocus!=item) {
				m_LastFocus = item;
				index = m_File->FindObject((int)cmpack_chart_data_get_param(m_ChartData, item));
				sprintf(buf, "Object ");
				ref_id = m_File->GetObjectRefID(index);
				if (ref_id>=0) {
					sprintf(aux, "#%d", ref_id);
					strcat(buf, aux);
				} else {
					strcat(buf, "(not matched)");
				}
				if (m_File->GetObjectPos(index, &pos_x, &pos_y)) {
					sprintf(aux, ": X = %.1f, Y = %.1f", pos_x, pos_y);
					strcat(buf, aux);
				}
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

void CPhtFileDlg::SetDisplayMode(tDispMode mode)
{
	switch (mode) 
	{
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

	default:
		gtk_widget_hide(m_TableScrWnd);
		gtk_widget_show(m_ChartScrWnd);
		SetInfoMode(INFO_NONE);
		break;
	}
	m_DispMode = mode;
	g_Project->SetInt("PhtFileDlg", "Mode", m_DispMode);
	UpdateChart();
	UpdateInfoBox();
	UpdateStatus();
	UpdateControls();
}

void CPhtFileDlg::SetInfoMode(tInfoMode mode)
{
	m_InfoMode = mode;
	switch (m_InfoMode) 
	{
	case INFO_OBJECT:
		m_InfoBox.Show(true);
		break;
	default:
		m_InfoBox.Show(false);
		break;
	}
	UpdateInfoBox();
	UpdateControls();
}

void CPhtFileDlg::UpdateInfoBox(void)
{
	int		index;
	char	buf[256];
	CmpackPhtData data;
	CmpackError code;

	if (m_InfoMode == INFO_OBJECT && m_File) {
		m_InfoBox.BeginUpdate();
		m_InfoBox.Clear();
		// Show information about selected star
		if (m_SelectedObjId>=0) {
			CmpackPhtObject obj;
			sprintf(buf, "Object #%d", m_SelectedObjId);
			m_InfoBox.AddTitle(1, buf);
			index = m_File->FindObject(m_SelectedObjId);
			m_File->GetObjectParams(index, CMPACK_PO_CENTER | CMPACK_PO_SKY | CMPACK_PO_FWHM, &obj);
			code = CMPACK_ERR_OK;
			data.mag_valid = false;
			m_File->GetMagnitudeAndCode(index, data, code);
			if (obj.ref_id>=0) {
				sprintf(buf, "Reference id = %d", obj.ref_id);
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
					sprintf(buf, "S/N ratio = %.1f dB\n", -10.0*log10(1.0857/data.mag_error));
				else
					sprintf(buf, "S/N ratio = ---- dB\n");
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
				sprintf(buf, "S/N ratio = ---- dB\n");
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
}

bool CPhtFileDlg::SaveFile(const char *fpath, GError **error)
{
	return m_File->SaveAs(fpath, error);
}

void CPhtFileDlg::Export(void)
{
	if (m_DispMode == DISP_CHART) {
		CChartExportDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Execute(m_ChartData, NULL, m_Path, m_Negative, m_RowsUpward);
	} else {
		CExportPhtFileDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Execute(*m_File, m_Path, m_SortColumnId, m_SortType);
	}
}

void CPhtFileDlg::table_column_clicked(GtkTreeViewColumn *treeviewcolumn, CPhtFileDlg *pDlg)
{
	pDlg->OnTableColumnClicked(treeviewcolumn);
}

void CPhtFileDlg::OnTableColumnClicked(GtkTreeViewColumn *pCol)
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

void CPhtFileDlg::table_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, CPhtFileDlg *pMe)
{
	pMe->OnTableRowActivated(treeview, path);
}

void CPhtFileDlg::OnTableRowActivated(GtkTreeView *treeview, GtkTreePath *path)
{
	if (m_InfoMode != INFO_OBJECT)
		SetInfoMode(INFO_OBJECT);
	OnSelectionChanged();
}

//---------------------------   INFO DIALOG   --------------------------------

//
// Constructor
//
CPhtFileInfoDlg::CPhtFileInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0),
	m_Name(NULL)
{
	GtkWidget *bbox;

	AddHeading(FIELD_FILE, 0, 0, "File");
	AddField(FIELD_FILENAME, 0, 1, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_DIRPATH, 0, 2, "Location", PANGO_ELLIPSIZE_MIDDLE);
	AddSeparator(0, 4);
	AddHeading(FIELD_FRAME, 0, 5, "Frame");
	AddField(FIELD_DATETIME, 0, 6, "Date and time (UTC)");
	AddField(FIELD_JULDAT, 0, 7, "Julian date");
	AddField(FIELD_FILTER, 0, 8, "Optical filter", PANGO_ELLIPSIZE_END);
	AddField(FIELD_EXPTIME, 0, 9, "Exposure duration");
	AddSeparator(0, 10);
	AddHeading(FIELD_PHOTOMETRY, 0, 11, "Photometry");
	AddField(FIELD_STARS, 0, 12, "Number of stars");
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
void CPhtFileInfoDlg::ShowModal(CPhot *file, const gchar *name, const gchar *path)
{
	char buf[512];
	CmpackDateTime dt;

	m_File = file;
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

	if (file->JulianDate()>0) {
		cmpack_decodejd(file->JulianDate(), &dt);
		sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
		SetField(FIELD_DATETIME, buf);
		SetField(FIELD_JULDAT, file->JulianDate(), JD_PREC);
	} else {
		SetField(FIELD_DATETIME, "Not available");
		SetField(FIELD_JULDAT, "Not available");
	}
	if (file->Filter())
		SetField(FIELD_FILTER, file->Filter());
	else
		SetField(FIELD_FILTER, "");
	if (file->ExposureDuration()>=0)
		SetField(FIELD_EXPTIME, file->ExposureDuration(), 3, "second(s)");
	else
		SetField(FIELD_EXPTIME, "Not available");

	if (file->ObjectCount()>=0)
		SetField(FIELD_STARS, file->ObjectCount());
	else
		SetField(FIELD_STARS, "Not available");

	CInfoDlg::ShowModal();
}

void CPhtFileInfoDlg::button_clicked(GtkWidget *button, CPhtFileInfoDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CPhtFileInfoDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_HdrBtn))
		ShowHeader();
}

void CPhtFileInfoDlg::ShowHeader(void)
{
	CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(m_File, m_Name);
}

//------------------------   SAVE PHOTOMETRY FILE   ---------------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
} FileFormats[] = {
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
CExportPhtFileDlg::CExportPhtFileDlg(GtkWindow *pParent):m_pParent(pParent), 
	m_FileType(TYPE_CSV), m_Matched(false), m_Updating(false)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export table", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("chart");
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
	m_SkipInvalid = gtk_check_button_new_with_label("Discard objects without valid measurement");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipInvalid), "toggled", G_CALLBACK(button_toggled), this);
	m_SkipUnmatched = gtk_check_button_new_with_label("Discard objects that were not matched");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipUnmatched, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipUnmatched), "toggled", G_CALLBACK(button_toggled), this);
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CExportPhtFileDlg::~CExportPhtFileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
}

bool CExportPhtFileDlg::Execute(const CPhot &file, const gchar *current_path, 
	int sort_column_id, GtkSortType sort_type)
{
	m_Phot.MakeCopy(file);
	m_Phot.SelectAperture(file.SelectedAperture());
	m_Matched = false;
	for (int i=0; i<m_Phot.ObjectCount(); i++) {
		if (m_Phot.GetObjectRefID(i)>0) {
			m_Matched = true;
			break;
		}
	}

	m_Updating = true;

	// Restore last folder and file name
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	gchar *basename = g_path_get_basename(current_path);
	gchar *filename = SetFileExtension(basename, FileExtension(m_FileType));
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), filename);
	g_free(filename);
	g_free(basename);

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
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("SavePhtFile", "CSV_SKIP_INVALID");
	m_Options[TYPE_CSV].skip_unmatched = m_Matched && g_Project->GetBool("SavePhtFile", "CSV_SKIP_UNMATCHED");
	m_Options[TYPE_CSV].header = g_Project->GetBool("SavePhtFile", "CSV_HEADER", true);

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	gchar *dirpath = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);

	// Save settings
	if (m_Matched)
		g_Project->SetBool("SavePhtFile", "CSV_SKIP_UNMATCHED", m_Options[TYPE_CSV].skip_unmatched);
	g_Project->SetBool("SavePhtFile", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("SavePhtFile", "CSV_HEADER", m_Options[TYPE_CSV].header);

	// Create output file
	int res = 0, flags = 0;
	GError *error = NULL;
	const tOptions *opt = &m_Options[m_FileType];
	if (opt->skip_invalid)
		flags |= CPhot::EXPORT_SKIP_INVALID;
	if (opt->skip_unmatched)
		flags |= CPhot::EXPORT_SKIP_UNMATCHED;
	if (!opt->header)
		flags |= CPhot::EXPORT_NO_HEADER;
	if (!m_Phot.ExportTable(filename, FileMimeType(m_FileType), flags, sort_column_id, sort_type, &error)) {
		if (error) {
			ShowError(m_pParent, error->message);
			g_error_free(error);
		}
		res = -1;
	}
	g_free(filename);
	return (res==0);
}

void CExportPhtFileDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_Header, TRUE);
	gtk_widget_set_sensitive(m_SkipInvalid, TRUE);
	gtk_widget_set_sensitive(m_SkipUnmatched, m_Matched);
}

void CExportPhtFileDlg::button_toggled(GtkToggleButton *pWidget, CExportPhtFileDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CExportPhtFileDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_SkipUnmatched) {
		m_Options[m_FileType].skip_unmatched = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CExportPhtFileDlg::selection_changed(GtkComboBox *pWidget, CExportPhtFileDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportPhtFileDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft>=0 && ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
			}
		}
	}
}

void CExportPhtFileDlg::OnTypeChanged(void)
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

	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipUnmatched), opt->skip_unmatched);

	m_Updating = false;

	UpdateControls();
}

void CExportPhtFileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CExportPhtFileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExportPhtFileDlg::OnResponseDialog(gint response_id)
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
