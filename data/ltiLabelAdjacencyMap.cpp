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
 * file .......: ltiLabelAdjacencyMap.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.11.2002
 * revisions ..: $Id: ltiLabelAdjacencyMap.cpp,v 1.11 2006/09/05 10:19:40 ltilib Exp $
 */

#include "ltiLabelAdjacencyMap.h"
#include "ltiColors.h"
#include "ltiUsePalette.h"
#include <limits>

namespace lti {
  // --------------------------------------------------
  // labelAdjacencyMap::parameters
  // --------------------------------------------------

  static const rgbPixel palData[] = {lti::Black,
                                     lti::Red,       lti::Green, lti::Blue,
                                     lti::Yellow,    lti::Cyan,  lti::Magenta,
                                     lti::DarkOrange,lti::DarkGreen,
                                     lti::DarkCyan,  lti::Violet,
                                     lti::Grey75};

  /*
   * Defaut color palette
   */
  const palette labelAdjacencyMap::parameters::defaultPalette(12,palData);

  // default constructor
  labelAdjacencyMap::parameters::parameters()
    : functor::parameters() {
    minColors = bool(false);
    thePalette.copy(defaultPalette);
    neighborhood = int(8);
  }

  // copy constructor
  labelAdjacencyMap::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  labelAdjacencyMap::parameters::~parameters() {
  }

  // get type name
  const char* labelAdjacencyMap::parameters::getTypeName() const {
    return "labelAdjacencyMap::parameters";
  }

  // copy member

  labelAdjacencyMap::parameters&
    labelAdjacencyMap::parameters::copy(const parameters& other) {
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


    minColors = other.minColors;
    thePalette.copy(other.thePalette);
    neighborhood = other.neighborhood;

    return *this;
  }

  // alias for copy member
  labelAdjacencyMap::parameters&
    labelAdjacencyMap::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* labelAdjacencyMap::parameters::clone() const {
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
  bool labelAdjacencyMap::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool labelAdjacencyMap::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"minColors",minColors);
      lti::write(handler,"thePalette",thePalette);
      lti::write(handler,"neighborhood",neighborhood);
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
  bool labelAdjacencyMap::parameters::write(ioHandler& handler,
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
  bool labelAdjacencyMap::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool labelAdjacencyMap::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"minColors",minColors);
      lti::read(handler,"thePalette",thePalette);
      lti::read(handler,"neighborhood",neighborhood);
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
  bool labelAdjacencyMap::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // labelAdjacencyMap
  // --------------------------------------------------

