//
// $Id: matrixTest.cpp,v 1.8 2003/08/07 17:47:28 knicewar Exp $
// 
// Copyright Keith Nicewarner.  All rights reserved.  See COPYING file
// in top source directory.
// 
// Run the Matrix classes through their courses.
// First release Date: Sep 20, 1994
// 

#include <iostream>
#include <stdexcept>

#include "Matrix.h"
#include "MatrixSVD.h"



using namespace std;
using namespace LinAlg;



#ifdef VXWORKS
extern "C"
int matrixTest()
#else
int main()
#endif
{
   try
   {

#if defined(_MSC_VER) && (_MSC_VER < 1300)
	// Without this, I get a "missing locale facet" exception!
	cout.imbue(locale::empty());
#endif

   Matrix A(3, 6, 3.21);
   cout << "A(3, 6, 3.21) = " << A << endl;

   double data[] = {  1,  2,  3,
		      4,  5,  6,
		      7,  8,  9,
		     10, 11, 12,
		     13, 14, 15,
		     16, 17, 18};
   Matrix B(6, 3, data);
   cout << "B(6, 3, data) = " << B << endl;

   cout << "B[3][0] = " << B[3][0] << endl;

   Matrix C(B);
   cout << "C(B) = " << C << endl;

   Matrix D;
   D = B;
   cout << "D = B = " << D << endl;

   Matrix E = -B;
   cout << "E = -B = " << E << endl;

   cout << "D = 3.1415 = " << (D = 3.1415) << endl;

   cout << "B += 0.1 = " << (B += 0.1) << endl;
   cout << "B *= 0.1 = " << (B *= 0.1) << endl;
   cout << "B -= 0.1 = " << (B -= 0.1) << endl;
   cout << "B /= 0.1 = " << (B /= 0.1) << endl;

   cout << "B += C = " << (B += C) << endl;
   cout << "B -= C = " << (B -= C) << endl;

   //cout << "B += A = " << (B += A) << endl;   // Error

   cout << "B + 2 = " << (B + 2) << endl;
   cout << "B * 2 = " << (B * 2) << endl;
   cout << "B - 2 = " << (B - 2) << endl;
   cout << "B / 2 = " << (B / 2) << endl;

   cout << "B + C = " << (B + C) << endl;
   cout << "B - C = " << (B - C) << endl;

   //cout << "B * C = " << (B * C) << endl;   // Error

   cout << "Matrix(1,6,1)*Matrix(6,1,1) = "
        <<  Matrix(1,6,1)*Matrix(6,1,1) << endl;   // test inner product case

   cout << "Matrix(3,6,1)*B = " << Matrix(3,6,1)*B << endl;

   cout << "-transpose(E) = " << -transpose(E) << endl;
   cout << "-E.transpose() = " << -E.transpose() << endl;

   SubMatrix F(E, Range<int>(1,2), Range<int>(3,4));
   cout << "SubMatrix F(E, Range<int>(1,2), Range<int>(3,4)) = " << F << endl;

   Matrix G(F);
   cout << "Matrix G(F) = " << G << endl;

   cout << "-F*2 + 1 = " << -F*2 + 1 << endl;
   cout << "E = " << E << endl;

   F = Matrix(2, 2, 1);
   cout << "F = Matrix(2, 2, 1) = " << F << endl;
   cout << "E = " << E << endl;
   cout << "G = " << G << endl;

   //F = Matrix(2, 3, 1);  // Error
   //cout << "F = Matrix(2, 3, 1) = " << F << endl;

   cout << "F *= 0.1 = " << (F *= 0.1) << endl;
   cout << "E = " << E << endl;

   cout << "E._(Range<int>(1,2), Range<int>(2,3)) *= G = "
        << (E._(Range<int>(1,2), Range<int>(2,3)) *= G) << endl;
   cout << "E = " << E << endl;

   cout << "F.transpose() = " << F.transpose() << endl;
   cout << "E = " << E << endl;

   // Error
   //cout << "E._(1,2, 7,8) *= G = " << (E._(1,2, 7,8) *= G) << endl;
   //cout << "E = " << E << endl;

   
   double data2[] = {  1,  2,  3,
		       6,  5,  4,
		       8,  9,  7  };
/*
invH =
   -0.0476    0.6190   -0.3333
   -0.4762   -0.8095    0.6667
    0.6667    0.3333   -0.3333
*/

   Matrix H(3, 3, data2);
   cout << "H = " << H << endl;

   Matrix Hinv = H.inverse();
   cout << "Hinv = H.inverse() = " << Hinv << endl;

   cout << "Hinv*H = " << Hinv*H << endl;
   cout << "H*Hinv = " << H*Hinv << endl;

   MatrixSVD Hsvd(H);
   cout << "Hsvd = " << Hsvd << endl;

   cout << "IdentityMatrix(3) = " << IdentityMatrix(3) << endl;
   cout << "2.3*IdentityMatrix(3) = " << 2.3*IdentityMatrix(3) << endl;

//   cout << "IdentityMatrix(3) = H" << (IdentityMatrix(3) = H) << endl;  // Compile Error

   cout << "H.det() = " << H.det() << endl;


   cout << "H.concatHorizontal(Hinv) = " << H.concatHorizontal(Hinv) << endl;
   cout << "H.concatVertical  (Hinv) = " << H.concatVertical  (Hinv) << endl;
   cout << "H.appendHorizontal(Hinv) = " << H.appendHorizontal(Hinv) << endl;
   Matrix tmp = H;
   cout << "H.appendVertical  (H) = " << H.appendVertical  (tmp) << endl;

   cout << "H.invert() = " << H.invert() << endl;

/*
   SubMatrix a;
   SubMatrix b(A, ...);
   a = Matrix(3,3);   // error?  submatrix is whole matrix.
   b = Matrix(3,3);   // Only valid if no resize
*/

/*
   Matrix A(MatrixEye(6));
   Matrix B(8,10);
   SubMatrix C(A, sRange(1,6), sRange(3,5));           // Uses A's data.
   Matrix D(SubMatrix(A, sRange(1,3), sRange(3,5)));   // Copies of part of A.
   
   B.subMatrix(sRange(1,6), sRange(5,10)) = A;

   Matrix E(20, 20);
   E.subMatrix(sRange(5,6), sRange(2,4)) = C.subMatrix(sRange(0,1),sRange(0,2)) * D;
   // Alternative:
   C = B._(sRange(2,7), sRange(4,9)); // Changes data in A.
   D = B._(sRange(2,3), sRange(2,3)); // Changes D.
   C = B._(sRange(2,3), sRange(2,3)); // Exception: dimension mismatch.
   E._(sRange(5,6), sRange(2,4)) = C._(sRange(0,1),sRange(0,2)) * D;
*/

   }
   catch (exception const & ex)
   {
	   cerr << "Unhandled exception: " << ex.what() << endl;
	   return -1;
   }
   catch (...)
   {
	   cerr << "Unhandled exception!" << endl;
	   return -1;
   }

   return 0;
}

