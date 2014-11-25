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

#include "proc_classes.h"
#include "catfile_class.h"
#include "configuration.h"
#include "configuration.h"
#include "profile.h"
#include "progress_dlg.h"
#include "utils.h"
#include "main.h"

//--------------------------   CONVERSION CONTEXT   --------------------------

//
// Constructor
//
CConvertProc::CConvertProc(void):m_Progress(NULL), m_Bitpix(CMPACK_BITPIX_AUTO),
	m_Channel(CMPACK_CHANNEL_DEFAULT), m_FlipV(0), m_FlipH(0), m_Binning(1), m_TimeOffset(0)
{
	m_Konv = cmpack_konv_init();
}


//
// Destructor
//
CConvertProc::~CConvertProc()
{
	cmpack_konv_destroy(m_Konv);
}


//
// Initialization
//
int CConvertProc::Init(CConsole *pProgress)
{
	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_konv_set_console(m_Konv, con);
		cmpack_con_destroy(con);
	}

	// Check directory
	force_directory(g_Project->DataDir());

	// Data format
	m_Bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::WORK_FORMAT);
	cmpack_konv_set_bitpix(m_Konv, m_Bitpix);

	// Frame border
	m_Border = g_Project->Profile()->GetBorder();
	cmpack_konv_set_border(m_Konv, &m_Border);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_konv_set_thresholds(m_Konv, minvalue, maxvalue);

	// Binning
	m_Binning = g_Project->Profile()->GetInt(CProfile::BINNING);
	cmpack_konv_set_binning(m_Konv, m_Binning, m_Binning);

	// Transposition
	m_FlipH = g_Project->Profile()->GetBool(CProfile::FLIP_H);
	m_FlipV = g_Project->Profile()->GetBool(CProfile::FLIP_V);
	cmpack_konv_set_transposition(m_Konv, m_FlipH, m_FlipV);

	// Color to grayscale conversion
	m_Channel = (CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT);
	cmpack_konv_set_channel(m_Konv, m_Channel);

	// Time offset
	m_TimeOffset = g_Project->Profile()->GetInt(CProfile::TIME_OFFSET);
	cmpack_konv_set_toffset(m_Konv, m_TimeOffset);
	
	return 0;
}


//
// Convert file
//
int CConvertProc::Execute(GtkTreePath *pPath)
{
	int res, fileid, avg_frames, sum_frames;
	char tmp[128], *filter, *fpath, *tpath;
	double jd, exptime, ccdtemp, toffset;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;
	CCCDFile infile;

	res = 0;
	jd = exptime = 0.0;
	ccdtemp = -999.9;
	filter = NULL;
	avg_frames = sum_frames = 1;
	toffset = m_TimeOffset;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Get frame data
	gtk_tree_model_get(pList, &iter, FRAME_ID, &fileid, FRAME_ORIGFILE, &fpath, -1);

	// Clear internal frame data
	gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_STATE, 0, FRAME_FLAGS, 0,
		FRAME_JULDAT, 0.0, FRAME_TEMPFILE, NULL, FRAME_PHOTFILE, NULL, 
		FRAME_REPORT, NULL, FRAME_BIASFILE, NULL, FRAME_DARKFILE, NULL, 
		FRAME_FLATFILE, NULL, FRAME_FILTER, NULL, FRAME_TIMECORR, toffset, 
		FRAME_EXPTIME, -1.0, FRAME_AVGFRAMES, 0, FRAME_SUMFRAMES, 0, FRAME_CCDTEMP, -999.9, 
		FRAME_STARS, 0, FRAME_MSTARS, 0, FRAME_OFFSET_X, 0.0, FRAME_OFFSET_Y, 0.0,
		FRAME_THUMBNAIL, NULL, -1);

	// Make temporary file name
	sprintf(tmp, "tmp%05d.%s", fileid, FILE_EXTENSION_FITS);
	tpath = g_build_filename(g_Project->DataDir(), tmp, NULL);

	// Open source file
	GError *error = NULL;
	if (infile.Open(fpath, CMPACK_OPEN_READONLY, &error)) {
		if (infile.isWorkingFormat() && (m_Binning==1) && (m_TimeOffset==0) && 
			(m_Bitpix==CMPACK_BITPIX_AUTO || infile.Depth()==m_Bitpix) &&
			(m_Channel==CMPACK_CHANNEL_DEFAULT) && (m_FlipV==0) && (m_FlipH==0) && 
			(m_Border.left==0 && m_Border.top==0 && m_Border.bottom==0 && m_Border.right==0)) {
				// We can make a simple copy, no transformation is required
				infile.Close();
				if (!copy_file(fpath, tpath, false, &error)) 
					res = -1;
		} else {
			// Conversion
			CmpackCcdFile *outfile;
			char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
			if (res==0) {
				res = cmpack_konv(m_Konv, infile.Handle(), outfile);
				cmpack_ccd_destroy(outfile);
			}
			g_free(f);
		}
		if (res==0) {
			// Read frame parameters
			CCCDFile tmpfile;
			if (tmpfile.Open(tpath, CMPACK_OPEN_READONLY, &error)) {
				jd = tmpfile.JulianDate();
				exptime = tmpfile.ExposureDuration();
				ccdtemp = tmpfile.CCDTemperature();
				avg_frames = tmpfile.AvgFrames();
				sum_frames = tmpfile.SumFrames();
				filter = g_strdup(tmpfile.Filter());
			} else
				res = -1;
		}
	} else
		res = -1;

	if (res<0) {
		// Failed with Gtk error
		if (error) {
			m_Progress->Print(error->message);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, error->message, CFILE_CONVERSION | CFILE_PHOTOMETRY | CFILE_MATCHING, 0);
			g_error_free(error);
		}
	} else
	if (res>0) {
		// Failed with C-Munipack error code
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, CFILE_CONVERSION | CFILE_PHOTOMETRY | CFILE_MATCHING, 0);
		cmpack_free(msg);
	} else {
		// OK 
		if (jd>0 && exptime>0)
			jd += 0.5*exptime/86400.0;
		m_Progress->Print("Conversion OK");
		g_Project->SetResult(&iter, "Conversion OK", CFILE_CONVERSION | CFILE_PHOTOMETRY | 
			CFILE_MATCHING, CFILE_CONVERSION);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_TEMPFILE, tmp, FRAME_JULDAT, jd, 
			FRAME_FILTER, filter, FRAME_EXPTIME, exptime, FRAME_CCDTEMP, ccdtemp, 
			FRAME_AVGFRAMES, avg_frames, FRAME_SUMFRAMES, sum_frames, -1);
	}
	
	g_free(filter);
	g_free(fpath);
	g_free(tpath);
	return res;
}

//-----------------------   BIAS CORRECTION CONTEXT   ----------------------

//
// Constructor
//
CBiasCorrProc::CBiasCorrProc():m_Progress(NULL), m_BiasFile(NULL)
{
	m_Bias = cmpack_bias_init();
}


//
// Destructor
//
CBiasCorrProc::~CBiasCorrProc()
{
	cmpack_bias_destroy(m_Bias);
	g_free(m_BiasFile);
}

//
// Check temporary bias file
//
bool CBiasCorrProc::CheckBiasFile(const gchar *orig_file)
{
	const CFileInfo *orig = g_Project->GetOrigBiasFile(),
		*temp = g_Project->GetTempBiasFile();

	if (!orig->Valid() || !temp->Valid()) 
		return false;
	if (orig->Compare(CFileInfo(orig_file))!=0) 
		return false;
	if (temp->Compare(CFileInfo(temp->FullPath()))!=0) 
		return false;
	return true;
}

//
// Initialization
//
int CBiasCorrProc::Init(CConsole *pProgress, const gchar *fbias)
{
	if (!fbias || !g_file_test(fbias, G_FILE_TEST_IS_REGULAR))
		return CMPACK_ERR_CANT_OPEN_BIAS;
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_bias_set_console(m_Bias, con);
		cmpack_con_destroy(con);
	}

	g_free(m_BiasFile);
	m_BiasFile = g_strdup(fbias);

	// Check directory
	force_directory(g_Project->DataDir());

	char tmp[512];
	sprintf(tmp, "Bias frame: %s", fbias);
	m_Progress->Print(tmp);

	// Convert bias frame
	int res = 0;
	if (!CheckBiasFile(fbias)) {
		gchar *temp = g_build_filename(g_Project->DataDir(), "bias." FILE_EXTENSION_FITS, NULL);
		res = ConvertBiasFrame(pProgress, fbias, temp);
		g_free(temp);
	}
	if (res==0) {
		// Load bias frame
		const gchar *tpath = g_Project->GetTempBiasFile()->FullPath();
		CmpackCcdFile *bias;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&bias, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_bias_rbias(m_Bias, bias);
			cmpack_ccd_destroy(bias);
		}
		g_free(f);
	}

	if (res<0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// OK
		CmpackBorder border(g_Project->Profile()->GetBorder());
		cmpack_bias_set_border(m_Bias, &border);
		double minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
		double maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
		cmpack_bias_set_thresholds(m_Bias, minvalue, maxvalue);
	}

	return res;
}


