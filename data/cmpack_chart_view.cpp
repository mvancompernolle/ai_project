/**************************************************************

cmpack_chart_view.cpp (C-Munipack project)
Widget which can draw a chart
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

$Id: cmpack_chart_view.cpp,v 1.7 2014/08/31 12:39:43 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gdk/gdkkeysyms.h>

#include "cmpack_chart_view.h"

#define CLICK_TOLERANCE 7

#define CURSOR_SIZE		12

#define FONT_SCALE		0.75

/* Signals */
enum {
	ITEM_ACTIVATED,
	SELECTION_CHANGED,
	PROFILE_CHANGED,
	MOUSE_MOVED,
	ZOOM_CHANGED,
	LAST_SIGNAL
};

/* Properties */
enum {
	PROP_0,
	PROP_SELECTION_MODE,
	PROP_MODEL,
	PROP_IMAGE,
	PROP_NEGATIVE
};

/* Internal colors */
enum {
	CHART_COLOR_SELECTED,
	CHART_COLOR_BACKGROUND,
	CHART_COLOR_ZOOMRECT,
	CHART_COLOR_CURSOR_LO,
	CHART_COLOR_CURSOR_HI,
	CHART_COLOR_RED,
	CHART_COLOR_GREEN,
	CHART_COLOR_BLUE,
	CHART_COLOR_YELLOW,
	CHART_N_COLORS
};

/* GObject vfuncs */
static void             cmpack_chart_view_set_property              (GObject            *object,
								 guint               prop_id,
								 const GValue       *value,
								 GParamSpec         *pspec);
static void             cmpack_chart_view_get_property              (GObject            *object,
								 guint               prop_id,
								 GValue             *value,
								 GParamSpec         *pspec);
static void             cmpack_chart_view_finalize                  (GObject          *object); 

/* GtkObject vfuncs */
static void             cmpack_chart_view_destroy                   (GtkObject          *object);
/* GtkWidget vfuncs */
static void             cmpack_chart_view_realize                   (GtkWidget          *widget);
static void             cmpack_chart_view_unrealize                 (GtkWidget          *widget);
static void             cmpack_chart_view_style_set                 (GtkWidget        *widget,
						                 GtkStyle         *previous_style);
static void             cmpack_chart_view_state_changed             (GtkWidget        *widget,
			                                         GtkStateType      previous_state);
static void             cmpack_chart_view_size_allocate             (GtkWidget          *widget,
								 GtkAllocation      *allocation);
static gboolean         cmpack_chart_view_expose                    (GtkWidget          *widget,
								 GdkEventExpose     *expose);
static gboolean         cmpack_chart_view_motion                    (GtkWidget          *widget,
								 GdkEventMotion     *event);
static gboolean         cmpack_chart_view_leave                  (GtkWidget          *widget,
								 GdkEventCrossing   *event);
static gboolean         cmpack_chart_view_button_press              (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_chart_view_button_release            (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_chart_view_key_press                 (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_chart_view_key_release               (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_chart_view_scroll		     		(GtkWidget      *widget,
								 GdkEventScroll     *event);

/* CmpackChartView vfuncs */
static void             cmpack_chart_view_set_adjustments           (CmpackChartView        *view,
								 GtkAdjustment      *hadj,
								 GtkAdjustment      *vadj);

 /* Internal functions */
static void             cmpack_chart_view_create_buffer		(CmpackChartView            *view); 
static void				cmpack_chart_view_paint_buffer				(CmpackChartView            *view); 

static void                 cmpack_chart_view_adjustment_changed             (GtkAdjustment          *adjustment,
									  CmpackChartView            *view);
static void                 cmpack_chart_view_enter_mouse_mode            (CmpackChartView            *view,
									  gint                    x,
									  gint                    y,
									  CmpackChartMouseMode    mode);
static void                 cmpack_chart_view_leave_mouse_mode             (CmpackChartView            *view);
static gboolean             cmpack_chart_view_unselect_all_internal          (CmpackChartView            *view);
static void                 cmpack_chart_view_select_item                    (CmpackChartView            *view,
									  CmpackChartViewItem        *item);
static void                 cmpack_chart_view_unselect_item                  (CmpackChartView            *view,
									  CmpackChartViewItem        *item);
static void cmpack_chart_view_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data);

static void clear_data(CmpackChartView *view);
static void rebuild_data(CmpackChartView *view);
static void update_data(CmpackChartView *view);
static void update_limits(CmpackChartView *view);
static void update_item(CmpackChartView *view, CmpackChartViewItem *item, CmpackChartData *model, gint row);
static gint find_item(CmpackChartView *view, gint view_x, gint view_y);
static void invalidate(CmpackChartView *view);
static void invalidate_cursor(CmpackChartView *view);
static void invalidate_profile(CmpackChartView *view);
static void update_cursor(CmpackChartView *view);

static gint RoundToInt(gdouble x);
static gdouble LimitDbl(gdouble val, gdouble min, gdouble max);
static gint LimitInt(gint val, gint min, gint max);

static void set_rgb(GdkColor *color, gdouble red, gdouble green, gdouble blue);
static const GdkColor *item_fg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartViewItem *item);
static const GdkColor *item_bg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartViewItem *item);
static const GdkColor *entity_fg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartEntity *entity);
static const GdkColor *entity_bg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartEntity *entity);

static gdouble xproj_to_view(CmpackChartView *view, gdouble x);
static gdouble view_to_xproj(CmpackChartView *view, gdouble u);
static gdouble x_to_view(CmpackChartView *view, gdouble x);
static gdouble view_to_x(CmpackChartView *view, gdouble u);
static gdouble yproj_to_view(CmpackChartView *view, gdouble y);
static gdouble view_to_yproj(CmpackChartView *view, gdouble v);
static gdouble y_to_view(CmpackChartView *view, gdouble y);
static gdouble view_to_y(CmpackChartView *view, gdouble v);

static gboolean fit_to_window(CmpackChartView *view);
static gboolean set_zoom_offset(CmpackChartView *view, gdouble zoom, gdouble xoffset, gdouble yoffset);
static gboolean set_zoom(CmpackChartView *view, gdouble zoom);
static gboolean set_offset(CmpackChartView *view, gdouble xoffset, gdouble yoffset);
static void update_rectangles(CmpackChartView *view);
static void update_pxlsize(CmpackChartView *view);
static void restrict_x_to_limits(CmpackChartView *view);
static void restrict_y_to_limits(CmpackChartView *view);

