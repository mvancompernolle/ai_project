//////////////////////////////////////////////////////////////////////////////////
// This file is distributed as part of the libLCS library.
// libLCS is C++ Logic Circuit Simulation library.
//
// Copyright (c) 2006-2007, B. R. Siva Chandra
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// In case you would like to contact the author, use the following e-mail
// address: sivachandra_br@yahoo.com
//////////////////////////////////////////////////////////////////////////////////

#include <lcs/clock.h>
#include <lcs/systime.h>

using namespace lcs;

Clock *Clock::clock = new Clock();
unsigned int *Clock::pulsewidth = NULL;
List<TickListener*> *Clock::tlList = NULL;

Clock::Clock(void) throw (MultipleClockException)
     : InputBus<1>()
{
    if (Clock::clock == NULL)
    {
        pulsewidth = new unsigned int;
        *pulsewidth = 100;

        tlList = new List<TickListener*>();

        SystemTimer::notifyClock(this);

        Line *data = InputBus<1>::dataPtr->data;
        data[0].setLineValue(LOW);
    }
    else
        throw MultipleClockException();
}

Clock::Clock(const Clock& clk)
     : InputBus<1>(clk)
{}

Clock::~Clock()
{}

Clock Clock::getClock(void) throw (NullClockException)
{
    if (Clock::clock != NULL)
    {
        return *clock;
    }
    else
        throw NullClockException();
}

void Clock::setPulseWidth(unsigned int width)
{
    *pulsewidth = width;
}

void Clock::notifyTick(TickListener *tl)
{
    if (tl != NULL)
        tlList->append(tl);
}

void Clock::stopTickNotification(TickListener *tl)
{
    if (tl != NULL)
        tlList->removeFirstMatch(tl);
}

void Clock::tick()
{
    unsigned time = SystemTimer::getTime();

    if (time > 0 && (time%(*pulsewidth) == 0))
    {
        ListIterator<TickListener*> iter = tlList->getListIterator();
        iter.reset();

        while (iter.hasNext())
        {
            TickListener *tl = iter.next();
            if (tl != NULL)
                tl->onTick();
        }

        Line *data = InputBus<1>::dataPtr->data;
        data[0].setLineValue(!data[0]);
    }
}
