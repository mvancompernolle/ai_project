// ==============================================================
//
//  Copyright (C) 2002-2004 Alex Vinokur.
//
//  ------------------------------------------------------------
//  This file is part of C/C++ Program Perfometer.
//
//  C/C++ Program Perfometer is free software; 
//  you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License,
//  or (at your option) any later version.
//
//  C/C++ Program Perfometer is distributed in the hope 
//  that it will be useful, but WITHOUT ANY WARRANTY; 
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with C/C++ Program Perfometer; 
//  if not, write to the Free Software Foundation, Inc., 
//  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ------------------------------------------------------------
// 
//  http://up.to/alexv
//
// ==============================================================


// ##############################################################
//
//  SOFTWARE : C/C++ Program Perfometer
//  FILE     : t_trd1.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============



// #########################################
// Technical Report on C++ Performance
// (ISO/IEC PDTR 18015; Date: 2003-08-11; WG21 N1487=03-0070) :
// http://std.dkuug.dk/JTC1/SC22/WG21/docs/PDTR18015.pdf
// http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/papers/2003/n1487.pdf
// 
// Appendix D: Timing Code
// 
// <QUOTE>
// D.1 Measuring the Overhead of Class Operations (Page 151)
// This is the sample program discussed in 2.3.2 and following.
// 
// Simple/naive measurements to give a rough idea of the relative
// cost of facilities related to OOP.
// This could be fooled/foiled by clever optimizers and by cache effects.
// Run at least three times to ensure that results are repeatable.
// Tests:
//    * virtual function
//    * global function called indirectly
//    * nonvirtual member function
//    * global function
//    * inline member function
//    * macro
//    * 1st branch of MI
//    * 2nd branch of MI
//    * call through virtual base
//    * call of virtual base function
//    * dynamic cast
//    * two-level dynamic cast
//    * typeid()
//    * call through pointer to member
//    * call-by-reference
//    * call-by-value
//    * pass as pointer to function
//    * pass as function object
// not yet:
//    * co-variant return
// 
// The cost of the loop is not measurable at this precision:
// see inline tests
// </QUOTE>
// 
// #########################################



// #########################################
class X 
{
  private:
    int        x;
    static int st;

  public:
    virtual void foo_f(int a);
    void         foo_g(int a);
    static void  foo_h(int a);
    void         foo_k(int i) { x+=i; } // inline
};

struct S { int x; };

int		glob = 0;
extern void	foo_f(S* p, int a);
extern void	foo_g(S* p, int a);
extern void	foo_h(int a);
typedef void	(*PF)(S* p, int a);

PF foo_p[10]	= { foo_g , foo_f };

// inline void k(S* p, i) { p->x+=i; }
#define K(p,i) ((p)->x+=(i))

struct T 
{
  const char*	s;
  double	t;
  T(const char* ss, double tt) : s(ss), t(tt) {}
  T() : s(0), t(0) {}
};

struct A 
{
  int		x;
  virtual void	foo_f(int) = 0;
  void		foo_g(int);
};

struct B 
{
  int		xx;
  virtual void	foo_ff(int) = 0;
  void		foo_gg(int);
};

struct C : A, B 
{
  void foo_f(int);
  void foo_ff(int);
};

struct CC : A, B 
{
  void foo_f(int);
  void foo_ff(int);
};

void A::foo_g(int i)   { x += i; }
void B::foo_gg(int i)  { xx += i; }
void C::foo_f(int i)   { x += i; }
void C::foo_ff(int i)  { xx += i; }
void CC::foo_f(int i)  { x += i; }
void CC::foo_ff(int i) { xx += i; }

template<class T, class T2> inline T* foo_cast(T*, T2* q)
{
  glob++;
  return dynamic_cast<T*>(q);
}

struct C2 : virtual A {};	// note: virtual base

struct C3 : virtual A {};

struct D : C2, C3 { void foo_f(int); };	// note: virtual base

void D::foo_f(int i) { x+=i; }

struct P 
{
  int x;
  int y;
};

void foo_by_ref(P& a) { a.x++; a.y++; }

