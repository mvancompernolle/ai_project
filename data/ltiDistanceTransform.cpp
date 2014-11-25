/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiDistanceTransform.cpp
 * authors ....: Pablo Alvarado, Markus Radermacher
 * organization: LTI, RWTH Aachen
 * creation ...: 22.8.2001
 * revisions ..: $Id: ltiDistanceTransform.cpp,v 1.17 2006/09/05 10:09:19 ltilib Exp $
 */

#include "ltiDistanceTransform.h"

namespace lti {
  // --------------------------------------------------
  // distanceTransform::parameters
  // --------------------------------------------------

  // default constructor
  distanceTransform::parameters::parameters()
    : morphology::parameters(),
      distance(Euclidean) {
  }

  // copy constructor
  distanceTransform::parameters::parameters(const parameters& other)
    : morphology::parameters() {
    copy(other);
  }

  // destructor
  distanceTransform::parameters::~parameters() {
  }

  // get type name
  const char* distanceTransform::parameters::getTypeName() const {
    return "distanceTransform::parameters";
  }

  // copy member

  distanceTransform::parameters&
    distanceTransform::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    morphology::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    morphology::parameters& (morphology::parameters::* p_copy)
      (const morphology::parameters&) =
      morphology::parameters::copy;
    (this->*p_copy)(other);
# endif


    distance = other.distance;

