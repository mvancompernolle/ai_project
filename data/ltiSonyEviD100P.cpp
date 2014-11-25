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
 * file .......: ltiSonyEviD100P.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 28.8.2003
 * revisions ..: $Id: ltiSonyEviD100P.cpp,v 1.13 2006/09/05 10:37:42 ltilib Exp $
 */

#include "ltiSonyEviD100P.h"
#include "ltiTimer.h"
#include "ltiConstants.h"
#include "ltiMath.h"
#include <iostream>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 4
#include "ltiDebug.h"

using std::cout;
using std::cerr;
using std::hex;
using std::dec;
using std::flush;
using std::endl;
namespace lti {

  //constants
  const float sonyEviD100P::m_panFactor = 1440.f;
  const float sonyEviD100P::m_tiltFactor = 360.f;
  const float camera::parameters::minGain = -3.f;
  const float camera::parameters::maxGain = 18.f;
  const float camera::parameters::minRBGain = 0.f;
  const float camera::parameters::maxRBGain = 256.f;
  const float camera::parameters::minShutterSpeed = 1.f/30.f;
  const float camera::parameters::maxShutterSpeed = .0001f;
  const float camera::parameters::minFocus = .09f;
  const float camera::parameters::maxFocus = 22.f;
  const float camera::parameters::maxZoom = 40.f;

  const float sonyEviD100P::parameters::minPanSpeed = 2.f;
  const float sonyEviD100P::parameters::maxPanSpeed = 300.f;
  const float sonyEviD100P::parameters::minTiltSpeed = 2.f;
  const float sonyEviD100P::parameters::maxTiltSpeed = 125.f;
  const int sonyEviD100P::parameters::maxBrightness = 23;
  const int sonyEviD100P::parameters::minExposureCompensation = -7;
  const int sonyEviD100P::parameters::maxExposureCompensation = 7;
  const int sonyEviD100P::parameters::maxAperture  = 15;
  const int sonyEviD100P::parameters::maxStillLevel = 32;
  const int sonyEviD100P::parameters::maxFlashLevel = 24;
  const int sonyEviD100P::parameters::maxLumiLevel = 32;
  const int sonyEviD100P::parameters::maxTrailLevel = 24;
  const int sonyEviD100P::parameters::minFocusSpeed = 0;
  const int sonyEviD100P::parameters::maxFocusSpeed = 7;
  const float sonyEviD100P::parameters::maxOpticalZoom = 10.f;
  const float sonyEviD100P::parameters::minZoomSpeed = 0.f;
  const float sonyEviD100P::parameters::maxZoomSpeed = 7.f;

  /*-----------------------------------------
   * Poll Helper
   *----------------------------------------*/
  //Constructor
  sonyEviD100P::poll::poll(serial::parameters::ePortType port, int waitForInit)
    : m_camAddress(0x01),
      m_recAddress(0x00),
      m_ifClear(false),
      m_waitForInit(waitForInit),
      m_panPos(0), m_tiltPos(0),
      m_zoomPos(0), m_focusPos(0),
      m_focusNearLimitPos(0),
      m_autoFocus(0),
      m_numPosInquiries(0),
      m_numBlockInquiries(0),
      m_numPosCalls(0),
      m_numLensCalls(0),
      m_pollLens(false),
      m_pollPanTilt(true),
      m_length(0),
      m_referenceCount(1) {
    //Initialize the serial port
    serial::parameters spPara;
    spPara.baudRate = serial::parameters::Baud9600;
    spPara.port = port;
    spPara.characterSize = serial::parameters::Cs8;
    spPara.parity = serial::parameters::No;
    spPara.stopBits = serial::parameters::One;
    spPara.receiveTimeout = 0;  //in s, we do no want to wait
    if (! m_serialPort.setParameters(spPara) ) {
      throw exception("sonyEviD100P::poll: Could not set serial port parameter");
    }
    //Initialize camera   
    //Network initialisation as device see init m_camAddress
    //m_[cam/rec]Address will be modified as a side effect
    if (!initCamNetwork()) {
      throw exception ("sonyEviD100P::poll: Could not initialize the camera network.");
    }
    //OOOOLD CODE: this only works after IFClear has been received
    //Switch power on
    //{
    //ubyte buffer[6] = {0x00, 0x01, 0x04, 0x00, 0x02, 0xFF};
    //vector<ubyte> command(6, buffer, true);
    //send(std::string("Power On"), command);
    //}
  }

  //Destructor
  sonyEviD100P::poll::~poll() {
  }

  bool sonyEviD100P::poll::initCamNetwork() {
    {
      ubyte buffer[4] =  {0x88, 0x30, m_camAddress, 0xFF};
      vector<ubyte> command(4, buffer, true);
      m_ifClear = true;      //ugly
      if (!send(std::string("Set camera address"), command)) {
	return false;
      }
    }
 
    //Send IF Clear by broadcast
    {
      ubyte buffer[5] = {0x88, 0x01, 0x00, 0x01, 0xFF};
      vector<ubyte> command(5, buffer, true);
      if (!send(std::string("IF_Clear (broadcast)"), command)) {
	return false;
      }
      m_ifClear = false;      //actually this will be set by processMessages()
    }
    passiveWait (m_waitForInit*1000);  //i.e. 1000 * 100 = 100000
    processMessages();
 
    return true;
  }

  void sonyEviD100P::poll::enroll() {
    m_registrateMutex.lock();
    m_referenceCount++;
    m_registrateMutex.unlock();
  }

  int sonyEviD100P::poll::signOff() {
    m_registrateMutex.lock();
    int tmp = --m_referenceCount;
    m_registrateMutex.unlock();
    return tmp;
  }

  const char* sonyEviD100P::poll::getTypeName() const {
    return "poll";
  }

