#include "image.h"

/*! \file
    \brief   JPEG image encoding and decoding
    \ingroup Misc

    $Id: jpeg.cpp,v 1.5 2003/07/22 03:58:58 nigels Exp $

    $Log: jpeg.cpp,v $
    Revision 1.5  2003/07/22 03:58:58  nigels
    *** empty log message ***

    Revision 1.4  2003/05/10 17:03:34  nigels
    *** empty log message ***

    Revision 1.3  2003/03/06 12:34:47  nigels
    *** empty log message ***

*/

#ifdef GLT_JPEG

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <math/real.h>

#include <cassert>
#include <string>
#include <list>
using namespace std;

#ifdef FAR
#undef FAR
#endif

extern "C" {
#include <jpeg/jpeglib.h>
}

/*!
    \class   GltJpgReader
    \brief   JPEG Decoder
    \ingroup Misc
    \author  Nigel Stewart, RMIT (nigels@nigels.com)

    This class is used internally by GLT to decode
    JPEG data using the JPEG library.
*/

class GltJpgReader
{
public:

    GltJpgReader(const string &data,jpeg_decompress_struct &cinfo)
    {
        src.manager.init_source       = init_source;
        src.manager.fill_input_buffer = fill_input_buffer;
        src.manager.skip_input_data   = skip_input_data;
        src.manager.resync_to_restart = jpeg_resync_to_restart;
        src.manager.term_source       = term_source;

        src.data = &data;

        cinfo.src = reinterpret_cast<jpeg_source_mgr *>(&src);
    }

    ~GltJpgReader()
    {
    }

protected:

    typedef struct
    {
        jpeg_source_mgr        manager;
        const string          *data;
    }
    source_mgr;

    source_mgr src;

    // jpeglib callbacks

    /// Initialise source
    static void init_source(j_decompress_ptr cinfo)
    {
        assert(cinfo);
        source_mgr *src = reinterpret_cast<source_mgr *>(cinfo->src);
        assert(src);

        const string &data = *(src->data);

        src->manager.bytes_in_buffer = data.size();
        src->manager.next_input_byte = reinterpret_cast<const JOCTET *>(&data[0]);

    }

    /// Read fresh data into the buffer
    static boolean fill_input_buffer(j_decompress_ptr cinfo)
    {
        return TRUE;
    }

    /// Skip count bytes of data in buffer
    static void skip_input_data(j_decompress_ptr cinfo, long count)
    {
        assert(cinfo);
        source_mgr *src = reinterpret_cast<source_mgr *>(cinfo->src);
        assert(src);

        if (count>0)
        {
            assert(src->manager.bytes_in_buffer>count);
            src->manager.bytes_in_buffer -= count;
            src->manager.next_input_byte += count;
        }
    }

    /// Finished reading from source
    static void term_source(j_decompress_ptr cinfo)
    {
    }
};

bool
decodeJPG(uint32 &width,uint32 &height,std::string &image,const std::string &data)
{
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr         jerr;

    // Setup error handler, decompression object and (custom) input source
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    GltJpgReader read(data,cinfo);

    // read file parameters with jpeg_read_header()
    if (jpeg_read_header(&cinfo, TRUE)!=JPEG_HEADER_OK)
    {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    // Start decompressor
    jpeg_start_decompress(&cinfo);

    // Get image data
    width  = cinfo.image_width;
    height = cinfo.image_height;

    // Allocate memory for buffer
    unsigned short rowspan = cinfo.image_width * cinfo.num_components;
    image.resize(rowspan*height);

    // Create array of row pointers for lib
    byte **rowPtr = new byte * [height];
    for( unsigned i = 0; i < height; i++ )
        rowPtr[i] = reinterpret_cast< byte * >(&image[(height-1-i)*rowspan]);

    // Read the scanlines
    unsigned rowsRead = 0;
    while (cinfo.output_scanline < cinfo.output_height)
        rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );

    // Free resources
    delete [] rowPtr;
    jpeg_finish_decompress(&cinfo);

    // Release JPEG decompression object
    jpeg_destroy_decompress(&cinfo);

    return true;
}

///////////////////////////////////////////////////////////////////

/*!
    \class   GltJpgWriter
    \brief   JPEG Encoder
    \ingroup Misc
    \author  Nigel Stewart, RMIT (nigels@nigels.com)

    This class is used internally by GLT to encode
    JPEG data using the JPEG library.
*/

