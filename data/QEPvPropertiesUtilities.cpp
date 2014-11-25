/*  QEPvPropertiesUtilities.cpp
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
 *  Copyright (c) 2012
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <stdlib.h>

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "QEPvPropertiesUtilities.h"

#define DEBUG qDebug() << "QEPvPropertiesUtilities::" << __FUNCTION__ << ":" << __LINE__


//==============================================================================
//
QERecordSpec::QERecordSpec (const QString recordTypeIn)
{
   this->recordType = recordTypeIn;
   this->clear ();
}

//------------------------------------------------------------------------------
//
QString QERecordSpec::getRecordType ()
{
   return this->recordType;
}

//------------------------------------------------------------------------------
//
QString QERecordSpec::getFieldName (const int index)
{
   if ((0 <= index) && (index < size ())) {
      return this->at (index);
   } else {
      return "";
   }
}


//==============================================================================
//
QERecordSpecList::QERecordSpecList ()
{
    // place holder:
}


//------------------------------------------------------------------------------
//
int QERecordSpecList::findSlot (const QString recordType) {
   int result = -1;
   QERecordSpec * checkSpec;

   for (int j = 0; j < this->size (); j++) {
      checkSpec = this->at (j);
      if (checkSpec->getRecordType () == recordType) {
         // Found it ;-)
         //
         result = j;
         break;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QERecordSpec * QERecordSpecList::find (const QString recordType) {
   QERecordSpec *result = NULL;
   int slot;

   slot = this->findSlot (recordType);
   if (slot >= 0) {
      result = this->at (slot);
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QERecordSpecList::appendOrReplace (QERecordSpec *newRecordSpec)
{
   int slot;

   if (newRecordSpec) {
      // Do we already have a record spec for this record type.
      //
      slot = this->findSlot (newRecordSpec->getRecordType());
      if (slot >= 0) {
         // Yes ...
         QERecordSpec *previousRecordSpec;

         previousRecordSpec = this->at (slot);
         this->replace (slot, newRecordSpec);
         delete previousRecordSpec;

      } else {
         // No - just append it.
         //
         this->append (newRecordSpec);
      }
   }
}

//------------------------------------------------------------------------------
//
bool QERecordSpecList::processRecordSpecFile (const QString& filename)
{
   if (filename == "") {
      return false;
   }

   QFile record_field_file (filename);

   if (!record_field_file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      return false;
   }

   QTextStream source (&record_field_file);
   QERecordSpec *recordSpec;
   QString recordType;
   unsigned int lineno;

   recordSpec = NULL;
   lineno = 0;
   while (!source.atEnd()) {
      QString line = source.readLine ().trimmed ();

      lineno++;

      // Skip empty line and comment lines.
      //
      if (line.length () == 0) continue;
      if (line.left (1) == "#") continue;

      // record types designated by: <<name>>
      //
      if ((line.left (2) == "<<") && (line.right (2) == ">>")) {

         recordType = line.mid (2, line.length() - 4).trimmed ();
         recordSpec = new QERecordSpec (recordType);
         this->appendOrReplace (recordSpec);

      } else {
         // Just a regular field.
         //
         if (recordSpec) {
            recordSpec->append (line);
         } else {
            DEBUG << "field occured before first record type: " << filename << lineno << line;
         }
      }
   }

   record_field_file.close ();
   return true;
}


//==============================================================================
//
QString QERecordFieldName::recordName (const QString & pvName)
{
   QString result;
   int dot_posn;

   result = pvName;
   dot_posn = result.indexOf (".", 0);
   if (dot_posn >= 0) {
      result.truncate (dot_posn);
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::fieldName (const QString & pvName)
{
   QString result = "VAL";
   int dot_posn;
   int fs;

   dot_posn = pvName.indexOf (".", 0);
   if (dot_posn >= 0) {
      fs = pvName.length() - dot_posn - 1;
      if (fs > 0) {
         result = pvName.right (fs);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::fieldPvName (const QString & pvName, const QString & field)
{
    return recordName (pvName) + "." + field;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::rtypePvName (const QString & pvName)
{
   return recordName (pvName) + ".RTYP";
}

//------------------------------------------------------------------------------
//
bool QERecordFieldName::pvNameIsValid (const QString & pvName)
{
   QChar c;
   bool result;
   int j;
   int colonCount;
   int dotCount;

   if (pvName.length () == 0) {
      return false;
   }

   // Must start with a letter.
   //
   c = pvName [0];
   if ((!c.isUpper ()) && (!c.isLower())) {
      return false;
   }

   // Hypothosize all okay
   //
   result = true;
   colonCount = 0;
   dotCount = 0;
   for (j = 1; j < pvName.length (); j++) {
      c = pvName [j];

      if (c.isUpper() || c.isLower() || c.isDigit() ||
         (c == '_')   || (c == '-')) {
         // is good
      } else if (c == ':') {
         colonCount++;
      } else if ( c== '.' ) {
         dotCount++;
      } else {
         // Invalid character
         result = false;
         break;
      }
   }

   // Expected format is CCCC:SSSS[.FFFF]
   // However many beamline PVs do not strictly follow the naming
   // convension, so allow 1 or 2 colons.
   //
   if ((colonCount < 1) || (colonCount > 2) || (dotCount > 1)) {
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
#define SUFFIX_NUM   13

static const char * suffixList [SUFFIX_NUM] = {
   " NPP", " PP", " CA", " CP", " CPP", " NMS", " MS",
   // truncated versions
   " N", " NP", " P", " C", " NM" , " M"
};

bool QERecordFieldName::extractPvName (const QString & item, QString & pvName)
{
   int i;
   int j;
   bool status;

   pvName = item.trimmed ();

   // Strip off standard suffix.
   //
   // Could be a little smarter, but as only a few suffix, this is okay.
   //
   for (i = 0; i < SUFFIX_NUM; i++) {
      for (j = 0; j < SUFFIX_NUM; j++) {

         if (pvName.endsWith (suffixList [j])) {
            int at = pvName.length () - strlen (suffixList [j]);
            pvName.truncate (at);
         }
      }
   }

   // Validate what is left.
   //
   status = QERecordFieldName::pvNameIsValid (pvName);
   if (!status) {
      pvName.clear ();
   }
   return status;
}

// end
