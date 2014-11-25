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
 * file .......: ltiSkinASM.cpp
 * authors ....: Holger Fillbrandt, Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 1.3.2002
 * revisions ..: $Id: ltiSkinASM.cpp,v 1.8 2006/09/05 10:31:24 ltilib Exp $
 */

//TODO: include files
#include "ltiSkinASM.h"

namespace lti {
  // --------------------------------------------------
  // skinASM::parameters
  // --------------------------------------------------

  // default constructor
  skinASM::parameters::parameters()
    : gradientASM::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    maxInsideSkin = 0.7f;
    shrinkExtent = 15;
    enlargeExtent = 30;
    skinThreshold = 0.5f;
    onlySkinBordersReliable = true;
  }

  // copy constructor
  skinASM::parameters::parameters(const parameters& other)
    : gradientASM::parameters()  {
    copy(other);
  }

  // destructor
  skinASM::parameters::~parameters() {
  }

  // get type name
  const char* skinASM::parameters::getTypeName() const {
    return "skinASM::parameters";
  }

  // copy member

  skinASM::parameters&
    skinASM::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    gradientASM::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    gradientASM::parameters& (gradientASM::parameters::* p_copy)
      (const gradientASM::parameters&) =
      gradientASM::parameters::copy;
    (this->*p_copy)(other);
# endif


      enlargeExtent = other.enlargeExtent;
      shrinkExtent = other.shrinkExtent;
      maxInsideSkin = other.maxInsideSkin;
      skinThreshold = other.skinThreshold;
      onlySkinBordersReliable = other.onlySkinBordersReliable;

    return *this;
  }

  // alias for copy member
  skinASM::parameters&
    skinASM::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* skinASM::parameters::clone() const {
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
  bool skinASM::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool skinASM::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"enlargeExtent",enlargeExtent);
      lti::write(handler,"shrinkExtent",shrinkExtent);
      lti::write(handler,"maxInsideSkin",maxInsideSkin);
      lti::write(handler,"skinThreshold",skinThreshold);
      lti::write(handler,"onlySkinBordersReliable",onlySkinBordersReliable);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && gradientASM::parameters::write(handler,false);
# else
    bool (gradientASM::parameters::* p_writeMS)(ioHandler&,const bool) const =
      gradientASM::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool skinASM::parameters::write(ioHandler& handler,
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
  bool skinASM::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool skinASM::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"enlargeExtent",enlargeExtent);
      lti::read(handler,"shrinkExtent",shrinkExtent);
      lti::read(handler,"maxInsideSkin",maxInsideSkin);
      lti::read(handler,"skinThreshold",skinThreshold);
      lti::read(handler,"onlySkinBordersReliable",onlySkinBordersReliable);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && gradientASM::parameters::read(handler,false);
