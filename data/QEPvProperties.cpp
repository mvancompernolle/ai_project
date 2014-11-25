/*  QEPvProperties.cpp
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
 *  Copyright (c) 2012, 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QFrame>
#include <QHeaderView>

#include <QEScaling.h>
#include <QECommon.h>
#include <QEAdaptationParameters.h>
#include <QELabel.h>
#include <QEStringFormatting.h>

#include "QEPvProperties.h"
#include "QEPvPropertiesUtilities.h"


#define DEBUG qDebug() << "QEPvProperties::" << __FUNCTION__ << ":" << __LINE__

// INP/OUT and CALC fields are 80, 120 should cover it.
//
#define MAX_FIELD_DATA_SIZE  120


//==============================================================================
// class wide data
//==============================================================================
//
static bool recordSpecsAreInitialised = false;       // setup housekeeping
static QERecordSpec *pDefaultRecordSpec = NULL;      // default for unknown record types
static QERecordSpecList recordSpecList;              // list of record type specs


//==============================================================================
// Utilities function
//==============================================================================
//
// This function is idempotent
//
static void initialiseRecordSpecs ()
{
   bool okay;
   QEAdaptationParameters ap ("QE_");

   // If already setup then exit.
   //
   if (recordSpecsAreInitialised) return;
   recordSpecsAreInitialised = true;

   recordSpecList.clear ();

   // Create a record spec to be used as default if we given an unknown record type.
   // All the common fields plus meta field RTYP plus VAL.
   //
   pDefaultRecordSpec = new QERecordSpec ("_default_");
   (*pDefaultRecordSpec)
         << "RTYP" << "NAME$" << "DESC$" << "ASG"   << "SCAN" << "PINI" << "PHAS"
         << "EVNT" << "TSE"   << "TSEL"  << "DTYP"  << "DISV" << "DISA" << "SDIS$"
         << "DISP" << "PROC"  << "STAT"  << "SEVR"  << "NSTA" << "NSEV" << "ACKS"
         << "ACKT" << "DISS"  << "LCNT"  << "PACT"  << "PUTF" << "RPRO" << "PRIO"
         << "TPRO" << "UDF"   << "FLNK$" << "VAL";

   okay = false;

   // First process the internal file list (from resource file).
   //
   okay |= recordSpecList.processRecordSpecFile (":/qe/pvproperties/record_field_list.txt");

   // Next agument from any file specified using the environment variable.
   //
   okay |= recordSpecList.processRecordSpecFile (ap.getString ("record_field_list", ""));

   if (okay == false) {
      DEBUG << "unable to read any record field files";
   }
}


//==============================================================================
// Tables columns
//
#define FIELD_COL                 0
#define VALUE_COL                 1
#define NUNBER_COLS               2
#define DEFAULT_SECTION_SIZE      22

#define WIDGET_MIN_WIDTH          340
#define WIDGET_MIN_HEIGHT         400

#define WIDGET_DEFAULT_WIDTH      448
#define WIDGET_DEFAULT_HEIGHT     696

#define ENUMERATIONS_MIN_HEIGHT   12
#define ENUMERATIONS_MAX_HEIGHT   100
#define NUMBER_OF_ENUMERATIONS    32


//==============================================================================
// QEPvProperties class functions
//==============================================================================
//
void QEPvProperties::createInternalWidgets ()
{
   const int label_height = 18;
   const int label_width = 48;

   int j;

   // Creates all the internal widgets including setting basic geometry.
   //
   this->topFrame = new QFrame (this);
   this->topFrame->setFixedHeight (128);     // go on - do the sums...
   this->topFrame->setObjectName ("topFrame");

   this->topFrameVlayout = new QVBoxLayout (this->topFrame);
   this->topFrameVlayout->setContentsMargins (0, 2, 0, 4);  // l, t, r, b
   this->topFrameVlayout->setSpacing (6);
   this->topFrameVlayout->setObjectName ("topFrameVlayout");

   for (j = 1; j <= 5; j++) {
      this->hlayouts [j] = new QHBoxLayout ();
      this->hlayouts [j]->setContentsMargins (2, 0, 2, 0);  // l, t, r, b
      this->hlayouts [j]->setSpacing (6);

      this->topFrameVlayout->addLayout (this->hlayouts [j], 1);
   }

   this->label1 = new QLabel ("NAME", this->topFrame);
   this->label1->setFixedSize (QSize (label_width, label_height));

   this->box = new QComboBox (this->topFrame);
   this->box->setFixedHeight (label_height + 9);
   this->hlayouts [1]->addWidget (this->label1, 0, Qt::AlignVCenter);
   this->hlayouts [1]->addWidget (this->box, 0, Qt::AlignVCenter);

   this->label2 = new QLabel ("VAL", this->topFrame);
   this->label2->setFixedSize (QSize (label_width, label_height));
   this->valueLabel = new QELabel (this->topFrame);
   this->valueLabel->setFixedHeight (label_height);
   this->hlayouts [2]->addWidget (this->label2);
   this->hlayouts [2]->addWidget (this->valueLabel);

   this->label3 = new QLabel ("HOST", this->topFrame);
   this->label3->setFixedSize (QSize (label_width, label_height));
   this->hostName = new QLabel (this->topFrame);
   this->hostName->setFixedHeight (label_height);
   this->hlayouts [3]->addWidget (this->label3);
   this->hlayouts [3]->addWidget (this->hostName);

   this->label4 = new QLabel ("TIME", this->topFrame);
   this->label4->setFixedSize (QSize (label_width, label_height));
   this->timeStamp = new QLabel (this->topFrame);
   this->timeStamp->setFixedHeight (label_height);
   this->hlayouts [4]->addWidget (this->label4);
   this->hlayouts [4]->addWidget (this->timeStamp);

   this->label5 = new QLabel ("DBF", this->topFrame);
   this->label5->setFixedSize (QSize (label_width, label_height));
   this->fieldType = new QLabel (this->topFrame);
   this->fieldType->setFixedHeight (label_height);
   this->label6 = new QLabel ("INDEX", this->topFrame);
   this->label6->setFixedSize (QSize (label_width, label_height));
   this->indexInfo = new QLabel (this->topFrame);
   this->indexInfo->setFixedHeight (label_height);
   this->hlayouts [5]->addWidget (this->label5);
   this->hlayouts [5]->addWidget (this->fieldType);
   this->hlayouts [5]->addWidget (this->label6);
   this->hlayouts [5]->addWidget (this->indexInfo);

   this->enumerationFrame = new QFrame (NULL); // is re-parented by enumerationScroll
   for (j = 0; j < NUMBER_OF_ENUMERATIONS; j++) {
      QLabel * item;
      item = new QLabel (this->enumerationFrame);
      item->setGeometry (0, 0, 128, label_height);
      this->enumerationLabelList.append (item);
   }

   // Create scrolling area and add pv frame.
   //
   this->enumerationScroll = new QScrollArea ();          // this will become parented by enumerationResize
   this->enumerationScroll->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->enumerationScroll->setWidgetResizable (true);    // MOST IMPORTANT
   this->enumerationScroll->setWidget (this->enumerationFrame);

   // Create user controllable resize area
   //
   this->enumerationResize = new QEResizeableFrame (QEResizeableFrame::BottomEdge,
                                                    ENUMERATIONS_MIN_HEIGHT,
                                                    ENUMERATIONS_MAX_HEIGHT, this);
   this->enumerationResize->setFixedHeight (ENUMERATIONS_MIN_HEIGHT);
   this->enumerationResize->setFrameShape (QFrame::Panel);
   this->enumerationResize->setGrabberToolTip ("Re size enuerations");
   this->enumerationResize->setWidget (this->enumerationScroll);

   // We create this with 40 rows initially - this will get expanded if/when necessary.
   // Mainly want enough to make it look sensible in designer.
   //
   this->table = new QTableWidget (40, NUNBER_COLS, this);
   this->tableContextMenu = new QMenu (this);

   this->vlayout = new QVBoxLayout (this);
   this->vlayout->setMargin (4);
   this->vlayout->setSpacing (4);
   this->vlayout->addWidget (this->topFrame);
   this->vlayout->addWidget (this->enumerationResize);
   this->vlayout->addWidget (this->table);
}

//------------------------------------------------------------------------------
//
QEPvProperties::QEPvProperties (QWidget* parent) : QEFrame (parent)
{
   this->recordBaseName = "";
   this->common_setup ();
}

//------------------------------------------------------------------------------
//
QEPvProperties::QEPvProperties (const QString & variableName, QWidget * parent) :
      QEFrame (parent)
{
   this->recordBaseName = QERecordFieldName::recordName (variableName);
   this->common_setup ();
   setVariableName (variableName, 0);
   this->valueLabel->setVariableName (variableName, 0);
}

//------------------------------------------------------------------------------
// NB. Need to do a deep clear to avoid memory loss.
//
QEPvProperties::~QEPvProperties ()
{
   // Free up all allocated QEString objects.
   //
   // Ensure the standardRecordType and alternateRecordType QEString objects
   // are deleted as well as the field QEString objects.
   //
   if (this->standardRecordType) {
      delete this->standardRecordType;
      this->standardRecordType = NULL;
   }

   if (this->alternateRecordType) {
      delete this->alternateRecordType;
      this->alternateRecordType = NULL;
   }

   while (!this->fieldChannels.isEmpty ()) {
      QEString *qca = this->fieldChannels.takeFirst ();
      if (qca) {
         delete qca;
      }
   }
}

//------------------------------------------------------------------------------
//
QSize QEPvProperties::sizeHint () const {
   return QSize (WIDGET_DEFAULT_WIDTH, WIDGET_DEFAULT_HEIGHT);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::common_setup ()
{
   QTableWidgetItem * item;
   QString style;
   int j;
   QLabel *enumLabel;

   // This function only perform required actions on first call.
   //
   initialiseRecordSpecs ();

   this->fieldChannels.clear ();

   this->standardRecordType = NULL;
   this->alternateRecordType = NULL;

   // configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // allocate and configure own widgets
   // ...and setup an alias
   //
   this->createInternalWidgets ();

   // Configure widgets
   //
#ifndef QT_NO_COMPLETER
   // Could not get completer to work
   this->box->setAutoCompletion (true);
   this->box->setAutoCompletionCaseSensitivity (Qt::CaseSensitive);
#endif
   this->box->setEditable (true);
   this->box->setMaxCount (36);
   this->box->setMaxVisibleItems (20);
   this->box->setEnabled (true);
   // These two don't seem to enforce what one might sensibly expect.
   this->box->setInsertPolicy (QComboBox::InsertAtTop);
   this->box->setDuplicatesEnabled (false);

   // We use the activated signal (as opposed to currentIndexChanged) as it
   // is only emmited on User change.
   //
   QObject::connect (this->box, SIGNAL (activated              (int)),
                     this,      SLOT   (boxCurrentIndexChanged (int)));

   // We allow ourselves to select the index programatically.
   //
   QObject::connect (this,      SIGNAL (setCurrentBoxIndex (int)),
                     this->box, SLOT   (setCurrentIndex    (int)));

   style = "QWidget { background-color: #F0F0F0; }";

   this->valueLabel->setStyleSheet (style);
   // We have to be general here
   this->valueLabel->setPrecision (9);
   this->valueLabel->setUseDbPrecision (false);
   this->valueLabel->setNotationProperty (QELabel::Automatic);

   this->hostName->setIndent (4);
   this->hostName->setStyleSheet (style);

   this->timeStamp->setIndent (4);
   this->timeStamp->setStyleSheet (style);

   this->fieldType->setAlignment(Qt::AlignHCenter);
   this->fieldType->setStyleSheet (style);

   this->indexInfo->setAlignment(Qt::AlignRight);
   this->indexInfo->setIndent (4);
   this->indexInfo->setStyleSheet (style);

   for (j = 0; j < this->enumerationLabelList.count (); j++) {
      enumLabel = this->enumerationLabelList.value (j);
      enumLabel->setIndent (4);
      enumLabel->setStyleSheet (style);
   }

   item = new QTableWidgetItem (" Field ");
   this->table->setHorizontalHeaderItem (FIELD_COL, item);

   item = new QTableWidgetItem (" Value ");
   this->table->setHorizontalHeaderItem (VALUE_COL, item);

   this->table->horizontalHeader()->setDefaultSectionSize (60);
   this->table->horizontalHeader()->setStretchLastSection (true);

   this->table->verticalHeader()->hide ();
   this->table->verticalHeader()->setDefaultSectionSize (DEFAULT_SECTION_SIZE);


   // Setup layout of widgets with the QEPvProperties QFrame
   //
   this->setMinimumWidth (WIDGET_MIN_WIDTH);
   this->setMinimumHeight(WIDGET_MIN_HEIGHT);

   this->fieldStringFormatting.setAddUnits (false);
   this->fieldStringFormatting.setUseDbPrecision (false);
   this->fieldStringFormatting.setPrecision (12);
   this->fieldStringFormatting.setNotation (QEStringFormatting::NOTATION_AUTOMATIC);
   this->fieldStringFormatting.setArrayAction (QEStringFormatting::ASCII);

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Framework boiler-plate stuff.
   //
   // This control used a single PV via the framework.
   //
   this->setNumVariables (1);

   // Enable drag drop onto this widget by default.
   //
   this->setAllowDrop (true);

   // By default, the PV properties widget does not display the alarm state.
   // The internal VALue widget does this on our behalf.
   //
   this->setDisplayAlarmState (false);

   // Use standard context menu for overall widget.
   //
   this->setupContextMenu();

   // Do special context for the table.
   //
   this->table->setContextMenuPolicy (Qt::CustomContextMenu);

   QObject::connect (this->table, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this,        SLOT   (customContextMenuRequested (const QPoint &)));

   QObject::connect (this->tableContextMenu, SIGNAL (triggered             (QAction* )),
                     this,                   SLOT   (contextMenuTriggered  (QAction* )));


   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   QObject::connect (
         &variableNamePropertyManager, SIGNAL (newVariableNameProperty    (QString, QString, unsigned int)),
         this,                         SLOT   (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
void  QEPvProperties::resizeEvent (QResizeEvent *)
{
   QRect g;
   QLabel *enumLabel;
   int pw;
   int ew;   // enumerations with
   int epr;  // enumerations per row.
   int gap;
   int lh;   // label height
   int lw;   // label width
   int j;

   // Find scaled gap and enumeration width values.
   //
   gap = QEScaling::scale (4);
   ew  = QEScaling::scale (172);

   pw = this->enumerationFrame->width ();
   epr = MAX (1, (pw / ew));    // calc enumerations per row.
   lw = ((pw - gap)/ epr) - gap;
   lh = this->enumerationLabelList.value (0)->geometry().height();

   for (j = 0; j < this->enumerationLabelList.count (); j++) {
      enumLabel = this->enumerationLabelList.value (j);
      enumLabel->setGeometry (gap + (j%epr)*(lw + gap), gap + (j/epr)*(lh + gap), lw, lh);
   }
}

//------------------------------------------------------------------------------
// NB. Need to do a deep clear to avoid memory loss.
//
void QEPvProperties::clearFieldChannels ()
{
   QEString *qca;
   QTableWidgetItem *item;
   QString gap ("           ");  // empirically found to be quivilent width of " DESC "
   int j;

   while (!this->fieldChannels.isEmpty ()) {
      qca = this->fieldChannels.takeFirst ();
      delete qca;
   }

   for (j = 0; j < this->table->rowCount (); j++) {
      item = table->verticalHeaderItem (j);
      if (item) {
         item->setText (gap);
      }

      item = this->table->item (j, 0);
      if (item) {
         item->setText ("");
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::useNewVariableNameProperty (QString variableNameIn,
                                                 QString variableNameSubstitutionsIn,
                                                 unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn, variableNameSubstitutionsIn, variableIndex);
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEPvProperties::createQcaItem (unsigned int variableIndex)
{
   DEBUG <<variableIndex;
   return NULL;  // We don't need a QEWidget managed connection.
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setUpRecordTypeChannels (QEString* &qca, const  PVReadModes readMode)
{
   QString pvName;
   QString recordTypeName;

   pvName = this->getSubstitutedVariableName (0).trimmed ();
   this->recordBaseName = QERecordFieldName::recordName (pvName);

   recordTypeName = QERecordFieldName::rtypePvName (pvName);
   if (readMode == ReadAsCharArray) {
      recordTypeName.append ("$");
   }

   // Delete any existing qca object if needs be.
   //
   if (qca) {
      delete qca;
      qca = NULL;
   }

   qca = new QEString (recordTypeName, this, &stringFormatting, (unsigned int) readMode);

   if (readMode == ReadAsCharArray)  {
      // Record type names are never longer than standard string.
      //
      qca->setRequestedElementCount (40);
   } else {
      qca->setRequestedElementCount (1);
   }

   QObject::connect (qca,  SIGNAL (connectionChanged       (QCaConnectionInfo&, const unsigned int& )),
                     this, SLOT   (setRecordTypeConnection (QCaConnectionInfo&, const unsigned int& )));

   QObject::connect (qca,  SIGNAL (stringChanged      (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                     this, SLOT   (setRecordTypeValue (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));

   qca->subscribe ();
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setUpLabelChannel ()
{
   QString pvName;
   qcaobject::QCaObject *qca = NULL;

   // The pseudo RTYP field has connected - we are good to go...
   //
   pvName = this->getSubstitutedVariableName (0).trimmed ();

   // Set PV name of internal QELabel.
   //
   this->valueLabel->setVariableNameAndSubstitutions (pvName, "", 0);

   // We know that QELabels use slot zero for their connection.
   //
   qca = this->valueLabel->getQcaItem (0);
   if (qca) {
      QObject::connect (qca, SIGNAL (connectionChanged  (QCaConnectionInfo&, const unsigned int&) ),
                        this,  SLOT (setValueConnection (QCaConnectionInfo&, const unsigned int&) ) );

      QObject::connect (qca, SIGNAL (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                        this,  SLOT (setValueValue (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::establishConnection (unsigned int variableIndex)
{
   QString substitutedPVName;

   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   substitutedPVName = this->getSubstitutedVariableName (0).trimmed ();
   this->recordBaseName = QERecordFieldName::recordName (substitutedPVName);

   // Set up field name label.
   //
   this->label2->setText (QERecordFieldName::fieldName (substitutedPVName));

   // Clear associated data fields.
   //
   this->hostName->setText ("");
   this->timeStamp->setText ("");
   this->fieldType->setText ("");
   this->indexInfo->setText ("");
   this->valueLabel->setText ("");

   //-----------------------------------------------------
   // Clear any exiting field connections.
   //
   this->clearFieldChannels ();

   // Remove this name from mid-list if it exists and (re) insert at top of list.
   //
   this->insertIntoDropDownList (substitutedPVName);

   // Ensure CombBox consistent .
   //
   emit setCurrentBoxIndex (0);

   // Set up internal QElabel object.
   //
   this->setUpLabelChannel ();

   // Set up connections to XXXX.RTYP and XXXX.RTYP$.
   // We do this to firstly establish the record type name (e.g. ai, calcout),
   // but also to determine if the PV server (IOC) supports character array mode
   // for string PVs. This is usefull for long strings (> 40 characters).
   //
   this->setUpRecordTypeChannels (this->alternateRecordType, ReadAsCharArray);
   this->setUpRecordTypeChannels (this->standardRecordType,  StandardRead);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setRecordTypeConnection (QCaConnectionInfo& connectionInfo,
                                              const unsigned int &variableIndex)
{
   const PVReadModes readMode = (PVReadModes) variableIndex;

   if ((readMode == ReadAsCharArray) && connectionInfo.isChannelConnected ()) {
      // XXX.RTYP$ connected - pre empty standard connection.
      //
      delete this->standardRecordType;
      this->standardRecordType = NULL;
   }

   // Update tool tip, but leave the basic widget enabled.
   //
   updateToolTipConnection (connectionInfo.isChannelConnected ());
}


//------------------------------------------------------------------------------
// Called when notified of the (new) record type value.
//
void QEPvProperties::setRecordTypeValue (const QString& rtypeValue,
                                         QCaAlarmInfo&,
                                         QCaDateTime&,
                                         const unsigned int& variableIndex)
{
   const PVReadModes readMode = (PVReadModes) variableIndex;

   int j;
   QERecordSpec *pRecordSpec;
   int numberOfFields;
   bool mayUseCharArray;
   bool fieldUsingCharArray;
   QString readField;
   QString pvField;
   QString displayField;

   QTableWidgetItem* item;
   QString pvName;
   QEString *qca;

   // Look for the record spec for the given record type if it exists.
   //
   pRecordSpec = recordSpecList.find (rtypeValue);

   // If we didn't find the specific record type, use the default record spec.
   //
   if (!pRecordSpec) {
       pRecordSpec = pDefaultRecordSpec;
   }

   // If we didn't find the specific record type or the default record spec
   // not defined then quit.
   //
   if(!pRecordSpec) {
      // Output some error???
      return;
   }

   // It is possible that a record may change RTYP (e.g. calc to calcout while IOC is off line)
   // Cannot rely soley on the clear called in createQcaItem / establish connection.
   //
   this->clearFieldChannels ();

   numberOfFields = pRecordSpec->size ();

   this->table->setRowCount (numberOfFields);
   for (j = 0; j < numberOfFields; j++) {

      readField = pRecordSpec->getFieldName (j);
      mayUseCharArray = readField.endsWith ('$');

      if (mayUseCharArray) {
         displayField = readField;
         displayField.chop (1);       // remove last character
      } else {
         displayField = readField;    // use as is.
      }

      fieldUsingCharArray = (readMode == ReadAsCharArray) && mayUseCharArray;

      if (fieldUsingCharArray) {
         pvField = displayField;
         pvField.append ('$');        // append CA array mode qualifier.
      } else {
         pvField = displayField;
      }

      // Ensure vertical header exists and set it.
      //
      item = this->table->item (j, FIELD_COL);
      if (!item) {
         // We need to allocate iteem and inset into the table.
         item = new QTableWidgetItem ();
         this->table->setItem (j, FIELD_COL, item);
      }
      item->setText  (" " + displayField + " ");

      // Ensure table entry item exists.
      //
      item = this->table->item (j, VALUE_COL);
      if (!item) {
         // We need to allocate item and inset into the table.
         item = new QTableWidgetItem ();
         this->table->setItem (j, VALUE_COL, item);
      }

      // Form the required PV name.
      //
      pvName = this->recordBaseName + "." + pvField;

      qca = new QEString (pvName, this, &this->fieldStringFormatting, j);

      if (fieldUsingCharArray) {
         qca->setRequestedElementCount (MAX_FIELD_DATA_SIZE);
      } else {
         qca->setRequestedElementCount (1);
      }

      QObject::connect (qca, SIGNAL (connectionChanged  (QCaConnectionInfo&, const unsigned int& )),
                        this,  SLOT (setFieldConnection (QCaConnectionInfo&, const unsigned int& )));

      QObject::connect (qca, SIGNAL (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                        this,  SLOT (setFieldValue (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));

      qca->subscribe();

      this->fieldChannels.append (qca);
   }

   // The alarmInfo not really applicabe to the RTYP field.
   // We pick up on the VAL field instead.
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setValueConnection (QCaConnectionInfo& connectionInfo, const unsigned int&)
{
   qcaobject::QCaObject *qca;
   QString s;

   // These are not QELabels - so gotta do manually.
   //
   this->hostName->setEnabled  (connectionInfo.isChannelConnected ());
   this->timeStamp->setEnabled (connectionInfo.isChannelConnected ());
   this->fieldType->setEnabled (connectionInfo.isChannelConnected ());
   this->indexInfo->setEnabled (connectionInfo.isChannelConnected ());

   if (connectionInfo.isChannelConnected ()) {
      // We "know" that the only/main channel is the 1st (slot 0) channel.
      //
      qca = this->valueLabel->getQcaItem (0);
      this->hostName->setText (qca->getHostName());
      this->fieldType->setText (qca->getFieldType());

      // Assume we are looking at 1st/only element for now.
      //
      s.sprintf ("%d / %ld", 1,  qca->getElementCount());
      this->indexInfo->setText (s);
      this->isFirstUpdate = true;
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setValueValue (const QString &,
                                    QCaAlarmInfo& alarmInfo,
                                    QCaDateTime& dateTime,
                                    const unsigned int&)
{
   qcaobject::QCaObject *qca;
   QStringList enumerations;
   QLabel *enumLabel;
   QLabel *enumLast;
   int n;
   int j;
   QRect g;
   int h;

   // NOTE: The value label updates itself.
   //
   this->timeStamp->setText (dateTime.text () + "  " + QEUtilities::getTimeZoneTLA (dateTime));

   if (this->isFirstUpdate) {

      // Ensure we do any required resizing.
      //
      this->resizeEvent (NULL);

      // Set up any enumeration values
      // We "know" that the only/main channel is the 1st (slot 0) channel.
      //
      qca = this->valueLabel->getQcaItem (0);
      enumerations = qca->getEnumerations ();
      n = enumerations.count();

      enumLast = NULL;
      for (j = 0; j < this->enumerationLabelList.count (); j++) {
         enumLabel = this->enumerationLabelList.value (j);
         if (j < n) {
            // Value is specified.
            enumLabel->setText (enumerations.value (j));
            enumLast = enumLabel;
            enumLabel->setVisible (true);
         } else {
            enumLabel->clear ();
            enumLabel->setVisible (false);
         }
      }

      if (enumLast) {
         g = enumLast->geometry ();
         h = g.top() + g.height() + 4;
      } else {
         h = 0;
      }
      this->enumerationFrame->setFixedHeight (h);

      // Set and expand to new max height.
      this->enumerationResize->setAllowedMaximum (ENUMERATIONS_MIN_HEIGHT + h);
      this->enumerationResize->setFixedHeight (ENUMERATIONS_MIN_HEIGHT + h);

      this->isFirstUpdate = false;
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setFieldConnection (QCaConnectionInfo& connectionInfo,
                                         const unsigned int &variableIndex)
{
   int numberOfRows;
   QTableWidgetItem *item;

   numberOfRows = this->table->rowCount ();
   if ((int) variableIndex < numberOfRows) {
      item = this->table->item (variableIndex, VALUE_COL);

      if (connectionInfo.isChannelConnected ()) {
         // connected
         item->setForeground (QColor (0, 0, 0));
         item->setText ("");
      } else {
         // disconnected - leave old text - readable but grayed out.
         item->setForeground (QColor (160, 160, 160));
      }
   } else {
      DEBUG << "variableIndex =" << variableIndex
            << ", out of range - must be <" << numberOfRows;
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setFieldValue (const QString &value,
                                    QCaAlarmInfo &,
                                    QCaDateTime &,
                                    const unsigned int & variableIndex)
{
   int numberOfRows;
   QTableWidgetItem *item;

   numberOfRows = this->table->rowCount ();
   if ((int) variableIndex < numberOfRows) {
      item = this->table->item (variableIndex, VALUE_COL);
      if (value.length () < MAX_FIELD_DATA_SIZE) {
         item->setText  (" " + value);
      } else {
         // The string has maxed-out the read length, add ...
         item->setText  (" " + value + "...");
      }
   } else {
      DEBUG << "variableIndex =" << variableIndex
            << ", out of range - must be <" << numberOfRows;
   }
}

//------------------------------------------------------------------------------
// Unlike most widgets, the frame is not disabled if/when PVs disconnect
// Normally, standardProperties::setApplicationEnabled() is called
// For this widget our own version which just calls the widget's setEnabled is called.
//
void QEPvProperties::setApplicationEnabled (const bool& state)
{
    QWidget::setEnabled (state);
}

//==============================================================================
// ComboBox
//
void QEPvProperties::boxCurrentIndexChanged (int index)
{
   QString newPvName;
   QString oldPvName;

   if (index >= 0) {
      newPvName = this->box->itemText (index);
      oldPvName = getSubstitutedVariableName (0);

      // belts 'n' braces.
      //
      if (newPvName != oldPvName) {
         this->setVariableName (newPvName, 0);
         this->establishConnection (0);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::insertIntoDropDownList (const QString& pvName)
{
   // Remove the PV name from mid-list if it exists and (re) insert at top of list.
   //
   for (int slot = this->box->count() - 1; slot >= 0; slot--) {
      if (this->box->itemText (slot).trimmed () == pvName) {
         this->box->removeItem (slot);
      }
   }

   // Make sure at least 2 free slots - one for this PV and one
   // for the user to type.
   //
   while (this->box->count() >= box->maxCount () - 2) {
      this->box->removeItem (box->count () - 1);
   }

   this->box->insertItem (0, pvName, QVariant ());
}

//==============================================================================
// Conextext menu.
//
void QEPvProperties::customContextMenuRequested (const QPoint & posIn)
{
   QTableWidgetItem* item = NULL;
   QString trimmed;
   int row;
   qcaobject::QCaObject* qca = NULL;
   QString newPV;
   QAction *action;
   QPoint pos = posIn;
   QPoint golbalPos;

   // Find the associated item
   //
   item = this->table->itemAt (posIn);
   if (!item) {
      return;  // just in case
   }

   switch (item->column ()) {
      case FIELD_COL:
         row = item->row ();
         qca = this->fieldChannels.value (row, NULL);
         if (qca) {
            newPV = qca->getRecordName ();
         } else {
            newPV = "";
         }
         break;

      case VALUE_COL:
         trimmed = item->text ().trimmed ();
         QERecordFieldName::extractPvName (trimmed, newPV);
         break;

      default:
         DEBUG << "unexpected column number:" << item->column () << trimmed;
         newPV = "";
         return;
   }

   action = new QAction ("Properties", this->tableContextMenu);
   action->setCheckable (false);
   action->setData (QVariant (newPV));
   action->setEnabled (!newPV.isEmpty ());
   this->tableContextMenu->clear ();
   this->tableContextMenu->addAction (action);

   pos.setY (pos.y () + DEFAULT_SECTION_SIZE);  // A feature of QTableWiget (because header visible maybe?).
   golbalPos = table->mapToGlobal (pos);
   this->tableContextMenu->exec (golbalPos, 0);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::contextMenuTriggered (QAction* action)
{
   QString newPV;

   if (action) {
      newPV = action->data ().toString ();

      this->setVariableName (newPV , 0);
      this->establishConnection (0);
   }
}

//==============================================================================
// Save / restore
//
void QEPvProperties::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->persistantName ("QEPvProperties");
   PMElement formElement = pm->addNamedConfiguration (formName);

   // qDebug () << "\nQEPvProperties " << __FUNCTION__ << formName << "\n";

   // Note: we save the subsituted name (as opposed to template name and any macros).
   //
   formElement.addValue ("Name", this->getSubstitutedVariableName (0));

}

//------------------------------------------------------------------------------
//
void QEPvProperties::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->persistantName ("QEPvProperties");
   PMElement formElement = pm->getNamedConfiguration (formName);
   bool status;
   QString pvName;

   // qDebug () << "\nQEPvProperties " << __FUNCTION__ << formName <<  restorePhase << "\n";

   if ((restorePhase == FRAMEWORK) && !formElement.isNull ()) {
      status = formElement.getValue ("Name", pvName);
      if (status) {
         this->setPvName (pvName);
      }
   }
}

//==============================================================================
//
void QEPvProperties::setPvName (const QString& pvNameIn)
{
   this->setVariableName (pvNameIn, 0);
   this->establishConnection (0);
}


//==============================================================================
// Copy / Paste
//
QString QEPvProperties::copyVariable ()
{
   return this->getSubstitutedVariableName (0);
}

//------------------------------------------------------------------------------
//
QVariant QEPvProperties::copyData ()
{
   QTableWidgetItem *f, *v;
   QString fieldList;
   QString field;
   QString value;
   QString line;

   // Create csv format.
   //
   fieldList.clear ();
   for (int i = 0; i < table->rowCount(); i++) {
      f = this->table->item (i, FIELD_COL);
      v = this->table->item (i, VALUE_COL);

      // Ensure both items have been allocated and assigned.
      //
      if (f && v) {

         field = f->text ().trimmed ();
         value = v->text ().trimmed ();

         // Right pad field to width of 6, suits most records.
         //
         while (field.length () < 6) field.append (" ");

         line = QString ("%1 , %2\n").arg (field).arg (value);
         fieldList.append (line);
      }
   }
   return QVariant (fieldList);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::paste (QVariant v)
{
   QStringList pvNameList;

   pvNameList = QEUtilities::variantToStringList (v);

   // Insert all suppled names into thw drop down list (in reverse order)
   // and select the first PV name (if it exists of course).
   //
   for (int j = pvNameList.count () - 1; j >= 0 ;j--) {
      QString pvName = pvNameList.value (j);
      if (j > 0) {
         this->insertIntoDropDownList (pvName);
      } else {
         this->setPvName (pvName);
      }
   }
}

// end
