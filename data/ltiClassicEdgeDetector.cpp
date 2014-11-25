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

/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiClassicEdgeDetector.cpp
 * authors ....: Arnd Hannemann 
 * organization: LTI, RWTH Aachen
 * creation ...: 27.11.2003
 * revisions ..: $Id: ltiClassicEdgeDetector.cpp,v 1.8 2006/09/05 10:05:36 ltilib Exp $
 */


#include "ltiClassicEdgeDetector.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#include "ltiDebug.h"

namespace lti {
  // ----------------------------
  // parameters class
  // ----------------------------

  // default constructor
  classicEdgeDetector::parameters::parameters()
    : edgeDetector::parameters() {

    gradientParameters.kernelType = gradientFunctor::parameters::Sobel;
    maximaSearchMethod = NonMaximaSuppression;
    suppressor         = new nonMaximaSuppression();
    thresholder        = 0;
    destroyFunctor     = true;
    _lti_debug("new parameters: default constrcutor"<<"\n");
  }

  // copy constructor
  classicEdgeDetector::parameters::parameters(const parameters& other)  
    : edgeDetector::parameters() {
    thresholder = 0;
    suppressor = 0;
    _lti_debug("new parameters: copy constructor"<<"\n");
    copy(other);
  }

  // destructor
  classicEdgeDetector::parameters::~parameters() {
    _lti_debug("destroy parameters"<<"\n");
    if (destroyFunctor) {
      if (suppressor != 0) {
        delete suppressor;
        suppressor=0;
        _lti_debug("destroyed suppressor"<<"\n");
      }
      if (thresholder !=0) {
        delete thresholder;
        thresholder=0;
        _lti_debug("destroyed thresholder"<<"\n");
      }
    }
  }

  void classicEdgeDetector::parameters::setMaximaSearchMethod(
                              const thresholding& t) {

    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
    }
    maximaSearchMethod = Thresholding;
    // copy thresholding functor
    thresholder = dynamic_cast<thresholding*>(t.clone());
    destroyFunctor = true;
  }

  void classicEdgeDetector::parameters::setMaximaSearchMethod(
                              const nonMaximaSuppression& s) {

    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder  = 0;
    }
    maximaSearchMethod = NonMaximaSuppression;
    // copy thresholding functor
    suppressor = dynamic_cast<nonMaximaSuppression*>(s.clone());
    destroyFunctor = true;
  }

  void classicEdgeDetector::parameters::attachMaximaSearchMethod(thresholding* t) {

    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
    }
    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    maximaSearchMethod = Thresholding;
    thresholder = t;
    destroyFunctor = true;
  }

  void classicEdgeDetector::parameters::attachMaximaSearchMethod(nonMaximaSuppression* s) {

    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
    }
    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    maximaSearchMethod = NonMaximaSuppression;
    suppressor = s;
    destroyFunctor = true;
  }

  void classicEdgeDetector::parameters::useExternalMaximaSearchMethod(thresholding* t) {
    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
    }
    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    maximaSearchMethod = Thresholding;
    thresholder = t;
    destroyFunctor = false;
  }

  void classicEdgeDetector::parameters::useExternalMaximaSearchMethod(nonMaximaSuppression* s) { 
   
    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
    }
    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
    }
    maximaSearchMethod = NonMaximaSuppression;
    suppressor = s;
    destroyFunctor = false;
  }

   // get type name
  const char* classicEdgeDetector::parameters::getTypeName() const {
    return "classicEdgeDetector::parameters";
  }


  // copy member
  classicEdgeDetector::parameters&
  classicEdgeDetector::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    edgeDetector::parameters::copy(other);
# else
    // ...so we have to use this workaround
    // Conditional on that, copy may not be virtual.
    edgeDetector::parameters& (edgeDetector::parameters::* p_copy)
      (const edgeDetector::parameters&) =
      edgeDetector::parameters::copy;
    (this->*p_copy)(other);