static void compute_grid_step(CmpackChartView *view, PangoContext *pg, gdouble *bigstep, gdouble *smallstep);
static void format_label(gchar *buf, gdouble value);
static void paint_data(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_scales(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_grid(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_layers(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable);

static void update_hsb(CmpackChartView *view);
static void update_vsb(CmpackChartView *view);

static CmpackChartLayer *create_layer(gint id);
static void add_layer(CmpackChartView *view, CmpackChartLayer *layer);
static CmpackChartLayer *find_layer(CmpackChartView *view, gint id);
static void destroy_layer(CmpackChartView *view, CmpackChartLayer *layer);
static CmpackChartEntity *create_entity(gint id, CmpackChartShape shape);
static void add_entity(CmpackChartView *view, CmpackChartLayer *layer, CmpackChartEntity *entity);
static CmpackChartEntity *find_entity(CmpackChartView *view, gint id);
static void destroy_entity(CmpackChartEntity *entity);
static void clear_layers(CmpackChartView *view);

static guint chart_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(CmpackChartView, cmpack_chart_view, GTK_TYPE_WIDGET)

/* Class initialization */
static void cmpack_chart_view_class_init(CmpackChartViewClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	gobject_class->set_property = cmpack_chart_view_set_property;
	gobject_class->get_property = cmpack_chart_view_get_property;
	gobject_class->finalize = cmpack_chart_view_finalize;

	object_class->destroy = cmpack_chart_view_destroy;

	widget_class->realize = cmpack_chart_view_realize;
	widget_class->unrealize = cmpack_chart_view_unrealize;
	widget_class->style_set = cmpack_chart_view_style_set;
	widget_class->size_allocate = cmpack_chart_view_size_allocate;
	widget_class->expose_event = cmpack_chart_view_expose;
	widget_class->motion_notify_event = cmpack_chart_view_motion;
	widget_class->leave_notify_event = cmpack_chart_view_leave;
	widget_class->button_press_event = cmpack_chart_view_button_press;
	widget_class->button_release_event = cmpack_chart_view_button_release;
	widget_class->key_press_event = cmpack_chart_view_key_press;
	widget_class->key_release_event = cmpack_chart_view_key_release;
	widget_class->state_changed = cmpack_chart_view_state_changed;
	widget_class->scroll_event = cmpack_chart_view_scroll;

	klass->set_scroll_adjustments = cmpack_chart_view_set_adjustments;
  
	/* Properties */
	g_object_class_install_property (gobject_class, PROP_SELECTION_MODE, 
		g_param_spec_enum("selection-mode", "Selection mode", "The selection mode",
			GTK_TYPE_SELECTION_MODE, GTK_SELECTION_SINGLE, 
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_MODEL,
		g_param_spec_object("model", "Chart Data Model", "The data model for the chart view",
			CMPACK_TYPE_CHART_DATA, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	g_object_class_install_property (gobject_class, PROP_IMAGE,
		g_param_spec_object("image", "Image Data Model", "The image data for the chart view",
			CMPACK_TYPE_IMAGE_DATA, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	g_object_class_install_property (gobject_class, PROP_NEGATIVE,
		g_param_spec_boolean("negative", "Drawing Style", "FALSE = white stars on dark background, TRUE = black stars on white background",
			FALSE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	/* Signals */
	widget_class->set_scroll_adjustments_signal = 
		g_signal_new ("set-scroll-adjustments", G_OBJECT_CLASS_TYPE (gobject_class),
			GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
			G_STRUCT_OFFSET (CmpackChartViewClass, set_scroll_adjustments),
			NULL, NULL, cmpack_chart_view_marshal_VOID__OBJECT_OBJECT,
			G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT); 

	chart_view_signals[ITEM_ACTIVATED] =
		g_signal_new ("item-activated", G_TYPE_FROM_CLASS (gobject_class),
			G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CmpackChartViewClass, item_activated),
			NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, GTK_TYPE_INT);

	chart_view_signals[SELECTION_CHANGED] =
		g_signal_new ("selection-changed", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackChartViewClass, selection_changed),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	chart_view_signals[PROFILE_CHANGED] =
		g_signal_new ("profile-changed", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackChartViewClass, profile_changed),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	chart_view_signals[MOUSE_MOVED] =
		g_signal_new ("mouse-moved", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackChartViewClass, mouse_moved),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	chart_view_signals[ZOOM_CHANGED] =
		g_signal_new ("zoom-changed", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackChartViewClass, zoom_changed),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Chart view initialization */
static void cmpack_chart_view_init(CmpackChartView *view)
{
	view->zoom_base = pow(100.0, 1.0/100.0);
	view->last_mouse_x = view->last_mouse_y = -1;
	view->auto_zoom = TRUE;
	view->zoom_min = -100.0;
	view->zoom_max = 60.0;
	view->focused_item = -1;
	view->last_single_clicked = -1;
	view->int_colors = (GdkColor*)g_malloc(CHART_N_COLORS*sizeof(GdkColor));
	view->ht_layers = g_hash_table_new(&g_int_hash, &g_int_equal);
	view->ht_entities = g_hash_table_new(&g_int_hash, &g_int_equal);
	cmpack_chart_view_set_adjustments(view, NULL, NULL);
}

static void cmpack_chart_view_destroy(GtkObject *object)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (object);

	/* Disconnect the models */
	cmpack_chart_view_set_model(view, NULL);
	cmpack_chart_view_set_image(view, NULL);
  
	/* Disconnect the adjustments */
	if (view->hadjustment != NULL) {
		g_object_unref (view->hadjustment);
		view->hadjustment = NULL;
	}
	if (view->vadjustment != NULL) {
		g_object_unref (view->vadjustment);
		view->vadjustment = NULL;
	}
  
	GTK_OBJECT_CLASS(cmpack_chart_view_parent_class)->destroy(object);
}

static void cmpack_chart_view_finalize(GObject *object)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(object);

	/* Free buffered data */
	clear_data(view);

	/* Delete overlays */
	clear_layers(view);
	g_hash_table_destroy(view->ht_layers);
	g_hash_table_destroy(view->ht_entities);

	/* Free pixmaps */
	if (view->offscreen_pixmap) 
		g_object_unref(view->offscreen_pixmap);

	/* Close timer */
	if (view->timer_id) 
		g_source_remove(view->timer_id);
	
	g_free(view->int_colors);

	G_OBJECT_CLASS(cmpack_chart_view_parent_class)->finalize(object);
} 

/* Set property */
static void cmpack_chart_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (object);

	switch (prop_id)
	{
    case PROP_SELECTION_MODE:
		cmpack_chart_view_set_selection_mode (view, (GtkSelectionMode)g_value_get_enum(value));
		break;
    case PROP_MODEL:
		cmpack_chart_view_set_model(view, (CmpackChartData*)g_value_get_object(value));
		break;
	case PROP_IMAGE:
		cmpack_chart_view_set_image(view, (CmpackImageData*)g_value_get_object(value));
		break;
	case PROP_NEGATIVE:
		cmpack_chart_view_set_negative(view, g_value_get_boolean(value));
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

/* Get property */
static void cmpack_chart_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (object);

	switch (prop_id)
    {
    case PROP_SELECTION_MODE:
		g_value_set_enum(value, view->selection_mode);
		break;
    case PROP_MODEL:
		g_value_set_object(value, view->model);
		break;
    case PROP_IMAGE:
		g_value_set_object(value, view->image);
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
static void cmpack_chart_view_realize (GtkWidget *widget)
{
	gboolean int_ok[CHART_N_COLORS];
	CmpackChartView *view = CMPACK_CHART_VIEW(widget);
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

	set_rgb(&view->int_colors[CHART_COLOR_BACKGROUND], 0.1, 0.0, 0.2);
	set_rgb(&view->int_colors[CHART_COLOR_SELECTED], 0.7, 0.7, 0);
	set_rgb(&view->int_colors[CHART_COLOR_ZOOMRECT], 1.0, 0.0, 0.0);
	set_rgb(&view->int_colors[CHART_COLOR_CURSOR_LO], 0.0, 0.7, 0.8);
	set_rgb(&view->int_colors[CHART_COLOR_CURSOR_HI], 0.0, 0.9, 1.0);
	set_rgb(&view->int_colors[CHART_COLOR_RED], 0.7, 0.0, 0.0);
	set_rgb(&view->int_colors[CHART_COLOR_GREEN], 0, 0.6, 0);
	set_rgb(&view->int_colors[CHART_COLOR_BLUE], 0.0, 0.4, 1.0);
	set_rgb(&view->int_colors[CHART_COLOR_YELLOW], 0.7, 0.7, 0.0);
	gdk_colormap_alloc_colors(gtk_widget_get_colormap(widget), view->int_colors, 
		CHART_N_COLORS, FALSE, TRUE, int_ok);

	cmpack_chart_view_create_buffer(view); 

	update_rectangles(view);
	if (view->auto_zoom) 
		fit_to_window(view);
	update_pxlsize(view);
	restrict_x_to_limits(view);
	restrict_y_to_limits(view);
	update_hsb(view);
	update_vsb(view);
	view->dirty = TRUE;

	if (view->auto_zoom)
		g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
}

/* Create GDK window and auxiliary pixmaps */
static void cmpack_chart_view_unrealize(GtkWidget *widget)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(widget);

	gdk_colormap_free_colors(gtk_widget_get_colormap(widget), view->int_colors, CHART_N_COLORS);

	GTK_WIDGET_CLASS(cmpack_chart_view_parent_class)->unrealize (widget); 
}

/* Widget state changed */
static void cmpack_chart_view_state_changed(GtkWidget *widget, GtkStateType previous_state)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);

	view->dirty = TRUE;
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS (cmpack_chart_view_parent_class)->state_changed(widget, previous_state);
}

/* Widget style changed */
static void cmpack_chart_view_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);

	view->dirty = TRUE; 
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS(cmpack_chart_view_parent_class)->style_set(widget, previous_style);
}

/* Widget size changed */
static void cmpack_chart_view_size_allocate(GtkWidget *widget, GtkAllocation  *allocation)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);
	gboolean zoom_changed = false;

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		cmpack_chart_view_create_buffer(view); 
		update_rectangles(view);
		if (view->auto_zoom) 
			zoom_changed = fit_to_window(view);
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		view->dirty = TRUE;
		if (zoom_changed)
			g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
	}
}

static void paint_rectangle(CmpackChartView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, GdkColor *color)
{
	gint i;
	gboolean ok;
	GdkRectangle rect;

	rect.x = MIN (view->mouse_start_x, view->mouse_pos_x);
	rect.y = MIN (view->mouse_start_y, view->mouse_pos_y);
	rect.width = ABS (view->mouse_start_x - view->mouse_pos_x);
	rect.height = ABS (view->mouse_start_y - view->mouse_pos_y);

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

static void paint_cursor(CmpackChartView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, int x, int y, int size)
{
	gint i, inner, outer, left, top, right, bottom;
	gboolean ok;

	inner = RoundToInt(size + 3.0 * pow(sin(view->cursor_phase/45.0*M_PI), 2));
	outer = RoundToInt(size + CURSOR_SIZE + 3.0 * pow(sin(view->cursor_phase/45.0*M_PI), 2));

	left = x - outer;
	top = y - outer;
	right = x + outer + 1;
	bottom = y + outer + 1;
	
	ok = FALSE;
	for (i=0; i<nrectangles; i++) {
		const GdkRectangle *area = &rectangles[i];
		if (left <= area->x + area->width && top <= area->y + area->height && right >= area->x && bottom >= area->y) {
			ok = TRUE;
			break;
		}
	}

	if (ok) {
		gdouble sina = sin(view->cursor_phase/180.0*M_PI), cosa = cos(view->cursor_phase/180.0*M_PI);
		GdkGC *gc = gdk_gc_new(drawable);	
		gdk_gc_set_foreground(gc, &view->int_colors[view->negative ? CHART_COLOR_CURSOR_LO : CHART_COLOR_CURSOR_HI]);
		gdk_gc_set_line_attributes(gc, 3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
		gdk_draw_line(GDK_DRAWABLE(widget->window), gc,
			RoundToInt(x + outer*cosa), RoundToInt(y - outer*sina), 
			RoundToInt(x + inner*cosa), RoundToInt(y - inner*sina));
		gdk_draw_line(GDK_DRAWABLE(widget->window), gc,
			RoundToInt(x - outer*cosa), RoundToInt(y + outer*sina), 
			RoundToInt(x - inner*cosa), RoundToInt(y + inner*sina));
		gdk_draw_line(GDK_DRAWABLE(widget->window), gc,
			RoundToInt(x + outer*sina), RoundToInt(y + outer*cosa), 
			RoundToInt(x + inner*sina), RoundToInt(y + inner*cosa));
		gdk_draw_line(GDK_DRAWABLE(widget->window), gc,
			RoundToInt(x - outer*sina), RoundToInt(y - outer*cosa), 
			RoundToInt(x - inner*sina), RoundToInt(y - inner*cosa));
		gdk_gc_destroy(gc);
	}
}

static void paint_profile(CmpackChartView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles)
{
	gint x3, y3, x1, y1, x2, y2, left, top, right, bottom;
	gdouble d, vx, vy;
	GdkGC *gc;

	x1 = RoundToInt(xproj_to_view(view, view->profile_x[0]));
	y1 = RoundToInt(yproj_to_view(view, view->profile_y[0]));
	x2 = RoundToInt(xproj_to_view(view, view->profile_x[1]));
	y2 = RoundToInt(yproj_to_view(view, view->profile_y[1]));

	left	= MIN(x1, x2) - 4;
	top		= MIN(y1, y2) - 4;
	right	= MAX(x1, x2) - 4;
	bottom	= MAX(y1, y2) - 4;
	
	gboolean ok = FALSE;
	for (int i=0; i<nrectangles; i++) {
		const GdkRectangle *area = &rectangles[i];
		if (left <= area->x + area->width && top <= area->y + area->height && right >= area->x && bottom >= area->y) {
			ok = TRUE;
			break;
		}
	}

	if (ok) {
		gc = gdk_gc_new(drawable);
		gdk_gc_set_clip_rectangle(gc, &view->canvas_rc);
		gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
		gdk_gc_set_foreground(gc, &view->int_colors[view->negative ? CHART_COLOR_CURSOR_LO : CHART_COLOR_CURSOR_HI]);
		gdk_draw_line(drawable, gc, x1, y1, x2, y2);
		d = sqrt((double)(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
		if (d>10) {
			vx = (x2-x1)/d;
			vy = (y2-y1)/d;
			x3 = RoundToInt(x1 + 4*vy);
			y3 = RoundToInt(y1 - 4*vx);
			gdk_draw_line(drawable, gc, x3, y3, x1, y1);
			x3 = RoundToInt(x1 - 4*vy);
			y3 = RoundToInt(y1 + 4*vx);
			gdk_draw_line(drawable, gc, x3, y3, x1, y1);
			x3 = RoundToInt(x2 - 14.0*vx + 4*vy);
			y3 = RoundToInt(y2 - 14.0*vy - 4*vx);
			gdk_draw_line(drawable, gc, x3, y3, x2, y2);
			x3 = RoundToInt(x2 - 14.0*vx - 4*vy);
			y3 = RoundToInt(y2 - 14.0*vy + 4*vx);
			gdk_draw_line(drawable, gc, x3, y3, x2, y2);
		}
		gdk_gc_unref(gc);
	}
}

/* Refresh the offscreen buffer (if needed) and paint it to the window */
static gboolean cmpack_chart_view_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	CmpackChartView *view;
	GdkRectangle *rectangles;
	gint n_rectangles, x, y, r;     

	if (GTK_WIDGET_DRAWABLE(widget)) {
		view = CMPACK_CHART_VIEW(widget);
		if (view->dirty) 
			cmpack_chart_view_paint_buffer(view);
		gdk_draw_drawable (widget->window, widget->style->fg_gc[widget->state],
				   CMPACK_CHART_VIEW(widget)->offscreen_pixmap,
				   expose->area.x, expose->area.y, expose->area.x, expose->area.y,
				   expose->area.width, expose->area.height);

		gdk_region_get_rectangles(expose->region, &rectangles, &n_rectangles);
		if (view->mouse_mode) {
			if (view->mouse_mode == CHART_MOUSE_SELECT) {
				paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
					&view->int_colors[view->negative ? CHART_COLOR_CURSOR_LO : CHART_COLOR_CURSOR_HI]);
			} else 
			if (view->mouse_mode == CHART_MOUSE_ZOOM) {
				paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
					&view->int_colors[CHART_COLOR_ZOOMRECT]);
			}
		} else {
			if (view->focused_item>=0 && view->focused_item<view->item_count) {
				CmpackChartViewItem *item = &view->items[view->focused_item];
				r = RoundToInt(3.0 + 1.5 * item->size / sqrt(view->pxl_size));
				if (r < 5)
					r = 5;
				x = RoundToInt(xproj_to_view(view, item->xproj));
				y = RoundToInt(yproj_to_view(view, item->yproj));
				paint_cursor(view, widget, widget->window, rectangles, n_rectangles, x, y, r);
			}
		}
		if (view->profile_valid) {
			paint_profile(view, widget, widget->window, rectangles, n_rectangles);
		}
		g_free(rectangles);
	}

	return FALSE; 
}

/* Rebuild the offscreen buffer */
static void cmpack_chart_view_create_buffer(CmpackChartView *view)
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
static void cmpack_chart_view_create_marker(CmpackChartView *view)
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
		cmpack_chart_view_paint_marker(view);
	}
}*/

static void cmpack_chart_view_paint_buffer(CmpackChartView *view)
{
	gint x, y, left, top, right, bottom;
	gint img_width, img_height, img_right, img_bottom;
	GdkGC *gc;
	cairo_t *cr;
	cairo_surface_t *cs;
	cairo_pattern_t *pat;
	GtkWidget *widget;
	cairo_matrix_t mx;

	if (view->offscreen_pixmap) {
		widget = GTK_WIDGET(view);
		gc = gdk_gc_new(view->offscreen_pixmap);
		// Light margins for scales
		if (view->canvas_rc.x > 0) {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->bg_gc[widget->state], 
				TRUE, 0, 0, view->canvas_rc.x, widget->allocation.height);
		}
		if (view->canvas_rc.y > 0) {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->bg_gc[widget->state], 
				TRUE, 0, 0, widget->allocation.width, view->canvas_rc.y);
		}
		left = view->canvas_rc.x + view->canvas_rc.width;
		if (left < widget->allocation.width) {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->bg_gc[widget->state], 
				TRUE, left, 0, widget->allocation.width - left, widget->allocation.height);
		}
		top = view->canvas_rc.y + view->canvas_rc.height;
		if (top < widget->allocation.height) {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->bg_gc[widget->state], 
				TRUE, 0, top, widget->allocation.width, widget->allocation.height - top);
		}
		
		left = view->canvas_rc.x;
		top = view->canvas_rc.y;
		right = (int)xproj_to_view(view, view->x.ProjMax);
		bottom = (int)yproj_to_view(view, view->y.ProjMin);
		if (right > left && bottom > top) {
			// Widget background (not occupied by a chart)
			if (right < view->canvas_rc.x + view->canvas_rc.width) {
				x = MAX(right, view->canvas_rc.x);
				gdk_draw_rectangle(view->offscreen_pixmap, widget->style->dark_gc[widget->state],
					TRUE, x, view->canvas_rc.y, view->canvas_rc.x + view->canvas_rc.width - x, 
					view->canvas_rc.height);
			}
			if (bottom < view->canvas_rc.y + view->canvas_rc.height) {
				y = MAX(bottom, view->canvas_rc.y);
				gdk_draw_rectangle(view->offscreen_pixmap, widget->style->dark_gc[widget->state],
					TRUE, view->canvas_rc.x, y, view->canvas_rc.width, 
					view->canvas_rc.y + view->canvas_rc.height - y);
			}
			if (view->image) {
				img_width = cmpack_image_data_width(view->image);
				img_height = cmpack_image_data_height(view->image);
			} else {
				img_width = img_height = 0;
			}
			if (img_width>0 && img_height>0) {
				img_right = (int)x_to_view(view, (!view->x.Reverse ? img_width : 0));
				if (img_right < right) {
					if (!view->negative)
						gdk_gc_set_foreground(gc, &view->int_colors[CHART_COLOR_BACKGROUND]);
					else
						gdk_gc_set_foreground(gc, &widget->style->base[widget->state]);
					gdk_draw_rectangle(view->offscreen_pixmap, gc, TRUE, 
						img_right, top, right - img_right, bottom - top);
				}
				img_bottom = (int)y_to_view(view, (!view->y.Reverse ? img_height : 0));
				if (img_bottom < bottom) {
					if (!view->negative)
						gdk_gc_set_foreground(gc, &view->int_colors[CHART_COLOR_BACKGROUND]);
					else
						gdk_gc_set_foreground(gc, &widget->style->base[widget->state]);
					gdk_draw_rectangle(view->offscreen_pixmap, gc, TRUE, 
						left, bottom, right - left, bottom - img_bottom);
				}
				// Draw image
				cr = gdk_cairo_create(view->offscreen_pixmap);
				cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
				cairo_rectangle(cr, view->canvas_rc.x, view->canvas_rc.y, 
					view->canvas_rc.width, view->canvas_rc.height);			
				cairo_clip(cr);
				cs = cmpack_image_data_get_surface(view->image);
				pat = cairo_pattern_create_for_surface(cs);
				cairo_pattern_set_filter(pat, (view->pxl_size>0.125 ? CAIRO_FILTER_GOOD : CAIRO_FILTER_NEAREST));
				cairo_matrix_init_identity(&mx);
				if (!view->y.Reverse)
					cairo_matrix_translate(&mx, view->x.ProjPos, -view->y.ProjPos);
				else
					cairo_matrix_translate(&mx, view->x.ProjPos, -(view->y.ProjPos-img_height));
				cairo_matrix_scale(&mx, view->pxl_size, 	view->pxl_size);
				cairo_pattern_set_matrix(pat, &mx);
				cairo_set_source(cr, pat);
				cairo_paint(cr);
				cairo_pattern_destroy(pat);
				cairo_destroy(cr);
			} else {
				// Draw clear frame 
				if (!view->negative)
					gdk_gc_set_foreground(gc, &view->int_colors[CHART_COLOR_BACKGROUND]);
				else
					gdk_gc_set_foreground(gc, &widget->style->base[widget->state]);
				gdk_draw_rectangle(view->offscreen_pixmap, gc, TRUE, 
					left, top, right - left, bottom - top);
			}
			// Grid
			if (view->show_grid) 
				paint_grid(view, widget, view->offscreen_pixmap);
			// Data
			if (view->item_count>0)
				paint_data(view, widget, view->offscreen_pixmap);
			// Scales
			if (view->show_labels)
				paint_scales(view, widget, view->offscreen_pixmap);
			// Overlays
			if (view->layers)
				paint_layers(view, widget, view->offscreen_pixmap);
		} else {
			gdk_draw_rectangle(view->offscreen_pixmap, widget->style->dark_gc[widget->state],
				TRUE, view->canvas_rc.x, view->canvas_rc.y, view->canvas_rc.width, view->canvas_rc.height);
		}

		gdk_gc_destroy(gc);
		view->dirty = FALSE;
	}
} 

