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

#include "FuzzyOperator.h"
#include "FuzzySetRelation.h"
#include "FuzzyMembershipFunction.h"
#include "FuzzySubPartition.h"
#include "FuzzyRule.h"
#include "FuzzyAntecedent.h"
#include "FuzzySet.h"
#include <cassert>
#include <fstream>

CFuzzySet::CFuzzySet()
  : m_Tnorm(Min), m_Tconorm(Max)
{
  m_LastIndex = 0;
  m_LastIterator = m_FuzzyElements.begin();
}

CFuzzySet::CFuzzySet(const CFuzzySet &right)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  CFuzzyElement FE;

  m_Name    = right.GetName();
  m_Tnorm   = right.GetTnorm();
  m_Tconorm = right.GetTconorm();

  m_LastIndex = 0;
  m_LastIterator = m_FuzzyElements.begin();

  for (long r = 0; r < right.GetNumFuzzyElements(); r++)
  {
    if (right.GetFuzzyElement(r,&FE))
    {
      AddFuzzyElement(FE);
    }
  }
}

CFuzzySet::CFuzzySet (const CFuzzyMembershipFunction& right, long segments)
  : m_Tnorm(Min), m_Tconorm(Max)
{
  if (segments > 1)
  {
    // Fill new CFuzzySet with segmented peices between each interval of the FMF.
    for (long v=0; v < right.GetNumVertices()-1; v++)
    {
      CFuzzyElement FE,
                     FEnext;
  
      if ((right.GetVertex(v,&FE)) && (right.GetVertex(v+1,&FEnext)))
      {
        long IncrementAmount = (FEnext.GetValue() - FE.GetValue()) / segments;
        if (IncrementAmount == 0)
        {
          IncrementAmount = 1;
        }

        // Add the end verticies.
        AddFuzzyElement(FE);
        AddFuzzyElement(FEnext);
  
        // Add intervening vertex segments.
        for (long i=FE.GetValue() + IncrementAmount; i < FEnext.GetValue(); i+= IncrementAmount)
        {
          FE.SetValue(i);
          FE.SetMembership(right.GetValue(i));
          AddFuzzyElement(FE);
        }
      }
    }//END for (long v=0; v < right.GetNumVertices(); v++)
  }//END if (segments > 1)

  m_LastIndex = 0;
  m_LastIterator = m_FuzzyElements.begin();
}


CFuzzySet::~CFuzzySet()
{
  DeleteFuzzyElements();
}


CFuzzySet & CFuzzySet::operator=(const CFuzzySet &right)
{
  m_FuzzyElements.clear();
  CFuzzyElement FEtransfer;
  SetName(right.GetName());
  
  for (long i=0; i < right.GetNumFuzzyElements(); i++)
  {
    if(!right.GetFuzzyElement(i,&FEtransfer))
    {
      return *this;
    }
    AddFuzzyElement(FEtransfer);
  }
  
  return *this;
}


int CFuzzySet::operator==(const CFuzzySet &right) const
{
  if (m_Name == right.GetName())
  {
    return true;
  }
  
  return false;
}

int CFuzzySet::operator!=(const CFuzzySet &right) const
{
  return !(*this == right);
}

void CFuzzySet::AddFuzzySet (const CFuzzySet& right)
{
  CFuzzyElement FE;
  for (long i=0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      AddFuzzyElement(FE);
    }
  }
  
  m_LastIndex = 0;
  m_LastIterator = m_FuzzyElements.begin();
}

void CFuzzySet::AddFuzzyElement (const CFuzzyElement& FuzzyElement)
{
  CFuzzyElement* pFuzzyElement = new CFuzzyElement(FuzzyElement);
  m_FuzzyElements.insert(T_CFuzzyElement_map::value_type(pFuzzyElement->GetValue(),pFuzzyElement));
  m_LastIndex = 0;
  m_LastIterator = m_FuzzyElements.begin();
}

bool CFuzzySet::DeleteFuzzyElement (long key)
{
  if (m_FuzzyElements.find(key) != m_FuzzyElements.end())
  {
    // The variable does exist so we can delete it.
    m_FuzzyElements.erase(key);
    m_LastIndex = 0;
    m_LastIterator = m_FuzzyElements.begin();
    return true;
  }
  
  return false;
}

