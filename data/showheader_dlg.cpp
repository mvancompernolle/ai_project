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

#include "showheader_dlg.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"

const static struct {
	const char *caption;		// Column name
} TreeViewColumns[] = {
	{ "Name" },
	{ "Value" },
	{ "Comment" },
	{ NULL }
};

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

CShowHeaderDlg::CShowHeaderDlg(GtkWindow *pParent):m_FileType(UNDEFINED), m_Name(NULL),
	m_CCDFile(NULL), m_CatFile(NULL), m_PhtFile(NULL), m_Table(NULL)
{
	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));

	// Make tree store
	m_Data = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	// Make tree view
	m_View = gtk_tree_view_new();
	for (int i=0; TreeViewColumns[i].caption!=NULL; i++) {
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		// Set column name and alignment
		gtk_tree_view_column_set_title(col, TreeViewColumns[i].caption);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_View), col);
		// Add text renderer
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		gtk_tree_view_column_add_attribute(col, renderer, "text", i);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_View), GTK_TREE_MODEL(m_Data));
	}
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(m_View), TRUE);

	// Scrolled window
	m_ScrWnd = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_ScrWnd),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m_ScrWnd), 
		GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(m_ScrWnd), m_View);
	gtk_widget_set_size_request(m_ScrWnd, 480, 320);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), m_ScrWnd);

	// Separator
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), gtk_label_new(NULL));

	// Buttons
	GtkWidget *bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), bbox);
	m_SaveBtn = gtk_button_new_with_label("Save header");
	gtk_box_pack_start(GTK_BOX(bbox), m_SaveBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_SaveBtn), "clicked", G_CALLBACK(button_clicked), this);

	// Separator
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pDlg)->vbox), gtk_label_new(NULL));

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CShowHeaderDlg::~CShowHeaderDlg()
{
	gtk_widget_destroy(m_pDlg);
	g_object_unref(m_Data);
	g_free(m_Name);
}

