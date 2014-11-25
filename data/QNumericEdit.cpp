/*  QNumericEdit.cpp
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>
#include <QDebug>

#include "QNumericEdit.h"

#define DEBUG  qDebug () << "QNumericEdit" << __LINE__ << __FUNCTION__ << ":  "

#define NUMBER_OF_RADICES  4

//==============================================================================
//
// For decimal, this is about 48.9 bits, for the other radix values this is 48 bits exactly.
// Note: order MUST be consistant with enum Radicies specification.
//
const static int maximumNumberDigits [NUMBER_OF_RADICES] = { 15, 12, 16, 48 };


//==============================================================================
// QNumericEdit
//==============================================================================
//
QNumericEdit::QNumericEdit (QWidget * parent) : QWidget (parent)
{
   this->commonConstructor ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::commonConstructor ()
{
   this->fpr.setRadix (QEFixedPointRadix::Decimal);
   this->fpr.setSeparator (QEFixedPointRadix::None);

   this->lineEdit = new QLineEdit (this);
   this->lineEdit->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
   this->lineEdit->setAlignment (Qt::AlignRight);
   this->lineEdit->installEventFilter (this);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);
   this->layout->setSpacing (0);
   this->layout->addWidget (this->lineEdit);

   //QFont font = this->font ();
   //font.setFamily ("Monospace");
   //this->setFont (font);


   // Ensure sensible auto values
   //
   this->mPrefix = "";
   this->mSuffix = "";
   this->mLeadingZeros = 3;
   this->mPrecision = 4;
   this->minimumMin  = this->calcLower ();
   this->maximumMax = this->calcUpper ();
   this->mMinimum = this->minimumMin;
   this->mMaximum = this->maximumMax;

   // force setValue to process.
   //
   this->mValue = 0.1;  // force initial update
   this->setValue (0.0);
   this->cursor = this->cursorFirst;
}

//------------------------------------------------------------------------------
//
QNumericEdit::~QNumericEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
int QNumericEdit::maximumSignificance () const
{
   return maximumNumberDigits [this->fpr.getRadix ()];
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditKeyPressEvent (QKeyEvent * event)
{
   const int key = event->key ();
   const double dblRadix = double (this->fpr.getRadixValue ());

   int index;
   int significance;
   QChar qc;
   QChar qk;
   int j;
   double delta;
   double newval;
   bool handledLocally;

   handledLocally = true;
   switch (key) {

      case Qt::Key_Up:
         index = this->getCursor ();
         qc = this->charAt (index);

         // Is this a digit charcter?
         //
         if (this->isRadixDigit (qc)) {
            significance = -this->mPrecision;
            for (j = index + 1; j <= this->cursorLast; j++) {
               qc = this->charAt (j);
               if (this->isRadixDigit(qc)) {
                  significance++;
               }
            }
            delta = pow (dblRadix, significance);
            this->setValue (this->getValue () + delta);
         } else if (this->cursorOverSign ()) {
            this->setValue (+fabs (this->getValue ()));
         }
         break;


      case Qt::Key_Down:
         index = this->getCursor ();
         qc = this->charAt (index);

         // Is this a digit charcter?
         //
         if (this->isRadixDigit(qc)) {
            significance = -this->mPrecision;
            for (j = index + 1; j <= this->cursorLast; j++) {
               qc = this->charAt (j);
               if (this->isRadixDigit (qc)) {
                  significance++;
               }
            }
            delta = pow (dblRadix, significance);
            this->setValue (this->getValue () - delta);
         } else if (this->cursorOverSign ()) {
            this->setValue (-fabs (this->getValue ()));
         }
         break;


      case Qt::Key_Left:
         this->setCursor (this->getCursor () - 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->charAt (this->getCursor ());
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () - 1);
         }
         break;


      case Qt::Key_Right:
         this->setCursor (this->getCursor () + 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->charAt (this->getCursor ());
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () + 1);
         }
         break;


      case Qt::Key_Plus:
      case Qt::Key_Minus:
         if (this->cursorOverSign ()) {
            if (key == Qt::Key_Plus) {
               this->setValue (+fabs (this->getValue ()));
            } else {
               this->setValue (-fabs (this->getValue ()));
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
         qc = this->charAt (index);

         // Both the new char and the existing char must both be radix digits.
         //
         if (this->isRadixDigit (qk) && this->isRadixDigit (qc)) {
            QString tryThis = this->lineEdit->text ();
            tryThis [index] = QChar (key);

            newval = this->valueOfImage (tryThis);
            this->setValue (newval);
            this->setCursor (this->getCursor () + 1);

            // If we have moved onto a filler character, then move again.
            //
            qc = this->charAt (this->getCursor ());
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

      case Qt::Key_Return:
      case Qt::Key_Enter:
         emit returnPressed ();
         emit editingFinished ();
         break;

      case Qt::Key_Tab:
         handledLocally = false;
         break;

      default:
         handledLocally = (key < 256);
         break;
   }

   // All events handled locally.
   //
   return handledLocally;
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditFocusInEvent (QFocusEvent * event)
{
   bool result = false;   // not handled unless we actually handle this event.

   if (event->gotFocus ()) {
      this->setDigitSelection ();
      result = true;

   } else if (event->lostFocus ()) {
      emit editingFinished ();
       result = false;    // sort of handled, but not totally.

   } else {
      DEBUG << "Unexpected (not handled)";
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditMouseReleaseEvent (QMouseEvent* /* event*/ )
{
   int posn = this->lineEdit->cursorPosition ();
   this->setCursor (posn);
   return true;  //  handled locally
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditEventFilter (QEvent *event)
{
   const QEvent::Type type = event->type ();

   bool result = false;   // not handled unless we actually handle this event.

   switch (type) {

      case QEvent::MouseButtonRelease:
         result = this->lineEditMouseReleaseEvent (static_cast<QMouseEvent *> (event));
         break;

      case QEvent::KeyPress:
         result = this->lineEditKeyPressEvent (static_cast<QKeyEvent *> (event));
         break;

      case QEvent::FocusIn:
      case QEvent::FocusOut:
         result = this->lineEditFocusInEvent (static_cast<QFocusEvent *> (event));
         break;

      default:
         // Just fall through - not handled.
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::eventFilter (QObject *obj, QEvent *event)
{
   bool result = false;   // not handled unless we actually handle this event.

   if (obj == this->lineEdit) {
      result = lineEditEventFilter (event);
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setDigitSelection ()
{
   // Only set/update selection if/when the widget has focus.
   //
   if (this->lineEdit->hasFocus ()) {
      int posn = this->getCursor ();
      this->lineEdit->setSelection (posn, 1);
   }
}

//------------------------------------------------------------------------------
//
double QNumericEdit::valueOfImage (const QString& image) const
{
   int length;
   QString intermediate;
   bool okay;
   double result;

   length = this->cursorLast - this-> cursorFirst + 1;
   intermediate = image.mid (this->cursorFirst, length);

   result = this->fpr.toValue (intermediate, okay);
   if (!okay) {
      result = this->mValue;
   }
   return result;
}

//------------------------------------------------------------------------------
// Compare with cleanText ()
//
QString QNumericEdit::imageOfValue () const
{
   QString image;

   image = this->fpr.toString (this->mValue, this->showSign (),
                               this->mLeadingZeros, this->mPrecision);
   image = image.trimmed ();

   // Using zero based text access....
   //
   return this->mPrefix + image + this->mSuffix;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::redisplayText ()
{
   QString image = this->imageOfValue ();

   // Note: this has an intended side effect.
   // TODO: Explain this more !!!
   //
   this->lineEdit->setMaxLength (image.length ());
   this->lineEdit->setText (image);

   this->cursorFirst = this->mPrefix.length ();
   this->cursorLast = image.length () - 1 - this->mSuffix.length ();

   this->setCursor (this->cursor);
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setCursor (const int value)
{
   // Ensure cursor is in range of interest, i.e. excluding prefix/suffix.
   //
   this->cursor = LIMIT (value, this->cursorFirst, this->cursorLast);
   this->setDigitSelection ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getCursor () const
{
   return this->cursor;
}

//------------------------------------------------------------------------------
//
QChar QNumericEdit::charAt (const int j) const
{
   return this->lineEdit->text () [j];
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isRadixDigit (QChar qc) const
{
   return this->fpr.isRadixDigit (qc);
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isSign (QChar qc) const
{
   char c = qc.toLatin1 ();

   return ((c == '+') || (c == '-'));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isSignOrDigit (QChar qc) const
{
   return (this->isSign (qc) || this->isRadixDigit (qc));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::showSign () const
{
   // Only force '+' if sign can be -ve.
   //
   return (this->mMinimum < 0.0);
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::cursorOverSign () const
{
   return (this->showSign () && (this->getCursor () == this->cursorFirst));
}

//------------------------------------------------------------------------------
// Example: leading zeros = 2, precision = 1, radix = 10, then max
// value is 99.9 =  10**2 - 10**(-1)
//
double QNumericEdit::calcUpper () const
{
   const double dblRadix = double (this->fpr.getRadixValue ());

   double a, b;

   a = pow (dblRadix, +this->mLeadingZeros);
   b = pow (dblRadix, -this->mPrecision);

   return a - b;
}

//------------------------------------------------------------------------------
//
double QNumericEdit::calcLower () const
{
   return -this->calcUpper ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::applyLimits ()
{
   // Recalculated allowed min/max range.
   //
   this->minimumMin = this->calcLower ();
   this->maximumMax = this->calcUpper ();

   // Restrict min and max as required.
   //
   this->mMinimum = MAX (this->mMinimum, this->minimumMin);
   this->mMaximum = MIN (this->mMaximum, this->maximumMax);

   this->setValue (this->getValue ());   // Set value forces min/max limits.
}

//------------------------------------------------------------------------------
// Property functions.
//------------------------------------------------------------------------------
//
void  QNumericEdit::setPrefix (const QString &prefix)
{
   this->mPrefix = prefix;
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
QString  QNumericEdit::getPrefix () const
{
   return this->mPrefix;
}


//------------------------------------------------------------------------------
//
void  QNumericEdit::setSuffix (const QString &suffix)
{
   this->mSuffix = suffix;
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
QString  QNumericEdit::getSuffix () const
{
   return this->mSuffix;
}

//------------------------------------------------------------------------------
//
QString QNumericEdit::getCleanText () const
{
   QString cleanText;

   cleanText = this->fpr.toString (this->mValue, this->showSign (),
                                   this->mLeadingZeros, this->mPrecision);

   cleanText = cleanText.trimmed ();
   return cleanText;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setLeadingZeros (const int value)
{
   this->mLeadingZeros = LIMIT (value, 0, this->maximumSignificance ());

   // Reduce precision so as not to exceed max significance if required.
   //
   this->mPrecision = MIN (this->mPrecision, this->maximumSignificance () - this->mLeadingZeros);

   this->applyLimits ();
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getLeadingZeros () const
{
   return this->mLeadingZeros;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setPrecision (const int value)
{
   this->mPrecision = LIMIT (value, 0, this->maximumSignificance ());

   // Reduce precision so as not to exceed max significance if required.
   //
   this->mLeadingZeros = MIN (this->mLeadingZeros, this->maximumSignificance () - this->mPrecision);

   this->applyLimits ();
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getPrecision () const
{
   return this->mPrecision;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setMinimum (const double value)
{
   this->mMinimum = LIMIT (value, this->minimumMin, this->maximumMax);

   // Ensure consistant
   //
   this->mMaximum = LIMIT (this->mMaximum, this->mMinimum, this->maximumMax);

   this->setValue (this->getValue ());   // Set value forces min/max limits.
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setMaximum (const double value)
{
   this->mMaximum = LIMIT (value, this->minimumMin, this->maximumMax);

   // Ensure consistant
   //
   this->mMinimum = LIMIT (this->mMinimum, this->minimumMin, this->mMaximum);

   this->setValue (this->getValue ());   // Set value forces min/max limits.
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getMaximum () const
{
   return this->mMaximum;
}


//------------------------------------------------------------------------------
//
void QNumericEdit::setRadix (const QEFixedPointRadix::Radicies value)
{
   if (this->fpr.getRadix () != value) {
      this->fpr.setRadix (value);

      this->applyLimits ();
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Radicies QNumericEdit::getRadix () const
{
   return this->fpr.getRadix ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setSeparator (const QEFixedPointRadix::Separators value)
{
   if (this->fpr.getSeparator () != value) {
      this->fpr.setSeparator (value);
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Separators QNumericEdit::getSeparator () const
{
   return this->fpr.getSeparator ();
}


//------------------------------------------------------------------------------
//
void QNumericEdit::setValue (const double value)
{
   double constrainedValue;

   constrainedValue = LIMIT (value, this->mMinimum, this->mMaximum);

   // If value the same then nothing to do, no signal to emit. This is the
   // behaviour of Qt own combo box, spin edit etc. We try to be consistant.
   //
   if (this->mValue != constrainedValue) {
      this->mValue = constrainedValue;
      emit valueChanged (this->mValue);
      emit valueChanged (int (this->mValue));   // range check?
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getValue () const
{
   return this->mValue;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setValue (const int value) {
   this->setValue (double (value));
}

// end
