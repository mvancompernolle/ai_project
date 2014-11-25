/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiDirectedPerceptionPTU.cpp
 * authors ....: Christian Hein, Florian Bley
 * organization: LTI, RWTH Aachen
 * creation ...: 20.12.2004
 * revisions ..: $Id: ltiDirectedPerceptionPTU.cpp,v 1.8 2006/09/05 10:35:04 ltilib Exp $
 */

//TODO: include files
#include "ltiDirectedPerceptionPTU.h"
#include "ltiConstants.h"
#include "ltiTimer.h"
#include <iostream>

using namespace std;

namespace lti {
  const float directedPerceptionPTU::parameters::resolutionPerPosition = 185.1428f;
  
  const float directedPerceptionPTU::parameters::maxPanSpeed  = (2800.f*185.1428f/3600.f)*constants<float>::Pi()/  
                                                                 180.f;
  
  const float directedPerceptionPTU::parameters::minPanSpeed  = (35.f*185.1428f/3600.f)*constants<float>::Pi()/  
                                                                 180.f;
  
  const float directedPerceptionPTU::parameters::maxTiltSpeed = (2800.f*185.1428f/3600.f)*constants<float>::Pi()/  
                                                                 180.f;
  
  const float directedPerceptionPTU::parameters::minTiltSpeed = (35.f*185.1428f/3600.f)*constants<float>::Pi()/  
                                                                180.f; 
  
  const float directedPerceptionPTU::parameters::maxPanAcceleration =(300000.f*185.1428f/3600.f)*
                                                                      constants<float>::Pi()/180.f;
                                                                 
  const float directedPerceptionPTU::parameters::minPanAcceleration =(1710.f*185.1428f/3600.f)*
                                                                      constants<float>::Pi()/180.f;
  
  const float directedPerceptionPTU::parameters::maxTiltAcceleration =(300000.f*185.1428f/3600.f)*
                                                                      constants<float>::Pi()/180.f;
  
  const float directedPerceptionPTU::parameters::minTiltAcceleration =(1700.f*185.1428f/3600.f)*
                                                                      constants<float>::Pi()/180.f;
  
  const int directedPerceptionPTU::parameters::ptuResponseTime = 100;
  
  int directedPerceptionPTU::parameters::instructionCounter=0;
                                                   
  // --------------------------------------------------
  // DirectedPerceptionPTU::parameters
  // --------------------------------------------------
                                                
  // default constructor
  directedPerceptionPTU::parameters::parameters()
    : panTiltUnit::parameters( (-3090.f*185.1428f/3600.f)*constants<float>::Pi()/180.f,
							   (+3090.f*185.1428f/3600.f)*constants<float>::Pi()/180.f,
							   (-910.f*185.1428f/3600.f)*constants<float>::Pi()/180.f,
							   (+605.f*185.1428f/3600.f)*constants<float>::Pi()/180.f )
   {
    posMode=absolute;
    execMode=immidiate;
    connectedToPTU=false;
    panSpeed        =(600.f*185.1428f/3600.f)*constants<float>::Pi()/180.f;
    tiltSpeed       =(300.f*185.1428f/3600.f)*constants<float>::Pi()/180.f;
    panAcceleration =(9999.f*185.1428f/3600.f)*constants<float>::Pi()/180.f;
    tiltAcceleration=(9999.f*185.1428f/3600.f)*constants<float>::Pi()/180.f;
    pan=0.f;
    tilt=0.f;
  }

  // copy constructor
  directedPerceptionPTU::parameters::parameters(const parameters& other)
    : panTiltUnit::parameters(other)
  {
	copy(other);
  }

  // destructor
  directedPerceptionPTU::parameters::~parameters() {
  }

  // get type name
  const char* directedPerceptionPTU::parameters::getTypeName() const {
    return "DirectedPerceptionPTU::parameters";
  }

  // copy member

  directedPerceptionPTU::parameters&
    directedPerceptionPTU::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    panTiltUnit::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    panTiltUnit::parameters& (panTiltUnit::parameters::* p_copy)
      (const panTiltUnit::parameters&) =
      panTiltUnit::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    // Add own Parameters
    connectionToPTU    = other.connectionToPTU;
    posMode            = other.posMode;
    execMode           = other.execMode;
    panSpeed           = other.panSpeed;
    tiltSpeed          = other.tiltSpeed;
    panAcceleration    = other.panAcceleration;
    tiltAcceleration   = other.tiltAcceleration; 
    instructionCounter = other.instructionCounter; 
    connectedToPTU     = other.connectedToPTU;
    return *this;
  }

  // alias for copy member
  directedPerceptionPTU::parameters&
    directedPerceptionPTU::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* directedPerceptionPTU::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
   // TODO
