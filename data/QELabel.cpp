/*  QELabel.cpp
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

/*
  This class is a CA aware label widget based on the Qt label widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QELabel.h>

/*
    Constructor with no initialisation
*/
QELabel::QELabel( QWidget *parent ) : QLabel( parent ), QEWidget( this ) {
    setup();
}

/*
    Constructor with known variable
*/
QELabel::QELabel( const QString &variableNameIn, QWidget *parent ) : QLabel( parent ), QEWidget( this )  {
    setup();
    setVariableName( variableNameIn, 0 );
}

/*
    Setup common to all constructors
*/
void QELabel::setup() {

    // Set up data
    // This control used a single data source
    setNumVariables(1);

    // Set up default properties
    setAllowDrop( false );

    // Set the initial state
    setText( "----" );
    setIndent( 6 );
    isConnected = false;
    updateConnectionStyle( isConnected );
    updateOption = UPDATE_TEXT;

    // Use standard context menu
    setupContextMenu();

//    defaultStyleSheet = styleSheet();
    // Use label signals
    // --Currently none--

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    QObject::connect( &variableNamePropertyManager, SIGNAL( newVariableNameProperty( QString, QString, unsigned int ) ), this, SLOT( useNewVariableNameProperty( QString, QString, unsigned int) ) );

}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a label a QCaObject that streams strings is required.
*/
qcaobject::QCaObject* QELabel::createQcaItem( unsigned int variableIndex ) {
    // Create the item as a QEString
   return new QEString( getSubstitutedVariableName( variableIndex ), this, &stringFormatting, variableIndex );
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QELabel::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    if(  qca ) {
        QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setLabelText( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        qca->setRequestedElementCount( 10000 );

        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo& ) ),
                          this, SLOT( connectionChanged( QCaConnectionInfo& ) ) );
        QObject::connect( this, SIGNAL( requestResend() ),
                          qca, SLOT( resendLastData() ) );
    }
}


/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QELabel::connectionChanged( QCaConnectionInfo& connectionInfo )
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    // Display the connected state
    updateToolTipConnection( isConnected );
    updateConnectionStyle( isConnected );
}

/*
    Update the label text
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QELabel::setLabelText( const QString& textIn, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& ) {

    // Extract any formatting info from the text
    // For example "<background-color: red>Engineering Mode" or "<color: red>not selected"
    currentText = textIn;
    QString textStyle;
    int textStyleStart = currentText.indexOf( '<' );
    if( textStyleStart >= 0 )
    {
        int textStyleEnd = currentText.indexOf( '>', textStyleStart );
        if( textStyleEnd >= 1 )
        {
            textStyle = currentText.mid( textStyleStart+1, textStyleEnd-textStyleStart-1 );
            currentText = currentText.left( textStyleStart ).append( currentText.right( currentText.length()-textStyleEnd-1 ));
        }
    }

    // Update the color
    if( textStyle.compare( lastTextStyle ) )
    {
        if( !textStyle.isEmpty() )
        {
            updateDataStyle( QString( "QWidget { " ).append( textStyle ).append( "; }") );
        }
        else
        {
            updateDataStyle( "" );
        }
        lastTextStyle = textStyle;
    }

    // Signal a database value change to any Link widgets
    emit dbValueChanged( currentText );

    switch( updateOption )
    {
        // Update the text if required
        case UPDATE_TEXT:
            setText( currentText );
            break;

        // Update the pixmap if required
        case UPDATE_PIXMAP:
            setPixmap( getDataPixmap( currentText ).scaled( size() ) );
            break;
    }

    // Invoke common alarm handling processing.
    processAlarmInfo( alarmInfo );
}

//==============================================================================
// Drag drop
void QELabel::setDrop( QVariant drop )
{
    setVariableName( drop.toString(), 0 );
    establishConnection( 0 );
}

QVariant QELabel::getDrop()
{
    if( isDraggingVariable() )
        return QVariant( copyVariable() );
    else
        return copyData();
}

//==============================================================================
// Copy / Paste
QString QELabel::copyVariable()
{
    return getSubstitutedVariableName(0);
}

QVariant QELabel::copyData()
{
    return QVariant( currentText );
}

void QELabel::paste( QVariant v )
{
    if( getAllowDrop() )
    {
        setDrop( v );
    }
}

//==============================================================================
// Property convenience functions

// Update option Property convenience function
void QELabel::setUpdateOption( updateOptions updateOptionIn )
{
    updateOption = updateOptionIn;
}
QELabel::updateOptions QELabel::getUpdateOption()
{
    return updateOption;
}
