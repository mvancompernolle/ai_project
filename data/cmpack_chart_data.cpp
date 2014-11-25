/**************************************************************

cmpack_chart_data.cpp (C-Munipack project)
Object which holds data for a chart
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

$Id: cmpack_chart_data.cpp,v 1.6 2014/08/31 12:39:43 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glib/gstdio.h>

#include "cmpack_chart_data.h"

enum {
	OBJECT_INSERTED, 
	OBJECT_UPDATED,
	OBJECT_DELETED,
	DIM_CHANGED,
	DATA_CLEARED,
	LAST_SIGNAL
}; 

/*--------------------   CHART DATA CLASS   ----------------------------*/

G_DEFINE_TYPE(CmpackChartData, cmpack_chart_data, G_TYPE_OBJECT)

static guint cmpack_chart_data_signals[LAST_SIGNAL] = { 0 }; 

static void cmpack_chart_data_finalize(GObject *object);

/* Class initialization */
static void cmpack_chart_data_class_init(CmpackChartDataClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;

    /* Override object destroy */
	object_class->finalize = cmpack_chart_data_finalize;

	/* Signals */
	cmpack_chart_data_signals[OBJECT_INSERTED] = g_signal_new("object-inserted",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackChartDataClass, object_inserted), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 

	cmpack_chart_data_signals[OBJECT_UPDATED] = g_signal_new("object-updated",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackChartDataClass, object_updated), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 

	cmpack_chart_data_signals[OBJECT_DELETED] = g_signal_new("object-deleted",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackChartDataClass, object_deleted), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 
	
	cmpack_chart_data_signals[DIM_CHANGED] = g_signal_new("dim-changed",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackChartDataClass, dim_changed), NULL, NULL, 
		  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0); 

	cmpack_chart_data_signals[DATA_CLEARED] = g_signal_new("data-cleared",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackChartDataClass, data_cleared), NULL, NULL, 
		  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Instance initialization */
static void cmpack_chart_data_init(CmpackChartData *data)
{
}

/* Free allocated memory */
static void cmpack_chart_data_finalize(GObject *object)
{
	gint i;
	CmpackChartData *data = CMPACK_CHART_DATA(object);

	/* Free row tags */
	for (i=0; i<data->count; i++) {
		g_free(data->items[i]->tag);
		g_free(data->items[i]);
	}
	g_free(data->items);
	data->items = NULL;
	data->count = data->capacity = 0;
	data->height = data->width = 0;

    /* Chain up to the parent class */
    G_OBJECT_CLASS(cmpack_chart_data_parent_class)->finalize(object);
}

/* -----------------------------   Public API ------------------------------------ */

/* Create chart widget */
CmpackChartData *cmpack_chart_data_new()
{
	return (CmpackChartData*)g_object_new(CMPACK_TYPE_CHART_DATA, NULL);
}

/* Create chart widget */
CmpackChartData *cmpack_chart_data_new_with_alloc(gint capacity)
{
	CmpackChartData *res = cmpack_chart_data_new();
	cmpack_chart_data_alloc(res, capacity);
	return res;
}

/* Clear data and set number of channels and initial capacity */
void cmpack_chart_data_alloc(CmpackChartData *data, gint capacity)
{
	g_return_if_fail(data != NULL);

	capacity = MAX(capacity, data->count);
	if (capacity != data->capacity) {
		if (capacity>0) {
			data->items = (CmpackChartItem**)g_realloc(data->items, capacity*sizeof(CmpackChartItem*));
		} else {
			g_free(data->items);
			data->items = NULL;
		}
		data->capacity = capacity;
	}
}

/* Clear all data */
void cmpack_chart_data_clear(CmpackChartData *data)
{
	gint i;

	g_return_if_fail(data != NULL);

	if (data->count>0 || data->width>0 || data->height>0) {
		for (i=0; i<data->count; i++) {
			g_free(data->items[i]->tag);
			g_free(data->items[i]);
		}
		g_free(data->items);
		data->items = NULL;
		data->count = data->capacity = 0;
		data->width = data->height = 0;
		g_signal_emit(data, cmpack_chart_data_signals[DATA_CLEARED], 0);
	}
}

/* Set allocated space for objects */
void cmpack_chart_data_set_dimensions(CmpackChartData *data, gint width, gint height)
{
	g_return_if_fail(data != NULL);

	if (data->width!=width || data->height!=height) {
		data->width = width;
		data->height = height;
		g_signal_emit(data, cmpack_chart_data_signals[DIM_CHANGED], 0);
	}
}

/* Get chart width in pixels */
gint cmpack_chart_data_width(CmpackChartData *data)
{
	g_return_val_if_fail(data != NULL, 0);

	return data->width;
}

