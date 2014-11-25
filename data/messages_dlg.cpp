/**************************************************************

messages_dlg.cpp (C-Munipack project)
The 'Show message log' dialog
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
#include "messages_dlg.h"
#include "progress_dlg.h"
#include "main_dlg.h"
#include "main.h"
#include "utils.h"
#include "ctxhelp.h"

//-------------------------   MESSAGE LOG DIALOG   --------------------------------

GtkTextBuffer *CMessagesDlg::ms_Buffer = NULL;

CMessagesDlg::CMessagesDlg(GtkWindow *pParent)
{
	GtkWidget *text_view, *scrolled_window;
	GdkRectangle rc;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Message log", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	
	// Dialog size
	GdkScreen *scr = gtk_window_get_screen(pParent);
	int mon = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(pParent)->window);
	gdk_screen_get_monitor_geometry(scr, mon, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.7*rc.width), RoundToInt(0.7*rc.height));

	// Dialog icon
	gchar *icon = get_icon_file("messagelog");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// List box
	text_view = gtk_text_view_new_with_buffer(ms_Buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), false);
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_widget_set_size_request(scrolled_window, 640, 480);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), scrolled_window, TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMessagesDlg::~CMessagesDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CMessagesDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMessagesDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMessagesDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MESSAGE_LOG);
		return false;
	}
	return true;
}

void CMessagesDlg::ShowModal()
{
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CMessagesDlg::InitBuffer(void)
{
	GtkTextIter start_iter, end_iter; 
  
	if (!ms_Buffer) {
		// Make new buffer
		ms_Buffer = gtk_text_buffer_new(NULL);
	} else {
		// Clear buffer content
		gtk_text_buffer_get_start_iter(ms_Buffer, &start_iter);
		gtk_text_buffer_get_end_iter(ms_Buffer, &end_iter);
		gtk_text_buffer_delete(ms_Buffer, &start_iter, &end_iter);
	}
}

void CMessagesDlg::FreeBuffer(void)
{
	if (ms_Buffer) {
		g_object_unref(ms_Buffer);
		ms_Buffer = NULL;
	}
}

void CMessagesDlg::LogMessage(const char *text)
{
	if (ms_Buffer) 
		gtk_text_buffer_insert_at_cursor(ms_Buffer, text, -1);
}
