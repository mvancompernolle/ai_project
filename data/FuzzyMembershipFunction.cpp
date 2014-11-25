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
#include "FuzzyOperator.h"
#include "FuzzySubPartition.h"
#include "FuzzyRule.h"
#include "MathHelper.h"
#include "FuzzyMembershipFunction.h"
#include <cassert>
#include <fstream>
#include <float.h>

CFuzzyMembershipFunction::CFuzzyMembershipFunction()
  : m_Tnorm(Min), m_Tconorm(Max)
{
  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}

CFuzzyMembershipFunction::CFuzzyMembershipFunction(const CFuzzyMembershipFunction &right)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  CFuzzyElement FE;
  
  m_Name    = right.GetName();
  m_Tnorm   = right.GetTnorm();
  m_Tconorm = right.GetTconorm();

  for (long r = 0; r < right.GetNumVertices(); r++)
  {
    if (right.GetVertex(r,&FE))
    {
      AddVertex(FE);
    }
  }

  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}

CFuzzyMembershipFunction::CFuzzyMembershipFunction (long x1, long x2, long x3)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  assert((x1 < x2) && (x2 < x3));

  InitTriangle(x1,x2,x3);

  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}

CFuzzyMembershipFunction::CFuzzyMembershipFunction (const CFuzzyElement& v1, const CFuzzyElement& v2, const CFuzzyElement& v3)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  assert((v1.GetValue() < v2.GetValue()) && (v2.GetValue() < v3.GetValue()));
  assert((v2.GetMembership() > v1.GetMembership()) && (v2.GetMembership() > v3.GetMembership()));

  InitTriangle(v1,v2,v3);

  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}

CFuzzyMembershipFunction::CFuzzyMembershipFunction (long x1, long x2, long x3, long x4)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  assert((x1 < x2) && (x2 < x3) && (x3 < x4));

  InitTrapezoid(x1,x2,x3,x4);

  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}

CFuzzyMembershipFunction::CFuzzyMembershipFunction (const CFuzzyElement& v1, const CFuzzyElement& v2, const CFuzzyElement &v3, const CFuzzyElement& v4)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  assert((v1.GetValue() < v2.GetValue()) && (v2.GetValue() < v3.GetValue()) && (v3.GetValue() < v4.GetValue()));
  assert((v2.GetMembership() > v1.GetMembership()) && (v2.GetMembership() > v4.GetMembership()));
  assert((v3.GetMembership() > v1.GetMembership()) && (v3.GetMembership() > v4.GetMembership()));

  InitTrapezoid(v1,v2,v3,v4);

  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
}


CFuzzyMembershipFunction::~CFuzzyMembershipFunction()
{
  DeleteVertices();
}


CFuzzyMembershipFunction & CFuzzyMembershipFunction::operator=(const CFuzzyMembershipFunction &right)
{
  CFuzzyElement FE;

  DeleteVertices();
  for (long v=0; v < right.GetNumVertices(); v++)
  {
    if (!right.GetVertex(v,&FE))
    {
      return *this;
    }

    AddVertex(FE);
  }

  return *this;
}


int CFuzzyMembershipFunction::operator==(const CFuzzyMembershipFunction &right) const
{
   if (m_Name == right.GetName())
   {
     return true;
   }

   return false;
}

int CFuzzyMembershipFunction::operator!=(const CFuzzyMembershipFunction &right) const
{
   return !(*this == right);
}

bool CFuzzyMembershipFunction::InitTriangle (long x1, long x2, long x3)
{
  if ((x1 < x2) && (x2 < x3))
  {
    CFuzzyElement FE;
    DeleteVertices();

    // Left vertex.
    FE.SetValue(x1);
    FE.SetMembership(0.0);
    AddVertex(FE);

    // Peak vertex.
    FE.SetValue(x2);
    FE.SetMembership(1.0);
    AddVertex(FE);

    // Right vertex.
    FE.SetValue(x3);
    FE.SetMembership(0.0);
    AddVertex(FE);

    return true;
  }

  return false;
}

