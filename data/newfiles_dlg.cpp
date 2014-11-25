/**************************************************************

newfiles_dlg.cpp (C-Munipack project)
The 'Process new frames' dialog
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

#include "configuration.h"
#include "newfiles_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "messages_dlg.h"
#include "ctxhelp.h"

//-------------------   HELPER FUNCTIONS   ---------------------------------

static gboolean foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, gpointer userdata)
{
	int state;
	GList **list = ((GList**)userdata);
	gtk_tree_model_get(model, iter, FRAME_STATE, &state, -1);
	if ((state & CFILE_CONVERSION)==0) {
		GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
		*list = g_list_append(*list, rowref);
	}
	return FALSE;
}

// Free the CFrameInfo structure
static void FileFree(CFrameInfo *data, gpointer *user_data)
{
	delete data;
}

// Compare two frame descriptors by time of observation (for sorting)
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

//-------------------   HELPER CLASSES   ---------------------------------

//
// Directory entry
//
class CDirEntry
{
public:
	CDirEntry() 
	{ m_htab = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, delete_item); }
	
	~CDirEntry()
	{ g_hash_table_destroy(m_htab); }
	
	void Clear(void)
	{ g_hash_table_remove_all(m_htab); }

	CDirEntry *AddDir(const gchar *filename)
	{ 
		CDirEntry *entry = new CDirEntry();
		g_hash_table_insert(m_htab, g_strdup(filename), entry); 
		return entry;
	}

	void AddFile(const gchar *filename)
	{ g_hash_table_insert(m_htab, g_strdup(filename), NULL); }

	bool Lookup(const gchar *filename, CDirEntry **entry)
	{ return g_hash_table_lookup_extended(m_htab, filename, NULL, (gpointer*)entry)!=0; }

protected:
	GHashTable *m_htab;

	static void delete_item(gpointer p)
	{ delete (CDirEntry*)p; }

};

//
// Constructor
//
CNewFilesChecker::CNewFilesChecker(void):m_DirPath(NULL), m_Prefix(NULL), m_Filter(NULL), 
	m_Object(NULL), m_Recursive(false), m_CBProc(NULL), m_CBData(NULL)
{
	m_Dir = new CDirEntry();
}

//
// Destructor
//
CNewFilesChecker::~CNewFilesChecker() 
{
	delete m_Dir;
	g_free(m_DirPath);
	g_free(m_Prefix);
	g_free(m_Filter);
	g_free(m_Object);
}

//
// Read configuration
//
bool CNewFilesChecker::Init()
{
	Clear();
	m_DirPath = g_Project->GetStr("AddFiles", "Folder", NULL);
	m_Recursive = g_Project->GetBool("NewFiles", "SubDirs");
	if (g_Project->GetBool("NewFiles", "Name"))
		m_Prefix = g_Project->GetStr("NewFiles", "NameText", "");
	if (g_Project->GetBool("NewFiles", "Filter"))
		m_Filter = g_Project->GetStr("NewFiles", "FilterText", "");
	if (g_Project->GetBool("NewFiles", "Object"))
		m_Object = g_Project->GetStr("NewFiles", "ObjectText", "");
	return m_DirPath && g_file_test(m_DirPath, G_FILE_TEST_IS_DIR);
}

//
// Check for new files
//
bool CNewFilesChecker::Check(NewFrameProc *cb_proc, gpointer cb_data)
{
	m_CBProc = cb_proc;
	m_CBData = cb_data;
	if (m_DirPath && m_Dir && m_CBProc) 
		return FindNewFiles(m_DirPath, m_Dir);
	return FALSE;
}

//
// Clear all stored information
//
void CNewFilesChecker::Clear()
{
	m_Dir->Clear();
	g_free(m_DirPath);
	m_DirPath = NULL;
	g_free(m_Filter);
	m_Filter = NULL;
	g_free(m_Prefix);
	m_Prefix = NULL;
	g_free(m_Object);
	m_Object = NULL;
}

// Check new files in given directory, updates the entry. New files
// are prepended to the given list and a new list is returned as 
// return value. Calls recursively itself for subdirectories.
bool CNewFilesChecker::FindNewFiles(const gchar *dirpath, CDirEntry *entry)
{
	bool retval = true;
	CDirEntry *child;

	GDir *dir = g_dir_open(dirpath, 0, NULL);
	if (dir) {
		const gchar *filename = g_dir_read_name(dir);
		while (filename && retval) {
			gchar *filepath = g_build_filename(dirpath, filename, NULL);
			if (!entry->Lookup(filename, &child)) {
				// New item
				if (m_Recursive && g_file_test(filepath, G_FILE_TEST_IS_DIR)) {
					// Recursive call for new subdirectories
					CDirEntry *child = entry->AddDir(filename);
					retval = FindNewFiles(filepath, child);
				} else 
				if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR)) {
					// Test regular files
					bool try_again = FALSE;
					retval = CheckNewFile(filepath, &try_again);
					if (!try_again)
						entry->AddFile(filename);
				}
			} else {
				if (m_Recursive && child && g_file_test(filepath, G_FILE_TEST_IS_DIR)) {
					// Recursive call for old subdirectories
					retval = FindNewFiles(filepath, child);
				}
			}
			g_free(filepath);
			filename = g_dir_read_name(dir);
		}
		g_dir_close(dir);
	}
	return retval;
}

// Check single file. If the is a valid CCD frame, creates a CFrameInfo 
// instance and prepends it the the list given as the last parameter.
// The new list is returned as return value.
bool CNewFilesChecker::CheckNewFile(const gchar *filepath, bool *try_again)
{
	bool retval = true;
	CFrameInfo info;

	gchar *basename = g_path_get_basename(filepath);
	if (!m_Prefix || CheckFileName(basename, m_Prefix)) {
		if (!g_Project->FileInProject(filepath)) {
			GError *error = NULL;
			if (!info.Init(filepath, &error)) {
				if (error) {
					*try_again = (error->code==CMPACK_ERR_CANT_OPEN_SRC);
					g_error_free(error);
				}
			} else {
				if ((!m_Filter || StrCaseCmp0(m_Filter, info.Filter())==0) &&
					(!m_Object || StrCaseCmp0(m_Object, info.Object())==0)) {
						retval = m_CBProc(info, m_CBData, try_again);
				}
			}
		}
	}
	g_free(basename);
	return retval;
}

// Test filename filter
bool CNewFilesChecker::CheckFileName(const gchar *filepath, const gchar *prefix)
{
	bool retval;
	gchar *basename = g_path_get_basename(filepath);
#ifdef _WIN32
	retval = StrCaseStr0((gchar*)basename, prefix)==basename;
#else
	retval = StrStr0((gchar*)basename, prefix)==basename;
#endif
	g_free(basename);
	return retval;
}

//-------------------------   MAIN WINDOW   --------------------------------

CNewFilesDlg::CNewFilesDlg(GtkWindow *pParent):m_pParent(pParent), m_InFiles(0), m_OutFiles(0), 
	m_FrameList(NULL)
{
	GtkWidget *tbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Process new frames", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("newfiles");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Frame
	tbox = gtk_table_new(8, 4, FALSE);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 8);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 0, 32);
	
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "Process all unprocessed frames in the current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Process all unprocessed frames in the current project");
	g_signal_connect(G_OBJECT(m_AllBtn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_AllBtn, 0, 4, 0, 1);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_FilesBtn = gtk_radio_button_new_with_label(group, "Process new files in the directory:");
	gtk_widget_set_tooltip_text(m_FilesBtn, "Find new frames in specified location, append them to the project and process them");
	g_signal_connect(G_OBJECT(m_FilesBtn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FilesBtn, 0, 4, 1, 2);

	// Directory path
	m_DirEdit = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_DirEdit, "Path to the directory where new frames are searched for");
	gtk_table_attach(GTK_TABLE(tbox), m_DirEdit, 1, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_set_size_request(m_DirEdit, 320, -1);
	m_DirBtn = gtk_button_new_with_label("Browse");
	gtk_widget_set_tooltip_text(m_DirBtn, "Change search directory");
	g_signal_connect(G_OBJECT(m_DirBtn), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), m_DirBtn, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

	// Include subdirectories
	m_SubDirs = gtk_check_button_new_with_label("Include subdirectories");
	gtk_widget_set_tooltip_text(m_SubDirs, "Search frames also in all sub-directories in the selected folder");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_SubDirs, 1, 4, 3, 4);

	m_ProcLabel = gtk_label_new("Process only files which meet the following conditions:");
	gtk_misc_set_alignment(GTK_MISC(m_ProcLabel), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ProcLabel, 1, 4, 4, 5);

	// File name
	m_NameCheck = gtk_check_button_new_with_label("File name prefix");
	gtk_widget_set_tooltip_text(m_NameCheck, "Use only files that start with a specified string");
	g_signal_connect(G_OBJECT(m_NameCheck), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_NameCheck, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	m_NameEdit = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_NameEdit, "File name prefix");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_NameEdit, 2, 3, 5, 6);

	// Filter
	m_FilterCheck = gtk_check_button_new_with_label("Filter");
	gtk_widget_set_tooltip_text(m_FilterCheck, "Use only frames that have been acquired with specified color filter");
	g_signal_connect(G_OBJECT(m_FilterCheck), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_FilterCheck, 1, 2, 6, 7, GTK_FILL, GTK_FILL, 0, 0);
	m_FilterEdit = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_FilterEdit, "Color filter designation");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FilterEdit, 2, 3, 6, 7);

	// Object name
	m_ObjectCheck = gtk_check_button_new_with_label("Object name");
	gtk_widget_set_tooltip_text(m_ObjectCheck, "Use only frames of specified object");
	g_signal_connect(G_OBJECT(m_ObjectCheck), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_ObjectCheck, 1, 2, 7, 8, GTK_FILL, GTK_FILL, 0, 0);
	m_ObjectEdit = gtk_entry_new();
	gtk_widget_set_tooltip_text(m_ObjectEdit, "Object's designation");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ObjectEdit, 2, 3, 7, 8);

	m_OptLabel = gtk_label_new("Run this tool on background:");
	gtk_misc_set_alignment(GTK_MISC(m_OptLabel), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_OptLabel, 1, 4, 8, 9);

	// Background process
	m_BgCheck = gtk_check_button_new_with_label("Periodically check and process new frames in the directory");
	gtk_widget_set_tooltip_text(m_BgCheck, "Start a new background process that checks for new files in specified directory, add them to the current project and process.");
	g_signal_connect(G_OBJECT(m_BgCheck), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_BgCheck, 1, 4, 9, 10, GTK_FILL, GTK_FILL, 0, 0);
	
	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CNewFilesDlg::~CNewFilesDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CNewFilesDlg::response_dialog(GtkDialog *pDlg, gint response_id, CNewFilesDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CNewFilesDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_PROCESS_NEW_FRAMES);
		return false;
	}
	return true;
}

void CNewFilesDlg::button_clicked(GtkButton *widget, CNewFilesDlg *pMe)
{
	pMe->OnButtonClicked(widget);
}

void CNewFilesDlg::OnButtonClicked(GtkButton *widget)
{
	if (widget == GTK_BUTTON(m_DirBtn))
		EditDirPath();
}

void CNewFilesDlg::button_toggled(GtkToggleButton *widget, CNewFilesDlg *pMe)
{
	pMe->OnButtonToggled(widget);
}

void CNewFilesDlg::OnButtonToggled(GtkToggleButton *widget)
{
	UpdateControls();
}

void CNewFilesDlg::UpdateControls(void)
{
	bool ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FilesBtn))!=0;
	gtk_widget_set_sensitive(m_SubDirs, ok);
	gtk_widget_set_sensitive(m_DirEdit, ok);
	gtk_widget_set_sensitive(m_DirBtn, ok);
	gtk_widget_set_sensitive(m_ProcLabel, ok);
	gtk_widget_set_sensitive(m_FilterCheck, ok);
	gtk_widget_set_sensitive(m_FilterEdit, ok && 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FilterCheck)));
	gtk_widget_set_sensitive(m_NameCheck, ok);
	gtk_widget_set_sensitive(m_NameEdit, ok &&
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_NameCheck)));
	gtk_widget_set_sensitive(m_ObjectCheck, ok);
	gtk_widget_set_sensitive(m_ObjectEdit, ok &&
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ObjectCheck)));
	gtk_widget_set_sensitive(m_OptLabel, ok);
	gtk_widget_set_sensitive(m_BgCheck, ok);
}

void CNewFilesDlg::EditDirPath(void)
{
	const gchar *dirname = gtk_entry_get_text(GTK_ENTRY(m_DirEdit));

	GtkWidget *pPathDlg = gtk_file_chooser_dialog_new("Select path",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CLOSE, 
		GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pPathDlg));
	if (dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pPathDlg), dirname);
	if (gtk_dialog_run(GTK_DIALOG(pPathDlg)) == GTK_RESPONSE_ACCEPT) {
		gchar *path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(pPathDlg));
		CConfig::SetStr("MakeCatDlg", "Folder", path);
		gtk_entry_set_text(GTK_ENTRY(m_DirEdit), path);
		g_free(path);
		UpdateControls();
	}
	gtk_widget_destroy(pPathDlg);
}

bool CNewFilesDlg::Execute(void)
{
	int mode, res;
	char msg[256];
	bool recursive, name_check, object_check, filter_check, background;
	const char *name, *object, *filter, *dirpath;

	// Default state
	mode = g_Project->GetInt("NewFiles", "Mode", 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), !mode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FilesBtn), mode);

	// Go to last folder
	gchar *aux = g_Project->GetStr("AddFiles", "Folder", NULL);
	if (!aux || !g_file_test(aux, G_FILE_TEST_IS_DIR))
		aux = CConfig::GetStr("AddFiles", "Folder", NULL);
	if (aux) 
		gtk_entry_set_text(GTK_ENTRY(m_DirEdit), aux);
	else
		gtk_entry_set_text(GTK_ENTRY(m_DirEdit), "");
	g_free(aux);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SubDirs), 
		g_Project->GetBool("NewFiles", "SubDirs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_NameCheck), 
		g_Project->GetBool("NewFiles", "Name"));
	aux = g_Project->GetStr("NewFiles", "NameText", "");
	if (aux) 
		gtk_entry_set_text(GTK_ENTRY(m_NameEdit), aux);
	else
		gtk_entry_set_text(GTK_ENTRY(m_NameEdit), "");
	g_free(aux);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FilterCheck), 
		g_Project->GetBool("NewFiles", "Filter"));
	aux = g_Project->GetStr("NewFiles", "FilterText", "");
	if (aux) 
		gtk_entry_set_text(GTK_ENTRY(m_FilterEdit), aux);
	else
		gtk_entry_set_text(GTK_ENTRY(m_FilterEdit), "");
	g_free(aux);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ObjectCheck), 
		g_Project->GetBool("NewFiles", "Object"));
	aux = g_Project->GetStr("NewFiles", "ObjectText", "");
	if (aux) 
		gtk_entry_set_text(GTK_ENTRY(m_ObjectEdit), aux);
	else
		gtk_entry_set_text(GTK_ENTRY(m_ObjectEdit), "");
	g_free(aux);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_BgCheck), 
		g_Project->GetBool("NewFiles", "Background"));

	m_InFiles = m_OutFiles = 0;
	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	// Save parameters
	mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FilesBtn));
	g_Project->SetInt("NewFiles", "Mode", (!mode ? 0 : 1));
	dirpath = gtk_entry_get_text(GTK_ENTRY(m_DirEdit));
	g_Project->SetStr("AddFiles", "Folder", dirpath);
	CConfig::SetStr("AddFiles", "Folder", dirpath);
	recursive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SubDirs))!=0;
	g_Project->SetBool("NewFiles", "SubDirs", recursive);
	name_check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_NameCheck))!=0;
	g_Project->SetBool("NewFiles", "Name", name_check);
	name = gtk_entry_get_text(GTK_ENTRY(m_NameEdit));
	g_Project->SetStr("NewFiles", "NameText", name);
	filter_check  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FilterCheck))!=0;
	g_Project->SetBool("NewFiles", "Filter", filter_check);
	filter = gtk_entry_get_text(GTK_ENTRY(m_FilterEdit));
	g_Project->SetStr("NewFiles", "FilterText", filter);
	object_check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ObjectCheck))!=0;
	g_Project->SetBool("NewFiles", "Object", object_check);
	object = gtk_entry_get_text(GTK_ENTRY(m_ObjectEdit));
	g_Project->SetStr("NewFiles", "ObjectText", object);
	background = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BgCheck))!=0;
	g_Project->SetBool("NewFiles", "Background", background);

	// Background process?
	if (background) {
		CNewFilesBox *pBox = g_MainWnd->NewFilesBox();
		if (pBox)
			pBox->Show(true);
		return true;
	}

	// Make list of files
	m_FrameList = NULL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))) {
		// Make list of unprocessed frames
		gtk_tree_model_foreach(g_Project->FileList(), foreach_all_files, &m_FrameList);
		if (!m_FrameList) {
			ShowError(m_pParent, "There are no unprocessed frames in the project.");	
			return false;
		}
		CProgressDlg pDlg(m_pParent, "Processing frames");
		pDlg.SetMinMax(0, g_list_length(m_FrameList));
		res = pDlg.Execute(FramesProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else if (m_OutFiles==0) {
			ShowError(m_pParent, "No frame was successfully processed.", true);
		} else if (m_OutFiles!=m_InFiles) {
			sprintf(msg, "%d frame(s) was successfully processed, %d frame(s) failed.", m_OutFiles, m_InFiles-m_OutFiles);
			ShowWarning(m_pParent, msg, true);
		} else {
			sprintf(msg, "All %d frame(s) was successfully processed.", m_OutFiles);
			ShowInformation(m_pParent, msg, true);
		}
		g_list_foreach(m_FrameList, (GFunc)gtk_tree_row_reference_free, NULL);
		g_list_free(m_FrameList);
		m_FrameList = NULL;
	} else {
		// Add new files to the project
		CProgressDlg pDlg(m_pParent, "Processing files");
		res = pDlg.Execute(FilesProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else if (m_InFiles==0) {
			ShowError(m_pParent, "There are no unprocessed files that fit the specified conditions.");
		} else if (m_OutFiles==0) {
			ShowError(m_pParent, "No file was successfully processed.", true);
		} else if (m_OutFiles!=m_InFiles) {
			sprintf(msg, "%d new file(s) were successfully processed, %d file(s) failed.", m_OutFiles, m_InFiles-m_OutFiles);
			ShowWarning(m_pParent, msg, true);
		} else {
			sprintf(msg, "All %d file(s) were successfully processed.", m_OutFiles);
			ShowInformation(m_pParent, msg, true);
		}
	}
	g_Project->Save();
	return true;
}

int CNewFilesDlg::FramesProc(CProgressDlg *sender, void *userdata)
{
	return ((CNewFilesDlg*)userdata)->ProcessFrames(sender);
}

int CNewFilesDlg::ProcessFrames(CProgressDlg *sender)
{
	char *fpath, *fbase, *tpath;
	CUpdateProc proc;
	GList *node;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Process new frames ------");

	int res = proc.Init(sender);
	if (res==0) {
		for (node=m_FrameList; node!=NULL; node=node->next) {
			GtkTreePath *path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				int state = g_Project->GetState(path);
				if ((state & CFILE_CONVERSION)==0) {
					fpath = g_Project->GetSourceFile(path);
					fbase = g_filename_display_basename(fpath);
					sender->SetFileName(fbase);
					g_free(fbase);
					g_free(fpath);
				} else {
					tpath = g_Project->GetImageFileName(path);
					sender->SetFileName(tpath);
					g_free(tpath);
				}
				sender->SetProgress(m_InFiles++);
				if (proc.Execute(path)==0)
					m_OutFiles++;
				gtk_tree_path_free(path);
			}
			if (sender->Cancelled()) 
				break;
		}
	}
	return res;
}

int CNewFilesDlg::FilesProc(CProgressDlg *sender, void *userdata)
{
	return ((CNewFilesDlg*)userdata)->ProcessFiles(sender);
}

int CNewFilesDlg::ProcessFiles(CProgressDlg *sender)
{
	GSList *files, *node;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Process new frames ------");

	CNewFilesChecker checker;
	if (!checker.Init()) {
		sender->Print("The directory does not exist.");
		return 0;
	}

	sender->SetTitle("Checking for new files");
	files = NULL;
	checker.Check(NewFileCB, &files);
	if (!files) {
		// No new files
		return 0;
	}

	CUpdateProc proc;
	int res = proc.Init(sender);
	if (res!=0) {
		// Initialization failed
		g_slist_foreach(files, (GFunc)FileFree, 0);
		g_slist_free(files);
		return res;
	}

	// Sort frames by observation time
	files = g_slist_sort(files, (GCompareFunc)CompareByTime);

	sender->SetTitle("Processing files");
	sender->SetFileName(NULL);
	sender->SetMinMax(0, g_slist_length(files));
	sender->SetProgress(0);
	g_Project->Lock();
	for (node=files; node!=NULL; node = node->next) {
		CFrameInfo *info = (CFrameInfo*)node->data;
		if (!g_Project->FileInProject(info->FullPath())) {
			GtkTreePath *path = g_Project->AddFile(*info);
			if (path) {
				g_Project->Unlock();
				gchar *basename = g_path_get_basename(info->FullPath());
				sender->SetFileName(basename);
				sender->SetProgress(m_InFiles++);
				if (proc.Execute(path)==0)
					m_OutFiles++;
				g_Project->Lock();
				g_free(basename);
				gtk_tree_path_free(path);
			}
		}
		if (sender->Cancelled()) 
			break;
	}
	g_Project->Unlock();

	g_slist_foreach(files, (GFunc)FileFree, 0);
	g_slist_free(files);
	return 0;
}

bool CNewFilesDlg::NewFileCB(const CFrameInfo &frame, gpointer data, bool *try_again)
{
	*((GSList**)data) = g_slist_prepend(*((GSList**)data), new CFrameInfo(frame));
	return true;
}

//-----------------------------   BACGROUND PROCESS   -----------------------------------

//
// Console class
//
class CNFConsole:public CConsole
{
public:
	CNFConsole(CNewFilesBox *parent):m_Parent(parent) {}
protected:
	virtual void OnPrint(const char *message)
	{ m_Parent->Print(message); }
private:
	CNewFilesBox *m_Parent;
};

//
// Constructor
//
CNewFilesBox::CNewFilesBox(void):CInfoBox("Process new frames"), m_Thread(NULL), 
	m_StopThread(false), m_SuspendRq(false), m_Delay(false), m_OutFiles(0), 
	m_ExitCode(0), m_State(STATE_STOP), m_Proc(NULL)
{
	GtkWidget *hbox = gtk_hbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(m_Box), hbox, FALSE, TRUE, 0);

	// Two text lines
	GtkWidget *vbox = gtk_vbox_new(TRUE, 8);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
	m_Line1 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Line1), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Line1, TRUE, TRUE, 0);
	m_Line2 = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Line2), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Line2, TRUE, TRUE, 0);

	// Button box
	GtkWidget *bbox = gtk_vbutton_box_new();
	gtk_box_pack_start(GTK_BOX(hbox), bbox, FALSE, TRUE, 0);
	m_CancelBtn = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(bbox), m_CancelBtn, FALSE, TRUE, 0);
	g_signal_connect(m_CancelBtn, "clicked", G_CALLBACK(cancel_clicked), this);
	m_PauseBtn = gtk_button_new_with_label("Pause");
	gtk_box_pack_start(GTK_BOX(bbox), m_PauseBtn, FALSE, TRUE, 0);
	g_signal_connect(m_PauseBtn, "clicked", G_CALLBACK(pause_clicked), this);

	m_Con = new CNFConsole(this);
	m_Queue = g_async_queue_new();
	m_DataMutex = g_mutex_new();
	m_Cond = g_cond_new();
}

//
// Destructor
//
CNewFilesBox::~CNewFilesBox(void)
{
	g_mutex_lock(m_DataMutex);
	if (m_Thread) {
		m_StopThread = true;
		g_cond_signal(m_Cond);
		g_mutex_unlock(m_DataMutex);
		gdk_threads_unlock();
		g_thread_join(m_Thread);
		gdk_threads_lock();
		g_mutex_lock(m_DataMutex);
		m_Thread = NULL;
	}
	g_mutex_unlock(m_DataMutex);
	delete m_Proc;
	delete m_Con;
	PurgeQueue();
	g_async_queue_unref(m_Queue);
	g_mutex_free(m_DataMutex);
	g_cond_free(m_Cond);
}

//
// Start process
//
void CNewFilesBox::OnShow(void)
{
	if (m_Thread)
		return;

	Clear();
	CMessagesDlg::InitBuffer();
	if (m_Checker.Init()) {
		m_State = STATE_WAIT;
		m_StopThread = m_SuspendRq = false;
		m_Thread = g_thread_create(GThreadFunc(thread_proc), this, TRUE, NULL);
		gtk_widget_set_sensitive(m_PauseBtn, TRUE);
	} else {
		gtk_label_set_text(GTK_LABEL(m_Line2), "The directory does not exist.");
		m_State = STATE_STOP;
	}
	PushMessage(EVENT_UPDATE, 0);
}

//
// Stop process
//
void CNewFilesBox::OnHide(void)
{
	g_mutex_lock(m_DataMutex);
	if (m_Thread && !m_StopThread) {
		m_StopThread = true;
		g_cond_signal(m_Cond);
		g_mutex_unlock(m_DataMutex);
		gdk_threads_unlock();
		g_thread_join(m_Thread);
		gdk_threads_lock();
		g_mutex_lock(m_DataMutex);
		m_Thread = NULL;
	}
	g_mutex_unlock(m_DataMutex);
	Clear();
}

void CNewFilesBox::Clear(void)
{
	m_OutFiles = m_ExitCode = 0;
	m_Checker.Clear();
	delete m_Proc;
	m_Proc = NULL;
	PurgeQueue();
	gtk_label_set_text(GTK_LABEL(m_Line1), "");
	gtk_label_set_text(GTK_LABEL(m_Line2), "");
}

void CNewFilesBox::Print(const char *text)
{
	int textlen;
	char *buffer;

	textlen = strlen(text);
	buffer = (char*)g_malloc(textlen+2);
	memcpy(buffer, text, textlen);
	buffer[textlen] = '\n';
	buffer[textlen+1] = '\0';
	PushMessage(EVENT_MESSAGE, buffer);
}

// Process is running
bool CNewFilesBox::IsRunning(void)
{
	bool res;
	g_mutex_lock(m_DataMutex);
	res = m_State!=STATE_STOP;
	g_mutex_unlock(m_DataMutex);
	return res;
}

void CNewFilesBox::PushMessage(tEventCode event, gchar *text)
{
	tMessage *msg = (tMessage*)g_malloc(sizeof(tMessage));
	msg->event = event;
	msg->text = text;
	g_async_queue_push(m_Queue, msg);
	g_idle_add(GSourceFunc(idle_func), this);
}

CNewFilesBox::tMessage *CNewFilesBox::PopMessage()
{
	return (tMessage*)g_async_queue_try_pop(m_Queue);
}

// Process all message in queue
void CNewFilesBox::PurgeQueue(void)
{
	tMessage *msg = (tMessage*)g_async_queue_try_pop(m_Queue);
	while (msg) {
		if (msg->event == EVENT_MESSAGE)
			CMessagesDlg::LogMessage(msg->text);
		g_free(msg->text);
		g_free(msg);
		msg = (tMessage*)g_async_queue_try_pop(m_Queue);
	}
}

gboolean CNewFilesBox::idle_func(CNewFilesBox *pMe)
{
	tMessage *msg = pMe->PopMessage();
	if (msg) {
		switch (msg->event) 
		{
		case EVENT_MESSAGE:
			gdk_threads_enter();
			CMessagesDlg::LogMessage(msg->text);
			gdk_threads_leave();
			break;
		case EVENT_UPDATE:
			gdk_threads_enter();
			pMe->OnUpdate();
			gdk_threads_leave();
			break;
		}
		g_free(msg->text);
		g_free(msg);
	}
	return FALSE;
}

void CNewFilesBox::OnUpdate(void)
{
	tStateCode state;
	int outfiles;
	gchar buf[256];

	g_mutex_lock(m_DataMutex);
	state = m_State;
	outfiles = m_OutFiles;
	g_mutex_unlock(m_DataMutex);

	switch (state)
	{
	case STATE_WAIT:
		gtk_label_set_text(GTK_LABEL(m_Line1), "Waiting for new files");
		gtk_button_set_label(GTK_BUTTON(m_PauseBtn), "Pause");
		break;
	case STATE_WORK:
		gtk_label_set_text(GTK_LABEL(m_Line1), "Processing files");
		break;
	case STATE_SUSPENDED:
		gtk_label_set_text(GTK_LABEL(m_Line1), "Paused");
		gtk_button_set_label(GTK_BUTTON(m_PauseBtn), "Resume");
		break;
	case STATE_STOP:
		gtk_label_set_text(GTK_LABEL(m_Line1), "Stopped");
		gtk_widget_set_sensitive(m_PauseBtn, FALSE);
		break;
	}

	if (m_ExitCode!=0) {
		char *msg = cmpack_formaterror(m_ExitCode);
		gtk_label_set_text(GTK_LABEL(m_Line2), buf);
		cmpack_free(msg);
	} else if (outfiles>0) {
		sprintf(buf, "Processed %d files", outfiles);
		gtk_label_set_text(GTK_LABEL(m_Line2), buf);
	}
}

void CNewFilesBox::thread_proc(CNewFilesBox *pMe)
{
	pMe->OnThreadProc();
}

void CNewFilesBox::OnThreadProc(void)
{
	GTimeVal time;

	g_mutex_lock(m_DataMutex);
	while (!Cancelled()) {
		m_Delay = true;
		g_mutex_unlock(m_DataMutex);
		m_Checker.Check(NewFrameCB, this);
		g_mutex_lock(m_DataMutex);
		if (m_Delay && !m_StopThread) {
			if (m_State!=STATE_WAIT) {
				m_State = STATE_WAIT;
				PushMessage(EVENT_UPDATE, 0);
			}
			g_get_current_time(&time);
			g_time_val_add(&time, 3000000);
			g_cond_timed_wait(m_Cond, m_DataMutex, &time);
		}
	}
	m_State = STATE_STOP;
	PushMessage(EVENT_UPDATE, 0);
	g_mutex_unlock(m_DataMutex);
}

bool CNewFilesBox::NewFrameCB(const CFrameInfo &frame, gpointer data, bool *try_again)
{
	return ((CNewFilesBox*)data)->OnNewFrame(frame, try_again);
}

bool CNewFilesBox::OnNewFrame(const CFrameInfo &frame, bool *try_again)
{
	int res;
	bool retval;

	// Initialization
	if (!m_Proc) {
		m_Proc = new CUpdateProc();
		if (m_Proc->Init(m_Con)!=0) {
			delete m_Proc;
			m_Proc = NULL;
			m_StopThread = TRUE;
		}
		if (!m_Proc)
			return false;
	}

	// Process file
	g_mutex_lock(m_DataMutex);
	m_State = STATE_WORK;
	PushMessage(EVENT_UPDATE, 0);
	g_mutex_unlock(m_DataMutex);
	g_Project->Lock();
	if (!g_Project->FileInProject(frame.FullPath())) {
		GtkTreePath *path = g_Project->AddFile(frame);
		if (path) {
			g_Project->Unlock();
			res = m_Proc->Execute(path);
			if (res==CMPACK_ERR_CANT_OPEN_SRC)
				*try_again = TRUE;
			if (res==0) {
				m_Delay = false;
				m_OutFiles++;
			}
			g_Project->Lock();
			gtk_tree_path_free(path);
		}
	}
	g_Project->Unlock();
	g_mutex_lock(m_DataMutex);
	retval = !Cancelled();
	g_mutex_unlock(m_DataMutex);
	return retval;
}

bool CNewFilesBox::Cancelled()
{
	if (m_SuspendRq) {
		// We can suspend the process between frames
		tStateCode old_state = m_State;
		m_State = STATE_SUSPENDED;
		PushMessage(EVENT_UPDATE, 0);
		g_cond_wait(m_Cond, m_DataMutex);
		m_State = old_state;
		PushMessage(EVENT_UPDATE, 0);
	}
	// Return FALSE to abort the process
	return m_StopThread;
}

// Cancel button clicked
void CNewFilesBox::cancel_clicked(GtkButton *button, CNewFilesBox *pMe)
{
	pMe->OnCancelClicked();
}

// Cancel button clicked
void CNewFilesBox::OnCancelClicked(void)
{
	Show(false);
}

// Cancel button clicked
void CNewFilesBox::pause_clicked(GtkButton *button, CNewFilesBox *pMe)
{
	pMe->OnPauseClicked();
}

// Cancel button clicked
void CNewFilesBox::OnPauseClicked(void)
{
	g_mutex_lock(m_DataMutex);
	if (m_State!=STATE_SUSPENDED) {
		m_SuspendRq = true;
		g_cond_signal(m_Cond);
	} else {
		m_SuspendRq = false;
		g_cond_signal(m_Cond);
	}
	g_mutex_unlock(m_DataMutex);
}
