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

/**
 * Set Generator for Classification Problems
 *
 * This is a little example program which takes an image with some
 * points in it, and generates a set of points used to train
 * classifiers.
 *
 * After compiling, you can execute "setgen --help" for more information.
 */


#include <iostream>
#include <string>
#include <cstdlib>

#include <ltiRGBPixel.h>
#include <ltiALLFunctor.h>
#include <ltiKMColorQuantization.h>
#include <ltiLispStreamHandler.h>
#include <ltiMultivariateGaussian.h>

using std::cout;
using std::endl;

void usage() {
  cout << "Usage: \n\n";
  cout << "  setget [-h from to] [-v from to] [-o file] [-d] image\n\n";
  cout << "      -h horizontal range.\n";
  cout << "         Values val1 and val2 must be real numbers.\n";
  cout << "         Default interval from -1 to 1\n";
  cout << "      -v vertical range.\n";
  cout << "         Values val1 and val2 must be real numbers.\n";
  cout << "         Default interval from -1 to 1\n";
  cout << "      -o output file.  If not given, stdout will be used.\n";
  cout << "      -d generate a test image file. \n";
  cout << "   image filename of the image (jpeg, bmp and png accepted)\n";
  cout << "  --help this help\n\n";
  cout << "setgen will assign one label per color to each non-black \n";
  cout << "pixel at the given image.\n";
  cout << "The coordinates of each pixel are linearly mapped to the \n";
  cout << "given interval.\n\n";
  cout << "To read the file created you can use following code: \n\n";
  cout << "  lti::dmatrix data; // the data matrix\n";
  cout << "  lti::ivector ids;  // the ids for each row of the data matrix\n";
  cout << "  std::ifstream in(\"filename\"); \n";
  cout << "  lti::lispStreamHandler lsh; \n";
  cout << "  lsh.use(in); // prepare the ioHandler to read from file\n";
  cout << "  lti::read(lsh,\"inputs\",data); // read the data matrix \n";
  cout << "  lti::read(lsh,\"ids\",ids); // read the ids \n" << endl;
}

/**
 * The setgen class can generate a test image or from a test image can
 * generate the set of points.
 */
class setgen {
public:
  /**
   * constructor
   */
  setgen() : out(&std::cout),border(-1,+1,1,-1) {
  }

  /**
   * destructor
   */
  ~setgen() {
    out = 0;
  }

  /**
   * do the job.
   * Load the image from the given filename, quantize it, and
   * generate a dmatrix with the coordinates of the "not background" points
   * and a separate ivector with the labels of each vector.
   */
  bool apply(const std::string& s) {
    lti::loadImage loader;   // functor to load different image formats
    lti::image img;          // container for loaded image
    lti::matrix<int> labels; // label assigned to each pixel in img
    lti::matrix<int>::iterator it,eit;
    lti::palette pal;        // color palette
    lti::kMColorQuantization km; // k-Means color quantization functor
    lti::dmatrix inputs;     // found data points will be stored here
    lti::ivector ids;        // ids.at(j) is id label for inputs.getRow(j)

    // load image
    if (!loader.load(s,img)) {
      cout << "Image " << s << " could not be loaded." << endl;
      return false;
    }

    // only use 256 colors.  The color label will be also used as the
    // pixel id label
    km.apply(img,labels,pal);

    // make some statistics to determine the background
    lti::ivector histo(256,0);

    for (it=labels.begin(),eit=labels.end();
         it!=eit;
         ++it) {
      histo.at(*it)++;
    }

    // let's assume that the background is the most frequent color
    const lti::ubyte background = histo.getIndexOfMaximum();

    // data points are all non-background points
    const int size = img.rows()*img.columns() - histo.at(background);

    // now we know how much rows the data matrix must have
    inputs.resize(size,2,0,false,false);
    // so the ids vector.
    ids.resize(size,0,false,false);

    // compute the linear transformation parameters
    int x,y,i;
    double mx,my;
    mx = (border.br.x - border.ul.x)/(img.columns()-1);
    my = (border.br.y - border.ul.y)/(img.rows()-1);

    // get the non-background points into the result containers
    i=0;
    for (y=0;y<labels.rows();++y) {
      for (x=0;x<labels.columns();++x) {
        if (labels.at(y,x) != background) {
          // get the point coordinates mapped into the desired interval
          inputs.at(i,0) = mx*x + border.ul.x;
          inputs.at(i,1) = my*y + border.ul.y;
          // the the point label
          ids.at(i) = (labels.at(y,x) < background) ? labels.at(y,x) :
            labels.at(y,x) - 1;
          ++i;
        }
      }
    }

    // write the list of points and labels into a lisp stream handler
    lti::lispStreamHandler lsh(*out);
    lti::write(lsh,"inputs",inputs);
    lti::write(lsh,"ids",ids);

    return true;
  }

  /**
   * set the intervall where the position of the points are going to be mapped
   */
  void setRanges(const lti::trectangle<double>& b) {
    border = b;
  }

  /**
   * set the stream where the data points are going to be written.
   */
  void setOutputStream(std::ostream& o) {
    out = &o;
  }

