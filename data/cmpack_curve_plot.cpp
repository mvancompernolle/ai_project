/**************************************************************

cmpack_curve_plot.cpp (C-Munipack project)
Widget which can draw a curve
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

$Id: cmpack_curve_plot.cpp,v 1.5 2013/08/17 08:48:30 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gdk/gdkkeysyms.h>

#include "cmpack_curve_plot.h"

#define MARKER_SIZE		7

#define CURSOR_SIZE		12

#define CLICK_TOLERANCE 7

#define NumTSteps		20

// This array enumerates allowed step of the scale in TIME mode
const double TSteps[NumTSteps] = {1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0, 120.0, 300.0, 600.0, 
        900.0, 1800.0, 3600.0, 7200.0, 10800, 21600.0, 43200.0, 86400.0, 172800.0, 
        259200.0};

/* Signals */
enum {
	ITEM_ACTIVATED,
	SELECTION_CHANGED,
	MOUSE_MOVED,
	LAST_SIGNAL
};

/* Properties */
enum {
	PROP_0,
	PROP_SELECTION_MODE,
	PROP_MODEL
};

/* Internal colors */
enum {
	GRAPH_COLOR_SELECTED,
	GRAPH_COLOR_ZOOMRECT,
	GRAPH_COLOR_CURSOR,
	GRAPH_COLOR_RED,
	GRAPH_COLOR_GREEN,
	GRAPH_COLOR_BLUE,
	GRAPH_COLOR_YELLOW,
	GRAPH_N_COLORS
};

struct _CmpackCurvePlotItem
{
	gboolean    visible;		// Item is visible
	gdouble		value, error;	// Position and error in projection units
}; 

/* GObject vfuncs */
static void             cmpack_curve_plot_set_property              (GObject            *object,
								 guint               prop_id,
								 const GValue       *value,
								 GParamSpec         *pspec);
static void             cmpack_curve_plot_get_property              (GObject            *object,
								 guint               prop_id,
								 GValue             *value,
								 GParamSpec         *pspec);
static void             cmpack_curve_plot_finalize                  (GObject          *object); 

/* GtkObject vfuncs */
static void             cmpack_curve_plot_destroy                   (GtkObject          *object);
/* GtkWidget vfuncs */
static void             cmpack_curve_plot_realize                   (GtkWidget          *widget);
static void             cmpack_curve_plot_unrealize                 (GtkWidget          *widget);
static void             cmpack_curve_plot_style_set                 (GtkWidget        *widget,
						                 GtkStyle         *previous_style);
static void             cmpack_curve_plot_state_changed             (GtkWidget        *widget,
			                                         GtkStateType      previous_state);
static void             cmpack_curve_plot_size_allocate             (GtkWidget          *widget,
								 GtkAllocation      *allocation);
static gboolean         cmpack_curve_plot_expose                    (GtkWidget          *widget,
								 GdkEventExpose     *expose);
static gboolean         cmpack_curve_plot_motion                    (GtkWidget          *widget,
								 GdkEventMotion     *event);
static gboolean         cmpack_curve_plot_leave                     (GtkWidget          *widget,
								 GdkEventCrossing   *event);
static gboolean         cmpack_curve_plot_button_press              (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_curve_plot_button_release            (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_curve_plot_key_press                 (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_curve_plot_key_release               (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_curve_plot_scroll		     		(GtkWidget      *widget,
								 GdkEventScroll     *event);

/* CmpackCurvePlot vfuncs */
static void             cmpack_curve_plot_set_adjustments           (CmpackCurvePlot        *view,
								 GtkAdjustment      *hadj,
								 GtkAdjustment      *vadj);

 /* Internal functions */
static void             cmpack_curve_plot_create_buffer		(CmpackCurvePlot            *view); 
//static void             cmpack_curve_plot_create_marker		(CmpackCurvePlot            *view); 

static void				cmpack_curve_plot_paint_buffer				(CmpackCurvePlot            *view); 
//static void				cmpack_curve_plot_paint_marker				(CmpackCurvePlot            *view); 

static void                 cmpack_curve_plot_adjustment_changed             (GtkAdjustment          *adjustment,
									  CmpackCurvePlot            *view);
static void                 cmpack_curve_plot_queue_draw_item                (CmpackCurvePlot            *view,
									  gint row, CmpackCurvePlotItem        *item);
static void                 cmpack_curve_plot_enter_mouse_mode            (CmpackCurvePlot            *view,
									  gint                    x,
									  gint                    y,
									  CmpackPlotMouseMode    mode);
static void                 cmpack_curve_plot_leave_mouse_mode             (CmpackCurvePlot            *view);
static gboolean             cmpack_curve_plot_unselect_all_internal          (CmpackCurvePlot            *view);
static void cmpack_curve_plot_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data);

static void clear_data(CmpackCurvePlot *view);
static void rebuild_data(CmpackCurvePlot *view);
static void update_data(CmpackCurvePlot *view);
static void invalidate(CmpackCurvePlot *view);
static void update_item(CmpackCurvePlot *view, CmpackCurvePlotItem *item, CmpackGraphData *model, gint row);

static gint RoundToInt(gdouble x);
static gdouble LimitDbl(gdouble val, gdouble min, gdouble max);
static gint LimitInt(gint val, gint min, gint max);

static void set_rgb(GdkColor *color, gdouble red, gdouble green, gdouble blue);

static gdouble xproj_to_view(CmpackCurvePlot *view, gdouble x);
static gdouble view_to_xproj(CmpackCurvePlot *view, gdouble u);
static gdouble x_to_view(CmpackCurvePlot *view, gdouble x);
static gdouble view_to_x(CmpackCurvePlot *view, gdouble u);
static gdouble yproj_to_view(CmpackCurvePlot *view, gdouble y);
static gdouble view_to_yproj(CmpackCurvePlot *view, gdouble v);
static gdouble y_to_view(CmpackCurvePlot *view, gdouble y);
static gdouble view_to_y(CmpackCurvePlot *view, gdouble v);

static gboolean set_x_axis(CmpackCurvePlot *view, gdouble zoom, gdouble center);
static gboolean set_y_axis(CmpackCurvePlot *view, gdouble zoom, gdouble center);
static void update_rectangles(CmpackCurvePlot *view);
static void update_x_pxlsize(CmpackCurvePlot *view);
static void restrict_x_to_limits(CmpackCurvePlot *view);
static void update_y_pxlsize(CmpackCurvePlot *view);
static void restrict_y_to_limits(CmpackCurvePlot *view);

static void compute_x_grid_step(CmpackCurvePlot *view, gdouble min, gdouble max, gdouble *prec, gint *step);
static void compute_y_grid_step(CmpackCurvePlot *view, gdouble min, gdouble max, gdouble *prec, gint *step);

static void format_label(gchar *buf, gdouble value, CmpackPlotFormat format, gint prec);
static gint compute_x_label_width(CmpackCurvePlot *view, CmpackPlotXAxis *axis, int prec);
static gint compute_y_label_width(CmpackCurvePlot *view, CmpackPlotYAxis *axis, int prec);
static gint compute_label_height(CmpackCurvePlot *view);

static void paint_data(CmpackCurvePlot *view, GdkDrawable *drawable);
static void paint_x_scale(CmpackCurvePlot *view, GdkDrawable *drawable);
static void paint_y_scale(CmpackCurvePlot *view, GdkDrawable *drawable);
static void paint_x_grid(CmpackCurvePlot *view, GdkDrawable *drawable);
static void paint_y_grid(CmpackCurvePlot *view, GdkDrawable *drawable);

static void update_hsb(CmpackCurvePlot *view);
static void update_vsb(CmpackCurvePlot *view);

static guint curve_plot_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(CmpackCurvePlot, cmpack_curve_plot, GTK_TYPE_WIDGET)

/* Class initialization */
static void cmpack_curve_plot_class_init(CmpackCurvePlotClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	gobject_class->set_property = cmpack_curve_plot_set_property;
	gobject_class->get_property = cmpack_curve_plot_get_property;
	gobject_class->finalize = cmpack_curve_plot_finalize;

	object_class->destroy = cmpack_curve_plot_destroy;

	widget_class->realize = cmpack_curve_plot_realize;
	widget_class->unrealize = cmpack_curve_plot_unrealize;
	widget_class->style_set = cmpack_curve_plot_style_set;
	widget_class->size_allocate = cmpack_curve_plot_size_allocate;
	widget_class->expose_event = cmpack_curve_plot_expose;
	widget_class->motion_notify_event = cmpack_curve_plot_motion;
	widget_class->leave_notify_event = cmpack_curve_plot_leave;
	widget_class->button_press_event = cmpack_curve_plot_button_press;
	widget_class->button_release_event = cmpack_curve_plot_button_release;
	widget_class->key_press_event = cmpack_curve_plot_key_press;
	widget_class->key_release_event = cmpack_curve_plot_key_release;
	widget_class->state_changed = cmpack_curve_plot_state_changed;
	widget_class->scroll_event = cmpack_curve_plot_scroll;

	klass->set_scroll_adjustments = cmpack_curve_plot_set_adjustments;
  
	/* Properties */
	g_object_class_install_property (gobject_class, PROP_SELECTION_MODE, 
		g_param_spec_enum("selection-mode", "Selection mode", "The selection mode",
			GTK_TYPE_SELECTION_MODE, GTK_SELECTION_SINGLE, 
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_MODEL,
		g_param_spec_object("model", "Graph Data Model", "The model for the curve plot",
			CMPACK_TYPE_GRAPH_DATA, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	/* Signals */
	widget_class->set_scroll_adjustments_signal = 
		g_signal_new ("set-scroll-adjustments", G_OBJECT_CLASS_TYPE (gobject_class),
			GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
			G_STRUCT_OFFSET (CmpackCurvePlotClass, set_scroll_adjustments),
			NULL, NULL, cmpack_curve_plot_marshal_VOID__OBJECT_OBJECT,
			G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT); 

	curve_plot_signals[ITEM_ACTIVATED] =
		g_signal_new ("item-activated", G_TYPE_FROM_CLASS (gobject_class),
			G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CmpackCurvePlotClass, item_activated),
			NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, GTK_TYPE_INT);

	curve_plot_signals[SELECTION_CHANGED] =
		g_signal_new ("selection-changed", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackCurvePlotClass, selection_changed),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	curve_plot_signals[MOUSE_MOVED] =
		g_signal_new ("mouse-moved", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackCurvePlotClass, mouse_moved),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Graph view initialization */
static void cmpack_curve_plot_init(CmpackCurvePlot *view)
{
	view->zoom_base = pow(100.0, 1.0/100.0);
	view->last_mouse_x = view->last_mouse_y = -1;
	view->last_single_clicked = -1;
	view->select_first = view->select_last = -1;
	view->int_colors = (GdkColor*)g_malloc(GRAPH_N_COLORS*sizeof(GdkColor));
	cmpack_curve_plot_set_adjustments(view, NULL, NULL);
}

static void cmpack_curve_plot_destroy(GtkObject *object)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (object);

	/* Disconnect the model */
	cmpack_curve_plot_set_model(view, NULL);
  
	/* Disconnect the adjustments */
	if (view->hadjustment != NULL) {
		g_object_unref (view->hadjustment);
		view->hadjustment = NULL;
	}
	if (view->vadjustment != NULL) {
		g_object_unref (view->vadjustment);
		view->vadjustment = NULL;
	}
  
	GTK_OBJECT_CLASS(cmpack_curve_plot_parent_class)->destroy(object);
}

static void cmpack_curve_plot_finalize(GObject *object)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(object);

	/* Free buffered data */
	clear_data(view);

	/* Free pixmaps */
	if (view->offscreen_pixmap)
		g_object_unref(view->offscreen_pixmap);

	/* Free allocated memory */
	g_free(view->x.Name);
	g_free(view->y.Name);
	g_free(view->int_colors);

	G_OBJECT_CLASS(cmpack_curve_plot_parent_class)->finalize(object);
} 

/* Set property */
static void cmpack_curve_plot_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  CmpackCurvePlot *view = CMPACK_CURVE_PLOT (object);

	switch (prop_id)
    {
    case PROP_SELECTION_MODE:
		cmpack_curve_plot_set_selection_mode (view, (GtkSelectionMode)g_value_get_enum (value));
		break;
    case PROP_MODEL:
		cmpack_curve_plot_set_model(view, (CmpackGraphData*)g_value_get_object (value));
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

/* Get property */
static void cmpack_curve_plot_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (object);

	switch (prop_id)
    {
    case PROP_SELECTION_MODE:
		g_value_set_enum (value, view->selection_mode);
		break;
    case PROP_MODEL:
		g_value_set_object (value, view->model);
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_curve_plot_realize (GtkWidget *widget)
{
	gboolean int_ok[GRAPH_N_COLORS];
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(widget);
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
	attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
		GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK | GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK;
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_ACTIVE);

	set_rgb(&view->int_colors[GRAPH_COLOR_SELECTED], 0.7, 0.7, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_ZOOMRECT], 1.0, 0.0, 0.0);
	set_rgb(&view->int_colors[GRAPH_COLOR_CURSOR], 0.0, 0.7, 0.8);
	set_rgb(&view->int_colors[GRAPH_COLOR_RED], 1, 0, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_GREEN], 0, 1, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_BLUE], 0, 0, 1);
	set_rgb(&view->int_colors[GRAPH_COLOR_YELLOW], 1, 1, 0);
	gdk_colormap_alloc_colors(gtk_widget_get_colormap(widget), view->int_colors, 
		GRAPH_N_COLORS, FALSE, TRUE, int_ok);

	cmpack_curve_plot_create_buffer(view); 

	update_rectangles(view);
	update_x_pxlsize(view);
	restrict_x_to_limits(view);
	update_y_pxlsize(view);
	restrict_y_to_limits(view);
	update_hsb(view);
	update_vsb(view);
	view->dirty = TRUE;
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_curve_plot_unrealize(GtkWidget *widget)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(widget);

	gdk_colormap_free_colors(gtk_widget_get_colormap(widget), view->int_colors, GRAPH_N_COLORS);

	GTK_WIDGET_CLASS(cmpack_curve_plot_parent_class)->unrealize (widget); 
}

/* Widget state changed */
static void cmpack_curve_plot_state_changed(GtkWidget *widget, GtkStateType previous_state)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	view->dirty = TRUE;
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS (cmpack_curve_plot_parent_class)->state_changed(widget, previous_state);
}

/* Widget style changed */
static void cmpack_curve_plot_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	view->dirty = TRUE; 
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS(cmpack_curve_plot_parent_class)->style_set(widget, previous_style);
}

