/**************************************************************

matching_dlg.cpp (C-Munipack project)
The 'Match stars' dialog
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

#include "project.h"
#include "configuration.h"
#include "configuration.h"
#include "matching_dlg.h"
#include "progress_dlg.h"
#include "catfile_dlg.h"
#include "phtfile_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"
#include "project_dlg.h"

//-------------------------   TABLES   ------------------------------------

enum tFrameColumnId
{
	FCOL_ID,
	FCOL_STATE,
	FCOL_JULDAT,
	FCOL_STARS,
	FNCOLS
};

enum tCatalogColumnId
{
	CCOL_NAME,
	CCOL_FILENAME,
	CNCOLS
};

struct tTreeViewColumn {
	const char *caption;		// Column name
	int column;					// Model column index
	GtkTreeCellDataFunc datafn;	// Data function
	gfloat align;				// Text alignment
	const char *maxtext;		// Maximum text (for width estimation)
};

struct tColData {
	GtkTreeViewColumn *col;
	const tTreeViewColumn *data;
};

struct tGetFileInfo
{
	int			id;
	GtkTreePath *path;
	char		*file;
};

struct tFindFirstInfo
{
	int			col;
	bool		valid;
	int			id;
	int			iVal;
	double		dVal;
};

//-------------------------   HELPER FUNCTIONS   --------------------------------

// Make list of row references
static gboolean make_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **list)
{
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
	return FALSE;
}

static gboolean find_frame(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	int fileid;
	tGetFileInfo *pData = (tGetFileInfo*)data;

	gtk_tree_model_get(model, iter, FCOL_ID, &fileid, -1);
	if (fileid == pData->id) {
		pData->path = gtk_tree_path_copy(path);
		return TRUE;
	}
	return FALSE;
}

static gboolean find_first(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	int fileid, stars;
	double juldat;
	tFindFirstInfo *pData = (tFindFirstInfo*)data;

	gtk_tree_model_get(model, iter, FCOL_ID, &fileid, FCOL_JULDAT, &juldat, FCOL_STARS, &stars, -1);
	switch (pData->col) 
	{
	case FCOL_ID:
		if (!pData->valid || (pData->iVal > fileid)) {
			pData->id = fileid;
			pData->iVal = fileid;
			pData->valid = 1;
		}
		break;
	case FCOL_STARS:
		if (!pData->valid || (pData->iVal < stars)) {
			pData->id = fileid;
			pData->iVal = stars;
			pData->valid = 1;
		}
		break;
	case FCOL_JULDAT:
		if (!pData->valid || (pData->dVal > juldat)) {
			pData->id = fileid;
			pData->dVal = juldat;
			pData->valid = 1;
		}
		break;
	}
	return FALSE;
}

static gboolean find_file(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gchar *fname = NULL;
	tGetFileInfo *pData = (tGetFileInfo*)data;

	gtk_tree_model_get(model, iter, CCOL_FILENAME, &fname, -1);
	if (strcmp(fname, pData->file)==0) {
		pData->path = gtk_tree_path_copy(path);
		return TRUE;
	}
	g_free(fname);
	return FALSE;
}

static int text_width(GtkWidget *widget, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		g_object_unref (layout);
		return logical_rect.width;
	}
	return 0;
}

//-------------------------   LIST COLUMNS   --------------------------------

static void GetFrameID(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int id;
	char buf[64];

	gtk_tree_model_get(tree_model, iter, FCOL_ID, &id, -1);

	g_snprintf(buf, sizeof(buf), "%d", id); 
	g_object_set(cell, "text", buf, NULL);
}

static void GetDateTime(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	double jd;
	CmpackDateTime dt;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, FCOL_JULDAT, &jd, -1);

	if (cmpack_decodejd(jd, &dt)==0) {
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetStars(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int state, nstars;
	char buf[64];

	gtk_tree_model_get(tree_model, iter, FCOL_STATE, &state, FCOL_STARS, &nstars, -1);

	if (state & CFILE_PHOTOMETRY) {
		g_snprintf(buf, sizeof(buf), "%d", nstars);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

const static tTreeViewColumn FrameColumns[] = {
	{ "Frame #",		FCOL_ID,		GetFrameID,		1.0 },
	{ "Date and time (UTC)",	FCOL_JULDAT,	GetDateTime,	0.0, "9999-19-99 99:99:99" },
	{ "Stars",			FCOL_STARS,		GetStars,		1.0 },
	{ NULL }
};

const static tTreeViewColumn CatalogColumns[] = {
	{ "File name",		CCOL_NAME,		NULL,			0.0 },
	{ NULL }
};

//-------------------------   MAIN WINDOW   --------------------------------

CMatchingDlg::CMatchingDlg(GtkWindow *pParent):m_pParent(pParent), m_FileList(NULL), m_FrameCols(NULL), 
	m_CatalogCols(NULL), m_ChartData(NULL), m_ImageData(NULL), m_CatFile(NULL), m_SelectionName(NULL),
	m_Updating(false)
{
	int i, w, width, mon;
	GtkWidget *tbox, *tbar, *frame, *hbox;
	GdkRectangle rc;
	GSList *group;

	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Match stars", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("matchstars");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Dialog size
	GdkScreen *scr = gtk_window_get_screen(pParent);
	mon = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(pParent)->window);
	gdk_screen_get_monitor_geometry(scr, mon, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.7*rc.width), RoundToInt(0.7*rc.height));

	// Initial state
	m_SelectMode = (tSelectMode)g_Project->GetInt("MatchingDlg", "Select", REFERENCE_FRAME);
	m_DMFrame = (tDisplayMode)g_Project->GetInt("MatchingDlg", "Display", DISPLAY_CHART);
	m_DMCatalog = (tDisplayMode)g_Project->GetInt("MatchingDlg", "Display2", DISPLAY_CHART);
	m_FrameSort = g_Project->GetInt("MatchingDlg", "Sort", FCOL_STARS);

	// List of frames
	m_Frames = gtk_list_store_new(FNCOLS, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_INT);

	// List of catalog files
	m_Catalogs = gtk_list_store_new(CNCOLS, G_TYPE_STRING, G_TYPE_STRING);

	// Reference frame or catalog file?
	m_UseFrame = gtk_vbox_new(TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), m_UseFrame, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>As a reference file, use:</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(m_UseFrame), label, FALSE, TRUE, 0);
	m_FrameBtn = gtk_radio_button_new_with_label(NULL, "a frame from the current project");
	gtk_widget_set_tooltip_text(m_FrameBtn, "Use one of the source frames as a reference frame");
	g_signal_connect(G_OBJECT(m_FrameBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(m_UseFrame), m_FrameBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_FrameBtn)); 
	m_CatalogBtn = gtk_radio_button_new_with_label(group, "a catalog file from the disk");
	gtk_widget_set_tooltip_text(m_CatalogBtn, "Use a catalog file as a reference frame");
	g_signal_connect(G_OBJECT(m_CatalogBtn), "toggled", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(m_UseFrame), m_CatalogBtn, TRUE, TRUE, 0);

	// Separator
	gtk_box_pack_start(GTK_BOX(m_UseFrame), gtk_label_new(NULL), FALSE, TRUE, 0);

	m_SelLabel = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_SelLabel), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_SelLabel, FALSE, TRUE, 0);
	tbox = gtk_table_new(2, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 0);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 1, 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 0);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 1, 8);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, TRUE, TRUE, 0);

	// List of reference frames
	m_FrameView = gtk_tree_view_new();
	width = 8;
	for (i=0; FrameColumns[i].caption!=NULL; i++) {
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		// Set column name and alignment
		gtk_tree_view_column_set_title(col, FrameColumns[i].caption);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_FrameView), col);
		// Add text renderer
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		g_object_set(renderer, "xalign", FrameColumns[i].align, NULL);
		if (FrameColumns[i].datafn) 
			gtk_tree_view_column_set_cell_data_func(col, renderer, FrameColumns[i].datafn, NULL, NULL);
		else
			gtk_tree_view_column_add_attribute(col, renderer, "text", FrameColumns[i].column);
		g_signal_connect(G_OBJECT(col), "clicked", G_CALLBACK(frame_column_clicked), this);
		tColData *data = (tColData*)g_malloc(sizeof(tColData));
		data->col = col;
		data->data = &FrameColumns[i];
		m_FrameCols = g_slist_append(m_FrameCols, data);
		if (FrameColumns[i].maxtext)
			w = text_width(m_FrameView, FrameColumns[i].maxtext);
		else
			w = text_width(m_FrameView, FrameColumns[i].caption);
		width += w + 24;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_FrameView), GTK_TREE_MODEL(m_Frames));
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(m_FrameView), true);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_FrameView)), GTK_SELECTION_SINGLE);
	m_FrameBox = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_FrameBox),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_FrameBox), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_FrameBox), m_FrameView);
	gtk_widget_set_size_request(m_FrameView, width, -1);
	gtk_table_attach(GTK_TABLE(tbox), m_FrameBox, 0, 1, 0, 2, GTK_FILL, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0);

	// Register callback for selection change
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_FrameView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Catalog files
	m_CatalogView = gtk_tree_view_new();
	for (i=0; CatalogColumns[i].caption!=NULL; i++) {
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		// Set column name and alignment
		gtk_tree_view_column_set_title(col, CatalogColumns[i].caption);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_CatalogView), col);
		// Add text renderer
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		g_object_set(renderer, "xalign", CatalogColumns[i].align, NULL);
		if (CatalogColumns[i].datafn) 
			gtk_tree_view_column_set_cell_data_func(col, renderer, CatalogColumns[i].datafn, NULL, NULL);
		else
			gtk_tree_view_column_add_attribute(col, renderer, "text", CatalogColumns[i].column);
		tColData *data = (tColData*)g_malloc(sizeof(tColData));
		data->col = col;
		data->data = &CatalogColumns[i];
		m_CatalogCols = g_slist_append(m_CatalogCols, data);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_CatalogView), GTK_TREE_MODEL(m_Catalogs));
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(m_CatalogView), true);
	m_CatalogBox = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_CatalogBox),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_CatalogBox), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_CatalogBox), m_CatalogView);
	gtk_widget_set_size_request(m_CatalogView, width, -1);
	gtk_table_attach(GTK_TABLE(tbox), m_CatalogBox, 1, 2, 0, 2, GTK_FILL, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0);

	// Register callback for selection change
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_CatalogView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Toolbox
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_TEXT);
	m_ShowChart = toolbar_new_radio_button(tbar, NULL, "Chart", "Display objects on a flat background");
	g_signal_connect(G_OBJECT(m_ShowChart), "toggled", G_CALLBACK(button_clicked), this);
	m_ShowImage = toolbar_new_radio_button(tbar, m_ShowChart, "Image", "Display an image only");
	g_signal_connect(G_OBJECT(m_ShowImage), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), tbar, 2, 3, 0, 1, 
		(GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);

	// Frame preview
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	m_Preview = cmpack_chart_view_new();
	gtk_container_add(GTK_CONTAINER(frame), m_Preview);
	gtk_widget_set_size_request(frame, 160, 120);
	gtk_table_attach_defaults(GTK_TABLE(tbox), frame, 2, 3, 1, 2);

	// Bottom toolbox
	hbox = gtk_hbox_new(FALSE, 8);
	m_OptionsBtn = gtk_button_new_with_label("Options");
	gtk_widget_set_tooltip_text(m_OptionsBtn, "Edit project settings");
	gtk_box_pack_start(GTK_BOX(hbox), m_OptionsBtn, 0, 0, 0);
	g_signal_connect(G_OBJECT(m_OptionsBtn), "clicked", G_CALLBACK(button_clicked), this);
	m_PathLabel = gtk_label_new("Search path:");
	gtk_box_pack_start(GTK_BOX(hbox), m_PathLabel, FALSE, TRUE, 0);
	m_PathEntry = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_PathEntry, "Path to the folder with catalog files");
	gtk_editable_set_editable(GTK_EDITABLE(m_PathEntry), false);
	gtk_box_pack_start(GTK_BOX(hbox), m_PathEntry, TRUE, TRUE, 0);
	m_PathBtn = gtk_button_new_with_label("Change folder");
	gtk_widget_set_tooltip_text(m_PathBtn, "Change folder with catalog files");
	g_signal_connect(G_OBJECT(m_PathBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_box_pack_start(GTK_BOX(hbox), m_PathBtn, FALSE, TRUE, 0);
	gtk_table_attach(GTK_TABLE(tbox), hbox, 0, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMatchingDlg::~CMatchingDlg()
{
	for (GSList *ptr=m_FrameCols; ptr!=NULL; ptr=ptr->next)
		g_free(ptr->data);
	g_slist_free(m_FrameCols);

	for (GSList *ptr=m_CatalogCols; ptr!=NULL; ptr=ptr->next)
		g_free(ptr->data);
	g_slist_free(m_CatalogCols);

	gtk_widget_destroy(m_pDlg);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_ImageData)
		g_object_unref(m_ImageData);
	g_object_unref(m_Frames);
	g_object_unref(m_Catalogs);
	g_list_free(m_FileList);
	g_free(m_CatFile);
	g_free(m_SelectionName);
}

void CMatchingDlg::Execute(void)
{
	int res;
	char msg[256];

	m_FrameID = 0;
	g_free(m_CatFile);
	m_CatFile = NULL;
	g_free(m_SelectionName);
	m_SelectionName = NULL;
	m_FileList = NULL;

	// Restore path to catalog files
	gchar *fpath = CConfig::GetStr("MakeCatDlg", "Folder", NULL);
	if (fpath && *fpath!='\0' && g_file_test(fpath, G_FILE_TEST_IS_DIR)) 
		gtk_entry_set_text(GTK_ENTRY(m_PathEntry), fpath);
	else {
		gchar *defpath = g_build_filename(get_user_data_dir(), "Catalog files", NULL);
		if (force_directory(defpath))
			gtk_entry_set_text(GTK_ENTRY(m_PathEntry), defpath);
		g_free(defpath);
	}
	g_free(fpath);

	// Update list of frames
	ReadFrames(false, g_Project->GetInt("MatchingDlg", "Frame", 0));

	// Update list of catalog files
	gchar *path = g_Project->GetStr("MatchingDlg", "File", NULL);
	if (!path)
		path = CConfig::GetStr("MatchingDlg", "File", NULL);
	ReadCatalogs(path);
	g_free(path);

	gtk_widget_show(m_UseFrame);

	// Check inputs
	if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Frames), NULL)==0) {
		ShowError(m_pParent, "There are no frames usable as a reference frame.");
		return;
	}

	// Select reference frame or the first frame
	SetSelectMode(m_SelectMode);
	SetSortMode(m_FrameSort);
	UpdatePreview(true);
	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	g_Project->SetInt("MatchingDlg", "Select", m_SelectMode);
	if (m_SelectMode == REFERENCE_FRAME)
		g_Project->SetInt("MatchingDlg", "Frame", m_FrameID);
	else {
		g_Project->SetStr("MatchingDlg", "File", m_CatFile);
		CConfig::SetStr("MatchingDlg", "File", m_CatFile);
	}
	g_Project->ClearReference();
	
	// Always all files
	gtk_tree_model_foreach(g_Project->FileList(), GtkTreeModelForeachFunc(make_list), &m_FileList);

	CProgressDlg pDlg(m_pParent, "Matching photometry files");
	pDlg.SetMinMax(0, g_list_length(m_FileList));
	res = pDlg.Execute(ExecuteProc, this);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		ShowError(m_pParent, msg, true);
		cmpack_free(msg);
	} else if (m_OutFiles==0) {
		ShowError(m_pParent, "No file was successfully processed.", true);
	} else if (m_OutFiles!=m_InFiles) {
		sprintf(msg, "%d file(s) were successfully processed, %d file(s) failed.", 
			m_OutFiles, m_InFiles-m_OutFiles);
		ShowWarning(m_pParent, msg, true);
	} else {
		sprintf(msg, "All %d file(s) were successfully processed.", m_OutFiles);
		ShowInformation(m_pParent, msg, true);
	}

	// Free allocated memory
	g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
	g_list_free(m_FileList);
	m_FileList = NULL;
	g_free(m_CatFile);
	m_CatFile = NULL;
	g_free(m_SelectionName);
	m_SelectionName = NULL;
}

bool CMatchingDlg::SelectFile(char **fpath)
{
	g_free(m_CatFile);
	m_CatFile = NULL;
	g_free(m_SelectionName);
	m_SelectionName = NULL;

	// Set path to catalog files
	gchar *dirpath = g_path_get_dirname(*fpath);
	if (dirpath && *dirpath!='\0' && strcmp(dirpath, ".")!=0 && g_file_test(dirpath, G_FILE_TEST_IS_DIR)) {
		gtk_entry_set_text(GTK_ENTRY(m_PathEntry), dirpath);
	} else {
		gchar *extpath = CConfig::GetStr("MakeCatDlg", "Folder", NULL);
		if (extpath && *extpath!='\0' && g_file_test(extpath, G_FILE_TEST_IS_DIR)) 
			gtk_entry_set_text(GTK_ENTRY(m_PathEntry), extpath);
		else {
			gchar *defpath = g_build_filename(get_user_data_dir(), "Catalog files", NULL);
			if (force_directory(defpath))
				gtk_entry_set_text(GTK_ENTRY(m_PathEntry), defpath);
			g_free(defpath);
		}
		g_free(extpath);
	}
	g_free(dirpath);

	gtk_widget_hide(m_UseFrame);
	SetSelectMode(CATALOG_FILE);
	gchar *basename = g_path_get_basename(*fpath);
	ReadCatalogs(basename);
	g_free(basename);
	UpdatePreview(true);
	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		g_Project->SetStr("MatchingDlg", "File", m_CatFile);
		CConfig::SetStr("MatchingDlg", "File", m_CatFile);
		g_free(*fpath);
		*fpath = g_strdup(m_CatFile);
		g_free(m_CatFile);
		m_CatFile = NULL;
		g_free(m_SelectionName);
		m_SelectionName = NULL;
		return true;
	}
	return false;
}

bool CMatchingDlg::SelectFrame(int *frame_id)
{
	m_FrameID = 0;

	gtk_widget_hide(m_UseFrame);
	ReadFrames(true, *frame_id);
	SetSelectMode(REFERENCE_FRAME);
	SetSortMode(m_FrameSort);
	UpdatePreview(true);
	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		g_Project->SetInt("MatchingDlg", "Frame", m_FrameID);
		*frame_id = m_FrameID;
		return true;
	}
	return false;
}


void CMatchingDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMatchingDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMatchingDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		if (m_SelectMode == REFERENCE_FRAME) {
			// Reference frame
			if (m_FrameID<0) {
				ShowError(GTK_WINDOW(m_pDlg), "Please, select a reference frame.");
				return false;
			}
		} else {
			// Catalog file
			if (!m_CatFile) {
				ShowError(GTK_WINDOW(m_pDlg), "Please, select a catalog file.");
				return false;
			}
		}
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MATCH_STARS);
		return false;
	}
	return true;
}

int CMatchingDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CMatchingDlg*)userdata)->ProcessFiles(sender);
}

int CMatchingDlg::ProcessFiles(CProgressDlg *sender)
{
	int frameid, res;
	gchar *phot, msg[128];
	GtkTreePath *path;
	CMatchingProc match;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Matching ------");

	if (m_SelectMode == REFERENCE_FRAME) {
		GtkTreePath *refpath = g_Project->GetFilePath(m_FrameID);
		res = match.InitWithReferenceFrame(sender, refpath);
		gtk_tree_path_free(refpath);
	} else
		res = match.InitWithCatalogFile(sender, m_CatFile);
	if (res==0) {
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				frameid = g_Project->GetFrameID(path);
				phot = g_Project->GetPhotFileName(path);
				sender->SetFileName(phot);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				g_free(phot);
				sender->SetProgress(m_InFiles++);
				if (match.Execute(path)==0)
					m_OutFiles++;
				gtk_tree_path_free(path);
			}
			if (sender->Cancelled()) 
				break;
		}
		if (sender->Cancelled())
			sprintf(msg, "Cancelled at the user's request");
		else
			sprintf(msg, "====== %d succeeded, %d failed ======", m_OutFiles, m_InFiles-m_OutFiles);
		sender->Print(msg);
	} else {
		char *aux = cmpack_formaterror(res);
		sender->Print(aux);
		cmpack_free(aux);
	}
	return res;
}

void CMatchingDlg::selection_changed(GtkTreeSelection *widget, CMatchingDlg *pMe)
{
	pMe->UpdatePreview();
	pMe->UpdateControls();
}

void CMatchingDlg::UpdatePreview(bool force_update)
{
	int frame_id, state;
	CImage img;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (m_SelectMode == REFERENCE_FRAME) {
		// Reference frame
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_FrameView));
		if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
			gtk_tree_model_get(model, &iter, FCOL_ID, &frame_id, FCOL_STATE, &state, -1);
			if (force_update || m_FrameID!=frame_id) {
				cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
				cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), NULL);
				if (m_ChartData) {
					g_object_unref(m_ChartData);
					m_ChartData = NULL;
				}
				if (m_ImageData) {
					g_object_unref(m_ImageData);
					m_ImageData = NULL;
				}
				m_FrameID = frame_id;
				if (m_DMFrame == DISPLAY_CHART && (state & CFILE_PHOTOMETRY)) {
					gchar *pht_file = g_Project->GetPhotFile(frame_id);
					CPhot pht;
					if (pht_file && pht.Load(pht_file))
						UpdateChart(pht);
					g_free(pht_file);
				} else 
				if (state & CFILE_CONVERSION) {
					gchar *fts_file = g_Project->GetImageFile(frame_id);
					if (fts_file && img.Load(fts_file)) 
						UpdateImage(img);
					g_free(fts_file);
				} else {
					gchar *src_file = g_Project->GetSourceFile(frame_id);
					if (src_file && img.Load(src_file)) 
						UpdateImage(img);
					g_free(src_file);
				}
			}
		} else {
			if (force_update || m_FrameID>=0) {
				m_FrameID = -1;
				cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
				cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), NULL);
				if (m_ChartData) {
					g_object_unref(m_ChartData);
					m_ChartData = NULL;
				}
				if (m_ImageData) {
					g_object_unref(m_ImageData);
					m_ImageData = NULL;
				}
			}
		}
	} else {
		// Catalog file
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_CatalogView));
		if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
			gchar *fname;
			gtk_tree_model_get(GTK_TREE_MODEL(m_Catalogs), &iter, CCOL_FILENAME, &fname, -1);
			gchar *fpath = g_build_filename(gtk_entry_get_text(GTK_ENTRY(m_PathEntry)), fname, NULL);
			if (force_update || StrCmp0(m_CatFile, fpath)!=0) {
				g_free(m_CatFile);
				m_CatFile = g_strdup(fpath);
				cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
				cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), NULL);
				if (m_ChartData) {
					g_object_unref(m_ChartData);
					m_ChartData = NULL;
				}
				if (m_ImageData) {
					g_object_unref(m_ImageData);
					m_ImageData = NULL;
				}
				CCatalog cat;
				if (cat.Load(m_CatFile)) 
					UpdateChart(cat);
				if (m_DMCatalog == DISPLAY_IMAGE) {
					gchar *fts_file = SetFileExtension(m_CatFile, FILE_EXTENSION_FITS);
					if (img.Load(fts_file)) 
						UpdateImage(img);
					g_free(fts_file);
				}
			}
			g_free(fpath);
			g_free(fname);
		} else {
			if (force_update || m_CatFile) {
				g_free(m_CatFile);
				m_CatFile = NULL;
				g_free(m_SelectionName);
				m_SelectionName = NULL;
				cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
				cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), NULL);
				if (m_ChartData) {
					g_object_unref(m_ChartData);
					m_ChartData = NULL;
				}
				if (m_ImageData) {
					g_object_unref(m_ImageData);
					m_ImageData = NULL;
				}
			}
		}
	}
}

void CMatchingDlg::button_clicked(GtkWidget *button, CMatchingDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CMatchingDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (!m_Updating) {
		m_Updating = true;
		if (pBtn==m_FrameBtn) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FrameBtn))) {
				SetSelectMode(REFERENCE_FRAME);
				UpdatePreview(true);
				UpdateControls();
			}
		} else if (pBtn==m_CatalogBtn) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_CatalogBtn))) {
				SetSelectMode(CATALOG_FILE);
				UpdatePreview(true);
				UpdateControls();
			}
		} else if (pBtn==GTK_WIDGET(m_ShowChart)) {
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart))) {
				SetDisplayMode(DISPLAY_CHART);
				UpdatePreview(true);
				UpdateControls();
			}
		} else if (pBtn==GTK_WIDGET(m_ShowImage)) {
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage))) {
				SetDisplayMode(DISPLAY_IMAGE);
				UpdatePreview(true);
				UpdateControls();
			}
		} else if (pBtn==m_PathBtn) 
			ChangeCatalogPath();
		else if (pBtn==m_OptionsBtn)
			EditPreferences();
		m_Updating = false;
	}
}

void CMatchingDlg::SetSelectMode(tSelectMode mode)
{
	m_SelectMode = mode;
	switch (mode)
	{
	case REFERENCE_FRAME:
		gtk_label_set_markup(GTK_LABEL(m_SelLabel), "<b>Select a reference frame</b>");
		gtk_widget_hide(m_CatalogBox);
		gtk_widget_hide(m_PathLabel);
		gtk_widget_hide(m_PathEntry);
		gtk_widget_hide(m_PathBtn);
		gtk_widget_show(m_FrameBox);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart), 
			m_DMFrame == DISPLAY_CHART);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage), 
			m_DMFrame == DISPLAY_IMAGE);
		break;

	case CATALOG_FILE:
		gtk_label_set_markup(GTK_LABEL(m_SelLabel), "<b>Select a catalog file</b>");
		gtk_widget_hide(m_FrameBox);
		gtk_widget_show(m_PathLabel);
		gtk_widget_show(m_PathEntry);
		gtk_widget_show(m_PathBtn);
		gtk_widget_show(m_CatalogBox);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart), 
			m_DMCatalog == DISPLAY_CHART);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage), 
			m_DMCatalog == DISPLAY_IMAGE);
		break;
	}
}

void CMatchingDlg::SetSortMode(int column)
{
	m_FrameSort = column;
	for (GSList *ptr=m_FrameCols; ptr!=NULL; ptr=ptr->next) {
		tColData *col = (tColData*)ptr->data;
		if (col->data->column == column) {
			gtk_tree_view_column_set_sort_indicator(col->col, true);
			GtkSortType dir = (column!=FCOL_STARS ? GTK_SORT_ASCENDING : GTK_SORT_DESCENDING);
			gtk_tree_view_column_set_sort_order(col->col, dir);
			gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(m_Frames), column, dir);
		} else {
			gtk_tree_view_column_set_sort_indicator(col->col, false);
		}
	}
}

void CMatchingDlg::SetDisplayMode(tDisplayMode mode)
{
	if (m_SelectMode == REFERENCE_FRAME) {
		m_DMFrame = mode;
		g_Project->SetInt("MatchingDlg", "Display", m_DMFrame);
	} else {
		m_DMCatalog = mode;
		g_Project->SetInt("MatchingDlg", "Display2", m_DMCatalog);
	}
}

CMatchingDlg::tDisplayMode CMatchingDlg::GetDisplayMode(void) const
{
	if (m_SelectMode == REFERENCE_FRAME)
		return m_DMFrame;
	else
		return m_DMCatalog;
}

void CMatchingDlg::frame_column_clicked(GtkTreeViewColumn *treeviewcolumn, CMatchingDlg *pDlg)
{
	pDlg->OnFrameColumnClicked(treeviewcolumn);
}

void CMatchingDlg::OnFrameColumnClicked(GtkTreeViewColumn *pCol)
{
	for (GSList *ptr=m_FrameCols; ptr!=NULL; ptr=ptr->next) {
		tColData *col = (tColData*)ptr->data;
		if (col->col == pCol) {
			SetSortMode(col->data->column);
			g_Project->SetInt("MatchingDlg", "Sort", m_FrameSort);
			break;
		}
	}
}

void CMatchingDlg::ReadFrames(bool all_frames, int init_frame)
{
	int id, state, stars;
	double jd;
	gboolean ok;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter, iter2;
	tGetFileInfo info;
	tFindFirstInfo info2;

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_FrameView), NULL);
	gtk_list_store_clear(m_Frames);
	ok = gtk_tree_model_get_iter_first(pList, &iter);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(pList), &iter, FRAME_ID, &id, 
			FRAME_STATE, &state, FRAME_STARS, &stars, FRAME_JULDAT, &jd, -1);
		if (all_frames || ((state & CFILE_PHOTOMETRY) && (stars>0))) {
			gtk_list_store_append(m_Frames, &iter2);
			gtk_list_store_set(m_Frames, &iter2, FCOL_ID, id, FCOL_STATE, state, 
				FCOL_STARS, stars, FCOL_JULDAT, jd, -1);
		}
		ok = gtk_tree_model_iter_next(pList, &iter);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_FrameView), GTK_TREE_MODEL(m_Frames));

	info.path = NULL;
	if (init_frame>0) {
		info.id = init_frame;
		gtk_tree_model_foreach(GTK_TREE_MODEL(m_Frames), GtkTreeModelForeachFunc(find_frame), &info);
	}
	if (!info.path) {
		// First first frame
		info2.col = m_FrameSort;
		info2.valid = 0;
		info2.id = -1;
		gtk_tree_model_foreach(GTK_TREE_MODEL(m_Frames), GtkTreeModelForeachFunc(find_first), &info2);
		if (info2.id>0) {
			info.id = info2.id;
			gtk_tree_model_foreach(GTK_TREE_MODEL(m_Frames), GtkTreeModelForeachFunc(find_frame), &info);
		}
	}
	if (info.path) {
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_FrameView));
		gtk_tree_selection_select_path(sel, info.path);
		gtk_tree_path_free(info.path);
	}
}

void CMatchingDlg::ReadCatalogs(const gchar *init_file)
{
	const char *dirpath = gtk_entry_get_text(GTK_ENTRY(m_PathEntry));
	GtkTreeIter iter2;
	tGetFileInfo info;

	gtk_tree_view_set_model(GTK_TREE_VIEW(m_CatalogView), NULL);
	gtk_list_store_clear(m_Catalogs);
	GDir *dir = g_dir_open(dirpath, 0, NULL);
	if (dir) {
		const gchar *filename = g_dir_read_name(dir);
		while (filename) {
			if (CheckFileExtension(filename, FILE_EXTENSION_CATALOG)) {
				gchar *filepath = g_build_filename(dirpath, filename, NULL);
				if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR)) {
					gchar *name = StripFileExtension(filename);
					gtk_list_store_append(m_Catalogs, &iter2);
					gtk_list_store_set(m_Catalogs, &iter2, CCOL_NAME, name, 
						CCOL_FILENAME, filename, -1);
					g_free(name);
				}
				g_free(filepath);
			}
			filename = g_dir_read_name(dir);
		}
		g_dir_close(dir);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_CatalogView), GTK_TREE_MODEL(m_Catalogs));

	info.path = NULL;	
	if (init_file) {
		gchar *basename = g_path_get_basename(init_file);
		info.file = (char*)basename;
		gtk_tree_model_foreach(GTK_TREE_MODEL(m_Catalogs), GtkTreeModelForeachFunc(find_file), &info);
		g_free(basename);
	}
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_CatalogView));
	if (info.path) {
		gtk_tree_selection_select_path(sel, info.path);
		gtk_tree_path_free(info.path);
	} else {
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Catalogs), &iter2))
			gtk_tree_selection_select_iter(sel, &iter2);
	}
}

void CMatchingDlg::ChangeCatalogPath(void)
{
	const gchar *dirname = gtk_entry_get_text(GTK_ENTRY(m_PathEntry));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select path",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		CConfig::SetStr("MakeCatDlg", "Folder", path);
		gtk_entry_set_text(GTK_ENTRY(m_PathEntry), path);
		g_free(path);
		ReadCatalogs(NULL);
		UpdatePreview(true);
		UpdateControls();
	}
	gtk_widget_destroy(pPathDlg);
}

void CMatchingDlg::UpdateControls(void)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FrameBtn), 
		(m_SelectMode == REFERENCE_FRAME));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_CatalogBtn), 
		(m_SelectMode == CATALOG_FILE));
	if (m_SelectMode == REFERENCE_FRAME) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage),
			(m_DMFrame == DISPLAY_IMAGE));
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart),
			(m_DMFrame == DISPLAY_CHART));
	} else {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage),
			(m_DMCatalog == DISPLAY_IMAGE));
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart),
			(m_DMCatalog == DISPLAY_CHART));
	}
}

void CMatchingDlg::UpdateImage(CImage &img)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), NULL);
	if (m_ImageData) 
		g_object_unref(m_ImageData);
	m_ImageData = img.ToImageData(m_Negative, false, true, m_RowsUpward);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Preview), m_ImageData);
	cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Preview), TRUE);
}

void CMatchingDlg::UpdateChart(CPhot &pht)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	pht.SelectAperture(0);
	m_ChartData = pht.ToChartData(false, GetDisplayMode() == DISPLAY_IMAGE);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Preview), m_Negative);
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Preview), (m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS));
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), m_ChartData);
	cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Preview), TRUE);
}

void CMatchingDlg::UpdateChart(CCatalog &cat)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), NULL);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	cat.SelectSelection(0);
	m_ChartData = cat.ToChartData(true, true, GetDisplayMode() == DISPLAY_IMAGE);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Preview), m_Negative);
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Preview), (m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS));
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Preview), m_ChartData);
	cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Preview), TRUE);
}

void CMatchingDlg::EditPreferences(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute(PAGE_MATCHING);
}