/* Get chart height in pixels */
gint cmpack_chart_data_height(CmpackChartData *data)
{
	g_return_val_if_fail(data != NULL, 0);

	return data->height;
}

/* Get number of objects */
gint cmpack_chart_data_count(CmpackChartData *data)
{
	g_return_val_if_fail(data != NULL, 0);

	return data->count;
}

/* Add new data to the chart */
gint cmpack_chart_data_add(CmpackChartData *data, const CmpackChartItem *d, gsize length)
{
	gint index;
	CmpackChartItem *item;

	g_return_val_if_fail(data != NULL, -1);

	if (data->count>=data->capacity) {
		data->capacity += 64;
		data->items = (CmpackChartItem**)g_realloc(data->items, data->capacity*sizeof(CmpackChartItem*));
	}
	index = data->count++;

	item = (CmpackChartItem*)g_malloc0(sizeof(CmpackChartItem));
	memcpy(item, d, MIN(sizeof(CmpackChartItem), length));
	item->tag = (d->tag!=NULL ? g_strdup(d->tag) : NULL);
	data->items[index] = item;
	
	g_signal_emit(data, cmpack_chart_data_signals[OBJECT_INSERTED], 0, index);
	return index;
}

/* Set values */
void cmpack_chart_data_set_center(CmpackChartData *data, gint row, gdouble x, gdouble y)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->x!=x || item->y!=y) {
		item->x = x;
		item->y = y;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Set values */
void cmpack_chart_data_set_diameter(CmpackChartData *data, gint row, gdouble diameter)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->d!=diameter) {
		item->d = diameter;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Set values */
void cmpack_chart_data_set_outline(CmpackChartData *data, gint row, gboolean outline)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->outline!=outline) {
		item->outline = outline;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Unset values */
void cmpack_chart_data_set_color(CmpackChartData *data, gint row, CmpackColor color)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->color!=color) {
		item->color = color;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Set sensitivity */
void cmpack_chart_data_set_sensitivity(CmpackChartData *data, gint row, gboolean sensitive)
{
	CmpackChartItem *item;

	gboolean disabled = sensitive==0;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->disabled != disabled) {
		item->disabled = disabled;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Set topmost flag */
void cmpack_chart_data_set_topmost(CmpackChartData *data, gint row, gboolean topmost)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->topmost!=topmost) {
		item->topmost = topmost;
		g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
	}
}

/* Set row tag */
void cmpack_chart_data_set_tag(CmpackChartData *data, gint row, const gchar *text)
{
	CmpackChartItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (text) {
		if (!item->tag || strcmp(item->tag, text)!=0) {
			g_free(item->tag); 
			item->tag = g_strdup(text);
			g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
		}
	} else {
		if (item->tag!=NULL) {
			g_free(item->tag); 
			item->tag = NULL;
			g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, row);
		}
	}
}

/* Clear all tags */
void cmpack_chart_data_clear_tags(CmpackChartData *data)
{
	gint i;

	g_return_if_fail(data != NULL);

	for (i=0; i<data->count; i++) {
		CmpackChartItem *item = data->items[i];
		if (item->tag) {
			g_free(item->tag); 
			item->tag = NULL;
			g_signal_emit(data, cmpack_chart_data_signals[OBJECT_UPDATED], 0, i);
		}
	}
}

/* Get row tag */
CmpackChartItem *cmpack_chart_data_get_item(CmpackChartData *data, gint row)
{
	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(row>=0 && row<data->count, NULL);

	return data->items[row];
}

/* Get row tag */
const gchar *cmpack_chart_data_get_tag(CmpackChartData *data, gint row)
{
	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(row>=0 && row<data->count, NULL);

	return data->items[row]->tag;
}

/* Get object's identifier */
intptr_t cmpack_chart_data_get_param(CmpackChartData *data, gint row)
{
	g_return_val_if_fail(data != NULL, 0);
	g_return_val_if_fail(row>=0 && row<data->count, 0);

	return data->items[row]->param;
}

/* Get object's position */
gboolean cmpack_chart_data_get_center(CmpackChartData *data, gint row, gdouble *x, double *y)
{
	g_return_val_if_fail(data != NULL, -1);
	g_return_val_if_fail(row>=0 && row<data->count, -1);

	if (x)
		*x = data->items[row]->x;
	if (y)
		*y = data->items[row]->y;
	return TRUE;
}


/* Get object's complete record */
const CmpackChartItem *cmpack_chart_data_get_object(CmpackChartData *data, gint row)
{
	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(row>=0 && row<data->count, FALSE);

	return data->items[row];
}