    return *this;
  }

  // alias for copy member
  distanceTransform::parameters&
    distanceTransform::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* distanceTransform::parameters::clone() const {
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
  bool distanceTransform::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool distanceTransform::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch(distance) {
        case EightNeighborhood:
          lti::write(handler, "distance","EightNeighborhood");
          break;
        case FourNeighborhood:
          lti::write(handler, "distance","FourNeighborhood");
          break;
        case EuclideanSqr:
          lti::write(handler, "distance","EuclideanSqr");
          break;
        case Euclidean:
          lti::write(handler, "distance","Euclidean");
          break;
        case EightSED:
            lti::write(handler,"distance","EightSED");
          break;
        case EightSEDSqr:
            lti::write(handler,"distance","EightSEDSqr");
          break;
        case FourSED:
            lti::write(handler,"distance","FourSED");
          break;
        case FourSEDSqr:
            lti::write(handler,"distance","FourSEDSqr");
          break;

        default:
          lti::write(handler, "distance","EightNeighborhood");
          break;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && morphology::parameters::write(handler,false);
# else
    bool (morphology::parameters::* p_writeMS)(ioHandler&,const bool) const =
      morphology::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool distanceTransform::parameters::write(ioHandler& handler,
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
  bool distanceTransform::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool distanceTransform::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"distance",str);
      if (str == "EightNeighborhood") {
         distance = EightNeighborhood;
      } else if (str == "FourNeighborhood") {
        distance = FourNeighborhood;
      } else if (str == "EuclideanSqr") {
        distance = EuclideanSqr;
      } else if (str == "Euclidean") {
        distance = Euclidean;
      } else if (str == "EightSED") {
        distance = EightSED;
      } else if (str == "EightSEDSqr") {
        distance = EightSEDSqr;
      } else if (str == "FourSED") {
        distance = FourSED;
      } else if (str == "FourSEDSqr") {
        distance = FourSEDSqr;
      } else {
        distance = EightNeighborhood;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && morphology::parameters::read(handler,false);
# else
    bool (morphology::parameters::* p_readMS)(ioHandler&,const bool) =
      morphology::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool distanceTransform::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // distanceTransform - protected inline methods
  // --------------------------------------------------
   
  inline void distanceTransform::EDT_2D(channel& chnl) const {
    //voronoiEDT_2D must be called for every column
    for(int x = 0; x < chnl.columns(); ++x){
      voronoiEDT_2D(chnl, x);
    }
  }

  inline bool distanceTransform::removeEDT(const int du, 
                                           const int dv, 
                                           const int dw, 
                                           const int u, 
                                           const int v, 
                                           const int w) const {
    //11 integer expressions
    const int a = v - u;
    const int b = w - v;
    const int c = w - u;
    return ( (c * dv - b * du - a * dw) > (a * b * c) );
  }

  inline void distanceTransform::sedMask::query_distance(point &shortest, 
                                                         point &other)const{
    if((other.x >= 0) &&
       ((shortest.x < 0) || (shortest.absSqr() > other.absSqr()))) {
      shortest = other;
    }
  }

  // --------------------------------------------------
  // distanceTransform
  // --------------------------------------------------

  // default constructor
  distanceTransform::distanceTransform()
    : morphology(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // default constructor
  distanceTransform::distanceTransform(const parameters& par)
    : morphology(){

    // set the default parameters
    setParameters(par);

  }

  // copy constructor
  distanceTransform::distanceTransform(const distanceTransform& other)
    : morphology()  {
    copy(other);
  }

  // destructor
  distanceTransform::~distanceTransform() {
  }

  // returns the name of this type
  const char* distanceTransform::getTypeName() const {
    return "distanceTransform";
  }

  // copy member
  distanceTransform& distanceTransform::copy(const distanceTransform& other) {
    morphology::copy(other);

    return (*this);
  }

  // alias for copy member
  distanceTransform&
  distanceTransform::operator=(const distanceTransform& other) {
    return (copy(other));
  }

  // clone member
  functor* distanceTransform::clone() const {
    return new distanceTransform(*this);
  }

  // return parameters
  const distanceTransform::parameters&
    distanceTransform::getParameters() const {
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
  bool distanceTransform::apply(channel8& srcdest) const {
    channel tmp;
    tmp.castFrom(static_cast<matrix<ubyte> >(srcdest));
    if (apply(tmp)) {
      srcdest.castFrom(static_cast<matrix<float> >(tmp));
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool distanceTransform::apply(const channel8& src,channel8& dest) const {
    channel tmp;
    tmp.castFrom(static_cast<matrix<ubyte> >(src));
    if (apply(tmp)) {
      dest.castFrom(static_cast<matrix<float> >(tmp));
      return true;
    }
    return false;
  };


  // On place apply for type channel8!
  bool distanceTransform::apply(channel& srcdest) const {

    if ((srcdest.rows() < 2) || (srcdest.columns() < 2)) {
      setStatusString("At least 2 pixels at each axis expected");
      return false;
    }

    const parameters& param = getParameters();

    if(   param.distance == parameters::EightNeighborhood
       || param.distance == parameters::FourNeighborhood){
      // ensure that the non-zero values are maximal
      int y;
      vector<channel::value_type>::iterator it,eit;
      const float max = static_cast<float>(srcdest.rows()+srcdest.columns());
      for (y=0;y<srcdest.rows();y++) {
        vector<channel::value_type>& vct = srcdest.getRow(y);
        for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
          if ((*it)>0.0f) {
            (*it)=max;
          }
        }
      }
    }

    switch(param.distance){
    case parameters::EightNeighborhood:
      
      iteration8back(srcdest);
      iteration8(srcdest);
      return true;

    case parameters::FourNeighborhood:
      
      iteration4back(srcdest);
      iteration4(srcdest);
      return true;

    case parameters::Euclidean:
      
      EDT_1D(srcdest);
      EDT_2D(srcdest);
      srcdest.apply(sqrt);
      return true;

    case parameters::EuclideanSqr:

      EDT_1D(srcdest);
      EDT_2D(srcdest);
      return true;

    case parameters::EightSED:
    
      sedFiltering(srcdest, true);
      srcdest.apply(sqrt);
      return true;
    
    case parameters::EightSEDSqr:

      sedFiltering(srcdest,true);
      return true;

    case parameters::FourSED:

      sedFiltering(srcdest, false);
      srcdest.apply(sqrt);
      return true;

    case parameters::FourSEDSqr:

      sedFiltering(srcdest, false);
      return true;

    default:
      return false;
    }
  };

  // On copy apply for type channel8!
  bool distanceTransform::apply(const channel& src,channel& dest) const {

    dest.copy(src);
    return apply(dest);

  };

  void distanceTransform::iteration8(channel& chnl) const {
    int x,y,z;

    const int rowm1 = chnl.rows()-1;
    const int colm1 = chnl.columns()-1;

    static const int deltax[12] = {1,1,0,-1,-1,-1, 0, 1,1,1,0,-1};
    static const int deltay[12] = {0,1,1, 1, 0,-1,-1,-1,0,1,1, 1};

    float minimum;

    // upper-left
    if (chnl.at(0,0) > 0) {
      chnl.at(0,0) = 1.0f+min(chnl.at(0,1),chnl.at(1,1),chnl.at(1,0));
    }

    // top
    y = 0;
    for (x=1;x<colm1;++x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[0],x+deltax[0]);

        for (z=1;z<5;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-right
    if (chnl.at(0,colm1) > 0) {
      chnl.at(0,colm1) = 1.0f+min(chnl.at(0,colm1-1),chnl.at(1,colm1-1),
                                  chnl.at(1,colm1));
    }

    // inner of the image only...
    for (y=1;y<rowm1;++y) {
      // left border
      x = 0;
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[6],x+deltax[6]);

        for (z=7;z<11;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }

      // inner of the line
      for (x=1;x<colm1;++x) {
        if (chnl.at(y,x) > 0) {
          // valid pixel, let's check for the distance value
          minimum = chnl.at(y+deltay[0],x+deltax[0]);

          for (z=1;z<8;++z) {
            minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
          }

          chnl.at(y,x) = minimum+1.0f;
        }
      }

      // right border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[2],x+deltax[2]);

        for (z=3;z<7;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // bottom-left
    if (chnl.at(rowm1,0) > 0) {
      chnl.at(rowm1,0) = 1.0f+min(chnl.at(rowm1,1),chnl.at(rowm1-1,1),
                                  chnl.at(rowm1-1,0));
    }

    // bottom
    for (x=1;x<colm1;++x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[4],x+deltax[4]);

        for (z=5;z<9;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // bottom-right
    if (chnl.at(rowm1,colm1) > 0) {
      chnl.at(rowm1,colm1) = 1.0f+min(chnl.at(rowm1,colm1-1),
                                      chnl.at(rowm1-1,colm1-1),
                                      chnl.at(rowm1-1,colm1));
    }

  }

  // iteration for 4-neighborhood

  void distanceTransform::iteration4(channel& chnl) const {
    int x,y,z;

    const int rowm1 = chnl.rows()-1;
    const int colm1 = chnl.columns()-1;

    static const int deltax[6] = {1,0,-1, 0, 1,0};
    static const int deltay[6] = {0,1, 0,-1, 0,1};

    float minimum;

    // upper-left
    if (chnl.at(0,0) > 0) {
      chnl.at(0,0) = 1.0f+min(chnl.at(0,1),chnl.at(1,0));
    }

    // top
    y = 0;
    for (x=1;x<colm1;++x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[0],x+deltax[0]);

        for (z=1;z<3;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-right
    if (chnl.at(0,colm1) > 0) {
      chnl.at(0,colm1) = 1.0f+min(chnl.at(0,colm1-1),chnl.at(1,colm1));
    }

    // inner of the image only...
    for (y=1;y<rowm1;++y) {
      // left border
      x = 0;
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[3],x+deltax[3]);

        for (z=0;z<2;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }

      // inner of the line
      for (x=1;x<colm1;++x) {
        if (chnl.at(y,x) > 0) {
          // valid pixel, let's check for the distance value
          minimum = chnl.at(y+deltay[0],x+deltax[0]);

          for (z=1;z<4;++z) {
            minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
          }

          chnl.at(y,x) = minimum+1.0f;
        }
      }

      // right border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[1],x+deltax[1]);

        for (z=2;z<4;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // bottom-left
    if (chnl.at(rowm1,0) > 0) {
      chnl.at(rowm1,0) = 1.0f+min(chnl.at(rowm1,1),chnl.at(rowm1-1,0));
    }

    // bottom
    for (x=1;x<colm1;++x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[2],x+deltax[2]);

        for (z=3;z<5;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // bottom-right
    if (chnl.at(rowm1,colm1) > 0) {
      chnl.at(rowm1,colm1) = 1.0f+min(chnl.at(rowm1,colm1-1),
                                      chnl.at(rowm1-1,colm1));
    }
  }

  void distanceTransform::iteration8back(channel& chnl) const {
    int x,y,z;

    const int rowm1 = chnl.lastRow();
    const int colm1 = chnl.lastColumn();

    static const int deltax[12] = {1,1,0,-1,-1,-1, 0, 1,1,1,0,-1};
    static const int deltay[12] = {0,1,1, 1, 0,-1,-1,-1,0,1,1, 1};

    float minimum;

    // bottom-right
    if (chnl.at(rowm1,colm1) > 0) {
      chnl.at(rowm1,colm1) = 1.0f+min(chnl.at(rowm1,colm1-1),
                                      chnl.at(rowm1-1,colm1-1),
                                      chnl.at(rowm1-1,colm1));
    }
    // bottom
    y = rowm1;
    for (x=colm1-1;x>0;--x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[4],x+deltax[4]);

        for (z=5;z<9;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }
    // bottom-left
    if (chnl.at(rowm1,0) > 0) {
      chnl.at(rowm1,0) = 1.0f+min(chnl.at(rowm1,1),chnl.at(rowm1-1,1),
                                  chnl.at(rowm1-1,0));
    }

    // inner of the image only...
    for (y=rowm1-1;y>0;--y) {
      x = colm1;
      // right border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[2],x+deltax[2]);

        for (z=3;z<7;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }

      // inner of the line
      for (x=colm1-1;x>0;--x) {
        if (chnl.at(y,x) > 0) {
          // valid pixel, let's check for the distance value
          minimum = chnl.at(y+deltay[0],x+deltax[0]);

          for (z=1;z<8;++z) {
            minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
          }

          chnl.at(y,x) = minimum+1.0f;
        }
      }

      // left border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[6],x+deltax[6]);

        for (z=7;z<11;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-right
    if (chnl.at(0,colm1) > 0) {
      chnl.at(0,colm1) = 1.0f+min(chnl.at(0,colm1-1),chnl.at(1,colm1-1),
                                  chnl.at(1,colm1));
    }

    // top
    for (x=colm1-1;x>0;--x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[0],x+deltax[0]);

        for (z=1;z<5;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-left
    if (chnl.at(0,0) > 0) {
      chnl.at(0,0) = 1.0f+min(chnl.at(0,1),chnl.at(1,1),chnl.at(1,0));
    }

  }

  void distanceTransform::iteration4back(channel& chnl) const {
    int x,y,z;

    const int rowm1 = chnl.lastRow();
    const int colm1 = chnl.lastColumn();

    static const int deltax[6] = {1,0,-1, 0, 1,0};
    static const int deltay[6] = {0,1, 0,-1, 0,1};

    float minimum;

    // bottom-right
    if (chnl.at(rowm1,colm1) > 0) {
      chnl.at(rowm1,colm1) = 1.0f+min(chnl.at(rowm1,colm1-1),
                                      chnl.at(rowm1-1,colm1));
    }

    // bottom
    y = rowm1;
    for (x=colm1-1;x>0;--x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[2],x+deltax[2]);

        for (z=3;z<5;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // bottom-left
    if (chnl.at(rowm1,0) > 0) {
      chnl.at(rowm1,0) = 1.0f+min(chnl.at(rowm1,1),
                                  chnl.at(rowm1-1,0));
    }

    // inner of the image only...
    for (y=rowm1-1;y>0;--y) {
      x = colm1;
      // right border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[1],x+deltax[1]);

        for (z=2;z<4;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }

      // inner of the line
      for (x=colm1-1;x>0;--x) {
        if (chnl.at(y,x) > 0) {
          // valid pixel, let's check for the distance value
          minimum = chnl.at(y+deltay[0],x+deltax[0]);

          for (z=1;z<4;++z) {
            minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
          }

          chnl.at(y,x) = minimum+1.0f;
        }
      }

      // left border
      if (chnl.at(y,x) > 0) {
        minimum = chnl.at(y+deltay[3],x+deltax[3]);

        for (z=0;z<2;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-right
    if (chnl.at(0,colm1) > 0) {
      chnl.at(0,colm1) = 1.0f+min(chnl.at(0,colm1-1),
                                  chnl.at(1,colm1));
    }

    // top
    for (x=colm1-1;x>0;--x) {
      if (chnl.at(y,x) > 0) {
        // valid pixel, let's check for the distance value
        minimum = chnl.at(y+deltay[0],x+deltax[0]);

        for (z=1;z<3;++z) {
          minimum = min(minimum,chnl.at(y+deltay[z],x+deltax[z]));
        }

        chnl.at(y,x) = minimum+1.0f;
      }
    }

    // upper-left
    if (chnl.at(0,0) > 0) {
      chnl.at(0,0) = 1.0f+min(chnl.at(0,1),chnl.at(1,0));
    }

  }

  void distanceTransform::voronoiEDT_2D(channel& chnl, const int j) const {
    int l = -1,
        fi;
    vector<int> g(chnl.rows()),
                h(chnl.rows());
    int x0 = j,
        x1;
  
    for(x1 = 0; x1 < chnl.rows(); ++x1){
      fi = static_cast<int>(chnl.at(x1, x0));
      if(fi >= 0.0f){  //any value below zero is undefined
        while(   l >= 1 
              && removeEDT(g.at(l - 1), g.at(l), fi, h.at(l - 1), h.at(l), x1))
          --l;
        ++l; g.at(l) = fi; h.at(l) = x1;
      }
    }
    if(l == -1) return;
    int ns = l;
    l = 0;
    for(x1 = 0; x1 < chnl.rows(); ++x1){
      int tmp0 = h.at(l) - x1,
          tmp1 = g.at(l) + tmp0 * tmp0,
          tmp2;
    
      while(true){
        if(l < ns){
          tmp2 = (h.at(l + 1) - x1);
          if(tmp1 > g.at(l + 1) + tmp2 * tmp2){
            ++l;
            tmp0 = h.at(l) - x1;
            tmp1 = g.at(l) + tmp0 * tmp0;
          }else break;
        }else break;
      }
        
      chnl.at(x1, x0) = static_cast<float>(tmp1);
    }
  }

  void distanceTransform::EDT_1D(channel& chnl) const {

    const float undef = -1.0f;  //means any undefined value (distance or pos)
    
    //remember: all foreground pixel are >  0.0f
    //          all background pixel are == 0.0f
    for(int y = 0; y < chnl.rows(); ++y){
      int x, pos = static_cast<int>(undef);
      //first step: forward propagation
      for(x = 0; x < chnl.columns(); ++x){
        if(chnl.at(y, x) == 0.0f){
          //found background pixel
          //now 0.0 means distance to closest background pixel
          pos = x; 
        }
        else if(pos >= 0){
          int tmp = pos - x;
          chnl.at(y, x) = static_cast<float>(tmp * tmp);
        }
        else
          chnl.at(y, x) = undef;
      }
    
      //no background pixel in row => all pixel are set to undef;
      //continue with next row
      if(pos == undef) continue;
      else{
        pos = static_cast<int>(undef);
        for(x = chnl.columns() - 1; x >= 0; --x){
          if(chnl.at(y, x) == 0){
            pos = x; //found fv
          }
          else if(pos != undef){
            int tmp = pos - x;
            tmp *=tmp;
            int ret = static_cast<int>(chnl.at(y, x));
            if(ret > tmp || ret == undef){
              chnl.at(y, x) = static_cast<float>(tmp);
            }
          }
        }
      }
    }
  }

  void distanceTransform::sedFiltering(channel &chnl, 
                                        bool useEightSED) const {

    const float fv  = 0.0f;
    const int undef = -2;

    matrix<point> dist(chnl.size());

    int row, 
        col;

    //init
    for(row = 0; row < chnl.rows(); ++row){
      for(col = 0; col < chnl.columns(); ++col){
        if(chnl.at(row, col) == fv)
          dist.at(row, col) = point(0, 0);
        else
          dist.at(row, col) = point(undef, undef);
      }
    }

    if(useEightSED) 
      eightSEDFiltering(chnl, dist);
    else            
      fourSEDFiltering(chnl, dist);

    //set the distances 
    for(row = 0; row < chnl.rows(); ++row)
      for(col = 0; col < chnl.columns(); ++col)
        chnl.at(row, col) = static_cast<float>(dist.at(row, col).distanceSqr(point(0,0)));
  }

  void distanceTransform::fourSEDFiltering(channel &chnl, 
                                            matrix<point> &dist) const {
    
    //create all masks
    point mask0[] = { point(-1, 0) };
    sedMask l(mask0, 1);

    point mask1[] = { point(0, -1) };
    sedMask u(mask1, 1);
    
    point mask2[] = { point(0, -1), point(-1, 0) };
    sedMask ul(mask2, 2);

    point mask3[] = { point(1, 0) };
    sedMask r(mask3, 1);

    point mask4[] = { point(0, 1) };
    sedMask d(mask4, 1);
    
    point mask5[] = { point(1, 0), point(0, 1) };
    sedMask rd(mask5, 2);

    
    point pos;
    pos.y = 0;
    
    //first line
    for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
      l.filter(dist, pos);
    for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
      r.filter(dist, pos);

    for(pos.y = 1; pos.y < chnl.rows(); ++pos.y){
      
      pos.x = 0;
      //step down
      u.filter(dist, pos);

      for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
        ul.filter(dist, pos);
      for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
        r.filter(dist, pos);
    }

    //and now filter the picture in the opposite direction
    pos.y = chnl.rows() - 1;

    //last line
    for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
      r.filter(dist, pos);
    for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
      l.filter(dist, pos);

    for(pos.y = chnl.rows() - 2; pos.y >= 0; --pos.y){
      
      pos.x = chnl.columns() - 1;     
      //step up
      d.filter(dist, pos);

      for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
        rd.filter(dist, pos);
      for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
        l.filter(dist, pos);
    }
  }
  
  void distanceTransform::eightSEDFiltering(channel &chnl,
                                            matrix<point> &dist)const{
  
    //create all masks
    point mask0[] = { point(-1, 0) };
    sedMask xo(mask0, 1);

    point mask1[] = { point(-1,-1), point(0,-1), point(1,-1), point(-1, 0) };
    sedMask xxxxo(mask1, 4);

    point mask2[] = { point(-1, -1), point(0, -1), point(-1, 0) };
    sedMask xxxo(mask2, 3);

    point mask3[] = { point(0, -1), point(1, -1) };
    sedMask xxo(mask3, 2);

    
    point mask4[] = { point(1, 0) };
    sedMask ox(mask4, 1);

    point mask5[] = { point(1, 0), point(-1, 1), point(0, 1), point(1, 1) };
    sedMask oxxxx(mask5, 4);

    point mask6[] = { point(1, 0), point(0, 1), point(1, 1) };
    sedMask oxxx(mask6, 3);

    point mask7[] = { point(-1, 1), point(0, 1) };
    sedMask oxx(mask7, 2);


    //filter the picture
    point pos;
    pos.y = 0; 
    
    //first row
    for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
      xo.filter(dist, pos);
    for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
      ox.filter(dist, pos);

    for(pos.y = 1; pos.y < chnl.rows(); ++pos.y){
      pos.x = 0;  
      //step up
      xxo.filter(dist, pos);

      for(pos.x = 1; pos.x < chnl.columns() - 1; ++pos.x)
        xxxxo.filter(dist, pos);
      xxxo.filter(dist, pos);
      for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
        ox.filter(dist, pos);
    }

    //and now filter the picture in the opposite direction
    pos.y = chnl.rows() - 1;

    //last row
    for(pos.x = chnl.columns() - 2; pos.x >= 0; --pos.x)
      ox.filter(dist, pos);
    for(pos.x = 1; pos.x < chnl.columns(); ++pos.x)
      xo.filter(dist, pos);

    for(pos.y = chnl.rows() - 2; pos.y >= 0; --pos.y){
      pos.x = chnl.columns() - 1;
      //step down
      oxx.filter(dist, pos);

      for(pos.x = chnl.columns() - 2; pos.x > 0; --pos.x)
        oxxxx.filter(dist, pos);
      oxxx.filter(dist, pos);
      for(pos.x = 1; pos.x < chnl.columns() - 1; ++pos.x)
        xo.filter(dist, pos);
    }
  }


  distanceTransform::sedMask::sedMask(const point mask[], 
                                      int size) 
    : mask_(mask), size_(size){

  }

  distanceTransform::sedMask::~sedMask() {
  }

  void distanceTransform::sedMask::filter(matrix<point> &dist, 
                                          const point   &pos) const {
      
    point shortest, 
          tmp;

    shortest =   dist.at(pos.y + mask_[0].y, pos.x + mask_[0].x) 
               + point(abs(mask_[0].x), abs(mask_[0].y));
    
    for(int i = 1; i < size_; ++i){  
      
      tmp =   dist.at(pos.y + mask_[i].y, pos.x + mask_[i].x) 
            + point(abs(mask_[i].x), abs(mask_[i].y));
      
      query_distance(shortest, tmp);
    }

    tmp = dist.at(pos);
    query_distance(shortest, tmp);

    if(shortest.x >= 0)
      dist.at(pos) = shortest;

  }

}  //end namespace lti
