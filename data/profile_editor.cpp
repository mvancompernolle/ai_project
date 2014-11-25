/**************************************************************

project_dlg.cpp (C-Munipack project)
Project settings dialog
Copyright (C) 2012 David Motl, dmotl@volny.cz

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

#include "profile_editor.h"
#include "observer_dlg.h"
#include "main.h"

// Options for image rotation
static const CEditProfileBase::tStringList BitpixStrings[] = {
	{ "Autodetection",					CMPACK_BITPIX_AUTO },
	{ "Signed short int (2 bytes)",		CMPACK_BITPIX_SSHORT },
	{ "Unsigned short int (2 bytes)",	CMPACK_BITPIX_USHORT },
	{ "Signed int (4 bytes)",			CMPACK_BITPIX_SLONG },
	{ "Unsigned int (4 bytes)",			CMPACK_BITPIX_ULONG },
	{ "Single precision FP (4 bytes)",	CMPACK_BITPIX_FLOAT },
	{ "Double precision FP (8 bytes)",	CMPACK_BITPIX_DOUBLE },
	{ NULL }
};

// Binning
static const CEditProfileBase::tStringList BinningStrings[] = {
	{ "No binning",			1 },
	{ "2x2",				2 },
	{ "3x3",				3 },
	{ "4x4",				4 },
	{ NULL }
};

static int binning_to_index(int binning)
{
	for (int i=0; BinningStrings[i].str!=NULL; i++) {
		if (binning == BinningStrings[i].id)
			return i;
	}
	return -1;
}

static int index_to_binning(int index)
{
	if (index>=0)
		return BinningStrings[index].id;
	return 0;
}

static int bitpix_to_index(int bitpix)
{
	for (int i=0; BitpixStrings[i].str!=NULL; i++) {
		if (bitpix == BitpixStrings[i].id)
			return i;
	}
	return 0;
}

static int index_to_bitpix(int index)
{
	if (index>=0)
		return BitpixStrings[index].id;
	return -1;
}

static const char *PageIds[EndOfProfilePages] = {
	"Profile", "Project", "Files", "Camera", "Source files", "Calibration",
	"Star detection", "Photometry", "Matching", "Light curve", "Master bias",
	"Master dark", "Master flat", "Merge frames", "Find variable", "Observer"
};

//-------------------------   CONFIGURATION DIALOG   --------------------------------

CEditProfileBase::CEditProfileBase(void):m_CurrentPage(NULL), m_PageList(NULL), 
	m_FilteredPageList(NULL), m_ProjectType(EndOfProjectTypes), m_ReadOnly(false), 
	m_Updating(false), m_Changed(false), m_currentPageId(EndOfProfilePages), 
	m_lastPageId(PAGE_CAMERA)
{
	GtkWidget *list_frame, *vbox_page;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	memset(&m_SizeRequest, 0, sizeof(GtkRequisition));

	// Editor layout
	m_pBox = gtk_hbox_new(FALSE, 4);

	// List of categories
	m_PageView = gtk_tree_view_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_PageView), col);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", 3);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView)), GTK_SELECTION_BROWSE);
	list_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(list_frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(list_frame), m_PageView);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_PageView), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView));
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tree_selection_changed), this);
	gtk_box_pack_start(GTK_BOX(m_pBox), list_frame, FALSE, TRUE, 0);
	gtk_widget_set_size_request(list_frame, 200, -1);

	m_PageScroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_PageScroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(m_pBox), m_PageScroller, TRUE, TRUE, 0);
	vbox_page = gtk_vbox_new(FALSE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_PageScroller), vbox_page);

    m_TitleFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(m_TitleFrame), GTK_SHADOW_NONE);
    gtk_box_pack_start(GTK_BOX(vbox_page), m_TitleFrame, FALSE, FALSE, 0);
	m_PageTitle = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(m_TitleFrame), m_PageTitle);

	m_Separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox_page), m_Separator, FALSE, FALSE, 4);
	
	m_PageFrame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(vbox_page), m_PageFrame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(m_PageFrame), GTK_SHADOW_NONE);
}

void CEditProfileBase::PreparePages()
{
	if (!m_PageList) {
		m_PageList = gtk_tree_store_new(5, G_TYPE_INT, G_TYPE_STRING, GTK_TYPE_WIDGET, GDK_TYPE_PIXBUF, GTK_TYPE_WIDGET);
		m_FilteredPageList = gtk_tree_model_filter_new(GTK_TREE_MODEL(m_PageList), NULL);
		gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(m_FilteredPageList), 
			(GtkTreeModelFilterVisibleFunc)page_visible, this, NULL);

		GSList *stack = g_slist_prepend(NULL, new tStackItem(-1, NULL));

		CreatePages(&stack);

		// Free remaining items on stack
		g_slist_foreach(stack->next, (GFunc)tStackItem::Release, NULL);
		g_slist_free(stack);

		m_CurrentPage = NULL;

		// Compute size requisition
		GtkRequisition sreq;
		m_SizeRequest.width = 320; 
		m_SizeRequest.height = 240;
		gtk_tree_model_foreach(GTK_TREE_MODEL(m_PageList), (GtkTreeModelForeachFunc)max_dialog_size, &m_SizeRequest);
		gtk_widget_show_all(m_TitleFrame);
		gtk_widget_size_request(m_TitleFrame, &sreq);
		m_SizeRequest.height += sreq.height;
		gtk_widget_size_request(m_Separator, &sreq);
		m_SizeRequest.height += sreq.height;
		GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_PageView));
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_PageView), GTK_TREE_MODEL(m_PageList));
		gtk_tree_view_expand_all(GTK_TREE_VIEW(m_PageView));
		gtk_widget_size_request(m_PageView, &sreq);
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_PageView), model);
		m_SizeRequest.width += sreq.width + gtk_box_get_spacing(GTK_BOX(m_pBox)) + 8;
		m_SizeRequest.height += 8;
	}
}

GtkRequisition CEditProfileBase::GetSizeRequest(void)
{
	PreparePages();
	return m_SizeRequest;
}

void CEditProfileBase::CreatePages(GSList **stack)
{
	int imin, imax;
	double dmin, dmax;

	// Camera
	GtkWidget *tbox = CreatePage(PAGE_CAMERA, 1, "Camera", NULL, TRUE, stack);
	m_RNoise = add_spin_button_dbl(GTK_TABLE(tbox), 0, "Readout noise", 
		CProfile::READ_NOISE, 0.1, 2, "Readout noise in electrons");
	m_ADCGain = add_spin_button_dbl(GTK_TABLE(tbox), 1, "ADC gain", 
		CProfile::ADC_GAIN, 0.1, 6, "A/D converter gain in electrons per ADU");
	
	// Source frames
	tbox = CreatePage(PAGE_SOURCE_FILES, 1, "Source frames", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Pixel value range</b>");
	GtkWidget *tbox2 = gtk_table_new(4, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_attach(GTK_TABLE(tbox), tbox2, 0, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	add_label(GTK_TABLE(tbox2), 0, "Bad pixels have value equal to or less than:");
	m_DataMin = add_spin_button_dbl(GTK_TABLE(tbox2), 1, "Min. pixel value", 
		CProfile::BAD_PIXEL_VALUE, 1, 0, "Pixel value that indicates bad pixel (usually 0)");
	add_label(GTK_TABLE(tbox2), 2, "Overexposed pixels have value equal to or greater than:");
	m_DataMax = add_spin_button_dbl(GTK_TABLE(tbox2), 3, "Max. pixel value", 
		CProfile::OVEREXPOSED_VALUE, 1, 0, "Pixel value that indicates overexposed value");
	add_label(GTK_TABLE(tbox), 3, "<b>Transformations</b>");
	tbox2 = gtk_table_new(8, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_attach(GTK_TABLE(tbox), tbox2, 0, 3, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	add_label(GTK_TABLE(tbox2), 0, "Note: The transformations are applied in the order as they appear below.");
	add_label(GTK_TABLE(tbox2), 1, "Convert frames to the following pixel data format:");
	m_WorkFormat = add_combo_box(GTK_TABLE(tbox2), 2, "Image data format", BitpixStrings,
		"Image data format used for internal data representation and storage.");
	add_label(GTK_TABLE(tbox2), 3, "Apply the following geometric transformations:");
	m_FlipV = add_check_btn(GTK_TABLE(tbox2), 4, "Flip image vertically", NULL);
	m_FlipH = add_check_btn(GTK_TABLE(tbox2), 5, "Flip image horizontally", NULL);
	add_label(GTK_TABLE(tbox2), 6, "Sum pixel values from neighboring pixels:");
	m_Binning = add_combo_box(GTK_TABLE(tbox2), 7, "Binning", BinningStrings,
		"Number of columns and rows summed together");
	add_label(GTK_TABLE(tbox2), 8, "Mask pixels that are close to the frame border:");
	tbox2 = gtk_table_new(2, 6, FALSE);
	gtk_table_attach(GTK_TABLE(tbox), tbox2, 0, 3, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_set_col_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_attach(GTK_TABLE(tbox2), new_label("Margin"), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	CProfile::GetBorderSizeLimits(&imin, &imax);
	gtk_table_attach(GTK_TABLE(tbox2), new_label("- left", 1), 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_BLeft = add_spin_button(GTK_TABLE(tbox2), 2, 0, imin, imax, 1, 0, "Number of unused pixels on the left side of each frame");
	gtk_table_attach(GTK_TABLE(tbox2), new_label("- right", 1), 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_BRight = add_spin_button(GTK_TABLE(tbox2), 2, 1, imin, imax, 1, 0, "Number of unused pixels on the right side of each frame");
	gtk_table_attach(GTK_TABLE(tbox2), new_label("- top", 1), 3, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	m_BTop = add_spin_button(GTK_TABLE(tbox2), 4, 0, imin, imax, 1, 0, "Number of unused pixels at the top of each frame");
	gtk_table_attach(GTK_TABLE(tbox2), new_label("- bottom", 1), 3, 4, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	m_BBottom = add_spin_button(GTK_TABLE(tbox2), 4, 1, imin, imax, 1, 0, "Number of unused pixels at the bottom of each frame");
	gtk_table_attach(GTK_TABLE(tbox2), new_label("", TRUE), 6, 7, 1, 2, 
		GtkAttachOptions(GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
	add_label(GTK_TABLE(tbox2), 6, "Add constant offset to the time of observation:");
	m_TimeOffset = add_spin_button_int(GTK_TABLE(tbox2), 7, "Time offset", 
		CProfile::TIME_OFFSET, 1, "Time offset in seconds that will be always added to the time of observation saved in source frames.");

	// Calibration
	tbox = CreatePage(PAGE_CALIBRATION, 1, "Calibration", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Calibration scheme</b>");
	GSList *group = NULL;
	m_StdCalibration = add_radio_btn(GTK_TABLE(tbox), 1, "Standard (dark + flat)", &group, 
		"Use standard calibration scheme");
	m_AdvCalibration = add_radio_btn(GTK_TABLE(tbox), 2, "Advanced (bias + scalable dark + flat)", &group,
		"Use advanced calibration scheme");

	// Star detection
	tbox = CreatePage(PAGE_STAR_DETECTION, 1, "Star detection", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Gaussian filter</b>");
	m_FWHM = add_spin_button_dbl(GTK_TABLE(tbox), 1, "Filter width (FWHM)", 
		CProfile::DETECTION_FWHM, 0.1, 2, "Expected average FWHM of objects.");
	add_label(GTK_TABLE(tbox), 3, "<b>Minimum brightness</b>");
	m_Thresh = add_spin_button_dbl(GTK_TABLE(tbox), 4, "Detection threshold", 
		CProfile::DETECTION_THRESHOLD, 0.1, 2, "Minimum signal from an object, filters background artifacts");
	add_label(GTK_TABLE(tbox), 6, "<b>Sharpness limits</b>");
	m_SharpMin = add_spin_button_dbl(GTK_TABLE(tbox), 7, "Minimum sharpness", 
		CProfile::MIN_SHARPNESS, 0.1, 2, "Minimum sharpness of an object, filters out cosmic particles, for example");
	m_SharpMax = add_spin_button_dbl(GTK_TABLE(tbox), 8, "Maximum sharpness", 
		CProfile::MAX_SHARPNESS, 0.1, 2, "Maximum sharpness of an object, filters out nebulous objects");
	add_label(GTK_TABLE(tbox), 10, "<b>Roundness limits</b>");
	m_RoundMin = add_spin_button_dbl(GTK_TABLE(tbox), 11, "Minimum roundness", 
		CProfile::MIN_ROUNDNESS, 0.1, 2, "Minimum roundness of an object, filters out elongated objects");
	m_RoundMax = add_spin_button_dbl(GTK_TABLE(tbox), 12, "Maximum roundness", 
		CProfile::MAX_ROUNDNESS, 0.1, 2, "Maximum roundness of an object, filters out elongated objects");

	// Photometry
	tbox = CreatePage(PAGE_PHOTOMETRY, 1, "Photometry", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Apertures</b>");
	add_label(GTK_TABLE(tbox), 1, "Radii of the apertures (1-12) for object brightness measurement (pixels):");
	tbox2 = gtk_table_new(4, 6, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox2), 8);
	gtk_table_attach(GTK_TABLE(tbox), tbox2, 0, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(tbox2), new_label("Aperture"), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	CProfile::GetApertureSizeLimits(&dmin, &dmax);
	for (int i=0; i<MAX_APERTURES; i++) {
		int row = (i % 4), col = (i / 4);
		char buf[256];
		sprintf(buf, "#%d", i+1);
		gtk_table_attach(GTK_TABLE(tbox2), new_label(buf, 1.0), col*2+1, col*2+2, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
		m_Aperture[i] = add_spin_button(GTK_TABLE(tbox2), col*2+2, row, 0, dmax, 0.1, 2, "Aperture radius in pixels");
	}
	add_label(GTK_TABLE(tbox), 4, "<b>Background</b>");
	add_label(GTK_TABLE(tbox), 5, "Size of the annulus for background level measurement (pixels):");
	m_SkyIn = add_spin_button_dbl(GTK_TABLE(tbox), 6, "Inner radius", 
		CProfile::SKY_INNER_RADIUS, 0.1, 2, "Inner radius of the annulus used to measure local background");
	m_SkyOut = add_spin_button_dbl(GTK_TABLE(tbox), 7, "Outer radius", 
		CProfile::SKY_OUTER_RADIUS, 0.1, 2, "Outer radius of the annulus used to measure local background");
	
	// Matching
	tbox = CreatePage(PAGE_MATCHING, 1, "Matching", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "<b>Matching algorithm</b>");
	group = NULL;
	m_MatchStandard = add_radio_btn(GTK_TABLE(tbox), 1, "Standard algorithm (requires at least 3 stars)", &group,
		"The standard algorithm is robust, each frame must contain at least 3 objects");
	m_MatchSparse = add_radio_btn(GTK_TABLE(tbox), 2, "Algorithm for sparse fields (2 stars or less)", &group,
		"The algorithm for sparse fields works also for frames that consists a single object only");
	m_MatchBox1 = gtk_table_new(3, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(m_MatchBox1), 8);
	gtk_table_set_col_spacings(GTK_TABLE(m_MatchBox1), 8);
	gtk_table_attach(GTK_TABLE(tbox), m_MatchBox1, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	add_label(GTK_TABLE(m_MatchBox1), 0, "<b>Standard matching parameters</b>");
	add_label(GTK_TABLE(m_MatchBox1), 1, "Read from the source file no more than");
	m_MatchRS = add_spin_button_int(GTK_TABLE(m_MatchBox1), 2, "Read stars", 
		CProfile::MATCH_READ_STARS, 1, "Number of objects that are used for finding transformation");
	add_label(GTK_TABLE(m_MatchBox1), 3, "Identify in each iteration at least");
	m_MatchIS = add_spin_button_int(GTK_TABLE(m_MatchBox1), 4, "Identification stars", 
		CProfile::MATCH_IDENT_STARS, 1, "Number of polygon vertices");
	add_label(GTK_TABLE(m_MatchBox1), 5, "Maximum misalignment of objects");
	m_MatchClip = add_spin_button_dbl(GTK_TABLE(m_MatchBox1), 6, "Clipping factor", 
		CProfile::MATCH_CLIP, 0.1, 1, "Sensitivity to the noise in position measurements");
	m_MatchBox2 = gtk_table_new(1, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(m_MatchBox2), 8);
	gtk_table_set_col_spacings(GTK_TABLE(m_MatchBox2), 8);
	gtk_table_attach(GTK_TABLE(tbox), m_MatchBox2, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	add_label(GTK_TABLE(m_MatchBox2), 0, "<b>Sparse fields matching parameters</b>");
	add_label(GTK_TABLE(m_MatchBox2), 1, "Highest allowed offset of the source and reference frame");
	m_MatchMax = add_spin_button_dbl(GTK_TABLE(m_MatchBox2), 2, "Max. offset (pixels)", 
		CProfile::MAX_OFFSET, 1, 0, "Maximum displacement between the source and the reference frame in pixels");

	// Light curve
	tbox = CreatePage(PAGE_LIGHT_CURVE, 1, "Light curve", NULL, TRUE, stack);
	m_ArtCompStar = add_check_btn(GTK_TABLE(tbox), 0, "Allow using an artificial comparison star",
		"Combine multiple comparison stars");

	// Master bias
	tbox = CreatePage(PAGE_MASTER_BIAS, 1, "Master bias", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Save output frame in the following pixel data format:");
	m_MBiasFormat = add_combo_box(GTK_TABLE(tbox), 1, "Output data format", BitpixStrings,
		"Target data format");

	// Master dark
	tbox = CreatePage(PAGE_MASTER_DARK, 1, "Master dark", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Save output frame in the following pixel data format:");
	m_MDarkFormat = add_combo_box(GTK_TABLE(tbox), 1, "Output data format", BitpixStrings,
		"Target data format");

	// Master flat
	tbox = CreatePage(PAGE_MASTER_FLAT, 1, "Master flat", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Save output frame in the following pixel data format:");
	m_MFlatFormat = add_combo_box(GTK_TABLE(tbox), 1, "Output data format", BitpixStrings,
		"Target data format");
	add_label(GTK_TABLE(tbox), 2, "Pixel values of output frame are normalized to the mean level:");
	m_MFlatLevel = add_spin_button_dbl(GTK_TABLE(tbox), 3, "Output mean level", 
		CProfile::MFLAT_LEVEL, 1, 0, "A master flat frame is normalized to the specified value in ADU");

	// Kombine
	tbox = CreatePage(PAGE_MERGE_FRAMES, 1, "Merge frames", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Save output frames in the following pixel data format:");
	m_KombineFormat = add_combo_box(GTK_TABLE(tbox), 1, "Output data format", BitpixStrings,
		"Target data format");

	// Find variables
	tbox = CreatePage(PAGE_FIND_VARIABLES, 1, "Find variables", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Do not report objects that have share of valid measurements less than:");
	m_VarFindThreshold = add_spin_button_dbl(GTK_TABLE(tbox), 1, "Clipping threshold", 
		CProfile::VARFIND_THRESHOLD, 1, 0, "Filter threshold in %, it rules out objects that don't have enough valid measurements");

	// Observer
	tbox = CreatePage(PAGE_OBSERVER, 1, "Observer", NULL, TRUE, stack);
	add_label(GTK_TABLE(tbox), 0, "Observer's default location:");
	m_LocName = add_entry(GTK_TABLE(tbox), 1, "Location", MAX_LOCATION_LEN, "Observer's location designation, e.g. Brno, Czech Republic");
	m_LocBtn = add_push_button(GTK_TABLE(tbox), 1, "More", "Load observer's coordinates from presets");
	m_Lon = add_entry(GTK_TABLE(tbox), 2, "Longitude", 32, "Observer's longitude in degrees, minutes and seconds, use E or W suffix to indicate hemisphere, e.g. 12 34 56 E");
	gtk_table_attach(GTK_TABLE(tbox), new_label("[d m s E/W]"), 2, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	m_Lat = add_entry(GTK_TABLE(tbox), 3, "Latitude", 32, "Observer's latitude in degrees, minutes and seconds, use N or S suffix to indicate hemisphere, e.g. 56 34 12 N");
	gtk_table_attach(GTK_TABLE(tbox), new_label("[d m s N/S]"), 2, 3, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
}

CEditProfileBase::~CEditProfileBase()
{
	g_object_unref(m_PageList);
	g_object_unref(m_FilteredPageList);
}

gboolean CEditProfileBase::max_dialog_size(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GtkRequisition *data)
{
	GtkWidget *page = NULL;
	GtkRequisition sreq;

	gtk_tree_model_get(model, iter, 2, &page, -1);
	gtk_widget_show_all(page);
    gtk_widget_size_request(page, &sreq);
	if (sreq.width > data->width)
		data->width = sreq.width;
	if (sreq.height > data->height)
		data->height = sreq.height;
    return FALSE;
}

GtkWidget *CEditProfileBase::CreatePage(tProfilePageId id, int indent, const gchar *caption, const gchar *icon, 
	bool def_btn, GSList **stack)
{
	GtkTreeIter iter, parent;

	while ((*stack)!=NULL && indent <= ((tStackItem*)(*stack)->data)->level) {
		GSList *next = (*stack)->next;
		delete (tStackItem*)(*stack)->data;
		g_slist_free_1(*stack);
		(*stack) = next;
	}
	g_assert((*stack)!=NULL && (*stack)->data!=NULL);

	GtkWidget *box = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(box), 4);
	if (((tStackItem*)(*stack)->data)->item!=NULL) {
		gtk_tree_model_get_iter(GTK_TREE_MODEL(m_PageList), &parent, ((tStackItem*)(*stack)->data)->item);
		gtk_tree_store_append(m_PageList, &iter, &parent);
	} else {
		gtk_tree_store_append(m_PageList, &iter, NULL);
	}

	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_PageList), &iter);
	*stack = g_slist_prepend(*stack, new tStackItem(indent, path));

	gtk_tree_store_set(m_PageList, &iter, 0, id, 1, caption, 2, box, -1);
	if (icon) {
		char *fpath = get_icon_file(icon);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(fpath, NULL);
		gtk_tree_store_set(m_PageList, &iter, 3, pixbuf, -1);
		g_free(fpath);
	}
	if (def_btn) {
		GtkWidget *btnbox = gtk_hbutton_box_new();
		gtk_button_box_set_layout(GTK_BUTTON_BOX(btnbox), GTK_BUTTONBOX_START);
		gtk_box_pack_end(GTK_BOX(box), btnbox, FALSE, TRUE, 0);
		GtkWidget *btn = gtk_button_new_with_label("Set defaults");
		gtk_widget_set_tooltip_text(btn, "Set parameters on the actual page to the default values");
		g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(setdefaults_clicked), this);
		gtk_tree_store_set(m_PageList, &iter, 4, btn, -1);
		gtk_container_add(GTK_CONTAINER(btnbox), btn);
	}
	GtkWidget *table = gtk_table_new(0, 0, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 8);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);
	gtk_box_pack_start(GTK_BOX(box), table, FALSE, TRUE, 0);
	return table;
}

gboolean CEditProfileBase::page_visible(GtkTreeModel *model, GtkTreeIter *iter, CEditProfileBase *pMe)
{
	tProfilePageId id;
	gtk_tree_model_get(model, iter, 0, &id, -1);
	return pMe->IsPageVisible(id);
}

bool CEditProfileBase::IsPageVisible(tProfilePageId id)
{
	switch (m_ProjectType)
	{
	case PROJECT_REDUCE:
		return (id==PAGE_CAMERA || id==PAGE_SOURCE_FILES ||	id==PAGE_CALIBRATION || 
			id==PAGE_STAR_DETECTION || id==PAGE_PHOTOMETRY || id==PAGE_MATCHING || 
			id==PAGE_LIGHT_CURVE || id==PAGE_FIND_VARIABLES || id==PAGE_OBSERVER);

	case PROJECT_COMBINING:
		return (id==PAGE_CAMERA || id==PAGE_SOURCE_FILES || id==PAGE_CALIBRATION || 
			id==PAGE_STAR_DETECTION || id==PAGE_PHOTOMETRY || id==PAGE_MATCHING || 
			id==PAGE_LIGHT_CURVE || id==PAGE_MERGE_FRAMES || id==PAGE_FIND_VARIABLES || 
			id==PAGE_OBSERVER);
			
	case PROJECT_MASTER_BIAS:
		return (id==PAGE_CAMERA || id==PAGE_SOURCE_FILES ||	id==PAGE_MASTER_BIAS);

	case PROJECT_MASTER_DARK:
		return (id==PAGE_CAMERA || id==PAGE_SOURCE_FILES ||	id==PAGE_CALIBRATION || 
			id==PAGE_MASTER_DARK);

	case PROJECT_MASTER_FLAT:
		return (id==PAGE_CAMERA || id==PAGE_SOURCE_FILES ||	id==PAGE_CALIBRATION || 
			id==PAGE_MASTER_FLAT);

	default:
		return TRUE;
	}
}

void CEditProfileBase::SetData(void)
{
	bool editable = !m_ReadOnly;
	bool adv_calib = m_Profile.GetBool(CProfile::ADVANCED_CALIBRATION);
	bool sparse_fields = m_Profile.GetBool(CProfile::SPARSE_FIELDS);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RNoise), 
		m_Profile.GetDbl(CProfile::READ_NOISE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ADCGain), 
		m_Profile.GetDbl(CProfile::ADC_GAIN));
	gtk_widget_set_sensitive(m_RNoise, editable);
	gtk_widget_set_sensitive(m_ADCGain, editable);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_DataMin), 
		m_Profile.GetDbl(CProfile::BAD_PIXEL_VALUE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_DataMax), 
		m_Profile.GetDbl(CProfile::OVEREXPOSED_VALUE));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_WorkFormat), bitpix_to_index(
		m_Profile.GetInt(CProfile::WORK_FORMAT)));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_Binning), binning_to_index(
		m_Profile.GetInt(CProfile::BINNING)));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FlipH), 
		m_Profile.GetBool(CProfile::FLIP_H));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FlipV), 
		m_Profile.GetBool(CProfile::FLIP_V));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_TimeOffset), 
		m_Profile.GetInt(CProfile::TIME_OFFSET));
	gtk_widget_set_sensitive(m_DataMin, editable);
	gtk_widget_set_sensitive(m_DataMax, editable);
	gtk_widget_set_sensitive(m_WorkFormat, editable);
	gtk_widget_set_sensitive(m_Binning, editable);
	gtk_widget_set_sensitive(m_FlipH, editable);
	gtk_widget_set_sensitive(m_FlipV, editable);
	gtk_widget_set_sensitive(m_TimeOffset, editable);
	CmpackBorder border(m_Profile.GetBorder());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BLeft), border.left);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BRight), border.right);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BTop), border.top);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BBottom), border.bottom);
	gtk_widget_set_sensitive(m_BLeft, editable);
	gtk_widget_set_sensitive(m_BRight, editable);
	gtk_widget_set_sensitive(m_BTop, editable);
	gtk_widget_set_sensitive(m_BBottom, editable);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_StdCalibration), !adv_calib);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AdvCalibration), adv_calib);
	gtk_widget_set_sensitive(m_StdCalibration, editable);
	gtk_widget_set_sensitive(m_AdvCalibration, editable);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FWHM), 
		m_Profile.GetDbl(CProfile::DETECTION_FWHM));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Thresh), 
		m_Profile.GetDbl(CProfile::DETECTION_THRESHOLD));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SharpMin), 
		m_Profile.GetDbl(CProfile::MIN_SHARPNESS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SharpMax), 
		m_Profile.GetDbl(CProfile::MAX_SHARPNESS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RoundMin), 
		m_Profile.GetDbl(CProfile::MIN_ROUNDNESS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RoundMax), 
		m_Profile.GetDbl(CProfile::MAX_ROUNDNESS));
	gtk_widget_set_sensitive(m_FWHM, editable);
	gtk_widget_set_sensitive(m_Thresh, editable);
	gtk_widget_set_sensitive(m_SharpMin, editable);
	gtk_widget_set_sensitive(m_SharpMax, editable);
	gtk_widget_set_sensitive(m_RoundMin, editable);
	gtk_widget_set_sensitive(m_RoundMax, editable);

	CApertures aper = m_Profile.Apertures();
	for (int i=0; i<MAX_APERTURES; i++) {
		int index = aper.Find(i+1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Aperture[i]), aper.GetRadius(index));
		gtk_widget_set_sensitive(GTK_WIDGET(m_Aperture[i]), editable);
	}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SkyIn), 
		m_Profile.GetDbl(CProfile::SKY_INNER_RADIUS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SkyOut), 
		m_Profile.GetDbl(CProfile::SKY_OUTER_RADIUS));
	gtk_widget_set_sensitive(m_SkyIn, editable);
	gtk_widget_set_sensitive(m_SkyOut, editable);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchStandard), 
		!sparse_fields);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchSparse), 
		sparse_fields);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchRS), 
		m_Profile.GetInt(CProfile::MATCH_READ_STARS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchIS), 
		m_Profile.GetInt(CProfile::MATCH_IDENT_STARS));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchClip), 
		m_Profile.GetDbl(CProfile::MATCH_CLIP));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchMax), 
		m_Profile.GetDbl(CProfile::MAX_OFFSET));
	gtk_widget_set_sensitive(m_MatchStandard, editable);
	gtk_widget_set_sensitive(m_MatchSparse, editable);
	gtk_widget_set_sensitive(m_MatchRS, editable);
	gtk_widget_set_sensitive(m_MatchIS, editable);
	gtk_widget_set_sensitive(m_MatchClip, editable);
	gtk_widget_set_sensitive(m_MatchMax, editable);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ArtCompStar), 
		m_Profile.GetBool(CProfile::MULTIPLE_COMP_STARS));
	gtk_widget_set_sensitive(m_ArtCompStar, editable);

	gtk_combo_box_set_active(GTK_COMBO_BOX(m_MBiasFormat), bitpix_to_index(
		m_Profile.GetInt(CProfile::MBIAS_FORMAT)));
	gtk_widget_set_sensitive(m_MBiasFormat, editable);

	gtk_combo_box_set_active(GTK_COMBO_BOX(m_MDarkFormat), bitpix_to_index(
		m_Profile.GetInt(CProfile::MDARK_FORMAT)));
	gtk_widget_set_sensitive(m_MDarkFormat, editable);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MFlatLevel), 
		m_Profile.GetDbl(CProfile::MFLAT_LEVEL));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_MFlatFormat), bitpix_to_index(
		m_Profile.GetInt(CProfile::MFLAT_FORMAT)));
	gtk_widget_set_sensitive(m_MFlatLevel, editable);
	gtk_widget_set_sensitive(m_MFlatFormat, editable);

	gtk_combo_box_set_active(GTK_COMBO_BOX(m_KombineFormat), bitpix_to_index(
		m_Profile.GetInt(CProfile::KOMBINE_FORMAT)));
	gtk_widget_set_sensitive(m_KombineFormat, editable);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_VarFindThreshold), 
		m_Profile.GetDbl(CProfile::VARFIND_THRESHOLD));
	gtk_widget_set_sensitive(m_VarFindThreshold, editable);

	const CLocation obs = m_Profile.DefaultLocation();
	if (obs.Name())
		gtk_entry_set_text(GTK_ENTRY(m_LocName), obs.Name());
	else
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
	if (obs.Lon())
		gtk_entry_set_text(GTK_ENTRY(m_Lon), obs.Lon());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
	if (obs.Lat())
		gtk_entry_set_text(GTK_ENTRY(m_Lat), obs.Lat());
	else
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");
	gtk_widget_set_sensitive(m_LocName, editable);
	gtk_widget_set_sensitive(m_LocBtn, editable);
	gtk_widget_set_sensitive(m_Lon, editable);
	gtk_widget_set_sensitive(m_Lat, editable);
}

void CEditProfileBase::GetData(void)
{
	m_Profile.SetDbl(CProfile::READ_NOISE, 
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_RNoise)));
	m_Profile.SetDbl(CProfile::ADC_GAIN,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_ADCGain)));

	m_Profile.SetDbl(CProfile::BAD_PIXEL_VALUE,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_DataMin)));
	m_Profile.SetDbl(CProfile::OVEREXPOSED_VALUE,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_DataMax)));
	m_Profile.SetInt(CProfile::WORK_FORMAT, index_to_bitpix(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_WorkFormat))));
	m_Profile.SetInt(CProfile::BINNING, index_to_binning(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_Binning))));
	m_Profile.SetBool(CProfile::FLIP_H, 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FlipH))!=FALSE);
	m_Profile.SetBool(CProfile::FLIP_V, 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_FlipV))!=FALSE);
	m_Profile.SetInt(CProfile::TIME_OFFSET, 
		(gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_TimeOffset)));
	CmpackBorder border;
	border.left = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_BLeft));
	border.right = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_BRight));
	border.top = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_BTop));
	border.bottom = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_BBottom));
	m_Profile.SetBorder(border);

	m_Profile.SetBool(CProfile::ADVANCED_CALIBRATION,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_AdvCalibration))!=FALSE);

	m_Profile.SetDbl(CProfile::DETECTION_FWHM,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_FWHM)));
	m_Profile.SetDbl(CProfile::DETECTION_THRESHOLD,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Thresh)));
	m_Profile.SetDbl(CProfile::MIN_SHARPNESS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_SharpMin)));
	m_Profile.SetDbl(CProfile::MAX_SHARPNESS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_SharpMax)));
	m_Profile.SetDbl(CProfile::MIN_ROUNDNESS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_RoundMin)));
	m_Profile.SetDbl(CProfile::MAX_ROUNDNESS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_RoundMax)));

	CApertures aper;
	for (int i=0; i<MAX_APERTURES; i++) {
		double radius = gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Aperture[i]));
		if (radius>0)
			aper.Add(CAperture(i+1, radius));
	}
	m_Profile.SetApertures(aper);
	m_Profile.SetDbl(CProfile::SKY_INNER_RADIUS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_SkyIn)));
	m_Profile.SetDbl(CProfile::SKY_OUTER_RADIUS,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_SkyOut)));

	m_Profile.SetBool(CProfile::SPARSE_FIELDS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MatchSparse))!=FALSE);
	m_Profile.SetInt(CProfile::MATCH_READ_STARS,
		(gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MatchRS)));
	m_Profile.SetInt(CProfile::MATCH_IDENT_STARS,
		(gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MatchIS)));
	m_Profile.SetDbl(CProfile::MATCH_CLIP,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MatchClip)));
	m_Profile.SetDbl(CProfile::MAX_OFFSET,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MatchMax)));

	m_Profile.SetBool(CProfile::MULTIPLE_COMP_STARS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_ArtCompStar))!=FALSE);

	m_Profile.SetInt(CProfile::MBIAS_FORMAT, index_to_bitpix(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_MBiasFormat))));

	m_Profile.SetInt(CProfile::MDARK_FORMAT, index_to_bitpix(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_MDarkFormat))));

	m_Profile.SetDbl(CProfile::MFLAT_LEVEL,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_MFlatLevel)));
	m_Profile.SetInt(CProfile::MFLAT_FORMAT, index_to_bitpix(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_MFlatFormat))));

	m_Profile.SetInt(CProfile::KOMBINE_FORMAT, index_to_bitpix(
		gtk_combo_box_get_active(GTK_COMBO_BOX(m_KombineFormat))));

	m_Profile.SetDbl(CProfile::VARFIND_THRESHOLD,
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_VarFindThreshold)));

	CLocation loc = m_Profile.DefaultLocation();

	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	loc.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	loc.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	loc.SetLat(g_strstrip(aux));
	g_free(aux);

	m_Profile.SetDefaultLocation(loc);
}

bool CEditProfileBase::CheckPage(tProfilePageId id, GError **error)
{
	if (id == PAGE_OBSERVER) {
		const gchar *lon = gtk_entry_get_text(GTK_ENTRY(m_Lon));
		const gchar *lat = gtk_entry_get_text(GTK_ENTRY(m_Lat));
		if (*lon!='\0' || *lat!='\0') {
			char buf[64];
			double x, y;
			if (cmpack_strtolon(lon, &x)!=0) {
				g_set_error(error, g_AppError, PAGE_OBSERVER, "Invalid value of the longitude.");
				return false;
			}
			cmpack_lontostr(x, buf, 64);
			gtk_entry_set_text(GTK_ENTRY(m_Lon), buf);
			if (cmpack_strtolat(lat, &y)!=0) {
				g_set_error(error, g_AppError, PAGE_OBSERVER, "Invalid value of the latitude.");
				return false;
			}
			cmpack_lattostr(y, buf, 64);
			gtk_entry_set_text(GTK_ENTRY(m_Lat), buf);
		}
	}

	return true;
}

bool CEditProfileBase::CheckProfile(GError **error)
{
	if (m_Profile.GetDbl(CProfile::SKY_INNER_RADIUS) >= m_Profile.GetDbl(CProfile::SKY_OUTER_RADIUS)) {
		g_set_error(error, g_AppError, PAGE_PHOTOMETRY,
			"Invalid size of the background annulus.\nThe inner radius must be smaller than the outer radius.");
		return false;
	}
	if (m_Profile.GetInt(CProfile::MATCH_READ_STARS) < m_Profile.GetInt(CProfile::MATCH_IDENT_STARS)) {
		g_set_error(error, g_AppError, PAGE_PHOTOMETRY,
			"Invalid parameters in the matching parameters.\nNumber of read stars must be greater than number of identification stars.");
		return false;
	}

	return true;
}

void CEditProfileBase::UpdateControls(void)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_MatchStandard))) {
		gtk_widget_hide(m_MatchBox2);
		gtk_widget_show(m_MatchBox1);
	} else {
		gtk_widget_hide(m_MatchBox1);
		gtk_widget_show(m_MatchBox2);
	}


}

void CEditProfileBase::setdefaults_clicked(GtkWidget *pWidget, CEditProfileBase *pDlg)
{
	pDlg->OnSetDefaultsClicked(pWidget);
}

void CEditProfileBase::OnSetDefaultsClicked(GtkWidget *pButton)
{
	CApertures aper;

	switch (m_currentPageId)
	{
	case PAGE_CAMERA:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RNoise), 
			CProfile::GetDefaultDbl(CProfile::READ_NOISE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_ADCGain), 
			CProfile::GetDefaultDbl(CProfile::ADC_GAIN));
		break;

	case PAGE_SOURCE_FILES:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_DataMin), 
			CProfile::GetDefaultDbl(CProfile::BAD_PIXEL_VALUE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_DataMax), 
			CProfile::GetDefaultDbl(CProfile::OVEREXPOSED_VALUE));
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_WorkFormat), bitpix_to_index(
			CProfile::GetDefaultInt(CProfile::WORK_FORMAT)));
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_Binning), binning_to_index(
			CProfile::GetDefaultInt(CProfile::BINNING)));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FlipH), 
			CProfile::GetDefaultBool(CProfile::FLIP_H));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_FlipV), 
			CProfile::GetDefaultBool(CProfile::FLIP_V));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_TimeOffset), 
			CProfile::GetDefaultInt(CProfile::TIME_OFFSET));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BLeft), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BRight), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BTop), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_BBottom), 0);
		break;

	case PAGE_CALIBRATION:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_StdCalibration), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AdvCalibration), FALSE);
		break;

	case PAGE_STAR_DETECTION:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_FWHM), 
			CProfile::GetDefaultDbl(CProfile::DETECTION_FWHM));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Thresh), 
			CProfile::GetDefaultDbl(CProfile::DETECTION_THRESHOLD));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SharpMin), 
			CProfile::GetDefaultDbl(CProfile::MIN_SHARPNESS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SharpMax), 
			CProfile::GetDefaultDbl(CProfile::MAX_SHARPNESS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RoundMin), 
			CProfile::GetDefaultDbl(CProfile::MIN_ROUNDNESS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_RoundMax), 
			CProfile::GetDefaultDbl(CProfile::MAX_ROUNDNESS));
		break;

	case PAGE_PHOTOMETRY:
		aper = CProfile::DefaultApertures();
		for (int i=0; i<MAX_APERTURES; i++) {
			int index = aper.Find(i+1);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Aperture[i]), aper.GetRadius(index));
		}
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SkyIn), 
			CProfile::GetDefaultDbl(CProfile::SKY_INNER_RADIUS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_SkyOut), 
			CProfile::GetDefaultDbl(CProfile::SKY_OUTER_RADIUS));
		break;

	case PAGE_MATCHING:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchStandard), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_MatchSparse), FALSE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchRS), 
			CProfile::GetDefaultInt(CProfile::MATCH_READ_STARS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchIS), 
			CProfile::GetDefaultInt(CProfile::MATCH_IDENT_STARS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchClip), 
			CProfile::GetDefaultDbl(CProfile::MATCH_CLIP));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MatchMax), 
			CProfile::GetDefaultDbl(CProfile::MAX_OFFSET));
		break;

	case PAGE_LIGHT_CURVE:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_ArtCompStar), 
			CProfile::GetDefaultBool(CProfile::MULTIPLE_COMP_STARS));
		break;

	case PAGE_MASTER_BIAS:
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_MBiasFormat), bitpix_to_index(
			CProfile::GetDefaultInt(CProfile::MBIAS_FORMAT)));
		break;

	case PAGE_MASTER_DARK:
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_MDarkFormat), bitpix_to_index(
			CProfile::GetDefaultInt(CProfile::MDARK_FORMAT)));
		break;

	case PAGE_MASTER_FLAT:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_MFlatLevel), 
			CProfile::GetDefaultDbl(CProfile::MFLAT_LEVEL));
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_MFlatFormat), bitpix_to_index(
			CProfile::GetDefaultInt(CProfile::MFLAT_FORMAT)));
		break;

	case PAGE_MERGE_FRAMES:
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_KombineFormat), bitpix_to_index(
			CProfile::GetDefaultInt(CProfile::KOMBINE_FORMAT)));
		break;

	case PAGE_FIND_VARIABLES:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_VarFindThreshold), 
			CProfile::GetDefaultDbl(CProfile::VARFIND_THRESHOLD));
		break;

	case PAGE_OBSERVER:
		gtk_entry_set_text(GTK_ENTRY(m_LocName), "");
		gtk_entry_set_text(GTK_ENTRY(m_Lat), "");
		gtk_entry_set_text(GTK_ENTRY(m_Lon), "");
		break;

	default:
		break;
	}
}

void CEditProfileBase::button_toggled(GtkWidget *togglebutton, CEditProfileBase *pMe)
{
	pMe->OnButtonToggled(togglebutton);
}

void CEditProfileBase::OnButtonToggled(GtkWidget *togglebutton)
{
	if (!m_Updating) {
		m_Changed = true;
		UpdateControls();
	}
}

void CEditProfileBase::entry_changed(GtkEditable *editable, CEditProfileBase *pMe)
{
	pMe->OnEntryChanged(editable);
}

void CEditProfileBase::OnEntryChanged(GtkEditable *editable)
{
	if (!m_Updating)
		m_Changed = true;
}

gint CEditProfileBase::tree_selection_changed(GtkTreeSelection *widget, CEditProfileBase *pDlg)
{
	pDlg->OnTreeSelectionChanged();
	return FALSE;
}

void CEditProfileBase::OnTreeSelectionChanged(void)
{
	if (!m_Updating) {
		m_Updating = true;
		if (m_currentPageId != EndOfProfilePages) {
			GError *error = NULL;
			if (!CheckPage(m_currentPageId, &error)) {
				ShowError(GTK_WINDOW(DialogWidget()), error->message);
				g_error_free(error);
				return;
			}
		}
		GtkTreeIter iter;
		GtkTreeModel *model;
		tProfilePageId id;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView));
		if (gtk_tree_selection_get_selected(selection, &model, &iter)) 
			gtk_tree_model_get(model, &iter, 0, &id, -1);
		else 
			id = EndOfProfilePages;
		if (id!=m_currentPageId) {
			GtkWidget *page, *defbtn;
			if (id != EndOfProfilePages) {
				gchar *caption, buf[1024];
				gtk_tree_model_get(model, &iter, 1, &caption, 2, &page, 4, &defbtn, -1);
				if (m_CurrentPage)
					gtk_container_remove(GTK_CONTAINER(m_PageFrame), m_CurrentPage);
				m_CurrentPage = page;
				if (m_CurrentPage)
					gtk_container_add(GTK_CONTAINER(m_PageFrame), m_CurrentPage);
				if (caption)
					sprintf(buf, "<span size='large'><b>%s</b></span>", caption);
				else
					buf[0] = '\0';
				g_free(caption);
				if (defbtn) {
					if (m_ReadOnly)
						gtk_widget_hide(defbtn);
					else
						gtk_widget_show(defbtn);
				}
				gtk_label_set_markup(GTK_LABEL(m_PageTitle), buf);
				tProfilePageId previousPage = m_currentPageId;
				m_currentPageId = m_lastPageId = id;
				OnPageChanged(m_currentPageId, previousPage);
			} else {
				if (m_CurrentPage)
					gtk_container_remove(GTK_CONTAINER(m_PageFrame), m_CurrentPage);
				m_CurrentPage = NULL;
				gtk_label_set_text(GTK_LABEL(m_PageTitle), NULL);
				m_currentPageId = EndOfProfilePages;
			}
			UpdateControls();
		}
		m_Updating = false;
	}
}

void CEditProfileBase::ShowPage(tProfilePageId page_id)
{
	PreparePages();

	m_Updating = true;
	tFindData data(page_id);
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_PageView));
	if (model)
		gtk_tree_model_foreach(model, (GtkTreeModelForeachFunc)find_page_proc, &data);
	if (data.path) {
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_PageView));
		gtk_tree_selection_select_path(selection, data.path);
		gtk_tree_path_free(data.path);
	}
	m_Updating = false;

	OnTreeSelectionChanged();
}

void CEditProfileBase::SetPageTitle(tProfilePageId page_id, const gchar *caption)
{
	PreparePages();

	tFindData data(page_id);
	gtk_tree_model_foreach(GTK_TREE_MODEL(m_PageList), (GtkTreeModelForeachFunc)find_page_proc, &data);
	if (data.path) {
		GtkTreeIter iter;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(m_PageList), &iter, data.path);
		gtk_tree_store_set(m_PageList, &iter, 1, caption, -1);
		if (m_currentPageId == page_id) {
			char buf[1024];
			if (caption)
				sprintf(buf, "<span size='large'><b>%s</b></span>", caption);
			else
				buf[0] = '\0';
			gtk_label_set_markup(GTK_LABEL(m_PageTitle), buf);
		}
		gtk_tree_path_free(data.path);
	}
}

void CEditProfileBase::button_clicked(GtkWidget *widget, CEditProfileBase *pDlg)
{
	pDlg->OnButtonClicked(widget);
}

void CEditProfileBase::OnButtonClicked(GtkWidget *pButton)
{
	if (pButton==m_LocBtn)
		EditLocation();
}

void CEditProfileBase::combo_box_changed(GtkWidget *widget, CEditProfileBase *pDlg)
{
	pDlg->OnComboBoxChanged(widget);
}

//
// Edit observer's coordinates
//
void CEditProfileBase::EditLocation(void)
{
	CLocation loc = m_Profile.DefaultLocation();

	gchar *aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_LocName)));
	loc.SetName(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lon)));
	loc.SetLon(g_strstrip(aux));
	g_free(aux);

	aux = g_strdup(gtk_entry_get_text(GTK_ENTRY(m_Lat)));
	loc.SetLat(g_strstrip(aux));
	g_free(aux);

	CLocationDlg dlg(GTK_WINDOW(DialogWidget()));
	if (!dlg.Execute(&loc, true)) 
		return;

	m_Profile.SetDefaultLocation(loc);
	m_Changed = true;
	m_Updating = true;
	SetData();
	m_Updating = false;
	UpdateControls();
}

tProfilePageId CEditProfileBase::str_to_page(const char *str, tProfilePageId defaultValue)
{
	if (str && *str!='\0') {
		for (int i=0; i<EndOfProfilePages; i++) {
			if (strcmp(PageIds[i], str)==0)
				return (tProfilePageId)i;
		}
	}
	return defaultValue;
}

const char *CEditProfileBase::page_to_str(tProfilePageId page)
{
	if (page>=0 && page<EndOfProfilePages)
		return PageIds[page];
	return NULL;
}

void CEditProfileBase::SetProfile(const CProfile &profile, bool readOnly)
{
	m_Profile = profile;
	m_Changed = false;
	m_ProjectType = profile.ProjectType();
	SetReadOnly(readOnly);

	m_Updating = true;
	PreparePages();
	SetData();
	m_Updating = false;

	tProfilePageId initPage = EndOfProfilePages;
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_PageView), NULL);
	if (m_ProjectType != EndOfProjectTypes) {
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(m_FilteredPageList));
		gtk_tree_view_set_model(GTK_TREE_VIEW(m_PageView), m_FilteredPageList);
		gtk_tree_view_expand_all(GTK_TREE_VIEW(m_PageView));
		if (m_lastPageId == EndOfProfilePages || !IsPageVisible(m_lastPageId)) {
			initPage = EndOfProfilePages;
			GtkTreeIter iter;
			gboolean ok = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_PageList), &iter);
			while (ok) {
				tProfilePageId id;
				gtk_tree_model_get(GTK_TREE_MODEL(m_PageList), &iter, 0, &id, -1);
				if (IsPageVisible(id)) {
					initPage = id;
					break;
				}
				ok = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_PageList), &iter);
			}
		} else
			initPage = m_lastPageId;
	}

	ShowPage(initPage);
	UpdateControls();
}

void CEditProfileBase::SetReadOnly(bool readOnly)
{
	if (m_ReadOnly != readOnly) {
		m_ReadOnly = readOnly;
		if (m_currentPageId != EndOfProfilePages) {
			tFindData data(m_currentPageId);
			gtk_tree_model_foreach(GTK_TREE_MODEL(m_PageList), (GtkTreeModelForeachFunc)find_page_proc, &data);
			if (data.path) {
				GtkTreeIter iter;
				gtk_tree_model_get_iter(GTK_TREE_MODEL(m_PageList), &iter, data.path);
				GtkWidget *defbtn;
				gtk_tree_model_get(GTK_TREE_MODEL(m_PageList), &iter, 4, &defbtn, -1);
				if (defbtn) {
					if (m_ReadOnly)
						gtk_widget_hide(defbtn);
					else
						gtk_widget_show(defbtn);
				}
			}
		}
		UpdateControls();
	}
}

gboolean CEditProfileBase::find_page_proc(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, tFindData *data)
{
	int id;
	gtk_tree_model_get(model, iter, 0, &id, -1);
	if (id == data->page_id) {
		data->path = gtk_tree_path_copy(path);
		return TRUE;
	}
	return FALSE;
}

GtkWidget *CEditProfileBase::new_label(const gchar *text, gfloat xalign)
{
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), text);
	gtk_misc_set_alignment(GTK_MISC(label), xalign, 0.5);
	return label;
}

GtkWidget *CEditProfileBase::add_label(GtkTable *tbox, gint row, const gchar *text)
{
	GtkWidget *label = new_label(text, 0);
	gtk_table_attach(tbox, label, 0, 3, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	return label;
}

GtkWidget *CEditProfileBase::add_entry(GtkTable *tbox, gint row, const gchar *caption, gint maxlen, const gchar *tooltip)
{
	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_tooltip_text(entry, tooltip);
	gtk_entry_set_max_length(GTK_ENTRY(entry), maxlen);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach(tbox, new_label(caption), 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(tbox, entry, 1, 2, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	return entry;
}

GtkWidget *CEditProfileBase::add_push_button(GtkTable *tbox, gint row, const gchar *label, 
	const gchar *tooltip)
{
	GtkWidget *button = gtk_button_new_with_label(label);
	gtk_widget_set_tooltip_text(button, tooltip);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), this);
	gtk_table_attach(GTK_TABLE(tbox), button, 2, 3, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	return button;
}

GtkWidget *CEditProfileBase::add_static(GtkTable *tbox, gint row, const gchar *caption, 
	PangoEllipsizeMode ellipsize, const gchar *tooltip)
{
	GtkWidget *label = new_label(caption);
	gtk_table_attach(tbox, label, 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	GtkWidget *entry = new_label(NULL);
	gtk_widget_set_tooltip_text(entry, tooltip);
	gtk_label_set_ellipsize(GTK_LABEL(entry), ellipsize);
	gtk_table_attach(tbox, entry, 1, 2, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
	return entry;
}

GtkWidget *CEditProfileBase::add_spin_button(GtkTable *tbox, int left, int top, 
	double dmin, double dmax, double step, int digits, const gchar *tooltip)
{
	GtkObject *adj = gtk_adjustment_new(0, dmin, dmax, step, 10*step, 0);
	GtkWidget *spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, digits);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
	gtk_widget_set_tooltip_text(spin, tooltip);
	g_signal_connect(G_OBJECT(spin), "changed", G_CALLBACK(entry_changed), this);
	gtk_table_attach(tbox, spin, left, left+1, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	return spin;
}

GtkWidget *CEditProfileBase::add_spin_button_dbl(GtkTable *tbox, int top, const gchar *caption, 
	CProfile::tParameter p, double step, int digits, const gchar *tooltip)
{
	double dmin, dmax, defval;
	gchar buf[512];

	GtkWidget *label = new_label(caption);
	gtk_table_attach(tbox, label, 0, 1, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	CProfile::GetLimitsDbl(p, &dmin, &dmax);
	GtkWidget *widget = add_spin_button(tbox, 1, top, dmin, dmax, step, digits, tooltip);
	defval = CProfile::GetDefaultDbl(p);
	sprintf(buf, "Default: %.*f", digits, defval);
	gtk_table_attach(tbox, new_label(buf), 2, 3, top, top+1, 
		GtkAttachOptions(GTK_FILL), GTK_FILL, 0, 0);
	return widget;
}

GtkWidget *CEditProfileBase::add_spin_button_int(GtkTable *tbox, int top, const gchar *caption,
	CProfile::tParameter p, int step, const gchar *tooltip)
{
	int imin, imax, defval;
	gchar buf[512];

	GtkWidget *label = new_label(caption);
	gtk_table_attach(tbox, label, 0, 1, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	CProfile::GetLimitsInt(p, &imin, &imax);
	GtkWidget *widget = add_spin_button(tbox, 1, top, imin, imax, step, 0, tooltip);
	defval = CProfile::GetDefaultInt(p);
	sprintf(buf, "Default: %d", defval);
	gtk_table_attach(tbox, new_label(buf), 2, 3, top, top+1, 
		GtkAttachOptions(GTK_FILL), GTK_FILL, 0, 0);
	return widget;
}

GtkWidget *CEditProfileBase::add_combo_box(GtkTable *tbox, int top, const gchar *caption, 
	const tStringList *strings, const gchar *tooltip)
{
	GtkWidget *label = new_label(caption);
	gtk_table_attach(tbox, label, 0, 1, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	GtkWidget *cbox = gtk_combo_box_new_text();
	for (int i=0; strings[i].str!=NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbox), strings[i].str);
	gtk_widget_set_tooltip_text(cbox, tooltip);
	g_signal_connect(G_OBJECT(cbox), "changed", G_CALLBACK(combo_box_changed), this);
	gtk_table_attach(tbox, cbox, 1, 2, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	return cbox;
}

GtkWidget *CEditProfileBase::add_combo_box(GtkTable *tbox, int top, const gchar *caption, 
	AddComboFn fn, int start, int end, const gchar *tooltip)
{
	const gchar *str;

	GtkWidget *label = new_label(caption);
	gtk_table_attach(tbox, label, 0, 1, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	GtkWidget *cbox = gtk_combo_box_new_text();
	for (int i=start; i<end && (str=fn(i))!=NULL; i++) 
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbox), str);
	gtk_widget_set_tooltip_text(cbox, tooltip);
	g_signal_connect(G_OBJECT(cbox), "changed", G_CALLBACK(combo_box_changed), this);
	gtk_table_attach(tbox, cbox, 1, 2, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	return cbox;
}

GtkWidget *CEditProfileBase::add_radio_btn(GtkTable *tbox, int top, const gchar *caption, 
	GSList **group, const gchar *tooltip)
{
	GtkWidget *btn = gtk_radio_button_new_with_label(*group, caption);
	gtk_widget_set_tooltip_text(btn, tooltip);
	gtk_table_attach(tbox, btn, 0, 3, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	if (*group == NULL)
		*group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(btn));
	g_signal_connect(G_OBJECT(btn), "toggled", G_CALLBACK(button_toggled), this);
	return btn;
}

GtkWidget *CEditProfileBase::add_check_btn(GtkTable *tbox, int top, const gchar *caption, const gchar *tooltip)
{
	GtkWidget *btn = gtk_check_button_new_with_label(caption);
	gtk_widget_set_tooltip_text(btn, tooltip);
	g_signal_connect(G_OBJECT(btn), "toggled", G_CALLBACK(button_toggled), this);
	gtk_table_attach(tbox, btn, 0, 3, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	return btn;
}

GtkWidget *CEditProfileBase::add_button_box(GtkTable *tbox, int top)
{
	GtkWidget *bbox = gtk_vbutton_box_new();
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 8);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
	gtk_table_attach(tbox, bbox, 0, 2, top, top+1, GTK_FILL, GTK_FILL, 0, 0);
	return bbox;
}
