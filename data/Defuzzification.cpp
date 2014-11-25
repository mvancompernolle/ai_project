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
#include "Defuzzification.h"
#include <cassert>

CDefuzzification::CDefuzzification()
{

}

CDefuzzification::CDefuzzification(const CDefuzzification &right)
{

}


CDefuzzification::~CDefuzzification()
{

}


CDefuzzification & CDefuzzification::operator=(const CDefuzzification &right)
{
  assert(0);
  return *this;
}


int CDefuzzification::operator==(const CDefuzzification &right) const
{
  assert(0);
  return false;
}

int CDefuzzification::operator!=(const CDefuzzification &right) const
{
  return !(*this == right);
}

const CFuzzyElement& CDefuzzification::GetMeanOfMaximum (const CFuzzySet& right)
{
  static CFuzzyElement FEreturn;
         CFuzzyElement FE;
         CFuzzyElement FEmax;            

  CFuzzySet FSmax;

  // Get maximum fuzzy variables from the set.
  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      // New maximum.
      if (FE.GetMembership() > FEmax.GetMembership())
      {
        FEmax = FE;
        FSmax.DeleteFuzzyElements();
        FSmax.AddFuzzyElement(FEmax);
      }

      // Another maximum, add it to the list of maximum.
      if (FE.GetMembership() == FEmax.GetMembership())
      {
        FSmax.AddFuzzyElement(FE);
      }
    }
  }

  // Get the mean of all maximum fuzzy variables.
  long   ValueTotal = 0,
         NumMaxs = 0;
  double MembershipTotal = 0.0;

  for (i = 0; i < FSmax.GetNumFuzzyElements(); i++)
  {
    if (FSmax.GetFuzzyElement(i,&FE))
    {
      ValueTotal += FE.GetValue();
      MembershipTotal += FE.GetMembership();
      NumMaxs++;
    }
  }

  // Error handling just in case the fuzzy set was empty.
  if (NumMaxs == 0)
  {
    NumMaxs = 1;
  }

  FEreturn.SetValue(ValueTotal / NumMaxs);
  FEreturn.SetMembership(MembershipTotal / double(NumMaxs));

  return FEreturn;
}

const CFuzzyElement& CDefuzzification::GetMeanOfMaximum (const CFuzzyMembershipFunction& right)
{
  static CFuzzyElement FEreturn;
         CFuzzyElement FE;
         CFuzzyElement FEmax;
         CFuzzyElement FElast;

  CFuzzySet FSpeak;
  CFuzzySet FSmessa;

  long   ValueTotal = 0;
  long   NumMaxs = 0;
  double MembershipTotal = 0.0;

  // Get maximum fuzzy variables from the set.
  long i;
  for (i = 0; i < right.GetNumVertices(); i++)
  {
    if (right.GetVertex(i,&FE))
    {
      // Check for end of mesa.
      if (FE.GetMembership() < FElast.GetMembership())
      {
        if (FSmessa.GetNumFuzzyElements() > 1)
        {
          // we are at the end of a messa, add totals up.
          CFuzzyElement FEbegin,
                         FEend;
          if (FSmessa.GetFuzzyElement(0,&FEbegin) && 
              FSmessa.GetFuzzyElement(FSmessa.GetNumFuzzyElements()-1,&FEend))
          {
            double n = double(FEend.GetValue() - FEbegin.GetValue()),
                   ValueSubTotal = ((n + 1) * (n / 2.0)) + (double(FEbegin.GetValue()) * (n + 1));
            ValueTotal += ValueSubTotal;
            NumMaxs += n+1;
            MembershipTotal += double(n + 1) * FElast.GetMembership();
          }

          FSmessa.DeleteFuzzyElements();
        }
      }

      // New maximum.
      if (FE.GetMembership() > FEmax.GetMembership())
      {
        FEmax = FE;
        FSpeak.DeleteFuzzyElements();
        FSmessa.DeleteFuzzyElements();
        ValueTotal = 0;
        NumMaxs = 0;
        MembershipTotal = 0;
      }

      // Another maximum, we are going to need to add it.
      if (FE.GetMembership() == FEmax.GetMembership())
      {
        // Check last variable.
        if ((i >= 1) && right.GetVertex(i-1,&FElast))
        {
          if (FElast.GetMembership() == FE.GetMembership())
          {
            // The last one and this one do make a mesa, so add it to the mesa catagory.
            FSmessa.AddFuzzyElement(FE);

            // The last one was added to the peaks, so it will need to be removed since it is part of a messa.
            FSpeak.DeleteFuzzyElement(FElast.GetValue());
            FSmessa.AddFuzzyElement(FElast);
          }
          else
          {
            // The last one and this one do not make a mesa, add it to the peaks catagory.
            FSpeak.AddFuzzyElement(FE);
          }
        }
        else
        {
          // There was no last variable, add it to the peaks catagory.
          FSpeak.AddFuzzyElement(FE);
        }
      }

      FElast = FE;
    }//END if (right.GetFuzzyElement(i,&FE))
  }//END for (long i = 0; i < right.GetNumFuzzyElements(); i++)

  // Check for left-over messa at the end.
  if (FSmessa.GetNumFuzzyElements() > 1)
  {
    // we are at the end of a messa, add totals up.
    CFuzzyElement FEbegin,
                   FEend;
    if (FSmessa.GetFuzzyElement(0,&FEbegin) && 
        FSmessa.GetFuzzyElement(FSmessa.GetNumFuzzyElements()-1,&FEend))
    {
      long             n = FEend.GetValue() - FEbegin.GetValue(),
           ValueSubTotal = ((n + 1) * (n/2)) + (FEbegin.GetValue() * (n + 1));

      ValueTotal += ValueSubTotal;
      NumMaxs += n+1;
      MembershipTotal += (n + 1) * FE.GetMembership();
    }
  }

  // Get the mean of all maximum peaks fuzzy variables.
  for (i = 0; i < FSpeak.GetNumFuzzyElements(); i++)
  {
    if (FSpeak.GetFuzzyElement(i,&FE))
    {
      ValueTotal += FE.GetValue();
      MembershipTotal += FE.GetMembership();
      NumMaxs++;
    }
  }

  // Error handling just in case the fuzzy membership function was empty.
  if (NumMaxs == 0)
  {
    NumMaxs = 1;
  }

  FEreturn.SetValue(ValueTotal / NumMaxs);
  FEreturn.SetMembership(MembershipTotal / double(NumMaxs));

  return FEreturn;
}

