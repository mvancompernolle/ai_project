/*  QEForm.cpp
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

/* This class is used as a container for QE widgets.

    It adds any variable name macro substitutions to the current environment profile, creates a form widget and
    reads a UI file which can contain QE widgets.

    As QE widgets are created, they note the current environment profile, including variable name substitutions.
    QE widgets also register themselves with this class so this class can activate them once they are fully created.
    QE widgets can't activate themselves. The Qt form loader creates each widget and calls the appropriate property
    functions to set it up. The widget itself does not know what properties are going to be set and when they have
    all been set. For this reason the QE widgets don't know when to request CA data. Both variable name properties
    and variable name substitution properties must be set up to request data and other properties may need to be set
    up before udates can be used.

    This class can be used directly (within a GUI application) as the top level form, or as a designer plugin class.
*/

#include <QUiLoader>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QPainter>
#include <QtDebug>
#include <QEScaling.h>
#include <QEForm.h>
#include <ContainerProfile.h>
#include <QEWidget.h>
#include <macroSubstitution.h>


// Constructor.
// No UI file is read. After construction uiFileName (and macroSubstitution) properties must be set and then QEForm::readUiFile() called.
// If this QEForm is itself a sub-form of some other QEForm, all properties will be set by the UI Loader as it reads the UI file for the parent form,
// then the QEForm::establishConnection() will be called by the QE widget 'activation' mechanism where each widget is 'activated' after a QEForm
// has been loaded by the UI Loader. For many QE widgets 'activation' means establish a CA connection to data sources. For
// QEForm widgets 'activation' means read the UI file.
QEForm::QEForm( QWidget* parent ) : QWidget( parent ), QEWidget( this )
{
    // Common construction.
    // Don't alert if UI is not found (it wont be as there isn't one specified) and
    // note that form won't be loaded manually. It will load automatically when QE widgets are 'activated' (when updates are initiated)
    commonInit( false, false );
}

// Constructor.
// UI filename is supplied and saved. No filename or macro substituition properties need to be set, and the caller must
// call QEForm::readUiFile() after construction to load the contents of the QEForm.
QEForm::QEForm( const QString& uiFileNameIn, QWidget* parent ) : QWidget( parent ), QEWidget( this )
{
    // Common construction
    // Alert if UI is not found and note that form will be loaded manually - it will not load automatically when QE widgets are 'activated' (when updates are initiated)
    commonInit( true, true );

    // Set up the filename during construction
    uiFileName = uiFileNameIn;
}

// Common construction
void QEForm::commonInit( const bool alertIfUINoFoundIn, const bool loadManuallyIn )
{
    // Set up the number of variables managed by the variable name manager.
    // NOTE: there is no data associated with this widget, but it uses the same mechanism as other data widgets to manage the UI filename and macro substitutions.
    // The standard variable name and macros mechanism is used by QEForm for UI file name and marcos
    setNumVariables(1);

    // Note if this QEForm widget will be manually loaded by calling QEForm::readUiFile()
    // If loadManually is set true, it will not load automatically when QE widgets are 'activated' (when updates are initiated)
    // If loadManually is set false, this QEForm widget will load itself when QE widgets are 'activated' (when updates are initiated)
    loadManually = loadManuallyIn;

    setAcceptDrops(true);

    ui = NULL;

    placeholderLabel = NULL;

    disconnectedCountRef = NULL;
    connectedCountRef = NULL;

    // If in designer mark up the form noting there is no file name set yet.
    // If not in designer, this will be done then establishConnection() is called.
    // This could be done all the time but QEForm would flash "No File Name" every time the form was loaded in QEGui.
    // This was done all the time without any problems when the file was loaded synchronously. Now establishConnection() loads the form as a timed event.
    if( inDesigner() )
    {
        displayPlaceholder( true, "No file name" );
    }

    alertIfUINoFound = alertIfUINoFoundIn;
    handleGuiLaunchRequests = false;
    resizeContents = true;

    // Set up the UserMessage class
    setFormFilter( MESSAGE_FILTER_MATCH );
    setSourceFilter( MESSAGE_FILTER_NONE );
    childMessageFormId = getNextMessageFormId();
    setChildFormId( childMessageFormId );

    // Setup a valid local profile if no profile was published
    if( !isProfileDefined() )
    {
        QStringList tempPathList;
        tempPathList.append( QDir::currentPath() );
        setupLocalProfile( this, tempPathList, "", "" );
    }

    // Altough we still monitor the file, we ignore chanhes unless ebabled.
    fileMonitoringIsEnabled = false;

    // Prepare to recieve notification that the ui file being displayed has changed
    QObject::connect( &fileMon, SIGNAL( fileChanged( const QString & ) ), this, SLOT( fileChanged( const QString & ) ) );


    // Set up a connection to recieve variable name property changes (Actually only interested in substitution changes
    QObject::connect( &variableNamePropertyManager, SIGNAL( newVariableNameProperty( QString, QString, unsigned int ) ),
                      this, SLOT( useNewVariableNameProperty( QString, QString, unsigned int) ) );
}

