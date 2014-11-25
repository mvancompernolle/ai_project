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

#include <sstream>

#include "ltiLispStreamHandler.h"
#include "ltiBinaryStreamHandler.h"
#include "mathObject.h"
#include "jniLAFunctors.h"
#include "laCollection.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

/*
 * Class:     de_rwth_0005faachen_techinfo_lti
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_closeTheGate
  (JNIEnv *env, jobject f) {

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/LAFunctors");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  LAFunctorCollection *coll=jlong2lafunctor(env->GetLongField(f,fid));

  delete coll;
  env->SetLongField(f, fid, jlong(0));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    jacobi
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_jacobi
(JNIEnv *env, jobject f, jobject ma, jobject eValues, jobject eVectors) {

  getLAFunctor(env,f)->eigenSystem.apply(*getMatrix(env,ma),
                                         *getVector(env,eValues),
                                         *getMatrix(env,eVectors));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    matrixInversion
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_matrixInversion__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject ma) {

  getLAFunctor(env,f)->inverter.useLUD();

  getLAFunctor(env,f)->inverter.apply(*getMatrix(env,ma));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    matrixInversion
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_matrixInversion__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject ma, jobject mb) {

  getLAFunctor(env,f)->inverter.useLUD();
  getLAFunctor(env,f)->inverter.apply(*getMatrix(env,ma),*getMatrix(env,mb));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    openTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_openTheGate
  (JNIEnv *env, jobject f) {

  LAFunctorCollection *c;

  c=new LAFunctorCollection();
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/LAFunctors");
  jfieldID fid=env->GetFieldID(cls,"nativeObject", "J");

  env->SetLongField(f, fid, lafunctor2jlong(c));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    luDecompose
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_luDecompose__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject src, jobject dest, jobject perm) {

  // insert your code here
  lti::vector<int> tmp;
  int result;
  result=getLAFunctor(env,f)->luDecomposer.apply(*getMatrix(env,src),
                                                 *getMatrix(env,dest),
                                                 tmp);
  getVector(env,perm)->castFrom(tmp);
  return result;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    luDecompose
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_luDecompose__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f,  jobject srcdest, jobject perm) {

  // insert your code here
  lti::vector<int> tmp;
  int result;

  result=getLAFunctor(env,f)->luDecomposer.apply(*getMatrix(env,srcdest),tmp);
  getVector(env,perm)->castFrom(tmp);
  return result;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    det
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_det
  (JNIEnv *env, jobject f, jobject m) {

  // insert your code here
  return getLAFunctor(env,f)->luDecomposer.det(*getMatrix(env,m));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    l1Norm
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_l1Norm__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {

  // insert your code here
  return getLAFunctor(env,f)->l1norm.apply(*getMatrix(env,a),*getMatrix(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    l2Norm
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_l2Norm__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here
  return getLAFunctor(env,f)->l2norm.apply(*getMatrix(env,a),*getMatrix(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    l1Norm
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_l1Norm__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {

  // insert your code here
  return getLAFunctor(env,f)->l1norm.apply(*getVector(env,a),*getVector(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    l2Norm
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_l2Norm__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here
  _lti_showVar(a);
  _lti_showVar(b);
  if (a != 0 && b != 0) {
    lti::dvector* va=getVector(env,a);
    lti::dvector* vb=getVector(env,b);
    _lti_showVar(va);
    _lti_showVar(vb);
    if (va != 0 && vb != 0) {
      return getLAFunctor(env,f)->l2norm.apply(*va,*vb);
    }
  }
  // if we get here, something has gone wrong; either the java reference
  // a or b was null, or one of the C++ pointers contained in them was 0.
  jclass exc=env->FindClass("java/lang/NullPointerException");
  if (exc == 0) {
    std::cerr << "Unable to find exception class, giving up...\n";
    exit(1);
  }
  env->ThrowNew(exc, "Vector was null");
  return 0;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    usePCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_usePCA__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here
  getLAFunctor(env,f)->pca.transform(*getMatrix(env,a),*getMatrix(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makePCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makePCA
  (JNIEnv *env, jobject f, jobject a, jint dim, jobject b, jdouble kap) {
  // insert your code here

  lti::regularizedPCA<double>::parameters p=getLAFunctor(env,f)->pca.getParameters();
  p.kappa=kap;
  if (dim > 0) {
    p.resultDim=dim;
  } else {
    p.autoDim=true;
  }
  _lti_debug("before setParameters\n");
  getLAFunctor(env,f)->pca.setParameters(p);
  _lti_debug("before apply\n");
  if (getLAFunctor(env,f)->pca.apply(*getMatrix(env,a),*getMatrix(env,b))) {
    return jint(getLAFunctor(env,f)->pca.getUsedDimension());
  } else {
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    usePCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_usePCA__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here
  getLAFunctor(env,f)->pca.transform(*getVector(env,a),*getVector(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    setPCA
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_setPCA
  (JNIEnv *env, jobject f, jstring s)  {
  // insert your code here

  // create a string stream from s
  //std::cerr << "Hallo!" << std::endl;
  jboolean isCopy;
  const char *ch=env->GetStringUTFChars(s,&isCopy);
  _lti_debug("Chars in = " << ch << std::endl);
  lti::lispStreamHandler lis(ch);
  lti::read(lis,getLAFunctor(env,f)->pca);
  env->ReleaseStringUTFChars(s,ch);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    getPCA
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_getPCA
  (JNIEnv *env, jobject f) {
  // insert your code here
  std::ostringstream oss;
  lti::lispStreamHandler lis(oss);
  lti::write(lis,getLAFunctor(env,f)->pca);
  /*
  int n=oss.pcount();
  char *cstr=new char[n+1];
  strncpy(cstr,oss.str(),n);
  cstr[n]=0;
  _lti_debug("Chars out = " << cstr << std::endl);
  jstring tmp=env->NewStringUTF(cstr);
  delete[] cstr;
  */
  jstring tmp=env->NewStringUTF(oss.str().c_str());
  return tmp;
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    cos2Similarity
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_cos2Similarity
  (JNIEnv *env, jobject obj, jobject a, jobject b) {

  return getLAFunctor(env, obj)->cos2.apply(*getVector(env,a),*getVector(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    euclidianSimilarity
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_euclidianSimilarity
  (JNIEnv *env, jobject obj, jobject a, jobject b) {

  return getLAFunctor(env, obj)->euclidian.apply(*getVector(env,a),*getVector(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    svd
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_svd__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject obj, jobject a, jobject u, jobject w, jobject v) {
  // insert your code here
  getLAFunctor(env, obj)->svd.apply(*getMatrix(env,a), *getMatrix(env,u),
                                    *getVector(env,w), *getMatrix(env,v));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    svd
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_svd__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject obj, jobject au, jobject w, jobject v) {
  // insert your code here
  getLAFunctor(env, obj)->svd.apply(*getMatrix(env,au),
                                    *getVector(env,w), *getMatrix(env,v));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeFastPCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeFastPCA
  (JNIEnv *env, jobject f, jobject a, jint dim, jobject b, jdouble kap) {
  // insert your code here

#ifdef HAVE_LAPACK
  _lti_debug("A");
  lti::regularizedPCA<double>::parameters p=getLAFunctor(env,f)->pca.getParameters();
  _lti_debug("B");
  delete p.eigen;
  _lti_debug("C");
  p.eigen=new lti::generalEigenVectors<double>;
  p.kappa=kap;
  _lti_debug("D");
  if (dim > 0) {
    p.resultDim=int(dim);
  } else {
    p.autoDim=true;
  }
  _lti_debug("E");
  getLAFunctor(env,f)->pca.setParameters(p);
  _lti_debug("F");
  if (getLAFunctor(env,f)->pca.apply(*getMatrix(env,a),*getMatrix(env,b))) {
    _lti_debug("G");
    return jint(getLAFunctor(env,f)->pca.getUsedDimension());
  } else {
    return 0;
  }
#else
#warning "Not configured for LAPACK. makeFastPCA will use makePCA"
  return Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makePCA(env,f,a,dim,b,kap);
#endif
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    getBinaryPCA
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_getBinaryPCA
  (JNIEnv *env, jobject f) {
  // insert your code here
  std::ostringstream oss(std::ios::binary);
  lti::binaryStreamHandler lis(oss);
  lti::write(lis,getLAFunctor(env,f)->pca);
  /*
  char *cstr=oss.str();
  int size=oss.pcount();
  */
  const std::string &str=oss.str();
  int size=str.length();

  // now we have a binary string, now copy the elements
  jboolean isCopy;
  jbyteArray tmp=env->NewByteArray(size);
  jbyte* tmpptr=env->GetByteArrayElements(tmp, &isCopy);

  //memcpy(tmpptr,cstr,size);
  memcpy(tmpptr, str.c_str(), size);
  env->ReleaseByteArrayElements(tmp, tmpptr, 0);

  //delete[] cstr;
  return tmp;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    setBinaryPCA
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_setBinaryPCA
  (JNIEnv *env, jobject f, jbyteArray data) {

  //cerr << "A";
  jboolean isCopy;
  jbyte* tmpptr=env->GetByteArrayElements(data, &isCopy);
  //cerr << "B";
  char *buf=reinterpret_cast<char *>(tmpptr);
  //cerr << "C";

  int size=env->GetArrayLength(data);

  /* cerr << "Array has " << size << "elements.\n";
  for (int i=0; i<size; i++) {
    cerr << " " << int(buf[i]);
  }
  */

  std::string bufstr(buf,size);
  std::istringstream* iss=new std::istringstream(bufstr,std::ios::binary);
  lti::binaryStreamHandler lis(*iss);
  getLAFunctor(env,f)->pca.read(lis);
  delete iss;
  env->ReleaseByteArrayElements(data,tmpptr,JNI_ABORT);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeWhite
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeWhite
  (JNIEnv *env, jobject f, jobject a, jint dim, jobject b, jdouble kap) {
  // insert your code here
  lti::regularizedPCA<double>::parameters p;
  p.kappa=kap;
  p.whitening=true;
  p.useCorrelation=false;
  if (dim > 0) {
    p.resultDim=dim;
  } else {
    p.autoDim=true;
  }
  getLAFunctor(env,f)->pca.setParameters(p);
  if (getLAFunctor(env,f)->pca.apply(*getMatrix(env,a),*getMatrix(env,b))) {
    return jint(getLAFunctor(env,f)->pca.getUsedDimension());
  } else {
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeFastWhite
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeFastWhite
  (JNIEnv *env, jobject f, jobject a, jint dim, jobject b, jdouble kap) {

#ifdef HAVE_LAPACK
  _lti_debug("A");
  lti::regularizedPCA<double>::parameters p=getLAFunctor(env,f)->pca.getParameters();
  _lti_debug("B");
  delete p.eigen;
  _lti_debug("C");
  p.eigen=new lti::generalEigenVectors<double>;
  p.useCorrelation=false;
  p.whitening=true;
  p.kappa=kap;
  _lti_debug("D");
  if (dim > 0) {
    p.resultDim=int(dim);
  } else {
    p.autoDim=true;
  }
  _lti_debug("E");
  getLAFunctor(env,f)->pca.setParameters(p);
  _lti_debug("F");
  if (getLAFunctor(env,f)->pca.apply(*getMatrix(env,a),*getMatrix(env,b))) {
    _lti_debug("G");
    return jint(getLAFunctor(env,f)->pca.getUsedDimension());
  } else {
    return 0;
  }
#else
#warning "Not configured for LAPACK. makeFastWhite will use makeWhite"
  return Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeWhite(env,f,a,dim,b,kap);
#endif
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    emphasizeDiagonal
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;D)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_emphasizeDiagonal
  (JNIEnv *env, jobject f, jobject a, jdouble c) {
  // insert your code here

  double cf=double(c);

  _lti_debug("cf = " << cf << std::endl);

  lti::dmatrix &mat=*getMatrix(env,a);

  _lti_debug("mat = " << mat.size() << std::endl);

  int dim=mat.rows() <= mat.columns() ? mat.rows() : mat.columns();
  int i=0;

   _lti_debug("dim = " << dim << std::endl);

  lti::dvector x;
  mat.getDiagonal(x);

  _lti_debug("diag = " << x << std::endl);

  mat.multiply(1.0-cf);

  for (i=0; i<dim; i++) {
    mat.at(i,i)+=x.at(i)*cf;
  }

  _lti_debug("e"  << std::endl);

  double det=getLAFunctor(env,f)->luDecomposer.det(mat);

  _lti_debug("det = " << det << std::endl);

  double eps=getLAFunctor(env,f)->luDecomposer.my_epsilon;
  double seps=std::numeric_limits<double>::epsilon();

  if (lti::abs(det) < eps) {
    // still singular
    for (i=0; i<dim; i++) {
      if (mat.at(i,i) < seps) {
        mat.at(i,i)=seps;
      }
    }
  }
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    safeMatrixInversion
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_safeMatrixInversion__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject ma, jobject mb) {

  getLAFunctor(env,f)->inverter.useSVD();
  getLAFunctor(env,f)->inverter.apply(*getMatrix(env,ma),*getMatrix(env,mb));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    safeMatrixInversion
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_safeMatrixInversion__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject ma) {

  getLAFunctor(env,f)->inverter.useSVD();
  getLAFunctor(env,f)->inverter.apply(*getMatrix(env,ma));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    setBinarySeqPCA
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_setBinarySeqPCA
  (JNIEnv *env, jobject f, jbyteArray data) {

  //cerr << "A";
  jboolean isCopy;
  jbyte* tmpptr=env->GetByteArrayElements(data, &isCopy);
  //cerr << "B";
  char *buf=reinterpret_cast<char *>(tmpptr);
  //cerr << "C";

  int size=env->GetArrayLength(data);

  /* cerr << "Array has " << size << "elements.\n";
  for (int i=0; i<size; i++) {
    cerr << " " << int(buf[i]);
  }
  */

  std::string bufstr(buf,size);

  std::istringstream* iss=new std::istringstream(bufstr,std::ios::binary);
  lti::binaryStreamHandler lis(*iss);
  getLAFunctor(env,f)->spca.read(lis);
  delete iss;
  env->ReleaseByteArrayElements(data,tmpptr,JNI_ABORT);
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeFastSeqPCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeFastSeqPCA
  (JNIEnv *env, jobject f, jobject a, jint dim) {
  // insert your code here

#ifdef HAVE_LAPACK
  _lti_debug("A");
  lti::serialPCA<double>::parameters p=getLAFunctor(env,f)->spca.getParameters();
  _lti_debug("B");
  delete p.eigen;
  _lti_debug("C");
  p.eigen=new lti::generalEigenVectors<double>;
  _lti_debug("D");
  if (dim > 0) {
    p.resultDim=int(dim);
  } else {
    p.autoDim=true;
  }
  _lti_debug("E");
  getLAFunctor(env,f)->spca.setParameters(p);
  _lti_debug("F");
  if (getLAFunctor(env,f)->spca.consider(*getMatrix(env,a))) {
    _lti_debug("G");
    return jint(getLAFunctor(env,f)->spca.getUsedDimension());
  } else {
    return 0;
  }
#else
#warning "Not configured for LAPACK. makeFastSetPCA will use makeSeqPCA"
  return Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeSeqPCA(env,f,a,dim);
#endif
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    getBinarySeqPCA
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_getBinarySeqPCA
  (JNIEnv *env, jobject f) {
  // insert your code here
  std::ostringstream oss(std::ios::binary);
  lti::binaryStreamHandler lis(oss);
  lti::write(lis,getLAFunctor(env,f)->spca);
  /*
  char *cstr=oss.str();
  int size=oss.pcount();
  */
  const std::string& str=oss.str();
  int size=str.length();

  // now we have a binary string, now copy the elements
  jboolean isCopy;
  jbyteArray tmp=env->NewByteArray(size);
  jbyte* tmpptr=env->GetByteArrayElements(tmp, &isCopy);
  //memcpy(tmpptr,cstr,size);
  memcpy(tmpptr,str.c_str(),size);
  env->ReleaseByteArrayElements(tmp, tmpptr, 0);

  //delete[] cstr;
  return tmp;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    getSeqPCA
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_getSeqPCA
  (JNIEnv *env, jobject f) {
  // insert your code here
  std::ostringstream oss;
  lti::lispStreamHandler lis(oss);
  lti::write(lis,getLAFunctor(env,f)->spca);
  /*
  int n=oss.pcount();
  char *cstr=new char[n+1];
  strncpy(cstr,oss.str(),n);
  cstr[n]=0;
  _lti_debug("Chars out = " << cstr << std::endl);
  jstring tmp=env->NewStringUTF(cstr);
  delete[] cstr;
  */
  jstring tmp=env->NewStringUTF(oss.str().c_str());
  return tmp;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeFastSeqWhite
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeFastSeqWhite
  (JNIEnv *env, jobject f, jobject a, jint dim) {

#ifdef HAVE_LAPACK
  _lti_debug("A");
  lti::serialPCA<double>::parameters p=getLAFunctor(env,f)->spca.getParameters();
  _lti_debug("B");
  delete p.eigen;
  _lti_debug("C");
  p.eigen=new lti::generalEigenVectors<double>;
  p.useCorrelation=false;
  p.whitening=true;
  _lti_debug("D");
  if (dim > 0) {
    p.resultDim=int(dim);
  } else {
    p.autoDim=true;
  }
  _lti_debug("E");
  getLAFunctor(env,f)->spca.setParameters(p);
  _lti_debug("F");
  if (getLAFunctor(env,f)->spca.consider(*getMatrix(env,a))) {
    _lti_debug("G");
    return jint(getLAFunctor(env,f)->spca.getUsedDimension());
  } else {
    return 0;
  }
#else
#warning "Not configured for LAPACK. makeFastSeqWhite will use makeSeqWhite"
  return Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeSeqWhite(env,f,a,dim);
#endif
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeSeqWhite
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeSeqWhite
  (JNIEnv *env, jobject f, jobject a, jint dim) {
  // insert your code here
  lti::serialPCA<double>::parameters p;
  p.whitening=true;
  p.useCorrelation=false;
  if (dim > 0) {
    p.resultDim=dim;
  } else {
    p.autoDim=true;
  }
  getLAFunctor(env,f)->spca.setParameters(p);
  if (getLAFunctor(env,f)->spca.consider(*getMatrix(env,a))) {
    return jint(getLAFunctor(env,f)->spca.getUsedDimension());
  } else {
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    makeSeqPCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_makeSeqPCA
  (JNIEnv *env, jobject f, jobject a, jint dim) {
  // insert your code here

  if (dim > 0) {
    getLAFunctor(env,f)->spca.setDimension(int(dim));
  } else {
    lti::serialPCA<double>::parameters p=getLAFunctor(env,f)->spca.getParameters();
    p.autoDim=true;
    getLAFunctor(env,f)->spca.setParameters(p);
  }
  if (getLAFunctor(env,f)->spca.consider(*getMatrix(env,a))) {
    return jint(getLAFunctor(env,f)->spca.getUsedDimension());
  } else {
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    useSeqPCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_useSeqPCA__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here

  getLAFunctor(env,f)->spca.transform(*getMatrix(env,a),*getMatrix(env,b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    setSeqPCA
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_setSeqPCA
  (JNIEnv *env, jobject f, jstring s)  {
  // insert your code here

  // create a string stream from s
  //std::cerr << "Hallo!" << std::endl;
  jboolean isCopy;
  const char *ch=env->GetStringUTFChars(s,&isCopy);
  _lti_debug("Chars in = " << ch << std::endl);
  lti::lispStreamHandler lis(ch);
  lti::read(lis,getLAFunctor(env,f)->spca);
  env->ReleaseStringUTFChars(s,ch);
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_LAFunctors
 * Method:    useSeqPCA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_LAFunctors_useSeqPCA__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here

  getLAFunctor(env,f)->spca.transform(*getVector(env,a),*getVector(env,b));
}

