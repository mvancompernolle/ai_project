/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Usage distort [-h] [+d angleStep] [-t degree] [-f filesWithFilenames]
 */


#include <fstream>
#include <cmath>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <iostream>

#include <ltiRGBPixel.h>
#include <ltiHTypes.h>
#include <ltiMath.h>
#include <ltiModifier.h>
#include <ltiSplitImageTorgI.h>
#include <ltiGeometricTransform.h>
#include <ltiPNGFunctor.h>
#include <ltiViewer.h>
#include <ltiFastViewer.h>

#include "ltiTimer.h"
#include <ltiGtkServer.h>

#include <iostream>
#include <fstream>


using std::cout;
using std::endl;

class fileReader {

public:
  fileReader(const char* fname="files.txt") {
    input.open(fname);
  };

  ~fileReader() {
    input.close();
  }

  bool next(std::string& filename) {
    if (input.good() && !input.eof()) {
      do {
        getline(input,filename);
        std::string::size_type pos;
        pos = filename.find(';');
        if (pos != std::string::npos) {
          filename.erase(pos);
        }
      } while (!input.eof() && (filename == ""));
      return (filename != "");
    } else {
      return false;
    }
  };

protected:
  std::ifstream input;
};

void usage() {
  std::cout 
    << "distort [-h] [-f filename] [-t val] [+/-d step]\n"
    << " -h          : show this help \n" 
    << " -f filename : indicate file with the image files to distort\n"
    << " -t val      : tumble the given floating point value \n"
    << " -d step     : angular step in degrees for the rotation\n" 
    << std::endl;
  exit(EXIT_SUCCESS);
}

void parseArgs(int argc, char*argv[], std::string& flistname, int& dir,
               double& tumble) {
  flistname="files.txt";
  dir=3;
  tumble=20;
  for (int i=1; i<argc; i++) {
    if (*argv[i] == '-' || *argv[i] == '+') {
      bool val=(*argv[i] == '+');
      switch (argv[i][1]) {
        case 'f': flistname=argv[++i]; break;
        case 't': tumble=atof(argv[++i]); break;
        case 'd':
          dir=atoi(argv[++i]);
          if (!val) {
            dir=-dir;
          }
          break;
        case 'h': usage(); break;
        default: break;
      }
    }
  }
}

inline double deg2rad(double f) {
  return lti::Pi*f/180.0;
}



int main(int argc, char* argv[]) {

  std::string flistname;
  int diff;
  double tumble;

  parseArgs(argc,argv,flistname,diff,tumble);

  lti::viewer viewOrig("Original");
  lti::viewer viewDist("Distort");
  lti::viewer viewRot("Rotated");

  lti::geometricTransform trans;
  lti::geometricTransform rot;

  lti::geometricTransform::parameters transPar;
  lti::geometricTransform::parameters rotPar;

  transPar.boundaryType = lti::modifier::parameters::Constant;
  transPar.keepDimensions = true;
  rotPar.copy(transPar);

  lti::loadPNG loader;
  lti::savePNG saver;
  lti::splitImageTorgI splitter;

  lti::image img;
  lti::channel chnl,chnl2,chnl3;

  fileReader theFiles(flistname.c_str());
  std::string filename;
  double rad;
  int pause = 100000;

  while (theFiles.next(filename)) {

    loader.load(filename,img);

    // --------------------------------------------------------------------

    splitter.getIntensity(img,chnl);

    //      chnl.fill(0.0f,0,chnl.columns()-10,10,chnl.columns()-1);

    viewOrig.show(chnl);
    lti::passiveWait(pause);

    lti::dvector center(3);
    center[0]=img.columns()/2;
    center[1]=img.rows()/2;
    center[2]=0;
    lti::dvector axis(3);
    axis[0]=0;
    axis[1]=0;
    axis[2]=1;

    int k=0;

    for (int a=0; a<360.0f && a > -360.0f; a+=diff) {
      cout << "Processing file: " << filename << " : (" 
           << a << " deg)" << endl;

      rad = deg2rad(a%360);
      transPar.transMatrix.setRotation(deg2rad(sqrt(double(a)/360.0)*tumble),
                                       lti::hPoint3D<float>(cos(4*rad),
                                                            sin(4*rad),
                                                            0.0f),
                                       lti::hPoint3D<float>(0.5*chnl.columns(),
                                                            0.5*chnl.rows(),
                                                            0));
      trans.setParameters(transPar);

      trans.apply(chnl,chnl2);

      viewDist.show(chnl2);
      lti::passiveWait(pause);

      rotPar.clear();
      rotPar.rotate(center,axis,deg2rad(a));
      rot.setParameters(rotPar);
      rot.apply(chnl2,chnl3);

      viewRot.show(chnl3);
      lti::passiveWait(pause);

      std::string path,name;
      char buf[256];

      int pos;
      path = filename.substr(0,pos = filename.rfind('/'));
      name = filename.substr(pos,filename.rfind('.')-pos);

      sprintf(buf,"%s%s_%03d.png",path.c_str(),name.c_str(),k++);

      // cout << "new filename: " << buf << endl;
      saver.save(buf,chnl3);
      // getchar();
      pause=1;
    }

  }

  viewRot.hide();
  viewDist.hide();
  viewOrig.hide();
  lti::passiveWait(100000);
  lti::gtkServer::shutdown();
}

