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
 * file .......: ltiSerial.cpp
 * authors ....: Guy Wafo
 * organization: LTI, RWTH Aachen
 * creation ...: 11.4.2001
 * revisions ..: $Id: ltiSerial.cpp,v 1.14 2008/08/17 22:20:13 alvarado Exp $
 */

#include "ltiSerial.h"
#include "ltiMath.h"
#include <string>
#include <cstring>

#ifdef _LTI_MSC_VER
#include <winnt.h>
#endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG

#include <iostream>
using std::cout;
using std::endl;

#endif

namespace lti {

  // default empty string for the serial class
  const char* const serial::emptyString = "";

  // --------------------------------------------------
  // serial::parameters
  // --------------------------------------------------

  // default constructor
  serial::parameters::parameters() : ioObject() {
    baudRate = Baud9600;
    port = Com1;
    characterSize = Cs8;
    parity = No;
    stopBits = One;
    receiveTimeout = 5000; // five seconds
  }

  // copy constructor
  serial::parameters::parameters(const parameters& other) 
    : ioObject() {
    copy(other);
  }

  // destructor
  serial::parameters::~parameters() {
  }

  // get type name
  const char* serial::parameters::getTypeName() const {
    return "serial::parameters";
  }

  // copy member

  serial::parameters&
    serial::parameters::copy(const parameters& other) {

    baudRate = other.baudRate;
    port = other.port;
    characterSize = other.characterSize;
    parity = other.parity;
    stopBits = other.stopBits;
    receiveTimeout=other.receiveTimeout;

    return *this;
  }

