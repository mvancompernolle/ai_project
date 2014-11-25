/**************************************************************

merge_dlg.cpp (C-Munipack project)
The 'Merge frames' dialog
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
#include "merge_dlg.h"
#include "progress_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"
#include "project_dlg.h"

//-------------------------   HELPER FUNCTIONS  --------------------------------

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

CMergeDlg::CMergeDlg(GtkWindow *pParent):m_pParent(pParent), m_FileList(NULL), m_FileName(NULL)
{
	GtkWidget *vbox, *tbox, *table, *bbox;
	GtkObject *adj;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Merge frames", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("merge");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Frame selection
	GtkWidget *label = gtk_label_new(NULL);
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

	// Target frames
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Merging rules</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	table = gtk_table_new(5, 4, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), table);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_table_set_row_spacing(GTK_TABLE(table), 3, 8);
	gtk_table_set_col_spacing(GTK_TABLE(table), 0, 16);

	m_MultiBtn = gtk_radio_button_new_with_label(NULL, "Split frames and process each group separately");
	gtk_widget_set_tooltip_text(m_MultiBtn, "Split input frames to groups using "
		"the following rules and create an output frame for each group.");
	g_signal_connect(G_OBJECT(m_MultiBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(table), m_MultiBtn, 0, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	m_MaxFramesLabel = gtk_label_new("Merge every");
	gtk_misc_set_alignment(GTK_MISC(m_MaxFramesLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxFramesLabel, 1, 2, 1, 2);
	adj = gtk_adjustment_new(5, 1, 999, 1, 5, 0);
	m_MaxFramesEdit = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_MaxFramesEdit, "Maximum number of input frames included in one output frame");
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxFramesEdit, 2, 3, 1, 2);
	m_MaxFramesUnit = gtk_label_new("frames");
	gtk_misc_set_alignment(GTK_MISC(m_MaxFramesUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxFramesUnit, 3, 4, 1, 2);

	m_MaxTimeLabel = gtk_label_new("Maximum time span");
	gtk_misc_set_alignment(GTK_MISC(m_MaxTimeLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxTimeLabel, 1, 2, 2, 3);
	adj = gtk_adjustment_new(600, 1, 99999, 1, 60, 0);
	m_MaxTimeEdit = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_MaxTimeEdit, "Maximum time difference between frames included in one output frame");
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxTimeEdit, 2, 3, 2, 3);
	m_MaxTimeUnit = gtk_label_new("seconds");
	gtk_misc_set_alignment(GTK_MISC(m_MaxTimeUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MaxTimeUnit, 3, 4, 2, 3);

	m_MinFramesLabel = gtk_label_new("Merge at least");
	gtk_misc_set_alignment(GTK_MISC(m_MinFramesLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MinFramesLabel, 1, 2, 3, 4);
	adj = gtk_adjustment_new(5, 1, 999, 1, 5, 0);
	m_MinFramesEdit = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MinFramesEdit, 2, 3, 3, 4);
	gtk_widget_set_tooltip_text(m_MinFramesEdit, "Do not create an output frame if a group contains less frames then the specified value");
	m_MinFramesUnit = gtk_label_new("frames");
	gtk_misc_set_alignment(GTK_MISC(m_MinFramesUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), m_MinFramesUnit, 3, 4, 3, 4);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_MultiBtn));
	m_SingleBtn = gtk_radio_button_new_with_label(group, "Merge all source frames to a single output frame");
	gtk_widget_set_tooltip_text(m_SingleBtn, "Create a single output frame using all source frames");
	g_signal_connect(G_OBJECT(m_SingleBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(table), m_SingleBtn, 0, 4, 4, 5, GTK_FILL, GTK_FILL, 0, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Target path and file name
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Output frames</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	tbox = gtk_table_new(3, 7, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), tbox);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 1, 8);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 3, 8);

	label = gtk_label_new("Save files to");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	m_OutPath = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_OutPath, "Path to the directory where output files shall be saved to");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OutPath, 1, 6, 0, 1);
	m_PathBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_PathBtn, "Change the target directory");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), m_PathBtn, 6, 7, 0, 1, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);

	label = gtk_label_new("File name prefix");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	m_OutName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_OutName, "Prefix of the output files");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OutName, 1, 6, 1, 2);

	label = gtk_label_new("Start index at");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 2, 3, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(1, 0, 99999999, 1, 10, 0);
	m_OutStart = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_OutStart, "Ordinal number of the first output file");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_OutStart), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OutStart, 1, 2, 2, 3);

	label = gtk_label_new("Step by");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 2, 3, 2, 3, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(1, 1, 10000000, 1, 10, 0);
	m_OutStep = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_OutStep, "Increment of ordinal numbers");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_OutStep), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OutStep, 3, 4, 2, 3);

	label = gtk_label_new("Digits");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 4, 5, 2, 3, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(1, 1, 8, 1, 1, 0);
	m_OutDigits = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_OutDigits, "Number of digits used for ordinal numbers in output file names");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_OutDigits), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OutDigits, 5, 6, 2, 3);

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

CMergeDlg::~CMergeDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_free(m_FileName);
}

void CMergeDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMergeDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMergeDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MERGE_FRAMES);
		return false;
	}
	return true;
}


bool CMergeDlg::OnCloseQuery()
{
	const gchar *dir = gtk_entry_get_text(GTK_ENTRY(m_OutPath));
	if (dir[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a target directory path.");
		return false;
	}
	if (!g_file_test(dir, G_FILE_TEST_IS_DIR)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A folder \"%s\" does not exist.  Do you want to create it?",
			dir);
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirmation");
		gtk_dialog_add_button(GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(GTK_DIALOG (dialog), "_Create", GTK_RESPONSE_ACCEPT);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
		int response = gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (response != GTK_RESPONSE_ACCEPT)
			return false;
		if (!force_directory(dir)) {
			gchar *msg = g_strdup_printf("Failed to create the directory: %s", dir);
			ShowError(GTK_WINDOW(m_pDlg), msg);
			g_free(msg);
			return false;
		}
	}
	return true;
}

void CMergeDlg::Execute()
{
	int i, res;
	char msg[256];
	gchar *path, *name;

	g_free(m_FileName);
	m_FileName = NULL;

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	// Restore last parameters
	if (g_Project->GetBool("MergeDlg", "Single", false))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SingleBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MultiBtn), true);
	i = g_Project->GetInt("MergeDlg", "MaxFrames", 5);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MaxFramesEdit), i);
	i = g_Project->GetInt("MergeDlg", "MaxTime", 300);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MaxTimeEdit), i);
	i = g_Project->GetInt("MergeDlg", "MinFrames", 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MinFramesEdit), i);
	path = g_Project->GetStr("MergeDlg", "Folder", "");
	if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR))
		path = CConfig::GetStr("MergeDlg", "Folder", NULL);
	gtk_entry_set_text(GTK_ENTRY(m_OutPath), (path ? path : ""));
	g_free(path);
	name = g_Project->GetStr("MergeDlg", "File", "merge");
	gtk_entry_set_text(GTK_ENTRY(m_OutName), (name ? name : ""));
	g_free(name);
	i = g_Project->GetInt("MergeDlg", "Start", 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_OutStart), i);
	i = g_Project->GetInt("MergeDlg", "Step", 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_OutStep), i);
	i = g_Project->GetInt("MergeDlg", "Digits", 3);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_OutDigits), i);

	// Execute the dialog
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	// Prepare list of files
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
	if (!m_FileList)
		return;

	// Sort file by Julian date
	m_FileList = g_list_sort(m_FileList, (GCompareFunc)CompareFiles);

	// Parameters
	m_Single = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SingleBtn))!=0;
	g_Project->SetBool("MergeDlg", "Single", m_Single);
	m_MaxFrames = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MaxFramesEdit));
	g_Project->SetInt("MergeDlg", "MaxFrames", m_MaxFrames);
	m_MaxTime = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MaxTimeEdit));
	g_Project->SetInt("MergeDlg", "MaxTime", m_MaxTime);
	m_MinFrames = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MinFramesEdit));
	g_Project->SetInt("MergeDlg", "MinFrames", m_MinFrames);

	// Output file names
	const char *prefix = gtk_entry_get_text(GTK_ENTRY(m_OutName));
	g_Project->SetStr("MergeDlg", "File", prefix);
	const char *dirpath = gtk_entry_get_text(GTK_ENTRY(m_OutPath));
	g_Project->SetStr("MergeDlg", "Folder", dirpath);
	CConfig::SetStr("MergeDlg", "Folder", dirpath);
	gchar *fname = (gchar*)g_malloc((strlen(prefix)+12)*sizeof(gchar));
	strcpy(fname, prefix);
	if (!m_Single)
		strcat(fname, ".%0*d");
	strcat(fname, "." FILE_EXTENSION_FITS);
	m_FileName = g_build_filename(dirpath, fname, NULL);
	g_free(fname);

	// Output file counter
	m_Start = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_OutStart));
	g_Project->SetInt("MergeDlg", "Start", m_Start);
	m_Step = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_OutStep));
	g_Project->SetInt("MergeDlg", "Step", m_Step);
	m_Digits = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_OutDigits));
	g_Project->SetInt("MergeDlg", "Digits", m_Digits);
	
	// Process files
	CProgressDlg pDlg(m_pParent, "Merging frames");
	pDlg.SetMinMax(0, g_list_length(m_FileList));
	res = pDlg.Execute(ExecuteProc, this);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		ShowError(m_pParent, msg, true);
		cmpack_free(msg);
	} else
	if (m_InFiles==0) {
		ShowError(m_pParent, "No file was successfully built.", true);
	} else {
		sprintf(msg, "%d input frame(s) were successfully processed,\n"
			"%d input frame(s) were skipped,\n%d output frame(s) were build.", 
			m_InFiles, m_AllFiles-m_InFiles, m_OutFiles);
		ShowInformation(m_pParent, msg, true);
	}
	g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
	g_list_free(m_FileList);
	m_FileList = NULL;
	g_free(m_FileName);
	m_FileName = NULL;

	g_Project->Save();
}

void CMergeDlg::button_clicked(GtkWidget *button, CMergeDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CMergeDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==m_OptionsBtn)
		EditPreferences();
	else if (pBtn==m_MultiBtn || pBtn==m_SingleBtn)
		UpdateControls();
	else if (pBtn==m_PathBtn)
		ChangePath();
}

void CMergeDlg::UpdateControls(void)
{
	bool split;

	split = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MultiBtn))!=0;
	gtk_widget_set_sensitive(m_MaxFramesLabel, split);
	gtk_widget_set_sensitive(m_MaxFramesEdit, split);
	gtk_widget_set_sensitive(m_MaxFramesUnit, split);
	gtk_widget_set_sensitive(m_MaxTimeLabel, split);
	gtk_widget_set_sensitive(m_MaxTimeEdit, split);
	gtk_widget_set_sensitive(m_MaxTimeUnit, split);
	gtk_widget_set_sensitive(m_MinFramesLabel, split);
	gtk_widget_set_sensitive(m_MinFramesEdit, split);
	gtk_widget_set_sensitive(m_MinFramesUnit, split);
	gtk_widget_set_sensitive(m_OutStart, split);
	gtk_widget_set_sensitive(m_OutStep, split);
	gtk_widget_set_sensitive(m_OutDigits, split);
}

int CMergeDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CMergeDlg*)userdata)->ProcessFiles(sender);
}

int CMergeDlg::ProcessFiles(CProgressDlg *sender)
{
	int frameid, frames, res, first_status, frame_status, index;
	char *tpath, *fbase, *filename, msg[128];
	double first_jd, frame_jd;
	GtkTreePath *frame_path, *first_path;
	GList *node, *last;
	CPhotometryProc phot;
	CCombineProc combine;

	m_AllFiles = m_InFiles = m_OutFiles = 0;
	sender->Print("------ Merge frames ------");

	if (m_Single) {
		// All input frames into a single output frames
		res = combine.Open(sender, m_FileName, 0);
		if (res==0) {
			for (node = m_FileList; node != NULL; node = node->next) {
				frame_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
				if (frame_path) {
					frame_status = g_Project->GetState(frame_path);
					if (frame_status & CFILE_MATCHING) {
						frameid = g_Project->GetFrameID(frame_path);
						tpath = g_Project->GetPhotFileName(frame_path);
						sender->SetFileName(tpath);
						g_free(tpath);
						sender->SetProgress(m_InFiles++);
						sprintf(msg, "Frame #%d:", frameid);
						sender->Print(msg);
						res = combine.Add(frame_path);
					}
					gtk_tree_path_free(frame_path);
				}
				if (res!=0 || sender->Cancelled()) 
					break;
			}
			if (res==0 && !sender->Cancelled()) {
				sender->SetTitle("Making output file");
				fbase = g_filename_display_basename(m_FileName);
				sender->SetFileName(fbase);
				g_free(fbase);
				res = combine.Close();
			}
			m_OutFiles = 1;
			m_AllFiles = m_InFiles;
		}
	} else {
		// Split frames to groups and process them independently
		filename = (gchar*)g_malloc((strlen(m_FileName)+16)*sizeof(gchar));
		GList *first = m_FileList;
		res = 0;
		index = m_Start;
		for (node=first; node != NULL; node = node->next) 
			m_AllFiles++;
		while (first && res==0 && !sender->Cancelled()) {
			first_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)first->data);
			first_status = g_Project->GetState(first_path);
			if (first_status & CFILE_MATCHING) {
				first_jd = g_Project->GetJulDate(first_path);
				frames = 1;
				for (last=first->next; frames<m_MaxFrames && last!=NULL; last=last->next) {
					frame_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)last->data);
					if (frame_path) {
						frame_jd = g_Project->GetJulDate(frame_path);
						frame_status = g_Project->GetState(frame_path);
						if (frame_status & CFILE_MATCHING) {
							if (frame_jd-first_jd > ((double)m_MaxTime)/3600/24) 
								break;
							frames++;
						}
						gtk_tree_path_free(frame_path);
					}
				}
				if (frames >= m_MinFrames) {
					sprintf(filename, m_FileName, m_Digits, index);
					res = combine.Open(sender, filename, index);
					if (res==0) {
						for (GList *node = first; node!=NULL && node!=last; node=node->next) {
							frame_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
							if (frame_path) {
								frame_status = g_Project->GetState(frame_path);
								if (frame_status & CFILE_MATCHING) {
									frameid = g_Project->GetFrameID(frame_path);
									tpath = g_Project->GetPhotFileName(frame_path);
									sender->SetFileName(tpath);
									g_free(tpath);
									sender->SetProgress(m_InFiles++);
									sprintf(msg, "Frame #%d:", frameid);
									sender->Print(msg);
									res = combine.Add(frame_path);
								}
								gtk_tree_path_free(frame_path);
							}
							if (res!=0 || sender->Cancelled())
								break;
						}
						if (res==0 && !sender->Cancelled()) {
							sender->SetTitle("Making output file");
							fbase = g_filename_display_basename(filename);
							sender->SetFileName(fbase);
							g_free(fbase);
							res = combine.Close();
						}
						m_OutFiles++;
					}
					first = (last!=NULL ? last->prev : NULL);
					index += m_Step;
				}
			}
			gtk_tree_path_free(first_path);
			if (first)
				first = first->next;
		}
		g_free(filename);
	}
	if (res==0) {
		if (sender->Cancelled())
			sprintf(msg, "Cancelled at the user's request");
		else
			sprintf(msg, "====== %d processed, %d built ======", m_InFiles, m_OutFiles);
		sender->Print(msg);
	} else {
		char *aux = cmpack_formaterror(res);
		sender->Print(aux);
		cmpack_free(aux);
	}
	
	return res;
}

int CMergeDlg::CompareFiles(GtkTreeRowReference *ref1, GtkTreeRowReference *ref2)
{
	int frame0 = 0, frame1 = 0;
	double jd0 = 0, jd1 = 0;
	
	GtkTreePath *path1 = gtk_tree_row_reference_get_path(ref1);
	if (path1) {
		jd0 = g_Project->GetJulDate(path1);
		frame0 = g_Project->GetFrameID(path1);
		gtk_tree_path_free(path1);
	}
	GtkTreePath *path2 = gtk_tree_row_reference_get_path(ref2);
	if (path2) {
		jd1 = g_Project->GetJulDate(path2);
		frame1 = g_Project->GetFrameID(path2);
		gtk_tree_path_free(path2);
	}

	if (jd0 > jd1)
		return 1;
	else if (jd0 < jd1)
		return -1;
	else 
		return (frame0 - frame1);
}

void CMergeDlg::ChangePath(void)
{
	const gchar *dirname = gtk_entry_get_text(GTK_ENTRY(m_OutPath));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select output directory",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		gtk_entry_set_text(GTK_ENTRY(m_OutPath), path);
		g_free(path);
	}
	gtk_widget_destroy(pPathDlg);
}

void CMergeDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_MERGE_FRAMES);
}
