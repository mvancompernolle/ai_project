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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiDataTransformer.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiDataTransformer.cpp,v 1.5 2008/10/02 15:35:36 alvarado Exp $
 */

#include "ltiDataTransformer.h"
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // dataTransformer::buffer
  // --------------------------------------------------

  bool dataTransformer::buffer::saveRaw(const std::string& name) const {
    FILE* f=fopen(name.c_str(),"w");
    if (f != 0) {
      int k=fwrite(&at(0),sizeof(value_type),size(),f);
      if (k != size()) {
        fclose(f);
        return false;
      }
      fclose(f);
      return true;
    } else {
      return false;
    }
  }


  bool dataTransformer::buffer::loadRaw(const std::string& name) {
    FILE* f=fopen(name.c_str(),"r");
    if (f != 0) {
      int k=fseek(f,0,SEEK_END);
      int len=ftell(f);
      resize(len);
      rewind(f);
      k=fread(&at(0),1,len,f);
      fclose(f);
      return (k == len);
    } else {
      return false;
    }
  }


  // --------------------------------------------------
  // dataTransformer
  // --------------------------------------------------

  const char* dataTransformer::notEnoughSpaceMsg="Not enough output buffer space.";

  // default constructor
  dataTransformer::dataTransformer()
    : functor(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

  }

  // default constructor
  dataTransformer::dataTransformer(const parameters& par)
    : functor() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  dataTransformer::dataTransformer(const dataTransformer& other)
    : functor() {
    copy(other);
  }

  // destructor
  dataTransformer::~dataTransformer() {
  }

  // returns the name of this type
  const char* dataTransformer::getTypeName() const {
    return "dataTransformer";
  }

  // copy member
  dataTransformer& dataTransformer::copy(const dataTransformer& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  dataTransformer& dataTransformer::operator=(const dataTransformer& other) {
    return (copy(other));
  }

  // return parameters
  const dataTransformer::parameters&
    dataTransformer::getParameters() const {
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



}
