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

#include "utils.h"
#include "main.h"
#include "ctxhelp.h"
#include "configuration.h"
#include "export_dlgs.h"
#include "export_options.h"

#define MIN_WIDTH			1
#define MIN_HEIGHT			1
#define MAX_WIDTH			8192
#define MAX_HEIGHT			8192

//-------------------------   SAVE CHART DIALOG   --------------------------------

//
// Constructor
//
CChartExportDlg::CChartExportDlg(GtkWindow *pParent):m_Parent(pParent)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export chart", GTK_WINDOW(pParent),
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
		GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Chart size widget
	m_Widget = new CExportChartOptions();
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), m_Widget->Handle());
	m_Widget->RegisterCallback(WidgetCallback, this);
}

//
// Destructor
//
CChartExportDlg::~CChartExportDlg()
{
	gtk_widget_destroy(m_pDlg);
	delete m_Widget;
}

//
// Execute the dialog
//
bool CChartExportDlg::Execute(CmpackChartData *chart, CmpackImageData *image, const gchar *name,
	bool invertImage, bool rowsUpwards)
{
	int source_width = 0, source_height = 0;
	if (chart) {
		source_width = cmpack_chart_data_width(chart);
		source_height = cmpack_chart_data_height(chart);
	} else if (image) {
		source_width = cmpack_image_data_width(image);
		source_height = cmpack_image_data_height(image);
	} else
		return false;

	gchar *type = g_Project->GetStr("ChartExport", "Type", DEFAULT_IMAGE_TYPE);
	m_Widget->Init(source_width, source_height,
		(CExportChartOptions::tSizeMode)g_Project->GetInt("ChartExport", "Mode", CExportChartOptions::RELATIVE, 0, 1),
		type,
		g_Project->GetDbl("ChartExport", "Zoom", 1.0),
		g_Project->GetInt("ChartExport", "Width", source_width, 1, 65535),
		g_Project->GetInt("ChartExport", "JpegQuality", 80, 0, 100));
	g_free(type);

	// Restore last folder and filename
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	// Set the default file name
	if (name) {
		gchar *basename = g_path_get_basename(name);
		gchar *newname = AddFileExtension(basename, ImageFileExtension(m_Widget->Type()));
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), newname);
		g_free(newname);
		g_free(basename);
	} else {
		gchar *newname = AddFileExtension("chart", ImageFileExtension(m_Widget->Type()));
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), newname);
		g_free(newname);
	}

	// Execute the dialog
	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	gchar *filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	int target_width = m_Widget->OutputWidth(), target_height = m_Widget->OutputHeight();
	int jpeg_quality = m_Widget->JpegQuality();
	const gchar *format = m_Widget->Type();
	if (!format)
		format = ImageTypeByExtension(filePath);

	// Save configuration
	g_Project->SetInt("ChartExport", "Mode", (int)m_Widget->Mode());
	g_Project->SetStr("ChartExport", "Type", m_Widget->Type());
	g_Project->SetDbl("ChartExport", "Zoom", m_Widget->Zoom());
	g_Project->SetInt("ChartExport", "Width", m_Widget->OutputWidth());
	g_Project->SetInt("ChartExport", "JpegQuality", jpeg_quality);
	folder = g_path_get_dirname(filePath);
	g_Project->SetStr("Output", "Folder", folder);
	g_free(folder);

	bool retval = false;
	CmpackOrientation orienation = (rowsUpwards ? CMPACK_ROWS_UPWARDS : CMPACK_ROWS_DOWNWARDS);
	if (image && !chart) {
		retval = cmpack_image_data_write_to_file(image, filePath, 
			format, target_width, target_height, jpeg_quality)!=0;
	} else
	if (chart && image) {
		retval = cmpack_chart_data_write_to_file(chart, image, filePath, 
			format, target_width, target_height, invertImage, orienation, jpeg_quality)!=0;
	} else 
	if (chart && !image) {
		retval = cmpack_chart_data_write_to_file(chart, NULL, filePath, 
			format, target_width, target_height, invertImage, orienation, jpeg_quality)!=0;
	}
	if (!retval) 
		ShowError(m_Parent, "Failed to write image to a file");
	g_free(filePath);
	return retval;
}

