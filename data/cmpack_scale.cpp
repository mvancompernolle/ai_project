/**************************************************************

cmpack_scale.cpp (C-Munipack project)
Widget which can draw a scale
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

$Id: cmpack_scale.cpp,v 1.3 2013/08/03 07:23:12 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "cmpack_scale.h"

#define IMAGE_WIDTH		512
#define IMAGE_HEIGHT	512

/* Properties */
enum
{
	PROP_0,
	PROP_ORIENTATION,
	PROP_PSEUDOCOLOR,
	PROP_INVERT
};

/* GObject vfuncs */
static void             cmpack_scale_set_property              (GObject            *object,
								 guint               prop_id,
								 const GValue       *value,
								 GParamSpec         *pspec);
static void             cmpack_scale_get_property              (GObject            *object,
								 guint               prop_id,
								 GValue             *value,
								 GParamSpec         *pspec);
static void             cmpack_scale_finalize                  (GObject          *object); 

/* GtkObject vfuncs */
static void             cmpack_scale_destroy                   (GtkObject          *object);

/* GtkWidget vfuncs */
static void             cmpack_scale_realize                   (GtkWidget          *widget);
static void             cmpack_scale_unrealize                 (GtkWidget          *widget);
static void             cmpack_scale_style_set                 (GtkWidget        *widget,
						                 GtkStyle         *previous_style);
static void             cmpack_scale_state_changed             (GtkWidget        *widget,
			                                         GtkStateType      previous_state);
static void             cmpack_scale_size_allocate             (GtkWidget          *widget,
								 GtkAllocation      *allocation);
static gboolean         cmpack_scale_expose                    (GtkWidget          *widget,
								 GdkEventExpose     *expose);

/* Internal functions */
static void update_rectangles(CmpackScale *scale);
static void create_buffer(CmpackScale *scale);
static void	paint_buffer(CmpackScale *scale);
static void rebuild_image(CmpackScale *scale);

static gint RoundToInt(gdouble x);
static void text_extents(GtkWidget *widget, const gchar *buf, gint *width, gint *height);
static gint text_width(GtkWidget *widget, const gchar *buf);
static gint text_height(GtkWidget *widget, const gchar *buf);
static void draw_text(GtkWidget *widget, GdkDrawable *drawable, gint x, gint y, 
	const char *buf, gdouble halign, gdouble valign);

static gint compute_label_width(CmpackScale *view, gint prec);
static gint compute_label_height(CmpackScale *view);
static void compute_x_grid_step(CmpackScale *view, gdouble *step, gint *minprec);
static void compute_y_grid_step(CmpackScale *view, gdouble *step, gint *minprec);

G_DEFINE_TYPE(CmpackScale, cmpack_scale, GTK_TYPE_WIDGET)

