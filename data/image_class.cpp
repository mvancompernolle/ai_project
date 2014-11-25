/**************************************************************

image_class.cpp (C-Munipack project)
CCD image class interface
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

**************************************************************/
#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image_class.h"
#include "configuration.h"
#include "main.h"

#define BAD_PIXEL_COLOR	0xFF0000

//--------------------------   HELPER FUNCTIONS   ----------------------------------

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
// pseudocolors mapping (black, violet, blue, cyan, green, yellow, red, white)
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

//--------------------------   CCD IMAGE CLASS INTERFACE   ----------------------------------

//
// Default constructor
//
CImage::CImage():m_Handle(NULL), m_AutoRangeValid(false), m_Range(0), m_BlackLevel(0)
{
}

//
// Destructor
//
CImage::~CImage()
{
	if (m_Handle)
		cmpack_image_destroy(m_Handle);
}

//
// Clear data
//
void CImage::Clear(void)
{
	if (m_Handle) {
		cmpack_image_destroy(m_Handle);
		m_Handle = NULL;
	}
	m_AutoRangeValid = false;
	m_Range = m_BlackLevel = 0.0;
}

//
// Load image info from a CCD file
//
bool CImage::Load(const gchar *fpath, CmpackBitpix bitpix, GError **error)
{
	Clear();

	CCCDFile f;
	return f.Open(fpath, CMPACK_OPEN_READONLY, error) && f.GetImageData(*this, bitpix, error);
}


// 
// Set another image
//
void CImage::Assign(CmpackImage *handle)
{
	Clear();
	m_Handle = handle;
}

void CImage::SetParams(int width, int height, CmpackBitpix depth)
{
	if (m_Handle) {
		if (cmpack_image_width(m_Handle)!=width || cmpack_image_height(m_Handle)!=height || cmpack_image_bitpix(m_Handle)!=depth) {
			cmpack_image_destroy(m_Handle);
			m_Handle = NULL;
		}
	}
	if (!m_Handle)
		m_Handle = cmpack_image_new(width, height, depth);
	m_AutoRangeValid = false;
}

void CImage::TransformOrigFrame(void)
{
	int binning = g_Project->Profile()->GetInt(CProfile::BINNING);
	if (m_Handle && binning>1) {
		CmpackImage *dst = cmpack_image_binning(m_Handle, binning, binning);
		cmpack_image_destroy(m_Handle);
		m_Handle = dst;
	}
	bool fliph, flipv;
	fliph = g_Project->Profile()->GetBool(CProfile::FLIP_H);
	flipv = g_Project->Profile()->GetBool(CProfile::FLIP_V);
	if (m_Handle && (fliph || flipv)) 
		cmpack_image_transpose(m_Handle, fliph, flipv);
}

void CImage::AutoRange(void)
{
	double nulvalue, badvalue, black, white;

	m_Range = 1.0; 
	m_BlackLevel = 0.0;
	nulvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	badvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	cmpack_image_autogb(m_Handle, nulvalue, badvalue, &black, &white);
	if (white>black) {
		m_BlackLevel = black;
		m_Range = (white-black);
	} else {
		m_BlackLevel = white - 0.5;
		m_Range = 1.0;
	}
	m_AutoRangeValid = true;
}


//
// Transpose image
//
void CImage::Transpose(bool hflip, bool vflip)
{
	if (m_Handle)
		cmpack_image_transpose(m_Handle, hflip ? 1 : 0, vflip ? 1 : 0);
}


//
// Make image data
//
CmpackImageData *CImage::ToImageData(bool invert, bool pseudocolors, bool badpixels, bool reverse_y)
{
	int width, height;

	if (m_Handle) {
		width = cmpack_image_width(m_Handle);
		height = cmpack_image_height(m_Handle);
		if (width>0 && height>0) {
			CmpackImageData *res = cmpack_image_data_new(CAIRO_FORMAT_RGB24, width, height);
			Paint(cmpack_image_data_get_surface(res), invert, pseudocolors, badpixels, reverse_y);
			cmpack_image_data_changed(res);
			return res;
		}
	}
	return NULL;
}

