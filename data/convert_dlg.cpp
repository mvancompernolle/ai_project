/**************************************************************

convert_dlg.cpp (C-Munipack project)
The 'Convert files' dialog
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
#include "profile.h"
#include "convert_dlg.h"
#include "project_dlg.h"
#include "progress_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"

static const struct {
	int type;
	const gchar *caption;
	bool debug;
} RAWChannels[] = {
	{ CMPACK_CHANNEL_DEFAULT,	"Grayscale (R+G1+G2+B)" },
	{ CMPACK_CHANNEL_RED,		"Red" },
	{ CMPACK_CHANNEL_GREEN,		"Green (G1+G2)/2" },
	{ CMPACK_CHANNEL_BLUE,		"Blue" },
	{ CMPACK_CHANNEL_0,			"Channel #1", true },
	{ CMPACK_CHANNEL_1,			"Channel #2", true },
	{ CMPACK_CHANNEL_2,			"Channel #3", true },
	{ CMPACK_CHANNEL_3,			"Channel #4", true },
	{ -1 }
};

//-------------------------   MAIN WINDOW   --------------------------------

CConvertDlg::CConvertDlg(GtkWindow *pParent):m_pParent(pParent), m_InFiles(0), m_OutFiles(0), 
	m_FileList(NULL)
{
	GtkWidget *vbox, *bbox, *label;
	GSList *group;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	tProjectType type = g_Project->ProjectType();

	m_Channels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	for (int i=0; RAWChannels[i].caption; i++) {
		if (!RAWChannels[i].debug || type==PROJECT_TEST) {
			gtk_list_store_append(m_Channels, &iter);
			gtk_list_store_set(m_Channels, &iter, 0, RAWChannels[i].type, 1, RAWChannels[i].caption, -1);
		}
	}

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Fetch/convert files", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_EXECUTE, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("convert");
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
	gtk_widget_set_tooltip_text(m_AllBtn, "Convert all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Convert frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Raw options
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>DSLR images conversion</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_ChCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_Channels));
	gtk_widget_set_tooltip_text(m_ChCombo, "Color to grayscale conversion mode");
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_ChCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_ChCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(vbox), m_ChCombo, FALSE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Options
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), bbox);
	m_OptionsBtn = gtk_button_new_with_label("More options...");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_OptionsBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CConvertDlg::~CConvertDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_Channels);
}

void CConvertDlg::response_dialog(GtkDialog *pDlg, gint response_id, CConvertDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CConvertDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CONVERT_FILES);
		return false;
	}
	return true;
}

gboolean CConvertDlg::foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
	return FALSE;
}

void CConvertDlg::foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
}

void CConvertDlg::Execute()
{
	int res;
	char msg[256];
	CProject::tStatus status;

	g_Project->GetStatus(&status);

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);
	SelectChannel((CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT));
			
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	g_Project->SetInt("Convert", "ColorChannel", SelectedChannel());

	m_FileList = NULL;
	bool all_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))!=0;
	if (all_files) {
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
		if (all_files) {
			g_Project->ClearThumbnails();
			g_Project->ClearTempFiles();
			g_Project->ClearCorrections();
			g_Project->ClearReference();
			g_Project->ClearObject();
		}
		CProgressDlg pDlg(m_pParent, "Converting files");
		pDlg.SetMinMax(0, g_list_length(m_FileList));
		res = pDlg.Execute(ExecuteProc, this);
		if (res>0) {
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

int CConvertDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CConvertDlg*)userdata)->ConvertFiles(sender);
}

int CConvertDlg::ConvertFiles(CProgressDlg *sender)
{
	int res, frame_id;
	char *fpath, *fbase, msg[128];
	GtkTreePath *path;
	CConvertProc konv;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Fetch/convert files ------");

	res = konv.Init(sender);
	if (res==0) {
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				frame_id = g_Project->GetFrameID(path);
				if (g_Project->IsReferenceFrame(path)) 
					g_Project->ClearReference();
				fpath = g_Project->GetSourceFile(path);
				fbase = g_filename_display_basename(fpath);
				sender->SetFileName(fbase);
				g_free(fbase);
				g_free(fpath);
				sprintf(msg, "Frame #%d:", frame_id);
				sender->Print(msg);
				sender->SetProgress(m_InFiles++);
				if (konv.Execute(path)==0)
					m_OutFiles++;
			}
			gtk_tree_path_free(path);
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

void CConvertDlg::button_clicked(GtkButton *button, CConvertDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CConvertDlg::OnButtonClicked(GtkButton *pButton)
{
	if (pButton==GTK_BUTTON(m_OptionsBtn))
		EditPreferences();
}

void CConvertDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_SOURCE_FILES);
}

void CConvertDlg::SelectChannel(CmpackChannel channel)
{
	int id;
	GtkTreeIter iter;

	gboolean ok = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Channels), &iter);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(m_Channels), &iter, 0, &id, -1);
		if (id==channel) {
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_ChCombo), &iter);
			break;
		}
		ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_Channels), &iter);
	}
	if (!ok && gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>0)
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_ChCombo), 0);
}

CmpackChannel CConvertDlg::SelectedChannel(void)
{
	return (CmpackChannel)SelectedItem(GTK_COMBO_BOX(m_ChCombo), CMPACK_CHANNEL_DEFAULT);
}