/*static void cmpack_chart_view_paint_marker(CmpackChartView *view)
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

static const GdkColor *item_fg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartViewItem *item)
{
	if (item->selected) 
		return &view->int_colors[CHART_COLOR_SELECTED];

	switch (item->color)
	{
	case CMPACK_COLOR_RED:
		return &view->int_colors[CHART_COLOR_RED];
	case CMPACK_COLOR_GREEN:
		return &view->int_colors[CHART_COLOR_GREEN];
	case CMPACK_COLOR_BLUE:
		return &view->int_colors[CHART_COLOR_BLUE];
	case CMPACK_COLOR_YELLOW:
		return &view->int_colors[CHART_COLOR_YELLOW];
	case CMPACK_COLOR_GRAY:
		return &widget->style->dark[widget->state];
	default:
		if (view->negative)
			return &widget->style->fg[widget->state];
		else
			return &widget->style->white;
	}
}

static const GdkColor *item_bg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartViewItem *item)
{
	if (view->negative)
		return &widget->style->base[widget->state];
	else
		return &widget->style->black;
}

static void paint_item(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, const CmpackChartViewItem *item)
{
	gint x, y, d;

	d = RoundToInt(2.0 * item->size / sqrt(view->pxl_size));
	if (d>1) {
		x = RoundToInt(xproj_to_view(view, item->xproj)) - (d+1)/2;
		y = RoundToInt(yproj_to_view(view, item->yproj)) - (d+1)/2;
		if (x + d >= view->canvas_rc.x && x <= view->canvas_rc.x + view->canvas_rc.width &&
			y + d >= view->canvas_rc.y && y <= view->canvas_rc.y + view->canvas_rc.height) {
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

static void paint_label(CmpackChartView *view, GtkWidget *widget, PangoLayout *layout, 
	GdkDrawable *drawable, GdkGC *gc, const CmpackChartViewItem *item)
{
	gint x, y, d, w, h;

	pango_layout_set_text(layout, item->tag_text, -1);
	d = RoundToInt(item->size / sqrt(view->pxl_size));
	x = RoundToInt(xproj_to_view(view, item->xproj) + 0.71*d);
	y = RoundToInt(yproj_to_view(view, item->yproj) + 0.71*d);
	pango_layout_get_pixel_size(layout, &w, &h);
	w += 4;
	if (x + w >= view->canvas_rc.x && x <= view->canvas_rc.x + view->canvas_rc.width &&
		y + h >= view->canvas_rc.y && y <= view->canvas_rc.y + view->canvas_rc.height) {
			gdk_gc_set_foreground(gc, item_fg_color(view, widget, item));
			gdk_draw_layout(drawable, gc, x+2, y, layout);
	}
}

static void paint_data(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint i, x0, x1, y0, y1, size;
	GdkRectangle clip_rc;
	PangoFontDescription *desc, *desc2;
	PangoLayout *layout;

	GdkGC *gc = gdk_gc_new(drawable);

	// Bold font
	PangoContext *pg = gtk_widget_create_pango_context(widget);
	desc = pango_context_get_font_description(pg);
	desc2 = pango_font_description_copy(desc);
	size = pango_font_description_get_size(desc2);
	if (pango_font_description_get_size_is_absolute(desc2))
		pango_font_description_set_absolute_size(desc2, size*1.2);
	else
		pango_font_description_set_size(desc2, (int)(size*1.2));
	pango_font_description_set_weight(desc2, PANGO_WEIGHT_BOLD);
	pango_context_set_font_description(pg, desc2);
	pango_font_description_free(desc2);
	layout = pango_layout_new(pg);

	// Set clipping rectangle
	x0 = (int)floor(xproj_to_view(view, view->x.ProjMin));
	clip_rc.x = MAX(x0, view->canvas_rc.x);
	x1 = (int)ceil(xproj_to_view(view, view->x.ProjMax));
	clip_rc.width = MIN(x1, view->canvas_rc.x + view->canvas_rc.width) - clip_rc.x;
	y0 = (int)floor(yproj_to_view(view, view->y.ProjMax));
	clip_rc.y = MAX(y0, view->canvas_rc.y);
	y1 = (int)ceil(yproj_to_view(view, view->y.ProjMin));
	clip_rc.height = MIN(y1, view->canvas_rc.y + view->canvas_rc.height) - clip_rc.y;
	gdk_gc_set_clip_rectangle(gc, &clip_rc);

	// First run: Draw non-topmost items
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && !item->topmost)
			paint_item(view, widget, drawable, gc, item);
	}

	// Second run: Draw topmost items
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && item->topmost)
			paint_item(view, widget, drawable, gc, item);
	}

	// Third run: Draw labels
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && item->tag_text) 
			paint_label(view, widget, layout, drawable, gc, item);
	}

	g_object_unref(layout);
	g_object_unref(pg);
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

static gint text_width(GtkWidget *widget, PangoContext *pg, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = pango_layout_new(pg);
		pango_layout_set_text(layout, buf, -1);
		pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
		g_object_unref(layout);
		return logical_rect.width;
	}
	return 0;
}

static gint text_height(GtkWidget *widget, PangoContext *pg, const gchar *buf)
{
	PangoRectangle logical_rect;

	if (buf) {
		PangoLayout *layout = pango_layout_new(pg);
		pango_layout_set_text(layout, buf, -1);
		pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
		g_object_unref (layout);
		return logical_rect.height;
	}
	return 0;
}

static void update_rectangles(CmpackChartView *view)
{
	gint size, x_height, y_width;
	gint cm_left, cm_top;
	PangoFontDescription *desc, *desc2;

	if (GTK_WIDGET_REALIZED(view)) {
		GtkWidget *widget = GTK_WIDGET(view);
		PangoContext *pg = gtk_widget_create_pango_context(widget);
		desc = pango_context_get_font_description(pg);
		desc2 = pango_font_description_copy(desc);
		size = pango_font_description_get_size(desc2);
		if (pango_font_description_get_size_is_absolute(desc2))
			pango_font_description_set_absolute_size(desc2, size*FONT_SCALE);
		else
			pango_font_description_set_size(desc2, (int)(size*FONT_SCALE));
		pango_context_set_font_description(pg, desc2);
		pango_font_description_free(desc2);

		y_width = x_height = (int)(text_height(widget, pg, "X") * 1.5);
		if (view->show_labels) {
			cm_top = x_height;			/* X axis is outside top */
			cm_left = y_width;			/* X axis is outside left */
		} else {
			cm_left = cm_top = 0;
		}

		view->canvas_rc.x = cm_left;
		view->canvas_rc.y = cm_top;
		view->canvas_rc.width = widget->allocation.width - cm_left;
		view->canvas_rc.height = widget->allocation.height - cm_top;

		if (view->show_labels) {
			view->xscale_rc.x = view->canvas_rc.x;
			view->xscale_rc.y = 0;
			view->xscale_rc.width = view->canvas_rc.width;
			view->xscale_rc.height = x_height;

			view->yscale_rc.x = 0;
			view->yscale_rc.y = view->canvas_rc.y;
			view->yscale_rc.width = y_width;
			view->yscale_rc.height = view->canvas_rc.height;
		} else {
			view->xscale_rc.x = view->xscale_rc.y = 0;
			view->xscale_rc.width = view->xscale_rc.height = 0;

			view->yscale_rc.x = view->yscale_rc.y = 0;
			view->yscale_rc.width = view->yscale_rc.height = 0;
		}

		g_object_unref(pg);
	}
}

/* Update mapping coefficients */
static void update_pxlsize(CmpackChartView *view)
{	
	view->pxl_size = pow(view->zoom_base, -view->zoom_pos);
}

/* Checks if the visible area is inside the limits */
static void restrict_x_to_limits(CmpackChartView *view)
{
	gint left, right;
    gdouble a, b;
	
	right = view->canvas_rc.x + view->canvas_rc.width;
    a = (view->x.ProjMax - view->x.ProjPos)/view->pxl_size;
    if (a < right) 
        view->x.ProjPos = view->x.ProjMax - view->pxl_size*right;
	left = view->canvas_rc.x;
    b = (view->x.ProjMin - view->x.ProjPos)/view->pxl_size;
    if (b > left)
        view->x.ProjPos = view->x.ProjMin - view->pxl_size*left;
}

static gboolean set_zoom_offset(CmpackChartView *view, gdouble zoom, gdouble xpos, gdouble ypos)
{
	zoom = LimitDbl(zoom, view->zoom_min, view->zoom_max);
	xpos = LimitDbl(xpos, view->x.ProjMin, view->x.ProjMax);
	ypos = LimitDbl(ypos, view->y.ProjMin, view->y.ProjMax);
	if (zoom!=view->zoom_pos || xpos!=view->x.ProjPos || ypos!=view->y.ProjPos) {
		view->zoom_pos = zoom;
		view->x.ProjPos = xpos;
		view->y.ProjPos = ypos;
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		return TRUE;
	}
	return FALSE;
}

static gboolean set_zoom(CmpackChartView *view, gdouble zoom)
{
	zoom = LimitDbl(zoom, view->zoom_min, view->zoom_max);
	if (zoom!=view->zoom_pos) {
		view->zoom_pos = zoom;
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		return TRUE;
	}
	return FALSE;
}

static gboolean set_offset(CmpackChartView *view, gdouble xpos, gdouble ypos)
{
	xpos = LimitDbl(xpos, view->x.ProjMin, view->x.ProjMax);
	ypos = LimitDbl(ypos, view->y.ProjMin, view->y.ProjMax);
	if (xpos!=view->x.ProjPos || ypos!=view->y.ProjPos) {
		view->x.ProjPos = xpos;
		view->y.ProjPos = ypos;
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		return TRUE;
	}
	return FALSE;
}

/* Fit chart to window */
static gboolean fit_to_window(CmpackChartView *view)
{
	double kx, ky, k, zoom;

	if (view->canvas_rc.width>0 && view->canvas_rc.height>0) {
		kx = (view->x.ProjMax-view->x.ProjMin)/view->canvas_rc.width;
		ky = (view->y.ProjMax-view->y.ProjMin)/view->canvas_rc.height;
		k = MAX(kx, ky);
		zoom = (k>0 ? -log(k) / log(view->zoom_base) : 0);
		zoom = LimitDbl(zoom, view->zoom_min, view->zoom_max);
		if (zoom != view->zoom_pos) {
			view->zoom_pos = zoom;
			view->x.ProjPos = view->y.ProjPos = 0;
			return TRUE;
		}
	}
	return FALSE;
}

/* Physical units -> projection units */
static gdouble x_to_proj(CmpackChartView *view, gdouble x)
{
	return (!view->x.Reverse ? 1.0 : -1.0) * x;
}

/* Projection units -> physical units */
static gdouble proj_to_x(CmpackChartView *view, gdouble u)
{
	return (!view->x.Reverse ? 1.0 : -1.0) * u;
}

/* Projection units -> display units */
static gdouble xproj_to_view(CmpackChartView *view, gdouble x)
{
	return (x - view->x.ProjPos)/view->pxl_size;
}

/* Display units -> projection units */
static gdouble view_to_xproj(CmpackChartView *view, gdouble u)
{
	return u*view->pxl_size + view->x.ProjPos;
}

/* Physical units -> display units */
static gdouble x_to_view(CmpackChartView *view, gdouble x)
{
	return xproj_to_view(view, x_to_proj(view, x));
}

/* Physical units -> projection units */
static gdouble view_to_x(CmpackChartView *view, gdouble u)
{
	return proj_to_x(view, view_to_xproj(view, u));
}

