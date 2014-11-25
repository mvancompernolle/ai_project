/** TDSP *******************************************************************
                          TDSPLTVChannel.cpp  -  description
                             -------------------
    begin                : Don Jul 17 2003
    copyright            : (C) 2003 by Peter Jung
    email                : jung@hhi.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TDSPLTVChannel.h"

ClassImpQ(TDSPLTVChannel)


#define DEBUG_CHANNELMATRIX   0x1
#define DEBUG_IMPULSERESPONSE 0x2

TDSPLTVChannel::TDSPLTVChannel(char *name): TDSPOutput1(name){
  
  fHaveRepresentation=kFALSE;
  fHaveImpulseResponse=kFALSE;
  fHaveChannelMatrix=kFALSE;

  fCalculateChannelMatrixCyclic=kFALSE;
  fCalculateChannelMatrix=kFALSE;
  
  fImpulseResponse    = NULL;
  fChannelMatrix      = NULL;
  
  fBasicChannelType   = 0; // not used
  fPowerDelayProfileSqrt  = NULL; 

}
TDSPLTVChannel::~TDSPLTVChannel(){
}


void        TDSPLTVChannel::SetModel(Option_t *mod,
				     TDSPVector *profileSqrt) {

  TString opt = mod;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("none"))!= kNPOS) {
    opt.Remove(pos,4);
    fBasicChannelType=0;
  }
  if ((pos = opt.Index("fixed"))!= kNPOS) {
    opt.Remove(pos,5);
    fBasicChannelType=1;
    if (profileSqrt) fPowerDelayProfileSqrt = profileSqrt;
  }
  if ((pos = opt.Index("randnc"))!= kNPOS) {
    opt.Remove(pos,6);
    fBasicChannelType=2;
    if (profileSqrt) fPowerDelayProfileSqrt = profileSqrt;
  }

  if (opt.Strip().Length()) 
    Error("SetOption","Unknown model specification \"%s\" !!",opt.Strip().Data());
}


/* this creates the channel impulse response for some basic
   channel models */

TDSPMatrix* TDSPLTVChannel::CreateImpulseResponse(TDSPMatrix *out) {
  
  Int_t bl = GetBlockLen();

  switch(fBasicChannelType) {

  case 0: /* none */
    Error("CreateImpulseResponse","No Basic Channeltype configured !!");
    return NULL;

  case 1: /* fixed LTI-Channel */
  case 2: /* randnc channel */

    if (!fPowerDelayProfileSqrt) {
      Error("CreateImpulseResponse","No Power delay profile specified !");
      return NULL;
    }

    if (!bl) {
      Error("CreateImpulseResponse","Specify a block length first !");
      return NULL;
    }
    if (!out) out = new TDSPMatrix();
    out->SetSize(fPowerDelayProfileSqrt->GetLen(),bl);

    for(Int_t time=0;time<bl;++time)
      out->SetColumn(time,fPowerDelayProfileSqrt);
    
    switch(fBasicChannelType) {
    case 2: /* randomize */
      out->RandNC(NULL,out);
      break;
    default:
      break;
    }
  }
  return out;
}  
/* create the dim x dim-channel matrix in the time domain for the range
   TimeStart..TimeEnd-1 from the time-variant filter (ImpulseResponse). 
   If TimeStart==TimeEnd==0 it creates a matrix for the full time axis */

TDSPMatrix* TDSPLTVChannel::CreateChannelMatrix(TDSPMatrix *out, 
						Int_t TimeStart,
						Int_t TimeEnd) {
  
  static TDSPMatrix submatrix;// this we will fix later

  if ((!TimeStart)&&(!TimeEnd)) {
    TimeStart = 0;
    TimeEnd   = GetChannelMatrixSize();
  }
  Int_t  dim=TimeEnd-TimeStart;

  
  TDSPMatrix*inp           = GetImpulseResponse();
  
  inp->TransposedSubMatrix(0, TimeStart, 
			   inp->GetRows(),dim, 
			   &submatrix);
  out  = ConvMatrix(&submatrix, 
		    dim,dim, out,fCalculateChannelMatrixCyclic);
  return out;
}

void TDSPLTVChannel::ChangeRepresentation() {
  fHaveRepresentation=kTRUE;
  fHaveImpulseResponse=kFALSE;
  fHaveChannelMatrix=kFALSE;
}


TDSPMatrix* TDSPLTVChannel::GetImpulseResponse() {
  if (!fHaveRepresentation)  
    ChangeRepresentation();
  if (!fHaveImpulseResponse) {
    fImpulseResponse = CreateImpulseResponse(fImpulseResponse);
    fHaveImpulseResponse=kTRUE;
    /* Emit a "new impulse response is there" */
    ImpulseResponse(fImpulseResponse);
  }
  return fImpulseResponse;
}

TDSPMatrix* TDSPLTVChannel::GetChannelMatrix(Int_t TimeSlot) {
  if ((!fHaveChannelMatrix)||(fChannelMatrixSlot!=TimeSlot)) {
    if (!(
	  fChannelMatrix = 
	  CreateChannelMatrix(fChannelMatrix, 
			      TimeSlot,
			      TimeSlot+GetChannelMatrixSize()))) {
      Error("GetChannelMatrix",
	    "Error during calculation of channel matrix for time slot %d!",
	    TimeSlot);
      return NULL;
    } else {
      fHaveChannelMatrix=kTRUE;
      fChannelMatrixSlot=TimeSlot;
    }
  }
  return fChannelMatrix;
}

