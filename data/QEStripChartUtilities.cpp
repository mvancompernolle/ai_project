/*  QEStripChartUtilities.cpp
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
 *  Copyright (c) 2013
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QECommon.h>

#include "QEStripChartUtilities.h"

//==============================================================================
//
ValueScaling::ValueScaling ()
{
   d = 0.0;
   m = 1.0;
   c = 0.0;
}

//------------------------------------------------------------------------------
//
void ValueScaling::reset ()
{
   d = 0.0;
   m = 1.0;
   c = 0.0;
}

//------------------------------------------------------------------------------
//
void ValueScaling::assign (const ValueScaling & source)
{
   d = source.d;
   m = source.m;
   c = source.c;
}

//------------------------------------------------------------------------------
//
void ValueScaling::set (const double dIn, const double mIn, const double cIn)
{
   d = dIn;
   m = mIn;
   c = cIn;
}

//------------------------------------------------------------------------------
//
void ValueScaling::get (double &dOut, double &mOut, double &cOut) const
{
   dOut = d;
   mOut = m;
   cOut = c;
}

//------------------------------------------------------------------------------
//
void ValueScaling::map (const double fromLower, const double fromUpper,
                        const double toLower,   const double toUpper)
{
   double delta;

   // Scaling is: y = (x - d)*m + c
   // We have three unknowns and two constraints, so have an extra
   // degree of freedom.
   //
   // Set origin to mid-point from value.
   //
   this->d = 0.5 * (fromLower + fromUpper);

   // Set Offset to mid-display value.
   //
   this->c = 0.5 * (toLower + toUpper);

   // Avoid the divide by zero.
   //
   delta = fromUpper - fromLower;
   if (delta >= 0.0) {
      delta = MAX (delta, +1.0E-9);
   } else {
      delta = MAX (delta, -1.0E-9);
   }

   // Set slope as ratio of to (display) span to form span.
   //
   this->m = (toUpper - toLower) / delta;
}

//------------------------------------------------------------------------------
//
bool ValueScaling::isScaled () const
{
   return ((this->d != 0.0) || (this->m != 1.0) || (this->c != 0.0));
}

//------------------------------------------------------------------------------
//
QEDisplayRanges ValueScaling::value (const QEDisplayRanges& x) const
{
   QEDisplayRanges result;           // undefined
   bool okay;
   double min, max;

   okay = x.getMinMax (min, max);
   if (okay) {
      // The range is defined - the extracted min and max are good, so scale
      // each limit and merge into result.
      //
      result.setRange (this->value (min), this->value (max));
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ValueScaling::saveConfiguration (PMElement & parentElement) const
{
   if (this->isScaled()) {
      PMElement scalingElement = parentElement.addElement ("Scaling");

      scalingElement.addAttribute ("Origin", this->d);
      scalingElement.addAttribute ("Slope",  this->m);
      scalingElement.addAttribute ("Offset", this->c);
   }
}

//------------------------------------------------------------------------------
//
void ValueScaling::restoreConfiguration (PMElement & parentElement)
{
   double ad, am, ac;
   bool status;

   PMElement scalingElement = parentElement.getElement ("Scaling");

   status = scalingElement.getAttribute ("Origin", ad) &&
            scalingElement.getAttribute ("Slope",  am) &&
            scalingElement.getAttribute ("Offset", ac);

   if (status) {
      this->set (ad, am, ac);
   }
}

// end
