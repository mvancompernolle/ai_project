/**************************************************************

progress_dlg.cpp (C-Munipack project)
Progress dialog
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

#include "progress_dlg.h"
#include "messages_dlg.h"
#include "main.h"

CProgressDlg::CProgressDlg(GtkWindow *pParent, const char *Title)
{
	GtkWidget *vbox, *scrolled_window, *bbox;

	m_pDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), g_AppTitle);
	gtk_window_set_transient_for(GTK_WINDOW(m_pDlg), pParent);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(m_pDlg), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(m_pDlg), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_position(GTK_WINDOW(m_pDlg), GTK_WIN_POS_CENTER); 
	gtk_window_set_modal(GTK_WINDOW(m_pDlg), TRUE);
	g_signal_connect(G_OBJECT(m_pDlg), "show", G_CALLBACK(show_dialog), this);
	g_signal_connect(G_OBJECT(m_pDlg), "delete-event", G_CALLBACK(delete_event), this);

	// Window layout
	vbox = gtk_vbox_new(FALSE, 8); 
	gtk_container_add(GTK_CONTAINER(m_pDlg), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	// Caption
	m_Caption = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(m_Caption), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), m_Caption, FALSE, FALSE, 0);
	gtk_widget_show(m_Caption);

	// Filename
	m_FileName = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(m_FileName), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), m_FileName, FALSE, FALSE, 0);
	gtk_widget_show(m_FileName);

	// Progress bar
	m_Progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), m_Progress, FALSE, FALSE, 0);
	gtk_widget_show(m_Progress);

	// List box
	m_TextView = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_TextView), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_TextView), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_TextView), false);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrolled_window), m_TextView);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scrolled_window, 480, 240);
	gtk_widget_show_all(scrolled_window);

	// Button box
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	m_CancelBtn = gtk_button_new_with_label("Cancel");
	gtk_widget_set_tooltip_text(m_CancelBtn, "Abort the current process");
	gtk_box_pack_start(GTK_BOX(bbox), m_CancelBtn, FALSE, TRUE, 0);
	g_signal_connect(m_CancelBtn, "clicked", G_CALLBACK(cancel_clicked), this);
	m_SuspendBtn = gtk_button_new_with_label("Pause");
	gtk_widget_set_tooltip_text(m_SuspendBtn, "Suspend and resume current process");
	gtk_box_pack_start(GTK_BOX(bbox), m_SuspendBtn, FALSE, TRUE, 0);
	g_signal_connect(m_SuspendBtn, "clicked", G_CALLBACK(suspend_clicked), this);
	
	// Message queue
	m_Queue = g_async_queue_new();

	// Synchronization objects
	m_DataMutex = g_mutex_new();
	m_Resume = g_cond_new();

	InternalSetTitle(Title);
	m_Min = m_Max = 0;
}

CProgressDlg::~CProgressDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_async_queue_unref(m_Queue);
	g_mutex_free(m_DataMutex);
	g_cond_free(m_Resume);
}

int CProgressDlg::Execute(ExecProc *pExecFn, void *UserData)
{
	m_Result = 0;
	m_ExecFn = pExecFn;
	m_UserData = UserData;
	m_Thread = NULL;
	m_StopRq = m_Suspend = m_Waiting = false;

	CMessagesDlg::InitBuffer();

	gtk_widget_show_all(GTK_WIDGET(m_pDlg));
	m_Loop = g_main_loop_new (NULL, FALSE); 
	gdk_threads_leave();  
	g_main_loop_run(m_Loop);
	if (m_Thread) {
		g_thread_join(m_Thread);
		m_Thread = NULL;
	}
	gdk_threads_enter();
	g_main_loop_unref(m_Loop); 
	m_Loop = NULL;
	gtk_widget_hide(m_pDlg);

	return m_Result;
}

void CProgressDlg::PushMessage(CProgressDlg::tEventCode event, 
		gint intval, gdouble dblval, gchar *text)
{
	tMessage *msg = (tMessage*)g_malloc(sizeof(tMessage));
	msg->event = event;
	msg->text = text;
	msg->int_val = intval;
	msg->dbl_val = dblval;
	g_async_queue_push(m_Queue, msg);
	g_idle_add(GSourceFunc(idle_func), this);
}

CProgressDlg::tMessage *CProgressDlg::PopMessage()
{
	return (tMessage*)g_async_queue_pop(m_Queue);
}

void CProgressDlg::FreeMessage(tMessage *msg)
{
	if (msg) {
		g_free(msg->text);
		g_free(msg);
	}
}

void CProgressDlg::SetTitle(const char *title)
{
	PushMessage(EVENT_TITLE, 0, 0, g_strdup(title));
}

void CProgressDlg::SetFileName(const char *filename)
{
	PushMessage(EVENT_FILENAME, 0, 0, g_strdup(filename));
}

void CProgressDlg::SetMinMax(double min, double max)
{
	m_Min = min;
	m_Max = max;
	PushMessage(EVENT_PROGRESS, 0, 0);
}

void CProgressDlg::SetProgress(double val)
{
	double progress;

	if (val > m_Max)
		progress = 1.0;
	else if (val<m_Min || m_Max<=m_Min)
		progress = 0.0;
	else
		progress = (val - m_Min)/(m_Max - m_Min);
	PushMessage(EVENT_PROGRESS, 0, progress);
}

void CProgressDlg::show_dialog(GtkWidget *widget, CProgressDlg *pMe)
{
	pMe->m_Thread = g_thread_create(GThreadFunc(thread_proc), pMe, true, NULL);
	if (!pMe->m_Thread)
		g_main_loop_quit(pMe->m_Loop);
}

gboolean CProgressDlg::delete_event(GtkWidget *pWnd, GdkEvent *event, CProgressDlg *pMe)
{
	if (pMe->m_Thread) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(pMe->m_pDlg), GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Do you want to stop the process?");
		if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_YES) {
			g_mutex_lock(pMe->m_DataMutex);
			pMe->m_StopRq = true;
			g_mutex_unlock(pMe->m_DataMutex);
		}
		gtk_widget_destroy(dialog);
		return TRUE;
	}
	return FALSE;
}

void CProgressDlg::thread_proc(CProgressDlg *pMe)
{
	if (pMe->m_ExecFn) 
		pMe->m_Result = pMe->m_ExecFn(pMe, pMe->m_UserData);
	pMe->PushMessage(EVENT_FINISHED);
}

void CProgressDlg::OnPrint(const char *text)
{
	int textlen;
	char *buffer;

	textlen = strlen(text);
	buffer = (char*)g_malloc(textlen+2);
	memcpy(buffer, text, textlen);
	buffer[textlen] = '\n';
	buffer[textlen+1] = '\0';
	PushMessage(EVENT_MESSAGE, 0, 0, buffer);
}

gboolean CProgressDlg::idle_func(CProgressDlg *pMe)
{
	tMessage *msg = pMe->PopMessage();
	if (msg) {
		switch (msg->event) 
		{
		case EVENT_MESSAGE:
			gdk_threads_enter();
			pMe->InternalAddMessage(msg->text);
			gdk_threads_leave();
			break;

		case EVENT_TITLE:
			gdk_threads_enter();
			pMe->InternalSetTitle(msg->text);
			gdk_threads_leave();
			break;

		case EVENT_FILENAME:
			gdk_threads_enter();
			gtk_label_set_text(GTK_LABEL(pMe->m_FileName), msg->text);
			gdk_threads_leave();
			break;

		case EVENT_PROGRESS:
			gdk_threads_enter();
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pMe->m_Progress), msg->dbl_val);
			gdk_threads_leave();
			break;

		case EVENT_FINISHED:
			gdk_threads_enter();
			g_main_loop_quit(pMe->m_Loop);
			gdk_threads_leave();
			break;

		case EVENT_SUSPEND:
			gdk_threads_enter();
			gtk_button_set_label(GTK_BUTTON(pMe->m_SuspendBtn), "Resume");
			gdk_threads_leave();
		}
		FreeMessage(msg);
	}
	return FALSE;
}

void CProgressDlg::InternalAddMessage(const char *text)
{
	GtkTextIter end;
	GtkTextMark *end_mark;

	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_TextView));
	gtk_text_buffer_get_end_iter(buf, &end);
	gtk_text_buffer_insert(buf, &end, text, -1);
	end_mark = gtk_text_buffer_create_mark(buf, NULL, &end, TRUE);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(m_TextView), end_mark, FALSE, TRUE, 1.0, 0.0);
	gtk_text_buffer_delete_mark(buf, end_mark); 

	CMessagesDlg::LogMessage(text);
}

//
// Internal set title
//
void CProgressDlg::InternalSetTitle(const char *text)
{
	int textlen;
	char *str;

	textlen = strlen(text);
	str = (char*)g_malloc((textlen+8)*sizeof(char));
	strcpy(str, "<b>");
	strcat(str, text);
	strcat(str, "</b>");
	gtk_label_set_markup(GTK_LABEL(m_Caption), str);
	g_free(str);
}


//
// Test stop request flag
//
bool CProgressDlg::Cancelled(void)
{
	bool res;
	g_mutex_lock(m_DataMutex);
	if (m_Suspend) {
		PushMessage(EVENT_SUSPEND);
		m_Waiting = true;
		g_cond_wait(m_Resume, m_DataMutex);
		m_Waiting = false;
	}
	res = m_StopRq;
	g_mutex_unlock(m_DataMutex);
	return res;
}


// 
// Set stop request flag
//
void CProgressDlg::cancel_clicked(GtkButton *button, CProgressDlg *pMe)
{
	g_mutex_lock(pMe->m_DataMutex);
	pMe->m_StopRq = true;
	if (pMe->m_Waiting) {
		pMe->m_Suspend = false;
		g_cond_signal(pMe->m_Resume);
		gtk_button_set_label(GTK_BUTTON(pMe->m_SuspendBtn), "Pause");
	}
	g_mutex_unlock(pMe->m_DataMutex);
}

// 
// Set stop request flag
//
void CProgressDlg::suspend_clicked(GtkButton *button, CProgressDlg *pMe)
{
	g_mutex_lock(pMe->m_DataMutex);
	if (!pMe->m_Waiting) {
		pMe->m_Suspend = true;
	} else {
		pMe->m_Suspend = false;
		g_cond_signal(pMe->m_Resume);
		gtk_button_set_label(GTK_BUTTON(pMe->m_SuspendBtn), "Pause");
	}
	g_mutex_unlock(pMe->m_DataMutex);
}