/* Update range and position of horizontal scroll bar */
static void update_hsb(CmpackChartView *view)
{
	gdouble pos, page_size, upper;

	GtkAdjustment *adj = view->hadjustment;
	if (adj && view->pxl_size>0) {
		if (!view->auto_zoom) {
			upper = RoundToInt((view->x.ProjMax - view->x.ProjMin) / view->pxl_size);
			page_size = view->canvas_rc.width;
			pos = (view->x.ProjPos - view->x.ProjMin) / view->pxl_size + view->canvas_rc.x;
		} else {
			upper = page_size = pos = 0;
		}
		g_object_set(G_OBJECT(adj), "upper", (gdouble)upper, "page-size", (gdouble)page_size, 
			"value", (gdouble)pos, NULL);
	}
}

/* Checks if the visible area is inside the limits */
static void restrict_y_to_limits(CmpackChartView *view)
{
	gint top, bottom;
    gdouble a, b;

	bottom = view->canvas_rc.y + view->canvas_rc.height;
    b = (view->y.ProjPos - view->y.ProjMin)/view->pxl_size;
    if (b < bottom)
        view->y.ProjPos = view->y.ProjMin + view->pxl_size*bottom;
	top = view->canvas_rc.y;
    a = (view->y.ProjPos - view->y.ProjMax)/view->pxl_size;
    if (a > top) 
        view->y.ProjPos = view->y.ProjMax + view->pxl_size*top;
}

/* Physical units -> projection units */
static gdouble y_to_proj(CmpackChartView *view, gdouble y)
{
	return (!view->y.Reverse ? -1.0 : 1.0) * y;
}

/* Projection units -> physical units */
static gdouble proj_to_y(CmpackChartView *view, gdouble v)
{
	return (!view->y.Reverse ? -1.0 : 1.0) * v;
}

/* Projection units -> display units */
static gdouble yproj_to_view(CmpackChartView *view, gdouble y)
{
	return (view->y.ProjPos - y)/view->pxl_size;
}

/* Display units -> projection units */
static gdouble view_to_yproj(CmpackChartView *view, gdouble v)
{
	return view->y.ProjPos - v*view->pxl_size;
}

/* Physical units -> display units */
static gdouble y_to_view(CmpackChartView *view, gdouble y)
{
	return yproj_to_view(view, y_to_proj(view, y));
}

/* Physical units -> projection units */
static gdouble view_to_y(CmpackChartView *view, gdouble v)
{
	return proj_to_y(view, view_to_yproj(view, v));
}

/* Update range and position of vertical scroll bar */
static void update_vsb(CmpackChartView *view)
{
	gdouble pos, upper, page_size;

	GtkAdjustment *adj = view->vadjustment;
	if (adj && view->pxl_size>0) {
		if (!view->auto_zoom) {
			upper = RoundToInt((view->y.ProjMax - view->y.ProjMin) / view->pxl_size);
			page_size = view->canvas_rc.height;
			pos = (view->y.ProjMax - view->y.ProjPos) / view->pxl_size + view->canvas_rc.y;
		} else {
			upper = page_size = pos = 0;
		}
		g_object_set(G_OBJECT(adj), "upper", (gdouble)upper, "page-size", (gdouble)page_size, 
			"value", (gdouble)pos, NULL);
	}
}

/*---------------------   SCALE LABELS & GRIDS   ---------------------------*/

/* Standard formatting function for scale labels */
static void format_label(gchar *buf, gdouble value)
{
	sprintf(buf, "%.0f", value);
	if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
		sprintf(buf, "%.0f", 0.0);
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_grid_step(CmpackChartView *view, PangoContext *pg, 
	gdouble *bigstep, gdouble *smallstep)
{
	gdouble dx, xstep, xstep1, delta;
	gchar	buf[256];
	
    // Decimal mode
	xstep = xstep1 = fabs(view->x.ProjMax - view->x.ProjMin);
	if (view->pxl_size>0) {
		format_label(buf, MAX(fabs(view->x.ProjMin), fabs(view->x.ProjMax)));
		dx = (text_width(GTK_WIDGET(view), pg, buf)+4)*view->pxl_size*3.0;
		delta = pow(10.0, floor(log10(dx)));
		if (delta>dx) {
			xstep = delta;
			xstep1 = delta/5.0;
		} else if (2.0 * delta>dx) {
			xstep = 2.0 * delta;
			xstep1 = xstep/4.0;
		} else if (5.0 * delta>dx) {
			xstep = 5.0 * delta;
			xstep1 = xstep/5.0; 
		} else {
			xstep = 10.0 * delta;
			xstep1 = xstep/5.0;
		}
	}
	if (bigstep)
		*bigstep = xstep;
	if (smallstep)
		*smallstep = xstep1;
}

//------------------------   PAINTING   -----------------------------------

/* Paint labels on x-axis scale */
static void paint_scales(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gchar	buf[256];
	gdouble	val, bigstep, smallstep, xmin, xmax, ymin, ymax;
	gint	size, d, x, y, left, right, top, bottom;
	PangoFontDescription *desc, *desc2;
	PangoLayout *layout;

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->text_gc[widget->state]);

	PangoContext *pg = gtk_widget_create_pango_context(widget);
	desc = pango_context_get_font_description(pg);
	desc2 = pango_font_description_copy(desc);
	size = pango_font_description_get_size(desc2);
	if (pango_font_description_get_size_is_absolute(desc2))
		pango_font_description_set_absolute_size(desc2, size*FONT_SCALE);
	else
		pango_font_description_set_size(desc2, (int)(size*FONT_SCALE));
	pango_context_set_font_description(pg, desc2);
	pango_font_description_free(desc2);
	layout = pango_layout_new(pg);

	compute_grid_step(view, pg, &bigstep, &smallstep);
	d = text_height(widget, pg, "X");

	/* Horizontal ruler */
	gdk_gc_set_clip_rectangle(gc, &view->xscale_rc);
	top = view->xscale_rc.y;
	bottom = view->xscale_rc.y + view->yscale_rc.height;
	left = view->xscale_rc.x;
	right = view->xscale_rc.x + view->xscale_rc.width;
	if (!view->x.Reverse) {
		xmin = view_to_x(view, view->canvas_rc.x);
		xmax = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);
	} else {
		xmin = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);
		xmax = view_to_x(view, view->canvas_rc.x);
	}
	xmin = LimitDbl(xmin, view->x.Min, view->x.Max);
	xmax = LimitDbl(xmax, view->x.Min, view->x.Max);
	pango_layout_set_width(layout, -1);
	for (val = ceil(xmin/smallstep)*smallstep; val<=xmax; val+=smallstep) {
		x = RoundToInt(x_to_view(view, val));
		if (x >= left && x < right) 
			gdk_draw_line(drawable, gc, x, top + d, x, bottom);
	}
	for (val = ceil(xmin/bigstep)*bigstep; val<=xmax; val+=bigstep) {
		x = RoundToInt(x_to_view(view, val));
		if (x >= left && x < right) {
			gdk_draw_line(drawable, gc, x, top + 2, x, bottom);
			format_label(buf, val);
			pango_layout_set_text(layout, buf, -1);
			if (!view->x.Reverse) {
				int text_width;
				pango_layout_get_pixel_size(layout, &text_width, NULL);
				if (x + 2 + text_width < right) 
					gdk_draw_layout(drawable, gc, x + 2, top, layout);
			} else {
				int text_width;
				pango_layout_get_pixel_size(layout, &text_width, NULL);
				if (x - 2 - text_width >= left) 
					gdk_draw_layout(drawable, gc, x - 2 - text_width, top, layout);
			}
		}
	}

	/* Vertical ruler */
	gdk_gc_set_clip_rectangle(gc, &view->yscale_rc);
	left = view->yscale_rc.x;
	right = view->yscale_rc.x + view->yscale_rc.width;
	top = view->yscale_rc.y;
	bottom = view->yscale_rc.y + view->yscale_rc.height;
	if (!view->y.Reverse) {
		ymin = view_to_y(view, view->canvas_rc.y); 
		ymax = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height);
	} else {
		ymin = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height); 
		ymax = view_to_y(view, view->canvas_rc.y);
	}
	ymin = LimitDbl(ymin, view->y.Min, view->y.Max);
	ymax = LimitDbl(ymax, view->y.Min, view->y.Max);
	pango_layout_set_width(layout, 1);
	pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
	for (val=ceil(ymin/smallstep)*smallstep; val<=ymax; val+=smallstep) {
		y = RoundToInt(y_to_view(view, val));
		if (y >= top && y < bottom)
			gdk_draw_line(drawable, gc, left + d, y, right, y);
	}
	for (val=ceil(ymin/bigstep)*bigstep; val<=ymax; val+=bigstep) {
		y = RoundToInt(y_to_view(view, val));
		if (y >= top && y < bottom) {
			gdk_draw_line(drawable, gc, left + 2, y, right, y);
			format_label(buf, val);
			pango_layout_set_text(layout, buf, -1);
			if (!view->y.Reverse) {
				int text_height;
				pango_layout_get_pixel_size(layout, NULL, &text_height);
				if (y + 2 + text_height < bottom)
					gdk_draw_layout(drawable, gc, left + 2, y + 2, layout);
			} else {
				int text_height;
				pango_layout_get_pixel_size(layout, NULL, &text_height);
				if (y - 2 - text_height >= top)
					gdk_draw_layout(drawable, gc, left + 2, y - 2 - text_height, layout);
			}
		}
	}

	g_object_unref(layout);
	g_object_unref(pg);
	g_object_unref(gc);
}

/* Paints x-axis grid */
static void paint_grid(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gdouble	val, step, xmin, xmax, ymin, ymax;
	gint	size, x, y, x0, x1, y0, y1, left, right, top, bottom;
	PangoFontDescription *desc, *desc2;

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->dark_gc[widget->state]);

	PangoContext *pg = gtk_widget_create_pango_context(widget);
	desc = pango_context_get_font_description(pg);
	desc2 = pango_font_description_copy(desc);
	size = pango_font_description_get_size(desc2);
	if (pango_font_description_get_size_is_absolute(desc2))
		pango_font_description_set_absolute_size(desc2, size*FONT_SCALE);
	else
		pango_font_description_set_size(desc2, (int)(size*FONT_SCALE));
	pango_context_set_font_description(pg, desc2);
	pango_font_description_free(desc2);

	x0 = (int)floor(xproj_to_view(view, view->x.ProjMin));
	left = MAX(x0, view->canvas_rc.x);
	x1 = (int)ceil(xproj_to_view(view, view->x.ProjMax));
	right = MIN(x1, view->canvas_rc.x + view->canvas_rc.width);
	y0 = (int)floor(yproj_to_view(view, view->y.ProjMax));
	top = MAX(y0, view->canvas_rc.y);
	y1 = (int)ceil(yproj_to_view(view, view->y.ProjMin));
	bottom = MIN(y1, view->canvas_rc.y + view->canvas_rc.height);

	compute_grid_step(view, pg, &step, NULL);

    /* Visible region in physical units */
	if (!view->x.Reverse) {
		xmin = view_to_x(view, view->canvas_rc.x);
		xmax = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);
	} else {
		xmin = view_to_x(view, view->canvas_rc.x + view->canvas_rc.width);
		xmax = view_to_x(view, view->canvas_rc.x);
	}
	xmin = LimitDbl(xmin, view->x.Min, view->x.Max);
	xmax = LimitDbl(xmax, view->x.Min, view->x.Max);
	for (val=ceil(xmin/step)*step; val<=xmax; val+=step) {
		x = RoundToInt(x_to_view(view, val));
		if (x >= left && x < right)
			gdk_draw_line(drawable, gc, x, top, x, bottom);
	}
	if (!view->y.Reverse) {
		ymin = view_to_y(view, view->canvas_rc.y); 
		ymax = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height);
	} else {
		ymin = view_to_y(view, view->canvas_rc.y + view->canvas_rc.height); 
		ymax = view_to_y(view, view->canvas_rc.y);
	}
	ymin = LimitDbl(ymin, view->y.Min, view->y.Max);
	ymax = LimitDbl(ymax, view->y.Min, view->y.Max);
	for (val=ceil(ymin/step)*step; val<ymax; val+=step) {
		y = RoundToInt(y_to_view(view, val));
		if (y >= top && y < bottom)
			gdk_draw_line(drawable, gc, left, y, right, y);
	}

	g_object_unref(pg);
	g_object_unref(gc);
}

static void invalidate_cursor(CmpackChartView *view)
{
	gint x, y, r;

	CmpackChartViewItem *item = &view->items[view->focused_item];
	r = RoundToInt(3.0 + 1.5 * item->size / sqrt(view->pxl_size));
	if (r < 5)
		r = 5;
	x = RoundToInt(xproj_to_view(view, item->xproj));
	y = RoundToInt(yproj_to_view(view, item->yproj));
	r = r + CURSOR_SIZE + 3;
	gtk_widget_queue_draw_area(GTK_WIDGET(view), x-r, y-r, 2*r+1, 2*r+1);
}

static void invalidate_profile(CmpackChartView *view)
{
	gint x1, y1, x2, y2, left, top, width, height;

	if (view->profile_valid) {
		x1 = RoundToInt(xproj_to_view(view, view->profile_x[0]));
		y1 = RoundToInt(yproj_to_view(view, view->profile_y[0]));
		x2 = RoundToInt(xproj_to_view(view, view->profile_x[1]));
		y2 = RoundToInt(yproj_to_view(view, view->profile_y[1]));
		left = MIN(x1, x2) - 4;
		width = MAX(x1, x2) + 4 - left + 1;
		top = MIN(y1, y2) - 4;
		height = MAX(y1, y2) + 4 - top + 1;
		gtk_widget_queue_draw_area(GTK_WIDGET(view), left, top, width, height);
	}
}

static gboolean timer_cb(CmpackChartView *view)
{
	if (view->focused_item>=0) {
		view->cursor_phase = (view->cursor_phase+5) % 360;
		invalidate_cursor(view);
		return TRUE;
	} else {
		view->timer_id = 0;
		return FALSE;
	}
}

