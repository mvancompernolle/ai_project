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
//  FILE     : adapt.cpp
//
//  DESCRIPTION :
//         Implementation of performeter's user-related metrics classes
//
// ##############################################################



// ===============
#include "adapt.h"
// ===============

// -----------------------------
#if (defined __BCPLUSPLUS__)	// if Borland C++ 5.5.1
template<>
ClassMeasurementCollection<clock_t, clock_t, RESOURCE_real_time>* ClassPerfoMeter<clock_t, clock_t, RESOURCE_real_time>::ptr_local_measurement_collection_s(new ClassMeasurementCollection<clock_t, clock_t, RESOURCE_real_time>());

template<>
ClassMeasurementCollection<clock_t, clock_t, RESOURCE_cpu_time_used>* ClassPerfoMeter<clock_t, clock_t, RESOURCE_cpu_time_used>::ptr_local_measurement_collection_s(new ClassMeasurementCollection<clock_t, clock_t, RESOURCE_cpu_time_used>());

template<>
ClassMeasurementCollection<Dummy1, Dummy2, RESOURCE_dummy>* ClassPerfoMeter<Dummy1, Dummy2, RESOURCE_dummy>::ptr_local_measurement_collection_s(new ClassMeasurementCollection<Dummy1, Dummy2, RESOURCE_dummy>());
#endif
// -----------------------------



// #########################################
// #
// #  Template Functions	(Specialization)
// #  Prototypes is in file user.h
// #
// #########################################


// ###############################################
// ###############################################

#define	SEC_COFACTOR	       1000


#define TEXT__TIME             "time"
#define TEXT__CLOCK            "clock"
#define TEXT__UCLOCK           "uclock"
#define TEXT__TIMEVAL          "timeval"

#define TEXT__REAL_TIME        "elapsed (walk clock) time"
#define TEXT__CPU_TIME         "CPU-time used"
#define TEXT__USER_CPU_TIME    "user CPU-time used"
#define TEXT__SYSTEM_CPU_TIME  "system CPU-time used"

#define TEXT__SEC              "sec"
#define TEXT__SECONDS          "seconds"
#define TEXT__MILLISECONDS     "milliseconds"
#define TEXT__MICROSECONDS     "microseconds"



// ====================================================
// === time_t, time_t, RESOURCE_real_time
// ====================================================

// =============================
template <>
string GetResourceStateUnitName<time_t, time_t, RESOURCE_real_time> ()
{
  return TEXT__TIME;
}

template <>
string GetResourceCostUnitName<time_t, time_t, RESOURCE_real_time> ()
{
  return TEXT__TIME;
}

template <>
string GetResourceName<time_t, time_t, RESOURCE_real_time> ()
{
  return TEXT__REAL_TIME;
}



// =============================
template <>
time_t GetCurrentResource<time_t, time_t, RESOURCE_real_time>()
{
  return time(NULL);
}

// =============================
template <>
time_t GetDiffResourceValue<time_t, time_t, RESOURCE_real_time>(
		const time_t&	start_resource_value_i, 
		const time_t&	end_resource_value_i
		)
{
  return (end_resource_value_i - start_resource_value_i);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<time_t, time_t, RESOURCE_real_time>(
		time_t	cost_value_i 
		)
{
  return (cost_value_i == 0);
}



// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<time_t, time_t, RESOURCE_real_time>(
		time_t	cost_value_i 
		)
{
time_t	shifted_value;
#define	BITS_IN_STOCK	2
const size_t	used_bits =  sizeof (cost_value_i) * CHAR_BIT - BITS_IN_STOCK;

  shifted_value = cost_value_i >> used_bits;

  return ((shifted_value == 0) ? false : true);
}


// =============================
template <>
bool ResourceStateValueIsValid<time_t, time_t, RESOURCE_real_time>(
		const time_t&	state_value_i 
		)
{
  return (!(state_value_i == time_t (-1)));
}

// =============================
template <>
string GetTimeFromResource<time_t, time_t, RESOURCE_real_time>(
		time_t	/* cost_value_i  */
		)
{
  return string();
}



// ===============================================
// === clock_t, clock_t, RESOURCE_cpu_time_used
// ===============================================

// =============================
template <>
string GetResourceStateUnitName<clock_t, clock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__CLOCK;
}

template <>
string GetResourceCostUnitName<clock_t, clock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__CLOCK;
}

template <>
string GetResourceName<clock_t, clock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__CPU_TIME;
}



