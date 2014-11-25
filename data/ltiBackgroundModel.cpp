/*
 * Copyright (C) 2019
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
 * file .......: ltiBackgroundModel.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 19.1.2019
 * revisions ..: $Id: ltiBackgroundModel.cpp,v 1.11 2006/09/05 10:02:44 ltilib Exp $
 */

//TODO: include files
#include "ltiBackgroundModel.h"
#include "ltiSTLIoInterface.h"



namespace lti {
  // --------------------------------------------------
  // backgroundModel::parameters
  // --------------------------------------------------

  // default constructor
  backgroundModel::parameters::parameters()
    : segmentation::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    outputThreshold = 0.0f;
    alpha = 0.2f;
    adaptModel = false;
    adaptationThreshold = 0.0f;
    binaryOutput = false;
  }

  // copy constructor
  backgroundModel::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  backgroundModel::parameters::~parameters() {
  }

  // get type name
  const char* backgroundModel::parameters::getTypeName() const {
    return "backgroundModel::parameters";
  }

  // copy member

  backgroundModel::parameters&
    backgroundModel::parameters::copy(const parameters& other) {
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


      outputThreshold = other.outputThreshold;
      alpha = other.alpha;
      binaryOutput = other.binaryOutput;
      adaptModel = other.adaptModel;
      adaptationThreshold = other.adaptationThreshold;

    return *this;
  }

  // alias for copy member
  backgroundModel::parameters&
    backgroundModel::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* backgroundModel::parameters::clone() const {
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
  bool backgroundModel::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool backgroundModel::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"outputThreshold",outputThreshold);
      lti::write(handler,"alpha",alpha);
      lti::write(handler,"binaryOutput",binaryOutput);
      lti::write(handler,"adaptModel",adaptModel);
      lti::write(handler,"adaptationThreshold",adaptationThreshold);
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
  bool backgroundModel::parameters::write(ioHandler& handler,
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
  bool backgroundModel::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool backgroundModel::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"outputThreshold",outputThreshold);
      lti::read(handler,"alpha",alpha);
      lti::read(handler,"binaryOutput",binaryOutput);
      lti::read(handler,"adaptModel",adaptModel);
      lti::read(handler,"adaptationThreshold",adaptationThreshold);

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
  bool backgroundModel::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // backgroundModel
  // --------------------------------------------------

  // default constructor
  backgroundModel::backgroundModel()
    : segmentation(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    bgStat = std::vector< std::vector<  serialVectorStats<float> > >();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  backgroundModel::backgroundModel(const backgroundModel& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  backgroundModel::~backgroundModel() {
  }

  // returns the name of this type
  const char* backgroundModel::getTypeName() const {
    return "backgroundModel";
  }

  // copy member
  backgroundModel& backgroundModel::copy(const backgroundModel& other) {
    segmentation::copy(other);

    bgStat = other.bgStat;

    return (*this);
  }

  // alias for copy member
  backgroundModel&
    backgroundModel::operator=(const backgroundModel& other) {
    return (copy(other));
  }


  // clone member
  functor* backgroundModel::clone() const {
    return new backgroundModel(*this);
  }

  // return parameters
  const backgroundModel::parameters&
    backgroundModel::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  bool backgroundModel::updateParameters() {
    const parameters& par = getParameters();
    if( par.alpha<0 || par.alpha>1 ){
      setStatusString("Parameter alpha should be between 0 and 1.");
      return false;
    }
    else if( par.adaptationThreshold < 0 ){
      setStatusString("Parameter adaptationThreshold should be positive.");
      return false;
    }
    else if( par.outputThreshold < 0 ){
      setStatusString("Parameter outputThreshold should be positive.");
      return false;
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // an apply for channel8
  bool backgroundModel::apply(const image& src, channel8& dest){

    channel re;
    bool result = apply(src,re);
    dest.castFrom(re);
    return result;
  }

  // On copy apply for type image!
  bool backgroundModel::apply(const image& src, channel& dest){

    channel8 distanceImg;
    int r,c;
    float dist = 0.0f;
    vector<float> v1(3,0.0),mean(3,0.0),var(3,0.0);
    //get the parameters
    const parameters pa = getParameters();

    //get the x and y size of the vector< vector< > > struct
    point big;
    big.y = bgStat.size(); big.x = (*bgStat.begin()).size();

    if( bgStat.size()<1 ){
      setStatusString("No background model calculated yet.");
      return false;
    }
    else if( src.size() != big ){
      setStatusString("The size of the image 'src' do not fit to the internal model.");
      return false;
    }
    //resize the destination
    dest.resize(src.size(),0.0,false,true);
    distanceImg.resize(src.size(),0,false,true);

    //calculate the distance of the coller of each pixel
    for(r=0; r<src.rows(); r++){
      for(c=0; c<src.columns(); c++){

        bgStat[r][c].apply(mean,var);

        v1.at(0) = (src.at(r,c).getRed()-mean.at(0))   * (src.at(r,c).getRed()-mean.at(0));
        v1.at(1) = (src.at(r,c).getGreen()-mean.at(1)) * (src.at(r,c).getGreen()-mean.at(1));
        v1.at(2) = (src.at(r,c).getBlue()-mean.at(2))  * (src.at(r,c).getBlue()-mean.at(2));

        if( var.at(0)!=0 ) var.at(0) = 1 / var.at(0);
        else var.at(0) =1;
        if( var.at(1)!=0 ) var.at(1) = 1 / var.at(1);
        else var.at(1) =1;
        if( var.at(2)!=0 ) var.at(2) = 1 / var.at(2);
        else var.at(2) =1;

        //calculates the malanowis distance
        dist = v1.dot(var);

        //check if the pixel should set or not
        if( dist >= pa.outputThreshold)
          dest.at(r,c) = (pa.binaryOutput)?(1):(dist);
        else
          dest.at(r,c) = 0;

        //create the mask for the adaptation
        if(pa.adaptModel){
          if(dist >= pa.adaptationThreshold)
            distanceImg.at(r,c) = 255;
          else
             distanceImg.at(r,c) = 0;
        }
        // implements v1(transposed) * bgVariance * v1
        //dest.at(r,c) = v1.dot(bgVariance[r][c].multiply(v2));
      }
    }

    //adapt the backgroundmodel if desired
    if(pa.adaptModel)
      return adaptBackground(src,distanceImg);

    return true;
  }


  //add a image to the background summation system
  bool backgroundModel::addBackground(const image& src){

    int r,c;
    point big;
    std::vector<float> entry(3,0.0);

    if( bgStat.size()>0 ){
      big.y = bgStat.size();
      big.x = bgStat[0].size();
    }

    if( bgStat.size()>0 ){ //ther are already images stored
      //if the new image is larger or smaler as the saved images
      if( big!=src.size() ){
        setStatusString("The image do not fit to the images of the model.");
        return false;
      }
      for(r=0; r<src.rows(); r++){
        for(c=0; c<src.columns(); c++){
          entry[0] = src.at(r,c).getRed();
          entry[1] = src.at(r,c).getGreen();
          entry[2] = src.at(r,c).getBlue();

          bgStat[r][c].consider(entry);
        }
      }
    }
    else{//initialize the class

      //initialize the matrix of serial stats functors
      std::vector< std::vector<  serialVectorStats<float> > >::iterator its;
      bgStat.resize(src.rows());
      for(its=bgStat.begin(); its!=bgStat.end(); its++)
        (*its).resize(src.columns());

      //add the first pixels in the statistic functors
      for(r=0; r<src.rows(); r++){
        for(c=0; c<src.columns(); c++){
          entry[0] = src.at(r,c).getRed();
          entry[1] = src.at(r,c).getGreen();
          entry[2] = src.at(r,c).getBlue();

          bgStat[r][c].consider(entry);
        }
      }

    }//end of else
    return true;
  }

  bool backgroundModel::adaptBackground(const image& src, const channel8& mask){

    int r,c;
    point big;
    std::vector<float> entry(3,0.0);
    //get the parameters
    const parameters pa = getParameters();

    //check if the model exist
    if( bgStat.size()<1 ){
      setStatusString("You can not adapt an empty model.");
      return false;
    }

    //calculate the size of the images in the model
    big.y = bgStat.size(); big.x = bgStat[0].size();

    //if a new image is larger or smaler as the saved images
    if( big!=src.size() ||  big!=mask.size() ){
      setStatusString("The image do not fit to the images of the model ore the mask do not fit.");
      return false;
    }

    //adapt the model
    for(r=0; r<src.rows(); r++){
      for(c=0; c<src.columns(); c++){
        //update only the pixel not set in the mask
        if(mask.at(r,c) == 0){
          entry[0] = src.at(r,c).getRed();
          entry[1] = src.at(r,c).getGreen();
          entry[2] = src.at(r,c).getBlue();

          //normalize the entries in the model
          bgStat[r][c].setN((1.0f-pa.alpha)/pa.alpha);

          bgStat[r][c].consider(entry);
        }
      }
    }

    return true;
  }

  bool backgroundModel::getModel(image& model){

    //get the x and y size of the vector< vector< > > struct
    point big;
    big.y = bgStat.size(); big.x = (*bgStat.begin()).size();
    rgbPixel p;
    vector<float> v;
    int r,c;

    //resize the target image
    model.resize(big.y,big.x,rgbPixel(),false,true);

    for(r=0; r<model.rows(); r++){
      for(c=0; c<model.columns(); c++){
        bgStat[r][c].getMean(v);
        p.setRed(static_cast<ubyte>(v[0]));
        p.setGreen(static_cast<ubyte>(v[1]));
        p.setBlue(static_cast<ubyte>(v[2]));

        model.at(r,c) = p;
      }
    }

    return true;
  }

  //empty the statistic saving matrix
  bool backgroundModel::clearMoldel(){

    std::vector< std::vector<  serialVectorStats<float> > >::iterator its;
    //clear the columns vectors
    for(its=bgStat.begin(); its!=bgStat.end(); its++)
      (*its).clear();
    //clear the row vector
    bgStat.clear();

    return true;
  }

    /*
   * write the functor in the given ioHandler. The default implementation
   * is to write just the parameters object.
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  bool backgroundModel::write(ioHandler& handler,
                        const bool complete) const {
    bool b = true;

    if (complete) {
      b = handler.writeBegin();
    }

    b = b && lti::write(handler,"backgroundmodel",bgStat);

    if (complete) {
      b = handler.writeEnd();
    }

    return b;
  }

  /*
   * read the parameters from the given ioHandler. The default implementation
   * is to read just the parameters object.
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  bool backgroundModel::read(ioHandler& handler,const bool complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    b = b && lti::read(handler,"backgroundmodel",bgStat);

    if (complete) {
      b = handler.readEnd();
    }

    return b;
  }

}
