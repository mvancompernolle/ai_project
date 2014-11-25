/**************************************************************

addfolder_dlg.cpp (C-Munipack project)
The 'Add folder' dialog
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

#include "addfolder_dlg.h"
#include "addfilter_dlg.h"
#include "project.h"
#include "configuration.h"
#include "ctxhelp.h"
#include "utils.h"
#include "main.h"

CAddFolderDlg::CAddFolderDlg(GtkWindow *pParent)
{
	GtkWidget *frame, *vbox;

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Add files from folder", pParent, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT, GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Search and add frames to the project");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("addfolder");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Options
	frame = gtk_frame_new("Search options");
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	m_SubDirs = gtk_check_button_new_with_label("Include subdirectories");
	gtk_widget_set_tooltip_text(m_SubDirs, "Search frames also in all sub-directories in the selected folder");
	gtk_box_pack_start(GTK_BOX(vbox), m_SubDirs, TRUE, TRUE, 0);
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CAddFolderDlg::~CAddFolderDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CAddFolderDlg::Execute()
{
	// Go to last folder
	gchar *folder = g_Project->GetStr("AddFiles", "Folder", NULL);
	if (!folder)
		folder = CConfig::GetStr("AddFiles", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	// Restore last state
	bool subdirs = g_Project->GetBool("AddFiles", "SubDirs", true);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SubDirs), subdirs);
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
	g_Project->Save();
}

void CAddFolderDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAddFolderDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAddFolderDlg::OnResponseDialog(gint response_id)
{
	bool	subdirs;
	gchar	*path;
	CAddFilterDlg *pFilterDlg;
	
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Add files to project
		path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pDlg));
		g_Project->SetStr("AddFiles", "Folder", path);
		CConfig::SetStr("AddFiles", "Folder", path);
		subdirs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SubDirs))!=0;
		g_Project->SetBool("AddFiles", "SubDirs", subdirs);
		pFilterDlg = new CAddFilterDlg(GTK_WINDOW(m_pDlg), IDH_ADD_FOLDER);
		pFilterDlg->Execute(path, subdirs);
		delete pFilterDlg;
		g_free(path);
		return false;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_ADD_FOLDER);
		return false;
	}
	return true;
}
