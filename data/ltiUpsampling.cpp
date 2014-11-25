/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiUpsampling.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.5.2000
 * revisions ..: $Id: ltiUpsampling.cpp,v 1.13 2006/09/05 10:33:10 ltilib Exp $
 */

#include "ltiUpsampling.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {
  // --------------------------------------------------
  // upsampling::parameters
  // --------------------------------------------------

  // default constructor
  upsampling::parameters::parameters()
    : filter::parameters() {

    kernel1D<channel::value_type> tmp(-1,1,0.5f);
    tmp.at(0)=1;
    kernel = new sepKernel<channel::value_type>(tmp);

    factor = point(2,2);
  }

  // copy constructor
  upsampling::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    kernel = 0;
    copy(other);
  }

  // destructor
  upsampling::parameters::~parameters() {
    delete kernel;
  }

  // get type name
  const char* upsampling::parameters::getTypeName() const {
    return "upsampling::parameters";
  }

  // copy member

  upsampling::parameters&
    upsampling::parameters::copy(const parameters& other) {
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

    factor = other.factor;

    return *this;
  }

  const mathObject& upsampling::parameters::getKernel() const {
    if (isNull(kernel)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel;
  }

  void upsampling::parameters::setKernel(const mathObject& aKernel) {
    delete kernel;
    kernel = 0;
    kernel = aKernel.clone();
  }


  // clone member
  functor::parameters* upsampling::parameters::clone() const {
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
  bool upsampling::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool upsampling::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"factor",factor);
      if (isNull(kernel)) {
        lti::write(handler,"kernelType","none");
      } else {
        lti::write(handler,"kernelType",kernel->getTypeName());
        lti::write(handler,"kernel",getKernel());
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
  bool upsampling::parameters::write(ioHandler& handler,
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
  bool upsampling::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool upsampling::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"factor",factor);
      std::string str;
      lti::read(handler,"kernelType",str);
      if (str == "kernel1D") {
        // assume float (maybe to be change in the near future ;-)
        kernel1D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "kernel2D") {
        // assume float (maybe to be change in the near future ;-)
        kernel2D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "sepKernel") {
        // assume float (maybe to be change in the near future ;-)
        sepKernel<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else {
        delete kernel;
        kernel = 0;
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
  bool upsampling::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // upsampling
  // --------------------------------------------------

  // default constructor
  upsampling::upsampling()
    : filter(){
    parameters tmp;
    setParameters(tmp);
  }

  // default constructor
  upsampling::upsampling(const parameters& par)
    : filter(){
    setParameters(par);
  }

  // copy constructor
  upsampling::upsampling(const upsampling& other)
    : filter()  {
    copy(other);
  }

  // destructor
  upsampling::~upsampling() {
  }

  // returns the name of this type
  const char* upsampling::getTypeName() const {
    return "upsampling";
  }

  // copy member
  upsampling& upsampling::copy(const upsampling& other) {
      filter::copy(other);
    return (*this);
  }

  // clone member
  functor* upsampling::clone() const {
    return new upsampling(*this);
  }

  // return parameters
  const upsampling::parameters&
    upsampling::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void upsampling::setKernel(const mathObject& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool upsampling::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool upsampling::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type vector<channel8::value_type>!
  bool upsampling::apply(vector<channel8::value_type>& srcdest) const {

    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type vector<channel::value_type>!
  bool upsampling::apply(vector<channel::value_type>& srcdest) const {

    vector<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool upsampling::apply(const channel8& src,channel8& dest) const {

    point p,q;

    const parameters& param = getParameters();
    const point& step = param.factor;

    channel8 tmp;
    tmp.resize(src.rows()*step.y,src.columns()*step.x,0,false,true);

    vector<channel8::value_type>::const_iterator sit,eit;
    vector<channel8::value_type>::iterator it;

    for (p.y=0,q.y=0;p.y<src.rows();p.y++,q.y+=step.y) {
      const vector<channel8::value_type>& svct = src.getRow(p.y);
      vector<channel8::value_type>& dvct = tmp.getRow(q.y);
      for (sit=svct.begin(),eit=svct.end(),it=dvct.begin();
           sit!=eit;
           ++sit,it+=step.x) {
        (*it)=(*sit);
      }
    }

    convolution conv;
    convolution::parameters paramc;

    if (param.boundaryType != lti::NoBoundary) {
      paramc.boundaryType = (param.boundaryType == lti::Periodic) ?
                            lti::Periodic : lti::Zero;
    } else {
      paramc.boundaryType = param.boundaryType;
    }

    paramc.setKernel(param.getKernel());

    conv.setParameters(paramc);
    conv.apply(tmp,dest);

    // correct boundaries
    if ((param.boundaryType == lti::Mirror) ||
        (param.boundaryType == lti::Constant)) {

      point rest;
      rest.x = (dest.lastColumn())%step.x;
      rest.y = (dest.lastRow())%step.y;

      // correct rows
      if (rest.x>0) {
        int endy = dest.rows()-rest.y;
        point last = point(dest.columns()-rest.x-1,0);
        for (p.y=0;p.y<endy;p.y++) {
          last.y = p.y;
          for (p.x=last.x+1;p.x<dest.columns();++p.x) {
            dest.at(p)=dest.at(last);
          }
        }
      }

      // correct columns
      if (rest.y>0) {
        int last = dest.rows()-rest.y-1;
        const vector<channel8::value_type>& vct = dest.getRow(last);
        for (p.y=last+1;p.y<dest.rows();p.y++) {
          dest.getRow(p.y).copy(vct);
        }
      }
    }

    return true;
  };

  // On copy apply for type channel!
  bool upsampling::apply(const channel& src,channel& dest) const {
    point p,q;

    const parameters& param = getParameters();
    const point& step = param.factor;

    channel tmp;
    tmp.resize(src.rows()*step.y,src.columns()*step.x,0,false,true);

    vector<channel::value_type>::const_iterator sit,eit;
    vector<channel::value_type>::iterator it;

    for (p.y=0,q.y=0;p.y<src.rows();p.y++,q.y+=step.y) {
      const vector<channel::value_type>& svct = src.getRow(p.y);
      vector<channel::value_type>& dvct = tmp.getRow(q.y);
      for (sit=svct.begin(),eit=svct.end(),it=dvct.begin();
           sit!=eit;
           ++sit,it+=step.x) {
        (*it)=(*sit);
      }
    }

    convolution conv;
    convolution::parameters paramc;

    if (param.boundaryType != lti::NoBoundary) {
      paramc.boundaryType = (param.boundaryType == lti::Periodic) ?
                            lti::Periodic : lti::Zero;
    } else {
      paramc.boundaryType = param.boundaryType;
    }

    paramc.setKernel(param.getKernel());
    conv.setParameters(paramc);

    conv.apply(tmp,dest);

    // fix boundaries
    if ((param.boundaryType == lti::Mirror) ||
        (param.boundaryType == lti::Constant)) {

      point rest;
      rest.x = (dest.lastColumn())%step.x;
      rest.y = (dest.lastRow())%step.y;

      // fix rows
      if (rest.x>0) {
        const int endy = dest.rows()-rest.y;
        point last = point(dest.columns()-rest.x-1,0);
        for (p.y=0;p.y<endy;p.y++) {
          last.y = p.y;
          for (p.x=last.x+1;p.x<dest.columns();++p.x) {
            dest.at(p)=dest.at(last);
          }
        }
      }

      // fix columns
      if (rest.y>0) {
        int last = dest.rows()-rest.y-1;
        const vector<channel::value_type>& vct = dest.getRow(last);
        for (p.y=last+1;p.y<dest.rows();p.y++) {
          dest.getRow(p.y).copy(vct);
        }
      }

    }

    return true;
  };

  // On copy apply for type vector<channel8::value_type>!
  bool upsampling::apply(const vector<channel8::value_type>& src,
                         vector<channel8::value_type>& dest) const {

    const parameters& param = getParameters();
    const int& step = param.factor.x;

    vector<channel8::value_type> tmp;
    tmp.resize(src.size()*step,0,false,true);

    vector<channel8::value_type>::const_iterator sit,eit;
    vector<channel8::value_type>::iterator it;

    for (sit=src.begin(),eit=src.end(),it=tmp.begin();
         sit!=eit;
         ++sit,it+=step) {
      (*it)=(*sit);
    }

    convolution conv;
    convolution::parameters paramc;
    paramc.setKernel(param.getKernel());
    paramc.boundaryType = param.boundaryType;
    conv.setParameters(paramc);
    conv.apply(tmp,dest);

    return true;
  };

  // On copy apply for type vector<channel::value_type>!
  bool upsampling::apply(const vector<channel::value_type>& src,
                               vector<channel::value_type>& dest) const {

    const parameters& param = getParameters();
    const int& step = param.factor.x;

    vector<channel::value_type> tmp;
    tmp.resize(src.size()*step,0,false,true);

    vector<channel::value_type>::const_iterator sit,eit;
    vector<channel::value_type>::iterator it;

    for (sit=src.begin(),eit=src.end(),it=tmp.begin();
         sit!=eit;
         ++sit,it+=step) {
      (*it)=(*sit);
    }

    convolution conv;
    convolution::parameters paramc;
    paramc.setKernel(param.getKernel());
    paramc.boundaryType = param.boundaryType;
    conv.setParameters(paramc);
    conv.apply(tmp,dest);

    return true;
  };

}
