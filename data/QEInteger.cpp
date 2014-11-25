/*
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
 *  Copyright (c) 2009, 2010
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Integer specific wrapper for QCaObject.

#include <QEInteger.h>
#include <QtDebug>

/*
    ???
*/
QEInteger::QEInteger( QString recordName, QObject *eventObject,
                      QEIntegerFormatting *integerFormattingIn,
                      unsigned int variableIndexIn ) : QCaObject( recordName, eventObject, variableIndexIn ) {
    initialise( integerFormattingIn );
}

QEInteger::QEInteger( QString recordName, QObject *eventObject,
                      QEIntegerFormatting *integerFormattingIn,
                      unsigned int variableIndexIn, UserMessage* userMessageIn ) : QCaObject( recordName, eventObject, variableIndexIn, userMessageIn ) {
    initialise( integerFormattingIn );
}

/*
    Stream the QCaObject data through this class to generate integer data updates
*/
void QEInteger::initialise( QEIntegerFormatting* integerFormattingIn ) {
    integerFormat = integerFormattingIn;

    QObject::connect( this, SIGNAL( connectionChanged(  QCaConnectionInfo&, const unsigned int&  ) ),
                      this, SLOT( forwardConnectionChanged( QCaConnectionInfo&, const unsigned int&  ) ) );

    QObject::connect( this, SIGNAL( dataChanged( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ),
                      this, SLOT( convertVariant( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ) );
}

/*
    Take a new integer value and write it to the database.
    The type of data formatted (text, floating, integer, etc) will be determined by the record data type,
    How the integer is parsed will be determined by the integer formatting. For example, integer to string may require always including a sign.
*/
void QEInteger::writeInteger( const long &data ) {
    writeData( integerFormat->formatValue( data, getDataType() ) );
}

/*
    Slot to recieve data updates from the base QCaObject and generate integer updates.
*/
void QEInteger::convertVariant( const QVariant &value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex ) {
    if( value.type() == QVariant::List )
    {
        emit integerArrayChanged( integerFormat->formatIntegerArray( value ), alarmInfo, timeStamp, variableIndex );
    }
    else
    {
        emit integerChanged( integerFormat->formatInteger( value ), alarmInfo, timeStamp, variableIndex );
    }
}

/*
    Re send connection change and with variableIndex - depricated.
*/
void QEInteger::forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex) {
    emit integerConnectionChanged( connectionInfo, variableIndex );
}

// end
