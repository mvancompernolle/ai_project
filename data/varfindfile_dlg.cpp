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

#include "varfindfile_dlg.h"
#include "configuration.h"
#include "main.h"
#include "ctxhelp.h"

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
	CMD_SHOW_PROPERTIES,
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
	{ CMenuBar::MB_ITEM,	CMD_SHOW_PROPERTIES,	"Show _Properties" },
	{ CMenuBar::MB_ITEM,	CMD_REBUILD,			"Reload" },
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

static const CMenuBar::tMenu PreviewMenu1[] = {
	{ "_File",	MENU_FILE,	FileMenu },
	{ "_View",	MENU_VIEW,	ViewMenu },
	{ "_Help",	MENU_HELP,	HelpMenu },
	{ NULL }
};

//-------------------------   VARFIND DIALOG   --------------------------------

//
// Constructor
//
CVarFindFileDlg::CVarFindFileDlg(void):m_PhotometryFile(false)
{
	// Menu bar
	m_Menu.Create(PreviewMenu1, false);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_Menu.Handle(), FALSE, FALSE, 0);

	// VarFind box
	m_VarFind.RegisterCallback(VarFindCallback, this);
	gtk_box_pack_start(GTK_BOX(m_MainBox), m_VarFind.Handle(), TRUE, TRUE, 0);

	// Show the dialog
	gtk_widget_show_all(m_MainBox);
}

//
// Load catalog file
//
bool CVarFindFileDlg::LoadFile(const char *fpath, GError **error)
{
	char *fts_file, *pht_file;
	
	m_Phot.Clear();
	m_Frame.Close();
	m_Catalog.Clear();
	m_Image.Clear();
	m_SelectionList.Clear();
	m_VarFind.SetSelectionList(&m_SelectionList);

	if (!m_File.Load(fpath, error)) 
		return false;

	bool ok = false;
	m_VarFind.SetApertures(*m_File.Apertures(), 1);
	pht_file = SetFileExtension(fpath, FILE_EXTENSION_CATALOG);
	if (m_Catalog.Load(pht_file)) {
		m_PhotometryFile = false;
		m_VarFind.SetCatalogFile(&m_Catalog);
		ok = true;
	}
	g_free(pht_file);
	if (!ok) {
		pht_file = SetFileExtension(fpath, "mat");
		if (m_Phot.Load(pht_file)) {
			m_VarFind.SetPhotometryFile(&m_Phot);
			m_PhotometryFile = true;
			ok = true;
		}
		g_free(pht_file);
	}
	if (!ok) {
		pht_file = SetFileExtension(fpath, FILE_EXTENSION_PHOTOMETRY);
		if (m_Phot.Load(pht_file)) {
			m_VarFind.SetPhotometryFile(&m_Phot);
			m_PhotometryFile = true;
			ok = true;
		}
		g_free(pht_file);
	}
	if (!ok)
		return false;

	fts_file = SetFileExtension(fpath, FILE_EXTENSION_FITS);
	CCCDFile frame;
	if (frame.Open(fts_file, CMPACK_OPEN_READONLY)) {
		if (frame.GetImageData(m_Image))
			m_VarFind.SetImage(&m_Image);
		m_Frame.MakeCopy(frame);
	}
	g_free(fts_file);
	UpdateMagDev();
	UpdateControls();
	return true;
}


//
// Reload data from a file
//
void CVarFindFileDlg::ReloadFrameSet(void)
{
	if (m_File.Load(m_Path)) {
		m_VarFind.SetApertures(*m_File.Apertures(), 1);
		m_VarFind.UpdateChart();
		UpdateMagDev();
		UpdateLightCurve();
		UpdateControls();
	}
}