/* Widget size changed */
static void cmpack_curve_plot_size_allocate(GtkWidget *widget, GtkAllocation  *allocation)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		cmpack_curve_plot_create_buffer(view); 
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		view->dirty = TRUE;
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
}

static void paint_rectangle(CmpackCurvePlot *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, GdkColor *color)
{
	gint i;
	gboolean ok;
	GdkRectangle rect;

	rect.x = MIN (view->mouse_x1, view->mouse_x2);
	rect.y = MIN (view->mouse_y1, view->mouse_y2);
	rect.width = ABS (view->mouse_x1 - view->mouse_x2);
	rect.height = ABS (view->mouse_y1 - view->mouse_y2);

	ok = FALSE;
	for (i=0; i<nrectangles; i++) {
		const GdkRectangle *area = &rectangles[i];
		if (rect.x <= area->x + area->width && rect.y <= area->y + area->height &&
			rect.x + rect.width >= area->x && rect.y + rect.height >= area->y) {
				ok = TRUE;
				break;
		}
	}

	if (ok) {
		GdkGC *gc = gdk_gc_new(drawable);
		gdk_gc_set_line_attributes(gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_gc_set_foreground(gc, color);
		gdk_draw_rectangle(widget->window, gc, FALSE, rect.x, rect.y, rect.width, rect.height);
		gdk_gc_destroy(gc);
	}
}

/* Refresh the offscreen buffer (if needed) and paint it to the window */
static gboolean cmpack_curve_plot_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	CmpackCurvePlot *view;
	GdkRectangle *rectangles;
	gint n_rectangles;     

	if (GTK_WIDGET_DRAWABLE(widget)) {
		view = CMPACK_CURVE_PLOT(widget);
		if (view->dirty) 
			cmpack_curve_plot_paint_buffer(view);
		gdk_draw_drawable (widget->window, widget->style->fg_gc[widget->state],
				   CMPACK_CURVE_PLOT(widget)->offscreen_pixmap,
				   expose->area.x, expose->area.y, expose->area.x, expose->area.y,
				   expose->area.width, expose->area.height);

		gdk_region_get_rectangles(expose->region, &rectangles, &n_rectangles);
		if (view->mouse_mode == PLOT_MOUSE_SELECT) {
			paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
				&view->int_colors[GRAPH_COLOR_CURSOR]);
		} else 
		if (view->mouse_mode == PLOT_MOUSE_ZOOM) {
			paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
				&view->int_colors[GRAPH_COLOR_ZOOMRECT]);
		}
		g_free(rectangles);
	}
	return FALSE; 
}

/* Rebuild the offscreen buffer */
static void cmpack_curve_plot_create_buffer(CmpackCurvePlot *view)
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

/* Make the marker pixmap 
static void cmpack_curve_plot_create_marker(CmpackCurvePlot *view)
{
	GtkWidget *widget;

	if (GTK_WIDGET_REALIZED (view)) {
		widget = GTK_WIDGET (view);
		if (view->marker_pixmap)
			g_object_unref (view->marker_pixmap);
		view->marker_width = MARKER_SIZE;
		view->marker_height = MARKER_SIZE;
		view->marker_pixmap = gdk_pixmap_new(widget->window, 
			view->marker_width, view->marker_height, -1);
		cmpack_curve_plot_paint_marker(view);
	}
}*/

static void cmpack_curve_plot_paint_buffer(CmpackCurvePlot *view)
{
	GtkWidget *widget;

	if (view->offscreen_pixmap) {
		widget = GTK_WIDGET(view);

		// Background
		gdk_draw_rectangle(view->offscreen_pixmap, widget->style->dark_gc[widget->state], 
			TRUE, 0, 0, widget->allocation.width, widget->allocation.height);
		gdk_draw_rectangle(view->offscreen_pixmap, widget->style->base_gc[widget->state],
			TRUE, view->canvas_rc.x, view->canvas_rc.y, view->canvas_rc.width,
			view->canvas_rc.height);

		// Grid
		if (view->x.ShowGrid)
			paint_x_grid(view, view->offscreen_pixmap);
		if (view->y.ShowGrid)
			paint_y_grid(view, view->offscreen_pixmap);

		// Data
		if (view->item_count>0)
			paint_data(view, view->offscreen_pixmap);
		
		// Labels
		if (view->x.ShowLabels)
			paint_x_scale(view, view->offscreen_pixmap);
		if (view->y.ShowLabels)
			paint_y_scale(view, view->offscreen_pixmap);

		view->dirty = FALSE;
	}
} 

/*static void cmpack_curve_plot_paint_marker(CmpackCurvePlot *view)
{
  GtkWidget *widget;
  cairo_t *cr;

	if (view->marker_pixmap) {
		widget = GTK_WIDGET(view);

		cr = gdk_cairo_create(view->marker_pixmap);

		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_rectangle(cr, 0, 0, view->marker_width, view->marker_height); 
		cairo_fill(cr);
		
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_set_line_width(cr, 1.0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
		cairo_arc(cr, view->marker_width / 2, view->marker_height / 2, 
			MIN(view->marker_height / 2, view->marker_width / 2), 
			0.0, 2.0*M_PI);
		cairo_fill(cr);
		cairo_destroy(cr);
	}
} */

static void draw_line(CmpackCurvePlot *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
	if (x2 >= view->canvas_rc.x && x1 < view->canvas_rc.x + view->canvas_rc.width) 
		gdk_draw_line(drawable, gc, x1, y1, x2, y2);
}

static void draw_step(CmpackCurvePlot *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
	GdkPoint pts[3];

	if (x2 >= view->canvas_rc.x && x1 < view->canvas_rc.x + view->canvas_rc.width) {
		if (y1==y2) {
			if (x1!=x2)
				gdk_draw_line(drawable, gc, x1, y1, x2, y1);
		} else {
			if (x1==x2) {
				gdk_draw_line(drawable, gc, x1, y1, x1, y2);
			} else {
				pts[1].x = pts[0].x = x1;
				pts[0].y = y1;
				pts[2].x = x2;
				pts[2].y = pts[1].y = y2;
				gdk_draw_lines(drawable, gc, pts, 3);
			}
		}
	}
}

static void draw_rect(CmpackCurvePlot *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
	if (x2 >= view->canvas_rc.x && x1 < view->canvas_rc.x + view->canvas_rc.width) 
		gdk_draw_rectangle(drawable, gc, TRUE, x1, y1, (x2-x1+1), (y2-y1+1));
}

static void draw_point(CmpackCurvePlot *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, gint x, gint y, const CmpackCurvePlotItem *item, gboolean selected)
{
	GdkSegment sgs[3];

	if (x - MARKER_SIZE/2 >= view->canvas_rc.x && x + MARKER_SIZE/2 <= view->canvas_rc.x + view->canvas_rc.width &&
		y - MARKER_SIZE/2 >= view->canvas_rc.y && y + MARKER_SIZE/2 <= view->canvas_rc.y + view->canvas_rc.height) {
		if (item->error > 0) {
			sgs[2].x1 = sgs[0].x1 = x - MARKER_SIZE/2;
			sgs[2].x2 = sgs[0].x2 = x + MARKER_SIZE/2 - 1;
			sgs[1].x1 = sgs[1].x2 = x;
			sgs[1].y1 = sgs[0].y1 = sgs[0].y2 = RoundToInt(yproj_to_view(view, item->value + item->error));
			sgs[1].y2 = sgs[2].y1 = sgs[2].y2 = RoundToInt(yproj_to_view(view, item->value - item->error));
			gdk_draw_segments(drawable, gc, sgs, 3);
		}
		gdk_draw_arc(drawable, gc, TRUE, x - MARKER_SIZE/2, y - MARKER_SIZE/2, 
			MARKER_SIZE, MARKER_SIZE, 0, 64*360);
	}
}