bool CFuzzySet::DeleteFuzzyElements ()
{
  if (GetNumFuzzyElements() != 0)
  {
    m_FuzzyElements.clear();
    m_LastIndex = 0;
    m_LastIterator = m_FuzzyElements.begin();
    return true;
  }
  
  return false;
}

long CFuzzySet::FindFuzzyElement (long key) const
{
  for (long i=0; i < GetNumFuzzyElements(); i++)
  {
    CFuzzyElement FE;
    GetFuzzyElement(i,&FE);
    if (FE.GetValue() == key)
    {
      return i;
    }
  }

  return -1;
}

long CFuzzySet::FindLowerIndex (long key) const
{
  long Index = FindFuzzyElement(key);
  if (Index < 0)
  {
    for (long i=GetNumFuzzyElements()-1; i != 0 ; i--)
    {
      CFuzzyElement FE;
      GetFuzzyElement(i,&FE);
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
}

long CFuzzySet::FindUpperIndex (long key) const
{
  long Index = FindFuzzyElement(key);
  if (Index < 0)
  {
    for (long i= 0; i < GetNumFuzzyElements() ; i++)
    {
      CFuzzyElement FE;
      GetFuzzyElement(i,&FE);
      if (FE.GetValue() > key)
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
    if (Index+1 < GetNumFuzzyElements())
    {
      return Index+1;
    }

    return -1;
  }
}

bool CFuzzySet::GetFuzzyElement (long index, CFuzzyElement* pFuzzyElement) const
{
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
    if (ThisVi == m_FuzzyElements.end())
    {
      *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = m_FuzzyElements.begin();
      *(const_cast<long*>(&m_LastIndex)) = 0;
      pFuzzyElement->SetMembership(0.0);
      return false;
    }

    // We have a valid iterator, so set last and return CFuzzyElement.
    *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = ThisVi;
    *(const_cast<long*>(&m_LastIndex)) = index;
    *pFuzzyElement = *((*ThisVi).second);
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
    if (ThisVi == m_FuzzyElements.end())
    {
      *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = m_FuzzyElements.begin();
      *(const_cast<long*>(&m_LastIndex)) = 0;
      pFuzzyElement->SetMembership(0.0);
      return false;
    }

    // We have a valid iterator, so set last and return CFuzzyElement.
    *(const_cast<T_CFuzzyElement_map::const_iterator*>(&m_LastIterator)) = ThisVi;
    *(const_cast<long*>(&m_LastIndex)) = index;
    *pFuzzyElement = *((*ThisVi).second);
    return true;
  }
}

const CFuzzyElement& CFuzzySet::GetFuzzyElement (long key) const
{
  static CFuzzyElement FE;
  T_CFuzzyElement_map::const_iterator ThisVi;
  ThisVi = m_FuzzyElements.find(key);

  if (ThisVi == m_FuzzyElements.end())
  {
    FE.SetValue(key);
    FE.SetMembership(0.0);
    return FE;
  }

  FE = *((*ThisVi).second);
  return FE;
}

long CFuzzySet::GetNumFuzzyElements () const
{
   return m_FuzzyElements.size();
}

CFuzzyElement* CFuzzySet::GetpFuzzyElement (long key) const
{
  T_CFuzzyElement_map::const_iterator ThisVi;
  ThisVi = m_FuzzyElements.find(key);
  
  if (ThisVi == m_FuzzyElements.end())
  {
    return 0;
  }
  
  return (*ThisVi).second;
}

const char* CFuzzySet::GetName () const
{
   return m_Name.c_str();
}

void CFuzzySet::SetName (const char* name)
{
  m_Name = name;
}

const CFuzzySet CFuzzySet::operator += (const CFuzzyElement& right)
{
  // Check to see if this fuzzy variable is already in the fuzzyset.
  if (m_FuzzyElements.find(right.GetValue()) == m_FuzzyElements.end())
  {
     CFuzzyElement* pFuzzyElement = new CFuzzyElement(right);
     m_FuzzyElements.insert(T_CFuzzyElement_map::value_type(right.GetValue(),pFuzzyElement));
  }

  return *this;
}

const CFuzzySet CFuzzySet::operator + (const CFuzzySet& right) const
{
  static CFuzzySet FSreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FSreturn.DeleteFuzzyElements();
  for (long l = 0; l < GetNumFuzzyElements(); l++)
  {
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      if ((GetFuzzyElement(l,&FEleft)) && (right.GetFuzzyElement(r,&FEright)))
      {
        FE = FEleft + FEright;
        pPreviousFE = FSreturn.GetpFuzzyElement(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FSreturn.AddFuzzyElement(FE);
      }
    }
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator - (const CFuzzySet& right) const
{
  static CFuzzySet FSreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FSreturn.DeleteFuzzyElements();
  for (long l = 0; l < GetNumFuzzyElements(); l++)
  {
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      if ((GetFuzzyElement(l,&FEleft)) && (right.GetFuzzyElement(r,&FEright)))
      {
        FE = FEleft - FEright;
        pPreviousFE = FSreturn.GetpFuzzyElement(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FSreturn.AddFuzzyElement(FE);
      }
    }
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator * (const CFuzzySet& right) const
{
  static CFuzzySet FSreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FSreturn.DeleteFuzzyElements();
  for (long l = 0; l < GetNumFuzzyElements(); l++)
  {
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      if ((GetFuzzyElement(l,&FEleft)) && (right.GetFuzzyElement(r,&FEright)))
      {
        FE = FEleft * FEright;
        pPreviousFE = FSreturn.GetpFuzzyElement(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FSreturn.AddFuzzyElement(FE);
      }
    }
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator / (const CFuzzySet& right) const
{
  static CFuzzySet FSreturn;
  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FE;
  CFuzzyElement *pPreviousFE;

  FSreturn.DeleteFuzzyElements();
  for (long l = 0; l < GetNumFuzzyElements(); l++)
  {
    for (long r = 0; r < right.GetNumFuzzyElements(); r++)
    {
      if ((GetFuzzyElement(l,&FEleft)) && (right.GetFuzzyElement(r,&FEright)))
      {
        FE = FEleft / FEright;
        pPreviousFE = FSreturn.GetpFuzzyElement(FE.GetValue());
        if (pPreviousFE)
        {
          *pPreviousFE = *pPreviousFE || FE;
        }

        FSreturn.AddFuzzyElement(FE);
      }
    }
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator ! ()
{
   static CFuzzySet FSreturn;
   CFuzzyElement FE;

   FSreturn.DeleteFuzzyElements();
   for (long i=0; i < GetNumFuzzyElements(); i++)
   {
      if (GetFuzzyElement(i,&FE))
      {
         FSreturn.AddFuzzyElement(!FE);
      }
   }

   return FSreturn;
}

const CFuzzySet CFuzzySet::operator && (const CFuzzySet& right) const
{
  CFuzzySet FSreturn;

  switch(m_Tnorm)
  {
  case Min : CFuzzyOperator::Minimum(&FSreturn, *this, right);
    break;
  case DrasticProduct : CFuzzyOperator::DrasticProduct(&FSreturn, *this, right);
    break;
  case BoundedDifference : CFuzzyOperator::BoundedDifference(&FSreturn, *this, right);
    break;
  case EinsteinProduct : CFuzzyOperator::EinsteinProduct(&FSreturn, *this, right);
    break;
  case AlgabraicProduct : CFuzzyOperator::AlgabraicProduct(&FSreturn, *this, right);
    break;
  case HamacherProduct : CFuzzyOperator::HamacherProduct(&FSreturn, *this, right);
    break;
  }
  
  return FSreturn;
}

const CFuzzySet CFuzzySet::operator && (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFleft;
  CFuzzyMembershipFunction FMF;
  CFuzzySet FSreturn;
  CFuzzyElement FE;
  
  long i;
  for (i = 0; i < GetNumFuzzyElements(); i++)
  {
    if (GetFuzzyElement(i,&FE))
    {
      FMFleft.AddVertex(FE);
    }
  }
  
  FMF = FMFleft && right;
  FSreturn.DeleteFuzzyElements();

  for (i = 0; i < FMF.GetNumVertices(); i++)
  {
    if (FMF.GetVertex(i,&FE))
    {
      FSreturn.AddFuzzyElement(FE);
    }
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator && (const CFuzzySetRelation& right)
{
  assert(GetNumFuzzyElements() == right.GetNumRows());

  CFuzzySet FSreturn;
  
  if (GetNumFuzzyElements() != right.GetNumRows())
  {
    return FSreturn;
  }

  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FEpair;
  CFuzzyElement FEtotal;
  
  for (long c=0; c < right.GetNumColumns(); c++)
  {
    bool FirstPass = true;

    for (long r=0; r < GetNumFuzzyElements(); r++)
    {
    
      GetFuzzyElement(r,&FEleft);
      FEleft.SetTnorm(GetTnorm());
      FEleft.SetTconorm(GetTconorm());
      FEright.SetMembership(right.GetValue(c,r));

      FEpair = FEleft && FEright;
      
      if (FirstPass)
      {
        FEtotal = FEpair || FEpair;
        FirstPass = false;
      }
      else
      {
        FEtotal = FEpair || FEtotal;
      }
    }
    right.GetpColumnHeadings()->GetFuzzyElement(c,&FEpair);
    FEtotal.SetValue(FEpair.GetValue());
    FSreturn.AddFuzzyElement(FEtotal);
  }

  return FSreturn;
}

const CFuzzySet CFuzzySet::operator || (const CFuzzySet& right) const
{
  CFuzzySet FSreturn;

  switch(m_Tconorm)
  {
  case Max : CFuzzyOperator::Maximum(&FSreturn, *this, right);
    break;
  case DrasticSum : CFuzzyOperator::DrasticSum(&FSreturn, *this, right);
    break;
  case BoundedSum : CFuzzyOperator::BoundedSum(&FSreturn, *this, right);
    break;
  case EinsteinSum : CFuzzyOperator::EinsteinSum(&FSreturn, *this, right);
    break;
  case AlgabraicSum : CFuzzyOperator::AlgabraicSum(&FSreturn, *this, right);
    break;
  case HamacherSum : CFuzzyOperator::HamacherSum(&FSreturn, *this, right);
    break;
  }
  
  return FSreturn;
}

const CFuzzySet CFuzzySet::operator || (const CFuzzyMembershipFunction& right) const
{
  CFuzzyMembershipFunction FMFleft;
  CFuzzyMembershipFunction FMF;
  CFuzzySet FSreturn;
  CFuzzyElement FE;
  
  long i;
  for (i = 0; i < GetNumFuzzyElements(); i++)
  {
    if (GetFuzzyElement(i,&FE))
    {
      FMFleft.AddVertex(FE);
    }
  }
  
  FMF = FMFleft || right;

  for (i = 0; i < FMF.GetNumVertices(); i++)
  {
    if (FMF.GetVertex(i,&FE))
    {
      FSreturn.AddFuzzyElement(FE);
    }
  }

  return FSreturn;
}

const CFuzzyAntecedent CFuzzySet::operator () (const CFuzzySet& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFS(this);
  Result.SetpAssociatedFS(&right);
  return Result;
}

const CFuzzyAntecedent CFuzzySet::operator () (const CFuzzyMembershipFunction& right)
{
  CFuzzyAntecedent Result;
  Result.SetpFS(this);
  Result.SetpAssociatedFMF(&right);
  return Result;
}

CFuzzySet& CFuzzySet::Is (const CFuzzySet& right)
{
  *this = right;
  return *this;
}

CFuzzySet& CFuzzySet::Is (const CFuzzyMembershipFunction& right, long segments)
{
  CFuzzySet FS(right,segments);
  *this = FS;
  return *this;
}

bool CFuzzySet::Save (const char* name)
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

bool CFuzzySet::Write (std::ostream& stream)
{
  bool WriteResult = true;

  // Write out the set type
  EFileTypes FileType = fFuzzySet;
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

  // Write out number of CFuzzyElements in the set.
  long NumFuzzyElements = GetNumFuzzyElements();
  stream.write((const char*)&NumFuzzyElements, sizeof(NumFuzzyElements));

  // Write out each CFuzzyElement.
  CFuzzyElement FE;
  for (long v = 0; v < NumFuzzyElements; v++)
  {
    bool Result = GetFuzzyElement(v,&FE);
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

bool CFuzzySet::Open (const char* name)
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

bool CFuzzySet::Read (std::istream& stream)
{
  // Check the set type
  EFileTypes FileType;
  stream.read((char*)&FileType, sizeof(FileType));
  if (FileType != fFuzzySet)
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

  // Read in number of CFuzzyElements in the set.
  long NumFuzzyElements;
  stream.read((char*)&NumFuzzyElements, sizeof(NumFuzzyElements));

  // Read in each CFuzzyElement.
  CFuzzyElement FE;
  for (long v = 0; v < NumFuzzyElements; v++)
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

    AddFuzzyElement(FE);
  }

  return true;
}

