/*
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
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
 * file .......: ltiSegmentationEvaluation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.11.2003
 * revisions ..: $Id: ltiSegmentationEvaluation.cpp,v 1.1 2006/03/10 02:53:11 alvarado Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiSegmentationEvaluation.h"
#include "ltiSerialVectorStats.h"
#include "ltiSerialStatsFunctor.h"
#include "ltiFastRelabeling.h"
#include "ltiLoadImageList.h"
#include "ltiALLFunctor.h"
#include "ltiLTIFunctor.h"
#include "ltiTimer.h"
#include "ltiConstants.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#define _LTI_SAVE_PARAM 1
#include "ltiLispStreamHandler.h"
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace lti {


  // --------------------------------------------------
  // segmentationEvaluation::parameters
  // --------------------------------------------------

  // default constructor
  segmentationEvaluation::parameters::parameters()
    : paretoFront::parameters() {

    numberOfRegions = false;
    numRegionsReciprocal = false;
    pixelWisePotentialAccuracy = true;
    objectWisePotentialAccuracy = false;
    regionWiseInformationContent = true;
    throughput = false;
    regionIntegrity = false;
    pixelWiseCertainty = false;

    images = std::string("images.txt");
    goldenPostfix = "_mask";
    prevStagePostfix = "_ibs";

    minValidRegionSize = 0.0005f;
 }

  // copy constructor
  segmentationEvaluation::parameters::parameters(const parameters& other)
    : paretoFront::parameters() {
    copy(other);
  }

  // destructor
  segmentationEvaluation::parameters::~parameters() {
  }

  // get type name
  const char* segmentationEvaluation::parameters::getTypeName() const {
    return "segmentationEvaluation::parameters";
  }

  // copy member

  segmentationEvaluation::parameters&
    segmentationEvaluation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    paretoFront::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    paretoFront::parameters& (paretoFront::parameters::* p_copy)
      (const paretoFront::parameters&) =
      paretoFront::parameters::copy;
    (this->*p_copy)(other);
# endif

    numberOfRegions = other.numberOfRegions;
    numRegionsReciprocal = other.numRegionsReciprocal;
    pixelWisePotentialAccuracy = other.pixelWisePotentialAccuracy;
    objectWisePotentialAccuracy = other.objectWisePotentialAccuracy;
    regionWiseInformationContent = other.regionWiseInformationContent;
    throughput = other.throughput;
    regionIntegrity = other.regionIntegrity;
    pixelWiseCertainty = other.pixelWiseCertainty;

    images = other.images;
    goldenPostfix = other.goldenPostfix;
    prevStagePostfix = other.prevStagePostfix;

    minValidRegionSize = other.minValidRegionSize;

    return *this;
  }

  // alias for copy member
  segmentationEvaluation::parameters&
  segmentationEvaluation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* segmentationEvaluation::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool segmentationEvaluation::parameters::write(ioHandler& handler,
                                         const bool& complete) const
# else
  bool segmentationEvaluation::parameters::writeMS(ioHandler& handler,
                                           const bool& complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"numberOfRegions",numberOfRegions);
      lti::write(handler,"numRegionsReciprocal",numRegionsReciprocal);
      lti::write(handler,"pixelWisePotentialAccuracy",
                 pixelWisePotentialAccuracy);
      lti::write(handler,"objectWisePotentialAccuracy",
                 objectWisePotentialAccuracy);
      lti::write(handler,"regionWiseInformationContent",
                 regionWiseInformationContent);
      lti::write(handler,"throughput",throughput);
      lti::write(handler,"regionIntegrity",regionIntegrity);
      lti::write(handler,"pixelWiseCertainty",pixelWiseCertainty);

      lti::write(handler,"images",images);
      lti::write(handler,"goldenPostfix",goldenPostfix);
      lti::write(handler,"prevStagePostfix",prevStagePostfix);

      lti::write(handler,"minValidRegionSize",minValidRegionSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && paretoFront::parameters::write(handler,false);
# else
    bool (paretoFront::parameters::* p_writeMS)(ioHandler&,const bool&) const =
      paretoFront::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool segmentationEvaluation::parameters::write(ioHandler& handler,
                                         const bool& complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if read was succeful
   */
# ifndef _LTI_MSC_6
  bool segmentationEvaluation::parameters::read(ioHandler& handler,
                                        const bool& complete)