# ifndef _LTI_MSC_6
  bool directedPerceptionPTU::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool directedPerceptionPTU::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      connectionToPTU.getParameters().write(handler,complete);
      switch (posMode) {
        case relative:
          lti::write(handler,"posMode","relative");
	        break;
        default:
	       lti::write(handler,"posMode","absolute");
	       break;
      }
      switch (execMode) {
        case relative:
	        lti::write(handler,"execMode","slaved");
	        break;
        default:
	        lti::write(handler,"execMode","immidiate");
	        break;
      }

      lti::write(handler,"maxPanSpeed",maxPanSpeed);
      lti::write(handler,"minPanSpeed",minPanSpeed);
      lti::write(handler,"maxTiltSpeed", maxTiltSpeed);
      lti::write(handler,"minTiltSpeed", minTiltSpeed);
      lti::write(handler,"panSpeed", panSpeed);
      lti::write(handler,"tiltSpeed", tiltSpeed);
      lti::write(handler,"maxPanAcceleration",maxPanAcceleration);
      lti::write(handler,"minPanAcceleration",minPanAcceleration);
      lti::write(handler,"maxTiltAcceleration", maxTiltAcceleration);
      lti::write(handler,"minTiltAcceleration", minTiltAcceleration);
      lti::write(handler,"panAcceleration", panAcceleration);
      lti::write(handler,"tiltAcceleration", tiltAcceleration);
      lti::write(handler,"instructionCounter", instructionCounter);
      lti::write(handler,"resolutionPerPosition", resolutionPerPosition);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && panTiltUnit::parameters::write(handler,false);
# else
    bool (panTiltUnit::parameters::* p_writeMS)(ioHandler&,const bool) const =
      panTiltUnit::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool directedPerceptionPTU::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
   //TODO
# ifndef _LTI_MSC_6
  bool directedPerceptionPTU::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool directedPerceptionPTU::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      std::string str;
      float dummy;
      
      // read serial parameters and write them to the ioHandler "handler"
      serial::parameters conPar = connectionToPTU.getParameters();
      conPar.read(handler, complete);
      
      lti::read(handler,"posMode",str);
      if (str == "relative")
          posMode = relative;
      else
          posMode = absolute;  //default
      
      lti::read(handler,"execMode",str);
      if (str == "slaved")
          execMode = slaved;
      else
          execMode = immidiate;  //default
      
      lti::read(handler,"maxPanSpeed",dummy);
      lti::read(handler,"minPanSpeed",dummy);
      lti::read(handler,"maxTiltSpeed", dummy);
      lti::read(handler,"minTiltSpeed", dummy);
      lti::read(handler,"panSpeed", panSpeed);
      lti::read(handler,"tiltSpeed", tiltSpeed);
      lti::read(handler,"maxPanAcceleration",dummy);
      lti::read(handler,"minPanAcceleration",dummy);
      lti::read(handler,"maxTiltAcceleration", dummy);
      lti::read(handler,"minTiltAcceleration", dummy);
      lti::read(handler,"panAcceleration", panAcceleration);
      lti::read(handler,"tiltAcceleration", tiltAcceleration);
      lti::read(handler,"instructionCounter", instructionCounter);
      lti::read(handler,"resolutionPerPosition", dummy);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && panTiltUnit::parameters::read(handler,false);
