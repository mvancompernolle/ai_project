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
 * file .......: ltiHuMoments.cpp
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 23.10.2001
 * revisions ..: $Id: ltiHuMoments.cpp,v 1.9 2006/09/05 10:17:17 ltilib Exp $
 */

//include files
#include "ltiHuMoments.h"

namespace lti {

  const int NUM_FEAT = 7; // total number of features
  const int MORE_FEAT = 6; // total number of features

  // --------------------------------------------------
  // huMoments::parameters
  // --------------------------------------------------

  const int  huMoments::hu1         = 0;
  const int  huMoments::hu2         = 1;
  const int  huMoments::hu3         = 2;
  const int  huMoments::hu4         = 3;
  const int  huMoments::hu5         = 4;
  const int  huMoments::hu6         = 5;
  const int  huMoments::hu7         = 6;

  const int  huMoments::xcog        = 0;
  const int  huMoments::ycog        = 1;
  const int  huMoments::eigen1      = 2;
  const int  huMoments::eigen2      = 3;
  const int  huMoments::orientation = 4;
  const int  huMoments::m00         = 5;


  // default constructor
  huMoments::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    // Initialize parameter values
    scaling = bool(false);
  }

  // copy constructor
  huMoments::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  huMoments::parameters::~parameters() {
  }

  // get type name
  const char* huMoments::parameters::getTypeName() const {
    return "huMoments::parameters";
  }

  // copy member

  huMoments::parameters&
    huMoments::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif


      scaling = other.scaling;

    return *this;
  }

  // alias for copy member
  huMoments::parameters&
    huMoments::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* huMoments::parameters::clone() const {
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
  bool huMoments::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool huMoments::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"scaling",scaling);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool huMoments::parameters::write(ioHandler& handler,
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
  bool huMoments::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool huMoments::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"scaling",scaling);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool huMoments::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // huMoments
  // --------------------------------------------------

  // default constructor
  huMoments::huMoments()
    : globalFeatureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  huMoments::huMoments(const huMoments& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  huMoments::~huMoments() {
  }

  // returns the name of this type
  const char* huMoments::getTypeName() const {
    return "huMoments";
  }

  // copy member
  huMoments& huMoments::copy(const huMoments& other) {
    globalFeatureExtractor::copy(other);

    return (*this);
  }

  // alias for copy member
  huMoments& huMoments::operator=(const huMoments& other) {
    return (copy(other));
  }


  // clone member
  functor* huMoments::clone() const {
    return new huMoments(*this);
  }

  // return parameters
  const huMoments::parameters&
    huMoments::getParameters() const {
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

  // On copy apply for type channel8!
  bool huMoments::apply(const channel8& src,dvector& dest, dvector& more) const {
    channel8::value_type val;

    dest.resize(NUM_FEAT,0,false,true);
    more.resize(MORE_FEAT,0,false,true);

    double m00=0.0;
    double cm11,cm20,cm02,cm30,cm03,cm12,cm21;
    cm11=cm20=cm02=cm30=cm03=cm12=cm21=0.0;
    double xcog, ycog;
    xcog=ycog=0.0;

    int r, rows=src.rows();
    int c, cols=src.columns();

    // compute simple moments and cog
    for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) {
        val = src.at(r,c);
        m00+=val;
        xcog+=c*val;
        ycog+=r*val;
      }
    }

    // end here, if no content
    if (m00==0) {
      return false;
    }

    // compute cog's
    more[huMoments::xcog]=xcog=xcog/m00;  //xcog
    more[huMoments::ycog]=ycog=ycog/m00;  //ycog

    // compute central moments
    for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) {
        val = src.at(r,c);
        double x_xcog = c-xcog;
        double y_ycog = r-ycog;
        cm11+=(x_xcog)*(y_ycog)*val;
        cm20+=(x_xcog)*(x_xcog)*val;
        cm02+=(y_ycog)*(y_ycog)*val;
        cm30+=(x_xcog)*(x_xcog)*(x_xcog)*val;
        cm03+=(y_ycog)*(y_ycog)*(y_ycog)*val;
        cm12+=(x_xcog)*(y_ycog)*(y_ycog)*val;
        cm21+=(x_xcog)*(x_xcog)*(y_ycog)*val;
      }
    }

    double m00pow2,m00pow2_5;
    m00pow2 = m00*m00;
    m00pow2_5 = pow(m00,2.5);
    // normalized central moments
    cm02 = cm02/m00pow2;
    cm03 = cm03/m00pow2_5;
    cm11 = cm11/m00pow2;
    cm12 = cm12/m00pow2_5;
    cm20 = cm20/m00pow2;
    cm21 = cm21/m00pow2_5;
    cm30 = cm30/m00pow2_5;

    // calculate moment invariants
    dest[huMoments::hu1] = cm20 + cm02;
    dest[huMoments::hu2] = pow((cm20 - cm02),2) + 4*pow(cm11,2);
    dest[huMoments::hu3] = pow((cm30 - 3*cm12),2) + pow((3*cm21 - cm03),2);
    dest[huMoments::hu4] = pow((cm30 + cm12),2) + pow((cm21 + cm03),2);
    dest[huMoments::hu5] = (cm30-3*cm12)*(cm30+cm12)*(   pow((cm30+cm12),2) - 3*pow((cm21+cm03),2) )
                         + (3*cm21-cm03)*(cm21+cm03)*( 3*pow((cm30+cm12),2) -   pow((cm21+cm03),2) );
    dest[huMoments::hu6] = (cm20-cm02)*( pow((cm30+cm12),2) - pow((cm21+cm03),2) )
                         + 4*cm11*(cm30+cm12)*(cm21+cm03);
    dest[huMoments::hu7] = (3*cm21-cm03)*(cm30+cm12)*(   pow((cm30+cm12),2) - 3*pow((cm21+cm03),2) )
                         - (cm30-3*cm12)*(cm21+cm03)*( 3*pow((cm30+cm12),2) -   pow((cm21+cm03),2) );


    double temp = sqrt( (cm20 - cm02)*(cm20 - cm02) + 4*cm11*cm11 );
    more[huMoments::eigen1]=m00*0.5*((cm20+cm02) + temp); //eigen 1
    more[huMoments::eigen2]=m00*0.5*((cm20+cm02) - temp); //eigen 2
    more[huMoments::orientation]=0.5*atan2(2*cm11, cm20 - cm02); //orientation
    more[huMoments::m00]=m00; //m00

    const parameters& param = getParameters();

    if (param.scaling) {
      int i;
      for (i=0; i<dest.size();i++){
        dest[i]=-logn(dest[i]);
      }
    }

    return true;
  }

  // On copy apply for type channel!
  bool huMoments::apply(const channel& src,dvector& dest, dvector& more) const {
    channel::value_type val;

    dest.resize(NUM_FEAT,0,false,true);
    more.resize(MORE_FEAT,0,false,true);

    double m00=0.0;
    double cm11,cm20,cm02,cm30,cm03,cm12,cm21;
    cm11=cm20=cm02=cm30=cm03=cm12=cm21=0.0;
    double xcog, ycog;
    xcog=ycog=0.0;

    int r, rows=src.rows();
    int c, cols=src.columns();

    // compute simple moments and cog
    for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) {
        val = src.at(r,c);
        m00+=val;
        xcog+=c*val;
        ycog+=r*val;
      }
    }

    // end here, if no content
    if (m00==0) {
      return false;
    }

    // compute cog's
    more[huMoments::xcog]=xcog=xcog/m00;  //xcog
    more[huMoments::ycog]=ycog=ycog/m00;  //ycog

    // compute central moments
    for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) {
        val = src.at(r,c);
        double x_xcog = c-xcog;
        double y_ycog = r-ycog;
        cm11+=(x_xcog)*(y_ycog)*val;
        cm20+=(x_xcog)*(x_xcog)*val;
        cm02+=(y_ycog)*(y_ycog)*val;
        cm30+=(x_xcog)*(x_xcog)*(x_xcog)*val;
        cm03+=(y_ycog)*(y_ycog)*(y_ycog)*val;
        cm12+=(x_xcog)*(y_ycog)*(y_ycog)*val;
        cm21+=(x_xcog)*(x_xcog)*(y_ycog)*val;
      }
    }

    double m00pow2,m00pow2_5;
    m00pow2 = m00*m00;
    m00pow2_5 = pow(m00,2.5);
    // normalized central moments
    cm02 = cm02/m00pow2;
    cm03 = cm03/m00pow2_5;
    cm11 = cm11/m00pow2;
    cm12 = cm12/m00pow2_5;
    cm20 = cm20/m00pow2;
    cm21 = cm21/m00pow2_5;
    cm30 = cm30/m00pow2_5;

    // calculate moment invariants
    dest[huMoments::hu1] = cm20 + cm02;
    dest[huMoments::hu2] = pow((cm20 - cm02),2) + 4*pow(cm11,2);
    dest[huMoments::hu3] = pow((cm30 - 3*cm12),2) + pow((3*cm21 - cm03),2);
    dest[huMoments::hu4] = pow((cm30 + cm12),2) + pow((cm21 + cm03),2);
    dest[huMoments::hu5] = (cm30-3*cm12)*(cm30+cm12)*(   pow((cm30+cm12),2) - 3*pow((cm21+cm03),2) )
                         + (3*cm21-cm03)*(cm21+cm03)*( 3*pow((cm30+cm12),2) -   pow((cm21+cm03),2) );
    dest[huMoments::hu6] = (cm20-cm02)*( pow((cm30+cm12),2) - pow((cm21+cm03),2) )
                         + 4*cm11*(cm30+cm12)*(cm21+cm03);
    dest[huMoments::hu7] = (3*cm21-cm03)*(cm30+cm12)*(   pow((cm30+cm12),2) - 3*pow((cm21+cm03),2) )
                         - (cm30-3*cm12)*(cm21+cm03)*( 3*pow((cm30+cm12),2) -   pow((cm21+cm03),2) );


    double temp = sqrt( (cm20 - cm02)*(cm20 - cm02) + 4*cm11*cm11 );
    more[huMoments::eigen1]=m00*0.5*((cm20+cm02) + temp); //eigen 1
    more[huMoments::eigen2]=m00*0.5*((cm20+cm02) - temp); //eigen 2
    more[huMoments::orientation]=0.5*atan2(2*cm11, cm20 - cm02); //orientation
    more[huMoments::m00]=m00; //m00

    const parameters& param = getParameters();

    if (param.scaling) {
      int i;
      for (i=0; i<dest.size();i++){
        dest[i]=-logn(dest[i]);
      }
    }

    return true;
  }

  // On copy apply for type channel8!
  bool huMoments::apply(const channel8& src,dvector& dest) const {

    dvector more;
    more.resize(MORE_FEAT,0,false,true);
    return apply(src,dest,more);
  };


  // On copy apply for type channel!
  bool huMoments::apply(const channel& src,dvector& dest) const {

    dvector more;
    more.resize(MORE_FEAT,0,false,true);
    return apply(src,dest,more);
  };

  // On copy apply for type channel8!
  bool huMoments::apply(const channel8& src, const rectangle& rect, dvector& dest, dvector& more) const {

    try {
      channel8 tmp(false,*const_cast<channel8*>(&src),rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
      if (!apply(tmp,dest,more)) {
        return false;
      }
      more[xcog]+=rect.ul.x;
      more[ycog]+=rect.ul.y;
    }
    catch (exception e) {
      setStatusString("Failed to create submatrix of input channel8!");
      return false;
    }
    return true;
  }

  // On copy apply for type channel!
  bool huMoments::apply(const channel& src, const rectangle& rect, dvector& dest, dvector& more) const {

    try {
      channel tmp(false,*const_cast<channel*>(&src),rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
      if (!apply(tmp,dest,more)) {
        return false;
      }
      more[xcog]+=rect.ul.x;
      more[ycog]+=rect.ul.y;
    }
    catch (exception e) {
      setStatusString("Failed to create submatrix of input channel8!");
      return false;
    }
    return true;
  }

  // On copy apply for type channel8!
  bool huMoments::apply(const channel8& src, const rectangle& rect, dvector& dest) const {

    try {
      channel8 tmp(false,*const_cast<channel8*>(&src),rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
      return apply(tmp,dest);
    }
    catch (exception e) {
      setStatusString("Failed to create submatrix of input channel8!");
      return false;
    }
  }

  // On copy apply for type channel!
  bool huMoments::apply(const channel& src, const rectangle& rect, dvector& dest) const {

    try {
      channel tmp(false,*const_cast<channel*>(&src),rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
      return apply(tmp,dest);
    }
    catch (exception e) {
      setStatusString("Failed to create submatrix of input channel8!");
      return false;
    }
  }


  double huMoments::logn(double& x) const {
		if (x<0.0) {
			return -log(-x);
		}
		else {
			return log(x);
		}
  }

}
