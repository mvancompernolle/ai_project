/*
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEPvPropertiesManager.h>
#include <QEPvProperties.h>
#include <QtPlugin>

/*
    ???
*/
QEPvPropertiesManager::QEPvPropertiesManager (QObject * parent) : QObject (parent)
{
   initialized = false;
}

/*
    ???
*/
void QEPvPropertiesManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

/*
    ???
*/
bool QEPvPropertiesManager::isInitialized () const
{
   return initialized;
}

/*
    Widget factory. Creates a QEPvProperties widget.
*/
QWidget *QEPvPropertiesManager::createWidget (QWidget * parent)
{
   return new QEPvProperties (parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QEPvPropertiesManager::name () const
{
   return "QEPvProperties";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QEPvPropertiesManager::group () const
{
   return "EPICSQt Tools";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QEPvPropertiesManager::icon () const
{
   return QIcon (":/qe/pvproperties/QEPvProperties.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QEPvPropertiesManager::toolTip () const
{
   return "Shows EPICS IOC record fields";
}

/*
    ???
*/
QString QEPvPropertiesManager::whatsThis () const
{
   return "Shows EPICS IOC record fields";
}

/*
    ???
*/
bool QEPvPropertiesManager::isContainer () const
{
   return false;  //???? true
}

/*
    ???
*/
/*QString QEPvPropertiesManager::domXml() const {
    return "<widget class=\"QEPvProperties\" name=\"qCaPvProperties\">\n"
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

/*
    ???
*/
QString QEPvPropertiesManager::includeFile () const
{
   return "QEPvProperties.h";
}

// end
