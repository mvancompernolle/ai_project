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


#include <styleManager.h>
#include <QEWidget.h>

// Construction.
styleManager::styleManager( QWidget* ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "styleManager constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;
    defaultStyleSheet = owner->styleSheet();

    level = userLevelTypes::USERLEVEL_USER;
}

// Set the Style Sheet string to be applied when the widget is displayed in 'User' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleUser( QString style )
{
    userUserStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'User' mode.
QString styleManager::getStyleUser()
{
    return userUserStyle;
}

// Set the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleScientist( QString style )
{
    userScientistStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
QString styleManager::getStyleScientist()
{
    return userScientistStyle;
}

// Set the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleEngineer( QString style )
{
    userEngineerStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
QString styleManager::getStyleEngineer()
{
    return userEngineerStyle;
}

// Set the Style Sheet string to be applied to reflect an aspect of the current data.
// For example, a value over a high limit may be displayed in red.
void styleManager::updateDataStyle( QString style )
{
    dataStyleSheet = style;
    updateStyleSheet();
}

// Set the Style Sheet string to be applied to reflect an aspect of the current status.
// For example, invalid data may be displayed with a white background.
void styleManager::updateStatusStyle( QString style )
{
    statusStyleSheet = style;
    updateStyleSheet();
}

// Set the Style Sheet string to be applied to reflect the current connection state
// (connected or disconnected) of the current data.
// For example, a disconnected value may be greyed out.
void styleManager::updateConnectionStyle( bool connected )
{
    if( connected )
    {
        connectionStyleSheet = "";
    }
    else
    {
        connectionStyleSheet = "QWidget { color: grey }";
    }
    updateStyleSheet();
}

// Set the Style Sheet string to be applied to implement a widget property.
// For example, a style string is used to set QE button text alignment.
void styleManager::updatePropertyStyle( QString style )
{
    propertyStyleSheet = style;
    updateStyleSheet();
}

// Update the style sheet with the various style sheet components used to modify the label style (user level, connection state, alarm info, enumeration color)
void styleManager::updateStyleSheet()
{
    // Do nothing if in designer.
    // Altering the style can alter properties that are written when designer saves a form.
    // For example, setting the background through the style will alter the pallete properties which will be saved when the form is saved.
    // Designer does not indicate the file has changed (a * beside the file name) but the pallet property can be seen to be updated
    // and if the user saves the file the pallette changes are saved.
    // So, for example, if a variable is disconnected, the style to indicate disconnected becomes permenant when the file
    // is saved because the property changed by the disconnected style is saved.
    if( QEWidget::inDesigner() )
    {
        return;
    }

    // Select the appropriate user level style
    QString levelStyle;
    switch( level )
    {
        case userLevelTypes::USERLEVEL_USER:
            levelStyle = userUserStyle;
            break;

        case userLevelTypes::USERLEVEL_SCIENTIST:
            levelStyle = userScientistStyle;
            break;

        case userLevelTypes::USERLEVEL_ENGINEER:
            levelStyle = userEngineerStyle;
            break;
    }

    // Compile and apply the entire style string if there is any difference with what is currently there
    QString newStyleSheet = defaultStyleSheet;

    if( !newStyleSheet.isEmpty() && !connectionStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( connectionStyleSheet );

    if( !newStyleSheet.isEmpty() && !propertyStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( propertyStyleSheet );

    if( !newStyleSheet.isEmpty() && !statusStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( statusStyleSheet );

    if( !newStyleSheet.isEmpty() && !dataStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( dataStyleSheet );

    if( !newStyleSheet.isEmpty() && !levelStyle.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( levelStyle );

    if( newStyleSheet.compare( owner->styleSheet() ))
    {
        owner->setStyleSheet( newStyleSheet );
    }

}

// Set the current user level.
void styleManager::styleUserLevelChanged( userLevelTypes::userLevels levelIn )
{
    // Note the new style and update the style string if changed
    bool newLevel = level != levelIn;
    level = levelIn;
    if( newLevel )
    {
        updateStyleSheet();
    }
}
