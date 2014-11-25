/**************************************************************

varfind_dlg.cpp (C-Munipack project)
The 'Find variables' dialog
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

#include "utils.h"
#include "configuration.h"
#include "main.h"
#include "makevarfind_dlg.h"
#include "varfindfile_dlg.h"
#include "findvariables_dlg.h"
#include "ctxhelp.h"
#include "project_dlg.h"

//-------------------------   MAKE VARFIND DIALOG   --------------------------------

CMakeVarFindDlg::CMakeVarFindDlg(GtkWindow *pParent):m_pParent(pParent), m_Reference(false), 
	m_Import(false)
{
	GtkWidget *vbox, *bbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Find variables", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, 
		"Use the entered values and continue");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("varfind");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_size_request(vbox, 600, -1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Process
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Include all frames in the current project");
	g_signal_connect(G_OBJECT(m_AllBtn), "toggled", G_CALLBACK(toggled), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Include frames that are selected in the main window");
	g_signal_connect(G_OBJECT(m_SelBtn), "toggled", G_CALLBACK(toggled), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>External file</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_ImportBtn = gtk_check_button_new_with_label("Import data from an external file");
	gtk_widget_set_tooltip_text(m_ImportBtn, "Import data from an external file saved before by the Find variables tool.");
	g_signal_connect(G_OBJECT(m_ImportBtn), "toggled", G_CALLBACK(toggled), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_ImportBtn, TRUE, TRUE, 0);
	
	m_PathBox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(vbox), m_PathBox, TRUE, TRUE, 0);
	m_PathLabel = gtk_label_new("Path:");
	gtk_box_pack_start(GTK_BOX(m_PathBox), m_PathLabel, FALSE, TRUE, 0);
	m_Path = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Path, "Path to the source file");
	gtk_widget_set_size_request(m_Path, 360, -1);
	gtk_box_pack_start(GTK_BOX(m_PathBox), m_Path, TRUE, TRUE, 0);
	m_PathBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_PathBtn, "Browse for file in a separate dialog");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(m_PathBox), m_PathBtn, FALSE, TRUE, 0);
		
	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Options
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), bbox);
	m_OptionsBtn = gtk_button_new_with_label("Options");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_OptionsBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMakeVarFindDlg::~CMakeVarFindDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CMakeVarFindDlg::Execute(void)
{
	char *fpath, *fname, *path;
	const char *cpath;

	// Restore last settings
	m_Import = g_Project->GetBool("VarFind", "Import");
	fpath = g_Project->GetStr("Output", "Folder", NULL);
	fname = g_Project->GetStr("VarFind", "FileName", NULL);
	if (fpath && fname) {
		path = g_build_filename(fpath, fname, NULL);
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
		g_free(path);
	}
	g_free(fpath);
	g_free(fname);

	m_Reference = g_Project->GetReferenceType()!=REF_UNDEFINED;
	if (!m_Reference) {
		// Allow import only
		gtk_widget_set_sensitive(m_SelBtn, FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), FALSE);
		gtk_widget_set_sensitive(m_AllBtn, FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ImportBtn), TRUE);
	} else {
		// Process frames or import data
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		gtk_widget_set_sensitive(m_AllBtn, TRUE);
		gtk_widget_set_sensitive(m_SelBtn, gtk_tree_selection_count_selected_rows(pSel)>0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ImportBtn), m_Import);
		if (gtk_tree_selection_count_selected_rows(pSel)>1)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), TRUE);
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), TRUE);
	}
	UpdateControls();

	// Show the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	g_Project->SetBool("VarFind", "Import", m_Import);
	if (m_Import) {
		// Save settings
		cpath = gtk_entry_get_text(GTK_ENTRY(m_Path));
		path = g_path_get_dirname(cpath);
		fname = g_path_get_basename(cpath);
		g_Project->SetStr("Output", "Folder", path);
		g_Project->SetStr("VarFind", "FileName", fname);
		g_free(path);
		g_free(fname);

		GError *error = NULL;
		CVarFindFileDlg *dlg = new CVarFindFileDlg();
		if (!dlg->Load(cpath, &error)) {
			if (error) {
				ShowError(m_pParent, error->message, true);
				g_error_free(error);
			}
			delete dlg;
			return;
		}
		dlg->Show();
	} else {
		CVarFindDlg *dlg = new CVarFindDlg();
		bool selected_frames = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SelBtn))!=0;
		if (!dlg->Open(m_pParent, selected_frames, g_Project->SelectionList())) {
			delete dlg;
			return;
		}
		dlg->Show();
	}
}

void CMakeVarFindDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMakeVarFindDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMakeVarFindDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		if (m_Import) {
			const char *path = gtk_entry_get_text(GTK_ENTRY(m_Path));
			if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
				ShowError(GTK_WINDOW(m_pDlg), "Please, click the Browse button and select a file.");
				return false;
			} 
			if (FileType(path)!=TYPE_VARFIND) {
				ShowError(GTK_WINDOW(m_pDlg), "Selected file is not a frame-set file.");
				return false;
			}
		}
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_FIND_VARIABLES);
		return false;
	}
	return true;
}

void CMakeVarFindDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_AllBtn, !m_Import);
	gtk_widget_set_sensitive(m_SelBtn, !m_Import);
	gtk_widget_set_sensitive(m_PathLabel, m_Import);
	gtk_widget_set_sensitive(m_Path, m_Import);
	gtk_widget_set_sensitive(m_PathBtn, m_Import);
}

void CMakeVarFindDlg::toggled(GtkWidget *widget, CMakeVarFindDlg *pMe)
{
	pMe->OnToggled(widget);
}

void CMakeVarFindDlg::OnToggled(GtkWidget *widget)
{
	if (widget==m_ImportBtn) {
		m_Import = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))!=0;
		if (!m_Reference && !m_Import)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
		UpdateControls();
	} else
	if (widget==m_SelBtn || widget==m_AllBtn) {
		m_Import = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))==0;
		UpdateControls();
	}
}

void CMakeVarFindDlg::button_clicked(GtkWidget *widget, CMakeVarFindDlg *pMe)
{
	pMe->OnButtonClicked(widget);
}

void CMakeVarFindDlg::OnButtonClicked(GtkWidget *widget)
{
	if (widget==m_OptionsBtn)
		EditPreferences();
	else if (widget==m_PathBtn) 
		ChangeFilePath();
}

void CMakeVarFindDlg::ChangeFilePath(void)
{
	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select file",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	const gchar *fpath = gtk_entry_get_text(GTK_ENTRY(m_Path));
	if (!gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pPathDlg), fpath)) {
		gchar *dir = g_path_get_dirname(fpath);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dir);
		g_free(dir);
	}
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pPathDlg));
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
		g_free(path);
		UpdateControls();
	}
	gtk_widget_destroy(pPathDlg);
}

void CMakeVarFindDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_FIND_VARIABLES);
}
