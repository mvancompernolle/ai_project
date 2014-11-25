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
 * This is a little tool that creates a collage from a list of
 * given bitmap images. It arranges the images (or excerpts of them)
 * in a rectangular grid. An interesting feature is that you can do
 * an object-background segmentation on each arranged image. That way,
 * you get only the most interesting parts of an image.
 * 
 * Usage: collage [options] files ...
 * Options are:
 * +S      do object-background segmentation on each image prior to
 *         insertion in the collage.
 * -t <f>  threshold parameter for segmentation
 * -e <f>  edge threshold parameter for segmentation
 * -k <n>  smoothing kernel size for segmentation
 * -s <n>  smoothing threshold size for segmentation
 * -b <c>  background color
 * 
 * -c <n>  number of images per row
 * -w <n>  width of a single collage element
 * -h <n>  height of a single collage element
 * -d <n>  downsampling factor of the collage
 * +v      show intermediate results
 * -o <s>  output file. Default output file is collage.png
 */

#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cstdio>
#include <string>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <ltiRGBPixel.h>
#include <ltiURL.h>
#include <ltiEpsDraw.h>
#include <ltiViewer.h>
#include <ltiALLFunctor.h>
#include <ltiOpponentColor.h>
#include <ltiAxOGDFeature.h>
#include <ltiOgdFilter.h>
#include <ltiGeometricTransform.h>
#include <ltiDownsampling.h>
#include <ltiColors.h>
#include <ltiRegionGrowing.h>
#include <ltiObjectsFromMask.h>
#include <ltiBoundingBox.h>
#include <ltiCsPresegmentation.h>

using std::cout;
using std::cerr;
using std::endl;


void parseColor(const std::string& name, lti::rgbColor& c) {
  int r,g,b,a=0;
  const char *cname=name.c_str();
  if (*cname == '#') {
    char cdef[3];
    cdef[2]='\0';
    strncpy(cdef,cname+1,2);
    sscanf(cdef,"%x",&r);
    strncpy(cdef,cname+3,2);
    sscanf(cdef,"%x",&g);
    strncpy(cdef,cname+5,2);
    sscanf(cdef,"%x",&b);
    if (strlen(cname) > 7) {
      // we have an alpha channel
      strncpy(cdef,cname+7,2);
      sscanf(cdef,"%x",&a);
    }

  } else {
    std::ifstream rgbDefs("/usr/X11R6/lib/X11/rgb.txt");

    bool notFound=true;

    while (!rgbDefs.eof() && notFound) {
      std::string tmp;
      std::getline(rgbDefs,tmp);
      const char* t=tmp.c_str();
      if (*t == '!') {
        continue;
      }
      const char* cdef=t;
      t+=12;
      while (isspace(*t)) t++;
      if (name == t) {
        sscanf(cdef,"%d %d %d",&r,&g,&b);
        notFound=false;
      }
    }
    if (notFound) {
      std::cerr << "Unknown color " << name << ". Exiting.\n";
    }
  }
  c.setRed(r);
  c.setGreen(g);
  c.setBlue(b);
  c.setDummy(a);
}

