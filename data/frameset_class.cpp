/**************************************************************

frameset_class.cpp (C-Munipack project)
Frameset class interface
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

#include "frameset_class.h"
#include "utils.h"
#include "main.h"

//-------------------------   FRAMESET   ------------------------

//
// Default constructor
//
CFrameSet::CFrameSet(void):m_Handle(NULL), m_CacheFlags(0)
{
}

//
// Destructor
//
CFrameSet::~CFrameSet() 
{
	if (m_Handle)
		cmpack_fset_destroy(m_Handle);
}

//
// Create a new frameset
//
void CFrameSet::Init(const CApertures &apertures, const CSelection &sel)
{
	int napertures, nobjects;
	CmpackPhtAperture aper;
	CmpackCatObject obj;

	Clear();

	m_Handle = cmpack_fset_init();
	napertures = apertures.Count();
	for (int i=0; i<napertures; i++) {
		aper.id = apertures.GetId(i);
		if (aper.id>=0)
			cmpack_fset_add_aperture(m_Handle, CMPACK_PA_ID, &aper);
	}
	nobjects = sel.Count();
	for (int i=0; i<nobjects; i++) {
		obj.id = sel.GetId(i);
		if (obj.id>=0)
			cmpack_fset_add_object(m_Handle, CMPACK_OM_ID, &obj);
	}

	InvalidateCache();
}

//
// Add objects
//
void CFrameSet::Init(int aperture_id, CPhot &pht)
{
	int i, nobjects;
	CmpackPhtAperture aper;
	CmpackCatObject obj;

	Clear();

	m_Handle = cmpack_fset_init();
	if (aperture_id>=0) {
		aper.id = aperture_id;
		cmpack_fset_add_aperture(m_Handle, CMPACK_PA_ID, &aper);
	}
	nobjects = pht.ObjectCount();
	for (i=0; i<nobjects; i++) {
		obj.id = pht.GetObjectRefID(i);
		if (obj.id>=0)
			cmpack_fset_add_object(m_Handle, CMPACK_OM_ID, &obj);
	}

	InvalidateCache();
}

//
// Add objects
//
void CFrameSet::Init(int aperture_id, CCatalog &cat)
{
	int i, nobjects;
	CmpackPhtAperture aper;
	CmpackCatObject obj;

	Clear();

	m_Handle = cmpack_fset_init();
	if (aperture_id>=0) {
		aper.id = aperture_id;
		cmpack_fset_add_aperture(m_Handle, CMPACK_PA_ID, &aper);
	}
	nobjects = cat.ObjectCount();
	for (i=0; i<nobjects; i++) {
		obj.id = cat.GetObjectID(i);
		if (obj.id>=0)
			cmpack_fset_add_object(m_Handle, CMPACK_OM_ID, &obj);
	}

	InvalidateCache();
}

//
// Load data from file
//
bool CFrameSet::Load(const gchar *fpath, GError **error) 
{
	Clear();

	// Check file path
	if (!fpath || !g_file_test(fpath, G_FILE_TEST_IS_REGULAR)) {
		g_set_error(error, g_AppError, 0, "The file does not exists.");
		return false;
	}

	// Open file
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_fset_load(&m_Handle, f, 0);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	g_free(f);
	return res==0;
}

//
// Save table
//
bool CFrameSet::Save(const gchar *fpath, int aperture, GError **error)
{
	assert (m_Handle && fpath);

	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	int res = cmpack_fset_export(m_Handle, f, aperture);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	g_free(f);
	return res==0;
}

//
// Close handle
//
void CFrameSet::Clear(void)
{
	if (m_Handle) {
		cmpack_fset_destroy(m_Handle);
		m_Handle = NULL;
	}
	InvalidateCache();
}

//
// Go to the first frame
//
bool CFrameSet::Rewind(void)
{
	if (m_Handle)
		return cmpack_fset_rewind(m_Handle)==0;
	return false;
}

//
// Go to the next frame
//
bool CFrameSet::Next(void)
{
	if (m_Handle)
		return cmpack_fset_next(m_Handle)==0;
	return false;
}

// 
// Get frame information
//
bool CFrameSet::GetFrameInfo(unsigned mask, CmpackFrameInfo &info)
{
	if (m_Handle)
		return cmpack_fset_get_frame(m_Handle, mask, &info)==0;
	return false;
}

//
// Append frame
//
bool CFrameSet::AppendFrame(CPhot &file, int frame_id, GError **error)
{
	assert (m_Handle && file.m_Handle);

	int res = cmpack_fset_append_frame(m_Handle, file.m_Handle, frame_id, NULL);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	return (res==0);
}

//
// Append frame
//
bool CFrameSet::AppendFrame(double juldat, int frame_id, GError **error)
{
	assert(m_Handle && juldat>0);

	CmpackFrameInfo info;
	info.juldat = juldat;
	info.frame_id = frame_id;
	int res = cmpack_fset_append(m_Handle, CMPACK_FI_ID | CMPACK_FI_JULDAT, &info);
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		g_set_error(error, g_AppError, res, msg);
		cmpack_free(msg);
	}
	return (res==0);
}

//
// Delete frame from the frame set
//
void CFrameSet::DeleteFrame(int frame_id)
{
	if (m_Handle) {
		if (cmpack_fset_find_frame(m_Handle, frame_id))
			cmpack_fset_delete_frame(m_Handle);
	}
}


//
// Delete object from the frame set
//
void CFrameSet::DeleteObject(int object_id)
{
	if (m_Handle) {
		int index = cmpack_fset_find_object(m_Handle, object_id);
		cmpack_fset_remove_object(m_Handle, index);
	}
}


//
// Number of frames
//
int CFrameSet::Size(void)
{
	if (m_Handle)
		return cmpack_fset_frame_count(m_Handle);
	return 0;
}


//
// Get chart width in pixels
//
void CFrameSet::InvalidateCache(void)
{
	m_CacheFlags = 0;
}


//
// Number of apertures
//
int CFrameSet::ApertureCount(void) const
{
	if (m_Handle)
		return cmpack_fset_aperture_count(m_Handle);
	return 0;
}


//
// Find aperture
//
int CFrameSet::FindAperture(int id) const
{
	if (m_Handle)
		return cmpack_fset_find_aperture(m_Handle, id);
	return -1;
}


//
// Find object
//
int CFrameSet::FindObject(int id) const
{
	if (m_Handle)
		return cmpack_fset_find_object(m_Handle, id);
	return -1;
}


//
// Update reference magnitudes
//
const CApertures *CFrameSet::Apertures(void)
{
	int i, count;
	CmpackPhtAperture aper;

	if (!(m_CacheFlags & CF_APERTURES)) {
		m_Apertures.Clear();
		if (m_Handle) {
			count = cmpack_fset_aperture_count(m_Handle);
			for (i=0; i<count; i++) {
				cmpack_fset_get_aperture(m_Handle, i, CMPACK_PA_ID | CMPACK_PA_RADIUS, &aper);
				m_Apertures.Add(CAperture(aper.id, aper.radius));
			}
		}
		m_CacheFlags |= CF_APERTURES;
	}
	return &m_Apertures;
}
