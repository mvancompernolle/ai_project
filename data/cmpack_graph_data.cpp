/**************************************************************

cmpack_graph_data.cpp (C-Munipack project)
Object which holds data for a graph
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

$Id: cmpack_graph_data.cpp,v 1.1.1.1 2012/08/12 17:00:32 dmotl Exp $

**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmpack_graph_data.h"

enum {
	ROW_INSERTED, 
	ROW_UPDATED,
	ROW_DELETED,
	DATA_CLEARED,
	LAST_SIGNAL
}; 

/*--------------------   GRAPH DATA CLASS   ----------------------------*/

G_DEFINE_TYPE(CmpackGraphData, cmpack_graph_data, G_TYPE_OBJECT)

static guint cmpack_graph_data_signals[LAST_SIGNAL] = { 0 }; 

static void cmpack_graph_data_finalize(GObject *object);

/* Class initialization */
static void cmpack_graph_data_class_init(CmpackGraphDataClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;

    /* Override object destroy */
	object_class->finalize = cmpack_graph_data_finalize;

	/* Signals */
	cmpack_graph_data_signals[ROW_INSERTED] = g_signal_new("row-inserted",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackGraphDataClass, row_inserted), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 

	cmpack_graph_data_signals[ROW_UPDATED] = g_signal_new("row-updated",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackGraphDataClass, row_updated), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 

	cmpack_graph_data_signals[ROW_DELETED] = g_signal_new("row-deleted",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackGraphDataClass, row_deleted), NULL, NULL, 
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT); 
	
	cmpack_graph_data_signals[DATA_CLEARED] = g_signal_new("data-cleared",
		  G_TYPE_FROM_CLASS(object_class), 
		  GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
		  G_STRUCT_OFFSET (CmpackGraphDataClass, data_cleared), NULL, NULL, 
		  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Instance initialization */
static void cmpack_graph_data_init(CmpackGraphData *data)
{
}

/* Create graph widget */
CmpackGraphData *cmpack_graph_data_new()
{
	return (CmpackGraphData*)g_object_new(CMPACK_TYPE_GRAPH_DATA, NULL);
}

/* Create graph widget */
CmpackGraphData *cmpack_graph_data_new_with_alloc(gint capacity)
{
	CmpackGraphData *res = cmpack_graph_data_new();
	cmpack_graph_data_alloc(res, capacity);
	return res;
}

/* Free allocated memory */
static void cmpack_graph_data_finalize(GObject *object)
{
	gint i;
	CmpackGraphData *data = CMPACK_GRAPH_DATA(object);

	/* Free allocated data */
	for (i=0; i<data->count; i++) {
		g_free(data->items[i]->tag);
		g_free(data->items[i]);
	}
	g_free(data->items);
	data->items = NULL;
	data->count = data->capacity = 0;

    /* Chain up to the parent class */
    G_OBJECT_CLASS(cmpack_graph_data_parent_class)->finalize(object);
}

/* Clear data and set number of channels and initial capacity */
void cmpack_graph_data_alloc(CmpackGraphData *data, gint capacity)
{
	g_return_if_fail(data != NULL);

	capacity = MAX(capacity, data->count);
	if (capacity != data->capacity) {
		if (capacity>0) {
			data->items = (CmpackGraphItem**)g_realloc(data->items, capacity*sizeof(CmpackGraphItem*));
		} else {
			g_free(data->items);
			data->items = NULL;
		}
		data->capacity = capacity;
	}
}

/* Clear all data */
void cmpack_graph_data_clear(CmpackGraphData *data)
{
	gboolean changed;
	gint i;
	g_return_if_fail(data != NULL);

	/* Free allocated data */
	changed = data->count>0;
	for (i=0; i<data->count; i++) {
		g_free(data->items[i]->tag);
		g_free(data->items[i]);
	}
	g_free(data->items);
	data->items = NULL;
	data->count = data->capacity = 0;

	if (changed)
		g_signal_emit(data, cmpack_graph_data_signals[DATA_CLEARED], 0);
}

/* Add new data to the graph */
gint cmpack_graph_data_add(CmpackGraphData *data, const CmpackGraphItem *d, gsize item_size)
{
	gint index;
	CmpackGraphItem *item;

	g_return_val_if_fail(data != NULL, -1);

	if (data->count>=data->capacity) {
		data->capacity += 64;
		data->items = (CmpackGraphItem**)g_realloc(data->items, data->capacity*sizeof(CmpackGraphItem*));
	}
	index = data->count++;
	
	item = (CmpackGraphItem*)g_malloc0(sizeof(CmpackGraphItem));
	memcpy(item, d, MIN(sizeof(CmpackGraphItem), item_size));
	item->tag = (d->tag!=NULL ? g_strdup(d->tag) : NULL);
	data->items[index] = item;
	
	g_signal_emit(data, cmpack_graph_data_signals[ROW_INSERTED], 0, index);
	return index;
}

/* Set row tag */
void cmpack_graph_data_set_tag(CmpackGraphData *data, gint row, const gchar *text)
{
	CmpackGraphItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (text) {
		if (!item->tag || strcmp(text, item->tag)!=0) {
			g_free(item->tag); 
			item->tag = g_strdup(text);
			g_signal_emit(data, cmpack_graph_data_signals[ROW_UPDATED], 0, row);
		}
	} else {
		if (item->tag) {
			g_free(item->tag); 
			item->tag = NULL;
			g_signal_emit(data, cmpack_graph_data_signals[ROW_UPDATED], 0, row);
		}
	}
}

/* Set topmost flag */
void cmpack_graph_data_set_topmost(CmpackGraphData *data, gint row, gboolean topmost)
{
	CmpackGraphItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->topmost != topmost) {
		data->items[row]->topmost = topmost;
		g_signal_emit(data, cmpack_graph_data_signals[ROW_UPDATED], 0, row);
	}
}

