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

#include <lcs/fa.h>

using namespace lcs;

FullAdder::FullAdder(const Bus<1> &S, const Bus<1> Cout, const InputBus<1> &A,
                     const InputBus<1> &B, const InputBus<1> &Cin)
         : Module(), s(S), cout(Cout), a(A), b(B), c(Cin)
{
    a.notify(this, LINE_STATE_CHANGE, 0);
    b.notify(this, LINE_STATE_CHANGE, 1);
    c.notify(this, LINE_STATE_CHANGE, 2);

    onStateChange(0);
}

FullAdder::~FullAdder()
{
    a.stopNotification(this, LINE_STATE_CHANGE, 0);
    b.stopNotification(this, LINE_STATE_CHANGE, 1);
    c.stopNotification(this, LINE_STATE_CHANGE, 2);
}

void FullAdder::onStateChange(int portId)
{
    s[0] = (~a[0] & ~b[0] & c[0]) | (~a[0] & b[0] & ~c[0]) |
                    (a[0] & ~b[0] & ~c[0]) | (a[0] & b[0] & c[0]);
    cout[0] = (~a[0] & b[0] & c[0]) | (a[0] & ~b[0] & c[0]) |
                      (a[0] & b[0] & ~c[0]) | (a[0] & b[0] & c[0]);
}