//
// Convert bias frame
//
int CBiasCorrProc::ConvertBiasFrame(CConsole *pProgress, const gchar *orig_file, const gchar *temp_file)
{
	int res = 0;
	CmpackKonv *konv = cmpack_konv_init();
	CCCDFile infile;

	// Delete temporary file
	g_unlink(temp_file);

	// Output handling
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_konv_set_console(konv, con);
		cmpack_con_destroy(con);
	}

	// Data format
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::WORK_FORMAT);
	cmpack_konv_set_bitpix(konv, bitpix);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_konv_set_thresholds(konv, minvalue, maxvalue);

	// Color to grayscale conversion
	CmpackChannel channel = (CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT);
	cmpack_konv_set_channel(konv, channel);

	// Open source file
	GError *error = NULL;
	if (infile.Open(orig_file, CMPACK_OPEN_READONLY, &error)) {
		if (infile.isWorkingFormat() && (bitpix==CMPACK_BITPIX_AUTO || infile.Depth()==bitpix)) {
			// We can make a simple copy, no transformation is required
			infile.Close();
			if (!copy_file(orig_file, temp_file, false, &error))
				res = -1;
		} else {
			CmpackCcdFile *outfile;
			char *f = g_locale_from_utf8(temp_file, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
			if (res==0)  {
				res = cmpack_konv(konv, infile.Handle(), outfile);
				cmpack_ccd_destroy(outfile);
			}
			g_free(f);
		}
	} else
		res = -1;

	if (res<0) {
		// Failed
		if (error) {
			m_Progress->Print(error->message);
			g_error_free(error);
		}
	} 
	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// OK
		g_Project->SetTempBiasFile(temp_file);
	}

	cmpack_konv_destroy(konv);
	return res;
}


//
// Bias correction
//
int CBiasCorrProc::Execute(GtkTreePath *pPath)
{
	int state;
	const char *msg;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before the bias correction. Use the function \"Fetch/convert files\" first.";
		m_Progress->Print(msg);
		g_Project->SetError(&iter, msg);
		return -1;
	}
	if ((state & CFILE_BIASCORR)!=0) {
		msg = "The bias correction has been applied to the frame already.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_DARKCORR)!=0) {
		msg = "You cannot do the bias correction after the dark correction.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_FLATCORR)!=0) {
		msg = "You cannot do the bias correction after the flat correction.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_PHOTOMETRY)!=0) {
		msg = "You cannot do the bias correction after the photometry.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}

	// Bias correction
	int res = -1;
	char *tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		CmpackCcdFile *infile;
		res = cmpack_ccd_open(&infile, f, CMPACK_OPEN_READWRITE, 0);
		if (res==0) {
			res = cmpack_bias(m_Bias, infile);
			cmpack_ccd_destroy(infile);
		}
		g_free(f);
		g_free(tpath);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, 0, 0);
		cmpack_free(msg);
	}
	if (res==0) {
		// Success
		m_Progress->Print("Bias correction OK");
		gtk_tree_model_get_iter(pList, &iter, pPath);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_BIASFILE, m_BiasFile, -1);
		g_Project->SetResult(&iter, "Bias correction OK", CFILE_BIASCORR, CFILE_BIASCORR);
	}
	return res;
}

//-----------------------   TIME CORRECTION CONTEXT   ----------------------

//
// Constructor
//
CTimeCorrProc::CTimeCorrProc():m_Progress(NULL), m_Console(NULL), m_Seconds(0),
	m_Reset(false)
{
}

//
// Destructor
//
CTimeCorrProc::~CTimeCorrProc()
{
	if (m_Console)
		cmpack_con_destroy(m_Console);
}


//
// Initialization
//
int CTimeCorrProc::Init(CConsole *pProgress, double seconds, bool reset)
{
	char msg[256];

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	if (m_Console) {
		cmpack_con_destroy(m_Console);
		m_Console = NULL;
	}
	m_Progress = pProgress;
	if (pProgress) {
		m_Console = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(m_Console, CMPACK_LEVEL_DEBUG);
	}

	m_Seconds = seconds;
	m_Reset = reset;

	// Check directory
	force_directory(g_Project->DataDir());

	// Print correction
	sprintf(msg, "Time correction: %.3f seconds", m_Seconds);
	m_Progress->Print(msg);

	return 0;
}


//
// Time correction
//
int CTimeCorrProc::Execute(GtkTreePath *pPath)
{
	double jd = 0, exptime = 0;
	int res = 0, state;
	const char *msg;
	char buf[256];
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before time correction. Use the function \"Fetch/convert files\" first.";
		m_Progress->Print(msg);
		g_Project->SetError(&iter, msg);
		return -1;
	}

	// Open source file 
	if (!m_Reset) {
		gchar *tpath = g_Project->GetImageFile(&iter);
		if (tpath) {
			CmpackCcdFile *ccdfile;
			char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_READWRITE, 0);
			if (res==0) {
				CmpackCcdParams params;
				if (cmpack_ccd_get_params(ccdfile, CMPACK_CM_JD | CMPACK_CM_EXPOSURE, &params)==0 && params.jd>0) {
					params.jd += m_Seconds/86400.0;
					jd = params.jd;
					exptime = params.exposure;
					cmpack_ccd_set_params(ccdfile, CMPACK_CM_JD, &params);
				}
				cmpack_ccd_destroy(ccdfile);
			}
			g_free(f);
			g_free(tpath);
		}
	} else {
		gchar *spath = g_Project->GetSourceFile(&iter);
		if (spath) {
			CmpackCcdFile *srcfile;
			char *sf = g_locale_from_utf8(spath, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&srcfile, sf, CMPACK_OPEN_READONLY, 0);
			if (res==0) {
				gchar *tpath = g_Project->GetImageFile(&iter);
				CmpackCcdFile *ccdfile;
				char *tf = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
				res = cmpack_ccd_open(&ccdfile, tf, CMPACK_OPEN_READWRITE, 0);
				if (res==0) {
					CmpackCcdParams params;
					if (cmpack_ccd_get_params(srcfile, CMPACK_CM_JD, &params)==0 && params.jd>0) {
						params.jd += m_Seconds/86400.0;
						jd = params.jd;
						cmpack_ccd_set_params(srcfile, CMPACK_CM_JD, &params);
					}
					cmpack_ccd_destroy(ccdfile);
				}
				CmpackCcdParams params;
				if (cmpack_ccd_get_params(srcfile, CMPACK_CM_EXPOSURE, &params)==0)
					exptime = params.exposure;
				g_free(tf);
				g_free(tpath);
				cmpack_ccd_destroy(srcfile);
			}
			g_free(sf);
			g_free(spath);
		}
	}

	if (jd>0 && res==0) {
		jd += 0.5*exptime/86400.0;
		if ((state & CFILE_PHOTOMETRY)!=0) {
			gchar *fpath = g_Project->GetPhotFile(&iter);
			if (fpath) {
				CmpackPhtFile *phtfile;
				char *ff = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
				res = cmpack_pht_open(&phtfile, ff, CMPACK_OPEN_READWRITE, 0);
				if (res==0) {
					CmpackPhtInfo params;
					params.jd = jd;
					cmpack_pht_set_info(phtfile, CMPACK_PI_JD, &params);
					cmpack_pht_destroy(phtfile);
				}
				g_free(ff);
				g_free(fpath);
			}
		}
	}

	// Time correction
	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, 0, 0);
		cmpack_free(msg);
	}
	if (res==0) {
		if (jd>0) {
			// Success
			double tc = 0;
			if (!m_Reset) 
				gtk_tree_model_get(pList, &iter, FRAME_TIMECORR, &tc, -1);
			tc += m_Seconds;
			sprintf(buf, "New JD: %.7f", jd);
			m_Progress->Print(buf);
			m_Progress->Print("Time correction OK");
			gtk_tree_model_get_iter(pList, &iter, pPath);
			gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_JULDAT, jd, FRAME_TIMECORR, tc, -1);
			g_Project->SetResult(&iter, "Time correction OK", CFILE_TIMECORR, CFILE_TIMECORR);
		} else {
			const char *msg = "Failed to read the Julian date from the file";
			m_Progress->Print(msg);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, msg, 0, 0);
		}
	}

	return res;
}

//-----------------------   DARK CORRECTION CONTEXT   ----------------------

//
// Constructor
//
CDarkCorrProc::CDarkCorrProc():m_Progress(NULL), m_DarkFile(NULL)
{
	m_Dark = cmpack_dark_init();
}


//
// Destructor
//
CDarkCorrProc::~CDarkCorrProc()
{
	cmpack_dark_destroy(m_Dark);
	g_free(m_DarkFile);
}

//
// Check temporary bias file
//
bool CDarkCorrProc::CheckDarkFile(const gchar *orig_file)
{
	const CFileInfo *orig = g_Project->GetOrigDarkFile(),
		*temp = g_Project->GetTempDarkFile();

	if (!orig->Valid() || !temp->Valid()) 
		return false;
	if (orig->Compare(CFileInfo(orig_file))!=0) 
		return false;
	if (temp->Compare(CFileInfo(temp->FullPath()))!=0) 
		return false;
	return true;
}

//
// Initialization
//
int CDarkCorrProc::Init(CConsole *pProgress, const gchar *fdark)
{
	char *temp, msg[512];

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_dark_set_console(m_Dark, con);
		cmpack_con_destroy(con);
	}

	g_free(m_DarkFile);
	m_DarkFile = g_strdup(fdark);

	if (!fdark || !g_file_test(fdark, G_FILE_TEST_IS_REGULAR))
		return CMPACK_ERR_CANT_OPEN_DARK;

	// Check directory
	force_directory(g_Project->DataDir());

	sprintf(msg, "Dark frame: %s", fdark);
	m_Progress->Print(msg);

	// Convert dark frame
	int res = 0;
	if (!CheckDarkFile(fdark)) {
		temp = g_build_filename(g_Project->DataDir(), "dark." FILE_EXTENSION_FITS, NULL);
		res = ConvertDarkFrame(pProgress, fdark, temp);
		g_free(temp);
	}
	if (res==0) {
		const gchar *tpath = g_Project->GetTempDarkFile()->FullPath();
		CmpackCcdFile *dark;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&dark, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_dark_rdark(m_Dark, dark);
			cmpack_ccd_destroy(dark);
		}
		g_free(f);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// OK
		CmpackBorder border(g_Project->Profile()->GetBorder());
		cmpack_dark_set_border(m_Dark, &border);
		double minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
		double maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
		cmpack_dark_set_thresholds(m_Dark, minvalue, maxvalue);
		bool scaling = g_Project->Profile()->GetBool(CProfile::ADVANCED_CALIBRATION);
		cmpack_dark_set_scaling(m_Dark, scaling);
	}
	return res;
}