class GltJpgWriter
{
public:

    GltJpgWriter(string &data,jpeg_compress_struct &cinfo,const uint32 blockSize = 10240)
    {
        // Completely initialise JPEG destination manager
        // struct and pass it to the compression context

        dest.manager.init_destination    = init_destination;
        dest.manager.empty_output_buffer = empty_output_buffer;
        dest.manager.term_destination    = term_destination;
        dest.manager.next_output_byte    = NULL;
        dest.data                        = &data;
        dest.size                        = 0;
        dest.blockSize                   = 10240;

        cinfo.dest = reinterpret_cast<jpeg_destination_mgr *>(&dest);
    }

    ~GltJpgWriter()
    {
    }

protected:

    typedef struct
    {
        jpeg_destination_mgr   manager;
        string                *data;
        list<string>           blocks;
        size_t                 size;
        size_t                 blockSize;
    }
    destination_mgr;

    /// Destination manager struct for use by JPEG library
    destination_mgr dest;

    /// JPEG Callback for initialising the output buffer
    static void init_destination(j_compress_ptr cinfo)
    {
        empty_output_buffer(cinfo);
    }

    /// JPEG Callback for allocating a new buffer to output into
    static boolean empty_output_buffer(j_compress_ptr cinfo)
    {
        // Obtain pointer to destination manager
        assert(cinfo);
        destination_mgr *dest = reinterpret_cast<destination_mgr *>(cinfo->dest);
        assert(dest);

        // Increment total count, if necessary
        if (dest->manager.next_output_byte)
            dest->size += dest->blockSize;

        // Append a new block to the list of appropriate size
        dest->blocks.push_back(string());
        dest->blocks.back().resize(dest->blockSize);

        // Set new block for output
        dest->manager.free_in_buffer   = dest->blockSize;
        dest->manager.next_output_byte = reinterpret_cast<JOCTET *>(&dest->blocks.back()[0]);

        return TRUE;
    }

    /// JPEG Callback for finally flushing the output
    static void term_destination(j_compress_ptr cinfo)
    {
        // Obtain pointer to destination manager
        assert(cinfo);
        destination_mgr *dest = reinterpret_cast<destination_mgr *>(cinfo->dest);
        assert(dest);

        // Increment total count, if necessary
        if (dest->manager.next_output_byte)
            dest->size += dest->blockSize - dest->manager.free_in_buffer;

        // Copy each block into output buffer
        if (dest->size>0)
        {
            // Allocate memory for output
            dest->data->resize(dest->size);

            list<string>::iterator i = dest->blocks.begin();    // Block iterator
            uint32                 j = dest->size;              // Countdown to zero
            char                  *k = &(*dest->data)[0];       // Output pointer

            // Traverse all the blocks and copy into output
            while (i!=dest->blocks.end() && j>0)
            {
                memcpy(k,i->data(),MIN(i->size(),j));
                j -= i->size();     // Decrease remaining bytes
                k += i->size();     // Advance output pointer
                i++;                // Increment block iterator
            }
        }

        // Reset the destination manager
        memset(dest,sizeof(destination_mgr),0);
    }
};

bool encodeJPG(std::string &data,const uint32 &width,const uint32 &height,const std::string &image,const uint32 quality)
{
    // Check the size of image buffer
    const uint32 size = width*height;
    if (image.size()%size!=0)
        return false;

    // Only RGB or GREYSCALE can be encoded
    const uint32 components = image.size()/size;
    if (components!=1 && components!=3)
        return false;

    //

    jpeg_compress_struct cinfo;
    jpeg_error_mgr       jerr;


    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    GltJpgWriter writer(data,cinfo);

    // Set compression parameters

    cinfo.image_width  = width;
    cinfo.image_height = height;
    cinfo.input_components = image.size()/size;
    cinfo.in_color_space = cinfo.input_components==3 ? JCS_RGB : JCS_GRAYSCALE;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, clamp(quality,5u,100u), TRUE); // 5 to 100

    // Compress
    jpeg_start_compress(&cinfo, TRUE);

    // bytes per row
    const int rowSize = cinfo.image_width*cinfo.input_components;

    char *row_pointer[1];
    int line = cinfo.image_height;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = const_cast<char *>(&image[(--line)*rowSize]);
        jpeg_write_scanlines(&cinfo, reinterpret_cast<byte **>(row_pointer), 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return true;
}

#endif

