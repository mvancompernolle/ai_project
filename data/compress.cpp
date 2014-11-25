#include "compress.h"

/*

  Lzf compression library
  Copyright (c) 2000 Marc Alexander Lehmann <pcg@goof.com>
  An extremely fast/free compression/decompression-method
  http://liblzf.plan9.de/

  Based on ideas by Hermann Vogt, but liblzf is a total
  re-implementation of LZV that is not compatible to the
  original lzv code.

  This algorithm is believed to be patent-free.

*/

/*! \file
    \brief   Compression routines
    \ingroup Misc

    $Id: compress.cpp,v 1.16 2003/07/29 08:33:19 nigels Exp $

    $Log: compress.cpp,v $
    Revision 1.16  2003/07/29 08:33:19  nigels
    *** empty log message ***

    Revision 1.15  2003/05/10 17:02:33  nigels
    *** empty log message ***

    Revision 1.13  2003/03/06 12:34:46  nigels
    *** empty log message ***

*/

#include <glt/config.h>

extern "C"
{
    #include "internal/lzf.h"
}

#include <misc/string.h>
#include <glt/error.h>

#ifdef GLT_ZLIB
#include <zlib.h>
#endif

#include <cstdio>
#include <cassert>
using namespace std;

////////////////////////////////////////////////////////////////////////////

// Room for more compression algorithms

#define COMPRESS_NONE   0
#define COMPRESS_LIBLZF 1
#define COMPRESS_ZLIB   2

// Create variable-length header for compressed data

bool makeHeader
(
    string &header,
    const uint32 algorithm,
    const uint32 sizeCompressed,
    const uint32 sizeDecompressed,
    const uint32 crc32
)
{
    sprintf(header,"GLTZ %u %u %u %u",algorithm,sizeCompressed,sizeDecompressed,crc32);
    header += '\0';
    return true;
}

// Decode variable-length header for compressed data

const byte * const getHeader
(
    const char * const h,
    uint32 &algorithm,
    uint32 &sizeCompressed,
    uint32 &sizeDecompressed,
    uint32 &crc32
)
{
    if (h[0]=='G' && h[1]=='L' && h[2]=='T' && h[3]=='Z' && h[4]==' ')
    {
        if (sscanf(h+5,"%u %u %u %u",&algorithm,&sizeCompressed,&sizeDecompressed,&crc32)==4)
            return (const byte * const) h + strlen(h) + 1;
    }

    return NULL;
}

bool compress(string &dest,const string &src)
{
    // Allocate a buffer for receiving the compressed data

    byte *buffer = new byte[src.size()];
    if (!buffer)
        return false;

    // Flag success

    bool ret = true;

    // Do compression

    unsigned int size = lzf_compress(src.data(),src.size(),buffer,src.size());

    // Get CRC32
    uint32 crc = 0;

    #ifdef GLT_ZLIB
    crc = ::crc32(src);
    #endif

    if (size==0 || size==src.size())
    {
        // If compression fails, just copy the input buffer

        string header;
        if (makeHeader(header,COMPRESS_NONE,src.size(),src.size(),crc))
            dest = header + src;
        else
            ret = false;
    }
    else
    {
        // If compression succeeds, copy the compressed buffer

        if (makeHeader(dest,COMPRESS_LIBLZF,size,src.size(),crc))
        {
            size_t pos = dest.size();
            dest.resize(pos+size);
            memcpy((void *)(dest.data()+pos),buffer,size);
        }
        else
            ret = false;
    }

    // Exit

    delete [] buffer;
    return ret;
}

