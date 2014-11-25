#include "mcubes.h"

/*! \file
    \ingroup GLT
    \author Steve Anger (70714.3113@compuserve.com)
    \todo Cleanup
    \todo Normals

    \ingroup GLT

    $Id: mcubes.cpp,v 1.7 2002/11/27 00:57:28 nigels Exp $

    $Log: mcubes.cpp,v $
    Revision 1.7  2002/11/27 00:57:28  nigels
    expand

    Revision 1.6  2002/11/07 15:40:45  nigels
    *** empty log message ***

    Revision 1.5  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <glt/gl.h>

#include <cstdlib>
#include <cmath>   // Only needed for plug-in functions
#include <iosfwd>
using namespace std;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
  Vector3 p1, p2;
} Pair;

typedef struct {
    float  value;
    Vector3 loc;
} Sample;

static int x_steps = 20;
static int y_steps = 20;
static int z_steps = 20;

static GltFunc3d func3d = NULL;

static Sample **level0 = NULL;
static Sample **level1 = NULL;

//static FILE *f = NULL;

static void write_polygon (Vector3 *poly, int size);
static int  allocate_cubes (void);
static void free_cubes (void);
static void sample_level (Sample **level, int ix, Vector3 *vmin, Vector3 *vmax);
static void tesselate_cube (int iy, int iz);
static void analyze_face (Sample *cube, int a, int b, int c, int d,
            Pair *line, int *numline);
static void interpolate (Vector3 *v, Sample *a, Sample *b);
static void vect_copy (Vector3 *v1, Vector3 *v2);
static int  vect_equal (Vector3 *v1, Vector3 *v2);
static void vect_add (Vector3 *v1, Vector3 *v2, Vector3 *v3);
static void vect_sub (Vector3 *v1, Vector3 *v2, Vector3 *v3);
static void vect_scale (Vector3 *v1, Vector3 *v2, float k);


int GltMarchingCubes(GltFunc3d func,
            float minx, float miny, float minz,
            float maxx, float maxy, float maxz,
            int xsteps, int ysteps, int zsteps)
{
    int ix, iy, iz;
    Sample **temp;
    Vector3 vmin;
    Vector3 vmax;

    func3d = func;

    vmin.x = minx;
    vmin.y = miny;
    vmin.z = minz;

    vmax.x = maxx;
    vmax.y = maxy;
    vmax.z = maxz;

    x_steps = xsteps;
    y_steps = ysteps;
    z_steps = zsteps;

    if (!allocate_cubes())
        return 0;

    sample_level (level0, 0, &vmin, &vmax);

    for (ix = 1; ix <= x_steps; ix++) {
        sample_level (level1, ix, &vmin, &vmax);

        for (iy = 0; iy < y_steps; iy++)
            for (iz = 0; iz < z_steps; iz++)
               tesselate_cube (iy, iz);

        temp = level0;
        level0 = level1;
        level1 = temp;
    }

    free_cubes();

    return 1;
}

/* Allocate memory for the marching cubes algorithm */
static int allocate_cubes()
{
    int i;

    level0 = (Sample **)malloc ((y_steps+1) * sizeof(Sample *));
    if (level0 == NULL)
        return 0;

    for (i = 0; i < y_steps+1; i++)
        level0[i] = NULL;

    for (i = 0; i < y_steps+1; i++) {
        level0[i] = (Sample *)malloc ((z_steps+1) * sizeof(Sample));
        if (level0[i] == NULL) {
            free_cubes();
            return 0;
        }
    }

    level1 = (Sample **)malloc ((y_steps+1) * sizeof(Sample *));
    if (level1 == NULL)
        return 0;

    for (i = 0; i < y_steps+1; i++)
        level1[i] = NULL;

    for (i = 0; i < y_steps+1; i++) {
        level1[i] = (Sample *)malloc ((z_steps+1) * sizeof(Sample));
        if (level1[i] == NULL) {
            free_cubes();
            return 0;
        }
    }

    return 1;
}


static void free_cubes()
{
    int i;

    if (level0 != NULL) {
        for (i = 0; i < y_steps+1; i++) {
            if (level0[i] != NULL)
                free (level0[i]);
        }

        free (level0);
    }

    if (level1 != NULL) {
        for (i = 0; i < y_steps+1; i++) {
            if (level1[i] != NULL)
                free (level1[i]);
        }

        free (level1);
    }

    level0 = NULL;
    level1 = NULL;
}


