/**************************************************************

cmpack_graph_view.cpp (C-Munipack project)
Widget which can draw a graph
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

$Id: cmpack_graph_view.cpp,v 1.15 2014/08/31 12:39:43 dmotl Exp $

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#include "cmpack_graph_view.h"

#define MARKER_SIZE		7

#define CROSS_BAR_SIZE	12

#define CLICK_TOLERANCE 7

#define CURSOR_SIZE		11

enum tDateUnit {
	YEARS, MONTHS, DAYS, SECONDS
};
enum tDateFormat {
	FORMAT_HMS, FORMAT_HM, FORMAT_YMD, FORMAT_MY, FORMAT_Y, FORMAT_YMDH
};

// This array enumerates allowed step of the scale in TIME mode
#define NumTSteps		17
static const struct {
	gint step;
	tDateFormat fmt;
} TimeSteps[NumTSteps] = {
	{ 1, FORMAT_HMS }, { 2, FORMAT_HMS }, { 5, FORMAT_HMS }, 
	{ 10, FORMAT_HMS }, { 15, FORMAT_HMS }, { 30, FORMAT_HMS }, 
	{ 60, FORMAT_HM }, { 120, FORMAT_HM }, { 300, FORMAT_HM }, 
	{ 600, FORMAT_HM }, { 900, FORMAT_HM }, { 1800, FORMAT_HM }, 
	{ 3600, FORMAT_HM }, { 7200, FORMAT_HM }, { 14400, FORMAT_HM }, 
	{ 21600, FORMAT_HM }, { 43200, FORMAT_HM }
};

#define NumDSteps		22
static const struct {
	gint step;
	tDateUnit unit;
	tDateFormat fmt;
} DateSteps[NumDSteps] = {
	{ 14400, SECONDS, FORMAT_YMDH }, { 21600, SECONDS, FORMAT_YMDH }, { 43200, SECONDS, FORMAT_YMDH },
	{ 1, DAYS, FORMAT_YMD }, { 2, DAYS, FORMAT_YMD }, { 5, DAYS, FORMAT_YMD }, 
	{ 10, DAYS, FORMAT_YMD }, { 20, DAYS, FORMAT_YMD }, { 1, MONTHS, FORMAT_MY }, 
	{ 2, MONTHS, FORMAT_MY }, { 3, MONTHS, FORMAT_MY }, { 6, MONTHS, FORMAT_MY }, 
	{ 1, YEARS, FORMAT_Y }, { 2, YEARS, FORMAT_Y }, { 5, YEARS, FORMAT_Y }, 
	{ 10, YEARS, FORMAT_Y }, { 20, YEARS, FORMAT_Y }, { 50, YEARS, FORMAT_Y }, 
	{ 100, YEARS, FORMAT_Y }, { 200, YEARS, FORMAT_Y }, { 500, YEARS, FORMAT_Y }, 
	{ 1000, YEARS, FORMAT_Y }
};

struct _CmpackGraphViewItem
{
	gboolean    visible;			// Item is visible
	gboolean	enabled;			// Can be selected or activated
	gboolean	topmost;			// Item is topmost
	gboolean	selected;			// Is this point selected?
	gboolean	selected_before_rubberbanding;	// Has this point been selected before rubberbanding?
	gdouble		xproj, yproj, error;	// Position and error in projection units
	gchar		*tag_text;			// Tag (displayed text)
	CmpackColor	color;				// Color
}; 

/* Signals */
enum {
	ITEM_ACTIVATED,
	SELECTION_CHANGED,
	MOUSE_MOVED,
	CURSOR_MOVED,
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

/* GObject vfuncs */
static void             cmpack_graph_view_set_property              (GObject            *object,
								 guint               prop_id,
								 const GValue       *value,
								 GParamSpec         *pspec);
static void             cmpack_graph_view_get_property              (GObject            *object,
								 guint               prop_id,
								 GValue             *value,
								 GParamSpec         *pspec);
static void             cmpack_graph_view_finalize                  (GObject          *object); 

/* GtkObject vfuncs */
static void             cmpack_graph_view_destroy                   (GtkObject          *object);
/* GtkWidget vfuncs */
static void             cmpack_graph_view_realize                   (GtkWidget          *widget);
static void             cmpack_graph_view_unrealize                 (GtkWidget          *widget);
static void             cmpack_graph_view_style_set                 (GtkWidget        *widget,
						                 GtkStyle         *previous_style);
static void             cmpack_graph_view_state_changed             (GtkWidget        *widget,
			                                         GtkStateType      previous_state);
static void             cmpack_graph_view_size_allocate             (GtkWidget          *widget,
								 GtkAllocation      *allocation);
static gboolean         cmpack_graph_view_expose                    (GtkWidget          *widget,
								 GdkEventExpose     *expose);
static gboolean         cmpack_graph_view_motion                    (GtkWidget          *widget,
								 GdkEventMotion     *event);
static gboolean         cmpack_graph_view_leave                     (GtkWidget          *widget,
								 GdkEventCrossing   *event);
static gboolean         cmpack_graph_view_button_press              (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_graph_view_button_release            (GtkWidget          *widget,
								 GdkEventButton     *event);
static gboolean         cmpack_graph_view_key_press                 (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_graph_view_key_release               (GtkWidget          *widget,
								 GdkEventKey        *event);
static gboolean         cmpack_graph_view_scroll		     		(GtkWidget      *widget,
								 GdkEventScroll     *event);

/* CmpackGraphView vfuncs */
static void             cmpack_graph_view_set_adjustments           (CmpackGraphView        *view,
								 GtkAdjustment      *hadj,
								 GtkAdjustment      *vadj);

static GdkCursorType	cmpack_graph_choose_mouse_cursor			(CmpackGraphView *view, 
									gint x, 
									gint y);

 /* Internal functions */
static void             cmpack_graph_view_create_buffer		(CmpackGraphView            *view); 
//static void             cmpack_graph_view_create_marker		(CmpackGraphView            *view); 

static void				cmpack_graph_view_paint_buffer				(CmpackGraphView            *view); 
//static void				cmpack_graph_view_paint_marker				(CmpackGraphView            *view); 

static void                 cmpack_graph_view_adjustment_changed             (GtkAdjustment          *adjustment,
									  CmpackGraphView            *view);
static void                 cmpack_graph_view_queue_draw_item                (CmpackGraphView            *view,
									  CmpackGraphViewItem        *item);
static void                 cmpack_graph_view_enter_mouse_mode            (CmpackGraphView            *view,
									  gint                    x,
									  gint                    y,
									  CmpackGraphMouseMode    mode,
									  gint                    param = -1);
static void                 cmpack_graph_view_leave_mouse_mode             (CmpackGraphView            *view);
static gboolean             cmpack_graph_view_unselect_all_internal          (CmpackGraphView            *view);
static void                 cmpack_graph_view_select_item                    (CmpackGraphView            *view,
									  CmpackGraphViewItem        *item);
static void                 cmpack_graph_view_unselect_item                  (CmpackGraphView            *view,
									  CmpackGraphViewItem        *item);
static void cmpack_graph_view_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data);
static void cmpack_graph_view_marshal_VOID__INT_INT(GClosure     *closure,
                                  GValue       *return_value G_GNUC_UNUSED,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                  gpointer      marshal_data);

static void clear_data(CmpackGraphView *view);
static void rebuild_data(CmpackGraphView *view);
static void update_data(CmpackGraphView *view);
static void invalidate(CmpackGraphView *view);
static void invalidate_cross_bar(CmpackGraphView *view);
static void invalidate_x_cursor(CmpackGraphView *view, CmpackGraphCursor *item, gint x1, gint x2);
static void invalidate_y_cursor(CmpackGraphView *view, CmpackGraphCursor *item, gint y1, gint y2);
static void update_item(CmpackGraphView *view, CmpackGraphViewItem *item, CmpackGraphData *model, gint row);
static gint find_item(CmpackGraphView *view, gint view_x, gint view_y);

static gint RoundToInt(gdouble x);
static gdouble LimitDbl(gdouble val, gdouble min, gdouble max);
static gint LimitInt(gint val, gint min, gint max);

static void set_rgb(GdkColor *color, gdouble red, gdouble green, gdouble blue);
static const GdkColor *item_fg_color(CmpackGraphView *view, GtkWidget *widget, const CmpackGraphViewItem *item);

static gdouble xproj_to_view(CmpackGraphView *view, gdouble x);
static gdouble view_to_xproj(CmpackGraphView *view, gdouble u);
static gdouble x_to_view(CmpackGraphView *view, gdouble x);
static gdouble view_to_x(CmpackGraphView *view, gdouble u);
static gdouble yproj_to_view(CmpackGraphView *view, gdouble y);
static gdouble view_to_yproj(CmpackGraphView *view, gdouble v);
static gdouble y_to_view(CmpackGraphView *view, gdouble y);
static gdouble view_to_y(CmpackGraphView *view, gdouble v);

static gboolean set_x_axis(CmpackGraphView *view, gdouble zoom, gdouble center);
static gboolean set_y_axis(CmpackGraphView *view, gdouble zoom, gdouble center);
static void update_rectangles(CmpackGraphView *view);
static void update_x_pxlsize(CmpackGraphView *view);
static void restrict_x_to_limits(CmpackGraphView *view);
static void update_y_pxlsize(CmpackGraphView *view);
static void restrict_y_to_limits(CmpackGraphView *view);

static void compute_x_grid_step_numeric(CmpackGraphView *view, gdouble min, gdouble max, gdouble *prec, gint *step);
static void compute_x_grid_step_datetime(CmpackGraphView *view, gdouble min, gdouble max, tDateUnit *unit, gint *step, tDateFormat *fmt);
static void compute_y_grid_step(CmpackGraphView *view, gdouble min, gdouble max, gdouble *prec, gint *step);

static void format_numeric(gchar *buf, gdouble value, CmpackGraphFormat format, gint prec);
static void format_datetime(gchar *buf, const struct tm *tm, tDateFormat fmt);
static gint compute_scale_labels_width(GtkWidget *widget, CmpackGraphViewAxis *axis, int prec);
static gint compute_scale_labels_height(GtkWidget *widget, CmpackGraphViewAxis *axis);
static gint compute_cursor_labels_width(GtkWidget *widget, CmpackGraphCursor *list, int count);
static gint compute_cursor_labels_height(GtkWidget *widget, CmpackGraphCursor *list, int count);

static void draw_text(GtkWidget *widget, GdkDrawable *drawable, GdkGC *gc, 
	gint x, gint y, const char *buf, gdouble halign, gdouble valign);
static void paint_data(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_x_scale(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_y_scale(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_x_grid(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable);
static void paint_y_grid(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable);

static void update_hsb(CmpackGraphView *view);
static void update_vsb(CmpackGraphView *view);

static gboolean jdtime(gdouble jd, struct tm *tm);
static gdouble timejd(const struct tm *tm);
static void ceildate(struct tm *tm, gint step, tDateUnit unit);

static guint graph_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(CmpackGraphView, cmpack_graph_view, GTK_TYPE_WIDGET)

/* Class initialization */
static void cmpack_graph_view_class_init(CmpackGraphViewClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	gobject_class->set_property = cmpack_graph_view_set_property;
	gobject_class->get_property = cmpack_graph_view_get_property;
	gobject_class->finalize = cmpack_graph_view_finalize;

	object_class->destroy = cmpack_graph_view_destroy;

	widget_class->realize = cmpack_graph_view_realize;
	widget_class->unrealize = cmpack_graph_view_unrealize;
	widget_class->style_set = cmpack_graph_view_style_set;
	widget_class->size_allocate = cmpack_graph_view_size_allocate;
	widget_class->expose_event = cmpack_graph_view_expose;
	widget_class->motion_notify_event = cmpack_graph_view_motion;
	widget_class->leave_notify_event = cmpack_graph_view_leave;
	widget_class->button_press_event = cmpack_graph_view_button_press;
	widget_class->button_release_event = cmpack_graph_view_button_release;
	widget_class->key_press_event = cmpack_graph_view_key_press;
	widget_class->key_release_event = cmpack_graph_view_key_release;
	widget_class->state_changed = cmpack_graph_view_state_changed;
	widget_class->scroll_event = cmpack_graph_view_scroll;

	klass->set_scroll_adjustments = cmpack_graph_view_set_adjustments;
  
	/* Properties */
	g_object_class_install_property (gobject_class, PROP_SELECTION_MODE, 
		g_param_spec_enum("selection-mode", "Selection mode", "The selection mode",
			GTK_TYPE_SELECTION_MODE, GTK_SELECTION_SINGLE, 
			(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property (gobject_class, PROP_MODEL,
		g_param_spec_object("model", "Graph Data Model", "The model for the graph view",
			CMPACK_TYPE_GRAPH_DATA, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE))); 

	/* Signals */
	widget_class->set_scroll_adjustments_signal = 
		g_signal_new ("set-scroll-adjustments", G_OBJECT_CLASS_TYPE (gobject_class),
			GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
			G_STRUCT_OFFSET (CmpackGraphViewClass, set_scroll_adjustments),
			NULL, NULL, cmpack_graph_view_marshal_VOID__OBJECT_OBJECT,
			G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT); 

	graph_view_signals[ITEM_ACTIVATED] =
		g_signal_new ("item-activated", G_TYPE_FROM_CLASS (gobject_class),
			G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CmpackGraphViewClass, item_activated),
			NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, GTK_TYPE_INT);

	graph_view_signals[SELECTION_CHANGED] =
		g_signal_new ("selection-changed", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackGraphViewClass, selection_changed),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	graph_view_signals[MOUSE_MOVED] =
		g_signal_new ("mouse-moved", G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CmpackGraphViewClass, mouse_moved),
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	graph_view_signals[CURSOR_MOVED] =
		g_signal_new ("cursor-moved", G_TYPE_FROM_CLASS (gobject_class),
			G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (CmpackGraphViewClass, cursor_moved),
			NULL, NULL, cmpack_graph_view_marshal_VOID__INT_INT, G_TYPE_NONE, 2, GTK_TYPE_INT, GTK_TYPE_INT);
}

/* Graph view initialization */
static void cmpack_graph_view_init(CmpackGraphView *view)
{
	view->zoom_base = pow(100.0, 1.0/100.0);
	view->last_mouse_x = view->last_mouse_y = -1;
	view->last_single_clicked = -1;
	view->focused_item = -1;
	view->error_bars = TRUE;
	view->int_colors = (GdkColor*)g_malloc(GRAPH_N_COLORS*sizeof(GdkColor));
	cmpack_graph_view_set_adjustments(view, NULL, NULL);
}

static void cmpack_graph_view_destroy(GtkObject *object)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (object);

	/* Disconnect the model */
	cmpack_graph_view_set_model(view, NULL);
  
	/* Disconnect the adjustments */
	if (view->hadjustment != NULL) {
		g_object_unref (view->hadjustment);
		view->hadjustment = NULL;
	}
	if (view->vadjustment != NULL) {
		g_object_unref (view->vadjustment);
		view->vadjustment = NULL;
	}
  
	GTK_OBJECT_CLASS(cmpack_graph_view_parent_class)->destroy(object);
}

static void cmpack_graph_view_finalize(GObject *object)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(object);

	/* Free buffered data */
	clear_data(view);

	/* Free pixmaps */
	if (view->offscreen_pixmap)
		g_object_unref(view->offscreen_pixmap);

	/* Free allocated memory */
	for (int i=0; i<view->x.CursorCount; i++)
		g_free(view->x.Cursor[i].caption);
	g_free(view->x.Cursor);
	for (int i=0; i<view->y.CursorCount; i++)
		g_free(view->y.Cursor[i].caption);
	g_free(view->y.Cursor);
	g_free(view->x.Name);
	g_free(view->y.Name);
	g_free(view->int_colors);

	G_OBJECT_CLASS(cmpack_graph_view_parent_class)->finalize(object);
} 

/* Set property */
static void cmpack_graph_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  CmpackGraphView *view = CMPACK_GRAPH_VIEW (object);

	switch (prop_id)
    {
    case PROP_SELECTION_MODE:
		cmpack_graph_view_set_selection_mode (view, (GtkSelectionMode)g_value_get_enum (value));
		break;
    case PROP_MODEL:
		cmpack_graph_view_set_model(view, (CmpackGraphData*)g_value_get_object (value));
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

/* Get property */
static void cmpack_graph_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (object);

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
static void cmpack_graph_view_realize (GtkWidget *widget)
{
	gboolean int_ok[GRAPH_N_COLORS];
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(widget);
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

	set_rgb(&view->int_colors[GRAPH_COLOR_SELECTED], 0.5, 0.5, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_ZOOMRECT], 1.0, 0.0, 0.0);
	set_rgb(&view->int_colors[GRAPH_COLOR_CURSOR], 0.0, 0.7, 0.8);
	set_rgb(&view->int_colors[GRAPH_COLOR_RED], 1, 0, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_GREEN], 0, 1, 0);
	set_rgb(&view->int_colors[GRAPH_COLOR_BLUE], 0, 0, 1);
	set_rgb(&view->int_colors[GRAPH_COLOR_YELLOW], 1, 1, 0);
	gdk_colormap_alloc_colors(gtk_widget_get_colormap(widget), view->int_colors, 
		GRAPH_N_COLORS, FALSE, TRUE, int_ok);

	cmpack_graph_view_create_buffer(view); 

	view->mouse_cursor = GDK_LAST_CURSOR;

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
static void cmpack_graph_view_unrealize(GtkWidget *widget)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(widget);

	gdk_colormap_free_colors(gtk_widget_get_colormap(widget), view->int_colors, GRAPH_N_COLORS);

	GTK_WIDGET_CLASS(cmpack_graph_view_parent_class)->unrealize (widget); 
}

/* Widget state changed */
static void cmpack_graph_view_state_changed(GtkWidget *widget, GtkStateType previous_state)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	view->dirty = TRUE;
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS (cmpack_graph_view_parent_class)->state_changed(widget, previous_state);
}

/* Widget style changed */
static void cmpack_graph_view_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	view->dirty = TRUE; 
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_set_background (widget->window, &widget->style->base[widget->state]);

	GTK_WIDGET_CLASS(cmpack_graph_view_parent_class)->style_set(widget, previous_style);
}

/* Widget size changed */
static void cmpack_graph_view_size_allocate(GtkWidget *widget, GtkAllocation  *allocation)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
				  allocation->x, allocation->y,
				  allocation->width, allocation->height);
		cmpack_graph_view_create_buffer(view); 
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		view->dirty = TRUE;
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

