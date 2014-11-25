/**************************************************************

cmpack_preview.cpp (C-Munipack project)
Widget which can draw a text, graph, chart or image
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

$Id: cmpack_preview.cpp,v 1.5 2013/12/27 10:02:51 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gdk/gdkkeysyms.h>

#include "cmpack_preview.h"
#include "cmpack_image_data.h"
#include "cmpack_chart_data.h"
#include "cmpack_graph_data.h"

/* Properties */
enum {
	PROP_0,
	PROP_MODEL,
	PROP_TEXT,
	PROP_NEGATIVE
};

/* Internal colors */
enum {
	PREVIEW_COLOR_BACKGROUND,
	PREVIEW_COLOR_RED,
	PREVIEW_COLOR_GREEN,
	PREVIEW_COLOR_BLUE,
	PREVIEW_COLOR_YELLOW,
	PREVIEW_N_COLORS
};

/* GObject vfuncs */
static void             cmpack_preview_set_property              (GObject            *object,
								 guint               prop_id,
								 const GValue       *value,
								 GParamSpec         *pspec);
static void             cmpack_preview_get_property              (GObject            *object,
								 guint               prop_id,
								 GValue             *value,
								 GParamSpec         *pspec);
static void             cmpack_preview_finalize                  (GObject          *object); 

/* GtkObject vfuncs */
static void             cmpack_preview_destroy                   (GtkObject          *object);
/* GtkWidget vfuncs */
static void             cmpack_preview_realize                   (GtkWidget          *widget);
static void             cmpack_preview_unrealize                 (GtkWidget          *widget);
static void             cmpack_preview_style_set                 (GtkWidget        *widget,
						                 GtkStyle         *previous_style);
static void             cmpack_preview_state_changed             (GtkWidget        *widget,
			                                         GtkStateType      previous_state);
static void             cmpack_preview_size_allocate             (GtkWidget          *widget,
								 GtkAllocation      *allocation);
static gboolean         cmpack_preview_expose                    (GtkWidget          *widget,
								 GdkEventExpose     *expose);

/* Internal functions */
static void             cmpack_preview_create_buffer		(CmpackPreview            *view); 
static void				cmpack_preview_paint_buffer				(CmpackPreview            *view); 

static void clear_data(CmpackPreview *view);
static void rebuild_data(CmpackPreview *view);
static void update_data(CmpackPreview *view);
static void update_chart_data(CmpackPreview *view, CmpackChartData *data);
static void update_graph_data(CmpackPreview *view, CmpackGraphData *data);
static void update_chart_item(CmpackPreview *view, CmpackPreviewItem *item, CmpackChartData *model, gint row);
static void update_graph_item(CmpackPreview *view, CmpackPreviewItem *item, CmpackGraphData *model, gint row);
static void invalidate(CmpackPreview *view);

static gint RoundToInt(gdouble x);
static gint LimitInt(gint val, gint min, gint max);

static void set_rgb(GdkColor *color, gdouble red, gdouble green, gdouble blue);
static const GdkColor *item_fg_color(CmpackPreview *view, GtkWidget *widget, const CmpackPreviewItem *item);
static const GdkColor *item_bg_color(CmpackPreview *view, GtkWidget *widget, const CmpackPreviewItem *item);

G_DEFINE_TYPE(CmpackPreview, cmpack_preview, GTK_TYPE_WIDGET)

