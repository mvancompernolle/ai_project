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
 * file .......: ltiActiveShapeModel.cpp
 * authors ....: Benjamin Winkler, Holger Fillbrandt
 * organization: LTI, RWTH Aachen
 * creation ...: 1.3.2002
 * revisions ..: $Id: ltiActiveShapeModel.cpp,v 1.10 2006/09/05 10:02:12 ltilib Exp $
 */

#include "ltiActiveShapeModel.h"

namespace lti {
  // --------------------------------------------------
  // activeShapeModel::parameters
  // --------------------------------------------------

  // default constructor
  activeShapeModel::parameters::parameters()
    : functor::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    varianceCoefficient = 3.0;
    model = pointDistributionModel();
    reliabilityAsWeightingFactor = false;
  }

  // copy constructor
  activeShapeModel::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  activeShapeModel::parameters::~parameters() {
  }

  // get type name
  const char* activeShapeModel::parameters::getTypeName() const {
    return "activeShapeModel::parameters";
  }

  // copy member

  activeShapeModel::parameters&
    activeShapeModel::parameters::copy(const parameters& other) {
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


      varianceCoefficient = other.varianceCoefficient;
      model = other.model;
      reliabilityAsWeightingFactor = other.reliabilityAsWeightingFactor;

    return *this;
  }

  // alias for copy member
  activeShapeModel::parameters&
    activeShapeModel::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* activeShapeModel::parameters::clone() const {
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
  bool activeShapeModel::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool activeShapeModel::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"varianceCoefficient",varianceCoefficient);
      lti::write(handler,"model",model);
      lti::write(handler,"reliabilityAsWeightingFactor",reliabilityAsWeightingFactor);
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
  bool activeShapeModel::parameters::write(ioHandler& handler,
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
  bool activeShapeModel::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool activeShapeModel::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"varianceCoefficient",varianceCoefficient);
      lti::read(handler,"model",model);
      lti::read(handler,"reliabilityAsWeightingFactor",reliabilityAsWeightingFactor);
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
  bool activeShapeModel::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // activeShapeModel
  // --------------------------------------------------

  // default constructor
  activeShapeModel::activeShapeModel()
    : functor(){

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
  activeShapeModel::activeShapeModel(const activeShapeModel& other)
    : functor()  {
    copy(other);
  }

  // destructor
  activeShapeModel::~activeShapeModel() {
  }

  // returns the name of this type
  const char* activeShapeModel::getTypeName() const {
    return "activeShapeModel";
  }

  // copy member
  activeShapeModel& activeShapeModel::copy(const activeShapeModel& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  activeShapeModel&
    activeShapeModel::operator=(const activeShapeModel& other) {
    return (copy(other));
  }


  // clone member
  functor* activeShapeModel::clone() const {
    return new activeShapeModel(*this);
  }

  // return parameters
  const activeShapeModel::parameters&
    activeShapeModel::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // convert a shape [(x1,y1), (x2,y2), ...] to a vector [x1, y1, x2, y2, ...]
  vector<double> activeShapeModel::buildVector(const pointDistributionModel::shape &src) const {
    vector<double> returnVector(src.size() * 2);

    pointDistributionModel::shape::const_iterator it = src.begin();
    for (int i=0; i < src.size(); ++i, ++it) {
      returnVector[2*i] = (*it).x;
      returnVector[2*i+1] = (*it).y;
    }

    return returnVector;
  }

  // convert a vector [x1, y1, x2, y2, ...] into a shape [(x1,y1), (x2,y2), ...]
  pointDistributionModel::shape activeShapeModel::buildShape(const vector<double> &src) const {
    pointDistributionModel::shape returnShape;

    for (int i=0; i < src.size(); i+=2)
      returnShape.push_back(tpoint<float>(static_cast<float>(src[i]),
                                          static_cast<float>(src[i+1])));

    return returnShape;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // trim shape
  void activeShapeModel::trimShape(pointDistributionModel::shape& srcdest) const {

    trimShape(srcdest, fvector(srcdest.size(), 1.0f));
  }




  // trim shape
  void activeShapeModel::trimShape(pointDistributionModel::shape& srcdest, const fvector &pointReliability) const {

    if (pointReliability.size() != srcdest.size())
      throw exception("invalid reliability vector");

    // align shape with meanShape
    geometricTransform::parameters trafoParameters = alignShape(srcdest, getParameters().model.meanShape, pointReliability);

    // truncate weights of the transformed shape
    vector<double> temp;
    if (getParameters().reliabilityAsWeightingFactor)
      // move unreliable points towards mean shape
      convertShapeToWeightsVector(srcdest, temp, pointReliability);
    else
      convertShapeToWeightsVector(srcdest, temp);

    trimWeightsVector(temp);
    convertWeightsVectorToShape(temp, srcdest);

    // retransform trimmed shape
    geometricTransform transformation;
    transformation.setParameters(trafoParameters);
    transformation.apply(srcdest);
  }




  // trim weights according to given variance coefficient
  void activeShapeModel::trimWeightsVector(vector<double>& srcdest) const {

    const vector<double> &variances = getParameters().model.varianceVector;
    double coeff = getParameters().varianceCoefficient;

    // cut srcdest if too big; fill with 0.0 if too small:
    srcdest.resize(variances.size(), 0.0f);

    // valid values range from -c*var to c*var
    for (int i=0; i < srcdest.size(); ++i)
      if (abs(srcdest[i]) > coeff * variances[i])
        srcdest[i] = 0.0;
  }




  // convert a shape to a weights vector, i.e. project a shape into the pdm space
  void activeShapeModel::convertShapeToWeightsVector(const pointDistributionModel::shape &src, vector<double> &dest) const {

    vector<double> vec = buildVector(src).subtract(buildVector(getParameters().model.meanShape));
    matrix<double> mat = getParameters().model.eigenVectorMatrix;
    dest = (mat.transpose()).multiply(vec);
  }




  // convert a shape to a weights vector and move unreliable points towards mean shape
  void activeShapeModel::convertShapeToWeightsVector(const pointDistributionModel::shape &src, vector<double> &dest, const fvector &pointReliability) const {

    if (pointReliability.size() != src.size())
      throw exception("invalid reliability vector");

    vector<double> vec = buildVector(src).subtract(buildVector(getParameters().model.meanShape));

    // move features towards mean shape
    for (int i=0; i<pointReliability.size(); ++i) {
      vec[2*i] *= pointReliability[i];
      vec[2*i+1] *= pointReliability[i];
    }

    matrix<double> mat = getParameters().model.eigenVectorMatrix;
    dest = (mat.transpose()).multiply(vec);
  }




  // convert a parameter vector into a shape
  void activeShapeModel::convertWeightsVectorToShape(const vector<double> &src, pointDistributionModel::shape &dest) const {

    vector<double> vec = src;
    vec = ((getParameters().model.eigenVectorMatrix).multiply(vec)).add(buildVector(getParameters().model.meanShape));
    dest = buildShape(vec);
  }




  // calculate the center of gravity of a shape
  tpoint<float> activeShapeModel::calculateShapeCenter(const pointDistributionModel::shape &shape) const {

    return calculateShapeCenter(shape, fvector(shape.size(), 1.0f));
  }




  // calculate the center of gravity of a weighted shape
  tpoint<float> activeShapeModel::calculateShapeCenter(const pointDistributionModel::shape &shape, const fvector &pointReliability) const {

    if (pointReliability.size() != shape.size())
      throw exception("invalid reliability vector");


    pointDistributionModel::shape::const_iterator it = shape.begin();

    tpoint<float> center(0.0, 0.0);
    for (int i=0; i<shape.size(); ++i, ++it)
      center += (*it) * pointReliability[i];;

    center /= pointReliability.sumOfElements();
    return center;
  }




  // calculate the medium distance of a given point to the points of a shape
  float activeShapeModel::calculateShapeDistance(const pointDistributionModel::shape &shape, const tpoint<float> &thePoint) const {

    return calculateShapeDistance(shape, fvector(shape.size(), 1.0f), thePoint);
  }




  // calculate the medium distance of a given point to the points of a weighted shape
  float activeShapeModel::calculateShapeDistance(const pointDistributionModel::shape &shape,
                                                 const fvector &pointReliability,
                                                 const tpoint<float> &thePoint) const {

    if (pointReliability.size() != shape.size())
      throw exception("invalid reliability vector");

    float sumOfElements = pointReliability.sumOfElements();
    if (sumOfElements == 0.0f)
      return 1.0f;

    float distance = 0.0;
//    tpoint<float> delta;

    pointDistributionModel::shape::const_iterator it = shape.begin();
    for (int i=0; i<shape.size(); ++i, ++it)
      distance += thePoint.distanceTo(*it) * pointReliability[i];

    distance /= pointReliability.sumOfElements();
    return distance;
  }




  // calculate the relative orientation of two shapes and a rotation center
  double activeShapeModel::calculateShapeOrientation(const pointDistributionModel::shape &shape,
                                                    const pointDistributionModel::shape &referenceShape,
                                                    const tpoint<float> &theCenter) const {

    return calculateShapeOrientation(shape, referenceShape, fvector(shape.size(), 1.0f), theCenter);
  }




  // calculate the relative orientation of two shapes and a rotation center
  double activeShapeModel::calculateShapeOrientation(const pointDistributionModel::shape &shape,
                                                    const pointDistributionModel::shape &referenceShape,
                                                    const fvector &pointReliability,
                                                    const tpoint<float> &theCenter) const {
    if (shape.size() != referenceShape.size())
      throw exception("shapes have different sizes");

    if (pointReliability.size() != shape.size())
      throw exception("invalid reliability vector");

    // calculate rotation
    double theRotation = 0.0;
    double relativeAngle;

    pointDistributionModel::shape::const_iterator shapeIt = shape.begin();
    pointDistributionModel::shape::const_iterator refShapeIt = referenceShape.begin();

    // accumulate relative angles of corresponding points
    for (int i=0; i<shape.size(); ++i, ++shapeIt, ++refShapeIt) {
      relativeAngle = calculateAngle((*refShapeIt) - theCenter) - calculateAngle((*shapeIt) - theCenter);

      if (relativeAngle > 3.14159265)
        relativeAngle -= 6.2831853;
      else if (relativeAngle < -3.14159265)
        relativeAngle += 6.2831853;

      // weigh angle
      theRotation += relativeAngle * pointReliability[i];
    }

    // average rotation of a point
    theRotation /= pointReliability.sumOfElements();

    if (theRotation > 3.14159265)
      theRotation -= 6.2831853;
    else if (theRotation < -3.14159265)
      theRotation += 6.2831853;

    return theRotation;
  }




  //
  // calculate weighted scaling factor
  //
  float activeShapeModel::calculateRelativeShapeScale(const pointDistributionModel::shape &shape,
                                    const pointDistributionModel::shape &referenceShape,
                                    const fvector &pointReliability,
                                    const tpoint<float> &shapeCenter,
                                    const tpoint<float> &referenceShapeCenter) const {

    if (shape.size() != referenceShape.size())
      throw exception("shapes have different sizes");

    if (pointReliability.size() != shape.size())
      throw exception("invalid reliability vector");

    pointDistributionModel::shape::const_iterator shapeIt = shape.begin();
    pointDistributionModel::shape::const_iterator referenceShapeIt = referenceShape.begin();

    float shapeDist, referenceShapeDist;

    float relativeScale = 0.0f;
    for (int i=0; i<shape.size(); ++i, ++shapeIt, ++referenceShapeIt) {

      shapeDist = shapeCenter.distanceTo(*shapeIt);
      referenceShapeDist = referenceShapeCenter.distanceTo(*referenceShapeIt);

      relativeScale += (shapeDist / referenceShapeDist) * pointReliability[i];
    }

    return relativeScale / pointReliability.sumOfElements();
  }




  // align a given shape to another
  geometricTransform::parameters activeShapeModel::alignShape(pointDistributionModel::shape &shape,
                                                              const pointDistributionModel::shape &referenceShape) const {

    return alignShape(shape, referenceShape, fvector(shape.size(), 1.0f));
  }




  // align a given weighted shape to another
  geometricTransform::parameters 
  activeShapeModel::alignShape(pointDistributionModel::shape &shape,
                           const pointDistributionModel::shape &referenceShape,
                               const fvector &pointReliability) const {
    if (shape.size() != referenceShape.size())
      throw exception("shapes have different sizes");
    
    if (pointReliability.size() != shape.size())
      throw exception("invalid reliability vector");
    
    geometricTransform transformation;
    geometricTransform::parameters trafoParameters;

    tpoint<double> temp; // (geometricTransform can only handle double
                         // values!)

    // step 1:
    // shift reference shape to (0,0)   (now called 'normalizedReference')
    pointDistributionModel::shape normalizedReference;
    tpoint<float> refShapeCOG = calculateShapeCenter(referenceShape, pointReliability);

    trafoParameters.shift(temp.castFrom(refShapeCOG * (-1.0f)));
    transformation.setParameters(trafoParameters);
    transformation.apply(referenceShape, normalizedReference);

    // step 2:
    // shift shape to (0,0)    (only calculated, not yet executed)
    tpoint<float> shapeCOG = calculateShapeCenter(shape, pointReliability);

    trafoParameters.clear();
    trafoParameters.shift(temp.castFrom(shapeCOG * (-1.0f)));

    // step 3:
    // rescale shape to the same size as normalizedReference
//    float refShapeScale = calculateShapeDistance(normalizedReference, pointReliability);
//    float shapeScale = calculateShapeDistance(shape, pointReliability, shapeCOG);
//    trafoParameters.scale(refShapeScale / shapeScale);

    float scaleFactor = calculateRelativeShapeScale(shape, normalizedReference, pointReliability, shapeCOG);
    trafoParameters.scale(1.0f / scaleFactor);

    transformation.setParameters(trafoParameters);
    transformation.apply(shape);

    // step 4:
    // rotate shape to fit normalizedReference  (only calculated)
    double shapeOrientation = calculateShapeOrientation(shape, normalizedReference, pointReliability);

    trafoParameters.clear();
    trafoParameters.rotate(shapeOrientation);

    // step 5:
    // move shape to reference shape position
    trafoParameters.shift(temp.castFrom(refShapeCOG));

    transformation.setParameters(trafoParameters);
    transformation.apply(shape);

    // shape has now been aligned to reference shape
    // now the re-transformation-steps have to be documented
    trafoParameters.clear();
    trafoParameters.shift(temp.castFrom(refShapeCOG * (-1.0f)));
    trafoParameters.rotate(-shapeOrientation);
    trafoParameters.scale(scaleFactor);
//    trafoParameters.scale(shapeScale / refShapeScale);
    trafoParameters.shift(temp.castFrom(shapeCOG));

    return trafoParameters;
  }




  // calculate difference of two shapes, i.e. the sum of squared distances of all
  // corresponding points divided by the number of points
  float activeShapeModel::calculateShapeDifference(const pointDistributionModel::shape &firstShape,
                                                   const pointDistributionModel::shape &secondShape) const {

    return calculateShapeDifference(firstShape, secondShape, fvector(firstShape.size(), 1.0f));
  }




  // calculate difference of two weighted shapes
  float activeShapeModel::calculateShapeDifference(const pointDistributionModel::shape &firstShape,
                                                   const pointDistributionModel::shape &secondShape,
                                                   const fvector &pointReliability) const {
    if (firstShape.size() != secondShape.size())
      throw exception("shapes have different sizes");

    if (pointReliability.size() != firstShape.size())
      throw exception("invalid reliability vector");

    float theDifference = 0.0;

    pointDistributionModel::shape::const_iterator firstIt = firstShape.begin();
    pointDistributionModel::shape::const_iterator secondIt = secondShape.begin();

    // calculate sum of square differences of all points
    for (int i=0; i<firstShape.size(); ++i, ++firstIt, ++secondIt)
      theDifference += (*firstIt).distanceSqr(*secondIt) * pointReliability[i];

    theDifference /= pointReliability.sumOfElements();
    return theDifference;
  }




  // calculate angle between a point and the x-axis
  double activeShapeModel::calculateAngle(const tpoint<float> &thePoint) const
  {
    double length = sqrt(thePoint.x * thePoint.x + thePoint.y * thePoint.y);

    if (length == 0.0f)
      return 0.0f;

    // avoid rounding and ensuing cosinus errors
    double cosAngle = thePoint.x / length;
    if (cosAngle > 1.0)
      cosAngle = 1.0;
    else if (cosAngle < -1.0)
      cosAngle = -1.0;

    double theAngle = acos(cosAngle);

    if (thePoint.y < 0)
      return -theAngle;

    return theAngle;
  }




  //
  // calculate normal (actually bisecting line) of a point between two line segments
  // n = normalized( normalized(v1) + normalized(v2) ) * (-90° rotation);
  //
  tpoint<float> activeShapeModel::calculateNormal(const tpoint<float> &segment1, const tpoint<float> &segment2) const {

    tpoint<float> normal = getNormalizedVector(segment1) + getNormalizedVector(segment2);
    if (normal != tpoint<float>(0.0f, 0.0f))
      normal = getNormalizedVector(normal);

    // rotate by -90° (i.e. rotate 90° to the right in a standard coordinate system)
    return tpoint<float>(normal.y, -normal.x);
  }




  //
  // return normalized vector
  //
  tpoint<float> activeShapeModel::getNormalizedVector(const tpoint<float> &vec) const {
    float squareLength = vec.x * vec.x + vec.y * vec.y;
    if (squareLength == 0.0)
      throw exception("can't calculate normal of a 0-vector");

    return vec / sqrt(squareLength);
  }




  //
  // create normals for each point of the shape
  //
  void activeShapeModel::createListOfNormals(const pointDistributionModel::shape &theShape, tpointList<float> &listOfNormals) const {
    listOfNormals.clear();

    tpoint<float> lastSegment, normal;
    tpoint<float> nextSegment = theShape.front() - theShape.back();

    // it is the current point, nextIt is the next point in the shape
    pointDistributionModel::shape::const_iterator it = theShape.begin();
    pointDistributionModel::shape::const_iterator nextIt = it;

    // nextIt is always one step ahead of it
    for (++nextIt; !(it == theShape.end()); ++it, ++nextIt) {

      // consider wrap-around
      if (nextIt == theShape.end())
        nextIt = theShape.begin();

      // shift segments
      lastSegment = nextSegment;
      nextSegment = (*nextIt) - (*it);

      // calculate normal, if possible
      normal = tpoint<float>(0.0f, 0.0f);
      if (lastSegment != normal) {
        if (nextSegment != normal)
          // calculate bisecting line of segments before and after current point
          normal = calculateNormal(lastSegment, nextSegment);
        else
          // calculate normal on last segment instead
          normal = calculateNormal(lastSegment, lastSegment);
      }

      listOfNormals.push_back(normal);
    }

  }



}