static void paint_rectangle(CmpackGraphView *view, GtkWidget *widget, GdkWindow *drawable, 
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

static void paint_cross_bar(CmpackGraphView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, int x, int y, int size)
{
	gint i, inner, outer;
	gboolean ok;
	GdkRectangle rect;

	inner = RoundToInt(size + 3.0 * pow(sin(view->cursor_phase/45.0*M_PI), 2));
	outer = RoundToInt(size + CROSS_BAR_SIZE + 3.0 * pow(sin(view->cursor_phase/45.0*M_PI), 2));

	rect.x = x - outer;
	rect.y = y - outer;
	rect.width = 2 * outer + 1;
	rect.height = 2 * outer + 1;

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
		gdouble sina = sin(view->cursor_phase/180.0*M_PI), cosa = cos(view->cursor_phase/180.0*M_PI);
		GdkGC *gc = gdk_gc_new(drawable);	
		gdk_gc_set_foreground(gc, &view->int_colors[GRAPH_COLOR_CURSOR]);
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

static void paint_x_cursor(CmpackGraphView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, int x, int width, const gchar *caption)
{
	gint x0, x1, y0, y1;
	GdkPoint pts[4];
	gboolean ok;

	if (x>=view->canvas_rc.x && x<view->canvas_rc.x + view->canvas_rc.width) {
		y0 = view->canvas_rc.y;
		y1 = view->canvas_rc.y + view->canvas_rc.height;
		ok = FALSE;
		for (int i=0; i<nrectangles; i++) {
			const GdkRectangle *area = &rectangles[i];
			if (x <= area->x + area->width && x >= area->x && 
				y0 <= area->y + area->height && y1 >= area->y) {
					ok = TRUE;
					break;
			}
		}
		if (ok) 
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x, y0, x, y1);
		if (!caption) {
			/* Draw a small triangle */
			x0 = x - CURSOR_SIZE/2;
			x1 = x + CURSOR_SIZE/2;
			y1 = view->xcursor_rc.y + view->xcursor_rc.height;
			y0 = y1 - 6;
			ok = FALSE;
			for (int i=0; i<nrectangles; i++) {
				const GdkRectangle *area = &rectangles[i];
				if (x0 <= area->x + area->width && x1 >= area->x && 
					y0 <= area->y + area->height && y1 >= area->y) {
						ok = TRUE;
						break;
				}
			}
			if (ok) {
				pts[0].x = x;	pts[0].y = y1;
				pts[1].x = x1;	pts[1].y = y0;
				pts[2].x = x0;	pts[2].y = y0;
				pts[3].x = x;   pts[3].y = y1;
				gdk_draw_polygon(widget->window, widget->style->fg_gc[widget->state], TRUE, pts, 4);
			}
		} else {
			/* Draw text */
			x0 = x - width/2;
			x1 = x + width/2;
			y0 = view->xcursor_rc.y + 2;
			y1 = view->xcursor_rc.y + view->xcursor_rc.height - 4;
			ok = FALSE;
			for (int i=0; i<nrectangles; i++) {
				const GdkRectangle *area = &rectangles[i];
				if (x0 <= area->x + area->width && x1 >= area->x && 
					y0 <= area->y + area->height && y1 >= area->y) {
						ok = TRUE;
						break;
				}
			}
			if (ok) {
				draw_text(widget, drawable, widget->style->text_gc[widget->state], x, 
					y0, caption, 0.5, 0.0);
			}
		}
	}
}

static void paint_y_cursor(CmpackGraphView *view, GtkWidget *widget, GdkWindow *drawable, 
	const GdkRectangle *rectangles, int nrectangles, int y, int height, const gchar *caption)
{
	gint x0, x1, y0, y1;
	gboolean ok;
	GdkPoint pts[4];

	if (y>=view->canvas_rc.y && y<view->canvas_rc.y + view->canvas_rc.height) {
		x0 = view->canvas_rc.x;
		x1 = view->canvas_rc.x + view->canvas_rc.width;
		ok = FALSE;
		for (int i=0; i<nrectangles; i++) {
			const GdkRectangle *area = &rectangles[i];
			if (y <= area->y + area->height &&y >= area->y && 
				x0 <= area->x + area->width && x1 >= area->x) {
					ok = TRUE;
					break;
			}
		}
		if (ok)
			gdk_draw_line(widget->window, widget->style->fg_gc[widget->state], x0, y, x1, y);
		if (!caption) {
			/* Draw a small triangle */
			y0 = y - CURSOR_SIZE/2;
			y1 = y + CURSOR_SIZE/2;
			x1 = view->ycursor_rc.x;
			x0 = x1 + 6;
			ok = FALSE;
			for (int i=0; i<nrectangles; i++) {
				const GdkRectangle *area = &rectangles[i];
				if (x0 <= area->x + area->width && x1 >= area->x && 
					y0 <= area->y + area->height && y1 >= area->y) {
						ok = TRUE;
						break;
				}
			}
			if (ok) {
				pts[0].y = y;	pts[0].x = x1;
				pts[1].y = y0;	pts[1].x = x0;
				pts[2].y = y1;	pts[2].x = x0;
				pts[3].y = y;	pts[3].x = x1;
				gdk_draw_polygon(widget->window, widget->style->fg_gc[widget->state], TRUE, pts, 4);
			}
		} else {
			/* Draw text */
			y0 = y - height/2;
			y1 = y + height/2;
			x0 = view->ycursor_rc.x + 2;
			x1 = view->ycursor_rc.x + view->xcursor_rc.width - 4;
			ok = FALSE;
			for (int i=0; i<nrectangles; i++) {
				const GdkRectangle *area = &rectangles[i];
				if (x0 <= area->x + area->width && x1 >= area->x && 
					y0 <= area->y + area->height && y1 >= area->y) {
						ok = TRUE;
						break;
				}
			}
			if (ok) {
				draw_text(widget, drawable, widget->style->text_gc[widget->state], x0, 
					y, caption, 0.0, 0.5);
			}
		}
	}
}

/* Refresh the offscreen buffer (if needed) and paint it to the window */
static gboolean cmpack_graph_view_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	CmpackGraphView *view;
	GdkRectangle *rectangles;
	gint x, y, n_rectangles;     

	if (GTK_WIDGET_DRAWABLE(widget)) {
		view = CMPACK_GRAPH_VIEW(widget);
		if (view->dirty) 
			cmpack_graph_view_paint_buffer(view);
		gdk_draw_drawable (widget->window, widget->style->fg_gc[widget->state],
				   CMPACK_GRAPH_VIEW(widget)->offscreen_pixmap,
				   expose->area.x, expose->area.y, expose->area.x, expose->area.y,
				   expose->area.width, expose->area.height);

		gdk_region_get_rectangles(expose->region, &rectangles, &n_rectangles);
		for (int i=0; i<view->x.CursorCount; i++) {
			CmpackGraphCursor *item = view->x.Cursor+i;
			x = RoundToInt(xproj_to_view(view, item->xproj));
			paint_x_cursor(view, widget, widget->window, rectangles, n_rectangles, 
				x, item->width, item->caption);
		}
		for (int i=0; i<view->y.CursorCount; i++) {
			CmpackGraphCursor *item = view->y.Cursor+i;
			y = RoundToInt(yproj_to_view(view, item->xproj));
			paint_y_cursor(view, widget, widget->window, rectangles, n_rectangles, 
				y, item->height, item->caption);
		}
		if (view->mouse_mode == GRAPH_MOUSE_SELECT) {
			paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
				&view->int_colors[GRAPH_COLOR_CURSOR]);
		} else 
		if (view->mouse_mode == GRAPH_MOUSE_ZOOM) {
			paint_rectangle(view, widget, widget->window, rectangles, n_rectangles,
				&view->int_colors[GRAPH_COLOR_ZOOMRECT]);
		} else 
		if (view->focused_item>=0 && view->focused_item<view->item_count) {
			CmpackGraphViewItem *item = &view->items[view->focused_item];
			x = RoundToInt(xproj_to_view(view, item->xproj));
			y = RoundToInt(yproj_to_view(view, item->yproj));
			paint_cross_bar(view, widget, widget->window, rectangles, n_rectangles, 
				x, y, MARKER_SIZE + 2);
		}
		g_free(rectangles);
	}
	return FALSE; 
}

/* Rebuild the offscreen buffer */
static void cmpack_graph_view_create_buffer(CmpackGraphView *view)
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

static void cmpack_graph_view_paint_buffer(CmpackGraphView *view)
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
			paint_x_grid(view, widget, view->offscreen_pixmap);
		if (view->y.ShowGrid)
			paint_y_grid(view, widget, view->offscreen_pixmap);

		// Data
		if (view->item_count>0)
			paint_data(view, widget, view->offscreen_pixmap);
		
		// Labels
		if (view->x.ShowLabels)
			paint_x_scale(view, widget, view->offscreen_pixmap);
		if (view->y.ShowLabels)
			paint_y_scale(view, widget, view->offscreen_pixmap);

		view->dirty = FALSE;
	}
} 