# else
    bool (gradientASM::parameters::* p_readMS)(ioHandler&,const bool) =
      gradientASM::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool skinASM::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // skinASM
  // --------------------------------------------------

  // default constructor
  skinASM::skinASM()
    : gradientASM(){

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
  skinASM::skinASM(const skinASM& other)
    : gradientASM()  {
    copy(other);
  }

  // destructor
  skinASM::~skinASM() {
  }

  // returns the name of this type
  const char* skinASM::getTypeName() const {
    return "skinASM";
  }

  // copy member
  skinASM& skinASM::copy(const skinASM& other) {
    gradientASM::copy(other);

    return (*this);
  }

  // alias for copy member
  skinASM&
    skinASM::operator=(const skinASM& other) {
    return (copy(other));
  }


  // clone member
  functor* skinASM::clone() const {
    return new skinASM(*this);
  }

  // return parameters
  const skinASM::parameters&
    skinASM::getParameters() const {
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

  // align shape iteratively on the given gradient and skin probability channels.
  pointDistributionModel::shape& skinASM::apply(pointDistributionModel::shape& srcdest, const channel &gradientChannel, const channel &skinProbChannel) const {
    fvector pointReliability;

    for (int it = getParameters().iterations; it>0; --it) {
      // move points to best image positions, obtain pointReliability
      adjustShape(srcdest, gradientChannel, skinProbChannel, pointReliability);

      // trim shape to make it a valid active shape model
      trimShape(srcdest, pointReliability);
    }

    return srcdest;
  }


  //
  // iteratively align shape on the given gradient and skin probality channels.
  //
  pointDistributionModel::shape& skinASM::adjustShape(pointDistributionModel::shape &srcdest,
                                               const channel &gradientChannel,
                                               const channel &skinProbChannel,
                                               fvector &pointReliability) const {

    if (!(gradientChannel.size() == skinProbChannel.size()))
      throw exception("channels differ in size");

    pointReliability.resize(srcdest.size());
    pointReliability.fill(0.0f);

    // copy parameter values:
    int searchExtent = getParameters().searchExtent;
    int shrinkExtent = getParameters().shrinkExtent;
    int enlargeExtent = getParameters().enlargeExtent;
    float skinThreshold = getParameters().skinThreshold;

    bool onlySkinBordersReliable = getParameters().onlySkinBordersReliable;
    bool gradientValueAsReliability = getParameters().gradientValueAsReliability;

    // collect normals of all points of the shape
    tpointList<float> listOfNormals;
    createListOfNormals(srcdest, listOfNormals);

    // prepare weighting function for gradient values
    kernel1D<float> theKernel;
    bool useFilter = createWeightingKernel(theKernel);

    //
    // move points along normals to best position
    //

    // do normals lie completely on skin?
    std::vector<bool> inSkin(srcdest.size(), false);
    int pointsInsideSkin = 0;

    // gradients from -searchExtent to +searchExtent will be saved here.
    array<float> gradientValues(-searchExtent, searchExtent, 0.0f);

    // skin values from -shrinkExtent to +searchExtent will be saved here.
    array<float> skinValues(-max(searchExtent, shrinkExtent), searchExtent, 0.0f);

    int pointNr = 0;
    tpointList<float>::const_iterator normalsIt = listOfNormals.begin();
    pointDistributionModel::shape::iterator it;

    for (it = srcdest.begin(); !(it == srcdest.end()); ++it, ++normalsIt, ++pointNr) {

      // if point has valid normal, look for better point along normal
      if ((*normalsIt)!=tpoint<float>(0.0f, 0.0f)) {

        //
        // step 1: collect gradient and skin probability values along normal
        //

        getIntensityArray(*it, *normalsIt, gradientChannel, gradientValues);
        getIntensityArray(*it, *normalsIt, skinProbChannel, skinValues);

        // weigh gradient values:
        if (useFilter)
          gradientValues.emultiply(theKernel);

        //
        // step 2: look for best valid points, i.e. points where only skin is inside
        //

        int bestIndex = 0;
        inSkin[pointNr] = true;

        // search for best gradient point with highest gradient value and only skin inside
        int index;
        for (index = -searchExtent; index <= searchExtent; ++index) {
          if (skinValues[index] < skinThreshold) {
            inSkin[pointNr] = false;

            break;
          }
          if (gradientValues[index] >= gradientValues[bestIndex] || index == -searchExtent)
            bestIndex = index;

          pointReliability[pointNr] = 1.0f;
        }

        //
        // step 3: no valid points found: look further inside
        //

        // no skin region inside (first pixel non-skin)
        if (skinValues[-searchExtent] < skinThreshold) {

          // try shrinking (i.e., look further inside for first skin point)
          for (index = -searchExtent; index >= -shrinkExtent; --index) {
            if (skinValues[index] >= skinThreshold) {
              bestIndex = index;                 // first skin point
              pointReliability[pointNr] = 1.0f;

              break;
            }
          }
        }

        // point lies completely within skin region
        if (inSkin[pointNr]) {
          bestIndex = 0;
          pointReliability[pointNr] = 0.0f;
          ++pointsInsideSkin;
        }

        // move point
        (*it).castFrom(roundPoint((*it) + (*normalsIt) * static_cast<float>(bestIndex)));
      }



      // all points equally reliable?
      if (!onlySkinBordersReliable)
        pointReliability[pointNr] = 1.0f;
    }

    //
    // step 4: if too many points are inside skin regions, look further outside
    //

    if (pointsInsideSkin > srcdest.size() * getParameters().maxInsideSkin) {

      // now collect skin values from +searchExtent to +enlargeExtent.
      skinValues.resize(searchExtent, enlargeExtent, 0.0f);

      pointNr = 0;
      normalsIt = listOfNormals.begin();
      for (it = srcdest.begin(); !(it == srcdest.end()); ++it, ++normalsIt, ++pointNr) {

        // move point only, when completely in skin
        if (inSkin[pointNr]) {

          // fetch skin probability values
          getIntensityArray(*it, *normalsIt, skinProbChannel, skinValues);

          int bestIndex = 0;

          // look futher outside for first non-skin point
          for (int index = searchExtent; index <= enlargeExtent; ++index) {
            if (skinValues[index] < skinThreshold) {
              bestIndex = index - 1;             // last point was skin border

              pointReliability[pointNr] = 1.0f;
              break;
            }
          }

          // move current point
          (*it).castFrom((*it) + (*normalsIt) * static_cast<float>(bestIndex));
        }
      }
    }

    //
    // step 5: adjust reliability values, if necessary
    //

    if (gradientValueAsReliability) {
      pointNr = 0;
      point channelSize = gradientChannel.size();
      point newPoint;

      for (it = srcdest.begin(); !(it == srcdest.end()); ++it, ++pointNr) {
        newPoint = roundPoint(*it);
        if (newPoint.x >= 0 && newPoint.y >= 0 && newPoint.x < channelSize.x && newPoint.y < channelSize.y)
          pointReliability[pointNr] *= gradientChannel.at(newPoint);
      }
    }

    return srcdest;
  }

}
