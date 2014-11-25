#include "image.h"

/*! \file
    \brief   Windows BMP image encoding and decoding
    \ingroup Misc

    $Id: bmp.cpp,v 1.6 2003/08/25 06:08:15 nigels Exp $

    $Log: bmp.cpp,v $
    Revision 1.6  2003/08/25 06:08:15  nigels
    *** empty log message ***

    Revision 1.5  2003/08/25 06:07:39  nigels
    Issues dealing with indexed BMP

    Revision 1.4  2003/07/22 03:55:30  nigels
    Added support for indexed BMP

    Revision 1.3  2003/03/06 12:34:46  nigels
    *** empty log message ***

*/

#include <misc/endian.h>
#include <glt/error.h>

#include <cassert>
using namespace std;

bool
decodeBMP(uint32 &width,uint32 &height,string &image,const string &data)
{
    //
    // For information about the BMP File Format:
    //
    // http://www.daubnet.com/formats/BMP.html
    // http://www.dcs.ed.ac.uk/home/mxr/gfx/2d/BMP.txt
    //

    const uint32 fileHeaderSize = 14;
    if (data.size()<fileHeaderSize)
        return false;

    // Check for "BM"

    if (data[0]!='B' || data[1]!='M')
        return false;

    // Check the filesize matches string size

    const uint32 fileSize = littleEndian((uint32 *)(data.data()+2));
    if (data.size()!=fileSize)
        return false;

    // Get the size of the image header

    const uint32 imageHeaderSize = littleEndian((uint32 *)(data.data()+fileHeaderSize));
    if (fileHeaderSize+imageHeaderSize>data.size())
        return false;

    // Get some image info

    const uint32 imageWidth    = littleEndian((uint32 *)(data.data()+fileHeaderSize+4 ));
    const uint32 imageHeight   = littleEndian((uint32 *)(data.data()+fileHeaderSize+8 ));
    const uint16 imageBits     = littleEndian((uint16 *)(data.data()+fileHeaderSize+14));
    const uint32 imageCompress = littleEndian((uint32 *)(data.data()+fileHeaderSize+16));

    // Do some checking

    // We support only RGB or indexed

    if (imageBits!=24 && imageBits!=8)
        return false;

    const uint32 bytesPerPixel    = imageBits>>3;
    const uint32 colorTableOffset = 54;

/*
    // We don't support compressed BMP.
    //
    // According to the specs, 4-bit and 8-bit RLE
    // compression is used only for indexed images.

    if (imageCompress!=0)
        return false;
*/

    const uint32 imagePos    = littleEndian((uint32 *)(data.data()+10));
    const uint32 imagePixels = imageWidth*imageHeight;
    const uint32 lineBytes   = (imageWidth*bytesPerPixel+3)&~3;
    const uint32 imageBytes  = lineBytes*imageHeight;

/*
    if (imagePos+imageBytes!=data.size())
        return false;
*/

    // Extract the image as RGB

    width  = imageWidth;
    height = imageHeight;
    image.resize(imagePixels*3);

    // Destination iterator

    byte *i = (byte *) image.data();

    // Uncompressed RGB

    if (bytesPerPixel==3 && imageCompress==0)
    {
        for (uint32 y=0; y<imageHeight; y++)
        {
            // Source iterator, beginning of y-th scanline

            const byte *j = (const byte *) data.data()+imagePos+lineBytes*y;

            // Copy the RGB scanline, swapping red and blue channels
            // as we go...

            for (uint32 x=0; x<imageWidth; x++)
            {
                *(i++) = *(j++ + 2);
                *(i++) = *(j++);
                *(i++) = *(j++ - 2);
            }
        }
    }

    // Uncompressed Indexed

    else if (bytesPerPixel==1 && imageCompress==0)
    {
        for (uint32 y=0; y<imageHeight; y++)
        {
            // Source iterator, beginning of y-th scanline

            const byte *j = (const byte *) data.data()+imagePos+lineBytes*y;

            // Copy the indexed scanline
            for (uint32 x=0; x<imageWidth; x++)
            {
                const byte *c = reinterpret_cast<const byte *>(data.data()+colorTableOffset+(*j)*4);
                *(i++) = c[2];
                *(i++) = c[1];
                *(i++) = c[0];
                j++;
            }
        }
    }

    else
    {
        gltWarning("Unsupported BMP variant.");
        assert(0);
        return false;
    }

    return true;
}

bool
encodeBMP(string &data,const uint32 width,const uint32 height,const string &image)
{
    assert(0);
    // TODO
    return false;
}