/* Class initialization */
static void cmpack_scale_class_init(CmpackScaleClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	gobject_class->set_property = cmpack_scale_set_property;
	gobject_class->get_property = cmpack_scale_get_property;
	gobject_class->finalize = cmpack_scale_finalize;

	object_class->destroy = cmpack_scale_destroy;

	widget_class->realize = cmpack_scale_realize;
	widget_class->unrealize = cmpack_scale_unrealize;
	widget_class->style_set = cmpack_scale_style_set;
	widget_class->size_allocate = cmpack_scale_size_allocate;
	widget_class->expose_event = cmpack_scale_expose;
	widget_class->state_changed = cmpack_scale_state_changed;

	/* Properties */
	g_object_class_install_property (gobject_class, PROP_ORIENTATION, 
		g_param_spec_enum("orientation", "Orientation", "The orientation of the scale",
			GTK_TYPE_ORIENTATION, GTK_ORIENTATION_HORIZONTAL, 
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_PSEUDOCOLOR,
		g_param_spec_boolean("pseudocolor", "Scale type", "Show pseudocolor scale", FALSE, 
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_INVERT,
		g_param_spec_boolean("invert", "Invert scale", "Invert the scale", FALSE,
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 
}

/* Graph view initialization */
static void cmpack_scale_init(CmpackScale *view)
{
}

static void cmpack_scale_destroy(GtkObject *object)
{
	GTK_OBJECT_CLASS(cmpack_scale_parent_class)->destroy (object);
}

static void cmpack_scale_finalize(GObject *object)
{
	CmpackScale *view = CMPACK_SCALE(object);

	/* Free pixmaps */
	if (view->offscreen_pixmap)
		g_object_unref(view->offscreen_pixmap);
	if (view->scale_image)
		cairo_surface_destroy(view->scale_image);

	G_OBJECT_CLASS(cmpack_scale_parent_class)->finalize(object);
} 

/* Set property */
static void cmpack_scale_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CmpackScale *view = CMPACK_SCALE(object);

	switch (prop_id)
	{
	case PROP_ORIENTATION:
		cmpack_scale_set_orientation(view, (GtkOrientation)g_value_get_enum(value));
		break;
	case PROP_PSEUDOCOLOR:
		cmpack_scale_set_pseudocolor(view, g_value_get_boolean (value));
		break;
	case PROP_INVERT:
		cmpack_scale_set_invert(view, g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/* Get property */
static void cmpack_scale_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CmpackScale *view = CMPACK_SCALE(object);

	switch (prop_id)
	{
	case PROP_ORIENTATION:
		g_value_set_enum(value, view->orientation);
		break;
    case PROP_PSEUDOCOLOR:
		g_value_set_boolean(value, view->pseudocolor);
		break;
    case PROP_INVERT:
		g_value_set_boolean(value, view->reverse);
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_scale_realize (GtkWidget *widget)
{
	CmpackScale *view = CMPACK_SCALE(widget);
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

	update_rectangles(view);
	create_buffer(view); 
	view->needs_rebuild = view->needs_repaint = TRUE;
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_scale_unrealize(GtkWidget *widget)
{
	GTK_WIDGET_CLASS(cmpack_scale_parent_class)->unrealize (widget); 
}

/* Widget state changed */
static void cmpack_scale_state_changed(GtkWidget *widget, GtkStateType previous_state)
{
	CmpackScale *view = CMPACK_SCALE (widget);

	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);
	view->needs_repaint = TRUE;

	GTK_WIDGET_CLASS (cmpack_scale_parent_class)->state_changed(widget, previous_state);
}

/* Widget style changed */
static void cmpack_scale_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
	CmpackScale *view = CMPACK_SCALE (widget);

	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);
	view->needs_repaint = TRUE; 

	GTK_WIDGET_CLASS(cmpack_scale_parent_class)->style_set(widget, previous_style);
}

/* Widget size changed */
static void cmpack_scale_size_allocate(GtkWidget *widget, GtkAllocation  *allocation)
{
	CmpackScale *view = CMPACK_SCALE (widget);

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		update_rectangles(view);
		create_buffer(view); 
	}
	view->needs_repaint = TRUE;
}

/* Refresh the offscreen buffer (if needed) and paint it to the window */
static gboolean cmpack_scale_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	CmpackScale *view;

	if (GTK_WIDGET_DRAWABLE(widget)) {
		view = CMPACK_SCALE(widget);
		if (view->needs_rebuild) 
			rebuild_image(view);
		if (view->needs_repaint)
			paint_buffer(view);
		gdk_draw_drawable (widget->window, widget->style->fg_gc[widget->state],
				   CMPACK_SCALE(widget)->offscreen_pixmap,
				   expose->area.x, expose->area.y, expose->area.x, expose->area.y,
				   expose->area.width, expose->area.height);
	}
	return FALSE; 
}

/* Rebuild the offscreen buffer */
static void create_buffer(CmpackScale *view)
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

static void update_rectangles(CmpackScale *view)
{
	int sx, sy, tprec;
	double tstep;

	if (GTK_WIDGET_REALIZED(view)) {
		GtkWidget *widget = GTK_WIDGET(view);
		int width = widget->allocation.width, height = widget->allocation.height;

		// Background
		text_extents(widget, "X", &sx, &sy);
		if (view->orientation == GTK_ORIENTATION_HORIZONTAL) {
			view->scale_rc.x = 20;
			view->scale_rc.y = 8;
			view->scale_rc.width = width - 40;
			view->scale_rc.height = height - (2*sy + 24);
		} else {
			compute_y_grid_step(view, &tstep, &tprec);
			sx = compute_label_width(view, tprec);
			view->scale_rc.x = 20;
			view->scale_rc.y = 12;
			view->scale_rc.width = width - (sx + 40);
			view->scale_rc.height = height - (sy + 24);
		}
	}
}

static gdouble x_to_view(CmpackScale *view, gdouble u)
{
	return view->scale_rc.x + (u - view->min)/(view->max-view->min)*view->scale_rc.width;
}

static gdouble y_to_view(CmpackScale *view, gdouble v)
{
	return view->scale_rc.y + (view->max - v)/(view->max-view->min)*view->scale_rc.height;
}

static void paint_buffer(CmpackScale *view)
{
	int x, y, sx, width, height, tprec;
	double v, tstep;
	char buf[512];
	GtkWidget *widget;

	if (view->offscreen_pixmap) {
		widget = GTK_WIDGET(view);
		width = widget->allocation.width;
		height = widget->allocation.height;

		// Background
		gdk_draw_rectangle(view->offscreen_pixmap, widget->style->bg_gc[widget->state], 
			TRUE, 0, 0, widget->allocation.width, widget->allocation.height);

		// Paint scale image
		if (view->scale_image && view->scale_rc.width>0 && view->scale_rc.height>0) {
			cairo_t *cr = gdk_cairo_create(view->offscreen_pixmap);
			cairo_pattern_t *pat = cairo_pattern_create_for_surface(view->scale_image);
			cairo_pattern_set_filter(pat, CAIRO_FILTER_NEAREST);
			cairo_translate(cr, view->scale_rc.x, view->scale_rc.y);
			if (view->orientation==GTK_ORIENTATION_HORIZONTAL) 
				cairo_scale(cr, ((double)view->scale_rc.width)/IMAGE_WIDTH, view->scale_rc.height);
			else
				cairo_scale(cr, view->scale_rc.width, ((double)view->scale_rc.height)/IMAGE_HEIGHT);
			cairo_set_source(cr, pat);
			cairo_paint(cr);
			cairo_pattern_destroy(pat);
			cairo_destroy(cr);

			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->fg_gc[widget->state], FALSE,
				view->scale_rc.x, view->scale_rc.y, view->scale_rc.width, view->scale_rc.height);
		}

		// Paint labels
		if (view->max > view->min) {
			if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
				// Values
				compute_x_grid_step(view, &tstep, &tprec);
				v = floor(view->min/tstep)*tstep;
				y = view->scale_rc.y + view->scale_rc.height;
				while (v < view->max + tstep) {
					x = RoundToInt(x_to_view(view, v));
					if (x >= view->scale_rc.x && x <= view->scale_rc.x + view->scale_rc.width) {
						gdk_draw_line(view->offscreen_pixmap, widget->style->fg_gc[widget->state], 
							x, y, x, y+2);
						sprintf(buf, "%.*f", tprec, v);
						draw_text(widget, view->offscreen_pixmap, x, y+6, buf, 0.5, 0.0);
					}
					v += tstep;
				}
				// Unit
				draw_text(widget, view->offscreen_pixmap, width-4, height-6, "ADU", 1.0, 1.0);
			} else {
				// Values
				compute_y_grid_step(view, &tstep, &tprec);
				v = floor(view->min/tstep)*tstep;
				x = view->scale_rc.x + view->scale_rc.width;
				sx = width - view->scale_rc.width - 40;
				while (v < view->max+tstep) {
					y = RoundToInt(y_to_view(view, v));
					if (y >= view->scale_rc.y && y <= view->scale_rc.y + view->scale_rc.height) {
						gdk_draw_line(view->offscreen_pixmap, widget->style->fg_gc[widget->state], 
							x, y, x+2, y);
						sprintf(buf, "%.*f", tprec, v);
						draw_text(widget, view->offscreen_pixmap, x+sx+6, y, buf, 1.0, 0.5);
					}
					v += tstep;
				}
				// Unit
				draw_text(widget, view->offscreen_pixmap, x + sx + 6, height-6, "ADU", 1.0, 1.0);
			}
		}
		view->needs_repaint = FALSE;
	}
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

//
// grayscale mapping
//
static uint32_t grayscale(double x)
{
	x = 255.0*x;

	if (x<=0.0)
		return 0x000000;
	if (x>=255.0)
		return 0xFFFFFF;

	uint32_t ci = (int)x;
	return ci | (ci<<8) | (ci<<16);
}


//
// pseudocolors mapping (black, violet, blue, cyan, green, yellow, red, white) }
//
static uint32_t pseudocolor(double x)
{
	x = x*1785.0;

	if (x>=0x6FF)
		return 0xFFFFFF;
	if (x<=0)
		return 0x000000;

	uint32_t ci = (int)x;
	switch (ci>>8)
	{
	case 0:
		return (ci & 0xFF) | (ci & 0xFF)<<16;
	case 1:
		return (~ci & 0xFF)<<16 | 0xFF;
	case 2:
		return (ci & 0xFF)<<8 | 0xFF;
	case 3:
		return 0xFF00 | (~ci & 0xFF);
	case 4:
		return (ci & 0xFF)<<16 | 0xFF00;
	case 5:
		return 0xFF0000 | (~ci & 0xFF)<<8;
	case 6:
		return 0xFF0000 | (ci & 0xFF)<<8 | (ci & 0xFF);
	default:
		return 0;
	}
}

static void text_extents(GtkWidget *widget, const gchar *buf, gint *width, gint *height)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		if (width)
			*width = logical_rect.width;
		if (height)
			*height = logical_rect.height;
		g_object_unref(layout);
	} else {
		if (width)
			*width = 0;
		if (height)
			*height = 0;
	}
}

