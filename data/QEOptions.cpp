/*  QEOptions.cpp $
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

#include <QCoreApplication>
#include <QDebug>

#include "QEOptions.h"

const QChar NullLetter = QChar ((ushort) 0xDEAD);    // A bit arbitary
const QString NullString =                           // Also a bit arbitary
      QString (NullLetter).append (NullLetter).append (NullLetter).append (NullLetter);

#define NOT_A_NUMBER  "__not_a_number__"


//--------------------------------------------------------------------------------------
//
QEOptions::QEOptions ()
{
   this->args = QCoreApplication::arguments ();

   // Remove the program name from the set of arguments.
   this->args.removeFirst ();

   this->setUpCommon ();
}

//--------------------------------------------------------------------------------------
//
QEOptions::QEOptions (const QStringList & argsIn)
{
   this->args = argsIn;
   this->setUpCommon ();
}

//--------------------------------------------------------------------------------------
//
void QEOptions::setUpCommon ()
{
   int j;

   // Find first parameter, i.e. non option argument.
   //
   // Example: Condider options  "--tom"  "dick"  "harry"
   //
   // "--tom" is an option, and "dick" is deemed the first true parameter,
   // so parameterOffset is set to 1.
   //
   // However if user calls getString ("tom"), then we know "dick" is the value
   // associated with "tom", so parameterOffset as set to 2, and "harry" is the
   // first parameter.
   //
   this->parameterOffset = 0;
   for (j = this->args.count() - 1; j >= 0; j--) {
      QString arg = this->args.value (j);
      if (arg.startsWith("-")) {
         // We have found the last option.
         //
         this->parameterOffset = j + 1;
         break;
      }
   }
}

//--------------------------------------------------------------------------------------
//
QEOptions::~QEOptions ()
{
   this->args.clear ();
}

//--------------------------------------------------------------------------------------
//
void QEOptions::registerOptionArgument (int p)
{
   if (this->parameterOffset < (p + 1)) {
      this->parameterOffset = (p + 1);
   }
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getParameter (const int i)
{
   if (i >= 0) {
      return this->args.value (this->parameterOffset + i, "");
   }
   return "";
}


//======================================================================================
//
bool QEOptions::isSpecified (const QString& option, const QChar letter)
{
   QString stringVal;
   stringVal = this->getString (option, letter, NullString);
   return (stringVal != NullString);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::isSpecified (const QString& option)
{
   return this->isSpecified (option, NullLetter);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::isSpecified (const QChar letter)
{
   return this->isSpecified (NullString, letter);
}


//======================================================================================
//
bool QEOptions::getBool (const QString& option, const QChar letter)
{
   const QString lookForA = QString ("--") + option;
   const QString lookForB = QString ("-") + letter;
   bool result = false;
   int j;

   for (j = 0; j < this->args.count (); j++ ) {
      QString arg = this->args.value (j);

      // Is arg == --option ?
      //
      if (arg == lookForA || arg == lookForB) {
         result = true;
         break;
      }
   }

   return result;
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::getBool (const QString& option)
{
   return this->getBool (option, NullLetter);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::getBool  (const QChar letter)
{
   return this->getBool (NullString, letter);
}


//======================================================================================
//
QString QEOptions::getString (const QString& option, const QChar letter, const QString& defaultValue)
{
   const QString lookForA = QString ("--") + option;
   const QString lookForB = QString ("-") + letter;
   const QString lookForEqA = lookForA + QString ("=");
   const QString lookForEqB = lookForB + QString ("=");
   QString result = defaultValue;
   int j;

   for (j = 0; j < this->args.count (); j++ ) {
      QString arg = this->args.value (j);

      // Is arg == --option=something ?
      //
      if (arg.startsWith (lookForEqA)) {
         // return something
         //
         result = arg.remove(0, lookForEqA.length ());
         break;
      }

      // Is arg == -l=something ?
      //
      if (arg.startsWith (lookForEqB)) {
         // return something
         //
         result = arg.remove(0, lookForEqB.length ());
         break;
      }

      // Is arg == --option  or -l?
      //
      if (arg == lookForA || arg == lookForB) {
         // return next arg.
         //
         this->registerOptionArgument (j + 1);
         result = this->args.value (j + 1, defaultValue);
         break;
      }
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getString (const QString& option, const QString& defaultValue)
{
   return this->getString (option, NullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getString (const QChar letter, const QString& defaultValue)
{
   return this->getString (NullString, letter, defaultValue);
}


//======================================================================================
//
int QEOptions::getInt (const QString& option, const QChar letter, const int defaultValue)
{
   int result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (option, letter, NOT_A_NUMBER);
   if (sval != NOT_A_NUMBER) {
      result = sval.toInt (&okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
int QEOptions::getInt (const QString& option, const int defaultValue)
{
   return this->getInt (option, NullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
int QEOptions::getInt (const QChar letter, const int defaultValue)
{
   return this->getInt (NullString, letter, defaultValue);
}


//======================================================================================
//
double QEOptions::getFloat  (const QString& option, const QChar letter, const double  defaultValue)
{
   double result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (option, letter, NOT_A_NUMBER);

   if (sval != NOT_A_NUMBER) {
      result = sval.toDouble (&okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
double QEOptions::getFloat (const QString& option, const double defaultValue)
{
   return this->getFloat (option, NullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
double QEOptions::getFloat (const QChar letter, const double defaultValue)
{
   return this->getFloat (NullString, letter, defaultValue);
}

// end
