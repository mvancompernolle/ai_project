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
#include "MathHelper.h"
#include <cassert>
#include "FuzzyRule.h"

CFuzzyRule* CFuzzyRule::m_pInstance = 0;

CFuzzyRule::CFuzzyRule()
  : m_Tnorm(Min), m_Tconorm(Max), m_Model(Mamdani)
{

}

CFuzzyRule::CFuzzyRule(const CFuzzyRule &right)
  : m_Tnorm(Min), m_Tconorm(Max), m_Model(Mamdani)
{
  assert(0);
}


CFuzzyRule::~CFuzzyRule()
{
  Reset();
}


CFuzzyRule & CFuzzyRule::operator=(const CFuzzyRule &right)
{
  assert(0);
  return *this;
}


int CFuzzyRule::operator==(const CFuzzyRule &right) const
{
  assert(0);
  return false;
}

int CFuzzyRule::operator!=(const CFuzzyRule &right) const
{
  return !(*this == right);
}

bool CFuzzyRule::CombineResults (const CFuzzyElement& result)
{
  T_CFuzzyElement_result::const_iterator ThisVi;
  ThisVi = m_ResultantVariables.find(const_cast<CFuzzyElement*>(&result));

  if (ThisVi == m_ResultantVariables.end())
  {
    // This item has not yet been added.
    CFuzzyElement* pFE = new CFuzzyElement(result);
    m_ResultantVariables.insert(T_CFuzzyElement_result::value_type(const_cast<CFuzzyElement*>(&result),pFE));
    return true;
  }

  CFuzzyElement *pCombinedResult = (*ThisVi).second,
                 FEleft = *pCombinedResult;

  FEleft.SetTconorm(GetTconorm());
  FEleft.SetTnorm(GetTnorm());

  switch (m_Model)
  {
    case Mamdani :
    {
      *pCombinedResult = FEleft || result;

      // Use difference in memberhsip to weight the distance to move between the two different values.
      double MembershipDifference = result.GetMembership() / (FEleft.GetMembership() + result.GetMembership());
      double Distance = double(result.GetValue() - FEleft.GetValue()) * MembershipDifference;
      pCombinedResult->SetValue(FEleft.GetValue() +  RoundNearest(Distance));
    }
    break;

    case SAM :
    {
      double Total = FEleft.GetMembership() + result.GetMembership();
      if (Total > 1.0)
      {
        Total = 1.0;
      }
      pCombinedResult->SetMembership(Total);


      // Use difference in memberhsip to weight the distance to move between the two different values.
      double MembershipDifference = result.GetMembership() / (FEleft.GetMembership() + result.GetMembership());
      double Distance = double(result.GetValue() - FEleft.GetValue()) * MembershipDifference;
      pCombinedResult->SetValue(FEleft.GetValue() +  RoundNearest(Distance));
    }
    break;

    default :
      return false;
    break;
  }

  return true;
}

bool CFuzzyRule::CombineResults (const CFuzzySet& result)
{
  T_CFuzzySet_result::const_iterator ThisVi;
  ThisVi = m_ResultantSets.find(const_cast<CFuzzySet*>(&result));

  if (ThisVi == m_ResultantSets.end())
  {
    // This item has not yet been added.
    CFuzzySet* pFS = new CFuzzySet(result);
    m_ResultantSets.insert(T_CFuzzySet_result::value_type(const_cast<CFuzzySet*>(&result),pFS));
    return true;
  }

  CFuzzySet *pCombinedResult = (*ThisVi).second,
             FSleft = *pCombinedResult;

  FSleft.SetTconorm(GetTconorm());
  FSleft.SetTnorm(GetTnorm());

  switch (m_Model)
  {
    case Mamdani :
    {
      *pCombinedResult = FSleft || result;
    }
    break;

    case SAM :
    {
      pCombinedResult->DeleteFuzzyElements();
      CFuzzyElement FEleft;
      CFuzzyElement *pFEright;
      CFuzzyElement FE;

      long l;
      for (l = 0; l < FSleft.GetNumFuzzyElements(); l++)
      {
        if (FSleft.GetFuzzyElement(l,&FEleft))
        {
          // Check for item in right side of list.
          pFEright = result.GetpFuzzyElement(FEleft.GetValue());
          if (pFEright)
          {
            // Variable is on right side so add two together and insert into the combined result. 
            double Total = FEleft.GetMembership() + pFEright->GetMembership();
            if (Total > 1.0)
            {
              Total = 1.0;
            }

            FE.SetMembership(Total);
            FE.SetValue(FEleft.GetValue());
            pCombinedResult->AddFuzzyElement(FE);
          }
          else
          {
            // Variable is not on right so just insert it into combinded result.
            pCombinedResult->AddFuzzyElement(FEleft);
          }
        }
      }//END for (long l = 0; l < FSleft.GetNumFuzzyElements(); l++)

      for (long r = 0; r < result.GetNumFuzzyElements(); r++)
      {
        if (result.GetFuzzyElement(r,&FE))
        {
          if (!FSleft.GetpFuzzyElement(FE.GetValue()))
          {
            // The variable has not yet been inserted into the combined result.
            pCombinedResult->AddFuzzyElement(FE);
          }
        }
      }//END for (long r = 0; r < result.GetNumFuzzyElements(); r++)
    }//END case SAM :
    break;

    default :
      return false;
    break;
  }//END switch (m_Model)

  return true;
}

