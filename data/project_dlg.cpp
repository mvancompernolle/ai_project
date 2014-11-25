/**************************************************************

project_dlg.cpp (C-Munipack project)
Project settings dialog
Copyright (C) 2012 David Motl, dmotl@volny.cz

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
#include "profile.h"
#include "project_dlg.h"
#include "main.h"
#include "project.h"
#include "ctxhelp.h"
#include "profile_editor.h"
#include "helper_dlgs.h"
#include "profiles_dlg.h"

//-------------------   NEW PROJECT DIALOG   ------------------------------

//
// Constructor
//
CNewProjectDlg::CNewProjectDlg(GtkWindow *pParent):m_Profiles(NULL), m_ParentDir(NULL), 
	m_FilePath(NULL), m_ProfilePath(NULL)
{
	GtkWidget *scrwnd;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("New project", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("newproject");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Project name and location
	GtkWidget *tbox = gtk_table_new(2, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new("Project name");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_Name = gtk_entry_new_with_max_length(MAX_PROJECT_NAME);
	gtk_widget_set_tooltip_text(m_Name, "Name of a new project, it must consist of characters allowed in a file name only.");
	gtk_table_attach(GTK_TABLE(tbox), m_Name, 1, 2, 0, 1, 
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_Name), "changed", G_CALLBACK(name_changed), this);
	label = gtk_label_new("Location");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_Path = gtk_entry_new_with_max_length(MAX_PROJECT_NAME);
	gtk_widget_set_tooltip_text(m_Path, "Path to the directory where a new project shall be saved to.");
	gtk_table_attach(GTK_TABLE(tbox), m_Path, 1, 2, 1, 2, 
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_Path), "changed", G_CALLBACK(path_changed), this);
	m_PathBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_PathBtn, "Change target directory");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), m_PathBtn, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	// Project settings
	label = gtk_label_new("Load initial project settings from:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_TreeView = gtk_tree_view_new();
	gtk_widget_set_tooltip_text(m_TreeView, "Select a profile to specify initial settings for a new project");
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_TreeView), FALSE);
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_TreeView), col);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", TCOL_PIXBUF);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", TCOL_CAPTION);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_TreeView);
	gtk_widget_set_size_request(scrwnd, 480, 400);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, TRUE, TRUE, 4);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tv_selection_changed), this);

	// Options
	GtkWidget *bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), bbox);
	m_OptionsBtn = gtk_button_new_with_label("Edit profiles");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Manage user-defined profiles");
	gtk_box_pack_start(GTK_BOX(bbox), m_OptionsBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CNewProjectDlg::~CNewProjectDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	g_free(m_FilePath); 
	g_free(m_ProfilePath);
	g_free(m_ParentDir);
}

bool CNewProjectDlg::Execute()
{
	// Project name is empty
	gtk_entry_set_text(GTK_ENTRY(m_Name), "");

	// Restore last folder
	gchar *path = CConfig::GetStr("Projects", "Folder", NULL);
	if (path && *path!='\0' && g_file_test(path, G_FILE_TEST_IS_DIR)) 
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
	else {
		gchar *defpath = g_build_filename(get_user_data_dir(), "Projects", NULL);
		if (force_directory(defpath))
			gtk_entry_set_text(GTK_ENTRY(m_Path), defpath);
		g_free(defpath);
	}
	g_free(path);
	
	// Update profiles, select most recently used profile
	g_free(m_ProfilePath);
	m_ProfilePath = CConfig::GetStr("NewProject", "LastProfile");
	UpdateProfiles();
	
	// Run the dialog
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	// Save selected folder and profile
	CConfig::SetStr("Projects", "Folder", m_ParentDir ? m_ParentDir : "");
	return true;
}

void CNewProjectDlg::UpdateControls(void)
{
	GtkTreeIter iter, parent;
	GtkTreeModel *model;
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(m_Name));
	const gchar *path = gtk_entry_get_text(GTK_ENTRY(m_Path));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	bool ok = (name && name[0]!='\0') && (path && path[0]!='\0') &&
		gtk_tree_selection_get_selected(selection, &model, &iter) && 
		gtk_tree_model_iter_parent(model, &parent, &iter);

	GtkWidget *ok_btn = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(ok_btn, ok);
}

void CNewProjectDlg::response_dialog(GtkDialog *pDlg, gint response_id, CNewProjectDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CNewProjectDlg::OnResponseDialog(gint response_id)
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_NEW_PROJECT);
		return false;
	}
	return true;
}

void CNewProjectDlg::button_clicked(GtkWidget *pButton, CNewProjectDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CNewProjectDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_PathBtn) 
		ChangeDirPath();
	else if (pButton == m_OptionsBtn) {
		CProfilesDlg pDlg(GTK_WINDOW(m_pDlg));
		pDlg.Execute();
		UpdateProfiles();
		UpdateControls();
	}
}

void CNewProjectDlg::tv_selection_changed(GtkTreeSelection *pWidget, CNewProjectDlg *pMe)
{
	pMe->OnTreeViewSelectionChanged(pWidget);
}

void CNewProjectDlg::OnTreeViewSelectionChanged(GtkTreeSelection *pWidget)
{
	gchar *fpath;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(pWidget, &model, &iter)) {
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &fpath, -1);
		if (fpath) {
			g_free(m_ProfilePath);
			m_ProfilePath = fpath;
			fpath = NULL;
		}
		g_free(fpath);
		UpdateControls();
	}
}

bool CNewProjectDlg::OnCloseQuery(void)
{
	char buf[256];
	GtkTreeModel *model;
	gchar *profile;
	GtkTreeIter iter;
	gboolean userDefined;

	g_free(m_FilePath);
	m_FilePath = NULL;
	g_free(m_ParentDir);
	m_ParentDir = NULL;
	
	// Check project name
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(m_Name));
	if (name[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a project name.");
		return false;
	}
	if (!CheckFileBaseName(name)) {
		ShowError(GTK_WINDOW(m_pDlg), "A project name must contain characters that are allowed in file name.\nDo not use: / \\ ? % * : | \" < and >");
		return false;
	}

	// Check that the selected parent directory exists
	m_ParentDir = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Path)));
	if (m_ParentDir[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a target directory path.");
		return false;
	}

	// Profile
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) 
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &profile, TCOL_USERDEFINED, &userDefined, -1);
	if (userDefined) {
		if (!profile) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select one of user-defined or default profiles to specify initial settings for the new project.");
			return false;
		}
		GError *error = NULL;
		if (!m_Profile.Import(profile, &error)) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
			g_free(profile);
			return false;
		}
		CConfig::SetStr("NewProject", "LastProfile", profile);
		g_free(profile);
	} else {
		if (!profile) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select one of user-defined or default profiles to specify initial settings for the new project.");
			return false;
		}
		m_Profile = CProfile::DefaultProfile(profile);
		CConfig::SetStr("NewProject", "LastProfile", profile);
		g_free(profile);
	}
	
	// Check that the selected parent directory exists
	if (!g_file_test(m_ParentDir, G_FILE_TEST_IS_DIR)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A folder \"%s\" does not exist.  Do you want to create it?",
			m_ParentDir);
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirmation");
		gtk_dialog_add_button(GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(GTK_DIALOG (dialog), "_Create", GTK_RESPONSE_ACCEPT);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
		int response = gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (response != GTK_RESPONSE_ACCEPT) {
			// Action cancelled
			return false;
		}
		if (!force_directory(m_ParentDir)) {
			gchar *msg = g_strdup_printf("Failed to create the directory: %s", m_ParentDir);
			ShowError(GTK_WINDOW(m_pDlg), msg);
			g_free(msg);
			return false;
		}
	}

	// Make a new project directory
	gchar *project_dir = g_build_filename(m_ParentDir, name, NULL);
	if (!force_directory(project_dir)) {
		gchar *msg = g_strdup_printf("Failed to create the directory: %s", project_dir);
		ShowError(GTK_WINDOW(m_pDlg), msg);
		g_free(msg);
		g_free(project_dir);
		return false;
	}
	
	// Make project file path
	sprintf(buf, "%s.%s", name, FILE_EXTENSION_PROJECT);
	m_FilePath = g_build_filename(project_dir, buf, NULL);
	g_free(project_dir);

	// Check if the project does not exist
	return ConfirmOverwrite(GTK_WINDOW(m_pDlg), m_FilePath);
}


//
// Change parent directory
//
void CNewProjectDlg::ChangeDirPath(void)
{
	const char *dirname = gtk_entry_get_text(GTK_ENTRY(m_Path));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select path",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		char *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
		g_free(path);
	}
	gtk_widget_destroy(pPathDlg);
}


//
// Rebuild list of profiles
//
void CNewProjectDlg::UpdateProfiles(void)
{
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	m_Profiles = gtk_tree_store_new(TNCOLS, GDK_TYPE_PIXBUF, GTK_TYPE_STRING, GTK_TYPE_STRING, G_TYPE_BOOLEAN);

	GSList *ul = GetProfileList(UserProfilesDir());
	if (ul) {
		bool first = true;
		GtkTreeIter parent;
		for (GSList *ptr=ul; ptr!=NULL; ptr=ptr->next) {
			const gchar *fpath = (gchar*)ptr->data;
			CProfile profile;
			if (profile.Import(fpath)) {
				if (first) {
					gtk_tree_store_append(m_Profiles, &parent, NULL);
					gtk_tree_store_set(m_Profiles, &parent, TCOL_CAPTION, "User-defined profiles", -1);
					gchar *icon = get_icon_file("profile16");
					if (icon) {
						gtk_tree_store_set(m_Profiles, &parent, TCOL_PIXBUF, 
							gdk_pixbuf_new_from_file(icon, NULL), -1);
						g_free(icon);
					}
					first = false;
				}
				GtkTreeIter child;
				gtk_tree_store_append(m_Profiles, &child, &parent);
				gchar *basename = g_path_get_basename(fpath);
				gchar *fname = StripFileExtension(basename);
				gtk_tree_store_set(m_Profiles, &child, TCOL_FILEPATH, fpath, TCOL_CAPTION, fname, TCOL_USERDEFINED, TRUE, -1);
				g_free(basename);
				g_free(fname);
				gchar *icon = get_icon_file(ProjectTypeIcon(profile.ProjectType()));
				if (icon) {
					gtk_tree_store_set(m_Profiles, &child, TCOL_PIXBUF, gdk_pixbuf_new_from_file(icon, NULL), -1);
					g_free(icon);
				}
			}
		}
		ProfileListFree(ul);
	}
	
	GSList *dl = DefaultProfileList();
	if (dl) {
		bool first = true;
		GtkTreeIter parent;
		for (GSList *ptr=dl; ptr!=NULL; ptr=ptr->next) {
			const gchar *name = (gchar*)ptr->data;
			CProfile profile = CProfile::DefaultProfile(name);
			if (profile.ProjectType()!=EndOfProjectTypes) {
				if (first) {
					gtk_tree_store_append(m_Profiles, &parent, NULL);
					gtk_tree_store_set(m_Profiles, &parent, TCOL_CAPTION, "Predefined profiles", -1);
					gchar *icon = get_icon_file("default");
					if (icon) {
						gtk_tree_store_set(m_Profiles, &parent, TCOL_PIXBUF, 
							gdk_pixbuf_new_from_file(icon, NULL), -1);
						g_free(icon);
					}
					first = false;
				}
				GtkTreeIter child;
				gtk_tree_store_append(m_Profiles, &child, &parent);
				gtk_tree_store_set(m_Profiles, &child, TCOL_FILEPATH, name, TCOL_CAPTION, name, TCOL_USERDEFINED, FALSE, -1);
				gchar *icon = get_icon_file(ProjectTypeIcon(profile.ProjectType()));
				if (icon) {
					gtk_tree_store_set(m_Profiles, &child, TCOL_PIXBUF, gdk_pixbuf_new_from_file(icon, NULL), -1);
					g_free(icon);
				}
			}
		}
		ProfileListFree(dl);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), GTK_TREE_MODEL(m_Profiles));
	gtk_tree_view_expand_all(GTK_TREE_VIEW(m_TreeView));

	if (m_ProfilePath) {
		GtkTreePath *lastProfileNode = FindNode(m_ProfilePath, GTK_TREE_MODEL(m_Profiles), NULL);
		if (lastProfileNode) {
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
			gtk_tree_selection_select_path(selection, lastProfileNode);
			gtk_tree_path_free(lastProfileNode);
		}
	}
}

GtkTreePath *CNewProjectDlg::FindNode(const gchar *fpath, GtkTreeModel *model, GtkTreeIter *parent)
{
	GtkTreeIter iter;
	gchar *ipath;

	gboolean ok = gtk_tree_model_iter_children(model, &iter, parent);
	while (ok) {
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &ipath, -1);
		if (ipath) {
			// Leaf nodes -> profiles
			if (ComparePaths(fpath, ipath)==0) {
				g_free(ipath);
				return gtk_tree_model_get_path(model, &iter);
			}
		} else {
			// Branches -> call recursively
			GtkTreePath *retval = FindNode(fpath, model, &iter);
			if (retval)
				return retval;
		}
		g_free(ipath);
		ok = gtk_tree_model_iter_next(model, &iter);
	}
	return NULL;
}

//-------------------   SAVE PROJECT DIALOG   ------------------------------

CSaveProjectDlg::CSaveProjectDlg(GtkWindow *pParent):m_Path(NULL)
{
	GtkFileFilter *filters[2];

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Save project as...", pParent, GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// File filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*." FILE_EXTENSION_PROJECT);
	gtk_file_filter_set_name(filters[0], "C-Munipack projects");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[1]);

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);

	// Dialog icon
	gchar *icon = get_icon_file("saveproject");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);
}

CSaveProjectDlg::~CSaveProjectDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_free(m_Path);
}

void CSaveProjectDlg::response_dialog(GtkDialog *pDlg, gint response_id, CSaveProjectDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CSaveProjectDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Check file
		if (!OnCloseQuery())
			return false;
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_PROJECT);
		return false;
	}
	return true;
}

bool CSaveProjectDlg::OnCloseQuery(void)
{
	// Get file name, check extension
	gchar *str = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	gchar *fpath = SetFileExtension(str, FILE_EXTENSION_PROJECT);
	g_free(str);

	// Compare paths
	if (ComparePaths(g_Project->Path(), fpath)==0) {
		ShowError(GTK_WINDOW(m_pDlg), "Cannot make copy to itself");
		g_free(fpath);
		return false;
	}

	// Ask for confirmation if the file exists already
	if (!ConfirmOverwrite(GTK_WINDOW(m_pDlg), fpath)) {
		g_free(fpath);
		return false;
	}

	g_free(fpath);
	return true;
}

bool CSaveProjectDlg::Execute(void)
{
	char buf[MAX_PROJECT_NAME+32];

	g_free(m_Path);
	m_Path = NULL;

	gchar *dirname = g_path_get_dirname(g_Project->Path());
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), dirname);
	g_free(dirname);

	sprintf(buf, "%s.%s", g_Project->Name(), FILE_EXTENSION_PROJECT);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), buf);

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		m_Path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		return true;
	}
	return false;
}

//------------------------------   IMPORT PROJECT DIALOG   ------------------------------

CImportProjectDlg::CImportProjectDlg(GtkWindow *pParent):m_Path(NULL)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Import data from ...", pParent, 
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT, 
		GTK_STOCK_APPLY, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("importproject");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);
}

CImportProjectDlg::~CImportProjectDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_free(m_Path);
}

bool CImportProjectDlg::Execute(void)
{
	g_free(m_Path);
	m_Path = NULL;

	// Go to last folder
	gchar *folder = CConfig::GetStr("ImportDataDlg", "Folder", NULL);
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		m_Path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pDlg));
		CConfig::SetStr("ImportDataDlg", "Folder", m_Path);
		return m_Path!=NULL;
	}
	return FALSE;
}

void CImportProjectDlg::response_dialog(GtkDialog *pDlg, gint response_id, CImportProjectDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CImportProjectDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_IMPORT_PROJECT);
		return false;
	}
	return true;
}

//-------------------   IMPORT PROJECT DIALOG   ------------------------------

CImportProject2Dlg::CImportProject2Dlg(GtkWindow *pParent):m_ParentDir(NULL), 
	m_FilePath(NULL)
{
	GtkWidget *scrwnd;
	GtkTreeIter iter;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;

	m_Type = (tProjectType)CConfig::GetInt("Projects", "Type", PROJECT_REDUCE, 0, EndOfProjectTypes-1);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Import project as ...", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("importproject");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Project name and location
	GtkWidget *tbox = gtk_table_new(2, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new("Project name");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_Name = gtk_entry_new_with_max_length(MAX_PROJECT_NAME);
	gtk_widget_set_tooltip_text(m_Name, "Name of a new project, it must consist of characters allowed in a file name only.");
	gtk_table_attach(GTK_TABLE(tbox), m_Name, 1, 2, 0, 1, 
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("Location");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_Path = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Path, "Path to the directory where a new project shall be saved to.");
	gtk_table_attach(GTK_TABLE(tbox), m_Path, 1, 2, 1, 2, 
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_PathBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_PathBtn, "Change target directory");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), m_PathBtn, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	// Project types
	label = gtk_label_new("Project type");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_Types = gtk_list_store_new(TNCOLS, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	for (int i=0; i<EndOfProjectTypes; i++) {
		const gchar *caption = ProjectTypeCaption((tProjectType)i);
		if (caption) {
			gtk_list_store_append(GTK_LIST_STORE(m_Types), &iter);
			gtk_list_store_set(GTK_LIST_STORE(m_Types), &iter, TCOL_CAPTION, caption, TCOL_ID, i, -1);
			if (ProjectTypeIcon((tProjectType)i)) {
				gchar *icon = get_icon_file(ProjectTypeIcon((tProjectType)i));
				gtk_list_store_set(GTK_LIST_STORE(m_Types), &iter, TCOL_PIXBUF, 
					gdk_pixbuf_new_from_file(icon, NULL), -1);
				g_free(icon);
			}
		}
	}
	m_TypeView = gtk_tree_view_new();
	gtk_widget_set_tooltip_text(m_TypeView, "A project type according to the type of source frames and a kind of output data");
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_TypeView), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TypeView)), GTK_SELECTION_BROWSE);
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_TypeView), col);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", TCOL_PIXBUF);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", TCOL_CAPTION);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_TypeView);
	gtk_widget_set_size_request(scrwnd, 480, 240);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, TRUE, TRUE, 4);
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TypeView), GTK_TREE_MODEL(m_Types));

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CImportProject2Dlg::~CImportProject2Dlg()
{
	g_object_unref(m_Types);
	gtk_widget_destroy(m_pDlg);
	g_free(m_FilePath); 
	g_free(m_ParentDir);
}

bool CImportProject2Dlg::Execute()
{
	int id;
	gboolean ok;
	GtkTreeIter iter;

	// Select project type
	ok = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Types), &iter);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(m_Types), &iter, TCOL_ID, &id, -1);
		if (id==m_Type) {
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TypeView));
			gtk_tree_selection_select_iter(selection, &iter);
			break;
		}
		ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_Types), &iter);
	}
	
	// Restore last folder
	gchar *path = CConfig::GetStr("Projects", "Folder", NULL);
	if (path && *path!='\0' && g_file_test(path, G_FILE_TEST_IS_DIR)) 
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
	else {
		gchar *defpath = g_build_filename(get_user_data_dir(), "Projects", NULL);
		if (force_directory(defpath))
			gtk_entry_set_text(GTK_ENTRY(m_Path), defpath);
		g_free(defpath);
	}
	g_free(path);

	// Project name is empty
	gtk_entry_set_text(GTK_ENTRY(m_Name), "");

	// Run the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	// Save last type (last folder is saved in push button handler)
	CConfig::SetInt("Projects", "Type", m_Type);
	if (m_ParentDir) 
		CConfig::SetStr("Projects", "Folder", m_ParentDir);
	return true;
}

void CImportProject2Dlg::response_dialog(GtkDialog *pDlg, gint response_id, CImportProject2Dlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CImportProject2Dlg::OnResponseDialog(gint response_id)
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
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_NEW_PROJECT);
		return false;
	}
	return true;
}

void CImportProject2Dlg::button_clicked(GtkWidget *pButton, CImportProject2Dlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CImportProject2Dlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_PathBtn) 
		ChangeDirPath();
}

bool CImportProject2Dlg::OnCloseQuery(void)
{
	int type;
	char buf[256];
	GtkTreeModel *model;
	GtkTreeIter iter;

	g_free(m_FilePath);
	m_FilePath = NULL;
	g_free(m_ParentDir);
	m_ParentDir = NULL;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TypeView));
	if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
		ShowError(GTK_WINDOW(m_pDlg), "Please, select a project type.");
		return false;
	}
	gtk_tree_model_get(model, &iter, TCOL_ID, &type, -1);
	m_Type = (tProjectType)type;

	// Check project name
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(m_Name));
	if (name[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a project name.");
		return false;
	}
	if (!CheckFileBaseName(name)) {
		ShowError(GTK_WINDOW(m_pDlg), "A project name must contain characters that are allowed in file name.\nDo not use: / \\ ? % * : | \" < and >");
		return false;
	}

	// Check that the selected parent directory exists
	const gchar *parent_dir = gtk_entry_get_text(GTK_ENTRY(m_Path));
	if (parent_dir[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a target directory path.");
		return false;
	}
	if (!g_file_test(parent_dir, G_FILE_TEST_IS_DIR)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A folder \"%s\" does not exist.  Do you want to create it?",
			parent_dir);
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirmation");
		gtk_dialog_add_button(GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(GTK_DIALOG (dialog), "_Create", GTK_RESPONSE_ACCEPT);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
		int response = gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (response != GTK_RESPONSE_ACCEPT)
			return false;
		if (!force_directory(parent_dir)) {
			gchar *msg = g_strdup_printf("Failed to create the directory: %s", parent_dir);
			ShowError(GTK_WINDOW(m_pDlg), msg);
			g_free(msg);
			return false;
		}
	}

	// Make a new project directory
	gchar *project_dir = g_build_filename(parent_dir, name, NULL);
	if (!force_directory(project_dir)) {
		gchar *msg = g_strdup_printf("Failed to create the directory: %s", project_dir);
		ShowError(GTK_WINDOW(m_pDlg), msg);
		g_free(msg);
		g_free(project_dir);
		return false;
	}
	
	// Make project file path
	sprintf(buf, "%s.%s", name, FILE_EXTENSION_PROJECT);
	gchar *fpath = g_build_filename(project_dir, buf, NULL);

	// Check if the project does not exist
	if (!ConfirmOverwrite(GTK_WINDOW(m_pDlg), fpath)) {
		g_free(fpath);
		g_free(project_dir);
		return false;
	}
	g_free(project_dir);

	// Create project file
	m_FilePath = fpath;
	m_ParentDir = g_strdup(parent_dir);
	return true;
}

void CImportProject2Dlg::ChangeDirPath(void)
{
	const char *dirname = gtk_entry_get_text(GTK_ENTRY(m_Path));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select path",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		char *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
		g_free(path);
	}
	gtk_widget_destroy(pPathDlg);
}

//-------------------------   EDIT PROJECT WIDGET   ------------------------------

static int type_to_index(tProjectType type)
{
	return (type - PROJECT_REDUCE);
}

static tProjectType index_to_type(int index)
{
	if (index>=0)
		return (tProjectType)index;
	return EndOfProjectTypes;
}

CEditProject::CEditProject(CEditProjectDlg *pParent):m_pParent(pParent) 
{
	gchar *str = CConfig::GetStr("ProjectDlg", "LastPage");
	SetInitialPage(str_to_page(str, PAGE_PROJECT));
	g_free(str);
}

GtkWidget *CEditProject::DialogWidget(void) 
{
	return m_pParent->m_pDlg;
}

void CEditProject::OnPageChanged(tProfilePageId currentPage, tProfilePageId previousPage) 
{
	if (currentPage != EndOfProfilePages)
		CConfig::SetStr("ProjectDlg", "LastPage", page_to_str(currentPage));
}

void CEditProject::CreatePages(GSList **stack)
{
	// Project
	GtkWidget *tbox = CreatePage(PAGE_PROJECT, 0, "Project", NULL, FALSE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>General properties</b>");
	m_Type = add_combo_box(GTK_TABLE(tbox), 1, "Type", (AddComboFn)ProjectTypeCaption, 
		PROJECT_REDUCE, EndOfProjectTypes, "Select a project type according to the type of source frames and a kind of output data");
	m_Name = add_entry(GTK_TABLE(tbox), 2, "Name", MAX_PROJECT_NAME, "Name of the project, it must consist of characters allowed in a file name only");
	m_Path = add_static(GTK_TABLE(tbox), 3, "Path", PANGO_ELLIPSIZE_MIDDLE, "Path to the directory with the project files");
	gtk_widget_set_size_request(m_Name, 320, -1);

	add_label(GTK_TABLE(tbox), 5, "<b>Import/export project settings</b>");
	GtkWidget *bbox = add_button_box(GTK_TABLE(tbox), 6);
	m_LoadFromProfile = gtk_button_new_with_label("Load from profile");
	gtk_widget_set_tooltip_text(m_LoadFromProfile, "Load project settings from a profile");
	gtk_container_add(GTK_CONTAINER(bbox), m_LoadFromProfile);
	g_signal_connect(G_OBJECT(m_LoadFromProfile), "clicked", G_CALLBACK(button_clicked), this);
	m_LoadFromProject = gtk_button_new_with_label("Import from project");
	gtk_widget_set_tooltip_text(m_LoadFromProject, "Import project settings from another project");
	gtk_container_add(GTK_CONTAINER(bbox), m_LoadFromProject);
	g_signal_connect(G_OBJECT(m_LoadFromProject), "clicked", G_CALLBACK(button_clicked), this);
	m_SaveAsProfile = gtk_button_new_with_label("Save as profile");
	gtk_widget_set_tooltip_text(m_SaveAsProfile, "Save current project settings as a user-defined profile");
	gtk_container_add(GTK_CONTAINER(bbox), m_SaveAsProfile);
	g_signal_connect(G_OBJECT(m_SaveAsProfile), "clicked", G_CALLBACK(button_clicked), this);

	// Profile settings
	CEditProfileBase::CreatePages(stack);

	// Files and directories
	tbox = CreatePage(PAGE_FILES, 1, "Files and directories", NULL, FALSE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Calibration</b>");
	m_Bias = add_static(GTK_TABLE(tbox), 1, "Bias frame", PANGO_ELLIPSIZE_MIDDLE, "Path to the most recently used bias frame");
	m_Dark = add_static(GTK_TABLE(tbox), 2, "Dark frame", PANGO_ELLIPSIZE_MIDDLE, "Path to the most recently used dark frame");
	m_Flat = add_static(GTK_TABLE(tbox), 3, "Flat frame", PANGO_ELLIPSIZE_MIDDLE, "Path to the most recently used flat frame");
	add_label(GTK_TABLE(tbox), 5, "<b>Matching</b>");
	m_Ref = add_static(GTK_TABLE(tbox), 6, "Reference frame", PANGO_ELLIPSIZE_NONE, "Identification of the reference frame used for matching");
	m_Cat = add_static(GTK_TABLE(tbox), 7, "Catalogue file", PANGO_ELLIPSIZE_MIDDLE, "Path to the catalog file used for matching");
	gtk_widget_set_size_request(m_Bias, 280, -1);
}

void CEditProject::SetData(void)
{
	bool editable = !g_Project->isReadOnly();
	
	// Project
	gtk_entry_set_text(GTK_ENTRY(m_Name), g_Project->Name());
	if (g_Project->Path()) {
		gchar *dirname = g_path_get_dirname(g_Project->Path());
		gtk_label_set_text(GTK_LABEL(m_Path), dirname);
		g_free(dirname);
	} else 
		gtk_label_set_text(GTK_LABEL(m_Path), "------");
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_Type), type_to_index(m_ProjectType));
	gtk_widget_set_sensitive(GTK_WIDGET(m_Name), editable);
	gtk_widget_set_sensitive(GTK_WIDGET(m_Type), editable);
	gtk_widget_set_sensitive(GTK_WIDGET(m_LoadFromProfile), editable);
	gtk_widget_set_sensitive(GTK_WIDGET(m_LoadFromProject), editable);

	// Files and directories
	if (g_Project->GetOrigBiasFile()->Valid()) {
		gtk_label_set_text(GTK_LABEL(m_Bias), 
			g_Project->GetOrigBiasFile()->FullPath());
	} else 
		gtk_label_set_text(GTK_LABEL(m_Bias), "------");
	if (g_Project->GetOrigDarkFile()->Valid()) {
		gtk_label_set_text(GTK_LABEL(m_Dark), 
			g_Project->GetOrigDarkFile()->FullPath());
	} else
		gtk_label_set_text(GTK_LABEL(m_Dark), "------");
	if (g_Project->GetOrigFlatFile()->Valid()) {
		gtk_label_set_text(GTK_LABEL(m_Flat), 
			g_Project->GetOrigFlatFile()->FullPath());
	} else
		gtk_label_set_text(GTK_LABEL(m_Flat), "------");
	if (g_Project->GetReferenceType()==REF_FRAME &&
		g_Project->GetReferenceFrame()>=0) {
			char buf[256];
			sprintf(buf, "%d", g_Project->GetReferenceFrame());
			gtk_label_set_text(GTK_LABEL(m_Ref), buf);
	} else 
		gtk_label_set_text(GTK_LABEL(m_Ref), "------");
	if (g_Project->GetReferenceType()==REF_CATALOG_FILE && 
		g_Project->GetReferenceCatalog()->Valid()) {
			gtk_label_set_text(GTK_LABEL(m_Cat), 
				g_Project->GetReferenceCatalog()->FullPath());
	} else 
		gtk_label_set_text(GTK_LABEL(m_Cat), "------");

	// Profile settings
	CEditProfileBase::SetData();
}

bool CEditProject::IsPageVisible(tProfilePageId id)
{
	return (id==PAGE_PROJECT || id==PAGE_FILES || CEditProfileBase::IsPageVisible(id));
}

bool CEditProject::OnCloseQuery(void)
{
	if (m_Path && !m_ReadOnly) {
		if (m_currentPageId != EndOfProfilePages && !m_ReadOnly) {
			GError *error = NULL;
			if (!CheckPage(m_currentPageId, &error)) {
				ShowError(GTK_WINDOW(DialogWidget()), error->message);
				g_error_free(error);
				return false;
			}
		}

		GetData();

		GError *error = NULL;
		if (!CheckProfile(&error)) {
			if (error->code != m_currentPageId)
				ShowPage((tProfilePageId)error->code);
			ShowError(GTK_WINDOW(DialogWidget()), error->message);
			g_error_free(error);
			return false;
		}

		// Project name
		const gchar *name = gtk_entry_get_text(GTK_ENTRY(m_Name));
		if (StrCmp0(name, g_Project->Name())!=0) {
			GError *error = NULL;
			if (!g_Project->Rename(name, &error)) {
				if (error) {
					ShowError(GTK_WINDOW(DialogWidget()), error->message);
					g_error_free(error);
				}
				return false;
			}
		}
	}

	return true;
}

void CEditProject::Init() 
{
	char buf[MAX_PROJECT_NAME+512];

	sprintf(buf, "Project '%s'", g_Project->Name());
	SetPageTitle(PAGE_PROJECT, buf);

	SetProfile(*g_Project->Profile(), g_Project->isReadOnly());
}


//
// Push button activated
//
void CEditProject::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton==m_LoadFromProfile)
		LoadFromProfile();
	else if (pButton==m_SaveAsProfile)
		SaveAsProfile();
	else if (pButton==m_LoadFromProject)
		LoadFromProject();
	else 
		CEditProfileBase::OnButtonClicked(pButton);
}

//
// Project type changed
//
void CEditProject::OnComboBoxChanged(GtkWidget *pBox)
{
	if (pBox == m_Type) {
		tProjectType type = index_to_type(gtk_combo_box_get_active(GTK_COMBO_BOX(pBox)));
		if (type>=0 && type<EndOfProjectTypes && type!=m_Profile.ProjectType()) {
			m_Profile.SetProjectType(type);
			m_ProjectType = type;
			gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(m_FilteredPageList));
		}
	} else
		CEditProfileBase::OnComboBoxChanged(pBox);
}

//
// Load project settings from a profile
//
void CEditProject::LoadFromProfile(void)
{
	CProfile profile;

	CLoadProfileDlg dlg(GTK_WINDOW(DialogWidget()));
	if (dlg.Execute(profile)) {
		m_Profile = profile;
		m_ProjectType = m_Profile.ProjectType();
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(m_FilteredPageList));
		m_Changed = true;
		m_Updating = true;
		SetData();
		m_Updating = false;
	}

	UpdateControls();
}


//
// Import project settings from a project
//
void CEditProject::LoadFromProject(void)
{
	CProfile profile;

	CImportProfileDlg dlg(GTK_WINDOW(DialogWidget()), m_ProjectType);
	if (dlg.Execute(profile)) {
		m_Profile = profile;
		m_ProjectType = m_Profile.ProjectType();
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(m_FilteredPageList));
		m_Changed = true;
		m_Updating = true;
		SetData();
		m_Updating = false;
	}

	UpdateControls();
}


//
// Save project settings
//
void CEditProject::SaveAsProfile(void)
{
	CSaveProfileDlg dlg(GTK_WINDOW(DialogWidget()), CSaveProfileDlg::SAVE_PROJECT_AS_PROFILE);
	gchar *fpath = dlg.Execute("New profile");
	if (fpath) {
		GError *error = NULL;
		if (!m_Profile.Export(fpath, &error)) {
			if (error) {
				ShowError(GTK_WINDOW(DialogWidget()), error->message);
				g_error_free(error);
			}
		}
		g_free(fpath);
	}
}

//-------------------------   EDIT PROJECT DIALOG   --------------------------------

CEditProjectDlg::CEditProjectDlg(GtkWindow *pParent)
{
	m_Widget = new CEditProject(this);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Project settings", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("preferences");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), m_Widget->GetHandle(), TRUE, TRUE, 0);
	
	// Dialog size
	GtkRequisition widget_req = m_Widget->GetSizeRequest();
	GtkRequisition aarea_req, dlg_req;
	gtk_widget_size_request(GTK_DIALOG(m_pDlg)->action_area, &aarea_req);
	dlg_req.width = widget_req.width;
	dlg_req.height = widget_req.height + aarea_req.height + 56;
	GdkScreen *scr = gdk_screen_get_default();
	GdkRectangle rc;
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0) {
		int maxw = RoundToInt(0.9*rc.width);
		if (dlg_req.width > maxw)
			dlg_req.width = maxw;
		int maxh = RoundToInt(0.8*rc.height);
		if (dlg_req.height > maxh)
			dlg_req.height = maxh;
	}
	gtk_window_set_default_size(GTK_WINDOW(m_pDlg), dlg_req.width, dlg_req.height);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CEditProjectDlg::~CEditProjectDlg()
{
	gtk_widget_destroy(m_pDlg);
	delete m_Widget;
}

bool CEditProjectDlg::Execute(tProfilePageId initPage)
{
	if (initPage != EndOfProfilePages)
		m_Widget->SetInitialPage(initPage);

	// Execute the dialog
	m_Widget->Init();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT) 
		return false;

	// Apply changes
	g_Project->SetProfile(*m_Widget->Profile());
	g_Project->Save();
	g_Project->ClearThumbnails();
	return true;
}

void CEditProjectDlg::response_dialog(GtkWidget *pDlg, gint response_id, CEditProjectDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CEditProjectDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return m_Widget->OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_PROJECT);
		return false;
	}
	return true;
}
