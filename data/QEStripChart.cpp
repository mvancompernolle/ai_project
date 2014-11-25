/*  QEStripChart.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2012 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>

#include <QBrush>
#include <QCursor>
#include <QDebug>
#include <QDockWidget>
#include <QFont>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QMutex>
#include <QPen>
#include <QPushButton>
#include <QRegExp>
#include <QScrollArea>
#include <QStringList>
#include <QToolButton>
#include <QApplication>
#include <QClipboard>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <QEGraphic.h>

#include <alarm.h>

#include <QECommon.h>
#include <QCaObject.h>
#include <QELabel.h>
#include <QCaVariableNamePropertyManager.h>

#include "QEStripChart.h"
#include "QEStripChartToolBar.h"
#include "QEStripChartItem.h"
#include "QEStripChartUtilities.h"


#define DEBUG  qDebug () << "QEStripChart::" << __FUNCTION__ << ":" << __LINE__

static const QColor clWhite (0xFF, 0xFF, 0xFF, 0xFF);
static const QColor clBlack (0x00, 0x00, 0x00, 0xFF);

#define PV_DELTA_HEIGHT    18

#define PV_FRAME_HEIGHT    (6 + (NUMBER_OF_PVS / 2) * PV_DELTA_HEIGHT)
#define PV_SCROLL_HEIGHT   (PV_FRAME_HEIGHT + 6)


//==============================================================================
// Local support classes.
//==============================================================================
//
//==============================================================================
//
class QEPVNameLists : public QStringList {
public:
   explicit QEPVNameLists ();
   virtual ~QEPVNameLists ();

   void prependOrMoveToFirst (const QString & item);
   void saveConfiguration (PMElement & parentElement);
   void restoreConfiguration (PMElement & parentElement);
private:
   QMutex *mutex;
};

//------------------------------------------------------------------------------
//
QEPVNameLists::QEPVNameLists ()
{
   this->mutex = new QMutex ();
   // inititialise from container and/or environment variable?
}

//------------------------------------------------------------------------------
//
QEPVNameLists::~QEPVNameLists ()
{
   delete this->mutex;
}

//------------------------------------------------------------------------------
//
void QEPVNameLists::prependOrMoveToFirst (const QString & item)
{
   QMutexLocker locker (this->mutex);

   int posn;

   posn = this->indexOf (item, 0);
   if (posn < 0) {
      this->prepend (item);
   } else if (posn > 0) {
      // item in list - move to front.
      this->swap (0, posn);
   }  // else posn = 0 - nothing to do.

   if (this->count () > QEStripChartNames::NumberPrefefinedItems) {
      this->removeLast ();
   }
}

//------------------------------------------------------------------------------
//
void QEPVNameLists::saveConfiguration (PMElement & parentElement)
{
   PMElement predefinedElement = parentElement.addElement ("Predefined");
   int number;
   int j;

   number = this->count ();
   predefinedElement.addAttribute ("Number", number);
   for (j = 0; j < number; j++) {
      PMElement pvElement = predefinedElement.addElement ("PV");
      pvElement.addAttribute ("id", j);
      pvElement.addValue ("Name", this->value (j));
   }

}

//------------------------------------------------------------------------------
//
void QEPVNameLists::restoreConfiguration (PMElement & parentElement)
{
   PMElement predefinedElement = parentElement.getElement ("Predefined");
   int number;
   int j;
   QString pvName;
   bool status;

   if (predefinedElement.isNull ()) return;

   status = predefinedElement.getAttribute ("Number", number);
   if (status) {
      this->clear ();

      // Read in reverse order (as use insert into list with prependOrMoveToFirst).
      //
      for (j = number - 1; j >= 0; j--) {
         PMElement pvElement = predefinedElement.getElement ("PV", "id", j);

         if (pvElement.isNull ()) continue;

         status = pvElement.getValue ("Name", pvName);
         if (status) {
            this->prependOrMoveToFirst (pvName);
         }
      }
   }
}

//------------------------------------------------------------------------------
// This is a static list shared amongst all instances of the strip chart widget.
//
static QEPVNameLists predefinedPVNameList;


//==============================================================================
// QEStripChart class functions
//==============================================================================
//
void QEStripChart::createInternalWidgets ()
{
   unsigned int slot;

   // Create dialog.
   // We have one dialog per strip chart (as opposed to per pv item) as this not only saves
   // resources, but a single dialog will remember filter and other state information.
   //
   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);

   // Create tool bar frame and tool buttons.
   //
   this->toolBar = new QEStripChartToolBar (); // this will become parented by toolBarResize

   // Connect various tool bar signals to the chart.
   //
   QObject::connect (this->toolBar, SIGNAL (stateSelected  (const QEStripChartNames::StateModes)),
                     this,          SLOT   (stateSelected  (const QEStripChartNames::StateModes)));

   QObject::connect (this->toolBar, SIGNAL (videoModeSelected  (const QEStripChartNames::VideoModes)),
                     this,          SLOT   (videoModeSelected  (const QEStripChartNames::VideoModes)));

   QObject::connect (this->toolBar, SIGNAL (yScaleModeSelected  (const QEStripChartNames::YScaleModes)),
                     this,          SLOT   (yScaleModeSelected  (const QEStripChartNames::YScaleModes)));

   QObject::connect (this->toolBar, SIGNAL (yRangeSelected  (const QEStripChartNames::ChartYRanges)),
                     this,          SLOT   (yRangeSelected  (const QEStripChartNames::ChartYRanges)));

   QObject::connect (this->toolBar, SIGNAL (durationSelected  (const int)),
                     this,          SLOT   (durationSelected  (const int)));

   QObject::connect (this->toolBar, SIGNAL (playModeSelected  (const QEStripChartNames::PlayModes)),
                     this,          SLOT   (playModeSelected  (const QEStripChartNames::PlayModes)));

   QObject::connect (this->toolBar, SIGNAL (timeZoneSelected (const Qt::TimeSpec)),
                     this,          SLOT   (timeZoneSelected (const Qt::TimeSpec)));

   QObject::connect (this->toolBar, SIGNAL (readArchiveSelected  ()),
                     this,          SLOT   (readArchiveSelected  ()));


   // Create user controllable resize area
   //
   this->toolBarResize = new QEResizeableFrame (QEResizeableFrame::BottomEdge, 8, 8 + this->toolBar->designHeight, this);
   this->toolBarResize->setFixedHeight (8 + this->toolBar->designHeight);
   this->toolBarResize->setFrameShape (QFrame::Panel);
   this->toolBarResize->setGrabberToolTip ("Re size tool bar display area");
   this->toolBarResize->setWidget (this->toolBar);

   // Create PV frame and PV name labels and associated CA labels.
   //
   this->pvFrame = new QFrame ();  // this will become parented by pvScrollArea
   this->pvFrame->setFixedHeight (PV_FRAME_HEIGHT);

   this->pvGridLayout = new QGridLayout (this->pvFrame);
   this->pvGridLayout->setContentsMargins (2, 2, 2, 2);
   this->pvGridLayout->setHorizontalSpacing (8);
   this->pvGridLayout->setVerticalSpacing (2);

   // Create widgets (parented by chart) and chart item that manages these.
   //
   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem * chartItem  = new QEStripChartItem (this, slot, this->pvFrame);

      // Add to grid.
      //
      this->pvGridLayout->addWidget (chartItem, slot / 2, slot %2);
      this->items [slot] = chartItem;
   }

   // Create scrolling area and add pv frame.
   //
   this->pvScrollArea = new QScrollArea ();          // this will become parented by pvResizeFrame
   this->pvScrollArea->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->pvScrollArea->setWidgetResizable (true);    // MOST IMPORTANT
   this->pvScrollArea->setWidget (this->pvFrame);

   // Create user controllable resize area
   //
   this->pvResizeFrame = new QEResizeableFrame (QEResizeableFrame::BottomEdge, 12, PV_SCROLL_HEIGHT + 8, this);
   this->pvResizeFrame->setFixedHeight (PV_SCROLL_HEIGHT + 8);
   this->pvResizeFrame->setFrameShape (QFrame::Panel);
   this->pvResizeFrame->setGrabberToolTip ("Re size PV display area");
   this->pvResizeFrame->setWidget (this->pvScrollArea);

   // Create plotting frame and plot area.
   //
   this->plotFrame = new QFrame (this);
   this->plotFrame->setFrameShape (QFrame::Panel);

   this->plotArea = new QEGraphic (this->plotFrame);

   QObject::connect (this->plotArea, SIGNAL (mouseMove     (const QPointF&)),
                     this,           SLOT   (plotMouseMove (const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (wheelRotate   (const QPointF&, const int)),
                     this,           SLOT   (zoomInOut     (const QPointF&, const int)));

   QObject::connect (this->plotArea, SIGNAL (areaDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (scaleSelect    (const QPointF&, const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (lineDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (lineSelected   (const QPointF&, const QPointF&)));

   // Create layouts.
   //
   this->layout1 = new QVBoxLayout (this);
   this->layout1->setMargin (4);
   this->layout1->setSpacing (4);
   this->layout1->addWidget (this->toolBarResize);
   this->layout1->addWidget (this->pvResizeFrame);
   this->layout1->addWidget (this->plotFrame);

   this->layout2 = new QVBoxLayout (this->plotFrame);
   this->layout2->setMargin (4);
   this->layout2->setSpacing (4);
   this->layout2->addWidget (this->plotArea);

   // We use the default context menu but we need to filter activation.
   //
   this->setContextMenuPolicy (Qt::CustomContextMenu);

   QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this, SLOT   (chartContextMenuRequested  (const QPoint &)));

   // Clear / initialise plot.
   //
   this->chartYScale = QEStripChartNames::dynamic;
   this->yScaleMode = QEStripChartNames::linear;
   this->chartTimeMode = QEStripChartNames::tmRealTime;
   this->timeScale = 1.0;
   this->timeUnits = "secs";

   this->setNormalBackground (true);
}

//------------------------------------------------------------------------------
//
QEStripChartItem* QEStripChart::getItem (unsigned int slot)
{
   return (slot < NUMBER_OF_PVS) ? this->items [slot] : NULL;
}

//------------------------------------------------------------------------------
//
void QEStripChart::chartContextMenuRequested (const QPoint & pos)
{
   // Don't want to do context menu over plot canvas area - we use right-click
   // for other stuff.
   //
   // NOTE: This check relies on the fact that the right mouse button event handler
   // in QEGraphic is called before this slot is invoked.
   //
   if (this->plotArea->rightButtonPressed () == false) {
      QMenu* menu = this->buildContextMenu ();     // Create/build standard menu.
      QPoint golbalPos = this->mapToGlobal (pos);  // map position.
      menu->exec (golbalPos, 0);
      delete menu;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setNormalBackground (bool isNormalVideoIn)
{
   QColor background;
   QRgb gridColour;
   QPen pen;

   this->isNormalVideo = isNormalVideoIn;

   background = this->isNormalVideo ? clWhite : clBlack;
   this->plotArea->setBackgroundColour (background);

   gridColour =  this->isNormalVideo ? 0x00c0c0c0 : 0x00404040;
   pen.setColor(QColor (gridColour));
   pen.setStyle (Qt::DashLine);
   this->plotArea->setGridPen (pen);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::calcDisplayMinMax ()
{
   int slot;
   QEDisplayRanges tr;
   double min;
   double max;

   if (this->chartYScale == QEStripChartNames::manual) return;

   tr.clear ();

   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {

      QEStripChartItem * item = this->getItem (slot);
      if (item->isInUse() == true) {
         switch (this->chartYScale) {
         case QEStripChartNames::operatingRange:  tr.merge (item->getLoprHopr (true));         break;
         case QEStripChartNames::plotted:         tr.merge (item->getDisplayedMinMax (true));  break;
         case QEStripChartNames::buffered:        tr.merge (item->getBufferedMinMax (true));   break;
         case QEStripChartNames::dynamic:         tr.merge (item->getDisplayedMinMax (true));  break;
         default:       DEBUG << "Well this is unexpected"; return; break;
         }
      }
   }

   if (tr.getMinMax (min, max) == true) {
      this->yMinimum = min;
      this->yMaximum = MAX (max, min + 1.0E-3);
   } // else do not change.
}


//------------------------------------------------------------------------------
//
void QEStripChart::recalculateData ()
{
   // Last - clear flag.
   //
   this->recalcIsRequired = false;
}

//------------------------------------------------------------------------------
//
void QEStripChart::plotData ()
{
   unsigned int slot;
   double d;
   QString format;
   QString times;
   QDateTime dt;
   QString zoneTLA;

   // First release any/all allocated curves.
   //
   this->plotArea->releaseCurves ();

   d = this->getDuration ();
   if (d <= 1.0) {
      this->timeScale = 0.001;
      this->timeUnits = "mS";
   } else if (d <= 60.0) {
      this->timeScale = 1.0;
      this->timeUnits = "secs";
   } else if (d <= 3600.0) {
      this->timeScale = 60.0;
      this->timeUnits = "mins";
   } else if (d <= 86400.0) {
      this->timeScale = 3600.0;
      this->timeUnits = "hrs";
   } else {
      this->timeScale = 86400.0;
      this->timeUnits = "days";
   }

   this->plotArea->setXScale (1.0 / this->timeScale);
   this->plotArea->setXLogarithmic (false);
   this->plotArea->setYLogarithmic (this->yScaleMode == QEStripChartNames::log);

   // Update the plot for each PV.
   // Allocate curve and call curve-setSample/setData.
   //
   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      if (this->getItem (slot)->isInUse ()) {
          this->getItem (slot)->plotData ();
      }
   }

   if (this->chartYScale == QEStripChartNames::dynamic) {
      // Re-calculate chart range.
      //
      this->calcDisplayMinMax ();
   }

   this->plotArea->setYRange (this->getYMinimum (), this->getYMaximum (), QEGraphic::SelectBySize, 40, false);
   this->plotArea->setXRange (-d/this->timeScale, 0.0, QEGraphic::SelectByValue, 5, false);

   this->plotArea->replot ();

   format = "yyyy-MM-dd hh:mm:ss";
   times = " ";

   dt = this->getStartDateTime ().toTimeSpec (this->timeZoneSpec);
   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, dt);

   times.append (dt.toString (format)).append (" ").append (zoneTLA);
   times.append (" to ");

   dt = this->getEndDateTime ().toTimeSpec (this->timeZoneSpec);
   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, dt);

   times.append (dt.toString (format)).append (" ").append (zoneTLA);

   // set on tool bar
   this->toolBar->setTimeStatus (times);

   QEStripChartNames meta;   // allows access to enumeration metta data.
   QString yRangeStatus;

   yRangeStatus = QEUtilities::enumToString (meta, "ChartYRanges", this->chartYScale);
   if (this->chartYScale == QEStripChartNames::operatingRange) {
      yRangeStatus = "operating range";
   }
   yRangeStatus.append (" scale");
   this->toolBar->setYRangeStatus (yRangeStatus);

   // Last - clear flag.
   //
   this->replotIsRequired = false;
}

//------------------------------------------------------------------------------
//
void QEStripChart::scaleSelect (const QPointF& start, const QPointF& finish)
{
   QPoint distance = this->plotArea->pixelDistance (start, finish);

   // The QEGraphic validates the selection, i.e. that user has un-ambiguously
   // selected x (time) scaling or y scaling. Need only figure out which one.
   //
   if (ABS (distance.y ()) >=  ABS (distance.x ())) {
      // Makeing a Y scale adjustment.
      //
      this->setYRange (finish.y (), start.y ());
      this->pushState ();

   } else  {
      // Makeing a time scale adjustment.
      //
      double dt;
      int duration;
      QDateTime et;
      QDateTime now;

      dt = finish.x () - start.x ();
      duration = MAX (1, int (dt));

      et = this->getEndDateTime ().addSecs (finish.x ());

      now = QDateTime::currentDateTime ();
      if (et >= now) {
         // constrain
         et = now;
      } else {
         this->chartTimeMode = QEStripChartNames::tmHistorical;
      }

      this->setDuration (duration);
      this->setEndDateTime (et);
      this->pushState ();

   }
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::lineSelected (const QPointF&, const QPointF&)
{
   // no action per se - just request a replot (without the line).
   //
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setReadOut (const QString & text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
void QEStripChart::plotMouseMove  (const QPointF& posn)
{
   const QPointF real = posn;
   qint64 mSec;
   QDateTime t;
   QString zoneTLA;
   QString format;
   QString mouseReadOut;
   QString f;
   QPointF slope;

   t = this->getEndDateTime ().toTimeSpec (this->timeZoneSpec);

   // Convert cursor x to absolute cursor time.
   // x is the time (in seconds) relative to the chart end time.
   //
   mSec = qint64(1000.0 * real.x());
   t = t.addMSecs (mSec);

   // Keep only most significant digit of the milli-seconds,
   // i.e. tenths of a second.
   //
   format = "yyyy-MM-dd hh:mm:ss.zzz";
   mouseReadOut = t.toString (format).left (format.length() - 2);

   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, t);
   mouseReadOut.append (" ").append (zoneTLA);

   f.sprintf (" %10.2f ", real.x () /this->timeScale);
   mouseReadOut.append (f);
   mouseReadOut.append (this->timeUnits);

   f.sprintf ("  %+.10g", real.y ());
   mouseReadOut.append (f);

   if (this->plotArea->getSlopeIsDefined (slope)) {
      const double dt = slope.x ();
      const double dy = slope.y ();

      f.sprintf ("    dt: %.1f s ", dt);
      mouseReadOut.append (f);

      f.sprintf ("  dy: %+.6g", dy);
      mouseReadOut.append (f);

      // Calculate slope, but avoid the divide by 0.
      //
      mouseReadOut.append ("  dy/dt: ");
      if (dt != 0.0) {
         f.sprintf ("%+.6g", dy/dt);
      } else {
         if (dy != 0.0) {
            f.sprintf ("%sinf", (dy >= 0.0) ? "+" : "-");
         } else {
            f.sprintf ("n/a");
         }
      }
      mouseReadOut.append (f);
   }

   this->setReadOut (mouseReadOut);
}

//------------------------------------------------------------------------------
//
void QEStripChart::captureState (QEStripChartState& chartState)
{
   // Capture current state.
   //
   chartState.isNormalVideo = this->isNormalVideo;
   chartState.yScaleMode = this->yScaleMode;
   chartState.chartYScale = this->chartYScale;
   chartState.yMinimum = this->getYMinimum ();
   chartState.yMaximum = this->getYMaximum ();
   chartState.chartTimeMode = this->chartTimeMode;
   chartState.duration = this->getDuration ();
   chartState.timeZoneSpec = this->timeZoneSpec;
   chartState.endDateTime = this->getEndDateTime ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::applyState (const QEStripChartState& chartState)
{
   this->setNormalBackground (chartState.isNormalVideo);
   this->yScaleMode = chartState.yScaleMode;
   this->chartYScale = chartState.chartYScale;
   this->setYRange (chartState.yMinimum, chartState.yMaximum);
   this->chartTimeMode =  chartState.chartTimeMode;
   this->setEndDateTime (chartState.endDateTime);
   this->setDuration (chartState.duration);
   this->timeZoneSpec = chartState.timeZoneSpec;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::pushState ()
{
   QEStripChartState chartState;

   // Capture current state.
   //
   this->captureState (chartState);
   this->chartStateList.push (chartState);

   // Enable/disble buttons according to availability.
   //
   this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
   this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::prevState ()
{
   QEStripChartState chartState;

   if (this->chartStateList.prev (chartState)) {
      this->applyState (chartState);
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::nextState ()
{
   QEStripChartState chartState;

   if (this->chartStateList.next (chartState)) {
      this->applyState (chartState);
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
// Constructor
//
QEStripChart::QEStripChart (QWidget * parent) : QEFrame (parent)
{
   // Configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   this->setMinimumSize (1080, 400);   // keep this and sizeHint consistant

   // Construct internal widgets for this chart.
   //
   this->createInternalWidgets ();

   this->timeZoneSpec = Qt::LocalTime;
   this->duration = 600;     // ten minutes.
   this->timeScale = 60.0;   // minutes
   this->timeUnits = "mins";

   // We always use UTC (EPICS) time within the strip chart.
   // Set directly here as using setEndTime has side effects.
   //
   this->endDateTime = QDateTime::currentDateTime ().toUTC ();

   this->yMinimum = 0.0;
   this->yMaximum = 100.0;

   this->plotArea->setXScale (1.0 / this->timeScale);
   this->plotArea->setXRange (-this->duration / this->timeScale, 0.0, QEGraphic::SelectByValue, 5, true);
   this->plotArea->setYRange (this->yMinimum, this->yMaximum, QEGraphic::SelectBySize, 40, true);

   this->variableNameSubstitutions = "";
   this->setNumVariables (0);

   // Construct dialogs.
   //
   this->timeDialog = new QEStripChartTimeDialog (this);
   this->yRangeDialog = new QEStripChartRangeDialog (this);

   // Refresh the stip chart at 1Hz.
   //
   this->tickTimer = new QTimer (this);
   this->tickTimerCount = 0;
   this->replotIsRequired = true; // ensure process on first tick.
   this->recalcIsRequired = false;

   connect (this->tickTimer, SIGNAL (timeout ()), this, SLOT (tickTimeout ()));
   this->tickTimer->start (50);  // mSec = 0.05 s

   // Enable drag drop onto this widget.
   //
   this->evaluateAllowDrop ();

   this->chartStateList.clear ();
   this->pushState ();  // baseline state - there is always at least one.
}

//------------------------------------------------------------------------------
//
QEStripChart::~QEStripChart ()
{
   this->tickTimer->stop ();
}

//------------------------------------------------------------------------------
//
QSize QEStripChart::sizeHint () const
{
   return QSize (1080, 400);
}

//------------------------------------------------------------------------------
//
void QEStripChart::setVariableNameProperty (unsigned int slot, QString pvName)
{
   if (slot < NUMBER_OF_PVS) {
      QEStripChartItem * item = this->getItem (slot);
      item->pvNameProperyManager.setVariableNameProperty (pvName);
   } else {
      DEBUG << "slot out of range " << slot;
   }
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getVariableNameProperty (unsigned int slot)
{
   if (slot < NUMBER_OF_PVS) {
      QEStripChartItem * item = this->getItem (slot);
      return item->pvNameProperyManager.getVariableNameProperty ();
   } else {
      DEBUG << "slot out of range " << slot;
      return "";
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setVariableNameSubstitutionsProperty (QString variableNameSubstitutionsIn)
{
   int j;

   // Save local copy - just for getVariableNameSubstitutionsProperty.
   //
   this->variableNameSubstitutions = variableNameSubstitutionsIn;

   // The same subtitutions apply to all PVs.
   //
   for (j = 0; j < NUMBER_OF_PVS; j++ ) {
      QEStripChartItem * item = this->getItem (j);
      item->pvNameProperyManager.setSubstitutionsProperty (variableNameSubstitutionsIn);
   }
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getVariableNameSubstitutionsProperty ()
{
   return this->variableNameSubstitutions;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setColourProperty (unsigned int slot, QColor colour)
{
   if (slot < NUMBER_OF_PVS) {
      QEStripChartItem * item = this->getItem (slot);
      item->setColour (colour);
   } else {
      DEBUG << "slot out of range " << slot;
   }
}

//------------------------------------------------------------------------------
//
QColor QEStripChart::getColourProperty (unsigned int slot)
{
   if (slot < NUMBER_OF_PVS) {
      QEStripChartItem * item = this->getItem (slot);
      return item->getColour ();
   } else {
      DEBUG << "slot out of range " << slot;
      return QColor (0x00, 0x00, 0x00, 0xFF);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::addPvName (const QString& pvName)
{
   unsigned int slot;

   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem * item = this->getItem (slot);
      if (item->isInUse() == false) {
         // Found an empty slot.
         //
         item->setPvName (pvName, "");
         break;
      }
   }

   // Determine if we are now full.
   //
   this->evaluateAllowDrop ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::tickTimeout ()
{
   this->tickTimerCount = (this->tickTimerCount + 1) % 20;

   if (this->recalcIsRequired) {
      this->recalculateData ();
   }

   if ((this->tickTimerCount % 20) == 0) {
      // 20th update, i.e. 1 second has passed - must replot.
      this->replotIsRequired = true;
   }

   if (this->replotIsRequired) {
      if (this->chartTimeMode == QEStripChartNames::tmRealTime) {
         this->setEndDateTime (QDateTime::currentDateTime ());
      }
      this->plotData ();  // clears replotIsRequired
   }
}


//=============================================================================
// Handle toolbar signals
//
void QEStripChart::stateSelected (const QEStripChartNames::StateModes mode)
{
   if (mode == QEStripChartNames::previous) {
      this->prevState ();
   } else {
      this->nextState ();
   }

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::videoModeSelected (const QEStripChartNames::VideoModes mode)
{
   this->setNormalBackground (mode == QEStripChartNames::normal);
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::yScaleModeSelected (const QEStripChartNames::YScaleModes mode)
{
   this->yScaleMode = mode;
   this->pushState ();
}


//------------------------------------------------------------------------------
//
void QEStripChart::yRangeSelected (const QEStripChartNames::ChartYRanges scale)
{
   int n;
   unsigned int slot;

   switch (scale) {
      case QEStripChartNames::manual:
         this->yRangeDialog->setRange (this->getYMinimum (), this->getYMaximum ());
         n = this->yRangeDialog->exec (this);
         if (n == 1) {
            this->chartYScale = scale;
            // User has selected okay.
            //
            this->setYRange (this->yRangeDialog->getMinimum (),
                             this->yRangeDialog->getMaximum ());
         }
         this->pushState ();
         break;

      case QEStripChartNames::operatingRange:
      case QEStripChartNames::plotted:
      case QEStripChartNames::buffered:
      case QEStripChartNames::dynamic:
         this->chartYScale = scale;
         this->calcDisplayMinMax ();
         this->pushState ();
         break;

      case QEStripChartNames::normalised:
         this->setYRange (0.0, 100.0);
         this->chartYScale = scale;

         for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
            QEStripChartItem * item = this->getItem (slot);
            if (item->isInUse ()) {
               item->normalise ();
            }
         }
         this->pushState ();
         break;

      default:
         DEBUG << "Well this is unexpected:" << (int) scale;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::durationSelected (const int seconds)
{
   this->setDuration (seconds);
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::timeZoneSelected (const Qt::TimeSpec timeZoneSpecIn)
{
   this->timeZoneSpec = timeZoneSpecIn;
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::playModeSelected (const QEStripChartNames::PlayModes mode)
{
   int n;
   int d;

   switch (mode) {

      case QEStripChartNames::play:
         this->chartTimeMode = QEStripChartNames::tmRealTime;
         // Note: using setEndTime causes a replot.
         this->setEndDateTime (QDateTime::currentDateTime ());
         this->pushState ();
         break;

      case QEStripChartNames::pause:
         this->chartTimeMode = QEStripChartNames::tmPaused;
         this->pushState ();
         break;

      case QEStripChartNames::forward:
         this->chartTimeMode = QEStripChartNames::tmHistorical;
         this->setEndDateTime (this->getEndDateTime ().addSecs (+this->duration));
         this->pushState ();
         break;

      case QEStripChartNames::backward:
         this->chartTimeMode = QEStripChartNames::tmHistorical;
         this->setEndDateTime (this->getEndDateTime ().addSecs (-this->duration));
         this->pushState ();
         break;

      case QEStripChartNames::selectTimes:
         this->timeDialog->setMaximumDateTime (QDateTime::currentDateTime ().toTimeSpec (this->timeZoneSpec));
         this->timeDialog->setStartDateTime (this->getStartDateTime().toTimeSpec (this->timeZoneSpec));
         this->timeDialog->setEndDateTime (this->getEndDateTime().toTimeSpec (this->timeZoneSpec));
         n = this->timeDialog->exec (this);
         if (n == 1) {
            // User has selected okay.
            //
            this->chartTimeMode = QEStripChartNames::tmHistorical;
            this->setEndDateTime (this->timeDialog->getEndDateTime ());

            // We use the possibly limited chart end time in order to calculate the
            // duration.
            //
            d = this->timeDialog->getStartDateTime ().secsTo (this->getEndDateTime());
            this->setDuration (d);
            this->pushState ();
         }
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::zoomInOut      (const QPointF& about, const int zoomAmount)
{
   if (zoomAmount) {
      // We really only need the sign of the zoomAmount.
      //
      const double factor = (zoomAmount >= 0) ? 0.95 : (1.0 / 0.95);

      double newMin;
      double newMax;

      if (this->yScaleMode == QEStripChartNames::log) {
         const double logAboutY = LOG10 (about.y ());

         newMin = EXP10 (logAboutY + (LOG10 (this->yMinimum) - logAboutY) * factor);
         newMax = EXP10 (logAboutY + (LOG10 (this->yMaximum) - logAboutY) * factor);
      } else {
         newMin = about.y () + (this->yMinimum - about.y ()) * factor;
         newMax = about.y () + (this->yMaximum - about.y ()) * factor;
      }

      this->setYRange (newMin, newMax);
      this->pushState ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::readArchiveSelected ()
{
   unsigned int slot;

   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item->isInUse ()) {
         item->readArchive ();
      }
   }
}
//
// end of tool bar handlers ====================================================


//------------------------------------------------------------------------------
//
void QEStripChart::addToPredefinedList (const QString & pvName)
{
   predefinedPVNameList.prependOrMoveToFirst (pvName);
}

//------------------------------------------------------------------------------
//
QStringList QEStripChart::getPredefinedPVNameList ()
{
   return QStringList (predefinedPVNameList);
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getPredefinedItem (int i)
{
   return predefinedPVNameList.value (i, "");
}

//------------------------------------------------------------------------------
// Start/end time
//
QDateTime QEStripChart::getStartDateTime ()
{
   return this->getEndDateTime().addSecs (-this->duration);
}

//------------------------------------------------------------------------------
//
QDateTime QEStripChart::getEndDateTime ()
{
   return this->endDateTime;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setEndDateTime (QDateTime endDateTimeIn)
{
   QDateTime useUTC = endDateTimeIn.toUTC ();
   QDateTime nowUTC = QDateTime::currentDateTime ().toUTC ();

   // No peeking into the future.
   //
   if (useUTC > nowUTC) {
      useUTC = nowUTC;
   }

   if (this->endDateTime != useUTC) {
      this->endDateTime = useUTC;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
int QEStripChart::getDuration ()
{
   return this->duration;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setDuration (int durationIn)
{
   // A duration of less than 1 second is not allowed.
   //
   if (durationIn < 1) {
      durationIn = 1;
   }

   if (this->duration != durationIn) {
      this->duration = durationIn;
      this->replotIsRequired = true;
   }
}

//----------------------------------------------------------------------------
//
double QEStripChart::getYMinimum ()
{
   return this->yMinimum;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYMinimum (const double yMinimumIn)
{
   this->yMinimum = yMinimumIn;
   this->yMaximum = MAX (this->yMaximum, this->yMinimum + 1.0E-3);
   this->chartYScale = QEStripChartNames::manual;
   this->replotIsRequired = true;
}

//----------------------------------------------------------------------------
//
double QEStripChart::getYMaximum ()
{
   return this->yMaximum;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYMaximum (const double yMaximumIn)
{
   this->yMaximum = yMaximumIn;
   this->yMinimum = MIN (this->yMinimum, this->yMaximum - 1.0E-3);
   this->chartYScale = QEStripChartNames::manual;
   this->replotIsRequired = true;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYRange (const double yMinimumIn, const double yMaximumIn)
{
    this->yMinimum = yMinimumIn;
    this->yMaximum = MAX (yMaximumIn, this->yMinimum + 1.0E-3);
    this->chartYScale = QEStripChartNames::manual;
    this->replotIsRequired = true;
}

//----------------------------------------------------------------------------
//
QString QEStripChart::copyVariable ()
{
   QString result;

   // Create space delimited set of PV names.
   //
   result = "";
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);

      if ((item) && (item->isInUse() == true)) {
         if (!result.isEmpty()) {
            result.append (" ");
         };
         result.append (item->getPvName ());
      }
   }
   return result;
}

//----------------------------------------------------------------------------
//
QVariant QEStripChart::copyData ()
{
   return  QVariant ();  // place holder.
}

//----------------------------------------------------------------------------
//
void QEStripChart::paste (QVariant s)
{
   QStringList pvNameList;

   pvNameList = QEUtilities::variantToStringList (s);
   for (int j = 0; j < pvNameList.count (); j++) {
      this->addPvName (pvNameList.value (j));
   }
}

//----------------------------------------------------------------------------
// Determine if user allowed to drop new PVs into this widget.
//
void QEStripChart::evaluateAllowDrop ()
{
   unsigned int slot;
   bool allowDrop;

   // Hypoyhesize that the strip chart is full.
   //
   allowDrop = false;
   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);

      if ((item) && (item->isInUse () == false)) {
         // Found an empty slot.
         //
         allowDrop = true;
         break;
      }
   }

   this->setAllowDrop (allowDrop);
}

//------------------------------------------------------------------------------
//
void QEStripChart::setup ()
{
   DEBUG << "unexpected call";
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEStripChart::createQcaItem (unsigned int variableIndex)
{
   DEBUG << "unexpected call, variableIndex = " << variableIndex;
   return NULL;
}

//------------------------------------------------------------------------------
//
void QEStripChart::establishConnection (unsigned int /* variableIndex */ )
{
   // A framework feature is that there is always at least one variable.
   // DEBUG << "unexpected call, variableIndex = " << variableIndex;
}

