/**************************************************************

photometry_dlg.cpp (C-Munipack project)
The 'Photometry' dialog
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
#include "photometry_dlg.h"
#include "progress_dlg.h"
#include "project_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"

//-------------------------   HELPER FUNCTION   --------------------------------

static gboolean foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
	return FALSE;
}

static void foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
}

//-------------------------   MAIN WINDOW   --------------------------------

CPhotometryDlg::CPhotometryDlg(GtkWindow *pParent):m_pParent(pParent), m_InFiles(0), m_OutFiles(0), 
	m_FileList(NULL)
{
	GtkWidget *label, *vbox, *bbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Photometry", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("photometry");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_size_request(vbox, 300, -1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Frame selection
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Process all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Process frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Options
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), bbox);
	m_Options1Btn = gtk_button_new_with_label("Star detection options");
	gtk_widget_set_tooltip_text(m_Options1Btn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_Options1Btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Options1Btn), "clicked", G_CALLBACK(button_clicked), this);
	m_Options2Btn = gtk_button_new_with_label("Photometry options");
	gtk_widget_set_tooltip_text(m_Options2Btn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_Options2Btn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Options2Btn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CPhotometryDlg::~CPhotometryDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CPhotometryDlg::response_dialog(GtkDialog *pDlg, gint response_id, CPhotometryDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CPhotometryDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_PHOTOMETRY);
		return false;
	}
	return true;
}

void CPhotometryDlg::Execute()
{
	int res;
	char msg[256];

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		gtk_widget_hide(m_pDlg);
		m_FileList = NULL;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))) {
			// All files
			GtkTreeModel *pList = g_Project->FileList();
			if (gtk_tree_model_iter_n_children(pList, NULL)>0) 
				gtk_tree_model_foreach(pList, foreach_all_files, &m_FileList);
			else
				ShowError(m_pParent, "There are no files in the project.");
		} else {
			// Selected files
			GtkTreeSelection *pSel = g_MainWnd->GetSelection();
			if (gtk_tree_selection_count_selected_rows(pSel)>0) 
				gtk_tree_selection_selected_foreach(pSel, foreach_sel_files, &m_FileList);
			else
				ShowError(m_pParent, "There are no selected files.");
		}
		if (m_FileList) {
			CProgressDlg pDlg(m_pParent, "Processing PHOTOMETRY");
			pDlg.SetMinMax(0, g_list_length(m_FileList));
			res = pDlg.Execute(ExecuteProc, this);
			if (res!=0) {
				char *msg = cmpack_formaterror(res);
				ShowError(m_pParent, msg, true);
				cmpack_free(msg);
			} else
			if (m_OutFiles==0) {
				ShowError(m_pParent, "No file was successfully processed.", true);
			} else {
				if (m_OutFiles!=m_InFiles) {
					sprintf(msg, "%d file(s) were successfully processed, %d file(s) failed.", 
						m_OutFiles, m_InFiles-m_OutFiles);
					ShowWarning(m_pParent, msg, true);
				} else {
					sprintf(msg, "All %d file(s) were successfully processed.", m_OutFiles);
					ShowInformation(m_pParent, msg, true);
				}
			}
			g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
			g_list_free(m_FileList);
			m_FileList = NULL;
		}
		g_Project->Save();
	}
}

int CPhotometryDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CPhotometryDlg*)userdata)->ProcessFiles(sender);
}

int CPhotometryDlg::ProcessFiles(CProgressDlg *sender)
{
	int frameid, res;
	char *tpath, msg[128];
	GtkTreePath *path;
	CPhotometryProc phot;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Photometry ------");

	res = phot.Init(sender);
	if (res==0) {
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				frameid = g_Project->GetFrameID(path);
				if (g_Project->IsReferenceFrame(path))
					g_Project->ClearReference();
				tpath = g_Project->GetImageFileName(path);
				sender->SetFileName(tpath);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				g_free(tpath);
				sender->SetProgress(m_InFiles++);
				if (phot.Execute(path)==0)
					m_OutFiles++;
				gtk_tree_path_free(path);
			}
			if (sender->Cancelled()) 
				break;
		}
		if (sender->Cancelled())
			sprintf(msg, "Cancelled at the user's request");
		else
			sprintf(msg, "====== %d succeeded, %d failed ======", m_OutFiles, m_InFiles-m_OutFiles);
		sender->Print(msg);
	} else {
		char *aux = cmpack_formaterror(res);
		sender->Print(aux);
		cmpack_free(aux);
	}
	return res;
}

void CPhotometryDlg::button_clicked(GtkButton *button, CPhotometryDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CPhotometryDlg::OnButtonClicked(GtkButton *pButton)
{
	if (pButton==GTK_BUTTON(m_Options1Btn))
		EditPreferences(PAGE_STAR_DETECTION);
	else if (pButton==GTK_BUTTON(m_Options2Btn))
		EditPreferences(PAGE_PHOTOMETRY);
}

void CPhotometryDlg::EditPreferences(tProfilePageId page)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(page);
}