static void sample_level (Sample **level, int ix, Vector3 *vmin, Vector3 *vmax)
{
    Vector3 v;
    int iy, iz;

    v.x = (float) ((float) ix/x_steps * vmin->x + (1.0 - (float) ix/x_steps)*vmax->x);

    for (iy = 0; iy <= y_steps; iy++) {
        v.y = (float) ((float) iy/y_steps * vmin->y + (1.0 - (float)iy/y_steps)*vmax->y);

        for (iz = 0; iz <= z_steps; iz++) {
            v.z =  (float) ((float) iz/z_steps * vmin->z + (1.0 - (float)iz/z_steps)*vmax->z);

            level[iy][iz].loc   = v;
            level[iy][iz].value = func3d (v.x, v.y, v.z);
        }
    }
}


static void tesselate_cube (int iy, int iz)
{
    Sample cube[8];
    int    i, j, polysize, numline;
    Pair   line[12], temp;
    Vector3 poly[6];

    cube[0] = level0[iy][iz];
    cube[1] = level1[iy][iz];
    cube[2] = level0[iy+1][iz];
    cube[3] = level1[iy+1][iz];
    cube[4] = level0[iy][iz+1];
    cube[5] = level1[iy][iz+1];
    cube[6] = level0[iy+1][iz+1];
    cube[7] = level1[iy+1][iz+1];

    /* Analyze each of the 6 faces of the cube one at a time. For each
       face that was intersected save the intersection line */

    numline = 0;

    analyze_face (cube, 0, 1, 3, 2, line, &numline);
    analyze_face (cube, 4, 6, 7, 5, line, &numline);
    analyze_face (cube, 0, 2, 6, 4, line, &numline);
    analyze_face (cube, 1, 5, 7, 3, line, &numline);
    analyze_face (cube, 2, 3, 7, 6, line, &numline);
    analyze_face (cube, 0, 4, 5, 1, line, &numline);

    if (numline > 0) {
        /* Sort the line segments into polygons */
        polysize = 0;

        for (i = 0; i < numline; i++) {
            poly[polysize++] = line[i].p1;

            for (j = i+1; j < numline; j++) {
                if (vect_equal (&line[j].p1, &line[i].p2)) {
                    temp = line[j];
                    line[j] = line[i+1];
                    line[i+1] = temp;
                    break;
                }
            }

            if (j >= numline) {
                write_polygon (poly, polysize);
                polysize = 0;
            }
        }
    }
}


/* Analyze a cube face for surface intersections. Intersections are detected by
   looking for positive to negative field strength changes between adjacent
   corners. If an intersection is found then the coords of the line segment(s)
   are added to array line and *numline is increased */
static void analyze_face (Sample *cube, int a, int b, int c, int d,
                          Pair *line, int *numline)
{
    Vector3 points[4], vcenter;
    float center;
    int sign[4];
    int l1, l2, l3, l4;
    int index = 0;

    /* Check face edge a-b */
    if (cube[a].value * cube[b].value < 0.0) {
        sign[index] = cube[b].value > 0.0;
        interpolate (&points[index++], &cube[a], &cube[b]);
    }

    /* etc. */
    if (cube[b].value * cube[c].value < 0.0) {
        sign[index] = cube[c].value > 0.0;
        interpolate (&points[index++], &cube[b], &cube[c]);
    }

    if (cube[c].value * cube[d].value < 0.0) {
        sign[index] = cube[d].value > 0.0;
        interpolate (&points[index++], &cube[c], &cube[d]);
    }

    if (cube[d].value * cube[a].value < 0.0) {
        sign[index] = cube[a].value > 0.0;
        interpolate (&points[index++], &cube[d], &cube[a]);
    }

    if (index == 2) { /* One line segment */
        if (sign[0] > 0) {  /* Get the line direction right */
            vect_copy (&line[*numline].p1, &points[0]);
            vect_copy (&line[*numline].p2, &points[1]);
        }
        else {
            vect_copy (&line[*numline].p1, &points[1]);
            vect_copy (&line[*numline].p2, &points[0]);
        }

        (*numline)++;
    }
    else if (index == 4) { /* Two line segments */
        /* Sample the center of the face to determine which points to connect */
        vect_add (&vcenter, &cube[a].loc, &cube[b].loc);
        vect_add (&vcenter, &vcenter, &cube[c].loc);
        vect_add (&vcenter, &vcenter, &cube[d].loc);
        vect_scale (&vcenter, &vcenter, 0.25F);
        center = func3d (vcenter.x, vcenter.y, vcenter.z);

        if ((center > 0.0) != sign[0])
            { l1 = 0; l2 = 1; l3 = 2; l4 = 3; }
        else
            { l1 = 1; l2 = 2; l3 = 3; l4 = 0; }

        if (sign[l1] > 0) {
            vect_copy (&line[*numline].p1, &points[l1]);
            vect_copy (&line[*numline].p2, &points[l2]);
        }
        else {
            vect_copy (&line[*numline].p1, &points[l2]);
            vect_copy (&line[*numline].p2, &points[l1]);
        }

        (*numline)++;

        if (sign[l3] > 0) {
            vect_copy (&line[*numline].p1, &points[l3]);
            vect_copy (&line[*numline].p2, &points[l4]);
        }
        else {
            vect_copy (&line[*numline].p1, &points[l4]);
            vect_copy (&line[*numline].p2, &points[l3]);
        }

        (*numline)++;
    }
}


