/*
 * Copyright (C) 2003, 2004, 2005, 2006
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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiview.cpp
 * authors ....: Jochen Wickel, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 14.1.2003
 * revisions ..: $Id: ltiview.cpp,v 1.6 2006/02/07 18:02:48 ltilib Exp $
 */

/**
 * \file ltiview.cpp
 *
 * This file creates as simple image file visualization tool using
 * the lti::viewer class.  The formats supported are the ones understood
 * by the lti::imageLoader functor.  At this time are png, jpeg or bmp.
 *
 * Usage ltiview [-h] [--help] filename1.ext [filename2.ext ... ]
 *
 * You can press the left button to get more information about a pixel, 
 * or the right botton to change the visualization options.
 *
 * With the Left and Right keys you can change the file being displayed.
 * With the Q or X keys you can exit the program.
 *
 * Options:
 *   -h --help       Show this description
 *   filename*.ext   At least one file is mandatory, the extension
 *                   must be png, jpg or png" << endl;
 */

#include <ltiRGBPixel.h>
#include <ltiViewer.h>
#include <ltiGtkServer.h>
#include <ltiALLFunctor.h>
#include <iostream>
#include <list>

using std::endl;
using std::cout;
using std::cerr;

/**
 * display a help text if no files are given or if an invalid option was used.
 */
void usage() {
  cout << "ltiview" << endl << endl;
  cout << "Usage ltiview [-h] [--help] filename1.ext [filename2.ext ... ]";
  cout << endl;
  cout << endl;
  cout << "ltiview is a small tool to show images in png, jpeg or bmp format";
  cout << endl;
  cout << "using the lti::viewer class of the LTI-Lib." << endl;
  cout << "You can press the left button to get more information about a";
  cout << endl;
  cout << "pixel, or the right botton to change the visualization options.";
  cout << endl;
  cout << "With the Left and Right keys you can change the file being";
  cout << endl;
  cout << "displayed." << endl;
  cout << "With the Q or X keys you can exit the program" << endl << endl;
  cout << "Options:" << endl;
  cout << "  -h --help       Show this description" << endl;
  cout << "  filename*.ext   At least one file is mandatory, the extension";
  cout << endl;
  cout << "                  must be png, jpg or png" << endl;
  cout << endl;
}

/**
 * get the command line args and store them as filenames into the vector 
 * string
 */
void parseArgs(int argc, char*argv[],std::list<std::string>& filenames) {
  filenames.clear();
  std::string par;
  for (int i=1; i<argc; i++) {
    par=argv[i];
    if (par[0]=='-' || par[0]=='+') {
      if ((par.find("--help") != std::string::npos) ||
          (par.find("-h") != std::string::npos)) {
        usage();
        exit(0);
      }
    } else {
      // input file name, set default output name
      filenames.push_back(par);
    }
  }

}

/**
 * container with all image information
 */
struct element {
  /**
   * filename of the image
   */ 
  std::string filename;
  
  /**
   * color image
   */
  lti::image img;

  /**
   * channel8 used as index or gray valued image
   */
  lti::channel8 chnl;

  /**
   * color palette used if chnl is not empty
   */
  lti::palette pal;
};

/**
 * the main function
 */
int main(int argc, char *argv[]) {

  // container of all filenames given
  std::list<std::string> filenames;

  // get the filenames from the command line
  parseArgs(argc,argv,filenames);

  // if no filename found, print help and exit
  if (filenames.empty()) {
    usage();
    return 0;
  }

  bool trueColor(false);
  element p;
  lti::point size;
  std::vector<element> images;
  lti::loadImage imgLoader;
  std::list<std::string>::const_iterator it;

  // read all files
  for (it=filenames.begin();it!=filenames.end();++it) {
    p.filename=(*it);
    p.img.clear();
    p.pal.clear();
    p.chnl.clear();

    // first, check if is a true color image or not
    if (imgLoader.checkHeader((*it),size,trueColor)) {
    
      if (trueColor) {
        // load color image
        if (imgLoader.load((*it),p.img)) {
          images.push_back(p);
        } else {
          // some error occured, let's inform the user what happend:
          std::cerr << "Loading true color image " << *it << " failed: ";
          std::cerr << imgLoader.getStatusString() << std::endl;
        }
      } else {
        // load indexed image
        if (imgLoader.load((*it),p.chnl,p.pal)) {
          images.push_back(p);
        } else {
          // some error occured, let's inform the user what happend:
          std::cerr << "Loading indexed image " << *it << " failed: ";
          std::cerr << imgLoader.getStatusString() << std::endl;
        }
      }

    } else {
      // some error occured, let's inform the user what happend:
      std::cerr << "Checking header of " << *it << " failed: ";
      std::cerr << imgLoader.getStatusString() << std::endl;
    }    

  }

  if (images.empty()) {
    std::cerr << "No image file could be opened or found." << endl;
    return 1;
  }

  lti::viewer view;
  lti::viewer::parameters viewPar;

  bool quit = false;
  int i(0);

  do {
    // get the current viewer parameters, in case the user changed them.
    const lti::viewer& cview=view;
    viewPar.copy(cview.getParameters());

    // set viewer title
    viewPar.title = images[i].filename;
    if (images[i].pal.size() != 0) {
      viewPar.colors=images[i].pal;
      viewPar.whichPalette = lti::viewer::parameters::UserDefined;
      view.setParameters(viewPar);
      // show image
      view.show(images[i].chnl);
    } else {
      view.setParameters(viewPar);
      // show image
      view.show(images[i].img);
    }

    // wait until the user press a key
    int key = view.waitKey();

    // some keys have a special function:
    switch (key) {
      // left keys
      case 65361: // left key
      case 65430: // left key in num block (4)
      case 65460: // left key in num block (with num lock)
        if (i>0) {
          --i;
        } else {
          i=images.size()-1;
        }
        break;

      // right keys
      case 65363:  // right key
      case 65432:  // right key in num block (6)
      case 65462:  // right key in num block (with num lock)
        if (i< static_cast<int>(images.size())-2) {
          ++i;
        } else {
          i=0;
        }
        break;

      // exit possibility
      case -1:
      case 'x':
      case 'X':
      case 'q':
      case 'Q':
        quit = true;
        break;

      default:
        key=0;
    }

  } while(!quit);

  view.hide();
  lti::gtkServer::shutdown();
}
