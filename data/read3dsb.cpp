#include "read3dsb.h"

/*! \file
    \ingroup Mesh

    \todo Migrate to 3ds.cpp/.h
    \todo Smooth lighting support
    \todo Texture support

    $Id: read3dsb.cpp,v 1.13 2003/08/21 04:29:06 nigels Exp $

    $Log: read3dsb.cpp,v $
    Revision 1.13  2003/08/21 04:29:06  nigels
    *** empty log message ***

    Revision 1.12  2003/06/02 03:15:56  nigels
    MD3 Tweaks

    Revision 1.11  2003/05/31 13:27:58  nigels
    *** empty log message ***

    Revision 1.10  2003/05/31 13:14:44  nigels
    Tested with several 3DS files from the net

    Revision 1.9  2003/05/31 12:53:41  nigels
    Cleanup of Binary 3DS Reader

    Revision 1.8  2003/05/31 11:38:02  nigels
    Some changes to 3DS support

    Revision 1.6  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#ifdef MSCVER
#pragma warning(disable : 4786)     // Lengthy STL symbols
#endif

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

using namespace std;

#include <mesh/mesh.h>

typedef uint16 chunkID;
typedef uint32 chunkSize;

bool readPrimary (const byte *i, const byte *end, GltShapes &shapes);
bool readEditor  (const byte *i, const byte *end, GltShapes &shapes);
bool readObject  (const byte *i, const byte *end, GltShapes &shapes);
bool readMesh    (const byte *i, const byte *end, Mesh &mesh);
bool readMaterial(const byte *i, const byte *end);

bool readChunk (const byte **i, const byte *end, chunkID &id, chunkSize &size);
bool readString(const byte **i, const byte *end, string &string);

bool
readChunk(const byte **i, const byte *end, chunkID &id, chunkSize &size)
{
    if (end-(*i)<6)
        return false;

    id   = * reinterpret_cast<const chunkID *>(*i);
    size = * reinterpret_cast<const chunkSize *>(*i+2);

    #ifndef NDEBUG
    cout << "Chunk 0x" << hex << id << " (" << dec << size << " bytes.)" << endl;
    #endif

    *i   += 6;
    size -= 6;

    if (*i+size>end)
        return false;

    return true;
}

bool
readString(const byte **i, const byte *end, string &str)
{
    str = string();

    while (*i<end && **i!='\0')
    {
        str += **i;
        (*i)++;
    }

    if (*i<end)
        (*i)++;

    return true;
}

bool
readMaterial(const byte *i, const byte *end)
{
    string name;
    if (!readString(&i,end,name))
        return false;

    if (i+2>end)
        return false;

    const uint16 &n = * reinterpret_cast<const uint16 *>(i);
    i += 2;

    #ifndef NDEBUG
    cout << "Material " << name << ", " << n << " faces." << endl;
    #endif

    return true;
}

bool
readMesh(const byte *i, const byte *end, Mesh &mesh)
{
    chunkID   id   = 0;
    chunkSize size = 0;

    while (i<end && readChunk(&i,end,id,size))
    {
        switch (id)
        {
            case 0x4110:            // Vertex List
                {
                    const uint16 &n = * reinterpret_cast<const uint16 *>(i);
                    i += 2;

                    #ifndef NDEBUG
                    cout << "vertex list: " << n << " points." << endl;
                    #endif

                    mesh.point().resize(n);

                    for (uint16 j=0; j<n && i<end; j++, i+=12)
                        mesh.point()[j] =
                            Point
                            (
                                Vector
                                (
                                    * reinterpret_cast<const float *>(i),
                                    * reinterpret_cast<const float *>(i+4),
                                    * reinterpret_cast<const float *>(i+8)
                                ),
                                Vector0
                            );

                    break;
                }

            case 0x4120:            // Faces
                {
                    const uint16 &n = * reinterpret_cast<const uint16 *>(i);
                    i += 2;

                    #ifndef NDEBUG
                    cout << "face list: " << n << " faces." << endl;
                    #endif

                    mesh.triangle().resize(n);

                    const uint16 maxIndex = mesh.point().size();

                    for (uint16 j=0; j<n && i<end; j++, i+=8)
                    {
                        const uint16 &a = * reinterpret_cast<const uint16 *>(i);
                        const uint16 &b = * reinterpret_cast<const uint16 *>(i+2);
                        const uint16 &c = * reinterpret_cast<const uint16 *>(i+4);

                        if (a<maxIndex && b<maxIndex && c<maxIndex)
                            mesh.triangle()[j] =
                                Triangle
                                (
                                    &mesh.point()[a],
                                    &mesh.point()[b],
                                    &mesh.point()[c]
                                );
                    }

                    break;
                }

            case 0x4130:        // Material
                readMaterial(i,end);
                break;

            case 0x4140:        // Texture co-ordinates
                {
                    const uint16 &n = * reinterpret_cast<const uint16 *>(i);
                    i += 2;

                    #ifndef NDEBUG
                    cout << "Texture co-ordinates: " << n << endl;
                    #endif

                    // In pairs of x,y

                    i += n*2*4;
                    break;
                }

            case 0x4111:        // Point flag array, TODO
            case 0x4160:        // Translation Matrix, TODO
                i += size;
                break;

            default:
                {
                    #ifndef NDEBUG
                    cout << "Unrecognised chunk: 0x" << hex << id << " (" << dec << size << " bytes)" << endl;
                    #endif
                    i += size;
                    break;
                }
        }
    }

    return true;
}

bool
readObject(const byte *i, const byte *end, GltShapes &shapes)
{
    chunkID   id   = 0;
    chunkSize size = 0;

    string name;
    if (!readString(&i,end,name))
        return false;

    #ifndef NDEBUG
    cout << "Object: " << name << endl;
    #endif

    while (i<end && readChunk(&i,end,id,size))
    {
        switch (id)
        {
            case 0x4100:        // Triangular Polygon Object
                {
                    Mesh *mesh = new Mesh();
                    mesh->inheritColor() = true;
                    if (readMesh(i,end,*mesh))
                        shapes.push_back(mesh);
                    break;
                }

            case 0x4600:            // Direct light data, not yet implemented
            case 0x4700:            // Camera data, not yet implemented
                break;

            default:
                {
                    #ifndef NDEBUG
                    cout << "Unrecognised chunk in readObject: 0x" << hex << id << " (" << dec << size << " bytes)" << endl;
                    #endif
                    break;
                }
        }

        i += size;
    }

    return true;
}

bool
readEditor(const byte *i, const byte *end, GltShapes &shapes)
{
    chunkID   id   = 0;
    chunkSize size = 0;

    while (i<end && readChunk(&i,end,id,size))
    {
        switch (id)
        {
            case 0x4000:
                readObject(i,i+size,shapes);
                break;

            case 0x0100:            // Master scale, not yet implemented
            case 0x3d3e:            // Mesh Version, not yet implemented
            case 0x4700:            // Camera data, not yet implemented
            case 0xafff:            // Material entry, not yet implemented
                break;

            default:
                {
                    #ifndef NDEBUG
                     cout << "Unrecognised chunk in readEditor: 0x" << hex << id << " (" << dec << size << " bytes)" << endl;
                    #endif
                    break;
                }
        }

        i += size;
    }

    return true;
}

bool
readPrimary(const byte *i, const byte *end, GltShapes &shapes)
{
    chunkID   id   = 0;
    chunkSize size = 0;

    while (i<end && readChunk(&i,end,id,size))
    {
        switch (id)
        {
            case 0x3d3d:
                readEditor(i,i+size,shapes);
                break;

            case 0x0002:            // 3D Mesh version, not yet implemented
            case 0xb000:            // Keyframe data, not yet implemented
                break;

            default:
                {
                    #ifndef NDEBUG
                    cout << "Unrecognised chunk in readPrimary: 0x" << hex << id << " (" << dec << size << " bytes)" << endl;
                    #endif
                    break;
                }
        }

        i += size;
    }

    return true;
}

GltShapes
read3DS(const byte *buffer,const uint32 bufferSize)
{
    const byte *i   = buffer;
    const byte *end = i+bufferSize;

    GltShapes shapes;
    shapes.inheritColor() = true;

    chunkID   id   = 0;
    chunkSize size = 0;

    while (readChunk(&i,end,id,size))
    {
        // Process Primary 3DS chunk

        switch (id)
        {
            case 0x4d4d:
            case 0xc23d:
                readPrimary(i,i+size,shapes);
                break;

            default:
                {
                    #ifndef NDEBUG
                    cout << "Unrecognised chunk in read3DS: 0x" << hex << id << " (" << dec << size << " bytes)" << endl;
                    #endif
                    break;
                }
        }

        i += size;
    }

    return shapes;
}
