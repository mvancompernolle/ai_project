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

#include <lcs/or.h>
#include <lcs/and.h>
#include <lcs/not.h>
#include <lcs/tester.h>
#include <lcs/simul.h>
#include <lcs/changeMonitor.h>

// All classes of the libLCS are defined in the namespace lcs.
using namespace lcs;

int main(void)
{
    // Each node in the circuit corresponds to a single line Bus object
    // in the case of libLCS. Hence we declare a single line bus for each
    // of the nodes marked in the above circuit diagram.
    Bus<1> a, b, c, a_, b_, c_, S, C, s1, s2, s3, s4, c1, c2, c3, c4;

    // Initialising NOT gates which obtain the inverses of the input busses.
    Not<> n1(a_, a), n2(b_, b), n3(c_, c);

    // Initialising an AND gate for each of the 8 min-terms involved. Each
    // AND should take 3 input data lines. Hence, the class And<3> is used.
    // Moreover, the input busses to these AND gates should have 3 lines each.
    // Hence, the 3 line input busses are generated using the overloaded
    // ',' operator. The ',' does not denote a comma separated list 
    // but is a bus/line concatenation operator. It concatenates two busses
    // to produce another bus which contains lines from both the operand busses.
    And<3> sa1(s1, (a_,b_,c)), sa2(s2, (a_,b,c_)), sa3(s3, (a,b_,c_)), sa4(s4, (a,b,c));
    And<3> ca1(c1, (a_,b,c)), ca2(c2, (a,b_,c)), ca3(c3, (a,b,c_)), ca4(c4, (a,b,c));

    // Initialising the 4-input OR gates which take the outputs of the AND gates
    // and produce the desired results.
    Or<4> so(S, (s1,s2,s3,s4)), co(C, (c1,c2,c3,c4));

    // Initialising change monitors which monitor the changes to the busses of
    // interest. Here, we initialise monitors which report the changes on the
    // input and output busses.
    ChangeMonitor<3> inputMonitor((c,b,a), "Input", DUMP_ON);
    ChangeMonitor<2> outputMonitor((S,C), "Sum", DUMP_ON);

    // Initialising a tester object which feeds a different value on to the input
    // busses at every clock pulse. This a way to feed inputs to our circuit for
    // the purpose of testing it.
    Tester<3> tester((c,b,a));

    Simulation::setStopTime(1000); // Set the time upto which the simulation should run.
    Simulation::start();           // Start the simulation.

    return 0;
}
