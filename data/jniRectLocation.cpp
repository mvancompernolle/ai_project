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


#include "jniRectLocation.h"
#include "libBase.h"
#include "ltiLocation.h"
#include "ltiPoint.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

typedef lti::rectLocation jniRectLocation;


inline jniRectLocation *jlong2rectLocation(jlong p) {
  jniRectLocation *v=(jniRectLocation *)long(p);
  assert(v != 0);
  return v;
}

inline jlong rectLocation2jlong(jniRectLocation *p) {
  return jlong(long(p));
}

inline jniRectLocation *getRectLocation(JNIEnv *env, jobject m) {
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/RectLocation");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  return jlong2rectLocation(env->GetLongField(m, fid));
}

inline void putRectLocation(JNIEnv *env, jobject m, jniRectLocation *vec) {
  jfieldID fid;
  //jniMatrix *mat;

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/RectLocation");
  fid=env->GetFieldID(cls, "nativeObject", "J");
  env->SetLongField(m,fid, rectLocation2jlong(vec));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    copy
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/RectLocation;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_copy
  (JNIEnv *env, jobject obj1, jobject obj2) {

  // insert your code here
  getRectLocation(env,obj1)->copy(*getRectLocation(env,obj2));

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    distanceSqr
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/RectLocation;[F[F)F
 */
JNIEXPORT jfloat JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_distance2__Lde_rwth_1aachen_techinfo_ltilib_RectLocation_2_3F_3F
  (JNIEnv *env, jobject obj1, jobject obj2, jfloatArray p1, jfloatArray p2) {

  // insert your code here
  lti::tpoint<float> pp1,pp2;
  jfloat res =
    getRectLocation(env,obj1)->distanceSqr(*getRectLocation(env,obj2),
           pp1,pp2);

  jfloat tp1[2];

  tp1[0]=pp1.x;
  tp1[1]=pp1.y;

  env->SetFloatArrayRegion(p1,0,2,tp1);

  tp1[0]=pp2.x;
  tp1[1]=pp2.y;

  env->SetFloatArrayRegion(p2,0,2,tp1);

  return res;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    copyCtoJava
 * Signature: ([DI)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_copyCtoJava
  (JNIEnv *env, jobject obj, jfloatArray data) {
  // insert your code here

  jfloat tmp[5];

  jniRectLocation* rect=getRectLocation(env,obj);

  tmp[0] = rect->position.x;
  tmp[1] = rect->position.y;
  tmp[2] = rect->angle;
  tmp[3] = rect->maxLength;
  tmp[4] = rect->minLength;

  env->SetFloatArrayRegion(data,0,5,tmp);

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    distanceSqr
 * Signature: (Lde/rwth_aachen/techinfo/ltilib/RectLocation;)F
 */
JNIEXPORT jfloat JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_distance2__Lde_rwth_1aachen_techinfo_ltilib_RectLocation_2
  (JNIEnv *env, jobject obj1, jobject obj2) {

  // insert your code here
  return jfloat(getRectLocation(env,obj1)->distanceSqr(*getRectLocation(env,obj2)));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    contains
 * Signature: (FF)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_contains__FF
  (JNIEnv *env, jobject obj1, jfloat x, jfloat y) {
  // insert your code here

  lti::tpoint<float> p(x,y);
  return jboolean(getRectLocation(env,obj1)->contains(p));

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    openTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_openTheGate
  (JNIEnv *env, jobject obj) {
  // insert your code here
  jniRectLocation *vec=new jniRectLocation();
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/RectLocation");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");

  _lti_debug("(");

  vec->position.x = 0;
  vec->position.y = 0;
  vec->angle = 0;
  vec->minLength = 0;
  vec->maxLength = 0;
  env->SetLongField(obj, fid, rectLocation2jlong(vec));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_closeTheGate
  (JNIEnv *env, jobject obj) {
  // insert your code here

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/RectLocation");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jniRectLocation *vec=jlong2rectLocation(env->GetLongField(obj,fid));

  _lti_debug(")");

  env->SetLongField(obj, fid, jlong(0));

  delete vec;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    setData
 * Signature: (FFFFF)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_setData
  (JNIEnv *env, jobject obj, jfloat x, jfloat y,
   jfloat angle, jfloat maxLength, jfloat minLength) {
  // insert your code here

  jniRectLocation *rect=getRectLocation(env,obj);

  rect->position.x = x;
  rect->position.y = y;
  rect->angle = angle;
  rect->minLength = minLength;
  rect->maxLength = maxLength;
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    contains
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_contains__II
  (JNIEnv *env, jobject obj, jint x, jint y) {
  // insert your code here
  lti::point p(x,y);
  return jboolean(getRectLocation(env,obj)->contains(p));

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    getArea
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_getArea
  (JNIEnv *env, jobject obj) {
  // insert your code here
  return jfloat(getRectLocation(env,obj)->getArea());
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    getAxes
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_getAxes
(JNIEnv *env, jobject rl, jfloatArray xy) {
  // insert your code here
  lti::rectLocation* rlp=getRectLocation(env,rl);
  jfloat buf[2];
  buf[0]=rlp->maxLength;
  buf[1]=rlp->minLength;
  env->SetFloatArrayRegion(xy,0,2,buf);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    getPosition
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_getPosition
  (JNIEnv *env, jobject rl, jfloatArray xy) {
  // insert your code here
  lti::rectLocation* rlp=getRectLocation(env,rl);
  jfloat buf[2];
  buf[0]=rlp->position.x;

  buf[1]=rlp->position.y;
  env->SetFloatArrayRegion(xy,0,2,buf);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_RectLocation
 * Method:    getAngle
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_de_rwth_1aachen_techinfo_ltilib_RectLocation_getAngle
  (JNIEnv *env, jobject rl) {
  // insert your code here
  return jfloat(getRectLocation(env,rl)->angle);
}

