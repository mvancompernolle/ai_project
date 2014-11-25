/*  QEScalarHistogram.cpp
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */


#include <QDebug>
#include <QECommon.h>
#include <QCaObject.h>
#include <QEScalarHistogram.h>

#define DEBUG  qDebug () << "QEScalarHistogram" << __FUNCTION__ << __LINE__

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEScalarHistogram::QEScalarHistogram (QWidget * parent) : QEFrame (parent)
{
   // Set default property values
   // Super class....
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // Create internal widget.
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->histogram = new QEHistogram (this);
   this->layout->addWidget (this->histogram);

   // Set histogram properties.
   //
   this->histogram->setFrameShape (QFrame::NoFrame);
   this->histogram->setFrameShadow (QFrame::Plain);
   this->histogram->setAutoScale (true);

   this->histogram->setMouseTracking (true);
   this->histogram->installEventFilter (this);

   this->mScaleMode = Manual;

   for (int j = 0; j < ARRAY_LENGTH (this->isFirstUpdate); j++) {
      this->isFirstUpdate [j] = true;
   }

   // Set up data
   //
   this->setNumVariables (ARRAY_LENGTH (this->vnpm));

   // Set up default properties
   //
   this->setAllowDrop (false);

   // Use default context menu.
   //
   this->setupContextMenu ();
   this->selectedChannel = -1;

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setVariableIndex (j);
      QObject::connect (&this->vnpm [j], SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
                        this,            SLOT   (newVariableNameProperty  (QString, QString, unsigned int)));
   }
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEScalarHistogram::newVariableNameProperty (QString pvName, QString subs, unsigned int pvi)
{
   this->histogram->clearValue ((int) pvi);
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a histogram floating point numbers are required.
//
qcaobject::QCaObject* QEScalarHistogram::createQcaItem (unsigned int pvi)
{
   qcaobject::QCaObject* result = NULL;

   if (pvi < ARRAY_LENGTH (this->vnpm)) {
      QString pvName = this->getSubstitutedVariableName (pvi);
      result = new QEFloating (pvName, this, &this->floatingFormatting, pvi);
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
//
void QEScalarHistogram::establishConnection (unsigned int variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca, SIGNAL (floatingChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (setChannelValue (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int &)));
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEScalarHistogram::connectionChanged (QCaConnectionInfo & connectionInfo,
                                           const unsigned int & variableIndex)
{
   bool pvConnected;

   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   pvConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (pvConnected, variableIndex);

   // This is a multi PV widget.
   // Do not use updateConnectionStyle.
   //
   // If this is a disconnect - set gray.
   // If this is a connect, we will soon change from gray to required colour.
   //
   this->histogram->setColour ((int) variableIndex, QColor (0xe8e8e8));

   // More trob. than it's worth to check if this is a connect or disconnect.
   //
   this->isFirstUpdate [variableIndex] = true;
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::updateHistogramScale ()
{
   qcaobject::QCaObject* qca = NULL;
   double lopr;
   double hopr;

   switch (this->mScaleMode) {

      case Manual:
         this->histogram->setAutoScale (false);
         break;

      case Auto:
         this->histogram->setAutoScale (true);
         break;

      case OperationalRange:
         lopr = 0.0;
         hopr = 0.0;
         for (int j = 0; j < QE_HISTOGRAM_NUMBER_VARIABLES; j++) {
            qca = this->getQcaItem (j);
            if (qca) {
               double lowLim = qca->getDisplayLimitLower ();
               double upLim = qca->getDisplayLimitUpper ();

DEBUG << j << lowLim << upLim;

               // Has the operating range been defined??
               //
               if ((lowLim != 0.0) || (upLim != 0.0)) {
                  // Yes - incorporate into overall operating range.
                  //
                  lopr = MIN (lopr, lowLim);
                  hopr = MAX (hopr, upLim);
               }
            }
         }

         // Has at least one PV specified a valid range?
         //
         if ((lopr != 0.0) || (hopr != 0.0)) {
            // Yes - use the range.
            //
            this->histogram->setMinimum (lopr);
            this->histogram->setMaximum (hopr);
            this->histogram->setAutoScale (false);
         }
         // else just leave as is.

         break;
   }
}



//------------------------------------------------------------------------------
// Update the histogram bar value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QEScalarHistogram::setChannelValue (const double& value,
                                         QCaAlarmInfo& alarmInfo,
                                         QCaDateTime&,
                                         const unsigned int &variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   QColor colour;

   if (this->getDisplayAlarmState ()) {
      colour = this->getColor (alarmInfo, 255);
   } else {
      colour = this->histogram->getBarColour ();
   }

   this->histogram->setColour ((int) variableIndex, colour);
   this->histogram->setValue ((int) variableIndex, value);

   // First update (for this connection).
   //
   if (this->isFirstUpdate [variableIndex]) {
      this->updateHistogramScale ();
   }

   // Don't invoke common alarm handling processing.
   // Invoke for tool tip processing directly.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setPvNameSubstitutions (const QString& pvNameSubstitutions)
{
   // Set all (but read 1).
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setSubstitutionsProperty (pvNameSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QEScalarHistogram::getPvNameSubstitutions () const
{
   // All the same - any will do.
   //
   return this->vnpm [0].getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setScaleMode (const ScaleModes scaleModeIn)
{
   this->mScaleMode = scaleModeIn;
   this->updateHistogramScale ();
}

//------------------------------------------------------------------------------
//
QEScalarHistogram::ScaleModes QEScalarHistogram::getScaleMode () const
{
   return this->mScaleMode;
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
// Index is channel number.
//
void QEScalarHistogram::genReadOut (const int index)
{
   qcaobject::QCaObject* qca = NULL;
   QString text;

   if (index >= 0) {
      qca = this->getQcaItem ((unsigned int) index);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);

      text = qca->getRecordName ();
      if (isDefined) {
         text.append (" ").append (value.toString ());
         text.append(" ").append (qca->getEgu ());
      } else {
         text.append ("  undefined.");
      }
   } else {
      text = "";
   }
   this->setReadOut (text);
}

//==============================================================================
// Events.
//
bool QEScalarHistogram::eventFilter (QObject *obj, QEvent *event)
{
   const Qt::MouseButtons leftRight= Qt::LeftButton | Qt::RightButton;
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   Qt::MouseButton button;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         button = mouseEvent->button();
         if ((obj == this->histogram) && ((button & leftRight) != 0)) {
            // We are about to launch the context menu or drag variable or data.
            // Identify current item (if any).
            //
            QMouseEvent*mouseEvent = static_cast<QMouseEvent *> (event);
            this->selectedChannel = this->histogram->indexOfPosition (mouseEvent->pos ());
            // event has not fully handled - fall through.
         }
         break;

      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->histogram) {
            int index = this->histogram->indexOfPosition (mouseEvent->pos ());
            this->genReadOut (index);
            return true; // event has been handled.
         }
         break;

      default:
         // Just fall through
         break;
   }

   return false; // we did not handle this event.
}

//==============================================================================
// Drag drop
//
void QEScalarHistogram::setDrop (QVariant drop)
{
   // Find unused slot ....
   setVariableName (drop.toString (), 0);
   establishConnection (0);
}

QVariant QEScalarHistogram::getDrop ()
{
   if (isDraggingVariable ()) {
      return QVariant (this->copyVariable ());
   } else {
      return this->copyData ();
   }
}

//==============================================================================
// Copy (no paste)
//
QString QEScalarHistogram::copyVariable ()
{
   if (this->selectedChannel >= 0) {
      return this->getSubstitutedVariableName ((unsigned int) this->selectedChannel);
   } else {
      return "";
   }
}

QVariant QEScalarHistogram::copyData ()
{
   QVariant result (QVariant::Invalid);
   qcaobject::QCaObject* qca = NULL;

   if (this->selectedChannel >= 0) {
      qca = this->getQcaItem ((unsigned int) this->selectedChannel);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);
      if (isDefined) {
         result = value;
      }
   } else {
      result = QVariant ();
   }

   return result;
}

// end
