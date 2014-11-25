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
#include <glib/gstdio.h>

#include "profiles_dlg.h"
#include "helper_dlgs.h"
#include "profile_editor.h"
#include "configuration.h"
#include "ctxhelp.h"
#include "main.h"

enum tTypeColumdId
{
	TCOL_PIXBUF,
	TCOL_CAPTION,
	TCOL_FILEPATH,
	TCOL_USERDEFINED,
	TNCOLS
};

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

//-------------------------   PROFILE EDITOR WIDGET   --------------------------------

CProfileEditor::CProfileEditor(CProfilesDlg *pParent):m_pParent(pParent), m_Path(NULL), 
	m_Name(NULL), m_origType(EndOfProjectTypes), m_UserDefined(false)
{
	gchar *str = CConfig::GetStr("ProfileEditor", "LastPage");
	SetInitialPage(str_to_page(str, PAGE_CAMERA));
	g_free(str);
}

CProfileEditor::~CProfileEditor()
{
	g_free(m_Path);
	g_free(m_Name);
}

bool CProfileEditor::Set(const gchar *filePath, bool userDefined, GError **error)
{
	if (m_Path && !m_ReadOnly && m_Changed) {
		if (!SaveChanges(error))
			return false;
	}

	if (filePath) {
		CProfile profile;
		g_free(m_Name);
		g_free(m_Path);
		if (userDefined) {
			if (!profile.Import(filePath, error)) 
				return false;
			m_Path = g_strdup(filePath);
			gchar *basename = g_path_get_basename(m_Path);
			m_Name = StripFileExtension(basename);
			g_free(basename);
		} else {
			profile = CProfile::DefaultProfile(filePath);
			if (profile.ProjectType()==EndOfProjectTypes)
				return false;
			m_Path = g_strdup(filePath);
			m_Name = g_strdup(filePath);
		}
		m_UserDefined = userDefined;
		char buf[MAX_PROFILE_NAME+512];
		sprintf(buf, "Profile '%s'", m_Name);
		SetPageTitle(PAGE_PROFILE, buf);
		SetProfile(profile, !userDefined);
	} else {
		g_free(m_Name);
		m_Name = NULL;
		g_free(m_Path);
		m_Path = NULL;
		m_UserDefined = userDefined;
		SetProfile(CProfile(), true);
	}
	m_origType = m_ProjectType;
	return true;
}

bool CProfileEditor::SaveChanges(GError **error)
{
	if (m_Path && !m_ReadOnly) {
		// Confirmation
		if (m_Changed) {
			char buf[MAX_PROFILE_NAME+1024];
			sprintf(buf, "The profile '%s' has been changed. Do you want to save changes?", m_Name);
			switch (ShowYesNoCancel(GTK_WINDOW(DialogWidget()), buf))
			{
			case GTK_RESPONSE_YES:
				// Save changes
				break;
			case GTK_RESPONSE_NO:
				// Discard changes
				if (m_ProjectType != m_origType)
					m_pParent->OnTypeChanged(m_origType);
				return true;
			default:
				// Cancel action
				return false;
			}
		}
		
		// Check current page
		if (m_currentPageId != EndOfProfilePages) {
			if (!CheckPage(m_currentPageId, error)) 
				return false;
		}

		// Read profile data
		GetData();

		// Check whole profile
		if (!CheckProfile(error)) {
			if ((*error)->code != m_currentPageId)
				ShowPage((tProfilePageId)(*error)->code);
			return false;
		}

		// Update file
		if (!m_Profile.Export(m_Path, error)) 
			return false;
	}

	return true;
}

bool CProfileEditor::SaveAs(const gchar *fpath, GError **error)
{
	if (m_Path) {
		// Check current page
		if (m_currentPageId != EndOfProfilePages) {
			if (!CheckPage(m_currentPageId, error)) 
				return false;
		}

		// Read profile data
		GetData();

		// Check whole profile
		if (!CheckProfile(error)) {
			if ((*error)->code != m_currentPageId)
				ShowPage((tProfilePageId)(*error)->code);
			return false;
		}

		// Update file
		if (!m_Profile.Export(fpath, error)) 
			return false;
		
		g_free(m_Path);
		m_Path = g_strdup(fpath);
		g_free(m_Name);
		gchar *basename = g_path_get_basename(m_Path);
		m_Name = StripFileExtension(basename);
		g_free(basename);
		char buf[MAX_PROFILE_NAME+512];
		sprintf(buf, "Profile '%s'", m_Name);
		SetPageTitle(PAGE_PROFILE, buf);
		m_Changed = false;
		m_UserDefined = true;
		SetReadOnly(false);
		UpdateControls();
	}

	return true;
}