bool CFuzzyMembershipFunction::InitTriangle (const CFuzzyElement& v1, const CFuzzyElement& v2, const CFuzzyElement& v3)
{
  if ((v1.GetValue() < v2.GetValue()) && (v2.GetValue() < v3.GetValue()) && 
    (v2.GetMembership() > v1.GetMembership()) && (v2.GetMembership() > v3.GetMembership()))
  {
    DeleteVertices();

    // Left vertex.
    AddVertex(v1);

    // Peak vertex.
    AddVertex(v2);

    // Right vertex.
    AddVertex(v3);

    return true;
  }

  return false;
}

bool CFuzzyMembershipFunction::InitTrapezoid (long x1, long x2, long x3, long x4)
{
  if ((x1 < x2) && (x2 < x3) && (x3 < x4))
  {
    CFuzzyElement FE;
    DeleteVertices();

    // Leftmost vertex.
    FE.SetValue(x1);
    FE.SetMembership(0.0);
    AddVertex(FE);

    // Left peak vertex.
    FE.SetValue(x2);
    FE.SetMembership(1.0);
    AddVertex(FE);

    // Right peak vertex.
    FE.SetValue(x3);
    FE.SetMembership(1.0);
    AddVertex(FE);

    // Rightmost vertex.
    FE.SetValue(x4);
    FE.SetMembership(0.0);
    AddVertex(FE);

    return true;
  }

  return false;
}

bool CFuzzyMembershipFunction::InitTrapezoid (const CFuzzyElement& v1, const CFuzzyElement& v2, const CFuzzyElement& v3, const CFuzzyElement v4)
{
  if ((v1.GetValue() < v2.GetValue()) && (v2.GetValue() < v3.GetValue()) && (v3.GetValue() < v4.GetValue()) &&
      (v2.GetMembership() > v1.GetMembership()) && (v2.GetMembership() > v4.GetMembership()) && 
      (v3.GetMembership() > v1.GetMembership()) && (v3.GetMembership() > v4.GetMembership()))
  {
    DeleteVertices();

    // Leftmost vertex.
    AddVertex(v1);
    
    // Left peak vertex.
    AddVertex(v2);

    // Right peak vertex.
    AddVertex(v3);

    // Rightmost vertex.
    AddVertex(v4);

    return true;
  }

  return false;
}

bool CFuzzyMembershipFunction::AddVertex (const CFuzzyElement& vertex)
{
  m_LastIndex = 0;
  m_LastIterator = m_Verticies.begin();
  
  CFuzzyElement* pVertex = new CFuzzyElement(vertex);
  if (m_Verticies.insert(T_CFuzzyElement_map::value_type(pVertex->GetValue(),pVertex)).second)
  {
    return true;
  }

  return false;
}

bool CFuzzyMembershipFunction::DeleteVertex (long key)
{
  if (m_Verticies.find(key) != m_Verticies.end())
  {
    m_LastIndex = 0;
    m_LastIterator = m_Verticies.begin();

    // The variable does exist so we can delete it.
    m_Verticies.erase(key);
    return true;
  }
  
  return false;
}

bool CFuzzyMembershipFunction::DeleteVertices ()
{
  if (GetNumVertices() != 0)
  {
    m_LastIndex = 0;
    m_LastIterator = m_Verticies.begin();

    m_Verticies.clear();
    return true;
  }
  
  return false;
}

long CFuzzyMembershipFunction::FindVertex (long key) const
{
  for (long i=0; i < GetNumVertices(); i++)
  {
    CFuzzyElement FE;
    GetVertex(i,&FE);
    if (FE.GetValue() == key)
    {
      return i;
    }
  }

  return -1;
}

long CFuzzyMembershipFunction::FindLowerIndex (long key) const
{
/*
  long Index = FindVertex(key);
  if (Index < 0)
  {
    for (long i=GetNumVertices()-1; i >= 0 ; i--)
    {
      CFuzzyElement FE;
      GetVertex(i,&FE);
      if (FE.GetValue() < key)
      {
        // We found the lower value.
        return i;
      }
    }
    
    // There is no lower index value, return -1;
    return -1;
  }
  else
  {
    return Index-1;
  }
*/
  T_CFuzzyElement_map::const_iterator ThisVi = m_Verticies.lower_bound(key);

  if (ThisVi != m_Verticies.end())
  {
    long i = 0;
    while (ThisVi != m_Verticies.begin())
    {
      ThisVi--;
      i++;
    };
    i--;
    assert(i >= -1);

    return i;
  }

  return -1;
}

