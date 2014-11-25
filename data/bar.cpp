/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2013  Warzone 2100 Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
/** @file
 *  Functions for the bar graph widget
 */

#include "widget.h"
#include "widgint.h"
#include "tip.h"
#include "form.h"
#include "bar.h"
// FIXME Direct iVis implementation include!
#include "lib/ivis_opengl/pieblitfunc.h"
#include "lib/ivis_opengl/piepalette.h"

W_BARINIT::W_BARINIT()
	: orientation(WBAR_LEFT)
	, size(0)
	, minorSize(0)
	, iRange(100)
	, denominator(1)
	, precision(0)
	//sCol
	//sMinorCol
	, pTip(NULL)
{
	sCol.rgba = 0;
	sMinorCol.rgba = 0;
}

W_BARGRAPH::W_BARGRAPH(W_BARINIT const *init)
	: WIDGET(init, WIDG_BARGRAPH)
	, barPos(init->orientation)
	, majorSize(init->size)
	, minorSize(init->minorSize)
	, iRange(init->iRange)
	, iValue(0)
	, iOriginal(0)
	, denominator(MAX(init->denominator, 1))
	, precision(init->precision)
	, majorCol(init->sCol)
	, minorCol(init->sMinorCol)
	, textCol(WZCOL_BLACK)
	, pTip(init->pTip)
{
	/* Set the display function */
	if (display == NULL)
	{
		if (init->style & WBAR_TROUGH)
		{
			display = barGraphDisplayTrough;
		}
		else if (init->style & WBAR_DOUBLE)
		{
			display = barGraphDisplayDouble;
		}
		else
		{
			display = barGraphDisplay;
		}
	}

	/* Set the minor colour if necessary */
	// Actually, this sets the major colour to the minor colour. The minor colour used to be left completely uninitialised... Wonder what it was for..?
	if (style & WBAR_DOUBLE)
	{
		majorCol = minorCol;
	}
}

/* Create a barGraph widget data structure */
W_BARGRAPH* barGraphCreate(const W_BARINIT* psInit)
{
	if (psInit->style & ~(WBAR_PLAIN | WBAR_TROUGH | WBAR_DOUBLE | WIDG_HIDDEN))
	{
		ASSERT(false, "Unknown bar graph style");
		return NULL;
	}

	if (psInit->orientation < WBAR_LEFT
	 || psInit->orientation > WBAR_BOTTOM)
	{
		ASSERT(false, "barGraphCreate: Unknown orientation");
		return NULL;
	}

	if (psInit->size > WBAR_SCALE)
	{
		ASSERT(false, "barGraphCreate: Bar size out of range");
		return NULL;
	}
	if ((psInit->style & WBAR_DOUBLE)
	 && (psInit->minorSize > WBAR_SCALE))
	{
		ASSERT(false, "barGraphCreate: Minor bar size out of range");
		return NULL;
	}

	/* Allocate the required memory */
	W_BARGRAPH *psWidget = new W_BARGRAPH(psInit);
	if (psWidget == NULL)
	{
		debug(LOG_FATAL, "barGraphCreate: Out of memory");
		abort();
		return NULL;
	}

	return psWidget;
}


/* Free the memory used by a barGraph */
void barGraphFree(W_BARGRAPH *psWidget)
{
	ASSERT(psWidget != NULL,"Invalid widget pointer");

	delete psWidget;
}


/* Set the current size of a bar graph */
void widgSetBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD iValue)
{
	W_BARGRAPH		*psBGraph;
	UDWORD			size;

	ASSERT(psScreen != NULL, "Invalid screen pointer");

	psBGraph = (W_BARGRAPH *)widgGetFromID(psScreen, id);
	if (psBGraph == NULL || psBGraph->type != WIDG_BARGRAPH)
	{
		ASSERT( false, "widgSetBarSize: Couldn't find widget from id" );
		return;
	}

	psBGraph->iOriginal = iValue;
	if ( iValue < psBGraph->iRange )
	{
		psBGraph->iValue = (UWORD) iValue;
	}
	else
	{
		psBGraph->iValue = psBGraph->iRange;
	}

	size = WBAR_SCALE * psBGraph->iValue / MAX(psBGraph->iRange, 1);

	psBGraph->majorSize = (UWORD)size;
}