// Destructor.
QEForm::~QEForm()
{
    // Close any existing form
    if( ui )
        ui->close();
}

/*
    Start updating. (or in the case of QEForm widget, load the form)
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    Normally this function is used to initiate data updates when loaded as a plugin, but in the case of QEForm it uses the variable name as a file name
*/
void QEForm::establishConnection( unsigned int variableIndex )
{
    // Do nothing if form will be manually loaded by calling QEForm::readUiFile()
    if( loadManually )
        return;

    // Get the fully substituted variable name
    QString newFileName = getSubstitutedVariableName( variableIndex );

    // Mark up the widget if there is no file name available
    if( newFileName.isEmpty() )
    {
        displayPlaceholder( true, "No file name" );
        return;
    }

    // Load the form.
    // Avoid loading a form twice if file name has not changed. This is
    // especially important if forms are deeply nested causing the problem
    // to grow exponentially
    if( newFileName != uiFileName )
    {
        // Note the required filename and schedule it to be loaded once all events have been processed.
        // It may be loaded immediately by calling readUiFile() now, but this keeps things a bit more interactive.
        uiFileName = newFileName;
        QTimer::singleShot( 0, this, SLOT(reloadLater()));
    }
}

// Load the form once all events have been processed.
void QEForm::reloadLater()
{
    // Load the form
    readUiFile();

    // The ui file has been loaded post construction, so this widget missed out on any applied scaling.
    // Therefore apply the current scaling now.
    QEScaling::applyToWidget( ui );
}

// Debug function to list the widget hierarchy
//void showObjects( QObject* obj )
//{
//    static int depth = 0;

//    QWidget* w;
//    if( obj->isWidgetType() )
//    {
//        w = (QWidget*)obj;
//        qDebug() << depth << obj->metaObject()->className() << w->pos() << w->size();
//    }
//    QObjectList objList = obj->children();
//    depth++;
//    for( int i = 0; i < objList.size(); i++ )
//    {
//        showObjects( objList[i] );
//    }
//    depth--;
//}

