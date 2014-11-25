/**************************************************************

makelightcurve_dlg.cpp (C-Munipack project)
'Make light curve files' dialog
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

#include "makelightcurve_dlg.h"
#include "ctxhelp.h"
#include "utils.h"
#include "main.h"
#include "object_dlg.h"
#include "observer_dlg.h"
#include "project_dlg.h"
#include "configuration.h"
#include "configuration.h"
#include "choosestars_dlg.h"
#include "chooseaperture_dlg.h"
#include "proc_classes.h"
#include "profile.h"

//-------------------------   MAKE LIGHT CURVE DIALOG   --------------------------------

CMakeLightCurveDlg::CMakeLightCurveDlg(GtkWindow *pParent):m_pParent(pParent)
{
	GtkWidget *tbox, *vbox, *bbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Make light curve", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Use the entered values and continue");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("lightcurve");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Frame selection
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Process all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Process frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Options
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Light curve options</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	tbox = gtk_table_new(9, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 4);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 5, 12);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);

	// Light curve options
	m_HelCor = gtk_check_button_new_with_label("Compute heliocentric correction values");
	gtk_widget_set_tooltip_text(m_HelCor, "Include heliocentric correction and heliocentric Julian date");
	g_signal_connect(G_OBJECT(m_HelCor), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_HelCor, 0, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_AirMass = gtk_check_button_new_with_label("Compute air mass coefficients");
	gtk_widget_set_tooltip_text(m_AirMass, "Include apparent altitude and air mass coefficients");
	g_signal_connect(G_OBJECT(m_AirMass), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_AirMass, 0, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_InstMag = gtk_check_button_new_with_label("Show raw instrumental magnitudes");
	gtk_widget_set_tooltip_text(m_InstMag, "Use raw magnitudes instead or the differential ones");
	g_signal_connect(G_OBJECT(m_InstMag), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_InstMag, 0, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

	// Object coordinates
	m_ObjLabel = gtk_label_new("Object - designation");
	gtk_misc_set_alignment(GTK_MISC(m_ObjLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjLabel, 0, 1, 3, 4);
	m_ObjName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_ObjName, "Object's designation, e.g. RT And");
	gtk_entry_set_max_length(GTK_ENTRY(m_ObjName), MAX_OBJECT_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjName, 1, 2, 3, 4);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 2, 3, 3, 4);
	m_RALabel = gtk_label_new("- right ascension");
	gtk_misc_set_alignment(GTK_MISC(m_RALabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RALabel, 0, 1, 4, 5);
	m_RA = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_RA), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 4, 5);
	m_RAUnit = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(m_RAUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RAUnit, 2, 3, 4, 5);
	m_DecLabel = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(m_DecLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecLabel, 0, 1, 5, 6);
	m_Dec = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_Dec), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 5, 6);
	m_DecUnit = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(m_DecUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DecUnit, 2, 3, 5, 6);

	// Location
	m_LocLabel = gtk_label_new("Observatory - name");
	gtk_misc_set_alignment(GTK_MISC(m_LocLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocLabel, 0, 1, 6, 7);
	m_LocName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_LocName, "Observer's location designation, e.g. Brno, Czech Republic");
	gtk_entry_set_max_length(GTK_ENTRY(m_LocName), MAX_LOCATION_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocName, 1, 2, 6, 7);
	m_LocBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_LocBtn, "Load observer's coordinates from presets");
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocBtn, 2, 3, 6, 7);
	m_LonLabel = gtk_label_new("- longitude");
	gtk_misc_set_alignment(GTK_MISC(m_LonLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonLabel, 0, 1, 7, 8);
	m_Lon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lon, 1, 2, 7, 8);
	m_LonUnit = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(m_LonUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LonUnit, 2, 3, 7, 8);
	m_LatLabel = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(m_LatLabel), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatLabel, 0, 1, 8, 9);
	m_Lat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lat, 1, 2, 8, 9);
	m_LatUnit = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(m_LatUnit), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LatUnit, 2, 3, 8, 9);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Options
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), bbox);
	m_OptionsBtn = gtk_button_new_with_label("More options...");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(bbox), m_OptionsBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMakeLightCurveDlg::~CMakeLightCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CMakeLightCurveDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMakeLightCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMakeLightCurveDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_LIGHT_CURVE);
		return false;
	}
	return true;
}

void CMakeLightCurveDlg::Execute()
{
	CLightCurveDlg::tParamsRec params;

	// Frames
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	// Light curve settings
	m_ComputeHC = g_Project->GetBool("LightCurve", "HelCorr", false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_HelCor), m_ComputeHC);
	m_ComputeAM = g_Project->GetBool("LightCurve", "AirMass", false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AirMass), m_ComputeAM);
	m_ShowInstMag = g_Project->GetBool("LightCurve", "InstMag", false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_InstMag), m_ShowInstMag);

	// Reference frame
	if (g_Project->GetReferenceType()==REF_UNDEFINED) {
		ShowError(m_pParent, "Please, execute the matching first.");
		return;
	}

	// Object coordinates
	m_ObjCoords = *g_Project->ObjectCoords();
	UpdateObjectCoords();

	// Observer coordinates
	m_Location = *g_Project->Location();
	UpdateLocation();

	// Light curve parameters
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	g_Project->SetBool("LightCurve", "HelCorr", m_ComputeHC);
	g_Project->SetBool("LightCurve", "AirMass", m_ComputeAM);
	g_Project->SetBool("LightCurve", "InstMag", m_ShowInstMag);
	g_Project->SetObjectCoords(m_ObjCoords);
	CConfig::SetLastObject(m_ObjCoords);
	g_Project->SetLocation(m_Location);
	CConfig::SetLastLocation(m_Location);

	params.amass = m_ComputeAM;
	params.helcor = m_ComputeHC;
	params.instmag = m_ShowInstMag;
	bool selected_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SelBtn))!=0;

	// Create light curve
	CLightCurveDlg *dlg = new CLightCurveDlg();
	if (!dlg->Make(m_pParent, selected_files, params, m_ObjCoords, m_Location)) 
		dlg->Close();
	else
		dlg->Show();
}

//
// Set parameters
//
void CMakeLightCurveDlg::UpdateObjectCoords(void)
{
	const CObjectCoords *obj = &m_ObjCoords;
	if (obj->Name())
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), obj->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), "");
	if (obj->RA())
		gtk_entry_set_text(GTK_ENTRY(m_RA), obj->RA());
	else
		gtk_entry_set_text(GTK_ENTRY(m_RA), "");
	if (obj->Dec())
		gtk_entry_set_text(GTK_ENTRY(m_Dec), obj->Dec());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Dec), "");
}

void CMakeLightCurveDlg::UpdateLocation(void)
{
	const CLocation *obs = &m_Location;
	if (obs->Name())
		gtk_entry_set_text(GTK_ENTRY(m_LocName), obs->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
	if (obs->Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), obs->Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (obs->Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), obs->Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");
}

void CMakeLightCurveDlg::UpdateControls()
{
	gboolean ok;

	ok = m_ComputeHC || m_ComputeAM;
	gtk_widget_set_sensitive(m_ObjBtn, ok);
	gtk_widget_set_sensitive(m_ObjName, ok);
	gtk_widget_set_sensitive(m_ObjLabel, ok);
	gtk_widget_set_sensitive(m_RA, ok);
	gtk_widget_set_sensitive(m_RALabel, ok);
	gtk_widget_set_sensitive(m_RAUnit, ok);
	gtk_widget_set_sensitive(m_Dec, ok);
	gtk_widget_set_sensitive(m_DecLabel, ok);
	gtk_widget_set_sensitive(m_DecUnit, ok);

	ok = m_ComputeAM;
	gtk_widget_set_sensitive(m_LocBtn, ok);
	gtk_widget_set_sensitive(m_LocName, ok);
	gtk_widget_set_sensitive(m_LocLabel, ok);
	gtk_widget_set_sensitive(m_Lat, ok);
	gtk_widget_set_sensitive(m_LatLabel, ok);
	gtk_widget_set_sensitive(m_LatUnit, ok);
	gtk_widget_set_sensitive(m_Lon, ok);
	gtk_widget_set_sensitive(m_LonLabel, ok);
	gtk_widget_set_sensitive(m_LonUnit, ok);
}

bool CMakeLightCurveDlg::OnCloseQuery()
{
	const gchar *ra, *dec, *lon, *lat;
	gchar buf[64];
	gdouble x, y;

	m_ComputeHC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_HelCor))!=0;
	m_ComputeAM = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AirMass))!=0;
	m_ShowInstMag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_InstMag))!=0;
	
	if (m_ComputeHC || m_ComputeAM) {
		ra = gtk_entry_get_text(GTK_ENTRY(m_RA));
		dec = gtk_entry_get_text(GTK_ENTRY(m_Dec));
		if (*ra=='\0' || *dec=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter object coordinates.");
			return false;
		}
		if (cmpack_strtora(ra, &x)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the right ascension.");
			return false;
		}
		cmpack_ratostr(x, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_RA), buf);
		
		if (cmpack_strtodec(dec, &y)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the declination.");
			return false;
		}
		cmpack_dectostr(y, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Dec), buf);
	}

	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	m_ObjCoords.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	m_ObjCoords.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	m_ObjCoords.SetDec(g_strstrip(aux));
	g_free(aux);

	if (m_ComputeAM) {
		lon = gtk_entry_get_text(GTK_ENTRY(m_Lon));
		lat = gtk_entry_get_text(GTK_ENTRY(m_Lat));
		if (*lon=='\0' || *lat=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter the geographic coordinates.");
			return false;
		}
		if (cmpack_strtolon(lon, &x)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the longitude.");
			return false;
		}
		cmpack_lontostr(x, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Lon), buf);
		if (cmpack_strtolat(lat, &y)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of the latitude.");
			return false;
		}
		cmpack_lattostr(y, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_Lat), buf);
	}

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	m_Location.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	m_Location.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	m_Location.SetLat(g_strstrip(aux));
	g_free(aux);

	return true;
}

void CMakeLightCurveDlg::button_clicked(GtkWidget *pButton, CMakeLightCurveDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CMakeLightCurveDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
	else if (pButton == m_OptionsBtn)
		EditPreferences();
	UpdateControls();
}

void CMakeLightCurveDlg::button_toggled(GtkWidget *pButton, CMakeLightCurveDlg *pMe)
{
	pMe->OnButtonToggled(pButton);
}

void CMakeLightCurveDlg::OnButtonToggled(GtkWidget *pButton)
{
	m_ComputeHC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_HelCor))!=0;
	m_ComputeAM = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AirMass))!=0;
	UpdateControls();
}

void CMakeLightCurveDlg::EditObjectCoords(void)
{
	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	m_ObjCoords.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	m_ObjCoords.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	m_ObjCoords.SetDec(g_strstrip(aux));
	g_free(aux);

	CObjectDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_ObjCoords)) {
		UpdateObjectCoords();
		UpdateControls();
	}
}

void CMakeLightCurveDlg::EditLocation(void)
{
	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	m_Location.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	m_Location.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	m_Location.SetLat(g_strstrip(aux));
	g_free(aux);

	CLocationDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_Location)) {
		UpdateLocation();
		UpdateControls();
	}
}

void CMakeLightCurveDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_LIGHT_CURVE);
}
