/**************************************************************

thumbnails_dlg.cpp (C-Munipack project)
The 'Show thumbnails' dialog
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
#include "configuration.h"
#include "thumbnails_dlg.h"
#include "progress_dlg.h"
#include "frameinfo_dlg.h"
#include "main_dlg.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"

enum tCommandId
{
	CMD_PROPERTIES = 100,
	CMD_REMOVE
};

static const CPopupMenu::tPopupMenuItem ContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_PROPERTIES, "Show _Properties" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_REMOVE, "_Remove From Project" },
	{ CPopupMenu::MB_END }
};

#define N_SIZES 3

static const struct {
	int width;
	const char *caption;
} Sizes[N_SIZES] = {
	{ 100, "Small" },
	{ 140, "Medium" },
	{ 200, "Large" },
};

//-------------------------   MESSAGE LOG DIALOG   --------------------------------

CThumbnailsDlg::CThumbnailsDlg(GtkWindow *pParent):m_pParent(pParent), m_IconView(NULL)
{
	int i, size;
	GtkWidget *tbar;
	GdkRectangle rc;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	m_SizeIndex = g_Project->GetInt("ThumbnailsDlg", "SizeIndex", 0, 0, N_SIZES-1);
	if (m_SizeIndex<=0 || m_SizeIndex>=N_SIZES) {
		m_SizeIndex = 1;
		size = CConfig::GetInt("Environment", "ImageSize", Sizes[1].width);
		for (i=0; i<N_SIZES; i++) {
			if (Sizes[i].width == size) {
				m_SizeIndex = i;
				break;
			}
		}
	}
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Thumbnails", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	
	// Dialog size
	GdkScreen *scr = gtk_window_get_screen(pParent);
	int mon = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(pParent)->window);
	gdk_screen_get_monitor_geometry(scr, mon, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.9*rc.width), RoundToInt(0.8*rc.height));

	// Dialog icon
	gchar *icon = get_icon_file("iconview");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Tool bar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_TEXT);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbar), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), tbar, FALSE, TRUE, 0);
	m_RemoveBtn = toolbar_new_button_from_stock(tbar, GTK_STOCK_DELETE, "Remove selected frames from the project");
	g_signal_connect(G_OBJECT(m_RemoveBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_PropertiesBtn = toolbar_new_button_from_stock(tbar, GTK_STOCK_INFO, "Show properties of a selected frame");
	g_signal_connect(G_OBJECT(m_PropertiesBtn), "clicked", G_CALLBACK(button_clicked), this);
	toolbar_new_separator(tbar);
	toolbar_new_label(tbar, "Size");
	m_IconSizes = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	for (i=0; i<N_SIZES; i++) {
		gtk_list_store_append(m_IconSizes, &iter);
		gtk_list_store_set(m_IconSizes, &iter, 0, Sizes[i].width, 1, Sizes[i].caption, -1);
	}
	m_SizeCombo = toolbar_new_combo(tbar, "Size of thumbnails");
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SizeCombo), GTK_TREE_MODEL(m_IconSizes));
	g_signal_connect(G_OBJECT(m_SizeCombo), "changed", G_CALLBACK(selection_changed), this);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_SizeCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_SizeCombo), renderer, "text", 1);

	// Icon view
	m_Scroller = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_Scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_Scroller), GTK_SHADOW_ETCHED_IN);
	gtk_widget_set_size_request(m_Scroller, 160, 160);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), m_Scroller, TRUE, TRUE, 0);

	// Popup menu
	m_Popup.Create(ContextMenu);

	// Show this icon to indicate a bad frame
	gchar *fpath = get_icon_file("badimage");
	m_pBadIcon = gdk_pixbuf_new_from_file(fpath, NULL);
	g_free(fpath);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CThumbnailsDlg::~CThumbnailsDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_pBadIcon);
	g_object_unref(m_IconSizes);
}

void CThumbnailsDlg::response_dialog(GtkDialog *pDlg, gint response_id, CThumbnailsDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CThumbnailsDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_THUMBNAILS);
		return false;
	}
	return true;
}

void CThumbnailsDlg::ShowModal()
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_SizeCombo), m_SizeIndex);
	if (RebuildData()) {
		UpdateControls();
		gtk_dialog_run(GTK_DIALOG(m_pDlg));
	}
}

bool CThumbnailsDlg::RebuildData(void)
{
	if (m_IconView) {
		gtk_widget_destroy(m_IconView);
		m_IconView = NULL;
	}
	CProgressDlg pDlg(m_pParent, "Updating thumbnails ...");
	pDlg.SetMinMax(0, g_Project->GetFileCount());
	if (pDlg.Execute(ExecuteProc, this)) {
		m_IconView = gtk_icon_view_new();
		gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(m_IconView), GTK_SELECTION_MULTIPLE);
		g_signal_connect(G_OBJECT(m_IconView), "selection-changed", G_CALLBACK(selection_changed), this);
		g_signal_connect(G_OBJECT(m_IconView), "button-press-event", G_CALLBACK(button_press_event), this);
		gtk_container_add(GTK_CONTAINER(m_Scroller), m_IconView);
		gtk_icon_view_set_text_column(GTK_ICON_VIEW(m_IconView), FRAME_STRINGID);
		gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(m_IconView), FRAME_THUMBNAIL);
		gtk_icon_view_set_tooltip_column(GTK_ICON_VIEW(m_IconView), FRAME_ORIGFILE);
		gtk_icon_view_set_model(GTK_ICON_VIEW(m_IconView), g_Project->FileList());
		gtk_widget_show(m_IconView);
		return true;
	}
	return false;
}

void CThumbnailsDlg::selection_changed(GtkWidget *pWidget, CThumbnailsDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CThumbnailsDlg::OnSelectionChanged(GtkWidget *pWidget)
{
	if (pWidget == m_IconView)
		UpdateControls();
	else if (pWidget == m_SizeCombo) {
		int index = gtk_combo_box_get_active(GTK_COMBO_BOX(pWidget));
		if (index!=m_SizeIndex) {
			m_SizeIndex = index;
			g_Project->SetInt("ThumbnailsDlg", "SizeIndex", m_SizeIndex);
			g_Project->ClearThumbnails();
			RebuildData();
			UpdateControls();
		}
	}
}

void CThumbnailsDlg::button_clicked(GtkToolItem *pWidget, CThumbnailsDlg *pMe)
{
	pMe->OnButtonClicked(pWidget);
}

void CThumbnailsDlg::OnButtonClicked(GtkToolItem *pWidget)
{
	if (pWidget==m_RemoveBtn)
		RemoveFromProject();
	else if (pWidget==m_PropertiesBtn)
		ShowProperties();
}

void CThumbnailsDlg::UpdateControls(void)
{
	int count = SelectedCount();
	gtk_widget_set_sensitive(GTK_WIDGET(m_PropertiesBtn), count==1);
	gtk_widget_set_sensitive(GTK_WIDGET(m_RemoveBtn), count>=1);
}

void CThumbnailsDlg::ShowProperties(void)
{
	GList *list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(m_IconView));
	if (list) {
		CFrameInfoDlg pDlg(GTK_WINDOW(m_pDlg));
		pDlg.Show((GtkTreePath*)list->data);
	}
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}

void CThumbnailsDlg::RemoveFromProject(void)
{
	bool ok, close_output;
	int count;

	GList *list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(m_IconView));
	if (list) {
		count = g_list_length(list);
		if (g_Project->ContainsReferenceFrame(list)) {
			if (count==1) 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "The selected frame is a reference frame. This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
			else
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "The selection includes a reference frame. This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
			close_output = true;
		} else {
			if (count==1) 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frame from the current project?");
			else
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frames from the current project?");
			close_output = false;
		}
		if (ok) {
			if (close_output) {
				g_MainWnd->CloseOutputDlgs();
				g_MainWnd->CloseNewFiles();
			}
			g_MainWnd->BeginUpdate();
			g_Project->RemoveFiles(list);
			g_MainWnd->EndUpdate();
			UpdateControls();
		}
		g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(list);
	}
}

int CThumbnailsDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CThumbnailsDlg*)userdata)->ProcessFiles(sender);
}

int CThumbnailsDlg::ProcessFiles(CProgressDlg *sender)
{
	int state, frameid, files = 0;
	gchar *filename, *ftemp, *fpath;
	GtkTreeModel *pModel = g_Project->FileList();
	GtkTreeIter iter;

	gboolean ok = gtk_tree_model_get_iter_first(pModel, &iter);
	while (ok) {
		gtk_tree_model_get(pModel, &iter, FRAME_ID, &frameid, FRAME_STATE, &state, -1);
		if ((state & CFILE_THUMBNAIL)==0) {
			if (state & CFILE_CONVERSION) {
				gtk_tree_model_get(pModel, &iter, FRAME_TEMPFILE, &ftemp, -1);
				filename = g_build_filename(g_Project->DataDir(), ftemp, NULL);
				g_free(ftemp);
			} else {
				gtk_tree_model_get(pModel, &iter, FRAME_ORIGFILE, &fpath, -1);
				filename = g_strdup(fpath);
			}
			sender->SetFileName(g_path_get_basename(filename));
			GdkPixbuf *pImg = MakeIcon(filename);
			if (pImg) {
				g_Project->SetThumbnail(&iter, pImg);
				g_object_unref(pImg);
			} else {
				g_Project->SetThumbnail(&iter, m_pBadIcon);
			}
			g_free(filename);
		}
		sender->SetProgress(files++);
		if (sender->Cancelled()) 
			return FALSE;
		ok = gtk_tree_model_iter_next(pModel, &iter);
	}
	return TRUE;
}

// Make thumbnail image
GdkPixbuf *CThumbnailsDlg::MakeIcon(const gchar *filename)
{
	int		src_width, src_height, w, h, size;
	CImage	img;
	GdkPixbuf *res = NULL;

	if (m_SizeIndex>=0 && m_SizeIndex<N_SIZES)
		size = Sizes[m_SizeIndex].width;
	else
		size = Sizes[0].width;

	// Open file
	if (img.Load(filename, CMPACK_BITPIX_AUTO)) {
		GdkPixbuf *src = img.ToPixBuf(m_Negative, false, false, m_RowsUpward);
		// Compute thumbail size
		src_width = gdk_pixbuf_get_width(src);
		src_height = gdk_pixbuf_get_height(src);
		if (src_width>size || src_height>size) {
			int stretchy = (size * src_height) / src_width;
			if (stretchy <= size) {
				w = size;
				h = MAX(1, stretchy);
			} else {
				int stretchx = (size * src_width) / src_height;
				w = MAX(1, stretchx);
				h = size;
			}
			res = gdk_pixbuf_scale_simple(src, w, h, GDK_INTERP_BILINEAR);
			g_object_unref(src);
		} else {
			res = src;
		}
	}
	return res;
}

gboolean CThumbnailsDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CThumbnailsDlg *pMe)
{
	if (widget==pMe->m_IconView && event->button == 3) {
		pMe->OnContextMenu(widget, event);
		return TRUE;
	}
	return FALSE;
}

int CThumbnailsDlg::SelectedCount(void)
{
	int count;

	GList *list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(m_IconView));
	count = g_list_length(list);
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
	return count;
}

void CThumbnailsDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int x = (int)event->x, y = (int)event->y;

	GtkTreePath *path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(m_IconView), x, y);
	if (path) {
		gtk_widget_grab_focus(m_IconView);
		if (SelectedCount()<=1) {
			gtk_icon_view_unselect_all(GTK_ICON_VIEW(m_IconView));
			gtk_icon_view_select_path(GTK_ICON_VIEW(m_IconView), path);
			gtk_icon_view_set_cursor(GTK_ICON_VIEW(m_IconView), path, NULL, FALSE);
		}
		int count = SelectedCount();
		m_Popup.Enable(CMD_PROPERTIES, count==1);
		m_Popup.Enable(CMD_REMOVE, count>=1);
		switch (m_Popup.Execute(event))
		{
		case CMD_PROPERTIES:
			ShowProperties();
			break;
		case CMD_REMOVE:
			RemoveFromProject();
			break;
		}
		gtk_tree_path_free(path);
	}
}