//
// Convert dark frame
//
int CDarkCorrProc::ConvertDarkFrame(CConsole *pProgress, const gchar *orig_file, const gchar *temp_file)
{
	int res = 0;
	CmpackKonv *konv = cmpack_konv_init();
	CCCDFile infile;

	// Delete temporary file
	g_unlink(temp_file);

	// Output handling
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_konv_set_console(konv, con);
		cmpack_con_destroy(con);
	}

	// Data format
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::WORK_FORMAT);
	cmpack_konv_set_bitpix(konv, bitpix);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_konv_set_thresholds(konv, minvalue, maxvalue);

	// Color to grayscale conversion
	CmpackChannel channel = (CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT);
	cmpack_konv_set_channel(konv, channel);

	// Open source file
	GError *error = NULL;
	if (infile.Open(orig_file, CMPACK_OPEN_READONLY, &error)) {
		if (infile.isWorkingFormat() && (bitpix==CMPACK_BITPIX_AUTO || infile.Depth()==bitpix)) {
			// We can make a simple copy, no transformation is required
			infile.Close();
			if (!copy_file(orig_file, temp_file, false, &error))
				res = -1;
		} else {
			CmpackCcdFile *outfile;
			char *f = g_locale_from_utf8(temp_file, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
			if (res==0) {
				res = cmpack_konv(konv, infile.Handle(), outfile);
				cmpack_ccd_destroy(outfile);
			}
			g_free(f);
		}
	} else
		res = -1;

	if (res<0) {
		// Failed
		if (error) {
			m_Progress->Print(error->message);
			g_error_free(error);
		}
	} else
	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	} else {
		// OK
		g_Project->SetTempDarkFile(temp_file);
	}

	cmpack_konv_destroy(konv);
	return res;
}


//
// Dark correction
//
int CDarkCorrProc::Execute(GtkTreePath *pPath)
{
	bool advcalib;
	int state;
	const char *msg;
	char *tpath;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	advcalib = g_Project->Profile()->GetBool(CProfile::ADVANCED_CALIBRATION);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before the dark correction. Use the function \"Fetch/convert files\" first.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if (advcalib && (state & CFILE_BIASCORR)==0) {
		msg = "The bias correction must be applied to the frame before the dark correction.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_DARKCORR)!=0) {
		msg = "The dark correction has been applied to the frame already.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_FLATCORR)!=0) {
		msg = "You cannot do the dark correction after flat correction.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_PHOTOMETRY)!=0) {
		msg = "You cannot do the dark correction after the photometry.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}

	// Dark correction
	int res = -1;
	tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		CmpackCcdFile *ccdfile;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_READWRITE, 0);
		if (res==0) {
			res = cmpack_dark(m_Dark, ccdfile);
			cmpack_ccd_destroy(ccdfile);
		}
		g_free(f);
		g_free(tpath);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, 0, 0);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// Success
		m_Progress->Print("Dark-frame correction OK");
		gtk_tree_model_get_iter(pList, &iter, pPath);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_DARKFILE, m_DarkFile, -1);
		g_Project->SetResult(&iter, "Dark-frame correction OK", CFILE_DARKCORR, CFILE_DARKCORR);
	}

	return res;
}

//-----------------------   FLAT CORRECTION CONTEXT   ----------------------

//
// Constructor
//
CFlatCorrProc::CFlatCorrProc():m_Progress(NULL), m_FlatFile(NULL)
{
	m_Flat = cmpack_flat_init();
}

//
// Destructor
//
CFlatCorrProc::~CFlatCorrProc()
{
	cmpack_flat_destroy(m_Flat);
	g_free(m_FlatFile);
}

//
// Check temporary bias file
//
bool CFlatCorrProc::CheckFlatFile(const gchar *orig_file)
{
	const CFileInfo *orig = g_Project->GetOrigFlatFile(),
		*temp = g_Project->GetTempFlatFile();

	if (!orig->Valid() || !temp->Valid()) 
		return false;
	if (orig->Compare(CFileInfo(orig_file))!=0) 
		return false;
	if (temp->Compare(CFileInfo(temp->FullPath()))!=0) 
		return false;
	return true;
}

//
// Initialization
//
int CFlatCorrProc::Init(CConsole *pProgress, const gchar *fflat)
{
	char *temp, msg[512];

	if (!fflat || !g_file_test(fflat, G_FILE_TEST_IS_REGULAR))
		return CMPACK_ERR_CANT_OPEN_FLAT;

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_flat_set_console(m_Flat, con);
		cmpack_con_destroy(con);
	}

	g_free(m_FlatFile);
	m_FlatFile = g_strdup(fflat);

	// Check directory
	force_directory(g_Project->DataDir());

	sprintf(msg, "Flat frame: %s", fflat);
	m_Progress->Print(msg);

	// Flat frame
	int res = 0;
	if (!CheckFlatFile(fflat)) {
		temp = g_build_filename(g_Project->DataDir(), "flat." FILE_EXTENSION_FITS, NULL);
		res = ConvertFlatFrame(pProgress, fflat, temp);
		g_free(temp);
	}
	if (res==0) {
		const gchar *tpath = g_Project->GetTempFlatFile()->FullPath();
		CmpackCcdFile *flat;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&flat, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_flat_rflat(m_Flat, flat);
			cmpack_ccd_destroy(flat);
		}
		g_free(f);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// OK
		CmpackBorder border(g_Project->Profile()->GetBorder());
		cmpack_flat_set_border(m_Flat, &border);
		double minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
		double maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
		cmpack_flat_set_thresholds(m_Flat, minvalue, maxvalue);
	}

	return res;
}

//
// Convert dark frame
//
int CFlatCorrProc::ConvertFlatFrame(CConsole *pProgress, const gchar *orig_file, const gchar *temp_file)
{
	int res = 0;
	CmpackKonv *konv = cmpack_konv_init();
	CCCDFile infile;

	// Delete temporary file
	g_unlink(temp_file);

	// Output handling
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_konv_set_console(konv, con);
		cmpack_con_destroy(con);
	}

	// Data format
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::WORK_FORMAT);
	cmpack_konv_set_bitpix(konv, bitpix);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_konv_set_thresholds(konv, minvalue, maxvalue);

	// Color to grayscale conversion
	CmpackChannel channel = (CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT);
	cmpack_konv_set_channel(konv, channel);

	// Open source file
	GError *error = NULL;
	if (infile.Open(orig_file, CMPACK_OPEN_READONLY, &error)) {
		if (infile.isWorkingFormat() && (bitpix==CMPACK_BITPIX_AUTO || infile.Depth()==bitpix)) {
			// We can make a simple copy, no transformation is required
			infile.Close();
			if (!copy_file(orig_file, temp_file, false, &error))
				res = -1;
		} else {
			CmpackCcdFile *outfile;
			char *f = g_locale_from_utf8(temp_file, -1, NULL, NULL, NULL);
			res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
			if (res==0) {
				res = cmpack_konv(konv, infile.Handle(), outfile);
				cmpack_ccd_destroy(outfile);
			}
			g_free(f);
		}
	} else
		res = -1;

	if (res<0) {
		// Failed
		if (error) {
			m_Progress->Print(error->message);
			g_error_free(error);
		}
	}
	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// OK
		g_Project->SetTempFlatFile(temp_file);
	}

	cmpack_konv_destroy(konv);
	return res;
}


//
// Flat correction
//
int CFlatCorrProc::Execute(GtkTreePath *pPath)
{
	int state;
	const gchar *msg;
	gchar *tpath;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before the flat correction. Use the function \"Fetch/convert files\" first.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_FLATCORR)!=0) {
		msg = "The flat correction has been applied to the frame already.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_PHOTOMETRY)!=0) {
		msg = "You cannot do time correction after the photometry.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}

	// Open source file 
	int res = -1;
	tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		CmpackCcdFile *infile;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&infile, f, CMPACK_OPEN_READWRITE, 0);
		if (res==0) {
			res = cmpack_flat(m_Flat, infile);
			cmpack_ccd_destroy(infile);
		}
		g_free(f);
		g_free(tpath);
	}
	
	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, 0, 0);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// Success
		m_Progress->Print("Flat-frame correction OK");
		gtk_tree_model_get_iter(pList, &iter, pPath);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_FLATFILE, m_FlatFile, -1);
		g_Project->SetResult(&iter, "Flat-frame correction OK", CFILE_FLATCORR, CFILE_FLATCORR);
	}
	return 0;
}

//-----------------------   PHOTOMETRY CONTEXT   ----------------------

//
// Constructor
//
CPhotometryProc::CPhotometryProc():m_Progress(NULL)
{
	m_Phot = cmpack_phot_init();
}


//
// Destructor
//
CPhotometryProc::~CPhotometryProc()
{
	cmpack_phot_destroy(m_Phot);
}