// =============================
template <>
clock_t GetCurrentResource<clock_t, clock_t, RESOURCE_cpu_time_used>()
{
  return clock();
}

// =============================
template <>
clock_t GetDiffResourceValue<clock_t, clock_t, RESOURCE_cpu_time_used>(
		const clock_t&	start_resource_value_i, 
		const clock_t&	end_resource_value_i
		)
{
  return (end_resource_value_i - start_resource_value_i);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<clock_t, clock_t, RESOURCE_cpu_time_used>(
		clock_t	cost_value_i 
		)
{
  return (cost_value_i == 0);
}



// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<clock_t, clock_t, RESOURCE_cpu_time_used>(
		clock_t	cost_value_i 
		)
{
clock_t	shifted_value;
#define	BITS_IN_STOCK	2
const size_t	used_bits =  sizeof (cost_value_i) * CHAR_BIT - BITS_IN_STOCK;

  shifted_value = cost_value_i >> used_bits;

  return ((shifted_value == 0) ? false : true);
}


// =============================
template <>
bool ResourceStateValueIsValid<clock_t, clock_t, RESOURCE_cpu_time_used>(
		const clock_t&	state_value_i 
		)
{
  return (!(state_value_i == clock_t (-1)));
}


// =============================
template <>
string GetTimeFromResource<clock_t, clock_t, RESOURCE_cpu_time_used>(
		clock_t	cost_value_i 
		)
{
#define SETW_TIME  6
ostringstream oss;
  oss << " = " << setw (SETW_TIME) << ulong (MILLISECS_IN_SEC * (double(cost_value_i)/CLOCKS_PER_SEC)) << " ms";
  return oss.str();
}



////////////////////////
#if (defined UCLOCK_ENV)
////////////////////////
// ===============================================
// === uclock_t, uclock_t, RESOURCE_cpu_time_used
// ===============================================

// =============================
template <>
string GetResourceStateUnitName<uclock_t, uclock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__UCLOCK;
}

template <>
string GetResourceCostUnitName<uclock_t, uclock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__UCLOCK;
}

template <>
string GetResourceName<uclock_t, uclock_t, RESOURCE_cpu_time_used> ()
{
  return TEXT__CPU_TIME;
}



// =============================
template <>
uclock_t GetCurrentResource<uclock_t, uclock_t, RESOURCE_cpu_time_used>()
{
  return uclock();
}

// =============================
template <>
uclock_t GetDiffResourceValue<uclock_t, uclock_t, RESOURCE_cpu_time_used>(
		const uclock_t&	start_resource_value_i, 
		const uclock_t&	end_resource_value_i
		)
{
  return (end_resource_value_i - start_resource_value_i);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<uclock_t, uclock_t, RESOURCE_cpu_time_used>(
		uclock_t	cost_value_i 
		)
{
  return (cost_value_i == 0);
}



// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<uclock_t, uclock_t, RESOURCE_cpu_time_used>(
		uclock_t	cost_value_i 
		)
{
uclock_t	shifted_value;
#define	BITS_IN_STOCK	2
const size_t	used_bits =  sizeof (cost_value_i) * CHAR_BIT - BITS_IN_STOCK;

  shifted_value = cost_value_i >> used_bits;

  return ((shifted_value == 0) ? false : true);
}



// =============================
template <>
bool ResourceStateValueIsValid<uclock_t, uclock_t, RESOURCE_cpu_time_used>(
		const uclock_t&	state_value_i 
		)
{
  return (state_value_i >= 0);
}

// =============================
template <>
string GetTimeFromResource<uclock_t, uclock_t, RESOURCE_cpu_time_used>(
		uclock_t	/* cost_value_i */
		)
{
  return string();
}


//////
#endif
//////


//////////////////////
#if (defined UNIX_ENV)
//////////////////////
// ############################################################
// ############################################################
// ============================================================
// === timeval, double/ulonglong, RESOURCE_user_cpu_time_used
// ===                            RESOURCE_system_cpu_time_used
// ===                            RESOURCE_real_time
// ============================================================

// =========================
static string GetRusageTimeUnitName (ulong cofactor_i)
{
ostringstream oss;

  switch (cofactor_i)
  {
    case 1 :
      oss << TEXT__SECONDS;
      break;

    case MILLISECS_IN_SEC :
      oss << TEXT__MILLISECONDS;
      break;

    case MICROSECS_IN_SEC :
      oss << TEXT__MICROSECONDS;
      break;

    default :
      oss << TEXT__SEC << "/" << cofactor_i;
      break;
  }

  return oss.str();
}

