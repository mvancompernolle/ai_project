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
 * file .......: ltiMeanShiftSegmentation.cpp
 * authors ....: Axel Berner,Jens Rietzschel
 * organization: LTI, RWTH Aachen
 * creation ...: 22.6.2001
 * revisions ..: $Id: ltiMeanShiftSegmentation.cpp,v 1.21 2008/08/29 20:29:02 alvarado Exp $
 */

#include "ltiMeanShiftSegmentation.h"
#include "ltiSTLIoInterface.h"
#include <ctime> 
#include <cstring> // for memcpy and similar functions

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include <iomanip>
#endif

namespace lti {

  // --------------------------------------------------
  // meanShiftSegmentation
  // --------------------------------------------------

  // static const attributes
  const int meanShiftSegmentation::FIRST_SIZE = 262144; // 2^18
  const int meanShiftSegmentation::SEC_SIZE = 64;     //2^6
  const int meanShiftSegmentation::nonColor = 255;


  // --------------------------------------------------
  // meanShiftSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  meanShiftSegmentation::parameters::parameters()
    : segmentation::parameters() {

    option = Undersegmentation;
    rects.resize(1,rectangle(0,0,0,0));
    maxTrial = 10;
    trial2converge = 15;
    classThreshold[Quantization] = 2.5;
    classThreshold[Oversegmentation] = 5.0;
    classThreshold[Undersegmentation] = 10.0;
    maxTrialRandomColor = 25;
    minRegionSize = 15;
    // nonAuto
    rectRadius[Quantization] = 8.0;
    rectRadius[Oversegmentation] = 6.0;
    rectRadius[Undersegmentation] = 4.0;
    // auto
    autoRadius[Quantization] = 2;
    autoRadius[Oversegmentation] = 3;
    autoRadius[Undersegmentation] = 4;
    minVar = 0.0f;
    multivariateNormalKernel=false;
    sigmaS = 5;
    sigmaR = 5;
    thresholdConverged = 0.1;
    speedup = parameters::MediumSpeedup;
    classicAlgorithm = true;
    maxNeighbourColorDistance = 3;
  }

  // copy constructor
  meanShiftSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  meanShiftSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* meanShiftSegmentation::parameters::getTypeName() const {
    return "meanShiftSegmentation::parameters";
  }

