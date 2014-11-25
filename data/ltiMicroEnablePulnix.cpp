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
 * file .......: ltiMicroEnable.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 13.08.99
 * revisions ..: $Id: ltiMicroEnablePulnix.cpp,v 1.9 2006/09/05 10:36:57 ltilib Exp $
 */


#include "ltiMicroEnablePulnix.h"
#include "ltiTimer.h"
#include <cstring>

#ifdef _USE_MICRO_ENABLE_FRAME_GRABBER

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
using std::cout;
using std::endl;
#endif


#ifdef __linux__
// this is not required (and non existent ;-) ) on Windows
#include <unistd.h>
#endif

namespace lti {

  // two "global" constants, required in some classes
  static const int minHWShutterValue = 1;
  static const int maxHWShutterValue = 524;

  const int* microEnablePulnix::bankType::bitReversionLUT = 0;
  microEnablePulnix::bankType* microEnablePulnix::bankType::allCameraValues=0;

  // -----------------------------------------------------------------
  //  microEnablePulnix::bankType
  // -----------------------------------------------------------------
  void microEnablePulnix::bankType::initBitReversionLUT() {

    if (isNull(bitReversionLUT)) {
      // create the look-up-table for an 8-bit value, which entries
      // are the bit reversed values.
      int* tmp = new int[256];
      int i,j;
      int b,r;

      // for each entrie (or 8-bit value) in the LUT
      for (i=0;i<256;++i) {
        // b will contain the bit-reversed value
        // j is a 1-bit mask which runs from right to left
        // r is a 1-bit mask which runs from left to right
        for (b=0,j=1,r=128;j<256;j=j<<1,r=r>>1) {
          // check each bit in the i value...
          if ((i & j) != 0) {
            b = b | r;  // and store the "reversed" bit position if necessary
          }
        }
        tmp[i] = b;
      }
      delete[] bitReversionLUT;
      bitReversionLUT = tmp;
    }
  }

  const int& microEnablePulnix::bankType::revert(const ubyte& b) const {
    return bitReversionLUT[b];
  }

  microEnablePulnix::bankType::bankType() : functor::parameters() {

    initBitReversionLUT();

    cameraValues = 0;
    bankNumber = -1;

    gamma = 0.45f;
    gain = 0.3569f;
    brightness = 0.2057f;
    shutterSpeed = 0.0625f;
    shutterMode = ::lti::microEnablePulnix::parameters::Manual;
    shutterEntry = 3;

    colorMatrix.resize(3,3,0.0f,false,true);
    colorMatrix.at(0,0) = colorMatrix.at(1,1) = colorMatrix.at(2,2) = 1.0f;

    edgeEnhancementMode = lti::microEnablePulnix::parameters::Off;
    edgeEnhancementLevel = 2;

    whiteBalance = lti::microEnablePulnix::parameters::WBManual;
    redGain = 0.171875f;
    blueGain = 0.142132163f;

    adcOffset = 238;
    adcReferenceTop = 255;

    rgbOffset = trgbPixel<int>(0,0,0);

    //
    manualShutterTable.resize(9,0.0f,false,false);
    asyncShutterTable.resize(8,0.0f,false,false);

    // default values:
    manualShutterTable.at(0) = asyncShutterTable.at(0) = 268.0f;
    manualShutterTable.at(1) = asyncShutterTable.at(1) = 396.0f;
    manualShutterTable.at(2) = asyncShutterTable.at(2) = 460.0f;
    manualShutterTable.at(3) = asyncShutterTable.at(3) = 492.0f;
    manualShutterTable.at(4) = asyncShutterTable.at(4) = 508.0f;
    manualShutterTable.at(5) = asyncShutterTable.at(5) = 516.0f;
    manualShutterTable.at(6) = asyncShutterTable.at(6) = 520.0f;
    manualShutterTable.at(7) = asyncShutterTable.at(7) = 522.0f;
    manualShutterTable.at(8) = 3083.0f;

    // standard serial parameters for the communication with the pulnix camera
    serialParameters.receiveTimeout = 0;
    serialParameters.baudRate       = serial::parameters::Baud9600;
    serialParameters.parity         = serial::parameters::No;
    serialParameters.stopBits       = serial::parameters::One;
    serialParameters.characterSize  = serial::parameters::Cs8;
    serialParameters.port = serial::parameters::Com1;
  }

  microEnablePulnix::bankType::bankType(const bankType& other)
    : functor::parameters() {
    setBankNumber(other.getBankNumber());
    copy( other );
  };

  void microEnablePulnix::bankType::uninitialize() {

    bankNumber = -1;
    cameraValues = 0;

    gamma      = -1;
    gain       = -1;
    brightness = -10;
    shutterSpeed = -1;
    shutterMode = ::lti::microEnablePulnix::parameters::SMUninitialized;
    shutterEntry = -1;
    colorMatrix.clear();

    edgeEnhancementMode =
      ::lti::microEnablePulnix::parameters::EEUninitialized;
    edgeEnhancementLevel = -1;

    whiteBalance = ::lti::microEnablePulnix::parameters::WBUninitialized;
    redGain = -10;
    blueGain = -10;

    adcOffset = 0;
    adcReferenceTop = -1;

    rgbOffset = trgbPixel<int>(-200,-200,-200);

    manualShutterTable.resize(9,-1.0f,false,true);
    asyncShutterTable.resize(8,-1.0f,false,true);

  }


  microEnablePulnix::bankType::~bankType() {
    cameraValues = 0;
  }


  /*
   * Set the bank identification number for this parameter set
   */
  void microEnablePulnix::bankType::setBankNumber(const int& bank) {
    if (isNull(allCameraValues)) {
      // static member with camera values hasn't been initialized yet,
      // so let's create it:
      allCameraValues = new bankType[4];

      for (int i=0;i<4;++i) {
        allCameraValues[i].uninitialize();
        allCameraValues[i].bankNumber = i;
      }
    }

    bankNumber = bank;
    if ((bankNumber >= 0) && (bankNumber <= 3)) {
      cameraValues = &allCameraValues[bankNumber];
    } else {
      cameraValues = 0;
    }
  }

  /*
   * Get the bank identification number for this parameter set
   */
  const int& microEnablePulnix::bankType::getBankNumber() const {
    return bankNumber;
  }

  functor::parameters* microEnablePulnix::bankType::clone() const {
    return ( new bankType(*this) );
  }