void CProfileEditor::SetData(void)
{
	bool editable = !m_ReadOnly;

	// Profile settings
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), type_to_index(m_ProjectType));
	gtk_widget_set_sensitive(GTK_WIDGET(m_TypeCombo), editable);

	CEditProfileBase::SetData();
}

void CProfileEditor::CreatePages(GSList **stack)
{
	// Profile
	GtkWidget *tbox = CreatePage(PAGE_PROFILE, 0, "Profile", NULL, FALSE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>General properties</b>");
	m_TypeCombo = add_combo_box(GTK_TABLE(tbox), 1, "Type", (AddComboFn)ProjectTypeCaption, 
		PROJECT_REDUCE, EndOfProjectTypes, "Select an option according to the type of source frames and a kind of output data");

	CEditProfileBase::CreatePages(stack);
}

bool CProfileEditor::IsPageVisible(tProfilePageId id)
{
	return (id==PAGE_PROFILE || CEditProfileBase::IsPageVisible(id));
}

void CProfileEditor::OnComboBoxChanged(GtkWidget *pBox)
{
	if (pBox == m_TypeCombo) {
		tProjectType type = index_to_type(gtk_combo_box_get_active(GTK_COMBO_BOX(pBox)));
		if (type>=0 && type<EndOfProjectTypes && type!=m_Profile.ProjectType()) {
			m_Profile.SetProjectType(type);
			m_ProjectType = type;
			gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(m_FilteredPageList));
			m_Changed = true;
			m_pParent->OnTypeChanged(type);
		}
	} else
		CEditProfileBase::OnComboBoxChanged(pBox);
}

GtkWidget *CProfileEditor::DialogWidget(void) 
{ 
	return m_pParent->m_pDlg; 
}

void CProfileEditor::OnPageChanged(tProfilePageId currentPage, tProfilePageId previousPage) 
{
	if (currentPage != EndOfProfilePages)
		CConfig::SetStr("ProfileEditor", "LastPage", page_to_str(currentPage));
}

//-------------------------   PROFILE MANAGER DIALOG   --------------------------------

