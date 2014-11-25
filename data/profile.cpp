/**************************************************************

profile.cpp (C-Munipack project)
Project options
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
#include "profile.h"
#include "utils.h"
#include "main.h"

// Aperture size limits (pixels)
#define MIN_APERTURE	1.0
#define MAX_APERTURE	100.0

// Max. border size (pixels)
#define MAX_BORDER		100000

// File revision identifier
#define FILE_REVISION	1

// Project types
static const struct {
	const gchar *id;
	const gchar *caption;
	const gchar *icon;
	const gchar *rc_group;
} ProjectTypes[EndOfProjectTypes] = {
	{ "Reduce",		"Light curve",								"lightcurve16", "Reduce Project" },
	{ "MasterBias", "Master bias frame",						"biascorr16",	"MasterBias Project" },
	{ "MasterDark", "Master dark frame",						"darkcorr16",	"MasterDark Project" },	
	{ "MasterFlat", "Master flat frame",						"flatcorr16",	"MasterFlat Project" },
	{ "Kombine",	"CCD frame merging",						"merge16",		"Kombine Project" },
	{ "Test",		"Test and debug (all features enabled)",	"photometry16",	"Test Project" }
};

// Default apertures (pixels)
static const double DefApertureRadius[MAX_APERTURES] = 
{ 
	2.0, 2.7273, 3.8182, 5.2727, 7.0909, 9.2727, 11.8182, 14.7273, 18.0, 21.6364, 25.6364, 30.0 
};

static const struct tParamDef {
	const char *group, *keyword;
	GType type;
	const char *defval;
	double min, max;
	int maxlength;
} Params[CProfile::EndOfParameters] = {
	{ "CCD", "FlipV", G_TYPE_BOOLEAN, "0" },
	{ "CCD", "FlipH", G_TYPE_BOOLEAN, "0" },
	{ "CCD", "Binning", G_TYPE_INT, "1", 1, 8, 1 },
	{ "CCD", "TimeOffset", G_TYPE_INT, "0", -12*3600, 12*3600, 6 },
	{ "CCD", "MinValue", G_TYPE_DOUBLE, "0", LONG_MIN, LONG_MAX, 11 },
	{ "CCD", "MaxValue", G_TYPE_DOUBLE, "65535.0", LONG_MIN, LONG_MAX, 11 },
	{ "Conversion", "Format", G_TYPE_INT, "0", -64, 64, 3 },
	{ "Calibration", "Advanced", G_TYPE_BOOLEAN, "0" },
	{ "MasterBias", "Format", G_TYPE_INT, "0", -64, 64, 3 },
	{ "MasterDark", "Format", G_TYPE_INT, "0", -64, 64, 3 },
	{ "MasterFlat", "Level", G_TYPE_DOUBLE, "10000.0", LONG_MIN, LONG_MAX, 11 },
	{ "MasterFlat", "Format", G_TYPE_INT, "0", -64, 64, 3 },
	{ "Kombine", "Format", G_TYPE_INT, "0", -64, 64, 3 },
	{ "Photometry", "ReadNoise", G_TYPE_DOUBLE, "15.0", 0.1, 100, 4 },
	{ "Photometry", "ADCGain", G_TYPE_DOUBLE, "2.3", 1e-6, 100, 4 },
	{ "Photometry", "Fwhm", G_TYPE_DOUBLE, "3.0", 1, 100, 4 },
	{ "Photometry", "Threshold", G_TYPE_DOUBLE, "4.0", 0.1, 100, 4 },
	{ "Photometry", "MinRoundness", G_TYPE_DOUBLE, "-1.0", -100, 100, 5 },
	{ "Photometry", "MaxRoundness", G_TYPE_DOUBLE, "1.0", -100, 100, 5 },
	{ "Photometry", "MinSharpness", G_TYPE_DOUBLE, "0.2", 0, 100, 4 },
	{ "Photometry", "MaxSharpness", G_TYPE_DOUBLE, "1.0", 0, 100, 4 },
	{ "Photometry", "SkyInRadius", G_TYPE_DOUBLE, "20.0", 1, 1000, 5 },
	{ "Photometry", "SkyOutRadius", G_TYPE_DOUBLE, "30.0", 1, 1000, 5 },
	{ "Matching", "RStars", G_TYPE_INT, "10", 10, 1000, 2 },
	{ "Matching", "IStars", G_TYPE_INT, "5", 3, 20, 2 },
	{ "Matching", "Clip", G_TYPE_DOUBLE, "2.5", 0.1, 100, 2 },
	{ "Matching", "SpFields", G_TYPE_BOOLEAN, "0", 0, 1, 1 },
	{ "Matching", "MaxOffset", G_TYPE_DOUBLE, "2", 0, 1000, 3 },
	{ "Plotting", "CompStars", G_TYPE_BOOLEAN, "0" },
	{ "MuniFind", "Threshold", G_TYPE_DOUBLE, "60.0", 0, 100, 4 }
};

static GMutex	*gs_mutex = NULL;
static gchar	*gs_profilesDir = NULL;

static const struct {
	const gchar *name;
	tProjectType type;
} DefaultProfiles[] = {
	{ "Light curve", PROJECT_REDUCE },
	{ "Master bias frame", PROJECT_MASTER_BIAS },
	{ "Master dark frame", PROJECT_MASTER_DARK },
	{ "Master flat frame", PROJECT_MASTER_FLAT },
	{ "CCD frame merging", PROJECT_COMBINING },
	{ "Test and debug", PROJECT_TEST },
	{ NULL }
};

//----------------------   PUBLIC FUNCTIONS  -------------------------

// Printable name for given project type
const gchar *ProjectTypeCaption(tProjectType type)
{
	if (type>=0 && type<EndOfProjectTypes)
		return ProjectTypes[type].caption;
	return NULL;
}

// Project type to string identifier
const gchar *ProjectTypeToStr(tProjectType type)
{
	if (type>=0 && type<EndOfProjectTypes)
		return ProjectTypes[type].id;
	return NULL;
}

// String identifier to project type
tProjectType StrToProjectType(const gchar *str)
{
	for (int i=0; i<EndOfProjectTypes; i++) {
		if (StrCaseCmp0(str, ProjectTypes[i].id)==0)
			return (tProjectType)i;
	}
	if (StrCaseCmp0(str, "Variable star observation")==0)
		return PROJECT_REDUCE;
	return EndOfProjectTypes;
}

// Get recent list for given project type
const gchar *ProjectTypeRecentGroup(tProjectType type)
{
	if (type>=0 && type<EndOfProjectTypes)
		return ProjectTypes[type].rc_group;
	return NULL;
}

// Get recent list for given project type
const gchar *ProjectTypeIcon(tProjectType type)
{
	if (type>=0 && type<EndOfProjectTypes)
		return ProjectTypes[type].icon;
	return NULL;
}

// Initialize shared variables
void ProfilesInitGlobals(void)
{
	if (!gs_mutex) 
		gs_mutex = g_mutex_new();
}

// Release allocated memory in shared variables
void ProfilesFreeGlobals(void)
{
	if (gs_mutex) {
		g_mutex_lock(gs_mutex);
		g_free(gs_profilesDir);
		g_mutex_unlock(gs_mutex);
		g_mutex_free(gs_mutex);
		gs_mutex = NULL;
	}
}

// Get path to the directory with profiles
const gchar *UserProfilesDir(void)
{
	g_mutex_lock(gs_mutex);
	gchar *usrpath = CConfig::GetStr(CConfig::USER_PROFILES);
	if (!usrpath || usrpath[0]=='\0') 
		usrpath = g_build_filename(get_user_data_dir(), "Profiles", NULL); // First letter is uppercase!
	if (!gs_profilesDir || strcmp(gs_profilesDir, usrpath)) {
		g_free(gs_profilesDir);
		gs_profilesDir = usrpath;
	}
	g_mutex_unlock(gs_mutex);
	return gs_profilesDir;
}

// Get list of user-defined / default profiles
GSList *GetProfileList(const gchar *dirpath)
{
	GSList *retval = NULL;
	GDir *dir = g_dir_open(dirpath, 0, NULL);
	if (dir) {
		const gchar *filename = g_dir_read_name(dir);
		while (filename) {
			retval = g_slist_prepend(retval, g_build_filename(dirpath, filename, NULL));
			filename = g_dir_read_name(dir);
		}
		g_dir_close(dir);
	}
	return retval;
}

// Free memory allocated in the profile list
void ProfileListFree(GSList *list)
{
	g_slist_foreach(list, (GFunc)g_free, NULL);
	g_slist_free(list);
}

// Get list of default profiles
GSList *DefaultProfileList(void) 
{ 
	GSList *list = NULL;
	for (int i=0; DefaultProfiles[i].name; i++) 
		list = g_slist_prepend(list, g_strdup(DefaultProfiles[i].name));
	return g_slist_reverse(list);
}

// Get default profile
CProfile CProfile::DefaultProfile(const gchar *name) 
{
	CProfile profile; 
	for (int i=0; DefaultProfiles[i].name; i++) {
		if (strcmp(name, DefaultProfiles[i].name)==0) {
			profile.SetProjectType(DefaultProfiles[i].type);
			break;
		}
	}
	return profile;
}

//----------------------   PROFILE CLASS METHODS  -------------------------

// Constructor
CProfile::CProfile(void)
{
	m_File = g_key_file_new();
}

// Destructor
CProfile::~CProfile(void)
{
	g_key_file_free(m_File);
}

// Copy constructor
CProfile::CProfile(const CProfile &other)
{
	gsize length;

	m_File = g_key_file_new();
	gchar *data = g_key_file_to_data(other.m_File, &length, NULL);
	if (data && length>0) 
		g_key_file_load_from_data(m_File, data, length, G_KEY_FILE_NONE, NULL);
}

// Assigment operator
CProfile &CProfile::operator =(const CProfile &other)
{
	Lock();
	if (&other!=this) {
		gsize length;
		gchar *data = g_key_file_to_data(other.m_File, &length, NULL);
		if (data && length>0) 
			g_key_file_load_from_data(m_File, data, length, G_KEY_FILE_NONE, NULL);
		else
			Clear();
		g_free(data);
	}
	Unlock();
	return *this;
}

// Lock data
void CProfile::Lock(void) const
{
	gdk_threads_lock();
}

// Unlock data
void CProfile::Unlock(void) const
{
	gdk_threads_unlock();
}

// Set all parameters to defaults
void CProfile::Clear(void)
{
	Lock();
	g_key_file_free(m_File);
	m_File = g_key_file_new();
	Unlock();
}

// Load project settings from key-value file
void CProfile::Load(GKeyFile *src)
{
	Lock();
	gchar **grps = g_key_file_get_groups(src, NULL);
	if (grps) {
		for (int i=0; grps[i]!=NULL; i++) {
			if (CProfile::IsProfileGroup(grps[i])) {
				gchar **keys = g_key_file_get_keys(src, grps[i], NULL, NULL);
				if (keys) {
					for (int j=0; keys[j]!=NULL; j++) {
						gchar *value = g_key_file_get_value(src, grps[i], keys[j], NULL);
						if (value && value[0]!='\0') 
							g_key_file_set_value(m_File, grps[i], keys[j], value);
						g_free(value);
					}
					g_strfreev(keys);
				}
			}
		}
		g_strfreev(grps);
	}
	Unlock();
}

// Save project settings to a key-value file
void CProfile::Save(GKeyFile *dst) const
{
	Lock();
	gchar **grps = g_key_file_get_groups(m_File, NULL);
	if (grps) {
		for (int i=0; grps[i]!=NULL; i++) {
			if (CProfile::IsProfileGroup(grps[i])) {
				gchar **keys = g_key_file_get_keys(m_File, grps[i], NULL, NULL);
				if (keys) {
					for (int j=0; keys[j]!=NULL; j++) {
						gchar *value = g_key_file_get_value(m_File, grps[i], keys[j], NULL);
						if (value && value[0]!='\0') 
							g_key_file_set_value(dst, grps[i], keys[j], value);
						g_free(value);
					}
					g_strfreev(keys);
				}
			}
		}
		g_strfreev(grps);
	}
	Unlock();
}

// Check if the section belongs to profile
bool CProfile::IsProfileGroup(const gchar *group)
{
	if (StrCaseCmp0(group, "Profile")==0 || StrCaseCmp0(group, "Observer")==0)
		return true;
	for (int p=0; p<CProfile::EndOfParameters; p++) {
		if (StrCaseCmp0(Params[p].group, group)==0)
			return true;
	}
	return false;
}

// Get project type
tProjectType CProfile::ProjectType(void) const
{
	tProjectType retval;
	gchar *str = GetStr("Profile", "Type");
	retval = StrToProjectType(str);
	g_free(str);
	return retval;
}

// Set project type
void CProfile::SetProjectType(tProjectType type)
{
	SetStr("Profile", "Type", ProjectTypeToStr(type));
}

// Get aperture radius
CApertures CProfile::Apertures(void) const
{
	gsize length;
	double *apertures;
	CApertures aper;

	Lock();
	apertures = g_key_file_get_double_list(m_File, "Photometry", "Apertures", &length, NULL);
	Unlock();

	if (apertures) {
		for (int i=0; i<MAX_APERTURES; i++) {
			if (i<(int)length && apertures[i]>0) {
				double radius = LimitValue(apertures[i], MIN_APERTURE, MAX_APERTURE);
				aper.Add(CAperture(i+1, radius));
			}
		}
	} else {
		for (int i=0; i<MAX_APERTURES; i++) 
			aper.Add(CAperture(i+1, DefApertureRadius[i]));
	}

	return aper;
}

void CProfile::SetApertures(const CApertures &aper)
{
	int i;
	double apertures[MAX_APERTURES];

	memset(apertures, 0, MAX_APERTURES*sizeof(double));
	for (i=0; i<aper.Count(); i++) {
		int index = aper.GetId(i)-1;
		if (index>=0 && index<MAX_APERTURES)
			apertures[index] = aper.GetRadius(i);
	}

	Lock();
	g_key_file_set_double_list(m_File, "Photometry", "Apertures", apertures, MAX_APERTURES);
	Unlock();
}

void CProfile::GetApertureSizeLimits(double *min, double *max)
{
	if (min)
		*min = MIN_APERTURE;
	if (max)
		*max = MAX_APERTURE;
}

CApertures CProfile::DefaultApertures(void)
{
	int i;
	double radius;
	CApertures aper;

	for (i=0; i<MAX_APERTURES; i++) {
		radius = DefApertureRadius[i];
		if (radius>0.0)
			aper.Add(CAperture(i+1, radius));
	}

	return aper;
}

CmpackBorder CProfile::GetBorder(void) const
{
	CmpackBorder border;
	int *borders;
	gsize length;

	Lock();
	borders = g_key_file_get_integer_list(m_File, "CCD", "Borders", &length, NULL);
	Unlock();

	memset(&border, 0, sizeof(CmpackBorder));
	if (length>0 && borders) {
		cmpack_border_set(&border, 
			LimitValue((length>=1 ? borders[0] : 0), 0, MAX_BORDER),
			LimitValue((length>=2 ? borders[1] : 0), 0, MAX_BORDER),
			LimitValue((length>=3 ? borders[2] : 0), 0, MAX_BORDER),
			LimitValue((length>=4 ? borders[3] : 0), 0, MAX_BORDER));
	}
	g_free(borders);
	return border;
}

void CProfile::SetBorder(const CmpackBorder &border)
{
	int borders[4];

	borders[0] = border.left;
	borders[1] = border.top;
	borders[2] = border.right;
	borders[3] = border.bottom;

	Lock();
	g_key_file_set_integer_list(m_File, "CCD", "Borders", borders, 4);
	Unlock();
}

void CProfile::GetBorderSizeLimits(int *min, int *max)
{
	if (min)
		*min = 0;
	if (max)
		*max = MAX_BORDER;
}

void CProfile::SetStr(tParameter param, const gchar *value)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_STRING);

	const tParamDef *p = &Params[param];
	SetStr(p->group, p->keyword, value);
}

void CProfile::SetInt(tParameter param, int value)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_INT);

	const tParamDef *p = &Params[param];
	SetInt(p->group, p->keyword, value);
}

void CProfile::SetDbl(tParameter param, double value)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_DOUBLE);

	const tParamDef *p = &Params[param];
	SetDbl(p->group, p->keyword, value);
}

void CProfile::SetBool(tParameter param, bool value)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_BOOLEAN);

	const tParamDef *p = &Params[param];
	SetBool(p->group, p->keyword, value);
}

char *CProfile::GetStr(tParameter param) const
{
	char *value;

	g_return_val_if_fail(param>=0 && param<EndOfParameters, NULL);
	g_return_val_if_fail(Params[param].type == G_TYPE_STRING, NULL);

	const tParamDef *p = &Params[param];
	if (TryGetStr(p->group, p->keyword, &value)) 
		return value;
	else if (p->defval)
		return g_strdup(p->defval);
	else
		return g_strdup("");
}

int CProfile::GetInt(tParameter param) const
{
	int value;

	g_return_val_if_fail(param>=0 && param<EndOfParameters, 0);
	g_return_val_if_fail(Params[param].type == G_TYPE_INT, 0);

	const tParamDef *p = &Params[param];
	if (TryGetInt(p->group, p->keyword, &value)) 
		return LimitValue(value, (int)p->min, (int)p->max);
	else if (p->defval)
		return LimitValue(atoi(p->defval), (int)p->min, (int)p->max);
	else
		return MAX(0, (int)p->min);
}

double CProfile::GetDbl(tParameter param) const
{
	double value;

	g_return_val_if_fail(param>=0 && param<EndOfParameters, 0);
	g_return_val_if_fail(Params[param].type == G_TYPE_DOUBLE, 0);

	const tParamDef *p = &Params[param];
	if (TryGetDbl(p->group, p->keyword, &value)) 
		return LimitValue(value, p->min, p->max);
	else if (p->defval)
		return LimitValue(atof(p->defval), p->min, p->max);
	else
		return MAX(0.0, p->min);
}

bool CProfile::GetBool(tParameter param) const
{
	bool value;

	g_return_val_if_fail(param>=0 && param<EndOfParameters, false);
	g_return_val_if_fail(Params[param].type == G_TYPE_BOOLEAN, false);

	const tParamDef *p = &Params[param];
	if (TryGetBool(p->group, p->keyword, &value)) 
		return value;
	else if (p->defval)
		return atoi(p->defval)!=0;
	else
		return false;
}

bool CProfile::GetDefaultBool(tParameter param)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, false);
	g_return_val_if_fail(Params[param].type == G_TYPE_BOOLEAN, false);

	const tParamDef *p = &Params[param];
	if (p->defval)
		return atoi(p->defval)!=0;
	else
		return false;
}

int CProfile::GetDefaultInt(tParameter param)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, 0);
	g_return_val_if_fail(Params[param].type == G_TYPE_INT, 0);

	const tParamDef *p = &Params[param];
	if (p->defval)
		return atoi(p->defval);
	else
		return 0;
}

int CProfile::BoundInt(tParameter param, int value)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, value);
	g_return_val_if_fail(Params[param].type == G_TYPE_INT, value);

	const tParamDef *p = &Params[param];
	return LimitValue(value, (int)p->min, (int)p->max);
}

const gchar *CProfile::GetDefaultStr(tParameter param)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, 0);
	g_return_val_if_fail(Params[param].type == G_TYPE_STRING, 0);

	const tParamDef *p = &Params[param];
	return (p->defval ? p->defval : "");
}

double CProfile::GetDefaultDbl(tParameter param)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, 0);
	g_return_val_if_fail(Params[param].type == G_TYPE_DOUBLE, 0);

	const tParamDef *p = &Params[param];
	if (p->defval)
		return atof(p->defval);
	else
		return 0;
}

void CProfile::GetLimitsDbl(tParameter param, double *min, double *max)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_DOUBLE);

	const tParamDef *p = &Params[param];
	if (min) 
		*min = p->min;
	if (max)
		*max = p->max;
}

double CProfile::BoundDbl(tParameter param, double value)
{
	g_return_val_if_fail(param>=0 && param<EndOfParameters, value);
	g_return_val_if_fail(Params[param].type == G_TYPE_DOUBLE, value);

	const tParamDef *p = &Params[param];
	return LimitValue(value, p->min, p->max);
}

void CProfile::GetLimitsInt(tParameter param, int *min, int *max)
{
	g_return_if_fail(param>=0 && param<EndOfParameters);
	g_return_if_fail(Params[param].type == G_TYPE_INT);

	const tParamDef *p = &Params[param];
	if (min) 
		*min = (int)p->min;
	if (max)
		*max = (int)p->max;
}

bool CProfile::Import(const gchar *filepath, GError **error)
{
	bool retval = FALSE;
	GKeyFile *f = g_key_file_new();

	if (g_key_file_load_from_file(f, filepath, G_KEY_FILE_NONE, error)) {
		if (g_key_file_has_group(f, "CMUNIPACK-PROFILE")) {
			int revision = g_key_file_get_integer(f, "CMUNIPACK-PROFILE", "Revision", NULL);
			if (revision!=FILE_REVISION) 
				g_set_error(error, g_AppError, 2, "Unsupported revision number (%d)", revision);
			else {
				Load(f);
				retval = true;
			}
		} else
		if (g_key_file_has_group(f, "CMUNIPACK-PROJECT")) {
			int revision = g_key_file_get_integer(f, "CMUNIPACK-PROJECT", "Revision", NULL);
			if (revision!=FILE_REVISION) 
				g_set_error(error, g_AppError, 3, "Unsupported revision number (%d)", revision);
			else {
				Load(f);
				retval = true;
			}
		} else {
			g_set_error(error, g_AppError, 4, "The file '%s' is not a valid project file.", filepath);
		}
	}

	g_key_file_free(f);
	return retval;
}

bool CProfile::isProfileFile(const gchar *filepath, tProjectType *type)
{
	bool retval = false;
	GKeyFile *f = g_key_file_new();
	if (g_key_file_load_from_file(f, filepath, G_KEY_FILE_NONE, NULL)) 
		retval = isProfileFile(f, type);
	g_key_file_free(f);
	return retval;
}

bool CProfile::isProfileFile(GKeyFile *file, tProjectType *type)
{
	if (g_key_file_has_group(file, "CMUNIPACK-PROFILE")) {
		int revision = g_key_file_get_integer(file, "CMUNIPACK-PROFILE", "Revision", NULL);
		if (revision==FILE_REVISION) {
			if (type) {
				gchar *str = g_key_file_get_string(file, "Profile", "Type", NULL);
				*type = StrToProjectType(str);
				g_free(str);
			}
			return true;
		}
	}
	return false;
}

bool CProfile::Export(const gchar *filepath, GError **perror) const
{
	bool retval = false;
	gsize length;
	GKeyFile *f = g_key_file_new();

	g_key_file_set_integer(f, "CMUNIPACK-PROFILE", "Revision", FILE_REVISION);

	Save(f);

	gchar *data = g_key_file_to_data(f, &length, perror);
	if (data) {
		GIOChannel *pFile = g_io_channel_new_file(filepath, "w+", perror);
		if (pFile) {
			g_io_channel_write_chars(pFile, data, length, NULL, perror);
			g_io_channel_unref(pFile);
			retval = true;
		}
		g_free(data);
	}
	g_key_file_free(f);
	return retval;
}

CLocation CProfile::DefaultLocation(void) const
{
	CLocation obs;
	char *name, *lon, *lat;

	Lock();
	if (TryGetStr("Observer", "Location", &name)) {
		obs.SetName(g_strstrip(name));
		g_free(name);
	}
	if (TryGetStr("Observer", "Longitude", &lon)) {
		obs.SetLon(g_strstrip(lon));
		g_free(lon);
	}
	if (TryGetStr("Observer", "Latitude", &lat)) {
		obs.SetLat(g_strstrip(lat));
		g_free(lat);
	}
	Unlock();

	return obs;
}

void CProfile::SetDefaultLocation(const CLocation &l)
{
	Lock();

	const gchar *name = l.Name();
	if (name) {
		gchar *aux = g_strdup(name);
		SetStr("Observer", "Location", g_strstrip(aux));
		g_free(aux);
	} else
		SetStr("Observer", "Location", NULL);

	const gchar *lon = l.Lon();
	if (lon) {
		gchar *aux = g_strdup(lon);
		SetStr("Observer", "Longitude", g_strstrip(aux));
		g_free(aux);
	} else 
		SetStr("Observer", "Longitude", NULL);

	const gchar *lat = l.Lat();
	if (lat) {
		gchar *aux = g_strdup(lat);
		SetStr("Observer", "Latitude", g_strstrip(aux));
		g_free(aux);
	} else
		SetStr("Observer", "Latitude", NULL);

	Unlock();
}

void CProfile::SetStr(const char *group, const char *key, const char *val)
{
	Lock();
	g_key_file_set_string(m_File, group, key, (val!=NULL ? val : ""));
	Unlock();
}

void CProfile::SetDbl(const char *group, const char *key, const double val)
{
	Lock();
	g_key_file_set_double(m_File, group, key, val);
	Unlock();
}

void CProfile::SetInt(const char *group, const char *key, const int val)
{
	Lock();
	g_key_file_set_integer(m_File, group, key, val);
	Unlock();
}

void CProfile::SetBool(const char *group, const char *key, bool val)
{
	Lock();
	g_key_file_set_boolean(m_File, group, key, val);
	Unlock();
}

char *CProfile::GetStr(const char *group, const char *key, const char *defval) const
{
	char *res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_string(m_File, group, key, &error);
	if (error) {
		res = (defval ? g_strdup(defval) : NULL);
		g_error_free(error);
	}
	Unlock();
	return res;
}

bool CProfile::TryGetStr(const char *group, const char *key, char **value) const
{
	bool retval;
	char *res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_string(m_File, group, key, &error);
	if (error) {
		if (value)
			*value = NULL;
		retval = false;
		g_error_free(error);
	} else {
		if (value)
			*value = g_strdup(res);
		retval = true;
	}
	Unlock();
	return retval;
}

double CProfile::GetDbl(const char *group, const char *key, double defval) const
{
	double res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_double(m_File, group, key, &error);
	if (error) {
		res = defval;
		g_error_free(error);
	}
	Unlock();
	return res;
}

bool CProfile::TryGetDbl(const char *group, const char *key, double *value) const
{
	bool retval;
	double res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_double(m_File, group, key, &error);
	if (error) {
		if (value)
			*value = 0;
		g_error_free(error);
		retval = false;
	} else {
		if (value)
			*value = res;
		retval = true;
	}
	Unlock();
	return retval;
}

int CProfile::GetInt(const char *group, const char *key, const int defval) const
{
	int res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_integer(m_File, group, key, &error);
	if (error) {
		res = defval;
		g_error_free(error);
	}
	Unlock();
	return res;
}

bool CProfile::TryGetInt(const char *group, const char *key, int *value) const
{
	bool retval;
	int res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_integer(m_File, group, key, &error);
	if (error) {
		if (value)
			*value = 0;
		retval = false;
		g_error_free(error);
	} else {
		if (value)
			*value = res;
		retval = true;
	}
	Unlock();
	return retval;
}

bool CProfile::GetBool(const char *group, const char *key, const bool defval) const
{
	bool res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_boolean(m_File, group, key, &error)!=0;
	if (error) {
		res = defval;
		g_error_free(error);
	}
	Unlock();
	return res;
}

bool CProfile::TryGetBool(const char *group, const char *key, bool *value) const
{
	bool retval, res;
	GError *error = NULL;

	Lock();
	res = g_key_file_get_boolean(m_File, group, key, &error)!=0;
	if (error) {
		if (value)
			*value = false;
		retval = false;
		g_error_free(error);
	} else {
		if (value)
			*value = res;
		retval = true;
	}
	Unlock();
	return retval;
}
