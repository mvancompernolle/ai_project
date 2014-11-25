/**************************************************************

main_dlg.cpp (C-Munipack project)
Main dialog
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
#include <assert.h>

#include "utils.h"
#include "main.h"
#include "ctxhelp.h"
#include "configuration.h"
#include "profile.h"
#include "configuration.h"
#include "main_dlg.h"
#include "addfiles_dlg.h"
#include "addfolder_dlg.h"
#include "convert_dlg.h"
#include "messages_dlg.h"
#include "helper_dlgs.h"
#include "timecorr_dlg.h"
#include "biascorr_dlg.h"
#include "darkcorr_dlg.h"
#include "flatcorr_dlg.h"
#include "photometry_dlg.h"
#include "matching_dlg.h"
#include "makelightcurve_dlg.h"
#include "chart_dlg.h"
#include "project_dlg.h"
#include "about_dlg.h"
#include "frame_dlg.h"
#include "masterbias_dlg.h"
#include "masterdark_dlg.h"
#include "masterflat_dlg.h"
#include "makevarfind_dlg.h"
#include "maketrackcurve_dlg.h"
#include "makeamasscurve_dlg.h"
#include "jdconv_dlg.h"
#include "helcor_dlg.h"
#include "airmass_dlg.h"
#include "newfiles_dlg.h"
#include "makecatalogfile_dlg.h"
#include "merge_dlg.h"
#include "express_dlg.h"
#include "thumbnails_dlg.h"
#include "frameinfo_dlg.h"
#include "preferences_dlg.h"
#include "profiles_dlg.h"

enum tMenuId
{
	MENU_PROJECT = 1,
	MENU_FRAMES,
	MENU_REDUCE,
	MENU_PLOT,
	MENU_MAKE,
	MENU_TOOLS,
	MENU_HELP
};

enum tCommandId
{
	CMD_NEW_PROJECT = 100,
	CMD_OPEN_PROJECT,
	CMD_SAVE_PROJECT_AS,
	CMD_EDIT_PROJECT,
	CMD_CLOSE_PROJECT,
	CMD_CLEAR_PROJECT,
	CMD_ADD_FILES,
	CMD_ADD_FOLDER,
	CMD_REMOVE_FILES,
	CMD_CONVERT,
	CMD_OPEN_FILE,
	CMD_TIME_CORR,
	CMD_BIAS_CORR,
	CMD_DARK_CORR,
	CMD_FLAT_CORR,
	CMD_PHOTOMETRY,
	CMD_MATCHING,
	CMD_PLOT_LIGHT_CURVE,
	CMD_PLOT_TRACK_LIST,
	CMD_PLOT_AIR_MASS,
	CMD_FIND_VARIABLES,
	CMD_MASTER_BIAS,
	CMD_MASTER_DARK,
	CMD_MASTER_FLAT,
	CMD_MERGE_FRAMES,
	CMD_PROCESS_NEW_FILES,
	CMD_EXPRESS_REDUCTION,
	CMD_MAKE_CAT_FILE,
	CMD_TOOL_JDCONV,
	CMD_TOOL_HELCORR,
	CMD_TOOL_AIRMASS,
	CMD_EDIT_PROFILES,
	CMD_EDIT_ENVIRONMENT,
	CMD_EXIT_APP,
	CMD_MESSAGES,
	CMD_ABOUT,
	CMD_SHOW_HELP,
	CMD_USER_MANUAL,
	CMD_SHOW_THEORY,
	CMD_THUMBNAILS,
	CMD_FRAME_PREVIEW,
	CMD_FRAME_PROPERTIES,
	CMD_SELECT_ALL,
	CMD_IMPORT_DATA
};

//-------------------------   MAIN TOOLBAR   ---------------------------

static const CToolBar::tToolBtn MainToolBar[] = {
	{ CToolBar::TB_PUSHBUTTON, CMD_NEW_PROJECT, "Create a new project", "newproject" },
	{ CToolBar::TB_PUSHBUTTON, CMD_OPEN_PROJECT, "Open an existing project", "openproject" },
	{ CToolBar::TB_PUSHBUTTON, CMD_EDIT_PROJECT, "Edit project settings", "preferences" },
	{ CToolBar::TB_SEPARATOR },
	{ CToolBar::TB_PUSHBUTTON, CMD_ADD_FILES, "Add individual files to the projects", "addfiles" },
	{ CToolBar::TB_PUSHBUTTON, CMD_ADD_FOLDER, "Add all files from a folder and its subfolders", "addfolder" },
	{ CToolBar::TB_PUSHBUTTON, CMD_REMOVE_FILES, "Remove selected framed from the project", "removefiles" },
	{ CToolBar::TB_PUSHBUTTON, CMD_FRAME_PREVIEW, "Show selected frame", "preview" },
	{ CToolBar::TB_SEPARATOR },
	{ CToolBar::TB_PUSHBUTTON, CMD_EXPRESS_REDUCTION, "Express reduction", "reduction" },
	{ CToolBar::TB_PUSHBUTTON, CMD_PROCESS_NEW_FILES, "Process new frames", "newfiles" },
	{ CToolBar::TB_PUSHBUTTON, CMD_CONVERT, "Convert input files to working format", "convert" },
	{ CToolBar::TB_PUSHBUTTON, CMD_TIME_CORR, "Correct date and time of observation", "timecorr" },
	{ CToolBar::TB_PUSHBUTTON, CMD_BIAS_CORR, "Make bias-frame correction", "biascorr" },
	{ CToolBar::TB_PUSHBUTTON, CMD_DARK_CORR, "Make dark-frame correction", "darkcorr" },
	{ CToolBar::TB_PUSHBUTTON, CMD_FLAT_CORR, "Make flat-frame correction", "flatcorr" },
	{ CToolBar::TB_PUSHBUTTON, CMD_PHOTOMETRY, "Run photometry", "photometry" },
	{ CToolBar::TB_PUSHBUTTON, CMD_MATCHING, "Find cross-references between photometry files", "matchstars" },
	{ CToolBar::TB_SEPARATOR },
	{ CToolBar::TB_PUSHBUTTON, CMD_PLOT_LIGHT_CURVE, "Plot light curve", "lightcurve" },
	{ CToolBar::TB_SEPARATOR },
	{ CToolBar::TB_PUSHBUTTON, CMD_MASTER_BIAS, "Make master bias frame", "masterbias" },
	{ CToolBar::TB_PUSHBUTTON, CMD_MASTER_DARK, "Make master dark frame", "masterdark" },
	{ CToolBar::TB_PUSHBUTTON, CMD_MASTER_FLAT, "Make master flat frame", "masterflat" },
	{ CToolBar::TB_PUSHBUTTON, CMD_MERGE_FRAMES, "Merge frames", "merge" },
	{ CToolBar::TB_SEPARATOR },
	{ CToolBar::TB_PUSHBUTTON, CMD_FIND_VARIABLES, "Find variables", "varfind" },
	{ CToolBar::TB_PUSHBUTTON, CMD_MAKE_CAT_FILE, "Make catalog file", "catalogfile" },
	{ CToolBar::TB_PUSHBUTTON, CMD_THUMBNAILS, "Show thumbnails for source frames", "iconview" },
	{ CToolBar::TB_END }
};

//-------------------------   MAIN MENU   ---------------------------

static const CMenuBar::tMenuItem ProjectMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_NEW_PROJECT, "_New", "newproject" },
	{ CMenuBar::MB_ITEM, CMD_OPEN_PROJECT, "_Open", "openproject" },
	{ CMenuBar::MB_RECENTMENU, 0, "_Recent projects", "recentprojects", NULL, RECENT_GROUP_PROJECT },
	{ CMenuBar::MB_ITEM, CMD_SAVE_PROJECT_AS, "_Save as ...", "saveproject" },
	{ CMenuBar::MB_ITEM, CMD_CLOSE_PROJECT, "_Close", "closeproject" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_EDIT_PROJECT, "_Edit project settings", "preferences" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_EXIT_APP, "E_xit", "exit" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem FramesMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_ADD_FILES, "_Add individual frames", "addfiles" },
	{ CMenuBar::MB_ITEM, CMD_ADD_FOLDER, "Add frames from _folder", "addfolder" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_REMOVE_FILES, "_Remove selected frames", "removefiles" },
	{ CMenuBar::MB_ITEM, CMD_CLEAR_PROJECT, "Remove all frames", "clearfiles" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_FRAME_PREVIEW, "Show selected frame", "preview" },
	{ CMenuBar::MB_ITEM, CMD_FRAME_PROPERTIES, "Show properties", "properties" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ReduceMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_EXPRESS_REDUCTION, "Express reduction", "reduction" },
	{ CMenuBar::MB_ITEM, CMD_PROCESS_NEW_FILES, "Process _new frames", "newfiles" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_CONVERT, "Fetch/convert files", "convert" },
	{ CMenuBar::MB_ITEM, CMD_TIME_CORR, "_Time correction", "timecorr" },
	{ CMenuBar::MB_ITEM, CMD_BIAS_CORR, "_Bias correction", "biascorr" },
	{ CMenuBar::MB_ITEM, CMD_DARK_CORR, "_Dark correction", "darkcorr" },
	{ CMenuBar::MB_ITEM, CMD_FLAT_CORR, "_Flat correction", "flatcorr" },
	{ CMenuBar::MB_ITEM, CMD_PHOTOMETRY, "_Photometry", "photometry" },
	{ CMenuBar::MB_ITEM, CMD_MATCHING, "_Match stars", "matchstars" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem PlotMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_PLOT_LIGHT_CURVE, "_Light curve", "lightcurve" },
	{ CMenuBar::MB_ITEM, CMD_PLOT_TRACK_LIST, "_Track curve", "tracklist" },
	{ CMenuBar::MB_ITEM, CMD_PLOT_AIR_MASS, "_Air mass curve", "airmasscurve" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem MakeMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_MASTER_BIAS, "Master _bias frame", "masterbias" },
	{ CMenuBar::MB_ITEM, CMD_MASTER_DARK, "Master _dark frame", "masterdark" },
	{ CMenuBar::MB_ITEM, CMD_MASTER_FLAT, "Master _flat frame", "masterflat" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_MERGE_FRAMES, "_Merge frames", "merge" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem ToolsMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_FIND_VARIABLES, "_Find variables", "varfind" },
	{ CMenuBar::MB_ITEM, CMD_MAKE_CAT_FILE, "Make _catalog file", "catalogfile" },
	{ CMenuBar::MB_ITEM, CMD_THUMBNAILS, "_Show thumbnails", "iconview" },
	{ CMenuBar::MB_ITEM, CMD_MESSAGES, "Show _message log", "messagelog" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_OPEN_FILE, "_Open file" },
	{ CMenuBar::MB_RECENTMENU, 0, "_Recent files", NULL, NULL, "C-Munipack file" },
	{ CMenuBar::MB_ITEM, CMD_IMPORT_DATA, "_Import data from C-Munipack 1.x" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_TOOL_JDCONV, "_JD converter", "jdconv" },
	{ CMenuBar::MB_ITEM, CMD_TOOL_HELCORR, "_Heliocentric correction", "helcorr" },
	{ CMenuBar::MB_ITEM, CMD_TOOL_AIRMASS, "_Air mass coefficient", "airmass" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_EDIT_PROFILES, "Edit _profiles", "profile" },
	{ CMenuBar::MB_ITEM, CMD_EDIT_ENVIRONMENT, "_Environment options", "environment" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenuItem HelpMenu[] = {
	{ CMenuBar::MB_ITEM, CMD_SHOW_HELP, "_Show help", "help" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_USER_MANUAL, "_User's manual", "manual" },
	{ CMenuBar::MB_ITEM, CMD_SHOW_THEORY, "_Theory of operation", "theory" },
	{ CMenuBar::MB_SEPARATOR },
	{ CMenuBar::MB_ITEM, CMD_ABOUT, "_About Muniwin", "about" },
	{ CMenuBar::MB_END }
};

static const CMenuBar::tMenu MainMenu[] = {
	{ "_Project", MENU_PROJECT, ProjectMenu },
	{ "_Frames", MENU_FRAMES, FramesMenu },
	{ "_Reduce", MENU_REDUCE, ReduceMenu },
	{ "P_lot", MENU_PLOT, PlotMenu },
	{ "_Make", MENU_MAKE, MakeMenu },
	{ "_Tools", MENU_TOOLS, ToolsMenu },
	{ "_Help", MENU_HELP, HelpMenu },
	{ NULL }
};

//-------------------------   POPUP MENU   ------------------------------------

static const CPopupMenu::tPopupMenuItem ContextMenu[] = {
	{ CPopupMenu::MB_ITEM, CMD_FRAME_PREVIEW, "_Open Preview", "preview" },
	{ CPopupMenu::MB_ITEM, CMD_FRAME_PROPERTIES, "_Show Properties", "properties" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_REMOVE_FILES, "_Remove From Project", "removefiles" },
	{ CPopupMenu::MB_SEPARATOR },
	{ CPopupMenu::MB_ITEM, CMD_SELECT_ALL, "Select _All" },
	{ CPopupMenu::MB_END }
};

//-------------------------   HELPER FUNCTIONS   ------------------------------------

static void foreach_selected(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, char **fpath)
{
	if (*fpath==NULL) 
		gtk_tree_model_get(model, iter, FRAME_ORIGFILE, fpath, -1);
}

//-------------------------   FILE LIST ------------------------------------

// Icon identifiers
enum tIcon {
	ICO_ERROR,
	ICO_NEW,
	ICO_FETCHED,
	ICO_BIASCORR,
	ICO_TIMECORR,
	ICO_DARKCORR,
	ICO_FLATCORR,
	ICO_PHOTOMETRED,
	ICO_MATCHED
};

// Icon files
static const struct tTreeIcon {
	int id;
	const char *name;		// Icon name
} TreeViewIcons[] = {
	{ ICO_ERROR, "error16" },
	{ ICO_NEW, "inputfiles16" },
	{ ICO_FETCHED, "convert16" },
	{ ICO_BIASCORR, "biascorr16" },
	{ ICO_TIMECORR, "timecorr16" },
	{ ICO_DARKCORR, "darkcorr16" },
	{ ICO_FLATCORR, "flatcorr16" },
	{ ICO_PHOTOMETRED, "photometry16" },
	{ ICO_MATCHED, "matchstars16" },
	{ 0, NULL }
};

static void GetDateTime(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	double jd;
	CmpackDateTime dt;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, FRAME_JULDAT, &jd, -1);

	if (cmpack_decodejd(jd, &dt)==0) {
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
			dt.time.hour, dt.time.minute, dt.time.second);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetJD(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	double jd;
	char buf[64];

	gtk_tree_model_get(tree_model, iter, FRAME_JULDAT, &jd, -1);

	if (jd>0) {
		sprintf(buf, "%.*f", JD_PREC, jd);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetExpTime(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	double exp_time;
	char buf[64];

	gtk_tree_model_get(tree_model, iter, FRAME_EXPTIME, &exp_time, -1);

	if (exp_time>=0) {
		sprintf(buf, "%.3f", exp_time);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetCCDTemp(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	double ccd_temp;
	char buf[64];

	gtk_tree_model_get(tree_model, iter, FRAME_CCDTEMP, &ccd_temp, -1);

	if (ccd_temp>-999 && ccd_temp<999) {
		sprintf(buf, "%.1f", ccd_temp);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetStars(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int state;
	int mstars, nstars;
	char buf[128];

	gtk_tree_model_get(tree_model, iter, FRAME_STATE, &state, 
		FRAME_MSTARS, &mstars, FRAME_STARS, &nstars, -1);

	if (state & CFILE_MATCHING) {
		sprintf(buf, "%d/%d", nstars, mstars);
		g_object_set(cell, "text", buf, NULL);
	} else
	if (state & CFILE_PHOTOMETRY) {
		sprintf(buf, "%d", nstars);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetOffsetX(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int state;
	double offset;
	char buf[128];

	gtk_tree_model_get(tree_model, iter, FRAME_STATE, &state, 
		FRAME_OFFSET_X, &offset, -1);

	if (state & CFILE_MATCHING) {
		sprintf(buf, "%.1f", offset);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

static void GetOffsetY(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int state;
	double offset;
	char buf[128];

	gtk_tree_model_get(tree_model, iter, FRAME_STATE, &state, 
		FRAME_OFFSET_Y, &offset, -1);

	if (state & CFILE_MATCHING) {
		sprintf(buf, "%.1f", offset);
		g_object_set(cell, "text", buf, NULL);
	} else {
		g_object_set(cell, "text", "", NULL);
	}
}

void CMainWindow::GetIcon(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	int state;
	CMainWindow *pMe = (CMainWindow*)data;

	gtk_tree_model_get(tree_model, iter, FRAME_STATE, &state, -1);

	if ((state & CFILE_ERROR)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_ERROR), NULL);
	else if ((state & CFILE_MATCHING)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_MATCHED), NULL);
	else if ((state & CFILE_PHOTOMETRY)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_PHOTOMETRED), NULL);
	else if ((state & CFILE_FLATCORR)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_FLATCORR), NULL);
	else if ((state & CFILE_DARKCORR)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_DARKCORR), NULL);
	else if ((state & CFILE_TIMECORR)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_TIMECORR), NULL);
	else if ((state & CFILE_BIASCORR)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_BIASCORR), NULL);
	else if ((state & CFILE_CONVERSION)!=0)
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_FETCHED), NULL);
	else
		g_object_set(cell, "pixbuf", pMe->GetIconRef(ICO_NEW), NULL);
}

const static struct tTreeColumn {
	const char *caption;		// Column name
	int column;					// Model column index
	GtkTreeCellDataFunc datafn;	// Data function
	gfloat align;				// Text alignment
	int bitmask;				// Configuration bit (0 = fixed column)
} TreeViewColumns[] = {
	{ "Frame #",		FRAME_STRINGID,	NULL,			0, 0 },
	{ "Date and time (UTC)", 0,			GetDateTime,	1, (1<<0) },
	{ "Julian date",	0,				GetJD,			1, (1<<1) },
	{ "Exposure",		0,				GetExpTime,		1, (1<<2) },
	{ "Temperature",	0,				GetCCDTemp,		1, (1<<3) },
	{ "Filter",			FRAME_FILTER,	NULL,			0, (1<<4) },
	{ "Stars",			0,				GetStars,		1, (1<<5) },
	{ "Offset X",		0,				GetOffsetX,		1, (1<<8) },
	{ "Offset Y",		0,				GetOffsetY,		1, (1<<8) },
	{ "Original file",  FRAME_ORIGFILE,	NULL,			0, (1<<6) },
	{ "Temporary file", FRAME_TEMPFILE,	NULL,			0, (1<<7) },
	{ "Status",			FRAME_REPORT,	NULL,			0, 0 },
	{ NULL }
};

//-------------------------   MAIN WINDOW   --------------------------------

CMainWindow::CMainWindow(const char *fpath):m_pDlg(NULL), m_Previews(NULL), 
	m_Files(NULL), m_Outputs(NULL), m_LockCounter(0), m_StatusCtx(-1),
	m_StatusMsg(-1)
{
	char *icon;
	int i, count;
	GdkRectangle rc;
	
	// Load icons
	count = 0;
	for (i=0; TreeViewIcons[i].name!=NULL; i++)
		count++;
	if (count>0) {
		m_Icons = (GdkPixbuf**)g_malloc(count*sizeof(GdkPixbuf*));
		memset(m_Icons, 0, count*sizeof(GdkPixbuf*));
		for (i=0; i<count; i++) {
			char *icon = get_icon_file(TreeViewIcons[i].name);
			m_Icons[i] = gdk_pixbuf_new_from_file(icon, NULL);
			g_free(icon);
		}
	}

	// Main window
	m_pDlg = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_Box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_pDlg), m_Box);
	icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW (m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Window size
	GdkScreen *scr = gdk_screen_get_default();
	gdk_screen_get_monitor_geometry(scr, 0, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.6*rc.width), RoundToInt(0.6*rc.height));
	gtk_window_maximize(GTK_WINDOW(m_pDlg));

	g_signal_connect(G_OBJECT(m_pDlg), "delete_event", G_CALLBACK (exit_event), this);

	// Menu bar
	m_Menu.Create(MainMenu);
	m_Menu.RegisterCallback(MenuCallback, this);
	gtk_box_pack_start (GTK_BOX(m_Box), m_Menu.Handle(), FALSE, FALSE, 0);

	// Tool bar
	m_TBar.Create(MainToolBar);
	m_TBar.RegisterCallback(ToolbarCallback, this);
	gtk_box_pack_start (GTK_BOX(m_Box), m_TBar.Handle(), FALSE, FALSE, 0);

	// Make tree view
	m_TreeView = gtk_tree_view_new();
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_TreeView), true);
	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(m_TreeView), FRAME_ORIGFILE);
	g_signal_connect(G_OBJECT(m_TreeView), "row-activated", G_CALLBACK(row_activated), this);
	g_signal_connect(G_OBJECT(m_TreeView), "button_press_event", G_CALLBACK(button_press_event), this);

	// Make scrolled window for a tree view
	m_TreeScrWnd = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_TreeScrWnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_TreeScrWnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_TreeScrWnd), m_TreeView);
	gtk_box_pack_start(GTK_BOX(m_Box), m_TreeScrWnd, TRUE, TRUE, 0);

	// Profile
	m_NewFiles.RegisterCallback(NewFilesCallback, this);
	gtk_box_pack_start(GTK_BOX(m_Box), m_NewFiles.Handle(), FALSE, FALSE, 0);

	// Status bar
	m_Status = gtk_statusbar_new();
	gtk_box_pack_end(GTK_BOX(m_Box), m_Status, FALSE, FALSE, 0);
	m_StatusCtx = gtk_statusbar_get_context_id(GTK_STATUSBAR(m_Status), "Main");

	// Popup menu
	m_Popup.Create(ContextMenu);
	m_Popup.RegisterCallback(MenuCallback, this);

	// Register callback for selection change
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), this);

	// Load project file
	if (fpath) {
		g_Project->Open(fpath, CProject::OPEN_READWRITE, NULL, NULL);
	} else {
		if (CConfig::GetBool("MainDlg", "OpenLastProject")) {
			char *path = CConfig::GetStr("Projects", "Last", NULL);
			g_Project->Open(path, CProject::OPEN_READWRITE, NULL, NULL);
			g_free(path);		
		}
	}
	CConfig::SetBool("MainDlg", "OpenLastProject", g_Project->isOpen());
	
	gtk_widget_show_all(m_Box);
	m_NewFiles.Show(false);
	RebuildTable();
	UpdateTitle();
	UpdateControls();
	gtk_widget_show(GTK_WIDGET(m_pDlg));
}

CMainWindow::~CMainWindow()
{
	g_free(m_Icons);
	g_slist_free(m_Previews);
	g_slist_free(m_Files);
	g_slist_free(m_Outputs);
}

void CMainWindow::RebuildTable()
{
	bool first = true;
	int i, bitmask = CConfig::GetInt(CConfig::FILE_LIST_COLUMNS);
	GList *list, *ptr;
	const tTreeColumn *col;
	GtkTreeViewColumn *tvcol;
	GtkCellRenderer *renderer;

	// Rebuild tree view
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	// Delete old columns
	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(m_TreeView));
	for (ptr=list; ptr!=NULL; ptr=ptr->next) {
		tvcol = (GtkTreeViewColumn*)(ptr->data);
		gtk_tree_view_remove_column(GTK_TREE_VIEW(m_TreeView), tvcol);
	}
	g_list_free(list);
	// Create new columns
	for (i=0; TreeViewColumns[i].caption!=NULL; i++) {
		col = &TreeViewColumns[i];
		if (col->bitmask==0 || (col->bitmask & bitmask)!=0) {
			tvcol = gtk_tree_view_column_new();
			// Set column name and alignment
			gtk_tree_view_column_set_title(tvcol, col->caption);
			gtk_tree_view_append_column(GTK_TREE_VIEW(m_TreeView), tvcol);
			// Add pixbuf renderer to the first column
			if (first) {
				renderer = gtk_cell_renderer_pixbuf_new();
				gtk_tree_view_column_pack_start(tvcol, renderer, FALSE);
				gtk_tree_view_column_set_cell_data_func(tvcol, renderer, GetIcon, this, NULL);
				first = false;
			}
			// Add text renderer
			renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_column_pack_start(tvcol, renderer, TRUE);
			g_object_set(renderer, "xalign", col->align, NULL);
			if (col->datafn) 
				gtk_tree_view_column_set_cell_data_func(tvcol, renderer, col->datafn, NULL, NULL);
			else
				gtk_tree_view_column_add_attribute(tvcol, renderer, "text", col->column);
		}
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), g_Project->FileList());
}

gint CMainWindow::exit_event(GtkWidget *widget, GdkEvent *event, CMainWindow *pMe)
{
	// Finish "New Files" background process
	pMe->m_NewFiles.Show(false);

	if (pMe->UnsavedData()) {
		// Ask for confirmation if there are unsaved data in output windows
		if (!ShowConfirmation(pMe->m_pDlg, "There are unsaved data in output windows (light curves, track curves, etc.) These data will be lost.\nDo you want to close the application?"))
			return TRUE;
	}

	// Close non-modal windows
	pMe->CloseOutputDlgs();
	pMe->CloseFrameDlgs();
	pMe->CloseFileDlgs();
	
	// Save project file
	g_Project->Save();

	gtk_main_quit();
	return FALSE;
}

void CMainWindow::selection_changed(GtkTreeSelection *widget, CMainWindow *pMe)
{
	pMe->UpdateControls();
}

void CMainWindow::row_activated(GtkTreeView *tree_view, GtkTreePath *path, 
		GtkTreeViewColumn *column, CMainWindow *pMe)
{
	pMe->OnRowActivated(tree_view, path);
}

void CMainWindow::OnRowActivated(GtkTreeView *tree_view, GtkTreePath *path)
{
	ShowFramePreview(path);
}

void CMainWindow::ShowFramePreview(GtkTreePath *path)
{
	if (!path) {
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
		GList *list = gtk_tree_selection_get_selected_rows(sel, NULL);
		if (list) {
			CFrameDlg *pDlg = FindFrame(path);
			if (!pDlg) 
				pDlg = new CFrameDlg();
			if (pDlg->LoadFrame((GtkTreePath*)list->data))
				pDlg->Show();
			else
				pDlg->Close();
		}
		g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(list);
	} else {
		CFrameDlg *pDlg = FindFrame(path);
		if (!pDlg) 
			pDlg = new CFrameDlg();
		if (pDlg->LoadFrame(path))
			pDlg->Show();
		else
			pDlg->Close();
	}
}

void CMainWindow::ShowFrameProperties(GtkTreePath *path)
{
	if (!path) {
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
		GList *list = gtk_tree_selection_get_selected_rows(sel, NULL);
		if (list) {
			CFrameInfoDlg dlg(m_pDlg);
			dlg.Show((GtkTreePath*)list->data);
		}
		g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(list);
	} else {
		CFrameInfoDlg dlg(m_pDlg);
		dlg.Show(path);
	}
}

CFrameDlg *CMainWindow::FindFrame(GtkTreePath *pPath)
{
	CFrameDlg *retval = NULL;
	for (GSList *ptr=m_Previews; ptr!=NULL; ptr=ptr->next) {
		GtkTreePath *pPath2 = gtk_tree_row_reference_get_path(((CFrameDlg*)ptr->data)->Frame());
		if (pPath2) {
			if (gtk_tree_path_compare(pPath, pPath2)==0) {
				retval = (CFrameDlg*)ptr->data;
				gtk_tree_path_free(pPath2);
				break;
			}
			gtk_tree_path_free(pPath2);
		}
	}
	return retval;
}

void CMainWindow::CloseFrameDlgs(void)
{
	GSList *ptr = m_Previews;
	while (ptr) {
		GSList *next = ptr->next;
		((CFrameDlg*)ptr->data)->Close();
		ptr = next;
	}
}

void CMainWindow::RegisterFrameDlg(CFrameDlg *pFrameDlg)
{
	if (pFrameDlg) 
		m_Previews = g_slist_prepend(m_Previews, pFrameDlg);
}

void CMainWindow::FrameDlgClosed(CFrameDlg *pFrameDlg)
{
	if (pFrameDlg) 
		m_Previews = g_slist_remove(m_Previews, pFrameDlg);
}

CFileDlg *CMainWindow::FindFile(const char *path)
{
	for (GSList *ptr=m_Files; ptr!=NULL; ptr=ptr->next) {
		const gchar *fpath = ((CFileDlg*)ptr->data)->FilePath();
		if (fpath && ComparePaths(path, fpath)==0)
			return (CFileDlg*)ptr->data;
	}
	return NULL;
}

void CMainWindow::RegisterOutputDlg(COutputDlg *pOutputDlg)
{
	if (pOutputDlg)
		m_Outputs = g_slist_prepend(m_Outputs, pOutputDlg);
}

void CMainWindow::OutputDlgClosed(COutputDlg *pOutputDlg)
{
	if (pOutputDlg)
		m_Outputs = g_slist_remove(m_Outputs, pOutputDlg);
}

bool CMainWindow::UnsavedData(void)
{
	bool unsaved = false;
	for (GSList *ptr=m_Files; ptr!=NULL; ptr=ptr->next) {
		if (!((CFileDlg*)ptr->data)->DataSaved()) {
			unsaved = true;
			break;
		}
	}
	for (GSList *ptr=m_Outputs; ptr!=NULL; ptr=ptr->next) {
		if (!((COutputDlg*)ptr->data)->DataSaved()) {
			unsaved = true;
			break;
		}
	}
	return unsaved;
}

bool CMainWindow::QueryCloseOutputDlgs(void)
{
	if (UnsavedData()) 
		return ShowConfirmation(m_pDlg, "This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
	return true;
}

void CMainWindow::CloseOutputDlgs(void)
{
	GSList *ptr = m_Outputs;
	while (ptr) {
		GSList *next = ptr->next;
		((COutputDlg*)ptr->data)->Close();
		ptr = next;
	}
}

bool CMainWindow::QueryCloseNewFiles(void)
{
	if (m_NewFiles.IsRunning()) 
		return ShowConfirmation(m_pDlg, "This action will stop the background process (Process new frames).\nDo you want to continue?");
	return true;
}

void CMainWindow::CloseNewFiles(void)
{
	m_NewFiles.Show(false);
}

void CMainWindow::OnCommand(int cmd_id)
{
	switch (cmd_id)
	{
	case CMD_NEW_PROJECT:
		NewProject();
		break;
	case CMD_OPEN_PROJECT:
		OpenProject();
		break;
	case CMD_SAVE_PROJECT_AS:
		SaveProjectAs();
		break;
	case CMD_EDIT_PROJECT:
		EditProject();
		break;
	case CMD_CLOSE_PROJECT:
		CloseProject();
		break;

	case CMD_ADD_FILES:
		AddIndividualFrames();
		break;
	case CMD_ADD_FOLDER:
		AddFramesFromFolder();
		break;
	case CMD_REMOVE_FILES:
		RemoveFiles();
		break;
	case CMD_CLEAR_PROJECT:
		ClearProject();
		break;
	case CMD_FRAME_PREVIEW:
		ShowFramePreview(NULL);
		break;
	case CMD_FRAME_PROPERTIES:
		ShowFrameProperties(NULL);
		break;

	case CMD_CONVERT:
		ConvertFiles();
		break;
	case CMD_OPEN_FILE:
		OpenFile();
		break;
	case CMD_EXIT_APP:
		ExitApplication();
		break;

	case CMD_TIME_CORR:
		TimeCorrection();
		break;
	case CMD_BIAS_CORR:
		BiasCorrection();
		break;
	case CMD_DARK_CORR:
		DarkCorrection();
		break;
	case CMD_FLAT_CORR:
		FlatCorrection();
		break;
	case CMD_PHOTOMETRY:
		Photometry();
		break;
	case CMD_MATCHING:
		Matching();
		break;

	case CMD_EXPRESS_REDUCTION:
		ExpressReduction();
		break;
	case CMD_PROCESS_NEW_FILES:
		NewFiles();
		break;
	case CMD_PLOT_LIGHT_CURVE:
		MakeLightCurve();
		break;
	case CMD_PLOT_TRACK_LIST:
		PlotTrackCurve();
		break;
	case CMD_PLOT_AIR_MASS:
		PlotAirMassCurve();
		break;

	case CMD_FIND_VARIABLES:
		FindVariables();
		break;
	case CMD_MAKE_CAT_FILE:
		MakeCatalogFile();
		break;
	case CMD_MASTER_BIAS:
		MasterBias();
		break;
	case CMD_MASTER_DARK:
		MasterDark();
		break;
	case CMD_MASTER_FLAT:
		MasterFlat();
		break;
	case CMD_MERGE_FRAMES:
		MergeFrames();
		break;
	case CMD_TOOL_JDCONV:
		JDConverter();
		break;
	case CMD_TOOL_HELCORR:
		HelCorrection();
		break;
	case CMD_TOOL_AIRMASS:
		AirMass();
		break;
	case CMD_EDIT_ENVIRONMENT:
		EditEnvironment();
		break;
	case CMD_EDIT_PROFILES:
		EditProfiles();
		break;
	case CMD_IMPORT_DATA:
		ImportData();
		break;

	case CMD_MESSAGES:
		ShowMessages();
		break;
	case CMD_THUMBNAILS:
		ShowThumbnails();
		break;

	case CMD_ABOUT:
		AboutApplication();
		break;
	case CMD_USER_MANUAL:
		ShowHelp(GTK_WINDOW(m_pDlg), 0);
		break;
	case CMD_SHOW_THEORY:
#ifdef _WIN32
		ShowDocument(GTK_WINDOW(m_pDlg), "theory/theory.pdf");
#else
		ShowDocument(GTK_WINDOW(m_pDlg), "theory/html/theory.html");
#endif
		break;
	case CMD_SHOW_HELP:
		ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAIN_WINDOW);
		break;

	case CMD_SELECT_ALL:
		SelectAll();
		break;
	}
}

void CMainWindow::MenuCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CMainWindow *pMe = (CMainWindow*)cb_data;

	switch(message)
	{
	case CMenuBar::CB_ACTIVATE:
		// Menu bar command
		pMe->OnCommand(wparam);
		break;
	case CMenuBar::CB_RECENT_ACTIVATE:
		// Recent menu selected
		pMe->OpenFileOrProject((const gchar*)lparam);
		break;
	}
}

void CMainWindow::ToolbarCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CMainWindow *pMe = (CMainWindow*)cb_data;

	switch(message)
	{
	case CToolBar::CB_ACTIVATE:
		// Menu bar command
		pMe->OnCommand(wparam);
		break;
	}
}

// Exit application
void CMainWindow::ExitApplication(void)
{
	gtk_main_quit();
}

// Add files to project
void CMainWindow::AddIndividualFrames(void)
{
	CAddFilesDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Add folder to project
void CMainWindow::AddFramesFromFolder(void)
{
	CAddFolderDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Create a new project
void CMainWindow::NewProject(void)
{
	GError *error = NULL;

	// Ask for name, location and type
	CNewProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	if (!pDlg.Execute())
		return;

	// Close current project and create a new one
	BeginUpdate();
	CloseProject();
	if (g_Project->Open(pDlg.FilePath(), CProject::OPEN_CREATE, &error, GTK_WINDOW(m_pDlg))) {
		g_Project->SetProfile(pDlg.Profile());
		g_Project->Save();
		RebuildTable();
		UpdateTitle();
		UpdateControls();
		CConfig::SetBool("MainDlg", "OpenLastProject", g_Project->isOpen());
	} else {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
	}
	EndUpdate();
}

// Open existing project
void CMainWindow::OpenProject(void)
{
	COpenDlg pDlg(GTK_WINDOW(m_pDlg), false);
	if (pDlg.Execute()) 
		OpenFileOrProject(pDlg.Path());
}

// Save project as...
void CMainWindow::SaveProjectAs(void)
{
	CSaveProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	if (pDlg.Execute()) {
		GError *error = NULL;
		if (!g_Project->Export(pDlg.Path(), &error)) {
			if (error) {
				ShowError(GTK_WINDOW(m_pDlg), error->message);
				g_error_free(error);
			}
			return;
		}
		OpenFileOrProject(pDlg.Path());
	}
}

// Close project
void CMainWindow::CloseProject(void)
{
	CloseOutputDlgs();
	CloseNewFiles();
	CloseFrameDlgs();
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	g_Project->Close();
	UpdateTitle();
	UpdateControls();
	CConfig::SetBool("MainDlg", "OpenLastProject", false);
}

// Clear project
void CMainWindow::ClearProject(void)
{
	if (!ShowConfirmation(m_pDlg, "Do you want to remove all files from the current project?"))
		return;

	CloseOutputDlgs();
	CloseNewFiles();
	CloseFrameDlgs();
	g_Project->RemoveAllFrames();
	UpdateControls();
}

// Remove selected files from the project
void CMainWindow::RemoveFiles(void)
{
	bool ok, close_output;
	int count;

	GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	GList *rows = gtk_tree_selection_get_selected_rows(pSel, NULL);
	if (rows) {
		count = g_list_length(rows);
		if (g_Project->ContainsReferenceFrame(rows)) {
			if (count==1) 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "The selected frame is a reference frame. This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
			else
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "The selection includes a reference frame. This action will close all output windows (light curves, track curves, etc.)\nDo you want to continue?");
			close_output = true;
		} else {
			if (count==1) 
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frame from the current project?");
			else
				ok = ShowConfirmation(GTK_WINDOW(m_pDlg), "Do you want to remove the selected frames from the current project?");
			close_output = false;
		}
		if (ok) {
			if (close_output) {
				CloseOutputDlgs();
				CloseNewFiles();
			}
			BeginUpdate();
			g_Project->RemoveFiles(rows);
			EndUpdate();
			UpdateControls();
		}
		g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(rows);
	}
}

// Open file
void CMainWindow::OpenFile(void)
{
	COpenDlg pDlg(GTK_WINDOW(m_pDlg), true);
	if (pDlg.Execute()) 
		OpenFileOrProject(pDlg.Path());
}

void CMainWindow::OpenFileOrProject(const gchar *path)
{
	GError *error = NULL;

	if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
		ShowError(m_pDlg, "The file does not exist.");
		return;
	}
	if (CProject::isProjectFile(path, NULL)) {
		if (ComparePaths(path, g_Project->Path())!=0) {
			BeginUpdate();
			CloseProject();
			if (g_Project->Open(path, CProject::OPEN_READWRITE, &error, GTK_WINDOW(m_pDlg))) {
				RebuildTable();
				UpdateTitle();
				UpdateControls();
				CConfig::SetStr("Projects", "Last", path);
				CConfig::SetBool("MainDlg", "OpenLastProject", true);
			} else {
				if (error) {
					ShowError(GTK_WINDOW(m_pDlg), error->message);
					g_error_free(error);
				}
			}
			EndUpdate();
		}
	} else {
		CFileDlg::Open(m_pDlg, path);
	}
}

// Register file dialog
void CMainWindow::RegisterFileDlg(CFileDlg *pFileDlg)
{
	if (pFileDlg)
		m_Files = g_slist_prepend(m_Files, pFileDlg);
}

// Unregister file dialog
void CMainWindow::FileDlgClosed(CFileDlg *pFileDlg)
{
	if (pFileDlg)
		m_Files = g_slist_remove(m_Files, pFileDlg);
}

void CMainWindow::CloseFileDlgs(void)
{
	GSList *ptr = m_Files;
	while (ptr) {
		GSList *next = ptr->next;
		((CFileDlg*)ptr->data)->Close();
		ptr = next;
	}
}

// Show context help
void CMainWindow::ShowHelp(GtkWindow *pParent, int context_id)
{
	::ShowHelp(pParent, context_id);
}

// Show message log
void CMainWindow::ShowMessages(void)
{
	CMessagesDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.ShowModal();
}

// Show thumbnails
void CMainWindow::ShowThumbnails(void)
{
	CThumbnailsDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.ShowModal();
}

// Fetch/convert files
void CMainWindow::ConvertFiles(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CConvertDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Time correction
void CMainWindow::TimeCorrection(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CTimeCorrDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Bias correction
void CMainWindow::BiasCorrection(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CBiasCorrDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Dark correction
void CMainWindow::DarkCorrection(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CDarkCorrDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Flat correction
void CMainWindow::FlatCorrection(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CFlatCorrDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Photometry
void CMainWindow::Photometry(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CPhotometryDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Matching
void CMainWindow::Matching(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CMatchingDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Make light curve
void CMainWindow::MakeLightCurve(void)
{
	CMakeLightCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// About application
void CMainWindow::AboutApplication(void)
{
	CAboutDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Find variables
void CMainWindow::FindVariables(void)
{
	CMakeVarFindDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Make catalog file
void CMainWindow::MakeCatalogFile(void)
{
	CMakeCatFileDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Master bias
void CMainWindow::MasterBias(void)
{
	CMasterBiasDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Master dark 
void CMainWindow::MasterDark(void)
{
	CMasterDarkDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Master flat
void CMainWindow::MasterFlat(void)
{
	CMasterFlatDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Merge frames
void CMainWindow::MergeFrames(void)
{
	CMergeDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Plot track list
void CMainWindow::PlotTrackCurve(void)
{
	CMakeTrackCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Plot air mass curve
void CMainWindow::PlotAirMassCurve(void)
{
	CMakeAMassCurveDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Express reduction
void CMainWindow::ExpressReduction(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;
	CloseOutputDlgs();
	CloseNewFiles();

	CExpressDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Process new frames
void CMainWindow::NewFiles(void)
{
	CloseNewFiles();

	CNewFilesDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// JD converter
void CMainWindow::JDConverter(void)
{
	CJDConvDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Heliocentric correction
void CMainWindow::HelCorrection(void)
{
	CHelCorDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Air mass computation
void CMainWindow::AirMass(void)
{
	CAirMassDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Edit preferences
void CMainWindow::EditProject(void)
{
	CEditProjectDlg pDlg(GTK_WINDOW(m_pDlg));
	if (pDlg.Execute()) {
		UpdateTitle();
		UpdateControls();
	}
}

// Environment options
void CMainWindow::EditEnvironment(void)
{
	CPreferencesDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	RebuildTable();
	for (GSList *frame=m_Previews; frame!=NULL; frame=frame->next) 
		((CFrameDlg*)frame->data)->EnvironmentChanged();
	for (GSList *file=m_Files; file!=NULL; file=file->next)
		((CFileDlg*)file->data)->EnvironmentChanged();
	for (GSList *output=m_Outputs; output!=NULL; output=output->next)
		((COutputDlg*)output->data)->EnvironmentChanged();
	UpdateControls();
}

// Edit profiles
void CMainWindow::EditProfiles(void)
{
	CProfilesDlg pDlg(GTK_WINDOW(m_pDlg));
	pDlg.Execute();
	UpdateControls();
}

// Import data from old C-Munipack
void CMainWindow::ImportData(void)
{
	if (!QueryCloseOutputDlgs() || !QueryCloseNewFiles())
		return;

	CImportProjectDlg dlg(GTK_WINDOW(m_pDlg));
	if (!dlg.Execute())
		return;

	CImportProject2Dlg dlg2(GTK_WINDOW(m_pDlg));
	if (!dlg2.Execute())
		return;

	// Close current project and create a new one
	GError *error = NULL;
	BeginUpdate();
	CloseProject();
	if (g_Project->Open(dlg2.FilePath(), CProject::OPEN_CREATE, &error, GTK_WINDOW(m_pDlg)) &&
		g_Project->Import(dlg.Path(), &error)) {
		RebuildTable();
		UpdateTitle();
		UpdateControls();
		CConfig::SetBool("MainDlg", "OpenLastProject", g_Project->isOpen());
	} else {
		if (error) {
			ShowError(GTK_WINDOW(m_pDlg), error->message);
			g_error_free(error);
		}
	}
	EndUpdate();
}

// Update window title
void CMainWindow::UpdateTitle()
{
	char buf[MAX_PROJECT_NAME+64];

	if (g_Project->isOpen()) {
		if (g_Project->isReadOnly()) 
			sprintf(buf, "%s - %s (READ-ONLY)", g_Project->Name(), g_AppTitle);
		else
			sprintf(buf, "%s - %s", g_Project->Name(), g_AppTitle);
	} else
		strcpy(buf, g_AppTitle);
	gtk_window_set_title(GTK_WINDOW (m_pDlg), buf);
}

// Enable/disable controls
void CMainWindow::UpdateControls()
{
	int selected;
	char *fpath, buf[256];
	CProject::tStatus status;

	// Number of selected items
	GtkTreeSelection *pSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	selected = gtk_tree_selection_count_selected_rows(pSel);

	// Number of m_Files in various states
	g_Project->GetStatus(&status);

	// Configuration
	bool advcalib = g_Project->Profile()->GetBool(CProfile::ADVANCED_CALIBRATION);
	tProjectType type = g_Project->ProjectType();

	// Update menu items
	m_Menu.ShowMenu(MENU_FRAMES, status.open);
	m_Menu.ShowMenu(MENU_REDUCE, status.open && !status.readonly);
	m_Menu.ShowMenu(MENU_MAKE, status.open && (type==PROJECT_MASTER_BIAS || type==PROJECT_MASTER_DARK || 
		type==PROJECT_MASTER_FLAT || type==PROJECT_COMBINING || type==PROJECT_TEST));
	m_Menu.ShowMenu(MENU_PLOT, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_MAKE_CAT_FILE, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_BIAS_CORR, advcalib && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_MASTER_DARK || type==PROJECT_MASTER_FLAT || type==PROJECT_TEST));
	m_Menu.Show(CMD_DARK_CORR, (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_MASTER_FLAT || type==PROJECT_TEST));
	m_Menu.Show(CMD_FLAT_CORR, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_TIME_CORR, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_PHOTOMETRY, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_MATCHING, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_Menu.Show(CMD_MASTER_BIAS, type==PROJECT_MASTER_BIAS || type==PROJECT_TEST);
	m_Menu.Show(CMD_MASTER_DARK, type==PROJECT_MASTER_DARK || type==PROJECT_TEST);
	m_Menu.Show(CMD_MASTER_FLAT, type==PROJECT_MASTER_FLAT || type==PROJECT_TEST);
	m_Menu.Show(CMD_MERGE_FRAMES, type==PROJECT_COMBINING || type==PROJECT_TEST);

	m_Menu.Enable(CMD_SAVE_PROJECT_AS, status.open);
	m_Menu.Enable(CMD_CLOSE_PROJECT, status.open);
	m_Menu.Enable(CMD_EDIT_PROJECT, status.open);
	m_Menu.Enable(CMD_ADD_FILES, status.open && !status.readonly);
	m_Menu.Enable(CMD_ADD_FOLDER, status.open && !status.readonly);
	m_Menu.Enable(CMD_FIND_VARIABLES, status.open);
	m_Menu.Enable(CMD_THUMBNAILS, status.files>0);
	m_Menu.Enable(CMD_CLEAR_PROJECT, status.files>0 && !status.readonly);
	m_Menu.Enable(CMD_FRAME_PREVIEW, selected==1);
	m_Menu.Enable(CMD_FRAME_PROPERTIES, selected==1);
	m_Menu.Enable(CMD_REMOVE_FILES, selected>0 && !status.readonly);
	m_Menu.Enable(CMD_MASTER_BIAS, status.converted>0);
	m_Menu.Enable(CMD_MASTER_DARK, status.converted>0);
	m_Menu.Enable(CMD_MASTER_FLAT, status.converted>0);
	m_Menu.Enable(CMD_MERGE_FRAMES, status.matched>0);
	m_Menu.Enable(CMD_PLOT_LIGHT_CURVE, status.matched>0);
	m_Menu.Enable(CMD_PLOT_TRACK_LIST, status.matched>0);
	m_Menu.Enable(CMD_PLOT_AIR_MASS, status.files>0);
	m_Menu.Enable(CMD_MAKE_CAT_FILE, status.matched>0);
	m_Menu.Enable(CMD_EXPRESS_REDUCTION, status.files>0 && !status.readonly);
	m_Menu.Enable(CMD_PROCESS_NEW_FILES, status.files>0 && !status.readonly);
	m_Menu.Enable(CMD_CONVERT, status.files>0 && !status.readonly);
	m_Menu.Enable(CMD_BIAS_CORR, status.converted>0 && !status.readonly);
	m_Menu.Enable(CMD_DARK_CORR, status.converted>0 && !status.readonly);
	m_Menu.Enable(CMD_TIME_CORR, status.converted>0 && !status.readonly);
	m_Menu.Enable(CMD_FLAT_CORR, status.converted>0 && !status.readonly);
	m_Menu.Enable(CMD_PHOTOMETRY, status.converted>0 && !status.readonly);
	m_Menu.Enable(CMD_MATCHING, status.photometred>0 && !status.readonly);

	// Update toolbar controls
	m_TBar.Show(CMD_EDIT_PROJECT, status.open);
	m_TBar.Show(CMD_ADD_FILES, status.open);
	m_TBar.Show(CMD_ADD_FOLDER, status.open);
	m_TBar.Show(CMD_REMOVE_FILES, status.open);
	m_TBar.Show(CMD_FRAME_PREVIEW, status.open);
	m_TBar.Show(CMD_EXPRESS_REDUCTION, status.open);
	m_TBar.Show(CMD_PROCESS_NEW_FILES, status.open);
	m_TBar.Show(CMD_CONVERT, status.open);
	m_TBar.Show(CMD_THUMBNAILS, status.open);
	m_TBar.Show(CMD_TIME_CORR, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_BIAS_CORR, status.open && advcalib && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_MASTER_DARK || type==PROJECT_MASTER_FLAT || type==PROJECT_TEST));
	m_TBar.Show(CMD_DARK_CORR, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_MASTER_FLAT || type==PROJECT_TEST));
	m_TBar.Show(CMD_FLAT_CORR, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_PHOTOMETRY, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_MATCHING, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_PLOT_LIGHT_CURVE, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_MAKE_CAT_FILE, status.open && (type==PROJECT_REDUCE || type==PROJECT_COMBINING || 
		type==PROJECT_TEST));
	m_TBar.Show(CMD_MASTER_BIAS, status.open && (type==PROJECT_MASTER_BIAS || type==PROJECT_TEST));
	m_TBar.Show(CMD_MASTER_DARK, status.open && (type==PROJECT_MASTER_DARK || type==PROJECT_TEST));
	m_TBar.Show(CMD_MASTER_FLAT, status.open && (type==PROJECT_MASTER_FLAT || type==PROJECT_TEST));
	m_TBar.Show(CMD_MERGE_FRAMES, status.open && (type==PROJECT_COMBINING || type==PROJECT_TEST));

	m_TBar.Enable(CMD_ADD_FILES, !status.readonly);
	m_TBar.Enable(CMD_ADD_FOLDER, !status.readonly);
	m_TBar.Enable(CMD_REMOVE_FILES, selected>0 && !status.readonly);
	m_TBar.Enable(CMD_CONVERT, status.files>0 && !status.readonly);
	m_TBar.Enable(CMD_EXPRESS_REDUCTION, status.files>0 && !status.readonly);
	m_TBar.Enable(CMD_PROCESS_NEW_FILES, status.files>0 && !status.readonly);
	m_TBar.Enable(CMD_TIME_CORR, status.converted>0 && !status.readonly);
	m_TBar.Enable(CMD_BIAS_CORR, status.converted>0 && !status.readonly);
	m_TBar.Enable(CMD_DARK_CORR, status.converted>0 && !status.readonly);
	m_TBar.Enable(CMD_FLAT_CORR, status.converted>0 && !status.readonly);
	m_TBar.Enable(CMD_PHOTOMETRY, status.converted>0 && !status.readonly);
	m_TBar.Enable(CMD_MATCHING, status.photometred>0 && !status.readonly);
	m_TBar.Enable(CMD_PLOT_LIGHT_CURVE, status.matched>0);
	m_TBar.Enable(CMD_MAKE_CAT_FILE, status.matched>0);
	m_TBar.Enable(CMD_MASTER_BIAS, status.converted>0);
	m_TBar.Enable(CMD_MASTER_DARK, status.converted>0);
	m_TBar.Enable(CMD_MASTER_FLAT, status.converted>0);
	m_TBar.Enable(CMD_MERGE_FRAMES, status.matched>0);
	m_TBar.Enable(CMD_THUMBNAILS, status.files>0);

	// Update status bar
	if (selected==0)
		SetStatus(NULL);
	else if (selected==1) {
		fpath = NULL;
		gtk_tree_selection_selected_foreach(pSel, 
			GtkTreeSelectionForeachFunc(foreach_selected), &fpath);
		SetStatus(fpath);
		g_free(fpath);
	} else {
		sprintf(buf, "%d selected files", selected);
		SetStatus(buf);
	}
}

gint CMainWindow::button_press_event(GtkWidget *widget, GdkEventButton *event, CMainWindow *pMe)
{
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		pMe->OnContextMenu(widget, event);
		return TRUE;
	}
	return FALSE;
}


void CMainWindow::BeginUpdate(void)
{
	assert(m_LockCounter >= 0);
	if (m_LockCounter==0) {
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), NULL);
	}
	m_LockCounter++;
}

void CMainWindow::EndUpdate(void)
{
	assert(m_LockCounter > 0);
	m_LockCounter--;
	if (m_LockCounter==0) {
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_TreeView), g_Project->FileList());
	}
}

GdkPixbuf *CMainWindow::GetIconRef(int icon_id)
{
	int i;

	for (i=0; TreeViewIcons[i].name!=NULL; i++) {
		if (TreeViewIcons[i].id == icon_id)
			return m_Icons[i];
	}
	return NULL;
}

void CMainWindow::OnContextMenu(GtkWidget *widget, GdkEventButton *event)
{
	int x = (int)event->x, y = (int)event->y;
	GtkTreePath *path;

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(m_TreeView), x, y, &path, NULL, NULL, NULL)) {
		gtk_widget_grab_focus(m_TreeView);
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
		if (gtk_tree_selection_count_selected_rows(sel)<=1)
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_TreeView), path, NULL, FALSE);

		int count = gtk_tree_selection_count_selected_rows(sel);
		m_Popup.Enable(CMD_FRAME_PROPERTIES, count==1);
		m_Popup.Enable(CMD_FRAME_PREVIEW, count==1);
		m_Popup.Enable(CMD_REMOVE_FILES, count>=1);

		int cmd_id = m_Popup.Execute(event);
		if (cmd_id>=0)
			OnCommand(cmd_id);
		gtk_tree_path_free(path);
	}
}

void CMainWindow::SelectAll()
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_TreeView));
	gtk_tree_selection_select_all(sel);
}

void CMainWindow::SetStatus(const char *text)
{
	if (m_StatusMsg>=0) {
		gtk_statusbar_pop(GTK_STATUSBAR(m_Status), m_StatusCtx);
		m_StatusMsg = -1;
	}
	if (text && strlen(text)>0) 
		m_StatusMsg = gtk_statusbar_push(GTK_STATUSBAR(m_Status), m_StatusCtx, text);
}

void CMainWindow::NewFilesCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
}
