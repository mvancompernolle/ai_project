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
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * This class is used to manage the save/restore configurations
 */

#include "manageConfigDialog.h"
#include "ui_manageConfigDialog.h"
#include <QDebug>
#include "persistanceManager.h"

manageConfigDialog::manageConfigDialog( QStringList names, bool hasDefault, QWidget *parent ) :
    QDialog(parent),
    ui(new Ui::manageConfigDialog)
{
    ui->setupUi(this);
    ui->namesListWidget->addItems( names );
    ui->deletePushButton->setEnabled( false );
    ui->deleteDefaultPushButton->setEnabled( hasDefault );
}

manageConfigDialog::~manageConfigDialog()
{
    delete ui;
}

void manageConfigDialog::on_namesListWidget_itemSelectionChanged()
{
    ui->deletePushButton->setEnabled( ui->namesListWidget->selectedItems().count() );
}

void manageConfigDialog::on_deletePushButton_clicked()
{
    QStringList names;
    QList<QListWidgetItem*> list = ui->namesListWidget->selectedItems();
    for( int i = 0; i < list.count(); i++ )
    {
        names.append( list.at( i )->text() );
    }
    emit deleteConfigs( this, names );
    ui->namesListWidget->clear();
    ui->namesListWidget->addItems( currentNames );
}

void manageConfigDialog::setCurrentNames( QStringList currentNamesIn )
{
    currentNames = currentNamesIn;
}

void manageConfigDialog::on_deleteDefaultPushButton_clicked()
{
    QStringList names;
    names.append( PersistanceManager::defaultName );
    emit deleteConfigs( this, names );
    ui->deleteDefaultPushButton->setEnabled( false );
}
