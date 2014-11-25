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
//  FILE     : t_trd3.cpp
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
// D.3 The Stepanov Abstraction Penalty Benchmark
// This is the sample code discussed in 2.3.1.
// 
// KAI's version of Stepanov Benchmark -- Version 1.2
// Version 1.2 -- removed some special code for GNU systems that
//                GNU complained about without -O
// 
// To verify how efficiently C++ (and in particular STL) is compiled by
// the present day compilers, I composed a little benchmark. It outputs
// 13 numbers. In the ideal world these numbers should be the same. In
// the real world, however, ...
// 
// The final number printed by the benchmark is a geometric mean of the
// performance degradation factors of individual tests. It claims to
// represent the factor by which you will be punished by your
// compiler if you attempt to use C++ data abstraction features. I call
// this number "Abstraction Penalty."
// 
// As with any benchmark it is hard to prove such a claim; some people
// told me that it does not represent typical C++ usage. It is, however,
// a noteworthy fact that majority of the people who so object are
// responsible for C++ compilers with disproportionatly large Abstraction
// Penalty.
// 
// The structure of the benchmark is really quite simple. It adds 2000
// doubles in an array 25000 times. It does it in 13 different ways that
// introduce more and more abstract ways of doing it:
// 
//   0      - uses simple Fortran-like for loop.
//   1 - 12   use STL style accumulate template function with plus function object.
//   1, 3, 5, 7 ,9, 11  use doubles.
//   2, 4, 6, 8, 10, 12 use Double - double wrapped in a class.
//   1, 2   - use regular pointers.
//   3, 4   - use pointers wrapped in a class.
//   5, 6   - use pointers wrapped in a reverse-iterator adaptor.
//   7, 8   - use wrapped pointers wrapped in a reverse-iterator adaptor.
//   9, 10  - use pointers wrapped in a reverse-iterator adaptor wrapped in a
//            reverse-iterator adaptor.
//   11, 12 - use wrapped pointers wrapped in a reverse-iterator adaptor wrapped in a
//            reverse-iterator adaptor.
// 
// All the operators on Double and different pointer-like classes are
// declared inline. The only thing that is really measured is the penalty for data
// abstraction. While templates are used, they do not cause any performance degradation.
// They are used only to simplify the code.
// 
// Since many of you are interested in the C++ performance issues, I
// decided to post the benchmark here. I would appreciate if you run it
// and (if possible) send me the results indicating what you have
// compiled it with (CPU, clock rate, compiler, optimization level). It
// is self contained and written so that it could be compiled even with
// those compilers that at present cannot compile STL at all.
// 
// It takes a fairly long time to run - on a really slow machine it might take a full
// hour. (For those of you who want to run it faster - give it a command line argument
// that specifies the number of iterations. The default is 25000, 
// but it gives an accurate predictions even with 500 or a thousand.)
// 
//    Alex Stepanov
// </QUOTE>
// 
// #########################################


// ###############################################
//
// Note. The original tests were slightly changed
//
//    Alex Vinokur
//
// ###############################################




// #########################################
template <class T>
inline int operator!=(const T& x, const T& y) 
{
  return !(x == y);
}


struct Double 
{
  double value;
  Double() {}
  Double(const double& x) : value(x) {}
  operator double() { return value; }
};


inline Double operator+(const Double& x, const Double& y) 
{
  return Double(x.value + y.value);
}


struct double_pointer 
{
  double* current;
  double_pointer() {}
  double_pointer(double* x) : current(x) {}
  double& operator*() const { return *current; }
  double_pointer& operator++() 
  {
    ++current;
    return *this;
  }
  double_pointer operator++(int) 
  {
    double_pointer tmp = *this;
    ++*this;
    return tmp;
  }
  double_pointer& operator--() 
  {
    --current;
    return *this;
  }
  double_pointer operator--(int) 
  {
    double_pointer tmp = *this;
    --*this;
    return tmp;
  }
};


inline int operator==(const double_pointer& x, const double_pointer& y) 
{
  return x.current == y.current;
}

