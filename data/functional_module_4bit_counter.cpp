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

#include <iostream>
#include <lcs/bus.h>
#include <lcs/clock.h>
#include <lcs/module.h>
#include <lcs/simul.h>
#include <lcs/changeMonitor.h>

using namespace lcs;
using namespace std;

// Let the name of our counter module be Counter.
// It has to be derived from the class lcs::Module.
class Counter : public Module
{
public:

    // The constructor should take the output bus and the
    // clock input as the arguments.
    Counter(const Bus<4> &outbus, const InputBus<> &clk);

    // The virtual destructor.
    virtual ~Counter();

    // Since our counter module will be triggered by the
    // positive edge of the clock, it has to override the
    // function lcs::Module::onPosEdge.
    virtual void onPosEdge(int portId);

private:
    // Private members to store references to the output
    // bus and the clock input.
    Bus<4> outBus;
    InputBus<> clock;
};

Counter::Counter(const Bus<4> &outbus, const InputBus<> &clk)
       : Module(), outBus(outbus), clock(clk)
{
    // Register with the clock input to be notified of
    // a lcs::LINE_POS_EDGE event.
    clock.notify(this, LINE_POS_EDGE, 0);
    outBus = 0;
}

Counter::~Counter()
{
    // De-register with the clock input so that it will stop
    // notifying line events to a non-existant module.
    clock.stopNotification(this, LINE_POS_EDGE, 0);
}

void Counter::onPosEdge(int portId)
{
    int val = outBus.toInt(); 

    if (val == 15) // If the bus holds a value of fifteen, then reset the value to zero.
    {
        outBus = 0;
    }
    else // If the bus holds a value other than 15, then increment it by 1.
    {
        outBus = val+1;
    }
}

int main(void)
{
    Bus<4> output;
    Clock clk = Clock::getClock();

    Counter counter(output, clk);

    ChangeMonitor<4> count(output, string("Count"), DUMP_ON);

    Simulation::setStopTime(4000);
    Simulation::start();

    return 0;
}