static void paint_data(CmpackCurvePlot *view, GdkDrawable *drawable)
{
	gint i, x, y, last_x=0, last_y=0, next_x, base_y=0, first=1;
	gboolean selected;
	GtkWidget *widget = GTK_WIDGET(view);

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->fg_gc[widget->state]);
	gdk_gc_set_clip_rectangle(gc, &view->canvas_rc);

	for (i=0; i<view->item_count; i++) {
		CmpackCurvePlotItem *item = view->items + i;
		if (item->visible) {
			selected = (i>=view->select_first && i<=view->select_last);
			x = RoundToInt(xproj_to_view(view, i));
			y = RoundToInt(yproj_to_view(view, item->value));
			gdk_gc_set_foreground(gc, &widget->style->fg[widget->state]);
			if ((view->style==PLOT_LINES || view->style==PLOT_LINES_POINTS) && !first) 
				draw_line(view, widget, drawable, gc, last_x, last_y, x, y);
			if (view->style==PLOT_STEPS) {
				next_x = RoundToInt(xproj_to_view(view, i+1));
				if (first)
					draw_line(view, widget, drawable, gc, x, y, next_x, y);
				else
					draw_step(view, widget, drawable, gc, x, last_y, next_x, y);
			}
			if (view->style==PLOT_HISTOGRAM) {
				next_x = RoundToInt(xproj_to_view(view, i+1));
				if (first)
					base_y = RoundToInt(yproj_to_view(view, 0));
				if (y==base_y)
					draw_line(view, widget, drawable, gc, x, y, next_x, y);
				else if (y<base_y)
					draw_rect(view, widget, drawable, gc, x, y, next_x, base_y);
				else
					draw_rect(view, widget, drawable, gc, x, base_y, next_x, y);
			}
			if (view->style==PLOT_POINTS || view->style==PLOT_LINES_POINTS) 
				draw_point(view, widget, drawable, gc, x, y, item, selected);
			first = 0;
			last_x = x;
			last_y = y;
		}
	}

	gdk_gc_unref(gc);
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

