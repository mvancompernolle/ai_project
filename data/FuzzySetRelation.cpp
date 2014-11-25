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

#include "FuzzyMembershipFunction.h"
#include "FuzzySetRelation.h"
#include <cassert>

CFuzzySetRelation::CFuzzySetRelation()
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{

}

CFuzzySetRelation::CFuzzySetRelation(const CFuzzySetRelation &right)
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{
  assert(0);
}

CFuzzySetRelation::CFuzzySetRelation (const CFuzzySet& left, const CFuzzySet& right)
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{
  Initialize(left,right);
}

CFuzzySetRelation::CFuzzySetRelation (const CFuzzySet& fs, long size, bool columns)
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{
  CylindricalExtension(fs,size,columns);
}

CFuzzySetRelation::CFuzzySetRelation (const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right, ETnormOperations tnorm, long segments)
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{
  Initialize(left,right,tnorm,segments);
}

CFuzzySetRelation::CFuzzySetRelation (const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right, ETconormOperations tconorm, long segments)
  : m_pData(0), m_Tnorm(Min), m_Tconorm(Max)
{
  Initialize(left,right,tconorm,segments);
}


CFuzzySetRelation::~CFuzzySetRelation()
{

}


CFuzzySetRelation & CFuzzySetRelation::operator=(const CFuzzySetRelation &right)
{
  Initialize(right.m_RowHeadings,right.m_ColumnHeadings);

  for (long r=0; r < GetNumRows(); r++)
  {
    for (long c=0; c < GetNumColumns(); c++)
    {
      SetValue(c,r,right.GetValue(c,r));
    }
  }

  return *this;
}


int CFuzzySetRelation::operator==(const CFuzzySetRelation &right) const
{
  assert(0);
  return false;
}

int CFuzzySetRelation::operator!=(const CFuzzySetRelation &right) const
{
  return !(*this == right);
}

bool CFuzzySetRelation::Initialize (const CFuzzySet& left, const CFuzzySet& right)
{
  long NumRows    = left.GetNumFuzzyElements();
  long NumColumns = right.GetNumFuzzyElements();

  Delete();

  // Initialize the matrix for the relation.
  m_pData = new double*[NumColumns];
  if (m_pData == 0)
  {
    return false;
  }
  
  for (long c=0; c < NumColumns; c++)
  {
    m_pData[c] = new double[NumRows];
    if (m_pData[c] == 0)
    {
      return false;
    }
    for (long r=0; r < NumRows; r++)
    {
      m_pData[c][r] = 0.0;
    }
  }

  m_RowHeadings = left;
  m_ColumnHeadings = right;

  return true;
}

bool CFuzzySetRelation::Initialize (const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right, ETnormOperations tnorm, long segments)
{
  if (segments < 2)
  {
    return false;
  }

  CFuzzySet Left,
            Right;

  // Construct the left CFuzzySet.
  long v;
  for (v = 0; v < left.GetNumVertices(); v++)
  {
    CFuzzyElement FE,
                   FEnext;

    if ((left.GetVertex(v,&FE)) && (left.GetVertex(v+1,&FEnext)))
    {
      long IncrementAmount = (FEnext.GetValue() - FE.GetValue()) / segments;
      if (IncrementAmount == 0)
      {
        IncrementAmount = 1;
      }

      // Add the end verticies.
      Left.AddFuzzyElement(FE);
      Left.AddFuzzyElement(FEnext);

      // Add intervening vertex segments.
      for (long i=FE.GetValue() + IncrementAmount; i < FEnext.GetValue(); i+= IncrementAmount)
      {
        FE.SetValue(i);
        FE.SetMembership(left.GetValue(i));
        Left.AddFuzzyElement(FE);
      }
    }
  }//END for (long v=0; v < right.GetNumVertices(); v++)

  // Construct the right CFuzzySet.
  for (v = 0; v < right.GetNumVertices(); v++)
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
      Right.AddFuzzyElement(FE);
      Right.AddFuzzyElement(FEnext);

      // Add intervening vertex segments.
      for (long i=FE.GetValue() + IncrementAmount; i < FEnext.GetValue(); i+= IncrementAmount)
      {
        FE.SetValue(i);
        FE.SetMembership(right.GetValue(i));
        Right.AddFuzzyElement(FE);
      }
    }
  }//END for (long v=0; v < right.GetNumVertices(); v++)

  //Initialize the relation.
  if (Initialize(Left,Right))
  {
    CFuzzyElement FEleft,
                   FEright,
                   FEpair;

    SetTnorm(tnorm);
   
    // Fill the fuzzy relation.
    for (long r=0; r < GetNumRows(); r++)
    {
      Left.GetFuzzyElement(r,&FEleft);
      FEleft.SetTnorm(tnorm);
      for (long c=0; c < GetNumColumns(); c++)
      {
        Right.GetFuzzyElement(c,&FEright);
        FEpair = FEleft && FEright;
        SetValue(c,r,FEpair.GetMembership());
      }
    }
  }

  return true;
}

