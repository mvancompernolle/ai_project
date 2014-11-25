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

#include <fstream>
#include <cmath>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <list>
#include <vector>
#include <iostream>

#include <ltiRGBPixel.h>
#include <ltiImage.h>
#include <ltiDraw.h>
#include <ltiDraw3D.h>
#include <ltiMath.h>
#include <ltiComputePalette.h>
#include <ltiUsePalette.h>
#include <ltiKMeansSegmentation.h>
#include <ltiPNGFunctor.h>
#include <ltiViewer.h>
#include <ltiLispStreamHandler.h>

#include <iostream>
#include <fstream>


using std::cout;
using std::endl;

/**
 * This class will read the file given in the constructor.
 * Each line in the file will be expected to have a complete filename
 * of the images (in PNG format) to be loaded.
 */
class fileReader {

public:
  /**
   * Default constructor.
   * Expects the file name of the file containing the image names to be
   * used in the example
   */
  fileReader(const char* fname="files.txt") {
    input.open(fname);
  };

  /**
   * destructor
   */
  ~fileReader() {
    input.close();
  }

  /**
   * get the next image name in the file
   */
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

/**
 * This class is used to generate a bitmap showing the clusters
 *
 */
class clusterViewer {
public:
  /**
   * default constructor
   */
  clusterViewer() {
  };

  /**
   * draw the clusters
   */
  void draw(const lti::draw3D<lti::rgbPixel>::parameters& camPar,
            const lti::vector< lti::trgbPixel<float> >& pal,
            const std::vector< lti::matrix<float> >& palCovar) {

    int i;
    for (i=0;i<pal.size();++i) {
      cout << "[Entry " << i << "] " << endl;
      cout << "  Mean : " << pal.at(i) << endl;
      cout << "  Covar: " << palCovar[i] << endl;
    }
  }


protected:
};


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

  // get the command line parameters
  parseArgs(argc,argv,flistname,diff,tumble);

  // the viewers required
  lti::viewer viewOrig("Original");


  // the k-Means Segmentation functor used in color quantization
  // the parameters are taken from a file called kmeans.dat
  lti::kMeansSegmentation kmeans;
  lti::kMeansSegmentation::parameters kmPar;
  std::ifstream in("kmeans.dat");

  if (in) {
    lti::lispStreamHandler lsh(in);
    kmPar.read(lsh);
    in.close();
  } else {
    kmPar.quantParameters.maximalNumberOfIterations = 50;
    kmPar.quantParameters.thresholdDeltaPalette = 0.3f;
    kmPar.quantParameters.numberOfColors = 8;

    std::ofstream out("kmeans.dat");
    lti::lispStreamHandler lsh(out);
    kmPar.write(lsh);
    out.close();
  }

  kmeans.setParameters(kmPar);

  // other functors required
  lti::usePalette colorizer;
  lti::computePalette computePal;

  lti::loadPNG loader;
  lti::savePNG saver;

  lti::image img;
  lti::matrix<int> mask;
  lti::palette pali;
  lti::vector<lti::trgbPixel<float> > pal;
  std::vector<lti::matrix<float> > palCovar;
  lti::vector<int> ns;
  clusterViewer clViewer;

  fileReader theFiles(flistname.c_str());
  std::string filename;


  lti::draw3D<lti::rgbPixel>::parameters camPar;


  while (theFiles.next(filename)) {

    loader.load(filename,img);

    // --------------------------------------------------------------------

    kmeans.apply(img,mask,pali);
    viewOrig.show(img);

    computePal.apply(img,mask,pal,palCovar,ns);
    clViewer.draw(camPar,pal,palCovar);
  }
}

