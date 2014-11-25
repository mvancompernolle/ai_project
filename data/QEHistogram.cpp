/*  QEHistogram.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
#include <QBrush>
#include <QPen>

#include <QEScaling.h>
#include <QEHistogram.h>
#include <QECommon.h>
#include <QEDisplayRanges.h>

#define DEBUG qDebug () << "QEHistogram (" << __FUNCTION__ << __LINE__ << "): "

// Magic null values - use NaN ?
// 'Unlikely' to occur and can be exactly represented as a double.
//
static const double  NO_DATA_VALUE = -1073741824.0;
static const QColor  NO_COLOUR_VALUE = QColor (3, 1, 4, 2);   // Pi colour ;-)


static const double  MINIMUM_SPAN  = +1.0E-6;
static const int     MAX_CAPACITY  = 10000;

//------------------------------------------------------------------------------
//
static bool isNullDataValue (const double x) {
   return x == NO_DATA_VALUE;
}

//------------------------------------------------------------------------------
//
static bool isNullColourValue (const QColor& x) {
   return x == NO_COLOUR_VALUE;
}


//------------------------------------------------------------------------------
//
QEHistogram::QEHistogram (QWidget *parent) : QFrame (parent)
{
   // Set default property values
   // Super class....
   //
   this->setMinimumSize (80, 40);
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // And local properties.
   //
   this->mBarColour = QColor (55, 155, 255);   // blue
   this->mDrawBorder = true;
   this->mAutoScale = false;
   this->mAutoBarGapWidths = false;
   this->mLogScale = false;

   this->mGap = 3;                // 0 .. 10
   this->mBarWidth = 8;           // 1 .. 80
   this->mBaseLine = 0.0;
   this->mMinimum = 0.0;
   this->mMaximum = 10.0;
   this->mOrientation = Qt::Horizontal;
   this->mTestSize = 0;

   // Create internal widgets
   //
   this->layout = new QVBoxLayout (this);
   this->layout->setMargin (3);
   this->layout->setSpacing (3);

   this->histogramArea = new QWidget (this);
   this->histogramArea->setMouseTracking (true);
   this->layout->addWidget (this->histogramArea);

   this->scrollbar = new QScrollBar (this);
   this->scrollbar->setOrientation (Qt::Horizontal);
   this->scrollbar->setRange (0, 0);
   this->layout->addWidget (this->scrollbar);

   this->dataArray.clear ();
   this->dataArray.reserve (100);
   this->numberDisplayed = 0;
   this->firstDisplayed = 0;

   // Do this only once, not in paintEvent as it causes another paint event.
   //
   this->histogramArea->setAutoFillBackground (false);
   this->histogramArea->setBackgroundRole (QPalette::NoRole);

   QObject::connect (this->scrollbar, SIGNAL (valueChanged (int)),
                     this,     SLOT (scrollBarValueChanged (int)));

   this->histogramArea->installEventFilter (this);
}


//------------------------------------------------------------------------------
//  Define default size for this widget class.
//
QSize QEHistogram::sizeHint () const
{
   return QSize (160, 80);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clearValue (const int index)
{
   this->setValue (index, NO_DATA_VALUE);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clearColour (const int index)
{
   this->setColour (index, NO_COLOUR_VALUE);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clear ()
{
   this->firstDisplayed = 0;
   this->dataArray.clear ();
   this->colourArray.clear ();
   this->update ();
}

//------------------------------------------------------------------------------
//
void QEHistogram::setColour (const int index, const QColor& value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->colourArray.count () < index + 1) {
         this->colourArray.append (NO_COLOUR_VALUE);
      }

      this->colourArray [index] = value;

      while ((this->colourArray.count () > 0) &&
             (isNullColourValue (this->colourArray.last ()))) {
         this->colourArray.remove (this->colourArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
void QEHistogram::setValue (const int index, const double value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->dataArray.count () < index + 1) {
         this->dataArray.append (NO_DATA_VALUE);
      }

      this->dataArray [index] = value;

      while ((this->dataArray.count () > 0) &&
             (isNullDataValue (this->dataArray.last ()))) {
         this->dataArray.remove (this->dataArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QEHistogram::count () const
{
   return this->dataArray.count ();
}

//------------------------------------------------------------------------------
//
double QEHistogram::value (const int index) const
{
   return this->dataArray.value (index, 0.0);
}

//------------------------------------------------------------------------------
//
void QEHistogram::setValues (const DataArray& values)
{
   this->dataArray = values;
   this->update ();
}

//------------------------------------------------------------------------------
//
QEHistogram::DataArray QEHistogram::values () const
{
   return this->dataArray;
}

//------------------------------------------------------------------------------
//
QColor QEHistogram::getPaintColour (const int index) const
{
   QColor result;

   result = this->colourArray.value (index, NO_COLOUR_VALUE);
   if (isNullColourValue (result)) {
      result = this->mBarColour;
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::scrollMaximum () const
{
   return MAX (0, this->dataArray.count () - this->numberDisplayed);
}

//------------------------------------------------------------------------------
//
void QEHistogram::scrollBarValueChanged (int value)
{
   this->firstDisplayed = LIMIT (value, 0, (MAX_CAPACITY - 1));
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEHistogram::firstBarLeft () const
{
   return this->paintArea.left ();
}

//------------------------------------------------------------------------------
//
QRect QEHistogram::fullBarRect  (const int position) const
{
   // paintArea defines overall paint area.
   //
   const int top = this->paintArea.top ();
   const int left = this->firstBarLeft () +
                    (this->useBarWidth + this->useGap + 1) * position;
   const int bottom = this->paintArea.bottom ();
   const int right = left + this->useBarWidth;

   QRect result;

   result.setTop (top);
   result.setLeft (left);
   result.setBottom (bottom);
   result.setRight (right);

   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfPosition (const int x, const int y) const
{
   // Convert from histogram co-ordinates (which is what the external world sees)
   // to local internal widger co-ordinates.
   //
   const int hax = x - this->histogramArea->geometry ().left ();
   const int hay = y - this->histogramArea->geometry ().top ();

   const int guess = (hax - this->firstBarLeft ()) /
                     MAX (1, this->useBarWidth + this->useGap + 1);

   // Add +/- 2 - very conservative.
   //
   const int lower = MAX (guess - 2, 0);
   const int upper = MIN (guess + 2, this->numberDisplayed - 1);

   int result = -1;

   for (int j = lower; j <= upper; j++) {
      QRect jbar = this->fullBarRect (j);

      if (hax >= jbar.left () && hax <= jbar.right () &&
          hay >= jbar.top ()  && hay <= jbar.bottom ()) {
         // found it.
         //
         result = j + this->firstDisplayed;
         if (result >= this->count ()) result = -1;
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfPosition (const QPoint& p) const
{
   return this->indexOfPosition (p.x (), p.y ());
}

//------------------------------------------------------------------------------
//
bool QEHistogram::paintItem (QPainter & painter,
                             const int position,
                             const int valueIndex) const
{
   const int finishRight = this->paintArea.right ();
   QRect bar;
   double value;
   double base;
   double baseLineFraction;
   double valueFraction;
   int top;
   int bot;
   QColor colour;
   QColor boarderColour;
   QBrush brush;
   QPen pen;

   bar = this->fullBarRect (position);
   if (bar.left () >= finishRight) return false;   // Off to the side
   if (bar.right () > finishRight) {
      bar.setRight (finishRight);                  // Truncate
      if (bar.width () < 5) return false;          // Tooo small!!
   }

   value = this->dataArray.value (valueIndex, NO_DATA_VALUE);
   base = this->mBaseLine;

   // Is value invalid, i.e. un-defined BUT still in paint area?
   //
   if (isNullDataValue (value)) return true;

   if (this->mLogScale) {
      value = LOG10 (value);
      base = LOG10 (base);
   }

   valueFraction = (value             - this->drawMinimum) /
                   (this->drawMaximum - this->drawMinimum);
   valueFraction = LIMIT (valueFraction, 0.0, 1.0);

   baseLineFraction = (base              - this->drawMinimum) /
                      (this->drawMaximum - this->drawMinimum);
   baseLineFraction = LIMIT (baseLineFraction, 0.0, 1.0);

   // Top based on fraction which in turn based on value.
   // Note: top increases as value/fraction decreases.
   //
   top = bar.bottom () - (int) (valueFraction * bar.height ());
   bot = bar.bottom () - (int) (baseLineFraction * bar.height ());

   bar.setTop (top);
   bar.setBottom (bot);

   // All good to go - set up colour.
   //
   colour = this->getPaintColour (valueIndex);
   boarderColour = QEUtilities::darkColour (colour);

   if (!this->isEnabled ()) {
      colour = QEUtilities::blandColour (colour);
      boarderColour = QEUtilities::blandColour (boarderColour);
   }

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (colour);
   painter.setBrush (brush);

   pen.setWidth (1);
   if (this->mDrawBorder) {
      // Use darker version of the color for the boarder.
      //
      pen.setColor (boarderColour);
   } else {
      pen.setColor (colour);
   }
   painter.setPen (pen);

   painter.drawRect (bar);
   return true;
}

//------------------------------------------------------------------------------
// Ensure text is generated consistantly.
//
QString QEHistogram::coordinateText (const double value) const
{
   QString result;

   if (this->mLogScale) {
      // Scale, this given value, is logged, must unlog it.
      //
      result = QString ("%1").arg (EXP10 (value), 0, 'e', 0);
   } else {
      result = QString ("%1").arg (value);
   }
   return result;
}

//------------------------------------------------------------------------------
// This is like paintGrid, BUT with no actual painting.
// Maybe we could keep / cache generated value text images.
//
int QEHistogram::maxPaintTextWidth (QPainter& painter) const
{
   QFontMetrics fm = painter.fontMetrics ();
   int result;
   int j;
   double value;
   QString text;
   int w;

   result = 1;
   for (j = 0; true; j++) {
      value = this->drawMinimum + (j*this->drawMajor);
      if (value > this->drawMaximum) break;
      if (j > 1000) break;  // sainity check

      text = this->coordinateText (value);
      w = fm.width (text);
      if (result < w) result = w;

   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEHistogram::paintGrid (QPainter& painter) const
{
   const int axisOffset = QEScaling::scale (4);

   QPen pen;
   int j;
   double value;
   double fraction;
   int y;
   QFontMetrics fm = painter.fontMetrics ();
   QFont pf (this->font ());
   QString text;
   int x;

   if (this->isEnabled ()) {
      pen.setColor (QColor (0, 0, 0));        // black
   } else {
      pen.setColor (QColor (128, 128, 128));  // gray
   }
   pen.setWidth (1);
   pen.setStyle (Qt::DashLine);
   painter.setPen (pen);

   for (j = 0; true; j++) {
      value = this->drawMinimum + (j*this->drawMajor);
      if (value > this->drawMaximum) break;
      if (j > 1000) break;  // sainity check

      fraction = (value             - this->drawMinimum) /
                 (this->drawMaximum - this->drawMinimum);

      // Same idea as we used in paintItem.
      //
      y = this->paintArea.bottom () - (int) (fraction * this->paintArea.height ());

      if (j > 0) {
         painter.drawLine (this->paintArea.left () - axisOffset, y,
                           this->paintArea.right(), y);
      }

      // Centre text. For height, pointSize seems better than fm.height ()
      // painter.drawText needs bottom left coordinates.
      //
      text = this->coordinateText (value);
      x = this->paintArea.left () - fm.width (text) - 2 * axisOffset;
      y = y +  (pf.pointSize () + 1)/2;

      painter.drawText (x, y, text);
   }

   pen.setWidth (1);
   pen.setStyle (Qt::SolidLine);
   painter.setPen (pen);

   painter.drawLine (this->paintArea.left () - axisOffset, this->paintArea.top (),
                     this->paintArea.left () - axisOffset, this->paintArea.bottom () + axisOffset);

   painter.drawLine (this->paintArea.left () - axisOffset,  this->paintArea.bottom () + axisOffset,
                     this->paintArea.right (), this->paintArea.bottom () + axisOffset);
}

//------------------------------------------------------------------------------
//
void QEHistogram::paintAllItems ()
{
   const int numberGrid = 5;   // approx number of y grid lines.
   const int extra = QEScaling::scale (16);

   QPainter painter (this->histogramArea);
   QEDisplayRanges displayRange;
   double useMinimum;
   double useMaximum;

   // Draw everything with antialiasing off.
   //
   painter.setRenderHint (QPainter::Antialiasing, false);

   useMinimum = this->mMinimum;
   useMaximum = this->mMaximum;
   if (this->mAutoScale) {
      bool foundValue = false;
      double searchMinimum = +1.0E25;
      double searchMaximum = -1.0E25;
      for (int j = 0; j < this->dataArray.count (); j++) {
         double v = this->dataArray.at (j);
         if (isNullDataValue (v)) continue;
         searchMinimum  = MIN (v, searchMinimum);
         searchMaximum  = MAX (v, searchMaximum);
         foundValue = true;
      }
      if (foundValue) {
         useMinimum  = searchMinimum;
         useMaximum  = searchMaximum;
      }
   }

   // Do not allow ultra small spans, which will occur when autoscaling
   // a histogram with a single value.
   //
   if ((useMaximum - useMinimum) < MINIMUM_SPAN) {
      double midway = (useMaximum + useMinimum)/2.0;
      useMinimum = midway - MINIMUM_SPAN/2.0;
      useMaximum = midway + MINIMUM_SPAN/2.0;
   }

   // Now calc draw min max  - log of min / max if necessary.
   //
   displayRange.setRange (useMinimum, useMaximum);

   if (this->mLogScale) {
      displayRange.adjustLogMinMax (this->drawMinimum, this->drawMaximum, this->drawMajor);
   } else {
      displayRange.adjustMinMax (numberGrid, true, this->drawMinimum, this->drawMaximum, this->drawMajor);
   }

   // Define actual chart draw area ...
   //
   QRect hostWidgetArea = this->histogramArea->geometry ();
   QFont ownFont (this->font ());
   int halfPointSize = (ownFont.pointSize () + 1) / 2;
   this->paintArea.setTop (2 + halfPointSize);
   this->paintArea.setBottom (hostWidgetArea.height () - 2 - halfPointSize);
   this->paintArea.setLeft (this->maxPaintTextWidth (painter) + extra);
   this->paintArea.setRight (hostWidgetArea.width () - 2);

   // Do grid and axis - not this might tweak useMinimum/useMaximum.
   //
   this->paintGrid (painter);

   this->useGap = this->mGap;
   this->useBarWidth = this->mBarWidth;
   if (this->mAutoBarGapWidths) {
      const int n = this->dataArray.count ();

      if (n <= 1) {
         this->useGap = 0;
         this->useBarWidth = this->paintArea.width ();
      } else {
         const int markSpace = 6;
         // For large n itemWidth is essentially paintArea.width / n
         // For small n, this accounts for n bars and n-1 gaps.
         //
         int itemWidth = ((markSpace + 1) * paintArea.width ()) / ((markSpace + 1)*n - 1);
         if (itemWidth < 3) itemWidth = 3;
         this->useGap = itemWidth / markSpace;
         // There is an implicit +1 in the fullBarRect function.
         this->useBarWidth = MAX (1, itemWidth - this->useGap - 1);
      }
   }

   // Maximum number of items that could be drawn.
   //
   const int maxDrawable = this->dataArray.count () - this->firstDisplayed;

   this->numberDisplayed = 0;
   for (int posnIndex = 0; posnIndex < maxDrawable; posnIndex++) {
      int dataIndex = this->firstDisplayed + posnIndex;
      bool painted = paintItem (painter, posnIndex, dataIndex);
      if (painted) {
         this->numberDisplayed = posnIndex + 1;
      } else {
         break;
      }
   }

   // Lastly ...
   //
   const int max = this->scrollMaximum ();
   this->scrollbar->setRange (0, max);
   this->scrollbar->setVisible (max > 0);
}

//------------------------------------------------------------------------------
//
bool QEHistogram::eventFilter (QObject *obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   if (type == QEvent::Paint) {
      if (obj == this->histogramArea) {
         this->paintAllItems ();
         result = true;  // event has been handled
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEHistogram::createTestData ()
{
   this->clear ();

   for (int j = 0; j < this->mTestSize; j++) {
      double f;
      double v;
      QColor c;

      // Do special for first and last.
      //
      if (j == 0 || j == this->mTestSize - 1) {
         v = this->mMaximum;
      } else {
         f = qrand () / (double) RAND_MAX;
         v = f * (this->mMaximum - this->mMinimum) +  this->mMinimum;
      }

      f = qrand () / (double) RAND_MAX;
      c.setHsl ((int)(f * 360.0), 255, 128);

      this->dataArray << v;
      this->colourArray << c;
   }
}

//==============================================================================
// Property functions (standard)
//==============================================================================
//
// Standard propery access macro.
//
#define PROPERTY_ACCESS(type, name, convert, extra)          \
                                                             \
void QEHistogram::set##name (const type value) {             \
   type temp;                                                \
   temp = convert;                                           \
   if (this->m##name != temp) {                              \
      this->m##name = temp;                                  \
      extra;                                                 \
      this->update ();                                       \
   }                                                         \
}                                                            \
                                                             \
type QEHistogram::get##name () const {                       \
   return this->m##name;                                     \
}

#define NO_EXTRA

PROPERTY_ACCESS (int,    BarWidth,         LIMIT (value, 1, 120),                                 this->mAutoBarGapWidths = false)
PROPERTY_ACCESS (int,    Gap,              LIMIT (value, 0, 20),                                  this->mAutoBarGapWidths= false)
PROPERTY_ACCESS (double, Minimum,          LIMIT (value, -1.0E20, this->mMaximum - MINIMUM_SPAN), this->mAutoScale = false)
PROPERTY_ACCESS (double, Maximum,          LIMIT (value, this->mMinimum + MINIMUM_SPAN, +1.0E20), this->mAutoScale = false)
PROPERTY_ACCESS (double, BaseLine,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoScale,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoBarGapWidths, value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   LogScale,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   DrawBorder,       value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, BarColour,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (Qt::Orientation,  Orientation,  value,                                           NO_EXTRA)
PROPERTY_ACCESS (int,    TestSize,         LIMIT (value, 0, MAX_CAPACITY),                        this->createTestData ())

#undef PROPERTY_ACCESS

// end
