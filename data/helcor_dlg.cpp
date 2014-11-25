/**************************************************************

helcor_dlg.cpp (C-Munipack project)
Heliocentric correction dialog
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
#include "helcor_dlg.h"
#include "main_dlg.h"
#include "object_dlg.h"
#include "main.h"
#include "ctxhelp.h"

//-------------------------   JD CONVERTER DIALOG   --------------------------------

CHelCorDlg::CHelCorDlg(GtkWindow *pParent):m_Updating(false), m_ActMode(MODE_HCORR),
	m_JDValue(0), m_InvalidUTC(false)
{
	GtkWidget *tbox, *label;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Heliocentric correction", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("helcorr");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	tbox = gtk_table_new(6, 3, FALSE);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 0, 16);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 16);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 4, 16);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 8);

	// Options
	label = gtk_label_new("Compute");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 0, 1);
	m_Mode = gtk_combo_box_new_text();
	gtk_widget_set_tooltip_text(m_Mode, "Select an operation; what do you need to compute?");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_Mode), "Heliocentric correction");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_Mode), "Heliocentric Julian date");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_Mode), "Geocentric Julian date");
	g_signal_connect(G_OBJECT(m_Mode), "changed", G_CALLBACK(select_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Mode, 1, 2, 0, 1);

	// Object coordinates
	label = gtk_label_new("Object - right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 1, 2);
	m_RA = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	g_signal_connect(G_OBJECT(m_RA), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 1, 2);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 1, 2);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 3, 4, 1, 2);

	label = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 2, 3);
	m_Dec = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	g_signal_connect(G_OBJECT(m_Dec), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 2, 3);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 2, 3);

	// Date and time of observation
	m_Label3 = gtk_label_new("Date and time (UTC)");
	gtk_misc_set_alignment(GTK_MISC(m_Label3), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Label3, 0, 1, 3, 4);
	m_UTC = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_UTC, "Coordinated Universal Time or observation");
	g_signal_connect(G_OBJECT(m_UTC), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_UTC, 1, 2, 3, 4);
	label = gtk_label_new("[y-m-d h:m:s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 3, 4);
	m_Label1 = gtk_label_new("Julian date");
	gtk_misc_set_alignment(GTK_MISC(m_Label1), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Label1, 0, 1, 4, 5);
	m_JD = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_JD, "Julian date and time");
	g_signal_connect(G_OBJECT(m_JD), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_JD, 1, 2, 4, 5);
	label = gtk_label_new("[24xxxxx.xxx]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 4, 5);

	// Output value
	m_Label2 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Label2), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Label2, 0, 1, 5, 6);
	m_Edit2 = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(m_Edit2), false);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Edit2, 1, 2, 5, 6);
	gtk_widget_set_size_request(m_Edit2, 200, -1);

	gtk_widget_show_all(tbox);
}

CHelCorDlg::~CHelCorDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CHelCorDlg::response_dialog(GtkDialog *pDlg, gint response_id, CHelCorDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CHelCorDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_HELIOC_CORRECTION);
		return false;
	}
	return true;
}

void CHelCorDlg::select_changed(GtkComboBox *pWidget, CHelCorDlg *pMe)
{
	pMe->OnSelectChanged(pWidget);
}

void CHelCorDlg::OnSelectChanged(GtkComboBox *pWidget)
{
	SetMode((tMode)gtk_combo_box_get_active(pWidget));
}

void CHelCorDlg::button_clicked(GtkWidget *pButton, CHelCorDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CHelCorDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn)
		EditObjectCoords();
}

void CHelCorDlg::entry_changed(GtkWidget *pWidget, CHelCorDlg *pMe)
{
	pMe->OnEntryChanged(pWidget);
}

void CHelCorDlg::OnEntryChanged(GtkWidget *pWidget)
{
	if (!m_Updating) {
		m_Updating = true;
		if (pWidget == m_JD) {
			m_JDValue = 0;
			const gchar *text = gtk_entry_get_text(GTK_ENTRY(m_JD));
			if (*text!='\0') {
				// Julian date has been changed
				gchar *aux = g_strdup(text);
				gchar *ptr = strchr(aux, ',');
				if (ptr)
					*ptr = '.';
				m_JDValue = atof(aux);
				CmpackDateTime dt;
				if (cmpack_decodejd(m_JDValue, &dt)==0) {
					gchar buf[512];
					sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
						dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
					gtk_entry_set_text(GTK_ENTRY(m_UTC), buf);
				} else {
					gtk_entry_set_text(GTK_ENTRY(m_UTC), "Invalid Julian date");
					m_InvalidUTC = false;
				}
				g_free(aux);
				m_NotFilled = false;
			} else {
				gtk_entry_set_text(GTK_ENTRY(m_UTC), "");
				m_InvalidUTC = false;
				m_NotFilled = true;
			}
		} else if (pWidget == m_UTC) {
			// UTC has been changed
			m_JDValue = 0;
			const gchar *text = gtk_entry_get_text(GTK_ENTRY(m_UTC));
			if (*text!='\0') {
				CmpackDateTime dt;
				memset(&dt, 0, sizeof(CmpackDateTime));
				if (cmpack_strtodate(text, &dt.date)==0) {
					const char *time = strchr(text, ' ');
					if (time) {
						if (cmpack_strtotime(time+1, &dt.time)==0) 
							m_JDValue = cmpack_encodejd(&dt);
					} else {
						m_JDValue = cmpack_encodejd(&dt);
					}
				}
				if (m_JDValue>0) {
					gchar buf[512];
					sprintf(buf, "%.7f", m_JDValue);
					gtk_entry_set_text(GTK_ENTRY(m_JD), buf);
				} else {
					gtk_entry_set_text(GTK_ENTRY(m_JD), "Invalid date or time");
					m_InvalidUTC = true;
				}
				m_NotFilled = false;
			} else {
				gtk_entry_set_text(GTK_ENTRY(m_JD), "");
				m_InvalidUTC = true;
				m_NotFilled = true;
			}
		}
		Refresh(false);
		m_Updating = false;
	}
}

void CHelCorDlg::Execute()
{
	// Restore last parameters
	m_ActMode = (tMode)CConfig::GetInt("HelCorDlg", "Mode", MODE_HCORR, 0, MODE_GJD);

	m_Updating = true;

	CObjectCoords obj = *g_Project->ObjectCoords();
	if (!obj.Valid())
		obj = CConfig::LastObject();
	if (obj.RA())
		gtk_entry_set_text(GTK_ENTRY(m_RA), obj.RA());
	else
		gtk_entry_set_text(GTK_ENTRY(m_RA), "");
	if (obj.Dec())
		gtk_entry_set_text(GTK_ENTRY(m_Dec), obj.Dec());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Dec), "");

	double jd = 0;
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	if (gtk_tree_selection_count_selected_rows(pSel)>0) {
		GtkTreeModel *model;
		GList *rrlist = gtk_tree_selection_get_selected_rows(pSel, &model);
		jd = g_Project->GetJulDate((GtkTreePath*)rrlist->data);
		if (m_ActMode == MODE_GJD && jd>0) {
			double x, y;
			if (cmpack_strtora(gtk_entry_get_text(GTK_ENTRY(m_RA)), &x)==0 &&
				cmpack_strtodec(gtk_entry_get_text(GTK_ENTRY(m_Dec)), &y)==0)
				jd += cmpack_helcorr(jd, x, y);
		}
		if (jd<2000000 || jd>3000000)
			jd = 0;
		g_list_foreach (rrlist, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rrlist);
	}
	if (jd>0) {
		gchar val[256];
		sprintf(val, "%.7f", jd);
		gtk_entry_set_text(GTK_ENTRY(m_JD), val);
	} else 
		gtk_entry_set_text(GTK_ENTRY(m_JD), "");
	m_Updating = false;

	OnEntryChanged(m_JD);
	SetMode(m_ActMode);

	gtk_dialog_run(GTK_DIALOG(m_pDlg));

	// Store last parameters
	obj.Clear();
	obj.SetRA(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	obj.SetDec(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	CConfig::SetLastObject(obj);

	CConfig::SetInt("HelCorDlg", "Mode", m_ActMode);
}

void CHelCorDlg::SetMode(tMode mode)
{
	if (!m_Updating) {
		m_Updating = true;
		m_ActMode = mode;
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_Mode), m_ActMode);
		switch (m_ActMode) 
		{
		case MODE_HCORR:
			gtk_label_set_label(GTK_LABEL(m_Label1), "Julian date");
			gtk_label_set_label(GTK_LABEL(m_Label3), "Date and time (UTC)");
			gtk_label_set_label(GTK_LABEL(m_Label2), "Heliocentric correction");
			gtk_widget_set_tooltip_text(m_Edit2, "Heliocentric correction");
			break;
		case MODE_HJD:
			gtk_label_set_label(GTK_LABEL(m_Label1), "Geocentric Julian date");
			gtk_label_set_label(GTK_LABEL(m_Label3), "Geocentric date and time (UTC)");
			gtk_label_set_label(GTK_LABEL(m_Label2), "Heliocentric Julian date");
			gtk_widget_set_tooltip_text(m_Edit2, "Heliocentric Julian date");
			break;
		case MODE_GJD:
			gtk_label_set_label(GTK_LABEL(m_Label1), "Heliocentric Julian date");
			gtk_label_set_label(GTK_LABEL(m_Label3), "Heliocentric date and time (UTC)");
			gtk_label_set_label(GTK_LABEL(m_Label2), "Geocentric Julian date");
			gtk_widget_set_tooltip_text(m_Edit2, "Geocentric Julian date");
		}
		Refresh(true);
		m_Updating = false;
	}
}

void CHelCorDlg::Refresh(bool refresh_all)
{
	double x, y;
	char buf[256];
	const char *text, *errmsg = NULL;

	text = gtk_entry_get_text(GTK_ENTRY(m_RA));
	if (*text=='\0') 
		errmsg = "Specify object's coordinates";
	else if (cmpack_strtora(text, &x)!=0) 
		errmsg = "Invalid right ascension";
	else {
		if (refresh_all) {
			cmpack_ratostr(x, buf, 256);
			gtk_entry_set_text(GTK_ENTRY(m_RA), buf);
		}
	}
	
	text = gtk_entry_get_text(GTK_ENTRY(m_Dec));
	if (*text=='\0') 
		errmsg = "Specify object's coordinates";
	else if (cmpack_strtodec(text, &y)!=0) 
		errmsg = "Invalid declination";
	else {
		if (refresh_all) {
			cmpack_dectostr(y, buf, 256);
			gtk_entry_set_text(GTK_ENTRY(m_Dec), buf);
		}
	}

	if (m_NotFilled)
		errmsg = "Specify time of observation";
	else if (m_JDValue<=0) 
		errmsg = (!m_InvalidUTC ? "Invalid Julian date" : "Invalid date or time");
	else {
		if (refresh_all) {
			if (m_JDValue>0) {
				sprintf(buf, "%.7f", m_JDValue);
				gtk_entry_set_text(GTK_ENTRY(m_JD), buf);
			} else 
				gtk_entry_set_text(GTK_ENTRY(m_JD), "");
			CmpackDateTime dt;
			if (cmpack_decodejd(m_JDValue, &dt)==0) {
				sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
					dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
				gtk_entry_set_text(GTK_ENTRY(m_UTC), buf);
			} else
				gtk_entry_set_text(GTK_ENTRY(m_UTC), "");
		}
	}

	if (errmsg) 
		gtk_entry_set_text(GTK_ENTRY(m_Edit2), errmsg);
	else {
		double helcor = cmpack_helcorr(m_JDValue, x, y);
		switch (m_ActMode)
		{
		case MODE_HCORR:
			sprintf(buf, "%.7f", helcor);
			break;
		case MODE_HJD:
			sprintf(buf, "%.7f", m_JDValue + helcor);
			break;
		case MODE_GJD:
			sprintf(buf, "%.7f", m_JDValue - helcor);
			break;
		}
		gtk_entry_set_text(GTK_ENTRY(m_Edit2), buf);
	}
}

void CHelCorDlg::EditObjectCoords(void)
{
	CObjectCoords obj;
	CObjectDlg dlg(GTK_WINDOW(m_pDlg));

	obj.SetRA(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	obj.SetDec(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	if (dlg.Execute(&obj)) {
		m_Updating = true;
		gtk_entry_set_text(GTK_ENTRY(m_RA), obj.RA());
		gtk_entry_set_text(GTK_ENTRY(m_Dec), obj.Dec());
		Refresh(true);
		m_Updating = false;
	}
}
