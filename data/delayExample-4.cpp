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

#include <lcs/xor.h>
#include <lcs/simul.h>
#include <lcs/tester.h>
#include <lcs/changeMonitor.h>

using namespace lcs;

int main()
{
    Bus<1> a, b, s1, s2;
	Clock clk = Clock::getClock();
	Clock::setPulseWidth(5);
	
	// Initialising an XOR gate with a propogation delay of 
	// seven system time units. This delay is more than the 
	// pulse width of the input clock signal.
	Xor<2, 7> xorGate1(s1, (a,b));
	
	// Initialising an XOR gate with a propogation delay of 
	// three system time units. This delay is less than the 
	// pulse width of the input clock signal.
	Xor<2, 3> xorGate2(s2, (a,b));
	
	// Initialising change monitors to monitor the output of
	// the two XOR gates.
	ChangeMonitor<> output1(s1, "Output1", DUMP_ON);
	ChangeMonitor<> output2(s2, "Output2", DUMP_ON);
	
	Tester<2> tester((a,b));
	
	Simulation::setStopTime(50);
	Simulation::start();
	
	return 0;
}