void CChartExportDlg::response_dialog(GtkWidget *pDlg, gint response_id, CChartExportDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CChartExportDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_ACCEPT:
		// Check values
		if (!CheckFormat()) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid file format.\n\n"
				"If you specify the file format by an extension, check that the file name contains a valid extension. "
				"Current version of the software supports the following formats: PNG and JPEG");
			return false;
		}
		if (m_Widget->OutputWidth()<MIN_WIDTH || m_Widget->OutputHeight()<MIN_HEIGHT) {
			ShowError(GTK_WINDOW(m_pDlg), "Invalid image size");
			return false;
		}
		if (m_Widget->OutputWidth()>MAX_WIDTH || m_Widget->OutputHeight()>MAX_HEIGHT) {
			ShowError(GTK_WINDOW(m_pDlg), "The image is too large.");
			return false;
		}
		break;

	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_CHART);
		return false;
	}
	return true;
}

void CChartExportDlg::WidgetCallback(CCBObject *sender, int message, int wparam, void* lparam, void* cb_data)
{
	CChartExportDlg *pMe = (CChartExportDlg*)cb_data;

	switch (message)
	{
	case CExportChartOptions::CB_TYPE_CHANGED:
		pMe->OnTypeChanged();
		break;
	}
}

void CChartExportDlg::OnTypeChanged(void)
{
	// Change file filters
	GSList *list = gtk_file_chooser_list_filters(GTK_FILE_CHOOSER(m_pDlg));
	for (GSList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(m_pDlg), (GtkFileFilter*)ptr->data);
	g_slist_free(list);
	GtkFileFilter *type_filter = ImageFileFilter(m_Widget->Type());
	if (type_filter)
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), type_filter);
	GtkFileFilter *all_files = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_files, "*");
	gtk_file_filter_set_name(all_files, "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), all_files);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), (type_filter ? type_filter : all_files));

	// Set file's extension
	const gchar *ext = ImageFileExtension(m_Widget->Type());
	if (ext) {
		gchar *oldname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		if (oldname) {
			gchar *newname = SetFileExtension(oldname, ext);
			gchar *basename = g_path_get_basename(newname);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), basename);
			g_free(basename);
			g_free(newname);
			g_free(oldname);
		}
	}
}

bool CChartExportDlg::CheckFormat(void)
{
	const gchar *type = m_Widget->Type();
	if (!type) {
		gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		if (fname) {
			type = ImageTypeByExtension(fname);
			g_free(fname);
		}
	}
	return (type!=NULL);
}

//---------------------------   GRAPH EXPORT DIALOG   ------------------------

CGraphExportDlg::CGraphExportDlg(GtkWindow *pParent):m_Parent(pParent)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export graph", GTK_WINDOW(pParent),
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
		GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	GtkFileFilter *type_filter = ImageFileFilter(DEFAULT_IMAGE_TYPE);
	if (type_filter)
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), type_filter);
	GtkFileFilter *all_files = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_files, "*");
	gtk_file_filter_set_name(all_files, "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), all_files);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), (type_filter ? type_filter : all_files));

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);
}

CGraphExportDlg::~CGraphExportDlg()
{
	gtk_widget_destroy(m_pDlg);
}

bool CGraphExportDlg::Execute(CmpackGraphView *widget, const gchar *name)
{
	// Restore last folder
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	// Set default file name
	if (name) {
		gchar *basename = g_path_get_basename(name);
		gchar *newname = AddFileExtension(basename, ImageFileExtension(DEFAULT_IMAGE_TYPE));
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), newname);
		g_free(newname);
		g_free(basename);
	} else {
		gchar *newname = AddFileExtension("graph", ImageFileExtension(DEFAULT_IMAGE_TYPE));
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), newname);
		g_free(newname);
	}

	// Execute the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	gchar *filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save configuration
	folder = g_path_get_dirname(filePath);
	g_Project->SetStr("Output", "Folder", folder);
	g_free(folder);

	// Make output file
	gboolean retval = cmpack_graph_view_write_to_file(widget, filePath, DEFAULT_IMAGE_TYPE);
	g_free(filePath);
	return retval!=0;
}

