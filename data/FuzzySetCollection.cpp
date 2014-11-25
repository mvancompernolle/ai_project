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

#include "FuzzySetCollection.h"
#include <cassert>
#include <string>
#include <fstream>

CFuzzySetCollection::CFuzzySetCollection()
{

}

CFuzzySetCollection::CFuzzySetCollection(const CFuzzySetCollection &right)
{
  RemovepFuzzySets();

  for (int i = 0; i < right.GetNumFuzzySets(); i++)
  {
    AddpFuzzySet(right.GetpFuzzySet(i));
  }
}


CFuzzySetCollection::~CFuzzySetCollection()
{

}


CFuzzySetCollection & CFuzzySetCollection::operator=(const CFuzzySetCollection &right)
{
  RemovepFuzzySets();

  for (int i = 0; i < right.GetNumFuzzySets(); i++)
  {
    AddpFuzzySet(right.GetpFuzzySet(i));
  }

  m_Name = right.m_Name;

  return *this;
}


int CFuzzySetCollection::operator==(const CFuzzySetCollection &right) const
{
  assert(0);
  return false;
}

int CFuzzySetCollection::operator!=(const CFuzzySetCollection &right) const
{
  return !(*this == right);
}

bool CFuzzySetCollection::AddpFuzzySet (CFuzzySet* pFuzzySet)
{
  if (pFuzzySet != 0)
  {
    std::string ParseText = pFuzzySet->GetName();

    if (ParseText.length())
    {
      if (!GetpFuzzySet(ParseText.c_str()))
      {
        // The name does not already exist, so we will add it to the collection.
        m_FuzzySets.insert(m_FuzzySets.end(),pFuzzySet);
        return true;
      }
    }
    else
    {
      // The name is blank so we ignore it for purposes of uniqueness, check for same pointer in list.
      if (FindFuzzySet(pFuzzySet) == -1)
      {
        m_FuzzySets.insert(m_FuzzySets.end(),pFuzzySet);
        return true;
      }
    }
  }

  return false;  
}

bool CFuzzySetCollection::RemovepFuzzySet (CFuzzySet* pFuzzySet)
{
  if (pFuzzySet)
  {
    m_FuzzySets.remove(pFuzzySet);
    return true;
  }

  return false;
}

bool CFuzzySetCollection::RemovepFuzzySets ()
{
  if (GetNumFuzzySets())
  {
    m_FuzzySets.clear();
    return true;
  }

  return false;
}