static gdouble LimitDbl(gdouble x, gdouble min, gdouble max)
{
	if (x < min)
		return min;
	else if (x > max)
		return max;
	else
		return x;
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

static void update_rectangles(CmpackCurvePlot *view)
{
	GtkWidget *widget;
	gint x_height, y_width, uh, uw, vh, vw;
	gint cm_left, cm_top, cm_right, cm_bottom;
	gint gm_left, gm_top, gm_right, gm_bottom;

	if (GTK_WIDGET_REALIZED(view)) {
		widget = GTK_WIDGET(view);
	  
		x_height = compute_label_height(view);
		text_extents(widget, view->x.Name, &uw, &uh);
		uw += 8;
		x_height = MAX(x_height, uh) + 4;

		y_width = compute_y_label_width(view, &view->y, view->y.MaxPrec);
		text_extents(widget, view->y.Name, &vw, &vh);
		vw += 8;
		vh += 4;
		y_width = MAX(y_width, vw) + 4;
		
		cm_left = cm_top = cm_right = cm_bottom = 0;
		gm_left = gm_top = gm_right = gm_bottom = 6;

		if (view->x.ShowLabels) {
			if (!view->x.LabelsOpposite) {
				/* X axis is outside bottom */
				cm_bottom += x_height;
			} else {
				/* X axis is outside top */
				cm_top += x_height;
			}
		}
		if (view->y.ShowLabels) {
			if (!view->y.LabelsOpposite) {
				/* X axis is outside left */
				cm_left += y_width;
			} else {
				/* X axis is outside right */
				cm_right += y_width;
			}
		}

		view->canvas_rc.x = cm_left;
		view->canvas_rc.y = cm_top;
		view->canvas_rc.width = widget->allocation.width - cm_left - cm_right;
		view->canvas_rc.height = widget->allocation.height - cm_top - cm_bottom;

		view->graph_rc.x = view->canvas_rc.x + gm_left;
		view->graph_rc.y = view->canvas_rc.y + gm_top;
		view->graph_rc.width = view->canvas_rc.width - gm_left - gm_right;
		view->graph_rc.height = view->canvas_rc.height - gm_top - gm_bottom;

		memset(&view->xscale_rc, 0, sizeof(GdkRectangle));
		if (view->x.ShowLabels) {
			/* X scale is outside */
			view->xscale_rc.x = view->canvas_rc.x;
			view->xscale_rc.width = view->canvas_rc.width;
			if (!view->x.LabelsOpposite) {
				/* X axis is outside bottom */
				view->xscale_rc.y = view->canvas_rc.y + view->canvas_rc.height;
			} else {
				/* X axis is outside top */
				view->xscale_rc.y = 0;
			}
			view->xscale_rc.height = x_height;
		}

		memset(&view->yscale_rc, 0, sizeof(GdkRectangle));
		if (view->y.ShowLabels) {
			/* Y scale is outside */
			view->yscale_rc.y = view->canvas_rc.y;
			view->yscale_rc.height = view->canvas_rc.height;
			if (!view->x.LabelsOpposite) {
				/* X axis is outside left */
				view->yscale_rc.x = 0;
			} else {
				/* X axis is outside right */
				view->yscale_rc.x = view->canvas_rc.x + view->canvas_rc.width;
			}
			view->yscale_rc.width = y_width;
		}

		memset(&view->xname_rc, 0, sizeof(GdkRectangle));
		if (view->x.ShowLabels && view->x.Name) {
			if (view->y.LabelsOpposite && view->y.Name) {
				/* Name is on the left */
				view->xname_rc.x = view->xscale_rc.x;
				view->xscale_rc.x += uw;
			} else {
				/* Name is on the right */
				view->xname_rc.x = view->xscale_rc.x + view->xscale_rc.width - uw;
			}
			view->xname_rc.width = uw;
			view->xname_rc.y = view->xscale_rc.y;
			view->xname_rc.height = view->xscale_rc.height;
			view->xscale_rc.width -= uw;
		}

		memset(&view->yname_rc, 0, sizeof(GdkRectangle));
		if (view->y.ShowLabels && view->y.Name) {
			if (view->x.LabelsOpposite && view->x.Name) {
				/* Name is on the bottom */
				view->yname_rc.y = view->yscale_rc.y + view->yscale_rc.height - vh;
			} else {
				/* Name is on the top */
				view->yname_rc.y = view->yscale_rc.y;
				view->yscale_rc.y += vh;
			}
			view->yname_rc.height = vh;
			view->yname_rc.x = view->yscale_rc.x;
			view->yname_rc.width = view->yscale_rc.width;
			view->yscale_rc.height -= vh;

			view->x.Center = view->graph_rc.x + 0.5 * view->graph_rc.width;
			view->y.Center = view->graph_rc.y + 0.5 * view->graph_rc.height;
		}
	}
}

/* Update mapping coefficients */
static void update_x_pxlsize(CmpackCurvePlot *view)
{	
	gdouble XZoom = pow(view->zoom_base, view->x.ZoomPos);
	if (view->graph_rc.width>0) {
		view->x.PxlSize = view->x.ProjWidth / (view->graph_rc.width * XZoom);
	} else {
		view->x.PxlSize = 0.0;
	}
}

/* Checks if the visible area is inside the limits */
static void restrict_x_to_limits(CmpackCurvePlot *view)
{
	gint left, right;
    gdouble a, b;
	
	right = view->graph_rc.x + view->graph_rc.width;
    a = view->x.Center + (view->x.ProjWidth - view->x.ProjPos)/view->x.PxlSize;
    if (a < right) {
        view->x.ProjPos = view->x.ProjWidth - view->x.PxlSize*(right - view->x.Center);
    } else {
		left = view->graph_rc.x;
        b = view->x.Center - view->x.ProjPos/view->x.PxlSize;
        if (b > left)
            view->x.ProjPos = -view->x.PxlSize*(left - view->x.Center);
    }
}

/* Physical units -> projection units */
static gdouble x_to_proj(CmpackCurvePlot *view, gdouble x)
{
	return (x - view->x.ZeroOffset) / view->x.ChannelWidth;
}

/* Projection units -> physical units */
static gdouble proj_to_x(CmpackCurvePlot *view, gdouble u)
{
	return (u * view->x.ChannelWidth) + view->x.ZeroOffset;
}

/* Projection units -> display units */
static gdouble xproj_to_view(CmpackCurvePlot *view, gdouble x)
{
	return view->x.Center + (x - view->x.ProjPos)/view->x.PxlSize;
}

/* Display units -> projection units */
static gdouble view_to_xproj(CmpackCurvePlot *view, gdouble u)
{
	return (u - view->x.Center)*view->x.PxlSize + view->x.ProjPos;
}

/* Physical units -> display units */
static gdouble x_to_view(CmpackCurvePlot *view, gdouble x)
{
	return xproj_to_view(view, x_to_proj(view, x));
}

/* Physical units -> projection units */
static gdouble view_to_x(CmpackCurvePlot *view, gdouble u)
{
	return proj_to_x(view, view_to_xproj(view, u));
}

/* Update range and position of horizontal scroll bar */
static void update_hsb(CmpackCurvePlot *view)
{
	gdouble upper, page_size, pos;

	GtkAdjustment *adj = view->hadjustment;
	if (adj && view->x.PxlSize>0) {
		upper = RoundToInt(view->x.ProjWidth / view->x.PxlSize);
		page_size = view->graph_rc.width;
		pos = view->x.ProjPos/view->x.PxlSize - view->x.Center + view->graph_rc.x;
		g_object_set(G_OBJECT(adj), "upper", (gdouble)upper, "page-size", (gdouble)page_size, 
			"value", (gdouble)pos, NULL);
	}
}

/* Update mapping coefficients */
static void update_y_pxlsize(CmpackCurvePlot *view)
{	
	gdouble YZoom = pow(view->zoom_base, view->y.ZoomPos);
	if (view->graph_rc.height>0) {
		view->y.PxlSize = (view->y.ProjMax - view->y.ProjMin) / 
			(view->graph_rc.height * YZoom);
	} else {
		view->y.PxlSize = 0.0;
	}
}

/* Checks if the visible area is inside the limits */
static void restrict_y_to_limits(CmpackCurvePlot *view)
{
	gint top, bottom;
    gdouble a, b;

	top = view->graph_rc.y;
    a = view->y.Center - (view->y.ProjMax - view->y.ProjPos)/view->y.PxlSize;
    if (a > top) {
        view->y.ProjPos = view->y.ProjMax - view->y.PxlSize*(view->y.Center - top);
    } else {
		bottom = view->graph_rc.y + view->graph_rc.height;
        b = view->y.Center - (view->y.ProjMin - view->y.ProjPos)/view->y.PxlSize;
        if (b < bottom)
            view->y.ProjPos = view->y.ProjMin - view->y.PxlSize*(view->y.Center - bottom);
    }
}

/* Physical units -> projection units */
static gdouble y_to_proj(CmpackCurvePlot *view, gdouble y)
{
	if (view->y.Log) 
		return (!view->y.Reverse ? 1.0 : -1.0) * log10(y);
	else
		return (!view->y.Reverse ? 1.0 : -1.0) * y;
}

/* Projection units -> physical units */
static gdouble proj_to_y(CmpackCurvePlot *view, gdouble v)
{
	if (view->y.Log) 
		return pow(10.0, (!view->y.Reverse ? 1.0 : -1.0) * v);
	else
		return (!view->y.Reverse ? 1.0 : -1.0) * v;
}

/* Projection units -> display units */
static gdouble yproj_to_view(CmpackCurvePlot *view, gdouble y)
{
	return view->y.Center - (y - view->y.ProjPos)/view->y.PxlSize;
}

/* Display units -> projection units */
static gdouble view_to_yproj(CmpackCurvePlot *view, gdouble v)
{
	return view->y.ProjPos - (v - view->y.Center)*view->y.PxlSize;
}

/* Physical units -> display units */
static gdouble y_to_view(CmpackCurvePlot *view, gdouble y)
{
	return yproj_to_view(view, y_to_proj(view, y));
}

/* Physical units -> projection units */
static gdouble view_to_y(CmpackCurvePlot *view, gdouble v)
{
	return proj_to_y(view, view_to_yproj(view, v));
}

/* Update range and position of vertical scroll bar */
static void update_vsb(CmpackCurvePlot *view)
{
	gdouble upper, page_size, pos;

	GtkAdjustment *adj = view->vadjustment;
	if (adj && view->y.PxlSize>0) {
		upper = RoundToInt((view->y.ProjMax - view->y.ProjMin) / view->y.PxlSize);
		page_size = view->graph_rc.height;
		pos = (view->y.ProjMax - view->y.ProjPos) / view->y.PxlSize + view->graph_rc.y - view->y.Center;
		g_object_set(G_OBJECT(adj), "upper", (gdouble)upper, "page-size", (gdouble)page_size, 
			"value", (gdouble)pos, NULL);
	}
}

/*---------------------   SCALE LABELS & GRIDS   ---------------------------*/

/* Standard formatting function for scale labels */
static void format_label(gchar *buf, gdouble value, CmpackPlotFormat format, gint prec)
{
	int time;

	buf[0] = '\0';
	switch(format) 
	{
	case PLOT_EXP:
        // Always exponential form
		sprintf(buf, "%.*e", prec, value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.*e", prec, 0.0);
		break;

	case PLOT_INT:
		// Integer number without decimal places
		sprintf(buf, "%.0f", value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.0f", 0.0);
		break;

	case PLOT_FIXED:
        // Number with fixed number of decimal places
		sprintf(buf, "%.*f", prec, value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.*f", prec, 0.0);
		break;

	case PLOT_TIME:
        // Time in the form hh:mm:ss
		time = RoundToInt((value - floor(value))*86400.0);
		sprintf(buf, "%d:%02d:%02d", time/3600, (time/60)%60, time%60);
		break;
	}
} 

/* Compute max. width of scale label */
static gint compute_x_label_width(CmpackCurvePlot *view, CmpackPlotXAxis *axis, int prec)
{
	gchar	buf[256];
    gint	w1, w2;

	format_label(buf, proj_to_x(view, 0), axis->Format, prec);
	w1 = text_width(GTK_WIDGET(view), buf);
	format_label(buf, proj_to_x(view, axis->ProjWidth), axis->Format, prec);
	w2 = text_width(GTK_WIDGET(view), buf);
	return MAX(w1, w2);
}

/* Compute max. width of scale label */
static gint compute_y_label_width(CmpackCurvePlot *view, CmpackPlotYAxis *axis, int prec)
{
	gchar	buf[256];
    gint	w1, w2;

	format_label(buf, axis->Min, axis->Format, prec);
	w1 = text_width(GTK_WIDGET(view), buf);
	format_label(buf, axis->Max, axis->Format, prec);
	w2 = text_width(GTK_WIDGET(view), buf);
	return MAX(w1, w2);
}

/* Compute height of scale label */
static gint compute_label_height(CmpackCurvePlot *view)
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
static void compute_x_grid_step(CmpackCurvePlot *view, gdouble minval, gdouble maxval, 
								gdouble *step, gint *prec)
{
    gint i, cx, nlines, xprec, oldprec;
	gdouble xstep, dx, delta;

	xprec = view->x.MinPrec;
	xstep = fabs(maxval - minval);

	switch (view->x.Format)
	{
	case PLOT_TIME:
        // Time mode
		cx = compute_x_label_width(view, &view->x, 0);
		nlines = (int)(view->canvas_rc.width/((cx+1)*2.2)) - 1;
		if (nlines<2) nlines = 2;
		for (i=NumTSteps-1; i>0; i--) {
			if ((fabs(maxval-minval)/TSteps[i])>nlines)
				break;
		}
		xstep = TSteps[i];
		break;

	case PLOT_INT:
		// Integer number without decimal places
		dx = 3.0*(compute_x_label_width(view, &view->x, xprec)+4)*view->x.PxlSize*view->x.ChannelWidth;
		delta = pow(10.0, floor(log10(dx)));
		if (delta<1)
			delta = 1;
		if (delta>dx)
			xstep = delta;
		else if (2.0 * delta>dx) 
			xstep = 2.0 * delta;
		else if (5.0 * delta>dx) 
			xstep = 5.0 * delta;
		else
			xstep = 10.0 * delta;
		break;

	default:
        // Decimal mode
		xprec = view->x.MinPrec;
		xstep = fabs(maxval - minval);
		do {
			oldprec = xprec;
			dx = 3.0*(compute_x_label_width(view, &view->x, xprec)+4)*view->x.PxlSize*view->x.ChannelWidth;
			delta = pow(10.0, floor(log10(dx)));
			if (delta>dx)
				xstep = delta;
			else if (2.0 * delta>dx)
				xstep = 2.0 * delta;
			else if (5.0 * delta>dx)
				xstep = 5.0 * delta;
			else
				xstep = 10.0 * delta;
			xprec = LimitInt(step_to_prec(xstep), view->x.MinPrec, view->x.MaxPrec);
		} while (xprec > oldprec);
		break;
	}

	if (step)
		*step = xstep;
	if (prec)
		*prec = xprec;
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_y_grid_step(CmpackCurvePlot *view, gdouble minval, gdouble maxval, 
								gdouble *step, gint *prec)
{
	gdouble dy, ystep, delta;

	dy = 4.0*(compute_label_height(view)+2)*view->y.PxlSize;

	switch (view->y.Format)
	{
	case PLOT_INT:
        // Integer mode without decimal places
		delta = pow(10.0, floor(log10(dy)));
		if (delta<1)
			delta = 1;
		if (delta>dy) 
			ystep = delta;
		else if (2.0 * delta>dy)
			ystep = 2.0 * delta;
		else if (5.0 * delta>dy)
			ystep = 5.0 * delta;
		else
			ystep = 10.0 * delta;
		break;

	default:
        // Decimal mode
		delta = pow(10.0, floor(log10(dy)));
		if (delta>dy)
			ystep = delta;
		else if (2.0 * delta>dy)
			ystep = 2.0 * delta;
		else if (5.0 * delta>dy)
			ystep = 5.0 * delta;
		else
			ystep = 10.0 * delta;
		break;
	}

	if (step)
		*step = ystep;
	if (prec) 
		*prec = LimitInt(step_to_prec(ystep), view->y.MinPrec, view->y.MaxPrec);
}

//------------------------   PAINTING   -----------------------------------

static void draw_text(GtkWidget *widget, GdkDrawable *drawable, GdkGC *gc, 
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
	gdk_draw_layout(drawable, gc, x, y, layout);
	g_object_unref(layout);
}

/* Paint labels on x-axis scale */
static void paint_x_scale(CmpackCurvePlot *view, GdkDrawable *drawable)
{
	gchar	buf[256];
	gdouble	val, step, xmin, xmax;
	gint	x, y, left, right, prec;

	GtkWidget *widget = GTK_WIDGET(view);
	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->text_gc[widget->state]);

    /* Visible region in physical units */
	xmin = view_to_x(view, view->canvas_rc.x);
	xmax = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);
	
	/* Print unit name */
	if (view->x.Name) {
		gdk_gc_set_clip_rectangle(gc, &view->xname_rc);
		draw_text(widget, drawable, gc, view->xname_rc.x + view->xname_rc.width - 4, view->xname_rc.y + 2,
			view->x.Name, 1.0, 0.0);
	}

	/* Set clipping region */
	gdk_gc_set_clip_rectangle(gc, &view->xscale_rc);
	y = view->xscale_rc.y + 2;
	left = view->xscale_rc.x;
	right = view->xscale_rc.x + view->xscale_rc.width;

	compute_x_grid_step(view, xmin, xmax, &step, &prec);
	xmin = ceil(xmin/step)*step;
	while (xmin <= xmax) {
		val = xmin;
		x = RoundToInt(x_to_view(view, val));
		if (x >= left && x < right) {
			format_label(buf, val, view->x.Format, prec);
			draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
		}
		xmin += step;
	}

	g_object_unref(gc);
}

/* Paint labels on y-axis scale */
static void paint_y_scale(CmpackCurvePlot *view, GdkDrawable *drawable)
{
	gchar	buf[256];
	gdouble	val, step, ymin, ymax;
	gint	x, y, top, bottom, prec;

	GtkWidget *widget = GTK_WIDGET(view);
	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->text_gc[widget->state]);

    /* Visible region in physical units */
	if (!view->y.Reverse) {
		ymin = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height); 
		ymax = view_to_y(view, view->canvas_rc.y);
	} else {
		ymin = view_to_y(view, view->canvas_rc.y); 
		ymax = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height);
	}	
	ymin = LimitDbl(ymin, view->y.Min, view->y.Max);
	ymax = LimitDbl(ymax, view->y.Min, view->y.Max);

	/* Print unit name */
	if (view->y.Name) {
		gdk_gc_set_clip_rectangle(gc, &view->yname_rc);
		draw_text(widget, drawable, gc, view->yname_rc.x + view->yname_rc.width - 4, 
			view->yname_rc.y + 2, view->y.Name, 1.0, 0.0);
	}

	/* Set clipping region */
	gdk_gc_set_clip_rectangle(gc, &view->yscale_rc);
	x = view->yscale_rc.x + view->yscale_rc.width - 4;
	top = view->yscale_rc.y;
	bottom = view->yscale_rc.y + view->yscale_rc.height;

	if (view->y.Log && log10(ymax/ymin)>3) {
		// Log scale if range is greater than three periods
		ymin = log10(MAX(DBL_MIN, ymin));
		ymax = log10(MAX(DBL_MIN, ymax));
		compute_y_grid_step(view, ymin, ymax, &step, &prec);
		ymin = ceil(ymin);
		while (ymin <= ymax) {
			val = pow(10, ymin);
			y = RoundToInt(y_to_view(view, val));
			if (y >= top && y < bottom) {
				format_label(buf, val, view->y.Format, prec);
				draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
			}
			ymin += step;
		}
	} else
	if (view->y.Log && log10(ymax/ymin)>1) {
		// Log scale if range is greater than one period
		ymin = log10(MAX(DBL_MIN, ymin));
		ymax = log10(MAX(DBL_MIN, ymax));
		step = 1.0;
		prec = 1;
		ymin = floor(ymin);
		while (ymin <= ymax) {
			val = pow(10, ymin);
			y = RoundToInt(y_to_view(view, 1.0*val));
			if (y >= top && y < bottom) {
				format_label(buf, 1.0*val, view->y.Format, prec);
				draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
			}
			y = RoundToInt(y_to_view(view, 2.0*val));
			if (y >= top && y < bottom) {
				format_label(buf, 2.0*val, view->y.Format, prec);
				draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
			}
			y = RoundToInt(y_to_view(view, 5.0*val));
			if (y >= top && y < bottom) {
				format_label(buf, 5.0*val, view->y.Format, prec);
				draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
			}
			ymin += step;
		}
	} else {
		// Lin scale or log scale if range is smaller than one period
		compute_y_grid_step(view, ymin, ymax, &step, &prec);
		ymin = ceil(ymin/step)*step;
		while (ymin <= ymax) {
			val = ymin;
			y = RoundToInt(y_to_view(view, val));
			if (y >= top && y < bottom) {
				format_label(buf, val, view->y.Format, prec);
				draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
			}
			ymin += step;
		}
	}

	g_object_unref(gc);
}

