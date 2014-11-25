#include <glutm/raypp.h>

#ifdef GLUTM_RAYPP

#include <raypp/worlds/scene.h>
#include <raypp/objects/simple_object.h>
#include <raypp/shapes/triangle.h>
#include <raypp/shapes/triangle_smooth.h>
#include <raypp/surfaces/phong.h>
#include <raypp/worlds/hmakers/pov_hmaker.h>

#include <mesh/mesh.h>

#include <string>
#include <cassert>

using namespace std;
using namespace RAYPP;

unsigned int priority = 1;

void initSceneMesh(const Mesh &mesh)
{
    SURFACE *surf = new (countable) PHONG(0.4, 0.8, 2.0,0,0,20, COLOUR(0.9,0.9,1.0));

    const std::deque<Triangle> &tri = mesh.triangle();
    for (int i=0; i<tri.size(); i++)
    {
        const Triangle &t = tri[i];

        const Point *a = t[0];
        const Point *b = t[1];
        const Point *c = t[2];

        assert(a && b && c);

        const VECTOR v1(a->x(),a->y(),a->z());
        const VECTOR v2(b->x(),b->y(),b->z());
        const VECTOR v3(c->x(),c->y(),c->z());

        const VECTOR n1(a->normal().x(),a->normal().y(),a->normal().z());
        const VECTOR n2(b->normal().x(),b->normal().y(),b->normal().z());
        const VECTOR n3(c->normal().x(),c->normal().y(),c->normal().z());

        SIMPLE_OBJECT *obj = new (countable) SIMPLE_OBJECT;
        obj->Set_Shape (new (countable) TRIANGLE_SMOOTH(v1,n1,v2,n2,v3,n3));
        obj->Set_Surface(surf);
        obj->Set_Priority(priority);
        scene->Add (obj);
    }
}

#endif


/*
#ifdef WIN32
#define USE_RAYPP
#endif

#ifdef USE_RAYPP
#include <raypp/kernel/handle.h>
#include <raypp/worlds/scene.h>
#include <raypp/worlds/hmakers/pov_hmaker.h>
#include <raypp/renderers/raytracer.h>
#include <raypp/outputs/tga_output.h>
#include <raypp/shapes/box.h>
#include <raypp/shapes/sphere.h>
#include <raypp/shapes/plane.h>
#include <raypp/shapes/triangle.h>
#include <raypp/surfaces/phong.h>
//#include <raypp/surfaces/matte.h>
#include <raypp/objects/simple_object.h>
#include <raypp/lights/pointlight.h>
#include <raypp/lights/ambient_light.h>
#include <raypp/cameras/ortho_camera.h>

#include <mesh/mesh.h>
#include <math/matrix4.h>

#include <cmath>
#include <cassert>

namespace RAYPP {

extern HANDLE<WORLD> World;
extern HANDLE<RENDERER> Renderer;

} // namespace RAYPP


void Raytrace(const Mesh &mesh,const Matrix &camera,int width,int height,char *filename)
{
  using namespace RAYPP;

  HANDLE<OUTPUT> Output;
  Output = new (countable) TGA_OUTPUT (width, height, filename);
  Output->Init();

  SCENE *Scn = new (countable) SCENE;
  RAYTRACER *Raytracer = new (countable) RAYTRACER;
    ORTHO_CAMERA *Camera = new (countable) ORTHO_CAMERA (VECTOR(0.0,0.0,1000.0), VECTOR(0,0,0));

  Camera->Set_Aspect_Ratio(double(width)/double(height));
  Raytracer->Add(Camera);

  SIMPLE_OBJECT *obj;
  uint4 Priority = 1;

    TRANSFORM transform;
    TRANSMAT matrix;

    {
        for (int i=0;i<4;i++)
            for (int j=0;j<3;j++)
                matrix.entry[i][j] = camera[i*4+j];
    }

//  matrix.entry[0][0] *= -1;
//  matrix.entry[1][0] *= -1;
//  matrix.entry[2][0] *= -1;

    transform.Make_General_Transform(matrix);

//      TRANSFORM camTrans;
//      camTrans.Make_General_Transform(matrix.Inverse());
//      Camera->Transform(camTrans);

  Scn->Add
    (new (countable) POINTLIGHT (VECTOR (-20,30,20), COLOUR (600,600,600)));
  Scn->Add
    (new (countable) POINTLIGHT (VECTOR (20,20,40), COLOUR (600,600,600)));
  Scn->Add
    (new (countable) AMBIENT_LIGHT (COLOUR (1,1,1)));

  SURFACE *surf = new (countable) PHONG(0.4, 0.8, 2.0,0,0,20, COLOUR(0.9,0.9,1.0));

  const std::deque<Triangle> &tri = mesh.triangle();
    for (int i=0; i<tri.size(); i++)
    {
        const Triangle &t = tri[i];

        const Point *a = t[0];
        const Point *b = t[1];
        const Point *c = t[2];

        assert(a && b && c);

        const VECTOR aa(a->x(),a->y(),a->z());
        const VECTOR bb(b->x(),b->y(),b->z());
        const VECTOR cc(c->x(),c->y(),c->z());

        obj = new (countable) SIMPLE_OBJECT;
        obj->Set_Shape (new (countable) TRIANGLE(aa,bb,cc));
        obj->Set_Priority (Priority);
        obj->Transform(transform);
        obj->Set_Surface(surf);
        Scn->Add (obj);
    }

    Scn->Set_HMaker (new (countable) POV_HMAKER());

//

  World = Scn;
  World->Init();
  Renderer = Raytracer;
  Renderer->Init();

  Output->Render();
}
#else
void Raytrace(Mesh &mesh,int width,int height,char *filename)
{
}
#endif
*/