  const char* microEnablePulnix::bankType::getTypeName() const {
    return "microEnablePulnix::bankType";
  }

  microEnablePulnix::bankType&
    microEnablePulnix::bankType::copy(const bankType& other ) {

#   ifndef _LTI_MSC_6
    // for normal  ANSI C++
    functor::parameters::copy(other);
#   else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)(const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
#   endif

    gamma = other.gamma;
    gain = other.gain;
    brightness = other.brightness;
    shutterSpeed = other.shutterSpeed;
    shutterMode = other.shutterMode;
    shutterEntry = other.shutterEntry;

    colorMatrix.copy(other.colorMatrix);

    edgeEnhancementMode = other.edgeEnhancementMode;
    edgeEnhancementLevel = other.edgeEnhancementLevel;

    whiteBalance = other.whiteBalance;
    redGain = other.redGain;
    blueGain = other.blueGain;

    adcOffset = other.adcOffset;
    adcReferenceTop = other.adcReferenceTop;

    rgbOffset = other.rgbOffset;

    manualShutterTable.copy(other.manualShutterTable);
    asyncShutterTable.copy(other.asyncShutterTable);

    return ( *this );
  }

  /*
   * write the bankType in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool microEnablePulnix::bankType::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool microEnablePulnix::bankType::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"gamma",gamma);
      lti::write(handler,"gain",gain);
      lti::write(handler,"brightness",brightness);
      lti::write(handler,"shutterSpeed",shutterSpeed);

      switch (shutterMode) {
        case ::lti::microEnablePulnix::parameters::SMUninitialized:
          lti::write(handler,"shutterMode","SMUninitialized");
          break;

        case ::lti::microEnablePulnix::parameters::DirectM:
          lti::write(handler,"shutterMode","DirectM");
          break;

        case ::lti::microEnablePulnix::parameters::DirectA:
          lti::write(handler,"shutterMode","DirectA");
          break;

        case ::lti::microEnablePulnix::parameters::Manual:
          lti::write(handler,"shutterMode","Manual");
          break;

        case ::lti::microEnablePulnix::parameters::Async:
          lti::write(handler,"shutterMode","Async");
          break;
      }

      lti::write(handler,"shutterEntry",shutterEntry);

      lti::write(handler,"colorMatrix",colorMatrix);

      switch (edgeEnhancementMode) {
        case ::lti::microEnablePulnix::parameters::EEUninitialized:
          lti::write(handler,"edgeEnhancementMode","EEUninitialized");
          break;
        case ::lti::microEnablePulnix::parameters::Off:
          lti::write(handler,"edgeEnhancementMode","Off");
          break;
        case ::lti::microEnablePulnix::parameters::Horizontal:
          lti::write(handler,"edgeEnhancementMode","Horizontal");
          break;
        case ::lti::microEnablePulnix::parameters::Vertical:
          lti::write(handler,"edgeEnhancementMode","Vertical");
          break;
        case ::lti::microEnablePulnix::parameters::Both:
          lti::write(handler,"edgeEnhancementMode","Both");
          break;
      }

      lti::write(handler,"edgeEnhancementLevel",edgeEnhancementLevel);

      switch (whiteBalance) {
        case ::lti::microEnablePulnix::parameters::WBUninitialized:
          lti::write(handler,"whiteBalance","WBUninitialized");
          break;
        case ::lti::microEnablePulnix::parameters::WBManual:
          lti::write(handler,"whiteBalance","WBManual");
          break;
        case ::lti::microEnablePulnix::parameters::WBAuto:
          lti::write(handler,"whiteBalance","WBAuto");
          break;
      }

      lti::write(handler,"redGain",redGain);
      lti::write(handler,"blueGain",blueGain);

      lti::write(handler,"adcOffset",adcOffset);
      lti::write(handler,"adcReferenceTop",adcReferenceTop);

      lti::write(handler,"rgbOffset",rgbOffset);

      lti::write(handler,"manualShutterTable",manualShutterTable);
      lti::write(handler,"asyncShutterTable",asyncShutterTable);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,
                                            const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool microEnablePulnix::bankType::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the bankType from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool microEnablePulnix::bankType::read(ioHandler& handler,
                                        const bool complete)
# else
  bool microEnablePulnix::bankType::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler,"gamma",gamma);
      b=b && lti::read(handler,"gain",gain);
      b=b && lti::read(handler,"brightness",brightness);
      b=b && lti::read(handler,"shutterSpeed",shutterSpeed);

      std::string str;
      lti::read(handler,"shutterMode",str);
      if (str == "DirectM") {
        shutterMode = ::lti::microEnablePulnix::parameters::DirectM;
      } else if (str == "DirectA") {
        shutterMode = ::lti::microEnablePulnix::parameters::DirectA;
      } else if (str == "Manual") {
        shutterMode = ::lti::microEnablePulnix::parameters::Manual;
      } else if (str == "Async") {
        shutterMode = ::lti::microEnablePulnix::parameters::Async;
      } else {
        shutterMode = ::lti::microEnablePulnix::parameters::SMUninitialized;
      }

      b=b && lti::read(handler,"shutterEntry",shutterEntry);

      b=b && lti::read(handler,"colorMatrix",colorMatrix);

      b=b && lti::read(handler,"edgeEnhancementMode",str);
      if (str == "Off") {
        edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Off;
      } else if (str == "Horizontal") {
        edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Horizontal;
      } else if (str == "Vertical") {
        edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Vertical;
      } else if (str == "Both") {
        edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Both;
      } else {
        edgeEnhancementMode =
          ::lti::microEnablePulnix::parameters::EEUninitialized;
      }

      b=b && lti::read(handler,"edgeEnhancementLevel",edgeEnhancementLevel);

      b=b && lti::read(handler,"whiteBalance",str);
      if (str == "WBManual") {
        whiteBalance = ::lti::microEnablePulnix::parameters::WBManual;
      } else if (str == "WBAuto") {
        whiteBalance = ::lti::microEnablePulnix::parameters::WBAuto;
      } else {
        whiteBalance = ::lti::microEnablePulnix::parameters::WBUninitialized;
      }

      b=b && lti::read(handler,"redGain",redGain);
      b=b && lti::read(handler,"blueGain",blueGain);

      b=b && lti::read(handler,"adcOffset",adcOffset);
      b=b && lti::read(handler,"adcReferenceTop",adcReferenceTop);

      b=b && lti::read(handler,"rgbOffset",rgbOffset);

      b=b && lti::read(handler,"manualShutterTable",manualShutterTable);
      b=b && lti::read(handler,"asyncShutterTable",asyncShutterTable);
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
  bool microEnablePulnix::bankType::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  bool
  microEnablePulnix::bankType::activate(const serial::parameters::ePortType&
                                        serialPort) const {

    // to activate this bank, send the options, that are not changed
    // changing the bank:  these are:
    // - ADC reference top
    // - ADC offset
    // - Red/Green/Blue offset
    // - Brightness

    cameraValues->adcOffset = 0;
    cameraValues->adcReferenceTop = -1;
//      cameraValues->rgbOffset = trgbPixel<int>(-200,-200,-200);
//      cameraValues->brightness = -10;

    return upload(serialPort,false);
  }

  /*
   * upload the data in the respective bank
   */
  bool
  microEnablePulnix::bankType::upload(const serial::parameters::ePortType&
                                      serialPort,
                                      const bool force) const {

    // confirmation code for this bank
    const ubyte bankChar = static_cast<ubyte>(0xA8 + (0x03 & bankNumber));
    float ftmp;
    ubyte tmp;
    ubyte buffer[256];
    int idx = 0;
    int val,i;
    char reg;

    serialParameters.port = serialPort;
    serial com(serialParameters);

    //
    // the PULNIX TMC6700 seems to have following register address map:
    //
    // Reg.    Function
    // -------------------------------------------
    // 0x40
    // 0x41  Gain (bit reversed value)
    // 0x42  ADC Offset (bit reversed value)
    // 0x43  ADC Reference Top
    // 0x44
    // 0x45
    // 0x46  Edge Enh., Palette, Hold, WB
    //       Bits: 76 5 4 32 1 0
    //             |  ? | || | |-> WB
    //             |    | || |---> Hold
    //             |    | ||-----> Edge Ehn. Horizontal
    //             |    | |------> Edge Enh. Vertical
    //             |    |--------> Edge Enh. Level
    //             |-------------> Palette 00 RGB, 01 4:4:4 Yuv, 10 2:2:2 Yuv
    //
    // 0x47  Shutter Mode (Manual, Async, Direct), Gamma (Bit 6 (-X-- ----))
    // 0x48
    // 0x49
    // 0x4A  DirectM Shutter Value
    // 0x4B  DirectM Shutter Value
    // 0x4C  Shutter Table Manual: Dial 9
    // 0x4D  Shutter Table Manual: Dial 9
    // 0x4E  Shutter Table Manual: Dial 8
    // 0x4F  Shutter Table Manual: Dial 8
    // 0x50  Shutter Table Manual: Dial 7
    // 0x51  Shutter Table Manual: Dial 7
    // 0x52  Shutter Table Manual: Dial 6
    // 0x53  Shutter Table Manual: Dial 6
    // 0x54  Shutter Table Manual: Dial 5
    // 0x55  Shutter Table Manual: Dial 5
    // 0x56  Shutter Table Manual: Dial 4
    // 0x57  Shutter Table Manual: Dial 4
    // 0x58  Shutter Table Manual: Dial 3
    // 0x59  Shutter Table Manual: Dial 3
    // 0x5A  Shutter Table Manual: Dial 2
    // 0x5B  Shutter Table Manual: Dial 2
    // 0x5C  Shutter Table Manual: Dial 1
    // 0x5D  Shutter Table Manual: Dial 1
    // 0x5E  Color Matrix:  Entry BB
    // 0x5F  Color Matrix:  Entry BG
    // 0x60  Color Matrix:  Entry BR
    // 0x61  Color Matrix:  Entry GB
    // 0x62  Color Matrix:  Entry GG
    // 0x63  Color Matrix:  Entry GR
    // 0x64  Color Matrix:  Entry RB
    // 0x65  Color Matrix:  Entry RG
    // 0x66  Color Matrix:  Entry RR
    // 0x67  RGB Offset Blue
    // 0x68  RGB Offset Green
    // 0x69  RGB Offset Red
    // 0x6A  Ext. WB: Blue Gain
    // 0x6B  Ext. WB: Red Gain
    // 0x6C  DSP Gain (Brightness)
    // 0x6D
    // 0x6E  Shutter Table Async: Dial 8
    // 0x6F  Shutter Table Async: Dial 8
    // 0x70  Shutter Table Async: Dial 7
    // 0x71  Shutter Table Async: Dial 7
    // 0x72  Shutter Table Async: Dial 6
    // 0x73  Shutter Table Async: Dial 6
    // 0x74  Shutter Table Async: Dial 5
    // 0x75  Shutter Table Async: Dial 5
    // 0x76  Shutter Table Async: Dial 4
    // 0x77  Shutter Table Async: Dial 4
    // 0x78  Shutter Table Async: Dial 3
    // 0x79  Shutter Table Async: Dial 3
    // 0x7A  Shutter Table Async: Dial 2
    // 0x7B  Shutter Table Async: Dial 2
    // 0x7C  Shutter Table Async: Dial 1
    // 0x7D  Shutter Table Async: Dial 1
    // 0x7E  Shutter Value: DirectA
    // 0x7F  Shutter Value: DirectA

    // ----
    // gain
    // ----

    if (force || (gain != cameraValues->gain)) {
      val = static_cast<int>(gain*255);

      tmp = revert(static_cast<ubyte>(val & 0xFF));

      buffer[idx++] = 0x41;
      buffer[idx++] = tmp;
      buffer[idx++] = bankChar;

      cameraValues->gain = gain;
    }

    // ------
    // Offset
    // ------
    if (force || (adcOffset != cameraValues->adcOffset)) {
      buffer[idx++] = 0x42;
      buffer[idx++] = revert(static_cast<ubyte>(0xFF & adcOffset));
      buffer[idx++] = bankChar;

      cameraValues->adcOffset = adcOffset;
    }

    // -------------
    // Reference Top
    // -------------
    if (force || (adcReferenceTop != cameraValues->adcReferenceTop)) {
      buffer[idx++] = 0x43;
      buffer[idx++] = revert(static_cast<ubyte>(0xFF & adcReferenceTop));
      buffer[idx++] = bankChar;

      cameraValues->adcReferenceTop = adcReferenceTop;
    }

    // ------------------------------
    // Palette, Edge Ehn. Hold and WB
    // ------------------------------
    if (force ||
        ((edgeEnhancementMode != cameraValues->edgeEnhancementMode) ||
         (edgeEnhancementLevel != cameraValues->edgeEnhancementLevel) ||
         (whiteBalance != cameraValues->whiteBalance))) {

      buffer[idx++] = 0x46;

      val = 0x20;
      val = val | ((edgeEnhancementLevel & 0x01) << 4);
      val = val | static_cast<ubyte>(edgeEnhancementMode);
      if (whiteBalance == ::lti::microEnablePulnix::parameters::WBManual) {
        val = val | 0x01;
      }

      buffer[idx++] = static_cast<ubyte>(val);
      buffer[idx++] = bankChar;

      cameraValues->edgeEnhancementMode = edgeEnhancementMode;
      cameraValues->edgeEnhancementLevel = edgeEnhancementLevel;
      cameraValues->whiteBalance = whiteBalance;
    }

    // ----------------------
    // gamma and shutter mode
    // ----------------------
    if (force || ((gamma != cameraValues->gamma) ||
                  (shutterMode != cameraValues->shutterMode) ||
                  (shutterEntry != cameraValues->shutterEntry) ||
                  (shutterSpeed != cameraValues->shutterSpeed))) {

      switch (shutterMode) {
        case ::lti::microEnablePulnix::parameters::DirectM:
          tmp = 0x20;
          break;
        case ::lti::microEnablePulnix::parameters::DirectA:
          tmp = 0x30;
          break;
        case ::lti::microEnablePulnix::parameters::Manual:
          tmp = shutterEntry % 10;
          break;
        case ::lti::microEnablePulnix::parameters::Async:
          tmp = shutterEntry % 10;
          tmp = tmp | 0x10;
          break;
        default:
          tmp = 0x20; // default DirectM
      }

      // set the bit for the gamma value
      if (gamma < (1.45/2.0)) {
        tmp = tmp |= 0x40;
      }

      // build the buffer to be transmitted
      buffer[idx++] = 0x47;
      buffer[idx++] = tmp;
      buffer[idx++] = bankChar;

      // if the direct value has changed...
      if (force ||
          ((shutterSpeed != cameraValues->shutterSpeed) ||
           (((shutterMode == ::lti::microEnablePulnix::parameters::DirectM) ||
             (shutterMode == ::lti::microEnablePulnix::parameters::DirectA))&&
            (shutterMode != cameraValues->shutterMode)))) {

        // send the value of the registers
        if ((shutterSpeed>=0) && (shutterSpeed<=1)) {
          val = maxHWShutterValue -
            static_cast<int>((maxHWShutterValue-minHWShutterValue)*
                             shutterSpeed);
        } else {
          val = static_cast<int>(shutterSpeed) & 0xFFFF;
        }

        reg = (shutterMode == ::lti::microEnablePulnix::parameters::DirectA) ?
          0x7E : 0x4A;

        buffer[idx++] = reg;
        buffer[idx++] = static_cast<ubyte>(val & 0xFF);

        reg++;
        buffer[idx++] = reg;
        buffer[idx++] = static_cast<ubyte>((val & 0xFF00) >> 8);
        buffer[idx++] = bankChar;

        cameraValues->shutterSpeed = shutterSpeed;
      }

      cameraValues->gamma = gamma;
      cameraValues->shutterMode = shutterMode;
      cameraValues->shutterEntry = shutterEntry;
    }

    // ---------------
    // shutter table
    // ---------------

    // send the shutter tables
    if (force || (manualShutterTable != cameraValues->manualShutterTable)) {
      for (i=8;i>=0;--i) {
        if (force || (manualShutterTable.at(i) !=
                      cameraValues->manualShutterTable.at(i))) {

          ftmp = manualShutterTable.at(i);
          if ((ftmp>=0.0f) && (ftmp<=1.0f)) {
            val = maxHWShutterValue -
              static_cast<int>((maxHWShutterValue-minHWShutterValue)*ftmp);
          } else {
            val = static_cast<int>(ftmp) & 0xFFFF;
          }

          buffer[idx++]=0x5C-(i*2);
          buffer[idx++]=static_cast<ubyte>(val & 0xFF);

          buffer[idx++]=0x5D-(i*2);
          buffer[idx++]=static_cast<ubyte>((val & 0xFF00) >> 8);

          cameraValues->manualShutterTable.at(i) = manualShutterTable.at(i);
        }
      }
      buffer[idx++] = bankChar;
    }

    // ------------
    // color matrix
    // ------------
    if (force || (!colorMatrix.equals(cameraValues->colorMatrix))) {

      // change the matrix only if the actual matrix is valid!
      if (colorMatrix.size() == point(3,3)) {
        int i,j;
        reg = 0x5E;
        for (j=2; j>=0;--j) {
          for (i=2; i>=0;--i) {
            buffer[idx++] = reg;
            buffer[idx++] = static_cast<ubyte>(colorMatrix.at(j,i)/0.03125);
            reg++;
          }
        }
        buffer[idx++] = bankChar;
        cameraValues->colorMatrix.copy(colorMatrix);
      }

    }

    // -----------------
    // RGB Offset
    // -----------------

    if (force ||
        ((rgbOffset.getRed() != cameraValues->rgbOffset.getRed()) ||
         (rgbOffset.getGreen() != cameraValues->rgbOffset.getGreen())||
         (rgbOffset.getBlue() != cameraValues->rgbOffset.getBlue())) ) {
      buffer[idx++] = 0x67;
      buffer[idx++] = static_cast<ubyte>(rgbOffset.getBlue() & 0xFF);

      buffer[idx++] = 0x68;
      buffer[idx++] = static_cast<ubyte>(rgbOffset.getGreen() & 0xFF);

      buffer[idx++] = 0x69;
      buffer[idx++] = static_cast<ubyte>(rgbOffset.getRed() & 0xFF);

      buffer[idx++] = bankChar;

      cameraValues->rgbOffset.setRed(rgbOffset.getRed());
      cameraValues->rgbOffset.setGreen(rgbOffset.getGreen());
      cameraValues->rgbOffset.setBlue(rgbOffset.getBlue());
    }

    // ------------------
    // Blue and Red Gain
    // ------------------

    if (force || ((blueGain != cameraValues->blueGain) ||
                  (redGain != cameraValues->redGain))) {

      buffer[idx++] = 0x6A;
      buffer[idx++] = static_cast<int>(-blueGain*127) & 0xFF;

      buffer[idx++] = 0x6B;
      buffer[idx++] = static_cast<int>(-redGain*127) & 0xFF;

      buffer[idx++] = bankChar;

      cameraValues->redGain = redGain;
      cameraValues->blueGain = blueGain;
    }

    // ----------
    // brightness
    // ----------
    if (force || (brightness != cameraValues->brightness)) {
      buffer[idx++] = 0x6C;
      tmp = static_cast<ubyte>(0xFF & int(brightness*127));
      buffer[idx++] = tmp;
      buffer[idx++] = bankChar;

      cameraValues->brightness = brightness;
    }

    // -----------------
    // shutter table (2)
    // -----------------

    // send the shutter tables
    if (force || (asyncShutterTable != cameraValues->asyncShutterTable)) {
      for (i=7; i >= 0; --i) {
        if (force || (asyncShutterTable.at(i) !=
                      cameraValues->asyncShutterTable.at(i))) {

          ftmp = asyncShutterTable.at(i);

          if ((ftmp >= 0.0f) && (ftmp <= 1.0f)) {
            val = maxHWShutterValue -
              static_cast<int>((maxHWShutterValue-minHWShutterValue)*ftmp);
          } else {
            val = static_cast<int>(ftmp) & 0xFFFF;
          }

          buffer[idx++]=0x7C-(i*2);
          buffer[idx++]=static_cast<ubyte>(val & 0xFF);

          buffer[idx++]=0x7D-(i*2);
          buffer[idx++]=static_cast<ubyte>((val & 0xFF00) >> 8);

          cameraValues->asyncShutterTable.at(i) = asyncShutterTable.at(i);
        }
      }

      buffer[idx++] = bankChar;
    }

    bool b = true;
    if (idx > 0) {

      for (i=0;b && (i<idx);++i) {
//          if (buffer[i] == bankChar) {
//            passiveWait(50000); // wait 1/10th of a second
//          }
        b = b && com.send(buffer[i]);
        if (buffer[i] == bankChar) {
          passiveWait(100000); // wait 1/10th of a second
        }
      }

#if defined(_LTI_DEBUG) && _LTI_DEBUG > 0
      _lti_debug("\n\r");
      for (i=0;b && (i<idx);++i) {
        std::cerr.setf(std::ios::hex, std::ios::basefield);
        _lti_debug(int(buffer[i]) << " \t");
        if (i%10 == 9) {
          std::cout << "\n\r";
        }
      }
#endif

    }

    return b;
  }

