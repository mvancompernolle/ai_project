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
#include "configuration.h"
#include "main.h"
#include "project.h"
#include "ctxhelp.h"
#include "preferences_dlg.h"
#include "sound.h"

enum tPageId
{
	PAGE_APPEARANCE,
	PAGE_NOTIFICATIONS,
	PAGE_FILES_AND_DIRECTORIES,
	PAGE_PERFORMANCE,
	PAGE_DEBUGGING
};

static const struct tColumnDef {
	const char *caption;
	int bitmask;
} Columns[] = {
	{ "Date and time (UTC)",1 << 0 },
	{ "Julian date",		1 << 1 },
	{ "Exposure",			1 << 2 },
	{ "Temperature",		1 << 3 },
	{ "Filter",				1 << 4 },
	{ "Stars",				1 << 5 },
	{ "Original file",		1 << 6 },
	{ "Temporary file",		1 << 7 },
	{ "Offset X and Y",		1 << 8 },
	{ NULL }
};

struct tColumnData {
	GtkWidget *btn;
	const tColumnDef *info;
};

static const struct tCatalogInfo {
	const char *caption, *filename;
	int par_use, par_path;
} Catalogs[] = {
	{ "GCVS", "iii.dat", CConfig::SEARCH_GCVS, CConfig::GCVS_PATH },
	{ "NSV", "nsv.dat", CConfig::SEARCH_NSV,  CConfig::NSV_PATH },
	{ "NSVS", "nsvs.dat", CConfig::SEARCH_NSVS, CConfig::NSVS_PATH },
	{ NULL }
};

struct tCatalogData {
	GtkWidget *toggle, *label, *entry, *browse;
	const tCatalogInfo *info;
};

typedef gchar *getDefault(int param_id);

static gchar *GetDefaultDir(int param_id);

static const struct tDirInfo {
	const char *caption, *caption2;
	int param_id;
	getDefault *def_fn;
} Dirs[] = {
	{ "User-defined profiles", "user-defined profiles", CConfig::USER_PROFILES, &GetDefaultDir },
	{ NULL }
};

struct tDirData {
	GtkWidget *toggle, *label, *entry, *browse;
	const tDirInfo *info;
};

static GtkWidget *add_label_with_markup(GtkWidget *vbox, const gchar *text)
{
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), text);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	return label;
}

static GtkWidget *add_check_button(GtkWidget *vbox, const gchar *text,
	const gchar *tooltip)
{
	GtkWidget *btn = gtk_check_button_new_with_label(text);
	gtk_widget_set_tooltip_text(btn, tooltip);
	gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, TRUE, 0);
	return btn;
}

static gchar *GetDefaultDir(int param_id)
{
	switch (param_id)
	{
	case CConfig::USER_PROFILES:
		return g_build_filename(get_user_data_dir(), "Profiles", NULL);	// First letter is uppercase!
	}
	return NULL;
}

//-------------------------   CONFIGURATION DIALOG   --------------------------------