/* Class initialization */
static void cmpack_preview_class_init(CmpackPreviewClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	gobject_class->set_property = cmpack_preview_set_property;
	gobject_class->get_property = cmpack_preview_get_property;
	gobject_class->finalize = cmpack_preview_finalize;

	object_class->destroy = cmpack_preview_destroy;

	widget_class->realize = cmpack_preview_realize;
	widget_class->unrealize = cmpack_preview_unrealize;
	widget_class->style_set = cmpack_preview_style_set;
	widget_class->size_allocate = cmpack_preview_size_allocate;
	widget_class->expose_event = cmpack_preview_expose;
	widget_class->state_changed = cmpack_preview_state_changed;

	/* Properties */
	g_object_class_install_property (gobject_class, PROP_MODEL,
		g_param_spec_object("model", "Data model", "The data model for the preview",
			G_TYPE_OBJECT, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	g_object_class_install_property (gobject_class, PROP_TEXT,
		g_param_spec_string("text", "Displayed text", "Displayed text", 
			"", (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_NEGATIVE,
		g_param_spec_boolean("negative", "Drawing Style", "FALSE = white stars on dark background, TRUE = black stars on white background",
			FALSE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 
}

/* Chart view initialization */
static void cmpack_preview_init(CmpackPreview *view)
{
	view->int_colors = (GdkColor*)g_malloc(PREVIEW_N_COLORS*sizeof(GdkColor));
}

static void cmpack_preview_destroy(GtkObject *object)
{
	CmpackPreview *view = CMPACK_PREVIEW (object);

	/* Disconnect the models */
	cmpack_preview_set_model(view, NULL);
	cmpack_preview_set_text(view, NULL);
  
	GTK_OBJECT_CLASS(cmpack_preview_parent_class)->destroy(object);
}

static void cmpack_preview_finalize(GObject *object)
{
	CmpackPreview *view = CMPACK_PREVIEW(object);

	/* Free buffered data */
	clear_data(view);

	/* Free pixmaps */
	if (view->offscreen_pixmap)
		g_object_unref(view->offscreen_pixmap);

	g_free(view->int_colors);

	G_OBJECT_CLASS(cmpack_preview_parent_class)->finalize(object);
} 

/* Set property */
static void cmpack_preview_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CmpackPreview *view = CMPACK_PREVIEW (object);

	switch (prop_id)
	{
    case PROP_MODEL:
		cmpack_preview_set_model(view, (GObject*)g_value_get_object(value));
		break;
	case PROP_TEXT:
		cmpack_preview_set_text(view, g_value_get_string(value));
		break;
	case PROP_NEGATIVE:
		cmpack_preview_set_negative(view, g_value_get_boolean(value));
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

/* Get property */
static void cmpack_preview_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CmpackPreview *view = CMPACK_PREVIEW (object);

	switch (prop_id)
    {
    case PROP_MODEL:
		g_value_set_object(value, view->model);
		break;
    case PROP_TEXT:
		g_value_set_string(value, view->text);
		break;
    case PROP_NEGATIVE:
		g_value_set_boolean(value, view->negative);
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_preview_realize (GtkWidget *widget)
{
	gboolean int_ok[PREVIEW_N_COLORS];
	CmpackPreview *view = CMPACK_PREVIEW(widget);
	GdkWindowAttr attributes;
	gint attributes_mask;

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	/* Make the main, clipping window */
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= GDK_EXPOSURE_MASK;
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_ACTIVE);

	set_rgb(&view->int_colors[PREVIEW_COLOR_BACKGROUND], 0.1, 0, 0.2);
	set_rgb(&view->int_colors[PREVIEW_COLOR_RED], 0.7, 0, 0);
	set_rgb(&view->int_colors[PREVIEW_COLOR_GREEN], 0, 0.6, 0);
	set_rgb(&view->int_colors[PREVIEW_COLOR_BLUE], 0.0, 0.4, 1.0);
	set_rgb(&view->int_colors[PREVIEW_COLOR_YELLOW], 0.7, 0.7, 0.0);
	gdk_colormap_alloc_colors(gtk_widget_get_colormap(widget), view->int_colors, 
		PREVIEW_N_COLORS, FALSE, TRUE, int_ok);

	cmpack_preview_create_buffer(view); 
	view->dirty = TRUE;
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_preview_unrealize(GtkWidget *widget)
{
	CmpackPreview *view = CMPACK_PREVIEW(widget);

	// Free custom colors
	gdk_colormap_free_colors(gtk_widget_get_colormap(widget), view->int_colors, PREVIEW_N_COLORS);

	GTK_WIDGET_CLASS(cmpack_preview_parent_class)->unrealize (widget); 
}

/* Widget state changed */
static void cmpack_preview_state_changed(GtkWidget *widget, GtkStateType previous_state)
{
	CmpackPreview *view = CMPACK_PREVIEW (widget);

	view->dirty = TRUE;
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS (cmpack_preview_parent_class)->state_changed(widget, previous_state);
}

/* Widget style changed */
static void cmpack_preview_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
	CmpackPreview *view = CMPACK_PREVIEW (widget);

	view->dirty = TRUE; 
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS(cmpack_preview_parent_class)->style_set(widget, previous_style);
}

/* Widget size changed */
static void cmpack_preview_size_allocate(GtkWidget *widget, GtkAllocation  *allocation)
{
	CmpackPreview *view = CMPACK_PREVIEW (widget);

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		cmpack_preview_create_buffer(view); 
	}
	view->dirty = TRUE;
}

/* Refresh the offscreen buffer (if needed) and paint it to the window */
static gboolean cmpack_preview_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	CmpackPreview *view;

	if (GTK_WIDGET_DRAWABLE(widget)) {
		view = CMPACK_PREVIEW(widget);
		if (view->dirty) 
			cmpack_preview_paint_buffer(view);
		gdk_draw_drawable (widget->window, widget->style->fg_gc[widget->state],
			CMPACK_PREVIEW(widget)->offscreen_pixmap,
			expose->area.x, expose->area.y, expose->area.x, expose->area.y,
			expose->area.width, expose->area.height);
	}
	return FALSE; 
}

/* Rebuild the offscreen buffer */
static void cmpack_preview_create_buffer(CmpackPreview *view)
{
	GtkWidget *widget;

	if (GTK_WIDGET_REALIZED (view)) {
		widget = GTK_WIDGET (view);
		if (view->offscreen_pixmap)
			g_object_unref (view->offscreen_pixmap);
		view->offscreen_pixmap = gdk_pixmap_new (widget->window,
			widget->allocation.width, widget->allocation.height, -1);
	}
} 

static const GdkColor *item_fg_color(CmpackPreview *view, GtkWidget *widget, const CmpackPreviewItem *item)
{
	switch (item->color)
	{
	case CMPACK_COLOR_RED:
		return &view->int_colors[PREVIEW_COLOR_RED];
	case CMPACK_COLOR_GREEN:
		return &view->int_colors[PREVIEW_COLOR_GREEN];
	case CMPACK_COLOR_BLUE:
		return &view->int_colors[PREVIEW_COLOR_BLUE];
	case CMPACK_COLOR_YELLOW:
		return &view->int_colors[PREVIEW_COLOR_YELLOW];
	case CMPACK_COLOR_GRAY:
		return &widget->style->dark[widget->state];
	default:
		if (view->negative)
			return &widget->style->fg[widget->state];
		else
			return &widget->style->white;
	}
}

static const GdkColor *item_bg_color(CmpackPreview *view, GtkWidget *widget, const CmpackPreviewItem *item)
{
	if (view->negative)
		return &widget->style->base[widget->state];
	else
		return &widget->style->black;
}

static void paint_chart(CmpackPreview *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint		i, d, x, y, dx, dy, width, height, left, top, right, bottom;
	gint		dst_width, dst_height;
	gdouble		src_width, src_height, zoom;
	GdkRectangle clip_rc;

	GdkGC *gc = gdk_gc_new(drawable);

	width = widget->allocation.width;
	height = widget->allocation.height;
	if (width<=0 || height<=0)
		return;

	// Auto scale
	src_width = fabs(view->x.ProjMax - view->x.ProjMin);
	if (src_width < view->x.ProjEps)
		src_width = view->x.ProjEps;
	src_height = fabs(view->y.ProjMax - view->y.ProjMin);
	if (src_height < view->y.ProjEps)
		src_height = view->y.ProjEps;
	zoom = MIN(width/src_width, height/src_height);
	dst_width = (gint)(zoom*src_width+0.5);
	dst_height = (gint)(zoom*src_height+0.5);
	dx = (gint)((width - dst_width)/2);
	dy = (gint)((height - dst_height)/2);

	// Widget background
	left = dx;
	if (left > 0) {
		right = LimitInt(left, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, right, height);
	}
	right = dx + dst_width;
	if (right < width) {
		left = LimitInt(right, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, left, 0, width - left, height);
	}
	top = dy;
	if (dy > 0) {
		bottom = LimitInt(top, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, width, bottom);
	}
	bottom = dy + dst_height;
	if (bottom < height) {
		top = LimitInt(bottom, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, top, width, height - top);
	}

	// Frame background
	if (dst_width>0 && dst_height>0) {
		if (!view->negative)
			gdk_gc_set_foreground(gc, &view->int_colors[PREVIEW_COLOR_BACKGROUND]);
		else
			gdk_gc_set_foreground(gc, &widget->style->base[widget->state]);
		gdk_draw_rectangle(drawable, gc, TRUE, dx, dy, dst_width, dst_height);
	}

	// Objects
	clip_rc.x = dx; 
	clip_rc.y = dy;
	clip_rc.width = dst_width;
	clip_rc.height = dst_height;
	gdk_gc_set_clip_rectangle(gc, &clip_rc);
	for (i=0; i<view->item_count; i++) {
		CmpackPreviewItem *item = view->items + i;
		if (item->valid) {
			d = RoundToInt(2.0 * item->size * sqrt(zoom));
			if (d>1) {
				x = dx + RoundToInt(item->xproj * zoom);
				if (view->image_orientation & CMPACK_ROWS_UPWARDS)
					y = dy + RoundToInt(item->yproj * zoom) + (dst_height - 1);
				else
					y = dy + RoundToInt(item->yproj * zoom);
				x -= (d+1)/2;
				y -= (d+1)/2;
				if (!item->filled) {
					gdk_gc_set_foreground(gc, item_fg_color(view, widget, item));
					gdk_draw_arc(drawable, gc, FALSE, x, y, d, d, 0, 64*360);
				} else {
					gdk_gc_set_foreground(gc, item_fg_color(view, widget, item));
					gdk_draw_arc(drawable, gc, TRUE, x, y, d, d, 0, 64*360);
					gdk_gc_set_foreground(gc, item_bg_color(view, widget, item));
					gdk_draw_arc(drawable, gc, FALSE, x, y, d, d, 0, 64*360);
				}
			}
		}
	}
	gdk_gc_destroy(gc);
}

static void paint_graph(CmpackPreview *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint		i, x, y, dx, dy, width, height, left, top, right, bottom;
	gint		dst_width, dst_height;
	gdouble		src_width, src_height, zoom, zoom_x, zoom_y, offset_x, offset_y;

	GdkGC *gc = gdk_gc_new(drawable);
	width = widget->allocation.width;
	height = widget->allocation.height;
	if (width<=0 || height<=0)
		return;

	// Auto scale
	zoom = MIN((gdouble)width/4.0, (gdouble)height/3.0);
	dst_width = (gint)(zoom*4.0+0.5);
	dst_height = (gint)(zoom*3.0+0.5);
	dx = (gint)((width - dst_width)/2);
	dy = (gint)((height - dst_height)/2);

	// Widget background
	left = dx;
	if (left > 0) {
		right = LimitInt(left, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, right, height);
	}
	right = dx + dst_width;
	if (right < width) {
		left = LimitInt(right, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, left, 0, width - left, height);
	}
	top = dy;
	if (dy > 0) {
		bottom = LimitInt(top, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, width, bottom);
	}
	bottom = dy + dst_height;
	if (bottom < height) {
		top = LimitInt(bottom, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, top, width, height - top);
	}

	// Frame background
	if (dst_width>0 && dst_height>0) {
		gdk_draw_rectangle(drawable, widget->style->base_gc[widget->state], 
			TRUE, dx, dy, dst_width, dst_height);
	}

	// Objects
	src_width = fabs(view->x.ProjMax - view->x.ProjMin);
	if (src_width < view->x.ProjEps) 
		src_width = view->x.ProjEps;
	zoom_x = (dst_width-8)/src_width;
	offset_x = dx + 0.5 * (dst_width - zoom_x * (view->x.ProjMax + view->x.ProjMin));
	src_height = fabs(view->y.ProjMax - view->y.ProjMin);
	if (src_height < view->y.ProjEps)
		src_height = view->y.ProjEps;
	zoom_y = (dst_height-8)/(-src_height);
	offset_y = dy + 0.5 * (dst_height - zoom_y * (view->y.ProjMax + view->y.ProjMin));
	for (i=0; i<view->item_count; i++) {
		CmpackPreviewItem *item = view->items + i;
		if (item->valid) {
			x = RoundToInt(item->xproj * zoom_x + offset_x);
			y = RoundToInt(item->yproj * zoom_y + offset_y);
			gdk_draw_arc(drawable, widget->style->fg_gc[widget->state], TRUE, x-2, y-2, 5, 5, 0, 64*360);
		}
	}
	gdk_gc_destroy(gc);
}

static void paint_image(CmpackPreview *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint		dx, dy, width, height, left, top, right, bottom;
	gint		dst_width, dst_height;
	gdouble		src_width, src_height, zoom;
	cairo_t		*cr;
	cairo_surface_t *cs;
	cairo_pattern_t *pat;
	cairo_matrix_t mx;

	GdkGC *gc = gdk_gc_new(drawable);
	width = widget->allocation.width;
	height = widget->allocation.height;
	if (width<=0 && height<=0)
		return;

	// Auto scale
	src_width = (view->x.ProjMax - view->x.ProjMin);
	src_height = (view->y.ProjMax - view->y.ProjMin);
	zoom = MIN((gdouble)width/src_width, (gdouble)height/src_height);
	dst_width = (gint)(zoom*src_width+0.5);
	dst_height = (gint)(zoom*src_height+0.5);
	dx = (gint)((width - dst_width)/2);
	dy = (gint)((height - dst_height)/2);

	// Widget background
	left = dx;
	if (left > 0) {
		right = LimitInt(left, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, right, height);
	}
	right = dx + dst_width;
	if (right < width) {
		left = LimitInt(right, 0, width);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, left, 0, width - left, height);
	}
	top = dy;
	if (dy > 0) {
		bottom = LimitInt(top, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, 0, width, bottom);
	}
	bottom = dy + dst_height;
	if (bottom < height) {
		top = LimitInt(bottom, 0, height);
		gdk_draw_rectangle(drawable, widget->style->dark_gc[widget->state],
			TRUE, 0, top, width, height - top);
	}

	// Image
	cs = cmpack_image_data_get_surface(CMPACK_IMAGE_DATA(view->model));
	if (cs) {
		cr = gdk_cairo_create(drawable);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		pat = cairo_pattern_create_for_surface(cs);
		cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);
		cairo_matrix_init_identity(&mx);
		cairo_matrix_scale(&mx, (double)src_width/dst_width, (double)src_height/dst_height);
		cairo_matrix_translate(&mx, -dx, -dy);
		cairo_pattern_set_matrix(pat, &mx);
		cairo_set_source(cr, pat);
		cairo_paint(cr);
		cairo_pattern_destroy(pat);
		cairo_destroy(cr);
	}

	gdk_gc_destroy(gc);
}

static void paint_text(CmpackPreview *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint		width, height;
	PangoRectangle extents;
	PangoLayout		*pg;

	GdkGC *gc = gdk_gc_new(drawable);
	width = widget->allocation.width;
	height = widget->allocation.height;

	// Widget background
	gdk_draw_rectangle(drawable, widget->style->bg_gc[widget->state],
		TRUE, 0, 0, width, height);

	// Text
	pg = gtk_widget_create_pango_layout(widget, view->text);
	pango_layout_set_alignment(pg, PANGO_ALIGN_CENTER);
	pango_layout_get_pixel_extents(pg, NULL, &extents); 
	gdk_draw_layout(drawable, gc, (width - extents.width)/2, (height - extents.height)/2, pg);
	g_object_unref(pg);

	gdk_gc_destroy(gc);
}

static void cmpack_preview_paint_buffer(CmpackPreview *view)
{
	if (view->offscreen_pixmap) {
		GtkWidget *widget = GTK_WIDGET(view);
		if (view->model && CMPACK_IS_CHART_DATA(view->model)) 
			paint_chart(view, widget, view->offscreen_pixmap);
		else if (view->model && CMPACK_IS_IMAGE_DATA(view->model)) 
			paint_image(view, widget, view->offscreen_pixmap);
		else if (view->model && CMPACK_IS_GRAPH_DATA(view->model)) 
			paint_graph(view, widget, view->offscreen_pixmap);
		else if (view->text) 
			paint_text(view, widget, view->offscreen_pixmap);
		else {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->dark_gc[widget->state],
				TRUE, 0, 0, widget->allocation.width, widget->allocation.height);
		}
		view->dirty = FALSE;
	}
} 

/* Physical units -> projection units */
static gdouble x_to_proj(CmpackPreview *view, gdouble x)
{
	if (view->x.Log) 
		return (!view->x.Reverse ? 1.0 : -1.0) * log10(x);
	else
		return (!view->x.Reverse ? 1.0 : -1.0) * x;
}

/* Physical units -> projection units */
static gdouble y_to_proj(CmpackPreview *view, gdouble y)
{
	if (view->y.Log) 
		return (!view->y.Reverse ? 1.0 : -1.0) * log10(y);
	else
		return (!view->y.Reverse ? 1.0 : -1.0) * y;
}

static gint RoundToInt(gdouble x)
{
	if (x > INT_MAX)
		return INT_MAX;
	else if (x < INT_MIN)
		return INT_MIN;
	else if (x >= 0.0)
		return (gint)(x + 0.5);
	else
		return (gint)(x - 0.5);
}

static gint LimitInt(gint x, gint min, gint max)
{
	if (x < min)
		return min;
	else if (x > max)
		return max;
	else
		return x;
}

static void set_rgb(GdkColor *color, gdouble red, gdouble green, gdouble blue)
{
	color->red = LimitInt((gint)(red*65535.0), 0, 65535);
	color->green = LimitInt((gint)(green*65535.0), 0, 65535);
	color->blue = LimitInt((gint)(blue*65535.0), 0, 65535);
}

//------------------------   PAINTING   -----------------------------------

static void graph_row_inserted(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<=view->item_count) {
		if (view->item_count >= view->item_capacity) {
			view->item_capacity += 64;
			view->items = (CmpackPreviewItem*)g_realloc(view->items, view->item_capacity*sizeof(CmpackPreviewItem));
		}
		if (row < view->item_count)
			memmove(view->items+(row+1), view->items+row, (view->item_count-row)*sizeof(CmpackPreviewItem));
		memset(view->items + row, 0, sizeof(CmpackPreviewItem));
		update_graph_item(view, view->items + row, model, row);
		view->item_count++;
		invalidate(view);
	}
}

static void graph_row_updated(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<view->item_count) {
		update_graph_item(view, view->items + row, model, row);
		invalidate(view);
	}
} 

