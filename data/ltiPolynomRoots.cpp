/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiPolynomRoots.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 6.4.2003
 * revisions ..: $Id: ltiPolynomRoots.cpp,v 1.8 2006/09/05 10:41:14 ltilib Exp $
 */

#include <limits>
#include "ltiPolynomRoots.h"
#include "ltiMath.h"

namespace lti {
  // --------------------------------------------------
  // polynomRoots::parameters
  // --------------------------------------------------

  // default constructor
  polynomRoots::parameters::parameters()
    : functor::parameters() {
    
  }

  // copy constructor
  polynomRoots::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  polynomRoots::parameters::~parameters() {
  }

  // get type name
  const char* polynomRoots::parameters::getTypeName() const {
    return "polynomRoots::parameters";
  }

  // copy member

  polynomRoots::parameters&
    polynomRoots::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    

    return *this;
  }

  // alias for copy member
  polynomRoots::parameters&
    polynomRoots::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* polynomRoots::parameters::clone() const {
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
  bool polynomRoots::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool polynomRoots::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool polynomRoots::parameters::write(ioHandler& handler,
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
  bool polynomRoots::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool polynomRoots::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool polynomRoots::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // polynomRoots
  // --------------------------------------------------

  // default constructor
  polynomRoots::polynomRoots()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  polynomRoots::polynomRoots(const parameters& par)
    : functor() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  polynomRoots::polynomRoots(const polynomRoots& other)
    : functor() {
    copy(other);
  }

  // destructor
  polynomRoots::~polynomRoots() {
  }

  // returns the name of this type
  const char* polynomRoots::getTypeName() const {
    return "polynomRoots";
  }

  // copy member
  polynomRoots&
    polynomRoots::copy(const polynomRoots& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  polynomRoots&
    polynomRoots::operator=(const polynomRoots& other) {
    return (copy(other));
  }


  // clone member
  functor* polynomRoots::clone() const {
    return new polynomRoots(*this);
  }

  // return parameters
  const polynomRoots::parameters&
    polynomRoots::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  /*
   * Solves the quadratic equation \f$ ax^2 + bx + c = 0 \f$,
   * for real \e a, \e b and \e c coefficients.
   *
   * @param a coefficient for the x^2 term
   * @param b coefficient for the x  term
   * @param c constant term
   *
   * @param rex1 real part of the first solution
   * @param imx1 imaginary part of the first solution
   * @param rex2 real part of the second solution
   * @param imx2 imaginary part of the second solution
   *
   * @return the number of real solutions.
   */
  int
  polynomRoots::quadratic(const double& a, const double& b, const double& c,
                          double& rex1, double& imx1,
                          double& rex2, double& imx2) const {
    const double aa=(a+a);
    const double det=(b*b)-(2*aa*c);
    if (det>=0) {
      // only real solutions
      imx1=imx2=0.0;
      const double t=sqrt(det);
      rex1=(-b+t)/aa;
      rex2=(-b-t)/aa;
      return ( (det==0.0f) ? 1 : 2);
    } else {
      // only imaginary solutions
      const double t=sqrt(-det);
      rex1=rex2=-b/aa;
      imx1=t;
      imx2=-t;
    }
    return 0;
  }

  int 
  polynomRoots::quadratic(const double& a, const double& b, const double& c,
                          complex<double>& x1, complex<double>& x2) const {
    const complex<double> det=b*b-4*a*c;
    const complex<double> q = -0.5*(b+signum(b)*sqrt(det));
    x1=q/a;
    x2=c/q;
    
    return (det<0.0) ? 0 : (det==0.0) ? 1 : 2;
  }


    
  /*
   * Solves the quadratic equation \f$ x^2 + p*x + q = 0 \f$,
   * for real \e p and \e q coefficients.
   *
   * @param p coefficient for the x term
   * @param q constant term
   *
   * @param rex1 real part of the first solution
   * @param imx1 imaginary part of the first solution
   * @param rex2 real part of the second solution
   * @param imx2 imaginary part of the second solution
   *
   * @return the number of real solutions.
   */
  int polynomRoots::quadratic(const double& p, const double& q,
                              double& rex1, double& imx1,
                              double& rex2, double& imx2) const {
    const double hb = (p/2.0);
    const double det= hb*hb - q;
    if (det>=0) {
      // only real solutions
      imx1=imx2=0.0;
      const double t=sqrt(det);
      rex1=-hb+t;
      rex2=-hb-t;
      return ( (det==0.0f) ? 1 : 2);
    } else {
      // only imaginary solution
      const double t=sqrt(-det);
      rex1=rex2=-hb;
      imx1=t;
      imx2=-t;
    }
    return 0;
  }

  int 
  polynomRoots::quadratic(const double& p, const double& q,
                          complex<double>& x1, complex<double>& x2) const {
    
    const double det=p*p-4*q;
    const complex<double> qq = -0.5*(p+signum(p)*sqrt(det));
    x1=qq;
    x2=q/qq;
    
    return (det<0.0) ? 0 : (det==0.0) ? 1 : 2;
  }

  /*
   * Solves the equation \f$ x^3 + a*x^2 + b*x + c= 0\f$, 
   * for real \c a, \c b, \c c and \c d coefficients.
   *
   * @param p coefficient for the x^2  term
   * @param q coefficient for the x term
   * @param r constant term
   *
   * @param rex1 real part of the first solution
   * @param imx1 imaginary part of the first solution
   * @param rex2 real part of the second solution
   * @param imx2 imaginary part of the second solution
   * @param rex3 real part of the third solution
   * @param imx3 imaginary part of the third solution
   *
   * @return the number of real solutions.
   */
  int polynomRoots::cubic(const double& a, const double& b, const double& c,
                          double& rex1, double& imx1,
                          double& rex2, double& imx2,
                          double& rex3, double& imx3) const {
    complex<double> x1,x2,x3;
    int res = cubic(a,b,c,x1,x2,x3);

    rex1=x1.real();  imx1=x1.imag();
    rex2=x2.real();  imx2=x2.imag();
    rex3=x3.real();  imx3=x3.imag();

    return res;
  }

  int polynomRoots::cubic(const double& a, const double& b, const double& c,
                          complex<double>& x1,
                          complex<double>& x2,
                          complex<double>& x3) const {
    double tmp=(a*a);
    const double q((tmp-3.0*b)/9.0);
    const double r((2.0*tmp*a - 9.0*a*b + 27.0*c)/54.0);
    const double r2(r*r),q3(q*q*q);
    tmp = a/3.0;

    if (r2<q3) {
      // three real roots
      static const double pi23=2.0*Pi/3.0;
      double qsqrt(sqrt(q));
      const double theta  = acos(r/(q*qsqrt))/3.0;
      qsqrt *= -2.0;
      x1     = qsqrt*cos(theta) - tmp;
      x2     = qsqrt*cos(theta+pi23) - tmp;
      x3     = qsqrt*cos(theta-pi23) - tmp;
      return 3;
    } else {
      static const double srt32 = sqrt(3.0)/2.0;

      const double aa = -pow(r+signum(r)*sqrt(r2-q3),1.0/3.0);
      double bb(0.0);
      if (aa != 0.0) {
        bb=q/aa;
      }
      const double apb = aa+bb;
      const double amb = aa-bb;
     
      x1 =      apb - tmp;
      x2 = complex<double>(-0.5*apb - tmp,srt32*amb);
      x3 = complex<double>(-0.5*apb - tmp,-srt32*amb);

      return 1;
    }
  }

  /*
   * General apply method.  The described polygon is
   * p[0]+p[1]*x + p[2]*x^2 + .. + p[n-1]*x^(n-1), with \a n the size of \a p
   *
   * @param p coefficients for the polynom
   * @param re real parts of the solutions
   * @param im imaginary parts of the solutions
   *
   * \warning at this point, only solutions for polynoms of first, second
   * and third degree have being implemented
   *
   * @return the number of real solutions
   */
  int polynomRoots::apply(const vector<double>& p,
                          vector<double>& re,
                          vector<double>& im) const {
    const int s = p.lastIdx();
    re.resize(s,double(),false,false);
    im.resize(s,double(),false,false);

    switch (s) {
      case 1:
        // linear
        re.at(0) = -p.at(0)/p.at(1);
        im.at(0) = 0.0;
        return 1;
      case 2:
        // quadratic
        return quadratic(p.at(2),p.at(1),p.at(0),
                         re.at(0),im.at(0),
                         re.at(1),im.at(1));
      case 3:
        // cubic
        if (p.at(3) == 0.0) {
          return quadratic(p.at(2),p.at(1),p.at(0),
                           re.at(0),im.at(0),
                           re.at(1),im.at(1));          
        } else {
          return cubic(p.at(2)/p.at(3),p.at(1)/p.at(3),p.at(0)/p.at(3),
                       re.at(0),im.at(0),
                       re.at(1),im.at(1),
                       re.at(2),im.at(2));
        }
        break;
      default:
        vector< complex<double> > roots;
        int res = findRoots(p,roots);
        int i;
        for (i=0;i<roots.size();++i) {
          re.at(i)=roots.at(i).real();
          im.at(i)=roots.at(i).imag();
        }
        return res;
    }

    return -1;
  }

  /*
   * General apply method.  The described polygon is
   * p[0]+p[1]*x + p[2]*x^2 + .. + p[n-1]*x^(n-1), with \a n the size of \a p
   *
   * @param p coefficients for the polynom
   * @param re real parts of the solutions
   * @param im imaginary parts of the solutions
   *
   * \warning at this point, only solutions for polynoms of first, second
   * and third degree have being implemented
   *
   * @return the number of real solutions
   */
  int polynomRoots::apply(const vector<double>& p,
                          vector<complex<double> >& roots) const {

    const int s = p.lastIdx();
    roots.resize(s,complex<double>(),false,false);

    if (p.size() < 2) {
      setStatusString("too few coefficients");
      return -1;
    }

    switch (s) {
      case 1:
        // linear
        roots.at(0) = complex<double>(-p.at(0)/p.at(1));
        return 1;
      case 2:
        // quadratic
        return quadratic(p.at(2),p.at(1),p.at(0),
                         roots.at(0),roots.at(1));
      case 3:
        // cubic
        if (p.at(3)==0.0) {
          return quadratic(p.at(2),p.at(1),p.at(0),
                           roots.at(0),roots.at(1));
        } else {
          return cubic(p.at(2)/p.at(3),p.at(1)/p.at(3),p.at(0)/p.at(3),
                       roots.at(0),roots.at(1),roots.at(2));
        }
        break;
      default:
        return findRoots(p,roots);
    }

    return -1;
  }

  int polynomRoots::laguerre(const vector< complex<double> >& p,
                             const int degree,
                             complex<double>& root) const {
    
    // estimated fractional round-off
    static const double epsilon = std::numeric_limits<double>::epsilon();

    enum {
      // number of different fractional values to break (rare) limit cycles
      mr = 8,
      // number of steps to break (rare) limit cycles
      mt = 10,
      // maximal number of iterations
      max_iterations = (mt*mr)
    };
    
    double absx,absp,absm,err;
    complex<double> dx,x1,b,d,f,g,h,sq,gp,gm,g2;

    // fractions used to break a limit cycle
    static const double frac[]={0.0,0.5,0.25,0.75,0.125,0.375,0.625,0.875,1.0};

    const int mm1 = degree-1;
    int it,j;
    
    // do maximal max_iterations
    for (it=1;it<=max_iterations;++it) {
      b=p.at(degree);
      err = abs(b);
      d=f=complex<double>(0.0,0.0);
      absx=abs(root);

      // computation of the polynomial and its first two derivatives
      for (j=mm1;j>=0;--j) {
        f   = root*f + d;        // second derivative
        d   = root*d + b;        // first derivative
        b   = root*b + p.at(j);  // polynom value
        err = abs(b) + absx*err;
      }
      
      // estimate of roundoff in polynomial evaluation
      err *= epsilon;

      if (abs(b) <= err) {
        // root reached
        return it;
      }

      g  = d/b;
      g2 = g*g;
      h  = g2-2.0*f/b;
      sq = sqrt(static_cast<double>(mm1)*(static_cast<double>(degree)*h - g2));
      gp = g+sq;
      gm = g-sq;
      absp = abs(gp);
      absm = abs(gm);
      if (absp < absm) {
        gp=gm;
      }
      dx = ( (max(absp,absm) > 0.0) ? static_cast<double>(degree)/gp
             : (1.0+absx)*complex<double>(cos(double(it)),sin(double(it))) );
      x1 = root - dx;
      
      if (root == x1) {
        // converged
        return it;
      }

      if ((it % mt) != 0) {
        root=x1;
      } else {
        root=root-frac[it/mt]*dx;
      }
    }
    
    setStatusString("too many iterations in Laguerre's root search");
    return it;
  }

  int polynomRoots::findRoots(const vector<double>& p,
                              vector<complex<double> >& roots,
                              const bool polish) const {

    // estimated fractional round-off
    static const double epsilon = std::numeric_limits<double>::epsilon();

    // degree of polynom
    const int degree = p.lastIdx();
    roots.resize(degree);
    
    // coefficients for successive deflation
    vector< complex<double> > ad;
    ad.castFrom(p);

    int i,its,j,jj;
    complex<double> x,b,c;

    // loop over each root to be found
    for (j=degree;j>=1;--j) {
      // start at 0,0 to favor convergence to smallest remaining root and
      // find it
      x=complex<double>(0.0,0.0);
      laguerre(ad,j,x);

      if (abs(x.imag()) <= 2.0*epsilon*abs(x.real())) {
        x=x.real();
      }
      
      roots.at(j-1)=x;

      // forward deflation
      b=ad.at(j);
      for (jj=j-1;jj>=0;--jj) {
        c=ad.at(jj);
        ad.at(jj)=b;
        b=(x*b)+c;
      }
    }

    if (polish) {
      ad.castFrom(p);
      for (j=0;j<degree;++j) {
        laguerre(ad,degree,roots[j]);
      }
    }

    its=(roots.at(0).imag() == 0.0) ? 1 : 0;
    // sort roots by their real parts by straight insertion
    for (j=1;j<degree;++j) {
      x=roots.at(j);
      if (x.imag() == 0.0) {
        its++;
      }
      for (i=j-1;i>=0;--i) {
        if (roots.at(i).real() <= x.real())
          break;
        roots.at(i+1)=roots.at(i);
      }
      roots.at(i+1)=x;
    }

    return its;
  }

}
