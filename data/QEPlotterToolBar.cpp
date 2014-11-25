/*  QEPlotterToolBar.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QIcon>
#include <QString>
#include <QVariant>

#include <QECommon.h>
#include "QEPlotterToolBar.h"
#include <QEPlotterMenu.h>

// Structure used in buttonSpecs for definining plotter tool bar.
// Note, a similar structure is used in QEStripChart. If they are the same name a
// strange problem occurs when built with GCC 4.5.1 where the QString destructor is
// called inappropriately on exit causing a crash.
//
struct QEPlotterPushButtonSpecifications {
   int gap;
   int width;
   bool isIcon;                         // when false is caption
   const QString captionOrIcon;         // caption text or iocn filename - not full path
   QEPlotterNames::MenuActions action;  // associated action
   const QString toolTip;
};

#define NO_SLOT            0
#define GAP                8          // group gap
#define ICW                26         // icon width

static const struct QEPlotterPushButtonSpecifications buttonSpecs [] = {
   { 0,   ICW, true,  QString ("go_back.png"),       QEPlotterNames::PLOTTER_PREV,               QString ("Previous state")     },
   { 0,   ICW, true,  QString ("go_fwd.png"),        QEPlotterNames::PLOTTER_NEXT,               QString ("Next state")         },

   { GAP, ICW, true,  QString ("normal_video.png"),  QEPlotterNames::PLOTTER_NORMAL_VIDEO,       QString ("White background")   },
   { 0,   ICW, true,  QString ("reverse_video.png"), QEPlotterNames::PLOTTER_REVERSE_VIDEO,      QString ("Black background")   },

   { GAP, ICW, true,  QString ("linear_scale.png"),  QEPlotterNames::PLOTTER_LINEAR_Y_SCALE,     QString ("Linear Y Scale")     },
   { 0,   ICW, true,  QString ("log_scale.png"),     QEPlotterNames::PLOTTER_LOG_Y_SCALE,        QString ("Log Y Scale")        },
   { 0,   ICW, false, QString ("My"),                QEPlotterNames::PLOTTER_MANUAL_Y_RANGE,     QString ("Manual Y Scale")     },
   { 0,   ICW, false, QString ("Ay"),                QEPlotterNames::PLOTTER_CURRENT_Y_RANGE,    QString ("Y Data Range Scale") },
   { 0,   ICW, false, QString ("Dy"),                QEPlotterNames::PLOTTER_DYNAMIC_Y_RANGE,    QString ("Dynamic Y Scale")    },
   { 0,   ICW, false, QString ("N"),                 QEPlotterNames::PLOTTER_NORAMLISED_Y_RANGE, QString ("Noramalised Scale")  },
   { 0,   ICW, false, QString ("F"),                 QEPlotterNames::PLOTTER_FRACTIONAL_Y_RANGE, QString ("Fractional Scale")   },

   { GAP, ICW, true,  QString ("linear_scale.png"),  QEPlotterNames::PLOTTER_LINEAR_X_SCALE,     QString ("Linear X Scale")     },
   { 0,   ICW, true,  QString ("log_scale.png"),     QEPlotterNames::PLOTTER_LOG_X_SCALE,        QString ("Log X Scale")        },
   { 0,   ICW, false, QString ("Mx"),                QEPlotterNames::PLOTTER_MANUAL_X_RANGE,     QString ("Manual X Scale")     },
   { 0,   ICW, false, QString ("Ax"),                QEPlotterNames::PLOTTER_CURRENT_X_RANGE,    QString ("X Data Range Scale") },
   { 0,   ICW, false, QString ("Dx"),                QEPlotterNames::PLOTTER_DYNAMIC_X_RANGE,    QString ("Dynamic X Scale")    },

   { GAP, ICW, true,  QString ("play.png"),          QEPlotterNames::PLOTTER_PLAY,               QString ("Play - Real time")   },
   { 0,   ICW, true,  QString ("pause.png"),         QEPlotterNames::PLOTTER_PAUSE,              QString ("Pause"),             }
};


//==============================================================================
//
QEPlotterToolBar::QEPlotterToolBar (QWidget *parent) : QFrame (parent)
{
   int left;
   int j;
   QPushButton *button;
   QString iconPathName;
   int gap;

   this->setFixedHeight (this->designHeight);

   // Clear array.
   //
   this->buttonToActionMap.clear ();
   this->actionToButtonMap.clear ();

   // Create toobar buttons
   // TODO: Try QToolBar - it may auto layout.
   //
   left = 4;
   for (j = 0; j < ARRAY_LENGTH (buttonSpecs); j++) {

      button = new QPushButton (this);

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

      QObject::connect (button, SIGNAL (clicked       (bool)),
                        this,   SLOT   (buttonClicked (bool)));

      // save two-way reference
      //
      this->buttonToActionMap.insert (button, buttonSpecs[j].action);
      this->actionToButtonMap.insert (buttonSpecs[j].action, button);
   }
}

//------------------------------------------------------------------------------
//
QEPlotterToolBar::~QEPlotterToolBar ()
{
   // no special action - place holder
}


//------------------------------------------------------------------------------
//
void QEPlotterToolBar::resizeEvent (QResizeEvent *)
{
    // place holder
}

//------------------------------------------------------------------------------
//
void QEPlotterToolBar::setEnabled (QEPlotterNames::MenuActions action, const bool value)
{
   QPushButton* button;

   button = this->actionToButtonMap.value (action, NULL);

   if (button) {
      button->setEnabled (value);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotterToolBar::buttonClicked (bool)
{
   QPushButton* button = dynamic_cast <QPushButton *> (this->sender ());
   QEPlotterNames::MenuActions action;

   if (button) {
      if (this->buttonToActionMap.contains (button)) {
         action = this->buttonToActionMap.value (button);
         emit this->selected (action, NO_SLOT);
      }
   }
}

// end