const CFuzzyElement& CDefuzzification::GetCenterOfArea (const CFuzzySet& right)
{
  double SumTotal = 0;
  double MembershipTotal = 0;

  CFuzzyElement FE;
  static CFuzzyElement FEreturn;

  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      SumTotal += (double(FE.GetValue()) * FE.GetMembership());
      MembershipTotal += FE.GetMembership();
    }
  }

  // Set the return value and it's membership.
  FEreturn.SetValue(long(SumTotal / MembershipTotal));
  long Index = right.FindFuzzyElement(FEreturn.GetValue());
  if (Index != -1)
  {
    right.GetFuzzyElement(Index,&FE);
    FEreturn.SetMembership(FE.GetMembership());
  }
  else
  {
    long LowerIndex = right.FindLowerIndex(FEreturn.GetValue()),
         UpperIndex = right.FindUpperIndex(FEreturn.GetValue());

    // Since value doesn't exist, we will interpolate, just like CFuzzyMembershipFunction::GetValue()
    if ((LowerIndex != -1) && (UpperIndex != -1))
    {
      CFuzzyElement FElower,
                     FEupper;
      right.GetFuzzyElement(LowerIndex,&FElower);
      right.GetFuzzyElement(UpperIndex,&FEupper);

      double M = (FEupper.GetMembership() - FElower.GetMembership()) / (FEupper.GetValue() - FElower.GetValue()),
             B = FElower.GetMembership() - (M * (FElower.GetValue()));
      FEreturn.SetMembership((M * FEreturn.GetValue()) + B);
    }
    else
    {
      // We have one at either end.
      if (UpperIndex == -1)
      {
        right.GetFuzzyElement(right.GetNumFuzzyElements()-1,&FE);
        FEreturn.SetMembership(FE.GetMembership());
      }
      if (LowerIndex == -1)
      {
        right.GetFuzzyElement(0,&FE);
        FEreturn.SetMembership(FE.GetMembership());
      }
    }
  }

  return FEreturn;
}

