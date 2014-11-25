/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiFastRelabeling.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.5.2003
 * revisions ..: $Id: ltiFastRelabeling.cpp,v 1.9 2006/09/05 10:12:23 ltilib Exp $
 */

#include "ltiFastRelabeling.h"
#include "ltiSort.h"

#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 3
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#endif

namespace lti {


  /**
   * The helper class partially relabels a matrix<T> in a matrix<U>.
   * The returned equivLabels vector contains all necessary informtation to
   * complete the relabeling process.  It does not the job here in order to
   * allow other classes to compute other information while finishing the
   * computations, saving some time in time critical operations.
   */
  template<class T,class U>
  class relabelingHelper {
  public:
    /**
     * The only constructor expects the threshold and
     * if the input mask is labeled or not
     */
    relabelingHelper(const T minThresh,
                     const T maxThresh,
                     const bool labeled,
                     const bool neigh4);

    /**
     * Partially relabel the src matrix and leave the result in
     * dest matrix.
     */
    bool apply(const matrix<T>& src,
               matrix<U>& dest,
               vector<U>& equivLabels,
               int& numObjects) const;

    /**
     * Just relabel the destination, without computing anything else
     */
    bool simpleRelabel(const vector<U>& equivLabels,
                       const int numObjects,
                       matrix<U>& dest) const;

    /**
     * Just relabel the destination, without computing anything else
     */
    bool simpleRelabel(const ivector& equivLabels,
                       const int numObjects,
                       const imatrix& idest,
                             matrix<U>& dest) const;

    /**
     * Just relabel the destination, but compute also the size of each object.
     */
    bool relabelWithArea(const vector<U>& equivLabels,
                         const int numObjects,
                         matrix<U>& dest,
                         ivector& objSize) const;

    /**
     * Just relabel the destination, but compute also the size of each object.
     */
    bool relabelWithArea(const ivector& equivLabels,
                         const int numObjects,
                         const imatrix& idest,
                         matrix<U>& dest,
                         ivector& objSize) const;

    /**
     * Sort labels and eliminate those objects with sizes smaller than the
     * given threshold.
     *
     * @param minSize minimum number of pixels an object must have.
     * @param numPixel number of pixels per object.  The index of the vector
     *                 corresponds to the label in the given mask.
     *                 After calling this vector is also sorted.
     * @param mask the new label index
     */
    bool sortLabels(const int minSize,
                    ivector& numPixels,
                    vector<U>& reindex) const;

    /**
     * Sort labels and eliminate those objects with sizes smaller than the
     * given threshold.
     *
     * @param minSize minimum number of pixels an object must have.
     * @param numPixel number of pixels per object.  The index of the vector
     *                 corresponds to the label in the given mask.
     *                 After calling this vector is also sorted.
     * @param mask new relabeled mask
     */
    bool sortLabels(const int minSize,
                    ivector& numPixels,                                  
                    matrix<U>& mask) const;


    /**
     * Sort labels and eliminate those objects with sizes smaller than the
     * given threshold.
     *
     * @param minSize minimum number of pixels an object must have.
     * @param numPixel number of pixels per object.  The index of the vector
     *                 corresponds to the label in the given mask.
     *                 After calling this vector is also sorted.
     * @param mask new relabeled mask
     * @param objects vector of objects
     */
    bool sortLabels(const int minSize,
                    ivector& numPixels,                                  
                    matrix<U>& mask,
                    std::vector<areaPoints>& objects) const;

    /**
     * Suppress small objects.
     *
     * @param minSize minimum number of pixels an object must have.
     * @param numPixel number of pixels per object.  The index of the vector
     *                 corresponds to the label in the given mask.
     *                 After calling this vector is also sorted.
     * @param mask new relabeled mask
     */
    bool suppress(const int minSize,
                  ivector& numPixels,                                  
                  matrix<U>& mask) const;

    /**
     * Suppress small objects.
     *
     * @param minSize minimum number of pixels an object must have.
     * @param numPixel number of pixels per object.  The index of the vector
     *                 corresponds to the label in the given mask.
     *                 After calling this vector is also sorted.
     * @param objects vector of objects
     * @param mask new relabeled mask
     */
    bool suppress(const int minSize,
                  ivector& numPixels,                                  
                  matrix<U>& mask,
                  std::vector<areaPoints>& objects) const;

  protected:
    /**
     * Set in constructor to specify if the input mask is labeled or not
     */
    const bool labeled;

    /**
     * Type of neighborhood used
     */
    const bool fourNeighborhood;


    /**
     * Set in constructor to the threshold to be used:
     * Only values >= minThreshold will be considered for relabeling
     */
    const T minThreshold;

    /**
     * Set in constructor to the threshold to be used:
     * Only values <= maxThreshold will be considered for relabeling
     */
    const T maxThreshold;

    /**
     * relabel unlabeled mask, 4 neighborhood
     */
    bool relabelUnlabeled4(const matrix<T>& src,
                                 matrix<U>& dest,
                                 vector<U>& equivLabels,
                                 int& numObjects) const;

    /**
     * relabel labeled mask, 4 neighborhood
     */
    bool relabelLabeled4(const matrix<T>& src,
                               matrix<U>& dest,
                               vector<U>& equivLabels,
                               int& numObjects) const;

    /**
     * relabel unlabeled mask, 8 neighborhood
     */
    bool relabelUnlabeled8(const matrix<T>& src,
                                 matrix<U>& dest,
                                 vector<U>& equivLabels,
                                 int& numObjects) const;

    /**
     * relabel labeled mask, 8 neighborhood
     */
    bool relabelLabeled8(const matrix<T>& src,
                               matrix<U>& dest,
                               vector<U>& equivLabels,
                               int& numObjects) const;

    /**
     * update equivalence vector
     *
     * @return number of objects found
     */
    int updateEquivalencesVector(      vector<U>& equivLabels,
                                 const int numLabels,
                                 const int background) const;

  };

  template<class T,class U>
  relabelingHelper<T,U>::relabelingHelper(const T minThresh,
                                          const T maxThresh,
                                          const bool lab,
                                          const bool neigh4)
    : labeled(lab),fourNeighborhood(neigh4),
      minThreshold(minThresh),maxThreshold(maxThresh) {
  }

  template<class T,class U>
  bool 
  relabelingHelper<T,U>::simpleRelabel(const vector<U>& equivLabels,
                                       const int numObjects,
                                             matrix<U>& dest) const {

    int y;
    typename vector<U>::iterator vit,eit;
    for (y=0;y<dest.rows();++y) {
      vector<U>& vct = dest.getRow(y);
      for (vit=vct.begin(),eit=vct.end();vit!=eit;++vit) {
        (*vit)=equivLabels.at((*vit));
      }
    }

    return true;
  }