  /**
   * read the parameters from the parameter-bank in the camera,
   * assuming that the serial port of the pulnix camera is
   * connected at the given serial port.  This will take a while!
   * @param serialPort the serial port used to communicate with
   * the camera.  (Com1, Com2, ...)
   * @return true if successful, false otherwise
   */
  bool
  microEnablePulnix::bankType::download(const serial::parameters::ePortType&
                                        serialPort) {

    // confirmation code for this bank

    const int regBankSize = 0x40;
    const ubyte bankChar = static_cast<ubyte>(0xA8 + (0x03 & bankNumber));
    char buffer[regBankSize];
    ubyte* ubuffer = reinterpret_cast<ubyte*>(buffer);

    char i,j,idx;
    ubyte tmp,reg;
    int val;

    serialParameters.port = serialPort;
    serialParameters.receiveTimeout=5000;
    serial com(serialParameters);

    _lti_debug1("Receiving camera data...");

    // request for camera data
    com.send(bankChar);

    passiveWait(50000); // wait 1/20th of a second before sending the rest...

    // read camera data
    for (i=0;i<regBankSize;++i) {
      com.send(i);
      if (!com.receive(buffer[i])) {
        return false;
      }
    }

#if defined(_LTI_DEBUG) && _LTI_DEBUG > 1
    std::cout << "done." << std::endl;
    std::cout.flush();

    for (i=0;i<regBankSize;++i) {
      std::cout.setf(std::ios::hex, std::ios::basefield);
      std::cout << int(i) << " " << int(ubuffer[i]) << "\t";
      std::cout.flush();
      if (i%4 == 3) {
       std::cout << std::endl;
      }
    }

#endif

    // Convert data buffer into the expected values

    // gain
    tmp = revert(ubuffer[0x01]);
    gain = float(tmp)/255.0f;
    cameraValues->gain = gain;

    // ADC offset
    adcOffset = revert(ubuffer[0x02]);
    cameraValues->adcOffset = adcOffset;

    // ADC Ref. Top
    adcReferenceTop=revert(ubuffer[0x03]);
    cameraValues->adcReferenceTop = adcReferenceTop;

    // Edge Enhancement and White Balance
    tmp = ubuffer[0x06] & 0x0C;
    if (tmp == 0) {
      edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Off;
    } else if (tmp == 0x04) {
      edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Horizontal;
    } else if (tmp == 0x08) {
      edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Vertical;
    } else {
      edgeEnhancementMode = ::lti::microEnablePulnix::parameters::Both;
    }

    if ((ubuffer[0x06] & 0x10) != 0) {
      edgeEnhancementLevel = 1;
    } else {
      edgeEnhancementLevel = 2;
    }

    if ((ubuffer[0x06] & 0x01) != 0) {
      whiteBalance = ::lti::microEnablePulnix::parameters::WBManual;
    } else {
      whiteBalance = ::lti::microEnablePulnix::parameters::WBAuto;
    }

    cameraValues->edgeEnhancementMode = edgeEnhancementMode;
    cameraValues->edgeEnhancementLevel = edgeEnhancementLevel;
    cameraValues->whiteBalance = whiteBalance;

    // Shutter and gamma
    tmp = ubuffer[0x07] & 0x30;
    if (tmp == 0x00) {
      shutterMode = ::lti::microEnablePulnix::parameters::Manual;
    } else if (tmp == 0x10) {
      shutterMode = ::lti::microEnablePulnix::parameters::Async;
    } else if (tmp == 0x20) {
      shutterMode = ::lti::microEnablePulnix::parameters::DirectM;
    } else if (tmp == 0x30) {
      shutterMode = ::lti::microEnablePulnix::parameters::DirectA;
    }

    shutterEntry = ubuffer[0x07] & 0x0F;

    if ((ubuffer[0x07] & 0x40) != 0) {
      gamma = 0.45;
    } else {
      gamma = 1.0;
    }

    cameraValues->shutterEntry = shutterEntry;
    cameraValues->shutterMode = shutterMode;
    cameraValues->gamma = gamma;

    // directM shutter value
    val = ubuffer[0x0A]+256*ubuffer[0x0B];
    if ((val >= minHWShutterValue) && (val <= maxHWShutterValue)) {
      shutterSpeed = float(maxHWShutterValue-val)/
        float(maxHWShutterValue-minHWShutterValue);
    } else {
      shutterSpeed = val;
    }
    cameraValues->shutterSpeed = shutterSpeed;

    // shutter table (manual)
    for (idx = 0x0C,i=8;i>=0;--i) {
      val = ubuffer[idx];
      idx++;
      val += ubuffer[idx]*256;
      idx++;

      if ((val >= minHWShutterValue) && (val <= maxHWShutterValue)) {
        manualShutterTable.at(i) = float(maxHWShutterValue-val)/
          float(maxHWShutterValue-minHWShutterValue);
      } else {
        manualShutterTable.at(i) = float(val);
      }
    }
    cameraValues->manualShutterTable.copy(manualShutterTable);

    // color matrix
    colorMatrix.resize(3,3,0.0f,false,false);
    reg = 0x1E;
    for (j=2; j>=0;--j) {
      for (i=2; i>=0;--i) {
        colorMatrix.at(j,i) = float(int(buffer[reg]))*0.03125f;
        reg++;
      }
    }
    cameraValues->colorMatrix.copy(colorMatrix);

    // rgbOffset
    rgbOffset = trgbPixel<int>(ubuffer[0x29],
                               ubuffer[0x28],
                               ubuffer[0x27]);
    cameraValues->rgbOffset = rgbOffset;

    // blue gain
    blueGain = -float(int(buffer[0x2A]))/127.0f;
    cameraValues->blueGain = blueGain;

    // red gain
    redGain = -float(int(buffer[0x2B]))/127.0f;
    cameraValues->redGain = redGain;

    // dsp gain (brightness)
    brightness = float(int(buffer[0x2C]))/127.0f;
    cameraValues->brightness = brightness;

    // shutter table async
    for (idx = 0x2E,i=7;i>=0;--i) {
      val = buffer[idx];
      idx++;
      val += buffer[idx]*256;
      idx++;

      if ((val >= minHWShutterValue) && (val <= maxHWShutterValue)) {
        asyncShutterTable.at(i) = float(maxHWShutterValue-val)/
          float(maxHWShutterValue-minHWShutterValue);
      } else {
        asyncShutterTable.at(i) = float(val);
      }
    }
    cameraValues->asyncShutterTable.copy(asyncShutterTable);

    // shutter
    if (shutterMode == ::lti::microEnablePulnix::parameters::DirectA) {
      // directA shutter value
      val = ubuffer[0x3E]+256*ubuffer[0x3F];

      if ((val >= minHWShutterValue) && (val <= maxHWShutterValue)) {
        shutterSpeed = float(maxHWShutterValue-val)/
          float(maxHWShutterValue-minHWShutterValue);
      } else {
        shutterSpeed = val;
      }

      cameraValues->shutterSpeed = shutterSpeed;
    }

    return true;
  }

