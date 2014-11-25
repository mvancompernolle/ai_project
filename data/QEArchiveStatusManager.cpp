/*  QEArchiveStatusManager.cpp
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

#include <QtPlugin>

#include "QEArchiveStatus.h"
#include "QEArchiveStatusManager.h"

//------------------------------------------------------------------------------
//
QEArchiveStatusManager::QEArchiveStatusManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void QEArchiveStatusManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//------------------------------------------------------------------------------
//
bool QEArchiveStatusManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
//
QWidget *QEArchiveStatusManager::createWidget (QWidget * parent)
{
   return new QEArchiveStatus (parent);
}

//------------------------------------------------------------------------------
//
QString QEArchiveStatusManager::name () const
{
   return "QEArchiveStatus";
}

//------------------------------------------------------------------------------
//
QString QEArchiveStatusManager::group () const
{
   return "EPICSQt Infrastructure Widgets";
}

//------------------------------------------------------------------------------
//
QIcon QEArchiveStatusManager::icon () const
{
   return QIcon (":/qe/archive/archive_status.png");
}

//------------------------------------------------------------------------------
//
QString QEArchiveStatusManager::toolTip () const
{
   return "QE framework archive status";
}

//------------------------------------------------------------------------------
//
QString QEArchiveStatusManager::whatsThis () const
{
   return "QE framework archive status";
}

//------------------------------------------------------------------------------
//
bool QEArchiveStatusManager::isContainer () const
{
   return false;
}

//------------------------------------------------------------------------------
//
QString QEArchiveStatusManager::includeFile () const
{
   return "QEArchiveStatus.h";
}

// end