CProfilesDlg::CProfilesDlg(GtkWindow *pParent):m_Profiles(NULL), m_Updating(false),
	m_currentProfilePath(NULL), m_currentProfileUserDefined(false)
{
	m_pWidget = new CProfileEditor(this);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Edit profiles", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("profile");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	GtkWidget *tbox = gtk_table_new(2, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 0);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), tbox, TRUE, TRUE, 0);
	
	// Profile tree
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
	gtk_widget_set_size_request(scrwnd, 240, 400);
	gtk_table_attach(GTK_TABLE(tbox), scrwnd, 0, 1, 0, 1, GTK_FILL, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Button box
	GtkWidget *bbox = gtk_hbox_new(FALSE, 4);
	gtk_table_attach(GTK_TABLE(tbox), bbox, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_SaveAsBtn = gtk_button_new_with_label("Save as ...");
	g_signal_connect(G_OBJECT(m_SaveAsBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_widget_set_tooltip_text(m_SaveAsBtn, "Save selected profile as a new user-defined profile");
	gtk_box_pack_start(GTK_BOX(bbox), m_SaveAsBtn, TRUE, TRUE, 0);
	m_RenameBtn = gtk_button_new_with_label("Rename");
	g_signal_connect(G_OBJECT(m_RenameBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_widget_set_tooltip_text(m_RenameBtn, "Rename selected profile");
	gtk_box_pack_start(GTK_BOX(bbox), m_RenameBtn, TRUE, TRUE, 0);
	m_RemoveBtn = gtk_button_new_with_label("Remove");
	g_signal_connect(G_OBJECT(m_RemoveBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_widget_set_tooltip_text(m_RemoveBtn, "Remove selected profile");
	gtk_box_pack_start(GTK_BOX(bbox), m_RemoveBtn, TRUE, TRUE, 0);

	// Profile editor widget
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_pWidget->GetHandle(), 1, 2, 0, 1);

	// Dialog size
	GtkRequisition edit_req = m_pWidget->GetSizeRequest();
	GtkRequisition bbox_req, aarea_req, dlg_req, tview_req;
	gtk_widget_size_request(GTK_DIALOG(m_pDlg)->action_area, &aarea_req);
	gtk_widget_size_request(scrwnd, &tview_req);
	gtk_widget_size_request(bbox, &bbox_req);
	dlg_req.width = edit_req.width + tview_req.width + 12;
	dlg_req.height = edit_req.height + aarea_req.height + bbox_req.height + 56;
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

CProfilesDlg::~CProfilesDlg()
{
	gtk_widget_destroy(m_pDlg);
	delete m_pWidget;
	if (m_Profiles)
		g_object_unref(m_Profiles);
	g_free(m_currentProfilePath);
}

void CProfilesDlg::Execute()
{
	g_free(m_currentProfilePath);
	m_currentProfilePath = CConfig::GetStr("ProfileEditor", "LastProfile");
	UpdateProfiles();
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CProfilesDlg::response_dialog(GtkWidget *pDlg, gint response_id, CProfilesDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CProfilesDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_PROFILES);
		return false;
	}
	return true;
}

//
// Check before closing the dialog
//
bool CProfilesDlg::OnCloseQuery(void)
{
	GError *error = NULL;
	if (!m_pWidget->SaveChanges(&error)) {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
		return false;
	}
	return true;
}

//
// Rebuild list of profiles
//
void CProfilesDlg::UpdateProfiles(void)
{
	m_Updating = true;

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	if (m_Profiles)
		g_object_unref(m_Profiles);
	m_Profiles = gtk_tree_store_new(TNCOLS, GDK_TYPE_PIXBUF, GTK_TYPE_STRING, GTK_TYPE_STRING, GTK_TYPE_BOOL);

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
				gchar *name = StripFileExtension(basename);
				g_free(basename);
				gtk_tree_store_set(m_Profiles, &child, TCOL_FILEPATH, fpath, TCOL_CAPTION, name, TCOL_USERDEFINED, TRUE, -1);
				g_free(name);
				gchar *icon = get_icon_file(ProjectTypeIcon(profile.ProjectType()));
				if (icon) {
					gtk_tree_store_set(m_Profiles, &child, TCOL_PIXBUF, 
						gdk_pixbuf_new_from_file(icon, NULL), -1);
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
					gtk_tree_store_set(m_Profiles, &child, TCOL_PIXBUF, 
						gdk_pixbuf_new_from_file(icon, NULL), -1);
					g_free(icon);
				}
			}
		}
		ProfileListFree(dl);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), GTK_TREE_MODEL(m_Profiles));
	gtk_tree_view_expand_all(GTK_TREE_VIEW(m_TreeView));

	GtkTreePath *lastProfileNode = NULL;
	if (m_currentProfilePath) 
		lastProfileNode = FindNode(m_currentProfilePath, GTK_TREE_MODEL(m_Profiles), NULL);
	if (!lastProfileNode) 
		lastProfileNode = gtk_tree_path_new_first();
	if (lastProfileNode) {
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
		gtk_tree_selection_select_path(selection, lastProfileNode);
		gtk_tree_path_free(lastProfileNode);
	}

	m_Updating = false;
	OnSelectionChanged(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView)));
}

GtkTreePath *CProfilesDlg::FindNode(const gchar *fpath, GtkTreeModel *model, GtkTreeIter *parent)
{
	GtkTreeIter iter;

	gboolean ok = gtk_tree_model_iter_children(model, &iter, parent);
	while (ok) {
		gchar *ipath;
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &ipath, -1);
		if (ipath) {
			// Leaf nodes -> profiles
			if (ComparePaths(fpath, ipath)==0) {
				g_free(ipath);
				return gtk_tree_model_get_path(model, &iter);
			}
			g_free(ipath);
		} else {
			// Branches -> call recursively
			GtkTreePath *retval = FindNode(fpath, model, &iter);
			if (retval)
				return retval;
		}
		ok = gtk_tree_model_iter_next(model, &iter);
	}
	return NULL;
}

