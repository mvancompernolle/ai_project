/**************************************************************

phot_class.cpp (C-Munipack project)
Photometry file class interface
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
#include <math.h>
#include <assert.h>

#include "phot_class.h"
#include "main.h"

//--------------------------   HELPER FUNCTIONS   ----------------------------------

static gchar *FromLocale(const char *str)
{
	if (str)
		return g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	return NULL;
}

struct tPhotPrintData
{
	CPhot::tColumnId m_col;
	double	m_min, m_max;
	int		m_prec;
	tPhotPrintData(CPhot::tColumnId col, double min, double max, int prec):m_col(col), 
		m_min(min), m_max(max), m_prec(prec) {}
};

static void DestroyPrintData(tPhotPrintData *data)
{
	delete data;
}

static void PrintInt(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int value;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, data->m_col, &value, -1);
	if (value>=data->m_min && value<=data->m_max) {
		sprintf(buf, "%d", value);
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "--", NULL);
}

static gint CompareInt(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, tPhotPrintData *data)
{
	gint value_a, value_b;
	gboolean valid_a, valid_b;

	gtk_tree_model_get(tree_model, a, data->m_col, &value_a, -1);
	gtk_tree_model_get(tree_model, b, data->m_col, &value_b, -1);
	valid_a = (value_a >= data->m_min && value_a <= data->m_max);
	valid_b = (value_b >= data->m_min && value_b <= data->m_max);
	if (!valid_a && !valid_b)
		return 0;
	else if (!valid_a)
		return 1;
	else if (!valid_b)
		return -1;
	else if (value_a < value_b)
		return -1;
	else if (value_a == value_b)
		return 0;
	else
		return 1;
}

static void PrintDouble(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	double value;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, data->m_col, &value, -1);
	if (value>=data->m_min && value<=data->m_max) {
		sprintf(buf, "%.*f", data->m_prec, value);
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareDbl(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, tPhotPrintData *data)
{
	gdouble value_a, value_b;
	gboolean valid_a, valid_b;

	gtk_tree_model_get(tree_model, a, data->m_col, &value_a, -1);
	gtk_tree_model_get(tree_model, b, data->m_col, &value_b, -1);
	valid_a = (value_a >= data->m_min && value_a <= data->m_max);
	valid_b = (value_b >= data->m_min && value_b <= data->m_max);

	if (!valid_a && !valid_b)
		return 0;
	else if (!valid_a)
		return 1;
	else if (!valid_b)
		return -1;
	else if (value_a < value_b)
		return -1;
	else if (value_a == value_b)
		return 0;
	else
		return 1;
}

static void PrintNetIntensity(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int mag_valid;
	double mag;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, CPhot::COL_MAG_VALID, &mag_valid, CPhot::COL_MAG, &mag, -1);
	if (mag_valid) {
		sprintf(buf, "%.1f", pow(10.0, (25.0-mag)/2.5));
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareNetIntensity(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tPhotPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double mag_a, mag_b;

	gtk_tree_model_get(tree_model, a, CPhot::COL_MAG_VALID, &mag_valid_a, CPhot::COL_MAG, &mag_a, -1);
	gtk_tree_model_get(tree_model, b, CPhot::COL_MAG_VALID, &mag_valid_b, CPhot::COL_MAG, &mag_b, -1);
	if (!mag_valid_a && !mag_valid_b)
		return 0;
	else if (!mag_valid_a)
		return 1;
	else if (!mag_valid_b)
		return -1;
	else if (mag_a > mag_b)
		return -1;
	else if (mag_a == mag_b)
		return 0;
	else
		return 1;
}

static void PrintNoise(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int mag_valid;
	double mag, err;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, CPhot::COL_MAG_VALID, &mag_valid, CPhot::COL_MAG, &mag, CPhot::COL_MAG_ERR, &err, -1);
	if (mag_valid) {
		sprintf(buf, "%.1f", err*pow(10.0, (25.0-mag)/2.5)/1.0857);
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareNoise(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tPhotPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double mag_a, mag_b, err_a, err_b;

	gtk_tree_model_get(tree_model, a, CPhot::COL_MAG_VALID, &mag_valid_a, CPhot::COL_MAG, &mag_a, CPhot::COL_MAG_ERR, &err_a, -1);
	gtk_tree_model_get(tree_model, b, CPhot::COL_MAG_VALID, &mag_valid_b, CPhot::COL_MAG, &mag_b, CPhot::COL_MAG_ERR, &err_b, -1);
	mag_valid_a = mag_valid_a && err_a>0;
	mag_valid_b = mag_valid_b && err_b>0;
	if (!mag_valid_a && !mag_valid_b)
		return 0;
	else if (!mag_valid_a)
		return 1;
	else if (!mag_valid_b)
		return -1;

	double noise_a = err_a*pow(10.0, (25.0-mag_a)/2.5), noise_b = err_b*pow(10.0, (25.0-mag_b)/2.5);
	if (noise_a < noise_b)
		return -1;
	else if (noise_a == noise_b)
		return 0;
	else
		return 1;
}

static void PrintSNRatio(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int mag_valid;
	double mag, err;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, CPhot::COL_MAG_VALID, &mag_valid, CPhot::COL_MAG, &mag, CPhot::COL_MAG_ERR, &err, -1);
	if (mag_valid && err>0) {
		sprintf(buf, "%.1f", -10.0*log10(1.0857/err));
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareSNRatio(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tPhotPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double err_a, err_b;

	gtk_tree_model_get(tree_model, a, CPhot::COL_MAG_VALID, &mag_valid_a, CPhot::COL_MAG_ERR, &err_a, -1);
	gtk_tree_model_get(tree_model, b, CPhot::COL_MAG_VALID, &mag_valid_b, CPhot::COL_MAG_ERR, &err_b, -1);
	mag_valid_a = mag_valid_a && err_a>0;
	mag_valid_b = mag_valid_b && err_b>0;
	if (!mag_valid_a && !mag_valid_b)
		return 0;
	else if (!mag_valid_a)
		return 1;
	else if (!mag_valid_b)
		return -1;
	else if (err_a < err_b)
		return -1;
	else if (err_a == err_b)
		return 0;
	else
		return 1;
}

static void PrintBrightness(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int mag_valid;
	double mag;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, CPhot::COL_MAG_VALID, &mag_valid, CPhot::COL_MAG, &mag, -1);
	if (mag_valid) {
		sprintf(buf, "%.4f", mag);
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareBrightness(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tPhotPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double mag_a, mag_b;

	gtk_tree_model_get(tree_model, a, CPhot::COL_MAG_VALID, &mag_valid_a, CPhot::COL_MAG, &mag_a, -1);
	gtk_tree_model_get(tree_model, b, CPhot::COL_MAG_VALID, &mag_valid_b, CPhot::COL_MAG, &mag_b, -1);
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

static void PrintError(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, tPhotPrintData *data)
{
	int mag_valid;
	double err;
	char buf[256];

	gtk_tree_model_get(tree_model, iter, CPhot::COL_MAG_VALID, &mag_valid, CPhot::COL_MAG_ERR, &err, -1);
	if (mag_valid && err>0) {
		sprintf(buf, "%.4f", err);
		g_object_set(cell, "text", buf, NULL);
	} else 
		g_object_set(cell, "text", "------", NULL);
}

static gint CompareError(GtkTreeModel *tree_model, GtkTreeIter *a, GtkTreeIter *b, 
	tPhotPrintData *data)
{
	int mag_valid_a, mag_valid_b;
	double err_a, err_b;

	gtk_tree_model_get(tree_model, a, CPhot::COL_MAG_VALID, &mag_valid_a, CPhot::COL_MAG_ERR, &err_a, -1);
	gtk_tree_model_get(tree_model, b, CPhot::COL_MAG_VALID, &mag_valid_b, CPhot::COL_MAG_ERR, &err_b, -1);
	mag_valid_a = mag_valid_a && err_a>0;
	mag_valid_b = mag_valid_b && err_b>0;
	if (!mag_valid_a && !mag_valid_b)
		return 0;
	else if (!mag_valid_a)
		return 1;
	else if (!mag_valid_b)
		return -1;
	else if (err_a < err_b)
		return -1;
	else if (err_a == err_b)
		return 0;
	else
		return 1;
}

//--------------------------   HELPER FUNCTIONS   ----------------------------------

static void AddViewCol(GtkTreeView *view, GtkTreeCellDataFunc print_fn, tPhotPrintData *print_data, 
	const gchar *name, double xalign, int sort_column_id)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", xalign, NULL);
	int index = gtk_tree_view_insert_column_with_data_func(view, -1, name, renderer, print_fn, print_data, (GDestroyNotify)DestroyPrintData)-1;
	GtkTreeViewColumn *col = gtk_tree_view_get_column(view, index);
	g_object_set_data(G_OBJECT(col), "sort-column-id", GINT_TO_POINTER(sort_column_id));
}

static void AddViewCol_Int(GtkTreeView *view, CPhot::tColumnId column, double min, double max, const gchar *name,
	int sort_column_id)
{
	AddViewCol(view, (GtkTreeCellDataFunc)PrintInt, new tPhotPrintData(column, min, max, 0), name, 
		1.0, sort_column_id);
}

static void AddViewCol_Dbl(GtkTreeView *view, CPhot::tColumnId column, double min, double max, int prec, 
	const gchar *name, int sort_column_id)
{
	AddViewCol(view, (GtkTreeCellDataFunc)PrintDouble, new tPhotPrintData(column, min, max, prec), name, 
		1.0, sort_column_id);
}

static void AddViewCol_Text(GtkTreeView *view, CPhot::tColumnId column, const gchar *name, int sort_column_id)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	int index = gtk_tree_view_insert_column_with_attributes(view, -1, name, renderer, "text", column, NULL)-1;
	GtkTreeViewColumn *col = gtk_tree_view_get_column(view, index);
	g_object_set_data(G_OBJECT(col), "sort-column-id", GINT_TO_POINTER(sort_column_id));
}

static void AddSortCol_Int(GtkTreeSortable *sortable, int sort_column_id, CPhot::tColumnId column, double min, double max)
{
	gtk_tree_sortable_set_sort_func(sortable, sort_column_id, (GtkTreeIterCompareFunc)CompareInt,
		new tPhotPrintData(column, min, max, 0), (GDestroyNotify)DestroyPrintData);
}

static void AddSortCol_Dbl(GtkTreeSortable *sortable, int sort_column_id, CPhot::tColumnId column, double min, double max)
{
	gtk_tree_sortable_set_sort_func(sortable, sort_column_id, (GtkTreeIterCompareFunc)CompareDbl,
		new tPhotPrintData(column, min, max, 0), (GDestroyNotify)DestroyPrintData);
}

//--------------------------   PHOTOMETRY FILE  ----------------------------------

//
// Destructor
//
CPhot::~CPhot()
{
	if (m_Handle)
		cmpack_pht_destroy(m_Handle);
	g_free(m_Filter);
}

//
// Clear data
//
void CPhot::Clear()
{
	if (m_Handle) {
		cmpack_pht_destroy(m_Handle);
		m_Handle = NULL;
	}
	m_Apertures.Clear();
	m_CurrentAperture = -1;
	InvalidateCache();
}

//
// Set image info from a file
//
bool CPhot::Load(const gchar *fpath, GError **error)
{
	g_assert(fpath != NULL);

	Clear();

	CmpackPhtFile *handle;
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_pht_open(&handle, f, CMPACK_OPEN_READONLY, 0);
	g_free(f);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		return false;
	}

	m_Handle = cmpack_pht_init();
	cmpack_pht_copy(m_Handle, handle);
	cmpack_pht_destroy(handle);
	InvalidateCache();
	return true;
}

//
// Save catalog file 
//
bool CPhot::SaveAs(const gchar *fpath, GError **error) const
{
	assert (m_Handle != NULL);

	CmpackPhtFile *file;
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_pht_open(&file, f, CMPACK_OPEN_CREATE, 0);
	g_free(f);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		return false;
	}

	res = cmpack_pht_copy(file, m_Handle);
	if (res==0)
		res = cmpack_pht_close(file);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		cmpack_pht_destroy(file);
		return false;
	}
	return res==0;
}

//
// Get chart width in pixels
//
void CPhot::InvalidateCache(void)
{
	m_RefMag = 0;
	g_free(m_Filter);
	m_Filter = NULL;
	m_CacheFlags = 0;
}

//
// Get number of stars
//
int CPhot::ObjectCount(void) const
{
	if (m_Handle)
		return cmpack_pht_object_count(m_Handle);
	return 0;
}

//
// Get object identifier
//
int CPhot::GetObjectID(int index)
{
	CmpackPhtObject obj;

	if (m_Handle && cmpack_pht_get_object(m_Handle, index, CMPACK_PO_ID, &obj)==0)
		return obj.id;
	return -1;
}

//
// Get object reference identifier
//
int CPhot::GetObjectRefID(int index)
{
	CmpackPhtObject obj;

	if (m_Handle && cmpack_pht_get_object(m_Handle, index, CMPACK_PO_REF_ID, &obj)==0)
		return obj.ref_id;
	return -1;
}

//
// Get object position
//
bool CPhot::GetObjectPos(int index, double *x, double *y)
{
	CmpackPhtObject obj;

	if (m_Handle && cmpack_pht_get_object(m_Handle, index, CMPACK_PO_CENTER, &obj)==0) {
		if (x)
			*x = obj.x;
		if (y)
			*y = obj.y;
		return true;
	}	
	return false;
}

//
// Get star attributes
//
bool CPhot::GetObjectParams(int index, unsigned mask, CmpackPhtObject *obj)
{
	return m_Handle && cmpack_pht_get_object(m_Handle, index, mask, obj)==0;
}

//
// Get object measurement
//
bool CPhot::GetMagnitude(int star_index, double *mag, double *err)
{
	CmpackPhtData data;

	if (m_Handle && m_CurrentAperture>=0) { 
		if (cmpack_pht_get_data(m_Handle, star_index, m_CurrentAperture, &data)==0 && data.mag_valid) {
			if (mag)
				*mag = data.magnitude;
			if (err)
				*err = data.mag_error;
			return true;
		}
	}
	return false;
}

//
// Get object measurement
//
bool CPhot::GetMagnitudeAndCode(int star_index, CmpackPhtData &data, CmpackError &code)
{
	if (m_Handle && m_CurrentAperture>=0) 
		return cmpack_pht_get_data_with_code(m_Handle, star_index, m_CurrentAperture, &data, &code)==0;
	return false;
}

//
// Update reference magnitudes
//
double CPhot::GetRefMag(void)
{
	if (!(m_CacheFlags & CF_REFMAG)) {
		m_RefMag = 0.0;
		if (m_Handle && m_CurrentAperture>=0) {
			int nstars = cmpack_pht_object_count(m_Handle);
			if (nstars>0) {
				double *maglist = (double*)g_malloc(nstars*sizeof(double));
				int count = 0;
				for (int i=0; i<nstars; i++) {
					double mag;
					if (GetMagnitude(i, &mag, NULL))
						maglist[count++] = mag;
				}
				if (count>0) 
					cmpack_robustmean(count, maglist, &m_RefMag, NULL);
				g_free(maglist);
			}
		}
		m_CacheFlags |= CF_REFMAG;
	}
	return m_RefMag;
}

//
// Number of apertures
//
int CPhot::ApertureCount(void) const
{
	if (m_Handle)
		return cmpack_pht_aperture_count(m_Handle);
	return 0;
}

//
// Selected aperture
//
void CPhot::SelectAperture(int apertureIndex)
{
	if (m_CurrentAperture!=apertureIndex) {
		m_CurrentAperture = apertureIndex;
		m_CacheFlags &= ~CF_REFMAG;
	}
}

//
// Update reference magnitudes
//
const CApertures *CPhot::Apertures(void)
{
	int i, count;
	CmpackPhtAperture aper;

	if (!(m_CacheFlags & CF_APERTURES)) {
		m_Apertures.Clear();
		if (m_Handle) {
			count = cmpack_pht_aperture_count(m_Handle);
			for (i=0; i<count; i++) {
				cmpack_pht_get_aperture(m_Handle, i, CMPACK_PA_ID | CMPACK_PA_RADIUS, &aper);
				m_Apertures.Add(CAperture(aper.id, aper.radius));
			}
		}
		m_CacheFlags |= CF_APERTURES;
	}
	return &m_Apertures;
}

//
// Update reference magnitudes
//
bool CPhot::GetParams(unsigned mask, CmpackPhtInfo &info) const
{
	if (m_Handle)
		return cmpack_pht_get_info(m_Handle, mask, &info)==0;
	return false;
}

//
// Optical filter
//
const char *CPhot::Filter(void)
{
	CmpackPhtInfo info;

	if (!(m_CacheFlags & CF_FILTER)) {
		g_free(m_Filter);
		if (m_Handle && cmpack_pht_get_info(m_Handle, CMPACK_PI_FRAME_PARAMS, &info)==0)
			m_Filter = FromLocale(info.filter);
		else
			m_Filter = NULL;
		m_CacheFlags |= CF_FILTER;
	}
	return m_Filter;
}

//
// Julian date
//
double CPhot::JulianDate(void) const
{
	CmpackPhtInfo info;
	if (m_Handle && GetParams(CMPACK_PI_JD, info))
		return info.jd;
	return 0.0;
}

//
// Exposure duration
//
double CPhot::ExposureDuration(void) const
{
	CmpackPhtInfo info;
	if (m_Handle && GetParams(CMPACK_PI_FRAME_PARAMS, info))
		return info.exptime;
	return -1.0;
}

//
// Has the file been matched?
//
bool CPhot::Matched(void) const
{
	CmpackPhtInfo info;

	if (m_Handle && GetParams(CMPACK_PI_MATCH_PARAMS, info))
		return info.matched!=0;
	return false;
}

//
// Find aperture
//
int CPhot::FindAperture(int id) const
{
	if (m_Handle)
		return cmpack_pht_find_aperture(m_Handle, id);
	return -1;
}

int CPhot::FindObject(int obj_id)
{
	if (m_Handle)
		return cmpack_pht_find_object(m_Handle, obj_id, 0);
	return -1;
}

int CPhot::FindObjectRefID(int ref_id)
{
	if (m_Handle)
		return cmpack_pht_find_object(m_Handle, ref_id, 1);
	return -1;
}

//
// Deep copy from another photometry file
//
bool CPhot::MakeCopy(const CPhot &phot, GError **error)
{
	Clear();

	if (phot.m_Handle) {
		CmpackPhtFile *handle = cmpack_pht_init();
		int res = cmpack_pht_copy(handle, phot.m_Handle);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			g_set_error(error, g_AppError, res, msg);
			cmpack_free(msg);
			cmpack_pht_destroy(handle);
			return false;
		}
		m_Handle = handle;
		m_CurrentAperture = phot.SelectedAperture();
		InvalidateCache();
	}
	return true;
}

//
// Make chart data
//
CmpackChartData *CPhot::ToChartData(bool gray_not_matched, bool transparent)
{
	CmpackPhtInfo info;
	CmpackPhtObject obj;
	CmpackPhtData mag;
	CmpackChartItem item;

	if (!m_Handle || m_CurrentAperture<0)
		return NULL;

	memset(&item, 0, sizeof(CmpackChartItem));

	CmpackChartData *data = cmpack_chart_data_new();
	cmpack_pht_get_info(m_Handle, CMPACK_PI_FRAME_PARAMS, &info);
	cmpack_chart_data_set_dimensions(data, info.width, info.height);
	int count = cmpack_pht_object_count(m_Handle);
	cmpack_chart_data_alloc(data, count);
	double ref = GetRefMag();
	for (int i=0; i<count; i++) {
		cmpack_pht_get_object(m_Handle, i, CMPACK_PO_ID | CMPACK_PO_REF_ID | CMPACK_PO_CENTER, &obj);
		item.x = obj.x;
		item.y = obj.y;
		item.param = obj.id;
		item.color = ((gray_not_matched && obj.ref_id<0) ? CMPACK_COLOR_GRAY : CMPACK_COLOR_DEFAULT);
		if (transparent) {
			item.outline = false;
			item.d = 0;
		} else {
			if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid) {
				item.outline = false;
				item.d = 2.0 + MAX(0, 1.0 + 0.75*(ref - mag.magnitude));
			} else {
				item.outline = true;
				item.d = 2.0 + 1.0;
			}
		}
		cmpack_chart_data_add(data, &item, sizeof(item));
	}
	return data;
}

//
// Make table model
//
GtkTreeModel *CPhot::ToTreeModel(void)
{
	static const int mask = CMPACK_PO_ID | CMPACK_PO_REF_ID | CMPACK_PO_CENTER | 
		CMPACK_PO_FWHM | CMPACK_PO_SKY;

	CmpackPhtData mag;
	CmpackPhtObject obj;
	GtkTreeIter iter;
	CmpackError code;

	if (!m_Handle || m_CurrentAperture<0)
		return NULL;

	GtkListStore *data = gtk_list_store_new(COL_COUNT, G_TYPE_INT, G_TYPE_INT, 
		G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, 
		G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_STRING);
	int count = cmpack_pht_object_count(m_Handle);
	for (int i=0; i<count; i++) {
		if (cmpack_pht_get_object(m_Handle, i, mask, &obj)==0) {
			gtk_list_store_append(data, &iter);
			gtk_list_store_set(data, &iter, COL_ID, obj.id, COL_REF_ID, obj.ref_id,
				COL_POS_X, obj.x, COL_POS_Y, obj.y, COL_FWHM, obj.fwhm, COL_SKY,
				obj.skymed, COL_SKY_DEV, obj.skysig, -1);
			if (cmpack_pht_get_data_with_code(m_Handle, i, m_CurrentAperture, &mag, &code)==0) {
				gtk_list_store_set(data, &iter, COL_MAG_VALID, mag.mag_valid,
					COL_MAG, mag.magnitude, COL_MAG_ERR, mag.mag_error, COL_CODE, code, -1);
				if (code!=0) {
					char *msg = cmpack_formaterror(code);
					gtk_list_store_set(data, &iter, COL_REMARKS, msg, -1);
					cmpack_free(msg);
				}
			}
		}
	}
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(data);
	AddSortCol_Int(sortable, COL_REF_ID, COL_REF_ID, 0, INT_MAX);
	AddSortCol_Dbl(sortable, COL_FWHM, COL_FWHM, DBL_MIN, DBL_MAX);
	gtk_tree_sortable_set_sort_func(sortable, SORT_INT, (GtkTreeIterCompareFunc)CompareNetIntensity, 0, 0);
	gtk_tree_sortable_set_sort_func(sortable, SORT_NOISE, (GtkTreeIterCompareFunc)CompareNoise, 0, 0);
	gtk_tree_sortable_set_sort_func(sortable, SORT_SNR, (GtkTreeIterCompareFunc)CompareSNRatio, 0, 0);
	gtk_tree_sortable_set_sort_func(sortable, COL_MAG, (GtkTreeIterCompareFunc)CompareBrightness, 0, 0);
	gtk_tree_sortable_set_sort_func(sortable, COL_MAG_ERR, (GtkTreeIterCompareFunc)CompareError, 0, 0);
	return GTK_TREE_MODEL(data);
}

//
// Set graph view parameters
//
void CPhot::SetView(GtkTreeView *view)
{
	// Delete old columns
	GList *list = gtk_tree_view_get_columns(view);
	for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_tree_view_remove_column(view, (GtkTreeViewColumn*)(ptr->data));
	g_list_free(list);

	// Create new columns
	if (m_Handle) {
		AddViewCol_Int(view, COL_ID, 0, INT_MAX, "Obj. #", COL_ID);
		AddViewCol_Int(view, COL_REF_ID, 0, INT_MAX, "Ref. #", COL_REF_ID);
		AddViewCol_Dbl(view, COL_POS_X, INT_MIN, INT_MAX, 2, "Center X", COL_POS_X);
		AddViewCol_Dbl(view, COL_POS_Y, INT_MIN, INT_MAX, 2, "Center Y", COL_POS_Y);
		AddViewCol_Dbl(view, COL_FWHM, DBL_MIN, DBL_MAX, 2, "FWHM [pxl]", COL_FWHM);
		AddViewCol_Dbl(view, COL_SKY, -DBL_MAX, DBL_MAX, 2, "Sky [ADU]", COL_SKY);
		AddViewCol_Dbl(view, COL_SKY_DEV, DBL_MIN, DBL_MAX, 2, "Sky dev. [ADU]", COL_SKY_DEV);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintNetIntensity, NULL, "Net intensity [ADU]", 1.0, SORT_INT);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintNoise, NULL, "Noise [ADU]", 1.0, SORT_NOISE);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintSNRatio, NULL, "S/N ratio [dB]", 1.0, SORT_SNR);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintBrightness, NULL, "Brightness [mag]", 1.0, COL_MAG);
		AddViewCol(view, (GtkTreeCellDataFunc)PrintError, NULL, "Error [mag]", 1.0, COL_MAG_ERR);
		AddViewCol_Text(view, COL_REMARKS, "Status", COL_CODE);
	}

	// Insert last empty column that will stretch to the rest of the table
	gtk_tree_view_insert_column_with_data_func(view, -1, NULL, gtk_cell_renderer_text_new(), NULL, NULL, NULL);
}


//
// Export list of objects to a file
//
bool CPhot::ExportTable(const gchar *filepath, const gchar *format, unsigned flags, int sort_column_id,
	GtkSortType sort_order, GError **error) const
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

void CPhot::ExportTable(CCSVWriter &writer, unsigned flags, int sort_column_id, GtkSortType sort_order) const
{
	CmpackPhtObject obj;
	CmpackPhtData mag;
	CmpackError code;

	g_assert(m_Handle != NULL);

	// Table header
	writer.AddColumn("Obj_ID");
	writer.AddColumn("REF_ID");
	writer.AddColumn("CENTER_X");
	writer.AddColumn("CENTER_Y");
	writer.AddColumn("FWHM");
	writer.AddColumn("SKY");
	writer.AddColumn("SKYDEV");
	writer.AddColumn("NETINT");
	writer.AddColumn("NOISE");
	writer.AddColumn("SNRATIO");
	writer.AddColumn("MAG");
	writer.AddColumn("MAGERR");
	writer.AddColumn("STATUS");

	// Make list of indices and sort them using sort_column_id and sort_order
	int count = cmpack_pht_object_count(m_Handle);
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
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_ID, &obj)==0) 
						items[i].value.i = obj.id;
					else
						items[i].value.i = -1;
				}
				break;
			case COL_REF_ID:
				type = SORT_TYPE_INT;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_REF_ID, &obj)==0 && obj.ref_id>=0) 
						items[i].value.i = obj.ref_id;
					else
						items[i].value.i = INT_MAX;
				}
				break;
			case COL_POS_X:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_CENTER, &obj)==0) 
						items[i].value.d = obj.x;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_POS_Y:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_CENTER, &obj)==0) 
						items[i].value.d = obj.y;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_FWHM:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_FWHM, &obj)==0 && obj.fwhm>0) 
						items[i].value.d = obj.fwhm;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_SKY:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_SKY, &obj)==0) 
						items[i].value.d = obj.skymed;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_SKY_DEV:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_object(m_Handle, i, CMPACK_PO_SKY, &obj)==0 && obj.skysig>0) 
						items[i].value.d = obj.skysig;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_MAG:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid) 
						items[i].value.d = mag.magnitude;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case SORT_INT:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid) 
						items[i].value.d = -mag.magnitude;	// Reverse
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_MAG_ERR:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid && mag.mag_error>0) 
						items[i].value.d = mag.mag_error;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case SORT_SNR:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid && mag.mag_error>0) 
						items[i].value.d = mag.mag_error;
					else
						items[i].value.d = DBL_MAX;
				}
				break;
			case COL_REMARKS:
			case COL_CODE:
				type = SORT_TYPE_INT;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_code(m_Handle, i, m_CurrentAperture, &code)==0 && code!=0) 
						items[i].value.i = code;
					else
						items[i].value.i = 0;
				}
				break;
			case SORT_NOISE:
				type = SORT_TYPE_DOUBLE;
				for (int i=0; i<count; i++) {
					items[i].row = i;
					if (cmpack_pht_get_data(m_Handle, i, m_CurrentAperture, &mag)==0 && mag.mag_valid && mag.mag_error>0) 
						items[i].value.d = mag.mag_error*pow(10.0, (25.0-mag.magnitude)/2.5);
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
			static const int mask = CMPACK_PO_ID | CMPACK_PO_REF_ID | CMPACK_PO_CENTER | 
				CMPACK_PO_FWHM | CMPACK_PO_SKY;
			if (cmpack_pht_get_object(m_Handle, items[i].row, mask, &obj)==0) {
				bool show = true;
				if (flags & EXPORT_SKIP_INVALID) 
					show &= cmpack_pht_get_data_with_code(m_Handle, items[i].row, m_CurrentAperture, &mag, &code)==0 && mag.mag_valid;
				if (flags & EXPORT_SKIP_UNMATCHED) 
					show &= (obj.ref_id>0);
				if (show) {
					writer.Append();
					writer.SetInt(0, obj.id);
					if (obj.ref_id>0)
						writer.SetInt(1, obj.ref_id);
					writer.SetDbl(2, obj.x, 2);
					writer.SetDbl(3, obj.y, 2);
					if (obj.fwhm>0)
						writer.SetDbl(4, obj.fwhm, 2);
					writer.SetDbl(5, obj.skymed, 2);
					if (obj.skysig>0)
						writer.SetDbl(6, obj.skysig, 2);
					if (cmpack_pht_get_data_with_code(m_Handle, items[i].row, m_CurrentAperture, &mag, &code)==0) {
						if (mag.mag_valid) {
							writer.SetDbl(7, pow(10.0, (25.0-mag.magnitude)/2.5), 1);
							writer.SetDbl(10, mag.magnitude, 4);
							if (mag.mag_error>0) {
								writer.SetDbl(8, mag.mag_error*pow(10.0, (25.0-mag.magnitude)/2.5)/1.0857, 1);
								writer.SetDbl(9, -10.0*log10(1.0857/mag.mag_error), 1);
								writer.SetDbl(11, mag.mag_error);
							}
						}
						writer.SetInt(12, code);
					}
				}
			}
		}
		g_free(items);
	}
}

//
// Export list of objects to a file
//
bool CPhot::ExportHeader(const gchar *filepath, const gchar *format, unsigned flags, GError **error) const
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
void CPhot::ExportHeader(CCSVWriter &writer, unsigned flags) const
{
	CmpackDateTime dt;
	CmpackPhtInfo info;
	const struct tm *t;
	char datestr[64], timestr[64], actdate[256];

	g_assert(m_Handle != NULL);

	GetParams(CMPACK_PI_FRAME_PARAMS | CMPACK_PI_ORIGIN_CRDATE | CMPACK_PI_PHOT_PARAMS, info);
	ExportHeaderItem(writer, "width", info.width, "Image width in pixels");
	ExportHeaderItem(writer, "height", info.height, "Image height in pixels");
	ExportHeaderItem(writer, "jd", info.jd, 12, "Julian date of observation");
	cmpack_decodejd(info.jd, &dt);
	cmpack_datetostr(&dt.date, datestr, 64);
	ExportHeaderItem(writer, "date", datestr, "Date of observation");
	cmpack_timetostr(&dt.time, timestr, 64);
	ExportHeaderItem(writer, "time", timestr, "Time of observation");
	ExportHeaderItem(writer, "filter", info.filter, "Optical filter");
	ExportHeaderItem(writer, "exptime", info.exptime, 6, "Exposure duration in seconds");
	if (info.ccdtemp>-900 && info.ccdtemp<900)
		ExportHeaderItem(writer, "ccdtemp", info.ccdtemp, 6, "CCD temperature in deg.C");
	ExportHeaderItem(writer, "origin", info.origin, "Reduction software");
	t = &info.crtime;
	sprintf(actdate, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year+1900, 
		t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	ExportHeaderItem(writer, "crdate", actdate, "Date of creation");

	// Photometry process information 
	ExportHeaderItem(writer, "phot_stars", ObjectCount(), "Number of stars");
	ExportHeaderItem(writer, "phot_apertures", ApertureCount(), "Number of apertures");
	ExportHeaderItem(writer, "phot_datalo", info.range[0], 6, "Lowest good pixel value");
	ExportHeaderItem(writer, "phot_datahi", info.range[1], 6, "Highest good pixel value");
	ExportHeaderItem(writer, "phot_gain", info.gain, 6, "Gain in electrons per ADU");
	ExportHeaderItem(writer, "phot_rnoise", info.rnoise, 6, "Readout noise in ADU");
	ExportHeaderItem(writer, "phot_fwhm_exp", info.fwhm_exp, 6, "Expected FWHM of objects");
	ExportHeaderItem(writer, "phot_fwhm_mean", info.fwhm_mean, 6, "Mean FWHM of objects");
	ExportHeaderItem(writer, "phot_fwhm_err", info.fwhm_err, 6, "Std. error of FWHM of objects");
	ExportHeaderItem(writer, "phot_thresh", info.threshold, 6, "Detection threshold");
	ExportHeaderItem(writer, "phot_losharp", info.sharpness[0], 6, "Low sharpness cutoff");
	ExportHeaderItem(writer, "phot_hisharp", info.sharpness[1], 6, "High sharpness cutoff");
	ExportHeaderItem(writer, "phot_loround", info.roundness[0], 6, "Low roundness cutoff");
	ExportHeaderItem(writer, "phot_hiround", info.roundness[1], 6, "High roundness cutoff");
}

void CPhot::ExportHeaderItem(CCSVWriter &writer, const gchar *keyword, const gchar *value, const gchar *comment) const
{
	writer.Append();
	writer.SetStr(0, keyword);
	if (value)
		writer.SetStr(1, value);
	if (comment)
		writer.SetStr(2, comment);
}

void CPhot::ExportHeaderItem(CCSVWriter &writer, const gchar *keyword, int value, const gchar *comment) const
{
	writer.Append();
	writer.SetStr(0, keyword);
	writer.SetInt(1, value);
	if (comment)
		writer.SetStr(2, comment);
}

void CPhot::ExportHeaderItem(CCSVWriter &writer, const gchar *keyword, double value, int prec, const gchar *comment) const
{
	writer.Append();
	writer.SetStr(0, keyword);
	writer.SetDbl(1, value, prec);
	if (comment)
		writer.SetStr(2, comment);
}
