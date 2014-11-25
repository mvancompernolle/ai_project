/*  QEArchiveStatus.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QLabel>
#include <QFrame>

#include <QECommon.h>
#include "QEArchiveStatus.h"

#define DEBUG  qDebug () << "QEArchiveStatus::" << __FUNCTION__ << __LINE__


//==============================================================================
//
void QEArchiveStatus::createInternalWidgets ()
{

#define CREATE_LABEL(member, width, align, text)  {              \
   this->rowList [j].member = new QLabel (text, frame);          \
   this->rowList [j].member->setIndent (6);                      \
   this->rowList [j].member->setMinimumWidth (width);            \
   this->rowList [j].member->setAlignment (align);               \
   this->rowList [j].member->setStyleSheet (sheet);              \
   hLayout->addWidget (this->rowList [j].member);                \
}

   const int frameHeight = 19;
   const int horMargin = 2;    // 19 - 2 - 2 => widget height is 15
   const int horSpacing = 4;

   int j;
   QColor background;
   QString sheet;
   QFrame* frame;
   QHBoxLayout *hLayout;

   this->archiveAccess = new QEArchiveAccess (this);

   this->vLayout = new QVBoxLayout (this);
   this->vLayout->setMargin (horMargin);
   this->vLayout->setSpacing (1);

   // Use use the last row as a header row.
   //
   j = NumberRows;
   sheet = "";
   this->rowList [j].frame = frame = new QFrame (this);
   frame->setFixedHeight (frameHeight);
   this->rowList [j].hLayout = hLayout = new QHBoxLayout (frame);
   hLayout->setMargin (horMargin);
   hLayout->setSpacing (horSpacing);

   CREATE_LABEL (hostNamePort, 160, Qt::AlignLeft,    "Host:Port");
   CREATE_LABEL (endPoint,     220, Qt::AlignLeft,    "End Point");
   CREATE_LABEL (state,         88, Qt::AlignHCenter, "Status");
   CREATE_LABEL (available,     68, Qt::AlignRight,   "Available");
   CREATE_LABEL (read,          68, Qt::AlignRight,   "Read");
   CREATE_LABEL (numberPVs,     68, Qt::AlignRight,   "Num PVs");

   this->vLayout->addWidget (frame);


   background = QColor (240, 240, 240, 255);
   sheet = QEUtilities::colourToStyle (background);

   for (j = 0; j < NumberRows; j++ ) {
      QEArchiveStatus::Rows* row = &this->rowList [j];

      row->frame = frame = new QFrame (this);
      frame->setFixedHeight (frameHeight);

      this->rowList [j].hLayout = hLayout = new QHBoxLayout (row->frame);
      hLayout->setMargin (horMargin);
      hLayout->setSpacing (horSpacing);

      CREATE_LABEL (hostNamePort, 160, Qt::AlignLeft,     " - ");
      CREATE_LABEL (endPoint,     220, Qt::AlignLeft,     " - ");
      CREATE_LABEL (state,         88, Qt::AlignHCenter,  " - ");
      CREATE_LABEL (available,     68, Qt::AlignRight,    " - ");
      CREATE_LABEL (read,          68, Qt::AlignRight,    " - ");
      CREATE_LABEL (numberPVs,     68, Qt::AlignRight,    " - ");

      this->vLayout->addWidget (row->frame);

      row->frame->setVisible (false);
   }

   this->vLayout->addStretch ();

#undef CREATE_LABEL

}

//---------------------------------------------------------------------------------
//
void QEArchiveStatus::calcMinimumHeight ()
{
   int count;
   int delta_top;

   // Allow +1 for titles.
   //
   count = this->inUseCount + 1;

   delta_top = 20;
   this->setMinimumHeight ((delta_top * count) + 24);
}

//------------------------------------------------------------------------------
//
QEArchiveStatus::QEArchiveStatus (QWidget* parent) : QEGroupBox (parent)
{
   this->createInternalWidgets();

   this->setTitle (" Archive Status Summary ");
   this->inUseCount = 0;

   QObject::connect (this->archiveAccess,
                     SIGNAL     (archiveStatus (const QEArchiveAccess::StatusList&)),
                     this, SLOT (archiveStatus (const QEArchiveAccess::StatusList&)));

   this->calcMinimumHeight ();
   this->setMinimumWidth (712);

   this->archiveAccess->resendStatus ();
}


//------------------------------------------------------------------------------
//
QEArchiveStatus::~QEArchiveStatus ()
{
}

//------------------------------------------------------------------------------
//
QSize QEArchiveStatus::sizeHint () const
{
   return QSize (712, 64);   // two rows
}

//------------------------------------------------------------------------------
//
void QEArchiveStatus::archiveStatus (const QEArchiveAccess::StatusList& statusList)
{
   int j;

   this->inUseCount = statusList.count ();
   this->calcMinimumHeight ();

   for (j = 0; j < QEArchiveStatus::NumberRows; j++ ) {
      QEArchiveStatus::Rows* row = &this->rowList [j];

      if (j <  statusList.count ()) {
         QEArchiveAccess::Status state = statusList.value (j);

         row->hostNamePort->setText (QString ("%1:%2").arg (state.hostName).arg (state.portNumber));
         row->endPoint->setText (state.endPoint);
         row->state->setText ( QEUtilities::enumToString (*this->archiveAccess,
                                                          QString ("States"), (int) state.state));
         row->available->setText (QString ("%1").arg (state.available));
         row->read->setText (QString ("%1").arg (state.read));
         row->numberPVs->setText (QString ("%1").arg (state.numberPVs));

         row->frame->setVisible (true);

      } else {
         row->frame->setVisible (false);
      }
   }
}

// end