// =========================
bool operator== (const timeval& inst1_i, const timeval& inst2_i)
{
 return (
	(inst1_i.tv_sec == inst2_i.tv_sec)
	&&
 	(inst1_i.tv_usec == inst2_i.tv_usec)
	);
}

// =========================
bool operator< (const timeval& inst1_i, const timeval& inst2_i)
{
 if (inst1_i.tv_sec < inst2_i.tv_sec)	return true;
 if (inst1_i.tv_sec > inst2_i.tv_sec)	return false;

 return (inst1_i.tv_usec < inst2_i.tv_usec);

}

// =========================
bool operator<= (const timeval& inst1_i, const timeval& inst2_i)
{
  return ((inst1_i < inst2_i) || (inst1_i == inst2_i));

}

// =========================
ostream& operator<< (ostream& o, const timeval& instance_i)
{
  return o << "timeval-(" 
	   << instance_i.tv_sec
	   << " sec, "
	   << instance_i.tv_usec
	   << " usec )";
}



// =============================
// -----------------------------
template <>
string GetResourceStateUnitName<timeval, double, RESOURCE_real_time> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, double, RESOURCE_real_time> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (double)";

  return oss.str();
}


template <>
string GetResourceName<timeval, double, RESOURCE_real_time> ()
{
  return TEXT__REAL_TIME;
}


// -----------------------------
template <>
string GetResourceStateUnitName<timeval, ulonglong, RESOURCE_real_time> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, ulonglong, RESOURCE_real_time> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (unsigned long long)";

  return oss.str();
}

template <>
string GetResourceName<timeval, ulonglong, RESOURCE_real_time> ()
{
  return TEXT__REAL_TIME;
}



// ===============================================
// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, double, RESOURCE_real_time>()
{
timeval  tp;
struct timezone tzp;
int rc = gettimeofday (&tp, &tzp);
  assert (rc == 0);	 // TO BE TREATED
  return tp;
}

// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, ulonglong, RESOURCE_real_time>()
{
timeval  tp;
struct timezone tzp;
int rc = gettimeofday (&tp, &tzp);
  assert (rc == 0);	 // TO BE TREATED
  return tp;
}


// =============================
template <>
double GetDiffResourceValue<timeval, double, RESOURCE_real_time>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{

double ret_value = 
	(
	  end_resource_value_i.tv_sec 
	  - 
	  start_resource_value_i.tv_sec
	)
	+ 
	(
	  end_resource_value_i.tv_usec 
	  -
	  start_resource_value_i.tv_usec
	)/ 1.0e6;

  ret_value *= SEC_COFACTOR;

  return ret_value;

}


template <>
ulonglong GetDiffResourceValue<timeval, ulonglong, RESOURCE_real_time>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{
  return static_cast<ulonglong> (GetDiffResourceValue<timeval, double, RESOURCE_real_time>(start_resource_value_i, end_resource_value_i));
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<timeval, double, RESOURCE_real_time>(
		double	cost_value_i 
		)
{
  return (cost_value_i == 0.0);
}

template <>
bool ResourceValueIsSuspiciouslySmall<timeval, ulonglong, RESOURCE_real_time>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i == 0);
}


// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, double, RESOURCE_real_time>(
		double	cost_value_i 
		)
{
  return (cost_value_i > 60.0 * 60.0 * 24.0 * 31.0);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, ulonglong, RESOURCE_real_time>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i > 60 * 60 * 24 * 31);
}


// =============================
template <>
bool ResourceStateValueIsValid<timeval, double, RESOURCE_real_time>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}

// =============================
template <>
bool ResourceStateValueIsValid<timeval, ulonglong, RESOURCE_real_time>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}


// =============================
template <>
string GetTimeFromResource<timeval, double, RESOURCE_real_time>(
		double	/* cost_value_i */ 
		)
{
  return string();
}


// =============================
template <>
string GetTimeFromResource<timeval, ulonglong, RESOURCE_real_time>(
		ulonglong  /* cost_value_i */ 
		)
{
  return string();
}




// =============================
// -----------------------------
template <>
string GetResourceStateUnitName<timeval, double, RESOURCE_user_cpu_time_used> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, double, RESOURCE_user_cpu_time_used> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (double)";

  return oss.str();
}