// Read a UI file.
// The file read depends on the value of uiFileName
bool QEForm::readUiFile()
{
    // Close any pre-existing gui in the form
    if( ui )
    {
        delete ui;
        ui = NULL;
    }

    // Assume file is bad
    bool fileLoaded = false;

    // If no name has been provided...
    if (uiFileName.isEmpty())
    {
        displayPlaceholder( true, "No file name" );
    }

    // A name has been provided...
    else
    {
        // Set up the environment profile for any QE widgets created by the form
        QObject* savedGuiLaunchConsumer = NULL;

        // Try to open the UI file
        QString substitutedFileName =  substituteThis( uiFileName );
        QFile* uiFile = openQEFile( substitutedFileName, QIODevice::ReadOnly );

        // If the file was not found and opened, notify as appropriate
        if( !uiFile )
        {
            displayPlaceholder( true, QString( "Could not open " ).append( substitutedFileName ) );
            if( alertIfUINoFound )
            {
                QString msg;
                QTextStream(&msg) << "User interface file '" << substitutedFileName << "' could not be opened";
                sendMessage( msg, "QEForm::readUiFile", message_types ( MESSAGE_TYPE_WARNING) );
            }
        }

        // If the file was found and opened, load it
        else
        {
            // Get filename info
            QFileInfo fileInfo( uiFile->fileName() );

            // Note the full file path
            QDir uiDir;
            fullUiFileName = uiDir.cleanPath( uiDir.absoluteFilePath( uiFile->fileName() ) );

            // Ensure no other files are being monitored (belt and braces)
            QStringList monitoredPaths = fileMon.files();
            if( monitoredPaths.count())
            {
                fileMon.removePaths( monitoredPaths );
            }

            // Is this a resource file?
            bool isResourceFile = (fullUiFileName.left(1).compare( QString( ":" )) == 0);

            // Monitor the opened file (if not from the Qt resource database which can't be monitored)
            if( !isResourceFile )
            {
                fileMon.addPath( fullUiFileName );
            }

            // If profile has been published (for example by an application creating this form), then publish our own local profile
            bool localProfile = false;
            if( !isProfileDefined() )
            {
                // Flag the profile was set up in this function (and so should be released in this function)
                localProfile = true;

                publishOwnProfile();
            }

            // Add this form's macro substitutions for all it's children to use
            // Note, any macros in the substitutions are themselves substituted before applying the substitutions to the form
            addMacroSubstitutions( substituteThis( getVariableNameSubstitutions()) );

            // Temporarily update the published current object's path to the path of the form being created.
            // Any objects created within the form (for example sub forms) can then know where their parent form is located.
            setPublishedParentPath( fileInfo.absolutePath() );

            // If this form is handling form launch requests from object created within it, replace any form launcher with our own
            if( handleGuiLaunchRequests )
                savedGuiLaunchConsumer = replaceGuiLaunchConsumer( this );

            // Note the current published message form ID, and set up a new
            // message form ID for widgets created within this form.
            // This new message form ID will also be used when matching the
            // form ID of received messages
            unsigned int parentMessageFormId = getPublishedMessageFormId();
            setPublishedMessageFormId( childMessageFormId );

            // Flag the newly created QE widgets of this form should hold off activating
            // themselves (connecting) until the form has been fully loaded.
            // (Note what the value was before setting this flag - a parent form may have
            // set it and we should leave it as we found it)
            bool oldDontActivateYet = setDontActivateYet( true );

            // Clear any placeholder
            displayPlaceholder( false );

            // Load the gui
            QUiLoader loader;

            if( isResourceFile ) {
                // Just load it.
                //
                ui = loader.load( uiFile );
            } else {
                // This is a regular file.
                // Change the current directory to the directory holding the ui file before
                // loading the file: this is because when desiginer saves a ui file, embedded
                // file references (e.g. the icon file refrence in a QPushButton) are saved
                // relative to the location of the ui file.  Our best bet is that the relative
                // location of any reference file has been maintained from designer environment
                // to the deployed environment.
                //
                QString savedCurrentPath = QDir::currentPath();

                // Find fullUiFileName containing directory name.
                QString loaderPath = QFileInfo (fullUiFileName).dir().path ();

                QDir::setCurrent( loaderPath );
                ui = loader.load( uiFile );

                // Change directory back to where we were.
                QDir::setCurrent (savedCurrentPath);
            }
            uiFile->close();

            if( !ui )
            {
                // Load a placeholder as the ui file could not be loaded
                displayPlaceholder( true, QString( "Could not load " ).append( fullUiFileName ) );
            }

            // Set the window title (performing macro substitutions if required)
            setWindowTitle( uiFile->fileName() );

            // Reset the flag indicating newly created QE widgets of this form should hold off activating
            // themselves (connecting) until the form has been fully loaded.
            setDontActivateYet( oldDontActivateYet );

            // Restore the original published message form ID
            setPublishedMessageFormId( parentMessageFormId );

            // Remove this form's macro substitutions now all it's children are created
            removeMacroSubstitutions();

            // Reset the published current object's path to what ever it was
            setPublishedParentPath( getParentPath() );

            // If this form is handling form launch requests from object created within it, put back any original
            // form launcher now all objects have been created
            if ( handleGuiLaunchRequests )
                 replaceGuiLaunchConsumer( savedGuiLaunchConsumer );

            // Any QE widgets that have just been created need to be activated.
            // They can be activated now all the widgets have been loaded in this form.
            // Although they can be activated now, we will still hold off if widgets have been told 'dont activate yet'.
            // This is important for sub forms. Even though they can activate their contents now, it is likely their
            // filename and macro substitution properties will be set after creation which may cause many widgets
            // they contain to reconnect, which is inefficient.
            // When the top form (which set the 'dont activate yet' flag) completes loading it will activate all widgets.
            //
            // Note, this is only required when QE widgets are not loaded within a form and not directly by 'designer'.
            // When loaded directly by 'designer' they are activated (a CA connection is established) as soon as either
            // the variable name or variable name substitution properties are set
            if( !getDontActivateYet() )
            {
                QEWidget* containedWidget;
                while( (containedWidget = getNextContainedWidget()) )
                {
                    if( containedFrameworkVersion.isEmpty() )
                    {
                        containedFrameworkVersion = containedWidget->getFrameworkVersion();
                        disconnectedCountRef = containedWidget->getDisconnectedCountRef();
                        connectedCountRef = containedWidget->getConnectedCountRef();
                    }
                    containedWidget->activate();
                }
            }

            // If the published profile was published within this method, release it so nothing created later tries to use this object's services
            if( localProfile )
            {
                releaseProfile();
            }

            // If a .ui is present, manage resizing it
            if( ui )
            {
                // If the QEForm contents should take all its sizing clues from the QEForm, then set the top ui widget to match
                // the QEForm's size related properties.
                if( resizeContents )
                {
                    QRect formRect = ui->geometry();
                    ui->setGeometry( formRect.x(), formRect.y(), width(), height() );
                    ui->setSizePolicy( sizePolicy() );
                    ui->setMinimumSize( minimumSize() );
                    ui->setMaximumSize( maximumSize() );
                    ui->setSizeIncrement( sizeIncrement() );
                    ui->setBaseSize( baseSize() );
                }

                // If the form should take all its sizing clues from the form's contents, then set the form to match
                // the top ui widget's size related properties.
                else
                {
                    QRect formRect = geometry();
                    setGeometry( formRect.x(), formRect.y(), ui->width(), ui->height() );
                    setSizePolicy( ui->sizePolicy() );
                    setMinimumSize( ui->minimumSize() );
                    setMaximumSize( ui->maximumSize() );
                    setSizeIncrement( ui->sizeIncrement() );
                    setBaseSize( ui->baseSize() );
                }

                // Set the QEForm contents margin to zero (should always be zero already???) and rely on the QEForm's
                // contents for any margin decoration. For example, if the contents is a frame with a border.
                setContentsMargins( QMargins( 0, 0, 0, 0 ) );

                // Reset the user interface's position.
                // Not sure why, but the loaded user interface does not always have a position of 0,0
                // When debugged, the particular example was a QDialog with a position of 0,0 when viewed in designer.
                QRect uiRect = ui->geometry();
                ui->setGeometry(0, 0, uiRect.width(), uiRect.height());
            }

            // Load the user interface into the QEForm widget if present
            if( ui )
            {
                ui->setParent( this );
                ui->show();         // note, this show is only needed when replacing ui in existing QEForm
            }

            // If the ui is present and is managed by a layout, add a layout to the QEform (if not already present) and add
            // the ui to the layout so layout requests are passed down
            if( ui && ui->layout() )
            {
                QLayout* lo = layout();
                if( !lo )
                {
                    lo = new QVBoxLayout;
                    lo->setMargin( 0 );   // seamless and boarderless
                    setLayout( lo );
                }
                lo->addWidget( ui );
            }

            // Release the QFile
            delete uiFile;
            uiFile = NULL;
            fileLoaded = true;

// Debuging only  showObjects( this );

        }
    }
    return fileLoaded;
}

