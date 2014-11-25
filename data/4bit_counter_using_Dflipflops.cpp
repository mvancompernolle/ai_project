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
#include <lcs/not.h>
#include <lcs/clock.h>
#include <lcs/changeMonitor.h>
#include <lcs/simul.h>
#include <lcs/dflipflop.h>

using namespace lcs;
using namespace std;

int main(void)
{
    // Declare single line busses for each node in the
    // circuit. The flipflop output nodes have been set
    // to zero to start with. The busses are initialised
    // with the default template parameter of 1. The reset
	// line to be used for all the flipflops has been set 
	// to 0.
    Bus<> q0(0), q1(0), q2(0), q3(0), d0, d1, d2, d3, rst(0);

    // Initialise a clock object.
    Clock clk = Clock::getClock();

    // Initialise the NOT gates in the circuit. The NOT
    // gates are initialised with the default template
    // parameter corresponding to a propogation delay of
    // zero.
    Not<> n1(d0, q0), n2(d1, q1), n3(d2, q2), n4(d3, q3);

    // Initialise the 4 positive edge-triggered flipflops
    // using the default template parameters.
    DFlipFlop<> ff1(q0, d0, clk, rst), ff2(q1, d1, q0, rst), 
                ff3(q2, d2, q1, rst), ff4(q3, d3, q2, rst);

    // Initialise a lcs::ChangeMonitor object to monitor
    // the output bit sequence of our counter circuit.
	ChangeMonitor<4> count((d0, d1, d2, d3), string("Count"), DUMP_ON);

    Simulation::setStopTime(4000); // Set the stop time.
    Simulation::start(); // Start the simulation.

    return 0;
}