struct Double_pointer 
{
  Double* current;
  Double_pointer() {}
  Double_pointer(Double* x) : current(x) {}
  Double& operator*() const { return *current; }
  Double_pointer& operator++() 
  {
    ++current;
    return *this;
  }
  Double_pointer operator++(int) 
  {
    Double_pointer tmp = *this;
    ++*this;
    return tmp;
  }
  Double_pointer& operator--() 
  {
    --current;
    return *this;
  }
  Double_pointer operator--(int) 
  {
    Double_pointer tmp = *this;
    --*this;
    return tmp;
  }
};


inline int operator==(const Double_pointer& x, const Double_pointer& y) 
{
  return x.current == y.current;
}

template <class Random_Access_Iterator, class T>
struct reverse__iterator 
{
  Random_Access_Iterator current;
  reverse__iterator(Random_Access_Iterator x) : current(x) {}
  T& operator*() const 
  {
    Random_Access_Iterator tmp = current;
    return *(--tmp);
  }
  reverse__iterator<Random_Access_Iterator, T>& operator++() 
  {
    --current;
    return *this;
  }
  reverse__iterator<Random_Access_Iterator, T> operator++(int) 
  {
    reverse__iterator<Random_Access_Iterator, T> tmp = *this;
    ++*this;
    return tmp;
  }
  reverse__iterator<Random_Access_Iterator, T>& operator--() 
  {
    ++current;
    return *this;
  }
  reverse__iterator<Random_Access_Iterator, T> operator--(int) 
  {
    reverse__iterator<Random_Access_Iterator, T> tmp = *this;
    --*this;
    return tmp;
  }
};


template <class Random_Access_Iterator, class T>
inline int operator==(
	const reverse__iterator<Random_Access_Iterator, T>& x,
        const reverse__iterator<Random_Access_Iterator, T>& y
        )
{
  return x.current == y.current;
}

struct Plus_Instance
{
  double operator()(const double& x, const double& y) 
  {
    return x + y;
  }
  Double operator()(const Double& x, const Double& y) 
  {
    return x + y;
  }
};
Plus_Instance plus_instance;


template <class Iterator, class Number>
Number accumulate_action(Iterator first, Iterator last, Number result) 
{
  while (first != last) result = plus_instance(result, *first++);
  return result;
}


// -----------------------------------------
template <class Iterator, class T>
void fill_action(Iterator first, Iterator last, T value) 
{
  while (first != last) *first++ = value;
}


// -----------------------------------------
#define CHECK(actual_result) \
	{ \
	  if (actual_result != expected_result) \
	  { \
	    printf("[%s, %d] : test failed\n", __FILE__, __LINE__); \
	  } \
	}
	

// #########################################

#define SIZE0	100
#define SIZE1	1000
#define SIZE2	10000