static void graph_row_deleted(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<view->item_count) {
		if (row < view->item_count-1)
			memmove(view->items+row, view->items+(row+1), (view->item_count-row-1)*sizeof(CmpackPreviewItem));
		view->item_count--;
		invalidate(view);
	}
} 

static void graph_data_cleared(CmpackGraphData *model, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	clear_data(view);
	invalidate(view);
} 

static void chart_object_inserted(CmpackChartData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<=view->item_count) {
		if (view->item_count >= view->item_capacity) {
			view->item_capacity += 64;
			view->items = (CmpackPreviewItem*)g_realloc(view->items, view->item_capacity*sizeof(CmpackPreviewItem));
		}
		if (row < view->item_count)
			memmove(view->items+(row+1), view->items+row, (view->item_count-row)*sizeof(CmpackPreviewItem));
		memset(view->items+row, 0, sizeof(CmpackPreviewItem));
		update_chart_item(view, view->items + row, model, row);
		view->item_count++;
		invalidate(view);
	}
}

static void chart_object_updated(CmpackChartData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<view->item_count) {
		update_chart_item(view, view->items + row, model, row);
		invalidate(view);
	}
} 

static void chart_object_deleted(CmpackChartData *model, gint row, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	if (row>=0 && row<view->item_count) {
		if (row < view->item_count-1)
			memmove(view->items+row, view->items+(row+1), (view->item_count-row-1)*sizeof(CmpackPreviewItem));
		view->item_count--;
		invalidate(view);
	}
} 

