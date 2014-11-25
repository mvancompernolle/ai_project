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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLinearRegressionTracking.cpp
 * authors ....: Holger Fillbrandt
 * organization: LTI, RWTH Aachen
 * creation ...: 4.7.2003
 * revisions ..: $Id: ltiLinearRegressionTracking.cpp,v 1.10 2006/09/05 10:20:07 ltilib Exp $
 */

#include "ltiLinearRegressionTracking.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include <iostream>
// #include <conio.h> // this only exists on MS C++ 
#include <stdio.h>

using std::cout;
using std::cin;
using std::endl;
#endif

namespace lti {
  // --------------------------------------------------
  // linearRegressionTracking::parameters
  // --------------------------------------------------

  // default constructor
  linearRegressionTracking::parameters::parameters()
    : linearRegression<double>::parameters() {

    maxXDisplace = 10;
    maxYDisplace = 10;
    maxAngleDiff = 0.2f;
    maxScaleDiff = 0.1f;
    trainingSize = 100;
   
  }

  // copy constructor
  linearRegressionTracking::parameters::parameters(const parameters& other)
    : linearRegression<double>::parameters() {
    copy(other);
  }

  // destructor
  linearRegressionTracking::parameters::~parameters() {
  }

  // get type name
  const char* linearRegressionTracking::parameters::getTypeName() const {
    return "linearRegressionTracking::parameters";
  }

  // copy member

  linearRegressionTracking::parameters&
    linearRegressionTracking::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    linearRegression<double>::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    linearRegression<double>::parameters& (linearRegression<double>::parameters::* p_copy)
      (const linearRegression<double>::parameters&) =
      linearRegression<double>::parameters::copy;
    (this->*p_copy)(other);

# endif

    maxXDisplace = other.maxXDisplace; 
    maxYDisplace = other.maxYDisplace; 
    maxAngleDiff = other.maxAngleDiff;
    maxScaleDiff = other.maxScaleDiff;
    trainingSize = other.trainingSize;    

    return *this;
  }

  // alias for copy member
  linearRegressionTracking::parameters&
    linearRegressionTracking::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* linearRegressionTracking::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool linearRegressionTracking::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool linearRegressionTracking::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"maxXDisplace", maxXDisplace);
      lti::write(handler,"maxYDisplace", maxYDisplace);
      lti::write(handler,"maxAngleDiff", maxAngleDiff);
      lti::write(handler,"maxScaleDiff", maxScaleDiff);
      lti::write(handler,"trainingSize", trainingSize);        
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && linearRegression<double>::parameters::write(handler,false);
# else
    bool (linearRegression<double>::parameters::* p_writeMS)(ioHandler&,const bool) const =
      linearRegression<double>::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool linearRegressionTracking::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool linearRegressionTracking::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool linearRegressionTracking::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"maxXDisplace", maxXDisplace);
      lti::read(handler,"maxYDisplace", maxYDisplace);
      lti::read(handler,"maxAngleDiff", maxAngleDiff);
      lti::read(handler,"maxScaleDiff", maxScaleDiff);
      lti::read(handler,"trainingSize", trainingSize);      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && linearRegression<double>::parameters::read(handler,false);