  template<class T,class U>
  bool 
  relabelingHelper<T,U>::simpleRelabel(const ivector& equivLabels,
                                       const int numObjects,
                                       const imatrix& idest,
                                             matrix<U>& dest) const {

    dest.resize(idest.size(),U(),false,false);
    int y;
    ivector::const_iterator ivit;
    typename vector<U>::iterator vit,eit;
    for (y=0;y<dest.rows();++y) {
      const ivector& ivct = idest.getRow(y);
      vector<U>& vct = dest.getRow(y);
      for (ivit=ivct.begin(),vit=vct.begin(),eit=vct.end();
           vit!=eit;
           ++vit,++ivit) {
        (*vit)=static_cast<U>(equivLabels.at(*ivit));
      }
    }

    return true;
  }

  template<class T,class U>
  bool 
  relabelingHelper<T,U>::relabelWithArea(const vector<U>& equivLabels,
                                         const int numObjects,
                                               matrix<U>& dest,
                                               ivector& numPixels) const {

    // initialize counters
    numPixels.resize(numObjects,0,false,true);

    int y;
    typename vector<U>::iterator vit,eit;
    for (y=0;y<dest.rows();++y) {
      vector<U>& vct = dest.getRow(y);
      for (vit=vct.begin(),eit=vct.end();vit!=eit;++vit) {
        (*vit)=equivLabels.at((*vit));
        numPixels.at(*vit)++;
      }
    }

    return true;
  }