CPreferencesDlg::CPreferencesDlg(GtkWindow *pParent):m_CurrentPage(NULL), m_Columns(NULL), 
	m_Catalogs(NULL), m_Dirs(NULL), m_Updating(false)
{
	GtkWidget *hbox_list_page, *tbox, *list_frame, *vbox, *vbox_page;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	GtkRequisition sreq;
	GdkRectangle rc;

	m_PageId = CConfig::GetInt("EnvironmentDlg", "LastPage", PAGE_APPEARANCE, 0, PAGE_DEBUGGING);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Environment options", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_widget_set_size_request(m_pDlg, 280, 160);

	// Dialog icon
	gchar *icon = get_icon_file("environment");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	hbox_list_page = gtk_hbox_new(FALSE, 4);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), hbox_list_page);
	gtk_container_set_border_width(GTK_CONTAINER(hbox_list_page), 0);

	// List of categories
	m_PageList = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, GTK_TYPE_WIDGET, GDK_TYPE_PIXBUF);
	m_PageView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_PageList));
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_PageView), col);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", 3);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView)), GTK_SELECTION_BROWSE);
	list_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(list_frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(list_frame), m_PageView);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_PageView), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView));
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);
	gtk_box_pack_start(GTK_BOX(hbox_list_page), list_frame, FALSE, TRUE, 0);
	gtk_widget_set_size_request(list_frame, 200, -1);

	m_PageScroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_PageScroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(hbox_list_page), m_PageScroller, TRUE, TRUE, 0);
	vbox_page = gtk_vbox_new(FALSE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_PageScroller), vbox_page);

    m_TitleFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(m_TitleFrame), GTK_SHADOW_NONE);
    gtk_box_pack_start(GTK_BOX(vbox_page), m_TitleFrame, FALSE, FALSE, 0);
	m_PageTitle = gtk_label_new("Title");
    gtk_container_add(GTK_CONTAINER(m_TitleFrame), m_PageTitle);

	GtkWidget *hsep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox_page), hsep, FALSE, FALSE, 4);
	
	m_PageFrame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(vbox_page), m_PageFrame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(m_PageFrame), GTK_SHADOW_NONE);
	
	// Appearance
	vbox = CreatePage(PAGE_APPEARANCE, "Appearance", "appearance", TRUE);
	add_label_with_markup(vbox, "<b>Charts and images:</b>");
	m_NegativeCharts = add_check_button(vbox, "Render stars in black color on white background.", NULL);
	m_RowsUpward = add_check_button(vbox, "Row number in FITS images increases upward.", "The convention is upward orientation (checked), Muniwin software used the downward orientation (unchecked)");
	add_label_with_markup(vbox, "<b>Which columns shall be displayed in the main window?</b>");
	tbox = gtk_table_new(0, 2, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	for (int i=0; Columns[i].caption; i++) {
		tColumnData *data = (tColumnData*)g_malloc(sizeof(tColumnData));
		data->info = &Columns[i];
		data->btn = gtk_check_button_new_with_label(Columns[i].caption);
		gtk_table_attach_defaults(GTK_TABLE(tbox), data->btn, i%2, (i%2)+1, i/2, (i/2)+1);
		m_Columns = g_slist_prepend(m_Columns, data);
	}

	// Notification
	vbox = CreatePage(PAGE_NOTIFICATIONS, "Notifications", "notifications", TRUE);
	m_SoundFinished = add_check_button(vbox, "Play sound when a process has been finished", NULL);
	if (!SoundIsSupported()) 
		add_label_with_markup(vbox, "The program was compiled without sound support");
	
	// Files and directories
	vbox = CreatePage(PAGE_FILES_AND_DIRECTORIES, "Files and directories", "files", TRUE);
	add_label_with_markup(vbox, "<b>Use the following catalogues to look up the object coordinates:</b>");
	add_label_with_markup(vbox, "Check the item and fill in the path to the catalog files.");
	tbox = gtk_table_new(0, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 0);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 0, 24);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	for (int i=0; Catalogs[i].caption; i++) {
		tCatalogData *data = (tCatalogData*)g_malloc(sizeof(tCatalogData));
		data->info = &Catalogs[i];
		data->toggle = gtk_check_button_new_with_label(Catalogs[i].caption);
		g_signal_connect(G_OBJECT(data->toggle), "toggled", G_CALLBACK(check_box_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(tbox), data->toggle, 0, 3, i*2, (i*2)+1);
		data->label = gtk_label_new("");
		gtk_misc_set_alignment(GTK_MISC(data->label), 0.0, 0.5);
		gtk_table_attach(GTK_TABLE(tbox), data->label, 0, 1, i*2+1, (i*2)+2, GTK_FILL, GTK_FILL, 0, 0);
		data->entry = gtk_entry_new();
		gtk_table_attach(GTK_TABLE(tbox), data->entry, 1, 2, i*2+1, (i*2)+2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		data->browse = gtk_button_new_with_label("...");
		gtk_table_attach(GTK_TABLE(tbox), data->browse, 2, 3, i*2+1, (i*2)+2, GTK_FILL, GTK_FILL, 0, 0);
		g_signal_connect(G_OBJECT(data->browse), "clicked", G_CALLBACK(filebutton_clicked), this);
		m_Catalogs = g_slist_prepend(m_Catalogs, data);
	}
	add_label_with_markup(vbox, "<b>Use the following direcotories:</b>");
	add_label_with_markup(vbox, "Check the item and specify your own path to the directory.");
	tbox = gtk_table_new(0, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 0);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 0, 24);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	for (int i=0; Dirs[i].caption; i++) {
		tDirData *data = (tDirData*)g_malloc(sizeof(tDirData));
		data->info = &Dirs[i];
		data->toggle = gtk_check_button_new_with_label(Dirs[i].caption);
		g_signal_connect(G_OBJECT(data->toggle), "toggled", G_CALLBACK(check_box_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(tbox), data->toggle, 0, 3, i*2, (i*2)+1);
		data->label = gtk_label_new("");
		gtk_misc_set_alignment(GTK_MISC(data->label), 0.0, 0.5);
		gtk_table_attach(GTK_TABLE(tbox), data->label, 0, 1, i*2+1, (i*2)+2, GTK_FILL, GTK_FILL, 0, 0);
		data->entry = gtk_entry_new();
		gtk_table_attach(GTK_TABLE(tbox), data->entry, 1, 2, i*2+1, (i*2)+2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		data->browse = gtk_button_new_with_label("...");
		gtk_table_attach(GTK_TABLE(tbox), data->browse, 2, 3, i*2+1, (i*2)+2, GTK_FILL, GTK_FILL, 0, 0);
		g_signal_connect(G_OBJECT(data->browse), "clicked", G_CALLBACK(filebutton_clicked), this);
		m_Dirs = g_slist_prepend(m_Dirs, data);
	}

	// Performance
	vbox = CreatePage(PAGE_PERFORMANCE, "Performance", "performance", TRUE);
	m_NoPreviews = add_check_button(vbox, "Do not show previews in \"Open file\" dialogs", NULL);

	// Testing and debugging
	vbox = CreatePage(PAGE_DEBUGGING, "Testing and debugging", "debugging", TRUE);
	m_DebugOutputs = add_check_button(vbox, "Print debug messages",
		"Enable printing of test and debug messages to the message log.");

	// Dialog size
	m_DlgWidth = 320; m_DlgHeight = 240;
    gtk_tree_model_foreach(GTK_TREE_MODEL(m_PageList), SetMaxDialogSize, this);
	gtk_widget_show_all(m_TitleFrame);
	gtk_widget_size_request(m_TitleFrame, &sreq);
	m_DlgHeight += sreq.height;
	gtk_widget_size_request(hsep, &sreq);
	m_DlgHeight += sreq.height;
	gtk_widget_size_request(m_PageView, &sreq);
	m_DlgWidth += sreq.width + 24;
	gtk_widget_size_request(GTK_DIALOG(m_pDlg)->action_area, &sreq);
	m_DlgHeight += sreq.height + 56;
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0) {
		int maxw = RoundToInt(0.9*rc.width);
		if (m_DlgWidth > maxw)
			m_DlgWidth = maxw;
		int maxh = RoundToInt(0.8*rc.height);
		if (m_DlgHeight > maxh)
			m_DlgHeight = maxh;
	}
	gtk_window_set_default_size(GTK_WINDOW(m_pDlg), m_DlgWidth, m_DlgHeight);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CPreferencesDlg::~CPreferencesDlg()
{
	g_slist_foreach(m_Columns, (GFunc)g_free, NULL);
	g_slist_free(m_Columns);
	g_slist_foreach(m_Catalogs, (GFunc)g_free, NULL);
	g_slist_free(m_Catalogs);
	g_slist_foreach(m_Dirs, (GFunc)g_free, NULL);
	g_slist_free(m_Dirs);
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_PageList);
}

gboolean CPreferencesDlg::SetMaxDialogSize(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	CPreferencesDlg *pMe = (CPreferencesDlg*)data;
	GtkWidget *page = NULL;
	GtkRequisition sreq;

	gtk_tree_model_get(model, iter, 2, &page, -1);
	gtk_widget_show_all(page);
    gtk_widget_size_request(page, &sreq);
	if (sreq.width > pMe->m_DlgWidth)
		pMe->m_DlgWidth = sreq.width;
	if (sreq.height > pMe->m_DlgHeight)
		pMe->m_DlgHeight = sreq.height;
    return FALSE;
}

GtkWidget *CPreferencesDlg::CreatePage(int id, const gchar *caption, const gchar *icon, bool def_btn)
{
	GtkTreeIter iter;

	GtkWidget *box = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(box), 4);
	gtk_list_store_append(GTK_LIST_STORE(m_PageList), &iter);
	gtk_list_store_set(GTK_LIST_STORE(m_PageList), &iter, 0, id, 1, caption, 2, box, -1);
	if (icon) {
		char *fpath = get_icon_file(icon);
		GdkPixbuf *i = gdk_pixbuf_new_from_file(fpath, NULL);
		gtk_list_store_set(GTK_LIST_STORE(m_PageList), &iter, 3, i, -1);
		g_free(fpath);
	}
	if (def_btn) {
		GtkWidget *btnbox = gtk_hbutton_box_new();
		gtk_button_box_set_layout(GTK_BUTTON_BOX(btnbox), GTK_BUTTONBOX_START);
		gtk_box_pack_end(GTK_BOX(box), btnbox, FALSE, TRUE, 0);
		GtkWidget *btn = gtk_button_new_with_label("Set defaults");
		gtk_widget_set_tooltip_text(btn, "Set parameters on the actual page to the default values");
		g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(setdefaults_clicked), this);
		gtk_container_add(GTK_CONTAINER(btnbox), btn);
	}
	return box;
}

