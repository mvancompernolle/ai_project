/**************************************************************

timecorr_dlg.cpp (C-Munipack project)
The 'Time correction' dialog
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

#include "project.h"
#include "timecorr_dlg.h"
#include "progress_dlg.h"
#include "main.h"
#include "utils.h"
#include "configuration.h"
#include "proc_classes.h"
#include "ctxhelp.h"

//-------------------------   MAIN WINDOW   --------------------------------

CTimeCorrDlg::CTimeCorrDlg(GtkWindow *pParent):m_pParent(pParent), m_InFiles(0),
	m_OutFiles(0), m_FileList(NULL), m_TMode(SHIFT), m_TCorr(0)
{
	GtkWidget *vbox, *tbox;
	GSList *group;
	GtkObject *adj;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Time correction", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	
	// Dialog icon
	gchar *icon = get_icon_file("timecorr");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Frame selection
	m_ProcLabel = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(m_ProcLabel), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(m_ProcLabel), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_ProcLabel, FALSE, TRUE, 0);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Convert all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Convert frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	// Separator
	m_ProcSep = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), m_ProcSep, FALSE, TRUE, 0);

	// Correction
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Shift date and time of observation</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

	// Frame layout
	tbox = gtk_table_new(7, 8, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), tbox);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 0, 8);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 1, 8);
	gtk_table_set_row_spacing(GTK_TABLE(tbox), 2, 8);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacing(GTK_TABLE(tbox), 4, 8);
	// By ... seconds
	m_BySeconds = gtk_radio_button_new_with_label(NULL, "by");
	gtk_widget_set_tooltip_text(m_BySeconds, "Specify time correction in seconds");
	g_signal_connect(G_OBJECT(m_BySeconds), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_BySeconds, 0, 1, 0, 1);
	adj = gtk_adjustment_new(0, 0, 24*3600, 1, 60, 0);
	m_Seconds = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 3);
	gtk_widget_set_tooltip_text(m_Seconds, "Time correction in seconds and fraction of seconds");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_Seconds), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Seconds, 1, 3, 0, 1);
	gtk_widget_set_size_request(m_Seconds, 80, -1);
	m_SecsUnit = gtk_label_new("seconds");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_SecsUnit, 3, 5, 0, 1);
	gtk_misc_set_alignment(GTK_MISC(m_SecsUnit), 0, 0.5);
	// By ... hours
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_BySeconds));
	m_ByHours = gtk_radio_button_new_with_label(group, "by");
	gtk_widget_set_tooltip_text(m_ByHours, "Specify time correction in hours");
	g_signal_connect(G_OBJECT(m_ByHours), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ByHours, 0, 1, 1, 2);
	adj = gtk_adjustment_new(0, 0, 1000, 1, 10, 0);
	m_Hours = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 3);
	gtk_widget_set_tooltip_text(m_Hours, "Time correction in hours and fraction of hours");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_Hours), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Hours, 1, 3, 1, 2);
	gtk_widget_set_size_request(m_Hours, 80, -1);
	m_HoursUnit = gtk_label_new("hours");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_HoursUnit, 3, 5, 1, 2);
	gtk_misc_set_alignment(GTK_MISC(m_HoursUnit), 0, 0.5);
	// By ... days
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_BySeconds));
	m_ByDays = gtk_radio_button_new_with_label(group, "by");
	gtk_widget_set_tooltip_text(m_ByDays, "Specify time correction in days");
	g_signal_connect(G_OBJECT(m_ByDays), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ByDays, 0, 1, 2, 3);
	adj = gtk_adjustment_new(0, 0, 9999, 1, 10, 0);
	m_Days = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_Days, "Time correction in days and fraction of days");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_Days), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_Days, 1, 3, 2, 3);
	gtk_widget_set_size_request(m_Days, 80, -1);
	m_DaysUnit = gtk_label_new("days");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DaysUnit, 3, 5, 2, 3);
	gtk_misc_set_alignment(GTK_MISC(m_DaysUnit), 0, 0.5);
	// To the future
	m_ToFuture = gtk_radio_button_new_with_label(NULL, "to the future");
	gtk_widget_set_tooltip_text(m_ToFuture, "Shift observation time forward");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToFuture, 5, 8, 0, 1);
	// To the past
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_ToFuture));
	m_ToPast = gtk_radio_button_new_with_label(group, "to the past");
	gtk_widget_set_tooltip_text(m_ToPast, "Shift observation time backward");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToPast, 5, 8, 1, 2);
	// By differention of two days
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_BySeconds));
	m_ByDiff = gtk_radio_button_new_with_label(group, "by differention of dates:");
	gtk_widget_set_tooltip_text(m_ByDiff, "Specify time correction as an interval between two dates");
	g_signal_connect(G_OBJECT(m_ByDiff), "toggled", G_CALLBACK(button_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ByDiff, 0, 8, 3, 4);
	m_YearLabel = gtk_label_new("Year");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_YearLabel, 2, 3, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_YearLabel), 0, 0.5);
	m_MonLabel = gtk_label_new("Month");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_MonLabel, 3, 4, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_MonLabel), 0, 0.5);
	m_DayLabel = gtk_label_new("Day");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_DayLabel, 4, 5, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_DayLabel), 0, 0.5);
	m_HourLabel = gtk_label_new("Hour");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_HourLabel, 5, 6, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_HourLabel), 0, 0.5);
	m_MinLabel = gtk_label_new("Minute");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_MinLabel, 6, 7, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_MinLabel), 0, 0.5);
	m_SecLabel = gtk_label_new("Second");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_SecLabel, 7, 8, 4, 5);
	gtk_misc_set_alignment(GTK_MISC(m_SecLabel), 0, 0.5);
	// From date
	m_FromLabel = gtk_label_new("from");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromLabel, 1, 2, 5, 6);
	gtk_misc_set_alignment(GTK_MISC(m_FromLabel), 0, 0.5);
	adj = gtk_adjustment_new(0, 1900, 2999, 1, 10, 0);
	m_FromYear = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromYear), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromYear, 2, 3, 5, 6);
	adj = gtk_adjustment_new(0, 1, 12, 1, 10, 0);
	m_FromMonth = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromMonth), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromMonth, 3, 4, 5, 6);
	adj = gtk_adjustment_new(0, 1, 31, 1, 10, 0);
	m_FromDay = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromDay), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromDay, 4, 5, 5, 6);
	adj = gtk_adjustment_new(0, 0, 23, 1, 10, 0);
	m_FromHour = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromHour), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromHour, 5, 6, 5, 6);
	adj = gtk_adjustment_new(0, 0, 59, 1, 10, 0);
	m_FromMinute = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromMinute), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromMinute, 6, 7, 5, 6);
	adj = gtk_adjustment_new(0, 0, 60, 1, 10, 0);
	m_FromSecond = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 3);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_FromSecond), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_FromSecond, 7, 8, 5, 6);
	// To date
	m_ToLabel = gtk_label_new("to");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToLabel, 1, 2, 6, 7);
	gtk_misc_set_alignment(GTK_MISC(m_ToLabel), 0, 0.5);
	adj = gtk_adjustment_new(0, 1900, 2999, 1, 10, 0);
	m_ToYear = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToYear), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToYear, 2, 3, 6, 7);
	adj = gtk_adjustment_new(0, 1, 12, 1, 10, 0);
	m_ToMonth = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToMonth), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToMonth, 3, 4, 6, 7);
	adj = gtk_adjustment_new(0, 1, 31, 1, 10, 0);
	m_ToDay = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToDay), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToDay, 4, 5, 6, 7);
	adj = gtk_adjustment_new(0, 0, 23, 1, 10, 0);
	m_ToHour = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToHour), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToHour, 5, 6, 6, 7);
	adj = gtk_adjustment_new(0, 0, 59, 1, 10, 0);
	m_ToMinute = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToMinute), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToMinute, 6, 7, 6, 7);
	adj = gtk_adjustment_new(0, 0, 60, 1, 10, 0);
	m_ToSecond = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 3);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ToSecond), true);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_ToSecond, 7, 8, 6, 7);

	// Separator
	m_OptSep = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), m_OptSep, FALSE, TRUE, 0);

	m_OptLabel = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(m_OptLabel), "<b>Options</b>");
	gtk_misc_set_alignment(GTK_MISC(m_OptLabel), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_OptLabel, FALSE, TRUE, 0);

	m_ResetBtn = gtk_check_button_new_with_label("Revert to original date and time");
	gtk_widget_set_tooltip_text(m_ResetBtn, "Reset time correction and revert to the original date and time of observation stored in the source files.");
	gtk_box_pack_start(GTK_BOX(vbox), m_ResetBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_ResetBtn), "toggled", G_CALLBACK(button_clicked), this);
	
	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CTimeCorrDlg::~CTimeCorrDlg()
{
	gtk_widget_destroy(m_pDlg);
}

gboolean CTimeCorrDlg::foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
	return FALSE;
}

void CTimeCorrDlg::foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
}

void CTimeCorrDlg::button_clicked(GtkWidget *button, CTimeCorrDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CTimeCorrDlg::OnButtonClicked(GtkWidget *pBtn)
{
	UpdateControls();
}

void CTimeCorrDlg::response_dialog(GtkDialog *pDlg, gint response_id, CTimeCorrDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CTimeCorrDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_TIME_CORRECTION);
		return false;
	}
	return true;
}

void CTimeCorrDlg::UpdateControls(void)
{
	int mode;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ResetBtn)))
		mode = 0;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BySeconds)))
		mode = 1;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ByHours)))
		mode = 2;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ByDays)))
		mode = 3;
	else
		mode = 4;

	gtk_widget_set_sensitive(GTK_WIDGET(m_BySeconds), mode>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_Seconds), mode==1);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SecsUnit), mode==1);

	gtk_widget_set_sensitive(GTK_WIDGET(m_ByHours), mode>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_Hours), mode==2);
	gtk_widget_set_sensitive(GTK_WIDGET(m_HoursUnit), mode==2);

	gtk_widget_set_sensitive(GTK_WIDGET(m_ByDays), mode>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_Days), mode==3);
	gtk_widget_set_sensitive(GTK_WIDGET(m_DaysUnit), mode==3);

	gtk_widget_set_sensitive(GTK_WIDGET(m_ToFuture), mode>=1 && mode<=3);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToPast), mode>=1 && mode<=3);

	gtk_widget_set_sensitive(GTK_WIDGET(m_ByDiff), mode>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_YearLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_MonLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_DayLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_HourLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_MinLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_SecLabel), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromYear), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromMonth), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromDay), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromHour), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromMinute), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_FromSecond), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToYear), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToMonth), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToDay), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToHour), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToMinute), mode==4);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ToSecond), mode==4);
}

void CTimeCorrDlg::Init()
{
	bool sign;
	double seconds, hours, days, jd;
	GtkTreeModel *model;
	GtkTreeIter iter;
	CmpackDateTime dt;

	switch (g_Project->GetInt("TimeCorr", "Mode", 0))
	{
	case 1:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ByDays), true);
		break;
	case 2:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ByDiff), true);
		break;
	case 4:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ByHours), true);
		break;
	default:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_BySeconds), true);
		break;
	}

	seconds = modf(fabs(m_TCorr/(24*3600)), &days)*(24*3600);
	seconds = modf(fabs(seconds/3600), &hours)*3600;
	sign = m_TCorr<0;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Seconds), seconds);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Hours), hours);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Days), days);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ToFuture), !sign);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ToPast), sign);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ResetBtn), m_TMode==RESET);

	jd = 0.0;
	GList *list = gtk_tree_selection_get_selected_rows(g_MainWnd->GetSelection(), &model);
	if (list && model && gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)list->data)) 
		gtk_tree_model_get(model, &iter, FRAME_JULDAT, &jd, -1);
	g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);
	if (jd==0 && gtk_tree_model_get_iter_first(g_Project->FileList(), &iter)) 
		gtk_tree_model_get(g_Project->FileList(), &iter, FRAME_JULDAT, &jd, -1);

	if (jd>0.0) {
		cmpack_decodejd(jd, &dt);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromYear), dt.date.year);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromMonth), dt.date.month);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromDay), dt.date.day);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromHour), dt.time.hour);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromMinute), dt.time.minute);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FromSecond), dt.time.second + 0.001*dt.time.milisecond);
		jd += m_TCorr/(24*3600);
		cmpack_decodejd(jd, &dt);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToYear), dt.date.year);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToMonth), dt.date.month);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToDay), dt.date.day);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToHour), dt.time.hour);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToMinute), dt.time.minute);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ToSecond), dt.time.second + 0.001*dt.time.milisecond);
	}
}

void CTimeCorrDlg::GetParams(void)
{
	bool ok;
	double seconds, hours, days, jd0, jd1;
	CmpackDateTime dt;

	m_TCorr = 0;
	m_TMode = SHIFT;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ResetBtn))) {
		m_TMode = RESET;
	} else
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BySeconds))) {
		g_Project->SetInt("TimeCorr", "Mode", 0);
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ToFuture))!=0;
		seconds = fabs(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Seconds)));
		m_TCorr = (ok ? 1.0 : -1.0) * seconds;
	} else
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ByHours))) {
		g_Project->SetInt("TimeCorr", "Mode", 4);
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ToFuture))!=0;
		hours = fabs(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Hours)));
		m_TCorr = (ok ? 1.0 : -1.0) * hours * 3600;
	} else
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ByDays))) {
		g_Project->SetInt("TimeCorr", "Mode", 1);
		days = gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Days));
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ToFuture))!=0;
		m_TCorr = (ok ? 1.0 : -1.0) * days * (24*3600);
	} else
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ByDiff))) {
		g_Project->SetInt("TimeCorr", "Mode", 2);
		memset(&dt, 0, sizeof(CmpackDateTime));
		dt.date.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_FromYear));
		dt.date.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_FromMonth));
		dt.date.day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_FromDay));
		jd0 = cmpack_encodejd(&dt);
		dt.date.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_ToYear));
		dt.date.month = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_ToMonth));
		dt.date.day = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_ToDay));
		jd1 = cmpack_encodejd(&dt);
		days = (jd1-jd0);
		seconds =
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_ToHour))*3600.0 +
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_ToMinute))*60.0 +
			gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_ToSecond)) -
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_FromHour))*3600.0 -
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_FromMinute))*60.0 -
			gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FromSecond));
		m_TCorr = days*(24*3600) + seconds;
	}
}

void CTimeCorrDlg::Execute()
{
	int res;
	char msg[256];

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	m_TCorr = g_Project->GetDbl("TimeCorr", "Seconds", 0);
	m_TMode = SHIFT;
	gtk_widget_show(m_ProcLabel);
	gtk_widget_show(m_ProcSep);
	gtk_widget_show(m_AllBtn);
	gtk_widget_show(m_SelBtn);
	gtk_widget_show(m_OptLabel);
	gtk_widget_show(m_OptSep);
	gtk_widget_show(m_ResetBtn);
	Init();
	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	GetParams();
	g_Project->SetDbl("TimeCorr", "Seconds", m_TCorr);

	m_FileList = NULL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))) {
		// All files
		GtkTreeModel *pList = g_Project->FileList();
		if (gtk_tree_model_iter_n_children(pList, NULL)>0) 
			gtk_tree_model_foreach(pList, foreach_all_files, &m_FileList);
		else
			ShowError(m_pParent, "There are no files in the project.");
	} else {
		// Selected files
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		if (gtk_tree_selection_count_selected_rows(pSel)>0) 
			gtk_tree_selection_selected_foreach(pSel, foreach_sel_files, &m_FileList);
		else
			ShowError(m_pParent, "There are no selected files.");
	}
	if (m_FileList) {
		CProgressDlg pDlg(m_pParent, "Processing TIME correction");
		pDlg.SetMinMax(0, g_list_length(m_FileList));
		res = pDlg.Execute(ExecuteProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else
		if (m_OutFiles==0) {
			ShowError(m_pParent, "No file was successfully processed.", true);
		} else {
			if (m_OutFiles!=m_InFiles) {
				sprintf(msg, "%d file(s) were successfully processed, %d file(s) failed.", 
					m_OutFiles, m_InFiles-m_OutFiles);
				ShowWarning(m_pParent, msg, true);
			} else {
				sprintf(msg, "All %d file(s) were successfully processed.", m_OutFiles);
				ShowInformation(m_pParent, msg, true);
			}
		}
		g_list_foreach(m_FileList, (GFunc)gtk_tree_row_reference_free, NULL);
		g_list_free(m_FileList);
		m_FileList = NULL;
	}
	g_Project->Save();
}

bool CTimeCorrDlg::EditParams(double *seconds)
{
	m_TCorr = *seconds;
	m_TMode = SHIFT;
	gtk_widget_hide(m_ProcLabel);
	gtk_widget_hide(m_AllBtn);
	gtk_widget_hide(m_SelBtn);
	gtk_widget_hide(m_ProcSep);
	gtk_widget_hide(m_OptLabel);
	gtk_widget_hide(m_OptSep);
	gtk_widget_hide(m_ResetBtn);
	Init();
	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))==GTK_RESPONSE_ACCEPT) {
		GetParams();
		*seconds = m_TCorr;
		return true;
	}
	return false;
}

int CTimeCorrDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CTimeCorrDlg*)userdata)->ProcessFiles(sender);
}

int CTimeCorrDlg::ProcessFiles(CProgressDlg *sender)
{
	int res, frameid;
	char *fpath, msg[128];
	GtkTreePath *path;
	CTimeCorrProc tcor;

	m_InFiles = m_OutFiles = 0;
	sender->Print("------ Time correction ------");

	res = tcor.Init(sender, m_TCorr, m_TMode==RESET);
	if (res==0) {
		g_Project->SetDbl("TimeCorr", "Seconds", m_TCorr);
		for (GList *node = m_FileList; node != NULL; node = node->next) {
			path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (path) {
				fpath = g_Project->GetImageFileName(path);
				frameid = g_Project->GetFrameID(path);
				sender->SetFileName(fpath);
				sender->SetProgress(m_InFiles++);
				g_free(fpath);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				if (tcor.Execute(path)==0)
					m_OutFiles++;
				gtk_tree_path_free(path);
			}
			if (sender->Cancelled()) 
				break;
		}
		if (sender->Cancelled())
			sprintf(msg, "Cancelled at the user's request");
		else
			sprintf(msg, "====== %d succeeded, %d failed ======", m_OutFiles, m_InFiles-m_OutFiles);
		sender->Print(msg);
	} else {
		char *aux = cmpack_formaterror(res);
		sender->Print(aux);
		cmpack_free(aux);
	}
	return res;
}