//
// Initialization
//
int CPhotometryProc::Init(CConsole *pProgress)
{
	int i;
	double aper[MAX_APERTURES];

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_phot_set_console(m_Phot, con);
		cmpack_con_destroy(con);
	}

	// Check directory
	force_directory(g_Project->DataDir());

	// Photometry parameters
	cmpack_phot_set_rnoise(m_Phot, g_Project->Profile()->GetDbl(CProfile::READ_NOISE));
	cmpack_phot_set_adcgain(m_Phot, g_Project->Profile()->GetDbl(CProfile::ADC_GAIN));
	cmpack_phot_set_minval(m_Phot, g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE));
	cmpack_phot_set_maxval(m_Phot, g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE));
	cmpack_phot_set_fwhm(m_Phot, g_Project->Profile()->GetDbl(CProfile::DETECTION_FWHM));
	cmpack_phot_set_thresh(m_Phot, g_Project->Profile()->GetDbl(CProfile::DETECTION_THRESHOLD));
	cmpack_phot_set_minrnd(m_Phot, g_Project->Profile()->GetDbl(CProfile::MIN_ROUNDNESS));
	cmpack_phot_set_maxrnd(m_Phot, g_Project->Profile()->GetDbl(CProfile::MAX_ROUNDNESS));
	cmpack_phot_set_minshrp(m_Phot, g_Project->Profile()->GetDbl(CProfile::MIN_SHARPNESS));
	cmpack_phot_set_maxshrp(m_Phot, g_Project->Profile()->GetDbl(CProfile::MAX_SHARPNESS));
	cmpack_phot_set_skyin(m_Phot, g_Project->Profile()->GetDbl(CProfile::SKY_INNER_RADIUS));
	cmpack_phot_set_skyout(m_Phot, g_Project->Profile()->GetDbl(CProfile::SKY_OUTER_RADIUS));

	// Frame Border
	CmpackBorder border(g_Project->Profile()->GetBorder());
	cmpack_phot_set_border(m_Phot, &border);

	// Apertures
	CApertures ap = g_Project->Profile()->Apertures();
	for (i=0; i<ap.Count() && i<MAX_APERTURES; i++) 
		aper[i] = ap.Get(i)->Radius();
	cmpack_phot_set_aper(m_Phot, aper, ap.Count());
	g_Project->SetApertures(ap);

	return 0;
}


//
// Photometry
//
int CPhotometryProc::Execute(GtkTreePath *pPath)
{
	int state, fileid, nstars;
	const char *msg;
	char pht[128], txt[128];
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before the photometry. Use the function \"Fetch/convert files\" first.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}

	// Open source file 
	int res = -1;
	gchar *srcpath = g_Project->GetImageFile(&iter);
	if (srcpath) {
		CmpackCcdFile *ccdfile;
		gchar *sf = g_locale_from_utf8(srcpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&ccdfile, sf, CMPACK_OPEN_READONLY, 0);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			m_Progress->Print(msg);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, msg);
			cmpack_free(msg);
			g_free(sf);
			g_free(srcpath);
			return res;
		}
		g_free(sf);
		g_free(srcpath);

		// Open target file
		fileid = g_Project->GetFrameID(&iter);
		sprintf(pht, "tmp%05d.%s", fileid, FILE_EXTENSION_PHOTOMETRY);
		char *outpath = g_build_filename(g_Project->DataDir(), pht, NULL);
		g_unlink(outpath);
		CmpackPhtFile *outfile;
		gchar *of = g_locale_from_utf8(outpath, -1, NULL, NULL, NULL);
		res = cmpack_pht_open(&outfile, of, CMPACK_OPEN_CREATE, 0);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			m_Progress->Print(msg);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, msg);
			cmpack_free(msg);
			g_free(of);
			g_free(outpath);
			cmpack_ccd_destroy(ccdfile);
			return res;
		}
		g_free(of);
		g_free(outpath);

		// Execute photometry
		res = cmpack_phot(m_Phot, ccdfile, outfile, &nstars);
		if (res==0) {
			// Success
			sprintf(txt, "Photometry OK (%d stars found)", nstars);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetResult(&iter, txt, CFILE_PHOTOMETRY | CFILE_MATCHING, CFILE_PHOTOMETRY);
			gtk_list_store_set(GTK_LIST_STORE(pList), &iter, 
				FRAME_PHOTFILE, pht, FRAME_STARS, nstars, FRAME_MSTARS, 0, 
				FRAME_OFFSET_X, 0.0, FRAME_OFFSET_Y, 0.0, -1);
			sprintf(txt, "%d stars found", nstars);
			m_Progress->Print(txt);
		} else {
			// Failed
			char *msg = cmpack_formaterror(res);
			m_Progress->Print(msg);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, msg, CFILE_PHOTOMETRY | CFILE_MATCHING, 0);
			gtk_list_store_set(GTK_LIST_STORE(pList), &iter,
				FRAME_PHOTFILE, NULL, FRAME_STARS, 0, FRAME_MSTARS, 0, 
				FRAME_OFFSET_X, 0.0, FRAME_OFFSET_Y, 0.0, -1);
			cmpack_free(msg);
		}

		cmpack_pht_close(outfile);
		cmpack_ccd_destroy(ccdfile);
	}
	return res;
}

//-----------------------   MATCHING CONTEXT   ----------------------

//
// Constructor
//
CMatchingProc::CMatchingProc():m_Progress(NULL)
{
	m_Match = cmpack_match_init();
}


//
// Destructor
//
CMatchingProc::~CMatchingProc()
{
	cmpack_match_destroy(m_Match);
}


//
// Initialization
//
int CMatchingProc::InitWithReferenceFrame(CConsole *pProgress, GtkTreePath *path)
{
	int		res;

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_match_set_console(m_Match, con);
		cmpack_con_destroy(con);
	}

	// Check directory
	force_directory(g_Project->DataDir());

	// Set configuration
	cmpack_match_set_maxstars(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_READ_STARS));
	cmpack_match_set_vertices(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_IDENT_STARS));
	cmpack_match_set_threshold(m_Match, g_Project->Profile()->GetDbl(CProfile::MATCH_CLIP));
	cmpack_match_set_method(m_Match, (g_Project->Profile()->GetBool(CProfile::SPARSE_FIELDS) ? CMPACK_MATCH_SPARSE_FIELDS : CMPACK_MATCH_STANDARD));
	cmpack_match_set_maxoffset(m_Match, g_Project->Profile()->GetDbl(CProfile::MAX_OFFSET));

	char txt[512];
	sprintf(txt, "Reference frame: #%d", g_Project->GetFrameID(path));
	m_Progress->Print(txt);

	// Read reference frame
	gchar *tpath = g_Project->GetPhotFile(path);
	CmpackPhtFile *phtfile;
	char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
	res = cmpack_pht_open(&phtfile, f, CMPACK_OPEN_READONLY, 0);
	if (res==0) {
		res = cmpack_match_readref_pht(m_Match, phtfile);
		cmpack_pht_destroy(phtfile);
	}
	g_free(f);
	g_free(tpath);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
		return res;
	}

	// Set reference frame
	g_Project->SetReference(path, NULL);

	// Object coordinates, location and observer's name
	CCCDFile ccd;
	gchar *fpath = g_Project->GetImageFile(path);
	if (fpath) {
		GError *error = NULL;
		if (ccd.Open(fpath, CMPACK_OPEN_READONLY, &error)) {
			if (ccd.Object()->Valid()) {
				g_Project->SetObjectCoords(*ccd.Object());
				CConfig::SetLastObject(*g_Project->ObjectCoords());
			} else
				g_Project->SetObjectCoords(CObjectCoords());
			if (ccd.Location()->Valid()) {
				g_Project->SetLocation(*ccd.Location());
				CConfig::SetLastLocation(*g_Project->Location());
			} else if (g_Project->Profile()->DefaultLocation().Valid()) {
				g_Project->SetLocation(g_Project->Profile()->DefaultLocation());
				CConfig::SetLastLocation(*g_Project->Location());
			} else
				g_Project->SetLocation(CLocation());
			g_Project->SetObserver(ccd.Observer());
			g_Project->SetTelescope(ccd.Telescope());
			g_Project->SetInstrument(ccd.Instrument());
		} else {
			if (error) {
				m_Progress->Print(error->message);
				g_error_free(error);
			}
			res = -1;
		}
	}
	g_free(fpath);

	return res;
}

