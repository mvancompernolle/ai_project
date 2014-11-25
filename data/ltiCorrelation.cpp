/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiCorrelation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.11.99
 * revisions ..: $Id: ltiCorrelation.cpp,v 1.11 2006/09/05 10:08:00 ltilib Exp $
 */

#include "ltiCorrelation.h"
#include "ltiMinimizeBasis.h"
#include "ltiConvolutionHelper.h"

#include <list>

namespace lti {

  // --------------------------------------------------
  // correlation::parameters
  // --------------------------------------------------

  // default constructor

  correlation::parameters::parameters()
    : filter::parameters(), kernel(0), mask(0), useMask(false), kernelAverage(0) {
    mode = C3;
  }

  // copy constructor

  correlation::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    kernel = 0;
    mask = 0;
    copy(other);
  }

  // destructor

  correlation::parameters::~parameters() {
    delete kernel;
    delete mask;
  }

  // get type name
  const char* correlation::parameters::getTypeName() const {
    return "correlation::parameters";
  }

  // copy member

  correlation::parameters&
    correlation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    filter::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    filter::parameters& (filter::parameters::* p_copy)
      (const filter::parameters&) =
      filter::parameters::copy;
    (this->*p_copy)(other);
# endif

    delete kernel;
    kernel = 0;

    if (notNull(other.kernel)) {
      kernel = other.kernel->clone();
    }

    delete mask;
    mask = 0;

    if (notNull(other.mask)) {
      mask = dynamic_cast<channel8*>(other.mask->clone());
    }

    kernelAverage = other.kernelAverage;
    kernelSize = other.kernelSize;
    mode = other.mode;
    useMask = other.useMask;

    return *this;
  }

  // clone member

  functor::parameters* correlation::parameters::clone() const {
    return new parameters(*this);
  }

  const mathObject& correlation::parameters::getKernel() const {
    if (isNull(kernel)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel;
  }

  const double& correlation::parameters::getKernelAverage() const {
    if (isNull(kernel)) {
      static const double zero = 0.0;
      return zero;
    } else {
      return kernelAverage;
    }
  }

  const int& correlation::parameters::getKernelSize() const {
    if (isNull(kernel)) {
      static const int zero = 0;
      return zero;
    } else {
      return kernelSize;
    }
  }

  void correlation::parameters::setKernel(const mathObject& aKernel) {
    // remove old kernel
    delete kernel;
    kernel = 0;

    // 2D Kernels and Matrices of float or ubyte!
    //
    if (notNull(dynamic_cast<const matrix<float>*>(&aKernel))) {
      // there are now two possibilities: its a matrix<float> (or channel)
      // or its a kernel2D<float>
      if (notNull(dynamic_cast<const kernel2D<float>*>(&aKernel))) {
        kernel = aKernel.clone();
        kernel2D<float>& tmp = *dynamic_cast<kernel2D<float>*>(kernel);
        tmp.mirror();
        kernelSize = (tmp.lastRow()-tmp.firstRow()+1)*
                     (tmp.lastColumn()-tmp.firstColumn()+1);
        kernelAverage = tmp.sumOfElements()/double(kernelSize);
      } else {
        // its a channel or matrix<float>
        kernel2D<float>* tmp = new kernel2D<float>();
        const matrix<float>& mat =
          *dynamic_cast<const matrix<float>*>(&aKernel);
        tmp->castFrom(mat,1-mat.rows(),1-mat.columns());
        tmp->mirror();
        kernel = tmp;
        kernelSize = mat.rows()*mat.columns();
        kernelAverage = mat.sumOfElements()/double(kernelSize);
      }
    } else if (notNull(dynamic_cast<const matrix<ubyte>*>(&aKernel))) {
      // there are now two possibilities: its a matrix<ubyte> (or channel8)
      // or its a kernel2D<ubyte>
      if (notNull(dynamic_cast<const kernel2D<ubyte>*>(&aKernel))) {
        kernel = aKernel.clone();
        kernel2D<ubyte>& tmp = *dynamic_cast<kernel2D<ubyte>*>(kernel);
        tmp.mirror();
        matrix<ubyte>::const_iterator it,eit;
        for (it=tmp.begin(),eit=tmp.end(),kernelAverage=0;
             it!=eit;
             ++it) {
          kernelAverage+=(*it);
        }
        kernelSize = (tmp.lastRow()-tmp.firstRow()+1)*
                     (tmp.lastColumn()-tmp.firstColumn()+1);
        kernelAverage/=double(kernelSize);
      } else {
        // its a channel8 or matrix<ubyte>
        kernel2D<ubyte>* tmp = new kernel2D<ubyte>();
        const matrix<ubyte>& mat =
          *dynamic_cast<const matrix<ubyte>*>(&aKernel);
        tmp->castFrom(mat,1-mat.rows(),1-mat.columns());
        tmp->mirror();
        kernel = tmp;
        matrix<ubyte>::const_iterator it,eit;
        for (it=tmp->begin(),eit=tmp->end(),kernelAverage=0;
             it!=eit;
             ++it) {
          kernelAverage+=(*it);
        }
        kernelSize = mat.rows()*mat.columns();
        kernelAverage/=double(kernelSize);
      }
    } else if (notNull(dynamic_cast<const matrix<double>*>(&aKernel))) {
      // there are now two possibilities: its a matrix<double>
      // or its a kernel2D<double>
      if (notNull(dynamic_cast<const kernel2D<double>*>(&aKernel))) {
        kernel = aKernel.clone();
        kernel2D<double>& tmp = *dynamic_cast<kernel2D<double>*>(kernel);
        tmp.mirror();
        kernelSize = (tmp.lastRow()-tmp.firstRow()+1)*
                     (tmp.lastColumn()-tmp.firstColumn()+1);
        kernelAverage = tmp.sumOfElements()/double(kernelSize);
      } else {
        // its a matrix<double>
        kernel2D<double>* tmp = new kernel2D<double>();
        const matrix<double>& mat =
          *dynamic_cast<const matrix<double>*>(&aKernel);
        tmp->castFrom(mat,1-mat.rows(),1-mat.columns());
        tmp->mirror();
        kernel = tmp;
        kernelSize = mat.rows()*mat.columns();
        kernelAverage = mat.sumOfElements()/double(kernelSize);
      }

      // continue with 1D kernels!
    } else if (notNull(dynamic_cast<const vector<double>*>(&aKernel))) {
      // there are now two possibilities: its a vector<double>
      // or its a kernel1D<double>
      if (notNull(dynamic_cast<const kernel1D<double>*>(&aKernel))) {
        kernel = aKernel.clone();
        kernel1D<double>& tmp = *dynamic_cast<kernel1D<double>*>(kernel);
        tmp.mirror();
        kernelSize = tmp.lastIdx()-tmp.firstIdx()+1;
        kernelAverage = tmp.sumOfElements()/double(kernelSize);
      } else {
        // its a vector<double>
        kernel1D<double>* tmp = new kernel1D<double>();
        const vector<double>& vct =
          *dynamic_cast<const vector<double>*>(&aKernel);
        tmp->castFrom(vct,1-vct.size());
        tmp->mirror();
        kernel = tmp;
        kernelSize = vct.size();
        kernelAverage = vct.sumOfElements()/double(kernelSize);
      }
    } else if (notNull(dynamic_cast<const vector<float>*>(&aKernel))) {
      // there are now two possibilities: its a vector<float>
      // or its a kernel1D<float>
      if (notNull(dynamic_cast<const kernel1D<float>*>(&aKernel))) {
        kernel = aKernel.clone();
        kernel1D<float>& tmp = *dynamic_cast<kernel1D<float>*>(kernel);
        tmp.mirror();
        kernelSize = tmp.lastIdx()-tmp.firstIdx()+1;
        kernelAverage = tmp.sumOfElements()/double(kernelSize);
      } else {
        // its a vector<float>
        kernel1D<float>* tmp = new kernel1D<float>();
        const vector<float>& vct =
          *dynamic_cast<const vector<float>*>(&aKernel);
        tmp->castFrom(vct,1-vct.size());
        tmp->mirror();
        kernel = tmp;
        kernelSize = vct.size();
        kernelAverage = vct.sumOfElements()/double(kernelSize);
      }

      // continue with separable kernels
    } else if (notNull(dynamic_cast<const sepKernel<float>*>(&aKernel))) {
      kernel = aKernel.clone();
      sepKernel<float>& tmp = *dynamic_cast<sepKernel<float>*>(kernel);
      tmp.mirror();
      kernelSize = tmp.getRowFilter(0).size()*
                   tmp.getColFilter(0).size();

      kernelAverage = double(tmp.sumOfElements())/double(kernelSize);
    } else if (notNull(dynamic_cast<const sepKernel<ubyte>*>(&aKernel))) {
      kernel = aKernel.clone();
      sepKernel<ubyte>& tmp = *dynamic_cast<sepKernel<ubyte>*>(kernel);
      tmp.mirror();
      kernelSize = tmp.getRowFilter(0).size()*
                   tmp.getColFilter(0).size();
      kernelAverage = double(tmp.sumOfElements())/double(kernelSize);
    } else if (notNull(dynamic_cast<const sepKernel<double>*>(&aKernel))) {
      kernel = aKernel.clone();
      sepKernel<double>& tmp = *dynamic_cast<sepKernel<double>*>(kernel);
      tmp.mirror();
      kernelSize = tmp.getRowFilter(0).size()*
                    tmp.getColFilter(0).size();

      kernelAverage = double(tmp.sumOfElements())/double(kernelSize);
    } else {
      throw exception("Correlation: Kernel type not supported");
    }

  }

  // set the mask to use
  void correlation::parameters::setMask(const channel8& aMask) {
    delete mask;
    mask=dynamic_cast<channel8*>(aMask.clone());
  }

  // return the mask in use, or throw exception if none present
  const channel8& correlation::parameters::getMask() const {
    if (notNull(mask)) {
      return *mask;
    } else {
      throw invalidParametersException(getTypeName());
    }
  }

  // decide whether to use the mask
  void correlation::parameters::setUseMask(const bool b) {
    useMask = b;
  }

  // tell whether the mask is used
  const bool& correlation::parameters::getUseMask() const {
    return useMask;
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool correlation::parameters::write(ioHandler& handler,
                                      const bool complete) const
# else
  bool correlation::parameters::writeMS(ioHandler& handler,
                                        const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (isNull(kernel)) {
        lti::write(handler,"kernelType","none");
      } else {
        lti::write(handler,"kernelType",kernel->getTypeName());
        lti::write(handler,"kernel",getKernel());
      }
      lti::write(handler,"maskPresent",notNull(mask));
      if (notNull(mask)) {
        mask->write(handler);
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool correlation::parameters::write(ioHandler& handler,
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
  bool correlation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool correlation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      delete kernel;
      kernel = 0;

      std::string str;
      lti::read(handler,"kernelType",str);
      if (str == "kernel1D") {
        // assume float (maybe to be change in the near future ;-)
        kernel1D<float> kern;
        lti::read(handler,"kernel",kern);
        kernel = kern.clone();
        kernelSize = kern.lastIdx()-kern.firstIdx()+1;
        kernelAverage = kern.sumOfElements()/double(kernelSize);
      } else if (str == "kernel2D") {
        // assume float (maybe to be change in the near future ;-)
        kernel2D<float> kern;
        lti::read(handler,"kernel",kern);
        kernel = kern.clone();
        kernelSize = (kern.lastRow()-kern.firstRow()+1)*
                     (kern.lastColumn()-kern.firstColumn()+1);
        kernelAverage = kern.sumOfElements()/double(kernelSize);
      } else if (str == "sepKernel") {
        // assume float (maybe to be change in the near future ;-)
        sepKernel<float> kern;
        lti::read(handler,"kernel",kern);
        kernel = kern.clone();
        kernelSize = kern.getRowFilter(0).size()*
                     kern.getColFilter(0).size();
        kernelAverage = double(kern.sumOfElements())/double(kernelSize);
      }

      delete mask;
      mask = 0;
      bool b;

      lti::read(handler,"maskPresent",b);
      if (b) {
        channel8* newMask = new channel8;
        newMask->read(handler);
        mask = newMask;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool correlation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // correlation
  // --------------------------------------------------

  // default constructor
  correlation::correlation()
    : filter() {
    parameters tmpParam;
    setParameters(tmpParam);
  }

  // constructor to set filter kernel directly
  correlation::correlation(const mathObject& aKernel)
    : filter() {
    parameters tmpParam;
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // copy constructor
  correlation::correlation(const correlation& other)
    : filter()  {
    copy(other);
  }

  // destructor
  correlation::~correlation() {
  }

  // returns the name of this type
  const char* correlation::getTypeName() const {
    return "correlation";
  }

  // copy member
  correlation& correlation::copy(const correlation& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* correlation::clone() const {
    return new correlation(*this);
  }

  // return parameters
  const correlation::parameters& correlation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void correlation::setKernel(const mathObject& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool correlation::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type matrix<float>!
  bool correlation::apply(matrix<float>& srcdest) const {
    matrix<float> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type matrix<float>!
  bool correlation::apply(dmatrix& srcdest) const {
    dmatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };


  // On place apply for type vector<channel8::value_type>!
  bool correlation::apply(vector<channel8::value_type>& srcdest) const {
    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type vector<channel::value_type>!
  bool correlation::apply(vector<channel::value_type>& srcdest) const {

    vector<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);

      return true;
    }

    return false;
  };

  // On place apply for type dvector!
  bool correlation::apply(dvector& srcdest) const {

    dvector tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);

      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool correlation::apply(const channel8& src,channel8& dest) const {

    // there are two possibilities: kernel2D or sepKernel
    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper2D<ubyte,classic<ubyte,int> >    convClas;
    typedef convHelper2D<ubyte,coefficient<ubyte,int> > convCoef;
    typedef convHelper2D<ubyte,criterium1<ubyte,int> >  convC1;
    typedef convHelper2D<ubyte,criterium2<ubyte,int> >  convC2;
    typedef convHelper2D<ubyte,criterium3<ubyte,int> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;
        conv.getAccumulator().setData(int(param.getKernelAverage()),
                                      param.getKernelSize());

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };

  // On copy apply for type channel!
  bool correlation::apply(const matrix<float>& src,
                                matrix<float>& dest) const {

    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper2D<float,classic<float> >     convClas;
    typedef convHelper2D<float,coefficient<float> > convCoef;
    typedef convHelper2D<float,criterium1<float> >  convC1;
    typedef convHelper2D<float,criterium2<float> >  convC2;
    typedef convHelper2D<float,criterium3<float> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;
        conv.getAccumulator().setData(static_cast<float>(param.getKernelAverage()),
                                      static_cast<float>(param.getKernelSize()));

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      } while(false);
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };

  // On copy apply for type dmatrix!
  bool correlation::apply(const dmatrix& src,dmatrix& dest) const {

    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper2D<double,classic<double> >     convClas;
    typedef convHelper2D<double,coefficient<double> > convCoef;
    typedef convHelper2D<double,criterium1<double> >  convC1;
    typedef convHelper2D<double,criterium2<double> >  convC2;
    typedef convHelper2D<double,criterium3<double> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;
        conv.getAccumulator().setData(param.getKernelAverage(),
                                      param.getKernelSize());

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        if (!param.getUseMask()) {
          if (conv.isKernelSeparable()) {
            conv.applySep(src,dest,param);
          } else {
            conv.apply(src,dest,param);
          }
        } else {
          conv.setMask(*param.mask);
          conv.applyMask(src,dest,param);
        }
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };


  // On copy apply for type vector<channel8::value_type>!
  bool correlation::apply(const vector<channel8::value_type>& src,
                                vector<channel8::value_type>& dest) const {

    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper1D<ubyte,classic<ubyte,int> >     convClas;
    typedef convHelper1D<ubyte,coefficient<ubyte,int> > convCoef;
    typedef convHelper1D<ubyte,criterium1<ubyte,int> >  convC1;
    typedef convHelper1D<ubyte,criterium2<ubyte,int> >  convC2;
    typedef convHelper1D<ubyte,criterium3<ubyte,int> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;

        const ubyte kernelAverage =
          static_cast<ubyte>(param.getKernelAverage());
        const ubyte kernelSize =
          static_cast<ubyte>(param.getKernelSize());

        conv.getAccumulator().setData(kernelAverage,
                                      kernelSize);

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };

  // On copy apply for type vector<channel::value_type>!
  bool correlation::apply(const vector<float>& src,
                                    vector<float>& dest) const {

    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper1D<float,classic<float> >     convClas;
    typedef convHelper1D<float,coefficient<float> > convCoef;
    typedef convHelper1D<float,criterium1<float> >  convC1;
    typedef convHelper1D<float,criterium2<float> >  convC2;
    typedef convHelper1D<float,criterium3<float> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;

        conv.getAccumulator().setData(static_cast<float>(param.getKernelAverage()),
                                      static_cast<float>(param.getKernelSize()));

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };

  // On copy apply for type dvector!
  bool correlation::apply(const dvector& src,
                                dvector& dest) const {

    const parameters& param = getParameters();

    // this typedefs are required as a workaround for another VC++ bug!
    typedef convHelper1D<double,classic<double> >     convClas;
    typedef convHelper1D<double,coefficient<double> > convCoef;
    typedef convHelper1D<double,criterium1<double> >  convC1;
    typedef convHelper1D<double,criterium2<double> >  convC2;
    typedef convHelper1D<double,criterium3<double> >  convC3;

    switch(param.mode) {
      case parameters::Classic: {
        convClas conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::Coefficient: {
        convCoef conv;
        conv.getAccumulator().setData(param.getKernelAverage(),
                                      param.getKernelSize());

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C1: {
        convC1 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C2: {
        convC2 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      case parameters::C3: {
        convC3 conv;

        if (!conv.setKernel(param.kernel)) {
          throw invalidParametersException(getTypeName());
        }

        conv.apply(src,dest,param);
      }
      break;
      default:
        throw exception("Invalid correlation mode");
    }

    return true;
  };

  // --------------------------------------------------
  // specializations!
  // --------------------------------------------------


  // -------------------------------
  // Classic

  template<>
  void lti::correlation::classic<ubyte,int>::accumulate(
      const ubyte& filter,
      const ubyte& src) {
    state += static_cast<int>(src)*static_cast<int>(filter);
  }

  /*
   * specialization for integers
   */
  template<>
  int lti::correlation::classic<int,int>::getResult() const {
    return state/norm;
  }

  template<>
  ubyte lti::correlation::classic<ubyte,int>::getResult() const {
    return static_cast<ubyte>(state/norm);
  }

  // -------------------------------
  // Coefficient

  template<>
  void lti::correlation::coefficient<ubyte,int>::accumulate(
      const ubyte& bfilter,
      const ubyte& bsrc) {
    int src = static_cast<int>(bsrc);
    int filter = static_cast<int>(bfilter);

    int tmp = static_cast<int>(filter-kernAvg);
    sumW+=tmp;
    sumFW+=(src*tmp);
    sumFF+=(src*src);
    sumF+=src;
  }

  /*
   * specialization for integers
   */
  template<>
  int lti::correlation::coefficient<int,int>::getResult() const {
    return static_cast<int>(norm*(sumFW-(sumF*sumW/kernSize))/
                            sqrt((sumFF-(sumF*sumF/kernSize))*sumWW));
  }

  template<>
  ubyte lti::correlation::coefficient<ubyte,int>::getResult() const {
    return static_cast<ubyte>((((sumFW-(sumF*sumW/kernSize))/
                                sqrt((sumFF-(sumF*sumF/kernSize))*sumWW))+1)*
                              norm/2);
  }


  // -------------------------------
  // match criterium 1

  /*
   * specialization for integers
   */
  template<>
  int lti::correlation::criterium1<int,int>::getResult() const {
    return static_cast<int>((norm)/(state+1));
  }

  template<>
  ubyte lti::correlation::criterium1<ubyte,int>::getResult() const {
    return static_cast<ubyte>(static_cast<int>(norm)/(state+1));
  }

}