static const GdkColor *item_fg_color(CmpackGraphView *view, GtkWidget *widget, const CmpackGraphViewItem *item)
{
	if (item->selected) 
		return &view->int_colors[GRAPH_COLOR_SELECTED];

	switch (item->color)
	{
	case CMPACK_COLOR_RED:
		return &view->int_colors[GRAPH_COLOR_RED];
	case CMPACK_COLOR_GREEN:
		return &view->int_colors[GRAPH_COLOR_GREEN];
	case CMPACK_COLOR_BLUE:
		return &view->int_colors[GRAPH_COLOR_BLUE];
	case CMPACK_COLOR_YELLOW:
		return &view->int_colors[GRAPH_COLOR_YELLOW];
	case CMPACK_COLOR_GRAY:
		return &widget->style->dark[widget->state];
	default:
		return &widget->style->fg[widget->state];
	}
}

static void paint_item(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable,
	GdkGC *gc, const CmpackGraphViewItem *item)
{
	gint x, y;
	GdkSegment sgs[3];

	x = RoundToInt(xproj_to_view(view, item->xproj));
	y = RoundToInt(yproj_to_view(view, item->yproj));
	if (x - MARKER_SIZE/2 >= view->canvas_rc.x && x + MARKER_SIZE/2 <= view->canvas_rc.x + view->canvas_rc.width &&
		y - MARKER_SIZE/2 >= view->canvas_rc.y && y + MARKER_SIZE/2 <= view->canvas_rc.y + view->canvas_rc.height) {
			gdk_gc_set_foreground(gc, item_fg_color(view, widget, item));
			if (item->error > 0 && view->error_bars) {
				sgs[2].x1 = sgs[0].x1 = x - MARKER_SIZE/2;
				sgs[2].x2 = sgs[0].x2 = x + MARKER_SIZE/2 - 1;
				sgs[1].x1 = sgs[1].x2 = x;
				sgs[1].y1 = sgs[0].y1 = sgs[0].y2 = RoundToInt(yproj_to_view(view, item->yproj + item->error));
				sgs[1].y2 = sgs[2].y1 = sgs[2].y2 = RoundToInt(yproj_to_view(view, item->yproj - item->error));
				gdk_draw_segments(drawable, gc, sgs, 3);
			}
			gdk_draw_arc(drawable, gc, TRUE, x - MARKER_SIZE/2, y - MARKER_SIZE/2, 
				MARKER_SIZE, MARKER_SIZE, 0, 64*360);
	}
}

static void paint_label(CmpackGraphView *view, GtkWidget *widget, PangoLayout *layout, 
	GdkDrawable *drawable, GdkGC *gc, const CmpackGraphViewItem *item)
{
	gint x, y, w, h;

	pango_layout_set_text(layout, item->tag_text, -1);
	x = RoundToInt(xproj_to_view(view, item->xproj) + MARKER_SIZE/2);
	y = RoundToInt(yproj_to_view(view, item->yproj) + MARKER_SIZE/2);
	pango_layout_get_pixel_size(layout, &w, &h);
	if (x + w >= view->canvas_rc.x && x <= view->canvas_rc.x + view->canvas_rc.width &&
		y + h >= view->canvas_rc.y && y <= view->canvas_rc.y + view->canvas_rc.height) {
			gdk_gc_set_foreground(gc, item_fg_color(view, widget, item));
			gdk_draw_layout(drawable, gc, x, y, layout);
	}
}

static void paint_data(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gint i;
	PangoContext *pg;
	PangoFontDescription *desc, *desc2;
	PangoLayout *layout;

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_set_clip_rectangle(gc, &view->canvas_rc);

	// Bold font
	pg = gtk_widget_create_pango_context(widget);
	desc = pango_context_get_font_description(pg);
	desc2 = pango_font_description_copy(desc);
	pango_font_description_set_weight(desc2, PANGO_WEIGHT_BOLD);
	pango_context_set_font_description(pg, desc2);
	pango_font_description_free(desc2);
	layout = pango_layout_new(pg);

	// First run: Draw non-topmost circles
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && !item->topmost)
			paint_item(view, widget, drawable, gc, item);
	}

	// Second run: Draw topmost circles
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && item->topmost)
			paint_item(view, widget, drawable, gc, item);
	}

	// Second run: Draw labels
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && item->tag_text)
			paint_label(view, widget, layout, drawable, gc, item);
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

static void update_rectangles(CmpackGraphView *view)
{
	GtkWidget *widget;
	gint x_height, sy_width, cx_height, cy_width, uh, uw, vh, vw;
	gint cm_left, cm_top, cm_right, cm_bottom;
	gint gm_left, gm_top, gm_right, gm_bottom;
	gboolean cursor_x, cursor_y;

	if (GTK_WIDGET_REALIZED(view)) {
		widget = GTK_WIDGET(view);
	  
		x_height = compute_scale_labels_height(widget, &view->x);
		text_extents(widget, view->x.Name, &uw, &uh);
		uw += 8;
		x_height = MAX(16, MAX(x_height, uh)) + 4;

		sy_width = compute_scale_labels_width(widget, &view->y, view->y.MaxPrec);
		text_extents(widget, view->y.Name, &vw, &vh);
		vh += 4;
		sy_width = MAX(32, MAX(sy_width, vw)) + 8;

		cx_height = compute_cursor_labels_height(widget, view->x.Cursor, view->x.CursorCount) + 4;
		cy_width = compute_cursor_labels_width(widget, view->y.Cursor, view->y.CursorCount) + 8;
		
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
				cm_left += sy_width;
			} else {
				/* X axis is outside right */
				cm_right += sy_width;
			}
		}

		cursor_x = cursor_y = FALSE;
		if (view->x.CursorCount) {
			if (!view->x.LabelsOpposite) {
				/* X cursor labels are outside top */
				cm_top += cx_height;
			} else {
				/* X cursor labels is outside bottom */
				cm_bottom += cx_height;
			}
			cursor_x = TRUE;
		}
		if (view->y.CursorCount) {
			if (!view->y.LabelsOpposite) {
				/* Y cursor labels are outside left */
				cm_right += cy_width;
			} else {
				/* Y cursor labels are outside right */
				cm_left += cy_width;
			}
			cursor_y = TRUE;
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

		memset(&view->xcursor_rc, 0, sizeof(GdkRectangle));
		if (cursor_x) {
			/* X scale is outside */
			view->xcursor_rc.x = view->canvas_rc.x;
			view->xcursor_rc.width = view->canvas_rc.width;
			if (!view->x.LabelsOpposite) {
				/* X cursors are outside top */
				view->xcursor_rc.y = 0;
			} else {
				/* X cursors are outside bottom */
				view->xcursor_rc.y = view->canvas_rc.y + view->canvas_rc.height;
			}
			view->xcursor_rc.height = cx_height;
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
			view->yscale_rc.width = sy_width;
		}

		memset(&view->ycursor_rc, 0, sizeof(GdkRectangle));
		if (cursor_y) {
			/* Y scale is outside */
			view->ycursor_rc.y = view->canvas_rc.y;
			view->ycursor_rc.height = view->canvas_rc.height;
			if (!view->x.LabelsOpposite) {
				/* X axis is outside right */
				view->ycursor_rc.x = view->canvas_rc.x + view->canvas_rc.width;
			} else {
				/* X axis is outside left */
				view->ycursor_rc.x = 0;
			}
			view->ycursor_rc.width = cy_width;
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
		}

		view->x.Center = view->graph_rc.x + 0.5 * view->graph_rc.width;
		view->y.Center = view->graph_rc.y + 0.5 * view->graph_rc.height;
	}
}

/* Update mapping coefficients */
static void update_x_pxlsize(CmpackGraphView *view)
{	
	gdouble XZoom = pow(view->zoom_base, view->x.ZoomPos);
	if (view->graph_rc.width>0) {
		view->x.PxlSize = (view->x.ProjMax - view->x.ProjMin) / 
			(view->graph_rc.width * XZoom);
	} else {
		view->x.PxlSize = 0.0;
	}
}

/* Checks if the visible area is inside the limits */
static void restrict_x_to_limits(CmpackGraphView *view)
{
	gint left, right;
    gdouble a, b;
	
	right = view->graph_rc.x + view->graph_rc.width;
    a = view->x.Center + (view->x.ProjMax - view->x.ProjPos)/view->x.PxlSize;
    if (a < right) {
        view->x.ProjPos = view->x.ProjMax - view->x.PxlSize*(right - view->x.Center);
    } else {
		left = view->graph_rc.x;
        b = view->x.Center + (view->x.ProjMin - view->x.ProjPos)/view->x.PxlSize;
        if (b > left)
            view->x.ProjPos = view->x.ProjMin - view->x.PxlSize*(left - view->x.Center);
    }
}

/* Physical units -> projection units */
static gdouble x_to_proj(CmpackGraphView *view, gdouble x)
{
	if (view->x.Log) 
		return (!view->x.Reverse ? 1.0 : -1.0) * log10(x);
	else
		return (!view->x.Reverse ? 1.0 : -1.0) * x;
}

/* Projection units -> physical units */
static gdouble proj_to_x(CmpackGraphView *view, gdouble u)
{
	if (view->x.Log) 
		return pow(10.0, (!view->x.Reverse ? 1.0 : -1.0) * u);
	else
		return (!view->x.Reverse ? 1.0 : -1.0) * u;
}

/* Projection units -> display units */
static gdouble xproj_to_view(CmpackGraphView *view, gdouble x)
{
	return view->x.Center + (x - view->x.ProjPos)/view->x.PxlSize;
}

/* Display units -> projection units */
static gdouble view_to_xproj(CmpackGraphView *view, gdouble u)
{
	return (u - view->x.Center)*view->x.PxlSize + view->x.ProjPos;
}

/* Physical units -> display units */
static gdouble x_to_view(CmpackGraphView *view, gdouble x)
{
	return xproj_to_view(view, x_to_proj(view, x));
}

/* Physical units -> projection units */
static gdouble view_to_x(CmpackGraphView *view, gdouble u)
{
	return proj_to_x(view, view_to_xproj(view, u));
}

/* Update range and position of horizontal scroll bar */
static void update_hsb(CmpackGraphView *view)
{
	gdouble upper, page_size, pos;

	GtkAdjustment *adj = view->hadjustment;
	if (adj && view->x.PxlSize>0) {
		upper = RoundToInt((view->x.ProjMax - view->x.ProjMin) / view->x.PxlSize);
		page_size = view->graph_rc.width;
		pos = (view->x.ProjPos - view->x.ProjMin)/view->x.PxlSize - view->x.Center + view->graph_rc.x;
		g_object_set(G_OBJECT(adj), "upper", (gdouble)upper, "page-size", (gdouble)page_size, 
			"value", (gdouble)pos, NULL);
	}
}

/* Update mapping coefficients */
static void update_y_pxlsize(CmpackGraphView *view)
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
static void restrict_y_to_limits(CmpackGraphView *view)
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
static gdouble y_to_proj(CmpackGraphView *view, gdouble y)
{
	if (view->y.Log) 
		return (!view->y.Reverse ? 1.0 : -1.0) * log10(y);
	else
		return (!view->y.Reverse ? 1.0 : -1.0) * y;
}

/* Projection units -> physical units */
static gdouble proj_to_y(CmpackGraphView *view, gdouble v)
{
	if (view->y.Log) 
		return pow(10.0, (!view->y.Reverse ? 1.0 : -1.0) * v);
	else
		return (!view->y.Reverse ? 1.0 : -1.0) * v;
}

/* Projection units -> display units */
static gdouble yproj_to_view(CmpackGraphView *view, gdouble y)
{
	return view->y.Center - (y - view->y.ProjPos)/view->y.PxlSize;
}

/* Display units -> projection units */
static gdouble view_to_yproj(CmpackGraphView *view, gdouble v)
{
	return view->y.ProjPos - (v - view->y.Center)*view->y.PxlSize;
}

/* Physical units -> display units */
static gdouble y_to_view(CmpackGraphView *view, gdouble y)
{
	return yproj_to_view(view, y_to_proj(view, y));
}

/* Physical units -> projection units */
static gdouble view_to_y(CmpackGraphView *view, gdouble v)
{
	return proj_to_y(view, view_to_yproj(view, v));
}