bool CFuzzyRule::CombineResults (const CFuzzyMembershipFunction& result)
{
  T_CFuzzyMembershipFunction_result::const_iterator ThisVi;
  ThisVi = m_ResultantMembershipFunctions.find(const_cast<CFuzzyMembershipFunction*>(&result));

  if (ThisVi == m_ResultantMembershipFunctions.end())
  {
    // This item has not yet been added.
    CFuzzyMembershipFunction* pFMF = new CFuzzyMembershipFunction(result);
    m_ResultantMembershipFunctions.insert(T_CFuzzyMembershipFunction_result::value_type(const_cast<CFuzzyMembershipFunction*>(&result),pFMF));
    return true;
  }

  CFuzzyMembershipFunction *pCombinedResult = (*ThisVi).second,
                 FMFleft = *pCombinedResult;

  FMFleft.SetTconorm(GetTconorm());
  FMFleft.SetTnorm(GetTnorm());

  switch (m_Model)
  {
    case Mamdani :
    {
      *pCombinedResult = FMFleft || result;
    }
    break;

    case SAM :
    {
      pCombinedResult->DeleteVertices();

      CFuzzyElement FEleft;
      CFuzzyElement FElastLeft;
      CFuzzyElement FEright;
      CFuzzyElement FElastRight;
      CFuzzyElement FEMid;

      long LeftIndex  = 0;
      long RightIndex = 0;
    
      bool LeftDone   = false;
      bool RightDone  = false;
    
      bool FirstPass  = true;
    
      // Get start of each side.
      while ((!LeftDone) || (!RightDone))
      {
        // Get new values;
        if (FMFleft.GetVertex(LeftIndex,&FEleft) && result.GetVertex(RightIndex,&FEright))
        {
          // Set lastLeft and lastRight on first pass.
          if (FirstPass)
          {
            FElastLeft = FEleft;
            FElastRight = FEright;
            FirstPass = false;
          }
    
          // Left is less than right.
          if (FEleft.GetValue() < FEright.GetValue())
          {
            if (FEleft.GetValue() < FElastRight.GetValue())
            {
              if (!LeftDone) //  Proccess left point.
              {
                // No match insert into combined result.
                pCombinedResult->AddVertex(FEleft);
    
                // Advance left point.
                LeftIndex++;
                if (LeftIndex == FMFleft.GetNumVertices())
                {
                  LeftIndex--;
                  LeftDone = true;
                }
                else
                {
                  FElastLeft = FEleft;
                }
              }
              else            // Process right point.
              {
                // No match, insert combined result into list.
                pCombinedResult->AddVertex(FEright);
    
                // Advance points.
                if (!RightDone)
                {
                  RightIndex++;
                  if (RightIndex == result.GetNumVertices())
                  {
                    RightIndex--;
                    RightDone = true;
                  }
                  else
                  {
                    FElastRight = FEright;
                  }
                }
              }//END if (!LeftDone)
            }
            else // if (FEleft.GetValue() < FElastRight.GetValue())
            {
              // Check for line segment intersection, if intersection insert into combined result.
              if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
              {
                double Total = FMFleft.GetValue(FEMid.GetValue()) + result.GetValue(FEMid.GetValue());
                if (Total > 1.0)
                {
                  Total = 1.0;
                }
          
                FEMid.SetMembership(Total);
                if (!pCombinedResult->AddVertex(FEMid))
                {
                  // Overlaping vertices due to roundoff error, use the larger one.
                  CFuzzyElement *pFE = pCombinedResult->GetpVertex(FEMid.GetValue());
                  if (pFE)
                  {
                    CFuzzyElement FEwork;
                    FEwork.SetTconorm(Max);
                    FEwork = *pFE || FEMid ;
                  }
                }
              }
    
              // Get inbetween value for FEleft.
              FEMid = FEleft;
              FEMid.SetMembership(result.GetValue(FEMid.GetValue()));
    
              // Calculate the total for FEleft and right's line segment and insert into combined result.
              double Total = FEleft.GetMembership() + FEMid.GetMembership();
              if (Total > 1.0)
              {
                Total = 1.0;
              }

              FEMid.SetMembership(Total);
              pCombinedResult->AddVertex(FEMid);
      
              // Advance points.
              if (!LeftDone)
              {
                LeftIndex++;
                if (LeftIndex == FMFleft.GetNumVertices())
                {
                  LeftIndex--;
                  LeftDone = true;
                }
                else
                {
                  FElastLeft = FEleft;
                }
              }
    
              if (LeftDone)
              {
                // Process right point before it is skipped.
                if (!RightDone)
                {
                  // No match, insert result into combined results.
                  pCombinedResult->AddVertex(FEright);
    
                  RightIndex++;
                  if (RightIndex == result.GetNumVertices())
                  {
                    RightIndex--;
                    RightDone = true;
                  }
                  else
                  {
                    FElastRight = FEright;
                  }
                }
              }//END if (!LeftDone)
            }//END if (FEleft.GetValue() < FElastRight.GetValue()) 
          }//END if (FEleft.GetValue() < FEright.GetValue())
    
          // Right is equal to left.
          if (FEleft.GetValue() == FEright.GetValue())
          {
            // Check for line segment intersection, if intersection insert into combined result.
            if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
            {
              double Total = FMFleft.GetValue(FEMid.GetValue()) + result.GetValue(FEMid.GetValue());
              if (Total > 1.0)
              {
                Total = 1.0;
              }
         
              FEMid.SetMembership(Total);
              if (!pCombinedResult->AddVertex(FEMid))
              {
                // Overlaping vertices due to roundoff error, use the larger one.
                CFuzzyElement *pFE = pCombinedResult->GetpVertex(FEMid.GetValue());
                if (pFE)
                {
                  CFuzzyElement FEwork;
                  FEwork.SetTconorm(Max);
                  FEwork = *pFE || FEMid ;
                }
              }
            }
      
            // Calculate the total for FEleft and FEright and insert into combined result.
            double Total = FEleft.GetMembership() + FEright.GetMembership();
            if (Total > 1.0)
            {
              Total = 1.0;
            }

            FEleft.SetMembership(Total);
            pCombinedResult->AddVertex(FEleft);
    
            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == FMFleft.GetNumVertices())
              {
                LeftIndex--;
                LeftDone = true;
              }
              else
              {
                FElastLeft = FEleft;
              }
            }
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == result.GetNumVertices())
              {
                RightIndex--;
                RightDone = true;
              }
              else
              {
                FElastRight = FEright;
              }
            }
          }//END if (FEleft.GetValue() == FEright.GetValue())
    
          // Right is less than left.
          if (FEright.GetValue() < FEleft.GetValue())
          {
            if (FEright.GetValue() < FElastLeft.GetValue())
            {
              if (!RightDone) // Process right point.
              {
                // No match, insert result into combined result.
                pCombinedResult->AddVertex(FEright);
    
                // Advance right point.
                RightIndex++;
                if (RightIndex == result.GetNumVertices())
                {
                  RightIndex--;
                  RightDone = true;
                }
                else
                {
                  FElastRight = FEright;
                }
              }
              else  // Process left point.
              {
                // No match, insert into combined result.
                pCombinedResult->AddVertex(FEleft);
    
                // Advance points.
                if (!LeftDone)
                {
                  LeftIndex++;
                  if (LeftIndex == FMFleft.GetNumVertices())
                  {
                    LeftIndex--;
                    LeftDone = true;
                  }
                  else
                  {
                    FElastLeft = FEleft;
                  }
                }
              }//END if (!RightDone)
            }
            else // if (FEright.GetValue() < FElastLeft.GetValue())
            {
              // Check for line segment intersection, if intersection insert into combined result.
              if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
              {
                double Total = FMFleft.GetValue(FEMid.GetValue()) + result.GetValue(FEMid.GetValue());
                if (Total > 1.0)
                {
                  Total = 1.0;
                }
          
                FEMid.SetMembership(Total);
                if (!pCombinedResult->AddVertex(FEMid))
                {
                  // Overlaping vertices due to roundoff error, use the larger one.
                  CFuzzyElement *pFE = pCombinedResult->GetpVertex(FEMid.GetValue());
                  if (pFE)
                  {
                    CFuzzyElement FEwork;
                    FEwork.SetTconorm(Max);
                    FEwork = *pFE || FEMid ;
                  }
                }
              }
    
              // Get inbetween value for FEleft.
              FEMid = FEright;
              FEMid.SetMembership(FMFleft.GetValue(FEMid.GetValue()));
    
              // Calculate the total for FEright and left's line segment and insert into combined list.
              double Total = FEright.GetMembership() + FEMid.GetMembership();
              if (Total > 1.0)
              {
                Total = 1.0;
              }

              FEMid.SetMembership(Total);
              pCombinedResult->AddVertex(FEMid);
    
              // Advance points.
              if (!RightDone)
              {
                RightIndex++;
                if (RightIndex == result.GetNumVertices())
                {
                  RightIndex--;
                  RightDone = true;
                }
                else
                {
                  FElastRight = FEright;
                }
              }
    
              if (RightDone)
              {
                // Process left pont before it gets skipped.
                if (!LeftDone)
                {
                  // No match, insert into combined result.
                  pCombinedResult->AddVertex(FEleft);
  
                  LeftIndex++;
                  if (LeftIndex == FMFleft.GetNumVertices())
                  {
                    LeftIndex--;
                    LeftDone = true;
                  }
                  else
                  {
                    FElastLeft = FEleft;
                  }
                }
              }//END if (!LeftDone)
            }//END if (FEright.GetValue() < FElastLeft.GetValue())
          }//END if (FEright.GetValue() < FEleft.GetValue())
        }//END if (FMFleft.GetVertex(LeftIndex,&FEleft) && result.GetVertex(RightIndex,&FEright))
      }//END while ((LeftIndex < FMFleft.GetNumVertices()) && (RightIndex < result.GetNumVertices()))
    }//END case SAM :
    break;

    default :
      assert(0);
      return false;
    break;
  }

  return true;
}

