/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiGradientASM.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 1.3.2002
 * revisions ..: $Id: ltiGradientASM.cpp,v 1.8 2006/09/05 10:14:18 ltilib Exp $
 */

//TODO: include files
#include "ltiGradientASM.h"

namespace lti {
  // --------------------------------------------------
  // gradientASM::parameters
  // --------------------------------------------------

  // default constructor
  gradientASM::parameters::parameters()
    : activeShapeModel::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    iterations = 3;
    searchExtent = 5;
    gradientValueAsReliability = false;
    weightingKernel = useGaussKernel;
  }

  // copy constructor
  gradientASM::parameters::parameters(const parameters& other)
    : activeShapeModel::parameters()  {
    copy(other);
  }

  // destructor
  gradientASM::parameters::~parameters() {
  }

  // get type name
  const char* gradientASM::parameters::getTypeName() const {
    return "gradientASM::parameters";
  }

  // copy member

  gradientASM::parameters&
    gradientASM::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    activeShapeModel::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    activeShapeModel::parameters& (activeShapeModel::parameters::* p_copy)
      (const activeShapeModel::parameters&) =
      activeShapeModel::parameters::copy;
    (this->*p_copy)(other);
# endif


      iterations = other.iterations;
      searchExtent = other.searchExtent;
      gradientValueAsReliability = other.gradientValueAsReliability;
      weightingKernel = other.weightingKernel;

    return *this;
  }

  // alias for copy member
  gradientASM::parameters&
    gradientASM::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* gradientASM::parameters::clone() const {
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
  bool gradientASM::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool gradientASM::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"iterations",iterations);
      lti::write(handler,"searchExtent",searchExtent);
      lti::write(handler,"gradientValueAsReliability",gradientValueAsReliability);
      switch(weightingKernel) {
        case useNoKernel:
          lti::write(handler, "weightingKernel", "noKernel");
          break;
        case useGaussKernel:
          lti::write(handler, "weightingKernel", "gaussKernel");
          break;
        case useTriangularKernel:
        default:
          lti::write(handler, "weightingKernel", "triangularKernel");
          break;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && activeShapeModel::parameters::write(handler,false);
# else
    bool (activeShapeModel::parameters::* p_writeMS)(ioHandler&,const bool) const =
      activeShapeModel::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool gradientASM::parameters::write(ioHandler& handler,
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
  bool gradientASM::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool gradientASM::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"iterations",iterations);
      lti::read(handler,"searchExtent",searchExtent);
      lti::read(handler,"gradientValueAsReliability",gradientValueAsReliability);
      std::string tempStr;
      lti::read(handler,"weightingKernel", tempStr);
      if (tempStr == "noKernel")
        weightingKernel = useNoKernel;
      else if (tempStr == "gaussKernel")
        weightingKernel = useGaussKernel;
      else if (tempStr == "triangularKernel")
        weightingKernel = useTriangularKernel;
      else {
        weightingKernel = useGaussKernel;
        b = false;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && activeShapeModel::parameters::read(handler,false);
# else
    bool (activeShapeModel::parameters::* p_readMS)(ioHandler&,const bool) =
      activeShapeModel::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool gradientASM::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // gradientASM
  // --------------------------------------------------

  // default constructor
  gradientASM::gradientASM()
    : activeShapeModel(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  gradientASM::gradientASM(const gradientASM& other)
    : activeShapeModel()  {
    copy(other);
  }

  // destructor
  gradientASM::~gradientASM() {
  }

  // returns the name of this type
  const char* gradientASM::getTypeName() const {
    return "gradientASM";
  }

  // copy member
  gradientASM&
    gradientASM::copy(const gradientASM& other) {
      activeShapeModel::copy(other);

    return (*this);
  }

  // alias for copy member
  gradientASM&
    gradientASM::operator=(const gradientASM& other) {
    return (copy(other));
  }


  // clone member
  functor* gradientASM::clone() const {
    return new gradientASM(*this);
  }

  // return parameters
  const gradientASM::parameters&
    gradientASM::getParameters() const {
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

  // align shape iteratively on the given gradient channel.
  pointDistributionModel::shape& gradientASM::apply(pointDistributionModel::shape& srcdest, const channel &gradientChannel) const {
    bool gradientValueAsReliability = getParameters().gradientValueAsReliability;
    fvector pointReliability;

    for (int it = getParameters().iterations; it>0; --it) {
      // move points to best image positions, obtain pointReliability
      adjustShape(srcdest, gradientChannel, pointReliability);

      // trim shape to make it a valid active shape model
      if (gradientValueAsReliability)
        trimShape(srcdest, pointReliability);
      else
        trimShape(srcdest);
    }

    return srcdest;
  }




  // adjust shape on the given gradient channel.
  // gradient values will be written to the vector pointReliability
  pointDistributionModel::shape& gradientASM::adjustShape(pointDistributionModel::shape &srcdest,
                                                          const channel &gradientChannel,
                                                          fvector &pointReliability) const {

    int searchExtent = getParameters().searchExtent;
    bool gradientValueAsReliability = getParameters().gradientValueAsReliability;

    lti::point channelSize = gradientChannel.size();
    lti::point tempPoint;

    // reliability values
    pointReliability.resize(srcdest.size());
    pointReliability.fill(1.0f);

    // weighting function
    kernel1D<float> theKernel;
    bool useFilter = createWeightingKernel(theKernel);

    // collect normals
    tpointList<float> listOfNormals;
    createListOfNormals(srcdest, listOfNormals);

    // intensity values will be saved here
    array<float> intensity(-searchExtent, searchExtent, 0.0f);

    // iterate on all points
    tpointList<float>::const_iterator normalsIt = listOfNormals.begin();
    pointDistributionModel::shape::iterator it = srcdest.begin();

    for (int pointNr = 0; pointNr < srcdest.size(); ++it, ++pointNr, ++normalsIt) {

      // null-segment following: leave point where it is
      if ((*normalsIt) != tpoint<float>(0.0, 0.0)) {

        // obtain intensities from channel
        getIntensityArray(*it, *normalsIt, gradientChannel, intensity);

        // weigh intensities
        if (useFilter)
          intensity.emultiply(theKernel);

        // search for maximum intensity
        int bestIndex = 0;
        for (int index = 1; index <= searchExtent; ++index) {
          if (intensity[index] > intensity[bestIndex])
            bestIndex = index;
          if (intensity[-index] > intensity[bestIndex])
            bestIndex = -index;
        }

        // modify current point
        (*it).castFrom(roundPoint((*it) + (*normalsIt) * static_cast<float>(bestIndex)));
      }

      // save reliability of this point
      if (gradientValueAsReliability) {
        tempPoint = roundPoint(*it);
        if (tempPoint.x >= 0 && tempPoint.y >= 0 && tempPoint.x < channelSize.x && tempPoint.y < channelSize.y)
          pointReliability[pointNr] = gradientChannel.at(roundPoint(*it));
        else
          pointReliability[pointNr] = 0.0f;
      }
    }

    return srcdest;
  }




  //
  // round tpoint<float> and convert to tpoint<int>
  //
  tpoint<int> gradientASM::roundPoint(const tpoint<float> &floatPoint) const {
    return tpoint<int>(static_cast<int>(round(floatPoint.x)),
                       static_cast<int>(round(floatPoint.y)));
  }




  //
  // create weighting function
  //
  bool gradientASM::createWeightingKernel(kernel1D<float> &theKernel) const {

    int searchExtent = getParameters().searchExtent;

    theKernel.resize(-searchExtent, searchExtent, 1.0f);

    gaussKernel1D<float> theGaussKernel(2*searchExtent + 1);
    triangularKernel1D<float> theTriangularKernel(2*searchExtent + 1);

    // decide which weighting function to use
    switch(getParameters().weightingKernel) {
    case parameters::useNoKernel:
      return false;
      break;

    case parameters::useGaussKernel:
      theKernel = theGaussKernel;
      break;

    case parameters::useTriangularKernel:
      theKernel = theTriangularKernel;
      break;

    default:
      throw exception("unknown weighting function");
      break;
    }

    //  multiply kernel, so middle value is 1.0
    theKernel.divide(theKernel.at(0));
    return true;
  }




  //
  // get intensity array for a specific starting point, normal and given borders
  //
  void gradientASM::getIntensityArray(const tpoint<float> &start, const tpoint<float> &normal, const channel &chnl, array<float> &values) const {
    tpoint<int> newPoint;

    int width = chnl.columns();
    int height = chnl.rows();
    int lastIdx = values.lastIdx();

    for (int index = values.firstIdx(); index <= lastIdx; ++index) {
      newPoint = roundPoint(start + normal * static_cast<float>(index));
      if (newPoint.x >= 0 && newPoint.y >= 0 && newPoint.x < width && newPoint.y < height)
        values[index] = chnl.at(newPoint);
      else
        values[index] = 0.0f;
    }
  }
}
