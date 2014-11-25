//  Copyright (c) 2005, Ben McCart
//  All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions are met:
//  
//      * Redistributions of source code must retain the above copyright notice,
//        this list of conditions and the following disclaimer.
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//      * Neither the name of the C++ Fuzzy Logic Programming Library nor the
//        names of its contributors may be used to endorse or promote products
//        derived from this software without specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.

#include "FuzzySubPartition.h"
#include "FuzzyRule.h"
#include "FuzzyElement.h"
#include "FuzzySet.h"
#include "FuzzyMembershipFunction.h"
#include "FuzzyAntecedent.h"
#include <cassert>
#include <cmath>

CFuzzyAntecedent::CFuzzyAntecedent()
  : m_pFE(0), m_pFS(0), m_pFMF(0), m_pAssociatedFS(0), m_pAssociatedFMF(0), m_pLeft(0), m_pRight(0), m_Tnorm(false), m_Tconorm(false)
{

}

CFuzzyAntecedent::CFuzzyAntecedent(const CFuzzyAntecedent &right)
  : m_pFE(0), m_pFS(0), m_pFMF(0), m_pAssociatedFS(0), m_pAssociatedFMF(0), m_pLeft(0), m_pRight(0), m_Tnorm(false), m_Tconorm(false)
{
  *this = right;
}

CFuzzyAntecedent::~CFuzzyAntecedent()
{
  delete m_pLeft;
  delete m_pRight;
}


CFuzzyAntecedent & CFuzzyAntecedent::operator=(const CFuzzyAntecedent &right)
{
  m_pFE            = right.GetpFE();
  m_pFS            = right.GetpFS();
  m_pFMF           = right.GetpFMF();
  m_pAssociatedFS  = right.GetpAssociatedFS();
  m_pAssociatedFMF = right.GetpAssociatedFMF();
  
  m_Tnorm = right.GetTnorm();
  m_Tconorm = right.GetTconorm();

  if (right.m_pLeft)
  {
    m_pLeft = new CFuzzyAntecedent(*right.m_pLeft);
  }
  if (right.m_pRight)
  {
    m_pRight = new CFuzzyAntecedent(*right.m_pRight);
  }

  return *this;
}

int CFuzzyAntecedent::operator==(const CFuzzyAntecedent &right) const
{
  assert(0);
  return false;
}

int CFuzzyAntecedent::operator!=(const CFuzzyAntecedent &right) const
{
  return !(*this == right);
}

void CFuzzyAntecedent::operator >> (const CFuzzyAntecedent right) const
{
  *right.GetpFE() = Recurse(*this,right);  

  // Combine results for fuzzy rule.
  if (CFuzzyRule::IsEnabled())
  {
    CFuzzyRule* pFuzzyRule = CFuzzyRule::GetpInstance();
    pFuzzyRule->CombineResults(*(right.GetpFE()));
  }
}

void CFuzzyAntecedent::operator >> (CFuzzySet& right) const
{
  right = Recurse(*this,right);  

  // Combine results for fuzzy rule.
  if (CFuzzyRule::IsEnabled())
  {
    CFuzzyRule* pFuzzyRule = CFuzzyRule::GetpInstance();
    pFuzzyRule->CombineResults(right);
  }
}

void CFuzzyAntecedent::operator >> (CFuzzyMembershipFunction& right) const
{
  right = Recurse(*this,right);  

  if (CFuzzyRule::IsEnabled())
  {
    CFuzzyRule* pFuzzyRule = CFuzzyRule::GetpInstance();
    pFuzzyRule->CombineResults(right);
  }
}

const CFuzzySet CFuzzyAntecedent::operator >>= (const CFuzzySet& right) const
{
  assert(0);
  CFuzzySet FS;
  return FS;
/*
// begin CFuzzyMembershipFunction::operator>>=
  CFuzzyMembershipFunction        FMFleft;
  CFuzzySet                       FSright;                                 
  CFuzzyMembershipFunction FMFreturn;
  bool LeftMF = false,
       RightSet = false;
  
  if (GetpAssociatedFS())
  {
    FMFleft = *this && *GetpAssociatedFS();
    LeftMF = true;
  }

  if (GetpAssociatedFMF())
  {
    FMFleft = *this && *GetpAssociatedFMF();
    LeftMF = true;
  }

  if (!LeftMF)
  {
    FMFleft = *this;
  }

  if (right.GetpAssociatedFS())
  {
    FSright = right && *right.GetpAssociatedFS();
    RightSet = true;
  }

  if (right.GetpAssociatedFMF())
  {
    FSright = right && *right.GetpAssociatedFMF();
    RightSet = true;
  }

  if (!RightSet)
  {
    FSright = right;
  }

  FMFreturn = !FMFleft || (FMFleft && FSright);
  return FMFreturn;
// end CFuzzyMembershipFunction::operator>>=

// begin CFuzzySet::operator>>
  CFuzzySet FSleft;
  CFuzzySet FSright;
  CFuzzySet FSreturn;
  bool LeftSet = false,
       RightSet = false;
  
  if (GetpAssociatedFS())
  {
    FSleft = *this && *GetpAssociatedFS();
    LeftSet = true;
  }

  if (GetpAssociatedFMF())
  {
    FSleft = *this && *GetpAssociatedFMF();
    LeftSet = true;
  }

  if (!LeftSet)
  {
    FSleft = *this;
  }

  if (right.GetpAssociatedFS())
  {
    FSright = right && *right.GetpAssociatedFS();
    RightSet = true;
  }

  if (right.m_pAssociatedFMF)
  {
    FSright = right && *right.GetpAssociatedFMF();
    RightSet = true;
  }

  if (!RightSet)
  {
    FSright = right;
  }

  FSreturn = !FSleft || (FSleft && FSright);
  return FSreturn;
// end CFuzzySet::operator>>=
*/
}

