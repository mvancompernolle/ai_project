#include "image.h"

/*! \file
    \brief   TGA image encoding and decoding
    \ingroup Misc

    \todo Graceful warning/failure

    $Id: tga.cpp,v 1.5 2003/08/25 06:09:00 nigels Exp $

    $Log: tga.cpp,v $
    Revision 1.5  2003/08/25 06:09:00  nigels
    Added RLE indexed support

    Revision 1.4  2003/07/22 03:58:58  nigels
    *** empty log message ***

    Revision 1.3  2003/05/10 17:04:23  nigels
    *** empty log message ***

    Revision 1.2  2003/03/06 12:34:47  nigels
    *** empty log message ***

*/

#include <glt/error.h>

#include <misc/endian.h>
#include <misc/string.h>

#include <cassert>
#include <iostream>
using namespace std;

bool
decodeTGA(uint32 &width,uint32 &height,string &image,const string &data)
{
    // Check that header appears to be valid

    const uint32 tgaHeaderSize = 18;
    if (data.size()<tgaHeaderSize)
    {
        gltWarning("TGA Header seems to be invalid.");
        return false;
    }

    // We only handle type 1, 2 and 10, for the moment

    const uint32 tgaType = data[2];

    if (tgaType!=1 && tgaType!=2 && tgaType!=9 && tgaType!=10)
    {
        string message;
        sprintf(message,"Found TGA type: %d", tgaType);
//      gltWarning(message);
        return false;
    }

    // There should be a color map for type 1 and 9

    if ((tgaType==1 || tgaType==9) && data[1]!=1)
    {
        gltWarning("Expecting color map in type 1 TGA.");
        return false;
    }

    // We only handle 24 bit or 32 bit images, for the moment

    uint32 bytesPerPixel=0;

    if (tgaType==1 || tgaType==9)
        bytesPerPixel = data[7]==24 ? 3 : 4;    // Color mapped
    else
        bytesPerPixel = data[16]==24 ? 3 : 4;   // Unmapped

    if (bytesPerPixel!=3 && bytesPerPixel!=4)
    {
        cerr << "Found TGA pixel depth: " << bytesPerPixel << endl;
        return false;
    }

    // Color map information

    const byte idSize = data[0];    // Upto 255 characters of text

    const uint16 origin   = littleEndian((uint16 *)(data.data()+3));
    const uint16 length   = littleEndian((uint16 *)(data.data()+5));
    const byte   size     = data[7]>>3;
    const uint32 mapSize  = length*size;
    const byte   *mapData = (const byte *) data.data()+tgaHeaderSize+idSize;

    //

    width  = littleEndian((uint16 *)(data.data()+12));
    height = littleEndian((uint16 *)(data.data()+14));

    // Check if TGA file seems to be the right size
    // (TGA allows for descriptive junk at the end of the
    //  file, we just ignore it)

    switch (tgaType)
    {
    case 1:     // Indexed
        {
            // TODO
        }
        break;

    case 2:     // Unmapped RGB(A)
        {
            if (data.size()<tgaHeaderSize+idSize+width*height*bytesPerPixel)
                return false;
        }
        break;

    case 9:    // Compressed Indexed
        {
            // TODO
        }
        break;

    case 10:    // Compressed RGB(A)
        {
            // TODO
        }
        break;
    }


    image.resize(width*height*bytesPerPixel);

    // Destination iterator

          byte *i = (byte *) image.data();                                      // Destination
    const byte *j = (const byte *) data.data()+tgaHeaderSize+idSize+mapSize;    // Source

    switch (tgaType)
    {

        // Indexed

        case 1:
            {
                const uint32 pixels = width*height;

                for (uint32 p=0; p<pixels; p++)
                {
                    const byte *entry = mapData+(*(j++))*bytesPerPixel;

                    *(i++) = entry[2];
                    *(i++) = entry[1];
                    *(i++) = entry[0];

                    if (bytesPerPixel==4)
                        *(i++) = entry[3];
                }
            }
            break;

        // Unmapped RGB(A)

        case 2:
            {
                for (uint32 y=0; y<height; y++)
                {
                    // Copy the scanline, swapping red and blue channels
                    // as we go...

                    for (uint32 x=0; x<width; x++)
                    {
                        *(i++) = *(j++ + 2);
                        *(i++) = *(j++);
                        *(i++) = *(j++ - 2);

                        if (bytesPerPixel==4)
                            *(i++) = *(j++);    // Copy alpha
                    }
                }
            }
            break;

        // Run Length Encoded, Indexed

        case 9:
            {
                const byte *iMax = (const byte *) image.data()+image.size();
                const byte *jMax = (const byte *) data.data()+data.size();

                while (i<iMax && j<jMax)
                {
                    const bool rle   = ((*j)&128)==128;
                    const int  count = ((*j)&127) + 1;

                   j++;

                    // Check if we're running out of output buffer
                    if (i+count*bytesPerPixel>iMax)
                    {
                        gltWarning("TGA Output Buffer Full.");
                        return false;
                    }

                    if (rle)
                    {
                        // Check if we're running out of input data
                        if (j>jMax)
                        {
                            gltWarning("TGA Input Buffer Empty.");
                            return false;
                        }

                        // Get the next pixel, swapping red and blue

                        const byte *pixel = i;
                        const byte *entry = mapData+(*(j++))*bytesPerPixel;

                        *(i++) = entry[2];
                        *(i++) = entry[1];
                        *(i++) = entry[0];

                        if (bytesPerPixel==4)
                            *(i++) = entry[3];

                        // Now duplicate for rest of the run

                        for (int k=0; k<count-1; k++,i+=bytesPerPixel)
                            memcpy(i,pixel,bytesPerPixel);
                    }
                    else
                    {
                        // Check if we're running out of input data
                        if (j+count>jMax)
                        {
                            gltWarning("TGA Input Buffer Empty.");
                            return false;
                        }

                        for (int k=0; k<count; k++)
                        {
                            const byte *entry = mapData+(*(j++))*bytesPerPixel;

                            *(i++) = entry[2];
                            *(i++) = entry[1];
                            *(i++) = entry[0];

                            if (bytesPerPixel==4)
                                *(i++) = entry[3];
                         }
                    }
                }

                if (i!=iMax)
                    return false;
            }
            break;

        // Run Length Encoded, RGB(A) images

        case 10:
            {
                const byte *iMax = (const byte *) image.data()+image.size();
                const byte *jMax = (const byte *) data.data()+data.size();

                while (i<iMax && j<jMax)
                {
                    const bool rle   = ((*j)&128)==128;
                    const int  count = ((*j)&127) + 1;

                    j++;

                    // Check if we're running out of output buffer
                    if (i+count*bytesPerPixel>iMax)
                        return false;

                    if (rle)
                    {
                        // Check if we're running out of input data
                        if (j+bytesPerPixel>jMax)
                            return false;

                        byte *pixel = i;

                        // Get the next pixel, swapping red and blue

                        *(i++) = *(j++ + 2);
                        *(i++) = *(j++);
                        *(i++) = *(j++ - 2);

                        if (bytesPerPixel==4)
                            *(i++) = *(j++);    // Copy alpha

                        // Now duplicate for rest of the run

                        for (int k=0; k<count-1; k++,i+=bytesPerPixel)
                            memcpy(i,pixel,bytesPerPixel);
                    }
                    else
                    {
                        // Check if we're running out of input data
                        if (j+count*bytesPerPixel>jMax)
                            return false;

                        // Copy pixels, swapping red and blue as
                        // we go...

                        for (int k=0; k<count; k++)
                        {
                            *(i++) = *(j++ + 2);
                            *(i++) = *(j++);
                            *(i++) = *(j++ - 2);

                            if (bytesPerPixel==4)
                                *(i++) = *(j++);    // Copy alpha
                        }

                    }
                }

                if (i!=iMax)
                    return false;
            }
            break;
    }

    // If the TGA origin is top-right
    // flip image

    if ((data[17]&32)==32)
    {
        // TODO - in-place flip
        string tmp = image;
        flipImage(image,tmp,width,height);
    }

    return true;
}

