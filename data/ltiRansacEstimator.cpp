/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiRansacEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.4.2004
 * revisions ..: $Id: ltiRansacEstimator.cpp,v 1.10 2006/09/05 10:27:39 ltilib Exp $
 */

#include "ltiRansacEstimator.h"
#include "ltiMath.h"
#include "ltiScramble.h"
#undef _LTI_DEBUG
//#define _LTI_DEBUG 3
#include "vector"
#include "limits"

#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // ransacEstimator::parameters
  // --------------------------------------------------

  // default constructor
  ransacEstimator::parameters::parameters()
    : monteCarloEstimator::parameters() {
 
    adaptiveContamination = bool(false);
    numCorrespondencesPerTrial = int(10);
    checkStop = bool(true);
    contamination = float(.5);
    confidence = float(.99);
    useMinCorrespondences = bool(true);
  }

  // copy constructor
  ransacEstimator::parameters::parameters(const parameters& other)
    : monteCarloEstimator::parameters() {
    _lti_debug("  _constructing Ransac parameters (copy) "
               << other.existsNormalization() << "\n");
    copy(other);
  }

  // destructor
  ransacEstimator::parameters::~parameters() {
  }

  // get type name
  const char* ransacEstimator::parameters::getTypeName() const {
    return "ransacEstimator::parameters";
  }

  // copy member

  ransacEstimator::parameters&
    ransacEstimator::parameters::copy(const parameters& other) {

# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    monteCarloEstimator::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    monteCarloEstimator::parameters& (monteCarloEstimator::parameters::* p_copy)
      (const monteCarloEstimator::parameters&) =
      monteCarloEstimator::parameters::copy;
    (this->*p_copy)(other);
# endif
      _lti_debug("  _copying Ransac parameters "
                 << other.existsNormalization()
                 << " this " << existsNormalization() << "\n ");
      adaptiveContamination = other.adaptiveContamination;
      numCorrespondencesPerTrial = other.numCorrespondencesPerTrial;
      checkStop = other.checkStop;
      contamination = other.contamination;
      confidence = other.confidence;
      useMinCorrespondences = other.useMinCorrespondences;

    return *this;
  }

  // alias for copy member
  ransacEstimator::parameters&
    ransacEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* ransacEstimator::parameters::clone() const {
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
  bool ransacEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool ransacEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"adaptiveContamination",adaptiveContamination);
      lti::write(handler,"numCorrespondencesPerTrial",
                 numCorrespondencesPerTrial);
      lti::write(handler,"checkStop",checkStop);
      lti::write(handler,"contamination",contamination);
      lti::write(handler,"confidence",confidence);
      lti::write(handler,"useMinCorrespondences",useMinCorrespondences);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && monteCarloEstimator::parameters::write(handler,false);
# else
    bool (monteCarloEstimator::parameters::* p_writeMS)(ioHandler&,const bool) const =
      monteCarloEstimator::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ransacEstimator::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool ransacEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool ransacEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"adaptiveContamination",adaptiveContamination);
      lti::read(handler,"numCorrespondencesPerTrial",
                numCorrespondencesPerTrial);
      lti::read(handler,"checkStop",checkStop);
      lti::read(handler,"contamination",contamination);
      lti::read(handler,"confidence",confidence);
      lti::read(handler,"useMinCorrespondences",useMinCorrespondences);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && monteCarloEstimator::parameters::read(handler,false);
