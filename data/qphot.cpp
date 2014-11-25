/**************************************************************

chart.cpp (C-Munipack project)
Widget which can draw a chart
Copyright (C) 2008 David Motl, dmotl@volny.cz

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

#include "configuration.h"
#include "profile.h"
#include "main.h"
#include "qphot.h"

// Constructor
CQuickPhotBox::CQuickPhotBox(void):CInfoBox("Quick photometry"), m_pChart(NULL), 
	m_Result(RES_ERR_POSITION), m_CenterX(-1), m_CenterY(-1), m_MaxValue(0), m_ADCGain(0), 
	m_AvgFrames(0), m_LoData(0), m_HiData(0), m_ApRadius(0), m_InnerSky(0), 
	m_OuterSky(0), m_SkyMean(0), m_SkyDev(0), m_FwhmX(0), m_FwhmY(0), m_Fwhm(0), 
	m_Signal(0), m_Noise(0), m_NSky(0), m_LayerId(-1)
{
	GtkObject *adj;

	m_Buffer = gtk_text_buffer_new(NULL);
	memset(m_ObjectId, 0, 4*sizeof(int));

	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(m_Box), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Apertures
	GtkWidget *label = gtk_label_new("Aperture radius");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	adj = gtk_adjustment_new(1, 1, 100, 1, 10, 0);
	m_Aperture = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 2);
	gtk_widget_set_tooltip_text(m_Aperture, "Aperture radius in pixels");
	g_signal_connect(G_OBJECT(m_Aperture), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_Aperture), true);
	gtk_box_pack_start(GTK_BOX(vbox), m_Aperture, FALSE, TRUE, 0);

	// Sky inner radius
	label = gtk_label_new("Sky inner radius");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	adj = gtk_adjustment_new(1, 1, 100, 1, 10, 0);
	m_InSky = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 2);
	gtk_widget_set_tooltip_text(m_InSky, "Inner radius of the annulus used to measure local background");
	g_signal_connect(G_OBJECT(m_InSky), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_InSky), true);
	gtk_box_pack_start(GTK_BOX(vbox), m_InSky, FALSE, TRUE, 0);

	// Sky outer radius
	label = gtk_label_new("Sky outer radius");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	adj = gtk_adjustment_new(1, 1, 100, 1, 10, 0);
	m_OutSky = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0, 2);
	gtk_widget_set_tooltip_text(m_OutSky, "Outer radius of the annulus used to measure local background");
	g_signal_connect(G_OBJECT(m_OutSky), "value-changed", G_CALLBACK(value_changed), this);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(m_OutSky), true);
	gtk_box_pack_start(GTK_BOX(vbox), m_OutSky, FALSE, TRUE, 0);

	label = gtk_label_new("Results");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

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
CQuickPhotBox::~CQuickPhotBox(void)
{
	if (m_pChart)
		g_object_unref(m_pChart);
	g_object_unref(m_Buffer);
}

// Set chart view
void CQuickPhotBox::SetChart(CmpackChartView *pChart)
{
	if (pChart!=m_pChart) {
		if (m_pChart) {
			if (m_LayerId>=0) {
				cmpack_chart_view_delete_layer(m_pChart, m_LayerId);
				m_LayerId = -1;
			}
			g_object_unref(m_pChart);
		}
		if (pChart) 
			m_pChart = (CmpackChartView*)g_object_ref(pChart);
		else
			m_pChart = NULL;
	}
}

// Clear results
void CQuickPhotBox::Clear(void)
{
	m_Result = RES_ERR_POSITION;
	m_CenterX = m_CenterY = -1;
	if (m_pChart && m_LayerId>=0) 
		cmpack_chart_view_show_layer(m_pChart, m_LayerId, FALSE);
	UpdateText();
}

// Initialization before the tool is shown
void CQuickPhotBox::OnShow(void)
{
	CApertures aper = g_Project->Profile()->Apertures();
	if (aper.Count()>0) {
		int index = aper.Find(g_Project->GetInt("LightCurve", "Aperture", 0));
		if (index<0 || index>=aper.Count())
			index = 0;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Aperture), 
			aper.GetRadius(index));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_InSky), 
			g_Project->Profile()->GetDbl(CProfile::SKY_INNER_RADIUS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_OutSky), 
			g_Project->Profile()->GetDbl(CProfile::SKY_OUTER_RADIUS));
	} else {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_Aperture), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_InSky), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_OutSky), 0);
	}
	UpdateText();
	UpdateOverlays();
}

// Clean up after the tool is hidden
void CQuickPhotBox::OnHide(void)
{
	Clear();
}

void CQuickPhotBox::value_changed(GtkSpinButton *spinbutton, CQuickPhotBox *pDlg)
{
	pDlg->OnValueChanged(spinbutton);
}

void CQuickPhotBox::OnValueChanged(GtkSpinButton *pBtn)
{
	if (GTK_WIDGET(pBtn)==m_InSky) {
		double value = gtk_spin_button_get_value(pBtn);
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_OutSky), value+1, 100);
	}
	if (GTK_WIDGET(pBtn)==m_OutSky) {
		double value = gtk_spin_button_get_value(pBtn);
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_InSky), 1, value-1);
	}

	if (m_CenterX>=0 && m_CenterY>=0) {
		UpdateValues();
		UpdateText();
		UpdateOverlays();
	}
}

bool CQuickPhotBox::sky(void)
{
	bool res = false;
	int i, j, x, y, nsky, hw, width, height;
	int left, right, top, bottom, pixels;
	double rinsq, routsq, dysq, rsq, is, os, lo_data, hi_data;
	double *s, value;

	width = m_Image->Width();
	height = m_Image->Height();
	
	is = m_InnerSky;
	os = m_OuterSky;
	hw = (int)ceil(m_OuterSky);
	if (hw<=1 || is<=1 || os<=1 || is>os-1)
		return false;

	x = m_CenterX;
	left = x - hw;
	if (left < 0)
		left = 0;
	right = x + hw;
	if (right >= width)
		right = width-1;

	y = m_CenterY;
	top = y - hw;
	if (top < 0)
		top = 0;
	bottom = y + hw;
	if (bottom >= height)
		bottom = height-1;
	if (left>=right || top>=bottom)
		return false;

	lo_data = m_LoData;
	hi_data = m_HiData;

	pixels = (right-left+1)*(bottom-top+1);
	s = (double*)g_malloc(pixels*sizeof(double));
	if (s) {
		rinsq  = is*is;		// Inner sky radius squared
		routsq = os*os;		// Outer sky radius squared
		nsky   = 0;
		for (j=top; j<=bottom; j++) {
			dysq = (j-y)*(j-y);
			for (i=left; i<=right; i++) {
				rsq = dysq + (i-x)*(i-x);    // squared distance from [i,j] to the center of the star
				if (rsq>=rinsq && rsq<=routsq) {
					value = m_Image->getPixel(i, j);
					if (value>lo_data && value<hi_data) 
						s[nsky++] = value;
				}
			}
		}
		if (nsky > 1) {
			cmpack_robustmean(nsky, s, &m_SkyMean, &m_SkyDev);
			m_NSky = nsky;
			res = true;
		}
		g_free(s);
	}
	return res;
}

bool CQuickPhotBox::full_width_x(double half)
{
	double x_left, x_right, y1, y2;
	int i, width = m_Image->Width(), x = m_CenterX, y = m_CenterY;

    // Find first value lower than Y on the left of the maximum
	x_left = x;
	for (i=x-1; i>=0; i--) {
		y1 = m_Image->getPixel(i, y);
		y2 = m_Image->getPixel(i+1, y);
		if (y1>=y2)
			break;
		if (y1<=half) {
			x_left = i + (half-y1)/(y2-y1);
			break;
		}
	}
	// Find first value lower than Y on the right of the maximum
	x_right = x;
	for (i=x+1; i<width; i++) {
		y1 = m_Image->getPixel(i, y);
		y2 = m_Image->getPixel(i-1, y);
		if (y1>=y2)
			break;
		if (y1<=half) {
			x_right = i - (half-y1)/(y2-y1);
			break;
		}
	}
	if (x_left>=x || x_right<=x)
		return false;

	m_FwhmX = (x_right - x_left);
	return true;
}

bool CQuickPhotBox::full_width_y(double half)
{
	double y_left, y_right, x1, x2;
	int i, width = m_Image->Width(), x = m_CenterX, y = m_CenterY;

    // Find first value lower than Y on the left of the maximum
	y_left = y;
	for (i=y-1; i>=0; i--) {
		x1 = m_Image->getPixel(x, i);
		x2 = m_Image->getPixel(x, i+1);
		if (x1>=x2)
			break;
		if (x1<=half) {
			y_left = i + (half-x1)/(x2-x1);
			break;
		}
	}
	// Find first value lower than Y on the right of the maximum
	y_right = y;
	for (i=y+1; i<width; i++) {
		x1 = m_Image->getPixel(x, i);
		x2 = m_Image->getPixel(x, i-1);
		if (x1>=x2)
			break;
		if (x1<=half) {
			y_right = i - (half-x1)/(x2-x1);
			break;
		}
	}
	if (y_left>=y || y_right<=y)
		return false;

	m_FwhmY = (y_right - y_left);
	return true;
}

bool CQuickPhotBox::signal(void)
{
	int i, j, x, y, hw = (int)ceil(m_ApRadius);
	int left, top, right, bottom, width, height;
	double f, dysq, rsq, d, area, apmag, lo_data, hi_data, rmin_sq, rmax_sq;
	double skyvar, phpadu, err1, err2, err3;

	width = m_Image->Width();
	height = m_Image->Height();

	x = m_CenterX;
	left = x - hw;
	if (left < 0)
		left = 0;
	right = x + hw;
	if (right >= width)
		right = width-1;

	y = m_CenterY;
	top = y - hw;
	if (top < 0)
		top = 0;
	bottom = y + hw;
	if (bottom >= height)
		bottom = height-1;
	if (left>right || top>bottom)
		return false;

	apmag = area = 0.0;
	rmin_sq = (m_ApRadius-0.5)*(m_ApRadius-0.5);
	rmax_sq = (m_ApRadius+0.5)*(m_ApRadius+0.5);
	lo_data = m_LoData;
	hi_data = m_HiData;
	for (j=top; j<=bottom; j++) {
		dysq = (j-y)*(j-y);
		for (i=left; i<=right; i++) {
			rsq = dysq + (i-x)*(i-x);    // squared distance from [i,j] to the center of the star
			if (rsq<rmax_sq) {
		        d = m_Image->getPixel(i, j);
				if (d>lo_data && d<hi_data) {
					f = (rsq<rmin_sq ? 1 : m_ApRadius - sqrt(rsq) + 0.5);
					apmag += f*d;	   // sum of pixel values that fits to the aperture k
					area  += f;         // size of the area of the aperture k
				}
			}
		}
	}
	if (area<1)
		return false;

	// Background corrected integral height
	m_Signal = apmag - m_SkyMean * area;

	// Compute noise
	phpadu = m_ADCGain*m_AvgFrames;
	if (m_Signal>0 && phpadu>0 && m_NSky>0) {
		skyvar = pow(m_SkyDev, 2.0);
		err1 = area*skyvar;
		err2 = m_Signal/phpadu;
		err3 = (skyvar/m_NSky)*area*area;
		m_Noise = sqrt(err1 + err2 + err3);
	}
	
	return true;
}

bool CQuickPhotBox::findmax(int x, int y)
{
	const static int tolerance = 5;

	double d, rmax_sq, dysq, rsq, d_max, lo_data;
	int i, j, width, height, left, top, right, bottom;
	int hw = tolerance, max_x, max_y;

	width = m_Image->Width();
	height = m_Image->Height();
	if (x<0 || y<0 || x>=width || y>=height)
		return false;

	left = (int)x - hw;
	if (left < 0)
		left = 0;
	right = (int)x + hw;
	if (right >= width)
		right = width-1;
	top = (int)y - hw;
	if (top < 0)
		top = 0;
	bottom = (int)y + hw;
	if (bottom >= height)
		bottom = height-1;
	if (left>right || top>bottom)
		return false;

	lo_data = m_LoData;

	// Search for maximum in the region of interest 
	rmax_sq = hw*hw;
	max_x = (int)x; 
	max_y = (int)y; 
	d_max = m_Image->getPixel(max_x, max_y);
	for (j=top; j<=bottom; j++) {
		dysq = (j-y)*(j-y);
		for (i=left; i<=right; i++) {
			rsq = dysq + (i-x)*(i-x);    // squared distance from [i,j] to the center of the star
			if (rsq<rmax_sq) {
				d = m_Image->getPixel(i, j);
				if (d>lo_data && d>d_max) {
					max_x = i;
					max_y = j;
					d_max = d;
				}
			}
		}
	}
	m_CenterX = max_x;
	m_CenterY = max_y;
	m_MaxValue = d_max;
	return true;
}

// Set CCD frame parameters
void CQuickPhotBox::SetParams(CCCDFile &pFile)
{
	m_AvgFrames = pFile.AvgFrames();
}

void CQuickPhotBox::Update(const CImage *img, double x, double y)
{
	m_Image = img;
	if (findmax((int)x, (int)y))
		UpdateValues();
	UpdateText();
	UpdateOverlays();
}

void CQuickPhotBox::UpdateValues(void)
{
	if (m_CenterX<0 || m_CenterY<0) {
		m_Result = RES_ERR_POSITION;
		return;
	} 

	m_ApRadius = gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_Aperture));
	if (m_ApRadius<=0) {
		m_Result = RES_ERR_APERTURE;
		return;
	}
		
	m_InnerSky = gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_InSky));
	m_OuterSky = gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_OutSky));
	if (m_InnerSky<0 || m_OuterSky<=m_InnerSky) {
		m_Result = RES_ERR_SKY_EMPTY;
		return;
	}
			
	m_ADCGain = g_Project->Profile()->GetDbl(CProfile::ADC_GAIN);
	m_LoData = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	m_HiData = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	if (!sky()) {
		m_Result = RES_ERR_SKY_FAILED;
		return;
	}

	if (m_SkyMean < m_MaxValue) {
		double thresh = 0.5*(m_MaxValue + m_SkyMean);
		full_width_x(thresh);
		full_width_y(thresh);
		if (m_FwhmX>0 && m_FwhmY>0)
			m_Fwhm = 0.5*(m_FwhmX + m_FwhmY);
		else if (m_FwhmX<=0)
			m_Fwhm = m_FwhmY;
		else if (m_FwhmY<=0)
			m_Fwhm = m_FwhmX;
		else
			m_Fwhm = -1.0;
	}

	if (!signal()) {
		m_Result = RES_ERR_SIGNAL_FAILED;
		return;
	}

	m_Result = RES_OK; 
}

void CQuickPhotBox::UpdateText(void)
{
	GtkTextIter ptr;
	char msg[1024];

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_Info), NULL);
	gtk_text_buffer_set_text(m_Buffer, "", -1);
	gtk_text_buffer_get_end_iter(m_Buffer, &ptr);

	switch (m_Result)
	{
	case RES_OK:
		sprintf(msg, "Max. pixel = (%d, %d)\n", m_CenterX, m_CenterY);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Max. value = %.1f ADU\n", m_MaxValue);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Sky = %.1f ADU\n", m_SkyMean);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		sprintf(msg, "Sky dev. = %.1f ADU\n", m_SkyDev);
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_FwhmX>0)
			sprintf(msg, "FWHM(X) = %.1f pxl\n", m_FwhmX);
		else
			sprintf(msg, "FWHM(X) = ---- pxl\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_FwhmY>0)
			sprintf(msg, "FWHM(Y) = %.1f pxl\n", m_FwhmY);
		else
			sprintf(msg, "FWHM(Y) = ---- pxl\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Fwhm>0)
			sprintf(msg, "FWHM = %.1f pxl\n", m_Fwhm);
		else
			sprintf(msg, "FWHM = ---- pxl\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Signal>=0)
			sprintf(msg, "Net intensity = %.1f ADU\n", m_Signal);
		else
			sprintf(msg, "Net intensity = ---- ADU\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Noise>=0)
			sprintf(msg, "Noise = %.1f ADU\n", m_Noise);
		else
			sprintf(msg, "Noise = ---- ADU\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Noise>0 && m_Signal>0) 
			sprintf(msg, "S/N ratio = %.1f dB\n", -10.0*log10(m_Signal/m_Noise));
		else
			sprintf(msg, "S/N ratio = ---- dB\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Noise>0 && m_Signal>0) 
			sprintf(msg, "Brightness = %.2f mag\n", 25.0 - 2.5*log10(m_Signal));
		else
			sprintf(msg, "Brightness = ---- mag\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);
		if (m_Noise>0 && m_Signal>0) 
			sprintf(msg, "Error = %.2f mag\n", 1.0857*m_Noise/m_Signal);
		else
			sprintf(msg, "Error = ---- mag\n");
		gtk_text_buffer_insert(m_Buffer, &ptr, msg, -1);

		// Legend
		gtk_text_buffer_insert(m_Buffer, &ptr, "\nLegend\n", -1);
		gtk_text_buffer_insert(m_Buffer, &ptr, "FWHM: red circle\n", -1);
		gtk_text_buffer_insert(m_Buffer, &ptr, "Aperture: green circle\n", -1);
		gtk_text_buffer_insert(m_Buffer, &ptr, "Sky: blue annulus\n", -1);
		break;

	case RES_ERR_POSITION:
		gtk_text_buffer_insert(m_Buffer, &ptr, "Click left mouse button on the chart.", -1);
		break;
	case RES_ERR_APERTURE:
		gtk_text_buffer_insert(m_Buffer, &ptr, "Invalid aperture size. The aperture must not be empty.", -1);
		break;
	case RES_ERR_SKY_EMPTY:
		gtk_text_buffer_insert(m_Buffer, &ptr, "Invalid sky annulus size. The sky annulus must not be empty", -1);
		break;
	case RES_ERR_SKY_FAILED:
		gtk_text_buffer_insert(m_Buffer, &ptr, "Failed to determine the background level and noise.", -1);
		break;
	case RES_ERR_SIGNAL_FAILED:
		gtk_text_buffer_insert(m_Buffer, &ptr, "Failed to determine peak height.", -1);
		break;
	}

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_Info), m_Buffer);
}

void CQuickPhotBox::UpdateOverlays(void)
{
	double x, y, r;

	if (!m_pChart)
		return;

	if (m_Result==RES_OK) {
		x = m_CenterX + 0.5;
		y = m_CenterY + 0.5;
		if (m_LayerId<0) {
			m_LayerId = cmpack_chart_view_add_layer(m_pChart);
			r = m_ApRadius;
			m_ObjectId[0] = cmpack_chart_view_add_circle(m_pChart, m_LayerId, 
				x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_GREEN, FALSE);
			r = 0.5 * m_Fwhm;
			m_ObjectId[1] = cmpack_chart_view_add_circle(m_pChart, m_LayerId, 
				x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_RED, FALSE);
			r = m_InnerSky;
			m_ObjectId[2] = cmpack_chart_view_add_circle(m_pChart, m_LayerId, 
				x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_BLUE, FALSE);
			r = m_OuterSky;
			m_ObjectId[3] = cmpack_chart_view_add_circle(m_pChart, m_LayerId, 
				x - r, y - r, 2.0*r, 2.0*r, CMPACK_COLOR_BLUE, FALSE);
		} else {
			r = m_ApRadius;
			cmpack_chart_view_move_object(m_pChart, m_ObjectId[0], x - r, y - r);
			cmpack_chart_view_resize_object(m_pChart, m_ObjectId[0], 2.0*r, 2.0*r);
			r = 0.5 * m_Fwhm;
			cmpack_chart_view_move_object(m_pChart, m_ObjectId[1], x - r, y - r);
			cmpack_chart_view_resize_object(m_pChart, m_ObjectId[1], 2.0*r, 2.0*r);
			r = m_InnerSky;
			cmpack_chart_view_move_object(m_pChart, m_ObjectId[2], x - r, y - r);
			cmpack_chart_view_resize_object(m_pChart, m_ObjectId[2],  2.0*r, 2.0*r);
			r = m_OuterSky;
			cmpack_chart_view_move_object(m_pChart, m_ObjectId[3], x - r, y - r);
			cmpack_chart_view_resize_object(m_pChart, m_ObjectId[3], 2.0*r, 2.0*r);
			cmpack_chart_view_show_layer(m_pChart, m_LayerId, TRUE);
		}
	} else {
		if (m_LayerId>=0) 
			cmpack_chart_view_show_layer(m_pChart, m_LayerId, FALSE);
	}
}
