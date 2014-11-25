/**************************************************************

image_class.cpp (C-Munipack project)
CCD image class interface
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

#include "ccdfile_class.h"
#include "main.h"

//--------------------------   HELPER FUNCTIONS   ----------------------------------

static gchar *FromLocale(const char *str)
{
	if (str)
		return g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	return NULL;
}

//--------------------------   CCD FILE CLASS INTERFACE   ----------------------------------

//
// Default constructor
//
CCCDFile::CCCDFile():m_Handle(NULL), m_CacheFlags(0), m_Filter(NULL), m_FormatName(NULL), 
	m_AvgFrames(0), m_SumFrames(0), m_Observer(NULL), m_Telescope(NULL), m_Instrument(NULL)
{
}

//
// Destructor
//
CCCDFile::~CCCDFile()
{
	if (m_Handle)
		cmpack_ccd_destroy(m_Handle);
	g_free(m_Filter);
	g_free(m_FormatName);
	g_free(m_Observer);
	g_free(m_Telescope);
	g_free(m_Instrument);
}

//
// Clear data
//
bool CCCDFile::Close(GError **error)
{
	if (m_Handle) {
		int res = cmpack_ccd_close(m_Handle);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			g_set_error(error, g_AppError, res, msg);
			cmpack_free(msg);
			return false;
		}
		m_Handle = NULL;
		m_Object.Clear();
		m_Location.Clear();
		g_free(m_Filter);
		m_Filter = NULL;
		g_free(m_FormatName);
		m_FormatName = NULL;
		g_free(m_Observer);
		m_Observer = NULL;
		g_free(m_Telescope);
		m_Telescope = NULL;
		g_free(m_Instrument);
		m_Instrument = NULL;
		m_AvgFrames = m_SumFrames = 0;
	}
	InvalidateCache();
	return true;
}

//
// Set image info from a file
//
bool CCCDFile::Open(const gchar *filepath, CmpackOpenMode mode, GError **error)
{
	Close();

	// Check file path
	if (mode != CMPACK_OPEN_CREATE) {
		if (!filepath || !g_file_test(filepath, G_FILE_TEST_IS_REGULAR)) {
			g_set_error(error, g_AppError, 0, "The file does not exists.");
			return false;
		}
	} else {
		if (!filepath) {
			g_set_error(error, g_AppError, 0, "Invalid file path.");
			return false;
		}
	}

	// Open file
	char *f = g_locale_from_utf8(filepath, -1, NULL, NULL, NULL);
	int res = cmpack_ccd_open(&m_Handle, f, mode, 0);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}

	InvalidateCache();
	g_free(f);
	return res==0;
}

//
// Make catalogue file from photometry file
//
bool CCCDFile::MakeCopy(const CCCDFile &img, GError **error)
{
	assert (img.m_Handle!=NULL);

	CmpackCcdFile *handle = cmpack_ccd_new();
	int res = cmpack_ccd_copy(img.m_Handle, handle, NULL);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(handle);
	}
	Close();
	m_Handle = handle;
	InvalidateCache();
	return res==0;
}

// Save CCD file as...
bool CCCDFile::SaveAs(const gchar *fpath, GError **error) const
{
	assert (m_Handle!=NULL);

	CmpackCcdFile *file;
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_ccd_open(&file, f, CMPACK_OPEN_CREATE, 0);
	g_free(f);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		return false;
	}

	res = cmpack_ccd_copy(file, m_Handle, NULL);
	if (res==0)
		res = cmpack_ccd_close(file);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(file);
		return false;
	}
	return res==0;
}

void CCCDFile::InvalidateCache(void)
{
	m_CacheFlags = 0;
}

const CObjectCoords *CCCDFile::Object(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_OBJECT)) {
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_OBJECT, &params)==0) 
			m_Object.Assign(&params.object);
		m_CacheFlags |= CF_OBJECT;
	}
	return &m_Object;
}

const gchar *CCCDFile::Observer(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_OBSERVER)) {
		g_free(m_Observer);
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_OBSERVER, &params)==0)
			m_Observer = FromLocale(params.observer);
		else
			m_Observer = NULL;
		m_CacheFlags |= CF_OBSERVER;
	}
	return m_Observer;
}

const gchar *CCCDFile::Telescope(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_TELESCOPE)) {
		g_free(m_Telescope);
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_TELESCOPE, &params)==0)
			m_Telescope = FromLocale(params.telescope);
		else
			m_Telescope = NULL;
		m_CacheFlags |= CF_TELESCOPE;
	}
	return m_Telescope;
}

const gchar *CCCDFile::Instrument(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_INSTRUMENT)) {
		g_free(m_Instrument);
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_INSTRUMENT, &params)==0)
			m_Instrument = FromLocale(params.instrument);
		else
			m_Instrument = NULL;
		m_CacheFlags |= CF_INSTRUMENT;
	}
	return m_Instrument;
}

const CLocation *CCCDFile::Location(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_LOCATION)) {
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_LOCATION, &params)==0) 
			m_Location.Assign(&params.location);
		m_CacheFlags |= CF_LOCATION;
	}
	return &m_Location;
}

const char *CCCDFile::Filter(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_FILTER)) {
		g_free(m_Filter);
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_FILTER, &params)==0)
			m_Filter = FromLocale(params.filter);
		else
			m_Filter = NULL;
		m_CacheFlags |= CF_FILTER;
	}
	return m_Filter;
}

//
// Get image width
//
int CCCDFile::Width(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_IMAGE, &params)==0)
		return params.image_width;
	return 0;
}

//
// Get image height
//
int CCCDFile::Height(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_IMAGE, &params)==0)
		return params.image_height;
	return 0;
}

//
// Get format identifier
//
CmpackFormat CCCDFile::FileFormat(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_FORMAT, &params)==0)
		return params.format_id;
	return CMPACK_FORMAT_UNKNOWN;
}

//
// Get file format name
//
const char *CCCDFile::Magic(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_FORMATNAME)) {
		g_free(m_FormatName);
		m_FormatName = NULL;
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_FORMAT, &params)==0)
			m_FormatName = FromLocale(params.format_name);
		m_CacheFlags |= CF_FORMATNAME;
	}
	return m_FormatName;
}

//
// Get date of observation
//
bool CCCDFile::DateTime(CmpackDateTime *dt) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_DATETIME, &params)==0) {
		*dt = params.date_time;
		return true;
	}
	return false;
}

//
// Get Julian date of observation
//
double CCCDFile::JulianDate(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_JD, &params)==0) 
		return params.jd;
	return 0.0;
}

//
// Exposure duration
//
double CCCDFile::ExposureDuration(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_EXPOSURE, &params)==0) 
		return params.exposure;
	return -1.0;
}

//
// CCD temperature
//
double CCCDFile::CCDTemperature(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_CCDTEMP, &params)==0) 
		return params.ccdtemp;
	return -999.9;
}


//
// Number of accumulated frames
//
int CCCDFile::AvgFrames(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_SUBFRAMES)) {
		m_AvgFrames = m_SumFrames = 0;
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_SUBFRAMES, &params)==0) {
			m_AvgFrames = params.subframes_avg;
			m_SumFrames = params.subframes_sum;
		}
		m_CacheFlags |= CF_SUBFRAMES;
	}
	return m_AvgFrames;
}

//
// Number of summed frames
//
int CCCDFile::SumFrames(void)
{
	CmpackCcdParams params;

	if (!(m_CacheFlags & CF_SUBFRAMES)) {
		m_AvgFrames = m_SumFrames = 0;
		if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_SUBFRAMES, &params)==0) {
			m_AvgFrames = params.subframes_avg;
			m_SumFrames = params.subframes_sum;
		}
		m_CacheFlags |= CF_SUBFRAMES;
	}
	return m_SumFrames;
}

//
// Get image depth identifier
//
CmpackBitpix CCCDFile::Depth(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_IMAGE, &params)==0)
		return params.image_format;
	return CMPACK_BITPIX_UNKNOWN;
}

//
// Check file format
//
bool CCCDFile::isWorkingFormat(void) const
{
	CmpackCcdParams params;

	if (m_Handle && cmpack_ccd_get_params(m_Handle, CMPACK_CM_WORKFORMAT, &params)==0)
		return params.working_format!=0;
	return false;
}

//
// Get header field by index
//
bool CCCDFile::GetParam(int index, char **key, char **val, char **com) const
{
	char *keyword, *value, *comment;

	if (m_Handle && cmpack_ccd_get_param(m_Handle, index, &keyword, &value, &comment)==0) {
		if (key) 
			*key = FromLocale(keyword);
		if (val) 
			*val = FromLocale(value);
		if (com) 
			*com = FromLocale(comment);
		cmpack_free(comment);
		cmpack_free(keyword);
		cmpack_free(value);
		return true;
	}
	return false;
}

//
// Read image data
//
bool CCCDFile::GetImageData(CImage &img, CmpackBitpix bitpix, GError **error)
{
	assert (m_Handle != NULL);

	CmpackImage *i;
	int res = cmpack_ccd_to_image(m_Handle, bitpix, &i);
	if (res==0) 
		img.Assign(i);
	else {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	return (res==0);
}

//
// Update image data
//
bool CCCDFile::SetImageData(const CImage &img, GError **error)
{
	assert (m_Handle != NULL);

	int res = cmpack_ccd_set_image(m_Handle, img.m_Handle);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	return (res==0);
}

//
// Export file header
//
bool CCCDFile::ExportHeader(const gchar *filepath, const gchar *format, unsigned flags, GError **error) const
{
	assert (m_Handle != NULL);
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
void CCCDFile::ExportHeader(CCSVWriter &writer, unsigned flags) const
{
	g_assert(m_Handle != NULL);

	// Table header
	writer.AddColumn("KEY");
	writer.AddColumn("VALUE");
	writer.AddColumn("COMMENT");

	gchar *key, *val, *com;
	for (int i=0; GetParam(i, &key, &val, &com); i++) {
		writer.Append();
		if (key)
			writer.SetStr(0, key);
		if (val)
			writer.SetStr(1, val);
		if (com)
			writer.SetStr(2, com);
		g_free(key);
		g_free(val);
		g_free(com);
	}
}
