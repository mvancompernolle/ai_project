/*  QEWidget.cpp
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
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

/*
  This class is used as a base for all CA aware wigets, such as QELabel, QESpinBox, etc.
  It manages common issues including creating a source of CA data updates, handling error,
  warning and status messages, and setting tool tips based on variable names.

  Note, there is tight integration between the CA aware widget classes, this class, and its
  base classes, especially VariableNameManager and QEToolTip.

  Refer to the class description in QEWidget.h for further details
 */

#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QEWidget.h>
#include <QEFrameworkVersion.h>
#include <QEForm.h>
#include <standardProperties.h>

/*
    Constructor
*/
QEWidget::QEWidget( QWidget *ownerIn ) : QEToolTip( ownerIn ), QEDragDrop( ownerIn ), styleManager( ownerIn ), contextMenu( this ), standardProperties( ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "QEWidget constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;

    // Initially flag no variables array is defined.
    // This will be corrected when the first variable is declared
    numVariables = 0;
    qcaItem = 0;

    //
    lastSeverity = QCaAlarmInfo::getInvalidSeverity();
    lastDisplayAlarmState = false;

    // Default properties
    subscribe = true;
    setSourceId( 0 );

    // Set the UserMessage form ID to be whatever has been published in the ContainerProfile
    setFormId( getMessageFormId() );

    // If there is a profile defining the environment containing this widget add this widget to the
    // list of contained widgets so whatever is managing the container can activate this widget.
    //
    // Although a widget is self contained, whatever is creating the widget has the option of providing
    // a list of services and other information through a containerProfile that QEWidgets can use.
    // For example, an application creating QEWidgets can provide a mechanism to display error
    // messages in a manner appropriate for the application.
    // In this case, the widget is taking the oppertunity to tell its creator it exists, and also to
    // get any variable name macro substitutions offered by its creator.
    if( isProfileDefined() )
    {
        addContainedWidget( this );
        setVariableNameSubstitutionsOverride( getMacroSubstitutions() );

        // Set up contextMenu consumer
        setConsumer( getGuiLaunchConsumer() );
    }

    // Setup to respond to requests to save or restore persistant data
    signalSlot.setOwner( this );
    PersistanceManager* persistanceManager = getPersistanceManager();
    if( persistanceManager )
    {
        QObject::connect( persistanceManager->getSaveRestoreObject(),
                          SIGNAL( saveRestore( SaveRestoreSignal::saveRestoreOptions ) ),
                          &signalSlot,
                          SLOT( saveRestore( SaveRestoreSignal::saveRestoreOptions ) ),
                          Qt::DirectConnection );
    }
}

/*
    Destruction:
    Delete all variable sources for the widgeet
*/
QEWidget::~QEWidget() {
    // Remove this widget remove this widget from the list of contained widgets if it is there.
    // The list is only used during form construction and generally widgets are not destroyed during form
    // construction, but there are exceptions. A typical exception is QEMotor, which creates and sometimes
    // destroys QELabels during contruction. These QELabels get added to the contained widgets list
    // but are then destroyed. Unless they are removed from the list, the form will attempt to activate them.
    removeContainedWidget( this );

    // Delete all the QCaObject instances
    for( unsigned int i = 0; i < numVariables; i++ ) {
        deleteQcaItem( i, true );
    }

    // Release the list
    delete[] qcaItem;
    qcaItem = NULL;
}

/*
    Set the number of variables that will be used for this widget.
    Create an array of QCaObject based objects to suit.
    This is called by the CA aware widgets based on this class, such as a QELabel.
*/
void QEWidget::setNumVariables( unsigned int numVariablesIn ) {

    // Get the number of variables that will be used by this widget
    // Don't accept zero or the qca array will be invalid
    if( numVariablesIn ) {
        numVariables = numVariablesIn;
    } else {
        numVariables = 1;
    }

    // Set up the number of variables managed by the variable name manager
    variableNameManagerInitialise( numVariables );

    // Allocate the array of QCa objects
    qcaItem = new qcaobject::QCaObject* [numVariables];
    for( unsigned int i = 0; i < numVariables; i++ ) {
        qcaItem[i] = NULL;
    }
}