CFuzzySetCollection* CFuzzySetCollection::RuleViolation (long rules)
{
  // Get starting and ending values.
  long Start = LONG_MAX;
  long End   = LONG_MIN;

  CFuzzySet* pSet;
  CFuzzyElement FEStart;
  CFuzzyElement FEEnd;
  
  // Set up the return set.
  CFuzzySetCollection* pViolationReturn = new CFuzzySetCollection;
  pViolationReturn->SetName("VIOlATION RETURN");

  bool MembershipViolation  = false;
  bool OverlappingViolation = false;

  for (long s = 0; s < GetNumFuzzySets(); s++)
  {
    pSet = GetpFuzzySet(s);
    if (pSet->GetFuzzyElement(0,&FEStart) && pSet->GetFuzzyElement(pSet->GetNumFuzzyElements()-1,&FEEnd))
    {
      // Check the start value.
      if (Start > FEStart.GetValue())
      {
        Start = FEStart.GetValue();
      }

      // Check the end value.
      if (End < FEEnd.GetValue())
      {
        End = FEEnd.GetValue();
      }
    }
  }//END for (long s = 0; s < GetNumFuzzySets(); s++)

  assert(Start <= End);

  // Iterator through the entire range covered by all fuzzy sets.
  for (long v = Start; v <= End; v++)
  {
    CFuzzyElement FE,
                  *pFE;
    double MembershipTotal   = 0.0,
           NumberOverlapping = 0.0;
    
    // Sum value for all sets covering this region.
    for (long s = 0; s < GetNumFuzzySets(); s++)
    {
      pSet = GetpFuzzySet(s);
      pSet->GetFuzzyElement(0,&FEStart);
      pSet->GetFuzzyElement(pSet->GetNumFuzzyElements()-1,&FEEnd);
    
      // Check range of each fuzzy set to see if we are located inside.
      if ((FEStart.GetValue() <= v) && (v <= FEEnd.GetValue()))
      {

        // Check to ensure that all sum to 1.0 in all places.
        if (rules & CFUZZYSETCOLLECTION_EQUAL_1)
        {
          pFE = pSet->GetpFuzzyElement(v);
          if (pFE)
          {
            MembershipTotal += pFE->GetMembership();
          }
          else
          {
            // There should be membership value here, this is a violation.
            pViolationReturn->AddpFuzzySet(pSet);
            MembershipViolation = true;
          }
        }//END if (rules & CFUZZYSETCOLLECTION_EQUAL_1)

        // Check to ensure that no more than 2 overlap at a single point.
        if (rules & CFUZZYSETCOLLECTION_OVERLAP_2)
        {
          pFE = pSet->GetpFuzzyElement(v);
          if ((pFE) && (pFE->GetMembership() == 0.0) && (v != Start) && (v != End))
          {
            // Two shared set adjavent ends vertices = 1.0, so each end verticie = 0.5.
            NumberOverlapping += 0.5;
          }
          else
          {
            NumberOverlapping += 1.0;
          }
        }//END if (rules & CFUZZYSETCOLLECTION_OVERLAP_2)
      }
    }//END for (long s = 0; s < GetNumFuzzySets(); s++)

    // Check for violations of greater than membership value of 1.0
    if ((rules & CFUZZYSETCOLLECTION_EQUAL_1) && (MembershipTotal != 1.0))
    {
      // Sum value for all sets covering this region.
      for (long s = 0; s < GetNumFuzzySets(); s++)
      {
        pSet = GetpFuzzySet(s);
        pSet->GetFuzzyElement(0,&FEStart);
        pSet->GetFuzzyElement(pSet->GetNumFuzzyElements()-1,&FEEnd);
      
        // Check range of each fuzzy set to see if we are located inside.
        if ((FEStart.GetValue() <= v) && (v <= FEEnd.GetValue()))
        {
          pViolationReturn->AddpFuzzySet(pSet);
          MembershipViolation = true;
        }
      }//END for (long s = 0; s < GetNumFuzzySets(); s++)
    }//END if ((rules & CFUZZYSETCOLLECTION_EQUAL_1) && (MembershipValue != 1.0))

    // Check for violations of more than 2 fuzzy sets overlaping at a single point.
    if ((rules & CFUZZYSETCOLLECTION_OVERLAP_2) && (NumberOverlapping != 2.0))
    {
      // Sum value for all sets covering this region.
      for (long s = 0; s < GetNumFuzzySets(); s++)
      {
        pSet = GetpFuzzySet(s);
        pSet->GetFuzzyElement(0,&FEStart);
        pSet->GetFuzzyElement(pSet->GetNumFuzzyElements()-1,&FEEnd);
      
        // Check range of each fuzzy set to see if we are located inside.
        if ((FEStart.GetValue() <= v) && (v <= FEEnd.GetValue()))
        {
          pViolationReturn->AddpFuzzySet(pSet);
          OverlappingViolation = true;
        }
      }//END for (long s = 0; s < GetNumFuzzySets(); s++)
    }//END if ((Overlapping != 2) && (rules & CFUZZYSETCOLLECTION_OVERLAP_2))
  }//END for (long v = Start; v != End; v++)

  std::string ParseText;
  if (MembershipViolation)
  {
    ParseText = pViolationReturn->GetName();
    ParseText += "| Membership != 1.0";
    pViolationReturn->SetName(ParseText.c_str());
  }

  if (OverlappingViolation)
  {
    ParseText = pViolationReturn->GetName();
    ParseText += "| Overlap != 2.0";
    pViolationReturn->SetName(ParseText.c_str());
  }

  if (MembershipViolation || OverlappingViolation)
  {
    return pViolationReturn;
  }
  else
  {
    delete pViolationReturn;
  }

  return 0;
}