  //Send a command to the camera. Thread Safe?
  //If the first character equals 0, it will be replaced by the camera address.
  bool sonyEviD100P::poll::send
  (const std::string& description, vector<ubyte>& command) {

    if ( !m_ifClear ) {
      return false;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    cout << " sending " << description << endl << flush;
#endif

    m_communicateMutex.lock();
    if (command[0] == 0) { command[0]  = m_camAddress; }
    if (!m_serialPort.send(command)) {
      m_communicateMutex.unlock();
      throw("sonyEviD100P::poll::send: Could not access serial port");
    }
    m_communicateMutex.unlock();
    return true;
  }
 
  //Poll for lens values
  void sonyEviD100P::poll::pollLens(bool on) {
    m_pollLens = on;
  }
  void sonyEviD100P::poll::pollPanTilt(bool on) {
    m_pollPanTilt = on;
  }

  //Get the current pan and tilt position.
  void sonyEviD100P::poll::getPanTilt(float& pan, float& tilt) const {
    m_bufferMutex.lock();
    pan =  static_cast<float>(m_panPos);
    tilt = static_cast<float>(m_tiltPos);
    m_bufferMutex.unlock();
  }

  int sonyEviD100P::poll::getPan()  const {
    return m_panPos;
  }

  int sonyEviD100P::poll::getTilt()  const {
    return m_tiltPos;
  }

  //set the current pan and tilt position.
  void sonyEviD100P::poll::setPanTilt(int pan, int tilt) {
    m_bufferMutex.lock();
    m_panPos = pan;
    m_tiltPos = tilt;
    m_bufferMutex.unlock();
  }

  //Get the current lens setting. I consider the focus near limit unimportant.
  void sonyEviD100P::poll::getLensSetting(int& focus, float& zoom) const {
    m_bufferMutex.lock();
    zoom =  static_cast<float>(m_zoomPos);
    focus = m_focusPos;
    m_bufferMutex.unlock();
  }

  //Get the current zoom position. 
  int sonyEviD100P::poll::getZoom()  const {
    return m_zoomPos;
   }

  //Get the current focus position.  
  int sonyEviD100P::poll::getFocus()  const {
    return m_focusPos;
   }

  //Set the current zoom position. 
  void sonyEviD100P::poll::setLensSetting(int zoom, int focus, 
					  int focusNearLimit) {
    m_bufferMutex.lock();
    m_zoomPos = zoom;
    m_focusPos = focus;
    m_focusNearLimitPos = focusNearLimit;
    m_bufferMutex.unlock();
  }

  //Get the current focus near limit position.  
  int sonyEviD100P::poll::getFocusNearLimit()  const {
    return m_focusNearLimitPos;
  }  

  //Remember focus mode.
  void sonyEviD100P::poll::rememberFocusMode(bool mode) {
    m_autoFocus = mode;
  }

  //Get focus mode.
  bool sonyEviD100P::poll::getFocusMode() const{
    return m_autoFocus;
  }

  //send inquiry commands
  // - only inquire settings which are unknown
  // - i.e. user specified speed and actual camera speed 
  //   can be asumed identical
  //CAUTION: use this function only in the constructor and threadMain.
  bool sonyEviD100P::poll::inquiry() { 

    m_inquiryMutex.lock();
    //max. 1 inquiry on way due to nonsense answers when inquiring pan too fast
    //Avoids unnecessary traffic to the camera. Also inquiries to the
    // lens system have a higher priority and override the pan tilt inquiry.
    //Price: 1) Hangs for a little while if the answers get lost. This
    //          did not happen in the tests though.
    //       2) too few inquiries are send if the polling frequency is too fast
    //Other Idea: 
    //  1) add timeout for old inquiries
    //  2) remember time of last inquiry and wait till min. time has elapsed
    if (m_pollPanTilt) {
      m_numPosCalls++;
      if (m_numPosInquiries < 3) { 
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
	cout << " ----- send pos inquiry" << endl;
#endif
	//pan-,tilt-inquiry
	m_numPosCalls = 0;
	ubyte buffer[5] =  {0,0x09,0x06,0x12,0xFF};
	vector<ubyte> command(5, buffer, true);
	if (!send(std::string("PosInquiry"), command))	{
	  m_inquiryMutex.unlock();
	  return false;
	}
	m_numPosInquiries++;
      }
      //Avoid hang ups for ever if answers got lost
      if (m_numPosCalls > 10) {   //ToDo: Make this a parameter
	m_numPosCalls = 0;
	m_numPosInquiries = 0;
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
	cout << endl 
	     << "SonyEviD100P: PollOnce: WARNING: Lost answers to pan tilt inquiry." 
	     << endl;
#endif
      }
    }

    //CAUTION: camera does not preserve order of single lens commands,
    //when lens system is moving while inquiry

    //zoom, focus near limit position and focus - higher priority than pan/tilt inquiry
    //this commands slows down frequency of returned positions
    // => call less frequently
    //ToDo: issue either posCalls or lensCalls
    if (m_pollLens) {
      m_numLensCalls++;
      if (m_numBlockInquiries < 3) {
	m_numLensCalls = 0;
	ubyte buffer[6] = {0,0x09, 0x7E, 0x7E, 0x00,0xFF};
	vector<ubyte> command(6, buffer, true);
	if (!send(std::string("lens1Inquiry"), command)) {
	  m_inquiryMutex.unlock();
	  return false;
	}
	m_numBlockInquiries++;
      }
      //Avoid hang ups for ever if answers got lost
      if (m_numLensCalls > 10) { //no answers for about a second or half a second
	m_numLensCalls = 0;
	m_numBlockInquiries = 0;
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
	cout << endl 
	     << "SonyEviD100P: PollOnce: WARNING: Lost answers to lens inquiry." 
	     << endl;
#endif
      }
    }
    m_inquiryMutex.unlock();

    return ( m_numPosCalls == 0 || m_numLensCalls == 0);
  }
  
  //check returned messages
  void sonyEviD100P::poll::processMessages() {
    //unsigned char SocketNumber;  //is here for debugging
 
    //cout << "...processing messages..." << flush << endl;
    bool message(true);
    while (message) {

      ubyte tmp(0xFF);
      bool recData(false);
      
      m_communicateMutex.lock();
      while ( ( recData = m_serialPort.receive(tmp) ) == true  //received data
              && tmp != 0xFF       //end of message
              && m_length < 15 ) { //max. message length (FF is not stored)
        
        m_recBuffer[m_length] = tmp;
        m_length++;
      }
      m_communicateMutex.unlock();
      
      int length = m_length;  //remember "old" length, used as double check later on
      if (recData == false) {
        //did not receive a complete message
        //get the remainder next time
        message=false;
        break;
      } else {
        //reset buffer of received messages
        m_length=0;
      }
      
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
     if ( message ) {
       cout << "Received: ";
       int i;
       for(i=0;i<length;i++) { 
	 cout<< hex << int(m_recBuffer[i]) << " "; 
       }
       cout << hex << int(tmp) //<< " recData " << dec << recData
	    << dec << " length " << m_length << endl;
     }
#endif

     //Initialisation messages
     if ( m_recBuffer[0] == 0x88 ) {
       if ( m_recBuffer[1] == 0x30 && length == 3) { //Address set
	 m_camAddress = m_recBuffer[2] + 0x7F;
	 m_recAddress = (m_recBuffer[2] + 7)*0x10;
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	 cout << "sonyEviD100P: set address " << hex << int(m_camAddress) << " " 
	      << int(m_recAddress) << endl;
#endif
	 continue;
       }
       if (m_recBuffer[1] == 0x01 && m_recBuffer[2] == 0x00 
	   && m_recBuffer[3] == 0x01 && length == 4) { //IF_Clear (broadcast)
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	 cout << "sonyEviD100P: received IF_CLear (broadcast)." << endl;
#endif
	 m_ifClear = true;
	 continue;
       }
     } //88
     if ( m_recBuffer[0] == m_recAddress ) { //recAddress
       if ( m_recBuffer[1] == 0x50 && length == 2) { //IF_Clear (for x)
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	 cout << "sonyEviD100P: received IF_CLear (for x). " << endl;
#endif
	 m_ifClear = true;
	 continue;
       }
       if ( m_recBuffer[1] == 0x38 && length == 2 ) {  //Network change message
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	 cerr << "sonyEviD100P: Network change." << endl
	    << "   Power was routed or VISCA OUT jack was (dis)connected." << endl;
#endif
	 m_ifClear = false;
	 if (!initCamNetwork()) { 
	   throw 
	     exception ("sonyEvi D100P::processMessages: Could not initialize the camera network.");
	 }
	 continue;
       }

       //ERRORS
       if ( (m_recBuffer[1] == 0x60 || m_recBuffer[1] == 0x61 ||
	     m_recBuffer[1] == 0x62)
	    && length == 3 ) {
	 if ( m_recBuffer[2] == 0x01 ) {
	   //SocketNumber=m_recBuffer[1]-0x60;
	   //cout<<"socket number: "<<SocketNumber;
	   //cerr << "sonyEviD100P: "<< "\nMessage lenght error(>14bytes)" << endl;
	   throw  exception("sonyEviD100P: Message lenght error(>14bytes)");
	 }
	 if ( m_recBuffer[2] == 0x02 ) {
	   //cerr << "sonyEviD100P: Syntax Error" << endl;
	   throw exception("sonyEviD100P: Syntax Error");
	 }
	 if ( m_recBuffer[2] == 0x03 )  {	     
	   //cerr << "sonyEviD100P: Command buffer full" << endl;
	   throw exception("sonyEviD100P: Command buffer full"); //maybe add an overflow exception
	 }
	 if ( m_recBuffer[2] == 0x04 ) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	   cerr << "sonyEviD100P: Command canceled\n\t" << endl;
#endif
	   continue;
	   //return message of command canceled is no error
	 }  
	 if ( m_recBuffer[2] == 0x05 ) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	   cerr << "sonyEviD100P: no socket (to be canceled or invalid socket number)\n"
		<< "\t at socket " << hex << int(m_recBuffer[1] - 0x60) << endl;
#endif
	   continue;
	   //return message of no socket is no error, command will be canceled
	   //command is not repeated, because it might be outdated
	 }  
	 if ( m_recBuffer[2] == 0x41 ) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	   cerr << "sonyEviD100P: Command not executable" << endl;
#endif
	   //no error ? i.e. camera does not like StopZoom
	   //throw  RBComError("SonyEviD100P: "+cmd+"\nCommand not executable");
	   continue;
	 }
       } //errors
       //ACK  
       //if ( (m_recBuffer[1] == 0x41 || m_recBuffer[1] == 0x42) && (length == 2) )
       // {
       //	   continue;
       // }
       //Completion
       //if ( (m_recBuffer[1] == 0x51 || m_recBuffer[1] == 0x52) && (length == 2)  )
       // { 
       //  int s = int(m_recBuffer[1] - 0x51);
       //  cout<<"Completed " << cmd
       //      <<" at socket "<<s<<endl;
       //  continue;
       //}
 
       //-----------------
       //INQUIRIES
       // - no completion is returned
       // - returned messages not unique
       // - predifined order conserved ?
       //-----------------
       if ( m_recBuffer[1] == 0x50) {
	 if (length == 10) {
	   m_inquiryMutex.lock();
	   m_numPosInquiries--;
	   m_inquiryMutex.unlock();
	   int pan(0), tilt(0);
	   if (m_recBuffer[2] ==0x00) {
	     pan = int(m_recBuffer[3]*256 + m_recBuffer[4]*16 + m_recBuffer[5]);
	   } else {
	     pan = int( ( (0x0F - m_recBuffer[3])*256
			  + (0x0F - m_recBuffer[4])*16 
			  + 0x0F - m_recBuffer[5] ) * (-1.) );
	   }
	   if (m_recBuffer[6] == 0x00) {
	     tilt = int(m_recBuffer[7]*256+m_recBuffer[8]*16+m_recBuffer[9]);
	   } else {
	     tilt = int( ( (0x0F - m_recBuffer[7])*256
			   + (0x0F - m_recBuffer[8])*16
			   + 0x0F - m_recBuffer[9] ) * (-1.) );
	     setPanTilt(pan, tilt);
	   }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
	       cout<<" rec pan: "<<m_panPos<<" tilt: "<<m_tiltPos<<endl;
#endif
	       continue;
	 } else if (length == 15) {     //lens system block inquiry 1
	   /*cout << "\t lens block inquiry ";
	     for (int i=2; i<15; i++)
	     cout << hex << int(m_recBuffer[i]) << " ";
	     cout << endl;*/
	   m_inquiryMutex.lock();
	   m_numBlockInquiries--;
	   m_inquiryMutex.unlock();
	   
	   int zoom( m_recBuffer[2]*4096 + m_recBuffer[3]*256 
		     + m_recBuffer[4]*16 + m_recBuffer[5]); //buffer 2-5
	   int focusNearLimit (m_recBuffer[6]*4096 + m_recBuffer[7]*256);
	   int focus (m_recBuffer[8]*4096 + m_recBuffer[9]*256
		      + m_recBuffer[10]*16 + m_recBuffer[11]); //buffer 8-11
	   setLensSetting(zoom, focus, focusNearLimit);
	   continue;
	 }
       }
     } //recAddress
    } //while messages
    //cout << "..end..processing messages..." << flush << endl;
    return;
  }

  /*-----------------------------------------
   * the spy class
   *----------------------------------------*/
  sonyEviD100P::externPoll::externPoll() :
    m_poller(0) {
  }

  sonyEviD100P::externPoll::externPoll(const externPoll& other) {
    copy(other);
  }

  sonyEviD100P::externPoll::~externPoll() {
    if (m_poller->signOff() == 0) {
      //passiveWait(400000);
      if (m_poller != 0) { delete m_poller; }
    }
  }
    
  sonyEviD100P::externPoll& sonyEviD100P::externPoll::copy(const externPoll& other) {
    m_poller = other.m_poller;
    m_poller->enroll();
    return *this;
  }
  
  sonyEviD100P::externPoll& sonyEviD100P::externPoll::operator=(const externPoll& other) {
    return copy(other);
  }
 
  const char* sonyEviD100P::externPoll::getTypeName () const {
    return "externPoll";
  }

  void sonyEviD100P::externPoll::attachPoller(poll *poller) {
    m_poller = poller;
    if (m_poller != 0) { m_poller->enroll(); }
  }
  
  bool sonyEviD100P::externPoll::processMessages() const{
    if (m_poller == 0) {
      return false;
    }
    m_poller->processMessages();
    return true;
  }

  bool sonyEviD100P::externPoll::inquiry() const{
    if (m_poller == 0) {
      return false;
    }
    return m_poller->inquiry();
  }

  // --------------------------------------------------
  // sonyEviD100P::parameters
  // --------------------------------------------------
 
  // default constructor
  sonyEviD100P::parameters::parameters()
    : camera::parameters(),
      panTiltUnit::parameters(0.f,
							  100.f*constants<float>::Pi()/180.f,
							  0.f,
							  25.f*constants<float>::Pi()/180.f),
      poller(ManualPoll), 
      pollLens(false),
      pollPanTilt(true),
      waitForInit(200),
      panSpeed(300.f),
      tiltSpeed(125.f),
      whiteBalanceMode(Auto),
      autoIris(true),
      iris(F11),
      enableBrightness(false),
      brightness(6),  
      exposureCompensation(0),
      enableBackLightCompensation(false),
      aperture(5), 
      pictureEffect(Color),
      digitalEffect(None),
      highAutoFocusSensitivity(true),
      focusNearLimit(0.f),
      focusSpeed(7),
      digitalZoom(false),
      zoomSpeed(7) {
  }

  // copy constructor
  sonyEviD100P::parameters::parameters(const parameters& other)
    : camera::parameters(other),panTiltUnit::parameters(other) {
    copy(other);
  }

  // destructor
  sonyEviD100P::parameters::~parameters() {
  }

  // get type name
  const char* sonyEviD100P::parameters::getTypeName() const {
    return "sonyEviD100P::parameters";
  }

  // copy member

  sonyEviD100P::parameters&
    sonyEviD100P::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    camera::parameters::copy(other);
    panTiltUnit::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    camera::parameters& (camera::parameters::* p_copy)
      (const camera::parameters&) =
      camera::parameters::copy;
    (this->*p_copy)(other);

    panTiltUnit::parameters& (panTiltUnit::parameters::* p_copy2)
      (const panTiltUnit::parameters&) =
      panTiltUnit::parameters::copy;

    (this->*p_copy2)(other);