long CFuzzyMembershipFunction::FindUpperIndex (long key) const
{
  T_CFuzzyElement_map::const_iterator ThisVi = m_Verticies.upper_bound(key);

  if (ThisVi != m_Verticies.end())
  {
    long i = 0;
    while (ThisVi != m_Verticies.begin())
    {
      ThisVi--;
      i++;
    };

    return i;
  }

  return -1;
}

bool CFuzzyMembershipFunction::Intersection (const CFuzzyElement& a1, const CFuzzyElement& a2, const CFuzzyElement& b1, const CFuzzyElement& b2, CFuzzyElement* pPoint)
{
  if (!pPoint)
  {
    return false;
  }

  double X1 = double(a1.GetValue());
  double X2 = double(a2.GetValue());
  double X3 = double(b1.GetValue());
  double X4 = double(b2.GetValue());
  double Y1 = a1.GetMembership();
  double Y2 = a2.GetMembership();
  double Y3 = b1.GetMembership();
  double Y4 = b2.GetMembership();

  double Numerator;
  double Denominator;
  double Y;
  double X;
  double M;
  double B;
  double Ua;

  Denominator = ((Y4 - Y3) * (X2 - X1)) - ((X4 - X3) * (Y2 - Y1));
  if (fabs(Denominator) <= DBL_EPSILON)
  {
    // Since denominator is zero, we either have parallel or coincident lines.
    return false;
  }

  Numerator   = ((X4 - X3) * (Y1 - Y3)) - ((Y4 - Y3) * (X1 - X3));
  Ua = Numerator / Denominator;
  X = X1 + (Ua * (X2 - X1));

  // Check line segment A
  if ((long(X) < a1.GetValue()) || (a2.GetValue() < long(X)))
  {
    // The line intersection is not within segment A, no segment intersection.
    return false;
  }

  // Check line segment B
  if ((long(X) < b1.GetValue()) || (b2.GetValue() < long(X)))
  {
    // The line intersection is not within segment B, no segment intersection.
    return false;
  }

  // Successfull segment intersection, calculate the Y value and return;
  M = ((Y2 - Y1) / (X2 - X1));
  B = Y1 - (M * X1);

  // We must properly handle roundoff to FuzzyElement's value and still have a straight segment line.
  X = RoundNearest(X);
  Y = (M * X) + B;
  pPoint->SetValue(long(RoundNearest(X)));
  pPoint->SetMembership(Y);
  return true;        
}

long CFuzzyMembershipFunction::GetNumVertices () const
{
   return m_Verticies.size();
}

CFuzzyElement* CFuzzyMembershipFunction::GetpVertex (long key)
{
   T_CFuzzyElement_map::const_iterator ThisVi;
   ThisVi = m_Verticies.find(key);

   if (ThisVi == m_Verticies.end())
   {
      return 0;
   }

   return (*ThisVi).second;
}

bool CFuzzyMembershipFunction::GetVertex (long index, CFuzzyElement* pVertex) const
{
  assert(index >= 0);
  assert(index < GetNumVertices());

  if ((index < 0) || (GetNumVertices() <= index))
  {
    return false;
  }

  T_CFuzzyElement_map::const_iterator ThisVi = m_LastIterator;
  long NumSteps = index - m_LastIndex;
  
  if (NumSteps > 0)
  {
    // Iterate number of times to disired index.
    for (long i=0; i < NumSteps; i++)
    {
      ThisVi++;
    }

    // Make sure we have a valid iterator.
    if (ThisVi == m_Verticies.end())
    {
      *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = m_Verticies.begin();
      *(const_cast<long*>(&m_LastIndex)) = 0;
      pVertex->SetMembership(0.0);
      return false;
    }

    // We have a valid iterator, so set last and return CFuzzyElement.
    *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = ThisVi;
    *(const_cast<long*>(&m_LastIndex)) = index;
    *pVertex = *((*ThisVi).second);
    return true;
  }
  else
  {
    NumSteps = labs(NumSteps);

    // Iterate number of times to disired index.
    for (long i=0; i < NumSteps; i++)
    {
      ThisVi--;
    }

    // Make sure we have a valid iterator.
    if (ThisVi == m_Verticies.end())
    {
      *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = m_Verticies.begin();
      *(const_cast<long*>(&m_LastIndex)) = 0;
      pVertex->SetMembership(0.0);
      assert(m_Verticies.begin() != m_Verticies.end());
      return false;
    }

    // We have a valid iterator, so set last and return CFuzzyElement.
    *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = ThisVi;
    *(const_cast<long*>(&m_LastIndex)) = index;
    *pVertex = *((*ThisVi).second);
    return true;
  }
}