  // default constructor
  labelAdjacencyMap::labelAdjacencyMap()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  labelAdjacencyMap::labelAdjacencyMap(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  labelAdjacencyMap::labelAdjacencyMap(const labelAdjacencyMap& other)
    : functor()  {
    copy(other);
  }

  // destructor
  labelAdjacencyMap::~labelAdjacencyMap() {
  }

  // returns the name of this type
  const char* labelAdjacencyMap::getTypeName() const {
    return "labelAdjacencyMap";
  }

  // copy member
  labelAdjacencyMap&
  labelAdjacencyMap::copy(const labelAdjacencyMap& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  labelAdjacencyMap&
  labelAdjacencyMap::operator=(const labelAdjacencyMap& other) {
    return (copy(other));
  }


  // clone member
  functor* labelAdjacencyMap::clone() const {
    return new labelAdjacencyMap(*this);
  }

  // return parameters
  const labelAdjacencyMap::parameters&
    labelAdjacencyMap::getParameters() const {
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

  // Private class here and not inner class of the functor due to
  // MS-VC++ bugs (otherwise the implementation should be in the
  // header).
  template<class T>
  class adjacencyHelper {
  public:
    /**
     * compute the adjacency for a 4 neighborhood
     */
    bool adjacency4(const matrix<T>& src,sparseMatrix<int>& adjMap) {
      T minl,maxl;
      int x,y;
      src.getExtremes(minl,maxl);

      x = static_cast<int>(minl);
      if (x<0) {
        return false;
      }

      const int lastx = src.lastColumn();
      const int lasty = src.lastRow();

      adjMap.resize(maxl+1,maxl+1,0);
      
      for (y=0;y<lasty;++y) {
        for (x=0;x<lastx;++x) {
          
          const T a = src.at(y,x);
          const T b = src.at(y,x+1);
          const T c = src.at(y+1,x);
          
          if (a != b) {
            adjMap.forceAt(a,b)++;
            adjMap.forceAt(b,a)++;
          }
          
          if (a != c) {
            adjMap.forceAt(a,c)++;
            adjMap.forceAt(c,a)++;
          }
        }
      }
      return true;
    }

    /**
     * compute the adjacency for an 8 neighborhood
     */
    bool adjacency8(const matrix<T>& src,sparseMatrix<int>& adjMap) {
      int x,y;
      T minl,maxl;
      src.getExtremes(minl,maxl);

      x = static_cast<int>(minl);
      if (x<0) {
        return false;
      }

      const int lastx = src.lastColumn();
      const int lasty = src.lastRow();

      adjMap.resize(maxl+1,maxl+1,0);
      
      for (y=0;y<lasty;++y) {
        for (x=0;x<lastx;++x) {
          const T a = src.at(y,x);
          const T b = src.at(y,x+1);
          const T c = src.at(y+1,x);
          const T d = src.at(y+1,x+1);
          
          if (a != b) {
            adjMap.forceAt(a,b)++;
            adjMap.forceAt(b,a)++;
          }
          
          if (a != c) {
            adjMap.forceAt(a,c)++;
            adjMap.forceAt(c,a)++;
          }
          
          if (a != d) {
            adjMap.forceAt(a,d)++;
            adjMap.forceAt(d,a)++;
          }
          
          if (b != c) {
            adjMap.forceAt(b,c)++;
            adjMap.forceAt(c,b)++;
          }
        }
      }
      return true;
    }
  };

    // On copy apply for type channel8!
  bool labelAdjacencyMap::apply(const matrix<ubyte>& src,image& dest) const {
    sparseMatrix<int> adj;
    usePalette colorizer;
    if (adjacency(src,adj)) {
      const parameters& par = getParameters();
      palette pal;
      if (par.minColors) {
        computeMinPalette(adj,pal);
      } else {
        computeMaxPalette(adj,pal);
      }
      return colorizer.apply(src,pal,dest);
    }

    return false;
  };

  // On copy apply for type matrix<int>!
  bool labelAdjacencyMap::apply(const matrix<int>& src,image& dest) const {
    sparseMatrix<int> adj;
    usePalette colorizer;
    if (adjacency(src,adj)) {
      const parameters& par = getParameters();
      palette pal;
      if (par.minColors) {
        computeMinPalette(adj,pal);
      } else {
        computeMaxPalette(adj,pal);
      }
      return colorizer.apply(src,pal,dest);
    }

    return false;
  };

  // On copy apply for type matrix<int>!
  bool labelAdjacencyMap::adjacency(const matrix<int>& src,
                                          sparseMatrix<int>& dest) const {

    adjacencyHelper<int> helper;
    bool result(false);
    const parameters& par = getParameters();
    if (par.neighborhood == 4) {
      result = helper.adjacency4(src,dest);
    } else {
      result = helper.adjacency8(src,dest);
    }

    if (!result) {
      setStatusString("Invalid labeled mask: negative labels");
      return false;
    }

    return true;
  };

  // On copy apply for type matrix<int>!
  bool labelAdjacencyMap::adjacency(const matrix<ubyte>& src,
                                          sparseMatrix<int>& dest) const {

    adjacencyHelper<ubyte> helper;
    const parameters& par = getParameters();
    if (par.neighborhood == 4) {
      return helper.adjacency4(src,dest);
    } else {
      return helper.adjacency8(src,dest);
    }
    return false;
  };

  bool labelAdjacencyMap::computeMinPalette(const sparseMatrix<int>& adj,
                                            palette& pal) const {

    // access to the adjacency matrix
    const ivector& rows = adj.getRowPtr();
    const std::vector<int>& cols = adj.getColIndex();

    const int last = rows.size()-1;
    const parameters& par = getParameters();
    const int palsize = par.thePalette.size();

    pal.resize(last,Black,false,true);
    ivector apal(last,-1); // -1 means, entry not assigned yet
    ivector tpal(palsize,-1);

    int i,j,f,l;
    for (i=0;i<last;++i) {
      f=rows.at(i);
      l=rows.at(i+1);

      // mark all palette entries already used by the neighbors of
      // this label
      tpal.fill(-1);

      for (j=f;j<l;++j) {
        if (apal.at(cols[j]) >= 0) {
          tpal.at(apal.at(cols[j])) = +1;
        }
      }

      // choose the first not used palette entry
      j=0;
      while (j<palsize && tpal.at(j)>=0) {
        ++j;
      };

      // assign the first found label
      apal.at(i) = j % palsize;
    }

    for (i=0;i<last;++i) {
      pal.at(i)=par.thePalette.at(apal.at(i));
    }

    return true;
  }


  bool labelAdjacencyMap::computeMaxPalette(const sparseMatrix<int>& adj,
                                            palette& pal) const {
    // access to the adjacency matrix
    const ivector& rows = adj.getRowPtr();
    const std::vector<int>& cols = adj.getColIndex();

    const int last = rows.size()-1;
    const parameters& par = getParameters();
    const int palsize = par.thePalette.size();

    pal.resize(last,Black,false,true);
    ivector apal(last,-1); // -1 means, entry not assigned yet
    ivector tpal(palsize,-1);

    int i,j,f,l,k=0;
    for (i=0;i<last;++i) {
      f=rows.at(i);
      l=rows.at(i+1);

      // mark all palette entries already used by the neighbors of
      // this label
      tpal.fill(-1);

      for (j=f;j<l;++j) {
        if (apal.at(cols[j]) >= 0) {
          tpal.at(apal.at(cols[j])) = +1;
        }
      }

      // choose the first not used palette entry
      j=0;
      k=i;
      while (j<palsize && tpal.at((k+j)%palsize)>=0) {
        ++j;
      };

      // assign the first found label
      apal.at(i) = (k+j) % palsize;
    }

    for (i=0;i<last;++i) {
      pal.at(i)=par.thePalette.at(apal.at(i));
    }

    return true;
  }
}