template <>
string GetResourceName<timeval, double, RESOURCE_user_cpu_time_used> ()
{
  return TEXT__USER_CPU_TIME;
}


// -----------------------------
template <>
string GetResourceStateUnitName<timeval, ulonglong, RESOURCE_user_cpu_time_used> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, ulonglong, RESOURCE_user_cpu_time_used> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (unsigned long long)";

  return oss.str();
}

template <>
string GetResourceName<timeval, ulonglong, RESOURCE_user_cpu_time_used> ()
{
  return TEXT__USER_CPU_TIME;
}



// ===============================================
// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, double, RESOURCE_user_cpu_time_used>()
{
rusage cur_rusage;
int rc = getrusage(RUSAGE_SELF, &cur_rusage);
  assert (!(rc == -1));	 // TO BE TREATED
  return cur_rusage.ru_utime;

}

// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, ulonglong, RESOURCE_user_cpu_time_used>()
{
rusage cur_rusage;
int rc = getrusage(RUSAGE_SELF, &cur_rusage);
  assert (!(rc == -1));  // TO BE TREATED
  return cur_rusage.ru_utime;
}


// =============================
template <>
double GetDiffResourceValue<timeval, double, RESOURCE_user_cpu_time_used>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{

double ret_value = 
	(
	  end_resource_value_i.tv_sec 
	  - 
	  start_resource_value_i.tv_sec
	)
	+ 
	(
	  end_resource_value_i.tv_usec 
	  -
	  start_resource_value_i.tv_usec
	)/ 1.0e6;

  ret_value *= SEC_COFACTOR;

  return ret_value;

}


template <>
ulonglong GetDiffResourceValue<timeval, ulonglong, RESOURCE_user_cpu_time_used>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{
  return static_cast<ulonglong> (GetDiffResourceValue<timeval, double, RESOURCE_user_cpu_time_used>(start_resource_value_i, end_resource_value_i));
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<timeval, double, RESOURCE_user_cpu_time_used>(
		double	cost_value_i 
		)
{
  return (cost_value_i == 0.0);
}

template <>
bool ResourceValueIsSuspiciouslySmall<timeval, ulonglong, RESOURCE_user_cpu_time_used>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i == 0);
}


// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, double, RESOURCE_user_cpu_time_used>(
		double	cost_value_i 
		)
{
  return (cost_value_i > 60.0 * 60.0 * 24.0 * 31.0);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, ulonglong, RESOURCE_user_cpu_time_used>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i > 60 * 60 * 24 * 31);
}


// =============================
template <>
bool ResourceStateValueIsValid<timeval, double, RESOURCE_user_cpu_time_used>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}

// =============================
template <>
bool ResourceStateValueIsValid<timeval, ulonglong, RESOURCE_user_cpu_time_used>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}


// =============================
template <>
string GetTimeFromResource<timeval, double, RESOURCE_user_cpu_time_used>(
		double  /* cost_value_i */ 
		)
{
  return string();
}



// =============================
template <>
string GetTimeFromResource<timeval, ulonglong, RESOURCE_user_cpu_time_used>(
		ulonglong  /* cost_value_i */ 
		)
{
  return string();
}


// =============================
// -----------------------------
template <>
string GetResourceStateUnitName<timeval, double, RESOURCE_system_cpu_time_used> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, double, RESOURCE_system_cpu_time_used> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (double)";

  return oss.str();
}


template <>
string GetResourceName<timeval, double, RESOURCE_system_cpu_time_used> ()
{
  return TEXT__SYSTEM_CPU_TIME;
}


// -----------------------------
template <>
string GetResourceStateUnitName<timeval, ulonglong, RESOURCE_system_cpu_time_used> ()
{
  return TEXT__TIMEVAL;
}

template <>
string GetResourceCostUnitName<timeval, ulonglong, RESOURCE_system_cpu_time_used> ()
{
ostringstream oss;

  oss <<GetRusageTimeUnitName (SEC_COFACTOR);
  oss << " (unsigned long long)";

  return oss.str();
}

template <>
string GetResourceName<timeval, ulonglong, RESOURCE_system_cpu_time_used> ()
{
  return TEXT__SYSTEM_CPU_TIME;
}



// ===============================================
// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, double, RESOURCE_system_cpu_time_used>()
{
rusage cur_rusage;
int rc = getrusage(RUSAGE_SELF, &cur_rusage);
  assert (!(rc == -1));	 // TO BE TREATED
  return cur_rusage.ru_utime;

}

