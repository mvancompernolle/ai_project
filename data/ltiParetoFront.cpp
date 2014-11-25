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
 * revisions ..: $Id: ltiParetoFront.cpp,v 1.10 2006/03/10 20:21:57 alvarado Exp $
 */

#include "ltiParetoFront.h"

#include <limits>
#include <string>
#include <sstream>
#include <algorithm>

#ifdef _LTI_GNUC_2
namespace std {
  typedef ios ios_base;
}
#endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // paretoFront::parameters
  // --------------------------------------------------

  // default constructor
  paretoFront::parameters::parameters()
    : functor::parameters() {
    
    crossoverProbability = double(0.7);
    initialMutationRate = double(-1);
    finalMutationRate = double(-1);
    mutationDecayRate = double(33.3808200696);
    externalPopulationSize = int(100);
    internalPopulationSize = int(10);
    fitnessSpaceDimensionality = int(2);
    numOfIterations = int(1000);
    logAllEvaluations = false;
    fitnessSpacePartition = 32;
    sortResult = true;
    logFront = true;
    logFilename = "pareto.log";
  }

  // copy constructor
  paretoFront::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  paretoFront::parameters::~parameters() {
  }

  // get type name
  const char* paretoFront::parameters::getTypeName() const {
    return "paretoFront::parameters";
  }

  // copy member

  paretoFront::parameters&
    paretoFront::parameters::copy(const parameters& other) {
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

    
    crossoverProbability = other.crossoverProbability;
    initialMutationRate = other.initialMutationRate;
    finalMutationRate = other.finalMutationRate;
    mutationDecayRate = other.mutationDecayRate;
    externalPopulationSize = other.externalPopulationSize;
    internalPopulationSize = other.internalPopulationSize;
    fitnessSpaceDimensionality = other.fitnessSpaceDimensionality;
    numOfIterations = other.numOfIterations;
    logAllEvaluations = other.logAllEvaluations;
    fitnessSpacePartition = other.fitnessSpacePartition;
    sortResult = other.sortResult;

    logFront = other.logFront;
    logFilename = other.logFilename;

    return *this;
  }

  // alias for copy member
  paretoFront::parameters&
    paretoFront::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* paretoFront::parameters::clone() const {
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
  bool paretoFront::parameters::write(ioHandler& handler,
                                         const bool& complete) const
# else
  bool paretoFront::parameters::writeMS(ioHandler& handler,
                                           const bool& complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"crossoverProbability",crossoverProbability);
      lti::write(handler,"initialMutationRate",initialMutationRate);
      lti::write(handler,"finalMutationRate",finalMutationRate);
      lti::write(handler,"mutationDecayRate",mutationDecayRate);
      lti::write(handler,"externalPopulationSize",externalPopulationSize);
      lti::write(handler,"internalPopulationSize",internalPopulationSize);
      lti::write(handler,"fitnessSpaceDimensionality",
                 fitnessSpaceDimensionality);
      lti::write(handler,"numOfIterations",numOfIterations);
      lti::write(handler,"logAllEvaluations",logAllEvaluations);
      lti::write(handler,"fitnessSpacePartition",fitnessSpacePartition);
      lti::write(handler,"sortResult",sortResult);

      lti::write(handler,"logFront",logFront);
      lti::write(handler,"logFilename",logFilename);

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
  bool paretoFront::parameters::write(ioHandler& handler,
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
  bool paretoFront::parameters::read(ioHandler& handler,
                                        const bool& complete)
# else
  bool paretoFront::parameters::readMS(ioHandler& handler,
                                          const bool& complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"crossoverProbability",crossoverProbability);
      lti::read(handler,"initialMutationRate",initialMutationRate);
      lti::read(handler,"finalMutationRate",finalMutationRate);
      lti::read(handler,"mutationDecayRate",mutationDecayRate);
      lti::read(handler,"externalPopulationSize",externalPopulationSize);
      lti::read(handler,"internalPopulationSize",internalPopulationSize);
      lti::read(handler,"fitnessSpaceDimensionality",
                fitnessSpaceDimensionality);
      lti::read(handler,"numOfIterations",numOfIterations);
      lti::read(handler,"logAllEvaluations",logAllEvaluations);
      lti::read(handler,"fitnessSpacePartition",fitnessSpacePartition);
      lti::read(handler,"sortResult",sortResult);

      lti::read(handler,"logFront",logFront);
      lti::read(handler,"logFilename",logFilename);


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
  bool paretoFront::parameters::read(ioHandler& handler,
                                        const bool& complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // paretoFront static members
  // --------------------------------------------------
  const double* paretoFront::expLUT = 0;

  bool paretoFront::initExpLUT() {
    if (isNull(expLUT)) {
      // A static "singleton" LUT.  The compiler will remove it at the end.
      static const int lutSize = 3*1024;
      static double theLUT[lutSize];
      int i;
      for (i=0;i<lutSize;++i) {
        double fi = static_cast<double>(3.0*i)/lutSize;
        theLUT[i] = exp(-(fi*fi)/2.0);
      }
      expLUT = theLUT;
    }



    return true;
  }

  // --------------------------------------------------
  // paretoFront inline private members
  // --------------------------------------------------

  /**
   * An efficient way to compute g(x)=exp(-x^2/2)
   */
  inline double paretoFront::exp2(const double& x) const {
    // 3 is the everybody known factor: above 3*sigma the Gaussian
    // is negligible.
    const double ax = abs(x);
    return (ax < 3.0) ? expLUT[static_cast<int>(ax*1024)] : 0.0;
  }
  
  /**
   * Compute the fitness distance between the given two fitness points
   */
  inline double paretoFront::fitnessDistance(const lti::dvector& a,
                                             const lti::dvector& b) const {
    double res = 1.0;
    int i;
    const int size = min(a.size(),sigmas.size());
    for (i=0;(res>0.0) && (i<size);++i) {
      res *= exp2((a[i]-b[i])/sigmas[i]);
    }
    return res;
  }
  

  // --------------------------------------------------
  // paretoFront
  // --------------------------------------------------

  // default constructor
  paretoFront::paretoFront()
    : functor(),progressBox(0),logOut(0),logFront(false) {

    initExpLUT();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  paretoFront::paretoFront(const parameters& par)
    : functor(),progressBox(0),logOut(0),logFront(false) {

    initExpLUT();

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  paretoFront::paretoFront(const paretoFront& other)
    : functor(),progressBox(0),logOut(0),logFront(false) {
    copy(other);
  }

  // destructor
  paretoFront::~paretoFront() {
    delete progressBox;
    progressBox = 0;
    if (notNull(logOut)) {
      delete logOut;
      logOut = 0;
    }
  }

  // returns the name of this type
  const char* paretoFront::getTypeName() const {
    return "paretoFront";
  }

  // copy member
  paretoFront& paretoFront::copy(const paretoFront& other) {
    functor::copy(other);

    delete progressBox;
    progressBox = 0;
    if (other.validProgressObject()) {
      progressBox = other.getProgressObject().clone();
    }
    return (*this);
  }

  // alias for copy member
  paretoFront&
  paretoFront::operator=(const paretoFront& other) {
    return (copy(other));
  }

  // return parameters
  const paretoFront::parameters& paretoFront::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  paretoFront::parameters& paretoFront::getRWParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // privat class used to sort the vectors in scanning order
  struct  paretoFront::scanLess 
    : public std::binary_function<dvector,dvector,bool> {
    bool operator()(const paretoFront::individual& a, 
                    const paretoFront::individual& b) const {
      int i=a.fitness.lastIdx();
      for (;i>=0;--i) {
        if (a.fitness[i]<b.fitness[i]) {
          return true;
        } else if (b.fitness[i]<a.fitness[i]) {
          return false;
        }
      }
      // they are equal
      return false;
    }
  };


  // On place apply for type matrix!
  bool paretoFront::apply(matrix<double>& front) {
    const parameters& par = getParameters();

    // some attributes need initialization before calling PESA
    logEvaluations = par.logAllEvaluations;
    deadIndividuals.clear();
    initBoundingBox(bbox);

    if (!initLog()) {
      return false;
    }

    std::vector<individual> PE;  // The external population

    if (pesa(PE)) {

      // convert the PE vector into the standard output expected by the user
      front.resize(PE.size(),par.fitnessSpaceDimensionality);

      // sort the result if desired.
      if (par.sortResult) {
        std::sort(PE.begin(),PE.end(),scanLess());
      }

      _lti_debug("Pareto Front:\n");
      unsigned int i;
      for (i=0;i<PE.size();++i) {
        front.getRow(i).copy(PE[i].fitness);
        _lti_debug(i << ": " << PE[i].fitness << " -> squeeze " <<
                   PE[i].squeezeFactor << "\n");
      }
      _lti_debug("----------THE END-----------" << std::endl);
      return true;
    }
    return false;
  };

  // On copy apply for type matrix!
  bool paretoFront::apply(matrix<double>& front,
                          std::vector<functor::parameters*>& phenotypes) {

    const parameters& par = getParameters();

    trash(phenotypes); // ensure that we removed all thing maybe allocated
                       // before

    // some attributes need initialization before calling PESA
    logEvaluations = par.logAllEvaluations;
    deadIndividuals.clear();
    initBoundingBox(bbox);

    initLog();

    std::vector<individual> PE;

    if (pesa(PE)) {

      // convert the PE vector into the standard output expected by the user
      front.resize(PE.size(),par.fitnessSpaceDimensionality);
      phenotypes.resize(PE.size(),static_cast<functor::parameters*>(0));

      // sort the result if desired.
      if (par.sortResult) {
        std::sort(PE.begin(),PE.end(),scanLess());
      }

      unsigned int i;
      for (i=0;i<PE.size();++i) {
        front.getRow(i).copy(PE[i].fitness);
        phenotypes[i] = chromosomeToPhenotype(PE[i].genotype);
      }

      return true;
    }
    return false;
  }

  // On copy apply for type matrix!
  bool paretoFront::resume(matrix<double>& front,
                           std::vector<functor::parameters*>& phenotypes) {

    const parameters& par = getParameters();

    trash(phenotypes); // ensure that we removed all thing maybe allocated
                       // before

    // some attributes need initialization before calling PESA
    logEvaluations = par.logAllEvaluations;
    deadIndividuals.clear();
    initBoundingBox(bbox);

    std::vector<individual> PE;

    if (pesa(PE,true)) { // "true" means initFromLog

      // convert the PE vector into the standard output expected by the user
      front.resize(PE.size(),par.fitnessSpaceDimensionality);
      phenotypes.resize(PE.size(),static_cast<functor::parameters*>(0));

      // sort the result if desired.
      if (par.sortResult) {
        std::sort(PE.begin(),PE.end(),scanLess());
      }

      unsigned int i;
      for (i=0;i<PE.size();++i) {
        front.getRow(i).copy(PE[i].fitness);
        phenotypes[i] = chromosomeToPhenotype(PE[i].genotype);
      }

      return true;
    }
    return false;
  }

  // remove all elements in the given vector
  bool paretoFront::trash(std::vector<functor::parameters*> phenotypes) const {
    std::vector<functor::parameters*>::iterator it;
    for (it=phenotypes.begin();it!=phenotypes.end();++it) {
      delete (*it);
      (*it) = 0;
    }
    phenotypes.clear();

    return true;
  }

  // random initialization
  bool paretoFront::initInternalPopulation(std::vector<individual>& data) {
    const parameters& par = getParameters();
    data.resize(par.internalPopulationSize);

    unsigned int i,abort;
    // if no valid individual can be generated after abortThreshold tries, 
    // there is something wrong
    static const unsigned int abortThreshold = 1000000;

    i=0;
    abort=0;
    while ((abort<abortThreshold) && (i<data.size())) {
      // for each individual 

      // for each bit 
      if (randomIndividual(data[i].genotype)) {
        // only accept valid chromosomes in the initial population
        i++;
        abort = 0;
      }
      else {
        abort++;
      }
    }
    
    if (abort >= abortThreshold) {
      setStatusString("Too many errors generating an individual.  Aborting.");
      return false;
    }

    return true;
  }

  bool paretoFront::randomIndividual(chromosome& genotype) {
    genotype.resize(getChromosomeSize());
    unsigned int j;
    for (j=0;j<genotype.size();++j) {
      genotype[j] = (random() >= 0.5);
    }
    return true;
  }



  // mutate
  bool paretoFront::mutate(const chromosome& parent,
                           chromosome& mutant) {
    
    if (mutant.size() != parent.size()) {
      mutant.resize(parent.size());
    }

    unsigned int i;
    for (i=0;i<parent.size();++i) {
      mutant[i] = (random() < mutationRate) ? !parent[i] : parent[i];
    }
    
    return true;
  }

  // crossover
  bool paretoFront::crossover(const chromosome& parent1,
                              const chromosome& parent2,
                              chromosome& child) {

    assert(parent1.size() == parent2.size());

    if (child.size() != parent1.size()) {
      child.resize(parent1.size());
    }

    // uniform crossover implementation with mutation
    unsigned int i;
    bool gen;
    for (i=0;i<child.size();++i) {
      gen = (random() < 0.5) ? parent1[i] : parent2[i];  // crossover
      child[i] = (random() < mutationRate) ? !gen : gen;      // mutation
    }

    return true;
  }

  bool paretoFront::dominate(const dvector& a,
                             const dvector& b) const {
    bool theOne = false;

    // a little pointer arithmetic to accelerate this frequently called
    // function
    dvector::const_iterator aPtr = a.begin();
    dvector::const_iterator bPtr = b.begin();
    const dvector::const_iterator ePtr = a.end();
    
    while (aPtr != ePtr) {
      if ((*aPtr) < (*bPtr)) {
        // if any element is smaller => definitively not greater!
        return false;
      } else if ((*aPtr) > (*bPtr)) {
        // only greater if at least one element has been strictly greater
        theOne = true;
      }
      
      ++aPtr;
      ++bPtr;
    }

    return theOne;
  }

  // The PESA Algorithm
  bool paretoFront::pesa(std::vector<individual>& PE,const bool initFromLog) {
    
    const parameters& par = getParameters();
    // set the shadow for the mutation rate
    const double initialMutationRate = (par.initialMutationRate < 0.0) ? 
      abs(par.initialMutationRate)/getChromosomeSize() : 
      par.initialMutationRate;

    const double finalMutationRate = (par.finalMutationRate < 0.0) ? 
      abs(par.finalMutationRate)/getChromosomeSize() : 
      par.finalMutationRate;

    // initial value for mutation rate
    mutationRate = initialMutationRate;

    std::vector<individual> PI; // internal population
    PE.clear();

    // if the user desires to watch the evolution progress
    if (validProgressObject()) {
      getProgressObject().reset();
      std::string str("Pareto Front Evaluation Test.\n");
      str += "Evaluation class: ";
      str += getTypeName();
      getProgressObject().setTitle(str);
      getProgressObject().setMaxSteps(par.numOfIterations+2);
    }

    // ensure that the PE and PI vectors will have all memory they need
    PE.reserve(par.internalPopulationSize+par.externalPopulationSize+1);

    unsigned int i;

    if (initFromLog) {
      // read the whole log and use it as initialization
      // Some output if desired
      if (validProgressObject()) {
        getProgressObject().step("Initialization from log file.");
      }
      if (getDataFromLog(par.logFilename,getRWParameters(),PI,bbox)) {
        // we need to re-adapt the parameters from the log file
        if (validProgressObject()) {
          getProgressObject().setMaxSteps(par.numOfIterations+2);
        }

        // well, we need to continue logging at the end of the file
        if (notNull(logOut)) {
          logOut->close();
          delete logOut;
          logOut=0;
        }
        if (par.logFront) {
          // append at the end of the file!
          logOut = new std::ofstream(par.logFilename.c_str(),
                                     std::ios_base::app);
          olsh.use(*logOut);
          logFront = false; // avoid rewriting the initialization 
        }        
      } else {
        if (validProgressObject()) {
          getProgressObject().step("Problems reading log file.  Aborting");
        }
        return false;
      }

      // If there are not enough individuals in the internal population
      // create a few more.
      if (static_cast<int>(PI.size()) < par.internalPopulationSize) {
        std::vector<individual> tmpPI;
        tmpPI.reserve(par.internalPopulationSize);
        
        // Initialization of internal population: create random individuals
        if (!initInternalPopulation(tmpPI)) {
          
          // Some output if desired
          if (validProgressObject()) {
            getProgressObject().step("Initialization failed.");
          }
          return false;
        }

        // copy all new generated elements
        int i;
        for (i=PI.size();i<par.internalPopulationSize;++i) {
          PI.push_back(tmpPI[i]);
        }
        
      }
    }
    else { 
      // normal initialization 
      
      PI.reserve(par.internalPopulationSize);

      // Some output if desired
      if (validProgressObject()) {
        getProgressObject().step("Initialization.");
      }
      
      // Initialization of internal population.
      if (!initInternalPopulation(PI)) {
        
        // Some output if desired
        if (validProgressObject()) {
          getProgressObject().step("Intialization failed.");
        }
        return false;
      }
    }

    // -----------------------------------------------------------------
    //                            LET'S EVOLVE!
    // -----------------------------------------------------------------

    int extPop = 0; // number of individuals in the external population
    int iter = 0;
    int inserted;
    bool updateSqueezeFactors;
    bool initFirstFromLog = initFromLog;
    unsigned int premortum;
    do {

      // Evaluate Internal Population (PI)
      updateSqueezeFactors = false;
      premortum = 0;
      for (i=0;i<PI.size();++i) { // for each individual in the internal pop.
                                  // (this should be done in parallel)!

        if (validProgressObject(1)) {
          std::ostringstream oss;
          oss << "Internal evaluation " << i+1 << "/" << PI.size();
          getProgressObject().substep(1,oss.str());
        }

        if (initFirstFromLog) {
          // initialization from the log file
          updateSqueezeFactors = true;
        }
        else {
          // normal algorithm
          if(evaluateChromosome(PI[i].genotype,PI[i].fitness)) {
            updateSqueezeFactors = (updateBoundingBox(PI[i].fitness,bbox) ||
                                    updateSqueezeFactors);
          } else {
            // evaluation failed, but we need some dummy fitness:
            // let's make the worst fitness for this one: zero everywhere
            PI[i].fitness.resize(par.fitnessSpaceDimensionality,0,false,true);
            premortum++;
          }

          if (validProgressObject(2)) {
            // if the user wants, show the fitness vector
            std::ostringstream oss;
            oss << "Fitness: " << PI[i].fitness;
            getProgressObject().substep(2,oss.str());
          }
        }
      }
      
      if (premortum >= PI.size()) {
        appendStatusString("\nAll evaluations in one iteration failed.");

        if (validProgressObject()) {
          getProgressObject().step("Error: All evalutations in one iteration" \
                                   " failed. Aborting.");
        }

        return false;
      }
      
      // If the bounding box changed, we need to recompute the density factors
      if (updateSqueezeFactors) {
        _lti_debug2("Updating Squeeze Factors"<<std::endl);

        updateFitnessSpaceSubdivision();
        updateDensityFactors(PE);

        _lti_debug3("Bounding box:\n" << bbox << std::endl);
        _lti_debug3("New sigmas:\n" << sigmas << std::endl);
      }

      // Add non-dominated members from PI to PE
      inserted = insert(PI,PE);
      extPop = PE.size();

      // Some output if desired
      if (validProgressObject()) {
        std::ostringstream oss;
    
        oss << "Front size: " << extPop
            << " \tNew individuals: " << inserted;
        getProgressObject().step(oss.str());
      }

      // Log which iteration has been currently logged
      if (par.logFront) {
        std::ostringstream oss;
        oss << ";; Iteration: " << iter << "  Front size: " << extPop
            << "  New individuals: " << inserted;
        oss << " (MR: " << mutationRate*getChromosomeSize() << " bits)";
        (*logOut) << oss.str() << std::endl;
      }

      // end of analysis?
      if ((++iter >= par.numOfIterations) ||
          (validProgressObject() && getProgressObject().breakRequested())) {

        // Some output if desired
        if (validProgressObject()) {
          if (iter >= par.numOfIterations) {
            getProgressObject().step("Ready.");
          } else {
            getProgressObject().step("Stopped by the user.");
          }
        }
        break;
      }

      // the next iteration should happen normally
      if (initFirstFromLog) {
        // we can have the wrong size for PI here, so let's fix it if necessary
        if (static_cast<int>(PI.size()) != par.internalPopulationSize) {
          PI.resize(par.internalPopulationSize);
        }
        initFirstFromLog = false;
        logFront = par.logFront; // log if desired
      }

      // the evaluation of algorithm could set the random number generator in
      // a deterministic state.
      // we hope that reinitializing it with the system time is random enough!
      crdist.init();

      // --------------------
      // Generate new PI set:
      // --------------------
      int j;
      j=0;
      while (j<par.internalPopulationSize) {
        const int a = binaryTournament(PE);
        if ((extPop >= 2) && (random() < par.crossoverProbability)) {
          // crossover
          int b = binaryTournament(PE);
          while (a == b) { // ups! two identical individuals, get another one.
            b = min(static_cast<int>(random()*extPop),extPop-1);
          }
          _lti_debug3("Crossover squeeze factors: " <<
                      PE[a].squeezeFactor << "," <<
                      PE[b].squeezeFactor << std::endl);
          crossover(PE[a].genotype,PE[b].genotype,PI[j].genotype);
        } else {
          _lti_debug3("Mutation squeeze factor: " <<
                      PE[a].squeezeFactor << std::endl);
          // mutation
          mutate(PE[a].genotype,PI[j].genotype);
        }
        ++j;
      }

      // update the mutation rate for the next time
      mutationRate = ((initialMutationRate-finalMutationRate)*
                      exp(-iter/par.mutationDecayRate)) + finalMutationRate;

    } while (true);

    if (logFront) {
      if (notNull(logOut)) {
        logOut->close();
        delete logOut;
        logOut = 0;
      }
    }

    return true;
  }

  // binary tournament
  int paretoFront::binaryTournament(const std::vector<individual>& PE) const {
    const int size = PE.size();

    if (size <= 1) {
      return 0;
    } else if (size <= 2) {
      if (PE[0].squeezeFactor < PE[1].squeezeFactor) {
        return 0;
      } else if (PE[0].squeezeFactor > PE[1].squeezeFactor) {
        return 1;
      } else {
        return (random() < 0.5) ? 0 : 1;
      }
    }

    // chose two random individuals
    int a = min(static_cast<int>(size*random()),size-1);
    int b = min(static_cast<int>(size*random()),size-1);
    while (b == a) {
      b = min(static_cast<int>(size*random()),size-1);
    }

    if (PE[a].squeezeFactor < PE[b].squeezeFactor) {
      return a;
    } else if (PE[a].squeezeFactor > PE[b].squeezeFactor) {
      return b;
    } else {
      return (random() < 0.5) ? a : b;
    }
  }

  // insert one individual into the external population
  bool paretoFront::insert(individual& genotype,
                           std::vector<individual>& PE) {

    std::list<int> removal;
    int freePlaces = 0;
    unsigned int j,i;
    double dist;
    genotype.squeezeFactor = 0.0;

    // Check which individuals in PE are dominated by the genotype,
    // and mark them for removal
    for (j=0;j<PE.size();++j) {
      if (dominate(genotype.fitness,PE[j].fitness)) {
        // the j element in PE needs to be removed.
        removal.push_back(j);
        if (logEvaluations) {
          deadIndividuals.push_back(PE[j]);
        }

        if (validProgressObject(3)) {
          // if the user wants, show the recently dead individuals
          std::ostringstream oss;
          oss << "RDIn: " << PE[j].fitness;
          getProgressObject().substep(3,oss.str());
        }

        // for each removed item (j) we need to update the density
        // influences it caused on the other Pareto members:
        for (i=0;i<PE.size();++i) {
          PE[i].squeezeFactor -= fitnessDistance(PE[i].fitness,PE[j].fitness);
        }

        ++freePlaces;
      } else {
        // element j survives.  We can already compute its influence on the
        // new element 
        dist = fitnessDistance(PE[j].fitness,genotype.fitness);
        genotype.squeezeFactor += dist;
        PE[j].squeezeFactor += dist;
      }
    }


    // Recycle as much removal places as possible, we can assume that
    // the elements in "removal" are sorted.
    if (freePlaces > 0) {
      std::list<int>::iterator it = removal.begin();
      PE[*it] = genotype;
      freePlaces--;
      ++it;
      
      // all incomers at place, now we need to really remove the rest
      // elements.
      int lastElem = PE.size()-1;
      std::list<int>::reverse_iterator rit;
      rit=removal.rbegin();

      int newVctSize = PE.size()-freePlaces;
      
      // shift all unused places to the end of the vector
      while (freePlaces > 0) {
        // get last used value in PE
        if ((*rit) < lastElem) {
          // swap the values (well, just half swap, since one element will be
          //                  removed anyway)
          PE[*it]=PE[lastElem];
          lastElem--;
          it++;
        } else {
          // element already at a position that will be removed
          lastElem--;
          rit++;          
        }
        freePlaces--;
      }

      // resize the vector, removing the last elements (std::vector keeps the
      // rest).
      PE.resize(newVctSize);
      
    } else {
      // no place left on PE, we just append it
      PE.push_back(genotype);
    }

    return true;
  }

  bool paretoFront::logEntry(const individual& ind,const bool markDead) {
    if (logFront) {
      std::string str;
      // save new incomer in the log if so desired
      olsh.writeBegin();
      ind.fitness.write(olsh);
      olsh.writeDataSeparator();
      chromosomeToString(ind.genotype,str);
      olsh.write(str);
      olsh.writeEnd();
      if (markDead) {
        (*logOut) << " ;; x" ;
      }
      (*logOut) << std::endl; // force EOL

      logOut->flush();                   // force to write the file
      return true;
    }
    return false;
  }

  int paretoFront::insert(std::vector<individual>& PI,
                          std::vector<individual>& PE) {

    const unsigned int extPopSize =
      static_cast<unsigned int>(getParameters().externalPopulationSize);

    // Check which elements of PI are, within PI, non-dominated.    
    std::vector<bool> nonDominated(PI.size(),true);
    bool dominated;
    unsigned int i,j;
    int toInsert = PI.size();
    for (i=0;i<PI.size();++i) {
      dominated = false;
      for (j=0;!dominated && (j<PI.size());++j) {
        dominated = dominate(PI[j].fitness,PI[i].fitness);        
      }
      nonDominated[i]=!dominated;
      if (dominated) {
        --toInsert;
        if (logEvaluations) {
          logEntry(PI[i],true);
          deadIndividuals.push_back(PI[i]);
        }
      }
    }

    // Only the non-Dominated points in PI need to be checked in PE
    // Check if any element of PI is non-dominated from the elements in PE
    for (i=0;i<PI.size();++i) {
      if (nonDominated[i]) {
        dominated = false;
        
        for (j=0;!dominated && (j<PE.size());++j) {
          dominated = (dominate(PE[j].fitness,PI[i].fitness));          
        }
        nonDominated[i]=!dominated;
        if (dominated) {
          --toInsert;
          if (logEvaluations) {
            logEntry(PI[i],true);
            deadIndividuals.push_back(PI[i]);
          }
        }
      }
    }
    
    if (toInsert == 0) {
      // nothing to be done:
      // all new individuals were dominated by someone else
      return toInsert;
    }

    // the points at PI with (nonDominated == true) belong definitively in PE
    for (i=0;i<PI.size();++i) {
      if (nonDominated[i]) {
        // save new incomer in the log if so desired
        logEntry(PI[i]);
        insert(PI[i],PE);

        if (validProgressObject(3)) {
          // if the user wants, show the new individuals
          std::ostringstream oss;
          oss << "NNDC: " << PI[i].fitness;
          getProgressObject().substep(3,oss.str());
        }
      }
    }

    // Now we need to check if the number of elements in the pareto front
    // is too high, and in that case we need to remove the elements with
    // the highest squeeze factors.
    if (PE.size() > extPopSize) {
      // it is indeed too big, check how many elements we need to remove

      // partially sort the elements
      std::nth_element(PE.begin(),PE.begin()+extPopSize,PE.end());
      
      // The elements to be removed are now at the end.
      // update the distances for the elements that are going to be removed.

      // for each removed item (j) we need to update the density
      // influences it caused on the other Pareto members:
      for (j=extPopSize;j<PE.size();++j) {
        _lti_debug3("Removing element with squeeze factor " << 
                    PE[j].squeezeFactor << std::endl);

        if (validProgressObject(3)) {
          // if the user wants, show the recently dead individuals
          std::ostringstream oss;
          oss << "HDRI: " << PE[j].fitness;
          getProgressObject().substep(3,oss.str());
        }
        
        for (i=0;i<extPopSize;++i) {
          PE[i].squeezeFactor -= fitnessDistance(PE[i].fitness,PE[j].fitness);
        }
      }
      
      // remove the elements
      PE.resize(extPopSize);
      
    }

    return toInsert;
  }

  void paretoFront::initBoundingBox(dmatrix& bbox) const {
    const parameters& par = getParameters();
    bbox.resize(2,par.fitnessSpaceDimensionality);
    // the min initialized with max
    bbox.getRow(0).fill(std::numeric_limits<double>::max());
    // the max initialized with min
    bbox.getRow(1).fill(-std::numeric_limits<double>::max());
  }
  
  /*
   * Update bounding box considering the given fitness space point
   */
  bool paretoFront::updateBoundingBox(const dvector& pnt,
                                            dmatrix& bbox) const {
    int i;
    bool changed = false;
    const int maxDim = min(bbox.columns(),pnt.size());
    for (i=0;i<maxDim;++i) {
      if (pnt.at(i) < bbox.at(0,i)) {
        bbox.at(0,i)=pnt.at(i);
        changed = true;
      } 
      
      if (pnt.at(i) > bbox.at(1,i)) {
        bbox.at(1,i)=pnt.at(i);
        changed = true;
      } 
    }

    return changed;
  }

  void paretoFront::updateFitnessSpaceSubdivision() {
    const parameters& par = getParameters();
    // bbox is the bounding box with 2xfitSpcDim 
    sigmas.resize(bbox.columns(),0.0,false,false); // sigmas with fitness
                                                   // space dimensionality
    int i;
    for (i=0;i<sigmas.size();++i) {
      sigmas.at(i)=(bbox.at(1,i)-bbox.at(0,i))/(par.fitnessSpacePartition*6.0);
    }
  }

  void paretoFront::updateDensityFactors(std::vector<individual>& PE) {
    std::vector<individual>::iterator it,jt;

    // clear the squeeze factors
    for (it=PE.begin();it!=PE.end();++it) {
      (*it).squeezeFactor = 0.0;
    }
    
    double d;

    for (it=PE.begin();it!=PE.end();++it) {
      for (jt=it;jt!=PE.end();++jt) {
        if (it != jt) {
          d = fitnessDistance((*it).fitness,(*jt).fitness);
          (*it).squeezeFactor += d;
          (*jt).squeezeFactor += d;
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  /*
   * Return the gray code of the given number
   */
  inline uint32 paretoFront::grayCode(const uint32 i) const {
    return i ^ (i >> 1);  // easy, isn't it?
  }
  
  /*
   * Return the integer value corresponding to the given gray code
   */
  inline uint32 paretoFront::iGrayCode(const uint32 g) const {
    uint32 a(g),idiv;
    int ish=1; // shift
    while(true) {
      a ^= (idiv = (a >> ish));
      if ((idiv <= 1) || (ish == 32)) return a;
      ish <<= 1; // shift x2
    }
  }
  
  int paretoFront::binToInt(const chromosome& chain,
                            const int startBit,
                            const int bitLength,
                            int32& result) const {

    uint32 r = 0;
    int next = binToUInt(chain,startBit,bitLength,r);

    if (next > 0) {
      if ((bitLength < 32) && 
          (r >= static_cast<uint32>(1<<(bitLength-1)))) {
        result = static_cast<int>(1<<bitLength) - static_cast<int>(r);
      } else {
        result = static_cast<int>(r);
      }
    }

    return next;
  }


  int paretoFront::binToInt(const chromosome& chain,
                            const int startBit,
                            const int bitLength,
                            const int lowLimit,
                            const int highLimit,
                            int32& result) const {

    uint32 r = 0;
    // value stored with intToBin(...) shifted the value range from lowLimit
    // to 0, so that r is now from zero to (highLimit-lowLimit)
    int next = binToUInt(chain,startBit,bitLength,r);

    int l,h;
    minmax(lowLimit,highLimit,l,h);

    if (next > 0) {
      const int mod = h-l+1;
      result = static_cast<int>(r%mod) + l;
    }

    return next;
  }

  int paretoFront::binToUInt(const chromosome& chain,
                             const int startBit,
                             const int bitLength,
                             uint32& result) const {

    // it will be assumed, that the input bit string encodes the number in
    // Gray code.

    if (static_cast<unsigned int>(startBit+bitLength) > chain.size()) {
      return -1;
    }
    
    assert(bitLength <= 32);

    int i;
    result = 0;
    for (i=0;i<bitLength;++i) {
      result = result << 1;
      result = result | ((chain[i+startBit] ? 1 : 0));
    }

    result = iGrayCode(result); // decode the result

    return bitLength+startBit;
  }

  int paretoFront::binToDouble(const chromosome& chain,
                               const int startBit,
                               const int bitLength,
                               const double& lowLimit,
                               const double& highLimit,
                               double& result) const {

    double l,h;
    minmax(lowLimit,highLimit,l,h);
    uint32 val;
    int r;
    r=binToUInt(chain,startBit,bitLength,val);
    if (r>0) {
      const uint32 maxis = 0xFFFFFFFF >> (32-bitLength);
      result = val*(h-l)/maxis + l;
      return r;
    }
    return r;
  }

  int paretoFront::uintToBin(const uint32 value,
                             const int startBit,
                             const int bitLength,
                             chromosome& chain) const {
    if (static_cast<unsigned int>(startBit+bitLength) > chain.size()) {
      return -2; // report size exceeded
    }
    
    uint32 v = grayCode(value); // encode with Gray codes
    int i = startBit+bitLength-1;
    while (i>=startBit) {
      chain[i]=((v & 0x01) != 0);
      v = v >> 1;
      --i;
    }

    return startBit+bitLength;
  }

  int paretoFront::intToBin(const int value,
                            const int startBit,
                            const int bitLength,
                            chromosome& chain) const {

    uint32 v = static_cast<uint32>(value);
    return uintToBin(v,startBit,bitLength,chain);
  }

  int paretoFront::intToBin(const int value,
                            const int startBit,
                            const int bitLength,
                            const int lowLimit,
                            const int highLimit,
                            chromosome& chain) const {

    int l,h;
    minmax(lowLimit,highLimit,l,h);
    // v is the input value shifted to map lowLimit to zero.  The value
    // is also cut to be between lowLimit and highLimit
    uint32 v = static_cast<uint32>(max(l,min(h,value))-l);
    return uintToBin(v,startBit,bitLength,chain);
  }

  int paretoFront::doubleToBin(const double& value,
                               const int startBit,
                               const int bitLength,
                               const double& lowLimit,
                               const double& highLimit,
                               chromosome& chain) const {

    double l,h;
    minmax(lowLimit,highLimit,l,h);

    if ((value < l) || (value > h)) {
      return -1;
    }

    double normed = (value-l)/(h-l);
    const uint32 maxis = 0xFFFFFFFF >> (32-bitLength);    
    
    uint32 ival = iround(maxis*normed);
    return uintToBin(ival,startBit,bitLength,chain);
  }

  // set the progress object
  void paretoFront::setProgressObject(const progressInfo& progBox) {
    removeProgressObject();
    progressBox = progBox.clone();
  }

  // remove the active progress object
  void paretoFront::removeProgressObject() {
    delete progressBox;
    progressBox = 0;
  }

  // valid progress object
  bool paretoFront::validProgressObject() const {
    return notNull(progressBox);
  }

  // valid progress object
  bool paretoFront::validProgressObject(const int detailLevel) const {
    return (notNull(progressBox) && 
            (progressBox->getDetailLevel() >= detailLevel));
  }

  progressInfo& paretoFront::getProgressObject() {
    return *progressBox;
  }

  const progressInfo& paretoFront::getProgressObject() const {
    return *progressBox;
  }

  void paretoFront::getAnalyzedBox(matrix<double>& bb) const {
    bb.copy(bbox);
  }

  void paretoFront::getDominatedIndividuals(matrix<double>& dindiv) const {

    dindiv.resize(deadIndividuals.size(),
                  getParameters().fitnessSpaceDimensionality);

    std::list<individual>::const_iterator it;
    int i;
    for (i=0,it=deadIndividuals.begin();it!=deadIndividuals.end();++it,++i) {
      dindiv.getRow(i).copy((*it).fitness);
    }
  }

  void paretoFront::chromosomeToString(const chromosome& genotype,
                                       std::string& str) const {
    str.resize(genotype.size(),'0');
    unsigned int i;
    for (i=0;i<genotype.size();++i) {
      if (genotype[i]) {
        str[i]='1';
      }
    }
  }

  void paretoFront::stringToChromosome(const std::string& str,
                                       chromosome& genotype) const {
    genotype.resize(str.length());
    unsigned int i;
    for (i=0;i<genotype.size();++i) {
      genotype[i] = (str[i]!='0');
    }
  }

  // ---------------------------------------------------------------------
  // Log related stuff
  // ---------------------------------------------------------------------

  bool paretoFront::initLog() {
    if (notNull(logOut)) {
      delete logOut;
      logOut=0;
    }

    const parameters& par = getParameters();
    logFront = par.logFront; // update shadow attribute of parameter

    if (logFront) {

      logOut = new std::ofstream(par.logFilename.c_str());
      if (isNull(logOut)) {
        setStatusString("Error opening log file.  Check your parameters.");
        return false;
      }
      
      olsh.use(*logOut);
      
      olsh.writeComment(std::string("Protocol for ") + getTypeName());
      par.write(olsh);
      olsh.writeComment("Data");
    }

    return true;
  }

  bool paretoFront::getDataFromLog(const std::string& logFile,
                                   matrix<double>& front,
                         std::vector<functor::parameters*>& phenotypes) {
  
    dmatrix dummyBbox;
    initBoundingBox(dummyBbox);
    trash(phenotypes);

    std::vector<individual> PE;
    if (getDataFromLog(logFile,getRWParameters(),PE,dummyBbox)) {
      const parameters& par = getParameters();

      // convert the PE vector into the standard output expected by the user
      front.resize(PE.size(),par.fitnessSpaceDimensionality);
      phenotypes.resize(PE.size(),static_cast<functor::parameters*>(0));

      // sort the result if desired.
      if (par.sortResult) {
        std::sort(PE.begin(),PE.end(),scanLess());
      }

      unsigned int i;
      for (i=0;i<PE.size();++i) {
        front.getRow(i).copy(PE[i].fitness);
        phenotypes[i] = chromosomeToPhenotype(PE[i].genotype);
      }

      return true;
      
    }

    return false;

  }

  bool paretoFront::getDataFromLog(const std::string& logFile,
                                   parameters& params,
                                   std::vector<individual>& data,
                                   dmatrix& boundingBox) const {
    std::ifstream in(logFile.c_str());
    if (in) {
      lispStreamHandler lsh(in);
      if (params.read(lsh)) {
        data.clear();
        initBoundingBox(boundingBox);
        std::string str;
        bool ok;
        // read data one by one
        while (lsh.tryBegin()) {
          data.push_back(individual());
          individual& indiv = data[data.size()-1];
          ok = indiv.fitness.read(lsh);
          updateBoundingBox(indiv.fitness,boundingBox);
          ok = lsh.readDataSeparator() && ok;
          ok = lsh.read(str) && ok;
          stringToChromosome(str,indiv.genotype);
          ok = lsh.readEnd() && ok;
          if (!ok || 
              (static_cast<int>(indiv.fitness.size()) !=
               params.fitnessSpaceDimensionality) ||
              (static_cast<int>(indiv.genotype.size()) !=
               getChromosomeSize())) {
            // wrong element.  Delete it
            data.pop_back();
          }
        }

        return true;
      }
    }
    return false;
  }

}
