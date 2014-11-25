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
 * This is a little program which converts a bitmap image image
 * to EPS using LTI-Lib's epsDraw functor.
 * 
 * Usage: bitmap2eps [-c <coding>] [-t <type>] [-o <output>]
 *                   [-s] [-b <rgb>] <filename>
 * 
 * <coding> is one of:
 * hex       means the image is not compressed and coded in ASCII
 *           hexadecimal digits (needs much space!).
 * asc85     means the image is not compressed and coded in ASCII-85,
 *           a code which represents 4 bytes as 5 ASCII chars.
 * deflate   means compressing the image by  zlib compression (gzip
 *           compatible). The resulting EPS can only be processed
 *           by PostScript Level 3 capable interpreters.
 * runlength the image is runlength-compressed. This usually makes
 *           things worse on noisy images, but can save much space
 *           in vector drawings with mostly background. Supported
 *           by all PS levels.
 * jpeg      the image is converted to JPEG. This is supported
 *           by PS Level 2. Due to the lossy compression, the
 *           image may show some artefacts (but mostly doesn't).
 * The default coding is asc85
 *
 * <type> is one of:
 * gray      converts the image to gray scale before creating the
 *           PS code.
 * rgb       leaves the image as RGB, creates an EPSC file.
 * bw        converts the image to a black-white bitmap.
 * The default type is rgb. 
 * 
 * <outfile> is the name of the output file. Default is <filename>.eps
 * 
 * -s means that the image is segmented prior to conversion. This works
 *    for homogeneous backgrounds only. The new background color can be
 *    set with -b.
 * 
 * <rgb> is a color specification for the new image background.
 *       The color is either a X11 color name (as found in rgb.txt),
 *       or of the form #rrggbbaa, where rr is the hex value of
 *       the red channel, etc. aa is the hex value of the alpha
 *       channel. 
 */

#include <ltiRGBPixel.h>
#include "ltiObject.h"
#include "ltiALLFunctor.h"
#include "ltiEpsDraw.h"
#include "ltiPoint.h"
#include "ltiColors.h"
#include "ltiRegionGrowing.h"
#include "ltiObjectsFromMask.h"
#include "ltiBoundingBox.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>

typedef lti::epsDraw<lti::rgbPixel> epsDraw;

// type of postscript file
typedef enum {
  rgb=0,
  gray=1,
  bw=2
} imageType;


void giveHelp(std::string cmd) {
  std::cerr
    << "This is a little program which converts a bitmap image image\n"
    << "to EPS using LTI-Lib's epsDraw functor.\n\n"
    << "Usage: bitmap2eps [-c <coding>] [-t <type>] [-o <output>]\n"
    << "                  [-s] [-b <rgb>] [-r <dpi>]<filename>\n\n"
    << "<coding> is one of:\n"
    << "hex       means the image is not compressed and coded in ASCII\n"
    << "          hexadecimal digits (needs much space!).\n"
    << "asc85     means the image is not compressed and coded in ASCII-85,\n"
    << "          a code which represents 4 bytes as 5 ASCII chars.\n"
    << "deflate   means compressing the image by  zlib compression (gzip\n"
    << "          compatible). The resulting EPS can only be processed\n"
    << "          by PostScript Level 3 capable interpreters.\n"
    << "runlength the image is runlength-compressed. This usually makes\n"
    << "          things worse on noisy images, but can save much space\n"
    << "          in vector drawings with mostly background. Supported\n"
    << "          by all PS levels.\n"
    << "jpeg      the image is converted to JPEG. This is supported\n"
    << "          by PS Level 2. Due to the lossy compression, the\n"
    << "          image may show some artefacts (but mostly doesn't).\n"
    << "auto      selects runlength or jpeg, depending what will yield the\n"
    << "          smaller file.\n"
    << "The default coding is asc85\n\n"
    << "<type> is one of:\n"
    << "gray      converts the image to gray scale before creating the\n"
    << "          PS code.\n"
    << "rgb       leaves the image as RGB, creates an EPSC file.\n"
    << "bw        converts the image to a black-white bitmap.\n"
    << "The default type is rgb. \n"
    << "\n"
    << "<outfile> is the name of the output file. Default is <filename>.eps\n"
    << "\n"
    << "-s means that the image is segmented prior to conversion. This works\n"
    << "   for homogeneous backgrounds only. The new background color can be\n"
    << "   set with -b.\n"
    << "\n"
    << "<rgb> is a color specification for the new image background.\n"
    << "      The color is either a X11 color name (as found in rgb.txt),\n"
    << "      or of the form #rrggbbaa, where rr is the hex value of\n"
    << "      the red channel, etc. aa is the hex value of the alpha\n"
    << "      channel. \n"
    << "\n"
    << "-r gives the resolution of the image file in DPI. By default, a\n"
    << "   resolution of 72 dpi is assumed.\n";
  
}



// parse a color definition. name either contains an X11 color name
// or a color definition #rrggbb[aa] with aa (the alpha channel value)
// being optional.
void parseColor(const std::string& name, lti::rgbColor& c) {
  if (name.at(0) == '#') {
    c.parseHTML(name,true);
  } else {
    c.parseName(name);
  }
}


// parses the command line arguments
void parseArgs(int argc, char*argv[], epsDraw::imageCoding& code, 
               std::string& filename, std::string& outFile, imageType& type,
               bool& dosegment, lti::rgbColor &bgcol, bool& autoCode,
               int& resolution) {
  // name of output file
  outFile="";
  // coding type of the image
  code=epsDraw::ascii85;
  // image type (color or gray-scale)
  type=rgb;
  // must we perform an object-background segmentation?
  dosegment=false;
  // new background color
  bgcol=lti::White;
  // automatic coding is off
  autoCode=false;
  // resolution is 72 dpi
  resolution=72;
  for (int i=1; i<argc; i++) {
    if (*argv[i] == '-' || *argv[i] == '+') {
      //bool val=(*argv[i] == '+');
      switch (argv[i][1]) {
        // EPS bitmap coding
        case 'c':
          if (i < argc-1) {
            ++i;
            if (strcasecmp(argv[i],"hex") == 0) {
              code=epsDraw::asciiHex;
            } else if (strcasecmp(argv[i],"asc85") == 0) {
              code=epsDraw::ascii85;
            } else if (strcasecmp(argv[i],"deflate") == 0) {
              code=epsDraw::flate;
            } else if (strcasecmp(argv[i],"jpeg") == 0) {
              code=epsDraw::jpeg;
            } else if (strcasecmp(argv[i],"runlength") == 0) {
              code=epsDraw::runLength;
            } else if (strcasecmp(argv[i],"auto") == 0) {
              code=epsDraw::runLength;
              autoCode=true;
            } else {
              std::cerr << "Warning: Coding " << argv[i] << " is not supported, "
                        << "using asc85\n";
            }
          } else {
            std::cerr << "Missing argument to -c\n";
          }
          break;
          // type  of image (RGB/Grayscale)
        case 't':
          if (i < argc-1) {
            ++i;
            if (strcasecmp(argv[i],"rgb") == 0) {
              type=rgb;
            } else if (strcasecmp(argv[i],"gray") == 0 || strcasecmp(argv[i],"grey") == 0) {
              type=gray;
            } else if (strcasecmp(argv[i],"bw") == 0) {
              type=bw;
            } else {
              std::cerr << "Warning: Type " << argv[i] << " is not supported, "
                        << "using rgb\n";
            }
          } else {
            std::cerr << "Missing argument to -t\n";
          }
          break;
        // output file name
        case 'o':
          if (i < argc-1) {
            outFile=argv[++i];
          } else {
            std::cerr << "Missing argument to -o\n";
          }
          break;
        // resolution
        case 'r':
          if (i < argc-1) {
            resolution=atoi(argv[++i]);
            if (resolution <= 0) {
              std::cerr << "Illegal argument to -r: Need a positive integer\n";
            }
          } else {
            std::cerr << "Missing argument to -r\n";
          }
          break;
        case 's':
          dosegment=true;
          break;
        case 'b':
          if (i < argc-1) {
            parseColor(argv[++i],bgcol);
          } else {
            std::cerr << "Missing argument to -b\n";
          }
          break;
        case 'h': case '?': 
          giveHelp(argv[0]); exit(0);
          break;
        default:
          std::cerr << "Unknown option " << argv[i] << "\n";
          giveHelp(argv[0]);
          exit(1);
          break;
      }
    } else {
      // input file name, set default output name
      filename=argv[i];
      if (outFile.length() == 0) {
        int dot=filename.rfind('.');
        if (dot > 0) {
          outFile=filename.substr(0,dot)+".eps";
        } else {
          outFile=filename+".eps";
        }
      }
    }
  }
  if (filename.length() == 0) {
    std::cerr << "You need to give a file name.\n";
    giveHelp(argv[0]);
    exit(1);
  }
}


// performs the object-background segmentation
void processImg(const lti::image src, lti::image& dest, 
                const lti::rgbColor bgcol) {

  // segmentation functor, is used only if doseg == true
  static bool init=false;
  static lti::regionGrowing getMask;
  static lti::objectsFromMask getContour;
  static lti::boundingBox<lti::rgbPixel> bbox;

  if (!init) {
    // initialize segmentation functors
    lti::regionGrowing::parameters segpar;

    segpar.averageThreshold=0.02;
    segpar.averageThresholds=lti::trgbPixel<float>(0.02,0.02,0.02);
    segpar.edgesThreshold=0.02;
    segpar.smoothingThreshold=0.6;
    segpar.smoothingKernelSize=3;

    //csPresegmentation getMask;
    //csPresegmentation::parameters segpar;

    //segpar.quantParameters.numberOfColors=2;
    //segpar.borderParts=csPresegmentation::parameters::Left;

    getMask.setParameters(segpar);

    lti::objectsFromMask::parameters cpar;
    cpar.level=0;
    cpar.minSize=5;
    cpar.sortObjects=true;
    getContour.setParameters(cpar);

    lti::boundingBox<lti::rgbPixel>::parameters bboxp;
    bboxp.backgroundColor=bgcol;
    bbox.setParameters(bboxp);
    init=true;
  }

  // now we are done, apply them
  std::list<lti::areaPoints> bpoints;
  lti::channel8 mask;
  getMask.apply(src,mask);
  if (!getContour.apply(mask,bpoints)) {
    std::cerr << "getContour failed: " << getContour.getStatusString() << "\n";
  }
  if (!bbox.apply(src,bpoints.front(),dest)) {
    std::cerr << "bbox failed: " << bbox.getStatusString() << "\n";
  }

}

std::string makeEPS(const lti::image& tmp, const epsDraw::imageCoding code,
                    const int r) {

  std::ostringstream out;
  epsDraw drawer(out);     
  drawer.drawImage(tmp,code);
  drawer.setSize(lti::point(tmp.columns(),tmp.rows()));
  drawer.setResolution(r);
  drawer.close();
  return out.str();
}

std::string makeEPS(const lti::channel8& tmp, const epsDraw::imageCoding code,
                    const int r, const bool bw) {

  std::ostringstream out;
  epsDraw drawer(out);     
  drawer.drawImage(tmp,code,lti::point(0,0),bw);
  drawer.setSize(lti::point(tmp.columns(),tmp.rows()));
  drawer.setResolution(r);
  drawer.close();
  return out.str();
}


int main(int argc, char* argv[]) {

  std::string fname,oname;
  epsDraw::imageCoding code;
  imageType type;
  lti::rgbColor bgcol;
  bool doseg,autoCode;
  int resolution;

  parseArgs(argc,argv,code,fname,oname,type,doseg,bgcol,autoCode,resolution);

  if (fname.length() == 0) {
    std::cerr << "You have to supply a filename.\n";
    return 1;
  }

  lti::loadImage loader;
  lti::image tmp;
  lti::image tmp2;

  // first, load the image
  if (loader.load(fname,tmp2)) {
    if (doseg) {
      // perform segmentation, if desired
      processImg(tmp2,tmp,bgcol);
    } else {
      tmp=tmp2;
    }
    // Ok, we have the image
    std::string buffer;

    // we have the file open, so initialize the EPS drawing
    
    if (type == gray || type == bw) {
      lti::channel8 tmp2;
      tmp2.castFrom(tmp);
      std::cerr <<  "Converting " << tmp2.columns() << "x" << tmp2.rows()
                << " channel\n";
      if (autoCode) {
        std::string out=makeEPS(tmp2,epsDraw::runLength,resolution, type == bw);
        std::string out2=makeEPS(tmp2,epsDraw::jpeg,resolution, type == bw);
        buffer=out.length() < out2.length() ? out : out2;
      } else {
        buffer=makeEPS(tmp2,code,resolution,type == bw);
      }
    } else {
      std::cerr << "Converting " << tmp.columns() << "x" << tmp.rows()
                << " image\n";
      if (autoCode) {
        std::string out=makeEPS(tmp,epsDraw::runLength,resolution);
        std::string out2=makeEPS(tmp,epsDraw::jpeg,resolution);;
        buffer=out.length() < out2.length() ? out : out2;
      } else {
        buffer=makeEPS(tmp,code,resolution);
      }
    }
    if (oname == "-") {
      // write to stdout
      std::cout << buffer;
    } else {
      std::ofstream out(oname.c_str());
      if (out.is_open() && out.good()) {
        out << buffer;
      } else {
        std::cerr << "Cannot open output file " << oname << "\n";
      }
    }
  } else {
    std::cerr << "Cannot load image: " << loader.getStatusString() << "\n";
  }
}