/* Paints x-axis grid */
static void paint_x_grid(CmpackCurvePlot *view, GdkDrawable *drawable)
{
	gdouble	val, step, xmin, xmax;
	gint	x, left, right, top, bottom;

	GtkWidget *widget = GTK_WIDGET(view);

	/* Visible region in physical units */
	xmin = view_to_x(view, view->canvas_rc.x);
	xmax = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);

	left = view->canvas_rc.x;
	right = view->canvas_rc.x + view->canvas_rc.width;
	top = view->canvas_rc.y;
	bottom = view->canvas_rc.y + view->canvas_rc.height;

	// Lin scale or log scale if range is smaller than one period
	compute_x_grid_step(view, xmin, xmax, &step, NULL);
	xmin = ceil(xmin/step)*step;
	while (xmin <= xmax) {
		val = xmin;
		x = RoundToInt(x_to_view(view, val));
		if (x >= left && x < right)
			gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
		xmin += step;
	}
}

/* Paint y-axis grid */
static void paint_y_grid(CmpackCurvePlot *view, GdkDrawable *drawable)
{
	gdouble	val, step, ymin, ymax;
	gint	y, left, right, top, bottom;

	GtkWidget *widget = GTK_WIDGET(view);

    /* Visible region in physical units */
	if (!view->y.Reverse) {
		ymin = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height); 
		ymax = view_to_y(view, view->canvas_rc.y);
	} else {
		ymin = view_to_y(view, view->canvas_rc.y); 
		ymax = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height);
	}	
	ymin = LimitDbl(ymin, view->y.Min, view->y.Max);
	ymax = LimitDbl(ymax, view->y.Min, view->y.Max);

	/* Set clipping region */
	left = view->canvas_rc.x;
	right = view->canvas_rc.x + view->canvas_rc.width;
	top = view->canvas_rc.y;
	bottom = view->canvas_rc.y + view->canvas_rc.height;

	if (view->y.Log && log10(ymax/ymin)>3) {
		// Log scale if range is greater than three periods
		ymin = log10(MAX(DBL_MIN, ymin));
		ymax = log10(MAX(DBL_MIN, ymax));
		compute_y_grid_step(view, ymin, ymax, &step, NULL);
		ymin = ceil(ymin);
		while (ymin <= ymax) {
			val = pow(10, ymin);
			y = RoundToInt(y_to_view(view, val));
			if (y >= top && y < bottom)
				gdk_draw_line(drawable, widget->style->dark_gc[widget->state], left, y, right, y);
			ymin += step;
		}
	} else
	if (view->y.Log && log10(ymax/ymin)>1) {
		// Log scale if range is greater than one period
		ymin = log10(MAX(DBL_MIN, ymin));
		ymax = log10(MAX(DBL_MIN, ymax));
		step = 1.0;
		ymin = floor(ymin);
		while (ymin <= ymax) {
			val = pow(10, ymin);
			y = RoundToInt(y_to_view(view, 1.0*val));
			if (y >= top && y < bottom)
				gdk_draw_line(drawable, widget->style->dark_gc[widget->state], left, y, right, y);
			y = RoundToInt(y_to_view(view, 2.0*val));
			if (y >= top && y < bottom)
				gdk_draw_line(drawable, widget->style->dark_gc[widget->state], left, y, right, y);
			y = RoundToInt(y_to_view(view, 5.0*val));
			if (y >= top && y < bottom)
				gdk_draw_line(drawable, widget->style->dark_gc[widget->state], left, y, right, y);
			ymin += step;
		}
	} else {
		// Lin scale or log scale if range is smaller than one period
		compute_y_grid_step(view, ymin, ymax, &step, NULL);
		ymin = ceil(ymin/step)*step;
		while (ymin <= ymax) {
			val = ymin;
			y = RoundToInt(y_to_view(view, val));
			if (y >= top && y < bottom)
				gdk_draw_line(drawable, widget->style->dark_gc[widget->state], left, y, right, y);
			ymin += step;
		}
	}
}

static gboolean valid_mouse_pos(CmpackCurvePlot *view, gint x, gint y)
{
	double a, b;

	if (x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
		x < view->canvas_rc.x + view->canvas_rc.width &&
		y < view->canvas_rc.y + view->canvas_rc.height) {
			a = view_to_xproj(view, x);
			if (a < 0 || a > view->x.ProjWidth)
				return FALSE;
			b = view_to_yproj(view, y);
			if (b < view->y.ProjMin || b > view->y.ProjMax)
				return FALSE;
			return TRUE;
	}
	return FALSE;
}

static gboolean cmpack_curve_plot_motion (GtkWidget *widget, GdkEventMotion *event)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(widget);
	GdkRectangle old_area, new_area, common;
	GdkRegion *invalid_region, *common_region;
	gint x, y;

	gdk_window_get_pointer (widget->window, &x, &y, NULL);

	if (view->mouse_mode == PLOT_MOUSE_ZOOM || view->mouse_mode == PLOT_MOUSE_SELECT) {
		x = LimitInt(x, view->canvas_rc.x, view->canvas_rc.x + view->canvas_rc.width);
		y = LimitInt(y, view->canvas_rc.y, view->canvas_rc.y + view->canvas_rc.height);

		old_area.x = MIN (view->mouse_x1, view->mouse_x2);
		old_area.y = MIN (view->mouse_y1, view->mouse_y2);
		old_area.width = ABS (view->mouse_x2 - view->mouse_x1) + 1;
		old_area.height = ABS (view->mouse_y2 - view->mouse_y1) + 1;
  
		new_area.x = MIN (view->mouse_x1, x);
		new_area.y = MIN (view->mouse_y1, y);
		new_area.width = ABS (x - view->mouse_x1) + 1;
		new_area.height = ABS (y - view->mouse_y1) + 1;

		invalid_region = gdk_region_rectangle (&old_area);
		gdk_region_union_with_rect (invalid_region, &new_area);
		gdk_rectangle_intersect (&old_area, &new_area, &common);
		if (common.width > 2 && common.height > 2) {
			/* make sure the border is invalidated */
			common.x += 1;
			common.y += 1;
			common.width -= 2;
			common.height -= 2;
			common_region = gdk_region_rectangle (&common);
			gdk_region_subtract (invalid_region, common_region);
			gdk_region_destroy (common_region);
		}
		gdk_window_invalidate_region (widget->window, invalid_region, TRUE);
		gdk_region_destroy (invalid_region);

		view->mouse_x2 = x;
		view->mouse_y2 = y;  
	} else
	if (view->mouse_mode == PLOT_MOUSE_SHIFT) {
		x = LimitInt(x, view->canvas_rc.x, view->canvas_rc.x + view->canvas_rc.width);
		y = LimitInt(y, view->canvas_rc.y, view->canvas_rc.y + view->canvas_rc.height);
		view->x.ProjPos = view->mouse_posx + view->x.PxlSize*(view->mouse_x1 - x);
		view->y.ProjPos = view->mouse_posy - view->y.PxlSize*(view->mouse_y1 - y);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
	}

	if (valid_mouse_pos(view, x, y)) {
		if (x!=view->last_mouse_x || y!=view->last_mouse_y) {
			view->last_mouse_x = x;
			view->last_mouse_y = y;
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
	} else {
		if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
			view->last_mouse_x = view->last_mouse_y = -1;
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
	}
	return FALSE;
}

static gboolean cmpack_curve_plot_leave(GtkWidget *widget, GdkEventCrossing *event)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(widget);

	if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
		view->last_mouse_x = view->last_mouse_y = -1;
		g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
	return FALSE;
}

static gboolean cmpack_curve_plot_button_press (GtkWidget *widget, GdkEventButton *event)
{
	gint x, y;
	CmpackCurvePlot *view;
	gboolean dirty = FALSE;

	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);

	view = CMPACK_CURVE_PLOT (widget);
	x = (gint)event->x;
	y = (gint)event->y;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		if (x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
			x < view->canvas_rc.x + view->canvas_rc.width &&
			y < view->canvas_rc.y + view->canvas_rc.height) {
				if (view->mouse_ctrl && event->state & GDK_CONTROL_MASK) {
					// Start zoom to region mode
					cmpack_curve_plot_enter_mouse_mode(view, x, y, PLOT_MOUSE_ZOOM);
				} else 
				if (event->state & GDK_SHIFT_MASK) {
					if (view->selection_mode != GTK_SELECTION_NONE) {
						// Start select region mode
						cmpack_curve_plot_enter_mouse_mode(view, x, y, PLOT_MOUSE_SELECT);
					}
				} else 
				if (view->selection_mode || view->activation_mode) {
					gint index = (int)view_to_xproj(view, x);
					if (index>=0) {
						// Single click selects item
						if (view->selection_mode != GTK_SELECTION_NONE) {
							if (index<view->select_first || index>view->select_last) {
								cmpack_curve_plot_unselect_all_internal(view);
								view->select_first = view->select_last = index;
								cmpack_curve_plot_queue_draw_item (view, index, view->items+index);
								dirty = TRUE;
							}
						}
						// Activation by single left click
						if (view->activation_mode == CMPACK_ACTIVATION_CLICK) 
							cmpack_curve_plot_item_activate(view, index);
						// Remember last item
						if (view->last_single_clicked < 0)
							view->last_single_clicked = index;
					} else {
						// Single click deselect item
						if (view->selection_mode != GTK_SELECTION_NONE && view->selection_mode != GTK_SELECTION_BROWSE)
							dirty = cmpack_curve_plot_unselect_all_internal(view);
						// Clear last item
						if (view->last_single_clicked >= 0)
							view->last_single_clicked = -1;
						// Start panning mode
						if (view->mouse_ctrl)
							cmpack_curve_plot_enter_mouse_mode(view, x, y, PLOT_MOUSE_SHIFT);
					}
				} else {
					// Start panning mode
					if (view->mouse_ctrl)
						cmpack_curve_plot_enter_mouse_mode(view, x, y, PLOT_MOUSE_SHIFT);
				}
		}
	}

	if (event->button==1 && event->type==GDK_2BUTTON_PRESS &&
		view->activation_mode == CMPACK_ACTIVATION_DBLCLICK &&
		x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
		x < view->canvas_rc.x + view->canvas_rc.width &&
		y < view->canvas_rc.y + view->canvas_rc.height) {
			// Left button double click
			gint index = (int)view_to_xproj(view, x);
			if (index>=0 && index == view->last_single_clicked) 
				cmpack_curve_plot_item_activate(view, index);
			view->last_single_clicked = -1;
	}

	if (dirty)
		g_signal_emit (view, curve_plot_signals[SELECTION_CHANGED], 0);

	return event->button == 1; 
}

