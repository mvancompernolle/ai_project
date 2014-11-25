/*
 * Copyright (C) //@date
 * //@copyright
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
 * file .......: //@filename
 * authors ....: //@author
 * organization: LTI, RWTH Aachen
 * creation ...: //@date
 * //@rev
 */

//@TODO: include files
//@includes

namespace lti {
  // --------------------------------------------------
  // /*@classname*/::parameters
  // --------------------------------------------------

  // implementation on header file due to MS VC++ bug

  // --------------------------------------------------
  // //@classname
  // --------------------------------------------------

  // default constructor
  template<class T>
  /*@classname*/<T>::/*@classname*/()
    : /*@parentclass*/(){

    //@TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  template<class T>
  /*@classname*/<T>::/*@classname*/(const parameters& par)
    : /*@parentclass*/() {

    //@TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  template<class T>
  /*@classname*/<T>::/*@classname*/(const /*@classname*/<T>& other)
    : /*@parentclass*/() {
    copy(other);
  }

  // destructor
  template<class T>
  /*@classname*/<T>::~/*@classname*/() {
  }

  // returns the name of this type
  template<class T>
  const char* /*@classname*/<T>::getTypeName() const {
    return "/*@classname*/<T>";
  }

  // copy member
  template<class T>
  /*@classname*/<T>&
  /*@classname*/<T>::copy(const /*@classname*/<T>& other) {
      /*@parentclass*/::copy(other);

    //@TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  template<class T>
  /*@classname*/<T>&
  /*@classname*/<T>::operator=(const /*@classname*/<T>& other) {
    return (copy(other));
  }


  // clone member
  template<class T>
  functor* /*@classname*/<T>::clone() const {
    return new /*@classname*/<T>(*this);
  }

  // return parameters
  template<class T>
  const typename /*@classname*/<T>::parameters&
  /*@classname*/<T>::getParameters() const {
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

  // TODO: apply methods still need to be made template methods.

  //@apply_members


}

