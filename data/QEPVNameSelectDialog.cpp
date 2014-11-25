/*  QEPVNameSelectDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2012
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QUiLoader>

#include <QEPVNameSelectDialog.h>
#include <ui_QEPVNameSelectDialog.h>

#include <QEArchiveManager.h>
#include <QEScaling.h>

#define DEBUG  qDebug () << "QEPVNameSelectDialog::" << __FUNCTION__ << __LINE__

static const QString filterHelpFilename (":/qe/common/QEPVNameSelectFilterHelp.ui");

QWidget* QEPVNameSelectDialog::helpUi = NULL;

//------------------------------------------------------------------------------
//
QEPVNameSelectDialog::QEPVNameSelectDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPVNameSelectDialog)
{
   this->ui->setupUi (this);

   // Ensure the dialog centres using this widget as reference point.
   //
   this->setSourceWidget (this->ui->pvNameEdit);

   // Load help ui file - do this only once.
   // NOTE: We use loader directly rather than requesting the application (QEGui) to
   // do this for us. The help ui file contains static text, no EPICS aware widgets.
   //
   // The dialog is modal. Do we need a mutex??
   //
   if (!QEPVNameSelectDialog::helpUi) {
      QFile helpUiFile (filterHelpFilename);
      if (helpUiFile.open (QIODevice::ReadOnly)) {
         QUiLoader loader;

         QEPVNameSelectDialog::helpUi = loader.load (&helpUiFile, NULL);
         QEScaling::applyToWidget (QEPVNameSelectDialog::helpUi);
         helpUiFile.close ();
      }
   }

   this->returnIsMasked = false;

   // Initiate PV name retreval if needs be.
   // initialise () is idempotent.
   //
   QEArchiveAccess::initialise ();

   QObject::connect (this->ui->filterEdit,  SIGNAL  (returnPressed ()),
                     this,                  SLOT    (filterEditReturnPressed ()));

   QObject::connect (this->ui->filterEdit,  SIGNAL (editingFinished       ()),
                     this,                  SLOT   (filterEditingFinished ()));

   QObject::connect (this->ui->pvNameEdit,  SIGNAL (editTextChanged (const QString&)),
                     this,                  SLOT   (editTextChanged (const QString&)));

   QObject::connect (this->ui->helpButton,  SIGNAL (clicked       (bool)),
                     this,                  SLOT   (helpClicked   (bool)));

#ifndef QT_NO_COMPLETER
   // Could not get completer to work - yet.
   this->ui->pvNameEdit->setAutoCompletion (true);
   this->ui->pvNameEdit->setAutoCompletionCaseSensitivity (Qt::CaseSensitive);
#endif

}

//------------------------------------------------------------------------------
//
QEPVNameSelectDialog::~QEPVNameSelectDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::setPvName (QString pvNameIn)
{
   this->originalPvName = pvNameIn.trimmed ();
   this->ui->pvNameEdit->clear ();
   this->ui->pvNameEdit->insertItem (0, this->originalPvName, QVariant ());
   this->ui->pvNameEdit->setCurrentIndex (0);

   // setPvName typically invoked just before exec () call.
   // Maybe we should override exec?
   //
   this->ui->pvNameEdit->setFocus ();
   this->returnIsMasked = false;
}

//------------------------------------------------------------------------------
//
QString QEPVNameSelectDialog::getPvName ()
{
   return this->ui->pvNameEdit->currentText ().trimmed ();
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::applyFilter ()
{
   QString pattern = this->ui->filterEdit->text ().trimmed ();
   QRegExp regExp (pattern, Qt::CaseSensitive, QRegExp::RegExp);
   int n;

   this->ui->pvNameEdit->clear ();

   // QEArchiveAccess ensures the list is sorted.
   //
   this->ui->pvNameEdit->insertItems (0, QEArchiveAccess::getMatchingPVnames (regExp, true));

   n = this->ui->pvNameEdit->count ();
   if ((n == 0) && (!this->originalPvName.isEmpty ())) {
      this->ui->pvNameEdit->insertItem (0, this->originalPvName, QVariant ());
      this->ui->pvNameEdit->setCurrentIndex (0);
   }

   this->ui->matchCountLabel->setText (QString ("%1").arg (n));
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::filterEditReturnPressed ()
{
   // This return also pick up by on_buttonBox_accepted, mask this return.
   //
   this->returnIsMasked = true;

   // This will cause filterEditingFinished to be invoked - no need
   // to apply filter here.
   //
   this->ui->pvNameEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::filterEditingFinished ()
{
   this->applyFilter ();
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::editTextChanged (const QString&)
{
   // NOTE: calling buttonBox->setStandardButtons causes a seg fault when cancel
   // eventually pressed and sometimes okay button as well, so do nothing for now.
   // Maybe do our own buttons instead of using a QDialogButtonBox.
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::helpClicked (bool /* checked */ )
{
   if (QEPVNameSelectDialog::helpUi) {
      QEPVNameSelectDialog::helpUi->show ();
   }
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::closeHelp ()
{
   if (QEPVNameSelectDialog::helpUi) {
      QEPVNameSelectDialog::helpUi->close ();
   }
}

//------------------------------------------------------------------------------
// User close closed the dialog.
//
void QEPVNameSelectDialog::closeEvent (QCloseEvent * event)
{
   this->closeHelp ();
   QEDialog::closeEvent (event);
}

//------------------------------------------------------------------------------
// User has pressed OK (or return)
//
void QEPVNameSelectDialog::on_buttonBox_accepted ()
{
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   if (!this->getPvName().isEmpty ()) {
      this->closeHelp ();
      this->accept ();
   }
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPVNameSelectDialog::on_buttonBox_rejected ()
{
   this->closeHelp ();
   this->close ();
}

// end