long CFuzzySetCollection::FindFuzzySet (CFuzzySet* pFuzzySet)
{
  if (pFuzzySet)
  {
    for (long i=0; i < GetNumFuzzySets(); i++)
    {
      if (pFuzzySet == GetpFuzzySet(i))
      {
        return i;
      }
    }
  }

  return -1;
}

long CFuzzySetCollection::GetNumFuzzySets () const
{
  return m_FuzzySets.size();
}

CFuzzySet* CFuzzySetCollection::GetpFuzzySet (long index) const
{
  if ((0 <= index) && (index < GetNumFuzzySets()))
  {
    T_CFuzzySet_list::const_iterator FSiterator = m_FuzzySets.begin();
    for (long i = 0; i < index; i++)
    {
      FSiterator++;
    }

    return *FSiterator;
  }

  return 0;
}

const char* CFuzzySetCollection::GetName () const
{
  return m_Name.c_str();
}

CFuzzySet* CFuzzySetCollection::GetpFuzzySet (const char* name) const
{
  std::string ParseText = name;

  T_CFuzzySet_list::const_iterator FSiterator;
  for (FSiterator = m_FuzzySets.begin(); FSiterator != m_FuzzySets.end(); FSiterator++)
  {
    if (ParseText == (*FSiterator)->GetName())
    {
      return *FSiterator;
    }
  }
  
  return 0;
}

void CFuzzySetCollection::SetName (const char* name)
{
  m_Name = name;
}

bool CFuzzySetCollection::Save (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::out | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::ostream OutStream(&FileBuffer);
    
    // Write out the set collection type
    EFileTypes FileType = fFuzzySetCollection;
    OutStream.write((const char*)&FileType, sizeof(FileType));

    // Write out number of characters in the name.
    unsigned short NumCharsInName = m_Name.length();
    OutStream.write((const char*)&NumCharsInName, sizeof(NumCharsInName));

    // Write the name one character at a time.
    OutStream.write(m_Name.c_str(),NumCharsInName);
    
    // Write out the number of fuzzy sets.
    long NumFuzzySets = GetNumFuzzySets();
    OutStream.write((const char*)&NumFuzzySets, sizeof(NumFuzzySets));

    // Write out each fuzzy set.
    for (long s = 0; s < NumFuzzySets; s++)
    {
      CFuzzySet *pFuzzySet = GetpFuzzySet(s);
      if (!pFuzzySet)
      {
        return false;
      }
      pFuzzySet->Write(OutStream);
    }

    OutStream.flush();
    FileBuffer.close();
    return true;
  }

  return false;
}

bool CFuzzySetCollection::Open (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::in | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::istream InStream(&FileBuffer);
    
    // Read in the set collection type
    EFileTypes FileType;
    InStream.read((char*)&FileType, sizeof(FileType));
    if (FileType != fFuzzySetCollection)
    {
      FileBuffer.close();
      return false;
    }

    // Read in number of characters in the name.
    unsigned short NumCharsInName;
    InStream.read((char*)&NumCharsInName, sizeof(NumCharsInName));

    m_Name.erase();
    // Read in name one character at a time.
    for (unsigned short n = 0; n < NumCharsInName; n++)
    {
      m_Name += InStream.get();
    }
     
    // Read in the number of fuzzy sets.
    long NumFuzzySets;
    InStream.read((char*)&NumFuzzySets, sizeof(NumFuzzySets));

    // Read in each fuzzy set.
    for (long s = 0; s < NumFuzzySets; s++)
    {
      CFuzzySet *pFuzzySet = new CFuzzySet;
      pFuzzySet->Read(InStream);
      AddpFuzzySet(pFuzzySet);
    }

    FileBuffer.close();
    return true;
  }

  return false;
}