// -----------------------------------------------
template <>
timeval GetCurrentResource<timeval, ulonglong, RESOURCE_system_cpu_time_used>()
{
rusage cur_rusage;
int rc = getrusage(RUSAGE_SELF, &cur_rusage);
  assert (!(rc == -1));  // TO BE TREATED
  return cur_rusage.ru_utime;
}


// =============================
template <>
double GetDiffResourceValue<timeval, double, RESOURCE_system_cpu_time_used>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{

double ret_value = 
	(
	  end_resource_value_i.tv_sec 
	  - 
	  start_resource_value_i.tv_sec
	)
	+ 
	(
	  end_resource_value_i.tv_usec 
	  -
	  start_resource_value_i.tv_usec
	)/ 1.0e6;

  ret_value *= SEC_COFACTOR;

  return ret_value;

}


template <>
ulonglong GetDiffResourceValue<timeval, ulonglong, RESOURCE_system_cpu_time_used>(
		const timeval&	start_resource_value_i, 
		const timeval&	end_resource_value_i
		)
{
  return static_cast<ulonglong> (GetDiffResourceValue<timeval, double, RESOURCE_system_cpu_time_used>(start_resource_value_i, end_resource_value_i));
}

// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<timeval, double, RESOURCE_system_cpu_time_used>(
		double	cost_value_i 
		)
{
  return (cost_value_i == 0.0);
}

template <>
bool ResourceValueIsSuspiciouslySmall<timeval, ulonglong, RESOURCE_system_cpu_time_used>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i == 0);
}


// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, double, RESOURCE_system_cpu_time_used>(
		double	cost_value_i 
		)
{
  return (cost_value_i > 60.0 * 60.0 * 24.0 * 31.0);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<timeval, ulonglong, RESOURCE_system_cpu_time_used>(
		ulonglong	cost_value_i 
		)
{
  return (cost_value_i > 60 * 60 * 24 * 31);
}


// =============================
template <>
bool ResourceStateValueIsValid<timeval, double, RESOURCE_system_cpu_time_used>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}

// =============================
template <>
bool ResourceStateValueIsValid<timeval, ulonglong, RESOURCE_system_cpu_time_used>(
		const timeval&	/* state_value_i */
		)
{
  return true;
}



// =============================
template <>
string GetTimeFromResource<timeval, double, RESOURCE_system_cpu_time_used>(
		double  /* cost_value_i */ 
		)
{
  return string();
}


// =============================
template <>
string GetTimeFromResource<timeval, ulonglong, RESOURCE_system_cpu_time_used>(
		ulonglong  /* cost_value_i */ 
		)
{
  return string();
}



//////
#endif
//////



// ###############################################
// ###############################################

// ===============================================
// === Dummy1, Dummy2, RESOURCE_dummy
// ===============================================


// ###################################################
// ############## "Resource" Test Class ##############
// ###################################################
// =========================
Dummy1::Dummy1 () 
{
  //counter_ = ++counter_s; 
  a_value_ = rand ();
}

// =========================
Dummy1::Dummy1 (int a_value_i) 
{
  //counter_ = ++counter_s; 
  a_value_ = a_value_i;
}

// =========================
Dummy1::~Dummy1 () 
{
}


// =========================
int Dummy1::get_a_value () const
{
  return a_value_;
}

// =========================
ostream& operator<< (ostream& o, const Dummy1& instance_i)
{
  return o << "Dummy1-" << instance_i.a_value_;
}
// ----------------------
bool operator< (const Dummy1& inst1_i, const Dummy1& inst2_i)
{
  return (inst1_i.a_value_ < inst2_i.a_value_);
}
// ----------------------
bool operator== (const Dummy1& inst1_i, const Dummy1& inst2_i)
{
  return (inst1_i.a_value_ == inst2_i.a_value_);
}
// ----------------------
bool operator<= (const Dummy1& inst1_i, const Dummy1& inst2_i)
{
  return ((inst1_i < inst2_i) || (inst1_i == inst2_i));
}
// ----------------------
Dummy1 operator- (const Dummy1& inst1_i, const Dummy1& inst2_i)
{
Dummy1 Dummy1;
  Dummy1.a_value_ = inst1_i.a_value_ - inst2_i.a_value_;
  if (Dummy1.a_value_ < 0) Dummy1.a_value_ = -Dummy1.a_value_;
  return Dummy1;
}



