/*  QEPvLoadSaveUtilities.cpp
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
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <stdlib.h>

#include <QDebug>
#include <qdom.h>
#include <QFile>
#include <QESettings.h>
#include <QStringList>
#include <QVariant>
#include <QVariantList>

#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"

#include "QEPvLoadSaveUtilities.h"

#define DEBUG qDebug() << "QEPvLoadSaveUtilities::" << __FUNCTION__ << ":" << __LINE__

static const int maxDepth        = 10;
static const QString namePrefix  = "*NAME";
static const QString groupPrefix = "*GROUP";
static const QString arrayPrefix = "*ARRAY";

// Special none values.
//
static const QVariant nilValue (QVariant::Invalid);

// XML tag/attribute names
//
static const QString fileTagName      = "QEPvLoadSave";
static const QString groupTagName     = "Group";
static const QString pvTagName        = "PV";       // scaler PV tag
static const QString arrayTagName     = "Array";
static const QString elementTagName   = "Element";

static const QString indexAttribute   = "Index";
static const QString nameAttribute    = "Name";
static const QString typeAttribute    = "Type";
static const QString valueAttribute   = "Value";
static const QString versionAttribute = "Version";
static const QString numberAttribute  = "Number";

//------------------------------------------------------------------------------
//
QVariant QEPvLoadSaveUtilities::readArray (QESettings* settings,
                                           const QString& arrayName,
                                           QString& pvName)
{
    QVariantList result;

    pvName = "SR00TDB01";
    result << 1 << 2 << arrayName << QVariant ( (bool) (settings == NULL));

    return result;
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readSection (QESettings* settings,
                                                      const QString& groupName,
                                                      QEPvLoadSaveItem* parent,
                                                      const int level)
{
   QEPvLoadSaveItem* result = NULL;
   QStringList theKeys;
   int j;
   bool isRenamed;
   QString variable;
   QString key;
   QString newName;
   QString sectionName;
   QVariant value;
   QString pvName;

   if (!settings) {
      DEBUG << "bad input";
      return result;
   }

   if (level >= maxDepth) {
      DEBUG << "Nesting too deep (" << level << "), group: " << groupName;
      return result;
   }

   result = new QEPvLoadSaveItem (groupName, false, nilValue, parent);

   isRenamed = false;
   theKeys = settings->groupKeys (groupName);
   for (j = 0; j < theKeys.count(); j++) {
      variable = theKeys.value (j);
      key = groupName + "/" + variable;

//      DEBUG << j << variable << " full" << key;

      if (variable.startsWith ("#")) continue;   // is a comment";

      if (variable.startsWith (namePrefix, Qt::CaseInsensitive)) {

         if (level == 1) {
            DEBUG << "An attempt to rename the root node section ignored";
            continue;
         }

         if (isRenamed) {
            DEBUG << "An attempt to rename an already renamed section ignored - first in, best dressed.";
            continue;
         }

         newName = settings->getString (key, "");
         if (newName.isEmpty()) {
            DEBUG << "An attempt to rename to empty name ignored";
            continue;
         }

         result->setNodeName (newName);
         isRenamed = true;
         continue;
      }

      // Is it a group?
      //
      if (variable.startsWith (groupPrefix, Qt::CaseInsensitive)) {
         // Valid group entry test??

         sectionName = settings->getString (key, "");

         if (sectionName.isEmpty ()) {
            DEBUG << "Unspecified group section name";
            continue;
         }

         QEPvLoadSaveUtilities::readSection (settings, sectionName, result, level + 1);
         continue;
      }

      // Is it an extented array?
      //
      if (variable.startsWith (arrayPrefix, Qt::CaseInsensitive)) {
         // Extented Array PV get ther own section.
         // Line length limits imposed by original program.
         //
         sectionName = settings->getString (key, "");

         if (sectionName.isEmpty ()) {
            DEBUG << "Unspecified array section name";
            continue;
         }

         value = QEPvLoadSaveUtilities::readArray (settings, sectionName, pvName);\

         if (pvName.isEmpty ()) {
            DEBUG << "Unspecified array PV name in section " << groupName <<  variable;
            continue;
         }

         new QEPvLoadSaveItem (pvName, true, value, result);

         continue;
      }

      // Assume just a regular PV.
      // TBD: Short array format
      //
      value = settings->getValue (key, nilValue);
      new QEPvLoadSaveItem (variable, true, value, result);

   }

   return result;
}


//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readPcfTree (const QString& filename)
{
   QEPvLoadSaveItem* result = NULL;
   QESettings* settings = NULL;

   settings = new QESettings (filename.trimmed ());
   if (settings) {
      result = QEPvLoadSaveUtilities::readSection (settings, "ROOT", result, 1);
      delete settings;
   }
   return result;
}


//------------------------------------------------------------------------------
//
QVariant QEPvLoadSaveUtilities::convert (const QString& dataType, const QString& valueImage)
{
   QVariant result = nilValue;

   if (dataType == "string") {
      result = QVariant (valueImage);

   } else if (dataType == "int") {
      int v;
      bool okay;

      v = valueImage.toInt (&okay);
      if (okay) {
         result = QVariant (v);
      } else {
         qWarning () << __FUNCTION__ << " ignoring invalid integer: " << valueImage;
      }

   } else if (dataType == "float") {
      double v;
      bool okay;

      v = valueImage.toDouble (&okay);
      if (okay) {
         result = QVariant (v);
      } else {
         qWarning () << __FUNCTION__ << " ignoring invalid float: " << valueImage;
      }

   } else {
      qWarning () << __FUNCTION__ << " ignoring unexpected data type: " << dataType;
   }

   return result;
}

//------------------------------------------------------------------------------
// A scaler PV could be defined as an array of one element, but this form
// provides a syntactical short cut for scaler values which are typically
// the most common in use.
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readXmlScalerPv (const QDomElement pvElement,
                                                          QEPvLoadSaveItem* parent)
{
   QEPvLoadSaveItem* result = NULL;
   QVariant value (QVariant::Invalid);

   QString pvName = pvElement.attribute (nameAttribute, "");
   QString dataType = pvElement.attribute (typeAttribute, "string");
   QString valueImage = pvElement.attribute (valueAttribute, "");

   if (pvName.isEmpty() ) {
      qWarning () << __FUNCTION__ << " ignoring null PV name";
      return result;
   }

   value = QEPvLoadSaveUtilities::convert (dataType, valueImage);
   result = new QEPvLoadSaveItem (pvName, true, value, parent);
   return result;
}


//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readXmlArrayPv (const QDomElement pvElement,
                                                         QEPvLoadSaveItem* parent)
{
   QEPvLoadSaveItem* result = NULL;
   QVariantList arrayValue;

   QString pvName = pvElement.attribute (nameAttribute);
   QString dataType = pvElement.attribute (typeAttribute, "string");
   QString elementCountImage = pvElement.attribute (numberAttribute, "1");

   if (pvName.isEmpty() ) {
      qWarning () << __FUNCTION__ << " ignoring null PV name";
      return result;
   }

   if ((dataType != "string") && (dataType != "int")&&  (dataType != "float")) {
      qWarning () << __FUNCTION__ << pvName << " ignoring unexpected data type: " << dataType;
      return result;
   }

   int elementCount = elementCountImage.toInt (NULL);

   // Initialise array with nil values.
   //
   for (int j = 0; j < elementCount; j++) {
      arrayValue << nilValue;
   }

   // Look for array values.
   //
   QDomElement itemElement = pvElement.firstChildElement (elementTagName);
   while (!itemElement.isNull ()) {
      bool okay;
      int index = itemElement.attribute (indexAttribute, "-1").toInt (&okay);
      if (okay && index >= 0 && index < elementCount) {
         QString valueImage = itemElement.attribute (valueAttribute, "");
         QVariant value = QEPvLoadSaveUtilities::convert (dataType, valueImage);

         arrayValue.replace (index, value);

      } else {
         qWarning () << __FUNCTION__ << " ignoring unexpected index " << index;
      }
      itemElement = itemElement.nextSiblingElement (elementTagName);
   }

   result = new QEPvLoadSaveItem (pvName, true, arrayValue, parent);
   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::readXmlGroup (const QDomElement groupElement,
                                          QEPvLoadSaveItem* parent,
                                          const int level)
{

   if (groupElement.isNull ()) {
      qWarning () << __FUNCTION__ << " null configElement, level => " << level;
      return;
   }

   // Parse XML using Qt's Document Object Model.
   // We look for Group and PV tags.
   //
   QDomElement itemElement = groupElement.firstChildElement ("");
   while (!itemElement.isNull ())   {

      QString tagName = itemElement.tagName ();

      if (tagName == groupTagName) {
         QString groupName = itemElement.attribute (nameAttribute);
         QEPvLoadSaveItem* group = new QEPvLoadSaveItem (groupName, false, nilValue, parent);

         QEPvLoadSaveUtilities::readXmlGroup (itemElement, group, level + 1);

      } else if  (tagName == pvTagName) {
         QEPvLoadSaveUtilities::readXmlScalerPv (itemElement, parent);

      } else if  (tagName == arrayTagName) {
         QEPvLoadSaveUtilities::readXmlArrayPv (itemElement, parent);

      } else {
         qWarning () << __FUNCTION__ << " ignoring unexpected tag " << tagName;
      }

      itemElement = itemElement.nextSiblingElement ("");
   }
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readXmlTree (const QString& filename)
{
   QEPvLoadSaveItem* result = NULL;

   if (filename.isEmpty()) {
      qWarning () << __FUNCTION__ << " null file filename";
      return result;
   }

   QFile file (filename);
   if (!file.open (QIODevice::ReadOnly)) {
      qWarning () << __FUNCTION__ << filename  << " file open (read) failed";
      return result;
   }

   QDomDocument doc;
   QString errorText;
   int errorLine;
   int errorCol;

   if (!doc.setContent (&file, &errorText, &errorLine, &errorCol)) {
      qWarning () << QString ("%1:%2:%3").arg (filename).arg (errorLine).arg (errorCol)
                  << " set content failed " << errorText;
      file.close ();
      return result;
   }

   QDomElement docElem = doc.documentElement ();

   // The file has been read - we can now close it.
   //
   file.close ();

   // Examine top level tag name - is this the tag we expect.
   //
   if (docElem.tagName () != fileTagName) {
      qWarning () << filename  << " unexpected tag name " << docElem.tagName ();
      return result;
   }

   QString versionImage = docElem.attribute (versionAttribute).trimmed ();
   bool versionOkay;
   int version = versionImage.toInt (&versionOkay);

   if (!versionImage.isEmpty()) {
      // A version has been specified - we must ensure it is sensible.
      //
      if (!versionOkay) {
         qWarning () << filename  << " invalid version string " << versionImage << " (integer expected)";
         return result;
      }

   } else {
      // no version - go with current version.
      //
      version = 1;
   }

   if (version != 1) {
      qWarning () << filename  << " unexpected version specified " << versionImage << " (out of range)";
      return result;
   }


   // Create the root item.
   //
   result = new QEPvLoadSaveItem ("ROOT", false, nilValue, NULL);

   // Parse XML using Qt's Document Object Model.
   //
   QEPvLoadSaveUtilities::readXmlGroup (docElem, result, 1);

   return result;
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readTree (const QString& filename)
{
   QEPvLoadSaveItem* result = NULL;

   if (filename.trimmed ().endsWith (".pcf")) {
      result = QEPvLoadSaveUtilities::readPcfTree (filename);

   } else if (filename.trimmed ().endsWith (".xml")) {
      result =  QEPvLoadSaveUtilities::readXmlTree (filename);

   }

   return result;
}


//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlScalerPv (const QEPvLoadSaveItem* item,
                                              QDomElement& pvElement)
{
   if (!item) {
      return;
   }

   QVariant value = item->getNodeValue ();

   pvElement.setAttribute (nameAttribute, item->getNodeName ());

   switch (value.type ()) {
      case QVariant::Int:
         pvElement.setAttribute (typeAttribute, "int");
         break;

      case QVariant::Double:
         pvElement.setAttribute (typeAttribute, "float");
         break;

      case QVariant::String:
         pvElement.setAttribute (typeAttribute, "string");
         break;

      default:
         // null.
         break;
   }

   pvElement.setAttribute (valueAttribute, value.toString ());
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlArrayPv (const QEPvLoadSaveItem* item,
                                             QDomDocument& doc,
                                             QDomElement& arrayElement)
{
   if (!item) {
      return;
   }

   QVariantList valueList = item->getNodeValue ().toList ();
   QVariant value = valueList.value (0);
   int n = valueList.size ();

   arrayElement.setAttribute (nameAttribute, item->getNodeName ());
   arrayElement.setAttribute (numberAttribute, QString ("%1").arg (n));

   // Use first element to figure out type - they should all be the same.
   //
   switch (value.type ()) {
      case QVariant::Int:
         arrayElement.setAttribute (typeAttribute, "int");
         break;

      case QVariant::Double:
         arrayElement.setAttribute (typeAttribute, "float");
         break;

      case QVariant::String:
         arrayElement.setAttribute (typeAttribute, "string");
         break;

      default:
         // null.
         break;
   }

   for (int j = 0; j < n; j++) {
      QDomElement itemElement = doc.createElement (elementTagName);
      arrayElement.appendChild (itemElement);
      itemElement.setAttribute (indexAttribute, QString ("%1").arg (j));

      value = valueList.value (j);
      itemElement.setAttribute (valueAttribute, value.toString ());
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlGroup (const QEPvLoadSaveItem* group,
                                           QDomDocument& doc,
                                           QDomElement& groupElement)
{
   int n;
   int j;

   // Sainity check.
   //
   if (!group || group->getIsPV ()) {
      return;
   }

   n = group->childCount ();
   for (j = 0; j < n; j++) {
      QEPvLoadSaveItem* child = group->getChild (j);
      QDomElement childElement;

      if (child->getIsGroup ()) {
         // This is a group node.
         //
         childElement = doc.createElement (groupTagName);
         groupElement.appendChild (childElement);
         childElement.setAttribute (nameAttribute, child->getNodeName());
         QEPvLoadSaveUtilities::writeXmlGroup (child, doc, childElement);

      } else {
         // This is a PV node. Scaler or Array?
         //
         if (child->getElementCount () > 1) {
            childElement = doc.createElement (arrayTagName);
            groupElement.appendChild (childElement);
            QEPvLoadSaveUtilities::writeXmlArrayPv (child, doc, childElement);
         } else {
            childElement = doc.createElement (pvTagName);
            groupElement.appendChild (childElement);
            QEPvLoadSaveUtilities::writeXmlScalerPv (child, childElement);
         }
      }
   }
}


//------------------------------------------------------------------------------
//
bool QEPvLoadSaveUtilities::writeXmlTree (const QString& filename, const QEPvLoadSaveItem* root)
{
   if (filename.isEmpty () || !root) {
      qWarning () << __FUNCTION__ << "null filename and/or root node specified";
      return false;
   }

   QDomDocument doc;
   QDomElement docElem;

   doc.clear ();
   docElem = doc.createElement (fileTagName);
   docElem.setAttribute (versionAttribute, 1);

   // Add the root to the document
   //
   doc.appendChild (docElem);

   QFile file (filename);
   if (!file.open (QIODevice::WriteOnly)) {
      qDebug() << "Could not save configuration " << filename;
      return false;
   }

   QEPvLoadSaveUtilities::writeXmlGroup (root, doc, docElem);

   QTextStream ts (&file);
   ts << doc.toString (2);  // setting the indent to 2 is purely cosmetic
   file.close ();

   return true;
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSaveUtilities::writePcfTree (const QString& filename, const QEPvLoadSaveItem* root)
{
   if (filename.isEmpty () || !root) {
      qWarning () << __FUNCTION__ << "null filename and/or root node specified";
      return false;
   }

   return false;   // Are we even going to support this functionality??
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSaveUtilities::writeTree (const QString& filename, const QEPvLoadSaveItem* root)
{
   bool result = false;

   if (filename.trimmed ().endsWith (".pcf")) {
      result = QEPvLoadSaveUtilities::writePcfTree (filename, root);

   } else if (filename.trimmed ().endsWith (".xml")) {
      result =  QEPvLoadSaveUtilities::writeXmlTree (filename, root);

   }

   return result;
}

// end