#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void tech_report_d3_action (void)
{
SET_SIZES;

const string htext__test0_dpb_dpe        ("test0(dpb, dpe)      ");
const string htext__test_dpb_dpe_d       ("test(dpb, dpe, d)    ");
const string htext__test_Dpb_Dpe_D       ("test(Dpb, Dpe, D)    ");
const string htext__test_dPb_dPe_d       ("test(dPb, dPe, d)    ");
const string htext__test_DPb_DPe_D       ("test(DPb, DPe, D)    ");
const string htext__test_rdpb_rdpe_d     ("test(rdpb, rdpe, d)  ");
const string htext__test_rDpb_rDpe_D     ("test(rDpb, rDpe, D)  ");
const string htext__test_rdPb_rdPe_d     ("test(rdPb, rdPe, d)  ");
const string htext__test_rDPb_rDPe_D     ("test(rDPb, rDPe, D)  ");
const string htext__test_rrdpb_rrdpe_d   ("test(rrdpb, rrdpe, d)");
const string htext__test_rrDpb_rrDpe_D   ("test(rrDpb, rrDpe, D)");
const string htext__test_rrdPb_rrdPe_d   ("test(rrdPb, rrdPe, d)");
const string htext__test_rrDPb_rrDPe_D   ("test(rrDPb, rrDPe, D)");


const double init_value = 3.;
  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);


    // --------------------
    // double data[cur_size];
    // Double Data[cur_size];
    double* data = new double[cur_size];
    Double* Data = new Double[cur_size];

    const double expected_result = cur_size * init_value;

    // --------------------
    double d = 0.;
    Double D = 0.;
    
    typedef double* dp;
    dp dpb = data;
    dp dpe = data + cur_size;
    
    typedef Double* Dp;
    Dp Dpb = Data;
    Dp Dpe = Data + cur_size;
    
    typedef double_pointer dP;
    dP dPb(dpb);
    dP dPe(dpe);
    
    typedef Double_pointer DP;
    DP DPb(Dpb);
    DP DPe(Dpe);
    
    typedef reverse__iterator<dp, double> rdp;
    rdp rdpb(dpe);
    rdp rdpe(dpb);
    
    typedef reverse__iterator<Dp, Double> rDp;
    rDp rDpb(Dpe);
    rDp rDpe(Dpb);
    
    typedef reverse__iterator<dP, double> rdP;
    rdP rdPb(dPe);
    rdP rdPe(dPb);
    
    typedef reverse__iterator<DP, Double> rDP;
    rDP rDPb(DPe);
    rDP rDPe(DPb);
    
    typedef reverse__iterator<rdp, double> rrdp;
    rrdp rrdpb(rdpe);
    rrdp rrdpe(rdpb);
    
    typedef reverse__iterator<rDp, Double> rrDp;
    rrDp rrDpb(rDpe);
    rrDp rrDpe(rDpb);
    
    typedef reverse__iterator<rdP, double> rrdP;
    rrdP rrdPb(rdPe);
    rrdP rrdPe(rdPb);
    
    typedef reverse__iterator<rDP, Double> rrDP;
    rrDP rrDPb(rDPe);
    rrDP rrDPe(rDPb);
    

    // -------------------------------
    fill_action(dpb, dpe, double(init_value));
    fill_action(Dpb, Dpe, Double(init_value));

    // -------------------------------
    {
      double* first = dpb;
      double* last  = dpe;
      TURN_ON_DEFAULT_TIMER (htext__test0_dpb_dpe, cur_size) 
      {
        double result = 0;
        for (int n = 0; n < last - first; ++n) result += first[n];
        CHECK(result);
      }
    }
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_dpb_dpe_d, cur_size) 
      {
        CHECK(double(accumulate_action(dpb, dpe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_Dpb_Dpe_D, cur_size) 
      {
        CHECK(double(accumulate_action(Dpb, Dpe, D)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_dPb_dPe_d, cur_size) 
      {
        CHECK(double(accumulate_action(dPb, dPe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_DPb_DPe_D, cur_size) 
      {
        CHECK(double(accumulate_action(DPb, DPe, D)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rdpb_rdpe_d, cur_size) 
      {
        CHECK(double(accumulate_action(rdpb, rdpe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rDpb_rDpe_D, cur_size) 
      {
        CHECK(double(accumulate_action(rDpb, rDpe, D)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rdPb_rdPe_d, cur_size) 
      {
        CHECK(double(accumulate_action(rdPb, rdPe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rDPb_rDPe_D, cur_size) 
      {
        CHECK(double(accumulate_action(rDPb, rDPe, D)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rrdpb_rrdpe_d, cur_size) 
      {
        CHECK(double(accumulate_action(rrdpb, rrdpe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rrDpb_rrDpe_D, cur_size) 
      {
        CHECK(double(accumulate_action(rrDpb,rrDpe, D)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rrdPb_rrdPe_d, cur_size) 
      {
        CHECK(double(accumulate_action(rrdPb, rrdPe, d)));
      }
    }
  
  
    {
      TURN_ON_DEFAULT_TIMER (htext__test_rrDPb_rrDPe_D, cur_size) 
      {
        CHECK(double(accumulate_action(rrDPb, rrDPe, D)));
      }
    }
  
  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // tech_report_d3_action


///////////////
// End-Of-File
///////////////

