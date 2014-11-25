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
    \brief   Utility for stitching images into one large combined image
    \ingroup Util
*/

#include <misc/image.h>
#include <misc/string.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdio>
using namespace std;

int dx = 0;
int dy = 0;
int n = 0;

vector<string> input;
string         output;

int main(int argc,char *argv[])
{
    if (argc<4)
    {
    }
    else
    {
        dx = atoi(argv[1]);
        dy = atoi(argv[2]);

        n = dx*dy;
        input.resize(n);

        cout << dx << 'x' << dy << endl;

        if (3+n>argc)
                return EXIT_FAILURE;

        string data;
        data.reserve(1024*1024*5);      // Optimise for images <5 Meg

        int lineSize=0;
        int w=0;
        int h=0;

        for (int i=0; i<n; i++)
        {
            uint32 width,height;

            cout << i << ' ' << argv[3+i] << ' ';
            cout.flush();

            data.erase();
            ifstream fin(argv[3+i],ios::binary);
            readStream(fin,data);
            if (!decode(width,height,input[i],data))
                return EXIT_FAILURE;

            cout << ' ' << width << ' ' << height << endl;
            cout.flush();

            lineSize = width*3;
            w = width*dx;
            h = height*dy;
        }

        cout << "Stitching " << w << 'x' << h << " ..." << endl;
        cout.flush();

        stitchImages(output,dx,dy,input,lineSize);
        data.erase();
        encodePPM(data,w,h,output);

        cout << "Writing..." << endl;
        ofstream fout("out.ppm",ios::binary);
        writeStream(fout,data);
    }

    return EXIT_SUCCESS;
}