/*
   Initiate updates.
   This is only required when QE widgets are loaded within a form and not directly by 'designer'.
   When loaded directly by 'designer' they are activated (a CA connection is established) as soon as either
   the variable name or variable name substitution properties are set
 */
void QEWidget::activate()
{
    // For each variable, ask the CA aware widget based on this class to initiate updates and to set up
    // whatever signal/slot connections are required to make use of data updates.
    // Note, establish connection is a virtual function of the VariableNameManager class and is normally
    // called by that class when a variable name is defined or changed
    for( unsigned int i = 0; i < numVariables; i++ )
        establishConnection( i );

    // Ask the widget to perform any tasks which should only be done once all other widgets have been created.
    // For example, if a widget wants to notify other widgets through signals during construction, other widgets
    // may not be present yet to recieve the signals. This type of notification could be held off untill now.
    activated();
}

/*
   Terminate updates.
   This has been provided for third party (non QEGui) applications using the framework.
 */
void QEWidget::deactivate()
{
    // Delete all the QCaObject instances
    for( unsigned int i = 0; i < numVariables; i++ ) {
        deleteQcaItem( i, false );
    }
}


/*
    Create a CA connection and initiates updates if required.
    This is called by the establishConnection function of CA aware widgets based on this class, such as a QELabel.
    If successfull it will return the QCaObject based object supplying data update signals
*/
qcaobject::QCaObject* QEWidget::createConnection( unsigned int variableIndex ) {

    // If the index is invalid do nothing
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables ) {
        return NULL;
    }

    // Update the variable names in the tooltip if required
    setToolTipFromVariableNames();

    // Remove any existing QCa connection
    deleteQcaItem( variableIndex, false );

    // Connect to new variable.
    // If a new variable name is present, ask the CA aware widget based on this class to create an
    // appropriate object based on a QCaObject (by calling its createQcaItem() function).
    // If that is successfull, supply it with a mechanism for handling errors and subscribe
    // to the new variable if required.
    if( getSubstitutedVariableName( variableIndex ).length() > 0 ) {
        qcaItem[variableIndex] = createQcaItem( variableIndex );
        if( qcaItem[variableIndex] ) {

            qcaItem[variableIndex]->setUserMessage( (UserMessage*)this );

            if( subscribe )
                qcaItem[variableIndex]->subscribe();
        }
    }

    // Return the QCaObject, if any
    return qcaItem[variableIndex];
}

// Default implementation of createQcaItem().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to create the specific flavour of QCaObject required using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA connection,
// this default implementation is here to always return NULL when asked to create a QCaObject
//
qcaobject::QCaObject* QEWidget::createQcaItem( unsigned int )
{
    return NULL;
}

// Default implementation of establishConnection().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to establish a connection on a newly created QCaObject using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA connection,
// this default implementation is here as a default when not implemented
//
void QEWidget::establishConnection( unsigned int )
{
}

// Default implementation of activated().
// Widgets may have tasks which should only be done once all other widgets have been created.
// For example, if a widget wants to notify other widgets through signals during construction, other widgets
// may not be present yet to recieve the signals. This type of notification could be held off untill now.
void QEWidget::activated()
{
}



/*
    Return a reference to one of the qCaObjects used to stream CA data updates to the widget
    This is called by CA aware widgets based on this class, such as a QELabel, mainly when they
    want to connect to its signals to recieve data updates.
*/
qcaobject::QCaObject* QEWidget::getQcaItem( unsigned int variableIndex ) {
    // If the index is invalid return NULL.
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables )
        return NULL;

    // Return the QCaObject used for the specified variable name
    return qcaItem[variableIndex];
}

/*
    Remove any previous QCaObject created to supply CA data updates for a variable name
    If the object connected to the QCaObject is being destroyed it is not good to receive signals so the disconnect parameter should be true in this case
*/
void QEWidget::deleteQcaItem( unsigned int variableIndex, bool disconnect ) {
    // If the index is invalid do nothing.
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables )
        return;

    // Remove the reference to the deleted object to prevent accidental use
    qcaobject::QCaObject* qca = qcaItem[variableIndex];
    qcaItem[variableIndex] = NULL;

    // Delete the QCaObject used for the specified variable name
    if( qca )
    {
        // If we need to disconnect first, do so.
        // If the object connected is being destroyed it is not good to receive signals. (this happened)
        // If the object connected is not being destroyed is will want to know a disconnection has occured.
        if( disconnect )
        {
            qca->disconnect();
        }

        // Delete the QCaObject
        delete qca;
    }
}

