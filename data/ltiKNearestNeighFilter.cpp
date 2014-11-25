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

/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiKNearestNeighFilter.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 3.2.2002
 * revisions ..: $Id: ltiKNearestNeighFilter.cpp,v 1.11 2006/09/05 10:18:25 ltilib Exp $
 */

#include <list>

#include "ltiKNearestNeighFilter.h"

namespace lti {
  // --------------------------------------------------
  // kNearestNeighFilter::parameters
  // --------------------------------------------------

  // default constructor
  kNearestNeighFilter::parameters::parameters()
    : filter::parameters() {

    kernelSize = 5;
  }

  // copy constructor
  kNearestNeighFilter::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    copy(other);
  }

  // destructor
  kNearestNeighFilter::parameters::~parameters() {
  }

  // get type name
  const char* kNearestNeighFilter::parameters::getTypeName() const {
    return "kNearestNeighFilter::parameters";
  }

  // copy member

  kNearestNeighFilter::parameters&
    kNearestNeighFilter::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    filter::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    filter::parameters& (filter::parameters::* p_copy)
      (const filter::parameters&) =
      filter::parameters::copy;
    (this->*p_copy)(other);
# endif

    kernelSize = other.kernelSize;

    return *this;
  }

  // alias for copy member
  kNearestNeighFilter::parameters&
    kNearestNeighFilter::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kNearestNeighFilter::parameters::clone() const {
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
  bool kNearestNeighFilter::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool kNearestNeighFilter::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

	lti::write(handler,"kernelSize",kernelSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (filter::parameters::* p_writeMS)(ioHandler&,const bool) const =
      filter::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kNearestNeighFilter::parameters::write(ioHandler& handler,
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
  bool kNearestNeighFilter::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kNearestNeighFilter::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

	lti::read(handler,"kernelSize",kernelSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (filter::parameters::* p_readMS)(ioHandler&,const bool) =
      filter::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kNearestNeighFilter::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // kNearestNeighFilter
  // --------------------------------------------------

  // default constructor
  kNearestNeighFilter::kNearestNeighFilter()
    : filter(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  kNearestNeighFilter::kNearestNeighFilter(const kNearestNeighFilter& other)
    : filter()  {
    copy(other);
  }

  // destructor
  kNearestNeighFilter::~kNearestNeighFilter() {
  }

  // returns the name of this type
  const char* kNearestNeighFilter::getTypeName() const {
    return "kNearestNeighFilter";
  }

  // copy member
  kNearestNeighFilter&
  kNearestNeighFilter::copy(const kNearestNeighFilter& other) {
    filter::copy(other);

    return (*this);
  }

  // alias for copy member
  kNearestNeighFilter&
    kNearestNeighFilter::operator=(const kNearestNeighFilter& other) {
    return (copy(other));
  }


  // clone member
  functor* kNearestNeighFilter::clone() const {
    return new kNearestNeighFilter(*this);
  }

  // return parameters
  const kNearestNeighFilter::parameters&
    kNearestNeighFilter::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // the kernel runs inside the image
  void kNearestNeighFilter::histogramMethodMiddle(const imatrix& src,
                                              imatrix& dest,
                                              ivector& histogram,
                                              const int& row,int& col) const {
    int i,j;//index
    int numOfMax, maxIndex;
    int max=0;
    const int maxChange = sizeOfKernel+1;//max change for "max"

    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int lastCol = src.lastColumn()-limit;
    const int r = row+limit;
    col = limit;

    int v; //del test
    while(col <= (lastCol-1)) {
      j = col-limit;
      // sub labels left form the kernel
      for(i=row-limit;i<=r;++i) {
        --histogram.at(src.at(i,j));
      }
      // add labels right from the kernel
      ++col;
      j = col+limit;
      for(i=row-limit;i<=r;++i) {
	v = src.at(i,j);
        ++histogram.at(src.at(i,j));
      }

      //get most(best) available label
      numOfMax = 0;
      maxIndex = -1;
      max -= maxChange; //=0;
      for(i=0;i<histoSize;++i) {
	if(histogram.at(i) < max);// for speed up (probability)
	else if(histogram.at(i) > max) {
	  max = histogram.at(i);
	  numOfMax = 1;
	  maxIndex = i;
	}
	else //if(histogram.at(i) == max)
	  ++numOfMax;
      }

      //is there more than one possibility ?
      if(numOfMax == 1)
	dest.at(row,col) =  maxIndex;
      // is the kernel center one of the max's?
      else if(histogram.at(src.at(row,col)) == max)
	dest.at(row,col) = src.at(row,col);
      else
	dest.at(row,col) = getMedian(histogram,max,numOfMax);
    }//while
  };

  //get most(best) available label from histogram
  int kNearestNeighFilter::getMostLabel(const ivector& histogram,
			       const imatrix& src,
			       const int& row, const int& col) const{

    int numOfMax = 0;
    int maxIndex = -1; // first index, which is max
    int max = 0; //
    for(int i=0;i<histoSize;++i) {
      if(histogram.at(i) < max); // for speed up (probability)
      else if(histogram.at(i) > max) {
	max = histogram.at(i);
	numOfMax = 1;
	maxIndex = i;
      }
      else //if(histogram.at(i) == max)
	++numOfMax;
    }

    //is there more than one possibility ?
    if (numOfMax == 1)
      return maxIndex;
    // is the kernel center one of the max's?
    else if(histogram.at(src.at(row,col)) == max)
      return src.at(row,col);
    else
      return getMedian(histogram,max,numOfMax);
  };

  int kNearestNeighFilter::getMedian(const ivector& histogram,
                                     const int max,
                                     const int numOfMax) const {
    ivector vect(numOfMax,0);
    int i,z=0;
    const int size=histogram.size();
    for(i=0;i<size;++i) {
      if (histogram.at(i) == max) {
        vect.at(z++) = i;
      }
    }

    return vect.at(z/2);
  }

   // applies the histogramMethod for the type boundary Constant
  bool kNearestNeighFilter::histogramMethodConstant(const imatrix& src,
					   imatrix& dest) const {

    // image is divided in 9 areas, which are calc one by one
    //  1|  2  |3
    //  ---------
    //   |     |
    //  4|  5  |6
    //   |     |
    // ----------
    //  7|  8  |9

    int i,j,row,col;//index

    ivector histogram(histoSize,0);
    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;
    dest.resize(src.size(),0,false,false);

    //runs through the src's columns
    // (area:4,5,6) only kernels with full kernel-height
    for(row=limit;row<=lastRow;++row) {
      histogram.fill(0);

      // first full kernel (area4)
      col=0;
      const int r = row+limit;
      const int c = col+limit;
      for(i=row-limit;i<=r;++i)
	for(j=col-limit;j<=c;++j) { //todo better
	  if(j<0)
            ++histogram.at(src.at(i,0));
	  else// if(j>=0)
            ++histogram.at(src.at(i,j));
        }
      dest.at(row,col)=getMostLabel(histogram,src,row,col);

      // rest (area 4)
      while(col < limit) {
        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,0));
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i)
          ++histogram.at(src.at(i,j));
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }

      // area 5
      histogramMethodMiddle(src,dest,histogram,row,col);

      // area 6
      col=lastCol;
      while(col < (columnSize-1)) {
        j=col-limit;
        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,j));
        ++col;
	j = columnSize-1;
	for(i=row-limit;i<=r;++i)
          ++histogram.at(src.at(i,j));
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
    } // area 4,5,6

    // areas 1,2,3,7,8,9
    for(row=0;row<limit;++row) {     //runs top rows (1,2,3)
      const int r=row+limit;

      // runs middle top rows (area 2)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              ++histogram.at(src.at(0,j));
	    else//if(i>=0) {
              ++histogram.at(src.at(i,j));
          }
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
      // runs left top corner (area 1)
      for(col=0;col<limit;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              ++histogram.at(src.at(0,0));
            else if(i>=0 && j<0)
              ++histogram.at(src.at(i,0));
            else if(i<0 && j>=0)
              ++histogram.at(src.at(0,j));
            else //if(i>=0 && j>=0)
              ++histogram.at(src.at(i,j));
          }
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
      //runs right top corner (area 3)
      for(col=lastCol+1;col<columnSize;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              ++histogram.at(src.at(0,j));
            else if(i<0 && j>columnSize-1)
              ++histogram.at(src.at(0,columnSize-1));
            else if(i>=0 && j>columnSize-1)
              ++histogram.at(src.at(i,columnSize-1));
            else //if(i>=0 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
	  }
        dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
    }

    for(row=lastRow+1;row<=rowSize-1;++row) { //runs bottom rows (7,8,9)
      const int r=row+limit;
      //runs middle bottom rows (area 8)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              ++histogram.at(src.at(i,j));
            else // if(i>rowSize-1)
              ++histogram.at(src.at(rowSize-1,j));
          }
        dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
      //runs left bottom corner (area 7)
      for(col=0;col<limit;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              ++histogram.at(src.at(i,0));
            else if(i<=rowSize-1 && j>=0)
              ++histogram.at(src.at(i,j));
            else if(i>rowSize-1 && j<0)
              ++histogram.at(src.at(rowSize-1,0));
            else //if(i>rowSize-1 && j>=0)
              ++histogram.at(src.at(rowSize-1,j));
          }
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
      //runs right bottom corner (area 9)
      for(col=lastCol+1;col<columnSize;++col){
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
            else if(i<=rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(i,columnSize-1));
            else if(i>rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(rowSize-1,j));
            else //if(i>rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(rowSize-1,columnSize-1));
          }
	dest.at(row,col)=getMostLabel(histogram,src,row,col);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary Periodic
  bool kNearestNeighFilter::histogramMethodPeriodic(const imatrix& src,
					   imatrix& dest) const {
    int i,j,row,col;//index
    ivector histogram(histoSize,0);

    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;

    dest.resize(src.size(),0,false,false);

    //runs through the src's columns area 4,5,6
    for(row=limit;row<=lastRow;++row) {
      histogram.fill(0);

      col=0;
      const int c = col+limit;
      const int r = row+limit;
      for(i=row-limit;i<=r;++i)
	for(j=col-limit;j<=c;++j) {
          if(j<0)
            ++histogram.at(src.at(i,j+columnSize));
          else // if(j>=0)
	    ++histogram.at(src.at(i,j));
        }
      dest.at(row,col)=getMostLabel(histogram,src,row,col);

      while(col < limit) {
	j = col-limit;
        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,j+columnSize));
	++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i)
	  ++histogram.at(src.at(i,j));

	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }

      //runs inside the image
      histogramMethodMiddle(src,dest,histogram,row,col);

      col=lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;
	for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,j));  // column of intensity,take off in the histogram
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i)
          ++histogram.at(src.at(i,j-columnSize));

	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }

    //runs top rows (area 1,2,3)
    for(row=0;row<limit;++row) {
      const int r=row+limit;
      for(col=limit;col<=lastCol;++col) {  // runs top middle rows (area 2)
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              ++histogram.at(src.at(i+rowSize,j));
            else //if(i>=0)
              ++histogram.at(src.at(i,j));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      for(col=0;col<limit;++col) {   // runs top left corner (area 1)
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              ++histogram.at(src.at(rowSize+i,columnSize+j));
            else if(i>=0 && j<0)
              ++histogram.at(src.at(i,columnSize+j));
            else if(i<0 && j>=0)
              ++histogram.at(src.at(rowSize+i,j));
            else //if(i>=0 && j>=0)
              ++histogram.at(src.at(i,j));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      for(col=lastCol+1;col<columnSize;++col) { //runs top right corner area 3
      	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              ++histogram.at(src.at(rowSize+i,j));
            else if(i<0 && j>columnSize-1)
              ++histogram.at(src.at(rowSize+i,j-columnSize));
            else if(i>=0 && j>columnSize-1)
              ++histogram.at(src.at(i,j-columnSize));
            else //if(i>=0 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
          }
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }

    //runs bottom rows (area 7,8,9)
    for(row=lastRow+1;row<=rowSize-1;++row) {
      const int r=row+limit;
      for(col=limit;col<=lastCol;++col) {       //runs middle botom rows
	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              ++histogram.at(src.at(i,j));
            else // if(i>rowSize-1)
              ++histogram.at(src.at(i-rowSize,j));
	  }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      for(col=0;col<limit;++col) {    //runs bottom left corner
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              ++histogram.at(src.at(i,columnSize+j));
            else if(i<=rowSize-1 && j>=0)
              ++histogram.at(src.at(i,j));
            else if(i>rowSize-1 && j<0)
              ++histogram.at(src.at(i-rowSize,columnSize+j));
            else //if(i>rowSize-1 && j>=0)
              ++histogram.at(src.at(i-rowSize,j));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      for(col=lastCol+1;col<columnSize;++col) { //runs bottom right corner
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
            else if(i<=rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(i,j-columnSize));
            else if(i>rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(i-rowSize,j));
            else //if(i>rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(i-rowSize,j-columnSize));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }
    return true;
  };


  // applies the histogramMethod for the type boundary Mirror
  bool kNearestNeighFilter::histogramMethodMirror(const imatrix& src,
					 imatrix& dest) const {
    int i,j,row,col;//index

    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;

    ivector histogram(histoSize,0);
    dest.resize(src.size(),0,false,false);

    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      histogram.fill(0);

      col=0;
      const int c=col+limit;
      const int r = row+limit;
      for(i=row-limit;i<=r;++i)
        for(j=col-limit;j<=c;++j) {
          if(j<0)
            ++histogram.at(src.at(i,-j-1));
          else // if(j>=0)
            ++histogram.at(src.at(i,j));
	}
      dest.at(row,col) = getMostLabel(histogram,src,row,col);

      while(col < limit) {
        j = col-limit;
        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,-j-1));
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i)
	  ++histogram.at(src.at(i,j));

	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      // runs inside the image
      histogramMethodMiddle(src,dest,histogram,row,col);

      col=lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;

        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,j));
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i)
          ++histogram.at(src.at(i,columnSize-1+(columnSize-j)));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }

    }

    // areas 1,2,3
    for(row=0;row<limit;++row) {
      const int r=row+limit;
      // runs middle top rows (area 2)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              ++histogram.at(src.at(-i-1,j));
            else //if(i>=0)
              ++histogram.at(src.at(i,j));
	  }
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      // runs top left corner (area 1)
      for(col=0;col<limit;++col) {
	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              ++histogram.at(src.at(-i-1,-j-1));
            else if(i>=0 && j<0)
              ++histogram.at(src.at(i,-j-1));
            else if(i<0 && j>=0)
              ++histogram.at(src.at(-i-1,j));
            else //if(i>=0 && j>=0)
              ++histogram.at(src.at(i,j));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      //runs top right corner (area 3)
      for(col=lastCol+1;col<columnSize;++col) {
	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              ++histogram.at(src.at(-i-1,j));
            else if(i<0 && j>columnSize-1)
              ++histogram.at(src.at(-i-1,columnSize-1+(columnSize-j)));
            else if(i>=0 && j>columnSize-1)
              ++histogram.at(src.at(i,columnSize-1+(columnSize-j)));
            else //if(i>=0 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
          }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }
    //runs bottom rows (areas 7,8,9)
    for(row=lastRow+1;row<=rowSize-1;++row) {
      const int r=row+limit;
      //runs middle bottom rows (area 8)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              ++histogram.at(src.at(i,j));
            else // if(i>rowSize-1)
              ++histogram.at(src.at(rowSize-1+(rowSize-i),j));
	  }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      //runs bottom left corner (area 7)
      for(col=0;col<limit;++col) {
	histogram.fill(0);
	const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              ++histogram.at(src.at(i,-j-1));
            else if(i<=rowSize-1 && j>=0)
              ++histogram.at(src.at(i,j));
            else if(i>rowSize-1 && j<0)
              ++histogram.at(src.at(rowSize-1+(rowSize-i),-j-1));
            else //if(i>rowSize-1 && j>=0)
              ++histogram.at(src.at(rowSize-1+(rowSize-i),j));
	  }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      //runs bottom right corner (area 9)
      for(col=lastCol+1;col<columnSize;++col) {
	histogram.fill(0);
        const int c=col+limit;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(i,j));
            else if(i<=rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(i,columnSize-1+(columnSize-j)));
            else if(i>rowSize-1 && j<=columnSize-1)
              ++histogram.at(src.at(rowSize-1+(rowSize-i),j));
            else //if(i>rowSize-1 && j>columnSize-1)
              ++histogram.at(src.at(rowSize-1+(rowSize-i),
				    columnSize-1+(columnSize-j)));
	  }
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary Zero
  bool kNearestNeighFilter::histogramMethodZero(const imatrix& src,
					imatrix& dest) const {
    int i,j,row,col;//index

    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;

    ivector histogram(histoSize,0);  //the histogram for a channel8
    dest.resize(src.size(),ubyte(0),false,false);

    //runs area 4,5,6
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(0);
      col=0;

      //number of 0's are known
      histogram.at(0) = sizeOfKernel*(sizeOfKernel-limit-1);
      for(i=row-limit;i<=r;++i)
        for(j=0;j<=limit;++j)
          ++histogram.at(src.at(i,j));
      dest.at(row,col) = getMostLabel(histogram,src,row,col);

      // the kernel at the position between the border and the image
      while(col < limit) {
        histogram.at(0) -= sizeOfKernel; // cut all the 0 in the leftmost column
	++col;
        j = col+limit;        // for each pixel in the rightmost column
        for(i=row-limit;i<=r;++i)
          ++histogram.at(src.at(i,j));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }

      //runs area 5
      histogramMethodMiddle(src,dest,histogram,row,col);

      // area 6
      col = lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;
        for(i=row-limit;i<=r;++i)
          --histogram.at(src.at(i,j));
        ++col;
        histogram.at(0) += sizeOfKernel;

	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }

    // runs area 1,2,3
    for(row=0;row<limit;++row) {
      const int r=row+limit;
      // runs middle top rows (area 2)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
        const int c=col+limit;
        histogram.at(0) = sizeOfKernel*(sizeOfKernel-(limit+1)-row);
	for(i=0;i<=r;++i)
          for(j=col-limit;j<=c;++j)
	    ++histogram.at(src.at(i,j));
        dest.at(row,col) = getMostLabel(histogram,src,row,col) ;
      }
      // runs left top corner (area 1);
      for(col=0;col<limit;++col) {
	histogram.fill(0);
	const int c=col+limit;
        histogram.at(0) = sizeOfKernel*sizeOfKernel-(limit+1)*(limit+1+row+col)-col*row;
        for(i=0;i<=r;++i)
	  for(j=0;j<=c;++j)
	    ++histogram.at(src.at(i,j));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      // runs top right corner (area 3)
      for(col=lastCol+1;col<=columnSize-1;++col) {
	histogram.fill(0);
	const int c=columnSize-1;
	histogram.at(0) = sizeOfKernel*sizeOfKernel
	  -(limit+1)*(limit+1+row+(columnSize-1-col))-row*(columnSize-1-col);
        for(i=0;i<=r;++i)
          for(j=col-limit;j<=c;++j)
            ++histogram.at(src.at(i,j));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }

    //runs the rows at the bottom (area 7,8,9)
    for(row=lastRow+1;row<=rowSize-1;++row) {
      //runs middle bottom rows (area 8)
      for(col=limit;col<=lastCol;++col) {
	histogram.fill(0);
        const int c=col+limit;
        histogram.at(0) = sizeOfKernel*(sizeOfKernel-(limit+1)-(rowSize-1-row));
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=col-limit;j<=c;++j)
            ++histogram.at(src.at(i,j));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      // runs bottom left corner (area 7)
      for(col=0;col<limit;++col) {
	histogram.fill(0);
	const int c=col+limit;
        histogram.at(0) = sizeOfKernel*sizeOfKernel-(limit+1)*
          (limit+1+(rowSize-1-row)+col)-col*(rowSize-1-row);
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=0;j<=c;++j)
            ++histogram.at(src.at(i,j));
	dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
      //runs bottom right corner (area 9)
      for(col=lastCol+1;col<=columnSize-1;++col) {
	histogram.fill(0);
	const int c = columnSize-1;
	histogram.at(0) = sizeOfKernel*sizeOfKernel
	  -(limit+1)*(limit+1+(rowSize-1-row)+(columnSize-1-col))
	  -(rowSize-1-row)*(columnSize-1-col);
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=col-limit;j<=c;++j)
            ++histogram.at(src.at(i,j));
        dest.at(row,col) = getMostLabel(histogram,src,row,col);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary NoBoundary
  bool kNearestNeighFilter::histogramMethodNoBoundary(const imatrix& src,
					     imatrix& dest) const {
    const int limit = sizeOfKernel/2; //half size of the kernel
    int i,j,row,col;

    ivector histogram(histoSize,0);
    dest.resize(src.size(),0,false,false);

    //runs through the src's columns,inside the image
    const int lastRow = src.lastRow()-limit;
    for(row=limit;row<=lastRow;++row) {
      histogram.fill(0);

      // first block per row
      const int r = row+limit;
      for(i=row-limit;i<=r;++i)
	for(j=0;j<sizeOfKernel;++j)
          ++histogram.at(src.at(i,j));
      dest.at(row,limit)=getMostLabel(histogram,src,row,limit);

      // runs inside the image
      histogramMethodMiddle(src,dest,histogram,row,col);
    }
    return true;
 };

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool kNearestNeighFilter::apply(channel8& srcdest) {
    imatrix tmpSrc,tmpDest;
    tmpSrc.castFrom(srcdest);
    apply(tmpSrc,tmpDest);
    srcdest.castFrom(tmpDest);
    return true;
  };

  bool kNearestNeighFilter::apply(imatrix& srcdest) {
    imatrix tmp;
    apply(srcdest,tmp);
    tmp.detach(srcdest);
    return true;
  };

  bool kNearestNeighFilter::apply(const channel8& src,channel8& dest) {
    imatrix tmpSrc,tmpDest;
    tmpSrc.castFrom(src);
    apply(tmpSrc,tmpDest);
    dest.castFrom(tmpDest);
    return true;
  }

  bool kNearestNeighFilter::apply(const imatrix& src,imatrix& dest) {

    if (src.empty()) {
      dest.clear();
      return true;
    }  

    const parameters& param = getParameters();
    if (param.kernelSize <= 1) {
      dest.copy(src);
      return true;
    }
    sizeOfKernel = param.kernelSize + ((param.kernelSize%2 == 0) ? 1 : 0);
    histoSize = src.maximum()+1;
    bool control = false;   // return variable

    // choose border behaviour
    switch(param.boundaryType) {
      case lti::Zero:
        control = histogramMethodZero(src,dest);
        break;
      case lti::Mirror:
        control = histogramMethodMirror(src,dest);
        break;
      case lti::Constant:
        control = histogramMethodConstant(src,dest);
        break;
      case lti::Periodic:
        control = histogramMethodPeriodic(src,dest);
        break;
      case lti::NoBoundary:
        control = histogramMethodNoBoundary(src,dest);
        break;
      default:
        setStatusString("Unknown boundary type");
        break;
    }

    return control;
  };


}