const CFuzzyElement CFuzzyMembershipFunction::GetVertex (long key) const
{
  CFuzzyElement FE;
  CFuzzyElement FElast;
  
  // Init last fuzzy variable index.
  GetVertex(0,&FElast);

  // Go through the verticies and campare against value.
  for (int v=0; v < GetNumVertices(); v++)
  {
    if (GetVertex(v,&FE))
    {
      if ((FElast.GetValue() <= key) && (key <= FE.GetValue()))
      {
        double Denominator = double(FE.GetValue()) - double(FElast.GetValue());
        if (Denominator != 0.0)
        {
          // Generate equation of the line and get intersection.
          double M = (FE.GetMembership() - FElast.GetMembership()) / Denominator;
          double B = FE.GetMembership() - (M * (FE.GetValue()));
          double Y = (M * double(key)) + B;
    
          FE.SetMembership(Y);
          FE.SetValue(key);
          return FE;
        }
        else
        {
          return FE;
        }
      }
      FElast = FE;
    }
  }

  // Outside of the fuzzy membership functions boundaries, return dummy data.
  FE.SetMembership(0);
  FE.SetValue(0);
  FE.SetTnorm(Min);
  FE.SetTconorm(Max);

  return FE;
}

double CFuzzyMembershipFunction::GetValue (long x) const
{
  CFuzzyElement FE;
  CFuzzyElement FElast;
  
  // Init last fuzzy variable index.
  GetVertex(0,&FElast);

  // Go through the verticies and campare against value.
  for (int v=0; v < GetNumVertices(); v++)
  {
    if (GetVertex(v,&FE))
    {
      if ((FElast.GetValue() <= x) && (x <= FE.GetValue()))
      {
        double Denominator = double(FE.GetValue()) - double(FElast.GetValue());
        if (Denominator != 0.0)
        {
          // Generate equation of the line and get intersection.
          double M = (FE.GetMembership() - FElast.GetMembership()) / Denominator;
          double B = FE.GetMembership() - (M * (FE.GetValue()));
          double Y = (M * double(x)) + B;
          return Y;
        }
      }
      FElast = FE;
    }
  }

  return 0.0;
}

const char* CFuzzyMembershipFunction::GetName () const
{
   return m_Name.c_str();
}