static gboolean cmpack_curve_plot_button_release(GtkWidget *widget, GdkEventButton *event)
{
	gint x0, x1;
	gboolean dirty;
	gdouble pos, zoom;
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);
	GdkRectangle rect;
  
	if (view->mouse_mode) {
		rect.x = MIN(view->mouse_x1, view->mouse_x2);
		rect.y = MIN(view->mouse_y1, view->mouse_y2);
		rect.width = ABS(view->mouse_x2 - view->mouse_x1) + 1;
		rect.height = ABS(view->mouse_y2 - view->mouse_y1) + 1;

		switch (view->mouse_mode)
		{
		case PLOT_MOUSE_SELECT:
			// Select points in an area
			x0 = (int)view_to_xproj(view, rect.x);
			x1 = (int)ceil(view_to_xproj(view, rect.x + rect.width));
			if (x0!=view->select_first || x1!=view->select_last) {
				view->select_first = x0;
				view->select_last = x1;
				g_signal_emit(view, curve_plot_signals[SELECTION_CHANGED], 0);
			}
			invalidate(view);
			break;

		case PLOT_MOUSE_ZOOM:
			// Zoom to selected area 
			dirty = FALSE;
			pos = view_to_xproj(view, rect.x + rect.width*0.5);
			zoom = log(view->x.ProjWidth/(rect.width*view->x.PxlSize))/log(view->zoom_base);
			dirty |= set_x_axis(view, zoom, pos);
			pos = view_to_yproj(view, rect.y + rect.height*0.5);
			zoom = log((view->y.ProjMax-view->y.ProjMin)/(rect.height*view->y.PxlSize))/log(view->zoom_base);
			dirty |= set_y_axis(view, zoom, pos);
			invalidate(view);
			if (dirty && (view->last_mouse_x>=0 || view->last_mouse_y>=0))
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
			break;

		default:
			break;
		}
		cmpack_curve_plot_leave_mouse_mode(view);
	}
	return TRUE;
}

static gboolean cmpack_curve_plot_key_press (GtkWidget *widget, GdkEventKey *event)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	if (view->mouse_mode) {
		if (event->keyval == GDK_Escape) {
			if (view->mouse_mode == PLOT_MOUSE_SELECT) {
				view->select_first = view->selected_before_rubberbanding[0];
				view->select_last = view->selected_before_rubberbanding[1];
				invalidate(view);
			}
			cmpack_curve_plot_leave_mouse_mode(view);
		}
		return TRUE;
	}
	return GTK_WIDGET_CLASS (cmpack_curve_plot_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_curve_plot_key_release (GtkWidget *widget, GdkEventKey *event)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	if (view->mouse_mode)
		return TRUE;

	return GTK_WIDGET_CLASS (cmpack_curve_plot_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_curve_plot_scroll(GtkWidget *widget, GdkEventScroll *event)
{
	gboolean dirty = false;
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT (widget);

	switch (event->direction)
	{
	case GDK_SCROLL_UP:
		// Center + increase zoom
		if (view->mouse_ctrl) {
			if ((event->state & GDK_CONTROL_MASK)==0 && (view->x.ZoomPos < view->x.ZoomMax))
				dirty |= set_x_axis(view, view->x.ZoomPos + 2.0, view->x.ProjPos);
			if (view->y.ZoomPos < view->y.ZoomMax)
				dirty |= set_y_axis(view, view->y.ZoomPos + 2.0, view->y.ProjPos); 
			if (dirty) {
				invalidate(view);
				if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
					g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
			}
		}
		break;

	case GDK_SCROLL_DOWN:
		// Decrease zoom
		if (view->mouse_ctrl) {
			if ((event->state & GDK_CONTROL_MASK)==0 && (view->x.ZoomPos > 0)) 
				dirty |= set_x_axis(view, view->x.ZoomPos - 2.0, view->x.ProjPos);
			if (view->y.ZoomPos > 0) 
				dirty |= set_y_axis(view, view->y.ZoomPos - 2.0, view->y.ProjPos); 
			if (dirty) {
				invalidate(view);
				if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
					g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
			}
		}
		break;

	default:
		break;
	}

	return TRUE;
}

static void row_inserted(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(data);

	if (row>=0 && row<=view->item_count) {
		if (view->item_count >= view->item_capacity) {
			view->item_capacity += 64;
			view->items = (CmpackCurvePlotItem*)g_realloc(view->items, view->item_capacity*sizeof(CmpackCurvePlotItem));
		}
		if (row < view->item_count)
			memmove(view->items+(row+1), view->items+row, (view->item_count-row)*sizeof(CmpackCurvePlotItem));
		memset(view->items + row, 0, sizeof(CmpackCurvePlotItem));
		update_item(view, view->items + row, model, row);
		view->item_count++;
		view->x.ProjWidth = view->item_count;
		view->x.ZoomMax = log(view->x.ProjWidth/view->x.ProjEps) / log(view->zoom_base);
		view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
		view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			update_rectangles(view);
			update_x_pxlsize(view);
			restrict_x_to_limits(view);
			update_y_pxlsize(view);
			restrict_y_to_limits(view);
			update_hsb(view);
			update_vsb(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
			invalidate(view);
		}
	}
}

static void row_updated(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(data);

	if (row>=0 && row<view->item_count) {
		update_item(view, view->items + row, model, row);
		invalidate(view);
	}
} 

static void row_deleted(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(data);

	if (row>=0 && row<view->item_count) {
		if (row < view->item_count-1) 
			memmove(view->items+row, view->items+(row+1), (view->item_count-row-1)*sizeof(CmpackCurvePlotItem));
		view->item_count--;
		view->x.ProjWidth = view->item_count;
		view->x.ZoomMax = log(view->x.ProjWidth/view->x.ProjEps) / log(view->zoom_base);
		view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
		view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			update_rectangles(view);
			update_x_pxlsize(view);
			restrict_x_to_limits(view);
			update_y_pxlsize(view);
			restrict_y_to_limits(view);
			update_hsb(view);
			update_vsb(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
			invalidate(view);
		}
	}
} 

static void data_cleared(CmpackGraphData *model, gpointer data)
{
	CmpackCurvePlot *view = CMPACK_CURVE_PLOT(data);

	clear_data(view);
	invalidate(view);
} 

static void cmpack_curve_plot_enter_mouse_mode(CmpackCurvePlot  *view,
				   gint          x,
				   gint          y, 
				   CmpackPlotMouseMode mode)
{
	if (!view->mouse_mode) {
		if (mode == PLOT_MOUSE_SELECT) {
			view->selected_before_rubberbanding[0] = view->select_first;
			view->selected_before_rubberbanding[1] = view->select_last;
		}
		view->mouse_mode = mode;
		view->mouse_x2 = view->mouse_x1 = x;
		view->mouse_y2 = view->mouse_y1 = y;
		view->mouse_posx = view->x.ProjPos;
		view->mouse_posy = view->y.ProjPos;
		gtk_grab_add (GTK_WIDGET (view));
	}
}

static void cmpack_curve_plot_leave_mouse_mode(CmpackCurvePlot *view)
{
	if (view->mouse_mode) {
		view->mouse_mode = PLOT_MOUSE_NONE;
		gtk_grab_remove (GTK_WIDGET (view));
		gtk_widget_queue_draw (GTK_WIDGET (view));
	}
}

static gboolean cmpack_curve_plot_unselect_all_internal (CmpackCurvePlot  *view)
{
	gint i;

	if (view->selection_mode == GTK_SELECTION_NONE)
		return FALSE;

	if (view->select_first>=0) {
		for (i=view->select_first; i<=view->select_last && i<view->item_count; i++) 
			cmpack_curve_plot_queue_draw_item(view, i, view->items + i);
		view->select_first = view->select_last = -1;
		return TRUE;
	}
	return FALSE;
}

/* CmpackCurvePlot signals */
static void cmpack_curve_plot_set_adjustments (CmpackCurvePlot   *view,
			       GtkAdjustment *hadj,
			       GtkAdjustment *vadj)
{
	gboolean need_adjust = FALSE;

	if (hadj)
		g_return_if_fail (GTK_IS_ADJUSTMENT (hadj));
	else
		hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	if (vadj)
		g_return_if_fail (GTK_IS_ADJUSTMENT (vadj));
	else
		vadj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

	if (view->hadjustment && (view->hadjustment != hadj)) {
		g_signal_handlers_disconnect_matched (view->hadjustment, G_SIGNAL_MATCH_DATA,
					   0, 0, NULL, NULL, view);
		g_object_unref (view->hadjustment);
	}

	if (view->vadjustment && (view->vadjustment != vadj)) {
		g_signal_handlers_disconnect_matched (view->vadjustment, G_SIGNAL_MATCH_DATA,
					0, 0, NULL, NULL, view);
		g_object_unref (view->vadjustment);
	}

	if (view->hadjustment != hadj) {
		view->hadjustment = hadj;
		g_object_ref_sink (view->hadjustment);
		g_signal_connect (view->hadjustment, "value-changed",
			G_CALLBACK (cmpack_curve_plot_adjustment_changed), view);
		g_object_set(G_OBJECT(hadj), "lower", (gdouble)0, "page-increment", (gdouble)10, "step-increment", 
			(gdouble)1, NULL);
		need_adjust = TRUE;
    }
	if (view->vadjustment != vadj) {
		view->vadjustment = vadj;
		g_object_ref_sink (view->vadjustment);
		g_signal_connect (view->vadjustment, "value-changed",
			G_CALLBACK (cmpack_curve_plot_adjustment_changed), view);
		g_object_set(G_OBJECT(vadj), "lower", (gdouble)0, "page-increment", (gdouble)10, "step-increment", 
			(gdouble)1, NULL);
		need_adjust = TRUE;
	}
	if (need_adjust)
		cmpack_curve_plot_adjustment_changed (NULL, view);
}

/* Handles scroll bars */
static void cmpack_curve_plot_adjustment_changed (GtkAdjustment *adjustment, CmpackCurvePlot *view)
{
	gdouble pos;
	gboolean dirty = FALSE;

	if (!adjustment || adjustment == view->hadjustment) {
		/* HSB */
		pos = view->x.PxlSize * (view->hadjustment->value + view->x.Center - view->graph_rc.x);
		if (view->x.ProjPos != pos) {
			view->x.ProjPos = pos;
			restrict_x_to_limits(view);
			dirty = TRUE;
		}
	}
	if (!adjustment || adjustment == view->vadjustment) {
		/* VSB */
		pos = view->y.ProjMax - view->y.PxlSize * (view->vadjustment->value + view->y.Center - view->graph_rc.y);
		if (view->y.ProjPos != pos) {
			view->y.ProjPos = pos;
			restrict_x_to_limits(view);
			dirty = TRUE;
		}
	}
	if (dirty) {
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
}

static void cmpack_curve_plot_queue_draw_item (CmpackCurvePlot *view, gint row, CmpackCurvePlotItem *item)
{
	GdkRectangle rect;

	rect.x = RoundToInt(xproj_to_view(view, row)) - MARKER_SIZE/2;
	rect.y = RoundToInt(yproj_to_view(view, item->value)) - MARKER_SIZE/2;
	rect.width = MARKER_SIZE;
	rect.height = MARKER_SIZE;

	view->dirty = TRUE;
	gdk_window_invalidate_rect(GTK_WIDGET(view)->window, &rect, TRUE);
}

/* VOID:OBJECT,OBJECT */
static void cmpack_curve_plot_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
	register GMarshalFunc_VOID__OBJECT_OBJECT callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;

	g_return_if_fail (n_param_values == 3);

	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	} else {
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
    }

	callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);
	callback (data1, g_value_get_object (param_values + 1),
		g_value_get_object (param_values + 2), data2);
}

