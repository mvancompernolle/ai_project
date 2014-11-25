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

#include "ltiRGBPixel.h"
#include "ltiObject.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <list>
#include <cstdio>

#include "ltiTester.h"
#include "ltiBMPFunctor.h"
#include "ltiViewer.h"
#include "ltiHistogramViewer.h"
#include "ltiGtkServer.h"

using std::cout;
using std::endl;


namespace lti {

  void tester::operator()(int argc,char *argv[]) {

#ifdef HAVE_GTK

    std::list<std::string> files;
    std::list<std::string>::const_iterator it;

    loadBMP loader;
    image img;

    viewer view("Original");
    histogramViewer hview("Histogram");

#ifdef _MSC_VER
    // read some files 
    files.push_back("../../img/testImg.bmp");
#else
    // read some files 
    files.push_back("../img/testImg.bmp");
#endif

    // for all images
    for (it=files.begin();it!=files.end();++it) {
      cout << "Showing " << *it << endl;
      loader.load(*it,img);
      
      view.show(img);
      hview.show(img);

      cout << "Press Enter to continue" << endl;
      getchar();
    }

    // Workaround to avoid "unexpected async reply" error 

    // first, hide/destroy all active viewers
    view.hide();
    hview.hide();

    // second, end GTK main event loop handling
    lti::gtkServer::shutdown();
#else

    cout << "Hello World without GTK :-(" << endl;
    cout << "This default tester program assumed you "
         << "had GTK- installed." << endl;
    cout << "GTK was not found." << endl;
      
#endif

    
  }
};
