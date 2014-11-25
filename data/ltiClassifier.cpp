/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiClassifier.cpp
 * authors ....: Pablo Alvarado, Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 10.8.2000
 * revisions ..: $Id: ltiClassifier.cpp,v 1.13 2006/09/05 09:56:39 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiClassifier.h"
#include "ltiException.h"
#include "ltiSTLIoInterface.h"
#include "ltiSort.h"
#include "ltiIoHandler.h"
#include "ltiLispStreamHandler.h"
#include <set>

namespace lti {

  // --------------------------------------------------
  // static member initialization
  // --------------------------------------------------

  const int classifier::unknownObj(-1);
  const int classifier::noObject(-2);
  const double classifier::nullProbability(0.0000001);

  // --------------------------------------------------
  // classifier::outputVector
  // --------------------------------------------------

  classifier::outputVector::outputVector()
    : ioObject(),ids(0), values(0) {

    winner=-1;
    confidence=1.;
    reject=false;
    valid=true;
    sortedAsc=false;
    sortedDesc=false;
  }

  classifier::outputVector::outputVector(const outputVector& other)
    : ioObject() {

    copy(other);
  }

  classifier::outputVector::outputVector(const int& size) {

    values=dvector(size);
    ids=ivector(size, unknownObj);

    winner=-1;
    confidence=1.;
    reject=false;
    valid=true;
    sortedAsc=false;
    sortedDesc=false;
  }

  classifier::outputVector::outputVector(const ivector& theIds,
                                         const dvector& theValues) {

    setIdsAndValues(theIds,theValues);
  }

