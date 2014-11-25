/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mathObject.h"
#include "jniMatrix.h"
#include "ltiVector.h"
#include "ltiMatrix.h"
#include "ltiAssert.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 2
#include "ltiDebug.h"

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _getRow
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1getRow
  (JNIEnv *env, jobject m, jlong nat, jint row) {

  jniMatrix *mat=jlong2matrix(nat);
  jniVector *vec=new jniVector();
 
  _lti_debug("getting row " << row << " from matrix " << mat->size() << "\n");
  mat->getRowCopy(int(row),*vec);
  return vector2jlong(vec);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _getColumn
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1getColumn
  (JNIEnv *env, jobject m, jlong nat, jint col) {

  jniMatrix *mat=jlong2matrix(nat);
  jniVector *vec=new jniVector();
  _lti_debug("getting column " << col << " from matrix " << mat->size() << "\n");
  mat->getColumnCopy(int(col),*vec);
  return vector2jlong(vec);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _getRows
 * Signature: ([I)J
 */
JNIEXPORT jlong JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1getRows
  (JNIEnv *env, jobject m, jlong nat, jintArray jindex) {
  // insert your code here
  jniMatrix *mat=jlong2matrix(nat);
  int n=env->GetArrayLength(jindex);
  jboolean isCopy=JNI_FALSE;
  int* index=env->GetIntArrayElements(jindex,&isCopy);
  jniMatrix *smat=new jniMatrix(n,mat->columns());
  for (int i=0; i<n; i++) {
    if (index[i] >= 0 && index[i] < mat->rows()) {
      smat->setRow(i,mat->getRow(index[i]));
    }
  }
  env->ReleaseIntArrayElements(jindex,index,JNI_ABORT);
  return matrix2jlong(smat);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _getColumns
 * Signature: ([I)J
 */
JNIEXPORT jlong JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1getColumns
  (JNIEnv *env, jobject m, jlong nat, jintArray jindex) {
  // insert your code here
  jniMatrix *mat=jlong2matrix(nat);
  int n=env->GetArrayLength(jindex);
  jboolean isCopy=JNI_FALSE;
  int* index=env->GetIntArrayElements(jindex,&isCopy);
  jniMatrix *smat=new jniMatrix(mat->rows(),n);
  for (int i=0; i<n; i++) {
    if (index[i] >= 0 && index[i] < mat->columns()) {
      smat->setColumn(i,mat->getColumnCopy(index[i]));
    }
  }
  env->ReleaseIntArrayElements(jindex,index,JNI_ABORT);
  return matrix2jlong(smat);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    openTheGate
 * Signature: (IID)J
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix_openTheGate
  (JNIEnv *env, jobject m, jint rows, jint cols, jdouble iniValue) {

  //jfieldID fid;
  jniMatrix *v;

  v=new jniMatrix(rows,cols,iniValue);
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/Matrix");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");

  _lti_debug("[");

  env->SetLongField(m, fid, matrix2jlong(v));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    closeTheGate
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix_closeTheGate
  (JNIEnv *env, jobject m) {

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/Matrix");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jniMatrix *mat=jlong2matrix(env->GetLongField(m,fid));

  _lti_debug("]");

  env->SetLongField(m, fid, jlong(0));

  delete mat;
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _rows
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1rows
  (JNIEnv *env, jobject m, jlong nat) {

  return jlong2matrix(nat)->rows();
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _columns
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1columns
  (JNIEnv *env, jobject m, jlong nat) {

  return jlong2matrix(nat)->columns();
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _resize
 * Signature: (JIID)I
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1resize
  (JNIEnv *env, jobject m, jlong nat, jint rows, jint cols, jdouble iniValue) {

  jlong2matrix(nat)->resize(rows,cols,iniValue);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _fill
 * Signature: (JDIIII)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1fill__JDIIII
  (JNIEnv *env, jobject m, jlong nat, jdouble iniValue, jint fromRow,
   jint fromCol,  jint toRow, jint toCol) {

  jlong2matrix(nat)->fill(iniValue,fromRow,fromCol,toRow,toCol);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _fill
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1fill__JD
  (JNIEnv *env, jobject m, jlong nat, jdouble iniValue) {

  jlong2matrix(nat)->fill(iniValue);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _at
 * Signature: (JII)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1at
  (JNIEnv *env, jobject m, jlong nat,  jint row, jint col) {

  return jlong2matrix(nat)->at(row,col);
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _setAt
 * Signature: (JIID)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1setAt
  (JNIEnv *env, jobject m, jlong nat,  jint row, jint col, jdouble value) {

  jlong2matrix(nat)->at(row,col)=double(value);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _minimum
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1minimum__J
  (JNIEnv *env, jobject m, jlong nat) {

  // insert your code here
  return jdouble(jlong2matrix(nat)->minimum());
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _minimum
 * Signature: (J[I)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1minimum__J_3I
  (JNIEnv *env, jobject m, jlong nat, jintArray indx) {
  // insert your code here
  lti::dmatrix* ltim=jlong2matrix(nat);
  lti::point p=ltim->getIndexOfMinimum();
  jint tmp[2];
  tmp[0]=p.x;
  tmp[1]=p.y;
  env->SetIntArrayRegion(indx,0,2,tmp);
  return jdouble(ltim->at(p));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _maximum
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1maximum__J
  (JNIEnv *env, jobject m, jlong nat) {
  // insert your code here
  return jdouble(jlong2matrix(nat)->maximum());
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _maximum
 * Signature: (J[I)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1maximum__J_3I
  (JNIEnv *env, jobject m, jlong nat, jintArray indx) {
  // insert your code here
  lti::dmatrix* ltim=jlong2matrix(nat);
  lti::point p=ltim->getIndexOfMaximum();
  jint tmp[2];
  tmp[0]=p.x;
  tmp[1]=p.y;
  env->SetIntArrayRegion(indx,0,2,tmp);
  return jdouble(ltim->at(p));
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _trace
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1trace
  (JNIEnv *env, jobject m, jlong nat) {
  // insert your code here
  return jdouble(jlong2matrix(nat)->trace());
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _getDiagonal
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1getDiagonal
  (JNIEnv *env, jobject m, jlong nat, jlong result) {
  // insert your code here
  jlong2matrix(nat)->getDiagonal(*jlong2vector(result));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _setRow
 * Signature: (JILde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1setRow
  (JNIEnv *env, jobject m, jlong nat, jint row, jlong vec) {

  _lti_debug("SR: " << jlong2matrix(nat) << " <- " << jlong2vector(vec)<<"\n");
  _lti_debug("SR: " << jlong2matrix(nat)->rows() << " " << row << "\n");


  jlong2matrix(nat)->setRow(row, *jlong2vector(vec));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _setColumn
 * Signature: (JILde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1setColumn
  (JNIEnv *env, jobject m, jlong nat, jint col, jlong vec) {

  jlong2matrix(nat)->setColumn(col,*jlong2vector(vec));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _copy
 * Signature: (JJIIII)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1copy__JJIIII
  (JNIEnv *env, jobject m, jlong nat, jlong other, jint fromRow, jint toRow,
   jint fromCol, jint toCol) {

  jlong2matrix(nat)->copy(*jlong2matrix(other),fromRow,toRow,fromCol,toCol);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _copy
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1copy__JJ
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  jlong2matrix(nat)->copy(*jlong2matrix(other));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _compare
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1compare
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  return jboolean(*jlong2matrix(nat) == *jlong2matrix(other));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _emultiply
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1emultiply__JJ
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  jlong2matrix(nat)->emultiply(*jlong2matrix(other));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _emultiply
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1emultiply__JJJ
  (JNIEnv *env, jobject m, jlong nat, jlong a, jlong b) {

  jlong2matrix(nat)->emultiply(*jlong2matrix(a), *jlong2matrix(b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _add
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1add__JJ
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  jlong2matrix(nat)->add(*jlong2matrix(other));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _add
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1add__JJJ
  (JNIEnv *env, jobject m, jlong nat, jlong a, jlong b) {

  jlong2matrix(nat)->add(*jlong2matrix(a), *jlong2matrix(b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _add
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1add__JD
  (JNIEnv *env, jobject m, jlong nat, jdouble s) {

  jlong2matrix(nat)->add(double(s));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _subtract
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1subtract__JJ
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  jlong2matrix(nat)->subtract(*jlong2matrix(other));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _subtract
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1subtract__JJJ
  (JNIEnv *env, jobject m, jlong nat, jlong a, jlong b) {

  jlong2matrix(nat)->subtract(*jlong2matrix(a),*jlong2matrix(b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _multiply
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1multiply__JD
  (JNIEnv *env, jobject m, jlong nat, jdouble s) {

  jlong2matrix(nat)->multiply(s);
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _multiply
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1multiply__JJ
(JNIEnv *env, jobject m, jlong nat, jlong other) {

  jlong2matrix(nat)->multiply(*jlong2matrix(other));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _multiply
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1multiply__JJJ
(JNIEnv *env, jobject m, jlong nat, jlong first, jlong second) {

  jlong2matrix(nat)->multiply(*jlong2matrix(first),
                               *jlong2matrix(second));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _multiplyV
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1multiplyV
(JNIEnv *env, jobject m, jlong nat, jlong other, jlong result) {

  jlong2matrix(nat)->multiply(*jlong2vector(other),
                               *jlong2vector(result));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _leftMultiply
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1leftMultiply__JJJ
  (JNIEnv *env, jobject m, jlong nat, jlong src, jlong dest) {

  jlong2matrix(nat)->leftMultiply(*jlong2vector(src),*jlong2vector(dest));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _leftMultiply
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1leftMultiply__JJ
  (JNIEnv *env, jobject m, jlong nat, jlong v) {

  jlong2matrix(nat)->leftMultiply(*jlong2vector(v));
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _outerProduct
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1outerProduct
(JNIEnv *env, jobject m, jlong nat, jlong first, jlong second) {

  jlong2matrix(nat)->outerProduct(*jlong2vector(first),
                                   *jlong2vector(second));
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _transpose
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1transpose
  (JNIEnv *env, jobject m, jlong nat) {

  jlong2matrix(nat)->transpose();
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _equals
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Matrix;)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1equals
  (JNIEnv *env, jobject m, jlong nat, jlong other) {

  // insert your code here
  return jboolean(jlong2matrix(nat)->equals(*jlong2matrix(other)));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _prettyCloseTo
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Matrix;D)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1prettyCloseTo
  (JNIEnv *env, jobject m, jlong nat, jlong other, jdouble tol) {

  // insert your code here
  return jboolean(jlong2matrix(nat)
->prettyCloseTo(*jlong2matrix(other),tol));
}


// ===



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    copyCtoJava
 * Signature: ([[FII)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix_copyCtoJava___3_3FII
  (JNIEnv *env, jobject obj,  jobjectArray data, jint row, jint col) {
  // insert your code here
  jniMatrix *mat;

  jclass cls=env->GetObjectClass(obj);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(obj,fid);

  if (id!=0) {
    mat=jlong2matrix(id);
  } else {
    data=NULL;
    return;
  }

  lti::fmatrix tmp;
  tmp.castFrom(*mat);
  
  for (int i=0; i<tmp.rows(); i++) {
    jfloatArray d=(jfloatArray)env->GetObjectArrayElement(data, i);
    env->SetFloatArrayRegion(d, 0, tmp.columns(), &(tmp.at(i,0)));
//      env->SetObjectArrayElement(data, i, d);
    env->DeleteLocalRef(d);
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    copyCtoJava
 * Signature: ([[DII)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix_copyCtoJava___3_3DII
  (JNIEnv *env, jobject obj,  jobjectArray data, jint row, jint col) {
  // insert your code here
  jniMatrix *mat;

  jclass cls=env->GetObjectClass(obj);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(obj,fid);

  if (id!=0) {
    mat=jlong2matrix(id);
  } else {
    data=NULL;
    return;
  }

  for (int i=0; i<mat->rows(); i++) {
    jdoubleArray d=(jdoubleArray)env->GetObjectArrayElement(data, i);
    env->SetDoubleArrayRegion(d, 0, mat->columns(), &(mat->at(i,0)));
//      env->SetObjectArrayElement(data, i, d);
    env->DeleteLocalRef(d);
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _setData
 * Signature: (J[[F)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1setData__J_3_3F
(JNIEnv *env, jobject m, jlong nat, jobjectArray data) {

  jniMatrix* mat;

  jclass cls=env->GetObjectClass(m);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(m,fid);

  if (id!=0) {
    mat=jlong2matrix(id);
  } else {
    mat=new jniMatrix();
  }

  jboolean copy=JNI_FALSE;
  int rows=env->GetArrayLength(data);
  if (rows > 0) {
    int cols=0;
    jfloatArray curr=jfloatArray(env->GetObjectArrayElement(data, 0));
    cols=env->GetArrayLength(curr);
    float* elem=env->GetFloatArrayElements(curr, &copy);
    double* elements=new double[rows*cols];
    for (int j=0; j<cols; j++) {
      elements[j]=elem[j];
    }

    env->ReleaseFloatArrayElements(curr, elem, JNI_ABORT);

    for (int i=1; i<rows; i++) {
      curr=jfloatArray(env->GetObjectArrayElement(data, i));
      if (env->GetArrayLength(curr) != cols) {
        //Error
        fprintf(stderr,"Error in jniMatrix: unequal cols");
      }
      elem=env->GetFloatArrayElements(curr, &copy);
      for (int j=0; j<cols; j++) {
        elements[i*cols+j]=elem[j];
      }
      env->ReleaseFloatArrayElements(curr, elem, JNI_ABORT);
    }

    mat->attach(rows, cols, elements);
  }

  env->SetLongField(m, fid, matrix2jlong(mat));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Matrix
 * Method:    _setData
 * Signature: (J[[D)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Matrix__1setData__J_3_3D
(JNIEnv *env, jobject m, jlong nat, jobjectArray data) {

  jniMatrix* mat;

  jclass cls=env->GetObjectClass(m);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(m,fid);

  if (id!=0) {
    mat=jlong2matrix(id);
  } else {
    mat=new jniMatrix();
  }

  jboolean copy=JNI_FALSE;
  int rows=env->GetArrayLength(data);
  if (rows > 0) {
    int cols=0;
    jdoubleArray curr=jdoubleArray(env->GetObjectArrayElement(data, 0));
    cols=env->GetArrayLength(curr);
    double* elem=env->GetDoubleArrayElements(curr, &copy);
    double* elements=new double[rows*cols];
    for (int j=0; j<cols; j++) {
      elements[j]=elem[j];
    }

    env->ReleaseDoubleArrayElements(curr, elem, JNI_ABORT);

    for (int i=1; i<rows; i++) {
      curr=jdoubleArray(env->GetObjectArrayElement(data, i));
      if (env->GetArrayLength(curr) != cols) {
        //Error
        printf("Error in jniMatrix: unequal cols");
      }
      elem=env->GetDoubleArrayElements(curr, &copy);
      for (int j=0; j<cols; j++) {
        elements[i*cols+j]=elem[j];
      }
      env->ReleaseDoubleArrayElements(curr, elem, JNI_ABORT);
    }

    mat->attach(rows, cols, elements);
  }

  env->SetLongField(m, fid, matrix2jlong(mat));
}