//
// Initialization
//
int CMatchingProc::InitWithCatalogFile(CConsole *pProgress, const char *cat_file)
{
	int res = 0;

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_match_set_console(m_Match, con);
		cmpack_con_destroy(con);
	}

	// Check directory
	force_directory(g_Project->DataDir());

	// Set configuration
	cmpack_match_set_maxstars(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_READ_STARS));
	cmpack_match_set_vertices(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_IDENT_STARS));
	cmpack_match_set_threshold(m_Match, g_Project->Profile()->GetDbl(CProfile::MATCH_CLIP));
	cmpack_match_set_method(m_Match, (g_Project->Profile()->GetBool(CProfile::SPARSE_FIELDS) ? CMPACK_MATCH_SPARSE_FIELDS : CMPACK_MATCH_STANDARD));
	cmpack_match_set_maxoffset(m_Match, g_Project->Profile()->GetDbl(CProfile::MAX_OFFSET));

	char txt[512];
	sprintf(txt, "Catalog file: %s", cat_file);
	m_Progress->Print(txt);

	if (!g_file_test(cat_file, G_FILE_TEST_IS_REGULAR))
		return CMPACK_ERR_CANT_OPEN_REF;

	// Make copy of catalog file
	char *tmp_file = g_build_filename(g_Project->DataDir(), "reference." FILE_EXTENSION_CATALOG, NULL);
	if (!CheckCatalogFile(cat_file)) 
		ConvertCatFile(cat_file, tmp_file);

	// Read catalog file
	CmpackCatFile *catfile;
	char *f = g_locale_from_utf8(tmp_file, -1, NULL, NULL, NULL);
	res = cmpack_cat_open(&catfile, f, CMPACK_OPEN_READONLY, 0);
	if (res==0) {
		res = cmpack_match_readref_cat(m_Match, catfile);
		if (res==0) {
			// Set reference frame
			g_Project->Lock();
			g_Project->SetReference(NULL, cat_file);
			// Selected stars, tags, object coordinates, location, observer's name
			CCatalog cat(catfile);
			const CSelectionList *list = cat.Selections();
			gchar *base_name = g_path_get_basename(cat_file);
			gchar *file_name = StripFileExtension(base_name);
			if (list && list->Count()>0) {
				g_Project->SetLastSelection(list->At(0));
				g_Project->SelectionList()->Clear();
				int index = 1;
				for (int i=0; i<list->Count(); i++) {
					gchar *news_name = NULL;
					if (list->Name(i) && g_Project->SelectionList()->IndexOf(list->Name(i))<0) 
						news_name = g_strdup(list->Name(i));
					else if (file_name && g_Project->SelectionList()->IndexOf(file_name)<0) 
						news_name = g_strdup(file_name);
					while (!news_name) {
						gchar *name = (gchar*)g_malloc(256*sizeof(gchar));
						sprintf(name, "Selection %d", index);
						if (g_Project->SelectionList()->IndexOf(name)<0) 
							news_name = name;
						else
							g_free(name);
						index++;
					}
					g_Project->SelectionList()->Set(news_name, list->At(i));
					g_free(news_name);
				}
			} else {
				g_Project->SetLastSelection(CSelection());
				g_Project->SelectionList()->Clear();
			}
			*g_Project->Tags() = *cat.Tags();
			if (cat.Object()->Valid()) {
				g_Project->SetObjectCoords(*cat.Object());
				CConfig::SetLastObject(*g_Project->ObjectCoords());
			} else
				g_Project->SetObjectCoords(CObjectCoords());
			if (cat.Location()->Valid()) {
				g_Project->SetLocation(*cat.Location());
				CConfig::SetLastLocation(*g_Project->Location());
			} else if (g_Project->Profile()->DefaultLocation().Valid()) {
				g_Project->SetLocation(g_Project->Profile()->DefaultLocation());
				CConfig::SetLastLocation(*g_Project->Location());
			} else
				g_Project->SetLocation(CConfig::LastLocation());
			g_Project->SetObserver(cat.Observer());
			g_Project->SetTelescope(cat.Telescope());
			g_Project->SetInstrument(cat.Instrument());
			g_Project->Unlock();
			g_free(base_name);
			g_free(file_name);
		}
		cmpack_cat_destroy(catfile);
	}
	g_free(f);
	g_free(tmp_file);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	return res;
}

//
// Initialization
//
int CMatchingProc::Reinitialize(CConsole *pProgress)
{
	int res = CMPACK_ERR_NO_REF_FILE, frame_id;
	char txt[512];
	const gchar *catpath;
	GtkTreePath *refpath;

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_match_set_console(m_Match, con);
		cmpack_con_destroy(con);
	}

	force_directory(g_Project->DataDir());

	// Set configuration
	cmpack_match_set_maxstars(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_READ_STARS));
	cmpack_match_set_vertices(m_Match, g_Project->Profile()->GetInt(CProfile::MATCH_IDENT_STARS));
	cmpack_match_set_threshold(m_Match, g_Project->Profile()->GetDbl(CProfile::MATCH_CLIP));
	cmpack_match_set_method(m_Match, (g_Project->Profile()->GetBool(CProfile::SPARSE_FIELDS) ? CMPACK_MATCH_SPARSE_FIELDS : CMPACK_MATCH_STANDARD));
	cmpack_match_set_maxoffset(m_Match, g_Project->Profile()->GetDbl(CProfile::MAX_OFFSET));

	switch (g_Project->GetReferenceType())
	{
	case REF_FRAME:
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			frame_id = g_Project->GetFrameID(refpath);
			sprintf(txt, "Reference frame: #%d", frame_id);
			m_Progress->Print(txt);
			// Read reference frame
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				CmpackPhtFile *phtfile;
				char *f = g_locale_from_utf8(pht_file, -1, NULL, NULL, NULL);
				res = cmpack_pht_open(&phtfile, f, CMPACK_OPEN_READONLY, 0);
				if (res==0) {
					res = cmpack_match_readref_pht(m_Match, phtfile);
					cmpack_pht_destroy(phtfile);
				}
				g_free(f);
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		catpath = g_Project->GetReferenceCatalog()->FullPath();
		if (catpath) {
			gchar *basename = g_path_get_basename(catpath);
			sprintf(txt, "Catalog file: %s", basename);
			m_Progress->Print(txt);
			g_free(basename);
			// Read catalogue file
			catpath = g_Project->GetTempCatFile()->FullPath();
			CmpackCatFile *catfile;
			char *f = g_locale_from_utf8(catpath, -1, NULL, NULL, NULL);
			res = cmpack_cat_open(&catfile, f, CMPACK_OPEN_READONLY, 0);
			if (res==0) {
				res = cmpack_match_readref_cat(m_Match, catfile);
				cmpack_cat_destroy(catfile);
			}
			g_free(f);
		}
		break;

	default:
		break;
	}

	if (res>0) {
		// Failed with Cmpack error
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	return res;
}

//
// Check temporary catalogue file
//
bool CMatchingProc::CheckCatalogFile(const gchar *orig_file)
{
	const CFileInfo *orig = g_Project->GetReferenceCatalog(),
		*temp = g_Project->GetTempCatFile();

	if (!orig->Valid() || !temp->Valid()) 
		return false;
	if (orig->Compare(CFileInfo(orig_file))!=0) 
		return false;
	if (temp->Compare(CFileInfo(temp->FullPath()))!=0) 
		return false;
	return true;
}

void CMatchingProc::ConvertCatFile(const gchar *orig_file, const gchar *temp_file)
{
	char *fts_path, *src_path, *dst_path;

	// Delete existing files
	g_unlink(temp_file);
	fts_path = SetFileExtension(temp_file, FILE_EXTENSION_FITS);
	g_unlink(fts_path);
	g_free(fts_path);

	// Copy catalog photometry file
	if (copy_file(orig_file, temp_file, false, NULL)) {
		// Copy catalog image file
		src_path = SetFileExtension(orig_file, FILE_EXTENSION_FITS);
		dst_path = SetFileExtension(temp_file, FILE_EXTENSION_FITS);
		copy_file(src_path, dst_path, false, NULL);
		g_free(src_path);
		g_free(dst_path);
		g_Project->SetTempCatFile(CFileInfo(temp_file));
	}
}

int CMatchingProc::Execute(GtkTreePath *pPath)
{
	int res, nstars, mstars, state;
	const char *msg;
	double offset_x, offset_y;
	char *fpath, txt[128];
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	mstars = nstars = 0;
	offset_x = offset_y = 0;
	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) {
		msg = "A working copy of the source files must be made before the photometry. Use the function \"Fetch/convert files\" first.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}
	if ((state & CFILE_PHOTOMETRY)==0) {
		msg = "The photometry must be applied to the frame before the matching.";
		g_Project->SetError(&iter, msg);
		m_Progress->Print(msg);
		return -1;
	}

	// Make photometry file
	fpath = g_Project->GetPhotFile(pPath);
	CmpackPhtFile *phtfile;
	char *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	res = cmpack_pht_open(&phtfile, f, CMPACK_OPEN_READWRITE, 0);
	if (res==0) {
		res = cmpack_match(m_Match, phtfile, &mstars);
		cmpack_match_get_offset(m_Match, &offset_x, &offset_y);
		cmpack_pht_destroy(phtfile);
	}
	g_free(f);
	g_free(fpath);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg, CFILE_MATCHING, 0);
		cmpack_free(msg);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_MSTARS, 0, 
			FRAME_OFFSET_X, 0.0, FRAME_OFFSET_Y, 0.0, -1);
	}
	if (res==0) {
		// Success
		gtk_tree_model_get_iter(pList, &iter, pPath);
		gtk_tree_model_get(pList, &iter, FRAME_STARS, &nstars, -1);
		gtk_list_store_set(GTK_LIST_STORE(pList), &iter, FRAME_MSTARS, mstars, 
			FRAME_OFFSET_X, offset_x, FRAME_OFFSET_Y, offset_y, -1);
		sprintf(txt, "Matching OK (%.0f %% stars matched)", 100.0*((double)mstars)/nstars);
		g_Project->SetResult(&iter, txt, CFILE_MATCHING, CFILE_MATCHING);
		sprintf(txt, "%d stars matched (%.0f %%).", mstars, 100.0*((double)mstars)/nstars);
		m_Progress->Print(txt);
	}
	return res;
}

//-----------------------   MASTER BIAS CONTEXT   ----------------------

//
// Constructor
//
CMasterBiasProc::CMasterBiasProc():m_Progress(NULL), m_File(NULL)
{
	m_MBias = cmpack_mbias_init();
}


//
// Destructor
//
CMasterBiasProc::~CMasterBiasProc()
{
	cmpack_mbias_destroy(m_MBias);
	if (m_File)
		cmpack_ccd_destroy(m_File);
}