static void chart_dim_changed(CmpackChartData *model, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	rebuild_data(view);
	invalidate(view);
} 

static void chart_data_cleared(CmpackChartData *model, gpointer data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	clear_data(view);
	invalidate(view);
} 

static void clear_data(CmpackPreview *view)
{
	g_free(view->items);
	view->items = NULL;
	view->item_count = view->item_capacity = 0;
}

static void update_chart_item(CmpackPreview *view, CmpackPreviewItem *item, CmpackChartData *model, gint row)
{
	const CmpackChartItem *obj;

	item->valid = FALSE;
	
	obj = cmpack_chart_data_get_item(model, row);
	if (obj && !obj->disabled) {
		item->valid = TRUE;
		item->xproj = x_to_proj(view, obj->x);
		item->yproj = y_to_proj(view, obj->y);
		item->size = RoundToInt(obj->d);
		item->color = obj->color;
		item->filled = !obj->outline;
		item->topmost = obj->topmost;
	}
}

static void update_chart_data(CmpackPreview *view, CmpackChartData *data)
{
	gint i;

	if (data && view->items) {
		// Update limits
		view->x.Min = 0;
		view->x.Max = cmpack_chart_data_width(data);
		view->x.Reverse = FALSE;
		if (!view->x.Reverse) {
			view->x.ProjMin = 0;
			view->x.ProjMax = view->x.Max;
		} else {
			view->x.ProjMin = -view->x.Max;
			view->x.ProjMax = 0;
		}
		view->y.Min = 0;
		view->y.Max = cmpack_chart_data_height(data);
		view->y.Reverse = view->image_orientation & CMPACK_ROWS_UPWARDS;
		if (!view->y.Reverse) {
			view->y.ProjMin = 0;
			view->y.ProjMax = view->y.Max;
		} else {
			view->y.ProjMin = -view->y.Max;
			view->y.ProjMax = 0;
		}
		// Update items
		for (i=0; i<view->item_count; i++)
			update_chart_item(view, view->items+i, data, i);
	}
}

