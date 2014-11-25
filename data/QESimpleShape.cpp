/*  QESimpleShape.cpp
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

#include <QPainter>

#include <QDebug>
#include <QECommon.h>

#include "QESimpleShape.h"

#define DEBUG qDebug () << "QESimpleShape" << __LINE__ << __FUNCTION__

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QESimpleShape::QESimpleShape (QWidget * parent) : QEFrame (parent)
{
   this->setup ();
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QESimpleShape::QESimpleShape (const QString & variableNameIn, QWidget * parent) : QEFrame (parent)
{
   this->setup ();
   this->setVariableName (variableNameIn, 0);
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QESimpleShape::setup ()
{
   // Set default property values
   // Super class....
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);

   // This control uses a single data source
   //
   this->setNumVariables (1);
   this->setVariableAsToolTip (true);
   this->setDisplayAlarmState (true);
   this->setAllowDrop (false);

   // This class properties.
   //
   this->value = 0;
   this->shape = rectangle;
   this->textFormat = FixedText;
   this->fixedText = "";
   this->textImage = "";
   this->isStaticValue = false;
   this->flashOffColour = QColor (200, 200, 200,  0);  // clear, alpha = 0

   this->flashRate = QEScanTimers::Medium;
   QEScanTimers::attach (this, SLOT (flashTimeout (const bool)), this->flashRate);

   for (int j = 0; j < 16; j++) {
      this->colourList[j] = QColor (200, 200, 200, 255);
      this->flashList [j] = false;
   }


   // Set the initial state
   //
   this->isConnected = false;

   // Use default context menu.
   //
   this->setupContextMenu();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   QObject::connect (&this->variableNamePropertyManager, SIGNAL (newVariableNameProperty    (QString, QString, unsigned int)),
                     this,                               SLOT   (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//-----------------------------------------------------------------------------
// Reduce width or height of rectangle so that it becomes square, and offset
// to keep it centred.
//
void QESimpleShape::equaliseRect (QRect & rect)
{
   int diff;

   diff = rect.width () - rect.height ();

   if (diff > 0) {
      rect.setWidth (rect.height ());
      rect.moveLeft (diff / 2);

   } else if (diff < 0) {
      rect.setHeight (rect.width ());
      rect.moveTop ((-diff) / 2);
   }
}

//------------------------------------------------------------------------------
//
void QESimpleShape::drawText (QPainter & painter, QPoint & textCentre, QString & text)
{
   QFont pf (this->font ());
   painter.setFont (pf);

   QFontMetrics fm = painter.fontMetrics ();
   int x;
   int y;

   // Centre text. For height, pointSize seems better than fm.height ()
   // painter.drawText takes bottom left coordinates.
   //
   x = textCentre.x () - fm.width (text) / 2;
   y = textCentre.y () + (pf.pointSize () + 1) / 2;

   // If text too wide, then ensure we show most significant part.
   //
   painter.drawText (MAX (1, x), y, text);
}

//-----------------------------------------------------------------------------
//
void QESimpleShape::paintEvent (QPaintEvent*)
{
   QPainter painter (this);
   QPen pen;
   QBrush brush;
   QRect rect;
   QPoint polygon[8];
   int mv;
   QColor colour;
   QColor boarderColour;
   bool washedOut = false;
   qcaobject::QCaObject* qca = NULL;
   QString text;
   int x0, x1, x2;
   int y0, y1, y2;

   // Use value to index colour table.
   //
   mv = this->getModuloValue ();  // contrained 0 .. 15
   if (this->isStaticValue) {
      // Static - no alarm, not considered disconnected.
      //
      colour = this->getColourProperty (mv);
      washedOut = !this->isEnabled ();
   } else {
      // Variable driven.
      //
      if (this->getDisplayAlarmState ()) {
         // Use alarm colour
         // Associated qca object - test to avoid the segmentation fault.
         // and test variable is connected.
         //
         qca = this->getQcaItem (0);
         if (qca && this->isConnected) {
            QCaAlarmInfo ai = qca->getAlarmInfo ();   // 1st param is & mode - cannot use a function.
            colour = this->getColor (ai, 255);
         } else {
            // No channel or disconnected, no alarm state available go with grey.
            //
            colour = QColor (200, 200, 200);
         }
      } else {
         // Use value to index colour table.
         // If disconnected we use last know value.
         //
         colour = this->getColourProperty (mv);
      }
      washedOut = !(this->isEnabled () && this->isConnected);
   }

   // Boarder colour is same colout just a darker colour.
   //
   boarderColour = QEUtilities::darkColour (colour);

   // flash the colour, but not the boarder.
   //
   if (this->flashList [mv] && !this->flashStateIsOn) {
      colour = this->flashOffColour;
   }

   if (washedOut) {
      // Disconnected or disabled - grey out colours.
      //
      colour = QEUtilities::blandColour (colour);
      boarderColour = QEUtilities::blandColour (boarderColour);
   }

   pen.setWidth (1);
   pen.setColor (boarderColour);
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (colour);
   painter.setBrush (brush);

   rect = this->geometry ();
   rect.moveTo (0, 0);

   rect.setWidth (rect.width () - 1);
   rect.setHeight (rect.height () - 1);

   switch (this->getShape ()) {

      case circle:
         this->equaliseRect (rect);
         // fall through
      case ellipse:
         painter.drawEllipse (rect);
         break;


      case square:
         this->equaliseRect (rect);
         // fall through
      case rectangle:
         painter.drawRect (rect);
         break;


      case roundSquare:
         this->equaliseRect (rect);
         // fall through
      case roundRectangle:
         // Normalise corner radii.
         //
         painter.drawRoundRect (rect, 1200.0 / rect.width (), 1200.0 / rect.height ());
         break;


      case equalDiamond:
         this->equaliseRect (rect);
         // fall through
      case diamond:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.top ());
         polygon[1] = QPoint (rect.right (), (rect.top () + rect.bottom ()) / 2);
         polygon[2] = QPoint ((rect.left () + rect.right ()) / 2, rect.bottom ());
         polygon[3] = QPoint (rect.left (), (rect.top () + rect.bottom ()) / 2);
         polygon[4] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 5);
         break;


      case triangleUp:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.top ());
         polygon[1] = QPoint (rect.right (), rect.bottom ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleDown:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.bottom ());
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleLeft:
         polygon[0] = QPoint (rect.left (), (rect.top () + rect.bottom ()) / 2);
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.right (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleRight:
         polygon[0] = QPoint (rect.right (), (rect.top () + rect.bottom ()) / 2);
         polygon[1] = QPoint (rect.left (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleTopRight:
         polygon[0] = QPoint (rect.right (), rect.top ());
         polygon[1] = QPoint (rect.right (), rect.bottom ());
         polygon[2] = QPoint (rect.left (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleBottomRight:
         polygon[0] = QPoint (rect.right (), rect.bottom ());
         polygon[1] = QPoint (rect.left (), rect.bottom ());
         polygon[2] = QPoint (rect.right (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleBottomLeft:
         polygon[0] = QPoint (rect.left (), rect.bottom ());
         polygon[1] = QPoint (rect.left (), rect.top ());
         polygon[2] = QPoint (rect.right (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleTopLeft:
         polygon[0] = QPoint (rect.left (), rect.top ());
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case arrowUp:
         // calculate some intermediate values.
         //
         y0 = rect.top () + rect.width () / 2;
         x0 = (rect.left () + rect.right ()) / 2;
         x1 = (2 * rect.left () + rect.right ()) / 3;
         x2 = (rect.left () + 2 * rect.right ()) / 3;

         polygon[0] = QPoint (x0, rect.top ());
         polygon[1] = QPoint (rect.right (), y0);
         polygon[2] = QPoint (x2, y0);
         polygon[3] = QPoint (x2, rect.bottom ());
         polygon[4] = QPoint (x1, rect.bottom ());
         polygon[5] = QPoint (x1, y0);
         polygon[6] = QPoint (rect.left (), y0);
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 7);
         break;

      case arrowDown:
         // calculate some intermediate values.
         //
         y0 = rect.bottom () - rect.width () / 2;
         x0 = (rect.left () + rect.right ()) / 2;
         x1 = (2 * rect.left () + rect.right ()) / 3;
         x2 = (rect.left () + 2 * rect.right ()) / 3;

         polygon[0] = QPoint (x0, rect.bottom ());
         polygon[1] = QPoint (rect.right (), y0);
         polygon[2] = QPoint (x2, y0);
         polygon[3] = QPoint (x2, rect.top ());
         polygon[4] = QPoint (x1, rect.top ());
         polygon[5] = QPoint (x1, y0);
         polygon[6] = QPoint (rect.left (), y0);
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 7);
         break;

      case arrowLeft:
         // calculate some intermediate values.
         //
         x0 = rect.left () + rect.height () / 2;
         y0 = (rect.top () + rect.bottom ()) / 2;
         y1 = (2 * rect.top () + rect.bottom ()) / 3;
         y2 = (rect.top () + 2 * rect.bottom ()) / 3;

         polygon[0] = QPoint (rect.left (),y0);
         polygon[1] = QPoint (x0, rect.top ());
         polygon[2] = QPoint (x0, y1);
         polygon[3] = QPoint (rect.right (), y1);
         polygon[4] = QPoint (rect.right (), y2);
         polygon[5] = QPoint (x0, y2);
         polygon[6] = QPoint (x0, rect.bottom ());
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 7);
         break;

      case arrowRight:
         // calculate some intermediate values.
         //
         x0 = rect.right () - rect.height () / 2;
         y0 = (rect.top () + rect.bottom ()) / 2;
         y1 = (2 * rect.top () + rect.bottom ()) / 3;
         y2 = (rect.top () + 2 * rect.bottom ()) / 3;

         polygon[0] = QPoint (rect.right (), y0);
         polygon[1] = QPoint (x0, rect.top ());
         polygon[2] = QPoint (x0, y1);
         polygon[3] = QPoint (rect.left (), y1);
         polygon[4] = QPoint (rect.left (), y2);
         polygon[5] = QPoint (x0, y2);
         polygon[6] = QPoint (x0, rect.bottom ());
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 7);
         break;


      default:
         break;
   }

   // Get the rquired text -f any.
   //
   text = this->getTextImage ();
   if (!text.isEmpty ()) {
      // Set default centre text positions.
      //
      QPoint textCentre (this->width () / 2, this->height () / 2);

      if (!washedOut) {
         pen.setColor (QEUtilities::fontColour (colour));
      } else {
         pen.setColor (QColor (140, 140, 140, 255));   // gray
      }
      painter.setPen (pen);
      this->drawText (painter, textCentre, text);
   }
}

//------------------------------------------------------------------------------
//
void QESimpleShape::flashTimeout (const bool isOn)
{
   this->flashStateIsOn = isOn;
   this->update ();   // only call is current state marked as flashing???
}

//------------------------------------------------------------------------------
// Update variable name etc.
//
void QESimpleShape::useNewVariableNameProperty (QString variableNameIn,
                                                QString variableNameSubstitutionsIn,
                                                unsigned int variableIndex)
{
   this->isStaticValue = false;

   // Note: essentially calls createQcaItem - provided expanded pv name is not empty.
   //
   this->setVariableNameAndSubstitutions (variableNameIn,
                                          variableNameSubstitutionsIn,
                                          variableIndex);

   this->update ();       // causes a paint event.
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For shape, a QCaObject that streams integers is required.
//
qcaobject::QCaObject* QESimpleShape::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;
   QString pvName;
   int number;
   bool okay;

   if (variableIndex == 0) {
      pvName = this->getSubstitutedVariableName (variableIndex);
      number = pvName.toInt (&okay) & 0x0F;
      // Has designer/user just set an integer (as opposed to a PV name)?.
      // Note: no sensible PV names are just integers.
      //
      if (okay) {
         this->isStaticValue = true;
         this->value = number;
         this->setTextImage ();
      } else {
         // Assume it is a PV.
         //
         result = new QEInteger (pvName, this, &this->integerFormatting, variableIndex);
      }

   } else {
      result = NULL;         // Unexpected
   }

   return result;
}


//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QESimpleShape::establishConnection (unsigned int variableIndex)
{
   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject * qca = createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if ((qca) && (variableIndex == 0)) {
      QObject::connect (qca,  SIGNAL (integerChanged (const long &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT   (setShapeValue  (const long &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (integerArrayChanged (const QVector < long >&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT   (setShapeValues      (const QVector < long >&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int &)));
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QESimpleShape::connectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int& variableIndex)
{
   // Note the connected state
   this->isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   this->updateToolTipConnection (this->isConnected, variableIndex);

   // Widget is self draw - styleShheet not applicable per se.
   // No need to call updateConnectionStyle( isConnected );

   this->isFirstUpdate = true;  // more trob. than it's worth to check if connect or disconnect.
   this->update ();
}


//------------------------------------------------------------------------------
// Update the shape value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QESimpleShape::setShapeValue (const long &valueIn, QCaAlarmInfo & alarmInfo,
                                   QCaDateTime &, const unsigned int& variableIndex)
{
   qcaobject::QCaObject* qca;

   // Associated qca object - avoid the segmentation fault.
   //
   qca = getQcaItem (0);
   if (!qca) {
      return;
   }
   
   // Set up variable details used by some formatting options.
   //
   if (this->isFirstUpdate) {
      this->stringFormatting.setDbEgu (qca->getEgu ());
      this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
      this->stringFormatting.setDbPrecision (qca->getPrecision ());
   }

   this->value = valueIn;
   this->setTextImage ();
   this->update ();    // causes a paint event.


   // Signal a database value change to any Link widgets
   //
   emit dbValueChanged (value);

   // Invoke tool tip handling directly. We don;t want to interfer with the style
   // as widget draws it's own stuff with own, possibly clear, colours.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);

   // This update is over, clear first update flag.
   //
   this->isFirstUpdate = false;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setTextImage ()
{
   switch (this->getTextFormat ()) {

      case QESimpleShape::FixedText:
         this->textImage = this->getFixedText ();
         break;

      case QESimpleShape::PvText:
         this->textImage = this->stringFormatting.formatString (this->getValue ());
         break;

      case QESimpleShape::LocalEnumeration:
         this->textImage = this->stringFormatting.formatString (this->getModuloValue ());
         break;

      default:
         this->textImage = "";
   }
}


//------------------------------------------------------------------------------
//
QString QESimpleShape::getTextImage () {
   return this->textImage;
}

//------------------------------------------------------------------------------
// Extract first element (0 index) and use this value.
//
void QESimpleShape::setShapeValues (const QVector<long> & values,
                                    QCaAlarmInfo & alarmInfo, QCaDateTime & dateTime,
                                    const unsigned int &variableIndex)
{
   int slot = 0;
   this->setShapeValue (values.value (slot), alarmInfo, dateTime, variableIndex);
}

//------------------------------------------------------------------------------
//
int QESimpleShape::getValue () const
{
   return this->value;
}

//------------------------------------------------------------------------------
//
int QESimpleShape::getModuloValue () const
{
   return this->value & 0x0F;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setShape (Shapes shapeIn)
{
   if (this->shape != shapeIn) {
      this->shape = shapeIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QESimpleShape::Shapes QESimpleShape::getShape () const
{
   return this->shape;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setTextFormat (TextFormats value)
{
   if (this->textFormat != value) {
      this->textFormat = value;

      this->textImage = "";

      // Convert local format into appropriate string formmating.
      //
      switch (this->textFormat) {

         case QESimpleShape::FixedText:
            this->textImage = this->fixedText;
            break;

         case QESimpleShape::PvText:
            this->setFormat (QEStringFormatting::FORMAT_DEFAULT);
            break;

         case QESimpleShape::LocalEnumeration:
            this->setFormat (QEStringFormatting::FORMAT_LOCAL_ENUMERATE);
            break;

         default:
            break;
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QESimpleShape::TextFormats QESimpleShape::getTextFormat () const
{
   return this->textFormat;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setFixedText (QString value)
{
   if (this->fixedText != value) {
      this->fixedText = value;
      if (this->getTextFormat () == FixedText) {
         this->textImage = this->fixedText;
         this->update ();
      }
   }
}

//------------------------------------------------------------------------------
//
QString QESimpleShape::getFixedText () const
{
   return this->fixedText;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setFlashRate (QEScanTimers::ScanRates flashRateIn)
{
   const char* member = SLOT (flashTimeout (const bool));

   if (this->flashRate != flashRateIn) {
      QEScanTimers::detach (this, member);
      this->flashRate = flashRateIn;
      QEScanTimers::attach (this, member, this->flashRate);
   }
}

//------------------------------------------------------------------------------
//
QEScanTimers::ScanRates QESimpleShape::getFlashRate () const
{
   return this->flashRate;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setFlashOffColour (const QColor& flashOffColourIn)
{
   this->flashOffColour = flashOffColourIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QESimpleShape::getFlashOffColour () const
{
   return this->flashOffColour;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setColourProperty (int slot, QColor colour)
{
   if ((slot >= 0) && (slot < 16)) {
      if (this->colourList[slot] != colour) {
         this->colourList[slot] = colour;
         if (this->getModuloValue () == slot) {
            this->update ();
         }
      }
   }
}

//------------------------------------------------------------------------------
//
QColor QESimpleShape::getColourProperty (int slot) const
{
   QColor result;

   if ((slot >= 0) && (slot < 16)) {
      result = this->colourList[slot];
   } else {
      result = QColor (0, 0, 0, 255);
   }
   return result;
}


//------------------------------------------------------------------------------
//
void QESimpleShape::setFlashProperty (int slot, bool flash)
{
   if ((slot >= 0) && (slot < 16)) {
      this->flashList [slot] = flash;
   }
}

//------------------------------------------------------------------------------
//
bool QESimpleShape::getFlashProperty (int slot) const
{
   bool result = false;

   if ((slot >= 0) && (slot < 16)) {
      result = this->flashList[slot];
   }
   return result;
}

//==============================================================================
// Drag drop
//
void QESimpleShape::setDrop (QVariant drop)
{
   this->setVariableName (drop.toString (), 0);
   this->establishConnection (0);
}

//------------------------------------------------------------------------------
//
QVariant QESimpleShape::getDrop ()
{
   if( isDraggingVariable() )
      return QVariant( this->copyVariable() );
   else
      return this->copyData();
}

//==============================================================================
// Copy / paste
//
QString QESimpleShape::copyVariable ()
{
   return this->getSubstitutedVariableName (0);
}

//------------------------------------------------------------------------------
//
QVariant QESimpleShape::copyData ()
{
   return QVariant (this->getValue ());
}

void QESimpleShape::paste (QVariant v)
{
   if( getAllowDrop() )
   {
      this->setDrop (v);
   }
}

// end