//
// Menu callback
//
void CVarFindFileDlg::MenuCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CVarFindFileDlg *pMe = (CVarFindFileDlg*)cb_data;

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
void CVarFindFileDlg::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	// File menu
	case CMD_SHOW_PROPERTIES:
		ShowProperties();
		break;
	case CMD_REBUILD:
		ReloadFrameSet();
		break;
	case CMD_SAVE_CHART:
		m_VarFind.ExportChart();
		break;
	case CMD_SAVE_LCURVE:
		m_VarFind.SaveLightCurve();
		break;
	case CMD_EXPORT_LCURVE:
		m_VarFind.ExportLightCurve();
		break;
	case CMD_SAVE_MAGDEV:
		m_VarFind.SaveMagDevCurve();
		break;
	case CMD_EXPORT_MAGDEV:
		m_VarFind.ExportMagDevCurve();
		break;
	case CMD_EXPORT_DATA:
		SaveAs("C-Munipack varfind files", "*.dat");
		break;
	case CMD_CLOSE:
		Close();
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
// VarFind callback
//
void CVarFindFileDlg::VarFindCallback(CCBObject *sender, int message, int wparam, void *lparam, void *cb_data)
{
	CVarFindFileDlg *pMe = (CVarFindFileDlg*)cb_data;

	switch (message)
	{
	case CVarFind::CB_COMPARISON_CHANGED:
		pMe->UpdateMagDev();
		pMe->UpdateLightCurve();
		pMe->UpdateControls();
		break;

	case CVarFind::CB_VARIABLE_CHANGED:
		pMe->UpdateLightCurve();
		pMe->UpdateControls();
		break;

	case CVarFind::CB_ENABLE_CTRL_QUERY:
		*((bool*)lparam) = pMe->OnEnableCtrlQuery((CVarFind::tControlId)wparam);
		break;

	case CVarFind::CB_OBJECT_VALID_QUERY:
		*((bool*)lparam) = pMe->m_File.FindObject(wparam)>=0;
		break;

	case CVarFind::CB_REMOVE_FRAMES_FROM_DATASET:
		pMe->RemoveFramesFromDataSet();
		break;
	case CVarFind::CB_REMOVE_OBJECTS_FROM_DATASET:
		pMe->RemoveObjectsFromDataSet();
		break;
	}
}