static void update_image_data(CmpackPreview *view, CmpackImageData *data)
{
	if (data) {
		// Update limits
		view->x.Min = 0;
		view->x.Max = cmpack_image_data_width(data);
		view->x.Reverse = FALSE;
		if (!view->x.Reverse) {
			view->x.ProjMin = 0;
			view->x.ProjMax = view->x.Max;
		} else {
			view->x.ProjMin = -view->x.Max;
			view->x.ProjMax = 0;
		}
		view->y.Min = 0;
		view->y.Max = cmpack_image_data_height(data);
		view->y.Reverse = view->image_orientation & CMPACK_ROWS_UPWARDS;
		if (!view->y.Reverse) {
			view->y.ProjMin = -view->y.Max;
			view->y.ProjMax = 0;
		} else {
			view->y.ProjMin = 0;
			view->y.ProjMax = view->y.Max;
		}
	}
}

static void update_graph_item(CmpackPreview *view, CmpackPreviewItem *item, CmpackGraphData *model, gint row)
{
	const CmpackGraphItem *data;

	item->valid = FALSE;
	
	data = cmpack_graph_data_get_item(model, row);
	if (data->x >= view->x.Min && data->x <= view->x.Max &&
		data->y >= view->y.Min && data->y <= view->y.Max) {
			item->valid = TRUE;
			item->xproj = x_to_proj(view, data->x);
			item->yproj = y_to_proj(view, data->y);
			item->size = 0;
			item->color = item->color;
			item->filled = TRUE;
	}
}

