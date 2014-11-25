/** TDSP *******************************************************************
                          TDSPLinearSolver.cpp  -  description
                             -------------------
    begin                : Thu Mar 14 2002
    copyright            : (C) 2002 by Peter Jung
    email                : jung@hhi.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <TCollection.h>
#include "TDSPLinearSolver.h"

ClassImp(TDSPLinearSolver)

TDSPLinearSolver::TDSPLinearSolver(TDSPMatrix *a, TDSPVector *b,
				   TDSPVector *y){
  
  fA = a;
  fX = NULL;
  fY = y;
  fB = b;

  fGuessX = NULL;
  
  fMethod = kLinearSolveJacobi;

  fMaxIterations = 100;
  fStopPrecision     = 1.0e-6;
  fIterations    = 0;
  
  // Default Hard Decisions

  fIter_Serial_HD   = 9999999;
  fIter_Parallel_HD = 9999999;

  // Reset the Mean calculations
  //
  _reset_means_();

  fUsingAStack = kFALSE;
  fAList.SetOwner();
  

}
TDSPLinearSolver::~TDSPLinearSolver(){
}

void TDSPLinearSolver::SetA(TDSPMatrix *a) {
  fA = a;
}

void TDSPLinearSolver::StackA(TDSPMatrix*A) {

  /* Hey-we trying to use a stack here - switch it on ! */
  if (!fUsingAStack) {
    Info("StackA","Enabling a Stack for the Matrices ...");
    fUsingAStack=kTRUE;
  }
  /* Duplicate A */
  A=A->Dup();

  /* add on the list */
  fAList.Add(A);
  
  /* need for BlockSolve... */
  if (!fA) SetA(A); 

}


void   TDSPLinearSolver::Print() {

  cout << "Method = ";
  switch(fMethod) {
  case kLinearSolveNothing:
    cout << "Nothing (report only that a solution was found" << endl;
    break;
  case kLinearSolveJacobi:
  case kLinearSolveCG:
  case kLinearSolveGaussSeidel:
  case kLinearSolveGaussSeidelBinary:
  case kLinearSolveGaussSeidelQpsk:
    {
      switch(fMethod) {
      case kLinearSolveGaussSeidel:
	cout << "Gauss-Seidel Algorithm" << endl;
	break;
      case kLinearSolveGaussSeidelBinary:
	cout << "Gauss-Seidel Algorithm with binary decisions" << endl;
	break;
      case kLinearSolveGaussSeidelQpsk:
	cout << "Gauss-Seidel Algorithm with QPSK decisions" << endl;
	break;
      case kLinearSolveJacobi:
	cout << "Jacobi Algorithm" << endl;
	break;
      case kLinearSolveCG:
	cout << "Conjugate Gradient method" << endl;
	break;
      }
      
      cout << "Stop precision : " << fStopPrecision << endl;
      cout << "MaxIterations  : " << fMaxIterations << endl;

      switch(fMethod) {
	
      case kLinearSolveGaussSeidelBinary:
      case kLinearSolveGaussSeidelQpsk:
	cout << "Serial   decisions starting at the " << fIter_Serial_HD << " iteration" << endl;
	cout << "Parallel decisions starting at the " << fIter_Parallel_HD << " iteration" << endl;
      default:
	break;
	
      }
      
    }
    break;
  case kLinearSolveML:
    cout << "Maximum Likelihood method" << endl;
    cout << "Number symbols per dimension = " << fNumXAlphabet << endl;
    if (fXAlphabet) {
      cout << "symbols =";
      for(Int_t i=0;i<fNumXAlphabet;++i) cout << " " << fXAlphabet[i];
      cout << endl;
    }
    
    break;
    
  }
  
}