//
// Initialization
//
int CMasterBiasProc::Open(CConsole *pProgress, const gchar *fpath)
{
	int res;
	char txt[512];

	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}

	if (!fpath || *fpath=='\0')
		return CMPACK_ERR_CANT_OPEN_OUT;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_mbias_set_console(m_MBias, con);
		cmpack_con_destroy(con);
	}

	sprintf(txt, "Output file: %s", fpath);
	m_Progress->Print(txt);

	// Image data type
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::MBIAS_FORMAT);
	cmpack_mbias_set_bitpix(m_MBias, bitpix);

	// Frame border
	CmpackBorder border(g_Project->Profile()->GetBorder());
	cmpack_mbias_set_border(m_MBias, &border);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_mbias_set_thresholds(m_MBias, minvalue, maxvalue);

	// Output file
	CmpackCcdFile *outfile;
	gchar *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
	if (res==0)
		res = cmpack_mbias_open(m_MBias, outfile);
	g_free(f);

	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(outfile);
	} else {
		// OK
		m_File = outfile;
	}
	return res;
}


//
// Flat correction
//
int CMasterBiasProc::Add(GtkTreePath *pPath)
{
	int res = -1, state;
	char *tpath;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) 
		return -1;

	// Open source file 
	tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		CmpackCcdFile *ccdfile;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_mbias_read(m_MBias, ccdfile);
			cmpack_ccd_destroy(ccdfile);
		}
		g_free(f);
		g_free(tpath);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg);
		cmpack_free(msg);
	}
	return res;
}

//
// Finalization
//
int CMasterBiasProc::Close(void)
{
	int res = cmpack_mbias_close(m_MBias);
	if (res==0 && m_File) {
		res = cmpack_ccd_close(m_File);
		m_File = NULL;
	}
	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}
	return res;
}

//-----------------------   MASTER DARK CONTEXT   ----------------------

//
// Constructor
//
CMasterDarkProc::CMasterDarkProc():m_Progress(NULL), m_File(NULL)
{
	m_MDark = cmpack_mdark_init();
}


//
// Destructor
//
CMasterDarkProc::~CMasterDarkProc()
{
	cmpack_mdark_destroy(m_MDark);
	if (m_File)
		cmpack_ccd_destroy(m_File);
}


//
// Initialization
//
int CMasterDarkProc::Open(CConsole *pProgress, const gchar *fpath)
{
	int res;
	char txt[512];

	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_mdark_set_console(m_MDark, con);
		cmpack_con_destroy(con);
	}

	if (!fpath || *fpath=='\0')
		return CMPACK_ERR_CANT_OPEN_OUT;

	sprintf(txt, "Output file: %s", fpath);
	m_Progress->Print(txt);

	// Scalable darks
	bool adv_calib = g_Project->Profile()->GetBool(CProfile::ADVANCED_CALIBRATION);
	cmpack_mdark_set_scalable(m_MDark, adv_calib);

	// Image data type
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::MDARK_FORMAT);
	cmpack_mdark_set_bitpix(m_MDark, bitpix);

	// Frame border
	CmpackBorder border(g_Project->Profile()->GetBorder());
	cmpack_mdark_set_border(m_MDark, &border);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_mdark_set_thresholds(m_MDark, minvalue, maxvalue);

	// Output file
	CmpackCcdFile *outfile;
	gchar *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
	if (res==0)
		res = cmpack_mdark_open(m_MDark, outfile);
	g_free(f);

	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(outfile);
	} else 
		m_File = outfile;
	return res;
}


//
// Flat correction
//
int CMasterDarkProc::Add(GtkTreePath *pPath)
{
	int res = -1, state;
	char *tpath;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) 
		return -1;

	// Open source file 
	tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		CmpackCcdFile *ccdfile;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_mdark_read(m_MDark, ccdfile);
			cmpack_ccd_destroy(ccdfile);
		}
		g_free(f);
		g_free(tpath);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg);
		cmpack_free(msg);
	}
	return res;
}

//
// Finalization
//
int CMasterDarkProc::Close(void)
{
	int res = cmpack_mdark_close(m_MDark);
	if (res==0 && m_File) {
		res = cmpack_ccd_close(m_File);
		m_File = NULL;
	}
	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}
	return res;
}

//-----------------------   MASTER FLAT CONTEXT   ----------------------

//
// Constructor
//
CMasterFlatProc::CMasterFlatProc():m_Progress(NULL), m_File(NULL)
{
	m_MFlat = cmpack_mflat_init();
}


//
// Destructor
//
CMasterFlatProc::~CMasterFlatProc()
{
	cmpack_mflat_destroy(m_MFlat);
	if (m_File)
		cmpack_ccd_destroy(m_File);
}


//
// Initialization
//
int CMasterFlatProc::Open(CConsole *pProgress, const gchar *outfile)
{
	int res;
	char txt[512];

	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}

	if (!outfile || *outfile=='\0')
		return CMPACK_ERR_CANT_OPEN_OUT;

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_mflat_set_console(m_MFlat, con);
		cmpack_con_destroy(con);
	}

	sprintf(txt, "Output file: %s", outfile);
	m_Progress->Print(txt);

	// Mean output level
	double level = g_Project->Profile()->GetDbl(CProfile::MFLAT_LEVEL);
	cmpack_mflat_set_level(m_MFlat, level);

	// Image data type
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::MFLAT_FORMAT);
	cmpack_mflat_set_bitpix(m_MFlat, bitpix);

	// Frame border
	CmpackBorder border(g_Project->Profile()->GetBorder());
	cmpack_mflat_set_border(m_MFlat, &border);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_mflat_set_thresholds(m_MFlat, minvalue, maxvalue);

	// Output file
	CmpackCcdFile *ccdfile;
	char *f = g_locale_from_utf8(outfile, -1, NULL, NULL, NULL);
	res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_CREATE, 0);
	if (res==0)
		res = cmpack_mflat_open(m_MFlat, ccdfile);
	g_free(f);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(ccdfile);
	} else
		m_File = ccdfile;

	return res;
}


//
// Flat correction
//
int CMasterFlatProc::Add(GtkTreePath *pPath)
{
	int res = -1, state;
	char *tpath;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_CONVERSION)==0) 
		return -1;

	// Open source file 
	tpath = g_Project->GetImageFile(&iter);
	if (tpath) {
		CmpackCcdFile *ccdfile;
		char *f = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
		res = cmpack_ccd_open(&ccdfile, f, CMPACK_OPEN_READONLY, 0);
		if (res==0) {
			res = cmpack_mflat_read(m_MFlat, ccdfile);
			cmpack_ccd_destroy(ccdfile);
		}
		g_free(f);
		g_free(tpath);
	}

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg);
		cmpack_free(msg);
	}
	return res;
}

//
// Finalization
//
int CMasterFlatProc::Close(void)
{
	int res = cmpack_mflat_close(m_MFlat);
	if (res==0 && m_File) {
		res = cmpack_ccd_close(m_File);
		m_File = NULL;
	}
	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}
	return res;
}

//-----------------------   KOMBINE CONTEXT   ----------------------

//
// Constructor
//
CCombineProc::CCombineProc():m_Progress(NULL), m_File(NULL), m_OutIndex(0)
{
	m_Kombine = cmpack_kombine_init();
}


//
// Destructor
//
CCombineProc::~CCombineProc()
{
	cmpack_kombine_destroy(m_Kombine);
	if (m_File)
		cmpack_ccd_destroy(m_File);
}


//
// Initialization
//
int CCombineProc::Open(CConsole *pProgress, const gchar *fpath, int outindex)
{
	int res;
	char txt[512];

	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}

	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_kombine_set_console(m_Kombine, con);
		cmpack_con_destroy(con);
	}

	m_OutIndex = outindex;

	if (!fpath || *fpath=='\0')
		return CMPACK_ERR_CANT_OPEN_OUT;

	sprintf(txt, "Output file: %s", fpath);
	m_Progress->Print(txt);

	// Image data type
	CmpackBitpix bitpix = (CmpackBitpix)g_Project->Profile()->GetInt(CProfile::KOMBINE_FORMAT);
	cmpack_kombine_set_bitpix(m_Kombine, bitpix);

	// Frame border
	CmpackBorder border(g_Project->Profile()->GetBorder());
	cmpack_kombine_set_border(m_Kombine, &border);

	// Threshold values
	double minvalue, maxvalue;
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_kombine_set_thresholds(m_Kombine, minvalue, maxvalue);

	// Output file
	gchar *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
	CmpackCcdFile *outfile;
	res = cmpack_ccd_open(&outfile, f, CMPACK_OPEN_CREATE, 0);
	if (res==0)
		res = cmpack_kombine_open(m_Kombine, outfile);
	g_free(f);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
		cmpack_ccd_destroy(outfile);
	} else
		m_File = outfile;

	return res;
}


