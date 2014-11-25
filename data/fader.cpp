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
 * This is a little program which transforms an image as follows:
 * - it extracts the intensity channel
 * - it then maps the intensity value on a range between two arbitrary
 *   colors; the colors can also specify the alpha channel value
 * - it saves the new image
 *
 * The use of the command line would be:
 *
 * fader [options] imgfile
 * 
 * where the options can be:
 *
 * -a use alpha channel
 * -b #RRGGBBAA, color which takes black's role (in hex)
 * -w #RRGGBBAA, color which takes white's role
 * -o output filename (if omitted the output file will be "faded-*" with
 *                     * the given input file name)
 */

#include <fstream>
#include <cmath>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <iostream>

#include <ltiRGBPixel.h>
#include <ltiMath.h>
#include <ltiModifier.h>
#include <ltiSplitImageToHSI.h>
#include <ltiViewer.h>
#include <ltiPNGFunctor.h>
#include <ltiColors.h>

#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::endl;
using std::cerr;

void parseColor(const std::string& name, lti::rgbColor& c) {
  if (name.at(0) == '#') {
    c.parseHTML(name,true);
  } else {
    c.parseName(name);
  }
}

void usage() {
  cerr << "Usage: " << endl;
  cerr << "  fader [options] <imgfile>" << endl;
  cerr << endl;
  cerr << "where the options can be:" << endl;
  cerr << "  -a use alpha channel (default is not to use it)" << endl;
  cerr << "  -c preserve colors (usually, the source is converted to grayscale)" << endl;
  cerr << "  -b #RRGGBB[AA], color which takes black's role (in hex)" 
       << endl;
  cerr << "                  default value \"#000000\" " << endl;
  cerr << "  -w #RRGGBB[AA], color which takes white's role" << endl;
  cerr << "                   default value \"#ffffff\" " << endl;
  cerr << "     the AA are alpha channel values: 00 means totally\n"
       << "     transparent, ff means totally opaque.\n";
  cerr << "  -o outfile (if omitted the output file will be \"faded-<imgfile>\"\n\n";
  cerr << "The space between the option and its value is mandatory." << endl;
  cerr << "The image must be in PNG format.\n";
  cerr << "Note: Keep in mind to protect the '#' character from the shell.\n";
  
}

bool parseArgs(int argc, char*argv[], lti::rgbColor& black,
               lti::rgbColor& white, std::string& filename,
               std::string& outFile, bool& useAlpha, bool& useColors) {

  //black=lti::rgbColor(255,255,255,255);
  //white=lti::rgbColor(255,255,255,000);

  // default should be no modification
  black=lti::rgbColor(0,0,0);
  white=lti::rgbColor(255,255,255);
  useAlpha=false;
  useColors=false;
  outFile="";
  filename="";

  if (argc==1) {
    usage();
    exit(1);
  }

  for (int i=1; i<argc; i++) {
    if (*argv[i] == '-' || *argv[i] == '+') {
      //bool val=(*argv[i] == '+');
      switch (argv[i][1]) {
        // flag for enabling the use of the alpha channel
        case 'a': useAlpha=true; break;
        case 'c': useColors=true; break;
        case 'b':
          // color which takes black's role
          if (i < argc-1) {
            parseColor(argv[++i],black);
          } else {
            cerr << "Missing argument to -b; please specify an RGB(A) color\n";
            return false;
          }
          break;
        case 'w':
          // color which takes white's role
          if (i < argc-1) {
            parseColor(argv[++i],white);
          } else {
            cerr << "Missing argument to -w; please specify an RGB(A) color\n";
            return false;
          }
          break;
        case 'o':
          // output file name
          if (i < argc-1) {
            outFile=argv[++i];
          } else {
            cerr << "Missing argument to -o; please specify the output file\n";
            return false;
          }
          break;
        case 'h':
          usage();
          exit(0);
          break;
        default:
          cerr << "Unknown option " << argv[i] << ", ignoring.\n";
          break;
      }
    } else {
      // input file name, set default output name
      filename=argv[i];
      if (outFile.length() == 0) {
        outFile="faded-";
        outFile+=filename;
      }
    }
  }
  unsigned int fn=filename.length();
  if (fn == 0) {
    cerr << "Missing filename; please specify the input file.\n";
    return false;
  } else {
    if (filename.rfind(".png") != fn-4 && filename.rfind(".PNG") != fn-4) {
      cerr << "Can only work on PNG files.\n";
      return false;
    }
  }
  return true;
}


