/*  QENumericEdit.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>
#include <QDebug>
#include <QVariant>
#include <QEFloating.h>

#include <QECommon.h>
#include "QENumericEdit.h"

#define DEBUG  qDebug () << "QENumericEdit::" << __FUNCTION__ << __LINE__

#define MAX_SIGNIFICANCE   15
#define NUMBER_OF_RADICES  4

//==============================================================================
// Local functions.
// Maybe they should be static functions of the QENumericEdit class
//==============================================================================
//

// For decimal, this is about 48.9 bits, for the others 48 bits exactly.
//
const static int maximum_number_digits_list [NUMBER_OF_RADICES] = { 15, 12, 16, 48 };

// Used to modify the tool tip.
//
static const char* custom_tips [NUMBER_OF_RADICES] = { "", "hex", "octal", "binary" };


//------------------------------------------------------------------------------
// Example: leading zeros = 2, precision = 1, radix = 10, then max
// value is 99.9 =  10**2 - 10**(-1)
//
double QENumericEdit::calcUpper ()
{
   const double dblRadix = double (this->fpr.getRadixValue ());

   double a, b;

   a = pow (dblRadix, +this->getLeadingZeros ());
   b = pow (dblRadix, -this->getPrecision ());

   return a - b;
}

//------------------------------------------------------------------------------
//
double QENumericEdit::calcLower ()
{
   return -calcUpper ();
}


//==============================================================================
// QENumericEdit
//==============================================================================
//
QENumericEdit::QENumericEdit (QWidget * parent) : QEGenericEdit (parent)
{
   this->commonConstructor ();
}

//------------------------------------------------------------------------------
//
QENumericEdit::QENumericEdit (const QString & variableNameIn, QWidget * parent): QEGenericEdit (variableNameIn, parent)
{
   this->commonConstructor ();
}

//------------------------------------------------------------------------------
//
void QENumericEdit::commonConstructor ()
{
   this->mAutoScale = true;
   this->mCursor = 1;

   this->addUnits = true;
   this->fpr.setRadix (QEFixedPointRadix::Decimal);
   this->fpr.setSeparator (QEFixedPointRadix::None);

   // Ensure sensible auto values
   //
   this->propertyPrecision = 4;
   this->propertyLeadingZeros = 3;
   this->propertyMinimum = this->calcLower ();
   this->propertyMaximum = this->calcUpper ();

   // Ensure sensible auto values
   // Note: these should be re-calcuated when the first update arrives.
   //
   this->autoPrecision = this->propertyPrecision;
   this->autoLeadingZeros = this->propertyLeadingZeros;
   this->autoMinimum = this->propertyMinimum;
   this->autoMaximum = this->propertyMaximum;

   // force setNumericValue to process.
   //
   this->firstNumericUpdate = true;
   this->setNumericValue (0.0, false);
}

//------------------------------------------------------------------------------
//
QENumericEdit::~QENumericEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a numeric edit, a QCaObject that streams floating
// point (double) numbers is required.
//
qcaobject::QCaObject * QENumericEdit::createQcaItem (unsigned int variableIndex)
{
   // Sanity check ...
   //
   if (variableIndex != 0) {
      QString message;

      message.sprintf ("QENumericEdit::createQcaItem - invalid variable index %d", variableIndex);
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      return NULL;
   }

   return new QEFloating (this->getSubstitutedVariableName (variableIndex),
                          this,    // eventObject
                          &this->floatingFormatting,
                          variableIndex);
}


//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QENumericEdit::establishConnection (unsigned int variableIndex)
{
   // Sanity check ...
   //
   if (variableIndex != 0) {
      QString message;

      message.sprintf ("QENumericEdit::establishConnection - invalid variable index %d", variableIndex);
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject * qca = createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if ((qca) && (variableIndex == 0)) {
      QObject::connect (qca, SIGNAL (floatingChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (setDoubleValue  (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      // The connectionChanged slot is in parent class.
      //
      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo &)),
                        this, SLOT  (connectionChanged (QCaConnectionInfo &)));
   }
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setDoubleValue (const double& valueIn, QCaAlarmInfo& alarmInfo,
                                    QCaDateTime& dateTime, const unsigned int& variableIndex)
{
   qcaobject::QCaObject * qca = this->getQcaItem (0);

   // Sanity check ...
   //
   if (variableIndex != 0) {
      QString message;

      message.sprintf ("QENumericEdit::setDoubleValue - invalid variable index %d", variableIndex);
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      return;
   }

   // Check first update.
   //
   this->firstNumericUpdate = this->testAndClearIsFirstUpdate ();
   if (this->firstNumericUpdate) {

      // Check for auto scale and avoid the segment fault.
      //
      if (this->getAutoScale () && (qca)) {

         // Do the auto scale calculations.
         //
         double ctrlLow;
         double ctrlUpp;
         enum Priority priority;

         // Check that sensible limits have been defined and not just left
         // at the default (i.e. zero) values by a lazy database creator.
         // Otherwise, leave as design time limits.
         //
         ctrlLow = qca->getControlLimitLower ();
         ctrlUpp = qca->getControlLimitUpper ();

         // If control limits are undefined - try display limits.
         //
         if ((ctrlLow == 0.0) && (ctrlUpp == 0.0)) {
            ctrlLow = qca->getDisplayLimitLower ();
            ctrlUpp = qca->getDisplayLimitUpper ();
         }

         if ((ctrlLow != 0.0) || (ctrlUpp != 0.0)) {
            // At least one of these limits is non-zero - assume database creator
            // has thought about this.
            //
            this->autoMinimum = ctrlLow;
            this->autoMaximum = ctrlUpp;
            priority = ABS (ctrlLow) > ABS (ctrlUpp) ? min : max;
         } else {
            // Just go with design property values.
            this->autoMinimum = this->getPropertyMinimum ();
            this->autoMaximum = this->getPropertyMaximum ();
            this->autoLeadingZeros = this->getPropertyLeadingZeros ();
            priority = prec;
         }
         this->autoPrecision = qca->getPrecision ();

         this->rationalise (priority, this->autoLeadingZeros, this->autoPrecision,
                            this->autoMinimum, this->autoMaximum);

      }  // else no auto scaling, just go with design property values.
         // Note: this have already been pre rationalised
   }

   // Get on with update proper.  Do generic update processing.
   //
   this->setDataIfNoFocus (QVariant (valueIn), alarmInfo, dateTime);

   // Signal a database value change to any Link widgets
   //
   emit dbValueChanged (valueIn);
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getRadixValue ()
{
   return this->fpr.getRadixValue ();
}

//------------------------------------------------------------------------------
//
int QENumericEdit::maximumSignificance ()
{
   return maximum_number_digits_list [this->getRadix ()];
}

//------------------------------------------------------------------------------
//
void QENumericEdit::keyPressEvent (QKeyEvent * event)
{
   const int key = event->key ();
   const double dblRadix = double (this->getRadixValue ());

   int index;
   int significance;
   QChar qc;
   QChar qk;
   int j;
   double delta;
   double newval;
   bool invokeInherited;

   invokeInherited = false;
   switch (key) {

      case Qt::Key_Up:
         index = this->getCursor ();
         qc = this->text () [index];

         // Is this a digit charcter?
         //
         if (this->isRadixDigit (qc)) {
            significance = -this->getPrecision ();
            for (j = index + 1; j <= this->lengthOfImageValue(); j++) {
               qc = this->text () [j];
               if (this->isRadixDigit(qc)) {
                  significance++;
               }
            }
            delta = pow (dblRadix, significance);
            this->setNumericValue (this->getNumericValue () + delta, true);
         } else if (this->cursorOverSign ()) {
            this->setNumericValue (+fabs (this->getNumericValue ()), true);
         }
         break;


      case Qt::Key_Down:
         index = this->getCursor ();
         qc = this->text () [index];

         // Is this a digit charcter?
         //
         if (this->isRadixDigit(qc)) {
            significance = -this->getPrecision ();
            for (j = index + 1; j <= this->lengthOfImageValue(); j++) {
               qc = this->text () [j];
               if (this->isRadixDigit(qc)) {
                  significance++;
               }
            }
            delta = pow (dblRadix, significance);
            this->setNumericValue (this->getNumericValue () - delta, true);
         } else if (this->cursorOverSign ()) {
            this->setNumericValue (-fabs (this->getNumericValue ()), true);
         }
         break;


      case Qt::Key_Left:
         this->setCursor (this->getCursor () - 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->text () [this->getCursor ()];
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () - 1);
         }
         break;


      case Qt::Key_Right:
         this->setCursor (this->getCursor () + 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->text () [this->getCursor ()];
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () + 1);
         }
         break;

      case Qt::Key_Plus:
      case Qt::Key_Minus:
         if (this->cursorOverSign ()) {
            if (key == Qt::Key_Plus) {
               this->setNumericValue (+fabs (this->getNumericValue ()), true);
            } else {
               this->setNumericValue (-fabs (this->getNumericValue ()), true);
            }
            this->setCursor (this->getCursor () + 1);
         }
         break;

      case Qt::Key_0:
      case Qt::Key_1:
      case Qt::Key_2:
      case Qt::Key_3:
      case Qt::Key_4:
      case Qt::Key_5:
      case Qt::Key_6:
      case Qt::Key_7:
      case Qt::Key_8:
      case Qt::Key_9:
      // we do not care about shift here, allow 'A' or 'a' etc.
      case Qt::Key_A:
      case Qt::Key_B:
      case Qt::Key_C:
      case Qt::Key_D:
      case Qt::Key_E:
      case Qt::Key_F:
         qk = QChar (key);
         index = this->getCursor ();
         qc = this->text () [index];

         // Both the new char and the existing char must both be radix digits.
         //
         if (this->isRadixDigit (qk) && this->isRadixDigit (qc)) {
            QString tryThis = this->text ();
            tryThis [index] = QChar (key);

            newval = this->valueOfImage (tryThis);
            this->setNumericValue (newval, true);
            this->setCursor (this->getCursor () + 1);

            // If we have moved onto a filler character, then move again.
            //
            qc = this->text () [this->getCursor ()];
            if (!this->isSignOrDigit (qc)) {
               this->setCursor (this->getCursor () + 1);
            }
         }
         break;

      case Qt::Key_Space:
         break;

      case Qt::Key_Backspace:
      case Qt::Key_Delete:
         break;

      default:
         // Only reprocess enter, return etc.
         invokeInherited = (key >= 256);
         break;
   }

   if (invokeInherited) {
      this->QEGenericEdit::keyPressEvent (event);
   }
}

//------------------------------------------------------------------------------
//
void QENumericEdit::focusInEvent (QFocusEvent * event)
{
   if (event->gotFocus () != true) {
      // This is kind of unexpected.
      //
      DEBUG << this->objectName () << "focusInEvent  got: "
            << event->gotFocus () << "   lost" << event->lostFocus ();
   }
   // Call parent function.
   //
   this->QEGenericEdit::focusInEvent (event);
   this->setDigitSelection ();
}

//------------------------------------------------------------------------------
//
void QENumericEdit::mouseReleaseEvent (QMouseEvent * event)
{
   int posn;

   // Call parent function.
   //
   this->QEGenericEdit::mouseReleaseEvent (event);

   posn = LIMIT (this->cursorPosition (), 1, this->lengthOfImageValue () - 1);

   this->setCursor (posn);
   this->setDigitSelection ();
}


//------------------------------------------------------------------------------
//
void QENumericEdit::setDigitSelection ()
{
   int posn;

   // Only set/update selection if/when the widget has focus.
   //
   if (this->hasFocus()) {
      posn = this->getCursor ();
      this->setSelection (posn, 1);
   }
}


//------------------------------------------------------------------------------
//
QString QENumericEdit::imageOfValue ()
{
   QString image;

   image = this->fpr.toString (this->mValue, this->showSign (),
                               this->getLeadingZeros(), this->getPrecision ());

   image.prepend (" ");
   return image;
}

//------------------------------------------------------------------------------
//
int QENumericEdit::lengthOfImageValue ()
{
   // maybe calc once each time critical parameters are set?
   // or attempt to calculate.
   return this->imageOfValue ().length ();
}

//------------------------------------------------------------------------------
//
double QENumericEdit::valueOfImage (const QString & image)
{
   bool okay;
   double result;

   result = this->fpr.toValue (image.mid (0, this->lengthOfImageValue ()), okay);

   if (!okay) {
      result = this->mValue;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setNumericText ()
{
   QString image = this->imageOfValue ();
   qcaobject::QCaObject* qca;
   QString units;

   if (this->getAddUnits ()) {
      qca = this->getQcaItem (0);
      if (qca) {
         units = qca->getEgu();
         if (!units.isEmpty ()) {
               image.append (" ").append (units);
         }
      }
   }

   // Note: this has an intended side effect.
   // TODO: Explain this more !!!
   //
   this->setMaxLength (image.length ());

   this->setText (image);
   this->setDigitSelection ();
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setCursor (const int value)
{
   // cursor postion is zero based - skip leading zero.
   //
   this->mCursor = LIMIT (value, 1, this->lengthOfImageValue () - 1);
   this->setDigitSelection ();
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getCursor ()
{
   return this->mCursor;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::isRadixDigit (QChar qc)
{
   return this->fpr.isRadixDigit (qc);
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::isSign (QChar qc)
{
   char c = qc.toLatin1 ();

   return ((c == '+') || (c == '-'));
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::isSignOrDigit (QChar qc)
{
   return (this->isSign (qc) || this->isRadixDigit (qc));
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setNumericValue (const double value, const bool userUpdate)
{
   double limited_value;

   limited_value = LIMIT (value, this->getMinimum (), this->getMaximum ());

   if (this->mValue != limited_value || this->firstNumericUpdate) {
      this->mValue = limited_value;
      this->setNumericText ();
      if (userUpdate) this->setModified (true);
   }
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getNumericValue ()
{
   return this->mValue;
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getPrecision ()
{
   return (this->getIsConnected () && this->getAutoScale ()) ? this->autoPrecision : this->propertyPrecision;
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getLeadingZeros ()
{
   return (this->getIsConnected () && this->getAutoScale ()) ? this->autoLeadingZeros : this->propertyLeadingZeros;
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getMinimum ()
{
    return (this->getIsConnected () && this->getAutoScale ()) ? this->autoMinimum : this->propertyMinimum;
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getMaximum ()
{
    return (this->getIsConnected () && this->getAutoScale ()) ? this->autoMaximum : this->propertyMaximum;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setAutoScale (const bool value)
{
   if (this->mAutoScale != value) {
      this->mAutoScale = value;
      this->setNumericText ();
   }
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getAutoScale ()
{
   return this->mAutoScale;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setPropertyPrecision (const int value)
{
   this->propertyPrecision = value;
   this->rationalise (prec, this->propertyLeadingZeros, this->propertyPrecision, this->propertyMinimum, this->propertyMaximum);
   if (!this->getIsConnected ()) {

   }
   this->setNumericText ();
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getPropertyPrecision ()
{
   return this->propertyPrecision;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setPropertyLeadingZeros (const int value)
{
   this->propertyLeadingZeros = value;
   this->rationalise (zeros, this->propertyLeadingZeros, this->propertyPrecision, this->propertyMinimum, this->propertyMaximum);
   this->setNumericText ();
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getPropertyLeadingZeros ()
{
   return this->propertyLeadingZeros;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setPropertyMinimum (const double value)
{
   this->propertyMinimum = value;
   this->rationalise (min, this->propertyLeadingZeros, this->propertyPrecision, this->propertyMinimum, this->propertyMaximum);
   this->setNumericText ();
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getPropertyMinimum ()
{
   return this->propertyMinimum;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setPropertyMaximum (const double value)
{
   this->propertyMaximum = value;
   this->rationalise (max, this->propertyLeadingZeros, this->propertyPrecision, this->propertyMinimum, this->propertyMaximum);
   this->setNumericText ();
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getPropertyMaximum ()
{
   return this->propertyMaximum;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setAddUnits (bool addUnitsIn)
{
   if (this->addUnits != addUnitsIn) {
      this->addUnits = addUnitsIn;
      this->setNumericText ();      // Redisplay text.
   }
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getAddUnits ()
{
   return this->addUnits;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setRadix (const QEFixedPointRadix::Radicies value)
{
   if (this->fpr.getRadix () != value) {
      this->fpr.setRadix (value);
      this->setNumericText ();      // Redisplay text.
      this->updateToolTipCustom ( custom_tips [this->getRadix ()] );
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Radicies QENumericEdit::getRadix ()
{
   return this->fpr.getRadix ();
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setSeparator (const QEFixedPointRadix::Separators value)
{
   if (this->fpr.getSeparator () != value) {
      this->fpr.setSeparator (value);
      this->setNumericText ();      // Redisplay text.
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Separators QENumericEdit::getSeparator ()
{
   return this->fpr.getSeparator ();
}


//------------------------------------------------------------------------------
//
bool QENumericEdit::showSign ()
{
   // Only force '+' if sign can be -ve.
   //
   return (this->getMinimum () < 0.0);
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::cursorOverSign ()
{
   return (this->showSign () && (this->getCursor () == 1));
}

//------------------------------------------------------------------------------
//
void QENumericEdit::rationalise (const enum Priority priority,
                                 int & leadingZerosInOut,
                                 int & precisionInOut,
                                 double & minimumInOut,
                                 double & maximumInOut)
{
   double temp;

   switch (priority) {

      case zeros:
         leadingZerosInOut = LIMIT (leadingZerosInOut, 1, MAX_SIGNIFICANCE);
         // Reduce precision so as not to exceed max significance if required.
         //
         precisionInOut = MIN (precisionInOut, MAX_SIGNIFICANCE - leadingZerosInOut);
         break;

      case prec:
         precisionInOut = LIMIT (precisionInOut, 0, MAX_SIGNIFICANCE);

         // Reduce (effective) number of leading zeros so as not to exceed max
         // significance if required.
         //
         leadingZerosInOut = MIN (leadingZerosInOut, MAX_SIGNIFICANCE - precisionInOut);
         break;

      case min:
         temp = MAX (1.0, ABS (minimumInOut));

         // the (int) cast truncates to 0
         leadingZerosInOut = 1 + (int) (log10 (temp));

         leadingZerosInOut = LIMIT (leadingZerosInOut, 1, MAX_SIGNIFICANCE);

         // Reduce precision so as not to exceed max significance if required.
         //
         precisionInOut = MIN (precisionInOut, MAX_SIGNIFICANCE - leadingZerosInOut);
         break;

      case max:
         temp = MAX (1.0, ABS (maximumInOut));

         // (int) cast truncates to 0
         leadingZerosInOut = 1 + (int) (log10 (temp));

         leadingZerosInOut = LIMIT (leadingZerosInOut, 1, MAX_SIGNIFICANCE);

         // Reduce precision so as not to exceed max significance if required.
         //
         precisionInOut = MIN (precisionInOut, MAX_SIGNIFICANCE - leadingZerosInOut);
         break;
   }

   // Re-caluclate the min/max values and constrain values if needs be.
   //
   minimumInOut = MAX (minimumInOut, this->calcLower ());
   maximumInOut = MIN (maximumInOut, this->calcUpper ());
}


//==============================================================================
// Set widget to the given value
//
void QENumericEdit::setValue (const QVariant & value)
{
   double d;
   bool ok;

   d = value.toDouble (&ok);
   if (ok) {
       this->setNumericValue (d, false);
   }
}

//------------------------------------------------------------------------------
//
QVariant QENumericEdit::getValue()
{
   return QVariant (this->getNumericValue ());
}

//------------------------------------------------------------------------------
// Write the given value to the associated channel.
//
bool QENumericEdit::writeData (const QVariant & value, QString& message)
{
   QEFloating *qca = dynamic_cast <QEFloating*> ( this->getQcaItem(0) );
   double d;
   bool ok;

   d = value.toDouble (&ok);
   if (qca && ok) {
      qca->writeFloating (d);
      return true;
   } else {
      message = "null qca object and/or value is not a double";
      return false;
   }
}

// end