// Display or clear a placeholder.
// A place holder is placed in the form if the form cannot be populated.
// (Either no file name has been provided, or the file cannot be opened.)
void QEForm::displayPlaceholder( bool display, QString message )
{
    // Add a message...
    if( display)
    {
        if( !placeholderLabel )
        {
            // Create the label with the required text
            placeholderLabel = new QLabel( message, this );

            // Make sure the label is not drawn through when marking out the area of the QEForm
            placeholderLabel->setAutoFillBackground( true );

            // Present the new label
            placeholderLabel->show();

            // Force a paint event so the area of the blank QEForm will be shown
            update();
        }
        else
        {
            // Update the label text as the message has changed
            placeholderLabel->setText( message );
            placeholderLabel->adjustSize();
        }
    }

    // Remove a message...
    else
    {
        if( placeholderLabel )
        {
            delete placeholderLabel;
            placeholderLabel = NULL;
        }
    }

}

// Mark out the area of the form until the contents is populated by a .ui file
void QEForm::paintEvent(QPaintEvent * /* event */)
{
    // If the placeholder label is present (if a message saying no .ui file has been loaded) then
    // mark out the area of the QEForm
    if( placeholderLabel )
    {
        // Move the placeholder label away from the very corner so the border can be seen.
        // (This coudln't be done during when creating the label as the sizing was not valid yet)
        placeholderLabel->setGeometry( 1, 1, placeholderLabel->width(), placeholderLabel->height() );

        // Mark out the area of the QEForm
        QPainter painter( this );
        painter.drawLine( 0, 0, width(), height() );
        painter.drawLine( 0, height(), width(), 0 );
        painter.drawRect( 0, 0, width()-1, height()-1 );
    }
}

