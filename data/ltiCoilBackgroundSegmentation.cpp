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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiCoilBackgroundSegmentation.cpp
 * authors ....: Helmuth Euler
 * organization: LTI, RWTH Aachen
 * creation ...: 29.4.2003
 * revisions ..: $Id: ltiCoilBackgroundSegmentation.cpp,v 1.13 2006/09/05 10:05:46 ltilib Exp $
 */

#include "ltiCoilBackgroundSegmentation.h"
#include "ltiProbabilityMap.h"
#include "ltiColorModelEstimator.h"
#include "ltiImage.h"
#include "ltiDraw.h"
#include "ltiKMColorQuantization.h"
#include "ltiColorQuantization.h"
#include "ltiKNearestNeighFilter.h"
#include "ltiMatrixInversion.h"
#include "ltiObjectsFromMask.h"
#include "ltiSplitImageToCIELuv.h"
#include "ltiConstants.h"


#undef _LTI_DEBUG
// #define _LTI_DEBUG 4
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // coilBackgroundSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  coilBackgroundSegmentation::parameters::parameters()
    : segmentation::parameters() {

    iterations = 10;
    maxNumberBestOfN = 100;
    maskFactor = 1;
    thresholdFactor = 1.30;
    meanBackgroundColor = 0;
    covarMatrixBackground.resize(3,3,0,false,true);
    covarMatrixBackground[0][0] = 100;
    covarMatrixBackground[1][1] = 100;
    covarMatrixBackground[2][2] = 100;
  }

  // copy constructor
  coilBackgroundSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  coilBackgroundSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* coilBackgroundSegmentation::parameters::getTypeName() const {
    return "coilBackgroundSegmentation::parameters";
  }

  // copy member

  coilBackgroundSegmentation::parameters&
    coilBackgroundSegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif


    iterations = other.iterations;
    maxNumberBestOfN = other.maxNumberBestOfN;
    maskFactor = other.maskFactor;
    thresholdFactor = other.thresholdFactor;
    meanBackgroundColor = other.meanBackgroundColor;
    covarMatrixBackground = other.covarMatrixBackground;

    return *this;
  }

  // alias for copy member
  coilBackgroundSegmentation::parameters&
    coilBackgroundSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* coilBackgroundSegmentation::parameters::clone() const {
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
  bool coilBackgroundSegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool coilBackgroundSegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"iterations",iterations);
      lti::write(handler,"maxNumberBestOfN",maxNumberBestOfN);
      lti::write(handler,"maskFactor",maskFactor);
      lti::write(handler,"thresholdFactor",thresholdFactor);
      lti::write(handler,"meanBackgroundColor",meanBackgroundColor);
      lti::write(handler,"covarMatrixBackground",covarMatrixBackground);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool coilBackgroundSegmentation::parameters::write(ioHandler& handler,
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
  bool coilBackgroundSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool coilBackgroundSegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"iterations",iterations);
      lti::read(handler,"maxNumberBestOfN",maxNumberBestOfN);
      lti::read(handler,"maskFactor",maskFactor);
      lti::read(handler,"thresholdFactor",thresholdFactor);
      lti::read(handler,"meanBackgroundColor",meanBackgroundColor);
      lti::read(handler,"covarMatrixBackground",covarMatrixBackground);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool coilBackgroundSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // coilBackgroundSegmentation
  // --------------------------------------------------

  // default constructor
  coilBackgroundSegmentation::coilBackgroundSegmentation()
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  coilBackgroundSegmentation::coilBackgroundSegmentation(const coilBackgroundSegmentation& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  coilBackgroundSegmentation::~coilBackgroundSegmentation() {
  }

  // returns the name of this type
  const char* coilBackgroundSegmentation::getTypeName() const {
    return "coilBackgroundSegmentation";
  }

  // copy member
  coilBackgroundSegmentation&
    coilBackgroundSegmentation::copy(const coilBackgroundSegmentation& other) {
      segmentation::copy(other);
    return (*this);
  }

  // alias for copy member
  coilBackgroundSegmentation&
    coilBackgroundSegmentation::operator=(const coilBackgroundSegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* coilBackgroundSegmentation::clone() const {
    return new coilBackgroundSegmentation(*this);
  }

  // return parameters
  const coilBackgroundSegmentation::parameters&
    coilBackgroundSegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  /**
   * delete such positions in the mask
   *
   *    ** **        *****
   *    ** **        *****
   *    **x**          x
   *    ** **        *****
   *    ** **        *****
   *
   * @param ch8 the channel8 to be optimized
   */
  void coilBackgroundSegmentation::deleteWeakLinks(channel8& ch8) const{

    int counterRow, counterColumn;
    for (counterRow=1; counterRow<ch8.rows()-1; ++counterRow) {
      for (counterColumn=1; counterColumn<ch8.columns()-1; ++counterColumn) {
        if (ch8.at(counterRow,counterColumn) == 255) {
          if (((ch8.at(counterRow-1,counterColumn) == 0)
            &&(ch8.at(counterRow+1,counterColumn) == 0)
            &&(ch8.at(counterRow,counterColumn-1) == 255)
            &&(ch8.at(counterRow,counterColumn+1) == 255))
          ||((ch8.at(counterRow-1,counterColumn) == 255)
            &&(ch8.at(counterRow+1,counterColumn) == 255)
            &&(ch8.at(counterRow,counterColumn-1) == 0)
            &&(ch8.at(counterRow,counterColumn+1) == 0))) {

            ch8.at(counterRow,counterColumn) = 0;
          }
        }
      }
    }
  }

  /**
   * calculating the inverted covariance matrix under consideration of 
   * the object/background mask
   * @param img the image to be considered
   * @param mask the mask which pixels from img should be used
   * @param mean the meanvalue (center) of the covarianve matrix is returned here
   * @param covarMatrix the resulting matrix is returned here
   * @return true if succesfull
   */
  bool coilBackgroundSegmentation::getInvertedCovarMatrixForMask(
                                                       const image& img,
                                                       channel8& mask,
                                                       trgbPixel<double>& mean,
                                                       dmatrix& covarMatrix
                                                       ) const {

    bool failureCheck = true;
    colorModelEstimator estimator;
    splitImageToCIELuv split;
    channel chL, chU, chV;

    split.apply(img, chL, chU, chV);

    colorModelEstimator::parameters estParam;
    estParam.histogramDimensions.setRed(1);
    estParam.histogramDimensions.setGreen(1);
    estParam.histogramDimensions.setBlue(1);
    estimator.setParameters(estParam);

    estimator.consider(chL, chU, chV, mask);
    failureCheck &= estimator.apply(mean, covarMatrix);

    matrixInversion<double> inverter;
    failureCheck &= inverter.apply(covarMatrix);

    return failureCheck;
  }

  /**
   * checking if a given color is close enough to the covariance matrix
   * @param mean the center of the covarianve matrix is considered here
   * @param covarMatrix the covariance matrix to be considered
   * @param pixel pixel with the color to be checked
   * @return true if succesfull
   */
  bool 
  coilBackgroundSegmentation::checkColorReasonable(
                                                 const trgbPixel<double>& mean,
                                                 const dmatrix& covarMatrix,
                                                 const rgbPixel& pixel
                                                  ) const {

    splitImageToCIELuv split;
    float fRed, fGreen, fBlue;
    split.apply(pixel, fRed, fGreen, fBlue);
    trgbPixel<double> tPixel;
    tPixel.red = fRed;
    tPixel.blue = fBlue;
    tPixel.green = fGreen;

    trgbPixel<double> XMinusM = tPixel-mean;

    double tmp[3];
    tmp[0] = XMinusM[0]*covarMatrix[0][0]+XMinusM[1]*covarMatrix[0][1]+XMinusM[2]*covarMatrix[0][2];
    tmp[1] = XMinusM[0]*covarMatrix[1][0]+XMinusM[1]*covarMatrix[1][1]+XMinusM[2]*covarMatrix[1][2];
    tmp[2] = XMinusM[0]*covarMatrix[2][0]+XMinusM[1]*covarMatrix[2][1]+XMinusM[2]*covarMatrix[2][2];

    double mahalanobisDistance = tmp[0]*XMinusM[0]+tmp[1]*XMinusM[1]+tmp[2]*XMinusM[2];

    // return false if the distance pixel-color to mean-color is too long
    return (mahalanobisDistance < 1);
  }

  /**
   * building the background mask under consideration of neighbourhoods, covariance
   * matrix and probabilities
   * @param img the image to be considered
   * @param ch8MaskChannel the mask estimating the background
   * @param intSparseMat matix containing which regions are neighbours
   * @param meanProbabilityList a list containing mean probability, color and
   *        relation towards background and object for each region
   * @param newLabelsMatrix matrix containing the labelnumber for each pixel
   * @param converged true if no change took place
   * @return true if succesfull
   */
  bool coilBackgroundSegmentation::growOutsideObject(
                            const image& img,
                            channel8& ch8MaskChannel,
                            sparseMatrix<int>& intSparseMat,
                            std::vector<TmeanProbability>& meanProbabilityList,
                            matrix<int>& newLabelsMatrix,
                            bool& converged
                            ) const {

    const float fThreshold = 0.25f;
    bool failureCheck = true;
    dmatrix covarMatrix;
    trgbPixel<double> mean, meanBackground;

    //utilizing the given backgroundcolor
    meanBackground.red = getParameters().meanBackgroundColor;
    meanBackground.green = getParameters().meanBackgroundColor;
    meanBackground.blue = getParameters().meanBackgroundColor;

    failureCheck &= getInvertedCovarMatrixForMask(img, ch8MaskChannel, mean, covarMatrix);

    int counterColorRow, counterColorColumn;
    for (counterColorRow=0; counterColorRow<intSparseMat.rows(); ++counterColorRow) {
      if (meanProbabilityList[counterColorRow].labelToBackgroundVector == true) { //check if region belongs to the background
        // search the upper right of intSparseMat
        for (counterColorColumn=counterColorRow; counterColorColumn<intSparseMat.columns(); ++counterColorColumn) {
          if (intSparseMat.getAt(counterColorRow,counterColorColumn) == 1) { //if regions are neighbours
            if (((checkColorReasonable(mean, covarMatrix, meanProbabilityList[counterColorColumn].meanColor)) // the color is sensible for the background
                  ||((getParameters().meanBackgroundColor >= 0)
                     &&(getParameters().meanBackgroundColor<256)
                     &&(checkColorReasonable(meanBackground, getParameters().covarMatrixBackground, meanProbabilityList[counterColorColumn].meanColor)))
              ||(meanProbabilityList[counterColorColumn].meanProbability < fThreshold))) {  // and the probability is high enough => region is considered to belog to the background
              meanProbabilityList[counterColorColumn].labelToBackgroundVector = true;
              converged = false;
            }
            else {  //otherwise the region does not belong to the background
              meanProbabilityList[counterColorColumn].labelToBackgroundVector = false;
              converged = false;
            };
          };
        };
      };
    };

    int counterMaskRow, counterMaskColumn;
    for (counterMaskRow=0; counterMaskRow<ch8MaskChannel.rows(); ++counterMaskRow) {
      for (counterMaskColumn=0; counterMaskColumn<ch8MaskChannel.columns(); ++counterMaskColumn) {
        if (meanProbabilityList[newLabelsMatrix.at(counterMaskRow,counterMaskColumn)].labelToBackgroundVector == true) {
          ch8MaskChannel.at(counterMaskRow,counterMaskColumn) = 255;
        } else {
          ch8MaskChannel.at(counterMaskRow,counterMaskColumn) = 0;
        }
      }
    }

    return failureCheck;
  }

  /**
   * building the object mask under consideration of neighbourhoods, covariance
   * matrix and probabilities
   * @param img the image to be considered
   * @param ch8MaskChannel the mask estimating the object
   * @param intSparseMat matix containing which labels are related
   * @param meanProbabilityList a list containing mean probability, color and
   *        relation towards background and object for each region
   * @param newLabelsMatrix matrix containing the labelnumber for each pixel
   * @param converged true if no change took place
   * @return true if succesfull
   */
  bool coilBackgroundSegmentation::growInsideObject(
                         const image& img,
                         channel8& ch8MaskChannel,
                         sparseMatrix<int>& intSparseMat,
                         std::vector<TmeanProbability>& meanProbabilityList,
                         matrix<int>& newLabelsMatrix, bool& converged) const {

    const float fThreshold = 0.75f;
    bool failureCheck = true;
    dmatrix covarMatrix;
    trgbPixel<double> mean;

    failureCheck &= getInvertedCovarMatrixForMask(img, ch8MaskChannel, mean, covarMatrix);

    int counterColorRow, counterColorColumn;
    for (counterColorRow=0; counterColorRow<intSparseMat.rows(); ++counterColorRow) {
      if (meanProbabilityList[counterColorRow].labelToObjectVector == true) { //check if region belongs to the object
        // search the upper right of intSparseMat
        for (counterColorColumn=counterColorRow; counterColorColumn<intSparseMat.columns(); ++counterColorColumn) {
          if (intSparseMat.getAt(counterColorRow,counterColorColumn) == 1) { //if regions are neighbours
            if ((checkColorReasonable(mean, covarMatrix, meanProbabilityList[counterColorColumn].meanColor)) // the color is sensible for the object
              ||(meanProbabilityList[counterColorColumn].meanProbability >= fThreshold)) {  // and the probability is high enough => region is considered to belog to the object
              meanProbabilityList[counterColorColumn].labelToObjectVector = true;
              converged = false;
            }
            else {  //otherwise the region does not belong to the object
              meanProbabilityList[counterColorColumn].labelToObjectVector = false;
              converged = false;
            };
          };
        };
      };
    };

    int counterMaskRow, counterMaskColumn;
    for (counterMaskRow=0; counterMaskRow<ch8MaskChannel.rows(); ++counterMaskRow) {
      for (counterMaskColumn=0; counterMaskColumn<ch8MaskChannel.columns(); ++counterMaskColumn) {
        if (meanProbabilityList[newLabelsMatrix.at(counterMaskRow,counterMaskColumn)].labelToObjectVector == true) {
          ch8MaskChannel.at(counterMaskRow,counterMaskColumn) = 255;
        } else {
          ch8MaskChannel.at(counterMaskRow,counterMaskColumn) = 0;
        }
      }
    }

    return failureCheck;
  }

  // -------------------------------------------------------------------
  // The apply-methode!
  // -------------------------------------------------------------------

  /**
   * generates a mask with the value 255 where the object is estimated
   * and 0 for the background.
   *
   * @param src image with the source data
   * @param result the resulting mask
   * @return true if apply successful or false otherwise
   */
  bool coilBackgroundSegmentation::apply(const image& src,
                                channel8& result) {

    /////////////////////////////////////////////////////////
    //Local variables
    /////////////////////////////////////////////////////////

    const int numberOfRegions = 20;
    int zeroCounter = 0;
    bool failureCheck = true, bestOfConverged = false;

    std::vector<channel8> masksVector;
    channel8 ch8MaskChannelBackground, ch8MaskChannelObject, ch8QuantChannel, ch8CurrentMask;
    channel chSegmentationChannel, chFinalChannel;
    palette quantPalette;

    kMColorQuantization colorQuantization;
    kMColorQuantization::parameters colorQuantParam;

    probabilityMap pMap;
    probabilityMap::parameters pMapPar;

    colorModelEstimator colModEstMask;
    thistogram<double> histObjColor, histBgColor;

    sparseMatrix<int> intSparseMat;
    objectsFromMask moreLabeledMask, largestObjFromMask;
    objectsFromMask::parameters objectsFromMaskParam, largestObjFromMaskParam;
    matrix<int> newLabelsMatrix, matrixObjectnumbers;
    std::list<areaPoints> dummy;

    draw<ubyte> ch8drawBackground, ch8drawObject;

    kNearestNeighFilter nearestNeighFilter;
    kNearestNeighFilter::parameters neighParam;
    neighParam.kernelSize = 5;
    nearestNeighFilter.setParameters(neighParam);

    int numberFormerZeros = 0;

    /////////////////////////////////////////////////////////


    if (getParameters().maxNumberBestOfN < 2) {
      failureCheck = false;
    }
     //best-of-loop
    int bestOfCounter;
    for (bestOfCounter=0; bestOfCounter < getParameters().maxNumberBestOfN&&bestOfConverged==false; ++bestOfCounter) {

      // initialisation
      bool converged = false, convergedOutside = false, convergedInside = false;
      ch8CurrentMask.resize(src.size(),0,false,true);
      ch8QuantChannel.resize(src.size(),0,false,true);
      ch8MaskChannelBackground.resize(src.size(),255,false,true);
      ch8MaskChannelObject.resize(src.size(),255,false,true);
      chSegmentationChannel.resize(src.size(),0,false,true);
      chFinalChannel.resize(src.size(),255,false,true);
      quantPalette.resize(numberOfRegions,rgbPixel(),false,true);
      //don´t initialise histograms because of better results <= like numberBestOfN*iterations steps


      // quantization of colors
      colorQuantParam.numberOfColors = numberOfRegions;
      colorQuantization.setParameters(colorQuantParam);
      failureCheck &= colorQuantization.apply(src, ch8QuantChannel, quantPalette);

      // generating labels for whole regions instead of multiple regions
      // with the same label
      newLabelsMatrix.resize(src.size(),255,false,true);
      objectsFromMaskParam.level = 1;
      objectsFromMaskParam.assumeLabeledMask = true;
      objectsFromMaskParam.threshold = 0;
      moreLabeledMask.setParameters(objectsFromMaskParam);
      failureCheck &= moreLabeledMask.apply(ch8QuantChannel, dummy, newLabelsMatrix);

      // generating the matrix containing the neighbourhoods
      intSparseMat.resize(newLabelsMatrix.maximum()+1,newLabelsMatrix.maximum()+1,0);
      int row, column;
      for (row=1; row<newLabelsMatrix.rows(); ++row) {
        for (column=1; column<newLabelsMatrix.columns(); ++column) {
          intSparseMat.setAt(newLabelsMatrix.at(row,column),newLabelsMatrix.at(row,column-1),1);
          intSparseMat.setAt(newLabelsMatrix.at(row,column-1),newLabelsMatrix.at(row,column),1);
          intSparseMat.setAt(newLabelsMatrix.at(row,column),newLabelsMatrix.at(row-1,column),1);
          intSparseMat.setAt(newLabelsMatrix.at(row-1,column),newLabelsMatrix.at(row,column),1);
        }
      }

      // initialising meanProbabilityList
      std::vector<TmeanProbability> meanProbabilityList(newLabelsMatrix.maximum()+1);
      int initCounter;
      for (initCounter=0;
	   initCounter<static_cast<int>(meanProbabilityList.size());
	   ++initCounter) {
        meanProbabilityList[initCounter].sumProbability = 0;
        meanProbabilityList[initCounter].appearanceCounter = 0;
        meanProbabilityList[initCounter].sumColorRed = 0;
        meanProbabilityList[initCounter].sumColorGreen = 0;
        meanProbabilityList[initCounter].sumColorBlue = 0;
        meanProbabilityList[initCounter].meanColor = rgbPixel(0,0,0);
        meanProbabilityList[initCounter].labelToObjectVector = false;
        meanProbabilityList[initCounter].labelToBackgroundVector = false;
      }

      // calculating the region´s mean colors
      int counterColorsI, counterColorsJ;
      for (counterColorsI=0; counterColorsI<newLabelsMatrix.rows(); ++counterColorsI) {
        for (counterColorsJ=0; counterColorsJ<newLabelsMatrix.columns(); ++counterColorsJ) {
          const int labCount=newLabelsMatrix.at(counterColorsI,counterColorsJ);
          ++meanProbabilityList[labCount].appearanceCounter;
          meanProbabilityList[labCount].sumColorRed += src.at(counterColorsI,counterColorsJ).getRed();
          meanProbabilityList[labCount].sumColorGreen += src.at(counterColorsI,counterColorsJ).getGreen();
          meanProbabilityList[labCount].sumColorBlue += src.at(counterColorsI,counterColorsJ).getBlue();
        }
      }

      int counterMeanColor;
      for (counterMeanColor=0;
	   counterMeanColor<static_cast<int>(meanProbabilityList.size());
	   ++counterMeanColor)
      {
        if (meanProbabilityList[counterMeanColor].appearanceCounter > 0) {
          meanProbabilityList[counterMeanColor].meanColor.setRed(meanProbabilityList[counterMeanColor].sumColorRed/meanProbabilityList[counterMeanColor].appearanceCounter);
          meanProbabilityList[counterMeanColor].meanColor.setGreen(meanProbabilityList[counterMeanColor].sumColorGreen/meanProbabilityList[counterMeanColor].appearanceCounter);
          meanProbabilityList[counterMeanColor].meanColor.setBlue(meanProbabilityList[counterMeanColor].sumColorBlue/meanProbabilityList[counterMeanColor].appearanceCounter);
        }
      }

      // initialising the masks
      // create a mask with the outer border set to 255 (white)
      ch8MaskChannelBackground.resize(src.size(),255,false,true);
      ch8drawBackground.use(ch8MaskChannelBackground);
      ch8drawBackground.setColor(ubyte(0));
      ch8drawBackground.rectangle(
              static_cast<int>(1+5*getParameters().maskFactor-1.0f),
              static_cast<int>(1+5*getParameters().maskFactor-1.0f),
              static_cast<int>(ch8MaskChannelBackground.columns()-2-
                               5*getParameters().maskFactor-1.0f),
              static_cast<int>(ch8MaskChannelBackground.rows()-2-
                               5*(getParameters().maskFactor-1.0f)),
              true);

      // draw a rectangle in the middle of the object-mask
      ch8drawObject.use(ch8MaskChannelObject);
      ch8drawObject.setColor(ubyte(0));
      ch8drawObject.clear();
      ch8drawObject.setColor(255);
      ch8drawObject.rectangle(
                        static_cast<int>(ch8MaskChannelObject.columns()/2-
                                         5-10*(getParameters().maskFactor-1)),
                        static_cast<int>(ch8MaskChannelObject.rows()/2-
                                         5-10*(getParameters().maskFactor-1)),
                        static_cast<int>(ch8MaskChannelObject.columns()/2+
                                         5+10*(getParameters().maskFactor-1)),
                        static_cast<int>(ch8MaskChannelObject.rows()/2+
                                         5+10*(getParameters().maskFactor-1)),
                        constants<float>::Pi()/4.0f,true);

      // initialising the matrix containing which label belongs to the object 
      // and which to the background
      int counterLabelObjRow, counterLabelObjColumn;
      for (counterLabelObjRow=0; 
           counterLabelObjRow<ch8MaskChannelObject.rows();
           ++counterLabelObjRow) {
        for (counterLabelObjColumn=0; 
             counterLabelObjColumn<ch8MaskChannelObject.columns();
             ++counterLabelObjColumn) {
          if (ch8MaskChannelObject.at(counterLabelObjRow,
                                      counterLabelObjColumn) == 255) {
            meanProbabilityList[
             newLabelsMatrix.at(counterLabelObjRow,
                                counterLabelObjColumn)].labelToObjectVector =
              true;
          }
        }
      }
      int counterLabelBgRow, counterLabelBgColumn;
      for (counterLabelBgRow=0; 
           counterLabelBgRow<ch8MaskChannelBackground.rows();
           ++counterLabelBgRow) {
        for (counterLabelBgColumn=0;
             counterLabelBgColumn<ch8MaskChannelBackground.columns(); 
             ++counterLabelBgColumn) {
          if (ch8MaskChannelBackground.at(counterLabelBgRow,
                                          counterLabelBgColumn) == 255) {
            meanProbabilityList[
              newLabelsMatrix.at(counterLabelBgRow,
                         counterLabelBgColumn)].labelToBackgroundVector = true;
          }
        }
      }


      // iteration loop
      int iterationCounter;
      for (iterationCounter=0; iterationCounter<getParameters().iterations&&converged==false; ++iterationCounter) {



        // create color model for the background
        colModEstMask.reset();
        colModEstMask.consider(src, ch8MaskChannelBackground);
        failureCheck &= colModEstMask.apply(histBgColor);

        // create color model for the object
        colModEstMask.reset();
        colModEstMask.consider(src, ch8MaskChannelObject);
        failureCheck &= colModEstMask.apply(histObjColor);

        // set the color models 
        //(inverted because all not occured colors would be considered as background)
        pMapPar.setObjectColorModel(histBgColor);
        pMapPar.setNonObjectColorModel(histObjColor);
        pMap.setParameters(pMapPar);
         //chSegmentationChannel will contain the probabilities
        failureCheck &= pMap.apply(src, chSegmentationChannel);
        // inverting background and object
        int changeCounterI, changeCounterJ;
        for (changeCounterI=0; changeCounterI<chSegmentationChannel.rows(); ++changeCounterI) {
          for (changeCounterJ=0; changeCounterJ<chSegmentationChannel.columns(); ++changeCounterJ) {
            chSegmentationChannel.at(changeCounterI,changeCounterJ) = 1-chSegmentationChannel.at(changeCounterI,changeCounterJ);
          }
        }


        // generating the mean probability for each region
        int counterI, counterJ;
        for (counterI=0; counterI<newLabelsMatrix.rows(); ++counterI) {
          for (counterJ=0; counterJ<newLabelsMatrix.columns(); ++counterJ) {
            meanProbabilityList[newLabelsMatrix.at(counterI,counterJ)].sumProbability += chSegmentationChannel.at(counterI,counterJ);
            ++meanProbabilityList[newLabelsMatrix.at(counterI,counterJ)].appearanceCounter;
          }
        }
        int counterMeanProb;
        for (counterMeanProb=0;
	     counterMeanProb<static_cast<int>(meanProbabilityList.size());
	     ++counterMeanProb)
        {
          if (meanProbabilityList[counterMeanProb].appearanceCounter > 0) {
            meanProbabilityList[counterMeanProb].meanProbability = meanProbabilityList[counterMeanProb].sumProbability/meanProbabilityList[counterMeanProb].appearanceCounter;
          }
        }


        // grow the outer and inner mask dependent on neighbourhoods and probabilities
        failureCheck &= growOutsideObject(src, ch8MaskChannelBackground, intSparseMat, meanProbabilityList, newLabelsMatrix, convergedOutside);
        failureCheck &= growInsideObject(src, ch8MaskChannelObject, intSparseMat, meanProbabilityList, newLabelsMatrix, convergedInside);

        // if there has been no change in one of the masks => finished
        if ((convergedOutside)&&(convergedInside)) {
          converged = true;
        }

      } //end of iteration loop

      // give whole region its mean probability
      int presentCounterI, presentCounterJ;
      for (presentCounterI=0; presentCounterI<newLabelsMatrix.rows(); ++presentCounterI) {
        for (presentCounterJ=0; presentCounterJ<newLabelsMatrix.columns(); ++presentCounterJ) {
          chFinalChannel.at(presentCounterI,presentCounterJ) = meanProbabilityList[newLabelsMatrix.at(presentCounterI,presentCounterJ)].meanProbability;
        }
      }


     // get threshold as a mean value from the outmost rectangle (this is expected to be background)
      double threshold = 0;
      int counterI, counterJ;
      for (counterI=0; counterI<chFinalChannel.rows(); ++counterI) {
        threshold += chFinalChannel.at(counterI,0);
        threshold += chFinalChannel.at(counterI,chFinalChannel.columns()-1);
      }
      for (counterJ=0; counterJ<chFinalChannel.columns(); ++counterJ) {
        threshold += chFinalChannel.at(0,counterJ);
        threshold += chFinalChannel.at(chFinalChannel.rows()-1,counterJ);
      }
      threshold /= 2*(chFinalChannel.rows()+chFinalChannel.columns());
      threshold *= getParameters().thresholdFactor;

      int counterX, counterY;
      for (counterX=0; counterX<chFinalChannel.rows(); ++counterX) {
        for (counterY=0; counterY<chFinalChannel.columns(); ++counterY) {
          if(chFinalChannel.at(counterX,counterY) > threshold) {
            ch8CurrentMask.at(counterX,counterY) = 255; //belonging to object
          } else {
            ch8CurrentMask.at(counterX,counterY) = 0; //belonging to background
          }
        }
      }


      // after-treatment
      // fill small holes and smooth edges
      failureCheck &= nearestNeighFilter.apply(ch8CurrentMask);

      // add current mask
      masksVector.push_back(ch8CurrentMask);

      // combine to best-of: only where all masks are nonzero the result will be nonzero
      // zerocounter indicates how often the difference of zero-values in succeding masks
      // has to be zero to end the iteration
      int numberZeros = 0, meltCounter = masksVector.size()-1, resultCounterX, resultCounterY, formerCounterX, formerCounterY;
      if (masksVector.size()>1) {
        bestOfConverged = true;
        for (resultCounterX=0; resultCounterX<chFinalChannel.rows(); ++resultCounterX) {
          for (resultCounterY=0; resultCounterY<chFinalChannel.columns(); ++resultCounterY) {
            if (masksVector[meltCounter-1].at(resultCounterX,resultCounterY) != masksVector[meltCounter].at(resultCounterX,resultCounterY)) {
              masksVector[meltCounter].at(resultCounterX,resultCounterY) = 0; //belonging to background
            }
            if (masksVector[meltCounter].at(resultCounterX,resultCounterY) == 0) {
              ++numberZeros;
            }
          }
        }

        if (numberZeros-numberFormerZeros == 0) {
          ++zeroCounter;
        } else if (numberZeros-numberFormerZeros > 4){
          zeroCounter--;
        }
        if (zeroCounter < 2) {
          bestOfConverged = false;
          numberFormerZeros = numberZeros;
        }
      } else {  // first run => calculating the first numberFormerZeros
        result = masksVector[0];
        numberFormerZeros = 0;
        for (formerCounterX=0; formerCounterX<chFinalChannel.rows(); ++formerCounterX) {
          for (formerCounterY=0; formerCounterY<chFinalChannel.columns(); ++formerCounterY) {
            if (masksVector[0].at(formerCounterX,formerCounterX) == 0) {
              ++numberFormerZeros;
            }
          }
        }
      }

      // delete special combinations
      deleteWeakLinks(masksVector[masksVector.size()-1]);

      // only the object in the middle is kept
      largestObjFromMaskParam.level = 1;
      largestObjFromMaskParam.assumeLabeledMask = true;
      largestObjFromMaskParam.sortByArea = true;
      largestObjFromMask.setParameters(largestObjFromMaskParam);
      largestObjFromMask.apply(masksVector[masksVector.size()-1], dummy, matrixObjectnumbers);
      int counterObjectnumbersRow, counterObjectnumbersColumn, objectNumber = matrixObjectnumbers.at(matrixObjectnumbers.rows()/2,matrixObjectnumbers.columns()/2);
      for (counterObjectnumbersRow=0; counterObjectnumbersRow<matrixObjectnumbers.rows(); ++counterObjectnumbersRow) {
        for (counterObjectnumbersColumn=0; counterObjectnumbersColumn<matrixObjectnumbers.columns(); ++counterObjectnumbersColumn) {
          if (matrixObjectnumbers.at(counterObjectnumbersRow,counterObjectnumbersColumn) != objectNumber) {
            masksVector[masksVector.size()-1].at(counterObjectnumbersRow,counterObjectnumbersColumn) = 0;
          }
        }
      }

    } //end of best-of-loop

    result = masksVector[masksVector.size()-1];


    return failureCheck;
  };
}
