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

#include <lcs/lcs.h>

using namespace lcs;
using namespace std;

int main(void)
{
    Bus<3> IN;
    Bus<2> S;

    // Continuous assignment statements to generate
    // the sum and carry outputs. The template parameters
    // indicate the assignment delay. We have used 0
    // delay here.
    S[0].cass<0>(IN[0]&~IN[1]&~IN[2] | ~IN[0]&IN[1]&~IN[2] | ~IN[0]&~IN[1]&IN[2] | IN[0]&IN[1]&IN[2]);
    S[1].cass<0>(IN[0]&IN[1]&~IN[2] | IN[0]&~IN[1]&IN[2] | ~IN[0]&IN[1]&IN[2] | IN[0]&IN[1]&IN[2]);

    ChangeMonitor<3> inputMonitor(IN, "Input", DUMP_ON);
    ChangeMonitor<2> outputMonitor(S, "Sum", DUMP_ON);

    Tester<3> tester(IN);

    Simulation::setStopTime(1000);
    Simulation::start();

    return 0;
}