GtkTreePath *CProfilesDlg::FindFirst(GtkTreeModel *model, GtkTreeIter *parent)
{
	GtkTreeIter iter;

	gboolean ok = gtk_tree_model_iter_children(model, &iter, parent);
	while (ok) {
		gchar *ipath;
		gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &ipath, -1);
		if (ipath) {
			// Leaf node -> profile
			g_free(ipath);
			return gtk_tree_model_get_path(model, &iter);
		} else {
			// Branches -> call recursively
			GtkTreePath *retval = FindFirst(model, &iter);
			if (retval)
				return retval;
		}
		ok = gtk_tree_model_iter_next(model, &iter);
	}
	return NULL;
}

void CProfilesDlg::selection_changed(GtkTreeSelection *selection, CProfilesDlg *pMe)
{
	pMe->OnSelectionChanged(selection);
}

void CProfilesDlg::OnSelectionChanged(GtkTreeSelection *selection)
{
	if (!m_Updating) {
		m_Updating = true;
		GtkTreeModel *model;
		GtkTreeIter iter;
		gchar *fpath = NULL;
		gboolean userDefined = FALSE;
		if (gtk_tree_selection_get_selected(selection, &model, &iter)) 
			gtk_tree_model_get(model, &iter, TCOL_FILEPATH, &fpath, TCOL_USERDEFINED, &userDefined, -1);
		GError *error = NULL;
		if (!m_pWidget->Set(fpath, userDefined!=FALSE, &error)) {
			if (error) {
				ShowError(GTK_WINDOW(m_pDlg), error->message);
				g_error_free(error);
			}
			g_free(fpath);
			// Revert selection to previous node
			if (m_currentProfilePath) {
				GtkTreePath *path = FindNode(m_currentProfilePath, GTK_TREE_MODEL(m_Profiles), NULL);
				if (path) {
					gtk_tree_selection_select_path(selection, path);
					gtk_tree_path_free(path);
				}
			}
		} else {
			g_free(m_currentProfilePath);
			m_currentProfilePath = fpath;
			m_currentProfileUserDefined = userDefined!=FALSE;
		}
		if (m_currentProfilePath)
			CConfig::SetStr("ProfileEditor", "LastProfile", m_currentProfilePath);
		m_Updating = false;
		UpdateControls();
	}
}

void CProfilesDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_SaveAsBtn, m_currentProfilePath!=NULL);
	gtk_widget_set_sensitive(m_RenameBtn, m_currentProfilePath!=NULL && m_currentProfileUserDefined);
	gtk_widget_set_sensitive(m_RemoveBtn, m_currentProfilePath!=NULL && m_currentProfileUserDefined);
}

void CProfilesDlg::OnTypeChanged(tProjectType type)
{
	if (m_currentProfilePath) {
		GtkTreePath *path = FindNode(m_currentProfilePath, GTK_TREE_MODEL(m_Profiles), NULL);
		if (path) {
			GtkTreeIter iter;
			gtk_tree_model_get_iter(GTK_TREE_MODEL(m_Profiles), &iter, path);
			gchar *icon = get_icon_file(ProjectTypeIcon(type));
			if (icon) 
				gtk_tree_store_set(m_Profiles, &iter, TCOL_PIXBUF, gdk_pixbuf_new_from_file(icon, NULL), -1);
			else
				gtk_tree_store_set(m_Profiles, &iter, TCOL_PIXBUF, NULL, -1);
			g_free(icon);
		}
		gtk_tree_path_free(path);
	}
}

void CProfilesDlg::button_clicked(GtkWidget *pButton, CProfilesDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CProfilesDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_SaveAsBtn)
		SaveProfileAs();
	else if (pButton == m_RenameBtn)
		RenameProfile();
	else if (pButton == m_RemoveBtn)
		RemoveProfile();
}