int main(int argc, char* argv[]) {

  std::string fname,oname;
  lti::rgbColor newBlack, newWhite;
  bool useAlpha,useColors;
  float baseAlpha,deltaAlpha;

  if (!parseArgs(argc,argv,newBlack,newWhite,fname,oname,useAlpha,useColors)) {
    return 1;
  }

  // define the color transform
  // newBlack is the color that will replace black
  // new White will replace white
  lti::trgbPixel<float> base;
  base.setRed(float(newBlack.getRed())/255.0);
  base.setGreen(float(newBlack.getGreen())/255.0);
  base.setBlue(float(newBlack.getBlue())/255.0);
  baseAlpha=float(newBlack.getDummy())/255.0;
  lti::trgbPixel<float> delta;
  delta.setRed(float(newWhite.getRed())/255.0);
  delta.setGreen(float(newWhite.getGreen())/255.0);
  delta.setBlue(float(newWhite.getBlue())/255.0);
  deltaAlpha=float(newWhite.getDummy())/255.0;
  delta.subtract(base);
  deltaAlpha-=baseAlpha;

  // now delta is a color which can be used for mapping the
  // intensity channel:
  // newColor:=base+delta*intensity
  // an intensity of 0 (black) will then be mapped to newblack
  // an intensity of 1 (white) will be mapped to newwhite 

  //cout << "Base = " << base << "\n";
  //cout << "Delta = " << delta << "\n";

  lti::loadPNG loader;
  lti::savePNG saver;

  //lti::mergeRGBToImage merger;

  lti::image img;
  // load image
  if (!loader.load(fname,img)) {
    std::cerr << "Problems loading image: " << loader.getStatusString() << std::endl;
    return 1;
  }

  //lti::viewer v1("Original");
  //v1.show(img);

  if (!useColors) {
    // convert source image to a channel
    lti::splitImageToHSI splitter;
    lti::channel chnl;
    // get intensity
    splitter.getIntensity(img,chnl);
    
    lti::channel::const_iterator i=chnl.begin();
    lti::image::iterator di=img.begin();
    
    // now transform image
    while (i != chnl.end()) {
      lti::trgbPixel<float> d(delta);
      float dAlpha=deltaAlpha;
      
      d      *= (*i);
      dAlpha *= (*i);
      d      += base;
      dAlpha += baseAlpha;
      
      (*di) = lti::rgbPixel(int(d.getRed()*255.0),
                            int(d.getGreen()*255.0),
                            int(d.getBlue()*255.0),
                            int(dAlpha*255.0));
      ++i;
      ++di;
    }
  } else {
    // use color image
    lti::image::iterator i=img.begin();
    
    // now transform image
    while (i != img.end()) {
      lti::rgbPixel src=*i;
      // make sure that *only* grey values are considered
      if (src.getRed() == src.getGreen() && src.getRed() == src.getBlue()) {
        float org=float(src.getRed())/255.0;
        float dAlpha=deltaAlpha;
        lti::trgbPixel<float> d(delta);
      
        d      *= org;
        dAlpha *= org;
        d      += base;
        dAlpha += baseAlpha;
        
        *i = lti::rgbPixel(int(d.getRed()*255.0),
                           int(d.getGreen()*255.0),
                           int(d.getBlue()*255.0),
                           int(dAlpha*255.0));
      } else {
        i->setDummy(255);
      }
      ++i;
    }
  }

  //lti::viewer v2("Faded");
  //v2.show(img);
  //std::getchar();

  // save new image

  if (useAlpha) {
    lti::ioPNG::parameters p=saver.getParameters();
    p.bitsPerPixel=32;
    p.useAlphaChannel=true;
    saver.setParameters(p);
  } else {
    lti::ioPNG::parameters p=saver.getParameters();
    p.bitsPerPixel=24;
    p.useAlphaChannel=false;
    saver.setParameters(p);
  }

  //std::cerr << "PaRAM\n";
  try {
    if (!saver.save(oname,img)) {
      std::cerr << "Problems saving image: " << loader.getStatusString() << std::endl;
    }
  } catch (std::exception x) {
    std::cerr << "Exception: " << x.what() << "\n";
  }
}

