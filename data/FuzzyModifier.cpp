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

#include "FuzzyModifier.h"
#include <cassert>
#include <cmath>

CFuzzyModifier::CFuzzyModifier()
{

}

CFuzzyModifier::CFuzzyModifier(const CFuzzyModifier &right)
{
  assert(0);
}


CFuzzyModifier::~CFuzzyModifier()
{

}


CFuzzyModifier & CFuzzyModifier::operator=(const CFuzzyModifier &right)
{
  assert(0);
  return *this;
}


int CFuzzyModifier::operator==(const CFuzzyModifier &right) const
{
  assert(0);
  return false;
}

int CFuzzyModifier::operator!=(const CFuzzyModifier &right) const
{
  return!(*this == right);
}

const CFuzzyElement& CFuzzyModifier::Very (const CFuzzyElement& right)
{
  static CFuzzyElement FEreturn;
  FEreturn = right;
  FEreturn.SetMembership(right.GetMembership() * right.GetMembership());
  return FEreturn;
}

const CFuzzySet& CFuzzyModifier::Very (const CFuzzySet& right)
{
  CFuzzyElement FE;
  static CFuzzySet FSreturn;
  FSreturn.DeleteFuzzyElements();

  for (long i=0; i < right.GetNumFuzzyElements(); i++)
  {
    right.GetFuzzyElement(i,&FE);
    FE.SetMembership(FE.GetMembership() * FE.GetMembership());
    FSreturn.AddFuzzyElement(FE);
  }

  return FSreturn;
}

const CFuzzyMembershipFunction& CFuzzyModifier::Very (const CFuzzyMembershipFunction& right, long granularity)
{
  CFuzzyElement FE,FEnext,FEadd;
  static CFuzzyMembershipFunction FMFreturn;
  FMFreturn.DeleteVertices();

  if (granularity < 1)
  {
    assert(granularity < 1);
    return FMFreturn;
  }

  for (long i=0; i < right.GetNumVertices(); i++)
  {
    if ((right.GetVertex(i,&FE)) && (right.GetVertex(i+1,&FEnext)))
    {
      FMFreturn.AddVertex(FE);

      // Get spacing and ensure spacing of at least 1.
      long VertexSpacing = (FEnext.GetValue() - FE.GetValue()) / granularity;
      if (VertexSpacing < 1)
      {
        VertexSpacing = 1;
      }

      // Add new points.
      long   Value = FE.GetValue();
      for (long g=1; g < granularity; g++)
      {
        Value += VertexSpacing;
        double Membership = right.GetValue(Value);
        Membership *= Membership;

        FEadd = FE;
        FEadd.SetValue(Value);
        FEadd.SetMembership(Membership);

        FMFreturn.AddVertex(FEadd);
      }
      FMFreturn.AddVertex(FEnext);
    }
  }

  return FMFreturn;
}

const CFuzzyElement& CFuzzyModifier::MoreOrLess (const CFuzzyElement& right)
{
  static CFuzzyElement FEreturn;
  FEreturn = right;
  FEreturn.SetMembership(sqrt(right.GetMembership()));
  return FEreturn;
}

const CFuzzySet& CFuzzyModifier::MoreOrLess (const CFuzzySet& right)
{
  CFuzzyElement FE;
  static CFuzzySet FSreturn;
  FSreturn.DeleteFuzzyElements();

  for (long i=0; i < right.GetNumFuzzyElements(); i++)
  {
    right.GetFuzzyElement(i,&FE);
    FE.SetMembership(sqrt(FE.GetMembership()));
    FSreturn.AddFuzzyElement(FE);
  }

  return FSreturn;
}

const CFuzzyMembershipFunction& CFuzzyModifier::MoreOrLess (const CFuzzyMembershipFunction& right, long granularity)
{
  CFuzzyElement FE,FEnext,FEadd;
  static CFuzzyMembershipFunction FMFreturn;
  FMFreturn.DeleteVertices();

  if (granularity < 1)
  {
    assert(granularity < 1);
    return FMFreturn;
  }

  for (long i=0; i < right.GetNumVertices(); i++)
  {
    if ((right.GetVertex(i,&FE)) && (right.GetVertex(i+1,&FEnext)))
    {
      FMFreturn.AddVertex(FE);

      // Get spacing and ensure spacing of at least 1.
      long VertexSpacing = (FEnext.GetValue() - FE.GetValue()) / granularity;
      if (VertexSpacing < 1)
      {
        VertexSpacing = 1;
      }

      // Add new points.
      long   Value = FE.GetValue();
      for (long g=1; g < granularity; g++)
      {
        Value += VertexSpacing;
        double Membership = sqrt(right.GetValue(Value));

        FEadd = FE;
        FEadd.SetValue(Value);
        FEadd.SetMembership(Membership);

        FMFreturn.AddVertex(FEadd);
      }
      FMFreturn.AddVertex(FEnext);
    }
  }

  return FMFreturn;
}

