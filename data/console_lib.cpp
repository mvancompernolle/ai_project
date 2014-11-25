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

#include "ltiException.h"
#include <iostream>

int main(int argc,char *argv[]) {
  try {
    // TODO:
    // Write your main application here:
    std::cout << "Hello World!" << std::endl;
  }
  catch (lti::exception& exp) {
    std::cout << "An LTI::EXCEPTION was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (std::exception& exp) {
    std::cout << "std::exception was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (...) {
    std::cout << "Unknown exception thrown!" << std::endl;
  }
  return 0;
} // main
