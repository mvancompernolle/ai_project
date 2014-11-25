/**************************************************************

ccd_image_size.cpp (C-Munipack project)
Widget that allows an user to define a size of exported image
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
#include <gtk/gtk.h>

#include "utils.h"
#include "export_options.h"

#define MIN_WIDTH			1
#define MIN_HEIGHT			1
#define MAX_WIDTH			8192
#define MAX_HEIGHT			8192

//-------------------------   IMAGE EXPORT FORMATS   --------------------------------

// Image file formats
static const struct {
	const gchar *MimeType, *Caption, *ExtensionList;
	const gchar *FilterName, *FilterPatternList;
} ImageFormats[] = {
	{ "image/png",	"Portable Network Graphics",	"png\0",		"PNG Files",	"*.png" },
	{ "image/jpeg",	"JPEG File Interchange Format",	"jpg\0jpeg\0",	"JPEG Files",	"*.jpg\0*.jpeg" },
	{ NULL }
};

// Caption (human readable format description) for given image mime type
const gchar *ImageFileCaption(const gchar *type)
{
	if (type) {
		for (int i=0; ImageFormats[i].MimeType!=NULL; i++) {
			if (strcmp(type, ImageFormats[i].MimeType)==0)
				return ImageFormats[i].Caption;
		}
	}
	return NULL;
}

// Get standard image file extension
const gchar *ImageFileExtension(const gchar *type)
{
	if (type) {
		for (int i=0; ImageFormats[i].MimeType!=NULL; i++) {
			if (strcmp(type, ImageFormats[i].MimeType)==0)
				return ImageFormats[i].ExtensionList;
		}
	}
	return NULL;
}

// Determine image type by file extension
const gchar *ImageTypeByExtension(const gchar *fpath)
{
	gchar *fext = GetFileExtension(fpath);
	if (fext) {
		for (int i=0; ImageFormats[i].MimeType!=NULL; i++) {
			const gchar *exts = ImageFormats[i].ExtensionList;
			if (exts) {
				while (*exts!='\0') {
					if (StrCaseCmp0(fext, exts)==0) {
						g_free(fext);
						return ImageFormats[i].MimeType;
					}
					exts += strlen(exts)+1;
				}
			}
		}
		g_free(fext);
	}
	return NULL;
}

// File filter for given image type
GtkFileFilter *ImageFileFilter(const gchar *type)
{
	if (type) {
		for (int i=0; ImageFormats[i].MimeType!=NULL; i++) {
			if (strcmp(type, ImageFormats[i].MimeType)==0) {
				if (type>=0 && ImageFormats[i].FilterName && ImageFormats[i].FilterPatternList) {
					GtkFileFilter *filter = gtk_file_filter_new();
					const gchar *fpl = ImageFormats[i].FilterPatternList;
					while (*fpl!=0) {
						gtk_file_filter_add_pattern(filter, fpl);
						fpl += strlen(fpl)+1;
					}
					gtk_file_filter_set_name(filter, ImageFormats[i].FilterName);
					return filter;
				}
				break;
			}
		}
	}
	return NULL;
}

//-------------------------   CHART SIZE WIDGET   --------------------------------

static const struct {
	const gchar *MimeType;
} FileFormats[] = {
	{ "image/png" }, 
	{ "image/jpeg" },
	{ NULL }
};

CExportChartOptions::CExportChartOptions(void):m_FileType(TYPE_PNG), m_Mode(RELATIVE), 
	m_ChartWidth(0), m_ChartHeight(0), m_ImageWidth(0), m_ImageHeight(0),
	m_Changed(false), m_Updating(false), m_Zoom(1.0), m_ZoomMin(1.0), m_ZoomMax(1.0)
{
	m_Frame = gtk_frame_new("Export options");
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(m_Frame), vbox);

	GtkWidget *tbox = gtk_table_new(2, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 4);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);

	// File types
	GtkWidget *label = gtk_label_new("File type");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_FileTypes = gtk_list_store_new(2, GTK_TYPE_INT, GTK_TYPE_STRING);
	for (int i=0; FileFormats[i].MimeType; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(m_FileTypes, &iter);
		gtk_list_store_set(m_FileTypes, &iter, 0, i, 1, FileTypeCaption((tFileType)i), -1);
	}
	m_TypeCombo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_FileTypes));
	gtk_widget_set_tooltip_text(m_TypeCombo, "Format of a target file");
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_TypeCombo, 1, 3, 0, 1);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_TypeCombo), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(m_TypeCombo), renderer, "text", 1);
	g_signal_connect(G_OBJECT(m_TypeCombo), "changed", G_CALLBACK(selection_changed), this);
	SelectItem(GTK_COMBO_BOX(m_TypeCombo), m_FileType);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_TypeCombo))<0) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), 0);
		m_FileType = (tFileType)SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
	}
	gtk_widget_set_sensitive(m_TypeCombo, 
		gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_FileTypes), NULL)>1);

	// Jpeg quality
	m_JpegQLabel = gtk_label_new("JPEG quality");
	gtk_misc_set_alignment(GTK_MISC(m_JpegQLabel), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), m_JpegQLabel, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	GtkObject *adj = gtk_adjustment_new(0, 50, 100, 1, 5, 0);
	m_JpegQuality = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_JpegQuality, "Specify quality factor of a target image (0=worst, 100=best)");
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_JpegQuality), true);
	gtk_table_attach(GTK_TABLE(tbox), m_JpegQuality, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_JpegQUnit = gtk_label_new("%");
	gtk_misc_set_alignment(GTK_MISC(m_JpegQUnit), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbox), m_JpegQUnit, 2, 3, 1, 2);

	// Image size
	tbox = gtk_table_new(3, 5, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), tbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 4);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 4);

	label = gtk_label_new("Image size");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), label, 0, 5, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	// Relative size
	m_Relative = gtk_radio_button_new_with_label(NULL, "Relative to original size:");
	gtk_widget_set_tooltip_text(m_Relative, "Image size is specified in % of the original frame");
	g_signal_connect(G_OBJECT(m_Relative), "toggled", G_CALLBACK(toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_Relative, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(0, 0, 1000, 1, 10, 0);
	m_ZoomAdj = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_ZoomAdj, "Size of the target image in % of the original frame");
	g_signal_connect(G_OBJECT(m_ZoomAdj), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_ZoomAdj), true);
	gtk_table_attach(GTK_TABLE(tbox), m_ZoomAdj, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_set_size_request(m_ZoomAdj, 60, -1);
	m_Percents = gtk_label_new("%");
	gtk_misc_set_alignment(GTK_MISC(m_Percents), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), m_Percents, 2, 5, 1, 2,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	// Absolute size
	GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_Relative));
	m_Absolute = gtk_radio_button_new_with_label(group, "Absolute size - width:");
	gtk_widget_set_tooltip_text(m_Absolute, "Image size is specified in pixels");
	g_signal_connect(G_OBJECT(m_Absolute), "toggled", G_CALLBACK(toggled), this);
	gtk_table_attach(GTK_TABLE(tbox), m_Absolute, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(0, MIN_WIDTH, MAX_WIDTH, 1, 10, 0);
	m_AbsWidth = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_AbsWidth, "Width of target image in pixels");
	g_signal_connect(G_OBJECT(m_AbsWidth), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_AbsWidth), true);
	gtk_table_attach(GTK_TABLE(tbox), m_AbsWidth, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_set_size_request(m_AbsWidth, 80, -1);
	m_HeightLabel = gtk_label_new("height:");
	gtk_misc_set_alignment(GTK_MISC(m_HeightLabel), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), m_HeightLabel, 2, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	adj = gtk_adjustment_new(0, MIN_HEIGHT, MAX_HEIGHT, 1, 10, 0);
	m_AbsHeight = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 0);
	gtk_widget_set_tooltip_text(m_AbsHeight, "Height of target image in pixels");
	g_signal_connect(G_OBJECT(m_AbsHeight), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_AbsHeight), true);
	gtk_table_attach(GTK_TABLE(tbox), m_AbsHeight, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_set_size_request(m_AbsHeight, 80, -1);
	m_Pixels = gtk_label_new("pixels");
	gtk_misc_set_alignment(GTK_MISC(m_Pixels), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbox), m_Pixels, 4, 5, 2, 3, 
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);

	gtk_widget_show_all(m_Frame);
}

CExportChartOptions::~CExportChartOptions()
{
	if (m_FileTypes)
		g_object_unref(m_FileTypes);
}

void CExportChartOptions::Init(int width, int height, tSizeMode mode, const gchar *type, 
	double zoom, int dst_width, int jpeg_quality)
{
	m_Updating = true;

	m_FileType = MimeTypeToFileType(type);
	SelectItem(GTK_COMBO_BOX(m_TypeCombo), m_FileType);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_TypeCombo))<0) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_TypeCombo), 0);
		m_FileType = (tFileType)SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
	}
	m_ChartWidth = width;
	m_ChartHeight = height;
	m_Mode = mode;
	if (m_ChartWidth>0 && m_ChartHeight>0) {
		m_ZoomMin = MAX(MIN_WIDTH/(double)m_ChartWidth, MIN_HEIGHT/(double)m_ChartHeight);
		m_ZoomMax = MIN(MAX_WIDTH/(double)m_ChartWidth, MAX_HEIGHT/(double)m_ChartHeight);
		m_Zoom = (m_Mode ? zoom : (double)dst_width/m_ChartWidth);
	} else {
		m_ZoomMin = m_ZoomMax = m_Zoom = 1.0;
	}
	m_Zoom = LimitValue(m_Zoom, m_ZoomMin, m_ZoomMax);
	m_ImageWidth = RoundToInt(m_ChartWidth * m_Zoom);
	m_ImageHeight = RoundToInt(m_ChartHeight * m_Zoom);

	// Restore configuration
	g_object_set((GObject*)gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_ZoomAdj)), 
		"lower", 100.0 * m_ZoomMin, "upper", 100.0 * m_ZoomMax, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Relative), m_Mode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Absolute), !m_Mode);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ZoomAdj), 100.0 * m_Zoom);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsWidth), m_ImageWidth);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsHeight), m_ImageHeight);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_JpegQuality), jpeg_quality);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Relative), m_Mode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Absolute), !m_Mode);
	m_Updating = false;

	UpdateControls();
	m_Changed = false;
}

void CExportChartOptions::value_changed(GtkSpinButton *spinbutton, CExportChartOptions *pMe)
{
	pMe->OnValueChanged(spinbutton);
}

void CExportChartOptions::OnValueChanged(GtkSpinButton *spinbutton)
{
	int width, height;

	if (!m_Updating) {
		m_Updating = true;
		if (spinbutton == GTK_SPIN_BUTTON(m_ZoomAdj)) {
			m_Zoom = LimitValue(0.01 * gtk_spin_button_get_value(spinbutton), m_ZoomMin, m_ZoomMax);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ZoomAdj), 100.0 * m_Zoom);
			m_ImageWidth = RoundToInt(m_ChartWidth * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsWidth), m_ImageWidth);
			m_ImageHeight = RoundToInt(m_ChartHeight * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsHeight), m_ImageHeight);
			m_Changed = true;
		} else if (spinbutton == GTK_SPIN_BUTTON(m_AbsWidth)) {
			width = gtk_spin_button_get_value_as_int(spinbutton);
			m_Zoom = LimitValue((m_ChartWidth>0 ? (double)width/m_ChartWidth : 1.0), m_ZoomMin, m_ZoomMax);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ZoomAdj), 100.0 * m_Zoom);
			m_ImageWidth = RoundToInt(m_ChartWidth * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsWidth), m_ImageWidth);
			m_ImageHeight = RoundToInt(m_ChartHeight * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsHeight), m_ImageHeight);
			m_Changed = true;
		} else if (spinbutton == GTK_SPIN_BUTTON(m_AbsHeight)) {
			height = gtk_spin_button_get_value_as_int(spinbutton);
			m_Zoom = LimitValue((m_ChartHeight>0 ? (double)height/m_ChartHeight : 1.0), m_ZoomMin, m_ZoomMax);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ZoomAdj), 100.0 * m_Zoom);
			m_ImageWidth = RoundToInt(m_ChartWidth * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsWidth), m_ImageWidth);
			m_ImageHeight = RoundToInt(m_ChartHeight * m_Zoom);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_AbsHeight), m_ImageHeight);
			m_Changed = true;
		}
		m_Updating = false;
	}
}

void CExportChartOptions::toggled(GtkToggleButton *togglebutton, CExportChartOptions *pMe)
{
	pMe->OnToggled(togglebutton);
}

void CExportChartOptions::OnToggled(GtkToggleButton *togglebutton)
{
	if (togglebutton == GTK_TOGGLE_BUTTON(m_Relative)) {
		if (!m_Updating) {
			m_Mode = RELATIVE;
			UpdateControls();
			m_Changed = true;
		}
	} else 
	if (togglebutton == GTK_TOGGLE_BUTTON(m_Absolute)) {
		if (!m_Updating) {
			m_Mode = ABSOLUTE;
			UpdateControls();
			m_Changed = true;
		}
	}
}

double CExportChartOptions::Zoom()
{
	if (m_Changed)
		UpdateData();
	return m_Zoom;
}

int CExportChartOptions::OutputWidth()
{
	if (m_Changed)
		UpdateData();
	return m_ImageWidth;
}

int CExportChartOptions::OutputHeight()
{
	if (m_Changed)
		UpdateData();
	return m_ImageHeight;
}

void CExportChartOptions::UpdateData()
{
	if (m_Mode) {
		// Relative size
		m_Zoom = 0.01 * gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_ZoomAdj));
		m_ImageWidth = RoundToInt(m_ChartWidth * m_Zoom);
		m_ImageHeight = RoundToInt(m_ChartHeight * m_Zoom);
	} else {
		// Absolute size
		m_ImageWidth = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_AbsWidth));
		m_ImageHeight = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_AbsHeight));
		m_Zoom = (m_ChartWidth>0 ? (double)m_ImageWidth/m_ChartWidth : 1.0);
	}
	m_Changed = false;
}

int CExportChartOptions::JpegQuality(void) const
{ 
	return RoundToInt(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_JpegQuality)));
}

void CExportChartOptions::selection_changed(GtkComboBox *pWidget, CExportChartOptions *pMe)
{
	pMe->OnSelectionChanged(pWidget);
}

void CExportChartOptions::OnSelectionChanged(GtkComboBox *pWidget)
{
	if (!m_Updating) {
		int ft = SelectedItem(GTK_COMBO_BOX(m_TypeCombo));
		if (ft>=0 && ft!=m_FileType) {
			m_FileType = (tFileType)ft;
			Callback(CB_TYPE_CHANGED);
			UpdateControls();
		}
	}
}

void CExportChartOptions::UpdateControls(void)
{
	gtk_widget_set_sensitive(m_JpegQuality, m_FileType==TYPE_JPEG);
	gtk_widget_set_sensitive(m_JpegQLabel, m_FileType==TYPE_JPEG);
	gtk_widget_set_sensitive(m_JpegQUnit, m_FileType==TYPE_JPEG);

	gtk_widget_set_sensitive(m_ZoomAdj, m_Mode);
	gtk_widget_set_sensitive(m_Percents, m_Mode);
	gtk_widget_set_sensitive(m_AbsWidth, !m_Mode);
	gtk_widget_set_sensitive(m_HeightLabel, !m_Mode);
	gtk_widget_set_sensitive(m_AbsHeight, !m_Mode);
	gtk_widget_set_sensitive(m_Pixels, !m_Mode);
}

const gchar *CExportChartOptions::FileTypeToMimeType(tFileType type)
{
	if (type>=0 && type<TYPE_N_ITEMS)
		return FileFormats[type].MimeType;
	return NULL;
}

CExportChartOptions::tFileType CExportChartOptions::MimeTypeToFileType(const gchar *type)
{
	if (type) {
		for (int i=0; FileFormats[i].MimeType; i++)
			if (strcmp(type, FileFormats[i].MimeType)==0)
				return (tFileType)i;
	}
	return TYPE_N_ITEMS;
}
