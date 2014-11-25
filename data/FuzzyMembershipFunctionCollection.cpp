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
#include "FuzzyMembershipFunctionCollection.h"
#include <cassert>
#include <fstream>

CFuzzyMembershipFunctionCollection::CFuzzyMembershipFunctionCollection()
{

}

CFuzzyMembershipFunctionCollection::CFuzzyMembershipFunctionCollection(const CFuzzyMembershipFunctionCollection &right)

{
  RemovepFuzzyMembershipFunctions();

  for (int i = 0; i < right.GetNumFuzzyMembershipFunctions(); i++)
  {
    AddpFuzzyMembershipFunction(right.GetpFuzzyMembershipFunction(i));
  }
}


CFuzzyMembershipFunctionCollection::~CFuzzyMembershipFunctionCollection()
{

}


CFuzzyMembershipFunctionCollection & CFuzzyMembershipFunctionCollection::operator=(const CFuzzyMembershipFunctionCollection &right)
{
  RemovepFuzzyMembershipFunctions();

  for (int i = 0; i < right.GetNumFuzzyMembershipFunctions(); i++)
  {
    AddpFuzzyMembershipFunction(right.GetpFuzzyMembershipFunction(i));
  }

  m_Name = right.m_Name;

  return *this;
}


int CFuzzyMembershipFunctionCollection::operator==(const CFuzzyMembershipFunctionCollection &right) const
{
  assert(0);
  return false;
}

int CFuzzyMembershipFunctionCollection::operator!=(const CFuzzyMembershipFunctionCollection &right) const
{
  return !(*this == right);
}

bool CFuzzyMembershipFunctionCollection::AddpFuzzyMembershipFunction (CFuzzyMembershipFunction* pFMF)
{
  if (pFMF != 0)
  {
    std::string ParseText = pFMF->GetName();

    if (ParseText.length())
    {
      if (!GetpFuzzyMembershipFunction(ParseText.c_str()))
      {
        // The name does not already exist, so we will add it to the collection.
        m_FuzzyMembershipFunctions.insert(m_FuzzyMembershipFunctions.end(),pFMF);
        return true;
      }
    }
    else
    {
      // The name is blank so we ignore it for purposes of uniqueness, check for same pointer in list.
      if (FindFuzzyMembershipFunction(pFMF) == -1)
      {
        m_FuzzyMembershipFunctions.insert(m_FuzzyMembershipFunctions.end(),pFMF);
        return true;
      }
    }
  }

  return false;  
}

bool CFuzzyMembershipFunctionCollection::RemovepFuzzyMembershipFunction (CFuzzyMembershipFunction* pFMF)
{
  if (pFMF)
  {
    m_FuzzyMembershipFunctions.remove(pFMF);
    return true;
  }

  return false;
}

bool CFuzzyMembershipFunctionCollection::RemovepFuzzyMembershipFunctions ()
{
  if (GetNumFuzzyMembershipFunctions())
  {
    m_FuzzyMembershipFunctions.clear();
    return true;
  }

  return false;
}

