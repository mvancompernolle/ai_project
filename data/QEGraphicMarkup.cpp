/*  QEGraphicMarkup.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QColor>
#include <QECommon.h>
#include "QEGraphic.h"
#include "QEGraphicMarkup.h"

#define DEBUG qDebug () << "QEGraphicMarkup" << __FUNCTION__ << __LINE__ << "::"

// Allowable distance (in pixels) from cursor to object which will still be considered 'over'.
// Plus default distance when not over.
//
#define OVER_TOLERANCE       6
#define NOT_OVER_DISTANCE    1000


//=============================================================================
// QEGraphicMarkup - base class
//=============================================================================
//
QEGraphicMarkup::QEGraphicMarkup (QEGraphic* ownerIn)
{
   this->owner = ownerIn;
   this->visible = false;
   this->enabled = true;
   this->selected = false;
   this->cursor = Qt::CrossCursor;

   this->current = QPointF (0.0, 0.0);
   this->pen.setColor (QColor (0,0,0));
   this->pen.setStyle (Qt::SolidLine);
   this->pen.setWidth (1);
}

//-----------------------------------------------------------------------------
//
QEGraphicMarkup::~QEGraphicMarkup ()
{
}

//-----------------------------------------------------------------------------
//
QCursor QEGraphicMarkup::getCursor () const
{
   return this->cursor;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicMarkup::isOver (const QPointF& /* point */, int& distance) const
{
   distance = NOT_OVER_DISTANCE;
   return false;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicMarkup::isOverHere (const QPointF& here, const QPointF& point, int& distance) const
{
   const QPoint s = this->getOwner()->pixelDistance (here, point);
   const int dx = s.x ();
   const int dy = s.y ();
   const int adx = ABS (dx);
   const int ady = ABS (dy);
   bool result;

   if (this->isVisible () && (adx <= OVER_TOLERANCE) && (ady <= OVER_TOLERANCE)) {
      result = true;
      distance = MIN (adx, ady);
   } else {
      result = false;
      distance = NOT_OVER_DISTANCE;    // ensure function not erroneous.
   }

   return result;
}

//-----------------------------------------------------------------------------
//
void QEGraphicMarkup::plot ()
{
   if (this->visible) {
      QPen   savedPen = this->owner->getCurvePen ();
      QBrush savedBrush = this->owner->getCurveBrush ();
      this->plotMarkup ();
      this->owner->setCurvePen   (savedPen);
      this->owner->setCurveBrush (savedBrush);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicMarkup::plotCurve (const QEGraphic::DoubleVector& xData,
                                 const QEGraphic::DoubleVector& yData)
{
   this->owner->setCurvePen   (this->pen);
   this->owner->setCurveBrush (this->brush);
   this->owner->plotMarkupCurveData (xData, yData);
}

//-----------------------------------------------------------------------------
//
QEGraphic* QEGraphicMarkup::getOwner () const
{
   return this->owner;
}

//-----------------------------------------------------------------------------
//
void QEGraphicMarkup::setVisible (const bool isVisibleIn)
{
   this->visible = isVisibleIn;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicMarkup::isVisible () const
{
   return this->visible;
}

//-----------------------------------------------------------------------------
//
void QEGraphicMarkup::setEnabled (const bool isActiveIn)
{
   this->enabled = isActiveIn;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicMarkup::isEnabled () const
{
   return this->enabled;
}

//-----------------------------------------------------------------------------
//
void QEGraphicMarkup::setSelected (const bool selectedIn)
{
   this->selected = selectedIn;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicMarkup::isSelected () const
{
   return this->selected;
}

//-----------------------------------------------------------------------------
// Place holders
//
void QEGraphicMarkup::mousePress (const QPointF&, const Qt::MouseButton) { }
void QEGraphicMarkup::mouseRelease   (const QPointF&, const Qt::MouseButton) { }
void QEGraphicMarkup::mouseMove (const QPointF&) { }


//=============================================================================
// QEGraphicAreaMarkup
//=============================================================================
//
QEGraphicAreaMarkup::QEGraphicAreaMarkup (QEGraphic* ownerIn) : QEGraphicMarkup (ownerIn)
{
   this->pen.setColor (QColor (0xC08080));   // redish gray
   this->origin = QPointF (0.0, 0.0);
}

//-----------------------------------------------------------------------------
//
void QEGraphicAreaMarkup::mousePress (const QPointF& realMousePosition, const Qt::MouseButton button)
{
   if (button == Qt::LeftButton) {
      this->origin = realMousePosition;
      this->current = realMousePosition;
      this->setVisible (true);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicAreaMarkup::mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button)
{
   if (button == Qt::LeftButton) {
      this->current = realMousePosition;
      if (this->isValidArea ()) {
         emit this->getOwner()->areaDefinition (this->origin, this->current);
      }
      this->setSelected (false);
      this->setVisible (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicAreaMarkup::mouseMove  (const QPointF& realMousePosition)
{
   this->current = realMousePosition;
   bool valid = this->isValidArea ();
   this->pen.setColor (valid ? QColor (0x60E060)    // greenish
                             : QColor (0xC08080));  // redish gray
}

//-----------------------------------------------------------------------------
// Must be distinctly tall and thin or wide and short.
//
bool QEGraphicAreaMarkup::isValidArea () const
{
   const int minDiff = 8;
   const QPoint diff = this->getOwner()->pixelDistance (this->origin, this->current);

   bool xokay = ((diff.x () > minDiff) && (diff.x () > ABS (3 * diff.y ())));
   bool yokay = ((diff.y () > minDiff) && (diff.y () > ABS (3 * diff.x ())));

   return xokay || yokay;
}

//-----------------------------------------------------------------------------
//
void QEGraphicAreaMarkup::plotMarkup ()
{
   QEGraphic::DoubleVector xdata;
   QEGraphic::DoubleVector ydata;

   xdata << this->current.x ();  ydata << this->current.y ();
   xdata << this->origin.x ();   ydata << this->current.y ();
   xdata << this->origin.x ();   ydata << this->origin.y ();
   xdata << this->current.x ();  ydata << this->origin.y ();
   xdata << this->current.x ();  ydata << this->current.y ();

   this->plotCurve (xdata, ydata);
}


//=============================================================================
// QEGraphicLineMarkup
//=============================================================================
//
QEGraphicLineMarkup::QEGraphicLineMarkup (QEGraphic* ownerIn) : QEGraphicMarkup (ownerIn)
{
   this->pen.setColor(QColor (0x80C0E0));  // blueish
   this->origin = QPointF (0.0, 0.0);
}

//-----------------------------------------------------------------------------
//
QPointF QEGraphicLineMarkup::getSlope () const
{
   return this->current - this->origin;
}

//-----------------------------------------------------------------------------
//
void QEGraphicLineMarkup::mousePress (const QPointF& realMousePosition, const Qt::MouseButton button)
{
   if (button == Qt::RightButton) {
      this->origin = realMousePosition;
      this->current = realMousePosition;
      this->setVisible (true);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicLineMarkup::mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button)
{
   if (button == Qt::RightButton) {
      this->current = realMousePosition;
      emit this->getOwner()->lineDefinition (this->origin, this->current);
      this->setSelected (false);
      this->setVisible (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicLineMarkup::mouseMove    (const QPointF& realMousePosition)
{
   this->current = realMousePosition;
}

//-----------------------------------------------------------------------------
//
void QEGraphicLineMarkup::plotMarkup ()
{
   QEGraphic::DoubleVector xdata;
   QEGraphic::DoubleVector ydata;

   xdata << this->current.x ();  ydata << this->current.y ();
   xdata << this->origin.x ();   ydata << this->origin.y ();

   this->plotCurve (xdata, ydata);
}


//=============================================================================
// QEGraphicCrosshairsMarkup
//=============================================================================
//
QEGraphicCrosshairsMarkup::QEGraphicCrosshairsMarkup (QEGraphic* ownerIn) : QEGraphicMarkup (ownerIn)
{
   this->pen.setColor (QColor (0xA0A0A0));  // light grayish
   this->cursor = Qt::PointingHandCursor;
}

//-----------------------------------------------------------------------------
//
bool QEGraphicCrosshairsMarkup::isOver (const QPointF& point, int& distance) const
{
   return this->isOverHere (this->current, point, distance);
}

//-----------------------------------------------------------------------------
//
void QEGraphicCrosshairsMarkup::mousePress (const QPointF& realMousePosition, const Qt::MouseButton)
{
   this->current = realMousePosition;
   emit this->getOwner()->crosshairsMove (this->current);
   this->pen.setColor (QColor (0x606060));  // dark grayish
}

//-----------------------------------------------------------------------------
//
void QEGraphicCrosshairsMarkup::mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton)
{
   this->current = realMousePosition;
   this->setSelected (false);
   this->pen.setColor (QColor (0xA0A0A0));  // light grayish
}

//-----------------------------------------------------------------------------
//
void QEGraphicCrosshairsMarkup::mouseMove (const QPointF& realMousePosition)
{
   this->current = realMousePosition;
   emit this->getOwner()->crosshairsMove (this->current);
}

//-----------------------------------------------------------------------------
//
void QEGraphicCrosshairsMarkup::setVisible (const bool visibleIn)
{
   double xmin;
   double xmax;
   double ymin;
   double ymax;
   QPointF middle;

   QEGraphicMarkup::setVisible (visibleIn);

   if (visibleIn) {
      this->getOwner()->xAxis->getRange (xmin, xmax);
      this->getOwner()->yAxis->getRange (ymin, ymax);

      middle = QPointF ((xmin + xmax)/2.0, (ymin + ymax)/2.0);
      this->current = middle;
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicCrosshairsMarkup::plotMarkup ()
{
   QEGraphic::DoubleVector xdata;
   QEGraphic::DoubleVector ydata;
   double min;
   double max;

   this->getOwner()->yAxis->getRange (min, max);
   xdata.clear ();    ydata.clear ();
   xdata << current.x ();  ydata << min;
   xdata << current.x ();  ydata << max;
   this->plotCurve (xdata, ydata);

   this->getOwner()->xAxis->getRange (min, max);
   xdata.clear ();  ydata.clear ();
   xdata << min;    ydata << current.y ();
   xdata << max;    ydata << current.y ();
   this->plotCurve (xdata, ydata);
}


//=============================================================================
// QEGraphicHVBaseMarkup
//=============================================================================
//
QEGraphicHVBaseMarkup::QEGraphicHVBaseMarkup
    (QEGraphic* ownerIn, const int instanceIn) : QEGraphicMarkup (ownerIn)
{
   this->instance = instanceIn;
   this->visible = false;
   this->enabled = false;
}

//-----------------------------------------------------------------------------
//
void QEGraphicHVBaseMarkup::mousePress (const QPointF& realMousePosition, const Qt::MouseButton button)
{
   switch (button) {
      case Qt::LeftButton:
         this->current = realMousePosition;
         this->setEnabled (true);
         break;

      case Qt::RightButton:
         this->setEnabled (false);
         this->setSelected (false);

      default:
         this->setSelected (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEGraphicHVBaseMarkup::mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton)
{
   this->current = realMousePosition;
   this->setSelected (false);
}

//-----------------------------------------------------------------------------
//
void QEGraphicHVBaseMarkup::mouseMove (const QPointF& realMousePosition)
{
   this->current = realMousePosition;
}

//-----------------------------------------------------------------------------
//
void QEGraphicHVBaseMarkup::setColours (const unsigned int baseRGB)
{
   this->pen.setColor   (QColor (baseRGB & 0x606060));   // dark line
   this->brushDisabled = QColor (baseRGB | 0xc0c0c0);    // washed out
   this->brushEnabled  = QColor (baseRGB | 0x808080);    // normal
   this->brushSelected = QColor (baseRGB | 0x000000);    // full saturation
}

//-----------------------------------------------------------------------------
//
void QEGraphicHVBaseMarkup::plotMarkup ()
{
   QPoint item [6];
   double xmin, xmax;
   double ymin, ymax;
   QEGraphic::DoubleVector xdata;
   QEGraphic::DoubleVector ydata;
   QPointF poiF;
   QPoint  poi;
   QPointF itemF;

   this->getLine (xmin, xmax, ymin, ymax);

   if (this->isEnabled ()) {
      xdata.clear (); ydata.clear ();
      xdata << xmin; ydata << ymin;
      xdata << xmax; ydata << ymax;
      this->pen.setStyle (Qt::DashDotLine);
      this->brush.setStyle (Qt::NoBrush);
      this->plotCurve (xdata, ydata);
   }

   this->getShape (item);
   poiF = QPointF (xmax, ymax);
   poi = this->getOwner()->realToPoint (poiF);

   xdata.clear (); ydata.clear ();
   for (int j = 0; j <  ARRAY_LENGTH (item); j++) {
      itemF = this->getOwner()->pointToReal (poi + item [j]);
      xdata << itemF.x (); ydata << itemF.y ();
   }

   this->pen.setStyle (Qt::SolidLine);
   if (this->isSelected ()) {
      this->brush.setColor (this->brushSelected);
   } else if (this->isEnabled ()) {
      this->brush.setColor (this->brushEnabled);
   } else {
      this->brush.setColor (this->brushDisabled);
   }
   this->brush.setStyle (Qt::SolidPattern);
   this->plotCurve (xdata, ydata);
}


//=============================================================================
// QEGraphicHorizontalMarkup
//=============================================================================
//
QEGraphicHorizontalMarkup::QEGraphicHorizontalMarkup
   (QEGraphic* ownerIn, const int instanceIn) : QEGraphicHVBaseMarkup (ownerIn, instanceIn)
{
   this->cursor = Qt::SplitVCursor;

   if (instance > 2) {
      this->setColours (0xff0000);    // red
   } else {
      this->setColours (0x00ff00);    // green
   }
   this->current.setY (instance * 0.02);
}

//-----------------------------------------------------------------------------
//
bool QEGraphicHorizontalMarkup::isOver (const QPointF& point, int& distance) const
{
   double xmin;
   double xmax;
   QPointF poiF;

   this->getOwner()->xAxis->getRange (xmin, xmax);
   if (this->isEnabled ()) {
      // Allow any x to match.
      poiF = QPointF (point.x (), this->current.y ());
   } else {
      poiF = QPointF (xmax, this->current.y());
   }

   return this->isOverHere (poiF, point, distance);
}

//-----------------------------------------------------------------------------
//
void QEGraphicHorizontalMarkup::getLine (double& xmin, double& xmax, double& ymin, double& ymax)
{
   this->getOwner()->xAxis->getRange (xmin, xmax);
   ymin = ymax = this->current.y ();
}

//-----------------------------------------------------------------------------
//
void QEGraphicHorizontalMarkup::getShape (QPoint shape [])
{
   static const QPoint item [6] = { QPoint (-6, +0), QPoint (-3, +3), QPoint (+2, +3),
                                    QPoint (+2, -3), QPoint (-3, -3), QPoint (-6, +0) };

   for (int j = 0; j < ARRAY_LENGTH (item); j++) {
      shape [j] = item [j];
   }
}


//=============================================================================
// QEGraphicVerticalMarkup
//=============================================================================
//
QEGraphicVerticalMarkup::QEGraphicVerticalMarkup
   (QEGraphic* ownerIn, const int instanceIn) : QEGraphicHVBaseMarkup (ownerIn, instanceIn)
{
   this->cursor = Qt::SplitHCursor;

   if (instance > 2) {
      this->setColours (0xff00ff);    // purple
   } else {
      this->setColours (0x0000ff);    // blue
   }
   this->current.setX (instance * 0.02);
}

//-----------------------------------------------------------------------------
//
bool QEGraphicVerticalMarkup::isOver (const QPointF& point, int& distance) const
{
   double ymin;
   double ymax;
   QPointF poiF;

   this->getOwner()->yAxis->getRange (ymin, ymax);
   if (this->isEnabled ()) {
      // Allow any y to match.
      poiF = QPointF (this->current.x (), point.y ());
   } else {
      poiF = QPointF (this->current.x (), ymax);
   }

   return this->isOverHere (poiF, point, distance);
}

//-----------------------------------------------------------------------------
//
void QEGraphicVerticalMarkup::getLine (double& xmin, double& xmax, double& ymin, double& ymax)
{
   xmin = xmax = this->current.x ();
   this->getOwner()->yAxis->getRange (ymin, ymax);
}

//-----------------------------------------------------------------------------
//
void QEGraphicVerticalMarkup::getShape (QPoint shape [])
{
   static const QPoint item [6] = { QPoint (+0, +6), QPoint (-3, +3), QPoint (-3, -2),
                                    QPoint (+3, -2), QPoint (+3, +3), QPoint (+0, +6) };
   for (int j = 0; j < ARRAY_LENGTH (item); j++) {
      shape [j] = item [j];
   }
}

// end
