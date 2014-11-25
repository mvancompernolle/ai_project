/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiImage.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 10.07.00
 * revisions ..: $Id: ltiImage.cpp,v 1.6 2006/02/08 11:18:34 ltilib Exp $
 */

#include "ltiImage.h"

namespace lti {
  // --------------------------------------------------------------------------
  //                             IMAGE
  // --------------------------------------------------------------------------

  /*
   * The one and only RGB-image format.
   *
   * This class is just a matrix of lti::rgbPixel
   */

  /*
   * default constructor creates an empty image
   */
  image::image() : matrix<rgbPixel>() {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> image
   * and initializes all elements with <code>iniValue</code>
   * @param rows number of rows of the image
   * @param cols number of columns of the image
   * @param iniValue all elements will be initialized with this value
   */
  image::image(const int& rows,const int& cols,
               const rgbPixel& iniValue)
    : matrix<rgbPixel>(rows,cols,iniValue) {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> image
   * and initializes all elements with the data pointed by
   * <code>data</code>.  The first <code>cols</code>-elements of the data
   * will be copied on the first row, the next ones on the second row and
   * so on.
   * @param rows number of rows of the image
   * @param cols number of columns of the image
   * @param data pointer to the memory block with the data to be initialized
   * with.
   */
  image::image(const int& rows,const int& cols,const rgbPixel data[])
    : matrix<rgbPixel>(rows,cols,data) {
  };

  /*
   * this constructor creates a connected <code>dim.y x dim.x</code>
   * image and initializes all elements with <code>iniValue</code>
   * @param dim lti::point with the size of the image
   *            (dim.x is the number of columns and dim.y the number of rows)
   * @param iniValue all elements will be initialized with this value
   */
  image::image(const point& size,const rgbPixel& iniValue)
      : matrix<rgbPixel>(size,iniValue) {
  };

  /*
   * copy constructor.
   *
   * create this image as a connected copy of another image
   * for this const version, the data will be always copied!
   * It is also possible to create a copy of a subimage of another image.
   *
   * @param other   the image to be copied.
   * @param fromRow initial row of the other image to be copied
   * @param toRow   last row to be copied of the other image
   * @param fromCol initial column of the other image to be copied
   * @param fromRow last column to be copied of the other image
   */
  image::image(const image& other,
               const int& fromRow,const int& toRow,
               const int& fromCol,const int& toCol)
      : matrix<rgbPixel>(other,fromRow,toRow,fromCol,toCol) {
  };

  /*
   * copy constructor (reference to a subimage).
   *
   * creates subimage of another image.
   *
   * if <code>copyData == true</code>, the new object has its own data
   * (equivalent to previous copy constructor).
   *
   * if <code>copyData == false</code>, the new object has references to
   * the other image, which means that the data is not necessarily
   * consecutive.  (This will not be a connected but a lined image)
   *
   * Those algorithms which use direct access to the image memory block
   * should check first if the memory lies in a consecutive block!
   * (see getMode())
   *
   * @param copyData should the data of the other image be copied or not
   * @param fromRow initial row of the other image to be copied
   * @param toRow last row to be copied of the other image
   * @param fromCol initial column of the other image to be copied
   * @param fromRow last column to be copied of the other image
   */
  image::image(const bool& copyData, image& other,
               const int& fromRow,const int& toRow,
               const int& fromCol,const int& toCol)
    : matrix<rgbPixel>(copyData,other,fromRow,toRow,fromCol,toCol) {
  };

  image::image(const bool& init, const int& rows, const int& cols)
    : matrix<rgbPixel>(init,rows,cols) {
  };

  image::image(const bool& init, const point& size)
    : matrix<rgbPixel>(init,size) {
  };

  /*
   * create a clone of this image
   * @return a pointer to a copy of this matrix
   */
  mathObject* image::clone() const {
    return new image(*this);
  };

  /*
   * return the name of this type
   */
  const char* image::getTypeName() const {
    return "image";
  };

  /*
   * cast from the <code>other</code> channel8.
   * For the transformation it assumes the channel8 as a gray valued
   * channel where 0 means black and 255 means white.
   *
   * @param other the channel8 to be casted
   * @return a reference to this image
   * Example:
   * \code
   *   lti::channel8 matA(10,10,255); // a channel8
   *   lti::image  matB;              // an image
   *
   *   matB.castFrom(matA);         // this will copy matA in matB!!
   *                                // and all elements will have
   *                                // rgbPixel(255,255,255)
   * \endcode
   */
  image& image::castFrom(const channel8& other) {
    resize(other.size(),rgbPixel(),false,false);
    if (other.empty()) {
      return *this;
    }
    iterator it;
    vector<ubyte>::const_iterator cit,eit;
    int y;
    it = begin();
    for (y=0;y<other.rows();++y) {
      cit = other.getRow(y).begin();
      eit = other.getRow(y).end();
      for (;cit!=eit;++cit,++it) {
        (*it).set(*cit,*cit,*cit);
      }
    }

    return *this;
  }

  /*
   * cast from the <code>other</code> channel.
   * For the transformation it assumes the channel as a gray valued
   * channel where 0 means black and 1.0f means white.  All other
   * values will be clipped (less than zero to zero and more than 1.0 to 1.0)
   *
   * @param other the channel8 to be casted
   * @return a reference to this image
   * Example:
   * \code
   *   lti::channel matA(10,10,1.0f); // a channel
   *   lti::image  matB;             // an image
   *
   *   matB.castFrom(matA);         // this will copy matA in matB!!
   *                                // and all elements will have
   *                                // rgbPixel(255,255,255)
   * \endcode
   */
  image& image::castFrom(const channel& other,
                         const bool minToBlack,
                         const bool maxToWhite) {
    
    resize(other.size(),rgbPixel(),false,false);
    if (other.empty()) {
      return *this;
    }
    iterator it;
    vector<float>::const_iterator cit,eit;
    int y;
    ubyte c;
    it = begin();
    if (!minToBlack && !maxToWhite) {
      for (y=0;y<other.rows();++y) {
        cit = other.getRow(y).begin();
        eit = other.getRow(y).end();
        for (;cit!=eit;++cit,++it) {
          c = ((*cit) <= 0) ? 0 : (((*cit) >= 1.0f) ?
                                   255 : static_cast<ubyte>((*cit)*255.0f));
          (*it).set(c,c,c);
        }
      }
    } else {

      float theMin,theMax;
      if (minToBlack && maxToWhite) {
        other.getExtremes(theMin,theMax);
      } else if (minToBlack) {
        theMin = other.minimum();
        theMax = 1.0f;
      } else {
        theMin = 0.0f;
        theMax = other.maximum();
      }

      float tm,tb;

      if (theMax == theMin) {
        tm = 0.0f;
        tb = 127.0f;
      } else {
        tm = 255.0f/(theMax-theMin);
        tb = -tm*theMin+0.5f;
      }

      const float m=tm;
      const float b=tb;

      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          c = static_cast<ubyte>((*cit)*m+b);
          (*it).set(c,c,c);
        }
      }
    }

    return *this;
  }


