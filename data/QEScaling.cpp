/*  QEScaling.cpp
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
 *
 */

#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QHeaderView>
#include <QSize>
#include <QTableWidget>
#include <QTreeView>
#include <QWidget>

#include <QEResizeableFrame.h>
#include <QEWidget.h>
#include <QECommon.h>

#include "QEScaling.h"

//------------------------------------------------------------------------------
//
int QEScaling::currentScaleM = 1;
int QEScaling::currentScaleD = 1;

//------------------------------------------------------------------------------
// Euclidean algorithm for computing greatest common divisor of two integers.
// Ref: Hand Book of Applied Crytpgraphy (1997), p66, sec 2.104
//
static int gcd (int a, int b)
{
   int r;

   while (b) {
      r = a % b;
      a = b;
      b = r;
   }
   return a;
}

//------------------------------------------------------------------------------
//
void QEScaling::setScaling (const int m, const int d)
{
   // sanity check - m and d both positive.
   //
   if ((m > 0) && (d > 0)) {
      // Normalise rational number.
      //
      int g = gcd (m, d);
      QEScaling::currentScaleM = m/g;
      QEScaling::currentScaleD = d/g;
   }
}

//------------------------------------------------------------------------------
//
void QEScaling::getScaling (int& m, int& d)
{
   m = QEScaling::currentScaleM;
   d = QEScaling::currentScaleD;
}

