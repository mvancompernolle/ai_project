#include "md3.h"

/*! \file
    \ingroup Mesh

    See also:

    - http://www.planetquake.com/polycount/cottages/wrath/tutorials/Quake3/Quake3-format.html
    - http://www.sulaco.co.za/opengl3.htm
    - http://www.4thdimented.com/dimented_tutorialQuake3_1.htm

    $Id: md3.cpp,v 1.10 2003/07/29 08:21:54 nigels Exp $

    $Log: md3.cpp,v $
    Revision 1.10  2003/07/29 08:21:54  nigels
    *** empty log message ***

    Revision 1.6  2003/06/02 03:15:56  nigels
    MD3 Tweaks

    Revision 1.4  2003/05/10 17:01:15  nigels
    Tidy and document

    Revision 1.3  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include <math/real.h>
#include <math/point.h>
#include <mesh/triangle.h>
#include <misc/endian.h>
#include <misc/string.h>

#include <glt/error.h>
#include <glt/rgb.h>

#include <iostream>
using namespace std;

const uint32 MD3_MAGIC        = 0x33504449; ///< Magic number for MD3 files

const uint32 MD3_MAX_QPATH     = 64;         ///< Maximum Quake3 Path Length
const uint32 MD3_MAX_FRAMES    = 1024;       ///< Maximum Quake3 Key Frames
const uint32 MD3_MAX_TAGS      = 16;         ///< Maximum Quake3 Tags
const uint32 MD3_MAX_SURFACES  = 32;         ///< Maximum Quake3 Surfaces
const uint32 MD3_MAX_SHADERS   = 256;        ///< Maximum Quake3 Shaders
const uint32 MD3_MAX_VERTS     = 4096;       ///< Maximum Quake3 Vertecies
const uint32 MD3_MAX_TRIANGLES = 8192;       ///< Maximum Quake3 Triangles

const float MD3_XYZ_SCALE     = (1.0/64.0); ///< Scale factor for vertecies

/*! \struct  MD3header
    \brief   Quake3 MD3 File Header
    \ingroup Mesh
*/

typedef struct
{
    uint32  magic;                   ///< Should be MD3_MAGIC
    int32   version;                 ///< Quake3 is Version 15
    char    filename[MD3_MAX_QPATH]; ///< Null-terminated path
    uint32  flags;                   ///< Unknown
    uint32  numFrames;               ///< Number of Frame objects   (MD3_MAX_FRAMES   maximum)
    uint32  numTags;                 ///< Number of Tag objects     (MD3_MAX_TAGS     maximum)
    uint32  numSurfaces;             ///< Number of Surface objects (MD3_MAX_SURFACES maximum)
    uint32  numSkins;                ///< Number of Skin objects    (Not used)
    uint32  ofsFrames;               ///< Offset to beginning of Frames
    uint32  ofsTags;                 ///< Offset to beginning of Tags
    uint32  ofsSurfaces;             ///< Offset to beginning of Surfaces
    uint32  ofsEOF;                  ///< Offset to end of MD3 data
} MD3header;

/*! \struct  MD3vector3
    \brief   Quake3 MD3 3d vector
    \ingroup Mesh
*/

typedef struct
{
    float x;
    float y;
    float z;
} MD3vector3;

/*! \struct  MD3frame
    \brief   Quake3 MD3 frame
    \ingroup Mesh
*/

typedef struct
{
    MD3vector3 min;                 ///< Bottom left corner of bounding box
    MD3vector3 max;                 ///< Top right corner of bounding box
    MD3vector3 origin;              ///< Local origin (usually <0,0,0>)
    float      radius;              ///< Radius of bounding sphere
    char       name[16];            ///< Null-terminated name of frame
} MD3frame;

/*! \struct  MD3tag
    \brief   Quake3 MD3 tag
    \ingroup Mesh
*/

typedef struct
{
    char       name[MD3_MAX_QPATH]; ///< Null-terminated name of tag
    MD3vector3 origin;              ///< Position
    MD3vector3 orientation[3];      ///< Orientation
} MD3tag;

/*! \struct  MD3surface
    \brief   Quake3 MD3 surface
    \ingroup Mesh
*/

typedef struct
{
    uint32 magic;                   ///< Should be MD3_MAGIC
    char   name[MD3_MAX_QPATH];     ///< Null-terminated name
    int32  flags;                   ///< Unknown
    uint32 numFrames;               ///< Number of Frame objects    (MD3_MAX_FRAMES    maximum)
    uint32 numShaders;              ///< Number of Shader objects   (MD3_MAX_SHADERS   maximum)
    uint32 numVerts;                ///< Number of Vertex objects   (MD3_MAX_VERTS     maximum)
    uint32 numTriangles;            ///< Number of Triangle objects (MD3_MAX_TRIANGLES maximum)
    uint32 ofsTriangles;            ///< Offset to beginning of Triangles
    uint32 ofsShaders;              ///< Offset to beginning of Shaders
    uint32 ofsST;                   ///< Offset to beginning of texture co-ordinates
    uint32 ofsXYZNormal;            ///< Offset to beginning of vertecies
    uint32 ofsEnd;                  ///< Offset to the end of the surface
} MD3surface;

