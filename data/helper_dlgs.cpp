/**************************************************************

newproject_dlg.cpp (C-Munipack project)
New project dialog
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

#include "helper_dlgs.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"
#include "configuration.h"
#include "profiles_dlg.h"

enum tTypeColumdId
{
	TCOL_PIXBUF,
	TCOL_CAPTION,
	TCOL_FILEPATH,
	TCOL_USERDEFINED,
	TNCOLS
};

//-------------------   OPEN PROJECT DIALOG   ------------------------------

COpenDlg::COpenDlg(GtkWindow *pParent, bool open_file):m_OpenFile(open_file), m_Path(NULL)
{
	GtkFileFilter *filters[2];

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new((m_OpenFile ? "Open file" : "Open project"), pParent, 
		GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	g_signal_connect(G_OBJECT(m_pDlg), "update-preview", G_CALLBACK(update_preview), this);
	g_signal_connect(G_OBJECT(m_pDlg), "selection-changed", G_CALLBACK(selection_changed), this);

	// File filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*." FILE_EXTENSION_PROJECT);
	gtk_file_filter_set_name(filters[0], "C-Munipack projects");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), filters[1]);
	if (!open_file)
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), filters[0]);
	else
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), filters[1]);

	// Dialog icon
	gchar *icon = get_icon_file(open_file ? "openfile" : "openproject");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Preview widget
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(m_pDlg), 
		GTK_WIDGET(g_object_ref(m_Preview.Handle())));
}

COpenDlg::~COpenDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_free(m_Path);
}

void COpenDlg::response_dialog(GtkDialog *pDlg, gint response_id, COpenDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool COpenDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), (m_OpenFile ? IDH_OPEN_FILE : IDH_OPEN_PROJECT));
		return false;
	}
	return true;
}

bool COpenDlg::Execute(void)
{
	gchar *defpath = NULL;

	g_free(m_Path);
	m_Path = NULL;
	m_Preview.Clear();

	if (m_OpenFile) {
		gchar *path = g_Project->GetStr("OpenFile", "File", NULL);
		if (path)
			defpath = g_path_get_dirname(path);
		g_free(path);
	} else {
		gchar *path = CConfig::GetStr("Projects", "Last", NULL);
		if (path)
			defpath = g_path_get_dirname(path);
		g_free(path);
	}
	if (!defpath)
		defpath = g_build_filename(get_user_data_dir(), "Projects", NULL);
	if (defpath && g_file_test(defpath, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), defpath);
	g_free(defpath);

	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(m_pDlg), 
		!CConfig::GetBool(CConfig::DISABLE_PREVIEWS));

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) 
		m_Path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	
	return m_Path!=NULL;
}

void COpenDlg::update_preview(GtkFileChooser *widget, COpenDlg *pMe)
{
	pMe->UpdatePreview(widget);
}

void COpenDlg::UpdatePreview(GtkFileChooser *widget)
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

void COpenDlg::selection_changed(GtkFileChooser *widget, COpenDlg *pMe)
{
	pMe->OnSelectionChanged(widget);
}

void COpenDlg::OnSelectionChanged(GtkFileChooser *widget)
{
	UpdateControls();
}

void COpenDlg::UpdateControls(void)
{
	GtkWidget *ok_btn = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
	gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
	gtk_widget_set_sensitive(ok_btn, (fname ? g_file_test(fname, G_FILE_TEST_IS_REGULAR)!=0 : false));
	g_free(fname);
}

//---------------------------   LOAD PROFILE DIALOG   ---------------------------------

//
// Constructor
//
CLoadProfileDlg::CLoadProfileDlg(GtkWindow *pParent):m_Profiles(NULL), m_Path(NULL)
{
	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Load project settings", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP,
		GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Load project settings from a profile");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog layout
	GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
	gtk_widget_set_size_request(vbox, 500, -1);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);

	// Label
	GtkWidget *label = gtk_label_new("Select a profile to load project settings from:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	// Project settings
	m_TreeView = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_TreeView), FALSE);
	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_TreeView), col);
	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", TCOL_PIXBUF);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", TCOL_CAPTION);
	GtkWidget *scrwnd = gtk_scrolled_window_new (NULL, NULL);
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


//
// Destructor
//
CLoadProfileDlg::~CLoadProfileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	g_free(m_Path);
}


//
// Execute the dialog
//
bool CLoadProfileDlg::Execute(CProfile &profile)
{
	g_free(m_Path);
	m_Path = CConfig::GetStr("NewProject", "LastProfile");

	UpdateProfiles();
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		profile = m_Profile;
		return true;
	}
	return false;
}


//
// Enable/disable OK button
//
void CLoadProfileDlg::UpdateControls(void)
{
	GtkTreeIter iter, parent;
	GtkTreeModel *model;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	bool ok = gtk_tree_selection_get_selected(selection, &model, &iter) && 
		gtk_tree_model_iter_parent(model, &parent, &iter);

	GtkWidget *ok_btn = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(ok_btn, ok);
}

void CLoadProfileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CLoadProfileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CLoadProfileDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_LOAD_PROFILE);
		return false;
	}
	return true;
}

bool CLoadProfileDlg::OnCloseQuery(void)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *path;
	gboolean userDefined;

	m_Profile.Clear();
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) 
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &path, TCOL_USERDEFINED, &userDefined, -1);
	if (userDefined) {
		if (!path) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select one of user-defined or default profiles to specify initial settings for the new project.");
			return false;
		}
		GError *error = NULL;
		if (!m_Profile.Import(path, &error)) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
			g_free(path);
			return false;
		}
		CConfig::SetStr("NewProject", "LastProfile", path);
		g_free(path);
	} else {
		if (!path) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select one of user-defined or default profiles to specify initial settings for the new project.");
			return false;
		}
		m_Profile = CProfile::DefaultProfile(path);
		CConfig::SetStr("NewProject", "LastProfile", path);
		g_free(path);
	}
	return true;
}

//
// Rebuild list of profiles
//
void CLoadProfileDlg::UpdateProfiles(void)
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
					gtk_tree_store_set(m_Profiles, &parent, TCOL_CAPTION, "User-defined profiles", TCOL_USERDEFINED, TRUE, -1);
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
				g_free(fname);
				g_free(basename);
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
					gtk_tree_store_set(m_Profiles, &parent, TCOL_CAPTION, "Predefined profiles", TCOL_USERDEFINED, FALSE, -1);
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

	if (m_Path) {
		GtkTreePath *lastProfileNode = FindNode(m_Path, GTK_TREE_MODEL(m_Profiles), NULL);
		if (lastProfileNode) {
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
			gtk_tree_selection_select_path(selection, lastProfileNode);
			gtk_tree_path_free(lastProfileNode);
		} else {
			g_free(m_Path);
			m_Path = NULL;
		}
	}
}

GtkTreePath *CLoadProfileDlg::FindNode(const gchar *fpath, GtkTreeModel *model, GtkTreeIter *parent)
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

void CLoadProfileDlg::tv_selection_changed(GtkTreeSelection *pWidget, CLoadProfileDlg *pMe)
{
	pMe->OnTreeViewSelectionChanged(pWidget);
}

void CLoadProfileDlg::OnTreeViewSelectionChanged(GtkTreeSelection *pWidget)
{
	gchar *fpath;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(pWidget, &model, &iter)) {
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &fpath, -1);
		if (fpath) {
			g_free(m_Path);
			m_Path = fpath;
			fpath = NULL;
		}
		g_free(fpath);
		UpdateControls();
	}
}

void CLoadProfileDlg::button_clicked(GtkButton *button, CLoadProfileDlg *pMe)
{
	pMe->OnButtonClicked(button);
}

void CLoadProfileDlg::OnButtonClicked(GtkButton *pButton)
{
	if (pButton==GTK_BUTTON(m_OptionsBtn)) {
		CProfilesDlg pDlg(GTK_WINDOW(m_pDlg));
		pDlg.Execute();
		UpdateProfiles();
		UpdateControls();
	}
}

//---------------------------   LOAD PROFILE DIALOG   ---------------------------------

static gboolean has_group(const GtkRecentFilterInfo *filter_info, const gchar *group)
{
	if (filter_info->groups) {
		for (int i=0; filter_info->groups[i]; i++) {
			if (strcmp(filter_info->groups[i], group) == 0)
				return TRUE;
		}
	}
	return FALSE;
}

CImportProfileDlg::CImportProfileDlg(GtkWindow *pParent, tProjectType type):m_Type(type),
	m_Path(NULL)
{
	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Import project settings", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Import project settings from a specified file");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog layout
	GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
	gtk_widget_set_size_request(vbox, 500, 500);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);

	GtkWidget *hlabel = gtk_label_new("Specify path to a project file to import project settings from:");
	gtk_misc_set_alignment(GTK_MISC(hlabel), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), hlabel, FALSE, TRUE, 0);

	// Data source path
	GtkWidget *hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	m_SrcEntry = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_SrcEntry, "Path to the file which the settings shall be loaded from:");
	gtk_editable_set_editable(GTK_EDITABLE(m_SrcEntry), false);
	gtk_box_pack_start(GTK_BOX(hbox), m_SrcEntry, TRUE, TRUE, 0);
	m_BrowseBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_BrowseBtn, "Browse for file in a separate dialog");
	g_signal_connect(G_OBJECT(m_BrowseBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(hbox), m_BrowseBtn, FALSE, TRUE, 0);

	// Label
	GtkWidget *label = gtk_label_new("Recent projects:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	// Recent projects
	m_RecentList = gtk_recent_chooser_widget_new();
	gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(m_RecentList), GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_set_show_private(GTK_RECENT_CHOOSER(m_RecentList), TRUE);
	gtk_recent_chooser_set_local_only(GTK_RECENT_CHOOSER(m_RecentList), TRUE);
	gtk_recent_chooser_set_show_not_found(GTK_RECENT_CHOOSER(m_RecentList), FALSE);
	gtk_widget_set_size_request(m_RecentList, -1, 200);
	gtk_box_pack_start(GTK_BOX(vbox), m_RecentList, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_RecentList), "selection-changed", G_CALLBACK(rc_selection_changed), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CImportProfileDlg::~CImportProfileDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_free(m_Path);
}

bool CImportProfileDlg::Execute(CProfile &profile)
{
	g_free(m_Path);
	m_Path = NULL;

	GtkRecentFilter *filter = gtk_recent_filter_new();
	gtk_recent_filter_add_custom(filter, GTK_RECENT_FILTER_GROUP, (GtkRecentFilterFunc)recent_filter, this, 0);
	gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(m_RecentList), filter);

	// Run the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	profile = m_Profile;
	return true;
}

void CImportProfileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CImportProfileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CImportProfileDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_IMPORT_PROFILE);
		return false;
	}
	return true;
}

bool CImportProfileDlg::OnCloseQuery(void)
{
	m_Profile.Clear();

	g_free(m_Path);
	
	m_Path = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_SrcEntry)));
	if (!m_Path[0]) {
		ShowError(GTK_WINDOW(m_pDlg), "Please, enter name of a project file.");
		return false;
	}
	if (!CProject::isProjectFile(m_Path, NULL)) {
		ShowError(GTK_WINDOW(m_pDlg), "Specified file is a valid project file.");
		return false;
	}

	GError *error = NULL;
	if (!m_Profile.Import(m_Path, &error)) {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
		return false;
	}
	return true;
}

void CImportProfileDlg::rc_selection_changed(GtkRecentChooser *widget, CImportProfileDlg *pDlg)
{
	pDlg->OnSelectionChanged(widget);
}

void CImportProfileDlg::OnSelectionChanged(GtkRecentChooser *widget)
{
	gchar *uri = gtk_recent_chooser_get_current_uri(widget);
	if (uri) {
		gchar *fpath = g_filename_from_uri(uri, NULL, NULL);
		if (fpath) {
			gtk_entry_set_text(GTK_ENTRY(m_SrcEntry), fpath);
			g_free(fpath);
		}
		g_free(uri);
	}
}

void CImportProfileDlg::button_clicked(GtkWidget *button, CImportProfileDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CImportProfileDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn == m_BrowseBtn) 
		BrowseForFile();
}

void CImportProfileDlg::BrowseForFile(void)
{
	GtkFileFilter *filters[2];

	GtkWidget *pOpenDlg = gtk_file_chooser_dialog_new("Select a file", 
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pOpenDlg));

	// File filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*." FILE_EXTENSION_PROJECT);
	gtk_file_filter_set_name(filters[0], "C-Munipack projects");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[0]);

	const gchar *fpath = gtk_entry_get_text(GTK_ENTRY(m_SrcEntry));
	if (!gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pOpenDlg), fpath)) {
		gchar *dir = g_path_get_dirname(fpath);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pOpenDlg), dir);
		g_free(dir);
	}
	if (gtk_dialog_run(GTK_DIALOG(pOpenDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *fpath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pOpenDlg));
		gtk_entry_set_text(GTK_ENTRY(m_SrcEntry), fpath);
		g_free(fpath);
	}
	gtk_widget_destroy(pOpenDlg);
}

gboolean CImportProfileDlg::recent_filter(const GtkRecentFilterInfo *filter_info, CImportProfileDlg *pMe)
{
	return pMe->OnRecentFilter(filter_info);
}

gboolean CImportProfileDlg::OnRecentFilter(const GtkRecentFilterInfo *filter_info)
{
	if (!has_group(filter_info, RECENT_GROUP_PROJECT))
		return FALSE;
	const gchar *group = ProjectTypeRecentGroup(m_Type);
	if (group && !has_group(filter_info, group))
		return FALSE;
	return TRUE;
}

//---------------------------   SAVE PROFILE DIALOG   ---------------------------------

//
// Constructor
//
CSaveProfileDlg::CSaveProfileDlg(GtkWindow *pParent, tAction action):m_Action(action),
	m_Profiles(NULL), m_Path(NULL), m_OldName(NULL)
{
	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons(NULL, pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog layout
	GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
	gtk_widget_set_size_request(vbox, 500, -1);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);

	// Label
	GtkWidget *hlabel = gtk_label_new("Enter profile name:");
	gtk_misc_set_alignment(GTK_MISC(hlabel), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), hlabel, FALSE, TRUE, 0);

	// Data source path
	m_SrcEntry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(m_SrcEntry), MAX_PROFILE_NAME);
	gtk_box_pack_start(GTK_BOX(vbox), m_SrcEntry, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SrcEntry), "changed", G_CALLBACK(name_changed), this);

	// Label
	GtkWidget *label = gtk_label_new("Existing user-defined profiles:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	
	// Profiles
	m_TreeView = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_TreeView), FALSE);
	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_TreeView), col);
	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", TCOL_PIXBUF);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", TCOL_CAPTION);
	GtkWidget *scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_TreeView);
	gtk_widget_set_size_request(scrwnd, 480, 400);
	gtk_box_pack_start(GTK_BOX(vbox), scrwnd, TRUE, TRUE, 4);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tv_selection_changed), this);

	switch (action)
	{
	case SAVE_PROJECT_AS_PROFILE:
		gtk_window_set_title(GTK_WINDOW(m_pDlg), "Save project settings");
		gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Save project settings to as a new user-defined profile");
		gtk_label_set_text(GTK_LABEL(hlabel), "Enter profile name:");
		gtk_widget_set_tooltip_text(m_SrcEntry, "Name of the new profile");
		break;

	case SAVE_PROFILE_AS:
		gtk_window_set_title(GTK_WINDOW(m_pDlg), "Save profile as ...");
		gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Save selected profile as a new user-defined profile");
		gtk_label_set_text(GTK_LABEL(hlabel), "Enter profile name:");
		gtk_widget_set_tooltip_text(m_SrcEntry, "Name of the new profile");
		break;

	case RENAME_PROFILE:
		gtk_window_set_title(GTK_WINDOW(m_pDlg), "Rename profile");
		gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Rename profile");
		gtk_label_set_text(GTK_LABEL(hlabel), "Enter new profile name:");
		gtk_widget_set_tooltip_text(m_SrcEntry, "New name of the selected profile");
		break;
	}

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


//
// Destructor
//
CSaveProfileDlg::~CSaveProfileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	g_free(m_Path);
}

//
// Execute the dialog
//
gchar *CSaveProfileDlg::Execute(const gchar *defaultName)
{
	g_free(m_Path);
	m_Path = NULL;

	m_OldName = defaultName;
	gtk_entry_set_text(GTK_ENTRY(m_SrcEntry), m_OldName);

	UpdateProfiles();
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return NULL;

	CConfig::SetStr("NewProject", "LastProfile", m_Path);
	return g_strdup(m_Path);
}

//
// Enable/disable OK button
//
void CSaveProfileDlg::UpdateControls(void)
{
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(m_SrcEntry));

	GtkWidget *ok_btn = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(ok_btn, name[0]!='\0');
}

void CSaveProfileDlg::response_dialog(GtkWidget *pDlg, gint response_id, CSaveProfileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CSaveProfileDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_PROFILE);
		return false;
	}
	return true;
}

bool CSaveProfileDlg::OnCloseQuery(void)
{
	char buf[MAX_PROJECT_NAME+32];

	g_free(m_Path);
	m_Path = NULL;
	
	gchar *name = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_SrcEntry)));
	if (name[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, select one of user-defined or default profiles to specify initial settings for the new project.");
		g_free(name);
		return false;
	}
	g_strstrip(name);
	gtk_entry_set_text(GTK_ENTRY(m_SrcEntry), name);

	sprintf(buf, "%s.%s", name, FILE_EXTENSION_PROFILE);
	m_Path = g_build_filename(UserProfilesDir(), buf, NULL);

	if (m_Action != RENAME_PROFILE) {
		// Ask for confirmation if the file exists already
		if (!ConfirmOverwrite(GTK_WINDOW(m_pDlg), m_Path)) {
			g_free(name);
			return false;
		}
	} else {
		if (strcmp(name, m_OldName)==0) {
			// Same name --> just exit
			g_free(name);
			return true;
		}
		if (!CheckFileBaseName(name)) {
			// Check profile name
			ShowError(GTK_WINDOW(m_pDlg), "A profile name must contain characters that are allowed in file name.\nDo not use: / \\ ? % * : | \" < and >");
			g_free(name);
			return false;
		}
		if (g_file_test(m_Path, G_FILE_TEST_EXISTS)) {
			// Already exists
			ShowError(GTK_WINDOW(m_pDlg), "A user-defined profile of the specified name already exists.");
			g_free(name);
			return false;
		}
	}

	g_free(name);
	return true;
}


//
// Rebuild list of profiles
//
void CSaveProfileDlg::UpdateProfiles(void)
{
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	m_Profiles = gtk_list_store_new(TNCOLS, GDK_TYPE_PIXBUF, GTK_TYPE_STRING, GTK_TYPE_STRING, G_TYPE_BOOLEAN);

	GSList *ul = GetProfileList(UserProfilesDir());
	if (ul) {
		for (GSList *ptr=ul; ptr!=NULL; ptr=ptr->next) {
			const gchar *fpath = (gchar*)ptr->data;
			CProfile profile;
			if (profile.Import(fpath)) {
				GtkTreeIter child;
				gtk_list_store_append(m_Profiles, &child);
				gchar *basename = g_path_get_basename(fpath);
				gchar *fname = StripFileExtension(basename);
				gtk_list_store_set(m_Profiles, &child, TCOL_FILEPATH, fpath, TCOL_CAPTION, fname, TCOL_USERDEFINED, TRUE, -1);
				g_free(fname);
				g_free(basename);
				gchar *icon = get_icon_file(ProjectTypeIcon(profile.ProjectType()));
				if (icon) {
					gtk_list_store_set(m_Profiles, &child, TCOL_PIXBUF, 
						gdk_pixbuf_new_from_file(icon, NULL), -1);
					g_free(icon);
				}
			}
		}
		ProfileListFree(ul);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), GTK_TREE_MODEL(m_Profiles));
	gtk_tree_view_expand_all(GTK_TREE_VIEW(m_TreeView));
}

GtkTreePath *CSaveProfileDlg::FindNode(const gchar *fpath, GtkTreeModel *model, GtkTreeIter *parent)
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
			g_free(ipath);
		}
		ok = gtk_tree_model_iter_next(model, &iter);
	}
	return NULL;
}

void CSaveProfileDlg::tv_selection_changed(GtkTreeSelection *widget, CSaveProfileDlg *pDlg)
{
	pDlg->OnSelectionChanged(widget);
}

void CSaveProfileDlg::OnSelectionChanged(GtkTreeSelection *widget)
{
	gchar *fname;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(widget, &model, &iter)) {
		gtk_tree_model_get(model, &iter, TCOL_CAPTION, &fname, -1);
		gtk_entry_set_text(GTK_ENTRY(m_SrcEntry), fname);
		g_free(fname);
	}
	UpdateControls();
}

void CSaveProfileDlg::button_clicked(GtkButton *button, CSaveProfileDlg *pMe)
{
	pMe->OnButtonClicked(button);
}

void CSaveProfileDlg::OnButtonClicked(GtkButton *pButton)
{
	if (pButton==GTK_BUTTON(m_OptionsBtn)) {
		CProfilesDlg pDlg(GTK_WINDOW(m_pDlg));
		pDlg.Execute();
		UpdateProfiles();
		UpdateControls();
	}
}