  // copy member
  meanShiftSegmentation::parameters&
  meanShiftSegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif
    int i;
    option = other.option;
    rects = other.rects;
    maxTrial = other.maxTrial;
    trial2converge = other.trial2converge;
    for( i=0; i<3; i++)
      classThreshold[i] = other.classThreshold[i];
    maxTrialRandomColor = other.maxTrialRandomColor;
    minRegionSize = other.minRegionSize;
    for( i=0; i<3; i++)
      rectRadius[i] = other.rectRadius[i];
    for( i=0; i<3; i++)
      autoRadius[i] = other.autoRadius[i];
    minVar = other.minVar;
    multivariateNormalKernel=other.multivariateNormalKernel;
    sigmaS=other.sigmaS;
    sigmaR=other.sigmaR;
    thresholdConverged=other.thresholdConverged;
    speedup = other.speedup;
    classicAlgorithm = other.classicAlgorithm;
    maxNeighbourColorDistance = other.maxNeighbourColorDistance;
    return *this;
  }

  // alias for copy member
  meanShiftSegmentation::parameters&
  meanShiftSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* meanShiftSegmentation::parameters::clone() const {
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
  bool meanShiftSegmentation::parameters::write(ioHandler& handler,
                                                const bool complete) const
# else
    bool meanShiftSegmentation::parameters::writeMS(ioHandler& handler,
                                                    const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {      
      switch(option) {
        case Quantization:
          lti::write(handler,"option","Quantization");
          break;
        case Oversegmentation:
          lti::write(handler,"option","Oversegmentation");
          break;
        case Undersegmentation:
          lti::write(handler,"option","Undersegmentation");
          break;
      }
      lti::write(handler,"rects",rects);
      lti::write(handler,"maxTrial",maxTrial);
      lti::write(handler,"trial2converge",trial2converge);
      lti::write(handler,"classThreshold[Quantization]",
                 classThreshold[Quantization]);
      lti::write(handler,"classThreshold[Oversegmentation]",
                 classThreshold[Oversegmentation]);
      lti::write(handler,"classThreshold[Undersegmentation]",
                 classThreshold[Undersegmentation]);
      lti::write(handler,"maxTrialRandomColor",maxTrialRandomColor);
      lti::write(handler,"minRegionSize",minRegionSize);
      lti::write(handler,"rectRadius[Quantization]",
                 rectRadius[Quantization]);
      lti::write(handler,"rectRadius[Oversegmentation]",
                 rectRadius[Oversegmentation]);
      lti::write(handler,"rectRadius[Undersegmentation]",
                 rectRadius[Undersegmentation]);
      lti::write(handler,"autoRadius[Quantization]",
                 autoRadius[Quantization]);
      lti::write(handler,"autoRadius[Oversegmentation]",
                 autoRadius[Oversegmentation]);
      lti::write(handler,"autoRadius[Undersegmentation]",
                 autoRadius[Undersegmentation]);
      lti::write(handler,"minVar",minVar);
      lti::write(handler,"multivariateNormalKernel",multivariateNormalKernel);
      lti::write(handler,"sigmaS",sigmaS);
      lti::write(handler,"sigmaR",sigmaR);
      lti::write(handler,"thresholdConverged",thresholdConverged);
      switch(speedup) {
        case NoSpeedup:
          lti::write(handler,"speedup","NoSpeedup");
          break;
        case MediumSpeedup:
          lti::write(handler,"speedup","MediumSpeedup");
          break;
        case HighSpeedup:
          lti::write(handler,"speedup","HighSpeedup");
          break;
      }
      lti::write(handler,"classicAlgorithm",classicAlgorithm);
      lti::write(handler,"maxNeighbourColorDistance",
                 maxNeighbourColorDistance);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool
      (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool meanShiftSegmentation::parameters::write(ioHandler& handler,
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
  bool meanShiftSegmentation::parameters::read(ioHandler& handler,
                                               const bool complete)
# else
    bool meanShiftSegmentation::parameters::readMS(ioHandler& handler,
                                                   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
      if (!b) {
        handler.setStatusString("Error reading begin");
      }
    }

    if (b) {
      std::string str;

      b = lti::read(handler,"option",str) && b;

      if (str=="0" || str=="Quantization") {
        option=Quantization;
      } else if (str=="1" || str=="Oversegmentation") {
        option=Oversegmentation;
      } else if (str=="2" || str=="Undersegmentation") {
        option=Undersegmentation;
      } else {
        option=Oversegmentation;
      }

      b = lti::read(handler,"rects",rects) && b;
      b = lti::read(handler,"maxTrial",maxTrial) && b;
      b = lti::read(handler,"trial2converge",trial2converge) && b;
      b = lti::read(handler,"classThreshold[Quantization]",
                classThreshold[Quantization]) && b;
      b = lti::read(handler,"classThreshold[Oversegmentation]",
                classThreshold[Oversegmentation]) && b;
      b = lti::read(handler,"classThreshold[Undersegmentation]",
                classThreshold[Undersegmentation]) && b;
      b = lti::read(handler,"maxTrialRandomColor",maxTrialRandomColor) && b; 
      b = lti::read(handler,"minRegionSize",minRegionSize) && b;

      b = lti::read(handler,"rectRadius[Quantization]",
                rectRadius[Quantization]) && b;
      b = lti::read(handler,"rectRadius[Oversegmentation]",
                rectRadius[Oversegmentation]) && b;
      b = lti::read(handler,"rectRadius[Undersegmentation]",
                rectRadius[Undersegmentation]) && b;
      b = lti::read(handler,"autoRadius[Quantization]",
                autoRadius[Quantization]) && b;
      b = lti::read(handler,"autoRadius[Oversegmentation]",
                autoRadius[Oversegmentation]) && b;
      b = lti::read(handler,"autoRadius[Undersegmentation]",
                autoRadius[Undersegmentation]) && b;

      b = lti::read(handler,"minVar",minVar) && b;
      
      b = lti::read(handler,"multivariateNormalKernel",
                multivariateNormalKernel) && b;
      b = lti::read(handler,"sigmaS",sigmaS) && b;
      b = lti::read(handler,"sigmaR",sigmaR) && b;
      
      b = lti::read(handler,"thresholdConverged",thresholdConverged) && b;
      b = lti::read(handler,"speedup",str) && b;

      if (str.find("No") != std::string::npos) {
        option=NoSpeedup;
      } else if (str.find("Medium") != std::string::npos) {
        option=MediumSpeedup;
      } else if (str.find("High") != std::string::npos) {
        option=HighSpeedup;
      }
      
      b = lti::read(handler,"classicAlgorithm",classicAlgorithm) && b;
      b = lti::read(handler,"maxNeighbourColorDistance",
                    maxNeighbourColorDistance) && b;
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }


# ifdef _LTI_MSC_6
  bool meanShiftSegmentation::parameters::read(ioHandler& handler,
                                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // meanShiftSegmentation
  // --------------------------------------------------

  // default constructor
  meanShiftSegmentation::meanShiftSegmentation()
    : segmentation() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    // initialize all members
    numOrgColors = 0;
    _col0=_col1=_col2=0;
    mapPxlToColor = 0;
    mapRemPxl = 0;
    numRemPxl = 0;
    mapRemColors = 0;
    howManyEachRemColor = 0;
    numRemColors = 0;
  }

  meanShiftSegmentation::meanShiftSegmentation(const parameters& par)
    : segmentation() {

    // set the default parameters
    setParameters(par);

    // initialize all members
    numOrgColors = 0;
    _col0=_col1=_col2=0;
    mapPxlToColor = 0;
    mapRemPxl = 0;
    numRemPxl = 0;
    mapRemColors = 0;
    howManyEachRemColor = 0;
    numRemColors = 0;
  }

  // copy constructor
  meanShiftSegmentation::meanShiftSegmentation(const 
                                               meanShiftSegmentation& other)
    : segmentation() {
    copy(other);
  }

  // destructor
  meanShiftSegmentation::~meanShiftSegmentation() {
  }

  // returns the name of this type
  const char* meanShiftSegmentation::getTypeName() const {
    return "meanShiftSegmentation";
  }

  // copy member
  meanShiftSegmentation&
  meanShiftSegmentation::copy(const meanShiftSegmentation& other) {
    segmentation::copy(other);
    return (*this);
  }

  // clone member
  functor* meanShiftSegmentation::clone() const {
    return new meanShiftSegmentation(*this);
  }

  // return parameters
  const meanShiftSegmentation::parameters&
  meanShiftSegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void range(int a, int& x, int b) {
    if(x < a)
      x = a;
    else if(x > b)
      x = b;
  }


  // --------------------------------------------------
  // the old implementation of meanShiftSegmentation
  // --------------------------------------------------

  // LUV (final_T[]) to RGB (TI[]) conversion
  void meanShiftSegmentation::luv2rgb( luvPixel& final_T, lti::rgbPixel& TI) {
    // Constants for LUV transformation
    //static const float     Xn = 0.9505;
    static const float     Yn = 1.0;
    //static const float     Zn = 1.0888;
    static const float     Un_prime = 0.1978f;
    static const float     Vn_prime = 0.4683f;
    static const float  RGB[3][3] = { { 3.2405f, -1.5371f, -0.4985f },
                                      {-0.9693f,  1.8760f,  0.0416f },
                                      { 0.0556f, -0.2040f,  1.0573f } };

    if(final_T.l<0.1) { // lightness
      TI = rgbPixel(0,0,0);
    }
    else {
      float my_x, my_y, my_z;
      if(final_T.l< 8.0f)
        my_y = Yn * final_T.l / 903.3f;
      else
        my_y = Yn * pow((final_T.l + 16.0f) / 116.0f, 3.0f);

      float u_prime = final_T.u / (13 * final_T.l) + Un_prime;
      float v_prime = final_T.v / (13 * final_T.l) + Vn_prime;

      my_x = 9 * u_prime * my_y / (4 * v_prime);
      my_z = (12 - 3 * u_prime - 20 * v_prime) * my_y / (4 * v_prime);

      int TI0 =int((RGB[0][0]*my_x + RGB[0][1]*my_y + RGB[0][2]*my_z)*255.0);
      int TI1 =int((RGB[1][0]*my_x + RGB[1][1]*my_y + RGB[1][2]*my_z)*255.0);
      int TI2 =int((RGB[2][0]*my_x + RGB[2][1]*my_y + RGB[2][2]*my_z)*255.0);

      range(0,TI0,255);
      range(0,TI1,255);
      range(0,TI2,255);

      TI = rgbPixel((unsigned char)TI0,(unsigned char)TI1,(unsigned char)TI2);
    }
  }

  // RGB to LUV conversion
  // To gain speed the conversion works on a table of colors (_col_RGB[])
  // rather than on the whole image
  void meanShiftSegmentation::convert_RGB_LUV(const lti::image signal,
                                              const int _col_RGB[],
                                              const int _col_misc[]) {
    _lti_debug("convert_RGB_LUV() begin" << std::endl);

    int j;
    static const int   MAXV = 256;
    static const float Lt = 0.008856f;
    // Coefficient matrix for xyz and rgb spaces
    static const int   XYZ[3][3] = { { 4125, 3576, 1804 },
                                     { 2125, 7154,  721 },
                                     {  193, 1192, 9502 } };
    static const float Un_prime = 0.1978f;
    static const float Vn_prime = 0.4683f;

    // the first time these LUT are empty
    static const int *A00 = 0;
    static const int *A01 = 0;
    static const int *A02 = 0;

    static const int *A10 = 0;
    static const int *A11 = 0;
    static const int *A12 = 0;

    static const int *A20 = 0;
    static const int *A21 = 0;
    static const int *A22 = 0;

    static const float *my_pow = 0;

    // if LUT still uninitialized
    if (isNull(A00)) {
      // initialized LUT
      int *tA00 = new int[MAXV];
      int *tA01 = new int[MAXV];
      int *tA02 = new int[MAXV];

      int *tA10 = new int[MAXV];
      int *tA11 = new int[MAXV];
      int *tA12 = new int[MAXV];

      int *tA20 = new int[MAXV];
      int *tA21 = new int[MAXV];
      int *tA22 = new int[MAXV];

      for(j=0; j<MAXV;j++) {
        tA00[j]=XYZ[0][0]*j; tA01[j]=XYZ[0][1]*j; tA02[j]=XYZ[0][2]*j;
        tA10[j]=XYZ[1][0]*j; tA11[j]=XYZ[1][1]*j; tA12[j]=XYZ[1][2]*j;
        tA20[j]=XYZ[2][0]*j; tA21[j]=XYZ[2][1]*j; tA22[j]=XYZ[2][2]*j;
      }

      A00=tA00; A01=tA01; A02=tA02;
      A10=tA10; A11=tA11; A12=tA12;
      A20=tA20; A21=tA21; A22=tA22;

      float* tmy_pow = new float[MAXV];
      for (j=0; j<MAXV;j++) {
        tmy_pow[j]= 116.0f * pow(j/255.0f, 0.3333333f) - 16;
      }

      my_pow = tmy_pow;
    }

    _lti_debug("  numOrgColors=" << numOrgColors << std::endl);

    _col0 = new int[numOrgColors];
    _col1 = new int[numOrgColors];
    _col2 = new int[numOrgColors];
    mapPxlToColor = new int[numImgPxl];

    _lti_debug("  first loop..." << std::endl);

    float l_star, u_star, v_star;
    float u_prime, v_prime;
    int temp_index(0), temp_ind;
    int x, y, z, my_temp;
    int R,G,B;
    for ( j = 0; j < numOrgColors; j++) {
      R = (_col_RGB[j]>>16) & 0xFF;
      G = (_col_RGB[j]>> 8) & 0xFF;
      B = (_col_RGB[j]    ) & 0xFF;

      x = A00[R] + A01[G] + A02[B];
      y = A10[R] + A11[G] + A12[B];
      z = A20[R] + A21[G] + A22[B];

      float  tval = y / 2550000.0f; //Yn==1
      if ( tval >  Lt)
        l_star = my_pow[(int)(tval*255+0.5f)];
      else
        l_star = 903.3f * tval;

      my_temp = x + 15 * y + 3 * z;
      if (my_temp) {
        u_prime = (float)(x << 2) / (float)(my_temp);
        v_prime = (float)(9 * y) / (float)(my_temp);
      }
      else {
        u_prime=4.0f;
        v_prime=9.0f/15.0f;
      }

      tval = 13*l_star;
      u_star = tval* (u_prime - Un_prime); // Un_prime = 0.1978
      v_star = tval* (v_prime - Vn_prime); // Vn_prime = 0.4683

      _col0[j] = iround(l_star);
      _col1[j] = iround(u_star);
      _col2[j] = iround(v_star);
    }

    _lti_debug("  second loop..." << std::endl);

    int k, temp_col;
    for(j=0;j<numImgPxl;j++) {
      temp_col = (signal.at(j).getValue() & 0x00FFFFFF);
      temp_ind = _col_misc[temp_col>>6];
      for( k=temp_ind;k<temp_ind+SEC_SIZE;k++)
        if(_col_RGB[k]==temp_col) {
          temp_index=mapPxlToColor[j]=k;
          break;
        }
      chL.at(j)=_col0[temp_index];
      chU.at(j)=_col1[temp_index];
      chV.at(j)=_col2[temp_index];
    }

    _lti_debug("  second loop end..." << std::endl);

    //     these LUT are now static, to save some time
    //     delete [] my_pow;
    //     delete [] A22; delete [] A21; delete [] A20;
    //     delete [] A12; delete [] A11; delete [] A10;
    //     delete [] A02; delete [] A01; delete [] A00;
    //     my_pow = 0;
    //     A22=A21=A20=A12=A11=A10=A02=A01=A00=0;

    _lti_debug("convert_RGB_LUV() end" << std::endl);
  }

  // Dummy conversion of the three channels into LUV conversion
  // To gain speed the conversion works on a table of colors (_col_RGB[])
  // rather than on the whole image
  void meanShiftSegmentation::convert_RGB_LUV(const lti::channel8& chnl1,
                                              const lti::channel8& chnl2,
                                              const lti::channel8& chnl3,
                                              const int _col_RGB[],
                                              const int _col_misc[]) {
    _lti_debug("convert_RGB_LUV() begin" << std::endl);

    int j;

    _lti_debug("  numOrgColors=" << numOrgColors << std::endl);

    _col0 = new int[numOrgColors];
    _col1 = new int[numOrgColors];
    _col2 = new int[numOrgColors];
    mapPxlToColor = new int[numImgPxl];

    _lti_debug("  first loop..." << std::endl);

    uint32 value;
    int temp_index(0), temp_ind;
    int L,U,V;

    for ( j = 0; j < numOrgColors; j++) {

      value = _col_RGB[j];

      L = (value >> 16) & 0xFF;
      U = (value >>  8) & 0xFF;
      V = (value      ) & 0xFF;

      _col0[j] = L;
      _col1[j] = U;
      _col2[j] = V;
    }

    _lti_debug("  second loop..." << std::endl);

    int k, temp_col;
    for(j=0;j<numImgPxl;j++) {
      value = (int(chnl1.at(j))<<16)+(int(chnl2.at(j))<<8)+(int(chnl3.at(j)));
      temp_col = value;
      temp_ind = _col_misc[temp_col>>6];
      for( k=temp_ind;k<temp_ind+SEC_SIZE;k++)
        if(_col_RGB[k]==temp_col) {
          temp_index=mapPxlToColor[j]=k;
          break;
        }
      chL.at(j)=_col0[temp_index];
      chU.at(j)=_col1[temp_index];
      chV.at(j)=_col2[temp_index];
    }

    _lti_debug("  second loop end..." << std::endl);

    _lti_debug("convert_RGB_LUV() end" << std::endl);
  }


  // 3-D Histogram computation
  // Implement a trade-off between speed and required memory
  void meanShiftSegmentation::buildHistogram(const lti::image& signal,
                                             int* _col_RGB[],
                                             int* _m_colors[],
                                             int* _col_misc[]) {

    _lti_debug("buildHistogram() begin" << std::endl);

    int *first_tab= new int[FIRST_SIZE]; // FIRST_SIZE == 2^18
    memset(first_tab,0,sizeof(int)*FIRST_SIZE);
    *_col_misc= new int[FIRST_SIZE];
    memset(*_col_misc,0,sizeof(int)*FIRST_SIZE);

    int k,j,p,r;
    int temp_ind, sec_ind, third_ind;
    //first_tab -> how many
    for(k=0;k<numImgPxl;k++) {
      temp_ind = (signal.at(k).getValue() & 0x00FFFFFF)>>6; // 18 bits left
      first_tab[temp_ind]++;
    }

    //_col_misc -> memo position
    int first_contor=0;
    for(k=0;k<FIRST_SIZE;k++)
      if(first_tab[k]) {
        (*_col_misc)[k]=first_contor;
        first_contor++;
      }
    //contors
    int* fourth_tab = new int[first_contor];
    memset(fourth_tab,0,sizeof(int)*first_contor);

    //tab of pointers to reduced colors
    int** third_tab = new int *[first_contor];
    first_contor=0;
    for(k=0;k<FIRST_SIZE;k++)
      if(first_tab[k]) {
        third_tab[first_contor]=new int[first_tab[k]];
        first_contor++;
      }

    for(k=0;k<numImgPxl;k++) {
      temp_ind = (signal.at(k).getValue() & 0x00FFFFFF)>>6;
      sec_ind  = signal.at(k).getValue() & 0x3F;
      third_ind=(*_col_misc)[temp_ind];
      third_tab[third_ind][fourth_tab[third_ind]]=sec_ind;
      fourth_tab[third_ind]++;
    }

    numOrgColors=0;
    int *fifth_tab=new int[SEC_SIZE];
    for(k=0;k<first_contor;k++) {
      memset(fifth_tab,0,sizeof(int)*SEC_SIZE);
      for(j=0;j<fourth_tab[k];j++)
        fifth_tab[third_tab[k][j]]++;
      for(j=0;j<SEC_SIZE;j++)
        if(fifth_tab[j])
          numOrgColors++;
    }
    *_col_RGB=new int[numOrgColors];
    *_m_colors=new int[numOrgColors];

    k=0;p=0;
    for(r=0;r<FIRST_SIZE;r++)
      if(first_tab[r]) {
        memset(fifth_tab,0,sizeof(int)*SEC_SIZE);
        for(j=0;j<fourth_tab[k];j++)
          fifth_tab[third_tab[k][j]]++;
        (*_col_misc)[r]=p;
        for(j=0;j<SEC_SIZE;j++)
          if(fifth_tab[j]) {
            (*_col_RGB)[p]=(r<<6)+j;
            (*_m_colors)[p]=fifth_tab[j];
            p++;
          }
        delete [] third_tab[k];
        third_tab[k]=0;
        k++;
      }
    delete [] third_tab;
    third_tab=0;
    delete [] fourth_tab;
    fourth_tab=0;
    delete [] fifth_tab;
    fifth_tab=0;
    delete [] first_tab;
    first_tab=0;

    _lti_debug("buildHistogram() end" << std::endl);

  }

  // Histogram
  // 3-D Histogram computation
  // Implement a trade-off between speed and required memory
  void meanShiftSegmentation::buildHistogram(const channel8& chnl1,
                                             const channel8& chnl2,
                                             const channel8& chnl3,
                                             int* _col_RGB[],
                                             int* _m_colors[],
                                             int* _col_misc[]) {

    _lti_debug("buildHistogram() begin" << std::endl);

    int *first_tab= new int[FIRST_SIZE]; // FIRST_SIZE == 2^18
    memset(first_tab,0,sizeof(int)*FIRST_SIZE);
    *_col_misc= new int[FIRST_SIZE];
    memset(*_col_misc,0,sizeof(int)*FIRST_SIZE);

    int k,j,p,r;
    int temp_ind, sec_ind, third_ind;

    image signal;
    signal.resize(chnl1.size(),Black,false,false);
    uint32 value;

    //first_tab -> how many
    for(k=0;k<numImgPxl;k++) {
      value = (int(chnl1.at(k))<<16)+(int(chnl2.at(k))<<8)+int(chnl3.at(k));
      signal.at(k).setValue(value);
      temp_ind = value >> 6; // 18 bits left
      first_tab[temp_ind]++;
    }

    //_col_misc -> memo position
    int first_contor=0;
    for(k=0;k<FIRST_SIZE;k++) {
      if(first_tab[k]) {
        (*_col_misc)[k]=first_contor;
        first_contor++;
      }
    }

    //contors
    int* fourth_tab = new int[first_contor];
    memset(fourth_tab,0,sizeof(int)*first_contor);

    //tab of pointers to reduced colors
    int** third_tab = new int *[first_contor];
    first_contor=0;
    for(k=0;k<FIRST_SIZE;k++)
      if(first_tab[k]) {
        third_tab[first_contor]=new int[first_tab[k]];
        first_contor++;
      }

    for(k=0;k<numImgPxl;k++) {
      value = signal.at(k).getValue();
      temp_ind = value >> 6;
      sec_ind  = value & 0x3F;
      third_ind=(*_col_misc)[temp_ind];
      third_tab[third_ind][fourth_tab[third_ind]]=sec_ind;
      fourth_tab[third_ind]++;
    }

    numOrgColors=0;
    int *fifth_tab=new int[SEC_SIZE];
    for(k=0;k<first_contor;k++) {
      memset(fifth_tab,0,sizeof(int)*SEC_SIZE);
      for(j=0;j<fourth_tab[k];j++)
        fifth_tab[third_tab[k][j]]++;
      for(j=0;j<SEC_SIZE;j++)
        if(fifth_tab[j])
          numOrgColors++;
    }

    *_col_RGB=new int[numOrgColors];
    *_m_colors=new int[numOrgColors];

    k=0;p=0;
    for(r=0;r<FIRST_SIZE;r++)
      if(first_tab[r]) {
        memset(fifth_tab,0,sizeof(int)*SEC_SIZE);
        for(j=0;j<fourth_tab[k];j++)
          fifth_tab[third_tab[k][j]]++;
        (*_col_misc)[r]=p;
        for(j=0;j<SEC_SIZE;j++)
          if(fifth_tab[j]) {
            (*_col_RGB)[p]=(r<<6)+j;
            (*_m_colors)[p]=fifth_tab[j];
            p++;
          }
        delete [] third_tab[k];
        third_tab[k]=0;
        k++;
      }
    delete [] third_tab;
    third_tab=0;
    delete [] fourth_tab;
    fourth_tab=0;
    delete [] fifth_tab;
    fifth_tab=0;
    delete [] first_tab;
    first_tab=0;

    _lti_debug("buildHistogram() end" << std::endl);
  }

  // Update numRemPxl, mapRemPxl[], numRemColors, mapRemColors[],
  // howManyEachRemColor[]
  void meanShiftSegmentation::updateTables(const bool boolSelectedPxls[])
  {
    int i;
    int temp_contor = numRemPxl;
    numRemPxl=0;
    for(i=0;i<temp_contor;i++)
      if(!boolSelectedPxls[mapRemPxl[i]])
        mapRemPxl[numRemPxl++]=mapRemPxl[i]; // only points with
                                             // boolSelectedPxls[false] are
                                             // remaining

    memset(mapRemColors,0,sizeof(int)*numRemColors);
    memset(howManyEachRemColor,0,sizeof(int)*numOrgColors);
    numRemColors=0;
    for(i=0;i<numRemPxl;i++)
      // how many points of each color remaining
      howManyEachRemColor[mapPxlToColor[mapRemPxl[i]]]++;
    for(i=0;i<numOrgColors;i++)
      if(howManyEachRemColor[i]) {//if the color is still remaining
        mapRemColors[numRemColors]=i; //put it in the map of remaining colors
        numRemColors++; //increment remaining colors
      }
  }

  void meanShiftSegmentation::add_neigh(bool boolSelectedPxls[],
                                        int selectedPxls[],
                                        int& numSelectedPxl) {
    int how_many(0); // if more than "how_many" neighbours, consider the point
    switch (getParameters().option) {
      case parameters::Quantization: // quantize
        how_many = 2; break;
      case parameters::Oversegmentation: // oversegmentaion
        how_many = 1; break;
      case parameters::Undersegmentation: // undersegmentaion
        how_many = 0; break;
    }
    int idxRemPxl,j,p,k,i;
    int temp_contor = numSelectedPxl;
    for ( p = 0; p < numRemPxl; p++ ) {
      idxRemPxl= (auto_segm ? mapRemPxl[p]:p);// i is index of rem point
      if(!boolSelectedPxls[idxRemPxl]) {// only if rem point isnt already
                                        // selected
                                        // (=boolSelectedPxls[idxRemPxl]==true)
        int neigh_contor = 0;
        bool enoughNeigh = false; 
        for(j=0;j<8;j++) {
          // k is index of neighbour of not selected point
          k=idxRemPxl + neigh[j];

          // if k within image and converges to T[rect] 
          if(k>=0 && k<numImgPxl && boolSelectedPxls[k]) {
            if(auto_segm  && (mapAssignedLabels[k] != nonColor))
              // ignore k if it already converges to a previous(another) color
              // of T[rect] than actual color
              continue; 
            neigh_contor++;//number of neighbours converging to T[rect]
            if(neigh_contor>how_many) {//enough neighbours are converging to
                                       //T[rect] and still have
              enoughNeigh = true;      //noncolor, so this one also gets
                                       //T[rect] assigned
              break;
            }
          }
        }
        if(enoughNeigh) {
          if(auto_segm)
            selectedPxls[numSelectedPxl] = idxRemPxl;// select neighbour point
                                                     // i for color T[rect]
          numSelectedPxl++;
        }
      }
    }
    for(i=temp_contor;i<numSelectedPxl;i++)
      boolSelectedPxls[selectedPxls[i]]=true;// give all selectedPxls
                                             // neighbours color T[rect]
  }

  // the actual meanshift procedure: compute mean color of the points within
  // sphere of RADIUS and center final_T in color space

  // Find the feature vectors inside the given window
  // Use Improved Absolute Error Inequality Criterion
  // when computing Euclidean distance
  // See J.S.Pan el al, Fast Clustering Alg. for VQ, Pattern Recognition,
  // Vol. 29, No. 3, pp. 511-518, 1996
  void meanShiftSegmentation::new_auto_loop(luvPixel& final_T,
                                            bool selectedColors[]) {
    
    const parameters& param = getParameters();
    
    if(param.multivariateNormalKernel){
      float L,U,V,Arg;
      int TT0=0, TT1=0, TT2=0;
      int local_contor=0,k;
      
      for ( int p = 0; p < numRemColors; p++ ) {
        k = mapRemColors[p];
  
        L = _col0[k]-final_T.l;
        U = _col1[k]-final_T.u;
        V = _col2[k]-final_T.v;

        Arg=(L*L+U*U+V*V)/RADIUS;

        TT0+=_col0[k]*howManyEachRemColor[k];
        TT1+=_col1[k]*howManyEachRemColor[k];
        TT2+=_col2[k]*howManyEachRemColor[k];
        local_contor += howManyEachRemColor[k];

      }
    }
    else{
      float L,U,V,R;
      int TT0=0, TT1=0, TT2=0;
      int local_contor=0;
      float RADIUS_S2 = sqrt(2)*RADIUS,
        RADIUS_S3 = sqrt(3)*RADIUS;
      int k;
      
      for ( int p = 0; p < numRemColors; p++ ) {
        k = mapRemColors[p];
    
        L = _col0[k]-final_T.l; if( (R =abs(L)) >= RADIUS    ) continue;
        U = _col1[k]-final_T.u; if( (R+=abs(U)) >= RADIUS_S2 ) continue;
        V = _col2[k]-final_T.v; if( (R+=abs(V)) >= RADIUS_S3 ) continue;
        if( L*L+U*U+V*V < RADIUS2 ) {
          TT0+=_col0[k]*howManyEachRemColor[k];
          TT1+=_col1[k]*howManyEachRemColor[k];
          TT2+=_col2[k]*howManyEachRemColor[k];
          local_contor += howManyEachRemColor[k];
          selectedColors[k] = true;
        }
      }
      final_T.l=(float)TT0/(float)local_contor;
      final_T.u=(float)TT1/(float)local_contor;
      final_T.v=(float)TT2/(float)local_contor;
    }
  }

  void meanShiftSegmentation::calcRepresColorAuto(int selectedPxls[],
                                                  bool boolSelectedPxls[],
                                                  luvPixel& T,
                                                  int& numSelectedPxl) {
    int i;
    luvPixel final_T = T;

    bool *selectedColors = new bool[numOrgColors];// these colors converge in
                                                  // color space to the
    //same local maximum because they are touched by the moving sphere??

    float L,U,V;
    int trial=0;
    int maxTrialToConverge = getParameters().trial2converge;
    bool underLimit = false;
    while(trial++ < maxTrialToConverge && !underLimit) {
      memset(selectedColors,0,numOrgColors);
      new_auto_loop(final_T,selectedColors);
      L=T.l-final_T.l;
      U=T.u-final_T.u;
      V=T.v-final_T.v;
      T = final_T;
      if( (L*L+U*U+V*V) < 0.1 )
        underLimit = true;
    }

#   ifdef _LTI_DEBUG
#   if _LTI_DEBUG > 5
    if (!underLimit)
      _lti_debug("if this message is freq. shown - change trial2converge\n");
#   endif
#   endif

    numSelectedPxl=0;
    memset(boolSelectedPxls,false,numImgPxl);
    int p;
    for(i=0;i<numRemPxl;i++) {// remaining points which have not yet a new
                              // color assigned
      p = mapRemPxl[i];// index of a remaining point
      if(selectedColors[mapPxlToColor[p]]) {// if sel_col.at(color-index of
                                            // remaining point)==true
        selectedPxls[numSelectedPxl++] = p;// indices of points that have
                                           // color T[rect] assigned
        boolSelectedPxls[p] = true;// true means that color of point p is T
                                   // that is T[rect]
      }
    }
    delete [] selectedColors;
    selectedColors=0;
  }

  // The same as above, but for non auto_segmentation
  void meanShiftSegmentation::nauto_loop(luvPixel& final_T,
                                         int selectedPxls[],
                                         bool boolSelectedPxls[],
                                         int& numSelectedPxl) {
    float L,U,V,R;
    float RADIUS_S2 = sqrt(2)*RADIUS;
    float RADIUS_S3 = sqrt(3)*RADIUS;

    for ( int k = 0; k < numSubimgPxl; k++ ) {
      L = final_T.l-chLcurr.at(k); if( (R =abs(L))>=RADIUS    ) continue;
      U = final_T.u-chUcurr.at(k); if( (R+=abs(U))>=RADIUS_S2 ) continue;
      V = final_T.v-chVcurr.at(k); if( (R+=abs(V))>=RADIUS_S3 ) continue;
      if( L*L+U*U+V*V < RADIUS2 ) {
        selectedPxls[numSelectedPxl++] = k;
        boolSelectedPxls[k] = true;
      }
    }
    meanColor(numSelectedPxl, selectedPxls, final_T);
  }

  void meanShiftSegmentation::calcRepresColorNAuto(int selectedPxls[],
                                                   bool boolSelectedPxls[],
                                                   luvPixel& T,
                                                   int& numSelectedPxl) {
    luvPixel final_T = T;
    float L,U,V;
    int gen_contor=0;
    int t2c = getParameters().trial2converge;
    while(gen_contor++<t2c) {
      numSelectedPxl=0;
      memset(boolSelectedPxls,false,numImgPxl);
      nauto_loop(final_T, selectedPxls, boolSelectedPxls,numSelectedPxl);
      L=T.l-final_T.l;
      U=T.u-final_T.u;
      V=T.v-final_T.v;
      T=final_T;
      if( (L*L+U*U+V*V) < 0.1 )
        return;
    }
  }

  // Set the Radius of the window
  void meanShiftSegmentation::setRadius(const double r) {
    RADIUS  = static_cast<float>(r);
    RADIUS2 = static_cast<float>(r*r);
  }

  // Test if the clusters have the same mean
  bool meanShiftSegmentation::testSameCluster(int rect, luvPixel T[]) {
    float L,U,V;
    for( int k=0; k<rect;k++) {
      L=T[k].l-T[rect].l;
      U=T[k].u-T[rect].u;
      V=T[k].v-T[rect].v;
      if( L*L+U*U+V*V < 1 )
        return true;
    }
    return false;
  }

  void meanShiftSegmentation::try2getPalEntry(const luvPixel T[],
                                              const int numColors) {
    float L,U,V,RAD2, min_RAD2;
    int min_ind;
    int i,k,u;
    int pres_class;

    // search for each pixel the best color in the palette(T),
    // but it's different must be under RADIUS2, else nonColor
    if(auto_segm) setRadius(fixRadius[0]);
    else          setRadius(fixRadius[sizeof(getParameters().rectRadius)-1]);
    for(k=0;k<numImgPxl;k++) {
      min_RAD2 = 1.0e+20f;
      min_ind = 0;
      for(i=0;i<numColors;i++) {
        L=chL.at(k)-T[i].l; if(abs(L)>=RADIUS) continue;
        U=chU.at(k)-T[i].u;
        V=chV.at(k)-T[i].v;
        RAD2 = L*L+U*U+V*V;
        if( RAD2<min_RAD2 ) {
          min_RAD2 = RAD2;
          min_ind = i;
        }
      }
      if(min_RAD2<RADIUS2)
        mapAssignedLabels[k] = min_ind;
      else
        mapAssignedLabels[k] = nonColor;
    }

    // search for all nonColor pixel the best palette entry,
    // for this take a neighbour
    if(auto_segm) setRadius(fixRadius[0]*1.26);
    else          setRadius(fixRadius[0]);
    for(k=0; k<numImgPxl; k++)
      if( mapAssignedLabels[k] == nonColor )
        for(i=0;i<8;i++) {
          u=k+neigh[i];
          if(u>=0 && u<numImgPxl)
            if((pres_class=mapAssignedLabels[u]) != nonColor) {
              L=chL.at(k)-T[pres_class].l;
              U=chU.at(k)-T[pres_class].u;
              V=chV.at(k)-T[pres_class].v;
              if( L*L+U*U+V*V < RADIUS2 )
                mapAssignedLabels[k] = pres_class;
            }
        }
  }

  // search for each pixel the best color in the palette(T)
  void meanShiftSegmentation::getBestPalEntry(const luvPixel T[],
                                              const int numColors) {
    float L,U,V,RAD2, min_RAD2;
    int min_ind;
    int i,k;

    for(k=0;k<numImgPxl;k++) {
      min_RAD2=1.0e+20f;
      min_ind=0;
      for(i=0;i<numColors;i++) {
        L=chL.at(k)-T[i].l;
        U=chU.at(k)-T[i].u;
        V=chV.at(k)-T[i].v;
        RAD2=L*L+U*U+V*V;
        if(RAD2<min_RAD2) {
          min_RAD2=RAD2;
          min_ind=i;
        }
      }
      mapAssignedLabels[k] = min_ind;
    }
  }

  // Compute the mean of feature vectors mapped into the same color
  void meanShiftSegmentation::newPalette(luvPixel T[], const int numColors) {
    int *tab_contor = new int[numColors];

    int i;
    for(i=0;i<numColors;i++) {
      T[i].l=0.0;
      T[i].u=0.0;
      T[i].v=0.0;
      tab_contor[i] = 0;
    }
    int prez_class;
    for(int k=0;k<numImgPxl;k++) {
      prez_class = mapAssignedLabels[k];
      if( prez_class != nonColor ) {
        T[prez_class].l+=chL.at(k);
        T[prez_class].u+=chU.at(k);
        T[prez_class].v+=chV.at(k);
        tab_contor[prez_class]++;
      }
    }
    for(i=0;i<numColors;i++) {
      T[i].l/=tab_contor[i];
      T[i].u/=tab_contor[i];
      T[i].v/=tab_contor[i];
    }
    delete [] tab_contor;
    tab_contor=0;
  }

  void 
  meanShiftSegmentation::updateGenclass(int& numColors,
                                        const genericVector<bool>& valid_class,
                                        luvPixel T[]) {
    ubyte my_map[256];
    int u=0, j;

    // update T
    for(j=0;j<numColors;j++) {
      if( valid_class[j] ) {
        T[u].l = T[j].l;
        T[u].u = T[j].u;
        T[u].v = T[j].v;
        my_map[j] = u;
        u++;
      }
      else
        my_map[j] = nonColor;
    }
    // for the points, which are already assign to the non-color-cluster
    my_map[nonColor] = nonColor;
    // update numColors
    numColors = u;
    // update mapAssignedLabels
    for(j=0;j<numImgPxl;j++)
      mapAssignedLabels[j]=my_map[mapAssignedLabels[j]];
  }

  // Eliminate colors that have only regions less than "act_threshold"
  // connected pixels
  void meanShiftSegmentation::eliminateSmallClasses( int& numColors,
                                                     luvPixel T[]) {
    int j;
    int *my_max_region = new int[256];
    memset(my_max_region,0,sizeof(int)*(256));
    region *firstRegion = createRegionList(my_max_region);
    destroyRegionList(firstRegion);

    genericVector<bool> valid_class(numColors);
    int trueCounter = 0;
    for(j=0; j<numColors; j++) {
      if(my_max_region[j]<act_threshold)
        valid_class[j] = false;
      else {
        valid_class[j] = true;
        trueCounter++;
      }
    }
    delete [] my_max_region;
    my_max_region=0;
    if( trueCounter >= 2 ) {
      // if there is an "invalid" class in mapAssignedLabels, T
      // and more than two valid classes ->
      // it will be set to the invalid class(nonColor) and T,n_rect will
      // be updated
      updateGenclass(numColors,valid_class,T);
    }
  }

  // Eliminate regions with less than "my_lim" pixels
  void meanShiftSegmentation::eliminateRegions(const luvPixel T[],
                                               const int my_lim) {
    int j,u,k,p, pres_class, min_ind(0);
    int* region;
    float L,U,V,RAD2,minRAD2;
    int increm;

    // create RegionList
    meanShiftSegmentation::region *firstRegion = createRegionList(NULL);
    meanShiftSegmentation::region *current_region = firstRegion;
    while(current_region != NULL) {
      if(current_region->my_contor < my_lim) {
        setRadius(fixRadius[0]);
        increm = 4;
        region = current_region->my_region;
        for(k=0; k<current_region->my_contor; k++)
          mapAssignedLabels[region[k]] = nonColor;

        bool changes = true;
        while( changes ) {
          changes = false;
          RADIUS += increm;
          RADIUS2 = RADIUS*RADIUS;
          increm += 4;
          for(k=0; k<current_region->my_contor; k++)
            if(mapAssignedLabels[p=region[k]] == nonColor) {
              minRAD2 = RADIUS2;
              // search the neigh with the most similar color and
              // adopt its class
              for(j=1;j<8;j+=2) {
                u=p+neigh[j];
                if(u>=0 && u<numImgPxl)
                  if((pres_class=mapAssignedLabels[u]) != nonColor) {
                    L=chL.at(p)-T[pres_class].l; // todo chL oder chLcurr ?
                    U=chU.at(p)-T[pres_class].u;
                    V=chV.at(p)-T[pres_class].v;
                    RAD2=L*L+U*U+V*V;
                    if(RAD2<minRAD2) {
                      minRAD2=RAD2;
                      min_ind=pres_class;
                    }
                  }
              }
              if(minRAD2<RADIUS2)
                mapAssignedLabels[p] = min_ind;
              changes = true;
            }
        }
      }
      current_region=current_region->next_region_str;
    }
    destroyRegionList(firstRegion);
  }

  // Destroy the region list
  void meanShiftSegmentation::destroyRegionList(region *firstRegion) {
    meanShiftSegmentation::region *currentRegion = firstRegion;
    while(true) {
      delete [] currentRegion->my_region;
      currentRegion->my_region = 0;
      firstRegion = currentRegion;
      if(currentRegion->next_region_str) {
        currentRegion=currentRegion->next_region_str;
        delete firstRegion;
        firstRegion = 0;
      }
      else {
        delete firstRegion;
        firstRegion = 0;
        break;
      }
    }
  }

  // Connected component main routine
  void meanShiftSegmentation::find_other_neigh(const int k,
                                               int ptr_tab[],
                                 meanShiftSegmentation::region *current_region,
                                               int conn_selected[],
                                               bool taken[]) {

    int i, u;
    int region_contor = current_region->my_contor;
    int region_class = current_region->my_class;
    int contor = 0;

    int j = k; // j =start-point for neigh-search
    while(true) {
      for(i=1;i<9;i+=2) {
        u = j + neigh[i]; // possible neigh-point
        if(u>=0 && u<numImgPxl)
          if( mapAssignedLabels[u]==region_class && !taken[u] ) {
            taken[u] = true;
            conn_selected[region_contor++] = u;
            ptr_tab[contor++] = u; //buffer for neighbour points
          }
      }
      if( contor > 0 )
        j = ptr_tab[--contor];
      else
        break;
    }
    current_region->my_contor = region_contor;
  }

  meanShiftSegmentation::region*
  meanShiftSegmentation::createRegionList(int *my_max_region) {
    bool change_type = (my_max_region==NULL ? false:true );
    region *first_region, *prev_region, *current_region;
    first_region = prev_region = current_region = 0;

    // contains all the points from a region
    int *conn_selected = new int [numImgPxl];

    // used only in find_other_neigh(pervents always new alloc)
    int *ptr_tab       = new int [numImgPxl];

    // point is assigned to a region
    bool *taken        = new bool[numImgPxl];

    memset(taken, false, numImgPxl);
    int local_label=0;

    for(int k=0;k<numImgPxl;k++)
      if( !taken[k] ) {
        current_region = new region;
        current_region->my_contor = 0;
        current_region->my_class  = mapAssignedLabels[k];
        current_region->my_label  = local_label;
        if(k!=0)
          prev_region->next_region_str = current_region;
        else // first Region-Element
          first_region = current_region;

        local_label++;
        conn_selected[current_region->my_contor++] = k;
        taken[k] = true;
        find_other_neigh(k, ptr_tab, current_region, conn_selected, taken);

        if( change_type )
          if (current_region->my_contor >
              my_max_region[current_region->my_class] )
            my_max_region[current_region->my_class] =
              current_region->my_contor;

        current_region->my_region = new int[current_region->my_contor];
        memcpy(current_region->my_region,
               conn_selected,
               sizeof(int)*current_region->my_contor);

        prev_region = current_region;
      }
    current_region->next_region_str = NULL;

    delete [] ptr_tab;
    ptr_tab=0;
    delete [] taken;
    taken=0;
    delete [] conn_selected;
    conn_selected=0;

    return first_region;
  }

  // Cut a rectangle from the entire input data
  void meanShiftSegmentation::cutRectangle(const rectangle& rect ) {
    chLcurr.copy(chL,rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
    chUcurr.copy(chU,rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
    chVcurr.copy(chV,rect.ul.y,rect.br.y,rect.ul.x,rect.br.x);
    int rcolms = chLcurr.columns();
    neigh_sub[0]= -rcolms-1;  neigh_sub[1]= -rcolms;
    neigh_sub[2]= -rcolms+1;  neigh_sub[3]= +1;
    neigh_sub[4]= +rcolms+1;  neigh_sub[5]= +rcolms;
    neigh_sub[6]= +rcolms-1;  neigh_sub[7]= -1;
  }

  // Compute the mean of N points given by J[]
  void meanShiftSegmentation::meanColor(const int N, int J[], luvPixel& T) {
    int i;
    T.l = 0;
    T.u = 0;
    T.v = 0;
    for (i = 0; i < N; i++ ) {
      T.l += chLcurr.at(J[i]);
      T.u += chUcurr.at(J[i]);
      T.v += chVcurr.at(J[i]);
    }
    T.l /= N;
    T.u /= N;
    T.v /= N;
  }

  // Build a subsample set of 9 points
  void meanShiftSegmentation::subsample(luvPixel& Xmean) {
    int pointList[9];
    int numOfPoints=0, uj, i0, i;

    if(auto_segm)
      i0 = pointList[numOfPoints++]
        = mapRemPxl[int(double(numRemPxl-1)*rand()/RAND_MAX)];
    else
      i0 = pointList[numOfPoints++]
        = int(double(numSubimgPxl-1)*rand()/RAND_MAX);
    for( i=0;i<8;i++) {
      uj = i0 + neigh_sub[i];
      if ((uj>=0) && (uj<numSubimgPxl)) { // point uj in image
        if (auto_segm && (mapAssignedLabels[uj] != nonColor))// ??
          {}
        else
          pointList[numOfPoints++] = uj;
      }
    }
    meanColor(numOfPoints, pointList, Xmean);
  }

  // Sampling routine with all needed tests
  bool 
  meanShiftSegmentation::my_sampling( luvPixel& T, int maxTrialRandomColor) {
    int  k, p;
    float L,U,V;
    luvPixel Xmean;

    //N = max number of subsample sets
    int c, cMax = 0;
    for( int l=0; l<maxTrialRandomColor; l++ ) {
      // find a new color inside the rect
      subsample(Xmean);// mean color of 9-neighbourhood of random point within
                       // rectangle
      // Compute the square residuals (Euclid dist.)
      c=0;
      if(auto_segm){
        for ( p = 0; p < numRemColors; p++ ) {
          k=mapRemColors[p];
          L=_col0[k]-Xmean.l; if(abs(L)>=RADIUS) continue;
          U=_col1[k]-Xmean.u; if(abs(U)>=RADIUS) continue;
          V=_col2[k]-Xmean.v; if(abs(V)>=RADIUS) continue;
          if( L*L+U*U+V*V < RADIUS2 )
            c+=howManyEachRemColor[k];
          // count how many points are within a sphere of RADIUS centered at
          // Xmean in color space
        }
      }
      else {
        for ( k = 0; k < numSubimgPxl; k++ ) {
          L=Xmean.l-chLcurr.at(k); if(abs(L)>=RADIUS) continue;
          U=Xmean.u-chUcurr.at(k); if(abs(U)>=RADIUS) continue;
          V=Xmean.v-chVcurr.at(k); if(abs(V)>=RADIUS) continue;
          if( L*L+U*U+V*V < RADIUS2 )
            c++;
        }
      }
      //  Choose color with the highest density in color space
      if( c > cMax ) {
        cMax = c;
        T = Xmean;
      }
    }

    if (cMax == 0){ // Not enough points
      return false;
    }
    return true;
  }

  // Compute the weighted covariance of N points
  void meanShiftSegmentation::covariance_w(int _m_colors[],
                                           luvPixel& mean,
                                           luvPixel& var) {
    int j;
    // mean
    mean.l =  mean.u = mean.v  = 0.0;
    for ( j = 0; j < numOrgColors; j++ ) {
      mean.l += _m_colors[j]*_col0[j];
      mean.u += _m_colors[j]*_col1[j];
      mean.v += _m_colors[j]*_col2[j];
    }
    mean.l /= numImgPxl;
    mean.u /= numImgPxl;
    mean.v /= numImgPxl;
    //  variance
    var.l =  var.u = var.v = 0.0;
    for ( j = 0; j < numOrgColors; j++ ) {
      var.l += _m_colors[j]*(_col0[j]-mean.l)*(_col0[j]-mean.l);
      var.u += _m_colors[j]*(_col1[j]-mean.u)*(_col1[j]-mean.u);
      var.v += _m_colors[j]*(_col2[j]-mean.v)*(_col2[j]-mean.v);
    }
    var.l /= numImgPxl;
    var.u /= numImgPxl;
    var.v /= numImgPxl;
  }

  // Init matrices parameters
  void meanShiftSegmentation::init_matr(int _m_colors[] ) {
    _lti_debug("init_matr() begin" << std::endl);

    const lti::meanShiftSegmentation::parameters& param = getParameters();
    int i;

    // General statistic parameters for X.
    luvPixel mean;  //sample mean of X
    luvPixel var;   //sample covariance matrix of X
    covariance_w( _m_colors, mean, var);

    if(auto_segm) {
      const int option = param.option;
      fixRadius.resize(1);
      fixRadius[0] = param.autoRadius[option]
        *max(param.minVar,sqrt((var.l+var.u+var.v)/100));
      act_threshold = int(numImgPxl*0.001*param.classThreshold[option]);
    }
    else {
      const int sizeRR = 3;
      fixRadius.resize(sizeRR);
      for(i=0;i<sizeRR;i++)
        fixRadius[i]=param.rectRadius[i]
          *max(param.minVar,sqrt((var.l+var.u+var.v)/100));
      act_threshold = -1;
    }

    _lti_debug("  Radius: " << fixRadius[0] << std::endl);
    _lti_debug("init_matr() end" << std::endl);

  }

  // Init
  void meanShiftSegmentation::initializations(const lti::image& src,
                                              std::vector<rectangle>& rects) {

    _lti_debug("initializations() begin" << std::endl);

    int i;
    const parameters& param = getParameters();
    numImgPxl = src.rows() * src.columns();

    // ch* are matrix<int> attributes of this class
    chL.resize(src.size());
    chU.resize(src.size());
    chV.resize(src.size());

    int colms = chL.columns();
    neigh[0]= -colms-1;  neigh[1]= -colms;
    neigh[2]= -colms+1;  neigh[3]= +1;
    neigh[4]= +colms+1;  neigh[5]= +colms;
    neigh[6]= +colms-1;  neigh[7]= -1;

    srand( (unsigned)time(0) ); // starts random generator 29254088

    int* _m_colors = 0;     //how many of each color
    int* _col_RGB = 0;      //colors in the image (RGB)
    int* _col_misc = 0;     //misc use in histogram and conversion color
                            //indices of every image-pixel
    buildHistogram( src, &_col_RGB, &_m_colors, &_col_misc); //build just the
                                                             //three
                                                             //histograms
    convert_RGB_LUV( src, _col_RGB, _col_misc);//chL,chU,chV and
                                               //_col0,_col1,_col2,
                                               //mapPxlToColor are filled
    delete [] _col_RGB;
    _col_RGB = 0;
    delete [] _col_misc;
    _col_misc = 0;
    init_matr(_m_colors); // set fixRadius and act_threshold

    //image represented as vector containing indices of color palette
    mapAssignedLabels = new ubyte[numImgPxl];

    //whole image initialized with nonColor(255)
    memset(mapAssignedLabels,nonColor,numImgPxl);

    //autosegment if first rectangle is 0
    auto_segm = param.rects.empty() || (param.rects[0]==rectangle(0,0,0,0));

    if(auto_segm) {
      // auto-segmentation
      
      numRemPxl = numImgPxl;     
      numRemColors = numOrgColors; 
      mapRemPxl = new int[numImgPxl];
      for ( i = 0; i< numImgPxl ; i++ )
        mapRemPxl[i] = i;    
      mapRemColors = new int[numOrgColors];
      for ( i = 0; i< numOrgColors ; i++ )
        mapRemColors[i] = i;    
      howManyEachRemColor = new int[numOrgColors]; 
      memcpy(howManyEachRemColor, _m_colors,sizeof(int)*numOrgColors);

      // initialize howManyEachRemColor with _m_colors (how many of each
      // color)
      rects.resize(0); // clear old stuff without releasing the memory
      int rectSize = (param.rects.size() <= 1) ? 50 : param.rects.size();
      rects.resize(rectSize,rectangle(0,0,src.lastColumn(),src.lastRow()));
      //50 (if unchanged) rectangles with size of original image
    }
    else {
      numRemPxl=0; // no remaining points
      rects = param.rects; //keep user set vector of rectangles
    }
    delete [] _m_colors;
    _m_colors=0;

    _lti_debug("initializations() end" << std::endl);

  }


  // Init
  void meanShiftSegmentation::initializations(const lti::channel8& chnl1,
                                              const lti::channel8& chnl2,
                                              const lti::channel8& chnl3,
                                              std::vector<rectangle>& rects) {

    _lti_debug("initializations() begin" << std::endl);

    int i;
    const parameters& param = getParameters();
    numImgPxl = chnl1.rows() * chnl1.columns();

    // ch* are matrix<int> attributes of this class
    chL.resize(chnl1.size());
    chU.resize(chnl2.size());
    chV.resize(chnl3.size());

    const int colms = chL.columns();
    neigh[0]= -colms-1;  neigh[1]= -colms;
    neigh[2]= -colms+1;  neigh[3]= +1;
    neigh[4]= +colms+1;  neigh[5]= +colms;
    neigh[6]= +colms-1;  neigh[7]= -1;

    srand( (unsigned)time(0) ); // starts random generator 29254088

    int* _m_colors = 0;     //how many of each color
    int* _col_RGB = 0;      //colors in the image (RGB)
    int* _col_misc = 0;     //misc use in histogram and conversion
    buildHistogram(chnl1,chnl2,chnl3, &_col_RGB, &_m_colors, &_col_misc);
    convert_RGB_LUV(chnl1,chnl2,chnl3, _col_RGB, _col_misc);

    delete [] _col_RGB;
    _col_RGB = 0;
    delete [] _col_misc;
    _col_misc = 0;
    init_matr(_m_colors);

    mapAssignedLabels = new ubyte[numImgPxl];
    memset(mapAssignedLabels,nonColor,numImgPxl);

    //autosegment if first rectangle is 0
    auto_segm = param.rects.empty() || (param.rects[0]==rectangle(0,0,0,0));

    if(auto_segm) {
      numRemPxl = numImgPxl;
      numRemColors = numOrgColors;
      mapRemPxl = new int[numImgPxl];
      for ( i = 0; i< numImgPxl ; i++ )
        mapRemPxl[i] = i;
      mapRemColors = new int[numOrgColors];
      for ( i = 0; i< numOrgColors ; i++ )
        mapRemColors[i] = i;
      howManyEachRemColor = new int[numOrgColors];
      memcpy(howManyEachRemColor, _m_colors,sizeof(int)*numOrgColors);

      int rectSize = (param.rects.size() <= 1) ? 50 : param.rects.size();
      rects.resize(0);
      rects.resize(rectSize,rectangle(0,0,chnl1.lastColumn(),chnl1.lastRow()));
    }
    else {
      numRemPxl=0;
      rects = param.rects;
    }
    delete [] _m_colors;
    _m_colors=0;

    _lti_debug("initializations() end" << std::endl);

  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool meanShiftSegmentation::apply(const lti::image& src, lti::image& dest) {

    // get parameters
    const parameters& param = getParameters();

    if(param.classicAlgorithm) {
      //      lti::channel8 dest8;
      lti::imatrix dest8;
      lti::palette colorMap;
      if( apply(src, dest8, colorMap) ) {
        const int maxIdx = colorMap.size();
        int i;
        // ensure a connected matrix
        dest.resize(0,0,lti::Black,false,false);
        dest.resize(src.size(),lti::Red,false,true);
        // copy the palette colors into dest
        for(int k=0; k<numImgPxl; k++) {
          i=dest8.at(k);
          if( i<maxIdx ) {
            dest.at(k) = colorMap.at(i);
          }
        }
        return true;
      }
      return false;
    }

    else{// new implementation
      lti::image imgFiltered;
      lti::palette colorMap;
      lti::imatrix labels;
      return apply(src,imgFiltered,dest,labels,colorMap);
    }
  }

  bool meanShiftSegmentation::apply(const lti::image& src,
                                    lti::imatrix& dest) {
    // get parameters
    const parameters& param = getParameters();

    if(param.classicAlgorithm) {
      lti::palette colorMap;
      return apply(src, dest, colorMap);
    }
    else{// new implementation
      lti::image imgFiltered,imgSegmented;
      lti::palette colorMap;
      return apply(src,imgFiltered,imgSegmented,dest,colorMap);
    }
  }


  bool meanShiftSegmentation::apply(const lti::image& src,
                                    lti::imatrix& labels,
                                    lti::palette& colorMap) {

    // get parameters
    const parameters& param = getParameters();

    if(param.classicAlgorithm) {
      
      if (src.empty()) {
        setStatusString("Input image empty");
        labels.clear();
        colorMap.clear();
        return false;
      }

      _lti_debug("Entering apply!\n");
      
      int failedTrials = 0;
      int k;
      // max trials todo better description
      int maxTrialRandomColor = param.maxTrialRandomColor;
      
      std::vector<rectangle> rects;
      initializations(src, rects);

      luvPixel* luvPalSegm;  //luv palette of segmented image
      // stores color for each rect (resp. luvPalette)
      luvPalSegm = new luvPixel[rects.size()];

      unsigned int rect = 0;
      bool breakup = false;
      while ( !breakup && rect<rects.size() ) {
        numSubimgPxl = rects[rect].getArea();
        // copy all pixel from chL,chU,chV ,
        // which are in rects[rect], to chLcurr, chUcurr,...
        cutRectangle( rects[rect] );
  
        setRadius(fixRadius[0]);
      
        //try to get a color by my_sampling(), when failing  maxTrial times
        while( !breakup && 
               !my_sampling( luvPalSegm[rect], maxTrialRandomColor) ) {

          failedTrials++;
          if(auto_segm)
            maxTrialRandomColor=1;
          if(failedTrials>=param.maxTrial)
            breakup = true;
        }

        if (!breakup) {
          // indices of points that change color to luvPalSegm[rect]
          int *selectedPxls = new int[numImgPxl];
          // true if point gets new color luvPalSegm[rect] assigned
          bool *boolSelectedPxls = new bool[numImgPxl];
          int numSelectedPxl; // number of touched points
          if(auto_segm) {
            setRadius(fixRadius[0]);// just set RADIUS and RADIUS2

            // find color of local maximum in color space starting at
            // luvPalSegm[rect] and find the points, which colors converge in
            // color space to the local maximum
            calcRepresColorAuto(selectedPxls, 
                                boolSelectedPxls,
                                luvPalSegm[rect],
                                numSelectedPxl);
      
            // also select points that have enough neighbours converging for
            // color assignment
            add_neigh(boolSelectedPxls, selectedPxls, numSelectedPxl);
          }
          else {
            int sizeRR = sizeof(param.rectRadius);
            for(int g = 0; g<sizeRR; g++) {
              setRadius(fixRadius[g]);
              calcRepresColorNAuto(selectedPxls, 
                                   boolSelectedPxls,
                                   luvPalSegm[rect],
                                   numSelectedPxl);
            }
          }
          if(auto_segm) {

            if( numSelectedPxl<act_threshold) { // not enough points converge
                                                // to luvPalSegm[rect], so
              failedTrials++;                   // try again with new color and
                                                // rect is not incremented
              maxTrialRandomColor=1;            // for next trial only one
                                                // random picked color
              if(failedTrials>=param.maxTrial)  // too many failures already:
                                                // stop it all
                breakup = true;
              // else once again
            }
            else { // regular
              updateTables(boolSelectedPxls);
              for( k=0; k<numImgPxl; k++ )
                if( boolSelectedPxls[k] ) {     // true if color of point was
                                                // touched by the moving
                                                // sphere
                  mapAssignedLabels[k] = rect;  // index of new color is set
                                                // to rect
                }
              rect++;
            }
          }
          else {  // non auto_segm
            if(testSameCluster(rect,luvPalSegm)) {
              // ignore double found cluster
              rect++;
            }
            else { // regular
              for( k=0; k<numImgPxl; k++ )
                if( boolSelectedPxls[k] )  // true if color of point was
                                           // touched by the moving sphere
                  mapAssignedLabels[k] = rect;// index of new color is set to
                                              // rect
              rect++;
            }
          }

          delete [] boolSelectedPxls;
          boolSelectedPxls=0;
          delete [] selectedPxls;
          selectedPxls=0;
        }//!breakup
      }
      _lti_debug("Leaving loop!\n");


      int numColors = rect; // = number of found color-clusters

      if(auto_segm) //elliminate classes with only small regions
        eliminateSmallClasses(numColors, luvPalSegm);
      
      // Determine the final palette and pixel-assignment

      try2getPalEntry(luvPalSegm,numColors);
      newPalette(luvPalSegm,numColors);
      
      if (auto_segm)
        getBestPalEntry(luvPalSegm,numColors);

      _lti_debug("Before auto_segm!\n");
      if (auto_segm) // Postprocessing: elliminate small regions
        if ( param.option==1 || param.option==2 ) {
          float stepSize = float(param.minRegionSize)/5;
          for( k=1; k<=5; k++)
            eliminateRegions(luvPalSegm, int(k*stepSize));
        }

      // write results to the parameter values
      colorMap.resize(numColors,rgbPixel(0,0,0));
      labels.resize(src.size(),nonColor);
      for(k=0; k<numColors; k++)
        luv2rgb(luvPalSegm[k], colorMap[k]);
      for( k=0; k<numImgPxl; k++ )
        labels.at(k) = mapAssignedLabels[k];

      if(auto_segm) {
        delete [] howManyEachRemColor;
        howManyEachRemColor=0;
        delete [] mapRemColors;
        mapRemColors=0;
        delete [] mapRemPxl;
        mapRemPxl=0;
      }
      delete [] mapAssignedLabels;
      mapAssignedLabels=0;
      delete [] mapPxlToColor;
      mapPxlToColor=0;
      delete [] _col0;
      _col0=0;
      delete [] _col1;
      _col1=0;
      delete [] _col2;
      _col2=0;
      delete [] luvPalSegm;
      luvPalSegm=0;
      _lti_debug("Leaving apply!\n");
      return true;
    }
   
    else{ // new implementation
      lti::image imgFiltered,imgSegmented;
      return apply(src,imgFiltered,imgSegmented,labels,colorMap);
    }
  }
 
  bool meanShiftSegmentation::apply(const lti::channel8& chnl1,
                                    const lti::channel8& chnl2,
                                    const lti::channel8& chnl3,
                                    lti::imatrix& dest) {

    // get parameters
    const parameters& param = getParameters();
      
    int i;

    if(param.classicAlgorithm) {
      
      if ((chnl1.empty()) ||
          ((chnl1.size() != chnl2.size()) ||
           (chnl2.size() != chnl3.size()) ||
           (chnl3.size() != chnl1.size()))) {
        setStatusString("Incompatible channel sizes");
        dest.clear();
        return false;
      }

      int failedTrials=0;
      int k;
      int maxTrialRandomColor = param.maxTrialRandomColor; // max trials todo
                                                           // better
                                                           // description

      std::vector<rectangle> rects;
      initializations(chnl1,chnl2,chnl3, rects);

      luvPixel* luvPalSegm;
      luvPalSegm = new luvPixel[rects.size()]; // stores color for each rect
                                               // (resp. luvPalette)

      unsigned int rect = 0;
      bool breakup = false;
      while ( !breakup && rect<rects.size() ) {
        numSubimgPxl = rects[rect].getDimensions().x * 
          rects[rect].getDimensions().y;
        // copy all pixel from chL,chU,chV ,
        // which are in rects[rect], to chLcurr, chUcurr,...
        cutRectangle( rects[rect] );
  
        setRadius(fixRadius[0]);
        while( !breakup && !my_sampling( luvPalSegm[rect], 
                                         maxTrialRandomColor) ) {
          failedTrials++;
          if(auto_segm)
            maxTrialRandomColor=1;
          if(failedTrials>=param.maxTrial)
            breakup = true;
        }
  
        if (!breakup) {
          int *selectedPxls = new int[numImgPxl];
          bool *boolSelectedPxls = new bool[numImgPxl];
          int numSelectedPxl;
          if(auto_segm) {
            setRadius(fixRadius[0]);
            calcRepresColorAuto(selectedPxls, boolSelectedPxls,
                                luvPalSegm[rect], numSelectedPxl);
            add_neigh(boolSelectedPxls, selectedPxls, numSelectedPxl);
          }
          else {
            int sizeRR = sizeof(param.rectRadius);
            for(int g = 0; g<sizeRR; g++) {
              setRadius(fixRadius[g]);
              calcRepresColorNAuto(selectedPxls, boolSelectedPxls, 
                                   luvPalSegm[rect], numSelectedPxl);
            }
          }
    
          if(auto_segm) {
            if( numSelectedPxl<act_threshold) {//
              failedTrials++;
              maxTrialRandomColor=1;
              if(failedTrials>=param.maxTrial)
                breakup = true;
              // else once again
            }
            else { // regular
              updateTables(boolSelectedPxls);
              for( k=0; k<numImgPxl; k++ )
                if( boolSelectedPxls[k] ) {
                  mapAssignedLabels[k] = rect;
                }
              rect++;
            }
          }
          else {  // non auto_segm
            if(testSameCluster(rect,luvPalSegm)) {
              // ignore double found cluster
              rect++;
            }
            else { // regular
              for( k=0; k<numImgPxl; k++ )
                if( boolSelectedPxls[k] )
                  mapAssignedLabels[k] = rect;
              rect++;
            }
          }
    
          delete [] boolSelectedPxls;
          boolSelectedPxls=0;
          delete [] selectedPxls;
          selectedPxls=0;
        }//!breakup
      }
      
      int numColors = rect; // = number of found color-clusters
      
      if(auto_segm) //elliminate classes with only small regions
        eliminateSmallClasses(numColors, luvPalSegm);
      
      // Determine the final palette and pixel-assignment

      try2getPalEntry(luvPalSegm,numColors);
      newPalette(luvPalSegm,numColors);
      
      if (auto_segm) {
        getBestPalEntry(luvPalSegm,numColors);
      }

      if (auto_segm) {// Postprocessing: elliminate small regions
        if ( param.option==1 || param.option==2 ) {
          float stepSize = float(param.minRegionSize)/5;
          for( k=1; k<=5; k++)
            eliminateRegions(luvPalSegm, int(k*stepSize));
        }
      }

      // write results to the parameter values
      dest.resize(chnl1.size(),nonColor);

      for( k=0; k<numImgPxl; k++ )
        dest.at(k) = mapAssignedLabels[k];

      if(auto_segm) {
        delete [] howManyEachRemColor;
        howManyEachRemColor=0;
        delete [] mapRemColors;
        mapRemColors=0;
        delete [] mapRemPxl;
        mapRemPxl=0;
      }
      delete [] mapAssignedLabels;
      mapAssignedLabels=0;
      delete [] mapPxlToColor;
      mapPxlToColor=0;
      delete [] _col0;
      _col0=0;
      delete [] _col1;
      _col1=0;
      delete [] _col2;
      _col2=0;
      delete [] luvPalSegm;
      luvPalSegm=0;
      return true;
    }
    
    else{ //new implementation

      if ((chnl1.empty()) ||
          ((chnl1.size() != chnl2.size()) ||
           (chnl2.size() != chnl3.size()) ||
           (chnl3.size() != chnl1.size()))) {
        setStatusString("Incompatible channel sizes");
        dest.clear();
        return false;
      }
    
      lti::timer time;
      
      // get parameters
      const parameters& param = getParameters();
    
      // determine dimensions
      dimensionRange = 3;
      dimensionSpace = 2;
      dimensionFeatureSpace = dimensionRange + dimensionSpace;

      width = chnl1.columns();
      height = chnl1.rows();
      dest.resize(height,width,0);
    
      // initialize members and allocate memory
      initialize();

      // copy the three channels into one array
      for(i = 0; i < imageSize; i++){
        imageLuvOrgF[i*dimensionRange] = (float) chnl1.at(i);
        imageLuvOrgF[i*dimensionRange+1] = (float) chnl2.at(i);
        imageLuvOrgF[i*dimensionRange+2] = (float) chnl3.at(i);
      }

      // Filter image 
      time.start();
      if(param.speedup == parameters::NoSpeedup){
        nonOptimizedFilter();
      }
      else optimizedFilter();
      time.stop();
      _lti_debug("\nfinished filtering... "<<time.getTime()/1000000.0<<
                 " seconds");
      
      // connect neighbour pixel with the same color to a region
      time.start();
      connect();
      time.stop();
      _lti_debug("\nfinished connecting...   "<<time.getTime()/1000000.0<<
                 " seconds");

      // fuse regions that have similar colors (difference less than sigmaS)
      time.start();
      fuseRegions();
      time.stop();
      _lti_debug("\nfinished fusing...       "<<time.getTime()/1000000.0<<
                 " seconds");

      // prune small regions that have less than minRegionSize pixels
      time.start();
      pruneRegions();
      time.stop();
      _lti_debug("\nfinished pruning...       "<<time.getTime()/1000000.0<<
                 " seconds");

      // store result
      for(i = 0; i < imageSize ; i++){
        dest.at(i) = labels[i];
      }
   
# ifdef _LTI_DEBUG
      // give some debug information
      _lti_debug("\n\nnumber of steps needed for convergence 1 2 3 ... " \
                 "/ number of pixels" << std::endl);
      int br = (int) log10(vecTrialsToConverge.maximum())+2;
      _lti_debug(std::endl);
      vector<int>::const_iterator it;
      for(it = vecTrialsToConverge.begin();
          it != vecTrialsToConverge.end();it++){
        std::cerr<<std::setiosflags(std::ios::left)<<std::setw(br)<<*it;
      }
      _lti_debug(std::endl);
#endif

      // free allocated memory
      freeMemory();
      
      return true;
    }
  }
    
  /*
   * Newer Mean-Shift implementation
   */
  bool meanShiftSegmentation::apply(const lti::image& src, 
                                    lti::image& destFiltered, 
                                    lti::image& destSegmented,
                                    lti::imatrix& destLabels,
                                    lti::palette& destColorMap) {

    // get parameters
    const parameters& param = getParameters();
    int i;
    
    if(param.classicAlgorithm) {
      destFiltered.clear(); //not available with classic algorithm
      destSegmented.resize(src.size());
      if(apply(src,destLabels,destColorMap)){
        for(i = 0; i < numImgPxl; i++){
          destSegmented.at(i) = destColorMap.at(destLabels.at(i));
        } 
        return true;
      }
      return false;
    }

    else { //new implementation
      timer time;

      // get parameters
      const parameters& param = getParameters();

      // determine dimensions
      dimensionRange = 3;
      dimensionSpace = 2;
      dimensionFeatureSpace = dimensionRange + dimensionSpace;

      width = src.columns();
      height = src.rows();
      destFiltered.resize(height,width, rgbPixel(0,0,0));
      destSegmented.resize(height,width, rgbPixel(0,0,0));
      destLabels.resize(height,width, 0);
    
      // initialize members and allocate memory
      initialize();
      
      // convert image from rgb to luv (imageLuvOrgF)
      rgbToLuv(src);

      // Filter image 
      time.start();
      if(param.speedup == parameters::NoSpeedup){
        nonOptimizedFilter();
      }
      else optimizedFilter();
      time.stop();
      _lti_debug("\nfinished filtering...  "<<time.getTime()/1000000.0<<
                 " seconds");
    
      // connect neighbour pixel with the same color to a region
      time.start();
      connect();
      time.stop();
      _lti_debug("\nfinished connecting...  "<<time.getTime()/1000000.0<<
                 " seconds");

      // fuse regions that have similar colors (difference less than sigmaS)
      time.start();
      fuseRegions();
      time.stop();
      _lti_debug("\nfinished fusing...      "<<time.getTime()/1000000.0<<
                 " seconds");

      // prune small regions that have less than minRegionSize pixels
      time.start();
      pruneRegions();
      time.stop();
      _lti_debug("\nfinished pruning...     "<<time.getTime()/1000000.0<<
                 " seconds");


      // store result

      // convert filtered image from luv to rgb
      luvToRgb(imageLuvFilteredF, destFiltered);
    
      // convert luv palette (modes) into rgb palette (colorMap)
      destColorMap.resize(regionCount);
      luvPixel luvColor; rgbPixel rgbColor;
      for(i = 0; i < regionCount; i++){
      
        luvColor.l = modes[i*dimensionRange];
        luvColor.u = modes[i*dimensionRange+1];
        luvColor.v = modes[i*dimensionRange+2];
        luvToRgb(luvColor, rgbColor);
        destColorMap.at(i) = rgbColor;
      }

      // copy labels into destLabels and store segmented image
      for(i = 0; i < imageSize ; i++){
        destLabels.at(i) = labels[i];
        destSegmented.at(i) = destColorMap.at(labels[i]);
      }

   
# ifdef _LTI_DEBUG
      // give some debug information
      _lti_debug("\n\nnumber of steps needed for convergence " \
                 "/ number of pixels\n");
      int br = (int) log10(vecTrialsToConverge.maximum())+2;
      _lti_debug(std::endl);
      vector<int>::const_iterator it;
      for(it = vecTrialsToConverge.begin();
          it != vecTrialsToConverge.end();it++){
        std::cerr<<std::setiosflags(std::ios::left)<<std::setw(br)<<*it;
      }
      _lti_debug(std::endl);
#endif
      // free allocated memory
      freeMemory();

      return true;
    }
  }

  //--------------------------------------------------------------------------
  // implementation of new algorithm similar to EDISON
  //--------------------------------------------------------------------------
  bool meanShiftSegmentation::filter(const lti::image& src, lti::image& dest){
    
    //get parameters
    const parameters& param = getParameters();

    //determine dimensions
    dimensionRange = 3;
    dimensionSpace = 2;
    dimensionFeatureSpace = dimensionRange + dimensionSpace;

    width = src.columns();
    height = src.rows();
    dest.resize(height,width, rgbPixel(0,0,0));
    
    //initialize members and allocate memory
    initialize();

    //convert image from rgb to luv (imageLuvOrgF)
    rgbToLuv(src);

    //Filter image 
    if(param.speedup == parameters::NoSpeedup){
      nonOptimizedFilter();
    }
    else optimizedFilter();
    
    //convert image from luv to rgb 
    luvToRgb(imageLuvFilteredF,dest);

    //free allocated memory
    freeMemory();

    return true;
  }

  bool meanShiftSegmentation::filter(lti::image& srcdest){
    
    lti::image img;
    filter(srcdest,img);
    srcdest = img;
    return true;
  }


  void  meanShiftSegmentation::initialize(){

    const parameters& param = getParameters();
    int i;

    imageSize = height * width;
    modePointCounts.resize(imageSize);
    imageLuvFilteredI = new int[imageSize*dimensionRange];
    imageLuvFilteredF = new float[imageSize*dimensionRange];
    imageLuvOrgF = new float[imageSize*dimensionRange];
    labels = new int[imageSize];
    modes = new float[(imageSize+1)*dimensionRange ];
    weightMap = new float[imageSize];
    for(i = 0; i < imageSize; i++){
      weightMap[i] = 1.0;
    }
    vecTrialsToConverge.resize(param.maxTrial,0);
  }

  void  meanShiftSegmentation::freeMemory(){
    
    delete[] imageLuvFilteredI;
    delete[] imageLuvFilteredF;
    delete[] imageLuvOrgF;
    delete[] labels;
    delete[] modes;
    delete[] weightMap;
  }

  void  meanShiftSegmentation::nonOptimizedFilter(){

    const parameters& param = getParameters();

    float center[5], newCenter[5], meanshiftVector[5];
    int lowerBoundX,lowerBoundY,upperBoundX,upperBoundY;
    int position, position1, position2;
    float msAbs = 0; //magnitude of meanshift vector
    float weightSum, sqrRadius = 0;
    int i,j;

    //initialize vectors
    for(i = 0; i < dimensionFeatureSpace; i++){
      center[i] = 0.0; newCenter[i] = 0.0; meanshiftVector[i] = 0.0;
    } 

    //traverse image
    for(i = 0; i < imageSize; i++) {

      // assign window center 
      center[0] = static_cast<float>(i%width); //column x
      center[1] = static_cast<float>(i/width); //row y
      for(j = 0; j < dimensionRange; j++){
        center[j+2] = imageLuvOrgF[dimensionRange*i+j];//luv colors
      }
  
      int iterationCount = 0;
      
      //determine by this loop the new center by following the meanshift vector
      do{
        //determine lower and upper bounds on grid
        lowerBoundX = int(center[0] - param.sigmaS);
        if(lowerBoundX < 0) lowerBoundX = 0;
        lowerBoundY = int(center[1] - param.sigmaS);
        if(lowerBoundY < 0) lowerBoundY = 0; 
        upperBoundX = int(center[0] + param.sigmaS);
        if(upperBoundX >= width) upperBoundX = width - 1;
        upperBoundY = int(center[1] + param.sigmaS);
        if(upperBoundY >= height) upperBoundY = height - 1;


        for(j = 0; j < dimensionFeatureSpace; j++){
          newCenter[j] = 0;
        }
        weightSum = 0;

        //iterate once through window of size sigmaS
        for(int x = lowerBoundX; x <= upperBoundX; x++){
          for(int y = lowerBoundY; y <= upperBoundY; y++){

            position1 = (y*width+x);
            position2 = (y*width+x)*dimensionRange;

            //test if feature point is within sphere of radius sigmaS / sigmaR
            sqrRadius += static_cast<float>(sqr((x - center[0]) / 
                                                param.sigmaS));
            sqrRadius += static_cast<float>(sqr((y - center[1]) /
                                                param.sigmaS));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2] -
                                                 center[2]) / param.sigmaR));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2+1] -
                                                 center[3]) / param.sigmaR));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2+2] -
                                                 center[4]) / param.sigmaR));
    
            // considered point is within sphere => accumulate to mean
            // for newCenter
            if (sqrRadius < 1.0) {
              newCenter[0] += weightMap[position1] * x;
              newCenter[1] += weightMap[position1] * y;
              newCenter[2] += weightMap[position1] * imageLuvOrgF[position2];
              newCenter[3] += weightMap[position1] * imageLuvOrgF[position2+1];
              newCenter[4] += weightMap[position1] * imageLuvOrgF[position2+2];
              weightSum    += weightMap[position1];
            }

            sqrRadius = 0;
          }
        }
  
        msAbs = 0;
  
        //determine the new center and the magnitude of the meanshift vector
        //meanshiftVector = newCenter - center;
        for(j = 0; j < dimensionFeatureSpace; j++){
          newCenter[j] /= weightSum;
          msAbs += sqr(newCenter[j] - center[j]);
          center[j] = newCenter[j];
        }

        iterationCount++;
        _lti_debug3("itCount: "<<iterationCount<<"   meanshift magnitude: "<<
                    msAbs<<std::endl);

      } while( msAbs > param.thresholdConverged && 
               iterationCount < param.maxTrial );
      //shifting window

      //for debug only
      vecTrialsToConverge[iterationCount-1]++;

      //store result
      position = i*dimensionRange;
      imageLuvFilteredF[position] = center[2];
      imageLuvFilteredF[position+1] = center[3];
      imageLuvFilteredF[position+2] = center[4];

      //store rounded result (faster than round())
      if(center[2] < 0) 
        imageLuvFilteredI[position] = (int) (center[2] - 0.5); 
      else
        imageLuvFilteredI[position] = (int) (center[2] + 0.5);
      if (center[3] < 0) 
        imageLuvFilteredI[position+1] = (int) (center[3] - 0.5); 
      else
        imageLuvFilteredI[position+1] = (int) (center[3] + 0.5);

      if(center[4] < 0) 
        imageLuvFilteredI[position+2] = (int) (center[4] - 0.5); 
      else
        imageLuvFilteredI[position+2] = (int) (center[4] + 0.5);
    }
  }

  void  meanShiftSegmentation::optimizedFilter(){

    const parameters& param = getParameters();

    float center[5], newCenter[5], meanshiftVector[5];
    float diff = 0, msAbs = 0, weightSum = 0, sqrRadius = 0;
    int lowerBoundX, lowerBoundY, upperBoundX, upperBoundY;
    int position, position1, position2, pointCount;
    int modeCandidateX, modeCandidateY, modeCandidate_i;
    int i,j;
    
    // one entry for each pixel: 
    // 0 means no mode yet assigned
    // 1 means mode already assigned  
    // 2 means the mode another pixel is now converging to should be assigned
    //        to this one too once the mode is determined
    int * modeTable = new int[imageSize];

    // stores the mode candidates that get the same mode assigned
    int * pointList = new int[imageSize]; 

    // the luv color of a mode candidate
    float* modeCandidatePoint = new float[dimensionRange];

    // initialize with 0
    memset(modeTable,0,imageSize*sizeof(int));
    memset(pointList,0,imageSize*sizeof(int));
    
    // initialize vectors
    for(j = 0; j < dimensionFeatureSpace; j++){
      center[j] = 0.0; newCenter[j] = 0.0; meanshiftVector[j] = 0.0;
    } 


    // traverse image
    for(i = 0; i < imageSize; i++) {

      // if a mode is already assigned to this data point then skip this 
      // point, otherwise proceed to find its mode by applying mean shift
      if (modeTable[i] == 1){
        continue;
      }
      pointCount = 0;
   
      // assign window center 
      center[0] = static_cast<float>(i%width);
      center[1] = static_cast<float>(i/width);
      for(j = 0; j < dimensionRange; j++){
        center[j+2] = imageLuvOrgF[dimensionRange*i+j];
      }
  
      int iterationCount = 0;
  
      // determine by this loop the new center by following the meanshift
      // vector
      do{
        // calculate the location of center on the lattice
        modeCandidateX  = (int) (center[0] + 0.5);
        modeCandidateY  = (int) (center[1] + 0.5);
        modeCandidate_i  = modeCandidateY * width + modeCandidateX;

        if((modeTable[modeCandidate_i] != 2) && (modeCandidate_i != i)) {
          // obtain the data point at basin_i to see if it is within h* 0.5 of
          // of center
          for (j = 0; j < dimensionRange; j++){
            modeCandidatePoint[j] = 
              imageLuvOrgF[dimensionRange * modeCandidate_i + j];
          }
    
          diff = 0.0f;
          diff += static_cast<float>(sqr((modeCandidatePoint[0] - center[2]) /
                                         param.sigmaR));
          diff += static_cast<float>(sqr((modeCandidatePoint[1] - center[3]) /
                                         param.sigmaR));
          diff += static_cast<float>(sqr((modeCandidatePoint[2] - center[4]) /
                                         param.sigmaR));
    
          // if the color is within radius of sigmaR than the same
          // mode is assigned
          if(diff < 0.5f){
            // if the data point at basin_i has not been associated to a mode 
            // then associate it with the mode that this one will converge to
            if (modeTable[modeCandidate_i] == 0){
              pointList[pointCount++] = modeCandidate_i;
              modeTable[modeCandidate_i] = 2;
            } 
            else{
              // the mode has already been associated with another
              // mode, therefore associate this one mode and the modes
              // in the point list with the mode associated with
              // data[basin_i]...

              // store the mode info into center using imageLuvFilteredF
              for (j = 0; j < dimensionRange; j++){
                center[j+2] = 
                  imageLuvFilteredF[modeCandidate_i*dimensionRange+j];
              }
        
              // update mode table for this data point indicating that a mode
              // has been associated with it
              modeTable[i] = 1;

              // stop mean shift calculation...
              break;
            }
          }
        }

        // determine lower and upper bounds on grid
        lowerBoundX = int(center[0] - param.sigmaS);
        if(lowerBoundX < 0) lowerBoundX = 0;
        lowerBoundY = int(center[1] - param.sigmaS);
        if(lowerBoundY < 0) lowerBoundY = 0; 
        upperBoundX = int(center[0] + param.sigmaS);
        if(upperBoundX >= width) upperBoundX = width - 1;
        upperBoundY = int(center[1] + param.sigmaS);
        if(upperBoundY >= height) upperBoundY = height - 1;

        for(int l = 0; l < dimensionFeatureSpace; l++){
          newCenter[l] = 0;
        }
        weightSum = 0;

        // iterate once through window of size sigmaS and determine the
        // newCenter
        for(int x = lowerBoundX; x <= upperBoundX; x++){
          for(int y = lowerBoundY; y <= upperBoundY; y++){

            position1 = (y*width+x);
            position2 = (y*width+x)*dimensionRange;

            // test if feature point is within sphere of radius sigmaS / sigmaR
            sqrRadius += static_cast<float>(sqr((x - center[0]) / 
                                                param.sigmaS));
            sqrRadius += static_cast<float>(sqr((y - center[1]) /
                                                param.sigmaS));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2] -
                                                 center[2]) / param.sigmaR));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2+1] -
                                                 center[3]) / param.sigmaR));
            sqrRadius += static_cast<float>(sqr((imageLuvOrgF[position2+2] -
                                                 center[4]) / param.sigmaR));
    
            // considered point is within sphere => accumulate to mean
            // for newCenter
            if(sqrRadius < 1.0f){
        
              newCenter[0] += weightMap[position1] * x;
              newCenter[1] += weightMap[position1] * y;
              newCenter[2] += weightMap[position1] * imageLuvOrgF[position2];
              newCenter[3] += weightMap[position1] * imageLuvOrgF[position2+1];
              newCenter[4] += weightMap[position1] * imageLuvOrgF[position2+2];
              weightSum += weightMap[position1];

              // highSpeedup: point within sphere => the same mode is
              // assigned to it even though it might have converged to
              // another mode (inaccuracy) the only difference to
              // mediumSpeedup
              if(param.speedup == parameters::HighSpeedup && 
                 modeTable[position1] == 0) {
                pointList[pointCount++]  = position1;
                modeTable[position1]  = 2;
              }
            }

            sqrRadius = 0;
          }
        }
  
        msAbs = 0;
  
        // determine the new center and the magnitude of the meanshift vector
        // meanshiftvector = newCenter - center
        for(j = 0; j < dimensionFeatureSpace; j++){
          newCenter[j] /= weightSum;
          msAbs += sqr(newCenter[j] - center[j]);
          center[j] = newCenter[j];
        }

        iterationCount++;
        _lti_debug3("itCount: "<<iterationCount<<"   meanshift magnitude: "<<
                    msAbs<<std::endl);

        // shifting window
      } while ((msAbs > param.thresholdConverged) &&
               (iterationCount < param.maxTrial));

      // for debug only
      vecTrialsToConverge[iterationCount-1]++;

      // associate the data point indexed by the point list with the mode
      // stored by center
      for (j = 0; j < pointCount; j++){
  
        // obtain the point location from the point list
        modeCandidate_i = pointList[j];

        // store result into imageLuvFilteredF and imageLuvFilteredI
        for(int k = 0; k < dimensionRange; k++){
          position =  modeCandidate_i * dimensionRange + k;
          imageLuvFilteredF[position] = center[k+2];
          if(center[k+2] < 0) 
            imageLuvFilteredI[position] = (int) (center[k+2] - 0.5); 
          else
            imageLuvFilteredI[position] = (int) (center[k+2] + 0.5);
        }

        // update the mode table for this point
        modeTable[modeCandidate_i] = 1;
      }
      
      // store result for point i
      position = i*dimensionRange;
      imageLuvFilteredF[position] = center[2];
      imageLuvFilteredF[position+1] = center[3];
      imageLuvFilteredF[position+2] = center[4];

      if(center[2] < 0)
        imageLuvFilteredI[position] = (int) (center[2] - 0.5); 
      else 
        imageLuvFilteredI[position] = (int) (center[2] + 0.5);

      if(center[3] < 0) 
        imageLuvFilteredI[position+1] = (int) (center[3] - 0.5); 
      else
        imageLuvFilteredI[position+1] = (int) (center[3] + 0.5);

      if(center[4] < 0)
        imageLuvFilteredI[position+2] = (int) (center[4] - 0.5); 
      else 
        imageLuvFilteredI[position+2] = (int) (center[4] + 0.5);

      // update mode table for this data point indicating that a mode has been 
      // associated with it
      modeTable[i] = 1;
     
    }

    delete[] modeTable;
    delete[] pointList; 
    delete[] modeCandidatePoint;
  }


  
  void  meanShiftSegmentation::connect(){
    
    regionCount = 0;
    int   i, k, neighLoc, neighborsFound, label = -1, regionLoc = 0;

    // stores all neighbour points of the considered seed that have
    // the same color and thus become themselves region seeds
    int* indexTable = new int[imageSize];
    
    // define eight connected neighbors
    int neigh[8];
    neigh[0]  = 1;
    neigh[1]  = 1-width;
    neigh[2]  = -width;
    neigh[3]  = -(1+width);
    neigh[4]  = -1;
    neigh[5]  = width-1;
    neigh[6]  = width;
    neigh[7]  = width+1;
  
    // initialize labels with -1
    for(i = 0; i < imageSize; i++){
      labels[i] = -1;
    }

    // traverse the image labeling each new region encountered
    for(i = 0; i < imageSize; i++){

      regionLoc = i;

      if(labels[i] < 0){  // if this region has not yet been labeled - label it
        labels[i] = ++label;// assign new label to this region

        // copy region color into modes
        for(k = 0; k < dimensionRange; k++){
          modes[(label * dimensionRange) + k]  =
            (float)(imageLuvFilteredI[(dimensionRange * i) + k]);
        }
  
        // initialzie indexTable
        int  index    = 0;
        indexTable[0]  = regionLoc;

        // increment mode point counts for this region to
        // indicate that one pixel belongs to this region
        modePointCounts[label]++;

        // populate labels with label for this specified region
        // calculating modePointCounts[label]
        while(true){

          //assume no neighbors will be found
          neighborsFound  = 0;

          // check the eight connected neighbors at regionLoc -
          // if a pixel has the same color as that located at 
          // regionLoc then declare it as part of this region
          for(int n = 0; n < 8; n++){

            // check bounds and if neighbor has been already labeled
            neighLoc = regionLoc + neigh[n];
            if((neighLoc >= 0) && 
               (neighLoc < imageSize) && (labels[neighLoc] < 0)) {
              for(k = 0; k < dimensionRange; ++k){
                if(imageLuvFilteredI[(regionLoc*dimensionRange)+k] != 
                   imageLuvFilteredI[(neighLoc*dimensionRange)+k])
                  break;
              }
              // neighbor i belongs to this region so label it and place 
              // it onto the index table buffer for further processing
              if(k == dimensionRange){

                //assign label to neighbor i
                labels[neighLoc] = label;
    
                //increment region point count
                modePointCounts[label]++;
    
                //place index of neighbor i onto the index tabel buffer
                indexTable[++index] = neighLoc;
    
                //indicate that a neighboring region pixel was
                //identified
                neighborsFound  = 1;
              }
            }
          }

          // check the indexTable to see if there are any more
          // entries to be explored - if so explore them, otherwise
          // exit the loop - we are finished
          if(neighborsFound)
            regionLoc  = indexTable[index];
          else if (index > 1)
            regionLoc  = indexTable[--index];
          else
            break; // fill complete
        }
      }
    }
    // calculate region count using label
    regionCount  = label+1;
    delete[] indexTable;
  }
  
  
  
  void meanShiftSegmentation::buildRegionAdjacencyMatrix(){

    const int memoryFactor = 10; // for size of allocated memory block
    int usedMemory = 0; //for debug only

    // allocate memory for region adjacency matrix 
    raMatrix = new regionAdjacencyList [regionCount];
    raPool = new regionAdjacencyList [memoryFactor * regionCount];

    // initialize the region adjacency array of lists
    int i;
    for(i = 0; i < regionCount; i++){
      raMatrix[i].label = i;
      raMatrix[i].next = NULL;
    }

    // initialize free list
    regionAdjacencyList* freeRaList = raPool;
    for(i = 0; i < memoryFactor*regionCount -1; i++){
      raPool[i].next = &raPool[i+1];
    }
    raPool[memoryFactor*regionCount-1].next = NULL;
    
    // traverse the labeled image building the RAM by looking 
    // to the right of and below the current pixel location thus
    // determining if a given region is adjacent to another
    int  j, curLabel, rightLabel, bottomLabel;
    bool inserted;

    regionAdjacencyList   *raNode1, *raNode2, *oldRAFreeList;
    for(i = 0; i < height - 1; i++){
      // check the right and below neighbors
      // for pixel locations whose x < width - 1
      for(j = 0; j < width - 1; j++){
        //calculate pixel labels
        curLabel = labels [i*width+j];  //current pixel
        rightLabel  = labels[i*width+j+1];  //right   pixel
        bottomLabel  = labels[(i+1)*width+j]; //bottom  pixel

        // check to the right, if the label of the right pixel is 
        // not the same as that of the current one then 
        // region[j] and region[j+1] are adjacent to one another
        // update the raMatrix
        if(curLabel != rightLabel){
          // obtain RAList object from region adjacency free list
          raNode1 = freeRaList;
          raNode2 = freeRaList->next;
          usedMemory += 2;

          // keep a pointer to the old region adj. free list just 
          // in case nodes already exist in respective region lists
          oldRAFreeList  = freeRaList;

          // update region adjacency free list
          freeRaList = freeRaList->next->next;

          // populate RAList nodes
          raNode1->label = curLabel;
          raNode2->label = rightLabel;

          // insert nodes into the RAM
          inserted = true;
          raMatrix[curLabel].insert(raNode2);
          inserted = raMatrix[rightLabel].insert(raNode1);
          // if the node already exists then place nodes back 
          // onto the region adjacency free list
          if(!inserted) { freeRaList = oldRAFreeList; usedMemory -= 2; }
        }

        // check below, if the label of
        // the bottom pixel is not the same as that
        // of the current one then region[j] and region[j+width]
        // are adjacent to one another - update the raMatrix
        if(curLabel != bottomLabel){
          // obtain RAList object from region adjacency free list
          raNode1 = freeRaList;
          raNode2 = freeRaList->next;
          usedMemory += 2;
          // keep a pointer to the old region adj. free list just in 
          // case nodes already exist in respective region lists
          oldRAFreeList  = freeRaList;

          // update region adjacency free list
          freeRaList = freeRaList->next->next;

          // populate RAList nodes
          raNode1->label = curLabel;
          raNode2->label = bottomLabel;

          // insert nodes into the RAM
          inserted = 0;
          raMatrix[curLabel].insert(raNode2);
          inserted = raMatrix[bottomLabel].insert(raNode1);

          // if the node already exists then place nodes back onto 
          // the region adjacency free list
          if(!inserted) { freeRaList = oldRAFreeList; usedMemory -= 2; }
        }

      }// (for(j = 0; j < width - 1;...

      // check only to the bottom neighbors of the right boundary pixels...

      // calculate pixel locations (j = width-1)
      curLabel  = labels[i*width+j]; //current pixel
      bottomLabel = labels[(i+1)*width+j]; //bottom  pixel
      
      // check below, if the label of the bottom pixel is not the same 
      // as that of the current one then region[j] and region[j+width]
      // are adjacent to one another - update the raMatrix
      if(curLabel != bottomLabel){
        //obtain RAList object from region adjacency free list
        raNode1  = freeRaList;
        raNode2  = freeRaList->next;
        usedMemory += 2;

        //keep a pointer to the old region adj. free list just in case 
        //nodes already exist in respective region lists
        oldRAFreeList = freeRaList;
  
        //update region adjacency free list
        freeRaList = freeRaList->next->next;
      
        //populate RAList nodes
        raNode1->label = curLabel;
        raNode2->label = bottomLabel;
      
        //insert nodes into the RAM
        inserted = true;
        raMatrix[curLabel].insert(raNode2);
        inserted = raMatrix[bottomLabel].insert(raNode1);
      
        //if the node already exists then place nodes back onto the 
        //region adjacency free list
        if(!inserted) { freeRaList = oldRAFreeList; usedMemory -= 2;}
      }
    }

    //check only to the right neighbors of the bottom boundary pixels...

    //check the right for pixel locations whose x < width - 1
    for(j = 0; j < width - 1; j++){
      //calculate pixel labels (i = height-1)
      curLabel  = labels[i*width+j];  //current pixel
      rightLabel  = labels[i*width+j+1];  //right   pixel
    
      //check to the right, if the label of the right pixel is not the same as
      //that of the current one then region[j] and region[j+1] are adjacent to
      //one another update the raMatrix
      if(curLabel != rightLabel){
        //obtain RAList object from region adjacency free list
        raNode1  = freeRaList;
        raNode2  = freeRaList->next;
        usedMemory += 2;

        //keep a pointer to the old region adj. free list just in case nodes
        //already exist in respective region lists
        oldRAFreeList = freeRaList;
  
        //update region adjacency free list
        freeRaList = freeRaList->next->next;
  
        //populate RAList nodes
        raNode1->label = curLabel;
        raNode2->label = rightLabel;
      
        //insert nodes into the RAM
        inserted = true;
        raMatrix[curLabel].insert(raNode2);
        inserted = raMatrix[rightLabel].insert(raNode1);
  
        //if the node already exists then place nodes back onto the region 
        //adjacency free list
        if(!inserted)  { freeRaList = oldRAFreeList; usedMemory -= 2; }
      }
    }

    _lti_debug2("\nallocated memory pool for region adjacency list: \n");
    _lti_debug2(memoryFactor * regionCount <<"   used memory : "<<
                usedMemory<<std::endl);
  }

  
  meanShiftSegmentation::regionAdjacencyList:: regionAdjacencyList(){
    
    label = -1;
    next = NULL;
  }


  bool meanShiftSegmentation::regionAdjacencyList::insert(
                                                  regionAdjacencyList *entry) {
    
    //if the list contains only one element then insert this element into next
    if(!next){
      next = entry;
      entry->next = NULL;
      return true;
    }

    //check first entry 
    if(next->label > entry->label){
  
      //insert entry into the list at this location
      entry->next  = next;
      next    = entry;
      return true;
    }

    //traverse the list until either:
    // (a) entry's label already exists - do not insert and return false
    // (b) the list ends or the current label is greater than entry's 
    // label, thus insert the entry at this location
 
    cur = next;
    while(cur){
      if(entry->label == cur->label){  //node already exists
        return false;
      }
      else if((!(cur->next))||(cur->next->label > entry->label)){
        //insert entry into the list at this location
        entry->next  = cur->next;
        cur->next  = entry;
        return true;
      }

      //traverse the region adjacency list
      cur = cur->next;
    }
    return false;
  }


  void meanShiftSegmentation::fuseRegions(){

    const parameters& param = getParameters();
    int oldRegionCount = regionCount;
    int counter = 0;
    std::vector<int> vecRegionCount;//for debug only
    bool inWindow;
    double diff;
    int mode1, mode2;
    double maxDistance = param.maxNeighbourColorDistance;

    do {
      oldRegionCount = regionCount;
      counter++;

      // find for each region its neighbours
      buildRegionAdjacencyMatrix();
  
      //traverse the raMatrix attempting to join raMatrix[i] with its
      //neighbour regions whose mode is a normalized distance < 0.5 from that
      //of region i.
      int i, iCanEl, neighCanEl;
      regionAdjacencyList* neighbor;
      
      //Step (1):
      //Treat each region Ri as a disjoint set:
      // - attempt to join Ri and Rj for all i != j that are neighbors and
      //   whose associated modes are a normalized distance of < 0.5 from one
      //   another

      // - the label of each region in the raMatrix is treated as a pointer
      //   to the canonical element of that region (e.g. raMatrix[i],
      //   initially has raMatrix[i].label = i, namely each region is
      //   initialized to have itself as its canonical element).
      for(i = 0; i < regionCount; i++){
        // aquire first neighbor in region adjacency list pointed to by
        // raMatrix[i]
        neighbor = raMatrix[i].next;
        while(neighbor){
    
          inWindow = false;
          diff = 0;
          mode1 = i; 
          mode2 = neighbor->label;
    
          // test if feature point is within a normalized distance of 0.5 from
          // mode
          diff += sqr((modes[mode1*dimensionRange] -
                       modes[mode2*dimensionRange]) / maxDistance);
          diff += sqr((modes[mode1*dimensionRange+1] -
                       modes[mode2*dimensionRange+1]) / maxDistance);
          diff += sqr((modes[mode1*dimensionRange+2] -
                       modes[mode2*dimensionRange+2]) / maxDistance);
          inWindow = (diff < 0.25); // range part of difference about 0.5 and
                                    // sqrt(0.5) = 0.25
    
          //attempt to join region and neighbor...
          if(inWindow){
            //region i and neighbor belong together so join them by:
            // (1) find the canonical element of region i
            iCanEl = i;
            while(raMatrix[iCanEl].label != iCanEl)
              iCanEl = raMatrix[iCanEl].label;
      
            // (2) find the canonical element of neighboring region
            neighCanEl = neighbor->label;
            while(raMatrix[neighCanEl].label != neighCanEl)
              neighCanEl = raMatrix[neighCanEl].label;

            // if the canonical elements of are not the same then assign
            // the canonical element having the smaller label to be the parent
            // of the other region...
            if(iCanEl < neighCanEl)
              raMatrix[neighCanEl].label = iCanEl;
            else{
              //must replace the canonical element of previous
              //parent as well
              raMatrix[raMatrix[iCanEl].label].label = neighCanEl;
        
              //re-assign canonical element
              raMatrix[iCanEl].label = neighCanEl;
            }
          }
          //check the next neighbor...
          neighbor = neighbor->next;
        }
      }

      // Step (2):
      // Level binary trees formed by canonical elements
      for(i = 0; i < regionCount; i++){
        iCanEl  = i;
        while(raMatrix[iCanEl].label != iCanEl){
          iCanEl = raMatrix[iCanEl].label;
        }
        raMatrix[i].label = iCanEl;
      }

      // Step (3):
      //Traverse joint sets, relabeling image.
      // (a)
      // Accumulate modes and re-compute point counts using canonical
      // elements generated by step 2.
      // allocate memory for mode and point count temporary buffers...
      float * modes_buffer = new float[dimensionRange*regionCount];
      int * MPC_buffer = new int[regionCount];
      
      //initialize buffers to zero
      for(i = 0; i < regionCount; i++)
        MPC_buffer[i]  = 0;
      for(i = 0; i < dimensionRange*regionCount; i++)
        modes_buffer[i]  = 0;
      
      //traverse raMatrix accumulating modes and point counts
      //using canoncial element information...
      int k, iMPC;
      for(i = 0; i < regionCount; i++){

        //obtain canonical element of region i
        iCanEl = raMatrix[i].label;
  
        //obtain mode point count of region i
        iMPC = modePointCounts[i];

        //color just mixed?
        //accumulate modes_buffer[iCanEl]
        for(k = 0; k < dimensionRange; k++)
          modes_buffer[(dimensionRange*iCanEl)+k] += 
            iMPC*modes[(dimensionRange*i)+k];
  
        //accumulate MPC_buffer[iCanEl]
        MPC_buffer[iCanEl] += iMPC;
      }

      // (b)
      // Re-label new regions of the image using the canonical
      // element information generated by step (1)
      // Also use this information to compute the modes of the newly
      // defined regions, and to assign new region point counts in
      // a consecute manner to the modePointCounts array
      
      int *label_buffer  = new int [regionCount];
      
      //initialize label buffer to -1
      for(i = 0; i < regionCount; i++){
        label_buffer[i]  = -1;
      }
      //traverse raMatrix re-labeling the regions
      int label = -1;
      for(i = 0; i < regionCount; i++){
        //obtain canonical element of region i
        iCanEl  = raMatrix[i].label;
        if(label_buffer[iCanEl] < 0){
          //assign a label to the new region indicated by canonical
          //element of i
          label_buffer[iCanEl]  = ++label;
    
          //recompute mode storing the result in modes[label]...
          iMPC  = MPC_buffer[iCanEl];
          for(k = 0; k < dimensionRange; k++)
            modes[(dimensionRange*label)+k] = 
              (modes_buffer[(dimensionRange*iCanEl)+k])/(iMPC);
          
          //assign a corresponding mode point count for this region into
          //the mode point counts array using the MPC buffer...
          modePointCounts[label] = MPC_buffer[iCanEl];
        }
      }
      
      //re-assign region count using label counter
      regionCount = label+1;

      // (c)
      // Use the label buffer to reconstruct the label map, which specified
      // the new image given its new regions calculated above
      for(i = 0; i < imageSize; i++){
        labels[i] = label_buffer[raMatrix[labels[i]].label];
      }
      //de-allocate memory
      delete [] modes_buffer;
      delete [] MPC_buffer;
      delete [] label_buffer;

#ifdef _LTI_DEBUG
      vecRegionCount.push_back(regionCount);
#endif

      // remove memory allocated in buildRegionAdjacencyMatrix() | alvarado 040217
      
      delete[] raPool;
      raPool = 0;
      
      delete[] raMatrix;
      raMatrix = 0;

    }
    while((oldRegionCount-regionCount > 0 )&&(counter<10));


#ifdef _LTI_DEBUG
    _lti_debug("\n\nnumber of regions: "<<std::endl);
    std::vector<int>::const_iterator it;
    for(it = vecRegionCount.begin(); it != vecRegionCount.end();it++){
      _lti_debug(*it<<"   ");
    }
    _lti_debug("\n");
#endif
    
  }


  void  meanShiftSegmentation::pruneRegions(){
    
    const parameters& param = getParameters();
    float* modes_buffer = new float  [dimensionRange*regionCount];
    int* MPC_buffer = new int[regionCount];
    int* label_buffer = new int[regionCount];
  
    int  i, k, candidate, iCanEl, neighCanEl, iMPC;
    int label, oldRegionCount, minRegionCount;
    double minSqDistance, neighborDistance;
    regionAdjacencyList *neighbor;

    // Apply pruning algorithm to classification structure, removing all
    // regions whose area is under the threshold area minRegionSize (pixels)
    do
      {
        //assume that no region has area under threshold area  of 
        minRegionCount  = 0;    
  
        //Step (1):
        // Build raMatrix using classifiction structure originally
        buildRegionAdjacencyMatrix();
  
        // Step (2):

        // Traverse the raMatrix joining regions whose area is less than
        // minRegion (pixels) with its respective candidate region.  A
        // candidate region is a region that displays the following
        // properties:
        //  - it is adjacent to the region being pruned
        //  - the distance of its mode is a minimum to that of the region
        //    being pruned such that or it is the only adjacent region having
        //    an area greater than minRegion
  
        for(i = 0; i < regionCount; i++){
    
          // ******************************************************************
          // Note: Adjust this if statement if a more sophisticated
          //       pruning criterion is desired. Basically in this step a
          //       region whose area is less than minRegion is pruned by
          //       joining it with its "closest" neighbor (in color).
          //       Therefore, by placing a different criterion for fusing a
          //       region the pruning method may be altered to implement a
          //       more sophisticated algorithm. 
          //******************************************************************
          if(modePointCounts[i] < param.minRegionSize){
            //update minRegionCount to indicate that a region
            //having area less than minRegion was found
            minRegionCount++;

            //obtain a pointer to the first region in the
            //region adjacency list of the ith region...
            neighbor = raMatrix[i].next;
        
            //calculate the distance between the mode of the ith
            //region and that of the neighboring region...
            candidate = neighbor->label;
            minSqDistance = 0.0;
            minSqDistance += sqr((modes[i*dimensionRange] - 
                                  modes[candidate*dimensionRange]));
            minSqDistance += sqr((modes[i*dimensionRange+1] -
                                  modes[candidate*dimensionRange+1]));
            minSqDistance += sqr((modes[i*dimensionRange+2] -
                                  modes[candidate*dimensionRange+2]));
                
            //traverse region adjacency list of region i and select
            //a candidate region
            neighbor  = neighbor->next;
            while(neighbor)  {
        
              //calculate the square distance between region i
              //and current neighbor...
              neighborDistance = 0.0;
              neighborDistance+=sqr((modes[i*dimensionRange]-
                                     modes[neighbor->label*dimensionRange]));
              neighborDistance+=sqr((modes[i*dimensionRange+1]-
                                     modes[neighbor->label*dimensionRange+1]));
              neighborDistance+=sqr((modes[i*dimensionRange+2]-
                                     modes[neighbor->label*dimensionRange+2]));

              //if this neighbors square distance to region i is less
              //than minSqDistance, then select this neighbor as the
              //candidate region for region i
              if(neighborDistance < minSqDistance){
                minSqDistance  = neighborDistance;
                candidate  = neighbor->label;
              }
              //traverse region list of region i
              neighbor  = neighbor->next;
            }

            //join region i with its candidate region:
            // (1) find the canonical element of region i
            iCanEl    = i;
            while(raMatrix[iCanEl].label != iCanEl)
              iCanEl    = raMatrix[iCanEl].label;
      
            // (2) find the canonical element of neighboring region
            neighCanEl  = candidate;
            while(raMatrix[neighCanEl].label != neighCanEl)
              neighCanEl  = raMatrix[neighCanEl].label;

            // if the canonical elements of are not the same then assign
            // the canonical element having the smaller label to be the parent
            // of the other region...
            if(iCanEl < neighCanEl)
              raMatrix[neighCanEl].label = iCanEl;
            else
              {
                //must replace the canonical element of previous
                //parent as well
                raMatrix[raMatrix[iCanEl].label].label  = neighCanEl;

                //re-assign canonical element
                raMatrix[iCanEl].label  = neighCanEl;
              }
          }

        }

        // Step (3):
    
        // Level binary trees formed by canonical elements
        for(i = 0; i < regionCount; i++)
          {
            iCanEl  = i;
            while(raMatrix[iCanEl].label != iCanEl)
              iCanEl  = raMatrix[iCanEl].label;
            raMatrix[i].label  = iCanEl;
          }
    
        // Step (4):
        //Traverse joint sets, relabeling image.
        // Accumulate modes and re-compute point counts using canonical
        // elements generated by step 2.
        //initialize buffers to zero
        for(i = 0; i < regionCount; i++)
          MPC_buffer[i]  = 0;
        for(i = 0; i < dimensionRange*regionCount; i++)
          modes_buffer[i]  = 0;

        //traverse raMatrix accumulating modes and point counts
        //using canoncial element information...
        for(i = 0; i < regionCount; i++){
      
          //obtain canonical element of region i
          iCanEl  = raMatrix[i].label;
      
          //obtain mode point count of region i
          iMPC  = modePointCounts[i];
      
          //accumulate modes_buffer[iCanEl]
          for(k = 0; k < dimensionRange; k++)
            modes_buffer[(dimensionRange*iCanEl)+k] +=
              iMPC*modes[(dimensionRange*i)+k];
      
          //accumulate MPC_buffer[iCanEl]
          MPC_buffer[iCanEl] += iMPC;
      
        }
  
        // (b)
        // Re-label new regions of the image using the canonical
        // element information generated by step (2)
        // Also use this information to compute the modes of the newly
        // defined regions, and to assign new region point counts in
        // a consecute manner to the modePointCounts array
    
        //initialize label buffer to -1
        for(i = 0; i < regionCount; i++)
          label_buffer[i]  = -1;
    
        //traverse raMatrix re-labeling the regions
        label = -1;
        for(i = 0; i < regionCount; i++){
          //obtain canonical element of region i
          iCanEl  = raMatrix[i].label;
          if(label_buffer[iCanEl] < 0){
            //assign a label to the new region indicated by canonical
            //element of i
            label_buffer[iCanEl]  = ++label;
        
            //recompute mode storing the result in modes[label]...
            iMPC  = MPC_buffer[iCanEl];
            for(k = 0; k < dimensionRange; k++)
              modes[(dimensionRange*label)+k]  = 
                (modes_buffer[(dimensionRange*iCanEl)+k])/(iMPC);
        
            //assign a corresponding mode point count for this region into
            //the mode point counts array using the MPC buffer...
            modePointCounts[label]  = MPC_buffer[iCanEl];
          }
        }
    
        //re-assign region count using label counter
        oldRegionCount  = regionCount;
        regionCount = label+1;
    
        // (c)
        // Use the label buffer to reconstruct the label map, which specified
        // the new image given its new regions calculated above
  
        for(i = 0; i < height*width; i++)
          labels[i]  = label_buffer[raMatrix[labels[i]].label];

        // remove memory allocated in buildRegionAdjacencyMatrix() | alvarado 040217

        delete[] raPool;
        raPool = 0;

        delete[] raMatrix;
        raMatrix = 0;

      }
    while(minRegionCount > 0);

   

    //de-allocate memory
    delete [] modes_buffer;
    delete [] MPC_buffer;
    delete [] label_buffer;
  }



  void  meanShiftSegmentation::rgbToLuv(const image& src){

    int i,j;

    static const double Yn      = 1.00000;
    static const double Un_prime  = 0.19784977571475;
    static const double Vn_prime  = 0.46834507665248;
    static const double Lt      = 0.008856;
    
    //RGB to LUV conversion
    static const double XYZ[3][3] = {  {  0.4125,  0.3576,  0.1804 },
                                       {  0.2125,  0.7154,  0.0721 },
                                       {  0.0193,  0.1192,  0.9502 }  };

    int rgbVal[3];
    int position;

    for(i = 0; i < height; i++){
      for(j = 0; j < width; j++){
      
        position = (i*width+j)*dimensionRange;

        rgbVal[0] = (src.at(i,j)).getRed(); 
        rgbVal[1] = (src.at(i,j)).getGreen(); 
        rgbVal[2] = (src.at(i,j)).getBlue();
      
        //declare variables
        double  x, y, z, L0, u_prime, v_prime, constant;
  
        //convert RGB to XYZ...
        x    = XYZ[0][0]*rgbVal[0] + XYZ[0][1]*rgbVal[1] + XYZ[0][2]*rgbVal[2];
        y    = XYZ[1][0]*rgbVal[0] + XYZ[1][1]*rgbVal[1] + XYZ[1][2]*rgbVal[2];
        z    = XYZ[2][0]*rgbVal[0] + XYZ[2][1]*rgbVal[1] + XYZ[2][2]*rgbVal[2];

        //convert XYZ to LUV...
  
        //compute L*
        L0    = y / (255.0 * Yn);
        if(L0 > Lt){
          imageLuvOrgF[position] = (float)(116.0 * (pow(L0, 1.0/3.0)) - 16.0);
        }
        else{
          imageLuvOrgF[position] = (float)(903.3 * L0);
        }
        //compute u_prime and v_prime
        constant  = x + 15 * y + 3 * z;
        if(constant != 0)  {
          u_prime  = (4 * x) / constant;
          v_prime = (9 * y) / constant;
        }
        else{
          u_prime  = 4.0;
          v_prime  = 9.0/15.0;
        }

        //compute u* and v*
        imageLuvOrgF[position +1] = (float) (13 * imageLuvOrgF[position] * 
                                             (u_prime - Un_prime));
        imageLuvOrgF[position +2] = (float) (13 * imageLuvOrgF[position] *
                                             (v_prime - Vn_prime));
      }
    }

  }
  

  void  meanShiftSegmentation::luvToRgb(float* src, image& dest){
 
    int i,j;
    
    static const double Yn        = 1.00000;
    static const double Un_prime  = 0.19784977571475;
    static const double Vn_prime  = 0.46834507665248;
   
    
    static const double RGB[3][3] = {  {  3.2405, -1.5371, -0.4985 },
                                       { -0.9693,  1.8760,  0.0416 },
                                       {  0.0556, -0.2040,  1.0573 }  };
    
    int    r, g, b;
    double  x, y, z, u_prime, v_prime, L, U, V;
  
    int position;

    for(i = 0; i < height; i++){
      for(j = 0; j < width; j++){
     
        position = (i*width+j)*dimensionRange;
        L = (double) src[position]; 
        U = (double) src[position+1];
        V = (double) src[position+2];
        if(L < 0.1)
          r = g = b = 0;
        else{
          //convert luv to xyz...
          if(L < 8.0)
            y  = Yn * L / 903.3;
          else{
            y  = (L + 16.0) / 116.0;
            y  *= Yn * y * y;
          }
    
          u_prime  = U / (13 * L) + Un_prime;
          v_prime  = V / (13 * L) + Vn_prime;

          x    = 9 * u_prime * y / (4 * v_prime);
          z    = (12 - 3 * u_prime - 20 * v_prime) * y / (4 * v_prime);

          //convert xyz to rgb...
          //[r, g, b] = RGB*[x, y, z]*255.0
          r    = (int) round((RGB[0][0]*x + RGB[0][1]*y + RGB[0][2]*z)*255.0);
          g    = (int) round((RGB[1][0]*x + RGB[1][1]*y + RGB[1][2]*z)*255.0);
          b    = (int) round((RGB[2][0]*x + RGB[2][1]*y + RGB[2][2]*z)*255.0);

          //check bounds...
          if(r < 0)  r = 0; if(r > 255)  r = 255;
          if(g < 0)  g = 0; if(g > 255)  g = 255;
          if(b < 0)  b = 0; if(b > 255)  b = 255;

        }

        //assign rgb values to dest
        dest.at(i,j) = rgbPixel(r,g,b);
      }
    }
  }
  
  void  meanShiftSegmentation::luvToRgb(luvPixel src, rgbPixel& dest){
    
    const double Yn      = 1.00000;
    const double Un_prime  = 0.19784977571475;
    const double Vn_prime  = 0.46834507665248;    
    
    const double RGB[3][3] = {  {  3.2405, -1.5371, -0.4985 },
        { -0.9693,  1.8760,  0.0416 },
        {  0.0556, -0.2040,  1.0573 }  };
    
    int    r, g, b;
    double  x, y, z, u_prime, v_prime, L, U, V;
  

    L = (double) src.l; U = (double) src.u; V = (double) src.v;
    if(L < 0.1)
      r = g = b = 0;
    else{
      //convert luv to xyz...
      if(L < 8.0)
        y  = Yn * L / 903.3;
      else{
        y  = (L + 16.0) / 116.0;
        y  *= Yn * y * y;
      }
      
      u_prime  = U / (13 * L) + Un_prime;
      v_prime  = V / (13 * L) + Vn_prime;
      
      x    = 9 * u_prime * y / (4 * v_prime);
      z    = (12 - 3 * u_prime - 20 * v_prime) * y / (4 * v_prime);
      
      //convert xyz to rgb...
      //[r, g, b] = RGB*[x, y, z]*255.0
      r    = (int) round((RGB[0][0]*x + RGB[0][1]*y + RGB[0][2]*z)*255.0);
      g    = (int) round((RGB[1][0]*x + RGB[1][1]*y + RGB[1][2]*z)*255.0);
      b    = (int) round((RGB[2][0]*x + RGB[2][1]*y + RGB[2][2]*z)*255.0);
      
      //check bounds...
      if(r < 0)  r = 0; if(r > 255)  r = 255;
      if(g < 0)  g = 0; if(g > 255)  g = 255;
      if(b < 0)  b = 0; if(b > 255)  b = 255;
      
    }
    
    //assign rgb values to dest
    dest = rgbPixel(r,g,b);
  }
}
  