# else
    bool (panTiltUnit::parameters::* p_readMS)(ioHandler&,const bool) =
      panTiltUnit::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool directedPerceptionPTU::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // DirectedPerceptionPTU
  // --------------------------------------------------

  // default constructor
  directedPerceptionPTU::directedPerceptionPTU()
    : panTiltUnit(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    
    serial::parameters conParameters; 
                    
    // Use Com1 as default
    conParameters.port = serial::parameters::Com1;
    
    /** 
     * Set Default Parameters for RS-232
     *  9600 baud, 1 start bit, 8 data bits, 1 stop bit, no parity
     * 
     */ 
    
    conParameters.baudRate       = serial::parameters::Baud9600;
    conParameters.characterSize  = serial::parameters::Cs8; 
    conParameters.parity         = serial::parameters::No;
    conParameters.stopBits       = serial::parameters::One;
    conParameters.receiveTimeout = 0;  // Timeout in milliseconds

    if (!defaultParameters.connectionToPTU.setParameters(conParameters))
    {
       setStatusString("DirectedPerceptionPTU::openConnectionToPTU Could not set serial port parameter!");
    } 
    else {
      cout << endl << "Setting Serial Communication to:  Com1, 9600 baud, 1 start bit, 8 data bits, 1 stop bit, no parity, Timeout: ";
      cout << conParameters.receiveTimeout << endl <<endl;
    }
    setParameters(defaultParameters);
  }

  // default constructor
  directedPerceptionPTU::directedPerceptionPTU(const parameters& par)
    : panTiltUnit() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  directedPerceptionPTU::directedPerceptionPTU(const directedPerceptionPTU& other)
    : panTiltUnit() {
    copy(other);
  }

  // destructor
  directedPerceptionPTU::~directedPerceptionPTU() {
  }

  // returns the name of this type
  const char* directedPerceptionPTU::getTypeName() const {
    return "DirectedPerceptionPTU";
  }

  // copy member
  directedPerceptionPTU&
    directedPerceptionPTU::copy(const directedPerceptionPTU& other) {
      panTiltUnit::copy(other);

    return (*this);
  }

  // alias for copy member
  directedPerceptionPTU&
    directedPerceptionPTU::operator=(const directedPerceptionPTU& other) {
    return (copy(other));
  }


  // clone member
  functor* directedPerceptionPTU::clone() const {
    return new directedPerceptionPTU(*this);
  }

  // return parameters
  const directedPerceptionPTU::parameters&
    directedPerceptionPTU::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }
  
  // return parameters: this time as not constant
  directedPerceptionPTU::parameters& 
  directedPerceptionPTU::getWritableParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }
 
  // Reset PTU
  bool directedPerceptionPTU::reset(){
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } 
    else if ( par.connectionToPTU.send("R A ") ) {       // Sending reset command
      cout << "Going through Reset Cycle..." << endl;
      par.instructionCounter=par.instructionCounter+6;   // +4 instructions because of the signs "!T" 
                                                         // and "!P" returned by the PTU when hitting the axis end 
      return(true);
    }
    else {
      setStatusString("Error with Serial Port : Command could not be send or timed out!");
      return(false);
    }
  }
  
  // Stop all movement 
  bool directedPerceptionPTU::stopPanTilt(){
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else if ( par.connectionToPTU.send("H ") ) {      // sending halt command
      cout << "All movement haltet!" << endl;
      par.instructionCounter++;
      return(true);
    }
    setStatusString("Error with Serial Port : Command could not be send or timed out!");
    return(false);
  }
  
  // Stop pan movement
  bool directedPerceptionPTU::stopPan() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else if ( par.connectionToPTU.send("HP ") ) {    // Sending halt-pan command
      cout << "Pan movement haltet!" << endl;
      par.instructionCounter++;
      return(true);
    }
    setStatusString("Error with Serial Port : Command could not be send or timed out!");
    return(false);
  }
  
  // Stop tilt movement
  bool directedPerceptionPTU::stopTilt() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU!  Please initialize first.");
      return(false);
    } else if ( par.connectionToPTU.send("HT ") ) {    // Sending halt-tilt command
      cout << "Tilt movement haltet!" << endl;
      par.instructionCounter++;
      return(true);
    }
    setStatusString("Error with Serial Port : Command could not be send or timed out!");
    return(false);
  }
  
  // Set pan-tilt
  bool directedPerceptionPTU::setPanTilt(float pan, float tilt){
    parameters& par = getWritableParameters();
     setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU!  Please initialize first.");
      return(false);
     } 
     else {
       if ( par.angleFormat == parameters::Degrees ) {
         // Convert to Radiants if necessary
         pan *= constants<float>::Pi()/180.f;
         tilt*= constants<float>::Pi()/180.f;
       }
       if ( par.posMode == parameters::absolute ) {
         // Work on Pan
         if ( pan < par.minPan ) {
           // Set pan to minPan
           pan=par.minPan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         else if ( pan > par.maxPan ) {
           // Set pan to maxPan
           pan=par.maxPan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         //Work on Tilt
         if ( tilt < par.minTilt ) {
           //Set tilt to minTilt
           tilt=par.minTilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         else if ( tilt > par.maxTilt ) {
           //Set tilt to maxTilt
           tilt=par.maxTilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         char commandString[25]="";       
         sprintf(commandString,"PP%d TP%d ",convertValueToPTUnits(pan),convertValueToPTUnits(tilt));
         if( par.connectionToPTU.send( string(commandString) ) ) {
           // Set the internal pan and tilt values to PTU-position equivalent values, cuase this is only int
           pan=convertValueToPTUnits(pan)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           tilt=convertValueToPTUnits(tilt)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           par.pan=pan;
           par.tilt=tilt;
           par.instructionCounter+=2;
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }
       // Relative Movement
       else if ( par.posMode == parameters::relative ) {
         // Work on Pan
         if ( par.pan+pan < par.minPan ) {
           // calculate Maximum offset that can be reached up to the end of min Range
           pan=par.minPan-par.pan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         else if ( par.pan+pan > par.maxPan ) {
           // calculate Maximum offset that can be reached up to the end of max Range
           pan=par.maxPan-par.pan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         //Work on Tilt
         if ( par.tilt+tilt < par.minTilt ) {
           // calculate Maximum offset that can be reached up to the end of min Range
           tilt=par.minTilt-par.tilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         else if ( par.tilt+tilt > par.maxTilt ) {
           // calculate Maximum offset that can be reached up to the end of max Range
           tilt=par.maxTilt-par.tilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         char commandString[25]="";
         sprintf(commandString,"PO%d TO%d",convertValueToPTUnits(pan),convertValueToPTUnits(tilt));
         if ( par.connectionToPTU.send(string(commandString) ) ) {
           // Set the internal pan and tilt values to PTU-position equivalent values, cause this is only int
           pan=convertValueToPTUnits(pan)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           tilt=convertValueToPTUnits(tilt)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           par.pan +=pan;
           par.tilt+=tilt;
           par.instructionCounter+=2;
           setStatusString("");
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }    
    }
    return(false);
  }
  
  // Set pan position
  bool directedPerceptionPTU::setPan(float pan){
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU!  Please initialize first.");
      return(false);
     } 
     else {
       if ( par.angleFormat == parameters::Degrees ) {
         // Convert to Radiants if necessary
         pan *= constants<float>::Pi()/180.f;
       }
       if ( par.posMode == parameters::absolute ) {
         // Work on Pan
         if ( pan < par.minPan ) {
           // Set pan to minPan
           pan=par.minPan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         else if ( pan > par.maxPan ) {
           // Set pan to maxPan
           pan=par.maxPan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
        
         char commandString[10]="";       
         sprintf(commandString,"PP%d ",convertValueToPTUnits(pan) );
         if( par.connectionToPTU.send( string(commandString) ) ) {
           // Set the internal pan value to a PTU-position equivalent value, cause this is only int.
           pan=convertValueToPTUnits(pan)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           par.instructionCounter++;
           par.pan=pan;
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }
       // Relative Movement
       else if ( par.posMode == parameters::relative ) {
         // Work on Pan
         if ( par.pan+pan < par.minPan ) {
           // calculate Maximum offset that can be reached up to the end of min Range
           pan=par.minPan-par.pan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         else if ( par.pan+pan > par.maxPan ) {
           // calculate Maximum offset that can be reached up to the end of max Range
           pan=par.maxPan-par.pan;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         char commandString[10]="";
         sprintf(commandString,"PO%d ",convertValueToPTUnits(pan) );
         if ( par.connectionToPTU.send(string(commandString) ) ) {
           // Set the internal pan value to PTU-position equivalent value, cause this is only int
           pan=convertValueToPTUnits(pan)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           par.pan +=pan;
           par.instructionCounter++;
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }    
    }
    return(false);
  } 
  
  //Set tilt position
  bool directedPerceptionPTU::setTilt(float tilt){
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU!  Please initialize first.");
      return(false);
     } 
     else {
       if ( par.angleFormat == parameters::Degrees ) {
         // Convert to Radiants if necessary
         tilt*= constants<float>::Pi()/180.f;
       }
       if ( par.posMode == parameters::absolute ) {
         //Work on Tilt
         if ( tilt < par.minTilt ) {
           // Set tilt to minTilt
           tilt=par.minTilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         else if ( tilt > par.maxTilt ) {
           // Set tilt to maxTilt
           tilt=par.maxTilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
         }
         char commandString[10]="";       
         sprintf(commandString,"TP%d ",convertValueToPTUnits(tilt));
         if( par.connectionToPTU.send( string(commandString) ) ) {
           // Set the internal tilt value to PTU-position equivalent value, cause this is only int
           tilt=convertValueToPTUnits(tilt)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;           
           par.instructionCounter++;
           par.tilt=tilt;
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }
       // Relative Movement
       else if ( par.posMode == parameters::relative ) {
         //Work on Tilt
         if ( par.tilt+tilt < par.minTilt ) {
            // calculate Maximum offset that can be reached up to the end of min Range
           tilt=par.minTilt-par.tilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         else if ( par.tilt+tilt > par.maxTilt ) {
            // calculate Maximum offset that can be reached up to the end of min Range
           tilt=par.maxTilt-par.tilt;          
           setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
           return(true);
         }
         char commandString[10]="";
         sprintf(commandString,"TO%d ",convertValueToPTUnits(tilt));
         if ( par.connectionToPTU.send(string(commandString) ) ) {
           // Set the internal tilt value to PTU-position equivalent value, cause this is only int
           tilt=convertValueToPTUnits(tilt)*par.resolutionPerPosition/3600.f*constants<float>::Pi()/180.f;
           par.tilt+=tilt;
           par.instructionCounter++;
           return(true);
         }
         else {
           setStatusString("Error with Serial Port : Command could not be send or timed out!");
           return(false);
         }
       }    
    }
    return(false);
  }
  
 // Returns internal pan-tilt position
  bool directedPerceptionPTU::getPanTilt(float& pan, float& tilt) const {
    const parameters& par = getParameters();
    pan = float(par.pan);
    tilt = float(par.tilt);
    if (par.angleFormat == parameters::Degrees) {
      pan  *= 180.f / constants<float>::Pi();
      tilt *= 180.f / constants<float>::Pi();
    }
    return(true);
  }
  
  // Returns internal pan position
  float directedPerceptionPTU::getPan() const {
    const parameters& par = getParameters();
    if (par.angleFormat == parameters::Degrees) {
      return(par.pan * 180.f / constants<float>::Pi());
    }
    return(par.pan);
  }
  
  // Returns internal tilt position
  float directedPerceptionPTU::getTilt() const {
    const parameters& par = getParameters();
    if (par.angleFormat == parameters::Degrees) {
      return(par.tilt * 180.f / constants<float>::Pi());
    }
    return(par.tilt);
  }

 
  // Initializes the PTU
  bool directedPerceptionPTU::initialize() {
    setStatusString("");
    cout << "Initializing PTU.....";
    parameters& par = getWritableParameters();
    emptySerialBuffer();
    // Determine, if PTU responds to any commands
    par.connectionToPTU.send("A ");
    if ( par.connectionToPTU.getParameters().receiveTimeout < par.ptuResponseTime ) {
       passiveWait(1000*(par.ptuResponseTime-par.connectionToPTU.getParameters().receiveTimeout));
           }
    char cserial;
    int counter=0;
    while ( par.connectionToPTU.receive(cserial) ) {
      if ( cserial == '*' ) {
        counter++;
      }
    }
    if ( counter > 0 ) {
      cout << "SUCCESSFULL" << endl;
      par.connectedToPTU=true;
      par.instructionCounter=0;
      // Make primary settings
      cout << "Setting pan/tilt position to " << par.pan << ","<< par.tilt << " .....";
      if ( setPanTilt(par.pan,par.tilt) ) {
        cout << "OK" << endl;
      }
      else {
        cout << "FAILED" << endl;
      }
      
      cout << "Setting pan  speed to " << par.panSpeed << " radiants/sec.....";
      if ( setPanSpeed(par.panSpeed) ) {
        cout << "OK" << endl;
      }
      else {
        cout << "FAILED" << endl;
      }
      
      cout << "Setting tilt speed to " << par.tiltSpeed << " radiants/sec.....";
      if ( setTiltSpeed(par.tiltSpeed) ) {
        cout << "OK" << endl;
      }
      else {
        cout << "FAILED" << endl;
      }
      
      cout << "Setting pan  acceleration to " << par.panAcceleration << " radiants/sec^2.....";
      if ( setPanAcceleration(par.panAcceleration) ) {
        cout << "OK" << endl;
      }
      else {
        cout << "FAILED" << endl;
      }
      
      cout << "Setting tilt acceleration to " << par.tiltAcceleration << " radiants/sec^2.....";
      if ( setTiltAcceleration(par.tiltAcceleration) ) {
        cout << "OK" << endl;
      }
      else {
        cout << "FAILED" << endl;
      }
      awaitPosCommandCompletion();
      // Ensure save starting conditions
      while ( !isPTUidle() ) {
        // do nothing
      }
      cout << "Initialization completed!" << endl << endl;
      return(true);
    }
    else {
      cout << "FAILED" << endl;
      cout << "Could not connect PTU! Make sure, you've switched on the interface and cable connection is ok, before you try to initialize again" << endl; 
      par.connectedToPTU=false;
      setStatusString("Could not connect PTU! Make sure, you've switched on the interface and cable connection is ok."); 
      return(false);
    }
  }
  
  // Enable Autoscan/Monitor in pan axis
  bool directedPerceptionPTU::setAutoscan( float panPos1, float panPos2 ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } 
    else if ( par.posMode == parameters::relative ) {
      setStatusString("Autoscan only in absolute positions!");
      return(false);
    }
    else {
      if (par.angleFormat == parameters::Degrees ) {
        // Convert to Radiants if necessary
        panPos1 *= constants<float>::Pi()/180.f;
        panPos2 *= constants<float>::Pi()/180.f;
      }
      if ( panPos1 > par.maxPan ) {
        panPos1 = par.maxPan;
         setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos1 < par.minPan ) {
        panPos1 = par.minPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos2 > par.maxPan ) {
        panPos1 = par.maxPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos2 < par.minPan ) {
        panPos2 = par.minPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      char commandString[40];
      sprintf(commandString,"M%d,%d ",convertValueToPTUnits(panPos1),convertValueToPTUnits(panPos2) );
      if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
  }
  
  // Enables Autoscan/Monitor in pan-tilt axis
  bool directedPerceptionPTU::setAutoscan( float panPos1, float panPos2, float tiltPos1, float tiltPos2 ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } 
    else if ( par.posMode == parameters::relative ) {
      setStatusString("Autoscan only in absolute positions!");
      return(false);
    }
    else {
      if (par.angleFormat == parameters::Degrees ) {
        // Convert to Radiants if necessary
        panPos1 *= constants<float>::Pi()/180.f;
        tiltPos1*= constants<float>::Pi()/180.f;
        panPos2 *= constants<float>::Pi()/180.f;
        tiltPos2*= constants<float>::Pi()/180.f;
      }
      if ( panPos1 > par.maxPan ) {
        panPos1 = par.maxPan;
         setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos1 < par.minPan ) {
        panPos1 = par.minPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos2 > par.maxPan ) {
        panPos1 = par.maxPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( panPos2 < par.minPan ) {
        panPos2 = par.minPan;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( tiltPos1 > par.maxTilt ) {
        tiltPos1 = par.maxTilt;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( tiltPos1 < par.minTilt ) {
        tiltPos1 = par.minTilt;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( tiltPos2 > par.maxTilt ) {
        tiltPos2 = par.maxTilt;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      if ( tiltPos2 < par.minTilt ) {
        tiltPos2 = par.minTilt;
        setStatusString("Given pan and/or tilt position/s were out of range! Min/max position/s applied"); 
      }
      char commandString[40];
      sprintf(commandString,"M%d,%d,%d,%d",convertValueToPTUnits(panPos1),convertValueToPTUnits(panPos2),
              convertValueToPTUnits(tiltPos1),convertValueToPTUnits(tiltPos2) );
      if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
  }
  
  // Enables Autoscan/Monitoring in the last applied pan-tilt positions.
  // If none was applied the PTU works between max and min pan positions. 
  bool directedPerceptionPTU::setAutoscan() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } 
    else if ( par.posMode == parameters::relative ) {
      setStatusString("Autoscan only in absolute positions!");
      return(false);
    }
    else {
      if ( par.connectionToPTU.send("M ") ) {
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }  
  }    
        
  // Stops autoscan/monitoring
  bool directedPerceptionPTU::stopAutoscan() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else if ( par.posMode == parameters::relative ) {
      setStatusString("Autoscan only in absolute positions!");
      return(false);
    } 
	else {	
	  if ( par.connectionToPTU.send(" ") ) {
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
	}
  }
      
  // Sets pan speed. The parameter is interpreted as absolute speed setting.
  bool directedPerceptionPTU::setPanSpeed ( float panSpeed ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } 
    else {
      if ( par.angleFormat == parameters::Degrees ) {
          // Convert to Radiants if necessary
          panSpeed *= constants<float>::Pi()/180.f;
      }
      if ( panSpeed < par.minPanSpeed ) {
        panSpeed=par.minPanSpeed;
        setStatusString("Given pan speed was to low! Min speed applied"); 
      }
      else if ( panSpeed > par.maxPanSpeed ) {
        panSpeed=par.maxPanSpeed;
        setStatusString("Given pan speed was to high! Max speed applied"); 
      }
      
      char commandString[15];
      sprintf(commandString,"PS%d ",convertValueToPTUnits(panSpeed));
      if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.panSpeed=panSpeed;
        par.instructionCounter++;
        return(true);
      }
      else {
       setStatusString("Error with Serial Port : Command could not be send or timed out!");
       return(false);
      }
    }
  }
 
  // Sets tilt speed. The parameter is interpreted as absolute speed setting.
  bool directedPerceptionPTU::setTiltSpeed( float tiltSpeed ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else {
      if (par.angleFormat == parameters::Degrees ) {
          // Convert to Radiants if necessary
          tiltSpeed *= constants<float>::Pi()/180.f;
      }
      if ( tiltSpeed < par.minTiltSpeed ) {
        tiltSpeed=par.minTiltSpeed;
        setStatusString("Given tilt speed was to low! Min speed applied"); 
      }
      else if ( tiltSpeed > par.maxTiltSpeed ) {
        tiltSpeed=par.maxTiltSpeed;
        setStatusString("Given tilt speed was to high! Max speed applied"); 
      }
      
      char commandString[15];
      sprintf(commandString,"TS%d ",convertValueToPTUnits(tiltSpeed));
      if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.tiltSpeed=tiltSpeed;
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
  }
      
  // Sets pan acceleration. The parameter is interpreted as absolute acceleration setting.
  bool directedPerceptionPTU::setPanAcceleration ( float panAcc ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else {
      if (par.angleFormat == parameters::Degrees ) {
          // Convert to Radiants if necessary
          panAcc *= constants<float>::Pi()/180.f;
      }
      if ( panAcc < par.minPanAcceleration ) {
        panAcc=par.minPanAcceleration;
        setStatusString("Given pan acceleration was to low! Min acceleration applied"); 
      }
      else if ( panAcc > par.maxPanAcceleration ) {
        panAcc=par.maxPanAcceleration;
        setStatusString("Given pan acceleration was to high! Max acceleration applied"); 
      }
      
      char commandString[15];
      sprintf(commandString,"PA%d ",convertValueToPTUnits(panAcc));
      if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.panAcceleration=panAcc;
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
  }

  // Sets tilt acceleration. The parameter is interpreted as absolute acceleration setting.    
  bool directedPerceptionPTU::setTiltAcceleration( float tiltAcc ) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection set to PTU! Please initialize first.");
      return(false);
    } else {
      if (par.angleFormat == parameters::Degrees ) {
          // Convert to Radiants if necessary
          tiltAcc *= constants<float>::Pi()/180.f;
      }
      if ( tiltAcc < par.minTiltAcceleration ) {
        tiltAcc=par.minTiltAcceleration;
        setStatusString("Given tilt acceleration was to low! Min acceleration applied"); 
      }
      else if ( tiltAcc > par.maxTiltAcceleration ) {
        tiltAcc=par.maxTiltAcceleration;
        setStatusString("Given tilt acceleration was to high! Max acceleration applied"); 
      }
      
     char commandString[15];
     sprintf(commandString,"TA%d ",convertValueToPTUnits(tiltAcc));
     if ( par.connectionToPTU.send( string(commandString) )  ) {
        par.tiltAcceleration=tiltAcc;
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
  }
      
  // Converts the given angle value into the internal PTU format.
  // Values must be in radiants
  int directedPerceptionPTU::convertValueToPTUnits(float position) {
    int ptUnits;
    ptUnits=static_cast<int>((1/parameters::resolutionPerPosition)*3600.f*position/constants<float>::Pi()*180.f);  
    
    return(ptUnits);
  }
  
   //Sets the desired angle format for working with
   bool directedPerceptionPTU::setAngleFormat(parameters::eAngleFormatType anAngleFormat)
   {
     parameters& par=getWritableParameters();
     if (anAngleFormat == parameters::Degrees || anAngleFormat == parameters::Radiant ) {
       par.angleFormat=anAngleFormat;
       return(true);
     }
     return(false);
   }
     
   //  Reads out the serial buffer until it is empty 
   void directedPerceptionPTU::emptySerialBuffer() {
     parameters& par = getWritableParameters();
     string serialString;
     while(par.connectionToPTU.receive(serialString)) {
     // Do nothing
     }
   }
   
   // Returns whether the ptu is idle
   bool directedPerceptionPTU::isPTUidle() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection to PTU set! Please intialize first.");
      return(false);
    } 
    else {
      char serialChar;
      int completedInstructions=0;
      int failedInstructions=0;
      while(par.connectionToPTU.receive(serialChar)) {
        if ( serialChar == '*' ) {
          completedInstructions++;
        }
        if ( serialChar == '!' ) {
          failedInstructions++;    
        }
      } 
      par.instructionCounter=par.instructionCounter-completedInstructions-failedInstructions;
      if ( par.instructionCounter <= 0 ) {
        par.instructionCounter=0;
        return(true);
      }
      else {
        return(false);
        setStatusString("PTU is not idle!");
      }
    }
  } 
  
  // Gets current pan and tilt values, if PTU is idle
  bool directedPerceptionPTU::getCurrentPanTilt(float& pan, float& tilt) {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection to PTU set! Please intialize first.");
      return(false);
    } 
    else if( isPTUidle() ) {
      par.connectionToPTU.send("PP ");
      // Wait if Timeout is shorter than response time;
      if ( par.connectionToPTU.getParameters().receiveTimeout < par.ptuResponseTime ) {
        // Active waiting till serialbuffer sends answer
        passiveWait( 1000*(par.ptuResponseTime-par.connectionToPTU.getParameters().receiveTimeout) );
      }
      string serialString="";
      if ( par.connectionToPTU.receive(serialString) ) {
        string pp=serialString.substr(serialString.find_last_of(" ")+1);
        pan= (float) atoi(pp.c_str());
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
      
      par.connectionToPTU.send("TP ");
      // Wait if Timeout is shorter than response time;
      if ( par.connectionToPTU.getParameters().receiveTimeout < par.ptuResponseTime ) {
        // Active waiting till serialbuffer sends answer
        passiveWait( 1000*(par.ptuResponseTime-par.connectionToPTU.getParameters().receiveTimeout) );
      }
      serialString="";
      if ( par.connectionToPTU.receive(serialString) ) {
        string tp=serialString.substr(serialString.find_last_of(" ")+1);
        tilt= (float) atoi(tp.c_str());
        // Convert from internal values to radiants
        pan=pan*(par.resolutionPerPosition/3600.f)*constants<float>::Pi()/180.f;
        tilt=tilt*(par.resolutionPerPosition/3600.f)*constants<float>::Pi()/180.f;
        if ( par.angleFormat == parameters::Degrees ) {
          pan=pan*180.f/constants<float>::Pi();
          tilt=tilt*180.f/constants<float>::Pi();
        }
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    }
    else {
      setStatusString("PTU not idle!");
      return(false);
    }
  }
  
  // Gets current pan value, if PTU is idle.
  float directedPerceptionPTU::getCurrentPan() {
    float currentPan;
    float dummy;
    if  ( getCurrentPanTilt(currentPan,dummy) ) {
      return ( currentPan );
    }
    else {
      return(0);
    }
  }
  
  // Gets current tilt value, if PTU is idle.
  float directedPerceptionPTU::getCurrentTilt() {
    float currentTilt;
    float dummy;
    if  ( getCurrentPanTilt(dummy,currentTilt) ) {
      return ( currentTilt );
    }
    else {
      return(0);
    }
  }
   
  // Returns max pan-speed in desired angle format per second
  float directedPerceptionPTU::getMaxPanSpeed() {
    const parameters& par = getParameters();   
    if ( par.angleFormat == parameters::Degrees ) {
      return(par.maxPanSpeed/constants<float>::Pi()*180.f);
    }
    return(par.maxPanSpeed);
  }
   
  // Returns min pan-speed in desired angle format per second   
  float directedPerceptionPTU::getMinPanSpeed() {
    const parameters& par = getParameters();  
    if ( par.angleFormat == parameters::Degrees ) {
      return(par.minPanSpeed/constants<float>::Pi()*180.f);
    } 
    return(par.minPanSpeed);
  }
   
  // Returns max tilt-speed in desired angle format per second
  float directedPerceptionPTU::getMaxTiltSpeed() {
     const parameters& par = getParameters();   
     if ( par.angleFormat == parameters::Degrees ) {
       return(par.maxTiltSpeed/constants<float>::Pi()*180.f);
     } 
     return(par.maxTiltSpeed);
  }  
   
   // Returns min tilt-speed in desired angle format per second
  float directedPerceptionPTU::getMinTiltSpeed() {
    const parameters& par = getParameters();   
    if ( par.angleFormat == parameters::Degrees ) {
      return(par.minTiltSpeed/constants<float>::Pi()*180.f);
    } 
    return(par.minTiltSpeed);
  }
  
    // Returns max pan-acceleration in desired angle format per second^2 
  float directedPerceptionPTU::getMaxPanAcceleration() {
     const parameters& par = getParameters();   
     if ( par.angleFormat == parameters::Degrees ) {
       return(par.maxPanAcceleration/constants<float>::Pi()*180.f);
     } 
     return(par.maxPanAcceleration);
  }  
   
  // Returns min pan-acceleration in desired angle format per second^2
  float directedPerceptionPTU::getMinPanAcceleration() {
    const parameters& par = getParameters();   
    if ( par.angleFormat == parameters::Degrees ) {
      return(par.minPanAcceleration/constants<float>::Pi()*180.f);
    } 
    return(par.minPanAcceleration);
  }
   
   /**
    *  Returns max tilt-acceleration in desired angle format per second^2
    */
   float directedPerceptionPTU::getMaxTiltAcceleration() {
     const parameters& par = getParameters();   
     if ( par.angleFormat == parameters::Degrees ) {
       return(par.maxTiltAcceleration/constants<float>::Pi()*180.f);
     } 
     return(par.maxTiltAcceleration);
   }
   
   // Returns min tilt-acceleration in desired angle format per second^2
  float directedPerceptionPTU::getMinTiltAcceleration() {
    const parameters& par = getParameters();   
    if ( par.angleFormat == parameters::Degrees ) {
      return(par.minTiltAcceleration/constants<float>::Pi()*180.f);
    } 
    return(par.minTiltAcceleration);
  }
   
  // Sets the execution mode to immidiate or slaved
  bool directedPerceptionPTU::setExecMode(parameters::eExecModes anExecMode ) {
    parameters& par=getWritableParameters();
    setStatusString("");
    if ( anExecMode == parameters::immidiate ) {
      if ( par.connectionToPTU.send("I ") ) {
        par.execMode=anExecMode;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);  
      }
    }
    else if ( anExecMode == parameters::slaved ) {
      if ( par.connectionToPTU.send("S ") ) {
        par.execMode=anExecMode;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);  
      }
    }  
    return(false);
  }
   
  // Sets the positon mode to absolute or relative.
  bool directedPerceptionPTU::setPosMode(parameters::ePosModes anPosMode) {
    parameters& par=getWritableParameters();
     if ( anPosMode == parameters::absolute )  {
       par.posMode=anPosMode;
       par.relativeMovement=false;
       return(true);
     }
     else if ( anPosMode == parameters::relative ) {
       par.posMode=anPosMode;
       par.relativeMovement=true;
       return(true); 
     }
     else {
       return(false); 
     }
   }
  
  // Tells the PTU to wait with new position commands till the last pan and/or
  // tilt movement was completed.
  bool directedPerceptionPTU::awaitPosCommandCompletion() {
      parameters& par = getWritableParameters();
      setStatusString("");
      if ( !par.connectedToPTU ) {
        setStatusString("No connection to PTU set! Please intialize first.");
        return(false);
      }
      else if ( par.connectionToPTU.send("A ") ) {
        par.instructionCounter++;
        return(true);
      }
      else {
        setStatusString("Error with Serial Port : Command could not be send or timed out!");
        return(false);
      }
    } 
    
    
  bool directedPerceptionPTU::updatePanTilt() {
    parameters& par = getWritableParameters();
    setStatusString("");
    if ( !par.connectedToPTU ) {
      setStatusString("No connection to PTU set! Please intialize first.");
      return(false);
    }
    else {
      float panUpdate;
      float tiltUpdate;
      parameters::eAngleFormatType eSave=par.angleFormat;
      if ( par.angleFormat == parameters::Degrees ) {
        par.angleFormat=parameters::Radiant;
      }
      if ( getCurrentPanTilt(panUpdate,tiltUpdate) ) {
        par.pan=panUpdate;
        par.tilt=tiltUpdate;
        par.angleFormat=eSave;
        return(true);
      }
      else {   
        return(false);
      }
    }
  }
// END of .cpp-File
}