/*! \struct  MD3shader
    \brief   Quake3 MD3 shader
    \ingroup Mesh
*/

typedef struct
{
    char       name[MD3_MAX_QPATH]; ///< Null-terminated name of tag
    uint32     index;               ///< Shader index
} MD3shader;

/*! \struct  MD3triangle
    \brief   Quake3 MD3 triangle
    \ingroup Mesh
*/

typedef struct
{
    uint32     index[3];            ///< Vertex indecies
} MD3triangle;

/*! \struct  MD3texCoord
    \brief   Quake3 MD3 texture co-ordinate pair
    \ingroup Mesh
*/

typedef struct
{
    float      s;                   ///< S Vertex coordinate
    float      t;                   ///< T Vertex coordinate
} MD3texCoord;

/*! \struct  MD3vertex
    \brief   Quake3 MD3 vertex
    \ingroup Mesh
*/

typedef struct
{
    int16       x;                  ///< x position
    int16       y;                  ///< y position
    int16       z;                  ///< z position
    int16       normal;             ///< normal
} MD3vertex;

///////////////////////////////////////////////////////////

template <class T,class S>
T ptr(S begin,const byte *end,const uint32 offset)
{
    const byte *i = reinterpret_cast<const byte *>(begin);
    if (i+offset+sizeof(T)>=end)
        return NULL;
    return reinterpret_cast<const T>(i + offset);
}

///////////////////////////////////////////////////////////

MD3Surface::MD3Surface()
: _current(0)
{
//  _texture = new GltTexture();
//  _texture->init(std::string("crash.tga"));
}

MD3Surface::~MD3Surface()
{
}

void
MD3Surface::draw() const
{
    GLERROR;

    if (!visible())
        return;

    glPushMatrix();

        glColor();
        transformation().glMultMatrix();
        if (_texture.get())
            _texture->set();

        if (_current<_frame.size())
            _frame[_current].draw();

    glPopMatrix();

    GLERROR;
}

void
MD3Surface::boundingBox(BoundingBox &box) const
{
    for (uint32 i=0; i<_frame.size(); i++)
    {
            BoundingBox tmp;
            _frame[i].boundingBox(tmp);
            box += tmp;
    }
}

//////////

MD3Object::MD3Object()
{
}

MD3Object::~MD3Object()
{
}

void
MD3Object::draw() const
{
    GLERROR;

    if (!visible())
        return;

    glPushMatrix();

        glColor();
        transformation().glMultMatrix();

        uint32 i;

        for (i=0; i<_surface.size(); i++)
            _surface[i].draw();

        for (i=0; i<_link.size(); i++)
            if (_link[i].get())
                _link[i]->draw();

    glPopMatrix();

    GLERROR;
}

void
MD3Object::boundingBox(BoundingBox &box) const
{
    uint32 i;
    BoundingBox tmp;

    for (i=0; i<_surface.size(); i++)
    {
        _surface[i].boundingBox(tmp);
        box += tmp;
    }

    for (i=0; i<_link.size(); i++)
        if (_link[i].get())
        {
            _link[i]->boundingBox(tmp);
            box += tmp;
        }
}


bool
MD3Object::link(GltShapePtr object,const std::string &tag)
{
    for (uint32 i=0; i<_tag.size(); i++)
        if (tag==_tag[i])
        {
            _link[i] = object;
            object->transformation() = _tagFrame[i];
            return true;
        }

    return false;
}

bool
MD3Object::loadModel(const std::string &filename)
{
    string md3;
    readFile(filename,md3);
    return loadModel(md3.c_str(),md3.size());
}

bool
MD3Object::loadSkin(const std::string &filename)
{
    string a,b;
    readFile(filename,a);
    dos2unix(b,a);
    return loadSkin(b.c_str(),b.size());
}

bool
MD3Object::loadSkin(const void *buffer,const uint32 size)
{
    const char *begin = static_cast<const char *>(buffer);
    const char *end   = begin + size;

    const char *i = begin;

    while (i<end)
    {
        // Find end of line

        const char *j = i;
        while (*j!='\n' && j<end)
            j++;

        // Find comma

        const char *k = i;
        while (*k!=',' && k<j)
            k++;

        if (i<k && k<j)
        {
            string surface(i,k-i);
            string filename(k+1,j-k-1);

            #ifndef NDEBUG
            cout << surface << ',' << filename << endl;
            #endif

            GltTexturePtr texture = new GltTexture();

            if (!texture->init(filename))
            {
                #ifndef NDEBUG
                cout << "Failed to load texture: " << filename << endl;
                #endif
            }

            uint32 m;
            for (m=0; m<_surface.size(); m++)
                if (_surface[m].name()==surface)
                    _surface[m]._texture = texture;
        }

        i = j+1;
    }

    return true;
}

