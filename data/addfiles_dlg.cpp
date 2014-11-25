/**************************************************************

addfiles_dlg.cpp (C-Munipack project)
The 'Add files' dialog
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

#include "addfiles_dlg.h"
#include "addfilter_dlg.h"
#include "project.h"
#include "configuration.h"
#include "configuration.h"
#include "ctxhelp.h"
#include "utils.h"
#include "main.h"

CAddFilesDlg::CAddFilesDlg(GtkWindow *pParent)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Add individual frames", pParent, GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT, GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP,
		GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(m_pDlg), true);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Add selected frames to the project");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	g_signal_connect(G_OBJECT(m_pDlg), "update-preview", G_CALLBACK(update_preview), this);

	// Dialog icon
	gchar *icon = get_icon_file("addfiles");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Preview widget
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(m_pDlg), 
		GTK_WIDGET(g_object_ref(m_Preview.Handle())));
}

CAddFilesDlg::~CAddFilesDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CAddFilesDlg::Execute()
{
	gchar *folder = g_Project->GetStr("AddFiles", "Folder", NULL);
	if (!folder)
		folder = CConfig::GetStr("AddFiles", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(m_pDlg), 
		!CConfig::GetBool(CConfig::DISABLE_PREVIEWS));
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
	g_Project->Save();
}

void CAddFilesDlg::update_preview(GtkFileChooser *pChooser, CAddFilesDlg *pMe)
{
	pMe->OnUpdatePreview(pChooser);
}

void CAddFilesDlg::OnUpdatePreview(GtkFileChooser *pChooser)
{
	if (!CConfig::GetBool(CConfig::DISABLE_PREVIEWS)) {
		gchar *file = gtk_file_chooser_get_preview_filename(pChooser);
		if (file) {
			m_Preview.Update(file);
			g_free(file);
		} else {
			m_Preview.Clear();
		}
	}
}

void CAddFilesDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAddFilesDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAddFilesDlg::OnResponseDialog(gint response_id)
{
	gchar *path;
	GSList *filelist;
	CAddFilterDlg *pFilterDlg;

	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Add files to project
		path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pDlg));
		g_Project->SetStr("AddFiles", "Folder", path);
		CConfig::SetStr("AddFiles", "Folder", path);
		filelist = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(m_pDlg));
		pFilterDlg = new CAddFilterDlg(GTK_WINDOW(m_pDlg), IDH_ADD_FILES);
		pFilterDlg->Execute(filelist);
		delete pFilterDlg;
		g_slist_free(filelist);
		g_free(path);
		return false;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_ADD_FILES);
		return false;
	}
	return true;
}
