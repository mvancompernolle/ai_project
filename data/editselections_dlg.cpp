/**************************************************************

stars_dlg.cpp (C-Munipack project)
The 'Choose stars' dialog
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
#include "editselections_dlg.h"
#include "main.h"
#include "utils.h"
#include "configuration.h"
#include "configuration.h"
#include "profile.h"
#include "ctxhelp.h"

//-------------------------   POPUP MENU   ---------------------------

enum tPopupCommand
{
	CMD_SET_VARIABLE,
	CMD_SET_COMPARISON,
	CMD_SET_CHECK,
	CMD_UNSET,
	CMD_NEW_SELECTION,
	CMD_SAVE_SELECTION,
	CMD_EDIT_TAG,
	CMD_REMOVE_TAG,
	CMD_CLEAR_TAGS
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
} Types[CMPACK_SELECT_COUNT] = {
	{ NULL, CMPACK_COLOR_DEFAULT },
	{ "var", CMPACK_COLOR_RED },
	{ "comp", CMPACK_COLOR_GREEN },
	{ "check", CMPACK_COLOR_BLUE }
};

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

//
// Constructor
//
CEditSelectionsDlg::CEditSelectionsDlg(GtkWindow *pParent, tEditMode mode):m_EditMode(mode), 
	m_pParent(pParent), m_SelectionList(NULL), m_ImageData(NULL), m_ChartData(NULL), 
	m_RefType(REF_UNDEFINED), m_SelectionIndex(-1), m_InstMagnitudes(false), 
	m_SingleComparison(false), m_ShowNewSelection(false), m_Updating(false)
{
	GtkWidget *tbox, *scrwnd;
	GdkRectangle rc;

	m_DisplayMode = (tDisplayMode)g_Project->GetInt("ChooseStarsDlg", "Mode", DISPLAY_IMAGE, 0, DISPLAY_FULL);
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Choose stars", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.9*rc.width), RoundToInt(0.8*rc.height));

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), tbox, FALSE, FALSE, 0);

	// View mode
	m_ViewLabel = toolbar_new_label(tbox, "View");
	m_ShowImage = toolbar_new_radio_button(tbox, NULL, "Image", "Display an image only");
	g_signal_connect(G_OBJECT(m_ShowImage), "toggled", G_CALLBACK(button_clicked), this);
	m_ShowChart = toolbar_new_radio_button(tbox, m_ShowImage, "Chart", "Display objects on a flat background");
	g_signal_connect(G_OBJECT(m_ShowChart), "toggled", G_CALLBACK(button_clicked), this);
	m_ShowMixed = toolbar_new_radio_button(tbox, m_ShowImage, "Mixed", "Display objects over an image");
	g_signal_connect(G_OBJECT(m_ShowMixed), "toggled", G_CALLBACK(button_clicked), this);
	toolbar_new_separator(tbox);

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

	// Chart
	m_Chart = cmpack_chart_view_new();
	cmpack_chart_view_set_mouse_control(CMPACK_CHART_VIEW(m_Chart), TRUE);
	cmpack_chart_view_set_activation_mode(CMPACK_CHART_VIEW(m_Chart), CMPACK_ACTIVATION_CLICK);
	g_signal_connect(G_OBJECT(m_Chart), "item-activated", G_CALLBACK(item_activated), this);
	g_signal_connect(G_OBJECT(m_Chart), "button_press_event", G_CALLBACK(button_press_event), this);
	gtk_widget_set_size_request(m_Chart, 320, 200);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Chart);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), scrwnd, TRUE, TRUE, 0);

	// Make popup menus
	m_SelectMenu.Create(SelectMenu);
	m_ContextMenu.Create(ContextMenu);

	// Signals
	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}


//
// Destructor
//
CEditSelectionsDlg::~CEditSelectionsDlg()
{
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_ImageData)
		g_object_unref(m_ImageData);
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_Selections);
}


//
// Left button click
//
void CEditSelectionsDlg::item_activated(GtkWidget *widget, gint item, CEditSelectionsDlg *pMe)
{
	GdkEventButton ev;
	ev.button = 1;
	ev.time = gtk_get_current_event_time();
	pMe->OnSelectMenu(&ev, item);
}

//
// Right mouse click
//
gint CEditSelectionsDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CEditSelectionsDlg *pMe)
{
	int focused;

	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		if (widget==pMe->m_Chart) {
			focused = cmpack_chart_view_get_focused(CMPACK_CHART_VIEW(widget));
			if (focused>=0) 
				pMe->OnSelectMenu(event, focused);
			else
				pMe->OnContextMenu(event);
		}
		return TRUE;
	}
	return FALSE;
}

//
// Object's context menu
//
void CEditSelectionsDlg::OnSelectMenu(GdkEventButton *event, gint row)
{
	int star_id, index;
	CmpackSelectionType type;

	if (!m_ChartData)
		return;

	star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
	index = m_Current.IndexOf(star_id);
	if (index>=0)
		type = m_Current.GetType(index);
	else
		type = CMPACK_SELECT_NONE;
	m_SelectMenu.Enable(CMD_SET_VARIABLE, type!=CMPACK_SELECT_VAR);
	m_SelectMenu.Enable(CMD_SET_COMPARISON, type!=CMPACK_SELECT_COMP);
	m_SelectMenu.Enable(CMD_SET_CHECK, type!=CMPACK_SELECT_CHECK);
	m_SelectMenu.Enable(CMD_UNSET, type!=CMPACK_SELECT_NONE);
	m_SelectMenu.Enable(CMD_NEW_SELECTION, m_SelectionIndex>=0 || m_Current.Count()>0);
	m_SelectMenu.Enable(CMD_SAVE_SELECTION, m_Current.Count()>0);
	m_SelectMenu.Enable(CMD_REMOVE_TAG, m_Tags && m_Tags->Get(star_id)!=0);
	m_SelectMenu.Enable(CMD_CLEAR_TAGS, m_Tags && m_Tags->Count()>0);
	switch (m_SelectMenu.Execute(event))
	{
	case CMD_SET_VARIABLE:
		if (!m_InstMagnitudes) 
			UnselectType(CMPACK_SELECT_VAR);
		Select(row, star_id, CMPACK_SELECT_VAR);
		break;
	case CMD_SET_COMPARISON:
		if (m_SingleComparison && !m_InstMagnitudes)
			UnselectType(CMPACK_SELECT_COMP);
		Select(row, star_id, CMPACK_SELECT_COMP);
		break;
	case CMD_SET_CHECK:
		Select(row, star_id, CMPACK_SELECT_CHECK);
		break;
	case CMD_UNSET:
		Unselect(row, star_id);
		break;
	case CMD_NEW_SELECTION:
		NewSelection();
		break;
	case CMD_SAVE_SELECTION:
		SaveSelection();
		break;
	case CMD_EDIT_TAG:
		EditTag(row, star_id);
		break;
	case CMD_REMOVE_TAG:
		RemoveTag(row, star_id);
		break;
	case CMD_CLEAR_TAGS:
		ClearTags();
		break;
	}
	UpdateControls();
}

//
// Context menu (no object focused)
//
void CEditSelectionsDlg::OnContextMenu(GdkEventButton *event)
{
	m_SelectMenu.Enable(CMD_NEW_SELECTION, m_SelectionIndex>=0 || m_Current.Count()>0);
	m_SelectMenu.Enable(CMD_SAVE_SELECTION, m_Current.Count()>0);
	m_ContextMenu.Enable(CMD_CLEAR_TAGS, m_Tags && m_Tags->Count()>0);
	switch (m_ContextMenu.Execute(event))
	{
	case CMD_NEW_SELECTION:
		NewSelection();
		break;
	case CMD_SAVE_SELECTION:
		SaveSelection();
		break;
	case CMD_CLEAR_TAGS:
		ClearTags();
		break;
	}
	UpdateControls();
}

void CEditSelectionsDlg::Select(int row, int star_id, CmpackSelectionType type)
{
	if (row>=0 && star_id>=0) {
		DettachSelection();
		m_Current.Select(star_id, type);
		UpdateObject(row, star_id);
	}
}

void CEditSelectionsDlg::Unselect(int row, int star_id)
{
	if (row>=0 && star_id>=0) {
		DettachSelection();
		m_Current.Select(star_id, CMPACK_SELECT_NONE);
		UpdateObject(row, star_id);
	}
}

void CEditSelectionsDlg::UnselectType(CmpackSelectionType type)
{
	int count = m_Current.CountStars(type);
	if (count>0 && m_ChartData) {
		DettachSelection();
		int *stars = (int*)g_malloc(count*sizeof(int));
		m_Current.GetStarList(type, stars, count);
		for (int i=0; i<count; i++) {
			int row = cmpack_chart_data_find_item(m_ChartData, stars[i]);
			Unselect(row, stars[i]);
		}
		g_free(stars);
	}
}

void CEditSelectionsDlg::NewSelection(void)
{
	DettachSelection();
	m_NewSelection = CSelection();
	m_Current = m_NewSelection;
	m_SelectionIndex = -1;
	UpdateAll();
	UpdateControls();
}

void CEditSelectionsDlg::SaveSelection(void)
{
	const gchar *defValue = (m_SelectionIndex>=0 ? m_SelectionList->Name(m_SelectionIndex) : "");
	CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Save selection as...");
	gchar *name = dlg.Execute("Enter name for the current selection:", 255, defValue, 
		(CTextQueryDlg::tValidator*)name_validator, this);
	if (name) {
		if (m_SelectionIndex<0) 
			m_ShowNewSelection = false;
		else
			m_SelectionList->RemoveAt(m_SelectionIndex);
		m_SelectionList->Set(name, m_Current);
		m_SelectionIndex = m_SelectionList->IndexOf(name);
		g_free(name);
		UpdateSelectionList();
		UpdateControls();
	}
}

bool CEditSelectionsDlg::name_validator(const gchar *name, GtkWindow *parent, CEditSelectionsDlg *pMe)
{
	return pMe->OnNameValidator(name, parent);
}

bool CEditSelectionsDlg::OnNameValidator(const gchar *name, GtkWindow *parent)
{
	if (!name || name[0]=='\0') {
		ShowError(parent, "Please, specify name of the selection.");
		return false;
	}
	int i = m_SelectionList->IndexOf(name);
	if (i>=0 && (m_SelectionIndex<0 || i!=m_SelectionIndex))
		return ShowConfirmation(parent, "A selection with the specified name already exists.\nDo you want to overwrite it?");
	return true;
}

void CEditSelectionsDlg::RemoveSelection(void)
{
	if (m_SelectionIndex<0) {
		if (m_SelectionList->Count()>0) {
			m_ShowNewSelection = false;
			m_SelectionIndex = 0;
			m_Current = m_SelectionList->At(m_SelectionIndex);
			m_SelectionIndex = m_SelectionIndex;
		} else {
			m_ShowNewSelection = true;
			m_NewSelection = CSelection();
			m_Current = m_NewSelection;
			m_SelectionIndex = -1;
		}
	} else {
		m_SelectionList->RemoveAt(m_SelectionIndex);
		if (m_SelectionList->Count()>0) {
			if (m_SelectionIndex>=m_SelectionList->Count())
				m_SelectionIndex = m_SelectionList->Count()-1;
			m_Current = m_SelectionList->At(m_SelectionIndex);
			m_SelectionIndex = m_SelectionIndex;
		} else {
			m_ShowNewSelection = true;
			m_NewSelection = CSelection();
			m_Current = m_NewSelection;
			m_SelectionIndex = -1;
		}
	}
	UpdateAll();
	UpdateSelectionList();
	UpdateControls();
}

void CEditSelectionsDlg::EditTag(int row, int star_id)
{
	char obj[256], buf[256];

	if (row>=0 && star_id>=0 && m_Tags) {
		CTextQueryDlg dlg(GTK_WINDOW(m_pDlg), "Edit tag");
		int i = m_Current.IndexOf(star_id);
		if (i>=0) {
			int index = m_Current.GetIndex(i);
			CmpackSelectionType type = m_Current.GetType(i);
			if (index==1)
				strcpy(obj, Types[type].label);
			else
				sprintf(obj, "%s #%d", Types[type].label, index);
		} else 
			sprintf(obj, "object #%d", star_id);
		sprintf(buf, "Enter caption for '%s':", obj);
		gchar *value = dlg.Execute(buf, MAX_TAG_SIZE, m_Tags->Get(star_id),
			(CTextQueryDlg::tValidator*)tag_validator, this);
		if (value) {
			SetTag(row, star_id, value);
			g_free(value);
		}
	}
}

bool CEditSelectionsDlg::tag_validator(const gchar *name, GtkWindow *parent, CEditSelectionsDlg *pMe)
{
	return pMe->OnTagValidator(name, parent);
}

bool CEditSelectionsDlg::OnTagValidator(const gchar *name, GtkWindow *parent)
{
	if (!name || name[0]=='\0') {
		ShowError(parent, "Please, specify caption for the new tag.");
		return false;
	}
	return true;
}

void CEditSelectionsDlg::SetTag(int row, int star_id, const gchar *value)
{
	if (row>=0 && star_id>=0 && m_Tags) {
		if (value && value[0]!='\0') 
			m_Tags->Set(star_id, value);
		else 
			m_Tags->Remove(star_id);
		UpdateObject(row, star_id);
	}
}

void CEditSelectionsDlg::RemoveTag(int row, int star_id)
{
	if (row>=0 && m_Tags && m_Tags->Get(star_id)) {
		m_Tags->Remove(star_id);
		UpdateObject(row, star_id);
	}
}

void CEditSelectionsDlg::ClearTags(void)
{
	if (m_Tags) {
		m_Tags->Clear();
		UpdateAll();
		UpdateControls();
	}
}

void CEditSelectionsDlg::UpdateSelectionList(void)
{
	m_Updating = true;

	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), NULL);
	gtk_list_store_clear(m_Selections);
	if (m_ShowNewSelection) {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, -1, 1, "New selection", -1);
	}
	int defIndex = m_SelectionList->IndexOf("");
	if (defIndex>=0) {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, defIndex, 1, "Default selection", -1);
	}
	for (int i=0; i<m_SelectionList->Count(); i++) {
		if (i!=defIndex) {
			GtkTreeIter iter;
			gtk_list_store_append(m_Selections, &iter);
			gtk_list_store_set(m_Selections, &iter, 0, i, 1, m_SelectionList->Name(i), -1);
		}
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

void CEditSelectionsDlg::DettachSelection(void)
{
	m_NewSelection = m_Current;
	m_Current = m_NewSelection;
	m_SelectionIndex = -1;
	UpdateAll();
	UpdateControls();
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

void CEditSelectionsDlg::button_clicked(GtkWidget *pButton, CEditSelectionsDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CEditSelectionsDlg::OnButtonClicked(GtkWidget *pBtn)
{
	double zoom;

	if (pBtn==GTK_WIDGET(m_ShowChart)) {
		m_DisplayMode = DISPLAY_CHART;
		g_Project->SetInt("ChooseStarsDlg", "Mode", m_DisplayMode);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
		UpdateImage();
		UpdateChart();
	} else
	if (pBtn==GTK_WIDGET(m_ShowImage)) {
		m_DisplayMode = DISPLAY_IMAGE;
		g_Project->SetInt("ChooseStarsDlg", "Mode", m_DisplayMode);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
		UpdateImage();
		UpdateChart();
	} else
	if (pBtn==GTK_WIDGET(m_ShowMixed)) {
		m_DisplayMode = DISPLAY_FULL;
		g_Project->SetInt("ChooseStarsDlg", "Mode", m_DisplayMode);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
		UpdateImage();
		UpdateChart();
	} else
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
	} else
	if (pBtn==GTK_WIDGET(m_ClearBtn)) {
		NewSelection();
	} else
	if (pBtn==GTK_WIDGET(m_SaveBtn)) {
		SaveSelection();
	} else
	if (pBtn==GTK_WIDGET(m_RemoveBtn)) {
		RemoveSelection();
	}
}

void CEditSelectionsDlg::UpdateImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	if (m_DisplayMode != DISPLAY_CHART) {
		m_ImageData = m_Image.ToImageData(m_Negative, false, false, m_RowsUpward);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	}
}

//
// Update chart
//
void CEditSelectionsDlg::UpdateChart(void)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_RefType==REF_FRAME)
		m_ChartData = m_Phot.ToChartData(false, m_DisplayMode==DISPLAY_IMAGE);
	else
		m_ChartData = m_Catalog.ToChartData(false, false, m_DisplayMode==DISPLAY_IMAGE);
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Chart), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Chart), m_Negative);
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), m_ChartData);
	UpdateAll();
}


//
// Enable/disable controls
//
void CEditSelectionsDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_SelectCbx, TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ClearBtn), m_SelectionIndex>=0 || m_Current.Count()>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SaveBtn), m_Current.Count()>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_RemoveBtn), TRUE);
}

//
// Update displayed object
//
void CEditSelectionsDlg::UpdateObject(int row, int star_id) 
{
	int i = m_Current.IndexOf(star_id);
	const gchar *tag = (m_Tags ? m_Tags->Get(star_id) : NULL);
	if (i>=0 && m_ChartData) {
		// Selected object
		int index = m_Current.GetIndex(i);
		CmpackSelectionType type = m_Current.GetType(i);
		gchar *buf = (gchar*)g_malloc((256+(tag ? strlen(tag)+1 : 0))*sizeof(gchar));
		if (index==1)
			strcpy(buf, Types[type].label);
		else
			sprintf(buf, "%s #%d", Types[type].label, index);
		if (tag) {
			strcat(buf, "\n");
			strcat(buf, tag);
		}
		cmpack_chart_data_set_tag(m_ChartData, row, buf);
		cmpack_chart_data_set_color(m_ChartData, row, Types[type].color);
		cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
		if (m_DisplayMode==DISPLAY_IMAGE)
			cmpack_chart_data_set_diameter(m_ChartData, row, 4.0);
		g_free(buf);
	} else 
	if (tag && m_ChartData) {
		// Not selected, with tag
		cmpack_chart_data_set_tag(m_ChartData, row, tag);
		cmpack_chart_data_set_color(m_ChartData, row, CMPACK_COLOR_YELLOW);
		cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
		if (m_DisplayMode==DISPLAY_IMAGE)
			cmpack_chart_data_set_diameter(m_ChartData, row, 4.0);
	} else 
	if (m_ChartData) {
		// Not selected, no tag
		cmpack_chart_data_set_tag(m_ChartData, row, NULL);
		cmpack_chart_data_set_color(m_ChartData, row, CMPACK_COLOR_DEFAULT);
		cmpack_chart_data_set_topmost(m_ChartData, row, FALSE);
		if (m_DisplayMode==DISPLAY_IMAGE)
			cmpack_chart_data_set_diameter(m_ChartData, row, 0);
	}
}

// 
// Update selection and tags for all object
//
void CEditSelectionsDlg::UpdateAll(void)
{
	if (m_ChartData) {
		int count = cmpack_chart_data_count(m_ChartData);
		for (int row=0; row<count; row++) 
			UpdateObject(row, cmpack_chart_data_get_param(m_ChartData, row));
	}
}

void CEditSelectionsDlg::response_dialog(GtkDialog *pDlg, gint response_id, CEditSelectionsDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CEditSelectionsDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Check input
		if (!OnCloseQuery())
			return false;
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CHOOSE_STARS);
		return false;
	}
	return true;
}

void CEditSelectionsDlg::combo_changed(GtkComboBox *widget, CEditSelectionsDlg *pDlg)
{
	pDlg->OnComboChanged(widget);
}

void CEditSelectionsDlg::OnComboChanged(GtkComboBox *widget)
{
	if (widget == GTK_COMBO_BOX(m_SelectCbx)) {
		if (!m_Updating) {
			int index = SelectedItem(widget);
			if (index!=m_SelectionIndex) {
				if (index<0) {
					// New selection
					m_Current = m_NewSelection;
				} else {
					// Stored selection
					if (m_SelectionIndex<0)
						m_NewSelection = m_Current;
					m_Current = m_SelectionList->At(index);
				}
				m_SelectionIndex = index;
				UpdateAll();
				UpdateControls();
			}
		}
	}
}

void CEditSelectionsDlg::ShowSelection(int index) 
{
	if (index<0) {
		// New selection
		if (!m_ShowNewSelection) {
			m_ShowNewSelection = true;
			m_NewSelection = CSelection();
		}
		m_Current = m_NewSelection;
		m_SelectionIndex = -1;
	} else {
		// Stored selection
		if (m_SelectionIndex<0)
			m_NewSelection = m_Current;
		m_Current = m_SelectionList->At(index);
		m_SelectionIndex = index;
	}
	UpdateSelectionList();
	UpdateAll();
	UpdateControls();
}

//-------------------------   TEXT QUERY DIALOG   ----------------------------

CTextQueryDlg::CTextQueryDlg(GtkWindow *pParent, const gchar *caption):m_ValidatorProc(NULL),
	m_ValidatorData(0)
{
	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons(caption, pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_size_request(vbox, 360, -1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	m_Query = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Query), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Query, TRUE, TRUE, 0);
	m_Entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), m_Entry, TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CTextQueryDlg::~CTextQueryDlg()
{
	gtk_widget_destroy(m_pDlg);
}

gchar *CTextQueryDlg::Execute(const gchar *query, int maxsize, const gchar *defval,
	CTextQueryDlg::tValidator validator_proc, gpointer validator_data)
{
	m_ValidatorProc = validator_proc;
	m_ValidatorData = validator_data;
	gtk_label_set_text(GTK_LABEL(m_Query), query);
	gtk_entry_set_text(GTK_ENTRY(m_Entry), (defval ? defval : ""));
	gtk_entry_set_max_length(GTK_ENTRY(m_Entry), maxsize);
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT)
		return g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Entry)));
	return NULL;	
}

void CTextQueryDlg::response_dialog(GtkDialog *pDlg, gint response_id, CTextQueryDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CTextQueryDlg::OnResponseDialog(gint response_id)
{
	if (response_id == GTK_RESPONSE_ACCEPT) 
		return OnCloseQuery();
	return true;
}

bool CTextQueryDlg::OnCloseQuery()
{
	if (m_ValidatorProc) {
		return m_ValidatorProc(gtk_entry_get_text(GTK_ENTRY(m_Entry)), GTK_WINDOW(m_pDlg),
			m_ValidatorData);
	}
	return true;
}
