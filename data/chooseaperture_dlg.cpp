/**************************************************************

aperture_dlg.cpp (C-Munipack project)
Aperture selection dialog
Copyright (C) 2009 David Motl, dmotl@volny.cz

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
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "configuration.h"
#include "ctxhelp.h"
#include "chooseaperture_dlg.h"

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CChooseApertureDlg::CChooseApertureDlg(GtkWindow *pParent):m_FrameSet(NULL), m_Table(NULL),
	m_GraphData(NULL), m_ApertureIndex(-1), m_Row(-1), m_Column(-1)
{
	GdkRectangle rc;
	GtkWidget *hbox, *vbox, *scrwnd;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Choose aperture", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.6*rc.width), RoundToInt(0.5*rc.height));

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Preview box
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), hbox, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);

	// List of apertures
	m_Apertures = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	m_AperView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_Apertures));
	gtk_widget_set_tooltip_text(m_AperView, "Click on a item to select an aperture");
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Apertures");
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_AperView), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_AperView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_AperView);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, FALSE, TRUE, 0);
	gtk_widget_set_size_request(scrwnd, 140, 300);

	// List of data columns
	m_Channels = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	m_DataView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_Channels));
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Data sets");
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_DataView), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_DataView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_DataView);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scrwnd, -1, 120);

	// Graph
	m_GraphView = cmpack_graph_view_new_with_model(NULL);
	gtk_widget_set_tooltip_text(m_GraphView, "Click on a point to select an aperture");
	cmpack_graph_view_set_mouse_control(CMPACK_GRAPH_VIEW(m_GraphView), FALSE);
	cmpack_graph_view_set_scales(CMPACK_GRAPH_VIEW(m_GraphView), TRUE, TRUE);
	cmpack_graph_view_set_activation_mode(CMPACK_GRAPH_VIEW(m_GraphView), CMPACK_ACTIVATION_CLICK);
	g_signal_connect(G_OBJECT(m_GraphView), "item-activated", G_CALLBACK(item_activated), this);
	gtk_widget_set_size_request(m_GraphView, 200, -1);
	gtk_box_pack_start(GTK_BOX(hbox), m_GraphView, TRUE, TRUE, 0);

	gtk_widget_show_all(hbox);
}


//
// Destructor
//
CChooseApertureDlg::~CChooseApertureDlg()
{
	g_object_unref(m_Apertures);
	g_object_unref(m_Channels);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	gtk_widget_destroy(m_pDlg);
	delete m_Table;
}


// 
// Change aperture stored in project
//
bool CChooseApertureDlg::Execute(CFrameSet &fset, const CApertures &aper, const CSelection &sel, int *ap_index)
{
	m_FrameSet = &fset;
	m_Aper = aper;
	m_Selection = sel;
	m_ApertureIndex = *ap_index;
	UpdateApertures();
	UpdateChannels();
	UpdateGraph(TRUE, TRUE);
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		*ap_index = m_ApertureIndex;
		return true;
	}
	return false;		
}

void CChooseApertureDlg::UpdateApertures(void)
{
	char txt[256];

	m_Row = -1;
	delete m_Table;
	m_Table = NULL;

	int selectedIndex = -1;
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_AperView), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_Aper.Count(); i++) {
		const CAperture *aper = m_Aper.Get(i);
		sprintf(txt, "#%d (%.2f)", aper->Id(), aper->Radius());
		GtkTreeIter iter;
		gtk_list_store_append(m_Apertures, &iter);
		gtk_list_store_set(m_Apertures, &iter, 0, i, 1, txt, -1);
		if (m_ApertureIndex == i)
			selectedIndex = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)-1;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_AperView), GTK_TREE_MODEL(m_Apertures));
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_AperView));
	if (selectedIndex>=0) {
		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_Apertures), &iter, NULL, selectedIndex))
			gtk_tree_selection_select_iter(sel, &iter);
	} else {
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Apertures), &iter)) {
			gtk_tree_model_get(GTK_TREE_MODEL(m_Apertures), &iter, 0, &m_ApertureIndex, -1);
			gtk_tree_selection_select_iter(sel, &iter);
		} else
			m_ApertureIndex = -1;
	}
	gtk_widget_set_sensitive(m_AperView, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1);
	
	CmpackApDevCurve(NULL, &m_Table, m_Selection, *m_FrameSet);
}

void CChooseApertureDlg::UpdateChannels(void)
{
	int i;
	GtkTreeIter iter;
	CChannels *cy = m_Table->ChannelsY();

	// Update list of dependent channels
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_DataView), NULL);
	gtk_list_store_clear(m_Channels);
	for (i=0; i<cy->Count(); i++) {
		gtk_list_store_append(m_Channels, &iter);
		gtk_list_store_set(m_Channels, &iter, 0, i, 1, cy->GetName(i), -1);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_DataView), GTK_TREE_MODEL(m_Channels));
	int count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL);
	if (m_Column<0 || m_Column>=count) 
		m_Column = (count>0 ? 0 : -1);
	if (m_Column>=0) {
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_Channels), &iter, NULL, m_Column)) {
			GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_DataView));
			gtk_tree_selection_select_iter(sel, &iter);
		}
	}
	gtk_widget_set_sensitive(m_DataView, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>1);
}

void CChooseApertureDlg::UpdateGraph(gboolean autozoom_x, gboolean autozoom_y)
{
	int id;
	char txt[256];
	const CChannel *x, *y;

	id = m_Aper.GetId(m_ApertureIndex);
	x = m_Table->ChannelsX()->Get(0);
	y = m_Table->ChannelsY()->Get(m_Column);

	cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), NULL);
	if (m_GraphData)
		g_object_unref(m_GraphData);
	if (x && y) {
		m_GraphData = m_Table->ToGraphData(0, m_Column);
		cmpack_graph_view_set_x_axis(CMPACK_GRAPH_VIEW(m_GraphView), 
			FALSE, FALSE, x->Min(), x->Max(), 1.0, GRAPH_INT, 0, 0, "Aperture #");
		cmpack_graph_view_set_y_axis(CMPACK_GRAPH_VIEW(m_GraphView), 
			FALSE, FALSE, 0.0, y->Max(), 0.05, GRAPH_FIXED, 1, 2, "Std. dev.");
		m_Row = cmpack_graph_data_find_item(m_GraphData, (intptr_t)id);
		if (m_Row>=0) {
			sprintf(txt, "Aperture #%d", id);
			cmpack_graph_data_set_tag(m_GraphData, m_Row, txt);
			cmpack_graph_data_set_color(m_GraphData, m_Row, CMPACK_COLOR_RED);
			cmpack_graph_data_set_topmost(m_GraphData, m_Row, TRUE);
		}
		cmpack_graph_view_set_model(CMPACK_GRAPH_VIEW(m_GraphView), m_GraphData);
		cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_GraphView), true, true);
		cmpack_graph_view_reset_zoom(CMPACK_GRAPH_VIEW(m_GraphView), autozoom_x, autozoom_y);
	} else {
		m_GraphData = NULL;
	}
}

void CChooseApertureDlg::response_dialog(GtkDialog *pDlg, gint response_id, CChooseApertureDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CChooseApertureDlg::OnResponseDialog(gint response_id)
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CHOOSE_APERTURE);
		return false;
	}
	return true;
}

bool CChooseApertureDlg::OnCloseQuery()
{
	if (m_ApertureIndex<0) {
		ShowError(GTK_WINDOW(m_pDlg), "Please, select an aperture.");
		return false;
	}
	return true;
}

void CChooseApertureDlg::selection_changed(GtkTreeSelection *widget, CChooseApertureDlg *pMe)
{
	pMe->OnSelectionChanged(widget);
}

void CChooseApertureDlg::OnSelectionChanged(GtkTreeSelection *selection)
{
	int index, ch;
	char buf[64];
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		if (model == GTK_TREE_MODEL(m_Apertures) && m_GraphData) {
			gtk_tree_model_get(model, &iter, 0, &index, -1);
			if (index!=m_ApertureIndex) {
				if (m_Row>=0) {
					cmpack_graph_data_set_tag(m_GraphData, m_Row, NULL);
					cmpack_graph_data_set_color(m_GraphData, m_Row, CMPACK_COLOR_DEFAULT);
					cmpack_graph_data_set_topmost(m_GraphData, m_Row, FALSE);
				}
				m_ApertureIndex = index;
				m_Row = cmpack_graph_data_find_item(m_GraphData, m_Aper.GetId(m_ApertureIndex));
				if (m_Row>=0) {
					sprintf(buf, "Aperture #%d", m_Aper.GetId(m_ApertureIndex));
					cmpack_graph_data_set_tag(m_GraphData, m_Row, buf);
					cmpack_graph_data_set_color(m_GraphData, m_Row, CMPACK_COLOR_RED);
					cmpack_graph_data_set_topmost(m_GraphData, m_Row, TRUE);
				}
			}
		} else 
		if (model == GTK_TREE_MODEL(m_Channels)) {
			gtk_tree_model_get(model, &iter, 0, &ch, -1);
			if (ch!=m_Column) {
				m_Column = ch;
				UpdateGraph(FALSE, TRUE);
			}
		}
	}
}

void CChooseApertureDlg::item_activated(GtkWidget *pGraph, gint item, CChooseApertureDlg *pMe)
{
	pMe->OnItemActivated(pGraph, item);
}

void CChooseApertureDlg::OnItemActivated(GtkWidget *pGraph, gint item)
{
	int id;
	char buf[256];
	GtkTreeIter iter;

	if (item!=m_Row && m_GraphData) {
		if (m_Row>=0) {
			cmpack_graph_data_set_tag(m_GraphData, m_Row, NULL);
			cmpack_graph_data_set_color(m_GraphData, m_Row, CMPACK_COLOR_DEFAULT);
			cmpack_graph_data_set_topmost(m_GraphData, m_Row, FALSE);
		}
		id = (int)cmpack_graph_data_get_param(m_GraphData, item);
		m_ApertureIndex = m_Aper.Find(id);
		m_Row = item;
		if (m_Row>=0) {
			sprintf(buf, "Aperture #%d", id);
			cmpack_graph_data_set_tag(m_GraphData, m_Row, buf);
			cmpack_graph_data_set_color(m_GraphData, m_Row, CMPACK_COLOR_RED);
			cmpack_graph_data_set_topmost(m_GraphData, m_Row, TRUE);
		}
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_AperView));
		if (m_ApertureIndex>=0) {
			if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_Apertures), &iter, NULL, m_ApertureIndex)) 
				gtk_tree_selection_select_iter(selection, &iter);
		}
	}
}

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CChooseApertureSimpleDlg::CChooseApertureSimpleDlg(GtkWindow *pParent):m_FrameSet(NULL),
	m_ApertureIndex(-1)
{
	GtkWidget *scrwnd;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Choose aperture", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// List of apertures
	m_Apertures = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	m_AperView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_Apertures));
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Apertures");
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_AperView), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_AperView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_AperView);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), scrwnd, FALSE, TRUE, 0);
	gtk_widget_set_size_request(scrwnd, 140, 300);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}


//
// Destructor
//
CChooseApertureSimpleDlg::~CChooseApertureSimpleDlg()
{
	g_object_unref(m_Apertures);
	gtk_widget_destroy(m_pDlg);
}


// 
// Change aperture stored in project
//
bool CChooseApertureSimpleDlg::Execute(CFrameSet &fset, const CApertures &aper, const CSelection &sel, int *ap_index)
{
	m_FrameSet = &fset;
	m_Aper = aper;
	m_ApertureIndex = *ap_index;
	UpdateApertures();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		*ap_index = m_ApertureIndex;
		return true;
	}
	return false;		
}

void CChooseApertureSimpleDlg::UpdateApertures(void)
{
	char txt[256];

	int selectedIndex = -1;
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_AperView), NULL);
	gtk_list_store_clear(m_Apertures);
	for (int i=0; i<m_Aper.Count(); i++) {
		const CAperture *aper = m_Aper.Get(i);
		sprintf(txt, "#%d (%.2f)", aper->Id(), aper->Radius());
		GtkTreeIter iter;
		gtk_list_store_append(m_Apertures, &iter);
		gtk_list_store_set(m_Apertures, &iter, 0, i, 1, txt, -1);
		if (i==m_ApertureIndex)
			selectedIndex = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)-1;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_AperView), GTK_TREE_MODEL(m_Apertures));
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_AperView));
	if (selectedIndex>=0) {
		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_Apertures), &iter, NULL, selectedIndex))
			gtk_tree_selection_select_iter(sel, &iter);
	} else {
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Apertures), &iter)) {
			gtk_tree_model_get(GTK_TREE_MODEL(m_Apertures), &iter, 0, &m_ApertureIndex, -1);
			gtk_tree_selection_select_iter(sel, &iter);
		} else
			m_ApertureIndex = -1;
	}
	gtk_widget_set_sensitive(m_AperView, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Apertures), NULL)>1);
}

void CChooseApertureSimpleDlg::response_dialog(GtkDialog *pDlg, gint response_id, CChooseApertureSimpleDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CChooseApertureSimpleDlg::OnResponseDialog(gint response_id)
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CHOOSE_APERTURE);
		return false;
	}
	return true;
}

bool CChooseApertureSimpleDlg::OnCloseQuery()
{
	if (m_ApertureIndex<0) {
		ShowError(GTK_WINDOW(m_pDlg), "Please, select an aperture.");
		return false;
	}
	return true;
}

void CChooseApertureSimpleDlg::selection_changed(GtkTreeSelection *widget, CChooseApertureSimpleDlg *pMe)
{
	pMe->OnSelectionChanged(widget);
}

void CChooseApertureSimpleDlg::OnSelectionChanged(GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		if (model == GTK_TREE_MODEL(m_Apertures)) 
			gtk_tree_model_get(model, &iter, 0, &m_ApertureIndex, -1);
	}
}