/*
  Return a colour to update the widget's look to reflect the current alarm state
  Note, the color is determined by the alarmInfo class, but since that class is used in non
  gui applications, it can't return a QColor
 */
QColor QEWidget::getColor( QCaAlarmInfo& alarmInfo, int saturation )
{
    QColor result(alarmInfo.getColorName());

    int h, s, v;
    result.getHsv( &h, &s, &v );
    result.setHsv( h, saturation, 255 );
    return result;
}

/*
  Provides default (and consistant) alarm handling for all QE widgets.
 */
void QEWidget::processAlarmInfo( QCaAlarmInfo& alarmInfo, const unsigned int variableIndex )
{
    // Gather the current info
    QCAALARMINFO_SEVERITY severity = alarmInfo.getSeverity();
    bool displayAlarmState = getDisplayAlarmState();

    // If anything has changed (either the alarm state itself, or if we have just started
    // or stopped displaying the alarm state), update the alarm style as appropriate.
    if( severity != lastSeverity || displayAlarmState != lastDisplayAlarmState )
    {
        // If displaying the alarm state, apply the current alarm style
        if ( displayAlarmState )
        {
            updateStatusStyle( alarmInfo.style() );
        }

        // If not displaying the alarm state, remove any alarm style
        else
        {
            updateStatusStyle( "" );
        }
    }

    // Regardless of whether we are displaying the alarm state in the widget, update the
    // tool tip to reflect current alarm state.
    updateToolTipAlarm( alarmInfo.severityName(), variableIndex );

    // Save state for processing next update.
    lastSeverity = severity;
    lastDisplayAlarmState = displayAlarmState;
}

// Update the variable name list used in tool tips if requried
void QEWidget::setToolTipFromVariableNames()
{
    // Set tip info
    setNumberToolTipVariables( numVariables );
    for( unsigned int i = 0; i < numVariables; i++ ) {
        // If a variable name is present, add it to the tip
        QString variableName = getSubstitutedVariableName( i );
        updateToolTipVariable( variableName, i );
    }
}

// Returns true if running within the Qt Designer application.
// used when the behaviour needs to be different in designer.
// For example, a run-time-visible property - always visible in designer, visible at run time dependant on the property.
bool QEWidget::inDesigner()
{
    // check if the current executable has 'designer' in the name
    // Note, depending on Qt version, (and installation?) designer image may be 'designer' or 'designer-qt4'
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo fi( appPath );
    return fi.baseName().contains( "designer" );
}

//==============================================================================
// User level

// The user level has changed
// Modify the widget visibility and style accordingly
void QEWidget::userLevelChangedGeneral( userLevelTypes::userLevels level )
{
    // Manage general QE widget aspects of the user level chagning
    styleUserLevelChanged( level );
    checkVisibilityEnabledLevel( level );

    // Allow specific QE widgets to act on a user level change
    userLevelChanged( level );
}

// Perform a single shot read on all variables.
// Widgets may be write only and do not need to subscribe (subscribe property is false).
// When not subscribing it may still be usefull to do a single shot read to get initial
// values, or perhaps confirm a write.
void QEWidget::readNow()
{
    // Perform a single shot read on all variables.
    qcaobject::QCaObject* qca;
    for( unsigned int i = 0; i < numVariables; i++ )
    {
        qca = getQcaItem( i );
        if( qca ) // If variable exists...
        {
            qca->singleShotRead();
        }
    }
}

// Access functions for variableName and variableNameSubstitutions
// variable substitutions Example: SECTOR=01 will result in any occurance of $SECTOR in variable name being replaced with 01.
void QEWidget::setVariableNameAndSubstitutions( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )
{
    setVariableNameSubstitutions( variableNameSubstitutionsIn );
    setVariableName( variableNameIn, variableIndex );
    updateToolTipConnection ( false, variableIndex );  // disconnected until we explicitly connect.
    if( !getDontActivateYet() )
    {
        establishConnection( variableIndex );
    }
}

