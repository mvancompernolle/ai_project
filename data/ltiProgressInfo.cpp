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
 * file .......: ltiProgressInfo.cpp
 * authors ....: Pablo Alvarado, Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 10.8.2000
 * revisions ..: $Id: ltiProgressInfo.cpp,v 1.5 2006/02/07 18:22:11 ltilib Exp $
 */

#include "ltiProgressInfo.h"
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // progressInfoBase
  // --------------------------------------------------

  progressInfo::progressInfo(const std::string& theTitle,
                                     const int& maximumSteps) 
    : object(),detailLevel(1) {
  }

  progressInfo::progressInfo(const progressInfo& other) : object() {
    copy(other);
  }

  progressInfo::~progressInfo() {
  }

  progressInfo& progressInfo::copy(const progressInfo& other) {
    return *this;
  }

  void progressInfo::setDetailLevel(const int level) {
    detailLevel = level;
  }

  int progressInfo::getDetailLevel() const {
    return detailLevel;
  }

  void progressInfo::substep(const int detail,
                             const std::string& info) {
  }

  // --------------------------------------------------
  // progressInfo
  // --------------------------------------------------

  streamProgressInfo::streamProgressInfo(std::ostream& outStream,
                                         const std::string& theTitle,
                                         const int& maximumSteps)
    : maxSteps(maximumSteps),lastStep(0),lastStepText(""),
      title(theTitle),out(&outStream) {
    endline="\n";
  }

  streamProgressInfo::streamProgressInfo(const std::string& theTitle,
                                         const int& maximumSteps)
    : progressInfo(),maxSteps(maximumSteps),lastStep(0),lastStepText(""),
      title(theTitle),out(&std::cout) {
    endline="\n";
  }

  streamProgressInfo::streamProgressInfo(const streamProgressInfo& other)
    : progressInfo() {
    copy(other);
  }

  streamProgressInfo::~streamProgressInfo() {
    // do not delete the out stream...
  }

  void streamProgressInfo::setTitle(const std::string& theTitle) {
    title = theTitle;
  }

  void streamProgressInfo::setMaxSteps(const int& maximalSteps) {
    maxSteps = maximalSteps;
  }

  bool streamProgressInfo::breakRequested() const {
    return false;
  }

  void streamProgressInfo::useLineFeed(bool uself) {
    endline=uself ? "\n" : "        \r";
  }

  void streamProgressInfo::step(const std::string& progressTxt) {
    lastStep++;
    lastStepText = progressTxt;

    int tmp=int(double(1000*lastStep)/double(maxSteps));
    char buf[32];

    sprintf(buf," %4d (%5.1f%%) ", lastStep, double(tmp)/10.0);

    *out << buf << progressTxt << endline;
    out->flush();
  }

  void streamProgressInfo::substep(const int detail,
                                   const std::string& progressTxt) {
    if (detail <= detailLevel) {
      std::string str(15+((detail<2)?0:(detail-1)*2),' ');
      *out << str << progressTxt << endline;
      out->flush();
    }
  }

  void streamProgressInfo::reset() {
    lastStep = 0;
    lastStepText = "";
  }

  streamProgressInfo&
  streamProgressInfo::copy(const streamProgressInfo& other) {
    maxSteps     = other.maxSteps;
    lastStep     = other.lastStep;
    lastStepText = other.lastStepText;
    title        = other.title;
    out          = other.out;
    endline      = other.endline;
    return *this;
  }

  progressInfo* streamProgressInfo::clone() const {
    return new streamProgressInfo(*this);
  }

}