static void update_graph_data(CmpackPreview *view, CmpackGraphData *data)
{
	gint i;

	if (view->model && view->items) {
		for (i=0; i<view->item_count; i++)
			update_graph_item(view, view->items+i, data, i);
	}
}

static void rebuild_data(CmpackPreview *view)
{
	gint count;

	clear_data(view);
	if (view->model) {
		if (CMPACK_IS_CHART_DATA(view->model)) {
			CmpackChartData *data = CMPACK_CHART_DATA(view->model);
			count = cmpack_chart_data_count(data);
			if (count>0) {
				view->item_count = view->item_capacity = count;
				view->items = (CmpackPreviewItem*)g_malloc0(view->item_capacity*sizeof(CmpackPreviewItem));
			}
			update_chart_data(view, data);
		} else
		if (CMPACK_IS_GRAPH_DATA(view->model)) {
			CmpackGraphData *data = CMPACK_GRAPH_DATA(view->model);
			count = cmpack_graph_data_nrows(data);
			if (count>0) {
				view->item_count = view->item_capacity = count;
				view->items = (CmpackPreviewItem*)g_malloc0(view->item_capacity*sizeof(CmpackPreviewItem));
			}
			update_graph_data(view, data);
		} else
		if (CMPACK_IS_IMAGE_DATA(view->model)) {
			CmpackImageData *data = CMPACK_IMAGE_DATA(view->model);
			update_image_data(view, data);
		}
	}
}

