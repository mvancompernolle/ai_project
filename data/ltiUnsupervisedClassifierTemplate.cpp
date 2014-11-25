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

  // default constructor
  /*@classname*/::parameters::parameters()
    : /*@parentclass*/::parameters() {
    //@TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    /*@parametersConstructor*/
  }

  // copy constructor
  /*@classname*/::parameters::parameters(const parameters& other)
    : /*@parentclass*/::parameters() {
    copy(other);
  }

  // destructor
  /*@classname*/::parameters::~parameters() {
  }

  // get type name
  const char* /*@classname*/::parameters::getTypeName() const {
    return "/*@classname*/::parameters";
  }

  // copy member

  /*@classname*/::parameters&
    /*@classname*/::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    /*@parentclass*/::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    /*@parentclass*/::parameters& (/*@parentclass*/::parameters::* p_copy)
      (const /*@parentclass*/::parameters&) =
      /*@parentclass*/::parameters::copy;
    (this->*p_copy)(other);
# endif

    //@parameters

    return *this;
  }

  // alias for copy member
  /*@classname*/::parameters&
    /*@classname*/::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* /*@classname*/::parameters::clone() const {
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
  bool /*@classname*/::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool /*@classname*/::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      //@writeparams
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && /*@parentclass*/::parameters::write(handler,false);
# else
    bool (/*@parentclass*/::parameters::* p_writeMS)(ioHandler&,const bool) const =
      /*@parentclass*/::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool /*@classname*/::parameters::write(ioHandler& handler,
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
  bool /*@classname*/::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool /*@classname*/::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      //@readparams
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && /*@parentclass*/::parameters::read(handler,false);
# else
    bool (/*@parentclass*/::parameters::* p_readMS)(ioHandler&,const bool) =
      /*@parentclass*/::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool /*@classname*/::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // //@classname
  // --------------------------------------------------

  // default constructor
  /*@classname*/::/*@classname*/()
    : /*@parentclass*/(){

    //@TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  /*@classname*/::/*@classname*/(const /*@classname*/& other)
    : /*@parentclass*/() {
    copy(other);
  }

  // destructor
  /*@classname*/::~/*@classname*/() {
  }

  // returns the name of this type
  const char* /*@classname*/::getTypeName() const {
    return "/*@classname*/";
  }

  // copy member
  /*@classname*/&
    /*@classname*/::copy(const /*@classname*/& other) {
      /*@parentclass*/::copy(other);

    //@TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  /*@classname*/&
    /*@classname*/::operator=(const /*@classname*/& other) {
    return (copy(other));
  }


  // clone member
  classifier* /*@classname*/::clone() const {
    return new /*@classname*/(*this);
  }

  // return parameters
  const /*@classname*/::parameters&
    /*@classname*/::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // Calls the same method of the superclass.
  bool /*@classname*/::train(const dmatrix& input, ivector& ids) {

    return /*@parentclass*/::train(input, ids);
  }

  //@TODO: comment your train method
  //Should you use a different method for the two argument train above
  //Change it also comment well.
  bool /*@classname*/::train(const dmatrix& data) {

    // tracks the status of the training process.
    // if an error occurs set to false and use setStatusString()
    // however, training should continue, fixing the error as well as possible
    bool b=true;

    //@TODO the following code is usually appropriate for setting the
    //the outputTemplate member. If not delete it. Set nbOutputs according to
    //your algorithm or replace by your variable name.

    int nbOutputs =

    //Put the id information into the result object
    //Each output value has the id of its position in the matrix
    ivector tids(nbOutputs);
    int i;
    for (i=0; i<nbOutputs; i++) {
      tids.at(i)=i;
    }
    outTemplate=outputTemplate(tids);

    return b;
  }


  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //@TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!

  bool
  /*@classname*/::classify(const dvector& feature,
                           outputVector& result) const {

    // tResult is a vector that results from your classification process.
    // rename if desired.
    dvector tResult();

    return outTemplate.apply(tResult, result);
  }

}

