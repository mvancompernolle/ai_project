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

using namespace std;

// Define a class MyFullAdder. The instances of this class will serve as
// 1-bit full-adder modules for our circuit.
class MyFullAdder : public Module
{
public:
    // The constructor should take single line Bus<1> objects as inputs, one each for
    // the two data lines, carry input, sum output and the carry output.
    MyFullAdder(const Bus<1> &S, const Bus<1> Cout, const InputBus<1> &A,
              const InputBus<1> &B, const InputBus<1> &Cin);

    // Destructor.
    //
    ~MyFullAdder();

    // This function is re-implemented from the class lcs::Module.
    // When state of any of the input bus lines changes, the corresponding bus
    // will call this function to notify the fulladder module of such a
    // change. This function then perform a 1-bit full binary addition of the
    // line states of input busses and propogate the result to the output busses.
    virtual void onStateChange(int portId);

private:
    Bus<1> s, cout;
    InputBus<1> a, b, c;
};

MyFullAdder::MyFullAdder(const Bus<1> &S, const Bus<1> Cout, const InputBus<1> &A,
                     const InputBus<1> &B, const InputBus<1> &Cin)
         : Module(), s(S), cout(Cout), a(A), b(B), c(Cin)
{
    // A functional module has to be driven by the data lines of the input busses.
    // In order to be driven, a module has to register itself with each of the input
    // busses using the drive function. If not registered, a change to the state of the data
    // lines of an input bus will not trigger the module to propogate the change and produce
    // the output.
    a.notify(this, LINE_STATE_CHANGE, 0);
    b.notify(this, LINE_STATE_CHANGE, 0);
    c.notify(this, LINE_STATE_CHANGE, 0);

    // After setting up the busses, the line data should be propogated to the output so that
    // the output is the one corresponding to the input after the module is constructed.
    onStateChange(0);
}

MyFullAdder::~MyFullAdder()
{
    // When a module ceases to exist anylonger, it should notify its input busses to stop triggering
    // it. This is done using the lcs::InputBus::stopNotification function. If not de-registered in this
    // way, the input busses will try to drive a non existant module, resulting in a segmentation fault.
    a.stopNotification(this, LINE_STATE_CHANGE, 0);
    b.stopNotification(this, LINE_STATE_CHANGE, 0);
    c.stopNotification(this, LINE_STATE_CHANGE, 0);
}

void MyFullAdder::onStateChange(int portId)
{
    // The LineAccessor objects come with an overloaded operator[] for convenience.
    // The sum and carry outputs are calculated using the overloaded bitwise operators
    // rather than through an ensemble of logics gates.
	s[0] = (~a[0] & ~b[0] & c[0]) | (~a[0] & b[0] & ~c[0]) |
                    (a[0] & ~b[0] & ~c[0]) | (a[0] & b[0] & c[0]);
    cout[0] = (~a[0] & b[0] & c[0]) | (a[0] & ~b[0] & c[0]) |
                      (a[0] & b[0] & ~c[0]) | (a[0] & b[0] & c[0]);
}

int main(void)
{
    // Declaring the busses involved in out circuit.
    // Note that the bus c0 (the carry input to the first full-adder)
    // has been initialised with a value of 0 (or lcs::LOW) on its line.
    Bus<> a1, b1, a2, b2, c0(0), S1, C1, S2, C2;

    // Initialising the 1-bit full adder modules.
    MyFullAdder fa1(S1, C1, a1, a2, c0), fa2(S2, C2, b1, b2, C1);

    // Initialising lcs::ChangeMonitor objects which monitor the inputs and the
    // 3 sum bits.
    ChangeMonitor<4> inputMonitor((a1,b1,a2,b2), "Input", DUMP_ON);
    ChangeMonitor<3> outputMonitor((S1,S2,C2), "Sum", DUMP_ON);

	// The tester object which will feed in a different input at every clock 
	// state change, starting from 0 in sequence.
    Tester<4> tester((a1,b1,a2,b2));

    Simulation::setStopTime(2000); // Set the time upto which the simulation should run.
    Simulation::start();           // Start the simulation.

    return 0;
}
