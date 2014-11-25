/** TDSP *******************************************************************
                          TDSPBlockOperation.cpp  -  description
                             -------------------
    begin                : Don Mai 16 2002
    copyright            : (C) 2002 by Peter Jung
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

#include <TError.h>
#include "TDSPBlockOperation.h"

ClassImp(TDSPBlockOperation)

TDSPBlockOperation::TDSPBlockOperation(){
}
TDSPBlockOperation::~TDSPBlockOperation(){
}

TDSPVector* TDSPBlockOperation::Sum(Int_t       bl,
				    TDSPVector *in, 
				    TDSPVector *result) {
  
  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();

  
  if (len%bl) {
    Warning("Sum","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  TComplex  *ov = result->GetVec();
  result->Zeros();


  for(register Int_t i=0;i<len;++i) {
    ov[i%bl] += iv[i];
  }


  return result;
}

TDSPVector* TDSPBlockOperation::SumDiff(Int_t       bl,
					TDSPVector *in1, 
					TDSPVector *in2, 
					TDSPVector *result,
					TComplex    scaler) {
  
  
  Int_t     len1  = in1->GetLen();
  TComplex  *iv1  = in1->GetVec();


  Int_t     len2  = in1->GetLen();
  TComplex  *iv2  = in2->GetVec();

  if (len1!=len2) {
    Warning("Diff","input length differ (%d<>%d)",len1,len2);
  }

  if (len1%bl) {
    Warning("Diff","length of input vector1 (%d) is not a multiple of the block len (%d)!",len1,bl);
  }

  if (len2%bl) {
    Warning("Diff","length of input vector2 (%d) is not a multiple of the block len (%d)!",len2,bl);
  }

  Int_t len=TMath::Min(len1,len2);

  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  TComplex  *ov = result->GetVec();

  result->Zeros();
  
  if (scaler==1.) {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += iv1[i]-iv2[i];
    }
  } else {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += iv1[i]-scaler*iv2[i];
    }
  }

  return result;
}

TDSPVector* TDSPBlockOperation::Sum2Diff(Int_t       bl,
					 TDSPVector *in1, 
					 TDSPVector *in2, 
					 TDSPVector *result,
					 TComplex    scaler) {
  
  
  Int_t     len1  = in1->GetLen();
  TComplex  *iv1  = in1->GetVec();


  Int_t     len2  = in1->GetLen();
  TComplex  *iv2  = in2->GetVec();

  if (len1!=len2) {
    Warning("Diff","input length differ (%d<>%d)",len1,len2);
  }

  if (len1%bl) {
    Warning("Diff","length of input vector1 (%d) is not a multiple of the block len (%d)!",len1,bl);
  }

  if (len2%bl) {
    Warning("Diff","length of input vector2 (%d) is not a multiple of the block len (%d)!",len2,bl);
  }

  Int_t len=TMath::Min(len1,len2);

  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  TComplex  *ov = result->GetVec();

  result->Zeros();
  
  if (scaler==1.) {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += Abs2(iv1[i]-iv2[i]);
    }
  } else {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += Abs2(iv1[i]-scaler*iv2[i]);
    }
  }

  return result;
}

TDSPVector* TDSPBlockOperation::SumPhaseDiff(Int_t       bl,
					     TDSPVector *in1, 
					     TDSPVector *in2, 
					     TDSPVector *result,
					     TComplex    scaler) {
  
  
  Int_t     len1  = in1->GetLen();
  TComplex  *iv1  = in1->GetVec();


  Int_t     len2  = in1->GetLen();
  TComplex  *iv2  = in2->GetVec();

  if (len1!=len2) {
    Warning("SumPhaseDiff","input length differ (%d<>%d)",len1,len2);
  }

  if (len1%bl) {
    Warning("SumPhaseDiff","length of input vector1 (%d) is not a multiple of the block len (%d)!",len1,bl);
  }

  if (len2%bl) {
    Warning("SumPhaseDiff","length of input vector2 (%d) is not a multiple of the block len (%d)!",len2,bl);
  }

  Int_t len=TMath::Min(len1,len2);

  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  TComplex  *ov = result->GetVec();

  result->Zeros();
  
  if (scaler==1.) {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += Phase(iv1[i]%iv2[i]);
    }
  } else {
    for(register Int_t i=0;i<len;++i) {
      ov[i%bl] += Phase(iv1[i]%(scaler*iv2[i]));
    }
  }

  return result;
}

TDSPVector* TDSPBlockOperation::Sum2PhaseDiff(Int_t       bl,
					      TDSPVector *in1, 
					      TDSPVector *in2, 
					      TDSPVector *result,
					      TComplex    scaler) {
  
  
  Int_t     len1  = in1->GetLen();
  TComplex  *iv1  = in1->GetVec();


  Int_t     len2  = in1->GetLen();
  TComplex  *iv2  = in2->GetVec();

  if (len1!=len2) {
    Warning("Sum2PhaseDiff","input length differ (%d<>%d)",len1,len2);
  }

  if (len1%bl) {
    Warning("Sum2PhaseDiff","length of input vector1 (%d) is not a multiple of the block len (%d)!",len1,bl);
  }

  if (len2%bl) {
    Warning("Sum2PhaseDiff","length of input vector2 (%d) is not a multiple of the block len (%d)!",len2,bl);
  }

  Int_t len=TMath::Min(len1,len2);

  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  TComplex  *ov = result->GetVec();
  
  result->Zeros();
  Double_t tmp;
  if (scaler==1.) {
    for(register Int_t i=0;i<len;++i) {
      tmp       = Phase(iv1[i]%iv2[i]);
      ov[i%bl] += tmp*tmp;
    }
  } else {
    for(register Int_t i=0;i<len;++i) {
      tmp       = Phase(iv1[i]%(scaler*iv2[i]));
      ov[i%bl] += tmp*tmp;
    }
  }

  return result;
}

TDSPVector* TDSPBlockOperation::MeanPhaseDiff(Int_t       bl,
					      TDSPVector *in1, 
					      TDSPVector *in2, 
					      TDSPVector *result,
					      TComplex    scaler) {
  result=SumPhaseDiff(bl,in1,in2,result,scaler);
  *result/=TMath::Min(in1->GetLen(),in2->GetLen())/bl;
  return result;
}

TDSPVector* TDSPBlockOperation::MeanDiff(Int_t       bl,
					 TDSPVector *in1, 
					 TDSPVector *in2, 
					 TDSPVector *result,
					 TComplex    scaler) {
  result=SumDiff(bl,in1,in2,result,scaler);
  *result/=TMath::Min(in1->GetLen(),in2->GetLen())/bl;
  return result;
}


TDSPVector* TDSPBlockOperation::Mean2Diff(Int_t       bl,
					  TDSPVector *in1, 
					  TDSPVector *in2, 
					  TDSPVector *result,
					  TComplex    scaler) {
  result=Sum2Diff(bl,in1,in2,result,scaler);
  *result/=TMath::Min(in1->GetLen(),in2->GetLen())/bl;
  return result;
}

TDSPVector* TDSPBlockOperation::Mean2PhaseDiff(Int_t       bl,
					       TDSPVector *in1, 
					       TDSPVector *in2, 
					       TDSPVector *result,
					       TComplex    scaler) {
  result=Sum2PhaseDiff(bl,in1,in2,result,scaler);
  *result/=TMath::Min(in1->GetLen(),in2->GetLen())/bl;
  return result;
}

TDSPVector* TDSPBlockOperation::Mean(Int_t       bl,
				     TDSPVector *in, 
				     TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();
  Int_t     num = len/bl;
  
  if (len%bl) {
    Error("Mean","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();

  for(register Int_t i=0;i<len;++i) {
    ov[i%bl] += iv[i];
  }
  { for(register Int_t i=0;i<bl;++i) ov[i] /= num; }

  return result;
}


TDSPVector* TDSPBlockOperation::MeanAbs(Int_t       bl,
					TDSPVector *in, 
					TDSPVector *result) {
  result = MeanAbs2(bl,in,result);
  result->SqrtReal();
  return result;
  
}

TDSPVector* TDSPBlockOperation::I_MeanAbs(Int_t       bl,
					  TDSPVector *in, 
					  TDSPVector *result) {
  result = I_MeanAbs2(bl,in,result);
  result->SqrtReal();
  return result;
  
}

TDSPVector* TDSPBlockOperation::MeanAbs2(Int_t       bl,
					 TDSPVector *in, 
					 TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();
  Int_t     num = len/bl;
  
  if (len%bl) {
    Error("MeanAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();

  for(register Int_t i=0;i<len;++i) {
    ov[i%bl].fRe += ::Abs2(iv[i]);
  }
  { for(register Int_t i=0;i<bl;++i) ov[i].fRe /= num; }

  return result;
}

TDSPVector* TDSPBlockOperation::I_MeanAbs2(Int_t       bl,
					   TDSPVector *in, 
					   TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  Int_t     len = in->GetLen();
  Int_t     num = len/bl;
  result->SetLen(num);
  
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();
  
  if (len%bl) {
    Error("I_MeanAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();

  for(register Int_t i=0;i<len;++i) {
    ov[i/bl].fRe += ::Abs2(*iv++);
  }
  { for(register Int_t i=0;i<num;++i) ov[i].fRe /= bl; }

  return result;
}

TDSPVector* TDSPBlockOperation::MaxAbs(Int_t       bl,
				       TDSPVector *in, 
				       TDSPVector *result) {
  result = MaxAbs2(bl,in,result);
  result->SqrtReal();
  return result;
  
}

TDSPVector* TDSPBlockOperation::I_MaxAbs(Int_t       bl,
					 TDSPVector *in, 
					 TDSPVector *result) {
  result = I_MaxAbs2(bl,in,result);
  result->SqrtReal();
  return result;
  
}

TDSPVector* TDSPBlockOperation::MinAbs(Int_t       bl,
				       TDSPVector *in, 
				       TDSPVector *result) {
  result = MinAbs2(bl,in,result);
  result->SqrtReal();
  return result;
}

TDSPVector* TDSPBlockOperation::I_MinAbs(Int_t       bl,
					 TDSPVector *in, 
					 TDSPVector *result) {
  result = I_MinAbs2(bl,in,result);
  result->SqrtReal();
  return result;
}

TDSPVector* TDSPBlockOperation::MaxAbs2(Int_t       bl,
					TDSPVector *in, 
					TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();
  
  if (len%bl) {
    Error("MaxAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();
  Double_t m;
  for(register Int_t i=0;i<len;++i) {
    m = iv[i] % iv[i];
    if (m>ov[i%bl].fRe) ov[i%bl].fRe = m;
  }
  return result;
}

TDSPVector* TDSPBlockOperation::I_MaxAbs2(Int_t       bl,
					  TDSPVector *in, 
					  TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  Int_t     len = in->GetLen();
  Int_t     num = len/bl;

  result->SetLen(num);
  
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();

  
  if (len%bl) {
    Error("I_MaxAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();
  Double_t m;
  for(register Int_t i=0;i<len;++i) {
    m = iv[i] % iv[i];
    if (m>ov[i/bl].fRe) ov[i/bl].fRe = m;
  }
  return result;
}

TDSPVector* TDSPBlockOperation::MinAbs2(Int_t       bl,
					TDSPVector *in, 
					TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  result->SetLen(bl);
  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();

  
  if (len%bl) {
    Error("MinAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();
  Double_t m;
  for(register Int_t i=0;i<len;++i) {
    m = iv[i] % iv[i];
    if (m<ov[i%bl].fRe) ov[i%bl].fRe = m;
  }
  return result;
}

TDSPVector* TDSPBlockOperation::I_MinAbs2(Int_t       bl,
					  TDSPVector *in, 
					  TDSPVector *result) {
  
  if (!result) result = new TDSPVector();
  Int_t     len = in->GetLen();
  Int_t     num = len/bl;

  result->SetLen(num);
  
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();

  
  if (len%bl) {
    Error("I_MinAbs2","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();
  Double_t m;
  for(register Int_t i=0;i<len;++i) {
    m = iv[i] % iv[i];
    if (m<ov[i/bl].fRe) ov[i/bl].fRe = m;
  }
  return result;
}

TDSPVector* TDSPBlockOperation::MeanPower(Int_t       bl,
					  TDSPVector *in, 
					  TDSPVector *result) {
  
  if (!result) result = new TDSPVector();

  result->SetLen(bl);

  
  Int_t     len = in->GetLen();
  TComplex  *iv = in->GetVec();
  TComplex  *ov = result->GetVec();

  Int_t     num = len/bl;
  
  if (len%bl) {
    Error("MeanPower","length of input vector (%d) is not a multiple of the block len (%d)!",len,bl);
  }

  result->Zeros();

  for(register Int_t i=0;i<len;++i) {
    ov[i%bl] += Abs2(iv[i]);
  }
  for(register Int_t j=0;j<bl;++j) ov[j] /= num;

  return result;
}



