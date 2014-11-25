/*  QEArchiveNameSearch.cpp
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QRegExp>

#include <QECommon.h>
#include "QEArchiveNameSearch.h"

#define DEBUG  qDebug () << "QEArchiveNameSearch::" << __FUNCTION__ << __LINE__


//==============================================================================
//
QEArchiveNameSearch::QEArchiveNameSearch (QWidget* parent) : QEFrame (parent)
{
   this->archiveAccess = new QEArchiveAccess (this);
   this->createInternalWidgets ();

   // Use standard context menu
   //
   this->setupContextMenu ();

   QObject::connect (this->lineEdit, SIGNAL  (returnPressed       ()),
                     this,           SLOT    (searchReturnPressed ()));
}

//------------------------------------------------------------------------------
//
QEArchiveNameSearch::~QEArchiveNameSearch ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::search ()
{
   QString searchText;
   QStringList parts;
   QStringList matchingNames;

   searchText = this->lineEdit->text ().trimmed ();

   if (searchText.isEmpty ()) return;

   // TODO: Replace special reg exp characters (such as '.', '$' and '\' ) with the
   // escaped character sequences.

   // Spilt the patterns into parts.
   //
   parts = searchText.split (QRegExp ("\\s+"), QString::SkipEmptyParts);

   matchingNames.clear ();

   // Use each part to find a set of matching names, and then merge the list.
   //
   for (int p = 0; p < parts.count (); p++) {
      QString part = parts.value (p);
      QStringList partMatches;

      // QEArchiveAccess ensures the list is sorted.
      // Find nay names containing this string (and ignore case as well).
      //
      partMatches = QEArchiveAccess::getMatchingPVnames (part, Qt::CaseInsensitive);

      // Now nmerge the lists.
      //
      matchingNames = this->merge (matchingNames, partMatches);
   }

   // Use names to populate the list.
   //
   this->listWidget->clear ();
   this->listWidget->addItems (matchingNames);
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::searchReturnPressed ()
{
   this->search ();
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveNameSearch::getSelectedNames () const
{
   QList<QListWidgetItem*> itemList;
   QStringList result;
   int n;

   itemList = this->listWidget->selectedItems ();
   n = itemList.count ();
   for (int j = 0; j < n; j++) {
      QListWidgetItem* item = itemList.value (j);
      result.append (item->text ());
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::clear ()
{
   this->lineEdit->setText ("");
   this->listWidget->clear ();
}

//------------------------------------------------------------------------------
//
QVariant QEArchiveNameSearch::getDrop ()
{
   QVariant result;

   // Can only sensibly drag strings.
   //
   result = QVariant (this->getSelectedNames ().join (" "));
   return result;
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearch::copyVariable ()
{
   QString result;

   result = this->getSelectedNames ().join (" ");
   return result;
}

//------------------------------------------------------------------------------
//
QSize QEArchiveNameSearch::sizeHint () const
{
   return QSize (700, 260);
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::createInternalWidgets ()
{
   this->setMinimumSize (512, 212);

   this->verticalLayout = new QVBoxLayout (this);
   this->verticalLayout->setSpacing (4);
   this->verticalLayout->setContentsMargins (2, 4, 2, 2);

   this->searchFrame = new QFrame (this);
   this->searchFrame->setMinimumSize (QSize(0, 40));
   this->searchFrame->setFrameShape (QFrame::StyledPanel);
   this->searchFrame->setFrameShadow (QFrame::Raised);

   this->horizontalLayout = new QHBoxLayout (searchFrame);
   this->horizontalLayout->setSpacing (8);
   this->horizontalLayout->setContentsMargins (6, 4, 6, 4);

   this->lineEdit = new QLineEdit (searchFrame);
   this->lineEdit->setToolTip ("Enter partial PV names(s) and press return");

   this->horizontalLayout->addWidget (lineEdit);

   this->horizontalLayout->setContentsMargins (32, 4, 4, 4);

   this->verticalLayout->addWidget (searchFrame);

   this->listWidget = new QListWidget (this);
   this->listWidget->setMinimumSize (QSize (500, 156));
   this->listWidget->setMaximumSize (QSize (1000, 16777215));

   QFont font;
   font.setFamily (QString::fromUtf8 ("Monospace"));

   this->listWidget->setFont (font);
   this->listWidget->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->listWidget->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
   this->listWidget->setSelectionMode (QAbstractItemView::ExtendedSelection);
   this->listWidget->setUniformItemSizes (true);

   this->verticalLayout->addWidget (listWidget);

   this->listWidget->setCurrentRow (-1);
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveNameSearch::merge (const QStringList& a, const QStringList& b)
{
   const int an = a.size ();
   const int bn = b.size ();

   QStringList result;
   int ai, bi;
   QString as, bs;

   // Handle degenerate cases.
   //
   if (an == 0) return b;
   if (bn == 0) return a;

   // At least one item in each list.
   //
   ai = bi = 0;

   // While items remaining in both lists ....
   //
   while (ai < an && bi < bn) {

      as = a.value (ai);
      bs = b.value (bi);

      if (as < bs) {
         result << as;
         ai++;
      } else if (bs < as) {
         result << bs;
         bi++;
      } else {
         // we have a duplucate
         //
         result << as;
         ai++;
         bi++;
      }
   }

   // At least one of the list is empty - just copy whats left.
   // More trouble than it is worth to figure out which is the empty list.
   //
   while (ai < an) {
      result << a.value (ai);
      ai++;
   }

   while (bi < bn) {
      result << b.value (bi);
      bi++;
   }

   return result;
}

// end