//------------------------------------------------------------------------------
//
void QEScaling::widgetScale (QWidget* widget)
{
   QLabel* label = NULL;
   QEWidget* qeWidget = NULL;
   QLayout* layout = NULL;
   QEResizeableFrame* resizeableFrame = NULL;
   QTableWidget* tableWidget = NULL;
   QTreeView* treeView = NULL;

   // sainity check.
   //
   if (!widget) return;

   QSize minSize = widget->minimumSize();
   QSize maxSize = widget->maximumSize();
   QRect geo = widget->geometry();
   QWidget *parent;

   minSize.setWidth  (QEScaling::scale (minSize.width ()));
   minSize.setHeight (QEScaling::scale (minSize.height ()));

   // QWIDGETSIZE_MAX is the default max size - do not scale nor exceed this value.
   //
   if (maxSize.width () != QWIDGETSIZE_MAX) {
      maxSize.setWidth  (MIN (QEScaling::scale (maxSize.width ()), QWIDGETSIZE_MAX));
   }
   if (maxSize.height () != QWIDGETSIZE_MAX) {
      maxSize.setHeight (MIN (QEScaling::scale (maxSize.height ()), QWIDGETSIZE_MAX));
   }

   geo = QRect (QEScaling::scale (geo.left ()),
                QEScaling::scale (geo.top ()),
                QEScaling::scale (geo.width ()),
                QEScaling::scale (geo.height ()));

   if (QEScaling::currentScaleM >= QEScaling::currentScaleD) {
      // getting bigger - ensure consistancy - do max size constraint first.
      //
      widget->setMaximumSize (maxSize);
      widget->setMinimumSize (minSize);
   } else {
      // getting smaller - to min size constraint first.
      //
      widget->setMinimumSize (minSize);
      widget->setMaximumSize (maxSize);
   }

   widget->setGeometry (geo);

   parent = dynamic_cast <QWidget *>(widget->parent ());

   // If a child's font same as parents then is scaled auto-magically
   // when the parent's font was scaled, and if we do it again it will
   // get scalled twice. And the font of a grand-child item will be
   // scaled three times etc. So only do font scale if no parent or this
   // widget is not using its parent font.
   //
   if (!parent || (widget->font() != parent->font())) {
      QFont font = widget->font();
      int pointSize = font.pointSize ();
      int pixelSize = font.pixelSize ();

      if (pointSize >= 0) {
         // Font point sizes must me at least one.
         font.setPointSize (MAX (1, QEScaling::scale (pointSize)));
      }
      else if (pixelSize >= 0) {
         font.setPixelSize (MAX (1, QEScaling::scale (pixelSize)));
      }
      widget->setFont (font);
   }

   // Check if there is a layout
   //
   layout = widget->layout ();
   if (layout) {
       int margin [5];   // left, top, right bottom, spacing
       int j;

       layout->getContentsMargins (&margin [0], &margin [1],&margin [2],&margin [3]);
       margin [4] = layout->spacing ();

       for (j = 0; j < 5; j++) {
          if (margin [j] > 0) {
             margin [j] = QEScaling::scale (margin [j]);
          }
       }

       layout->setContentsMargins (margin [0], margin [1],margin [2], margin [3]);
       layout->setSpacing (margin [4]);
   }

   // Specials.
   // Q? How expensive are dynamic castes? Use Qt's own caste?
   //    Leverage off some items being mutually exclusive.
   //
   label = dynamic_cast <QLabel*>(widget);
   if (label) {
      int indent = label->indent ();

      if (indent > 0) {
         indent = QEScaling::scale (indent);
         label->setIndent (indent);
      }
   }

   resizeableFrame = dynamic_cast <QEResizeableFrame*>(widget);
   if (resizeableFrame) {
      int allowedMin = resizeableFrame->getAllowedMinimum ();
      int allowedMax = resizeableFrame->getAllowedMaximum ();

      // scale
      allowedMin = QEScaling::scale (allowedMin);
      allowedMax = QEScaling::scale (allowedMax);

      if (QEScaling::currentScaleM >= QEScaling::currentScaleD) {
         // getting bigger - ensure consistancy - do max size constraint first.
         //
         resizeableFrame->setAllowedMaximum (allowedMax);
         resizeableFrame->setAllowedMinimum (allowedMin);
      } else {
         // getting smaller - to min size constraint first.
         //
         resizeableFrame->setAllowedMinimum (allowedMin);
         resizeableFrame->setAllowedMaximum (allowedMax);
      }
   }

   tableWidget = dynamic_cast <QTableWidget *>(widget);
   if (tableWidget) {
      int defaultSectionSize;

      defaultSectionSize = tableWidget->horizontalHeader ()->defaultSectionSize ();
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->horizontalHeader ()->setDefaultSectionSize (defaultSectionSize);

      defaultSectionSize = tableWidget->verticalHeader ()->defaultSectionSize ();
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->verticalHeader ()->setDefaultSectionSize (defaultSectionSize);
   }

   treeView = dynamic_cast <QTreeView *>(widget);
   if (treeView) {
      int indentation = treeView->indentation ();

      if (indentation > 0) {
         indentation = QEScaling::scale (indentation);
         treeView->setIndentation (indentation);
      }
   }

   qeWidget = dynamic_cast <QEWidget *>(widget);
   if (qeWidget) {
      // For QEWidget objects, scaleBy is virtual function. This allows geometrically
      // complicated widgets, such as QEShape, to provide a bespoke scaling function.
      //
      qeWidget->scaleBy (QEScaling::currentScaleM, QEScaling::currentScaleD);
   }
}

//------------------------------------------------------------------------------
//
void QEScaling::widgetTreeWalk (QWidget* widget, const int maxDepth)
{
   int j, n;
   QObjectList childList;
   QObject* child = NULL;
   QWidget* childWidget = NULL;

   // sainity checks and avoid divide by zero.
   //
   if (!widget) return;
   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling
   if (maxDepth < 0) return;

   // Apply scaling to this widget.
   //
   QEScaling::widgetScale (widget);

   // Apply scaling to any child widgets.
   //
   childList = widget->children ();
   n = childList.count();
   for (j = 0; j < n; j++) {
      child = childList.value (j);
      // We need only tree walk widgets. All widget parents are themselves widgets.
      //
      childWidget = dynamic_cast <QWidget *>(child);
      if (childWidget) {
         // Recursive call.
         //
         QEScaling::widgetTreeWalk (childWidget, maxDepth - 1);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEScaling::applyToWidget (QWidget* widget, const int maxDepth)
{
   if (!widget) return;
   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling

   QEScaling::widgetTreeWalk (widget, maxDepth);
}

//------------------------------------------------------------------------------
//
void QEScaling::applyToPoint (QPoint& point)
{
   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling

   int x = point.x ();
   int y = point.y ();

   x = QEScaling::scale (x);
   y = QEScaling::scale (y);
   point = QPoint (x, y);
}

// end