# endif

    poller = other.poller;
    pollLens = other.pollLens;
    pollPanTilt = other.pollPanTilt;
    waitForInit = other.waitForInit; 
    tiltSpeed = other.tiltSpeed;
    panSpeed = other.panSpeed;
    whiteBalanceMode = other.whiteBalanceMode;
    autoIris = other.autoIris;
    iris = other.iris;
    enableBrightness = other.enableBrightness;
    brightness = other.brightness;
    exposureCompensation = other.exposureCompensation;
    enableBackLightCompensation = other.enableBackLightCompensation;
    aperture = other.aperture;
    pictureEffect = other.pictureEffect;
    digitalEffect = other.digitalEffect;
    highAutoFocusSensitivity = other.highAutoFocusSensitivity;
    focusNearLimit = other.focusNearLimit;
    focusSpeed = other.focusSpeed;
    digitalZoom = other.digitalZoom;
    zoomSpeed = other.zoomSpeed;

    return *this;
  }

  // alias for copy member
  sonyEviD100P::parameters&
    sonyEviD100P::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* sonyEviD100P::parameters::clone() const {
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
  bool sonyEviD100P::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool sonyEviD100P::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"poller", "ManalPoll");

      lti::write(handler,"pollLens",pollLens);
      lti::write(handler,"pollPanTilt",pollPanTilt);
      lti::write(handler,"waitForInit",waitForInit);
      lti::write(handler,"panSpeed",panSpeed);
      lti::write(handler,"minPanSpeed",minPanSpeed);
      lti::write(handler,"maxPanSpeed",maxPanSpeed);
      lti::write(handler,"tiltSpeed",tiltSpeed);
      lti::write(handler,"minTiltSpeed",minTiltSpeed);
      lti::write(handler,"maxTiltSpeed",maxTiltSpeed);
      
      switch ( whiteBalanceMode ) {
      case ExtendedAuto:
	lti::write(handler,"whiteBalanceMode","ExtendedAuto"); break;
      case Indoor:
	lti::write(handler,"whiteBalanceMode","Indoor"); break;
      case Outdoor:
	lti::write(handler,"whiteBalanceMode","Outdoor"); break;	
      case OnePush:
	lti::write(handler,"whiteBalanceMode","OnePush"); break;	
      case Manual:
	lti::write(handler,"whiteBalanceMode","Manual"); break;
      default:
	lti::write(handler,"whiteBalanceMode","Auto"); break;
      }

      lti::write(handler,"autoIris",autoIris);
   
      switch ( iris ) {
      case Close:
	lti::write(handler,"iris","Close"); break;
      case F28:
	lti::write(handler,"iris","F28"); break;
      case F22:
	lti::write(handler,"iris","F22"); break;
      case F19:
	lti::write(handler,"iris","F19"); break;
      case F16:
	lti::write(handler,"iris","F16"); break;
      case F14:
	lti::write(handler,"iris","F14"); break;
      case F11:
	lti::write(handler,"iris","F11"); break;
      case F9_6:
	lti::write(handler,"iris"," F9_6"); break;
      case F8:
	lti::write(handler,"iris","F8"); break;
      case F6_8:
	lti::write(handler,"iris","F6_8"); break;
      case F5_6:
	lti::write(handler,"iris","F5_6"); break;
      case F4_8:
	lti::write(handler,"iris","F4_8"); break;
      case F4:
	lti::write(handler,"iris","F4"); break;
      case F3_4:
	lti::write(handler,"iris","F3_4"); break;
      case F2_8:
	lti::write(handler,"iris","F2_8"); break;
      case F2_4:
	lti::write(handler,"iris","F2_4"); break;
      case  F2:
	lti::write(handler,"iris"," F2"); break;
      default:
	lti::write(handler,"iris","F1_8"); break;
      }

      lti::write(handler,"enableBrightness",enableBrightness);
      lti::write(handler,"brightness",brightness);
      lti::write(handler,"maxBrightness",maxBrightness);
      lti::write(handler,"exposureCompensation",exposureCompensation);
      lti::write(handler,"minExposureCompensation",minExposureCompensation);
      lti::write(handler,"maxExposureCompensation",maxExposureCompensation);
      lti::write(handler,"enableBackLightCompensation",enableBackLightCompensation);
      lti::write(handler,"aperture",aperture);
      lti::write(handler,"maxAperture",maxAperture);

      switch ( pictureEffect ) {
      case Pastel:
	lti::write(handler,"pictureEffect","Pastel"); break;
      case NegArt:
	lti::write(handler,"pictureEffect","NegArt"); break;
      case Sepia:
	lti::write(handler,"pictureEffect","Sepia"); break;
      case Monochrome:
	lti::write(handler,"pictureEffect","Monochrome"); break;
      case Solarize:
	lti::write(handler,"pictureEffect","Solarize"); break;
      case Mosaic:
	lti::write(handler,"pictureEffect","Mosaic"); break;
      case Slim:
	lti::write(handler,"pictureEffect","Slim"); break;
      case Stretch:
	lti::write(handler,"pictureEffect","Stretch"); break;
      default:
	lti::write(handler,"pictureEffect"," Color"); break;
      }
 
      switch ( digitalEffect ) {
      case Still:
	lti::write(handler,"digitalEffect","Still"); break;
      case Flash:
	lti::write(handler,"digitalEffect","Flash"); break;
      case Lumi:
	lti::write(handler,"digitalEffect","Lumi"); break;
      case Trail:
	lti::write(handler,"digitalEffect","Trail"); break;	
      default:
	lti::write(handler,"digitalEffect","None"); break;
      }
      
      lti::write(handler,"maxStillLevel",maxStillLevel);
      lti::write(handler,"maxFlashLevel",maxFlashLevel);
      lti::write(handler,"maxLumiLevel",maxLumiLevel);
      lti::write(handler,"maxTrailLevel",maxTrailLevel);
      lti::write(handler,"highAutoFocusSensitivity",highAutoFocusSensitivity);
      lti::write(handler,"focusNearLimit",focusNearLimit);
      lti::write(handler,"focusSpeed",focusSpeed);
      lti::write(handler,"minFocusSpeed",minFocusSpeed);
      lti::write(handler,"maxFocusSpeed",maxFocusSpeed);
      lti::write(handler,"maxOpticalZoom",maxOpticalZoom);
      lti::write(handler,"digitalZoom",digitalZoom);
      lti::write(handler,"zoomSpeed",zoomSpeed);
      lti::write(handler,"minZoomSpeed",minZoomSpeed);
      lti::write(handler,"maxZoomSpeed",maxZoomSpeed);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && camera::parameters::write(handler,false);
    b = b && panTiltUnit::parameters::write(handler,false);
