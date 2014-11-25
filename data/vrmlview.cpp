/*

  Glt OpenGL C++ Toolkit (Glt)
  Copyright (C) 2000 Nigel Stewart
  Email: nigels@nigels.com   WWW: http://www.nigels.com/glt/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "vrmlview.h"

#include <mesh/vrmlpars.h>

#include <iostream>
using namespace std;

GltShapes *insert = NULL;
void insertShape(GltShapePtr &shape)
{
    if (insert)
        insert->push_back(shape);
}

#include <mesh/md3.h>
#include <mesh/read3dsb.h>

#include <misc/string.h>
#include <glutm/axes.h>

#include <fstream>

GlutVrmlViewer::GlutVrmlViewer(const std::string &filename,int width,int height,int x,int y,unsigned int displayMode)
: GlutWindowExaminer(filename,width,height,x,y,displayMode),
  _light0(GL_LIGHT0),
  _light1(GL_LIGHT1)
#ifdef OPENVRML
, _scene(NULL)
, _viewer(NULL)
#else
//, _mesh(filename,MESH_VRML)
#endif
{
    #ifdef OPENVRML
    loadScene(filename);
    #else

/*
    insert = &_shapes;
    VrmlParseShape = insertShape;

    Vrmlin = fopen(filename.c_str(),"rt");
    if (Vrmlin)
    {
        Vrmlparse();
        fclose(Vrmlin);
    }

    Vrmlin = NULL;
    insert = NULL;
    VrmlParseShape = NULL;
  */

    string buffer;
    readFile(filename,buffer);
    _shapes = read3DS(reinterpret_cast<const byte *>(&buffer[0]),buffer.size());

    #endif

    // Setup raytracing data structures

    #ifdef GLUTM_RAYPP
    initScene();
    initSceneMesh(_mesh);
    #endif
}

GlutVrmlViewer::~GlutVrmlViewer()
{
}

void GlutVrmlViewer::OnOpen()
{
    _ortho.zNear() = -100;
    _ortho.zFar() = 100;

    // Setup OpenGL lighting

    _light0.ambient()  = GltColor(0.1, 0.1, 0.1, 1.0);
    _light1.specular() = white;
    _light0.position() = Vector( 50.0, 50.0, 0);
    _light0.enabled()  = true;
    _light0.set();

    _light1.ambient()  = GltColor(0.1, 0.1, 0.1, 1.0);
    _light1.specular() = white;
    _light1.position() = Vector(-50.0, 50.0, 0);
    _light1.enabled()  = true;
    _light1.set();

    _material.ambient()   = GltColor(0.2, 0.2, 0.2, 0.0);
    _material.specular()  = white;
    _material.shininess() = 40.0;
    _material.set();

    _lightModel.setAmbient(0.0, 0.0, 0.0, 0.0);
    _lightModel.setLocalViewer(GL_FALSE);
    _lightModel.setTwoSide(GL_TRUE);

    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
//  glShadeModel(GL_FLAT);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glEnable(GL_LIGHTING);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    glEnable(GL_TEXTURE_2D);

    /*
    {
        string base("models\\players\\ed209\\");

        MD3Object *lower = new MD3Object();
        lower->loadModel(base + "lower.md3");
        lower->loadSkin(base + "lower_default.skin");

        MD3Object *upper = new MD3Object();
        upper->loadModel(base + "upper.md3");
        upper->loadSkin(base + "upper_default.skin");
        lower->link(upper,"tag_torso");

        MD3Object *head = new MD3Object();
        head->loadModel(base + "head.md3");
        head->loadSkin(base + "head_default.skin");
        upper->link(head,"tag_head");

        _shapes.push_back(lower);

        GltShape *axes = new GlutAxes();
        axes->transformation() = matrixScale(10.0);
        _shapes.push_back(axes);
    }
    */


    glClearColor(0.2, 0.2, 0.2, 0.0);

    //

    // Fit the objects into the unit cube.

    BoundingBox box;
    _shapes.boundingBox(box);

    if (box.defined())
    {
        _shapes.transformation() *= matrixTranslate(-box.center());

        const double size = MAX(box.width(),MAX(box.height(),box.depth()));

        if (size>1e-6)
            _shapes.transformation() *= matrixScale(1.8/size);
    }

    // Compile the display list

    #ifndef OPENVRML
    _list.newList();
//  _mesh.draw();
    _shapes.draw();
    _list.endList();
    #endif
}

void
GlutVrmlViewer::OnClose()
{
    _shapes.clear();
    _list.reset();
}

void GlutVrmlViewer::OnDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #ifdef OPENVRML
    drawScene();
    #else
//    _list.draw();
     _shapes.draw();
   #endif
}

void
GlutVrmlViewer::OnKeyboard(unsigned char key, int x, int y)
{
    GlutWindowExaminer::OnKeyboard(key,x,y);

    switch (key)
    {
    #ifdef GLUTM_RAYPP
    case 'r':
        raytraceScene("vrmlview.tga",_viewport.width(),_viewport.height(),Matrix(GL_MODELVIEW_MATRIX));
        break;
    #endif
    default:
        break;
    }
}

#include <glutm/main.h>

/*
   Sources of VRML models...

   RC CAD
   http://209.15.130.75/index.html

*/

bool GlutMain(const std::vector<std::string> &arg)
{
    string filename("sample.wrl");

    if (arg.size()>1)
        filename = arg[1];

/*
    {
        string md3;
        ifstream is(filename.c_str(),ios::binary);
        readStream(is,md3);
        cout << parseMD3(md3.c_str(),md3.size()) << endl;
    }
*/
    static GlutWindow *main = new GlutVrmlViewer(filename.c_str(),450,450,10,10);
    main->open();

    return true;
}


