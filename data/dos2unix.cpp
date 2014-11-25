/*

  GLT OpenGL C++ Toolkit (LGPL)
  Copyright (C) 2000-2003 Nigel Stewart

  Email:  nigels@nigels.com
  WWW:    http://www.nigels.com/glt/
  Forums: http://sourceforge.net/forum/?group_id=36869

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*! \file
    \brief   Utility for converting DOS/Windows text files to UNIX
    \ingroup Util
*/

#include <misc/string.h>

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

bool GlutMain(const std::vector<std::string> &arg)
{
    uint32 i;
    bool help = false;

    for (i=1; i<arg.size(); i++)
        if (arg[i]=="--help" || arg[i]=="/?")
            help = true;

    if (arg.size()==1 || help)
    {
        cout << endl;
        cout << "dos2unix" << endl;
        cout << endl;
        cout << "UNIX end-of-line conversion tool." << endl;
        cout << "(C) 2001-2003 Nigel Stewart (nigels@nigels.com)" << endl;
        cout << "GLT library - http://www.nigels.com/glt/" << endl;
        cout << endl;
        cout << "Usage: dos2unix SOURCE..." << endl;
        cout << endl;
        cout << "\tEach source file is converted to a" << endl;
        cout << "\tUNIX style ASCII file." << endl;
        cout << endl;

        return false;
    }

    for (i=1; i<arg.size(); i++)
    {
        // Read the file into a string

        string in;

        {
            ifstream is(arg[i].c_str(),ios::in|ios::binary);
            readStream(is,in);
        }

        // Convert the string and write it

        if (isBinary(in))
            cout << "Binary:   ";
        else
            if (in.size()>0)
            {
                string out;
                dos2unix(out,in);

                if (in!=out)
                {
                    cout << "Updated:  ";
                    ofstream os(arg[i].c_str(),ios::out|ios::binary);
                    writeStream(os,out);
                }
                else
                    cout << "Skipped:  ";
            }
            else
                cout << "Empty:    ";

        cout << arg[i] << endl;
    }

    return true;
}