# else
    bool (camera::parameters::* p_writeMS)(ioHandler&,const bool) const =
      camera::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
    bool (panTiltUnit::parameters::* p_writeMS2)
      (ioHandler&,const bool) const =
      panTiltUnit::parameters::writeMS;
    b = b && (this->*p_writeMS2)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool sonyEviD100P::parameters::write(ioHandler& handler,
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
  bool sonyEviD100P::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool sonyEviD100P::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      float fDummy;
      int iDummy;
 
      lti::read(handler,"poller",str);
      if (str == "ManualPoll")  //will be usefull when there are more pollers
        poller = ManualPoll;
      else poller = ManualPoll;

      lti::read(handler,"pollLens",pollLens);
      lti::read(handler,"pollPanTilt",pollPanTilt);
      lti::read(handler,"waitForInit",waitForInit);
      lti::read(handler,"panSpeed",panSpeed);
      lti::read(handler,"minPanSpeed",fDummy);
      lti::read(handler,"maxPanSpeed",fDummy);
      lti::read(handler,"tiltSpeed",tiltSpeed);
      lti::read(handler,"minTiltSpeed",fDummy);
      lti::read(handler,"maxTiltSpeed",fDummy);
      lti::read(handler,"whiteBalanceMode",str);
      
      if ( str == "ExtendedAuto" ) {
	whiteBalanceMode = ExtendedAuto;
      } else if ( str == "Indoor" ) {
	whiteBalanceMode = Indoor;
      } else if ( str == "Outdoor" ) {
	whiteBalanceMode = Outdoor;
      } else if( str == "OnePush" ) {
	whiteBalanceMode = OnePush;
      } else if ( str == "Manual" ) {
	whiteBalanceMode = Manual;
      } else {
	whiteBalanceMode = Auto;
      }

      lti::read(handler,"autoIris",autoIris);

      lti::read(handler,"iris",str);
      if ( str == "Close" ) {
	iris = Close;
      } else if ( str == " F28" ) {
	iris =  F28;
      } else if ( str == "F22" ) {
	iris = F22;
      } else if ( str == "F19" ) {
	iris = F19;
      } else if ( str == "F16" ) {
	iris = F16;
      } else if ( str == "F14" ) {
	iris = F14;
      } else if ( str == "F11" ) {
	iris = F11;
      } else if ( str == "F9_6" ) {
	iris =  F9_6;
      } else if ( str == "F8" ) {
	iris = F8;
      } else if ( str == "F6_8" ) {
	iris = F6_8;
      } else if ( str == "F5_6" ) {
	iris = F5_6;
      } else if ( str == "F4_8" ) {
	iris = F4_8;
      } else if ( str == "F4" ) {
	iris = F4;
      } else if ( str == "F3_4" ) {
	iris = F3_4;
      } else if ( str == "F2_8" ) {
	iris = F2_8;
      } else if ( str == "F2_4" ) {
	iris = F2_4;
      } else if ( str == "F2" ) {
	iris = F2;
      } else {
	iris = F1_8;
      }
      lti::read(handler,"enableBrightness",enableBrightness);
      lti::read(handler,"brightness",brightness);
      lti::read(handler,"maxBrightness",iDummy);
      lti::read(handler,"exposureCompensation",exposureCompensation);
      lti::read(handler,"minExposureCompensation",iDummy);
      lti::read(handler,"maxExposureCompensation",iDummy);
      lti::read(handler,"enableBackLightCompensation",enableBackLightCompensation);
      lti::read(handler,"aperture",aperture);
      lti::read(handler,"maxAperture",iDummy);

      lti::read(handler,"pictureEffect",str);
      if ( str == "Pastel" ) {
	pictureEffect = Pastel;
      } else if ( str == "NegArt" ) {
	pictureEffect = NegArt;
      } else if ( str == "Sepia" ) {
	pictureEffect = Sepia;
      } else if ( str == "Monochrome" ) {
	pictureEffect = Monochrome;
      } else if ( str == "Solarize" ) {
	pictureEffect = Solarize;
      } else if ( str == "Mosaic" ) {
	pictureEffect = Mosaic;
      } else if ( str == "Slim" ) {
	pictureEffect = Slim;
      } else if ( str == "Stretch" ) {
	pictureEffect = Stretch;
      } else {
	pictureEffect = Color;
      }

      lti::read(handler,"digitalEffect",str);
      if ( str == "Still" ) {
	digitalEffect = Still;
      } else if ( str == "Flash" ) {
	digitalEffect = Flash;
      } else if ( str == "Lumi" ) {
	digitalEffect = Lumi;
      } else if ( str == "Trail" ) {
	digitalEffect = Trail;
      } else {
	digitalEffect = None;
      }

      lti::read(handler,"maxStillLevel",iDummy);
      lti::read(handler,"maxFlashLevel",iDummy);
      lti::read(handler,"maxLumiLevel",iDummy);
      lti::read(handler,"maxTrailLevel",iDummy);
      lti::read(handler,"highAutoFocusSensitivity",highAutoFocusSensitivity);
      lti::read(handler,"focusNearLimit",focusNearLimit);
      lti::read(handler,"focusSpeed",focusSpeed);
      lti::read(handler,"minFocusSpeed",iDummy);
      lti::read(handler,"maxFocusSpeed",iDummy);
      lti::write(handler,"maxOpticalZoom",fDummy);
      lti::write(handler,"digitalZoom",digitalZoom);	
      lti::read(handler,"zoomSpeed",zoomSpeed);
      lti::read(handler,"minZoomSpeed",iDummy);
      lti::read(handler,"maxZoomSpeed",iDummy); 
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && camera::parameters::read(handler,false);
    b = b && panTiltUnit::parameters::read(handler,false);
# else
    //??? what about panTiltUnit
    bool (camera::parameters::* p_readMS)(ioHandler&,const bool) =
      camera::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
    bool (panTiltUnit::parameters::* p_readMS2)(ioHandler&,const bool) =
      panTiltUnit::parameters::readMS;
    b = b && (this->*p_readMS2)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool sonyEviD100P::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // sonyEviD100P
  // --------------------------------------------------

  // default constructor
  sonyEviD100P::sonyEviD100P()
    : camera(),
      panTiltUnit(),
      m_poller(0),
      m_completeInit(false)
  {
    // create an instance of the parameters with the default values
    parameters defaultParameters;

    //create a default parameter instance parameters
    panTiltUnit::setParameters(defaultParameters);

    //initialize the device
    init();

    // set the default parameters, i.e. tell the hardware
    setParameters(defaultParameters);
  }

  // default constructor
  sonyEviD100P::sonyEviD100P(const parameters& par)
    : camera(),
      panTiltUnit(),
      m_poller(0),
      m_completeInit(false)
  {
   //create a default parameter instance parameters
    panTiltUnit::setParameters( parameters() );
    
    //initialize the device
    init();

    // set the given parameters, i.e. tell the hardware
    setParameters(par);
  }


  // copy constructor
  sonyEviD100P::sonyEviD100P(const sonyEviD100P& other)
    : camera(),
      panTiltUnit() {
    copy(other);
  }

  // destructor
  sonyEviD100P::~sonyEviD100P() {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << "...destructing SonyEviD100P..." << endl;
#endif
    //ToDo: Check registered objects on pollThread
    if (m_poller->signOff() == 0) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
      cout << "...destructing Polling..." << endl;
#endif
      ubyte buffer[15] = {0,0x01,0x06,0x02,0x18,0x14,
			  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};
      vector<ubyte> command(15, buffer, true);
      
      if (m_poller != 0) delete m_poller;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << "...destructed." << endl << flush;
#endif
  }

  // returns the name of this type
  const char* sonyEviD100P::getTypeName() const {
    return "sonyEviD100P";
  }

  // copy member
  sonyEviD100P& sonyEviD100P::copy(const sonyEviD100P& other) {
    camera::copy(other);
    panTiltUnit::copy(other);
    m_poller = other.m_poller;
    m_poller->enroll();
    m_externPoller = other.m_externPoller;
    m_completeInit = other.m_completeInit;
    
    return (*this);
  }

  // alias for copy member
  sonyEviD100P&
    sonyEviD100P::operator=(const sonyEviD100P& other) {
    return (copy(other));
  }


  // clone member
  functor* sonyEviD100P::clone() const {
    return new sonyEviD100P(*this);
  }

  // return parameters
  const sonyEviD100P::parameters&
    sonyEviD100P::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  sonyEviD100P::parameters& 
  sonyEviD100P::getWritableParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool sonyEviD100P::setParameters(const functor::parameters& newPar) {

    if (isNull(dynamic_cast<const parameters*>(&newPar))) {
      setStatusString("Incorrect parameters type");
      return false;
    }

    const parameters& par = *dynamic_cast<const parameters*>(&newPar);

    //get the old parameters
    parameters& oldPar = getWritableParameters();
 
    //we do not use functor::setParameters because we only remember those
    //values which are consistent with the new hardware setting

    //adjust member variables
    oldPar.angleFormat = par.angleFormat;
    oldPar.relativeMovement = par.relativeMovement;

    //check wether the device is properly initialized
    if ( !m_completeInit ) {
      oldPar.poller = par.poller;
      oldPar.waitForInit = par.waitForInit;

      if ( !init() ) { return false; }
    }

    if ( oldPar.pan != par.pan || oldPar.tilt != par.tilt ||
	 oldPar.panSpeed != par.panSpeed ||
	 oldPar.tiltSpeed != par.tiltSpeed ) {
      setPanTilt(par.pan,par.tilt,par.panSpeed,par.tiltSpeed);
    }

    if ( oldPar.autoFocus != par.autoFocus ) {
      if ( par.autoFocus ) {
	autoFocusOn();
      } else {
	manualFocusOn();
      }
    }
     
    if ( oldPar.focus != par.focus && !par.autoFocus ) {
      setFocus(par.focus);
    }

    if ( oldPar.digitalZoom != par.digitalZoom) {
      if ( par.digitalZoom ) {
	digitalZoomOn();
      } else {
	digitalZoomOff();
      }
    }

    if ( oldPar.zoom != par.zoom) {
      setZoom(par.zoom);
    }
    if ( oldPar.autoWhiteBalance != par.autoWhiteBalance ) {
      //the following changes the white balance mode
      if ( par.autoWhiteBalance ) {
	autoWhiteBalanceOn();
	//we check the white balance mode now
	if ( ( par.whiteBalanceMode != parameters::Manual 
	       || par.whiteBalanceMode !=  parameters::OnePush )
	     && oldPar.whiteBalanceMode != par.whiteBalanceMode ) {
	  switch (par.whiteBalanceMode) {
	  case  parameters::ExtendedAuto:
	    extendedAutoWhiteBalanceOn();
	    break;
	  case  parameters::Indoor:
	    indoorWhiteBalanceOn();
	    break;
	  case  parameters::Outdoor:
	    outdoorWhiteBalanceOn();
	    break;
	  default:
	    break;
	  }
	}
      }
      else {
	manualWhiteBalanceOn();
	//we check the white balance mode now
	if ( par.whiteBalanceMode ==  parameters::OnePush ) {
	  onePushWhiteBalanceOn();
	}
      }
    }

    if ( par.whiteBalanceMode ==  parameters::Manual ) {
      if ( oldPar.redGain != par.redGain ) {
	setRedGain(par.redGain);
      }
      if ( oldPar.blueGain != par.blueGain ) {
	setBlueGain(par.blueGain);
      }
    }

    if ( oldPar.autoGain != par.autoGain 
	 || oldPar.autoIris != par.autoIris
	 || oldPar.autoShutter != par.autoShutter ) {
      if ( par.autoGain ) {
	if ( par.autoShutter ) {
	  if ( par.autoIris ) {
	    autoExposureOn();
	  } else {
	    irisPriorityOn();
	  } 
	} else {
	  if ( par.autoIris ) {
	    shutterPriorityOn();
	  } else { 
	    onlyAutoGainOn();
	  } 
	}
      } else {
	if ( par.autoShutter ) {
	  if ( par.autoIris ) {
	    gainPriorityOn();
	  } else {
	    onlyAutoShutterOn();
	  } 
	} else {
	  if ( par.autoIris ) {
	    onlyAutoIrisOn();
	  } else {
	    manualExposureOn();
	  }
	}
      }
    }

    if ( !par.autoGain && oldPar.gain != par.gain ) {
	setGain(par.gain);
    }
    if ( !par.autoIris && oldPar.iris != par.iris ) {
      setIris(par.iris);
    }
    if ( !par.autoShutter && oldPar.autoShutter != par.autoShutter ) {
      setShutterSpeed(par.shutterSpeed);
    }
    if ( par.enableBrightness ) {
      if ( oldPar.enableBrightness != par.enableBrightness ) {
	brightnessOn();
      }
      if ( oldPar.brightness != par.brightness ) {
	setBrightness(par.brightness);
      }
    }
      
    if ( par.exposureCompensation != oldPar.exposureCompensation ) {
      setExposureCompensation(par.exposureCompensation);
    }
    if ( par.aperture != oldPar.aperture ) {
      setAperture(par.aperture);
    }
    if ( par.enableBackLightCompensation 
	 != oldPar.enableBackLightCompensation ) {
      if ( par.enableBackLightCompensation ) {
	backLightCompensationOn();
      } else {
	backLightCompensationOff();
      }
    }

    if ( oldPar.pictureEffect != par.pictureEffect ) {
      setPictureEffect(par.pictureEffect);
    }
    if ( oldPar.digitalEffect != par.digitalEffect ) {
      setDigitalEffect(par.digitalEffect);
    }
    if ( par.digitalEffect != parameters::None &&
	 par.digitalEffectLevel != oldPar.digitalEffectLevel ) {
      setDigitalEffectLevel(par.digitalEffectLevel);
    }
    //camera consistency
    //if ( par.autoWhiteBalance &&
    // ( par.whiteBalanceMode == parameters::Manual ||
    //   par.whiteBalanceMode == parameters::OnePush ) )
    // {
    //internalPar.whiteBalanceMode = parameters::Auto;
    // }

    return true;
  }

  bool sonyEviD100P::completeInit() {

    return m_completeInit;
  }

  bool sonyEviD100P::init() {
    const parameters& par=getParameters();

    if (m_poller == 0)  //ToDo: parameters determine which poller
      m_poller = new poll(serial::parameters::Com1, par.waitForInit);

    m_completeInit = false;
    int passiveWaitTime ( 20000);
    
    //========== Pan Tilt stuff ==========
    //No Pan/Tilt limitations
    {
      ubyte buffer[15] =  {0,0x01,0x06,0x07,0x01,0x00,0x07,0x0F,0x0F,
			   0x0F,0x07,0x0F,0x0F,0x0F,0xFF};
      vector<ubyte> command(15, buffer, true);
      if (!m_poller->send(std::string("Pan limitations off"), command)) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	cout << "sonyEviD100P: Could not sent turn off pan limitations" << endl;
#endif
	return false;
      }
      command[5]  = 0x01;
      if (!m_poller->send(std::string("Tilt limitations off"), command)) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	cout << "sonyEviD100P: Could not sent turn off tilt limitations" << endl;
#endif
	return false;
      }
    }
 
    //========== Lens stuff ==========
    //zoom with max. speed to zoom Wide (no zoom)
    {
      passiveWait ( passiveWaitTime );
      ubyte buffer[6] = {0,0x01,0x04,0x07,0x37,0xFF};
      vector<ubyte> command(6, buffer, true);
      if (!m_poller->send(std::string("Zoom wide (variable)"),command)) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	cout << "sonyEviD100P: Could not sent init wide zoom" << endl;
#endif
	return false;
      }
    }

    //max. focus range
    {
      passiveWait ( passiveWaitTime );
      ubyte buffer[9] = {0,0x01,0x04,0x28,0x08,0x04,0x00,0x00,0xFF};
      vector<ubyte> command(9, buffer, true);
      if (!m_poller->send(std::string("NearLimit"),command)) {
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
	cout << "sonyEviD100P: Could not sent init near limit" << endl;
#endif
	return false;
      }
    }

    //Automatic modes
    passiveWait ( passiveWaitTime );
    autoFocusOn();
    passiveWait ( passiveWaitTime );
    autoExposureOn();

    passiveWait ( passiveWaitTime );
    setPictureEffect(parameters::Color);
    
    passiveWait ( passiveWaitTime );
    setDigitalEffect(parameters::None);

    passiveWait ( passiveWaitTime );
    backLightCompensationOff();

    passiveWait ( passiveWaitTime );
    setAperture(par.aperture);

    {
      ubyte buffer[6] =  {0,0x01,0x04,0x3E,02,0xFF};
      vector<ubyte> command(6, buffer, true);
      if ( !m_poller->send("exposureComp on", command) ) {
	return false;
      }
    }
    passiveWait ( par.waitForInit );

    passiveWait ( passiveWaitTime );
    setExposureCompensation(par.exposureCompensation);

    //white balance
    autoWhiteBalanceOn();

    m_completeInit = true;
    return true;
  }

  const sonyEviD100P::externPoll& sonyEviD100P::getPoller() {
    m_externPoller.attachPoller(m_poller);
    return m_externPoller;
  }

  // -------------------------------------------------------------------
  // The pan tilt methods
  // -------------------------------------------------------------------

   //Reset the pan tilt device
  bool sonyEviD100P::reset() {
    ubyte buffer[5] = {0,0x01,0x06,0x05,0xFF};
    vector<ubyte> command(5, buffer, true); 
    return m_poller->send("Reset", command);
  } 

  //Stop the movement.
  bool sonyEviD100P::stopPanTilt() {
    ubyte buffer[9] = {0,0x01,0x06,0x01,0,0,0x03,0x03,0xFF};
    vector<ubyte> command(9, buffer, true);
    return m_poller->send("Stop the movement", command);
  }

  bool sonyEviD100P::setPanTilt(float pan, float tilt,
				float panSpeed, float tiltSpeed) {

    ubyte buffer[15] =  {0,0x01,0x06,0x02,0,0,0,0,0,0,0,0,0,0,0xFF};
    vector<ubyte> command(15, buffer, true);
    initPosCommand(command, pan, tilt, panSpeed, tiltSpeed);

    char values[40];
    sprintf(values, "SetPanTilt %3.3f %3.3f %3.1f %3.1f", 
	    pan, tilt, panSpeed, tiltSpeed);
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " -- sonyEviD100P:: " << dec << values << endl << flush;
    cout << " command " << hex << command << endl << dec << flush;
#endif
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.pan = pan;
    par.tilt = tilt;
    par.panSpeed = panSpeed;
    par.tiltSpeed = tiltSpeed;
    return true;
  }

  //Set the absolute pan and tilt position and  the pan and tilt speeds.
  //If no speed is specified, the speeds are set to their max. values.
  bool sonyEviD100P::setPanTilt(float pan, float tilt) {
    parameters& par = getWritableParameters();

    ubyte buffer[15] =  {0,0x01,0x06,0x02,0,0,0,0,0,0,0,0,0,0,0xFF};
    if ( par.relativeMovement ) buffer[3] = 0x03;
    vector<ubyte> command(15, buffer, true);

    initPosCommand(command, pan, tilt, par.maxPanSpeed, par.maxTiltSpeed);

    char values[40];
    sprintf(values, "SetPanTilt %3.3f %3.3f %3.1f %3.1f", 
	    pan, tilt, par.maxPanSpeed, par.maxTiltSpeed);
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " -- sonyEviD100P:: " << dec << values << endl << flush;
    cout << " command " << hex << command << endl << dec << flush;
#endif
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.pan = pan;
    par.tilt = tilt;
    return true;
  }

  bool sonyEviD100P::setPan(float pan) {
    parameters& par = getWritableParameters();
    if (par.relativeMovement) {
      if (setPanTilt(pan, 0.f) ) {
	par.pan = pan;
	return true;
      }
    }
    else {
      if ( setPanTilt(pan, par.tilt) ) {
	par.pan = pan;
	return true;
      }
    }
    return false;
  }
  
  bool sonyEviD100P::setTilt(float tilt) {
    parameters& par = getWritableParameters();
    if (par.relativeMovement) {
      if ( setPanTilt(0.f, tilt) ) {
	par.tilt = tilt;
	return true;
      }
    }
    else {
      if ( setPanTilt(par.pan, tilt) ) {
	par.tilt = tilt;
	return true;
      }
    }
    return false;
  }

  bool sonyEviD100P::initPosCommand(vector<ubyte>& command, 
				    float pan, float tilt, 
				    float panSpeed, float tiltSpeed) {

    const parameters& par = getParameters();
    float maxPan  ( getMaxPan() );
    float maxTilt ( getMaxTilt() );
    if (par.relativeMovement) {
      command[3] = 0x03;
      //To avoid buffer overflow!!!
      if (pan > 2*maxPan) { pan = 2*maxPan; }
      if (pan < -2*maxPan) { pan = -2*maxPan; }
      if (tilt < -2*maxTilt) { tilt = -2*maxTilt; }
      if (tilt > 2*maxTilt) { tilt = 2*maxTilt; }
    } else {
      if (pan > maxPan) { pan = maxPan; }
      if (pan < -maxPan) { pan = -maxPan; }
      if (tilt < -maxTilt) { tilt = -maxTilt; }
      if (tilt > maxTilt) { tilt = maxTilt; }
    }
    unsigned long panHelp = long(m_panFactor * (pan/maxPan+1.) + .5);
    panHelp += 64096;  //moving all values

    command[6] =  int (panHelp/4096);
    panHelp -= command[6]*4096;
    command[7] =  int (panHelp / 256);
    panHelp -= command[7]*256;
    command[8] =  int (panHelp / 16);
    command[9] =  int (panHelp - command[8]*16);
    if ( command[6] == 0x10 ) { command[6] = 0; }

    unsigned long tiltHelp = long(m_tiltFactor * (tilt/maxTilt+1.) + .5);
    tiltHelp += 65176;

    command[10] = int (tiltHelp / 4096);
    tiltHelp -= command[10]*4096;
    command[11] = int (tiltHelp / 256);
    tiltHelp -= command[11]*256;
    command[12] = int (tiltHelp / 16);  
    command[13] = int (tiltHelp - command[12]*16);
    if ( command[10] == 0x10 ) { command[10] = 0; }

    if (panSpeed < 0) { 
      panSpeed = 0;
    } else if( panSpeed >= 300) {
      command[4]=0x18;
    } else if( panSpeed >= 240) {
      command[4]=0x17;
    } else if( panSpeed >= 190) {
      command[4]=0x16;
    } else if( panSpeed >= 155) { 
      command[4]=0x15;
    }
    else if( panSpeed >= 125) { 
      command[4]=0x14;
    } else if( panSpeed >= 100) {
      command[4]=0x13;
    } else if( panSpeed >= 81) { 
      command[4]=0x12;
    } else if( panSpeed >= 65) { 
       command[4]=0x11;
    } else if( panSpeed >= 52) {
      command[4]=0x10;
    } else if( panSpeed >= 42) {
      command[4]=0x0F;
    } else if( panSpeed >= 34) {
      command[4]=0x0E;
    } else if( panSpeed >= 27) {
      command[4]=0x0D;
    } else if( panSpeed >= 22) {
      command[4]=0x0C;
    } else if( panSpeed >= 18) {
       command[4]=0x0B;
    } else if( panSpeed >= 14) {
      command[4]=0x0A;
    } else if( panSpeed >= 11) {
      command[4]=0x09;
    } else if( panSpeed >= 9.1) {
      command[4]=0x08;
    } else if( panSpeed >= 7.4) {
      command[4]=0x07;
    } else if( panSpeed >= 6.1) {
      command[4]=0x06;
    } else if( panSpeed >= 4.7) {
      command[4]=0x05;
    } else if( panSpeed >= 3.7) {
      command[4]=0x04;
    } else if( panSpeed >= 3.0) {
      command[4]=0x03;
    } else if( panSpeed >= 2.4) {
      command[4]=0x02;
    } else {
      command[4]=0x01; 
    }

    if ( tiltSpeed < 0) {
      tiltSpeed = 0;
    } else if( tiltSpeed >= 125) {
      command[5]=0x14;
    } else if( tiltSpeed >= 100) {
      command[5]=0x13;
    } else if( tiltSpeed >= 81) {
      command[5]=0x12;
    } else if( tiltSpeed >= 65) {
      command[5]=0x11;
    } else if( tiltSpeed >= 52) {
      command[5]=0x10;
    } else if( tiltSpeed >= 42) {
      command[5]=0x0F;
    } else if( tiltSpeed >= 34) {
      command[5]=0x0E;
    } else if( tiltSpeed >= 27) {
      command[5]=0x0D;
    } else if( tiltSpeed >= 22) {
      command[5]=0x0C;
    } else if( tiltSpeed >= 18) {
      command[5]=0x0B;
    } else if( tiltSpeed >= 14) {
       command[5]=0x0A;
    } else if( tiltSpeed >= 11) {
      command[5]=0x09;
    } else if( tiltSpeed >= 9.1) {
      command[5]=0x08;
    } else if( tiltSpeed >= 7.4) {
      command[5]=0x07;
    } else if( tiltSpeed >= 6.1) {
      command[5]=0x06;
    } else if( tiltSpeed >= 4.7) {
      command[5]=0x05;
    } else if( tiltSpeed >= 3.7) {
      command[5]=0x04;
    } else if( tiltSpeed >= 3.0) {
      command[5]=0x03;
    } else if( tiltSpeed >= 2.4) {
      command[5]=0x02;
    } else {
      command[5]=0x01; 
    }

    return true;
  }
  
  //Get the current pan and tilt position.
  bool sonyEviD100P::getPanTilt(float& pan, float& tilt) const {

    m_poller->getPanTilt(pan, tilt);
    pan *= getMaxPan() / m_panFactor;
    tilt *= getMaxTilt() / m_tiltFactor;
    return true;
  }

  float sonyEviD100P::getPan() const {

    return float(m_poller->getPan()) / m_panFactor;
  }

  float sonyEviD100P::getTilt() const {

    return float(m_poller->getTilt()) / m_tiltFactor;
  }

  //--------
  // camera
  //--------
  bool sonyEviD100P::autoExposureOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x39,0,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("autoExposureOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters(); 
    par.autoShutter = true;
    par.autoIris = true;
    par.autoGain = true; 
    return true;
  }

  bool sonyEviD100P::manualExposureOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("manualExposureOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoShutter = false;
    par.autoIris = false; 
    par.autoGain = false;
    return true;
  }

  bool sonyEviD100P::shutterPriorityOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x0A,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("shutterPrioriryOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters(); 
    par.autoShutter = false;
    par.autoIris = true;
    par.autoGain = true; 
    return true;
  }

  bool sonyEviD100P::irisPriorityOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x0B,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("irisPriorityOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters(); 
    par.autoShutter = true;
    par.autoIris = false;
    par.autoGain = true; 
    return true;
  }

  bool sonyEviD100P::gainPriorityOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x0C,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("gainPrioriryOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters(); 
    par.autoShutter = true;
    par.autoIris = true;
    par.autoGain = false; 
    return true;
  }

  bool sonyEviD100P::onlyAutoShutterOn() {

    //only automatic shutter
    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x1A,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("onlyAutoShutterOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoShutter = true; 
    return true;
  }

  bool sonyEviD100P::onlyAutoGainOn() {

    //only automatic gain
    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x1C,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("onlyAutoGainOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoGain = true; 
    return true;
  }

  bool sonyEviD100P::onlyAutoIrisOn() {

    //only automatic iris
    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x1B,0xFF};
    vector<ubyte> command(6, buffer, true);
    if (!m_poller->send("onlyAutoIrisOn", command)) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoIris = true; 
    return true;
  }

  //----------
  // WB + gain
  //----------
  bool sonyEviD100P::triggerWhiteBalance() {

    parameters& par = getWritableParameters();
    if ( par.whiteBalanceMode != parameters::OnePush &&
	 !onePushWhiteBalanceOn() ) {
      return false;
    }
    ubyte buffer[6] =  {0,0x01,0x04,0x10,0x05,0xFF};
    vector<ubyte> command(6, buffer, true);
    return m_poller->send("triggerWB", command);
  }
  
  bool sonyEviD100P::onePushWhiteBalanceOn() {
    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("onePushWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = false;
    par.whiteBalanceMode = parameters::OnePush;
    return true;
  }
  

  bool sonyEviD100P::extendedAutoWhiteBalanceOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x04,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("extendedAutoWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = true;
    par.whiteBalanceMode = parameters::ExtendedAuto;
    return true;
  }
  
  
  bool sonyEviD100P::outdoorWhiteBalanceOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("outdoorWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = true;
    par.whiteBalanceMode = parameters::Outdoor;
    return true;
  }
  
  bool sonyEviD100P::indoorWhiteBalanceOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x01,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("indoorWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = true;
    par.whiteBalanceMode = parameters::Indoor;
    return true;
  }
  
  bool sonyEviD100P::autoWhiteBalanceOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x00,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("autoWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = true;
    par.whiteBalanceMode = parameters::Auto;
    return true;
  }

  bool sonyEviD100P::manualWhiteBalanceOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x35,0x05,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("manualWBOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoWhiteBalance = false;
    par.whiteBalanceMode = parameters::Manual;   
    return true;
  }
  
  bool sonyEviD100P::setRedGain(float value) {

    //check manual wB
    parameters& par = getWritableParameters();
    if ( par.whiteBalanceMode != parameters::Manual ) {
      if ( !manualWhiteBalanceOn() ) {
	return false;
      }
      passiveWait ( par.waitForInit );
    }
    ubyte hexValue1(0);
    ubyte hexValue2(0);
    if ( value >= par.minRBGain ) {
      if ( value >= par.maxRBGain ) {
	hexValue1 = 0x0f;
	hexValue2 = 0x0f;
	value = par.maxRBGain;
      } else {
	const ubyte tmp( iround ( value + par.minRBGain ) );
	value = float(tmp);
	hexValue1 = tmp / 16;
	hexValue2 = tmp - hexValue1 * 16;
      }
    } else {
      value = par.minRBGain;
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x43,0,0,hexValue1,hexValue2,0xFF};
    vector<ubyte> command(9, buffer, true);
    char values[30];
    sprintf(values, "setRGain %3.3f", value);
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.redGain = value;
    return true;
  }
  
  bool sonyEviD100P::setBlueGain(float value) {

    //check manual wB
    parameters& par = getWritableParameters();
    if ( par.whiteBalanceMode != parameters::Manual ) {
      if ( !manualWhiteBalanceOn() ) {
	return false;
      }
      passiveWait ( par.waitForInit );
    }

    ubyte hexValue1(0);
    ubyte hexValue2(0);
    if ( value >= par.minRBGain ) {
      if ( value >= par.maxRBGain ) {
	hexValue1 = 0x0f;
	hexValue2 = 0x0f;
	value = par.maxRBGain;
      } else {
	const ubyte tmp( iround ( value + par.minRBGain ) );
	value = float(tmp);
	hexValue1 = tmp / 16;
	hexValue2 = tmp - hexValue1 * 16;
      }
    } else {
      value = par.minRBGain;
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x44,0,0,hexValue1,hexValue2,0xFF};
    vector<ubyte> command(9, buffer, true);
    char values[30];
    sprintf(values, "setBGain %3.3f", value);
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.blueGain = value;
    return true;
  }

  bool sonyEviD100P::autoGainOn() {

    parameters& par = getWritableParameters();
    if ( par.autoShutter )
      {
	if ( par.autoIris ) {
	  return autoExposureOn();
	} else {
	  return irisPriorityOn();
	}
      }
    else
      {
	if ( par.autoIris ) {
	  return shutterPriorityOn();
	} else { 
	  return onlyAutoGainOn();
	}
      }

    return true;
  }

  bool sonyEviD100P::setGain(float value) {

    parameters& par = getWritableParameters();
    if ( par.autoGain ) {
      //disable the automatic gain
      if ( par.autoShutter ) {
	if ( par.autoIris ) {
	  if ( !gainPriorityOn() ) {
	    return false;
	  }
	} else {
	  if ( !onlyAutoGainOn() ) {
	    return false;
	  }
	}
      } else {
	if ( par.autoIris ) {
	  if ( !onlyAutoIrisOn() ) {
	    return false;
	  }
	} else {
	  if ( !manualExposureOn() ) {
	    return false;
	  }
	}
      }
      passiveWait(par.waitForInit);
    }
   
    ubyte hexValue(0);
    if ( value >= par.minGain ) {
      if ( value >= par.maxGain ) {
	hexValue = 0x07;
	value = par.maxGain;
      }
      else {
	hexValue = int ( value - par.minGain + 1.5 ); //round by half step
	hexValue /= 3;
	value = float(hexValue * 3);
      }
    } else {
      value = par.minGain;
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x4C,0,0,0,hexValue,0xFF};
    vector<ubyte> command(9, buffer, true);
    char values[30];
    sprintf(values, "setGain %3.3f", value);
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.gain = value;
    return true;
  }
  
  //---------------
  // shutter speed
  //---------------
  bool sonyEviD100P::autoShutterSpeedOn() {

    parameters& par = getWritableParameters();
    if ( par.autoGain ) {
      if ( par.autoIris ) { 
	return autoExposureOn();
      } else {
	return irisPriorityOn();
      }
    } else {
      if ( par.autoIris ) {
	  return gainPriorityOn();
      } else {
	return onlyAutoShutterOn();
      }
    }

    return true;  
  }
  
  bool sonyEviD100P::setShutterSpeed(float value) {

    parameters& par = getWritableParameters();
    if ( par.autoShutter ) {
      //disable the automatic shutter
      if ( par.autoGain ) {
	if ( par.autoIris ) {
	  if ( !shutterPriorityOn() ){
	    return false;
	  }
	} else {
	  if ( !onlyAutoIrisOn() ) {
	    return false;
	  }
	}
      } else {
	if ( par.autoIris ) {
	  if ( !onlyAutoIrisOn() ) {
	    return false;
	  }
	} else {
	  if ( !manualExposureOn() ) {
	    return false;
	  }
	}
      }
      passiveWait(par.waitForInit);
    }

    //send the shutter value - 20 steps till 0x13
    ubyte hexValue1 (0);
    ubyte hexValue2 (0);
    const int revValue = iround( 1.f/value );
    if ( revValue >= 10000 ) {
      hexValue1 = 0x01;
      hexValue2 = 0x03;
      value = 0.0001f;
    } else if ( revValue >= 6000 ) {
      hexValue1 = 0x01;
      hexValue2 = 0x02;
      value = 1.f/6000.f;
    } else if ( revValue >= 3500 ) {
      hexValue1 = 0x01;
      hexValue2 = 0x01;
      value = 1.f/3500.f;
    } else if ( revValue >= 2500 ) {
      hexValue2 = 0x01;
      value = 1.f/2500.f;
    } else if ( revValue >= 1750 ) {
      hexValue1 = 0x0F;
      value = 1.f/1750.f;
    } else if ( revValue >= 1250 ) {
      hexValue1 = 0x0E;
      value = 1.f/1250.f;
    } else if ( revValue >= 1000 ) {
      hexValue1 = 0x0D;
      value = .001f;
    } else if ( revValue >= 600 ) {
      hexValue1 = 0x0C;
      value = 1.f/600.f;
    } else if ( revValue >= 425 ) {
      hexValue1 = 0x0B;
      value = 1.f/425.f;
    } else if ( revValue >= 300 ) {
      hexValue1 = 0x0A;
      value = 1.f/300.f;
    } else if ( revValue >= 215 ) {
      hexValue1 = 0x09;
      value = 1.f/215.f;
    } else if ( revValue >= 150 ) {
      hexValue1 = 0x08;
      value = 1.f/150.f;
    } else if ( revValue >= 120 ) {
      hexValue1 = 0x07;
      value = 1.f/120.f;
    } else if ( revValue >= 100 ) {
      hexValue1 = 0x06;
      value = .01f;
    } else if ( revValue >= 75 ) {
      hexValue1 = 0x05;
      value = 1.f/75.f;
    } else if ( revValue >= 50 ) {
      hexValue1 = 0x04;
      value = .02f;
    } else if ( revValue >= 25 ) {
      hexValue1 = 0x03;
      value = .04f;
    } else if ( revValue >= 12 ) {
      hexValue1 = 0x02;
      value = 1.f/12.f;
    } else if ( revValue >= 6 ) {
      hexValue1 = 0x01;
      value = 1.f/6.f;
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x4A,0,0,hexValue2,hexValue1,0xFF};
    vector<ubyte> command(9, buffer, true);
    char values[30];
    sprintf(values, "setShutter %1.8f", value);
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.shutterSpeed = value;
    return true; 
  }
  
  //-------
  // iris
  //-------
  bool sonyEviD100P::autoIrisOn()
  {
    parameters& par = getWritableParameters();
    if ( par.autoGain ) {
      if ( par.autoShutter ) {
	  return autoExposureOn();
      } else {
	return gainPriorityOn();
      }
    } else {
      if ( par.autoShutter ) {
	return shutterPriorityOn();
      } else {
	return onlyAutoIrisOn();
      }
    }

    return true;   
  }
  
  bool sonyEviD100P::setIris(parameters::eIrisType value) {

    parameters& par = getWritableParameters();
    if ( par.autoIris ) {
      //disable the automatic iris
      if ( par.autoGain ) {
	if ( par.autoShutter ) {
	  if ( !irisPriorityOn() ) { 
	    return false; 
	  }
	} else {
	  if ( !onlyAutoShutterOn() ) { 
	    return false; 
	  }
	}
      } else {
	if ( par.autoShutter ) {
	  if ( !onlyAutoShutterOn() ) {
	    return false; 
	  }
	} else {
	  if ( !manualExposureOn() ) {
	    return false;
	  }
	}
      }
      passiveWait(par.waitForInit);
    }

    //send the iris value
    ubyte hexValue1 = value / 16;
    ubyte hexValue2 = value - hexValue1 * 16;
    cout << " h1 " << int(hexValue1) << " " << int (hexValue2) << endl;
    ubyte buffer[9] =  {0,0x01,0x04,0x4B,0,0,hexValue1,hexValue2,0xFF};
    vector<ubyte> command(9, buffer, true);
    char values[20];
    sprintf(values, "setIris %i", value);
    if (!m_poller->send(std::string(values), command)) {
      return false;
    }
    par.iris = value;
    return true;    
  }
  
  //-----------------
  // brightness & co
  //-----------------
 
  bool sonyEviD100P::brightnessOn() {

    parameters& par = getWritableParameters();
    if ( !par.autoGain || !par.autoIris ) {
      return false;
    }
    ubyte buffer[6] =  {0,0x01,0x04,0x39,0x0D,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("brightnessOn", command) ) {
      return false;
    }
    par.enableBrightness = true;
    return true;
  } 
  
  bool sonyEviD100P::setBrightness(int value) {

    parameters& par = getWritableParameters();
    if ( !par.enableBrightness ) {
      if ( !brightnessOn() ) {
	return false;
      }
      passiveWait(par.waitForInit);
    }
    ubyte hexValue1 (0);
    ubyte hexValue2 (0);
    if ( value < 0 ) {
      value = 0;
    } else {
      if ( value >= par.maxBrightness ) {
	value = par.maxBrightness;
	hexValue1 = 0x01;
	hexValue2 = 0x07;
      } else {
	hexValue1 = value / 16;
	hexValue2 = value - hexValue1 * 16;
      }
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x4D,0,0,hexValue1,hexValue2,0xFF};
    vector<ubyte> command(9, buffer, true);
    char valueStr[20];
    sprintf(valueStr, "setBrightness %i", value);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.brightness = value;
    return true;
  }
  
  bool sonyEviD100P::setExposureCompensation(int value) {

    parameters& par = getWritableParameters();
    ubyte hexValue ( 0 );
    if ( value < par.minExposureCompensation ) {
      value = 0;
    } else {
      if ( value >= par.maxExposureCompensation ) {
	value = par.maxExposureCompensation;
	hexValue = 0x0E;
      } else {
	hexValue = value - par.minExposureCompensation;
      }
    }
 
    ubyte buffer[9] =  {0,0x01,0x04,0x4E,0,0,0,hexValue,0xFF};
    vector<ubyte> command(9, buffer, true);
    char valueStr[20];
    sprintf(valueStr, "exposureComp %i", value);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.exposureCompensation = value;
    return true;
  }
  
  //-----------------
  // special effects
  //-----------------
  
  bool sonyEviD100P::backLightCompensationOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x33,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("backLightCompensationOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.enableBackLightCompensation = true;
    return true;
  }
  
  bool sonyEviD100P::backLightCompensationOff() { 

    ubyte buffer[6] =  {0,0x01,0x04,0x33,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("backLightCompensationOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.enableBackLightCompensation = false;
    return true;
  }

  bool sonyEviD100P::setAperture(int value) {

    parameters& par = getWritableParameters();
    ubyte hexValue ( 0 );
    if ( value < 0 ) {
      value = 0;
    } else {
      if ( value > par.maxAperture )
	value = par.maxAperture;
    }
    ubyte buffer[9] =  {0,0x01,0x04,0x42,0,0,0,hexValue,0xFF};
    vector<ubyte> command(9, buffer, true);
    char valueStr[20];
    sprintf(valueStr, "setAperture %i", value);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.aperture = value;
    return true; 
  }
  
  bool sonyEviD100P::setPictureEffect(parameters::ePictureEffectType effect) {

    ubyte hexValue ( 0 );
    switch ( effect ) {
    case parameters::Pastel:
	hexValue = 0x01; break;
    case parameters::NegArt:
      hexValue = 0x02; break;
    case parameters::Sepia:
      hexValue = 0x03; break;
    case parameters::Monochrome:
      hexValue = 0x04; break;
    case parameters::Solarize:
      hexValue = 0x05; break;
    case parameters::Mosaic:
      hexValue = 0x06; break;
    case parameters::Slim:
      hexValue = 0x07; break;
    case parameters::Stretch:
      hexValue = 0x08; break;
    default:
      break;
    }
    ubyte buffer[6] =  {0,0x01,0x04,0x63,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[25];
    sprintf(valueStr, "pictureEffect %i", effect);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.pictureEffect = effect;
    return true; 
  }
  
  bool sonyEviD100P::setDigitalEffect(parameters::eDigitalEffectType effect) {

    ubyte hexValue ( 0 );
    switch ( effect ) {
      case parameters::Still:
	hexValue = 0x01; break;
      case parameters::Flash:
	hexValue = 0x02; break;
      case parameters::Lumi:
	hexValue = 0x03; break;
      case parameters::Trail:
	hexValue = 0x04; break;	
      default:
	break;
      }
    ubyte buffer[6] =  {0,0x01,0x04,0x64,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[25];
    sprintf(valueStr, "digitalEffect %i", effect);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.digitalEffect = effect;
    return true; 
  }
  
  bool sonyEviD100P::setDigitalEffectLevel(int value) {

    parameters& par = getWritableParameters();
    if ( value < 0 ) {
      value = 0;

    } else if ( par.digitalEffect == parameters::Flash ||
		par.digitalEffect == parameters::Trail ) {
      if ( value > par.maxFlashLevel ) {
	value = par.maxFlashLevel;
      }

    } else if ( par.digitalEffect == parameters::Lumi ||
		par.digitalEffect == parameters::Still ) {
      if ( value > par.maxLumiLevel ) {
	value = par.maxLumiLevel;
      }

    } else {
      return false;
    }
    
    ubyte buffer[6] =  {0,0x01,0x04,0x65,value,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[30];
    sprintf(valueStr, "digitalEffectLevel %i", value);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.digitalEffectLevel = value;
    return true;
  } 
  
  //-------
  // focus
  //-------
  bool sonyEviD100P::highAutoFocusSensibilityOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x58,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("highAutoFocusSensOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.highAutoFocusSensitivity = true;
    return true;
  }
  
  bool sonyEviD100P::lowAutoFocusSensibilityOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x58,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("lowAutoFocusSensOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.highAutoFocusSensitivity = false;
    return true;
  }
  
  bool sonyEviD100P::setFocus(float value) {
    parameters& par = getWritableParameters();
    if ( par.autoFocus && !manualFocusOn() ) {
      return false;
    }
    ubyte buffer[9] ={0,0x01,0x04,0x48,0,0,0,0,0xFF};
    int focus ( distance2Focus(value) );
 
    buffer[4] =  int (focus/4096);
    focus -= buffer[4]*4096;
    buffer[5] =  int (focus / 256);
    focus -= buffer[5]*256;
    buffer[6] =  int (focus / 16);
    buffer[7] =  int (focus - buffer[6]*16);
    vector<ubyte> command(9, buffer, true);
    char valueStr[30];
    sprintf(valueStr, "setFocus %2.3f", value);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.focus = value;
    return true;
  }
 
  bool sonyEviD100P::setFarFocus(int speed) {

    parameters& par = getWritableParameters();
    ubyte hexValue (0x20);
    if ( speed <= par.minFocusSpeed ) {
      speed = par.minFocusSpeed;
    } else {
      if ( speed > par.maxFocusSpeed ) speed = par.maxFocusSpeed;
      hexValue += speed;
    }

    ubyte buffer[6] =  {0,0x01,0x04,0x08,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[20];
    sprintf(valueStr, "farFocus %i", speed);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.autoFocus = false;
    par.focus = par.maxFocus;
    par.focusSpeed = speed;
    return true;
  }
 
  bool sonyEviD100P::setNearFocus(int speed) {

    parameters& par = getWritableParameters();
    ubyte hexValue (0x30);
    if ( speed <= par.minFocusSpeed ) {
      speed = par.minFocusSpeed;
    } else {
      if ( speed > par.maxFocusSpeed ) speed = par.maxFocusSpeed;
      hexValue += speed;
    }

    ubyte buffer[6] =  {0,0x01,0x04,0x08,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[20];
    sprintf(valueStr, "nearFocus %i", speed);
    if ( !m_poller->send(valueStr, command) ) { return false; }

    par.autoFocus = false;
    par.focus = par.minFocus;
    par.focusSpeed = speed;
    return true;
  }

  bool sonyEviD100P::stopFocus() {

    ubyte buffer[6] =  {0,0x01,0x04,0x08,0,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("stopFocus", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoFocus = false;
    return true;
  }

  bool sonyEviD100P::setFocusToInfinity() {

    ubyte buffer[6] ={0,0x01,0x04,0x18,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("SetFocus2Infinity", command) ) {
      return false; 
    }
    parameters& par = getWritableParameters();
    par.autoFocus = false;
    par.focus = par.maxFocus;
    return true; 
  }
  
  bool sonyEviD100P::manualFocusOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x38,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("manualFocusOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoFocus = false;
    return true;
  }

  bool sonyEviD100P::autoFocusOn() {

    ubyte buffer[6] =  {0,0x01,0x04,0x38,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("autoFocusOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.autoFocus = true;
    return true;
  }
  
  bool sonyEviD100P::triggerFocus() {

    parameters& par = getWritableParameters();
    if ( par.autoFocus && !manualFocusOn() ) {
      return false;
    }
    ubyte buffer[6] ={0,0x01,0x04,0x18,0x01,0xFF};
    vector<ubyte> command(6, buffer, true);
    return m_poller->send("TriggerFocus", command);
  }
  
  float sonyEviD100P::getFocus() const {

    return (  focus2Distance( m_poller->getFocus() ) );
  }

  //Convert focus from distance in m to internal value
  int sonyEviD100P::distance2Focus (float& distance) const {

    const parameters& par = getParameters();
    int focusLUT[16] = {4096,4449,4461,4650,4668,4851,5058,5406,5430,6212,
			8815,16170,16554,25289,33473,33792};
    float distanceLUT[16] = {par.maxFocus,20.f,18.f,16.f,14.f,12.f,10.f,8.f,6.f,
			     4.f,2.f,1.5f,1.f,0.5f,0.1f,par.minFocus};

    int i(0);
    bool search(1);
    while (i<16 && search) {
      if (distance > distanceLUT[i]) {
        search = 0;
      } else {
        i++;
      }
    }
    i--;   //index points to closest bigger or equal value
    if (i<0) {
      distance = distanceLUT[0];
      return ( focusLUT[0] );
    }
    //cout << " index " << dec << i << " " ;
    if (i==15) {
      distance = static_cast<float>(focusLUT[15]);
      return ( focusLUT[15] );
    }
    
    //linear interpolation - bad approximation: function is not linear
    float weight( (distance-distanceLUT[i])/(distanceLUT[i+1]-distanceLUT[i]) );
    //cout << " w " << dec << weight << " ";
    return ( int((focusLUT[i+1]-focusLUT[i])*weight + focusLUT[i]) );
  }

  //Convert focus from internal value to distance in m
  float sonyEviD100P::focus2Distance (int focus) const {

    const parameters& par = getParameters();
    int focusLUT[16] = {4096,4449,4461,4650,4668,4851,5058,5406,5430,6212,8815,
			16170,16554,25289,33473};
    float distanceLUT[16] = {par.maxFocus,20.f,18.f,16.f,14.f,12.f,10.f,8.f,6.f,
			     4.f,2.f, 1.5f,1.f,0.5f,0.1f,par.minFocus};
    
    //cout << dec << " focusI " << focus << " ";
    int i(0);
    bool search ( true );
    while (i<16 && search) {
      if (focus < focusLUT[i]) {
	search = 0;
      } else {
	i++;
      }
    }
    i--;   //index points to closest smaller or equal value
    if (i<0) { return ( distanceLUT[0] ); }
    //cout << " index " << dec << i << " " ;
    if (i==15) { return ( distanceLUT[15] ); }
    
    //linear interpolation - bad approximation: function is not linear
    float weight( float(focus-focusLUT[i])/float(focusLUT[i+1]-focusLUT[i]) ); //positiv
    //cout << " w " << dec << weight << " ";
    return ( (distanceLUT[i+1]-distanceLUT[i])*weight + distanceLUT[i] );
  }
 
  //---------
  // zoom
  //---------
  bool sonyEviD100P::setZoom(float value) {

    unsigned char buffer[9]={0,0x01,0x04,0x47,0x00,0x00,0x00,0x00,0xFF};
    parameters& par = getWritableParameters();
    if ( value < 0 ) value = 0.f;
    else if ( value > par.maxZoom ) value = par.maxZoom;
    if ( value < 10. ) {  //hex 0..4000
      unsigned int optZoom = int (value * 1638.4);  
      buffer[4] = int (optZoom/4096);
      optZoom -= buffer[4]*4096;
      buffer[5] = int (optZoom / 256);
      optZoom -= buffer[5]*256;
      buffer[6] = int (optZoom / 16);
      buffer[7] = optZoom - buffer[6]*16;
    } else {
      //check whether the digitalZoom is enabled
      if ( !par.digitalZoom ) {  //no digital zoom
	buffer[4] = 0x04;
	value = 10.;
      } else {
	if (value >= 40.) {
	  buffer[4] = 0x07;
	  value = 40.;
	} else {  //hex 4000..7000
	  buffer[4] = int (value/10.);
	  unsigned int optZoom ( int ((value - 10. * buffer[4]) * 409.6) );
	  buffer[5] = int (optZoom / 256);
	  optZoom -= buffer[5]*256;
	  buffer[6] = int (optZoom / 16);
	  buffer[7] = optZoom - buffer[6]*16;
	  buffer[4] += 3;
	}
      }
    }
    /*cout << "pos " << hex << int(sendBuffer[4])
	 << " " << int (sendBuffer[5]) << " "
	 << int(sendBuffer[6])
	 << " " << int(sendBuffer[7]) << endl;*/
    char values[16];
    sprintf(values, "setZoom %3.1f", value);
    vector<ubyte> command(9, buffer, true);
    if ( !m_poller->send(std::string(values), command) ) {
      return false;
    }
    par.zoom = value;
    return true; 
  }

  //Stop the zoom.   
  bool sonyEviD100P::stopZoom() {

    ubyte buffer[6]={0,0x01,0x04,0x07,0x00,0xFF};
    vector<ubyte> command(6, buffer, true);
    return m_poller->send(std::string("StopZoom"), command);
  }

  bool sonyEviD100P::setWideZoom(int speed) {

    parameters& par = getWritableParameters();
    ubyte hexValue (0x30);
    if ( speed <= par.minZoomSpeed ) {
      speed = iround(par.minZoomSpeed);
    } else {
      if ( speed > par.maxZoomSpeed ) {
	speed = iround(par.maxZoomSpeed);
      }
      hexValue += speed;
    }
    ubyte buffer[6]={0,0x01,0x04,0x07,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[20];
    sprintf(valueStr,"WideZoom %i", speed);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.zoom = 0.f;

    return true;
  }

  bool sonyEviD100P::setTeleZoom(int speed) {

    parameters& par = getWritableParameters();
    ubyte hexValue (0x20);
    if ( speed <= par.minZoomSpeed ) {
      speed = iround(par.minZoomSpeed);
    } else {
      if ( speed > par.maxZoomSpeed ) {
	speed = iround(par.maxZoomSpeed);
      }
      hexValue += speed;
    }
    ubyte buffer[6]={0,0x01,0x04,0x07,hexValue,0xFF};
    vector<ubyte> command(6, buffer, true);
    char valueStr[20];
    sprintf(valueStr,"TeleZoom %i", speed);
    if ( !m_poller->send(valueStr, command) ) {
      return false;
    }
    par.zoom = par.maxOpticalZoom;

    return true;
  }

  bool sonyEviD100P::digitalZoomOn() {

    ubyte buffer[6]={0,0x01,0x04,0x06,0x02,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("digitalZoomOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.digitalZoom = true;
    return true;
  }

  bool sonyEviD100P::digitalZoomOff() {

    ubyte buffer[6]={0,0x01,0x04,0x06,0x03,0xFF};
    vector<ubyte> command(6, buffer, true);
    if ( !m_poller->send("digitalZoomOn", command) ) {
      return false;
    }
    parameters& par = getWritableParameters();
    par.digitalZoom = false;
    return true;
  } 

  float sonyEviD100P::getZoom() const {

    float zoom ( float( m_poller->getZoom()) );
    
    if (zoom < 16384) {
	zoom = zoom *10.f / 16384.f;
    } else {
      zoom = zoom * 10.f / 4096.f - 30.f;
    } 
    
    return zoom;
  }
  
  bool sonyEviD100P::getLensSetting(float& focus, float& zoom) const {

    int iFocus;
    m_poller->getLensSetting(iFocus, zoom);
    focus = focus2Distance( iFocus );
    if (zoom < 16384) {
      zoom = zoom * 10.f / 16384.f;
    } else {
      zoom = zoom * 10.f / 4096.f - 30.f;
    } 

    return true;
  }
  
  bool sonyEviD100P::pollLensOn() {
    m_poller->pollLens(true);
    parameters& par = getWritableParameters();
    par.pollLens = true;
    return true; 
  }

  bool sonyEviD100P::pollLensOff() {

    m_poller->pollLens(false);
    parameters& par = getWritableParameters();
    par.pollLens = false;
    return true; 
  }
 
  bool sonyEviD100P::pollPanTiltOn() {

    m_poller->pollPanTilt(true);
    parameters& par = getWritableParameters();
    par.pollPanTilt = true;
    return true; 
  }
 
  bool sonyEviD100P::pollPanTiltOff() {

    m_poller->pollPanTilt(false);
    parameters& par = getWritableParameters();
    par.pollPanTilt = false;
    return true; 
  }

}
