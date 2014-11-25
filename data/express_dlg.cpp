/**************************************************************

express_dlg.cpp (C-Munipack project)
The 'Express reduction' dialog
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

#include "configuration.h"
#include "profile.h"
#include "project.h"
#include "express_dlg.h"
#include "progress_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "timecorr_dlg.h"
#include "biascorr_dlg.h"
#include "darkcorr_dlg.h"
#include "flatcorr_dlg.h"
#include "matching_dlg.h"
#include "ctxhelp.h"

static const struct {
	int type;
	const gchar *caption;
	bool debug;
} RAWChannels[] = {
	{ CMPACK_CHANNEL_DEFAULT,	"Grayscale (R+G1+G2+B)" },
	{ CMPACK_CHANNEL_RED,		"Red" },
	{ CMPACK_CHANNEL_GREEN,		"Green (G1+G2)/2" },
	{ CMPACK_CHANNEL_BLUE,		"Blue" },
	{ CMPACK_CHANNEL_0,			"Channel #1", true },
	{ CMPACK_CHANNEL_1,			"Channel #2", true },
	{ CMPACK_CHANNEL_2,			"Channel #3", true },
	{ CMPACK_CHANNEL_3,			"Channel #4", true },
	{ -1 }
};

//-------------------------   MAIN WINDOW   --------------------------------

CExpressDlg::CExpressDlg(GtkWindow *pParent):m_pParent(pParent), m_ConvertBtn(NULL), m_ChannelCombo(NULL), 
	m_TimeCorrBtn(NULL), m_SecondsEdit(NULL), m_SecondsBtn(NULL), m_BiasCorrBtn(NULL), 
	m_BiasFrameEdit(NULL), m_BiasFrameBtn(NULL), m_DarkCorrBtn(NULL), m_DarkFrameEdit(NULL), 
	m_DarkFrameBtn(NULL), m_FlatCorrBtn(NULL), m_FlatFrameEdit(NULL), m_FlatFrameBtn(NULL), 
	m_PhotometryBtn(NULL), m_MatchingBtn(NULL), m_RefBtn(NULL), m_RefFrameCombo(NULL), 
	m_RefFrameBtn(NULL), m_CatBtn(NULL), m_CatFrameEdit(NULL), m_CatFrameBtn(NULL), m_TimeCorr(0), 
	m_Updating(false), m_Matching(false)
{
	int row = 0;
	tProjectType type = g_Project->ProjectType();
	GtkWidget *table;
	GSList *group;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	memset(&m_Params, 0, sizeof(tProcParams));

	m_Channels = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	for (int i=0; RAWChannels[i].caption; i++) {
		if (!RAWChannels[i].debug || type==PROJECT_TEST) {
			gtk_list_store_append(m_Channels, &iter);
			gtk_list_store_set(m_Channels, &iter, 0, RAWChannels[i].type, 1, RAWChannels[i].caption, -1);
		}
	}

	m_Frames = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Express reduction", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Start the process");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("reduction");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);
	
	// Dialog layout
	table = gtk_table_new(4, 3, FALSE);
	gtk_widget_set_size_request(table, 360, -1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(table), 8);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_table_set_row_spacing(GTK_TABLE(table), 2, 12);
	gtk_table_set_col_spacing(GTK_TABLE(table), 0, 16);

	// All frames / selected frames
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 3, row, row+1);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Process all frames in the current project");
	g_signal_connect(G_OBJECT(m_AllBtn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach_defaults(GTK_TABLE(table), m_AllBtn, 0, 3, row+1, row+2);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Process frames that are selected in the main window");
	g_signal_connect(G_OBJECT(m_SelBtn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach_defaults(GTK_TABLE(table), m_SelBtn, 0, 3, row+2, row+3);
	row += 3;

	// Actions
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Execute</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 3, row, row+1);
	row++;

	// Fetch / convert
	m_ConvertBtn = gtk_check_button_new_with_label("Fetch/convert files");
	gtk_widget_set_tooltip_text(m_ConvertBtn, "Start reduction with a fresh copy of the source frames");
	g_signal_connect(G_OBJECT(m_ConvertBtn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach_defaults(GTK_TABLE(table), m_ConvertBtn, 0, 3, row, row+1);
	m_ChannelCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_Channels));
	gtk_widget_set_tooltip_text(m_ChannelCombo, "Color to grayscale conversion mode");
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_ChannelCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_ChannelCombo), renderer, "text", 1);
	gtk_table_attach(GTK_TABLE(table), m_ChannelCombo, 1, 2, row+1, row+2, 
		(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
	row+=2;

	// Time correction
	if (type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
		m_TimeCorrBtn = gtk_check_button_new_with_label("Time correction");
		gtk_widget_set_tooltip_text(m_TimeCorrBtn, "Adjust the date and time of observation (time correction)");
		g_signal_connect(G_OBJECT(m_TimeCorrBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_TimeCorrBtn, 0, 3, row, row+1);
		m_SecondsEdit = gtk_entry_new();
		gtk_entry_set_editable(GTK_ENTRY(m_SecondsEdit), false);
		gtk_table_attach(GTK_TABLE(table), m_SecondsEdit, 1, 2, row+1, row+2, 
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
		m_SecondsBtn = gtk_button_new_with_label("More");
		gtk_widget_set_tooltip_text(m_SecondsBtn, "Specify time correction in a separate dialog");
		g_signal_connect(G_OBJECT(m_SecondsBtn), "clicked", G_CALLBACK(button_clicked), this);
		gtk_table_attach(GTK_TABLE(table), m_SecondsBtn, 2, 3, row+1, row+2, 
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
		row+=2;
	}

	// Bias correction
	if (g_Project->Profile()->GetBool(CProfile::ADVANCED_CALIBRATION)) {
		if (type==PROJECT_MASTER_DARK || type==PROJECT_MASTER_FLAT || type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
			m_BiasCorrBtn = gtk_check_button_new_with_label("Bias-frame correction");
			gtk_widget_set_tooltip_text(m_BiasCorrBtn, "Apply the bias correction");
			g_signal_connect(G_OBJECT(m_BiasCorrBtn), "toggled", G_CALLBACK(button_toggled), this);
			gtk_table_attach_defaults(GTK_TABLE(table), m_BiasCorrBtn, 0, 3, row, row+1);
			m_BiasFrameEdit = gtk_entry_new();
			gtk_widget_set_tooltip_text(m_BiasFrameEdit, "Path to the bias frame");
			gtk_table_attach(GTK_TABLE(table), m_BiasFrameEdit, 1, 2, row+1, row+2, 
				(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
			m_BiasFrameBtn = gtk_button_new_with_label("Browse");
			gtk_widget_set_tooltip_text(m_BiasCorrBtn, "Choose a bias frame in a separate dialog");
			g_signal_connect(G_OBJECT(m_BiasFrameBtn), "clicked", G_CALLBACK(button_clicked), this);
			gtk_table_attach(GTK_TABLE(table), m_BiasFrameBtn, 2, 3, row+1, row+2, 
				(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
			row+=2;
		}
	}

	// Dark correction
	if (type==PROJECT_MASTER_FLAT || type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
		m_DarkCorrBtn = gtk_check_button_new_with_label("Dark-frame correction");
		gtk_widget_set_tooltip_text(m_DarkCorrBtn, "Apply the dark correction");
		g_signal_connect(G_OBJECT(m_DarkCorrBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_DarkCorrBtn, 0, 3, row, row+1);
		m_DarkFrameEdit = gtk_entry_new();
		gtk_widget_set_tooltip_text(m_DarkFrameEdit, "Path to the dark frame");
		gtk_table_attach(GTK_TABLE(table), m_DarkFrameEdit, 1, 2, row+1, row+2, 
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		m_DarkFrameBtn = gtk_button_new_with_label("Browse");
		gtk_widget_set_tooltip_text(m_DarkFrameBtn, "Choose a dark frame in a separate dialog");
		g_signal_connect(G_OBJECT(m_DarkFrameBtn), "clicked", G_CALLBACK(button_clicked), this);
		gtk_table_attach(GTK_TABLE(table), m_DarkFrameBtn, 2, 3, row+1, row+2, 
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
		row+=2;
	}

	// Flat correction
	if (type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
		m_FlatCorrBtn = gtk_check_button_new_with_label("Flat-frame correction");
		gtk_widget_set_tooltip_text(m_FlatCorrBtn, "Apply the flat correction");
		g_signal_connect(G_OBJECT(m_FlatCorrBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_FlatCorrBtn, 0, 3, row, row+1);
		m_FlatFrameEdit = gtk_entry_new();
		gtk_widget_set_tooltip_text(m_FlatFrameEdit, "Path to the flat frame");
		gtk_table_attach(GTK_TABLE(table), m_FlatFrameEdit, 1, 2, row+1, row+2, 
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		m_FlatFrameBtn = gtk_button_new_with_label("Browse");
		gtk_widget_set_tooltip_text(m_FlatFrameBtn, "Choose a flat frame in a separate dialog");
		g_signal_connect(G_OBJECT(m_FlatFrameBtn), "clicked", G_CALLBACK(button_clicked), this);
		gtk_table_attach(GTK_TABLE(table), m_FlatFrameBtn, 2, 3, row+1, row+2,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
		row+=2;
	}

	// Photometry
	if (type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
		m_PhotometryBtn = gtk_check_button_new_with_label("Photometry");
		gtk_widget_set_tooltip_text(m_PhotometryBtn, "Run photometry");
		g_signal_connect(G_OBJECT(m_PhotometryBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_PhotometryBtn, 0, 3, row, row+1);
		row++;
	}

	// Matching
	if (type==PROJECT_REDUCE || type==PROJECT_COMBINING || type==PROJECT_TEST) {
		m_MatchingBtn = gtk_check_button_new_with_label("Matching");
		gtk_widget_set_tooltip_text(m_MatchingBtn, "Run matching");
		g_signal_connect(G_OBJECT(m_MatchingBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_MatchingBtn, 0, 3, row, row+1);

		m_RefBtn = gtk_radio_button_new_with_label(NULL, "using reference frame");
		gtk_widget_set_tooltip_text(m_RefBtn, "Use one of the source frames as a reference frame");
		g_signal_connect(G_OBJECT(m_RefBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_RefBtn, 1, 3, row+1, row+2);

		m_RefFrameCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_Frames));
		gtk_widget_set_tooltip_text(m_RefFrameCombo, "A frame that shall be used as a reference frame");
		renderer = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_RefFrameCombo), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_RefFrameCombo), renderer, "text", 1);
		gtk_table_attach(GTK_TABLE(table), m_RefFrameCombo, 1, 2, row+2, row+3, 
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		m_RefFrameBtn = gtk_button_new_with_label("Browse");
		gtk_widget_set_tooltip_text(m_RefFrameBtn, "Choose a reference frame in a separate dialog");
		g_signal_connect(G_OBJECT(m_RefFrameBtn), "clicked", G_CALLBACK(button_clicked), this);
		gtk_table_attach(GTK_TABLE(table), m_RefFrameBtn, 2, 3, row+2, row+3, 
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);

		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_RefBtn));
		m_CatBtn = gtk_radio_button_new_with_label(group, "using catalog file");
		gtk_widget_set_tooltip_text(m_CatBtn, "Use a catalog file as a reference frame");
		g_signal_connect(G_OBJECT(m_CatBtn), "toggled", G_CALLBACK(button_toggled), this);
		gtk_table_attach_defaults(GTK_TABLE(table), m_CatBtn, 1, 3, row+3, row+4);

		m_CatFrameEdit = gtk_entry_new();
		gtk_widget_set_tooltip_text(m_CatFrameEdit, "Path to the catalog file that shall be used as a reference frame");
		gtk_table_attach(GTK_TABLE(table), m_CatFrameEdit, 1, 2, row+4, row+5, 
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		m_CatFrameBtn = gtk_button_new_with_label("Browse");
		gtk_widget_set_tooltip_text(m_CatFrameBtn, "Choose a catalog file in a separate dialog");
		g_signal_connect(G_OBJECT(m_CatFrameBtn), "clicked", G_CALLBACK(button_clicked), this);
		gtk_table_attach(GTK_TABLE(table), m_CatFrameBtn, 2, 3, row+4, row+5, 
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), GTK_FILL, 0, 0);
		row+=5;
	}

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CExpressDlg::~CExpressDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_Frames);
	g_object_unref(m_Channels);
}

void CExpressDlg::button_toggled(GtkToggleButton *widget, CExpressDlg *pMe)
{
	pMe->OnButtonToggled(widget);
}

void CExpressDlg::OnButtonToggled(GtkToggleButton *widget)
{
	if (!m_Updating) {
		m_Updating = true;
		if (widget == GTK_TOGGLE_BUTTON(m_MatchingBtn)) 
			m_Matching = gtk_toggle_button_get_active(widget)!=0;
		else if (widget == GTK_TOGGLE_BUTTON(m_SelBtn)) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SelBtn))) {
				m_Selected = true;
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchingBtn), false);
			}
		} else if (widget == GTK_TOGGLE_BUTTON(m_AllBtn)) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))) {
				m_Selected = false;
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchingBtn), m_Matching);
			}
		}
		m_Updating = false;
		UpdateControls();
	}
}

void CExpressDlg::button_clicked(GtkButton *widget, CExpressDlg *pMe)
{
	pMe->OnButtonClicked(widget);
}

void CExpressDlg::OnButtonClicked(GtkButton *widget)
{
	if (widget == GTK_BUTTON(m_SecondsBtn))
		EditTimeCorrection();
	else if (widget == GTK_BUTTON(m_BiasFrameBtn))
		ChooseBiasFrame();
	else if (widget == GTK_BUTTON(m_DarkFrameBtn))
		ChooseDarkFrame();
	else if (widget == GTK_BUTTON(m_FlatFrameBtn))
		ChooseFlatFrame();
	else if (widget == GTK_BUTTON(m_RefFrameBtn))
		ChooseReferenceFrame();
	else if (widget == GTK_BUTTON(m_CatFrameBtn))
		ChooseCatalogFile();
}

gboolean CExpressDlg::foreach_all_files(GtkTreeModel *model, GtkTreePath *path, 
										GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
	return FALSE;
}

void CExpressDlg::foreach_sel_files(GtkTreeModel *model, GtkTreePath *path, 
							   GtkTreeIter *iter, gpointer userdata)
{
	GList **list = ((GList**)userdata);
	GtkTreeRowReference *rowref = gtk_tree_row_reference_new(model, path);
	*list = g_list_append(*list, rowref);
}

void CExpressDlg::UpdateControls(void)
{
	bool ok;

	if (m_ConvertBtn && m_ChannelCombo) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ConvertBtn))!=0;
		gtk_widget_set_sensitive(m_ChannelCombo, ok);
	}
	if (m_TimeCorrBtn && m_SecondsBtn && m_SecondsEdit) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_TimeCorrBtn))!=0;
		gtk_widget_set_sensitive(m_SecondsEdit, ok);
		gtk_widget_set_sensitive(m_SecondsBtn, ok);
	}
	if (m_BiasCorrBtn && m_BiasFrameEdit && m_BiasFrameBtn) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BiasCorrBtn))!=0;
		gtk_widget_set_sensitive(m_BiasFrameEdit, ok);
		gtk_widget_set_sensitive(m_BiasFrameBtn, ok);
	}
	if (m_DarkCorrBtn && m_DarkFrameEdit && m_DarkFrameBtn) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_DarkCorrBtn))!=0;
		gtk_widget_set_sensitive(m_DarkFrameEdit, ok);
		gtk_widget_set_sensitive(m_DarkFrameBtn, ok);
	}
	if (m_FlatCorrBtn && m_FlatFrameEdit && m_FlatFrameBtn) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FlatCorrBtn))!=0;
		gtk_widget_set_sensitive(m_FlatFrameEdit, ok);
		gtk_widget_set_sensitive(m_FlatFrameBtn, ok);
	}
	if (m_MatchingBtn && m_RefBtn && m_CatBtn) {
		ok = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AllBtn))!=0;
		gtk_widget_set_sensitive(m_MatchingBtn, ok);
		ok = ok && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MatchingBtn))!=0;
		gtk_widget_set_sensitive(m_RefBtn, ok);
		gtk_widget_set_sensitive(m_CatBtn, ok);
		if (m_RefFrameCombo && m_RefFrameBtn) {
			bool ok2 = ok && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RefBtn))!=0;
			gtk_widget_set_sensitive(m_RefFrameCombo, ok2);
			gtk_widget_set_sensitive(m_RefFrameBtn, ok2);
		}
		if (m_CatFrameEdit && m_CatFrameBtn) {
			bool ok2 = ok && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_CatBtn))!=0;
			gtk_widget_set_sensitive(m_CatFrameEdit, ok2);
			gtk_widget_set_sensitive(m_CatFrameBtn, ok2);
		}
	}
}

void CExpressDlg::Execute()
{
	int res, id;
	char msg[256], buf[256];
	GtkTreeModel *pList = g_Project->FileList();
	GtkTreeIter iter, iter2;

	m_Updating = true;

	// Default state
	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_SelBtn, gtk_tree_selection_count_selected_rows(pSel)>0);
	m_Selected = gtk_tree_selection_count_selected_rows(pSel)>1;
	if (m_Selected)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), true);

	// Restore last settings
	if (m_ConvertBtn) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ConvertBtn), 
			g_Project->GetBool("ExpressReduction", "Convert", true));
		CmpackChannel channel = (CmpackChannel)g_Project->GetInt("Convert", "ColorChannel", CMPACK_CHANNEL_DEFAULT);
		SelectChannel(channel);
	}
	if (m_TimeCorrBtn) {
		m_TimeCorr = g_Project->GetDbl("TimeCorr", "Seconds", 0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_TimeCorrBtn), 
			g_Project->GetBool("ExpressReduction", "TimeCorr", false));
		UpdateTimeCorrection();
	}
	if (m_BiasCorrBtn && m_BiasFrameEdit) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_BiasCorrBtn), 
			g_Project->GetBool("ExpressReduction", "BiasCorr", false));
		char *bias = g_Project->GetStr("BiasCorr", "File", NULL);
		if (bias)
			gtk_entry_set_text(GTK_ENTRY(m_BiasFrameEdit), bias);
		else
			gtk_entry_set_text(GTK_ENTRY(m_BiasFrameEdit), "");
		g_free(bias);
	}
	if (m_DarkCorrBtn && m_DarkFrameEdit) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_DarkCorrBtn), 
			g_Project->GetBool("ExpressReduction", "DarkCorr", false));
		char *dark = g_Project->GetStr("DarkCorr", "File", NULL);
		if (dark)
			gtk_entry_set_text(GTK_ENTRY(m_DarkFrameEdit), dark);
		else
			gtk_entry_set_text(GTK_ENTRY(m_DarkFrameEdit), "");
		g_free(dark);
	}
	if (m_FlatCorrBtn && m_FlatFrameEdit) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FlatCorrBtn), 
			g_Project->GetBool("ExpressReduction", "FlatCorr", false));
		char *flat = g_Project->GetStr("FlatCorr", "File", NULL);
		if (flat)
			gtk_entry_set_text(GTK_ENTRY(m_FlatFrameEdit), flat);
		else
			gtk_entry_set_text(GTK_ENTRY(m_FlatFrameEdit), "");
		g_free(flat);
	}
	if (m_PhotometryBtn) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_PhotometryBtn), 
			g_Project->GetBool("ExpressReduction", "Photometry", false));
	}
	if (m_MatchingBtn && m_RefBtn && m_CatBtn) {
		m_Matching = g_Project->GetBool("ExpressReduction", "Matching", false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchingBtn), m_Matching && !m_Selected);
		if (g_Project->GetInt("MatchingDlg", "Select", 0)==0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RefBtn), true);
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_CatBtn), true);
	}
	if (m_RefFrameCombo) {
		gtk_combo_box_set_model(GTK_COMBO_BOX(m_RefFrameCombo), NULL);
		gtk_list_store_clear(m_Frames);
		gboolean ok = gtk_tree_model_get_iter_first(pList, &iter);
		while (ok) {
			gtk_tree_model_get(GTK_TREE_MODEL(pList), &iter, FRAME_ID, &id, -1);
			gtk_list_store_append(m_Frames, &iter2);
			sprintf(buf, "Frame #%d", id);
			gtk_list_store_set(m_Frames, &iter2, 0, id, 1, buf, -1);
			ok = gtk_tree_model_iter_next(pList, &iter);
		}
		gtk_combo_box_set_model(GTK_COMBO_BOX(m_RefFrameCombo), GTK_TREE_MODEL(m_Frames));
		SelectRefFrame(g_Project->GetInt("MatchingDlg", "Frame", 0));
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_RefFrameCombo))<0) {
			GtkTreeIter iter3;
			if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Frames), &iter3))
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_RefFrameCombo), &iter3);
		}
	}
	if (m_CatFrameEdit) {
		char *path = g_Project->GetStr("MatchingDlg", "File", NULL);
		if (path)
			gtk_entry_set_text(GTK_ENTRY(m_CatFrameEdit), path);
		else
			gtk_entry_set_text(GTK_ENTRY(m_CatFrameEdit), "");
		g_free(path);
	}
	m_Updating = false;

	UpdateControls();

	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return;
	gtk_widget_hide(m_pDlg);

	// Save parameters
	memset(&m_Params, 0, sizeof(tProcParams));
	if (m_ConvertBtn) {
		m_Params.Convert = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ConvertBtn))!=0;
		g_Project->SetBool("ExpressReduction", "Convert", m_Params.Convert);
		g_Project->SetInt("Convert", "ColorChannel", SelectedChannel());
	}
	if (m_TimeCorrBtn) {
		m_Params.TimeCorr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_TimeCorrBtn))!=0;
		g_Project->SetBool("ExpressReduction", "TimeCorr", m_Params.TimeCorr);
		m_Params.Seconds = m_TimeCorr;
		g_Project->SetDbl("TimeCorr", "Seconds", m_Params.Seconds);
	}
	if (m_BiasCorrBtn && m_BiasFrameEdit) {
		m_Params.BiasCorr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BiasCorrBtn))!=0;
		g_Project->SetBool("ExpressReduction", "BiasCorr", m_Params.BiasCorr);
		m_Params.BiasFrame = gtk_entry_get_text(GTK_ENTRY(m_BiasFrameEdit));
		g_Project->SetStr("BiasCorr", "File", m_Params.BiasFrame);
		CConfig::SetStr("BiasCorr", "File", m_Params.BiasFrame);
	}
	if (m_DarkCorrBtn && m_DarkFrameEdit) {
		m_Params.DarkCorr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_DarkCorrBtn))!=0;
		g_Project->SetBool("ExpressReduction", "DarkCorr", m_Params.DarkCorr);
		m_Params.DarkFrame = gtk_entry_get_text(GTK_ENTRY(m_DarkFrameEdit));
		g_Project->SetStr("DarkCorr", "File", m_Params.DarkFrame);
		CConfig::SetStr("DarkCorr", "File", m_Params.DarkFrame);
	}
	if (m_FlatCorrBtn && m_FlatFrameEdit) {
		m_Params.FlatCorr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FlatCorrBtn))!=0;
		g_Project->SetBool("ExpressReduction", "FlatCorr", m_Params.FlatCorr);
		m_Params.FlatFrame = gtk_entry_get_text(GTK_ENTRY(m_FlatFrameEdit));
		g_Project->SetStr("FlatCorr", "File", m_Params.FlatFrame);
		CConfig::SetStr("FlatCorr", "File", m_Params.FlatFrame);
	}
	if (m_PhotometryBtn) {
		m_Params.Photometry = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_PhotometryBtn))!=0;
		g_Project->SetBool("ExpressReduction", "Photometry", m_Params.Photometry);
	}
	if (m_MatchingBtn && m_RefBtn) {
		m_Params.Matching = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MatchingBtn))!=0;
		g_Project->SetBool("ExpressReduction", "Matching", m_Matching);
		m_Params.UseRef = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RefBtn))!=0;
		g_Project->SetInt("MatchingDlg", "Select", (m_Params.UseRef ? 0 : 1));
	}
	if (m_RefFrameCombo) {
		m_Params.RefFrame = SelectedRefFrame();
		g_Project->SetInt("MatchingDlg", "Frame", m_Params.RefFrame);
	}
	if (m_CatFrameEdit) {
		m_Params.CatFile = gtk_entry_get_text(GTK_ENTRY(m_CatFrameEdit));
		g_Project->SetStr("MatchingDlg", "File", m_Params.CatFile);
		CConfig::SetStr("MatchingDlg", "File", m_Params.CatFile);
	}
	
	// Make list of unprocessed frames
	if (!m_Selected) {
		// All files
		GtkTreeModel *pList = g_Project->FileList();
		if (gtk_tree_model_iter_n_children(pList, NULL)>0) 
			gtk_tree_model_foreach(pList, foreach_all_files, &m_Params.FileList);
		else
			ShowError(m_pParent, "There are no files in the project.");
	} else {
		// Selected files
		GtkTreeSelection *pSel = g_MainWnd->GetSelection();
		if (gtk_tree_selection_count_selected_rows(pSel)>0) 
			gtk_tree_selection_selected_foreach(pSel, foreach_sel_files, &m_Params.FileList);
		else
			ShowError(m_pParent, "There are no selected files.");
	}

	if (m_Params.FileList) {
		if (m_Params.Convert && !m_Selected) {
			g_Project->ClearThumbnails();
			g_Project->ClearTempFiles();
			g_Project->ClearCorrections();
			g_Project->ClearReference();
			g_Project->ClearObject();
		}
		if (m_Params.Matching)
			g_Project->ClearReference();
		CProgressDlg pDlg(m_pParent, "Processing files");
		pDlg.SetMinMax(0, g_list_length(m_Params.FileList));
		res = pDlg.Execute(ExecuteProc, this);
		if (res!=0) {
			char *msg = cmpack_formaterror(res);
			ShowError(m_pParent, msg, true);
			cmpack_free(msg);
		} else
		if (m_Params.OutFiles==0) {
			ShowError(m_pParent, "No file was successfully processed.", true);
		} else 
		if (m_Params.OutFiles!=m_Params.InFiles) {
			sprintf(msg, "%d file(s) were successfully processed, %d file(s) failed.", 
				m_Params.OutFiles, m_Params.InFiles-m_Params.OutFiles);
			ShowWarning(m_pParent, msg, true);
		} else {
			sprintf(msg, "All %d file(s) were successfully processed.", m_Params.OutFiles);
			ShowInformation(m_pParent, msg, true);
		}
		g_list_foreach(m_Params.FileList, (GFunc)gtk_tree_row_reference_free, NULL);
		g_list_free(m_Params.FileList);
	}
	g_Project->Save();
}

void CExpressDlg::response_dialog(GtkDialog *pDlg, gint response_id, CExpressDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExpressDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		if (m_BiasCorrBtn && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_BiasCorrBtn))) {
			const char *path = gtk_entry_get_text(GTK_ENTRY(m_BiasFrameEdit));
			if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
				ShowError(GTK_WINDOW(m_pDlg), "Specified bias-frame file doesn't exist.");
				return false;
			}
		}
		if (m_DarkCorrBtn && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_DarkCorrBtn))) {
			const char *path = gtk_entry_get_text(GTK_ENTRY(m_DarkFrameEdit));
			if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
				ShowError(GTK_WINDOW(m_pDlg), "Specified dark-frame file doesn't exist.");
				return false;
			}
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FlatCorrBtn))) {
			const char *path = gtk_entry_get_text(GTK_ENTRY(m_FlatFrameEdit));
			if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
				ShowError(GTK_WINDOW(m_pDlg), "Specified flat-frame file doesn't exist.");
				return false;
			}
		}
		if (m_MatchingBtn && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MatchingBtn))) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RefBtn))) {
				if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_RefFrameCombo))<0) {
					ShowError(GTK_WINDOW(m_pDlg), "Please, select a reference frame.");
					return false;
				}
			} else {
				const char *path = gtk_entry_get_text(GTK_ENTRY(m_CatFrameEdit));
				if (!g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
					ShowError(GTK_WINDOW(m_pDlg), "Specified catalog file doesn't exist.");
					return false;
				}
			}
		}
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_EXPRESS_REDUCTION);
		return false;
	}
	return true;
}

int CExpressDlg::ExecuteProc(CProgressDlg *sender, void *userdata)
{
	return ((CExpressDlg*)userdata)->ProcessFiles(sender);
}

int CExpressDlg::ProcessFiles(CProgressDlg *sender)
{
	int res, file_res, frameid;
	char msg[128];
	gchar *fpath, *fbase;
	GtkTreePath *path;
	CConvertProc konv;
	CTimeCorrProc tcor;
	CBiasCorrProc bias;
	CDarkCorrProc dark;
	CFlatCorrProc flat;
	CPhotometryProc phot;
	CMatchingProc match;

	m_Params.InFiles = m_Params.OutFiles = 0;
	sender->Print("------ Express reduction ------");

	// Initialization
	res = 0;
	if (m_Params.Convert) 
		res = konv.Init(sender);
	if (res==0 && m_Params.TimeCorr) {
		res = tcor.Init(sender, m_Params.Seconds, false);
		if (res==0)
			g_Project->SetDbl("TimeCorr", "Seconds", m_Params.Seconds);
	}
	if (res==0 && m_Params.BiasCorr) {
		res = bias.Init(sender, m_Params.BiasFrame);
		if (res==0)
			g_Project->SetOrigBiasFile(m_Params.BiasFrame);
	}
	if (res==0 && m_Params.DarkCorr) {
		res = dark.Init(sender, m_Params.DarkFrame);
		if (res==0)
			g_Project->SetOrigDarkFile(m_Params.DarkFrame);
	}
	if (res==0 && m_Params.FlatCorr) {
		res = flat.Init(sender, m_Params.FlatFrame);
		if (res==0)
			g_Project->SetOrigFlatFile(m_Params.FlatFrame);
	}
	if (res==0 && m_Params.Photometry) 
		res = phot.Init(sender);
	if (res==0 && m_Params.Matching) {
		if (m_Params.UseRef) {
			// If matching is requested, we have to process the reference frame first
			GtkTreePath *refpath = g_Project->GetFilePath(m_Params.RefFrame);
			sender->SetProgress(m_Params.InFiles++);
			fpath = g_Project->GetSourceFile(refpath);
			fbase = g_filename_display_basename(fpath);
			sender->SetFileName(fbase);
			g_free(fbase);
			g_free(fpath);
			sprintf(msg, "Frame #%d:", m_Params.RefFrame);
			sender->Print(msg);
			file_res = 0;
			if (file_res==0 && m_Params.Convert)
				file_res = konv.Execute(refpath);
			if (file_res==0 && m_Params.TimeCorr)
				file_res = tcor.Execute(refpath);
			if (file_res==0 && m_Params.BiasCorr)
				file_res = bias.Execute(refpath);
			if (file_res==0 && m_Params.DarkCorr)
				file_res = dark.Execute(refpath);
			if (file_res==0 && m_Params.FlatCorr)
				file_res = flat.Execute(refpath);
			if (file_res==0 && m_Params.Photometry)
				file_res = phot.Execute(refpath);
			if (file_res==0) {
				res = match.InitWithReferenceFrame(sender, refpath);
				if (res==0)
					file_res = match.Execute(refpath);
			}
			gtk_tree_path_free(refpath);
			if (res!=0)
				return res;
			if (file_res!=0) 
				return file_res;
			m_Params.OutFiles++;
		} else {
			// Initialize with a catalog file 
			res = match.InitWithCatalogFile(sender, m_Params.CatFile);
		}
	}
	if (res!=0) {
		// Initialization failed
		return res;
	}

	// Process files
	for (GList *node=m_Params.FileList; node!=NULL; node = node->next) {
		path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
		if (path) {
			if (!g_Project->IsReferenceFrame(path) || !m_Params.Matching || !m_Params.UseRef) {
				frameid = g_Project->GetFrameID(path);
				fpath = g_Project->GetSourceFile(path);
				if (g_Project->IsReferenceFrame(path))
					g_Project->ClearReference();
				fbase = g_filename_display_basename(fpath);
				sender->SetFileName(fbase);
				g_free(fbase);
				g_free(fpath);
				sprintf(msg, "Frame #%d:", frameid);
				sender->Print(msg);
				sender->SetProgress(m_Params.InFiles++);
				file_res = 0;
				if (file_res==0 && m_Params.Convert)
					file_res = konv.Execute(path);
				if (file_res==0 && m_Params.TimeCorr)
					file_res = tcor.Execute(path);
				if (file_res==0 && m_Params.BiasCorr)
					file_res = bias.Execute(path);
				if (file_res==0 && m_Params.DarkCorr)
					file_res = dark.Execute(path);
				if (file_res==0 && m_Params.FlatCorr)
					file_res = flat.Execute(path);
				if (file_res==0 && m_Params.Photometry)
					file_res = phot.Execute(path);
				if (file_res==0 && m_Params.Matching)
					file_res = match.Execute(path);
				if (file_res==0)
					m_Params.OutFiles++;
			}
			gtk_tree_path_free(path);
		}
		if (sender->Cancelled()) 
			break;
	}

	// Finished
	return res;
}

void CExpressDlg::EditTimeCorrection(void)
{
	CTimeCorrDlg dlg(GTK_WINDOW(m_pDlg));

	if (dlg.EditParams(&m_TimeCorr))
		UpdateTimeCorrection();
}

void CExpressDlg::UpdateTimeCorrection(void)
{
	double days, seconds;
	char msg[256];

	if (!m_SecondsEdit)
		return;

	seconds = modf(fabs(m_TimeCorr/(24*3600)), &days)*(24*3600);
	if (days==0) {
		if (fmod(seconds, 1.0)<0.001)
			sprintf(msg, "%.0f seconds", seconds);
		else
			sprintf(msg, "%.3f seconds", seconds);
	} else {
		if (seconds < 0.001) 
			sprintf(msg, "%.0f days", days);
		else if (fmod(seconds, 1.0)<0.001)
			sprintf(msg, "%.0f days %.0f seconds", days, seconds);
		else
			sprintf(msg, "%.0f days %.3f seconds", days, seconds);
	}
	if (m_TimeCorr<0)
		strcat(msg, " to past");
	else if (m_TimeCorr>0)
		strcat(msg, " to future");

	gtk_entry_set_text(GTK_ENTRY(m_SecondsEdit), msg);
}

void CExpressDlg::ChooseBiasFrame(void)
{
	CBiasCorrDlg dlg(GTK_WINDOW(m_pDlg));

	char *path = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_BiasFrameEdit)));
	if (dlg.SelectFrame(&path)) 
		gtk_entry_set_text(GTK_ENTRY(m_BiasFrameEdit), path);
	g_free(path);
}

void CExpressDlg::ChooseDarkFrame(void)
{
	CDarkCorrDlg dlg(GTK_WINDOW(m_pDlg));

	char *path = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_DarkFrameEdit)));
	if (dlg.SelectFrame(&path)) 
		gtk_entry_set_text(GTK_ENTRY(m_DarkFrameEdit), path);
	g_free(path);
}

void CExpressDlg::ChooseFlatFrame(void)
{
	CFlatCorrDlg dlg(GTK_WINDOW(m_pDlg));

	char *path = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_FlatFrameEdit)));
	if (dlg.SelectFrame(&path)) 
		gtk_entry_set_text(GTK_ENTRY(m_FlatFrameEdit), path);
	g_free(path);
}

int CExpressDlg::SelectedRefFrame(void)
{
	if (m_RefFrameCombo)
		return SelectedItem(GTK_COMBO_BOX(m_RefFrameCombo), -1);
	return -1;
}

void CExpressDlg::SelectRefFrame(int frame)
{
	int id;
	GtkTreeIter iter;

	if (!m_RefFrameCombo)
		return;

	gboolean ok = gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_Frames), &iter, NULL, 0);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(m_Frames), &iter, 0, &id, -1);
		if (id==frame) {
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_RefFrameCombo), &iter);
			break;
		}
		ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_Frames), &iter);
	}
}

void CExpressDlg::SelectChannel(CmpackChannel channel)
{
	int id;
	GtkTreeIter iter;

	if (!m_ChannelCombo)
		return;

	gboolean ok = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_Channels), &iter);
	while (ok) {
		gtk_tree_model_get(GTK_TREE_MODEL(m_Channels), &iter, 0, &id, -1);
		if (id==channel) {
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_ChannelCombo), &iter);
			break;
		}
		ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_Channels), &iter);
	}
	if (!ok && gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_Channels), NULL)>0)
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_ChannelCombo), 0);
}

CmpackChannel CExpressDlg::SelectedChannel(void)
{
	if (m_ChannelCombo)
		return (CmpackChannel)SelectedItem(GTK_COMBO_BOX(m_ChannelCombo), CMPACK_CHANNEL_DEFAULT);
	return CMPACK_CHANNEL_DEFAULT;
}

void CExpressDlg::ChooseReferenceFrame(void)
{
	CMatchingDlg dlg(GTK_WINDOW(m_pDlg));

	int frame = SelectedRefFrame();
	if (dlg.SelectFrame(&frame)) 
		SelectRefFrame(frame);
}

void CExpressDlg::ChooseCatalogFile(void)
{
	CMatchingDlg dlg(GTK_WINDOW(m_pDlg));

	char *path = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_CatFrameEdit)));
	if (dlg.SelectFile(&path)) 
		gtk_entry_set_text(GTK_ENTRY(m_CatFrameEdit), path);
	g_free(path);
}