  classifier::outputVector&
  classifier::outputVector::copy(const outputVector& other) {

    values.copy(other.values);
    ids.copy(other.ids);
    winner=other.winner;
    reject=other.reject;
    valid=other.valid;
    sortedAsc=other.sortedAsc;
    sortedDesc=other.sortedDesc;

    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::operator= (const outputVector& other) {

    return copy(other);
  }

  classifier::outputVector* classifier::outputVector::clone() const {

    return new outputVector(*this);
  }

  int classifier::outputVector::size() const {
    return values.size();
  }

  void classifier::outputVector::setValues(const dvector& theValues) {
    values.copy(theValues);
    sortedAsc=false;
    sortedDesc=false;
  }

  bool
  classifier::outputVector::setValueByPosition(const int& pos,
                                               const double& value) {

    if (pos<0 || pos>=values.size()) {
      return false;
    }

    values.at(pos)=value;
    return true;
  }

  bool
  classifier::outputVector::setValueById(const int& id, const double& value) {

    int pos=0;
    ivector::const_iterator it=ids.begin();
    for (;*it!=id; pos++, ++it);

    if (it==ids.end()) {
      return false;
    }

    values.at(pos)=value;
    return true;
  }

  const dvector& classifier::outputVector::getValues() const {

    return values;
  }

  bool classifier::outputVector::find(const int& id, int& pos) const {

    const int size = this->size();
    for (pos=0; pos < size; ++pos) {
      if (ids.at(pos) == id) {
        break;
      }
    }

    if (pos == size) {
      pos=-1;
      return false;
    }

    return true;
  }

  void classifier::outputVector::setIds(const ivector& theIds) {
    ids.copy(theIds);
    sortedAsc=false;
    sortedDesc=false;
  }

  void classifier::outputVector::setIdsAndValues(const ivector& theIds,
                                                 const dvector& theValues) {

    if (theIds.size() == theValues.size()) {
      ids.copy(theIds);
      values.copy(theValues);
    } else {
      // get the minimum common data block
      int size = lti::min(theIds.size(),theValues.size());
      ids.resize(size,0,false,false);
      ids.fill(theIds);
      values.resize(size,0,false,false);
      values.fill(theValues);
    }

    winner=-1;
    confidence=1.;
    reject=false;
    valid=true;
    sortedAsc=false;
    sortedDesc=false;
  }

  bool classifier::outputVector::setPair(const int& pos, const int& id,
                                         const double& value) {

    if (pos<0 || pos>=values.size()) {
      return false;
    }

    values.at(pos)=value;
    ids.at(pos)=id;

    sortedAsc=false;
    sortedDesc=false;
    return true;
  }

  bool classifier::outputVector::getId(const int& pos, int& id) const {

    if (pos<0 || pos>=values.size()) {
      id=noObject;
      return false;
    }

    id=ids.at(pos);
    return true;
  }

  const ivector& classifier::outputVector::getIds() const {

    return ids;
  }

  bool classifier::outputVector::getValueByPosition(const int& pos,
                                                    double& value) const {

    if (pos<0 || pos>=values.size()) {
      value=0.;
      return false;
    }

    value=values.at(pos);
    return true;
  }

  bool classifier::outputVector::getValueById(const int& id,
                                              double& value) const {

    int i=0;
    while (i<ids.size()) {
      if (ids.at(i)==id) {
        value=values.at(i);
        return true;
      }
      i++;
    }
    value=0.;
    return false;
  }

  bool classifier::outputVector::getPair(const int& pos,
                                         int& id, double& value) const {

    if (pos<0 || pos>=values.size()) {
      id=noObject;
      value=0.;
      return false;
    }

    id=ids.at(pos);
    value=values.at(pos);
    return true;
  }

  bool classifier::outputVector::setWinnerUnit(const int& pos) {
    if (pos>=0 && pos<ids.size()) {
      winner=pos;
      return true;
    }
    return false;
  }

  int classifier::outputVector::setWinnerAtMax() {

    if (!values.empty()) {
      winner=values.getIndexOfMaximum();
    } else {
      winner = -1;
    }
    return winner;
  }

  int classifier::outputVector::getWinnerUnit() const {
    return winner;
  }

  void classifier::outputVector::setReject(const bool& rej) {
    reject=rej;
  }

  bool classifier::outputVector::isRejected() const {
    return reject;
  }

  void classifier::outputVector::setConfidenceValue(const double& conf) {
    confidence=conf;
  }

  double classifier::outputVector::getConfidenceValue() const {
    return confidence;
  }

  void classifier::outputVector::sortAscending() {

    //To ensure that the winner member still contains the right value
    //after sorting a nifty little trick suggested by wickel is used here:
    //We set the id of the winner to the negative id. After sorting
    //only find the negative id, set winner and revert negation. Great.

    sort2<double, int> sorter;

    dvector dtemp(values);
    ivector itemp(ids);
    if (winner!=-1) {
      itemp.at(winner)*=-1;
    }

    sorter.apply(dtemp, itemp, values, ids);

    for (int i=0; i<ids.size(); i++) {
      if (ids.at(i)<0) {
        winner=i;
        ids.at(i)*=-1;
        break;
      }
    }
    sortedAsc=true;
    sortedDesc=false;
  }

  void classifier::outputVector::sortDescending() {

    //To ensure that the winner member still contains the right value
    //after sorting a nifty little trick suggested by wickel is used here:
    //We set the id of the winner to the negative id. After sorting
    //only find the negative id, set winner and revert negation. Great.

    //argument makes the functor sort in descending order
    sort2<double, int> sorter(true);

    dvector dtemp(values);
    ivector itemp(ids);
    if (winner!=-1) {
      itemp.at(winner)*=-1;
    }

    sorter.apply(dtemp, itemp, values, ids);

    for (int i=0; i<ids.size(); i++) {
      if (ids.at(i)<0) {
        winner=i;
        ids.at(i)*=-1;
        break;
      }
    }
    sortedAsc=false;
    sortedDesc=true;
  }

  void classifier::outputVector::makeProbDistribution() {

    double sum(0.0);
    double val;
    for (int i=0; i<values.size(); i++) {
      val = values.at(i);
      if (val < 0) {
        values.at(i)=0;
      } else {
        sum+=val;
      }
    }

    values.divide(sum);
  }

  bool classifier::outputVector::compatible(const outputVector& other) const {

    if (ids.size()!=other.ids.size()) {
      return false;
    }

    sort<int> sorter;

    ivector sThis, sOther;
    sorter.apply(ids, sThis);
    sorter.apply(other.ids, sOther);

    for (int i=0; i<sThis.size(); i++) {
      if (sThis.at(i)!=sOther.at(i)) {
        return false;
      }
    }
    return true;
  }

  void classifier::outputVector::idMaximize() {

    int i,j;
    sort2<int, double> sorter;
    int size, newSize;
    size=newSize=ids.size();

    dvector tValues;
    ivector tIds;

    sorter.apply(ids, values, tIds, tValues);

    int currId=-1;
    double max=-1;
    int maxPos=0;
    for (i=0; i<size; i++) {
      if (tIds.at(i)==currId) {
        if (tValues.at(i)>max) {
          tIds.at(maxPos)=-1;
          maxPos=i;
          max=tValues.at(i);
        } else {
          tIds.at(i)=-1;
        }
        newSize--;
      } else {
        maxPos=i;
        currId=tIds.at(i);
        max=tValues.at(i);
      }
    }

    ids.resize(newSize, 0, false, false);
    values.resize(newSize, 0, false, false);

    j=0;
    for (i=0; i<size; i++) {
      if (tIds.at(i)>0) {
        ids.at(j)=tIds.at(i);
        values.at(j)=tValues.at(i);
        j++;
      }
    }
    sortedAsc=false;
    sortedDesc=false;
  }

  void classifier::outputVector::idSum() {

    int i,j;
    sort2<int, double> sorter;
    int size, newSize;
    size=newSize=ids.size();

    dvector tValues;
    ivector tIds;

    sorter.apply(ids, values, tIds, tValues);

    int currId=tIds.at(0);
    double sum=tValues.at(0);
    int sumPos=0;
    for (i=1; i<size; i++) {
      if (tIds.at(i)==currId) {
        sum+=tValues.at(i);
        tIds.at(i)=-1;
        newSize--;
      } else {
        //write the old sum
        tValues.at(sumPos)=sum;
        sumPos=i;
        currId=tIds.at(i);
        sum=tValues.at(i);
      }
    }

    ids.resize(newSize, 0, false, false);
    values.resize(newSize, 0, false, false);

    j=0;
    for (i=0; i<size; i++) {
      if (tIds.at(i)>0) {
        ids.at(j)=tIds.at(i);
        values.at(j)=tValues.at(i);
        j++;
      }
    }
    sortedAsc=false;
    sortedDesc=false;
  }

  double classifier::outputVector::maxValue() const {

    return values.maximum();
  }

  int classifier::outputVector::maxPosition() const {

    return values.getIndexOfMaximum();
  }

  int classifier::outputVector::maxId() const {

    return ids.at(values.getIndexOfMaximum());
  }

  void classifier::outputVector::maxPair(int& id, double& value) const {

    int i=values.getIndexOfMaximum();
    id=ids.at(i);
    value=values.at(i);
  }

  double classifier::outputVector::minValue() const {

    return values.minimum();
  }

  int classifier::outputVector::minPosition() const {

    return values.getIndexOfMinimum();
  }

  int classifier::outputVector::minId() const {

    return ids.at(values.getIndexOfMinimum());
  }

  void classifier::outputVector::minPair(int& id, double& value) const {

    int i=values.getIndexOfMinimum();
    id=ids.at(i);
    value=values.at(i);
  }

  classifier::outputVector&
  classifier::outputVector::add(const outputVector& other) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(noMultipleIds());
    assert(other.noMultipleIds());

    sort2<int, double> sorter;

    ivector tIds, oIds;
    dvector tValues, oValues;

    sorter.apply(ids, values, tIds, tValues);
    sorter.apply(other.ids, other.values, oIds, oValues);

    int s=ids.size()+other.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<tIds.size() && j<oIds.size()) {
      if (tIds.at(i)==oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i)+oValues.at(j);
        k++;
        i++;
        j++;
      } else if (tIds.at(i)<oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i);
        k++;
        i++;
      } else {
        ids.at(k)=oIds.at(j);
        values.at(k)=oValues.at(j);
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<tIds.size()) {
      ids.at(k)=tIds.at(i);
      values.at(k)=tValues.at(i);
      k++;
      i++;
    }
    while (j<oIds.size()) {
      ids.at(k)=oIds.at(j);
      values.at(k)=oValues.at(j);
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::add(const outputVector& a, const outputVector& b) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(a.noMultipleIds());
    assert(b.noMultipleIds());

    sort2<int, double> sorter;

    ivector aIds, bIds;
    dvector aValues, bValues;

    sorter.apply(a.ids, a.values, aIds, aValues);
    sorter.apply(b.ids, b.values, bIds, bValues);

    int s=a.ids.size()+b.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<aIds.size() && j<bIds.size()) {
      if (aIds.at(i)==bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=aValues.at(i)+bValues.at(j);
        k++;
        i++;
        j++;
      } else if (aIds.at(i)<bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=aValues.at(i);
        k++;
        i++;
      } else {
        ids.at(k)=bIds.at(j);
        values.at(k)=bValues.at(j);
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<aIds.size()) {
      ids.at(k)=aIds.at(i);
      values.at(k)=aValues.at(i);
      k++;
      i++;
    }
    while (j<bIds.size()) {
      ids.at(k)=bIds.at(j);
      values.at(k)=bValues.at(j);
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector& classifier::outputVector::add(const double& s) {

    values.add(s);
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::addScaled(const outputVector& other,
                                      const double& scale) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(noMultipleIds());
    assert(other.noMultipleIds());

    sort2<int, double> sorter;

    ivector tIds, oIds;
    dvector tValues, oValues;

    sorter.apply(ids, values, tIds, tValues);
    sorter.apply(other.ids, other.values, oIds, oValues);

    int s=ids.size()+other.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<tIds.size() && j<oIds.size()) {
      if (tIds.at(i)==oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i)+scale*oValues.at(j);
        k++;
        i++;
        j++;
      } else if (tIds.at(i)<oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i);
        k++;
        i++;
      } else {
        ids.at(k)=oIds.at(j);
        values.at(k)=scale*oValues.at(j);
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<tIds.size()) {
      ids.at(k)=tIds.at(i);
      values.at(k)=tValues.at(i);
      k++;
      i++;
    }
    while (j<oIds.size()) {
      ids.at(k)=oIds.at(j);
      values.at(k)=scale*oValues.at(j);
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::mul(const outputVector& other) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(noMultipleIds());
    assert(other.noMultipleIds());

    sort2<int, double> sorter;

    ivector tIds, oIds;
    dvector tValues, oValues;

    sorter.apply(ids, values, tIds, tValues);
    sorter.apply(other.ids, other.values, oIds, oValues);

    int s=ids.size()+other.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<tIds.size() && j<oIds.size()) {
      if (tIds.at(i)==oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i)*oValues.at(j);
        k++;
        i++;
        j++;
      } else if (tIds.at(i)<oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=0.;
        k++;
        i++;
      } else {
        ids.at(k)=oIds.at(j);
        values.at(k)=0.;
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<tIds.size()) {
      ids.at(k)=tIds.at(i);
      values.at(k)=0.;
      k++;
      i++;
    }
    while (j<oIds.size()) {
      ids.at(k)=oIds.at(j);
      values.at(k)=0.;
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::mul(const outputVector& a, const outputVector& b) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(a.noMultipleIds());
    assert(b.noMultipleIds());

    sort2<int, double> sorter;

    ivector aIds, bIds;
    dvector aValues, bValues;

    sorter.apply(a.ids, a.values, aIds, aValues);
    sorter.apply(b.ids, b.values, bIds, bValues);

    int s=a.ids.size()+b.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<aIds.size() && j<bIds.size()) {
      if (aIds.at(i)==bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=aValues.at(i)*bValues.at(j);
        k++;
        i++;
        j++;
      } else if (aIds.at(i)<bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=0.;
        k++;
        i++;
      } else {
        ids.at(k)=bIds.at(j);
        values.at(k)=0.;
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<aIds.size()) {
      ids.at(k)=aIds.at(i);
      values.at(k)=0.;
      k++;
      i++;
    }
    while (j<bIds.size()) {
      ids.at(k)=bIds.at(j);
      values.at(k)=0.;
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector& classifier::outputVector::mul(const double& s) {

    values.multiply(s);
    return *this;
  }

  classifier::outputVector& classifier::outputVector::divide(const double& s) {

    values.divide(s);
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::max(const outputVector& other) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(noMultipleIds());
    assert(other.noMultipleIds());

    sort2<int, double> sorter;

    ivector tIds, oIds;
    dvector tValues, oValues;

    sorter.apply(ids, values, tIds, tValues);
    sorter.apply(other.ids, other.values, oIds, oValues);

    int s=ids.size()+other.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<tIds.size() && j<oIds.size()) {
      if (tIds.at(i)==oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=lti::max(tValues.at(i),oValues.at(j));
        k++;
        i++;
        j++;
      } else if (tIds.at(i)<oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=tValues.at(i);
        k++;
        i++;
      } else {
        ids.at(k)=oIds.at(j);
        values.at(k)=oValues.at(j);
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<tIds.size()) {
      ids.at(k)=tIds.at(i);
      values.at(k)=tValues.at(i);
      k++;
      i++;
    }
    while (j<oIds.size()) {
      ids.at(k)=oIds.at(j);
      values.at(k)=oValues.at(j);
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::max(const outputVector& a, const outputVector& b) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(a.noMultipleIds());
    assert(b.noMultipleIds());

    sort2<int, double> sorter;

    ivector aIds, bIds;
    dvector aValues, bValues;

    sorter.apply(a.ids, a.values, aIds, aValues);
    sorter.apply(b.ids, b.values, bIds, bValues);

    int s=a.ids.size()+b.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<aIds.size() && j<bIds.size()) {
      if (aIds.at(i)==bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=lti::max(aValues.at(i),bValues.at(j));
        k++;
        i++;
        j++;
      } else if (aIds.at(i)<bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=aValues.at(i);
        k++;
        i++;
      } else {
        ids.at(k)=bIds.at(j);
        values.at(k)=bValues.at(j);
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<aIds.size()) {
      ids.at(k)=aIds.at(i);
      values.at(k)=aValues.at(i);
      k++;
      i++;
    }
    while (j<bIds.size()) {
      ids.at(k)=bIds.at(j);
      values.at(k)=bValues.at(j);
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::min(const outputVector& other) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(noMultipleIds());
    assert(other.noMultipleIds());

    sort2<int, double> sorter;

    ivector tIds, oIds;
    dvector tValues, oValues;

    sorter.apply(ids, values, tIds, tValues);
    sorter.apply(other.ids, other.values, oIds, oValues);

    int s=ids.size()+other.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<tIds.size() && j<oIds.size()) {
      if (tIds.at(i)==oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=lti::min(tValues.at(i), oValues.at(j));
        k++;
        i++;
        j++;
      } else if (tIds.at(i)<oIds.at(j)) {
        ids.at(k)=tIds.at(i);
        values.at(k)=0.;
        k++;
        i++;
      } else {
        ids.at(k)=oIds.at(j);
        values.at(k)=0.;
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<tIds.size()) {
      ids.at(k)=tIds.at(i);
      values.at(k)=0.;
      k++;
      i++;
    }
    while (j<oIds.size()) {
      ids.at(k)=oIds.at(j);
      values.at(k)=0.;
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  classifier::outputVector&
  classifier::outputVector::min(const outputVector& a, const outputVector& b) {

    // Check whether ids are used more than once in the oV -> not allowed
    assert(a.noMultipleIds());
    assert(b.noMultipleIds());

    sort2<int, double> sorter;

    ivector aIds, bIds;
    dvector aValues, bValues;

    sorter.apply(a.ids, a.values, aIds, aValues);
    sorter.apply(b.ids, b.values, bIds, bValues);

    int s=a.ids.size()+b.ids.size();
    ids.resize(s, -1, false, false);
    values.resize(s, 0., false, false);

    int i=0;
    int j=0;
    int k=0;
    //First merge/add vectors when there are values left in both vectors
    //Take smaller ids first. Because of sorting there is self-syncing, ie
    //equal ids will be found again.
    while (i<aIds.size() && j<bIds.size()) {
      if (aIds.at(i)==bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=lti::min(aValues.at(i), bValues.at(j));
        k++;
        i++;
        j++;
      } else if (aIds.at(i)<bIds.at(j)) {
        ids.at(k)=aIds.at(i);
        values.at(k)=0.;
        k++;
        i++;
      } else {
        ids.at(k)=bIds.at(j);
        values.at(k)=0.;
        k++;
        j++;
      }
    }
    //Check whether there are any values left at the end of one of the
    //vectors. If yes copy to the target.
    while (i<aIds.size()) {
      ids.at(k)=aIds.at(i);
      values.at(k)=0.;
      k++;
      i++;
    }
    while (j<bIds.size()) {
      ids.at(k)=bIds.at(j);
      values.at(k)=0.;
      k++;
      j++;
    }

    ids.resize(k);
    values.resize(k);

    sortedAsc=false;
    sortedDesc=false;
    return *this;
  }

  bool classifier::outputVector::noMultipleIds() const {

    sort<int> sorter;
    ivector tids;
    sorter.apply(ids, tids);

    int i;
    for (i=0; i<tids.size()-1; i++) {
      if (ids.at(i)==ids.at(i+1)) {
        return false;
      }
    }
    return true;
  }

# ifndef _LTI_MSC_6
  bool classifier::outputVector::write(ioHandler& handler,
                                       const bool complete) const
# else
  bool classifier::outputVector::writeMS(ioHandler& handler,
                                         const bool complete) const
# endif
  {
    bool b(true);

    if (complete) {
      b = b && handler.writeBegin();
    }

    b = b && lti::write(handler, "winner", winner);
    b = b && lti::write(handler, "confidence", confidence);
    b = b && lti::write(handler, "reject", reject);
    b = b && lti::write(handler, "valid", valid);
    b = b && lti::write(handler, "sortedAsc", sortedAsc);
    b = b && lti::write(handler, "sortedDesc", sortedDesc);
    b = b && lti::write(handler, "ids", ids);
    b = b && lti::write(handler, "values", values);


    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::outputVector::write(ioHandler& handler,
                                       const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return writeMS(handler,complete);
  }
# endif


# ifndef _LTI_MSC_6
  bool classifier::outputVector::read(ioHandler& handler,
                                      const bool complete)
# else
  bool classifier::outputVector::readMS(ioHandler& handler,
                                        const bool complete)
# endif
  {
    bool b(true);

    if (complete) {
      b = b && handler.readBegin();
    }

    b = b && lti::read(handler, "winner", winner);
    b = b && lti::read(handler, "confidence", confidence);
    b = b && lti::read(handler, "reject", reject);
    b = b && lti::read(handler, "valid", valid);
    b = b && lti::read(handler, "sortedAsc", sortedAsc);
    b = b && lti::read(handler, "sortedDesc", sortedDesc);
    b = b && lti::read(handler, "ids", ids);
    b = b && lti::read(handler, "values", values);


    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::outputVector::read(ioHandler& handler,
                                      const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // classifier::outputTemplate
  // --------------------------------------------------

  classifier::outputTemplate::outputTemplate()
    : ioObject(), probList(0), defaultIds(0) {
    multipleMode=ObjProb;
  }

  classifier::outputTemplate::outputTemplate(const outputTemplate& other) 
    : ioObject() {

    copy(other);
  }

  classifier::outputTemplate::outputTemplate(const ivector& theIds)
    : probList(0) {

    defaultIds.copy(theIds);
    multipleMode=Ignore;
  }

  classifier::outputTemplate::outputTemplate(const int& size)
    : defaultIds(size) {

    probList=std::vector<outputVector>(size);
    multipleMode=ObjProb;
  }

  classifier::outputTemplate&
  classifier::outputTemplate::copy(const outputTemplate& other) {

    multipleMode=other.multipleMode;
    probList=other.probList;
    defaultIds.copy(other.defaultIds);
    return *this;
  }

  classifier::outputTemplate* classifier::outputTemplate::clone() const {

    return new outputTemplate(*this);
  }

  void classifier::outputTemplate::setMultipleMode(const eMultipleMode& mode) {

    multipleMode=mode;
  }

  classifier::outputTemplate::eMultipleMode
  classifier::outputTemplate::getMultipleMode() const {

    return multipleMode;
  }

  bool classifier::outputTemplate::setIds(const ivector& theIds) {

    defaultIds.copy(theIds);
    return true;
  }

  const ivector& classifier::outputTemplate::getIds() const {

    return defaultIds;
  }

  bool classifier::outputTemplate::setProbs(const int& pos,
                                            const ivector& theIds,
                                            const dvector& theValues) {

    if (pos<0 || pos>=static_cast<int>(probList.size())) {
      return false;
    }

    probList[pos]=outputVector(theIds, theValues);

    return true;
  }

  bool classifier::outputTemplate::setProbs(const int& pos,
                                            const outputVector& outV) {

    if (pos<0 || pos>=static_cast<int>(probList.size())) {
      return false;
    }

    probList[pos]=outV;

    return true;
  }

  bool classifier::outputTemplate::setData(const int& pos,
                                           const int& id,
                                           const outputVector& outV) {

    bool b=true;

    if (pos<0) {
      return false;
    }

    if (pos<static_cast<int>(probList.size())) {
      probList[pos]=outV;
    } else {
      b=false;
    }

    if (pos<defaultIds.size()) {
      defaultIds.at(pos)=id;
    } else {
      b=false;
    }

    return b;
  }

  const classifier::outputVector&
  classifier::outputTemplate::getProbs(const int& pos) const {

    if (pos<0 || pos>(signed int)probList.size()) {
      return probList[0];
    } else {
      return probList[pos];
    }
  }

  int classifier::outputTemplate::size() const {

    int pSize=probList.size();
    int iSize=defaultIds.size();
    return (pSize>iSize ? pSize : iSize);
  }

  bool classifier::outputTemplate::apply(const dvector& data,
                                         outputVector& result) const {

    bool b=true;
    eMultipleMode localMode=multipleMode;

    // check if defaultIds is valid when mode is Ignore
    if (localMode==Ignore && defaultIds.size()==0) {
      // if not ok set mode to Max if probList is there else bad error
      if (probList.size()==0) {
        result=outputVector(0);
        return false;
      } else {
        localMode = Max;
        b=false;
      }
    }

    // check if probList is valid when mode is other than Ignore
    if (localMode!=Ignore && probList.size()==0) {
      // if not ok, set mode to Ignore if defaultIds !=0 else bad error
      if (defaultIds.size()==0) {
        result=outputVector(0);
        return false;
      } else {
        localMode = Ignore;
        b = false;
      }
    }

    switch (localMode) {

    case Ignore: {
      result=outputVector(defaultIds, data);
      break;
    }

    case Max: {
      ivector ids(data.size());
      std::vector<outputVector>::const_iterator it=probList.begin();
      std::vector<outputVector>::const_iterator ite=probList.end();
      int i=0;
      while (it!=ite) {
        ids.at(i++)=it->maxId();
        it++;
      }
      result=outputVector(ids, data);
      break;
    }

    case Uniform: {
      outputVector tmpVec;
      dvector uniProbs;
      int uniSize;
      tmpVec=probList[0];
      uniSize=tmpVec.size();
      uniProbs=dvector(uniSize, 1./uniSize);
      tmpVec.setValues(uniProbs);
      result=tmpVec.mul(data.at(0));
      std::vector<outputVector>::const_iterator it=probList.begin();
      std::vector<outputVector>::const_iterator ite=probList.end();
      int i=1;
      it++;
      while (it!=ite) {
        tmpVec=*it++;
        uniSize=tmpVec.size();
        uniProbs=dvector(uniSize, 1./uniSize);
        tmpVec.setValues(uniProbs);
        result.add(tmpVec.mul(data.at(i++)));
      }
      break;
    }

    case ObjProb:
    default: {
      result=probList[0];
      result.mul(data.at(0));
      std::vector<outputVector>::const_iterator it=probList.begin();
      std::vector<outputVector>::const_iterator ite=probList.end();
      int i=1;
      outputVector tmpVec;
      it++;
      while (it!=ite) {
        tmpVec=*it++;
        result.add(tmpVec.mul(data.at(i++)));
      }
    }
    }
    return b;
  }

# ifndef _LTI_MSC_6
  bool classifier::outputTemplate::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool classifier::outputTemplate::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b(true);

    if (complete) {
      b = b && handler.writeBegin();
    }


    switch (multipleMode) {
    case Ignore:
      b = b && lti::write(handler, "multipleMode", "Ignore");
      break;
    case Max:
      b = b && lti::write(handler, "multipleMode", "Max");
      break;
    case Uniform:
      b = b && lti::write(handler, "multipleMode", "Uniform");
      break;
    case ObjProb:
      b = b && lti::write(handler, "multipleMode", "ObjProb");
      break;
    default:
      b = b && lti::write(handler, "multipleMode", "ObjProb");
    }

    b = b && lti::write(handler, "defaultIds", defaultIds);
    b = b && lti::write(handler, "probList", probList);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::outputTemplate::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return writeMS(handler,complete);
  }
# endif

# ifndef _LTI_MSC_6
  bool classifier::outputTemplate::read(ioHandler& handler,
                                        const bool complete)
# else
  bool classifier::outputTemplate::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b(true);

    if (complete) {
      b = b && handler.readBegin();
    }

    std::string str;
    lti::read(handler,"multipleMode",str);

    if (str == "Ignore") {
      multipleMode = Ignore;
    } else if (str == "Max") {
      multipleMode = Max;
    } else if (str == "Uniform") {
      multipleMode = Uniform;
    } else if (str == "ObjProb") {
      multipleMode = ObjProb;
    } else {
      multipleMode = ObjProb;
    }

    b = b && lti::read(handler, "defaultIds", defaultIds);
    b = b && lti::read(handler, "probList", probList);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::outputTemplate::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // classifier::parameters
  // --------------------------------------------------

  // default constructor
  classifier::parameters::parameters() : ioObject() {
    multipleMode=outputTemplate::ObjProb;
  }

  // copy constructor
  classifier::parameters::parameters(const parameters& other) : ioObject() {
    copy(other);
  }

  // destructor
  classifier::parameters::~parameters() {
  }

  // get type name
  const char* classifier::parameters::getTypeName() const {
    return "classifier::parameters";
  }

  // copy member

  classifier::parameters&
    classifier::parameters::copy(const parameters& other) {
    multipleMode=other.multipleMode;
    return *this;
  }

  // clone member
  classifier::parameters* classifier::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool classifier::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool classifier::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    switch (multipleMode) {
    case outputTemplate::Ignore:
      b = b && lti::write(handler, "multipleMode", "Ignore");
      break;
    case outputTemplate::Uniform:
      b = b && lti::write(handler, "multipleMode", "Uniform");
      break;
    case outputTemplate::ObjProb:
      b = b && lti::write(handler, "multipleMode", "ObjProb");
      break;
    default:
      b = b && lti::write(handler, "multipleMode", "ObjProb");
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool classifier::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool classifier::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    std::string str;
    lti::read(handler,"multipleMode",str);

    if (str == "Ignore") {
      multipleMode = outputTemplate::Ignore;
    } else if (str == "Uniform") {
      multipleMode = outputTemplate::Uniform;
    } else if (str == "ObjProb") {
      multipleMode = outputTemplate::ObjProb;
    } else {
      multipleMode = outputTemplate::ObjProb;
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classifier::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------------------------------
  // exceptions
  // --------------------------------------------------------------------------

  const char* classifier::invalidParametersException::getTypeName() const {
    return "classifier::invalidParametersException";
  }

  const char* classifier::invalidMethodException::getTypeName() const {
    return "classifier::invalidMethodException";
  }

  const char* classifier::parametersOutOfRangeException::getTypeName() const {
    return "classifier::parametersOutOfRangeException";
  }


  // --------------------------------------------------
  // classifier
  // --------------------------------------------------

  // default empty string for the functor
	//  const char* const classifier::emptyString = "";

  // default constructor
  classifier::classifier()
    : ioObject(),status(),params(0),progressBox(0) {
  }

  // copy constructor
  classifier::classifier(const classifier& other)
    : ioObject(), status(), params(0)  {
    copy(other);
  }

  // destructor
  classifier::~classifier() {
    delete params;
    params = 0;
    delete progressBox;
    progressBox = 0;
  }

//   /*
//    * return the last message set with setStatusString().  This will
//    * never return 0.  If no status-string has been set yet an empty string
//    * (pointer to a string with only the char(0)) will be returned.
//    */
//   const char* classifier::getStatusString() const {
//     if (isNull(statusString)) {
//       return emptyString;
//     } else {
//       return statusString;
//     }
//   }

//   /*
//    * set a status string.
//    *
//    * @param msg the const string to be reported next time by
//    * getStatusString()
//    * This message will be usually set within the apply methods to indicate
//    * an error cause.
//    */
//   void classifier::setStatusString(const char* msg) const {
//     delete[] statusString;
//     statusString = 0;

//     statusString = new char[strlen(msg)+1];
//     strcpy(statusString,msg);
//   }

  // returns the name of this type
  const char* classifier::getTypeName() const {
    return "classifier";
  }

  // copy member
  classifier&
    classifier::copy(const classifier& other) {

    if(other.validParameters()) {
      setParameters(other.getParameters());
    } else {
      params = 0;
    }

    outTemplate=other.outTemplate;

    if (other.validProgressObject()) {
      setProgressObject(other.getProgressObject());
    } else {
      progressBox = 0;
    }


    return (*this);

  }

  // return parameters
  const classifier::parameters& classifier::getParameters() const {
    return *params;
  }

  // set the progress object
  void classifier::setProgressObject(const progressInfo& progBox) {
    removeProgressObject();
    progressBox = progBox.clone();
  }

  // remove the active progress object
  void classifier::removeProgressObject() {
    delete progressBox;
    progressBox = 0;
  }

  // valid progress object
  bool classifier::validProgressObject() const {
    return notNull(progressBox);
  }

  progressInfo& classifier::getProgressObject() {
    return *progressBox;
  }

  const progressInfo& classifier::getProgressObject() const {
    return *progressBox;
  }

  /*
   * sets the classifier's parameters.
   * The functor keeps its own copy of the given parameters.
   */
  bool classifier::setParameters(const classifier::parameters& theParams) {
    delete params;
    params = theParams.clone();
    outTemplate.setMultipleMode(params->multipleMode);

    return true;
  }

  /*
   * sets the classifier's outputTemplate
   */
  void
  classifier::setOutputTemplate(const classifier::outputTemplate& outTempl) {
    outTemplate=outTempl;
  }

  /*
   * returns the outputTemplate
   */
  const classifier::outputTemplate& classifier::getOutputTemplate() const {
    return outTemplate;
  }


  /*
   * write the classifier in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  bool classifier::write(ioHandler& handler,const bool complete) const {
    bool b(true);


    if (complete) {
      b = b && handler.writeBegin();
    }

    b = b && lti::write(handler,"parameters",getParameters());
    b = b && lti::write(handler,"outputTemplate", outTemplate);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  /*
   * read the classifier from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  bool classifier::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = b && handler.readBegin();
    }

    // read parameters
    b = b && handler.readBegin();
    b = b && handler.trySymbol("parameters");
    b = b && (params!=0);
    b = b && params->read(handler, true);
    b = b && handler.readEnd();


//      b = b && lti::read(handler,"parameters",getParameters());
    b = b && lti::read(handler,"outputTemplate", outTemplate);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  // ------------------------------------------------------------------
  // Global storable interface
  // ------------------------------------------------------------------


  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::parameters& p,
             const bool complete) {
    return p.write(handler,complete);
  }

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::parameters& p,
            const bool complete) {
    return p.read(handler,complete);
  }

  /*
   * write the functor in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier& p,
             const bool complete) {
    return p.write(handler,complete);
  }

  /*
   * read the functor from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier& p,
            const bool complete) {
    return p.read(handler,complete);
  }


  /*
   * write the functor::output in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::outputVector& p,
             const bool complete) {
    return p.write(handler,complete);
  }

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::outputVector& p,
            const bool complete) {
    return p.read(handler,complete);
  }

  /*
   * write the functor::output in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::outputTemplate& p,
             const bool complete) {
    return p.write(handler,complete);
  }

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::outputTemplate& p,
            const bool complete) {
    return p.read(handler,complete);
  }

}

namespace std {

  ostream&
  operator<<(ostream& s,const lti::classifier::outputVector& o) {
    lti::lispStreamHandler h(s);
    o.write(h);
    return s;
  }

}