//
// Make GdkPixbuf object
//
GdkPixbuf *CImage::ToPixBuf(bool invert, bool pseudocolors, bool badpixels, bool reverse_y)
{
	int width, height;

	if (m_Handle) {
		width = cmpack_image_width(m_Handle);
		height = cmpack_image_height(m_Handle);
		if (width>0 && height>0) {
			GdkPixbuf *res = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
			Paint(res, invert, pseudocolors, badpixels, reverse_y);
			return res;
		}
	}
	return NULL;
}

void CImage::Paint(cairo_surface_t *target, bool invert, bool pseudocolors, bool badpixels, bool reverse_y)
{
	double scale, offset, minvalue, maxvalue;
	int x, y, width, height;

	// Auto range / black level
	if (!m_AutoRangeValid)
		AutoRange();
	if (m_Range==0)
		return;

	if (!invert) {
		scale = 1.0/m_Range;
		offset = -m_BlackLevel/m_Range;
	} else {
		scale = -1.0/m_Range;
		offset = 1.0+m_BlackLevel/m_Range;
	}
	
	unsigned char *dst = cairo_image_surface_get_data(target);
	int drowsride = cairo_image_surface_get_stride(target);
	if (reverse_y) {
		dst += (cairo_image_surface_get_height(target)-1)*drowsride;
		drowsride = -drowsride;
	}
	width = cmpack_image_width(m_Handle);
	height = cmpack_image_height(m_Handle);
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	switch (cmpack_image_bitpix(m_Handle))
	{
	case CMPACK_BITPIX_SSHORT:
		{
			int iLo = (int)minvalue, iHi = (int)maxvalue;
			int16_t *sptr = (int16_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_USHORT:
		{
			int iLo = (int)minvalue, iHi = (int)maxvalue;
			uint16_t *sptr = (uint16_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_SLONG:
		{
			int32_t iLo = (int32_t)minvalue, iHi = (int32_t)maxvalue;
			int32_t *sptr = (int32_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int32_t val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						int32_t val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_ULONG:
		{
			uint32_t iLo = (uint32_t)minvalue, iHi = (uint32_t)maxvalue;
			uint32_t *sptr = (uint32_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						uint32_t val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						uint32_t val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_FLOAT:
		{
			float *sptr = (float*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						double val = *sptr++;
						if (!badpixels || (val>minvalue && val<maxvalue))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						double val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_DOUBLE:
		{
			double *sptr = (double*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						double val = *sptr++;
						if (!badpixels || (val>minvalue && val<maxvalue))
							*dptr++ = grayscale(scale*val + offset);
						else
							*dptr++ = BAD_PIXEL_COLOR;
					}
				}
			} else {
				for (y=0; y<height; y++) {
					uint32_t *dptr = (uint32_t*)(dst + y*drowsride);
					for (x=0; x<width; x++) {
						double val = *sptr++;
						*dptr++ = pseudocolor(scale*val + offset);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void CImage::Paint(GdkPixbuf *target, bool invert, bool pseudocolors, bool badpixels, bool reverse_y)
{
	int		width, height, drowstride;
	double	scale, offset, minvalue, maxvalue;
	uint32_t color;
	guchar	*dst;

	// Auto range / black level
	if (!m_AutoRangeValid)
		AutoRange();
	if (m_Range==0)
		return;

	if (!invert) {
		scale = 1.0/m_Range;
		offset = -m_BlackLevel/m_Range;
	} else {
		scale = -1.0/m_Range;
		offset = 1.0+m_BlackLevel/m_Range;
	}
	
	width = cmpack_image_width(m_Handle);
	height = cmpack_image_height(m_Handle);
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	dst = (guchar*)gdk_pixbuf_get_pixels(target);
	drowstride = gdk_pixbuf_get_rowstride(target);
	if (reverse_y) {
		dst += (gdk_pixbuf_get_height(target)-1)*drowstride;
		drowstride = -drowstride;
	}
	switch (cmpack_image_bitpix(m_Handle))
	{
	case CMPACK_BITPIX_SSHORT:
		{
			int iLo = (int)minvalue, iHi = (int)maxvalue;
			int16_t *sptr = (int16_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi)) 
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_USHORT:
		{
			int iLo = (int)minvalue, iHi = (int)maxvalue;
			uint16_t *sptr = (uint16_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_SLONG:
		{
			int32_t iLo = (int32_t)minvalue, iHi = (int32_t)maxvalue;
			int32_t *sptr = (int32_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int32_t val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						int32_t val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_ULONG:
		{
			uint32_t iLo = (uint32_t)minvalue, iHi = (uint32_t)maxvalue;
			uint32_t *sptr = (uint32_t*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						uint32_t val = *sptr++;
						if (!badpixels || (val>iLo && val<iHi))
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						uint32_t val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_FLOAT:
		{
			double val;
			float *sptr = (float*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						val = *sptr++;
						if (!badpixels || (val>minvalue && val<maxvalue))
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	case CMPACK_BITPIX_DOUBLE:
		{
			double val;
			double *sptr = (double*)cmpack_image_data(m_Handle);
			if (!pseudocolors) {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						val = *sptr++;
						if (!badpixels || (val>minvalue && val<maxvalue))
							color = grayscale(scale*val + offset);
						else
							color = BAD_PIXEL_COLOR;
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			} else {
				for (int y=0; y<height; y++) {
					guchar *dptr = dst + drowstride*y;
					for (int x=0; x<width; x++) {
						val = *sptr++;
						color = pseudocolor(scale*val + offset);
						*dptr++ = (color>>16);
						*dptr++ = (color>>8) & 0xFF;
						*dptr++ = (color & 0xFF);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

double CImage::getPixel(int x, int y) const
{
	if (m_Handle)
		return cmpack_image_getpixel(m_Handle, x, y);
	return 0.0;
}

// Compute pixel value range
bool CImage::MinMax(double *minvalue, double *maxvalue) const
{
	if (m_Handle) {
		double nulvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
		double badvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
		return cmpack_image_minmax(m_Handle, nulvalue, badvalue, minvalue, maxvalue)==0;
	}
	return false;
}

// Compute mean and standard deviation
bool CImage::MeanDev(double *mean, double *stddev) const
{
	if (m_Handle) {
		double nulvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
		double badvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
		return cmpack_image_meandev(m_Handle, nulvalue, badvalue, mean, stddev)==0;
	}
	return false;
}

int CImage::Width(void) const
{
	if (m_Handle)
		return cmpack_image_width(m_Handle);
	return 0;
}

int CImage::Height(void) const
{
	if (m_Handle)
		return cmpack_image_height(m_Handle);
	return 0;
}

double CImage::Range(void)
{
	if (!m_AutoRangeValid)
		AutoRange();
	return m_Range;
}

double CImage::BlackLevel(void)
{
	if (!m_AutoRangeValid)
		AutoRange();
	return m_BlackLevel;
}

// Set scale parameters
bool CImage::Profile(CmpackGraphData *target, int x0, int y0, int x1, int y1,
	double *ymin, double *ymax, double *mean, double *stddev) const
{
	int x, y, k, width, height, length, count;
	double d, phi, minvalue, maxvalue;
	double *data;
	CmpackGraphItem item;

	cmpack_graph_data_clear(target);
	if (mean)
		*mean = 0.0;
	if (stddev) 
		*stddev = 0.0;
	if (ymin)
		*ymin = 0.0;
	if (ymax)
		*ymax = 0.0;

	if (!m_Handle)
		return false;

	memset(&item, 0, sizeof(item));
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);

	width = cmpack_image_width(m_Handle);
	height = cmpack_image_height(m_Handle);

	data = NULL;
	count = 0;

	d = sqrt((double)(x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
	if (d>=1) {
		phi = atan2((double)(y1-y0), (double)(x1-x0));
		if (phi>=-M_PI_4 && phi<=M_PI_4) {
			length = MAX(0, x1-x0)+1;
			data = (double*)g_malloc(length*sizeof(double));
			for (k=0; k<length; k++) {
				x = x0 + k;
				y = y0 + (int)(((double)k)/(x1-x0)*(y1-y0));
				if (x>=0 && x<width && y>=0 && y<height) {
					item.x = d*((double)k)/(x1-x0);
					item.y = cmpack_image_getpixel(m_Handle, x, y);
					item.color = (item.y>minvalue && item.y<maxvalue ? CMPACK_COLOR_DEFAULT : CMPACK_COLOR_RED);
					cmpack_graph_data_add(target, &item, sizeof(item));
					if (item.y>minvalue)
						data[count++] = item.y;
				}
			}
		} else 
		if (phi>=3*M_PI_4 || phi<=-3*M_PI_4) {
			length = MAX(0, x0-x1)+1;
			data = (double*)g_malloc(length*sizeof(double));
			for (k=0; k<length; k++) {
				x = x0 - k;
				y = y0 - (int)(((double)k)/(x1-x0)*(y1-y0));
				if (x>=0 && x<width && y>=0 && y<height) {
					item.x = d*((double)k)/(x0-x1);
					item.y = cmpack_image_getpixel(m_Handle, x, y);
					item.color = (item.y>minvalue && item.y<maxvalue ? CMPACK_COLOR_DEFAULT : CMPACK_COLOR_RED);
					cmpack_graph_data_add(target, &item, sizeof(item));
					if (item.y>minvalue)
						data[count++] = item.y;
				}
			}
		} else
		if (phi>0) {
			length = MAX(0, y1-y0)+1;
			data = (double*)g_malloc(length*sizeof(double));
			for (k=0; k<length; k++) {
				x = x0 + (int)(((double)k)/(y1-y0)*(x1-x0));
				y = y0 + k;
				if (x>=0 && x<width && y>=0 && y<height) {
					item.x = d*((double)k)/(y1-y0);
					item.y = cmpack_image_getpixel(m_Handle, x, y);
					item.color = (item.y>minvalue && item.y<maxvalue ? CMPACK_COLOR_DEFAULT : CMPACK_COLOR_RED);
					cmpack_graph_data_add(target, &item, sizeof(item));
					if (item.y>minvalue)
						data[count++] = item.y;
				}
			}
		} else {
			length = MAX(0, y0-y1)+1;
			data = (double*)g_malloc(length*sizeof(double));
			for (k=0; k<length; k++) {
				y = y0 - k;
				x = x0 - (int)(((double)k)/(y1-y0)*(x1-x0));
				if (x>=0 && x<width && y>=0 && y<height) {
					item.x = d*((double)k)/(y0-y1);
					item.y = cmpack_image_getpixel(m_Handle, x, y);
					item.color = (item.y>minvalue && item.y<maxvalue ? CMPACK_COLOR_DEFAULT : CMPACK_COLOR_RED);
					data[k] = item.y;
					cmpack_graph_data_add(target, &item, sizeof(item));
					if (item.y>minvalue)
						data[count++] = item.y;
				}
			}
		}
	}

	if (ComputeMinMax(count, data, ymin, ymax)) 
		cmpack_robustmean(count, data, mean, stddev);

	g_free(data);
	return true;
}

// Get histogram
bool CImage::Histogram(CmpackGraphData *target, int length, double channel_width,
		double zero_offset, double *ymax, bool logScale) const
{
	int i;
	double minvalue, maxvalue; 
	unsigned *hist, hmax;
	CmpackGraphItem item;

	cmpack_graph_data_clear(target);

	if (!m_Handle)
		return false;

	memset(&item, 0, sizeof(item));

	// Make histogram
	hist = (unsigned*)cmpack_malloc(length*sizeof(unsigned));
	if (cmpack_image_histogram(m_Handle, length, channel_width, zero_offset, hist)!=0) {
		cmpack_free(hist);
		return false;
	}

	// Find maximum channel value
	if (ymax) {
		hmax = 0;
		for (i=0; i<length; i++) {
			if (hist[i] > hmax)
				hmax = hist[i];
		}
		if (!logScale)
			*ymax = hmax;
		else 
			*ymax = hmax>0 ? log((double)hmax)/log(2.0)+1 : 0;
	}

	// Make graph data
	minvalue = g_Project->Profile()->GetDbl(CProfile::BAD_PIXEL_VALUE);
	maxvalue = g_Project->Profile()->GetDbl(CProfile::OVEREXPOSED_VALUE);
	for (i=0; i<length; i++) {
		item.x = zero_offset + (i+0.5)*channel_width;
		if (!logScale)
			item.y = hist[i];
		else 
			item.y = hist[i]>0 ? log((double)hist[i])/log(2.0)+1 : 0;
		double bottom = zero_offset + i*channel_width, top = zero_offset + (i+1)*channel_width;
		item.color = (bottom>minvalue && top<maxvalue ? CMPACK_COLOR_DEFAULT : CMPACK_COLOR_RED);
		cmpack_graph_data_add(target, &item, sizeof(item));
	}
	cmpack_free(hist);
	return true;
}