/* Find object by its identifier */
gint cmpack_chart_data_find_item(CmpackChartData *data, intptr_t param)
{
	gint i;

	g_return_val_if_fail(data != NULL, -1);

	for (i=0; i<data->count; i++) {
		if (data->items[i]->param == param)
			return i;
	}
	return -1;
}

static void draw_star(cairo_t *cr, double kx, double dx, double ky, double dy, 
	const CmpackChartItem *star, gboolean invert)
{
	gdouble r;

	switch (star->color)
	{
	case CMPACK_COLOR_RED:
		cairo_set_source_rgb(cr, 0.7, 0, 0);
		break;
	case CMPACK_COLOR_GREEN:
		cairo_set_source_rgb(cr, 0, 0.6, 0);
		break;
	case CMPACK_COLOR_BLUE:
		cairo_set_source_rgb(cr, 0, 0.4, 1.0);
		break;
	case CMPACK_COLOR_YELLOW:
		cairo_set_source_rgb(cr, 0.7, 0.6, 0.0);
		break;
	case CMPACK_COLOR_GRAY:
		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
		break;
	default:
		if (!invert)
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		else
			cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		break;
	}
	cairo_new_sub_path(cr);
	r = star->d * sqrt(0.5*(fabs(kx)+fabs(ky)));
	cairo_arc(cr, star->x*kx + dx, star->y*ky + dy, r, 0.0, 2.0*M_PI);
	if (!star->outline) {
		cairo_fill_preserve(cr);
		if (!invert) 
			cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
		else
			cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	}
	cairo_stroke(cr);
}

static gchar **split_lines(const gchar *text)
{
	gint count;
	const gchar *sptr, *start;
	gchar **list, **dptr;

	if (!text)
		return NULL;

	count = 1;
	sptr = text + strcspn(text, "\r\n");
	while (*sptr!='\0') {
		count++;
		if (sptr[0]=='\r' && sptr[1]=='\n')
			sptr++;
		sptr++;
		sptr += strcspn(sptr, "\r\n");
	}

	list = (gchar**)g_malloc((count+1)*sizeof(gchar*));
	dptr = list;
	start = text;
	sptr = start + strcspn(start, "\r\n");
	gchar *buf = (gchar*)g_malloc((sptr-start+1)*sizeof(gchar));
	memcpy(buf, start, (sptr-start)*sizeof(gchar));
	buf[sptr-start] = '\0';
	*dptr++ = buf;
	while (*sptr!='\0') {
		if (sptr[0]=='\r' && sptr[1]=='\n')
			sptr++;
		sptr++;
		start = sptr;
		sptr += strcspn(sptr, "\r\n");
		gchar *buf = (gchar*)g_malloc((sptr-start+1)*sizeof(gchar));
		memcpy(buf, start, (sptr-start)*sizeof(gchar));
		buf[sptr-start] = '\0';
		*dptr++ = buf;
	}
	*dptr = NULL;
	return list;
}

static void draw_tag(cairo_t *cr, double kx, double dx, double ky, double dy, 
	const CmpackChartItem *star, gboolean invert)
{
	gint i, x, y, line_height, line_spacing;
	gdouble r;
	gchar **line;
	
	gchar **strv = split_lines(star->tag);
	if (!strv)
		return;

	gint count = 0;
	for (line=strv; *line!=NULL; line++)
		count++;
	if (count==0)
		return;

	// Line height
	cairo_text_extents_t extents;
	cairo_text_extents(cr, "X", &extents);
	line_height = (gint)ceil(extents.height);
	line_spacing = line_height/2;
	
	// Compute size of the paragraph
	gint width = 0, height = 0;
	for (line=strv, i=0; *line!=NULL; line++, i++) {
		cairo_text_extents(cr, star->tag, &extents);
		height += line_height;
		width = MAX(width, (gint)ceil(extents.width));
	}
	height += (count-1)*line_spacing;

	r = star->d * sqrt(0.5*(kx+ky));
	x = (int)(star->x*kx + dx + 0.71*r + 2);
	y = (int)(star->y*ky + dy + 0.71*r + 2);
	switch (star->color)
	{
	case CMPACK_COLOR_RED:
		cairo_set_source_rgb(cr, 0.7, 0, 0);
		break;
	case CMPACK_COLOR_GREEN:
		cairo_set_source_rgb(cr, 0, 0.6, 0);
		break;
	case CMPACK_COLOR_BLUE:
		cairo_set_source_rgb(cr, 0, 0.4, 1.0);
		break;
	case CMPACK_COLOR_YELLOW:
		cairo_set_source_rgb(cr, 0.7, 0.6, 0.0);
		break;
	case CMPACK_COLOR_GRAY:
		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
		break;
	default:
		if (!invert)
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		else
			cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		break;
	}
	for (line=strv, i=0; *line!=NULL; line++, i++) {
		cairo_move_to(cr, x, y + line_height);
		cairo_show_text(cr, *line);
		y += line_height + line_spacing;
	}
	g_strfreev(strv);
}

