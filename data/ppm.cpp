#include "image.h"

/*! \file
    \brief   PPM image encoding and decoding
    \ingroup Misc

    $Id: ppm.cpp,v 1.6 2003/08/25 06:08:32 nigels Exp $

    $Log: ppm.cpp,v $
    Revision 1.6  2003/08/25 06:08:32  nigels
    Improved PPM error handling

    Revision 1.5  2003/05/10 17:04:06  nigels
    Handle ASCII ppm and pgm

    Revision 1.4  2003/03/06 12:34:47  nigels
    *** empty log message ***

*/

#include <misc/string.h>
#include <glt/error.h>

#include <cassert>
#include <cstdlib>
using namespace std;

bool
decodePPM(string &type,uint32 &width,uint32 &height,string &image,const string &data)
{
    if (data.size()<2 || data[0]!='P')
        return false;

    //

    bool   binary = false;
    size_t channels = 0;

    switch (data[1])
    {
        case '3':   // 24-bit ASCII RGB PPM file
            type     = "P3";
            binary   = false;
            channels = 3;
            break;

        case '5':   // 8-bit binary greyscale PGM file
            type     = "P5";
            binary   = true;
            channels = 1;
            break;

        case '6':   // 24-bit binary RGB PPM file
            type     = "P6";
            binary   = true;
            channels = 3;
            break;

        default:
            gltWarning("Unrecognised PPM type.");
            return false;
    }

    //

    const string eol("\n");
    const string digits("0123456789");

    size_t i = 0;

    int depth = 0;

    width = 0;
    height = 0;

    for (;;)
    {
        // Find end-of-line

        i = data.find_first_of(eol,i);
        if (i==string::npos)
            break;
        else
            i++;

        // If this line is a comment, try next line

        if (data[i]=='#')
            continue;

        // Get width

        width = atoi(data.c_str()+i);
        i = data.find_first_not_of(digits,i); if (i==string::npos) break;
        i = data.find_first_of(digits,i);     if (i==string::npos) break;

        // Get height

        height = atoi(data.c_str()+i);
        i = data.find_first_not_of(digits,i); if (i==string::npos) break;
        i = data.find_first_of(digits,i);     if (i==string::npos) break;

        // Get depth

        depth = atoi(data.c_str()+i);
        i = data.find(eol,i);
        if (i!=string::npos)
            i++;

        break;
    }

    // Check that PPM seems to be valid

    if (width==0)
    {
        gltWarning("PPM width is zero.");
        return false;
    }

    if (height==0)
    {
        gltWarning("PPM height is zero.");
        return false;
    }

    if (depth==0)
    {
        gltWarning("PPM depth is zero.");
        return false;
    }

    const uint32 imageSize = width*height*channels;

    if (width!=0 && height!=0 && depth!=0)
    {
        // Handle binary PPM data

        if (binary)
        {
            if (depth==255 && imageSize==data.size()-i)
            {
                // Extract image from input & flip
                flipImage(image,data.substr(i),width,height);
                return true;
            }
            else
            {
                gltWarning("Incomplete PPM data.");
            }
        }

        // Handle text PPM data

        else
        {
            if (depth==255)
            {
                std::string tmp;
                tmp.resize(imageSize);

                const char *src = &data[i          ];
                const char *end = &data[data.size()];

                byte *j    = reinterpret_cast<byte *>(&tmp[0]);
                byte *jEnd = j + imageSize;

                while (src<end && j<jEnd)
                {
                    *(j++) = ::atoi(src);

                    // Advance past current number
                    while (src<end && (*src>='0' && *src<='9'))
                        src++;

                    // Advance past whitespace to the start of next number
                    while (src<end && (*src==' ' || *src=='\n' || *src=='\t'))
                        src++;
                }

                // TODO - Could avoid flip by tricky iteration of output buffer
                flipImage(image,tmp,width,height);

                return src==end && j==jEnd;
            }
        }
    }

    return false;
}

bool
encodePPM(string &data,const uint32 width,const uint32 height,const string &image)
{
    const bool ppm = width*height*3==image.size();
    const bool pgm = width*height  ==image.size();

    // Check that image data matches the dimensions

    if (!ppm && !pgm)
        return false;

    // PPM P6/P5 header

    string header;
    sprintf(header,"%s\n%u %u\n255\n",ppm ? "P6" : "P5",width,height);

    // Assemble PPM file

    data.resize(header.size()+image.size());
    assert(data.size()==header.size()+image.size());
    strcpy((char *) data.data(),header.c_str());

    // Copy and flip

    const uint32 lineSize = ppm ? width*3 : width;

    // Destination pointer in data buffer
    byte *i = (byte *) data.data() + header.size();

    // Source pointer in image buffer
    const byte *j = (const byte *) image.data() + image.size() - lineSize;

    // Copy scan-lines until finished
    for (uint32 k=0; k<height; i+=lineSize, j-=lineSize, k++)
        memcpy(i,j,lineSize);

    return true;
}

