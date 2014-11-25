/**************************************************************

preview_dlg.cpp (C-Munipack project)
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

#include "info_dlg.h"
#include "main.h"

struct tFieldRec
{
	int id;
	GtkWidget *label;
	GtkWidget *value;
};

static void fieldrec_free(struct tFieldRec *rec, gpointer user_data)
{
	g_free(rec);
}

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CInfoDlg::CInfoDlg(GtkWindow *pParent,int help_id):m_HelpID(help_id), m_Labels(NULL)
{
	// Dialog with buttons
	if (m_HelpID>0) {
		m_pDlg = gtk_dialog_new_with_buttons("", pParent, 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	} else {
		m_pDlg = gtk_dialog_new_with_buttons("", pParent, 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	}
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog layout
	m_Box = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), m_Box, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(m_Box), 8);

	// Table layout
	m_Tab = gtk_table_new(0, 0, false);
	gtk_table_set_row_spacings(GTK_TABLE(m_Tab), 4);
	gtk_table_set_col_spacings(GTK_TABLE(m_Tab), 4);
	gtk_box_pack_start(GTK_BOX(m_Box), m_Tab, TRUE, TRUE, 0);
}

CInfoDlg::~CInfoDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_list_foreach(m_Labels, (GFunc)fieldrec_free, NULL);
	g_list_free(m_Labels);
}

void CInfoDlg::response_dialog(GtkDialog *pDlg, gint response_id, CInfoDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CInfoDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		if (m_HelpID>0) {
			// Show context help
			g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), m_HelpID);
		}
		return false;
	}
	return true;
}

void CInfoDlg::AddField(int field, int col, int row, const char *caption, int ellipsize)
{
	tFieldRec *rec = (tFieldRec*)g_malloc0(sizeof(tFieldRec));
	
	rec->id = field;
	rec->label = gtk_label_new(caption);
	gtk_misc_set_alignment(GTK_MISC(rec->label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_Tab), rec->label, 2*col, 2*col+1, row, row+1, 
		GTK_FILL, (GtkAttachOptions)0, 4, 0);
	rec->value = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(rec->value), 0.0, 0.5);
	gtk_label_set_max_width_chars(GTK_LABEL(rec->value), 60);
	gtk_table_attach(GTK_TABLE(m_Tab), rec->value, 2*col+1, 2*col+2, row, row+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), (GtkAttachOptions)0, 4, 0);
	m_Labels = g_list_append(m_Labels, rec);
	if (ellipsize)
		gtk_label_set_ellipsize(GTK_LABEL(rec->value), (PangoEllipsizeMode)ellipsize);
}

void CInfoDlg::AddHeading(int field, int col, int row, const char *caption)
{
	GtkWidget *label;
	char buf[512];

	label = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	sprintf(buf, "<b>%s</b>", caption);
	gtk_label_set_markup(GTK_LABEL(label), buf);
	gtk_table_attach(GTK_TABLE(m_Tab), label, 2*col, 2*col+2, row, row+1, 
		GTK_FILL, (GtkAttachOptions)0, 4, 0);
}

void CInfoDlg::AddSeparator(int col, int row)
{
	gtk_table_attach(GTK_TABLE(m_Tab), gtk_label_new(""), 2*col, 2*col+2, row, row+1,
		GTK_FILL, (GtkAttachOptions)0, 4, 0);
}

void CInfoDlg::ShowModal()
{
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CInfoDlg::SetField(int label, const char *value)
{
	GList *ptr;

	for (ptr=m_Labels; ptr!=NULL; ptr=ptr->next) {
		if (((tFieldRec*)(ptr->data))->id == label) {
			if (value)
				gtk_label_set_label(GTK_LABEL(((tFieldRec*)(ptr->data))->value), value);
			else
				gtk_label_set_label(GTK_LABEL(((tFieldRec*)(ptr->data))->value), "------");
			break;
		}
	}
}

void CInfoDlg::SetField(int label, int value, const char *unit)
{
	GList *ptr;
	char buf[128];

	for (ptr=m_Labels; ptr!=NULL; ptr=ptr->next) {
		if (((tFieldRec*)(ptr->data))->id == label) {
			if (unit)
				sprintf(buf, "%d %s", value, unit);
			else
				sprintf(buf, "%d", value);
			gtk_label_set_label(GTK_LABEL(((tFieldRec*)(ptr->data))->value), buf);
			break;
		}
	}
}

void CInfoDlg::SetField(int label, double value, int prec, const char *unit)
{
	GList *ptr;
	char buf[128];

	for (ptr=m_Labels; ptr!=NULL; ptr=ptr->next) {
		if (((tFieldRec*)(ptr->data))->id == label) {
			if (unit)
				sprintf(buf, "%.*f %s", prec, value, unit);
			else
				sprintf(buf, "%.*f", prec, value);
			gtk_label_set_label(GTK_LABEL(((tFieldRec*)(ptr->data))->value), buf);
			break;
		}
	}
}

void CInfoDlg::SetHeading(int id, const char *caption, const char *color)
{
	char buf[128];
	GList *ptr;

	for (ptr=m_Labels; ptr!=NULL; ptr=ptr->next) {
		if (((tFieldRec*)(ptr->data))->id == id) {
			if (color) {
				sprintf(buf, "<span weight=\"bold\" color=\"%s\" underline=\"single\">%s</span>", color, caption);
				gtk_label_set_markup(GTK_LABEL(((tFieldRec*)(ptr->data))->label), buf);
			} else {
				sprintf(buf, "<span weight=\"bold\" underline=\"single\">%s</span>", caption);
				gtk_label_set_markup(GTK_LABEL(((tFieldRec*)(ptr->data))->label), buf);
			}
			break;
		}
	}
}
