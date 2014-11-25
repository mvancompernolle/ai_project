/*  QCaAlarmInfo.cpp
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
 *  Copyright (c) 2009, 2010
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// CA alarm info manager

#include <alarm.h>
#include <QCaAlarmInfo.h>

/*
  Construct an empty instance.
  By default there is no alarm present.
 */
QCaAlarmInfo::QCaAlarmInfo()
{
    status = NO_ALARM;
    severity = NO_ALARM;
}

/*
  Construct an instance given an alarm state and severity
 */
QCaAlarmInfo::QCaAlarmInfo( unsigned short statusIn, unsigned short severityIn ) {
    status = statusIn;
    severity = severityIn;
}

/*
  Equality function.
 */
bool QCaAlarmInfo::operator==(const QCaAlarmInfo& other) const {
    return( ( this->status == other.status ) && ( this->severity == other.severity ) );
}

/*
  InEquality function - defined in terms of == to ensure consistancy.
 */
bool QCaAlarmInfo::operator!=(const QCaAlarmInfo& other) const {
    return !(*this == other);
}


/*
  Return a string identifying the alarm state
 */
QString QCaAlarmInfo::statusName() const {

    if( status <= lastEpicsAlarmCond )
        return QString( epicsAlarmConditionStrings[status] );
    else
        return QString( "" );
}

/*
  Return a string identifying the alarm severity
 */
QString QCaAlarmInfo::severityName() const {

    if( severity <= lastEpicsAlarmSev )
        return QString( epicsAlarmSeverityStrings[severity] );
    else
        return QString( "" );
}

/*
  Return true if there is an alarm
 */
bool QCaAlarmInfo::isInAlarm() const {
    return( status != NO_ALARM );
}

/*
  Return true if there is a minor alarm
 */
bool QCaAlarmInfo::isMinor() const {
    return( severity == MINOR_ALARM );
}

/*
  Return true if there is a major alarm
 */
bool QCaAlarmInfo::isMajor() const {
    return( severity == MAJOR_ALARM );
}

/*
  Return true if there is an invalid alarm
 */
bool QCaAlarmInfo::isInvalid() const {
    return( severity == INVALID_ALARM );
}

/*
  Return a style string to update the widget's look to reflect the current alarm state
 */
QString QCaAlarmInfo::style() const
{
    QString styleColor = getStyleColorName();
    QString result;

    switch( severity )
    {
        case NO_ALARM:
        case MINOR_ALARM:
        case MAJOR_ALARM:
        case INVALID_ALARM:
            result = QString( "QWidget { background-color: %1; }" ).arg( styleColor );
            break;

        default:
            result = "";
    }

    return result;
}

/*
  Return the style color name for the alarm state
 */
QString QCaAlarmInfo::getStyleColorName() const
{
    switch( severity )
    {
        case NO_ALARM:      return "#e0eae0"; // pale green
        case MINOR_ALARM:   return "#ffff00"; // yellow
        case MAJOR_ALARM:   return "#ff8080"; // red
        case INVALID_ALARM: return "#ffffff"; // white
        default:            return "#ffffff"; // white
    }
}


/*
  Return the color name for the alarm state
 */
QString QCaAlarmInfo::getColorName() const
{
    switch( severity )
    {
        case NO_ALARM:      return "#00ff00"; // green
        case MINOR_ALARM:   return "#ffff00"; // yellow
        case MAJOR_ALARM:   return "#ff0000"; // red
        case INVALID_ALARM: return "#ffffff"; // white
        default:            return "#ffffff"; // white
    }
}

/*
  Return a severity that will not match any valid severity (static)
 */
QCAALARMINFO_SEVERITY QCaAlarmInfo::getInvalidSeverity()
{
    return ALARM_NSEV;
}

/*
  Return the severity
  The caller is only expected to compare this to
 */
QCAALARMINFO_SEVERITY QCaAlarmInfo::getSeverity() const
{
    return severity;
}

// end