void CGraphExportDlg::response_dialog(GtkWidget *pDlg, gint response_id, CGraphExportDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CGraphExportDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_GRAPH);
		return false;
	}
	return true;
}

//---------------------------   EXPORT PHOTOMETRY FILE   ------------------------

static const struct {
	const gchar *Id, *Caption, *Extension, *MimeType;
	const gchar *FilterName, *FilterPattern;
} FileFormats[] = {
	{ "CSV",	"CSV (comma separated values)",		"csv",	"text/csv",		"CSV files",		"*.csv" },
	{ NULL }
};

static const gchar *FileExtension(gint type)
{
	if (type>=0)
		return FileFormats[type].Extension;
	return "";
}

static const gchar *FileMimeType(gint type)
{
	if (type>=0)
		return FileFormats[type].MimeType;
	return NULL;
}

static GtkFileFilter *FileFilter(gint type)
{
	if (type>=0 && FileFormats[type].FilterName && FileFormats[type].FilterPattern) {
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, FileFormats[type].FilterPattern);
		gtk_file_filter_set_name(filter, FileFormats[type].FilterName);
		return filter;
	}
	return NULL;
}

//
// Constructor
//
CFrameExportDlg::CFrameExportDlg(GtkWindow *pParent):m_Parent(pParent), 
	m_FileType(TYPE_CSV), m_Matched(false), m_Updating(false)
{
	memset(m_Options, 0, TYPE_N_ITEMS*sizeof(tOptions));

	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export photometry file", GTK_WINDOW(pParent),
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
		GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

	// Dialog icon
	gchar *icon = get_icon_file("muniwin");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Options
	GtkWidget *frame = gtk_frame_new("Export options");
	GtkWidget *hbox = gtk_hbox_new(TRUE, 8);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);

	// File format
	GtkWidget *lbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), lbox, TRUE, TRUE, 0);
	m_FileTypes = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	GtkWidget *label = gtk_label_new("File type");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(lbox), label, FALSE, TRUE, 0);
	m_TypeCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_FileTypes));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_TypeCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_TypeCombo), renderer, "text", 1);
	gtk_box_pack_start(GTK_BOX(lbox), m_TypeCombo, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_TypeCombo), "changed", G_CALLBACK(selection_changed), this);
	m_Header = gtk_check_button_new_with_label("Include column names");
	gtk_box_pack_start(GTK_BOX(lbox), m_Header, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_Header), "toggled", G_CALLBACK(button_toggled), this);
	m_SkipInvalid = gtk_check_button_new_with_label("Discard objects without valid measurement");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipInvalid, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipInvalid), "toggled", G_CALLBACK(button_toggled), this);
	m_SkipUnmatched = gtk_check_button_new_with_label("Discard objects that were not matched");
	gtk_box_pack_start(GTK_BOX(lbox), m_SkipUnmatched, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SkipUnmatched), "toggled", G_CALLBACK(button_toggled), this);
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CFrameExportDlg::~CFrameExportDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
}