static void clear_data(CmpackCurvePlot *view)
{
	g_free(view->items);
	view->items = NULL;
	view->item_count = view->item_capacity = 0;
	view->last_single_clicked = -1;
	view->x.ProjWidth = 1.0;
	view->x.ZoomMax = log(view->x.ProjWidth/view->x.ProjEps) / log(view->zoom_base);
	view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
	view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);
	if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

static void update_item(CmpackCurvePlot *view, CmpackCurvePlotItem *item, CmpackGraphData *model, gint row)
{
	item->visible = FALSE;
	if (model) {
		const CmpackGraphItem *data = cmpack_graph_data_get_item(model, row);
		if (data->y >= view->y.Min && data->y <= view->y.Max) {
			item->visible = !data->hidden;
			item->value = y_to_proj(view, data->y);
			item->error = data->error;
		}
	}
}

static void update_data(CmpackCurvePlot *view)
{
	gint i;

	if (view->model && view->items) {
		for (i=0; i<view->item_count; i++)
			update_item(view, view->items+i, view->model, i);
	}
}

static void rebuild_data(CmpackCurvePlot *view)
{
	gint nrows;

	clear_data(view);

	if (view->model) {
		nrows = cmpack_graph_data_nrows(view->model);
		if (view->model && nrows>0) {
			view->item_count = view->item_capacity = nrows;
			view->items = (CmpackCurvePlotItem*)g_malloc0(view->item_capacity*sizeof(CmpackCurvePlotItem));
			view->x.ProjWidth = view->item_count;
			update_data(view);
			view->x.ZoomMax = log(view->x.ProjWidth/view->x.ProjEps) / log(view->zoom_base);
			view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
			view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);
			if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
				update_rectangles(view);
				update_x_pxlsize(view);
				restrict_x_to_limits(view);
				update_y_pxlsize(view);
				restrict_y_to_limits(view);
				update_hsb(view);
				update_vsb(view);
				if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
					g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
				invalidate(view);
			}
		}
	}
}

static gboolean update_x_axis_name(CmpackPlotXAxis *axis, const gchar *name)
{
	if (name && name[0]!='\0') {
		if (axis->Name) {
			if (strcmp(name, axis->Name)!=0) {
				g_free(axis->Name);
				axis->Name = g_strdup(name);
				return TRUE;
			}
		} else {
			axis->Name = g_strdup(name);
			return TRUE;
		}
	} else {
		if (axis->Name) {
			g_free(axis->Name);
			axis->Name = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean update_y_axis_name(CmpackPlotYAxis *axis, const gchar *name)
{
	if (name && name[0]!='\0') {
		if (axis->Name) {
			if (strcmp(name, axis->Name)!=0) {
				g_free(axis->Name);
				axis->Name = g_strdup(name);
				return TRUE;
			}
		} else {
			axis->Name = g_strdup(name);
			return TRUE;
		}
	} else {
		if (axis->Name) {
			g_free(axis->Name);
			axis->Name = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

static void invalidate(CmpackCurvePlot *view)
{
	view->dirty = TRUE;
	gtk_widget_queue_draw(GTK_WIDGET(view));
}

static gboolean set_x_axis(CmpackCurvePlot *view, gdouble zoom, gdouble center)
{
	zoom = LimitDbl(zoom, 0.0, view->x.ZoomMax);
	center = LimitDbl(center, 0.0, view->x.ProjWidth);
	if (zoom!=view->x.ZoomPos || center!=view->y.ProjPos) {
		view->x.ZoomPos = zoom;
		view->x.ProjPos = center;
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_hsb(view);
		return TRUE;
	}
	return FALSE;
}

static gboolean set_auto_zoom_x(CmpackCurvePlot *view)
{
	gint i, first=1;
    gdouble xmin=0, xmax=0, zoom, a, b;

	for (i=0; i<view->item_count; i++) {
		CmpackCurvePlotItem *item = &view->items[i];
		if (item->visible) {
			if (first) {
				xmin = xmax = i;
				first = 0;
			} else {
				if (i < xmin)
					xmin = i;
				if (i > xmax)
					xmax = i;
			}
		}
	}
	if (first)
		return FALSE;

	xmin = LimitDbl(xmin, 0, view->x.ProjWidth);
	xmax = LimitDbl(xmax+1.0, 0, view->x.ProjWidth);
	if ((xmax-xmin)<view->x.ProjEps) {
		a = (xmax + xmin - view->x.ProjEps)/2.0;
		b = (xmax + xmin + view->x.ProjEps)/2.0;
	} else {
		a = xmin;
		b = xmax;
    }
	zoom = log(fabs(view->x.ProjWidth)/fabs(xmax-xmin))/log(view->zoom_base);
	return set_x_axis(view, zoom, 0.5*(a+b));
}

static gboolean set_y_axis(CmpackCurvePlot *view, gdouble zoom, gdouble center)
{
	zoom = LimitDbl(zoom, 0.0, view->y.ZoomMax);
	center = LimitDbl(center, view->y.ProjMin, view->y.ProjMax);
	if (zoom!=view->y.ZoomPos || center!=view->y.ProjPos) {
		view->y.ZoomPos = zoom;
		view->y.ProjPos = center;
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_vsb(view);
		return TRUE;
	}
	return FALSE;
}

static gboolean set_auto_zoom_y(CmpackCurvePlot *view)
{
	gint i, xmin, xmax, first;
	double f, a, b, zoom, ymin, ymax;

	ymin = ymax = 0.0;
	xmin = (int)view_to_xproj(view, view->canvas_rc.x);
	if (xmin < 0)
		xmin = 0;
	xmax = (int)ceil(view_to_xproj(view, view->canvas_rc.x + view->canvas_rc.width));
	if (xmax >= view->item_count)
		xmax = view->item_count-1;

	first = 1;
	for (i=xmin; i<=xmax; i++) {
		CmpackCurvePlotItem *item = &view->items[i];
		if (item->visible) {
			if (first) {
				ymin = ymax = item->value;
				first = 0;
			} else {
				if (item->value < ymin)
					ymin = item->value;
				if (item->value > ymax)
					ymax = item->value;
			}
		}
	}
	if (first)
		return FALSE;

	ymin = LimitDbl(ymin, view->y.ProjMin, view->y.ProjMax);
	ymax = LimitDbl(ymax, view->y.ProjMin, view->y.ProjMax);
	if ((ymax-ymin) < view->y.ProjEps) {
		f = ((ymin+ymax)/2.0 - view->y.ProjMin)/(view->y.ProjMax-view->y.ProjMin);
        a = ymin - f * (view->y.ProjEps - fabs(ymax - ymin));
		b = ymax + (1.0-f) * (view->y.ProjEps - fabs(ymax - ymin));
    } else {
        a = ymin;
        b = ymax;
    }
	zoom = log(fabs(view->y.ProjMax-view->y.ProjMin)/fabs(b-a))/log(view->zoom_base);
	return set_y_axis(view, zoom, 0.5*(a+b));
}

/* Public API */

/* Create a new graph view */
GtkWidget *cmpack_curve_plot_new(void)
{
	return cmpack_curve_plot_new_with_model(NULL);
}

/* Create a new graph with specified model */
GtkWidget *cmpack_curve_plot_new_with_model(CmpackGraphData *model)
{
  return (GtkWidget*)g_object_new(CMPACK_TYPE_CURVE_PLOT, "model", model, NULL);
}

/* Call custom function for all selected items */
void cmpack_curve_plot_selected_foreach (CmpackCurvePlot *view, CmpackCurvePlotForeachFunc func, gpointer data)
{
	gint i;

	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));
  
	if (view->select_first>=0) {
		for (i=view->select_first; i<=view->select_last && i<view->item_count; i++) 
			(*func)(view, i, data);
	}
}

/* Set selection mode */
void cmpack_curve_plot_set_selection_mode (CmpackCurvePlot *view, GtkSelectionMode mode)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	if (mode == view->selection_mode)
		return;
  
	if (mode == GTK_SELECTION_NONE || view->selection_mode == GTK_SELECTION_MULTIPLE)
		cmpack_curve_plot_unselect_all (view);

	view->selection_mode = mode;

	g_object_notify(G_OBJECT (view), "selection-mode");
}

/* Get selection mode */
GtkSelectionMode cmpack_curve_plot_get_selection_mode (CmpackCurvePlot *view)
{
	g_return_val_if_fail (CMPACK_IS_CURVE_PLOT (view), GTK_SELECTION_SINGLE);
	return view->selection_mode;
}

/* Set activation mode */
void cmpack_curve_plot_set_activation_mode(CmpackCurvePlot *view, CmpackActivationMode mode)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	if (mode == view->activation_mode)
		return;

	view->activation_mode = mode;
}

/* Enable/disable mouse control */
void cmpack_curve_plot_set_mouse_control(CmpackCurvePlot *view, gboolean enable)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	view->mouse_ctrl = enable;
}

/* Set data model */
void cmpack_curve_plot_set_model (CmpackCurvePlot *view, CmpackGraphData *model)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));
	g_return_if_fail (model == NULL || CMPACK_IS_GRAPH_DATA(model));
  
	if (view->model == model)
		return;

	if (view->model) {
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)row_inserted, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)row_updated, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)row_deleted, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)data_cleared, view);
		g_object_unref(view->model);
	}
	view->model = model;
	if (view->model) {
		g_object_ref(view->model);
		g_signal_connect(view->model, "row-inserted", G_CALLBACK(row_inserted), view);
		g_signal_connect(view->model, "row-updated", G_CALLBACK(row_updated), view);
		g_signal_connect(view->model, "row-deleted", G_CALLBACK(row_deleted), view);
		g_signal_connect(view->model, "data-cleared", G_CALLBACK(data_cleared), view);
	}
	rebuild_data(view);

	if (GTK_WIDGET_REALIZED (view))
		gtk_widget_queue_resize (GTK_WIDGET (view));

	g_object_notify(G_OBJECT (view), "model");  
}