  /**
   * generate a test image with three classes, represented by red, blue and
   * green points.
   *
   * Two of the classes overlap (forming sort of an X) and the third is
   * arranged as circle points around the X.
   */
  void testImageXO(const std::string& filename) {
    // multivariate gaussian point generator
    lti::multivariateGaussian mvg;

    // our canvas
    lti::image img(512,512,lti::Black);

    // rotation matrix (2x2)
    lti::dmatrix trans(2,2);

    // the points
    lti::dmatrix inputs;

    // temporarily container
    lti::dmatrix tmpMat;
    int i;

    // rotate 45 deg
    trans.at(0,0)=cos(lti::Pi/4.0);
    trans.at(0,1)=sin(lti::Pi/4.0);
    trans.at(1,1)=trans.at(0,0);
    trans.at(1,0)=-trans.at(0,1);

    // covariance matrix generated from an ellipsoid oriented in the x-axis
    lti::dmatrix covar(2,2,0.0);
    covar.at(0,0) = (img.columns()/6.5);
    covar.at(1,1) = (img.rows()/35);

    // rotate
    covar.multiply(trans);
    // invert the matrix (orthogonal matrix => inversion = transposition)
    covar.transpose();

    // transpose covar and leave the result in tmpMat
    tmpMat.transpose(covar);
    covar.multiply(tmpMat);

    // mean value at the center of the image
    lti::dvector mean(2);
    mean.at(0)=img.columns()/2.0;
    mean.at(1)=img.rows()/2.0;

    // generate points from a gaussian distribution with the given covariance
    // and mean value.
    mvg.apply(mean,covar,200,inputs);

    // paint the points on the canvas using a red color.
    for (i=0;i<inputs.rows();++i) {
      lti::point p(static_cast<int>(inputs.at(i,0)),
                   static_cast<int>(inputs.at(i,1)));
      if ((p.x >=0) && (p.x < img.columns()) &&
          (p.y >=0) && (p.y < img.rows())) {
        img.at(p)=lti::rgbPixel(255,128,64);
      }
    }

    // generate the second ellipse (-45 degrees)

    // again, first the rotation matrix
    trans.at(0,0)=cos(-lti::Pi/4.0);
    trans.at(0,1)=sin(-lti::Pi/4.0);
    trans.at(1,1)=trans.at(0,0);
    trans.at(1,0)=-trans.at(0,1);

    // then compute the covariance
    covar.fill(0.0);
    covar.at(0,0) = (img.columns()/6.5);
    covar.at(1,1) = (img.rows()/35);

    covar.multiply(trans);
    covar.transpose();
    tmpMat.transpose(covar);
    covar.multiply(tmpMat);

    // and generate the points
    mvg.apply(mean,covar,200,inputs);

    // which should be painted with the color green
    for (i=0;i<inputs.rows();++i) {
      lti::point p(static_cast<int>(inputs.at(i,0)),
                   static_cast<int>(inputs.at(i,1)));
      if ((p.x >=0) && (p.x < img.columns()) &&
          (p.y >=0) && (p.y < img.rows())) {
        img.at(p)=lti::Green;
      }
    }


    // now draw blue points in the border of a circle
    double r(lti::min(img.columns(),img.rows())*0.45),a;
    lti::point m(img.columns()/2,img.rows()/2);
    for (i=0;i<200;++i) {
      a = 2.0*lti::Pi*i/200.0;
      lti::point p(lti::iround(m.x + r*cos(a)),lti::iround(m.y + r*sin(a)));
      img.at(p)=lti::rgbPixel(128,128,255);
    }

    lti::saveImage saver;
    saver.save(filename,img);
  }

protected:
  /**
   * stream used for output
   */
  std::ostream* out;

  /**
   * rectangle that contaings the values of the corners for the used image.
   * Default values: (-1,-1,1,1)
   */
  lti::trectangle<double> border;

  /**
   * image filename
   */
  std::string imageName;
};

int main(int argc,char *argv[]) {
  try {
    int i;
    std::string imgfile;
    std::string outfile;

    // default interval where the pixel coordinates will be mapped.
    lti::trectangle<double> border(-1,+1,+1,-1);

    // the object that does everything.
    setgen obj;

    i=1;
    while (i<argc) {
      if (*argv[i] == '-') {
        switch (argv[i][1]) {
          case 'd':
            // option "d" specify to draw a test image
            ++i;
            if (i<argc) {
              // get the filename
              imgfile=argv[i];
            } else {
              cout << "Syntax error: filename expected for -d" << endl;
              exit(1);
            }

            // draw the test image.
            obj.testImageXO(imgfile);
            exit(0);
            break;
          case 'h':
            // horizontal range
            ++i;
            if (i<argc) {
              border.ul.x=atof(argv[i]);
            } else {
              cout << "Syntax error: number expected for -h" << endl;
              exit(1);
            }

            ++i;
            if (i<argc) {
              border.br.x=atof(argv[i]);
            } else {
              cout << "Syntax error: second number expected for -h" << endl;
              exit(1);
            }

            break;
          case 'v':
            // vertical range
            ++i;
            if (i<argc) {
              border.ul.y=atof(argv[i]);
            } else {
              cout << "Syntax error: number expected for -v" << endl;
              exit(1);
            }

            ++i;
            if (i<argc) {
              border.br.y=atof(argv[i]);
            } else {
              cout << "Syntax error: second number expected for -v" << endl;
              exit(1);
            }

            break;
          case 'o':
            // output file
            ++i;
            if (i<argc) {
              outfile = argv[i];
            } else {
              cout << "Syntax error: filename expected for -o" << endl;
              exit(1);
            }
            break;
          default:
            // show help
            imgfile="";
        }
      } else {
        imgfile = argv[i];
      }
      ++i;
    }

    if (imgfile.empty()) {
      usage();
      exit(1);
    }

    // do the job
    obj.setRanges(border);
    std::ofstream out;
    if (!outfile.empty()) {
      out.open(outfile.c_str());
      obj.setOutputStream(out);
    }

    obj.apply(imgfile);

    if (!outfile.empty()) {
      out.close();
    }

  }
  catch (lti::exception& exp) {
    std::cout << "An lti::exception was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (std::exception& exp) {
    std::cout << "std::exception was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (...) {
    std::cout << "Unknown exception thrown!" << std::endl;
  }
  return 0;
}