  const microEnablePulnix::bankType&
  microEnablePulnix::bankType::getCameraValues() const {
    return *cameraValues;
  }

  // -----------------------------------------------------------------
  //  microEnablePulnix::parameters
  // -----------------------------------------------------------------

  // static members

  point microEnablePulnix::parameters::camWindowSize(-1,-1);
  point microEnablePulnix::parameters::camWindowOffset(-1,-1);
  int microEnablePulnix::parameters::camCurrentBank = -1;


  microEnablePulnix::parameters::parameters() : frameGrabber::parameters() {
    serialPort = serial::parameters::Com1;
    currentBank = 0;

    bufferSize = int(8*1024*1024); // 8MB
    windowSize = point(640,480);
    windowOffset = point(104,40);

    int i;
    bank = new bankType[4];
    for (i=0;i<4;++i) {
      bank[i].setBankNumber(i);
    }
  }

  microEnablePulnix::parameters::parameters(const parameters& other)
    : frameGrabber::parameters() {
    bank = new bankType[4];
    for (int i=0;i<4;++i) {
      bank[i].setBankNumber(i);
    }

    copy( other );
  };

  microEnablePulnix::parameters::~parameters() {
    delete[] bank;
    bank = 0;
  }

  functor::parameters* microEnablePulnix::parameters::clone() const {
    return ( new parameters(*this) );
  }