static gint text_width(GtkWidget *widget, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		g_object_unref (layout);
		return logical_rect.width;
	}
	return 0;
}

static gint text_height(GtkWidget *widget, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		g_object_unref (layout);
		return logical_rect.height;
	}
	return 0;
}

/*---------------------   SCALE LABELS & GRIDS   ---------------------------*/

/* Compute max. width of scale label */
static gint compute_label_width(CmpackScale *view, int prec)
{
	gchar	buf[256];
    gdouble	val;
	
	val = MAX(fabs(view->min), fabs(view->max));
	sprintf(buf, "%.*f", prec, -val);
	return text_width(GTK_WIDGET(view), buf);
}

/* Compute height of scale label */
static gint compute_label_height(CmpackScale *view)
{
    return text_height(GTK_WIDGET(view), "X");
}

static gint step_to_prec(gdouble step)
{
	if (step<=0.0 || step>=1.0)
		return 0;
	return (gint)ceil(-log10(step));
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_x_grid_step(CmpackScale *view, gdouble *step, gint *minprec)
{
    gint	cx, nlines, xprec = 0, oldprec, e;
	gdouble	xstep, delta, minval = view->min, maxval = view->max;
	
    // Decimal mode
	xstep = fabs(maxval - minval);
	do {
		oldprec = xprec;
		cx = compute_label_width(view, xprec);
		nlines = (int)(view->scale_rc.width/((cx+1)*3.0)) - 1;
		if (nlines<2) 
			nlines = 2;
		e = (int)ceil(log10(fabs(maxval-minval)/nlines));
		if (e < DBL_MIN_10_EXP)
			e = DBL_MIN_10_EXP;
		if (e > DBL_MAX_10_EXP)
			e = DBL_MAX_10_EXP;
		delta = pow(10.0, e);
		if ((fabs(maxval-minval)/delta)>nlines) {
			xstep = delta;
		} else {
			delta = 0.5 * pow(10.0, e);
			if ((fabs(maxval-minval)/delta)>nlines) {
				xstep = delta;
			} else {
				delta = 0.2 * pow(10.0, e);
				if ((fabs(maxval-minval)/delta)>nlines) {
					xstep = delta;
				} else {
					delta = 0.1 * pow(10.0, e);
					if ((fabs(maxval-minval)/delta)>nlines) {
						xstep = delta;
					}
				}
			}
		}
		xprec = step_to_prec(xstep);
	} while (xprec > oldprec);
	if (step)
		*step = xstep;
	if (minprec)
		*minprec = xprec;
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_y_grid_step(CmpackScale *view, gdouble *step, gint *minprec)
{
    int		cy, nlines, e;
	double	ystep, delta, minval = view->min, maxval = view->max;

	cy = compute_label_height(view);
	nlines = (int)(view->scale_rc.height/(cy*4.0)) - 1;
	if (nlines<2) nlines = 1;

    // Decimal mode
	e = (int)ceil(log10(fabs(maxval-minval)/nlines));
	if (e < DBL_MIN_10_EXP)
		e = DBL_MIN_10_EXP;
	if (e > DBL_MAX_10_EXP)
		e = DBL_MAX_10_EXP;
	delta = pow(10.0, e);
	ystep = fabs(maxval-minval);
	if ((fabs(maxval-minval)/delta)>nlines) {
		ystep = delta;
	} else {
		delta = 0.5 * pow(10.0, e);
		if ((fabs(maxval-minval)/delta)>nlines) {
			ystep = delta;
		} else {
			delta = 0.2 * pow(10.0, e);
			if ((fabs(maxval-minval)/delta)>nlines) {
				ystep = delta;
			} else {
				delta = 0.1 * pow(10.0, e);
				if ((fabs(maxval-minval)/delta)>nlines) {
					ystep = delta;
				}
			}
		}
	}
	if (step)
		*step = ystep;
	if (minprec)
		*minprec = step_to_prec(ystep);
}

//------------------------   PAINTING   -----------------------------------

static void draw_text(GtkWidget *widget, GdkDrawable *drawable, 
	gint x, gint y, const char *buf, gdouble halign, gdouble valign)
{
	PangoRectangle logical_rect;

	PangoLayout *layout = gtk_widget_create_pango_layout(widget, buf);
	if (halign>0 || valign>0) {
		pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
		if (halign>0)
			x -= RoundToInt(logical_rect.width*halign);
		if (valign>0)
			y -= RoundToInt(logical_rect.height*valign);
	}
	gdk_draw_layout(drawable, widget->style->text_gc[widget->state], x, y, layout);
	g_object_unref(layout);
}

/* Rebuild image */
static void rebuild_image(CmpackScale *view)
{
	int x, y;

	if (view->scale_image) {
		if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
			if (cairo_image_surface_get_height(view->scale_image)!=1) {
				cairo_surface_destroy(view->scale_image);
				view->scale_image = NULL;
			}
		} else {
			if (cairo_image_surface_get_width(view->scale_image)!=1) {
				cairo_surface_destroy(view->scale_image);
				view->scale_image = NULL;
			}
		}
	}
	if (!view->scale_image) {
		if (view->orientation==GTK_ORIENTATION_HORIZONTAL)
			view->scale_image = cairo_image_surface_create(CAIRO_FORMAT_RGB24, IMAGE_WIDTH, 1);
		else
			view->scale_image = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 1, IMAGE_HEIGHT);
	}
	if (view->scale_image) {
		uint32_t *ptr = (uint32_t*)cairo_image_surface_get_data(view->scale_image);
		if (!view->pseudocolor) {
			if (!view->reverse) {
				if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
					for (x=0; x<IMAGE_WIDTH; x++) 
						*ptr++ = grayscale(((double)x)/IMAGE_WIDTH);
				} else {
					for (y=0; y<IMAGE_HEIGHT; y++)
						*ptr++ = grayscale(1.0-((double)y)/IMAGE_HEIGHT);
				}
			} else {
				if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
					for (x=0; x<IMAGE_WIDTH; x++)
						*ptr = grayscale(1.0-((double)x)/IMAGE_WIDTH);
				} else {
					for (y=0; y<IMAGE_HEIGHT; y++)
						*ptr++ = grayscale(((double)y)/IMAGE_HEIGHT);
				}
			}
		} else {
			if (!view->reverse) {
				if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
					for (x=0; x<IMAGE_WIDTH; x++)
						*ptr++ = pseudocolor(((double)x)/IMAGE_WIDTH);
				} else {
					for (y=0; y<IMAGE_HEIGHT; y++)
						*ptr++ = pseudocolor(1.0-((double)y)/IMAGE_HEIGHT);
				}
			} else {
				if (view->orientation==GTK_ORIENTATION_HORIZONTAL) {
					for (x=0; x<IMAGE_WIDTH; x++)
						*ptr++ = pseudocolor(1.0-((double)x)/IMAGE_WIDTH);
				} else {
					for (y=0; y<IMAGE_HEIGHT;y++)
						*ptr++ = pseudocolor(((double)y)/IMAGE_HEIGHT);
				}
			}
		}
	}
}

