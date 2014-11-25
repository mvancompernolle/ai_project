/**************************************************************

makecatalogfile_dlg.cpp (C-Munipack project)
The 'Convert files' dialog
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
#include <glib/gstdio.h>

#include "project.h"
#include "configuration.h"
#include "configuration.h"
#include "makecatalogfile_dlg.h"
#include "utils.h"
#include "main.h"
#include "object_dlg.h"
#include "observer_dlg.h"
#include "editselections_dlg.h"
#include "ctxhelp.h"
#include "profile.h"

//-------------------------   PRIVATE DATA   ---------------------------

static const struct {
	const gchar *label;
	CmpackColor color;
} Types[CMPACK_SELECT_COUNT] = {
	{ NULL, CMPACK_COLOR_DEFAULT },
	{ "var", CMPACK_COLOR_RED },
	{ "comp", CMPACK_COLOR_GREEN },
	{ "check", CMPACK_COLOR_BLUE }
};

//-------------------------   PRIVATE CLASSES  --------------------------------

class MyEditSelectionsDlg:public CEditSelectionsDlg
{
public:
	// Constructor
	MyEditSelectionsDlg(GtkWindow *pParent);

	// Execute the dialog
	bool Execute(CSelectionList &list, CTags &tags);

private:
	CSelectionList	m_list;
	CTags			m_tags;

protected:
	bool OnCloseQuery();
};

//-------------------------   MAIN WINDOW   --------------------------------

CMakeCatFileDlg::CMakeCatFileDlg(GtkWindow *pParent):m_pParent(pParent), 
	m_ChartData(NULL), m_ApertureIndex(-1), m_SelectionIndex(-1), m_Updating(false)
{
	GtkWidget *tbox, *frame, *hsep, *label, *scrwnd, *tbar;

	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Make catalog file", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Create a new catalog file and close the dialog");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("catalogfile");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Path and file name
	tbox = gtk_table_new(2, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);

	// File name
	label = gtk_label_new("File name");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_FileName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_FileName, "Name of the target file without extension (usually same as the object's designation)");
	gtk_table_attach(GTK_TABLE(tbox), m_FileName, 1, 2, 0, 1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	// Folder
	label = gtk_label_new("Folder");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_Path = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Path, "Path to the directory where a new catalog file shall be saved to.");
	gtk_table_attach(GTK_TABLE(tbox), m_Path, 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_PathBtn = gtk_button_new_with_label("Change folder");
	gtk_widget_set_tooltip_text(m_PathBtn, "Change the target folder");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), m_PathBtn, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	// Separator
	hsep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), hsep, FALSE, TRUE, 4);

	// File header information
	tbox = gtk_table_new(15, 4, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, TRUE, TRUE, 4);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 0, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 3, 12);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 6, 12);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 2, 8);

	// Object
	label = gtk_label_new("Object - designation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 1, 2);
	m_ObjName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_ObjName, "Object's designation, e.g. RT And");
	gtk_entry_set_max_length(GTK_ENTRY(m_ObjName), MAX_OBJECT_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjName, 1, 2, 1, 2);
	g_signal_connect(G_OBJECT(m_ObjName), "changed", G_CALLBACK(entry_changed), this);
	m_ObjBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_ObjBtn, "Load object coordinates from presets or variable star catalogs");
	g_signal_connect(G_OBJECT(m_ObjBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjBtn, 2, 3, 1, 2);
	label = gtk_label_new("- right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 2, 3);
	m_RA = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_RA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_RA), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_RA, 1, 2, 2, 3);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 2, 3);
	label = gtk_label_new("- declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 3, 4);
	m_Dec = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Dec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	gtk_entry_set_max_length(GTK_ENTRY(m_Dec), 64);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Dec, 1, 2, 3, 4);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 3, 4);

	// Location
	label = gtk_label_new("Observatory - name");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 4, 5);
	m_LocName = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_LocName, "Observer's location designation, e.g. Brno, Czech Republic");
	gtk_entry_set_max_length(GTK_ENTRY(m_LocName), MAX_LOCATION_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocName, 1, 2, 4, 5);
	m_LocBtn = gtk_button_new_with_label("More");
	gtk_widget_set_tooltip_text(m_LocBtn, "Load observer's coordinates from presets");
	g_signal_connect(G_OBJECT(m_LocBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_LocBtn, 2, 3, 4, 5);
	label = gtk_label_new("- longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 5, 6);
	m_Lon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lon, 1, 2, 5, 6);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 5, 6);
	label = gtk_label_new("- latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 6,7);
	m_Lat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_Lat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Lat, 1, 2, 6, 7);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 2, 3, 6, 7);

	// Observer
	label = gtk_label_new("Observer's name");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 7, 8);
	m_Observer = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Observer, "Observer's name");
	gtk_entry_set_max_length(GTK_ENTRY(m_Observer), MAX_OBSERVER_LEN);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Observer, 1, 3, 7, 8);

	// Equipment
	label = gtk_label_new("Telescope");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 8, 9);
	m_Telescope = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Telescope, "Short description of the telescope");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Telescope, 1, 3, 8, 9);
	label = gtk_label_new("Instrument");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 9, 10);
	m_Camera = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Camera, "Camera brand name and model");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Camera, 1, 3, 9, 10);
	label = gtk_label_new("Color filter");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 10, 11);
	m_Filter = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Filter, "Color filter designation");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Filter, 1, 3, 10, 11);
	label = gtk_label_new("Field of view");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 11, 12);
	m_FOV = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_FOV, "Size of view field, width x height");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FOV, 1, 3, 11, 12);
	label = gtk_label_new("Orientation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 12, 13);
	m_Orientation = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_Orientation, "View field orientation, e.g. top=N, left=W");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Orientation, 1, 3, 12, 13);

	// Notes
	label = gtk_label_new("Remarks");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 4, 13, 14, GTK_FILL, GTK_FILL, 0, 0);
	m_Notes = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_Notes), GTK_WRAP_WORD_CHAR);
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Notes);
	gtk_widget_set_size_request(scrwnd, -1, 140);
	gtk_table_attach_defaults(GTK_TABLE(tbox), scrwnd, 0, 4, 14, 15);

	// Catalog file preview
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tbox), vbox, 3, 4, 0, 13);
	gtk_widget_set_size_request(vbox, 360, -1);

	tbar = gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbar), GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_TEXT);
	gtk_box_pack_start(GTK_BOX(vbox), tbar, FALSE, TRUE, 0);

	toolbar_new_label(tbar, "Selection");
	m_SelectCbx = toolbar_new_combo(tbar, "Choose an item to restore recently used object selection");
	m_Selections = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), GTK_TREE_MODEL(m_Selections));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_SelectCbx), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_SelectCbx), renderer, "text", 1);
	g_signal_connect(G_OBJECT(m_SelectCbx), "changed", G_CALLBACK(combo_changed), this);
	m_EditSel = gtk_tool_button_new(NULL, "Edit");
	gtk_widget_set_tooltip_text(GTK_WIDGET(m_EditSel), "Change variable, comparison and check stars");
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), m_EditSel, -1);
	g_signal_connect(G_OBJECT(m_EditSel), "clicked", G_CALLBACK(button_clicked), this);

	m_Chart = cmpack_chart_view_new();
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(frame), m_Chart);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMakeCatFileDlg::~CMakeCatFileDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_Selections)
		g_object_unref(m_Selections);
}

void CMakeCatFileDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMakeCatFileDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMakeCatFileDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Check input
		if (!OnCloseQuery())
			return false;
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_CATALOG_FILE);
		return false;
	}
	return true;
}

void CMakeCatFileDlg::Execute()
{
	int res = 0;
	char *fov, *orient, *notes;
	GtkTreePath *refpath;

	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	m_Object.Clear();
	m_Location.Clear();
	m_OutFile.Clear();
	m_ApertureIndex = 0;

	m_RefType = g_Project->GetReferenceType();
	switch (m_RefType)
	{
	case REF_FRAME:
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				GError *error = NULL;
				CPhot phot;
				if (!phot.Load(pht_file, &error)) {
					if (error) {
						ShowError(m_pParent, error->message);
						g_error_free(error);
					}
					res = -1;
				} else {
					phot.SelectAperture(m_ApertureIndex);
					m_OutFile.Create(phot);
				}
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		// Load catalog file
		{
			const gchar *tmp_file = g_Project->GetTempCatFile()->FullPath();
			GError *error = NULL;
			CCatalog cat;
			if (!cat.Load(tmp_file, &error)) {
				if (error) {
					ShowError(m_pParent, error->message);
					g_error_free(error);
				}
				res = -1;
			} else {
				m_OutFile.MakeCopy(cat);
			}
		}
		break;

	default:
		ShowError(m_pParent, "No reference file.");
		res = -1;
	}
	if (res!=0)
		return;

	// Choose stars
	m_SelectionList = *g_Project->SelectionList();
	if (g_Project->LastSelection()->Count()>0) {
		m_SelectionIndex = m_SelectionList.IndexOf(*g_Project->LastSelection());
		if (m_SelectionIndex<0) {
			m_SelectionList.Set("", *g_Project->LastSelection());
			m_SelectionIndex = 0;
		}
	} else {
		if (m_SelectionList.Count()>0) 
			m_SelectionList.Set("", CSelection());
		m_SelectionIndex = 0;
	}
	m_Tags = *g_Project->Tags();
	UpdateSelectionList();
	UpdateChart();

	// Object coordinates
	m_Object = *g_Project->ObjectCoords();
	UpdateObjectCoords();

	// Observer coordinates
	m_Location = *g_Project->Location();
	UpdateLocation();

	if (g_Project->Observer()) {
		gtk_entry_set_text(GTK_ENTRY(m_Observer), g_Project->Observer());
	} else {
		gchar *obsname = CConfig::GetStr("MakeCatDlg", "ObsName");
		gtk_entry_set_text(GTK_ENTRY(m_Observer), obsname);
		g_free(obsname);
	}
	if (g_Project->Telescope()) {
		gtk_entry_set_text(GTK_ENTRY(m_Telescope), g_Project->Telescope());
	} else {
		char *telescope = CConfig::GetStr("MakeCatDlg", "Telescope");
		gtk_entry_set_text(GTK_ENTRY(m_Telescope), telescope);
		g_free(telescope);
	}
	if (g_Project->Instrument()) {
		gtk_entry_set_text(GTK_ENTRY(m_Camera), g_Project->Instrument());
	} else {
		gchar *camera = CConfig::GetStr("MakeCatDlg", "Camera");
		gtk_entry_set_text(GTK_ENTRY(m_Camera), camera);
		g_free(camera);
	}

	if (m_RefType==REF_FRAME) {
		if (m_OutFile.Filter())
			gtk_entry_set_text(GTK_ENTRY(m_Filter), m_OutFile.Filter());
		else
			gtk_entry_set_text(GTK_ENTRY(m_Filter), "");
		fov = CConfig::GetStr("MakeCatDlg", "FOV");
		gtk_entry_set_text(GTK_ENTRY(m_FOV), fov);
		g_free(fov);
		orient = CConfig::GetStr("MakeCatDlg", "Orientation");
		gtk_entry_set_text(GTK_ENTRY(m_Orientation), orient);
		g_free(orient);
		notes = CConfig::GetStr("MakeCatDlg", "Notes");
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes)), notes, -1);
		g_free(notes);
	} else {
		if (m_OutFile.Filter())
			gtk_entry_set_text(GTK_ENTRY(m_Filter), m_OutFile.Filter());
		else
			gtk_entry_set_text(GTK_ENTRY(m_Filter), "");
		if (m_OutFile.FieldOfView())
			gtk_entry_set_text(GTK_ENTRY(m_FOV), m_OutFile.FieldOfView());
		else
			gtk_entry_set_text(GTK_ENTRY(m_FOV), "");
		if (m_OutFile.Orientation())
			gtk_entry_set_text(GTK_ENTRY(m_Orientation), m_OutFile.Orientation());
		else
			gtk_entry_set_text(GTK_ENTRY(m_Orientation), "");
		if (m_OutFile.Notes())
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes)), m_OutFile.Notes(), -1);
		else
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes)), "", 0);
	}

	// Restore last folder
	gchar *path = CConfig::GetStr("MakeCatDlg", "Folder", NULL);
	if (path && *path!='\0' && g_file_test(path, G_FILE_TEST_IS_DIR))
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
	else {
		gchar *defpath = g_build_filename(get_user_data_dir(), "Catalog files", NULL);
		if (force_directory(defpath))
			gtk_entry_set_text(GTK_ENTRY(m_Path), defpath);
		g_free(defpath);
	}
	g_free(path);

	// Suggest file name from the object name
	if (m_Object.Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_FileName), m_Object.Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_FileName), "");
		
	// Edit form
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

bool CMakeCatFileDlg::MakeFile(const gchar *exportPath, GError **error)
{
	char *fts_file = SetFileExtension(exportPath, FILE_EXTENSION_FITS);
	
	// Delete previous files
	g_unlink(exportPath);
	if (fts_file)
		g_unlink(fts_file);

	// Make target file
	if (!m_OutFile.SaveAs(exportPath, error))
		return false;

	// Copy image file
	bool ok = true;
	if (fts_file) {
		if (m_RefType==REF_FRAME) {
			GtkTreePath *refpath = g_Project->GetReferencePath();
			if (refpath) {
				gchar *tpath = g_Project->GetImageFile(refpath);
				if (tpath) {
					ok = copy_file(tpath, fts_file, false, error);
					g_free(tpath);
				}
				gtk_tree_path_free(refpath);
			}
		} else {
			const gchar *tmp_file = g_Project->GetTempCatFile()->FullPath();
			if (tmp_file) {
				gchar *tpath = SetFileExtension(tmp_file, FILE_EXTENSION_FITS);
				ok = copy_file(tpath, fts_file, false, error);
				g_free(tpath);
			}
		}
	}

	g_free(fts_file);
	return ok;
}

//
// Update list of selections
//
void CMakeCatFileDlg::UpdateSelectionList(void)
{
	m_Updating = true;
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), NULL);
	gtk_list_store_clear(m_Selections);
	int defIndex = m_SelectionList.IndexOf("");
	if (defIndex>=0) {
		GtkTreeIter iter;
		gtk_list_store_append(m_Selections, &iter);
		gtk_list_store_set(m_Selections, &iter, 0, defIndex, 1, "Default selection", -1);
	}
	for (int i=0; i<m_SelectionList.Count(); i++) {
		if (i!=defIndex) {
			GtkTreeIter iter;
			gtk_list_store_append(m_Selections, &iter);
			gtk_list_store_set(m_Selections, &iter, 0, i, 1, m_SelectionList.Name(i), -1);
		}
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_SelectCbx), GTK_TREE_MODEL(m_Selections));
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Selections), NULL)>0) {
		SelectItem(GTK_COMBO_BOX(m_SelectCbx), m_SelectionIndex);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_SelectCbx))<0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_SelectCbx), 0);
			m_SelectionIndex = (tDateFormat)SelectedItem(GTK_COMBO_BOX(m_SelectCbx));
		}
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_SelectCbx), -1);
		m_SelectionIndex = -1;
	}
	gtk_widget_set_sensitive(m_SelectCbx, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Selections), NULL)>1);
	m_Updating = false;
}

void CMakeCatFileDlg::button_clicked(GtkWidget *pButton, CMakeCatFileDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CMakeCatFileDlg::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton == m_ObjBtn) 
		EditObjectCoords();
	else if (pButton == m_LocBtn) 
		EditLocation();
	else if (pButton == m_PathBtn) 
		ChangeCatalogPath();
	else if (pButton == GTK_WIDGET(m_EditSel))
		EditSelectionList();
}

void CMakeCatFileDlg::combo_changed(GtkComboBox *widget, CMakeCatFileDlg *pDlg)
{
	pDlg->OnComboChanged(widget);
}

void CMakeCatFileDlg::OnComboChanged(GtkComboBox *widget)
{
	if (widget == GTK_COMBO_BOX(m_SelectCbx)) {
		if (!m_Updating) {
			int index = SelectedItem(widget);
			if (index!=m_SelectionIndex) {
				m_SelectionIndex = index;
				UpdateChart();
			}
		}
	}
}

void CMakeCatFileDlg::entry_changed(GtkEntry *widget, CMakeCatFileDlg *pDlg)
{
	pDlg->OnEntryChanged(widget);
}

void CMakeCatFileDlg::OnEntryChanged(GtkEntry *widget)
{
}

void CMakeCatFileDlg::EditObjectCoords(void)
{
	CObjectDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_Object)) 
		UpdateObjectCoords();
}

void CMakeCatFileDlg::EditLocation(void)
{
	CLocationDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(&m_Location)) 
		UpdateLocation();
}

void CMakeCatFileDlg::EditSelectionList(void)
{
	MyEditSelectionsDlg dlg(GTK_WINDOW(m_pDlg));
	if (dlg.Execute(m_SelectionList, m_Tags)) {
		m_SelectionIndex = 0;
		UpdateSelectionList();
		UpdateChart();
	}
}

void CMakeCatFileDlg::UpdateObjectCoords(void)
{
	if (m_Object.Name())
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), m_Object.Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_ObjName), "");
	if (m_Object.RA())
		gtk_entry_set_text(GTK_ENTRY(m_RA), m_Object.RA());
	else
		gtk_entry_set_text(GTK_ENTRY(m_RA), "");
	if (m_Object.Dec())
		gtk_entry_set_text(GTK_ENTRY(m_Dec), m_Object.Dec());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Dec), "");
}

void CMakeCatFileDlg::UpdateLocation(void)
{
	if (m_Location.Name())
		gtk_entry_set_text(GTK_ENTRY(m_LocName), m_Location.Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
	if (m_Location.Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), m_Location.Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (m_Location.Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), m_Location.Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");
}

void CMakeCatFileDlg::UpdateChart(void)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	m_ChartData = m_OutFile.ToChartData(false, false, false);
	if (m_ChartData) {
		CSelection sel = m_SelectionList.At(m_SelectionIndex);
		int count = cmpack_chart_data_count(m_ChartData);
		for (int row=0; row<count; row++) {
			int star_id = (int)cmpack_chart_data_get_param(m_ChartData, row);
			int i = sel.IndexOf(star_id);
			if (i>=0) {
				CmpackSelectionType type = sel.GetType(i);
				cmpack_chart_data_set_color(m_ChartData, row, Types[type].color);
				cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
			} else if (m_Tags.Get(star_id)) {
				cmpack_chart_data_set_color(m_ChartData, row, CMPACK_COLOR_YELLOW);
				cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
			}
		}
	}
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Chart), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Chart), m_Negative);
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), m_ChartData);
	cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Chart), TRUE);
}

bool CMakeCatFileDlg::OnCloseQuery(void)
{
	gchar *notes;
	gchar *path, *filename, *fts_file;
	GtkTextIter start, end;

	const gchar *dir = gtk_entry_get_text(GTK_ENTRY(m_Path));
	if (dir[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a target directory path.");
		return false;
	}
	if (!g_file_test(dir, G_FILE_TEST_IS_DIR)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), 
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A folder \"%s\" does not exist.  Do you want to create it?",
			dir);
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirmation");
		gtk_dialog_add_button(GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(GTK_DIALOG (dialog), "_Create", GTK_RESPONSE_ACCEPT);
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
		gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
		int response = gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (response != GTK_RESPONSE_ACCEPT) 
			return false;
		if (!force_directory(dir)) {
			gchar *msg = g_strdup_printf("Failed to create the directory: %s", dir);
			ShowError(GTK_WINDOW(m_pDlg), msg);
			g_free(msg);
			return false;
		}
	}
	
	gchar *basename = g_path_get_basename(gtk_entry_get_text(GTK_ENTRY(m_FileName)));
	if (basename[0]=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, specify a name of the target file.");
		g_free(basename);
		return false;
	}
	filename = SetFileExtension(basename, FILE_EXTENSION_CATALOG);
	path = g_build_filename(dir, filename, NULL);
	g_free(filename);
	g_free(basename);
	if (!ConfirmOverwrite(GTK_WINDOW(m_pDlg), path)) {
		g_free(path);
		return false;
	}

	fts_file = SetFileExtension(path, FILE_EXTENSION_FITS);
	if (!ConfirmOverwrite(GTK_WINDOW(m_pDlg), fts_file)) {
		g_free(fts_file);
		return false;
	}
	
	// Selection
	m_OutFile.RemoveAllSelections();
	for (int i=0; i<m_SelectionList.Count(); i++) 
		m_OutFile.AddSelection(m_SelectionList.Name(i), m_SelectionList.At(i));
	m_OutFile.SetTags(m_Tags);

	// Object
	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_ObjName)));
	m_Object.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_RA)));
	m_Object.SetRA(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Dec)));
	m_Object.SetDec(g_strstrip(aux));
	g_free(aux);

	m_OutFile.SetObject(m_Object);

	// Location
	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	m_Location.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	m_Location.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	m_Location.SetLat(g_strstrip(aux));
	g_free(aux);

	// Other fields
	m_OutFile.SetLocation(m_Location);
	m_OutFile.SetObserver(gtk_entry_get_text(GTK_ENTRY(m_Observer)));
	CConfig::SetStr("MakeCatDlg", "ObsName", m_OutFile.Observer());
	m_OutFile.SetFilter(gtk_entry_get_text(GTK_ENTRY(m_Filter)));
	m_OutFile.SetTelescope(gtk_entry_get_text(GTK_ENTRY(m_Telescope)));
	CConfig::SetStr("MakeCatDlg", "Telescope", m_OutFile.Telescope());
	m_OutFile.SetInstrument(gtk_entry_get_text(GTK_ENTRY(m_Camera)));
	CConfig::SetStr("MakeCatDlg", "Camera", m_OutFile.Instrument());
	m_OutFile.SetFieldOfView(gtk_entry_get_text(GTK_ENTRY(m_FOV)));
	CConfig::SetStr("MakeCatDlg", "FOV", m_OutFile.FieldOfView());
	m_OutFile.SetOrientation(gtk_entry_get_text(GTK_ENTRY(m_Orientation)));
	CConfig::SetStr("MakeCatDlg", "Orientation", m_OutFile.Orientation());

	// Notes
	GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_Notes));
	gtk_text_buffer_get_start_iter(textbuf, &start);
	gtk_text_buffer_get_end_iter(textbuf, &end);
	notes = gtk_text_buffer_get_text(textbuf, &start, &end, FALSE);
	CConfig::SetStr("MakeCatDlg", "Notes", notes);
	m_OutFile.SetNotes(notes);
	g_free(notes);

	CConfig::SetStr("MakeCatDlg", "Folder", gtk_entry_get_text(GTK_ENTRY(m_Path)));

	// Make file in separate thread
	basename = g_path_get_basename(gtk_entry_get_text(GTK_ENTRY(m_FileName)));
	filename = SetFileExtension(basename, FILE_EXTENSION_CATALOG);
	gchar *exportPath = g_build_filename(gtk_entry_get_text(GTK_ENTRY(m_Path)), filename, NULL);

	int res = 0;
	GError *error = NULL;
	if (!MakeFile(exportPath, &error)) {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
		res = -1;
	}
	g_free(exportPath);
	g_free(filename);
	g_free(basename);
	return res==0;
}
	
void CMakeCatFileDlg::ChangeCatalogPath(void)
{
	const gchar *dirname = gtk_entry_get_text(GTK_ENTRY(m_Path));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select path",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		gtk_entry_set_text(GTK_ENTRY(m_Path), path);
		g_free(path);
	}
	gtk_widget_destroy(pPathDlg);
}

//-------------------------------------------------------------------

//
// Constructor
//
MyEditSelectionsDlg::MyEditSelectionsDlg(GtkWindow *pParent):CEditSelectionsDlg(pParent, MAKE_CATALOG_FILE) 
{
	m_SelectionList = &m_list;
	m_Tags = &m_tags;
}

//
// Execute the dialog
//
bool MyEditSelectionsDlg::Execute(CSelectionList &list, CTags &tags)
{
	int	res = 0;
	gchar *fts_file;
	GtkTreePath *refpath;

	m_list = list;
	m_tags = tags;
	
	m_InstMagnitudes = false;
	m_Current = m_NewSelection = CSelection();
	m_SingleComparison = !g_Project->Profile()->GetBool(CProfile::MULTIPLE_COMP_STARS);
	if (m_list.Count()>0) {
		m_ShowNewSelection = false;
		ShowSelection(0);
	} else {
		m_ShowNewSelection = true;
		ShowSelection(-1);
	}
		
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	m_Image.Clear();

	m_RefType = g_Project->GetReferenceType();
	switch(m_RefType)
	{
	case REF_FRAME:
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				GError *error = NULL;
				if (m_Phot.Load(pht_file, &error)) {
					m_Phot.SelectAperture(0);
					UpdateChart();
					fts_file = g_Project->GetImageFile(refpath);
					if (fts_file) {
						if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
							UpdateImage();
						g_free(fts_file);
					}
				} 
				if (error) {
					ShowError(m_pParent, error->message);
					g_error_free(error);
					res = -1;
				}
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		// Load catalog file
		{
			const gchar *tmp_file = g_Project->GetTempCatFile()->FullPath();
			if (tmp_file) {
				GError *error = NULL;
				if (m_Catalog.Load(tmp_file, &error)) {
					UpdateChart();
					fts_file = SetFileExtension(tmp_file, FILE_EXTENSION_FITS);
					if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
						UpdateImage();
					g_free(fts_file);
				}
				if (error) {
					ShowError(m_pParent, error->message);
					g_error_free(error);
					res = -1;
				}
			}
		}
		break;

	default:
		ShowError(m_pParent, "No reference file.");
		res = -1;
	}
	if (res!=0)
		return false;

	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart),
		m_DisplayMode == DISPLAY_CHART);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage),
		m_DisplayMode == DISPLAY_IMAGE);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowMixed),
		m_DisplayMode == DISPLAY_FULL);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ShowImage), 
		m_Image.Width()>0 && m_Image.Height()>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ShowMixed), 
		m_Image.Width()>0 && m_Image.Height()>0);

	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) == GTK_RESPONSE_ACCEPT) {
		list = m_list;
		tags = m_tags;
		return true;
	}
	return false;
}

bool MyEditSelectionsDlg::OnCloseQuery()
{
	if (m_ShowNewSelection) {
		if (m_SelectionIndex<0 && m_Current.Count()>0)
			m_NewSelection = m_Current;
		if (m_NewSelection.Count()>0) {
			ShowSelection(-1);
			int res = ShowYesNoCancel(GTK_WINDOW(m_pDlg), "The new selection does not have a name. If you want to save it to the catalog file, you have to name it.\n\nDo you want to save the new selection?");
			if (res==GTK_RESPONSE_YES) 
				SaveSelection();
			if (res!=GTK_RESPONSE_NO)
				return false;
		}
	}
	return CEditSelectionsDlg::OnCloseQuery();
}