bool CFuzzySetRelation::Initialize (const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right, ETconormOperations tconorm, long segments)
{
  if (segments < 2)
  {
    return false;
  }

  CFuzzySet Left;
  CFuzzySet Right;

  // Construct the left CFuzzySet.
  long v;
  for (v = 0; v < left.GetNumVertices(); v++)
  {
    CFuzzyElement FE,
                   FEnext;

    if ((left.GetVertex(v,&FE)) && (left.GetVertex(v+1,&FEnext)))
    {
      long IncrementAmount = (FEnext.GetValue() - FE.GetValue()) / segments;
      if (IncrementAmount == 0)
      {
        IncrementAmount = 1;
      }

      // Add the end verticies.
      Left.AddFuzzyElement(FE);
      Left.AddFuzzyElement(FEnext);

      // Add intervening vertex segments.
      for (long i=FE.GetValue() + IncrementAmount; i < FEnext.GetValue(); i+= IncrementAmount)
      {
        FE.SetValue(i);
        FE.SetMembership(left.GetValue(i));
        Left.AddFuzzyElement(FE);
      }
    }
  }//END for (long v=0; v < right.GetNumVertices(); v++)

  // Construct the right CFuzzySet.
  for (v = 0; v < right.GetNumVertices(); v++)
  {
    CFuzzyElement FE;
    CFuzzyElement FEnext;

    if ((right.GetVertex(v,&FE)) && (right.GetVertex(v+1,&FEnext)))
    {
      long IncrementAmount = (FEnext.GetValue() - FE.GetValue()) / segments;
      if (IncrementAmount == 0)
      {
        IncrementAmount = 1;
      }

      // Add the end verticies.
      Right.AddFuzzyElement(FE);
      Right.AddFuzzyElement(FEnext);

      // Add intervening vertex segments.
      for (long i=FE.GetValue() + IncrementAmount; i < FEnext.GetValue(); i+= IncrementAmount)
      {
        FE.SetValue(i);
        FE.SetMembership(right.GetValue(i));
        Right.AddFuzzyElement(FE);
      }
    }
  }//END for (long v=0; v < right.GetNumVertices(); v++)

  //Initialize the relation.
  if (Initialize(Left,Right))
  {
    CFuzzyElement FEleft;
    CFuzzyElement FEright;
    CFuzzyElement FEpair;

    SetTconorm(tconorm);
   
    // Fill the fuzzy relation.
    for (long r=0; r < GetNumRows(); r++)
    {
      Left.GetFuzzyElement(r,&FEleft);
      FEleft.SetTconorm(tconorm);
      for (long c=0; c < GetNumColumns(); c++)
      {
        Right.GetFuzzyElement(c,&FEright);
        FEpair = FEleft || FEright;
        SetValue(c,r,FEpair.GetMembership());
      }
    }
  }

  return true;
}

bool CFuzzySetRelation::CylindricalExtension (const CFuzzySet& fs, long size, bool columns)
{
  if (size > 0)
  {
    CFuzzySet FScylindrical;
    CFuzzyElement FE;

    // Create the extension set for initialization.
    for (long i=0; i < size; i++)
    {
      FE.SetValue(i);
      FScylindrical.AddFuzzyElement(FE);
    }

    // Do exension in specified direction.
    if (columns)
    {
      Initialize(fs,FScylindrical);
    }
    else
    {
      Initialize(FScylindrical,fs);
    }

    return true;
  }

  return false;
}

bool CFuzzySetRelation::Delete ()
{
  long NumColumns = m_ColumnHeadings.GetNumFuzzyElements();

  // If matrix has already been initialized, de-alocate the memory before continuing.
  if (m_pData)
  {
    for (long c = 0; c < NumColumns; c++)
    {
      if (m_pData[c])
      {
        delete[] m_pData[c];
      }
    }
    delete[] m_pData;
    return true;
  }

  return false;
}