void CPreferencesDlg::Execute()
{
	bool res;

	LoadConfig();
	ShowPage(m_PageId);
	UpdateControls();
	res = gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT;
	if (res) {
		SaveConfig();
		CConfig::Flush();
		g_Project->ClearThumbnails();
	}
}

void CPreferencesDlg::response_dialog(GtkWidget *pDlg, gint response_id, CPreferencesDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CPreferencesDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Commit changes
		return OnCloseQuery();

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_ENVIRONMENT);
		return false;
	}
	return true;
}

void CPreferencesDlg::LoadConfig()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_NegativeCharts), 
		CConfig::GetBool(CConfig::NEGATIVE_CHARTS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_NoPreviews), 
		CConfig::GetBool(CConfig::DISABLE_PREVIEWS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SoundFinished), 
		CConfig::GetBool(CConfig::EVENT_SOUNDS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DebugOutputs), 
		CConfig::GetBool(CConfig::DEBUG_OUTPUTS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RowsUpward), 
		CConfig::GetBool(CConfig::ROWS_UPWARD));

	int cols = CConfig::GetInt(CConfig::FILE_LIST_COLUMNS);
	for (GSList *ptr=m_Columns; ptr!=NULL; ptr=ptr->next) {
		tColumnData *data = (tColumnData*)ptr->data;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->btn), (cols & data->info->bitmask)!=0);
	}

	for (GSList *ptr=m_Catalogs; ptr!=NULL; ptr=ptr->next) {
		tCatalogData *data = (tCatalogData*)ptr->data;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->toggle), 
			CConfig::GetBool((CConfig::tParameter)data->info->par_use));
		gchar *text = CConfig::GetStr((CConfig::tParameter)data->info->par_path);
		gtk_entry_set_text(GTK_ENTRY(data->entry), (text!=NULL ? text : ""));
		g_free(text);
	}

	for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
		tDirData *data = (tDirData*)ptr->data;
		gchar *path = CConfig::GetStr((CConfig::tParameter)data->info->param_id);
		if (path && path[0]!='\0') {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->toggle), TRUE);
			gtk_entry_set_text(GTK_ENTRY(data->entry), path);
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->toggle), FALSE);
			gchar *defpath = data->info->def_fn(data->info->param_id);
			gtk_entry_set_text(GTK_ENTRY(data->entry), (defpath!=NULL ? defpath : ""));
			g_free(defpath);
		}
		g_free(path);
	}
}