# else
    bool (linearRegression<double>::parameters::* p_readMS)(ioHandler&,const bool) =
      linearRegression<double>::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool linearRegressionTracking::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // linearRegressionTracking
  // --------------------------------------------------

  // default constructor
  linearRegressionTracking::linearRegressionTracking()
    : linearRegression<double>(){

    referenceValues = vector<double>();
    width = 0;
    height = 0;

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  linearRegressionTracking::linearRegressionTracking(const parameters& par)
    : linearRegression<double>() {

    referenceValues = vector<double>();
    width = 0;
    height = 0;

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  linearRegressionTracking::linearRegressionTracking(const linearRegressionTracking& other)
    : linearRegression<double>() {
    copy(other);
  }

  // destructor
  linearRegressionTracking::~linearRegressionTracking() {
  }

  // returns the name of this type
  const char* linearRegressionTracking::getTypeName() const {
    return "linearRegressionTracking";
  }

  // copy member
  linearRegressionTracking&
    linearRegressionTracking::copy(const linearRegressionTracking& other) {
      linearRegression<double>::copy(other);

    referenceValues = other.referenceValues;
    width = other.width;
    height = other.height;

    return (*this);
  }

  // alias for copy member
  linearRegressionTracking&
    linearRegressionTracking::operator=(const linearRegressionTracking& other) {
    return (copy(other));
  }


  // clone member
  functor* linearRegressionTracking::clone() const {
    return new linearRegressionTracking(*this);
  }

  // return parameters
  const linearRegressionTracking::parameters&
    linearRegressionTracking::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  /*
   * training of x- and y-movement in the image plane using channel values
   * @param referenceImage reference channel to be used for training
   * @param imageSection rectangle that surrounds the object in the given image
   */
  bool linearRegressionTracking::learnDisplacement(channel& referenceImage, rectangle imageSection) {
    parameters pars;
	  pars = getParameters();
  
    // check, if displaced image section is located inside image borders
    if ((imageSection.ul.x - pars.maxXDisplace < 0)||(imageSection.ul.y - pars.maxYDisplace < 0)||(imageSection.br.x + pars.maxXDisplace > referenceImage.size().x)||(imageSection.br.y + pars.maxYDisplace > referenceImage.size().y)) {
		  return false;
    }

    // initialize variables
	  width = imageSection.br.x - imageSection.ul.x + 1;
	  height = imageSection.br.y - imageSection.ul.y + 1;
	  int sectionSize = width * height;
	  int i=0,index=0;
	  int x,y,dx,dy;

    // read reference values from image
	  referenceValues.resize(sectionSize);
	  index=0;
	  for (y=imageSection.ul.y; y <= imageSection.br.y; y++) {
		  for (x=imageSection.ul.x; x <= imageSection.br.x; x++) {
		    referenceValues.at(index) = referenceImage.at(y,x);
		    index++;
		  }
	  }


    // matrices to keep the training data for calculating the linear regression matrix
	  matrix<double> imageDiffMatrix(sectionSize, pars.trainingSize, 0.0); // columns: image differences
	  matrix<double> displaceMatrix(2, pars.trainingSize, 0.0);            // columns: x- and y-displacements

    // collect training data: displace reference image region and calculate the difference
    for (i=0; i<pars.trainingSize; i++) {
      dx = int(float(rand())  * 2.0f * float(pars.maxXDisplace) / float(RAND_MAX) - float(pars.maxXDisplace));
      dy = int(float(rand()) * 2.0f * float(pars.maxYDisplace) / float(RAND_MAX) - float(pars.maxYDisplace));

      displaceMatrix.at(0,i) = dx;
      displaceMatrix.at(1,i) = dy;

      index = 0;
		  for (y=imageSection.ul.y - dy; y <= imageSection.br.y - dy; y++) {
			  for (x=imageSection.ul.x - dx; x <= imageSection.br.x - dx; x++) {
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x);
          index++;          
        }
      }
    }

    // calculate the linear regression matrix
	  apply(imageDiffMatrix, displaceMatrix);
      
	  return true;
  }

  /*
   * estimates the object displacement from the difference of current and reference
   * channel values
   * @param theImage current frame channel
   * @param xpos current x-position of left upper edge of object
   * @param ypos current y-position of left upper edge of object
   * @param dx estimated displacement in x-direction
   * @param dy estimated displacement in y-direction
   */
  void linearRegressionTracking::getDisplacement(channel& theImage, int xpos, int ypos, double &dx, double &dy) const {
	  int x,y, index;
    int xmax = theImage.size().x-1;
    int ymax = theImage.size().y-1;
	  vector<double> imageDiff(width*height, 0.0);
    vector<double> displacement(2, 0.0);

    index = 0;
	  for (y=ypos; y < ypos+height; y++) {
	    for (x=xpos; x < xpos+width; x++) {
        if ((x<0)||(x>xmax)||(y<0)||(y>ymax)) {
          imageDiff.at(index) = 0;
        } else {
  			  imageDiff.at(index) = referenceValues.at(index) - theImage.at(y,x);
        }
			  index++;
		  }
	  }
  
	  transform(imageDiff, displacement);
    dx = displacement.at(0);
    dy = displacement.at(1);
  }

  /*
   * training of x- and y-movement in the image plane using RGB values
   * @param referenceImage reference image to be used for training
   * @param imageSection rectangle that surrounds the object in the given image
   */
  bool linearRegressionTracking::learnDisplacementRGB(image& referenceImage, rectangle imageSection) {
    parameters pars;
	  pars = getParameters();
  
    // check, if displaced image section is located inside image borders
    if ((imageSection.ul.x - pars.maxXDisplace < 0)||(imageSection.ul.y - pars.maxYDisplace < 0)||(imageSection.br.x + pars.maxXDisplace > referenceImage.size().x)||(imageSection.br.y + pars.maxYDisplace > referenceImage.size().y)) {
		  return false;
    }

    // initialize variables
	  width = imageSection.br.x - imageSection.ul.x + 1;
	  height = imageSection.br.y - imageSection.ul.y + 1;
	  int sectionSize = width * height;
	  int i=0,index=0;
	  int x,y, dx,dy;

    // read reference values from image
	  referenceValues.resize(3*sectionSize);
	  index=0;
	  for (y=imageSection.ul.y; y <= imageSection.br.y; y++) {
		  for (x=imageSection.ul.x; x <= imageSection.br.x; x++) {
		    referenceValues.at(index) = referenceImage.at(y,x).getRed();
		    index++;
		    referenceValues.at(index) = referenceImage.at(y,x).getGreen();
		    index++;
		    referenceValues.at(index) = referenceImage.at(y,x).getBlue();
		    index++;

		  }
	  }

    // matrices to keep the training data for calculating the linear regression matrix
	  matrix<double> imageDiffMatrix(3*sectionSize, pars.trainingSize, 0.0); // columns: image differences
	  matrix<double> displaceMatrix(2, pars.trainingSize, 0.0);              // columns: x- and y-displacements

    // collect training data: displace reference image region and calculate the difference
    for (i=0; i<pars.trainingSize; i++) {
      dx = int(float(rand())  * 2.0f * float(pars.maxXDisplace) / float(RAND_MAX) - float(pars.maxXDisplace));
      dy = int(float(rand()) * 2.0f * float(pars.maxYDisplace) / float(RAND_MAX) - float(pars.maxYDisplace));

      displaceMatrix.at(0,i) = dx;
      displaceMatrix.at(1,i) = dy;

      index = 0;
		  for (y=imageSection.ul.y - dy; y <= imageSection.br.y - dy; y++) {
			  for (x=imageSection.ul.x - dx; x <= imageSection.br.x - dx; x++) {
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getRed();
          index++;          
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getGreen();
          index++;          
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getBlue();
          index++;          

        }
      }
    }

    // calculate the linear regression matrix
	  apply(imageDiffMatrix, displaceMatrix);
      
	  return true;
  }

  /*
   * estimates the object displacement from the difference of current and reference
   * RGB values
   * @param theImage current frame channel
   * @param xpos current x-position of left upper edge of object
   * @param ypos current y-position of left upper edge of object
   * @param dx estimated displacement in x-direction
   * @param dy estimated displacement in y-direction
   */
  void linearRegressionTracking::getDisplacementRGB(image& theImage, int xpos, int ypos, double &dx, double &dy) const {
	  int x,y, index;
    int xmax = theImage.size().x-1;
    int ymax = theImage.size().y-1;
	  vector<double> imageDiff(3*width*height, 0.0);
    vector<double> displacement(2, 0.0);

    index = 0;
	  for (y=ypos; y < ypos+height; y++) {
	    for (x=xpos; x < xpos+width; x++) {
        if ((x<0)||(x>xmax)||(y<0)||(y>ymax)) {
          imageDiff.at(index) = 0;
          index++;
          imageDiff.at(index) = 0;
          index++;
          imageDiff.at(index) = 0;
          index++;

        } else {
  			  imageDiff.at(index) = referenceValues.at(index) - theImage.at(y,x).getRed();
          index++;
  			  imageDiff.at(index) = referenceValues.at(index) - theImage.at(y,x).getGreen();
          index++;
  			  imageDiff.at(index) = referenceValues.at(index) - theImage.at(y,x).getBlue();
          index++;

        }
		  }
	  }
  
	  transform(imageDiff, displacement);
    dx = displacement.at(0);
    dy = displacement.at(1);
  }

  /*
   * training of x- and y-movement as well as scaling and rotation in the image plane 
   * using channel values
   * @param referenceImage reference channel to be used for training
   * @param imageSection rectangle that surrounds the object in the given image
   */
  bool linearRegressionTracking::learnTracking(channel& referenceImage, rectangle imageSection) {
    parameters pars;
	  pars = getParameters();

	  width = imageSection.br.x - imageSection.ul.x + 1;
	  height = imageSection.br.y - imageSection.ul.y + 1;

    // check, if displaced image section is located inside image borders
    float r = sqrt(float(width * width + height * height)) * 0.5f; // radius of image section
    float xMiddle = float(imageSection.ul.x + imageSection.br.x) * 0.5f;
    float yMiddle = float(imageSection.ul.y + imageSection.br.y) * 0.5f;
    if ((xMiddle - pars.maxXDisplace - r*(1.0f + pars.maxScaleDiff) < 0)||(yMiddle - pars.maxYDisplace - r*(1.0f + pars.maxScaleDiff) < 0)||(xMiddle + pars.maxXDisplace + r*(1.0f + pars.maxScaleDiff) > referenceImage.size().x)||(yMiddle + pars.maxYDisplace + r*(1.0f + pars.maxScaleDiff) > referenceImage.size().y)) {
		  return false;
    }
    
    
    float xRad = xMiddle - float(imageSection.ul.x);
    float yRad = yMiddle - float(imageSection.ul.y);
    int sectionSize = width * height;
    int index;
    int x, y, i;
    
    // read reference values from image
    referenceValues.resize(sectionSize);
    index=0;
    for (y=imageSection.ul.y; y <= imageSection.br.y; y++) {
      for (x=imageSection.ul.x; x <= imageSection.br.x; x++) {
        referenceValues.at(index) = referenceImage.at(y,x);
        index++;
      }
    }
    
    matrix<double> imageDiffMatrix(sectionSize, pars.trainingSize, 0.0);
    matrix<double> displaceMatrix(2, pars.trainingSize, 0.0);
    
    float sinAlpha, cosAlpha;
    float dx, dy, dAlpha, dScale;
    float scale;
    float xRel, yRel;
    
    // learn scaling and rotation
    for (i=0; i<pars.trainingSize; i++) {
      dAlpha = float(rand()) * 2.0f * float(pars.maxAngleDiff) / float(RAND_MAX) - float(pars.maxAngleDiff);
      dScale = float(rand()) * 2.0f * float(pars.maxScaleDiff) / float(RAND_MAX) - float(pars.maxScaleDiff);
      
      displaceMatrix.at(0,i) = dAlpha;
      displaceMatrix.at(1,i) = dScale;
      
      sincos(-dAlpha, sinAlpha, cosAlpha);
      scale = 1.0f - dScale;
      
      index = 0;
      for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
        for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
          x = int(0.5f + xMiddle + scale*(xRel*cosAlpha - yRel*sinAlpha));
          y = int(0.5f + yMiddle + scale*(yRel*cosAlpha + xRel*sinAlpha));
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x);
          index++;         
        }
      }
    }
    
    apply(imageDiffMatrix, displaceMatrix);

    matrix<double> tempMatrix(linRegMatrix);

    // learn displacement
    for (i=0; i<pars.trainingSize; i++) {
      dx = float(rand())  * 2.0f * float(pars.maxXDisplace) / float(RAND_MAX) - float(pars.maxXDisplace);
      dy = float(rand()) * 2.0f * float(pars.maxYDisplace) / float(RAND_MAX) - float(pars.maxYDisplace);

      displaceMatrix.at(0,i) = dx;
      displaceMatrix.at(1,i) = dy;

      index = 0;
      for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
        for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
          x = int(0.5f + xMiddle - dx + xRel);
          y = int(0.5f + yMiddle - dy + yRel);
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x);
          index++;          
        }
      }
    }

  	apply(imageDiffMatrix, displaceMatrix);

    // combine both regression matrices
    linRegMatrix.resize(4, sectionSize, 0.0, true, true);
    linRegMatrix.setRow(2, tempMatrix.getRowCopy(0));
    linRegMatrix.setRow(3, tempMatrix.getRowCopy(1));

    return true;
  }

  /*
   * estimates the object displacement, scaling and rotation from the difference of 
   * current and reference channel values. Use more than one iteration per frame
   * for best results.
   * @param theImage current frame channel
   * @param xpos current x-position of object center
   * @param ypos current y-position of object center
   * @param alpha current object angle
   * @param scale current object scale
   * @param dx estimated displacement in x-direction
   * @param dy estimated displacement in y-direction
   * @param dAlpha estimated angle difference
   * @param dScale estimated scale difference
   */
  void linearRegressionTracking::getDispRotScale(channel& theImage, double xpos, double ypos, double alpha, double scale, double &dx, double &dy, double &dAlpha, double &dScale) const {

	  int x,y, index;
    int xmax = theImage.size().x-1;
    int ymax = theImage.size().y-1;
    vector<double> imageDiff(width*height, 0.0);
    vector<double> displacement(4, 0.0);

    float xRad = float(width-1) * 0.5f;
    float yRad = float(height-1) * 0.5f;
    float xRel, yRel;
    double sina, cosa;
    sincos(alpha, sina, cosa);
    float sinAlpha = static_cast<float>(sina);
    float cosAlpha = static_cast<float>(cosa);

    index = 0;
    for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
      for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
        x = int(0.5f + xpos + scale*(xRel*cosAlpha - yRel*sinAlpha));
        y = int(0.5f + ypos + scale*(yRel*cosAlpha + xRel*sinAlpha));
        if ((x<0)||(x>xmax)||(y<0)||(y>ymax)) {
          imageDiff.at(index) = 0;
        } else {
          imageDiff.at(index) = referenceValues.at(index) - theImage.at(y, x);
        }
        index++;

      }
    }

	  transform(imageDiff, displacement);

    dx = scale * (displacement.at(0) * cosAlpha - displacement.at(1) * sinAlpha);
    dy = scale * (displacement.at(1) * cosAlpha + displacement.at(0) * sinAlpha);
    if (displacement.size() == 4)
    {
      dAlpha = displacement.at(2);
      dScale = displacement.at(3);
    } else {
      dAlpha = 0;
      dScale = 0;
    }

  }

  /*
   * training of x- and y-movement as well as scaling and rotation in the image plane 
   * using RGB values
   * @param referenceImage reference channel to be used for training
   * @param imageSection rectangle that surrounds the object in the given image
   */
  bool linearRegressionTracking::learnTrackingRGB(image& referenceImage, rectangle imageSection) {
    parameters pars;
	  pars = getParameters();

	  width = imageSection.br.x - imageSection.ul.x + 1;
	  height = imageSection.br.y - imageSection.ul.y + 1;

    // check, if displaced image section is located inside image borders
    float r = sqrt(float(width * width + height * height)) * 0.5f; // radius of image section
    float xMiddle = float(imageSection.ul.x + imageSection.br.x) * 0.5f;
    float yMiddle = float(imageSection.ul.y + imageSection.br.y) * 0.5f;
    if ((xMiddle - pars.maxXDisplace - r*(1.0f + pars.maxScaleDiff) < 0)
        ||(yMiddle - pars.maxYDisplace - r*(1.0f + pars.maxScaleDiff) < 0)
        ||(xMiddle + pars.maxXDisplace + r*(1.0f + pars.maxScaleDiff) > referenceImage.size().x)
        ||(yMiddle + pars.maxYDisplace + r*(1.0f + pars.maxScaleDiff) > referenceImage.size().y)) {
      return false;
    }


    float xRad = xMiddle - float(imageSection.ul.x);
    float yRad = yMiddle - float(imageSection.ul.y);
    int sectionSize = width * height;
    int index;
    int x, y, i;
    
    // read reference values from image
    referenceValues.resize(3*sectionSize);
    index=0;
    for (y=imageSection.ul.y; y <= imageSection.br.y; y++) {
      for (x=imageSection.ul.x; x <= imageSection.br.x; x++) {
        referenceValues.at(index) = referenceImage.at(y,x).getRed();
        index++;
        referenceValues.at(index) = referenceImage.at(y,x).getGreen();
        index++;
        referenceValues.at(index) = referenceImage.at(y,x).getBlue();
        index++;
      }
    }
    
    matrix<double> imageDiffMatrix(3*sectionSize, pars.trainingSize, 0.0);
    matrix<double> displaceMatrix(2, pars.trainingSize, 0.0);
    
    float sinAlpha, cosAlpha;
    float dx, dy, dAlpha, dScale;
    float scale;
    float xRel, yRel;
    
    // learn scaling and rotation
    for (i=0; i<pars.trainingSize; i++) {
      dAlpha = float(rand()) * 2.0f * float(pars.maxAngleDiff) / float(RAND_MAX) - float(pars.maxAngleDiff);
      dScale = float(rand()) * 2.0f * float(pars.maxScaleDiff) / float(RAND_MAX) - float(pars.maxScaleDiff);

      displaceMatrix.at(0,i) = dAlpha;
      displaceMatrix.at(1,i) = dScale;

      sincos(-dAlpha, sinAlpha, cosAlpha);
      scale = 1.0f - dScale;

      index = 0;
      for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
        for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
          x = int(0.5f + xMiddle + scale*(xRel*cosAlpha - yRel*sinAlpha));
          y = int(0.5f + yMiddle + scale*(yRel*cosAlpha + xRel*sinAlpha));
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getRed();
          index++;
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getGreen();
          index++;
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getBlue();
          index++;

        }
      }
    }

    apply(imageDiffMatrix, displaceMatrix);
    matrix<double> tempMatrix(linRegMatrix);

    // learn displacement
    for (i=0; i<pars.trainingSize; i++) {
      dx = float(rand())  * 2.0f * float(pars.maxXDisplace) / float(RAND_MAX) - float(pars.maxXDisplace);
      dy = float(rand()) * 2.0f * float(pars.maxYDisplace) / float(RAND_MAX) - float(pars.maxYDisplace);

      displaceMatrix.at(0,i) = dx;
      displaceMatrix.at(1,i) = dy;

      index = 0;
      for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
        for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
          x = int(0.5f + xMiddle - dx + xRel);
          y = int(0.5f + yMiddle - dy + yRel);
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getRed();
          index++;
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getGreen();
          index++;
          imageDiffMatrix.at(index, i) = referenceValues.at(index) - referenceImage.at(y, x).getBlue();
          index++;

        }
      }
    }

  	apply(imageDiffMatrix, displaceMatrix);

    // combine both regression matrices
    linRegMatrix.resize(4, 3*sectionSize, 0.0, true, true);
    linRegMatrix.setRow(2, tempMatrix.getRowCopy(0));
    linRegMatrix.setRow(3, tempMatrix.getRowCopy(1));

    return true;
  }

  /*
   * estimates the object displacement, scaling and rotation from the difference of 
   * current and reference RGB values. Use more than one iteration per frame
   * for best results.
   * @param theImage current frame image
   * @param xpos current x-position of object center
   * @param ypos current y-position of object center
   * @param alpha current object angle
   * @param scale current object scale
   * @param dx estimated displacement in x-direction
   * @param dy estimated displacement in y-direction
   * @param dAlpha estimated angle difference
   * @param dScale estimated scale difference
   */
  void linearRegressionTracking::getDispRotScaleRGB(image& theImage, double xpos, double ypos, double alpha, double scale, double &dx, double &dy, double &dAlpha, double &dScale) const {
	  int x,y, index;
    int xmax = theImage.size().x-1;
    int ymax = theImage.size().y-1;
	  vector<double> imageDiff(3*width*height, 0.0);
    vector<double> displacement(4, 0.0);

    float xRad = float(width-1) * 0.5f;
    float yRad = float(height-1) * 0.5f;
    float xRel, yRel;
    double sina, cosa;
    sincos(alpha, sina, cosa);
    const float sinAlpha = static_cast<float>(sina);
    const float cosAlpha = static_cast<float>(cosa);

    index = 0;
    for (yRel = -yRad; yRel <= +yRad; yRel += 1.0f) {
      for (xRel = -xRad; xRel <= +xRad; xRel += 1.0f) {
        x = int(0.5f + xpos + scale*(xRel*cosAlpha - yRel*sinAlpha));
        y = int(0.5f + ypos + scale*(yRel*cosAlpha + xRel*sinAlpha));
        if ((x<0)||(x>xmax)||(y<0)||(y>ymax)) {
          imageDiff.at(index) = 0;
          index++;
          imageDiff.at(index) = 0;
          index++;
          imageDiff.at(index) = 0;
          index++;

        } else {
          imageDiff.at(index) = referenceValues.at(index) - theImage.at(y, x).getRed();
          index++;
          imageDiff.at(index) = referenceValues.at(index) - theImage.at(y, x).getGreen();
          index++;
          imageDiff.at(index) = referenceValues.at(index) - theImage.at(y, x).getBlue();
          index++;

        }

      }
    }

	  transform(imageDiff, displacement);

    dx = scale * (displacement.at(0) * cosAlpha - displacement.at(1) * sinAlpha);
    dy = scale * (displacement.at(1) * cosAlpha + displacement.at(0) * sinAlpha);
    if (displacement.size() == 4)
    {
      dAlpha = displacement.at(2);
      dScale = displacement.at(3);
    } else {
      dAlpha = 0;
      dScale = 0;
    }
  }


    /*
     * reads this functor from the given handler.
     */
    bool linearRegressionTracking::read(ioHandler &handler, const bool complete) {
      
      bool b=true;

      if (complete) {
        b=handler.readBegin();
      }


      if (b) {

        lti::read(handler,"linRegMatrix",linRegMatrix); 
        lti::read(handler, "referenceValues", referenceValues);
        lti::read(handler, "width", width);
        lti::read(handler, "height", height);
        parameters param;
        lti::read(handler,"parameters",param);
        setParameters(param);
        
      }

      return b;
    }

    /*
     * writes this functor to the given handler.
     */
    bool linearRegressionTracking::write(ioHandler &handler, const bool complete) const {

      bool b=true;

      if (complete) {
        b=handler.writeBegin();
      }

      if (b) {
        lti::write(handler,"linRegMatrix", linRegMatrix); 
        lti::write(handler, "referenceValues", referenceValues);
        lti::write(handler, "width", width);
        lti::write(handler, "height", height);
        lti::write(handler,"parameters",getParameters());

        if (complete) {
          b=b && handler.writeEnd();
        }
      }
      return b;
    }

    /*
     * gets the values of the reference image section used for calculating the error vector
     * @param referenceSection contains either the grey-scale or RGB-values in a vector
     * @param w width of image section
     * @param h height of image section
     */
    void linearRegressionTracking::getReferenceSection(vector<double> &referenceSection, int &w, int &h) const {

      referenceSection.copy(referenceValues);
      w = width;
      h = height;
    }

    /*
     * sets the values of the reference image section used for calculating the error vector
     * @param referenceSection contains either the grey-scale or RGB-values in a vector
     * @param w width of image section
     * @param h height of image section
     */
    void linearRegressionTracking::setReferenceSection(vector<double> &referenceSection, int w, int h) {
      referenceValues.copy(referenceSection);
      width = w;
      height = h;
    }


}