void CShowHeaderDlg::Execute(const CCCDFile *img, const gchar *name)
{
	gchar *key, *val, *com;
	
	if (!img->Valid())
		return;

	m_CCDFile = img;
	m_FileType = CCDFILE;
	BeginUpdate();
	SetCaption(name);
	for (int i=0; img->GetParam(i, &key, &val, &com); i++) {
		Add(key, val, com);
		g_free(key);
		g_free(val);
		g_free(com);
	}
	EndUpdate();

	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CShowHeaderDlg::Execute(const CPhot *pht, const gchar *name)
{
	CmpackDateTime dt;
	CmpackPhtInfo info;
	const struct tm *t;
	char datestr[64], timestr[64], actdate[256];

	// Fill the table
	if (!pht->Valid())
		return;

	m_PhtFile = pht;
	m_FileType = PHOT;
	BeginUpdate();
	SetCaption(name);
	pht->GetParams(CMPACK_PI_FRAME_PARAMS | CMPACK_PI_ORIGIN_CRDATE | CMPACK_PI_PHOT_PARAMS, info);
	Add("width", info.width, "Image width in pixels");
	Add("height", info.height, "Image height in pixels");
	Add("jd", info.jd, 12, "Julian date of observation");
	cmpack_decodejd(info.jd, &dt);
	cmpack_datetostr(&dt.date, datestr, 64);
	Add("date", datestr, "Date of observation");
	cmpack_timetostr(&dt.time, timestr, 64);
	Add("time", timestr, "Time of observation");
	Add("filter", info.filter, "Optical filter");
	Add("exptime", info.exptime, 6, "Exposure duration in seconds");
	if (info.ccdtemp>-900 && info.ccdtemp<900)
		Add("ccdtemp", info.ccdtemp, 6, "CCD temperature in deg.C");
	Add("origin", info.origin, "Reduction software");
	t = &info.crtime;
	sprintf(actdate, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year+1900, 
		t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	Add("crdate", actdate, "Date of creation");

	// Photometry process information 
	Add("phot_stars", pht->ObjectCount(), "Number of stars");
	Add("phot_apertures", pht->ApertureCount(), "Number of apertures");
	Add("phot_datalo", info.range[0], 6, "Lowest good pixel value");
	Add("phot_datahi", info.range[1], 6, "Highest good pixel value");
	Add("phot_gain", info.gain, 6, "Gain in electrons per ADU");
	Add("phot_rnoise", info.rnoise, 6, "Readout noise in ADU");
	Add("phot_fwhm_exp", info.fwhm_exp, 6, "Expected FWHM of objects");
	Add("phot_fwhm_mean", info.fwhm_mean, 6, "Mean FWHM of objects");
	Add("phot_fwhm_err", info.fwhm_err, 6, "Std. error of FWHM of objects");
	Add("phot_thresh", info.threshold, 6, "Detection threshold");
	Add("phot_losharp", info.sharpness[0], 6, "Low sharpness cutoff");
	Add("phot_hisharp", info.sharpness[1], 6, "High sharpness cutoff");
	Add("phot_loround", info.roundness[0], 6, "Low roundness cutoff");
	Add("phot_hiround", info.roundness[1], 6, "High roundness cutoff");
	EndUpdate();

	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CShowHeaderDlg::Execute(const CCatalog *cat, const gchar *name)
{
	const char	*key, *val, *com;

	if (!cat->Valid())
		return;

	m_CatFile = cat;
	m_FileType = CATALOG;
	BeginUpdate();
	SetCaption(name);
	for (int i=0; cat->GetParam(i, &key, &val, &com); i++)
		Add(key, val, com);
	EndUpdate();

	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CShowHeaderDlg::Execute(const CTable *tab, const gchar *name)
{
	char	*key, *val;

	if (!tab->Valid())
		return;

	m_Table = tab;
	m_FileType = TABLE;
	BeginUpdate();
	SetCaption(name);
	for (int i=0; tab->GetParam(i, &key, &val); i++) {
		Add(key, val, NULL);
		g_free(key);
		g_free(val);
	}
	EndUpdate();

	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

//
// Set window caption
//
void CShowHeaderDlg::SetCaption(const char *name)
{
	char buf[512];
	sprintf(buf, "%s - header - %s", name, g_AppTitle);
	gtk_window_set_title(GTK_WINDOW(m_pDlg), buf);
	g_free(m_Name);
	m_Name = g_strdup(name);
}

//
// Start update of multiple rows
//
void CShowHeaderDlg::BeginUpdate(void)
{
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_View), NULL);
	gtk_tree_store_clear(m_Data);
}

//
// Finish update of multiple files
//
void CShowHeaderDlg::EndUpdate(void)
{
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_View), GTK_TREE_MODEL(m_Data));
	gtk_tree_view_expand_all(GTK_TREE_VIEW(m_View));
}

//
// Add field
//
void CShowHeaderDlg::Add(const gchar *keyword, const gchar *value, const gchar *comment)
{
	GtkTreeIter iter;

	gtk_tree_store_append(m_Data, &iter, NULL);
	if (keyword)
		gtk_tree_store_set(m_Data, &iter, 0, keyword, -1);
	if (value)
		gtk_tree_store_set(m_Data, &iter, 1, value, -1);
	if (comment)
		gtk_tree_store_set(m_Data, &iter, 2, comment, -1);
}

void CShowHeaderDlg::Add(const gchar *keyword, int value, const gchar *comment)
{
	char buf[64];

	sprintf(buf, "%d", value);
	Add(keyword, buf, comment);
}

void CShowHeaderDlg::Add(const gchar *keyword, double value, int prec, const gchar *comment)
{
	char buf[64];

	sprintf(buf, "%.*f", prec, value);
	Add(keyword, buf, comment);
}

void CShowHeaderDlg::button_clicked(GtkWidget *button, CShowHeaderDlg *pDlg)
{
	pDlg->OnButtonClicked(button);
}

void CShowHeaderDlg::OnButtonClicked(GtkWidget *pBtn)
{
	if (pBtn==GTK_WIDGET(m_SaveBtn))
		SaveAs();
}

void CShowHeaderDlg::SaveAs(void)
{
	CExportHeaderDlg dlg(GTK_WINDOW(m_pDlg));
	switch (m_FileType)
	{
	case CCDFILE:
		dlg.Execute(m_CCDFile, m_Name);
		break;
	case CATALOG:
		dlg.Execute(m_CatFile, m_Name);
		break;
	case PHOT:
		dlg.Execute(m_PhtFile, m_Name);
		break;
	case TABLE:
		dlg.Execute(m_Table, m_Name);
		break;
	default:
		break;
	}
}

//------------------------   SAVE PHOTOMETRY FILE   ---------------------------------

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
CExportHeaderDlg::CExportHeaderDlg(GtkWindow *pParent):m_pParent(pParent), 
	m_FileType(TYPE_CSV), m_Matched(false), m_Updating(false)
{
	// Dialog with buttons
	m_pDlg = gtk_file_chooser_dialog_new("Export header", pParent,
		GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_file_chooser_standard_tooltips(GTK_FILE_CHOOSER(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(m_pDlg), true);

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
	
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(m_pDlg), frame);
	gtk_widget_show_all(frame);
}

CExportHeaderDlg::~CExportHeaderDlg()
{
	gtk_widget_destroy(m_pDlg);
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
}

void CExportHeaderDlg::Execute(const CCCDFile *file, const gchar *name)
{
	gchar *filename = Execute(name);
	if (filename) {
		GError *error = NULL;
		if (!file->ExportHeader(filename, MimeType(), Flags(), &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
			}
		}
		g_free(filename);
	}
}

void CExportHeaderDlg::Execute(const CPhot *file, const gchar *name)
{
	gchar *filename = Execute(name);
	if (filename) {
		GError *error = NULL;
		if (!file->ExportHeader(filename, MimeType(), Flags(), &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
			}
		}
		g_free(filename);
	}
}

void CExportHeaderDlg::Execute(const CCatalog *file, const gchar *name)
{
	gchar *filename = Execute(name);
	if (filename) {
		GError *error = NULL;
		if (!file->ExportHeader(filename, MimeType(), Flags(), &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
			}
		}
		g_free(filename);
	}
}

void CExportHeaderDlg::Execute(const CTable *file, const gchar *name)
{
	gchar *filename = Execute(name);
	if (filename) {
		GError *error = NULL;
		if (!file->ExportHeader(filename, MimeType(), Flags(), &error)) {
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
			}
		}
		g_free(filename);
	}
}

gchar *CExportHeaderDlg::Execute(const gchar *name)
{
	m_Updating = true;

	// Restore last folder and file name
	gchar *folder = g_Project->GetStr("Output", "Folder", NULL);
	if (!folder)
		folder = g_path_get_dirname(g_Project->Path());
	if (folder && g_file_test(folder, G_FILE_TEST_IS_DIR)) 
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDlg), folder);
	g_free(folder);

	gchar *filename = SetFileExtension(name, FileExtension(m_FileType));
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
	m_Options[TYPE_CSV].header = g_Project->GetBool("ExportHeader", "CSV_HEADER", true);

	m_Updating = false;

	OnTypeChanged();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT)
		return NULL;
	gtk_widget_hide(m_pDlg);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_pDlg));

	// Save last folder and file name
	gchar *dirpath = g_path_get_dirname(filename);
	g_Project->SetStr("Output", "Folder", dirpath);
	g_free(dirpath);

	// Save settings
	g_Project->SetBool("ExportHeader", "CSV_HEADER", m_Options[TYPE_CSV].header);

	return filename;
}