/* ------------------------------   Public API   -----------------------------*/

/* Create a new graph with default model */
GtkWidget *cmpack_scale_new(GtkOrientation orient)
{
	return (GtkWidget*)g_object_new(CMPACK_TYPE_SCALE, "orientation", orient, NULL);
}

/* Change the orientation of the displayed scale */
void cmpack_scale_set_orientation(CmpackScale *view, GtkOrientation orientation)
{
	g_return_if_fail(CMPACK_IS_SCALE(view));

	if (orientation!=view->orientation) {
		view->orientation = orientation;
		update_rectangles(view);
		view->needs_rebuild = view->needs_repaint = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

/* Get current orientation of displayed scale */
GtkOrientation cmpack_scale_get_orientation(CmpackScale *view)
{
	g_return_val_if_fail(CMPACK_IS_SCALE(view), GTK_ORIENTATION_HORIZONTAL);

	return view->orientation;
}

/* Change the type of the displayed scale */
void cmpack_scale_set_pseudocolor(CmpackScale *view, gboolean pseudocolor)
{
	g_return_if_fail(CMPACK_IS_SCALE(view));

	if (view->pseudocolor!=pseudocolor) {
		view->pseudocolor = pseudocolor;
		view->needs_rebuild = view->needs_repaint = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

/* Get current type of displayed scale */
gboolean cmpack_scale_get_pseudocolor(CmpackScale *view)
{
	g_return_val_if_fail(CMPACK_IS_SCALE(view), FALSE);

	return view->pseudocolor;
}

/* Change the inversion flag of the scale */
void cmpack_scale_set_invert(CmpackScale *view, gboolean invert)
{
	g_return_if_fail(CMPACK_IS_SCALE(view));

	if (invert!=view->reverse) {
		view->reverse = invert;
		view->needs_rebuild = view->needs_repaint = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

/* Get the state of the inversion flag */
gboolean cmpack_scale_get_invert(CmpackScale *view)
{
	g_return_val_if_fail(CMPACK_IS_SCALE(view), FALSE);

	return view->reverse;
}

//
// Change the range of values
//
void cmpack_scale_set_range(CmpackScale *view, gdouble min, gdouble max)
{
	gdouble a, b;

	g_return_if_fail(CMPACK_IS_SCALE(view));

	a = MIN(min, max);
	b = MAX(min, max);

	if (view->min!=a || view->max!=b) {
		view->min = a;
		view->max = b;
		update_rectangles(view);
		view->needs_repaint = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

