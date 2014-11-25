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
#include "FuzzyElement.h"
#include "FuzzyMembershipFunction.h"
#include <LIMITS.H>
#include <cassert>
#include <cmath>

#define LEASTVALUE  LONG_MIN

bool F_INTERSECTION_(const CFuzzyElement& Fv1, const CFuzzyElement& Fv2, const CFuzzyElement& Fv3, const CFuzzyElement& Fv4, CFuzzyElement* pFv);

CFuzzyOperator::CFuzzyOperator()
{

}

CFuzzyOperator::CFuzzyOperator(const CFuzzyOperator &right)

{
   assert(0);
}


CFuzzyOperator::~CFuzzyOperator()
{

}


CFuzzyOperator & CFuzzyOperator::operator=(const CFuzzyOperator &right)
{
   assert(0);
   return *this;
}


int CFuzzyOperator::operator==(const CFuzzyOperator &right) const
{
   assert(0);
   return 0;
}

int CFuzzyOperator::operator!=(const CFuzzyOperator &right) const
{
   return !(*this == right);
}

void CFuzzyOperator::Minimum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  *result = left;
  if (left.GetMembership() < right.GetMembership())
  {
    result->SetMembership(left.GetMembership());
  }
  else
  {
    result->SetMembership(right.GetMembership());
  }
}