static void render_to_surface(CmpackChartData *data, CmpackImageData *image,
	cairo_surface_t *surface, gboolean negative, CmpackOrientation orientation)
{
	gint i, width, height, img_width, img_height;
	cairo_surface_t *cs;
	cairo_pattern_t *pat;
	cairo_matrix_t mx;
	cairo_t *cr = cairo_create(surface);

	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);

	// Draw background
	if (image) {
		img_width = cmpack_image_data_width(image);
		img_height = cmpack_image_data_height(image);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		cs = cmpack_image_data_get_surface(image);
		pat = cairo_pattern_create_for_surface(cs);
		cairo_pattern_set_filter(pat, CAIRO_FILTER_BEST);
		cairo_matrix_init_identity(&mx);
		cairo_matrix_scale(&mx, (gdouble)img_width/width, (gdouble)img_height/height);
		cairo_pattern_set_matrix(pat, &mx);
		cairo_set_source(cr, pat);
		cairo_paint(cr);
		cairo_pattern_destroy(pat);
	} else {
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		if (!negative)
			cairo_set_source_rgb(cr, 0.1, 0.0, 0.2);
		else
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);
	}

	if (data->count>0 && data->height>0 && data->width>0) {
		gdouble kx, ky, dx, dy;
		dx = 0;
		kx = (gdouble)width/data->width;
		if (orientation & CMPACK_ROWS_UPWARDS) {
			dy = height;
			ky = (gdouble)-height/data->height; 
		} else {
			dy = 0;
			ky = (gdouble)height/data->height; 
		}
		// Draw all stars
		cairo_set_line_width(cr, 1);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
		for (i=0; i<data->count; i++) {
			if (data->items[i]->d>0)
				draw_star(cr, kx, dx, ky, dy, data->items[i], negative);
		}
		// Draw labels
		cairo_set_font_size(cr, 12.0 * sqrt(0.5*(fabs(kx)+fabs(ky))));
		cairo_select_font_face(cr, "", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
		for (i=0; i<data->count; i++) {
			if (data->items[i]->tag) 
				draw_tag(cr, kx, dx, ky, dy, data->items[i], negative);
		}
	}
	cairo_destroy(cr);
}

// Open a file (file name is UTF-8 encoded)
static FILE *open_file(const gchar *fpath, const gchar *mode)
{
	FILE *f;
	gchar *lp;
	
	if (fpath) {
		lp = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
		f = fopen(lp, mode);
		g_free(lp);
		return f;
	}
	return NULL;
}

// Write data to file
static cairo_status_t cairo_write_fn(void *closure, const unsigned char *data, unsigned int length)
{
	if (fwrite(data, length, 1, (FILE*)closure)!=1)
		return CAIRO_STATUS_WRITE_ERROR;
	else
		return CAIRO_STATUS_SUCCESS;
}

static gboolean write_to_png(cairo_surface_t *surface, const gchar *filepath)
{
	gboolean res;

	FILE *f = open_file(filepath, "wb");
	if (!f)
		return FALSE;

	res = cairo_surface_write_to_png_stream(surface, cairo_write_fn, f)==CAIRO_STATUS_SUCCESS;
	if (!res) {
		fclose(f);
		g_unlink(filepath);
		return FALSE;
	}
	
	fclose(f);
	return TRUE;
}

static gboolean write_to_jpeg(cairo_surface_t *surface, const gchar *filepath,
	gint quality)
{
	gboolean res;
	guchar *dptr, *pixels;
	gchar aux[64];
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

/* Export chart to a file */
gboolean cmpack_chart_data_write_to_file(CmpackChartData *data, CmpackImageData *image,
	const gchar *filepath, const gchar *format, gint width, gint height, gboolean negative,
	CmpackOrientation orientation, gint jpeg_quality)
{
	gboolean res;
	cairo_surface_t *surface;

	g_return_val_if_fail(filepath != NULL, FALSE);
	g_return_val_if_fail(format != NULL, FALSE);
	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(width>0 && height>0, FALSE);

	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
	render_to_surface(data, image, surface, negative, orientation);
	if (strcmp(format, "image/png")==0)
		res = write_to_png(surface, filepath);
	else if (strcmp(format, "image/jpeg")==0)
		res = write_to_jpeg(surface, filepath, jpeg_quality);
	else
		res = FALSE;
	cairo_surface_destroy(surface);
	return res;
}
