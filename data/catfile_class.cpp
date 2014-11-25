/**************************************************************

catfile_class.cpp (C-Munipack project)
Catalog file class interface
Copyright (C) 2011 David Motl, dmotl@volny.cz

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

#include "catfile_class.h"
#include "main.h"

//-------------------------   PRIVATE DATA   ---------------------------

static const struct {
	const gchar *label;
	CmpackColor color;
	const gchar *color_name;
} Types[CMPACK_SELECT_COUNT] = {
	{ NULL, CMPACK_COLOR_DEFAULT, NULL },
	{ "var", CMPACK_COLOR_RED, "red" },
	{ "comp", CMPACK_COLOR_GREEN, "green" },
	{ "check", CMPACK_COLOR_BLUE, "blue" }
};

//--------------------------   HELPER FUNCTIONS   ----------------------------------

struct tCatalogPrintData
{
	CCatalog::tColumnId m_col;
	double	m_min, m_max;
	int		m_prec;
	tCatalogPrintData(CCatalog::tColumnId col, double min, double max, int prec):m_col(col), 
		m_min(min), m_max(max), m_prec(prec) {}
};

static void DestroyPrintData(tCatalogPrintData *data)
{
	delete data;
}

static void PrintStr(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tCatalogPrintData *data)
{
	gchar *text, *fg_color;

	gtk_tree_model_get(tree_model, iter, data->m_col, &text, CCatalog::COL_FG_COLOR, &fg_color, -1);
	g_object_set(cell, "text", text, "foreground", fg_color, NULL);
	g_free(fg_color);
	g_free(text);
}

static void PrintInt(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tCatalogPrintData *data)
{
	int value;
	gchar buf[256], *fg_color;

	gtk_tree_model_get(tree_model, iter, data->m_col, &value, CCatalog::COL_FG_COLOR, &fg_color, -1);
	if (value>=data->m_min && value<=data->m_max) {
		sprintf(buf, "%d", value);
		g_object_set(cell, "text", buf, "foreground", fg_color, NULL);
	} else 
		g_object_set(cell, "text", "--", "foreground", fg_color, NULL);
	g_free(fg_color);
}

static void PrintDouble(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tCatalogPrintData *data)
{
	double value;
	gchar buf[256], *fg_color;

	gtk_tree_model_get(tree_model, iter, data->m_col, &value, CCatalog::COL_FG_COLOR, &fg_color, -1);
	if (value>=data->m_min && value<=data->m_max) {
		sprintf(buf, "%.*f", data->m_prec, value);
		g_object_set(cell, "text", buf, "foreground", fg_color, NULL);
	} else 
		g_object_set(cell, "text", "------", "foreground", fg_color, NULL);
	g_free(fg_color);
}

static void PrintBrightness(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tCatalogPrintData *data)
{
	int mag_valid;
	double mag;
	gchar buf[256], *fg_color;

	gtk_tree_model_get(tree_model, iter, CCatalog::COL_MAG_VALID, &mag_valid, CCatalog::COL_MAG, &mag, 
		CCatalog::COL_FG_COLOR, &fg_color, -1);
	if (mag_valid) {
		sprintf(buf, "%.4f", mag);
		g_object_set(cell, "text", buf, "foreground", fg_color, NULL);
	} else 
		g_object_set(cell, "text", "------", "foreground", fg_color, NULL);
	g_free(fg_color);
}

static gint CompareBrightness(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tCatalogPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double mag_a, mag_b;

	gtk_tree_model_get(tree_model, a, CCatalog::COL_MAG_VALID, &mag_valid_a, CCatalog::COL_MAG, &mag_a, -1);
	gtk_tree_model_get(tree_model, b, CCatalog::COL_MAG_VALID, &mag_valid_b, CCatalog::COL_MAG, &mag_b, -1);
	if (!mag_valid_a && !mag_valid_b)
		return 0;
	else if (!mag_valid_a)
		return 1;
	else if (!mag_valid_b)
		return -1;
	else if (mag_a < mag_b)
		return -1;
	else if (mag_a == mag_b)
		return 0;
	else
		return 1;
}

static void AddViewCol(GtkTreeView *view, GtkTreeCellDataFunc print_fn, tCatalogPrintData *print_data, 
	const gchar *name, double xalign, int sort_column_id)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", xalign, NULL);
	int index = gtk_tree_view_insert_column_with_data_func(view, -1, name, renderer, print_fn, print_data, (GDestroyNotify)DestroyPrintData)-1;
	GtkTreeViewColumn *col = gtk_tree_view_get_column(view, index);
	g_object_set_data(G_OBJECT(col), "sort-column-id", GINT_TO_POINTER(sort_column_id));
}

static void AddViewCol_Text(GtkTreeView *view, CCatalog::tColumnId column, const gchar *name, 
	int sort_column_id)
{
	AddViewCol(view, (GtkTreeCellDataFunc)PrintStr, new tCatalogPrintData(column, 0, 0, 0), name, 
		0.0, sort_column_id);
}

static void AddViewCol_Int(GtkTreeView *view, CCatalog::tColumnId column, double min, double max, 
	const gchar *name, int sort_column_id)
{
	AddViewCol(view, (GtkTreeCellDataFunc)PrintInt, new tCatalogPrintData(column, min, max, 0), name, 
		1.0, sort_column_id);
}

static void AddViewCol_Dbl(GtkTreeView *view, CCatalog::tColumnId column, double min, double max, int prec, 
	const gchar *name, int sort_column_id)
{
	AddViewCol(view, (GtkTreeCellDataFunc)PrintDouble, new tCatalogPrintData(column, min, max, prec), name, 
		1.0, sort_column_id);
}

//-------------------   Catalogue file class interface  -------------------------------------


//
// Constructor with intialization (steals the reference)
//
CCatalog::CCatalog(CmpackCatFile *handle):m_Handle(NULL), m_CacheFlags(0), m_RefMag(0), 
	m_CurrentSelection(-1) 
{
	if (handle)
		m_Handle = cmpack_cat_reference(handle);
}


//
// Destructor
//
CCatalog::~CCatalog()
{
	if (m_Handle)
		cmpack_cat_destroy(m_Handle);
}

//
// Create memory only file
//
void CCatalog::Create(void)
{
	Clear();

	m_Handle = cmpack_cat_new();
	InvalidateCache();
}

//
// Copy data from a file
//
bool CCatalog::Load(const gchar *fpath, GError **error)
{
	g_assert(fpath != NULL);

	Clear();

	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	CmpackCatFile *handle;
	int res = cmpack_cat_open(&handle, f, CMPACK_OPEN_READONLY, 0);
	g_free(f);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		return false;
	}

	m_Handle = cmpack_cat_new();
	cmpack_cat_copy(m_Handle, handle);
	cmpack_cat_close(handle);
	InvalidateCache();
	return true;
}

//
// Save catalog file 
//
bool CCatalog::SaveAs(const gchar *fpath, GError **error) const
{
	g_assert (m_Handle!=NULL);

	CmpackCatFile *file;
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_cat_open(&file, f, CMPACK_OPEN_CREATE, 0);
	g_free(f);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		return false;
	}

	res = cmpack_cat_copy(file, m_Handle);
	if (res==0)
		res = cmpack_cat_close(file);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		cmpack_cat_destroy(file);
		return false;
	}
	return res==0;
}

//
// Refresh cached data
//
void CCatalog::InvalidateCache()
{
	m_CacheFlags = 0;
}

//
// Get selection
//
const CSelectionList *CCatalog::Selections(void)
{
	if (!(m_CacheFlags & CF_SELECTIONS)) {
		m_Selections.Clear();
		if (m_Handle) {
			int count = cmpack_cat_selection_set_count(m_Handle);
			for (int i=0; i<count; i++) {
				cmpack_cat_set_current_selection_set(m_Handle, i);
				CSelection sel;
				LoadSelection(sel, CMPACK_SELECT_VAR);
				LoadSelection(sel, CMPACK_SELECT_COMP);
				LoadSelection(sel, CMPACK_SELECT_CHECK);
				m_Selections.Set(cmpack_cat_get_selection_set_name(m_Handle), sel);
			}
		}
		m_CacheFlags |= CF_SELECTIONS;
	}
	return &m_Selections;
}

//
// Get selection
//
const CTags *CCatalog::Tags(void)
{
	int id;
	const char *tag;

	if (!(m_CacheFlags & CF_TAGS)) {
		m_Tags.Clear();
		if (m_Handle) {
			int count = cmpack_cat_get_tag_count(m_Handle);
			for (int i=0; i<count; i++) {
				cmpack_cat_get_tag(m_Handle, i, &id, &tag);
				m_Tags.Set(id, tag);
			}
		}
		m_CacheFlags |= CF_TAGS;
	}
	return &m_Tags;
}

//
// Get object coordinates
//
const CObjectCoords *CCatalog::Object(void)
{
	if (!(m_CacheFlags & CF_OBJECT)) {
		m_Object.Clear();
		if (m_Handle) {
			const gchar *name = cmpack_cat_gkys(m_Handle, "object");
			if (name) {
				gchar *aux = g_strdup(name);
				m_Object.SetName(g_strstrip(aux));
				g_free(aux);
			}
			const gchar *ra = cmpack_cat_gkys(m_Handle, "ra2000");
			if (ra) {
				gchar *aux = g_strdup(ra);
				m_Object.SetRA(g_strstrip(aux));
				g_free(aux);
			}
			const gchar *dec = cmpack_cat_gkys(m_Handle, "dec2000");
			if (dec) {
				gchar *aux = g_strdup(dec);
				m_Object.SetDec(g_strstrip(aux));
				g_free(aux);
			}
		}
		m_CacheFlags |= CF_OBJECT;
	}
	return &m_Object;
}

//
// Get location
//
const CLocation *CCatalog::Location(void)
{
	if (!(m_CacheFlags & CF_LOCATION)) {
		m_Location.Clear();
		if (m_Handle) {
			const gchar *name = cmpack_cat_gkys(m_Handle, "observatory");
			if (name) {
				gchar *aux = g_strdup(name);
				m_Location.SetName(g_strstrip(aux));
				g_free(aux);
			}
			const gchar *lon = cmpack_cat_gkys(m_Handle, "longitude");
			if (lon) {
				gchar *aux = g_strdup(lon);
				m_Location.SetLon(g_strstrip(aux));
				g_free(aux);
			}
			const gchar *lat = cmpack_cat_gkys(m_Handle, "latitude");
			if (lat) {
				gchar *aux = g_strdup(lat);
				m_Location.SetLat(g_strstrip(aux));
				g_free(aux);
			}
		}
		m_CacheFlags |= CF_LOCATION;
	}
	return &m_Location;
}

//
// Clear data
//
void CCatalog::Clear()
{
	if (m_Handle) {
		cmpack_cat_destroy(m_Handle);
		m_Handle = NULL;
	}
	m_Selections.Clear();
	m_Object.Clear();
	m_Location.Clear();
	m_Tags.Clear();
	m_RefMag = 0;
	m_CurrentSelection = -1;
	InvalidateCache();
}

//
// Date of observation
//
double CCatalog::JulianDate(void) const
{
	double jd;
	if (m_Handle && cmpack_cat_gkyd(m_Handle, "jd", &jd)==0)
		return jd;
	return 0.0;
}

//
// Exposure duration
//
double CCatalog::ExposureDuration(void) const
{
	double exptime;
	if (m_Handle && cmpack_cat_gkyd(m_Handle, "exptime", &exptime)==0)
		return exptime;
	return 0.0;
}

//
// Number of stars
//
int CCatalog::ObjectCount(void) const
{
	if (m_Handle)
		return cmpack_cat_nstar(m_Handle);
	return 0;
}

//
// Find object
//
int CCatalog::FindObject(int id)
{
	if (m_Handle)
		return cmpack_cat_find_star(m_Handle, id);
	return -1;
}


//
// Get star identifier
//
int CCatalog::GetObjectID(int index) const
{
	CmpackCatObject obj;
	if (m_Handle && cmpack_cat_get_star(m_Handle, index, CMPACK_OM_ID, &obj)==0)
		return obj.id;
	return -1;
}

//
// Get star identifier
//
bool CCatalog::GetObject(int star_index, unsigned mask, CmpackCatObject *obj) const
{
	if (m_Handle)
		return cmpack_cat_get_star(m_Handle, star_index, mask, obj)==0;
	return false;
}

//
// Load selection (one type)
//
void CCatalog::LoadSelection(CSelection &selection, CmpackSelectionType type1)
{
	if (m_Handle) {
		int nstars = cmpack_cat_get_selection_count(m_Handle);
		int count = 0;
		for (int i=0; i<nstars; i++) {
			CmpackSelectionType type;
			cmpack_cat_get_selection(m_Handle, i, NULL, &type);
			if (type == type1) 
				count++;
		}
		if (count>0) {
			int *buf = (int*)g_malloc(count*sizeof(int));
			int j = 0;
			for (int i = 0; i<nstars; i++) {
				CmpackSelectionType type;
				int id;
				cmpack_cat_get_selection(m_Handle, i, &id, &type);
				if (type == type1)
					buf[j++] = id;
			}
			selection.SetStarList(type1, buf, j);
			g_free(buf);
		}
	}
}

//
// Save selection
//
void CCatalog::SaveSelection(const CSelection &selection, CmpackSelectionType type) const
{
	int i, count, *buf;

	g_assert (m_Handle!=NULL);

	count = selection.CountStars(type);
	if (count>0) {
		buf = (int*)g_malloc(count*sizeof(int));
		selection.GetStarList(type, buf, count);
		for (i=0; i<count; i++) 
			cmpack_cat_update_selection(m_Handle, buf[i], type);
		g_free(buf);
	}
}

//
// Remove all selections
//
void CCatalog::RemoveAllSelections(void)
{
	if (m_Handle) {
		m_Selections.Clear();
		if (m_Handle)
			cmpack_cat_clear_all_selections(m_Handle);
		m_CacheFlags |= CF_SELECTIONS;
	}
}

//
// Add one selection
//
void CCatalog::AddSelection(const gchar *name, const CSelection &selection)
{
	g_assert (m_Handle!=NULL);

	Selections();
	m_Selections.Set(name, selection);
	if (cmpack_cat_selection_set_new(m_Handle, name, -1)==0) {
		SaveSelection(selection, CMPACK_SELECT_VAR);
		SaveSelection(selection, CMPACK_SELECT_COMP);
		SaveSelection(selection, CMPACK_SELECT_CHECK);
	}
}

//
// Save tags to a file
//
void CCatalog::SetTags(const CTags &tags)
{
	int id;
	const char *tag;

	g_assert (m_Handle!=NULL);

	m_Tags = tags;
	cmpack_cat_clear_tags(m_Handle);
	CTags::tIter iter;
	bool ok = m_Tags.First(&iter, &id, &tag);
	while (ok) {
		cmpack_cat_update_tag(m_Handle, id, tag);
		ok = m_Tags.Next(&iter, &id, &tag);
	}
	m_CacheFlags |= CF_TAGS;
}

//
// Chart width in pixels
//
int CCatalog::Width(void) const
{
	if (m_Handle)
		return cmpack_cat_get_width(m_Handle);
	return 0;
}

//
// Chart height in pixels
//
int CCatalog::Height(void) const
{
	if (m_Handle)
		return cmpack_cat_get_height(m_Handle);
	return 0;
}

//
// Date of observation
//
bool CCatalog::DateTime(CmpackDateTime *dt) const
{
	double jd = JulianDate();
	if (jd>0.0)
		return cmpack_decodejd(jd, dt)==0;
	return false;
}

//
// Filter name
//
const char *CCatalog::Filter(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "filter");
	return NULL;
}
void CCatalog::SetFilter(const gchar *filter)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "filter", filter, NULL);
}

//
// Observer's name
//
const gchar *CCatalog::Observer(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "observer");
	return NULL;
}
void CCatalog::SetObserver(const gchar *observer)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "observer", observer, NULL);
}

//
// Telescope
//
const gchar *CCatalog::Telescope(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "telescope");
	return NULL;
}
void CCatalog::SetTelescope(const gchar *telescope)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "telescope", telescope, NULL);
}

//
// Camera
//
const gchar *CCatalog::Instrument(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "camera");
	return NULL;
}
void CCatalog::SetInstrument(const gchar *camera)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "camera", camera, NULL);
}

//
// Field of view
//
const gchar *CCatalog::FieldOfView(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "fov");
	return NULL;
}
void CCatalog::SetFieldOfView(const gchar *fov)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "fov", fov, NULL);
}

//
// Orientation
//
const gchar *CCatalog::Orientation(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "orientation");
	return NULL;
}
void CCatalog::SetOrientation(const gchar *orientation)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "orientation", orientation, NULL);
}

//
// Notes
//
const gchar *CCatalog::Notes(void) const
{
	if (m_Handle)
		return cmpack_cat_gkys(m_Handle, "notes");
	return NULL;
}
void CCatalog::SetNotes(const gchar *notes)
{
	g_assert (m_Handle!=NULL);

	cmpack_cat_pkys(m_Handle, "notes", notes, NULL);
}

//
// Get header field by index
//
bool CCatalog::GetParam(int index, const char **key, const char **val, const char **com) const
{
	if (m_Handle)
		return cmpack_cat_gkyn(m_Handle, index, key, val, com)!=0;
	return false;
}

//
// Update reference magnitudes
//
double CCatalog::GetRefMag(void)
{
	int i, count;
	CmpackCatObject obj;

	if (!(m_CacheFlags & CF_REF_MAG)) {
		m_RefMag = 0.0;
		if (m_Handle) {
			int nstars = cmpack_cat_nstar(m_Handle);
			if (nstars>0) {
				double *maglist = (double*)g_malloc(nstars*sizeof(double));
				count = 0;
				for (i=0; i<nstars; i++) {
					if (GetObject(i, CMPACK_OM_MAGNITUDE, &obj) && obj.refmag_valid) 
						maglist[count++] = obj.refmagnitude;
				}
				if (count>0) 
					cmpack_robustmean(count, maglist, &m_RefMag, NULL);
				g_free(maglist);
			}
		}
		m_CacheFlags |= CF_REF_MAG;
	}
	return m_RefMag;
}

//
// Set location
//
void CCatalog::SetLocation(const CLocation &loc)
{
	g_assert (m_Handle!=NULL);

	if (m_Location!=loc) {
		m_Location = loc;
		if (m_Location.Name())
			cmpack_cat_pkys(m_Handle, "observatory", m_Location.Name(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "observatory");
		if (m_Location.Lon()) 
			cmpack_cat_pkys(m_Handle, "longitude", m_Location.Lon(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "longitude");
		if (m_Location.Lat()) 
			cmpack_cat_pkys(m_Handle, "latitude", m_Location.Lat(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "latitude");
		m_CacheFlags |= CF_LOCATION;
	}
}

//
// Set object coordinates
//
void CCatalog::SetObject(const CObjectCoords &obj)
{
	g_assert (m_Handle!=NULL);

	if (m_Object!=obj) {
		m_Object = obj;
		if (m_Object.Name())
			cmpack_cat_pkys(m_Handle, "object", m_Object.Name(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "object");
		if (m_Object.RA())
			cmpack_cat_pkys(m_Handle, "ra2000", m_Object.RA(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "ra2000");
		if (m_Object.Dec())
			cmpack_cat_pkys(m_Handle, "dec2000", m_Object.Dec(), NULL);
		else
			cmpack_cat_dkey(m_Handle, "dec2000");
		m_CacheFlags |= CF_OBJECT;
	}
}

//
// Set current selection
//
void CCatalog::SelectSelection(int index)
{
	m_CurrentSelection = index;
}

//
// Make catalogue file from photometry file
//
bool CCatalog::Create(const CPhot &pht, GError **error)
{
	assert (pht.m_Handle != NULL);

	Clear();

	CmpackCatFile *handle = cmpack_cat_new();
	int res = cmpack_cat_make(handle, pht.m_Handle, pht.SelectedAperture());
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		cmpack_cat_destroy(handle);
		return false;
	}
	m_Handle = handle;
	InvalidateCache();
	return true;
}

//
// Make deep copy of a catalogue file
//
bool CCatalog::MakeCopy(const CCatalog &cat, GError **error)
{
	Clear();

	if (cat.m_Handle) {
		CmpackCatFile *handle = cmpack_cat_new();
		int res = cmpack_cat_copy(handle, cat.m_Handle);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			g_set_error(error, g_AppError, res, msg);
			cmpack_free(msg);
			cmpack_cat_destroy(handle);
			return false;
		}
		m_Handle = handle;
		m_CurrentSelection = cat.CurrentSelectionIndex();
		InvalidateCache();
	}
	return true;
}


//
// Make chart data
//
CmpackChartData *CCatalog::ToChartData(bool show_selection, bool show_tags, bool transparent)
{
	static const unsigned mask = CMPACK_OM_ID | CMPACK_OM_CENTER | CMPACK_OM_MAGNITUDE;

	gint count, width, height;
	CmpackCatObject obj;
	CmpackChartItem item;

	if (!m_Handle)
		return NULL;

	memset(&item, 0, sizeof(CmpackChartItem));
	
	CmpackChartData *data = cmpack_chart_data_new();
	width = cmpack_cat_get_width(m_Handle);
	height = cmpack_cat_get_height(m_Handle);
	cmpack_chart_data_set_dimensions(data, width, height);
	count = cmpack_cat_nstar(m_Handle);
	cmpack_chart_data_alloc(data, count);
	gdouble ref = GetRefMag();
	for (int i=0; i<count; i++) {
		cmpack_cat_get_star(m_Handle, i, mask, &obj);
		item.x = obj.center_x;
		item.y = obj.center_y;
		item.outline = !obj.refmag_valid;
		item.param = obj.id;
		if (!transparent)
			item.d = 2.0 + MAX(0, (obj.refmag_valid ? 1.0 + 0.75*(ref - obj.refmagnitude) : 1.0));
		else
			item.d = 0;
		cmpack_chart_data_add(data, &item, sizeof(item));
	}
	const CTags *tags = NULL;
	if (show_tags)
		tags = Tags();
	CSelection selection;
	if (show_selection && m_CurrentSelection>=0)
		selection = Selections()->At(m_CurrentSelection);
	if (tags) {
		int count = cmpack_chart_data_count(data);
		for (int row=0; row<count; row++) {
			int star_id = cmpack_chart_data_get_param(data, row);
			const gchar *tag = (tags ? tags->Get(star_id) : NULL);
			int i = selection.IndexOf(star_id);
			if (i>=0) {
				CmpackSelectionType type = selection.GetType(i);
				gchar *buf = (gchar*)g_malloc((256+(tag ? strlen(tag)+1 : 0))*sizeof(gchar));
				if (selection.GetIndex(i)==1)
					strcpy(buf, Types[type].label);
				else
					sprintf(buf, "%s #%d", Types[type].label, selection.GetIndex(i));
				if (tag) {
					strcat(buf, "\n");
					strcat(buf, tag);
				}
				cmpack_chart_data_set_tag(data, row, buf);
				cmpack_chart_data_set_color(data, row, Types[type].color);
				cmpack_chart_data_set_topmost(data, row, TRUE);
				if (transparent)
					cmpack_chart_data_set_diameter(data, row, 4.0);
				g_free(buf);
			} else if (tag) {
				cmpack_chart_data_set_tag(data, row, tag);
				cmpack_chart_data_set_color(data, row, CMPACK_COLOR_YELLOW);
				cmpack_chart_data_set_topmost(data, row, TRUE);
				if (transparent)
					cmpack_chart_data_set_diameter(data, row, 4.0);
			}
		}
	}
	return data;
}

//
// Make table model
//
GtkTreeModel *CCatalog::ToTreeModel(bool show_selection, bool show_tags)
{
	static const int mask = CMPACK_OM_ID | CMPACK_OM_MAGNITUDE | CMPACK_OM_CENTER;

	if (!m_Handle)
		return NULL;

	GtkTreeIter iter;

	const CTags *tags = NULL;
	if (show_tags)
		tags = Tags();
	CSelection selection;
	if (show_selection && m_CurrentSelection>=0)
		selection = Selections()->At(m_CurrentSelection);

	GtkListStore *data = gtk_list_store_new(COL_COUNT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, 
		G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_STRING, G_TYPE_STRING);
	int count = cmpack_cat_nstar(m_Handle);
	for (int i=0; i<count; i++) {
		CmpackCatObject obj;
		if (cmpack_cat_get_star(m_Handle, i, mask, &obj)==0) {
			gtk_list_store_append(data, &iter);
			gtk_list_store_set(data, &iter, COL_ID, obj.id, COL_POS_X, obj.center_x, 
				COL_POS_Y, obj.center_y, COL_MAG_VALID, obj.refmag_valid,
				COL_MAG, obj.refmagnitude, -1);
			int i = selection.IndexOf(obj.id);
			if (i>=0) {
				CmpackSelectionType type = selection.GetType(i);
				gchar buf[256];
				if (selection.GetIndex(i)==1)
					strcpy(buf, Types[type].label);
				else
					sprintf(buf, "%s #%d", Types[type].label, selection.GetIndex(i));
				gulong typidx = ((gulong)type << 16) | (selection.GetIndex(i) & 0xFFFF);
				gtk_list_store_set(data, &iter, COL_SEL_TYPIDX, typidx,
					COL_SEL_CAPTION, buf, COL_FG_COLOR, Types[type].color_name, -1);
			}
			if (tags) {
				const gchar *tag = tags->Get(obj.id);
				if (tag) 
					gtk_list_store_set(data, &iter, COL_TAGS, tag, -1);
			}
		}
	}
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(data);
	gtk_tree_sortable_set_sort_func(sortable, COL_MAG, (GtkTreeIterCompareFunc)CompareBrightness, 0, 0);
	return GTK_TREE_MODEL(data);
}

//
// Set graph view parameters
//
void CCatalog::SetView(GtkTreeView *view) const
{
	// Delete old columns
	GList *list = gtk_tree_view_get_columns(view);
	for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_tree_view_remove_column(view, (GtkTreeViewColumn*)(ptr->data));
	g_list_free(list);

	// Create new columns
	if (m_Handle) {
		AddViewCol_Int(view, COL_ID, 0, INT_MAX, "Obj. #", COL_ID);
		AddViewCol_Dbl(view, COL_POS_X, INT_MIN, INT_MAX, 2, "Center X", COL_POS_X);
		AddViewCol_Dbl(view, COL_POS_Y, INT_MIN, INT_MAX, 2, "Center Y", COL_POS_Y);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintBrightness, NULL, "Brightness [mag]", 1.0, COL_MAG);
		AddViewCol_Text(view, COL_SEL_CAPTION, "Selection", COL_SEL_TYPIDX);
		AddViewCol_Text(view, COL_TAGS, "Tag", COL_TAGS);
	}

	// Insert last empty column that will stretch to the rest of the table
	gtk_tree_view_insert_column_with_data_func(view, -1, NULL, gtk_cell_renderer_text_new(), NULL, NULL, NULL);
}

//
// Export list of objects to a file
//
bool CCatalog::ExportTable(const gchar *filepath, const gchar *format, unsigned flags, int sort_column_id,
	GtkSortType sort_order, GError **error)
{
	g_assert(m_Handle != NULL);
	g_assert(format != NULL);
	g_assert(filepath != NULL);

	if (strcmp(format, "text/csv")) {
		g_set_error(error, g_AppError, 0, "Unsupported file format (%s)", format);
		return false;
	}

	FILE *f = open_file(filepath, "w");
	if (!f) {
		g_set_error(error, g_AppError, 0, "Failed to create the file: %s", filepath);
		return false;
	}

	CCSVWriter *writer = new CCSVWriter(f, ',', '\"', (flags & EXPORT_NO_HEADER)!=0);
	if (writer) {
		ExportTable(*writer, flags, sort_column_id, sort_order);
		delete writer;
	}

	fclose(f);
	return true;
}

void CCatalog::ExportTable(CCSVWriter &writer, unsigned flags, int sort_column_id, GtkSortType sort_order)
{
	g_assert(m_Handle != NULL);

	CmpackCatObject obj;

	// Table header
	writer.AddColumn("Obj_ID");
	writer.AddColumn("CENTER_X");
	writer.AddColumn("CENTER_Y");
	writer.AddColumn("MAG");
	writer.AddColumn("SELECTION");
	writer.AddColumn("TAG");

	const CTags *tags = Tags();
	CSelection selection;
	if (m_CurrentSelection>=0)
		selection = Selections()->At(m_CurrentSelection);

	// Make list of indices and sort them using sort_column_id and sort_order
	int count = cmpack_cat_nstar(m_Handle);
	if (count>0) {
		tSortItem *items = (tSortItem*)g_malloc(count*sizeof(tSortItem));
		tSortType type = SORT_TYPE_INT;
		if (items) {
			switch (sort_column_id) 
			{
			case COL_ID:
			default:
				type = SORT_TYPE_INT;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_cat_get_star(m_Handle, i, CMPACK_OM_ID, &obj)==0) 
						items[i].value.i = obj.id;
					else
						items[i].value.i = -1;
				}
				break;
			case COL_POS_X:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_cat_get_star(m_Handle, i, CMPACK_OM_CENTER, &obj)==0) 
						items[i].value.d = obj.center_x;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_POS_Y:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_cat_get_star(m_Handle, i, CMPACK_OM_CENTER, &obj)==0) 
						items[i].value.d = obj.center_y;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_MAG:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_cat_get_star(m_Handle, i, CMPACK_OM_MAGNITUDE, &obj)==0 && obj.refmag_valid) 
						items[i].value.d = obj.refmagnitude;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			}
		}
		if (count>1)
			SortItems(items, count, type, sort_order);

		// Go through row indices and export them to the file
		for (int i=0; i<count; i++) {
			static const int mask = CMPACK_OM_ID | CMPACK_OM_CENTER | CMPACK_OM_MAGNITUDE;
			if (cmpack_cat_get_star(m_Handle, items[i].row, mask, &obj)==0) {
				bool show = 1;
				if (flags & EXPORT_SKIP_INVALID) 
					show &= (obj.refmag_valid!=0);
				if (show) {
					writer.Append();
					writer.SetInt(0, obj.id);
					writer.SetDbl(1, obj.center_x, 2);
					writer.SetDbl(2, obj.center_y, 2);
					if (obj.refmag_valid) 
						writer.SetDbl(3, obj.refmagnitude, 4);
					int i = selection.IndexOf(obj.id);
					if (i>=0) {
						CmpackSelectionType type = selection.GetType(i);
						gchar buf[256];
						if (selection.GetIndex(i)==1)
							strcpy(buf, Types[type].label);
						else
							sprintf(buf, "%s #%d", Types[type].label, selection.GetIndex(i));
						writer.SetStr(4, buf);
					}
					if (tags) {
						const gchar *tag = tags->Get(obj.id);
						if (tag)
							writer.SetStr(5, tag);
					}
				}
			}
		}
		// Free allocated memory
		if (type == SORT_TYPE_STRING) {
			for (int i=0; i<count; i++)
				cmpack_free(items[i].value.s);
		}
		g_free(items);
	}
}

//
// Export list of objects to a file
//
bool CCatalog::ExportHeader(const gchar *filepath, const gchar *format, unsigned flags, GError **error) const
{
	g_assert(m_Handle != NULL);
	g_assert(format != NULL);
	g_assert(filepath != NULL);

	if (strcmp(format, "text/csv")) {
		g_set_error(error, g_AppError, 0, "Unsupported file format (%s)", format);
		return false;
	}

	FILE *f = open_file(filepath, "w");
	if (!f) {
		g_set_error(error, g_AppError, 0, "Failed to create the file: %s", filepath);
		return false;
	}

	CCSVWriter *writer = new CCSVWriter(f, ',', '\"', (flags & EXPORT_NO_HEADER)!=0);
	if (writer) {
		ExportHeader(*writer, flags);
		delete writer;
	}

	fclose(f);
	return true;
}

//
// Export file header
//
void CCatalog::ExportHeader(CCSVWriter &writer, unsigned flags) const
{
	g_assert(m_Handle != NULL);

	// Table header
	writer.AddColumn("KEY");
	writer.AddColumn("VALUE");
	writer.AddColumn("COMMENT");

	const char *key, *val, *com;
	for (int i=0; GetParam(i, &key, &val, &com); i++) {
		writer.Append();
		if (key)
			writer.SetStr(0, key);
		if (val)
			writer.SetStr(1, val);
		if (com)
			writer.SetStr(2, com);
	}
}
