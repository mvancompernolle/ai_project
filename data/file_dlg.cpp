/**************************************************************

file_dlg.cpp (C-Munipack project)
The preview dialog
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

#include "file_dlg.h"
#include "helper_dlgs.h"
#include "main.h"
#include "utils.h"
#include "ctxhelp.h"
#include "ccdfile_dlg.h"
#include "phtfile_dlg.h"
#include "catfile_dlg.h"
#include "table_dlg.h"
#include "varfindfile_dlg.h"
#include "configuration.h"

//-------------------------   FILE PREVIEW WINDOW   --------------------------------

//
// Constructor
//
CFileDlg::CFileDlg():m_StatusCtx(-1), m_StatusMsg(-1), m_Path(NULL), m_Name(NULL), 
	m_NotSaved(false)
{
	GdkRectangle rc;

	g_MainWnd->RegisterFileDlg(this);

	// Dialog with buttons
	m_pDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(m_pDlg), "destroy", G_CALLBACK(destroyed), this);
	g_signal_connect(G_OBJECT(m_pDlg), "realize", G_CALLBACK(realized), this);
	g_signal_connect(G_OBJECT(m_pDlg), "delete-event", G_CALLBACK(delete_event), this);

	// Window size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.6*rc.width), RoundToInt(0.6*rc.height));
	gtk_window_set_position(GTK_WINDOW(m_pDlg), GTK_WIN_POS_CENTER);

	// Window layout
	m_MainBox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_pDlg), m_MainBox);

	// Status bar
	m_Status = gtk_statusbar_new();
	gtk_box_pack_end(GTK_BOX(m_MainBox), m_Status, FALSE, FALSE, 0);
	m_StatusCtx = gtk_statusbar_get_context_id(GTK_STATUSBAR(m_Status), "Main");

	// Dialog icon
	char *icon = get_icon_file("chart");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);
}

//
// Destructor
//
CFileDlg::~CFileDlg()
{
	g_MainWnd->FileDlgClosed(this);
	g_free(m_Path);
	g_free(m_Name);
}

//
// Load a file
//
bool CFileDlg::Load(const char *fpath, GError **error)
{
	g_assert(fpath != NULL);

	m_NotSaved = false;
	SetPath(fpath);
	if (!LoadFile(fpath, error)) {
		SetPath(NULL);
		return false;
	}
	return true;
}

//
// Update window title
//
void CFileDlg::UpdateTitle(void)
{
	if (m_Name) {
		char buf[FILENAME_MAX+128];
		if (m_NotSaved)
			sprintf(buf, "%s - %s (not saved)", m_Name, g_AppTitle);
		else
			sprintf(buf, "%s - %s", m_Name, g_AppTitle);
		gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);
	}
}


//
// Dialog initialization
//
void CFileDlg::OnInitDialog(void)
{
	const char *icon_name = GetIconName();
	gchar *icon_file = get_icon_file((icon_name ? icon_name : "muniwin"));
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon_file, NULL));
	g_free(icon_file);
}


//
// Show a window
//
void CFileDlg::Show(void)
{
	gtk_window_present(GTK_WINDOW(m_pDlg));
}


// 
// Ask if there are unsaved data
//
gboolean CFileDlg::delete_event(GtkWidget *pWnd, GdkEvent *event, CFileDlg *pMe)
{
	return pMe->OnCloseQuery() ? FALSE : TRUE;
}

//
// Ask if there are unsaved data
//
bool CFileDlg::OnCloseQuery(void)
{
	int res;
	GtkWidget *pDlg, *label, *image, *hbox;

	if (m_NotSaved) {
		pDlg = gtk_dialog_new_with_buttons(m_Name, GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
			"Do not close", GTK_RESPONSE_CANCEL, "Close without saving", GTK_RESPONSE_NO, 
			"Save and close", GTK_RESPONSE_YES, NULL);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(pDlg), GTK_RESPONSE_YES, GTK_RESPONSE_NO,
			GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG(pDlg), GTK_RESPONSE_YES);
		hbox = gtk_hbox_new(FALSE, 12); 
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pDlg)->vbox), hbox, FALSE, FALSE, 0); 		
		image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
		gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);
		gtk_box_pack_start(GTK_BOX (hbox), image, FALSE, FALSE, 0);   
		label = gtk_label_new("The content of the file was changed but not saved.\nDo you want to save changes?");
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_container_set_border_width(GTK_CONTAINER(pDlg), 5);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
		gtk_box_set_spacing (GTK_BOX (GTK_DIALOG(pDlg)->vbox), 14);
		gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(pDlg)->action_area), 5);
		gtk_box_set_spacing (GTK_BOX(GTK_DIALOG(pDlg)->action_area), 6); 
		gtk_widget_show_all(GTK_DIALOG(pDlg)->vbox);
		res = gtk_dialog_run(GTK_DIALOG(pDlg));
		gtk_widget_destroy(pDlg);
		if (res==GTK_RESPONSE_YES) {
			GError *error = NULL;
			if (!SaveFile(m_Path, &error)) {
				// Saving failed
				if (error) {
					ShowError(GTK_WINDOW(m_pDlg), error->message, true);
					g_error_free(error);
				}
				return false;
			}
			m_NotSaved = false;
			UpdateTitle();
			UpdateControls();
		}
		if (res!=GTK_RESPONSE_YES && res!=GTK_RESPONSE_NO) {
			// Do not close the dialog
			return false;
		}
	}
	return true;
}

//
// Hide a window
//
void CFileDlg::Close(void)
{
	if (OnCloseQuery())
		gtk_widget_destroy(GTK_WIDGET(m_pDlg));
}

//
// Set file path
//
void CFileDlg::SetPath(const char *fpath)
{
	g_free(m_Path);
	m_Path = (fpath!=NULL ? g_strdup(fpath) : NULL);
	g_free(m_Name);
	if (fpath) {
		gchar *basename = g_path_get_basename(fpath);
		m_Name = StripFileExtension(basename);
		g_free(basename);
	} else
		m_Name = NULL;
	UpdateTitle();
}

//
// Set status text
//
void CFileDlg::SetStatus(const char *text)
{
	if (m_StatusMsg>=0) {
		gtk_statusbar_pop(GTK_STATUSBAR(m_Status), m_StatusCtx);
		m_StatusMsg = -1;
	}
	if (text && strlen(text)>0) 
		m_StatusMsg = gtk_statusbar_push(GTK_STATUSBAR(m_Status), m_StatusCtx, text);
}

//
// Close dialog
//
void CFileDlg::destroyed(GtkObject *pWnd, CFileDlg *pDlg)
{
	delete pDlg;
}

//
// Dialog initialization
//
void CFileDlg::realized(GtkWidget *pWidget, CFileDlg *pDlg)
{
	pDlg->OnInitDialog();
}

//
// Open another file
//
void CFileDlg::Open(void)
{
	COpenDlg pDlg(GTK_WINDOW(m_pDlg), true);
	if (pDlg.Execute()) 
		CFileDlg::Open(GTK_WINDOW(m_pDlg), pDlg.Path());
}

void CFileDlg::SaveAs(const gchar *filterName, const gchar *filterPattern)
{
	GtkFileFilter *filters[2];
	gchar buf[FILENAME_MAX+32];

	sprintf(buf, "Save %s as...", m_Name);
	GtkWidget *pSaveDlg = gtk_file_chooser_dialog_new(buf, GTK_WINDOW(m_pDlg), 
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pSaveDlg));
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(pSaveDlg), true);

	// File filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], filterPattern);
	gtk_file_filter_set_name(filters[0], filterName);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);

	gchar *folder = g_path_get_dirname(m_Path);
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pSaveDlg), folder);
	g_free(folder);

	gchar *basename = g_path_get_basename(m_Path);
	if (basename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pSaveDlg), basename);
	g_free(basename);

	if (gtk_dialog_run(GTK_DIALOG(pSaveDlg))!=GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(pSaveDlg);
		return;
	}
	gtk_widget_hide(pSaveDlg);

	gchar *fpath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pSaveDlg));
	GError *error = NULL;
	if (!SaveFile(fpath, &error)) {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message, true);
			g_error_free(error);
		}
	} else {
		m_NotSaved = false;
		SetPath(fpath);
		UpdateTitle();
		UpdateControls();
	}
	g_free(fpath);
	gtk_widget_destroy(pSaveDlg);
}

// 
// Detect file type and create a new preview window
//
CFileDlg *CFileDlg::Open(GtkWindow *pParent, const gchar *fpath)
{
	CFileDlg *pDlg = g_MainWnd->FindFile(fpath);
	if (pDlg) {
		// If already open, put window to front
		g_Project->SetStr("OpenFile", "File", fpath);
		pDlg->Show();
		return pDlg;
	}
	
	// Autodetect file type
	tFileType ftype = FileType(fpath);
	switch (ftype)
	{
	case TYPE_IMAGE:
		pDlg = new CCCDFileDlg();
		break;
	case TYPE_PHOT:
		pDlg = new CPhtFileDlg();
		break;
	case TYPE_CAT:
		pDlg = new CCatFileDlg();
		break;
	case TYPE_TABLE:
		pDlg = new CTableDlg();
		break;
	case TYPE_VARFIND:
		pDlg = new CVarFindFileDlg();
		break;
	default:
		pDlg = NULL;
	}
	if (!pDlg) {
		// Failed to recognize the type of the file
		ShowError(pParent, "Unsupported format of file.");
		return NULL;
	}

	GError *error = NULL;
	if (!pDlg->Load(fpath, &error)) {
		// Failed to open the file
		if (error) {
			ShowError(pParent, error->message, true);
			g_error_free(error);
		}
		delete pDlg;
		return NULL;
	}
	// Success
	CConfig::AddFileToRecentList(ftype, fpath);
	g_Project->SetStr("OpenFile", "File", fpath);
	pDlg->Show();
	return pDlg;
}
