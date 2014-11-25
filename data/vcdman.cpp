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

#include <lcs/vcdman.h>

#ifndef NULL
#define NULL 0
#endif

using namespace lcs;
using namespace std;

ofstream VCDManager::out_;
List<string> VCDManager::varNameList_;
List<string> VCDManager::initValList_;
List<int> VCDManager::varSizeList_;
string VCDManager::fname_    = string("dump.vcd");
int VCDManager::varcount_    = -1;
int VCDManager::currtime_    = -1;
TimeUnit VCDManager::unit_   = MICRO_SECOND;

void VCDManager::init(void)
{
    if (varcount_ >= 0)
    {
        out_.open(fname_.c_str(), ios::out);
        if (!out_.is_open())
        {
            cerr << "Unable to generate the VCD file.\n\n"
                 << "1) Provide the correct path and/or\n"
                 << "2) suitable write permissions and/or\n"
                 << "3) sufficient disk space to generate the file!!\n\n";

            varcount_ = -1;
            return;
        }

        // Writing the date information.
        out_ << "$date\n\tCreated on some day! Does it matter which?\n$end\n";

        // Writing the version information.
        out_ << "$version\n\tCreated by libLCS\n\tSee http://liblcs.sourceforge.net/ for more info.\n"
             << "$end\n";

        // Writing timescale information.
        switch (unit_)
        {
            case(FEMTO_SECOND) : { out_ << "$timescale\n\t1fs\n$end\n"; break; }
            case(PICO_SECOND)  : { out_ << "$timescale\n\t1ps\n$end\n"; break; }
            case(NANO_SECOND)  : { out_ << "$timescale\n\t1ns\n$end\n"; break; }
            case(MICRO_SECOND) : { out_ << "$timescale\n\t1us\n$end\n"; break; }
            case(MILLI_SECOND) : { out_ << "$timescale\n\t1ms\n$end\n"; break; }
            case(SECOND)       : { out_ << "$timescale\n\t 1s\n$end\n"; break; }
            default            : { out_ << "$timescale\n\t1us\n$end\n"; break; }
        }

        // Writing the scope information.
        out_ << "$scope module all $end\n";

        // Writing the variable information.
        ListIterator<string> nameIter = varNameList_.getListIterator();
        ListIterator<int>    sizeIter = varSizeList_.getListIterator();
        nameIter.reset(); sizeIter.reset();
        while (nameIter.hasNext())
        {
            string name = nameIter.next();
            int size = sizeIter.next();

            if (size == 1)
                out_ << "$var reg " << size << " " << name << " " << name << " $end\n";
            else
                out_ << "$var reg " << size << " " << name << " " << name
                     << "[" << size-1 << ":0]" << " $end\n";
        }

        // Writing $enddefinitions.
        out_ << "$enddefinitions $end\n";

    }
}

void VCDManager::terminate(void)
{
    if (varcount_ >= 0)
    {
        out_.close();
        cout << "The simulation results have been successfully written into '" << fname_ << "'.\n";
    }
}
