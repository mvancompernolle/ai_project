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



#include "jniIteratingFunctors.h"
#include "iterCollection.h"
#include "mathObject.h"

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    absElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_absElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->absoluteMatrix.apply(*getMatrix(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    absElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_absElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->absoluteMatrix.apply(*getMatrix(env, ma),
                                                    *getMatrix(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    absElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_absElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->absoluteVector.apply(*getVector(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    absElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_absElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->absoluteVector.apply(*getVector(env, ma),
                                                    *getVector(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_closeTheGate
(JNIEnv *env, jobject f) {

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/IteratingFunctors");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  IteratingFunctorCollection *coll=jlong2iteratingfunctor(env->GetLongField(f,fid));
  delete coll;

  env->SetLongField(f, fid, jlong(0));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    logElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_logElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->logarithmMatrix.apply(*getMatrix(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    logElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_logElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->logarithmMatrix.apply(*getMatrix(env, ma),
                                                     *getMatrix(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    logElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_logElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->logarithmVector.apply(*getVector(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    logElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_logElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->logarithmVector.apply(*getVector(env, ma),
                                                     *getVector(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    openTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_openTheGate
(JNIEnv *env, jobject f) {

  IteratingFunctorCollection *c;

  c=new IteratingFunctorCollection();
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/IteratingFunctors");
  jfieldID fid=env->GetFieldID(cls,"nativeObject", "J");

  env->SetLongField(f, fid, iteratingfunctor2jlong(c));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    squareElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_squareElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->squareMatrix.apply(*getMatrix(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    squareElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_squareElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->squareMatrix.apply(*getMatrix(env, ma),
                                                  *getMatrix(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    squareElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_squareElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->squareVector.apply(*getVector(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    squareElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_squareElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->squareVector.apply(*getVector(env, ma),
                                                  *getVector(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    sqrtElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_sqrtElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->squareRootMatrix.apply(*getMatrix(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    sqrtElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Matrix;Lde/rwth_aachen/techinfo/ltilib/Matrix;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_sqrtElem__Lde_rwth_1aachen_techinfo_ltilib_Matrix_2Lde_rwth_1aachen_techinfo_ltilib_Matrix_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->squareRootMatrix.apply(*getMatrix(env, ma),
                                                      *getMatrix(env, res));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    sqrtElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_sqrtElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma) {

  getIteratingFunctor(env, f)->squareRootVector.apply(*getVector(env, ma));

}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_IteratingFunctors
 * Method:    sqrtElem
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/Vector;Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_IteratingFunctors_sqrtElem__Lde_rwth_1aachen_techinfo_ltilib_Vector_2Lde_rwth_1aachen_techinfo_ltilib_Vector_2
(JNIEnv *env, jobject f, jobject ma, jobject res) {

  getIteratingFunctor(env, f)->squareRootVector.apply(*getVector(env, ma),
                                                      *getVector(env, res));

}

