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

#include <lcs/linestate.h>

using namespace lcs;
using namespace std;

const LineState operator&(const LineState& op1, const LineState& op2)
{
    if (op1 == LOW || op2 == LOW)
        return LOW;
    else if (op1 == HIGH && op2 == HIGH)
        return HIGH;
    else
        return LOW;
}

const LineState operator|(const LineState& op1, const LineState& op2)
{
    if (op1 == HIGH || op2 == HIGH)
        return HIGH;
    else
        return LOW;
}

const LineState operator^(const LineState& op1, const LineState& op2)
{
    if (op1 == LOW && op2 == LOW)
        return LOW;
    else if (op1 == LOW && op2 == HIGH)
        return HIGH;
    else if (op1 == HIGH && op2 == LOW)
        return HIGH;
    else
        return LOW;
}

const lcs::LineState operator~(const lcs::LineState &s)
{
    if (s == HIGH)
        return LOW;
    else if (s == LOW)
        return HIGH;
    else
        return HIGH;
}
