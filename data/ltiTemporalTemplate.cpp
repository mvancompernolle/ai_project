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
 * file .......: ltiTemporalTemplate.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 4.12.2000
 * revisions ..: $Id: ltiTemporalTemplate.cpp,v 1.8 2006/09/05 10:32:10 ltilib Exp $
 */

#include "ltiTemporalTemplate.h"

namespace lti {
  // --------------------------------------------------
  // temporalTemplate::parameters
  // --------------------------------------------------

  // default constructor
  temporalTemplate::parameters::parameters()
    : transform::parameters() {

    tau = int(5);
    threshold = 0.1f;
    useAverage = false;
  }

  // copy constructor
  temporalTemplate::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  temporalTemplate::parameters::~parameters() {
  }

  // get type name
  const char* temporalTemplate::parameters::getTypeName() const {
    return "temporalTemplate::parameters";
  }

  // copy member

  temporalTemplate::parameters&
    temporalTemplate::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    tau = other.tau;
    threshold = other.threshold;
    useAverage = false;

    return *this;
  }


  temporalTemplate::parameters&
  temporalTemplate::parameters::operator=(const parameters& other) {
    return copy(other);
  }


  // clone member
  functor::parameters* temporalTemplate::parameters::clone() const {
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
  bool temporalTemplate::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool temporalTemplate::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"tau",tau);
      lti::write(handler,"threshold",threshold);
      lti::write(handler,"useAverage",useAverage);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool temporalTemplate::parameters::write(ioHandler& handler,
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
  bool temporalTemplate::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool temporalTemplate::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"tau",tau);
      lti::read(handler,"threshold",threshold);
      lti::read(handler,"useAverage",useAverage);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool temporalTemplate::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // temporalTemplate
  // --------------------------------------------------

  // default constructor
  temporalTemplate::temporalTemplate()
    : transform(){
    parameters tmp;
    setParameters(tmp);
  }

  // copy constructor
  temporalTemplate::temporalTemplate(const temporalTemplate& other)
    : transform()  {
    copy(other);
  }

  // destructor
  temporalTemplate::~temporalTemplate() {
  }

  // returns the name of this type
  const char* temporalTemplate::getTypeName() const {
    return "temporalTemplate";
  }

  // copy member
  temporalTemplate& temporalTemplate::copy(const temporalTemplate& other) {
    transform::copy(other);
    return (*this);
  }

  // clone member
  functor* temporalTemplate::clone() const {
    return new temporalTemplate(*this);
  }

  // return parameters
  const temporalTemplate::parameters&
    temporalTemplate::getParameters() const {
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

  // On place apply for type channel!
  bool temporalTemplate::apply(channel& srcdest) {

    float threshold = difference(srcdest,srcdest);
    accumulate(srcdest,threshold);

    return true;
  };

  // On copy apply for type image!
  bool temporalTemplate::apply(const image& src,channel& dest) {
    image img(src);

    float threshold = difference(img,dest);
    accumulate(dest,threshold);

    return true;
  };

  // On copy apply for type channel!
  bool temporalTemplate::apply(const channel& src,channel& dest) {
    channel chnl(src);

    float threshold = difference(chnl,dest);
    accumulate(dest,threshold);

    return true;
  };

  // On copy apply for type channel8!
  bool temporalTemplate::apply(const channel8& src,channel& dest) {

    channel8 chnl(src);

    float threshold = difference(chnl,dest);
    accumulate(dest,threshold);

    return true;
  };

  // Clear the mhi and last images
  void temporalTemplate::reset() {
    mhi.clear();
    lastChannel.clear();
    lastChannel8.clear();
    lastImage.clear();
  }

  // Difference between the last and the given channels
  float temporalTemplate::difference(channel& newChannel, channel& diff) {

    float theThreshold;
    const parameters& param = getParameters();

    theThreshold = param.threshold;

    // if this is the first image

    if ((lastChannel.rows() == 0) ||
        (newChannel.size() != lastChannel.size())) {
      newChannel.detach(lastChannel);
      diff.resize(lastChannel.size(),0.0f,false,true);
      return theThreshold;
    }

    // calculate the difference

    vector<channel::value_type>::const_iterator vit,eit;
    channel::iterator it;

    if (param.useAverage) {

      lastChannel.detach(diff);

      int y;
      float sum = 0.0f;

      for (y=0,it=diff.begin();y<newChannel.rows();++y) {
        const vector<channel::value_type>& vct = newChannel.getRow(y);
        for (vit=vct.begin(),eit=vct.end();
             vit!=eit;
             ++vit,++it) {
          if ((*it)>(*vit)) {
            (*it) -= (*vit);
          } else {
            (*it) = (*vit)-(*it);
          }

          sum+=(*it);
        }
      }

      theThreshold *= sum/(newChannel.rows()*newChannel.columns());

    } else {

      lastChannel.detach(diff);

      int y;
      for (y=0,it=diff.begin();y<newChannel.rows();++y) {
        const vector<channel::value_type>& vct = newChannel.getRow(y);
        for (vit=vct.begin(),eit=vct.end();
             vit!=eit;
             ++vit,++it) {
          if ((*it)>(*vit)) {
            (*it) -= (*vit);
          } else {
            (*it) = (*vit)-(*it);
          }
        }
      }
    }

    newChannel.detach(lastChannel);

    return theThreshold;
  }

  // Difference between the last and the given channels
  float temporalTemplate::difference(channel8& newChannel8,
                                     channel& diff) {

    float theThreshold;
    const parameters& param = getParameters();

    theThreshold = param.threshold*255.0f;

    // if this is the first image

    if ((lastChannel8.rows() == 0) ||
        (newChannel8.size()!=lastChannel8.size())) {
      newChannel8.detach(lastChannel8);
      diff.resize(lastChannel8.size(),0.0f,false,true);
      return theThreshold;
    }

    // calculate the difference

    channel8::iterator c8it,l8it,e8it;
    channel::iterator it;

    diff.resize(newChannel8.size(),0.0f,false,false);


    if (param.useAverage) {

      float sum = 0.0f;

      for (it=diff.begin(),l8it=lastChannel8.begin(),c8it=newChannel8.begin(),
             e8it=newChannel8.end();
           c8it!=e8it;
           ++c8it,++it,++l8it) {
        if ((*l8it) > (*c8it)) {
          (*it) = static_cast<float>((*l8it)-(*c8it));
        } else {
          (*it) = static_cast<float>((*c8it)-(*l8it));
        }

        sum+=(*it);
      }

      theThreshold = param.threshold*sum/
        (newChannel8.rows()*newChannel8.columns());

    } else {

      for (it=diff.begin(),l8it=lastChannel8.begin(),c8it=newChannel8.begin(),
             e8it=newChannel8.end();
           c8it!=e8it;
           ++c8it,++it,++l8it) {
        if ((*l8it) > (*c8it)) {
          (*it) = static_cast<float>((*l8it)-(*c8it));
        } else {
          (*it) = static_cast<float>((*c8it)-(*l8it));
        }
      }
    }

    newChannel8.detach(lastChannel8);

    return theThreshold;
  }

  // Difference between the last and the given channels
  float temporalTemplate::difference(image& newImage,
                                     channel& diff) {

    float theThreshold;
    const parameters& param = getParameters();

    theThreshold = param.threshold*255.0f;

    // if this is the first image

    if ((lastImage.rows() == 0) ||
        (newImage.size()!=lastImage.size())) {
      newImage.detach(lastImage);
      diff.resize(lastImage.size(),0.0f,false,true);
      return theThreshold;
    }

    // calculate the difference

    image::iterator c8it,l8it,e8it;
    channel::iterator it;

    diff.resize(newImage.size(),0.0f,false,false);

    if (param.useAverage) {

      trgbPixel<float> sum(0,0,0);
      trgbPixel<float> pxDiff;

      for (it=diff.begin(),l8it=lastImage.begin(),c8it=newImage.begin(),
             e8it=newImage.end();
           c8it!=e8it;
           ++c8it,++it,++l8it) {


        if ((*l8it).getRed() > (*c8it).getRed()) {
          pxDiff.setRed(static_cast<float>((*l8it).getRed()-
                                           (*c8it).getRed()));
        } else {
          pxDiff.setRed(static_cast<float>((*c8it).getRed()-
                                           (*l8it).getRed()));
        }

        if ((*l8it).getGreen() > (*c8it).getGreen()) {
          pxDiff.setGreen(static_cast<float>((*l8it).getGreen()-
                                             (*c8it).getGreen()));
        } else {
          pxDiff.setGreen(static_cast<float>((*c8it).getGreen()-
                                             (*l8it).getGreen()));
        }

        if ((*l8it).getBlue() > (*c8it).getBlue()) {
          pxDiff.setBlue(static_cast<float>((*l8it).getBlue()-
                                            (*c8it).getBlue()));
        } else {
          pxDiff.setBlue(static_cast<float>((*c8it).getBlue()-
                                            (*l8it).getBlue()));
        }

        sum+=pxDiff;

        (*it) = pxDiff.absSqr();
      }

      sum.divide(static_cast<float>(newImage.rows()*newImage.columns()));
      theThreshold = param.threshold*sum.absSqr();

    } else {

      trgbPixel<float> pxDiff;

      for (it=diff.begin(),l8it=lastImage.begin(),c8it=newImage.begin(),
             e8it=newImage.end();
           c8it!=e8it;
           ++c8it,++it,++l8it) {

        if ((*l8it).getRed() > (*c8it).getRed()) {
          pxDiff.setRed(static_cast<float>((*l8it).getRed()-(*c8it).getRed()));
        } else {
          pxDiff.setRed(static_cast<float>((*c8it).getRed()-(*l8it).getRed()));
        }

        if ((*l8it).getGreen() > (*c8it).getGreen()) {
          pxDiff.setGreen(static_cast<float>((*l8it).getGreen()-
                                             (*c8it).getGreen()));
        } else {
          pxDiff.setGreen(static_cast<float>((*c8it).getGreen()-
                                             (*l8it).getGreen()));
        }

        if ((*l8it).getBlue() > (*c8it).getBlue()) {
          pxDiff.setBlue(static_cast<float>((*l8it).getBlue()-
                                            (*c8it).getBlue()));
        } else {
          pxDiff.setBlue(static_cast<float>((*c8it).getBlue()-
                                            (*l8it).getBlue()));
        }

        if ((pxDiff.getRed()<theThreshold) &&
            (pxDiff.getGreen()<theThreshold) &&
            (pxDiff.getBlue()<theThreshold)) {
          (*it)=0.0f;
        } else {
          (*it)=255.0f;
        }
      }
    }

    newImage.detach(lastImage);

    return theThreshold;
  }

  void temporalTemplate::accumulate(channel& diff,const float& threshold) {
    channel::iterator mit,dit,emit;

    if ((mhi.rows() == 0) ||
        (mhi.size() != diff.size())) {
      mhi.resize(diff.size(),0.0f,false,false);
      for (mit=mhi.begin(),emit=mhi.end(),dit=diff.begin();
           mit!=emit;
           ++mit,++dit) {
        (*mit) = ((*dit)<threshold) ? 0.0f : 1.0f;
        (*dit) = (*mit);
      }
      return;
    }

    const parameters& param = getParameters();

    const float delta = 1.0f/param.tau;

    for (mit=mhi.begin(),emit=mhi.end(),dit=diff.begin();
         mit!=emit;
         ++mit,++dit) {
      (*mit) = ((*dit)<threshold) ? (((*mit)>delta)?(*mit)-delta:0.0f) : 1.0f;
      (*dit) = (*mit);
    }
  }
}
