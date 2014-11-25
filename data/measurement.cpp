/**************************************************************

measurement.cpp (C-Munipack project)
Measurement tool
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
#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "measurement.h"
#include "utils.h"

// Constructor
CMeasurementBox::CMeasurementBox(void):CInfoBox("Measurement"), m_pGraph(NULL),
	m_pTable(NULL), m_Updating(false), m_Axis(CMPACK_AXIS_X), m_ChannelX(-1), 
	m_ChannelY(-1), m_DateFormat(JULIAN_DATE), m_PosX(NULL), m_PosY(NULL), 
	m_ChannelsX(0), m_ChannelsY(0), m_PrecisionX(0), m_PrecisionY(0), 
	m_NameX(NULL), m_NameY(NULL), m_UnitX(NULL), m_UnitY(NULL), m_PosList(NULL)
{
	m_Buffer = gtk_text_buffer_new(NULL);

	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(m_Box), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Axis selection
	m_XAxisBtn = gtk_radio_button_new_with_label(NULL, NULL);
	gtk_widget_set_tooltip_text(m_XAxisBtn, "Measure in horizontal axis");
	gtk_box_pack_start(GTK_BOX(vbox), m_XAxisBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_XAxisBtn), "toggled", G_CALLBACK(toggled), this);
	GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_XAxisBtn));
	m_YAxisBtn = gtk_radio_button_new_with_label(group, NULL);
	gtk_widget_set_tooltip_text(m_YAxisBtn, "Measure in vertical axis");
	gtk_box_pack_start(GTK_BOX(vbox), m_YAxisBtn, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(m_YAxisBtn), "toggled", G_CALLBACK(toggled), this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_XAxisBtn), TRUE);

	// Cursor positions
	m_Cursor[0] = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Cursor[0]), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Cursor[0], FALSE, TRUE, 0);
	m_Cursor[1] = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Cursor[1]), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Cursor[1], FALSE, TRUE, 0);
	m_Distance = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(m_Distance), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), m_Distance, FALSE, TRUE, 0);

	// Text area
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	m_Info = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_Info), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_Info), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_Info), FALSE);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(m_Info), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(frame), m_Info);
	
	int w = text_width(m_Info, "Max. pixel = (9999, 9999)");
	gtk_widget_set_size_request(m_Info, w+24, -1);
}

// Destructor
CMeasurementBox::~CMeasurementBox(void)
{
	if (m_pGraph)
		g_object_unref(m_pGraph);
	g_object_unref(m_Buffer);
	for (GSList *ptr=m_PosList; ptr!=NULL; ptr=ptr->next) {
		if (ptr->data) 
			g_free(((tPosition*)ptr->data)->name);
		g_free(ptr->data);
	}
	g_slist_free(m_PosList);
	g_free(m_PosX);
	g_free(m_PosY);
}

// Set active axis
void CMeasurementBox::SetAxis(CmpackGraphAxis axis)
{
	if (axis!=m_Axis) {
		m_Updating = true;
		m_Axis = axis;
		if (axis == CMPACK_AXIS_X) 
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_XAxisBtn), TRUE);
		else 
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_YAxisBtn), TRUE);
		UpdateGraph();
		UpdateValues();
		UpdateStatistics();
		m_UpdateStats = false;
		m_Updating = false;
	}
}

// Set date format
void CMeasurementBox::SetDateFormat(tDateFormat datef)
{
	if (datef != m_DateFormat) {
		m_DateFormat = datef;
		UpdateLabels();
		UpdateValues();
		UpdateStatistics();
		m_UpdateStats = false;
	}
}

// Set chart view
void CMeasurementBox::SetGraphView(CmpackGraphView *pGraph)
{
	if (pGraph!=m_pGraph) {
		if (m_pGraph) {
			g_signal_handlers_disconnect_by_func(G_OBJECT(m_pGraph), (gpointer)cursor_moved, this);
			g_object_unref(m_pGraph);
			m_pGraph = NULL;
		}
		if (pGraph) {
			m_pGraph = (CmpackGraphView*)g_object_ref(pGraph);
			g_signal_connect(G_OBJECT(m_pGraph), "cursor-moved", G_CALLBACK(cursor_moved), this);
		}
		UpdateGraph();
	}
}

// Update axis labels
void CMeasurementBox::UpdateLabels(void)
{
	if (m_pTable && m_ChannelX>=0) {
		const gchar *name = m_pTable->ChannelsX()->GetName(m_ChannelX);
		if (strcmp(name, "JD")==0)
			name = (m_DateFormat==JULIAN_DATE ? "Geocentric JD" : "Geocentric UTC");
		else if (strcmp(name, "JDHEL")==0)
			name = (m_DateFormat==JULIAN_DATE ? "Heliocentric JD" : "Heliocentric UTC");
		gtk_button_set_label(GTK_BUTTON(m_XAxisBtn), name);
	} else 
		gtk_button_set_label(GTK_BUTTON(m_XAxisBtn), NULL);
	
	if (m_pTable && m_ChannelY>=0) {
		const gchar *name = m_pTable->ChannelsY()->GetName(m_ChannelY);
		gtk_button_set_label(GTK_BUTTON(m_YAxisBtn), name);
	} else
		gtk_button_set_label(GTK_BUTTON(m_YAxisBtn), NULL);
}

// Update graph view
void CMeasurementBox::UpdateGraph(void)
{
	if (m_pGraph) {
		if (m_Axis == CMPACK_AXIS_X) {
			cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_Y, 0);
			if (m_Visible && m_ChannelX>=0 && m_ChannelX<m_ChannelsX && m_PosX[m_ChannelX]) {
				double *pos = m_PosX[m_ChannelX]->pos;
				cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_X, 2);
				cmpack_graph_view_set_cursor_pos(m_pGraph, CMPACK_AXIS_X, 0, pos[0]);
				cmpack_graph_view_set_cursor_caption(m_pGraph, CMPACK_AXIS_X, 0, "1");
				cmpack_graph_view_set_cursor_pos(m_pGraph, CMPACK_AXIS_X, 1, pos[1]);
				cmpack_graph_view_set_cursor_caption(m_pGraph, CMPACK_AXIS_X, 1, "2");
			} else 
				cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_X, 0);
		} else {
			cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_X, 0);
			if (m_Visible && m_ChannelY>=0 && m_ChannelY<m_ChannelsY && m_PosY[m_ChannelY]) {
				double *pos = m_PosY[m_ChannelY]->pos;
				cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_Y, 2);
				cmpack_graph_view_set_cursor_pos(m_pGraph, CMPACK_AXIS_Y, 0, pos[0]);
				cmpack_graph_view_set_cursor_caption(m_pGraph, CMPACK_AXIS_Y, 0, "1");
				cmpack_graph_view_set_cursor_pos(m_pGraph, CMPACK_AXIS_Y, 1, pos[1]);
				cmpack_graph_view_set_cursor_caption(m_pGraph, CMPACK_AXIS_Y, 1, "2");
			} else
				cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_Y, 0);
		}
	}
}

// Set reference to a chart view
void CMeasurementBox::SetTable(CTable *pTable)
{
	if (pTable!=m_pTable) {
		m_pTable = pTable;
		m_ChannelX = m_ChannelY = -1;
		if (m_pTable) {
			// X channels
			int count = m_pTable->ChannelsX()->Count();
			if (count!=m_ChannelsX) {
				m_ChannelsX = count;
				g_free(m_PosX);
				m_PosX = (tPosition**)g_malloc(m_ChannelsX*2*sizeof(tPosition*));
			}
			for (int i=0; i<m_ChannelsX; i++) {
				CChannel *ch = m_pTable->ChannelsX()->Get(i);
				tPosition *pos = findPos(ch->Name());
				if (pos) {
					if (pos->user_defined[0])
						pos->pos[0] = LimitValue(pos->pos[0], ch->Min(), ch->Max());
					else
						pos->pos[0] = ch->Min();
					if (pos->user_defined[1])
						pos->pos[1] = LimitValue(pos->pos[1], ch->Min(), ch->Max());
					else
						pos->pos[1] = ch->Max();
				} else {
					pos = addPos(ch->Name(), ch->Min(), ch->Max());
				} 
				m_PosX[i] = pos;
			}
			m_PrecisionX = m_pTable->ChannelsX()->GetPrecision(m_ChannelX);
			m_NameX		 = m_pTable->ChannelsX()->GetName(m_ChannelX);
			m_UnitX      = m_pTable->ChannelsX()->GetUnit(m_ChannelX);
			// Y channels
			count = m_pTable->ChannelsY()->Count();
			if (count!=m_ChannelsY) {
				m_ChannelsY = count;
				g_free(m_PosY);
				m_PosY = (tPosition**)g_malloc(m_ChannelsY*2*sizeof(tPosition*));
			}
			for (int j=0; j<m_ChannelsY; j++) {
				CChannel *ch = m_pTable->ChannelsY()->Get(j);
				tPosition *pos = findPos(ch->Name());
				if (pos) {
					if (pos->user_defined[0])
						pos->pos[0] = LimitValue(pos->pos[0], ch->Min(), ch->Max());
					else
						pos->pos[0] = ch->Min();
					if (pos->user_defined[1])
						pos->pos[1] = LimitValue(pos->pos[1], ch->Min(), ch->Max());
					else
						pos->pos[1] = ch->Max();
				} else {
					pos = addPos(ch->Name(), ch->Min(), ch->Max());
				}
				m_PosY[j] = pos;
			}
			m_PrecisionY = m_pTable->ChannelsY()->GetPrecision(m_ChannelY);
			m_NameY		 = m_pTable->ChannelsY()->GetName(m_ChannelY);
			m_UnitY      = m_pTable->ChannelsY()->GetUnit(m_ChannelY);
		} else {
			g_free(m_PosX);
			m_PosX = NULL;
			m_ChannelsX = 0;
			g_free(m_PosY);
			m_PosY = NULL;
			m_ChannelsY = 0;
			m_PrecisionX = m_PrecisionY = 0;
			m_NameX = m_NameY = m_UnitX = m_UnitY = NULL;
		}
		m_UpdateStats = true;
		UpdateLabels();
		UpdateGraph();
		UpdateValues();
	}
}

// Initialization before the tool is shown
void CMeasurementBox::OnShow(void)
{
	if (!m_Timer)
		m_Timer = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 1000, GSourceFunc(timer_cb), this, NULL);
	UpdateLabels();
	UpdateGraph();
	UpdateValues();
	UpdateStatistics();
	m_UpdateStats = false;
}

// Clean up after the tool is hidden
void CMeasurementBox::OnHide(void)
{
	if (m_pGraph) {
		cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_X, 0);
		cmpack_graph_view_set_cursors(m_pGraph, CMPACK_AXIS_Y, 0);
	}
	if (m_Timer) {
		g_source_remove(m_Timer);
		m_Timer = 0;
	}
}

// Set channel
void CMeasurementBox::SetChannel(CmpackGraphAxis axis, int channel)
{
	if (axis == CMPACK_AXIS_X) {
		if (m_ChannelX != channel) {
			m_ChannelX = channel;
			if (m_pTable) {
				m_PrecisionX = m_pTable->ChannelsX()->GetPrecision(m_ChannelX);
				m_NameX      = m_pTable->ChannelsX()->GetName(m_ChannelX);
				m_UnitX      = m_pTable->ChannelsX()->GetUnit(m_ChannelX);
			}
			UpdateStatistics();
			m_UpdateStats = false;
			UpdateLabels();
			UpdateGraph();
			UpdateValues();
		}
	} else {
		if (m_ChannelY != channel) {
			m_ChannelY = channel;
			if (m_pTable) {
				m_PrecisionY = m_pTable->ChannelsY()->GetPrecision(m_ChannelY);
				m_NameY      = m_pTable->ChannelsY()->GetName(m_ChannelY);
				m_UnitY      = m_pTable->ChannelsY()->GetUnit(m_ChannelY);
			}
			UpdateStatistics();
			m_UpdateStats = false;
			UpdateLabels();
			UpdateGraph();
			UpdateValues();
		}
	}
}

void CMeasurementBox::toggled(GtkToggleButton *togglebutton, CMeasurementBox *pMe)
{
	pMe->OnToggled(togglebutton);
}

void CMeasurementBox::OnToggled(GtkToggleButton *togglebutton)
{
	if (!m_Updating) {
		m_Updating = true;
		if (togglebutton == GTK_TOGGLE_BUTTON(m_XAxisBtn)) 
			SetAxis(CMPACK_AXIS_X);
		else if (togglebutton == GTK_TOGGLE_BUTTON(m_YAxisBtn)) 
			SetAxis(CMPACK_AXIS_Y);
		m_Updating = false;
		UpdateValues();
	}
}

void CMeasurementBox::cursor_moved(CmpackGraphView *pView, CmpackGraphAxis axis, int cursor, CMeasurementBox *pMe)
{
	pMe->OnCursorMoved(pView, axis, cursor);
}

void CMeasurementBox::OnCursorMoved(CmpackGraphView *pView, CmpackGraphAxis axis, int cursor)
{
	char msg[256], buf[256];

	if (m_Visible) {
		double value = cmpack_graph_view_get_cursor_pos(pView, axis, cursor);
		if (axis == CMPACK_AXIS_X) {
			if (m_ChannelX>=0 && m_ChannelX<m_ChannelsX && cursor>=0 && cursor<2 && m_PosX[m_ChannelX]) {
				tPosition *pos = m_PosX[m_ChannelX];
				pos->pos[cursor] = value;
				pos->user_defined[cursor] = true;
				PrintValue(buf, value, m_PrecisionX, m_NameX, m_UnitX);
				sprintf(msg, "Cursor %d: %s", cursor+1, buf);
				gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), msg);
				PrintValue(buf, fabs(pos->pos[1]-pos->pos[0]), m_PrecisionX, m_NameX, m_UnitX, true);
				sprintf(msg, "Distance: %s", buf);
				gtk_label_set_text(GTK_LABEL(m_Distance), msg);
				m_UpdateStats = true;
			}
		} else {
			if (m_ChannelY>=0 && m_ChannelY<m_ChannelsY && cursor>=0 && cursor<2 && m_PosY[m_ChannelY]) {
				tPosition *pos = m_PosY[m_ChannelY];
				pos->pos[cursor] = value;
				pos->user_defined[cursor] = true;
				PrintValue(buf, value, m_PrecisionY, m_NameY, m_UnitY);
				sprintf(msg, "Cursor %d: %s", cursor+1, buf);
				gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), msg);
				PrintValue(buf, fabs(pos->pos[1]-pos->pos[0]), m_PrecisionY, m_NameY, m_UnitY, true);
				sprintf(msg, "Distance: %s", buf);
				gtk_label_set_text(GTK_LABEL(m_Distance), msg);
				m_UpdateStats = true;
			}
		}
	}
}

// Update displayed values
void CMeasurementBox::UpdateValues(void)
{
	char msg[256], buf[256];

	if (m_Visible) {
		if (m_Axis == CMPACK_AXIS_X) {
			if (m_ChannelX>=0 && m_ChannelX<m_ChannelsX && m_PosX[m_ChannelX]) {
				double *pos = m_PosX[m_ChannelX]->pos;
				for (int cursor=0; cursor<2; cursor++) {
					PrintValue(buf, pos[cursor], m_PrecisionX, m_NameX, m_UnitX);
					sprintf(msg, "Cursor %d: %s", cursor+1, buf);
					gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), msg);
				}
				PrintValue(buf, fabs(pos[1]-pos[0]), m_PrecisionX, m_NameX, m_UnitX, true);
				sprintf(msg, "Distance: %s", buf);
				gtk_label_set_text(GTK_LABEL(m_Distance), msg);
			} else {
				for (int cursor=0; cursor<2; cursor++) 
					gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), NULL);
				gtk_label_set_text(GTK_LABEL(m_Distance), NULL);
			}
		} else {
			if (m_ChannelY>=0 && m_ChannelY<m_ChannelsY && m_PosY[m_ChannelY]) {
				double *pos = m_PosY[m_ChannelY]->pos;
				for (int cursor=0; cursor<2; cursor++) {
					PrintValue(buf, pos[cursor], m_PrecisionY, m_NameY, m_UnitY);
					sprintf(msg, "Cursor %d: %s", cursor+1, buf);
					gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), msg);
				}
				PrintValue(msg, fabs(pos[1]-pos[0]), m_PrecisionY, m_NameY, m_UnitY, true);
				sprintf(msg, "Distance: %s", buf);
				gtk_label_set_text(GTK_LABEL(m_Distance), msg);
			} else {
				for (int cursor=0; cursor<2; cursor++) 
					gtk_label_set_text(GTK_LABEL(m_Cursor[cursor]), NULL);
				gtk_label_set_text(GTK_LABEL(m_Distance), NULL);
			}
		}
	}
}

gboolean CMeasurementBox::timer_cb(CMeasurementBox *pMe)
{
	pMe->OnTimer();
	return TRUE;
}

void CMeasurementBox::OnTimer(void)
{
	if (m_Visible && m_Axis==CMPACK_AXIS_X && m_UpdateStats) {
		m_UpdateStats = false;
		UpdateStatistics();
	}
}

// Update displayed values
void CMeasurementBox::UpdateStatistics(void)
{
	GtkTextIter ptr;
	char msg[1024], buf[256];

	if (!m_Visible)
		return;

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_Info), NULL);
	gtk_text_buffer_set_text(m_Buffer, "", -1);
	gtk_text_buffer_get_end_iter(m_Buffer, &ptr);
	if (m_pTable && m_ChannelX>=0 && m_ChannelX<m_ChannelsX && m_ChannelY>=0 && m_ChannelY<=m_ChannelsY) {
		if (m_Axis == CMPACK_AXIS_X && m_PosX[m_ChannelX]) {
			double *pos = m_PosX[m_ChannelX]->pos;
			double xmin = MIN(pos[0], pos[1]), xmax = MAX(pos[0], pos[1]);
			int xcol = m_pTable->ChannelsX()->GetColumn(m_ChannelX),
				ycol = m_pTable->ChannelsY()->GetColumn(m_ChannelY);
			// Compute statistics
			int count = 0, length = m_pTable->Rows();
			double x, *y = new double[length], ymin, ymax, mean, sdev;
			bool ok = m_pTable->Rewind();
			while (ok) {
				if (m_pTable->GetDbl(xcol, &x) && x>=xmin && x<=xmax && m_pTable->GetDbl(ycol, y+count))
					count++;
				ok = m_pTable->Next();
			}
			sprintf(msg, "There are %d points between cursor 1 and 2.\n\n", count);
			gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			if (ComputeMinMax(count, y, &ymin, &ymax)) {
				cmpack_robustmean(count, y, &mean, &sdev);
				sprintf(msg, "Data %s:\n", m_pTable->ChannelsY()->GetName(m_ChannelY));
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
				PrintValue(buf, ymin, m_PrecisionY, m_NameY, m_UnitY);
				sprintf(msg, "Min.: %s\n", buf);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
				PrintValue(buf, ymax, m_PrecisionY, m_NameY, m_UnitY);
				sprintf(msg, "Max.: %s\n", buf);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
				PrintValue(buf, mean, m_PrecisionY, m_NameY, m_UnitY);
				sprintf(msg, "Mean: %s\n", buf);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
				PrintValue(buf, sdev, m_PrecisionY, m_NameY, m_UnitY);
				sprintf(msg, "St.dev.: %s\n", buf);
				gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
			}
			delete[] y;
		}
	}
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_Info), m_Buffer);
}

// Print distance
void CMeasurementBox::PrintValue(gchar *buf, double value, int prec, const gchar *name, const gchar *unit,
	bool distance)
{
	if ((strcmp(name, "JD")==0 || strcmp(name, "JDHEL")==0) && m_DateFormat != JULIAN_DATE) {
		if (!distance) {
			CmpackDateTime dt;
			cmpack_decodejd(value, &dt);
			sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.day,
				dt.time.hour, dt.time.minute, dt.time.second);
		} else {
			if (value >= 99.95) 
				sprintf(buf, "%.0f days", value);
			else {
				int hours = (int)(value*24+0.5);
				if (hours>=24) {
					sprintf(buf, "%d %s %d %s", hours/24, (hours/24)==1 ? "day" : "days", 
						hours%24, (hours%24)==1 ? "hour" : "hours");
				} else {
					int minutes = (int)(value*24*60+0.5);
					if (minutes>=60) {
						sprintf(buf, "%d %s %d %s", minutes/60, (minutes/60)==1 ? "hour" : "hours",
							minutes%60, (minutes%60)==1 ? "minute" : "minutes");
					} else {
						int seconds = (int)(value*24*60*60+0.5);
						if (seconds>=60) {
							sprintf(buf, "%d %s %d %s", seconds/60, (seconds/60)==1 ? "minute" : "minutes",
								seconds%60, (seconds%60)==1 ? "second" : "seconds");
						} else {
							sprintf(buf, "%d %s", seconds, seconds==1 ? "second" : "seconds");
						}
					}
				}
			}
		}
	} else {
		if (unit)
			sprintf(buf, "%.*f %s", prec, value, unit);
		else
			sprintf(buf, "%.*f", prec, value);
	}
}

// Find record in PosList
CMeasurementBox::tPosition *CMeasurementBox::findPos(const gchar *name) const
{
	for (GSList *ptr=m_PosList; ptr!=NULL; ptr=ptr->next) {
		if (StrCmp0(((tPosition*)ptr->data)->name, name)==0)
			return (tPosition*)ptr->data;
	}
	return NULL;
}

// Add new record to PosList
CMeasurementBox::tPosition *CMeasurementBox::addPos(const gchar *name, double min, double max)
{
	tPosition *pos = (tPosition*)g_malloc0(sizeof(tPosition));
	pos->name = (name ? g_strdup(name) : NULL);
	pos->pos[0] = min;
	pos->pos[1] = max;
	m_PosList = g_slist_prepend(m_PosList, pos);
	return pos;
}