bool CFuzzyRule::GetCombinedResults (const CFuzzyElement* pResultant, CFuzzyElement* pCombinedResult)
{
  T_CFuzzyElement_result::const_iterator ThisVi;
  ThisVi = m_ResultantVariables.find(const_cast<CFuzzyElement*>(pResultant));

  if (ThisVi == m_ResultantVariables.end())
  {
    return false;
  }

  *pCombinedResult = *(*ThisVi).second;
  return true;
}

bool CFuzzyRule::GetCombinedResults (const CFuzzySet* pResultant, CFuzzySet* pCombinedResult)
{
  T_CFuzzySet_result::const_iterator ThisVi;
  ThisVi = m_ResultantSets.find(const_cast<CFuzzySet*>(pResultant));

  if (ThisVi == m_ResultantSets.end())
  {
    return false;
  }

  *pCombinedResult = *(*ThisVi).second;
  return true;
}

bool CFuzzyRule::GetCombinedResults (const CFuzzyMembershipFunction* pResultant, CFuzzyMembershipFunction* pCombinedResult)
{
  T_CFuzzyMembershipFunction_result::const_iterator ThisVi;
  ThisVi = m_ResultantMembershipFunctions.find(const_cast<CFuzzyMembershipFunction*>(pResultant));

  if (ThisVi == m_ResultantMembershipFunctions.end())
  {
    return false;
  }

  *pCombinedResult = *(*ThisVi).second;
  return true;
}