bool decompress(string &dest,const string &src)
{
    uint32 algorithm;
    uint32 sizeCompressed;
    uint32 sizeDecompressed;
    uint32 crc32;

    const byte * const pos = getHeader(src.data(),algorithm,sizeCompressed,sizeDecompressed,crc32);

    if (pos)
    {
        // Prepare output buffer

        dest.resize(sizeDecompressed);

        switch (algorithm)
        {
        case COMPRESS_NONE:
            {
                // Data is not compressed, just strip off header
                memcpy((void *) dest.data(),pos,sizeCompressed);
                return true;
            }

        case COMPRESS_LIBLZF:
            {
                // Data is compressed with lzf
                unsigned int ret = lzf_decompress(pos,sizeCompressed,(void *) dest.data(),sizeDecompressed);
                return ret==sizeDecompressed;
            }

        case COMPRESS_ZLIB:
            {
                #ifdef GLT_ZLIB
                uLongf size = sizeDecompressed;
                int ok = uncompress((Bytef *) dest.data(),&size,pos,sizeCompressed);
                return ok==Z_OK && size==sizeDecompressed;
                #else
                assert(false);
                return false;
                #endif
            }
        }
    }

    assert(false);
    return false;
}

bool decompress(string &dest,const void *s)
{
    uint32 algorithm;
    uint32 sizeCompressed;
    uint32 sizeDecompressed;
    uint32 crc32;

    const byte * const pos = getHeader((const char * const) s,algorithm,sizeCompressed,sizeDecompressed,crc32);

    if (pos)
    {
        // Prepare output buffer

        dest.resize(sizeDecompressed);

        switch (algorithm)
        {
        case COMPRESS_NONE:
            {
                // Data is not compressed, just strip off header
                memcpy((void *) dest.data(),pos,sizeCompressed);

                #ifdef GLT_ZLIB
                if (::crc32(dest)!=crc32)
                {
                    gltWarning("CRC error check failed for uncompressed data.");
                    return false;
                }
                #endif

                return true;
            }

        case COMPRESS_LIBLZF:
            {
                // Data is compressed with lzf
                unsigned int ret = lzf_decompress(pos,sizeCompressed,(void *) dest.data(),sizeDecompressed);

                #ifdef GLT_ZLIB
                if (::crc32(dest)!=crc32)
                {
                    gltWarning("CRC error check failed for LZF compressed data.");
                    return false;
                }
                #endif

                return ret==sizeDecompressed;
            }

        case COMPRESS_ZLIB:
            {
                #ifdef GLT_ZLIB
                uLongf size = sizeDecompressed;
                int ok = uncompress((Bytef *) dest.data(),&size,pos,sizeCompressed);

                if (::crc32(dest)!=crc32)
                {
                    gltWarning("CRC error check failed for zLib compressed data.");
                    return false;
                }

                return ok==Z_OK && size==sizeDecompressed;
                #else
                gltError("Zlib support not compiled into GLT.");
                assert(false);
                return false;
                #endif
            }
        }
    }

    assert(false);
    return false;
}

////////////////////////////////

#ifdef GLT_ZLIB

bool compressZLib(string &dest,const string &src,const int level)
{
    // Allocate a buffer for receiving the compressed data

    byte *buffer = new byte[src.size()];
    if (!buffer)
        return false;

    // Flag success

    bool ret = true;

    // Do compression
    uLongf bufferSize = src.size();
    unsigned int ok = compress2(buffer,&bufferSize,(Bytef *)src.data(),src.size(),level);

    // Get CRC32
    uint32 crc = 0;

    #ifdef GLT_ZLIB
    crc = ::crc32(src);
    #endif

    if (ok!=Z_OK)
    {
        // If compression fails, just copy the input buffer

        string header;
        if (makeHeader(header,COMPRESS_NONE,src.size(),src.size(),crc))
            dest = header + src;
        else
            ret = false;
    }
    else
    {
        // If compression succeeds, copy the compressed buffer

        if (makeHeader(dest,COMPRESS_ZLIB,bufferSize,src.size(),crc))
        {
            size_t pos = dest.size();
            dest.resize(pos+bufferSize);
            memcpy((void *)(dest.data()+pos),buffer,bufferSize);
        }
        else
            ret = false;
    }

    // Exit

    delete [] buffer;
    return ret;
}

uint32 crc32(const string &data)
{
    return crc32(crc32(0L,Z_NULL,0),(const Bytef *) data.data(),data.size());
}

uint32 crc32(const void *data,const uint32 bytes)
{
    return crc32(crc32(0L,Z_NULL,0),(const Bytef *) data,bytes);
}

#endif