  const char* microEnablePulnix::parameters::getTypeName() const {
    return "microEnablePulnix::parameters";
  }

  microEnablePulnix::parameters&
    microEnablePulnix::parameters::copy(const parameters& other ) {

#   ifndef _LTI_MSC_6
    // for normal  ANSI C++
    frameGrabber::parameters::copy(other);
#   else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    frameGrabber::parameters&
      (frameGrabber::parameters::* p_copy)(const frameGrabber::parameters&) =
      frameGrabber::parameters::copy;
    (this->*p_copy)(other);
#   endif

    serialPort = other.serialPort;
    currentBank = other.currentBank;

    bufferSize = other.bufferSize;
    windowSize = other.windowSize;
    windowOffset = other.windowOffset;

    int i;
    for (i=0;i<4;++i) {
      bank[i].copy(other.bank[i]);
    }

    return ( *this );
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool microEnablePulnix::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool microEnablePulnix::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"serialPort",static_cast<int>(serialPort));
      lti::write(handler,"currentBank",currentBank);
      lti::write(handler,"bufferSize",bufferSize);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"windowOffset",windowOffset);

      lti::write(handler,"bank0",bank[0]);
      lti::write(handler,"bank1",bank[1]);
      lti::write(handler,"bank2",bank[2]);
      lti::write(handler,"bank3",bank[3]);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && frameGrabber::parameters::write(handler,false);
