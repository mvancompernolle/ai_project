/**************************************************************

preview_dlg.cpp (C-Munipack project)
The preview dialog
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
#include <math.h>

#include "main.h"
#include "utils.h"
#include "showheader_dlg.h"
#include "frameinfo_dlg.h"
#include "configuration.h"

enum tFieldId
{
	LABEL_FRAME,
	LABEL_FILENAME,
	LABEL_DIRPATH,
	LABEL_DATETIME,
	LABEL_JULDAT,
	LABEL_FILTER,
	LABEL_EXPTIME,
	LABEL_CCDTEMP,
	LABEL_AVGFRAMES,
	LABEL_SUMFRAMES,
	LABEL_BIAS,
	LABEL_DARK,
	LABEL_FLAT,
	LABEL_TIME,
	LABEL_STARS,
	LABEL_MATCHED,
	LABEL_SOURCE,
	LABEL_CALIBRATION,
	LABEL_PHOTOMETRY
};

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CFrameInfoDlg::CFrameInfoDlg(GtkWindow *pParent):CInfoDlg(pParent, 0), m_pFile(NULL)
{
	GtkWidget *bbox;

	// Labels
	AddHeading(LABEL_FRAME, 0, 0, "Frame");
	AddField(LABEL_DATETIME, 0, 1, "Date and time (UTC)");
	AddField(LABEL_JULDAT, 0, 2, "Julian date");
	AddField(LABEL_FILTER, 0, 3, "Optical filter", PANGO_ELLIPSIZE_END);
	AddField(LABEL_EXPTIME, 0, 4, "Exposure duration");
	AddField(LABEL_CCDTEMP, 0, 5, "CCD temperature");
	AddField(LABEL_AVGFRAMES, 0, 6, "Subframes averaged");
	AddField(LABEL_SUMFRAMES, 0, 7, "Subframes summed");
	AddSeparator(0, 8);
	AddHeading(LABEL_SOURCE, 0, 9, "Source file");
	AddField(LABEL_FILENAME, 0, 10, "File name", PANGO_ELLIPSIZE_MIDDLE);
	AddField(LABEL_DIRPATH, 0, 11, "Location", PANGO_ELLIPSIZE_MIDDLE);
	AddSeparator(0, 12);
	AddHeading(LABEL_CALIBRATION, 0, 13, "Calibration");
	AddField(LABEL_BIAS, 0, 14, "Bias frame", PANGO_ELLIPSIZE_MIDDLE);
	AddField(LABEL_DARK, 0, 15, "Dark frame", PANGO_ELLIPSIZE_MIDDLE);
	AddField(LABEL_FLAT, 0, 16, "Flat frame", PANGO_ELLIPSIZE_MIDDLE);
	AddField(LABEL_TIME, 0, 17, "Time corr.");
	AddSeparator(0, 18);
	AddHeading(LABEL_PHOTOMETRY, 0, 19, "Photometry and matching");
	AddField(LABEL_STARS, 0, 20, "Stars found");
	AddField(LABEL_MATCHED, 0, 21, "Stars matched");
	gtk_widget_show_all(m_Tab);

	// Separator
	gtk_box_pack_start(GTK_BOX(m_Box), gtk_label_new(NULL), FALSE, TRUE, 0);

	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_box_pack_start(GTK_BOX(m_Box), bbox, FALSE, TRUE, 0);
	m_OrigBtn = gtk_button_new_with_label("Show original image header");
	gtk_box_pack_start(GTK_BOX(bbox), m_OrigBtn, FALSE, TRUE, 0);
	m_TempBtn = gtk_button_new_with_label("Show calibrated image header");
	gtk_box_pack_start(GTK_BOX(bbox), m_TempBtn, FALSE, TRUE, 0);
	m_PhotBtn = gtk_button_new_with_label("Show photometry file header");
	gtk_box_pack_start(GTK_BOX(bbox), m_PhotBtn, FALSE, TRUE, 0);
	
	g_signal_connect(G_OBJECT(m_OrigBtn), "clicked", G_CALLBACK(button_clicked), this);
	g_signal_connect(G_OBJECT(m_TempBtn), "clicked", G_CALLBACK(button_clicked), this);
	g_signal_connect(G_OBJECT(m_PhotBtn), "clicked", G_CALLBACK(button_clicked), this);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CFrameInfoDlg::~CFrameInfoDlg()
{
	if (m_pFile)
		gtk_tree_row_reference_free(m_pFile);
}

void CFrameInfoDlg::Show(GtkTreePath *pFile)
{
	int state, avg_frames, sum_frames, mstars, nstars, frame_id;
	char buf[512], msg[512];
	double exptime, ccdtemp, timecorr, jd;
	char *orig_path, *temp_file, *phot_file, *filter;
	char *bias_file, *dark_file, *flat_file, *report;
	CmpackDateTime dt;
	GtkTreeIter iter;
	GtkTreeModel *pModel = g_Project->FileList();

	if (m_pFile) {
		gtk_tree_row_reference_free(m_pFile);
		m_pFile = NULL;
	}

	// Get frame data
	if (pFile) {
		m_pFile = gtk_tree_row_reference_new(pModel, pFile);
		gtk_tree_model_get_iter(pModel, &iter, pFile);
		// Enable/disable controls
		gtk_tree_model_get(pModel, &iter, FRAME_STATE, &state, 
			FRAME_JULDAT, &jd, FRAME_ID, &frame_id,
			FRAME_ORIGFILE, &orig_path, FRAME_PHOTFILE, &phot_file, 
			FRAME_TEMPFILE, &temp_file, FRAME_REPORT, &report,
			FRAME_BIASFILE, &bias_file, FRAME_DARKFILE, &dark_file, 
			FRAME_FLATFILE, &flat_file, FRAME_TIMECORR, &timecorr, 
			FRAME_FILTER, &filter, FRAME_EXPTIME, &exptime, 
			FRAME_CCDTEMP, &ccdtemp, FRAME_STARS, &nstars, 
			FRAME_MSTARS, &mstars, FRAME_AVGFRAMES, &avg_frames, 
			FRAME_SUMFRAMES, &sum_frames, -1);
		// Init internal variables
		sprintf(buf, "Frame #%d - properties - %s", frame_id, g_AppTitle);
		gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);
		// Update properties
		if (orig_path) {
			gchar *basename = g_path_get_basename(orig_path);
			SetField(LABEL_FILENAME, basename);
			g_free(basename);
			gchar *dirpath = g_path_get_dirname(orig_path);
			SetField(LABEL_DIRPATH, dirpath);
			g_free(dirpath);
		} else {
			SetField(LABEL_FILENAME, "Not available");
			SetField(LABEL_DIRPATH, "Not available");
		}
		if (jd>0) {
			cmpack_decodejd(jd, &dt);
			sprintf(buf, "%04d-%02d-%02d %d:%02d:%02d.%03d", dt.date.year, dt.date.month, dt.date.day,
				dt.time.hour, dt.time.minute, dt.time.second, dt.time.milisecond);
			SetField(LABEL_DATETIME, buf);
			SetField(LABEL_JULDAT, jd, JD_PREC);
		} else {
			SetField(LABEL_DATETIME, "Not available");
			SetField(LABEL_JULDAT, "Not available");
		}
		if (filter)
			SetField(LABEL_FILTER, filter);
		else
			SetField(LABEL_FILTER, "Not available");
		if (exptime>=0.0)
			SetField(LABEL_EXPTIME, exptime, 3, "second(s)");
		else
			SetField(LABEL_EXPTIME, "Not available");
		if (ccdtemp>-999 && ccdtemp<999)
			SetField(LABEL_CCDTEMP, ccdtemp, 2, "\xC2\xB0""C");
		else
			SetField(LABEL_CCDTEMP, "Not available");
		if (avg_frames>0)
			SetField(LABEL_AVGFRAMES, avg_frames, "frame(s)");
		else
			SetField(LABEL_AVGFRAMES, "Not available");
		if (sum_frames>0)
			SetField(LABEL_SUMFRAMES, sum_frames, "frame(s)");
		else
			SetField(LABEL_SUMFRAMES, "Not available");
		if (bias_file)
			SetField(LABEL_BIAS, bias_file);
		else
			SetField(LABEL_BIAS, "Not used");
		if (dark_file)
			SetField(LABEL_DARK, dark_file);
		else
			SetField(LABEL_DARK, "Not used");
		if (flat_file)
			SetField(LABEL_FLAT, flat_file);
		else
			SetField(LABEL_FLAT, "Not used");
		if (timecorr!=0) {
			double days;
			int msecs = (int)(modf(fabs(timecorr)/86400.0, &days)*86400000);
			*msg = '\0';
			if (days>0) {
				sprintf(buf, "%d %s ", (int)days, (days==1 ? "day" : "days"));
				strcat(msg, buf);
			} 
			int hours = msecs/3600000;
			if (hours>0) {
				sprintf(buf, "%d %s ", hours, (hours==1 ? "hour" : "hours"));
				strcat(msg, buf);
			} 
			int minutes = (msecs/60000)%60;
			if (minutes>0) {
				sprintf(buf, "%d %s ", minutes, (minutes==1 ? "minute" : "minutes"));
				strcat(msg, buf);
			} 
			int seconds = (msecs/1000)%60;
			msecs = msecs%1000;
			if (seconds>0 || msecs>0) {
				if (seconds>0 && msecs==0) 
					sprintf(buf, "%.2d %s ", seconds, (seconds==1 ? "second" : "seconds"));
				else if (seconds==0 && msecs>0)
					sprintf(buf, "%.2d %s ", msecs, (msecs==1 ? "millisecond" : "milliseconds"));
				else 
					sprintf(buf, "%.2d.%.3d %s ", seconds, msecs, "seconds");
				strcat(msg, buf);
			}
			strcat(msg, (timecorr>0 ? "to future" : "to past"));
			SetField(LABEL_TIME, msg);
		} else
			SetField(LABEL_TIME, "Not used");
		if (nstars>=0)
			SetField(LABEL_STARS, nstars, "star(s)");
		else
			SetField(LABEL_STARS, "Not available");
		if (mstars>=0)
			SetField(LABEL_MATCHED, mstars, "star(s)");
		else
			SetField(LABEL_MATCHED, "Not available");
		// Enable/disable buttons
		gtk_widget_set_sensitive(m_OrigBtn, orig_path && *orig_path!='\0');
		gtk_widget_set_sensitive(m_TempBtn, (state & CFILE_CONVERSION)!=0 && temp_file && *temp_file!='\0');
		gtk_widget_set_sensitive(m_PhotBtn, (state & CFILE_PHOTOMETRY)!=0 && phot_file && *phot_file!='\0');
		g_free(orig_path);
		g_free(filter);
		g_free(temp_file);
		g_free(phot_file);
		g_free(bias_file);
		g_free(dark_file);
		g_free(flat_file);
		g_free(report);
	}

	CInfoDlg::ShowModal();
}

void CFrameInfoDlg::button_clicked(GtkWidget *button, CFrameInfoDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CFrameInfoDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_OrigBtn))
		ShowHeader_Orig();
	else if (pBtn==GTK_WIDGET(m_TempBtn))
		ShowHeader_Temp();
	else if (pBtn==GTK_WIDGET(m_PhotBtn))
		ShowHeader_Phot();
}

void CFrameInfoDlg::ShowHeader_Orig(void)
{
	gchar *orig_file;
	GtkTreeIter iter;

	if (!m_pFile)
		return;

	GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		GtkTreeModel *pModel = g_Project->FileList();
		gtk_tree_model_get_iter(pModel, &iter, pPath);
		gtk_tree_model_get(pModel, &iter, FRAME_ORIGFILE, &orig_file, -1);
		CCCDFile file;
		GError *error = NULL;
		if (file.Open(orig_file, CMPACK_OPEN_READONLY, &error)) {
			CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
			gchar *basename = g_path_get_basename(orig_file);
			dlg.Execute(&file, basename);
			g_free(basename);
			g_free(orig_file);
		} else {
			if (error) {
				ShowError(GTK_WINDOW(m_pDlg), error->message);
				g_error_free(error);
			}
		}
		gtk_tree_path_free(pPath);
	}
}

void CFrameInfoDlg::ShowHeader_Temp(void)
{
	int state;
	gchar *temp_file;
	GtkTreeIter iter;

	if (!m_pFile)
		return;

	GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		GtkTreeModel *pModel = g_Project->FileList();
		gtk_tree_model_get_iter(pModel, &iter, pPath);
		gtk_tree_model_get(pModel, &iter, FRAME_STATE, &state, -1);
		if ((state & CFILE_CONVERSION)!=0) {
			gtk_tree_model_get(pModel, &iter, FRAME_TEMPFILE, &temp_file, -1);
			if (temp_file) {
				gchar *fpath = g_build_filename(g_Project->DataDir(), temp_file, NULL);
				CCCDFile file;
				GError *error = NULL;
				if (file.Open(fpath, CMPACK_OPEN_READONLY, &error)) {
					CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
					gchar *basename = g_path_get_basename(fpath);
					dlg.Execute(&file, basename);
					g_free(basename);
				} else {
					if (error) {
						ShowError(GTK_WINDOW(m_pDlg), error->message);
						g_error_free(error);
					}
				}
				g_free(fpath);
				g_free(temp_file);
			}
		}
		gtk_tree_path_free(pPath);
	}
}

void CFrameInfoDlg::ShowHeader_Phot(void)
{
	if (!m_pFile)
		return;

	GtkTreePath *pPath = gtk_tree_row_reference_get_path(m_pFile);
	if (pPath) {
		if (g_Project->GetState(pPath) & CFILE_PHOTOMETRY) {
			gchar *pht_file = g_Project->GetPhotFile(pPath);
			if (pht_file) {
				CPhot phot;
				GError *error = NULL;
				if (phot.Load(pht_file, &error)) {
					CShowHeaderDlg dlg(GTK_WINDOW(m_pDlg));
					gchar *basename = g_path_get_basename(pht_file);
					dlg.Execute(&phot, basename);
					g_free(basename);
				} else {
					if (error) {
						ShowError(GTK_WINDOW(m_pDlg), error->message);
						g_error_free(error);
					}
				}
				g_free(pht_file);
			}
		}
		gtk_tree_path_free(pPath);
	}
}
