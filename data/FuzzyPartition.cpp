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
#include "FuzzyPartition.h"
#include <cassert>

CFuzzyPartition::CFuzzyPartition()
 : m_pPartitionSpace(0)
{
  m_pLeftFSC = 0;
  m_pLeftFMFC = 0;
  m_pRightFSC = 0;
  m_pRightFMFC = 0;
}

CFuzzyPartition::CFuzzyPartition(const CFuzzyPartition &right)
  : m_pPartitionSpace(0)
{
  assert(0);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzySetCollection& left, const CFuzzySetCollection& right, ETnormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzyMembershipFunctionCollection& left, const CFuzzyMembershipFunctionCollection& right, ETnormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzySetCollection& left, const CFuzzyMembershipFunctionCollection& right, ETnormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzyMembershipFunctionCollection& left, const CFuzzySetCollection& right, ETnormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzySetCollection& left, const CFuzzySetCollection& right, ETconormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzyMembershipFunctionCollection& left, const CFuzzyMembershipFunctionCollection& right, ETconormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzySetCollection& left, const CFuzzyMembershipFunctionCollection& right, ETconormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}

CFuzzyPartition::CFuzzyPartition (const CFuzzyMembershipFunctionCollection& left, const CFuzzySetCollection& right, ETconormOperations operation)
  : m_pPartitionSpace(0)
{
  Initialize(left,right,operation);
}


CFuzzyPartition::~CFuzzyPartition()
{

}


CFuzzyPartition & CFuzzyPartition::operator=(const CFuzzyPartition &right)
{
  assert(0);
  return *this;
}


int CFuzzyPartition::operator==(const CFuzzyPartition &right) const
{
  assert(0);
  return false;
}

int CFuzzyPartition::operator!=(const CFuzzyPartition &right) const
{
  return !(*this == right);
}

