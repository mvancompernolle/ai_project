/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiKMColorQuantization.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.04.99
 * revisions ..: $Id: ltiKMColorQuantization.cpp,v 1.11 2006/09/05 10:17:48 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

#ifdef _LTI_MSC_6
#pragma warning ( disable : 4786 )
#endif

#include <set>
#include "ltiKMColorQuantization.h"
#include "ltiSort.h"

// #define _LTI_DEBUG 1

#ifdef _LTI_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif


namespace lti {

  // --------------------------------------------------
  // kMColorQuantization::parameters
  // --------------------------------------------------

  // default constructor
  kMColorQuantization::parameters::parameters()
    : colorQuantization::parameters() {

    maximalNumberOfIterations = int(50);
    thresholdDeltaPalette = 0.2f;
  }

  // copy constructor
  kMColorQuantization::parameters::parameters(const parameters& other)
    : colorQuantization::parameters()  {
    copy(other);
  }

  // destructor
  kMColorQuantization::parameters::~parameters() {
  }

  // get type name
  const char* kMColorQuantization::parameters::getTypeName() const {
    return "kMColorQuantization::parameters";
  }

  // copy member

  kMColorQuantization::parameters&
    kMColorQuantization::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    colorQuantization::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    colorQuantization::parameters& (colorQuantization::parameters::* p_copy)
      (const colorQuantization::parameters&) =
      colorQuantization::parameters::copy;
    (this->*p_copy)(other);
# endif

    maximalNumberOfIterations = other.maximalNumberOfIterations;
    thresholdDeltaPalette = other.thresholdDeltaPalette;

    return *this;
  }

  // alias for copy member
  kMColorQuantization::parameters&
    kMColorQuantization::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kMColorQuantization::parameters::clone() const {
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
  bool kMColorQuantization::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool kMColorQuantization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"maximalNumberOfIterations",
                 maximalNumberOfIterations);
      lti::write(handler,"thresholdDeltaPalette",
		 thresholdDeltaPalette);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorQuantization::parameters::write(handler,false);
# else
    bool (colorQuantization::parameters::* p_writeMS)(ioHandler&,
                                                      const bool) const =
      colorQuantization::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kMColorQuantization::parameters::write(ioHandler& handler,
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
  bool kMColorQuantization::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kMColorQuantization::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"maximalNumberOfIterations",
                maximalNumberOfIterations);
      lti::read(handler,"thresholdDeltaPalette",
		thresholdDeltaPalette);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorQuantization::parameters::read(handler,false);