bool CFrameExportDlg::Execute(const CPhot &file, bool matched, const gchar *name, int sort_column_id, GtkSortType sort_type)
{
	m_File.MakeCopy(file);
	m_File.SelectAperture(file.SelectedAperture());
	m_Matched = matched;

	m_Updating = true;

	// Restore last folder
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	// Set default file name
	gchar *filename = AddFileExtension(name, FileExtension(m_FileType));
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), filename);
	g_free(filename);

	// File types
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), NULL);
	gtk_list_store_clear(m_FileTypes);
	for (int i=0; FileFormats[i].Caption; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(m_FileTypes, &iter);
		gtk_list_store_set(m_FileTypes, &iter, 0, i, 1, FileFormats[i].Caption, -1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_TypeCombo), GTK_TREE_MODEL(m_FileTypes));
	SelectItem(GTK_COMBO_BOX(m_TypeCombo), m_FileType);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_TypeCombo))<0) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), 0);
		m_FileType = (tFileType)SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
	}
	gtk_widget_set_sensitive(m_TypeCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_FileTypes), NULL)>1);

	// Restore options (format dependent customizable options)
	m_Options[TYPE_CSV].skip_invalid = g_Project->GetBool("ExportFrameDlg", "CSV_SKIP_INVALID");
	m_Options[TYPE_CSV].skip_unmatched = m_Matched && g_Project->GetBool("ExportFrameDlg", "CSV_SKIP_UNMATCHED");
	m_Options[TYPE_CSV].header = g_Project->GetBool("ExportFrameDlg", "CSV_HEADER", true);

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) != GTK_RESPONSE_ACCEPT)
		return false;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	folder = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", folder);
	g_free(folder);

	// Save settings
	if (m_Matched)
		g_Project->SetBool("ExportFrameDlg", "CSV_SKIP_UNMATCHED", m_Options[TYPE_CSV].skip_unmatched);
	g_Project->SetBool("ExportFrameDlg", "CSV_SKIP_INVALID", m_Options[TYPE_CSV].skip_invalid);
	g_Project->SetBool("ExportFrameDlg", "CSV_HEADER", m_Options[TYPE_CSV].header);

	// Create output file
	int res = 0, flags = 0;
	GError *error = NULL;
	const tOptions *opt = &m_Options[m_FileType];
	if (opt->skip_invalid)
		flags |= CPhot::EXPORT_SKIP_INVALID;
	if (opt->skip_unmatched)
		flags |= CPhot::EXPORT_SKIP_UNMATCHED;
	if (!opt->header)
		flags |= CPhot::EXPORT_NO_HEADER;
	if (!m_File.ExportTable(filename, FileMimeType(m_FileType), flags, sort_column_id, sort_type, &error)) {
		if (error) {
			ShowError(m_Parent, error->message);
			g_error_free(error);
		}
		res = -1;
	}
	g_free(filename);
	return (res==0);
}

void CFrameExportDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_Header, TRUE);
	gtk_widget_set_sensitive(m_SkipInvalid, TRUE);
	gtk_widget_set_sensitive(m_SkipUnmatched, m_Matched);
}

void CFrameExportDlg::button_toggled(GtkToggleButton *pWidget, CFrameExportDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CFrameExportDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_SkipInvalid) {
		m_Options[m_FileType].skip_invalid = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_SkipUnmatched) {
		m_Options[m_FileType].skip_unmatched = gtk_toggle_button_get_active(pWidget)!=0;
	} else
	if (GTK_WIDGET(pWidget) == m_Header) {
		m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	}

	UpdateControls();
}

void CFrameExportDlg::selection_changed(GtkComboBox *pWidget, CFrameExportDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CFrameExportDlg::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		if (GTK_WIDGET(pWidget) == m_TypeCombo) {
			int ft = SelectedItem(pWidget);
			if (ft>=0 && ft!=m_FileType) {
				m_FileType = (tFileType)ft;
				OnTypeChanged();
			}
		}
	}
}

void CFrameExportDlg::OnTypeChanged(void)
{
	// Change file filters
	GSList *list = gtk_file_chooser_list_filters(GTK_FILE_CHOOSER(m_pDlg));
	for (GSList *ptr=list; ptr!=NULL; ptr=ptr->next) 
		gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(m_pDlg), (GtkFileFilter*)ptr->data);
	g_slist_free(list);
	GtkFileFilter *type_filter = FileFilter(m_FileType);
	if (type_filter)
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), type_filter);
	GtkFileFilter *all_files = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_files, "*");
	gtk_file_filter_set_name(all_files, "All files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(m_pDlg), all_files);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(m_pDlg), (type_filter ? type_filter : all_files));

	// Set file's extension
	const gchar *ext = FileExtension(m_FileType);
	if (ext) {
		gchar *oldname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));
		if (oldname) {
			gchar *newname = SetFileExtension(oldname, ext);
			gchar *basename = g_path_get_basename(newname);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDlg), basename);
			g_free(basename);
			g_free(newname);
			g_free(oldname);
		}
	}

	m_Updating = true;

	const tOptions *opt = m_Options+m_FileType;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Header), opt->header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipInvalid), opt->skip_invalid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SkipUnmatched), opt->skip_unmatched);

	m_Updating = false;

	UpdateControls();
}
