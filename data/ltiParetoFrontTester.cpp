/*
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
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
 * file .......: ltiParetoFront.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.11.2003
 * revisions ..: $Id: ltiParetoFrontTester.cpp,v 1.2 2004/05/03 18:50:10 ltilib Exp $
 */

#include "ltiParetoFrontTester.h"
#undef _LTI_DEBUG
#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // --------------------------------------------------
  // dummyFunctor::parameters
  // --------------------------------------------------

  // default constructor
  dummyFunctor::parameters::parameters()
    : functor::parameters() {
    linear = float(1);
    quadratic = float(1);
  }

  // copy constructor
  dummyFunctor::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  dummyFunctor::parameters::~parameters() {
  }

  // get type name
  const char* dummyFunctor::parameters::getTypeName() const {
    return "dummyFunctor::parameters";
  }

  // copy member

  dummyFunctor::parameters& 
  dummyFunctor::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    
    linear = other.linear;
    quadratic = other.quadratic;
      
    return *this;
  }

  // alias for copy member
  dummyFunctor::parameters&
    dummyFunctor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* dummyFunctor::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool dummyFunctor::parameters::write(ioHandler& handler,
                                         const bool& complete) const
# else
  bool dummyFunctor::parameters::writeMS(ioHandler& handler,
                                           const bool& complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"linear",linear);
      lti::write(handler,"quadratic",quadratic);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool&) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool dummyFunctor::parameters::write(ioHandler& handler,
                                         const bool& complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if read was succeful
   */
# ifndef _LTI_MSC_6
  bool dummyFunctor::parameters::read(ioHandler& handler,
                                        const bool& complete)
# else
  bool dummyFunctor::parameters::readMS(ioHandler& handler,
                                          const bool& complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"linear",linear);
      lti::read(handler,"quadratic",quadratic);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool&) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool dummyFunctor::parameters::read(ioHandler& handler,
                                        const bool& complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // dummyFunctor
  // --------------------------------------------------

  // default constructor
  dummyFunctor::dummyFunctor()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  dummyFunctor::dummyFunctor(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  dummyFunctor::dummyFunctor(const dummyFunctor& other)
    : functor() {
    copy(other);
  }

  // destructor
  dummyFunctor::~dummyFunctor() {
  }

  // returns the name of this type
  const char* dummyFunctor::getTypeName() const {
    return "dummyFunctor";
  }

  // copy member
  dummyFunctor&
  dummyFunctor::copy(const dummyFunctor& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  dummyFunctor&
  dummyFunctor::operator=(const dummyFunctor& other) {
    return (copy(other));
  }


  // clone member
  functor* dummyFunctor::clone() const {
    return new dummyFunctor(*this);
  }

  // return parameters
  const dummyFunctor::parameters&
    dummyFunctor::getParameters() const {
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

  
  // On copy apply for type double!
  bool dummyFunctor::apply(const double& src,double& dest) const {

    // the "dummy" functor just evaluates a quadratic function:
    // rect (-x^2 + 4*x)

    const parameters& par = getParameters();


    double x = static_cast<double>(par.quadratic);
    double y = static_cast<double>(par.linear);

    // force an optimum by (2,2), which should be somehow found by the
    // PESA algorithm

    double f = (-x*x - y*y + 4*(x+y) - 4);

    dest = 0.5*(f + sqrt(f*f + 0.01)) -  (1/(1.0+y));

    _lti_debug2("Q: " << par.quadratic <<
                " L: " << par.linear <<
                " -> " << dest << std::endl);

    return true;
  };

  // --------------------------------------------------
  //
  // P A R E T O   T E S T E R
  //
  // --------------------------------------------------

  // --------------------------------------------------
  // paretoFrontTester
  // --------------------------------------------------

  // default constructor
  paretoFrontTester::paretoFrontTester()
    : paretoFront() {
  }

  // default constructor
  paretoFrontTester::paretoFrontTester(const parameters& par)
    : paretoFront(par) {
  }


  // copy constructor
  paretoFrontTester::paretoFrontTester(const paretoFrontTester& other)
    : paretoFront(other) {
  }

  // destructor
  paretoFrontTester::~paretoFrontTester() {
  }

  // returns the name of this type
  const char* paretoFrontTester::getTypeName() const {
    return "paretoFrontTester";
  }

  functor* paretoFrontTester::clone() const {
    return new paretoFrontTester(*this);
  }

  /*
   * Convert a binary-chain representation of a chromosome to a valid
   * parameter object.
   */
  bool paretoFrontTester::chromosomeToPhenotype(const chromosome& genotype,
                                        functor::parameters& phenotype) const {
    
    static const int bitsForLinear = 16;
    static const int bitsForQuadratic = 16;

    dummyFunctor::parameters* par = 
      dynamic_cast<dummyFunctor::parameters*>(&phenotype);

    if (isNull(par)) {
      return false;
    }

    int pos=0;
    double ires;
    double dres;
    pos = binToDouble(genotype,pos,bitsForLinear,0.0,15.0,ires);
    par->linear = static_cast<float>(ires);

    pos = binToDouble(genotype,pos,bitsForQuadratic,0.0,10.0,dres);
    par->quadratic = static_cast<float>(dres);

    return true;

    
  }

  /*
   * Return a fresh allocated parameters for the evaluated functor, which is
   * equivalent to the given genotype.
   */
  functor::parameters* 
  paretoFrontTester::chromosomeToPhenotype(const chromosome& genotype) const {

    dummyFunctor::parameters par;
    chromosomeToPhenotype(genotype,par);

    return par.clone();
  }

  /*
   * Convert a valid parameters object (phenotype) into binary-chain
   * representation of a chromosome.
   */
  bool paretoFrontTester::phenotypeToChromosome(
                                          const functor::parameters& phenotype,
                                          chromosome& genotype) const {
    static const int bitsForLinear = 16;
    static const int bitsForQuadratic = 16;

    const dummyFunctor::parameters* par = 
      dynamic_cast<const dummyFunctor::parameters*>(&phenotype);

    if (isNull(par)) {
      return false;
    }

    genotype.resize(bitsForLinear + bitsForQuadratic);
    int pos;
    pos = doubleToBin(par->linear,0,bitsForLinear,0.0,15.0,genotype);
    pos = doubleToBin(par->quadratic,pos,bitsForQuadratic,
                      0.0,10.0,genotype);

    return true;
  }
   
  /*
   * Return the length in bits for a chromosome.
   *
   * This method needs to be reimplemented, in order to get some 
   * default implementations to work.
   */
  int paretoFrontTester::getChromosomeSize() const {
    static const int bitsForLinear = 16;
    static const int bitsForQuadratic = 16;

    return bitsForLinear+bitsForQuadratic;
  }

  /*
   * Evaluate Chromosome
   *
   * This method is one of the most important ones for the pareto evaluation.
   * Its task is to produce a multidimensional fitness measure for a given
   * chromosome.
   *
   * It returns true if the evaluation was successful, of false if the
   * phenotype represents some invalid parameterization.  It is highly 
   * recomended that the mutation and crossover methods are reimplemented to 
   * avoid invalid parameterizations.
   *
   * There are mainly two types of fitness measures that can be
   * analyzed with this functor of this kind: empirical goodness and
   * empirical discrepancy (Zhang).  The empirical goodness computes some
   * measure using exclusively the test data, without requiring any ground
   * truth.  The empirical discrepancy assumes the existency of ground truth
   * and provides as measure some distance between the result of an algorithm
   * and the ground truth.  Each class derived from paretoFrontTester should
   * specify clearly which kind of fitness measures it provides.
   * 
   */
  bool paretoFrontTester::evaluateChromosome(const chromosome& individual,
                                             dvector& fitness) {
    
    // usually there will be a huge test set and some statistics for all
    // results will be used to build the multidimensional fitness measure.
    
    // Here we assume that dummyFunctor provides already a fitness measure
    // and that one of its parameters tell already how long it will take.
    dummyFunctor::parameters dfpar;
    chromosomeToPhenotype(individual,dfpar);
    dummyFunctor df(dfpar);

    fitness.resize(2,double(),false,false);
//     df.apply(0.0,fitness.at(0));
//     fitness.at(1)=1.0/(dfpar.linear+1); // fitness = 1/time cost

    fitness.at(0) = dfpar.linear;
    fitness.at(1) = sqrt(256.0-dfpar.linear*dfpar.linear)*dfpar.quadratic/10.0;

    _lti_debug3("Fitness: " << fitness.at(0) << "," << 
                fitness.at(1) << std::endl);

    return true;
  }


  
  /*
   * Generate a random individual.
   *
   * You usually will need to reimplement this method to ensure that
   * the generated random individuals have a valid phenotype, i.e. that
   * the chromosome binary representation in "genotype" has an equivalent
   * parameter object for the class you are using.
   *
   * Return true if successful, false otherwise.
   */
  bool paretoFrontTester::randomIndividual(chromosome& genotype) {
    return paretoFront::randomIndividual(genotype);
  }

  /*
   * Mutate the given chromosome.
   *
   * This should be reimplemented to ensure that the mutation is a 
   * valid phenotype.
   *
   * The default implementation flips the bits with the probability
   * given in the parameters.
   */
  bool paretoFrontTester::mutate(const chromosome& parent,
                                 chromosome& mutant) {
    return paretoFront::mutate(parent,mutant);
  }

  /*
   * Crossover between two chromosomes.
   * 
   * This should be reimplemented to ensure that the crossover produces a 
   * valid phenotype.
   *
   * The default implementation does a so called uniform crossover, in
   * which each pair of corresponding bits are exchanged with a 
   * probability of 0.5.
   */
  bool paretoFrontTester::crossover(const chromosome& parent1,
                                    const chromosome& parent2,
                                    chromosome& child) {
    return paretoFront::crossover(parent1,parent2,child);
  };
  


}