void giveHelp(std::string cmd) {
  std::cerr  << "Collage v0.1, an LTIlib example\n";
  std::cerr << "Assembles several images into one large image\n";
  std::cerr << "Usage: " << cmd << " [options] { files }\n";
  std::cerr 
    << "Valid options are:\n"
    << "-S      do object-background segmentation of source images;\n"
    << "        default is no segmentation\n"
    << "-t <r>  set average threshold for segmentation\n"
    << "        default is 0.02\n"
    << "-e <r>  set egde threshold for segmentation\n"
    << "        default is 0.02\n"
    << "-k <n>  set smoothing kernel size for segmentation\n"
    << "        default is 3\n"
    << "-s <r>  set smoothing threshold for segmentation\n"
    << "        default is 0.6\n"
    << "-b <c>  set the background color (in form #rrggbb) for the segmented images\n"
    << "        default is black\n\n"
    << "-c <n>  set the number of columns in which images should be arranged\n"
    << "        default is 4\n"
    << "-w <n>  set the width of the tiles\n"
    << "        default is 128\n"
    << "-h <n>  set the height of the tiles\n"
    << "        default is 128\n"
    << "-d <r>  set the downsampling factor of the result image\n"
    << "        default is 2.0\n"
    << "-v      use the image viewer for viewing intermediate and final results\n"
    << "        default is no viewer\n"
    << "-o <s>  set the name of the output file\n"
    << "        default is collage.png\n";
  
}



  void parseArgs(int argc, char*argv[], std::list<std::string>& filelist,
                 float& st, float& et, int &sks, float& sm,
                 int& cols, int &width, int &height, float& down,
                 bool& doSegment, std::string& resFile, bool& useViewer,
                 lti::rgbColor& bgcol) {
    st=0.02;
    et=0.02;
    sks=3;
    sm=0.6;
    cols=4;
    filelist.clear();
    width=128;
    height=128;
    down=2.0;
    doSegment=false;
    resFile="collage.png";
    useViewer=false;
    bgcol=lti::Black;
    for (int i=1; i<argc; i++) {
      if (*argv[i] == '-' || *argv[i] == '+') {
        bool val=(*argv[i] == '+');
        switch (argv[i][1]) {
          // determine if we should segment the image
          case 'S': doSegment=val; break;
            // determine the average threshold for segmentation
          case 't': 
            if (i < argc-1) {
              st=atof(argv[++i]);
            } else {
              std::cerr << "Missing floating-point argument.\n";
            }
            break;
            // determine the edge threshold for segmentation
          case 'e':
            if (i < argc-1) {
              et=atof(argv[++i]);
            } else {
              std::cerr << "Missing floating-point argument.\n";
            }
            break;
            // determine the smoothing kernel size for segmentation
          case 'k':
            if (i < argc-1) {
              sks=atoi(argv[++i]);
            } else {
              std::cerr << "Missing integer argument.\n";
            }
            break;
            // determine the smoothing threshold size for segmentation
          case 's':
            if (i < argc-1) {
              sm=atof(argv[++i]);
            } else {
              std::cerr << "Missing floating-point argument.\n";
            }
            break;
            // determine the number of images per row
          case 'c':
            if (i < argc-1) {
              cols=atoi(argv[++i]);
            } else {
              std::cerr << "Missing integer argument.\n";
            }
            break;
            // determine the width of an element of the collage
          case 'w':
            if (i < argc-1) {
              width=atoi(argv[++i]);
            } else {
              std::cerr << "Missing integer argument.\n";
            }
            break;
            // determine the height of an element of the collage
          case 'h':
            if (i < argc-1) {
              height=atoi(argv[++i]);
            } else {
              std::cerr << "Missing integer argument.\n";
            }
            break;
            // determine the downsampling factor
          case 'd':
            if (i < argc-1) {
              down=atof(argv[++i]);
            } else {
              std::cerr << "Missing floating-point argument.\n";
            }
            break;
            // determine the output file
          case 'o':
            if (i < argc-1) {
              resFile=argv[++i];
            } else {
              std::cerr << "Missing string argument.\n";
            }
            break;
          case 'v':
            useViewer=val;
            break;
            // determine the background color
          case 'b':
            if (i < argc-1) {
              parseColor(argv[++i],bgcol);
            } else {
              std::cerr << "Missing color argument.\n";
            }
            break;
          case '?': giveHelp(argv[0]); exit(0); break;
          default: break;
            std::cerr << "Invalid option " << argv[i] << "\n";
            giveHelp(argv[0]);
            exit(1);
            break;
        }
      } else {
        filelist.push_back(argv[i]);
      }
    }
    if (filelist.size() == 0) {
      giveHelp(argv[0]);
      exit(1);
    }
  }


int xstep,ystep,width,height,dx,dy;
  
// add tmp2 to result at position px/py.
// px and py refer to the column/row index, not absolute pixel positions
void addToCollage(lti::image& result, const lti::image& tmp2,
                  const int px, const int py) {

  // determine absolute position: ystep is the height of one
  // collage element plus some space
  // width and height are the desired size of one collage
  // cell. 
  int ypos=dy+py*ystep+(height-tmp2.rows())/2; 
  int xpos= dx+px*xstep+(width-tmp2.columns())/2; 
  if (tmp2.rows() < height) {
    if (tmp2.columns() < width) {
      // image is smaller than result cell, so copy entire image
      result.fill(tmp2,ypos,xpos);
    } else {
      // image is wider than result cell, so copy only a part
      result.fill(tmp2,ypos,dx+px*xstep,
                  ypos+height,dx+px*xstep+width,
                  0,(tmp2.columns()-width)/2);
    }
  } else {
    if (tmp2.columns() < width) {
      // image is higher than result cell, so copy only a part
      result.fill(tmp2, dy+py*ystep,xpos,
                  dy+py*ystep+height,xpos+width,
                  (tmp2.rows()-height)/2,0);
    } else {
      // image is larger than result cell, so copy only a part
      result.fill(tmp2, dy+py*ystep, dx+px*xstep,
                  height+dy+py*ystep, dx+px*xstep+width,
                  (tmp2.rows()-height)/2,(tmp2.columns()-width)/2);
    }
  }
}



