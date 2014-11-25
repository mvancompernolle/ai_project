/*  QEPvLoadSaveItem.cpp
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
#include <QFrame>
#include <QModelIndex>
#include <QPushButton>
#include <QWidget>

#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"

#define DEBUG  qDebug () << "QEPvLoadSaveItem::" << __FUNCTION__ << ":" << __LINE__

//=============================================================================
//
QEPvLoadSaveItem::QEPvLoadSaveItem (const QString & nodeNameIn,
                                    const bool isPVIn,
                                    const QVariant& valueIn,
                                    QEPvLoadSaveItem *parentIn) : QObject (NULL)
{
   this->parentItem = parentIn;
   this->nodeName = nodeNameIn;
   this->isPV = isPVIn;
   this->value = valueIn;

   // Add to parent's own/specific QEPvLoadSaveItem child list.
   //
   if (this->parentItem) {
      this->parentItem->appendChild (this);
   }

   this->isConnected = false;
   this->qca = NULL;
   this->archiveAccess = NULL;

   // Is this a leaf node, i.e. a PV ?
   // Note: we do not enforce rule that leaf and only leaf items are PVs.
   //
   if (isPV) {
      // Interpret node name as a PV name.
      //
      this->qca = new qcaobject::QCaObject (this->nodeName, this, 0);

      // QCaObject does not do this automatically. Maybe it should?.
       //
      qca->setParent (this);

      this->connect (this->qca, SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int& )),
                     this,      SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&  )));

      this->connect (this->qca, SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo& , QCaDateTime&, const unsigned int&  )),
                     this,      SLOT   (dataChanged (const QVariant&, QCaAlarmInfo& , QCaDateTime&, const unsigned int&  )));


      // Allow item to retrive archive data values.
      //
      this->archiveAccess = new QEArchiveAccess (this);

      this->connect (this->archiveAccess, SIGNAL (setArchiveData ( const QObject*, const bool, const QCaDataPointList&)),
                     this,                SLOT   (setArchiveData ( const QObject*, const bool, const QCaDataPointList&)));

   }
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem::~QEPvLoadSaveItem ()
{
   // Remove from parent's own/specific QEPvLoadSaveItem child list.
   //
   qDeleteAll (this->childItems);
}


//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem *QEPvLoadSaveItem::getChild (int position) const
{
   return this->childItems.value (position, NULL);
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem * QEPvLoadSaveItem::getParent () const
{
   return this->parentItem;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::childCount () const
{
   return this->childItems.count ();
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::columnCount () const
{
   // Number rcols fixed at 1.
   // Could split single "Name = Value" into pair ("Name, "Value")
   return 1;
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveItem::getData (int column) const
{
   QVariant result;

   if (column == 0) {
      QString valueImage = this->nodeName;

      if (this->isPV) {
         valueImage.append (" = ");

         if (this->value.type() == QVariant::List) {
            QVariantList vl = this->value.toList ();
            valueImage.append (QString (" << %1 element array >>").arg (vl.size ()));
         } else {
            valueImage.append (this->value.toString ());
         }
      }
      result = valueImage;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::childPosition () const
{
   if (this->parentItem) {
      return this->parentItem->childItems.indexOf (const_cast<QEPvLoadSaveItem*>(this));
   }
   return 0;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveItem::insertChild (int position, QEPvLoadSaveItem* child)
{
   if (position < 0 || position > childItems.size ()) {
       return false;
   }

   if (!child) {
      return false;
   }

   child->parentItem = this;
   this->childItems.insert (position, child);

   return true;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveItem::removeChildren(int position, int count)
{
   if (position < 0 || position + count > childItems.size ()) {
       return false;
   }

   for (int row = 0; row < count; ++row) {
       delete childItems.takeAt (position);
   }

   return true;
}


//=============================================================================
// Specific non-example functions.
//=============================================================================
//
QEPvLoadSaveItem* QEPvLoadSaveItem::getNamedChild (const QString& searchName)
{
   QEPvLoadSaveItem* result = NULL;
   int r;

   for (r = 0; r < this->childCount(); r++) {
      QEPvLoadSaveItem* child = this->getChild (r);
      if (child->getNodeName () == searchName) {
         // found it.
         //
         result = child;
         break;
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveItem::clone (const bool doDeep, QEPvLoadSaveItem* parent)
{
   QEPvLoadSaveItem* result = NULL;

   result = new QEPvLoadSaveItem (this->getNodeName (), this->getIsPV (), this->getNodeValue (), parent);

   if (doDeep && this->getIsGroup ()) {
       // Now clone each child.
      //
       for (int j = 0; j < this->childItems.count(); j++) {
           QEPvLoadSaveItem* theChild = this->getChild (j);
           theChild->clone (doDeep, result);
       }
   }

   return result;
}

//-----------------------------------------------------------------------------
//
QStringList QEPvLoadSaveItem::getNodePath ()
{
   QStringList result;
   QEPvLoadSaveItem* parentNode;
   QEPvLoadSaveItem* grandParentNode;

   parentNode = this->getParent ();
   if (parentNode) {
      // This object has a parent.
      //
      // Want to exclude the 'internal' tree core node, i.e. start from user root.
      //
      grandParentNode = parentNode->getParent ();
      if (grandParentNode) {
         result = parentNode->getNodePath ();
         result << parentNode->getNodeName ();
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::setNodeName (const QString& nodeNameIn)
{
   this->nodeName = nodeNameIn;
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveItem::getNodeName () const
{
   return this->nodeName;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::setNodeValue (const QVariant& valueIn)
{
   this->value = valueIn;
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveItem::getNodeValue () const
{
   return this->value;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::getElementCount () const
{
   int result;

   if (this->value.type() == QVariant::List) {
      QVariantList vl = this->value.toList ();
      result = vl.size ();
   } else {
      result = 1;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::actionConnect (QObject* actionCompleteObject, const char* actionCompleteSlot)
{
   if (this->isPV) {
      QObject::connect (this, SIGNAL (reportActionComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds, bool)),
                        actionCompleteObject, actionCompleteSlot);
   } else {
      for (int j = 0; j < this->childItems.count(); j++) {
          QEPvLoadSaveItem* item = this->getChild (j);
          if (item) item->actionConnect (actionCompleteObject, actionCompleteSlot);
      }
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::extractPVData ()
{
    if (this->isPV) {
       if (this->qca && isConnected)  {
          bool status;
          status = qca->singleShotRead ();
          if (!status) {
             emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, false);
          }
       } else {
          emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, false);
       }

    } else {
        for (int j = 0; j < this->childItems.count(); j++) {
            QEPvLoadSaveItem* item = this->getChild (j);
            if (item) item->extractPVData ();
        }
    }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::applyPVData ()
{
   if (this->isPV) {
      if (this->qca && isConnected)  {
         bool status;
         status = qca->writeData (this->value);
         emit this->reportActionComplete (this, QEPvLoadSaveCommon::Apply, status);
      } else {
         emit this->reportActionComplete (this, QEPvLoadSaveCommon::Apply, false);
      }

   } else {
       for (int j = 0; j < this->childItems.count(); j++) {
           QEPvLoadSaveItem* item = this->getChild (j);
           if (item) item->applyPVData ();
       }
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::readArchiveData (const QCaDateTime& dateTime)
{
   if (this->isPV) {
      if (this->archiveAccess) {
         this->archiveAccess->readArchive (this, this->getNodeName (),
                                           dateTime, dateTime, 1,
                                           QEArchiveInterface::Linear, 0);
      } else {
         //
      }
   } else {
       for (int j = 0; j < this->childItems.count(); j++) {
           QEPvLoadSaveItem* item = this->getChild (j);
           if (item) item->readArchiveData (dateTime);
       }
   }
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::leafCount ()
{
   int result;

   if (this->isPV) {
      result = 1;
   } else {
       result = 0;
       for (int j = 0; j < this->childItems.count(); j++) {
           QEPvLoadSaveItem* item = this->getChild (j);
           if (item) result += item->leafCount ();
       }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&)
{
   this->isConnected = connectionInfo.isChannelConnected ();
}

//-----------------------------------------------------------------------------
//
void  QEPvLoadSaveItem::dataChanged (const QVariant& valueIn, QCaAlarmInfo& alarmInfoIn, QCaDateTime&, const unsigned int& )
{
   //
   this->value = valueIn;
   this->alarmInfo = alarmInfoIn;
   emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, true);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::setArchiveData (const QObject*, const bool okay, const QCaDataPointList & dataPointList)
{
   if (okay && dataPointList.count() > 0) {
      QCaDataPoint item = dataPointList.value (0);

      this->value = QVariant (item.value);
      this->alarmInfo = item.alarm;

      emit this->reportActionComplete (this, QEPvLoadSaveCommon::ReadArchive, true);
   } else {
      emit this->reportActionComplete (this, QEPvLoadSaveCommon::ReadArchive, false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::appendChild (QEPvLoadSaveItem *child)
{
   // Check for duplicates???
   //
   this->childItems.append (child);

   // Ensure consistency
   //
   child->parentItem = this;
}

// end
