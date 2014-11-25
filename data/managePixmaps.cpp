/*  managePixmaps.cpp
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
 *  Copyright (c) 2011
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <managePixmaps.h>

managePixmaps::managePixmaps()
{
    // Initialy set all pixmaps to a blank pixmap
    QPixmap blank;
    for( int i = 0; i < NUM_PIXMAPS_MANAGED; i++)
    {
        pixmaps.append( blank );
    }
}

void managePixmaps::setDataPixmap( const QPixmap& pixmap, const unsigned int index )
{
    // Sanity check
    if( index >= (unsigned int)pixmaps.count() )
        return;

    // Save the pixmap
    pixmaps[index] = pixmap;
}

QPixmap managePixmaps::getDataPixmap( const unsigned int index ) const
{
    // Sanity check
    if( index >= (unsigned int)pixmaps.count() )
    {
        QPixmap blank;
        return blank;
    }

    // Return the pixmap
    return pixmaps[index];
}

QPixmap managePixmaps::getDataPixmap( const QString text ) const
{
    QStringList list = text.simplified().split(QRegExp("\\s+"));

    // Attempt to interpret the text as a floating point number
    bool ok = false;
    double dValue = 0.0;

    if( list.count() )
        dValue = list[0].toDouble( &ok );

    // Convert any resultant floating point number to a pixmap index
    unsigned int iValue = (unsigned int)dValue;

    // If the text was interpreted as a floating point number, select and return the pixmap
    if( ok )
    {
        return getDataPixmap( iValue );
    }

    // If the text could not be interpreted as a floating point number, return a blank pixmap
    else
    {
        QPixmap blank;
        return blank;
    }
}

// end
