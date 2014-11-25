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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLoadVectorImageList.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 28.11.2003
 * revisions ..: $Id: ltiLoadVectorImageList.cpp,v 1.7 2006/09/05 10:36:43 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiLoadVectorImageList.h"

namespace lti {
  // --------------------------------------------------
  // loadVectorImageList::parameters
  // --------------------------------------------------

  // default constructor
  loadVectorImageList::parameters::parameters()
    : loadImageList::parameters(), downsamplingParam() {
    
    downsample = false;
  }

  // copy constructor
  loadVectorImageList::parameters::parameters(const parameters& other)
    : loadImageList::parameters() {
    copy(other);
  }

  // destructor
  loadVectorImageList::parameters::~parameters() {
  }

  // get type name
  const char* loadVectorImageList::parameters::getTypeName() const {
    return "loadVectorImageList::parameters";
  }

  // copy member

  loadVectorImageList::parameters&
    loadVectorImageList::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    loadImageList::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    loadImageList::parameters& (loadImageList::parameters::* p_copy)
      (const loadImageList::parameters&) =
      loadImageList::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      downsamplingParam.copy(other.downsamplingParam);
      downsample = other.downsample;

    return *this;
  }

  // alias for copy member
  loadVectorImageList::parameters&
    loadVectorImageList::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* loadVectorImageList::parameters::clone() const {
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
  bool loadVectorImageList::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool loadVectorImageList::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"downsamplingParam",downsamplingParam);
      lti::write(handler,"downsample",downsample);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && loadImageList::parameters::write(handler,false);
# else
    bool (loadImageList::parameters::* p_writeMS)(ioHandler&,const bool) const =
      loadImageList::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool loadVectorImageList::parameters::write(ioHandler& handler,
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
  bool loadVectorImageList::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool loadVectorImageList::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"downsamplingParam",downsamplingParam);
      lti::read(handler,"downsample",downsample);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && loadImageList::parameters::read(handler,false);
