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

// QE based version of CA Monitor
// This program uses a QEString to source text based CA data.
// A QEString is the natural source for a text output based program,
// however the QEString can be replaced with QEInteger if you want
// to play with QEInteger instead. Look for references to QEString and QEInteger
// in this class.

#include <QTime>
#include <QString>
#include <monitor.h>

monitor::monitor( QString pvIn )
{
    stream = new QTextStream( stdout );

    // Save the PV for logging udpates
    pv = pvIn;

    // Create the data source, connect to data update and message signals, then subscribe to updates.
    // Normal
    source = new QEString( pv, this, &formatting, 1, &messages );
    QObject::connect( source, SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int & ) ),
                      this, SLOT( log( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int & ) ) );

    // Integer only output
//    source = new QEInteger( pv, this, &formatting, 1, &messages );
//    QObject::connect( source, SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int & ) ),
//                      this, SLOT( log( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int & ) ) );

    QObject::connect( source, SIGNAL( connectionChanged( QCaConnectionInfo& ) ),
                      this, SLOT( connectionChanged( QCaConnectionInfo& ) ) );

    source->subscribe();
}

// Log connection issues
void monitor::connectionChanged( QCaConnectionInfo& connectionInfo )
{
    if( !connectionInfo.isChannelConnected() )
        *stream << QString( "%1: %2   Channel not connected\n").arg( QTime::currentTime().toString() ).arg( pv );
    if( !connectionInfo.isLinkUp() )
        *stream << QString( "%1: %2   Link not up\n").arg( QTime::currentTime().toString() ).arg( pv );

    stream->flush();
}

// Log data updates and messages
// Normal
void monitor::log( const QString& data, QCaAlarmInfo&, QCaDateTime& timeStamp, const unsigned int & )
// Integer only output
// void monitor::log( const long& data, QCaAlarmInfo&, QCaDateTime& timeStamp, const unsigned int & )
{
    *stream << QString( "%1: %2   %3\n").arg( timeStamp.text() ).arg( pv ).arg( data );
    stream->flush();
}

// Log messages
void monitor::newMessage( QString msg, message_types )
{
    *stream << QString( "%1 %2   %3\n").arg( QTime::currentTime().toString() ).arg( pv ).arg( msg );
    stream->flush();
}
