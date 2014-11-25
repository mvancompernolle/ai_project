/**************************************************************

varcat.h (C-Munipack project)
Catalogs of variable stars
Copyright (C) 2010 David Motl, dmotl@volny.cz

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

#include "varcat.h"
#include "utils.h"
#include "configuration.h"
#include "file_classes.h"

#define MAXLINE 1024

static void InsertString(char *buf, size_t buflen, const char *substr, size_t index)
{
	size_t slen = strlen(buf), sublen = strlen(substr);
	if (sublen>0 && index<=slen) {
		if (slen+sublen+1 <= buflen) {
			memmove(buf+index+sublen, buf+index, slen-index+1);
			memmove(buf+index, substr, sublen);
		} else 
		if (index+sublen+1 < buflen) {
			memmove(buf+index+sublen, buf+index, buflen-index-sublen-1);
			memmove(buf+index, substr, sublen);
			buf[buflen-1] = '\0';
		} else {
			memmove(buf+index, substr, buflen-index-1);
			buf[buflen-1] = '\0';
		}
	}
}

static char *MakePattern(const char *searchString)
{
	int state;
	const char *sptr;
	char *buf, *dptr;
	size_t len = strlen(searchString);
	if (len>0) {
		buf = (char*)g_malloc(len+1);
		for (sptr=searchString, dptr=buf, state=0; *sptr!='\0'; sptr++) {
			if (state==0) {
				if (*sptr>='1' && *sptr<='9') {
					*dptr++ = *sptr;
					state = 1;
				} else {
					if (*sptr>='A' && *sptr<='Z') 
						*dptr++ = *sptr;
					else if (*sptr>='a' && *sptr<='z')
						*dptr++ = *sptr - 'a' + 'A';
				}
			} else {
				if (*sptr>='0' && *sptr<='9') {
					*dptr++ = *sptr;
				} else {
					if (*sptr>='A' && *sptr<='Z') 
						*dptr++ = *sptr;
					else if (*sptr>='a' && *sptr<='z') 
						*dptr++ = *sptr - 'a' + 'A';
					state = 0;
				}
			}
		}
		*dptr = '\0';
		if (*buf!='\0')
			return buf;
		else
			g_free(buf);
	}
	return NULL;
}

static bool TestName(const char *pattern, const char *name)
{
	char *buf = MakePattern(name);
	if (buf) {
		int res = strcmp(buf, pattern);
		g_free(buf);
		return res==0;
	}
	return false;
}

static void StripLeadingZeros(char *buf)
{
	if (strlen(buf)>1) {
		char *sptr = buf;
		while ((*sptr=='0' || *sptr==' ') && *(sptr+1)!='\0')
			sptr++;
		if (sptr!=buf)
			memmove(buf, sptr, strlen(sptr)+1);
	}
}

static void VarCat_GCVS(const char *searchstr, tVarCatProc *cb_proc, void *cb_data)
{
	char name[MAXLINE], aux[MAXLINE], objra[MAXLINE], objdec[MAXLINE];
	char *fpath, *pattern;

	if (CConfig::GetBool(CConfig::SEARCH_GCVS)) {
		fpath = CConfig::GetStr(CConfig::GCVS_PATH);
		pattern = MakePattern(searchstr);
		if (fpath && pattern) {
			FILE *f = open_file(fpath, "r");
			if (f) {
				CTextReader reader(f);
				while (reader.ReadLine()) {
					if (reader.Length()>53 && reader.Length()<MAXLINE && 
						reader.Col(7)=='|' && reader.Col(19)=='|' && reader.Col(53)=='|') {
						// GCVS I-III 2006
						reader.GetField(8, 5, name, MAXLINE);
						strcat(name, " ");
						reader.GetField(14, 3, aux, MAXLINE);
						strcat(name, aux);
						if (TestName(pattern, name)) {
							reader.GetField(37, 8, objra, MAXLINE);
							InsertString(objra, MAXLINE, " ", 2);
							InsertString(objra, MAXLINE, " ", 5);
							reader.GetField(45, 7, objdec, MAXLINE);
							InsertString(objdec, MAXLINE, " ", 3);
							InsertString(objdec, MAXLINE, " ", 6);
							if (cb_proc)
								cb_proc(name, objra, objdec, "GCVS I-III", NULL, cb_data);
						}
					} else
					if (reader.Length()>53 && reader.Length()<MAXLINE && 
						reader.Col(7)=='|' && reader.Col(19)=='|' && reader.Col(47)=='|') {
							// GCVS I-III 2007 - 2009
							reader.GetField(8, 5, name, MAXLINE);
							strcat(name, " ");
							reader.GetField(14, 3, aux, MAXLINE);
							strcat(name, aux);
							if (TestName(pattern, name)) {
								reader.GetField(20, 8, objra, MAXLINE);
								InsertString(objra, MAXLINE, " ", 2);
								InsertString(objra, MAXLINE, " ", 5);
								reader.GetField(28, 7, objdec, MAXLINE);
								InsertString(objdec, MAXLINE, " ", 3);
								InsertString(objdec, MAXLINE, " ", 6);
								if (cb_proc)
									cb_proc(name, objra, objdec, "GCVS I-III", NULL, cb_data);
							}
					}
				}
				fclose(f);
			}
		}
		g_free(pattern);
		g_free(fpath);
	}
}

static void VarCat_NSV(const char *searchstr, tVarCatProc *cb_proc, void *cb_data)
{
	char name[MAXLINE], objra[MAXLINE], objdec[MAXLINE], comment[MAXLINE];
	char *pattern, *fpath;

	if (CConfig::GetBool(CConfig::SEARCH_NSV)) {
		fpath = CConfig::GetStr(CConfig::NSV_PATH);
		pattern = MakePattern(searchstr);
		if (pattern && fpath) {
			FILE *f = open_file(fpath, "r");
			if (f) {
				CTextReader reader(f);
				while (reader.ReadLine()) {
					if (reader.Length()>108 && reader.Length()<MAXLINE) {
						if (reader.Col(8)=='|' && reader.Col(26)=='|' && (reader.Col(108)=='|' || reader.Col(108)=='=') && (reader.Col(17)=='+' || reader.Col(17)=='-')) {
							// NSV
							reader.GetField(0, 5, name, MAXLINE);
							StripLeadingZeros(name);
							InsertString(name, MAXLINE, "NSV ", 0);
							if (TestName(pattern, name)) {
								reader.GetField(27, 8, objra, MAXLINE);
								InsertString(objra, MAXLINE, " ", 2);
								InsertString(objra, MAXLINE, " ", 5);
								reader.GetField(35, 7, objdec, MAXLINE);
								InsertString(objdec, MAXLINE, " ", 3);
								InsertString(objdec, MAXLINE, " ", 6);
								reader.GetField(81, 13, comment, MAXLINE);
								if (cb_proc)
									cb_proc(name, objra, objdec, "NSV", comment, cb_data);
								if (reader.Col(108)=='=') {
									reader.GetField(109, 10, name, MAXLINE);
									VarCat_GCVS(name, cb_proc, cb_data);
								}
							}
						} else
						if (reader.Col(8)==' ' && reader.Col(26)==' ' && reader.Col(108)=='=') {
							// Cross reference to another star
							reader.GetField(0, 5, name, MAXLINE);
							StripLeadingZeros(name);
							InsertString(name, MAXLINE, "NSV ", 0);
							if (TestName(pattern, name)) {
								reader.GetField(109, 10, name, MAXLINE);
								VarCat_GCVS(name, cb_proc, cb_data);
							}
						}
					}
				}
				fclose(f);
			}
		}
		g_free(pattern);
		g_free(fpath);
	}
}

static void VarCat_NSVS(const char *searchstr, tVarCatProc *cb_proc, void *cb_data)
{
	char name[MAXLINE], objra[MAXLINE], objdec[MAXLINE];
	char *pattern, *fpath;

	if (CConfig::GetBool(CConfig::SEARCH_NSVS)) {
		fpath = CConfig::GetStr(CConfig::NSVS_PATH);
		pattern = MakePattern(searchstr);
		if (pattern && fpath) {
			FILE *f = open_file(fpath, "r");
			if (f) {
				CTextReader reader(f);
				while (reader.ReadLine()) {
					if (reader.Length()>104 && reader.Length()<MAXLINE) {
						if (reader.Col(6)=='|' && reader.Col(23)=='|' && (reader.Col(104)=='|' || reader.Col(104)=='=') && (reader.Col(15)=='+' || reader.Col(15)=='-')) {
							// NSV Supplement
							reader.GetField(0, 5, name, MAXLINE);
							StripLeadingZeros(name);
							InsertString(name, MAXLINE, "NSV ", 0);
							if (TestName(pattern, name)) {
								reader.GetField(24, 8, objra, MAXLINE);
								InsertString(objra, MAXLINE, " ", 2);
								InsertString(objra, MAXLINE, " ", 5);
								reader.GetField(32, 7, objdec, MAXLINE);
								InsertString(objdec, MAXLINE, " ", 3);
								InsertString(objdec, MAXLINE, " ", 6);
								if (cb_proc)
									cb_proc(name, objra, objdec, "NSVS", NULL, cb_data);
								if (reader.Col(104)=='=') {
									reader.GetField(105, 10, name, MAXLINE);
									VarCat_GCVS(name, cb_proc, cb_data);
								}
							}
						}
					}
				}
				fclose(f);
			}
		}
		g_free(pattern);
		g_free(fpath);
	}
}

bool VarCat_Test(void)
{
	gboolean ok = false;
	char *fpath;

	if (!ok && CConfig::GetBool(CConfig::SEARCH_GCVS)) {
		fpath = CConfig::GetStr(CConfig::GCVS_PATH);
		ok = g_file_test(fpath, G_FILE_TEST_IS_REGULAR);
		g_free(fpath);
	}
	if (!ok && CConfig::GetBool(CConfig::SEARCH_NSV)) {
		fpath = CConfig::GetStr(CConfig::NSV_PATH);
		ok = g_file_test(fpath, G_FILE_TEST_IS_REGULAR);
		g_free(fpath);
	}
	if (!ok && CConfig::GetBool(CConfig::SEARCH_NSVS)) {
		fpath = CConfig::GetStr(CConfig::NSVS_PATH);
		ok = g_file_test(fpath, G_FILE_TEST_IS_REGULAR);
		g_free(fpath);
	}
	return ok!=0;
}

void VarCat_Search(const char *searchstr, tVarCatProc *cb_proc, void *cb_data)
{
	VarCat_GCVS(searchstr, cb_proc, cb_data);
	VarCat_NSV(searchstr, cb_proc, cb_data);
	VarCat_NSVS(searchstr, cb_proc, cb_data);
}