# else
  bool segmentationEvaluation::parameters::readMS(ioHandler& handler,
                                          const bool& complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"numberOfRegions",numberOfRegions);
      lti::read(handler,"numRegionsReciprocal",numRegionsReciprocal);
      lti::read(handler,"pixelWisePotentialAccuracy",
                 pixelWisePotentialAccuracy);
      lti::read(handler,"objectWisePotentialAccuracy",
                 objectWisePotentialAccuracy);
      lti::read(handler,"regionWiseInformationContent",
                 regionWiseInformationContent);
      lti::read(handler,"throughput",throughput);
      lti::read(handler,"regionIntegrity",regionIntegrity);
      lti::read(handler,"pixelWiseCertainty",pixelWiseCertainty);

      lti::read(handler,"images",images);
      lti::read(handler,"goldenPostfix",goldenPostfix);
      lti::read(handler,"prevStagePostfix",prevStagePostfix);

      lti::read(handler,"minValidRegionSize",minValidRegionSize);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && paretoFront::parameters::read(handler,false);
# else
    bool (paretoFront::parameters::* p_readMS)(ioHandler&,const bool&) =
      paretoFront::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool segmentationEvaluation::parameters::read(ioHandler& handler,
                                        const bool& complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // segmentationEvaluation
  // --------------------------------------------------

  const imatrix segmentationEvaluation::emptyMatrix;

  // default constructor
  segmentationEvaluation::segmentationEvaluation()
    : paretoFront() {
  }

  // default constructor
  segmentationEvaluation::segmentationEvaluation(const parameters& par)
    : paretoFront(par) {
  }


  // copy constructor
  segmentationEvaluation::segmentationEvaluation(
                 const segmentationEvaluation& other) : paretoFront(other) {
  }

  // destructor
  segmentationEvaluation::~segmentationEvaluation() {
  }

  // returns the name of this type
  const char* segmentationEvaluation::getTypeName() const {
    return "segmentationEvaluation";
  }

  // return parameters
  const segmentationEvaluation::parameters& 
  segmentationEvaluation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool segmentationEvaluation::setParameters(const functor::parameters& par){
    if (paretoFront::setParameters(par)) {
      return readData();
    }

    return false;
  }
  
  
  std::string 
  segmentationEvaluation::getMaskName(const std::string& imgName,
                                      const std::string& goldenPostfix) const {
    std::string file(imgName);
    std::string::size_type pos = file.rfind('.');
    if (pos != std::string::npos) {
      file.insert(pos,goldenPostfix);
    } else {
      file+=goldenPostfix;
    }
    return file;
  }

  std::string 
  segmentationEvaluation::getPrevName(const std::string& imgName,
                                      const std::string& prevPostfix) const {
    std::string file;
    std::string::size_type pos = imgName.rfind('.');
    if (pos != std::string::npos) {
      file = imgName.substr(0,pos) + prevPostfix + ".lti";
    } else {
      file = imgName + prevPostfix + ".lti";
    }
    return file;
  }

  bool segmentationEvaluation::readData() {
    const parameters& param = getParameters();
    
    // initialize loader
    loadImageList loader(param.images);
    loadImage iloader;
    loadLTI ploader;

    image img;
    channel8 mask;
    imatrix pmask;
    palette pal;
    int i;
    bool error = false;
    std::string file;
    image emptyImage;
    channel8 emptyChannel;

    // the channels to be read (imageData) are a class attribute:
    imageData.clear();
    imageData.reserve(loader.size());

    maskData.clear();
    maskData.reserve(loader.size());

    prevMaskData.clear();
    prevMaskData.reserve(loader.size());

    goldenNames.clear();
    goldenNames.reserve(loader.size());

    i=0;
    while (loader.hasNext()) {
      file = loader.getNextFileName();
      if (loader.apply(img) && 
          iloader.load(getMaskName(file,param.goldenPostfix),mask,pal)) {
        
        goldenNames.push_back(file); // cache the filename

        maskData.push_back(emptyChannel);
        imageData.push_back(emptyImage);

        mask.detach(maskData[i]);
        img.detach(imageData[i]);

        // try to load previous stage mask
        if (ploader.load(getPrevName(file,param.prevStagePostfix),pmask)) {
          if (i == static_cast<int>(prevMaskData.size())) {
            prevMaskData.push_back(emptyMatrix);
          } else {
            prevMaskData.resize(i+1,emptyMatrix);
          }
          pmask.detach(prevMaskData[i]);
        } else {
          if (!prevMaskData.empty()) {
            if (!error) {
              setStatusString("Some masks for the previous level not found:");
              error = true;
            }
            file = getPrevName(file,param.prevStagePostfix) + "\n  ";
            appendStatusString(file.c_str());
          }
        }

        i++;
      } else {
        if (!error) {
          setStatusString("Following images could not be read:\n");
          error = true;
        }

        file += "\n  ";
        appendStatusString(file.c_str());
      }
    }
    
    if ( !error && (i==0) ) {
      setStatusString("No images could be found.  Maybe wrong path?");
      return false;
    }
    
    return !error;
  }
   
  // -----------------------------------------------------------------------
  // Static member initialization
  // -----------------------------------------------------------------------
  const int segmentationEvaluation::totalFitnessDimensionality = 8;

  functor::parameters* 
  segmentationEvaluation::chromosomeToPhenotype(
                                            const chromosome& genotype) const {
    functor::parameters* parPtr = getInstanceOfParam();
    if (notNull(parPtr)) {
      chromosomeToPhenotype(genotype,*parPtr);
    }
    return parPtr;
  }


  /*
   * Evaluate Chromosome
   *
   * This method is one of the most important ones for the pareto evaluation.
   * Its task is to produce a multidimensional fitness measure for a given
   * chromosome.
   *
   * It returns true if the evaluation was successful, of false if the
   * phenotype represents some invalid parameterization.  It is highly 
   * recomended that the mutation and crossover methods are reimplemented to 
   * avoid invalid parameterizations.
   */
  bool segmentationEvaluation::evaluateChromosome(const chromosome& individual,
                                                  dvector& fitness) {
    
    functor::parameters* segPar = chromosomeToPhenotype(individual);
    dvector mfitness;

#ifdef _LTI_DEBUG
#ifdef _LTI_SAVE_PARAM
    // for debug purposes, if the system crashes, the given file will contain
    // the parameters currently under evaluation, so that it can be possible
    // to isolate the case:
    char buffer[128];
    sprintf(buffer,"lti_ibseval_dbg_%i.txt",getpid());
    std::ofstream _dbg_out(buffer);
    lispStreamHandler lsh(_dbg_out);
    segPar->write(lsh);
    _dbg_out << std::endl;
    _dbg_out.close();
#endif
#endif  

    if (evaluate(*segPar,mfitness)) {
      const parameters& par = getParameters();
      if (fitness.size() != par.fitnessSpaceDimensionality) {
        fitness.resize(par.fitnessSpaceDimensionality,double(),false,false);
      }

      int j;
      j = 0;

      if ((par.numberOfRegions) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxNumRegions);
        j++;
      } 

      if ((par.numRegionsReciprocal) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxNumRegionsRecip);
        j++;
      } 

      if ((par.pixelWisePotentialAccuracy) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxPixelAccuracy);
        j++;
      }

      if ((par.objectWisePotentialAccuracy) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxObjectAccuracy);
        j++;
      }

      if ((par.regionWiseInformationContent) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxRegionWiseInfo);
        j++;
      }

      if ((par.throughput) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxThroughput);
        j++;
      }

      if ((par.regionIntegrity) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxRegionIntegrity);
        j++;
      }

      if ((par.pixelWiseCertainty) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(IdxPixelCertainty);
        j++;
      }
    } else {
      delete segPar;
      segPar = 0;
      return false;
    }

    delete segPar;
    segPar = 0;

    return true;
  }

  
  bool segmentationEvaluation::evaluate(const functor::parameters& param,
                                        dvector& fitness) {

    // const parameters& par = getParameters();
    image img;
    channel chnl;

    // initialize the segmentation functor
    if (!init(param)) {
      return false;
    }

    const int size = static_cast<int>(imageData.size());

    if (size == 0) {
      // the statusString was initialized while setting the parameters!
      return false;
    }

    // assume all images are ok
    dvector accFitness(totalFitnessDimensionality,0.0);
    dvector accNorm(totalFitnessDimensionality,0.0);
    dvector norm(totalFitnessDimensionality,0.0);

    int i=0; // i counts the number of successful readed images
    int j=0;
    
    // are there any mask in the previous masks cache?
    
    if (prevMaskData.empty()) {
      static const imatrix emptyIMatrix;

      // for all channels read in setParameters()
      while (i<size) {
        // evaluate the image with the given ground data
        if (evaluate(imageData[i],emptyIMatrix,maskData[i],fitness,norm)) {
          accFitness.add(fitness);
          accNorm.add(norm);
          j++;
        } 
        ++i;
      }

    } else {
      // for all channels read in setParameters()
      while (i<size) {
        // evaluate the image with the given ground data
        if (evaluate(imageData[i],prevMaskData[i],maskData[i],fitness,norm)) {
          accFitness.add(fitness);
          accNorm.add(norm);
          j++;
        }
        ++i;
      }
    }

    fitness.edivide(accFitness,accNorm);

    return (j==size);
  }

  bool segmentationEvaluation::evaluate(const image& img,
                                        const imatrix& prevStage,
                                        const channel8& mask,
                                        const functor::parameters& param,
                                        dvector& fitness,
                                        dvector& norm) {

    if (init(param)) {
      return evaluate(img,prevStage,mask,fitness,norm);
    } 
    return false;
  }

  bool segmentationEvaluation::evaluate(const image& img,
                                        const channel8& mask,
                                        const functor::parameters& param,
                                        dvector& fitness,
                                        dvector& norm) {

    if (init(param)) {
      static const imatrix emptyMatrix;
      return evaluate(img,emptyMatrix,mask,fitness,norm);
    } 
    return false;
  }

  bool segmentationEvaluation::evaluate(const image& img,
                                        const imatrix& prevStage,
                                        const channel8& refMask,
                                              dvector& fitness,
                                              dvector& norm) {


    timer chronos;

    imatrix mask;
    channel certainty;

    // call the segmentation method of the inherited class and measure the
    // time it takes.
    chronos.start();
    if (!segment(img,prevStage,mask,certainty)) {
      chronos.stop();
      return false;
    }
    chronos.stop();

    const parameters& par = getParameters();

    if (evaluate(mask,refMask,fitness,norm,par.minValidRegionSize)) {
      // evaluate did't care about certainty and throughput, but we do
      fitness.at(IdxThroughput) = 1000000.0;
      norm.at(IdxThroughput) = chronos.getTime(); // segms per second

      if (certainty.empty()) {
        fitness.at(IdxPixelCertainty) = 1.0;
        norm.at(IdxPixelCertainty) = 1.0;
      } else {
        fitness.at(IdxPixelCertainty) = certainty.sumOfElements();
        norm.at(IdxPixelCertainty) = certainty.rows()*certainty.columns();
      }      
    }

    return true;
  }

  bool segmentationEvaluation::evaluate(const imatrix& mask,
                                        const channel8& refMask,
                                              dvector& fitness,
                                              dvector& norm,
                                        const float minValidRegionSize) {

    fitness.resize(totalFitnessDimensionality,0.0,false,true);
    norm.resize(totalFitnessDimensionality,0.0,false,true);

    imatrix rmask,rrefMask;

    // mask   : the result of the current parameterization/algorithm
    // refMask: ground truth

    // analyze the mask
    int potentialMax,potentialMin;
    mask.getExtremes(potentialMin,potentialMax);
    imatrix::const_iterator imit,eimit;
    int i,j,x,y;
    ivector tmpVct;

    // compute region sizes
    tmpVct.resize(1+potentialMax-potentialMin,0);
    for (imit=mask.begin(),eimit=mask.end();imit!=eimit;++imit) {
      tmpVct.at((*imit)-potentialMin)++;
    }
   
    // count number of regions
    int numRegs = 0;
    ivector equivSeg(tmpVct.size(),-1);
    for (i=0,j=0;i<tmpVct.size();++i) {
      if (tmpVct.at(i) != 0) {
        equivSeg.at(i)=j;
        numRegs++;
        j++;
      }
    }
    
    // compact all region sizes into a vector indexed from 0 to numRegs-1
    ivector regSizes(false,numRegs);
    for (i=0,j=0;i<tmpVct.size();++i) {
      if (tmpVct.at(i) != 0) {
        regSizes.at(j)=tmpVct.at(i);
        j++;
      }
    }
    
    // analyze the reference mask

    // compute region sizes
    channel8::const_iterator cit,ecit;

    // count number of pixels per region (since channel8 -> only 256 regs pos.)
    tmpVct.resize(256,0,false,true);
    for (cit=refMask.begin(),ecit=refMask.end();cit!=ecit;++cit) {
      tmpVct.at(*cit)++;
    }
   
    // count number of regions
    int rNumRegs = 0;
    ivector rEquivSeg(tmpVct.size(),0);
    for (i=0,j=0;i<tmpVct.size();++i) {
      if (tmpVct.at(i) != 0) {
        rEquivSeg.at(i)=j;
        rNumRegs++;
        j++;
      }
    }

    // compact the region sizes into a vector with rNumRegs elements
    ivector rRegSizes(false,rNumRegs);
    for (i=0,j=0;i<tmpVct.size();++i) {
      if (tmpVct.at(i) != 0) {
        rRegSizes.at(j)=tmpVct.at(i);
        j++;
      }
    }
    
    // now construct the matrix with all necessary data: It contains in each
    // row the information for a region of the evaluated segmentation many of
    // its pixels belong to each of the ground-truth regions
    //
    // info Matrix: Ref 0 | Ref 1 | ... | Ref n | 
    //              -----------------------------
    //     region 0 |  5  |  16   | ... |   2   |
    //              -----------------------------
    //     region 1 |  0  | 231   | ... |   0   |
    //              -----------------------------
    //       ...
    //              -----------------------------
    //     region m | 10  |  1    | ... |   0   |
    //              -----------------------------
    imatrix info(numRegs,rNumRegs,0);
    
    for (y=0;y<mask.rows();++y) {
      for (x=0;x<mask.columns();++x) {
        info.at(equivSeg.at(mask.at(y,x)-potentialMin),
                rEquivSeg.at(refMask.at(y,x)))++;
      }
    }

    // compute the probability of each ground-truth region
    dvector plambda;
    plambda.castFrom(rRegSizes);
    plambda.divide(refMask.rows()*refMask.columns());
    const int sizeThresh =
      iround(mask.rows()*mask.columns()*minValidRegionSize);
        
    // From info, all fitness measures can be computed:

    int osize;
    double wri;
    double tmp;
    double sumri=0;
    double sumwri=0;
    dvector sumo(rNumRegs,0.0);
    dvector sumi(rNumRegs,0.0);

    // for each region i
    for (i=0;i<info.rows();++i) {
      j = info.getRow(i).getIndexOfMaximum(); // j best object class
      tmp = info.at(i,j);  // number of pixels of i in ref.-region j
      sumo.at(j)+=tmp; // acc best object areas
      sumi.at(j)+=1.0; // count how many regions per reference region are used

      // region-wise information content
      osize = rRegSizes.at(j);
      wri = (osize > sizeThresh) ? 1.0 : 0.0;
      sumri += tmp*wri/osize;
      sumwri += wri;
    }

    double pa = 0;
    double sum = 0;

    double oa = 0.0;
    double woa = 0.0;
    double sumwoa = 0.0;

    for (i=0;i<sumo.size();++i) {
      osize = rRegSizes.at(i);
      // compute numerator and denominator of the pixel accuracy
      pa += sumo.at(i);
      sum += osize;

      // compute numerator and denominator of the object accuracy
      woa = (osize > sizeThresh) ? (1.0/plambda.at(i)) : 0;
      oa += sumo.at(i)*woa/osize;
      sumwoa +=woa;
    }

    // insert the first statistics in the results vector
    fitness.at(IdxNumRegions) = numRegs;
    norm.at(IdxNumRegions) = 1;

    fitness.at(IdxNumRegionsRecip) = 1.0/numRegs;
    norm.at(IdxNumRegionsRecip) = 1;

    fitness.at(IdxPixelAccuracy) = pa;
    norm.at(IdxPixelAccuracy) = sum;

    fitness.at(IdxObjectAccuracy) = oa;
    norm.at(IdxObjectAccuracy) = sumwoa;

    fitness.at(IdxRegionWiseInfo) = sumri;
    norm.at(IdxRegionWiseInfo) = sumwri;    

    double dx = sumi.sumOfElements()/sumi.size();
    static const double enorm = exp(-1.0);

    fitness.at(IdxRegionIntegrity) = dx*exp(-dx)/enorm;
    norm.at(IdxRegionIntegrity) = 1.0;

    return true;
  }

}

