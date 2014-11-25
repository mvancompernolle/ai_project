//
// $Id: MatrixSVD.cpp,v 1.7 2003/08/07 17:47:28 knicewar Exp $
//
// This code was adapted from the C++LinAlg package by Oleg Kiselyov
// <oleg@pobox.com or oleg@acm.org> (available from www.netlib.
//

// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *			  Numerical Math Package
 *	Singular Value Decomposition of a rectangular matrix
 *			     A = U * Sig * V'
 *
 * where matrices U and V are orthogonal and Sig is a digonal matrix.
 *
 * The singular value decomposition is performed by constructing an SVD
 * object from an M*N matrix A with M>=N (that is, at least as many rows
 * as columns). Note, in case M > N, matrix Sig has to be a M*N diagonal
 * matrix. However, it has only N diag elements, which we store in a 1:N
 * Vector sig.
 *
 * Algorithm
 *	Bidiagonalization with Householder reflections followed by a
 * modification of a QR-algorithm. For more details, see
 *   G.E. Forsythe, M.A. Malcolm, C.B. Moler
 *   Computer methods for mathematical computations. - Prentice-Hall, 1977
 * However, in the persent implementation, matrices U and V are computed
 * right away rather than delayed until after all Householder reflections.
 *
 * This code is based for the most part on a Algol68 code I wrote
 * ca. 1987
 *
 * $Id: MatrixSVD.cpp,v 1.7 2003/08/07 17:47:28 knicewar Exp $
 *
 ************************************************************************
 */

#include <algorithm>
#include <math.h>
#include <float.h>
#include "MatrixSVD.h"

#if defined(_MSC_VER) && (_MSC_VER < 1300)
//
// For some reason, Bill left this out of his old STL algorithm
// implementation.
//
template<class T>
T const & max(T const & x, T const & y)
{
  return ((x > y) ? x : y);
}
#endif


using namespace std;
using namespace LinAlg;



/*
 *------------------------------------------------------------------------
 *				Bidiagonalization
 */

 /*
 *			Left Householder Transformations
 *
 * Zero out an entire subdiagonal of the i-th column of A and compute the
 * modified A[i,i] by multiplication (UP' * A) with a matrix UP'
 *   (1)  UP' = E - UPi * UPi' / beta
 *
 * where a column-vector UPi is as follows
 *   (2)  UPi = [ (i-1) zeros, A[i,i] + Norm, vector A[i+1:M,i] ]
 * where beta = UPi' * A[,i] and Norm is the norm of a vector A[i:M,i]
 * (sub-diag part of the i-th column of A). Note we assign the Norm the
 * same sign as that of A[i,i].
 * By construction, (1) does not affect the first (i-1) rows of A. Since
 * A[*,1:i-1] is bidiagonal (the result of the i-1 previous steps of
 * the bidiag algorithm), transform (1) doesn't affect these i-1 columns
 * either as one can easily verify.
 * The i-th column of A is transformed as
 *   (3)  UP' * A[*,i] = A[*,i] - UPi
 * (since UPi'*A[*,i]/beta = 1 by construction of UPi and beta)
 * This means effectively zeroing out A[i+1:M,i] (the entire subdiagonal
 * of the i-th column of A) and replacing A[i,i] with the -Norm. Thus
 * modified A[i,i] is returned by the present function.
 * The other (i+1:N) columns of A are transformed as
 *    (4)  UP' * A[,j] = A[,j] - UPi * ( UPi' * A[,j] / beta )
 * Note, due to (2), only elements of rows i+1:M actually  participate
 * in above transforms; the upper i-1 rows of A are not affected.
 * As was mentioned earlier,
 * (5)  beta = UPi' * A[,i] = (A[i,i] + Norm)*A[i,i] + A[i+1:M,i]*A[i+1:M,i]
 *	= ||A[i:M,i]||^2 + Norm*A[i,i] = Norm^2 + Norm*A[i,i]
 * (note the sign of the Norm is the same as A[i,i])
 * For extra precision, vector UPi (and so is Norm and beta) are scaled,
 * which would not affect (4) as easy to see.
 *
 * To satisfy the definition
 *   (6)  .SIG = U' A V
 * the result of consecutive transformations (1) over matrix A is accumulated
 * in matrix U' (which is initialized to be a unit matrix). At each step,
 * U' is left-multiplied by UP' = UP (UP is symmetric by construction,
 * see (1)). That is, U is right-multiplied by UP, that is, rows of U are
 * transformed similarly to columns of A, see eq. (4). We also keep in mind
 * that multiplication by UP at the i-th step does not affect the first i-1
 * columns of U.
 * Note that the vector UPi doesn't have to be allocated explicitly: its
 * first i-1 components are zeros (which we can always imply in computations),
 * and the rest of the components (but the UPi[i]) are the same as those
 * of A[i:M,i], the subdiagonal of A[,i]. This column, A[,i] is affected only
 * trivially as explained above, that is, we don't need to carry this
 * transformation explicitly (only A[i,i] is going to be non-trivially
 * affected, that is, replaced by -Norm, but we will use sig[i] to store
 * the result).
 *
 */
 
