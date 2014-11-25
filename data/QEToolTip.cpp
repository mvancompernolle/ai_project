/*  QEToolTip.cpp
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
 *  Copyright (c) 2009, 2010
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QDebug>
#include "QEToolTip.h"

#define DEBUG qDebug () << "QEToolTip" << __FUNCTION__ << __LINE__

QEToolTip::QEToolTip(  QWidget* ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "QEToolTip constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;

    number = 0;
    variableAsToolTip = true;
}

// Property set: variable as tool tip
void QEToolTip::setVariableAsToolTip( const bool variableAsToolTipIn )
{

    // Set the new tool tip type
    variableAsToolTip = variableAsToolTipIn;

    // Update the tool tip to match the new state
    displayToolTip();
}

// Property get: variable as tool tip
bool QEToolTip::getVariableAsToolTip() const
{
    return variableAsToolTip;
}

// Ensures list are large enough.
//
void QEToolTip::setNumberToolTipVariables (const unsigned int numberIn)
{
    number = numberIn;

    while (toolTipVariable.count()    < (int) number) toolTipVariable << "";
    while (toolTipAlarm.count()       < (int) number) toolTipAlarm << "";
    while (toolTipIsConnected.count() < (int) number) toolTipIsConnected << false;

    while (toolTipVariable.count()    > (int) number) toolTipVariable.removeLast();
    while (toolTipAlarm.count()       > (int) number) toolTipAlarm.removeLast();
    while (toolTipIsConnected.count() > (int) number) toolTipIsConnected.removeLast();
}

// Update the variable used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipVariable ( const QString& variable, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipVariable.count ()) {
        toolTipVariable.replace( variableIndex, variable );
        displayToolTip();
    }
}

// Update the variable alarm status used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipAlarm ( const QString& alarm, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipAlarm.count ()) {
        toolTipAlarm .replace( variableIndex, alarm );
        displayToolTip();
    }
}

// Update the variable custom information used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipCustom ( const QString& custom )
{
    toolTipCustom = custom;
    displayToolTip();
}

// Update the variable connection status used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipConnection ( bool isConnectedIn, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipIsConnected.count ()) {
        toolTipIsConnected.replace (variableIndex, isConnectedIn);
        displayToolTip();
    }
}


// Build and display the tool tip from the name and state if dynamic
void QEToolTip::displayToolTip()
{
    // If using the variable name as the tool tip, build the tool tip
    if( variableAsToolTip )
    {
        int count = 0;
        QString toolTip = "";

        for (unsigned int j = 0; j < number; j++) {
            QString pvName = toolTipVariable.value (j, "");
            if( pvName.size() ) {
                if (count > 0) {
                    toolTip.append( "\n" );
                }
                count++;
                toolTip.append( pvName );

                if( toolTipIsConnected.value (j, false ) ) {
                    // Only connected PVs have an alarm state.
                    if( toolTipAlarm [j].size() )
                        toolTip.append( " - " ).append( toolTipAlarm.value (j, "") );

                } else {
                    toolTip.append( " - Disconnected" );
                }
            }
        }
        if (count == 0) {
            toolTip = "No variables defined";
        }
        if( toolTipCustom.size() )
            toolTip.append( " - " ).append( toolTipCustom );

        owner->setToolTip( toolTip );
    }
}

// end
