/**************************************************************

airmass_dlg.cpp (C-Munipack project)
Air mass coefficient dialog
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
#include <math.h>

#include "utils.h"
#include "configuration.h"
#include "configuration.h"
#include "airmass_dlg.h"
#include "main_dlg.h"
#include "object_dlg.h"
#include "observer_dlg.h"
#include "main.h"
#include "ctxhelp.h"
#include "profile.h"

//-------------------------   JD CONVERTER DIALOG   --------------------------------

CAirMassDlg::CAirMassDlg(GtkWindow *pParent):m_Updating(false), m_JDValue(0),
	m_InvalidUTC(false)
{
	GtkWidget *tbox, *label;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Air mass coefficient", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("airmass");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Object coordinates
	tbox = gtk_table_new(8, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 1, 16);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 3, 16);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 5, 16);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 8);

	// Object coordinates
	label = gtk_label_new("Object - right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 0, 1);
	m_RA = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	g_signal_connect(G_OBJECT(m_RA), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 0, 1);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 0, 1);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 3, 4, 0, 1);
	label = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 1, 2);
	m_Dec = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	g_signal_connect(G_OBJECT(m_Dec), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 1, 2);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 1, 2);

	// Observer's coordinates
	label = gtk_label_new("Observer - longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 2, 3);
	m_Lon = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_Lon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	g_signal_connect(G_OBJECT(m_Lon), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lon, 1, 2, 2, 3);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 2, 3);
	m_LocBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_LocBtn, "Load observer's coordinates from presets");
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocBtn, 3, 4, 2, 3);
	label = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 3, 4);
	m_Lat = gtk_entry_new_with_max_length(16);
	gtk_widget_set_tooltip_text(m_Lat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	g_signal_connect(G_OBJECT(m_Lat), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lat, 1, 2, 3, 4);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 3, 4);

	// Date and time of observation
	label = gtk_label_new("Date and time (UTC)");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 4, 5);
	m_UTC = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_UTC, "Coordinated Universal Time or observation");
	g_signal_connect(G_OBJECT(m_UTC), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_UTC, 1, 2, 4, 5);
	label = gtk_label_new("[y-m-d h:m:s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 4, 5);
	label = gtk_label_new("Julian date");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 5, 6);
	m_JD = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_JD, "Julian date and time or observation");
	g_signal_connect(G_OBJECT(m_JD), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_JD, 1, 2, 5, 6);
	label = gtk_label_new("[24xxxxx.xxx]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 5, 6);

	// Output value
	label = gtk_label_new("Air mass coefficient");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 6, 7);
	m_AMass = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_AMass, "Air mass coefficient");
	gtk_editable_set_editable(GTK_EDITABLE(m_AMass), false);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_AMass, 1, 2, 6, 7);
	gtk_widget_set_size_request(m_AMass, 200, -1);
	label = gtk_label_new("Altitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 7, 8);
	m_Alt = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Alt, "Apparent altitude of the object in degrees");
	gtk_editable_set_editable(GTK_EDITABLE(m_Alt), false);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Alt, 1, 2, 7, 8);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CAirMassDlg::~CAirMassDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CAirMassDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAirMassDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAirMassDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_AIR_MASS);
		return false;
	}
	return true;
}

void CAirMassDlg::button_clicked(GtkWidget *pButton, CAirMassDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CAirMassDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
}

void CAirMassDlg::entry_changed(GtkWidget *pWidget, CAirMassDlg *pMe)
{
	pMe->OnEntryChanged(pWidget);
}

void CAirMassDlg::OnEntryChanged(GtkWidget *pWidget)
{
	if (!m_Updating) {
		m_Updating = true;
		if (pWidget == m_JD) {
			m_JDValue = 0;
			// Julian date has been changed
			const gchar *text = gtk_entry_get_text(GTK_ENTRY(m_JD));
			if (*text!='\0') {
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

void CAirMassDlg::Execute()
{
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
	
	CLocation loc = *g_Project->Location();
	if (!loc.Valid())
		loc = CConfig::LastLocation();
	if (loc.Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), loc.Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (loc.Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), loc.Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");

	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	if (gtk_tree_selection_count_selected_rows(pSel)>0) {
		GtkTreeModel *model;
		GList *rrlist = gtk_tree_selection_get_selected_rows(pSel, &model);
		double jd = g_Project->GetJulDate((GtkTreePath*)rrlist->data);
		gchar val[256];
		sprintf(val, "%.7f", jd);
		gtk_entry_set_text(GTK_ENTRY(m_JD), val);
		g_list_foreach (rrlist, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rrlist);
	} else {
		gtk_entry_set_text(GTK_ENTRY(m_JD), "");
	}

	m_Updating = false;

	OnEntryChanged(m_JD);
	Refresh(true);

	gtk_dialog_run(GTK_DIALOG(m_pDlg));

	// Store last parameters
	obj.Clear();
	obj.SetRA(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	obj.SetDec(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	CConfig::SetLastObject(obj);

	loc.Clear();
	loc.SetLon(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	loc.SetLat(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	CConfig::SetLastLocation(loc);
}

void CAirMassDlg::Refresh(bool refresh_all)
{
	double x, y, l, b, airmass, alt;
	char buf[512];
	const gchar *text, *errmsg = NULL;

	text = gtk_entry_get_text(GTK_ENTRY(m_RA));
	if (*text=='\0')
		errmsg = "Specify object coordinates";
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
		errmsg = "Specify object coordinates";
	else if (cmpack_strtodec(text, &y)!=0) 
		errmsg = "Invalid declination";
	else {
		if (refresh_all) {
			cmpack_dectostr(y, buf, 256);
			gtk_entry_set_text(GTK_ENTRY(m_Dec), buf);
		}
	}

	text = gtk_entry_get_text(GTK_ENTRY(m_Lon));
	if (*text=='\0')
		errmsg = "Specify observer coordinates";
	else if (cmpack_strtolon(text, &l)!=0) 
		errmsg = "Invalid longitude";
	else {
		if (refresh_all) {
			cmpack_lontostr(l, buf, 256);
			gtk_entry_set_text(GTK_ENTRY(m_Lon), buf);
		}
	}
	
	text = gtk_entry_get_text(GTK_ENTRY(m_Lat));
	if (*text=='\0')
		errmsg = "Specify observer coordinates";
	else if (cmpack_strtolat(text, &b)!=0)
		errmsg = "Invalid latitude";
	else {
		if (refresh_all) {
			cmpack_lattostr(b, buf, 256);
			gtk_entry_set_text(GTK_ENTRY(m_Lat), buf);
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

	if (errmsg) {
		gtk_entry_set_text(GTK_ENTRY(m_AMass), errmsg);
		gtk_entry_set_text(GTK_ENTRY(m_Alt), "");
	} else {
		cmpack_airmass(m_JDValue, x, y, l, b, &airmass, &alt);
		if (airmass>=1.0) 
			sprintf(buf, "%.3f", airmass);
		else if (alt>=0.0) 
			strcpy(buf, "The object is too close to the horizon");
		else
			strcpy(buf, "Object is below horizon");
		gtk_entry_set_text(GTK_ENTRY(m_AMass), buf);
		if (alt>=0.0)
			sprintf(buf, "%.1f degrees above the horizon", fabs(alt));
		else
			sprintf(buf, "%.1f degrees below the horizon", fabs(alt));
		gtk_entry_set_text(GTK_ENTRY(m_Alt), buf);
	}
}

void CAirMassDlg::EditObjectCoords(void)
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

void CAirMassDlg::EditLocation(void)
{
	CLocation obj;
	CLocationDlg dlg(GTK_WINDOW(m_pDlg));

	obj.SetLon(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	obj.SetLat(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	if (dlg.Execute(&obj)) {
		m_Updating = true;
		gtk_entry_set_text(GTK_ENTRY(m_Lon), obj.Lon());
		gtk_entry_set_text(GTK_ENTRY(m_Lat), obj.Lat());
		Refresh(true);
		m_Updating = false;
	}
}