CFuzzyMembershipFunctionCollection* CFuzzyMembershipFunctionCollection::RuleViolation (long rules)
{
  CFuzzyElement FEstart;
  CFuzzyElement FEend;
  CFuzzyElement FE;
  
  // Set up the return set.
  CFuzzyMembershipFunctionCollection* pViolationReturn = new CFuzzyMembershipFunctionCollection;
  pViolationReturn->SetName("VIOlATION RETURN");

  bool MembershipViolation  = false,
       OverlappingViolation = false;
  
  // Get each fuzzy membership function.
  for (long ThisFMF=0; ThisFMF < GetNumFuzzyMembershipFunctions(); ThisFMF++)
  {
    CFuzzyMembershipFunction *pThisFMF = GetpFuzzyMembershipFunction(ThisFMF);
    assert(pThisFMF);

    for (long v=0; v < pThisFMF->GetNumVertices(); v++)
    {
      pThisFMF->GetVertex(v,&FE);

      double MembershipTotal   = FE.GetMembership(),
             NumberOverlapping = 1.0;

      // Check for special case of overlapping end verticies
      if (((v == 0) || (v == pThisFMF->GetNumVertices()-1)) && FE.GetMembership() == 0.0)
      {
        NumberOverlapping = 0.5;
      }

      // Check it against each other fuzzy membership function.
      for (long OtherFMF=0; OtherFMF < GetNumFuzzyMembershipFunctions(); OtherFMF++)
      {
        if (ThisFMF != OtherFMF)
        {
          CFuzzyMembershipFunction *pOtherFMF = GetpFuzzyMembershipFunction(OtherFMF);
          assert(pOtherFMF);
     
          pOtherFMF->GetVertex(0,&FEstart);
          pOtherFMF->GetVertex(pOtherFMF->GetNumVertices()-1,&FEend);
        
          // Check for overlap with thisFMF at each vertex with otherFMF.
          if ((FEstart.GetValue() <= FE.GetValue()) && (FE.GetValue() <= FEend.GetValue()))
          {
            // Check to ensure that all sum to 1.0 in all places.
            if (rules & CFMFCOLLECTION_EQUAL_1)
            {
              MembershipTotal += pOtherFMF->GetValue(FE.GetValue());
            }//END if (rules & CFMFCOLLECTION_EQUAL_1)

            // Check to ensure that no more than 2 overlap at a single point.
            if (rules & CFMFCOLLECTION_OVERLAP_2)
            {
              if ((pOtherFMF->GetValue(FE.GetValue()) == 0.0) && ((FE.GetValue() == FEstart.GetValue()) || 
                  (FE.GetValue() == FEend.GetValue())))
              {
                // Two shared set adjavent ends vertices = 1.0, so each end verticie = 0.5.
                NumberOverlapping += 0.5;
              }
              else
              {
                NumberOverlapping += 1.0;
              }
            }//END if (rules & CFMFCOLLECTION_OVERLAP_2)
          }
        }
      }//END for (long ThisFMF=0; ThisFMF < GetNumFuzzyMembershipFunctions(); ThisFMF++)
  
      // If we have a membership value violation we need to add this membership function.
      if ((rules & CFMFCOLLECTION_EQUAL_1) && (MembershipTotal != 1.0))
      {
        pViolationReturn->AddpFuzzyMembershipFunction(pThisFMF);
        MembershipViolation = true;
  
        for (long OtherFMF=0; OtherFMF < GetNumFuzzyMembershipFunctions(); OtherFMF++)
        {
          if (ThisFMF != OtherFMF)
          {
            CFuzzyMembershipFunction *pOtherFMF = GetpFuzzyMembershipFunction(OtherFMF);
            assert(pOtherFMF);
       
            pOtherFMF->GetVertex(0,&FEstart);
            pOtherFMF->GetVertex(pOtherFMF->GetNumVertices()-1,&FEend);
          
            // Fuzzy membership functions must overlap to count.
            if ((FEstart.GetValue() <= FE.GetValue()) && (FE.GetValue() <= FEend.GetValue()))
            {
              pViolationReturn->AddpFuzzyMembershipFunction(pOtherFMF);
            }
          }
        }
      }//END if ((rules & CFMFCOLLECTION_EQUAL_1) && (MembershipTotal != 1.0))
  
      // If we have an overlap violation we need to add this membership function.
      if ((rules & CFMFCOLLECTION_OVERLAP_2) && ((NumberOverlapping <= 1.0) || (2.0 < NumberOverlapping)))
      {
        pViolationReturn->AddpFuzzyMembershipFunction(pThisFMF);
        OverlappingViolation = true;
  
        for (long OtherFMF=0; OtherFMF < GetNumFuzzyMembershipFunctions(); OtherFMF++)
        {
          if (ThisFMF != OtherFMF)
          {
            CFuzzyMembershipFunction *pOtherFMF = GetpFuzzyMembershipFunction(OtherFMF);
            assert(pOtherFMF);
       
            pOtherFMF->GetVertex(0,&FEstart);
            pOtherFMF->GetVertex(pOtherFMF->GetNumVertices()-1,&FEend);
          
            // Fuzzy membership functions must overlap to count.
            if ((FEstart.GetValue() <= FE.GetValue()) && (FE.GetValue() <= FEend.GetValue()))
            { 
              pViolationReturn->AddpFuzzyMembershipFunction(pOtherFMF);
            }
          }
        }
      }//END if ((rules & CFMFCOLLECTION_OVERLAP_2) && (NumberOverlapping != 2.0))
    }//END for (v=0; v < pThisFMF->GetNumVerticies(); v++)
  }//END for (long ThisFMF=0; ThisFMF < GetNumFuzzyMembershipFunctions(); ThisFMF++)
  
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

long CFuzzyMembershipFunctionCollection::FindFuzzyMembershipFunction (CFuzzyMembershipFunction* pFMF)
{
  if (pFMF)
  {
    for (long i=0; i < GetNumFuzzyMembershipFunctions(); i++)
    {
      if (pFMF == GetpFuzzyMembershipFunction(i))
      {
        return i;
      }
    }
  }

  return -1;
}

long CFuzzyMembershipFunctionCollection::GetNumFuzzyMembershipFunctions () const
{
  return m_FuzzyMembershipFunctions.size();
}

CFuzzyMembershipFunction* CFuzzyMembershipFunctionCollection::GetpFuzzyMembershipFunction (long index) const
{
  if ((0 <= index) && (index < GetNumFuzzyMembershipFunctions()))
  {
    T_CFuzzyMembershipFunction_list::const_iterator FMFiterator = m_FuzzyMembershipFunctions.begin();
    for (long i = 0; i < index; i++)
    {
      FMFiterator++;
    }

    return *FMFiterator;
  }

  return 0;
}

CFuzzyMembershipFunction* CFuzzyMembershipFunctionCollection::GetpFuzzyMembershipFunction (const char* name) const
{
  std::string ParseText = name;

  T_CFuzzyMembershipFunction_list::const_iterator FMFiterator;
  for (FMFiterator = m_FuzzyMembershipFunctions.begin(); FMFiterator != m_FuzzyMembershipFunctions.end(); FMFiterator++)
  {
    if (ParseText == (*FMFiterator)->GetName())
    {
      return *FMFiterator;
    }
  }
  
  return 0;
}

const char* CFuzzyMembershipFunctionCollection::GetName () const
{
  return m_Name.c_str();
}

void CFuzzyMembershipFunctionCollection::SetName (const char* name)
{
  m_Name = name;
}

bool CFuzzyMembershipFunctionCollection::Save (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::out | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::ostream OutStream(&FileBuffer);
    
    // Write out the membership function collection type
    EFileTypes FileType = fFuzzyMembershipFunctionCollection;
    OutStream.write((const char*)&FileType, sizeof(FileType));

    // Write out number of characters in the name.
    unsigned short NumCharsInName = m_Name.length();
    OutStream.write((const char*)&NumCharsInName, sizeof(NumCharsInName));

    // Write the name one character at a time.
    OutStream.write(m_Name.c_str(),NumCharsInName);
    
    // Write out the number of fuzzy membership functions.
    long NumFuzzyMembershipFunctions = GetNumFuzzyMembershipFunctions();
    OutStream.write((const char*)&NumFuzzyMembershipFunctions, sizeof(NumFuzzyMembershipFunctions));

    // Write out each fuzzy membership function.
    for (long s = 0; s < NumFuzzyMembershipFunctions; s++)
    {
      CFuzzyMembershipFunction *pFuzzyMembershipFunction = GetpFuzzyMembershipFunction(s);
      if (!pFuzzyMembershipFunction)
      {
        return false;
      }
      pFuzzyMembershipFunction->Write(OutStream);
    }

    OutStream.flush();
    FileBuffer.close();
    return true;
  }

  return false;
}

bool CFuzzyMembershipFunctionCollection::Open (const char* name)
{
  std::filebuf FileBuffer;
  
  FileBuffer.open(name,std::ios::in | std::ios::binary);
  if (FileBuffer.is_open())
  {
    std::istream InStream(&FileBuffer);
    
    // Read in the set collection type
    EFileTypes FileType;
    InStream.read((char*)&FileType, sizeof(FileType));
    if (FileType != fFuzzyMembershipFunctionCollection)
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
     
    // Read in the number of fuzzy membership function.
    long NumFuzzyMembershipFunctions;
    InStream.read((char*)&NumFuzzyMembershipFunctions, sizeof(NumFuzzyMembershipFunctions));

    // Read in each fuzzy set.
    for (long s = 0; s < NumFuzzyMembershipFunctions; s++)
    {
      CFuzzyMembershipFunction *pFuzzyMembershipFunction = new CFuzzyMembershipFunction;
      pFuzzyMembershipFunction->Read(InStream);
      AddpFuzzyMembershipFunction(pFuzzyMembershipFunction);
    }

    FileBuffer.close();
    return true;
  }

  return false;
}