const gchar *CExportHeaderDlg::MimeType(void)
{
	return FileMimeType(m_FileType);
}

unsigned CExportHeaderDlg::Flags(void)
{
	int flags = 0;
	const tOptions *opt = &m_Options[m_FileType];
	if (!opt->header)
		flags |= CPhot::EXPORT_NO_HEADER;
	return flags;
}

void CExportHeaderDlg::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_Header, TRUE);
}

void CExportHeaderDlg::button_toggled(GtkToggleButton *pWidget, CExportHeaderDlg *pMe)
{
	pMe->OnButtonToggled(pWidget);
}

void CExportHeaderDlg::OnButtonToggled(GtkToggleButton *pWidget)
{
	if (GTK_WIDGET(pWidget) == m_Header) 
		m_Options[m_FileType].header = gtk_toggle_button_get_active(pWidget)!=0;
	UpdateControls();
}

void CExportHeaderDlg::selection_changed(GtkComboBox *pWidget, CExportHeaderDlg *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportHeaderDlg::OnSelectionChanged(GtkComboBox *pWidget)
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

void CExportHeaderDlg::OnTypeChanged(void)
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
	m_Updating = false;

	UpdateControls();
}

void CExportHeaderDlg::response_dialog(GtkWidget *pDlg, gint response_id, CExportHeaderDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id)) 
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CExportHeaderDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_SAVE_TABLE);
		return false;
	}
	return true;
}
