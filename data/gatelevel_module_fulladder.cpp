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
#include <lcs/or.h>
#include <lcs/and.h>
#include <lcs/not.h>
#include <lcs/tester.h>
#include <lcs/simul.h>
#include <lcs/changeMonitor.h>

// All classes of the libLCS are defined in the namespace lcs.
using namespace lcs;

using namespace std;

// Define a class MyFullAdder. Our 1-bit fulladder modules will be instances
// of this class.
class MyFullAdder
{
public:
    // The constructor should take single line Bus<1> objects as inputs, one each for
    // the two data lines, carry input, sum output and the carry output.
    MyFullAdder(const Bus<1> &S, const Bus<1> &Cout,
              const Bus<1> &A, Bus<1> &B, Bus<1> &Cin);

    // The destructor.
    ~MyFullAdder();

private:
    // a, b, c are the two data inputs and the carry input respectively.
    Bus<1> a, b, c;

    // s is the sum output, and cout is the carry output.
    Bus<1> s, cout;

    // A pointer member for each of the gates which constitute our FullAdder.
    // The number and type of the gates is same as in the 1st basic example.
    Not<> *n1, *n2, *n3;
    And<3> *sa1, *sa2, *sa3, *sa4;
    And<3> *ca1, *ca2, *ca3, *ca4;
    Or<4> *so, *co;
};


MyFullAdder::MyFullAdder(const Bus<1> &S, const Bus<1> &Cout,
                     const Bus<1> &A, Bus<1> &B, Bus<1> &Cin)
         : a(A), b(B), c(Cin), s(S), cout(Cout)
{
    Bus<> a_, b_, c_, s1, s2, s3, s4, c1, c2, c3, c4;

    // Each of the gates should be initialised with proper bus
    // connections. The connections are same as in the 1st basic
    // example.

     // Initialising the NOT gates.
    n1 = new Not<>(a_, a); n2 = new Not<>(b_, b); n3 = new Not<>(c_, c);

    // Initialising the AND gates for the
    // minterms corresponding to the sum output.
    sa1 = new And<3>(s1, (a_,b_,c)); sa3 = new And<3>(s3, (a,b_,c_));
    sa2 = new And<3>(s2, (a_,b,c_)); sa4 = new And<3>(s4, (a,b,c));

    // Initialising the AND gates for the
    // minterms corresponding to the carry output.
    ca1 = new And<3>(c1, (a_,b,c)); ca2 = new And<3>(c2, (a,b_,c));
    ca3 = new And<3>(c3, (a,b,c_)); ca4 = new And<3>(c4, (a,b,c));

    // Initialising the OR gates which
    // generate the final outputs.
    so = new Or<4>(s, (s1,s2,s3,s4)), co = new Or<4>(cout, (c1,c2,c3,c4));
}

MyFullAdder::~MyFullAdder()
{
    // The destructor should delete each of the gates
    // which were initialised during construction.

    delete n1; delete n2; delete n3;
    delete sa1; delete sa2; delete sa3; delete sa4;
    delete ca1; delete ca2; delete ca3; delete ca4;
    delete so; delete co;
}

int main(void)
{
    // Declaring the busses involved in out circuit.
    // Note that the bus c0 (the carry input to the first full-adder)
    // has been initialised with a value of 0 (or lcs::LOW) on its line.
    Bus<> a1, b1, a2, b2, c0(0), S1, C1, S2, C2;

    // Initialising the 1-bit full adder modules.
    MyFullAdder fa1(S1, C1, a1, a2, c0), fa2(S2, C2, b1, b2, C1);

    // Initialising monitor objects which monitor the inputs and the
    // 3 sum bits.
    ChangeMonitor<4> inputMonitor((a1,b1,a2,b2), "Input", DUMP_ON);
    ChangeMonitor<3> outputMonitor((S1,S2,C2), "Sum", DUMP_ON);

    // Initialising a tester object which feeds in different inputs
    // into our circuit.
    Tester<4> tester((a1,b1,a2,b2));

    Simulation::setStopTime(2000); // Set the time upto which the simulation should run.
    Simulation::start();           // Start the simulation.

    return 0;
}
