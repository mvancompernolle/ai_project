/*
 * Copyright (C) 2004, 2005, 2006
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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiMathFunction.cpp
 * authors ....: Lars Libuda
 * organization: LTI, RWTH Aachen
 * creation ...: 2.11.2004
 * revisions ..: $Id: ltiFunctionGenerator.cpp,v 1.6 2008/08/17 22:20:13 alvarado Exp $
 */

#include "ltiFunctionGenerator.h"
#include <cstdio>
#include <cstring>

namespace lti {
  // --------------------------------------------------
  // functionGenerator::parameters
  // --------------------------------------------------

  // default constructor
  functionGenerator::parameters::parameters()
    : mathFunction::parameters() {
   
    function = std::string();
    nameVarX = "x";
    nameVarY = "y";
  }

  // copy constructor
  functionGenerator::parameters::parameters(const parameters& other)
    : mathFunction::parameters() {
    copy(other);
  }

  // destructor
  functionGenerator::parameters::~parameters() {
  }

  // get type name
  const char* functionGenerator::parameters::getTypeName() const {
    return "functionGenerator::parameters";
  }

  // copy member

  functionGenerator::parameters&
    functionGenerator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    mathFunction::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    mathFunction::parameters& (mathFunction::parameters::* p_copy)
      (const mathFunction::parameters&) =
      mathFunction::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    function = other.function;
    nameVarX = other.nameVarX;
    nameVarY = other.nameVarY;

    return *this;
  }

  // alias for copy member
  functionGenerator::parameters&
    functionGenerator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* functionGenerator::parameters::clone() const {
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
  bool functionGenerator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool functionGenerator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"function",function);
      lti::write(handler,"nameVarX",nameVarX);
      lti::write(handler,"nameVarY",nameVarY);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && mathFunction::parameters::write(handler,false);
# else
    bool (mathFunction::parameters::* p_writeMS)(ioHandler&,const bool) const =
      mathFunction::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool functionGenerator::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool functionGenerator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool functionGenerator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"function",function);
      lti::read(handler,"nameVarX",nameVarX);
      lti::read(handler,"nameVarY",nameVarY);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && mathFunction::parameters::read(handler,false);