/* Update range and position of vertical scroll bar */
static void update_vsb(CmpackGraphView *view)
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
static void format_numeric(gchar *buf, gdouble value, CmpackGraphFormat format, gint prec)
{
	switch(format) 
	{
	case GRAPH_EXP:
        // Always exponential form
		sprintf(buf, "%.*e", prec, value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.*e", prec, 0.0);
		break;

	case GRAPH_INT:
		// Integer number without decimal places
		sprintf(buf, "%.0f", value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.0f", 0.0);
		break;

	default:
        // Number with fixed number of decimal places
		sprintf(buf, "%.*f", prec, value);
		if (buf[0]=='-' && strspn(buf, "-0.,e")==strlen(buf))
			sprintf(buf, "%.*f", prec, 0.0);
		break;
	}
}

/* Date and time formatting function for scale labels */
static void format_datetime(gchar *buf, const struct tm *tm, tDateFormat prec)
{
	switch (prec)
	{
	case FORMAT_Y:
		sprintf(buf, "%04d", tm->tm_year+1900);
		break;
	case FORMAT_MY:
		sprintf(buf, "%04d-%02d", tm->tm_year+1900, tm->tm_mon+1);
		break;
	case FORMAT_YMD:
		sprintf(buf, "%04d-%02d-%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		break;
	case FORMAT_YMDH:
		sprintf(buf, "%04d-%02d-%02d %02d:00", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour);
		break;
	case FORMAT_HM:
		sprintf(buf, "%d:%02d", tm->tm_hour, tm->tm_min);
		break;
	default:
		sprintf(buf, "%d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
		break;
	}		
}

/* Compute max. width of scale label */
static gint compute_scale_labels_width(GtkWidget *widget, CmpackGraphViewAxis *axis, int prec)
{
	gchar	buf[512];
	//gint	w1; //, w2;

	if (axis->Format!=GRAPH_TIME) {
		double val = -MAX(fabs(axis->Min), fabs(axis->Max));
		format_numeric(buf, val, axis->Format, prec);
		gint w1 = text_width(widget, buf);
		//format_numeric(buf, axis->Max, axis->Format, prec);
		//w2 = text_width(widget, buf);
		return w1; //MAX(w1, w2);
	} else {
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		format_datetime(buf, &tm, (tDateFormat)prec);
		return text_width(widget, buf);
	}
}

/* Compute height of scale label */
static gint compute_scale_labels_height(GtkWidget *widget, CmpackGraphViewAxis *axis)
{
    return text_height(widget, "X");
}

/* Compute height of cursor labels */
static gint compute_cursor_labels_height(GtkWidget *widget, CmpackGraphCursor *list, int count)
{
	gint height = 0, h = text_height(widget, "X");
	for (int i=0; i<count; i++) {
		if (list[i].caption) {
			list[i].width = (text_width(widget, list[i].caption)+1)/2*2+1;
			list[i].height = h;
		} else {
			list[i].width = CURSOR_SIZE;
			list[i].height = 8;
		}
		height = MAX(height, list[i].height);
	}
	return height;
}

/* Compute width of cursor labels */
static gint compute_cursor_labels_width(GtkWidget *widget, CmpackGraphCursor *list, int count)
{
    gint width = 0, h = (text_height(widget, "X")+1)/2*2+1;
	for (int i=0; i<count; i++) {
		if (list[i].caption) {
			list[i].height = h;
			list[i].width = text_width(widget, list->caption);
		} else {
			list[i].height = 8;
			list[i].width = CURSOR_SIZE;
		}
		width = MAX(width, list[i].width);
	}
	return width;
}

/* Compute number of decimal points for given step size */
static gint step_to_prec(gdouble step)
{
	if (step<=0.0 || step>=1.0)
		return 0;
	return (gint)ceil(-log10(step));
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_x_grid_step_numeric(CmpackGraphView *view, gdouble minval, gdouble maxval, 
								gdouble *step, gint *minprec)
{
    gint xprec, oldprec;
	gdouble xstep, dx, delta;
	GtkWidget *widget = GTK_WIDGET(view);

	xprec = 0;
	xstep = fabs(maxval - minval);

	switch (view->x.Format)
	{
	case GRAPH_INT:
		// Integer number without decimal places
		dx = (compute_scale_labels_width(widget, &view->x, xprec)+40)*view->x.PxlSize;
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
			dx = (compute_scale_labels_width(widget, &view->x, xprec)+40)*view->x.PxlSize;
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
	if (minprec)
		*minprec = xprec;
}

static void compute_x_grid_step_datetime(CmpackGraphView *view, gdouble xmin, gdouble xmax, 
	tDateUnit *unit, gint *step, tDateFormat *fmt)
{
	int i;
	gdouble delta;
	
	GtkWidget *widget = GTK_WIDGET(view);

    // Time mode
	if ((xmax-xmin) > 1.0) {
		for (i=NumDSteps-2; i>=0; i--) {
			gdouble dx = (compute_scale_labels_width(widget, &view->x, DateSteps[i].fmt)+40)*view->x.PxlSize;
			switch (DateSteps[i].unit)
			{
			case YEARS:		delta = DateSteps[i].step * 365.25; break;
			case MONTHS:	delta = DateSteps[i].step * 30.5; break;
			case DAYS:		delta = DateSteps[i].step * 1.0; break;
			default:		delta = DateSteps[i].step / 86400.0; break;
			}
			if (delta < dx)
				break;
		}
		*step = DateSteps[i+1].step;
		*unit = DateSteps[i+1].unit;
		*fmt  = DateSteps[i+1].fmt;
	} else {
		for (i=NumTSteps-2; i>=0; i--) {
			gdouble dx = (compute_scale_labels_width(widget, &view->x, TimeSteps[i].fmt)+40)*view->x.PxlSize;
			if (TimeSteps[i].step / 86400.0 < dx)
				break;
		}
		*step = TimeSteps[i+1].step;
		*fmt  = TimeSteps[i+1].fmt;
		*unit = SECONDS;
	}
}

/* Compute horizontal grid step and distance between two labels on horizontal scale */
static void compute_y_grid_step(CmpackGraphView *view, gdouble minval, gdouble maxval, 
								gdouble *step, gint *minprec)
{
    gint	yprec;
	gdouble dy, ystep, delta;

	dy = (compute_scale_labels_height(GTK_WIDGET(view), &view->y)+32)*view->y.PxlSize;

	switch (view->y.Format)
	{
	case GRAPH_INT:
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
		yprec = 0;
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
		yprec = LimitInt(step_to_prec(ystep), view->y.MinPrec, view->y.MaxPrec);
		break;
	}

	if (step)
		*step = ystep;
	if (minprec) 
		*minprec = yprec;
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
static void paint_x_scale(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gchar	buf[512];
	gdouble	val, xmin, xmax;
	gint	y, left, right;

	GdkGC *gc = gdk_gc_new(drawable);
	gdk_gc_copy(gc, widget->style->text_gc[widget->state]);

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

	if (view->x.Format != GRAPH_TIME) {
		gdouble step;
		gint x, prec;

		if (view->x.Log && log10(xmax/xmin)>3) {
			// Log scale if range is greater than three periods
			xmin = log10(MAX(DBL_MIN, xmin));
			xmax = log10(MAX(DBL_MIN, xmax));
			compute_x_grid_step_numeric(view, xmin, xmax, &step, &prec);
			xmin = ceil(xmin);
			while (xmin <= xmax) {
				val = pow(10, xmin);
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					format_numeric(buf, val, view->x.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				xmin += step;
			}
		} else
		if (view->x.Log && log10(xmax/xmin)>1) {
			// Log scale if range is greater than one period
			xmin = log10(MAX(DBL_MIN, xmin));
			xmax = log10(MAX(DBL_MIN, xmax));
			step = 1.0;
			prec = 1;
			xmin = floor(xmin);
			while (xmin <= xmax) {
				val = pow(10, xmin);
				x = RoundToInt(x_to_view(view, 1.0*val));
				if (x >= left && x < right) {
					format_numeric(buf, 1.0*val, view->x.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				x = RoundToInt(x_to_view(view, 2.0*val));
				if (x >= left && x < right) {
					format_numeric(buf, 2.0*val, view->x.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				x = RoundToInt(x_to_view(view, 5.0*val));
				if (x >= left && x < right) {
					format_numeric(buf, 5.0*val, view->x.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				xmin += step;
			}
		} else {
			compute_x_grid_step_numeric(view, xmin, xmax, &step, &prec);
			xmin = ceil(xmin/step)*step;
			while (xmin <= xmax) {
				val = xmin;
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					format_numeric(buf, val, view->x.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				xmin += step;
			}
		}
	} else {
		tDateUnit unit;
		tDateFormat fmt;
		gint step, x, x0, x1;
		gdouble ipart;
		struct tm t0, t1;

		// Time scale
		compute_x_grid_step_datetime(view, xmin, xmax, &unit, &step, &fmt);
		switch (unit)
		{
		case YEARS:
			jdtime(xmin, &t0);
			ceildate(&t0, step, YEARS);
			jdtime(xmax, &t1);
			while (t0.tm_year <= t1.tm_year) {
				val = timejd(&t0);
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					format_datetime(buf, &t0, fmt);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				t0.tm_year += step;
			}
			break;

		case MONTHS:
			jdtime(xmin, &t0);
			ceildate(&t0, step, MONTHS);
			x0 = t0.tm_year*12 + t0.tm_mon;
			jdtime(xmax, &t1);
			x1 = t1.tm_year*12 + t1.tm_mon;
			while (x0 <= x1) {
				val = timejd(&t0);
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					format_datetime(buf, &t0, fmt);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				x0 += step;
				t0.tm_year = x0 / 12;
				t0.tm_mon  = x0 % 12;
			}
			break;

		case DAYS:
			xmin = ceil((xmin-0.5)/step)*step+0.5;		
			while (xmin <= xmax) {
				val = xmin;
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					jdtime(val, &t0);
					format_datetime(buf, &t0, fmt);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				xmin += step;
			}
			break;

		default:
			xmin = ceil(modf(xmin-0.5, &ipart)*86400.0/step)*step;
			xmax = ((xmax-0.5) - ipart)*86400.0;
			while (xmin <= xmax) {
				val = ipart + xmin/86400.0 + 0.5;
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) {
					jdtime(val, &t0);
					format_datetime(buf, &t0, fmt);
					draw_text(widget, drawable, gc, x, y, buf, 0.5, 0.0);
				}
				xmin += step;
			}
			break;
		}
	}

	g_object_unref(gc);
}

/* Paint labels on y-axis scale */
static void paint_y_scale(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gchar	buf[512];
	gdouble	val, step, ymin, ymax;
	gint	x, y, top, bottom, prec;

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

	if (view->y.Format != GRAPH_TIME) {
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
					format_numeric(buf, val, view->y.Format, prec);
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
					format_numeric(buf, 1.0*val, view->y.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
				}
				y = RoundToInt(y_to_view(view, 2.0*val));
				if (y >= top && y < bottom) {
					format_numeric(buf, 2.0*val, view->y.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
				}
				y = RoundToInt(y_to_view(view, 5.0*val));
				if (y >= top && y < bottom) {
					format_numeric(buf, 5.0*val, view->y.Format, prec);
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
					format_numeric(buf, val, view->y.Format, prec);
					draw_text(widget, drawable, gc, x, y, buf, 1.0, 0.5);
				}
				ymin += step;
			}
		}
	}

	g_object_unref(gc);
}

/* Paints x-axis grid */
static void paint_x_grid(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gdouble	val, xmin, xmax;
	gint	left, right, top, bottom;

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

	left = view->canvas_rc.x;
	right = view->canvas_rc.x + view->canvas_rc.width;
	top = view->canvas_rc.y;
	bottom = view->canvas_rc.y + view->canvas_rc.height;

	if (view->x.Format != GRAPH_TIME) {
		gdouble step;
		gint x;

		if (view->x.Log && log10(xmax/xmin)>3) {
			// Log scale if range is greater than three periods
			xmin = log10(MAX(DBL_MIN, xmin));
			xmax = log10(MAX(DBL_MIN, xmax));
			compute_x_grid_step_numeric(view, xmin, xmax, &step, NULL);
			xmin = ceil(xmin);
			while (xmin <= xmax) {
				val = pow(10, xmin);
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right) 
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				xmin += step;
			}
		} else
		if (view->x.Log && log10(xmax/xmin)>1) {
			// Log scale if range is greater than one period
			xmin = log10(MAX(DBL_MIN, xmin));
			xmax = log10(MAX(DBL_MIN, xmax));
			step = 1.0;
			xmin = floor(xmin);
			while (xmin <= xmax) {
				val = pow(10, xmin);
				x = RoundToInt(x_to_view(view, 1.0*val));
				if (x >= left && x < right)
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				x = RoundToInt(x_to_view(view, 2.0*val));
				if (x >= left && x < right) 
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				x = RoundToInt(x_to_view(view, 5.0*val));
				if (x >= left && x < right) 
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				xmin += step;
			}
		} else {
			// Lin scale or log scale if range is smaller than one period
			compute_x_grid_step_numeric(view, xmin, xmax, &step, NULL);
			xmin = ceil(xmin/step)*step;
			while (xmin <= xmax) {
				x = RoundToInt(x_to_view(view, xmin));
				if (x >= left && x < right)
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				xmin += step;
			}
		}
	} else {
		// Time scale
		tDateUnit unit;
		tDateFormat fmt;
		gint step, x, x0, x1;
		gdouble ipart;
		struct tm t0, t1;

		// Time scale
		compute_x_grid_step_datetime(view, xmin, xmax, &unit, &step, &fmt);
		switch (unit)
		{
		case YEARS:
			jdtime(xmin, &t0);
			ceildate(&t0, step, YEARS);
			jdtime(xmax, &t1);
			while (t0.tm_year <= t1.tm_year) {
				x = RoundToInt(x_to_view(view, timejd(&t0)));
				if (x >= left && x < right) 
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				t0.tm_year += step;
			}
			break;

		case MONTHS:
			jdtime(xmin, &t0);
			ceildate(&t0, step, MONTHS);
			x0 = t0.tm_year*12 + t0.tm_mon;
			jdtime(xmax, &t1);
			x1 = t1.tm_year*12 + t1.tm_mon;
			while (x0 <= x1) {
				x = RoundToInt(x_to_view(view, timejd(&t0)));
				if (x >= left && x < right)
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				x0 += step;
				t0.tm_year = x0 / 12;
				t0.tm_mon  = x0 % 12;
			}
			break;

		case DAYS:
			xmin = ceil((xmin-0.5)/step)*step+0.5;		
			while (xmin <= xmax) {
				x = RoundToInt(x_to_view(view, xmin));
				if (x >= left && x < right) 
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				xmin += step;
			}
			break;

		default:
			xmin = ceil(modf(xmin-0.5, &ipart)*86400.0/step)*step;
			xmax = ((xmax-0.5) - ipart)*86400.0;
			while (xmin <= xmax) {
				val = ipart + xmin/86400.0 + 0.5;
				x = RoundToInt(x_to_view(view, val));
				if (x >= left && x < right)
					gdk_draw_line(drawable, widget->style->dark_gc[widget->state], x, top, x, bottom);
				xmin += step;
			}
			break;
		}
	}
}

/* Paint y-axis grid */
static void paint_y_grid(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *drawable)
{
	gdouble	val, step, ymin, ymax;
	gint	y, left, right, top, bottom;

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

	if (view->y.Format != GRAPH_TIME) {
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
}

static gboolean valid_mouse_pos(CmpackGraphView *view, gint x, gint y)
{
	double a, b;

	if (x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
		x < view->canvas_rc.x + view->canvas_rc.width &&
		y < view->canvas_rc.y + view->canvas_rc.height) {
			a = view_to_xproj(view, x);
			if (a < view->x.ProjMin || a > view->x.ProjMax)
				return FALSE;
			b = view_to_yproj(view, y);
			if (b < view->y.ProjMin || b > view->y.ProjMax)
				return FALSE;
			return TRUE;
	}
	return FALSE;
}

static void invalidate_cross_bar(CmpackGraphView *view)
{
	gint x, y, r = CROSS_BAR_SIZE + MARKER_SIZE + 5;

	CmpackGraphViewItem *item = &view->items[view->focused_item];
	x = RoundToInt(xproj_to_view(view, item->xproj));
	y = RoundToInt(yproj_to_view(view, item->yproj));
	gtk_widget_queue_draw_area(GTK_WIDGET(view), x-r, y-r, 2*r+1, 2*r+1);
}

static void invalidate_x_cursor(CmpackGraphView *view, CmpackGraphCursor *item, gint x1, gint x2)
{
	gint left, width, top, height;

	left = MIN(x1, x2) - item->width/2;
	width = ABS(x1 - x2) + 2*(item->width/2) + 1;
	top = MIN(view->xcursor_rc.y, view->canvas_rc.y);
	height = MAX(view->xcursor_rc.y+view->xcursor_rc.height, view->canvas_rc.y+view->canvas_rc.height) - top;
	gtk_widget_queue_draw_area(GTK_WIDGET(view), left, top, width, height);
}

static void invalidate_y_cursor(CmpackGraphView *view, CmpackGraphCursor *item, gint y1, gint y2)
{
	gint left, width, top, height;

	top = MIN(y1, y2) - item->height/2;
	height = ABS(y1 - y2) + 2*(item->height/2) + 1;
	left = MIN(view->ycursor_rc.x, view->canvas_rc.x);
	width = MAX(view->ycursor_rc.x+view->ycursor_rc.width, view->canvas_rc.x+view->canvas_rc.width);
	gtk_widget_queue_draw_area(GTK_WIDGET(view), left, top, width, height);
}

static gboolean timer_cb(CmpackGraphView *view)
{
	if (view->focused_item>=0) {
		view->cursor_phase = (view->cursor_phase+5) % 360;
		invalidate_cross_bar(view);
		return TRUE;
	} else {
		view->timer_id = 0;
		return FALSE;
	}
}

static gboolean cmpack_graph_view_motion (GtkWidget *widget, GdkEventMotion *event)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(widget);
	GdkRectangle old_area, new_area, common;
	GdkRegion *invalid_region, *common_region;
	gint x, y, row, old_pos, new_pos;

	gdk_window_get_pointer (widget->window, &x, &y, NULL);

	switch (view->mouse_mode)
	{
	case GRAPH_MOUSE_ZOOM:
	case GRAPH_MOUSE_SELECT:
		// Draw a rectangle
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
			// make sure the border is invalidated 
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
		break;

	case GRAPH_MOUSE_SHIFT:
		// Graph panning
		x = LimitInt(x, view->canvas_rc.x, view->canvas_rc.x + view->canvas_rc.width);
		y = LimitInt(y, view->canvas_rc.y, view->canvas_rc.y + view->canvas_rc.height);
		view->x.ProjPos = view->mouse_posx + view->x.PxlSize*(view->mouse_x1 - x);
		view->y.ProjPos = view->mouse_posy - view->y.PxlSize*(view->mouse_y1 - y);
		restrict_x_to_limits(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		break;

	case GRAPH_MOUSE_MOVE_CX:
		// Move X-axis cursor
		if (view->mouse_param>=0 && view->mouse_param<view->x.CursorCount) {
			CmpackGraphCursor *item = view->x.Cursor + view->mouse_param;
			old_pos = RoundToInt(xproj_to_view(view, item->xproj));
			item->xproj = LimitDbl(view_to_xproj(view, x), view->x.ProjMin, view->x.ProjMax);
			new_pos = RoundToInt(xproj_to_view(view, item->xproj));
			invalidate_x_cursor(view, item, old_pos, new_pos);
			g_signal_emit(view, graph_view_signals[CURSOR_MOVED], 0, CMPACK_AXIS_X, view->mouse_param);
		}
		break;

	case GRAPH_MOUSE_MOVE_CY:
		// Move X-axis cursor
		if (view->mouse_param>=0 && view->mouse_param<view->y.CursorCount) {
			CmpackGraphCursor *item = view->y.Cursor + view->mouse_param;
			old_pos = RoundToInt(yproj_to_view(view, item->xproj));
			item->xproj = LimitDbl(view_to_yproj(view, y), view->y.ProjMin, view->y.ProjMax);
			new_pos = RoundToInt(yproj_to_view(view, item->xproj));
			invalidate_y_cursor(view, item, old_pos, new_pos);
			g_signal_emit(view, graph_view_signals[CURSOR_MOVED], 0, CMPACK_AXIS_Y, view->mouse_param);
		}
		break;
		
	default:
		break;
	}

	// Mouse cursor tracking
	if (valid_mouse_pos(view, x, y)) {
		if (x!=view->last_mouse_x || y!=view->last_mouse_y) {
			view->last_mouse_x = x;
			view->last_mouse_y = y;
			// Update mouse cursor
			GdkCursorType cursor = cmpack_graph_choose_mouse_cursor(view, x, y);
			if (view->mouse_cursor != cursor) {
				if (cursor!=GDK_LAST_CURSOR) {
					GdkCursor *c = gdk_cursor_new_for_display(gtk_widget_get_display(widget), cursor);
					gdk_window_set_cursor(widget->window, c);
					if (c)
						gdk_cursor_unref(c);
				} else {
					gdk_window_set_cursor(widget->window, NULL);
				}
				view->mouse_cursor = cursor;
			}
			if (!view->mouse_mode && (view->activation_mode || view->selection_mode)) {
				row = find_item(view, x, y);
				if (row>=0 && !view->items[row].enabled)
					row = -1;
				if (row!=view->focused_item) {
					if (view->focused_item>=0)
						invalidate_cross_bar(view);
					view->focused_item = row;
					if (view->focused_item>=0) {
						view->cursor_phase = 0;
						if (!view->timer_id) 
							view->timer_id = g_timeout_add(100, GSourceFunc(timer_cb), view);
						invalidate_cross_bar(view);
					}
				}
			}
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
	} else {
		if (view->mouse_cursor != GDK_LAST_CURSOR) {
			gdk_window_set_cursor(widget->window, NULL);
			view->mouse_cursor = GDK_LAST_CURSOR;
		}
		if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
			view->last_mouse_x = view->last_mouse_y = -1;
			if (view->focused_item>=0) {
				invalidate_cross_bar(view);
				view->focused_item = -1;
			}
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
	}
	return FALSE;
}

static gboolean cmpack_graph_view_leave(GtkWidget *widget, GdkEventCrossing *event)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(widget);

	if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
		view->last_mouse_x = view->last_mouse_y = -1;
		if (view->focused_item>=0) {
			invalidate_cross_bar(view);
			view->focused_item = -1;
		}
		g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
	return FALSE;
}

static gboolean cmpack_graph_view_left_click (CmpackGraphView *view, GtkWidget *widget, GdkEventButton *event)
{
	gint x = (gint)event->x, y = (gint)event->y;

	if (x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
		x < view->canvas_rc.x + view->canvas_rc.width &&
		y < view->canvas_rc.y + view->canvas_rc.height) {
			// Zoom to region mode
			if (event->state & GDK_CONTROL_MASK && view->mouse_ctrl) {
				cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_ZOOM);
				return TRUE;
			}
			// Select by region mode
			if (event->state & GDK_SHIFT_MASK && (view->selection_mode != GTK_SELECTION_NONE)) {
				cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_SELECT);
				return TRUE;
			}
			// Vertical (X) cursors
			for (int i=view->x.CursorCount-1; i>=0; i--) {
				gint cx = (int)(xproj_to_view(view, view->x.Cursor[i].xproj)+0.5);
				if (abs(x-cx) <= CLICK_TOLERANCE) {
					cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_MOVE_CX, i);
					return TRUE;
				}
			}
			// Horizontal (Y) cursors
			for (int i=view->y.CursorCount-1; i>=0; i--) {
				gint cy = (int)(yproj_to_view(view, view->y.Cursor[i].xproj)+0.5);
				if (abs(y-cy) <= CLICK_TOLERANCE) {
					cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_MOVE_CY, i);
					return TRUE;
				}
			}
			if (view->selection_mode || view->activation_mode) {
				gint index = find_item(view, x, y);
				if (index>=0 && view->items[index].enabled && view->items[index].visible) {
					if (view->activation_mode == CMPACK_ACTIVATION_CLICK) {
						// Single click activates item
						cmpack_graph_view_item_activate(view, index);
					} else {
						// Single click selects item
						if (view->selection_mode != GTK_SELECTION_NONE && !view->items[index].selected) {
							cmpack_graph_view_unselect_all_internal(view);
							view->items[index].selected = TRUE;
							cmpack_graph_view_queue_draw_item (view, view->items+index);
							g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);
						}
					}
					// Remember last item
					if (view->last_single_clicked < 0)
						view->last_single_clicked = index;
				} else {
					// Single click deselect item
					if (view->selection_mode != GTK_SELECTION_NONE && view->selection_mode != GTK_SELECTION_BROWSE) {
						if (cmpack_graph_view_unselect_all_internal(view))
							g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);
					}
					// Clear last item
					if (view->last_single_clicked >= 0)
						view->last_single_clicked = -1;
					// Start panning mode
					if (view->mouse_ctrl)
						cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_SHIFT);
				}
				return TRUE;
			}
			// Start panning mode
			if (view->mouse_ctrl) {
				cmpack_graph_view_enter_mouse_mode(view, x, y, GRAPH_MOUSE_SHIFT);
				return TRUE;
			}
	}
	return FALSE;
}

//
// Left button double click
//
static gboolean cmpack_graph_view_left_dblclick (CmpackGraphView *view, GtkWidget *widget, GdkEventButton *event)
{
	gboolean ok = FALSE;
	gint x = (gint)event->x, y = (gint)event->y;

	if (x >= view->canvas_rc.x && y >= view->canvas_rc.y && 
		x < view->canvas_rc.x + view->canvas_rc.width &&
		y < view->canvas_rc.y + view->canvas_rc.height) {
			if (view->activation_mode == CMPACK_ACTIVATION_DBLCLICK) {
				gint index = find_item(view, x, y);
				if (index>=0 && index == view->last_single_clicked &&
					view->items[index].enabled && view->items[index].visible) {
						cmpack_graph_view_item_activate(view, index);
						ok = TRUE;
				}
				view->last_single_clicked = -1;
			}
	}
	return ok;
}

static gboolean cmpack_graph_view_button_press (GtkWidget *widget, GdkEventButton *event)
{
	if (!GTK_WIDGET_HAS_FOCUS(widget))
		gtk_widget_grab_focus(widget);

	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		// Left button single click
		return cmpack_graph_view_left_click(view, widget, event);
	}
	if (event->button==1 && event->type==GDK_2BUTTON_PRESS) {
		// Left button double click
		return cmpack_graph_view_left_dblclick(view, widget, event);
	}
	return FALSE; 
}

static gboolean cmpack_graph_view_button_release(GtkWidget *widget, GdkEventButton *event)
{
	gint i;
	gboolean dirty, selected;
	gdouble x0, y0, x1, y1, pos, zoom;
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);
	GdkRectangle rect;
  
	if (view->mouse_mode) {
		rect.x = MIN(view->mouse_x1, view->mouse_x2);
		rect.y = MIN(view->mouse_y1, view->mouse_y2);
		rect.width = ABS(view->mouse_x2 - view->mouse_x1) + 1;
		rect.height = ABS(view->mouse_y2 - view->mouse_y1) + 1;

		switch (view->mouse_mode)
		{
		case GRAPH_MOUSE_SELECT:
			// Select points in an area
			x0 = view_to_xproj(view, rect.x);
			x1 = view_to_xproj(view, rect.x + rect.width);
			y0 = view_to_yproj(view, rect.y + rect.height);
			y1 = view_to_yproj(view, rect.y);
			dirty = FALSE;
			for (i=0; i<view->item_count; i++) {
				CmpackGraphViewItem *item = &view->items[i];
				selected = (item->visible && item->enabled && item->xproj>=x0 && item->xproj<=x1 && 
					item->yproj>=y0 && item->yproj<=y1);
				if (item->selected!=selected) {
					item->selected = selected;
					dirty = TRUE;
				}
			}
			invalidate(view);
			if (dirty) 
				g_signal_emit(view, graph_view_signals[SELECTION_CHANGED], 0);
			break;

		case GRAPH_MOUSE_ZOOM:
			// Zoom to selected area 
			dirty = FALSE;
			pos = view_to_xproj(view, rect.x + rect.width*0.5);
			zoom = log((view->x.ProjMax-view->x.ProjMin)/(rect.width*view->x.PxlSize))/log(view->zoom_base);
			dirty |= set_x_axis(view, zoom, pos);
			pos = view_to_yproj(view, rect.y + rect.height*0.5);
			zoom = log((view->y.ProjMax-view->y.ProjMin)/(rect.height*view->y.PxlSize))/log(view->zoom_base);
			dirty |= set_y_axis(view, zoom, pos);
			invalidate(view);
			if (dirty && (view->last_mouse_x>=0 || view->last_mouse_y>=0))
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
			break;

		default:
			break;
		}
		cmpack_graph_view_leave_mouse_mode(view);
	}
	return TRUE;
}

static gboolean cmpack_graph_view_key_press (GtkWidget *widget, GdkEventKey *event)
{
	gint i;
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	if (view->mouse_mode) {
		if (event->keyval == GDK_Escape) {
			if (view->mouse_mode == GRAPH_MOUSE_SELECT) {
				for (i=0; i<view->item_count; i++) {
					CmpackGraphViewItem *item = &view->items[i];
					item->selected = item->selected_before_rubberbanding;
				}
				invalidate(view);
			}
			cmpack_graph_view_leave_mouse_mode(view);
		}
		return TRUE;
	}
	return GTK_WIDGET_CLASS (cmpack_graph_view_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_graph_view_key_release (GtkWidget *widget, GdkEventKey *event)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	if (view->mouse_mode)
		return TRUE;

	return GTK_WIDGET_CLASS (cmpack_graph_view_parent_class)->key_press_event (widget, event);
}

static gboolean cmpack_graph_view_scroll(GtkWidget *widget, GdkEventScroll *event)
{
	gboolean dirty = false;
	CmpackGraphView *view = CMPACK_GRAPH_VIEW (widget);

	switch (event->direction)
	{
	case GDK_SCROLL_UP:
		// Center + increase zoom
		if (view->mouse_ctrl) {
			if ((event->state & GDK_CONTROL_MASK)==0 && (view->x.ZoomPos < view->x.ZoomMax))
				dirty = set_x_axis(view, view->x.ZoomPos + 2.0, view->x.ProjPos);	
			if (view->y.ZoomPos < view->y.ZoomMax) 
				dirty |= set_y_axis(view, view->y.ZoomPos + 2.0, view->y.ProjPos); 
			if (dirty) {
				invalidate(view);
				if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
					g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
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
					g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
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
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(data);

	if (row>=0 && row<=view->item_count) {
		if (view->item_count >= view->item_capacity) {
			view->item_capacity += 64;
			view->items = (CmpackGraphViewItem*)g_realloc(view->items, view->item_capacity*sizeof(CmpackGraphViewItem));
		}
		if (row < view->item_count)
			memmove(view->items+(row+1), view->items+row, (view->item_count-row)*sizeof(CmpackGraphViewItem));
		memset(view->items + row, 0, sizeof(CmpackGraphViewItem));
		update_item(view, view->items + row, model, row);
		view->item_count++;
		invalidate(view);
	}
}

static void row_updated(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(data);

	if (row>=0 && row<view->item_count) {
		update_item(view, view->items + row, model, row);
		invalidate(view);
	}
} 

static void row_deleted(CmpackGraphData *model, gint row, gpointer data)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(data);

	if (row>=0 && row<view->item_count) {
		if (row < view->item_count-1)
			memmove(view->items+row, view->items+(row+1), (view->item_count-row-1)*sizeof(CmpackGraphViewItem));
		if (row == view->focused_item)
			view->focused_item = -1;
		view->item_count--;
		invalidate(view);
	}
} 

static void data_cleared(CmpackGraphData *model, gpointer data)
{
	CmpackGraphView *view = CMPACK_GRAPH_VIEW(data);

	clear_data(view);
	invalidate(view);
} 

static void cmpack_graph_view_enter_mouse_mode(CmpackGraphView  *view,
				   gint          x,
				   gint          y, 
				   CmpackGraphMouseMode mode,
				   gint			param)
{
	gint i;

	if (!view->mouse_mode) {
		GtkWidget *widget  = GTK_WIDGET(view);
		// Remeber which items were selected before action
		if (mode == GRAPH_MOUSE_SELECT) {
			for (i=0; i<view->item_count; i++) {
				CmpackGraphViewItem *item = view->items + i;
				item->selected_before_rubberbanding = item->selected;
			}
		}
		if (view->focused_item)
			view->focused_item = -1;
		// Store first reference point
		view->mouse_mode = mode;
		view->mouse_param = param;
		view->mouse_x2 = view->mouse_x1 = x;
		view->mouse_y2 = view->mouse_y1 = y;
		view->mouse_posx = view->x.ProjPos;
		view->mouse_posy = view->y.ProjPos;
		// Update mouse cursor
		GdkCursorType cursor = cmpack_graph_choose_mouse_cursor(view, x, y);
		if (view->mouse_cursor != cursor) {
			if (cursor!=GDK_LAST_CURSOR) {
				GdkCursor *c = gdk_cursor_new_for_display(gtk_widget_get_display(widget), cursor);
				gdk_window_set_cursor(widget->window, c);
				if (c)
					gdk_cursor_unref(c);
			} else {
				gdk_window_set_cursor(widget->window, NULL);
			}
			view->mouse_cursor = cursor;
		}
		// Ask for all mouse events
		gtk_grab_add (GTK_WIDGET (view));
	}
}

static void cmpack_graph_view_leave_mouse_mode(CmpackGraphView *view)
{
	if (view->mouse_mode) {
		view->mouse_mode = GRAPH_MOUSE_NONE;

		// Update mouse cursor
		GdkCursorType cursor = cmpack_graph_choose_mouse_cursor(view, view->last_mouse_x, view->last_mouse_y);
		if (view->mouse_cursor != cursor) {
			GtkWidget *widget = GTK_WIDGET(view);
			if (cursor!=GDK_LAST_CURSOR) {
				GdkCursor *c = gdk_cursor_new_for_display(gtk_widget_get_display(widget), cursor);
				gdk_window_set_cursor(widget->window, c);
				if (c)
					gdk_cursor_unref(c);
			} else {
				gdk_window_set_cursor(widget->window, NULL);
			}
			view->mouse_cursor = cursor;
		}
		gtk_grab_remove (GTK_WIDGET (view));
		gtk_widget_queue_draw (GTK_WIDGET (view));
	}
}

static GdkCursorType cmpack_graph_choose_mouse_cursor(CmpackGraphView *view, gint x, gint y)
{
	switch (view->mouse_mode)
	{
	case GRAPH_MOUSE_SELECT:
		return GDK_TCROSS;
	case GRAPH_MOUSE_ZOOM:
		return GDK_SIZING;
	case GRAPH_MOUSE_SHIFT:
		return GDK_LAST_CURSOR;
	case GRAPH_MOUSE_MOVE_CX:
		return GDK_RIGHT_SIDE;
	case GRAPH_MOUSE_MOVE_CY:
		return GDK_BOTTOM_SIDE;
	default:
		if (y >= view->canvas_rc.y && y < view->canvas_rc.y + view->canvas_rc.height) {
			for (int i=view->x.CursorCount-1; i>=0; i--) {
				gint cx = (int)(xproj_to_view(view, view->x.Cursor[i].xproj)+0.5);
				if (abs(x-cx) <= CLICK_TOLERANCE) 
					return GDK_RIGHT_SIDE;
			}
		}
		if (x >= view->canvas_rc.x && x < view->canvas_rc.x + view->canvas_rc.width) {
			for (int i=view->y.CursorCount-1; i>=0; i--) {
				gint cy = (int)(yproj_to_view(view, view->y.Cursor[i].xproj)+0.5);
				if (abs(y-cy) <= CLICK_TOLERANCE) 
					return GDK_BOTTOM_SIDE;
			}
		}
		if (view->focused_item>=0)
			return GDK_HAND1;
		break;
	}
	return GDK_LAST_CURSOR;
}

static gboolean cmpack_graph_view_unselect_all_internal (CmpackGraphView  *view)
{
	gboolean dirty = FALSE;
	gint i;

	if (view->selection_mode == GTK_SELECTION_NONE)
		return FALSE;

	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->selected) {
			item->selected = FALSE;
			dirty = TRUE;
			cmpack_graph_view_queue_draw_item(view, item);
		}
	}
	return dirty;
}

/* CmpackGraphView signals */
static void cmpack_graph_view_set_adjustments (CmpackGraphView   *view,
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
			G_CALLBACK (cmpack_graph_view_adjustment_changed), view);
		g_object_set(G_OBJECT(hadj), "lower", (gdouble)0, "page-increment", (gdouble)10, "step-increment", 
			(gdouble)1, NULL);
		need_adjust = TRUE;
    }
	if (view->vadjustment != vadj) {
		view->vadjustment = vadj;
		g_object_ref_sink (view->vadjustment);
		g_signal_connect (view->vadjustment, "value-changed",
			G_CALLBACK (cmpack_graph_view_adjustment_changed), view);
		g_object_set(G_OBJECT(vadj), "lower", (gdouble)0, "page-increment", (gdouble)10, "step-increment", 
			(gdouble)1, NULL);
		need_adjust = TRUE;
	}
	if (need_adjust)
		cmpack_graph_view_adjustment_changed (NULL, view);
}

/* Handles scroll bars */
static void cmpack_graph_view_adjustment_changed (GtkAdjustment *adjustment, CmpackGraphView *view)
{
	gdouble pos;
	gboolean dirty = FALSE;

	if (!adjustment || adjustment == view->hadjustment) {
		/* HSB */
		pos = view->x.ProjMin + view->x.PxlSize * (view->hadjustment->value + view->x.Center - view->graph_rc.x);
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
			restrict_y_to_limits(view);
			dirty = TRUE;
		}
	}
	if (dirty) {
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

static void cmpack_graph_view_queue_draw_item (CmpackGraphView *view, CmpackGraphViewItem *item)
{
	GdkRectangle rect;

	rect.x = RoundToInt(xproj_to_view(view, item->xproj)) - MARKER_SIZE/2;
	rect.y = RoundToInt(yproj_to_view(view, item->yproj)) - MARKER_SIZE/2;
	rect.width = MARKER_SIZE;
	rect.height = MARKER_SIZE;

	view->dirty = TRUE;
	gdk_window_invalidate_rect(GTK_WIDGET(view)->window, &rect, TRUE);
}

static void
cmpack_graph_view_select_item (CmpackGraphView      *view,
			   CmpackGraphViewItem  *item)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
	g_return_if_fail (item != NULL);

	if (item->selected || !item->enabled || !item->visible || view->selection_mode == GTK_SELECTION_NONE)
		return;
  
	if (view->selection_mode != GTK_SELECTION_MULTIPLE)
		cmpack_graph_view_unselect_all_internal (view);
	item->selected = TRUE;
	g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);
	cmpack_graph_view_queue_draw_item (view, item);
}


static void
cmpack_graph_view_unselect_item (CmpackGraphView      *view,
			     CmpackGraphViewItem  *item)
{
  g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
  g_return_if_fail (item != NULL);

  if (!item->selected)
    return;
  
  if (view->selection_mode == GTK_SELECTION_NONE ||
      view->selection_mode == GTK_SELECTION_BROWSE)
    return;
  
  item->selected = FALSE;

  g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);

  cmpack_graph_view_queue_draw_item (view, item);
}

/* VOID:OBJECT,OBJECT */
static void cmpack_graph_view_marshal_VOID__OBJECT_OBJECT(GClosure     *closure,
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

/* VOID:INT,INT */
static void cmpack_graph_view_marshal_VOID__INT_INT (GClosure     *closure,
                               GValue       *return_value G_GNUC_UNUSED,
                               guint         n_param_values,
                               const GValue *param_values,
                               gpointer      invocation_hint G_GNUC_UNUSED,
                               gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__INT_INT) (gpointer     data1,
                                           gint         arg_1,
										   gint			arg_2,
                                           gpointer     data2);
  register GMarshalFunc_VOID__INT_INT callback;
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
  callback = (GMarshalFunc_VOID__INT_INT) (marshal_data ? marshal_data : cc->callback);
  callback (data1, g_value_get_int (param_values + 1), g_value_get_int (param_values + 2), data2);
}
 
static void clear_data(CmpackGraphView *view)
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

static void update_item(CmpackGraphView *view, CmpackGraphViewItem *item, CmpackGraphData *model, gint row)
{
	item->visible = FALSE;
	if (model) {
		const CmpackGraphItem *data = cmpack_graph_data_get_item(model, row);
		if (data->x >= view->x.Min && data->x <= view->x.Max &&
			data->y >= view->y.Min && data->y <= view->y.Max) {
				item->visible = !data->hidden;
				item->xproj = x_to_proj(view, data->x);
				item->yproj = y_to_proj(view, data->y);
				item->error = data->error;
				item->color = data->color;
				item->enabled = !data->disabled;
				item->topmost = data->topmost;
				if (data->tag) {
					if (!item->tag_text || strcmp(data->tag, item->tag_text)!=0) {
						g_free(item->tag_text);
						item->tag_text = g_strdup(data->tag);
					}
				} else {
					if (item->tag_text) {
						g_free(item->tag_text);
						item->tag_text = NULL;
					}
				}
		}
	}
}

static gint find_item(CmpackGraphView *view, gint view_x, gint view_y)
{
	gint i, x, y, r2, r2min = CLICK_TOLERANCE * CLICK_TOLERANCE, index = -1;

	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible) {
			x = RoundToInt(xproj_to_view(view, item->xproj));
			y = RoundToInt(yproj_to_view(view, item->yproj));
			if (x - MARKER_SIZE/2 >= view->canvas_rc.x && x + MARKER_SIZE/2 <= view->canvas_rc.x + view->canvas_rc.width &&
				y - MARKER_SIZE/2 >= view->canvas_rc.y && y + MARKER_SIZE/2 <= view->canvas_rc.y + view->canvas_rc.height) {
					r2 = (x-view_x)*(x-view_x) + (y-view_y)*(y-view_y);
					if (r2 < r2min) {
						index = i;
						r2min = r2;
					}
			}
		}
	}
	return index;
}