double MatrixSVD::_left_householder(Matrix       & A,
                                    unsigned const i)
{					// Note that only UPi[i:M] matter
  double scale = 0;			// Compute the scaling factor
  register unsigned j, k;
  for (j = i; j < _M; ++j)
  {
    scale += fabs(A[j][i]);
  }
  if (scale == 0)			// If A[,i] is a null vector, no
  {
    return 0;				// transform is required
  }

  double Norm_sqr = 0;			// Scale UPi (that is, A[,i])
  for (j = i; j < _M; ++j)		// and compute its norm, Norm^2
  {
    A[j][i] /= scale;
    double s = A[j][i];
    Norm_sqr += s*s;
  }
  double new_Aii = sqrt(Norm_sqr);	// new_Aii = -Norm, Norm has the
  double & peek_Aii = A[i][i];
  if (peek_Aii > 0)			// same sign as Aii (that is, UPi[i])
  {
    new_Aii = -new_Aii;
  }
  float beta = float(-peek_Aii*new_Aii + Norm_sqr);  // intentional loss of data
  peek_Aii -= new_Aii;			// UPi[i] = A[i,i] - (-Norm)

  for (j = i + 1; j < _N; ++j)		// Transform i+1:N columns of A
  {
    double factor = 0;
    for (k = i; k < _M; ++k)
    {
      factor += A[k][i]*A[k][j];	// Compute UPi' * A[,j]
    }
    factor /= beta;
    for (k = i; k < _M; ++k)
    {
      A[k][j] -= A[k][i]*factor;
    }
  }

  for (j = 0; j < _M; j++)		// Accumulate the transform in U
  {
    double factor = 0;
    for (k = i; k < _M; ++k)
    {
      factor += A[k][i]*_u[j][k];	// Compute  U[j,] * UPi
    }
    factor /= beta;
    for (k = i; k < _M; ++k)
    {
      _u[j][k] -= A[k][i]*factor;
    }
  }
  return new_Aii*scale;			// Scale new Aii back (our new Sig[i])
}
 