//------------------------------------------------------------------------------
//
void QEStripChart::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->persistantName ("QEStripChart");

   // Do common stuff first.
   // How can we avoid doing this mutiple times??
   //
   PMElement commonElement = pm->addNamedConfiguration ("QEStripChart_Common");
   predefinedPVNameList.saveConfiguration (commonElement);

   // Now do form instance specific stuff.
   //
   PMElement formElement = pm->addNamedConfiguration (formName);

   // Capture current state.
   //
   QEStripChartState chartState;
   this->captureState (chartState);
   chartState.saveConfiguration (formElement);

   // Save each active PV.
   //
   PMElement pvListElement = formElement.addElement ("PV_List");
   unsigned int slot;

   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem * item = this->getItem (slot);
      if (item) {
         item->saveConfiguration (pvListElement);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::restoreConfiguration (PersistanceManager * pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->persistantName ("QEStripChart");

   // Do common stuff first.
   // How can we avoid doing this mutiple times??
   //
   PMElement commonElement = pm->getNamedConfiguration ("QEStripChart_Common");
   predefinedPVNameList.restoreConfiguration (commonElement);

   // Now do form instance specific stuff.
   //
   PMElement formElement = pm->getNamedConfiguration (formName);

   // Restore chart state.
   //
   QEStripChartState chartState;
   chartState.restoreConfiguration (formElement);
   this->applyState (chartState);

   // Restore each PV.
   //
   PMElement pvListElement = formElement.getElement ("PV_List");
   unsigned int slot;

   for (slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem * item = this->getItem (slot);
      if (item) {
         item->restoreConfiguration (pvListElement);
      }
   }
}

// end
