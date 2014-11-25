/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiPdmGenerator.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 26.4.2001
 * revisions ..: $Id: ltiPdmGenerator.cpp,v 1.8 2006/09/05 10:24:59 ltilib Exp $
 */

//TODO: include files
#include "ltiPdmGenerator.h"

namespace lti {
  // --------------------------------------------------
  // pdmGenerator::parameters
  // --------------------------------------------------

  // default constructor
  pdmGenerator::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    maxNumberOfWeights = 0;
    maxRelativeWeights = 1.0f;
  }

  // copy constructor
  pdmGenerator::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  pdmGenerator::parameters::~parameters() {
  }

  // get type name
  const char* pdmGenerator::parameters::getTypeName() const {
    return "pdmGenerator::parameters";
  }

  // copy member

  pdmGenerator::parameters&
    pdmGenerator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif


      maxNumberOfWeights = other.maxNumberOfWeights;
      maxRelativeWeights = other.maxRelativeWeights;

    return *this;
  }

  // alias for copy member
  pdmGenerator::parameters&
    pdmGenerator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* pdmGenerator::parameters::clone() const {
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
  bool pdmGenerator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool pdmGenerator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"maxNumberOfWeights",maxNumberOfWeights);
      lti::write(handler,"maxRelativeWeights",maxRelativeWeights);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool pdmGenerator::parameters::write(ioHandler& handler,
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
  bool pdmGenerator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool pdmGenerator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"maxNumberOfWeights",maxNumberOfWeights);
      lti::read(handler,"maxRelativeWeights",maxRelativeWeights);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool pdmGenerator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // pdmGenerator
  // --------------------------------------------------

  // default constructor
  pdmGenerator::pdmGenerator()
    : functor(){
    clear();
  }

  // copy constructor
  pdmGenerator::pdmGenerator(const pdmGenerator& other)
    : functor()  {
    copy(other);
  }

  // destructor
  pdmGenerator::~pdmGenerator() {
  }

  // returns the name of this type
  const char* pdmGenerator::getTypeName() const {
    return "pdmGenerator";
  }

  // copy member
  pdmGenerator& pdmGenerator::copy(const pdmGenerator& other) {
    functor::copy(other);
    return (*this);
  }

  // clone member
  functor* pdmGenerator::clone() const {
    return new pdmGenerator(*this);
  }

  // return parameters
  const pdmGenerator::parameters&
    pdmGenerator::getParameters() const {
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
   * generate a pointDistributionModel given a set of shapes
   */
  void pdmGenerator::generate(pointDistributionModel &theModel) {

    pointDistributionModel::shape meanShape = alignShapes();

    matrix<double> covarMatrix = calculateCovarianceMatrix(meanShape);

    matrix<double> eigenVectorMatrix;
    vector<double> eigenVector;

    createEigenSystem(covarMatrix, eigenVectorMatrix, eigenVector);

    theModel.eigenVectorMatrix = eigenVectorMatrix;
    for (int i=0; i < eigenVector.size(); i++) {
      eigenVector[i] = sqrt(abs(eigenVector[i]));
    }
    theModel.varianceVector = eigenVector;
    theModel.meanShape = meanShape;
  }


  /*
   * generate pointDistributionModel
   */
  pointDistributionModel pdmGenerator::apply() {

    if (shapes.size() < 2)
      throw exception("more than one shape needed");

    pointDistributionModel theModel;
    generate(theModel);

    vector<double> eigenValues = theModel.varianceVector;
    eigenValues.emultiply(eigenValues);

    double absoluteIndex = eigenValues.sumOfElements();

    float relativeMax = getParameters().maxRelativeWeights;
    int dimensionCount = getParameters().maxNumberOfWeights * 2;
    if (dimensionCount <= 0 || dimensionCount > eigenValues.size())
      dimensionCount = eigenValues.size();

    double relativeIndex = 0.0;

    for (int index = 0; index < dimensionCount; ++index) {
      relativeIndex += eigenValues[index];

      if (relativeIndex > absoluteIndex * relativeMax) {
        dimensionCount = index + 1;
        break;
      }
    }

    // get next even index
    dimensionCount += (dimensionCount % 2);

    theModel.eigenVectorMatrix.resize(theModel.eigenVectorMatrix.rows(), dimensionCount);
    theModel.varianceVector.resize(dimensionCount);

    return theModel;
  }


  //
  // take shape into consideration
  //
  void pdmGenerator::consider(const pointDistributionModel::shape &theShape) {
    if (shapeSize == 0)
      shapeSize = theShape.size();

    if (theShape.size() != shapeSize)
      throw exception("shapes differ in size");

    shapes.push_back(theShape);
  }

  //
  // take list of shapes into consideration
  //
  void pdmGenerator::consider(const std::list<pointDistributionModel::shape> &theShapes) {
    std::list<pointDistributionModel::shape>::const_iterator it;
    for (it = theShapes.begin(); !(it == theShapes.end()); ++it)
      consider(*it);
  }

  //
  // return number of elements considered so far
  //
  int pdmGenerator::getN() const {
    return shapes.size();
  }

  //
  // clear all considered shapes
  //
  void pdmGenerator::clear() {
    shapes.clear();
    shapeSize = 0;
  }




  //
  // private methods
  //

  /*
   * - align all shapes to cog=(0,0), meanDistance=1.0
   * - create mean shape
   * - align all shapes iteratively to the mean shape
   */
  pointDistributionModel::shape pdmGenerator::alignShapes() {

    if (shapes.size() == 0)
      return pointDistributionModel::shape();

    std::list<pointDistributionModel::shape>::iterator it = shapes.begin();

    // normalize first shape
    normalizeShape(*it);

    // align all shapes to the first (1)
    while (! (++it == shapes.end()))
      theASM.alignShape(*it, shapes.front());

    // calculate (normalized) mean shape (2)
    pointDistributionModel::shape meanShape;
    pointDistributionModel::shape newMeanShape = createMeanShape();
    normalizeShape(newMeanShape);

    float difference = 0;
    float newDifference = 0;  // avoid "not initialized" warning
    int run = 0;

    // while meanShape converges:
    do {
      meanShape = newMeanShape;

      // align mean shape to first (3)
      theASM.alignShape(meanShape, shapes.front());

      // align all shapes except first to mean shape (4)
      it = shapes.begin();
      while (!(++it == shapes.end()))
        theASM.alignShape(*it, meanShape);

      // calculate (normalized) mean shape (5a)
      newMeanShape = createMeanShape();
      normalizeShape(newMeanShape);

      // calculate difference (5b, least squares approach)
      difference = newDifference;
      newDifference = theASM.calculateShapeDifference(meanShape, newMeanShape);
      run++;

    } while ((newDifference < difference) || (run < 2));    // at least two measurements are needed!

    return meanShape;

  }


  /*
   * create mean shape
   */
  pointDistributionModel::shape pdmGenerator::createMeanShape() const {

    pointDistributionModel::shape newShape;
    for (int i = 0; i < shapeSize; i++)
      newShape.push_back(tpoint<float>(0.0, 0.0));

    pointDistributionModel::shape::iterator thisIt;
    std::list<pointDistributionModel::shape>::const_iterator shapesIt;

    for (shapesIt = shapes.begin(); !(shapesIt == shapes.end()); ++shapesIt) {

      pointDistributionModel::shape::const_iterator refIt = (*shapesIt).begin();
      for (thisIt = newShape.begin(); !(thisIt == newShape.end()); ++thisIt, ++refIt)
        (*thisIt) += (*refIt);
    }

    return newShape;
  }


  /*
   * sum of outerproducts of shape-vectors divided by number of shapes
   * (i.e. [v]*(v), where [v] represents the vector in a row and (v) represents the vector in a column
   */
  matrix<double> pdmGenerator::calculateCovarianceMatrix(const pointDistributionModel::shape &meanShape) const {

    std::list<pointDistributionModel::shape>::const_iterator shapesIt = shapes.begin();

    vector<double> meanVector = buildVector(meanShape);
    matrix<double> theMatrix;
    matrix<double> covarMatrix(meanVector.size(), meanVector.size());

    while (! (shapesIt == shapes.end())) {
      vector<double> theVector = buildVector(*shapesIt);
      theVector.subtract(meanVector);
      theMatrix.outerProduct(theVector, theVector);
      covarMatrix.add(theMatrix);
      shapesIt++;
    }

    covarMatrix.multiply(1.0 / shapes.size());

    return covarMatrix;
  }


  /*
   * create eigenmatrix and vector of eigenvalues
   */
  void pdmGenerator::createEigenSystem(const matrix<double> &covarMatrix,
                                       matrix<double> &eigenVectorMatrix,
                                       vector<double> &eigenValues) const {

    jacobi<double> theEigenSystem;

    jacobi<double>::parameters theParameters;
    theParameters.sort = true;
    theEigenSystem.setParameters(theParameters);
    theEigenSystem.apply(covarMatrix, eigenValues, eigenVectorMatrix);

  }


  /*
   * create shape vector (x1, y1, x2, y2, ...)
   */
  vector<double> pdmGenerator::buildVector(const pointDistributionModel::shape &theShape) const {

    int size = theShape.size() * 2;
    vector<double> vectorRepresentation(size);

    tpointList<float>::const_iterator vIt = theShape.begin();

    int index = 0;

    while (index < size) {
      vectorRepresentation[index++] = (*vIt).x;
      vectorRepresentation[index++] = (*vIt).y;
      ++vIt;
    }

    return vectorRepresentation;
  }

  //
  // normalize shape (i.e. move to (0,0), scale to 1.0)
  //
  void pdmGenerator::normalizeShape(pointDistributionModel::shape &theShape) const {

    geometricTransform transformation;
    geometricTransform::parameters trafoParams;

    // shift shape to (0,0)
    tpoint<float> shapeCOG = theASM.calculateShapeCenter(theShape);
    tpoint<double> location;
    location.castFrom(shapeCOG);
    trafoParams.shift(location * (-1.0f));

    // scale shape to size 1.0
    trafoParams.scale(1.0f / theASM.calculateShapeDistance(theShape, shapeCOG));
    transformation.setParameters(trafoParams);
    transformation.apply(theShape);
  }

} // namespace lti
