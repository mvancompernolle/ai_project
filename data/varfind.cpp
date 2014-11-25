/**************************************************************

varfind.cpp (C-Munipack project)
Widget for the VarFind dialog
Copyright (C) 2011 David Motl, dmotl@volny.cz

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

#include "varfind.h"
#include "configuration.h"
#include "configuration.h"
#include "export_dlgs.h"
#include "lightcurve_dlg.h"
#include "main.h"
#include "proc_classes.h"
#include "utils.h"
#include "ctxhelp.h"
#include "choosestars_dlg.h"

#define MIN_MAG_RANGE 0.05
#define MAX_MAG_RANGE 20.0

enum tCommandId
{
	CMD_HIDE = 100,
	CMD_DELETE,
	CMD_PREVIEW,
	CMD_FRAMEINFO
};

static const CPopupMenu::tPopupMenuItem LCGraphContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_PREVIEW,		"_Show frame" },
	{ CPopupMenu::MB_ITEM, CMD_FRAMEINFO,	"_Show properties" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_HIDE,		"_Delete frames from data set" },
	{ CPopupMenu::MB_ITEM, CMD_DELETE,		"_Remove frames from project" },
	{ CPopupMenu::MB_END }
};

static const CPopupMenu::tPopupMenuItem MDGraphContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_HIDE,		"_Delete objects from data set" },
	{ CPopupMenu::MB_END }
};

//---------------------------   FIND VARIABLES   --------------------------------

// 
// Default constructor
//
CVarFind::CVarFind(void):m_ChartData(NULL), m_ImageData(NULL), m_MDData(NULL), m_LCData(NULL), 
	m_Image(NULL), m_Phot(NULL), m_Catalog(NULL), m_ApertureIndex(-1), m_MDChannelX(-1), m_MDChannelY(-1), 
	m_LCChannelX(-1), m_LCChannelY(-1), m_Variable(-1), m_VarIndex(-1), m_VarIndex2(-1), m_Comparison(-1), 
	m_CompIndex(-1), m_SelectMode(SELECT_VARIABLE), m_Updating(false), m_UpdatePos(true), 
	m_Cancelled(false), m_SelectionList(NULL), m_SelectionIndex(-1), m_NewSelectionVar(-1), 
	m_NewSelectionComp(-1), m_ShowNewSelection(false), m_JDMin(0), m_JDMax(0),
	m_MagScaleMode(MAG_SCALE_VAR), m_FixedMagRange(0)
{
	GtkWidget *scrwnd;
	GtkTreeIter iter;

	m_DisplayMode = (tDisplayMode)g_Project->GetInt("VarFind", "Mode", DISPLAY_CHART, 0, DISPLAY_MIXED);
	m_DateFormat = (tDateFormat)g_Project->GetInt("Display", "DateFormat", JULIAN_DATE, 0, GREGORIAN_DATE);
	m_MagScaleMode = (tMagScaleMode)g_Project->GetInt("Display", "MagScale", MAG_SCALE_VAR, 0, MAG_SCALE_CUSTOM);
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog layout
	m_Box = gtk_table_new(2, 3, FALSE);
	gtk_widget_set_size_request(m_Box, 600, 400); 

	// Varfind-curve graph
	m_MDView = cmpack_graph_view_new();
	cmpack_graph_view_set_activation_mode(CMPACK_GRAPH_VIEW(m_MDView), CMPACK_ACTIVATION_CLICK);
	cmpack_graph_view_set_mouse_control(CMPACK_GRAPH_VIEW(m_MDView), TRUE);
	cmpack_graph_view_set_selection_mode(CMPACK_GRAPH_VIEW(m_MDView), GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(m_MDView), "button_press_event", G_CALLBACK(button_press_event), this);
	g_signal_connect(G_OBJECT(m_MDView), "item-activated", G_CALLBACK(item_activated), this);
	cmpack_graph_view_set_scales(CMPACK_GRAPH_VIEW(m_MDView), TRUE, TRUE);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_MDView);
	gtk_table_attach_defaults(GTK_TABLE(m_Box), scrwnd, 0, 1, 0, 1);

	// Chart
	m_Chart = cmpack_chart_view_new();
	cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_Chart), CMPACK_ACTIVATION_CLICK);
	cmpack_chart_view_set_mouse_control(CMPACK_CHART_VIEW(m_Chart), TRUE);
	g_signal_connect(G_OBJECT(m_Chart), "item-activated", G_CALLBACK(item_activated), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Chart);
	gtk_table_attach_defaults(GTK_TABLE(m_Box), scrwnd, 1, 2, 0, 1);

	// Light-curve graph
	m_LCView = cmpack_graph_view_new_with_model(NULL);
	cmpack_graph_view_set_mouse_control(CMPACK_GRAPH_VIEW(m_LCView), TRUE);
	cmpack_graph_view_set_scales(CMPACK_GRAPH_VIEW(m_LCView), TRUE, TRUE);
	cmpack_graph_view_set_selection_mode(CMPACK_GRAPH_VIEW(m_LCView), GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(m_LCView), "button_press_event", G_CALLBACK(button_press_event), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_LCView);
	gtk_table_attach_defaults(GTK_TABLE(m_Box), scrwnd, 0, 2, 1, 2);
	
	// Upper right toolbar
	GtkWidget *tbar = gtk_toolbar_new();
	gtk_toolbar_set_style (GTK_TOOLBAR (tbar), GTK_TOOLBAR_TEXT);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbar), GTK_ORIENTATION_VERTICAL);
	gtk_table_attach(GTK_TABLE(m_Box), tbar, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_set_size_request(tbar, 160, -1); 

	// Selection mode
	toolbar_new_label(tbar, "Object selection");
	m_SelectVar = toolbar_new_radio_button(tbar, NULL, "Change variable", "Switch to selection of a variable star");
	g_signal_connect(G_OBJECT(m_SelectVar), "toggled", G_CALLBACK(button_clicked), this);
	GtkWidget *bbox = toolbar_new_toolbar(tbar);
	m_GoBack = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_GoBack), "Select previous object");
	gtk_toolbar_insert(GTK_TOOLBAR(bbox), m_GoBack, -1);
	g_signal_connect(G_OBJECT(m_GoBack), "clicked", G_CALLBACK(button_clicked), this);
	m_GoForward = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_GoForward), "Select next object");
	gtk_toolbar_insert(GTK_TOOLBAR(bbox), m_GoForward, -1);
	g_signal_connect(G_OBJECT(m_GoForward), "clicked", G_CALLBACK(button_clicked), this);
	m_SelectComp = toolbar_new_radio_button(tbar, m_SelectVar, "Change comparison", "Switch to selection of a comparison star");
	g_signal_connect(G_OBJECT(m_SelectComp), "toggled", G_CALLBACK(button_clicked), this);
	toolbar_new_separator(tbar);

	// Saved selections
	toolbar_new_label(tbar, "Saved selections");
	m_SelectCbx = toolbar_new_combo(tbar, "Choose an item to restore recently used object selection");
	m_Selections = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), GTK_TREE_MODEL(m_Selections));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_SelectCbx), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_SelectCbx), renderer, "text", 1);
	g_signal_connect(G_OBJECT(m_SelectCbx), "changed", G_CALLBACK(combo_changed), this);
	m_SaveBtn = toolbar_new_button(tbar, "Save as...", "Save the current object selection");
	g_signal_connect(G_OBJECT(m_SaveBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_RemoveBtn = toolbar_new_button(tbar, "Remove", "Remove the current object selection from the list");
	g_signal_connect(G_OBJECT(m_RemoveBtn), "clicked", G_CALLBACK(button_clicked), this);
	toolbar_new_separator(tbar);

	// Aperture selection
	m_AperLabel = toolbar_new_label(tbar, "Aperture");
	m_AperCombo = toolbar_new_combo(tbar, "Aperture used to get brightness of the objects");
	g_signal_connect(G_OBJECT(m_AperCombo), "changed", G_CALLBACK(button_clicked), this);
	m_Apertures = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_AperCombo), renderer2, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_AperCombo), renderer2, "text", 1);
	toolbar_new_separator(tbar);

	// Save buttons
	m_SaveChart = gtk_tool_button_new(NULL, "Save chart");
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_SaveChart), "Save chart to a file");
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), m_SaveChart, -1);
	g_signal_connect(G_OBJECT(m_SaveChart), "clicked", G_CALLBACK(button_clicked), this);

	// Lower right toolbar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_style (GTK_TOOLBAR (tbar), GTK_TOOLBAR_TEXT);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbar), GTK_ORIENTATION_VERTICAL);
	gtk_table_attach(GTK_TABLE(m_Box), tbar, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	// X axis
	m_LCLabel = toolbar_new_label(tbar, "Light curve");
	m_XLabel = toolbar_new_label(tbar, "Date and time");
	m_XChannels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_list_store_append(m_XChannels, &iter);
	gtk_list_store_set(m_XChannels, &iter, 0, JULIAN_DATE, 1, "JD", -1);
	gtk_list_store_append(m_XChannels, &iter);
	gtk_list_store_set(m_XChannels, &iter, 0, GREGORIAN_DATE, 1, "UTC", -1);
	m_XCombo = toolbar_new_combo(tbar, "Column shown on the horizontal axis of the graph");
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_XCombo), GTK_TREE_MODEL(m_XChannels));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_XCombo), m_DateFormat);
	g_signal_connect(G_OBJECT(m_XCombo), "changed", G_CALLBACK(button_clicked), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_XCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_XCombo), renderer, "text", 1);

	// Y axis
	m_YLabel = toolbar_new_label(tbar, "Magnitude scale");
	m_YModes = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_list_store_append(m_YModes, &iter);
	gtk_list_store_set(m_YModes, &iter, 0, MAG_SCALE_VAR, 1, "Adaptive", -1);
	gtk_list_store_append(m_YModes, &iter);
	gtk_list_store_set(m_YModes, &iter, 0, MAG_SCALE_FIXED, 1, "Fixed", -1);
	gtk_list_store_append(m_YModes, &iter);
	gtk_list_store_set(m_YModes, &iter, 0, MAG_SCALE_CUSTOM, 1, "Custom", -1);
	m_YCombo = toolbar_new_combo(tbar, "Scaling mode of the light curves");
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_YCombo), GTK_TREE_MODEL(m_YModes));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_YCombo), m_MagScaleMode);
	g_signal_connect(G_OBJECT(m_YCombo), "changed", G_CALLBACK(button_clicked), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_YCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_YCombo), renderer, "text", 1);
	m_MagScale = toolbar_new_combo_box_entry(tbar, "Displayed range in magnitudes");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "0.05");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "0.1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "0.2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "0.5");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "1.0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "2.0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "5.0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_MagScale), "10.0");
	g_signal_connect(G_OBJECT(gtk_bin_get_child(GTK_BIN(m_MagScale))), "activate", G_CALLBACK(button_clicked), this);
	g_signal_connect(G_OBJECT(m_MagScale), "changed", G_CALLBACK(button_clicked), this);

	// Save buttons
	toolbar_new_separator(tbar);
	m_SaveCurve = gtk_tool_button_new(NULL, "Save light curve");
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_SaveCurve), "Save light curve to a file");
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), m_SaveCurve, -1);
	g_signal_connect(G_OBJECT(m_SaveCurve), "clicked", G_CALLBACK(button_clicked), this);

	// Popup menus
	m_LCGraphMenu.Create(LCGraphContextMenu);
	m_MDGraphMenu.Create(MDGraphContextMenu);
}

//
// Destructor
//
CVarFind::~CVarFind()
{
	// Disconnect graph signals
	g_signal_handlers_disconnect_by_func(G_OBJECT(m_LCView), (gpointer)button_press_event, this);

	// Release objects, free allocated memory
	if (m_MDData)
		g_object_unref(m_MDData);
	if (m_LCData)
		g_object_unref(m_LCData);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_ImageData)
		g_object_unref(m_ImageData);
	g_object_unref(m_Selections);
	g_object_unref(m_Apertures);
	g_object_unref(m_XChannels);
	g_object_unref(m_YModes);
}

//
// Button click handler
//
void CVarFind::button_clicked(GtkWidget *pButton, CVarFind *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CVarFind::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==m_XCombo) {
		int ch = SelectedItem(GTK_COMBO_BOX(m_XCombo));
		if (ch>=0 && ch!=m_DateFormat) {
			m_DateFormat = (tDateFormat)ch;
			g_Project->SetInt("Display", "DateFormat", m_DateFormat);
			UpdateLightCurve();
			UpdateControls();
		}
	} else
	if (pBtn==m_YCombo) {
		int mode = SelectedItem(GTK_COMBO_BOX(m_YCombo));
		if (mode>=0 && mode!=m_MagScaleMode) {
			m_MagScaleMode = (tMagScaleMode)mode;
			g_Project->SetInt("Display", "MagScale", m_MagScaleMode);
			UpdateLightCurve();
			UpdateControls();
		}
	} else
	if (pBtn==m_MagScale) {
		if (m_MagScaleMode == MAG_SCALE_CUSTOM && gtk_combo_box_get_active(GTK_COMBO_BOX(m_MagScale))>=0) {
			gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_MagScale));
			char *endptr;
			double x = strtod(text, &endptr);
			x = MIN(MAX_MAG_RANGE, MAX(x, MIN_MAG_RANGE));
			if (*endptr=='\0' && x!=m_CustomMagRange) {
				m_CustomMagRange = x;
				UpdateLightCurve();
			}
			g_free(text);
		}
	} else
	if (pBtn==gtk_bin_get_child(GTK_BIN(m_MagScale))) {
		// Use has entered a value into a edit field in the combo box
		// This is triggered by pressing the 'Enter' key while editing a value
		if (m_MagScaleMode == MAG_SCALE_CUSTOM) {
			gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_MagScale));
			char *endptr;
			double x = strtod(text, &endptr);
			x = MIN(MAX_MAG_RANGE, MAX(x, MIN_MAG_RANGE));
			if (*endptr=='\0' && x!=m_CustomMagRange) {
				m_CustomMagRange = x;
				UpdateLightCurve();
			}
			g_free(text);
		}
	} else
	if (pBtn==m_AperCombo) {
		int index = SelectedItem(GTK_COMBO_BOX(m_AperCombo));
		if (index>=0 && index!=m_ApertureIndex) {
			m_ApertureIndex = index;
			if (m_Phot)
				m_Phot->SelectAperture(m_ApertureIndex);
			Callback(CB_APERTURE_CHANGED);
			UpdateChart();
			UpdateControls();
		}
	} else
	if (pBtn==GTK_WIDGET(m_SelectComp)) {
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(m_SelectComp)))
			m_SelectMode = SELECT_COMPARISON;
	} else
	if (pBtn==GTK_WIDGET(m_SelectVar)) {
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(m_SelectVar)))
			m_SelectMode = SELECT_VARIABLE;
	} else
	if (pBtn==GTK_WIDGET(m_GoBack)) {
		if (m_MDData && m_VarIndex2>0) {
			int star_id = cmpack_graph_data_get_param(m_MDData, m_VarIndex2-1);
			SetVariableStar(star_id);
			Callback(CB_VARIABLE_CHANGED);
			UpdateControls();
		}
	} else
	if (pBtn==GTK_WIDGET(m_GoForward)) {
		if (m_MDData && m_VarIndex2<cmpack_graph_data_nrows(m_MDData)-1) {
			int star_id = cmpack_graph_data_get_param(m_MDData, m_VarIndex2+1);
			SetVariableStar(star_id);
			Callback(CB_VARIABLE_CHANGED);
			UpdateControls();
		}
	} else
	if (pBtn==GTK_WIDGET(m_SaveChart)) {
		if (m_ChartData && cmpack_chart_data_count(m_ChartData)>0)
			ExportChart();
	} else
	if (pBtn==GTK_WIDGET(m_SaveCurve)) {
		if (m_LCData && cmpack_graph_data_nrows(m_LCData)>0) 
			SaveLightCurve();
	} else
	if (pBtn==GTK_WIDGET(m_SaveBtn)) {
		SaveSelection();
	} else
	if (pBtn==GTK_WIDGET(m_RemoveBtn)) {
		RemoveSelection();
	}
}


//
// Get top level window
//
GtkWindow *CVarFind::GetTopLevel(void)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(m_Box);
	if (GTK_WIDGET_TOPLEVEL(toplevel) && GTK_IS_WINDOW(toplevel)) 
		return GTK_WINDOW(toplevel);
	return NULL;		
}

//
// Set selection list
//
void CVarFind::SetSelectionList(CSelectionList *list) 
{
	m_SelectionList = list;
	m_SelectionIndex = -1;
	m_ShowNewSelection = true;
	UpdateSelectionList();
	UpdateControls();
}

//
// Update list of selections
//
void CVarFind::UpdateSelectionList(void)
{
	m_Updating = true;

	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), NULL);
	gtk_list_store_clear(m_Selections);
	if (m_SelectionList) {
		if (m_ShowNewSelection) {
			GtkTreeIter iter;
			gtk_list_store_append(m_Selections, &iter);
			gtk_list_store_set(m_Selections, &iter, 0, -1, 1, "New selection", -1);
		}
		for (int i=0; i<m_SelectionList->Count(); i++) {
			GtkTreeIter iter;
			gtk_list_store_append(m_Selections, &iter);
			gtk_list_store_set(m_Selections, &iter, 0, i, 1, m_SelectionList->Name(i), -1);
		}
	} else {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, -1, 1, "New selection", -1);
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

void CVarFind::SetSelection(int index, int comp_id, int var_id)
{
	bool var_changed = false, comp_changed = false;
	if (comp_id!=m_Comparison && m_ChartData) {
		if (m_CompIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_CompIndex, NULL);
			cmpack_chart_data_set_color(m_ChartData, m_CompIndex, CMPACK_COLOR_DEFAULT);
			cmpack_chart_data_set_topmost(m_ChartData, m_CompIndex, FALSE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_CompIndex, 0);
		}
		m_Comparison = comp_id;
		m_CompIndex = cmpack_chart_data_find_item(m_ChartData, m_Comparison);
		if (m_CompIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_CompIndex, "comp");
			cmpack_chart_data_set_color(m_ChartData, m_CompIndex, CMPACK_COLOR_GREEN);
			cmpack_chart_data_set_topmost(m_ChartData, m_CompIndex, TRUE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_CompIndex, 4.0);
		}
		comp_changed = true;
	}		
	if (var_id!=m_Variable && m_ChartData && m_MDData) {
		if (m_VarIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_VarIndex, NULL);
			cmpack_chart_data_set_color(m_ChartData, m_VarIndex, CMPACK_COLOR_DEFAULT);
			cmpack_chart_data_set_topmost(m_ChartData, m_VarIndex, FALSE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_VarIndex, 0);
		}
		if (m_VarIndex2>=0) {
			cmpack_graph_data_set_tag(m_MDData, m_VarIndex2, NULL);
			cmpack_graph_data_set_color(m_MDData, m_VarIndex2, CMPACK_COLOR_DEFAULT);
			cmpack_graph_data_set_topmost(m_MDData, m_VarIndex2, FALSE);
		}
		m_Variable = var_id;
		m_VarIndex = cmpack_chart_data_find_item(m_ChartData, m_Variable);
		if (m_VarIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_VarIndex, "var");
			cmpack_chart_data_set_color(m_ChartData, m_VarIndex, CMPACK_COLOR_RED);
			cmpack_chart_data_set_topmost(m_ChartData, m_VarIndex, TRUE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_VarIndex, 4.0);
		}
		m_VarIndex2 = cmpack_graph_data_find_item(m_MDData, m_Variable);
		if (m_VarIndex2>=0) {
			cmpack_graph_data_set_tag(m_MDData, m_VarIndex2, "var");
			cmpack_graph_data_set_color(m_MDData, m_VarIndex2, CMPACK_COLOR_RED);
			cmpack_graph_data_set_topmost(m_MDData, m_VarIndex2, TRUE);
		}
		cmpack_graph_view_unselect_all(CMPACK_GRAPH_VIEW(m_MDView));
		var_changed = true;
	}
	m_SelectionIndex = index;
	if (comp_changed)
		Callback(CB_COMPARISON_CHANGED);
	else if (var_changed)
		Callback(CB_VARIABLE_CHANGED);
	UpdateControls();
}

void CVarFind::DettachSelection(void)
{
	GtkTreeIter iter;

	m_NewSelectionVar = m_Variable;
	m_NewSelectionComp = m_Comparison;
	SetSelection(-1, m_Comparison, m_Variable);
	if (!m_ShowNewSelection) {
		m_ShowNewSelection = true;
		UpdateSelectionList();
	}
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Selections), &iter)) {
		m_Updating = true;
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_SelectCbx), &iter);
		m_Updating = false;
	}
}

void CVarFind::SaveSelection(void)
{
	if (!m_SelectionList)
		return;

	const gchar *defValue = (m_SelectionIndex>=0 ? m_SelectionList->Name(m_SelectionIndex) : "");
	CTextQueryDlg dlg(GetTopLevel(), "Save selection as...");
	gchar *name = dlg.Execute("Enter name for the current selection:", 255, defValue, 
		(CTextQueryDlg::tValidator*)name_validator, this);
	if (name) {
		if (m_SelectionIndex<0) 
			m_ShowNewSelection = false;
		else
			m_SelectionList->RemoveAt(m_SelectionIndex);
		CSelection s;
		s.Select(m_Comparison, CMPACK_SELECT_COMP);
		s.Select(m_Variable, CMPACK_SELECT_VAR);
		m_SelectionList->Set(name, s);
		m_SelectionIndex = m_SelectionList->IndexOf(name);
		g_free(name);
		UpdateSelectionList();
		UpdateControls();
	}
}

bool CVarFind::name_validator(const gchar *name, GtkWindow *parent, CVarFind *pMe)
{
	return pMe->OnNameValidator(name, parent);
}

bool CVarFind::OnNameValidator(const gchar *name, GtkWindow *parent)
{
	if (!name || name[0]=='\0') {
		ShowError(parent, "Please, specify name of the selection.");
		return false;
	}
	if (m_SelectionList) {
		int i = m_SelectionList->IndexOf(name);
		if (i>=0 && (m_SelectionIndex<0 || i!=m_SelectionIndex))
			return ShowConfirmation(parent, "A selection with the specified name already exists.\nDo you want to overwrite it?");
	}
	return true;
}

void CVarFind::RemoveSelection(void)
{
	if (!m_SelectionList || m_SelectionIndex<0)
		return;

	m_SelectionList->RemoveAt(m_SelectionIndex);
	if (m_SelectionIndex>=m_SelectionList->Count())
		m_SelectionIndex = m_SelectionList->Count()-1;
	if (m_SelectionIndex<0) {
		m_ShowNewSelection = true;
		m_NewSelectionVar = m_NewSelectionComp = -1;
		SetSelection(-1, m_NewSelectionComp, m_NewSelectionVar);
	} else {
		CSelection s = m_SelectionList->At(m_SelectionIndex);
		int v = -1, c = -1;
		for (int i=0; i<s.Count(); i++) {
			if (s.GetType(i) == CMPACK_SELECT_VAR) {
				if (v<0)
					v = s.GetId(i);
			} else if (s.GetType(i) == CMPACK_SELECT_COMP) {
				if (c<0)
					c = s.GetId(i);
			}
		}
		SetSelection(m_SelectionIndex, c, v);
	}
	UpdateSelectionList();
	UpdateControls();
}

//
// Update list of apertures
//
void CVarFind::SetApertures(const CApertures &aper, int defaultApertureId)
{
	char txt[128];
	GtkTreeIter iter;

	m_Aper = aper;

	gtk_combo_box_set_model(GTK_COMBO_BOX(m_AperCombo), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_Aper.Count(); i++) {
		const CAperture *ap = m_Aper.Get(i);
		if (ap->Radius()>0)
			sprintf(txt, "#%d (%.2f)", ap->Id(), ap->Radius());
		else
			sprintf(txt, "#%d", ap->Id());
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
	
	UpdateControls();
}

void CVarFind::SetImage(CImage *img)
{
	m_Image = img;
	UpdateImage();
	UpdateControls();
}

void CVarFind::UpdateImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	if (m_DisplayMode != DISPLAY_CHART && m_Image) {
		m_ImageData = m_Image->ToImageData(m_Negative, false, true, m_RowsUpward);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	}
}

void CVarFind::SetPhotometryFile(CPhot *phot)
{
	m_Phot = phot;
	m_Catalog = NULL;
	UpdateChart();
	UpdateControls();
}

void CVarFind::SetCatalogFile(CCatalog *cat)
{
	m_Catalog = cat;
	m_Phot = NULL;
	UpdateChart();
	UpdateControls();
}

void CVarFind::UpdateChart(void)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	if (m_Phot) {
		m_ApertureIndex = m_Phot->FindAperture(ApertureId());
		m_Phot->SelectAperture(m_ApertureIndex);
		m_ChartData = m_Phot->ToChartData(false, m_DisplayMode==DISPLAY_IMAGE);
	} else
	if (m_Catalog) {
		m_ChartData = m_Catalog->ToChartData(false, false, m_DisplayMode==DISPLAY_IMAGE);
	}
	if (m_ChartData) {
		int count = cmpack_chart_data_count(m_ChartData);
		for (int i=0; i<count; i++) {
			CmpackChartItem *info = cmpack_chart_data_get_item(m_ChartData, i);
			if (!info->outline && !info->hidden) {
				gboolean retval = TRUE;
				Callback(CB_OBJECT_VALID_QUERY, (int)info->param, &retval);
				if (!retval)
					info->outline = TRUE;
			}
			info->disabled = info->outline;
		}
		m_CompIndex = cmpack_chart_data_find_item(m_ChartData, m_Comparison);
		if (m_CompIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_CompIndex, "comp");
			cmpack_chart_data_set_color(m_ChartData, m_CompIndex, CMPACK_COLOR_GREEN);
			cmpack_chart_data_set_topmost(m_ChartData, m_CompIndex, TRUE);
			if (m_DisplayMode!=DISPLAY_CHART)
				cmpack_chart_data_set_diameter(m_ChartData, m_CompIndex, 4.0);
		}
		m_VarIndex = cmpack_chart_data_find_item(m_ChartData, m_Variable);
		if (m_VarIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_VarIndex, "var");
			cmpack_chart_data_set_color(m_ChartData, m_VarIndex, CMPACK_COLOR_RED);
			cmpack_chart_data_set_topmost(m_ChartData, m_VarIndex, TRUE);
			if (m_DisplayMode!=DISPLAY_CHART)
				cmpack_chart_data_set_diameter(m_ChartData, m_VarIndex, 4.0);
		}
	}
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Chart), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Chart), m_Negative);
	if (m_DisplayMode != DISPLAY_CHART) 
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), m_ChartData);
}

void CVarFind::SetMagDev(const CTable &tab)
{
	m_MagDev.MakeCopy(tab);
	m_MDChannelX = m_MDChannelY = -1;
	UpdateMagDevCurve();
	UpdateControls();
}

void CVarFind::UpdateMagDevCurve()
{
	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_MDView), NULL);
	if (m_MDData) {
		g_object_unref(m_MDData);
		m_MDData = NULL;
	}
	if (m_MagDev.Valid()) {
		for (int i=0; i<m_MagDev.ChannelsX()->Count(); i++) {
			if (m_MagDev.ChannelsX()->GetInfo(i) == CChannel::DATA_MAGNITUDE) {
				m_MDChannelX = i;
				break;
			}
		}
		for (int j=0; j<m_MagDev.ChannelsY()->Count(); j++) {
			if (m_MagDev.ChannelsY()->GetInfo(j) == CChannel::DATA_DEVIATION) {
				m_MDChannelY = j;
				break;
			}
		}
		if (m_MDChannelX>=0 && m_MDChannelY>=0) {
			m_MagDev.SetView(CMPACK_GRAPH_VIEW(m_MDView), m_MDChannelX, m_MDChannelY, false, "mag", "dev");
			m_MDData = m_MagDev.ToGraphData(m_MDChannelX, m_MDChannelY);
			if (m_MDData) {
				m_VarIndex2 = cmpack_graph_data_find_item(m_MDData, m_Variable);
				if (m_VarIndex2>=0) {
					cmpack_graph_data_set_tag(m_MDData, m_VarIndex2, "var");
					cmpack_graph_data_set_color(m_MDData, m_VarIndex2, CMPACK_COLOR_RED);
					cmpack_graph_data_set_topmost(m_MDData, m_VarIndex2, TRUE);
				}
				cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_MDView), m_MDData);
				cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_MDView), true, true);
			}
		}
	} 
	UpdateControls();
}

void CVarFind::SetLightCurve(const CTable &tab)
{
	m_LCurve.MakeCopy(tab);
	m_LCChannelX = m_LCChannelY = -1;
	UpdateLightCurve();
	UpdateControls();
}

void CVarFind::SetFixedJDRange(double jdmin, double jdmax)
{
	m_JDMin = jdmin;
	m_JDMax = jdmax;
	UpdateLightCurve();
	UpdateControls();
}

void CVarFind::SetFixedMagRange(double magrange)
{
	m_FixedMagRange = magrange;
	UpdateLightCurve();
	UpdateControls();
}

void CVarFind::UpdateLightCurve()
{
	double ymin, ymax;
	char buf[256];

	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_LCView), NULL);
	if (m_LCData) {
		g_object_unref(m_LCData);
		m_LCData = NULL;
	}
	if (m_LCurve.Valid()) {
		for (int i=0; i<m_LCurve.ChannelsX()->Count(); i++) {
			if (m_LCurve.ChannelsX()->GetInfo(i) == CChannel::DATA_JD) {
				m_LCChannelX = i;
				break;
			}
		}
		for (int j=0; j<m_LCurve.ChannelsY()->Count(); j++) {
			if (m_LCurve.ChannelsY()->GetInfo(j) == CChannel::DATA_MAGNITUDE) {
				m_LCChannelY = j;
				break;
			}
		}
		if (m_LCChannelX>=0 && m_LCChannelY>=0) {
			m_LCurve.GetMinMaxY(m_LCChannelY, TRUE, ymin, ymax);
			double yavg = 0.5*(ymin+ymax);
			if (m_MagScaleMode == MAG_SCALE_FIXED) 
				m_CustomMagRange = 0.01 * ceil(100 * MIN(MAX_MAG_RANGE, MAX(m_FixedMagRange, MIN_MAG_RANGE)));
			else if (m_MagScaleMode == MAG_SCALE_VAR) 
				m_CustomMagRange = 0.01 * ceil(100 * MIN(MAX_MAG_RANGE, MAX(ymax-ymin, MIN_MAG_RANGE)));
			double yrange = 0.01 * ceil(100 * m_CustomMagRange);
			sprintf(buf, "%.2lf", yrange);
			gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_MagScale))), buf);
			m_LCurve.SetView(CMPACK_GRAPH_VIEW(m_LCView), m_LCChannelX, m_LCChannelY, TRUE, NULL, "V-C", 
				m_DateFormat, true, m_JDMin, m_JDMax, true, yavg-0.5*yrange, yavg+0.5*yrange);
			m_LCData = m_LCurve.ToGraphData(m_LCChannelX, m_LCChannelY);
			if (m_LCData) {
				cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_LCView), m_LCData);
				cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_LCView), TRUE, TRUE);
			}
		}
	} else 
		gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_MagScale))), "");
}

void CVarFind::SetComparisonStar(int star_id)
{
	if (star_id != m_Comparison && m_ChartData) {
		if (m_CompIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_CompIndex, NULL);
			cmpack_chart_data_set_color(m_ChartData, m_CompIndex, CMPACK_COLOR_DEFAULT);
			cmpack_chart_data_set_topmost(m_ChartData, m_CompIndex, FALSE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_CompIndex, 0);
		}
		DettachSelection();
		if (star_id == m_Variable)
			SetVariableStar(-1);
		m_Comparison = star_id;
		m_CompIndex = cmpack_chart_data_find_item(m_ChartData, star_id);
		if (m_CompIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_CompIndex, "comp");
			cmpack_chart_data_set_color(m_ChartData, m_CompIndex, CMPACK_COLOR_GREEN);
			cmpack_chart_data_set_topmost(m_ChartData, m_CompIndex, TRUE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_CompIndex, 4.0);
		}
	}
}

void CVarFind::SetVariableStar(int star_id)
{
	if (star_id != m_Variable && m_MDData && m_ChartData) {
		// Unselect current variable star 
		if (m_VarIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_VarIndex, NULL);
			cmpack_chart_data_set_color(m_ChartData, m_VarIndex, CMPACK_COLOR_DEFAULT);
			cmpack_chart_data_set_topmost(m_ChartData, m_VarIndex, FALSE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_VarIndex, 0);
		}
		if (m_VarIndex2>=0) {
			cmpack_graph_data_set_tag(m_MDData, m_VarIndex2, NULL);
			cmpack_graph_data_set_color(m_MDData, m_VarIndex2, CMPACK_COLOR_DEFAULT);
			cmpack_graph_data_set_topmost(m_MDData, m_VarIndex2, FALSE);
		}
		// Select new variable star 
		DettachSelection();
		m_Variable = star_id;
		m_VarIndex = cmpack_chart_data_find_item(m_ChartData, star_id);
		if (m_VarIndex>=0) {
			cmpack_chart_data_set_tag(m_ChartData, m_VarIndex, "var");
			cmpack_chart_data_set_color(m_ChartData, m_VarIndex, CMPACK_COLOR_RED);
			cmpack_chart_data_set_topmost(m_ChartData, m_VarIndex, TRUE);
			if (m_DisplayMode==DISPLAY_IMAGE)
				cmpack_chart_data_set_diameter(m_ChartData, m_VarIndex, 4.0);
		}
		m_VarIndex2 = cmpack_graph_data_find_item(m_MDData, m_Variable);
		if (m_VarIndex2>=0) {
			cmpack_graph_data_set_tag(m_MDData, m_VarIndex2, "var");
			cmpack_graph_data_set_color(m_MDData, m_VarIndex2, CMPACK_COLOR_RED);
			cmpack_graph_data_set_topmost(m_MDData, m_VarIndex2, TRUE);
		}
		cmpack_graph_view_unselect_all(CMPACK_GRAPH_VIEW(m_MDView));
	}
}

void CVarFind::UpdateControls(void)
{
	bool ok;
	int var_count;

	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_SelectVar), 
		(m_SelectMode == SELECT_VARIABLE));
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_SelectComp), 
		(m_SelectMode == SELECT_COMPARISON));

	if (m_SelectMode == SELECT_VARIABLE) {
		gtk_widget_set_sensitive(GTK_WIDGET(m_GoBack), m_VarIndex2>0);
		var_count = (m_MDData ? cmpack_graph_data_nrows(m_MDData) : 0);
		gtk_widget_set_sensitive(GTK_WIDGET(m_GoForward), m_VarIndex2>=0 && m_VarIndex2<var_count-1);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(m_GoBack), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(m_GoForward), FALSE);
	}

	ok = EnableCtrlQuery(ID_SAVE_CHART);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SaveChart), ok);

	ok = m_SelectionList!=NULL;
	gtk_widget_set_sensitive(m_SelectCbx, ok);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SaveBtn), ok && m_VarIndex>=0 && m_CompIndex>=0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_RemoveBtn), ok && m_SelectionIndex>=0);

	ok = m_LCurve.Valid();
	gtk_widget_set_sensitive(m_LCLabel, ok);
	gtk_widget_set_sensitive(m_XLabel, ok);
	gtk_widget_set_sensitive(m_XCombo, ok);
	gtk_widget_set_sensitive(m_YLabel, ok);
	gtk_widget_set_sensitive(m_YCombo, ok);
	gtk_widget_set_sensitive(m_MagScale, ok && m_MagScaleMode==MAG_SCALE_CUSTOM);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SaveCurve), ok);

	ok = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1;
	gtk_widget_set_sensitive(m_AperCombo, ok);
	gtk_widget_set_sensitive(m_AperLabel, ok);
}

bool CVarFind::EnableCtrlQuery(tControlId ctrl)
{
	bool ok = false;
	Callback(CB_ENABLE_CTRL_QUERY, ctrl, &ok);
	return ok;
}

void CVarFind::SetDisplayMode(tDisplayMode mode)
{
	m_DisplayMode = mode;
	g_Project->SetInt("VarFind", "Mode", m_DisplayMode);
	UpdateImage();
	UpdateChart();
}

//
// Left button click
//
void CVarFind::item_activated(GtkWidget *pWidget, gint item, CVarFind *pDlg)
{
	pDlg->OnItemActivated(pWidget, item);
}

void CVarFind::OnItemActivated(GtkWidget *pWidget, gint item)
{
	int id;

	if (pWidget==m_Chart && m_ChartData) {
		id = (int)cmpack_chart_data_get_param(m_ChartData, item);
		if (m_SelectMode == SELECT_VARIABLE) {
			if (id != m_Variable && id != m_Comparison) {
				SetVariableStar(id);
				Callback(CB_VARIABLE_CHANGED);
				UpdateControls();
			}
		} else {
			if (id != m_Comparison) {
				if (id == m_Variable)
					SetVariableStar(-1);
				SetComparisonStar(id);
				Callback(CB_COMPARISON_CHANGED);
				UpdateControls();
			}
		}
	} else
	if (pWidget == m_MDView && m_MDData) {
		id = (int)cmpack_graph_data_get_param(m_MDData, item);
		if (m_SelectMode == SELECT_VARIABLE) {
			if (id != m_Variable && id != m_Comparison) {
				SetVariableStar(id);
				Callback(CB_VARIABLE_CHANGED);
				UpdateControls();
			}
		} else {
			if (id != m_Comparison) {
				if (id == m_Variable)
					SetVariableStar(-1);
				SetComparisonStar(id);
				Callback(CB_COMPARISON_CHANGED);
				UpdateControls();
			}
		}
	}
}

//
// Save chart to a file
//
void CVarFind::ExportChart(void)
{
	if (m_ChartData) {
		CChartExportDlg dlg(GetTopLevel());
		if (m_DisplayMode==DISPLAY_CHART) 
			dlg.Execute(m_ChartData, NULL, "chart", m_Negative, m_RowsUpward);
		else
			dlg.Execute(m_ChartData, m_ImageData, "chart", m_Negative, m_RowsUpward);
	}
}

//
// Save light curve to file
//
void CVarFind::SaveLightCurve(void)
{
	if (m_LCurve.Valid()) {
		CSaveLightCurveDlg pDlg(GetTopLevel());
		pDlg.Execute(m_LCurve, m_LCChannelX, m_LCChannelY);
	}
}

//
// Save light curve as image
//
void CVarFind::ExportLightCurve(void)
{
	CGraphExportDlg pDlg(GetTopLevel());
	pDlg.Execute(CMPACK_GRAPH_VIEW(m_LCView), NULL);
}

//
// Save mag-dev curve to a file
//
void CVarFind::SaveMagDevCurve(void)
{
	if (m_MagDev.Valid()) {
		CExportMagDevCurveDlg dlg(GetTopLevel());
		dlg.Execute(m_MagDev);
	}
}

//
// Save mag-dev curve as image
//
void CVarFind::ExportMagDevCurve(void)
{
	CGraphExportDlg pDlg(GetTopLevel());
	pDlg.Execute(CMPACK_GRAPH_VIEW(m_MDView), NULL);
}

//
// Mouse button handler
//
gint CVarFind::button_press_event(GtkWidget *widget, GdkEventButton *event, CVarFind *pMe)
{
	int focused;

	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		if (widget==pMe->m_LCView || widget==pMe->m_MDView) {
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
void CVarFind::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int res, selected;

	if (widget==m_LCView) {
		selected = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_LCView));
		m_LCGraphMenu.Enable(CMD_DELETE, selected>0 && EnableCtrlQuery(ID_DELETE_FROM_PROJECT));
		m_LCGraphMenu.Enable(CMD_HIDE, selected>0 && EnableCtrlQuery(ID_REMOVE_FROM_DATASET));
		m_LCGraphMenu.Enable(CMD_FRAMEINFO, selected==1 && EnableCtrlQuery(ID_SHOW_FRAME_INFO));
		m_LCGraphMenu.Enable(CMD_PREVIEW, selected==1 && EnableCtrlQuery(ID_SHOW_FRAME_PREVIEW));
		res = m_LCGraphMenu.Execute(event);
		switch (res) 
		{
		case CMD_PREVIEW:
			// Show frame preview
			Callback(CB_SHOW_FRAME_PREVIEW);
			break;
		case CMD_FRAMEINFO:
			// Show frame properties
			Callback(CB_SHOW_FRAME_INFO);
			break;
		case CMD_HIDE:
			// Remove selected frames from data set
			Callback(CB_REMOVE_FRAMES_FROM_DATASET);
			break;
		case CMD_DELETE:
			// Delete selected frames from project
			Callback(CB_DELETE_FRAMES_FROM_PROJECT);
			break;
		}
	} else
	if (widget==m_MDView) {
		selected = cmpack_graph_view_get_selected_count(CMPACK_GRAPH_VIEW(m_MDView));
		m_LCGraphMenu.Enable(CMD_HIDE, selected>0 && EnableCtrlQuery(ID_REMOVE_FROM_DATASET));
		res = m_MDGraphMenu.Execute(event);
		switch (res) 
		{
		case CMD_HIDE:
			// Remove selected objects from data set
			Callback(CB_REMOVE_OBJECTS_FROM_DATASET);
			break;
		}
	}
}

//
// Get list of indices of selected rows
//
GList *CVarFind::GetSelectedFrames(void) const
{
	return cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_LCView));
}

//
// Get list of indices of selected rows
//
GList *CVarFind::GetSelectedObjects(void) const
{
	return cmpack_graph_view_get_selected_rows(CMPACK_GRAPH_VIEW(m_MDView));
}

//
// Get frame ID of first selected row
//
int CVarFind::SelectedFrameID(void) const
{
	int row = cmpack_graph_view_get_selected(CMPACK_GRAPH_VIEW(m_LCView));
	if (row>=0 && m_LCData) 
		return (gint)cmpack_graph_data_get_param(m_LCData, row);
	return -1;
}

void CVarFind::combo_changed(GtkComboBox *widget, CVarFind *pDlg)
{
	pDlg->OnComboChanged(widget);
}

void CVarFind::OnComboChanged(GtkComboBox *widget)
{
	if (widget == GTK_COMBO_BOX(m_SelectCbx)) {
		if (!m_Updating) {
			int index = SelectedItem(widget);
			if (index!=m_SelectionIndex) {
				if (index<0) {
					// New selection
					SetSelection(index, m_NewSelectionComp, m_NewSelectionVar);
				} else {
					// Stored selection
					if (m_SelectionIndex<0) {
						m_NewSelectionVar = m_Variable;
						m_NewSelectionComp = m_Comparison;
					}
					CSelection s = m_SelectionList->At(index);
					int v = -1, c = -1;
					for (int i=0; i<s.Count(); i++) {
						if (s.GetType(i) == CMPACK_SELECT_VAR) {
							if (v<0)
								v = s.GetId(i);
						} else if (s.GetType(i) == CMPACK_SELECT_COMP) {
							if (c<0)
								c = s.GetId(i);
						}
					}
					SetSelection(index, c, v);
				}
			}
		}
	}
}

//----------------   SAVE MAG-DEV CURVE DIALOG   --------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
} FileFormats[] = {
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
CExportMagDevCurveDlg::CExportMagDevCurveDlg(GtkWindow *pParent):m_pParent(pParent),
	m_Updating(false), m_FileType(TYPE_CSV), m_SelectedY(-1)
{
	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));
	
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Save mag-dev curve", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("varfind");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Options
	GtkWidget *frame = gtk_frame_new("Export options");
	GtkWidget *hbox = gtk_hbox_new(FALSE, 8);
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
	m_SkipInvalid = gtk_check_button_new_with_label("Discard rows with invalid values");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipInvalid), "toggled", G_CALLBACK(button_toggled), this);
	
	// Column selection
	GtkWidget *rbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), rbox, TRUE, TRUE, 0);
	m_Channels = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	label = gtk_label_new("Columns");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(rbox), label, FALSE, TRUE, 0);
	m_VCCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_Channels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_VCCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_VCCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(rbox), m_VCCombo, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_VCCombo), "changed", G_CALLBACK(selection_changed), this);
	m_AllValues = gtk_check_button_new_with_label("Export all columns");
	gtk_box_pack_start(GTK_BOX(rbox), m_AllValues, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_AllValues), "toggled", G_CALLBACK(button_toggled), this);
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CExportMagDevCurveDlg::~CExportMagDevCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
	if (m_Channels)
		g_object_unref(m_Channels);
}

CExportMagDevCurveDlg::tFileType CExportMagDevCurveDlg::StrToFileType(const gchar *str)
{
	if (str) {
		for (gint i=0; FileFormats[i].Id!=NULL; i++) {
			if (strcmp(FileFormats[i].Id, str)==0)
				return (tFileType)i;
		}
	}
	return TYPE_CSV;
}

const gchar *CExportMagDevCurveDlg::FileTypeToStr(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].Id;
	return "";
}

bool CExportMagDevCurveDlg::Execute(const CTable &table)
{
	m_Table.MakeCopy(table);
	m_SelectedY = -1;

	for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
		if (m_Table.ChannelsY()->GetInfo(i) == CChannel::DATA_DEVIATION) {
			m_SelectedY = i;
			break;
		}
	}

	gchar *aux = g_Project->GetStr("MagDevCurve", "FileType");
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
	gchar *filename = g_Project->GetStr("MagDevCurve", "File", "magdev.txt");
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
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("MagDevCurve", "CSV_SKIP_INVALID", true);
	m_Options[TYPE_CSV].header = g_Project->GetBool("MagDevCurve", "CSV_HEADER", true);
	m_Options[TYPE_CSV].all_values = g_Project->GetBool("MagDevCurve", "CSV_ALL_VALUES");
	m_Options[TYPE_TEXT].skip_invalid = g_Project->GetBool("MagDevCurve", "TEXT_SKIP_INVALID", true);
	m_Options[TYPE_TEXT].header = g_Project->GetBool("MagDevCurve", "TEXT_HEADER", true);
	m_Options[TYPE_TEXT].all_values = g_Project->GetBool("MagDevCurve", "TEXT_ALL_VALUES");

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
	g_Project->SetStr("MagDevCurve", "File", basename);
	g_free(basename);

	// Save settings
	g_Project->SetStr("MagDevCurve", "FileType", FileTypeToStr(m_FileType));
	g_Project->SetBool("MagDevCurve", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("MagDevCurve", "CSV_HEADER", m_Options[TYPE_CSV].header);
	g_Project->SetBool("MagDevCurve", "CSV_ALL_VALUES", m_Options[TYPE_CSV].all_values);
	g_Project->SetBool("MagDevCurve", "TEXT_SKIP_INVALID", m_Options[TYPE_TEXT].skip_invalid);
	g_Project->SetBool("MagDevCurve", "TEXT_HEADER", m_Options[TYPE_TEXT].header);
	g_Project->SetBool("MagDevCurve", "TEXT_ALL_VALUES", m_Options[TYPE_TEXT].all_values);

	const tOptions *opt = m_Options+m_FileType;

	CChannels *cx = m_Table.ChannelsX();
	for (int i=0; i<cx->Count(); i++) {
		CChannel *ch = m_Table.ChannelsX()->Get(i);
		switch (ch->Info())
		{
		case CChannel::DATA_OBJECT_ID:
		case CChannel::DATA_MAGNITUDE:
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
		case CChannel::DATA_DEVIATION:
		case CChannel::DATA_FREQUENCY:
			if (opt->all_values || i==m_SelectedY) 
				ch->SetExportFlags(0);
			else
				ch->SetExportFlags(CChannel::EXPORT_SKIP);
			break;
		default:
			ch->SetExportFlags(CChannel::EXPORT_SKIP);
		}
	}

	int res = 0;
	unsigned flags = 0;
	if (!opt->header)
		flags |= CTable::EXPORT_NO_HEADER;
	if (opt->skip_invalid)
		flags |= CTable::EXPORT_NULVAL_SKIP_ROW;
	GError *error = NULL;
	if (!m_Table.ExportTable(filename, FileMimeType(m_FileType), flags, &error)) {
		if (error) {
			ShowError(m_pParent, error->message);
			g_error_free(error);
		}
		res = -1;
	}

	g_free(filename);
	return (res==0);
}

void CExportMagDevCurveDlg::selection_changed(GtkComboBox *pWidget, CExportMagDevCurveDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportMagDevCurveDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft>=0 && ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
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

void CExportMagDevCurveDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_VCCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1 &&
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllValues)));
	gtk_widget_set_sensitive(m_AllValues, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1);
	gtk_widget_set_sensitive(m_Header, TRUE);
	gtk_widget_set_sensitive(m_SkipInvalid, TRUE);
}

void CExportMagDevCurveDlg::OnTypeChanged(void)
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
	for (int i=0; i<m_Table.ChannelsY()->Count(); i++) {
		CChannel *ch = m_Table.ChannelsY()->Get(i);
		if (ch && (ch->Info() == CChannel::DATA_DEVIATION || ch->Info() == CChannel::DATA_FREQUENCY)) {
			GtkTreeIter iter;
			gtk_list_store_append(m_Channels, &iter);
			gtk_list_store_set(m_Channels, &iter, 0, i, 1, ch->Name(), -1);
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
	OnSelectionChanged(GTK_COMBO_BOX(m_VCCombo));

	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllValues), opt->all_values);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);

	m_Updating = false;

	UpdateControls();
}

void CExportMagDevCurveDlg::button_toggled(GtkToggleButton *pWidget, CExportMagDevCurveDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CExportMagDevCurveDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_AllValues) {
		m_Options[m_FileType].all_values = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CExportMagDevCurveDlg::response_dialog(GtkWidget *pDlg, gint response_id, CExportMagDevCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExportMagDevCurveDlg::OnResponseDialog(gint response_id)
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
