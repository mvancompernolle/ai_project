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



#include "jniVector.h"
#include "mathObject.h"
#include "ltiVector.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 3
#include "ltiDebug.h"


//lti::vector<double> v;

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    openTheGate
 * Signature: (ID)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector_openTheGate
(JNIEnv *env, jobject v, jint size, jdouble iniValue) {

  jniVector *vec;

  vec=new jniVector(size,iniValue);
  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/Vector");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");

  _lti_debug("(");

  env->SetLongField(v, fid, vector2jlong(vec));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    closeTheGate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector_closeTheGate
(JNIEnv *env, jobject v) {

  jclass cls=env->FindClass("de/rwth_aachen/techinfo/ltilib/Vector");
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jniVector *vec=jlong2vector(env->GetLongField(v,fid));

  _lti_debug(")");

  env->SetLongField(v, fid, jlong(0));

  delete vec;
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    csubvector
 * Signature: ([I)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector_csubvector
  (JNIEnv *env, jobject v, jintArray jindex) {
  // insert your code here
  jniVector *vec=getVector(env, v);
  int n=env->GetArrayLength(jindex);
  jboolean isCopy=JNI_FALSE;
  int* index=env->GetIntArrayElements(jindex,&isCopy);
  jniVector *smat=new jniVector(n);
  for (int i=0; i<n; i++) {
    if (index[i] >= 0 && index[i] < vec->size()) {
      smat->at(i)=(vec->at(index[i]));
    }
  }
  env->ReleaseIntArrayElements(jindex,index,JNI_ABORT);
  return vector2jlong(smat);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _resize
 * Signature: (JID)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1resize
  (JNIEnv *env, jclass v, jlong nat, jint n, jdouble ini) {
  // insert your code here
  jniVector *vec=jlong2vector(nat);
  vec->resize(n,ini);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1size
(JNIEnv *env, jclass v, jlong nat) {

  return jint(jlong2vector(nat)->size());
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _fill
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1fill__JD
(JNIEnv *env, jclass v, jlong nat, jdouble value) {

  jlong2vector(nat)->fill(double(value));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _fill
 * Signature: (JDII)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1fill__JDII
(JNIEnv *env, jclass v, jlong nat, jdouble value, jint from, jint to)  {

  jlong2vector(nat)->fill(double(value),int(from),int(to));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _at
 * Signature: (JI)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1at
(JNIEnv *env, jclass v, jlong nat, jint pos) {

  _lti_debug3("Accessing vector " << nat << " at " << pos);

  return jdouble(jlong2vector(nat)->at(pos));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _setAt
 * Signature: (JID)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1setAt
(JNIEnv *env, jclass v, jlong nat, jint pos, jdouble value) {

  _lti_debug3("Writing vector " << nat << " at " << pos);
  jlong2vector(nat)->at(int(pos))=double(value);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _copy
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1copy
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  jlong2vector(nat)->copy(*jlong2vector(other));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _sortAscending
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1sortAscending
(JNIEnv *env, jclass v, jlong nat) {

  void qSort(lti::vector<double>* v, int l, int r);

  lti::vector<double>* temp=jlong2vector(nat);
  qSort(temp, 0, temp->size()-1);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _sortDescending
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1sortDescending
(JNIEnv *env, jclass v, jlong nat) {

  void qSortDesc(lti::vector<double>* v, int l, int r);

  lti::vector<double>* temp=jlong2vector(nat);
  qSortDesc(temp, 0, temp->size()-1);
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _dot
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector;)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1dot
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  return jlong2vector(nat)->dot(*jlong2vector(other));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _emultiply
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1emultiply__JJ
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  jlong2vector(nat)->emultiply(*jlong2vector(other));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _emultiply
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1emultiply__JJJ
(JNIEnv *env, jclass v, jlong nat, jlong a, jlong b) {

  jlong2vector(nat)->emultiply(*jlong2vector( a), *jlong2vector(b));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _edivide
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1edivide__JJJ
(JNIEnv *env, jclass v, jlong nat, jlong op1, jlong op2) {
  // insert your code here

  jlong2vector(nat)->edivide(*jlong2vector(op1),*jlong2vector(op2));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _edivide
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1edivide__JJ
(JNIEnv *env, jclass v, jlong nat, jlong op) {
  // insert your code here
  jlong2vector(nat)->edivide(*jlong2vector(op));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _sumOfElements
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1sumOfElements
(JNIEnv *env, jclass v, jlong nat) {
  // insert your code here

  return jdouble(jlong2vector(nat)->sumOfElements());
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _productOfElements
 * Signature: (J)V
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1productOfElements
(JNIEnv *env, jclass v, jlong nat)  {
  // insert your code here

  return jdouble(jlong2vector(nat)->productOfElements());
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _add
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1add__JJ
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  jlong2vector(nat)->add(*jlong2vector(other));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _addScaled
 * Signature: (JDLde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1addScaled
  (JNIEnv *env, jclass v, jlong nat, jdouble s, jlong op) {
  // insert your code here
  jlong2vector(nat)->addScaled(double(s),*jlong2vector(op));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _add
 * Signature: (JJJ)
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1add__JJJ
(JNIEnv *env , jclass v, jlong nat, jlong a, jlong b) {

  jlong2vector(nat)->add(*jlong2vector( a), *jlong2vector( a));
}




/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _subtract
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1subtract__JJ
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  jlong2vector(nat)->subtract(*jlong2vector( other));
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _subtract
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1subtract__JJJ
(JNIEnv *env, jclass v, jlong nat, jlong a, jlong b) {

  jlong2vector(nat)->subtract(*jlong2vector( a), *jlong2vector( b));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _multiply
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1multiply
(JNIEnv *env, jclass v, jlong nat, jdouble s) {

  jlong2vector(nat)->multiply(double(s));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _divide
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1divide
  (JNIEnv *env, jclass v, jlong nat, jdouble s) {
  // insert your code here
  jlong2vector(nat)->multiply(double(s));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _add
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1add__JD
(JNIEnv *env, jclass v, jlong nat, jdouble s) {

  jlong2vector(nat)->add(double(s));
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _equals
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector;)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1equals
(JNIEnv *env, jclass v, jlong nat, jlong other) {

  // insert your code here
  return jboolean(jlong2vector(nat)->equals(*jlong2vector( other)));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _prettyCloseTo
 * Signature: (JLde/rwth_aachen/techinfo/ltilib/Vector;D)Z
 */
JNIEXPORT jboolean JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1prettyCloseTo
(JNIEnv *env, jclass v, jlong nat, jlong other, jdouble tol) {
  // insert your code here

  return jboolean(jlong2vector(nat)->prettyCloseTo(*jlong2vector( other),tol));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _maximum
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1maximum
(JNIEnv *env, jclass v, jlong nat) {
  return jdouble(jlong2vector(nat)->maximum());
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _getIndexOfMaximum
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1getIndexOfMaximum
  (JNIEnv *env, jclass v, jlong nat) {
  // insert your code here
  return jint(jlong2vector(nat)->getIndexOfMaximum());
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _minimum
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1minimum
(JNIEnv *env, jclass v, jlong nat) {
  // insert your code here
  return jdouble(jlong2vector(nat)->minimum());
}





/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _getIndexOfMinimum
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1getIndexOfMinimum
  (JNIEnv *env, jclass v, jlong nat) {
  // insert your code here

  return jint(jlong2vector(nat)->getIndexOfMinimum());
}



/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _concat
 * Signature: (J[Lde/rwth_aachen/techinfo/ltilib/Vector;)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1concat
  (JNIEnv *env, jclass v, jlong nat, jobjectArray sources) {
  // insert your code here

  lti::dvector *vect=jlong2vector(nat);

  int n=env->GetArrayLength(sources);
  lti::dvector **ltisrc=new (lti::dvector*)[n];
  int len=0;
  for (int i=0; i<n; i++) {
    ltisrc[i]=getVector(env,env->GetObjectArrayElement(sources,i));
    len+=ltisrc[i]->size();
  }

  vect->resize(len);
  int offset=0;
  for (int i=0; i<n; i++) {
    int k=ltisrc[i]->size();
    vect->fill(*ltisrc[i],offset,offset+k-1,0);
    offset+=k;
  }

}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _normalize
 * Signature: (JDD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1normalize__JDD
  (JNIEnv *env, jclass v, jlong nat, jdouble low, jdouble high) {
  // insert your code here

  lti::dvector* vec=jlong2vector(nat);

  double mini=vec->minimum();
  double maxi=vec->maximum();

  lti::dvector offset(vec->size());
  offset.fill(mini);

  // move to zero minimum
  vec->subtract(offset);
  // scale to correct high
  vec->multiply((double(high)-double(low))/(maxi-mini));
  // now move to new minimum
  vec->add(double(low));
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _normalize
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1normalize__JD
  (JNIEnv *env, jclass v, jlong nat, jdouble length) {
  // insert your code here

  lti::dvector* vec=jlong2vector(nat);

  lti::dvector tmp(*vec);
  tmp.emultiply(tmp);
  double sum=tmp.sumOfElements();

  if (sum != 0.0) {
    vec->multiply(double(length)/sqrt(sum));
  }
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _normalizeSum
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1normalizeSum
  (JNIEnv *env, jclass v, jlong nat, jdouble s) {
  // insert your code here

  lti::dvector* vec=jlong2vector(nat);

  double sum=vec->sumOfElements();

  if (sum != 0.0) {
    vec->multiply(double(s)/sum);
  }
}


// ====


void qSort(lti::vector<double>* v, int l, int r) {

  if (r-l<10) {
    void insertionSort(lti::vector<double>* v, int l, int r);
    insertionSort(v, l, r);
    return;
  }

  int i=l;
  int j=r;

  double a=v->at(l);
  double b=v->at((l+r)/2);
  double c=v->at(r);

  double x=0;
  if (a<=b) {
    if (b<=c) {x=b;}
    if (c<=a) {x=a;}
  }
  if (b<a) {
    if (a<=c) {x=a;}
    if (c<=b) {x=b;}
  }
  if ((a<=c)&&(c<=b)||
      (b<=c)&&(c<=a)) {
    x=c;
  }

  double temp;

  do {

    while (v->at(i) < x) {i++;}
    while (x < v->at(j)) {j--;}

    if (i<j) {
      temp=v->at(i);
      v->at(i)=v->at(j);
      v->at(j)=temp;
    }
    if (i<=j) {
      i++;
      j--;
    }

  } while (i<=j);

  if (l<j) {qSort(v, l, j);}
  if (i<r) {qSort(v, i, r);}

}

void insertionSort(lti::vector<double>* v, int l, int r) {

  double x;
  int i,j;

  for (i=l+1; i<=r; i++) {

    x=v->at(i);
    j=i;

    while (x < v->at(j-1) && j>0) {
      v->at(j)=v->at(j-1);
      j--;
    }
    v->at(j)=x;
  }
}



void qSortDesc(lti::vector<double>* v, int l, int r) {

  if (r-l<10) {
    void insertionSortDesc(lti::vector<double>* v, int l, int r);
    insertionSortDesc(v, l, r);
    return;
  }

  int i=l;
  int j=r;

  double a=v->at(l);
  double b=v->at((l+r)/2);
  double c=v->at(r);

  double x=0;
  if (a<=b) {
    if (b<=c) {x=b;}
    if (c<=a) {x=a;}
  }
  if (b<a) {
    if (a<=c) {x=a;}
    if (c<=b) {x=b;}
  }
  if ((a<=c)&&(c<=b)||
      (b<=c)&&(c<=a)) {
    x=c;
  }

  double temp;

  do {

    while (v->at(i) > x) {i++;}
    while (x > v->at(j)) {j--;}

    if (i<j) {
      temp=v->at(i);
      v->at(i)=v->at(j);
      v->at(j)=temp;
    }
    if (i<=j) {
      i++;
      j--;
    }

  } while (i<=j);

  if (l<j) {qSortDesc(v, l, j);}
  if (i<r) {qSortDesc(v, i, r);}

}

void insertionSortDesc(lti::vector<double>* v, int l, int r) {

  double x;
  int i,j;

  for (i=l+1; i<=r; i++) {

    x=v->at(i);
    j=i;

    while (x > v->at(j-1) && j>0) {
      v->at(j)=v->at(j-1);
      j--;
    }
    v->at(j)=x;
  }
}

/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _setData
 * Signature: (J[D)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1setData__J_3D
(JNIEnv *env, jclass v, jlong nat, jdoubleArray data) {

  jniVector *vec=jlong2vector(nat);

  jboolean copy=JNI_TRUE;
  double* elems=env->GetDoubleArrayElements(data, &copy);
  int size=env->GetArrayLength(data);

  if (size > 0) {
    double* copied=0;

    if(copy==JNI_TRUE) {
      copied=elems;
    } else if (copy==JNI_FALSE){
      copied=new double[size];
      for (int i=0; i<size; i++) {
        copied[i]=elems[i];
      }
    } else {
      printf("funny error in jniVector.cpp");
    }

    vec->attach(size, copied);
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    copyCtoJava
 * Signature: ([FI)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector_copyCtoJava___3FI
(JNIEnv *env, jobject obj, jfloatArray data,  jint size) {
  // insert your code here

  jniVector *vec;

  jclass cls=env->GetObjectClass(obj);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(obj,fid);

  if (id != 0) {
    vec=jlong2vector(id);
  } else {
    data=NULL;
    return;
  }

  lti::fvector tmp;
  tmp.castFrom(*vec);

  if (vec->size() > 0) {
    env->SetFloatArrayRegion(data, 0, tmp.size(), &(tmp.at(0)));
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    copyCtoJava
 * Signature: ([DI)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector_copyCtoJava___3DI
(JNIEnv *env, jobject obj, jdoubleArray data,  jint size) {
  // insert your code here

  jniVector *vec;

  jclass cls=env->GetObjectClass(obj);
  jfieldID fid=env->GetFieldID(cls, "nativeObject", "J");
  jlong id=env->GetLongField(obj,fid);

  if (id != 0) {
    vec=jlong2vector(id);
  } else {
    data=NULL;
    return;
  }

  if (vec->size() > 0) {
    env->SetDoubleArrayRegion(data, 0, vec->size(), &(vec->at(0)));
  }
}


/*
 * Class:     de_rwth_0005faachen_techinfo_ltilib_Vector
 * Method:    _setData
 * Signature: (J[F)V
 */
JNIEXPORT void JNICALL Java_de_rwth_1aachen_techinfo_ltilib_Vector__1setData__J_3F
(JNIEnv *env, jclass v, jlong nat, jfloatArray data) {

  jniVector *vec=jlong2vector(nat);

  jboolean copy=JNI_TRUE;
  float* elems=env->GetFloatArrayElements(data, &copy);
  int size=env->GetArrayLength(data);

  if (size > 0) {
    double* copied=new double[size];
    for (int i=0; i<size; i++) {
      copied[i]=elems[i];
    }

    env->ReleaseFloatArrayElements(data,elems,JNI_ABORT);
    vec->attach(size, copied);
  }
}

