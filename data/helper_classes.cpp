/**************************************************************

selection.cpp (C-Munipack project)
Table of selected stars
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
#include <glib/gstdio.h>

#include "helper_classes.h"
#include "ccdfile_class.h"
#include "utils.h"
#include "configuration.h"
#include "main.h"

#define ALLOC_BY 64

// File types
static const struct {
	tFileType type;
	const gchar *rc_group;
} FileTypes[] = {
	{ TYPE_IMAGE, "CCD image" },
	{ TYPE_PHOT, "Photometry file" },
	{ TYPE_CAT, "Catalog file" },
	{ TYPE_TABLE, "Table" },
	{ TYPE_VARFIND, "Frame set" },
	{ TYPE_PROJECT, RECENT_GROUP_PROJECT },
	{ TYPE_PROFILE, NULL },
	{ TYPE_UNKNOWN, NULL }
};

// Selection types
static const gchar *typeLabel[CMPACK_SELECT_COUNT] = {
	NULL, "var", "comp", "check"
};

// Table types
static struct {
	CmpackTableType type;
	const gchar *str;
} TableTypes[] = {
	{ CMPACK_TABLE_UNSPECIFIED,	"Other" },
	{ CMPACK_TABLE_LCURVE_DIFF,	"LCD" },
	{ CMPACK_TABLE_LCURVE_INST,	"LCI" },
	{ CMPACK_TABLE_MAGDEV,		"MAGDEV" },
	{ CMPACK_TABLE_TRACKLIST,	"TRACK" },
	{ CMPACK_TABLE_APERTURES,	"APER" },
	{ CMPACK_TABLE_AIRMASS,		"AIRMAS" },
	{ (CmpackTableType)(-1),		NULL }
};

//--------------------------   HELPER FUNCTIONS   ----------------------------------

//
// Make copy of a string
//
static gchar *CopyString(const gchar *str)
{
	return (str ? g_strdup(str) : NULL);
}

//
// Convert from current locale
//
static gchar *FromLocale(const char *str)
{
	if (str)
		return g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	return NULL;
}


// Autodetect file type
tFileType FileType(const char *fpath)
{
	const int BLOCKSIZE = 2048;

	char buffer[BLOCKSIZE];
	int bytes, filesize;

	if (CProject::isProjectFile(fpath, NULL))
		return TYPE_PROJECT;
	if (CProfile::isProfileFile(fpath, NULL))
		return TYPE_PROFILE;

	tFileType type = TYPE_UNKNOWN;
	FILE *f = open_file(fpath, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		filesize = ftell(f);
		fseek(f, 0, SEEK_SET);
		bytes = fread(buffer, 1, BLOCKSIZE, f);
		if (cmpack_pht_test_buffer(buffer, bytes, filesize))
			type = TYPE_PHOT;
		else if (cmpack_cat_test_buffer(buffer, bytes, filesize))
			type = TYPE_CAT;
		else if (cmpack_fset_test_buffer(buffer, bytes, filesize))
			type = TYPE_VARFIND;
		else if (cmpack_tab_test_buffer(buffer, bytes, filesize))
			type = TYPE_TABLE;
		else if (cmpack_ccd_test_buffer(buffer, bytes, filesize))
			type = TYPE_IMAGE;
		fclose(f);
	}
	return type;
}

// Project type to recent group
const gchar *FileTypeRecentGroup(tFileType type)
{
	for (int i=0; FileTypes[i].type!=TYPE_UNKNOWN; i++)
		if (FileTypes[i].type == type)
			return FileTypes[i].rc_group;
	return NULL;
}

// String representing table type
const gchar *TableTypeToStr(CmpackTableType type)
{
	for (int i=0; TableTypes[i].str!=NULL; i++) {
		if (TableTypes[i].type == type)
			return TableTypes[i].str;
	}
	return TableTypes[0].str;
}

//----------------------   CONSOLE   -------------------------

//
// Call OnPrint
//
void CConsole::OutputProc(const char *text, void *user_data)
{
	char *buffer = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
	((CConsole*)user_data)->OnPrint(buffer);
	g_free(buffer);
}

//
// Call OnPrint
//
void CConsole::Print(const char *text)
{
	OnPrint(text); 
}

//----------------------   SELECTION OF STARS   -------------------------

//
// Default constructor
//
CSelection::CSelection():m_Count(0), m_Capacity(0), m_List(NULL), 
	m_Changed(false), m_LockCount(0)
{
}

//
// Constructor from string
//
CSelection::CSelection(const gchar *string):m_Count(0), m_Capacity(0), m_List(NULL), 
	m_Changed(false), m_LockCount(0)
{
	const gchar *ptr = string;

	BeginUpdate();
	while (*ptr!='\0') {
		gchar *endptr;
		int id = strtol(ptr, &endptr, 10);
		if (endptr!=ptr && *endptr==':') {
			endptr++;
			for (int i=CMPACK_SELECT_VAR; i<CMPACK_SELECT_COUNT; i++) {
				if (memcmp(endptr, typeLabel[i], strlen(typeLabel[i]))==0) {
					Select(id, (CmpackSelectionType)i);
					break;
				}
			}
		}
		// Find start of the next piece
		ptr += strcspn(ptr, ";");
		if (*ptr == ';')
			ptr++;
	}
	EndUpdate();
}


//
// Copy constructor
//
CSelection::CSelection(const CSelection &orig):m_Count(0), m_Capacity(0), 
	m_List(NULL), m_Changed(false), m_LockCount(0)
{
	if (orig.m_Count>0) {
		m_Count = m_Capacity = orig.m_Count;
		m_List = (tStarRec*)g_malloc(m_Capacity*sizeof(tStarRec));
		memcpy(m_List, orig.m_List, m_Count*sizeof(tStarRec));
	}
}


//
// Destructor
//
CSelection::~CSelection()
{
	g_free(m_List);
}


//
// assignment operator
//
CSelection &CSelection::operator=(const CSelection &orig)
{
	if (&orig!=this) {
		BeginUpdate();
		g_free(m_List);
		m_Count = m_Capacity = orig.m_Count;
		if (m_Count>0) {
			m_List = (tStarRec*)g_malloc(m_Capacity*sizeof(tStarRec));
			memcpy(m_List, orig.m_List, m_Count*sizeof(tStarRec));
		} else 
			m_List = NULL;
		m_Changed = true;
		EndUpdate();
	}
	return *this;
}

//
// Comparison operators
//
bool operator==(const CSelection &a, const CSelection &b)
{
	if (a.Count() == b.Count()) {
		for (int i=0; i<a.Count(); i++) {
			if (a.m_List[i].id != b.m_List[i].id || a.m_List[i].type != b.m_List[i].type)
				return false;
		}
		return true;
	}
	return false;
}

//
// Increment update lock
//
void CSelection::BeginUpdate()
{
	g_return_if_fail(m_LockCount >= 0);
	m_LockCount++;
	m_Changed = false;
}

//
// Decrement update lock
//
void CSelection::EndUpdate()
{
	g_return_if_fail(m_LockCount > 0);
	m_LockCount--;
	if (m_LockCount==0) {
		if (m_Changed) {
			Reindex();
			m_Changed = false;
		}
	} else {
		m_Changed = true;
	}
}

//
// Postpone callbacks if locked
//
void CSelection::Changed(void)
{
	if (m_LockCount>0) {
		m_Changed = true;
	} else {
		Reindex();
		m_Changed = false;
	}
}

//
// Clear data
//
void CSelection::Clear()
{
	m_Count = m_Capacity = 0;
	g_free(m_List);
	m_List = NULL;
	Changed();
}

//
// Select a star
//
int CSelection::Select(int id, CmpackSelectionType type)
{
	int i, index = Search(id);
	if (index>=0) {
		if (m_List[index].type != type) {
			for (i=index+1; i<m_Count; i++)
				m_List[i-1] = m_List[i];
			if (type!=CMPACK_SELECT_NONE) {
				index = m_Count-1;
				m_List[index].id = id;
				m_List[index].type = type;
				m_List[index].index = 0;
			} else {
				index = -1;
				m_Count--;
			}
			Changed();
		}
	} else {
		if (type != CMPACK_SELECT_NONE) {
			if (m_Count >= m_Capacity) {
				m_Capacity += 64;
				m_List = (tStarRec*)g_realloc(m_List, m_Capacity*sizeof(tStarRec));
			}
			index = m_Count++;
			m_List[index].id = id;
			m_List[index].type = type;
			m_List[index].index = 0;
			Changed();
		}
	}
	return index;
}

//
// Unselect a star
//
void CSelection::Unselect(CmpackSelectionType type)
{
	int i, j;

	BeginUpdate();
	for (j=0; j<m_Count; j++) {
		if (m_List[j].type == type) {
			for (i=j+1; i<m_Count; i++)
				m_List[i-1] = m_List[i];
			m_Count--;
			m_Changed = true;
		}
	}
	EndUpdate();
}

//
// Get selection
//
int CSelection::IndexOf(int id) const
{
	return Search(id);
}

//
// Get star identifier
//
int CSelection::GetId(int i) const
{
	if (i>=0 && i<m_Count) 
		return m_List[i].id;
	else
		return 0;
}

//
// Get star type
//
CmpackSelectionType CSelection::GetType(int i) const
{
	if (i>=0 && i<m_Count)
		return m_List[i].type;
	else
		return CMPACK_SELECT_NONE;
}

//
// Get star index
//
int CSelection::GetIndex(int i) const
{
	if (i>=0 && i<m_Count)
		return m_List[i].index;
	else
		return 0;
}

//
// Search record by its id
//
int CSelection::Search(int id) const
{
	int i;

	for (i=0; i<m_Count; i++) {
		if (m_List[i].id == id)
			return i;
	}
	return -1;
}

//
// Reindex stars
//
void CSelection::Reindex(void)
{
	int i, count[CMPACK_SELECT_COUNT];

	memset(count, 0, CMPACK_SELECT_COUNT*sizeof(int));
	for (i=0; i<m_Count; i++) 
		m_List[i].index = ++count[m_List[i].type];
}

//
// Set selection
//
void CSelection::SetStarList(CmpackSelectionType type, const char *string)
{
	int i, j, id;
	const char *str;
	char *endptr;

	// Delete all stars with specified type
	for (i=0; i<m_Count; i++) {
		if (m_List[i].type == type) {
			for (j=i+1; j<m_Count; j++)
				m_List[j-1] = m_List[j];
			m_Count--;
		}
	}

	// Parse string and add all stars
	if (string) {
		str = string;
		id = strtol(str, &endptr, 10);
		while (endptr > str) {
			int index = Search(id);
			if (index>=0) {
				m_List[index].type = type;
				m_List[index].index = 0;
			} else {
				if (m_Count >= m_Capacity) {
					m_Capacity += 64;
					m_List = (tStarRec*)g_realloc(m_List, m_Capacity*sizeof(tStarRec));
				}
				m_List[m_Count].id = id;
				m_List[m_Count].type = type;
				m_List[m_Count].index = 0;
				m_Count++;
			}
			str = endptr + strcspn(endptr, "-+0123456789");
			id = strtol(str, &endptr, 10);
		}
	}
	Changed();
}

//
// Set selection
//
void CSelection::SetStarList(CmpackSelectionType type, const int *list, int count)
{
	int i, j;

	// Delete all stars with specified type
	for (i=0; i<m_Count; i++) {
		if (m_List[i].type == type) {
			for (j=i+1; j<m_Count; j++)
				m_List[j-1] = m_List[j];
			m_Count--;
		}
	}

	// Parse string and add all stars
	for (i=0; i<count; i++) {
		int index = Search(list[i]);
		if (index>=0) {
			m_List[index].type = type;
			m_List[index].index = 0;
		} else {
			if (m_Count >= m_Capacity) {
				m_Capacity += 64;
				m_List = (tStarRec*)g_realloc(m_List, m_Capacity*sizeof(tStarRec));
			}
			m_List[m_Count].id = list[i];
			m_List[m_Count].type = type;
			m_List[m_Count].index = 0;
			m_Count++;
		}
	}
	Changed();
}

//
// Get number of stars of specified type
//
int CSelection::CountStars(CmpackSelectionType type) const
{
	int i, count;

	count = 0;
	for (i=0; i<m_Count; i++) {
		if (m_List[i].type == type) 
			count++;
	}
	return count;
}

//
// Get selection
//
int CSelection::GetStarList(CmpackSelectionType type, int *list, int maxlen) const
{
	int i, count;

	count = 0;
	for (i=0; i<m_Count; i++) {
		if (m_List[i].type == type) 
			list[count++] = m_List[i].id;
	}
	return count;
}

//
// Get selection
//
char *CSelection::GetStarList(CmpackSelectionType type) const
{
	int i, count;
	char aux[12];

	count = 0;
	for (i=0; i<m_Count; i++) {
		if (m_List[i].type == type) 
			count++;
	}

	if (count>0) {
		char *buf = (char*)g_malloc((count*12+1)*sizeof(char));
		if (buf) {
			buf[0] = '\0';
			for (i=0; i<m_Count; i++) {
				if (m_List[i].type == type) {
					if (buf[0]!='\0')
						strcat(buf, ",");
					sprintf(aux, "%d", m_List[i].id);
					strcat(buf, aux);
				}
			}
		}
		return buf;
	}
	return NULL;
}

//
// Convert to string
//
gchar *CSelection::toString(void) const
{
	gchar *buf = (gchar*)g_malloc((m_Count*16+1)*sizeof(gchar)), *ptr = buf;
	for (int i=0; i<m_Count; i++) {
		if (m_List[i].type>CMPACK_SELECT_NONE && m_List[i].type<CMPACK_SELECT_COUNT) 
			ptr += sprintf(ptr, ";%d:%s", m_List[i].id, typeLabel[m_List[i].type]);
	}
	*ptr = '\0';
	return buf;
}

//----------------------   LIST OF TAGS   -------------------------

// Constructor(s)
CTags::CTags(const CTags &orig):m_List(NULL)
{
	int id;
	const gchar *tag;

	CTags::tIter iter;
	bool ok = orig.First(&iter, &id, &tag);
	while (ok) {
		if (tag && *tag!='\0')
			Set(id, tag);
		ok = orig.Next(&iter, &id, &tag);
	}
}
	
// Destructor
CTags::~CTags()
{
	Clear();
}

// Clear data
void CTags::Clear()
{
	tData *ptr = m_List, *next;
	while (ptr) {
		next = ptr->next;
		g_free(ptr->tag);
		g_free(ptr);
		ptr = next;
	}
	m_List = NULL;
}

// Assignment operator
CTags &CTags::operator=(const CTags &orig)
{
	int id;
	const gchar *tag;

	if (&orig!=this) {
		Clear();
		CTags::tIter iter;
		bool ok = orig.First(&iter, &id, &tag);
		while (ok) {
			if (tag && *tag!='\0')
				Set(id, tag);
			ok = orig.Next(&iter, &id, &tag);
		}
	}
	return *this;
}

// Set a tag
void CTags::Set(int id, const gchar *tag)
{
	if (tag && *tag!='\0') {
		tData *ptr=m_List, *prev=NULL;
		// Sort items by id
		while (ptr && ptr->id<id) {
			prev = ptr;
			ptr = ptr->next;
		}
		if (ptr && ptr->id == id) {
			// Change existing tag
			if (strcmp(ptr->tag, tag)!=0) {
				g_free(ptr->tag);
				ptr->tag = g_strdup(tag);
			}
		} else {
			// Create a new tag before ptr
			tData *it = (tData*)g_malloc0(sizeof(tData));
			it->id = id;
			it->tag = g_strdup(tag);
			it->next = ptr;
			if (prev)
				prev->next = it;
			else
				m_List = it;
		}
	} else
		Remove(id);
}

// Remove a tag
void CTags::Remove(int id)
{
	tData *prev=NULL, *next;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (ptr->id == id) {
			next = ptr->next;
			g_free(ptr->tag);
			g_free(ptr);
			if (prev)
				prev->next = next;
			else
				m_List = next;
			break;
		}	
		prev = ptr;
	}
}

// Get a tag
const gchar *CTags::Get(int id) const
{
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (ptr->id == id) 
			return ptr->tag;
	}
	return NULL;
}

// Get number of tags
bool CTags::First(tIter *iter, int *pid, const gchar **ptag) const
{
	tData *tag = m_List;
	if (tag) {
		if (pid)
			*pid = tag->id;
		if (ptag)
			*ptag = tag->tag;
		iter->ptr = tag;
		return true;
	} else {
		iter->ptr = NULL;
		return false;
	}
}

// Get tag by index
bool CTags::Next(tIter *iter, int *pid, const gchar **ptag) const
{
	if (!iter->ptr)
		return false;

	tData *tag = iter->ptr->next;
	if (tag) {
		if (pid)
			*pid = tag->id;
		if (ptag)
			*ptag = tag->tag;
		iter->ptr = tag;
		return true;
	} else {
		iter->ptr = NULL;
		return false;
	}
}

// Number of tags
int CTags::Count(void) const
{
	int count = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) 
		count++;
	return count;
}

//----------------------   OBJECT COORDINATES   -----------------------------

//
// Default constructor
//
CObjectCoords::CObjectCoords():m_Name(NULL), m_RA(NULL), m_Dec(NULL), 
	m_Source(NULL), m_Remarks(NULL)
{
}

//
// Copy constructor
//
CObjectCoords::CObjectCoords(const CObjectCoords &orig)
{
	m_Name = CopyString(orig.m_Name);
	m_RA = CopyString(orig.m_RA);
	m_Dec = CopyString(orig.m_Dec);
	m_Source = CopyString(orig.m_Source);
	m_Remarks = CopyString(orig.m_Remarks);
}


//
// Destructor
//
CObjectCoords::~CObjectCoords()
{
	g_free(m_Name);
	g_free(m_RA);
	g_free(m_Dec);
	g_free(m_Source);
	g_free(m_Remarks);
}


// assignment operator
//
CObjectCoords &CObjectCoords::operator=(const CObjectCoords &orig)
{
	if (&orig!=this) {
		g_free(m_Name);
		m_Name = CopyString(orig.m_Name);
		g_free(m_RA);
		m_RA = CopyString(orig.m_RA);
		g_free(m_Dec);
		m_Dec = CopyString(orig.m_Dec);
		g_free(m_Source);
		m_Source = CopyString(orig.m_Source);
		g_free(m_Remarks);
		m_Remarks = CopyString(orig.m_Remarks);
	}
	return *this;
}

//
// Set data
// 
void CObjectCoords::Assign(const CmpackObjCoords *c)
{
	char buf[256];

	g_free(m_Name);
	m_Name = FromLocale(c->designation);
	g_free(m_RA);
	if (c->ra_valid) {
		cmpack_ratostr(c->ra, buf, 256);
		m_RA = CopyString(buf);
	} else
		m_RA = NULL;
	g_free(m_Dec);
	if (c->dec_valid) {
		cmpack_dectostr(c->dec, buf, 256);
		m_Dec = CopyString(buf);
	} else
		m_Dec = NULL;
	g_free(m_Remarks);
	m_Remarks = NULL;
	g_free(m_Source);
	m_Source = NULL;
}

//
// Set object designation
//
void CObjectCoords::SetName(const char *name)
{
	g_free(m_Name);
	m_Name = CopyString(name);
}

//
// Set source
//
void CObjectCoords::SetSource(const char *source)
{
	g_free(m_Source);
	m_Source = CopyString(source);
}

//
// Set remarks
//
void CObjectCoords::SetRemarks(const char *remarks)
{
	g_free(m_Remarks);
	m_Remarks = CopyString(remarks);
}

//
// Set object coordinates
//
void CObjectCoords::SetRA(const char *ra)
{
	g_free(m_RA);
	m_RA = CopyString(ra);
}
void CObjectCoords::SetDec(const char *dec)
{
	g_free(m_Dec);
	m_Dec = CopyString(dec);
}

//
// Clear data
//
void CObjectCoords::Clear()
{
	g_free(m_Name);
	m_Name = NULL;
	g_free(m_RA);
	m_RA = NULL;
	g_free(m_Dec);
	m_Dec = NULL;
	g_free(m_Source);
	m_Source = NULL;
	g_free(m_Remarks);
	m_Remarks = NULL;
}


//
// Check validity of coordinates
//
bool CObjectCoords::Valid(void) const
{
	double x, y;

	if (m_RA && m_Dec && *m_RA!='\0' && *m_Dec!='\0') 
		return cmpack_strtora(m_RA, &x)==0 && cmpack_strtodec(m_Dec, &y)==0;
	return false;
}

//
// Comparison operators
//
bool operator==(const CObjectCoords &a, const CObjectCoords &b)
{
	return StrCmp0(a.m_Name, b.m_Name)==0 &&
		StrCmp0(a.m_RA, b.m_RA)==0 && StrCmp0(a.m_Dec, b.m_Dec)==0;
}

//----------------------   OBSERVER COORDINATES   -----------------------------

//
// Default constructor
//
CLocation::CLocation():m_Name(NULL), m_Lon(NULL), m_Lat(NULL), m_Com(NULL)
{
}


//
// Copy constructor
//
CLocation::CLocation(const CLocation &orig)
{
	m_Name = CopyString(orig.m_Name);
	m_Lon = CopyString(orig.m_Lon);
	m_Lat = CopyString(orig.m_Lat);
	m_Com = CopyString(orig.m_Com);
}


//
// Destructor
//
CLocation::~CLocation()
{
	Clear();
}


// assignment operator
//
CLocation &CLocation::operator=(const CLocation &orig)
{
	if (&orig!=this) {
		g_free(m_Name);
		m_Name = CopyString(orig.m_Name);
		g_free(m_Lon);
		m_Lon = CopyString(orig.m_Lon);
		g_free(m_Lat);
		m_Lat = CopyString(orig.m_Lat);
		g_free(m_Com);
		m_Com = CopyString(orig.m_Com);
	}
	return *this;
}

//
// Set data
// 
void CLocation::Assign(const CmpackLocation *c)
{
	char buf[256];

	g_free(m_Name);
	m_Name = FromLocale(c->designation);
	g_free(m_Lon);
	if (c->lon_valid) {
		cmpack_lontostr(c->lon, buf, 256);
		m_Lon = CopyString(buf);
	} else
		m_Lon = NULL;
	g_free(m_Lat);
	if (c->lat_valid) {
		cmpack_lattostr(c->lat, buf, 256);
		m_Lat = CopyString(buf);
	} else
		m_Lat = NULL;
	g_free(m_Com);
	m_Com = NULL;
}

//
// Set observer's designation
//
void CLocation::SetName(const char *name)
{
	g_free(m_Name);
	m_Name = CopyString(name);
}

//
// Set remarks
//
void CLocation::SetComment(const char *com)
{
	g_free(m_Com);
	m_Com = CopyString(com);
}

//
// Set observer's coordinates
//
void CLocation::SetLon(const char *lon)
{
	g_free(m_Lon);
	m_Lon = CopyString(lon);
}
void CLocation::SetLat(const char *lat)
{
	g_free(m_Lat);
	m_Lat = CopyString(lat);
}

//
// Clear data
//
void CLocation::Clear()
{
	g_free(m_Name);
	m_Name = NULL;
	g_free(m_Lon);
	m_Lon = NULL;
	g_free(m_Lat);
	m_Lat = NULL;
	g_free(m_Com);
	m_Com = NULL;
}

//
// Check validity of coordinates
//
bool CLocation::Valid(void) const
{
	double x, y;

	if (m_Lon && m_Lat && *m_Lon!='\0' && *m_Lat!='\0') 
		return cmpack_strtolon(m_Lon, &x)==0 && cmpack_strtolat(m_Lat, &y)==0;
	return false;
}

//
// Comparison operators
//
bool operator==(const CLocation &a, const CLocation &b)
{
	return StrCmp0(a.m_Name, b.m_Name)==0 &&
		StrCmp0(a.m_Lon, b.m_Lon)==0 && StrCmp0(a.m_Lat, b.m_Lat)==0;
}

//------------------------   FILE INFO   --------------------------

//
// Default constructor
//
CFileInfo::CFileInfo():m_FullPath(NULL)
{
	memset(&m_MTime, 0, sizeof(time_t));
}


//
// Constructor with initialization
//
CFileInfo::CFileInfo(const gchar *path):m_FullPath(NULL)
{
	char *fullpath;
	struct stat fs;

	memset(&m_MTime, 0, sizeof(time_t));
	if (path) {
		if (g_path_is_absolute(path))
			fullpath = g_strdup(path);
		else
			fullpath = g_build_filename(g_Project->DataDir(), path, NULL);
		if (g_stat(fullpath, &fs)==0) {
			m_MTime = fs.st_mtime;
			m_FullPath = fullpath;
		} else {
			g_free(fullpath);
		}
	}
}


//
// Copy constructor
//
CFileInfo::CFileInfo(const CFileInfo &orig)
{
	m_FullPath = CopyString(orig.m_FullPath);
	m_MTime = orig.m_MTime;
}


//
// Destructor
//
CFileInfo::~CFileInfo()
{
	g_free(m_FullPath);
}


//
// Assignment operator
//
CFileInfo &CFileInfo::operator=(const CFileInfo &orig)
{
	if (&orig!=this) {
		g_free(m_FullPath);
		m_FullPath = CopyString(orig.m_FullPath);
		m_MTime = orig.m_MTime;
	}
	return *this;
}


//
// Clear data
//
void CFileInfo::Clear(void)
{
	g_free(m_FullPath);
	m_FullPath = NULL;
	memset(&m_MTime, 0, sizeof(time_t));
}


//
// Load from project file
//
void CFileInfo::Load(GKeyFile *cfg, const gchar *section, const gchar *prefix)
{
	gchar key[128], *val;

	Clear();

	sprintf(key, "%s.path", prefix);
	val = g_key_file_get_string(cfg, section, key, NULL);
	if (val && *val!='\0')
		m_FullPath = g_strdup(val);
	g_free(val);

	sprintf(key, "%s.time", prefix);
	val = g_key_file_get_string(cfg, section, key, NULL);
	if (val && *val!='\0') {
		if (sizeof(time_t)==sizeof(int)) 
			sscanf(val, " %11d ", (int*)&m_MTime);
		else {
			int time[2];
			sscanf(val, " %11d %11d", time, time+1);
			memcpy(&m_MTime, time, 2*sizeof(int));
		}
	}
	g_free(val);
}


//
// Save to project file
//
void CFileInfo::Save(GKeyFile *cfg, const gchar *section, const gchar *prefix) const
{
	gchar key[128], val[128];

	sprintf(key, "%s.path", prefix);
	if (m_FullPath) 
		g_key_file_set_string(cfg, section, key, m_FullPath);
	else
		g_key_file_remove_key(cfg, section, key, NULL);

	sprintf(key, "%s.time", prefix);
	if (m_MTime!=0) {
		if (sizeof(time_t)==sizeof(int))
			sprintf(val, "%d", (int)m_MTime);
		else {
			int time[2];
			memcpy(time, &m_MTime, 2*sizeof(int));
			sprintf(val, "%d %d", time[0], time[1]);
		}
		g_key_file_set_string(cfg, section, key, val);
	} else {
		g_key_file_remove_key(cfg, section, key, NULL);
	}
}


//
// Equivalence operator
//
int CFileInfo::Compare(const CFileInfo &b) const
{
	if (!m_FullPath) 
		return (b.m_FullPath ? -1 : 0);
	if (!b.m_FullPath)
		return 1;
	
	int res = strcmp(m_FullPath, b.m_FullPath);
	if (res!=0)
		return res;

	if (m_MTime < b.m_MTime)
		return -1;
	if (m_MTime > b.m_MTime)
		return 1;

	return 0;
}

//-------------------------   TABLE OF APERTURES   ------------------------

// Default constructor
CApertures::CApertures():m_Count(0), m_Capacity(0), m_List(NULL)
{
}

// Copy constructor
CApertures::CApertures(const CApertures &orig):m_Count(0), m_Capacity(0), 
	m_List(NULL)
{
	int i, count;

	count = orig.Count();
	if (count>0) {
		m_Capacity = count;
		m_List = (CAperture**)g_malloc(m_Capacity*sizeof(CAperture*));
		for (i=0; i<count; i++) {
			const CAperture *ap = orig.Get(i);
			m_List[i] = (ap!=NULL ? new CAperture(*ap) : NULL);
		}
		m_Count = count;
	}
}

// Destructor
CApertures::~CApertures()
{
	for (int i=0; i<m_Count; i++)
		delete m_List[i];
	g_free(m_List);
}

// assignment operator
CApertures &CApertures::operator=(const CApertures &orig)
{
	int i, count;

	if (&orig!=this) {
		Clear();
		count = orig.Count();
		if (count>0) {
			m_Capacity = count;
			m_List = (CAperture**)g_malloc(m_Capacity*sizeof(CAperture*));
			for (i=0; i<count; i++) {
				const CAperture *ap = orig.Get(i);
				m_List[i] = (ap!=NULL ? new CAperture(*ap) : NULL);
			}
			m_Count = count;
		}
	}
	return *this;
}

// Clear the table
void CApertures::Clear(void)
{
	for (int i=0; i<m_Count; i++)
		delete m_List[i];
	g_free(m_List);
	m_List = NULL;
	m_Count = m_Capacity = 0;
}

// Add an aperture, if the aperture with the same id is 
// in the table, it changes its parameters
void CApertures::Add(const CAperture &item)
{
	int index = Find(item.Id());
	if (index<0) {
		if (m_Count>=m_Capacity) {
			m_Capacity += ALLOC_BY;
			m_List = (CAperture**)g_realloc(m_List, m_Capacity*sizeof(CAperture*));
		}
		m_List[m_Count++] = new CAperture(item);
	} else 
		*m_List[index] = item;
}

// Find aperture by id 
int CApertures::Find(int id) const
{
	int i;

	for (i=0; i<m_Count; i++) {
		if (m_List[i]->Id() == id)
			return i;
	}
	return -1;
}

// Return index of smallest aperture
int CApertures::FindSmallest(void) const
{
	int i, imin = -1;
	double qmin = 1e99;

	for (i=0; i<m_Count; i++) {
		if (m_List[i]->Radius() < qmin) {
			imin = i;
			qmin = m_List[i]->Radius();
		}
	}
	return imin;
}

const CAperture *CApertures::Get(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index];
	return NULL;
}

// Get aperture by index
int CApertures::GetId(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Id();
	return -1;
}

// Get aperture by index
double CApertures::GetRadius(int index) const
{
	if (index>=0 && index<m_Count)
		return m_List[index]->Radius();
	return 0.0;
}

//-----------------------   FRAME DESCRIPTOR   ------------------------------

//
// Default constructor
//
CFrameInfo::CFrameInfo(void):m_FullPath(NULL), m_Filter(NULL), m_Object(NULL),
	m_JulDat(0), m_ExpTime(-1.0), m_CCDTemp(-999.99), m_AvgFrames(0), m_SumFrames(0),
	m_Format(CMPACK_FORMAT_UNKNOWN)
{
}

//
// Copy constructor
//
CFrameInfo::CFrameInfo(const CFrameInfo &orig):m_JulDat(orig.m_JulDat), 
	m_ExpTime(orig.m_ExpTime), m_CCDTemp(orig.m_CCDTemp), m_AvgFrames(orig.m_AvgFrames),
	m_SumFrames(orig.m_SumFrames), m_Format(orig.m_Format)
{
	m_FullPath = CopyString(orig.m_FullPath);
	m_Object = CopyString(orig.m_Object);
	m_Filter = CopyString(orig.m_Filter);
}

//
// Destructor
//
CFrameInfo::~CFrameInfo(void)
{
	g_free(m_FullPath);
	g_free(m_Object);
	g_free(m_Filter);
}

//
// Initialization
//
bool CFrameInfo::Init(const gchar *filepath, GError **error)
{
	Clear();

	CCCDFile file;
	if (!file.Open(filepath, CMPACK_OPEN_READONLY, error)) 
		return false;

	m_FullPath = CopyString(filepath);
	m_Filter = CopyString(file.Filter());
	m_Object = CopyString(file.Object()->Name());
	m_JulDat = file.JulianDate();
	m_ExpTime = file.ExposureDuration();
	m_CCDTemp = file.CCDTemperature();
	m_AvgFrames = file.AvgFrames();
	m_SumFrames = file.SumFrames();
	m_Format = file.FileFormat();
	return true;
}

//
// Clear data
//
void CFrameInfo::Clear(void)
{
	g_free(m_FullPath);
	m_FullPath = NULL;
	g_free(m_Object);
	m_Object = NULL;
	g_free(m_Filter);
	m_Filter = NULL;
	m_JulDat = 0.0;
	m_ExpTime = -1.0;
	m_CCDTemp = -999.99;
	m_AvgFrames = m_SumFrames = 0;
	m_Format = CMPACK_FORMAT_UNKNOWN;
}

//
// Assignment operator
//
CFrameInfo &CFrameInfo::operator=(const CFrameInfo &orig)
{
	if (&orig!=this) {
		g_free(m_FullPath);
		m_FullPath = CopyString(orig.m_FullPath);
		g_free(m_Object);
		m_Object = CopyString(orig.m_Object);
		g_free(m_Filter);
		m_Filter = CopyString(orig.m_Filter);
		m_JulDat = orig.m_JulDat;
		m_ExpTime = orig.m_ExpTime;
		m_CCDTemp = orig.m_CCDTemp;
		m_AvgFrames = orig.m_AvgFrames;
		m_SumFrames = orig.m_SumFrames;
		m_Format = orig.m_Format;
	}
	return *this;
}

//------------------------   SELECTION LIST   -----------------------------

CSelectionList::CSelectionList(const CSelectionList &other):m_List(NULL)
{
	tData *last = NULL;
	for (const tData *ptr = other.m_List; ptr!=NULL; ptr=ptr->next) {
		tData *newData = (tData*)g_malloc0(sizeof(tData));
		if (ptr->name)
			newData->name = g_strdup(ptr->name);
		if (ptr->data)
			newData->data = new CSelection(*ptr->data);
		if (!last)
			m_List = newData;
		else
			last->next = newData;
		last = newData;
	}
}

CSelectionList::~CSelectionList(void)
{
	Clear();
}

void CSelectionList::Clear(void)
{
	tData *ptr = m_List;
	while (ptr!=NULL) {
		tData *next = ptr->next;
		g_free(ptr->name);
		delete ptr->data;
		g_free(ptr);
		ptr = next;
	}
	m_List = NULL;
}

// Add a new selection (takes ownership of the instance)
void CSelectionList::Set(const gchar *name, const CSelection &selection)
{
	tData *data = findData(name);
	if (data) {
		delete data->data;
		data->data = new CSelection(selection);
		g_free(data->name);
		data->name = (name ? g_strdup(name) : NULL);
	} else {
		tData *item = (tData*)g_malloc0(sizeof(tData));
		item->data = new CSelection(selection);
		item->name = (name ? g_strdup(name) : NULL);
		tData *insertPos = findInsertPos(name);
		if (insertPos) {
			// Insert before given item
			item->next = insertPos->next;
			insertPos->next = item;
		} else {
			// Insert before all other items
			item->next = m_List;
			m_List = item;
		}
	}
}

// Remove a selection from the list (caller must delete it)
void CSelectionList::RemoveAt(int index)
{
	int i = 0;
	tData *prevItem = NULL;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (i == index) {
			if (!prevItem) {
				// Remove the first item
				m_List = ptr->next;
			} else {
				// Remove item after prev
				prevItem->next = ptr->next;
			}
			g_free(ptr->name);
			delete ptr->data;
			g_free(ptr);
			break;
		}
		prevItem = ptr;
		i++;
	}
}

// Get number of records
int CSelectionList::Count(void) const
{
	int count = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) 
		count++;
	return count;
}

const gchar *CSelectionList::Name(int index) const
{
	int i = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (i == index) 
			return ptr->name;
		i++;
	}
	return NULL;
}

int CSelectionList::IndexOf(const gchar *name) const
{
	int i = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (StrCmp0(ptr->name, name) == 0) 
			return i;
		i++;
	}
	return -1;
}

int CSelectionList::IndexOf(const CSelection &sel) const
{
	int i = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (ptr->data && *ptr->data == sel) 
			return i;
		i++;
	}
	return -1;
}

CSelection CSelectionList::At(int index) const 
{ 
	int i = 0;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (i == index) 
			return (ptr->data ? *ptr->data : CSelection());
		i++;
	}
	return CSelection();
}

CSelectionList::tData *CSelectionList::findInsertPos(const gchar *name) const 
{ 
	tData *prevItem = NULL;
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (StrCmp0(ptr->name, name) > 0) 
			break;
		prevItem = ptr;
	}
	return prevItem;
}

CSelectionList::tData *CSelectionList::findData(const gchar *name) const
{
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (StrCmp0(ptr->name, name) == 0) 
			return ptr;
	}
	return NULL;
}

// Find selection by name
CSelection CSelectionList::Value(const gchar *name) const
{
	for (tData *ptr=m_List; ptr!=NULL; ptr=ptr->next) {
		if (StrCmp0(ptr->name, name) == 0)
			return (ptr->data ? *ptr->data : CSelection());
	}
	return CSelection();
}

// Assignment operator (makes deep copy)
CSelectionList &CSelectionList::operator=(const CSelectionList &other)
{
	if (this != &other) {
		Clear();

		tData *last = NULL;
		for (const tData *ptr = other.m_List; ptr!=NULL; ptr=ptr->next) {
			tData *newData = (tData*)g_malloc0(sizeof(tData));
			if (ptr->name)
				newData->name = g_strdup(ptr->name);
			if (ptr->data)
				newData->data = new CSelection(*ptr->data);
			if (!last)
				m_List = newData;
			else
				last->next = newData;
			last = newData;
		}
	}
	return *this;
}