static void interpolate (Vector3 *v, Sample *a, Sample *b)
{
    Vector3 vtemp;

    if (a->value < b->value) {
        vect_sub (&vtemp, &a->loc, &b->loc);
        vect_scale (&vtemp, &vtemp, a->value/(a->value - b->value));
        vect_sub (v, &a->loc, &vtemp);
    }
    else {
        vect_sub (&vtemp, &b->loc, &a->loc);
        vect_scale (&vtemp, &vtemp, b->value/(b->value - a->value));
        vect_sub (v, &b->loc, &vtemp);
    }
}


static void vect_copy (Vector3 *v1, Vector3 *v2)
{
    v1->x = v2->x;
    v1->y = v2->y;
    v1->z = v2->z;
}


static int vect_equal (Vector3 *v1, Vector3 *v2)
{
    return (v1->x == v2->x && v1->y == v2->y && v1->z == v2->z);
}


static void vect_add (Vector3 *v1, Vector3 *v2, Vector3 *v3)
{
    v1->x = v2->x + v3->x;
    v1->y = v2->y + v3->y;
    v1->z = v2->z + v3->z;
}


static void vect_sub (Vector3 *v1, Vector3 *v2, Vector3 *v3)
{
    v1->x = v2->x - v3->x;
    v1->y = v2->y - v3->y;
    v1->z = v2->z - v3->z;
}


static void vect_scale (Vector3 *v1, Vector3 *v2, float k)
{
    v1->x = k * v2->x;
    v1->y = k * v2->y;
    v1->z = k * v2->z;
}

static void write_polygon (Vector3 *poly, int size)
{
    // Use the cross-product to find a normal
    // Per-vertex lighting would be nicer

    Vector3 normal;
    if (size>=3)
    {
        Vector3 a,b;
        vect_sub(&a,poly+0,poly+1);
        vect_sub(&b,poly+2,poly+1);
        normal.x = a.y*b.z - a.z*b.y;
        normal.y = a.z*b.x - a.x*b.z;
        normal.z = a.x*b.y - a.y*b.x;
    }

    //

    glBegin(GL_POLYGON);
        glNormal3f(normal.x,normal.y,normal.z);
        for (int i=size-1; i>=0; i--)
            glVertex3f(poly[i].x,poly[i].y,poly[i].z);
    glEnd();
}

// http://atrey.karlin.mff.cuni.cz/~0rfelyus/povray/index.html

template<class T> T sqr(const T &a) { return a*a; }
template<class T> T cub(const T &a) { return a*a*a; }

float sphere(float x, float y, float z)
{
    return float( x*x + y*y + z*z - 0.95 );
}

float heart (float x, float y, float z)
{
    x *= 1.3f;
    y *= 1.3f;
    z *= 1.3f;

    return float( cub(2*sqr(x)+sqr(y)+sqr(z)-1) - (0.1*sqr(x)+sqr(y))*cub(z) );
}

float klein(float x,float y,float z)
{
    x *= 5.0f;
    y *= 5.0f;
    z *= 5.0f;

    return float( (sqr(x)+sqr(y)+sqr(z)+2*y-1) * (sqr(sqr(x)+sqr(y)+sqr(z)-2*y-1)
                  -8*sqr(z)) + 16*x*z*(sqr(x)+sqr(y)+sqr(z)-2*y-1) );
}

float wave(float x,float y,float z)
{
    return float( cos(15*(sqrt(sqr(x)+sqr(y))))*0.3 - z );
}

float sss(float x,float y,float z)
{
    return float( sin(x*6.1) + sin(y*6.1) + sin(z*6.1) - 0.5 );
}
