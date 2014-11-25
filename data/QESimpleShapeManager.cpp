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
 *  Copyright (c) 2011
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QESimpleShapeManager.h>
#include <QESimpleShape.h>
#include <QtPlugin>

/*
    ???
*/
QESimpleShapeManager::QESimpleShapeManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

/*
    ???
*/
void QESimpleShapeManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

/*
    ???
*/
bool QESimpleShapeManager::isInitialized () const
{
   return initialized;
}

/*
    Widget factory. Creates a QESimpleShape widget.
*/
QWidget *QESimpleShapeManager::createWidget (QWidget * parent)
{
   return new QESimpleShape (parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QESimpleShapeManager::name () const
{
   return "QESimpleShape";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QESimpleShapeManager::group () const
{
   return "EPICSQt Monitors";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QESimpleShapeManager::icon () const
{
   return QIcon (":/qe/simpleshape/QESimpleShape.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QESimpleShapeManager::toolTip () const
{
   return "EPICS Simple Shape";
}

/*
    ???
*/
QString QESimpleShapeManager::whatsThis () const
{
   return "EPICS Simple Shape";
}

/*
    ???
*/
bool QESimpleShapeManager::isContainer () const
{
   return false;
}

/*
    ???
*/
/*QString QESimpleShapeManager::domXml() const {
    return "<widget class=\"QESimpleShape\" name=\"qESimpleShape\">\n"
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
QString QESimpleShapeManager::includeFile () const
{
   return "QESimpleShape.h";
}

// end