// Returns the default location to create files.
// Use this to create files in a consistant location
QString QEWidget::defaultFileLocation()
{
    QString path = getParentPath();
    if( !path.isEmpty() )
    {
        return path;
    }

    path = getPath();
    if( !path.isEmpty() )
    {
        return path;
    }

    path = QDir::currentPath();
    if( !path.isEmpty() )
    {
        return path;
    }
    return "";
}

// Returns an open file given a file name.
// This uses findQEFile() to find files in a consistant set of locations. Refer to findQEFile() for details.
QFile* QEWidget::openQEFile( QString name, QFile::OpenModeFlag mode )
{
    // Find the file
    QFile* uiFile = findQEFile( name, this );
    if( uiFile )
    {
        if( !uiFile->open( mode ) )
        {
            delete uiFile;
            uiFile = NULL;
        }
    }
    return uiFile;
}

// Returns a QFile given a file name, or NULL if can't find the file
// Use this to find files in a consistant set of locations:
// If the file name contains an absolute path, then no options, just try to open it
// If the file name contains a relative path (including no path) look in the following locations:
//  - The directory where the parent object (form) was read from (set up in the application profile)
//  - The application's path (set up in the application profile) (the -p switch for QEGui)
//  - The current directory
//  - The environment variable QE_UI_PATH

QFile* QEWidget::findQEFile( QString name )
{
    ContainerProfile publishedProfile;
    return findQEFile( name, &publishedProfile );
}

QFile* QEWidget::findQEFile( QString name, ContainerProfile* profile )
{
        // Build a list of all the places we expect to find the file
        // Use a single location if an absolute path was specified.
        // Use the following list of locations if a relative path was specified:
        //  - The directory where the parent object (form) was read from (set up in the application profile)
        //  - The application's path list (set up in the application profile) (the -p switch for QEGui)
        //  - The current directory
        //  - The environment variable QE_UI_PATH
        QStringList searchList;
        if(  QDir::isAbsolutePath( name ) )
        {
            searchList.append( name );
        }
        else
        {
            QFileInfo fileInfo;

            // Add the parent path from any parent QEForm
            QString parentPath =  profile->getParentPath();
            if( !parentPath.isEmpty() )
            {
                fileInfo.setFile( parentPath, name );
                searchList.append( fileInfo.filePath() );
            }

            // Add the paths from the path list in the container profile
            QStringList pathList = profile->getPathList();
            for( int i = 0; i < pathList.count(); i++ )
            {
                QString path = pathList[i];
                addPathToSearchList( path, name, searchList );
            }

            // Add paths from environment variable
            QStringList envPathList = profile->getEnvPathList();
            for( int i = 0; i < envPathList.count(); i++ )
            {
                addPathToSearchList( envPathList[i], name, searchList );
            }

            // Add the current directory
            fileInfo.setFile( QDir::currentPath(), name );
            searchList.append(  fileInfo.filePath() );
        }

        // Attempt to open the file
        QFile* uiFile = NULL;
        for( int i = 0; i < searchList.count(); i++ )
        {
            uiFile = new QFile( searchList[i] );
            if( uiFile->exists() )
                break;
            delete uiFile;
            uiFile = NULL;
        }
        return uiFile;
}

// Add a path and filename to a search list.
// If the path ends in '...' then add all the path's sub directories
// else, use the path as is.
// For example, assume /home/rhydera/adir and /home/rhydera/bdir are the only sub directories of /home/rhydera:
//    addPathToSearchList( "/home/rhydera/...", myFile.ui, searchList )
//       will add /home/rhydera/adir/myFile.ui and /home/rhydera/bdir/myFile.ui to the search list
//
//    addPathToSearchList( "/home/rhydera", myFile.ui, searchList )
//       will add /home/rhydera/myFile.ui to the search list
void QEWidget::addPathToSearchList( QString path, QString name, QStringList& searchList )
{
    QFileInfo fileInfo;

    // If path ends with ... add sub directories
    if( path.endsWith( "..." ) )
    {
        QString pathTop = path;
        pathTop.chop( 3 );
        QDir dir( pathTop );
        QFileInfoList contents = dir.entryInfoList ( QDir::AllDirs );
        for( int i = 0; i < contents.count(); i++ )
        {
            QFileInfo dirInfo = contents[i];
            // Add a search path for each sub directory.
            // Directories with no base name are ignored. These are '.' and '..'
            if( !dirInfo.baseName().isEmpty() )
            {
                fileInfo.setFile( dirInfo.absoluteFilePath(), name );
                searchList.append(  fileInfo.filePath() );
            }
        }
    }

    // If path does not end with ... add path
    else
    {
        fileInfo.setFile( path, name );
        searchList.append(  fileInfo.filePath() );
    }
}