bool CFuzzyPartition::Initialize (const CFuzzySetCollection& left, const CFuzzySetCollection& right, ETnormOperations operation)
{
  Delete();

  m_pLeftFSC = new CFuzzySetCollection(left);
  m_pRightFSC = new CFuzzySetCollection(right);

  long Rows    = GetNumRows();
  long Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTnorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(true);
      m_pPartitionSpace[c][r].SetpLeftFS(left.GetpFuzzySet(r));
      m_pPartitionSpace[c][r].SetpRightFS(right.GetpFuzzySet(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzyMembershipFunctionCollection& left, const CFuzzyMembershipFunctionCollection& right, ETnormOperations operation)
{
  Delete();

  m_pLeftFMFC = new CFuzzyMembershipFunctionCollection(left);
  m_pRightFMFC = new CFuzzyMembershipFunctionCollection(right);

  long Rows    = GetNumRows();
  long Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTnorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(true);
      m_pPartitionSpace[c][r].SetpLeftFMF(left.GetpFuzzyMembershipFunction(r));
      m_pPartitionSpace[c][r].SetpRightFMF(right.GetpFuzzyMembershipFunction(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzySetCollection& left, const CFuzzyMembershipFunctionCollection& right, ETnormOperations operation)
{
  Delete();

  m_pLeftFSC = new CFuzzySetCollection(left);
  m_pRightFMFC = new CFuzzyMembershipFunctionCollection(right);

  long Rows    = GetNumRows();
  long Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTnorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(true);
      m_pPartitionSpace[c][r].SetpLeftFS(left.GetpFuzzySet(r));
      m_pPartitionSpace[c][r].SetpRightFMF(right.GetpFuzzyMembershipFunction(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzyMembershipFunctionCollection& left, const CFuzzySetCollection& right, ETnormOperations operation)
{
  Delete();

  m_pLeftFMFC = new CFuzzyMembershipFunctionCollection(left);
  m_pRightFSC = new CFuzzySetCollection(right);

  long Rows    = GetNumRows(),
       Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTnorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(true);
      m_pPartitionSpace[c][r].SetpLeftFMF(left.GetpFuzzyMembershipFunction(r));
      m_pPartitionSpace[c][r].SetpRightFS(right.GetpFuzzySet(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzySetCollection& left, const CFuzzySetCollection& right, ETconormOperations operation)
{
  Delete();

  m_pLeftFSC = new CFuzzySetCollection(left);
  m_pRightFSC = new CFuzzySetCollection(right);

  long Rows    = GetNumRows(),
       Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTconorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(false);
      m_pPartitionSpace[c][r].SetpLeftFS(left.GetpFuzzySet(r));
      m_pPartitionSpace[c][r].SetpRightFS(right.GetpFuzzySet(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzyMembershipFunctionCollection& left, const CFuzzyMembershipFunctionCollection& right, ETconormOperations operation)
{
  Delete();

  m_pLeftFMFC = new CFuzzyMembershipFunctionCollection(left);
  m_pRightFMFC = new CFuzzyMembershipFunctionCollection(right);

  long Rows    = GetNumRows(),
       Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTconorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(false);
      m_pPartitionSpace[c][r].SetpLeftFMF(left.GetpFuzzyMembershipFunction(r));
      m_pPartitionSpace[c][r].SetpRightFMF(right.GetpFuzzyMembershipFunction(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzySetCollection& left, const CFuzzyMembershipFunctionCollection& right, ETconormOperations operation)
{
  Delete();

  m_pLeftFSC = new CFuzzySetCollection(left);
  m_pRightFMFC = new CFuzzyMembershipFunctionCollection(right);

  long Rows    = GetNumRows(),
       Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTconorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(false);
      m_pPartitionSpace[c][r].SetpLeftFS(left.GetpFuzzySet(r));
      m_pPartitionSpace[c][r].SetpRightFMF(right.GetpFuzzyMembershipFunction(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Initialize (const CFuzzyMembershipFunctionCollection& left, const CFuzzySetCollection& right, ETconormOperations operation)
{
  Delete();

  m_pLeftFMFC = new CFuzzyMembershipFunctionCollection(left);
  m_pRightFSC = new CFuzzySetCollection(right);

  long Rows    = GetNumRows(),
       Columns = GetNumColumns();

  m_pPartitionSpace = new CFuzzySubPartition*[Columns];
  for (long c = 0; c < Columns; c++)
  {
    m_pPartitionSpace[c] = new CFuzzySubPartition[Rows];
  }

  for (long r = 0; r < Rows; r++)
  {
    for (c = 0; c < Columns; c++)
    {
      m_pPartitionSpace[c][r].SetTconorm(operation);
      m_pPartitionSpace[c][r].SetUseTnorm(false);
      m_pPartitionSpace[c][r].SetpLeftFMF(left.GetpFuzzyMembershipFunction(r));
      m_pPartitionSpace[c][r].SetpRightFS(right.GetpFuzzySet(c));
    }
  }

  return true;
}

bool CFuzzyPartition::Delete ()
{
  bool Deleted = false;

  long Rows    = GetNumRows();
  long Columns = GetNumColumns();

  // Free up memory used for grid of sub-partitions.
  for (long c = 0; c < Columns; c++)
  {
    delete[] m_pPartitionSpace[c];
  }
  delete [] m_pPartitionSpace;
  m_pPartitionSpace = 0;


  if (m_pLeftFSC)
  {
    delete m_pLeftFSC;
    m_pLeftFSC = 0;
    Deleted = true;
  }

  if (m_pRightFSC)
  {
    delete m_pRightFSC;
    m_pRightFSC = 0;
    Deleted = true;
  }

  if (m_pLeftFMFC)
  {
    delete m_pLeftFMFC;
    m_pLeftFMFC = 0;
    Deleted = true;
  }

  if (m_pRightFMFC)
  {
    delete m_pRightFMFC;
    m_pRightFMFC = 0;
    Deleted = true;
  }

  return Deleted;
}

long CFuzzyPartition::GetNumColumns () const
{
  if (m_pRightFSC)
  {
    return m_pRightFSC->GetNumFuzzySets();
  }

  if (m_pRightFMFC)
  {
    return m_pRightFMFC->GetNumFuzzyMembershipFunctions();
  }

  return 0;
}

long CFuzzyPartition::GetNumRows () const
{
  if (m_pLeftFSC)
  {
    return m_pLeftFSC->GetNumFuzzySets();
  }

  if (m_pLeftFMFC)
  {
    return m_pLeftFMFC->GetNumFuzzyMembershipFunctions();
  }

  return 0;
}

double CFuzzyPartition::GetValue (long col, long row, long x, long y) const
{
  CFuzzySubPartition *pSubPartition = 0;

  if (m_pPartitionSpace)
  {
    if ((0 <= col) && (col < GetNumColumns()))
    {
      if ((0 <= row) && (row < GetNumRows()))
      {
        pSubPartition =  &m_pPartitionSpace[col][row];
        return pSubPartition->GetValue(x,y);
      }
    }
  }

  return 0.0;
}

double CFuzzyPartition::GetValue (long x, long y) const
{
  CFuzzyElement FE;

  for (long r = 0; r < GetNumRows(); r++)
  {
    for (long c = 0; c < GetNumColumns(); c++)
    {
      CFuzzySubPartition *pSubPartition = 0;

      if (m_pPartitionSpace)
      {
        if ((0 <= c) && (c < GetNumColumns()))
        {
          if ((0 <= r) && (r < GetNumRows()))
          {
            pSubPartition =  &m_pPartitionSpace[c][r];     
          }
        }
      }

      if (pSubPartition)
      {
        double Total = pSubPartition->GetValue(x,y) + FE.GetMembership();
        if (Total > 1.0)
        {
          Total = 1.0;
        }

        FE.SetMembership(Total);
      }
    }
  }

  return FE.GetMembership();
}

CFuzzySubPartition* CFuzzyPartition::GetpSubPartition (long col, long row)
{
  if (m_pPartitionSpace)
  {
    if ((0 <= col) && (col < GetNumColumns()))
    {
      if ((0 <= row) && (row < GetNumRows()))
      {
        return &m_pPartitionSpace[col][row];     
      }
    }
  }

  return 0;
}

