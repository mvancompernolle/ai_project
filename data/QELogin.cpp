/*  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2012, 2013
 *
 *  Author:
 *    Ricardo Fernandes, Andrew Rhyder
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

/*
  This class manages the current user type (USER, SCIENTIST, ENGINEER) for the QE framework and applications
  It uses passwords defined by the application, or if absent by its own properties.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QGridLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QELogin.h>

// ============================================================
//  QELogin class.

QELogin::QELogin(QWidget *pParent):QFrame(pParent), QEWidget( this )
{
    // Presentation of the frame
    setFrameStyle( QFrame::StyledPanel );
    setFrameShadow( QFrame::Raised );

    // Create the widgets that make up a QELogin widget
    qLabelUserType = new QLabel(this);
    loginForm = new loginWidget( this );
    qPushButtonLogin = new QPushButton(this);
    qPushButtonLogout = new QPushButton(this);

    // Arrange the widget
    QGridLayout* qGridLayout = new QGridLayout(this);
    qGridLayout->addWidget( qLabelUserType, 0, 0, 1, 2 );
    qGridLayout->addWidget( loginForm, 1, 0, 1, 2 );
    qGridLayout->addWidget( qPushButtonLogin, 2, 0);
    qGridLayout->addWidget( qPushButtonLogout, 2, 1);

    // Assume compact style
    setCompactStyle( true );
    setStatusOnly( false );

    qLabelUserType->setToolTip("Current user");

    // Set up login button
    qPushButtonLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    qPushButtonLogin->setText("Login");
    qPushButtonLogin->setToolTip("Change current user");
    QObject::connect(qPushButtonLogin, SIGNAL(clicked()), this, SLOT(buttonLoginClicked()));

    // Set up logout button
    qPushButtonLogout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    qPushButtonLogout->setText("Logout");
    qPushButtonLogout->setToolTip("Logout current user");
    qPushButtonLogout->setEnabled(false);
    QObject::connect(qPushButtonLogout, SIGNAL(clicked()), this, SLOT(buttonLogoutClicked()));

    // Set initial state
    setCurrentLevelText();
}

// Virtual function implementation called when the user level changes
void QELogin::userLevelChanged( userLevelTypes::userLevels )
{
    setCurrentLevelText();
}

// Set up current level text
void QELogin::setCurrentLevelText()
{
    qLabelUserType->setText(QString( "Current Level: " ).append( getUserTypeName( getUserLevel() ) ));
}

// Compact mode property fuctions
// Set if the widget is small and pops up a dialog to allow login,
// or larger and displays the login all the time
void QELogin::setCompactStyle(bool compactStyleIn )
{
    compactStyle = compactStyleIn;
    loginForm->setHidden( compactStyle || statusOnly );
}

bool QELogin::getCompactStyle()
{
    return compactStyle;
}

// Status only property fuctions
void QELogin::setStatusOnly( bool statusOnlyIn )
{
    statusOnly = statusOnlyIn;
    loginForm->setHidden( compactStyle || statusOnly );
    qPushButtonLogin->setHidden( statusOnly );
    qPushButtonLogout->setHidden( statusOnly );
}

bool QELogin::getStatusOnly()
{
    return statusOnly;
}

// User password property fuctions
void QELogin::setUserPassword(QString pValue)
{
    userPassword = pValue;
}

QString QELogin::getUserPassword()
{
    return userPassword;
}

// Scientist password property fuctions
void QELogin::setScientistPassword(QString pValue)
{
    scientistPassword = pValue;
}

QString QELogin::getScientistPassword()
{
    return scientistPassword;
}

// Engineer password property fuctions
void QELogin::setEngineerPassword(QString pValue)
{
    engineerPassword = pValue;
}

QString QELogin::getEngineerPassword()
{
    return engineerPassword;
}

// Return a name for each user type
QString QELogin::getUserTypeName(userLevelTypes::userLevels type)
{
    switch( type )
    {
        case userLevelTypes::USERLEVEL_USER:      return "User";
        case userLevelTypes::USERLEVEL_SCIENTIST: return "Scientist";
        case userLevelTypes::USERLEVEL_ENGINEER:  return "Engineer";
        default:                                  return "Unknown";
    }
}

// The login button has been clicked
void QELogin::buttonLoginClicked()
{
    // For compact style, present the login dialog
    if( compactStyle )
    {
        QELoginDialog qELoginDialog( this );
        qELoginDialog.exec();
    }

    // For non compact style, try to login with the selected user type and password
    else
    {
        login( loginForm->getUserType(), loginForm->getPassword() );
        loginForm->clearPassword();
    }
}

// Try to login with a selected user type and password.
// This is called by the QEWidget directly, or from the QEWidget's login dialog
bool QELogin::login( userLevelTypes::userLevels level, QString password )
{
    // Get the required password
    QString requiredPassword;
    switch( level )
    {
        case userLevelTypes::USERLEVEL_USER:      requiredPassword = getPriorityUserPassword();      break;
        case userLevelTypes::USERLEVEL_SCIENTIST: requiredPassword = getPriorityScientistPassword(); break;
        case userLevelTypes::USERLEVEL_ENGINEER:  requiredPassword = getPriorityEngineerPassword();  break;
    }

    // If the password is OK, change the user type
    if( requiredPassword.isEmpty() || password == requiredPassword )
    {
        // Change user level if required
        userLevelTypes::userLevels currentLevel = getUserLevel();
        if( level != currentLevel )
        {
            sendMessage("The user type was changed from '" + getUserTypeName( currentLevel ) + "' to '" + getUserTypeName( level ) + "'");
            loginHistory.push( currentLevel );
            setUserLevel( level);
            setCurrentLevelText();
            qPushButtonLogout->setEnabled( true );
        }

        // Signal a successfull login has occured
        // This is usefull to for closing a dialog this widget may be a part of
        emit login();

        // Indicate successfull login
        return true;
    }

    // bad password, tell the user
    QMessageBox::critical(this, "Error", "The password is invalid. Please try again!");
    return false;
}

// Logout to the last user type logged into by THIS widget
void QELogin::buttonLogoutClicked()
{
    // If this widget has any record of previous user levels, then logout
    if( loginHistory.count() )
    {
        // Logout
        userLevelTypes::userLevels currentLevel = getUserLevel();
        sendMessage("The user type was changed from '" + getUserTypeName( currentLevel ) + "' to '" + getUserTypeName( loginHistory.top() ) + "'");
        setUserLevel( loginHistory.pop() );
        setCurrentLevelText();

        // If no more history, disable the logout button
        if( loginHistory.count() == 0 )
        {
            qPushButtonLogout->setEnabled( false );
        }
    }
}

// Return the user level password from the profile if available, otherwise use the local user level password property
QString QELogin::getPriorityUserPassword()
{
    ContainerProfile profile;
    if( profile.areUserLevelPasswordsSet() )
    {
        return profile.getUserLevelPassword( userLevelTypes::USERLEVEL_USER );
    }
    else
    {
        return userPassword;
    }
}

// Return the scientist level password from the profile if available, otherwise use the local scientist level password property
QString QELogin::getPriorityScientistPassword()
{
    ContainerProfile profile;
    if( profile.areUserLevelPasswordsSet() )
    {
        return profile.getUserLevelPassword( userLevelTypes::USERLEVEL_SCIENTIST );
    }
    else
    {
        return scientistPassword;
    }
}

// Return the engineer level password from the profile if available, otherwise use the local engineer level password property
QString QELogin::getPriorityEngineerPassword()
{
    ContainerProfile profile;
    if( profile.areUserLevelPasswordsSet() )
    {
        return profile.getUserLevelPassword( userLevelTypes::USERLEVEL_ENGINEER );
    }
    else
    {
        return engineerPassword;
    }
}

// ============================================================
// loginWidget
// This widget is used in the dialog launched by the compact form of QELogin,
// and by the larger form of QELogin directly.

loginWidget::loginWidget( QELogin* ownerIn )
{
    // Note the QELogin widget
    owner = ownerIn;

    // Create the widgets making up the login form
    QGroupBox*   qGroupBox = new QGroupBox(this);
    qRadioButtonUser = new QRadioButton();
    qRadioButtonScientist = new QRadioButton(this);
    qRadioButtonEngineer = new QRadioButton(this);
    qLineEditPassword = new QLineEdit(this);

    // Set up the widgets
    qGroupBox->setTitle( "Login as:");

    qRadioButtonUser->setText(owner->getUserTypeName(userLevelTypes::USERLEVEL_USER));
    QObject::connect(qRadioButtonUser, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

    qRadioButtonScientist->setText(owner->getUserTypeName(userLevelTypes::USERLEVEL_SCIENTIST));
    QObject::connect(qRadioButtonScientist, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

    qRadioButtonEngineer->setText(owner->getUserTypeName(userLevelTypes::USERLEVEL_ENGINEER));
    QObject::connect(qRadioButtonEngineer, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

    qLineEditPassword->setEchoMode(QLineEdit::Password);
    qLineEditPassword->setToolTip("Password for the selected type");

    // Layout the form
    QVBoxLayout* qVBoxLayout = new QVBoxLayout();
    qVBoxLayout->addWidget(qRadioButtonUser);
    qVBoxLayout->addWidget(qRadioButtonScientist);
    qVBoxLayout->addWidget(qRadioButtonEngineer);
    qGroupBox->setLayout(qVBoxLayout);

    QGridLayout* qGridLayout = new QGridLayout(this);
    qGridLayout->addWidget(qGroupBox, 0, 0);
    qGridLayout->addWidget(qLineEditPassword, 1, 0);

    // Select a radio button to match the current user level
    switch( owner->getUserLevel() )
    {
        case userLevelTypes::USERLEVEL_USER:        qRadioButtonUser->setChecked(true);      break;
        case userLevelTypes::USERLEVEL_SCIENTIST:   qRadioButtonScientist->setChecked(true); break;
        case userLevelTypes::USERLEVEL_ENGINEER:    qRadioButtonEngineer->setChecked(true);  break;
    }

    // Enable or disable the password according to the user type
    radioButtonClicked();
}

// A user type has been selected.
// Enable or disable the password according to the user type
void loginWidget::radioButtonClicked()
{
    // Enable password entry if a password is required for the selected user type
    if (qRadioButtonUser->isChecked())
    {
        qLineEditPassword->setEnabled(owner->getPriorityUserPassword().isEmpty() == false);
    }
    else if (qRadioButtonScientist->isChecked())
    {
        qLineEditPassword->setEnabled(owner->getPriorityScientistPassword().isEmpty() == false);
    }
    else
    {
        qLineEditPassword->setEnabled(owner->getPriorityEngineerPassword().isEmpty() == false);
    }
}

// Return the user type selected by the radio button group
userLevelTypes::userLevels loginWidget::getUserType()
{
    if( qRadioButtonUser->isChecked() )
        return userLevelTypes::USERLEVEL_USER;

    if( qRadioButtonScientist->isChecked() )
        return userLevelTypes::USERLEVEL_SCIENTIST;

    if( qRadioButtonEngineer->isChecked() )
        return userLevelTypes::USERLEVEL_ENGINEER;

    // Default
    return userLevelTypes::USERLEVEL_USER;
}

// Return the password entered by the user
QString loginWidget::getPassword()
{
    return qLineEditPassword->text();
}

// Clear the password entered by the user
void loginWidget::clearPassword()
{
    qLineEditPassword->clear();
}

// ============================================================
// QELoginDialog widget
// This widget is used when the QELogin widget is in compact form to present the login options.

QELoginDialog::QELoginDialog(QELogin* ownerIn)
{
    // Remove maximise and minimise buttons
    // !!! doesn't work on linux???
    setWindowFlags( Qt::Dialog );

    // Note the QELogin widget
    owner = ownerIn;

    // Create the widgets making up the login dialog
    loginForm = new loginWidget(owner);
    qPushButtonOk = new QPushButton(this);
    qPushButtonCancel = new QPushButton(this);

    // Set up the widgets
    setWindowTitle("Login/Logout");

    qPushButtonOk->setText("Ok");
    qPushButtonOk->setToolTip("Perform login");
    QObject::connect(qPushButtonOk, SIGNAL(clicked()), this, SLOT(buttonOkClicked()));

    qPushButtonCancel->setText("Cancel");
    qPushButtonCancel->setToolTip("Cancel login");
    QObject::connect(qPushButtonCancel, SIGNAL(clicked()), this, SLOT(buttonCancelClicked()));

    // Layout the form
    QGridLayout* qGridLayout = new QGridLayout(this);
    qGridLayout->addWidget( loginForm, 0, 0, 1, 2 );
    qGridLayout->addWidget( qPushButtonCancel, 1, 0);
    qGridLayout->addWidget( qPushButtonOk, 1, 1);
}

// Dialog OK clicked
void QELoginDialog::buttonOkClicked()
{
    // Attempt to login and close the dialog if successfull
    if( owner->login( loginForm->getUserType(), loginForm->getPassword() ) )
    {
        close();
    }
}

// Dialog cancel clicked
void QELoginDialog::buttonCancelClicked()
{
    close();
}