int main(int argc, char* argv[]) {

  lti::loadImage loader;
  lti::saveImage saver;

  std::list<std::string> files;

  float avgThresh,edgeThresh,smoothThresh;
  int smoothSize;
  int cols;
  float down;
  bool doseg,useViewer;
  std::string outputFile;
  lti::rgbColor bgcol;

  parseArgs(argc,argv,files,avgThresh,edgeThresh,smoothSize,smoothThresh,
            cols,width,height,down,doseg,outputFile, useViewer, bgcol);

  // initialize global variables
  dx=1;
  dy=1;

  xstep=width+dx;
  ystep=height+dy;

  // determine number of rows
  int rows=files.size()/cols;
  if (files.size()%cols != 0) {
    rows++;
  }

  lti::image result(1,1);
  lti::image tmp,tmp2;
  
  // initialize segmentation functors, are used only if doseg == true
  lti::regionGrowing getMask;
  lti::regionGrowing::parameters segpar;

  segpar.averageThreshold=avgThresh;
  segpar.averageThresholds=lti::trgbPixel<float>(avgThresh,avgThresh,avgThresh);
  segpar.edgesThreshold=edgeThresh;
  segpar.smoothingThreshold=smoothThresh;
  segpar.smoothingKernelSize=smoothSize;

  //csPresegmentation getMask;
  //csPresegmentation::parameters segpar;

  //segpar.quantParameters.numberOfColors=2;
  //segpar.borderParts=csPresegmentation::parameters::Left;

  getMask.setParameters(segpar);

  lti::objectsFromMask getContour;
  lti::objectsFromMask::parameters cpar;
  cpar.level=0;
  cpar.minSize=5;
  cpar.sortObjects=true;
  getContour.setParameters(cpar);

  std::list<lti::areaPoints> bpoints;

  lti::boundingBox<lti::rgbPixel> bbox;
  lti::boundingBox<lti::rgbPixel>::parameters bboxp;

  bboxp.backgroundColor=bgcol;
  bbox.setParameters(bboxp);

  lti::viewer *v=0;
  lti::viewer *m=0;

  result.resize(dy+rows*ystep,dx+cols*xstep);
  result.fill(bgcol);

  int i=0;
  
  // iterate over all files
  for (std::list<std::string>::const_iterator fi=files.begin();
       fi != files.end(); fi++) {
    lti::channel8 mask;
    std::cerr << "Processing " << *fi << endl;
    if (!loader.load(*fi,tmp)) {
      std::cerr << "loading " << *fi << " failed: "
                << loader.getStatusString() << endl;
    }
    if (doseg) {
      // do segmentation if desired
      getMask.apply(tmp,mask);
      if (useViewer) {
        lti::viewer::parameters vp;
        vp.title=*fi;
        vp.title+=" segmented";
        v->setParameters(vp);
        vp.title=*fi;
        vp.title+=" mask";
        m->setParameters(vp);
        m->show(mask);
      }
      if (!getContour.apply(mask,bpoints)) {
        std::cerr << "getContour failed: " << getContour.getStatusString()
                  << endl;
      }
      if (!bbox.apply(tmp,bpoints.front(),tmp2)) {
        cerr << "bbox failed: " << bbox.getStatusString() << endl;
      }
    } else {
      tmp2=tmp;
    }
    if (useViewer) {
      v->show(tmp2);
    }
    // add image to the result
    addToCollage(result,tmp2,i%cols,i/cols);
    i++;
  }

  // use downsampling if possible
  if ((float)((int)down) == down) {
    // whole number sampling factor
    lti::downsampling sampler;
    lti::downsampling::parameters downpar;
    downpar.factor=lti::point(static_cast<int>(down),static_cast<int>(down));
    sampler.setParameters(downpar);
    sampler.apply(result);
  } else {
    // float number sampling factor
    lti::geometricTransform sampler;
    lti::geometricTransform::parameters downpar;
    downpar.keepDimensions=false;
    downpar.scale(1.0/down);
    sampler.setParameters(downpar);
    sampler.apply(result);
  }


  saver.save(outputFile,result);

  //getchar();
  if (useViewer) {
    delete m;
    delete v;
  }
}

