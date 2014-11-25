/**************************************************************

table_class.cpp (C-Munipack project)
Table class interface
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

#include "table_class.h"
#include "utils.h"
#include "main.h"

#define ALLOC_BY 64

//--------------------------   HELPER FUNCTIONS   ----------------------------------

static gchar *FromLocale(const char *str)
{
	if (str)
		return g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	return NULL;
}

class CTablePrinter
{
public:
	CTablePrinter(int col):m_col(col) {}
	virtual ~CTablePrinter() {}

	static void print(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
		GtkTreeModel *tree_model, GtkTreeIter *iter, CTablePrinter *data) 
	{
		char buf[256];
		buf[0] = '\0';
		data->fprint(tree_model, iter, buf);
		g_object_set(cell, "text", buf, NULL);
	}

	static void free(CTablePrinter *data) { delete data; }

protected:
	int m_col;

	virtual void fprint(GtkTreeModel *tree_model, GtkTreeIter *iter, char *buf) = 0;
};

class CTablePrintInt:public CTablePrinter
{
public:
	CTablePrintInt(int col, int min, int max):CTablePrinter(col), m_min(min), m_max(max) {}

private:
	int m_min, m_max;

protected:
	virtual void fprint(GtkTreeModel *tree_model, GtkTreeIter *iter, char *buf)
	{
		int value;
		gtk_tree_model_get(tree_model, iter, m_col, &value, -1);
		if (value>=m_min && value<=m_max)
			sprintf(buf, "%d", value);
		else
			sprintf(buf, "--");
	}
};

class CTablePrintDbl:public CTablePrinter
{
public:
	CTablePrintDbl(int col, double min, double max, int prec):CTablePrinter(col), m_min(min), 
		m_max(max), m_prec(prec) {}

private:
	double m_min, m_max;
	int m_prec;

protected:
	virtual void fprint(GtkTreeModel *tree_model, GtkTreeIter *iter, char *buf)
	{
		double value;
		gtk_tree_model_get(tree_model, iter, m_col, &value, -1);
		if (value>=m_min && value<=m_max) 
			sprintf(buf, "%.*f", m_prec, value);
		else
			sprintf(buf, "------");
	}
};

class CTablePrintUTC:public CTablePrinter
{
public:
	CTablePrintUTC(int col):CTablePrinter(col) {}

protected:
	virtual void fprint(GtkTreeModel *tree_model, GtkTreeIter *iter, char *buf)
	{
		double jd;
		CmpackDateTime dt;
		gtk_tree_model_get(tree_model, iter, m_col, &jd, -1);
		if (jd>=JD_MIN && jd<=JD_MAX && cmpack_decodejd(jd, &dt)==0) {
			sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
				dt.time.hour, dt.time.minute, dt.time.second);
		} else 
			sprintf(buf, "------");
	}
};

static void CheckLimits(double *a, double *b, double eps, double min, double max)
{
	g_assert(a != NULL && b != NULL);

	*a = LimitValue(*a, min, max);
	*b = LimitValue(*b, min, max);
	if ((*b-*a)<eps && (max-min)>=eps) {
		double z = 0.5 * (*a + *b);
		double g = z - (z - min)/(max - min)*eps;
		g = LimitValue(g, MAX(min, *b-eps), MIN(*a, max-eps));
		*a = g;
		*b = g + eps;
	}
}

static void AddViewCol(GtkTreeView *view, CTablePrinter *print_data, const gchar *name, double xalign)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", xalign, NULL);
	
	gchar *aux = g_strdup(name);
	for (gchar *ptr=aux; *ptr!=0; ptr++) {
		if (*ptr=='_')
			*ptr = ' ';
	}
	gtk_tree_view_insert_column_with_data_func(view, -1, aux, renderer, (GtkTreeCellDataFunc)CTablePrinter::print, 
		print_data, (GDestroyNotify)CTablePrinter::free);
	g_free(aux);
}

static void AddViewCol_Int(GtkTreeView *view, int column, int min, int max, const gchar *name)
{
	AddViewCol(view, new CTablePrintInt(column, min, max), name, 1.0);
}

static void AddViewCol_Dbl(GtkTreeView *view, int column, double min, double max, int prec, const gchar *name)
{
	AddViewCol(view, new CTablePrintDbl(column, min, max, prec), name, 1.0);
}

static void AddViewCol_UTC(GtkTreeView *view, int column, const gchar *name)
{
	AddViewCol(view, new CTablePrintUTC(column), name, 1.0);
}

//-----------------------   LIST OF CHANNELS   ---------------------

// Constructor with initialization
CChannel::CChannel(CTable *tab, int ycol, int ucol, tChannelInfo info):m_Table(tab), 
	m_Column(ycol), m_ColumnU(ucol), m_Name(NULL), m_Exported(true), m_Info(info),
	m_DataType(CMPACK_TYPE_VOID), m_Precision(0), m_ExportFlags(0)
{
	CmpackTabColumn iinfo;

	g_assert(m_Table != NULL && m_Column >= 0);

	cmpack_tab_get_column(m_Table->m_Handle, m_Column, CMPACK_TM_NAME | CMPACK_TM_TYPE_PREC, &iinfo);
	if (iinfo.name)
		m_Name = FromLocale(iinfo.name);
	else
		m_Name = g_strdup("");
	m_DataType = iinfo.dtype;
	m_Precision = iinfo.prec;
}

// Destructor
CChannel::~CChannel(void)
{
	g_free(m_Name);
}

const gchar *CChannel::Unit(void) const
{
	switch (m_Info)
	{
	case DATA_MAGNITUDE:
	case DATA_DEVIATION:
		return "mag";
	case DATA_HELCOR:
		return "days";
	case DATA_OFFSET:
		return "px";
	case DATA_ALTITUDE:
		return "deg";
	default:
		return NULL;
	}
}

// Get minimum value
double CChannel::Min(void) const
{
	g_assert(m_Table != NULL && m_Column >= 0);

	CmpackTabColumn info;
	cmpack_tab_get_column(m_Table->m_Handle, m_Column, CMPACK_TM_RANGE, &info);
	if (info.range_valid)
		return info.range_min;
	return 0.0;
}

// Get minimum value
double CChannel::Max(void) const
{
	g_assert(m_Table != NULL && m_Column >= 0);

	CmpackTabColumn info;
	cmpack_tab_get_column(m_Table->m_Handle, m_Column, CMPACK_TM_RANGE, &info);
	if (info.range_valid)
		return info.range_max;
	return 0.0;
}

// Get minimum value
double CChannel::MaxU(void) const
{
	g_assert(m_Table != NULL && m_ColumnU >= 0);

	CmpackTabColumn info;
	cmpack_tab_get_column(m_Table->m_Handle, m_ColumnU, CMPACK_TM_RANGE, &info);
	if (info.range_valid)
		return info.range_max;
	return 0.0;
}

// Default Constructor
CChannels::CChannels(void):m_Count(0), m_Capacity(0), m_List(NULL)
{
}

// Destructor
CChannels::~CChannels()
{
	for (int i=0; i<m_Count; i++)
		delete m_List[i];
	g_free(m_List);
}

// Clear the table
void CChannels::Clear(void)
{
	for (int i=0; i<m_Count; i++)
		delete m_List[i];
	m_Count = 0;
}

// Add an aperture, if the aperture with the same id is 
// in the table, it changes its parameters
void CChannels::Add(CChannel *item)
{
	if (m_Count>=m_Capacity) {
		m_Capacity += ALLOC_BY;
		m_List = (CChannel**)g_realloc(m_List, m_Capacity*sizeof(CChannel));
	}
	m_List[m_Count++] = item;
}

// Find aperture by id 
int CChannels::FindColumn(int col) const
{
	for (int i=0; i<m_Count; i++) {
		if (m_List[i]->Column() == col || m_List[i]->ColumnU() == col)
			return i;
	}
	return -1;
}

// Find aperture by id 
int CChannels::FindFirst(const gchar *name) const
{
	for (int i=0; i<m_Count; i++) {
		if (strcmp(m_List[i]->Name(), name)==0)
			return i;
	}
	return -1;
}

// Find aperture by id 
int CChannels::FindFirst(CChannel::tChannelInfo info) const
{
	for (int i=0; i<m_Count; i++) {
		if (m_List[i]->Info() == info)
			return i;
	}
	return -1;
}

// Get aperture by index
const CChannel *CChannels::Get(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index];
	return NULL;
}
CChannel *CChannels::Get(int index) 
{
	if (index>=0 && index<m_Count)
		return m_List[index];
	return NULL;
}

// Get column by index
int CChannels::GetColumn(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Column();
	return -1;
}

// Get column by index
int CChannels::GetColumnU(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->ColumnU();
	return -1;
}

// Get name by index
const gchar *CChannels::GetName(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Name();
	return NULL;
}

// Get name by index
int CChannels::GetPrecision(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Precision();
	return 0;
}

// Get channel info
CChannel::tChannelInfo CChannels::GetInfo(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Info();
	return CChannel::DATA_UNDEFINED;
}

// Get channel info
const gchar *CChannels::GetUnit(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Unit();
	return NULL;
}

// Get channel info
double CChannels::GetMin(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Min();
	return CChannel::DATA_UNDEFINED;
}

// Get channel info
double CChannels::GetMax(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Max();
	return CChannel::DATA_UNDEFINED;
}

// Get channel info
double CChannels::GetMaxU(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->MaxU();
	return CChannel::DATA_UNDEFINED;
}

// Get channel info
unsigned CChannels::GetExportFlags(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->ExportFlags();
	return 0;
}

//-----------------------   TABLE   --------------------------------

//
// Default constructor
//
CTable::CTable(void):m_Handle(NULL), m_CacheFlags(0), m_PKColumn(-1)
{
}

//
// Constructor with initialization
//
CTable::CTable(CmpackTable *handle):m_Handle(NULL), m_CacheFlags(0), m_PKColumn(-1)
{
	if (handle)
		m_Handle = cmpack_tab_reference(handle);
}

//
// Constructor with initialization
//
CTable::CTable(CmpackTableType type):m_Handle(NULL), m_CacheFlags(0), m_PKColumn(-1)
{
	m_Handle = cmpack_tab_init(type);
}

//
// Destructor
//
CTable::~CTable(void)
{
	if (m_Handle)
		cmpack_tab_destroy(m_Handle);
}

//
// Deep copy
//
bool CTable::MakeCopy(const CTable &orig, GError **error)
{
	Clear();

	if (orig.m_Handle) {
		CmpackTable *handle = cmpack_tab_init(CMPACK_TABLE_UNSPECIFIED);
		int res = cmpack_tab_copy(handle, orig.m_Handle);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			g_set_error(error, g_AppError, res, msg);
			cmpack_free(msg);
			cmpack_tab_destroy(handle);
			return false;
		}
		m_Handle = handle;
		InvalidateCache();
	}
	return true;
}

//
// Load table
//
bool CTable::Load(const gchar *fpath, GError **error)
{
	g_assert(fpath != NULL);

	Clear();

	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_tab_load(&m_Handle, f, 0);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	g_free(f);
	InvalidateCache();
	return res==0;
}


//
// Close handle
//
void CTable::Clear(void)
{
	if (m_Handle) {
		cmpack_tab_destroy(m_Handle);
		m_Handle = NULL;
	}
	m_ChannelsX.Clear();
	m_ChannelsY.Clear();
	m_PKColumn = -1;
	InvalidateCache();
}


//
// Get type identifier
//
CmpackTableType CTable::Type(void) const
{
	if (m_Handle)
		return cmpack_tab_get_type(m_Handle);
	return CMPACK_TABLE_UNSPECIFIED;
}

//
// Get header field by index
//
bool CTable::GetParam(int index, char **key, char **val) const
{
	const char *keyword, *value;
	if (m_Handle && cmpack_tab_gkyn(m_Handle, index, &keyword, &value)) {
		if (key)
			*key = (keyword ? g_strdup(keyword) : NULL);
		if (val)
			*val = (value ? g_strdup(value) : NULL);
		return true;
	}
	return false;
}


// Set header field
void CTable::SetParam(const char *key, const char *value)
{
	g_assert(m_Handle != NULL);

	cmpack_tab_pkys(m_Handle, key, value);
}

//
// Get number of rows
//
int CTable::Rows(void) const
{
	if (m_Handle)
		return cmpack_tab_nrows(m_Handle);
	return 0;
}

//
// Browsing
//
bool CTable::Rewind(void)
{
	if (m_Handle)
		return (cmpack_tab_rewind(m_Handle)==0);
	return false;
}
bool CTable::Next(void)
{
	if (m_Handle)
		return (cmpack_tab_next(m_Handle)==0);
	return false;
}
bool CTable::EndOfTable(void)
{
	if (m_Handle)
		return (cmpack_tab_eof(m_Handle)!=0);
	return true;
}

//
// Get color filter name
//
const char *CTable::Filter(void) const
{
	if (m_Handle)
		return cmpack_tab_gkys(m_Handle, "Filter");
	return NULL;
}

//
// Get aperture index
//
int CTable::Aperture(void) const
{
	int value;
	if (m_Handle && cmpack_tab_gkyi(m_Handle, "Aperture", &value)==0)
		return value;
	return 0;
}

//
// Make graph data
//
CmpackGraphData *CTable::ToGraphData(int xch, int ych)
{
	int		row_count, param, xcol, ycol, ucol;
	double	x, y, error;
	CmpackGraphItem item;

	if (!m_Handle)
		return NULL;

	memset(&item, 0, sizeof(CmpackGraphItem));

	CmpackGraphData *data = cmpack_graph_data_new();
	const CChannel *ch_x = ChannelsX()->Get(xch), *ch_y = ChannelsY()->Get(ych);
	row_count = cmpack_tab_nrows(m_Handle);
	if (ch_x && ch_y && row_count>0) {
		cmpack_graph_data_alloc(data, row_count);
		xcol = ch_x->Column();
		ycol = ch_y->Column();
		ucol = ch_y->ColumnU();
		int index = 0;
		cmpack_tab_rewind(m_Handle);
		while (!cmpack_tab_eof(m_Handle)) {
			if (cmpack_tab_gtdd(m_Handle, xcol, &x)==0 && 
				cmpack_tab_gtdd(m_Handle, ycol, &y)==0) {
					if (ucol<0 || cmpack_tab_gtdd(m_Handle, ucol, &error)!=0)
						error = 0.0;
					item.x = x;
					item.y = y;
					item.error = error;
					if (m_PKColumn>=0)
						cmpack_tab_gtdi(m_Handle, m_PKColumn, &param);
					else
						param = index;
					item.param = param;
					cmpack_graph_data_add(data, &item, sizeof(item));
			}
			cmpack_tab_next(m_Handle);
			index++;
		}
	}
	return data;
}

//
// Make tree model
//
GtkTreeModel *CTable::ToTreeModel(void)
{
	const static int mask = CMPACK_TM_NAME | CMPACK_TM_TYPE_PREC;

	if (!m_Handle)
		return NULL;

	int ncol = cmpack_tab_ncolumns(m_Handle);
	GType *cols = (GType*)g_malloc(ncol*sizeof(GType));
	for (int i=0; i<ncol; i++) {
		CmpackTabColumn col;
		cmpack_tab_get_column(m_Handle, i, mask, &col);
		if (col.dtype==CMPACK_TYPE_DBL) 
			cols[i] = G_TYPE_DOUBLE;
		else if (col.dtype==CMPACK_TYPE_INT)
			cols[i] = G_TYPE_INT;
		else
			cols[i] = G_TYPE_STRING;
	}
	GtkListStore *data = gtk_list_store_newv(ncol, cols);
	cmpack_tab_rewind(m_Handle);
	while (!cmpack_tab_eof(m_Handle)) {
		GtkTreeIter iter;
		gtk_list_store_append(data, &iter);
		for (int i=0; i<ncol; i++) {
			if (cols[i] == G_TYPE_DOUBLE) {
				double value;
				if (cmpack_tab_gtdd(m_Handle, i, &value)==0)
					gtk_list_store_set(data, &iter, i, value, -1);
				else
					gtk_list_store_set(data, &iter, i, DBL_MAX, -1);
			} else
			if (cols[i] == G_TYPE_INT) {
				int param;
				if (cmpack_tab_gtdi(m_Handle, i, &param)==0)
					gtk_list_store_set(data, &iter, i, param, -1);
			} else
			if (cols[i] == G_TYPE_STRING) {
				char *string;
				if (cmpack_tab_gtds(m_Handle, i, &string)==0) {
					gtk_list_store_set(data, &iter, i, string, -1);
					cmpack_free(string);
				}
			}
		}
		cmpack_tab_next(m_Handle);
	}
	g_free(cols);
	return GTK_TREE_MODEL(data);
}

//
// Invalidate cache
//
void CTable::InvalidateCache(void)
{
	m_CacheFlags = 0;
}

//
// Get double value from actual row
//
bool CTable::GetDbl(int column, double *val) const
{
	if (m_Handle)
		return cmpack_tab_gtdd(m_Handle, column, val)==0;
	return false;
}

//
// Get column with primary keys
//
int CTable::PKColumn(void)
{
	UpdateChannels();
	return m_PKColumn;
}

//
// Update list of idependent channels
//
CChannels *CTable::ChannelsX(void)
{
	UpdateChannels();
	return &m_ChannelsX;
}

//
// Update list of idependent channels
//
CChannels *CTable::ChannelsY(void)
{
	UpdateChannels();
	return &m_ChannelsY;
}

//
// Update channels
//
void CTable::UpdateChannels(void)
{
	const static int mask = CMPACK_TM_NAME | CMPACK_TM_TYPE_PREC;
	CmpackTabColumn col, col2;

	if (m_CacheFlags & CF_CHANNELS)
		return;
	
	m_ChannelsX.Clear();
	m_ChannelsY.Clear();
	m_PKColumn = -1;
	if (m_Handle) {
		int ncol = cmpack_tab_ncolumns(m_Handle);
		CmpackTableType ttype = cmpack_tab_get_type(m_Handle);
		for (int i=0; i<ncol; i++) {
			cmpack_tab_get_column(m_Handle, i, mask, &col);
			switch (ttype)
			{
			case CMPACK_TABLE_LCURVE_DIFF:
			case CMPACK_TABLE_LCURVE_INST:
			case CMPACK_TABLE_TRACKLIST:
			case CMPACK_TABLE_AIRMASS:
				if (col.dtype == CMPACK_TYPE_INT && strcmp(col.name, "FRAME")==0) {
					m_PKColumn = i;
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_FRAME_ID));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && (strcmp(col.name, "JD")==0 || strcmp(col.name, "GJD")==0 || strcmp(col.name, "JDGEO")==0)) {
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_JD));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && (strcmp(col.name, "HJD")==0 || strcmp(col.name, "JDHEL")==0)) {
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_JD_HEL));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strcmp(col.name, "AIRMASS")==0) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_AIRMASS));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strcmp(col.name, "ALTITUDE")==0) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_ALTITUDE));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && (strcmp(col.name, "OFFSETX")==0 || strcmp(col.name, "OFFSETY")==0)) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_OFFSET));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strcmp(col.name, "HELCOR")==0) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_HELCOR));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && (col.name[0]=='V' || col.name[0]=='C' || col.name[0]=='K')) {
					int err = -1;
					if (cmpack_tab_get_column(m_Handle, i+1, mask, &col2)==0 && col2.name[0]=='s') 
						err = i+1;
					m_ChannelsY.Add(new CChannel(this, i, err, CChannel::DATA_MAGNITUDE));
					if (err>=0)
						i++;
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strstr(col.name,"MAG")==col.name) {
					int err = -1;
					if (cmpack_tab_get_column(m_Handle, i+1, mask, &col2)==0 && strstr(col2.name, "ERR")==col2.name)
						err = i+1;
					m_ChannelsY.Add(new CChannel(this, i, err, CChannel::DATA_MAGNITUDE));
					if (err>=0)
						i++;
				}
				break;

			case CMPACK_TABLE_MAGDEV:
				if (col.dtype==CMPACK_TYPE_INT && strcmp(col.name, "INDEX")==0) {
					m_PKColumn = i;
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_OBJECT_ID));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strcmp(col.name, "MEAN_MAG")==0) {
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_MAGNITUDE));
				} else
				if (col.dtype==CMPACK_TYPE_DBL && strcmp(col.name, "STDEV")==0) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_DEVIATION));
				} else
				if (col.dtype==CMPACK_TYPE_INT && strcmp(col.name, "GOODPOINTS")==0) {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_FREQUENCY));
				}
				break;

			case CMPACK_TABLE_APERTURES:
				if (strcmp(col.name, "APERTURE")==0) {
					m_PKColumn = i;
					m_ChannelsX.Add(new CChannel(this, i, -1, CChannel::DATA_APERTURE_ID));
				} else
				if (col.name[0]=='C' || col.name[0]=='K') {
					m_ChannelsY.Add(new CChannel(this, i, -1, CChannel::DATA_DEVIATION));
				}
				break;

			default:
				break;
			}
		}
	}

	m_CacheFlags |= CF_CHANNELS;
}

//
// Set graph view parameters
//
void CTable::SetView(CmpackGraphView *view, int xch, int ych, bool no_errors,
	const gchar *user_name_x, const gchar *user_name_y, tDateFormat datef, 
	bool fixed_x, double xmin, double xmax, bool fixed_y, double ymin, double ymax)
{
	const gchar *name;

	cmpack_graph_view_set_error_bars(view, !no_errors);

	CChannel *ch_x = ChannelsX()->Get(xch);
	if (ch_x) {
		if (!fixed_x) 
			GetMinMaxX(xch, xmin, xmax);
		name = (user_name_x ? user_name_x : ch_x->Name());
		switch (ch_x->Info())
		{
		case CChannel::DATA_MAGNITUDE:
			CheckLimits(&xmin, &xmax, 0.05, -99.0, +99.0);
			cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.05, GRAPH_FIXED, 1, 3, name);
			break;
		case CChannel::DATA_JD:
			CheckLimits(&xmin, &xmax, 0.05, JD_MIN, JD_MAX);
			if (datef == JULIAN_DATE) 
				cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.005, GRAPH_FIXED, 0, 3, "JD");
			else
				cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.005, GRAPH_TIME, 0, 3,  "UTC");
			break;
		case CChannel::DATA_JD_HEL:
			CheckLimits(&xmin, &xmax, 0.05, JD_MIN, JD_MAX);
			if (datef == JULIAN_DATE) 
				cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.005, GRAPH_FIXED, 0, 3, "JD (hel.)");
			else
				cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.005, GRAPH_TIME, 0, 3, "UTC (hel.)");
			break;
		default:
			CheckLimits(&xmin, &xmax, 0.005, -1e99, 1e99);
			cmpack_graph_view_set_x_axis(view, FALSE, FALSE, xmin, xmax, 0.005, GRAPH_FIXED, 0, 3, name);
			break;
		}
	}

	CChannel *ch_y = ChannelsY()->Get(ych);
	if (ch_y) {
		if (!fixed_y)
			GetMinMaxY(ych, no_errors, ymin, ymax);
		name = (user_name_y ? user_name_y : ch_y->Name());
		switch (ch_y->Info())
		{
		case CChannel::DATA_MAGNITUDE:
			CheckLimits(&ymin, &ymax, 0.05, -99.0, +99.0);
			cmpack_graph_view_set_y_axis(view, FALSE, TRUE, ymin, ymax, 0.05, GRAPH_FIXED, 1, 3, name);
			break;
		case CChannel::DATA_DEVIATION:
			CheckLimits(&ymin, &ymax, 0.05, 1e-6, 99.0);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 0.05, GRAPH_FIXED, 1, 3, name);
			break;
		case CChannel::DATA_HELCOR:
			CheckLimits(&ymin, &ymax, 0.0001, -1.0, 1.0);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 0.0001, GRAPH_FIXED, 1, 4, name);
			break;
		case CChannel::DATA_AIRMASS:
			CheckLimits(&ymin, &ymax, 0.01, 1.0, 99.0);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 0.01, GRAPH_FIXED, 1, 2, name);
			break;
		case CChannel::DATA_OFFSET:
			CheckLimits(&ymin, &ymax, 0.1, -1e6, 1e6);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 0.1, GRAPH_FIXED, 0, 1, name);
			break;
		case CChannel::DATA_ALTITUDE:
			CheckLimits(&ymin, &ymax, 1.0, -90.0, +90.0);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 1.0, GRAPH_FIXED, 0, 0, name);
			break;
		default:
			CheckLimits(&ymin, &ymax, 0.005, -1e99, 1e99);
			cmpack_graph_view_set_y_axis(view, FALSE, FALSE, ymin, ymax, 0.005, GRAPH_FIXED, 0, 3, name);
			break;
		}
	}
}

//
// Get min-max for given channel
//
bool CTable::GetMinMaxX(int xch, double &min, double &max)
{
	CChannel *ch_x = ChannelsX()->Get(xch);
	if (ch_x) {
		min = ch_x->Min();
		max = ch_x->Max();
		return true;
	} else {
		min = max = 0;
		return false;
	}
}

//
// Get min-max for given channel
//
bool CTable::GetMinMaxY(int ych, bool no_errors, double &min, double &max)
{
	CChannel *ch_y = ChannelsY()->Get(ych);
	if (ch_y) {
		min = ch_y->Min();
		max = ch_y->Max();
		if (!no_errors && ch_y->ColumnU()>0) {
			min -= ch_y->MaxU();
			max += ch_y->MaxU();
		}
		return true;
	} else {
		min = max = 0;
		return false;
	}
}

//
// Set tree view parameters
//
void CTable::SetView(GtkTreeView *view)
{
	const static int mask = CMPACK_TM_NAME | CMPACK_TM_TYPE_PREC;
	CmpackTabColumn col, col2;

	// Delete old columns
	GList *list = gtk_tree_view_get_columns(view);
	for (GList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_tree_view_remove_column(view, (GtkTreeViewColumn*)(ptr->data));
	g_list_free(list);

	UpdateChannels();

	for (int i=0; i<m_ChannelsX.Count(); i++) {
		CChannel *cx = m_ChannelsX.Get(i);
		cmpack_tab_get_column(m_Handle, cx->Column(), mask, &col);
		switch (cx->Info())
		{
		case CChannel::DATA_FRAME_ID:
			AddViewCol_Int(view, cx->Column(), 1, INT_MAX, "FRAME #");
			break;
		case CChannel::DATA_OBJECT_ID:
			AddViewCol_Int(view, cx->Column(), 1, INT_MAX, "OBJECT #");
			break;
		case CChannel::DATA_APERTURE_ID:
			AddViewCol_Int(view, cx->Column(), 1, INT_MAX, "APERTURE #");
			break;
		case CChannel::DATA_JD:
			AddViewCol_Dbl(view, cx->Column(), JD_MIN, JD_MAX, col.prec, "JD");
			AddViewCol_UTC(view, cx->Column(), "UTC");
			break;
		case CChannel::DATA_JD_HEL:
			AddViewCol_Dbl(view, cx->Column(), JD_MIN, JD_MAX, col.prec, "Heliocentric JD");
			AddViewCol_UTC(view, cx->Column(), "Heliocentric UTC");
			break;
		case CChannel::DATA_MAGNITUDE:
			AddViewCol_Dbl(view, cx->Column(), -99.0, 99.0, col.prec, col.name);
			break;
		default:
			break;
		}
	}

	for (int i=0; i<m_ChannelsY.Count(); i++) {
		CChannel *cy = m_ChannelsY.Get(i);
		cmpack_tab_get_column(m_Handle, cy->Column(), mask, &col);
		switch (cy->Info())
		{
		case CChannel::DATA_MAGNITUDE:
			AddViewCol_Dbl(view, cy->Column(), -99.0, 99.0, col.prec, col.name);
			if (cmpack_tab_get_column(m_Handle, cy->ColumnU(), mask, &col2)==0) 
				AddViewCol_Dbl(view, cy->ColumnU(), 1e-6, 99.0, col2.prec, col2.name);
			break;
		case CChannel::DATA_DEVIATION:
			AddViewCol_Dbl(view, cy->Column(), 1e-6, 99.0, col.prec, col.name);
			break;
		case CChannel::DATA_AIRMASS:
			AddViewCol_Dbl(view, cy->Column(), 1.0, 99.0, col.prec, col.name);
			break;
		case CChannel::DATA_HELCOR:
			AddViewCol_Dbl(view, cy->Column(), -1.0, 1.0, col.prec, col.name);
			break;
		case CChannel::DATA_ALTITUDE:
			AddViewCol_Dbl(view, cy->Column(), -90.0, 90.0, col.prec, col.name);
			break;
		case CChannel::DATA_OFFSET:
			AddViewCol_Dbl(view, cy->Column(), -1e6, +1e6, col.prec, col.name);
			break;
		case CChannel::DATA_FREQUENCY:
			AddViewCol_Int(view, cy->Column(), 0, 0x7fffffff, col.name);
			break;
		default:
			break;
		}
	}

	// Insert last empty column that will stretch to the rest of the table
	gtk_tree_view_insert_column_with_data_func(view, -1, NULL, gtk_cell_renderer_text_new(), NULL, NULL, NULL);
}

//
// Save table to file
//
bool CTable::Save(const gchar *fpath, GError **error)
{
	static const int flags = 0;
	int res, i, count, ncols, *cols;

	g_assert(m_Handle != NULL);
	g_assert(fpath != NULL);

	UpdateChannels();
	
	ncols = cmpack_tab_ncolumns(m_Handle);
	cols = (int*)g_malloc(ncols*sizeof(int));
	count = 0;
	for (i=0; i<ncols; i++) {
		bool skip = false;
		int channel = m_ChannelsX.FindColumn(i);
		if (channel>=0) {
			unsigned f = m_ChannelsX.GetExportFlags(channel);
			if (i == m_ChannelsX.GetColumn(channel)) 
				skip = skip || (f & CChannel::EXPORT_SKIP);
			else if (i == m_ChannelsX.GetColumnU(channel))
				skip = skip || (f & CChannel::EXPORT_SKIP) || (f & CChannel::EXPORT_VALUE_ONLY);
		}
		channel = m_ChannelsY.FindColumn(i);
		if (channel>=0) {
			unsigned f = m_ChannelsY.GetExportFlags(channel);
			if (i == m_ChannelsY.GetColumn(channel)) 
				skip = skip || (f & CChannel::EXPORT_SKIP);
			else if (i == m_ChannelsY.GetColumnU(channel))
				skip = skip || (f & CChannel::EXPORT_SKIP) || (f & CChannel::EXPORT_VALUE_ONLY);
		}
		if (!skip)
			cols[count++] = i;
	}

	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	res = cmpack_tab_save(m_Handle, f, flags, cols, count);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	g_free(f);
	g_free(cols);
	return res==0;
}

bool CTable::ExportTable(const gchar *filepath, const gchar *format, unsigned flags, GError **error)
{
	g_assert(m_Handle != NULL);
	g_assert(format != NULL);
	g_assert(filepath != NULL);

	FILE *f = open_file(filepath, "w");
	if (!f) {
		g_set_error(error, g_AppError, 0, "Failed to create the file: %s", filepath);
		return false;
	}

	CCSVWriter *writer = NULL;
	if (strcmp(format, "text/csv")==0) {
		writer = new CCSVWriter(f, ',', 0, flags & CTable::EXPORT_NO_HEADER);
	} else 
	if (strcmp(format, "text/plain")==0) {
		writer = new CCSVWriter(f, ' ', 0, flags & CTable::EXPORT_NO_HEADER);
	}
	if (!writer) {
		g_set_error(error, g_AppError, 0, "Unsupported file format (%s)", format);
		return false;
	}
	
	ExportTable(*writer, flags);
	delete writer;

	fclose(f);
	return true;
}

void CTable::ExportTable(CCSVWriter &writer, unsigned flags)
{
	g_assert(m_Handle != NULL);

	UpdateChannels();

	// Which columns will be visible?
	int ncols = cmpack_tab_ncolumns(m_Handle);
	int *col_indices = (int*)g_malloc(ncols*sizeof(int));
	CmpackTabColumn *cols = (CmpackTabColumn*)g_malloc(ncols*sizeof(CmpackTabColumn));
	int count = 0;
	for (int i=0; i<ncols; i++) {
		bool skip = false;
		int channel = m_ChannelsX.FindColumn(i);
		if (channel>=0) {
			unsigned f = m_ChannelsX.GetExportFlags(channel);
			if (i == m_ChannelsX.GetColumn(channel)) 
				skip = skip || (f & CChannel::EXPORT_SKIP);
			else if (i == m_ChannelsX.GetColumnU(channel))
				skip = skip || (f & CChannel::EXPORT_SKIP) || (f & CChannel::EXPORT_VALUE_ONLY);
		}
		channel = m_ChannelsY.FindColumn(i);
		if (channel>=0) {
			unsigned f = m_ChannelsY.GetExportFlags(channel);
			if (i == m_ChannelsY.GetColumn(channel)) 
				skip = skip || (f & CChannel::EXPORT_SKIP);
			else if (i == m_ChannelsY.GetColumnU(channel))
				skip = skip || (f & CChannel::EXPORT_SKIP) || (f & CChannel::EXPORT_VALUE_ONLY);
		}
		if (!skip) {
			col_indices[count] = i;
			cmpack_tab_get_column(m_Handle, i, CMPACK_TM_TYPE_PREC | CMPACK_TM_NULVAL, &cols[count]);
			count++;
		}
	}

	// Table header
	for (int j=0; j<count; j++) {
		CmpackTabColumn col;
		cmpack_tab_get_column(m_Handle, col_indices[j], CMPACK_TM_NAME, &col);
		writer.AddColumn(col.name);
	}

	// Table data
	cmpack_tab_rewind(m_Handle);
	while (!cmpack_tab_eof(m_Handle)) {
		bool show = true;
		if (flags & CTable::EXPORT_NULVAL_SKIP_ROW) {
			for (int j=0; j<count; j++) {
				if (cols[j].dtype == CMPACK_TYPE_DBL) {
					double value;
					if (cmpack_tab_gtdd(m_Handle, col_indices[j], &value)!=0) {
						show = false;
						break;
					}
				} else
				if (cols[j].dtype == CMPACK_TYPE_INT) {
					int param;
					if (cmpack_tab_gtdi(m_Handle, col_indices[j], &param)!=0) {
						show = false;
						break;
					}
				}
			}
		}
		if (show) {
			writer.Append();
			for (int j=0; j<count; j++) {
				if (cols[j].dtype == CMPACK_TYPE_DBL) {
					double value;
					if (cmpack_tab_gtdd(m_Handle, col_indices[j], &value)==0)
						writer.SetDbl(j, value, cols[j].prec);
					else {
						if (flags & CTable::EXPORT_NULVAL_ZERO)
							writer.SetDbl(j, 0.0, cols[j].prec);
						else
							writer.SetDbl(j, cols[j].nul_value, cols[j].prec);
					}
				} else
				if (cols[j].dtype == CMPACK_TYPE_INT) {
					int param;
					if (cmpack_tab_gtdi(m_Handle, col_indices[j], &param)==0)
						writer.SetInt(j, param);
					else {
						if (flags & CTable::EXPORT_NULVAL_ZERO)
							writer.SetInt(j, 0);
						else
							writer.SetInt(j, (int)cols[j].nul_value);
					}
				} else
				if (cols[j].dtype == CMPACK_TYPE_STR) {
					char *string;
					if (cmpack_tab_gtds(m_Handle, col_indices[j], &string)==0) {
						writer.SetStr(j, string);
						cmpack_free(string);
					}
				}
			}
		}
		cmpack_tab_next(m_Handle);
	}
	g_free(col_indices);
	g_free(cols);
}

// Find row by primary key
bool CTable::Find(int value) const
{
	if (m_Handle) {
		if (m_PKColumn>=0) {
			cmpack_tab_rewind(m_Handle);
			return cmpack_tab_ftdi(m_Handle, m_PKColumn, value)!=0;
		} else {
			// Note: Return codes of cmpack_tab_ftdi and cmpack_tab_setpos are different!
			return cmpack_tab_setpos(m_Handle, value)==0;
		}
	}
	return false;
}

void CTable::Delete(void)
{
	g_assert(m_Handle != NULL);

	cmpack_tab_delete(m_Handle);
}

bool CTable::ExportHeader(const gchar *filepath, const gchar *format, unsigned flags, GError **error) const
{
	g_assert(m_Handle != NULL);
	g_assert(format != NULL);
	g_assert(filepath != NULL);

	FILE *f = open_file(filepath, "w");
	if (!f) {
		g_set_error(error, g_AppError, 0, "Failed to create the file: %s", filepath);
		return false;
	}

	CCSVWriter *writer = NULL;
	if (strcmp(format, "text/csv")==0) {
		writer = new CCSVWriter(f, ',', 0, flags & CTable::EXPORT_NO_HEADER);
	} else 
	if (strcmp(format, "text/plain")==0) {
		writer = new CCSVWriter(f, ' ', 0, flags & CTable::EXPORT_NO_HEADER);
	}
	if (!writer) {
		g_set_error(error, g_AppError, 0, "Unsupported file format (%s)", format);
		return false;
	}
	
	ExportHeader(*writer, flags);
	delete writer;

	fclose(f);
	return true;
}

//
// Export file header
//
void CTable::ExportHeader(CCSVWriter &writer, unsigned flags) const
{
	g_assert(m_Handle != NULL);

	// Table header
	writer.AddColumn("KEY");
	writer.AddColumn("VALUE");

	char *key, *val;
	for (int i=0; GetParam(i, &key, &val); i++) {
		writer.Append();
		if (key)
			writer.SetStr(0, key);
		if (val)
			writer.SetStr(1, val);
		g_free(key);
		g_free(val);
	}
}
