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
 *  Copyright (c) 2012
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/* Description:
 *
 * This class is similar to QEInteger, QEFloating and QEString classes in that it will always
 * emit signals containing QByteArray data no mater what the type of the underlying PV.
 * The other classes mentioned achieve this by converting the data retrieved as appropriate.
 * This class always simply returns the raw data as a byte array
 *
 */

#include <QEByteArray.h>
#include <QtDebug>

/*
    QE Byte array creation.

    Note, the QCaObject is created with low priorityfor the following scenario:
    Several large rapidly updating images being displayed. Network bandwith is far less than would support the image update rate.
    When scalar values are requested, the request times out before the scalar updates.
*/
QEByteArray::QEByteArray( QString recordName, QObject *eventObject,
                        unsigned int variableIndexIn ) : QCaObject( recordName, eventObject, variableIndexIn, SIG_BYTEARRAY, QE_PRIORITY_LOW ) {
    initialise();
}
QEByteArray::QEByteArray( QString recordName, QObject *eventObject,
                        unsigned int variableIndexIn, UserMessage* userMessageIn ) : QCaObject( recordName, eventObject, variableIndexIn, userMessageIn, SIG_BYTEARRAY, QE_PRIORITY_LOW ) {
    initialise();
}

/*
    Stream the QCaObject data through this class to generate byte array data updates
*/
void QEByteArray::initialise() {

    QObject::connect( this, SIGNAL( connectionChanged(  QCaConnectionInfo&, const unsigned int&  ) ),
                      this, SLOT( forwardConnectionChanged( QCaConnectionInfo& , const unsigned int& ) ) );

    QObject::connect( this, SIGNAL( dataChanged( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ),
                      this, SLOT( forwardDataChanged( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ) );
}

/*
    Take a new byte array value and write it to the database.
*/
void QEByteArray::writeByteArray( const QByteArray &data ) {
    writeData( QVariant( data ));
}

/*
    Slot to recieve data updates from the base QCaObject and generate byte array updates.
*/
void QEByteArray::forwardDataChanged( const QByteArray &value, unsigned long dataSize, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex  ) {
    emit byteArrayChanged( value, dataSize, alarmInfo, timeStamp, variableIndex );
}

/*
    Re send connection change and with variableIndex - depricated.
*/
void QEByteArray::forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex ) {
    emit byteArrayConnectionChanged( connectionInfo, variableIndex );
}

// end
