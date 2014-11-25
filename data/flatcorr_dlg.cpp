/**************************************************************

flatcorr_dlg.cpp (C-Munipack project)
The 'Flat correction' dialog
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

#include "flatcorr_dlg.h"
#include "main.h"
#include "progress_dlg.h"
#include "utils.h"
#include "project.h"
#include "configuration.h"
#include "configuration.h"
#include "proc_classes.h"
#include "ctxhelp.h"

//-------------------------   MAIN WINDOW   --------------------------------

CFlatCorrDlg::CFlatCorrDlg(GtkWindow *pParent):m_pParent(pParent)
{
	GtkWidget *vbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Flat correction", pParent, GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_EXECUTE, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	g_signal_connect(G_OBJECT(m_pDlg), "update-preview", G_CALLBACK(update_preview), this);
	g_signal_connect(G_OBJECT(m_pDlg), "selection-changed", G_CALLBACK(selection_changed), this);

	// Dialog icon
	gchar *icon = get_icon_file("flatcorr");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Process selection
	m_ProcFrame = gtk_frame_new("Apply correction to");
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(m_ProcFrame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Apply correction to all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Apply correction to frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), m_ProcFrame);
	gtk_widget_show_all(m_ProcFrame);

	// Preview widget
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(m_pDlg), 
		GTK_WIDGET(g_object_ref(m_Preview.Handle())));
}

CFlatCorrDlg::~CFlatCorrDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CFlatCorrDlg::Execute()
{
	int res;
	char msg[256];
	char *path;

	// Selection
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	// Restore selection
	m_Preview.Clear();
	path = g_Project->GetStr("FlatCorr", "File", NULL);
	if (!path)
		path = CConfig::GetStr("FlatCorr", "File", NULL);
	if (path && gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(m_pDlg), path))
		m_Preview.Update(path);
	else {
		gchar *dir = g_path_get_dirname(path ? path : g_Project->Path());
		if (dir && g_file_test(dir, G_FILE_TEST_IS_DIR))
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), dir);
		g_free(dir);
	}
	g_free(path);

	// Enable/disable previews
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(m_pDlg), 
		!CConfig::GetBool(CConfig::DISABLE_PREVIEWS));

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;

	// Save the selection
	gtk_widget_hide(m_pDlg);
	m_FlatFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	g_Project->SetStr("FlatCorr", "File", m_FlatFile);
	CConfig::SetStr("FlatCorr", "File", m_FlatFile);

	// Make list of files
	m_FileList = NULL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))) {
		// All files
		GtkTreeModel *pList = g_Project->FileList();
		if (gtk_tree_model_iter_n_children(pList, NULL)>0) 
			gtk_tree_model_foreach(pList, GtkTreeModelForeachFunc(foreach_all_files), &m_FileList);
		else
			ShowError(m_pParent, "There are no files in the project.");
	} else {
		// Selected files
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		if (gtk_tree_selection_count_selected_rows(pSel)>0) 
			gtk_tree_selection_selected_foreach(pSel, GtkTreeSelectionForeachFunc(foreach_sel_files), &m_FileList);
		else
			ShowError(m_pParent, "There are no selected files.");
	}

	// Process files
	if (m_FileList) {
		CProgressDlg pDlg(m_pParent, "Processing FLAT correction");
		pDlg.SetMinMax(0, g_list_length(m_FileList));
		res = pDlg.Execute(ExecuteProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else if (m_OutFiles==0) {
			ShowError(m_pParent, "No file was successfully processed.", true);
		} else if (m_OutFiles!=m_InFiles) {
			sprintf(msg, "%d file(s) were successfully processed, %d file(s) failed.", 
				m_OutFiles, m_InFiles-m_OutFiles);
			ShowWarning(m_pParent, msg, true);
		} else {
			sprintf(msg, "All %d file(s) were successfully processed.", m_OutFiles);
			ShowInformation(m_pParent, msg, true);
		}
		g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
		g_list_free(m_FileList);
		m_FileList = NULL;
	}
	g_free(m_FlatFile);
	m_FlatFile = NULL;
	g_Project->Save();
}

bool CFlatCorrDlg::SelectFrame(char **fpath)
{
	// Selection
	gtk_widget_hide(m_ProcFrame);
	m_Preview.Clear();
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(m_pDlg), 
		!CConfig::GetBool(CConfig::DISABLE_PREVIEWS));

	// Select dark frame
	if (*fpath && (*fpath)[0]!=0) {
		if (gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(m_pDlg), *fpath)) 
			m_Preview.Update(*fpath);
		else {
			gchar *dir = g_path_get_dirname(*fpath ? *fpath : g_Project->Path());
			if (dir && g_file_test(dir, G_FILE_TEST_IS_DIR))
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), dir);
			g_free(dir);
		}
	} else {
		gchar *path = g_Project->GetStr("FlatCorr", "File", NULL);
		if (!path)
			path = CConfig::GetStr("FlatCorr", "File", NULL);
		if (gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(m_pDlg), path))
			m_Preview.Update(path);
		else {
			gchar *dir = g_path_get_dirname(path ? path : g_Project->Path());
			if (dir && g_file_test(dir, G_FILE_TEST_IS_DIR))
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), dir);
			g_free(dir);
		}
		g_free(path);
	}

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		g_Project->SetStr("FlatCorr", "File", path);
		CConfig::SetStr("FlatCorr", "File", path);
		*fpath = (char*)g_realloc(*fpath, strlen(path)+1);
		strcpy(*fpath, path);
		g_free(path);
		return true;
	}
	return false;
}

gboolean CFlatCorrDlg::foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, GList **pList)
{
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*pList = g_list_append(*pList, rowref);
	return FALSE;
}

void CFlatCorrDlg::foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, GList **pList)
{
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*pList = g_list_append(*pList, rowref);
}

void CFlatCorrDlg::response_dialog(GtkDialog *pDlg, gint response_id, CFlatCorrDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CFlatCorrDlg::OnResponseDialog(gint response_id)
{
	gchar *fname, *f;

	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		if (!fname || !g_file_test(fname, G_FILE_TEST_IS_REGULAR)) {
			ShowError(GTK_WINDOW(m_pDlg), "Choose a flat correction frame");
			g_free(fname);
			return false;
		}
		f = g_locale_from_utf8(fname, -1, NULL, NULL, NULL);
		if (!cmpack_ccd_test(f)) {
			ShowError(GTK_WINDOW(m_pDlg), "The selected file is not a CCD frame.");
			g_free(f);
			g_free(fname);
			return false;
		}
		g_free(f);
		g_free(fname);
		break;

	case GTK_RESPONSE_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_FLAT_CORRECTION);
		return false;
	}
	return true;
}

int CFlatCorrDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CFlatCorrDlg*)userdata)->ProcessFiles(sender);
}

int CFlatCorrDlg::ProcessFiles(CProgressDlg *sender)
{
	int frameid, res;
	char *tpath, msg[128];
	GtkTreePath *path;
	CFlatCorrProc flat;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Flat correction ------");

	res = flat.Init(sender, m_FlatFile);
	if (res==0) {
		g_Project->SetOrigFlatFile(m_FlatFile);
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				frameid = g_Project->GetFrameID(path);
				tpath = g_Project->GetImageFileName(path);
				sender->SetFileName(tpath);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				g_free(tpath);
				sender->SetProgress(m_InFiles++);
				if (flat.Execute(path)==0)
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

void CFlatCorrDlg::update_preview(GtkFileChooser *widget, CFlatCorrDlg *pMe)
{
	pMe->OnUpdatePreview(widget);
}

void CFlatCorrDlg::OnUpdatePreview(GtkFileChooser *widget)
{
	if (!CConfig::GetBool(CConfig::DISABLE_PREVIEWS)) {
		gchar *file = gtk_file_chooser_get_preview_filename(widget);
		if (file) {
			m_Preview.Update(file);
			g_free(file);
		} else {
			m_Preview.Clear();
		}
	}
}

void CFlatCorrDlg::selection_changed(GtkFileChooser *widget, CFlatCorrDlg *pMe)
{
	pMe->OnSelectionChanged(widget);
}

void CFlatCorrDlg::OnSelectionChanged(GtkFileChooser *widget)
{
	UpdateControls();
}

void CFlatCorrDlg::UpdateControls(void)
{
	bool ok;

	gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	if (fname) {
		ok = g_file_test(fname, G_FILE_TEST_IS_REGULAR)!=0;
		g_free(fname);
	} else {
		ok = false;
	}

	GtkWidget *ok_btn = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(ok_btn, ok);
}