// Set the title to the name of the top level widget title, if it has one, or to the file name
void QEForm::setWindowTitle( QString filename )
{
    // Set the title to the name of the top level widget title, if it has one
    title.clear();
    if( ui )
    {
        QVariant windowTitleV = ui->property( "windowTitle" );

        if( windowTitleV.isValid() && windowTitleV.type() == QVariant::String )
        {
            QString windowTitle = windowTitleV.toString();
            if( !windowTitle.isEmpty() )
            {
                // Use the window title property as the title as long as it is not the default title.
                // (It is no use to have titles like 'Form' or 'Frame')
                // The default title is generally the class name minus the leading 'Q' (such as
                // 'Frame' for a QFrame), or for some classes (such as a QWidget) it is the word 'Form'
                QString defaultTitle = ui->metaObject()->className();
                if( windowTitle != defaultTitle.right( defaultTitle.size() - 1 ) &&
                    windowTitle != QString( "Dialog" ) &&
                    windowTitle != QString( "Form" ) )
                {
                    title = substituteThis( windowTitle );
                }
            }
        }
    }

    // If no title was obtained from the ui, use the file name
    if( title.isEmpty() )
    {
        // Extract the file name part used for the window title
        QFileInfo fileInfo( filename );
        title = QString( "QEGui " ).append( fileInfo.fileName() );
        if( title.endsWith( ".ui" ) )
            title.chop( 3 );
    }
}


// Set the form title.
// This is used to override any title set - through setWindowTitle() - when reading a ui file in readUiFile().
void QEForm::setQEGuiTitle( const QString titleIn )
{
    title = titleIn;
}

// Get the form title
QString QEForm::getQEGuiTitle()
{
    return title;
}

// Get the standard, absolute UI file name
QString QEForm::getFullFileName()
{
    return fullUiFileName;
}