  template<class T,class U>
  bool 
  relabelingHelper<T,U>::relabelWithArea(const ivector& equivLabels,
                                         const int numObjects,
                                         const imatrix& idest,
                                               matrix<U>& dest,
                                               ivector& numPixels) const {
    // destination
    dest.resize(idest.size(),U(),false,false);

    // initialize counters
    numPixels.resize(numObjects,0,false,true);

    int y;
    ivector::const_iterator ivit;
    typename vector<U>::iterator vit,eit;
    for (y=0;y<dest.rows();++y) {
      vector<U>& vct = dest.getRow(y);
      const ivector ivct = idest.getRow(y);
      for (ivit=ivct.begin(),vit=vct.begin(),eit=vct.end();
           vit!=eit;
           ++vit,++ivit) {
        (*vit)=static_cast<U>(equivLabels.at(*ivit));
        numPixels.at(*vit)++;
      }
    }

    return true;
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::sortLabels(const int minSize,
					 ivector& numPixels,
					 vector<U>& reindex) const {

    vector<U> labels;
    labels.resize(numPixels.size(),U(),false,false);
    int i;
    for (i=0;i<labels.size();++i) {
      labels.at(i)=static_cast<U>(i);
    }

    // avoid removing the background label from 0
    int bkgPix = numPixels.at(0);
    numPixels.at(0)=std::numeric_limits<int>::max();

    // sort the labels
    sort2<int,U> sorter(true); // descending order
    sorter.apply(numPixels,labels);

    reindex.resize(labels.size(),U(),false,false);

    // labels for biggest objects
    for (i=0;(i<reindex.size()) && (numPixels.at(i) >= minSize);++i) {
      reindex.at(labels.at(i))=i;
    }

    // reset labels for smallest objects
    for (;i<reindex.size();++i) {
      reindex.at(labels.at(i))=0; // background label
    }

    numPixels.at(labels.at(0)) = bkgPix; // restore pixel count

    return true;
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::sortLabels(const int minSize,
                                               ivector& numPixels,
                                               matrix<U>& mask) const {

    if (mask.empty()) {
      return true;
    }

    //This should be changed to call sortLabels(minSize, numPixels, reindex)
    //I did not do it, because Pablo is in holidays ( Claudia )
    //I implemented sortLabels(minSize, numPixels) because a geomFeatures
    //class does not need to call the relabeling here.

    vector<U> labels,reindex;
    labels.resize(numPixels.size(),U(),false,false);
    int i;
    for (i=0;i<labels.size();++i) {
      labels.at(i)=static_cast<U>(i);
    }

    // avoid removing the background label from 0
    int bkgPix = numPixels.at(0);
    numPixels.at(0)=std::numeric_limits<int>::max();

    // sort the labels
    sort2<int,U> sorter(true); // descending order
    sorter.apply(numPixels,labels);

    reindex.resize(labels.size(),U(),false,false);

    // labels for biggest objects
    for (i=0;(i<reindex.size()) && (numPixels.at(i) >= minSize);++i) {
      reindex.at(labels.at(i))=i;
    }

    // reset labels for smallest objects
    for (;i<reindex.size();++i) {
      reindex.at(labels.at(i))=0; // background label
    }

    numPixels.at(labels.at(0)) = bkgPix; // restore pixel count

    return simpleRelabel(reindex,reindex.size(),mask);
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::sortLabels(const int minSize,
                                         ivector& numPixels,
                                         matrix<U>& mask,
                                         std::vector<areaPoints>& objects
                                         ) const {
    if (mask.empty()) {
      return true;
    }

    vector<U> labels,reindex;
    labels.resize(numPixels.size(),U(),false,false);
    int i;

    objects.clear();

    for (i=0;i<labels.size();++i) {
      labels.at(i)=static_cast<U>(i);
    }

    // avoid removing the background label from 0
    int bkgPix = numPixels.at(0);
    numPixels.at(0)=std::numeric_limits<int>::max();

    // sort the labels
    sort2<int,U> sorter(true); // descending order
    sorter.apply(numPixels,labels);

    reindex.resize(labels.size(),U(),false,false);

    // labels for biggest objects
    for (i=0;(i<reindex.size()) && (numPixels.at(i) >= minSize);++i) {
      reindex.at(labels.at(i))=i;
    }
    
    objects.resize(i);

    // reset labels for smallest objects
    for (;i<reindex.size();++i) {
      reindex.at(labels.at(i))=0; // background label
    }

    numPixels.at(labels.at(0)) = bkgPix; // restore pixel count

    // relabel
    int x,y;
    for (y=0;y<mask.rows();++y) {
      for (x=0;x<mask.columns();++x) {
        U& vit = mask.at(y,x);
        vit=reindex.at(vit);
        (objects[vit]).push_back(point(x,y));
      }
    }
    return true;
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::suppress(const int minSize,
                                             ivector& numPixels,
                                             matrix<U>& mask) const {

    if (mask.empty()) {
      return true;
    }

    vector<U> labels;
    labels.resize(numPixels.size(),U(),false,false);
    labels.at(0)=0;
    int i,j;
    // beginning with 1, to ignore background
    for (i=1,j=1;i<labels.size();++i) {
      if (numPixels.at(i) >= minSize) {
        labels.at(i) = j;
        j++;
      } else {
        labels.at(i) = 0;
      }
    }

    return simpleRelabel(labels,j,mask);
  }


  template<class T,class U>
  bool relabelingHelper<T,U>::suppress(const int minSize,
                                       ivector& numPixels,
                                       matrix<U>& mask,
                                       std::vector<areaPoints>& objects
                                       ) const {

    if (mask.empty()) {
      return true;
    }

    vector<U> labels;
    objects.clear();

    labels.resize(numPixels.size(),U(),false,false);
    int i,j;
    // beginning with 1, to ignore background
    labels.at(0)=0;
    for (i=1,j=1;i<labels.size();++i) {
      if (numPixels.at(i) >= minSize) {
        labels.at(i) = j;
        j++;
      } else {
        labels.at(i) = 0;
      }
    }


    objects.resize(j);

    // relabel
    int x,y;
    for (y=0;y<mask.rows();++y) {
      for (x=0;x<mask.columns();++x) {
        U& vit = mask.at(y,x);
        _lti_debug3("mask.at("<<y<<","<<x<<")= "<<vit<<" -> " 
                    << labels.at(vit) << std::endl);
        vit=labels.at(vit);
        (objects[vit]).push_back(point(x,y));
      }
    }
    
    return true;
  }


  template<class T,class U>
  bool relabelingHelper<T,U>::apply(const matrix<T>& src,
                                          matrix<U>& dest,
                                          vector<U>& equivLabels,
                                          int& numObjects) const {
    if (src.empty()) {
      dest.clear();
      equivLabels.clear();
      numObjects=0;
      return true;
    }

    if (fourNeighborhood) {
      if (labeled) {
        return relabelLabeled4(src,dest,equivLabels,numObjects);
      } else {
        return relabelUnlabeled4(src,dest,equivLabels,numObjects);
      }
    } else {
      if (labeled) {
        return relabelLabeled8(src,dest,equivLabels,numObjects);
      } else {
        return relabelUnlabeled8(src,dest,equivLabels,numObjects);
      }
    }
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::relabelLabeled4(const matrix<T>& src,
                                                    matrix<U>& dest,
                                                    vector<U>& equivLabels,
                                                    int& numObjects) const {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    static viewer::parameters vpar;
    vpar.contrast=2.0f;
    vpar.zoomFactor=2;
    vpar.title="Relabeling";
    static viewer view(vpar);

    vpar.title="Labels";
    vpar.pixelsPerElement = 1;
    vpar.useBoxes=true;
    vpar.useLines=true;
    vpar.drawMeanValues = false;

    static viewer viewl(vpar);
#endif

    dest.resize(src.size(),U(),false,false);
    
    // first line    
    int x,y;

    // the labels vector worst case is one where each pixel is
    // isolated.
    const int size = src.rows()*src.columns();
    const U background = 
      static_cast<U>(min(static_cast<int>(std::numeric_limits<U>::max()),
                         size-1));
    U currentLabel = 0;
    U lastLabel;
    U tmpLabel;
    T lastSrc;

    equivLabels.resize(size,background,false,true);
    
    // ---------------------------------
    // first iteration in scan direction
    // ---------------------------------

    // TODO: this first iteration can use iterators or pointer arithmetic
    //       to considerably speed up the computations.

    // first line
    lastSrc = src.at(0,0);
    lastLabel = dest.at(0,0) = 
      ((lastSrc < minThreshold) || (lastSrc > maxThreshold)) ? background :
                                                               currentLabel++;
    
    for (x=1;x<src.columns();++x) {
      const T thisSrc = src.at(0,x);
      
      if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
        dest.at(0,x) = lastLabel = background;
      } else {
        if (lastSrc != thisSrc) {
          dest.at(0,x) = lastLabel = currentLabel;
          ++currentLabel;
        } else {
          dest.at(0,x) = lastLabel;
        }
      }

      lastSrc=thisSrc;
    }

    // the rest of the image
    for (y=1;y<src.rows();++y) {      
      const T thisSrc = src.at(y,0);

      // first line pixel
      if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
        dest.at(y,0) = lastLabel = background;
      }
      else {
        if (src.at(y-1,0) != thisSrc) {
          dest.at(y,0) = lastLabel = currentLabel;
          ++currentLabel;
        } else {
          dest.at(y,0) = lastLabel = dest.at(y-1,0);
        }
      }
      
      lastSrc=src.at(y,0);

      // rest of the line
      for (x=1;x<src.columns();++x) {
        const T& thisSrc = src.at(y,x);

        if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
          lastLabel = dest.at(y,x) = background;
        }
        else {
          tmpLabel=dest.at(y-1,x);
          if (lastLabel != background) {
            if (tmpLabel==background) {
              // on the top background,
              if (lastSrc == thisSrc) {
                // same label on the left, keep it!
                dest.at(y,x) = lastLabel;
              } else {
                // on the left a different label, so get new one.
                dest.at(y,x) = lastLabel = currentLabel;
                currentLabel++;
              }
            } else {
              // here comes the difficult one, on the top and on the left
              // there are labels, and we need to "synchronize" them
              
              if ((thisSrc == lastSrc) && (thisSrc == src.at(y-1,x))) {
                // all three labels belong to the same object
                
                // both pixels have different labels, synchronize them:
                
                // find the lowest valid label for the left element
                while (equivLabels.at(lastLabel) != background) {
                  lastLabel = equivLabels.at(lastLabel);
                }
                
                // find the lowest valid label for the top element
                while (equivLabels.at(tmpLabel) != background) {
                  tmpLabel = equivLabels.at(tmpLabel);
                }
                
                if (tmpLabel != lastLabel) {
                  if (lastLabel < tmpLabel) {
                    equivLabels.at(tmpLabel) = lastLabel;
                  } else {
                    equivLabels.at(lastLabel) = tmpLabel;
                    lastLabel = tmpLabel;
                  }                  
                }
                dest.at(y,x) = lastLabel;

              } else if (thisSrc == lastSrc) {
                // just the left pixel share the same label
                dest.at(y,x) = lastLabel;
              } else if (thisSrc == src.at(y-1,x)) {
                dest.at(y,x) = lastLabel = tmpLabel;               
              } else {
                // new label necessary
                // on the left background and different label on the top
                dest.at(y,x) = lastLabel = currentLabel;
                currentLabel++;
              }
            }
                       
          } else if (thisSrc == src.at(y-1,x)) {
            // on the left is background, but same label on the top,
            // so let's take the top label!
            dest.at(y,x) = lastLabel = tmpLabel;
          } else {
            // on the left background and different label on the top
            dest.at(y,x) = lastLabel = currentLabel;
            currentLabel++;
          }
        }

        lastSrc=thisSrc;
      }
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    view.show(dest);
    vector<U> tmpLabels(currentLabel,&equivLabels.at(0));
    viewl.show(tmpLabels);
#endif

    // relabel 
    numObjects = updateEquivalencesVector(equivLabels,
                                          currentLabel,
                                          background);

    // in case the type U does not allow too many objects, the relabeled
    // mask can be very inconsistent!
    return (currentLabel < background);
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::relabelUnlabeled4(const matrix<T>& src,
                                                      matrix<U>& dest,
                                                      vector<U>& equivLabels,
                                                      int& numObjects) const {

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    static viewer::parameters vpar;
    vpar.contrast=2.0f;
    vpar.zoomFactor=4;
    vpar.title="Relabeling";
    static viewer view(vpar);

    vpar.title="Labels";
    vpar.pixelsPerElement = 3;
    vpar.useBoxes=true;
    vpar.useLines=true;
    vpar.drawMeanValues = false;

    static viewer viewl(vpar);
#endif

    dest.resize(src.size(),U(),false,false);
    
    // first line
    int x,y;

    // the labels vector worst case is one where each pixel is
    // isolated.
    const int size = src.rows()*src.columns();
    const U background = 
      static_cast<U>(min(static_cast<int>(std::numeric_limits<U>::max()),
                         size-1));
    U currentLabel = 0;
    U lastLabel;
    U tmpLabel;

    equivLabels.resize(size,background,false,true);
    
    // ---------------------------------
    // first iteration in scan direction
    // ---------------------------------

    // TODO: this first iteration can use iterators or pointer arithmetic
    //       to considerably speed up the computations.

    // first line
    lastLabel = dest.at(0,0) = ((src.at(0,0) < minThreshold) ||
                                (src.at(0,0) > maxThreshold)) ? background :
                                                                currentLabel++;
    for (x=1;x<src.columns();++x) {
      if ((src.at(0,x)  < minThreshold) || (src.at(0,x)  > maxThreshold)) {
        lastLabel = dest.at(0,x) = background;
      } else {
        if (lastLabel == background) {
          lastLabel = dest.at(0,x) = currentLabel;
          ++currentLabel;
        } else {
          dest.at(0,x) = lastLabel;
        }
      }
    }

    // the rest of the image
    for (y=1;y<src.rows();++y) {      

      // first line pixel
      if ((src.at(y,0)  < minThreshold) || (src.at(y,0)  > maxThreshold)) {
        lastLabel = dest.at(y,0) = background;
      }
      else {
        if ((lastLabel = dest.at(y-1,0)) == background)  {
          lastLabel = dest.at(y,0) = currentLabel;
          currentLabel++;
        } else {
          dest.at(y,0) = lastLabel = dest.at(y-1,0);
        }
      }
        
      // rest of the line
      for (x=1;x<src.columns();++x) {
        if ((src.at(y,x)  < minThreshold) || (src.at(y,x)  > maxThreshold)) {
          dest.at(y,x) = lastLabel = background;
        }
        else {
          tmpLabel=dest.at(y-1,x);
          if (lastLabel != background) {
            if ((lastLabel != tmpLabel) && (tmpLabel != background)) {
              // find the lowest valid label for the left element
              while (equivLabels.at(lastLabel) != background) {
                lastLabel = equivLabels.at(lastLabel);
              }

              // find the lowest valid label for the top element
              while (equivLabels.at(tmpLabel) != background) {
                tmpLabel = equivLabels.at(tmpLabel);
              }

              if (lastLabel < tmpLabel) {
                equivLabels.at(tmpLabel) = lastLabel;
              } else if (lastLabel > tmpLabel) {
                equivLabels.at(lastLabel) = tmpLabel;
                lastLabel = tmpLabel;
              }              
            }
            dest.at(y,x) = lastLabel;

          } else if (tmpLabel != background) {
            dest.at(y,x) = lastLabel = tmpLabel;
          } else {
            dest.at(y,x) = lastLabel = currentLabel;
            currentLabel++;
          }
        }
      }
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    view.show(dest);
    vector<U> tmpLabels(currentLabel,&equivLabels.at(0));
    viewl.show(tmpLabels);
#endif

    // relabel 
    numObjects = updateEquivalencesVector(equivLabels,
                                          currentLabel,
                                          background);

    // in case the type U does not allow too many objects, the relabeled
    // mask can be very inconsistent!
    return (currentLabel < background);
  }


  template<class T,class U>
  bool relabelingHelper<T,U>::relabelLabeled8(const matrix<T>& src,
                                                    matrix<U>& dest,
                                                    vector<U>& equivLabels,
                                                    int& numObjects) const {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    static viewer::parameters vpar;
    vpar.contrast=2.0f;
    vpar.zoomFactor=2;
    vpar.title="Relabeling";
    static viewer view(vpar);

    vpar.title="Labels";
    vpar.pixelsPerElement = 1;
    vpar.useBoxes=true;
    vpar.useLines=true;
    vpar.drawMeanValues = false;

    static viewer viewl(vpar);
#endif

    dest.resize(src.size(),U(),false,false);
    
    // first line    
    int x,y,ym1;

    // the labels vector worst case is one where each pixel is
    // isolated.
    const int size = src.rows()*src.columns();
    const U background = 
      static_cast<U>(min(static_cast<int>(std::numeric_limits<U>::max()),
                         size-1));
    U currentLabel = 0;
    U lastLabel;
    U tmpLabel;
    T lastSrc;

    equivLabels.resize(size,background,false,true);
    
    U neighbors[4];
    int n;

    // ---------------------------------
    // first iteration in scan direction
    // ---------------------------------

    // TODO: this first iteration can use iterators or pointer arithmetic
    //       to considerably speed up the computations.

    // first line
    lastSrc = src.at(0,0);
    lastLabel = dest.at(0,0) = 
      ((lastSrc < minThreshold) || (lastSrc > maxThreshold)) ? background :
                                                               currentLabel++;
    
    for (x=1;x<src.columns();++x) {
      const T thisSrc = src.at(0,x);
      
      if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
        dest.at(0,x) = lastLabel = background;
      } else {
        if (lastSrc != thisSrc) {
          dest.at(0,x) = lastLabel = currentLabel;
          ++currentLabel;
        } else {
          dest.at(0,x) = lastLabel;
        }
      }

      lastSrc=thisSrc;
    }

    // the rest of the image
    for (y=1,ym1=0;y<src.rows();++y,++ym1) {      
      const T thisSrc = src.at(y,0);

      // first line pixel
      if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
        dest.at(y,0) = background;
      }
      else {
          // which neighbor pixels share label
        x=n=0;
        
        if (thisSrc == src.at(ym1,x)) {
          tmpLabel = dest.at(ym1,x);
          neighbors[n++]=tmpLabel;
        }
        if (thisSrc == src.at(ym1,x+1)) {
          tmpLabel = dest.at(ym1,x+1);
          if ((n==0) || (tmpLabel != neighbors[n-1])) {              
            neighbors[n++]=tmpLabel;
          }
        }

        if (n==0) {
          // no neighbors -> new label
          dest.at(y,x) = currentLabel;
          currentLabel++;
        } else if (n==1) {
          // only one neighbor, just get its label
          dest.at(y,x) = neighbors[0];
        } else {
          lastLabel=neighbors[n-1];              
          tmpLabel=neighbors[n-2];

          while (equivLabels.at(lastLabel) != background) {
            lastLabel = equivLabels.at(lastLabel);
          }
          
          // find the lowest valid label for the top element
          while (equivLabels.at(tmpLabel) != background) {
            tmpLabel = equivLabels.at(tmpLabel);
          }
          
          if (lastLabel < tmpLabel) {
            equivLabels.at(tmpLabel) = lastLabel;
          } else if (lastLabel > tmpLabel) {
            equivLabels.at(lastLabel) = tmpLabel;
            lastLabel = tmpLabel;
          }
          dest.at(y,x) = lastLabel;
        }
      }
      
      // rest of the line
      for (x=1;x<src.lastColumn();++x) {
        const T& thisSrc = src.at(y,x);

        if ((thisSrc < minThreshold) || (thisSrc > maxThreshold)) {
          dest.at(y,x) = background;
        }
        else {
          // which neighbor pixels share label
          n=0;
          
          if (thisSrc == src.at(y,x-1)) {
            neighbors[n++]=dest.at(y,x-1);
          }
          if (thisSrc == src.at(ym1,x-1)) {
            tmpLabel = dest.at(ym1,x-1);
            if ((n==0) || (tmpLabel != neighbors[n-1])) {
              neighbors[n++]=tmpLabel;
            }
          }
          if (thisSrc == src.at(ym1,x)) {
            tmpLabel = dest.at(ym1,x);
            if ((n==0) || (tmpLabel != neighbors[n-1])) {
              neighbors[n++]=tmpLabel;
            }
          }
          if (thisSrc == src.at(ym1,x+1)) {
            tmpLabel = dest.at(ym1,x+1);
            if ((n==0) || (tmpLabel != neighbors[n-1])) {              
              neighbors[n++]=tmpLabel;
            }
          }

          if (n==0) {
            // no neighbors -> new label
            dest.at(y,x) = currentLabel;
            currentLabel++;
          } else if (n==1) {
            // only one neighbor, just get its label
            dest.at(y,x) = neighbors[0];
          } else {
            lastLabel=neighbors[n-1];              
            while (n>=2) {
              tmpLabel=neighbors[n-2];

              while (equivLabels.at(lastLabel) != background) {
                lastLabel = equivLabels.at(lastLabel);
              }

              // find the lowest valid label for the top element
              while (equivLabels.at(tmpLabel) != background) {
                tmpLabel = equivLabels.at(tmpLabel);
              }

              if (lastLabel < tmpLabel) {
                equivLabels.at(tmpLabel) = lastLabel;
              } else if (lastLabel > tmpLabel) {
                equivLabels.at(lastLabel) = tmpLabel;
                lastLabel = tmpLabel;
              }
              neighbors[n-2]=lastLabel;
              --n;
            } // end while n
            dest.at(y,x) = lastLabel;
          }
        } 
      }

      // last pixel
      const T& thisSrcL = src.at(y,x);
      
      if ((thisSrcL < minThreshold) || (thisSrcL > maxThreshold)) {
        dest.at(y,x) = background;
      }
      else {
        // which neighbor pixels share label
        n=0;
        
        if (thisSrcL == src.at(y,x-1)) {
          neighbors[n++]=dest.at(y,x-1);
        }
        if (thisSrcL == src.at(ym1,x-1)) {
          tmpLabel = dest.at(ym1,x-1);
          if ((n==0) || (tmpLabel != neighbors[n-1])) {
            neighbors[n++]=tmpLabel;
          }
        }
        if (thisSrcL == src.at(ym1,x)) {
          tmpLabel = dest.at(ym1,x);
          if ((n==0) || (tmpLabel != neighbors[n-1])) {
            neighbors[n++]=tmpLabel;
          }
        }

        if (n==0) {
          // no neighbors -> new label
          dest.at(y,x) = currentLabel;
          currentLabel++;
        } else if (n==1) {
          // only one neighbor, just get its label
          dest.at(y,x) = neighbors[0];
        } else {
          lastLabel=neighbors[n-1];              
          while (n>=2) {
            tmpLabel=neighbors[n-2];
            
            while (equivLabels.at(lastLabel) != background) {
              lastLabel = equivLabels.at(lastLabel);
            }
            
            // find the lowest valid label for the top element
            while (equivLabels.at(tmpLabel) != background) {
              tmpLabel = equivLabels.at(tmpLabel);
            }
            
            if (lastLabel < tmpLabel) {
              equivLabels.at(tmpLabel) = lastLabel;
            } else if (lastLabel > tmpLabel) {
              equivLabels.at(lastLabel) = tmpLabel;
              lastLabel = tmpLabel;
            }
            neighbors[n-2]=lastLabel;
            --n;
          } // end while n
          dest.at(y,x) = lastLabel;
        }
      } 
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    view.show(dest);
    vector<U> tmpLabels(currentLabel,&equivLabels.at(0));
    viewl.show(tmpLabels);
#endif

    // relabel 
    numObjects = updateEquivalencesVector(equivLabels,
                                          currentLabel,
                                          background);

    // in case the type U does not allow too many objects, the relabeled
    // mask can be very inconsistent!
    return (currentLabel < background);
  }

  template<class T,class U>
  bool relabelingHelper<T,U>::relabelUnlabeled8(const matrix<T>& src,
                                                      matrix<U>& dest,
                                                      vector<U>& equivLabels,
                                                      int& numObjects) const {

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    static viewer::parameters vpar;
    vpar.contrast=2.0f;
    vpar.zoomFactor=4;
    vpar.title="Relabeling";
    static viewer view(vpar);

    vpar.title="Labels";
    vpar.pixelsPerElement = 3;
    vpar.useBoxes=true;
    vpar.useLines=true;
    vpar.drawMeanValues = false;

    static viewer viewl(vpar);
#endif

    dest.resize(src.size(),U(),false,false);
    
    // first line
    
    int x,y,ym1;

    // the labels vector worst case is one where each pixel is
    // isolated.
    const int size = src.rows()*src.columns();
    const U background = 
      static_cast<U>(min(static_cast<int>(std::numeric_limits<U>::max()),
                         size-1));
    U currentLabel = 0;
    U lastLabel;
    U tmpLabel;

    equivLabels.resize(size,background,false,true);

    U neighbors[4];
    int n;
    
    // ---------------------------------
    // first iteration in scan direction
    // ---------------------------------

    // TODO: this first iteration can use iterators or pointer arithmetic
    //       to considerably speed up the computations.

    // first line
    lastLabel = dest.at(0,0) = ((src.at(0,0) < minThreshold) ||
                                (src.at(0,0) > maxThreshold)) ? background :
                                                                currentLabel++;
    for (x=1;x<src.columns();++x) {
      if ((src.at(0,x)  < minThreshold) || (src.at(0,x)  > maxThreshold)) {
        lastLabel = dest.at(0,x) = background;
      } else {
        if (lastLabel == background) {
          lastLabel = dest.at(0,x) = currentLabel;
          ++currentLabel;
        } else {
          dest.at(0,x) = lastLabel;
        }
      }
    }

    // the rest of the image
    for (y=1,ym1=0;y<src.rows();++y,++ym1) {      
      // first line pixel
      if ((src.at(y,0)  < minThreshold) || (src.at(y,0)  > maxThreshold)) {
        lastLabel = dest.at(y,0) = background;
      }
      else {
        // which neighbor pixels share label
        x=n=0;
        if ((tmpLabel=dest.at(ym1,x))   != background) {
          neighbors[n++]=tmpLabel;          
        }
        if ((tmpLabel=dest.at(ym1,x+1)) != background) {
          if ((n==0) || (tmpLabel != neighbors[n-1])) {              
            neighbors[n++]=tmpLabel;
          }
        }

        if (n==0) {
          // no neighbors -> new label
          dest.at(y,x) = currentLabel;
          currentLabel++;
        } else if (n==1) {
          // only one neighbor, just get its label
          dest.at(y,x) = neighbors[0];
        } else {
          lastLabel=neighbors[n-1];              
          tmpLabel=neighbors[n-2];

          while (equivLabels.at(lastLabel) != background) {
            lastLabel = equivLabels.at(lastLabel);
          }
          
          // find the lowest valid label for the top element
          while (equivLabels.at(tmpLabel) != background) {
            tmpLabel = equivLabels.at(tmpLabel);
          }

          if (lastLabel < tmpLabel) {
            equivLabels.at(tmpLabel) = lastLabel;
          } else if (lastLabel > tmpLabel) {
            equivLabels.at(lastLabel) = tmpLabel;
            lastLabel = tmpLabel;
          }
          neighbors[n-2]=lastLabel;
          dest.at(y,x) = lastLabel;
        }
      }
        
      // rest of the line
      for (x=1;x<src.lastColumn();++x) {
        if ((src.at(y,x) < minThreshold) || (src.at(y,x) > maxThreshold)) {
          // pixel is "background"
          dest.at(y,x) = background;
        }
        else {
          // which neighbor pixels share label
          n=0;

          if ((tmpLabel=dest.at(y,x-1))   != background) {
            neighbors[n++]=tmpLabel;
          }
          if ((tmpLabel=dest.at(ym1,x-1)) != background) {
            if ((n==0) || (tmpLabel != neighbors[n-1])) {
              neighbors[n++]=tmpLabel;
            }
          }
          if ((tmpLabel=dest.at(ym1,x))   != background) {
            if ((n==0) || (tmpLabel != neighbors[n-1])) {
              neighbors[n++]=tmpLabel;
            }
          }
          if ((tmpLabel=dest.at(ym1,x+1)) != background) {
            if ((n==0) || (tmpLabel != neighbors[n-1])) {              
              neighbors[n++]=tmpLabel;
            }
          }

          if (n==0) {
            // no neighbors -> new label
            dest.at(y,x) = currentLabel;
            currentLabel++;
          } else if (n==1) {
            // only one neighbor, just get its label
            dest.at(y,x) = neighbors[0];
          } else {
            lastLabel=neighbors[n-1];              
            while (n>=2) {
              tmpLabel=neighbors[n-2];

              while (equivLabels.at(lastLabel) != background) {
                lastLabel = equivLabels.at(lastLabel);
              }

              // find the lowest valid label for the top element
              while (equivLabels.at(tmpLabel) != background) {
                tmpLabel = equivLabels.at(tmpLabel);
              }

              if (lastLabel < tmpLabel) {
                equivLabels.at(tmpLabel) = lastLabel;
              } else if (lastLabel > tmpLabel) {
                equivLabels.at(lastLabel) = tmpLabel;
                lastLabel = tmpLabel;
              }
              neighbors[n-2]=lastLabel;
              --n;
            }
            dest.at(y,x) = lastLabel;
          }
        } // end while n
      } // end for all x in this y

      // last pixel
      if ((src.at(y,x) < minThreshold) || (src.at(y,x) > maxThreshold)) {
        // pixel is "background"
        dest.at(y,x) = background;
      }
      else {
        // which neighbor pixels share label
        n=0;
        
        if ((tmpLabel=dest.at(y,x-1))   != background) {
          neighbors[n++]=tmpLabel;
        }
        if ((tmpLabel=dest.at(ym1,x-1)) != background) {
          if ((n==0) || (tmpLabel != neighbors[n-1])) {
            neighbors[n++]=tmpLabel;
          }
        }
        if ((tmpLabel=dest.at(ym1,x))   != background) {
          if ((n==0) || (tmpLabel != neighbors[n-1])) {
            neighbors[n++]=tmpLabel;
          }
        }

        if (n==0) {
          // no neighbors -> new label
          dest.at(y,x) = currentLabel;
          currentLabel++;
        } else if (n==1) {
          // only one neighbor, just get its label
          dest.at(y,x) = neighbors[0];
        } else {
          lastLabel=neighbors[n-1];              
          while (n>=2) {
            tmpLabel=neighbors[n-2];
            
            while (equivLabels.at(lastLabel) != background) {
              lastLabel = equivLabels.at(lastLabel);
            }
            
            // find the lowest valid label for the top element
            while (equivLabels.at(tmpLabel) != background) {
              tmpLabel = equivLabels.at(tmpLabel);
            }
            
            if (lastLabel < tmpLabel) {
              equivLabels.at(tmpLabel) = lastLabel;
            } else if (lastLabel > tmpLabel) {
              equivLabels.at(lastLabel) = tmpLabel;
              lastLabel = tmpLabel;
            }
            neighbors[n-2]=lastLabel;
            --n;
          }
          dest.at(y,x) = lastLabel;
        }
      }
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
    view.show(dest);
    vector<U> tmpLabels(currentLabel,&equivLabels.at(0));
    viewl.show(tmpLabels);
#endif

    // relabel 
    numObjects = updateEquivalencesVector(equivLabels,
                                          currentLabel,
                                          background);

    // in case the type U does not allow too many objects, the relabeled
    // mask can be very inconsistent!
    return (currentLabel < background);
  }


  template<class T,class U>
  int 
  relabelingHelper<T,U>::updateEquivalencesVector(      vector<U>& equivLabels,
                                                  const int numLabels,
                                                  const int background
                                                  ) const {

    // first pass is ready.  At this point currentLabel contains
    // one plus the last label used.
    
    // fix the equivalence vector
    int x;

    for (x=0;x<numLabels;++x) {
      if (equivLabels.at(x) != background) {
        while (equivLabels.at(equivLabels.at(x)) != background) {
          equivLabels.at(x)=equivLabels.at(equivLabels.at(x));
        }
      } 
    }

    int lastLabel = 1;
    for (x=0;x<numLabels;++x) {
      if (equivLabels.at(x) == background) {
        equivLabels.at(x) = lastLabel;
        ++lastLabel;
      } else {
        equivLabels.at(x) = equivLabels.at(equivLabels.at(x));
      }
    }
    equivLabels.at(background)=0;

    return lastLabel;
  }

  // --------------------------------------------------
  // fastRelabeling::parameters
  // --------------------------------------------------

  // default constructor
  fastRelabeling::parameters::parameters()
    : modifier::parameters() {
    
    minThreshold = int(1);
    maxThreshold = std::numeric_limits<int>::max();
    assumeLabeledMask = bool(true);
    fourNeighborhood  = bool(true);
    sortSize = false;
    minimumObjectSize = 1;
  }

  // copy constructor
  fastRelabeling::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  fastRelabeling::parameters::~parameters() {
  }

  // get type name
  const char* fastRelabeling::parameters::getTypeName() const {
    return "fastRelabeling::parameters";
  }

  // copy member

  fastRelabeling::parameters&
  fastRelabeling::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif
  
    minThreshold = other.minThreshold;
    maxThreshold = other.maxThreshold;
    assumeLabeledMask = other.assumeLabeledMask;
    fourNeighborhood = other.fourNeighborhood;
    sortSize = other.sortSize;
    minimumObjectSize = other.minimumObjectSize;

    return *this;
  }

  // alias for copy member
  fastRelabeling::parameters&
  fastRelabeling::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fastRelabeling::parameters::clone() const {
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
  bool fastRelabeling::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fastRelabeling::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"minThreshold",minThreshold);    
      lti::write(handler,"maxThreshold",maxThreshold);
      lti::write(handler,"assumeLabeledMask",assumeLabeledMask);
      lti::write(handler,"fourNeighborhood",fourNeighborhood);
      lti::write(handler,"sortSize",sortSize);
      lti::write(handler,"minimumObjectSize",minimumObjectSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fastRelabeling::parameters::write(ioHandler& handler,
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
  bool fastRelabeling::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fastRelabeling::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"minThreshold",minThreshold);     
      lti::read(handler,"maxThreshold",maxThreshold);
      lti::read(handler,"assumeLabeledMask",assumeLabeledMask);
      lti::read(handler,"fourNeighborhood",fourNeighborhood);
      lti::read(handler,"sortSize",sortSize);
      lti::read(handler,"minimumObjectSize",minimumObjectSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fastRelabeling::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fastRelabeling
  // --------------------------------------------------

  // default constructor
  fastRelabeling::fastRelabeling()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  fastRelabeling::fastRelabeling(const parameters& par)
    : modifier() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  fastRelabeling::fastRelabeling(const fastRelabeling& other)
    : modifier() {
    copy(other);
  }

  // destructor
  fastRelabeling::~fastRelabeling() {
  }

  // returns the name of this type
  const char* fastRelabeling::getTypeName() const {
    return "fastRelabeling";
  }

  // copy member
  fastRelabeling&
  fastRelabeling::copy(const fastRelabeling& other) {
    modifier::copy(other);
    return (*this);
  }

  // alias for copy member
  fastRelabeling&
  fastRelabeling::operator=(const fastRelabeling& other) {
    return (copy(other));
  }

  // clone member
  functor* fastRelabeling::clone() const {
    return new fastRelabeling(*this);
  }

  // return parameters
  const fastRelabeling::parameters&
  fastRelabeling::getParameters() const {
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
  bool fastRelabeling::apply(channel8& srcdest) const {
    channel8 dest;
    if (apply(srcdest,dest)) {
      if (srcdest.getMode() == channel8::Connected) {
        dest.detach(srcdest);
      } else {
        srcdest.fill(dest);
      }
      return true;
    }
    return false;
  };

  // On place apply for type imatrix!
  bool fastRelabeling::apply(imatrix& srcdest) const {
    imatrix dest;
    if (apply(srcdest,dest)) {
      if (srcdest.getMode() == imatrix::Connected) {
        dest.detach(srcdest);
      } else {
        srcdest.fill(dest);
      }
      return true;
    }
    dest.detach(srcdest);
    return false;
  };

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,channel8& dest) const {
    
    int numObjects;
    ivector equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,ubyte> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    relabelingHelper<ubyte,int> 
      ihelper(static_cast<ubyte>(max(0,par.minThreshold)),
              static_cast<ubyte>(min(255,par.maxThreshold)),
              par.assumeLabeledMask,
              par.fourNeighborhood);

    imatrix idest;

    if (ihelper.apply(src,idest,equivLabels,numObjects)) {
      if (par.sortSize) {
        ivector objSize;
        return (helper.relabelWithArea(equivLabels,numObjects,
                                       idest,dest,objSize) &&
                helper.sortLabels(par.minimumObjectSize,objSize,dest));
      } else {
        if (par.minimumObjectSize <= 1) {
          return helper.simpleRelabel(equivLabels,numObjects,idest,dest);
        } else {
          ivector objSize;
          return (helper.relabelWithArea(equivLabels,numObjects,
                                         idest,dest,objSize)&&
                  helper.suppress(par.minimumObjectSize,objSize,dest));
        }
      }
    }
    return false;
  };

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,imatrix& dest) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,int> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);
    
    if (helper.apply(src,dest,equivLabels,numObjects)) {
      if (par.sortSize) {
        ivector objSize;
        return (helper.relabelWithArea(equivLabels,numObjects,dest,objSize) &&
                helper.sortLabels(par.minimumObjectSize,objSize,dest));
      } else {
        if (par.minimumObjectSize <= 1) {
          return helper.simpleRelabel(equivLabels,numObjects,dest);
        } else {
          ivector objSize;
          return (helper.relabelWithArea(equivLabels,numObjects,dest,objSize)&&
                  helper.suppress(par.minimumObjectSize,objSize,dest));
        }
      }
    }

    return false;
  };

  // On copy apply for type imatrix!
  bool fastRelabeling::apply(const imatrix& src,imatrix& dest) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<int,int> helper(par.minThreshold,
                                     par.maxThreshold,
                                     par.assumeLabeledMask,
                                     par.fourNeighborhood);
    
    if (helper.apply(src,dest,equivLabels,numObjects)) {
      if (par.sortSize) {
        ivector objSize;
        return (helper.relabelWithArea(equivLabels,numObjects,dest,objSize) &&
                helper.sortLabels(par.minimumObjectSize,objSize,dest));
      } else {
        if (par.minimumObjectSize <= 1) {
          return helper.simpleRelabel(equivLabels,numObjects,dest);
        } else {
          ivector objSize;
          return (helper.relabelWithArea(equivLabels,numObjects,dest,objSize)&&
                  helper.suppress(par.minimumObjectSize,objSize,dest));
        }
      }
    }

    return false;
  };

  ///////////////////////


  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                             channel8& dest,
                             ivector& objSize) const {
    
    int numObjects;
    ivector equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,ubyte> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    relabelingHelper<ubyte,int> 
      ihelper(static_cast<ubyte>(max(0,par.minThreshold)),
              static_cast<ubyte>(min(255,par.maxThreshold)),
              par.assumeLabeledMask,
              par.fourNeighborhood);

    imatrix idest;

    if (ihelper.apply(src,idest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,idest,dest,objSize)) {
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest);
      }
    }
    return false;
  };

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                             imatrix& dest,
                             ivector& objSize) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,int> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    if (helper.apply(src,dest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,dest,objSize)) {
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest);
      }
    }

    return false;
  };

  // On copy apply for type imatrix!
  bool fastRelabeling::apply(const imatrix& src,
                             imatrix& dest,
                             ivector& objSize) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<int,int> helper(par.minThreshold,
                                     par.maxThreshold,
                                     par.assumeLabeledMask,
                                     par.fourNeighborhood);
    
    if (helper.apply(src,dest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,dest,objSize)) {
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest);
      }
    }

    return false;
  };

  ///////////////////////


  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                             channel8& dest,
                             ivector& objSize,
                             std::vector<areaPoints>& objects) const {
    
    int numObjects;
    ivector equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,ubyte> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    relabelingHelper<ubyte,int> 
      ihelper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    imatrix idest;

    if (ihelper.apply(src,idest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,idest,dest,objSize)) {
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest,objects);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest,objects);
      }
    }
    return false;
  };

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                             imatrix& dest,
                             ivector& objSize,
                             std::vector<areaPoints>& objects) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<ubyte,int> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(255,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);

    if (helper.apply(src,dest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,dest,objSize)) {
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest,objects);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest,objects);
      }
    }

    return false;
  };

  // On copy apply for type imatrix!
  bool fastRelabeling::apply(const imatrix& src,
                             imatrix& dest,
                             ivector& objSize,
                             std::vector<areaPoints>& objects) const {
    int numObjects;
    vector<int> equivLabels;
    const parameters& par = getParameters();

    relabelingHelper<int,int> helper(par.minThreshold,
                                     par.maxThreshold,
                                     par.assumeLabeledMask,
                                     par.fourNeighborhood);
    
    if (helper.apply(src,dest,equivLabels,numObjects) &&
        helper.relabelWithArea(equivLabels,numObjects,dest,objSize)) {

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 3)
      static viewer view("Mask before suppress");;
      view.show(dest);
      view.waitKey();
#endif
      if (par.sortSize) {
        return helper.sortLabels(par.minimumObjectSize,objSize,dest,objects);
      } else {
        return helper.suppress(par.minimumObjectSize,objSize,dest,objects);
      }
    }

    return false;
  };

  //////////////////////

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                                   channel8& dest,
                                   ivector& equivLabels,
                                   int& numObjects) const {
    
    const parameters& par = getParameters();

    relabelingHelper<ubyte,ubyte> helper(par.minThreshold,
                                         par.maxThreshold,
                                         par.assumeLabeledMask,
                                         par.fourNeighborhood);

    vector<ubyte> tmp;
    if (helper.apply(src,dest,tmp,numObjects)) {
      equivLabels.castFrom(tmp);
      return true;
    }
    return false;
  };

  // On copy apply for type channel8!
  bool fastRelabeling::apply(const channel8& src,
                                   imatrix& dest,
                                   ivector& equivLabels,
                                   int& numObjects) const {
    const parameters& par = getParameters();

    relabelingHelper<ubyte,int> helper(par.minThreshold,
                                       par.maxThreshold,
                                       par.assumeLabeledMask,
                                       par.fourNeighborhood);
    
    return helper.apply(src,dest,equivLabels,numObjects);
  };

  // On copy apply for type imatrix!
  bool fastRelabeling::apply(const imatrix& src, 
                                   imatrix& dest,
                                   ivector& equivLabels,
                                   int& numObjects) const {
    const parameters& par = getParameters();

    relabelingHelper<int,int> helper(par.minThreshold,
                                     par.maxThreshold,
                                     par.assumeLabeledMask,
                                     par.fourNeighborhood);
    
    return helper.apply(src,dest,equivLabels,numObjects);
  };

  //sort integer labels
  bool fastRelabeling::sortLabels(const int minSize, 
				  ivector& numPixels, 
				  ivector& reindex) const
  {
    const parameters& par = getParameters();

    relabelingHelper<int,int> helper(par.minThreshold,
				     par.maxThreshold,
				     par.assumeLabeledMask,
				     par.fourNeighborhood);
    
    return helper.sortLabels(minSize, numPixels, reindex);
  };
  
  // Just relabel the destination, but compute also the size of each object.
  bool fastRelabeling::relabelWithArea(const ivector& equivLabels,
				       const int numObjects,
				       imatrix& dest,
				       ivector& numPixels) const
  {
    const parameters& par = getParameters();
    
    relabelingHelper<int,int> helper(par.minThreshold,
				     par.maxThreshold,
				     par.assumeLabeledMask,
				     par.fourNeighborhood);
    
    return helper.relabelWithArea(equivLabels, numObjects, dest, numPixels);
  };
  
  /**
   * Just relabel the destination, but compute also the size of each object.
   */
  bool fastRelabeling::relabelWithArea(const vector<ubyte>& equivLabels,
				       const int numObjects,
				       channel8& dest,
				       ivector& numPixels) const
  {
    const parameters& par = getParameters();

    relabelingHelper<ubyte,ubyte> 
      helper(static_cast<ubyte>(max(0,par.minThreshold)),
             static_cast<ubyte>(min(0,par.maxThreshold)),
             par.assumeLabeledMask,
             par.fourNeighborhood);
    
    return helper.relabelWithArea(equivLabels, numObjects, dest, numPixels);
  };

}