// Returns the QE framework that built this instance of the widget.
QString QEWidget::getFrameworkVersion()
{
    return QE_VERSION_STRING " " QE_VERSION_DATE_TIME;
}

// Returns a string that will not change between runs of the application (given the same configuration)
QString QEWidget::persistantName( QString prefix )
{
    QString name = prefix;
    buildPersistantName( owner, name );
    return name;
}

// Returns a string that will not change between runs of the application (given the same configuration)
void QEWidget::buildPersistantName( QWidget* w, QString& name )
{
    // Stop when a QEForm is found with a unique identifier.
    // From this level up the application using the framework is responsible
    if( QString( "QEForm" ).compare( w->metaObject()->className() ) == false )
    {
        QEForm* form = (QEForm*)w;
        if( form->getUniqueIdentifier().isEmpty() == false )
        {
            name.prepend( "_" ).prepend( form->getUniqueIdentifier() );
            return;
        }
    }

    // If no parent, all done
    QWidget* p = w->parentWidget();
    if( !p )
    {
        return;
    }

    // Get the widget's sibling, add the widget's position in the list of
    // siblings to the persistant name, then repeat for the widget's parent.
    QObjectList c = p->children();
    int num = c.count();
    for( int i = 0; i < num; i++ )
    {
        if( c[i] == w )
        {
            buildPersistantName( p, name );
            name.append( QString( "_%1" ).arg( i ) );
            return;
        }
    }

    // Should never get here
    return;
}

//===========================================================================
// Class used to recieve save and restore signals from persistance manager.

// Constructor, destructor
signalSlotHandler::signalSlotHandler()
{
    owner = NULL;
}
signalSlotHandler::~signalSlotHandler()
{
}

// Set the owner of this class which will be called when a signal is received
void signalSlotHandler::setOwner( QEWidget* ownerIn )
{
    owner = ownerIn;
}

// A save or restore has been requested
void signalSlotHandler::saveRestore( SaveRestoreSignal::saveRestoreOptions option )
{
    // Sanity check
    if( !owner )
    {
        return;
    }

    // Get the persistance manager
    PersistanceManager* pm = owner->getPersistanceManager();
    if( !pm )
    {
        return;
    }

    // Get the QE widget to perform the appropriate action
    switch( option )
    {
        // Save the persistant widget data
        case SaveRestoreSignal::SAVE:
            owner->saveConfiguration( pm );
            break;

        // Restore the widget persistant data (application phase)
        // If the restore is being performed from QEGui there probably
        // won't be many QE widgets around at the start of the restore.
        // It is in this phase that QEGui will be creating the widgets.
        // This phase is still delivered to QEWidgets as they can be
        // used directly within an application, or unlike QEGui an
        // application may have already created QEWidgets.
        case SaveRestoreSignal::RESTORE_APPLICATION:
            owner->restoreConfiguration( pm, QEWidget::APPLICATION );
            break;

        // Restore the widget persistant data (framework phase)
        // If the restore is being performed from QEGui all the widgets
        // required will be created by now and be ready to collect and use their own persistant datra
        case SaveRestoreSignal::RESTORE_QEFRAMEWORK:
            owner->restoreConfiguration( pm, QEWidget::FRAMEWORK );
            break;
    }
}

// Get the QWidget that the parent of this QEWidget instance is based on.
// For example, the parent of a QEWidget might be a QELabel, which is based on QLabel which is based on QWidget.
QWidget* QEWidget::getQWidget()
{
    return owner;
}