void CProfilesDlg::SaveProfileAs(void)
{
	if (!m_currentProfilePath) 
		return;

	gchar *basename = g_path_get_basename(m_currentProfilePath);
	gchar *name = StripFileExtension(basename);
	gchar *fname = (gchar*)g_malloc((strlen(name)+64*sizeof(gchar)));
	sprintf(fname, "%s.%s", name, FILE_EXTENSION_PROFILE);
	gchar *fpath = g_build_filename(UserProfilesDir(), fname, NULL);
	if (g_file_test(fpath, G_FILE_TEST_EXISTS)) {
		int index = 1;
		while (g_file_test(fpath, G_FILE_TEST_EXISTS)) {
			g_free(fname);
			fname = (gchar*)g_malloc((strlen(name)+64*sizeof(gchar)));
			sprintf(fname, "%s %d.%s", name, ++index, FILE_EXTENSION_PROFILE);
			g_free(fpath);
			fpath = g_build_filename(UserProfilesDir(), fname, NULL);
		}
		gchar *newName = (gchar*)g_malloc((strlen(name)+64*sizeof(gchar)));
		sprintf(newName, "%s %d", name, index);
		g_free(name);
		name = newName;
	}
	g_free(fname);
	g_free(fpath);
	g_free(basename);

	CSaveProfileDlg dlg(GTK_WINDOW(m_pDlg), CSaveProfileDlg::SAVE_PROFILE_AS);
	fpath = dlg.Execute(name);
	if (!fpath) {
		g_free(name);
		return;
	}
	g_free(name);

	GError *error = NULL;
	if (!m_pWidget->SaveAs(fpath, &error)) {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
		g_free(fpath);
		return;
	}

	g_free(m_currentProfilePath);
	m_currentProfilePath = fpath;
	m_currentProfileUserDefined = true;
	UpdateProfiles();
}

void CProfilesDlg::RenameProfile(void)
{
	if (!m_currentProfilePath || !m_currentProfileUserDefined) 
		return;

	CSaveProfileDlg dlg(GTK_WINDOW(m_pDlg), CSaveProfileDlg::RENAME_PROFILE);
	gchar *basename = g_path_get_basename(m_currentProfilePath);
	gchar *name = StripFileExtension(basename);
	gchar *fpath = dlg.Execute(name);
	if (!fpath) {
		g_free(name);
		g_free(basename);
		return;
	}
	g_free(name);
	g_free(basename);

	if (g_rename(m_currentProfilePath, fpath)!=0) {
		char buf[2024];
		sprintf(buf, "Failed to rename the file \"%s\" to \"%s\".", m_currentProfilePath, fpath);
		ShowError(GTK_WINDOW(m_pDlg), buf);
		g_free(fpath);
		return;
	}

	g_free(m_currentProfilePath);
	m_currentProfilePath = fpath;
	UpdateProfiles();
}

void CProfilesDlg::RemoveProfile(void)
{
	char buf[1024];

	if (!m_currentProfilePath || !m_currentProfileUserDefined) 
		return;

	gchar *basename = g_path_get_basename(m_currentProfilePath);
	gchar *name = StripFileExtension(basename);
	sprintf(buf, "Do you want to remove the profile '%s'?", name);
	if (!ShowConfirmation(GTK_WINDOW(m_pDlg), buf)) {
		g_free(name);
		g_free(basename);
		return;
	}
	g_free(name);
	g_free(basename);

	if (g_unlink(m_currentProfilePath)!=0) {
		char buf[2024];
		sprintf(buf, "Failed to delete the file \"%s\".", m_currentProfilePath);
		ShowError(GTK_WINDOW(m_pDlg), buf);
		return;
	}

	// Which profile shall be selected when the current profile is removed?
	GtkTreePath *path = FindNode(m_currentProfilePath, GTK_TREE_MODEL(m_Profiles), NULL);
	if (path) {
		g_free(m_currentProfilePath);
		GtkTreeIter iter;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(m_Profiles), &iter, path);
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(m_Profiles), &iter)) 
			gtk_tree_model_get(GTK_TREE_MODEL(m_Profiles), &iter, TCOL_FILEPATH, &m_currentProfilePath, -1);
		else if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(GTK_TREE_MODEL(m_Profiles), &iter, path))
			gtk_tree_model_get(GTK_TREE_MODEL(m_Profiles), &iter, TCOL_FILEPATH, &m_currentProfilePath, -1);
		else
			m_currentProfilePath = NULL;
		gtk_tree_path_free(path);
	} else {
		g_free(m_currentProfilePath);
		m_currentProfilePath = NULL;
	}

	UpdateProfiles();
}
