/**************************************************************

catfile_dlg.cpp (C-Munipack project)
The preview dialog for catalog files
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

#include "catfile_dlg.h"
#include "configuration.h"
#include "configuration.h"
#include "showheader_dlg.h"
#include "export_dlgs.h"
#include "utils.h"
#include "profile.h"
#include "object_dlg.h"
#include "observer_dlg.h"
#include "choosestars_dlg.h"
#include "main.h"
#include "ctxhelp.h"

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
	CMD_OPEN = 100,
	CMD_SAVE_AS,
	CMD_CLOSE,
	CMD_PRINT,
	CMD_EXPORT,
	CMD_PROPERTIES,
	CMD_HEADER,
	CMD_STARS,
	CMD_CHART,
	CMD_TABLE,
	CMD_RULERS,
	CMD_INSPECTOR,
	CMD_HELP,
	CMD_SET_VARIABLE,
	CMD_SET_COMPARISON,
	CMD_SET_CHECK,
	CMD_UNSET,
	CMD_NEW_SELECTION,
	CMD_SAVE_SELECTION,
	CMD_OBJECT_PROPERTIES,
	CMD_EDIT_TAG,
	CMD_REMOVE_TAG,
	CMD_CLEAR_TAGS
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
	{ CMenuBar::MB_ITEM,	CMD_SAVE_AS,	"_Save as ..." },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT,		"_Export" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_PROPERTIES, "Show _Properties" },
	//{ CMenuBar::MB_SEPARATOR },
	//{ CMenuBar::MB_ITEM,	CMD_PRINT,		"P_rint" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_CLOSE,		"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem EditMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_HEADER,		"Edit _properties" },
	{ CMenuBar::MB_CHECKBTN, CMD_STARS,		"Change selected _stars" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN, CMD_CHART,			"_Chart" },
	{ CMenuBar::MB_RADIOBTN, CMD_TABLE,			"_Table" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_CHECKBTN, CMD_RULERS,	"_Rulers" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_CHECKBTN, CMD_INSPECTOR,	"_Object inspector" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_HELP,		"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu PreviewMenu[] = {
	{ "_File",	MENU_FILE,	FileMenu },
	{ "_Edit",	MENU_EDIT,	EditMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Tools", MENU_TOOLS,	ToolsMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

static const CPopupMenu::tPopupMenuItem ObjectMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_INSPECTOR,		"Show object _properties" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_STARS,			"Change selected _stars" },
	{ CPopupMenu::MB_END }
};

static const CPopupMenu::tPopupMenuItem ChartMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_STARS,			"Change selected _stars" },
	{ CPopupMenu::MB_END }
};

static const CPopupMenu::tPopupMenuItem SelectMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_SET_VARIABLE,	"_Variable" },
	{ CPopupMenu::MB_ITEM, CMD_SET_COMPARISON,	"_Comparison" },
	{ CPopupMenu::MB_ITEM, CMD_SET_CHECK,		"Chec_k" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_UNSET,			"_Unselect" },
	{ CPopupMenu::MB_ITEM, CMD_NEW_SELECTION,	"_New selection" },
	{ CPopupMenu::MB_ITEM, CMD_SAVE_SELECTION,	"_Save selection as..." },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_EDIT_TAG,		"_Edit tag" },
	{ CPopupMenu::MB_ITEM, CMD_REMOVE_TAG,		"_Remove tag" },
	{ CPopupMenu::MB_ITEM, CMD_CLEAR_TAGS,		"Clear _all tags" },
	{ CPopupMenu::MB_END }
};

static const CPopupMenu::tPopupMenuItem ContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_NEW_SELECTION,	"_New selection" },
	{ CPopupMenu::MB_ITEM, CMD_SAVE_SELECTION,	"_Save selection as..." },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_CLEAR_TAGS,		"Clear _all tags" },
	{ CPopupMenu::MB_END }
};

//-------------------------   PRIVATE DATA   ---------------------------

static const struct {
	const gchar *label;
	CmpackColor color;
	const gchar *color_name;
} Types[CMPACK_SELECT_COUNT] = {
	{ NULL, CMPACK_COLOR_DEFAULT, NULL },
	{ "var", CMPACK_COLOR_RED, "red" },
	{ "comp", CMPACK_COLOR_GREEN, "green" },
	{ "check", CMPACK_COLOR_BLUE, "blue" }
};

//-------------------------   PREVIEW DIALOG   --------------------------------

//
// Constructor
//
CCatFileDlg::CCatFileDlg(void):m_File(NULL), m_ChartData(NULL), m_TableData(NULL), m_Updating(false), 
	m_UpdatePos(true), m_UpdateZoom(true), m_EditSelection(false), m_NotSavedBackup(false), 
	m_ShowNewSelection(false), m_LastPosX(-1), m_LastPosY(-1), m_LastFocus(-1), 
	m_SelectedRow(-1), m_SelectedObjId(-1), m_SelectionIndex(-1), 
	m_SelectionIndexBackup(-1), m_InfoMode(INFO_NONE), m_SortColumnId(-1), 
	m_SortType(GTK_SORT_ASCENDING), m_SortCol(NULL), m_SelectedPath(NULL)
{
	GtkWidget *tbox, *hbox;

	m_DispMode = (tDispMode)g_Project->GetInt("CatFileDlg", "Mode", DISP_CHART, 0, DISP_MODE_COUNT-1);
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

	// Object selection
	toolbar_new_label(tbox, "Selection");
	m_SelectCbx = toolbar_new_combo(tbox, "Choose an item to restore recently used object selection");
	m_Selections = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), GTK_TREE_MODEL(m_Selections));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_SelectCbx), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_SelectCbx), renderer, "text", 1);
	g_signal_connect(G_OBJECT(m_SelectCbx), "changed", G_CALLBACK(combo_changed), this);
	m_ClearBtn = toolbar_new_button(tbox, "New", "Start a new object selection");
	g_signal_connect(G_OBJECT(m_ClearBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_SaveBtn = toolbar_new_button(tbox, "Save as...", "Save the current object selection");
	g_signal_connect(G_OBJECT(m_SaveBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_RemoveBtn = toolbar_new_button(tbox, "Remove", "Remove the current object selection from the list");
	g_signal_connect(G_OBJECT(m_RemoveBtn), "clicked", G_CALLBACK(button_clicked), this);
	toolbar_new_separator(tbox);

	// Zoom
	toolbar_new_label(tbox, "Zoom");
	m_ZoomFit = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_FIT, "Fit the frame to the window");
	g_signal_connect(G_OBJECT(m_ZoomFit), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomOut = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_OUT, "Zoom out");
	g_signal_connect(G_OBJECT(m_ZoomOut), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomIn = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_IN, "Zoom in");
	g_signal_connect(G_OBJECT(m_ZoomIn), "clicked", G_CALLBACK(button_clicked), this);

	// Notice bar at the top
	m_NoteBox = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(m_NoteBox), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_NoteBox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_NoteBox), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	m_NoteIcon = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(hbox), m_NoteIcon, FALSE, TRUE, 8);
	m_NoteCap = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), m_NoteCap, TRUE, TRUE, 8);
	gtk_misc_set_alignment(GTK_MISC(m_NoteCap), 0.0, 0.5);
	m_NoteApplyBtn = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_tooltip_text(m_NoteApplyBtn, "Save changes and leave editing mode");
	gtk_box_pack_start(GTK_BOX(hbox), m_NoteApplyBtn, FALSE, TRUE, 8);
	g_signal_connect(G_OBJECT(m_NoteApplyBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_NoteDiscardBtn = gtk_button_new_from_stock(GTK_STOCK_DISCARD);
	gtk_widget_set_tooltip_text(m_NoteDiscardBtn, "Discard changes and leave editing mode");
	gtk_box_pack_start(GTK_BOX(hbox), m_NoteDiscardBtn, FALSE, TRUE, 8);
	g_signal_connect(G_OBJECT(m_NoteDiscardBtn), "clicked", G_CALLBACK(button_clicked), this);

	// Chart and Infobox
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
	g_signal_connect(G_OBJECT(m_ChartView), "button_press_event", G_CALLBACK(button_press_event), this);
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
	g_signal_connect(G_OBJECT(m_TableView), "button_press_event", G_CALLBACK(button_press_event), this);
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

	// Make popup menus
	m_ObjectMenu.Create(ObjectMenu);
	m_ChartMenu.Create(ChartMenu);
	m_SelectMenu.Create(SelectMenu);
	m_ContextMenu.Create(ContextMenu);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
	gtk_widget_hide(m_NoteBox);
	gtk_widget_hide(GTK_WIDGET(m_ClearBtn));
	gtk_widget_hide(GTK_WIDGET(m_SaveBtn));
	gtk_widget_hide(GTK_WIDGET(m_RemoveBtn));
	SetDisplayMode(m_DispMode);
	SetInfoMode(INFO_NONE);
}

//
// Destructor
//
CCatFileDlg::~CCatFileDlg(void)
{
	// Disconnect chart signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)zoom_changed, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_ChartView), (gpointer)mouse_moved, this);

	// Release objects, free allocated memory
	g_source_remove(m_TimerId);
	gtk_tree_path_free(m_SelectedPath);
	if (m_TableData)
		g_object_unref(m_TableData);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_Selections)
		g_object_unref(m_Selections);
	delete m_File;
}

//
// Load catalog file
//
bool CCatFileDlg::LoadFile(const char *fpath, GError **error)
{
	CCatalog *file = new CCatalog;
	if (!file->Load(fpath, error)) {
		delete file;
		return false;
	}

	delete m_File;
	m_File = file;
	SetStatus(NULL);
	m_SelectionList.Clear();
	m_Tags.Clear();
	m_ShowNewSelection = false;
	m_SelectionIndex = -1;
	m_SelectedRow = m_SelectedObjId = -1;
	SetInfoMode(INFO_NONE);
	if (m_DispMode==DISP_EMPTY)
		SetDisplayMode(DISP_CHART);
	if (m_File->Selections())
		LoadSelectionList(*m_File->Selections());
	m_Tags = *m_File->Tags();
	UpdateTableHeader();
	UpdateSelectionList();
	UpdateChart();
	UpdateControls();
	UpdateZoom();
	return true;
}

// Environment changed, reload settings
void CCatFileDlg::EnvironmentChanged(void)
{
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);
	UpdateChart();
	UpdateControls();
}

void CCatFileDlg::UpdateControls(void)
{
	bool ok = m_File && m_File->Valid();

	// Menu File
	m_Menu.Enable(CMD_SAVE_AS, ok);
	m_Menu.Enable(CMD_EXPORT, ok);
	m_Menu.Enable(CMD_PROPERTIES, ok);

	// Menu Edit
	m_Menu.Enable(CMD_STARS, ok);
	m_Menu.Check(CMD_STARS, m_EditSelection);

	// Menu View
	m_Menu.Enable(CMD_CHART, ok);
	m_Menu.Check(CMD_CHART, m_DispMode==DISP_CHART);
	m_Menu.Enable(CMD_TABLE, ok);
	m_Menu.Check(CMD_TABLE, m_DispMode==DISP_TABLE);
	m_Menu.Enable(CMD_RULERS, ok && m_DispMode==DISP_CHART);
	m_Menu.Check(CMD_RULERS, m_Rulers);

	// Menu Tools
	m_Menu.Enable(CMD_INSPECTOR, !m_EditSelection);
	m_Menu.Check(CMD_INSPECTOR, m_InfoMode == INFO_OBJECT);

	// Selection
	gtk_widget_set_sensitive(GTK_WIDGET(m_ClearBtn), m_EditSelection && (m_SelectionIndex>=0 || m_CurrentSelection.Count()>0));
	gtk_widget_set_sensitive(GTK_WIDGET(m_SaveBtn), m_EditSelection && m_CurrentSelection.Count()>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_RemoveBtn), m_EditSelection && m_SelectionIndex>=0);
	gtk_widget_set_sensitive(m_SelectCbx, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Selections), NULL)>1);
}

void CCatFileDlg::LoadSelectionList(const CSelectionList &list)
{
	m_SelectionList.Clear();
	if (list.Count()>0) {
		int index = 1;
		for (int i=0; i<list.Count(); i++) {
			gchar *news_name = NULL;
			if (list.Name(i) && m_SelectionList.IndexOf(list.Name(i))<0) 
				news_name = g_strdup(list.Name(i));
			else if (m_Name && m_SelectionList.IndexOf(m_Name)<0) 
				news_name = g_strdup(m_Name);
			while (!news_name) {
				gchar *name = (gchar*)g_malloc(256*sizeof(gchar));
				sprintf(name, "Selection %d", index);
				if (m_SelectionList.IndexOf(name)<0) 
					news_name = name;
				else
					g_free(name);
				index++;
			}
			m_SelectionList.Set(news_name, list.At(i));
			g_free(news_name);
		}
	}
	if (m_SelectionList.Count()>0) {
		m_SelectionIndex = 0;
		m_CurrentSelection = m_SelectionList.At(0);
	} else {
		m_SelectionIndex = -1;
		m_CurrentSelection.Clear();
	}
}

void CCatFileDlg::UpdateSelectionList(void)
{
	m_Updating = true;

	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), NULL);
	gtk_list_store_clear(m_Selections);
	if (m_ShowNewSelection) {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, -1, 1, "New selection", -1);
	}
	for (int i=0; i<m_SelectionList.Count(); i++) {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, i, 1, m_SelectionList.Name(i), -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), GTK_TREE_MODEL(m_Selections));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Selections), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_SelectCbx), m_SelectionIndex);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_SelectCbx))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_SelectCbx), 0);
			m_SelectionIndex = (tDateFormat)SelectedItem(GTK_COMBO_BOX(m_SelectCbx));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_SelectCbx), -1);
		m_SelectionIndex = -1;
	}
	m_Updating = false;
}

void CCatFileDlg::UpdateTableHeader(void)
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

void CCatFileDlg::UpdateChart(void)
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
			m_ChartData = m_File->ToChartData(false, true, false);
			if (m_ChartData) {
				int count = cmpack_chart_data_count(m_ChartData);
				for (int row=0; row<count; row++) {
					if (m_SelectedObjId == cmpack_chart_data_get_param(m_ChartData, row))
						m_SelectedRow = row;
				}
				cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_ChartView), m_ChartData);
				if (m_SelectedRow>=0)
					cmpack_chart_view_select(CMPACK_CHART_VIEW(m_ChartView), m_SelectedRow);
				cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_ChartView), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
				cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_ChartView), m_Negative);
				UpdateAll();
				UpdateZoom();
			}
		} else {
			m_TableData = m_File->ToTreeModel(false, true);
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
				UpdateAll();
			}
		}
	}
	m_Updating = false;
}

void CCatFileDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CCatFileDlg *pMe = (CCatFileDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

void CCatFileDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_OPEN:
		Open();
		break;
	case CMD_SAVE_AS:
		SaveAs("C-Munipack catalog files", "*.xml");
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

	// Edit menu
	case CMD_HEADER:
		EditHeader();
		break;
	case CMD_STARS:
		if (!m_EditSelection) {				
			BeginEditSelection();				// Start editing
		} else {
			switch (ConfirmSelectionChanges())
			{
			case GTK_RESPONSE_ACCEPT:
				EndEditSelection(true);			// Apply changes
				break;
			case GTK_RESPONSE_REJECT:
				EndEditSelection(false);		// Discard changes
				break;
			default:
				break;							// Continue editing
			}
		}
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
	case CMD_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CATALOG_FILE_WINDOW);
		break;
	}
}

void CCatFileDlg::ShowProperties(void)
{
	CCatFileInfoDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.ShowModal(m_File, m_Name, m_Path);
}

void CCatFileDlg::EditHeader(void)
{
	CCatEditHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(*m_File, m_Name)) {
		m_NotSaved = true;
		UpdateTitle();
		UpdateControls();
	}
}

// Turn editing on
void CCatFileDlg::BeginEditSelection()
{
	if (!m_EditSelection) {
		m_EditSelection = true;
		m_SelectionChanged = false;
		m_ShowNewSelection = (m_SelectionIndex<0);
		m_NewSelection.Clear();
		m_SelectionListBackup = m_SelectionList;
		m_SelectionIndexBackup = m_SelectionIndex;
		m_TagsBackup = m_Tags;
		SetInfoMode(INFO_NONE);
		UpdateSelectionList();
		cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_NONE);
		cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_CLICK);
		gtk_image_set_from_stock(GTK_IMAGE(m_NoteIcon), GTK_STOCK_EDIT, GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_label_set_text(GTK_LABEL(m_NoteCap), "You can edit the selection of stars now. Click on a star to change its type.");
		gtk_widget_show_all(m_NoteBox);
		gtk_widget_show(GTK_WIDGET(m_ClearBtn));
		gtk_widget_show(GTK_WIDGET(m_SaveBtn));
		gtk_widget_show(GTK_WIDGET(m_RemoveBtn));
		UpdateControls();
	}
}

// Turn editing off, apply or discard changes
void CCatFileDlg::EndEditSelection(bool apply)
{
	if (m_EditSelection) {
		if (apply && m_SelectionIndex<0 && m_CurrentSelection.Count()>0) {
			int res = ShowYesNoCancel(GTK_WINDOW(m_pDlg), "The current selection does not have a name. If you want to save it to the catalog file, you have to name it.\n\nDo you want to save the current selection?");
			if (res == GTK_RESPONSE_YES) {
				// Save changes
				CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Save selection as...");
				gchar *name = dlg.Execute("Enter name for the current selection:", 255, "", 
					(CTextQueryDlg::tValidator*)name_validator, this);
				if (name) {
					m_SelectionList.Set(name, m_CurrentSelection);
					m_SelectionIndex = m_SelectionList.IndexOf(name);
					g_free(name);
				}
			}
			if (res != GTK_RESPONSE_NO)
				return;
		}
		m_EditSelection = false;
		cmpack_chart_view_set_selection_mode(CMPACK_CHART_VIEW(m_ChartView), GTK_SELECTION_SINGLE);
		cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_ChartView), CMPACK_ACTIVATION_DBLCLICK);
		gtk_widget_hide(m_NoteBox);
		gtk_widget_hide(GTK_WIDGET(m_ClearBtn));
		gtk_widget_hide(GTK_WIDGET(m_SaveBtn));
		gtk_widget_hide(GTK_WIDGET(m_RemoveBtn));
		if (!apply && m_SelectionChanged) {
			m_NotSaved = m_NotSavedBackup;
			m_SelectionList = m_SelectionListBackup;
			m_SelectionIndex = m_SelectionIndexBackup;
			m_Tags = m_TagsBackup;
		}
		m_CurrentSelection.Clear();
		if (m_SelectionIndex<0 || m_SelectionIndex>=m_SelectionList.Count()) {
			if (m_SelectionList.Count()>0)
				m_SelectionIndex = 0;
			else
				m_SelectionIndex = -1;
		}
		if (m_SelectionIndex>=0)
			m_CurrentSelection = m_SelectionList.At(m_SelectionIndex);
		else 
			m_CurrentSelection.Clear();
		m_ShowNewSelection = false;
		UpdateSelectionList();
		UpdateAll();
		UpdateTitle();
		UpdateStatus();
		UpdateControls();
	}
}

void CCatFileDlg::button_clicked(GtkWidget *button, CCatFileDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CCatFileDlg::OnButtonClicked(GtkWidget *pBtn)
{
	double zoom;

	if (pBtn==GTK_WIDGET(m_ZoomIn)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_ChartView), zoom + 5.0);
	} else if (pBtn==GTK_WIDGET(m_ZoomOut)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_ChartView), zoom - 5.0);
	} else if (pBtn==GTK_WIDGET(m_ZoomFit))
		cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_ChartView), TRUE);
	else if (pBtn==GTK_WIDGET(m_NoteApplyBtn)) 
		EndEditSelection(true);
	else if (pBtn==GTK_WIDGET(m_NoteDiscardBtn))
		EndEditSelection(false);
	if (pBtn==GTK_WIDGET(m_ClearBtn)) 
		NewSelection();
	else if (pBtn==GTK_WIDGET(m_SaveBtn)) 
		SaveSelection();
	else if (pBtn==GTK_WIDGET(m_RemoveBtn)) 
		RemoveSelection();
}

//
// Left button click
//
void CCatFileDlg::selection_changed(GtkWidget *pChart, CCatFileDlg *pDlg)
{
	pDlg->OnSelectionChanged();
}

void CCatFileDlg::OnSelectionChanged(void)
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

void CCatFileDlg::item_activated(GtkWidget *button, gint row, CCatFileDlg *pDlg)
{
	pDlg->OnItemActivated(row);
}

void CCatFileDlg::OnItemActivated(gint row)
{
	if (!m_EditSelection) {
		if (m_InfoMode != INFO_OBJECT)
			SetInfoMode(INFO_OBJECT);
		OnSelectionChanged();
	} else {
		GdkEventButton ev;
		ev.button = 1;
		ev.time = gtk_get_current_event_time();
		OnSelectMenu(&ev, row);
	}
}

//
// Right mouse click
//
gint CCatFileDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CCatFileDlg *pMe)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		if (widget==pMe->m_ChartView) {
			int focused = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(widget));
			if (focused>=0) {
				if (!pMe->m_EditSelection)
					pMe->OnObjectMenu(event, focused);
				else
					pMe->OnSelectMenu(event, focused);
			} else {
				if (!pMe->m_EditSelection)
					pMe->OnChartMenu(event);
				else
					pMe->OnContextMenu(event);
			}
		} else
		if (widget==pMe->m_TableView) {
			int x = (int)event->x, y = (int)event->y;
			GtkTreePath *path;
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(pMe->m_TableView), x, y, &path, NULL, NULL, NULL)) {
				GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(pMe->m_TableView));
				if (gtk_tree_selection_count_selected_rows(sel)<=1)
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(pMe->m_TableView), path, NULL, FALSE);
				if (!pMe->m_EditSelection)
					pMe->OnObjectMenu(event, path);
				else
					pMe->OnSelectMenu(event, path);
				gtk_tree_path_free(path);
			} else {
				if (!pMe->m_EditSelection)
					pMe->OnChartMenu(event);
				else
					pMe->OnContextMenu(event);
			}
		}
		return TRUE;
	}
	return FALSE;
}

//
// Object's context menu (preview mode)
//
void CCatFileDlg::OnObjectMenu(GdkEventButton *ev, gint row)
{
	switch (m_ObjectMenu.Execute(ev))
	{
	case CMD_STARS:
		BeginEditSelection();
		break;
	case CMD_INSPECTOR:
		cmpack_chart_view_select(CMPACK_CHART_VIEW(m_ChartView), row);
		if (m_InfoMode!=INFO_OBJECT) 
			SetInfoMode(INFO_OBJECT);
		break;
	}
}

//
// Object's context menu (preview mode)
//
void CCatFileDlg::OnObjectMenu(GdkEventButton *ev, GtkTreePath *path)
{
	switch (m_ObjectMenu.Execute(ev))
	{
	case CMD_STARS:
		BeginEditSelection();
		break;
	case CMD_INSPECTOR:
		gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TableView)), path);
		if (m_InfoMode!=INFO_OBJECT) 
			SetInfoMode(INFO_OBJECT);
		break;
	}
}

//
// Context menu (no object focused)
//
void CCatFileDlg::OnChartMenu(GdkEventButton *ev)
{
	switch (m_ChartMenu.Execute(ev))
	{
	case CMD_STARS:
		BeginEditSelection();
		break;
	}
}

//
// Object's context menu (edit mode)
//
void CCatFileDlg::OnSelectMenu(GdkEventButton *ev, gint row)
{
	int star_id, index;
	CmpackSelectionType type;

	if (!m_ChartData)
		return;

	star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
	index = m_CurrentSelection.IndexOf(star_id);
	if (index>=0)
		type = m_CurrentSelection.GetType(index);
	else
		type = CMPACK_SELECT_NONE;
	m_SelectMenu.Enable(CMD_SET_VARIABLE, type!=CMPACK_SELECT_VAR);
	m_SelectMenu.Enable(CMD_SET_COMPARISON, type!=CMPACK_SELECT_COMP);
	m_SelectMenu.Enable(CMD_SET_CHECK, type!=CMPACK_SELECT_CHECK);
	m_SelectMenu.Enable(CMD_UNSET, type!=CMPACK_SELECT_NONE);
	m_SelectMenu.Enable(CMD_NEW_SELECTION, m_CurrentSelection.Count()>0);
	m_SelectMenu.Enable(CMD_REMOVE_TAG, m_Tags.Get(star_id)!=0);
	m_SelectMenu.Enable(CMD_CLEAR_TAGS, m_Tags.Count()>0);
	switch (m_SelectMenu.Execute(ev))
	{
	case CMD_SET_VARIABLE:
		UnselectType(CMPACK_SELECT_VAR);
		Select(row, CMPACK_SELECT_VAR);
		break;
	case CMD_SET_COMPARISON:
		if (!g_Project->Profile()->GetBool(CProfile::MULTIPLE_COMP_STARS))
			UnselectType(CMPACK_SELECT_COMP);
		Select(row, CMPACK_SELECT_COMP);
		break;
	case CMD_SET_CHECK:
		Select(row, CMPACK_SELECT_CHECK);
		break;
	case CMD_UNSET:
		Unselect(row);
		break;
	case CMD_NEW_SELECTION:
		NewSelection();
		break;
	case CMD_EDIT_TAG:
		EditTag(row);
		break;
	case CMD_REMOVE_TAG:
		RemoveTag(row);
		break;
	case CMD_CLEAR_TAGS:
		ClearTags();
		break;
	}
}

//
// Object's context menu (edit mode)
//
void CCatFileDlg::OnSelectMenu(GdkEventButton *ev, GtkTreePath *path)
{
	int star_id;
	CmpackSelectionType type;
	GtkTreeIter iter;

	if (!m_TableData || !gtk_tree_model_get_iter(m_TableData, &iter, path))
		return;

	gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
	int index = m_CurrentSelection.IndexOf(star_id);
	if (index>=0)
		type = m_CurrentSelection.GetType(index);
	else
		type = CMPACK_SELECT_NONE;
	m_SelectMenu.Enable(CMD_SET_VARIABLE, type!=CMPACK_SELECT_VAR);
	m_SelectMenu.Enable(CMD_SET_COMPARISON, type!=CMPACK_SELECT_COMP);
	m_SelectMenu.Enable(CMD_SET_CHECK, type!=CMPACK_SELECT_CHECK);
	m_SelectMenu.Enable(CMD_UNSET, type!=CMPACK_SELECT_NONE);
	m_SelectMenu.Enable(CMD_NEW_SELECTION, m_CurrentSelection.Count()>0);
	m_SelectMenu.Enable(CMD_REMOVE_TAG, m_Tags.Get(star_id)!=0);
	m_SelectMenu.Enable(CMD_CLEAR_TAGS, m_Tags.Count()>0);
	switch (m_SelectMenu.Execute(ev))
	{
	case CMD_SET_VARIABLE:
		UnselectType(CMPACK_SELECT_VAR);
		Select(path, CMPACK_SELECT_VAR);
		break;
	case CMD_SET_COMPARISON:
		if (!g_Project->Profile()->GetBool(CProfile::MULTIPLE_COMP_STARS))
			UnselectType(CMPACK_SELECT_COMP);
		Select(path, CMPACK_SELECT_COMP);
		break;
	case CMD_SET_CHECK:
		Select(path, CMPACK_SELECT_CHECK);
		break;
	case CMD_UNSET:
		Unselect(path);
		break;
	case CMD_NEW_SELECTION:
		NewSelection();
		break;
	case CMD_EDIT_TAG:
		EditTag(path);
		break;
	case CMD_REMOVE_TAG:
		RemoveTag(path);
		break;
	case CMD_CLEAR_TAGS:
		ClearTags();
		break;
	}
}

//
// Context menu (no object focused)
//
void CCatFileDlg::OnContextMenu(GdkEventButton *ev)
{
	m_ContextMenu.Enable(CMD_NEW_SELECTION, m_CurrentSelection.Count()>0);
	m_ContextMenu.Enable(CMD_CLEAR_TAGS, m_Tags.Count()>0);
	switch (m_ContextMenu.Execute(ev))
	{
	case CMD_NEW_SELECTION:
		NewSelection();
		break;
	case CMD_CLEAR_TAGS:
		ClearTags();
		break;
	}
}

void CCatFileDlg::Select(int row, CmpackSelectionType type)
{
	if (row>=0 && m_ChartData) {
		DettachSelection();
		int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
		m_CurrentSelection.Select(star_id, type);
		UpdateObject(m_ChartData, row);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
		}
		UpdateControls();
	}
}

void CCatFileDlg::Select(GtkTreePath *path, CmpackSelectionType type)
{
	GtkTreeIter iter;
	if (m_TableData && gtk_tree_model_get_iter(m_TableData, &iter, path)) {
		DettachSelection();
		int star_id;
		gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
		m_CurrentSelection.Select(star_id, type);
		UpdateObject(m_TableData, &iter);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
		}
		UpdateControls();
	}
}

void CCatFileDlg::Unselect(int row)
{
	if (row>=0 && m_ChartData) {
		DettachSelection();
		int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
		m_CurrentSelection.Select(star_id, CMPACK_SELECT_NONE);
		UpdateObject(m_ChartData, row);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
		}
		UpdateControls();
	}
}

void CCatFileDlg::Unselect(GtkTreePath *path)
{
	GtkTreeIter iter;
	if (m_TableData && gtk_tree_model_get_iter(m_TableData, &iter, path)) {
		DettachSelection();
		int star_id;
		gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
		m_CurrentSelection.Select(star_id, CMPACK_SELECT_NONE);
		UpdateObject(m_TableData, &iter);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
		}
		UpdateControls();
	}
}

void CCatFileDlg::NewSelection(void)
{
	DettachSelection();
	m_CurrentSelection = CSelection();
	m_NewSelection = m_CurrentSelection;
	m_SelectionIndex = -1;
	UpdateAll();
	m_SelectionChanged = true;
	if (!m_NotSaved) {
		m_NotSaved = true;
		UpdateTitle();
	}
	UpdateControls();
}

void CCatFileDlg::SaveSelection(void)
{
	const gchar *defValue = (m_SelectionIndex>=0 ? m_SelectionList.Name(m_SelectionIndex) : "");
	CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Save selection as...");
	gchar *name = dlg.Execute("Enter name for the current selection:", 255, defValue, 
		(CTextQueryDlg::tValidator*)name_validator, this);
	if (name) {
		if (m_SelectionIndex<0) 
			m_ShowNewSelection = false;
		else
			m_SelectionList.RemoveAt(m_SelectionIndex);
		m_SelectionList.Set(name, m_CurrentSelection);
		m_SelectionIndex = m_SelectionList.IndexOf(name);
		g_free(name);
		UpdateSelectionList();
		UpdateControls();
	}
}

bool CCatFileDlg::name_validator(const gchar *name, GtkWindow *parent, CCatFileDlg *pMe)
{
	return pMe->OnNameValidator(name, parent);
}

bool CCatFileDlg::OnNameValidator(const gchar *name, GtkWindow *parent)
{
	if (!name || name[0]=='\0') {
		ShowError(parent, "Please, specify name of the selection.");
		return false;
	}
	int i = m_SelectionList.IndexOf(name);
	if (i>=0 && (m_SelectionIndex<0 || i!=m_SelectionIndex))
		return ShowConfirmation(parent, "A selection with the specified name already exists.\nDo you want to overwrite it?");
	return true;
}

void CCatFileDlg::RemoveSelection(void)
{
	if (m_SelectionIndex<0)
		return;

	m_SelectionList.RemoveAt(m_SelectionIndex);
	if (m_SelectionIndex>=m_SelectionList.Count())
		m_SelectionIndex = m_SelectionList.Count()-1;
	if (m_SelectionIndex<0) {
		m_ShowNewSelection = true;
		m_NewSelection = CSelection();
		m_CurrentSelection = m_NewSelection;
		m_SelectionIndex = -1;
	} else  {
		m_CurrentSelection = m_SelectionList.At(m_SelectionIndex);
		m_SelectionIndex = m_SelectionIndex;
	}
	UpdateAll();
	UpdateSelectionList();
	UpdateControls();
}

void CCatFileDlg::EditTag(int row)
{
	char obj[256], buf[256];

	if (row>=0) {
		CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Edit tag");
		int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
		int i = m_CurrentSelection.IndexOf(star_id);
		if (i>=0) {
			int index = m_CurrentSelection.GetIndex(i);
			CmpackSelectionType type = m_CurrentSelection.GetType(i);
			if (index==1)
				strcpy(obj, Types[type].label);
			else
				sprintf(obj, "%s #%d", Types[type].label, index);
		} else 
			sprintf(obj, "object #%d", star_id);
		sprintf(buf, "Enter caption for '%s':", obj);
		gchar *value = dlg.Execute(buf, MAX_TAG_SIZE, m_Tags.Get(star_id), 
			(CTextQueryDlg::tValidator*)tag_validator, this);
		SetTag(row, value);
		g_free(value);
	}
}

void CCatFileDlg::EditTag(GtkTreePath *path)
{
	char obj[256], buf[256];

	GtkTreeIter iter;
	if (m_TableData && gtk_tree_model_get_iter(m_TableData, &iter, path)) {
		CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Edit tag");
		int star_id;
		gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
		int i = m_CurrentSelection.IndexOf(star_id);
		if (i>=0) {
			int index = m_CurrentSelection.GetIndex(i);
			CmpackSelectionType type = m_CurrentSelection.GetType(i);
			if (index==1)
				strcpy(obj, Types[type].label);
			else
				sprintf(obj, "%s #%d", Types[type].label, index);
		} else 
			sprintf(obj, "object #%d", star_id);
		sprintf(buf, "Enter caption for '%s':", obj);
		gchar *value = dlg.Execute(buf, MAX_TAG_SIZE, m_Tags.Get(star_id), 
			(CTextQueryDlg::tValidator*)tag_validator, this);
		SetTag(path, value);
		g_free(value);
	}
}

bool CCatFileDlg::tag_validator(const gchar *name, GtkWindow *parent, CCatFileDlg *pMe)
{
	return pMe->OnTagValidator(name, parent);
}

bool CCatFileDlg::OnTagValidator(const gchar *name, GtkWindow *parent)
{
	if (!name || name[0]=='\0') {
		ShowError(parent, "Please, specify caption for the new tag.");
		return false;
	}
	return true;
}

void CCatFileDlg::SetTag(int row, const gchar *value)
{
	if (row>=0) {
		int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
		if (value && value[0]!='\0') 
			m_Tags.Set(star_id, value);
		else
			m_Tags.Remove(star_id);
		UpdateObject(m_ChartData, row);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
			UpdateControls();
		}
	}
}

void CCatFileDlg::SetTag(GtkTreePath *path, const gchar *value)
{
	GtkTreeIter iter;
	if (m_TableData && gtk_tree_model_get_iter(m_TableData, &iter, path)) {
		int star_id;
		gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
		if (value && value[0]!='\0') 
			m_Tags.Set(star_id, value);
		else
			m_Tags.Remove(star_id);
		UpdateObject(m_TableData, &iter);
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
			UpdateControls();
		}
	}
}

void CCatFileDlg::RemoveTag(int row)
{
	if (row>=0) {
		int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
		if (m_Tags.Get(star_id)) {
			m_Tags.Remove(star_id);
			UpdateObject(m_ChartData, row);
			m_SelectionChanged = true;
			if (!m_NotSaved) {
				m_NotSaved = true;
				UpdateTitle();
			}
			UpdateControls();
		}
	}
}

void CCatFileDlg::RemoveTag(GtkTreePath *path)
{
	GtkTreeIter iter;
	if (m_TableData && gtk_tree_model_get_iter(m_TableData, &iter, path)) {
		int star_id;
		gtk_tree_model_get(m_TableData, &iter, CCatalog::COL_ID, &star_id, -1);
		if (m_Tags.Get(star_id)) {
			m_Tags.Remove(star_id);
			UpdateObject(m_TableData, &iter);
			m_SelectionChanged = true;
			if (!m_NotSaved) {
				m_NotSaved = true;
				UpdateTitle();
			}
			UpdateControls();
		}
	}
}

void CCatFileDlg::ClearTags(void)
{
	if (m_Tags.Count()>0) {
		m_Tags.Clear();
		UpdateAll();
		m_SelectionChanged = true;
		if (!m_NotSaved) {
			m_NotSaved = true;
			UpdateTitle();
		}
		UpdateControls();
	}
}

void CCatFileDlg::UnselectType(CmpackSelectionType type)
{
	int count = m_CurrentSelection.CountStars(type);
	if (count>0 && m_ChartData) {
		DettachSelection();
		int *stars = (int*)g_malloc(count*sizeof(int));
		m_CurrentSelection.GetStarList(type, stars, count);
		for (int i=0; i<count; i++) 
			Unselect(cmpack_chart_data_find_item(m_ChartData, stars[i]));
		g_free(stars);
	}
}

//
// Update displayed object
//
void CCatFileDlg::UpdateObject(CmpackChartData *data, int row) 
{
	int star_id = cmpack_chart_data_get_param(data, row);
	int i = m_CurrentSelection.IndexOf(star_id);
	const gchar *tag = m_Tags.Get(star_id);
	if (i>=0) {
		// Selected object
		int index = m_CurrentSelection.GetIndex(i);
		CmpackSelectionType type = m_CurrentSelection.GetType(i);
		gchar *buf = (gchar*)g_malloc((256+(tag ? strlen(tag)+1 : 0))*sizeof(gchar));
		if (index==1)
			strcpy(buf, Types[type].label);
		else
			sprintf(buf, "%s #%d", Types[type].label, index);
		if (tag) {
			strcat(buf, "\n");
			strcat(buf, tag);
		}
		cmpack_chart_data_set_tag(data, row, buf);
		cmpack_chart_data_set_color(data, row, Types[type].color);
		cmpack_chart_data_set_topmost(data, row, TRUE);
		g_free(buf);
	} else 
	if (tag) {
		// Not selected, with tag
		cmpack_chart_data_set_tag(data, row, tag);
		cmpack_chart_data_set_color(data, row, CMPACK_COLOR_YELLOW);
		cmpack_chart_data_set_topmost(data, row, TRUE);
	} else {
		// Not selected, no tag
		cmpack_chart_data_set_tag(data, row, NULL);
		cmpack_chart_data_set_color(data, row, CMPACK_COLOR_DEFAULT);
		cmpack_chart_data_set_topmost(data, row, FALSE);
	}
}

//
// Update displayed object
//
void CCatFileDlg::UpdateObject(GtkTreeModel *model, GtkTreeIter *iter) 
{
	int star_id;
	
	gtk_tree_model_get(model, iter, CCatalog::COL_ID, &star_id, -1);
	int i = m_CurrentSelection.IndexOf(star_id);
	if (i>=0) {
		// Selected object
		int index = m_CurrentSelection.GetIndex(i);
		CmpackSelectionType type = m_CurrentSelection.GetType(i);
		gchar buf[256];
		if (index==1)
			strcpy(buf, Types[type].label);
		else
			sprintf(buf, "%s #%d", Types[type].label, index);
		gulong typidx = ((gulong)type << 16) | (index & 0xFFFF);
		gtk_list_store_set(GTK_LIST_STORE(model), iter, CCatalog::COL_SEL_TYPIDX, typidx,
			CCatalog::COL_SEL_CAPTION, buf, CCatalog::COL_FG_COLOR, Types[type].color_name, -1);
	} else {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, CCatalog::COL_SEL_TYPIDX, 0,
			CCatalog::COL_SEL_CAPTION, NULL, CCatalog::COL_FG_COLOR, NULL, -1);
	}

	const gchar *tag = m_Tags.Get(star_id);
	gtk_list_store_set(GTK_LIST_STORE(model), iter, CCatalog::COL_TAGS, tag, -1);
}

// 
// Update selection and tags for all object
//
void CCatFileDlg::UpdateAll(void)
{
	if (m_DispMode == DISP_CHART) {
		if (m_ChartData) {
			int count = cmpack_chart_data_count(m_ChartData);
			for (int row=0; row<count; row++) 
				UpdateObject(m_ChartData, row);
		}
	} else {
		if (m_TableData) {
			GtkTreeIter iter;
			GtkTreePath *path = gtk_tree_path_new_first();
			while (gtk_tree_model_get_iter(m_TableData, &iter, path)) {
				UpdateObject(m_TableData, &iter);
				gtk_tree_path_next(path);
			}
			gtk_tree_path_free(path);
		}
	}
}

void CCatFileDlg::mouse_moved(GtkWidget *button, CCatFileDlg *pDlg)
{
	pDlg->m_UpdatePos = true;
}

void CCatFileDlg::zoom_changed(GtkWidget *button, CCatFileDlg *pDlg)
{
	pDlg->m_UpdateZoom = true;
}

gboolean CCatFileDlg::timer_cb(CCatFileDlg *pDlg)
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

void CCatFileDlg::InfoBoxCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CCatFileDlg *pMe = (CCatFileDlg*)cb_data;

	switch (message)
	{
	case CInfoBox::CB_CLOSED:
		pMe->OnInfoBoxClosed();
		break;
	}
}

void CCatFileDlg::OnInfoBoxClosed(void)
{
	m_InfoMode = INFO_NONE;
	UpdateStatus();
	UpdateControls();
}

void CCatFileDlg::UpdateStatus(void)
{
	char	buf[256];
	CmpackCatObject obj;
	int		x, y, star_id, index;

	if (m_DispMode==DISP_CHART && m_File) {
		int item = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(m_ChartView));
		if (item>=0 && m_ChartData) {
			if (m_LastFocus!=item) {
				m_LastFocus = item;
				star_id = (int)cmpack_chart_data_get_param(m_ChartData, item);
				index = m_File->FindObject(star_id);
				m_File->GetObject(index, CMPACK_OM_CENTER, &obj);
				sprintf(buf, "Object #%d: X = %.1f, Y = %.1f", 
					star_id, obj.center_x, obj.center_y);
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

void CCatFileDlg::UpdateZoom(void)
{
	if (m_DispMode==DISP_CHART) {
		double zoom, min, max;
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_ChartView));
		cmpack_chart_view_get_zoom_limits(CMPACK_CHART_VIEW(m_ChartView), &min, &max);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), zoom < max);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), zoom > min);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomFit), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomIn), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ZoomOut), FALSE);
	}
}

void CCatFileDlg::SetInfoMode(tInfoMode mode)
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

void CCatFileDlg::UpdateInfoBox(void)
{
	int		index;
	char	buf[256];
	CmpackCatObject obj;

	if (m_InfoMode == INFO_OBJECT && m_File) {
		m_InfoBox.BeginUpdate();
		m_InfoBox.Clear();
		// Show information about selected star
		if (m_SelectedObjId>=0) {
			sprintf(buf, "Object #%d", m_SelectedObjId);
			m_InfoBox.AddTitle(1, buf);
			// Position
			index = m_File->FindObject(m_SelectedObjId);
			m_File->GetObject(index, CMPACK_OM_CENTER | CMPACK_OM_MAGNITUDE, &obj);
			sprintf(buf, "Center(X) = %.1f", obj.center_x);
			m_InfoBox.AddText(buf);
			sprintf(buf, "Center(Y) = %.1f", obj.center_y);
			m_InfoBox.AddText(buf);
			// Brightness
			if (obj.refmag_valid) {
				sprintf(buf, "Brightness = %.4f mag", obj.refmagnitude);
				m_InfoBox.AddText(buf);
			}
			// Selection
			index = m_CurrentSelection.IndexOf(m_SelectedObjId);
			if (index>=0) {
				switch (m_CurrentSelection.GetType(index))
				{
				case CMPACK_SELECT_VAR:
					if (m_CurrentSelection.GetIndex(index)==1)
						sprintf(buf, "Variable star");
					else
						sprintf(buf, "Variable star #%d", m_CurrentSelection.GetIndex(index));
					m_InfoBox.AddText(buf);
					break;
				case CMPACK_SELECT_COMP:
					if (m_CurrentSelection.GetIndex(index)==1)
						sprintf(buf, "Comparison star");
					else
						sprintf(buf, "Comparison star #%d", m_CurrentSelection.GetIndex(index));
					m_InfoBox.AddText(buf);
					break;
				case CMPACK_SELECT_CHECK:
					sprintf(buf, "Check star #%d", m_CurrentSelection.GetIndex(index));
					m_InfoBox.AddText(buf);
					break;
				default:
					break;
				}
			}
			const gchar *tag = m_Tags.Get(m_SelectedObjId);
			if (tag)
				m_InfoBox.AddText(tag);
		} else {
			m_InfoBox.AddText("Click on the object");
			m_InfoBox.AddText("to show its properties");
		}
		m_InfoBox.EndUpdate();
	}
}

void CCatFileDlg::SetDisplayMode(tDispMode mode)
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
	g_Project->SetInt("CatFileDlg", "Mode", m_DispMode);
	UpdateChart();
	UpdateInfoBox();
	UpdateStatus();
	UpdateControls();
	UpdateZoom();
}

void CCatFileDlg::Export(void)
{
	if (m_DispMode == DISP_CHART) {
		CChartExportDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Execute(m_ChartData, NULL, m_Path, m_Negative, m_RowsUpward);
	} else {
		CExportCatFileDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Execute(*m_File, m_Tags, m_CurrentSelection, m_Path, m_SortColumnId, m_SortType);
	}
}

//
// Ask if there are unsaved data
//
int CCatFileDlg::ConfirmSelectionChanges(void)
{
	int res = GTK_RESPONSE_ACCEPT;
	GtkWidget *pDlg, *label, *image, *hbox;

	if (m_SelectionChanged) {
		pDlg = gtk_dialog_new_with_buttons(m_Name, GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
			GTK_STOCK_APPLY, GTK_RESPONSE_ACCEPT, GTK_STOCK_DISCARD, GTK_RESPONSE_REJECT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(pDlg), GTK_RESPONSE_YES, GTK_RESPONSE_NO,
			GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG(pDlg), GTK_RESPONSE_YES);
		hbox = gtk_hbox_new(FALSE, 12); 
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pDlg)->vbox), hbox, FALSE, FALSE, 0); 		
		image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
		gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);
		gtk_box_pack_start(GTK_BOX (hbox), image, FALSE, FALSE, 0);   
		label = gtk_label_new("The selection of stars has been changed.\nDo you want to apply or discard the changes?");
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_container_set_border_width(GTK_CONTAINER(pDlg), 5);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
		gtk_box_set_spacing (GTK_BOX (GTK_DIALOG(pDlg)->vbox), 14);
		gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(pDlg)->action_area), 5);
		gtk_box_set_spacing (GTK_BOX(GTK_DIALOG(pDlg)->action_area), 6); 
		gtk_widget_show_all(GTK_DIALOG(pDlg)->vbox);
		res = gtk_dialog_run(GTK_DIALOG(pDlg));
		gtk_widget_destroy(pDlg);
	}
	return res;
}

void CCatFileDlg::DettachSelection(void)
{
	m_NewSelection = m_CurrentSelection;
	m_SelectionIndex = -1;
	if (!m_ShowNewSelection) {
		m_ShowNewSelection = true;
		UpdateSelectionList();
	}
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Selections), &iter)) {
		m_Updating = true;
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_SelectCbx), &iter);
		m_Updating = false;
	}
}

void CCatFileDlg::combo_changed(GtkComboBox *widget, CCatFileDlg *pDlg)
{
	pDlg->OnComboChanged(widget);
}

void CCatFileDlg::OnComboChanged(GtkComboBox *widget)
{
	if (widget == GTK_COMBO_BOX(m_SelectCbx)) {
		if (!m_Updating) {
			int index = SelectedItem(widget);
			if (index!=m_SelectionIndex) {
				if (index<0) {
					// New selection
					m_CurrentSelection = m_NewSelection;
				} else {
					// Stored selection
					if (m_SelectionIndex<0)
						m_NewSelection = m_CurrentSelection;
					m_CurrentSelection = m_SelectionList.At(index);
				}
				m_SelectionIndex = index;
				UpdateAll();
				UpdateControls();
			}
		}
	}
}

void CCatFileDlg::table_column_clicked(GtkTreeViewColumn *treeviewcolumn, CCatFileDlg *pDlg)
{
	pDlg->OnTableColumnClicked(treeviewcolumn);
}

void CCatFileDlg::OnTableColumnClicked(GtkTreeViewColumn *pCol)
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

void CCatFileDlg::table_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, CCatFileDlg *pMe)
{
	pMe->OnTableRowActivated(treeview, path);
}

void CCatFileDlg::OnTableRowActivated(GtkTreeView *treeview, GtkTreePath *path)
{
	if (m_InfoMode != INFO_OBJECT)
		SetInfoMode(INFO_OBJECT);
	OnSelectionChanged();
}

bool CCatFileDlg::SaveFile(const gchar *fpath, GError **error)
{
	// Make catalog file
	CCatalog dst;
	dst.MakeCopy(*m_File);
	dst.RemoveAllSelections();
	for (int i=0; i<m_SelectionList.Count(); i++) 
		dst.AddSelection(m_SelectionList.Name(i), m_SelectionList.At(i));
	dst.SetTags(m_Tags);
	if (!dst.SaveAs(fpath, error))
		return false;

	// Copy FTS file
	gchar *src_fits = SetFileExtension(m_Path, FILE_EXTENSION_FITS);
	if (g_file_test(src_fits, G_FILE_TEST_IS_REGULAR)) {
		gchar *dst_fits = SetFileExtension(m_Path, FILE_EXTENSION_FITS);
		if (ComparePaths(src_fits, dst_fits)!=0) {
			GError *error = NULL;
			if (!copy_file(src_fits, dst_fits, false, &error)) {
				g_free(dst_fits);
				g_free(src_fits);
				return false;
			}
		}
		g_free(dst_fits);
	}
	g_free(src_fits);
	return true;
}

//---------------------------   INFO DIALOG   --------------------------------

//
// Constructor
//
CCatFileInfoDlg::CCatFileInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0),
	m_Name(NULL)
{
	GtkWidget *bbox;

	AddHeading(FIELD_FILE, 0, 0, "File");
	AddField(FIELD_FILENAME, 0, 1, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_DIRPATH, 0, 2, "Location", PANGO_ELLIPSIZE_MIDDLE);
	AddSeparator(0, 3);
	AddHeading(FIELD_FRAME, 0, 4, "Frame");
	AddField(FIELD_DATETIME, 0, 5, "Date and time (UTC)");
	AddField(FIELD_JULDAT, 0, 6, "Julian date");
	AddField(FIELD_FILTER, 0, 7, "Optical filter", PANGO_ELLIPSIZE_END);
	AddField(FIELD_EXPTIME, 0, 8, "Exposure duration");
	AddSeparator(0, 9);
	AddHeading(FIELD_PHOTOMETRY, 0, 10, "Photometry");
	AddField(FIELD_STARS, 0, 11, "Number of stars");
	gtk_widget_show_all(m_Tab);

	// Separator
	gtk_box_pack_start(GTK_BOX(m_Box), gtk_label_new(NULL), FALSE, TRUE, 0);

	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_box_pack_start(GTK_BOX(m_Box), bbox, FALSE, TRUE, 0);
	m_HdrBtn = gtk_button_new_with_label("Show header");
	gtk_box_pack_start(GTK_BOX(bbox), m_HdrBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_HdrBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

//
// Show dialog
//
void CCatFileInfoDlg::ShowModal(CCatalog *file, const gchar *name, const gchar *path)
{
	gchar buf[FILENAME_MAX+32];
	CmpackDateTime dt;

	m_File = file;
	m_Name = name;

	// Window caption
	sprintf(buf, "%s - %s - %s", m_Name, "properties", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Update properties
	gchar *basename = g_path_get_basename(path);
	SetField(FIELD_FILENAME, basename);
	g_free(basename);
	gchar *dirpath = g_path_get_dirname(path);
	SetField(FIELD_DIRPATH, dirpath);
	g_free(dirpath);

	if (file->DateTime(&dt)) {
		sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second);
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
	if (file->ExposureDuration()>0)
		SetField(FIELD_EXPTIME, file->ExposureDuration(), 3, "second(s)");
	else
		SetField(FIELD_EXPTIME, "Not available");

	if (file->ObjectCount()>=0)
		SetField(FIELD_STARS, file->ObjectCount());
	else
		SetField(FIELD_STARS, "Not available");

	CInfoDlg::ShowModal();
}

void CCatFileInfoDlg::button_clicked(GtkWidget *button, CCatFileInfoDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CCatFileInfoDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_HdrBtn))
		ShowHeader();
}

void CCatFileInfoDlg::ShowHeader(void)
{
	CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.Execute(m_File, m_Name);
}

//-------------------------   MAIN WINDOW   --------------------------------

CCatEditHeaderDlg::CCatEditHeaderDlg(GtkWindow *pParent)
{
	GtkWidget *hbox, *vbox, *tbox, *label, *scrwnd;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons(NULL, pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));

	// Dialog icon
	gchar *icon = get_icon_file("catalogfile");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	hbox = gtk_hbox_new(TRUE, 16);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), hbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);

	// Object + Location (left panel)
	tbox = gtk_table_new(6, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), tbox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 5, 12);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	label = gtk_label_new("Object - designation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_ObjName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_ObjName, "Object's designation, e.g. RT And");
	gtk_entry_set_max_length(GTK_ENTRY(m_ObjName), MAX_OBJECT_LEN);
	gtk_table_attach(GTK_TABLE(tbox), m_ObjName, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	gtk_table_attach(GTK_TABLE(tbox), m_ObjBtn, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("- right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_RA = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_RA), 64);
	gtk_table_attach(GTK_TABLE(tbox), m_RA, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	m_Dec = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_Dec), 64);
	gtk_table_attach(GTK_TABLE(tbox), m_Dec, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 2, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Observatory - name");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	m_LocName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_LocName, "Observer's location designation, e.g. Brno, Czech Republic");
	gtk_entry_set_max_length(GTK_ENTRY(m_LocName), MAX_LOCATION_LEN);
	gtk_table_attach(GTK_TABLE(tbox), m_LocName, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	m_LocBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_LocBtn, "Load observer's coordinates from presets");
	gtk_table_attach(GTK_TABLE(tbox), m_LocBtn, 2, 3, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("- longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	m_Lon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach(GTK_TABLE(tbox), m_Lon, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 2, 3, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	m_Lat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach(GTK_TABLE(tbox), m_Lat, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 2, 3, 5, 6, GTK_FILL, GTK_FILL, 0, 0);

	// Other fields (right panel)
	tbox = gtk_table_new(6, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), tbox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	label = gtk_label_new("Observer's name");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_Observer = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Observer, "Observer's name");
	gtk_entry_set_max_length(GTK_ENTRY(m_Observer), MAX_OBSERVER_LEN);
	gtk_table_attach(GTK_TABLE(tbox), m_Observer, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Telescope");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_Telescope = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Telescope, "Short description of the telescope");
	gtk_table_attach(GTK_TABLE(tbox), m_Telescope, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Instrument");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	m_Camera = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Camera, "Camera brand name and model");
	gtk_table_attach(GTK_TABLE(tbox), m_Camera, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Color filter");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	m_Filter = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Filter, "Color filter designation");
	gtk_table_attach(GTK_TABLE(tbox), m_Filter, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Field of view");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	m_FOV = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_FOV, "Size of view field, width x height");
	gtk_table_attach(GTK_TABLE(tbox), m_FOV, 1, 2,  4, 5, GTK_FILL, GTK_FILL, 0, 0);
	label = gtk_label_new("Orientation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	m_Orientation = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Orientation, "View field orientation, e.g. top=N, left=W");
	gtk_table_attach(GTK_TABLE(tbox), m_Orientation, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);

	// Notes (bottom)
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	label = gtk_label_new("Remarks");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_Notes = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_Notes), GTK_WRAP_WORD_CHAR);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Notes);
	gtk_widget_set_size_request(scrwnd, 640, 140);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, FALSE, TRUE, 0);

	// Signals
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CCatEditHeaderDlg::~CCatEditHeaderDlg()
{
	gtk_widget_destroy(m_pDlg);
}

bool CCatEditHeaderDlg::Execute(CCatalog &file, const gchar *name)
{
	gchar title[512];

	SetData(file);
	sprintf(title, "%s - %s - %s", name, "Edit header", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), title);
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		GetData(file);
		return true;
	}
	return false;
}

void CCatEditHeaderDlg::SetData(CCatalog &file)
{
	// Object
	const CObjectCoords *obj = file.Object();
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

	// Location
	const CLocation *loc = file.Location();
	if (loc->Name())
		gtk_entry_set_text(GTK_ENTRY(m_LocName), loc->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
	if (loc->Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), loc->Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (loc->Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), loc->Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");

	// Other parameters
	const gchar *observer = file.Observer();
	if (observer)
		gtk_entry_set_text(GTK_ENTRY(m_Observer), observer);
	else
		gtk_entry_set_text(GTK_ENTRY(m_Observer), "");
	const gchar *filter = file.Filter();
	if (filter)
		gtk_entry_set_text(GTK_ENTRY(m_Filter), filter);
	else
		gtk_entry_set_text(GTK_ENTRY(m_Filter), "");
	const gchar *telescope = file.Telescope();
	if (telescope)
		gtk_entry_set_text(GTK_ENTRY(m_Telescope), telescope);
	else
		gtk_entry_set_text(GTK_ENTRY(m_Telescope), "");
	const gchar *camera = file.Instrument();
	if (camera)
		gtk_entry_set_text(GTK_ENTRY(m_Camera), camera);
	else
		gtk_entry_set_text(GTK_ENTRY(m_Camera), "");
	const gchar *fov = file.FieldOfView();
	if (fov)
		gtk_entry_set_text(GTK_ENTRY(m_FOV), fov);
	else
		gtk_entry_set_text(GTK_ENTRY(m_FOV), "");
	const gchar *orientation = file.Orientation();
	if (orientation)
		gtk_entry_set_text(GTK_ENTRY(m_Orientation), orientation);
	else
		gtk_entry_set_text(GTK_ENTRY(m_Orientation), "");

	// Comments
	const gchar *notes = file.Notes();
	if (notes)
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes)), notes, -1);
	else
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes)), "", -1);
}

void CCatEditHeaderDlg::GetData(CCatalog &file) const
{
	char *notes;
	GtkTextBuffer *textbuf;
	GtkTextIter start, end;

	// Object
	CObjectCoords obj;

	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	obj.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	obj.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	obj.SetDec(g_strstrip(aux));
	g_free(aux);

	file.SetObject(obj);

	// Location
	CLocation loc;

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	loc.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	loc.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	loc.SetLat(g_strstrip(aux));
	g_free(aux);

	file.SetLocation(loc);

	// Other fields
	file.SetObserver(gtk_entry_get_text(GTK_ENTRY(m_Observer)));
	file.SetFilter(gtk_entry_get_text(GTK_ENTRY(m_Filter)));
	file.SetTelescope(gtk_entry_get_text(GTK_ENTRY(m_Telescope)));
	file.SetInstrument(gtk_entry_get_text(GTK_ENTRY(m_Camera)));
	file.SetFieldOfView(gtk_entry_get_text(GTK_ENTRY(m_FOV)));
	file.SetOrientation(gtk_entry_get_text(GTK_ENTRY(m_Orientation)));

	// Comments
	textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes));
	gtk_text_buffer_get_start_iter(textbuf, &start);
	gtk_text_buffer_get_end_iter(textbuf, &end);
	notes = gtk_text_buffer_get_text(textbuf, &start, &end, FALSE);
	file.SetNotes(notes);
	g_free(notes);
}

void CCatEditHeaderDlg::button_clicked(GtkWidget *pButton, CCatEditHeaderDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CCatEditHeaderDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
}

void CCatEditHeaderDlg::EditObjectCoords(void)
{
	CObjectCoords obj;
	CObjectDlg dlg(GTK_WINDOW(m_pDlg));

	obj.SetName(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	obj.SetRA(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	obj.SetDec(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	if (dlg.Execute(&obj)) {
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), obj.Name());
		gtk_entry_set_text(GTK_ENTRY(m_RA), obj.RA());
		gtk_entry_set_text(GTK_ENTRY(m_Dec), obj.Dec());
	}
}

void CCatEditHeaderDlg::EditLocation(void)
{
	CLocation obs;
	CLocationDlg dlg(GTK_WINDOW(m_pDlg));

	obs.SetName(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	obs.SetLon(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	obs.SetLat(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	if (dlg.Execute(&obs)) {
		gtk_entry_set_text(GTK_ENTRY(m_LocName), obs.Name());
		gtk_entry_set_text(GTK_ENTRY(m_Lon), obs.Lon());
		gtk_entry_set_text(GTK_ENTRY(m_Lat), obs.Lat());
	}
}

//----------------------   EXPORT TABLE   --------------------------

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
CExportCatFileDlg::CExportCatFileDlg(GtkWindow *pParent):m_pParent(pParent), 
	m_FileType(TYPE_CSV), m_Updating(false)
{
	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export catalogue file", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("catalogfile");
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
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CExportCatFileDlg::~CExportCatFileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
}

bool CExportCatFileDlg::Execute(const CCatalog &file, const CTags &tags, const CSelection &selection,
	const gchar *current_path, int sort_column_id, GtkSortType sort_type)
{
	m_File.MakeCopy(file);
	m_File.RemoveAllSelections();
	m_File.AddSelection("", selection);
	m_File.SelectSelection(0);
	m_File.SetTags(tags);

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
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("SaveCatFile", "CSV_SKIP_INVALID");
	m_Options[TYPE_CSV].header = g_Project->GetBool("SaveCatFile", "CSV_HEADER", true);

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	folder = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", folder);
	g_free(folder);

	// Save settings
	g_Project->SetBool("SaveCatFile", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("SaveCatFile", "CSV_HEADER", m_Options[TYPE_CSV].header);

	int res = 0, flags = 0;
	GError *error = NULL;
	const tOptions *opt = &m_Options[m_FileType];
	if (opt->skip_invalid)
		flags |= CCatalog::EXPORT_SKIP_INVALID;
	if (!opt->header)
		flags |= CCatalog::EXPORT_NO_HEADER;
	if (!m_File.ExportTable(filename, FileMimeType(m_FileType), flags, sort_column_id, sort_type, &error)) {
		if (error) {
			ShowError(m_pParent, error->message);
			g_error_free(error);
			error = NULL;
		}
		res = -1;
	}
	g_free(filename);
	return (res==0);
}

void CExportCatFileDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_Header, TRUE);
	gtk_widget_set_sensitive(m_SkipInvalid, TRUE);
}

void CExportCatFileDlg::button_toggled(GtkToggleButton *pWidget, CExportCatFileDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CExportCatFileDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CExportCatFileDlg::selection_changed(GtkComboBox *pWidget, CExportCatFileDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportCatFileDlg::OnSelectionChanged(GtkComboBox *pWidget)
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

void CExportCatFileDlg::OnTypeChanged(void)
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

	m_Updating = false;

	UpdateControls();
}

void CExportCatFileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CExportCatFileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExportCatFileDlg::OnResponseDialog(gint response_id)
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