// ###################################################
// ############## "Source" Test Class ################
// ###################################################
// =========================
Dummy2::Dummy2()
{
  // counter_ = ++counter_s;
  b_value_ = 0;
}

// =========================
Dummy2::Dummy2(int b_value_i)
{
  // counter_ = ++counter_s;
  b_value_ = b_value_i;
}

// =========================
Dummy2::~Dummy2()
{
}
// =========================
int Dummy2::get_b_value () const
{
  return b_value_;
}

// =========================
Dummy2& Dummy2::operator+= (const Dummy2& inst_i)
{
  b_value_ += inst_i.b_value_;
  return (*this);
}

// =========================
ostream& operator<< (ostream& o, const Dummy2& instance_i)
{
  return o << "Dummy2-" << instance_i.b_value_;
}
// ----------------------
bool operator< (const Dummy2& inst1_i, const Dummy2& inst2_i)
{
  return (inst1_i.b_value_ < inst2_i.b_value_);
}
// ----------------------
bool operator== (const Dummy2& inst1_i, const Dummy2& inst2_i)
{
  return (inst1_i.b_value_ == inst2_i.b_value_);
}
// ----------------------
bool operator<= (const Dummy2& inst1_i, const Dummy2& inst2_i)
{
  return ((inst1_i < inst2_i) || (inst1_i == inst2_i));
}
// ----------------------
Dummy2 operator- (const Dummy2& inst1_i, const Dummy2& inst2_i)
{
Dummy2 Dummy2;
  Dummy2.b_value_ = inst1_i.b_value_ - inst2_i.b_value_;
  if (Dummy2.b_value_ < 0) Dummy2.b_value_ = -Dummy2.b_value_;
  return Dummy2;
}

// ----------------------
Dummy2 operator/ (const Dummy2& inst1_i, unsigned int int_value_i)
{
Dummy2 Dummy2;
  Dummy2.b_value_ = inst1_i.b_value_/int_value_i;
  return Dummy2;
}


// =============================
template <>
string GetResourceStateUnitName<Dummy1, Dummy2, RESOURCE_dummy> ()
{
  return "state-Dummy1-unit";
}

template <>
string GetResourceCostUnitName<Dummy1, Dummy2, RESOURCE_dummy> ()
{
  return "cost-Dummy2-unit";
}

template <>
string GetResourceName<Dummy1, Dummy2, RESOURCE_dummy> ()
{
  return "dummy resource";
}


// ===============================================
template <>
Dummy1 GetCurrentResource<Dummy1, Dummy2, RESOURCE_dummy>()
{
static int init_a_value = 125;
  init_a_value += 27000 + rand ()%10000;
  return Dummy1(init_a_value);
}

// =============================
template <>
Dummy2 GetDiffResourceValue<Dummy1, Dummy2, RESOURCE_dummy>(
		const Dummy1&	start_resource_value_i, 
		const Dummy1&	end_resource_value_i
		)
{
Dummy1 Dummy1;
  Dummy1 = end_resource_value_i - start_resource_value_i;
  return Dummy2 (3 * Dummy1.get_a_value());
  //return Dummy2 ((Dummy1.get_a_value() * 1000)/start_resource_value_i.get_a_value());
}



// =============================
template <>
bool ResourceValueIsSuspiciouslySmall<Dummy1, Dummy2, RESOURCE_dummy>(
		Dummy2	cost_value_i 
		)
{
  // return false;
  return (cost_value_i.get_b_value() == 0);
}

// =============================
template <>
bool ResourceValueIsSuspiciouslyBig<Dummy1, Dummy2, RESOURCE_dummy>(
		Dummy2	cost_value_i 
		)
{
longlong	shifted_value;
#define	BITS_IN_STOCK	2
const size_t	used_bits =  sizeof (cost_value_i.get_b_value()) * CHAR_BIT - BITS_IN_STOCK;

  shifted_value = (cost_value_i.get_b_value() >> used_bits);

  return ((shifted_value == 0) ? false : true);
}


// =============================
template <>
bool ResourceStateValueIsValid<Dummy1, Dummy2, RESOURCE_dummy>(
		const Dummy1&	/* state_value_i */
		)
{
  return true;
}


// =============================
template <>
string GetTimeFromResource<Dummy1, Dummy2, RESOURCE_dummy>(
		Dummy2  /* cost_value_i */ 
		)
{
  return string();
}



///////////////
// End-Of-File
///////////////
