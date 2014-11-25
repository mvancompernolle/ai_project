 
#include <lcs/lcs.h>

using namespace lcs;

// Since the module is going to have few behavioral constructs, we should derive 
// it from the base class lcs::Module.
class TwoBitFullAdder : public Module
{
public:
    // The constructor takes arguments which establish the external connections.
    TwoBitFullAdder(const Bus<3> &sum, const InputBus<2> &a, const InputBus<2> &a);

    ~TwoBitFullAdder();

    // This is a virtual function which has to be overidden. It is declared in the 
    // base class Module. This is the function which should incorporate the behavioral
    // model for a one bit fulladder.
    virtual void onStateChange(int portId);

private:

    InOutBus<1> c1; // The carry input for the fulladder with a behavioral model.
    InputBus<2> a_, b_; // The inputs to our 2-bit fulladder.
    Bus<3> s; // The sum output of our full adder.
    FullAdder *fa; // The fulladder module which will be created in the constructor.
};

TwoBitFullAdder::TwoBitFullAdder(const Bus<3> &sum, const InputBus<2> &a, const InputBus<2> &b)
    : Module(), c1(0), a_(a), b_(b), s(sum)
{
    Bus<> cin(0); // The carry input to the fulladder module.
    fa = new FullAdder(s[0], c1, a[0], b[0], cin); // Initialising the fulladder module.

    // The 2-bit fulladder module has to respond to state changes in the relevant 
    // input bits and the carry output of the fulladder module. Hence the module has to request 
    // notification of line events from these signals. Note here that the carry input c1 for the 
    // behavioral model is declared to be of type lcs::InOutBus. If it were to be declared 
    // as a normal lcs::Bus object, then it will not be able to notify the behavioral
    // model about the events on its lines.
    c1.notify(this, LINE_STATE_CHANGE, 1);
    a_.notify(this, LINE_STATE_CHANGE, 2, 1);
    b_.notify(this, LINE_STATE_CHANGE, 3, 1);
}

TwoBitFullAdder::~TwoBitFullAdder()
{
    delete fa;

    c1.stopNotification(this, LINE_STATE_CHANGE, 1);
    a_.stopNotification(this, LINE_STATE_CHANGE, 2, 1);
    b_.stopNotification(this, LINE_STATE_CHANGE, 3, 1);
}

void TwoBitFullAdder::onStateChange(int portId)
{
    // When a state change occurs on of the lines c1[0] or a_[1] or b[1],
    // The behavioral model should compute the bit values for the second 
    // and third sum output bits and write them onto the output bus. 
    // This is done as follows.
    s[1] = (~a_[1] & ~b_[1] & c1[0]) | (~a_[1] & b_[1] & ~c1[0]) |
                    (a_[1] & ~b_[1] & ~c1[0]) | (a_[1] & b_[1] & c1[0]);;
    s[2] = (~a_[1] & b_[1] & c1[0]) | (a_[1] & ~b_[1] & c1[0]) |
                      (a_[1] & b_[1] & ~c1[0]) | (a_[1] & b_[1] & c1[0]);
}

int main(void)
{
    Bus<2> a, b; // Inputs 
    Bus<3> sum;  // Sum output

    // Initialising a 2-bit fulladder module.
    TwoBitFullAdder adder(sum, a, b);

    // A tester to generate inputs for the 2-bit fulladder module.
    Tester<4> tester((a, b)); 

    ChangeMonitor<2> amon(a, "A");
    ChangeMonitor<2> bmon(b, "B");
    ChangeMonitor<3> smon(sum, "SUM");

    Simulation::setStopTime(2000);
    Simulation::start();
	
    return 0;
}