void CPreferencesDlg::SaveConfig()
{
	CConfig::SetBool(CConfig::NEGATIVE_CHARTS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_NegativeCharts))!=0);
	CConfig::SetBool(CConfig::DISABLE_PREVIEWS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_NoPreviews))!=0);
	CConfig::SetBool(CConfig::EVENT_SOUNDS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SoundFinished))!=0);
	CConfig::SetBool(CConfig::DEBUG_OUTPUTS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_DebugOutputs))!=0);
	CConfig::SetBool(CConfig::ROWS_UPWARD,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RowsUpward))!=0);

	int cols = 0;
	for (GSList *ptr=m_Columns; ptr!=NULL; ptr=ptr->next) {
		tColumnData *data = (tColumnData*)ptr->data;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->btn)))
			cols |= data->info->bitmask;
	}
	CConfig::SetInt(CConfig::FILE_LIST_COLUMNS, cols);

	for (GSList *ptr=m_Catalogs; ptr!=NULL; ptr=ptr->next) {
		tCatalogData *data = (tCatalogData*)ptr->data;
		CConfig::SetBool((CConfig::tParameter)data->info->par_use,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->toggle))!=0);
		const gchar *text = gtk_entry_get_text(GTK_ENTRY(data->entry));
		CConfig::SetStr((CConfig::tParameter)data->info->par_path, text);
	}

	for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
		tDirData *data = (tDirData*)ptr->data;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->toggle))) {
			const gchar *text = gtk_entry_get_text(GTK_ENTRY(data->entry));
			CConfig::SetStr((CConfig::tParameter)data->info->param_id, text);
		} else {
			CConfig::SetStr((CConfig::tParameter)data->info->param_id, "");
		}
	}
}