static void update_data(CmpackGraphView *view)
{
	gint i;

	if (view->model && view->items) {
		for (i=0; i<view->item_count; i++)
			update_item(view, view->items+i, view->model, i);
	}
}

static void rebuild_data(CmpackGraphView *view)
{
	gint nrows;

	clear_data(view);

	if (view->model) {
		nrows = cmpack_graph_data_nrows(view->model);
		if (view->model && nrows>0) {
			view->item_count = view->item_capacity = nrows;
			view->items = (CmpackGraphViewItem*)g_malloc0(view->item_capacity*sizeof(CmpackGraphViewItem));
			update_data(view);
		}
	}
}

static gboolean update_caption(gchar **buffer, const gchar *name)
{
	if (name && name[0]!='\0') {
		if (*buffer) {
			if (strcmp(name, *buffer)!=0) {
				g_free(*buffer);
				*buffer = g_strdup(name);
				return TRUE;
			}
		} else {
			*buffer = g_strdup(name);
			return TRUE;
		}
	} else {
		if (*buffer) {
			g_free(*buffer);
			*buffer = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

static void invalidate(CmpackGraphView *view)
{
	view->dirty = TRUE;
	gtk_widget_queue_draw(GTK_WIDGET(view));
}

static gboolean set_x_axis(CmpackGraphView *view, gdouble zoom, gdouble center)
{
	zoom = LimitDbl(zoom, 0.0, view->x.ZoomMax);
	center = LimitDbl(center, view->x.ProjMin, view->x.ProjMax);
	if (zoom!=view->x.ZoomPos || center!=view->x.ProjPos) {
		view->x.ZoomPos = zoom;
		view->x.ProjPos = center;
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_hsb(view);
		return TRUE;
	}
	return FALSE;
}

static gboolean set_auto_zoom_x(CmpackGraphView *view)
{
	gint i, first;
    gdouble zoom, xmin, xmax, a, b;

	xmin = xmax = 0.0;
	first = 1;
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = &view->items[i];
		if (item->visible) {
			if (first) {
				xmin = xmax = item->xproj;
				first = 0;
			} else {
				if (item->xproj < xmin)
					xmin = item->xproj;
				if (item->xproj > xmax)
					xmax = item->xproj;
			}
		}
	}
	if (first)
		return FALSE;

	xmin = LimitDbl(xmin, view->x.ProjMin, view->x.ProjMax);
	xmax = LimitDbl(xmax, view->x.ProjMin, view->x.ProjMax);
	if ((xmax-xmin)<view->x.ProjEps) {
		a = (xmax + xmin - view->x.ProjEps)/2.0;
		b = (xmax + xmin + view->x.ProjEps)/2.0;
	} else {
		a = xmin;
		b = xmax;
    }
	zoom = log(fabs(view->x.ProjMax-view->x.ProjMin)/fabs(xmax-xmin))/log(view->zoom_base);
	return set_x_axis(view, zoom, 0.5*(a+b));
}

static gboolean set_y_axis(CmpackGraphView *view, gdouble zoom, gdouble center)
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

static gboolean set_auto_zoom_y(CmpackGraphView *view)
{
	gint i, first;
	double f, a, b, zoom, xmin, xmax, ymin, ymax;

	ymin = ymax = 0.0;
	xmin = view_to_xproj(view, view->canvas_rc.x);
	xmax = view_to_xproj(view, view->canvas_rc.x + view->canvas_rc.width);
	first = 1;
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = &view->items[i];
		if (item->visible && item->xproj>=xmin && item->xproj<=xmax) {
			if (first) {
				ymin = ymax = item->yproj;
				first = 0;
			} else {
				if (item->yproj < ymin)
					ymin = item->yproj;
				if (item->yproj > ymax)
					ymax = item->yproj;
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
GtkWidget *cmpack_graph_view_new(void)
{
	return cmpack_graph_view_new_with_model(NULL);
}

/* Create a new graph with specified model */
GtkWidget *cmpack_graph_view_new_with_model(CmpackGraphData *model)
{
  return (GtkWidget*)g_object_new(CMPACK_TYPE_GRAPH_VIEW, "model", model, NULL);
}

/* Call custom function for all selected items */
void cmpack_graph_view_selected_foreach (CmpackGraphView *view, CmpackGraphViewForeachFunc func, gpointer data)
{
	gint i;

	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
  
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->selected)
			(*func)(view, i, data);
	}
}

/* Set selection mode */
void cmpack_graph_view_set_selection_mode (CmpackGraphView *view, GtkSelectionMode mode)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	if (mode == view->selection_mode)
		return;
  
	if (mode == GTK_SELECTION_NONE || view->selection_mode == GTK_SELECTION_MULTIPLE)
		cmpack_graph_view_unselect_all (view);

	view->selection_mode = mode;
	if (!view->selection_mode && !view->activation_mode && view->focused_item>=0) {
		invalidate_cross_bar(view);
		view->focused_item = -1;
	}

	g_object_notify(G_OBJECT (view), "selection-mode");
}

/* Get selection mode */
GtkSelectionMode cmpack_graph_view_get_selection_mode (CmpackGraphView *view)
{
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), GTK_SELECTION_SINGLE);
	return view->selection_mode;
}