Bool_t TDSPLinearSolver::Solve() {

#ifndef LINALG_INDEX_OPT
# warning "!! Using UNOPTIMIZED version of <TDSPLinearSolver::Solve> !!"
#else
  //# error   "there is still in an Error in the optimized <TDSPLinearSolver::Solve>"
#endif

  fHaveSolution = kFALSE;
  
  // Initial Checkups
  
  if (!fA) {
    Error("Solve","No Matrix A given !");
    return kFALSE;
  }
  if (!fY) {
    Error("Solve","No Vector Y given !");
    return kFALSE;
  }

  Int_t N = fA->GetRows();

  if (!fB) {
    Warning("Solve","No Vector B given - setting B=0!");
    fB=new TDSPVector(N);
    fB->Zeros();
  }

  if (!fX) fX = new TDSPVector();
  fX->SetLen(N);

  // Do we have a initial Guess ?
  //
  Bool_t myGuess = kFALSE;
  if (!fGuessX) {
    fGuessX = new TDSPVector(N);
    fGuessX->Zeros();
    myGuess = kTRUE;
  }

    
#define BEGIN_ITERATION \
if (fA->DiagZero()) { \
    Error("Solve","Matrix a has a zero on the diagonal"); \
    break; \
} fGuessX->Copy(&Xtmp);for(fIterations=0;fIterations<fMaxIterations;++fIterations) { fPrecision=0

#define END_ITERATION fX->Copy(&Xtmp);} fHaveSolution = kTRUE
    
#define BEGIN_INDEX  for(register Int_t i=0;i<N;++i) { register Int_t j=0
#define END_INDEX } if (fPrecision<fStopPrecision) break


    
  switch(fMethod) {
  case kLinearSolveNothing:
    fHaveSolution = kTRUE;
    break;

  case kLinearSolveJacobi:

   
    BEGIN_ITERATION;
    
    fX->Zeros();
    
#ifdef LINALG_INDEX_OPT
    TComplex *_fA = fA->GetVec();
    TComplex *_fX = fX->GetVec();
#endif	

    BEGIN_INDEX;
    
#ifdef LINALG_INDEX_OPT
    
    TComplex *_Xtmp = Xtmp.GetVec();
    
# ifdef LINALG_MAT_ROWWISE
    for(;j<i;++j) (*_fX)+=*(_fA++)**(_Xtmp++);
    ++_fA;++_Xtmp;++j;
    for(;j<N;++j) (*_fX)+=*(_fA++)**(_Xtmp++);
    
    fPrecision+=Abs2(((*_fX)=(fY->Element(i)-fB->Element(i)-*_fX)/fA->Element(i,i))
		     -Xtmp.Element(i));
    _fX++;

# else
#   warning "Column-wise Jacobi not yet implemented" 
    Error("Solve"," Column-wise Jacobi not yet implemented!");
    return;
# endif
#else
# warning "Using unoptimized Jacobi"
    
    for(;j<i;++j) 
      fX->Element(i)+=fA->Element(i,j)*Xtmp.Element(j);
    for(++j;j<N;++j) 
      fX->Element(i)+=fA->Element(i,j)*Xtmp.Element(j);
    fPrecision+=Abs2((fX->Element(i)=(fY->Element(i)-fB->Element(i)-fX->Element(i))/fA->Element(i,i))
		     -Xtmp.Element(i));
    
#endif	
    
    END_INDEX;
    END_ITERATION;
    
    break;

    
  case kLinearSolveGaussSeidel:
        
    
    BEGIN_ITERATION;
    
    // Iterations
    
    fX->Zeros();
    
    
#ifdef LINALG_INDEX_OPT
    TComplex *_fA = fA->GetVec();
    TComplex *_fX = fX->GetVec();
#endif	
    
    BEGIN_INDEX;
    
#ifdef LINALG_INDEX_OPT
    
    TComplex *_fXX  = fX->GetVec();
    TComplex *_Xtmp = Xtmp.GetVec()+i+1;
    
# ifdef LINALG_MAT_ROWWISE
    for(;j<i;++j) (*_fX)+=*(_fA++)**(_fXX++);
    ++_fA;++j;
    for(;j<N;++j) (*_fX)+=*(_fA++)**(_Xtmp++);
# else
#   warning "Column-wise GaussSeidel not yet implemented" 
    Error("Solve"," Column-wise GaussSeidel not yet implemented!");
    return;
# endif
    fPrecision+=Abs2(((*_fX)=(fY->Element(i)-fB->Element(i)-*_fX)/fA->Element(i,i))
 		     -Xtmp.Element(i));
    _fX++;
#else
# warning "Using unoptimized GaussSeidel"
    for(;j<i;++j) 
      fX->Element(i)+=fA->Element(i,j)*fX->Element(j);
    for(++j;j<N;++j) 
      fX->Element(i)+=fA->Element(i,j)*Xtmp.Element(j);
    fPrecision+=Abs2((fX->Element(i)=(fY->Element(i)-fB->Element(i)-fX->Element(i))/fA->Element(i,i))
		     -Xtmp.Element(i));
#endif
    END_INDEX;
    
    END_ITERATION;
      
    
    break;

    
    
#ifdef LINALG_INDEX_OPT
# ifdef  LINALG_MAT_ROWWISE

#  define GS_HD_INIT

#  define GS_HD_MULT \
TComplex *_fX   = fX->GetVec(); \
TComplex *_Xtmp = Xtmp.GetVec()+i+1; \
t=0; \
for(;j<i;++j) t+=*(_fA++)**(_fX++); ++_fA;++j; for(;j<N;++j) t+=*(_fA++)**(_Xtmp++)

# define GS_HD_INIT_INDEX \
TComplex *_fA = fA->GetVec();

# else
#  warning "Column-wise GaussSeidel-Mult. not implemented"
#  define GS_HD_MULT Error("Solve"," Column-wise GaussSeidel not yet implemented!");return;
#  define GS_HD_INIT_INDEX 
# endif
#else

# warning "Using unoptimized GaussSeidel-Mult."
# define GS_HD_MULT \
t=0; \
for(;j<i;++j) t+=fA->Element(i,j)*fX->Element(j);for(++j;j<N;++j) t+=fA->Element(i,j)*Xtmp.Element(j)
# define GS_HD_INIT_INDEX 

#endif

#define GS_BEGIN_ITERATION  \
    TComplex t; \
    BEGIN_ITERATION; \
    GS_HD_INIT_INDEX \
    BEGIN_INDEX; \
    GS_HD_MULT; \
    t+=fB->Element(i); \
    t-=fY->Element(i); \
    t*=-1.0/fA->Element(i,i) 

#define GS_END_ITERATION END_ITERATION
#define GS_CALC_PRECISION fPrecision+=Abs2((fX->Element(i)=t)-Xtmp.Element(i)); END_INDEX
#define GS_SERIAL_HD  if (fIterations>=fIter_Serial_HD)
#define GS_PARALLEL_HD if (fIterations>=fIter_Parallel_HD) for(register Int_t i=0;i<N;++i)
  case kLinearSolveGaussSeidelBinary:
    {
      GS_BEGIN_ITERATION;
      GS_SERIAL_HD        t=t.fRe>0 ? 1:-1; // BPSK Serial Hard Decisions
      GS_CALC_PRECISION;
      GS_PARALLEL_HD      fX->Element(i)=fX->Element(i).fRe>0 ? 1:-1;// parallel BPSK Hard decisions
      GS_END_ITERATION;
      
    }
    break;
    
  case kLinearSolveGaussSeidelQpsk:
    {

      Double_t x,y;

      GS_BEGIN_ITERATION;
      GS_SERIAL_HD        {
	// rotation : 1   0
	//            2   3
	// 0 = 1 , 1=i , 2=-1, 3=-i
	x = y = t.fRe;x-=t.fIm; y+=t.fIm;
	t = x>0 ? ( y>0 ? TComplex(1,0) : -I) : (y>0 ? I : TComplex(-1,0)); // Serial QPSK Hard Decisions
      }
      GS_CALC_PRECISION;
      GS_PARALLEL_HD {
	x = y = fX->Element(i).fRe;x-=fX->Element(i).fIm;y+=fX->Element(i).fIm;
	fX->Element(i) =  x>0 ? ( y>0 ? TComplex(1,0) : -I) : (y>0 ? I : TComplex(-1,0)); // Parallel QPSK Hard Decisions
      }
      GS_END_ITERATION;
      
    }
    break;
    
  case kLinearSolveCG:

    {
      
      fGuessX->Copy(fX);

      // Calculate the Start Residual
      
      fA->RightApply(fX, &Xtmp);
      Xtmp*=-1;
      Xtmp+=(*fY);
      Xtmp-=(*fB);
      
      Double_t r1=Xtmp.Norm2();
      Double_t r2 = 0;
      
      TDSPVector Dir,RDir,ScaleDir;
      
      for(fIterations=0;fIterations<fMaxIterations;++fIterations) { // Iterations
	
	fPrecision=0;
	
	// No Precontitioning 
	//
	
	if (fIterations) {
	  Dir*=r1/r2;
	  Dir+=Xtmp;
	} else {
	  Xtmp.Copy(&Dir);
	}
	
	// Calculate the Scaling
	
	fA->RightApply(&Dir,&RDir);
	Double_t scale=r1/Dot(Dir,RDir);
	
	// Scale the Direction
	
	Dir.Copy(&ScaleDir);
	ScaleDir*=scale;
	RDir*=scale;
	
	// Update the Residual
	
	Xtmp-=RDir;
	
	// Update the Solution
	
	(*fX)+=ScaleDir;
	
	// Update the Precision
	
	if ((r1 = Xtmp.Norm2())<fStopPrecision) break;
	
	r2 = r1;
	
      }
      
      fPrecision = r1;
      
    }      
    
    fHaveSolution = kTRUE;

    break;
    
  case kLinearSolveML:
    {
      if (!fXAlphabet) {
	Error("Solve","ML-Solver needs finite alphabet (use SetXAlphabet)!");
	break;
      }
      TDSPVector Xtmp1;
      Double_t minDist=999999.;
      Double_t   dist=0;
      // Create start codeword
      Xtmp.SetLen(N);
      Xtmp.Set(fXAlphabet[0]);
      
      while(1) {
	fA->RightApply(&Xtmp,&Xtmp1);
	dist = Dist(fY,&Xtmp1);
	if (dist<minDist) { 
	  minDist=dist;
	  Xtmp.Copy(fX);
	}
	if (minDist<=fMinDist) break;
	if (!Xtmp.OnCodeInc(fXAlphabet,fNumXAlphabet)) break;
      }
      fPrecision = minDist;
    }
    // Info("Solve","fPrecision = %e",fPrecision);
    fHaveSolution = kTRUE;
    break;
    
  default:
    
    Error("Solve","Not yet implemented!");
    break;
  }

  if (myGuess) { delete fGuessX; fGuessX=NULL;};
  
  // Update some mean numbers like :
  //    fMeanIterations - mean number iterations needed
  //    fMeanPrecision  - mean precision

  if (fHaveSolution) _push_means_();

  // If using a matrix stack - remove the first next matrix

  if (fUsingAStack) {
    delete fAList.Remove(fAList.First());
    TDSPMatrix *next=(TDSPMatrix*)fAList.First();
    SetA(next);
  }
  return fHaveSolution;

}

