//#include "read3dsb.h"
#include "vrmlpars.h"

#include "mesh.h"

/*! \file
    \ingroup Mesh

    $Id: mesh.cpp,v 1.22 2003/05/31 13:31:15 nigels Exp $

    $Log: mesh.cpp,v $
    Revision 1.22  2003/05/31 13:31:15  nigels
    3DS now loaded as GltShapes rather than via Mesh

    Revision 1.21  2003/05/31 11:37:48  nigels
    Some changes to 3DS support

    Revision 1.20  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include <glt/gl.h>

#include <math/vector3.h>
#include <math/matrix4.h>

#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cassert>
using namespace std;

void
parseVrml(IndexedFaceSetPtr &faceSet)
{
    if (!faceSet.get())
        return;

    if
    (
        faceSet->coord.size() &&
        faceSet->coordIndex.size()
    )
    {
        uint32 pointBase = yyVrmlMesh->point().size();
        uint32 pointMax = faceSet->coord.size();

        //
        // Build points in mesh
        //

        for (uint32 p=0; p<pointMax; p++)
        {
            const Vector &pos  = faceSet->coord[p];
            const Vector &norm = p<faceSet->normal.size() ? faceSet->normal[p] : Vector0;
            yyVrmlMesh->point().push_back(Point(pos,norm));
        }

        //
        // Build triangles in mesh
        //

        for (uint32 t=0;t< faceSet->coordIndex.size();t+=4)
        {
            uint32 i[3];

            i[0] = faceSet->coordIndex[t];
            i[1] = faceSet->coordIndex[t+1];
            i[2] = faceSet->coordIndex[t+2];

            for (uint32 p=0;p<4;p++)
                if (i[0]<0 || i[0]>=pointMax)
                    goto error;

            if (faceSet->coordIndex[t+3]!=-1)
                goto error;

            yyVrmlMesh->triangle().push_back
            (
                Triangle(
                    &yyVrmlMesh->point()[pointBase+i[0]],
                    &yyVrmlMesh->point()[pointBase+i[faceSet->ccw ? 1 : 2]],
                    &yyVrmlMesh->point()[pointBase+i[faceSet->ccw ? 2 : 1]]
                )
            );

            continue;
error:
            cout << "Error in face data." << endl;
            break;
        }

        if (!faceSet->solid)
            yyVrmlMesh->cullFace() = false;
    }

}

//////////////////////////////////////////////////////////////////////////////////////

Mesh::Mesh()
: _cullFace(false)
{
}

Mesh::Mesh(const Mesh &mesh)
: _cullFace(true)
{
    Mesh::operator=(mesh);
}

Mesh::Mesh(const std::string &filename,MeshFileType type)
: _cullFace(true)
{
    if (type==MESH_AUTO)
    {
        string::size_type n = filename.find('.');

        if (n==string::npos)
            return;

        string ext = filename.substr(n+1);

             if (ext=="wrl" || ext=="WRL") type = MESH_VRML;
        else if (ext=="off" || ext=="OFF") type = MESH_OFF;
    }

    switch (type)
    {
    case MESH_VRML:
        {
            yyVrmlMesh = this;

            VrmlParseIndexedFaceSet = parseVrml;

            Vrmlin = fopen(filename.c_str(),"rt");
            if (Vrmlin)
            {
                Vrmlparse();
                fclose(Vrmlin);
            }

            Vrmlin = NULL;
            yyVrmlMesh = NULL;
            VrmlParseIndexedFaceSet = NULL;
        }
        break;

    case MESH_OFF:
        {
            fstream is(filename.c_str(),ios::in);
            if (!is.good())
                return;
            readOff(is);
        }
        break;

    case MESH_AUTO:
        assert(0);
        break;
    }
}

Mesh::~Mesh()
{
}

Mesh &
Mesh::operator=(const Mesh &mesh)
{
    GltShape::operator=(mesh);

    _point.clear();
    _triangle.clear();
    _quad.clear();

    {
        std::deque<Point>::const_iterator i;
        for (i=mesh.point().begin(); i!=mesh.point().end(); i++)
            _point.push_back(*i);
    }

    {
        std::deque<Triangle>::const_iterator i;
        for (i=mesh.triangle().begin(); i!=mesh.triangle().end(); i++)
        {
            int    pos[3];
            Point *pnt[3];

            int j;
            for (j=0; j<3; j++)
            {
                pos[j] = mesh.index((*i)[j]);
                assert(pos[j]>=0);

                pnt[j] = &_point[pos[j]];
            }

            _triangle.push_back(Triangle(pnt[0],pnt[1],pnt[2]));
        }
    }

    {
        std::deque<Quad>::const_iterator i;
        for (i=mesh.quad().begin(); i!=mesh.quad().end(); i++)
        {
            int    pos[4];
            Point *pnt[4];

            int j;
            for (j=0; j<4; j++)
            {
                pos[j] = mesh.index((*i)[j]);
                assert(pos[j]>=0);

                pnt[j] = &_point[pos[j]];
            }

            _quad.push_back(Quad(pnt[0],pnt[1],pnt[2],pnt[3]));
        }
    }

    return *this;
}

void
Mesh::draw() const
{
    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
    glPushMatrix();
        transformation().glMultMatrix();

        glColor();

        if (!inheritSolid())
            glPolygonMode(GL_FRONT_AND_BACK,solid() ? GL_FILL : GL_LINE);

        if (_cullFace)
            glEnable(GL_CULL_FACE);

        glBegin(GL_TRIANGLES);
        {
            for (deque<Triangle>::const_iterator i=_triangle.begin(); i!=_triangle.end(); i++)
                (*i).glDraw();
        }
        glEnd();

        glBegin(GL_QUADS);
        {
            for (deque<Quad>::const_iterator i=_quad.begin(); i!=_quad.end(); i++)
                (*i).glDraw();
        }
        glEnd();

    glPopMatrix();
    glPopAttrib();
}

void
Mesh::boundingBox(BoundingBox &box) const
{
    vector<Vector> p;
    p.reserve(_point.size());
    for (uint32 i=0; i<_point.size(); i++)
        p.push_back(_point[i]);
    transformPoints(p);

    box.reset();
    box += p;
}

std::string
Mesh::description() const
{
    return "Mesh";
}

      deque<Point> &Mesh::point()       { return _point; }
const deque<Point> &Mesh::point() const { return _point; }

      deque<Triangle> &Mesh::triangle()       { return _triangle; }
const deque<Triangle> &Mesh::triangle() const { return _triangle; }

      deque<Quad> &Mesh::quad()       { return _quad; }
const deque<Quad> &Mesh::quad() const { return _quad; }

      bool &Mesh::cullFace()       { return _cullFace; }
const bool &Mesh::cullFace() const { return _cullFace; }

bool
Mesh::inside(const Vector &pos) const
{
    // Check plane equations for all triangles

    {
        for (deque<Triangle>::const_iterator i = _triangle.begin(); i!=_triangle.end(); i++)
            if ((*i).plane().inside(pos))
                return false;
    }

    // Check plane equations for all quads

    {
        for (deque<Quad>::const_iterator i = _quad.begin(); i!=_quad.end(); i++)
            if ((*i).plane().inside(pos))
                return false;
    }

    return true;
}

int
Mesh::index(const Point *point) const
{
    int i=0;

    std::deque<Point>::const_iterator j;
    for (j=_point.begin(); j!=_point.end(); i++,j++)
        if (point==&(*j))
            return i;

    return -1;
}

//////////////////////////////////////////////////////////////////////////


ostream &
Mesh::writeDebugInfo(ostream &os) const
{
    os << "Mesh::writeDebugInfo" << endl;
    os << "  " << _point.size()    << " points." << endl;
    os << "  " << _triangle.size() << " triangles." << endl;
    os << "  " << _quad.size()     << " quads." << endl;

    if (_point.size()>0)
    {
        Vector min_ = _point[0];
        Vector max_ = _point[0];

        for (uint32 p=1; p<_point.size(); p++)
        {
            min_.x() = MIN(min_.x(),_point[p].x());
            min_.y() = MIN(min_.y(),_point[p].y());
            min_.z() = MIN(min_.z(),_point[p].z());

            max_.x() = MAX(max_.x(),_point[p].x());
            max_.y() = MAX(max_.y(),_point[p].y());
            max_.z() = MAX(max_.z(),_point[p].z());
        }

        os << "  min    = " << min_ << endl;
        os << "  max    = " << max_ << endl;
        os << "  size   = " << max_-min_ << endl;
        os << "  center = " << (min_+max_)*0.5 << endl;
    }

    os << endl;

    return os;
}

bool
Mesh::povrayExport(std::ostream &os) const
{
    os << "mesh {" << endl;

        for (uint32 t=0; t<_triangle.size(); t++)
            _triangle[t].writePOV(os);

    os << "pigment { color rgb ";
    color().writePov(os) << '}' << endl;

    transformation().writePov(os) << endl;

    os << "}" << endl;

    return true;
}

ostream &
Mesh::writeAOff(ostream &os) const
{
    map<Point const *,int,greater<void const *> > mapPoint;

    os << _point.size() << ' ' << _triangle.size() << ' ' << 0 << endl;

    for (uint32 v=0; v<_point.size(); v++)
    {
        mapPoint[&_point[v]] = v;

        os << _point[v][0] << ' ';
        os << _point[v][1] << ' ';
        os << _point[v][2] << endl;
    }

    for (uint32 t=0;t<_triangle.size(); t++)
    {
        os << 3 << '\t';
        os << mapPoint[_triangle[t][0]] + 1 << '\t';
        os << mapPoint[_triangle[t][1]] + 1 << '\t';
        os << mapPoint[_triangle[t][2]] + 1 << endl;
    }

    return os;
}

void
Mesh::readOff(istream &is)
{
    int dimensions;
    int vertex,faces,cells;

    is >> dimensions;

    if (dimensions!=2 && dimensions!=3)
        return;

    is >> vertex;
    is >> faces;
    is >> cells;

    for (int v=0; v<vertex; v++)
    {
        double x,y,z;

        is >> x;
        is >> y;

        if (dimensions==3)
            is >> z;
        else
            z = 0.0;

        _point.push_back( Point(Vector(x,y,z), Vector0) );
    }

    for (int f=0; f<faces; f++)
    {
        int number;

        is >> number;

        switch (dimensions)
        {
        case 2:
            {
            }
        case 3:
            {
                switch (number)
                {
                case 3:                 // Triangle
                    {
                        int a,b,c;

                        is >> a;
                        is >> b;
                        is >> c;

                        _triangle.push_back(Triangle(&_point[a],&_point[b],&_point[c]));
                    }
                    break;
                case 4:                     // Prism
                    {
                        int a,b,c,d;

                        is >> a;
                        is >> b;
                        is >> c;
                        is >> d;

                        _triangle.push_back(Triangle(&_point[a],&_point[b],&_point[c]));
                        _triangle.push_back(Triangle(&_point[a],&_point[b],&_point[d]));
                        _triangle.push_back(Triangle(&_point[a],&_point[c],&_point[d]));
                        _triangle.push_back(Triangle(&_point[b],&_point[c],&_point[d]));
                    }
                    break;
                }
            }
            break;
        }
    }
}

