#include "image.h"

/*! \file
    \brief   PNG image encoding and decoding
    \ingroup Misc

    $Id: png.cpp,v 1.4 2003/05/10 17:03:46 nigels Exp $

    $Log: png.cpp,v $
    Revision 1.4  2003/05/10 17:03:46  nigels
    int -> uint32

    Revision 1.3  2003/03/06 12:34:47  nigels
    *** empty log message ***

*/

#include <list>
#include <cassert>
using namespace std;

//

#ifdef GLT_PNG

#include <png.h>

class GltPngReader
{
public:

    GltPngReader(const std::string &data,png_structp png_ptr)
    : pos(data.c_str()), end(data.c_str()+data.size())
    {
        png_set_read_fn(png_ptr,this,(png_rw_ptr) &GltPngReader::read);
    }

    size_t remaining() const { return end-pos; }

private:

    static void read(png_structp png_ptr, png_bytep data, png_uint_32 length)
    {
        GltPngReader *reader = (GltPngReader *) png_get_io_ptr(png_ptr);

        assert(reader);
        if (!reader)
            return;

        const char *&pos = reader->pos;
        const char *&end = reader->end;

        assert(pos);
        assert(end);

        if (pos && end)
        {
            if (pos+length<=end)
            {
                memcpy(data,pos,length);
                pos += length;
            }
            else
            {
                memcpy(data,pos,end-pos);
                pos = end = NULL;
            }
        }
    }

    const char *pos;
    const char *end;
};

class GltPngWriter
{
public:

    GltPngWriter(std::string &data,png_structp png_ptr)
    : _png(png_ptr), _data(data), _size(0)
    {
        png_set_write_fn(_png,this,(png_rw_ptr) &GltPngWriter::write,(png_flush_ptr) &GltPngWriter::flush);
    }

    ~GltPngWriter()
    {
        GltPngWriter::flush(_png);
        png_set_write_fn(_png,NULL,NULL,NULL);
    }

private:

    static void write(png_structp png_ptr, png_bytep data, png_uint_32 length)
    {
        GltPngWriter *writer = (GltPngWriter *) png_get_io_ptr(png_ptr);

        assert(writer);
        if (!writer)
            return;

        list<string> &blocks = writer->_blocks;
        size_t       &size   = writer->_size;

        if (length>0)
        {
            blocks.push_back(string());
            string &block = blocks.back();
            block.resize(length);
            memcpy((void *) block.data(),data,length);
            size += length;
        }
    }

    static void flush(png_structp png_ptr)
    {
        GltPngWriter *writer = (GltPngWriter *) png_get_io_ptr(png_ptr);

        assert(writer);
        if (!writer)
            return;

        string       &data   = writer->_data;
        list<string> &blocks = writer->_blocks;
        size_t       &size   = writer->_size;

        if (size>0)
        {
            size_t begin = data.size();
            data.resize(begin+size);
            for (list<string>::iterator i=blocks.begin(); i!=blocks.end(); i++)
            {
                memcpy((void *)(data.data()+begin),(void *) i->data(),i->size());
                begin += i->size();
            }
            blocks.clear();
            size = 0;
        }
    }

    png_structp   _png;
    string       &_data;
    list<string>  _blocks;
    size_t        _size;
};

//

bool
decodePNG(uint32 &width,uint32 &height,std::string &image,const std::string &data)
{
    const char *pngSignature = "\211PNG\r\n\032\n";
    if (data.size()<8 || strncmp(data.c_str(),pngSignature,8))
        return false;

    png_structp png_ptr =
        png_create_read_struct(PNG_LIBPNG_VER_STRING,(png_voidp) NULL,NULL,NULL);

    if (!png_ptr)
        return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
            return false;
    }

//  png_init_io(png_ptr, fp);

    GltPngReader read(data,png_ptr);

    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type, interlace_type,
        compression_type, filter_type;

    png_uint_32 w,h;

    png_get_IHDR(png_ptr, info_ptr, &w, &h,
                 &bit_depth, &color_type, &interlace_type,
                 &compression_type, &filter_type);

    width = w;
    height = h;

    // TODO - Handle other bit-depths

    if (bit_depth<8)
        png_set_packing(png_ptr);

    if (bit_depth==16)
        png_set_strip_16(png_ptr);

    int channels = png_get_channels(png_ptr,info_ptr);

    switch (color_type)
    {
        case PNG_COLOR_TYPE_GRAY:       image.resize(width*height);   break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: image.resize(width*height*2); break;
        case PNG_COLOR_TYPE_RGB:        image.resize(width*height*3); break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  image.resize(width*height*4); break;
        case PNG_COLOR_TYPE_PALETTE:
            image.resize(width*height*3);
            png_set_palette_to_rgb(png_ptr);
            break;
    }

    int rowbytes = image.size()/height;
    assert(image.size()==rowbytes*height);

    const char **row_pointers = (const char **) malloc(height*sizeof(char *));
    for(uint32 i=0;i<height;i++)
        row_pointers[i] = image.c_str() + rowbytes*(height-1-i);

    png_read_image(png_ptr, (png_bytepp)row_pointers);

    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    free(row_pointers);

//    free(info_ptr);
//    free(png_ptr);

    return true;
}

bool
encodePNG(std::string &data,const uint32 &width,const uint32 &height,const std::string &image)
{
    png_structp png_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING,(png_voidp) NULL,NULL,NULL);

    if (!png_ptr)
        return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
            return false;
    }

    {
        GltPngWriter write(data,png_ptr);

        const int channels   = image.size()/(width*height);
        const int bit_depth  = 8;
              int color_type = 0;

        switch (channels)
        {
            case 1: color_type = PNG_COLOR_TYPE_GRAY;       break;
            case 2: color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
            case 3: color_type = PNG_COLOR_TYPE_RGB;        break;
            case 4: color_type = PNG_COLOR_TYPE_RGB_ALPHA;  break;
        }

        assert(color_type);

        png_set_IHDR(
            png_ptr,info_ptr,width,height,bit_depth,color_type,
            PNG_INTERLACE_ADAM7,PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_set_strip_16(png_ptr);
        png_set_packing(png_ptr);

        int rowbytes = image.size()/height;
        assert(image.size()==rowbytes*height);

        const char **row_pointers = (const char **) malloc(height*sizeof(char *));
        for(uint32 i=0;i<height;i++)
            row_pointers[i] = image.c_str() + rowbytes*(height-1-i);

        png_write_info(png_ptr, info_ptr);
        png_write_image(png_ptr,(png_bytepp) row_pointers);
        png_write_end(png_ptr,info_ptr);

        free(row_pointers);
    }

    free(info_ptr);
    free(png_ptr);

    return true;
}

#endif

