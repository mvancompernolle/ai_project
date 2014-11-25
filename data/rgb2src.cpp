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
    \brief   Utility for converting UNIX RGB database files to C/C++ source
    \ingroup Util
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
using namespace std;

#include <glt/config.h>
#include <glt/color.h>

int main(int argc,char *argv[])
{
    if (argc==1)
    {
        cout << endl;
        cout << "rgb2src" << endl;
        cout << endl;
        cout << "Unix rgb database to source conversion utility." << endl;
        cout << "(C) 2001-2003 Nigel Stewart (nigels@nigels.com)" << endl;
        cout << "GLT library - http://www.nigels.com/glt/" << endl;
        cout << endl;
        cout << "Usage: rgb2src [rgb.txt]" << endl;
        cout << endl;
        cout << "\trgb.h, rgb.cpp and rgb.dox will be output to the current directory." << endl;
        cout << endl;

        return EXIT_FAILURE;
    }

    map<string,GltColor> color;

    //
    // Read the rgb.txt file into a std::map
    //

    ifstream is(argv[1]);

    char buffer[1024];
    while (is.good())
    {
        is.getline(buffer,1024);

        int r,g,b;
        char name[1024];
        if (sscanf(buffer,"%d %d %d %[^\n]",&r,&g,&b,name)==4)
        {
            // Ignore empty names, or names with whitespace

            if (strchr(name,' ') || strchr(name,'\t') || strlen(name)==0)
                continue;

            // Ignore colors that collide with std namespace

            if (!strcmp(name,"tan"))
                continue;

            // Force first character to lower case

            if (name[0]>='A' && name[0]<='Z')
                name[0] -= 'A'-'a';

            // Add the color to the map

            color[name] = GltColor(r/255.0,g/255.0,b/255.0);
        }
    }

    //
    // Output the std::map into .cpp and .h files
    //

    ofstream h  ("rgb.h");
    ofstream cpp("rgb.cpp");
    ofstream dox("rgb.dox");

    h   << "#ifndef GLT_RGB_H\n#define GLT_RGB_H" << endl << endl;
    h   << "#include <glt/color.h>"               << endl << endl;

    cpp << "#include \"rgb.h\""                   << endl << endl;
    cpp << setprecision(5) << fixed;

    // Doxygen tags

    cpp   << "/*! \\file"                              << endl;
    cpp   << "    \\ingroup GLT"                       << endl;
    cpp   << "*/"                                     << endl;
    cpp   << endl;

    map<string,GltColor>::const_iterator i;

    for (i=color.begin(); i!=color.end(); i++)
    {
        const std::string &name  = i->first;
        const GltColor    &color = i->second;

        h   << "extern const GltColor " << name << ";" << endl;

        cpp << "const GltColor ";
        cpp.setf(ios::left,ios::adjustfield);
        cpp << setw(20) << name << "(";
        cpp << setw(7)  << color.red()   << ",";
        cpp << setw(7)  << color.green() << ",";
        cpp << setw(7)  << color.blue()  << ");  ";

        cpp << "/// \\htmlonly <table><tr><td width=\"300\" bgcolor=\"" << color.html() << "\">&nbsp;</td></tr></table> \\endhtmlonly"<< endl;
    }

    // Doxygen color table

    dox << "/*! \\file    glt/rgb.h"                 << endl;
    dox << "    \\brief   OpenGL Convenience Colors" << endl;
    dox << "    \\ingroup GLT"                       << endl;
    dox << "    \\htmlonly <table>"                  << endl;

    for (i=color.begin(); i!=color.end(); i++)
    {
        const std::string &name  = i->first;
        const GltColor    &color = i->second;

        dox << "<tr><td>" << name << "</td>";
        dox << "<td width=\"300\" bgcolor=\"" << color.html() << "\">&nbsp;</td>";
        dox << "<td>" << color.html() << "</td></tr>" << endl;
    }

    dox << "</table>"          << endl;
    dox << "    \\endhtmlonly" << endl;
    dox << "*/"                << endl << endl;

#if 0
    // Doxygen descriptions for all colors

    for (i=color.begin(); i!=color.end(); i++)
    {
        const std::string &name  = i->first;
        const GltColor    &color = i->second;

        dox << "/*! \\var " << name << endl;
        dox << "    \\brief \\htmlonly <table><tr><td width=\"300\" bgcolor=\"" << color.html() << "\">&nbsp;</td></tr></table> \\endhtmlonly"<< endl;
        dox << "*/";
        dox << endl;
    }
#endif

    //
    // Write arrays
    //

    const int size = color.size();

//  h << endl;
//  h << "extern const int       rgbSize;"                  << endl;
//  h << "extern const char     *rgbName [" << size << "];" << endl;
//  h << "extern const GltColor *rgbValue[" << size << "];" << endl;

    // Output names

    cpp << endl;
    cpp << "const int   GltColor::_rgbSize = " << size << ";"  << endl << endl;
    cpp << "const char *GltColor::_rgbName[" << size << "] =\n{\n\t";

    for (i=color.begin(); i!=color.end(); i++)
    {
        if (i!=color.begin())
            cpp << ",\n\t";

        const std::string &name  = i->first;
        cpp << "\"" << name << "\"";
    }

    cpp << "\n};" << endl << endl;

    // Output array of colours

    cpp << "const GltColor *GltColor::_rgbValue[" << size << "] =\n{\n\t";

    for (i=color.begin(); i!=color.end(); i++)
    {
        if (i!=color.begin())
            cpp << ",\n\t";

        const std::string &name  = i->first;
        cpp << "&::" << name;
    }

    cpp << "\n};" << endl << endl;

    //

    h   << "\n#endif" << endl;

    //
    // All done
    //

    return EXIT_SUCCESS;
}
