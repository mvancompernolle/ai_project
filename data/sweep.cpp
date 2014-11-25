#include "sweep.h"

/*! \file
    \ingroup Mesh

    $Id: sweep.cpp,v 1.9 2003/05/10 17:02:03 nigels Exp $

    $Log: sweep.cpp,v $
    Revision 1.9  2003/05/10 17:02:03  nigels
    *** empty log message ***

    Revision 1.8  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include "mesh.h"

#include <math/path.h>

#include <node/shape.h>
#include <node/shapes.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <cmath>
#include <cassert>
#include <vector>
using namespace std;

//
// Mesh version
//

GltShape *
MeshSweep(const GltPath3D &f,const double radius,const int slices,const int stacks,const bool convex)
{
    Vector position, prev_position;
    Vector orientation, prev_orientation;
    Vector accel;
    Vector r;

    vector<Vector> vertex(slices);
    vector<Vector> prev_vertex(slices);

    Mesh *mesh = new Mesh();

    GltShapes *meshes = NULL;

    if (convex)
    {
        meshes = new GltShapes();
        meshes->push_back(mesh);
    }

    int pos,vert;

    for (pos=-1;pos<=stacks;pos++)
    {
        // Get position and orientation

        position    = f.f((double) pos / stacks);
        orientation = f.df((double) pos / stacks);
        accel       = f.ddf((double) pos / stacks);

        orientation.normalize();
        accel.normalize();

        // Find a vector in plane normal to orientation

        if (pos==-1)
        {
            if (orientation.x()>orientation.y() && orientation.x()>orientation.z())
                r = xProduct(orientation,VectorY);
            else
            if (orientation.y()>orientation.z())
                r = xProduct(orientation,VectorZ);
            else
                r = xProduct(orientation,VectorX);

            r.normalize();
        }
        else
        {
            double dot = orientation*r;
            double alpha = acos(dot) - M_PI_2;

            Vector axis  = xProduct(orientation,r);
            axis.normalize();
            r = matrixRotate(axis,alpha*M_DEG_PI)*r;
            r.normalize();
        }

        // Calculate vertices around circle

        if (pos>=0)
            for (vert=0;vert<slices;vert++)
            {
                Matrix rotate = matrixRotate(orientation,(double) vert * (360.0/slices));
                vertex[vert] = (rotate*r)*radius+position;
            }

        // Start face

        if (pos==1 || pos>=1 && convex)
        {
            uint32 vBase = mesh->point().size();

            for (vert=0;vert<slices;vert++)
                mesh->point().push_back(Point(prev_vertex[vert],-prev_orientation));

            uint32 vCenter = mesh->point().size();
            mesh->point().push_back(Point(prev_position,-prev_orientation));

            for (vert=0;vert<slices;vert++)
                mesh->triangle().push_back(
                    Triangle(
                        &mesh->point()[vert+vBase],
                        &mesh->point()[((vert+1)%slices)+vBase],
                        &mesh->point()[vCenter]
                    )
                );
        }

        //
        // Connecting segments
        //

        if (pos>=0 && convex || pos==1 && !convex)
            for (vert=0;vert<slices;vert++)
                mesh->point().push_back(Point(prev_vertex[vert],prev_vertex[vert]-prev_position));

        if (pos>=0)
            for (vert=0;vert<slices;vert++)
                mesh->point().push_back(Point(vertex[vert],vertex[vert]-position));

        if (pos>0)
        {
            uint32 vBase = mesh->point().size()-slices;
            uint32 pvBase = vBase-slices;

            for (vert=0;vert<slices;vert++)
            {
                mesh->triangle().push_back(
                    Triangle(
                        &mesh->point()[vert+vBase],
                        &mesh->point()[((vert+1)%slices)+vBase],
                        &mesh->point()[vert+pvBase]
                    )
                );

                mesh->triangle().push_back(
                    Triangle(
                        &mesh->point()[((vert+1)%slices)+vBase],
                        &mesh->point()[((vert+1)%slices)+pvBase],
                        &mesh->point()[vert+pvBase]
                    )
                );
            }

//          #define SWEEP_POV_OUTPUT
            #ifdef SWEEP_POV_OUTPUT

            Vector core = position - prev_position;
            core.normalise();

            double angle = acos(core*VectorX)/Degr;
            Vector axis  = xProduct(core,VectorX);

            Matrix trans = matrixTranslate(prev_position);
            trans = matrixRotate(axis,angle) * trans;
            Matrix inv = trans.inverse();

            double maxx = (inv * prev_vertex[0]).x();
            double minx = (inv * vertex[0]).x();

            for (uint32 v=0;v<stacks;v++)
            {
                double x = (inv * prev_vertex[v]).x();

                maxx = MIN(x,maxx);

                x = (inv * vertex[v]).x();

                minx = MAX(x,minx);
            }

//          cout << minx << ' ' << maxx << endl;

            cout << "cylinder { " << endl;
            (trans * ( VectorX * minx)).writePOV(cout) << ',';
            (trans * ( VectorX * maxx)).writePOV(cout) << ',';
            cout << radius << '}' << endl;
            #endif

        }

        //
        // End face
        //

        if (pos==stacks || pos>=1 && convex)
        {
            uint32 vBase = mesh->point().size();

            for (vert=0;vert<slices;vert++)
                mesh->point().push_back(Point(vertex[vert],orientation));

            uint32 vCenter = mesh->point().size();
            mesh->point().push_back(Point(position,orientation));

            for (vert=0;vert<slices;vert++)
                mesh->triangle().push_back(
                    Triangle(
                        &mesh->point()[((vert+1)%slices)+vBase],
                        &mesh->point()[vert+vBase],
                        &mesh->point()[vCenter]
                    )
                );
        }

        if (pos>0 && pos<stacks && convex)
        {
            mesh = new Mesh();
            meshes->push_back(mesh);
        }

        prev_position = position;
        prev_orientation = orientation;

        swap(vertex,prev_vertex);       // Not efficent way to swap vectors
    }

    if (convex)
        return meshes;

    return mesh;
}

/*

Mesh *MeshSweep(Func &f,const double radius,const int slices,const int stacks)
{
     meshes = _MeshSweep(f,radius,slices,stacks);
    assert(meshes.size()==1);
    return meshes.front();
}

GltShapes
MeshSweepConvex(const Func &f,const double radius,const int slices,const int stacks)
{
    return _MeshSweep(f,radius,slices,stacks,true);
}


*/
