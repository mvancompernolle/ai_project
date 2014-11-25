/**************************************************************

observer_dlg.cpp (C-Munipack project)
The 'Enter observer coordinates' dialog
Copyright (C) 2009 David Motl, dmotl@volny.cz

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
#include "configuration.h"
#include "observer_dlg.h"
#include "main.h"
#include "ctxhelp.h"
#include "profile.h"

//-------------------------   TABLES   ------------------------------------

struct tTreeViewColumn {
	const char *caption;		// Column name
	int column;					// Model column index
	double align;				// Column alignment
};

const static tTreeViewColumn UserColumns[] = {
	{ "Location",			LOCATION_NAME },
	{ "Longitude",			LOCATION_LON, 1.0 },
	{ "Latitude",			LOCATION_LAT, 1.0 },
	{ "Remarks",			LOCATION_REMARKS },
	{ NULL }
};

//-------------------------   POPUP MENU   ------------------------------------

enum tCommandId
{
	CMD_DELETE = 100,
	CMD_SELECT_ALL,
	CMD_IMPORT,
	CMD_EXPORT
};

static const CPopupMenu::tPopupMenuItem UserContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_DELETE,		"_Remove from bookmarks" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_SELECT_ALL,	"Select _All" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_IMPORT,		"_Import from file" },
	{ CPopupMenu::MB_ITEM, CMD_EXPORT,		"_Export to file" },
	{ CPopupMenu::MB_END }
};

//-------------------------   HELPER FUNCTIONS   --------------------------------

static void make_path_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **pList)
{
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*pList = g_slist_prepend(*pList, rowref);
}

//-------------------------   DIALOG   --------------------------------

// 
// Constructor
//
CLocationDlg::CLocationDlg(GtkWindow *pParent):m_Updating(false), m_UserFileChanged(false),
	m_UserEntryChanged(false), m_DefEntryModified(false), m_SelPath(NULL)
{
	int i;
	GtkWidget *label, *bbox, *vbox, *scrwnd;
	GSList *group;
	GtkTreeSelection *selection;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Observer's coordinates", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_widget_set_size_request(vbox, 480, 480);

	// Method
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Get observer's location</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_ManBtn = gtk_radio_button_new_with_label(NULL, "enter geographic coordinates manually");
	gtk_widget_set_tooltip_text(m_ManBtn, "Check to specify observer's coordinates manually");
	g_signal_connect(G_OBJECT(m_ManBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_ManBtn, FALSE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_ManBtn)); 
	m_DefBtn = gtk_radio_button_new_with_label(group, "use the default location");
	gtk_widget_set_tooltip_text(m_DefBtn, "Use the default location from the project settings");
	g_signal_connect(G_OBJECT(m_DefBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_DefBtn, FALSE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_ManBtn)); 
	m_RefBtn = gtk_radio_button_new_with_label(group, "get location from the reference frame");
	gtk_widget_set_tooltip_text(m_RefBtn, "Check to use observer's coordinates stored in a reference frame that was used in matching");
	g_signal_connect(G_OBJECT(m_RefBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_RefBtn, FALSE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Default location
	m_DefBox = gtk_table_new(4, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_DefBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_DefBox), 16);
	gtk_table_set_col_spacings(GTK_TABLE(m_DefBox), 8);
	gtk_table_attach_defaults(GTK_TABLE(m_DefBox), gtk_label_new(NULL), 0, 3, 0, 1);

	label = gtk_label_new("Observatory");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_DefBox), label, 0, 1, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_DefName = gtk_entry_new_with_max_length(256);
	gtk_widget_set_sensitive(m_DefName, FALSE);
	g_signal_connect(G_OBJECT(m_DefName), "changed", G_CALLBACK(def_entry_changed), this);
	gtk_table_attach(GTK_TABLE(m_DefBox), m_DefName, 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_DefBox), label, 0, 1, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_DefLon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_DefLon, FALSE);
	g_signal_connect(G_OBJECT(m_DefLon), "changed", G_CALLBACK(def_entry_changed), this);
	gtk_table_attach(GTK_TABLE(m_DefBox), m_DefLon, 1, 2, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_DefBox), label, 2, 3, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_DefBox), label, 0, 1, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_DefLat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_DefLat, FALSE);
	g_signal_connect(G_OBJECT(m_DefLat), "changed", G_CALLBACK(def_entry_changed), this);
	gtk_table_attach(GTK_TABLE(m_DefBox), m_DefLat, 1, 2, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_DefBox), label, 2, 3, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(m_DefBox), gtk_label_new(NULL), 0, 3, 4, 5);

	// Reference coordinates
	m_RefBox = gtk_table_new(4, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_RefBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_RefBox), 16);
	gtk_table_set_col_spacings(GTK_TABLE(m_RefBox), 8);
	gtk_table_attach_defaults(GTK_TABLE(m_RefBox), gtk_label_new(NULL), 0, 3, 0, 1);
	label = gtk_label_new("Observatory");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_RefBox), label, 0, 1, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_RefName = gtk_entry_new_with_max_length(256);
	gtk_widget_set_sensitive(m_RefName, FALSE);
	gtk_table_attach(GTK_TABLE(m_RefBox), m_RefName, 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("Longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_RefBox), label, 0, 1, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_RefLon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_RefLon, FALSE);
	gtk_table_attach(GTK_TABLE(m_RefBox), m_RefLon, 1, 2, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_RefBox), label, 2, 3, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("Latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_RefBox), label, 0, 1, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_RefLat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_RefLat, FALSE);
	gtk_table_attach(GTK_TABLE(m_RefBox), m_RefLat, 1, 2, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_RefBox), label, 2, 3, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(m_RefBox), gtk_label_new(NULL), 0, 3, 4, 5);

	// User coordinates
	m_EntryBox = gtk_table_new(4, 5, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_EntryBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_EntryBox), 8);
	gtk_table_set_col_spacings(GTK_TABLE(m_EntryBox), 8);
	gtk_table_attach(GTK_TABLE(m_EntryBox), gtk_label_new(NULL), 0, 1, 0, 5, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(m_EntryBox), gtk_label_new(NULL), 4, 5, 0, 5, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Observatory");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryName = gtk_entry_new_with_max_length(256);
	gtk_widget_set_tooltip_text(m_EntryName, "Observer's location designation, e.g. Brno, Czech Republic");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryName, 2, 3, 0, 1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryName), "changed", G_CALLBACK(usr_entry_changed), this);

	label = gtk_label_new("Longitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryLon = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_EntryLon, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryLon, 2, 3, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s E/W]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 3, 4, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryLon), "changed", G_CALLBACK(usr_entry_changed), this);

	label = gtk_label_new("Latitude");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryLat = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_EntryLat, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryLat, 2, 3, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[d m s N/S]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryLat), "changed", G_CALLBACK(usr_entry_changed), this);

	label = gtk_label_new("Remarks");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryRem = gtk_entry_new_with_max_length(32);
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryRem, 2, 3, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryRem), "changed", G_CALLBACK(usr_entry_changed), this);

	// User catalog
	m_UserView = gtk_tree_view_new();
	gtk_widget_set_tooltip_text(m_UserView, "List of the predefined locations");
	for (i=0; UserColumns[i].caption!=NULL; i++) {
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		// Set column name and alignment
		gtk_tree_view_column_set_title(col, UserColumns[i].caption);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_UserView), col);
		// Add text renderer
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		g_object_set(renderer, "xalign", UserColumns[i].align, NULL);
		gtk_tree_view_column_add_attribute(col, renderer, "text", UserColumns[i].column);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(m_UserView), true);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_UserView), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	g_signal_connect(G_OBJECT(m_UserView), "row-activated", G_CALLBACK(row_activated), this);
	g_signal_connect(G_OBJECT(m_UserView), "button_press_event", G_CALLBACK(button_press_event), this);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_UserView);
	gtk_table_attach_defaults(GTK_TABLE(m_EntryBox), scrwnd, 0, 5, 5, 6);

	// Popup menu
	m_UserMenu.Create(UserContextMenu);
	
	// Buttons
	bbox = gtk_hbutton_box_new();
	gtk_table_attach(GTK_TABLE(m_EntryBox), bbox, 1, 4, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	m_AddBtn = gtk_button_new_with_label("Add");
	gtk_widget_set_tooltip_text(m_AddBtn, "Store specified observer's coordinates into a list of predefined locations");
	g_signal_connect(G_OBJECT(m_AddBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_AddBtn, false, false, 0);
	m_SaveBtn = gtk_button_new_with_label("Save");
	gtk_widget_set_tooltip_text(m_SaveBtn, "Save specified observer's coordinates by updating selected item");
	g_signal_connect(G_OBJECT(m_SaveBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_SaveBtn, false, false, 0);
	m_DelBtn = gtk_button_new_with_label("Remove");
	gtk_widget_set_tooltip_text(m_DelBtn, "Remove selected item from the list of predefined objects");
	g_signal_connect(G_OBJECT(m_DelBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_DelBtn, false, false, 0);

	// User bookmarks
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), NULL);
	m_UserList.Load();
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
	gtk_tree_path_free(m_SelPath);
	m_SelPath = NULL;
	m_UserFileChanged = false;
	m_UserEntryChanged = true;

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);

	SetDefData(*g_Project->Location());
	SetSelectMode(MANUAL_ENTRY);
}


//
// Destructor
//
CLocationDlg::~CLocationDlg()
{
	// Save bookmarks if changed
	if (m_UserFileChanged)
		m_UserList.Save();
	gtk_tree_path_free(m_SelPath);
	gtk_widget_destroy(m_pDlg);
}


//
// Set user coordinates
//
void CLocationDlg::SetUserData(const CLocation *data)
{
	char buf[64];
	double lon, lat;

	if (data->Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_EntryName), data->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_EntryName), "");
	if (cmpack_strtolon(data->Lon(), &lon)==0) {
		cmpack_lontostr(lon, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_EntryLon), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_EntryLon), "");
	if (cmpack_strtolat(data->Lat(), &lat)==0) {
		cmpack_lattostr(lat, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_EntryLat), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_EntryLat), "");
	if (data->Comment()) 
		gtk_entry_set_text(GTK_ENTRY(m_EntryRem), data->Comment());
	else
		gtk_entry_set_text(GTK_ENTRY(m_EntryRem), "");
}

//
// Set default coordinates
//
void CLocationDlg::SetDefData(const CLocation &data)
{
	char buf[64];
	double lon, lat;

	gtk_widget_set_sensitive(m_DefBtn, data.Valid());
	if (data.Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_DefName), data.Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_DefName), "");
	if (cmpack_strtolon(data.Lon(), &lon)==0) {
		cmpack_lontostr(lon, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_DefLon), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_DefLon), "");
	if (cmpack_strtolat(data.Lat(), &lat)==0) {
		cmpack_lattostr(lat, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_DefLat), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_DefLat), "");

	m_DefEntryModified = false;
}

//
// Set reference coordinates
//
void CLocationDlg::SetRefData(const CLocation *data)
{
	char buf[64];
	double lon, lat;

	gtk_widget_set_sensitive(m_RefBtn, data->Valid());
	if (data->Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_RefName), data->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_RefName), "");
	if (cmpack_strtolon(data->Lon(), &lon)==0) {
		cmpack_lontostr(lon, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_RefLon), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_RefLon), "");
	if (cmpack_strtolat(data->Lat(), &lat)==0) {
		cmpack_lattostr(lat, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_RefLat), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_RefLat), "");
}

//
// Get coordinates
//
bool CLocationDlg::GetData(CLocation *data, bool name_required)
{
	const gchar *name, *lon, *lat, *remarks;
	char buf[64];
	double x, y;

	data->Clear();
	name = lon = lat = remarks = NULL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_DefBtn))) {
		// Default location
		name = gtk_entry_get_text(GTK_ENTRY(m_DefName));
		lon = gtk_entry_get_text(GTK_ENTRY(m_DefLon));
		lat = gtk_entry_get_text(GTK_ENTRY(m_DefLat));
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RefBtn))) {
		// Reference location
		name = gtk_entry_get_text(GTK_ENTRY(m_RefName));
		lon = gtk_entry_get_text(GTK_ENTRY(m_RefLon));
		lat = gtk_entry_get_text(GTK_ENTRY(m_RefLat));
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ManBtn))) {
		// User location
		name = gtk_entry_get_text(GTK_ENTRY(m_EntryName));
		lon = gtk_entry_get_text(GTK_ENTRY(m_EntryLon));
		lat = gtk_entry_get_text(GTK_ENTRY(m_EntryLat));
		remarks = gtk_entry_get_text(GTK_ENTRY(m_EntryRem));
	}
	if (name_required && (!name || *name=='\0')) {
		ShowError(GTK_WINDOW(m_pDlg), "Please, enter the name of the location.");
		return false;
	}
	if (!lon || *lon=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, enter the observer's longitude.");
		return false;
	}
	if (!lat || *lat=='\0') {
		ShowError(GTK_WINDOW(m_pDlg), "Please, enter the observer's latitude.");
		return false;
	}
	if (cmpack_strtolon(lon, &x)!=0) {
		ShowError(GTK_WINDOW(m_pDlg), "Invalid value of longitude");
		return false;
	}
	if (cmpack_strtolat(lat, &y)!=0) {
		ShowError(GTK_WINDOW(m_pDlg), "Invalid value of latitude");
		return false;
	}
	if (name) {
		gchar *aux = g_strdup(name);
		data->SetName(g_strstrip(aux));
		g_free(aux);
	}
	cmpack_lontostr(x, buf, 64);
	data->SetLon(buf);
	cmpack_lattostr(y, buf, 64);
	data->SetLat(buf);
	if (remarks) {
		gchar *aux = g_strdup(remarks);
		data->SetComment(g_strstrip(aux));
		g_free(aux);
	}
	return true;
}


//
// Button click handler
//
void CLocationDlg::button_clicked(GtkWidget *button, CLocationDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CLocationDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (!m_Updating) {
		if (pBtn==m_DefBtn)
			SetSelectMode(DEFAULT_LOCATION);
		else if (pBtn==m_RefBtn)
			SetSelectMode(REFERENCE_FRAME);
		else if (pBtn==m_ManBtn)
			SetSelectMode(MANUAL_ENTRY);
		else if (pBtn==m_AddBtn)
			AddToTable();
		else if (pBtn==m_SaveBtn)
			SaveToTable();
		else if (pBtn==m_DelBtn)
			RemoveFromTable();
	}
}


//
// Text entry changed
//
void CLocationDlg::usr_entry_changed(GtkWidget *widget, CLocationDlg *pDlg)
{
	pDlg->OnUsrEntryChanged(widget);
}

void CLocationDlg::OnUsrEntryChanged(GtkWidget *widget)
{
	m_UserEntryChanged = true;
	UpdateControls();
}

void CLocationDlg::def_entry_changed(GtkWidget *widget, CLocationDlg *pDlg)
{
	pDlg->OnDefEntryChanged(widget);
}

void CLocationDlg::OnDefEntryChanged(GtkWidget *widget)
{
	m_DefEntryModified = true;
	UpdateControls();
}

//
// Change dialog mode
//
void CLocationDlg::SetSelectMode(tSelectMode mode)
{
	if (!m_Updating) {
		m_Updating = true;
		m_SelectMode = mode;
		switch (mode)
		{
		case DEFAULT_LOCATION:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DefBtn), true);
			gtk_entry_set_text(GTK_ENTRY(m_EntryName), gtk_entry_get_text(GTK_ENTRY(m_DefName)));
			gtk_entry_set_text(GTK_ENTRY(m_EntryLon), gtk_entry_get_text(GTK_ENTRY(m_DefLon)));
			gtk_entry_set_text(GTK_ENTRY(m_EntryLat), gtk_entry_get_text(GTK_ENTRY(m_DefLat)));
			gtk_widget_hide(m_RefBox);
			gtk_widget_hide(m_EntryBox);
			gtk_widget_show(m_DefBox);
			break;
		case REFERENCE_FRAME:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), true);
			gtk_entry_set_text(GTK_ENTRY(m_EntryName), gtk_entry_get_text(GTK_ENTRY(m_RefName)));
			gtk_entry_set_text(GTK_ENTRY(m_EntryLon), gtk_entry_get_text(GTK_ENTRY(m_RefLon)));
			gtk_entry_set_text(GTK_ENTRY(m_EntryLat), gtk_entry_get_text(GTK_ENTRY(m_RefLat)));
			gtk_widget_hide(m_DefBox);
			gtk_widget_hide(m_EntryBox);
			gtk_widget_show(m_RefBox);
			break;
		case MANUAL_ENTRY:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), true);
			gtk_widget_hide(m_DefBox);
			gtk_widget_hide(m_RefBox);
			gtk_widget_show(m_EntryBox);
			break;
		}
		UpdateControls();
		m_Updating = false;
	}
}


//
// Enable and disable controls
//
void CLocationDlg::UpdateControls(void)
{
	bool name, coords;
	int selected;

	if (m_SelectMode == MANUAL_ENTRY) {
		name = strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryName)))>0;
		coords = strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryLon)))>0 &&
			strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryLat)))>0;
		GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
		selected = gtk_tree_selection_count_selected_rows(pSel);
		gtk_widget_set_sensitive(m_AddBtn, (!m_SelPath || m_UserEntryChanged) && name && coords);
		gtk_widget_set_sensitive(m_SaveBtn, (selected==1 && m_SelPath && m_UserEntryChanged) && name && coords);
		gtk_widget_set_sensitive(m_DelBtn, selected>0);
	}
}

//
// Double click on table confirms the dialog
//
void CLocationDlg::row_activated(GtkTreeView *tree_view, GtkTreePath *path, 
		GtkTreeViewColumn *column, CLocationDlg *pMe)
{
	pMe->OnRowActivated(tree_view, path);
}

//
// Mouse button handler
//
gint CLocationDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CLocationDlg *pMe)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_widget_grab_focus(widget);
		pMe->OnContextMenu(widget, event);
		return TRUE;
	}
	return FALSE;
}


//
// TreeView selection changed
//
void CLocationDlg::selection_changed(GtkTreeSelection *selection, CLocationDlg *pMe)
{
	pMe->OnSelectionChanged(selection);
}

void CLocationDlg::OnSelectionChanged(GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	CLocation data;

	GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
	if (list) {
		if (model==m_UserList.List()) {
			// Update displayed coordinates
			gtk_tree_path_free(m_SelPath);
			m_SelPath = NULL;
			if (g_list_length(list)==1) {
				if (m_UserList.Get((GtkTreePath*)(list->data), &data)) {
					SetUserData(&data);
					m_SelPath = gtk_tree_path_copy((GtkTreePath*)(list->data));
					m_UserEntryChanged = false;
				}
			}
		}
		g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(list);
	} else {
		if (model==m_UserList.List()) {
			gtk_tree_path_free(m_SelPath);
			m_SelPath = NULL;
		}
	}
	UpdateControls();
}


//
// Show context menu
//
void CLocationDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int count, selected, x = (int)event->x, y = (int)event->y;
	GtkTreePath *path;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	if (gtk_tree_selection_count_selected_rows(sel)<=1) {
		if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), x, y, &path, NULL, NULL, NULL)) {
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, NULL, FALSE);
			gtk_tree_path_free(path);
		}
	}
	count = gtk_tree_model_iter_n_children(model, NULL);
	selected = gtk_tree_selection_count_selected_rows(sel);
	m_UserMenu.Enable(CMD_DELETE, selected>0);
	m_UserMenu.Enable(CMD_EXPORT, count>0);
	m_UserMenu.Enable(CMD_SELECT_ALL, count>0);
	switch (m_UserMenu.Execute(event))
	{
	case CMD_DELETE:
		RemoveFromTable();
		break;
	case CMD_IMPORT:
		ImportFromFile();
		break;
	case CMD_EXPORT:
		ExportToFile();
		break;
	case CMD_SELECT_ALL:
		SelectAll();
		break;
	}
}

//
// Select all bookmarks
//
void CLocationDlg::SelectAll(void)
{
	GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	gtk_tree_selection_select_all(pSel);
}


//
// Add coordinates to bookmark
//
void CLocationDlg::AddToTable(void)
{
	CLocation data;
	GtkTreeIter iter;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	if (GetData(&data, true)) {
		GtkTreePath *pPath = m_UserList.Add(&data);
		if (pPath) {
			gtk_tree_model_get_iter(m_UserList.List(), &iter, pPath);
			gtk_tree_selection_unselect_all(selection);
			gtk_tree_selection_select_iter(selection, &iter);
			gtk_tree_path_free(m_SelPath);
			m_SelPath = pPath;
			m_UserEntryChanged = false;
			m_UserFileChanged = true;
		}
	}
	UpdateControls();
}


//
// Save coordinates to bookmarks (replace selected item)
//
void CLocationDlg::SaveToTable(void)
{
	CLocation data;
	GtkTreeModel *model;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
	if (list) {
		if (gtk_tree_path_compare((GtkTreePath*)(list->data), m_SelPath)==0) {
			if (GetData(&data, true)) {
				// Update file
				m_UserList.Update(m_SelPath, &data);
				m_UserEntryChanged = false;
				m_UserFileChanged = true;
			}
		}
	}
	UpdateControls();
}


//
// Remove selected item from the table
//
void CLocationDlg::RemoveFromTable(void)
{
	GSList *rr_list = NULL;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	if (gtk_tree_selection_count_selected_rows(selection)>0) {
		gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)make_path_list, &rr_list);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), NULL);
		for (GSList *node = rr_list; node != NULL; node = node->next) {
			GtkTreePath *path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				m_UserList.Remove(path);
				gtk_tree_path_free(path);
			}
		}
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
		g_slist_foreach(rr_list, (GFunc)gtk_tree_row_reference_free, NULL);
		g_slist_free(rr_list);
	}
	UpdateControls();
}


//
// Import locations from an external file
//
void CLocationDlg::ImportFromFile(void)
{
	int res;
	char buf[512];
	gchar *defpath = NULL;
	GtkFileFilter *filters[2];

	// Dialog with buttons
	GtkWidget *pOpenDlg = gtk_file_chooser_dialog_new("Import locations from a file", GTK_WINDOW(m_pDlg), 
		GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pOpenDlg));
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*.csv");
	gtk_file_filter_set_name(filters[0], "CSV files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pOpenDlg), filters[0]);

	// Dialog icon
	gchar *icon = get_icon_file("import");
	gtk_window_set_icon(GTK_WINDOW(pOpenDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Initial selection
	gchar *path = CConfig::GetStr("ObserverDlg", "ImportPath", NULL);
	if (path)
		defpath = g_path_get_dirname(path);
	else
		defpath = g_strdup(get_user_data_dir());
	if (defpath && g_file_test(defpath, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pOpenDlg), defpath);
	g_free(path);
	g_free(defpath);

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(pOpenDlg))==GTK_RESPONSE_ACCEPT) {
		gchar *fpath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pOpenDlg));
		CConfig::SetStr("ObserverDlg", "ImportPath", fpath);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), NULL);
		res = m_UserList.Import(fpath);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
		g_free(fpath);
		gtk_widget_hide(pOpenDlg);
		if (res==1){
			sprintf(buf, "One location was successfully imported.");
			ShowInformation(GTK_WINDOW(m_pDlg), buf);
		} else if (res>1) {
			sprintf(buf, "%d locations were successfully imported.", res);
			ShowInformation(GTK_WINDOW(m_pDlg), buf);
		} else {
			sprintf(buf, "No location was successfully imported.");
			ShowError(GTK_WINDOW(m_pDlg), buf);
		}
		if (res>0)
			m_UserFileChanged = true;
	}

	gtk_widget_destroy(pOpenDlg);
}


//
// Export locations to a file
//
void CLocationDlg::ExportToFile(void)
{
	char *folder, *filename, *filepath;
	GtkFileFilter *filters[2];

	// Create a save dialog
	GtkWidget *pSaveDlg = gtk_file_chooser_dialog_new("Export locations to a file", GTK_WINDOW(m_pDlg), 
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pSaveDlg));
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(pSaveDlg), true);
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*.csv");
	gtk_file_filter_set_name(filters[0], "CSV files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(pSaveDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Initial folder and file name
	folder = CConfig::GetStr("ObserverDlg", "ExportDir", NULL);
	if (!folder)
		folder = g_strdup(get_user_data_dir());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pSaveDlg), folder);
	g_free(folder);
	filename = CConfig::GetStr("ObserverDlg", "ExportFile", "locations.csv");
	if (filename) 
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pSaveDlg), filename);
	g_free(filename);

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(pSaveDlg))==GTK_RESPONSE_ACCEPT) {
		filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pSaveDlg));
		// Save folder and file name
		gchar *dirpath = g_path_get_dirname(filepath);
		CConfig::SetStr("ObserverDlg", "ExportDir", dirpath);
		g_free(dirpath);
		gchar *basename = g_path_get_basename(filepath);
		CConfig::SetStr("ObserverDlg", "ExportFile", basename);
		g_free(basename);
		// Make target file
		m_UserList.Export(filepath);
		g_free(filepath);
	}

	gtk_widget_destroy(pSaveDlg);
}

//
// Edit coordinates
//
bool CLocationDlg::Execute(CLocation *pCoords, bool no_default)
{
	m_pCoords = pCoords;
	SetUserData(m_pCoords);
	SetRefData(g_Project->Location());
	if (no_default) {
		gtk_widget_set_sensitive(m_DefBtn, FALSE);
		if (m_SelectMode == DEFAULT_LOCATION)
			SetSelectMode(MANUAL_ENTRY);
	}
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT)
		return false;

	CConfig::SetLastLocation(*pCoords);
	return true;
}

//
// Handle dialog response
//
void CLocationDlg::response_dialog(GtkDialog *pDlg, gint response_id, CLocationDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CLocationDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Read and check input
		return GetData(m_pCoords, false);

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_OBSERVER_COORDS);
		return false;
	}
	return true;
}

void CLocationDlg::OnRowActivated(GtkTreeView *view, GtkTreePath *path)
{
	gtk_dialog_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
}