long CFuzzySetRelation::GetNumColumns () const
{
  return m_ColumnHeadings.GetNumFuzzyElements();
}

long CFuzzySetRelation::GetNumRows () const
{
  return m_RowHeadings.GetNumFuzzyElements();
}

const CFuzzySet* CFuzzySetRelation::GetpColumnHeadings () const
{
  return &m_ColumnHeadings;
}

const CFuzzySet* CFuzzySetRelation::GetpRowHeadings () const
{
  return &m_RowHeadings;
}

double CFuzzySetRelation::GetValue (long col, long row) const
{
  long NumRows    = m_RowHeadings.GetNumFuzzyElements();
  long NumColumns = m_ColumnHeadings.GetNumFuzzyElements();

  if ((0 <= col) && (col < NumColumns) && (0 <= row) && (row < NumRows))
  {
    return m_pData[col][row];
  }

  return 0.0;
}

bool CFuzzySetRelation::SetValue (long col, long row, double value)
{
  long NumRows    = m_RowHeadings.GetNumFuzzyElements();
  long NumColumns = m_ColumnHeadings.GetNumFuzzyElements();

  if ((0 <= col) && (col < NumColumns) && (0 <= row) && (row < NumRows) && (0.0 <= value) && (value <= 1.0))
  {
    m_pData[col][row] = value;
    return true;
  }

  return false;
}

const CFuzzySetRelation& CFuzzySetRelation::operator % (const CFuzzySetRelation& right)
{
  assert(GetNumColumns() == right.GetNumRows());

  static CFuzzySetRelation FSRreturn;
  FSRreturn.Delete();

  if (GetNumColumns() == right.GetNumRows())
  {
    FSRreturn.Initialize(m_RowHeadings,right.m_ColumnHeadings);
    CFuzzyElement FEleft;
    CFuzzyElement FEright;
    CFuzzyElement FEpair;
    CFuzzyElement FEtotal;

    FEleft.SetTnorm(GetTnorm());
    FEleft.SetTconorm(GetTconorm());

    for (long r=0; r < FSRreturn.GetNumRows(); r++)
    {
      for (long c=0; c < FSRreturn.GetNumColumns(); c++)
      {
        
        bool FirstPass = true;

        // Do total T-norm for pairwise T-conorm.
        for (long i=0; i < GetNumColumns(); i++)
        {
          FEleft.SetMembership(GetValue(i,r));
          FEright.SetMembership(right.GetValue(c,i));

          // Pairwise T-conorm.
          FEpair = FEleft && FEright;

          // Total T-norm.
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
        FSRreturn.SetValue(c,r,FEtotal.GetMembership());
      }//END for (long c=0; c < FSRreturn.GetNumColumns(); c++)
    }//END for (long r=0; r < FSRreturn.GetNumRows(); r++)
  }//END if (GetNumColumns() == right.GetNumRows())

  return FSRreturn;
}

const CFuzzySet& CFuzzySetRelation::operator && (const CFuzzySet& right)
{
  assert(m_ColumnHeadings.GetNumFuzzyElements() == right.GetNumFuzzyElements());

  static CFuzzySet FSreturn;
  FSreturn.DeleteFuzzyElements();

  if (m_ColumnHeadings.GetNumFuzzyElements() != right.GetNumFuzzyElements())
  {
    return FSreturn;
  }

  CFuzzyElement FEleft;
  CFuzzyElement FEright;
  CFuzzyElement FEpair;
  CFuzzyElement FEtotal;

  // Use the relation's T-norm & T-conorm.
  FEleft.SetTnorm(GetTnorm());
  FEleft.SetTconorm(GetTconorm());
  
  for (long r=0; r < m_RowHeadings.GetNumFuzzyElements(); r++)
  {
    bool FirstPass = true;
    
    for (long c=0; c < m_ColumnHeadings.GetNumFuzzyElements(); c++)
    {
      FEleft.SetMembership(GetValue(c,r));
      right.GetFuzzyElement(c,&FEright);

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
    m_RowHeadings.GetFuzzyElement(r,&FEpair);
    FEtotal.SetValue(FEpair.GetValue());

    FSreturn.AddFuzzyElement(FEtotal);
  }
  
  return FSreturn;  
}