void        TDSPLTVChannel::ChannelMatrices(Int_t ilen) {

  Int_t ChanMatSize      = GetChannelMatrixSize();
  TDSPMatrix* imp        = GetImpulseResponse();
  Int_t ChanLength       = 0;

  if (imp) 
    ChanLength = imp->GetCols();
  else {
    Error("ChannelMatrices","No impulse response !");
    return;
  }
  
  if (!ilen) ilen = ChanLength;
  
  if (ChanMatSize) {
    /* Check for the right block-processing for channel matrices */
    if (ilen%ChanMatSize) {
      Error("Apply","Input signal len(%d) is not a multiple of the channel matrix size (%d)!",
	    ilen,ChanMatSize);
      return;
    }
  }
  
  if (ChanMatSize&&ilen) {
    
    /* number of channel matrices we have to emit */
    Int_t numChanMat = ilen/ChanMatSize; 
    
    /* loop over the blocks (for each 1 channel matrix) */
    for(Int_t iChanMat=0;iChanMat<numChanMat;++iChanMat) {
      
      if (fIsActive) {
	/* calculate the corresponding slot in the channel 
	   must be modulo the total channel length */
	Int_t iChannel = (iChanMat*ChanMatSize)%ChanLength;
	/* Calculate the channel matrix*/
	GetChannelMatrix(iChannel);
      } else {
	/* UnitMatrix */
	if (!fChannelMatrix) fChannelMatrix = new TDSPMatrix();
	fChannelMatrix->SetSize(ChanMatSize,ChanMatSize);
	fChannelMatrix->Unit();
      }
      
      /* Emit a "new channel matrix is there" */
      ChannelMatrix(fChannelMatrix);
      
    }
  } else 
    Error("Apply","channel matrix size not specified (use SetChannelMatrixSize(..))!");
}

TDSPVector* TDSPLTVChannel::Apply(TDSPVector *in, TDSPVector *out) {
  
  TDSPMatrix *imp = NULL;
  
  if (!out) out          = new TDSPVector();
  Int_t    ilen          = in->GetLen();
  
  
  
  if (fIsActive) {

    /* get the impulse response */
    imp = GetImpulseResponse();
    if (!imp) {
      Error("Apply","GetImpulseResponse() failed !!");
      return NULL;
    }
    
    /* process the input data */
    imp->RowConvolute(in, out,
		      kTRUE, /* cyclic input ? */
		      kTRUE  /* zero the vector before ! */
		      );
  } else
    
    /* simply copy ... */
    in->Copy(out);
  

  out->Configure(in);

  /* Calculate and emit channel matrices */

  if (fCalculateChannelMatrix) 
    ChannelMatrices(ilen);

  return out;
}




TDSPSignal* TDSPLTVChannel::Update(TDSPSignal* input) {
  Apply(input,fSignal);
  Emit("Update(TDSPSignal*)",fSignal);
  return fSignal;  
}




void TDSPLTVChannel::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("debugchannelmatrix"))!= kNPOS) {
    fDebugBits|=DEBUG_CHANNELMATRIX;
    opt.Remove(pos,20);
  }
  if ((pos = opt.Index("cyclicchannelmatrix"))!= kNPOS) {
    fCalculateChannelMatrix=kTRUE;
    fCalculateChannelMatrixCyclic=kTRUE;
    opt.Remove(pos,21);
  }
  if ((pos = opt.Index("nochannelmatrix"))!= kNPOS) {
    fCalculateChannelMatrix=kFALSE;
    opt.Remove(pos,15);
  }
  if ((pos = opt.Index("channelmatrix"))!= kNPOS) {
    fCalculateChannelMatrix=kTRUE;
    fCalculateChannelMatrixCyclic=kFALSE;
    opt.Remove(pos,15);
  }
  if (opt.Strip().Length()) 
    Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
}



void TDSPLTVChannel::Print() {

  switch(fBasicChannelType) {

  case 0:
    cout << "No basic channel type set..." << endl;
    break;
  case 1:
    cout << "Fixed Time-Invariant Channel with given power delay profile" << endl;
    cout << "Sqrt(PowerDelayProfile)=";
    if (fPowerDelayProfileSqrt) {
      fPowerDelayProfileSqrt->Print();
    } else
      cout << "(not specified)" << endl;
      
    break;
  case 2:
    cout << "RandNC Time-Variant Channel with given power delay profile" << endl;
    cout << "Sqrt(PowerDelayProfile)=";
    if (fPowerDelayProfileSqrt) {
      fPowerDelayProfileSqrt->Print();
    } else 
      cout << "(not specified)" << endl;
    break;

  }


}

void    TDSPLTVChannel::ImpulseResponse(TDSPMatrix*i){ 
 // if (fDebugBits&DEBUG_IMPULSERESPONSE) {
//     i->Print();
//     TCanvas *dw = DebugGetWindow("Impulse Response");
//     dw->Clear();
//     dw->cd();
//     i->Draw();
//     dw->Update();    
//     DebugWaitToContinue();
    
//  }
 Emit("ImpulseResponse(TDSPMatrix*)",i);
}


void TDSPLTVChannel::ChannelMatrix(TDSPMatrix*i) { 
  // if (fDebugBits&DEBUG_CHANNELMATRIX) {
//     i->Print();
//     TCanvas *dw = DebugGetWindow("Channel Matrix");
//     dw->Clear();
//     dw->cd();
//     i->Draw("hist box");
//     dw->Update();    
//     if (!DebugWaitToContinue()) fDebugBits&=!(DEBUG_CHANNELMATRIX);
    
//   }
 Emit("ChannelMatrix(TDSPMatrix*)",i);

}