bool CPreferencesDlg::OnCloseQuery(void)
{
	return true;
}

void CPreferencesDlg::UpdateControls(void)
{
	switch (m_PageId)
	{
	case PAGE_FILES_AND_DIRECTORIES:
		for (GSList *ptr=m_Catalogs; ptr!=NULL; ptr=ptr->next) {
			tCatalogData *data = (tCatalogData*)ptr->data;
			gboolean ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->toggle));
			gtk_widget_set_sensitive(data->label, ok);
			gtk_widget_set_sensitive(data->entry, ok);
			gtk_widget_set_sensitive(data->browse, ok);
		}
		for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
			tDirData *data = (tDirData*)ptr->data;
			gboolean ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->toggle));
			gtk_widget_set_sensitive(data->label, ok);
			gtk_widget_set_sensitive(data->entry, ok);
			gtk_widget_set_sensitive(data->browse, ok);
		}
		break;

	case PAGE_NOTIFICATIONS:
		if (!SoundIsSupported())
			gtk_widget_hide(m_SoundFinished);
		break;

	default:
		break;
	}
}

void CPreferencesDlg::setdefaults_clicked(GtkButton *pWidget, CPreferencesDlg *pDlg)
{
	pDlg->OnSetDefaultsClicked(pWidget);
}

void CPreferencesDlg::OnSetDefaultsClicked(GtkButton *pButton)
{
	int cols;

	switch (m_PageId)
	{
	case PAGE_APPEARANCE:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_NegativeCharts), 
			CConfig::GetDefaultBool(CConfig::NEGATIVE_CHARTS));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RowsUpward), 
			CConfig::GetDefaultBool(CConfig::ROWS_UPWARD));
		cols = CConfig::GetDefaultInt(CConfig::FILE_LIST_COLUMNS);
		for (GSList *ptr=m_Columns; ptr!=NULL; ptr=ptr->next) {
			tColumnData *data = (tColumnData*)ptr->data;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->btn), (cols & data->info->bitmask)!=0);
		}
		break;

	case PAGE_NOTIFICATIONS:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SoundFinished), 
			CConfig::GetDefaultBool(CConfig::EVENT_SOUNDS));
		break;

	case PAGE_PERFORMANCE:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_NoPreviews), 
			CConfig::GetDefaultBool(CConfig::DISABLE_PREVIEWS));
		break;

	case PAGE_DEBUGGING:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DebugOutputs), 
			CConfig::GetDefaultBool(CConfig::DEBUG_OUTPUTS));
		break;

	case PAGE_FILES_AND_DIRECTORIES:
		for (GSList *ptr=m_Catalogs; ptr!=NULL; ptr=ptr->next) {
			tCatalogData *data = (tCatalogData*)ptr->data;
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(data->toggle), FALSE);
		}
		for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
			tDirData *data = (tDirData*)ptr->data;
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(data->toggle), FALSE);
		}
		break;
		
	default:
		break;
	}
}

void CPreferencesDlg::check_box_toggled(GtkToggleButton *togglebutton, CPreferencesDlg *pMe)
{
	pMe->OnCheckBoxToggled(togglebutton);
}

void CPreferencesDlg::OnCheckBoxToggled(GtkToggleButton *pButton)
{
	if (!gtk_toggle_button_get_active(pButton)) {
		for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
			tDirData *data = (tDirData*)ptr->data;
			if (GTK_TOGGLE_BUTTON(data->toggle) == pButton) {
				gchar *dpath = data->info->def_fn(data->info->param_id);
				gtk_entry_set_text(GTK_ENTRY(data->entry), dpath!=NULL ? dpath : "");
				g_free(dpath);
				break;
			}
		}
	}

	UpdateControls();
}

