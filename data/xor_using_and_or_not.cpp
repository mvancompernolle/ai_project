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

#include <lcs/bus.h>
#include <lcs/not.h>
#include <lcs/and.h>
#include <lcs/or.h>
#include <lcs/tester.h>
#include <lcs/simul.h>
#include <lcs/changeMonitor.h>

int main()
{
    lcs::Bus<1> a, b, a_, b_, p1, p2, s;
    
    lcs::Not<> ng1(a_, a), ng2(b_, b);
    lcs::And<2> ag1(p1, (a,b_)), ag2(p2, (a_,b));
    lcs::Or<2> og(s, (p1,p2));

    lcs::ChangeMonitor<2> inputMonitor((a,b), "Input", lcs::DUMP_ON);
    lcs::ChangeMonitor<1> outputMonitor(s, "Output", lcs::DUMP_ON);

    lcs::Tester<2> tester((a,b));

    lcs::Simulation::setStopTime(1000);
    lcs::Simulation::start();

    return 0;
}