const CFuzzySet CFuzzyAntecedent::operator >>= (const CFuzzyMembershipFunction& right) const
{
  assert(0);
  CFuzzySet FS;
  return FS;

  /*
// begin CFuzzySet::operator>>=
  CFuzzySet FSleft;
   
  assert(1 == 2);
  CFuzzySet FMFright;

  CFuzzySet FSreturn;
  bool LeftSet = false;
  bool RightSet = false;
  
  if (GetpAssociatedFS())
  {
    FSleft = *this && *GetpAssociatedFS();
    LeftSet = true;
  }

  if (GetpAssociatedFMF())
  {
    FSleft = *this && *GetpAssociatedFMF();
    LeftSet = true;
  }

  if (!LeftSet)
  {
    FSleft = *this;
  }

  if (right.GetpAssociatedFS())
  {
    FMFright = right && *right.GetpAssociatedFS();
    RightSet = true;
  }

  if (right.GetpAssociatedFMF())
  {
    FMFright = right && *right.GetpAssociatedFMF();
    RightSet = true;
  }

  if (!RightSet)
  {
    FMFright = right;
  }

  FSreturn = !FSleft || (FSleft && FMFright);
  return FSreturn;
// end CFuzzySet::operator>>=

// begin CFuzzyMembershipFunction::operator>>=
  CFuzzyMembershipFunction        FMFleft,
                                  FMFright;
  CFuzzyMembershipFunction FMFreturn;
  bool LeftMF = false,
       RightMF = false;
  
  if (GetpAssociatedFS())
  {
    FMFleft = *this && *GetpAssociatedFS();
    LeftMF = true;
  }

  if (GetpAssociatedFMF())
  {
    FMFleft = *this && *GetpAssociatedFMF();
    LeftMF = true;
  }

  if (!LeftMF)
  {
    FMFleft = *this;
  }

  if (right.GetpAssociatedFS())
  {
    FMFright = right && *right.GetpAssociatedFS();
    RightMF = true;
  }

  if (right.GetpAssociatedFMF())
  {
    FMFright = right && *right.GetpAssociatedFMF();
    RightMF = true;
  }

  if (!RightMF)
  {
    FMFright = right;
  }

  FMFreturn = !FMFleft || (FMFleft && FMFright);
  return FMFreturn;
// end CFuzzyMembershipFunction::operator>>=
*/
}

const CFuzzyAntecedent CFuzzyAntecedent::operator || (const CFuzzyAntecedent& right) const
{
  CFuzzyAntecedent Result;
  CFuzzyAntecedent *pLeft = new CFuzzyAntecedent(*this);
  CFuzzyAntecedent *pRight = new CFuzzyAntecedent(right);
  Result.SetpLeft(pLeft);
  Result.SetpRight(pRight);
  Result.SetTconorm(true);
  return Result;
}

const CFuzzyAntecedent CFuzzyAntecedent::operator && (const CFuzzyAntecedent& right) const
{
  CFuzzyAntecedent Result;
  CFuzzyAntecedent *pLeft = new CFuzzyAntecedent(*this);
  CFuzzyAntecedent *pRight = new CFuzzyAntecedent(right);
  Result.SetpLeft(pLeft);
  Result.SetpRight(pRight);
  Result.SetTnorm(true);
  return Result;
}

void CFuzzyAntecedent::SetpFE (CFuzzyElement* pFE)
{
  m_pFE  = pFE;
  m_pFS  = 0;
  m_pFMF = 0;
}

void CFuzzyAntecedent::SetpFS (const CFuzzySet* pFS)
{
  m_pFS  = pFS;
  m_pFE  = 0;
  m_pFMF = 0;
}

void CFuzzyAntecedent::SetpFMF (const CFuzzyMembershipFunction* pFMF)
{
  m_pFMF = pFMF;
  m_pFE  = 0;
  m_pFS  = 0;
}

void CFuzzyAntecedent::SetpAssociatedFS (const CFuzzySet* value)
{
  m_pAssociatedFS  = value;
  m_pAssociatedFMF = 0;
}

void CFuzzyAntecedent::SetpAssociatedFMF (const CFuzzyMembershipFunction* value)
{
  m_pAssociatedFMF = value;
  m_pAssociatedFS  = 0;
}

void CFuzzyAntecedent::SetTnorm (bool value)
{
  m_Tnorm   = value;
  m_Tconorm = false;
}

