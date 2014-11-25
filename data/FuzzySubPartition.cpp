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

#include "FuzzyElement.h"
#include "FuzzySet.h"
#include "FuzzyMembershipFunction.h"
#include "FuzzySubPartition.h"
#include <cassert>

CFuzzySubPartition::CFuzzySubPartition()
  : m_pLeftFS(0), m_pRightFS(0), m_pLeftFMF(0), m_pRightFMF(0), m_Tnorm(Min), m_Tconorm(Max), m_UseTnorm(true)
{

}

CFuzzySubPartition::CFuzzySubPartition(const CFuzzySubPartition &right)
  : m_pLeftFS(0), m_pRightFS(0), m_pLeftFMF(0), m_pRightFMF(0), m_Tnorm(Min), m_Tconorm(Max), m_UseTnorm(true)
{
  assert(0);
}


CFuzzySubPartition::~CFuzzySubPartition()
{

}


CFuzzySubPartition & CFuzzySubPartition::operator=(const CFuzzySubPartition &right)
{
  assert(0);
  return *this;
}


int CFuzzySubPartition::operator==(const CFuzzySubPartition &right) const
{
  assert(0);
  return false;
}

int CFuzzySubPartition::operator!=(const CFuzzySubPartition &right) const
{
  return ! (*this == right);
}

double CFuzzySubPartition::GetValue (long x, long y) const
{
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pFE;

  // Get left side.
  if (m_pLeftFS)
  {
    pFE = m_pLeftFS->GetpFuzzyElement(y);
    if (pFE)
    {
      FEleft.SetMembership(pFE->GetMembership());
    }
  }

  if (m_pLeftFMF)
  {
    FEleft.SetMembership(m_pLeftFMF->GetValue(y));
  }

  // Get right side.
  if (m_pRightFS)
  {
    pFE = m_pRightFS->GetpFuzzyElement(x);
    if (pFE)
    {
      FEright.SetMembership(pFE->GetMembership());
    }
  }

  if (m_pRightFMF)
  {
    FEright.SetMembership(m_pRightFMF->GetValue(x));
  }

  // Check for which operation is being and calculate result.
  if (m_UseTnorm)
  {
    FEleft.SetTnorm(m_Tnorm);
    FE = FEleft && FEright;
  }
  else
  {
    FEleft.SetTconorm(m_Tconorm);
    FE = FEleft || FEright;
  }

  return FE.GetMembership();
}

const CFuzzyElement& CFuzzySubPartition::GetVertex (long x, long y) const
{
  static CFuzzyElement FE;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;

  // Get y.
  if (m_pLeftFS)
  {
    CFuzzyElement *pFound = m_pLeftFS->GetpFuzzyElement(y);
    if (pFound)
    {
      FEleft = *pFound;
    }
    else
    {
      FEleft.SetValue(y);
      FEleft.SetMembership(0.0);
    }
  }

  if (m_pLeftFMF)
  {
    FEleft = m_pLeftFMF->GetVertex(y);
  }

  // Get x.
  if (m_pRightFS)
  {
    CFuzzyElement *pFound = m_pRightFS->GetpFuzzyElement(x);
    if (pFound)
    {
      FEright = *pFound;
    }
    else
    {
      FEright.SetValue(x);
      FEright.SetMembership(0.0);
    }
  }

  if (m_pRightFMF)
  {
    FEright = m_pRightFMF->GetVertex(x);
  }

  // Check for which operation is being and calculate result.
  if (m_UseTnorm)
  {
    FEleft.SetTnorm(m_Tnorm);
    FE = FEleft && FEright;
  }
  else
  {
    FEleft.SetTconorm(m_Tconorm);
    FE = FEleft || FEright;
  }

  return FE;

}

long CFuzzySubPartition::GetNumColumns ()
{
  if (m_pRightFS)
  {
    return m_pRightFS->GetNumFuzzyElements();
  }

  if (m_pRightFMF)
  {
    return m_pRightFMF->GetNumVertices();
  }

  return 0;
}

long CFuzzySubPartition::GetNumRows ()
{
  if (m_pLeftFS)
  {
    return m_pLeftFS->GetNumFuzzyElements();
  }

  if (m_pLeftFMF)
  {
    return m_pLeftFMF->GetNumVertices();
  }

  return 0;
}

void CFuzzySubPartition::SetpLeftFS (const CFuzzySet* value)
{
  m_pLeftFS = const_cast<CFuzzySet*>(value);
  m_pLeftFMF = 0;
}

void CFuzzySubPartition::SetpRightFS (CFuzzySet* value)
{
  m_pRightFS = value;
  m_pRightFMF = 0;
}

void CFuzzySubPartition::SetpLeftFMF (const CFuzzyMembershipFunction* value)
{
  m_pLeftFMF = const_cast<CFuzzyMembershipFunction*>(value);
  m_pLeftFS = 0;
}

void CFuzzySubPartition::SetpRightFMF (CFuzzyMembershipFunction* value)
{
  m_pRightFMF = value;
  m_pRightFS = 0;
}

