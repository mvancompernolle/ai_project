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

// $Id: jniStatFunctors.cpp,v 1.3 2004/11/04 21:56:17 wickel Exp $

#include <sstream>
#include "ltiLispStreamHandler.h"

#include "jniStatFunctors.h"
#include "mathObject.h"
#include "statCollection.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

inline void resetStatus(JNIEnv *env, jobject f) {
  getStatFunctor(env,f)->status="ok";
}

// these macros are defined depending on the installation of LAPACK:
// - if LAPACK is installed, they reset the status string
// - otherwise, they set a message and abort the using function

#ifdef HAVE_LAPACK
#define checkLAPACK(env,f) resetStatus(env,f)
#define checkLAPACKreturn(env,f,r) resetStatus(env,f)
#else
#define checkLAPACKreturn(env,f,r) \
  do { \
    getStatFunctor(env,f)->status="LAPACK not installed"; \
    std::cerr << "WARNING: LAPACK is not installed.\n"; 
    return r; \
  } while (0)
#define checkLAPACK(env,f) \
  do { \
    getStatFunctor(env,f)->status="LAPACK not installed"; \
    std::cerr << "WARNING: LAPACK is not installed.\n"; 
    return; \
  } while (0)
#endif
  

inline void noLapack(JNIEnv *env, jobject f) {
  getStatFunctor(env,f)->status="LAPACK not installed";
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_closeTheGate
(JNIEnv *env, jobject f) {
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/StatFunctors");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  StatFunctorCollection *coll=jlong2statfunctor(env->GetLongField(f,fid));
  delete coll;
  env->SetLongField(f, fid, jlong(0));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    covarianceMatrixOfColumns
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_covarianceMatrixOfColumns
(JNIEnv *env, jobject f, jobject data, jobject rmat) {

  getStatFunctor(env,f)->variance.covarianceMatrixOfColumns(*getMatrix(env,data),
                                                           *getMatrix(env,rmat));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    covarianceMatrixOfRows
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_covarianceMatrixOfRows
(JNIEnv *env, jobject f, jobject data, jobject rmat) {

  getStatFunctor(env,f)->variance.covarianceMatrixOfRows(*getMatrix(env,data),
                                                         *getMatrix(env,rmat));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    meanOfColumns
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_meanOfColumns
(JNIEnv *env, jobject f, jobject data, jobject rvec) {

  getStatFunctor(env,f)->means.meanOfColumns(*getMatrix(env,data),
                                             *getVector(env,rvec));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    meanOfRows
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_meanOfRows
(JNIEnv *env, jobject f, jobject data, jobject rvec) {

  getStatFunctor(env,f)->means.meanOfRows(*getMatrix(env,data),
                                         *getVector(env,rvec));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    openTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_openTheGate
(JNIEnv *env, jobject f) {

  StatFunctorCollection *c;

  c=new StatFunctorCollection();
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/StatFunctors");
  jfieldID fid=env->GetFieldID(cls,"nativeObject", "J");

  env->SetLongField(f, fid, statfunctor2jlong(c));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    varianceOfColumns
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_varianceOfColumns
(JNIEnv *env, jobject f, jobject data, jobject rvec) {

  getStatFunctor(env,f)->variance.varianceOfColumns(*getMatrix(env,data),
                                                   *getVector(env,rvec));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    varianceOfRows
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_varianceOfRows
(JNIEnv *env, jobject f, jobject data, jobject rvec) {

  getStatFunctor(env,f)->variance.varianceOfRows(*getMatrix(env,data),
                                                *getVector(env,rvec));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    fillGaussVector
 * Signature: (DDLde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_fillGaussVector
  (JNIEnv *env, jobject f,  jdouble mu, jdouble sigma, jobject vec) {
  // insert your code here

  lti::gaussianDistribution::parameters p;
  lti::gaussianDistribution &gauss=getStatFunctor(env,f)->gaussDist;
  p.mu=double(mu);
  p.sigma=sqrt(double(sigma));
  gauss.setParameters(p);

  lti::vector<double> *d=getVector(env,vec);

  for (int i=0; i < d->size(); i++) {
    d->at(i)=gauss.draw();
  }
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    meanOfVector
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_meanOfVector
  (JNIEnv *env, jobject f, jobject vec) {

  double temp;
  getStatFunctor(env,f)->means.apply(*getVector(env,vec),
                                     temp);
  return jdouble(temp);
}








/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    boundsOfRows
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_boundsOfRows
  (JNIEnv *env, jobject f, jobject data, jobject min, jobject max) {
  // insert your code here
  getStatFunctor(env,f)->bounds.boundsOfRows(*getMatrix(env,data),
                                             *getVector(env,min),
                                             *getVector(env,max));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    boundsOfColumns
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_boundsOfColumns
  (JNIEnv *env, jobject f, jobject data, jobject min, jobject max) {
  // insert your code here
  getStatFunctor(env,f)->bounds.boundsOfColumns(*getMatrix(env,data),
                                             *getVector(env,min),
                                             *getVector(env,max));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    min
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_min
  (JNIEnv *env, jobject f, jobject a, jobject b, jobject result) {
  // insert your code here
  getStatFunctor(env,f)->bounds.min(*getVector(env,a),*getVector(env,b),

                                    *getVector(env,result));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    max
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_max
  (JNIEnv *env, jobject f, jobject a, jobject b, jobject result) {
  // insert your code here
  getStatFunctor(env,f)->bounds.max(*getVector(env,a),*getVector(env,b),
                                    *getVector(env,result));
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    entropy
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_entropy
  (JNIEnv *env, jobject f, jobject vec) {

  double temp;
  getStatFunctor(env,f)->entropy.apply(*getVector(env,vec),temp);
  return jdouble(temp);

}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    makeLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_makeLDA
  (JNIEnv *env, jobject obj, jobject data, jobject labels, jint dim, jobject result) {
  // insert your code here
  checkLAPACKreturn(env,obj,0);
  int d=int(dim);
  _lti_debug("A");
  lti::linearDiscriminantAnalysis<double>::parameters p=getStatFunctor(env,obj)->lda.getParameters();
  _lti_debug("B");
  p.useSVD=true;
  if (d > 0) {
    p.resultDim=d;
  } else {
    p.autoDim=true;
  }    
  getStatFunctor(env,obj)->lda.setParameters(p);
  _lti_debug("C");
  
  lti::ivector ilabs;
  ilabs.castFrom(*getVector(env,labels));

  _lti_debug("D: " << getMatrix(env,data) << ", " << getMatrix(env,result) << "\n");
  if (getStatFunctor(env,obj)->lda.apply(*getMatrix(env,data),*getMatrix(env,result),ilabs)) {
    _lti_debug("E1");
    return jint(getStatFunctor(env,obj)->lda.getUsedDimension());
  } else {
    _lti_debug("E2");
    getStatFunctor(env,obj)->status=getStatFunctor(env,obj)->lda.getStatusString();
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    useLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_useLDA__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject o, jobject v, jobject result) {
  // insert your code here
  checkLAPACK(env,o);
  getStatFunctor(env,o)->lda.apply(*getVector(env,v),*getVector(env,result));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    useLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_useLDA__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject o, jobject m, jobject result) {
  // insert your code here
  checkLAPACK(env,o);
  getStatFunctor(env,o)->lda.apply(*getMatrix(env,m),*getMatrix(env,result));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    setLDA
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_setLDA
  (JNIEnv *env, jobject o, jstring s)  {
  // insert your code here
  // create a string stream from s
  //std::cerr << "Hallo!" << std::endl;
  checkLAPACK(env,o);
  const char *ch=env->GetStringUTFChars(s,0);
  //std::cerr << "Chars = " << ch << std::endl;
  std::istringstream iss(ch);
  lti::lispStreamHandler lis(iss);
  lti::read(lis,getStatFunctor(env,o)->lda);
  _lti_debug(getStatFunctor(env,o)->lda.getEigenValues());
  env->ReleaseStringUTFChars(s,ch);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    getLDA
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_getLDA
  (JNIEnv *env, jobject o) {
  // insert your code here

  checkLAPACKreturn(env,o,env->NewStringUTF(""));
  std::ostringstream oss;
  lti::lispStreamHandler lis(oss);
  lti::write(lis,getStatFunctor(env,o)->lda);
  /* This is code for the old ostrstream class:
  int n=oss.pcount();
  char *cstr=new char[n+1];
  strncpy(cstr,oss.str(),n);
  cstr[n]=0;
  jstring tmp=env->NewStringUTF(cstr);
  delete[] cstr;
  */
  // this is code for the standard class:
  jstring tmp=env->NewStringUTF(oss.str().c_str());
  return tmp;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    covariance
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_covariance
  (JNIEnv *env, jobject f, jobject a, jobject b) {
  // insert your code here
  return jdouble(getStatFunctor(env,f)->variance.covariance(*getVector(env,a),
                                                            *getVector(env,b)));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    makeClassLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_makeClassLDA
  (JNIEnv *env, jobject obj, jobject data, jobject totalcov, jint dim, jobject result) {
  // insert your code here
  checkLAPACKreturn(env,obj,0);
  int d=int(dim);
  lti::linearDiscriminantAnalysis<double>::parameters p=getStatFunctor(env,obj)->lda.getParameters();
  _lti_debug("B");
  p.useSVD=true;
  if (d > 0) {
    p.resultDim=d;
  } else {
    p.autoDim=true;
  }    
  getStatFunctor(env,obj)->lda.setParameters(p);

  if (getStatFunctor(env,obj)->lda.apply(*getMatrix(env,data),*getMatrix(env,result),*getMatrix(env,totalcov))) {
    return jint(getStatFunctor(env,obj)->lda.getUsedDimension());
  } else {
    getStatFunctor(env,obj)->status=getStatFunctor(env,obj)->lda.getStatusString();
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    computeSb
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_computeSb
  (JNIEnv *env, jobject obj, jobject data, jobject labels, jobject sb) {
  // insert your code here

  checkLAPACK(env,obj);
  lti::ivector ilabs;
  ilabs.castFrom(*getVector(env,labels));
  getStatFunctor(env,obj)->lda.computeSb(*getMatrix(env,data),*getMatrix(env,sb),ilabs);
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    geometricMeanOfVector
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_geometricMeanOfVector
  (JNIEnv *env, jobject f, jobject vec) {
  // insert your code here
  double temp;
  getStatFunctor(env,f)->means.geometricMean(*getVector(env,vec),
                                             temp);
  return jdouble(temp);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    fitMixtureModel
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;[Lde/rwth_aachen/techinfo/ltilib/Vector;[Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_fitMixtureModel
  (JNIEnv *env, jobject f, jobject d, jobjectArray m, jobjectArray s, jobject a, jdouble lambda, jint iter) {
  // insert your code here
  _lti_debug("A");
  lti::dmatrix* data=getMatrix(env,d);
  lti::dvector* alpha=getVector(env,a);
  lti::gaussianMixtureModel<double>& mixture=getStatFunctor(env,f)->mixture;
  lti::gaussianMixtureModel<double>::parameters p=mixture.getParameters();
  p.numberOfComponents=env->GetArrayLength(m);
  p.iterations=iter;
  p.lambda=lambda;
  mixture.setParameters(p);
  bool success=mixture.apply(*data);
  const std::vector< lti::dmatrix >& sigmas=mixture.getComponentSigmas();
  const std::vector< lti::dvector >& mus=mixture.getComponentMus();
  if (success) {
    _lti_debug("D");
    // copy the means and sigmas back to the java objects
    for (unsigned int i=0; i<mus.size(); i++) {
      lti::dvector* mi=getVector(env,env->GetObjectArrayElement(m,i));
      mi->copy(mus[i]);
    }
    for (unsigned int i=0; i<sigmas.size(); i++) {
      lti::dmatrix* si=getMatrix(env,env->GetObjectArrayElement(s,i));
      si->copy(sigmas[i]);
    }
    alpha->copy(mixture.getAlpha());
  } else {
    getStatFunctor(env,f)->status=mixture.getStatusString();
  }
  _lti_debug("E");
  return jboolean(success);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    setSeqLDA
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_setSeqLDA
  (JNIEnv *env, jobject f, jstring s) {
  // insert your code here
  checkLAPACK(env,f);
  const char *ch=env->GetStringUTFChars(s,0);

  std::istringstream iss(ch);
  lti::lispStreamHandler lis(iss);
  lti::read(lis,getStatFunctor(env,f)->slda);
  env->ReleaseStringUTFChars(s,ch);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    makeSeqLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_makeSeqLDA
  (JNIEnv *env, jobject f, jobject data) {
  // insert your code here
  checkLAPACKreturn(env,f,0);
  if (getStatFunctor(env,f)->slda.consider(*getMatrix(env,data))) {
    return 1;
  } else {
    getStatFunctor(env,f)->status=getStatFunctor(env,f)->slda.getStatusString();
    return 0;
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    useSeqLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;ILde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_useSeqLDA__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2ILde_rwth_1aachen_techinfo_ltilib_Matrix_2
  (JNIEnv *env, jobject f, jobject srcm, jint dim, jobject destm) {
  // insert your code here
  checkLAPACK(env,f);
  int d=int(dim);
  if (d > 0) {
    getStatFunctor(env,f)->slda.setDimension(int(dim));
  } else {
    lti::serialLDA<double>::parameters p=getStatFunctor(env,f)->slda.getParameters();
    p.autoDim=true;
    getStatFunctor(env,f)->slda.setParameters(p);
  }

  getStatFunctor(env,f)->slda.apply(*getMatrix(env,srcm),*getMatrix(env,destm));

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    getSeqLDA
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_getSeqLDA
  (JNIEnv *env, jobject f) {
  // insert your code here

  checkLAPACKreturn(env,f,env->NewStringUTF(""));
  std::ostringstream oss;
  lti::lispStreamHandler lis(oss);
  lti::write(lis,getStatFunctor(env,f)->slda);
  /* This is code for the old ostrstream class:
  int n=oss.pcount();
  char *cstr=new char[n+1];
  strncpy(cstr,oss.str(),n);
  cstr[n]=0;
  jstring tmp=env->NewStringUTF(cstr);
  delete[] cstr;
  */
  // this is code for the standard class:
  jstring tmp=env->NewStringUTF(oss.str().c_str());
  return tmp;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    useSeqLDA
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;ILde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_useSeqLDA__Lde_rwth_1aachen_techinfo_ltilib_Vector_2ILde_rwth_1aachen_techinfo_ltilib_Vector_2
  (JNIEnv *env, jobject f, jobject srcv, jint dim, jobject destv) {
  // insert your code here
  checkLAPACK(env,f);
  int d=int(dim);
  if (d > 0) {
    getStatFunctor(env,f)->slda.setDimension(int(dim));
  } else {
    lti::serialLDA<double>::parameters p=getStatFunctor(env,f)->slda.getParameters();
    p.autoDim=true;
    getStatFunctor(env,f)->slda.setParameters(p);
  }

  getStatFunctor(env,f)->slda.apply(*getVector(env,srcv),*getVector(env,destv));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_StatFunctors
 * Method:    getStatus
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_rwth_1aachen_techinfo_ltilib_StatFunctors_getStatus
  (JNIEnv *env, jobject f) {
  // insert your code here
  return env->NewStringUTF(getStatFunctor(env,f)->status.c_str());
}