static void update_data(CmpackPreview *view)
{
	if (view->model) {
		if (CMPACK_IS_CHART_DATA(view->model))
			update_chart_data(view, CMPACK_CHART_DATA(view->model));
		else if (CMPACK_IS_GRAPH_DATA(view->model))
			update_graph_data(view, CMPACK_GRAPH_DATA(view->model));
		else if (CMPACK_IS_IMAGE_DATA(view->model))
			update_image_data(view, CMPACK_IMAGE_DATA(view->model));
	}
}

static void image_changed(CmpackImageData *image, gpointer *data)
{
	CmpackPreview *view = CMPACK_PREVIEW(data);

	invalidate(view);
}

static void invalidate(CmpackPreview *view)
{
	view->dirty = TRUE;
	gtk_widget_queue_draw(GTK_WIDGET(view));
}

/* -----------------------------   Public API ------------------------------------ */

/* Create a new chart with default model */
GtkWidget *cmpack_preview_new(void)
{
	return cmpack_preview_new_with_model(NULL);
}

/* Create a new chart with specified model */
GtkWidget *cmpack_preview_new_with_model(GObject *model)
{
  return (GtkWidget*)g_object_new(CMPACK_TYPE_PREVIEW, "model", model, NULL);
}

/* Set data model */
void cmpack_preview_set_model(CmpackPreview *view, GObject *model)
{
	g_return_if_fail (CMPACK_IS_PREVIEW (view));
	g_return_if_fail (model == NULL || CMPACK_IS_CHART_DATA(model) || 
		CMPACK_IS_IMAGE_DATA(model) || CMPACK_IS_GRAPH_DATA(model));
  
	if (view->model == model)
		return;

	if (view->text) {
		g_free(view->text);
		view->text = NULL;
	}
	if (view->model) {
		if (CMPACK_IS_CHART_DATA(view->model)) {
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)chart_object_inserted, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)chart_object_updated, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)chart_object_deleted, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)chart_dim_changed, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)chart_data_cleared, view);
		} else
		if (CMPACK_IS_GRAPH_DATA(view->model)) {
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)graph_row_inserted, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)graph_row_updated, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)graph_row_deleted, view);
			g_signal_handlers_disconnect_by_func (view->model, (gpointer)graph_data_cleared, view);
		} else
		if (CMPACK_IS_IMAGE_DATA(view->model)) {
			g_signal_handlers_disconnect_by_func(view->model, (gpointer)image_changed, view);
		}
		g_object_unref(view->model);
	}
	view->model = model;
	if (view->model) {
		g_object_ref(view->model);
		if (CMPACK_IS_CHART_DATA(view->model)) {
			g_signal_connect(view->model, "object-inserted", G_CALLBACK(chart_object_inserted), view);
			g_signal_connect(view->model, "object-updated", G_CALLBACK(chart_object_updated), view);
			g_signal_connect(view->model, "object-deleted", G_CALLBACK(chart_object_deleted), view);
			g_signal_connect(view->model, "dim-changed", G_CALLBACK(chart_dim_changed), view);
			g_signal_connect(view->model, "data-cleared", G_CALLBACK(chart_data_cleared), view);
		} else
		if (CMPACK_IS_GRAPH_DATA(view->model)) {
			g_signal_connect(view->model, "row-inserted", G_CALLBACK(graph_row_inserted), view);
			g_signal_connect(view->model, "row-updated", G_CALLBACK(graph_row_updated), view);
			g_signal_connect(view->model, "row-deleted", G_CALLBACK(graph_row_deleted), view);
			g_signal_connect(view->model, "data-cleared", G_CALLBACK(graph_data_cleared), view);
		} else
		if (CMPACK_IS_IMAGE_DATA(view->model)) {
			g_signal_connect(view->model, "data-changed", G_CALLBACK(image_changed), view);
		}
	}
	rebuild_data(view);
	invalidate(view);

	g_object_notify(G_OBJECT(view), "model");  
}

