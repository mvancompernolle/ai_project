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
 * file .......: ltiMedianCut.cpp
 * authors ....: Norman Pfeil
 * organization: LTI, RWTH Aachen
 * creation ...: 18.5.2001
 * revisions ..: $Id: ltiMedianCut.cpp,v 1.9 2006/09/05 10:22:09 ltilib Exp $
 */

#include "ltiMedianCut.h"
#include "ltiUsePalette.h"
#include <set>

namespace lti {

  // --------------------------------------------------
  // medianCut::parameters
  // --------------------------------------------------

  // default constructor
  medianCut::parameters::parameters()
    : colorQuantization::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    preQuant = int(32);
  }

  // copy constructor
  medianCut::parameters::parameters(const parameters& other)
    : colorQuantization::parameters()  {
    copy(other);
  }

  // destructor
  medianCut::parameters::~parameters() {
  }

  // get type name
  const char* medianCut::parameters::getTypeName() const {
    return "medianCut::parameters";
  }

  // copy member

  medianCut::parameters&
    medianCut::parameters::copy(const parameters& other) {
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


    preQuant = other.preQuant;

    return *this;
  }

  // alias for copy member
  medianCut::parameters&
    medianCut::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* medianCut::parameters::clone() const {
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
  bool medianCut::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool medianCut::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"preQuant",preQuant);
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
  bool medianCut::parameters::write(ioHandler& handler,
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
  bool medianCut::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool medianCut::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"preQuant",preQuant);
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
  bool medianCut::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // medianCut
  // --------------------------------------------------

  // default constructor
  medianCut::medianCut()
    : colorQuantization(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  medianCut::medianCut(const medianCut& other)
    : colorQuantization()  {
    copy(other);
  }

  // destructor
  medianCut::~medianCut() {
  }

  // returns the name of this type
  const char* medianCut::getTypeName() const {
    return "medianCut";
  }

  // copy member
  medianCut&
    medianCut::copy(const medianCut& other) {
      colorQuantization::copy(other);
    return (*this);
  }

  // clone member
  functor* medianCut::clone() const {
    return (new medianCut(*this));
  }

  // return parameters
  const medianCut::parameters&
    medianCut::getParameters() const {
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


  // On place apply for type image
  bool medianCut::apply(image& srcdest) const {

    image tempImage;
    channel8 dummyMask;
    palette dummyPalette;
    bool success = false;
    success = performQuantization(srcdest, tempImage, dummyMask, dummyPalette);
    tempImage.detach(srcdest);
    return success;
  };

  // On copy apply for type image
  bool medianCut::apply(const image& src,image& dest) const {

    channel8 dummyMask;
    palette dummyPalette;
    return performQuantization(src, dest, dummyMask, dummyPalette);
  };

  // On copy apply for type channel8
  bool medianCut::apply(const image &src, channel8 &dest) const {

    image dummyImage;
    palette dummyPalette;
    return performQuantization(src, dummyImage, dest, dummyPalette);
  }

  // On copy apply for type channel8 with respective palette
  bool medianCut::apply(const image &src,
                        channel8 &dest,
                        palette &thePalette) const {

    image dummyImage;
    return performQuantization(src, dummyImage, dest, thePalette);
  };

  // On copy apply for type image with respective color palette
  bool medianCut::apply(const image& src ,
                        image& dest ,
                        palette& thePalette) const {

    channel8 dummyMask;
    return performQuantization(src, dest, dummyMask, thePalette);
  };

  // protected functions
  void medianCut::computeBoxInfo(const histogram& hist,
                                       boxInfo& theBox) const {

    // boxInfo.min and .max must be specified before entry and histogram
    // must be valid.  Missing information in boxInfo is computed (mean,
    // var, colorFrequency, colors) and box boundaries (min,max) are set
    // to the smallest size, that still encloses all entries in the
    // specified range of the histogram.

    int rLow=theBox.min.getRed();
    int gLow=theBox.min.getGreen();
    int bLow=theBox.min.getBlue();

    int rUp=theBox.max.getRed();
    int gUp=theBox.max.getGreen();
    int bUp=theBox.max.getBlue();

    int rMin=rUp,rMax=rLow,gMin=gUp,gMax=gLow,bMin=bUp,bMax=bLow;
    int i=rLow,j=gLow,k=bLow;
    int freq;
    ivector iVec(3);

    double meanR, meanG, meanB, meanSquareR, meanSquareG, meanSquareB;
    meanR = meanG = meanB = meanSquareR = meanSquareG = meanSquareB = 0;
    double accu = 0;

    theBox.colors = 0;
    for (i=rLow;i<=rUp;i++) {
      for (j=gLow;j<=gUp;j++) {
        for (k=bLow;k<=bUp;k++) {
          iVec[0]=i;
          iVec[1]=j;
          iVec[2]=k;
          if (hist.at(iVec)>0.0f) {
            if (k<bMin) {bMin=k;}
            if (k>bMax) {bMax=k;}
            if (j<gMin) {gMin=j;}
            if (j>gMax) {gMax=j;}
            if (i<rMin) {rMin=i;}
            if (i>rMax) {rMax=i;}

            freq = static_cast<const int>(hist.at(iVec));

            meanR += freq * i;
            meanG += freq * j;
            meanB += freq * k;

            meanSquareR += freq *i*i;
            meanSquareG += freq *j*j;
            meanSquareB += freq *k*k;

            accu  += freq;
            // Count number of distinct colors
            theBox.colors++;
          }
        }
      }
    }

    meanR /= accu;
    meanG /= accu;
    meanB /= accu;

    meanSquareR /= accu;
    meanSquareG /= accu;
    meanSquareB /= accu;

    // Set minimum and maximum enclosing bounds
    theBox.min.setRed(rMin);
    theBox.min.setGreen(gMin);
    theBox.min.setBlue(bMin);

    theBox.max.setRed(rMax);
    theBox.max.setGreen(gMax);
    theBox.max.setBlue(bMax);

    // Set number of entries inside box
    theBox.colorFrequency = static_cast<long int>(accu);

    // set mean and variance inside box
    theBox.mean[0] = meanR;
    theBox.mean[1] = meanG;
    theBox.mean[2] = meanB;

    theBox.var[0] = meanSquareR - meanR*meanR;
    theBox.var[1] = meanSquareG - meanG*meanG;
    theBox.var[2] = meanSquareB - meanB*meanB;
  }

  // Quantization takes place here!
  bool medianCut::performQuantization(const image& src,
                                      image& dest,
                                      channel8& mask,
                                      palette &thePalette) const {

    // parameters and const variables
    const parameters& param = getParameters();
    const int imageRows=src.rows();          // number of rows in src
    const int imageCols=src.columns();       // number of columns in src

    // resize destination containers
    dest.resize(imageRows,imageCols,rgbPixel(),false,false);
    mask.resize(imageRows,imageCols,ubyte(),false,false);

    // Variables
    int row,col;            // row, column counters
    int r,g,b;              // red,green,blue
    ivector iVec(3);        // int-vector

    std::list<boxInfo> theLeaves; // list of leaves (tree without root
                                  // and nodes)
    std::list<boxInfo>::iterator  splitPos;   // position to split
    std::list<boxInfo>::iterator  iter;       // iterator for theLeaves

    // create histogram with desired pre-quantization dimensions from src
    histogram theHist(3,param.preQuant);

    const float factor = param.preQuant/256.0f;

    for (row = 0 ; row < imageRows ; row++) {
      for (col = 0 ; col < imageCols ; col++) {

        r = static_cast<int>(src.at( row,col ).getRed()   * factor);
        g = static_cast<int>(src.at( row,col ).getGreen() * factor);
        b = static_cast<int>(src.at( row,col ).getBlue()  * factor);
        
        // insert point with quantized color
        dest.at(row,col).set((r*256+128)/param.preQuant,
                             (g*256+128)/param.preQuant,
                             (b*256+128)/param.preQuant,0); 

        iVec[0] = r;
        iVec[1] = g;
        iVec[2] = b;

        theHist.put(iVec);
      }
    }

    // initialization of first box of list (the whole histogram)
    boxInfo theBox(rgbPixel(0,0,0),
                   rgbPixel(param.preQuant-1,
                            param.preQuant-1,
                            param.preQuant-1));

    computeBoxInfo(theHist,theBox);

    // return, if desired number of colors smaller than colors in
    // pre-quantized image
    if (theBox.colors < param.numberOfColors) {

      thePalette.resize(theBox.colors,rgbPixel(),false,false);

      // prepare palette     
      int i = 0;
      for (r=0;r<param.preQuant;++r) {
        for (g=0;g<param.preQuant;++g) {
          for (b=0;b<param.preQuant;++b) {
            iVec[0] = r;
            iVec[1] = g;
            iVec[2] = b;
            if (theHist.at(iVec) > 0) {
              thePalette.at(i).set((r*256+128)/param.preQuant,
                                   (g*256+128)/param.preQuant,
                                   (b*256+128)/param.preQuant);
            }
          }
        }
      }

      // use the palette to generate the corresponding channel
      usePalette colorizer;
      colorizer.apply(dest,thePalette,mask);
      
      return true;
    }

    // Push first box into List
    theLeaves.push_back(theBox);

    // MAIN LOOP (do this until you have enough leaves (count), or no
    // splittable boxes (entries))
    int count, entries=1;  // auxiliary variables for the main loop
    for (count=1; (count<param.numberOfColors) && (entries!=0); count++) {

      // find box with largest number of entries from list
      entries = 0;
      for (iter = theLeaves.begin() ; iter != theLeaves.end() ; iter++) {
        if ( (*iter).colorFrequency > entries ) {
          // Avoid choosing single colors, i.e. unsplittable boxes
          if (  ((*iter).max.getRed()   > (*iter).min.getRed())   ||
                ((*iter).max.getGreen() > (*iter).min.getGreen()) ||
                ((*iter).max.getBlue()  > (*iter).min.getBlue()) ) {
            entries   = (*iter).colorFrequency;
            splitPos  = iter;
          }
        }
      }

      // A splittable box was found.
      // The iterator "splitPos" indicates its position in the List
      if (entries >0) {
        // Determine next axis to split (largest variance) and box dimensions
        int splitAxis;  // split axis indicator
        if ( ((*splitPos).var[0] >= (*splitPos).var[1]) &&
             ((*splitPos).var[0] >= (*splitPos).var[2]) ) {
          splitAxis = 0;  // red axis
        }
        else if ( (*splitPos).var[1] >= (*splitPos).var[2] ) {
          splitAxis = 1;  // green axis
        }
        else {
          splitAxis = 2;  // blue axis
        }

        int rMax  = ((*splitPos).max.getRed());
        int rMin  = ((*splitPos).min.getRed());
        int gMax  = ((*splitPos).max.getGreen());
        int gMin  = ((*splitPos).min.getGreen());
        int bMax  = ((*splitPos).max.getBlue());
        int bMin  = ((*splitPos).min.getBlue());

        // pass through box along the axis to split
        bool found;             // becomes true when split plane is found
        int nrOfCols=0;         // counter: number of colors of box
        int prevNrOfCols=0;     // forerunner of nrOfCols
        rgbPixel lower1;   // lower pixel from box 1
        rgbPixel upper1;   // upper pixel from box 1
        rgbPixel lower2;   // lower pixel from box 2
        rgbPixel upper2;   // upper pixel from box 2

        switch (splitAxis) {
          case 0: // red axis
            nrOfCols = 0;
            for (r = rMin , found = false ; (!found) && (r<=rMax) ; r++) {
              prevNrOfCols = nrOfCols;
              for (g = gMin ; g <= gMax ; g++) {
                for (b=bMin;b<=bMax;b++) {
                  iVec[0] = r;
                  iVec[1] = g;
                  iVec[2] = b;
                  if (theHist.at(iVec) > 0.0) {
                    nrOfCols += static_cast<long int>(theHist.at(iVec));
                  }
                }
              }
              if ( nrOfCols >= (*splitPos).colorFrequency/2 ) {
                found=true;
              }
            }
            if (fabs(prevNrOfCols -
                     static_cast<float>((*splitPos).colorFrequency)/2) <
                fabs(nrOfCols     -
                     static_cast<float>((*splitPos).colorFrequency)/2)) {
                r--;
                nrOfCols = prevNrOfCols;
            }
            // first box
            lower1.setRed(rMin);  lower1.setGreen(gMin);  lower1.setBlue(bMin);
            upper1.setRed(r-1);   upper1.setGreen(gMax);  upper1.setBlue(bMax);
            // second box
            lower2.setRed(r);     lower2.setGreen(gMin);  lower2.setBlue(bMin);
            upper2.setRed(rMax);  upper2.setGreen(gMax);  upper2.setBlue(bMax);
            break;

          case 1: // g axis
            nrOfCols = 0;
            for (g = gMin , found = false ; (!found) && (g<=gMax) ; g++) {
              prevNrOfCols = nrOfCols;
              for (r = rMin ; r <= rMax ; r++) {
                for (b = bMin ; b <= bMax ; b++) {
                  iVec[0] = r;
                  iVec[1] = g;
                  iVec[2] = b;
                  if (theHist.at(iVec) > 0.0) {
                    nrOfCols += static_cast<long int>(theHist.at(iVec));
                  }
                }
              }
              if ( nrOfCols >= (*splitPos).colorFrequency/2 ) {
                found=true;
              }
            }
            if (fabs(prevNrOfCols -
                     static_cast<float>((*splitPos).colorFrequency)/2) <
                fabs(nrOfCols     -
                     static_cast<float>((*splitPos).colorFrequency)/2)) {
                g--;
                nrOfCols = prevNrOfCols;
            }
            // first box
            lower1.setRed(rMin);  lower1.setGreen(gMin);  lower1.setBlue(bMin);
            upper1.setRed(rMax);  upper1.setGreen(g-1);   upper1.setBlue(bMax);
            // second box
            lower2.setRed(rMin);  lower2.setGreen(g);     lower2.setBlue(bMin);
            upper2.setRed(rMax);  upper2.setGreen(gMax);  upper2.setBlue(bMax);
            break;

          case 2: // b axis
            nrOfCols = 0;
            for (b = bMin , found = false ; (!found) && (b<=bMax) ; b++) {
              prevNrOfCols = nrOfCols;
              for (r = rMin ; r <= rMax ; r++) {
                for (g = gMin ; g <= gMax ; g++) {
                  iVec[0] = r;
                  iVec[1] = g;
                  iVec[2] = b;
                  if (theHist.at(iVec) > 0.0) {
                    nrOfCols += static_cast<long int>(theHist.at(iVec));
                  }
                }
              }
              if ( nrOfCols >= (*splitPos).colorFrequency/2 ) {
                found=true;
              }
            }
            if (fabs(prevNrOfCols -
                     static_cast<float>((*splitPos).colorFrequency)/2) <
                fabs(nrOfCols     -
                     static_cast<float>((*splitPos).colorFrequency)/2)) {
                b--;
                nrOfCols = prevNrOfCols;
            }
            // first box
            lower1.setRed(rMin); lower1.setGreen(gMin); lower1.setBlue(bMin);
            upper1.setRed(rMax); upper1.setGreen(gMax); upper1.setBlue(b-1);
            // second box
            lower2.setRed(rMin); lower2.setGreen(gMin); lower2.setBlue(b);
            upper2.setRed(rMax); upper2.setGreen(gMax); upper2.setBlue(bMax);
            break;
          default:
            break;
        } // end of switch

        // compute box info of new boxes and
        // append both at the end of list
        theBox.min = lower1;
        theBox.max = upper1;
        computeBoxInfo(theHist,theBox);
        theLeaves.push_back(theBox);

        theBox.min = lower2;
        theBox.max = upper2;
        computeBoxInfo(theHist,theBox);
        theLeaves.push_back(theBox);

        // delete splited box from list
        theLeaves.erase(splitPos);
      }

    } // end of for (MAIN LOOP)

    // compute block histogram and respective color palette
    thePalette.resize(theLeaves.size());
    int i;
    for (iter = theLeaves.begin() , i=0 ;
         iter != theLeaves.end() ;
         iter++ , i++) {
      // misuse histogram as a look-up-table
      for (r = (*iter).min.getRed(); r <= (*iter).max.getRed(); r++) {
        for (g = (*iter).min.getGreen(); g <= (*iter).max.getGreen(); g++) {
          for (b = (*iter).min.getBlue(); b <= (*iter).max.getBlue(); b++) {
            iVec[0] = r;
            iVec[1] = g;
            iVec[2] = b;
            theHist.at(iVec) = i; // insert palette-index (refers to
                                  // color in palette)
          }
        }
      }

      // create palette
      r = (static_cast<int>((*iter).mean[0]*factor)*256+128)/param.preQuant;
      g = (static_cast<int>((*iter).mean[1]*factor)*256+128)/param.preQuant;
      b = (static_cast<int>((*iter).mean[2]*factor)*256+128)/param.preQuant;
      thePalette[i].set(r,g,b,0);  // insert color
    }

    // create new image with palette and theHist
    dest.resize(imageRows,imageCols);
    mask.resize(imageRows,imageCols,0,false,true);

    // <= 256 colors? then also fill the mask
    if (thePalette.size() <= 256) {
      for (row = 0 ; row < imageRows ; row++) {
        for (col = 0 ; col < imageCols ; col++) {
          iVec[0] = static_cast<int>(src.at( row,col ).getRed()   * factor);
          iVec[1] = static_cast<int>(src.at( row,col ).getGreen() * factor);
          iVec[2] = static_cast<int>(src.at( row,col ).getBlue()  * factor);

          i = static_cast<int>(theHist.at( iVec ));

          dest.at(row,col) = thePalette[i];// insert point with quantized color
          mask.at(row,col) = i;     // insert palette index of quantized color
        }
      }
    }
    else {
      for (row = 0 ; row < imageRows ; row++) {
        for (col = 0 ; col < imageCols ; col++) {
          iVec[0] = static_cast<int>(src.at( row,col ).getRed()   * factor);
          iVec[1] = static_cast<int>(src.at( row,col ).getGreen() * factor);
          iVec[2] = static_cast<int>(src.at( row,col ).getBlue()  * factor);

          i = static_cast<int>(theHist.at( iVec ));

          r = thePalette[i].getRed();
          g = thePalette[i].getGreen();
          b = thePalette[i].getBlue();

          dest.at(row,col).set(r,g,b,0); // insert point with quantized color
        }
      }
    }

    return true;
  }

}