//
// Update mag-dev curve
//
void CVarFindFileDlg::UpdateMagDev(void)
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
			CTable *magdev = NULL;
			if (mfind.Execute(m_File, &magdev, &refstar, &jdmin, &jdmax, &magrange)==0) {
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
void CVarFindFileDlg::UpdateLightCurve(void)
{
	int varstar = m_VarFind.VariableID(), compstar = m_VarFind.ComparisonID();
	int apertureId = m_VarFind.ApertureId();

	m_VarFind.SetLightCurve(CTable());

	if (varstar>=0 && compstar>=0 && apertureId>=0) {
		CSelection sel;
		sel.Select(varstar, CMPACK_SELECT_VAR);
		sel.Select(compstar, CMPACK_SELECT_COMP);
		CTable *lcurve = NULL;
		if (CmpackLightCurve(NULL, &lcurve, sel, m_File, apertureId, CMPACK_LCURVE_FRAME_IDS)==0) {
			m_VarFind.SetLightCurve(*lcurve);
			delete lcurve;
		}
	}
}

//
// Enable/disable controls
//
void CVarFindFileDlg::UpdateControls(void)
{
	int varstar = m_VarFind.VariableID(), compstar = m_VarFind.ComparisonID(), 
		aperture = m_VarFind.ApertureIndex();

	m_Menu.Enable(CMD_SHOW_IMAGE, m_Frame.Valid());
	m_Menu.Enable(CMD_SHOW_MIXED, m_Frame.Valid());
	m_Menu.Check(CMD_SHOW_IMAGE, m_VarFind.DisplayMode()==CVarFind::DISPLAY_IMAGE);
	m_Menu.Check(CMD_SHOW_CHART, m_VarFind.DisplayMode()==CVarFind::DISPLAY_CHART);
	m_Menu.Check(CMD_SHOW_MIXED, m_VarFind.DisplayMode()==CVarFind::DISPLAY_MIXED);
	m_Menu.Enable(CMD_SAVE_MAGDEV, compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_EXPORT_MAGDEV, compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_SAVE_LCURVE, varstar>=0 && compstar>=0 && aperture>=0);
	m_Menu.Enable(CMD_EXPORT_LCURVE, varstar>=0 && compstar>=0 && aperture>=0);
}

//
// Show properties
//
void CVarFindFileDlg::ShowProperties(void)
{
	CVarFindInfoDlg dlg(GTK_WINDOW(m_pDlg));
	dlg.ShowModal(m_File, m_Name, m_Path);
}

//
// Is this control enabled?
//
bool CVarFindFileDlg::OnEnableCtrlQuery(CVarFind::tControlId ctrl)
{
	switch (ctrl)
	{
	case CVarFind::ID_SAVE_CHART:
		return (m_PhotometryFile ? m_Phot.Valid() : m_Catalog.Valid());
	case CVarFind::ID_REMOVE_FROM_DATASET:
		return true;
	default:
		return false;
	}
}

bool CVarFindFileDlg::SaveFile(const char *fpath, GError **error)
{
	if (!m_File.Save(fpath, 0, error)) 
		return false;

	int res = 0;
	if (m_PhotometryFile) {
		// Save reference photometry file
		gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_CATALOG);
		m_Phot.SelectAperture(m_VarFind.ApertureIndex());
		CCatalog cat2;
		if (!cat2.Create(m_Phot, error) || !cat2.SaveAs(dst_path, error))
			res = -1;
		g_free(dst_path);
	} else {
		// Copy catalog file
		gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_CATALOG);
		if (!m_Catalog.SaveAs(dst_path, error))
			res = -1;
		g_free(dst_path);
	}
	if (res==0 && m_Frame.Valid()) {
		// Save image file
		gchar *dst_path = SetFileExtension(fpath, FILE_EXTENSION_FITS);
		if (!m_Frame.SaveAs(dst_path, error))
			res = -1;
		g_free(dst_path);
	}
	return res==0;
}

void CVarFindFileDlg::RemoveFramesFromDataSet(void)
{
	bool changed = false;

	GList *rows = m_VarFind.GetSelectedFrames();
	if (rows) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			int frame = (gint)cmpack_graph_data_get_param(m_VarFind.LightCurveData(), (intptr_t)ptr->data);
			m_File.DeleteFrame(frame);
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

void CVarFindFileDlg::RemoveObjectsFromDataSet(void)
{
	bool changed = false;

	GList *rows = m_VarFind.GetSelectedObjects();
	if (rows) {
		for (GList *ptr=rows; ptr!=NULL; ptr=ptr->next) {
			int frame = (gint)cmpack_graph_data_get_param(m_VarFind.MagDevCurveData(), (intptr_t)ptr->data);
			m_File.DeleteObject(frame);
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

//---------------------------   INFO DIALOG   --------------------------------

enum tFieldId
{
	FIELD_FILE,
	FIELD_FILENAME,
	FIELD_DIRPATH
};

//
// Constructor
//
CVarFindInfoDlg::CVarFindInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0)
{
	AddHeading(FIELD_FILE, 0, 0, "File");
	AddField(FIELD_FILENAME, 0, 1, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(FIELD_DIRPATH, 0, 2, "Location", PANGO_ELLIPSIZE_MIDDLE);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

//
// Show dialog
//
void CVarFindInfoDlg::ShowModal(const CFrameSet &file, const gchar *name, const gchar *path)
{
	char buf[512];

	// Window caption
	sprintf(buf, "%s - %s - %s", name, "properties", g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);

	// Update properties
	gchar *basename = g_path_get_basename(path);
	SetField(FIELD_FILENAME, basename);
	g_free(basename);

	gchar *dirpath = g_path_get_dirname(path);
	SetField(FIELD_DIRPATH, dirpath);
	g_free(dirpath);

	// Show dialog
	CInfoDlg::ShowModal();
}