/* Set data model */
void cmpack_preview_set_text(CmpackPreview *view, const gchar *text)
{
	g_return_if_fail(CMPACK_IS_PREVIEW (view));

	if (text) {
		if (view->model)
			cmpack_preview_set_model(view, NULL);
		if (!view->text || strcmp(text, view->text)!=0) {
			g_free(view->text);
			view->text = g_strdup(text);
			invalidate(view);
		}
	} else {
		if (view->text) {
			g_free(view->text);
			view->text = NULL;
			invalidate(view);
		}
	}

	g_object_notify(G_OBJECT(view), "text");  
}

/* Get data model */
GObject *cmpack_preview_get_model(CmpackPreview *view)
{
	g_return_val_if_fail (CMPACK_IS_PREVIEW(view), NULL);
	
	return view->model;
}

void cmpack_preview_set_x_axis(CmpackPreview *view, 
	gboolean log_scale, gboolean reverse, gdouble min, gdouble max, gdouble eps)
{
	g_return_if_fail (CMPACK_IS_PREVIEW(view));

	view->x.Log = log_scale;
	view->x.Reverse = reverse;
	if (!view->x.Log) {
		view->x.Log = FALSE;
		view->x.Min = min;
		view->x.Max = max;
		if (!view->x.Reverse) {
			view->x.ProjMin = min;
			view->x.ProjMax = max;
		} else {
			view->x.ProjMin = -max;
			view->x.ProjMax = -min;
		}
        view->x.ProjEps = eps;
	} else {
		view->x.Log = TRUE;
		view->x.Min = MAX(DBL_MIN, min);
		view->x.Max = MAX(DBL_MIN, max);
		if (!view->x.Reverse) {
			view->x.ProjMin = log10(view->x.Min);
			view->x.ProjMax = log10(view->x.Max);
		} else {
			view->x.ProjMin = -log10(view->x.Max);
			view->x.ProjMax = -log10(view->x.Min);
		}
        view->x.ProjEps = log10(MAX(DBL_MIN, eps));
	}
	update_data(view);
	invalidate(view);
}

void cmpack_preview_set_y_axis(CmpackPreview *view, 
	gboolean log_scale, gboolean reverse, gdouble min, gdouble max, gdouble eps)
{
	g_return_if_fail(CMPACK_IS_PREVIEW(view));

	view->y.Log = log_scale;
	view->y.Reverse = reverse;
	if (!view->y.Log) {
		view->y.Log = FALSE;
		view->y.Min = min;
		view->y.Max = max;
		if (!view->y.Reverse) {
			view->y.ProjMin = min;
			view->y.ProjMax = max;
		} else {
			view->y.ProjMin = -max;
			view->y.ProjMax = -min;
		}
        view->y.ProjEps = eps;
	} else {
		view->y.Log = TRUE;
		view->y.Min = MAX(DBL_MIN, min);
		view->y.Max = MAX(DBL_MIN, max);
		if (!view->y.Reverse) {
			view->y.ProjMin = log10(view->y.Min);
			view->y.ProjMax = log10(view->y.Max);
		} else {
			view->y.ProjMin = -log10(view->y.Max);
			view->y.ProjMax = -log10(view->y.Min);
		}
        view->y.ProjEps = log10(MAX(DBL_MIN, eps));
	}
	update_data(view);
	invalidate(view);
}

void cmpack_preview_set_negative(CmpackPreview *view, gboolean negative)
{
	g_return_if_fail (CMPACK_IS_PREVIEW(view));

	if (view->negative != negative) {
		view->negative = negative;
		update_data(view);
		invalidate(view);
	}
}

void cmpack_preview_set_image_orientation(CmpackPreview *view, CmpackOrientation orientation)
{
	g_return_if_fail (CMPACK_IS_PREVIEW(view));

	if (view->image_orientation != orientation) {
		view->image_orientation = orientation;
		update_data(view);
		invalidate(view);
	}
}

gboolean cmpack_preview_get_negative(CmpackPreview *view)
{
	g_return_val_if_fail(CMPACK_IS_PREVIEW (view), FALSE);

	return view->negative;
}