void foo_by_val(P a) { a.x++; a.y++; }

template<class F, class V> inline void foo_oper(F f, V val) { f(val); }

struct FO  { void operator () (int i) { glob += i; } };

template<class T> inline T* foo_ti(T* p)
{
  if (typeid(p) == typeid(int*)) p++;
  return p;
}


int X::st = 0;
void X::foo_f(int a) { x += a; }
void X::foo_g(int a) { x += a; }
void X::foo_h(int a) { st += a; }
void foo_f(S* p, int a) { p->x += a; }
void foo_g(S* p, int a) { p->x += a; }
void foo_h(int a) { glob += a; }
// #########################################



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void tech_report_d1_action (void)
{
const string htext__virtual_px        ("virtual px->f(1)             ");
const string htext__ptr_to_fct_val    ("ptr-to-fct p[1](ps,1)        ");
const string htext__virtual_x         ("virtual x.f(1)               ");
const string htext__ptr_to_fct_ptr    ("ptr-to-fct p[1](&s,1)        ");

const string htext__member_px         ("member px->g(1)              ");
const string htext__global_g_val      ("global g(ps,1)               ");
const string htext__member_x          ("member x.g(1)                ");
const string htext__global_g_ptr      ("global g(&s,1)               ");
const string htext__static_X_h        ("static X::h(1)               ");
const string htext__global_h          ("global h(1)                  ");
const string htext__inline_px         ("inline px->k(1)              ");
const string htext__macro_K_val       ("macro K(ps,1)                ");
const string htext__inline_x          ("inline x.k(1)                ");
const string htext__macro_K_ptr       ("macro K(&s,1)                ");

const string htext__base1_member_pc   ("base1 member pc->g(i)        ");
const string htext__base2_member_pc   ("base2 member pc->gg(i)       ");
const string htext__base1_virtual_pa  ("base1 virtual pa->f(i)       ");
const string htext__base2_virtual_pb  ("base2 virtual pb->ff(i)      ");
const string htext__base1_down_cast   ("base1 down-cast cast(pa,pc)  ");
const string htext__base2_down_cast   ("base2 down-cast cast(pb,pc)  ");
const string htext__base1_up_cast     ("base1 up-cast cast(pc,pa)    ");
const string htext__base2_up_cast     ("base2 up-cast cast(pc,pb)    ");
const string htext__base2_cross_cast  ("base2 cross-cast cast(pb,pa) ");

const string htext__base1_down_cast2  ("base1 down-cast2 cast(pa,pcc)");
const string htext__base2_down_cast2  ("base2 down-cast cast(pb,pcc) ");
const string htext__base1_up_cast2    ("base1 up-cast cast(pcc,pa)   ");
const string htext__base2_up_cast2    ("base2 up-cast2 cast(pcc,pb)  ");
const string htext__base2_cross_cast2 ("base2 cross-cast2 cast(pa,pb)");
const string htext__base1_cross_cast2 ("base1 cross-cast2 cast(pb,pa)");

const string htext__vbase_member_pd   ("vbase member pd->gg(i)       ");
const string htext__vbase_virtual_pa  ("vbase virtual pa->f(i)       ");
const string htext__vbase_down_cast   ("vbase down-cast cast(pa,pd)  ");
const string htext__vbase_up_cast     ("vbase up-cast cast(pd,pa)    ");
const string htext__vbase_typeid_pa   ("vbase typeid(pa)             ");
const string htext__vbase_typeid_pd   ("vbase typeid(pd)             ");
const string htext__pmf_virtual       ("pmf virtual (pa->*pmf)(i)    ");
const string htext__pmf               ("pmf (pa->*pmf)(i)            ");
const string htext__call_by_ref       ("call by_ref(pp)              ");
const string htext__call_by_val       ("call by_val(pp)              ");
const string htext__call_ptr_to_fct   ("call ptr-to-fct oper(h,glob) ");
const string htext__call_fct_obj      ("call fct-obj oper(fct,glob)  ");


X* px = new X;
X  x;

S* ps = new S;
S  s;

  // -------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext__virtual_px, "No") 
    {
      px->foo_f(1);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__ptr_to_fct_val, "No") 
    {
      foo_p[1](ps, 1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__virtual_x, "No") 
    {
      x.foo_f(1);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__ptr_to_fct_ptr, "No") 
    {
      foo_p[1](&s, 1);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__member_px, "No") 
    {
      px->foo_g(1);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__global_g_val, "No") 
    {
      foo_g(ps, 1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__member_x, "No") 
    {
      x.foo_g(1);
    }
  }




  {
    TURN_ON_DEFAULT_TIMER (htext__global_g_ptr, "No") 
    {
      foo_g(&s, 1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__static_X_h, "No") 
    {
      X::foo_h(1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__global_h, "No") 
    {
      foo_h(1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__inline_px, "No") 
    {
      px->foo_k(1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__macro_K_val, "No") 
    {
      K(ps, 1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__inline_x, "No") 
    {
      x.foo_k(1);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__macro_K_ptr, "No") 
    {
      K(&s, 1);
    }
  }


C* pc = new C;
A* pa = pc;
B* pb = pc;
  {
    TURN_ON_DEFAULT_TIMER (htext__base1_member_pc, "No") 
    {
      pc->foo_g(repetition_no);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_member_pc, "No") 
    {
      pc->foo_gg(repetition_no);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base1_virtual_pa, "No") 
    {
      pa->foo_f(repetition_no);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_virtual_pb, "No") 
    {
      pb->foo_ff(repetition_no);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base1_down_cast, "No") 
    {
      foo_cast(pa, pc);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_down_cast, "No") 
    {
      foo_cast(pb, pc);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base1_up_cast, "No") 
    {
      foo_cast(pc, pa);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_up_cast, "No") 
    {
      foo_cast(pc, pb);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_cross_cast, "No") 
    {
      foo_cast(pb, pa);
    }
  }


CC* pcc = new CC;
pa = pcc;
pb = pcc;
  {
    TURN_ON_DEFAULT_TIMER (htext__base1_down_cast2, "No") 
    {
      foo_cast(pa, pcc);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_down_cast2, "No") 
    {
      foo_cast(pb, pcc);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base1_up_cast2, "No") 
    {
      foo_cast(pcc, pa);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_up_cast2, "No") 
    {
      foo_cast(pcc, pb);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base2_cross_cast2, "No") 
    {
      foo_cast(pb, pa);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__base1_cross_cast2, "No") 
    {
      foo_cast(pa, pb);
    }
  }


D* pd = new D;
pa = pd;
  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_member_pd, "No") 
    {
      pd->foo_g(repetition_no);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_virtual_pa, "No") 
    {
      pa->foo_f(repetition_no);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_down_cast, "No") 
    {
      foo_cast(pa, pd);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_up_cast, "No") 
    {
      foo_cast(pd, pa);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_typeid_pa, "No") 
    {
      foo_ti(pa);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__vbase_typeid_pd, "No") 
    {
      foo_ti(pd);
    }
  }


void (A::* foo_pmf)(int) = &A::foo_f; // virtual
  {
    TURN_ON_DEFAULT_TIMER (htext__pmf_virtual, "No") 
    {
      (pa->*foo_pmf)(repetition_no);
    }
  }


foo_pmf = &A::foo_g; // non virtual
  {
    TURN_ON_DEFAULT_TIMER (htext__pmf, "No") 
    {
      (pa->*foo_pmf)(repetition_no);
    }
  }


P pp;
  {
    TURN_ON_DEFAULT_TIMER (htext__call_by_ref, "No") 
    {
      foo_by_ref(pp);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__call_by_val, "No") 
    {
      foo_by_val(pp);
    }
  }


FO fct;
  {
    TURN_ON_DEFAULT_TIMER (htext__call_ptr_to_fct, "No") 
    {
      foo_oper(foo_h, glob);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__call_fct_obj, "No") 
    {
      foo_oper(fct, glob);
    }
  }


} // tech_report_d1_action



///////////////
// End-Of-File
///////////////