/* Set the current size of a minor bar on a double graph */
void widgSetMinorBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD iValue )
{
	W_BARGRAPH		*psBGraph;
	UDWORD			size;

	ASSERT(psScreen != NULL, "Invalid screen pointer");

	psBGraph = (W_BARGRAPH *)widgGetFromID(psScreen, id);
	if (psBGraph == NULL || psBGraph->type != WIDG_BARGRAPH)
	{
		ASSERT(false, "Couldn't find widget from id");
		return;
	}

	size = WBAR_SCALE * iValue / MAX(psBGraph->iRange, 1);
	if (size > WBAR_SCALE)
	{
		size = WBAR_SCALE;
	}

	psBGraph->minorSize = (UWORD)size;
}


/* Respond to a mouse moving over a barGraph */
void W_BARGRAPH::highlight(W_CONTEXT *psContext)
{
	if (pTip)
	{
		tipStart(this, pTip, psContext->psScreen->TipFontID,
		         psContext->psForm->aColours,
		         x + psContext->xOffset, y + psContext->yOffset,
		         width, height);
	}
}


/* Respond to the mouse moving off a barGraph */
void W_BARGRAPH::highlightLost(W_CONTEXT *psContext)
{
	tipStop(this);
}


static void barGraphDisplayText(W_BARGRAPH *barGraph, int x0, int x1, int y1, PIELIGHT *pColours)
{
	if (!barGraph->text.isEmpty())
	{
		QByteArray utf = barGraph->text.toUtf8();
		iV_SetFont(font_small);
		int textWidth = iV_GetTextWidth(utf.constData());
		Vector2i pos((x0 + x1 - textWidth)/2, y1);
		iV_SetTextColour(WZCOL_BLACK);  // Add a shadow, to make it visible against any background.
		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
			{
				iV_DrawText(utf.constData(), pos.x + dx*1.25f, pos.y + dy*1.25f);
			}
		iV_SetTextColour(barGraph->textCol);
		iV_DrawText(utf.constData(), pos.x, pos.y - 0.25f);
		iV_DrawText(utf.constData(), pos.x, pos.y + 0.25f);  // Draw twice, to make it more visible.
	}
}

/* The simple bar graph display function */
void barGraphDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, PIELIGHT *pColours)
{
	SDWORD		x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	W_BARGRAPH	*psBGraph;

	psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		break;
	case WBAR_RIGHT:
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		break;
	case WBAR_TOP:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		break;
	case WBAR_BOTTOM:
		x0 = xOffset + psWidget->x;
		x1 = x0 + psWidget->width;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		break;
	}

	/* Now draw the graph */
	pie_BoxFill(x0,y0, x1,y1,psBGraph->majorCol);
	iV_Line(x0,y1, x0,y0, pColours[WCOL_LIGHT]);
	iV_Line(x0,y0, x1,y0, pColours[WCOL_LIGHT]);
	iV_Line(x1,y0, x1,y1, pColours[WCOL_DARK]);
	iV_Line(x0,y1, x1,y1, pColours[WCOL_DARK]);

	barGraphDisplayText(psBGraph, x0, x1, y1, pColours);
}