/*
 *			Right Householder Transformations
 *
 * Zero out i+2:N elements of a row A[i,] of matrix A by right
 * multiplication (A * VP) with a matrix VP
 *   (1)  VP = E - VPi * VPi' / beta
 *
 * where a vector-column .VPi is as follows
 *   (2)  VPi = [ i zeros, A[i,i+1] + Norm, vector A[i,i+2:N] ]
 * where beta = A[i,] * VPi and Norm is the norm of a vector A[i,i+1:N]
 * (right-diag part of the i-th row of A). Note we assign the Norm the
 * same sign as that of A[i,i+1].
 * By construction, (1) does not affect the first i columns of A. Since
 * A[1:i-1,] is bidiagonal (the result of the previous steps of
 * the bidiag algorithm), transform (1) doesn't affect these i-1 rows
 * either as one can easily verify.
 * The i-th row of A is transformed as
 *  (3)  A[i,*] * VP = A[i,*] - VPi'
 * (since A[i,*]*VPi/beta = 1 by construction of VPi and beta)
 * This means effectively zeroing out A[i,i+2:N] (the entire right super-
 * diagonal of the i-th row of A, but ONE superdiag element) and replacing
 * A[i,i+1] with - Norm. Thus modified A[i,i+1] is returned as the result of
 * the present function.
 * The other (i+1:M) rows of A are transformed as
 *    (4)  A[j,] * VP = A[j,] - VPi' * ( A[j,] * VPi / beta )
 * Note, due to (2), only elements of columns i+1:N actually  participate
 * in above transforms; the left i columns of A are not affected.
 * As was mentioned earlier,
 * (5)  beta = A[i,] * VPi = (A[i,i+1] + Norm)*A[i,i+1]
 *			   + A[i,i+2:N]*A[i,i+2:N]
 *	= ||A[i,i+1:N]||^2 + Norm*A[i,i+1] = Norm^2 + Norm*A[i,i+1]
 * (note the sign of the Norm is the same as A[i,i+1])
 * For extra precision, vector VPi (and so is Norm and beta) are scaled,
 * which would not affect (4) as easy to see.
 *
 * The result of consecutive transformations (1) over matrix A is accumulated
 * in matrix V (which is initialized to be a unit matrix). At each step,
 * V is right-multiplied by VP. That is, rows of V are transformed similarly
 * to rows of A, see eq. (4). We also keep in mind that multiplication by
 * VP at the i-th step does not affect the first i rows of V.
 * Note that vector VPi doesn't have to be allocated explicitly: its
 * first i components are zeros (which we can always imply in computations),
 * and the rest of the components (but the VPi[i+1]) are the same as those
 * of A[i,i+1:N], the superdiagonal of A[i,]. This row, A[i,] is affected
 * only trivially as explained above, that is, we don't need to carry this
 * transformation explicitly (only A[i,i+1] is going to be non-trivially
 * affected, that is, replaced by -Norm, but we will use super_diag[i+1] to
 * store the result).
 *
 */
 
double MatrixSVD::_right_householder(Matrix       & A,
                                     unsigned const i)
{
  // Note only VPi[i+1:N] matter
  register unsigned j, k;
  double scale = 0;			// Compute the scaling factor
  for (j = i + 1; j < _N; ++j)
  {
    scale += fabs(A[i][j]);
  }
  if (scale == 0)			// If A[i,] is a null vector, no
  {
    return 0;				// transform is required
  }
 
  double Norm_sqr = 0;			// Scale VPi (that is, A[i,])
  for (j = i + 1; j < _N; ++j)		// and compute its norm, Norm^2
  {
    A[i][j] /= scale;
    double s = A[i][j];
    Norm_sqr += s*s;
  }
  double new_Aii1 = sqrt(Norm_sqr);	// new_Aii1 = -Norm, Norm has the
  double f = A[i][i + 1];
  if (f > 0)				// same sign as
  {
    new_Aii1 = -new_Aii1; 		// Aii1 (that is, VPi[i+1])
  }
  float beta = float(-f*new_Aii1 + Norm_sqr);  // intentional loss of data
  A[i][i + 1] = f - new_Aii1;		// VPi[i+1] = A[i,i+1] - (-Norm)
   
  for (j = i + 1; j < _M; ++j)		// Transform i+1:M rows of A
  {
    double factor = 0;
    for (k = i + 1; k < _N; ++k)
    {       
      factor += A[i][k]*A[j][k];	// Compute A[j,] * VPi
    }
    factor /= beta;
    for (k = i + 1; k < _N; ++k)
    {
      A[j][k] -= A[i][k]*factor;
    }
  }

  for (j = 0; j < _N; ++j)		// Accumulate the transform in V
  {
    double factor = 0;
    for (k = i + 1; k < _N; ++k)
    {
      factor += _v[j][k]*A[i][k];	// Compute  V[j,] * VPi
    }
    factor /= beta;
    for (k = i + 1; k < _N; ++k)
    {
      _v[j][k] -= A[i][k]*factor;
    }
  }
  return new_Aii1*scale;		// Scale new Aii1 back
}

/*
 *------------------------------------------------------------------------
 *			  Bidiagonalization
 * This nethod turns matrix A into a bidiagonal one. Its N diagonal elements
 * are stored in a vector sig, while N-1 superdiagonal elements are stored
 * in a vector super_diag(2:N) (with super_diag(1) being always 0).
 * Matrices U and V store the record of orthogonal Householder
 * reflections that were used to convert A to this form. The method
 * returns the norm of the resulting bidiagonal matrix, that is, the
 * maximal column sum.
 */