static gboolean point_in_rect(gint x, gint y, const GdkRectangle *rc)
{
	return (x>=rc->x && y>=rc->y && x<rc->x+rc->width && y<rc->y+rc->height);
}

static gboolean point_on_profile_point(CmpackChartView *view, gint x0, gint y0, gint pt)
{
	gint x, y, r2;

	x = RoundToInt(xproj_to_view(view, view->profile_x[pt]));
	y = RoundToInt(yproj_to_view(view, view->profile_y[pt]));
	r2 = (x-x0)*(x-x0) + (y-y0)*(y-y0);
	return (r2 < CLICK_TOLERANCE * CLICK_TOLERANCE);
}

static gboolean point_on_profile(CmpackChartView *view, gint x0, gint y0)
{
	gint	x1, x2, y1, y2;
	gdouble d, t, r2;

	x1 = RoundToInt(xproj_to_view(view, view->profile_x[0]));
	y1 = RoundToInt(yproj_to_view(view, view->profile_y[0]));
	x2 = RoundToInt(xproj_to_view(view, view->profile_x[1]));
	y2 = RoundToInt(yproj_to_view(view, view->profile_y[1]));

	d = (gdouble)(x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
	if (d>0) {
		t = (gdouble)(x2-x1)*(x0-x1) + (y2-y1)*(y0-y1);
		if (t<=0)
			r2 = sqrt((gdouble)(x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
		else if (t>=d)
			r2 = sqrt((gdouble)(x2-x0)*(x2-x0) + (y2-y0)*(y2-y0));
		else
			r2 = fabs((gdouble)(y2-y1)*(x0-x1) - (x2-x1)*(y0-y1))/sqrt(d);
	} else
		r2 = sqrt((gdouble)(x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
	return (r2 < CLICK_TOLERANCE);
}

static GdkCursor *get_cursor(GtkWidget *widget, CmpackChartCursor cursor_type)
{
	GdkDisplay *display = gtk_widget_get_display(widget);

	switch (cursor_type)
	{
	case CHART_CURSOR_ARROW_NS:
		return gdk_cursor_new_for_display(display, GDK_SB_V_DOUBLE_ARROW);
	case CHART_CURSOR_ARROW_WE:
		return gdk_cursor_new_for_display(display, GDK_SB_H_DOUBLE_ARROW);
	case CHART_CURSOR_ARROW_NSWE:
		return gdk_cursor_new_for_display(display, GDK_FLEUR);
	case CHART_CURSOR_CROSSHAIR:
		return gdk_cursor_new_for_display(display, GDK_TCROSS);
	default:
		return NULL;
	}
}

static void update_cursor(CmpackChartView *view)
{
	GtkWidget *widget;
	CmpackChartCursor type;
	GdkCursor *cursor;

	if (GTK_WIDGET_REALIZED(view)) {
		widget = GTK_WIDGET(view);
		if (view->mouse_mode==CHART_MOUSE_ZOOM || view->mouse_mode==CHART_MOUSE_SELECT ||
			view->mouse_mode==CHART_MOUSE_MOVE_START || view->mouse_mode==CHART_MOUSE_MOVE_END ||
			view->mouse_mode==CHART_MOUSE_NEW_PROFILE)
			type = CHART_CURSOR_CROSSHAIR;
		else if (view->mouse_mode==CHART_MOUSE_SHIFT || view->mouse_mode==CHART_MOUSE_MOVE_PROFILE)
			type = CHART_CURSOR_ARROW_NSWE;
		else if (view->mouse_mode==CHART_MOUSE_HSHIFT)
			type = CHART_CURSOR_ARROW_WE;
		else if (view->mouse_mode==CHART_MOUSE_VSHIFT)
			type = CHART_CURSOR_ARROW_NS;
		else if (view->mouse_pos & (CHART_MOUSE_PROFILE | CHART_MOUSE_PSTART | CHART_MOUSE_PEND))
			type = CHART_CURSOR_ARROW_NSWE;
		else
			type = CHART_CURSOR_DEFAULT;
		if (view->cursor != type) {
			cursor = get_cursor(widget, type);
			gdk_window_set_cursor(widget->window, cursor);
			if (cursor)
				gdk_cursor_unref (cursor); 
		}
		view->cursor = type;
	}
}

static gboolean cmpack_chart_view_motion (GtkWidget *widget, GdkEventMotion *event)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(widget);
	GdkRectangle old_area, new_area, common;
	GdkRegion *invalid_region, *common_region;
	gdouble dx, dy, x0, y0, left, right, top, bottom;
	gint x, y, row;
	guint new_pos;

	gdk_window_get_pointer (widget->window, &x, &y, NULL);

	switch (view->mouse_mode)
	{
	case CHART_MOUSE_ZOOM:
	case CHART_MOUSE_SELECT:
		x = LimitInt(x, view->canvas_rc.x, view->canvas_rc.x + view->canvas_rc.width);
		y = LimitInt(y, view->canvas_rc.y, view->canvas_rc.y + view->canvas_rc.height);

		old_area.x = MIN (view->mouse_start_x, view->mouse_pos_x);
		old_area.y = MIN (view->mouse_start_y, view->mouse_pos_y);
		old_area.width = ABS (view->mouse_pos_x - view->mouse_start_x) + 1;
		old_area.height = ABS (view->mouse_start_y - view->mouse_pos_y) + 1;
  
		new_area.x = MIN (view->mouse_start_x, x);
		new_area.y = MIN (view->mouse_start_y, y);
		new_area.width = ABS (x - view->mouse_start_x) + 1;
		new_area.height = ABS (y - view->mouse_start_y) + 1;

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
		view->mouse_pos_x = x;
		view->mouse_pos_y = y;  
		break;

	case CHART_MOUSE_SHIFT:
		dx = view->mouse_ref_x + view->pxl_size*(view->mouse_start_x - x);
		dy = view->mouse_ref_y - view->pxl_size*(view->mouse_start_y - y);
		if (dx!=view->x.ProjPos || dy!=view->y.ProjPos) {
			view->x.ProjPos = dx;
			view->y.ProjPos = dy;
			restrict_x_to_limits(view);
			restrict_y_to_limits(view);
			update_hsb(view);
			update_vsb(view);
			invalidate(view);
		}
		break;

	case CHART_MOUSE_HSHIFT:
		dx = view->mouse_ref_x + view->pxl_size*(view->mouse_start_x - x);
		if (dx!=view->x.ProjPos) {
			view->x.ProjPos = dx;
			restrict_x_to_limits(view);
			update_hsb(view);
			invalidate(view);
		}
		break;

	case CHART_MOUSE_VSHIFT:
		dy = view->mouse_ref_y - view->pxl_size*(view->mouse_start_y - y);
		if (dy!=view->y.ProjPos) {
			view->y.ProjPos = dy;
			restrict_y_to_limits(view);
			update_vsb(view);
			invalidate(view);
		}
		break;

	case CHART_MOUSE_MOVE_PROFILE:
		dx = view->profile_x[1] - view->profile_x[0];
		dy = view->profile_y[1] - view->profile_y[0];
		x0 = view_to_xproj(view, x) + view->mouse_ref_x;
		left = (dx>=0 ? x0 : x0+dx);
		if (left < view->x.ProjMin)
			x0 += view->x.ProjMin - left;
		right = (dx>=0 ? x0+dx : x0);
		if (right > view->x.ProjMax)
			x0 -= right - view->x.ProjMax;
		y0 = view_to_yproj(view, y) + view->mouse_ref_y;
		top = (dy>=0 ? y0 : y0+dy);
		if (top < view->y.ProjMin)
			y0 += view->y.ProjMin - top;
		bottom = (dy>=0 ? y0+dy : y0);
		if (bottom > view->y.ProjMax)
			y0 -= bottom - view->y.ProjMax;
		if (x0!=view->profile_x[0] || y0!=view->profile_y[0]) {
			invalidate_profile(view);
			view->profile_x[0] = x0;
			view->profile_x[1] = x0 + dx;
			view->profile_y[0] = y0;
			view->profile_y[1] = y0 + dy;
			invalidate_profile(view);
		}
		break;

	case CHART_MOUSE_MOVE_START:
		dx = LimitDbl(view_to_xproj(view, x), view->x.ProjMin, view->x.ProjMax);
		dy = LimitDbl(view_to_yproj(view, y), view->y.ProjMin, view->y.ProjMax);
		if (dx!=view->profile_x[0] || dy!=view->profile_y[0]) {
			invalidate_profile(view);
			view->profile_x[0] = dx;
			view->profile_y[0] = dy;
			invalidate_profile(view);
		}
		break;

	case CHART_MOUSE_MOVE_END:
	case CHART_MOUSE_NEW_PROFILE:
		dx = LimitDbl(view_to_xproj(view, x), view->x.ProjMin, view->x.ProjMax);
		dy = LimitDbl(view_to_yproj(view, y), view->y.ProjMin, view->y.ProjMax);
		if (dx!=view->profile_x[1] || dy!=view->profile_y[1]) {
			invalidate_profile(view);
			view->profile_x[1] = dx;
			view->profile_y[1] = dy;
			invalidate_profile(view);
		}
		view->mouse_pos_x = x;
		view->mouse_pos_y = y;  
		break;
	
	default:
		new_pos = CHART_MOUSE_OUTSIDE;
		if (view->show_labels && point_in_rect(x, y, &view->xscale_rc)) 
			new_pos = CHART_MOUSE_HSCALE;
		else if (view->show_labels && point_in_rect(x, y, &view->yscale_rc)) 
			new_pos = CHART_MOUSE_VSCALE;
		else if (point_in_rect(x, y, &view->canvas_rc)) {
			new_pos = CHART_MOUSE_CANVAS;
			if (view->profile_enabled && view->profile_valid && point_on_profile_point(view, x, y, 0))
				new_pos |= CHART_MOUSE_PSTART; 
			else if (view->profile_enabled && view->profile_valid && point_on_profile_point(view, x, y, 1))
				new_pos |= CHART_MOUSE_PEND; 
			else if (view->profile_enabled && view->profile_valid && point_on_profile(view, x, y))
				new_pos |= CHART_MOUSE_PROFILE;
			dx = view_to_xproj(view, x);
			dy = view_to_yproj(view, y);
			if (dx>=view->x.ProjMin && dx<=view->x.ProjMax && dy>=view->y.ProjMin && dy<=view->y.ProjMax)
				new_pos |= CHART_MOUSE_CHART;
		}
		if (new_pos != view->mouse_pos) {
			view->mouse_pos = new_pos;
			update_cursor(view);
		}
	}

	if (view->mouse_pos & CHART_MOUSE_CHART) {
		if (x!=view->last_mouse_x || y!=view->last_mouse_y) {
			view->last_mouse_x = x;
			view->last_mouse_y = y;
			if ((view->activation_mode || view->selection_mode) && !view->profile_enabled) {
				row = find_item(view, x, y);
				if (row>=0 && !view->items[row].enabled)
					row = -1;
				if (row!=view->focused_item) {
					if (view->focused_item>=0)
						invalidate_cursor(view);
					view->focused_item = row;
					if (view->focused_item>=0) {
						view->cursor_phase = 0;
						if (!view->timer_id) 
							view->timer_id = g_timeout_add(100, GSourceFunc(timer_cb), view);
						invalidate_cursor(view);
					}
				}
			}
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		} 
	} else {
		if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
			view->last_mouse_x = view->last_mouse_y = -1;
			if (view->focused_item>=0) {
				invalidate_cursor(view);
				view->focused_item = -1;
			}
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		}
	}
	
	return FALSE;
}

static gboolean cmpack_chart_view_leave(GtkWidget *widget, GdkEventCrossing *event)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(widget);

	if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
		view->mouse_pos = CHART_MOUSE_OUTSIDE;
		view->last_mouse_x = view->last_mouse_y = -1;
		if (view->focused_item>=0) {
			invalidate_cursor(view);
			view->focused_item = -1;
		}
		g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
	}
	
	return FALSE;
}

static gboolean cmpack_chart_view_button_press (GtkWidget *widget, GdkEventButton *event)
{
	gint x, y;
	CmpackChartView *view;
	gboolean selection_changed = FALSE;

	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);

	view = CMPACK_CHART_VIEW (widget);
	x = (gint)event->x;
	y = (gint)event->y;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		if (view->mouse_pos & CHART_MOUSE_CHART) {
			if (view->profile_enabled) {
				if (view->mouse_pos & CHART_MOUSE_PSTART) {
					// Move profile start
					cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_MOVE_START);
				} else
				if (view->mouse_pos & CHART_MOUSE_PEND) {
					// Move profile end
					cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_MOVE_END);
				} else
				if (view->profile_enabled && (view->mouse_pos & CHART_MOUSE_PROFILE)) {
					// Move whole profile
					cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_MOVE_PROFILE);
				} else {
					// Start a new profile
					invalidate_profile(view);
					view->profile_valid = TRUE;
					view->profile_x[0] = view->profile_x[1] = view_to_xproj(view, x);
					view->profile_y[0] = view->profile_y[1] = view_to_yproj(view, y);
					invalidate_profile(view);
					cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_NEW_PROFILE);
				}
			} else 
			if (view->selection_mode==GTK_SELECTION_MULTIPLE && (event->state & GDK_SHIFT_MASK)) {
				// Start select region mode
				cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_SELECT);
			} else
			if (view->mouse_ctrl && (event->state & GDK_CONTROL_MASK)) {
				// Start zoom to region mode
				cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_ZOOM);
			} else
			if (view->mouse_ctrl && (!view->selection_mode && !view->activation_mode)) {
				// // Start panning mode
				cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_SHIFT);
			} else
			if (view->selection_mode || view->activation_mode) {
				gint index = find_item(view, x, y);
				if (index>=0 && view->items[index].enabled && view->items[index].visible) {
					// Single click selects item
					if (view->selection_mode != GTK_SELECTION_NONE && !view->items[index].selected) {
						cmpack_chart_view_unselect_all_internal(view);
						view->items[index].selected = TRUE;
						selection_changed = TRUE;
						invalidate(view);
					}
					// Activation by single left click
					if (view->activation_mode == CMPACK_ACTIVATION_CLICK) 
						cmpack_chart_view_item_activate(view, index);
					// Remember last item
					if (view->last_single_clicked < 0)
						view->last_single_clicked = index;
				} else {
					// Single click deselect item
					if (view->selection_mode != GTK_SELECTION_NONE && view->selection_mode != GTK_SELECTION_BROWSE)
						selection_changed = cmpack_chart_view_unselect_all_internal(view);
					// Clear last item
					if (view->last_single_clicked >= 0)
						view->last_single_clicked = -1;
					// Start panning mode
					if (view->mouse_ctrl)
						cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_SHIFT);
				}
			}
		} else
		if (view->mouse_pos & CHART_MOUSE_HSCALE) 
			cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_HSHIFT);
		else if (view->mouse_pos & CHART_MOUSE_VSCALE)
			cmpack_chart_view_enter_mouse_mode(view, x, y, CHART_MOUSE_VSHIFT);
	}

	// Activation by left double click
	if (event->button==1 && event->type==GDK_2BUTTON_PRESS &&
		view->activation_mode == CMPACK_ACTIVATION_DBLCLICK &&
		!view->profile_enabled && point_in_rect(x, y, &view->canvas_rc)) {
			gint index = find_item(view, x, y);
			if (index>=0 && index == view->last_single_clicked &&
				view->items[index].enabled && view->items[index].visible) {
					cmpack_chart_view_item_activate(view, index);
			}
			view->last_single_clicked = -1;
	}

	if (selection_changed) 
		g_signal_emit (view, chart_view_signals[SELECTION_CHANGED], 0);
	
	return event->button == 1; 
}