/* The double bar graph display function */
void barGraphDisplayDouble(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, PIELIGHT *pColours)
{
	SDWORD		x0 = 0, y0 = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
	W_BARGRAPH	*psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + 2*psWidget->height/3;

		/* Calculate the minor bar */
		x2 = x0;
		y2 = y0 + psWidget->height/3;
		x3 = x2 + psWidget->width * psBGraph->minorSize / WBAR_SCALE;
		y3 = y0 + psWidget->height;
		break;
	case WBAR_RIGHT:
		/* Calculate the major bar */
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + 2*psWidget->height/3;

		/* Calculate the minor bar */
		x3 = x1;
		y2 = y0 + psWidget->height/3;
		x2 = x3 - psWidget->width * psBGraph->minorSize / WBAR_SCALE;
		y3 = y0 + psWidget->height;
		break;
	case WBAR_TOP:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + 2*psWidget->width/3;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;

		/* Calculate the minor bar */
		x2 = x0 + psWidget->width/3;
		y2 = y0;
		x3 = x0 + psWidget->width;
		y3 = y2 + psWidget->height * psBGraph->minorSize / WBAR_SCALE;
		break;
	case WBAR_BOTTOM:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		x1 = x0 + 2*psWidget->width/3;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;

		/* Calculate the minor bar */
		x2 = x0 + psWidget->width/3;
		x3 = x0 + psWidget->width;
		y3 = y1;
		y2 = y3 - psWidget->height * psBGraph->minorSize / WBAR_SCALE;
		break;
	}

	/* Draw the minor bar graph */
	if (psBGraph->minorSize > 0)
	{
		pie_BoxFill(x2,y2, x3,y3,psBGraph->minorCol);
		iV_Line(x2,y3, x2,y2, pColours[WCOL_LIGHT]);
		iV_Line(x2,y2, x3,y2, pColours[WCOL_LIGHT]);
		iV_Line(x3,y2, x3,y3, pColours[WCOL_DARK]);
		iV_Line(x2,y3, x3,y3, pColours[WCOL_DARK]);
	}

	/* Draw the major bar graph */
	pie_BoxFill(x0,y0, x1,y1,psBGraph->majorCol);
	iV_Line(x0,y1, x0,y0, pColours[WCOL_LIGHT]);
	iV_Line(x0,y0, x1,y0, pColours[WCOL_LIGHT]);
	iV_Line(x1,y0, x1,y1, pColours[WCOL_DARK]);
	iV_Line(x0,y1, x1,y1, pColours[WCOL_DARK]);

	barGraphDisplayText(psBGraph, x0, x1, y1, pColours);
}


/* The trough bar graph display function */
void barGraphDisplayTrough(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, PIELIGHT *pColours)
{
	SDWORD		x0 = 0, y0 = 0, x1 = 0, y1 = 0;		// Position of the bar
	SDWORD		tx0 = 0, ty0 = 0, tx1 = 0, ty1 = 0;	// Position of the trough
	bool		showBar=true, showTrough=true;
	W_BARGRAPH	*psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		if (x0 == x1)
		{
			showBar = false;
		}
		tx0 = x1+1;
		ty0 = y0;
		tx1 = x0 + psWidget->width;
		ty1 = y1;
		if (tx0 >= tx1)
		{
			showTrough = false;
		}
		break;
	case WBAR_RIGHT:
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		if (x0 == x1)
		{
			showBar = false;
		}
		tx0 = xOffset + psWidget->x;
		ty0 = y0;
		tx1 = x0-1;
		ty1 = y1;
		if (tx0 >= tx1)
		{
			showTrough = false;
		}
		break;
	case WBAR_TOP:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		if (y0 == y1)
		{
			showBar = false;
		}
		tx0 = x0;
		ty0 = y1+1;
		tx1 = x1;
		ty1 = y0 + psWidget->height;
		if (ty0 >= ty1)
		{
			showTrough = false;
		}
		break;
	case WBAR_BOTTOM:
		x0 = xOffset + psWidget->x;
		x1 = x0 + psWidget->width;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		if (y0 == y1)
		{
			showBar = false;
		}
		tx0 = x0;
		ty0 = yOffset + psWidget->y;
		tx1 = x1;
		ty1 = y0-1;
		if (ty0 >= ty1)
		{
			showTrough = false;
		}
		break;
	}

	/* Now draw the graph */
	if (showBar)
	{
		pie_BoxFill(x0, y0, x1, y1, psBGraph->majorCol);
	}
	if (showTrough)
	{
		pie_BoxFill(tx0, ty0, tx1, ty1, pColours[WCOL_BKGRND]);
		iV_Line(tx0,ty1, tx0,ty0, pColours[WCOL_DARK]);
		iV_Line(tx0,ty0, tx1,ty0, pColours[WCOL_DARK]);
		iV_Line(tx1,ty0, tx1,ty1, pColours[WCOL_LIGHT]);
		iV_Line(tx0,ty1, tx1,ty1, pColours[WCOL_LIGHT]);
	}

	barGraphDisplayText(psBGraph, x0, tx1, ty1, pColours);
}