/* Set activation mode */
void cmpack_graph_view_set_activation_mode(CmpackGraphView *view, CmpackActivationMode mode)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	if (mode == view->activation_mode)
		return;

	view->activation_mode = mode;
	if (!view->selection_mode && !view->activation_mode && view->focused_item>=0) {
		invalidate_cross_bar(view);
		view->focused_item = -1;
	}
}

/* Enable/disable mouse control */
void cmpack_graph_view_set_mouse_control(CmpackGraphView *view, gboolean enable)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	view->mouse_ctrl = enable;
}

/* Set data model */
void cmpack_graph_view_set_model (CmpackGraphView *view, CmpackGraphData *model)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
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

/* Get data model */
CmpackGraphData *cmpack_graph_view_get_model (CmpackGraphView *view)
{
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), NULL);
	
	return view->model;
}

/* Add specified item to selection */
void cmpack_graph_view_select(CmpackGraphView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	cmpack_graph_view_select_item(view, view->items + row);
}

/* Remove item from selection */
void cmpack_graph_view_unselect(CmpackGraphView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);
	
	cmpack_graph_view_unselect_item(view, view->items + row);
}

/* 
 * cmpack_graph_view_get_selected_items:
 * @view: A #CmpackGraphView.
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
GList *cmpack_graph_view_get_selected_rows(CmpackGraphView *view)
{
	gint i;
	GList *selected = NULL;
  
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), NULL);
  
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && item->selected) 
			selected = g_list_prepend (selected, (gpointer)(intptr_t)i);
	}
	return selected;
}

gint cmpack_graph_view_get_selected_count(CmpackGraphView *view)
{
	gint i, count = 0;
  
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), 0);
  
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && item->selected) 
			count++;
	}
	return count;
}

gint cmpack_graph_view_get_selected(CmpackGraphView *view)
{
	gint i;
  
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), -1);
  
	for (i=0; i<view->item_count; i++) {
		CmpackGraphViewItem *item = view->items + i;
		if (item->visible && item->selected) 
			return i;
	}
	return -1;
}

gint cmpack_graph_view_get_focused(CmpackGraphView *view)
{
	g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), -1);

	return view->focused_item;
}

/**
 * cmpack_graph_view_select_all:
 * @view: A #CmpackGraphView.
 * 
 * Selects all the icons. @view must has its selection mode set
 * to #GTK_SELECTION_MULTIPLE.
 *
 * Since: 2.6
 **/
