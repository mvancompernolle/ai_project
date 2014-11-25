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
 * file .......: ltiHmmTrellisDiagram.cpp
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 16.5.2002
 * revisions ..: $Id: ltiHmmTrellisDiagram.cpp,v 1.9 2006/09/05 10:16:12 ltilib Exp $
 */

// include files
#include "ltiObject.h"
#include "ltiHmmTrellisDiagram.h"
#include <sstream>

namespace lti {
  // --------------------------------------------------
  // hmmTrellisDiagram::parameters
  // --------------------------------------------------

  // default constructor
  hmmTrellisDiagram::parameters::parameters()
    : functor::parameters() {
    // Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    emissions = bool(false);
    densities = bool(false);
    loosers = bool(false);
    bgcol = ubyte(0);
    textcol = ubyte(255);
    scaleFactor = int(3);
    statecol = ubyte(64);
    winnercol = ubyte(255);
  }

  // copy constructor
  hmmTrellisDiagram::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  hmmTrellisDiagram::parameters::~parameters() {
  }

  // get type name
  const char* hmmTrellisDiagram::parameters::getTypeName() const {
    return "hmmTrellisDiagram::parameters";
  }

  // copy member

  hmmTrellisDiagram::parameters&
    hmmTrellisDiagram::parameters::copy(const parameters& other) {
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


      emissions = other.emissions;
      densities = other.densities;
      loosers = other.loosers;
      bgcol = other.bgcol;
      textcol = other.textcol;
      scaleFactor = other.scaleFactor;
      statecol = other.statecol;
      winnercol = other.winnercol;

    return *this;
  }

  // alias for copy member
  hmmTrellisDiagram::parameters&
    hmmTrellisDiagram::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* hmmTrellisDiagram::parameters::clone() const {
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
  bool hmmTrellisDiagram::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmTrellisDiagram::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"emissions",emissions);
      lti::write(handler,"densities",densities);
      lti::write(handler,"loosers",loosers);
      lti::write(handler,"bgcol",bgcol);
      lti::write(handler,"textcol",textcol);
      lti::write(handler,"scaleFactor",scaleFactor);
      lti::write(handler,"statecol",statecol);
      lti::write(handler,"winnercol",winnercol);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmTrellisDiagram::parameters::write(ioHandler& handler,
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
  bool hmmTrellisDiagram::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmTrellisDiagram::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"emissions",emissions);
      lti::read(handler,"densities",densities);
      lti::read(handler,"loosers",loosers);
      lti::read(handler,"bgcol",bgcol);
      lti::read(handler,"textcol",textcol);
      lti::read(handler,"scaleFactor",scaleFactor);
      lti::read(handler,"statecol",statecol);
      lti::read(handler,"winnercol",winnercol);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmTrellisDiagram::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // hmmTrellisDiagram
  // --------------------------------------------------

