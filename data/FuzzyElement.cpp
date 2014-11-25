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

#include "FuzzySet.h"
#include "FuzzyMembershipFunction.h"
#include "FuzzyOperator.h"
#include "FuzzyRule.h"
#include "FuzzyElement.h"
#include <cassert>
#include <cmath>

CFuzzyElement::CFuzzyElement()
  : m_Tnorm(Min), m_Tconorm(Max), m_Value(-1.0), m_Membership(0)
{

}

CFuzzyElement::CFuzzyElement(const CFuzzyElement &right)
  : m_Tnorm(Min), m_Tconorm(Max), m_Value(-1.0), m_Membership(0)
{
  *this = right;
}

CFuzzyElement::CFuzzyElement (const double membership, CFuzzySet* powner, long value)
  : m_Tnorm(Min), m_Tconorm(Max), m_Value(-1.0), m_Membership(0)
{
  SetValue(value);
  SetMembership(membership);
  m_pOwner = powner;
}

CFuzzyElement::~CFuzzyElement()
{

}

CFuzzyElement & CFuzzyElement::operator=(const CFuzzyElement &right)
{
  SetValue(right.GetValue());
  SetMembership(right.GetMembership());
  SetTconorm(right.GetTconorm());
  SetTnorm(right.GetTnorm());

  return *this;
}


int CFuzzyElement::operator==(const CFuzzyElement &right) const
{
  if (GetValue() != right.GetValue())
  {
    return false;
  }

  if (GetMembership() != right.GetMembership())
  {
    return false;
  }

  return true;
}

int CFuzzyElement::operator!=(const CFuzzyElement &right) const
{
  return !(*this == right);
}

const CFuzzySet CFuzzyElement::operator & (const CFuzzyElement& right) const
{
   CFuzzySet FSreturn;
   assert(0);
   return FSreturn;
}

const CFuzzyElement CFuzzyElement::operator + (const CFuzzyElement& right) const
{
	CFuzzyElement FEreturn;

	FEreturn = *this && right;  // do t-conorm.
	FEreturn.SetValue(GetValue() + right.GetValue()); // add values.

	return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator - (const CFuzzyElement& right) const
{
	CFuzzyElement FEreturn;

	FEreturn = *this && right;  // do t-conorm.
	FEreturn.SetValue(GetValue() - right.GetValue()); // subtract values.

	return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator * (const CFuzzyElement& right) const
{
  CFuzzyElement FEreturn;

  CFuzzyOperator::Minimum(&FEreturn, *this, right);  // do Minimum.
	FEreturn.SetValue(GetValue() * right.GetValue()); // multiply values.

	return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator / (const CFuzzyElement& right) const
{
  CFuzzyElement FEreturn;

  CFuzzyOperator::Minimum(&FEreturn, *this, right);  // do Minimum.
	FEreturn.SetValue(GetValue() / right.GetValue()); // divide values.

	return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator ! ()
{
  CFuzzyElement FEreturn;

  FEreturn = *this;
  FEreturn.SetMembership(1.0 - FEreturn.GetMembership());

  return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator && (const CFuzzyElement& right) const
{
  CFuzzyElement FEreturn;

  switch(m_Tnorm)
  {
    case Min : CFuzzyOperator::Minimum(&FEreturn, *this, right);
      break;
    case DrasticProduct : CFuzzyOperator::DrasticProduct(&FEreturn, *this, right);
      break;
    case BoundedDifference : CFuzzyOperator::BoundedDifference(&FEreturn, *this, right);
      break;
    case EinsteinProduct : CFuzzyOperator::EinsteinProduct(&FEreturn, *this, right);
      break;
    case AlgabraicProduct : CFuzzyOperator::AlgabraicProduct(&FEreturn, *this, right);
      break;
    case HamacherProduct : CFuzzyOperator::HamacherProduct(&FEreturn, *this, right);
      break;
  }

  return FEreturn;
}

const CFuzzyElement CFuzzyElement::operator || (const CFuzzyElement& right) const
{
  CFuzzyElement FEreturn;

   switch(m_Tconorm)
   {
      case Max : CFuzzyOperator::Maximum(&FEreturn, *this, right);
         break;
      case DrasticSum : CFuzzyOperator::DrasticSum(&FEreturn, *this, right);
         break;
      case BoundedSum : CFuzzyOperator::BoundedSum(&FEreturn, *this, right);
         break;
      case EinsteinSum : CFuzzyOperator::EinsteinSum(&FEreturn, *this, right);
         break;
      case AlgabraicSum : CFuzzyOperator::AlgabraicSum(&FEreturn, *this, right);
         break;
      case HamacherSum : CFuzzyOperator::HamacherSum(&FEreturn, *this, right);
         break;
   }
   
   return FEreturn;
}

const CFuzzyAntecedent CFuzzyElement::operator () (const CFuzzySet& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFE(this);
  Result.SetpAssociatedFS(&right);
  return Result;
}

const CFuzzyAntecedent CFuzzyElement::operator () (const CFuzzyMembershipFunction& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFE(this);
  Result.SetpAssociatedFMF(&right);
  return Result;
}

const CFuzzyAntecedent CFuzzyElement::Is (const CFuzzySet& FS)
{
  CFuzzyAntecedent Result;
  Result.SetpFE(this);
  Result.SetpAssociatedFS(&FS);
  return Result;
}

const CFuzzyAntecedent CFuzzyElement::Is (const CFuzzyMembershipFunction& FMF)
{
  CFuzzyAntecedent Result;
  Result.SetpFE(this);
  Result.SetpAssociatedFMF(&FMF);
  return Result;
}

void CFuzzyElement::SetMembership (double value)
{
  assert(0.0 <= value);
  assert(value <= 1.0);
  m_Membership = value;
  m_Membership = std::_MAX(m_Membership, 0.0);
  m_Membership = std::_MIN(m_Membership, 1.0);
}