double MatrixSVD::_bidiagonalize(Vector       & super_diag,
                                 Matrix const & _A)
{
  double norm_acc = 0;
  super_diag[0] = 0;			// No superdiag elem above A(1,1)
  Matrix A = _A;			// A being transformed
  for (register unsigned i = 0; i < _N; ++i)
  {
    double const & diagi = _sig[i] = _left_householder(A, i);
    if (i < (_N - 1))
    {
      super_diag[i + 1] = _right_householder(A, i);
    }
    norm_acc = max(norm_acc, fabs(diagi) + fabs(super_diag[i]));
  }
  return norm_acc;
}

/*
 *------------------------------------------------------------------------
 *		QR-diagonalization of a bidiagonal matrix
 *
 * After bidiagonalization we get a bidiagonal matrix J:
 *    (1)  J = U' * A * V
 * The present method turns J into a matrix JJ by applying a set of
 * orthogonal transforms
 *    (2)  JJ = S' * J * T
 * Orthogonal matrices S and T are chosen so that JJ were also a
 * bidiagonal matrix, but with superdiag elements smaller than those of J.
 * We repeat (2) until non-diag elements of JJ become smaller than EPS
 * and can be disregarded.
 * Matrices S and T are constructed as
 *    (3)  S = S1 * S2 * S3 ... Sn, and similarly T
 * where Sk and Tk are matrices of simple rotations
 *    (4)  Sk[i,j] = i==j ? 1 : 0 for all i>k or i<k-1
 *         Sk[k-1,k-1] = cos(Phk),  Sk[k-1,k] = -sin(Phk),
 *         SK[k,k-1] = sin(Phk),    Sk[k,k] = cos(Phk), k=2..N
 * Matrix Tk is constructed similarly, only with angle Thk rather than Phk.
 *
 * Thus left multiplication of J by SK' can be spelled out as
 *    (5)  (Sk' * J)[i,j] = J[i,j] when i>k or i<k-1,
 *                  [k-1,j] = cos(Phk)*J[k-1,j] + sin(Phk)*J[k,j]
 *                  [k,j] =  -sin(Phk)*J[k-1,j] + cos(Phk)*J[k,j]
 * That is, k-1 and k rows of J are replaced by their linear combinations;
 * the rest of J is unaffected. Right multiplication of J by Tk similarly
 * changes only k-1 and k columns of J.
 * Matrix T2 is chosen the way that T2'J'JT2 were a QR-transform with a
 * shift. Note that multiplying J by T2 gives rise to a J[2,1] element of
 * the product J (which is below the main diagonal). Angle Ph2 is then
 * chosen so that multiplication by S2' (which combines 1 and 2 rows of J)
 * gets rid of that elemnent. But this will create a [1,3] non-zero element.
 * T3 is made to make it disappear, but this leads to [3,2], etc.
 * In the end, Sn removes a [N,N-1] element of J and matrix S'JT becomes
 * bidiagonal again. However, because of a special choice
 * of T2 (QR-algorithm), its non-diag elements are smaller than those of J.
 *
 * All this process in more detail is described in
 *    J.H. Wilkinson, C. Reinsch. Linear algebra - Springer-Verlag, 1971
 *
 * If during transforms (1), JJ[N-1,N] turns 0, then JJ[N,N] is a singular
 * number (possibly with a wrong (that is, negative) sign). This is a
 * consequence of Frantsis' Theorem, see the book above. In that case, we can
 * eliminate the N-th row and column of JJ and carry out further transforms
 * with a smaller matrix. If any other superdiag element of JJ turns 0,
 * then JJ effectively falls into two independent matrices. We will process
 * them independently (the bottom one first).
 *
 * Since matrix J is a bidiagonal, it can be stored efficiently. As a matter
 * of fact, its N diagonal elements are in array Sig, and superdiag elements
 * are stored in array super_diag.
 */
 
				// Carry out U * S with a rotation matrix
				// S (which combines i-th and j-th columns
				// of U, i>j)
