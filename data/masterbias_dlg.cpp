/**************************************************************

masterbias_dlg.cpp (C-Munipack project)
The 'Master bias-frame' dialog
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
#include "utils.h"
#include "configuration.h"
#include "masterbias_dlg.h"
#include "ccdfile_dlg.h"
#include "progress_dlg.h"
#include "proc_classes.h"
#include "main.h"
#include "ctxhelp.h"
#include "project_dlg.h"

//-------------------------   HELPER FUNCTIONS   --------------------------------

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

CMasterBiasDlg::CMasterBiasDlg(GtkWindow *pParent):m_pParent(pParent), m_InFiles(0), 
	m_FileList(NULL), m_FilePath(NULL)
{
	GtkWidget *vbox, *xbox, *bbox;
	GSList *group;
	GtkFileFilter *filters[2];

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Master bias frame", pParent, 
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Create a master bias frame and save it to the specified file");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*." FILE_EXTENSION_FITS);
	gtk_file_filter_set_name(filters[0], "FITS files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);

	// Dialog icon
	gchar *icon = get_icon_file("masterbias");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Frame selection
	xbox = gtk_vbox_new(FALSE, 8);
	m_ProcFrame = gtk_frame_new("Process");
	gtk_box_pack_start(GTK_BOX(xbox), m_ProcFrame, FALSE, TRUE, 0);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(m_ProcFrame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Include all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Include frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Options
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_box_pack_start(GTK_BOX(xbox), bbox, FALSE, TRUE, 0);
	m_OptionsBtn = gtk_button_new_with_label("Options");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_OptionsBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), xbox);
	gtk_widget_show_all(xbox);
}

CMasterBiasDlg::~CMasterBiasDlg()
{
	g_free(m_FilePath);
	gtk_widget_destroy(m_pDlg);
}

void CMasterBiasDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMasterBiasDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMasterBiasDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_MASTER_BIAS);
		return false;
	}
	return true;
}

void CMasterBiasDlg::button_clicked(GtkWidget *button, CMasterBiasDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CMasterBiasDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==m_OptionsBtn)
		EditPreferences();
}

void CMasterBiasDlg::Execute()
{
	char msg[256];
	char *folder, *filename;

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	// Restore last folder and file name
	folder = g_Project->GetStr("MasterBias", "Folder", NULL);
	if (!folder)
		folder = CConfig::GetStr("MasterBias", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);
	filename = g_Project->GetStr("MasterBias", "File", "masterbias." FILE_EXTENSION_FITS);
	if (filename) 
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), filename);
	g_free(filename);

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	// Target file path and name
	m_FilePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	CConfig::SetStr("BiasCorr", "File", m_FilePath);

	gchar *dirpath = g_path_get_dirname(m_FilePath);
	g_Project->SetStr("MasterBias", "Folder", dirpath);
	CConfig::SetStr("MasterBias", "Folder", dirpath);
	g_free(dirpath);
	gchar *basename = g_path_get_basename(m_FilePath);
	g_Project->SetStr("MasterBias", "File", basename);
	g_free(basename);

	// Make list of files
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

	// Process files
	if (m_FileList) {
		CProgressDlg pDlg(m_pParent, "Reading files");
		pDlg.SetMinMax(0, g_list_length(m_FileList));
		int res = pDlg.Execute(ExecuteProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else if (!pDlg.Cancelled()) {
			sprintf(msg, "All %d file(s) were successfully processed.", m_InFiles);
			ShowInformation(m_pParent, msg, true);
			CCCDFileDlg *pdlg = new CCCDFileDlg();
			GError *error = NULL;
			if (pdlg->Load(m_FilePath, &error)) {
				pdlg->Show();
				CConfig::AddFileToRecentList(TYPE_IMAGE, m_FilePath);
			} else {
				if (error) {
					ShowError(m_pParent, error->message, true);
					g_error_free(error);
				}
				delete pdlg;
			}
		}
		g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
		g_list_free(m_FileList);
		m_FileList = NULL;
	}
	g_free(m_FilePath);
	m_FilePath = NULL;
	g_Project->Save();
	CConfig::Flush();
}

int CMasterBiasDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CMasterBiasDlg*)userdata)->ProcessFiles(sender);
}

int CMasterBiasDlg::ProcessFiles(CProgressDlg *sender)
{
	int frameid, res;
	char *fpath, *fbase, msg[128];
	GtkTreePath *path;
	CMasterBiasProc mbias;

	m_InFiles = 0;
	sender->Print("------ Make master bias frame ------");

	res = mbias.Open(sender, m_FilePath);
	if (res==0) {
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				frameid = g_Project->GetFrameID(path);
				fpath = g_Project->GetImageFileName(path);
				sender->SetFileName(fpath);
				g_free(fpath);
				sender->SetProgress(m_InFiles++);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				res = mbias.Add(path);
				gtk_tree_path_free(path);
			}
			if (res!=0 || sender->Cancelled()) 
				break;
		}
		if (res==0 && !sender->Cancelled()) {
			sender->SetTitle("Making output file");
			fbase = g_filename_display_basename(m_FilePath);
			sender->SetFileName(fbase);
			g_free(fbase);
			res = mbias.Close();
		}
		if (sender->Cancelled())
			sprintf(msg, "Cancelled at the user's request");
		else
			sprintf(msg, "====== %d succeeded ======", m_InFiles);
		sender->Print(msg);
	} else {
		char *aux = cmpack_formaterror(res);
		sender->Print(aux);
		cmpack_free(aux);
	}
	return res;
}

void CMasterBiasDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_MASTER_BIAS);
}