# else
    bool (loadImageList::parameters::* p_readMS)(ioHandler&,const bool) =
      loadImageList::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool loadVectorImageList::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // loadVectorImageList
  // --------------------------------------------------

  // default constructor
  loadVectorImageList::loadVectorImageList()
    : loadImageList(), down() {


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  loadVectorImageList::loadVectorImageList(const parameters& par)
    : loadImageList(), down() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  loadVectorImageList::loadVectorImageList(const loadVectorImageList& other)
    : loadImageList(), down() {
    copy(other);
  }

  loadVectorImageList::loadVectorImageList(const std::string& name,
                       loadImageList::parameters::eOperationMode mode)
    : loadImageList(), down() {

    parameters defaultParameters;
    defaultParameters.mode=mode;
    if (mode==parameters::ListFileName) {
      defaultParameters.listFileName=name;
    } else if (mode==parameters::DirName) {
      defaultParameters.dirName=name;
    }
    // set the default parameters
    setParameters(defaultParameters);
  }

  loadVectorImageList::loadVectorImageList
  (const std::list<std::string>& names)
    : loadImageList(), down() {
    
    parameters defaultParameters;
    defaultParameters.mode=parameters::NameList;
    defaultParameters.nameList=names;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // destructor
  loadVectorImageList::~loadVectorImageList() {
  }

  // returns the name of this type
  const char* loadVectorImageList::getTypeName() const {
    return "loadVectorImageList";
  }

  // copy member
  loadVectorImageList&
  loadVectorImageList::copy(const loadVectorImageList& other) {
    loadImageList::copy(other);
    
    down.copy(other.down);
    
    return (*this);
  }

  // alias for copy member
  loadVectorImageList&
  loadVectorImageList::operator=(const loadVectorImageList& other) {
    return (copy(other));
  }


  // clone member
  functor* loadVectorImageList::clone() const {
    return new loadVectorImageList(*this);
  }

  // return parameters
  const loadVectorImageList::parameters&
    loadVectorImageList::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool loadVectorImageList::updateParameters() {

    bool b=true;    
    b = b && down.setParameters(getParameters().downsamplingParam);

    return b;
  }


  bool loadVectorImageList::read(ioHandler& handler,
                                 const bool complete) {

    bool b = loadImageList::read(handler,complete);
    b = b && setParameters(getParameters());
    return b;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool loadVectorImageList::apply(fvector& img) {
    int r,c;
    return apply(img,r,c);
  }
  
  // On place apply for type fvector!
  bool loadVectorImageList::apply(fvector& img, int& rows, int& columns) {

    channel ch;
    if (!loadImageList::apply(ch)) {
      img.clear();
      return false;
    }

    if (getParameters().downsample) {
      down.apply(ch);
    }

    rows=ch.rows();
    columns=ch.columns();
    int len = rows*columns;
    img.resize(len,0.f,false,false);
    img.fill(&ch.at(0),0,len-1);
    
    return true;
  };

  bool loadVectorImageList::apply(dvector& img) {
    int r,c;
    return apply(img,r,c);
  }

  // On place apply for type fvector!
  bool loadVectorImageList::apply(dvector& img, int& rows, int& columns) {

    channel ch;
    if (!loadImageList::apply(ch)) {
      img.clear();
      return false;
    }

    if (getParameters().downsample) {
      down.apply(ch);
    }

    rows=ch.rows();
    columns=ch.columns();

    fvector tmp;
    ch.detach(tmp);
    img.castFrom(tmp);

    return true;
  };

  bool loadVectorImageList::apply(fmatrix& images) {
    int r,c;
    return apply(images,r,c);
  }

  // On place apply for type fmatrix!
  bool loadVectorImageList::apply(fmatrix& images, int& rows, int& columns) {

    bool b=true;
    
    const parameters& param=getParameters();

    channel ch;
    int r=size();
    int c;
    
    if (hasNext()) {
      if (!loadImageList::apply(ch)) {
        images.clear();
        return false;
      }
      if (param.downsample) {
        down.apply(ch);
      }
      rows=ch.rows();
      columns=ch.columns();
      c=rows*columns;
      images.resize(r,c,0.f,false,false);
      images.getRow(0).fill(&ch.at(0),0,c-1);
    } else {
      setStatusString("Empty image list\n");
      images.clear();
      return false;
    }

    int i;
    for (i=1; i<r; i++) {
      b = b && loadImageList::apply(ch); 
      if (param.downsample) {
        down.apply(ch);
      }
      // check if all images have the same size
      assert(ch.rows()*ch.columns()==c);
      images.getRow(i).fill(&ch.at(0),0,c-1);
    }

    if (!b) {
      images.clear();
    }
    
    return b;
  };

  bool loadVectorImageList::apply(dmatrix& images) {
    int r,c;
    return apply(images,r,c);
  }

  // On place apply for type dmatrix!
  bool loadVectorImageList::apply(dmatrix& images, int& rows, int& columns) {

    bool b=true;
    
    const parameters& param=getParameters();

    channel ch;
    fvector tmp;
    int r=size();
    int c;
    
    if (hasNext()) {
      if (!loadImageList::apply(ch)) {
        images.clear();
        return false;
      }
      if (param.downsample) {
        down.apply(ch);
      }
      rows=ch.rows();
      columns=ch.columns();
      c=rows*columns;
      images.resize(r,c,0.f,false,false);
      ch.detach(tmp);
      images.getRow(0).castFrom(tmp);
    } else {
      setStatusString("Empty image list\n");
      images.clear();
      return false;
    }

    int i;
    for (i=1; i<r; i++) {
      b = b && loadImageList::apply(ch);
      if (param.downsample) {
        down.apply(ch);
      }
      // check if all images have the same size
      assert(ch.rows()*ch.columns()==c);
      ch.detach(tmp);
      images.getRow(i).castFrom(tmp);
    }

    if (!b) {
      images.clear();
    }
    
    return b;
  };

}
