/*  QEGraphic.cpp
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
 *  Copyright (c) 2013, 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>

#include <QDebug>
#include <qevent.h>       // QEvent maps to qcoreevent.h, not qevent.h
#include <QECommon.h>

#include <qwt_scale_engine.h>

#include "QEGraphicMarkup.h"
#include "QEGraphic.h"

#define DEBUG qDebug () << "QEGraphic" << __FUNCTION__ <<  __LINE__

// These should be consistant with adjustMinMax functions.
//
#define MINIMUM_SPAN              (1.0e-12)
#define MAXIMUM_SPAN              (1.0e+26)

#define NUMBER_TRANISTION_STEPS   6

// Each markup has a pre-allocated slot.
//
#define AREA_MARKUP            0
#define LINE_MARKUP            1
#define CROSSHAIRES_MARKUP     2
#define HORIZONTAL_1_MARKUP    3
#define HORIZONTAL_2_MARKUP    4
#define HORIZONTAL_3_MARKUP    5
#define HORIZONTAL_4_MARKUP    6
#define VERTICAL_1_MARKUP      7
#define VERTICAL_2_MARKUP      8
#define VERTICAL_3_MARKUP      9
#define VERTICAL_4_MARKUP     10


//==============================================================================
// QEGraphic::Axis class
//==============================================================================
//
QEGraphic::Axis::Axis (QwtPlot* plotIn, const int axisIdIn)
{
   this->plot = plotIn;
   this->axisId = axisIdIn;
   this->plot->setAxisScaleEngine (this->axisId, new QwtLinearScaleEngine);

   // Set defaults.
   //
   this->isLogarithmic = false;
   this->scale = 1.0;
   this->offset = 0.0;
   this->axisEnabled = true;

   // Set 'current' ranges.
   //
   this->current.setRange (0.0, 1.0);
   this->source = this->current;
   this->target = this->current;
   this->transitionCount = 0;
   this->intervalMode = QEGraphic::SelectByValue;
   this->intervalValue = 8;
   this->determineAxisScale ();
}

//------------------------------------------------------------------------------
//
QEGraphic::Axis::~Axis ()
{
   this->plot->setAxisScaleEngine (this->axisId, NULL);
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setRange (const double minIn, const double maxIn,
                                const AxisMajorIntervalModes modeIn, const int valueIn,
                                const bool immediate)
{
   QEDisplayRanges newTarget;
   bool rescaleIsRequired;

   newTarget.setRange (minIn, LIMIT (maxIn, minIn + MINIMUM_SPAN, minIn + MAXIMUM_SPAN));

   // Is this a significant change? Hypothosize not.
   //
   rescaleIsRequired = false;

   // Avoid rescaling for trivial changes.
   //
   if (!this->target.isSimilar (newTarget, 0.001)) {
      this->target = newTarget;
      if (immediate) {
         // Immediate - no animation.
         //
         this->source = this->target;
         this->current = this->target;
         this->transitionCount = 0;
      } else {
         // Not immediate - provide an animated transition.
         // New source is where we currently are.
         // Set up transition count down.
         //
         this->source = this->current;
         this->transitionCount = NUMBER_TRANISTION_STEPS;
      }
      rescaleIsRequired = true;
   }

   if (this->intervalMode != modeIn) {
      this->intervalMode = modeIn;
      rescaleIsRequired = true;
   }

   if (this->intervalValue != valueIn) {
      this->intervalValue = valueIn;
      rescaleIsRequired = true;
   }

   // Something changed  - re do the scaling.
   //
   if (rescaleIsRequired) {
      this->determineAxisScale ();
   }
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::getRange (double& min, double& max)
{
    min = this->useMin;
    max = this->useMax;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::doDynamicRescaling ()
{
   bool result = false;

   if (this->transitionCount > 0) {
      this->transitionCount--;

      // Calulate the new current point and re set axis scale.
      //
      this->current = QEGraphic::calcTransitionPoint (this->source, this->target,
                                                      this->transitionCount);
      this->determineAxisScale ();
      result = true;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::determineAxisScale ()
{
   int canvasSize;
   int number;

   if (this->isLogarithmic) {
      this->current.adjustLogMinMax (this->useMin, this->useMax, this->useStep);
   } else {
      if (this->intervalMode == QEGraphic::SelectBySize) {
         switch (this->axisId) {
            case QwtPlot::xTop:
            case QwtPlot::xBottom:
               canvasSize = this->plot->canvas()->width ();
               break;

            case QwtPlot::yLeft:
            case QwtPlot::yRight:
               canvasSize = this->plot->canvas()->height ();
               break;

            default:
               canvasSize = 800;   // avoid compiler warning
               break;
         }

         number = canvasSize / MAX (1, this->intervalValue);
      } else {
         number = this->intervalValue;
      }
      current.adjustMinMax (number, false, this->useMin, this->useMax, this->useStep);

      // Subtract/add tolerance as Qwt Axis ploting of minor ticks a bit slack.
      //
      this->useMin = this->useMin - (0.01 * this->useStep);
      this->useMax = this->useMax + (0.01 * this->useStep);
   }

   // This is the only place we set the actual axis scale.
   //
   this->plot->setAxisScale (this->axisId, this->useMin, this->useMax, this->useStep);
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::pointToReal (const int pos) const
{
   double x;

   // Perform basic inverse transformation - pixel to axis coordinates.
   //
   x = this->plot->invTransform (this->axisId, pos);

   // Scale from axis to real world units.
   //
   x = (x - this->offset) / this->scale;

   return x;
}

//------------------------------------------------------------------------------
//
int QEGraphic::Axis::realToPoint (const double pos) const
{
   int x;
   double useX;

   // Do linear scaling (if any) followed by log scaling if required.
   //
   useX = this->scale * (double) pos + this->offset;

   if (this->isLogarithmic) {
      useX = MAX (1.0E-20, useX);   // avoid going out of range
   }

   // Perform basic plot transformation.
   //
   x = this->plot->transform (this->axisId, useX);

   return x;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::scaleValue (const double coordinate) const
{
   double x;

   x = this->scale * coordinate + this->offset;
   if (this->isLogarithmic) {
      x = MAX (1.0E-20, x);   // avoid going out of range
   }

   return x;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setAxisEnable (const bool axisEnable)
{
   this->axisEnabled = axisEnable;
   this->plot->enableAxis (this->axisId, this->axisEnabled);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::getAxisEnable () const
{
   return this->axisEnabled;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setScale (const double scaleIn)
{
   this->scale = scaleIn;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::getScale () const
{
   return this->scale;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setOffset (const double offsetIn)
{
   this->offset = offsetIn;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::getOffset () const
{
   return this->offset;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setLogarithmic (const bool isLogarithmicIn)
{
   if (this->isLogarithmic != isLogarithmicIn) {
      this->isLogarithmic = isLogarithmicIn;

      if (this->isLogarithmic) {
#if QWT_VERSION >= 0x060100
        this->plot->setAxisScaleEngine (this->axisId, new QwtLogScaleEngine);
#else
        this->plot->setAxisScaleEngine (this->axisId, new QwtLog10ScaleEngine);
#endif
      } else {
         this->plot->setAxisScaleEngine (this->axisId, new QwtLinearScaleEngine);
      }

      // Do immediate trasition and reset
      //
      this->determineAxisScale ();
      this->transitionCount = 0;
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::getLogarithmic () const
{
   return this->isLogarithmic;
}

//==============================================================================
// QEGraphic class
//==============================================================================
//
QEGraphic::QEGraphic (QWidget* parent) : QWidget (parent)
{
   this->plot = new QwtPlot (parent);
   this->construct ();
}

//------------------------------------------------------------------------------
//
QEGraphic::QEGraphic (const QString& title, QWidget* parent) : QWidget (parent)
{
   this->plot = new QwtPlot (title, parent);
   this->construct ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::construct ()
{
   // Create a louout within the containing widget.
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);
   this->layout->setSpacing (0);
   this->layout->addWidget (this->plot);

   this->plotGrid = new QwtPlotGrid ();
   this->plotGrid->attach (this->plot);

   this->xAxis = new Axis (this->plot, QwtPlot::xBottom);
   this->yAxis = new Axis (this->plot, QwtPlot::yLeft);

   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      this->markups [j] = NULL;
   }

   this->markups [AREA_MARKUP] = new QEGraphicAreaMarkup (this);
   this->markups [LINE_MARKUP] = new QEGraphicLineMarkup (this);
   this->markups [CROSSHAIRES_MARKUP] = new QEGraphicCrosshairsMarkup (this);
   this->markups [HORIZONTAL_1_MARKUP] = new QEGraphicHorizontalMarkup (this, 1);
   this->markups [HORIZONTAL_2_MARKUP] = new QEGraphicHorizontalMarkup (this, 2);
   this->markups [HORIZONTAL_3_MARKUP] = new QEGraphicHorizontalMarkup (this, 3);
   this->markups [HORIZONTAL_4_MARKUP] = new QEGraphicHorizontalMarkup (this, 4);
   this->markups [VERTICAL_1_MARKUP] = new QEGraphicVerticalMarkup (this, 1);
   this->markups [VERTICAL_2_MARKUP] = new QEGraphicVerticalMarkup (this, 2);
   this->markups [VERTICAL_3_MARKUP] = new QEGraphicVerticalMarkup (this, 3);
   this->markups [VERTICAL_4_MARKUP] = new QEGraphicVerticalMarkup (this, 4);

   // Set defaults.
   //
   this->rightIsDefined = false;

   this->pen = QPen (QColor (0, 0, 0, 255));  // black
   // go with default brush for now.
   this->hint = QwtPlotItem::RenderAntialiased;
   this->style = QwtPlotCurve::Lines;

#if QWT_VERSION < 0x060100
   this->plot->setCanvasLineWidth (1);
#endif
   this->plot->setLineWidth (1);

   this->plot->canvas()->setMouseTracking (true);
   this->plot->canvas()->installEventFilter (this);

   // Refresh Dynamic Rescaling the stip chart at 20Hz.
   //
   this->tickTimer = new QTimer (this);
   connect (this->tickTimer, SIGNAL (timeout ()), this, SLOT (tickTimeout ()));
   this->tickTimer->start (50);  // mSec = 0.05 s
}

//------------------------------------------------------------------------------
//
QEGraphic::~QEGraphic ()
{
   // Note: must detach curves and grids, otherwise some (older) versions of qwt
   // cause a segmentation fault when the associated QwtPolot object is deleted.
   //
   this->releaseCurves ();

   if (this->plotGrid) {
      this->plotGrid->detach();
      delete this->plotGrid;
      this->plotGrid  = NULL;
   }

   delete this->xAxis;
   delete this->yAxis;

   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      if (this->markups [j]) {
         delete this->markups [j];
      }
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::doDynamicRescaling ()
{
   bool result;
   bool a, b;

   a = this->xAxis->doDynamicRescaling ();
   b = this->yAxis->doDynamicRescaling ();

   result = a||b;
   if (result) {
      this->graphicReplot ();
   }

   return result;
}

//------------------------------------------------------------------------------
//
void  QEGraphic::tickTimeout ()
{
   this->doDynamicRescaling ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setBackgroundColour (const QColor colour)
{
#if QWT_VERSION >= 0x060000
   this->plot->setCanvasBackground (QBrush (colour));
#else
   this->plot->setCanvasBackground (colour);
#endif
}

//------------------------------------------------------------------------------
//
void QEGraphic::setGridPen (const QPen& pen)
{
   this->plotGrid->setPen (pen);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCrosshairsVisible (const bool isVisible)
{
   QEGraphicMarkup* markup =  this->markups [CROSSHAIRES_MARKUP];
   if (markup) {
      markup->setVisible (isVisible);
   }
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::pointToReal (const QPoint& pos) const
{
   double x, y;

   x = this->xAxis->pointToReal (pos.x ());
   y = this->yAxis->pointToReal (pos.y ());

   return QPointF (x, y);
}

//------------------------------------------------------------------------------
//
QPoint QEGraphic::realToPoint (const QPointF& pos) const
{
   int x, y;

   x = this->xAxis->realToPoint (pos.x ());
   y = this->yAxis->realToPoint (pos.y ());

   return QPoint (x, y);
}

//------------------------------------------------------------------------------
//
void QEGraphic::releaseCurveList (CurveList& list)
{
   for (int j = 0; j < list.size (); j++) {
      QwtPlotCurve* curve = list.value (j);
      if (curve) {
         curve->detach ();
         delete curve;
      }
   }

   // This clears the list of (now) dangaling curve references.
   //
   list.clear ();
}

//------------------------------------------------------------------------------
// Releases all curves
//
void QEGraphic::releaseCurves ()
{
   this->releaseCurveList (this->userCurveList);
   this->releaseCurveList (this->markupCurveList);
}

//------------------------------------------------------------------------------
//
void QEGraphic::attchOwnCurve (QwtPlotCurve* curve)
{
   if (curve) {
      curve->attach (this->plot);
      this->userCurveList.append (curve);
   }
}

//------------------------------------------------------------------------------
//
QwtPlotCurve* QEGraphic::createCurveData (const DoubleVector& xData, const DoubleVector& yData)
{
   QwtPlotCurve* curve;
   DoubleVector useXData;
   DoubleVector useYData;
   int n;
   curve = new QwtPlotCurve ();

   // Set curve propeties using current curve attributes.
   //
   curve->setPen (this->getCurvePen ());
   curve->setBrush (this->getCurveBrush ());
   curve->setRenderHint (this->getCurveRenderHint ());
   curve->setStyle (this->getCurveStyle ());

   // Scale data as need be. Underlying Qwr widget does basic transformation,
   // but we need to do any required real world/log scaling.
   //
   useXData.clear();
   useYData.clear();
   n = MIN (xData.size (), yData.size ());
   for (int j = 0; j < n; j++) {
      double x, y;

      x = this->xAxis->scaleValue (xData.value (j));
      useXData.append (x);

      y = this->yAxis->scaleValue (yData.value (j));
      useYData.append (y);
   }

#if QWT_VERSION >= 0x060000
   curve->setSamples (useXData, useYData);
#else
   curve->setData (useXData, useYData);
#endif

   // Attach new curve to the plot object.
   //
   curve->attach (this->plot);

   return curve;
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotCurveData (const DoubleVector& xData, const DoubleVector& yData)
{
   QwtPlotCurve* curve;
   curve = this->createCurveData (xData, yData);
   this->userCurveList.append (curve);
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotMarkupCurveData (const DoubleVector& xData, const DoubleVector& yData)
{
   QwtPlotCurve* curve;
   curve = this->createCurveData (xData, yData);
   this->markupCurveList.append (curve);
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotMarkups ()
{
   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      if (this->markups [j]) {
         this->markups [j]->plot ();
      }
   }
}

//------------------------------------------------------------------------------
//
void QEGraphic::graphicReplot ()
{
   this->releaseCurveList (this->markupCurveList);
   this->plotMarkups ();
   this->plot->replot ();
}

//------------------------------------------------------------------------------
//
bool QEGraphic::rightButtonPressed () const
{
   return this->rightIsDefined;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getSlopeIsDefined (QPointF& slope) const
{
   bool result;
   QEGraphicLineMarkup* markup = static_cast <QEGraphicLineMarkup*> (this->markups [LINE_MARKUP]);

   if (markup && markup->isVisible ()) {
      slope = markup->getSlope ();
      result = true;
   } else {
      slope = QPointF (0.0, 0.0);
      result = false;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QPoint QEGraphic::pixelDistance (const QPointF& from, const QPointF& to) const
{
   QPoint pointFrom = this->realToPoint (from);
   QPoint pointTo = this->realToPoint (to);
   return pointTo - pointFrom;
}

//------------------------------------------------------------------------------
//
QEGraphicMarkup* QEGraphic::mouseIsOverMarkup ()
{
   QEGraphicMarkup* search;
   int minDistance;

   search = NULL;
   minDistance = 100000;  // some unfeasible large distance. A real distance much smaller.

   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      QEGraphicMarkup* markup = this->markups [j];
      if (markup) {
         int dist;
         if (markup->isOver (this->realMousePosition, dist)) {
            // Note: <=  operator. All things being equal this means:
            // Last in, best dressed.  That is essentially the same as the
            // plotMarkups, i.e. we find the markup the user can see.
            //
            if (dist <= minDistance) {
               minDistance = dist;
               search = markup;
            }
         }
      }
   }
   return search;
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMousePress (QMouseEvent* mouseEvent)
{
   Qt::MouseButton button;
   QEGraphicMarkup* search;

   button = mouseEvent->button ();
   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   search = NULL;

   // We can always "find" AREA_MARKUP/LINE_MARKUP.
   //
   if (button == Qt::LeftButton) {
      search = this->markups [AREA_MARKUP];
   } else if (button == Qt::RightButton) {
      search = this->markups [LINE_MARKUP];
   }

   // Is press over/closer an existing/visible markup?
   // Iff we found something, then replace search.
   //
   QEGraphicMarkup* t = this->mouseIsOverMarkup ();
   if (t) search = t;

   // Mark this markup as selected.
   //
   if (search) {
      search->setSelected (true);
   }

   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      QEGraphicMarkup* markup = this->markups [j];
      if (markup && markup->isSelected ()) {
         markup->mousePress (this->realMousePosition, button);
      }
   }

   if (button == Qt::RightButton) {
      this->rightIsDefined = true;
   }

   // Treat as a mouse move as well.
   this->canvasMouseMove (mouseEvent, true);
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMouseRelease (QMouseEvent* mouseEvent)
{
   Qt::MouseButton button;

   button = mouseEvent->button ();
   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      QEGraphicMarkup* markup = this->markups [j];
      if (markup && markup->isSelected ()) {
         markup->mouseRelease (this->realMousePosition, button);
         this->plot->canvas()->setCursor (Qt::CrossCursor);
      }
   }

   if (button == Qt::RightButton) {
      this->rightIsDefined = false;
   }

   // Treat as a mouse move as well.
   this->canvasMouseMove (mouseEvent, true);
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMouseMove (QMouseEvent* mouseEvent, const bool isButtonAction)
{
   bool replotIsRequired;

   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   replotIsRequired = false;
   for (int j = 0; j < ARRAY_LENGTH (this->markups); j++) {
      QEGraphicMarkup* markup = this->markups [j];
      if (markup && markup->isSelected ()) {
         markup->mouseMove (this->realMousePosition);
         // A selected item will need replotted.
         //
         replotIsRequired = true;
      }
   }

   if (replotIsRequired | isButtonAction ) {
      this->graphicReplot ();
   }

   if (!replotIsRequired) {
      // Nothing selected. Is cursor over markup
      //
      QEGraphicMarkup* search;
      QCursor cursor;

      search = this->mouseIsOverMarkup ();
      if (search) {
         cursor = search->getCursor ();
      } else {
         cursor = Qt::CrossCursor;
      }
      this->plot->canvas()->setCursor (cursor);
   }

   emit mouseMove (this->realMousePosition);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::eventFilter (QObject* obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   QWheelEvent* wheelEvent = NULL;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMousePress (mouseEvent);
            return true;  // we have handled this mouse press
         }
         break;

      case QEvent::MouseButtonRelease:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMouseRelease (mouseEvent);
            return true;  // we have handled this mouse press
         }
         break;

      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMouseMove (mouseEvent, false);
            return true;  // we have handled move nouse event
         }
         break;

      case QEvent::Wheel:
         wheelEvent = static_cast<QWheelEvent *> (event);
         if (obj == this->plot->canvas ()) {

            emit wheelRotate (this->realMousePosition,
                              wheelEvent->delta ());

            return true;  // we have handled wheel event
         }
         break;

      case QEvent::Resize:
         if (obj == this->plot->canvas ()) {
            this->graphicReplot ();
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setXRange (const double min, const double max,
                           const AxisMajorIntervalModes mode, const int value,
                           const bool immediate)
{
   this->xAxis->setRange (min, max, mode, value, immediate);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYRange (const double min, const double max,
                           const AxisMajorIntervalModes mode, const int value,
                           const bool immediate)
{
   this->yAxis->setRange (min, max, mode, value, immediate);
}

//------------------------------------------------------------------------------
//
void QEGraphic::replot ()
{
   // User artefacts already plotted - now do markup plots.
   //
   this->plotMarkups ();
   this->plot->replot ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurvePen (const QPen& penIn)
{
   this->pen = penIn;
}

//------------------------------------------------------------------------------
//
QPen QEGraphic::getCurvePen () const {
   return this->pen;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveBrush (const QBrush& brushIn)
{
   this->brush = brushIn;
}

//------------------------------------------------------------------------------
//
QBrush QEGraphic::getCurveBrush () const
{
   return this->brush;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveRenderHint (const QwtPlotItem::RenderHint hintIn)
{
   this->hint = hintIn;
}

//------------------------------------------------------------------------------
//
QwtPlotItem::RenderHint QEGraphic::getCurveRenderHint ()
{
   return this->hint;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveStyle (const QwtPlotCurve::CurveStyle styleIn)
{
   this->style = styleIn;
}

//------------------------------------------------------------------------------
//
QwtPlotCurve::CurveStyle QEGraphic::getCurveStyle ()
{
   return this->style;
}

//------------------------------------------------------------------------------
// static
//------------------------------------------------------------------------------
//
QEDisplayRanges QEGraphic::calcTransitionPoint (const QEDisplayRanges& start,
                                                const QEDisplayRanges& finish,
                                                const int step)
{
   QEDisplayRanges result;
   double minimum;
   double maximum;

   if (step <= 0) {
      result = finish;
   } else if (step >= NUMBER_TRANISTION_STEPS) {
      result = start;
   } else  {

      // Truely in transition - perform a linear interpolation.
      //
      const double s = double (step) / (double) NUMBER_TRANISTION_STEPS;
      const double f = 1.0 - s;

      minimum = (s * start.getMinimum ()) + (f * finish.getMinimum ());
      maximum = (s * start.getMaximum ()) + (f * finish.getMaximum ());

      result.setRange (minimum, maximum);
   }
   return result;
}

// end