  // --------------------------------------------------------------------------
  //                    CHANNEL
  // --------------------------------------------------------------------------


  /*
   * a format for float channels.
   *
   * This class is identical to a matrix of floats except for the method
   * castFrom(channel8)
   */

  /*
   * default constructor creates an empty channel
   */
  channel::channel()
    : matrix<float>() {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> channel
   * and initializes all elements with <code>iniValue</code>
   * @param rows number of rows of the channel
   * @param cols number of columns of the channel
   * @param iniValue all elements will be initialized with this value
   */
  channel::channel(const int& rows,const int& cols,
                   const float& iniValue)
      : matrix<float>(rows,cols,iniValue) {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> channel
   * and initializes all elements with the data pointed by
   * <code>data</code>.  The first <code>cols</code>-elements of the data
   * will be copied on the first row, the next ones on the second row and
   * so on.
   * @param rows number of rows of the channel
   * @param cols number of columns of the channel
   * @param data pointer to the memory block with the data to be initialized
   * with.
   */
  channel::channel(const int& rows,const int& cols,const float data[])
    : matrix<float>(rows,cols,data) {
  };

  /*
   * this constructor creates a connected <code>dim.y x dim.x</code>
   * channel and initializes all elements with <code>iniValue</code>
   * @param dim lti::point with the size of the channel
   *            (dim.x is the number of columns and dim.y the number of rows)
   * @param iniValue all elements will be initialized with this value
   */
  channel::channel(const point& size,const float& iniValue)
    : matrix<float>(size,iniValue) {
  };

  /*
   * copy constructor.
   *
   * create this channel as a connected copy of another channel
   * for this const version, the data will be always copied!
   * It is also possible to create a copy of a subchannel of another channel.
   *
   * @param other   the channel to be copied.
   * @param fromRow initial row of the other channel to be copied
   * @param toRow   last row to be copied of the other channel
   * @param fromCol initial column of the other channel to be copied
   * @param fromRow last column to be copied of the other channel
   */
  channel::channel(const channel& other,
          const int& fromRow,const int& toRow,
          const int& fromCol,const int& toCol)
    : matrix<float>(other,fromRow,toRow,fromCol,toCol) {
  };

  /*
   * copy constructor (reference to a subchannel).
   *
   * creates subchannel of another channel.
   *
   * if <code>copyData == true</code>, the new object has its own data
   * (equivalent to previous copy constructor).
   *
   * if <code>copyData == false</code>, the new object has references to
   * the other channel, which means that the data is not necessarily
   * consecutive.  (This will not be a connected but a lined channel)
   *
   * Those algorithms which use direct access to the channel memory block
   * should check first if the memory lies in a consecutive block!
   * (see getMode())
   *
   * @param copyData should the data of the other channel be copied or not
   * @param fromRow initial row of the other channel to be copied
   * @param toRow last row to be copied of the other channel
   * @param fromCol initial column of the other channel to be copied
   * @param fromRow last column to be copied of the other channel
   */
  channel::channel(const bool& copyData, channel& other,
                   const int& fromRow,const int& toRow,
                   const int& fromCol,const int& toCol)
    : matrix<float>(copyData,other,fromRow,toRow,fromCol,toCol) {
  };

  channel::channel(const bool& init, const int& rows, const int& cols)
    : matrix<float>(init,rows,cols) {
  };

  channel::channel(const bool& init, const point& size)
    : matrix<float>(init,size) {
  };

  /*
   * create a clone of this channel
   * @return a pointer to a copy of this matrix
   */
  mathObject* channel::clone() const {
    return new channel(*this);
  };

  /*
   * return the name of this type
   */
  const char* channel::getTypeName() const {
    return "channel";
  };

  /*
   * copy the <code>other</code> channel8 by casting each of its elements.
   *
   * The elements of the channel8 will be also multiplied by 1/255.
   *
   * @param other the channel8 to be casted
   * @return a reference to this channel
   * Example:
   * \code
   *   lti::channel8 matA(10,10,255); // a channel8
   *   lti::channel  matB;            // a channel
   *
   *   matB.castFrom(matA);         // this will copy matA in matB!!
   *                                // and all elements will have 1.0f
   * \endcode
   */
  channel& channel::castFrom(const channel8& other) {
    resize(other.rows(),other.columns(),0.0f,false,false);
    iterator it;
    vector<channel8::value_type>::const_iterator cit,eit;
    int y;
    for (y=0,it=begin();y<other.rows();y++) {
      const vector<channel8::value_type>& vct=other.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;cit++,it++) {
        (*it)=static_cast<float>(*cit)/255.0f;
      }
    }
    return (*this);
  };

  /*
   * cast from image
   */
  channel& channel::castFrom(const image& other) {
    resize(other.rows(),other.columns(),0.0f,false,false);
    iterator it;
    vector<rgbPixel>::const_iterator cit,eit;
    int y;
    for (y=0,it=begin();y<other.rows();y++) {
      const vector<rgbPixel>& vct=other.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;cit++,it++) {
        (*it)=(static_cast<float>((*cit).getRed())+
               static_cast<float>((*cit).getGreen())+
               static_cast<float>((*cit).getBlue()))/765.0f;
      }
    }

    return (*this);
  };