bool
encodeTGA(string &data,const uint32 width,const uint32 height,const string &image)
{
    // Check that image data matches the
    // dimensions

    if (width*height*3!=image.size())
        return false;

    // Allocate space for header and image
    data.resize(18+image.size());

    data[0] = 0;
    data[1] = 0;
    data[2] = 2;
    data[3] = 0;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 0;
    data[10] = 0;
    data[11] = 0;
    *(uint16 *) (data.data()+12) = littleEndian(uint16(width));
    *(uint16 *) (data.data()+14) = littleEndian(uint16(height));
    data[16] = 24;
    data[17] = 0;

    // Destination iterator

          byte *i = (byte *) data.data()+18;                // Destination
    const byte *j = (const byte *) image.data();            // Source

    for (uint32 y=0; y<height; y++)
    {
        // Copy the scanline, swapping red and blue channels
        // as we go...

        for (uint32 x=0; x<width; x++)
        {
            *(i++) = *(j++ + 2);
            *(i++) = *(j++);
            *(i++) = *(j++ - 2);
        }
    }

//-------------------------------------------------------------------------------
//DATA TYPE 2:  Unmapped RGB images.                                             |
//_______________________________________________________________________________|
//| Offset | Length |                     Description                            |
//|--------|--------|------------------------------------------------------------|
//|--------|--------|------------------------------------------------------------|
//|    0   |     1  |  Number of Characters in Identification Field.             |
//|        |        |                                                            |
//|        |        |  This field is a one-byte unsigned integer, specifying     |
//|        |        |  the length of the Image Identification Field.  Its value  |
//|        |        |  is 0 to 255.  A value of 0 means that no Image            |
//|        |        |  Identification Field is included.                         |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//|    1   |     1  |  Color Map Type.                                           |
//|        |        |                                                            |
//|        |        |  This field contains either 0 or 1.  0 means no color map  |
//|        |        |  is included.  1 means a color map is included, but since  |
//|        |        |  this is an unmapped image it is usually ignored.  TIPS    |
//|        |        |  ( a Targa paint system ) will set the border color        |
//|        |        |  the first map color if it is present.                     |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//|    2   |     1  |  Image Type Code.                                          |
//|        |        |                                                            |
//|        |        |  This field will always contain a binary 2.                |
//|        |        |  ( That's what makes it Data Type 2 ).                     |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//|    3   |     5  |  Color Map Specification.                                  |
//|        |        |                                                            |
//|        |        |  Ignored if Color Map Type is 0; otherwise, interpreted    |
//|        |        |  as follows:                                               |
//|        |        |                                                            |
//|    3   |     2  |  Color Map Origin.                                         |
//|        |        |  Integer ( lo-hi ) index of first color map entry.         |
//|        |        |                                                            |
//|    5   |     2  |  Color Map Length.                                         |
//|        |        |  Integer ( lo-hi ) count of color map entries.             |
//|        |        |                                                            |
//|    7   |     1  |  Color Map Entry Size.                                     |
//|        |        |  Number of bits in color map entry.  16 for the Targa 16,  |
//|        |        |  24 for the Targa 24, 32 for the Targa 32.                 |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//|    8   |    10  |  Image Specification.                                      |
//|        |        |                                                            |
//|    8   |     2  |  X Origin of Image.                                        |
//|        |        |  Integer ( lo-hi ) X coordinate of the lower left corner   |
//|        |        |  of the image.                                             |
//|        |        |                                                            |
//|   10   |     2  |  Y Origin of Image.                                        |
//|        |        |  Integer ( lo-hi ) Y coordinate of the lower left corner   |
//|        |        |  of the image.                                             |
//|        |        |                                                            |
//|   12   |     2  |  Width of Image.                                           |
//|        |        |  Integer ( lo-hi ) width of the image in pixels.           |
//|        |        |                                                            |
//|   14   |     2  |  Height of Image.                                          |
//|        |        |  Integer ( lo-hi ) height of the image in pixels.          |
//|        |        |                                                            |
//|   16   |     1  |  Image Pixel Size.                                         |
//|        |        |  Number of bits in a pixel.  This is 16 for Targa 16,      |
//|        |        |  24 for Targa 24, and .... well, you get the idea.         |
//|        |        |                                                            |
//|   17   |     1  |  Image Descriptor Byte.                                    |
//|        |        |  Bits 3-0 - number of attribute bits associated with each  |
//|        |        |             pixel.  For the Targa 16, this would be 0 or   |
//|        |        |             1.  For the Targa 24, it should be 0.  For     |
//|        |        |             Targa 32, it should be 8.                      |
//|        |        |  Bit 4    - reserved.  Must be set to 0.                   |
//|        |        |  Bit 5    - screen origin bit.                             |
//|        |        |             0 = Origin in lower left-hand corner.          |
//|        |        |             1 = Origin in upper left-hand corner.          |
//|        |        |             Must be 0 for Truevision images.               |
//|        |        |  Bits 7-6 - Data storage interleaving flag.                |
//|        |        |             00 = non-interleaved.                          |
//|        |        |             01 = two-way (even/odd) interleaving.          |
//|        |        |             10 = four way interleaving.                    |
//|        |        |             11 = reserved.                                 |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//|   18   | varies |  Image Identification Field.                               |
//|        |        |  Contains a free-form identification field of the length   |
//|        |        |  specified in byte 1 of the image record.  It's usually    |
//|        |        |  omitted ( length in byte 1 = 0 ), but can be up to 255    |
//|        |        |  characters.  If more identification information is        |
//|        |        |  required, it can be stored after the image data.          |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//| varies | varies |  Color map data.                                           |
//|        |        |                                                            |
//|        |        |  If the Color Map Type is 0, this field doesn't exist.     |
//|        |        |  Otherwise, just read past it to get to the image.         |
//|        |        |  The Color Map Specification describes the size of each    |
//|        |        |  entry, and the number of entries you'll have to skip.     |
//|        |        |  Each color map entry is 2, 3, or 4 bytes.                 |
//|        |        |                                                            |
//|--------|--------|------------------------------------------------------------|
//| varies | varies |  Image Data Field.                                         |
//|        |        |                                                            |
//|        |        |  This field specifies (width) x (height) pixels.  Each     |
//|        |        |  pixel specifies an RGB color value, which is stored as    |
//|        |        |  an integral number of bytes.                              |
//|        |        |                                                            |
//|        |        |  The 2 byte entry is broken down as follows:               |
//|        |        |  ARRRRRGG GGGBBBBB, where each letter represents a bit.    |
//|        |        |  But, because of the lo-hi storage order, the first byte   |
//|        |        |  coming from the file will actually be GGGBBBBB, and the   |
//|        |        |  second will be ARRRRRGG. "A" represents an attribute bit. |
//|        |        |                                                            |
//|        |        |  The 3 byte entry contains 1 byte each of blue, green,     |
//|        |        |  and red.                                                  |
//|        |        |                                                            |
//|        |        |  The 4 byte entry contains 1 byte each of blue, green,     |
//|        |        |  red, and attribute.  For faster speed (because of the     |
//|        |        |  hardware of the Targa board itself), Targa 24 images are  |
//|        |        |  sometimes stored as Targa 32 images.                      |
//|        |        |                                                            |
//--------------------------------------------------------------------------------

    return true;
}
