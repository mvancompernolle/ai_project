/**************************************************************

cmpack_image_data.cpp (C-Munipack project)
Object which holds data for a image
Copyright (C) 2011 David Motl, dmotl@volny.cz

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

$Id: cmpack_image_data.cpp,v 1.3 2014/08/31 12:39:43 dmotl Exp $

**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cmpack_image_data.h"

enum {
	DATA_CHANGED,
	LAST_SIGNAL
}; 

/*--------------------   IMAGE DATA CLASS   ----------------------------*/

G_DEFINE_TYPE(CmpackImageData, cmpack_image_data, G_TYPE_OBJECT)

static guint cmpack_image_data_signals[LAST_SIGNAL] = { 0 }; 

static void cmpack_image_data_finalize(GObject *object);

/* Class initialization */
static void cmpack_image_data_class_init(CmpackImageDataClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;

    /* Override object destroy */
	object_class->finalize = cmpack_image_data_finalize;

	/* Signals */
	cmpack_image_data_signals[DATA_CHANGED] = g_signal_new("data-changed",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET(CmpackImageDataClass, data_changed), NULL, NULL, 
		  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Instance initialization */
static void cmpack_image_data_init(CmpackImageData *data)
{
}

/* Free allocated memory */
static void cmpack_image_data_finalize(GObject *object)
{
	CmpackImageData *data = CMPACK_IMAGE_DATA(object);

	if (data->data) {
		cairo_surface_destroy(data->data);
		data->data = NULL;
	}

    /* Chain up to the parent class */
    G_OBJECT_CLASS(cmpack_image_data_parent_class)->finalize(object);
}

/* -----------------------------   Public API ------------------------------------ */

/* Create image widget */
CmpackImageData *cmpack_image_data_new(cairo_format_t format, gint width, gint height)
{
	CmpackImageData *res = (CmpackImageData*)g_object_new(CMPACK_TYPE_IMAGE_DATA, NULL);
	if (width>0 && height>0 && cairo_format_stride_for_width(format, width)>0)
		res->data = cairo_image_surface_create(format, width, height);
	return res;
}

/* Get image width in pixels */
gint cmpack_image_data_width(CmpackImageData *data)
{
	g_return_val_if_fail(data != NULL, 0);

	if (data->data)
		return cairo_image_surface_get_width(data->data);
	return 0;
}

/* Get image height in pixels */
gint cmpack_image_data_height(CmpackImageData *data)
{
	g_return_val_if_fail(data != NULL, 0);

	if (data->data)
		return cairo_image_surface_get_height(data->data);
	return 0;
}

/* Get pointer to image surface */
cairo_surface_t *cmpack_image_data_get_surface(CmpackImageData *data)
{
	g_return_val_if_fail(data != NULL, NULL);

	return data->data;
}

/* Get pointer to image surface */
void cmpack_image_data_changed(CmpackImageData *data)
{
	g_return_if_fail(data != NULL);

	g_signal_emit(data, cmpack_image_data_signals[DATA_CHANGED], 0);	
}

/* Save cairo surface to jpeg */
static gboolean write_to_jpeg(cairo_surface_t *surface, const gchar *filepath,
	gint quality)
{
	gboolean res;
	gchar aux[64];
	guchar *dptr, *pixels;
	gint x, y, width, height, row_stride;
	GdkPixbuf *buf;

	uint32_t *sptr = (uint32_t*)cairo_image_surface_get_data(surface);
	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	buf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
	pixels = gdk_pixbuf_get_pixels(buf);
	row_stride = gdk_pixbuf_get_rowstride(buf);
	for (y=0; y<height; y++) {
		dptr = pixels + y*row_stride;
		for (x=0; x<width; x++) {
			uint32_t value = *sptr++;
			*dptr++ = (value >> 16) & 0xFF;
			*dptr++ = (value >> 8) & 0xFF;
			*dptr++ = (value) & 0xFF;
		}
	}
	sprintf(aux, "%d", quality);
	res = gdk_pixbuf_save(buf, filepath, "jpeg", NULL, "quality", aux, NULL);
	g_object_unref(buf);
	return res;
}

/* Export image into a file in PNG format */
gboolean cmpack_image_data_write_to_file(CmpackImageData *data, const gchar *filepath, 
	const gchar *format, gint width, gint height, gint jpeg_quality)
{
	gboolean res;
	gint src_width, src_height;
	cairo_t *cr;
	cairo_surface_t *surface;
	cairo_pattern_t *pat;
	cairo_matrix_t mx;

	g_return_val_if_fail(filepath != NULL, FALSE);
	g_return_val_if_fail(format != NULL, FALSE);
	g_return_val_if_fail(data && data->data, FALSE);
	g_return_val_if_fail(width>0 && height>0, FALSE);

	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);

	pat = cairo_pattern_create_for_surface(data->data);
	cairo_pattern_set_filter(pat, CAIRO_FILTER_BEST);
	src_width = cairo_image_surface_get_width(data->data);
	src_height = cairo_image_surface_get_height(data->data);

	cairo_matrix_init_identity(&mx);
	cairo_matrix_scale(&mx, (gdouble)src_width/width, (gdouble)src_height/height);
	cairo_pattern_set_matrix(pat, &mx);

	cr = cairo_create(surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source(cr, pat);
	cairo_paint(cr);
	cairo_destroy(cr);
	cairo_pattern_destroy(pat);
	
	if (strcmp(format, "image/png")==0)
		res = cairo_surface_write_to_png(surface, filepath)==CAIRO_STATUS_SUCCESS;
	else if (strcmp(format, "image/jpeg")==0)
		res = write_to_jpeg(surface, filepath, jpeg_quality);
	else
		res = FALSE;
	cairo_surface_destroy(surface);
	return res;
}