const CFuzzyElement& CDefuzzification::GetCenterOfArea (const CFuzzyMembershipFunction& right)
{
  double SumTotal = 0;
  double MembershipTotal = 0;

  CFuzzyElement FE,
                 FElast;
  static CFuzzyElement FEreturn;

  for (long i = 1; i < right.GetNumVertices(); i++)
  {
    if (right.GetVertex(i,&FE) && right.GetVertex(i-1,&FElast))
    {
      double n = double(FE.GetValue() - FElast.GetValue()),
             ValueSubTotal = ((n + 1) * (n / 2.0)) + (double(FElast.GetValue()) * (n + 1)),
             AverageMembership  = (FElast.GetMembership() + FE.GetMembership())/2.0,
             MembershipSubTotal = (n + 1) * ((FElast.GetMembership() + FE.GetMembership())/2.0);

      SumTotal += ValueSubTotal * AverageMembership;
      MembershipTotal += MembershipSubTotal;
    }
  }

  FEreturn.SetValue(long(SumTotal / MembershipTotal));
  FEreturn.SetMembership(right.GetValue(FEreturn.GetValue()));

  return FEreturn;
}

const CFuzzyElement& CDefuzzification::GetCenterOfLargestArea (const CFuzzySet& right)
{
  double SumTotal = 0;
  double MembershipTotal = 0;
  double MaxTotal = 0;
  double MaxMembershipTotal = 0;

  CFuzzyElement FE;
  CFuzzyElement FElast;
  CFuzzyElement FEarea;
  static CFuzzyElement FEreturn;
  CFuzzySet FSarea;
  bool Ascending = true;

  for (long i = 0; i < right.GetNumFuzzyElements(); i++)
  {
    if (right.GetFuzzyElement(i,&FE))
    {
      // Initialization.
      if (!i)
      {
        FElast = FE;
      }

      if (Ascending)
      {
        // Waiting until we get to a maximu.
        if ((FE.GetMembership() < FElast.GetMembership()) || (FE.GetValue() - FElast.GetValue() > 1))
        {
          Ascending = false;
        }
        
        FSarea.AddFuzzyElement(FE);
      }
      else
      {
        // Continue descending until we get to a minimum.
        if ((FE.GetMembership() > FElast.GetMembership()) || (FE.GetValue() - FElast.GetValue() > 1))
        {
          Ascending = true;
          
          // We've already hit a peak and just hit the next trough so find area for this section.
          for (long a = 0; a < FSarea.GetNumFuzzyElements(); a++)
          {
            if (FSarea.GetFuzzyElement(a,&FEarea))
            {
              SumTotal += (double(FEarea.GetValue()) * FEarea.GetMembership());
              MembershipTotal += FEarea.GetMembership();
            }
          }

          // Check current area against maximum area.
          if ((MembershipTotal) > (MaxMembershipTotal))
          {
            MaxTotal = SumTotal;
            MaxMembershipTotal = MembershipTotal;
          }

          // Reset for next area.
          FSarea.DeleteFuzzyElements();
          FSarea.AddFuzzyElement(FE);
          SumTotal = 0.0;
          MembershipTotal = 0.0;
        }
        else
        {
          // Continue descending.
          FSarea.AddFuzzyElement(FE);
        }//END if (FE.GetMembership() > FElast.GetMembership())
      }
    }//END if (right.GetFuzzyElement(i,&FE))
    FElast = FE;
  }//END for (long i = 0; i < right.GetNumFuzzyElements(); i++)

  // Check for leftovers in FSarea just in case there are any.
  if (FSarea.GetNumFuzzyElements())
  {
    // We've already hit a peak and just hit the next trough so find area for this section.
    for (long a = 0; a < FSarea.GetNumFuzzyElements(); a++)
    {
      if (FSarea.GetFuzzyElement(a,&FEarea))
      {
        SumTotal += (double(FEarea.GetValue()) * FEarea.GetMembership());
        MembershipTotal += FEarea.GetMembership();
      }
    }
          
    // Check current area against maximum area.
    if ((MembershipTotal) > (MaxMembershipTotal))
    {
      MaxTotal = SumTotal;
      MaxMembershipTotal = MembershipTotal;
    }
  }//END if (FSarea.GetFuzzyElements())

  // Error correction.
  if (MaxMembershipTotal == 0.0)
  {
    MaxMembershipTotal = 1.0;
  }

  // Set the return value and it's membership.
  FEreturn.SetValue(long(MaxTotal / MaxMembershipTotal));
  long Index = right.FindFuzzyElement(FEreturn.GetValue());
  if (Index != -1)
  {
    right.GetFuzzyElement(Index,&FE);
    FEreturn.SetMembership(FE.GetMembership());
  }
  else
  {
    assert(Index != -1);
    FEreturn.SetMembership(0.0);
  }

  return FEreturn;
}

