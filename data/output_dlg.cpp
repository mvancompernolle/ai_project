/**************************************************************

output_dlg.cpp (C-Munipack project)
The base class for output windows
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

#include "output_dlg.h"
#include "main.h"
#include "utils.h"
#include "ctxhelp.h"
#include "frameinfo_dlg.h"

//-------------------------   HELPER FUNCTIONS   --------------------------------

// 
// Keeps frame identifier and its path
// The path is stored as row reference, so it gets updated
// when the file list changes. The path method returns NULL
// if the frame was deleted.
//
class CLCFrame
{
public:
	// Constructors
	CLCFrame(GtkTreeModel *tree_model, GtkTreePath *tree_path):m_id(-1)
	{ 
		m_ref = gtk_tree_row_reference_new(tree_model, tree_path); 
		UpdateId();
	}
	CLCFrame(GtkTreeRowReference *ref):m_id(-1)
	{ 
		m_ref = gtk_tree_row_reference_copy(ref); 
		UpdateId();
	}
	CLCFrame(const CLCFrame &other):m_id(other.m_id)
	{ m_ref = gtk_tree_row_reference_copy(other.m_ref); }

	// Destructor
	~CLCFrame() 
	{ gtk_tree_row_reference_free(m_ref); }

	// Is the frame valid
	gboolean valid(void)
	{ return gtk_tree_row_reference_valid(m_ref); }

	// Get path
	GtkTreePath *path(void)
	{ return gtk_tree_row_reference_get_path(m_ref); }

	// Get frame identifier
	gint frameId(void) const
	{ return m_id; }
	
	// Returns TRUE if given path corresponds to the frame
	gboolean isEqual(GtkTreePath *other)
	{ 
		gboolean retval = FALSE;
		if (other) {
			GtkTreePath *p = path();
			if (p) {
				retval = gtk_tree_path_compare(p, other)==0;
				gtk_tree_path_free(p);
			}
		}
		return retval;
	}

private:
	gint m_id;
	GtkTreeRowReference *m_ref;

	// Read frame identifier from the model
	void UpdateId(void)
	{
		GtkTreeIter iter;

		GtkTreePath *p = path();
		if (p) {
			GtkTreeModel *model = gtk_tree_row_reference_get_model(m_ref);
			if (gtk_tree_model_get_iter(model, &iter, p)) 
				gtk_tree_model_get(g_Project->FileList(), &iter, FRAME_ID, &m_id, -1);
			gtk_tree_path_free(p);
		} else {
			m_id = -1;
		}
	}
};

// Free CLCFrame instance
static void Free_LCFrame(gpointer data, gpointer user_data)
{
	delete (CLCFrame*)data;
}

// Check if it is on the list already
static bool IsOnList(GSList *list, GtkTreePath *path)
{
	for (GSList *f=list; f!=NULL; f=f->next) {
		if (((CLCFrame*)f->data)->isEqual(path)) 
			return true;
	}
	return false;
}

static gboolean foreach_all_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	*list = g_slist_append(*list, new CLCFrame(model, path));
	return FALSE;
}

static gboolean foreach_matched_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	if (g_Project->GetState(iter) & CFILE_MATCHING) 
		*list = g_slist_append(*list, new CLCFrame(model, path));
	return FALSE;
}

static void foreach_selected_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	*list = g_slist_append(*list, new CLCFrame(model, path));
}

static void foreach_selected_matched_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	if (g_Project->GetState(iter) & CFILE_MATCHING) 
		*list = g_slist_append(*list, new CLCFrame(model, path));
}

//-----------------------------   OUTPUT DIALOG   ------------------------------------

//
// Constructor
//
COutputDlg::COutputDlg(void):m_StatusCtx(-1), m_StatusMsg(-1)
{
	GdkRectangle rc;

	// Register dialog
	g_MainWnd->RegisterOutputDlg(this);

	// Dialog with buttons
	m_pDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(m_pDlg), "destroy", G_CALLBACK(destroyed), this);
	g_signal_connect(G_OBJECT(m_pDlg), "realize", G_CALLBACK(realized), this);

	// Dialog size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.7*rc.width), RoundToInt(0.7*rc.height));
	gtk_window_set_position(GTK_WINDOW(m_pDlg), GTK_WIN_POS_CENTER);

	// Window layout
	m_MainBox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_pDlg), m_MainBox);

	// Status bar
	m_Status = gtk_statusbar_new();
	gtk_box_pack_end(GTK_BOX(m_MainBox), m_Status, FALSE, FALSE, 0);
	m_StatusCtx = gtk_statusbar_get_context_id(GTK_STATUSBAR(m_Status), "Main");
}

//
// Dialog initialization
//
void COutputDlg::OnInitDialog(void)
{
	const char *icon_name = GetIconName();
	gchar *icon_file = get_icon_file((icon_name ? icon_name : "muniwin"));
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon_file, NULL));
	g_free(icon_file);
}

//
// Destructor
//
COutputDlg::~COutputDlg()
{
	// Unregister dialog from the main frame
	g_MainWnd->OutputDlgClosed(this);
}

//
// Hide a window
//
void COutputDlg::Close()
{
	gtk_widget_destroy(GTK_WIDGET(m_pDlg));
}

//
// Unregister window
//
void COutputDlg::destroyed(GtkObject *pWnd, COutputDlg *pDlg)
{
	delete pDlg;
}

//
// Show a window
//
void COutputDlg::Show(void)
{
	gtk_window_present(GTK_WINDOW(m_pDlg));
}

//
// Set status text
//
void COutputDlg::SetStatus(const char *text)
{
	if (m_StatusMsg>=0) {
		gtk_statusbar_pop(GTK_STATUSBAR(m_Status), m_StatusCtx);
		m_StatusMsg = -1;
	}
	if (text && strlen(text)>0) 
		m_StatusMsg = gtk_statusbar_push(GTK_STATUSBAR(m_Status), m_StatusCtx, text);
}

//
// Dialog initialization
//
void COutputDlg::realized(GtkWidget *pWidget, COutputDlg *pDlg)
{
	pDlg->OnInitDialog();
}

//-----------------------------   OUTPUT DIALOG   ------------------------------------

//
// Constructor
//
COutputCurveDlg::COutputCurveDlg(void):m_FileList(NULL), m_NewFiles(NULL), 
	m_RawFiles(false), m_DataSaved(false), m_AllFiles(false), m_Updating(false), 
	m_DeletedFiles(false), m_Timer(0)
{
	// Monitor changes in project
	GtkTreeModel *model = g_Project->FileList();
	g_signal_connect(G_OBJECT(model), "row-changed", G_CALLBACK(frame_changed), this);
	g_signal_connect(G_OBJECT(model), "row-deleted", G_CALLBACK(frame_deleted), this);

}

//
// Destructor
//
COutputCurveDlg::~COutputCurveDlg()
{
	if (m_Timer)
		g_source_remove(m_Timer);

	// Disconnect project signals
	GtkTreeModel *model = g_Project->FileList();
	g_signal_handlers_disconnect_by_func(G_OBJECT(model), (gpointer)frame_changed, this);
	g_signal_handlers_disconnect_by_func(G_OBJECT(model), (gpointer)frame_deleted, this);

	g_slist_foreach(m_FileList, (GFunc)Free_LCFrame, NULL);
	g_slist_free(m_FileList);
	g_slist_foreach(m_NewFiles, (GFunc)Free_LCFrame, NULL);
	g_slist_free(m_NewFiles);
}

//
// Dialog initialization
//
void COutputCurveDlg::OnInitDialog(void)
{
	COutputDlg::OnInitDialog();

	m_Timer = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 3000, GSourceFunc(update_timer), this, NULL);
}

//
// Initialize list of files
//
bool COutputCurveDlg::InitFileList(GtkWindow *parent, bool selected_files)
{
	g_slist_foreach(m_FileList, (GFunc)Free_LCFrame, NULL);
	g_slist_free(m_FileList);
	g_slist_foreach(m_NewFiles, (GFunc)Free_LCFrame, NULL);
	g_slist_free(m_NewFiles);
	m_FileList = m_NewFiles = NULL;
	m_DeletedFiles = false;
	m_DataSaved = false;

	m_AllFiles = !selected_files;
	if (m_AllFiles) {
		// All files
		if (!m_RawFiles)
			gtk_tree_model_foreach(g_Project->FileList(), (GtkTreeModelForeachFunc)foreach_matched_files, &m_FileList);
		else
			gtk_tree_model_foreach(g_Project->FileList(), (GtkTreeModelForeachFunc)foreach_all_files, &m_FileList);
		if (!m_FileList) {
			ShowError(parent, "There are no files in the project.");
			return false;
		}
	} else {
		// Selected files
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		if (gtk_tree_selection_count_selected_rows(pSel)>0) {
			if (!m_RawFiles)
				gtk_tree_selection_selected_foreach(pSel, (GtkTreeSelectionForeachFunc)foreach_selected_matched_files, &m_FileList);
			else
				gtk_tree_selection_selected_foreach(pSel, (GtkTreeSelectionForeachFunc)foreach_selected_files, &m_FileList);
		}
		if (!m_FileList) {
			ShowError(parent, "There are no selected files.");
			return false;
		}
	}
	return true;
}

//
// Rebuild frame set
//
bool COutputCurveDlg::RebuildData(GtkWindow *parent)
{
	int res = 0;

	m_Updating = true;
	m_DataSaved = false;
	UpdateFileList();
	if (!m_RawFiles) {
		CProgressDlg pDlg(GTK_WINDOW(m_pDlg), "Processing files");
		pDlg.SetMinMax(0, g_slist_length(m_FileList));
		res = pDlg.Execute(ExecuteProc, this);
	} else {
		res = OnProcessFiles(NULL);
	}
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		ShowError(parent, msg, true);
		cmpack_free(msg);
	}
	m_Updating = false;
	return (res==0);
}

int COutputCurveDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((COutputCurveDlg*)userdata)->OnProcessFiles(sender);
}

int COutputCurveDlg::OnProcessFiles(CProgressDlg *sender)
{
	int res, infiles;
	CListProc list;

	m_FrameSet.Init(m_AperList, m_Selection);

	// Make listing context
	res = list.Init(sender, &m_FrameSet, m_RawFiles);
	if (res==0) {
		// Process files
		infiles = 0;
		for (GSList *node = m_FileList; node != NULL; node = node->next) {
			GtkTreePath *path = ((CLCFrame*)node->data)->path();
			if (path) {
				res = list.Add(path);
				gtk_tree_path_free(path);
				if (res!=0)
					break;
			}
			if (sender) {
				if (sender->Cancelled())
					break;
				sender->SetProgress(infiles++);
			}
		}
	}
	return res;
}

//
// Update list of files
//
void COutputCurveDlg::UpdateFileList(void)
{
	// Append new files
	if (m_NewFiles) {
		m_FileList = g_slist_concat(m_FileList, m_NewFiles);
		m_NewFiles = NULL;
	}
	// Remove deleted files
	GSList *new_list = NULL;
	for (GSList *f=m_FileList; f!=NULL; f=f->next) {
		if (!((CLCFrame*)f->data)->valid()) {
			delete (CLCFrame*)f->data;
			f->data = NULL;
		} else
			new_list = g_slist_prepend(new_list, f->data);
	}
	g_slist_free(m_FileList);
	m_FileList = g_slist_reverse(new_list);
}

void COutputCurveDlg::frame_changed(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, COutputCurveDlg *pMe)
{
	pMe->OnFrameChanged(tree_model, path, iter);
}

void COutputCurveDlg::OnFrameChanged(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter)
{
	// If the light curve is done for selected files only,
	// do not all new files to the selection
	if (m_AllFiles && !IsOnList(m_FileList, path) && !IsOnList(m_NewFiles, path)) {
		CLCFrame *it = new CLCFrame(tree_model, path);
		m_NewFiles = g_slist_prepend(m_NewFiles, it);
	}
}

void COutputCurveDlg::frame_deleted(GtkTreeModel *tree_model, GtkTreePath *path, COutputCurveDlg *pMe)
{
	pMe->OnFrameDeleted(tree_model, path);
}

void COutputCurveDlg::OnFrameDeleted(GtkTreeModel *tree_model, GtkTreePath *path)
{
	m_DeletedFiles = true;
}

// 3 sec. timer
gboolean COutputCurveDlg::update_timer(COutputCurveDlg *pDlg)
{
	pDlg->OnUpdateCurve();
	return TRUE;
}

void COutputCurveDlg::OnUpdateCurve(void)
{
	bool	changed;
	int		frame_id, state;
	GtkTreeIter iter;
	GtkTreeModel *model;

	gdk_threads_lock();
	if ((m_NewFiles || m_DeletedFiles) && !m_Updating) {
		m_Updating = true;
		changed = false;
		model = g_Project->FileList();
		// Append new files
		if (m_NewFiles) {
			for (GSList *f=m_NewFiles; f!=NULL; f=f->next) {
				GtkTreePath *path = ((CLCFrame*)f->data)->path();
				if (path) {
					if (!IsOnList(m_FileList, path)) {
						gtk_tree_model_get_iter(model, &iter, path);
						gtk_tree_model_get(model, &iter, FRAME_STATE, &state, FRAME_ID, &frame_id, -1);
						if (frame_id>=0 && (state & CFILE_MATCHING)!=0) {
							gchar *pht_file = g_Project->GetPhotFile(path);
							CPhot infile;
							if (pht_file && infile.Load(pht_file)) {
								if (m_FrameSet.AppendFrame(infile, frame_id, NULL)) {
									CLCFrame *it = new CLCFrame(*((CLCFrame*)f->data));
									m_FileList = g_slist_prepend(m_FileList, it);
									changed = true;
								}
							}
							g_free(pht_file);
						}
					}
					gtk_tree_path_free(path);
				}
			}
			g_slist_foreach(m_NewFiles, (GFunc)Free_LCFrame, NULL);
			g_slist_free(m_NewFiles);
			m_NewFiles = NULL;
		}
		// Remove deleted files
		if (m_DeletedFiles) {
			GSList *new_list = NULL;
			for (GSList *f=m_FileList; f!=NULL; f=f->next) {
				if (!((CLCFrame*)f->data)->valid()) {
					m_FrameSet.DeleteFrame(((CLCFrame*)f->data)->frameId());
					delete (CLCFrame*)f->data;
					f->data = NULL;
					changed = true;
				} else
					new_list = g_slist_prepend(new_list, f->data);
			}
			g_slist_free(m_FileList);
			m_FileList = g_slist_reverse(new_list);
			m_DeletedFiles = false;
		}
		// Update light curve and graph
		if (changed) {
			OnFrameSetChanged();
			m_DataSaved = false;
		}
		m_Updating = false;
	}
	gdk_threads_unlock();
}

void COutputCurveDlg::RemoveFromDataSet(void)
{
	bool changed = false;

	GList *rows = GetSelectedFrames();
	if (rows) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			GtkTreePath *path = (GtkTreePath*)ptr->data;
			if (path) {
				int frame = g_Project->GetFrameID(path);
				m_FrameSet.DeleteFrame(frame);
				changed = true;
			}
		}
		g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rows);
		if (changed) {
			m_DataSaved = false;
			OnFrameSetChanged();
		}
	}
}

void COutputCurveDlg::DeleteFromProject(void)
{
	bool ok, changed = false;

	GList *rows = GetSelectedFrames();
	if (rows) {
		int count = g_list_length(rows);
		bool selected_ref = false;
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			if (g_Project->IsReferenceFrame((GtkTreePath*)ptr->data)) {
				selected_ref = true;
				break;
			}
		}
		if (selected_ref) {
			if (count==1) 
				ShowError(GTK_WINDOW(m_pDlg), "The selected frame is a reference frame. It it not allowed to remove it.");
			else
				ShowError(GTK_WINDOW(m_pDlg), "The selection includes a reference frame. It is not allowed to remove it.");
			ok = false;
		} else {
			if (count==1) 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frame from the project?");
			else 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frames from the project?");
		}
		if (ok) {
			for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
				GtkTreePath *path = (GtkTreePath*)ptr->data;
				if (path) {
					int frame = g_Project->GetFrameID(path);
					m_FrameSet.DeleteFrame(frame);
					g_Project->RemoveFrame(frame);
					changed = true;
				}
			}
		}
		g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rows);
		if (changed) {
			m_DataSaved = false;
			OnFrameSetChanged();
		}
	}
}

void COutputCurveDlg::ShowFramePreview(void)
{
	GtkTreePath *path = GetSelectedFrame();
	if (path) {
		g_MainWnd->ShowFramePreview(path);
		gtk_tree_path_free(path);
	}
}

void COutputCurveDlg::ShowFrameInfo(void)
{
	GtkTreePath *path = GetSelectedFrame();
	if (path) {
		CFrameInfoDlg dlg(GTK_WINDOW(m_pDlg));
		dlg.Show(path);
		gtk_tree_path_free(path);
	}
}