static gboolean cmpack_chart_view_button_release (GtkWidget *widget, GdkEventButton *event)
{
	gint i;
	gboolean dirty, selected;
	gdouble x0, y0, dx, dy, x1, y1, zx, zy;
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);
	GdkRectangle rect;
  
	if (view->mouse_mode) {
		rect.x = MIN(view->mouse_start_x, view->mouse_pos_x);
		rect.y = MIN(view->mouse_start_y, view->mouse_pos_y);
		rect.width = ABS(view->mouse_pos_x - view->mouse_start_x) + 1;
		rect.height = ABS(view->mouse_pos_y - view->mouse_start_y) + 1;

		switch (view->mouse_mode)
		{
		case CHART_MOUSE_NEW_PROFILE:
			x0 = view->mouse_start_x;
			y0 = view->mouse_start_y;
			x1 = view->mouse_pos_x;
			y1 = view->mouse_pos_y;
			view->profile_valid = (sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)) >= 8);
			invalidate_profile(view);
			g_signal_emit(view, chart_view_signals[PROFILE_CHANGED], 0);
			break;

		case CHART_MOUSE_MOVE_PROFILE:
		case CHART_MOUSE_MOVE_START:
		case CHART_MOUSE_MOVE_END:
			g_signal_emit(view, chart_view_signals[PROFILE_CHANGED], 0);
			break;

		case CHART_MOUSE_SELECT:
			// Select points in an area
			x0 = view_to_xproj(view, rect.x);
			x1 = view_to_xproj(view, rect.x + rect.width);
			y0 = view_to_yproj(view, rect.y + rect.height);
			y1 = view_to_yproj(view, rect.y);
			dirty = FALSE;
			for (i=0; i<view->item_count; i++) {
				CmpackChartViewItem *item = &view->items[i];
				selected = (item->visible && item->enabled && item->xproj>=x0 && item->xproj<=x1 && 
					item->yproj>=y0 && item->yproj<=y1);
				if (item->selected!=selected) {
					item->selected = selected;
					dirty = TRUE;
				}
			}
			if (dirty) 
				g_signal_emit(view, chart_view_signals[SELECTION_CHANGED], 0);
			invalidate(view);
			break;

		case CHART_MOUSE_ZOOM:
			// Zoom to selected area 
			view->auto_zoom = FALSE;
			dx = view_to_xproj(view, rect.x);
			dy = view_to_yproj(view, rect.y);
			zx = log(view->canvas_rc.width/(rect.width*view->pxl_size))/log(view->zoom_base);
			zy = log(view->canvas_rc.height/(rect.height*view->pxl_size))/log(view->zoom_base);
			if (set_zoom_offset(view, MIN(zx, zy), dx, dy)) {
				g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
				if (view->last_mouse_x>=0 || view->last_mouse_y>=0) 
					g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
			}
			invalidate(view);
			break;

		default:
			break;
		}
		cmpack_chart_view_leave_mouse_mode(view);
	}
	return TRUE;
}

static gboolean cmpack_chart_view_key_press (GtkWidget *widget, GdkEventKey *event)
{
	gint i;
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);

	if (view->mouse_mode) {
		if (event->keyval == GDK_Escape) {
			if (view->mouse_mode == CHART_MOUSE_SELECT) {
				for (i=0; i<view->item_count; i++) {
					CmpackChartViewItem *item = &view->items[i];
					item->selected = item->selected_before_rubberbanding;
				}
				invalidate(view);
			}
			cmpack_chart_view_leave_mouse_mode(view);
		}
		return TRUE;
	}
	return GTK_WIDGET_CLASS (cmpack_chart_view_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_chart_view_key_release (GtkWidget *widget, GdkEventKey *event)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);

	if (view->mouse_mode)
		return TRUE;

	return GTK_WIDGET_CLASS (cmpack_chart_view_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_chart_view_scroll(GtkWidget *widget, GdkEventScroll *event)
{
	CmpackChartView *view = CMPACK_CHART_VIEW (widget);

	switch (event->direction)
	{
	case GDK_SCROLL_UP:
		// Center + increase zoom
		if (view->mouse_ctrl && (view->zoom_pos < view->zoom_max)) 
			cmpack_chart_view_set_zoom(view, view->zoom_pos + 2.0);
		break;

	case GDK_SCROLL_DOWN:
		// Decrease zoom
		if (view->mouse_ctrl && (view->zoom_pos > view->zoom_min))
			cmpack_chart_view_set_zoom(view, view->zoom_pos - 2.0);
		break;

	default:
		break;
	}

	return TRUE;
}

static void object_inserted(CmpackChartData *model, gint row, gpointer data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	if (row>=0 && row<=view->item_count) {
		if (view->item_count >= view->item_capacity) {
			view->item_capacity += 64;
			view->items = (CmpackChartViewItem*)g_realloc(view->items, view->item_capacity*sizeof(CmpackChartViewItem));
		}
		if (row < view->item_count)
			memmove(view->items+(row+1), view->items+row, (view->item_count-row)*sizeof(CmpackChartViewItem));
		memset(view->items + row, 0, sizeof(CmpackChartViewItem));
		update_item(view, view->items + row, model, row);
		view->item_count++;
		invalidate(view);
	}
}

static void object_updated(CmpackChartData *model, gint row, gpointer data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	if (row>=0 && row<view->item_count) {
		update_item(view, view->items + row, model, row);
		invalidate(view);
	}
} 

static void object_deleted(CmpackChartData *model, gint row, gpointer data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	if (row>=0 && row<view->item_count) {
		if (row < view->item_count-1)
			memmove(view->items+row, view->items+(row+1), (view->item_count-row-1)*sizeof(CmpackChartViewItem));
		view->item_count--;
		if (row == view->focused_item)
			view->focused_item = -1;
		invalidate(view);
	}
} 

static void dim_changed(CmpackChartData *model, gpointer data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	update_limits(view);
	update_data(view);
	invalidate(view);
} 

static void data_cleared(CmpackChartData *model, gpointer data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	clear_data(view);
	invalidate(view);
} 

static void cmpack_chart_view_enter_mouse_mode(CmpackChartView  *view,
				   gint          x,
				   gint          y, 
				   CmpackChartMouseMode mode)
{
	gint i;

	if (!view->mouse_mode) {
		if (mode == CHART_MOUSE_SELECT) {
			for (i=0; i<view->item_count; i++) {
				CmpackChartViewItem *item = view->items + i;
				item->selected_before_rubberbanding = item->selected;
			}
		}
		if (mode == CHART_MOUSE_SHIFT || mode == CHART_MOUSE_HSHIFT || mode == CHART_MOUSE_VSHIFT) {
			view->mouse_ref_x = view->x.ProjPos;
			view->mouse_ref_y = view->y.ProjPos;
		}
		if (mode == CHART_MOUSE_MOVE_PROFILE) {
			view->mouse_ref_x = view->profile_x[0] - view_to_xproj(view, x);
			view->mouse_ref_y = view->profile_y[0] - view_to_yproj(view, y);
		}
		if (view->focused_item)
			view->focused_item = -1;
		view->mouse_mode = mode;
		view->mouse_pos_x = view->mouse_start_x = x;
		view->mouse_pos_y = view->mouse_start_y = y;
		gtk_grab_add (GTK_WIDGET (view));
		update_cursor(view);
	}
}

static void cmpack_chart_view_leave_mouse_mode(CmpackChartView *view)
{
	if (view->mouse_mode) {
		view->mouse_mode = CHART_MOUSE_NONE;
		gtk_grab_remove (GTK_WIDGET (view));
		gtk_widget_queue_draw (GTK_WIDGET (view));
		update_cursor(view);
	}
}

static gboolean cmpack_chart_view_unselect_all_internal (CmpackChartView  *view)
{
	gboolean dirty = FALSE;
	gint i;

	if (view->selection_mode == GTK_SELECTION_NONE)
		return FALSE;

	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->selected) {
			item->selected = FALSE;
			dirty = TRUE;
		}
	}
	if (dirty)
		invalidate(view);
	return dirty;
}

/* CmpackChartView signals */
static void cmpack_chart_view_set_adjustments (CmpackChartView   *view,
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
			G_CALLBACK (cmpack_chart_view_adjustment_changed), view);
		g_object_set(G_OBJECT(hadj), "lower", (gdouble)0, "page-increment", (gdouble)10, 
			"step-increment", (gdouble)1, NULL);
		need_adjust = TRUE;
    }
	if (view->vadjustment != vadj) {
		view->vadjustment = vadj;
		g_object_ref_sink (view->vadjustment);
		g_signal_connect (view->vadjustment, "value-changed",
			G_CALLBACK (cmpack_chart_view_adjustment_changed), view);
		g_object_set(G_OBJECT(vadj), "lower", (gdouble)0, "page-increment", (gdouble)10, 
			"step-increment", (gdouble)1, NULL);
		need_adjust = TRUE;
	}
	if (need_adjust)
		cmpack_chart_view_adjustment_changed (NULL, view);
}

/* Handles scroll bars */
static void cmpack_chart_view_adjustment_changed (GtkAdjustment *adjustment, CmpackChartView *view)
{
	gdouble pos;
	gboolean dirty = FALSE;

	if (!adjustment || adjustment == view->hadjustment) {
		/* HSB */
		pos = view->x.ProjMin + view->pxl_size * (view->hadjustment->value - view->canvas_rc.y);
		if (view->x.ProjPos != pos) {
			view->x.ProjPos = pos;
			restrict_x_to_limits(view);
			dirty = TRUE;
		}
	}
	if (!adjustment || adjustment == view->vadjustment) {
		/* VSB */
		pos = view->y.ProjMax - view->pxl_size * (view->vadjustment->value - view->canvas_rc.y);
		if (view->y.ProjPos != pos) {
			view->y.ProjPos = pos;
			restrict_x_to_limits(view);
			dirty = TRUE;
		}
	}
	if (dirty) {
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
	}
}

static void cmpack_chart_view_queue_draw_item (CmpackChartView *view, CmpackChartViewItem *item)
{
	int d;
	GdkRectangle rect;

	d = RoundToInt(item->size / sqrt(view->pxl_size));
	rect.x = RoundToInt(xproj_to_view(view, item->xproj)) - (d+1)/2;
	rect.y = RoundToInt(yproj_to_view(view, item->yproj)) - (d+1)/2;
	rect.width = rect.height = d;
	view->dirty = TRUE;
	gdk_window_invalidate_rect(GTK_WIDGET(view)->window, &rect, TRUE);
}

static void cmpack_chart_view_select_item (CmpackChartView *view, CmpackChartViewItem  *item)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (item != NULL);

	if (item->selected || !item->enabled || !item->visible || view->selection_mode == GTK_SELECTION_NONE)
		return;
  
	if (view->selection_mode != GTK_SELECTION_MULTIPLE)
		cmpack_chart_view_unselect_all_internal(view);
	item->selected = TRUE;
	g_signal_emit (view, chart_view_signals[SELECTION_CHANGED], 0);
	invalidate(view);
}

static void cmpack_chart_view_unselect_item (CmpackChartView *view, CmpackChartViewItem *item)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (item != NULL);

	if (!item->selected || view->selection_mode == GTK_SELECTION_BROWSE)
		return;
  
	item->selected = FALSE;
	g_signal_emit (view, chart_view_signals[SELECTION_CHANGED], 0);
	invalidate(view);
}

/* VOID:OBJECT,OBJECT */
static void cmpack_chart_view_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
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

static void clear_data(CmpackChartView *view)
{
	gint i;

	for (i=0; i<view->item_count; i++) 
		g_free(view->items[i].tag_text);
	g_free(view->items);
	view->items = NULL;
	view->item_count = view->item_capacity = 0;
	view->focused_item = -1;
	view->last_single_clicked = -1;
}

