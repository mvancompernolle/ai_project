/*  QELineEdit.cpp
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
 *  Copyright (c) 2009, 2010, 2012
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class is a CA aware line edit widget based on the Qt line edit widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QELineEdit.h>
#include <QMessageBox>


/*
    Constructor with no initialisation
*/
QELineEdit::QELineEdit( QWidget *parent ) : QEGenericEdit( parent )
{
    setup();
}

/*
    Constructor with known variable
*/
QELineEdit::QELineEdit( const QString& variableNameIn, QWidget *parent ) : QEGenericEdit( variableNameIn, parent )
{
    setup();
}

void QELineEdit::setup()
{
    setAddUnits( false );
}


/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a line edit a QCaObject that streams strings is required.
*/
qcaobject::QCaObject* QELineEdit::createQcaItem( unsigned int variableIndex ) {

    // Create the item as a QEString
    return new QEString( getSubstitutedVariableName( variableIndex ), this, &stringFormatting, variableIndex );
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QELineEdit::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    if(  qca ) {
        QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setTextIfNoFocus( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo& ) ),
                          this, SLOT( connectionChanged( QCaConnectionInfo& ) ) );
        QObject::connect( this, SIGNAL( requestResend() ),
                          qca, SLOT( resendLastData() ) );
    }
}


/*
    Pass the text update straight on to the QLineEdit unless the user is
    editing the text.
    Note, it would not be common to have a user editing a regularly updating
    value. However, this scenario should be allowed for. A reasonable reason
    for a user updated value to update on a gui is if is is written to by
    another user on another gui.
    This is the slot used to recieve data updates from a QCaObject based class.
*/
void QELineEdit::setTextIfNoFocus( const QString& value, QCaAlarmInfo& alarmInfo, QCaDateTime& dateTime, const unsigned int& ) {

    // Do generic update processing.
    setDataIfNoFocus (QVariant (value), alarmInfo, dateTime);

    // Signal a database value change to any Link widgets
    emit dbValueChanged( value );
}

// Set widget to the given value
//
void QELineEdit::setValue (const QVariant & value)
{
    setText( value.toString() );
}

QVariant QELineEdit::getValue()
{
   return QVariant (text());
}

// Wite the given value to the associated channel.
//
bool QELineEdit::writeData (const QVariant & value, QString& message)
{
    QEString *qca = dynamic_cast <QEString*> ( getQcaItem(0) );

    if( qca ) {
       return qca->writeString( value.toString (), message);
    } else {
        message = "null qca object";
        return false;
    }
}

// end
