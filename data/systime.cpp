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

#include <lcs/systime.h>
#include <lcs/line.h>
#include <lcs/clock.h>

using namespace lcs;

unsigned int SystemTimer::time = 0;
unsigned int SystemTimer::hiddenTime = 0;
unsigned int SystemTimer::stoptime = 0;

List<TickListener*> SystemTimer::tlList = List<TickListener*>();
List< List<Line*>* > SystemTimer::refLists = List< List<Line*>* >();
Queue< List<Line*>* > SystemTimer::hiddenQueue = Queue< List<Line*>* >();

Clock *SystemTimer::clock = NULL;

void SystemTimer::setStopTime(unsigned int stopTime) { stoptime = stopTime; }

void SystemTimer::notifyTick(TickListener *tl)
{
    if (tl != NULL)
        tlList.append(tl);
}

void SystemTimer::notifyClock(Clock *clk)
{
    if (clk != NULL && clock == NULL)
        clock = clk;
}

void SystemTimer::notifyLine(List<Line*> *list)
{
    if (list != NULL)
        refLists.append(list);
}

void SystemTimer::notifyHiddenTick(List<Line*> *list)
{
    if (list != NULL)
        hiddenQueue.enQueue(list);
}

unsigned int SystemTimer::getTime(void) { return time; }

unsigned int SystemTimer::getHiddenTime(void) { return hiddenTime; }

void SystemTimer::reset(void) { time = 0; }

void SystemTimer::stopTickNotification(TickListener *tl)
{
    if (tl != NULL)
        tlList.removeFirstMatch(tl);
}

void SystemTimer::stopLineNotification(List<Line*> *list)
{
    if (list != NULL)
        refLists.removeFirstMatch(list);
}

void SystemTimer::stopClockNotification()
{
    clock = NULL;
}

void SystemTimer::start(void)
{
    updateHiddenDelays();

    ListIterator<TickListener*> iter = tlList.getListIterator();
    iter.reset();
    while (iter.hasNext())
    {
        TickListener *tl = iter.next();
        if (tl != NULL)
            tl->onTick();
    }

    while (time < stoptime)
    {
        time++;

        ListIterator< List<Line*>* > refIter = refLists.getListIterator();
        refIter.reset();
        while (refIter.hasNext())
        {
            List<Line*> *list = refIter.next();

            Line *line = list->first();
            if (line != NULL)
                line->update();
        }

        updateHiddenDelays();

        if (clock != NULL)
            clock->tick();

        updateHiddenDelays();

        ListIterator<TickListener*> iter = tlList.getListIterator();
        iter.reset();

        while (iter.hasNext())
        {
            TickListener *tl = iter.next();
            if (tl != NULL)
                tl->onTick();
        }
    }
}

void SystemTimer::updateHiddenDelays(void)
{
    while (hiddenQueue.getSize() != 0)
    {
        hiddenTime++;
        List<Line*> *hdList = hiddenQueue.getFirstInQueue();

        Line *hdLine = hdList->first();
        if (hdLine != NULL)
            hdLine->hiddenUpdate();

        hiddenQueue.deQueue();
    }

    hiddenTime = 0;
}