  // default constructor
  hmmTrellisDiagram::hmmTrellisDiagram()
    : functor(){

    // comment the attributes of your functor
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
  hmmTrellisDiagram::hmmTrellisDiagram(const hmmTrellisDiagram& other)
    : functor()  {
    copy(other);
  }

  // destructor
  hmmTrellisDiagram::~hmmTrellisDiagram() {
  }

  // returns the name of this type
  const char* hmmTrellisDiagram::getTypeName() const {
    return "hmmTrellisDiagram";
  }

  // copy member
  hmmTrellisDiagram& hmmTrellisDiagram::copy(const hmmTrellisDiagram& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  hmmTrellisDiagram&
  hmmTrellisDiagram::operator=(const hmmTrellisDiagram& other) {
    return (copy(other));
  }


  // clone member
  functor* hmmTrellisDiagram::clone() const {
    return new hmmTrellisDiagram(*this);
  }

  // return parameters
  const hmmTrellisDiagram::parameters&
    hmmTrellisDiagram::getParameters() const {
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


  // On place apply for type channel8!
  bool hmmTrellisDiagram::apply(const lti::sequence<lti::dvector>& seq,
                                const lti::hiddenMarkovModel& hmm,
                                lti::channel8& result,
                                const std::string& text) const {

    // get const reference to the parameters
    const parameters& param = getParameters();

    // evaluate given observation sequence
    lti::hmmViterbiPathSearch viterbi;
    viterbi.logStateSelection(true);
    viterbi.logDensitySelection(true);
    viterbi.logEmissionScore(true);

    try {
      if (!viterbi.apply(hmm, seq)) {

        setStatusString("Couldn't evaluate sequence with given model");
        return false;
      }
    }
    catch (lti::exception e) {
      std::string msg = "Exception: ";
      msg = msg + e.what();
      setStatusString(msg.c_str());
      return false;
    }

    // get references to the required matrices and vectors
    const int size = param.scaleFactor;

    const lti::matrix<int>& bpMatrix = viterbi.getBackpointerMatrix();
    const lti::vector<int>& stateVec = viterbi.getStatePath();
    const lti::vector<int>& densVec = viterbi.getDensitySelectionPath();
    const lti::vector<double>& emissionVec = viterbi.getEmissionScorePath();

    // do testing
    if ( (stateVec.size()!=densVec.size()) ||
         (densVec.size()!=emissionVec.size()) ||
         (emissionVec.size()!=(bpMatrix.rows()+1)) ) {

      throw exception("Incompatible size of reference elements!");
    }

    double minScore, maxScore;
    emissionVec.getExtremes(minScore,maxScore);

    const lti::point grid(size*5,size*5);
    const lti::point border(size*5,size*5);
    const lti::point offset(0,size*5);
    const int radius = size;

    // Resize and initialize result channel
    const int states = bpMatrix.columns();
    const int frames = bpMatrix.rows() + 1;  // bpMatrix contains only transitions,
                                             // i.e. frames-1

    result.resize(lti::point((frames-1)*grid.x+2*border.x+offset.x,
                             (states-1)*grid.y+2*border.y+offset.y),
                             param.bgcol,
                             false,
                             true);


    // draw time-state grid in param.statecol
    lti::draw<lti::channel8::value_type> drawer;
    drawer.use(result);
    drawer.setColor(param.statecol);

    int x, y;
    for (x=0; x<frames; x++) {
      for (y=0; y<states; y++) {

        lti::point pos;
        pos.x = grid.x*(x) + border.x + offset.x;
        pos.y = grid.y*(y) + border.y + offset.y;

        if (radius<3) {
          drawer.set(pos);
        }
        else {
          drawer.circle(pos,radius,true);
        }
      }
    }


    // draw backpointer matrix
    int r, c, cc, cR, cL;
    lti::point posL;
    lti::point posR;
    lti::point pos;

    // draw all paths (including loosers), if desired in param.statecol
    if (param.loosers) {

      drawer.setColor(param.statecol);
      for (r=bpMatrix.lastRow();r>=0;r--) {

        for (c=bpMatrix.lastColumn();c>=0;c--) {

          cR = c;
          cL = bpMatrix.at(r,c);

          posL.x = grid.x*(r)   + border.x + offset.x;
          posL.y = result.size().y - (grid.y*(cL) + border.y);
          posR.x = grid.x*(r+1) + border.x + offset.x;
          posR.y = result.size().y - (grid.y*(cR) + border.y);

          drawer.line(posL,posR);
        }
      }
    }

    // draw density selection and score visualization
    if (param.densities || param.emissions) {
      for (r=stateVec.size()-1;r>=0;r--) {

        // next position
        cc = stateVec.at(r);

        pos.x = grid.x*(r) + border.x + offset.x;
        pos.y = result.size().y - (grid.y*(cc) + border.y);

        // draw state with grey value according to relative score
        if (param.emissions) {
          double score = emissionVec.at(r);
          int col = iround(255*fabs(maxScore - score)/
                           fabs(maxScore - minScore));
          drawer.setColor(col);

          if (radius<3) {
            drawer.set(pos);
          }
          else {
            drawer.circle(pos,radius,true);
          }
        }

        // write selected density
        if (param.densities ) {
          drawer.setColor(param.winnercol);

          posR.x = pos.x + size;
          posR.y = pos.y + size;

          std::ostringstream oss;
          oss << densVec.at(r);
          std::string num = oss.str();

          drawer.text(num,posR);
        }

      }

    }

    // now draw the winner path in param.winnercol
    drawer.setColor(param.winnercol);
    cc = bpMatrix.lastColumn();
    for (r=bpMatrix.lastRow();r>=0;r--) {

      // next position
      cR = cc;
      cL = cc = bpMatrix.at(r,cc);

      posL.x = grid.x*(r)   + border.x + offset.x;
      posL.y = result.size().y - (grid.y*(cL) + border.y);
      posR.x = grid.x*(r+1) + border.x + offset.x;
      posR.y = result.size().y - (grid.y*(cR) + border.y);

      drawer.line(posL,posR);
    }

    // write text in param.textcol
    drawer.setColor(param.textcol);
    pos.x = border.x + offset.x;
    pos.y = size;
    drawer.text(text,pos);

    return true;
  }


}