gint CPreferencesDlg::selection_changed(GtkTreeSelection *widget, CPreferencesDlg *pDlg)
{
	pDlg->OnSelectionChanged();
	return FALSE;
}

void CPreferencesDlg::OnSelectionChanged(void)
{
	int id;
	GtkWidget *page;
	gchar *caption, buf[1024];

	if (!m_Updating) {
		m_Updating = true;
		GtkTreeModel *model;
		GtkTreeIter iter;
		if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView)), &model, &iter)) {
			gtk_tree_model_get(model, &iter, 0, &id, 1, &caption, 2, &page, -1);
		} else {
			id = -1;
			page = NULL;
			caption = NULL;
		}
		if (page!=m_CurrentPage) {
			if (m_CurrentPage)
				gtk_container_remove(GTK_CONTAINER(m_PageFrame), m_CurrentPage);
			m_CurrentPage = page;
			if (m_CurrentPage)
				gtk_container_add(GTK_CONTAINER(m_PageFrame), m_CurrentPage);
			if (caption)
				sprintf(buf, "<span size='large'><b>%s</b></span>", caption);
			else
				buf[0] = '\0';
			gtk_label_set_markup(GTK_LABEL(m_PageTitle), buf);
		}
		m_PageId = id;
		if (m_PageId>=0)
			CConfig::SetInt("EnvironmentDlg", "LastPage", m_PageId);
		g_free(caption);
		UpdateControls();
		m_Updating = false;
	}
}

void CPreferencesDlg::ShowPage(int page_id)
{
	gint id;
	GtkTreeIter iter;

	gboolean ok = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_PageList), &iter);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(m_PageList), &iter, 0, &id, -1);
		if (id == page_id) {
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView));
			gtk_tree_selection_select_iter(selection, &iter);
			break;
		}
		ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_PageList), &iter);
	}
}

void CPreferencesDlg::filebutton_clicked(GtkButton *pWidget, CPreferencesDlg *pDlg)
{
	pDlg->OnFileButtonClicked(pWidget);
}

void CPreferencesDlg::OnFileButtonClicked(GtkButton *pButton)
{
	char buf[1024];

	for (GSList *ptr=m_Catalogs; ptr!=NULL; ptr=ptr->next) {
		tCatalogData *data = (tCatalogData*)ptr->data;
		if (GTK_BUTTON(data->browse) == pButton) {
			sprintf(buf, "Path to the %s catalog file (%s)", data->info->caption, data->info->filename);
			GtkWidget *pDlg = gtk_file_chooser_dialog_new(buf, GTK_WINDOW(m_pDlg), 
				GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
			gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pDlg));
			const gchar *fpath = gtk_entry_get_text(GTK_ENTRY(data->entry));
			if (!gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pDlg), fpath)) {
				gchar *dir = g_path_get_dirname(fpath);
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pDlg), dir);
				g_free(dir);
			}
			if (gtk_dialog_run(GTK_DIALOG(pDlg)) == GTK_RESPONSE_ACCEPT) {
				gchar *filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pDlg));
				gtk_entry_set_text(GTK_ENTRY(data->entry), filepath);
				g_free(filepath);
			}
			gtk_widget_destroy(pDlg);
			break;
		}
	}

	for (GSList *ptr=m_Dirs; ptr!=NULL; ptr=ptr->next) {
		tDirData *data = (tDirData*)ptr->data;
		if (GTK_BUTTON(data->browse) == pButton) {
			sprintf(buf, "Path to the %s", data->info->caption2);
			GtkWidget *pDlg = gtk_file_chooser_dialog_new(buf, GTK_WINDOW(m_pDlg), 
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
			gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pDlg));
			const gchar *dpath = gtk_entry_get_text(GTK_ENTRY(data->entry));
			if (dpath && g_file_test(dpath, G_FILE_TEST_IS_DIR))
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pDlg), dpath);
			if (gtk_dialog_run(GTK_DIALOG(pDlg)) == GTK_RESPONSE_ACCEPT) {
				gchar *dirpath = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pDlg));
				gtk_entry_set_text(GTK_ENTRY(data->entry), dirpath);
				g_free(dirpath);
			}
			gtk_widget_destroy(pDlg);
			break;
		}
	}
}