void CFuzzyAntecedent::SetTconorm (bool value)
{
  m_Tconorm = value;
  m_Tnorm   = false;
}

void CFuzzyAntecedent::FSandFSimplyFS (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right) const
{
  CFuzzySet FS = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFS(&left);
  SubPartition.SetpRightFS(&FS);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteFuzzyElements();

  CFuzzyElement FE;
  CFuzzyElement FEcol;
  CFuzzyElement FErow;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;
  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      right.GetFuzzyElement(r,&FErow);
      FS.GetFuzzyElement(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FE = FEr && FErow;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FEtotal.SetValue(FEcol.GetValue());
    result->AddFuzzyElement(FEtotal);
  }
}

void CFuzzyAntecedent::FSandFMFimplyFS (CFuzzySet* result, const CFuzzySet& left, const CFuzzyMembershipFunction right) const
{
  CFuzzySet FS = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFS(&left);
  SubPartition.SetpRightFS(&FS);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());
  
  CFuzzyElement FE;
  CFuzzyElement FErow;
  CFuzzyElement FEcol;
  CFuzzyElement FEmf;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < SubPartition.GetNumRows(); r++)
    {
      left.GetFuzzyElement(r,&FErow);
      FS.GetFuzzyElement(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FEmf = right.GetVertex(FErow.GetValue());
      FE = FEr && FEmf;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FS.GetFuzzyElement(c,&FE);
    FEtotal.SetValue(FE.GetValue());
    result->AddFuzzyElement(FEtotal);
  }
}

void CFuzzyAntecedent::FMFandFSimplyFS (CFuzzySet* result, const CFuzzyMembershipFunction& left, const CFuzzySet& right) const
{
  CFuzzySet FSright = right;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFMF(&left);
  SubPartition.SetpRightFS(&FSright);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteFuzzyElements();

  CFuzzyElement FE;
  CFuzzyElement FEcol;
  CFuzzyElement FErow;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      right.GetFuzzyElement(r,&FErow);
      FSright.GetFuzzyElement(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FE = FEr && FErow;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FEtotal.SetValue(FEcol.GetValue());
    result->AddFuzzyElement(FEtotal);
  }
}

void CFuzzyAntecedent::FMFandFMFimplyFS (CFuzzySet* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction right) const
{
  CFuzzySet FSright = right;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFMF(&left);
  SubPartition.SetpRightFS(&FSright);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteFuzzyElements();

  CFuzzyElement FE;
  CFuzzyElement FErow;
  CFuzzyElement FEcol;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  CFuzzyMembershipFunction FMFcol;
  CFuzzyMembershipFunction FMFresult;

  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FMFcol.DeleteVertices();
    FMFcol.SetTnorm(left.GetTnorm());
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      right.GetVertex(r,&FErow);
      FSright.GetFuzzyElement(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetValue(FErow.GetValue());
      FMFcol.AddVertex(FEr);
    }
    
    FMFresult = FMFcol && right;
    FEtotal.SetMembership(0);
    
    for (long i = 1; i < FMFresult.GetNumVertices(); i++)
    {
      if (FMFresult.GetVertex(i,&FE))
      {
        FEtotal.SetTconorm(left.GetTconorm());
        FEtotal = FEtotal || FE;
      }
    }
    
    FEtotal.SetValue(FEcol.GetValue());
    result->AddFuzzyElement(FEtotal);
  }
}