//
// Flat correction
//
int CCombineProc::Add(GtkTreePath *pPath)
{
	int			res, state;
	char		*tpath, *ppath, txt[128];
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter	iter;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	// Check frame status
	state = g_Project->GetState(&iter);
	if ((state & CFILE_MATCHING)==0) 
		return -1;

	// Open source image file 
	tpath = g_Project->GetImageFile(&iter);
	CmpackCcdFile *ccdfile;
	char *tf = g_locale_from_utf8(tpath, -1, NULL, NULL, NULL);
	res = cmpack_ccd_open(&ccdfile, tf, CMPACK_OPEN_READONLY, 0);
	if (res==0) {
		// Open source photometry file
		ppath = g_Project->GetPhotFile(&iter);
		if (ppath) {
			CmpackPhtFile *phtfile;
			char *pf = g_locale_from_utf8(ppath, -1, NULL, NULL, NULL);
			res = cmpack_pht_open(&phtfile, pf, CMPACK_OPEN_READONLY, 0);
			if (res==0) {
				// Add frame to combined frame
				res = cmpack_kombine_read(m_Kombine, ccdfile, phtfile);
				cmpack_pht_destroy(phtfile);
			}
			g_free(pf);
			g_free(ppath);
		}
		cmpack_ccd_destroy(ccdfile);
	}
	g_free(tf);
	g_free(tpath);

	if (res>0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetError(&iter, msg);
		cmpack_free(msg);
	}
	if (res==0) {
		// Success
		if (m_OutIndex>0)
			sprintf(txt, "Merging OK (output frame #%d)", m_OutIndex);
		else
			sprintf(txt, "Merging OK");
		gtk_tree_model_get_iter(pList, &iter, pPath);
		g_Project->SetResult(&iter, txt);
		m_Progress->Print(txt);
	}
	return res;
}

//
// Finalization
//
int CCombineProc::Close(void)
{
	int res = cmpack_kombine_close(m_Kombine);
	if (res==0 && m_File) {
		res = cmpack_ccd_close(m_File);
		m_File = NULL;
	}
	if (res!=0) {
		// Failed
		char *msg = cmpack_formaterror(res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	if (m_File) {
		cmpack_ccd_destroy(m_File);
		m_File = NULL;
	}
	m_OutIndex = 0;
	return res;
}

//-----------------------   MAKE TABLE OF APERTURES   ----------------------

//
// Constructor
//
CListProc::CListProc():m_Progress(NULL), m_FrameSet(NULL), m_Console(NULL),
	m_RawFrames(false), m_FrameID(0)
{
}


//
// Destructor
//
CListProc::~CListProc()
{
	if (m_Console)
		cmpack_con_destroy(m_Console);
}


//
// Initialization
//
int CListProc::Init(CConsole *pProgress, CFrameSet *pFrames, bool raw_frames)
{
	m_FrameSet = pFrames;
	m_RawFrames = raw_frames;
	m_FrameID = 0;

	// Output handling
	if (m_Console) {
		cmpack_con_destroy(m_Console);
		m_Console = NULL;
	}
	m_Progress = pProgress;
	if (pProgress) {
		m_Console = cmpack_con_init_cb(OutputProc, this);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(m_Console, CMPACK_LEVEL_DEBUG);
	}

	return CMPACK_ERR_OK;
}


//
// Print to console
//
void CListProc::OutputProc(const char *text, void *user_data)
{
	((CListProc*)user_data)->Print(text, false);
}

void CListProc::Print(const char *text, bool utf8)
{
	gsize len;

	if (m_Progress) {
		if (!utf8) {
			char *buffer = g_locale_to_utf8(text, -1, NULL, &len, NULL);
			if (m_FrameID>0) {
				char *msg = (char*)g_malloc(len+16);
				sprintf(msg, "%d> %s", m_FrameID, buffer);
				m_Progress->Print(msg);
				g_free(msg);
			} else {
				m_Progress->Print(buffer);
			}
			g_free(buffer);
		} else {
			len = strlen(text);
			if (m_FrameID>0) {
				char *msg = (char*)g_malloc(len+16);
				sprintf(msg, "%d> %s", m_FrameID, text);
				m_Progress->Print(msg);
				g_free(msg);
			} else {
				m_Progress->Print(text);
			}
		}
	}
}


//
// Flat correction
//
int CListProc::Add(GtkTreePath *pPath)
{
	int res = 0;
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter;
	GError *error = NULL;

	gtk_tree_model_get_iter(pList, &iter, pPath);

	m_FrameID = g_Project->GetFrameID(&iter);
	if (!m_RawFrames) {
		// Open source file 
		char *tpath = g_Project->GetPhotFile(&iter);
		if (tpath) {
			CPhot infile;
			if (!infile.Load(tpath, &error) || !m_FrameSet->AppendFrame(infile, m_FrameID, &error))
				res = -1;
			g_free(tpath);
		}
	} else {
		double juldat = g_Project->GetJulDate(&iter);
		if (juldat>0)
			if (!m_FrameSet->AppendFrame(juldat, m_FrameID, &error))
				res = -1;
	}

	if (res<0) {
		if (error) {
			Print(error->message, false);
			gtk_tree_model_get_iter(pList, &iter, pPath);
			g_Project->SetError(&iter, error->message);
			g_error_free(error);
		}
	}
	return res;
}

//-----------------------   MUNIFIND CLASS INTERFACE   ----------------------

//
// Constructor
//
CMFindProc::CMFindProc():m_Progress(NULL), m_MFind(NULL), m_CompStarId(-1)
{
	m_MFind = cmpack_mfind_init();
}

//
// Destructor
//
CMFindProc::~CMFindProc()
{
	if (m_MFind)
		cmpack_mfind_destroy(m_MFind);
}

//
// Initialization
//
int CMFindProc::Init(CConsole *pProgress, int aperture_id, int comp_star)
{
	// Output handling
	m_Progress = pProgress;
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_mfind_set_console(m_MFind, con);
		cmpack_con_destroy(con);
	}

	// Set parameters
	cmpack_mfind_set_aperture(m_MFind, aperture_id);
	cmpack_mfind_set_threshold(m_MFind, g_Project->Profile()->GetDbl(CProfile::VARFIND_THRESHOLD));
	m_CompStarId = comp_star;
	return 0;
}

//
// Make mag-dev curve
//
int CMFindProc::Execute(const CFrameSet &fset, CTable **table, int *comp_star,
	double *jdmin, double *jdmax, double *magrange)
{
	*table = NULL;
	if (jdmin)
		*jdmin = 0;
	if (jdmax)
		*jdmax = 0;
	if (magrange)
		*magrange = 0;

	int comp = m_CompStarId;
	if (comp<0) {
		int res = cmpack_mfind_autocomp(m_MFind, fset.Handle(), &comp, CMPACK_MFIND_DEFAULT);
		if (res!=0)
			return res;
	}
	if (comp_star)
		*comp_star = comp;

	cmpack_mfind_set_comparison(m_MFind, comp);
	CmpackTable *tab;
	int res = cmpack_mfind(m_MFind, fset.Handle(), &tab, CMPACK_MFIND_DEFAULT);
	if (res==0) {
		*table = new CTable(tab);
		cmpack_tab_destroy(tab);  // Release my reference
		if (jdmin && jdmax)
			cmpack_mfind_jdrange(m_MFind, fset.Handle(), jdmin, jdmax, CMPACK_MFIND_DEFAULT);
		if (magrange)
			cmpack_mfind_magrange(m_MFind, fset.Handle(), magrange, CMPACK_MFIND_DEFAULT);
	}
	return res;
}

//-----------------------   LIGHT CURVE CLASS INTERFACE   ----------------------

//
// Make light curve
//
int CmpackLightCurve(CConsole *pProgress, CTable **table, const CSelection &sel, 
	const CFrameSet &fset, int aperture, CmpackLCurveFlags flags)
{
	int res, count, stars[MAX_SELECTION];

	*table = NULL;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;
	if (sel.Count()==0)
		return CMPACK_ERR_INVALID_CONTEXT;
	
	CmpackLCurve *lc = cmpack_lcurve_init();

	// Set aperture
	cmpack_lcurve_set_aperture(lc, aperture);

	// Set selected stars
	count = sel.GetStarList(CMPACK_SELECT_VAR, stars, MAX_SELECTION);
	cmpack_lcurve_set_var(lc, stars, count);
	count = sel.GetStarList(CMPACK_SELECT_COMP, stars, MAX_SELECTION);
	cmpack_lcurve_set_comp(lc, stars, count);
	count = sel.GetStarList(CMPACK_SELECT_CHECK, stars, MAX_SELECTION);
	cmpack_lcurve_set_check(lc, stars, count);

	// Output handling
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_lcurve_set_console(lc, con);
		cmpack_con_destroy(con);
	}

	CmpackTable *tab = NULL;
	res = cmpack_lcurve(lc, fset.Handle(), &tab, flags);
	if (res==0) {
		*table = new CTable(tab);
		cmpack_tab_destroy(tab);  // Release my reference
	}
	cmpack_lcurve_destroy(lc);
	return res;
}

//-----------------------   LIGHT CURVE CLASS INTERFACE   ----------------------

//
// Make light curve
//
int CmpackApDevCurve(CConsole *pProgress, CTable **table, const CSelection &sel, 
	const CFrameSet &fset)
{
	int res, count, stars[MAX_SELECTION];

	*table = NULL;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;
	if (sel.Count()==0)
		return CMPACK_ERR_INVALID_CONTEXT;
	
	CmpackADCurve *lc = cmpack_adcurve_init();

	// Set selected stars
	count = sel.GetStarList(CMPACK_SELECT_COMP, stars, MAX_SELECTION);
	cmpack_adcurve_set_comp(lc, stars, count);
	count = sel.GetStarList(CMPACK_SELECT_CHECK, stars, MAX_SELECTION);
	cmpack_adcurve_set_check(lc, stars, count);

	// Output handling
	if (pProgress) {
		CmpackConsole *con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
		cmpack_adcurve_set_console(lc, con);
		cmpack_con_destroy(con);
	}

	CmpackTable *tab = NULL;
	res = cmpack_adcurve(lc, fset.Handle(), &tab, CMPACK_ADCURVE_DEFAULT);
	if (res==0) {
		*table = new CTable(tab);
		cmpack_tab_destroy(tab);  // Release my reference
	}
	cmpack_adcurve_destroy(lc);
	return res;
}