void CFuzzyOperator::Minimum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

   CFuzzySet      FSwork;
   CFuzzyElement *pFEreturn;
   CFuzzyElement *pFEwork;
   CFuzzyElement  FE;

   // Initialize.
   *result = left;
   FSwork  = right;

   // Do Minimum operation for all variables on left side.
   long i=0;
   while (i < result->GetNumFuzzyElements())
   {
      if (result->GetFuzzyElement(i,&FE))
      {
         pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
         if (pFEwork)
         {
            // The fuzzy variable exists in both sets, do Minimum of each.
            pFEreturn = result->GetpFuzzyElement(FE.GetValue());
            Minimum(pFEreturn, *pFEreturn, *pFEwork);

            // After processing each right side fuzzy variable it needs to be removed.
            FSwork.DeleteFuzzyElement(pFEwork->GetValue());
            i++;
         }
         else
         {
            // The fuzzy variable doesn't exist on the right.
            pFEreturn = result->GetpFuzzyElement(FE.GetValue());
            Minimum(pFEreturn, *pFEreturn, CFuzzyElement(0.0,0,FE.GetValue()));

            // Any zero membership values are not returned.
            if (pFEreturn->GetMembership() == 0.0)
            {
               pFEreturn = result->GetpFuzzyElement(FE.GetValue());
               result->DeleteFuzzyElement(pFEreturn->GetValue());
            }
            else
            {
               i++;
            }
         }//END if (pFEwork)
      }//END if (result->GetFuzzyElement(i,&FE))
   }//END for (long i=0; i < result->GetNumFuzzyElements(); i++)

   // Do Minimum operation for remaining variables on right side.
   for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
   {
      if (FSwork.GetFuzzyElement(i,&FE))
      {
         Minimum(&FE, CFuzzyElement(0.0,0,FE.GetValue()), CFuzzyElement(FE));

         // Zero membership values are not returned.
         if (FE.GetMembership() > 0.0)
         {
            result->AddFuzzyElement(FE);
         }
      }//END if (FSwork.GetFuzzyElement(i,&FE)
   }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::Minimum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do Minimum with zero.
            Minimum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do Minimum with zero.
            Minimum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do Minimum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            Minimum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the Minimum for FEleft and right's line segment and add to the return FMF.
          Minimum(&FE, FEleft, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do Minimum with zero.
              Minimum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do Minimum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          Minimum(&FE, FEMid, FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the Minimum for FEleft and FEright and add to FMF.
        Minimum(&FE, FEleft, FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do Minimum with zero.
            Minimum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            Minimum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do Minimum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            Minimum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the Minimum for FEright and left's line segment and add to the return FMF.
          Minimum(&FE, FEright, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do Minimum with zero.
              Minimum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::Maximum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  *result = left;
  if (left.GetMembership() > right.GetMembership())
  {
    result->SetMembership(left.GetMembership());
  }
  else
  {
    result->SetMembership(right.GetMembership());
  }
}

void CFuzzyOperator::Maximum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
  

   CFuzzySet      FSwork;
   CFuzzyElement *pFEreturn;
   CFuzzyElement *pFEwork;
   CFuzzyElement  FE;

   // Initialize.
   *result = left;
   FSwork   = right;

   // Do Maximum operation for all variables on left side.
   long i=0;
   while (i < result->GetNumFuzzyElements())
   {
      if (result->GetFuzzyElement(i,&FE))
      {
         pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
         if (pFEwork)
         {
            // The fuzzy variable exists in both sets, do Maximum of each.
            pFEreturn = result->GetpFuzzyElement(FE.GetValue());
            Maximum(pFEreturn, FE, *pFEwork);

            // After processing each right side fuzzy variable it needs to be removed.
            FSwork.DeleteFuzzyElement(pFEwork->GetValue());
            i++;
         }
         else
         {
            // The fuzzy variable doesn't exist on the right.
            pFEreturn = result->GetpFuzzyElement(FE.GetValue());
            Maximum(pFEreturn, FE, CFuzzyElement(0.0,0,FE.GetValue()));

            // Any zero membership values are not returned.
            if (pFEreturn->GetMembership() == 0.0)
            {
               pFEreturn = result->GetpFuzzyElement(FE.GetValue());
               result->DeleteFuzzyElement(pFEreturn->GetValue());
            }
            else
            {
               i++;
            }
         }//END if (pFEwork)
      }//END if (result->GetFuzzyElement(i,&FE))
   }//END while (i < result->GetNumFuzzyElements())

   // Do Maximum operation for remaining variables on right side.
   CFuzzyElement FEwork;
   for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
   {
      if (FSwork.GetFuzzyElement(i,&FEwork))
      {
         Maximum(&FE, CFuzzyElement(0.0,0,FEwork.GetValue()), CFuzzyElement(FEwork));

         // Zero membership values are not returned.
         if (FE.GetMembership() > 0.0)
         {
            result->AddFuzzyElement(FE);
         }
      }//END if (FSwork.GetFuzzyElement(i,&FE)
   }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::Maximum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  //## begin CFuzzyOperator::Maximum%3BD37CD70356.body preserve=yes
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do Maximum with zero.
            Maximum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do Maximum with zero.
            Maximum(&FE, FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do Maximum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            Maximum(&FE, FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the Maximum for FEleft and right's line segment and add to the return FMF.
          Maximum(&FE, FEleft, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do Maximum with zero.
              Maximum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do Maximum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          Maximum(&FE, FEMid, FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the Maximum for FEleft and FEright and add to FMF.
        Maximum(&FE, FEleft, FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do Maximum with zero.
            Maximum(&FE, FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            Maximum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do Maximum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            Maximum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the Maximum for FEright and left's line segment and add to the return FMF.
          Maximum(&FE, FEright, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do Maximum with zero.
              Maximum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::DrasticProduct (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  if ((left.GetMembership() < 1.0) && (right.GetMembership() < 1.0))
  {
    *result = left;
    result->SetMembership(0.0);
  }
  
  Minimum(result, left, right);
}

void CFuzzyOperator::DrasticProduct (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
  
  
  CFuzzySet      FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;
  
  // Initialize.
  *result = left;
  FSwork  = right;
  
  // Do DrasticProduct operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do DrasticProduct of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        DrasticProduct(pFEreturn, *pFEreturn, *pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        DrasticProduct(pFEreturn, *pFEreturn, CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do DrasticProduct operation for remaining variables on right side.
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FE))
    {
      DrasticProduct(&FE, CFuzzyElement(0.0,0,FE.GetValue()),CFuzzyElement(FE));
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::DrasticProduct (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);
  
  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do DrasticProduct with zero.
            DrasticProduct(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do DrasticProduct with zero.
            DrasticProduct(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do DrasticProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            DrasticProduct(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the DrasticProduct for FEleft and right's line segment and add to the return FMF.
          DrasticProduct(&FE, FEleft, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do DrasticProduct with zero.
              DrasticProduct(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do DrasticProduct.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          DrasticProduct(&FE, FEMid, FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the DrasticProduct for FEleft and FEright and add to FMF.
        DrasticProduct(&FE, FEleft, FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do DrasticProduct with zero.
            DrasticProduct(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            DrasticProduct(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do DrasticProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            DrasticProduct(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the DrasticProduct for FEright and left's line segment and add to the return FMF.
          DrasticProduct(&FE, FEright, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do DrasticProduct with zero.
              DrasticProduct(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::DrasticSum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  if ((left.GetMembership() > 0.0) && (right.GetMembership() > 0.0))
  {
    *result = left;
    result->SetMembership(1.0);
  }
  
  Maximum(result, left, right);
}

void CFuzzyOperator::DrasticSum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzySet      FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;
  
  // Initialize.
  *result = left;
  FSwork   = right;
  
  // Do DrasticSum operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do DrasticSum of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        DrasticSum(pFEreturn, *pFEreturn, *pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        DrasticSum(pFEreturn, *pFEreturn, CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do DrasticSum operation for remaining variables on right side.
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FE))
    {
      DrasticSum(&FE, CFuzzyElement(0.0,0,FE.GetValue()), CFuzzyElement(FE));
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::DrasticSum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do DrasticSum with zero.
            DrasticSum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do DrasticSum with zero.
            DrasticSum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do DrasticSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            DrasticSum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the DrasticSum for FEleft and right's line segment and add to the return FMF.
          DrasticSum(&FE, FEleft, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do DrasticSum with zero.
              DrasticSum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do DrasticSum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          DrasticSum(&FE, FEMid, FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the DrasticSum for FEleft and FEright and add to FMF.
        DrasticSum(&FE, FEleft, FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do DrasticSum with zero.
            DrasticSum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            DrasticSum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do DrasticSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            DrasticSum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the DrasticSum for FEright and left's line segment and add to the return FMF.
          DrasticSum(&FE, FEright, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do DrasticSum with zero.
              DrasticSum(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::BoundedDifference (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);
 
  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = (X + Y - 1.0);
  
  *result = left;
  if (Value > 0.0)
  {
    result->SetMembership(Value);
  }
  else
  {
    result->SetMembership(0.0);
  }
}

void CFuzzyOperator::BoundedDifference (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
    
  // Initialize.
  *result          = left;
  CFuzzySet FSwork = right;

  CFuzzyElement *pFEleft;
  CFuzzyElement *pFEright;
  CFuzzyElement  FE;
  CFuzzyElement  FEresult;
  
  // Do BoundedDifference operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEright = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEright)
      {
        // The fuzzy variable exists in both sets, do BoundedDifference of each.
        pFEleft = result->GetpFuzzyElement(FE.GetValue());
        BoundedDifference(&FEresult,*pFEleft,*pFEright);
        *pFEleft = FEresult;
        
        // Any zero membership values are not returned.
        if (pFEleft->GetMembership() == 0.0)
        {
          //               FEresult = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEleft->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEright->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEleft = result->GetpFuzzyElement(FE.GetValue());
        BoundedDifference(&FEresult, *pFEleft, CFuzzyElement(0.0,0,FE.GetValue()));
        *pFEleft = FEresult;
        
        // Any zero membership values are not returned.
        if (pFEleft->GetMembership() == 0.0)
        {
          //               pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEleft->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do BoundedDifference operation for remaining variables on right side.
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FE))
    {
      BoundedDifference(&FEresult, CFuzzyElement(0.0,0,FE.GetValue()), FE);
      
      // Zero membership values are not returned.
      if (FEresult.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FEresult);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::BoundedDifference (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do BoundedDifference with zero.
            BoundedDifference(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do BoundedDifference with zero.
            BoundedDifference(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do BoundedDifference.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            BoundedDifference(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the BoundedDifference for FEleft and right's line segment and add to the return FMF.
          BoundedDifference(&FE, FEleft, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do BoundedDifference with zero.
              BoundedDifference(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do BoundedDifference.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          BoundedDifference(&FE, FEMid, FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the BoundedDifference for FEleft and FEright and add to FMF.
        BoundedDifference(&FE, FEleft, FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do BoundedDifference with zero.
            BoundedDifference(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            BoundedDifference(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do BoundedDifference.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            BoundedDifference(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the BoundedDifference for FEright and left's line segment and add to the return FMF.
          BoundedDifference(&FE, FEright, FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do BoundedDifference with zero.
              BoundedDifference(&FE, FEleft, CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::BoundedSum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = (X + Y);
  
  *result = left;
  if (Value < 1.0)
  {
    result->SetMembership(Value);
  }
  else
  {
    result->SetMembership(1.0);
  }
}

void CFuzzyOperator::BoundedSum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement *pFEright;
  CFuzzyElement *pFEleft;
  CFuzzyElement  FE;
  CFuzzyElement  FEresult;

  // Initialize.
  *result          = left;
  CFuzzySet FSwork = right;
  
  // Do BoundedSum operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEright = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEright)
      {
        // The fuzzy variable exists in both sets, do BoundedSum of each.
        pFEleft = result->GetpFuzzyElement(FE.GetValue());
        BoundedSum(&FEresult, *pFEleft, *pFEright);
        *pFEleft = FEresult;
        
        // Any zero membership values are not returned.
        if (pFEleft->GetMembership() == 0.0)
        {
//        pFEleft = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEleft->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEright->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEleft = result->GetpFuzzyElement(FE.GetValue());
        BoundedSum(&FEresult, *pFEleft, CFuzzyElement(0.0,0,FE.GetValue()));
        *pFEleft = FEresult;
        
        // Any zero membership values are not returned.
        if (pFEleft->GetMembership() == 0.0)
        {
//        pFEleft = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEleft->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do BoundedSum operation for remaining variables on right side.
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FE))
    {
      BoundedSum(&FEresult, CFuzzyElement(0.0,0,FE.GetValue()), FE);
      
      // Zero membership values are not returned.
      if (FEresult.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FEresult);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::BoundedSum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);
  

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do BoundedSum with zero.
            BoundedSum(&FE, FEleft, CFuzzyElement(0.0,0, FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do BoundedSum with zero.
            BoundedSum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do BoundedSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            BoundedSum(&FE, FEMid, FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the BoundedSum for FEleft and right's line segment and add to the return FMF.
          BoundedSum(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do BoundedSum with zero.
              BoundedSum(&FE, FEright, CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do BoundedSum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          BoundedSum(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the BoundedSum for FEleft and FEright and add to FMF.
        BoundedSum(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do BoundedSum with zero.
            BoundedSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            BoundedSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do BoundedSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            BoundedSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the BoundedSum for FEright and left's line segment and add to the return FMF.
          BoundedSum(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do BoundedSum with zero.
              BoundedSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::EinsteinProduct (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = (X * Y) / (2.0 - (X + Y - (X * Y)));
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::EinsteinProduct (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzySet FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;

  // Initialize.
  *result = left;
  FSwork   = right;
  
  // Do EinsteinProduct operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do EinsteinProduct of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        EinsteinProduct(pFEreturn,FE,*pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        EinsteinProduct(pFEreturn, FE,CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do EinstienProduct operation for remaining variables on right side.
  CFuzzyElement FEwork;
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FEwork))
    {
      EinsteinProduct(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::EinsteinProduct (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do EinsteinProduct with zero.
            EinsteinProduct(&FE, FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do EinsteinProduct with zero.
            EinsteinProduct(&FE, FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do EinsteinProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            EinsteinProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the EinsteinProduct for FEleft and right's line segment and add to the return FMF.
          EinsteinProduct(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do EinstienProduct with zero.
              EinsteinProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do EinsteinProduct.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          EinsteinProduct(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the EinsteinProduct for FEleft and FEright and add to FMF.
        EinsteinProduct(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do EisnteinProduct with zero.
            EinsteinProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            EinsteinProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do EinsteinProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            EinsteinProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the EinsteinProduct for FEright and left's line segment and add to the return FMF.
          EinsteinProduct(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do EinsteinProduct with zero.
              EinsteinProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::EinsteinSum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);
  
  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = (X + Y) / (1.0 + (X * Y));
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::EinsteinSum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzySet FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;

   // Initialize.
   *result = left;
   FSwork  = right;

   // Do EinstienSum operation for all variables on left side.
   long i=0;
   while (i < result->GetNumFuzzyElements())
   {
     if (result->GetFuzzyElement(i,&FE))
     {
       pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
       if (pFEwork)
       {
         // The fuzzy variable exists in both sets, do EinsteinSum of each.
         pFEreturn = result->GetpFuzzyElement(FE.GetValue());
         EinsteinSum(pFEreturn,FE,*pFEwork);
         
         // Any zero membership values are not returned.
         if (pFEreturn->GetMembership() == 0.0)
         {
           pFEreturn = result->GetpFuzzyElement(FE.GetValue());
           result->DeleteFuzzyElement(pFEreturn->GetValue());
         }
         else
         {
           i++;
         }
         
         // After processing each right side fuzzy variable it needs to be removed.
         FSwork.DeleteFuzzyElement(pFEwork->GetValue());
       }
       else
       {
         // The fuzzy variable doesn't exist on the right.
         pFEreturn = result->GetpFuzzyElement(FE.GetValue());
         EinsteinSum(pFEreturn,FE,CFuzzyElement(0.0,0,FE.GetValue()));
         
         // Any zero membership values are not returned.
         if (pFEreturn->GetMembership() == 0.0)
         {
           pFEreturn = result->GetpFuzzyElement(FE.GetValue());
           result->DeleteFuzzyElement(pFEreturn->GetValue());
         }
         else
         {
           i++;
         }
       }//END if (pFEwork)
     }//END if (result->GetFuzzyElement(i,&FE))
   }//END while (i < result->GetNumFuzzyElements())
   
   // Do EinsteinSum operation for remaining variables on right side.
   CFuzzyElement FEwork;
   for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
   {
     if (FSwork.GetFuzzyElement(i,&FEwork))
     {
       EinsteinSum(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
       
       // Zero membership values are not returned.
       if (FE.GetMembership() > 0.0)
       {
         result->AddFuzzyElement(FE);
       }
     }//END if (FSwork.GetFuzzyElement(i,&FE)
   }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::EinsteinSum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do EinsteinSum with zero.
            EinsteinSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do EinsteinSum with zero.
            EinsteinSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do EinsteinSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            EinsteinSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the EinsteinSum for FEleft and right's line segment and add to the return FMF.
          EinsteinSum(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do EinsteinSum with zero.
              EinsteinSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do EinsteinSum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          EinsteinSum(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the EinsteinSum for FEleft and FEright and add to FMF.
        EinsteinSum(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do EinsteinSum with zero.
            EinsteinSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            EinsteinSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do EinsteinSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            EinsteinSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the EinsteinSum for FEright and left's line segment and add to the return FMF.
          EinsteinSum(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do EinsteinSum with zero.
              EinsteinSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::AlgabraicProduct (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);

  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = X * Y;
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::AlgabraicProduct (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
  
  CFuzzySet FSwork;
  CFuzzySet Left = left;
  CFuzzySet Right = right;
  
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;
  
  // Initialize.
  *result = left;
  FSwork  = right;
  
  // Do AlgabraicProduct operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do AlgabraicProduct of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        AlgabraicProduct(pFEreturn,FE,*pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        AlgabraicProduct(pFEreturn,FE,CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do AlgabraicProduct operation for remaining variables on right side.
  CFuzzyElement FEwork;
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FEwork))
    {
      AlgabraicProduct(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::AlgabraicProduct (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);
  

  CFuzzyMembershipFunction Left = left;
  CFuzzyMembershipFunction Right = right;


  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0;
  long RightIndex = 0;

  bool LeftDone   = false;
  bool RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (Left.GetVertex(LeftIndex,&FEleft) && Right.GetVertex(RightIndex,&FEright))
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
            // No match.. do AlgabraicProduct with zero.
            AlgabraicProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == Left.GetNumVertices())
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
            // No match.. do AlgabraicProduct with zero.
            AlgabraicProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == Right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do AlgabaraicProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            AlgabraicProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the AlgabraicProduct for FEleft and right's line segment and add to the return FMF.
          AlgabraicProduct(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex ==Left.GetNumVertices())
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
              // No match.. do AlgabraicProduct with zero.
              AlgabraicProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == Right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do AlgabraicProduct.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          AlgabraicProduct(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the AlgabraicProduct for FEleft and FEright and add to FMF.
        AlgabraicProduct(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == Left.GetNumVertices())
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
          if (RightIndex == Right.GetNumVertices())
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
            // No match.. do AlgabraicProduct with zero.
            AlgabraicProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == Right.GetNumVertices())
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
            AlgabraicProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == Left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do AlgabraicProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            AlgabraicProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the AlgabraicProduct for FEright and left's line segment and add to the return FMF.
          AlgabraicProduct(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == Right.GetNumVertices())
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
              // No match.. do AlgabraicProduct with zero.
              AlgabraicProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == Left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && Right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < Left.GetNumVertices()) && (RightIndex < Right.GetNumVertices()))
}

void CFuzzyOperator::AlgabraicSum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);
  
  double X,Y,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Value = (X + Y) - (X * Y);
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::AlgabraicSum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
  
  CFuzzySet FSwork;
  
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;
  
  // Initialize.
  *result = left;
  FSwork  = right;
  
  // Do AlgabraicSum operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do AlgabraicSum of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        AlgabraicSum(pFEreturn,FE,*pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        AlgabraicSum(pFEreturn,FE,CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do AlgabraicSum operation for remaining variables on right side.
  CFuzzyElement FEwork;
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FEwork))
    {
      AlgabraicSum(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::AlgabraicSum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);
  

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do AlgabraicSum with zero.
            AlgabraicSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do AlgabraicSum with zero.
            AlgabraicSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do AlgabraicSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            AlgabraicSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the AlgabraicSum for FEleft and right's line segment and add to the return FMF.
          AlgabraicSum(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do AlgabraicSum with zero.
              AlgabraicSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do AlgabraicSum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          AlgabraicSum(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the AlgabraicSum for FEleft and FEright and add to FMF.
        AlgabraicSum(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do AlgabraicSum with zero.
            AlgabraicSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            AlgabraicSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do AlgabraicSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            AlgabraicSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the AlgabraicSum for FEright and left's line segment and add to the return FMF.
          AlgabraicSum(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do AlgabraicSum with zero.
              AlgabraicSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::HamacherProduct (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);
  
  double X,Y,Denominator,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Denominator = (X + Y) - (X * Y);

  if (Denominator != 0.0)
  {
    Value = (X * Y) / Denominator;
  }
  else
  {
    Value = 0.0;
  }
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::HamacherProduct (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);
  
  CFuzzySet      FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;
  
  // Initialize.
  *result = left;
  FSwork   = right;
  
  // Do HamacherProduct operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do HamacherProduct of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        HamacherProduct(pFEreturn,FE,*pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        HamacherProduct(pFEreturn,FE,CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do HamacherProduct operation for remaining variables on right side.
  CFuzzyElement FEwork;
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FEwork))
    {
      HamacherProduct(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::HamacherProduct (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);
  

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do HamacherProduct with zero.
            HamacherProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do HamacherProduct with zero.
            HamacherProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do HamacherProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            HamacherProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the HamacherProduct for FEleft and right's line segment and add to the return FMF.
          HamacherProduct(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do HamacherProduct with zero.
              HamacherProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do HamacherProduct.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          HamacherProduct(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the HamacherProduct for FEleft and FEright and add to FMF.
        HamacherProduct(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do HamacherProduct with zero.
            HamacherProduct(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            HamacherProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do HamacherProduct.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            HamacherProduct(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the HamacherProduct for FEright and left's line segment and add to the return FMF.
          HamacherProduct(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do HamacherProduct with zero.
              HamacherProduct(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

void CFuzzyOperator::HamacherSum (CFuzzyElement* result, const CFuzzyElement& left, const CFuzzyElement& right)
{
  assert(result != &left);
  assert(result != &right);
  
  double X,Y,Denominator,Value;
  
  X = left.GetMembership();
  Y = right.GetMembership();
  Denominator = 1.0 - (X * Y);
  if (Denominator != 0.0)
  {
    Value = ((X + Y) - (2.0 * X * Y)) / Denominator;
  }
  else
  {
    Value = 0.0;
  }
  
  *result = left;
  result->SetMembership(Value);
}

void CFuzzyOperator::HamacherSum (CFuzzySet* result, const CFuzzySet& left, const CFuzzySet& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzySet      FSwork;
  CFuzzyElement *pFEreturn;
  CFuzzyElement *pFEwork;
  CFuzzyElement  FE;

  // Initialize.
  *result = left;
  FSwork  = right;
  
  // Do HamacherSum operation for all variables on left side.
  long i=0;
  while (i < result->GetNumFuzzyElements())
  {
    if (result->GetFuzzyElement(i,&FE))
    {
      pFEwork = FSwork.GetpFuzzyElement(FE.GetValue());
      if (pFEwork)
      {
        // The fuzzy variable exists in both sets, do HamacherSum of each.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        HamacherSum(pFEreturn,FE,*pFEwork);
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
        
        // After processing each right side fuzzy variable it needs to be removed.
        FSwork.DeleteFuzzyElement(pFEwork->GetValue());
      }
      else
      {
        // The fuzzy variable doesn't exist on the right.
        pFEreturn = result->GetpFuzzyElement(FE.GetValue());
        HamacherSum(pFEreturn,FE,CFuzzyElement(0.0,0,FE.GetValue()));
        
        // Any zero membership values are not returned.
        if (pFEreturn->GetMembership() == 0.0)
        {
          pFEreturn = result->GetpFuzzyElement(FE.GetValue());
          result->DeleteFuzzyElement(pFEreturn->GetValue());
        }
        else
        {
          i++;
        }
      }//END if (pFEwork)
    }//END if (result->GetFuzzyElement(i,&FE))
  }//END while (i < result->GetNumFuzzyElements())
  
  // Do HamacherSum operation for remaining variables on right side.
  CFuzzyElement FEwork;
  for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
  {
    if (FSwork.GetFuzzyElement(i,&FEwork))
    {
      HamacherSum(&FE,CFuzzyElement(0.0,0,FEwork.GetValue()),FEwork);
      
      // Zero membership values are not returned.
      if (FE.GetMembership() > 0.0)
      {
        result->AddFuzzyElement(FE);
      }
    }//END if (FSwork.GetFuzzyElement(i,&FE)
  }//END for (i = 0; i < FSwork.GetNumFuzzyElements(); i++)
}

void CFuzzyOperator::HamacherSum (CFuzzyMembershipFunction* result, const CFuzzyMembershipFunction& left, const CFuzzyMembershipFunction& right)
{
  assert(result != &left);
  assert(result != &right);

  CFuzzyElement FEleft;
  CFuzzyElement FElastLeft;
  CFuzzyElement FEright;
  CFuzzyElement FElastRight;
  CFuzzyElement FEMid;
  CFuzzyElement FE;

  long LeftIndex  = 0,
       RightIndex = 0;

  bool LeftDone   = false,
       RightDone  = false;

  bool FirstPass  = true;

  // Setup.
  result->DeleteVertices();

  // Get start of each side.
  while ((!LeftDone) || (!RightDone))
  {
    // Get new values;
    if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
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
            // No match.. do HamacherSum with zero.
            HamacherSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance left point.
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
            // No match.. do HamacherSum with zero.
            HamacherSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!RightDone)
            {
              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
          // Check for line segment intersection, if intersection do HamacherSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            HamacherSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEleft;
          FEMid.SetMembership(right.GetValue(FEMid.GetValue()));

          // Calculate the HamacherSum for FEleft and right's line segment and add to the return FMF.
          HamacherSum(&FE,FEleft,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!LeftDone)
          {
            LeftIndex++;
            if (LeftIndex == left.GetNumVertices())
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
              // No match.. do HamacherSum with zero.
              HamacherSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
              result->AddVertex(FE);

              RightIndex++;
              if (RightIndex == right.GetNumVertices())
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
        // Check for line segment intersection, if intersection do HamacherSum.
        if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
        {
          HamacherSum(&FE,FEMid,FEMid);
          if (!result->AddVertex(FE))
          {
            // Overlaping vertices due to roundoff error, use the larger one.
            CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
            if (pFE)
            {
              CFuzzyElement FEwork = *pFE;
              Maximum(pFE, FE, FEwork);
            }
          }
        }

        // Calculate the HamacherSum for FEleft and FEright and add to FMF.
        HamacherSum(&FE,FEleft,FEright);
        result->AddVertex(FE);

        // Advance points.
        if (!LeftDone)
        {
          LeftIndex++;
          if (LeftIndex == left.GetNumVertices())
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
          if (RightIndex == right.GetNumVertices())
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
            // No match.. do HamacherSum with zero.
            HamacherSum(&FE,FEright,CFuzzyElement(0.0,0,FEright.GetValue()));
            result->AddVertex(FE);

            // Advance right point.
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
            HamacherSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
            result->AddVertex(FE);

            // Advance points.
            if (!LeftDone)
            {
              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
          // Check for line segment intersection, if intersection do HamacherSum.
          if (CFuzzyMembershipFunction::Intersection(FElastLeft,FEleft,FElastRight,FEright,&FEMid))
          {
            HamacherSum(&FE,FEMid,FEMid);
            if (!result->AddVertex(FE))
            {
              // Overlaping vertices due to roundoff error, use the larger one.
              CFuzzyElement *pFE = result->GetpVertex(FE.GetValue());
              if (pFE)
              {
                CFuzzyElement FEwork = *pFE;
                Maximum(pFE, FE, FEwork);
              }
            }
          }

          // Get inbetween value for FEleft.
          FEMid = FEright;
          FEMid.SetMembership(left.GetValue(FEMid.GetValue()));

          // Calculate the HamacherSum for FEright and left's line segment and add to the return FMF.
          HamacherSum(&FE,FEright,FEMid);
          result->AddVertex(FE);

          // Advance points.
          if (!RightDone)
          {
            RightIndex++;
            if (RightIndex == right.GetNumVertices())
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
              // No match.. do HamacherSum with zero.
              HamacherSum(&FE,FEleft,CFuzzyElement(0.0,0,FEleft.GetValue()));
              result->AddVertex(FE);

              LeftIndex++;
              if (LeftIndex == left.GetNumVertices())
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
    }
    else
    {
      assert(left.GetNumVertices() > 0);
      assert(right.GetNumVertices() > 0);
      return;
    }//END if (left.GetVertex(LeftIndex,&FEleft) && right.GetVertex(RightIndex,&FEright))
  }//END while ((LeftIndex < left.GetNumVertices()) && (RightIndex < right.GetNumVertices()))
}

