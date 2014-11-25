/* QCaDataPoint.cpp
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
 */

#include <QEArchiveInterface.h>
#include <QCaDataPoint.h>
#include <QECommon.h>


static const QString stdFormat = "dd/MMM/yyyy HH:mm:ss";

//------------------------------------------------------------------------------
//
QCaDataPoint::QCaDataPoint ()
{
   this->value = 0.0;

   // Register type.
   //
   qRegisterMetaType<QCaDataPoint> ("QCaDataPoint");
}

//------------------------------------------------------------------------------
//
bool QCaDataPoint::isDisplayable () const
{
   bool result;
   QEArchiveInterface::archiveAlarmSeverity severity;

   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();

   switch (severity) {

      case QEArchiveInterface::archSevNone:
      case QEArchiveInterface::archSevMinor:
      case QEArchiveInterface::archSevMajor:
      case QEArchiveInterface::archSevEstRepeat:
      case QEArchiveInterface::archSevRepeat:
         result = true;
         break;

      case QEArchiveInterface::archSevInvalid:
      case QEArchiveInterface::archSevDisconnect:
      case QEArchiveInterface::archSevStopped:
      case QEArchiveInterface::archSevDisabled:
         result = false;
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QCaDataPoint::toString () const
{
   QString result;
   QString zone;
   QString valid = "?";
   QEArchiveInterface::archiveAlarmSeverity severity;
   QString severityText = "?";
   QString statusText = "?";

   zone = QEUtilities::getTimeZoneTLA (this->datetime);
   valid = this->isDisplayable () ? "True " : "False";
   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();
   severityText = QEArchiveInterface::alarmSeverityName (severity);
   statusText = this->alarm.statusName();

   result = QString ("%1  %2  %3  %4  %5  %6")
               .arg (this->datetime.toString (stdFormat), 20)
               .arg (zone)
               .arg (this->value, 16, 'e', 8)
               .arg (valid, 10)
               .arg (severityText, 10)
               .arg (statusText, 10);

   return result;
}

//------------------------------------------------------------------------------
//
QString QCaDataPoint::toString (const QCaDateTime& originDateTime) const
{
   QString result;
   QString zone;
   double relative;
   QString valid = "?";
   QEArchiveInterface::archiveAlarmSeverity severity;
   QString severityText = "?";
   QString statusText = "?";

   zone = QEUtilities::getTimeZoneTLA (this->datetime);
   valid = this->isDisplayable () ? "True " : "False";
   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();
   severityText = QEArchiveInterface::alarmSeverityName (severity);
   statusText = this->alarm.statusName();

   // Calculate the relative time from start.
   //
   relative = originDateTime.secondsTo (this->datetime);

   result = QString ("%1  %2  %3  %4  %5  %6  %7")
               .arg (this->datetime.toString (stdFormat), 20)
               .arg (zone)
               .arg (relative, 16, 'f', 3)
               .arg (this->value, 16, 'e', 8)
               .arg (valid, 10)
               .arg (severityText, 10)
               .arg (statusText, 10);

   return result;
}

//==============================================================================
//
QCaDataPointList::QCaDataPointList () // : QList<QCaDataPoint> ()
{
   // Register type.
   //
   qRegisterMetaType<QCaDataPointList> ("QCaDataPointList");
}

//------------------------------------------------------------------------------
//
void  QCaDataPointList::append (const QCaDataPointList& other)
{
   this->data.append (other.data);
}

//------------------------------------------------------------------------------
//
QCaDataPoint QCaDataPointList::value (const int j) const
{
   return data.value (j);
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::resample (const QCaDataPointList& source,
                                 const double interval,
                                 const QCaDateTime& endTime)
{
   QCaDateTime firstTime;
   int j;
   int next;
   QCaDateTime jthTime;
   QCaDataPoint point;

   this->clear ();
   if (source.count () <= 0) return;

   firstTime = source.value (0).datetime;
   jthTime = firstTime;
   next = 0;
   for (j = 0; jthTime < endTime; j++) {

      // Calculate to nearest mSec.
      //
      jthTime = firstTime.addMSecs ((qint64)( (double) j * 1000.0 * interval));

      while (next < source.count () && source.value (next).datetime <= jthTime) next++;
      point = source.value (next - 1);
      point.datetime = jthTime;
      this->append (point);
   }
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::compact (const QCaDataPointList& source)
{
   int j;
   QCaDataPoint lastPoint;

   this->clear ();
   if (source.count () <= 0) return;

   // Copy first point.
   lastPoint = source.value (0);
   this->append (lastPoint);

   for (j = 1; j < source.data.count (); j++) {
      QCaDataPoint point = source.data.value (j);
      if ((point.value != lastPoint.value) ||
          (point.alarm != lastPoint.alarm)) {
         this->append (point);
         lastPoint = point;
      }
   }
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::toStream (QTextStream& target,
                                 bool withIndex,
                                 bool withRelativeTime) const
{
   int number = this->count ();
   int j;
   QCaDateTime originDateTime;

   if (number > 0) {
      originDateTime = this->value (0).datetime;

      for (j = 0; j < number; j++) {
         QCaDataPoint point = this->value (j);
         QString item;

         item = "";
         if (withIndex) {
            item.append (QString ("%1  ").arg (j + 1, 6));
         }

         if (withRelativeTime) {
            item.append (point.toString (originDateTime));
         } else {
            item.append (point.toString ());
         }

         target << item << "\n";
      }
   }
   else {
      target << "(QCaDataPointList empty)" << "\n";
   }
}

// end
