/**************************************************************

chart_dlg.cpp (C-Munipack project)
The 'Plot chart' dialog
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

#include "project.h"
#include "chart_dlg.h"
#include "main.h"
#include "utils.h"
#include "configuration.h"
#include "configuration.h"
#include "export_dlgs.h"
#include "chart_dlg.h"
#include "ctxhelp.h"
#include "profile.h"

//-------------------------   PRIVATE DATA   ---------------------------

static const struct {
	const gchar *label;
	CmpackColor color;
} Types[CMPACK_SELECT_COUNT] = {
	{ NULL, CMPACK_COLOR_DEFAULT },
	{ "var", CMPACK_COLOR_RED },
	{ "comp", CMPACK_COLOR_GREEN },
	{ "check", CMPACK_COLOR_BLUE }
};

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CChartDlg::CChartDlg(GtkWindow *pParent):m_pParent(pParent), m_ChartData(NULL), m_ImageData(NULL)
{
	GtkWidget *scrwnd, *tbox;
	GdkRectangle rc;

	m_DisplayMode = (tDisplayMode)g_Project->GetInt("ChartDlg", "Mode", DISPLAY_IMAGE, 0, DISPLAY_FULL);
	m_Negative = CConfig::GetBool(CConfig::NEGATIVE_CHARTS);
	m_RowsUpward = CConfig::GetBool(CConfig::ROWS_UPWARD);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Chart", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_SAVE, GTK_RESPONSE_NONE, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_NONE, "Export the chart to a file");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog size
	GdkScreen *scr = gtk_window_get_screen(pParent);
	int mon = gdk_screen_get_monitor_at_window(scr, GTK_WIDGET(pParent)->window);
	gdk_screen_get_monitor_geometry(scr, mon, &rc);
	if (rc.width>0 && rc.height>0)
		gtk_window_set_default_size(GTK_WINDOW(m_pDlg), RoundToInt(0.7*rc.width), RoundToInt(0.7*rc.height));

	// Toolbar
	tbox = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(tbox), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tbox), GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), tbox, FALSE, FALSE, 0);

	m_ViewLabel = toolbar_new_label(tbox, "View");
	m_ShowImage = toolbar_new_radio_button(tbox, NULL, "Image", "Display an image only");
	g_signal_connect(G_OBJECT(m_ShowImage), "toggled", G_CALLBACK(button_clicked), this);
	m_ShowChart = toolbar_new_radio_button(tbox, m_ShowImage, "Chart", "Display objects on a flat background");
	g_signal_connect(G_OBJECT(m_ShowChart), "toggled", G_CALLBACK(button_clicked), this);
	m_ShowMixed = toolbar_new_radio_button(tbox, m_ShowImage, "Mixed", "Display objects over an image");
	g_signal_connect(G_OBJECT(m_ShowMixed), "toggled", G_CALLBACK(button_clicked), this);

	toolbar_new_separator(tbox);

	toolbar_new_label(tbox, "Zoom");
	m_ZoomFit = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_FIT, "Fit the frame to the window");
	g_signal_connect(G_OBJECT(m_ZoomFit), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomOut = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_OUT, "Zoom out");
	g_signal_connect(G_OBJECT(m_ZoomOut), "clicked", G_CALLBACK(button_clicked), this);
	m_ZoomIn = toolbar_new_button_from_stock(tbox, GTK_STOCK_ZOOM_IN, "Zoom in");
	g_signal_connect(G_OBJECT(m_ZoomIn), "clicked", G_CALLBACK(button_clicked), this);

	// Chart
	m_Chart = cmpack_chart_view_new();
	scrwnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrwnd), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(scrwnd), m_Chart);
	gtk_widget_set_size_request(m_Chart, 300, 200);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), scrwnd, TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CChartDlg::~CChartDlg()
{
	if (m_ImageData)
		g_object_unref(m_ImageData);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	gtk_widget_destroy(m_pDlg);
}

void CChartDlg::Execute(const CSelection &sel, int aperIndex)
{
	int	res = 0;
	const gchar *tmp_file;
	GtkTreePath *refpath;
	GError *error = NULL;

	m_Selection = sel;
	m_Tags = *g_Project->Tags();
	
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	m_Phot.Clear();
	m_Catalog.Clear();
	m_Image.Clear();

	m_RefType = g_Project->GetReferenceType();
	switch (m_RefType)
	{
	case REF_FRAME:
		// Load reference frame
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				GError *error = NULL;
				if (m_Phot.Load(pht_file, &error)) {
					m_Phot.SelectAperture(aperIndex);
					UpdateChart();
					gchar *fts_file = g_Project->GetImageFile(refpath);
					if (fts_file) {
						if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
							UpdateImage();
						g_free(fts_file);
					}
				} 
				if (error) {
					ShowError(m_pParent, error->message);
					g_error_free(error);
					res = -1;
				}
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		// Load catalog file
		tmp_file = g_Project->GetTempCatFile()->FullPath();
		if (m_Catalog.Load(tmp_file, &error)) {
			UpdateChart();
			gchar *fts_file = SetFileExtension(tmp_file, FILE_EXTENSION_FITS);
			if (fts_file) {
				if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
					UpdateImage();
				g_free(fts_file);
			}
		}
		if (error) {
			ShowError(m_pParent, error->message);
			g_error_free(error);
			res = -1;
		}
		break;

	default:
		ShowError(m_pParent, "No reference file.");
		res = -1;
	}

	if (res==0) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage), 
			m_DisplayMode==DISPLAY_IMAGE);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart), 
			m_DisplayMode==DISPLAY_CHART);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowMixed), 
			m_DisplayMode==DISPLAY_FULL);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ShowImage), 
			m_Image.Width()>0 && m_Image.Height()>0);
		gtk_widget_set_sensitive(GTK_WIDGET(m_ShowMixed), 
			m_Image.Width()>0 && m_Image.Height()>0);
		gtk_dialog_run(GTK_DIALOG(m_pDlg));
	}
}

void CChartDlg::UpdateImage(void)
{
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	if (m_DisplayMode!=DISPLAY_CHART) {
		m_ImageData = m_Image.ToImageData(m_Negative, false, true, m_RowsUpward);
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	}
}

void CChartDlg::UpdateChart(void)
{
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData)
		g_object_unref(m_ChartData);
	if (m_RefType==REF_FRAME)
		m_ChartData = m_Phot.ToChartData(false, m_DisplayMode==DISPLAY_IMAGE);
	else
		m_ChartData = m_Catalog.ToChartData(false, false, m_DisplayMode==DISPLAY_IMAGE);
	if (m_DisplayMode==DISPLAY_CHART)
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	else
		cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), m_ImageData);
	
	if (m_ChartData) {
		int count = cmpack_chart_data_count(m_ChartData);
		for (int row=0; row<count; row++) {
			int star_id = cmpack_chart_data_get_param(m_ChartData, row);
			const gchar *tag = m_Tags.Get(star_id);
			int i = m_Selection.IndexOf(star_id);
			if (i>=0) {
				int index = m_Selection.GetIndex(i);
				CmpackSelectionType type = m_Selection.GetType(i);
				gchar *buf = (gchar*)g_malloc((256+(tag ? strlen(tag)+1 : 0))*sizeof(gchar));
				if (index==1)
					strcpy(buf, Types[type].label);
				else
					sprintf(buf, "%s #%d", Types[type].label, index);
				if (tag) {
					strcat(buf, "\n");
					strcat(buf, tag);
				}
				cmpack_chart_data_set_tag(m_ChartData, row, buf);
				cmpack_chart_data_set_color(m_ChartData, row, Types[type].color);
				cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
				if (m_DisplayMode==DISPLAY_IMAGE)
					cmpack_chart_data_set_diameter(m_ChartData, row, 4.0);
				g_free(buf);
			} else if (tag) {
				cmpack_chart_data_set_tag(m_ChartData, row, tag);
				cmpack_chart_data_set_color(m_ChartData, row, CMPACK_COLOR_YELLOW);
				cmpack_chart_data_set_topmost(m_ChartData, row, TRUE);
				if (m_DisplayMode==DISPLAY_IMAGE)
					cmpack_chart_data_set_diameter(m_ChartData, row, 4.0);
			}
		}
	}
	cmpack_chart_view_set_orientation(CMPACK_CHART_VIEW(m_Chart), m_RowsUpward ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	cmpack_chart_view_set_negative(CMPACK_CHART_VIEW(m_Chart), m_Negative);
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), m_ChartData);
}

void CChartDlg::response_dialog(GtkWidget *pDlg, gint response_id, CChartDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CChartDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_NONE:
		SaveChart();
		return false;

	case GTK_RESPONSE_HELP:
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_CHART);
		return false;
	}
	return true;
}

void CChartDlg::button_clicked(GtkWidget *pButton, CChartDlg *pMe)
{
	pMe->OnButtonClicked(pButton);
}

void CChartDlg::OnButtonClicked(GtkWidget *pBtn)
{
	double zoom;

	if (pBtn==GTK_WIDGET(m_ShowChart)) {
		m_DisplayMode = DISPLAY_CHART;
		g_Project->SetInt("ChartDlg", "Mode", m_DisplayMode);
		UpdateImage();
		UpdateChart();
	} else
	if (pBtn==GTK_WIDGET(m_ShowImage)) {
		m_DisplayMode = DISPLAY_IMAGE;
		g_Project->SetInt("ChartDlg", "Mode", m_DisplayMode);
		UpdateImage();
		UpdateChart();
	} else
	if (pBtn==GTK_WIDGET(m_ShowMixed)) {
		m_DisplayMode = DISPLAY_FULL;
		g_Project->SetInt("ChartDlg", "Mode", m_DisplayMode);
		UpdateImage();
		UpdateChart();
	} else
	if (pBtn==GTK_WIDGET(m_ZoomIn)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_Chart));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_Chart), zoom + 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomOut)) {
		zoom = cmpack_chart_view_get_zoom(CMPACK_CHART_VIEW(m_Chart));
		cmpack_chart_view_set_zoom(CMPACK_CHART_VIEW(m_Chart), zoom - 5.0);
	} else 
	if (pBtn==GTK_WIDGET(m_ZoomFit)) {
		cmpack_chart_view_set_auto_zoom(CMPACK_CHART_VIEW(m_Chart), TRUE);
	}
}

void CChartDlg::SaveChart()
{
	if (m_ChartData) {
		CChartExportDlg dlg(GTK_WINDOW(m_pDlg));
		gchar *filename = g_Project->GetStr("LightCurve", "File", "lightcurve.txt");
		gchar *basename = g_path_get_basename(filename);
		gchar *newname = StripFileExtension(basename);
		if (m_DisplayMode==DISPLAY_CHART)
			dlg.Execute(m_ChartData, NULL, newname, m_Negative, m_RowsUpward);
		else
			dlg.Execute(m_ChartData, m_ImageData, newname, m_Negative, m_RowsUpward);
		g_free(basename);
		g_free(newname);
		g_free(filename);
	}
}
