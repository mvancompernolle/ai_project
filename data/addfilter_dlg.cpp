/**************************************************************

checkfiles_dlg.cpp (C-Munipack project)
Filter for files that are being appended to the project
Copyright (C) 2010 David Motl, dmotl@volny.cz

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

#include "addfilter_dlg.h"
#include "utils.h"
#include "main.h"

struct tPageData
{
	GtkWidget *box, *scrwnd, *title, *check, *view, *total;
	GtkTreeModel *model, *fmodel;
	bool allfiles, skip;
};

enum tTreeColumns
{
	DATA_CHECK,
	DATA_VALUE,
	DATA_FILES,
	DATA_COLUMNS
};

static void UpdateTotal(tPageData *data);

static void PrintFiles(char *buf, int files)
{
	if (files==0)
		strcpy(buf, "No file");
	else if (files==1)
		strcpy(buf, "1 file");
	else 
		sprintf(buf, "%d files", files); 
}

static void RenderFilter(GtkTreeViewColumn *col, GtkCellRenderer *cell,
	GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	int files;
	char buf[256], aux[64], *filter;

	gtk_tree_model_get(model, iter, DATA_VALUE, &filter, DATA_FILES, &files, -1);
	PrintFiles(aux, files);
	if (filter)
		g_snprintf(buf, sizeof(buf), "%s (%s)", filter, aux); 
	else
		g_snprintf(buf, sizeof(buf), "%s (%s)", "<empty string>", aux); 
	g_object_set(cell, "text", buf, NULL);
	g_free(filter);
}

static void RenderExposure(GtkTreeViewColumn *col, GtkCellRenderer *cell,
	GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	int files;
	double exptime;
	char aux[64], buf[256];

	gtk_tree_model_get(model, iter, DATA_VALUE, &exptime, DATA_FILES, &files, -1);
	PrintFiles(aux, files);
	g_snprintf(buf, sizeof(buf), "%.3f seconds (%s)", exptime, aux);
	g_object_set(cell, "text", buf, NULL);
}

static void RenderObject(GtkTreeViewColumn *col, GtkCellRenderer *cell,
	GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	int files;
	char buf[256], aux[64], *object;

	gtk_tree_model_get(model, iter, DATA_VALUE, &object, DATA_FILES, &files, -1);
	PrintFiles(aux, files);
	if (object)
		g_snprintf(buf, sizeof(buf), "%s (%s)", object, aux); 
	else
		g_snprintf(buf, sizeof(buf), "%s (%s)", "<empty string>", aux); 
	g_object_set(cell, "text", buf, NULL);
	g_free(object);
}

static void OnCellToggled(GtkCellRendererToggle *cell, char *path, tPageData *pData)
{
	GtkTreeIter fiter, iter;

	if (gtk_tree_model_get_iter_from_string(pData->fmodel, &fiter, path)) {
		gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(pData->fmodel),
			&iter, &fiter);
		gtk_list_store_set(GTK_LIST_STORE(pData->model), &iter, DATA_CHECK,
			!gtk_cell_renderer_toggle_get_active(cell), -1);
		UpdateTotal(pData);
	}
}

static void OnCheckToggled(GtkToggleButton *button, tPageData *pData)
{
	pData->allfiles = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pData->check))!=0;
	gtk_widget_set_sensitive(pData->view, !pData->allfiles);
	UpdateTotal(pData);
}

static gboolean FilterRows(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	int files;

	gtk_tree_model_get(model, iter, DATA_FILES, &files, -1);
	return files>0;
}

static tPageData *MakePage(const char *feature, GtkListStore *model,
						   GtkTreeCellDataFunc datafunc)
{
	char buf[512];
	tPageData *data = (tPageData*)g_malloc0(sizeof(tPageData));
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;

	data->box = gtk_vbox_new(FALSE, 8);

	// Title
	data->title = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(data->title), 0.0, 0.5);
	gtk_widget_set_size_request(data->title, 400, -1);
	sprintf(buf, "<b>The selection consists of CCD frames of different %s. "
		"Which files do you want to add to the project?</b>", feature);
	gtk_label_set_markup(GTK_LABEL(data->title), buf);
	gtk_label_set_line_wrap(GTK_LABEL(data->title), TRUE);
	gtk_box_pack_start(GTK_BOX(data->box), data->title, FALSE, TRUE, 0);

	// Check box (all files)
	data->check = gtk_check_button_new_with_label("All files");
	gtk_widget_set_tooltip_text(data->check, "Append frames from all categories listed below");
	g_signal_connect(G_OBJECT(data->check), "toggled", G_CALLBACK(OnCheckToggled), data);
	gtk_box_pack_start(GTK_BOX(data->box), data->check, FALSE, TRUE, 0);

	// Scrolled window
	data->scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(data->scrwnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(data->scrwnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(data->box), data->scrwnd, TRUE, TRUE, 0);

	// Tree model
	data->model = GTK_TREE_MODEL(model);
	g_object_ref(data->model);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(data->model), DATA_VALUE, GTK_SORT_ASCENDING);
	data->fmodel = gtk_tree_model_filter_new(data->model, NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(data->fmodel), FilterRows, NULL, NULL);
	g_object_ref(data->fmodel);

	// Tree view 
	data->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(data->fmodel));
	gtk_container_add(GTK_CONTAINER(data->scrwnd), data->view);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(data->view), FALSE);
	gtk_widget_set_tooltip_text(data->view, "Append frames from selected categories");
	// Value column (check box + text)
	col = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(data->view), col);
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(OnCellToggled), data);
	g_object_set(renderer, "activatable", TRUE, NULL);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "active", DATA_CHECK);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(col, renderer, datafunc, NULL, NULL);
	
	// Number of selected files
	data->total = gtk_label_new(NULL);
	gtk_widget_set_size_request(data->title, 400, -1);
	gtk_misc_set_alignment(GTK_MISC(data->total), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(data->box), data->total, FALSE, TRUE, 0);	

	return data;
}

static void UpdateTotal(tPageData *data)
{
	gboolean ok, checked;
	char buf[256];
	int count, files;
	GtkTreeIter iter;

	// For each filter, compute number of files
	files = 0;
	ok = gtk_tree_model_get_iter_first(data->fmodel, &iter);
	while (ok) {
		gtk_tree_model_get(data->fmodel, &iter, DATA_FILES, &count, DATA_CHECK, &checked, -1);
		if (data->allfiles || checked)
			files += count;
		ok = gtk_tree_model_iter_next(data->fmodel, &iter);
	}
	// Update label
	if (files==0)
		gtk_label_set_text(GTK_LABEL(data->total), "No file selected.");
	else if (files==1)
		gtk_label_set_text(GTK_LABEL(data->total), "1 file selected.");
	else  {
		sprintf(buf, "%d files selected.", files);
		gtk_label_set_text(GTK_LABEL(data->total), buf);
	}
}

static GtkWidget *MakeSummary(void)
{
	return gtk_label_new(NULL);
}


static void PageFree(tPageData *data, gpointer *user_data)
{
	if (data->model)
		g_object_unref(data->model);
	if (data->fmodel)
		g_object_unref(data->fmodel);
	g_free(data);
}

static void FileFree(CFrameInfo *data, gpointer *user_data)
{
	delete data;
}

static gint CompareByTime(const CFrameInfo *a, const CFrameInfo *b)
{
	// Compare by time
	if (a->JulDat() < b->JulDat())
		return -1;
	else if (a->JulDat() > b->JulDat())
		return 1;
	
	// If they are equal, compare by path
	if (!a->FullPath()) {
		if (b->FullPath())
			return 1;
		else
			return 0;
	} else {
		if (!b->FullPath())
			return -1;
		else
			return strcmp(a->FullPath(), b->FullPath());
	}
}

//
// Constructor
//
CAddFilterDlg::CAddFilterDlg(GtkWindow *pParent, int context_id):m_Pages(NULL), m_Frames(NULL), 
	m_ContextId(context_id)
{
	m_pDlg = gtk_dialog_new_with_buttons("Add files", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_GO_BACK, GTK_RESPONSE_NO, GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	m_pClose = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_CLOSE);
	m_pCancel = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_CANCEL);
	m_pBack = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_NO);
	gtk_widget_set_tooltip_text(m_pBack, "Go back to the previous page");
	m_pNext = get_dialog_widget_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_YES);
	gtk_widget_set_tooltip_text(m_pNext, "Continue to the next page");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("addfiles");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Page 1: Color filter
	m_FilList = gtk_list_store_new(DATA_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INT);
	m_Pages = g_slist_append(m_Pages, MakePage("color filter", m_FilList, RenderFilter));

	// Page 2: Exposure duration
	m_ExpList = gtk_list_store_new(DATA_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_DOUBLE, G_TYPE_INT);
	m_Pages = g_slist_append(m_Pages, MakePage("exposure duration", m_ExpList, RenderExposure));

	// Page 3: Object name
	m_ObjList = gtk_list_store_new(DATA_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INT);
	m_Pages = g_slist_append(m_Pages, MakePage("object name", m_ObjList, RenderObject));

	// Page 4: summary
	m_pLast = MakeSummary();

	// Notebook
	m_pTabs = gtk_notebook_new();
	gtk_widget_set_size_request(m_pTabs, 400, 320);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), m_pTabs);
	gtk_container_set_border_width(GTK_CONTAINER(m_pTabs), 8);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(m_pTabs), FALSE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_pTabs), FALSE);
	for (GSList *ptr=m_Pages; ptr!=NULL; ptr=ptr->next) 
		gtk_notebook_append_page(GTK_NOTEBOOK(m_pTabs), ((tPageData*)ptr->data)->box, NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(m_pTabs), m_pLast, NULL);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CAddFilterDlg::~CAddFilterDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_slist_foreach(m_Pages, (GFunc)PageFree, NULL);
	g_slist_free(m_Pages);
	g_object_unref(m_FilList);
	g_object_unref(m_ExpList);
	g_object_unref(m_ObjList);
	m_Pages = NULL;
}


//
// Add files from the list
//
void CAddFilterDlg::Execute(const GSList *files)
{
	GtkWindow *pParent = gtk_window_get_transient_for(GTK_WINDOW(m_pDlg));

	m_SrcFiles = files;

	CProgressDlg dlg(pParent, "Scanning directories");
	if (dlg.Execute(PrepareFilesProc, this)) {
		UpdateModels();
		GoToFirstPage();
		gtk_dialog_run(GTK_DIALOG(m_pDlg));
	}
	FreeFiles();
}

//
// Add files from the folder
//
void CAddFilterDlg::Execute(const char *dirpath, bool recursive)
{
	GtkWindow *pParent = gtk_window_get_transient_for(GTK_WINDOW(m_pDlg));

	m_DirPath = dirpath;
	m_Recursive = recursive;

	CProgressDlg dlg(pParent, "Scanning directories");
	if (dlg.Execute(PrepareFolderProc, this)) {
		UpdateModels();
		GoToFirstPage();
		gtk_dialog_run(GTK_DIALOG(m_pDlg));
	}
	FreeFiles();
}

//
// Make list of files
//
int CAddFilterDlg::PrepareFilesProc(CProgressDlg *sender, void *user_data)
{
	CAddFilterDlg *pMe = (CAddFilterDlg*)user_data;
	CFrameInfo info;

	GList *list = NULL;
	for (const GSList *ptr=pMe->m_SrcFiles; ptr!=NULL; ptr=ptr->next) {
		const gchar *filepath = (const gchar*)ptr->data;
		gchar *basename = g_path_get_basename(filepath);
		sender->Print(basename);
		g_free(basename);
		if (!g_Project->FileInProject(filepath)) {
			if (info.Init(filepath))
				list = g_list_prepend(list, new CFrameInfo(info));
		}
		if (sender->Cancelled()) {
			g_list_foreach(list, (GFunc)FileFree, NULL);
			g_list_free(list);		
			return FALSE;
		}
	}
	pMe->m_Frames = g_list_sort(list, (GCompareFunc)CompareByTime);
	return TRUE;
}

int CAddFilterDlg::PrepareFolderProc(CProgressDlg *sender, void *user_data)
{
	int result = TRUE;
	CAddFilterDlg *pMe = (CAddFilterDlg*)user_data;
	
	GList *list = ScanDir(sender, NULL, pMe->m_DirPath, ".", pMe->m_Recursive, &result);
	if (!result) {
		g_list_foreach(list, (GFunc)FileFree, NULL);
		g_list_free(list);		
		return FALSE;
	}
	pMe->m_Frames = g_list_sort(list, (GCompareFunc)CompareByTime);
	return TRUE;
}
	
GList *CAddFilterDlg::ScanDir(CProgressDlg *sender, GList *list, const char *dirpath, 
	const gchar *relpath, bool recursive, int *presult)
{
	const gchar *filename;
	gchar *filepath, *rpath;
	CFrameInfo info;

	sender->Print(relpath);

	GDir *dir = g_dir_open(dirpath, 0, NULL);
	if (dir) {
		// First process files in current directory
		filename = g_dir_read_name(dir);
		while (filename) {
			filepath = g_build_filename(dirpath, filename, NULL);
			if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR)) {
				if (!g_Project->FileInProject(filepath)) {
					if (info.Init(filepath))
						list = g_list_prepend(list, new CFrameInfo(info));
				}
			}
			g_free(filepath);
			if (sender->Cancelled()) {
				*presult = FALSE;
				break;
			}
			filename = g_dir_read_name(dir);
		}
		if (*presult && recursive) {
			// Go to subdirectories 
			g_dir_rewind(dir);
			filename = g_dir_read_name(dir);
			while (*presult && filename) {
				filepath = g_build_filename(dirpath, filename, NULL);
				rpath = g_build_filename(relpath, filename, NULL);
				if (g_file_test(filepath, G_FILE_TEST_IS_DIR)) 
					list = ScanDir(sender, list, filepath, rpath, true, presult);
				g_free(filepath);
				g_free(rpath);
				if (!*presult)
					break;
				filename = g_dir_read_name(dir);
			}
		}
		g_dir_close(dir);
	}
	return list;
}


//
// Free allocated memory
//
void CAddFilterDlg::FreeFiles(void)
{
	g_list_foreach(m_Frames, (GFunc)FileFree, NULL);
	g_list_free(m_Frames);
	m_Frames = NULL;
}

void CAddFilterDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAddFilterDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAddFilterDlg::OnResponseDialog(int response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_YES:
		// Go to next page
		GoForward();
		return false;

	case GTK_RESPONSE_NO:
		// Go to previous page
		GoBackward();
		return false;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), m_ContextId);
		return false;
	}
	return true;
}

void CAddFilterDlg::UpdateModels(void)
{
	bool ok;
	GtkTreeIter iter;

	// Filters
	gtk_list_store_clear(GTK_LIST_STORE(m_FilList));
	for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
		ok = true;
		for (GList *aux=ptr->prev; aux!=NULL; aux=aux->prev) {
			if (StrCmp0(((CFrameInfo*)ptr->data)->Filter(), ((CFrameInfo*)aux->data)->Filter())==0) {
				ok = false;
				break;
			}
		}
		if (ok) {
			gtk_list_store_append(GTK_LIST_STORE(m_FilList), &iter);
			gtk_list_store_set(GTK_LIST_STORE(m_FilList), &iter, DATA_CHECK, false, 
				DATA_VALUE, ((CFrameInfo*)ptr->data)->Filter(), DATA_FILES, 0, -1);
		}
	}
	// Exposures
	gtk_list_store_clear(GTK_LIST_STORE(m_ExpList));
	for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
		ok = true;
		for (GList *aux=ptr->prev; aux!=NULL; aux=aux->prev) {
			if (((CFrameInfo*)ptr->data)->ExpTime() == ((CFrameInfo*)aux->data)->ExpTime()) {
				ok = false;
				break;
			}
		}
		if (ok) {
			gtk_list_store_append(GTK_LIST_STORE(m_ExpList), &iter);
			gtk_list_store_set(GTK_LIST_STORE(m_ExpList), &iter, DATA_CHECK, false, 
				DATA_VALUE, ((CFrameInfo*)ptr->data)->ExpTime(), DATA_FILES, 0, -1);
		}
	}
	// Object names
	gtk_list_store_clear(GTK_LIST_STORE(m_ObjList));
	for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
		ok = true;
		for (GList *aux=ptr->prev; aux!=NULL; aux=aux->prev) {
			if (StrCmp0(((CFrameInfo*)ptr->data)->Object(), ((CFrameInfo*)aux->data)->Object())==0) {
				ok = false;
				break;
			}
		}
		if (ok) {
			gtk_list_store_append(GTK_LIST_STORE(m_ObjList), &iter);
			gtk_list_store_set(GTK_LIST_STORE(m_ObjList), &iter, DATA_CHECK, false, 
				DATA_VALUE, ((CFrameInfo*)ptr->data)->Object(), DATA_FILES, 0, -1);
		}
	}
}


void CAddFilterDlg::UpdateFilterPage(void)
{
	bool ok;
	int count, files;
	char *filter, aux[64], buf[256];
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 0);
	GtkTreeIter iter;

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), NULL);
	// For each filter, compute number of files
	files = 0;
	ok = gtk_tree_model_get_iter_first(data->model, &iter)!=0;
	while (ok) {
		count = 0;
		gtk_tree_model_get(data->model, &iter, DATA_VALUE, &filter, -1);
		for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
			if (StrCmp0(filter, ((CFrameInfo*)ptr->data)->Filter())==0)
				count++;
		}
		g_free(filter);
		gtk_list_store_set(GTK_LIST_STORE(data->model), &iter, DATA_FILES, count, -1);
		ok = gtk_tree_model_iter_next(data->model, &iter)!=0;
		files += count;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), data->fmodel);

	PrintFiles(aux, files);
	g_snprintf(buf, sizeof(buf), "All files (%s)", aux);
	gtk_button_set_label(GTK_BUTTON(data->check), buf);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->check), data->allfiles);
	gtk_widget_set_sensitive(data->view, !data->allfiles);

	UpdateTotal(data);
}

void CAddFilterDlg::UpdateExposurePage(void)
{
	bool ok;
	int count, files;
	double exptime;
	char aux[64], buf[256];
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 1);
	GtkTreeIter iter;

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), NULL);
	// For each filter, compute number of files
	files = 0;
	ok = gtk_tree_model_get_iter_first(data->model, &iter)!=0;
	while (ok) {
		count = 0;
		gtk_tree_model_get(data->model, &iter, DATA_VALUE, &exptime, -1);
		for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
			if (IsFilterSelected(((CFrameInfo*)ptr->data)->Filter())) {
				if (exptime == ((CFrameInfo*)ptr->data)->ExpTime())
					count++;
			}
		}
		gtk_list_store_set(GTK_LIST_STORE(data->model), &iter, DATA_FILES, count, -1);
		ok = gtk_tree_model_iter_next(data->model, &iter)!=0;
		files += count;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), data->fmodel);

	PrintFiles(aux, files);
	g_snprintf(buf, sizeof(buf), "All files (%s)", aux);
	gtk_button_set_label(GTK_BUTTON(data->check), buf);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->check), data->allfiles);
	gtk_widget_set_sensitive(data->view, !data->allfiles);

	UpdateTotal(data);
}

void CAddFilterDlg::UpdateObjectPage(void)
{
	bool ok;
	int count, files;
	char *object, aux[64], buf[256];
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 2);
	GtkTreeIter iter;

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), NULL);
	// For each filter, compute number of files
	files = 0;
	ok = gtk_tree_model_get_iter_first(data->model, &iter)!=0;
	while (ok) {
		count = 0;
		gtk_tree_model_get(data->model, &iter, DATA_VALUE, &object, -1);
		for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
			if (IsFilterSelected(((CFrameInfo*)ptr->data)->Filter()) &&
				IsExposureSelected(((CFrameInfo*)ptr->data)->ExpTime())) {
				if (StrCmp0(object, ((CFrameInfo*)ptr->data)->Object())==0)
					count++;
			}
		}
		g_free(object);
		gtk_list_store_set(GTK_LIST_STORE(data->model), &iter, DATA_FILES, count, -1);
		ok = gtk_tree_model_iter_next(data->model, &iter)!=0;
		files += count;
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->view), data->fmodel);

	PrintFiles(aux, files);
	g_snprintf(buf, sizeof(buf), "All files (%s)", aux);
	gtk_button_set_label(GTK_BUTTON(data->check), buf);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->check), data->allfiles);
	gtk_widget_set_sensitive(data->view, !data->allfiles);

	UpdateTotal(data);
}

void CAddFilterDlg::ProcessFiles()
{
	int count;
	char buf[256];

	count = 0;
	g_MainWnd->BeginUpdate();
	g_Project->Lock();
	for (GList *ptr=m_Frames; ptr!=NULL; ptr=ptr->next) {
		if (IsFilterSelected(((CFrameInfo*)ptr->data)->Filter()) &&
			IsExposureSelected(((CFrameInfo*)ptr->data)->ExpTime()) &&
			IsObjectSelected(((CFrameInfo*)ptr->data)->Object())) {
				GtkTreePath *path = g_Project->AddFile(*((CFrameInfo*)ptr->data));
				if (path) {
					count++;
					gtk_tree_path_free(path);
				}
		}
	}
	g_Project->Unlock();
	g_MainWnd->EndUpdate();

	if (count==0)
		gtk_label_set_text(GTK_LABEL(m_pLast), "No file was added to the project.");
	else if (count==1)
		gtk_label_set_text(GTK_LABEL(m_pLast), "1 file was added to the project.");
	else {
		sprintf(buf, "%d files were added to the project.", count);
		gtk_label_set_text(GTK_LABEL(m_pLast), buf);
	}
}

void CAddFilterDlg::GoToFirstPage(void)
{
	tPageData *data;

	m_FirstPage = 0;
	UpdateFilterPage();
	if (FilterItemCount()>1) {
		GoToPage(0);
		return;
	}
	data = (tPageData*)g_slist_nth_data(m_Pages, 0);
	data->skip = true;
	m_FirstPage = 1;
	UpdateExposurePage();
	if (ExposureItemCount()>1) {
		GoToPage(1);
		return;
	}
	data = (tPageData*)g_slist_nth_data(m_Pages, 1);
	data->skip = true;
	m_FirstPage = 2;
	UpdateObjectPage();
	if (ObjectItemCount()>1) {
		GoToPage(2);
		return;
	}
	data = (tPageData*)g_slist_nth_data(m_Pages, 2);
	data->skip = true;
	m_FirstPage = 3;
	ProcessFiles();
	GoToPage(3);
}

void CAddFilterDlg::GoForward(void)
{
	tPageData *data;

	int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pTabs));
	switch (page)
	{
	case 0:
		// Go to 'exposure' page
		UpdateExposurePage();
		if (ExposureItemCount()>1) {
			GoToPage(1);
			break;
		}
		data = (tPageData*)g_slist_nth_data(m_Pages, 1);
		data->skip = true;
		// fallthrough
	case 1:
		// Go to 'object' page
		UpdateObjectPage();
		if (ObjectItemCount()>1) {
			GoToPage(2);
			break;
		}
		data = (tPageData*)g_slist_nth_data(m_Pages, 2);
		data->skip = true;
		// fallthrough
	default:
		// Add files and go to 'summary' page
		ProcessFiles();
		GoToPage(3);
	}
}

void CAddFilterDlg::GoBackward()
{
	tPageData *data;

	int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pTabs));
	switch (page)
	{
	case 2:
		// Go to 'exposure' page
		data = (tPageData*)g_slist_nth_data(m_Pages, 1);
		if (!data->skip) {
			GoToPage(1);
			return;
		}
		// falltrough
	case 1:
		// Go to 'filter' page
		data = (tPageData*)g_slist_nth_data(m_Pages, 0);
		if (!data->skip) { 
			GoToPage(0);
			return;
		}
	default:
		// Do nothing
		break;
	}
}

void CAddFilterDlg::GoToPage(int page)
{
	switch (page)
	{
	case 0:
		// Go to 'filter' page
		gtk_widget_hide(m_pBack);
		gtk_widget_hide(m_pClose);
		gtk_widget_show(m_pCancel);
		gtk_widget_show(m_pNext);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pTabs), 0);
		break;

	case 1:
		// Go to 'exposure' page
		if (m_FirstPage<1)
			gtk_widget_show(m_pBack);
		else
			gtk_widget_hide(m_pBack);
		gtk_widget_hide(m_pClose);
		gtk_widget_show(m_pCancel);
		gtk_widget_show(m_pNext);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pTabs), 1);
		break;

	case 2:
		// Go to 'object' page
		if (m_FirstPage<2)
			gtk_widget_show(m_pBack);
		else
			gtk_widget_hide(m_pBack);
		gtk_widget_hide(m_pClose);
		gtk_widget_show(m_pCancel);
		gtk_widget_show(m_pNext);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pTabs), 2);
		break;

	case 3:
		// Add files and go to 'summary' page
		gtk_widget_hide(m_pBack);
		gtk_widget_show(m_pClose);
		gtk_widget_hide(m_pCancel);
		gtk_widget_hide(m_pNext);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pTabs), 3);
	}
}

bool CAddFilterDlg::IsFilterSelected(const char *filter)
{
	gboolean ok, checked;
	bool retval = true;
	char *str;
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 0);
	GtkTreeIter iter;

	if (!data->allfiles && !data->skip) {
		ok = gtk_tree_model_get_iter_first(data->model, &iter);
		while (ok) {
			gtk_tree_model_get(data->model, &iter, DATA_VALUE, &str, DATA_CHECK, &checked, -1);
			if (StrCmp0(str, filter)==0) {
				retval = checked!=0;
				break;
			}
			g_free(str);
			ok = gtk_tree_model_iter_next(data->model, &iter);
		}
	}
	return retval;
}

int CAddFilterDlg::FilterItemCount()
{
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 0);
	return gtk_tree_model_iter_n_children(data->fmodel, NULL);
}

bool CAddFilterDlg::IsExposureSelected(double exptime)
{
	gboolean ok, checked;
	bool retval = true;
	double value;
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 1);
	GtkTreeIter iter;

	if (!data->allfiles && !data->skip) {
		ok = gtk_tree_model_get_iter_first(data->model, &iter);
		while (ok) {
			gtk_tree_model_get(data->model, &iter, DATA_VALUE, &value, DATA_CHECK, &checked, -1);
			if (value == exptime) {
				retval = checked!=0;
				break;
			}
			ok = gtk_tree_model_iter_next(data->model, &iter);
		}
	}
	return retval;
}

int CAddFilterDlg::ExposureItemCount()
{
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 1);
	return gtk_tree_model_iter_n_children(data->fmodel, NULL);
}

bool CAddFilterDlg::IsObjectSelected(const char *object)
{
	gboolean ok, checked;
	bool retval = true;
	char *str;
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 2);
	GtkTreeIter iter;

	if (!data->allfiles && !data->skip) {
		ok = gtk_tree_model_get_iter_first(data->model, &iter);
		while (ok) {
			gtk_tree_model_get(data->model, &iter, DATA_VALUE, &str, DATA_CHECK, &checked, -1);
			if (StrCmp0(str, object)==0) {
				retval = checked!=0;
				break;
			}
			g_free(str);
			ok = gtk_tree_model_iter_next(data->model, &iter);
		}
	}
	return retval;
}

int CAddFilterDlg::ObjectItemCount()
{
	tPageData *data = (tPageData*)g_slist_nth_data(m_Pages, 2);
	return gtk_tree_model_iter_n_children(data->fmodel, NULL);
}