# else
    bool (monteCarloEstimator::parameters::* p_readMS)(ioHandler&,const bool) =
      monteCarloEstimator::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ransacEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // ransacEstimator
  // --------------------------------------------------

  // default constructor
  ransacEstimator::ransacEstimator()
    : monteCarloEstimator(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  ransacEstimator::ransacEstimator(const parameters& par)
    : monteCarloEstimator() {
    _lti_debug("  _constructing Ransac " << par.existsNormalization() << " \n");
    // set the given parameters
    setParameters(par);
    _lti_debug("  _constructed Ransac\n");
  }


  // copy constructor
  ransacEstimator::ransacEstimator(const ransacEstimator& other)
    : monteCarloEstimator() {
    copy(other);
  }

  // destructor
  ransacEstimator::~ransacEstimator() {
  }

  // returns the name of this type
  const char* ransacEstimator::getTypeName() const {
    return "ransacEstimator";
  }

  // copy member
  ransacEstimator&
    ransacEstimator::copy(const ransacEstimator& other) {
      monteCarloEstimator::copy(other);

    return (*this);
  }

  // alias for copy member
  ransacEstimator&
    ransacEstimator::operator=(const ransacEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* ransacEstimator::clone() const {
    return new ransacEstimator(*this);
  }

  // return parameters
  const ransacEstimator::parameters&
    ransacEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // A very private helper
  // -------------------------------------------------------------------
  template<class T, class U>
  class ransacHelper {
  public:
    ransacHelper(const ransacEstimator::parameters& para) :
      m_parameters ( para ) {
      m_numCorresp = ( m_parameters.useMinCorrespondences ?
                       m_parameters.getTransform().minNumberCorrespondences() :
                       m_parameters.numCorrespondencesPerTrial );
      m_logConf = log10(1.f-m_parameters.confidence);
      _lti_debug(" *** m_logConf " << m_logConf);
    };

    ~ransacHelper() {
    };
   
    bool apply(const matrix<tpoint<T> >& src,
               vector<U>& dest) const;

    bool apply(const matrix<tpoint<T> >& src,
               vector<U>& dest, vector<U>& error) const;

  protected:
    bool compute(const matrix<tpoint<T> >& src, ivector& inliers,
                 vector<U>& dest) const;

    int getNumInliers(const float contamination, const int total) const;

    int getNumTrials(const float inlierProb) const;

    const ransacEstimator::parameters& m_parameters;

    int m_numCorresp;

    float m_logConf;
  };

  template<class T, class U>
  inline int ransacHelper<T,U>
  ::getNumInliers(const float contamination, const int total) const {
    return static_cast<int>( static_cast<float>(total)
                             * ( 1.f - contamination ) + .5 );
  }

  template<class T, class U>
  inline int ransacHelper<T,U>
  ::getNumTrials(const float inlierProb) const {

    const float tmp ( pow(inlierProb,m_numCorresp) );
    if ( tmp > -std::numeric_limits<float>::epsilon() &&
         tmp < std::numeric_limits<float>::epsilon() ) {
      return std::numeric_limits<int>::max();
    }
    return static_cast<int>( m_logConf / log10(1-tmp) + .5 );
  }

  template<class T, class U>
  inline bool ransacHelper<T,U>::compute(const matrix<tpoint<T> >& src,
                                         ivector& inliers,
                                         vector<U>& dest) const {
    bool success ( false );
    const transformEstimator& transform = m_parameters.getTransform();

    //normalize the correspondences
    const int rows ( src.rows() );
    const int cols ( src.columns() );
    matrix<tpoint<T> > normedSrc ( false, rows, cols );
    vector<tpoint<T> > scaleVect ( false, rows );
    vector<tpoint<T> > shiftVect ( false, rows );
    if ( m_parameters.existsNormalization() ) {
      const pointSetNormalization& normalizer=m_parameters.getNormalization();
      typename vector<tpoint<T> >::iterator scaleIt ( scaleVect.begin() );
      typename vector<tpoint<T> >::iterator shiftIt ( shiftVect.begin() );
      int i ( 0 );
      for ( ; i<rows; ++i, ++scaleIt, ++shiftIt ) {
        const vector<tpoint<T> >& row = src.getRow(i);
        vector<tpoint<T> >& newRow = normedSrc.getRow(i);
        normalizer.apply(row, newRow, *scaleIt, *shiftIt);
#if defined(_LTI_DEBUG) && _LTI_DEBUG > 5
        vector<tpoint<T> >::const_iterator it  ( row.begin() );
        vector<tpoint<T> >::const_iterator end ( row.end() );
        vector<tpoint<T> >::iterator nit  ( newRow.begin() );
        for ( ; it!=end; ++it,++nit ) {
          _lti_debug(" || " << *it << " " << *nit);
        }
#endif
      }
    }
    const matrix<tpoint<T> >& usedSrc = ( m_parameters.existsNormalization() ?
                                          normedSrc : src );
    _lti_debug ( " *** using norma " <<  m_parameters.existsNormalization()
                 << "\n scale\n" << scaleVect << "\n shift\n" << shiftVect );

    //create empty vectors because we do not know the number of inliers yet
    float pointRatio ( 1.f / static_cast<float>(cols) );
    int minInliers ( getNumInliers(m_parameters.contamination, cols) );
    _lti_debug(" *** minInliers start " << minInliers);
    vector<U> winningError;
    U winningResidualSize (std::numeric_limits<U>::max());
    int numWinningInliers ( 0 );
    std::vector<int> tmpInliers ( cols );

    const transformEstimator::parameters& transPar = transform.getParameters();
    const U maxError ( transPar.computeSqError ?
                       static_cast<U>(m_parameters.maxError *
                                      m_parameters.maxError) :
                       static_cast<U>(m_parameters.maxError) );

    // create a random index vector, to select the correspondences
    // todo: efficiency ???
    scramble<int> scrambler;
    int i;
    ivector indices(false,cols);
    for (i=0;i<cols;++i) {
      indices.at(i)=i;
    }
    vector<U> error;
    int trial ( 0 );
    int numIterations ( m_parameters.numIterations );
    for (; trial < numIterations &&
           (!m_parameters.checkStop || numWinningInliers < minInliers );
         trial++ ) {
      _lti_debug("|");
      //random draw
      scrambler.apply(indices);

      if ( !transform.apply(usedSrc,dest,error,indices,m_numCorresp) ) {
        _lti_debug(" __no_transform__ "<< m_numCorresp);
        continue;
      }
 
      //compute the number of inliers and the residual size
      int numInliers ( 0 );
      U residualSize ( 0 );
#if defined(_LTI_DEBUG) && _LTI_DEBUG > 2
      U totalResidualSize ( 0 );
#endif
      {
        tmpInliers.clear();
        int index ( 0 );
        typename vector<U>::iterator it  ( error.begin() );
        typename vector<U>::iterator end ( error.end()   );
        for ( ; it!=end; ++it, ++index ) {
          if ( *it < maxError ) {
            //_lti_debug(" || res " << *it << " of " << maxError);
            residualSize += *it;
            tmpInliers.push_back(index);
            numInliers++;
          }
#if defined(_LTI_DEBUG) && _LTI_DEBUG > 2
          totalResidualSize += *it;
#endif
        }
        const U tmp ( transPar.computeSqError?
                      static_cast<U>(numInliers * numInliers) :
                      static_cast<U>(numInliers) );
        residualSize /= tmp;
#if defined(_LTI_DEBUG) && _LTI_DEBUG > 2
        const U tmpT ( transPar.computeSqError?
                      cols * cols : cols );
        totalResidualSize /= tmpT; //only used for debugging output
        _lti_debug(" || res " << residualSize 
                   << " total " << totalResidualSize);
#endif
      }

      if ( residualSize > maxError ) {
        _lti_debug(" __too_big_residual__ " << residualSize);
        continue;
      }
      success = true;

      //remember the estimated transform with the maximum inliers
      if ( ( numInliers > numWinningInliers ) ||
           ( numInliers == numWinningInliers &&
             residualSize < winningResidualSize ) ) {
        numWinningInliers = numInliers;
        winningResidualSize = residualSize;
        //remember the winning inliers
        inliers.resize( tmpInliers.size(), 0, false, false );
        std::vector<int>::iterator it ( tmpInliers.begin() );
          std::vector<int>::iterator end ( tmpInliers.end() );
          ivector::iterator nit ( inliers.begin() );
          for ( ; it!=end; ++it, ++nit ) {
            *nit = *it;
          }
      }

      if ( m_parameters.adaptiveContamination ) {
        //do not increase the number of iterations
        // only decrease them
        numIterations 
          = min(getNumTrials(static_cast<float>(numInliers) * pointRatio),
                numIterations);
#if defined(_LTI_DEBUG) && _LTI_DEBUG > 1
        _lti_debug(" num. iterations " << numIterations 
                   << " inliers " << numInliers);
#endif
      }

    }//trials
    _lti_debug(" RANSAC success " << success 
               << " winning inliers " << numWinningInliers
               << " required " << minInliers 
               << " residual size " << winningResidualSize << "\n");

    //ensure that we determined enough inliers (no adaptive contamination)
    //or performed enough iterations (adaptive contamination)
    if ( !( ( numWinningInliers >= minInliers ||
              m_parameters.adaptiveContamination ) && success ) ) {
      _lti_debug(" ****RANSAC:FALSE****");
      return false;
    }

    //restimate from all inliers (incl. norma)
    if ( !transform.apply(usedSrc,dest,inliers,inliers.size()) ) {
      return false;
    }

    //denormalize the estimated transform to fit the original data
    if ( m_parameters.existsNormalization() ) {
      return transform.denormalize(dest, scaleVect, shiftVect);
    }
    
    return true;
  }

  template<class T, class U>
  inline bool ransacHelper<T,U>::apply(const matrix<tpoint<T> >& src,
                                       vector<U>& dest) const {

    //maximize the number of inliers
    ivector inliers;
    if ( !compute(src,inliers,dest) ) {
      return false;
    }
 
    return true;
  }

  template<class T, class U>
  inline bool ransacHelper<T,U>::apply(const matrix<tpoint<T> >& src,
                                       vector<U>& dest,
                                       vector<U>& error) const {

    //maximize the number of inliers
    ivector inliers;
    if ( !compute(src,inliers,dest) ) {
      return false;
    }
     
    //compute the residual ( without normalization )
    const transformEstimator& transform = m_parameters.getTransform();
    return transform.computeResidual(src,dest,error);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool ransacEstimator::apply(const matrix<dpoint>& src,
                              dvector& dest) const {
 
    const parameters& par = getParameters();
    ransacHelper<double,double> help ( par );
    return help.apply(src,dest);
  }
  
  bool ransacEstimator::apply(const matrix<dpoint>& src,
                              dvector& dest, dvector& error) const {
    
    const parameters& par = getParameters();
    ransacHelper<double,double> help ( par );
    return help.apply(src,dest,error);
  }

  bool ransacEstimator::apply(const matrix<fpoint>& src,
                              fvector& dest) const {
    
    const parameters& par = getParameters();
    ransacHelper<float,float> help ( par );
    return help.apply(src,dest);
  }
  
  bool ransacEstimator::apply(const matrix<fpoint>& src,
                              fvector& dest, fvector& error) const {
    const parameters& par = getParameters();
    ransacHelper<float,float> help ( par );
    return help.apply(src,dest,error);
  }

}

#include "ltiUndebug.h"