static void update_limits(CmpackChartView *view)
{
	int w1, h1, w2, h2, width, height;
	if (view->model) {
		w1 = cmpack_chart_data_width(view->model);
		h1 = cmpack_chart_data_height(view->model);
	} else 
		w1 = h1 = 0;
	if (view->image) {
		w2 = cmpack_image_data_width(view->image);
		h2 = cmpack_image_data_height(view->image);
	} else
		w2 = h2 = 0;
	width = MAX(w1, w2);
	height = MAX(h1, h2);
	if (view->x.Max!=width && view->y.Max!=height) {
		view->x.Max = width;
		if (!view->x.Reverse) {
			view->x.ProjMin = 0;
			view->x.ProjMax = width;
		} else {
			view->x.ProjMin = -width;
			view->x.ProjMax = 0;
		}
		view->y.Max = height;
		if (!view->y.Reverse) {
			view->y.ProjMin = -height;
			view->y.ProjMax = 0;
		} else {
			view->y.ProjMin = 0;
			view->y.ProjMax = height;
		}
		update_rectangles(view);
		if (view->auto_zoom) 
			fit_to_window(view);
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
	}
}

static void update_item(CmpackChartView *view, CmpackChartViewItem *item, CmpackChartData *model, gint row)
{
	const CmpackChartItem *obj;
	const char *tag;

	item->visible = FALSE;
	
	obj = cmpack_chart_data_get_item(model, row);
	if (obj) {
		item->visible  = TRUE;
		item->xproj  = x_to_proj(view, obj->x);
		item->yproj  = y_to_proj(view, obj->y);
		item->size   = MAX(0.0, obj->d);
		item->color  = obj->color;
		item->filled = !obj->outline;
		item->topmost = obj->topmost;
		item->enabled = !obj->disabled;
		tag = cmpack_chart_data_get_tag(model, row);
		if (tag) {
			g_free(item->tag_text);
			item->tag_text = g_strdup(tag);
		} else {
			g_free(item->tag_text);
			item->tag_text = NULL;
		}
	}
}

static gint find_item(CmpackChartView *view, gint view_x, gint view_y)
{
	gint i, x, y, r2, r2min, index = -1;

	r2min = CLICK_TOLERANCE * CLICK_TOLERANCE;
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible) {
			x = RoundToInt(xproj_to_view(view, item->xproj));
			y = RoundToInt(yproj_to_view(view, item->yproj));
			r2 = (x-view_x)*(x-view_x) + (y-view_y)*(y-view_y);
			if (r2 < r2min) {
				index = i;
				r2min = r2;
			}
		}
	}
	return index;
}

static void update_data(CmpackChartView *view)
{
	gint i;

	if (view->model && view->items) {
		for (i=0; i<view->item_count; i++)
			update_item(view, view->items+i, view->model, i);
	}
}

static void rebuild_data(CmpackChartView *view)
{
	gint count;

	clear_data(view);
	if (view->model) {
		count = cmpack_chart_data_count(view->model);
		if (view->model && count>0) {
			view->item_count = view->item_capacity = count;
			view->items = (CmpackChartViewItem*)g_malloc0(view->item_capacity*sizeof(CmpackChartViewItem));
			update_data(view);
		}
	}
}

static void image_changed(CmpackImageData *image, gpointer *data)
{
	CmpackChartView *view = CMPACK_CHART_VIEW(data);

	invalidate(view);
}

static void invalidate(CmpackChartView *view)
{
	view->dirty = TRUE;
	gtk_widget_queue_draw(GTK_WIDGET(view));
}

/* -------------------------------   OVERLAYS -------------------------------------- */

static CmpackChartLayer *create_layer(gint id)
{
	CmpackChartLayer *layer = (CmpackChartLayer*)g_malloc0(sizeof(CmpackChartLayer));
	layer->handle = id;
	layer->visible = TRUE;
	return layer;
}

static void add_layer(CmpackChartView *view, CmpackChartLayer *layer)
{
	view->layers = g_slist_prepend(view->layers, layer);
	g_hash_table_insert(view->ht_layers, &layer->handle, layer);
}

static CmpackChartLayer *find_layer(CmpackChartView *view, gint id)
{
	return (CmpackChartLayer*)g_hash_table_lookup(view->ht_layers, &id);
}

static void destroy_layer(CmpackChartView *view, CmpackChartLayer *layer)
{
	GSList *ptr;

	for (ptr=layer->entities; ptr!=NULL; ptr=ptr->next) {
		g_hash_table_remove(view->ht_entities, (CmpackChartEntity*)ptr->data);
		destroy_entity((CmpackChartEntity*)ptr->data);
	}
	g_slist_free(layer->entities);
	g_free(layer);
}

static CmpackChartEntity *create_entity(gint id, CmpackChartShape shape)
{
	CmpackChartEntity *entity = (CmpackChartEntity*)g_malloc0(sizeof(CmpackChartEntity));
	entity->handle = id;
	entity->shape = shape;
	return entity;
}

static void add_entity(CmpackChartView *view, CmpackChartLayer *layer, CmpackChartEntity *entity)
{
	layer->entities = g_slist_prepend(layer->entities, entity);
	g_hash_table_insert(view->ht_entities, &entity->handle, entity);
	entity->layer = layer->handle;
}

static CmpackChartEntity *find_entity(CmpackChartView *view, gint id)
{
	return (CmpackChartEntity*)g_hash_table_lookup(view->ht_entities, &id);
}

static void destroy_entity(CmpackChartEntity *entity)
{
	g_free(entity);
}

static void clear_layers(CmpackChartView *view)
{
	GSList *ptr;

	for (ptr=view->layers; ptr!=NULL; ptr=ptr->next) {
		CmpackChartLayer *layer = (CmpackChartLayer*)ptr->data;
		g_hash_table_remove(view->ht_layers, &layer->handle);
		destroy_layer(view, layer);
	}
	g_slist_free(view->layers);
	view->layers = NULL;
}

static const GdkColor *entity_fg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartEntity *entity)
{
	switch (entity->color)
	{
	case CMPACK_COLOR_RED:
		return &view->int_colors[CHART_COLOR_RED];
	case CMPACK_COLOR_GREEN:
		return &view->int_colors[CHART_COLOR_GREEN];
	case CMPACK_COLOR_BLUE:
		return &view->int_colors[CHART_COLOR_BLUE];
	case CMPACK_COLOR_YELLOW:
		return &view->int_colors[CHART_COLOR_YELLOW];
	case CMPACK_COLOR_GRAY:
		return &widget->style->dark[widget->state];
	default:
		if (view->negative)
			return &widget->style->fg[widget->state];
		else
			return &widget->style->white;
	}
}

static const GdkColor *entity_bg_color(CmpackChartView *view, GtkWidget *widget, const CmpackChartEntity *entity)
{
	if (view->negative)
		return &widget->style->base[widget->state];
	else
		return &widget->style->black;
}

static void paint_layers(CmpackChartView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint x1, y1, x2, y2;
	GSList *lptr, *eptr;

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_set_clip_rectangle(gc, &view->canvas_rc);
	gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	for (lptr=view->layers; lptr!=NULL; lptr=lptr->next) {
		CmpackChartLayer *layer = (CmpackChartLayer*)lptr->data;
		if (layer->visible && layer->entities) {
			for (eptr=layer->entities; eptr!=NULL; eptr=eptr->next) {
				CmpackChartEntity *item = (CmpackChartEntity*)eptr->data;
				if (!view->x.Reverse) {
					x1 = RoundToInt(x_to_view(view, item->xphys));
					x2 = RoundToInt(x_to_view(view, item->xphys + item->width));
				} else {
					x1 = RoundToInt(x_to_view(view, item->xphys + item->width));
					x2 = RoundToInt(x_to_view(view, item->xphys));
				}
				if (!view->y.Reverse) {
					y1 = RoundToInt(y_to_view(view, item->yphys));
					y2 = RoundToInt(y_to_view(view, item->yphys + item->height));
				} else {
					y1 = RoundToInt(y_to_view(view, item->yphys + item->height));
					y2 = RoundToInt(y_to_view(view, item->yphys));
				}
				if (x2 >= view->canvas_rc.x && x1 <= view->canvas_rc.x + view->canvas_rc.width &&
					y2 >= view->canvas_rc.y && y1 <= view->canvas_rc.y + view->canvas_rc.height) {
						switch (item->shape)
						{
						case CHART_SHAPE_CIRCLE:
							if (!item->filled) {
								gdk_gc_set_foreground(gc, entity_fg_color(view, widget, item));
								gdk_draw_arc(drawable, gc, FALSE, x1, y1, x2-x1, y2-y1, 0, 64*360);
							} else {
								gdk_gc_set_foreground(gc, entity_fg_color(view, widget, item));
								gdk_draw_arc(drawable, gc, TRUE, x1, y1, x2-x1, y2-y1, 0, 64*360);
								gdk_gc_set_foreground(gc, entity_bg_color(view, widget, item));
								gdk_draw_arc(drawable, gc, FALSE, x1, y1, x2-x1, y2-y1, 0, 64*360);
							}
							break;

						default:
							break;
						}
				}
			}
		}
	}
	gdk_gc_set_clip_rectangle(gc, NULL);
	gdk_gc_unref(gc);
}

/* -----------------------------   Public API ------------------------------------ */

/* Create a new chart with default model */
GtkWidget *cmpack_chart_view_new (void)
{
	return cmpack_chart_view_new_with_model(NULL);
}

/* Create a new chart with specified model */
GtkWidget *cmpack_chart_view_new_with_model(CmpackChartData *model)
{
  return (GtkWidget*)g_object_new(CMPACK_TYPE_CHART_VIEW, "model", model, NULL);
}

/* Set selection mode */
void cmpack_chart_view_set_selection_mode(CmpackChartView *view, GtkSelectionMode mode)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	if (mode == view->selection_mode)
		return;
  
	if (mode == GTK_SELECTION_NONE || view->selection_mode == GTK_SELECTION_MULTIPLE)
		cmpack_chart_view_unselect_all (view);
  
	view->selection_mode = mode;
	if (!view->selection_mode && !view->activation_mode && view->focused_item>=0) {
		invalidate_cursor(view);
		view->focused_item = -1;
	}

	g_object_notify(G_OBJECT (view), "selection-mode");
}

/* Get selection mode */
GtkSelectionMode cmpack_chart_view_get_selection_mode (CmpackChartView *view)
{
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), GTK_SELECTION_SINGLE);
	return view->selection_mode;
}

/* Set activation mode */
void cmpack_chart_view_set_activation_mode(CmpackChartView *view, CmpackActivationMode mode)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	if (mode == view->activation_mode)
		return;

	view->activation_mode = mode;
	if (!view->selection_mode && !view->activation_mode && view->focused_item>=0) {
		invalidate_cursor(view);
		view->focused_item = -1;
	}
}

/* Enable mouse control */
void cmpack_chart_view_set_mouse_control(CmpackChartView *view, gboolean enabled)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	view->mouse_ctrl = enabled;
}

/* Set data model */
void cmpack_chart_view_set_model(CmpackChartView *view, CmpackChartData *model)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (model == NULL || CMPACK_IS_CHART_DATA(model));
  
	if (view->model == model)
		return;

	if (view->model) {
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)object_inserted, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)object_updated, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)object_deleted, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)dim_changed, view);
		g_signal_handlers_disconnect_by_func (view->model, (gpointer)data_cleared, view);
		g_object_unref(view->model);
	}
	view->model = model;
	if (view->model) {
		g_object_ref(view->model);
		g_signal_connect(view->model, "object-inserted", G_CALLBACK(object_inserted), view);
		g_signal_connect(view->model, "object-updated", G_CALLBACK(object_updated), view);
		g_signal_connect(view->model, "object-deleted", G_CALLBACK(object_deleted), view);
		g_signal_connect(view->model, "dim-changed", G_CALLBACK(dim_changed), view);
		g_signal_connect(view->model, "data-cleared", G_CALLBACK(data_cleared), view);
	}
	update_limits(view);
	rebuild_data(view);

	if (GTK_WIDGET_REALIZED(view)) {
		g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		gtk_widget_queue_resize (GTK_WIDGET(view));
	}

	g_object_notify(G_OBJECT(view), "model");  
}

/* Set data model */
void cmpack_chart_view_set_image(CmpackChartView *view, CmpackImageData *image)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (image == NULL || CMPACK_IS_IMAGE_DATA(image));
  
	if (view->image == image)
		return;

	if (view->image) {
		g_signal_handlers_disconnect_by_func(view->image, (gpointer)image_changed, view);
		g_object_unref(view->image);
	}
	view->image = image;
	if (view->image) {
		g_object_ref(view->image);
		g_signal_connect(view->image, "data-changed", G_CALLBACK(image_changed), view);
	}
	update_limits(view);
	invalidate(view);

	if (GTK_WIDGET_REALIZED(view)) {
		g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		gtk_widget_queue_resize(GTK_WIDGET(view));
	}

	g_object_notify(G_OBJECT(view), "image");  
}

/* Get data model */
CmpackChartData *cmpack_chart_view_get_chart_data(CmpackChartView *view)
{
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), NULL);
	
	return view->model;
}

/* Get image data model */
CmpackImageData* cmpack_chart_view_get_image(CmpackChartView *view)
{
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), NULL);
	
	return view->image;
}

/* Add specified item to selection */
void cmpack_chart_view_select(CmpackChartView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	cmpack_chart_view_select_item(view, view->items + row);
}

/* Remove item from selection */
void cmpack_chart_view_unselect(CmpackChartView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);
	
	cmpack_chart_view_unselect_item(view, view->items + row);
}

