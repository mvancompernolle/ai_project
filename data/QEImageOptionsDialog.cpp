/*  QEImageOptionsDialog.cpp
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
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages option selection for the QEImage widget
 */

#include "QEImageOptionsDialog.h"
#include "ui_QEImageOptionsDialog.h"

QEImageOptionsDialog::QEImageOptionsDialog(QWidget *parent) :
    QEDialog(parent),
    ui(new Ui::QEImageOptionsDialog)
{
    ui->setupUi(this);
}

// Initialise options.
// Set default values (emits optionChange() signal for each option).
// Note, can't be done during construction as the signals will not be connected yet.
void QEImageOptionsDialog::initialise()
{
    // Initial default settings
    optionSet( imageContextMenu::ICM_ENABLE_TIME,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL,         false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ,                  false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_LINE,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_AREA1,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA2,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA3,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA4,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_TARGET,               false );
    optionSet( imageContextMenu::ICM_ENABLE_BEAM,                 false );
    optionSet( imageContextMenu::ICM_DISPLAY_BUTTON_BAR,          false );
    optionSet( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES, false );
    optionSet( imageContextMenu::ICM_DISPLAY_RECORDER,            false );
}

QEImageOptionsDialog::~QEImageOptionsDialog()
{
    delete ui;
}

// Set an option in the dialog.
// Used when setting related properties
void QEImageOptionsDialog::optionSet( imageContextMenu::imageContextMenuOptions option, bool checked )
{
    switch( option )
    {
        case imageContextMenu::ICM_ENABLE_TIME:                 ui->checkBoxTime              ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_CURSOR_PIXEL:         ui->checkBoxInfo              ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ:                  ui->checkBoxHorizontalProfile ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT:                 ui->checkBoxVerticalProfile   ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_LINE:                 ui->checkBoxArbitraryProfile  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA1:                ui->checkBoxArea1Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA2:                ui->checkBoxArea2Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA3:                ui->checkBoxArea3Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA4:                ui->checkBoxArea4Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_TARGET:               ui->checkBoxTarget            ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_BEAM:                 ui->checkBoxBeam              ->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_BUTTON_BAR:          ui->checkBoxButtonBar         ->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES: ui->checkBoxBrightnessContrast->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_RECORDER:            ui->checkBoxRecorder          ->setChecked( checked ); break;

            // This switch should cater for all check boxes in the dialog, but this is not all context menu options
            // so include a default to keep compiler from generating warnings
        default: break;
    }

    // Act on the option change.
    emit optionChange( option, checked );
}

// Get a current setting in the dialog.
// Used when getting related properties.
bool QEImageOptionsDialog::optionGet( imageContextMenu::imageContextMenuOptions option )
{
    switch( option )
    {
        case imageContextMenu::ICM_ENABLE_TIME:                 return ui->checkBoxTime              ->isChecked();
        case imageContextMenu::ICM_ENABLE_CURSOR_PIXEL:         return ui->checkBoxInfo              ->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ:                  return ui->checkBoxHorizontalProfile ->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT:                 return ui->checkBoxVerticalProfile   ->isChecked();
        case imageContextMenu::ICM_ENABLE_LINE:                 return ui->checkBoxArbitraryProfile  ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA1:                return ui->checkBoxArea1Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA2:                return ui->checkBoxArea2Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA3:                return ui->checkBoxArea3Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA4:                return ui->checkBoxArea4Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_TARGET:               return ui->checkBoxTarget            ->isChecked();
        case imageContextMenu::ICM_ENABLE_BEAM:                 return ui->checkBoxBeam              ->isChecked();
        case imageContextMenu::ICM_DISPLAY_BUTTON_BAR:          return ui->checkBoxButtonBar         ->isChecked();
        case imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES: return ui->checkBoxBrightnessContrast->isChecked();
        case imageContextMenu::ICM_DISPLAY_RECORDER:            return ui->checkBoxRecorder->isChecked();

            // This switch should cater for all check boxes in the dialog, but this is not all context menu options
            // so include a default to keep compiler from generating warnings
        default: return 0;
    }
}

// Slots for acting on configuration check boxes
void QEImageOptionsDialog::on_checkBoxVerticalProfile_clicked   (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT,                      checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile_clicked (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ,                       checked ); }
void QEImageOptionsDialog::on_checkBoxArbitraryProfile_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_LINE,                      checked ); }
void QEImageOptionsDialog::on_checkBoxButtonBar_clicked         (bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_BUTTON_BAR,               checked ); }
void QEImageOptionsDialog::on_checkBoxBrightnessContrast_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES, checked ); }
void QEImageOptionsDialog::on_checkBoxRecorder_clicked          (bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_RECORDER,                 checked ); }
void QEImageOptionsDialog::on_checkBoxTime_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_TIME,                      checked ); }
void QEImageOptionsDialog::on_checkBoxInfo_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL,              checked ); }
void QEImageOptionsDialog::on_checkBoxArea1Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA1,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea2Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA2,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea3Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA3,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea4Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA4,                     checked ); }
void QEImageOptionsDialog::on_checkBoxTarget_clicked            (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_TARGET,                    checked ); }
void QEImageOptionsDialog::on_checkBoxBeam_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_BEAM,                      checked ); }