# else
    bool (colorQuantization::parameters::* p_readMS)(ioHandler&,const bool) =
      colorQuantization::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kMColorQuantization::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // -----------------------------------------------------------------
  // kMColorQuantization
  // -----------------------------------------------------------------

  kMColorQuantization::kMColorQuantization()
    : colorQuantization() {
    parameters defaultParameters;
    setParameters(defaultParameters);
  }

  kMColorQuantization::kMColorQuantization(const parameters& par)
    : colorQuantization() {
    setParameters(par);
  }

  kMColorQuantization::kMColorQuantization(const kMColorQuantization& other)
    : colorQuantization()  {
    copy(other);
  }

  kMColorQuantization::~kMColorQuantization() {
  }

  // returns the current parameters
  const kMColorQuantization::parameters&
  kMColorQuantization::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // copy data of "other" functor.
  kMColorQuantization&
  kMColorQuantization::copy(const kMColorQuantization& other) {
    colorQuantization::copy(other);
    return (*this);
  }

  functor* kMColorQuantization::clone() const {
    return (new kMColorQuantization(*this));
  };

  const char* kMColorQuantization::getTypeName() const {
    return "kMColorQuantization";
  }

  bool kMColorQuantization::apply(const image& src,
                                  channel8& dest,
                                  palette& thePalette) const {

    const parameters& param = getParameters();

    if (param.numberOfColors <= 256) {
      matrix<int> tmp;
      bool result = apply(src,tmp,thePalette);
      dest.castFrom(tmp);
      return result;
    } else {
      setStatusString("8-bit mask can represent a maximum of 256 colors");
      return false;
    }
  }

  bool kMColorQuantization::apply(const image& src,
                                  matrix<int>& dest,
                                  palette& thePalette) const {

    const parameters& param = getParameters();
    kMeanColor kMeans(param.numberOfColors,
		      param.maximalNumberOfIterations,
		      param.thresholdDeltaPalette);
    return kMeans(src,dest,thePalette);
  }

  bool kMColorQuantization::apply(const image& src,
                                        image& dest) const {
    return colorQuantization::apply(src,dest);
  };

  bool kMColorQuantization::apply(image& srcdest) const {
    return colorQuantization::apply(srcdest);
  };


  const int kMColorQuantization::kMeanColor::firstKeySize = 4096;

  kMColorQuantization::kMeanColor::kMeanColor(const int& maxNumOfClasses,
                                              const int& maxIterations,
					      const float& thresDeltaPal)
    : theHash(0), maxNumberOfClasses(maxNumOfClasses),
      maxNumberOfIterations(maxIterations),
      thresholdDeltaPalette(thresDeltaPal),uni(0.0,1.0,1) {

  }

  kMColorQuantization::kMeanColor::~kMeanColor() {
  }


  bool kMColorQuantization::kMeanColor::operator()(const image& img,
                                                   matrix<int>& colorMap,
                                                   palette& thePalette) {

    if (img.empty()) {
      colorMap.clear();
      thePalette.clear();
      return true;
    }

    // find the clusters
    initialize(img);
    getInitialPalette(thePalette);
    iterate();

    // fill the colorMap
    int y,x;
    colorMap.resize(img.size(),0,false,false);
    for (y=0;y<img.rows();++y) {
      for (x=0;x<img.columns();++x) {
        colorMap.at(y,x) = at(img.at(y,x)).index;
      }
    }
    // fill the palette
    thePalette.resize(centroids.size(),rgbPixel(),false,false);
    for (x=0;x<centroids.size();++x) {
      thePalette.at(x) = centroids.at(x).getRGBPixel();
    }

    delete[] theHash;
    theHash = 0;

    return true;
  }

  /**
   * put the given pixel in the hash table
   */
  inline kMColorQuantization::kMeanColor::hashEntry&
  kMColorQuantization::kMeanColor::at(const rgbPixel& px) {
    const int key = px.getValue() & 0x00000FFF; // lower 12 bits
    const int secondkey = px.getValue() & 0x00FFF000; // upper 12 bits;
    return theHash[key][secondkey];
  }

  /**
   * put the given pixel in the hash table
   */
  inline bool kMColorQuantization::kMeanColor::put(const rgbPixel& px) {
    const int key = px.getValue() & 0x00000FFF; // lower 12 bits
    const int secondkey = px.getValue() & 0x00FFF000; // upper 12 bits;
    hashMapType::iterator it;
    it = theHash[key].find(secondkey);
    if (it == theHash[key].end()) {
      theHash[key][secondkey] = hashEntry(-1,1); // idx=-1, cnt=1
      return true;
    }
    else {
      (*it).second.counter++;
      return false;
    }
  }

  void kMColorQuantization::kMeanColor::initialize(const image& src) {
    int y;

#ifdef _LTI_DEBUG
    cout << "Creating hash table. " << endl;
#endif

    // create the the hash table
    delete[] theHash;
    theHash = new hashMapType[firstKeySize];
    realNumberOfClasses = 0;

    // insert the pixels in the hash table
    vector<rgbPixel>::const_iterator it,eit;
    // for each row in the image
    for (y=0;y<src.rows();++y) {
      const vector<rgbPixel>& vct = src.getRow(y);
      // for each col in the row
      for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
        if (put(*it)) {
          // if newly added it was a new color never used before...
          realNumberOfClasses++;
        }
      }
    }
  }

  rgbPixel kMColorQuantization::kMeanColor::getAnImageColor() {
    int i,j,k(0);
    int steps = static_cast<int>(1.5+(uni.draw()*7));
    uint32 val;
    j = lastHashPosition;
    rgbPixel px;

    hashMapType::iterator it;
    i=(j+1)%firstKeySize;

    // for each entry in the array of maps of hashEntries...
    it=theHash[i].begin();
    while ((i!=j) && (k < steps)) {
      if (it==theHash[i].end()) {
        i++;
        it=theHash[i].begin();
      } else {
        ++it;
        if (it != theHash[i].end()) {
          k++;
        }
      }
    }

    if (k >= steps) {
      // find which centroid corresponds to this entry
      val = static_cast<uint32>( (*it).first | i);
      px = rgbPixel(val);
    }

    lastHashPosition=(i+1)%firstKeySize;
    return px;
  }

  void
  kMColorQuantization::kMeanColor::getInitialPalette(const
                                                   lti::palette& thePalette) {
    // initialize with black
    centroids.resize(min(maxNumberOfClasses,realNumberOfClasses),
                     lti::Black,false,true);
    centerElems.resize(centroids.size(),0,false,false);

#ifdef _LTI_DEBUG
    cout << "Initial Palette with "<< centroids.size()
         << " colors from " << realNumberOfClasses << endl;
#endif

    int i,j,k,kk(0);
    int idx;
    uint32 val;
    trgbPixel<float> px;
    bool allEntriesUsed = true;
    ivector centrIndex(centerElems.size());
    ivector tmpCenterElems;
    sort2<int,int> sorter(true); // sort in descending order
    uniformDistribution unirnd; // random number generator from 0 to 1;


    if (maxNumberOfClasses < realNumberOfClasses) {
      // initialize the centroids with palette and gray values
      int palSize = thePalette.size();
      int centSize = centroids.size();
      int greyValues = centSize-palSize;

      // if there is a palette(size>0), init the centroids
      for (k=0;k<min(centSize,palSize);++k) {
        centroids.at(k) = thePalette.at(k);
      }

      // init the "rest"-centroids with grey-values
      if (greyValues!=1) {
        for (;k<centroids.size();++k) {
          float val = (k-palSize)*255.0f/(greyValues-1);
          centroids.at(k) = trgbPixel<float>(val,val,val);
        }
      }
      else { // only one greyvalue entry
        float val = 255.0f/2.0;
        centroids.at(k) = trgbPixel<float>(val,val,val);
      }
      j = centroids.size(); // do not take the values from the image...
    }
    else {
      // quantization not really required (just a few colors on image)
      j = 0; // search the colors in the quantization
    }

    do {
      centerElems.fill(0);

      // assign a cluster label to each pixel

      hashMapType::iterator it;
      // for each entry in the array of maps of hashEntries...
      for (i=firstKeySize-1;i>=0;--i) {
        for (it=theHash[i].begin();
             it!=theHash[i].end();
             ++it) {
          hashEntry& he = (*it).second;
          // reconstruct the color value for the entry at (*it)
          val = static_cast<uint32>((*it).first | i);

          // j is used as flag to indicate the if the image has less
          // colors than the desired ones (<centroids.size()) or not.
          if (j<centroids.size()) {
            centerElems.at(j) += he.counter;
            he.index = j;
            j++;
          }
          else {
            // find which centroid corresponds to this entry
            px = rgbPixel(val);
            idx = 0;
            float dist = centroids.at(0).distanceSqr(px);
            float tmp;
            for (k=1;k<centroids.size();++k) {
              if ((tmp = centroids.at(k).distanceSqr(px)) < dist) {
                idx = k;
                dist = tmp;
              }
            }
            he.index = idx;

            // count number of pixels that belong to centroid(idx)
            centerElems.at(idx) +=  he.counter;
          }
        }
      }

      // recompute centroid-colors
      genericVector<bool> adapted(centroids.size(),false);
      for (i=firstKeySize-1;i>=0;--i) {
        for (it=theHash[i].begin();
             it!=theHash[i].end();
             ++it) {
          hashEntry& he = (*it).second;
          idx = he.index;
          val = static_cast<uint32>((*it).first | i);
          px = rgbPixel(val);
          // centerElemes[idx] is always >= he.counter
          // the centroids will contain at the end the average of all
          // colors assigned to it.
          if (!adapted.at(idx)) {
            centroids.at(idx)=trgbPixel<float>(0,0,0);
            adapted.at(idx)=true;
          }

          px.multiply(float(he.counter)/float(centerElems.at(idx)));
          centroids.at(idx).add(px);
        }
      }

      // if there are colors unused, they need to be assigned again...
      allEntriesUsed = true;
      kk = (kk % centrIndex.size());

      for (i=0;i<adapted.size();++i) {
        if (!adapted.at(i)) {
          allEntriesUsed = false;
          if (tmpCenterElems.empty()) {
            tmpCenterElems.copy(centerElems);
            for (k=0;k<centrIndex.size();++k)
              centrIndex.at(k)=k;
            sorter.apply(tmpCenterElems,centrIndex);
            k=0;
          }
          // split the biggest clusters
#ifdef _LTI_DEBUG
          cout << "Cluster " << i << " unassigned." << endl;
          cout << "  Splitting " << centrIndex.at(kk) << endl;
#endif

          centroids.at(i)=centroids.at(centrIndex.at(kk)) +
            trgbPixel<float>(static_cast<float>(4*uni.draw()-2),
                             static_cast<float>(4*uni.draw()-2),
                             static_cast<float>(4*uni.draw()-2));
          kk++;
          kk = (kk % centrIndex.size());
        }
      }

    } while (j!=0 && !allEntriesUsed);
  }

  void kMColorQuantization::kMeanColor::iterate() {
    bool changed = true;
    vector<trgbPixel<float> > centroidsOld;
    float changePal = thresholdDeltaPalette+1;
    int iter = 0;
    int i,k,total,counter;
    int idx,idx2;
    float dist,tmp;
    uint32 val;
    trgbPixel<float> px,px2;
    hashMapType::iterator it;

    while (changed &&
	   iter<maxNumberOfIterations &&
	   changePal>thresholdDeltaPalette) {

      changed = false;
      centroidsOld.copy(centroids);

      for (i=0;i<firstKeySize;++i) {
        for (it=theHash[i].begin();
             it!=theHash[i].end();
             ++it) {
          hashEntry& he = (*it).second;

          // find which centroid corresponds to this entry
          val = static_cast<uint32>( (*it).first | i);
          px = rgbPixel(val);
          idx = 0;
          dist =  centroids.at(0).distanceSqr(px);
          for (k=1;k<centroids.size();++k) {
            if ((tmp = centroids.at(k).distanceSqr(px)) < dist) {
              idx = k;
              dist = tmp;
            }
          }

          if (idx != he.index) { // centroid changed!
	    changed = true;

	    counter = he.counter;
            idx2 = he.index; //old
            he.index = idx;  //new
            px2 = px;

            // update the old centroid
            total = centerElems.at(idx2) - counter;
            if (total!=0) {
              px2.multiply(float(counter)/float(total));
              centroids.at(idx2).multiply(float(centerElems.at(idx2))/
                                          float(total));
              centroids.at(idx2).subtract(px2);
            }
            centerElems.at(idx2) = total;

            // recompute centroid
            total = centerElems.at(idx) + counter;
            px.multiply(float(counter)/
                        float(total));
            centroids.at(idx).multiply(float(centerElems.at(idx))/
                                       float(total));
            centroids.at(idx).add(px);
            centerElems.at(idx) = total;
          }
        }
      }
      changePal = 0.0f;
      for(k=0;k<centroids.size();++k) {
	changePal += centroids.at(k).distanceSqr(centroidsOld.at(k));
      }

#ifdef _LTI_DEBUG
      cout << "Iteration: " << iter << " change:"<<changePal<<endl;
#endif

      iter++;
    }
  }

} // namespace