/* 
 * cmpack_chart_view_get_selected_items:
 * @view: A #CmpackChartView.
 *
 * Creates a list of paths of all selected items. Additionally, if you are
 * planning on modifying the model after calling this function, you may
 * want to convert the returned list into a list of #GtkTreeRowReference<!-- -->s.
 * To do this, you can use gtk_tree_row_reference_new().
 *
 * To free the return value, use:
 * |[
 * g_list_foreach (list, gtk_tree_path_free, NULL);
 * g_list_free (list);
 * ]|
 *
 * Return value: A #GList containing a #GtkTreePath for each selected row.
 *
 * Since: 2.6
 **/
GList *cmpack_chart_view_get_selected_rows(CmpackChartView *view)
{
	gint i;
	GList *selected = NULL;
  
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), NULL);
  
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && item->enabled && item->selected) 
			selected = g_list_prepend (selected, (gpointer)(intptr_t)i);
	}
	return selected;
}

gint cmpack_chart_view_get_selected_count(CmpackChartView *view)
{
	gint i, count = 0;
  
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), 0);
  
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && item->enabled && item->selected) 
			count++;
	}
	return count;
}

gint cmpack_chart_view_get_selected(CmpackChartView *view)
{
	gint i;
  
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), -1);
  
	for (i=0; i<view->item_count; i++) {
		CmpackChartViewItem *item = view->items + i;
		if (item->visible && item->enabled && item->selected) 
			return i;
	}
	return -1;
}

gint cmpack_chart_view_get_focused(CmpackChartView *view)
{
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), -1);

	return view->focused_item;
}

/**
 * cmpack_chart_view_select_all:
 * @view: A #CmpackChartView.
 * 
 * Selects all the icons. @view must has its selection mode set
 * to #GTK_SELECTION_MULTIPLE.
 *
 * Since: 2.6
 **/
void cmpack_chart_view_select_all (CmpackChartView *view)
{
  gint i;
  gboolean dirty = FALSE;
  
  g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

  if (view->selection_mode != GTK_SELECTION_MULTIPLE)
    return;

  for (i=0; i<view->item_count; i++)
    {
      CmpackChartViewItem *item = view->items + i;
      if (!item->selected)
	{
	  dirty = TRUE;
	  item->selected = TRUE;
	  cmpack_chart_view_queue_draw_item (view, item);
	}
    }

  if (dirty)
    g_signal_emit (view, chart_view_signals[SELECTION_CHANGED], 0);
}

/**
 * cmpack_chart_view_unselect_all:
 * @view: A #CmpackChartView.
 * 
 * Unselects all the icons.
 *
 * Since: 2.6
 **/
void
cmpack_chart_view_unselect_all (CmpackChartView *view)
{
  gboolean dirty = FALSE;
  
  g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

  if (view->selection_mode == GTK_SELECTION_BROWSE)
    return;

  dirty = cmpack_chart_view_unselect_all_internal (view);

  if (dirty)
    g_signal_emit (view, chart_view_signals[SELECTION_CHANGED], 0);
}

/**
 * cmpack_chart_view_path_is_selected:
 * @view: A #CmpackChartView.
 * @path: A #GtkTreePath to check selection on.
 * 
 * Returns %TRUE if the icon pointed to by @path is currently
 * selected. If @path does not point to a valid location, %FALSE is returned.
 * 
 * Return value: %TRUE if @path is selected.
 *
 * Since: 2.6
 **/
gboolean cmpack_chart_view_is_selected (CmpackChartView *view, gint row)
{
  g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), FALSE);
  g_return_val_if_fail (view->model != NULL, FALSE);
  g_return_val_if_fail (row>=0 && row<view->item_count, FALSE);

  return view->items[row].selected;
}

/**
 * cmpack_chart_view_item_activated:
 * @view: A #CmpackChartView
 * @path: The #GtkTreePath to be activated
 * 
 * Activates the item determined by @path.
 *
 * Since: 2.6
 **/
void cmpack_chart_view_item_activate(CmpackChartView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	g_signal_emit (view, chart_view_signals[ITEM_ACTIVATED], 0, row);
}

/* Set mapping parameters */
void cmpack_chart_view_set_orientation(CmpackChartView *view, CmpackOrientation orientation)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	gboolean zoom_changed = false;

	view->x.Reverse = 0; //(reverse_x!=0);
	if (!view->x.Reverse) {
		view->x.ProjMin = 0;
		view->x.ProjMax = view->x.Max;
	} else {
		view->x.ProjMin = -view->x.Max;
		view->x.ProjMax = 0;
	}
	view->y.Reverse = orientation & CMPACK_ROWS_UPWARDS;
	if (!view->y.Reverse) {
		view->y.ProjMin = -view->y.Max;
		view->y.ProjMax = 0;
	} else {
		view->y.ProjMin = 0;
		view->y.ProjMax = view->y.Max;
	}
	update_data(view);
	update_rectangles(view);
	if (view->auto_zoom) 
		zoom_changed = fit_to_window(view);
	update_pxlsize(view);
	restrict_x_to_limits(view);
	restrict_y_to_limits(view);
	update_hsb(view);
	update_vsb(view);
	invalidate(view);

	if (zoom_changed)
		g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
	if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
		g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
}

/* Enable/disable auto zoom mode */
void cmpack_chart_view_set_auto_zoom(CmpackChartView *view, gboolean enable)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	gboolean zoom_changed = false;

	if (view->auto_zoom!=enable) {
		view->auto_zoom = enable;
		if (view->auto_zoom) 
			zoom_changed = fit_to_window(view);
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (zoom_changed)
			g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
	}
}

gboolean cmpack_chart_view_get_auto_zoom(CmpackChartView *view)
{
	g_return_val_if_fail (CMPACK_IS_CHART_VIEW (view), FALSE);

	return view->auto_zoom;
}

/* Set labels */
void cmpack_chart_view_show_scales(CmpackChartView *view, gboolean show)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	gboolean zoom_changed = false;

	if (view->show_labels!=show) {
		view->show_labels = show;
		update_rectangles(view);
		if (view->auto_zoom)
			zoom_changed = fit_to_window(view);
		update_pxlsize(view);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (zoom_changed)
			g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
	}
} 

void cmpack_chart_view_show_grid(CmpackChartView *view, gboolean show)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	if (view->show_grid != show) {
		view->show_grid = show;
		invalidate(view);
	}
}

void cmpack_chart_view_set_zoom(CmpackChartView *view, gdouble zoom)
{
	gdouble x, y;

	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	view->auto_zoom = FALSE;
	x = view_to_xproj(view, view->canvas_rc.x + view->canvas_rc.width/2);
	y = view_to_yproj(view, view->canvas_rc.y + view->canvas_rc.height/2);
	if (set_zoom(view, zoom)) {
		x -= (view->canvas_rc.x + view->canvas_rc.width/2) * view->pxl_size;
		y += (view->canvas_rc.y + view->canvas_rc.height/2) * view->pxl_size;
		set_offset(view, x, y);
		g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

gdouble cmpack_chart_view_get_zoom(CmpackChartView *view)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW (view), 0.0);

	return view->zoom_pos;
}

void cmpack_chart_view_get_zoom_limits(CmpackChartView *view, gdouble *min, gdouble *max)
{
	g_return_if_fail(CMPACK_IS_CHART_VIEW (view));

	if (min)
		*min = view->zoom_min;
	if (max)
		*max = view->zoom_max;
}

void cmpack_chart_view_set_magnification(CmpackChartView *view, gdouble mag)
{
	gdouble x, y;

	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));
	g_return_if_fail (mag > 0.0);

	view->auto_zoom = FALSE;
	x = view_to_xproj(view, view->canvas_rc.x + view->canvas_rc.width/2);
	y = view_to_yproj(view, view->canvas_rc.y + view->canvas_rc.height/2);
	if (set_zoom(view, log(mag) / log(view->zoom_base))) {
		x -= (view->canvas_rc.x + view->canvas_rc.width/2) * view->pxl_size;
		y += (view->canvas_rc.y + view->canvas_rc.height/2) * view->pxl_size;
		set_offset(view, x, y);
		g_signal_emit(view, chart_view_signals[ZOOM_CHANGED], 0);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

gdouble cmpack_chart_view_get_magnification(CmpackChartView *view)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW (view), 0.0);

	return 1.0 / view->pxl_size;
}

void cmpack_chart_view_set_offset(CmpackChartView *view, gdouble x, gdouble y)
{
	g_return_if_fail(CMPACK_IS_CHART_VIEW (view));

	if (set_offset(view, x_to_proj(view, x), y_to_proj(view, y))) {
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, chart_view_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

gboolean cmpack_chart_view_get_center(CmpackChartView *view, gdouble *x, gdouble *y)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW(view), FALSE);

	if (x)
		*x = proj_to_x(view, view->x.ProjPos);
	if (y)
		*y = proj_to_y(view, view->y.ProjPos);
	return TRUE;
}

gboolean cmpack_chart_view_mouse_pos(CmpackChartView *view, gint *x, gint *y)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW(view), FALSE);

	if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
		*x = (int)view_to_x(view, view->last_mouse_x);
		*y = (int)view_to_y(view, view->last_mouse_y);
		return (*x>=view->x.Min && *x<view->x.Max && *y>=view->y.Min && *y<view->y.Max);
	}
	return FALSE;
}

//
// Add an overlay layer over the chart 
//
gint cmpack_chart_view_add_layer(CmpackChartView *view)
{
	CmpackChartLayer *layer;

	g_return_val_if_fail(CMPACK_IS_CHART_VIEW(view), 0);

	layer = create_layer(view->next_layer);
	add_layer(view, layer);
	view->next_layer = (view->next_layer+1) & 0x7FFFFFFF;
	return layer->handle;
}

//
// Show and hide a layer
//
void cmpack_chart_view_show_layer(CmpackChartView *view, gint index, gboolean show)
{
	CmpackChartLayer *layer;

	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	layer = find_layer(view, index);
	if (!layer) {
		g_error("Layer doesn't exist: %ud", index);
		return;
	}

	layer->visible = show;
	invalidate(view);
}

//
// Delete a layer
//
void cmpack_chart_view_delete_layer(CmpackChartView *view, gint index)
{
	CmpackChartLayer *layer;

	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	layer = find_layer(view, index);
	if (!layer) {
		g_error("Layer doesn't exist: %ud", index);
		return;
	}

	view->layers = g_slist_remove(view->layers, layer);
	g_hash_table_remove(view->ht_layers, &layer->handle);
	destroy_layer(view, layer);
	invalidate(view);
}

//
// Add a circle into a layer
//
gint cmpack_chart_view_add_circle(CmpackChartView *view, gint layer, 
	gdouble x, gdouble y, gdouble w, gdouble h, CmpackColor color, gboolean filled)
{
	CmpackChartLayer *l;
	CmpackChartEntity *e;

	g_return_val_if_fail(CMPACK_IS_CHART_VIEW(view), 0);

	l = find_layer(view, layer);
	if (!l) {
		g_error("Layer doesn't exist: %ud", layer);
		return 0;
	}

	e = create_entity(view->next_entity, CHART_SHAPE_CIRCLE);
	e->xphys = x;
	e->yphys = y;
	e->color = color;
	e->filled = filled;
	e->width = w;
	e->height = h;
	add_entity(view, l, e);
	view->next_entity = (view->next_entity+1) & 0x7FFFFFFF;
	invalidate(view);

	return e->handle;
}

//
// Move an object
//
void cmpack_chart_view_move_object(CmpackChartView *view, gint object, 
	gdouble x, gdouble y)
{
	CmpackChartEntity *e;

	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	e = find_entity(view, object);
	if (!e) {
		g_error("Entity doesn't exist: %ud", object);
		return;
	}

	if (x!=e->xphys || y!=e->yphys) {
		e->xphys = x;
		e->yphys = y;
		invalidate(view);
	}
}

//
// Change a radius of a circle
//
void cmpack_chart_view_resize_object(CmpackChartView *view, gint object, gdouble w, gdouble h)
{
	CmpackChartEntity *e;

	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	e = find_entity(view, object);
	if (!e) {
		g_error("Entity doesn't exist: %ud", object);
		return;
	}
	
	if (e->width!=w || e->height!=h) {
		e->width = w;
		e->height = h;
		invalidate(view);
	}
}

//
// Delete an object 
//
void cmpack_chart_view_delete_object(CmpackChartView *view, gint object)
{
	CmpackChartEntity *entity;
	CmpackChartLayer *layer;

	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	entity = find_entity(view, object);
	if (!entity) {
		g_error("Entity doesn't exist: %ud", object);
		return;
	}

	layer = find_layer(view, entity->layer);
	if (layer)
		layer->entities = g_slist_remove(layer->entities, entity);
	g_hash_table_remove(view->ht_entities, &entity->handle);
	destroy_entity(entity);
	invalidate(view);
}

void cmpack_chart_view_set_negative(CmpackChartView *view, gboolean negative)
{
	g_return_if_fail (CMPACK_IS_CHART_VIEW (view));

	if (view->negative != negative) {
		view->negative = negative;
		invalidate(view);
	}
}

gboolean cmpack_chart_view_get_negative(CmpackChartView *view)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW (view), FALSE);

	return view->negative;
}

void cmpack_chart_view_profile_enable(CmpackChartView *view, gboolean enable)
{
	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	view->profile_enabled = enable;
}

void cmpack_chart_view_profile_clear(CmpackChartView *view)
{
	g_return_if_fail(CMPACK_IS_CHART_VIEW(view));

	if (view->profile_valid)
		invalidate_profile(view);
	view->profile_valid = FALSE;
}

gboolean cmpack_chart_view_profile_get_pos(CmpackChartView *view, 
	CmpackChartProfilePoint point, gint *x, gint *y)
{
	g_return_val_if_fail(CMPACK_IS_CHART_VIEW(view), FALSE);
	g_assert(point>=0 && point<CHART_PROFILE_NPOINTS);

	if (x)
		*x = (gint)proj_to_x(view, view->profile_x[point]);
	if (y)
		*y = (gint)proj_to_y(view, view->profile_y[point]);
	return view->profile_valid;
}