// Reload the ui file
void QEForm::reloadFile()
{
    if( ui )
    {
        ui->close();
    }
    readUiFile();
    
    // The ui file has been loaded post construction, so this widget missed out on any applied scaling.
    // Therefore apply the current scaling now.
    QEScaling::applyToWidget( ui );
}

// Slot for reloading the file if it has changed.
// It doesn't matter if it has been deleted, a reload attempt will still tell the user what they need to know - that the file has gone.
void QEForm::fileChanged ( const QString & /*path*/ )
{
    // Only action if monitoring is enabled.
    if( fileMonitoringIsEnabled ){
        // Ensure we aren't monitoring files any more
        QStringList monitoredPaths = fileMon.files();
        fileMon.removePaths( monitoredPaths );

        // Reload the file
       reloadFile();
    }
}

// Receive new log messages.
// This widget doesn't do anything itself with messages, but it can regenerate the message as if it came from itself.
void QEForm::newMessage( QString msg, message_types type )
{
    // A QEForm deals with any message it receives from widgets it contains by resending it with its own form and source ids.
    // This way messages from widgets in QEForm widgets will be filtered as if they came from the form. This means a widget can
    // treat a sibling QEForm as a single message generating entity (and set up filters accordingly) and not worry about
    // exactly what widget within the form generated the message.
    sendMessage( msg, type );
}

// The form is being resized.
// Resize the ui to match.
// (not required if a layout is present)
void QEForm::resizeEvent ( QResizeEvent * event )
{
    event->ignore();
    // If the form's ui does not have a layout, resize it to match the QEForm
    // If it does have a layout, then the QEForm will also have given itself a
    // layout to ensure layout requests are propogated. In this case a resize is not nessesary.
    if( ui && !ui->layout() )
    {
        ui->resize( event->size() );
    }
}

// Get the version of the framework that loaded this form.
// Note this may vary within the same application.
// For example, QEGui may create a QEform programatically using the QE framework library it has loaded on startup,
// and then use Qt's UI loader to load a UI file containing a QEForm which is created by another version of the
// QE framework found by the UI Loader plugin location process.
QString QEForm::getContainedFrameworkVersion()
{
    return containedFrameworkVersion;
}

// Return the disconnected count of all widgets loaded by UILoader.
// Note, this originates from the a static counter in the QEPlugin shared library loaded by UILoader.
// If this QEForm widget has been loaded by UILoader, it could access these counters directly.
// In typical use, however, this QEForm widget can be created by an application (such as QEGui) and
// all widgets within this widgets (those with connections to be counted) will have been created by
// the same QEPlugin library but (on Windows at least) mapped to a different location. This is a problem on windows,
// not Linux where the library is not mapped twice.
int QEForm::getDisconnectedCount()
{
    // Return the disconnected count if it is available.
    return disconnectedCountRef?*disconnectedCountRef:0;
}

// Return the disconnected count of all widgets loaded by UILoader.
// Note, this originates from the a static counter in the QEPlugin shared library loaded by UILoader.
// If this QEForm widget has been loaded by UILoader, it could access these counters directly.
// In typical use, however, this QEForm widget can be created by an application (such as QEGui) and
// all widgets within this widgets (those with connections to be counted) will have been created by
// the same QEPlugin library but (on Windows at least) mapped to a different location. This is a problem on windows,
// not Linux where the library is not mapped twice.
int QEForm::getConnectedCount()
{
    // Return the connected count if it is available.
    return connectedCountRef?*connectedCountRef:0;
}

// Get the full form file name as used to open the file (inclusing all substitutions)
QString QEForm::getUiFileName()
{
    return fullUiFileName;
}

// Flag indicating if form should action (i.e. reload) ui file when ui file changes.
// Current set by qegui when edit menu item enabled. May it should/could be a property as well.
void QEForm::setFileMonitoringIsEnabled( bool fileMonitoringIsEnabledIn )
{
     fileMonitoringIsEnabled = fileMonitoringIsEnabledIn;
}