# else
    bool (mathFunction::parameters::* p_readMS)(ioHandler&,const bool) =
      mathFunction::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool functionGenerator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // functionGenerator
  // --------------------------------------------------

  // default constructor
  functionGenerator::functionGenerator()
    : mathFunction(){

    iError = 0;
    szErrorPos = NULL;     
    bHasTwoVars = false;    
    pRootNode = NULL;      

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  functionGenerator::functionGenerator(const parameters& par)
    : mathFunction() {

    iError = 0;         
    szErrorPos = NULL;     
    bHasTwoVars = false;    
    pRootNode = NULL;      

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  functionGenerator::functionGenerator(const functionGenerator& other)
    : mathFunction() {
    copy(other);
  }

  // destructor
  functionGenerator::~functionGenerator() {
    if (pRootNode) {
      delete pRootNode;
      pRootNode = NULL;
    }
  }

  // returns the name of this type
  const char* functionGenerator::getTypeName() const {
    return "functionGenerator";
  }

  // copy member
  functionGenerator&
    functionGenerator::copy(const functionGenerator& other) {
      mathFunction::copy(other);

    iError = 0;         
    szErrorPos = NULL;     
    bHasTwoVars = false;    
    pRootNode = NULL;      

    return (*this);
  }

  // alias for copy member
  functionGenerator&
    functionGenerator::operator=(const functionGenerator& other) {
    return (copy(other));
  }


  // clone member
  functor* functionGenerator::clone() const {
    return new functionGenerator(*this);
  }

  // return parameters
  const functionGenerator::parameters&
    functionGenerator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool functionGenerator::updateParameters() {
    const parameters& params = getParameters();
    if (pRootNode) {
      delete pRootNode;
      pRootNode = NULL;
    }
    iError = 0;
    bHasTwoVars = false;
    char* szTerm = new char[params.function.size() + 1];
    strcpy(szTerm,params.function.c_str());
    strLower(szTerm);
    szErrorPos = szTerm + strlen(szTerm) - 1;
    char* szHelpTerm = skipWhiteSpace(szTerm);
    if (strlen(szHelpTerm)) {
      if (strchr("+-*/^",szHelpTerm[strlen(szHelpTerm) - 1]) == NULL) {
        pRootNode = compile4(szHelpTerm);
      }
      else {
        iError = 3;
      }
    }
    else {
      iError = 5;
    }
    int iErrorPos = (int)(szErrorPos - szTerm);
    if (iError) {
      delete pRootNode;
      pRootNode = NULL;
      char szMsg[100];
      switch(iError) {
        case 1:
          sprintf(szMsg,"Error in term at position %i: Invalid character found.",iErrorPos);
          break;
        case 2:
          sprintf(szMsg,"Error in term at position %i: Missing bracket.",iErrorPos);
          break;
        case 3:
          sprintf(szMsg,"Error in term at position %i: Unexpected end of term.",iErrorPos);
          break;
        case 4:
          sprintf(szMsg,"Error in term at position %i: Missing operand.",iErrorPos);
          break;
        case 5:
          sprintf(szMsg,"Error in term at position %i: Invalid input.",iErrorPos);
          break;
      }
      setStatusString(szMsg);
    }
    delete [] szTerm;
    return (iError != 0);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type double!
  bool functionGenerator::apply(const double& x,const double& y,double& result) const{
    if (iError) {
      return false;
    }
    result = pRootNode->calculate(x,y);
    return pRootNode->error();
  };

  bool functionGenerator::apply(const double& x,double& result) const{
    if (iError) {
      return false;
    }
    double y = 1.0;
    result = pRootNode->calculate(x,y);
    return pRootNode->error();
  };

  // -------------------------------------------------------------------
  // private methods
  // -------------------------------------------------------------------
  const double functionGenerator::pi = 3.14159265358979323846;
  const double functionGenerator::e  = 2.71828182845904523536;
  const char*  functionGenerator::szFuncs[] = {"arcsin","arccos","arctan","arccot","sinh","cosh","tanh","coth","arsinh","arcosh","artanh","arcoth","sin","cos","tan","cot","sec","csc","ln","lg","ld","sqrt","abs","sgn","int",0};

  char* functionGenerator::strLower(char* s) {
    for(char* ptr = s;*ptr;ptr++)
      if ((*ptr >= 'A') && (*ptr <= 'Z')) {
        *ptr += 32;
      }
    return s;
  }

  int functionGenerator::skipBrackets(char* s, int& i) {
    int count = -1;
    while ((count != 0) && (i >= 0)) {
      i--;
      switch(s[i]) {
        case ')' : count--;
                   break;
        case '(' : count++;
                   break;
      }
    }
    return count;
  }

  char* functionGenerator::deleteBrackets(char* s) {
    unsigned int i, klammer = 0;
    while ((s[0] == '(') && (s[strlen(s) - 1] == ')')) {
      for (i = 0; i < strlen(s); i++) {
        switch(s[i]) {
          case '(' : klammer++;
                     break;
          case ')' : klammer--;
                     break;
        }
        if ((klammer == 0) && (i < strlen(s) - 1)) {
          return s;
        }
      }
      s[strlen(s) - 1] = '\0';
      s++;
    }
    return s;
  }

  char* functionGenerator::skipWhiteSpace(char* s) {
    char* ptr = s;
    while((isspace(*ptr)) && (*ptr)) ptr++;
    if (!(*ptr)) return ptr;
    char* endptr = ptr + strlen(ptr) - 1;
    while(isspace(*endptr)) endptr--;
    endptr++;
    *endptr = '\0';
    return ptr;
  }

  functionGenerator::node* functionGenerator::compile1(char* s) {
    const parameters& params = getParameters();
    s = skipWhiteSpace(s);
    s = deleteBrackets(s);
    s = skipWhiteSpace(s);
    szErrorPos = s;
    if (strlen(s) == 0) {
      iError = 4;
      return NULL;
    }
    for(int i = 0; szFuncs[i] != 0; i++) {                      
      if (strncmp(s,szFuncs[i],strlen(szFuncs[i])) == 0) {
        if (s[strlen(szFuncs[i])] != '(') {                                     
          iError = 1;
          return NULL;
        }
        return new nodeFunc(compile4(s += strlen(szFuncs[i])),i);
      }
    }
    if (strcmp(s,params.nameVarX.c_str()) == 0) {
	    return new nodeVarX();
    }
    else if (strcmp(s,params.nameVarY.c_str()) == 0) {
      bHasTwoVars = 1;
      return new nodeVarY();
    }
    else if (strcmp(s,"e") == 0) {
      return new nodeConst(e);
    }
    else if (strcmp(s,"pi") == 0) {
      return new nodeConst(pi);
    }
    else {
      char *endptr;
      double dResult = strtod(s,&endptr);
      if (*endptr) {
        if (strchr("()",*s)) {
          iError = 2;
        }
        else {
          iError = 1;
        }
        szErrorPos = endptr;
      }
      return new nodeConst(dResult);
    }
  }

  functionGenerator::node* functionGenerator::compile2(char* s) {
    s = skipWhiteSpace(s);
    s = deleteBrackets(s);
    s = skipWhiteSpace(s);
    szErrorPos = s;
    for (int i = static_cast<int>(strlen(s)); i >= 0; i--) {
      switch(s[i]) {
        case ')' : if (skipBrackets(s,i)) {
                     iError = 2;
                     szErrorPos = s + strlen(s) - 1;
                     return NULL;
                   }
                   break;
        case '^' : s[i] = '\0';
                   return new nodePow(compile4(s),compile4(s + i + 1));
     }
    }
    return compile1(s);
  }

  functionGenerator::node* functionGenerator::compile3(char* s) {
    s = skipWhiteSpace(s);
    s = deleteBrackets(s);
    s = skipWhiteSpace(s);
    szErrorPos = s;
    for (int i = static_cast<int>(strlen(s)); i >= 0; i--) {
      switch(s[i]) {
        case ')' : if (skipBrackets(s,i)) {
                      iError = 2;
                      szErrorPos = s + strlen(s) - 1;
                      return NULL;
                   }
                   break;
        case '*' : s[i] = '\0';
                   return new nodeMul(compile4(s),compile4(s + i + 1));
        case '/' : s[i] = '\0';
                   return new nodeDiv(compile4(s),compile4(s + i + 1));
      }
    }
    return compile2(s);
  }

  functionGenerator::node* functionGenerator::compile4(char* s) {
    if (iError) return NULL;
    s = skipWhiteSpace(s);
    s = deleteBrackets(s);
    s = skipWhiteSpace(s);
    szErrorPos = s;
    for (int i = static_cast<int>(strlen(s)); i >= 0; i--) {
      switch(s[i]) {
        case ')' : if (skipBrackets(s,i)) {
                     iError = 2;
                     szErrorPos = s + strlen(s) - 1;
                     return NULL;
                   }
                   break;
        case '+' : s[i] = '\0';
                   if (strlen(s) == 0) {
                     return compile4(s + i + 1);
                   }
                   else {
                     return new nodeAdd(compile4(s),compile4(s + i + 1));
                   }
        case '-' : s[i] = '\0';
                   if (strlen(s) == 0) {
                     return new nodeMul(new nodeConst(-1),compile4(s + i + 1));
                   }
                   else {
                     return new nodeSub(compile4(s),compile4(s + i + 1));
                   }
      }
    }
    return compile3(s);
  }

  // -------------------------------------------------------------------
  // internal classes for the tree representation of the function
  // -------------------------------------------------------------------

  bool functionGenerator::node::bMathError = false;

  functionGenerator::node::node(node *pl, node *pr) {
    pLeft  = pl;
    pRight = pr;
  }

  functionGenerator::node::~node() {
    delete getLeftChild();
    delete getRightChild();
  }

  bool functionGenerator::node::error() {
    bool bError = bMathError;
    bMathError = false;
    return bError;
  }

  functionGenerator::nodeDiv::nodeDiv(node *l,node *r) : node(l,r) {
  }

  double functionGenerator::nodeDiv::calculate(const double& x, const double& y) {
    double dResult = getRightChild()->calculate(x,y);
    if (dResult == 0) {
      bMathError = true;
      return 1.0;
    }
    else {
      return getLeftChild()->calculate(x,y) / dResult;
    }
  }

  functionGenerator::nodePow::nodePow(node *l, node *r) : node(l,r) {
  }

  double functionGenerator::nodePow::calculate(const double& x, const double& y) {
    return nodeFunc::Pow(getLeftChild()->calculate(x,y),getRightChild()->calculate(x,y));
  }

  functionGenerator::nodeFunc::tMathFunc functionGenerator::nodeFunc::pFuncs[] = 
    {nodeFunc::Arcsin,nodeFunc::Arccos,nodeFunc::Arctan,nodeFunc::Arccot,nodeFunc::Sinh,nodeFunc::Cosh,nodeFunc::Tanh,nodeFunc::Coth,
     nodeFunc::Arsinh,nodeFunc::Arcosh,nodeFunc::Artanh,nodeFunc::Arcoth,nodeFunc::Sin,nodeFunc::Cos,nodeFunc::Tan,nodeFunc::Cot,
     nodeFunc::Sec,nodeFunc::Cosec,nodeFunc::Ln,nodeFunc::Lg,nodeFunc::Ld,nodeFunc::Sqrt,nodeFunc::Abs,nodeFunc::Sgn,nodeFunc::Intfunc};

  functionGenerator::nodeFunc::nodeFunc(node *pNode, int iFuncNr) : node(NULL,pNode),iFunc(iFuncNr) {
  }

  double functionGenerator::nodeFunc::calculate(const double& x, const double& y) {
    return pFuncs[iFunc] (getRightChild()->calculate(x,y));
  }

  double functionGenerator::nodeFunc::Pow(double x,double y) {
    if ((x == 0.0) && (y <= 0.0)) {
      bMathError = true;
      return 1.0;
    }
    double intValue,floatValue;
    floatValue = modf(y,&intValue);
    if ((x < 0) && (floatValue != 0.0)) {
      bMathError = true;
      return 1.0;
    }
    return pow(x,y);
  }

  double functionGenerator::nodeFunc::Sin(double x) {
    return sin(x);
  }

  double functionGenerator::nodeFunc::Cos(double x) {
    return cos(x);
  }

  double functionGenerator::nodeFunc::Tan(double x) {
    return tan(x);
  }

  double functionGenerator::nodeFunc::Cot(double x) {
    double Value = tan(x);
    if (Value != 0.0) return 1/Value;
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Sec(double x) {
    double Value = cos(x);
    if (Value != 0.0) return 1/Value;
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Cosec(double x) {
    double Value = sin(x);
    if (Value != 0.0) return 1/Value;
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Arcsin(double x) {
    if ((x <= 1.0) && (x >= -1.0)) return asin(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Arccos(double x) {
    if ((x <= 1.0) && (x >= -1.0)) return acos(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Arctan(double x) {
    return atan(x);
  }

  double functionGenerator::nodeFunc::Arccot(double x) {
    return (3.14159265358979323846 / 2) - atan(x);
  }

  double functionGenerator::nodeFunc::Sinh(double x) {
    return sinh(x);
  }

  double functionGenerator::nodeFunc::Cosh(double x) {
    return cosh(x);
  }

  double functionGenerator::nodeFunc::Tanh(double x) {
    return tanh(x);
  }

  double functionGenerator::nodeFunc::Coth(double x) {
    double Value = tanh(x);
    if (Value != 0.0) return 1/Value;
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Arsinh(double x) {
    return log(x + sqrt(x * x + 1));
  }

  double functionGenerator::nodeFunc::Arcosh(double x) {
    if (x >= 1.0) return log(x + sqrt(x * x - 1));
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Artanh(double x) {
    if ((x < 1.0) && (x > -1.0)) return 0.5 * log((1 + x)/(1 - x));
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Arcoth(double x) {
    if ((x > 1.0) || (x < -1.0)) return 0.5 * log((x + 1)/(x - 1));
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Ln(double x) {
    if (x > 0) return log(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Lg(double x) {
    if (x > 0) return log10(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Ld(double x) {
    if (x > 0) return (1 / log(2.0)) * log(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Sqrt(double x) {
    if (x >= 0) return sqrt(x);
    bMathError = true;
    return 1.0;
  }

  double functionGenerator::nodeFunc::Abs(double x) {
    return fabs(x);
  }

  double functionGenerator::nodeFunc::Sgn(double x) {
    if (x < 0)
      return -1.0;
    else
      return 1.0;
  }

  double functionGenerator::nodeFunc::Intfunc(double x) {
    long int i = (long int)x;
    if (x < 0) i--;
    return (double)i;
  }

}