/* Enable/disable mouse control */
void cmpack_curve_plot_set_style(CmpackCurvePlot *view, CmpackPlotStyle style)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	if (view->style!=style) {
		view->style = style;
		invalidate(view);
	}
}

/* Get data model */
CmpackGraphData *cmpack_curve_plot_get_model (CmpackCurvePlot *view)
{
	g_return_val_if_fail (CMPACK_IS_CURVE_PLOT (view), NULL);
	
	return view->model;
}

/* Add specified item to selection */
void cmpack_curve_plot_select(CmpackCurvePlot *view, gint row)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	if (view->select_first==row && view->select_last==row)
		return;
	if (view->selection_mode == GTK_SELECTION_NONE)
		return;
	
	if (view->selection_mode != GTK_SELECTION_MULTIPLE)
		cmpack_curve_plot_unselect_all_internal (view);

	view->select_first = view->select_last = row;
	g_signal_emit (view, curve_plot_signals[SELECTION_CHANGED], 0);
	cmpack_curve_plot_queue_draw_item (view, row, view->items + row);
}

/* Add specified item to selection */
void cmpack_curve_plot_select_range(CmpackCurvePlot *view, gint start, gint end)
{
	gint i;

	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));
	g_return_if_fail (start>=0 && start<=end && end<view->item_count);

	if (view->select_first==start && view->select_last==end)
		return;
	if (view->selection_mode == GTK_SELECTION_NONE)
		return;
	
	if (view->selection_mode != GTK_SELECTION_MULTIPLE)
		cmpack_curve_plot_unselect_all_internal (view);

	view->select_first = start;
	view->select_last = end;
	g_signal_emit (view, curve_plot_signals[SELECTION_CHANGED], 0);

	for (i=start; i<=end; i++) 
		cmpack_curve_plot_queue_draw_item (view, i, view->items + i);
}

gboolean cmpack_curve_plot_get_selected_range(CmpackCurvePlot *view, gint *first, gint *last)
{
	g_return_val_if_fail (CMPACK_IS_CURVE_PLOT (view), FALSE);

	if (first)
		*first = view->select_first;
	if (last)
		*last = view->select_last;
	return (view->select_first>=0);
}

gint cmpack_curve_plot_get_selected(CmpackCurvePlot *view)
{
	g_return_val_if_fail (CMPACK_IS_CURVE_PLOT (view), -1);
  
	return view->select_first;
}

void cmpack_curve_plot_select_all (CmpackCurvePlot *view)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	cmpack_curve_plot_select_range(view, 0, view->item_count-1);
}

void cmpack_curve_plot_unselect_all (CmpackCurvePlot *view)
{
	gboolean dirty = FALSE;
  
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	if (view->selection_mode == GTK_SELECTION_BROWSE)
		return;

	dirty = cmpack_curve_plot_unselect_all_internal (view);

	if (dirty)
		g_signal_emit (view, curve_plot_signals[SELECTION_CHANGED], 0);
}

gboolean cmpack_curve_plot_is_selected(CmpackCurvePlot *view, gint row)
{
	g_return_val_if_fail (CMPACK_IS_CURVE_PLOT (view), FALSE);
	g_return_val_if_fail (row>=0 && row<view->item_count, FALSE);

	return (row>=view->select_first && row<=view->select_last);
}

void cmpack_curve_plot_item_activate(CmpackCurvePlot *view, gint row)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	g_signal_emit (view, curve_plot_signals[ITEM_ACTIVATED], 0, row);
}

/* Set mapping parameters */
void cmpack_curve_plot_set_x_axis(CmpackCurvePlot *view, gdouble channel_width, gdouble zero_offset, 
	gdouble eps, CmpackPlotFormat format, gint minprec, gint maxprec, const gchar *caption)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	view->x.Format = format;
	view->x.MinPrec = LimitInt(minprec, 0, 16);
	view->x.MaxPrec = LimitInt(maxprec, minprec, 16);
	update_x_axis_name(&view->x, caption);
	view->x.ChannelWidth = channel_width;
	view->x.ZeroOffset = zero_offset; 
    view->x.ProjEps = eps;
    view->x.ZoomMax = log(view->x.ProjWidth/view->x.ProjEps) / log(view->zoom_base);
    view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
	view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);

	if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
}

/* Set mapping parameters */
void cmpack_curve_plot_set_y_axis(CmpackCurvePlot *view, gboolean log_scale, 
	gboolean reverse, gdouble min, gdouble max, gdouble eps, CmpackPlotFormat format, 
	gint minprec, gint maxprec, const gchar *caption)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	view->y.Log = log_scale;
	view->y.Reverse = reverse;
	view->y.Format = format;
	view->y.MinPrec = LimitInt(minprec, 0, 16);
	view->y.MaxPrec = LimitInt(maxprec, minprec, 16);
	update_y_axis_name(&view->y, caption);
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
    view->y.ZoomMax = log((view->y.ProjMax-view->y.ProjMin)/view->y.ProjEps) / log(view->zoom_base);
    view->y.ZoomMax = MAX(0.0, view->y.ZoomMax);
	view->y.ZoomPos = LimitDbl(view->y.ZoomPos, 0.0, view->y.ZoomMax);

	update_data(view);

	if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
}

/* Set labels */
void cmpack_curve_plot_set_scales(CmpackCurvePlot *view, gboolean x_axis, gboolean y_axis)
{
	if (view->x.ShowLabels!=x_axis || view->y.ShowLabels!=y_axis) {
		view->x.ShowLabels = x_axis;
		view->y.ShowLabels = y_axis;
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			update_rectangles(view);
			update_x_pxlsize(view);
			restrict_x_to_limits(view);
			update_y_pxlsize(view);
			restrict_y_to_limits(view);
			update_hsb(view);
			update_vsb(view);
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
	}
} 

void cmpack_curve_plot_set_grid(CmpackCurvePlot *view, gboolean x_axis, gboolean y_axis)
{
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	if (view->x.ShowGrid != x_axis || view->y.ShowGrid != y_axis) {
		view->x.ShowGrid = x_axis;
		view->y.ShowGrid = y_axis;
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) 
			invalidate(view);
	}
}

void cmpack_curve_plot_set_zoom(CmpackCurvePlot *view, CmpackGraphAxis axis, gdouble zoom)
{
	gboolean dirty;
	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	switch (axis) 
	{
	case CMPACK_AXIS_X:
		dirty = set_x_axis(view, zoom, view->x.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;
	case CMPACK_AXIS_Y:
		dirty = set_y_axis(view, zoom, view->y.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_curve_plot_get_zoom(CmpackCurvePlot *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_CURVE_PLOT (view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		return view->x.ZoomPos;
	case CMPACK_AXIS_Y:
		return view->y.ZoomPos;
	}
	return 0.0;
}

void cmpack_curve_plot_set_viewfield(CmpackCurvePlot *view, CmpackGraphAxis axis, gdouble viewfield)
{
	gboolean dirty;
	gdouble zoom;

	g_return_if_fail (CMPACK_IS_CURVE_PLOT (view));

	switch (axis)
	{
	case CMPACK_AXIS_X:
		if (viewfield>0.0) 
			zoom = log((view->x.ProjWidth)/viewfield) / log(view->zoom_base);
		else
			zoom = view->x.ZoomMax;
		dirty = set_x_axis(view, zoom, view->x.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;

	case CMPACK_AXIS_Y:
		if (viewfield>0.0) 
			zoom = log((view->y.ProjMax-view->y.ProjMin)/viewfield) / log(view->zoom_base);
		else
			zoom = view->y.ZoomMax;
		dirty = set_y_axis(view, zoom, view->y.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_curve_plot_get_viewfield(CmpackCurvePlot *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_CURVE_PLOT(view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		return view->x.PxlSize * view->graph_rc.width;
	case CMPACK_AXIS_Y:
		return view->y.PxlSize * view->graph_rc.height;
	}
	return 0.0;
}

void cmpack_curve_plot_set_center(CmpackCurvePlot *view, CmpackGraphAxis axis, gdouble center)
{
	gboolean dirty;

	g_return_if_fail(CMPACK_IS_CURVE_PLOT (view));

	switch (axis) 
	{
	case CMPACK_AXIS_X:
		dirty = set_x_axis(view, view->x.ZoomPos, center);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;
	case CMPACK_AXIS_Y:
		if (view->y.Log)
			center = log10(MAX(DBL_MIN, center));
		dirty = set_y_axis(view, view->y.ProjPos, center);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_curve_plot_get_center(CmpackCurvePlot *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_CURVE_PLOT(view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		return view->x.ProjPos;

	case CMPACK_AXIS_Y:
		if (!view->y.Log)
			return view->y.ProjPos;
		else
			return pow(10, view->y.ProjPos);
	}
	return 0.0;
}

void cmpack_curve_plot_auto_zoom(CmpackCurvePlot *view, gboolean x_axis, gboolean y_axis)
{
	gboolean dirty = FALSE;
	g_return_if_fail(CMPACK_IS_CURVE_PLOT(view));

	if (x_axis)
		dirty |= set_auto_zoom_x(view);
	if (y_axis)
		dirty |= set_auto_zoom_y(view);

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

void cmpack_curve_plot_reset_zoom(CmpackCurvePlot *view, gboolean x_axis, gboolean y_axis)
{
	gboolean dirty = FALSE;
	g_return_if_fail(CMPACK_IS_CURVE_PLOT(view));

	if (x_axis)
		dirty |= set_x_axis(view, 0, 0.5*view->x.ProjWidth);
	if (y_axis)
		dirty |= set_y_axis(view, 0, 0.5*(view->y.ProjMin+view->y.ProjMax));

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, curve_plot_signals[MOUSE_MOVED], 0);
	}
}

gint cmpack_curve_plot_mouse_pos(CmpackCurvePlot *view)
{
	g_return_val_if_fail(CMPACK_IS_CURVE_PLOT(view), FALSE);

	if (view->last_mouse_x>=0) {
		gdouble pos = view_to_xproj(view, view->last_mouse_x);
		if (pos>=0 && pos<view->item_count) 
			return (int)pos;
	}
	return -1;
}
