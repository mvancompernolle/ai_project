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
//  FILE     : t_algo.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============


// #########################################
// -------------------------------------------
#define SIZE0	10
#define SIZE1	100
#define SIZE2	1000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------


// #########################################

// ====== DECLARATIONS etc =======
// ===============================
// -------------------------------
typedef int typeT;
class Foo;

#define VALUET_S  10	// VALUET_S is of typeT

// -------------------------------
typeT func1_ref (Foo& inst_i);
typeT func1_val (Foo inst_i);
typeT func2_val (Foo inst_i, typeT value_i);
typeT func3_val (Foo inst1_i, Foo inst2_i);


// ===============================
// === class Foo : DESCRIPTION ===
// ===============================
// -------------------------------
class Foo
{
  private :
    typeT value_;

  public :
    explicit Foo () {}
    explicit Foo (typeT value_i) : value_ (value_i) {}
    ~Foo () {}

    typeT meth1 ();
    typeT meth2 (typeT value_i);
    typeT meth3_val (Foo inst_i);
    typeT meth3_ptr (Foo* ptr_i);

};

// -------------------------------
typeT Foo::meth1 ()         { return typeT(); }
typeT Foo::meth2 (typeT)    { return typeT(); }
typeT Foo::meth3_val (Foo)  { return typeT(); }
typeT Foo::meth3_ptr (Foo*) { return typeT(); }


// ============================
// ===== Action FUNCTIONS =====
// ============================
typeT func1_ref (Foo&)       { return typeT(); }
typeT func1_val (Foo)        { return typeT(); }
typeT func2_val (Foo, typeT) { return typeT(); }
typeT func3_val (Foo, Foo)   { return typeT(); }



// ============================
// ==== Action FuncObjects ====
// ============================
// ---------------------------
class FuncObj
{
  private :
    Foo  inst_;

  public :
    FuncObj ();
    FuncObj (const Foo& inst_i);
    FuncObj (const Foo* const ptr_i);
    ~FuncObj () {}

   typeT operator() (Foo& inst_i);
   typeT operator() (Foo* const ptr_i);

};


// -----------
FuncObj::FuncObj ()                       : inst_ (Foo(VALUET_S)) { }
FuncObj::FuncObj (const Foo& inst_i)      : inst_ (inst_i) {}
FuncObj::FuncObj (const Foo* const ptr_i) : inst_ (*ptr_i) {}

typeT FuncObj::operator() (Foo&)       { return typeT(); }
typeT FuncObj::operator() (Foo* const) { return typeT(); }





// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void algo_action1 (void)
{
SET_SIZES;

const string htext__foreach_data_func_ref            ("for_each;  data, function, ref to arg ");
const string htext__foreach_data_func_val            ("for_each;  data, function, arg        ");
const string htext__foreach_data_func_ref__ptr_fun   ("for_each;  data, function, ptr_fun    ");

const string htext__foreach_data_func_bind2_arg      ("for_each;  data, function, bind2, arg ");
const string htext__foreach_data_func_bind2_ins      ("for_each;  data, function, bind2, inst");
const string htext__foreach_data_func_bind1_ins      ("for_each;  data, function, bind1, inst");

const string htext__foreach_data_meth__mem_fun_ref   ("for_each;  data, function, mem_fun_ref");
const string htext__foreach_ptrs_meth__mem_fun       ("for_each;  ptr,  function, mem_fun    ");

const string htext__foreach_ptrs_func_bind2_arg      ("for_each;  ptr,  function, bind2, arg ");
const string htext__foreach_ptrs_func_bind2_ins      ("for_each;  ptr,  function, bind2, inst");
const string htext__foreach_ptrs_func_bind1_ins      ("for_each;  ptr,  function, bind1, inst");

const string htext__foreach_data_fobj                ("for_each;  data, functor              ");
const string htext__foreach_data_fobj_ins            ("for_each;  data, functor, instance    ");
const string htext__foreach_ptrs_fobj                ("for_each;  ptr,  functor              ");
const string htext__foreach_ptrs_fobj_ins            ("for_each;  ptr,  functor, instance    ");
const string htext__foreach_ptrs_fobj_ref            ("for_each;  ptr,  functor, ref to inst ");


const string htext__transform_data_func_ref          ("transform; data, function, ref to arg ");
const string htext__transform_data_func_val          ("transform; data, function, arg        ");
const string htext__transform_data_func_ref__ptr_fun ("transform; data, function, ptr_fun    ");

const string htext__transform_data_func_bind2_arg    ("transform; data, function, bind2, arg ");
const string htext__transform_data_func_bind2_ins    ("transform; data, function, bind2, inst");
const string htext__transform_data_func_bind1_ins    ("transform; data, function, bind1, inst");

const string htext__transform_data_meth__mem_fun_ref ("transform; data, function, mem_fun_ref");
const string htext__transform_ptrs_meth__mem_fun     ("transform; ptr,  function, mem_fun    ");

const string htext__transform_ptrs_func_bind2_arg    ("transform; ptr,  function, bind2, arg ");
const string htext__transform_ptrs_func_bind2_ins    ("transform; ptr,  function, bind2, inst");
const string htext__transform_ptrs_func_bind1_ins    ("transform; ptr,  function, bind1, inst");

const string htext__transform_data_fobj              ("transform; data, functor              ");
const string htext__transform_data_fobj_ins          ("transform; data, functor, instance    ");
const string htext__transform_ptrs_fobj              ("transform; ptr,  functor              ");
const string htext__transform_ptrs_fobj_ins          ("transform; ptr,  functor, instance    ");
const string htext__transform_ptrs_fobj_ref          ("transform; ptr,  functor, ref to inst ");


// ------------
vector<Foo>  data;
vector<Foo*> ptrs;

vector<typeT> out_typeT;

const typeT valueT (VALUET_S);

//const Foo instance (valueT);
Foo instance (valueT);

  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t	cur_size (v_sizes[i]);

    
    for (size_t i = 0; i < cur_size; i++)
    {
      data.push_back(instance);
      ptrs.push_back(new Foo(instance));
    }

    // ---------------
    // --- for_each
    // ---------------

    // --- func1_x ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_ref, cur_size) 
      {
        for_each (data.begin(), data.end(), func1_ref);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_val, cur_size) 
      {
        for_each (data.begin(), data.end(), func1_val);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_ref__ptr_fun, cur_size) 
      {
        for_each (data.begin(), data.end(), ptr_fun (func1_ref));
      }
    }


    // --- func2_val ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_bind2_arg, cur_size) 
      {
        for_each (data.begin(), data.end(), bind2nd(ptr_fun(func2_val), valueT));
      }
    }


    // --- func3_val ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_bind2_ins, cur_size) 
      {
        for_each (data.begin(), data.end(), bind2nd (ptr_fun (func3_val), instance));
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_func_bind1_ins, cur_size) 
      {
        for_each (data.begin(), data.end(), bind1st (ptr_fun (func3_val), instance));
      }
    }


    // --- meth1 AND mem_fun_ref ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_meth__mem_fun_ref, cur_size) 
      {
        for_each (data.begin(), data.end(), mem_fun_ref (&Foo::meth1));
      }
    }



    // --- meth2 AND mem_fun_ref ---
    // None

    // --- meth3_val AND mem_fun_ref ---
    // None


    // --- meth1 AND mem_fun ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_meth__mem_fun_ref, cur_size) 
      {
        for_each (ptrs.begin(), ptrs.end(), mem_fun (&Foo::meth1));
      }
    }


    // --- meth2 AND mem_fun ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_func_bind2_arg, cur_size) 
      {
        for_each (ptrs.begin(), ptrs.end(), bind2nd(mem_fun(&Foo::meth2), valueT));
      }
    }


    // --- func_x_mem AND mem_fun ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_func_bind1_ins, cur_size) 
      {
        for_each (ptrs.begin(), ptrs.end(), bind1st(mem_fun(&Foo::meth3_ptr), &instance));
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_func_bind2_ins, cur_size) 
      {
        for_each (ptrs.begin(), ptrs.end(), bind2nd(mem_fun(&Foo::meth3_val), instance));
      }
    }


    // --- FuncObj ---
    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_fobj, cur_size) 
      {
        for_each (data.begin(),  data.end(),  FuncObj());
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_data_fobj_ins, cur_size) 
      {
        for_each (data.begin(),  data.end(),  FuncObj (instance));
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_fobj, cur_size) 
      {
        for_each (ptrs.begin(),  ptrs.end(),  FuncObj());
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_fobj_ins, cur_size) 
      {
        for_each (ptrs.begin(),  ptrs.end(),  FuncObj (instance));
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__foreach_ptrs_fobj_ref, cur_size) 
      {
        for_each (ptrs.begin(),  ptrs.end(),  FuncObj (&instance));
      }
    }


    // ---------------
    // --- transform
    // ---------------

    // --- func1_x ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_ref, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), func1_ref);
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_val, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), func1_val);
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_ref__ptr_fun, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), ptr_fun (func1_ref));
      }
    }


    // --- func2_val ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_bind2_arg, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), bind2nd(ptr_fun(func2_val), valueT));
      }
    }


    // --- func3_val ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_bind2_ins, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), bind2nd (ptr_fun (func3_val), instance));
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_func_bind1_ins, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), bind1st (ptr_fun (func3_val), instance));
      }
    }


    // --- meth1 AND mem_fun_ref ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_meth__mem_fun_ref, cur_size) 
      {
        transform (data.begin(), data.end(), out_typeT.begin(), mem_fun_ref (&Foo::meth1));
      }
    }



    // --- meth2 AND mem_fun_ref ---
    // None

    // --- meth3_val AND mem_fun_ref ---
    // None


    // --- meth1 AND mem_fun ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_meth__mem_fun_ref, cur_size) 
      {
        transform (ptrs.begin(), ptrs.end(), out_typeT.begin(), mem_fun (&Foo::meth1));
      }
    }


    // --- meth2 AND mem_fun ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_func_bind2_arg, cur_size) 
      {
        transform (ptrs.begin(), ptrs.end(), out_typeT.begin(), bind2nd(mem_fun(&Foo::meth2), valueT));
      }
    }


    // --- func_x_mem AND mem_fun ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_func_bind1_ins, cur_size) 
      {
        transform (ptrs.begin(), ptrs.end(), out_typeT.begin(), bind1st(mem_fun(&Foo::meth3_ptr), &instance));
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_func_bind2_ins, cur_size) 
      {
        transform (ptrs.begin(), ptrs.end(), out_typeT.begin(), bind2nd(mem_fun(&Foo::meth3_val), instance));
      }
    }


    // --- FuncObj ---
    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_fobj, cur_size) 
      {
        transform (data.begin(),  data.end(), out_typeT.begin(),  FuncObj());
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_data_fobj_ins, cur_size) 
      {
        transform (data.begin(),  data.end(), out_typeT.begin(),  FuncObj (instance));
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_fobj, cur_size) 
      {
        transform (ptrs.begin(),  ptrs.end(), out_typeT.begin(),  FuncObj());
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_fobj_ins, cur_size) 
      {
        transform (ptrs.begin(),  ptrs.end(), out_typeT.begin(),  FuncObj (instance));
      }
    }

    {
      out_typeT.clear();
      assert (out_typeT.empty());
      out_typeT.resize(data.size());
      TURN_ON_DEFAULT_TIMER (htext__transform_ptrs_fobj_ref, cur_size) 
      {
        transform (ptrs.begin(),  ptrs.end(), out_typeT.begin(),  FuncObj (&instance));
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // algo_action1



///////////////
// End-Of-File
///////////////

