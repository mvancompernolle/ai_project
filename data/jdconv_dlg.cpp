/**************************************************************

config_dlg.cpp (C-Munipack project)
Configuration dialog
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
#include "jdconv_dlg.h"
#include "main_dlg.h"
#include "main.h"
#include "ctxhelp.h"

//-------------------------   JD CONVERTER DIALOG   --------------------------------

CJDConvDlg::CJDConvDlg(GtkWindow *pParent):m_Updating(false), m_JD(0), 
	m_ActMode(MODE_DATETIME_TO_JD)
{
	GtkWidget *tbox, *label;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("JD converter", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("jdconv");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	tbox = gtk_table_new(3, 3, FALSE);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 0, 16);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 1, 16);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 8);

	// Options
	label = gtk_label_new("Compute");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 0, 1);
	m_Mode = gtk_combo_box_new_text();
	gtk_widget_set_tooltip_text(m_Mode, "Select an operation; what do you need to compute?");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_Mode), "Julian date to UTC");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_Mode), "UTC to Julian date");
	g_signal_connect(G_OBJECT(m_Mode), "changed", G_CALLBACK(select_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Mode, 1, 2, 0, 1);

	// Input entry
	m_Label1 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Label1), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Label1, 0, 1, 1, 2);
	m_Edit1 = gtk_entry_new_with_max_length(32);
	g_signal_connect(G_OBJECT(m_Edit1), "changed", G_CALLBACK(entry_changed), this);	
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Edit1, 1, 2, 1, 2);
	m_Unit1 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Unit1), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Unit1, 2, 3, 1, 2);

	// Output entry
	m_Label2 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Label2), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Label2, 0, 1, 2, 3);
	m_Edit2 = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Edit2, "Output value");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Edit2, 1, 2, 2, 3);
	gtk_editable_set_editable(GTK_EDITABLE(m_Edit2), false);
	gtk_widget_set_size_request(m_Edit2, 200, -1);
	m_Unit2 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Unit2), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Unit2, 2, 3, 2, 3);

	gtk_widget_show_all(tbox);
}

CJDConvDlg::~CJDConvDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CJDConvDlg::response_dialog(GtkDialog *pDlg, gint response_id, CJDConvDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CJDConvDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_JD_CONVERSION);
		return false;
	}
	return true;
}

void CJDConvDlg::select_changed(GtkComboBox *pWidget, CJDConvDlg *pMe)
{
	pMe->OnSelectChanged(pWidget);
}

void CJDConvDlg::OnSelectChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		m_Updating = true;
		GetData();
		SetMode((tMode)gtk_combo_box_get_active(pWidget));
		SetData();
		m_Updating = false;
	}
}

void CJDConvDlg::entry_changed(GtkEntry *pWidget, CJDConvDlg *pMe)
{
	pMe->OnEntryChanged(pWidget);
}

void CJDConvDlg::OnEntryChanged(GtkEntry *pWidget)
{
	if (!m_Updating) {
		m_Updating = true;
		GetData();
		SetData();
		m_Updating = false;
	}
}

void CJDConvDlg::Execute()
{
	m_Updating = true;

	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	if (gtk_tree_selection_count_selected_rows(pSel)>0) {
		GtkTreeModel *model;
		GList *rrlist = gtk_tree_selection_get_selected_rows(pSel, &model);
		m_JD = g_Project->GetJulDate((GtkTreePath*)rrlist->data);
		g_list_foreach (rrlist, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rrlist);
		if (m_JD<2000000 || m_JD>3000000)
			m_JD = 0;
	} else {
		m_JD = 0;
	}

	SetMode((tMode)CConfig::GetInt("JDConvDlg", "Mode", MODE_JD_TO_DATETIME, 0, MODE_DATETIME_TO_JD));
	SetData();
	m_Updating = false;

	gtk_dialog_run(GTK_DIALOG(m_pDlg));

	CConfig::SetInt("JDConvDlg", "Mode", m_ActMode);
}

void CJDConvDlg::SetMode(tMode mode)
{
	CmpackDateTime dt;
	char buf[256];

	m_ActMode = mode;
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_Mode), m_ActMode);
	switch (m_ActMode)
	{
	case MODE_JD_TO_DATETIME:
		gtk_label_set_label(GTK_LABEL(m_Label1), "Julian date");
		gtk_label_set_label(GTK_LABEL(m_Unit1), "[24xxxxx.xxx]");
		gtk_widget_set_tooltip_text(m_Edit1, "Julian date and time (JD), e.g. 2459999.876");
		gtk_label_set_label(GTK_LABEL(m_Label2), "Date and time (UTC)");
		gtk_label_set_label(GTK_LABEL(m_Unit2), "[y-m-d h:m:s]");
		if (m_JD>0) {
			sprintf(buf, "%.7f", m_JD);
			gtk_entry_set_text(GTK_ENTRY(m_Edit1), buf);
		} else 
			gtk_entry_set_text(GTK_ENTRY(m_Edit1), "");
		break;

	case MODE_DATETIME_TO_JD:
		gtk_label_set_label(GTK_LABEL(m_Label1), "Date and time (UTC)");
		gtk_label_set_label(GTK_LABEL(m_Unit1), "[y-m-d h:m:s]");
		gtk_widget_set_tooltip_text(m_Edit1, "Coordinated Universal Time (UTC), e.g. 2012-12-25 12:34:59");
		gtk_label_set_label(GTK_LABEL(m_Label2), "Julian date");
		gtk_label_set_label(GTK_LABEL(m_Unit2), "[24xxxxx.xxx]");
		if (m_JD>0 && cmpack_decodejd(m_JD, &dt)==0) {
			sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
				dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
			gtk_entry_set_text(GTK_ENTRY(m_Edit1), buf);
		} else 
			gtk_entry_set_text(GTK_ENTRY(m_Edit1), "");
		break;
	}
}

void CJDConvDlg::GetData()
{
	CmpackDateTime dt;
	const char *text;
	char *buf, *ptr;

	m_JD = 0;

	text = gtk_entry_get_text(GTK_ENTRY(m_Edit1));
	if (*text!='\0') {
		switch (m_ActMode) 
		{
		case MODE_JD_TO_DATETIME:
			// Read Julian date
			buf = g_strdup(text);
			ptr = strchr(buf, ',');
			if (ptr)
				*ptr = '.';
			m_JD = atof(buf);
			g_free(buf);
			break;

		case MODE_DATETIME_TO_JD:
			// Read date and time
			memset(&dt, 0, sizeof(CmpackDateTime));
			if (cmpack_strtodate(text, &dt.date)==0) {
				const char *time = strchr(text, ' ');
				if (time) {
					if (cmpack_strtotime(time+1, &dt.time)==0) 
						m_JD = cmpack_encodejd(&dt);
				} else {
					m_JD = cmpack_encodejd(&dt);
				}
			}
			break;
		}
	}
}

void CJDConvDlg::SetData()
{
	CmpackDateTime dt;
	char buf[256];

	gtk_entry_set_text(GTK_ENTRY(m_Edit2), "");
	if (m_JD>0) {
		switch (m_ActMode) 
		{
		case MODE_JD_TO_DATETIME:
			if (cmpack_decodejd(m_JD, &dt)==0) {
				sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
					dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
				gtk_entry_set_text(GTK_ENTRY(m_Edit2), buf);
			} else
				gtk_entry_set_text(GTK_ENTRY(m_Edit2), "Invalid Julian date");
			break;

		case MODE_DATETIME_TO_JD:
			if (m_JD>0) {
				sprintf(buf, "%.7f", m_JD);
				gtk_entry_set_text(GTK_ENTRY(m_Edit2), buf);
			} else
				gtk_entry_set_text(GTK_ENTRY(m_Edit2), "Invalid date or time");
			break;
		}
	}
}
