/*  QETable.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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

#include <QColor>
#include <QDebug>
#include <QECommon.h>
#include <QEFloating.h>
#include <QHeaderView>

#include "QETable.h"

#define DEBUG qDebug () << "QETable" << __FUNCTION__ << __LINE__

#define DEFAULT_CELL_HEIGHT     22
#define NULL_SELECTION          (-1)


//=============================================================================
// DataSets class - manages a single PV
//=============================================================================
//
QETable::DataSets::DataSets ()
{
   this->pvName = "";
   this->isConnected = false;
   this->owner = NULL;
   this->slot = -1;
}

//-----------------------------------------------------------------------------
//
QETable::DataSets::~DataSets ()
{
   // place holder
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::setContext (QETable* ownerIn, int slotIn)
{
   this->owner = ownerIn;
   this->slot = slotIn;
}

//-----------------------------------------------------------------------------
//
bool QETable::DataSets::isInUse () const
{
   // Data set item in use if PV name defined (as opposed to PV actually exists).
   //
   return !this->pvName.isEmpty ();
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::rePopulateTable ()
{
   QTableWidget* table;
   int index;
   int numberElements;
   int currentSize;
   QTableWidgetItem* item;
   double value;
   QString image;
   QColor colour;

   if (!this->isInUse ()) return;        // nothing to see here ... move along ...
   table = this->owner->table;  // alias
   if (!table) return;                   // sainity check

   // Find own row/col index
   //
   index = 0;
   for (int j = 0; j < this->slot; j++) {
      if (this->owner->dataSet [j].isInUse ()) {
         index++;
      }
   }

   // The number of elements used/displayed is the lesser of the number avialable
   // data elements and the overall display maximum.
   //
   numberElements = MIN (this->data.count (), this->owner->getDisplayMaximum ());

   // Ensure table large enough to accomodate all data.
   //
   if (this->owner->isVertical ()) {
      currentSize = table->rowCount ();
      table->setRowCount (MAX (numberElements, currentSize));

      currentSize = table->columnCount ();
      table->setColumnCount (MAX (currentSize, index + 1));
   } else {
      currentSize = table->columnCount ();
      table->setColumnCount (MAX (numberElements, currentSize));

      currentSize = table->rowCount ();
      table->setRowCount (MAX (currentSize, index + 1));
   }

   if (this->owner->getDisplayAlarmState ()) {
      colour = QColor (alarmInfo.getStyleColorName ());
   } else {
      colour = QColor ("#f0f0f0");
   }

   for (int j = 0; j < numberElements; j++) {
      int row = this->owner->isVertical () ? j : index;
      int col = this->owner->isVertical () ? index : j;

      item = table->item (row, col);
      if (!item) {
         // We need to allocate iteem and insert it into the table.
         //
         item = new QTableWidgetItem ();
         table->setItem (row, col, item);
      }

      value = this->data.value (j);
      image = QString (" %1").arg (value);   // no EGU or formatting (yet).
      item->setText (image);
      item->setBackgroundColor (colour);
   }
}


//==============================================================================
// Slot range checking macro function.
// Set defaultValue to nil for void functions.
//
#define SLOT_CHECK(slot, defaultValue) {                           \
   if ((slot < 0) || (slot >= ARRAY_LENGTH (this->dataSet))) {     \
      DEBUG << "slot out of range: " << slot;                      \
      return defaultValue;                                         \
   }                                                               \
}

//=============================================================================
// Constructor with no initialisation
//=============================================================================
//
QETable::QETable (QWidget* parent) : QEAbstractWidget (parent)
{
   // Create internal widget. We always have at least one row and one col.
   //
   this->table = new QTableWidget (1, 1, this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->table->sizePolicy ());
   this->table->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->table);

   // Initialise data set objects.
   // These are declared as array as opposed to being dynamically allocated,
   // so we need need post contruction configuration.
   //
   for (int slot = 0 ; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].setContext (this, slot);
   }

   // Set default property values
   //
   this->displayMaximum = 0x1000;
   this->selection = NULL_SELECTION;
   this->emitSelectionChangeInhibited = false;
   this->emitPvNameSetChangeInhibited = false;
   this->columnWidthMinimum = 80;
   this->orientation = Qt::Vertical;
   this->setNumVariables (ARRAY_LENGTH (this->dataSet));
   this->setMinimumSize (120, 50);
   this->setVariableAsToolTip (true);
   this->setAllowDrop (true);

   this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
   this->table->verticalHeader()->setDefaultSectionSize (DEFAULT_CELL_HEIGHT);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connections to receive variable name property changes
   //
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int slot = 0 ; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QCaVariableNamePropertyManager* vpnm;

      vpnm = &this->dataSet [slot].variableNameManager;
      vpnm->setVariableIndex (slot);   // Use slot as variable index.

      QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                        this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));
   }

   // Set up a connections to receive table signals
   //
   QObject::connect (this->table, SIGNAL (currentCellChanged (int, int, int, int)),
                     this,        SLOT   (currentCellChanged (int, int, int, int)));

}

//---------------------------------------------------------------------------------
//
QSize QETable::sizeHint () const
{
   return QSize (222, 118);
}

//---------------------------------------------------------------------------------
//
void QETable::fontChange (const QFont&)
{
   // We use this overridden function as a trigger to update the internal
   // widget's font. The given parameter (which we don't use) lags by one change,
   // but this->font () is up to date, so we use that.
   //
   if (this->table) {
      this->table->setFont (this->font ());
   }
}

//---------------------------------------------------------------------------------
//
void QETable::resizeEvent (QResizeEvent*)
{
   this->resizeCoulumns ();
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QETable::createQcaItem (unsigned int variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot, NULL);

   qcaobject::QCaObject* result = NULL;
   QString pvName;

   pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();
   result = new QEFloating (pvName, this, &this->floatingFormatting, variableIndex);

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QETable::establishConnection (unsigned int variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot, );

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);

   // Sanity check
   //
   if (!qca) {
      return;
   }

   QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                     this, SLOT  (connectionChanged (QCaConnectionInfo &, const unsigned int &)));

   QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                     this, SLOT  (dataArrayChanged     (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));
}

//------------------------------------------------------------------------------
//
void QETable::activated ()
{
   // This prevents infinite looping in the case of cyclic connections.
   //
   if (!this->emitPvNameSetChangeInhibited) {
      emit this->pvNameSetChanged (this->getPvNameSet ());
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QETable::connectionChanged (QCaConnectionInfo& connectionInfo,
                                 const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot,);

   this->dataSet [slot].isConnected = connectionInfo.isChannelConnected ();
   this->updateToolTipConnection (dataSet [slot].isConnected, variableIndex);
}

//-----------------------------------------------------------------------------
//
void QETable::dataArrayChanged (const QVector<double>& values,
                                QCaAlarmInfo& alarmInfo,
                                QCaDateTime&,
                                const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot,);

   this->dataSet [slot].data = QEFloatingArray (values);
   this->dataSet [slot].alarmInfo = alarmInfo;

   this->dataSet [slot].rePopulateTable ();

   // Signal a database value change to any Link widgets
   //
   emit this->dbValueChanged (values);

   // Don't invoke common alarm handling processing, as we use a PV specifc alarm
   // dinication per col/row. Update the tool tip to reflect current alarm state.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);
}

//---------------------------------------------------------------------------------
//
bool QETable::isVertical () const
{
   return (this->orientation != Qt::Horizontal);
}

//---------------------------------------------------------------------------------
//
void QETable::resizeCoulumns ()
{
   int count;
   int otherStuff;
   int colWidth;

   count = this->table->columnCount ();
   count = MAX (1, count);

   // Allow for side headers and scroll bar.
   //
   otherStuff = table->verticalHeader()->width() + 20;
   colWidth = (this->table->width () - otherStuff) / count;
   colWidth = MAX (this->columnWidthMinimum, colWidth);

   for (int col = 0; col < count; col++) {
      this->table->setColumnWidth (col, colWidth);
   }
}

//---------------------------------------------------------------------------------
//
void QETable::rePopulateTable ()
{
   this->table->setRowCount (1);
   this->table->setColumnCount (1);

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].rePopulateTable ();
   }
   this->resizeCoulumns ();
}

//------------------------------------------------------------------------------
//
void QETable::setNewVariableName (QString variableName,
                                  QString substitutions,
                                  unsigned int variableIndex)
{
   int slot = this->slotOf (variableIndex);
   QString pvName;

   SLOT_CHECK (slot,);

   // Note: essentially calls createQcaItem.
   //
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);

   pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();
   this->dataSet [slot].pvName = pvName;

   // Count number of slots in use.
   //
   int count = 0;
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      if (this->dataSet [slot].isInUse()) {
         count++;
      }
   }

   // Set the col/row count accordingly. Always display at least one col/row.
   //
   count = MAX (1, count);
   if (this->isVertical ()) {
      this->table->setColumnCount (count);
   } else {
      this->table->setRowCount (count);
   }
   this->resizeCoulumns ();

   // This prevents infinite looping in the case of cyclic connections.
   //
   if (!this->emitPvNameSetChangeInhibited) {
      emit this->pvNameSetChanged (this->getPvNameSet ());
   }
}

//---------------------------------------------------------------------------------
// User has clicked on cell or used up/down/left/right key to select cell,
// or we have programtically selected a row/coll.
//
void QETable::currentCellChanged (int currentRow, int currentCol, int, int)
{
   this->selection = (this->isVertical () ? currentRow : currentCol);

   // This prevents infinite looping in the case of cyclic connections.
   //
   if (!this->emitSelectionChangeInhibited) {
      emit this->selectionChanged (this->selection);
   }
}

//------------------------------------------------------------------------------
//
void QETable::setSelection (int selectionIn)
{
   // A negative selection means no selection
   //
   if (selectionIn < 0) selectionIn = NULL_SELECTION;
   if (this->selection != selectionIn) {
      this->selection = selectionIn;

      this->emitSelectionChangeInhibited = true;
      if (selectionIn >= 0) {
         if (this->isVertical ()) {
            this->table->selectRow (selectionIn);
         } else {
            this->table->selectColumn (selectionIn);
         }
      } else {
         this->table->clearSelection ();
      }
      this->emitSelectionChangeInhibited = false;
   }
}

//------------------------------------------------------------------------------
//
int QETable::getSelection () const {
   return this->selection;
}


//------------------------------------------------------------------------------
//
void QETable::setPvNameSet (const QStringList& pvNameSet)
{
   this->emitPvNameSetChangeInhibited = true;

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = pvNameSet.value (slot, "");
      this->setNewVariableName (pvName, "", slot);
   }

   this->emitPvNameSetChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
QStringList QETable::getPvNameSet () const
{
   QStringList result;

   result.clear ();
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = this->getSubstitutedVariableName (slot);
      if (!pvName.isEmpty()) {
         result.append (pvName);
      }
   }

   return result;
}


//==============================================================================
// Properties
// Update variable name etc.
//
void QETable::setVariableName (const int slot, const QString& pvName)
{
   SLOT_CHECK (slot,);
   this->dataSet [slot].variableNameManager.setVariableNameProperty (pvName);
}

//------------------------------------------------------------------------------
//
QString QETable::getVariableName (const int slot) const
{
   SLOT_CHECK (slot, "");
   return this->dataSet [slot].variableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QETable::setSubstitutions (const QString& defaultSubstitutions)
{
   // Use same default subsitutions for all PVs used by this widget.
   //
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].variableNameManager.setSubstitutionsProperty (defaultSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QETable::getSubstitutions () const
{
   // Any one of the PV name managers can provide the subsitutions.
   //
   return this->dataSet [0].variableNameManager.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QETable::setDisplayMaximum (const int displayMaximumIn)
{
   int temp = LIMIT (displayMaximumIn, 1, 0x10000);

   if (this->displayMaximum != temp) {
      this->displayMaximum = temp;
      this->rePopulateTable ();
   }
}

//------------------------------------------------------------------------------
//
int QETable::getDisplayMaximum () const
{
    return this->displayMaximum;
}

//------------------------------------------------------------------------------
//
void QETable::setColumnWidthMinimum (const int minimumColumnWidthIn)
{
   int temp = LIMIT (minimumColumnWidthIn, 20, 320);

   if (this->columnWidthMinimum != temp) {
      this->columnWidthMinimum = temp;
      this->resizeCoulumns ();
   }
}

//------------------------------------------------------------------------------
//
int QETable::getColumnWidthMinimum () const
{
   return this->columnWidthMinimum;
}

//------------------------------------------------------------------------------
//
void QETable::setOrientation (const Qt::Orientation orientationIn)
{
   if (this->orientation != orientationIn) {
      this->orientation = orientationIn;
      if (this->isVertical ()) {
         this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
      } else {
         this->table->setSelectionBehavior (QAbstractItemView::SelectColumns);
      }
      this->rePopulateTable ();
   }
}

//------------------------------------------------------------------------------
//
Qt::Orientation QETable::getOrientation () const
{
   return this->orientation;
}

//---------------------------------------------------------------------------------
//
void QETable::addVariableName (const QString& pvName)
{
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      if (this->dataSet [slot].isInUse() == false) {
         // Found an empty slot.
         //
         this->setVariableName (slot, pvName);
         break;
      }
   }
}

//==============================================================================
// Copy / Paste
//
QString QETable::copyVariable ()
{
   QString result;

   // Create a space separated list of PV names.
   //
   result = "";
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = this->getSubstitutedVariableName (slot);
      if (!pvName.isEmpty()) {
         if (!result.isEmpty()) result.append (" ");
         result.append (pvName);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QVariant QETable::copyData ()
{
   return QVariant ();    // TBD
}

//------------------------------------------------------------------------------
//
void QETable::paste (QVariant v)
{
   QStringList pvNameList;

   pvNameList = QEUtilities::variantToStringList (v);
   for (int j = 0; j < pvNameList.count (); j++) {
      this->addVariableName (pvNameList.value (j));
   }
}

// end
