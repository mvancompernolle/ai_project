/**************************************************************

varfind_dlg.cpp (C-Munipack project)
The 'Find variables' dialog
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

#include "findvariables_dlg.h"
#include "configuration.h"
#include "main.h"
#include "ctxhelp.h"
#include "frameinfo_dlg.h"

enum tMenuId
{
	MENU_FILE = 1,
	MENU_VIEW,
	MENU_HELP
};

enum tCommandId
{
	CMD_CLOSE = 100,
	CMD_PRINT,
	CMD_REBUILD,
	CMD_SAVE_CHART,
	CMD_SAVE_MAGDEV,
	CMD_SAVE_LCURVE,
	CMD_SHOW_CHART,
	CMD_SHOW_IMAGE,
	CMD_SHOW_MIXED,
	CMD_EXPORT_DATA,
	CMD_EXPORT_MAGDEV,
	CMD_EXPORT_LCURVE,
	CMD_HELP
};

static const CMenuBar::tMenuItem FileMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_SAVE_MAGDEV,		"Save _mag-dev curve" },
	{ CMenuBar::MB_ITEM,	CMD_SAVE_CHART,			"Save _chart" },
	{ CMenuBar::MB_ITEM,	CMD_SAVE_LCURVE,		"Save _light curve" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT_DATA,		"Export varfind data" },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT_MAGDEV,		"Export mag-dev curve as image" },
	{ CMenuBar::MB_ITEM,	CMD_EXPORT_LCURVE,		"Export light curve as image" },
	//{ CMenuBar::MB_ITEM,	CMD_PRINT,				"P_rint" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_REBUILD,			"Rebuild" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM,	CMD_CLOSE,				"_Close" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ViewMenu[] = {
	{ CMenuBar::MB_RADIOBTN, CMD_SHOW_IMAGE,		"_Image only" },
	{ CMenuBar::MB_RADIOBTN, CMD_SHOW_CHART,		"_Chart only" },
	{ CMenuBar::MB_RADIOBTN, CMD_SHOW_MIXED,		"Image and chart" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM,	CMD_HELP,				"_Show help", "help" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu PreviewMenu2[] = {
	{ "_File",	MENU_FILE,	FileMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   HELPER FUNCTIONS   --------------------------------

static gboolean foreach_all_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	if (g_Project->GetState(iter) & CFILE_MATCHING) 
		*list = g_slist_append(*list, gtk_tree_row_reference_new(model, path));
	return FALSE;
}

static void foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GSList **list)
{
	if (g_Project->GetState(iter) & CFILE_MATCHING) 
		*list = g_slist_append(*list, gtk_tree_row_reference_new(model, path));
}

//-------------------------   VARFIND DIALOG   --------------------------------

//
// Constructor
//
CVarFindDlg::CVarFindDlg(void):m_FileList(NULL), m_RefType(REF_UNDEFINED)
{
	gchar buf[512];

	// Dialog caption
	sprintf(buf, "%s - %s", "Find variables", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Menu bar
	m_Menu.Create(PreviewMenu2, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), FALSE, FALSE, 0);

	// VarFind box
	m_VarFind.RegisterCallback(VarFindCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_VarFind.Handle(), TRUE, TRUE, 0);
	
	// Show the dialog
	gtk_widget_show_all(m_MainBox);
}

//
// Destructor
//
CVarFindDlg::~CVarFindDlg(void)
{
	g_slist_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
	g_slist_free(m_FileList);
}

//
// Open a new dialog
//
bool CVarFindDlg::Open(GtkWindow *pParent, bool selected_files, CSelectionList *list)
{
	int res = 0;
	GtkTreePath *refpath;

	m_FrameSet.Clear();
	m_Phot.Clear();
	m_Frame.Close();
	m_Image.Clear();
	m_Catalog.Clear();
	g_slist_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
	g_slist_free(m_FileList);
	m_FileList = NULL;

	// Reference file
	m_RefType = g_Project->GetReferenceType();
	switch (m_RefType)
	{
	case REF_FRAME:
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				GError *error = NULL;
				if (m_Phot.Load(pht_file, &error)) {
					m_VarFind.SetPhotometryFile(&m_Phot);
					gchar *fts_file = g_Project->GetImageFile(refpath);
					if (fts_file) {
						CCCDFile frame;
						if (frame.Open(fts_file, CMPACK_OPEN_READONLY, &error)) {
							if (frame.GetImageData(m_Image)) 
								m_VarFind.SetImage(&m_Image);
							m_Frame.MakeCopy(frame);
						}
						g_free(fts_file);
					}
				} 
				if (error) {
					ShowError(pParent, error->message);
					g_error_free(error);
					res = -1;
				}
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		{
			const gchar *tmp_file = g_Project->GetTempCatFile()->FullPath();
			if (tmp_file) {
				GError *error = NULL;
				if (m_Catalog.Load(tmp_file, &error)) {
					m_VarFind.SetCatalogFile(&m_Catalog);
					gchar *fts_file = SetFileExtension(tmp_file, FILE_EXTENSION_FITS);
					CCCDFile frame;
					if (frame.Open(fts_file, CMPACK_OPEN_READONLY, &error)) {
						if (frame.GetImageData(m_Image)) 
							m_VarFind.SetImage(&m_Image);
						m_Frame.MakeCopy(frame);
					}
					g_free(fts_file);
				} 
				if (error) {
					ShowError(pParent, error->message);
					g_error_free(error);
					res = -1;
				}
			}
		}
		break;

	default:
		ShowError(pParent, "No reference file.");
		res = -1;
	}
	if (res!=0)
		return false;

	if (!selected_files) {
		// All files
		gtk_tree_model_foreach(g_Project->FileList(), (GtkTreeModelForeachFunc)foreach_all_files, &m_FileList);
		if (!m_FileList) {
			ShowError(pParent, "There are no files in the project.");
			return false;
		}
	} else {
		// Selected files
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		if (gtk_tree_selection_count_selected_rows(pSel)>0) 
			gtk_tree_selection_selected_foreach(pSel, (GtkTreeSelectionForeachFunc)foreach_sel_files, &m_FileList);
		if (!m_FileList) {
			ShowError(pParent, "There are no selected files.");
			return false;
		}
	}

	m_VarFind.SetApertures(*g_Project->Apertures(), g_Project->GetInt("MagDevCurve", "Aperture", 0));
	m_VarFind.SetSelectionList(list);
	return true;
}

//
// Dialog initialization
//
void CVarFindDlg::OnInitDialog(void)
{
	COutputDlg::OnInitDialog();

	RebuildFrameSet();
}

//
// Menu callback
//
void CVarFindDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CVarFindDlg *pMe = (CVarFindDlg*)cb_data;

	switch (message)
	{
	case CMenuBar::CB_ACTIVATE:
		pMe->OnCommand(wparam);
		break;
	}
}

//
// Command handler
//
void CVarFindDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_SAVE_CHART:
		m_VarFind.ExportChart();
		break;
	case CMD_SAVE_LCURVE:
		m_VarFind.SaveLightCurve();
		break;
	case CMD_SAVE_MAGDEV:
		m_VarFind.SaveMagDevCurve();
		break;
	case CMD_EXPORT_LCURVE:
		m_VarFind.ExportLightCurve();
		break;
	case CMD_EXPORT_MAGDEV:
		m_VarFind.ExportMagDevCurve();
		break;
	case CMD_EXPORT_DATA:
		ExportData();
		break;
	case CMD_CLOSE:
		Close();
		break;
	case CMD_REBUILD:
		UpdateFileList();
		RebuildFrameSet();
		break;

	// View menu
	case CMD_SHOW_CHART:
		m_VarFind.SetDisplayMode(CVarFind::DISPLAY_CHART);
		UpdateControls();
		break;
	case CMD_SHOW_IMAGE:
		m_VarFind.SetDisplayMode(CVarFind::DISPLAY_IMAGE);
		UpdateControls();
		break;
	case CMD_SHOW_MIXED:
		m_VarFind.SetDisplayMode(CVarFind::DISPLAY_MIXED);
		UpdateControls();
		break;
		
	// Help menu
	case CMD_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_FIND_VARIABLES);
		break;
	}
}

//
// Update frame list
//
void CVarFindDlg::UpdateFileList(void)
{
	g_slist_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
	g_slist_free(m_FileList);
	m_FileList = NULL;

	g_Project->Lock();
	GtkTreeModel *frames = g_Project->FileList();
	gtk_tree_model_foreach(frames, (GtkTreeModelForeachFunc)foreach_all_files, &m_FileList);
	g_Project->Unlock();
}

//
// VarFind callback
//
void CVarFindDlg::VarFindCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CVarFindDlg *pMe = (CVarFindDlg*)cb_data;

	switch (message)
	{
	case CVarFind::CB_APERTURE_CHANGED:
		pMe->RebuildFrameSet();
		break;

	case CVarFind::CB_COMPARISON_CHANGED:
		pMe->SaveSelection();
		pMe->UpdateMagDev();
		pMe->UpdateLightCurve();
		pMe->UpdateControls();
		break;

	case CVarFind::CB_VARIABLE_CHANGED:
		pMe->SaveSelection();
		pMe->UpdateLightCurve();
		pMe->UpdateControls();
		break;

	case CVarFind::CB_ENABLE_CTRL_QUERY:
		*((bool*)lparam) = pMe->OnEnableCtrlQuery((CVarFind::tControlId)wparam);
		break;

	case CVarFind::CB_OBJECT_VALID_QUERY:
		*((bool*)lparam) = pMe->m_FrameSet.FindObject(wparam)>=0;
		break;

	case CVarFind::CB_REMOVE_FRAMES_FROM_DATASET:
		pMe->RemoveFramesFromDataSet();
		break;
	case CVarFind::CB_DELETE_FRAMES_FROM_PROJECT:
		pMe->DeleteFramesFromProject();
		break;
	case CVarFind::CB_SHOW_FRAME_INFO:
		pMe->ShowFrameInfo();
		break;
	case CVarFind::CB_SHOW_FRAME_PREVIEW:
		pMe->ShowFramePreview();
		break;
	case CVarFind::CB_REMOVE_OBJECTS_FROM_DATASET:
		pMe->RemoveObjectsFromDataSet();
		break;
	}
}

//
// Aperture change, rebuild data
//
void CVarFindDlg::RebuildFrameSet(void)
{
	int res, apertureId = m_VarFind.ApertureId();

	m_FrameSet.Clear();
	if (apertureId>=0) {
		if (m_RefType==REF_FRAME)
			m_FrameSet.Init(apertureId, m_Phot);
		else
			m_FrameSet.Init(apertureId, m_Catalog);
		g_Project->SetInt("LightCurve", "Aperture", apertureId);

		CProgressDlg pDlg(GTK_WINDOW(m_pDlg), "Processing files");
		pDlg.SetMinMax(0, g_slist_length(m_FileList));
		res = pDlg.Execute(ExecuteProc, this);
		if (res!=0 && !pDlg.Cancelled()) {
			char *msg = cmpack_formaterror(res);
			ShowError(GTK_WINDOW(m_pDlg), msg);
			cmpack_free(msg);
		}
	}

	m_VarFind.UpdateChart();
	UpdateMagDev();
	UpdateLightCurve();
	UpdateControls();
}

//
// Comparison changed
//
void CVarFindDlg::UpdateMagDev(void)
{
	int refstar;
	double jdmin, jdmax, magrange;
	CMFindProc mfind;

	m_VarFind.SetMagDev(CTable());

	int apertureId = m_VarFind.ApertureId();
	if (apertureId>=0) {
		refstar = m_VarFind.ComparisonID();
		if (refstar<0)
			refstar = CMPACK_MFIND_AUTO;
		if (mfind.Init(NULL, apertureId, refstar)==0) {
			CTable *magdev;
			if (mfind.Execute(m_FrameSet, &magdev, &refstar, &jdmin, &jdmax, &magrange)==0) {
				if (refstar!=m_VarFind.ComparisonID())
					m_VarFind.SetComparisonStar(refstar);
				m_VarFind.SetFixedJDRange(jdmin, jdmax);
				m_VarFind.SetFixedMagRange(magrange);
				m_VarFind.SetMagDev(*magdev);
				delete magdev;
			}
		}
	}	
}

//
// Variable changed
//
void CVarFindDlg::UpdateLightCurve(void)
{
	int varstar = m_VarFind.VariableID(), compstar = m_VarFind.ComparisonID();
	int apertureId = m_VarFind.ApertureId();

	m_VarFind.SetLightCurve(CTable());
	if (varstar>=0 && compstar>=0 && apertureId>=0) {
		CSelection sel;
		sel.Select(varstar, CMPACK_SELECT_VAR);
		sel.Select(compstar, CMPACK_SELECT_COMP);
		CTable *lcurve = NULL;
		if (CmpackLightCurve(NULL, &lcurve, sel, m_FrameSet, apertureId, CMPACK_LCURVE_FRAME_IDS)==0) {
			m_VarFind.SetLightCurve(*lcurve);
			delete lcurve;
		}
	}
}

//
// Enable/disable controls
//
void CVarFindDlg::UpdateControls(void)
{
	int varstar = m_VarFind.VariableID(), compstar = m_VarFind.ComparisonID(), 
		aperture = m_VarFind.ApertureIndex();

	m_Menu.Enable(CMD_SHOW_IMAGE, m_Frame.Valid());
	m_Menu.Enable(CMD_SHOW_MIXED, m_Frame.Valid());
	m_Menu.Check(CMD_SHOW_IMAGE, m_VarFind.DisplayMode()==CVarFind::DISPLAY_IMAGE);
	m_Menu.Check(CMD_SHOW_CHART, m_VarFind.DisplayMode()==CVarFind::DISPLAY_CHART);
	m_Menu.Check(CMD_SHOW_MIXED, m_VarFind.DisplayMode()==CVarFind::DISPLAY_MIXED);
	m_Menu.Enable(CMD_EXPORT_DATA, aperture>=0);
	m_Menu.Enable(CMD_SAVE_MAGDEV, compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_SAVE_LCURVE, varstar>=0 && compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_EXPORT_MAGDEV, compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_EXPORT_LCURVE, varstar>=0 && compstar>=0 && aperture>=0);
}

//
// Is this control enabled?
//
bool CVarFindDlg::OnEnableCtrlQuery(CVarFind::tControlId ctrl)
{
	switch (ctrl)
	{
	case CVarFind::ID_SAVE_CHART:
		return (m_RefType==REF_FRAME ? m_Phot.Valid() : m_Catalog.Valid());
	case CVarFind::ID_SHOW_FRAME_PREVIEW:
	case CVarFind::ID_SHOW_FRAME_INFO:
	case CVarFind::ID_REMOVE_FROM_DATASET:
	case CVarFind::ID_DELETE_FROM_PROJECT:
		return true;
	default:
		return false;
	}
}

int CVarFindDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CVarFindDlg*)userdata)->OnProcessFiles(sender);
}

int CVarFindDlg::OnProcessFiles(CProgressDlg *sender)
{
	int res, infiles;
	GtkTreePath *path;
	CListProc list;

	// Make listing context
	res = list.Init(sender, &m_FrameSet, false);
	if (res==0) {
		// Process files
		infiles = 0;
		for (GSList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
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

void CVarFindDlg::ExportData(void)
{
	GtkFileFilter *filters[2];

	GtkWidget *pSaveDlg = gtk_file_chooser_dialog_new("Export varfind data",
		GTK_WINDOW(m_pDlg), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(pSaveDlg));
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(pSaveDlg), true);

	// File filters
	filters[0] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[0], "*.dat");
	gtk_file_filter_set_name(filters[0], "C-Munipack varfind files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);
	filters[1] = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filters[1], "*");
	gtk_file_filter_set_name(filters[1], "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[1]);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(pSaveDlg), filters[0]);

	// Restore last folder and file name
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pSaveDlg), folder);
	g_free(folder);

	gchar *filename = g_Project->GetStr("VarFind", "FileName", "varfind.dat");
	if (filename) 
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pSaveDlg), filename);
	g_free(filename);

	if (gtk_dialog_run(GTK_DIALOG(pSaveDlg))!=GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(pSaveDlg);
		return;
	}
	gtk_widget_hide(pSaveDlg);

	gchar *fpath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pSaveDlg));

	// Save last folder and file name
	gchar *dirpath = g_path_get_dirname(fpath);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);

	gchar *basename = g_path_get_basename(fpath);
	g_Project->SetStr("VarFind", "FileName", basename);
	g_free(basename);
	
	// Export data in separate thread
	int res = 0;
	GError *error = NULL;
	if (m_FrameSet.Save(fpath, 0, &error)) {
		if (m_RefType==REF_FRAME) {
			// Save reference CCD image
			gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_CATALOG);
			CCatalog file;
			if (!file.Create(m_Phot, &error) || !file.SaveAs(dst_path, &error))
				res = -1;
			g_free(dst_path);
		} else {
			// Copy catalog file
			gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_CATALOG);
			if (!m_Catalog.SaveAs(dst_path, &error))
				res = -1;
			g_free(dst_path);
		}
		if (res==0 && m_Frame.Valid()) {
			// Save CCD frame 
			gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_FITS);
			if (!m_Frame.SaveAs(dst_path, &error))
				res = -1;
			g_free(dst_path);
		}
	}
	if (error) {
		ShowError(GTK_WINDOW(m_pDlg), error->message, true);
		g_error_free(error);
	}
	g_free(fpath);
	gtk_widget_destroy(pSaveDlg);
}

void CVarFindDlg::RemoveFramesFromDataSet(void)
{
	bool changed = false;

	GList *rows = m_VarFind.GetSelectedFrames();
	if (rows) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			int frame = (gint)cmpack_graph_data_get_param(m_VarFind.LightCurveData(), (intptr_t)ptr->data);
			m_FrameSet.DeleteFrame(frame);
			changed = true;
		}
		g_list_free(rows);
	}
	if (changed) {
		UpdateMagDev();
		UpdateLightCurve();
		UpdateControls();
	}
}

void CVarFindDlg::RemoveObjectsFromDataSet(void)
{
	bool changed = false;

	GList *rows = m_VarFind.GetSelectedObjects();
	if (rows) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			int object = (gint)cmpack_graph_data_get_param(m_VarFind.MagDevCurveData(), (intptr_t)ptr->data);
			m_FrameSet.DeleteObject(object);
			changed = true;
		}
		g_list_free(rows);
	}
	if (changed) {
		m_VarFind.UpdateChart();
		UpdateMagDev();
		UpdateLightCurve();
		UpdateControls();
	}
}

void CVarFindDlg::DeleteFramesFromProject(void)
{
	bool ok, selected_ref;
	int frame, count;

	GList *rows = m_VarFind.GetSelectedFrames();
	if (rows) {
		count = g_list_length(rows);
		selected_ref = false;
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			frame = (gint)cmpack_graph_data_get_param(m_VarFind.LightCurveData(), (intptr_t)ptr->data);
			if (frame == g_Project->GetReferenceFrame()) {
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
				frame = (gint)cmpack_graph_data_get_param(m_VarFind.LightCurveData(), (intptr_t)ptr->data);
				m_FrameSet.DeleteFrame(frame);
				g_Project->RemoveFrame(frame);
			}
			UpdateMagDev();
			UpdateLightCurve();
			UpdateControls();
		}
		g_list_free(rows);
	}
}

void CVarFindDlg::ShowFramePreview(void)
{
	int frame = m_VarFind.SelectedFrameID();
	if (frame>=0) {
		GtkTreePath *path = g_Project->GetFilePath(frame);
		if (path) {
			g_MainWnd->ShowFramePreview(path);
			gtk_tree_path_free(path);
		}
	}
}

void CVarFindDlg::ShowFrameInfo(void)
{
	int frame = m_VarFind.SelectedFrameID();
	if (frame>=0) {
		GtkTreePath *path = g_Project->GetFilePath(frame);
		if (path) {
			CFrameInfoDlg dlg(GTK_WINDOW(m_pDlg));
			dlg.Show(path);
			gtk_tree_path_free(path);
		}
	}
}

//
// Set selection to the project
//
void CVarFindDlg::SaveSelection(void)
{
	CSelection sel;
	int comp = m_VarFind.ComparisonID(), var = m_VarFind.VariableID();
	if (comp>=0 && var>=0) {
		sel.Select(comp, CMPACK_SELECT_COMP);
		sel.Select(var, CMPACK_SELECT_VAR);
		g_Project->SetLastSelection(sel);
	}
}