// Find a QE widget and request an action.
// The widget hierarchy under a supplied widget is searched for a QE widget with a given name and optional title.
// If found the QE widget will attecjpt to carry out the requested action which consists of an action string and an argument list.
// This method allows an application to initiate QE widget activity. The QEGui application uses this mechanism when providing custom menus defined in XML files.
// The method returns true if the named widget was found. (The action was not nessesarily performed, or even recognised by the widget)
void QEWidget::doAction( QWidget* searchPoint, QString widgetName, QString action, QStringList arguments, bool initialise, QAction* originator )
{
    // Do nothing if no widget to search for is provided
    if( widgetName.isEmpty() )
    {
        return;
    }

    // Request the action of any matching widgets
    QList<QWidget*> targets = ((QObject*)searchPoint)->findChildren<QWidget*>( widgetName );
    for( int i = 0; i < targets.count(); i++)
    {
        QEWidget* qeWidget = dynamic_cast <QEWidget *>(targets.at(i));
        if (qeWidget)
        {
            qeWidget->actionRequest( action, arguments, initialise, originator );
        }
    }
}

// Return information about the data sources
// Used by PSI OPI viewer
// Example of use:
/*
        QEWidget* aWidgetBasedOnQEWidget = ...;
        QList<QCaInfo> info = aWidgetBasedOnQEWidget->getQCaInfo();
        qDebug() << "info" << info.count();
        for( int i = 0; i < info.count(); i++ )
        {
            qDebug() << "===================";
            qDebug() << info[i].variable;
            qDebug() << info[i].type;
            qDebug() << info[i].value;
            qDebug() << info[i].severity;
            qDebug() << info[i].status;
            qDebug() << info[i].host;
            qDebug() << info[i].precision;
            qDebug() << info[i].precisionUser;
            qDebug() << info[i].alarmUserMin;
            qDebug() << info[i].alarmUserMax;
            qDebug() << info[i].controlLimitLower;
            qDebug() << info[i].controlLimitUpper;
            qDebug() << info[i].alarmLimitLower;
            qDebug() << info[i].alarmLimitUpper;
            qDebug() << info[i].warningLimitLower;
            qDebug() << info[i].warningLimitUpper;
            qDebug() << info[i].driveLimitLow;
            qDebug() << info[i].driveLimitHigh;
            qDebug() << info[i].alarmSensitive;
            qDebug() << info[i].accessMode;
        }
*/

const QList<QCaInfo> QEWidget::getQCaInfo()
{
    QList<QCaInfo> list;

    qcaobject::QCaObject* qca;
    for( unsigned int i = 0; i < numVariables; i++ )
    {
        qca = getQcaItem( i );
        if( qca ) // If variable exists...
        {
            QCaInfo info(
                            qca->getRecordName(),               // variable
                            qca->getFieldType(),                // type
                            copyData().toString(),              // value
                            qca->getAlarmInfo().severityName(), // severity
                            qca->getAlarmInfo().statusName(),   // status
                            qca->getHostName(),                 // host
                            qca->getPrecision(),                // precision
                            getUserPrecision(),                 // user precision
                            getUserAlarmMin(),                  // user alarm minimum
                            getUserAlarmMax(),                  // user alarm maximum
                            qca->getControlLimitLower(),        // lower control limit
                            qca->getControlLimitUpper(),        // upper conmtrol limit
                            qca->getAlarmLimitLower(),          // lower alarm limit
                            qca->getAlarmLimitUpper(),          // upper alarm limit
                            qca->getWarningLimitLower(),        // lower warning limit
                            qca->getWarningLimitUpper(),        // upper warning limit
                            qca->getControlLimitLower(),        // lower control limit
                            qca->getControlLimitUpper(),        // upper control limit
                            getAlarmSensitive(),                // alarm sensitivity
                            QCaInfo::UNKNOWN );                 // Access mode

            list.append( info );
        }
    }
    return list;
}

using namespace qcastatemachine;

// Return references to the current count of disconnections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
int* QEWidget::getDisconnectedCountRef()
{
    return &ConnectionQCaStateMachine::disconnectedCount;
}

// Return references to the current count of connections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
int* QEWidget::getConnectedCountRef()
{
    return &ConnectionQCaStateMachine::connectedCount;
}
