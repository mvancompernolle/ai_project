/*  QEStripChartToolBar.cpp
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
 *  but WITHOUT ANY WARRANTY { } without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QString>
#include <QVariant>
#include <QDateTime>

#include <QECommon.h>
#include "QEStripChartUtilities.h"
#include "QEStripChartToolBar.h"


#define NUMBER_OF_BUTTONS  21
#define ICW                26         // icon width

// Special slots NUMBERS  - must be consistent with below
//
#define PREV_SLOT          0
#define NEXT_SLOT          1
#define YSCALE_SLOT        6
#define TSCALE_SLOT        12

// Structure used in buttonSpecs for definining strip chart tool bar.
// Note, a similar structure is used in QEPlotter. If they are the same name a
// strange problem occurs when built with GCC 4.5.1 where the QString destructor is
// called inappropriately on exit causing a crash.
//
struct QEStripChartPushButtonSpecifications {
   int gap;
   int width;
   bool isIcon;  // when false is caption
   const QString captionOrIcon;
   const QString toolTip;
   const char * member;
};

static const QString localZone = QEUtilities::getTimeZoneTLA (Qt::LocalTime, QDateTime::currentDateTime ());

static const struct QEStripChartPushButtonSpecifications buttonSpecs [NUMBER_OF_BUTTONS] = {
   { 0,   ICW, true,  QString ("go_back.png"),           QString ("Previous state"),               SLOT (prevStateClicked (bool))        },
   { 0,   ICW, true,  QString ("go_fwd.png"),            QString ("Next state"),                   SLOT (nextStateClicked (bool))        },

   { 4,   ICW, true,  QString ("normal_video.png"),      QString ("White background"),             SLOT (normalVideoClicked (bool))      },
   { 0,   ICW, true,  QString ("reverse_video.png"),     QString ("Black background"),             SLOT (reverseVideoClicked (bool))     },

   { 4,   ICW, true,  QString ("linear_scale.png"),      QString ("Linear scale"),                 SLOT (linearScaleClicked (bool))      },
   { 0,   ICW, true,  QString ("log_scale.png"),         QString ("Log Scale"),                    SLOT (logScaleClicked (bool))         },

   { 4,   ICW, false, QString ("M"),                     QString ("Manual Scale"),                 SLOT (manualYScaleClicked (bool))     },
   { 0,   ICW, false, QString ("A"),                     QString ("HOPR/LOPR Scale"),              SLOT (automaticYScaleClicked (bool))  },
   { 0,   ICW, false, QString ("P"),                     QString ("Plotted Data Scale"),           SLOT (plottedYScaleClicked (bool))    },
   { 0,   ICW, false, QString ("B"),                     QString ("Buffer Data Scale"),            SLOT (bufferedYScaleClicked (bool))   },
   { 0,   ICW, false, QString ("D"),                     QString ("Dynamic Scale"),                SLOT (dynamicYScaleClicked (bool))    },
   { 0,   ICW, false, QString ("N"),                     QString ("Normalised Scale"),             SLOT (normalisedYScaleClicked (bool)) },

   { 4,   96,  false, QString ("Duration"),              QString ("Select chart duration"),        NULL                                  },

   { 4,   40,  false, localZone,                         QString ("Use local time"),               SLOT (localTimeClicked (bool))        },
   { 0,   40,  false, QString ("UTC"),                   QString ("Use UTC (GMT) time"),           SLOT (utcTimeClicked (bool))          },

   { 4,   ICW, true,  QString ("archive.png"),           QString ("Extract data from archive(s)"), SLOT (readArchiveClicked (bool))      },
   { 0,   ICW, true,  QString ("select_date_times.png"), QString ("Set chart start/end time"),     SLOT (selectTimeClicked (bool))       },
   { 0,   ICW, true,  QString ("play.png"),              QString ("Play - Real time"),             SLOT (playClicked (bool))             },
   { 0,   ICW, true,  QString ("pause.png"),             QString ("Pause"),                        SLOT (pauseClicked (bool))            },
   { 0,   ICW, true,  QString ("page_backward.png"),     QString ("Back one page"),                SLOT (backwardClicked (bool))         },
   { 0,   ICW, true,  QString ("page_forward.png"),      QString ("Forward one page"),             SLOT (forwardClicked (bool))          }
};



//==============================================================================
//
class QEStripChartToolBar::OwnWidgets : public QObject {
public:
   OwnWidgets (QEStripChartToolBar *parent);
   ~OwnWidgets ();

   QPushButton *pushButtons [NUMBER_OF_BUTTONS];
   QLabel *yScaleStatus;
   QLabel *timeStatus;

private:
   QMenu *m2;
   QMenu *m2s;
   QMenu *m2m;
   QMenu *m2h;
   QMenu *m2d;
   QMenu *m2w;
};


//------------------------------------------------------------------------------
//
QEStripChartToolBar::OwnWidgets::OwnWidgets (QEStripChartToolBar *parent) : QObject (parent)
{
   static const int seconds_per_minute = 60;
   static const int seconds_per_hour = 60 * seconds_per_minute;
   static const int seconds_per_day = 24 * seconds_per_hour;
   static const int seconds_per_week = 7 * seconds_per_day;

   int left;
   int j;
   QLabel *status;
   QPushButton *button;
   QString iconPathName;
   int gap;

   // Create toobar buttons
   // TODO: Try QToolBar - it may auto layout.
   //
   left = 4;
   for (j = 0 ; j < NUMBER_OF_BUTTONS; j++) {

      button = new QPushButton (parent);

      // Set up icon or caption text.
      //
      if (buttonSpecs[j].isIcon) {
         iconPathName = ":/qe/stripchart/";
         iconPathName.append (buttonSpecs[j].captionOrIcon);
         button->setIcon (QIcon (iconPathName));
      } else {
         button->setText (buttonSpecs[j].captionOrIcon);
      }

      button->setToolTip(buttonSpecs[j].toolTip);
      gap = buttonSpecs[j].gap;
      button->setGeometry (left + gap, 2, buttonSpecs[j].width, 26);
      left += gap + buttonSpecs[j].width + 2;
      if (buttonSpecs[j].member != NULL) {
         QObject::connect (button, SIGNAL (clicked (bool)),
                           parent, buttonSpecs[j].member);
      }
      this->pushButtons [j] = button;
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   this->m2 = new QMenu (parent);

   this->m2s = new QMenu ("seconds", this->m2);
   this->m2m = new QMenu ("minutes", this->m2);
   this->m2h = new QMenu ("hours", this->m2);
   this->m2d = new QMenu ("days", this->m2);
   this->m2w = new QMenu ("weeks", this->m2);

   this->m2->addMenu (this->m2s);
   this->m2->addMenu (this->m2m);
   this->m2->addMenu (this->m2h);
   this->m2->addMenu (this->m2d);
   this->m2->addMenu (this->m2w);

   this->m2s->addAction ("1 sec   ")->setData (QVariant (1));
   this->m2s->addAction ("2 secs  ")->setData (QVariant (2));
   this->m2s->addAction ("5 secs  ")->setData (QVariant (5));
   this->m2s->addAction ("10 secs ")->setData (QVariant (10));
   this->m2s->addAction ("20 secs ")->setData (QVariant (20));
   this->m2s->addAction ("30 secs ")->setData (QVariant (30));

   this->m2m->addAction ("1 min   ")->setData (QVariant (1 * seconds_per_minute));
   this->m2m->addAction ("2 mins  ")->setData (QVariant (2 * seconds_per_minute));
   this->m2m->addAction ("5 mins  ")->setData (QVariant (5 * seconds_per_minute));
   this->m2m->addAction ("10 mins ")->setData (QVariant (10 * seconds_per_minute));
   this->m2m->addAction ("20 mins ")->setData (QVariant (20 * seconds_per_minute));
   this->m2m->addAction ("30 mins ")->setData (QVariant (30 * seconds_per_minute));

   this->m2h->addAction ("1 hour   ")->setData (QVariant (1 * seconds_per_hour));
   this->m2h->addAction ("2 hours  ")->setData (QVariant (2 * seconds_per_hour));
   this->m2h->addAction ("5 hours  ")->setData (QVariant (5 * seconds_per_hour));
   this->m2h->addAction ("10 hours ")->setData (QVariant (10 * seconds_per_hour));
   this->m2h->addAction ("20 hours ")->setData (QVariant (20 * seconds_per_hour));

   this->m2d->addAction ("1 day    ")->setData (QVariant (1 * seconds_per_day));
   this->m2d->addAction ("2 days   ")->setData (QVariant (2 * seconds_per_day));
   this->m2d->addAction ("5 days   ")->setData (QVariant (5 * seconds_per_day));
   this->m2d->addAction ("10 days  ")->setData (QVariant (10 * seconds_per_day));
   this->m2d->addAction ("20 days  ")->setData (QVariant (20 * seconds_per_day));

   this->m2w->addAction ("1 week   ")->setData (QVariant (1 * seconds_per_week));
   this->m2w->addAction ("2 weeks  ")->setData (QVariant (2 * seconds_per_week));
   this->m2w->addAction ("5 weeks  ")->setData (QVariant (5 * seconds_per_week));
   this->m2w->addAction ("10 weeks ")->setData (QVariant (10 * seconds_per_week));
   this->m2w->addAction ("20 weeks ")->setData (QVariant (20 * seconds_per_week));

   // Connextion seems to apply to all the sub-menus as well
   //
   QObject::connect (this->m2,  SIGNAL (triggered       (QAction *)),
                     parent,    SLOT   (durationClicked (QAction *)));

   button = this->pushButtons [TSCALE_SLOT];
   button->setMenu (this->m2);

   // Set up status labels.
   //
   this->timeStatus = status = new QLabel (parent);
   left = this->pushButtons [TSCALE_SLOT]->geometry().x ();
   status->setGeometry (left, 28, 368, 16);

   QFont font = status->font ();
   font.setFamily ("Monospace");
   font.setPointSize (9);
   status->setFont (font);
   // status->setStyleSheet ("QWidget { background-color: #ffffe0; }");

   this->yScaleStatus = status = new QLabel ("Dynamic", parent);
   left = this->pushButtons [YSCALE_SLOT]->geometry().x ();
   status->setGeometry (left, 28, 160, 16);
   status->setAlignment (Qt::AlignHCenter);
   status->setFont (font);
   // status->setStyleSheet ("QWidget { background-color: #ffffe0; }");

}

//------------------------------------------------------------------------------
//
QEStripChartToolBar::OwnWidgets::~OwnWidgets ()
{
  // no special action - place holder
}


//==============================================================================
//
QEStripChartToolBar::QEStripChartToolBar (QWidget *parent) : QFrame (parent)
{
   this->setFixedHeight (32);
   this->ownWidgets = new OwnWidgets (this);
}

//------------------------------------------------------------------------------
//
QEStripChartToolBar::~QEStripChartToolBar ()
{
   // no special action - place holder
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setYRangeStatus (const QString & status)
{
   this->ownWidgets->yScaleStatus->setText (status);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setTimeStatus (const QString & timeStatusIn)
{
   this->ownWidgets->timeStatus->setText (timeStatusIn);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setStateSelectionEnabled (const QEStripChartNames::StateModes mode, const bool enabled)
{
   switch (mode) {
      case QEStripChartNames::previous:
         this->ownWidgets->pushButtons [PREV_SLOT]->setEnabled (enabled);
         break;

      case QEStripChartNames::next:
         this->ownWidgets->pushButtons [NEXT_SLOT]->setEnabled (enabled);
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::resizeEvent (QResizeEvent *)
{
    // place holder
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::durationClicked (QAction *action)
{
   int d;
   bool okay;

   d = action->data().toInt (&okay);
   if (okay) {
      emit this->durationSelected (d);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::prevStateClicked (bool)
{
   emit this->stateSelected (QEStripChartNames::previous);
}

void QEStripChartToolBar::nextStateClicked (bool)
{
   emit this->stateSelected (QEStripChartNames::next);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::normalVideoClicked (bool)
{
   emit this->videoModeSelected (QEStripChartNames::normal);
}

void QEStripChartToolBar::reverseVideoClicked (bool)
{
   emit this->videoModeSelected (QEStripChartNames::reverse);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::linearScaleClicked (bool)
{
   this->yScaleModeSelected (QEStripChartNames::linear);
}

void QEStripChartToolBar::logScaleClicked (bool)
{
   this->yScaleModeSelected (QEStripChartNames::log);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::manualYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::manual);
}

void QEStripChartToolBar::automaticYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::operatingRange);
}

void QEStripChartToolBar::plottedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::plotted);
}

void QEStripChartToolBar::bufferedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::buffered);
}

void QEStripChartToolBar::dynamicYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::dynamic);
}

void QEStripChartToolBar::normalisedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::normalised);
}


//------------------------------------------------------------------------------
//
void QEStripChartToolBar::playClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::play);
}

void QEStripChartToolBar::pauseClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::pause);
}

void QEStripChartToolBar::forwardClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::forward);
}

void QEStripChartToolBar::backwardClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::backward);
}

void QEStripChartToolBar::selectTimeClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::selectTimes);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::readArchiveClicked (bool)
{
    emit this->readArchiveSelected ();
}

//------------------------------------------------------------------------------
//
void  QEStripChartToolBar::localTimeClicked (bool)
{
   emit this->timeZoneSelected (Qt::LocalTime);
}

void  QEStripChartToolBar::utcTimeClicked (bool)
{
   emit this->timeZoneSelected (Qt::UTC);
}

// end