static
void rotate(Matrix       & U,
            unsigned const i,
            unsigned const j,
            double   const cos_ph,
            double   const sin_ph)
{
  unsigned N = U.numRows();
  for (register unsigned k = 0; k < N; ++k)
  {
    double & Uil = U[k][i];
    double & Ujl = U[k][j];
    double Ujl_was = Ujl;
    Ujl =  cos_ph*Ujl_was + sin_ph*Uil;
    Uil = -sin_ph*Ujl_was + cos_ph*Uil;
  }
}

/*
 * A diagonal element J[l-1,l-1] turns out 0 at the k-th step of the
 * algorithm. That means that one of the original matrix' singular numbers
 * shall be zero. In that case, we multiply J by specially selected
 * matrices S' of simple rotations to eliminate a superdiag element J[l-1,l].
 * After that, matrix J falls into two pieces, which can be dealt with
 * in a regular way (the bottom piece first).
 * 
 * These special S transformations are accumulated into matrix U: since J
 * is left-multiplied by S', U would be right-multiplied by S. Transform
 * formulas for doing these rotations are similar to (5) above. See the
 * book cited above for more details.
 */
void MatrixSVD::_rip_through(Vector       & super_diag,
                             unsigned const k,
                             unsigned const l,
                             double   const eps)
{
  double cos_ph = 0;
  double sin_ph = 1;		// Accumulate cos,sin of Ph
  				// The first step of the loop below
  				// (when i==l) would eliminate J[l-1,l],
  				// which is stored in super_diag(l)
  				// However, it gives rise to J[l-1,l+1]
  				// and J[l,l+2]
  				// The following steps eliminate these
  				// until they fall below
  				// significance
  for (register unsigned i = l; i <= k; ++i)
  {
    double f = sin_ph*super_diag[i];
    super_diag[i] *= cos_ph;
    if (fabs(f) <= eps)
    {
      break;			// Current J[l-1,l] became unsignificant
    }
    cos_ph = _sig[i]; sin_ph = -f;	// unnormalized sin/cos
    double norm = _sig[i] = hypot(cos_ph, sin_ph); // sqrt(sin^2+cos^2)
    cos_ph /= norm; sin_ph /= norm;	// Normalize sin/cos
    rotate(_u, i, l - 1, cos_ph, sin_ph);
  }
}

			// We're about to proceed doing QR-transforms
			// on a (bidiag) matrix J[1:k,1:k]. It may happen
			// though that the matrix splits (or can be
			// split) into two independent pieces. This function
			// checks for splitting and returns the lowerbound
			// index l of the bottom piece, J[l:k,l:k]
unsigned MatrixSVD::_get_submatrix_to_work_on(Vector       & super_diag,
                                              unsigned const k,
                                              double   const eps)
{
  for (register unsigned l = k; l > 0; --l)
  {
    if (fabs(super_diag[l]) <= eps)
    {
      return l;				// The breaking point: zero J[l-1,l]
    }
    else if (fabs(_sig[l - 1]) <= eps)	// Diagonal J[l,l] turns out 0
    {					// meaning J[l-1,l] _can_ be made
      _rip_through(super_diag, k, l, eps);	// zero after some rotations
      return l;
    }
  }
  return 0;			// Deal with J[1:k,1:k] as a whole
}

		// Diagonalize root module