bool QEForm::getFileMonitoringIsEnabled()
{
    return fileMonitoringIsEnabled;
}

//==============================================================================
// Property convenience functions

// Flag indicating form should handle gui form launch requests
void QEForm::setHandleGuiLaunchRequests( bool handleGuiLaunchRequestsIn )
{
    handleGuiLaunchRequests = handleGuiLaunchRequestsIn;
}
bool QEForm::getHandleGuiLaunchRequests()
{
    return handleGuiLaunchRequests;
}

// Flag indicating form should resize contents to match form size (otherwise resize form to match contents)
void QEForm::setResizeContents( bool resizeContentsIn )
{
    resizeContents = resizeContentsIn;
}
bool QEForm::getResizeContents()
{
    return resizeContents;
}

// Save configuration
void QEForm::saveConfiguration( PersistanceManager* pm )
{
    // Add this form
    QString pname = persistantName( "QEForm" );
    PMElement f =  pm->addNamedConfiguration( pname );

    // Save macro substitutions
    QString macroSubs = getMacroSubstitutions();
    macroSubs = macroSubs.trimmed();
    if( !macroSubs.isEmpty() )
    {
        // Build a list of macro substitution parts from the string
        //!!! this won't be nessesary when the macroSubstitutionList class is used to hold macro substitutions instead of a string
        macroSubstitutionList parts = macroSubstitutionList( getMacroSubstitutions() );

        // Add a clean macro substitutionns string from the parts
        f.addValue( "MacroSubstitutions", parts.getString() );
//        f.addValue( "MacroSubstitutions", getMacroSubstitutions() );
    }

    // Save the path list
    QStringList pathList = getPathList();
    for( int i = 0; i < pathList.count(); i++ )
    {
        PMElement pl = f.addElement( "PathListItem" );
        pl.addAttribute( "Order", i );
        pl.addValue( QString( "Path" ), pathList.at( i ) );
    }
}

// Apply any saved configuration
void QEForm::restoreConfiguration( PersistanceManager* pm, restorePhases restorePhase )
{
    // Ignore application phase
    // (Actaully, it should not be called during the application phase as that is when it is created)
    if( restorePhase != FRAMEWORK )
    {
        return;
    }

    // Get data for this form, if any
    // (do nothing if no data)
    QString pname = persistantName( "QEForm" );
    PMElement f =  pm->getNamedConfiguration( pname );

    if( f.isNull() )
    {
        return;
    }

    // Restore the path list
    PMElementList pl = f.getElementList( "PathListItem" );
    QVector<QString> paths( pl.count() );
    for( int i = 0; i < pl.count(); i++ )
    {
        PMElement ple = pl.getElement( i );
        int order;
        if( ple.getAttribute( "Order", order ) )
        {
            QString path;
            if( ple.getValue( "Path", path ) )
            {
                paths[order] = path;
            }
        }
    }

    QStringList pathList;
    for( int i = 0; i < paths.size(); i++  )
    {
        pathList.append( paths[i] );
    }

    // Restore macro substitutions
    QString macroSubstitutions;
    f.getValue( "MacroSubstitutions", macroSubstitutions );

    // Determine if the environment this form was created in was the correct environment
    // (Was the macro substitutions and paths the same)
    bool environmentChanged = false;
    if( macroSubstitutions != getMacroSubstitutions() )
    {
        environmentChanged = true;
    }
    else
    {
        QStringList currentPathList = getPathList();
        if( currentPathList.count() != pathList.count() )
        {
            environmentChanged = true;
        }
        else
        {
            for( int i = 0; i < pathList.count(); i++ )
            {
                if( currentPathList[i] != pathList[i] )
                {
                    environmentChanged = true;
                    break;
                }
            }
        }
    }

    // Reload the file in the correct environment if the environment it was created in was not correct
    if( environmentChanged )
    {
        setupProfile( getGuiLaunchConsumer(), pathList, getParentPath(), macroSubstitutions );
        reloadFile();
        releaseProfile();
    }
}