# else
    bool (frameGrabber::parameters::* p_writeMS)(ioHandler&,const bool) const =
      frameGrabber::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool microEnablePulnix::parameters::write(ioHandler& handler,
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
  bool microEnablePulnix::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool microEnablePulnix::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      int s;
      b=b && lti::read(handler,"serialPort",s);
      serialPort = static_cast<serial::parameters::ePortType>(serialPort);

      b=b && lti::read(handler,"currentBank",currentBank);
      b=b && lti::read(handler,"bufferSize",bufferSize);
      b=b && lti::read(handler,"windowSize",windowSize);
      b=b && lti::read(handler,"windowOffset",windowOffset);

      _lti_debug("1: b is " << b << endl);

      b=b && lti::read(handler,"bank0",bank[0]);
      b=b && lti::read(handler,"bank1",bank[1]);
      b=b && lti::read(handler,"bank2",bank[2]);
      b=b && lti::read(handler,"bank3",bank[3]);

      _lti_debug("2: b is " << b << endl);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && frameGrabber::parameters::read(handler,false);
# else
    bool (frameGrabber::parameters::* p_readMS)(ioHandler&,const bool) =
      frameGrabber::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool microEnablePulnix::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  /*
   * upload the data in the respective bank
   */
  bool
  microEnablePulnix::parameters::upload(const serial::parameters::ePortType&
                                        serialPort,
                                        const bool allBanks,
                                        const bool force) const {
    int i;
    bool b = true;

    if (allBanks) {
      for (i=0;i<4;i++) {
        b = b && bank[i].upload(serialPort,force);
      }
    } else {
      b = bank[currentBank & 0x03].upload(serialPort,force);
    }

    if (!b) {
      return false;
    }

    if (currentBank != camCurrentBank) {
      // dummy data to change bank
      bank[currentBank].activate(serialPort);
    }

    return b;
  }

  /*
   * upload the data in the respective bank
   */
  bool
  microEnablePulnix::parameters::download(const serial::parameters::ePortType&
                                          serialPort,
                                          const bool& allBanks) {


    int i;
    bool b = true;

    currentBank = currentBank & 0x03;

    if (allBanks) {
      for (i=0;i<4;i++) {
        b = b && bank[i].download(serialPort);
      }
    } else {
      b = bank[currentBank].download(serialPort);
    }

    if (!b) {
      return false;
    }

    if (b = bank[currentBank].activate(serialPort)) {
      camCurrentBank = currentBank;
    }

    return b;
  }


  // --------------------------------

  // static members

  bool     microEnablePulnix::active      = false;
  int      microEnablePulnix::instances   = 0;
  ubyte*   microEnablePulnix::frameBuffer = 0;
  Fg_Info* microEnablePulnix::fg          = 0;
  image*   microEnablePulnix::lastImage   = 0;

  // --------------------------------

  microEnablePulnix::microEnablePulnix(const bool& initialize)
    : frameGrabber() {

    // default parameters
    parameters param;
    // set the parameters without sending them to the frame grabber or
    // camera
    frameGrabber::setParameters(param);

    instances++;

    if (initialize) {
      init();
    }

  }

  microEnablePulnix::microEnablePulnix( const parameters& theParam ) {

    setParameters(theParam);

    instances++;
  }

  microEnablePulnix::microEnablePulnix(const microEnablePulnix& other)
    : frameGrabber() {

    copy(other);

    instances++;
  }

  microEnablePulnix::~microEnablePulnix() {
    instances--;
    if (instances <= 0) {

      _lti_debug2("microEnablePulnix::destructor called and grabber freed");

      instances = 0;
      freeGrabber();
    }
  }

  const char* microEnablePulnix::getTypeName( void ) const {
    return "microEnablePulnix";
  }

  microEnablePulnix& microEnablePulnix::copy(const microEnablePulnix& other) {
    frameGrabber::copy(other);

    return (*this);
  }

  functor* microEnablePulnix::clone() const {
    return (new microEnablePulnix(*this));
  }

  const microEnablePulnix::parameters&
  microEnablePulnix::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if( params==0 )
      throw invalidParametersException(getTypeName());
    return *params;
  }

  //
  // set the parameters
  //
  bool microEnablePulnix::setParameters(const functor::parameters& theParam) {
    // first set the parameters
    frameGrabber::setParameters(theParam);

    if (!activateParameters()) {
      // if something went wrong use the previous error message
      return false;
    }

    const parameters& param = getParameters();

    return param.upload(param.serialPort);
  }


  //
  // initialization method
  //
  bool microEnablePulnix::init() {
    if (active) {
      // already done!
      return true;
    }

    _lti_debug2("microEnablePulnix::init()");

    char dummy[256];
    strcpy(dummy,"pulnix_channel_tmc6700.dll");
    // initialize microEnablePulnix frame grabber:
    if (isNull(fg = Fg_Init(dummy,0))) {
      setStatusString(getErrorMessage().c_str());
      return false;
    }

    // load configuration
    if(Fg_ConfigureEx(fg,1) < 0 ){
      setStatusString(getErrorMessage().c_str());
      return false;
    }

    // alloc memory
    if (!allocMemory()) {
      return false;
    }

    active = true;

    // activate parameters: window size and offset
    if (!activateParameters()) {
      return false;
    }

    int port = 0;

    // activate camera port
    Fg_SetParameter(fg,FG_PORT,&port,0);

    return true;
  }

  // activate parameters
  bool microEnablePulnix::activateParameters() {
    const parameters& param = getParameters();

    int tmp;

    if (param.windowSize.x != param.camWindowSize.x) {
      tmp = param.windowSize.x;
      if (Fg_SetParameter(fg, FG_WIDTH,&tmp,0) == FG_ERROR){
        setStatusString(getErrorMessage().c_str());
        return false;
      }

      param.camWindowSize.x = param.windowSize.x;
    }

    if (param.windowSize.y != param.camWindowSize.y) {
      tmp = param.windowSize.y;
      if (Fg_SetParameter(fg, FG_HEIGHT,&tmp,0) == FG_ERROR){
        setStatusString(getErrorMessage().c_str());
        return false;
      }
      param.camWindowSize.y = param.windowSize.y;
    }

    if (param.windowOffset.x != param.camWindowOffset.x) {
      tmp = param.windowOffset.x;
      if (Fg_SetParameter(fg, FG_XOFFSET,&tmp,0) == FG_ERROR){
        setStatusString(getErrorMessage().c_str());
        return false;
      }

      param.camWindowOffset.x = param.windowOffset.x;
    }


    if (param.windowOffset.y != param.camWindowOffset.y) {
      tmp = param.windowOffset.y;
      if (Fg_SetParameter(fg, FG_YOFFSET,&tmp,0) == FG_ERROR){
        setStatusString(getErrorMessage().c_str());
        return false;
      }

      param.camWindowOffset.y = param.windowOffset.y;
    }

    return true;
  }

  bool microEnablePulnix::grab(image& theImage) {
    if (active) {
      _lti_debug2("microEnablePulnix::grab(image)... ");

      //
      if (Fg_StartGrabbing(fg, START_SYNC, 0,0,0) < 0) {
        setStatusString(getErrorMessage().c_str());
        return false;
      }

      _lti_debug2("done. ");

      const parameters& param = getParameters();

      // image resize required by buffer to image
      theImage.resize(param.windowSize,rgbPixel(),false,false);
      bufferToImage(theImage,param.turnaround);

      return true;
    }

    return false;
  }

  bool microEnablePulnix::grab(channel8& theChannel) {
    if (active) {
      //
      if (Fg_StartGrabbing(fg, START_SYNC, 0,0,0) < 0) {
        setStatusString("Grabber could not be started");
        return false;
      }

      const parameters& param = getParameters();

      theChannel.resize(param.windowSize,0,false,false);
      // buffer Element is set in the call-back function with the buffer
      // to be read...
      bufferToChannel(theChannel,param.turnaround);
      return true;
    }

    return false;
  }


  void microEnablePulnix::bufferToImage(image& theImage,
                                        const bool turnaround) {

    ubyte *ptr;
    ubyte *imgptr,*imgend;

    // assuming a connected image! (valid due the the resize...)
    ptr=frameBuffer;

    if (!turnaround) {
      imgptr = (ubyte*)(&(theImage.at(0,0)));
      imgend = imgptr+(theImage.rows()*theImage.columns()*4);

      // This works on Intel only (the frameGrabber works on PCI-PC only with
      // intel processors!
      while(imgptr != imgend) {
        *imgptr=*ptr++;
        imgptr++;
        *imgptr=*ptr++;
        imgptr++;
        *imgptr=*ptr++;
        imgptr+=2;
      }
    } else {
      imgend = (ubyte*)(&(theImage.at(0,0)));
      imgptr = imgend+(theImage.rows()*theImage.columns()*4);
      // This works on Intel only (the frameGrabber works on PCI-PC only with
      // intel processors!
      while(imgptr != imgend) {
        imgptr-=4;
        *imgptr=*ptr++;
        imgptr++;
        *imgptr=*ptr++;
        imgptr++;
        *imgptr=*ptr++;
        imgptr-=2;
      }
    }
  }

  void microEnablePulnix::bufferToChannel(channel8& theChannel,
                                          const bool turnaround) {

    ubyte *ptr;
    ubyte *imgptr,*imgend;
    int tmp;

    // assuming a connected image! (valid due to the resize...)
    ptr=frameBuffer;

    if (!turnaround) {
      imgptr = (ubyte*)(&(theChannel.at(0,0)));
      imgend = imgptr+(theChannel.rows()*theChannel.columns());

      // This works on Intel only (the frameGrabber works on PCI-PC only with
      // intel processors!

      while(imgptr != imgend) {
        tmp = (*ptr++);
        tmp+=(*ptr++);
        tmp+=(*ptr++);

        *imgptr=static_cast<ubyte>(tmp/3);
        imgptr++;
      }
    } else {
      imgend = (ubyte*)(&(theChannel.at(0,0)));
      imgptr = imgend+(theChannel.rows()*theChannel.columns());

      // This works on Intel only (the frameGrabber works on PCI-PC only with
      // intel processors!

      while(imgptr != imgend) {
        tmp = (*ptr++);
        tmp+=(*ptr++);
        tmp+=(*ptr++);

        --imgptr;
        *imgptr=static_cast<ubyte>(tmp/3);
      }
    }
  }

  // ----------------------------------------------------------------

  bool microEnablePulnix::isActive() const {
    return active;
  }

  bool microEnablePulnix::allocMemory() {
    const parameters& param = getParameters();
    int i;
    i=0;
    if (notNull(frameBuffer)) {
      if (!freeMemory()) {
        return false;
      }
    }

    unsigned long bufferNr = 0;

    // the 4th parameter means the number of image buffers: 1
    // the 5th parameter means the camera number: 0
    frameBuffer = (ubyte*)Fg_AllocMem(fg,param.bufferSize,&bufferNr,1,0);

    if (isNull(frameBuffer)) {
      setStatusString(getErrorMessage().c_str());
      return false;
    }

    _lti_debug2("memory allocated. ");

    return true;
  }

  bool microEnablePulnix::freeMemory() {
    int i,j;
    j=0;
    while ((j<10) && (frameBuffer == 0)) {
      i = Fg_FreeMem(fg,0,0);
      if (i!=FG_STILL_ACTIVE) {
        frameBuffer = 0;

        _lti_debug2("microEnablePulnix::freeMemory() ok");
        return true;
      }
      j++;

      passiveWait(500000);
    }

    setStatusString("FG_Error: memory cannot be deallocated");
    return false;
  }

  bool microEnablePulnix::freeGrabber() {

    if (Fg_FreeGrabber(fg) == FG_ERROR) {
      setStatusString(getErrorMessage().c_str());
      return false;
    }

    frameBuffer = 0;
    active = false;
    fg = 0;
    _lti_debug2("microEnablePulnix::freeGrabber() ok");
    return true;
  }

  /// load an (color) image from the grabber
  bool microEnablePulnix::apply(image& theImage) {

    if (!grab(theImage)) {
      theImage.resize(0,0,rgbPixel(0,0,0),false,false);
      return false;
    }

    return true;
  }

  // load a grey value channel from the
  bool microEnablePulnix::apply(channel8& theChannel) {
    if (!grab(theChannel)) {
      theChannel.clear();
      return false;
    }

    return true;
  }

  std::string microEnablePulnix::getErrorMessage() {
    char str[256];
    std::string tmp;

    tmp = "microEnablePulnix ";
    Fg_GetLastError(fg,str,256);
    tmp+=str;
    return tmp;
  }


} // namespace lti

#endif // _USE_MICRO_ENABLE_FRAME_GRABBER