const CFuzzyElement& CDefuzzification::GetCenterOfLargestArea (const CFuzzyMembershipFunction& right)
{
  double SumTotal = 0;
  double MembershipTotal = 0;
  double MaxTotal = 0;
  double MaxMembershipTotal = 0;

  CFuzzyElement FE;
  CFuzzyElement FElast;
  CFuzzyElement FEarea;
  CFuzzyElement FEareaLast;
  static CFuzzyElement FEreturn;
  CFuzzyMembershipFunction FMFarea;
  bool Ascending = true;

  long i;
  for (i = 0; i < right.GetNumVertices(); i++)
  {
    if (right.GetVertex(i,&FE))
    {
      // Initialization.
      if (!i)
      {
        FElast = FE;
      }

      if (Ascending)
      {
        // Waiting until we get to a maximu.
        if (FE.GetMembership() < FElast.GetMembership())
        {
          Ascending = false;
        }
        
        FMFarea.AddVertex(FE);
      }
      else
      {
        // Continue descending until we get to a minimum.
        if (FE.GetMembership() > FElast.GetMembership())
        {
          Ascending = true;
          
          // We've already hit a peak and just hit the next trough so find area for this section.
          for (long a = 1; a < FMFarea.GetNumVertices(); a++)
          {
            if (FMFarea.GetVertex(a,&FEarea) && FMFarea.GetVertex(a-1,&FEareaLast))
            {
              double n = double(FEarea.GetValue() - FEareaLast.GetValue()),
                     ValueSubTotal = ((n + 1) * (n / 2.0)) + (double(FEareaLast.GetValue()) * (n + 1)),
                     AverageMembership  = (FEareaLast.GetMembership() + FEarea.GetMembership())/2.0,
                     MembershipSubTotal = (n + 1) * ((FEareaLast.GetMembership() + FEarea.GetMembership())/2.0);

              SumTotal += ValueSubTotal * AverageMembership;
              MembershipTotal += MembershipSubTotal;
            }
          }

          // Check current area against maximum area.
          if ((MembershipTotal) > (MaxMembershipTotal))
          {
            MaxTotal = SumTotal;
            MaxMembershipTotal = MembershipTotal;
          }

          // Reset values for current area.
          FMFarea.DeleteVertices();
          FMFarea.AddVertex(FElast);
          FMFarea.AddVertex(FE);
          SumTotal = 0.0;
          MembershipTotal = 0.0;
        }
        else
        {
          // Continue descending.
          FMFarea.AddVertex(FE);
        }//END if (FE.GetMembership() > FElast.GetMembership())
      }
    }//END if (right.GetFuzzyElement(i,&FE))
    FElast = FE;
  }//END for (long i = 0; i < right.GetNumFuzzyElements(); i++)

  // Check for leftovers in FMFarea just in case there are any.
  if (FMFarea.GetNumVertices())
  {
    for (i = 1; i < FMFarea.GetNumVertices(); i++)
    {
      if (FMFarea.GetVertex(i,&FEarea) && FMFarea.GetVertex(i-1,&FEareaLast))
      {
        double n = double(FEarea.GetValue() - FEareaLast.GetValue()),
               ValueSubTotal = ((n + 1) * (n / 2.0)) + (double(FEareaLast.GetValue()) * (n + 1)),
               AverageMembership  = (FEareaLast.GetMembership() + FEarea.GetMembership())/2.0,
               MembershipSubTotal = (n + 1) * ((FEareaLast.GetMembership() + FEarea.GetMembership())/2.0);

        SumTotal += ValueSubTotal * AverageMembership;
        MembershipTotal += MembershipSubTotal;
      }
    }

    // Check current area against maximum area.
    if ((MembershipTotal) > (MaxMembershipTotal))
    {
      MaxTotal = SumTotal;
      MaxMembershipTotal = MembershipTotal;
    }
  }//END if (FMFarea.GetNumVertices())

  // Error correction.
  if (MaxMembershipTotal == 0.0)
  {
    MaxMembershipTotal = 1.0;
  }

  FEreturn.SetValue(long(MaxTotal / MaxMembershipTotal));
  FEreturn.SetMembership(right.GetValue(FEreturn.GetValue()));

  return FEreturn;
}