void CFuzzyAntecedent::FSandFSimplyFMF (CFuzzyMembershipFunction* result, const CFuzzySet& left, const CFuzzySet& right) const
{
  CFuzzyMembershipFunction FMF = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFS(&left);
  SubPartition.SetpRightFMF(&FMF);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteVertices();

  CFuzzyElement FE;
  CFuzzyElement FEcol;
  CFuzzyElement FErow;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  // Iterate through the domain's set.
  for (long c = 0; c < FMF.GetNumVertices(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      right.GetFuzzyElement(r,&FErow);
      FMF.GetVertex(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FE = FEr &&  FErow;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FEtotal.SetValue(FEcol.GetValue());
    result->AddVertex(FEtotal);
  }
}

void CFuzzyAntecedent::FSandFMFimplyFMF (CFuzzyMembershipFunction* result, const CFuzzySet& left, const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMF = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFS(&left);
  SubPartition.SetpRightFMF(&FMF);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteVertices();

  CFuzzyElement FE;
  CFuzzyElement FErow;
  CFuzzyElement FEcol;
  CFuzzyElement FEmf;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < SubPartition.GetNumRows(); r++)
    {
      left.GetFuzzyElement(r,&FErow);
      FMF.GetVertex(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FEmf = right.GetVertex(FErow.GetValue());
      FE = FEr && FEmf;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FEtotal.SetValue(FEcol.GetValue());
    result->AddVertex(FEtotal);
  }
}

void CFuzzyAntecedent::FMFandFSimplyFMF (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzySet& right) const
{
  CFuzzyMembershipFunction FMFright = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFMF(&left);
  SubPartition.SetpRightFMF(&FMFright);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteVertices();

  CFuzzyElement FE;
  CFuzzyElement FEcol;
  CFuzzyElement FErow;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FEtotal.SetValue(0);
    FEtotal.SetMembership(0.0);
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      right.GetFuzzyElement(r,&FErow);
      FMFright.GetVertex(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetTnorm(left.GetTnorm());
      FE = FEr && FErow;
      FEtotal.SetTconorm(left.GetTconorm());
      FEtotal = FEtotal || FE;
    }
    FEtotal.SetValue(FEcol.GetValue());
    result->AddVertex(FEtotal);
  }
}

void CFuzzyAntecedent::FMFandFMFimplyFMF (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFright = *result;
  CFuzzySubPartition SubPartition;

  // Set up the sub-partition to emulate the fuzzy relation.
  SubPartition.SetpLeftFMF(&left);
  SubPartition.SetpRightFMF(&FMFright);
  SubPartition.SetUseTnorm(true);
  SubPartition.SetTnorm(left.GetTnorm());

  result->DeleteVertices();

  CFuzzyElement FE;
  CFuzzyElement FErow;
  CFuzzyElement FEcol;
  CFuzzyElement FEr;
  CFuzzyElement FEtotal;

  CFuzzyMembershipFunction FMFcol;
  CFuzzyMembershipFunction FMFresult;
  
  // Iterate through the domain's set.
  for (long c = 0; c < SubPartition.GetNumColumns(); c++)
  {
    FMFcol.DeleteVertices();
    FMFcol.SetTnorm(left.GetTnorm());
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      
      right.GetVertex(r,&FErow);
      FMFright.GetVertex(c,&FEcol);
      FEr = SubPartition.GetVertex(FEcol.GetValue(),FErow.GetValue());
      FEr.SetValue(FErow.GetValue());
      FMFcol.AddVertex(FEr);
    }
    
    FMFresult = FMFcol && right;
    FEtotal.SetMembership(0);
    
    for (long i = 0; i < FMFresult.GetNumVertices(); i++)
    {
      if (FMFresult.GetVertex(i,&FE))
      {
        FEtotal.SetTconorm(left.GetTconorm());
        FEtotal = FEtotal || FE; // Does this actually do anything????
      }
    }
    
    FEtotal.SetValue(FEcol.GetValue());
    result->AddVertex(FEtotal);
  }
}

