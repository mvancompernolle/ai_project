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
 * file .......: ltiMedianFilter.cpp
 * authors ....: Guy Wafo Moudhe
 * organization: LTI, RWTH Aachen
 * creation ...: 16.3.2001
 * revisions ..: $Id: ltiMedianFilter.cpp,v 1.13 2006/09/05 10:22:21 ltilib Exp $
 */

#include "ltiMedianFilter.h"

namespace lti {
  // --------------------------------------------------
  // medianFilter::parameters
  // --------------------------------------------------

  // default constructor,set the default value of the size of the kernel,
  // the parity of the value should always be even,
  // if the kernelSize is not even,its value is increment with 1,
  // this happen in the function histogramMethod
  medianFilter::parameters::parameters()
    : filter::parameters() {
    // TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    kernelSize = int(3);
  }

  // copy constructor
  medianFilter::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    copy(other);
  }

  // destructor
  medianFilter::parameters::~parameters() {
  }

  // get type name
  const char* medianFilter::parameters::getTypeName() const {
    return "medianFilter::parameters";
  }

  // copy member

  medianFilter::parameters&
    medianFilter::parameters::copy(const parameters& other) {
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
  medianFilter::parameters&
    medianFilter::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* medianFilter::parameters::clone() const {
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

  bool medianFilter::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool medianFilter::parameters::writeMS(ioHandler& handler,
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
  bool medianFilter::parameters::write(ioHandler& handler,
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
  bool medianFilter::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool medianFilter::parameters::readMS(ioHandler& handler,
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
  bool medianFilter::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // medianFilter
  // --------------------------------------------------

  // default constructor
  medianFilter::medianFilter()
    : filter(){
    parameters defaultParameters;
    setParameters(defaultParameters);
  }

  // default constructor
  medianFilter::medianFilter(const parameters& par)
    : filter(){
    setParameters(par);
  }

  // copy constructor
  medianFilter::medianFilter(const medianFilter& other)
    : filter()  {
    copy(other);
  }

  // destructor
  medianFilter::~medianFilter() {
  }

  // returns the name of this type
  const char* medianFilter::getTypeName() const {
    return "medianFilter";
  }

  // copy member
  medianFilter& medianFilter::copy(const medianFilter& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* medianFilter::clone() const {
    return new medianFilter(*this);
  }

  // return parameters
  const medianFilter::parameters&
    medianFilter::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // shortcut for setting kernel size
  void medianFilter::setKernelSize(const int& newKernelSize) {
    parameters param(getParameters());
    param.kernelSize = newKernelSize;
    setParameters(param);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel!
  bool medianFilter::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type channel8!
  bool medianFilter::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest); // faster than srcdest.copy(tmp);
      return true;
    }
    return false;
  };

  // filters src with the median filter and gives the result to dest
  bool medianFilter::apply(const channel& src,channel& dest) const {
    return realMedian(src,dest);
  }

  // On copy apply for type channel8!
  bool medianFilter::apply(const channel8& src,channel8& dest) const {
    return histogramMethod(src,dest);
  };

  // the kernel runs inside the image
  void medianFilter::histogramMethodMiddle(const channel8& src,
                                           channel8& dest,
					   ivector& histogram,
                                           int& lessThanMedian,
                                           const int& row,int& col,
                                           ubyte& median,
                                           const int& sizeOfKernel) const {
    int i,j;//index
    int tmp;
    ubyte value;
    const int limit = sizeOfKernel/2;   //half size of the kernel
    const int halfKernelSize = (sizeOfKernel*sizeOfKernel)/2;
    const int lastCol = src.lastColumn()-limit;
    const int r = row+limit;
    col = limit;
    while(col <= (lastCol-1)) {
      j = col-limit;
      // 3. Step
      for(i=row-limit;i<=r;++i) {
        value=src[i][j];       // for each pixel in the leftmost
        --histogram[value];    // column of intensity,take off in the histogram
        if(value < median)    // the number of the corresponding pixels
          --lessThanMedian;
      }
      // 4. Step:
      ++col;
      j = col+limit;               // for each pixel in the rightmost column
      for(i=row-limit;i<=r;++i) {  // of intensity,add in the histogram
        value=src[i][j];         // the number of the corresponding pixels
        ++histogram[value];
        if(value < median)
          ++lessThanMedian;
      }
      if (lessThanMedian > halfKernelSize) {
        // 6. Step
        do {
          --median;
          lessThanMedian -= histogram[median];
        } while(lessThanMedian > halfKernelSize);
        dest.at(row,col) = median;
      }
      else { //if(lessThanMedian<=halfKernelSize) {
        // 5. Step
        tmp = lessThanMedian + histogram[median];
        while (tmp <= halfKernelSize) {
          lessThanMedian = tmp;
          ++median;
          tmp+=histogram[median];
        }
        dest.at(row,col) = median;
      }
    }

  };

   // applies the histogramMethod for the type boundary Constant
  bool medianFilter::histogramMethodConstant(const channel8& src,
                                             channel8& dest,
                                             const int& sizeOfKernel) const {
    const int limit = sizeOfKernel/2;   //half size of the kernel
        // Median Filter
    int i,j,row,col,z;//index
    int tmp;
    int lessThanMedian = 0;
    ubyte median;
    lti::quickMedian<ubyte> quickmedia;

    ivector histogram(256,int(0));  //the histogram for a channel8
    vector<ubyte> vect(sizeOfKernel*sizeOfKernel,ubyte(0));
    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int halfKernelSize = (sizeOfKernel*sizeOfKernel)/2;
    //resizes the matrix destination
    dest.resize(src.rows(),src.columns(),ubyte(0),false,false);

    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;

    ubyte value;
    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(int(0));

      col=0;
      z=0;
      const int c=col+limit;
      for(i=row-limit;i<=r;++i)
        for(j=col-limit;j<=c;++j) {
	  if(j<0)
            value=src[i][0];
	  else// if(j>=0)
            value=src[i][j];
	  vect.at(z)=value;
	  ++histogram[value];
	  ++z;
        }

      median = quickmedia.apply(vect);
      dest.at(row,col)=median;

      lessThanMedian=0;
      for(i=0;i<vect.size();++i)
        if(vect[i] < median)
          ++lessThanMedian;

      while(col < limit) {
        for(i=row-limit;i<=r;++i) {
          value = src[i][0];
          --histogram[value];
          if(value < median)
            --lessThanMedian;
	}
        ++col;
        j=col+limit;

        for(i=row-limit;i<=r;++i) {
          value=src[i][j];
          ++histogram[value];
          if(value < median) {
            ++lessThanMedian;
          }
        }
        if (lessThanMedian>halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
              // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col)=median;
        }
      }
      // runs inside the image
      histogramMethodMiddle(src, dest,histogram,lessThanMedian,
                            row,col,median,sizeOfKernel);
      col=lastCol;
      while(col < (columnSize-1)) {
        j=col-limit;
        for(i=row-limit;i<=r;++i) {
          value=src[i][j];
          --histogram[value];
          if(value < median)
            --lessThanMedian;
        }
        ++col;

        for(i=row-limit;i<=r;++i) {
          value=src[i][columnSize-1];
          ++histogram[value];
          if(value < median)
            ++lessThanMedian;
        }
        if (lessThanMedian > halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian -= histogram[median];
          } while(lessThanMedian > halfKernelSize);
          dest.at(row,col) = median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col) = median;
        }
      }
    }
    for(row=0;row<limit;++row) {     //runs the rows at the top
      const int r=row+limit;
      // runs the rows at the top in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              vect.at(z)=src[0][j];
	    else//if(i>=0) {
              vect.at(z)=src[i][j];
	    ++z;
          }
	dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {   // runs at the top,left in the corner
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              vect.at(z)=src[0][0];
            else if(i>=0 && j<0)
              vect.at(z)=src[i][0];
            else if(i<0 && j>=0)
              vect.at(z)=src[0][j];
            else //if(i>=0 && j>=0)
              vect.at(z)=src[i][j];
	    ++z;
          }
	dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the top,right in the corner
      for(col=lastCol+1;col<columnSize;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              vect.at(z)=src[0][j];
            else if(i<0 && j>columnSize-1)
              vect.at(z)=src[0][columnSize-1];
            else if(i>=0 && j>columnSize-1)
              vect.at(z)=src[i][columnSize-1];
            else //if(i>=0 && j<=columnSize-1)
              vect.at(z)=src[i][j];
	    ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    for(row=lastRow+1;row<=rowSize-1;++row) {  //runs the rows at the bottom
      const int r=row+limit;
      //runs the rows at the bottom in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              vect.at(z)=src[i][j];
            else // if(i>rowSize-1)
              vect.at(z)=src[rowSize-1][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the bottom,left in the corner
      for(col=0;col<limit;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              vect.at(z)=src[i][0];
            else if(i<=rowSize-1 && j>=0)
              vect.at(z)=src[i][j];
            else if(i>rowSize-1 && j<0)
              vect.at(z)=src[rowSize-1][0];
            else //if(i>rowSize-1 && j>=0)
              vect.at(z)=src[rowSize-1][j];
	    ++z;
          }
	dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the bottom,right in the corner
      for(col=lastCol+1;col<columnSize;++col){
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[i][j];
            else if(i<=rowSize-1 && j>columnSize-1)
              vect.at(z)=src[i][columnSize-1];
            else if(i>rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[rowSize-1][j];
            else //if(i>rowSize-1 && j>columnSize-1)
              vect.at(z)=src[rowSize-1][columnSize-1];
	    ++z;
          }
	dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary Periodic
  bool medianFilter::histogramMethodPeriodic(const channel8& src,
                                             channel8& dest,
                                             const int& sizeOfKernel) const {
    const int limit = sizeOfKernel/2;   //half size of the kernel
    int i,j,row,col,z;//index
    int tmp;
    int lessThanMedian = 0;
    ubyte median;
    lti::quickMedian<ubyte> quickmedia;

    vector<ubyte> vect(sizeOfKernel*sizeOfKernel,ubyte(0));
    ivector histogram(256,int(0));  //the histogram for a channel8

    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int halfKernelSize = (sizeOfKernel*sizeOfKernel)/2;
    //resizes the matrix destination
    dest.resize(src.rows(),src.columns(),ubyte(0),false,false);

    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;

    ubyte value;
    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(int(0));

      z=0;
      col=0;
      const int c=col+limit;
      for(i=row-limit;i<=r;++i)
	for(j=col-limit;j<=c;++j) {
          if(j<0)
            value=src[i][j+columnSize];
          else // if(j>=0)
            value=src[i][j];
	  vect.at(z)=value;
	  ++histogram[value];
	  ++z;
        }
      median=quickmedia.apply(vect);
      dest.at(row,col)=median;

      lessThanMedian = 0;
      for(i=0;i<vect.size();++i)   // count the number of pixels with intensity
        if(vect[i] < median)       // can be optimized for Zero boundary type
          ++lessThanMedian;        // less than or equal to median

      while(col < limit) {
	j = col-limit;
        for(i=row-limit;i<=r;++i){
          value=src[i][j+columnSize];
          --histogram[value];
          if(value < median)
            --lessThanMedian;
        }
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i) {
          value=src[i][j];
          ++histogram[value];
          if(value < median)
            ++lessThanMedian;
        }
        if (lessThanMedian > halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col) = median;
        }
      }
      //runs inside the image
      histogramMethodMiddle(src,dest,histogram,lessThanMedian,
			    row,col,median,sizeOfKernel);
      col=lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;

        for(i=row-limit;i<=r;++i) {
          value=src[i][j];     // for each pixel in the leftmost
          --histogram[value];  // column of intensity,take off in the histogram
          if(value < median) // the number of the corresponding pixels
            --lessThanMedian;
	}
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i) {
          value = src[i][j-columnSize];
          ++histogram[value];
          if(value < median)
            ++lessThanMedian;
	}
        if (lessThanMedian>halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
              // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col)=median;
        }
      }
    }
    for(row=0;row<limit;++row) {          //runs the rows at the top
      const int r=row+limit;
      // runs the rows at the top in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              vect.at(z)=src[i+rowSize][j];
            else //if(i>=0)
              vect.at(z)=src[i][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {       // runs at the top,left in the corner
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              vect.at(z)=src[rowSize+i][columnSize+j];
            else if(i>=0 && j<0)
              vect.at(z)=src[i][columnSize+j];
            else if(i<0 && j>=0)
              vect.at(z)=src[rowSize+i][j];
            else //if(i>=0 && j>=0)
              vect.at(z)=src[i][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the top,right in the corner
      for(col=lastCol+1;col<columnSize;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              vect.at(z)=src[rowSize+i][j];
            else if(i<0 && j>columnSize-1)
              vect.at(z)=src[rowSize+i][j-columnSize];
            else if(i>=0 && j>columnSize-1)
              vect.at(z)=src[i][j-columnSize];
            else //if(i>=0 && j<=columnSize-1)
              vect.at(z)=src[i][j];
	    ++z;
          }
	dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    for(row=lastRow+1;row<=rowSize-1;++row) {  //runs the rows at the bottom
      const int r=row+limit;
      //runs the rows at the bottom in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              vect.at(z)=src[i][j];
            else // if(i>rowSize-1)
              vect.at(z)=src[i-rowSize][j];
	    ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {    //runs at the bottom,left in the corner
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              vect.at(z)=src[i][columnSize+j];
            else if(i<=rowSize-1 && j>=0)
              vect.at(z)=src[i][j];
            else if(i>rowSize-1 && j<0)
              vect.at(z)=src[i-rowSize][columnSize+j];
            else //if(i>rowSize-1 && j>=0)
              vect.at(z)=src[i-rowSize][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the bottom,right in the corner
      for(col=lastCol+1;col<columnSize;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[i][j];
            else if(i<=rowSize-1 && j>columnSize-1)
              vect.at(z)=src[i][j-columnSize];
            else if(i>rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[i-rowSize][j];
            else //if(i>rowSize-1 && j>columnSize-1)
              vect.at(z)=src[i-rowSize][j-columnSize];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary Mirror
  bool medianFilter::histogramMethodMirror(const channel8& src,
                                           channel8& dest,
                                           const int& sizeOfKernel) const {
    const int limit = sizeOfKernel/2;   //half size of the kernel

    // Median Filter
    int i,j,row,col,z;//index
    int tmp;
    int lessThanMedian = 0;
    ubyte median;
    lti::quickMedian<ubyte> quickmedia;

    vector<ubyte> vect(sizeOfKernel*sizeOfKernel,ubyte(0));
    ivector histogram(256,int(0));  //the histogram for a channel8

    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int halfKernelSize = (sizeOfKernel*sizeOfKernel)/2;
    //resizes the matrix destination
    dest.resize(src.rows(),src.columns(),ubyte(0),false,false);

    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;
    ubyte value;

    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(int(0));

      z=0;
      col=0;
      const int c=col+limit;
      for(i=row-limit;i<=r;++i)
	for(j=col-limit;j<=c;++j) {
          if(j<0)
            value=src[i][-j-1];
          else // if(j>=0)
            value=src[i][j];
	  vect.at(z)=value;
	  ++histogram[value];
	  ++z;
	}
      median=quickmedia.apply(vect);
      dest.at(row,col)=median;

      lessThanMedian = 0;
      for(i=0;i<vect.size();++i)  // count the number of pixels with intensity
        if(vect[i] < median)     // less than median
          ++lessThanMedian;

      while(col < limit) {
        j = col-limit;
        for(i=row-limit;i<=r;++i) {
          value=src[i][-j-1];
          --histogram[value];
          if(value < median)
            --lessThanMedian;
        }
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i) {
          value=src[i][j];
          ++histogram[value];
          if(value < median)
            ++lessThanMedian;
        }
        if (lessThanMedian > halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col)=median;
        }
      }
      // runs inside the image
      histogramMethodMiddle(src,dest,histogram,lessThanMedian,
                            row,col,median,sizeOfKernel);
      col=lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;

        for(i=row-limit;i<=r;++i) {
          value=src[i][j];     // for each pixel in the leftmost
          --histogram[value];  // column of intensity,take off in the histogram
	  if(value < median)   // the number of the corresponding pixels
            --lessThanMedian;
        }
        ++col;
        j=col+limit;
        for(i=row-limit;i<=r;++i) {
          value = src[i][columnSize-1+(columnSize-j)];
          ++histogram[value];
          if(value < median)
            ++lessThanMedian;
        }
        if (lessThanMedian>halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col)=median;
        }
      }

    }
    for(row=0;row<limit;++row) {      //runs the rows at the top
      const int r=row+limit;          // runs the rows at the top in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
	    if(i<0)
              vect.at(z)=src[-i-1][j];
            else //if(i>=0)
              vect.at(z)=src[i][j];
	    ++z;
	  }
	dest.at(row,col)=quickmedia.apply(vect);
      }

      for(col=0;col<limit;++col) {      // runs at the top,left in the corner
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
	  for(j=col-limit;j<=c;++j) {
            if(i<0 && j<0)
              vect.at(z)=src[-i-1][-j-1];
            else if(i>=0 && j<0)
              vect.at(z)=src[i][-j-1];
            else if(i<0 && j>=0)
              vect.at(z)=src[-i-1][j];
            else //if(i>=0 && j>=0)
              vect.at(z)=src[i][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the top,right in the corner
      for(col=lastCol+1;col<columnSize;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<0 && j<=columnSize-1)
              vect.at(z)=src[-i-1][j];
            else if(i<0 && j>columnSize-1)
              vect.at(z)=src[-i-1][columnSize-1+(columnSize-j)];
            else if(i>=0 && j>columnSize-1)
              vect.at(z)=src[i][columnSize-1+(columnSize-j)];
            else //if(i>=0 && j<=columnSize-1)
              vect.at(z)=src[i][j];
	    ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    for(row=lastRow+1;row<=rowSize-1;++row) {  //runs the rows at the bottom
      const int r=row+limit;
      //runs the rows at the bottom in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1)
              vect.at(z)=src[i][j];
            else // if(i>rowSize-1)
              vect.at(z)=src[rowSize-1+(rowSize-i)][j];
	    ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {    //runs at the bottom,left in the corner
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<0)
              vect.at(z)=src[i][-j-1];
            else if(i<=rowSize-1 && j>=0)
              vect.at(z)=src[i][j];
            else if(i>rowSize-1 && j<0)
              vect.at(z)=src[rowSize-1+(rowSize-i)][-j-1];
            else //if(i>rowSize-1 && j>=0)
              vect.at(z)=src[rowSize-1+(rowSize-i)][j];
	    ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the bottom,right in the corner
      for(col=lastCol+1;col<columnSize;++col) {
        const int c=col+limit;
        z=0;
        for(i=row-limit;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
            if(i<=rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[i][j];
            else if(i<=rowSize-1 && j>columnSize-1)
              vect.at(z)=src[i][columnSize-1+(columnSize-j)];
            else if(i>rowSize-1 && j<=columnSize-1)
              vect.at(z)=src[rowSize-1+(rowSize-i)][j];
            else //if(i>rowSize-1 && j>columnSize-1)
              vect.at(z)
                =src[rowSize-1+(rowSize-i)][columnSize-1+(columnSize-j)];
	    ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary Zero
  bool medianFilter::histogramMethodZero(const channel8& src,
                                          channel8& dest,
                                          const int& sizeOfKernel) const {
    const int limit = sizeOfKernel/2;   //half size of the kernel
        // Median Filter
    int i,j,row,col,z;//index
    int tmp;
    int lessThanMedian = 0;
    ubyte median;
    lti::quickMedian<ubyte> quickmedia;

    vector<ubyte> vect(sizeOfKernel*sizeOfKernel,ubyte(0));
    ivector histogram(256,int(0));  //the histogram for a channel8

    const int rowSize = src.rows();
    const int columnSize = src.columns();
    const int halfKernelSize = (sizeOfKernel*sizeOfKernel)/2;
    //resizes the matrix destination
    dest.resize(src.rows(),src.columns(),ubyte(0),false,false);

    const int lastCol = src.lastColumn()-limit;
    const int lastRow = src.lastRow()-limit;
    ubyte value;

    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(int(0));
      col=0;

      z = sizeOfKernel*(sizeOfKernel-limit-1);
      vect.fill(ubyte(0),0,z-1);
      histogram[0] = z;  //the number of 0's are known

      for(i=row-limit;i<=r;++i)  // fills the vector to calculate the median
                                 // and fills the histogram
        for(j=0;j<=limit;++j) {
          value=src[i][j];
          vect.at(z)=value;
          ++z;
          ++histogram[value];
        }
      median = quickmedia.apply(vect);    //here is the median
      dest.at(row,0) = median;

      lessThanMedian = 0;
      for(i=0;i<vect.size();++i)  // count the number of pixels with intensity
        if(vect[i] < median)      // less than or equal to median
          ++lessThanMedian;       // this can be optimized for Zero boundary type

      //  col=0;
      // the kernel at the position between the border and the image
      while(col < limit) {
        histogram[0] -= sizeOfKernel;   // cut all the 0 in the leftmost column
                                // because 0 is the smallest value
        if(median != 0) {
          lessThanMedian -= sizeOfKernel;
        }
        ++col;
        j = col+limit;              // for each pixel in the rightmost column
        for(i=row-limit;i<=r;++i) { // of intensity,add the value
                                    //in the histogram
          value=src[i][j];       // and increment the number of pixels less
          ++histogram[value];    // than the median
          if(value < median)
            ++lessThanMedian;
        }
        if (lessThanMedian > halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian-=histogram[median];
          } while(lessThanMedian>halfKernelSize);
          dest.at(row,col)=median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col)=median;
        }
      }
      //runs inside the image
      histogramMethodMiddle(src,dest,histogram,lessThanMedian,
                            row,col,median,sizeOfKernel);
      col = lastCol;
      while(col < (columnSize-1)) {
        j = col-limit;
        for(i=row-limit;i<=r;++i) {
          value=src[i][j];     // for each pixel in the leftmost
          --histogram[value];  // column of intensity,take off in the histogram
          if(value < median) { // the number of the corresponding pixels
            --lessThanMedian;
          }
        }
        ++col;
        histogram[0] += sizeOfKernel;

        if(median != 0) {
          lessThanMedian += sizeOfKernel;
        }
        if (lessThanMedian > halfKernelSize) {
          // 6. Step
          do {
            --median;
            lessThanMedian -= histogram[median];
          } while(lessThanMedian > halfKernelSize);
          dest.at(row,col) = median;
        }
        else { //if(lessThanMedian<=halfKernelSize) {
          // 5. Step
          tmp = lessThanMedian + histogram[median];
          while (tmp <= halfKernelSize) {
            lessThanMedian = tmp;
            ++median;
            tmp+=histogram[median];
          }
          dest.at(row,col) = median;
        }
      }
    }
    for(row=0;row<limit;++row) {  //runs the rows at the top
      z=sizeOfKernel*(sizeOfKernel-limit-1);
      vect.fill(ubyte(0),0,z-1);
      const int r=row+limit;
      // runs the rows at the top in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=sizeOfKernel*(sizeOfKernel-(limit+1)-row);

        for(i=0;i<=r;++i)
          for(j=col-limit;j<=c;++j) {
	    vect.at(z)=src[i][j];
            ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {    // runs at the top,left in the corner
        const int c=col+limit;
        z=sizeOfKernel*sizeOfKernel-(limit+1)*(limit+1+row+col)-col*row;
        vect.fill(ubyte(0),0,z-1);
        for(i=0;i<=r;++i)
	  for(j=0;j<=c;++j) {
	    vect.at(z)=src[i][j];
            ++z;
	  }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the top,right in the corner
      for(col=lastCol+1;col<=columnSize-1;++col) {
        z=sizeOfKernel*sizeOfKernel-(limit+1)*(limit+1+row+(columnSize-1-col))-
          row*(columnSize-1-col);
        vect.fill(ubyte(0),0,z-1);
        for(i=0;i<=r;++i) {
          for(j=col-limit;j<=columnSize-1;++j) {
            vect.at(z)=src[i][j];
            ++z;
          }
        }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    for(row=lastRow+1;row<=rowSize-1;++row) {  //runs the rows at the bottom
      z=sizeOfKernel*(sizeOfKernel-limit-1);
      vect.fill(ubyte(0),0,z-1);
      //runs the rows at the bottom in the middle
      for(col=limit;col<=lastCol;++col) {
        const int c=col+limit;
        z=sizeOfKernel*(sizeOfKernel-(limit+1)-(rowSize-1-row));
        vect.fill(ubyte(0),0,z-1);
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=col-limit;j<=c;++j) {
            vect.at(z)=src[i][j];
            ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      for(col=0;col<limit;++col) {   //runs at the bottom,left in the corner
        const int c=col+limit;
        z=sizeOfKernel*sizeOfKernel-(limit+1)*
          (limit+1+(rowSize-1-row)+col)-col*(rowSize-1-row);
        vect.fill(ubyte(0),0,z-1);
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=0;j<=c;++j) {
            vect.at(z)=src[i][j];
            ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
      //runs at the bottom,right in the corner
      for(col=lastCol+1;col<=columnSize-1;++col) {
        z=sizeOfKernel*sizeOfKernel-(limit+1)*(limit+1+(rowSize-1-row)+
           (columnSize-1-col))-
          (rowSize-1-row)*(columnSize-1-col);
        vect.fill(ubyte(0),0,z-1);
        for(i=row-limit;i<=rowSize-1;++i)
          for(j=col-limit;j<=columnSize-1;++j) {
            vect.at(z)=src[i][j];
            ++z;
          }
        dest.at(row,col)=quickmedia.apply(vect);
      }
    }
    return true;
  };

  // applies the histogramMethod for the type boundary NoBoundary
  bool medianFilter::histogramMethodNoBoundary(const channel8& src,
					       channel8& dest,
					       const int& sizeOfKernel) const {
    const int limit = sizeOfKernel/2;   //half size of the kernel
    int i,j,row,col,z;
    int lessThanMedian = 0;
    ubyte median;
    lti::quickMedian<ubyte> quickmedia;

    vector<ubyte> vect(sizeOfKernel*sizeOfKernel,ubyte(0));
    ivector histogram(256,int(0));   // the histogram for a channel8

    const int lastRow = src.lastRow()-limit;

    //resizes the destination matrix
    dest.resize(src.rows(),src.columns(),ubyte(0),false,false);
    ubyte value;
    //runs through the src's columns,inside the image
    for(row=limit;row<=lastRow;++row) {
      const int r = row+limit;
      histogram.fill(int(0));

      z=0;
      for(i=row-limit;i<=r;++i)
	for(j=0;j<sizeOfKernel;++j) {    // determinate the median
                                         // at the beginning
          value = src[i][j];
          vect.at(z) = value; // of each rows and fill the histogram
          ++z;
          ++histogram[value];
        }
      median = quickmedia.apply(vect);
      dest.at(row,limit) = median;

      lessThanMedian=0;
      for(i=0;i<vect.size();++i) // count the number of pixels with intensity
        if(vect[i] < median)     // less than or equal to median
          ++lessThanMedian;

      // runs inside the image
      histogramMethodMiddle(src,dest,histogram,lessThanMedian,
			    row,col,median,sizeOfKernel);
    }
    return true;
 };

  // apply especially with a histogram,only for type channel8
  bool medianFilter::histogramMethod(const channel8& src,
                                     channel8& dest) const {
      
    if (src.empty()) {
      dest.clear();
      return true;
    }

    const parameters& param = getParameters();

    if (param.kernelSize <= 1) {
      dest.copy(src);
      return true;
    }

    const int sizeOfKernel = param.kernelSize + ((param.kernelSize%2 == 0) ? 1:0);
    bool control = false;   // return variable

    // 2. Step
    switch(param.boundaryType) {
      case lti::Zero:
        control = histogramMethodZero(src,dest,sizeOfKernel);
        break;
      case lti::Mirror:
        control = histogramMethodMirror(src,dest,sizeOfKernel);
        break;
      case lti::Constant:
        control = histogramMethodConstant(src,dest,sizeOfKernel);
        break;
      case lti::Periodic:
        control = histogramMethodPeriodic(src,dest,sizeOfKernel);
        break;
      case lti::NoBoundary:
        control = histogramMethodNoBoundary(src,dest,sizeOfKernel);
        break;
      default:
        setStatusString("Unknown boundary type");
        break;
    }
    return control;
  };

}