# endif
    _lti_debug("classicEdgeDetector::parameters::copy invoked\n");    
    _lti_debug("thresholder = "<<thresholder<<"\n");
    _lti_debug("suppressor = "<<suppressor<<"\n");
   
    // kernelType = other.kernelType;
    gradientParameters = other.gradientParameters;

    maximaSearchMethod = other.maximaSearchMethod;

    if (thresholder != 0 && destroyFunctor) {
      delete thresholder;
      thresholder = 0;
      _lti_debug("classicEdgeDetector::parameters::copy: destroyed thresholder");
    }
    if (suppressor != 0 && destroyFunctor) {
      delete suppressor;
      suppressor = 0;
      _lti_debug("classicEdgeDetector::parameters::copy: destroyed suppressor");
    }
    switch (other.maximaSearchMethod) {
      case Thresholding: {
        thresholder = 
          dynamic_cast<thresholding*>(other.thresholder->clone());
        suppressor = 0;
        break;
      }
      case NonMaximaSuppression: {
        suppressor = 
          dynamic_cast<nonMaximaSuppression*>(other.suppressor->clone());
        thresholder = 0;
        break;
      }
    }
    destroyFunctor=true;

    return *this;
  }
  // alias for copy member
  classicEdgeDetector::parameters&
  classicEdgeDetector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* classicEdgeDetector::parameters::clone() const {
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
  bool classicEdgeDetector::parameters::write(ioHandler& handler,
					      const bool complete) const
# else
    bool classicEdgeDetector::parameters::writeMS(ioHandler& handler,
						  const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      std::string str2;
      switch (maximaSearchMethod) {
        case parameters::NonMaximaSuppression:
          str2="NonMaximaSuppression";
          break;
        case parameters::Thresholding:
          str2="Thresholding";
          break;
      }
      // write gradientFunctor::parameters
      gradientParameters.write(handler,false);
      lti::write(handler,"maximaSearchMethod",str2);
      /*

      // at last write parameters of searchmethod
      switch (maximaSearchMethod) {
        case parameters::NonMaximaSuppression:
          b = b && suppressor->getParameters().write(handler,false);
        break;
        case parameters::Thresholding:
          b = b && thresholder->getParameters().write(handler,false);
        break;
        } */

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::write(handler,false);
# else
    bool (edgeDetector::parameters::* p_writeMS)(ioHandler&,const bool) const =
      edgeDetector::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classicEdgeDetector::parameters::write(ioHandler& handler,
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
  bool classicEdgeDetector::parameters::read(ioHandler& handler,
					     const bool complete)
# else
    bool classicEdgeDetector::parameters::readMS(ioHandler& handler,
						 const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      gradientParameters.read(handler,false);
      lti::read(handler,"maximaSearchMethod",str);

      if (str == "Thresholding") {
        maximaSearchMethod = Thresholding;
        thresholder = new thresholding();
        suppressor = 0;
        destroyFunctor = true;
// Default wird eine NonMaximumSuppression erstellt.
      } else {
        maximaSearchMethod = NonMaximaSuppression;
        suppressor = new nonMaximaSuppression();
        thresholder = 0;
        destroyFunctor = true;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::read(handler,false);
# else
    bool (edgeDetector::parameters::* p_readMS)(ioHandler&,const bool) =
      edgeDetector::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classicEdgeDetector::parameters::read(ioHandler& handler,
                                             const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif





  // ---------------------------
  // classicEdgeDetector class
  // ---------------------------

  // default constructor
  classicEdgeDetector::classicEdgeDetector()
    : edgeDetector() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the Default parameters
    setParameters(defaultParameters);
  
  }

  // default constructor
  classicEdgeDetector::classicEdgeDetector(const parameters& par)
    : edgeDetector() {

    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  classicEdgeDetector::classicEdgeDetector(const classicEdgeDetector& other)
    : edgeDetector() {
    copy(other);
  }

  // destructor
  classicEdgeDetector::~classicEdgeDetector() {
  }


  // get type name
  const char* classicEdgeDetector::getTypeName() const {
    return "classicEdgeDetector";
  }

  // copy member
  classicEdgeDetector&
  classicEdgeDetector::copy(const classicEdgeDetector& other) {
    edgeDetector::copy(other);
    return(*this);
  }

  // alias for copy member
  classicEdgeDetector&
  classicEdgeDetector::operator=(const classicEdgeDetector& other) {
    return(copy(other));
  }

  // clone member
  functor* classicEdgeDetector::clone() const {
    return new classicEdgeDetector(*this);
  }



  // return parameters 
  const classicEdgeDetector::parameters& 
  classicEdgeDetector::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par; 
  }

  // --------------------------------------------------
  // The apply-methods!
  // --------------------------------------------------


  // on copy apply method
  bool classicEdgeDetector::apply(const channel& src, channel8& dest) const {
    channel gradAbs;
    const parameters& par = getParameters();

    bool rc = true;

    // apply NonMaximaSuppression 
    if (rc) {
      switch (par.maximaSearchMethod) {
        case parameters::NonMaximaSuppression: {
          channel gradPol;
          if (par.suppressor != 0) {
            // compute gradients
            rc && gradienter.apply(src,gradAbs,gradPol);
            rc = rc && par.suppressor->apply(gradAbs,gradPol,dest);
          } else {
            setStatusString("maximaSearchMethod is NonMaximaSuppression but " \
                            "suppressor is null pointer");
            rc = false;
          }
          break;
        }

        case parameters::Thresholding: {
          if (par.thresholder != 0) {
            _lti_debug("only compute gradient magnitude\n");
            rc = rc && gradienter.apply(src,gradAbs);
            _lti_debug("do thresholding\n");
            rc = par.thresholder->apply(gradAbs,dest);
          } else {
            setStatusString("maximaSearchMethod is Thresholding but " \
                            "thresholder is null pointer");
            rc = false;
          }
          break;
        }
      } // switch
    } // if (rc)

    return rc;
  }

  // on place apply method
  bool classicEdgeDetector::apply(channel8& srcdest) const {
    channel tmp;
    tmp.castFrom(srcdest);
    return apply(tmp,srcdest);
  }



  bool classicEdgeDetector::updateParameters(const parameters& theParam) {
    // catch errors from all setParameters invocations
    bool b = true;
    
    const parameters& par = getParameters();

    // maxima search method selection
    if (b) {
      switch (par.maximaSearchMethod) {
        case parameters::NonMaximaSuppression: {
          // if no suppressor instance is set return error
          if (par.suppressor!=0) {             
            // init nonMaximaSuppresion
            nonMaximaSuppression::parameters nmsPar = 
              par.suppressor->getParameters();
            nmsPar.edgeValue = par.edgeValue;
            nmsPar.background = par.noEdgeValue;
            // cartesionToPolar should return valid angles
            nmsPar.checkAngles = false;
            b = b && (par.suppressor->setParameters(nmsPar));
          } else {
            setStatusString("maximaSearchMethod is NonMaximaSuppression but " \
                            "suppressor is null pointer");
            b = false;
          }
            break;
          }
          case parameters::Thresholding: {
            // if no thresholder instance is set return error
            if (par.thresholder!=0) {
              // init Thresholding
              thresholding::parameters thrPar =
                par.thresholder->getParameters();
              thrPar.inRegionValue = static_cast<float>(par.edgeValue) / 255;
              thrPar.outRegionValue = static_cast<float>(par.noEdgeValue)/255;
              thrPar.keepInRegion = false;
              thrPar.keepOutRegion = false;
              b = b && (par.thresholder->setParameters(thrPar));
            } else {
              setStatusString("maximaSearchMethod is Thresholding but " \
                              "thresholder is null pointer");
              b = false;            
            }
            break;
          } // case Thresholding
      } // switch
    } // if (b)


    // set gradientfunctor parameters 
    if (b) {
      gradientFunctor::parameters gfPar = par.gradientParameters;
      // always set output format to polar because nonmaximasuppression needs
      // this
      gfPar.format = gradientFunctor::parameters::Polar;
      b = gradienter.setParameters(gfPar);
    }

    return b;
  }
}  

#include "ltiUndebug.h"