bool
MD3Object::loadModel(const void *buffer,const uint32 size)
{
    const byte *begin = static_cast<const byte *>(buffer);
    const byte *end   = begin + size;

    // If the buffer is too small to contain
    // an MD3, bail out early

    if (size<sizeof(MD3header))
        return false;

    // Access the MD3 header

    const MD3header *header
        = ptr<const MD3header *>(begin,end,0);

    if (!header)
        return false;

    if (littleEndian(&header->magic)!=MD3_MAGIC)
        return false;

    if (header->ofsEOF!=size)
        return false;

    if (header->version!=15)
    {
        gltWarning("Unexpected MD3 version.");
        return false;
    }

    // Examine the tags

    // Base case
    if (_tag.size()==0)
    {
        _tag.resize(header->numTags);
        _tagFrame.resize(header->numTags);
        _link.resize(header->numTags);

        const MD3tag *t
            = ptr<const MD3tag *>(begin,end,header->ofsTags);

        for (uint32 i=0; i<header->numTags; i++)
        {
            _tag[i] = t->name;
            Matrix &matrix = _tagFrame[i];

            matrix[ 0] = t->orientation[0].x;
            matrix[ 1] = t->orientation[0].y;
            matrix[ 2] = t->orientation[0].z;
            matrix[ 4] = t->orientation[1].x;
            matrix[ 5] = t->orientation[1].y;
            matrix[ 6] = t->orientation[1].z;
            matrix[ 8] = t->orientation[2].x;
            matrix[ 9] = t->orientation[2].y;
            matrix[10] = t->orientation[2].z;
            matrix[12] = t->origin.x;
            matrix[13] = t->origin.y;
            matrix[14] = t->origin.z;

//          cout << _tag[i] << endl;
//          cout << matrix  << endl;

            t++;
        }
    }

    // Access each surface

    const MD3surface *surface
        = ptr<const MD3surface *>(begin,end,header->ofsSurfaces);

    if (!surface)
        return false;

    _surface.resize(header->numSurfaces);

    for (uint32 i=0; surface && i<header->numSurfaces; i++)
    {
        if (littleEndian(&surface->magic)!=MD3_MAGIC)
            return false;

        _surface[i].color() = white;
        _surface[i].name()  = surface->name;

        _surface[i]._frame.resize(surface->numFrames);

        {
            for (uint32 j=0; j<surface->numFrames; j++)
                _surface[i]._frame[j].inheritColor() = true;
        }

        // Convert vertecies

        const MD3vertex *vertex
            = ptr<const MD3vertex *>(surface,end,surface->ofsXYZNormal);

        if (!vertex)
            return false;

        {
            for (uint32 j=0; j<surface->numFrames; j++)
            {

                const MD3texCoord *texCoord
                    = ptr<const MD3texCoord *>(surface,end,surface->ofsST);


                for (uint32 k=0; k<surface->numVerts; k++)
                {
                    // Convert position
                    const Vector pos(vertex->x*MD3_XYZ_SCALE,vertex->y*MD3_XYZ_SCALE,vertex->z*MD3_XYZ_SCALE);

                    // Convert normal
                    const double lat =  (vertex->normal    &255)*M_2PI/255.0;
                    const double lng = ((vertex->normal>>8)&255)*M_2PI/255.0;
                    const Vector normal(cos(lat)*sin(lng),sin(lat)*sin(lng),cos(lng));

                    // Convert texture

                    const Vector texture(texCoord->s,1.0-texCoord->t,0.0);

                    // Add point
                    _surface[i]._frame[j].point().push_back(Point(pos,normal,texture));

                    vertex++;
                    texCoord++;
                }
            }
        }

        // Convert triangles

        const MD3triangle *tri
            = ptr<const MD3triangle *>(surface,end,surface->ofsTriangles);

        for (uint32 j=0; j<surface->numTriangles; j++)
        {
            if (tri->index[0]>surface->numVerts)
                return false;

            if (tri->index[1]>surface->numVerts)
                return false;

            if (tri->index[2]>surface->numVerts)
                return false;

            for (uint32 k=0; k<surface->numFrames; k++)
            {
                Point *a = &_surface[i]._frame[k].point()[tri->index[0]];
                Point *b = &_surface[i]._frame[k].point()[tri->index[1]];
                Point *c = &_surface[i]._frame[k].point()[tri->index[2]];
                _surface[i]._frame[k].triangle().push_back(Triangle(a,c,b));        // Swap orientation
            }

            tri++;
        }

        // Advance to next surface

        surface = ptr<const MD3surface *>(surface,end,surface->ofsEnd);
    }

    return true;
}

////////////////////////////////////////////////////////////

GltShapes
readMD3(const string &basename)
{
    GltShapes shapes;

    MD3Object *lower = new MD3Object();
    MD3Object *upper = new MD3Object();
    MD3Object *head  = new MD3Object();

    lower->loadModel(basename + "\\lower.md3");
    upper->loadModel(basename + "\\upper.md3");
    head ->loadModel(basename + "\\head.md3" );

    lower->loadSkin(basename + "\\lower_default.skin");
    upper->loadSkin(basename + "\\upper_default.skin");
    head ->loadSkin(basename + "\\head_default.skin" );

    lower->link(upper,"tag_torso");
    upper->link(head,"tag_head");

    shapes.push_back(lower);

    return shapes;
}