//-----------------------   TRACK CURVE CLASS INTERFACE   ----------------------

//
// Make track curve
//
int CmpackTrackCurve(CConsole *pProgress, CTable **table, const CFrameSet &fset)
{
	int res;
	CmpackConsole *con = NULL;
	
	*table = NULL;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;

	// Output handling
	if (pProgress) {
		con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
	}

	CmpackTable *tab = NULL;
	res = cmpack_tcurve(fset.Handle(), &tab, CMPACK_TCURVE_FRAME_IDS, con);
	if (res==0) {
		*table = new CTable(tab);
		cmpack_tab_destroy(tab);
	}
	if (con)
		cmpack_con_destroy(con);
	return res;
}

//-----------------------   AIR MASS CLASS INTERFACE   ----------------------

//
// Air mass coefficient computation
//
int CmpackAirMass(CConsole *pProgress, CFrameSet &fset, const CObjectCoords &obj, 
	const CLocation &obs)
{
	int res;
	double ra, dec, lon, lat;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;

	if (!obj.Valid())
		return CMPACK_ERR_NO_OBJ_COORDS;
	if (cmpack_strtora(obj.RA(), &ra)!=0)
		return CMPACK_ERR_INVALID_RA;
	if (cmpack_strtodec(obj.Dec(), &dec)!=0)
		return CMPACK_ERR_INVALID_DEC;

	if (!obs.Valid())
		return CMPACK_ERR_NO_OBS_COORDS;
	if (cmpack_strtolon(obs.Lon(), &lon)!=0)
		return CMPACK_ERR_INVALID_LON;
	if (cmpack_strtolat(obs.Lat(), &lat)!=0)
		return CMPACK_ERR_INVALID_LAT;

	CmpackConsole *con = NULL;
	if (pProgress) {
		con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
	}

	res = cmpack_airmass_fset(fset.Handle(), obj.Name(), ra, dec,
		obs.Name(), lon, lat, con);
		
	if (con)
		cmpack_con_destroy(con);
	return res;
}

//
// Make light curve
//
int CmpackAirMassCurve(CConsole *pProgress, CTable **table, const CFrameSet &fset,
	const CObjectCoords &obj, const CLocation &loc)
{
	int res;
	double ra, dec, lon, lat;
	CmpackConsole *con = NULL;

	*table = NULL;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;
	
	// Output handling
	if (pProgress) {
		con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
	}

	if (!obj.Valid())
		return CMPACK_ERR_NO_OBJ_COORDS;
	if (cmpack_strtora(obj.RA(), &ra)!=0)
		return CMPACK_ERR_INVALID_RA;
	if (cmpack_strtodec(obj.Dec(), &dec)!=0)
		return CMPACK_ERR_INVALID_DEC;

	if (!loc.Valid())
		return CMPACK_ERR_NO_OBS_COORDS;
	if (cmpack_strtolon(loc.Lon(), &lon)!=0)
		return CMPACK_ERR_INVALID_LON;
	if (cmpack_strtolat(loc.Lat(), &lat)!=0)
		return CMPACK_ERR_INVALID_LAT;

	CmpackTable *tab = NULL;
	res = cmpack_airmass_curve(fset.Handle(), &tab, NULL, ra, dec, NULL, lon, lat,
		CMPACK_AMASS_FRAME_IDS, con);
	if (res==0) {
		*table = new CTable(tab);
		cmpack_tab_destroy(tab);
	}
	if (con)
		cmpack_con_destroy(con);
	return res;
}

//-----------------------   HELIOC. CORRECTION INTERFACE   ----------------------

//
// Heliocentric correction computation
//
int CmpackHelCorr(CConsole *pProgress, CFrameSet &fset, 
	const CObjectCoords &obj)
{
	int res;
	double ra, dec;

	if (!fset.Valid())
		return CMPACK_ERR_INVALID_CONTEXT;

	if (!obj.Valid())
		return CMPACK_ERR_NO_OBJ_COORDS;
	if (cmpack_strtora(obj.RA(), &ra)!=0)
		return CMPACK_ERR_INVALID_RA;
	if (cmpack_strtodec(obj.Dec(), &dec)!=0)
		return CMPACK_ERR_INVALID_DEC;

	// Output handling
	CmpackConsole *con = NULL;
	if (pProgress) {
		con = cmpack_con_init_cb(CConsole::OutputProc, pProgress);
		if (CConfig::GetBool(CConfig::DEBUG_OUTPUTS))
			cmpack_con_set_level(con, CMPACK_LEVEL_DEBUG);
	}

	res = cmpack_helcorr_fset(fset.Handle(), obj.Name(), ra, dec, con);

	if (con)
		cmpack_con_destroy(con);
	return res;
}

//----------------------   NEW FILES   ----------------------------

// Constructor
CUpdateProc::CUpdateProc(void):m_Progress(NULL), m_Konv(NULL), m_TCor(NULL), 
	m_Bias(NULL), m_Dark(NULL), m_Flat(NULL), m_Phot(NULL), m_Match(NULL)
{
}

// Destructor
CUpdateProc::~CUpdateProc()
{
	if (m_Konv) {
		delete m_Konv;
		m_Konv = NULL;
	}
	if (m_TCor) {
		delete m_TCor;
		m_TCor = NULL;
	}
	if (m_Bias) {
		delete m_Bias;
		m_Bias = NULL;
	}
	if (m_Dark) {
		delete m_Dark;
		m_Dark = NULL;
	}
	if (m_Flat) {
		delete m_Flat;
		m_Flat = NULL;
	}
	if (m_Phot) {
		delete m_Phot;
		m_Phot = NULL;
	}
	if (m_Match) {
		delete m_Match;
		m_Match = NULL;
	}
}

int CUpdateProc::Init(CConsole *pCon)
{
	int res = 0;
	bool do_konv, do_tcor, do_bias, do_dark, do_flat, do_phot, do_match;
	CProject::tStatus status;

	// Check project
	if (!g_Project->isOpen())
		return CMPACK_ERR_CLOSED_FILE;
	if (g_Project->isReadOnly())
		return CMPACK_ERR_READ_ONLY;

	m_Progress = pCon;

	g_Project->GetStatus(&status);
	do_konv = status.converted>0;
	do_tcor = g_Project->GetDbl("TimeCorr", "Seconds", 0)!=0;
	do_bias = g_Project->GetOrigBiasFile()->Valid();
	do_dark = g_Project->GetOrigDarkFile()->Valid();
	do_flat = g_Project->GetTempFlatFile()->Valid();
	do_phot = status.photometred>0;
	do_match = g_Project->GetReferenceType()!=REF_UNDEFINED;
	if (do_konv) {
		m_Konv = new CConvertProc();
		res = m_Konv->Init(pCon);
	}
	if (res==0 && do_tcor) {
		m_TCor = new CTimeCorrProc();
		res = m_TCor->Init(pCon, g_Project->GetDbl("TimeCorr", "Seconds", 0), false);
	}
	if (res==0 && do_bias) {
		m_Bias = new CBiasCorrProc();
		res = m_Bias->Init(pCon, g_Project->GetOrigBiasFile()->FullPath());
	}
	if (res==0 && do_dark) {
		m_Dark = new CDarkCorrProc();
		res = m_Dark->Init(pCon, g_Project->GetOrigDarkFile()->FullPath());
	}
	if (res==0 && do_flat) {
		m_Flat = new CFlatCorrProc();
		res = m_Flat->Init(pCon, g_Project->GetOrigFlatFile()->FullPath());
	}
	if (res==0 && do_phot) {
		m_Phot = new CPhotometryProc();
		res = m_Phot->Init(pCon);
	}
	if (res==0 && do_match) {
		m_Match = new CMatchingProc();
		res = m_Match->Reinitialize(pCon);
	}
	if (res!=0) {
		char *msg = cmpack_formaterror(res);
		pCon->Print(msg);
		cmpack_free(msg);
	}
	return res;
}

int CUpdateProc::Execute(GtkTreePath *path)
{
	gchar msg[256];

	int state = g_Project->GetState(path);
	sprintf(msg, "Frame #%d:", g_Project->GetFrameID(path));
	m_Progress->Print(msg);

	int file_res = 0;
	if (file_res==0 && m_Konv && (state & CFILE_CONVERSION)==0)
		file_res = m_Konv->Execute(path);
	if (file_res==0 && m_TCor && (state & (CFILE_TIMECORR | CFILE_PHOTOMETRY))==0)
		file_res = m_TCor->Execute(path);
	if (file_res==0 && m_Bias && (state & (CFILE_BIASCORR | CFILE_DARKCORR | CFILE_FLATCORR | CFILE_PHOTOMETRY))==0) 
		file_res = m_Bias->Execute(path);
	if (file_res==0 && m_Dark && (state & (CFILE_DARKCORR | CFILE_FLATCORR | CFILE_PHOTOMETRY))==0) 
		file_res = m_Dark->Execute(path);
	if (file_res==0 && m_Flat && (state & (CFILE_FLATCORR | CFILE_PHOTOMETRY))==0) 
		file_res = m_Flat->Execute(path);
	if (file_res==0 && m_Phot && (state & CFILE_PHOTOMETRY)==0)
		file_res = m_Phot->Execute(path);
	if (file_res==0 && m_Match && (state & CFILE_MATCHING)==0)
		file_res = m_Match->Execute(path);

	if (file_res!=0) {
		char *msg = cmpack_formaterror(file_res);
		m_Progress->Print(msg);
		cmpack_free(msg);
	}
	return file_res;
}