void cmpack_graph_view_select_all (CmpackGraphView *view)
{
	gint i;
	gboolean dirty = FALSE;
  
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	if (view->selection_mode != GTK_SELECTION_MULTIPLE)
		return;

	for (i=0; i<view->item_count; i++) {
      CmpackGraphViewItem *item = view->items + i;
      if (!item->selected) {
		  dirty = TRUE;
		  item->selected = TRUE;
		  cmpack_graph_view_queue_draw_item (view, item);
		}
	}

	if (dirty)
		g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);
}

/**
 * cmpack_graph_view_unselect_all:
 * @view: A #CmpackGraphView.
 * 
 * Unselects all the icons.
 *
 * Since: 2.6
 **/
void
cmpack_graph_view_unselect_all (CmpackGraphView *view)
{
  gboolean dirty = FALSE;
  
  g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

  if (view->selection_mode == GTK_SELECTION_BROWSE)
    return;

  dirty = cmpack_graph_view_unselect_all_internal (view);

  if (dirty)
    g_signal_emit (view, graph_view_signals[SELECTION_CHANGED], 0);
}

/**
 * cmpack_graph_view_path_is_selected:
 * @view: A #CmpackGraphView.
 * @path: A #GtkTreePath to check selection on.
 * 
 * Returns %TRUE if the icon pointed to by @path is currently
 * selected. If @path does not point to a valid location, %FALSE is returned.
 * 
 * Return value: %TRUE if @path is selected.
 *
 * Since: 2.6
 **/
gboolean cmpack_graph_view_is_selected (CmpackGraphView *view, gint row)
{
  g_return_val_if_fail (CMPACK_IS_GRAPH_VIEW (view), FALSE);
  g_return_val_if_fail (view->model != NULL, FALSE);
  g_return_val_if_fail (row>=0 && row<view->item_count, FALSE);

  return view->items[row].selected;
}

/**
 * cmpack_graph_view_item_activated:
 * @view: A #CmpackGraphView
 * @path: The #GtkTreePath to be activated
 * 
 * Activates the item determined by @path.
 *
 * Since: 2.6
 **/
void cmpack_graph_view_item_activate(CmpackGraphView *view, gint row)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));
	g_return_if_fail (row>=0 && row<view->item_count);

	g_signal_emit (view, graph_view_signals[ITEM_ACTIVATED], 0, row);
}

/* Set mapping parameters */
void cmpack_graph_view_set_x_axis(CmpackGraphView *view, gboolean log_scale, 
	gboolean reverse, gdouble min, gdouble max, gdouble eps, CmpackGraphFormat format, 
	gint minprec, gint maxprec, const gchar *caption)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	view->x.Log = log_scale;
	view->x.Reverse = reverse;
	view->x.Format = format;
	view->x.MinPrec = LimitInt(minprec, 0, 16);
	view->x.MaxPrec = LimitInt(maxprec, minprec, 16);
	update_caption(&view->x.Name, caption);
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
    view->x.ZoomMax = log((view->x.ProjMax-view->x.ProjMin)/view->x.ProjEps) / log(view->zoom_base);
    view->x.ZoomMax = MAX(0.0, view->x.ZoomMax);
	view->x.ZoomPos = LimitDbl(view->x.ZoomPos, 0.0, view->x.ZoomMax);
	for (int i=0; i<view->x.CursorCount; i++) {
		CmpackGraphCursor *item = view->x.Cursor+i;
		item->xproj = LimitDbl(item->xproj, view->x.ProjMin, view->x.ProjMax);
	}

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
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

/* Set mapping parameters */
void cmpack_graph_view_set_y_axis(CmpackGraphView *view, gboolean log_scale, 
	gboolean reverse, gdouble min, gdouble max, gdouble eps, CmpackGraphFormat format, 
	gint minprec, gint maxprec, const gchar *caption)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	view->y.Log = log_scale;
	view->y.Reverse = reverse;
	view->y.Format = format;
	view->y.MinPrec = LimitInt(minprec, 0, 16);
	view->y.MaxPrec = LimitInt(maxprec, minprec, 16);
	update_caption(&view->y.Name, caption);
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
	for (int i=0; i<view->y.CursorCount; i++) {
		CmpackGraphCursor *item = view->y.Cursor+i;
		item->xproj = LimitDbl(item->xproj, view->y.ProjMin, view->y.ProjMax);
	}

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
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

/* Set labels */
void cmpack_graph_view_set_scales(CmpackGraphView *view, gboolean x_axis, gboolean y_axis)
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
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
	}
} 

void cmpack_graph_view_set_grid(CmpackGraphView *view, gboolean x_axis, gboolean y_axis)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	if (view->x.ShowGrid != x_axis || view->y.ShowGrid != y_axis) {
		view->x.ShowGrid = x_axis;
		view->y.ShowGrid = y_axis;
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) 
			invalidate(view);
	}
}

void cmpack_graph_view_set_error_bars(CmpackGraphView *view, gboolean visible)
{
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	if (view->error_bars != visible) {
		view->error_bars = visible;
		if (GTK_WIDGET_REALIZED(GTK_WIDGET(view))) 
			invalidate(view);
	}
}