void CFuzzyAntecedent::FEimplyFEandFS (CFuzzyElement* result, const CFuzzyAntecedent& left, const CFuzzySet& right) const
{
  CFuzzyElement FEfirst;
  CFuzzyElement FElast;
  CFuzzyElement FEy;

  // Get X% from FS.
  if (left.GetpAssociatedFS())
  {
    left.GetpAssociatedFS()->GetFuzzyElement(0,&FEfirst);
    left.GetpAssociatedFS()->GetFuzzyElement(left.GetpAssociatedFS()->GetNumFuzzyElements()-1,&FElast);
  }

  // Get X% from FMF.
  if (left.GetpAssociatedFMF())
  {
    left.GetpAssociatedFMF()->GetVertex(0,&FEfirst);
    left.GetpAssociatedFMF()->GetVertex(left.GetpAssociatedFMF()->GetNumVertices()-1,&FElast);
  }

  double Range = (FElast.GetValue() - FEfirst.GetValue());
  if (Range <= 0.0)
  {
    assert (Range > 0.0);
    return;
  }
           
  double Xpercentage = (left.GetpFE()->GetValue() - FEfirst.GetValue()) / Range;
  
  FEfirst.SetValue(0);
  FElast.SetValue(0);

  
  right.GetFuzzyElement(0,&FEfirst);
  right.GetFuzzyElement(right.GetNumFuzzyElements()-1,&FElast);
  long Yrange = FElast.GetValue() - FEfirst.GetValue(),
    Yvalue = long(double(Yrange) * Xpercentage) + FEfirst.GetValue();
  
  // Check for out of range and set the correct value and membership to zero.
  if ((Xpercentage < 0.0) || (1.0 < Xpercentage))
  {
    result->SetValue(Yvalue);
    result->SetMembership(0.0);
    return;
  }
  
  FEy = right.GetFuzzyElement(Yvalue);
  long Yindex = right.FindFuzzyElement(Yvalue);
  
  if (Yindex < 0)
  {
    Yindex = right.FindLowerIndex(Yvalue);
    right.GetFuzzyElement(Yindex,&FEy);
    if (Yindex < 0)
    {
      Yindex = right.FindUpperIndex(Yvalue);
      right.GetFuzzyElement(Yindex,&FEy);
    }
  }
  
  CFuzzyElement FEprevious,
    FEnext;
  
  double BackwardSlope,
    ForwardSlope;
  
  // Calculate backward slope;
  if (Yindex-1 >= 0)
  {
    right.GetFuzzyElement(Yindex-1,&FEprevious);
    BackwardSlope = (FEy.GetMembership() - FEprevious.GetMembership()) / 
      (double(FEy.GetValue() - FEprevious.GetValue())); 
  }
  else
  {
    BackwardSlope = 0.0;
  }
  
  // Calculate forward slope.
  if (Yindex+1 < right.GetNumFuzzyElements())
  {
    right.GetFuzzyElement(Yindex+1,&FEnext);
    ForwardSlope = (FEnext.GetMembership() - FEy.GetMembership()) / 
      (double(FEnext.GetValue() - FEy.GetValue())); 
  }
  else
  {
    ForwardSlope = 0.0;
  }
  
  long Direction = 1;
  
  // While Ymembership > Xmembership search downhill for local minimum.
  while ((FEy.GetMembership() > left.GetpFE()->GetMembership()) && (Direction != 0))
  {
    // Determine direction.
    if ((BackwardSlope > 0.0) && (-BackwardSlope <= ForwardSlope))
    {
      // Going backward approaches target fastest.
      Direction = -1;
    }
    else
    {
      if ((ForwardSlope < 0.0) && (ForwardSlope < -BackwardSlope))
      {
        // Going forward approaches target fastest.
        Direction = 1;
      }
      else
      {
        // We are at a local minimum, there is nowhere to go - done.
        Direction = 0;
      }
    }
    
    // Check backward.
    if (Direction < 0)
    {
      // Go backward.
      if (fabs(left.GetpFE()->GetMembership() - FEprevious.GetMembership()) < fabs(left.GetpFE()->GetMembership() - FEy.GetMembership()))
      {
        Yindex--;
        right.GetFuzzyElement(Yindex,&FEy);
        if (Yindex-1 >= 0)
        {
          right.GetFuzzyElement(Yindex-1,&FEprevious);
          right.GetFuzzyElement(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
      else
      {
        // We're as close as we can get, so we're done.
        Direction = 0;
      }
    }
    
    // Check forward.
    if (0 < Direction)
    {
      // Go backward.
      if (fabs(left.GetpFE()->GetMembership() - FEnext.GetMembership()) < fabs(left.GetpFE()->GetMembership() - FEy.GetMembership()))
      {
        Yindex++;
        right.GetFuzzyElement(Yindex,&FEy);
        if (Yindex+1 < right.GetNumFuzzyElements())
        {
          right.GetFuzzyElement(Yindex-1,&FEprevious);
          right.GetFuzzyElement(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
      else
      {
        // We're as close as we can get, so we're done.
        Direction = 0;
      }
    }
  }// END while ((FEy.GetMembership > GetMembership()) && (Direction != 0))
  
  // While Ymembership < Xmembership search uphill for local maxima.
  while ((FEy.GetMembership() < left.GetpFE()->GetMembership()) && (Direction != 0))
  {
    // Determine direction.
    if ((BackwardSlope < 0.0) && (-BackwardSlope >= ForwardSlope))
    {
      // Going backward approaches target fastest.
      Direction = -1;
    }
    else
    {
      if ((ForwardSlope > 0.0) && (ForwardSlope > -BackwardSlope))
      {
        // Going forward approaches target fastest.
        Direction = 1;
      }
      else
      {
        // We are at a local maximum, there is nowhere to go - done.
        Direction = 0;
      }
    }
    
    // Check backward.
    if (Direction < 0)
    {
      // Go backward.
      if (fabs(left.GetpFE()->GetMembership() - FEprevious.GetMembership()) < fabs(left.GetpFE()->GetMembership() - FEy.GetMembership()))
      {
        Yindex--;
        right.GetFuzzyElement(Yindex,&FEy);
        if (Yindex-1 >= 0)
        {
          right.GetFuzzyElement(Yindex-1,&FEprevious);
          right.GetFuzzyElement(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
      else
      {
        // We're as close as we can get, so we're done.
        Direction = 0;
      }
    }
    
    // Check forward.
    if (0 < Direction)
    {
      // Go backward.
      if (fabs(left.GetpFE()->GetMembership() - FEnext.GetMembership()) < fabs(left.GetpFE()->GetMembership() - FEy.GetMembership()))
      {
        Yindex++;
        right.GetFuzzyElement(Yindex,&FEy);
        if (Yindex+1 < right.GetNumFuzzyElements())
        {
          right.GetFuzzyElement(Yindex-1,&FEprevious);
          right.GetFuzzyElement(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
      else
      {
        // We're as close as we can get, so we're done.
        Direction = 0;
      }
    }
  }// END while ((FEy.GetMembership < GetMembership()) && (Direction != 0))

  result->SetValue(FEy.GetValue());
  result->SetMembership(FEy.GetMembership());
}

void CFuzzyAntecedent::FEimplyFEandFMF (CFuzzyElement* result, const CFuzzyAntecedent& left, const CFuzzyMembershipFunction& right) const
{
  CFuzzyElement FEfirst;
  CFuzzyElement FElast;
  CFuzzyElement FEy;

  // Get X% from FS.
  if (left.GetpAssociatedFS())
  {
    left.GetpAssociatedFS()->GetFuzzyElement(0,&FEfirst);
    left.GetpAssociatedFS()->GetFuzzyElement(left.GetpAssociatedFS()->GetNumFuzzyElements()-1,&FElast);
  }

  // Get X% from FMF.
  if (left.GetpAssociatedFMF())
  {
    left.GetpAssociatedFMF()->GetVertex(0,&FEfirst);
    left.GetpAssociatedFMF()->GetVertex(left.GetpAssociatedFMF()->GetNumVertices()-1,&FElast);
  }

  double Range = (FElast.GetValue() - FEfirst.GetValue());
  if (Range <= 0.0)
  {
    assert (Range > 0.0);
    return;
  }

  double Xpercentage = (left.GetpFE()->GetValue() - FEfirst.GetValue()) / Range;

  right.GetVertex(0,&FEfirst);
  right.GetVertex(right.GetNumVertices()-1,&FElast);
  long Yrange = FElast.GetValue() - FEfirst.GetValue(),
    Yvalue = long(double(Yrange) * Xpercentage) + FEfirst.GetValue();
  
  // Check for out of range and set the correct value and membership to zero.
  if ((Xpercentage < 0.0) || (1.0 < Xpercentage))
  {
    result->SetValue(Yvalue);
    result->SetMembership(0.0);
    return;
  }
  
  long Yindex = right.FindVertex(Yvalue);
  if (Yindex >= 0)
  {
    right.GetVertex(Yindex,&FEy);
  }
  
  double BackwardSlope,
    ForwardSlope;
  
  CFuzzyElement FEprevious,
    FEnext;
  
  if (Yindex < 0)
  {
    Yindex = right.FindLowerIndex(Yvalue);
    right.GetVertex(Yindex,&FEy);
    if (Yindex < 0)
    {
      Yindex = right.FindUpperIndex(Yvalue);
      right.GetVertex(Yindex,&FEy);
    }
  }
  
  // Calculate backward slope;
  if (Yindex-1 >= 0)
  {
    right.GetVertex(Yindex-1,&FEprevious);
    double Denominator = double(FEy.GetValue() - FEprevious.GetValue());
    if (Denominator != 0.0)
    {
      BackwardSlope = (FEy.GetMembership() - FEprevious.GetMembership()) / Denominator;
    }
    else
    {
      BackwardSlope = 0.0;
    }
  }
  else
  {
    BackwardSlope = 0.0;
  }
  
  // Calculate forward slope.
  if (Yindex+1 < right.GetNumVertices())
  {
    right.GetVertex(Yindex+1,&FEnext);
    double Denominator = double(FEnext.GetValue() - FEy.GetValue());
    if (Denominator != 0.0)
    {
      ForwardSlope = (FEnext.GetMembership() - FEy.GetMembership()) / Denominator;
    }
    else
    {
      ForwardSlope = 0.0;
    }
  }
  else
  {
    ForwardSlope = 0.0;
  }
  
  
  
  // Special case, set the value of FEy...
  FEy.SetValue(Yvalue);
  
  long Direction = 1; // Keep track of search direction, {-1,0,1} = {backward, none, forward}
  
  // While Ymembership > Xmembership search downhill for local minimum.
  while ((FEy.GetMembership() > left.GetpFE()->GetMembership()) && (Direction != 0))
  {
    bool DirectionPicked = false;
    
    // Special case of determining direction.
    if ((left.GetpFE()->GetMembership() > FEnext.GetMembership()) && 
      (double(FEy.GetValue() - FEfirst.GetValue())/double(Yrange) < Xpercentage))
    {
      DirectionPicked = true;
      Direction = 1;
    }
    if ((left.GetpFE()->GetMembership() > FEprevious.GetMembership()) && 
      ((double(FEy.GetValue() - FEfirst.GetValue()) / double(Yrange)) > Xpercentage))
    {
      DirectionPicked = true;
      Direction = -1;
    }
    
    // Determine direction normally.
    if (!DirectionPicked)
    {
      if ((BackwardSlope > 0.0) && (-BackwardSlope <= ForwardSlope))
      {
        // Going backward approaches target fastest.
        Direction = -1;
      }
      else
      {
        if ((ForwardSlope < 0.0) && (ForwardSlope < -BackwardSlope))
        {
          // Going forward approaches target fastest.
          Direction = 1;
        }
        else
        {
          // We are at a local minimum, there is nowhere to go - done.
          Direction = 0;
        }
      }
    }
    
    // Check backward.
    if (Direction < 0)
    {
      if (FEprevious.GetMembership() <= left.GetpFE()->GetMembership())
      {
        // Were done.
        Direction = 0;
        
        // Calculate the proper value for y.
        double Percentage = (left.GetpFE()->GetMembership() - FEprevious.GetMembership()) / 
          (FEy.GetMembership() - FEprevious.GetMembership());
        
        long   Value      = FEprevious.GetValue() + long(Percentage * (FEy.GetValue() - FEprevious.GetValue()));
        FEy.SetValue(Value);
        FEy.SetMembership(right.GetValue(Value));
      }
      else
      {
        Yindex--;
        right.GetVertex(Yindex,&FEy);
        if (Yindex > 0)
        {
          right.GetVertex(Yindex-1,&FEprevious);
          right.GetVertex(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
    }
    
    // Check forward.
    if (0 < Direction)
    {
      
      if (FEnext.GetMembership() <= left.GetpFE()->GetMembership())
      {
        // Were done.
        Direction = 0;
        
        // Calculate the proper value for y.
        double Percentage = (left.GetpFE()->GetMembership() - FEnext.GetMembership()) / 
          (FEy.GetMembership() - FEnext.GetMembership());
        
        long   Value      = FEnext.GetValue() - long(Percentage * (FEnext.GetValue() - FEy.GetValue()));
        FEy.SetValue(Value);
        FEy.SetMembership(right.GetValue(Value));
      }
      else
      {
        Yindex++;
        assert(0);
        //          right.GetFuzzyElement(Yindex,&FEy);
        //          if (Yindex+1 < right.GetNumFuzzyElements())
        {
          //            right.GetFuzzyElement(Yindex-1,&FEprevious);
          //            right.GetFuzzyElement(Yindex+1,&FEnext);
        }
        //          else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
    }
  }// END while ((FEy.GetMembership > GetMembership()) && (Direction != 0))
  
  
  // While Ymembership < Xmembership search downhill for local minimum.
  while ((FEy.GetMembership() < left.GetpFE()->GetMembership()) && (Direction != 0))
  {
    bool DirectionPicked = false;
    
    // Special case of determining direction.
    if ((left.GetpFE()->GetMembership() < FEnext.GetMembership()) && 
      (double(FEy.GetValue() - FEfirst.GetValue()) / double(Yrange) < Xpercentage))
    {
      DirectionPicked = true;
      Direction = 1;
    }
    if ((left.GetpFE()->GetMembership() < FEprevious.GetMembership()) && 
      (Xpercentage < double(FEy.GetValue() - FEfirst.GetValue()) / double(Yrange)))
    {
      DirectionPicked = true;
      Direction = -1;
    }
    
    // Determine direction normally.
    if (!DirectionPicked)
    {
      if ((BackwardSlope < 0.0) && (-BackwardSlope >= ForwardSlope))
      {
        // Going backward approaches target fastest.
        Direction = -1;
      }
      else
      {
        if ((!DirectionPicked) && (ForwardSlope > 0.0) && (ForwardSlope > -BackwardSlope))
        {
          // Going forward approaches target fastest.
          Direction = 1;
        }
        else
        {
          // We are at a local maximum, there is nowhere to go - done.
          Direction = 0;
        }
      }
    }
    
    // Check backward.
    if (Direction < 0)
    {
      if (FEprevious.GetMembership() >= left.GetpFE()->GetMembership())
      {
        // Were done.
        Direction = 0;
        
        // Calculate the proper value for y.
        double Percentage = (FEprevious.GetMembership() - left.GetpFE()->GetMembership()) / 
          (FEprevious.GetMembership() - FEy.GetMembership());
        
        long   Value      = FEprevious.GetValue() + long(Percentage * (FEy.GetValue() - FEprevious.GetValue()));
        FEy.SetValue(Value);
        FEy.SetMembership(right.GetValue(Value));
      }
      else
      {
        Yindex--;
        right.GetVertex(Yindex,&FEy);
        if (Yindex > 0)
        {
          right.GetVertex(Yindex-1,&FEprevious);
          right.GetVertex(Yindex+1,&FEnext);
        }
        else
        {
          // can't go any farther so were done.
          Direction = 0;
        }
      }
    }
    
    // Check forward.
    if (0 < Direction)
    {
      if (FEnext.GetMembership() >= left.GetpFE()->GetMembership())
      {
        // Were done.
        Direction = 0;
        
        // Calculate the proper value for y.
        double Percentage = (left.GetpFE()->GetMembership() - FEy.GetMembership()) / 
          (FEnext.GetMembership() - FEy.GetMembership());
        
        long   Value      = FEy.GetValue() + long(Percentage * (FEnext.GetValue() - FEy.GetValue()));
        FEy.SetValue(Value);
        FEy.SetMembership(right.GetValue(Value));
      }
      else
      {
        Yindex++;
        assert(0);
        // can't go any farther so were done.
        Direction = 0;
      }
    }
  }// END while ((FEy.GetMembership < GetMembership()) && (Direction != 0))
  
  result->SetValue(FEy.GetValue());
  result->SetMembership(FEy.GetMembership());
}

const CFuzzyMembershipFunction CFuzzyAntecedent::Recurse (const CFuzzyAntecedent& antecedent, const CFuzzyMembershipFunction& result) const
{
  if (antecedent.m_pLeft && antecedent.m_pRight)
  {
    assert(antecedent.GetpAssociatedFMF() == 0);
    assert(antecedent.GetpAssociatedFS() == 0);
    assert(antecedent.GetpFMF() == 0);
    assert(antecedent.GetpFS() == 0);
    assert(antecedent.GetpFE() == 0);

    CFuzzyMembershipFunction Left;
    CFuzzyMembershipFunction Right;
    Left = Recurse(*antecedent.m_pLeft, result);
    Right = Recurse(*antecedent.m_pRight, result);

    assert(antecedent.GetTconorm() || antecedent.GetTnorm());
    if (antecedent.GetTconorm())
    {
      return Left || Right;
    }
    if (antecedent.GetTnorm())
    {
      return Left && Right;
    }
  }
  else
  {
    assert(antecedent.GetpFE() == 0);
    assert(antecedent.m_pLeft == 0);
    assert(antecedent.m_pRight == 0);
    assert(antecedent.GetpAssociatedFMF() || antecedent.GetpAssociatedFS());
    assert(antecedent.GetpFMF() || antecedent.GetpFS());

    CFuzzyMembershipFunction FMF = result;

    if (antecedent.GetpFS())
    {
      if (antecedent.GetpAssociatedFMF())
      {
        FSandFMFimplyFMF(&FMF, *antecedent.GetpFS(), *antecedent.GetpAssociatedFMF());
        return FMF;
      }
      if (antecedent.GetpAssociatedFS())
      {
        FSandFSimplyFMF(&FMF, *antecedent.GetpFS(), *antecedent.GetpAssociatedFS());
        return FMF;
      }
    }

    if (antecedent.GetpFMF())
    {
      if (antecedent.GetpAssociatedFMF())
      {
        FMFandFMFimplyFMF(&FMF, *antecedent.GetpFMF(), *antecedent.GetpAssociatedFMF());
        return FMF;
      }
      if (antecedent.GetpAssociatedFS())
      {
        FMFandFSimplyFMF(&FMF, *antecedent.GetpFMF(), *antecedent.GetpAssociatedFS());
        return FMF;
      }
    }
  }

  assert(0);
  CFuzzyMembershipFunction Dummy;
  return Dummy;
}

const CFuzzySet CFuzzyAntecedent::Recurse (const CFuzzyAntecedent& antecedent, const CFuzzySet& result) const
{
  if (antecedent.m_pLeft && antecedent.m_pRight)
  {
    assert(antecedent.GetpAssociatedFMF() == 0);
    assert(antecedent.GetpAssociatedFS() == 0);
    assert(antecedent.GetpFMF() == 0);
    assert(antecedent.GetpFS() == 0);
    assert(antecedent.GetpFE() == 0);

    CFuzzySet Left;
    CFuzzySet Right;
    Left = Recurse(*antecedent.m_pLeft, result);
    Right = Recurse(*antecedent.m_pRight, result);

    assert(antecedent.GetTconorm() || antecedent.GetTnorm());
    if (antecedent.GetTconorm())
    {
      return Left || Right;
    }
    if (antecedent.GetTnorm())
    {
      return Left && Right;
    }
  }
  else
  {
    assert(antecedent.m_pLeft == 0);
    assert(antecedent.m_pRight == 0);
    assert(antecedent.GetpAssociatedFMF() || antecedent.GetpAssociatedFS());
    assert(antecedent.GetpFMF() || antecedent.GetpFS());
    assert(antecedent.GetpFE() == 0);

    CFuzzySet FS = result;

    if (antecedent.GetpFS())
    {
      if (antecedent.GetpAssociatedFMF())
      {
        FSandFMFimplyFS(&FS, *antecedent.GetpFS(), *antecedent.GetpAssociatedFMF());
        return FS;
      }
      if (antecedent.GetpAssociatedFS())
      {
        FSandFSimplyFS(&FS, *antecedent.GetpFS(), *antecedent.GetpAssociatedFS());
        return FS;
      }
    }

    if (antecedent.GetpFMF())
    {
      if (antecedent.GetpAssociatedFMF())
      {
        FMFandFMFimplyFS(&FS, *antecedent.GetpFMF(), *antecedent.GetpAssociatedFMF());
        return FS;
      }
      if (antecedent.GetpAssociatedFS())
      {
        FMFandFSimplyFS(&FS, *antecedent.GetpFMF(), *antecedent.GetpAssociatedFS());
        return FS;
      }
    }
  }

  assert(0);
  CFuzzySet Dummy;
  return Dummy;
}

const CFuzzyElement CFuzzyAntecedent::Recurse (const CFuzzyAntecedent& left, const CFuzzyAntecedent& right) const
{
  if (left.m_pLeft && left.m_pRight)
  {
    assert(left.GetpAssociatedFMF() == 0);
    assert(left.GetpAssociatedFS() == 0);
    assert(left.GetpFMF() == 0);
    assert(left.GetpFS() == 0);
    assert(left.GetpFE() == 0);

    CFuzzyElement Left;
    CFuzzyElement Right;
    Left = Recurse(*left.m_pLeft, right);
    Right = Recurse(*left.m_pRight, right);

    assert(left.GetTconorm() || left.GetTnorm());
    if (left.GetTconorm())
    {
      return Left || Right;
    }
    if (left.GetTnorm())
    {
      return Left && Right;
    }
  }
  else
  {
    assert(left.m_pLeft == 0);
    assert(left.m_pRight == 0);
    assert(left.GetpAssociatedFMF() || left.GetpAssociatedFS());
    assert(left.GetpFMF() == 0);
    assert(left.GetpFS() == 0);
    assert(right.GetpFE());

    CFuzzyElement FE = *right.GetpFE();

    if (left.GetpFE())
    {
      if (right.GetpAssociatedFS())
      {
        FEimplyFEandFS(&FE, left, *right.GetpAssociatedFS());
      }
      if (right.GetpAssociatedFMF())
      {
        FEimplyFEandFMF(&FE, left, *right.GetpAssociatedFMF());
      }
      return FE;
    }
  }

  assert(0);
  CFuzzyElement Dummy;
  return Dummy;
}