CFuzzyRule* CFuzzyRule::GetpInstance ()
{
  if (!m_pInstance)
  {
    m_pInstance = new CFuzzyRule;
  }
  
  return m_pInstance;
}

void CFuzzyRule::Disable ()
{
  delete m_pInstance;
  m_pInstance = 0;
}

bool CFuzzyRule::IsEnabled ()
{
  if (m_pInstance)
  {
    return true;
  }

  return false;
}

void CFuzzyRule::Reset ()
{
  // De-allocate the list of fuzzy variables.
  T_CFuzzyElement_result::iterator Vi;
  for (Vi = m_ResultantVariables.begin(); Vi != m_ResultantVariables.end(); Vi++)
  {
    CFuzzyElement *pFE = (*Vi).second;
    delete pFE;
  }
  m_ResultantVariables.clear();
   
  // De-allocate the list of fuzzy sets.
  T_CFuzzySet_result::iterator Si;
  for (Si = m_ResultantSets.begin(); Si != m_ResultantSets.end(); Si++)
  {
    CFuzzySet *pFS = (*Si).second;
    delete pFS;
  }
  m_ResultantSets.clear();

  // De-allocate the list of fuzzy membership functions.
  T_CFuzzyMembershipFunction_result::iterator Fi;
  for (Fi = m_ResultantMembershipFunctions.begin(); Fi != m_ResultantMembershipFunctions.end(); Fi++)
  {
    CFuzzyMembershipFunction *pFMF = (*Fi).second;
    delete pFMF;
  }
  m_ResultantMembershipFunctions.clear();
}

