/*  QEStripChartManager.cpp
 *
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


#include <QEStripChartManager.h>
#include <QEStripChart.h>
#include <QtPlugin>

//------------------------------------------------------------------------------
//
QEStripChartManager::QEStripChartManager (QObject * parent) : QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void QEStripChartManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//------------------------------------------------------------------------------
//
bool QEStripChartManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
// Widget factory. Creates a QEStripChart widget.
//
QWidget *QEStripChartManager::createWidget (QWidget* parent)
{
   return new QEStripChart (parent);
}

//------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.
//
QString QEStripChartManager::name () const
{
   return "QEStripChart";
}

//------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.
//
QString QEStripChartManager::group () const
{
   return "EPICSQt Graphics";
}

//------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QEStripChartManager::icon () const
{
   return QIcon (":/qe/stripchart/QEStripChart.png");
}

//------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QEStripChartManager::toolTip () const
{
   return "EPICS PV/Archive Strip Chart Viewer";
}

//------------------------------------------------------------------------------
//
QString QEStripChartManager::whatsThis () const
{
   return "PV/Archive Strip Chart Viewer";
}

//------------------------------------------------------------------------------
//
bool QEStripChartManager::isContainer () const
{
   return false;  //???? true
}

//------------------------------------------------------------------------------
//
/*
 *QString QEStripChartManager::domXml() const {
    return "<widget class=\"QEStripChart\" name=\"qCaStripChart\">\n"
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

//------------------------------------------------------------------------------
//
QString QEStripChartManager::includeFile () const
{
   return "QEStripChart.h";
}

// end
