/**************************************************************

object_dlg.cpp (C-Munipack project)
The 'Object coordinates' dialog
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
#include "object_dlg.h"
#include "main.h"
#include "varcat.h"
#include "ctxhelp.h"

//-------------------------   TABLES   ------------------------------------

enum tColumnId
{
	COL_ID,
	COL_NAME,
	COL_RA,
	COL_DEC,
	COL_SRC,
	COL_REM,
	NCOLS
};

struct tTreeViewColumn {
	const char *caption;		// Column name
	int column;					// Model column index
	double align;				// Column alignment
};

const static tTreeViewColumn SearchColumns[] = {
	{ "Designation",		COL_NAME },
	{ "Right ascension",	COL_RA, 1.0 },
	{ "Declination",		COL_DEC, 1.0 },
	{ "Source",				COL_SRC },
	{ "Remarks",			COL_REM },
	{ NULL }
};

const static tTreeViewColumn UserColumns[] = {
	{ "Designation",		OBJECT_NAME },
	{ "Right ascension",	OBJECT_RA, 1.0 },
	{ "Declination",		OBJECT_DEC, 1.0 },
	{ "Source",				OBJECT_SOURCE },
	{ "Remarks",			OBJECT_REMARKS },
	{ NULL }
};

//-------------------------   POPUP MENU   ------------------------------------

enum tCommandId
{
	CMD_ADD = 100,
	CMD_DELETE,
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

static const CPopupMenu::tPopupMenuItem SearchContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_ADD,			"_Add to bookmarks" },
	{ CPopupMenu::MB_END }
};

//-------------------------   HELPER FUNCTIONS   --------------------------------

static void make_path_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **pList)
{
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*pList = g_slist_prepend(*pList, rowref);
}

//-------------------------   OBJECT COORDINATES DIALOG   --------------------------------

//
// Constructor
//
CObjectDlg::CObjectDlg(GtkWindow *pParent):m_Updating(false), m_UserFileChanged(false),
	m_UserEntryChanged(false), m_SelPath(NULL)
{
	int i;
	GtkWidget *label, *vbox, *bbox, *scrwnd;
	GtkTreeSelection *selection;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Object coordinates", pParent, 
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
	gtk_widget_set_size_request(vbox, 520, 560);

	// Search result
	m_SearchList = gtk_list_store_new(NCOLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, 
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(m_SearchList), COL_NAME, GTK_SORT_ASCENDING);

	// Method
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Get object position</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_ManBtn = gtk_radio_button_new_with_label(NULL, "enter equatorial coordinates manually");
	gtk_widget_set_tooltip_text(m_ManBtn, "Check to specify object's coordinates manually");
	g_signal_connect(G_OBJECT(m_ManBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_ManBtn, FALSE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_ManBtn)); 
	m_CatBtn = gtk_radio_button_new_with_label(group, "get position from a catalog of variable stars");
	gtk_widget_set_tooltip_text(m_CatBtn, "Check to look up object's coordinates from the GCVS, NSV or NSVS catalog");
	g_signal_connect(G_OBJECT(m_CatBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_CatBtn, FALSE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_ManBtn)); 
	m_RefBtn = gtk_radio_button_new_with_label(group, "get position from the reference file");
	gtk_widget_set_tooltip_text(m_RefBtn, "Check to use object's coordinates stored in a reference frame that was used in matching");
	g_signal_connect(G_OBJECT(m_RefBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(vbox), m_RefBtn, FALSE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(NULL), FALSE, TRUE, 0);

	// Coordinates from the referece file
	m_AutoBox = gtk_table_new(5, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_AutoBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_AutoBox), 16);
	gtk_table_set_col_spacings(GTK_TABLE(m_AutoBox), 8);
	gtk_table_attach_defaults(GTK_TABLE(m_AutoBox), gtk_label_new(NULL), 0, 3, 0, 1);

	label = gtk_label_new("Designation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_AutoBox), label, 0, 1, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_AutoName = gtk_entry_new_with_max_length(256);
	gtk_widget_set_sensitive(m_AutoName, FALSE);
	gtk_table_attach(GTK_TABLE(m_AutoBox), m_AutoName, 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_AutoBox), label, 0, 1, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_AutoRA = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_AutoRA, FALSE);
	gtk_table_attach(GTK_TABLE(m_AutoBox), m_AutoRA, 1, 2, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_AutoBox), label, 2, 3, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_AutoBox), label, 0, 1, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_AutoDec = gtk_entry_new_with_max_length(32);
	gtk_widget_set_sensitive(m_AutoDec, FALSE);
	gtk_table_attach(GTK_TABLE(m_AutoBox), m_AutoDec, 1, 2, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_AutoBox), label, 2, 3, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(m_AutoBox), gtk_label_new(NULL), 0, 3, 4, 5);

	// User coordinates
	m_EntryBox = gtk_table_new(7, 5, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_EntryBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_EntryBox), 8);
	gtk_table_set_col_spacings(GTK_TABLE(m_EntryBox), 8);
	gtk_table_attach(GTK_TABLE(m_EntryBox), gtk_label_new(NULL), 0, 1, 0, 6, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(m_EntryBox), gtk_label_new(NULL), 4, 5, 0, 6, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	label = gtk_label_new("Designation");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryName = gtk_entry_new_with_max_length(256);
	gtk_widget_set_tooltip_text(m_EntryName, "Object's designation, e.g. RT And");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryName, 2, 3, 0, 1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryName), "changed", G_CALLBACK(entry_changed), this);

	label = gtk_label_new("Right ascension");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryRA = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_EntryRA, "Object's right ascension in hours, minutes and seconds, e.g. 12 34 56");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryRA, 2, 3, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[h m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 3, 4, 1, 2, (GtkAttachOptions)(GTK_FILL | 0), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryRA), "changed", G_CALLBACK(entry_changed), this);

	label = gtk_label_new("Declination");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryDec = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_EntryDec, "Object's declination in degrees, minutes and seconds, e.g. -12 34 56");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryDec, 2, 3, 2, 3, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	label = gtk_label_new("[\xC2\xB1""d m s]");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryDec), "changed", G_CALLBACK(entry_changed), this);

	label = gtk_label_new("Source");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	m_EntrySrc = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_EntrySrc, "Source of coordinates, e.g. catalog name");
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntrySrc, 2, 3, 3, 4, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntrySrc), "changed", G_CALLBACK(entry_changed), this);

	label = gtk_label_new("Remarks");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_EntryBox), label, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	m_EntryRem = gtk_entry_new_with_max_length(32);
	gtk_table_attach(GTK_TABLE(m_EntryBox), m_EntryRem, 2, 3, 4, 5, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(m_EntryRem), "changed", G_CALLBACK(entry_changed), this);

	// User catalog
	m_UserView = gtk_tree_view_new();
	gtk_widget_set_tooltip_text(m_UserView, "List of the predefined objects");
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
	gtk_table_attach_defaults(GTK_TABLE(m_EntryBox), scrwnd, 0, 5, 6, 7);

	// Popup menu
	m_UserMenu.Create(UserContextMenu);

	// Buttons
	bbox = gtk_hbutton_box_new();
	gtk_table_attach(GTK_TABLE(m_EntryBox), bbox, 1, 4, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	m_AddBtn = gtk_button_new_with_label("Add");
	gtk_widget_set_tooltip_text(m_AddBtn, "Store specified object's coordinates into a list of predefined objects");
	g_signal_connect(G_OBJECT(m_AddBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_AddBtn, false, false, 0);
	m_SaveBtn = gtk_button_new_with_label("Save");
	gtk_widget_set_tooltip_text(m_SaveBtn, "Save specified object's coordinates by updating selected item");
	g_signal_connect(G_OBJECT(m_SaveBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_SaveBtn, false, false, 0);
	m_DelBtn = gtk_button_new_with_label("Remove");
	gtk_widget_set_tooltip_text(m_DelBtn, "Remove selected item from the list of predefined objects");
	g_signal_connect(G_OBJECT(m_DelBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(bbox), m_DelBtn, false, false, 0);

	// Search box
	m_SearchBox = gtk_table_new(2, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), m_SearchBox, TRUE, TRUE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(m_SearchBox), 8);
	gtk_table_set_col_spacings(GTK_TABLE(m_SearchBox), 8);

	label = gtk_label_new("Search for");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(m_SearchBox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_FindText = gtk_entry_new_with_max_length(32);
	gtk_widget_set_tooltip_text(m_FindText, "Object's designation or a part of it");
	gtk_table_attach(GTK_TABLE(m_SearchBox), m_FindText, 1, 2, 0, 1, 
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	m_FindBtn = gtk_button_new_with_label("Search");
	gtk_widget_set_tooltip_text(m_FindBtn, "Start search");
	g_signal_connect(G_OBJECT(m_FindBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(m_SearchBox), m_FindBtn, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	// Search results
	m_SearchView = gtk_tree_view_new();
	gtk_widget_set_tooltip_text(m_SearchView, "List of objects that meet specified criteria");
	for (i=0; SearchColumns[i].caption!=NULL; i++) {
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		// Set column name and alignment
		gtk_tree_view_column_set_title(col, SearchColumns[i].caption);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_SearchView), col);
		// Add text renderer
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		g_object_set(renderer, "xalign", SearchColumns[i].align, NULL);
		gtk_tree_view_column_add_attribute(col, renderer, "text", SearchColumns[i].column);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_SearchView), GTK_TREE_MODEL(m_SearchList));
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(m_SearchView), true);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_SearchView), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_SearchView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	g_signal_connect(G_OBJECT(m_SearchView), "row-activated", G_CALLBACK(row_activated), this);
	g_signal_connect(G_OBJECT(m_SearchView), "button_press_event", G_CALLBACK(button_press_event), this);
	scrwnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_SearchView);
	gtk_table_attach_defaults(GTK_TABLE(m_SearchBox), scrwnd, 0, 3, 1, 2);

	// Popup menu
	m_SearchMenu.Create(SearchContextMenu);

	// User defined objects
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), NULL);
	m_UserList.Load();
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
	gtk_tree_path_free(m_SelPath);
	m_SelPath = NULL;
	m_UserFileChanged = false;
	m_UserEntryChanged = true;

	// Catalog files
	gtk_widget_set_sensitive(m_CatBtn, VarCat_Test());
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_SearchView), NULL);
	gtk_list_store_clear(m_SearchList);
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_SearchView), GTK_TREE_MODEL(m_SearchList));

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}


//
// Destructor
//
CObjectDlg::~CObjectDlg()
{
	if (m_UserFileChanged)
		m_UserList.Save();
	g_object_unref(m_SearchList);
	gtk_tree_path_free(m_SelPath);
	gtk_widget_destroy(m_pDlg);
}


//
// Set user coordinates
//
void CObjectDlg::SetUserData(const CObjectCoords *data)
{
	char buf[64];
	double ra, dec;

	if (data->Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_EntryName), data->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_EntryName), "");
	if (cmpack_strtora(data->RA(), &ra)==0) {
		cmpack_ratostr(ra, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_EntryRA), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_EntryRA), "");
	if (cmpack_strtodec(data->Dec(), &dec)==0) {
		cmpack_dectostr(dec, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_EntryDec), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_EntryDec), "");
	if (data->Source()) 
		gtk_entry_set_text(GTK_ENTRY(m_EntrySrc), data->Source());
	else
		gtk_entry_set_text(GTK_ENTRY(m_EntrySrc), "");
	if (data->Remarks()) 
		gtk_entry_set_text(GTK_ENTRY(m_EntryRem), data->Remarks());
	else
		gtk_entry_set_text(GTK_ENTRY(m_EntryRem), "");
}


//
// Set reference coordinates
//
void CObjectDlg::SetRefData(const CObjectCoords *data)
{
	char buf[64];
	double ra, dec;

	gtk_widget_set_sensitive(m_RefBtn, data->Valid());
	if (data->Name()) 
		gtk_entry_set_text(GTK_ENTRY(m_AutoName), data->Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_AutoName), "");
	if (cmpack_strtora(data->RA(), &ra)==0) {
		cmpack_ratostr(ra, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_AutoRA), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_AutoRA), "");
	if (cmpack_strtodec(data->Dec(), &dec)==0) {
		cmpack_dectostr(dec, buf, 64);
		gtk_entry_set_text(GTK_ENTRY(m_AutoDec), buf);
	} else
		gtk_entry_set_text(GTK_ENTRY(m_AutoDec), "");
}


//
// Get coordinates
//
bool CObjectDlg::GetData(CObjectCoords *data, bool name_required)
{
	bool res = true;
	gchar *name, *ra, *dec, *source, *remarks;
	char buf[64];
	double x, y;
	GtkTreeIter iter;

	data->Clear();
	name = ra = dec = source = remarks = NULL;
	if (m_SelectMode == REFERENCE_FRAME) {
		name = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_AutoName)));
		ra = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_AutoRA)));
		dec = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_AutoDec)));
	} else if (m_SelectMode == MANUAL_ENTRY) {
		name = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_EntryName)));
		ra = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_EntryRA)));
		dec = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_EntryDec)));
		source = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_EntrySrc)));
		remarks = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_EntryRem)));
	} else {
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_SearchView));
		if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
			gtk_tree_model_get(GTK_TREE_MODEL(m_SearchList), &iter, COL_NAME,
				&name, COL_RA, &ra, COL_DEC, &dec, COL_SRC, &source, COL_REM, &remarks, -1);
		} else {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select a row in the table of objects.");
			res = false;
		}
	}
	if (res) {
		name = g_strstrip(name);
		if (name_required && (!name || *name=='\0')) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter the object designation.");
			res = false;
		} else
		if (!ra || *ra=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter the object right ascension.");
			res = false;
		} else
		if (!dec || *dec=='\0') {
			ShowError(GTK_WINDOW(m_pDlg), "Please, enter the object declination.");
			res = false;
		} else
		if (cmpack_strtora(ra, &x)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of right ascension.");
			res = false;
		} else
		if (cmpack_strtodec(dec, &y)!=0) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid value of declination.");
			res = false;
		}
		remarks = g_strstrip(remarks);
	}
	if (res) {
		data->SetName(name);
		cmpack_ratostr(x, buf, 64);
		data->SetRA(buf);
		cmpack_dectostr(y, buf, 64);
		data->SetDec(buf);
		data->SetSource(source);
		data->SetRemarks(remarks);
	}
	g_free(name);
	g_free(ra);
	g_free(dec);	
	g_free(source);
	g_free(remarks);
	return res;
}


//
// Button click handler
//
void CObjectDlg::button_clicked(GtkWidget *button, CObjectDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CObjectDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (!m_Updating) {
		if (pBtn==m_RefBtn)
			SetSelectMode(REFERENCE_FRAME);
		else if (pBtn==m_ManBtn)
			SetSelectMode(MANUAL_ENTRY);
		else if (pBtn==m_CatBtn)
			SetSelectMode(CATALOG_FILE);
		else if (pBtn==m_FindBtn)
			FindObjects();
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
void CObjectDlg::entry_changed(GtkWidget *widget, CObjectDlg *pDlg)
{
	pDlg->OnEntryChanged(widget);
}

void CObjectDlg::OnEntryChanged(GtkWidget *widget)
{
	m_UserEntryChanged = true;
	UpdateControls();
}


//
// Change dialog mode
//
void CObjectDlg::SetSelectMode(tSelectMode mode)
{
	if (!m_Updating) {
		m_Updating = true;
		m_SelectMode = mode;
		switch (mode)
		{
		case REFERENCE_FRAME:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_CatBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), true);
			gtk_widget_hide(m_SearchBox);
			gtk_widget_hide(m_EntryBox);
			gtk_widget_show(m_AutoBox);
			break;
		case MANUAL_ENTRY:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_CatBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), true);
			gtk_widget_hide(m_SearchBox);
			gtk_widget_hide(m_AutoBox);
			gtk_widget_show(m_EntryBox);
			break;
		case CATALOG_FILE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ManBtn), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_CatBtn), true);
			gtk_widget_hide(m_EntryBox);
			gtk_widget_hide(m_AutoBox);
			gtk_widget_show(m_SearchBox);
			break;
		}
		UpdateControls();
		m_Updating = false;
	}
}


//
// Enable and disable controls
//
void CObjectDlg::UpdateControls(void)
{
	bool name, coords;
	int selected;

	if (m_SelectMode == MANUAL_ENTRY) {
		name = strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryName)))>0;
		coords = strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryRA)))>0 &&
			strlen(gtk_entry_get_text(GTK_ENTRY(m_EntryDec)))>0;
		GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
		selected = gtk_tree_selection_count_selected_rows(pSel);
		gtk_widget_set_sensitive(m_AddBtn, (!m_SelPath || m_UserEntryChanged) && name && coords);
		gtk_widget_set_sensitive(m_SaveBtn, (selected==1 && m_SelPath && m_UserEntryChanged) && name && coords);
		gtk_widget_set_sensitive(m_DelBtn, selected>0);
	}
}


//
// Execute search of variables
//
void CObjectDlg::FindObjects(void)
{
	char *text = g_locale_from_utf8(gtk_entry_get_text(GTK_ENTRY(m_FindText)), -1, NULL, NULL, NULL);
	if (strlen(text)>=2) {
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_SearchView), NULL);
		gtk_list_store_clear(m_SearchList);
		VarCat_Search(text, (tVarCatProc*)AddToList, this);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_SearchView), GTK_TREE_MODEL(m_SearchList));
	} else {
		ShowError(GTK_WINDOW(m_pDlg), "Please, enter at least 2 characters as a search pattern.");
	}
	g_free(text);
	UpdateControls();
}


//
// Add item to the list of found items
//
void CObjectDlg::AddToList(const char *objname, const char *ra, const char *dec,
		const char *catalog, const char *comment, CObjectDlg *data)
{
	int id;
	GtkTreeIter iter;

	id = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(data->m_SearchList), NULL);
	gtk_list_store_append(data->m_SearchList, &iter);
	gtk_list_store_set(data->m_SearchList, &iter, COL_ID, id, COL_NAME, objname, 
		COL_RA, ra, COL_DEC, dec, COL_SRC, catalog, COL_REM, comment, -1);
}

//
// Double click on table confirms the dialog
//
void CObjectDlg::row_activated(GtkTreeView *tree_view, GtkTreePath *path, 
		GtkTreeViewColumn *column, CObjectDlg *pMe)
{
	pMe->OnRowActivated(tree_view, path);
}

//
// Mouse button handler
//
gint CObjectDlg::button_press_event(GtkWidget *widget, GdkEventButton *event, CObjectDlg *pMe)
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
void CObjectDlg::selection_changed(GtkTreeSelection *selection, CObjectDlg *pMe)
{
	pMe->OnSelectionChanged(selection);
}

void CObjectDlg::OnSelectionChanged(GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	CObjectCoords data;

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
		} else
		if (model==GTK_TREE_MODEL(m_SearchList)) {
			gchar *name, *ra, *dec, *source, *remarks;
			gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)(list->data));
			gtk_tree_model_get(GTK_TREE_MODEL(m_SearchList), &iter, COL_NAME,
				&name, COL_RA, &ra, COL_DEC, &dec, COL_SRC, &source, COL_REM, &remarks, -1);
			gtk_entry_set_text(GTK_ENTRY(m_EntryName), name);
			gtk_entry_set_text(GTK_ENTRY(m_EntryRA), ra);
			gtk_entry_set_text(GTK_ENTRY(m_EntryDec), dec);
			gtk_entry_set_text(GTK_ENTRY(m_EntryRem), remarks);
			gtk_entry_set_text(GTK_ENTRY(m_EntrySrc), source);
			g_free(name);
			g_free(ra);
			g_free(dec);
			g_free(remarks);
			g_free(source);
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
void CObjectDlg::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
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
	if (widget==m_UserView) {
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
	} else {
		m_SearchMenu.Enable(CMD_ADD, selected>0);
		switch (m_SearchMenu.Execute(event))
		{
		case CMD_ADD:
			AddToTable();
			break;
		}
	}
}

//
// Select all bookmarks
//
void CObjectDlg::SelectAll(void)
{
	GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_UserView));
	gtk_tree_selection_select_all(pSel);
}


//
// Add coordinates to bookmark
//
void CObjectDlg::AddToTable(void)
{
	CObjectCoords data;
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
void CObjectDlg::SaveToTable(void)
{
	CObjectCoords data;
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
void CObjectDlg::RemoveFromTable(void)
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
// Import objects from an external file
//
void CObjectDlg::ImportFromFile(void)
{
	int res;
	gchar *defpath = NULL;
	char buf[512];
	GtkFileFilter *filters[2];

	// Dialog with buttons
	GtkWidget *pOpenDlg = gtk_file_chooser_dialog_new("Import objects from a file", GTK_WINDOW(m_pDlg), 
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
	gchar *path = CConfig::GetStr("ObjectDlg", "ImportPath", NULL);
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
		CConfig::SetStr("ObjectDlg", "ImportPath", fpath);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), NULL);
		res = m_UserList.Import(fpath);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_UserView), m_UserList.List());
		g_free(fpath);
		gtk_widget_hide(pOpenDlg);
		if (res==1){
			sprintf(buf, "One object was successfully imported.");
			ShowInformation(GTK_WINDOW(m_pDlg), buf);
		} else if (res>1) {
			sprintf(buf, "%d objects were successfully imported.", res);
			ShowInformation(GTK_WINDOW(m_pDlg), buf);
		} else {
			sprintf(buf, "No object was successfully imported.");
			ShowError(GTK_WINDOW(m_pDlg), buf);
		}
		if (res>0)
			m_UserFileChanged = true;
	}

	gtk_widget_destroy(pOpenDlg);
}


//
// Export objects to a file
//
void CObjectDlg::ExportToFile(void)
{
	char *folder, *filename, *filepath;
	GtkFileFilter *filters[2];

	// Create a save dialog
	GtkWidget *pSaveDlg = gtk_file_chooser_dialog_new("Export objects to a file", GTK_WINDOW(m_pDlg), 
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
	folder = CConfig::GetStr("ObjectDlg", "ExportDir", NULL);
	if (!folder)
		folder = g_strdup(get_user_data_dir());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pSaveDlg), folder);
	g_free(folder);
	filename = CConfig::GetStr("ObjectDlg", "ExportFile", "objects.csv");
	if (filename) 
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pSaveDlg), filename);
	g_free(filename);

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(pSaveDlg))==GTK_RESPONSE_ACCEPT) {
		filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pSaveDlg));
		// Save folder and file name
		gchar *dirpath = g_path_get_dirname(filepath);
		CConfig::SetStr("ObjectDlg", "ExportDir", dirpath);
		g_free(dirpath);
		gchar *basename = g_path_get_basename(filepath);
		CConfig::SetStr("ObjectDlg", "ExportFile", basename);
		g_free(basename);
		// Make target file
		m_UserList.Export(filepath);
		g_free(filepath);
	}

	gtk_widget_destroy(pSaveDlg);
}

//-------------------------   OBJECT COORDINATES DIALOG   --------------------------------

//
// Edit coordinates
//
bool CObjectDlg::Execute(CObjectCoords *coords)
{
	m_pCoords = coords;
	SetUserData(m_pCoords);
	SetRefData(g_Project->ObjectCoords());
	SetSelectMode(MANUAL_ENTRY);
	return gtk_dialog_run(GTK_DIALOG(m_pDlg)) == GTK_RESPONSE_ACCEPT;
}

//
// Handle dialog response
//
void CObjectDlg::response_dialog(GtkDialog *pDlg, gint response_id, CObjectDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CObjectDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Read and check input
		if (!GetData(m_pCoords, false))
			return false;
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_OBJECT_COORDINATES);
		return false;
	}
	return true;
}

void CObjectDlg::OnRowActivated(GtkTreeView *view, GtkTreePath *path)
{
	gtk_dialog_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT);
}