void cmpack_graph_view_set_zoom(CmpackGraphView *view, CmpackGraphAxis axis, gdouble zoom)
{
	gboolean dirty;
	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	switch (axis) 
	{
	case CMPACK_AXIS_X:
		dirty = set_x_axis(view, zoom, view->x.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
		break;
	case CMPACK_AXIS_Y:
		dirty = set_y_axis(view, zoom, view->y.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_graph_view_get_zoom(CmpackGraphView *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW (view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		return view->x.ZoomPos;
	case CMPACK_AXIS_Y:
		return view->y.ZoomPos;
	}
	return 0.0;
}

void cmpack_graph_view_set_viewfield(CmpackGraphView *view, CmpackGraphAxis axis, gdouble viewfield)
{
	gboolean dirty;
	gdouble zoom;

	g_return_if_fail (CMPACK_IS_GRAPH_VIEW (view));

	switch (axis)
	{
	case CMPACK_AXIS_X:
		if (viewfield>0.0) 
			zoom = log((view->x.ProjMax-view->x.ProjMin)/viewfield) / log(view->zoom_base);
		else
			zoom = view->x.ZoomMax;
		dirty = set_x_axis(view, zoom, view->x.ProjPos);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
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
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_graph_view_get_viewfield(CmpackGraphView *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW(view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		return view->x.PxlSize * view->graph_rc.width;
	case CMPACK_AXIS_Y:
		return view->y.PxlSize * view->graph_rc.height;
	}
	return 0.0;
}

void cmpack_graph_view_set_center(CmpackGraphView *view, CmpackGraphAxis axis, gdouble center)
{
	gboolean dirty;

	g_return_if_fail(CMPACK_IS_GRAPH_VIEW (view));

	switch (axis) 
	{
	case CMPACK_AXIS_X:
		if (view->x.Log)
			center = log10(MAX(DBL_MIN, center));
		dirty = set_x_axis(view, view->x.ZoomPos, center);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
		break;
	case CMPACK_AXIS_Y:
		if (view->y.Log)
			center = log10(MAX(DBL_MIN, center));
		dirty = set_y_axis(view, view->y.ProjPos, center);
		if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
			invalidate(view);
			if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
				g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		}
		break;
	}
}

gdouble cmpack_graph_view_get_center(CmpackGraphView *view, CmpackGraphAxis axis)
{
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW(view), 0.0);

	switch (axis)
	{
	case CMPACK_AXIS_X:
		if (!view->x.Log)
			return view->x.ProjPos;
		else
			return pow(10, view->x.ProjPos);
	case CMPACK_AXIS_Y:
		if (!view->y.Log)
			return view->y.ProjPos;
		else
			return pow(10, view->y.ProjPos);
	}
	return 0.0;
}

void cmpack_graph_view_auto_zoom(CmpackGraphView *view, gboolean x_axis, gboolean y_axis)
{
	gboolean dirty = FALSE;
	g_return_if_fail(CMPACK_IS_GRAPH_VIEW(view));

	if (x_axis)
		dirty |= set_auto_zoom_x(view);
	if (y_axis)
		dirty |= set_auto_zoom_y(view);

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
		invalidate(view);
	}
}

void cmpack_graph_view_reset_zoom(CmpackGraphView *view, gboolean x_axis, gboolean y_axis)
{
	gboolean dirty = FALSE;
	g_return_if_fail(CMPACK_IS_GRAPH_VIEW(view));

	if (x_axis)
		dirty |= set_x_axis(view, 0, 0.5*(view->x.ProjMin+view->x.ProjMax));
	if (y_axis)
		dirty |= set_y_axis(view, 0, 0.5*(view->y.ProjMin+view->y.ProjMax));

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

gboolean cmpack_graph_view_mouse_pos(CmpackGraphView *view, gdouble *x, gdouble *y)
{
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW(view), FALSE);

	if (view->last_mouse_x>=0 || view->last_mouse_y>=0) {
		if (x)
			*x = view_to_x(view, view->last_mouse_x);
		if (y)
			*y = view_to_y(view, view->last_mouse_y);
		return TRUE;
	}
	return FALSE;
}

static void render_to_target(CmpackGraphView *view, GtkWidget *widget, GdkDrawable *target)
{
	// Background
	gdk_draw_rectangle(target, widget->style->base_gc[widget->state], 
		TRUE, 0, 0, widget->allocation.width, widget->allocation.height);

	// Frame
	gdk_draw_rectangle(target, widget->style->fg_gc[widget->state], FALSE, 
		view->canvas_rc.x, view->canvas_rc.y, view->canvas_rc.width, view->canvas_rc.height);

	// Grid
	if (view->x.ShowGrid)
		paint_x_grid(view, widget, target);
	if (view->y.ShowGrid)
		paint_y_grid(view, widget, target);

	// Data
	if (view->item_count>0)
		paint_data(view, widget, target);
		
	// Labels
	if (view->x.ShowLabels)
		paint_x_scale(view, widget, target);
	if (view->y.ShowLabels)
		paint_y_scale(view, widget, target);
} 

/* Export chart to a JPEG file */
gboolean cmpack_graph_view_write_to_file(CmpackGraphView *view, const gchar *filepath, 
	const gchar *format)
{
	gboolean res;
	
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW(view), FALSE);
	g_return_val_if_fail(filepath != NULL, FALSE);
	g_return_val_if_fail(format != NULL, FALSE);

	GtkWidget *widget = GTK_WIDGET(view);
	int width = widget->allocation.width, height = widget->allocation.height;
	
	GdkPixmap *img = gdk_pixmap_new(NULL, width, height, 24);
	render_to_target(view, GTK_WIDGET(view), img);
	gdk_flush();

	GdkPixbuf *buf = gdk_pixbuf_get_from_drawable(NULL, img, gdk_colormap_get_system(), 
		0, 0, 0, 0, width, height);
	if (strcmp(format, "image/png")==0)
		res = gdk_pixbuf_save(buf, filepath, "png", NULL, NULL);
	else
		res = FALSE;
	g_object_unref(buf);
	gdk_pixmap_unref(img);
	return res;
}

/* Define/remove cursors */
void cmpack_graph_view_set_cursors(CmpackGraphView *view, CmpackGraphAxis axis, gint count)
{
	gboolean dirty = FALSE;

	g_return_if_fail(CMPACK_IS_GRAPH_VIEW(view));
	g_return_if_fail(axis == CMPACK_AXIS_X || axis == CMPACK_AXIS_Y);
	g_return_if_fail(count>=0 && count<=100);

	CmpackGraphViewAxis *a = (axis==CMPACK_AXIS_X ? &view->x : &view->y);
	if (a->CursorCount != count) {
		for (int i=count; i<a->CursorCount; i++) 
			g_free(a->Cursor[i].caption);
		if (count >= a->CursorSpace) {
			a->CursorSpace = count;
			a->Cursor = (CmpackGraphCursor*)g_realloc(a->Cursor, a->CursorSpace*sizeof(CmpackGraphCursor));
		}
		for (int i=a->CursorCount; i<count; i++) {
			a->Cursor[i].caption = NULL;
			a->Cursor[i].xproj = a->ProjMin;
			a->Cursor[i].width = (axis == CMPACK_AXIS_X ? CURSOR_SIZE : 8);
			a->Cursor[i].height = (axis == CMPACK_AXIS_X ? 8 : CURSOR_SIZE);
		}
		a->CursorCount = count;
		if (axis==CMPACK_AXIS_X) {
			if (view->mouse_mode==GRAPH_MOUSE_MOVE_CX)
				cmpack_graph_view_leave_mouse_mode(view);
		} else {
			if (view->mouse_mode==GRAPH_MOUSE_MOVE_CY)
				cmpack_graph_view_leave_mouse_mode(view);
		}
		dirty = TRUE;
	}

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

/* Set cursor position */
void cmpack_graph_view_set_cursor_pos(CmpackGraphView *view, CmpackGraphAxis axis, 
	gint cursor, gdouble pos)
{
	gboolean dirty = FALSE;

	g_return_if_fail(CMPACK_IS_GRAPH_VIEW(view));
	g_return_if_fail(axis == CMPACK_AXIS_X || axis == CMPACK_AXIS_Y);

	gdouble proj = (axis==CMPACK_AXIS_X ? x_to_proj(view, pos) : y_to_proj(view, pos));

	CmpackGraphViewAxis *a = (axis==CMPACK_AXIS_X ? &view->x : &view->y);
	g_return_if_fail(cursor>=0 && cursor<a->CursorCount);
	if (proj != a->Cursor[cursor].xproj) {
		a->Cursor[cursor].xproj = proj;
		dirty = TRUE;
	}
	if (axis==CMPACK_AXIS_X) {
		if (view->mouse_mode==GRAPH_MOUSE_MOVE_CX && view->mouse_param==cursor)
			cmpack_graph_view_leave_mouse_mode(view);
	} else {
		if (view->mouse_mode==GRAPH_MOUSE_MOVE_CY && view->mouse_param==cursor)
			cmpack_graph_view_leave_mouse_mode(view);
	}
	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view)))
		gtk_widget_queue_draw(GTK_WIDGET(view));
}

/* Set cursor caption */
void cmpack_graph_view_set_cursor_caption(CmpackGraphView *view, CmpackGraphAxis axis, 
	gint cursor, const gchar *caption)
{
	g_return_if_fail(CMPACK_IS_GRAPH_VIEW(view));
	g_return_if_fail(axis == CMPACK_AXIS_X || axis == CMPACK_AXIS_Y);

	CmpackGraphViewAxis *a = (axis==CMPACK_AXIS_X ? &view->x : &view->y);
	g_return_if_fail(cursor>=0 && cursor<a->CursorCount);

	gboolean dirty = update_caption(&a->Cursor[cursor].caption, caption);

	if (dirty && GTK_WIDGET_REALIZED(GTK_WIDGET(view))) {
		update_rectangles(view);
		update_x_pxlsize(view);
		restrict_x_to_limits(view);
		update_y_pxlsize(view);
		restrict_y_to_limits(view);
		update_hsb(view);
		update_vsb(view);
		invalidate(view);
		if (view->last_mouse_x>=0 && view->last_mouse_y>=0) 
			g_signal_emit(view, graph_view_signals[MOUSE_MOVED], 0);
	}
}

/* Get cursor position */
gdouble cmpack_graph_view_get_cursor_pos(CmpackGraphView *view, CmpackGraphAxis axis, gint cursor)
{
	g_return_val_if_fail(CMPACK_IS_GRAPH_VIEW(view), 0);
	g_return_val_if_fail(axis == CMPACK_AXIS_X || axis == CMPACK_AXIS_Y, 0);

	CmpackGraphViewAxis *a = (axis==CMPACK_AXIS_X ? &view->x : &view->y);
	g_return_val_if_fail(cursor>=0 && cursor<a->CursorCount, 0);

	if (axis==CMPACK_AXIS_X) 
		return proj_to_x(view, a->Cursor[cursor].xproj);
	else
		return proj_to_y(view, a->Cursor[cursor].xproj);
}

gboolean jdtime(double jd, struct tm* tm)
{
    int time, z, e, c, a, b, x;

	/* Check Julian date */
	memset(tm, 0, sizeof(struct tm));
	if (jd<=0.0) 
		return FALSE;

    /* Compute citizen date: year, month and day from input Julian date. */
    /* Only for JD>0! Tested for YEAR>0 except 1582-10-07/15. */
    z = (int)(jd+0.5);
    if (z>=2299163) {
	    int alpha = (int)((z-1867216.25)/36524.25);
	    a = z + 1 + alpha - (alpha/4);
    } else {
	    a = z;
    }
    b = a + 1524;
    c = (int)((b-122.1)/365.25);
    x = (int)(365.25*c);
    e = (int)((b-x)/30.6001);
	tm->tm_mday = b-x-(int)(30.6001*e);					/* Day in month (1-31) */
	tm->tm_mon  = (e<=13 ? e-2 : e-14);					/* Months since January (0-11) */
	tm->tm_year = (tm->tm_mon>=2 ? c-6616 : c-6615);		/* Years since 1900 */

    /* Time in hours, minutes and seconds */
    time = (int)((jd+0.5-z)*86400.0+0.5);
	tm->tm_hour = (time/3600);							/* Hours since midnight (0-23) */
	tm->tm_min  = (time/60)%60;							/* Minutes after the hour (0-59) */
	tm->tm_sec  = time%60;								/* Seconds after the minute (0-59) */
	return TRUE;
}

/* Converts citizen date to Julian date */
gdouble timejd(const struct tm *tm)
{
    double jd1;
	int before, y = tm->tm_year+1900, m = tm->tm_mon+1, d = tm->tm_mday, d1, d2;

	/* Check date and time */
	if (d<=0 || m<=0 || y<=0)
		return 0.0;

    /* Compute Julian date from input citizen year, month and day. */
    /* Tested for YEAR>0 except 1582-10-07/15 */
    if (y > 1582) {
	    before = 0;
    } else if (y < 1582) {
	    before = 1;
    } else
	if (m > 10) {
	    before = 0;
	} else if (m < 10) {
	    before = 1;
	} else
	if (d >= 15) {
		before = 0;
	} else {
	    before = 1;
	}
    if (m <= 2) {
		d1 = (int)(365.25*(y-1));
		d2 = (int)(30.6001*(m+13));
	} else {
		d1 = (int)(365.25*(y));
		d2 = (int)(30.6001*(m+1));
	}
	jd1 = 1720994.5 + d1 + d2 + d + tm->tm_hour/24.0 + tm->tm_min/1440.0 + tm->tm_sec/86400.0;
    if (!before) 
		jd1 += 2 - (y/100) + (y/400);
	return jd1;
}

/* Adjusts given date to nearest boundary between year, month, day, ...
   that is GREATER than given date */
void ceildate(struct tm *tm, gint step, tDateUnit unit)
{
	int y, ym;

	switch (unit)
	{
	case YEARS:
		y = tm->tm_year;
		if (tm->tm_mon>0 || tm->tm_mday>1 || tm->tm_hour>0 || tm->tm_min>0 || tm->tm_sec>0) {
			y++;
			tm->tm_mon = tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
			tm->tm_mday = 1;
		}
		y = (y+step-1)/step*step;
		tm->tm_year = y;
		break;

	case MONTHS:
		ym = (tm->tm_year)*12 + tm->tm_mon;
		if (tm->tm_mday>1 || tm->tm_hour>0 || tm->tm_min>0 || tm->tm_sec>0) {
			ym++;
			tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
			tm->tm_mday = 1;
		}
		ym = (ym+step-1)/step*step;
		tm->tm_year = ym/12;
		tm->tm_mon  = ym%12;
		break;

	default:
		break;
	}
}