/* Set row color */
void cmpack_graph_data_set_color(CmpackGraphData *data, gint row, CmpackColor color)
{
	CmpackGraphItem *item;

	g_return_if_fail(data != NULL);
	g_return_if_fail(row>=0 && row<data->count);

	item = data->items[row];
	if (item->color!=color) {
		item->color = color;
		g_signal_emit(data, cmpack_graph_data_signals[ROW_UPDATED], 0, row);
	}
}

/* Clear all tags */
void cmpack_graph_data_clear_tags(CmpackGraphData *data)
{
	gint i;

	g_return_if_fail(data != NULL);

	for (i=0; i<data->count; i++) {
		CmpackGraphItem *item = data->items[i];
		if (item->tag) {
			g_free(item->tag); 
			item->tag = NULL;
			g_signal_emit(data, cmpack_graph_data_signals[ROW_UPDATED], 0, i);
		}
	}
}

/* Get row tag */
const gchar *cmpack_graph_data_get_tag(CmpackGraphData *data, gint row)
{
	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(row>=0 && row<data->count, NULL);

	return data->items[row]->tag;
}

/* Get row tag */
CmpackColor cmpack_graph_data_get_color(CmpackGraphData *data, gint row)
{
	g_return_val_if_fail(data != NULL, CMPACK_COLOR_DEFAULT);
	g_return_val_if_fail(row>=0 && row<data->count, CMPACK_COLOR_DEFAULT);

	return data->items[row]->color;
}

/* Get row tag */
intptr_t cmpack_graph_data_get_param(CmpackGraphData *data, gint row)
{
	g_return_val_if_fail(data != NULL, 0);
	g_return_val_if_fail(row>=0 && row<data->count, 0);

	return data->items[row]->param;
}

/* Get number of measurements */
gint cmpack_graph_data_nrows(CmpackGraphData *data)
{
	g_return_val_if_fail(data!=NULL, 0);

	return data->count; 
}

/* Get single value */
const CmpackGraphItem *cmpack_graph_data_get_item(CmpackGraphData *data, gint row)
{
	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(row>=0 && row<data->count, NULL);

	return data->items[row];
}

/* Find object by its identifier */
gint cmpack_graph_data_find_item(CmpackGraphData *data, intptr_t param)
{
	gint i;

	g_return_val_if_fail(data != NULL, -1);

	for (i=0; i<data->count; i++) {
		if (data->items[i]->param == param)
			return i;
	}
	return -1;
}
