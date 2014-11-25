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
 * file .......: ltiQmf.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 14.12.2000
 * revisions ..: $Id: ltiQmf.cpp,v 1.9 2006/09/05 10:26:46 ltilib Exp $
 */

#include "ltiQmf.h"
#include "ltiQmf_template.h"
#include "ltiQmfKernels.h"

namespace lti {
  // --------------------------------------------------
  // qmf::parameters
  // --------------------------------------------------

  // default constructor
  qmf::parameters::parameters()
    : filter::parameters() {
    kernel.copy(tap9Symmetric());
    levels = int(3);
  }

  // copy constructor
  qmf::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    copy(other);
  }

  // destructor
  qmf::parameters::~parameters() {
  }

  // get type name
  const char* qmf::parameters::getTypeName() const {
    return "qmf::parameters";
  }

  // copy member

  qmf::parameters&
    qmf::parameters::copy(const parameters& other) {
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

    kernel = other.kernel;
    levels = other.levels;

    return *this;
  }

  // alias for copy member
  qmf::parameters&
    qmf::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* qmf::parameters::clone() const {
    return new parameters(*this);
  }

  const mathObject& qmf::parameters::getKernel() const {
    return kernel;
  }

  void qmf::parameters::setKernel(const mathObject& aKernel) {
    if (notNull(dynamic_cast<const kernel1D<float>*>(&aKernel))) {
      kernel.copy(*dynamic_cast<const kernel1D<float>*>(&aKernel));
    } else if (notNull(dynamic_cast<const kernel1D<double>*>(&aKernel))) {
      kernel.castFrom(*dynamic_cast<const kernel1D<double>*>(&aKernel));
    } else if (notNull(dynamic_cast<const kernel1D<int>*>(&aKernel))) {
      kernel.castFrom(*dynamic_cast<const kernel1D<int>*>(&aKernel));
    } else if (notNull(dynamic_cast<const kernel1D<ubyte>*>(&aKernel))) {
      kernel.castFrom(*dynamic_cast<const kernel1D<ubyte>*>(&aKernel));
    }
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool qmf::parameters::write(ioHandler& handler,
                              const bool complete) const
# else
  bool qmf::parameters::writeMS(ioHandler& handler,
                                const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"kernel",kernel);
      lti::write(handler,"levels",levels);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (filter::parameters::* p_writeMS)(ioHandler&,const bool) const =
      filter::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool qmf::parameters::write(ioHandler& handler,
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
  bool qmf::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool qmf::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"kernel",kernel);
      lti::read(handler,"levels",levels);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (filter::parameters::* p_readMS)(ioHandler&,const bool) =
      filter::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool qmf::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // qmf
  // --------------------------------------------------

  // default constructor
  qmf::qmf()
    : filter(){
    parameters defaultParam;
    setParameters(defaultParam);
  }

  // default constructor
  qmf::qmf(const qmf::parameters& par)
    : filter() {
    setParameters(par);
  }

  // copy constructor
  qmf::qmf(const qmf& other)
    : filter()  {
    copy(other);
  }

  // destructor
  qmf::~qmf() {
  }

  // returns the name of this type
  const char* qmf::getTypeName() const {
    return "qmf";
  }

  // copy member
  qmf& qmf::copy(const qmf& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* qmf::clone() const {
    return new qmf(*this);
  }

  // return parameters
  const qmf::parameters&
    qmf::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  /*
   * returns a list with the bottom-left points of each band.
   * The first element will correspond to the last valid indices of the
   * original vector/image.  The second one will be the first one divided by
   * two, and so on.  If the last apply was made for vectors, the 'y' component
   * of the points will be zero.
   */
  const vector<point>& qmf::getLastPartitioning() const {
    return lastPartitioning;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  /*
   * operates on the given %parameter.
   * @param srcdest channel with the source data.  The result
   *                 will be left here too.
   * @result a reference to the <code>srcdest</code>.
   */
  bool qmf::apply(vector<float>& srcdest) {

    vector<float> tmp;

    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  }



  /*
   * operates on a copy of the given %parameters.
   * @param src vector with the source data.
   * @param dest vector where the result will be left.
   * @result a reference to the <code>dest</code>.
   */
  bool qmf::apply(const vector<float>& src,
                        vector<float>& dest) {

    const parameters& param = getParameters();

    convolve1D<float,float> conv(param.kernel);

    if (param.levels>1) {
      int i,border;

      lastPartitioning.resize(param.levels+1,point(),false,false);

      vector<float> tmp;

      border = src.lastIdx();
      lastPartitioning.at(0)=point(border,0);
      conv.apply(src,0,border,dest,param);

      for (i=1;i<param.levels;++i) {
        border = ((border/2)+(border%2));
        lastPartitioning.at(i)=point(border,0);
        conv.reinitialize();
        conv.apply(dest,0,border,tmp,param);
        dest.fill(tmp,0,border);
      }

      border = ((border/2)+(border%2));
      lastPartitioning.at(i)=point(border,0);

    } else {
      dest.copy(src);
    }

    return true;
  }

  // On place apply for type channel!
  bool qmf::apply(channel& srcdest) {

    channel tmp;

    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On copy apply for type channel!
  bool qmf::apply(const channel& src,channel& dest) {

    const parameters& param = getParameters();

    convolve1D<float,float> conv(param.kernel);

    if (param.levels>1) {

      lastPartitioning.resize(param.levels+1,point(),false,false);

      int i;
      rectangle border(point(0,0),src.size());
      rectangle area = border;
      area.br.subtract(point(1,1));

      lastPartitioning.at(0) = area.br;

      channel tmp,tmp2;

      conv.applyRow(src,area,tmp,param);

      conv.reinitialize();
      conv.applyCol(tmp,area,dest,param);

      for (i=1;i<param.levels;++i) {
        border.br = ((border.br/2)+(border.br%2));
        area.br = border.br-point(1,1);
        lastPartitioning.at(i) = area.br;

        conv.reinitialize();
        conv.applyRow(dest,area,tmp,param);

        conv.reinitialize();
        conv.applyCol(tmp,area,tmp2,param);

        dest.fill(tmp2,area);
      }

      border.br = ((border.br/2)+(border.br%2));
      area.br = border.br-point(1,1);
      lastPartitioning.at(i) = area.br;

    } else {
      dest.copy(src);
    }

    return true;
  };

  /*
   * generate the appropriate low and high-pass kernel pair which
   * correponds to the low-pass kernel given in the parameters object.
   * @param lpk the kernel where the resulting low-pass kernel will be
   *            left.
   * @param hpk the kernel where the resulting high-pass kernel will be
   *            left.
   */
  void qmf::generateHighPassKernel(const kernel1D<float>& src,
                                   kernel1D<float>& hpk) const {
    convolve<float,float> conv;
    conv.generateHighPassKernel(src,hpk);
  }


  /*
   * accumulator (specialization for ubyte elements)
   */
  template<>
  void lti::qmf::convolve<ubyte,int>::accumulate(const ubyte& filter,
                                                          const ubyte& src,
                                                           int& result) {
    result += static_cast<int>(filter)*static_cast<int>(src);
  }

  /*
   * get result (specialization for integers)
   */
  template<>
  int lti::qmf::convolve<int,int>::getResult(const int& result) {
    return (result/norm);
  }

  /*
   * get result (specialization for ubyte)
   */
  template<>
  ubyte lti::qmf::convolve<ubyte,int>::getResult(const int& result) {
    return (result/norm);
  }
}