void MatrixSVD::_diagonalize(Vector     & super_diag,
                             double const eps)
{
  for (register int k = _N - 1; k >= 0; --k)  // QR-iterate upon J[l:k,l:k]
  {
    register unsigned l;
    while (l = _get_submatrix_to_work_on(super_diag, k, eps),
           fabs(super_diag[k]) > eps)	// until superdiag J[k-1,k] becomes 0
    {
      double shift;			// Compute a QR-shift from a bottom
      {					// corner minor of J[l:k,l:k] order 2
      	double Jk2k1 = super_diag[k-1],	// J[k-2,k-1]
      	       Jk1k  = super_diag[k],
      	       Jk1k1 = _sig[k-1],	// J[k-1,k-1]
      	       Jkk   = _sig[k],
      	       Jll   = _sig[l];		// J[l,l]
      	shift = (Jk1k1-Jkk)*(Jk1k1+Jkk) + (Jk2k1-Jk1k)*(Jk2k1+Jk1k);
      	shift /= 2*Jk1k*Jk1k1;
      	shift += (shift < 0 ? -1 : 1) * sqrt(shift*shift+1);
      	shift = ( (Jll-Jkk)*(Jll+Jkk) + Jk1k*(Jk1k1/shift-Jk1k) )/Jll;
      }
      				// Carry on multiplications by T2, S2, T3...
      double cos_th = 1, sin_th = 1;
      double Ji1i1 = _sig[l];	// J[i-1,i-1] at i=l+1...k
      for (register int i = l + 1; i <= k; ++i)
      {
      	double Ji1i = super_diag[i], Jii = _sig[i];  // J[i-1,i] and J[i,i]
      	sin_th *= Ji1i; Ji1i *= cos_th; cos_th = shift;
      	double norm_f = (super_diag[i-1] = hypot(cos_th,sin_th));
      	cos_th /= norm_f, sin_th /= norm_f;
      					// Rotate J[i-1:i,i-1:i] by Ti
      	shift = cos_th*Ji1i1 + sin_th*Ji1i;	// new J[i-1,i-1]
      	Ji1i = -sin_th*Ji1i1 + cos_th*Ji1i;	// J[i-1,i] after rotation
      	double Jii1 = Jii*sin_th;		// Emerged J[i,i-1]
      	Jii *= cos_th;				// new J[i,i]
        rotate(_v, i, i-1, cos_th, sin_th); // Accumulate T rotations in V
        
        double cos_ph = shift, sin_ph = Jii1;// Make Si to get rid of J[i,i-1]
        _sig[i-1] = (norm_f = hypot(cos_ph,sin_ph));	// New J[i-1,i-1]
        if( norm_f == 0 )		// If norm =0, rotation angle
        {
          cos_ph = cos_th, sin_ph = sin_th; // can be anything now
        }
        else
        {
          cos_ph /= norm_f, sin_ph /= norm_f;
        }
      					// Rotate J[i-1:i,i-1:i] by Si
        shift = cos_ph * Ji1i + sin_ph*Jii;	// New J[i-1,i]
        Ji1i1 = -sin_ph*Ji1i + cos_ph*Jii;	// New Jii, would carry over
        					// as J[i-1,i-1] for next i
        rotate(_u, i, i-1, cos_ph, sin_ph);  // Accumulate S rotations in U
        				// Jii1 disappears, sin_th would
        cos_th = cos_ph, sin_th = sin_ph; // carry over a (scaled) J[i-1,i+1]
        				// to eliminate on the next i, cos_th
        				// would carry over a scaled J[i,i+1]
      }
      super_diag[l] = 0;		// Supposed to be eliminated by now
      super_diag[k] = shift;
      _sig[k] = Ji1i1;
    }		// --- end-of-QR-iterations
    if (_sig[k] < 0)		// Correct the sign of the sing number
    {
      _sig[k] = -_sig[k];
      for (register unsigned i = 0; i < _N; ++i)
      {
        double & vk = _v[i][k];
        vk = -vk;
      }
    }
  }
} 


/*
 *------------------------------------------------------------------------
 *				The root Module
 */

MatrixSVD::MatrixSVD(Matrix const & A):
  _M(A.numRows()),
  _N(A.numCols()),
  _u(IdentityMatrix(_M)),
  _sig(_N),
  _w(_M, _N, 0.0),
  _v(IdentityMatrix(_N))
{
  if (_M < _N)
  {
    cerr << "MatrixSVD: Matrix A should have at least "
         << "as many rows as it has columns." << endl;
    throw Matrix::Exception();
  }

  Vector super_diag(_N);
  double bidiag_norm = _bidiagonalize(super_diag, A);
  double eps = FLT_EPSILON * bidiag_norm;	// Significance threshold
  _diagonalize(super_diag, eps);

  for (unsigned i = 0; i < _N; ++i)
  {
    _w[i][i] = _sig[i];
  }
}


ostream & operator<< (ostream         & s,
                      MatrixSVD const & svd)
{
  s << "U = " << svd.U() << endl;
  s << "W = " << svd.W() << endl;
  return s << "V = " << svd.V();
}
