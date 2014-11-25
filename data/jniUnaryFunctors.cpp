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

#include "jniUnaryFunctors.h"
#include "functorCollection.h"

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_UnaryFunctors
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_UnaryFunctors_closeTheGate
  (JNIEnv *env, jobject f) {

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/UnaryFunctors");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  UnaryFunctorCollection *coll=jlong2unaryfunctor(env->GetLongField(f,fid));

	delete coll;
  env->SetLongField(f, fid, jlong(0));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_UnaryFunctors
 * Method:    openTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_UnaryFunctors_openTheGate
  (JNIEnv *env, jobject f) {

  UnaryFunctorCollection *c;

  c=new UnaryFunctorCollection();
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/UnaryFunctors");
  jfieldID fid=env->GetFieldID(cls,"nativeObject", "J");

  env->SetLongField(f, fid, unaryfunctor2jlong(c));
}