  /*
   * Apply a gray valued transformation which maps the given intervall to
   * [0.0,1.0] (default) or the explicitly given "destination" interval
   * @param minVal the lower limit of the original data interval
   * @param maxVal the higher limit of the original data interval
   * @param minDest the lower limit of the mapped interval (default 0.0f)
   * @param maxDest the higher limit of the mapped interval (default 1.0f)
   * @return a reference to this object
   */
  channel& channel::mapLinear(const channel& other,
                              const float& minVal, const float& maxVal,
                              const float& minDest,const float& maxDest) {

    resize(other.size(),0.0f,false,false);

    iterator it;
    vector<channel::value_type>::const_iterator cit,eit;
    int y;

    float tm,tb;

    if (maxVal != minVal) {
      tm = (maxDest-minDest)/(maxVal-minVal);
    } else {
      tm = 1.0f;
    }

    tb = maxDest-maxVal*tm;

    const float m=tm;
    const float b=tb;

    if (b == 0.0f) {
      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it)=(*cit)*m;
        }
      }
    } else if (m == 1.0f) {
      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it)=(*cit)+b;
        }
      }
    } else {
      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it)=(*cit)*m+b;
        }
      }
    }
    return (*this);
  }

  /*
   * Apply a gray valued transformation which maps the given intervall to
   * [0.0,1.0] (default) or the explicitly given "destination" interval
   * @param minVal the lower limit of the original data interval
   * @param maxVal the higher limit of the original data interval
   * @param minDest the lower limit of the mapped interval (default 0.0f)
   * @param maxDest the higher limit of the mapped interval (default 1.0f)
   * @return a reference to this object
   */
  channel& channel::mapLinear(const float& minVal, const float& maxVal,
                              const float& minDest, const float& maxDest) {

    vector<channel::value_type>::iterator cit,eit;
    int y;

    float tm,tb;

    if (maxVal != minVal) {
      tm = (maxDest-minDest)/(maxVal-minVal);
    } else {
      tm = 1.0f;
    }

    tb = maxDest-maxVal*tm;

    const float m=tm;
    const float b=tb;

    if (b == 0.0f) {
      for (y=0;y<rows();++y) {
        vector<channel::value_type>& vct=getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit) {
          (*cit)*=m;
        }
      }
    } else if (m == 1.0f) {
      for (y=0;y<rows();++y) {
        vector<channel::value_type>& vct=getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit) {
          (*cit)+=b;
        }
      }
    } else {
      for (y=0;y<rows();++y) {
        vector<channel::value_type>& vct=getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit) {
          (*cit)=(*cit)*m+b;
        }
      }
    }
    return (*this);
  }

  // --------------------------------------------------------------------------
  //                    CHANNEL8
  // --------------------------------------------------------------------------

  /*
   * a format for 8-bit-channels.
   *
   * This class is identical to a matrix of floats except for the method
   * castFrom(channel8)
   */

  /*
   * default constructor creates an empty channel8
   */
  channel8::channel8() : matrix<ubyte>() {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> Channel8
   * and initializes all elements with <code>iniValue</code>
   * @param rows number of rows of the channel8
   * @param cols number of columns of the channel8
   * @param iniValue all elements will be initialized with this value
   */
  channel8::channel8(const int& rows,const int& cols,
                     const ubyte& iniValue)
      : matrix<ubyte>(rows,cols,iniValue) {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> Channel8
   * and initializes all elements with the data pointed by
   * <code>data</code>.  The first <code>cols</code>-elements of the data
   * will be copied on the first row, the next ones on the second row and
   * so on.
   * @param rows number of rows of the channel8
   * @param cols number of columns of the channel8
   * @param data pointer to the memory block with the data to be initialized
   * with.
   */
  channel8::channel8(const int& rows,const int& cols,const ubyte data[])
    : matrix<ubyte>(rows,cols,data) {
  };

  /*
   * this constructor creates a connected <code>dim.y x dim.x</code>
   * Channel8 and initializes all elements with <code>iniValue</code>
   * @param dim lti::point with the size of the channel8
   * (dim.x is the number of columns and dim.y the number of rows)
   * @param iniValue all elements will be initialized with this value
   */
  channel8::channel8(const point& size,const ubyte& iniValue)
    : matrix<ubyte>(size,iniValue) {
  };

  /*
   * copy constructor.
   *
   * create this channel8 as a connected copy of another channel8
   * for this const version, the data will be always copied!
   * It is also possible to create a copy of a subchannel of another
   * channel.
   *
   * @param other the channel8 to be copied.
   * @param fromRow initial row of the other channel8 to be copied
   * @param toRow last row to be copied of the other channel8
   * @param fromCol initial column of the other channel8 to be copied
   * @param fromRow last column to be copied of the other channel8
   *
   * Example:
   * \code
   * lti::channel8 m(4,6,0); // channel8 with 24 elements
   * // ...
   * // initialize channel8 with:
   * //        0  1  2  3  4  5
   * //        2  1  5  4  0  3
   * //        1  2  1  2  3  2
   * //        3  3  2  1  2  3
   *
   * lti::channel8 sm(m,1,3,0,2)  // last line will leat to
   * //                              following contents in sm:
   * //        1  2  3
   * //        1  5  4
   * //        2  1  2
   * \endcode
   *
   */
  channel8::channel8(const channel8& other,
                     const int& fromRow,const int& toRow,
                     const int& fromCol,const int& toCol)
    : matrix<ubyte>(other,fromRow,toRow,fromCol,toCol) {
  };

  /*
   * copy constructor (reference to a subchannel8).
   *
   * creates subchannel8 of another channel8.
   *
   * if <code>copyData == true</code>, the new object has its own data
   * (equivalent to previous copy constructor).
   *
   * if <code>copyData == false</code>, the new object has references to
   * the other channel8, which means that the data is not necessarily
   * consecutive.  (This will not be a connected but a lined channel8)
   *
   * Those algorithms which use direct access to the channel8 memory block
   * should check first if the memory lies in a consecutive block!
   * (see getMode())
   *
   * @param copyData should the data of the other channel8 be copied or not
   * @param fromRow initial row of the other channel8 to be copied
   * @param toRow last row to be copied of the other channel8
   * @param fromCol initial column of the other channel8 to be copied
   * @param fromRow last column to be copied of the other channel8
   */
  channel8::channel8(const bool& copyData, channel8& other,
                     const int& fromRow,const int& toRow,
                     const int& fromCol,const int& toCol)
    : matrix<ubyte>(copyData,other,fromRow,toRow,fromCol,toCol) {
  };

  channel8::channel8(const bool& init, const int& rows, const int& cols)
    : matrix<ubyte>(init,rows,cols) {
  };

  channel8::channel8(const bool& init, const point& size)
    : matrix<ubyte>(init,size) {
  };

  /*
   * create a clone of this channel
   * @return a pointer to a copy of this matrix
   */
  mathObject* channel8::clone() const {
    return new channel8(*this);
  };

  /*
   * return the name of this type
   */
  const char* channel8::getTypeName() const {
    return "channel8";
  };

  /*
   * sum of elements
   */
  int channel8::sumOfElements() const {
    int y;
    int acc = 0;
    vector<channel8::value_type>::const_iterator cit,eit;
    for (y=0;y<rows();y++) {
      const vector<value_type>& vct = getRow(y);
      for (cit = vct.begin(),eit=vct.end();
           cit!=eit;
           ++cit) {
        acc+=static_cast<int>(*cit);
      }
    }
    return acc;
  }

  /*
   * copy the <code>other</code> channel by casting each of its elements.
   *
   * The elements of the channel will be also multiplied by 255.
   *
   * @param other the channel8 to be casted
   * @return a reference to this channel
   * Example:
   * \code
   *   lti::channel matA(10,10,1); // a channel
   *   lti::channel8  matB;          // a channel8
   *
   *   matB.castFrom(matA);         // this will copy matA in matB!!
   *                                // and all elements will have 255
   * \endcode
   */
  channel8& channel8::castFrom(const channel& other,
                               const bool minToBlack,
                               const bool maxToWhite) {

    resize(other.rows(),other.columns(),0,false,false);

    iterator it;
    vector<channel::value_type>::const_iterator cit,eit;
    int y;

    if (!minToBlack && !maxToWhite) {
      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it) = ((*cit) <= 0) ? 0 : (((*cit) >= 1.0f) ?
                            255 : static_cast<ubyte>((*cit)*255.0f));
        }
      }
    } else {
      float theMin,theMax;
      if (minToBlack && maxToWhite) {
        other.getExtremes(theMin,theMax);
      } else if (minToBlack) {
        theMin = other.minimum();
        theMax = 1.0f;
      } else {
        theMin = 0.0f;
        theMax = other.maximum();
      }

      float tm,tb;

      if (theMax == theMin) {
        tm = 0.0f;
        tb = 127.0f;
      } else {
        tm = 255.0f/(theMax-theMin);
        tb = -tm*theMin+0.5f;
      }

      const float m=tm;
      const float b=tb;

      for (y=0,it=begin();y<other.rows();++y) {
        const vector<channel::value_type>& vct=other.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it)=static_cast<ubyte>((*cit)*m+b);
        }
      }

    };

    return (*this);
  };

  /*
   * cast from image
   */
  channel8& channel8::castFrom(const image& other) {
    resize(other.rows(),other.columns(),ubyte(0),false,false);
    iterator it;
    vector<rgbPixel>::const_iterator cit,eit;
    int y;

    for (y=0,it=begin();y<other.rows();y++) {
      const vector<rgbPixel>& vct=other.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;cit++,it++) {
        (*it)=static_cast<ubyte>((static_cast<int>((*cit).getRed())+
                                  static_cast<int>((*cit).getGreen())+
                                  static_cast<int>((*cit).getBlue()))/3);
      }
    }

    return (*this);
  };

  /*
   * a format for signed 32-bit-channels.
   *
   * This class is identical to a matrix of integers except for the method
   * castFrom(channel)
   */


  /*
   * default constructor creates an empty channel32
   */
  channel32::channel32() : matrix<int>() {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> Channel32
   * and initializes all elements with <code>iniValue</code>
   * @param rows number of rows of the channel32
   * @param cols number of columns of the channel32
   * @param iniValue all elements will be initialized with this value
   */
  channel32::channel32(const int& rows,const int& cols,
                       const int& iniValue)
    : matrix<int>(rows,cols,iniValue) {
  };

  /*
   * this constructor creates a connected <code>rows x cols</code> Channel32
   * and initializes all elements with the data pointed by
   * <code>data</code>.  The first <code>cols</code>-elements of the data
   * will be copied on the first row, the next ones on the second row and
   * so on.
   * @param rows number of rows of the channel32
   * @param cols number of columns of the channel32
   * @param data pointer to the memory block with the data to be initialized
   * with.
   */
  channel32::channel32(const int& rows,const int& cols,const int data[])
    : matrix<int>(rows,cols,data) {
  };

  /*
   * this constructor creates a connected <code>dim.y x dim.x</code>
   * Channel32 and initializes all elements with <code>iniValue</code>
   * @param dim lti::point with the size of the channel32
   * (dim.x is the number of columns and dim.y the number of rows)
   * @param iniValue all elements will be initialized with this value
   */
  channel32::channel32(const point& size,const int& iniValue)
    : matrix<int>(size,iniValue) {
  };

  /*
   * copy constructor.
   *
   * create this channel32 as a connected copy of another channel32
   * for this const version, the data will be always copied!
   * It is also possible to create a copy of a subchannel of another
   * channel.
   *
   * @param other the channel32 to be copied.
   * @param fromRow initial row of the other channel32 to be copied
   * @param toRow last row to be copied of the other channel32
   * @param fromCol initial column of the other channel32 to be copied
   * @param fromRow last column to be copied of the other channel32
   *
   * Example:
   * \code
   * lti::channel32 m(4,6,0); // integer channel32 with 25 elements
   * // ...
   * // initialize Channel32 with:
   * //        0  1  2  3  4  5
   * //        2  1  5  4  0  3
   * //        1  2  1  2  3  2
   * //        3  3  2  1  2  3
   *
   * lti::channel32<int> sm(m,1,3,0,2)  // last line will leat to
   * //                                 following contents in sm:
   * //        1  2  3
   * //        1  5  4
   * //        2  1  2
   * \endcode
   *
   */
  channel32::channel32(const channel32& other,
                       const int& fromRow,const int& toRow,
                       const int& fromCol,const int& toCol)
    : matrix<int>(other,fromRow,toRow,fromCol,toCol) {
  };

  /*
   * copy constructor (reference to a subchannel32).
   *
   * creates subchannel32 of another channel32.
   *
   * if <code>copyData == true</code>, the new object has its own data
   * (equivalent to previous copy constructor).
   *
   * if <code>copyData == false</code>, the new object has references to
   * the other channel32, which means that the data is not necessarily
   * consecutive.  (This will not be a connected but a lined channel32)
   *
   * Those algorithms which use direct access to the channel32 memory block
   * should check first if the memory lies in a consecutive block!
   * (see getMode())
   *
   * @param copyData should the data of the other channel32 be copied or not
   * @param fromRow initial row of the other channel32 to be copied
   * @param toRow last row to be copied of the other channel32
   * @param fromCol initial column of the other channel32 to be copied
   * @param fromRow last column to be copied of the other channel32
   */
  channel32::channel32(const bool& copyData, channel32& other,
                       const int& fromRow,const int& toRow,
                       const int& fromCol,const int& toCol)
    : matrix<int>(copyData,other,fromRow,toRow,fromCol,toCol) {
  };

  channel32::channel32(const bool& init, const int& rows, const int& cols)
    : matrix<int>(init,rows,cols) {
  };

  channel32::channel32(const bool& init, const point& size)
    : matrix<int>(init,size) {
  };

  /*
   * create a clone of this channel
   * @return a pointer to a copy of this matrix
   */
  mathObject* channel32::clone() const {
    return new channel32(*this);
  };

  /*
   * return the name of this type
   */
  const char* channel32::getTypeName() const {
    return "channel32";
  };

}