  // alias for copy member
  serial::parameters&
    serial::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  serial::parameters* serial::parameters::clone() const {
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
  bool serial::parameters::write(ioHandler& handler,
                                         const bool& complete) const
# else
  bool serial::parameters::writeMS(ioHandler& handler,
                                           const bool& complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch (baudRate) {
          case Baud0:
            lti::write(handler,"baudRate","Baud0");
            break;
          case Baud300:
            lti::write(handler,"baudRate","Baud300");
            break;
          case Baud600:
            lti::write(handler,"baudRate","Baud600");
            break;
          case Baud1200:
            lti::write(handler,"baudRate","Baud1200");
            break;
          case Baud1800:
            lti::write(handler,"baudRate","Baud1800");
            break;
          case Baud2400:
            lti::write(handler,"baudRate","Baud2400");
            break;
          case Baud4800:
            lti::write(handler,"baudRate","Baud4800");
            break;
          case Baud9600:
            lti::write(handler,"baudRate","Baud9600");
            break;
          case Baud19200:
            lti::write(handler,"baudRate","Baud19200");
            break;
          case Baud38400:
            lti::write(handler,"baudRate","Baud38400");
            break;
          case Baud57600:
            lti::write(handler,"baudRate","Baud57600");
            break;
          case Baud76800:
            lti::write(handler,"baudRate","Baud76800");
            break;
          case Baud115200:
            lti::write(handler,"baudRate","Baud115200");
            break;
      }

      switch(port) {
          case Com1:
            lti::write(handler,"port","Com1");
            break;
          case Com2:
            lti::write(handler,"port","Com2");
            break;
          case Com3:
            lti::write(handler,"port","Com3");
            break;
          case Com4:
            lti::write(handler,"port","Com4");
            break;
          case Com5:
            lti::write(handler,"port","Com5");
            break;
          case Com6:
            lti::write(handler,"port","Com6");
            break;
          case Com7:
            lti::write(handler,"port","Com7");
            break;
          case Com8:
            lti::write(handler,"port","Com8");
            break;
          case USB0:
            lti::write(handler,"port","USB0");
            break;
          case USB1:
            lti::write(handler,"port","USB1");
            break;
          default:
            lti::write(handler,"port","Com1");
      }

      switch (characterSize) {
          case Cs4:
            lti::write(handler,"characterSize","Cs4");
            break;
          case Cs5:
            lti::write(handler,"characterSize","Cs5");
            break;
          case Cs6:
            lti::write(handler,"characterSize","Cs6");
            break;
          case Cs7:
            lti::write(handler,"characterSize","Cs7");
            break;
          default:
            lti::write(handler,"characterSize","Cs8");
            break;
      }

      switch (parity) {
          case No:
            lti::write(handler,"parity","No");
            break;
          case Even:
            lti::write(handler,"parity","Even");
            break;
          case Odd:
            lti::write(handler,"parity","Odd");
            break;
          case Space:
            lti::write(handler,"parity","Space");
            break;
          case Mark:
            lti::write(handler,"parity","Mark");
            break;
          default:
            lti::write(handler,"parity","No");
            break;
      }

      switch (stopBits) {
        case One:
          lti::write(handler,"stopBits","One");
          break;
        case OneFive:
          lti::write(handler,"stopBits","OneFive");
          break;
        case Two:
          lti::write(handler,"stopBits","Two");
          break;
        default:
          lti::write(handler,"stopBits","One");
          break;
      }

      lti::write(handler,"receiveTimeout",receiveTimeout);
    }



    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool serial::parameters::write(ioHandler& handler,
                                         const bool& complete) const {
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
  bool serial::parameters::read(ioHandler& handler,
                                        const bool& complete)
# else
  bool serial::parameters::readMS(ioHandler& handler,
                                          const bool& complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string tmp;

      lti::read(handler,"baudRate",tmp);
      if (tmp == "Baud0") {
        baudRate = Baud0;
      } else if (tmp == "Baud300") {
        baudRate = Baud300;
      } else if (tmp == "Baud600") {
        baudRate = Baud600;
      } else if (tmp == "Baud1200") {
        baudRate = Baud1200;
      } else if (tmp == "Baud1800") {
        baudRate = Baud1800;
      } else if (tmp == "Baud2400") {
        baudRate = Baud2400;
      } else if (tmp == "Baud4800") {
        baudRate = Baud4800;
      } else if (tmp == "Baud9600") {
        baudRate = Baud9600;
      } else if (tmp == "Baud19200") {
        baudRate = Baud19200;
      } else if (tmp == "Baud38400") {
        baudRate = Baud38400;
      } else if (tmp == "Baud57600") {
        baudRate = Baud57600;
      } else if (tmp == "Baud76800") {
        baudRate = Baud76800;
      } else if (tmp == "Baud115200") {
        baudRate = Baud115200;
      } else {
        // default value if unknown tag
        baudRate = Baud9600;
      }

      lti::read(handler,"port",tmp);
      if (tmp == "Com1") {
        port = Com1;
      } else if (tmp == "Com2") {
        port = Com2;
      } else if (tmp == "Com3") {
        port = Com3;
      } else if (tmp == "Com4") {
        port = Com4;
      } else if (tmp == "Com5") {
        port = Com5;
      } else if (tmp == "Com6") {
        port = Com6;
      } else if (tmp == "Com7") {
        port = Com7;
      } else if (tmp == "Com8") {
        port = Com8;
      } else if (tmp == "USB0") {
        port = USB0;
      } else if (tmp == "USB1") {
        port = USB1;
      } else {
        // default value
        port = Com1;
      }

      lti::read(handler,"characterSize",tmp);

      if (tmp == "Cs4") {
        characterSize = Cs4;
      } else if (tmp == "Cs5") {
        characterSize = Cs5;
      } else if (tmp == "Cs6") {
        characterSize = Cs6;
      } else if (tmp == "Cs7") {
        characterSize = Cs7;
      } else if (tmp == "Cs8") {
        characterSize = Cs8;
      } else {
        // default value
        characterSize = Cs8;
      }

      lti::read(handler,"parity",tmp);
      if (tmp == "No") {
        parity = No;
      } else if (tmp == "Even") {
        parity = Even;
      } else if (tmp == "Odd") {
        parity = Odd;
      } else if (tmp == "Space") {
        parity = Space;
      } else if (tmp == "Mark") {
        parity = Mark;
      } else {
        // default value
        parity = No;
      }

      lti::read(handler,"stopBits",tmp);
      if (tmp == "One") {
        stopBits = One;
      } else if (tmp == "OneFive") {
        stopBits = OneFive;
      } else if (tmp == "Two") {
        stopBits = Two;
      } else {
        // default value
        stopBits = One;
      }

      lti::read(handler,"receiveTimeout",receiveTimeout);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool serial::parameters::read(ioHandler& handler,
                                        const bool& complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // serial
  // --------------------------------------------------

  // default constructor
  serial::serial()
    : object(),statusString(0) {

    isPortOpened=false;

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  serial::serial(const serial::parameters& serialPar)
    : object(),statusString(0) {

    isPortOpened=false;

    // set the default parameters
    setParameters(serialPar);
  }

  // copy constructor
  serial::serial(const serial& other)
    : object(),statusString(0) {
    copy(other);
  }

  // destructor
  serial::~serial() {
    delete[] statusString;
    statusString = 0;
  }

  // returns the name of this type
  const char* serial::getTypeName() const {
    return "serial";
  }


  // copy member
   serial& serial::copy(const serial& other) {

     closePort();
     param.copy(other.param);
     return *this;
  }

  // return parameters
  const serial::parameters&
    serial::getParameters() const {
    return param;
  }

  //set parameters
  bool serial::setParameters(const parameters& sParameters) {
    param.copy(sParameters);
    return true;
  }
  /*
   * return the last message set with setStatusString().  This will
   * never return 0.  If no status-string has been set yet an empty string
   * (pointer to a string with only the char(0)) will be returned.
   */
  const char* serial::getStatusString() const {
    if (isNull(statusString)) {
      return emptyString;
    } else {
      return statusString;
    }
  }

  /*
   * set a status string.
   *
   * @param msg the const string to be reported next time by
   * getStatusString()
   * This message will be usually set within the apply methods to indicate
   * an error cause.
   */
  void serial::setStatusString(const char* msg) const {
    delete[] statusString;
    statusString = 0;

    statusString = new char[strlen(msg)+1];
    strcpy(statusString,msg);
  }

  bool serial::openPort(){

    if(isPortOpened){
      closePort();
    }

#ifdef _LTI_MSC_VER
    const char* portName;

    switch(param.port){
        case parameters::Com1 :  portName = "COM1"; break;
        case parameters::Com2 :  portName = "COM2"; break;
        case parameters::Com3 :  portName = "COM3"; break;
        case parameters::Com4 :  portName = "COM4"; break;
        case parameters::Com5 :  portName = "COM5"; break;
        case parameters::Com6 :  portName = "COM6"; break;
        case parameters::Com7 :  portName = "COM7"; break;
        case parameters::Com8 :  portName = "COM8"; break;
        case parameters::USB0 :  portName = "USB0"; break;
        case parameters::USB1 :  portName = "USB1"; break;
        default:                 portName = "COM1"; break;
    };

    DCB	dcb; // Device Control Block

    BOOL ioError = 0;

    // opening serial interface
    hHandle=CreateFile(portName, // the port name
                       GENERIC_READ | GENERIC_WRITE,
                       0,0,
                       OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0); // nonoverlaped

    if (hHandle != INVALID_HANDLE_VALUE) {
      // in case of success

      if(GetCommState(hHandle,&dcb)) {  // fetch DeviceControllBlock

        switch(param.baudRate) {

            case parameters::Baud600    : dcb.BaudRate = CBR_600;    break;
            case parameters::Baud1200   : dcb.BaudRate = CBR_1200;   break;
            case parameters::Baud2400   : dcb.BaudRate = CBR_2400;   break;
            case parameters::Baud4800   : dcb.BaudRate = CBR_4800;   break;
            case parameters::Baud9600   : dcb.BaudRate = CBR_9600;   break;
            case parameters::Baud19200  : dcb.BaudRate = CBR_19200;  break;
            case parameters::Baud38400  : dcb.BaudRate = CBR_38400;  break;
            case parameters::Baud57600  : dcb.BaudRate = CBR_57600;  break;
            case parameters::Baud115200 : dcb.BaudRate = CBR_115200; break;
            default :                     dcb.BaudRate = CBR_9600;   break;
        }

        dcb.ByteSize = (unsigned char) param.characterSize;

        switch(param.parity) {

            case parameters::Odd   : dcb.Parity=ODDPARITY;   break;
            case parameters::Even  : dcb.Parity=EVENPARITY;  break;
            case parameters::Mark  : dcb.Parity=MARKPARITY;  break;
            case parameters::Space : dcb.Parity=SPACEPARITY; break;
            case parameters::No    : dcb.Parity=NOPARITY;    break;
            default                : dcb.Parity=NOPARITY;    break;
        }

        switch(param.stopBits){

          case parameters::One    : dcb.StopBits = ONESTOPBIT;   break;
          case parameters::OneFive: dcb.StopBits = ONE5STOPBITS; break;
          case parameters::Two    : dcb.StopBits = TWOSTOPBITS;  break;
          default	          : dcb.StopBits = ONESTOPBIT;   break;
        }

        if(!SetCommState(hHandle,&dcb)) {    // set device settimgs
          closePort(); // in case of invalid settings : close port
          return false;
        }
      } else {
        // some error occured

        setStatusString("error:can not access the control setting parameters");
        closePort(); // DCB not accessible, close port
        return false;
      }

    } else {
      // handle invalid
      setStatusString("error: the port could not be opened");
      return false;	 // COMx could not be opened, terminate thread
    }

    timeOut.ReadIntervalTimeout = MAXDWORD;
    timeOut.ReadTotalTimeoutMultiplier = MAXDWORD;
    timeOut.ReadTotalTimeoutConstant = param.receiveTimeout; // in ms
    timeOut.WriteTotalTimeoutMultiplier = 0;
    timeOut.WriteTotalTimeoutConstant = 0;

    ioError = SetCommTimeouts(hHandle, &timeOut);
    if (ioError == 0) {
      setStatusString("error: time out could not be set");
      return false;
    }

    SetCommMask(hHandle,EV_RXCHAR);

    _lti_debug(" the port: Port " << portName << 
               " was opened successfully!" << endl);

#else
    // POSIX Version:

    struct termios allParameters;

    // original flags
    int flags = O_RDWR | O_NOCTTY; // | O_NDELAY
    
    // flags used in serial of robotic group
    // int flags = O_CREAT|O_RDWR|O_NONBLOCK

    switch(param.port) {
        case parameters::Com1: hHandle=open("/dev/ttyS0",flags); break;
        case parameters::Com2: hHandle=open("/dev/ttyS1",flags); break;
        case parameters::Com3: hHandle=open("/dev/ttyS2",flags); break;
        case parameters::Com4: hHandle=open("/dev/ttyS3",flags); break;
        case parameters::Com5: hHandle=open("/dev/ttyS4",flags); break;
        case parameters::Com6: hHandle=open("/dev/ttyS5",flags); break;
        case parameters::Com7: hHandle=open("/dev/ttyS6",flags); break;
        case parameters::Com8: hHandle=open("/dev/ttyS7",flags); break;
        case parameters::USB0: hHandle=open("/dev/ttyUSB0",flags); break;
        case parameters::USB1: hHandle=open("/dev/ttyUSB1",flags); break;
        default:               hHandle=open("/dev/ttyS0",flags); break;
    }

    if(hHandle != -1) {

      //get the current parameters for the port
      if (tcgetattr(hHandle,&allParameters) != -1) {

        // first than all, clear everything!
        allParameters.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                                   |INLCR|IGNCR|ICRNL);
        // Raw output option,similary as the raw input.The characters
        // are transmitted as-is
        allParameters.c_oflag &= ~OPOST;
        allParameters.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
        allParameters.c_cflag &= ~(CSIZE|PARENB);

        // now set the parameters chosen by the user:
        switch(param.baudRate) {

            case parameters::Baud0:
              cfsetispeed(&allParameters,B0);
              cfsetospeed(&allParameters,B0);
              break;
            case parameters::Baud300:
              cfsetispeed(&allParameters,B300);
              cfsetospeed(&allParameters,B300);
              break;
            case parameters::Baud600:
              cfsetispeed(&allParameters,B600);
              cfsetospeed(&allParameters,B600);
              break;
            case parameters::Baud1200:
              cfsetispeed(&allParameters,B1200);
              cfsetospeed(&allParameters,B1200);
              break;
            case parameters::Baud1800:
              cfsetispeed(&allParameters,B1800);
              cfsetospeed(&allParameters,B1800);
              break;
            case parameters::Baud2400:
              cfsetispeed(&allParameters,B2400);
              cfsetospeed(&allParameters,B2400);
              break;
            case parameters::Baud4800:
              cfsetispeed(&allParameters,B4800);
              cfsetospeed(&allParameters,B4800);
              break;
            case parameters::Baud9600:
              cfsetispeed(&allParameters,B9600);
              cfsetospeed(&allParameters,B9600);
              break;
            case parameters::Baud19200:
              cfsetispeed(&allParameters,B19200);
              cfsetospeed(&allParameters,B19200);
              break;
            case parameters::Baud38400:
              cfsetispeed(&allParameters,B38400);
              cfsetospeed(&allParameters,B38400);
              break;
            case parameters::Baud57600:
              cfsetispeed(&allParameters,B57600);
              cfsetospeed(&allParameters,B57600);
              break;
            case parameters::Baud76800:
              setStatusString("Baud76800 not supported");
              return false;
              break;
            case parameters::Baud115200:
              cfsetispeed(&allParameters,B115200);
              cfsetospeed(&allParameters,B115200);
              break;
            default:
              cfsetispeed(&allParameters,B9600);
              cfsetospeed(&allParameters,B9600);
        }

        switch(param.parity) {

            case parameters::Odd:
              allParameters.c_cflag |= PARENB; // enable parity
              allParameters.c_cflag |= PARODD; // odd parity
              allParameters.c_iflag |= (INPCK | ISTRIP);
              break;

            case parameters::Even:
              allParameters.c_cflag |= PARENB;  // enable parity
              allParameters.c_cflag &= ~PARODD; // even parity
              allParameters.c_iflag |= (INPCK | ISTRIP);
              break;

            case parameters::Space:
            case parameters::No:
              allParameters.c_cflag &= ~PARENB;
              break;
            default:
              allParameters.c_cflag &= ~PARENB;
        }

        allParameters.c_cflag &= ~CSIZE;    // mask the character size bits

        switch(param.characterSize){
            case parameters::Cs5: allParameters.c_cflag |=CS5; break;
            case parameters::Cs6: allParameters.c_cflag |=CS6; break;
            case parameters::Cs7: allParameters.c_cflag |=CS7; break;
            case parameters::Cs8: allParameters.c_cflag |=CS8; break;
            default:              allParameters.c_cflag |=CS8; break;
        }

        if (param.stopBits == parameters::Two) { //if the stop bit is
           //set,2 stop bits are used.Otherwise,only 1 stop bit is used.
          allParameters.c_cflag |= CSTOPB;   //set the stop bit.
        } else {
          allParameters.c_cflag &= ~CSTOPB;   //clear the 2 stop bit.
        }

        // These ones should always be set:
        allParameters.c_cflag |= CREAD;  // enable receiver
        allParameters.c_cflag |= CLOCAL; // Local line -
                                         // do not change "owner" of port

        // software flow control disabled (TODO: this could also be
        // somehow parameterized)
        allParameters.c_iflag &= ~(IXON | IXOFF | IXANY);

        // Raw input.
        // Input characters are passed through exactly as they are received
        allParameters.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);

        //
        // Taken from: http://www.easysw.com/~mike/serial/serial.html#2_5_2
        //
        // VMIN specifies the minimum number of characters to read. If
        // it is set to 0, then the VTIME value specifies the time to
        // wait for every character read. Note that this does not mean
        // that a read call for N bytes will wait for N characters to
        // come in. Rather, the timeout will apply to the first
        // character and the read call will return the number of
        // characters immediately available (up to the number you
        // request).
        //
        // If VMIN is non-zero, VTIME specifies the time to wait for
        // the first character read. If a character is read within the
        // time given, any read will block (wait) until all VMIN
        // characters are read. That is, once the first character is
        // read, the serial interface driver expects to receive an
        // entire packet of characters (VMIN bytes total). If no
        // character is read within the time allowed, then the call to
        // read returns 0. This method allows you to tell the serial
        // driver you need exactly N bytes and any read call will
        // return 0 or N bytes. However, the timeout only applies to
        // the first character read, so if for some reason the driver
        // misses one character inside the N byte packet then the read
        // call could block forever waiting for additional input
        // characters.
        //
        // VTIME specifies the amount of time to wait for incoming
        // characters in tenths of seconds. If VTIME is set to 0 (the
        // default), reads will block (wait) indefinitely unless the
        // NDELAY option is set on the port with open or fcntl.

        // number of characters to be received (zero means wait until anything
        // comes!
        allParameters.c_cc[VMIN]=0;
        //time to wait in deciseconds to receive every character,while VMIN=0
        allParameters.c_cc[VTIME]=iround(param.receiveTimeout/100.0);     

        tcflush(hHandle, TCIFLUSH);

        if (tcsetattr(hHandle,TCSANOW,&allParameters)==-1) {
          //set all the parameters without waiting for data to complete
          setStatusString("could not set the parameters");
          closePort();
          return false;
        }
      } else {
        setStatusString("error: unable to get the current parameters");
        closePort();
        return false;
      }
    } else {
      setStatusString(strerror(errno));
      closePort();
      return false;
    }

    fcntl(hHandle,F_SETFL,0);

#endif

    isPortOpened=true;
    return isPortOpened;
  }

  //sends only one character
  bool serial::send(const unsigned char c) {
    return send(static_cast<char>(c));
  }

  bool serial::send(const char c) {

    if (!isPortOpened){
      if (!openPort()) {
        // some error occured by opening port
        return false;
      }
    }

#ifdef _LTI_MSC_VER

    BOOL ioError = 0;
    DWORD numberOfBytes;

    ioError = WriteFile(hHandle,&c,1,&numberOfBytes,0);
    if (ioError == 0 || numberOfBytes!=1) {
      setStatusString("error while writing a character to the serial port");
      return false;
    }

    return true;

#else
    int numberOfBytes;
    numberOfBytes = ::write(hHandle,&c,1);
    if(numberOfBytes==1) {
      return true;
    } else {
      setStatusString("error: could not send a character");
      return false;
    }
#endif
  }

  //sends a string
  bool serial::send(const std::string& theString) {

    if(!isPortOpened) {
      if (!openPort()) {
        // some error occured by opening port
        return false;
      }
    }

    int counter=0;

    while((counter < static_cast<int>(theString.size())) &&
          (send(theString[counter])==true)) {
      counter++;
    }

    return (counter == static_cast<int>(theString.size()));
  }

  //sends a string
  bool serial::send(const vector<ubyte>& theString) {

    if(!isPortOpened) {
      if (!openPort()) {
        // some error occured by opening port
        return false;
      }
    }

    int counter=0;

    while((counter < theString.size()) &&
          (send(theString[counter])==true)) {
      counter++;
    }

    return (counter == static_cast<int>(theString.size()));
  }

  bool serial::receive(unsigned char& c) {
    char* tmp = reinterpret_cast<char*>(&c);
    return receive(*tmp);
  }

  //receives only one character
  bool serial::receive(char& c) {

    if (!isPortOpened) {
      if (!openPort()) {
        // some error occured by opening port
        return false;
      }
    }

#ifdef _LTI_MSC_VER
    BOOL ioError;
    DWORD numberOfBytes;
    ioError=ReadFile(hHandle,&c,1,&numberOfBytes,0);

    if (ioError==0 || numberOfBytes != 1) {
      c = 0;
      setStatusString("error: could not receive the character");
      return false;
    }

    return true;
#else

    int numberOfBytes;
    numberOfBytes = ::read(hHandle,&c,1);

    if (numberOfBytes==1) {
      _lti_debug("{"<<static_cast<int>(static_cast<unsigned char>(c))<<"}");
      return true;
    } else {
      setStatusString("error: could not receive the character");
      return false;
    }
#endif
  }

  //receives a string
  bool serial::receive(std::string& theString) {

    bool control=false;
    char ch;

    theString = "";

    do {
      if ( (control = receive(ch)) ) {
        theString += ch;
      }
    } while ((ch!='\n') && (ch!=0) && control);

    return control;
  }

  // receives a string
  bool serial::receive(vector<ubyte>& theString) {

    bool control=true;
    char ch;

    int i=0;
    for (i=0;control && (i<theString.size());++i) {
      if ( (control = receive(ch)) ) {
        theString[i] = static_cast<ubyte>(ch);
      }
    }

    return control;
  }


  // close the port
  void serial::closePort() {
#ifdef _LTI_MSC_VER
    CloseHandle(hHandle);
    hHandle = INVALID_HANDLE_VALUE; // an invalid handle
#else
    if(close(hHandle)==-1) {
      setStatusString("error: the port could not be closed");
    }
    hHandle=-1;
#endif
    isPortOpened=false;
  }

}