void CFuzzyMembershipFunction::SetName (const char* name)
{
   m_Name = name;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator + (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FMFreturn.DeleteVertices();
  for (long l = 0; l < GetNumVertices(); l++)
  {
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      if ((GetVertex(l,&FEleft)) && (right.GetVertex(r,&FEright)))
      {
        FE = FEleft + FEright;
        pPreviousFE = FMFreturn.GetpVertex(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FMFreturn.AddVertex(FE);
      }
    }
  }

  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator - (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFreturn;
  CFuzzyElement  FEleft;
  CFuzzyElement  FEright;
  CFuzzyElement  FE;
  CFuzzyElement *pPreviousFE;

  for (long l = 0; l < GetNumVertices(); l++)
  {
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      if ((GetVertex(l,&FEleft)) && (right.GetVertex(r,&FEright)))
      {
        FE = FEleft - FEright;
        pPreviousFE = FMFreturn.GetpVertex(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FMFreturn.AddVertex(FE);
      }
    }
  }

  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator * (const CFuzzyMembershipFunction& right) const
{
  static CFuzzyMembershipFunction FMFreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FMFreturn.DeleteVertices();
  for (long l = 0; l < GetNumVertices(); l++)
  {
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      if ((GetVertex(l,&FEleft)) && (right.GetVertex(r,&FEright)))
      {
        FE = FEleft * FEright;
        pPreviousFE = FMFreturn.GetpVertex(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FMFreturn.AddVertex(FE);
      }
    }
  }

  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator / (const CFuzzyMembershipFunction& right) const
{
  static CFuzzyMembershipFunction FMFreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FMFreturn.DeleteVertices();
  for (long l = 0; l < GetNumVertices(); l++)
  {
    for (long r = 0; r < right.GetNumVertices(); r++)
    {
      if ((GetVertex(l,&FEleft)) && (right.GetVertex(r,&FEright)))
      {
        FE = FEleft / FEright;
        pPreviousFE = FMFreturn.GetpVertex(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FMFreturn.AddVertex(FE);
      }
    }
  }

  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator ! ()
{
  CFuzzyElement FE;
  static CFuzzyMembershipFunction FMFreturn;

  FMFreturn.DeleteVertices();
  for (long i=0; i < GetNumVertices(); i++)
  {
    if (GetVertex(i,&FE))
    {
      FMFreturn.AddVertex(!FE);
    }
  }

  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator && (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFreturn;

   switch(m_Tnorm)
   {
      case Min : CFuzzyOperator::Minimum(&FMFreturn, *this, right);
         break;
      case DrasticProduct : CFuzzyOperator::DrasticProduct(&FMFreturn, *this, right);
         break;
      case BoundedDifference : CFuzzyOperator::BoundedDifference(&FMFreturn, *this, right);
         break;
      case EinsteinProduct : CFuzzyOperator::EinsteinProduct(&FMFreturn, *this, right);
         break;
      case AlgabraicProduct : CFuzzyOperator::AlgabraicProduct(&FMFreturn, *this, right);
         break;
      case HamacherProduct : CFuzzyOperator::HamacherProduct(&FMFreturn, *this, right);
         break;
   }
   
   return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator && (const CFuzzySet& right) const
{
  CFuzzyMembershipFunction FMFright;
  CFuzzyElement FE;
  
  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      FMFright.AddVertex(FE);
    }
  }

  return (*this && FMFright);
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator || (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFreturn;
  
  switch(m_Tconorm)
  {
  case Max : CFuzzyOperator::Maximum(&FMFreturn, *this, right);
    break;
  case DrasticSum : CFuzzyOperator::DrasticSum(&FMFreturn, *this, right);
    break;
  case BoundedSum : CFuzzyOperator::BoundedSum(&FMFreturn, *this, right);
    break;
  case EinsteinSum : CFuzzyOperator::EinsteinSum(&FMFreturn, *this, right);
    break;
  case AlgabraicSum : CFuzzyOperator::AlgabraicSum(&FMFreturn, *this, right);
    break;
  case HamacherSum : CFuzzyOperator::HamacherSum(&FMFreturn, *this, right);
    break;
  }
  
  return FMFreturn;
}

const CFuzzyMembershipFunction CFuzzyMembershipFunction::operator || (const CFuzzySet& right) const
{
  CFuzzyMembershipFunction FMFright;
  CFuzzyElement FE;
  
  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      FMFright.AddVertex(FE);
    }
  }

  return (*this || FMFright);
}

const CFuzzyAntecedent CFuzzyMembershipFunction::operator () (const CFuzzySet& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFMF(this);
  Result.SetpAssociatedFS(&right);
  return Result;
}

const CFuzzyAntecedent CFuzzyMembershipFunction::operator () (const CFuzzyMembershipFunction& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFMF(this);
  Result.SetpAssociatedFMF(&right);
  return Result;
}

CFuzzyMembershipFunction& CFuzzyMembershipFunction::Is (const CFuzzySet& right)
{
  CFuzzyElement FE;

  DeleteVertices();
  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      AddVertex(FE);
    }
  }

  return *this;
}

CFuzzyMembershipFunction& CFuzzyMembershipFunction::Is (const CFuzzyMembershipFunction& right)
{
  *this = right;
  return *this;
}

bool CFuzzyMembershipFunction::Save (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::out | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::ostream OutStream(&FileBuffer);
    bool Result = Write(OutStream);
    OutStream.flush();
    FileBuffer.close();
    return Result;
  }

  return false;
}

bool CFuzzyMembershipFunction::Write (std::ostream& stream)
{
  bool WriteResult = true;

  // Write out the membership function type
  EFileTypes FileType = fFuzzyMembershipFunction;
  stream.write((const char*)&FileType, sizeof(FileType));

  // Write out number of characters in the name.
  unsigned short NumCharsInName = m_Name.length();
  stream.write((const char*)&NumCharsInName, sizeof(NumCharsInName));

  // Write the name one character at a time.
  stream.write(m_Name.c_str(),NumCharsInName);

  // Write out Tnorm.
  stream.write((const char*)&m_Tnorm,sizeof(m_Tnorm));

  // Write out Tconorm.
  stream.write((const char*)&m_Tconorm,sizeof(m_Tconorm));

  // Write out number of CFuzzyElements in the membership function.
  long NumVertices = GetNumVertices();
  stream.write((const char*)&NumVertices, sizeof(NumVertices));

  // Write out each CFuzzyElement.
  CFuzzyElement FE;
  for (long v = 0; v < NumVertices; v++)
  {
    bool Result = GetVertex(v,&FE);
    if (!Result)
    {
      WriteResult = false;
    }

    // Write out value. 
    long Value = FE.GetValue();
    stream.write((const char*)&Value, sizeof(Value));

    // Write out membership.
    double Membership = FE.GetMembership();
    stream.write((const char*)&Membership, sizeof(Membership));

    // Write out Tnorm. 
    ETnormOperations Tnorm = FE.GetTnorm();
    stream.write((const char*)&Tnorm,sizeof(Tnorm));

    // Write out Tconorm.
    ETconormOperations Tconorm = FE.GetTconorm();
    stream.write((const char*)&Tconorm,sizeof(Tconorm));
  }

  return WriteResult;
}

bool CFuzzyMembershipFunction::Open (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::in | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::istream InStream(&FileBuffer);
    bool Result = Read(InStream);
    FileBuffer.close();
    return Result;
  }

  return false;
}

bool CFuzzyMembershipFunction::Read (std::istream& stream)
{
  // Check the fuzzy membership function type type
  EFileTypes FileType;
  stream.read((char*)&FileType, sizeof(FileType));
  if (FileType != fFuzzyMembershipFunction)
  {
    return false;
  }

  // Read in number of characters in the name.
  unsigned short NumCharsInName;
  stream.read((char*)&NumCharsInName, sizeof(NumCharsInName));

  // Read in name one character at a time.
  m_Name.erase();
  for (unsigned short s = 0; s < NumCharsInName; s++)
  {
    m_Name += stream.get();
  }

  // Read in Tnorm.
  stream.read((char*)&m_Tnorm,sizeof(m_Tnorm));

  // Read in Tconorm.
  stream.read((char*)&m_Tconorm,sizeof(m_Tconorm));

  // Read in number of CFuzzyElements in the membership function.
  long NumVertices;
  stream.read((char*)&NumVertices, sizeof(NumVertices));

  // Read in each CFuzzyElement.
  CFuzzyElement FE;
  for (long v = 0; v < NumVertices; v++)
  {
    // Read in value. 
    long Value;
    stream.read((char*)&Value, sizeof(Value));
    FE.SetValue(Value);

    // Read in membership.
    double Membership;
    stream.read((char*)&Membership, sizeof(Membership));
    FE.SetMembership(Membership);

    // Read in Tnorm. 
    ETnormOperations Tnorm;
    stream.read((char*)&Tnorm,sizeof(Tnorm));
    FE.SetTnorm(Tnorm);

    // Read in Tconorm.
    ETconormOperations Tconorm;
    stream.read((char*)&Tconorm,sizeof(Tconorm));
    FE.SetTconorm(Tconorm);

    AddVertex(FE);
  }

  return true;
}
