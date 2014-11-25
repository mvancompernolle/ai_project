#include "raypp.h"

/*! \file
    \ingroup GlutMaster
*/

#ifdef GLUTM_RAYPP

#include <raypp/kernel/handle.h>
#include <raypp/worlds/scene.h>
#include <raypp/worlds/hmakers/pov_hmaker.h>
#include <raypp/renderers/raytracer.h>
#include <raypp/outputs/tga_output.h>
#include <raypp/cameras/ortho_camera.h>
#include <raypp/lights/pointlight.h>
#include <raypp/lights/ambient_light.h>

#include <math/matrix4.h>

#include <cassert>

namespace RAYPP {

extern HANDLE<WORLD> World;
extern HANDLE<RENDERER> Renderer;

} // namespace RAYPP

using namespace RAYPP;
using namespace std;

SCENE        *scene = NULL;

bool
initScene()
{
    World = scene = new (countable) SCENE;

    scene->Add(new (countable) POINTLIGHT(VECTOR (-20,30,20), COLOUR (600,600,600)));
    scene->Add(new (countable) POINTLIGHT(VECTOR (20,20,40), COLOUR (600,600,600)));
    scene->Add(new (countable) AMBIENT_LIGHT(COLOUR (1,1,1)));

    scene->Set_HMaker (new (countable) POV_HMAKER());

    return true;
}

bool
raytraceScene(const std::string &filename,const int width,const int height,const Matrix &modelview)
{
    assert(width>0 && height>0);

    RAYTRACER *raytracer = new (countable) RAYTRACER;
    raytracer->Set_Shadow_Rays(false);
    raytracer->Set_Super_Samples(2);
    Renderer = raytracer;

    ORTHO_CAMERA *camera = new (countable) ORTHO_CAMERA;
    camera->Set_Infinite(true);

    TRANSFORM transform;
    {
        TRANSMAT matrix;
        for (int i=0;i<4;i++)
            for (int j=0;j<3;j++)
                matrix.entry[i][j] = modelview[i*4+j];

        matrix.Invert();
        transform.Make_General_Transform(matrix);
        camera->Transform(transform);
    }

    raytracer->Add(camera);
    camera->Set_Aspect_Ratio(double(width)/double(height));

    HANDLE<OUTPUT> output;
    output = new (countable) TGA_OUTPUT(width,height,filename.c_str());
    output->Init();

    scene->Init();
    Renderer->Init();
    output->Render();

    return true;
}

#endif
