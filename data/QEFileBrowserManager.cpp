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
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QEFileBrowserManager.h>
#include <QEFileBrowser.h>



QEFileBrowserManager::QEFileBrowserManager(QObject *pParent):QObject(pParent)
{

    initialized = false;

}



void QEFileBrowserManager::initialize(QDesignerFormEditorInterface *)
{

    if (initialized == false)
    {
        initialized = true;
    }

}



bool QEFileBrowserManager::isInitialized() const
{

    return initialized;

}



QWidget *QEFileBrowserManager::createWidget(QWidget *pParent)
{

    return new QEFileBrowser(pParent);
}



QString QEFileBrowserManager::name() const
{

    return "QEFileBrowser";

}



QString QEFileBrowserManager::group() const
{

    return "EPICSQt Controls";

}



QIcon QEFileBrowserManager::icon() const
{

    return QIcon(":/qe/filebrowser/QEFileBrowser.png");

}



QString QEFileBrowserManager::toolTip() const
{

    return "EPICS File Browser";

}



QString QEFileBrowserManager::whatsThis() const
{

    return "EPICS File Browser";

}



bool QEFileBrowserManager::isContainer() const
{

    return false;
}



QString QEFileBrowserManager::includeFile() const
{

    return "QEFileBrowserManager.h";

}




/*QString QEFileBrowserManager::domXml() const {
    return "<widget class=\"QEFileBrowser\" name=\"qEFileBrowser\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>100</width>\n"
           "   <height>100</height>\n"
           "  </rect>\n"
           " </property>\n"
           " <property name=\"toolTip\" >\n"
           "  <string></string>\n"
           " </property>\n"
           " <property name=\"whatsThis\" >\n"
           "  <string> "
           ".</string>\n"
           " </property>\n"
           "</widget>\n";
}*/


