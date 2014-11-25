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
//  FILE     : pfmeter.cpp
//
//  DESCRIPTION :
//         Implementation of perfometer's common (non-template) classes
//
// ##############################################################



// ===================
#include "pfmeter2.h"
// ===================

// -----------------------------------------
static const string log_file_suffix_CNS (".log");
static const string setup_file_suffix_CNS (".stp");

#define	SETUP_FILE_COMMENT_SIGN	'#'
#define	ACTIVE_GROUP_SIGN	'+'
#define	PASSIVE_GROUP_SIGN	'-'

#define SPACES			" \t"

// -----------------------------------------

// -----------------------------------------
unsigned long meas_counter_exn = 0;
unsigned long curr_meas_exn = 0;
// -----------------------------------------

// -----------------------------------------
// -
// -  Constants for IO Manipulation	
// -
// -----------------------------------------
static const string	space1	(1, ' ');
static const string	space2	(2, ' ');
static const string	space3	(3, ' ');
static const string	space4	(4, ' ');
static const string	space5	(5, ' ');
static const string	space9	(9, ' ');

static const string	slash2	(2, '/');

static const string	colon1 (1, ':');

static const string	sulamit1 (1, '#');

static const string	pref1	("# ");

static const size_t	size12	= 12;
static const size_t	size25	= 25;
static const size_t	size27	= 27;
static const size_t	size38	= 38;
static const size_t	size40	= 40;
static const size_t	size43	= 43;
static const size_t	size45	= 45;
static const size_t	setw16	= 16;

// #########################################
// #
// #  Global (Non-Template) Functions 
// #  	IMPLEMENTATION
// #
// #########################################
// =============================
string CutName(const string& str_i)
{
string::size_type pos = str_i.find_first_not_of (SPACES);
  if (pos == string::npos) return string();

string ret_str (str_i.substr (pos)); 
  pos = ret_str.find_last_not_of (SPACES);
  assert (!(pos == string::npos));

  return ret_str.erase(pos + 1);
}

// =============================
void MeasureFunc (MeasuredFuncType const func_i)
{
  mng.add_measured_func (func_i);
}


// =============================
void CompareThisFunc(
		const string&	group_name_i, 
		const string&	file_name_i, 
		size_t		line_no_i, 
		MeasuredFuncType const	func_i
		)
{
  // ------------------------
const string cut_group_name (CutName (group_name_i));
  if (cut_group_name.empty())
  {
    ostringstream oss;
    oss << endl

        << "\t--- WARNING! Empty group name : See " 
        << file_name_i
        << ", #"
        << line_no_i 
        << endl

        << "\t--- -------- The group rejected" 
        << endl
        << endl;

    cerr << oss.str();
    sout << oss.str();

    return;
  }
  assert (!cut_group_name.empty());

  // -----------------------------
  mng.add_comparison_group (
			cut_group_name, 
			file_name_i, 
			line_no_i
			);

  // -----------------------------
  if (!mng.is_active_comparison_group (group_name_i)) return;


  // -----------------------------
  // -----------------------------
  mng.add_compared_func (
		GroupDetailedName (
			cut_group_name, 
			file_name_i, 
			line_no_i
			),
		func_i
		);
}

// =============================
void SetTotalTests(ulong tests_i)
{
  mng.set_tests (tests_i);
}


// =============================
void SetDiscardingThreshold(double discarding_threshold_i)
{
  mng.set_discarding_threshold (discarding_threshold_i);
}


// =============================
void SetScaleAndTotalRepetitions (ulong per_calls_i, ulong repetitions_i)
{
  mng.set_per_calls (per_calls_i);	// Must be before set_repetitions()
  mng.set_repetitions (repetitions_i);
}

// =============================
void SetDetailedMeasurementReportFlag(bool flag_i)
{
  mng.set_detailed_measurement_report_flag(flag_i);
}

// =============================
void SetMeasurementReportFlag(bool flag_i)
{
  mng.set_measurement_report_flag(flag_i);
}

// =============================
void SetTracingReportFlag(bool flag_i)
{
  mng.set_tracing_report_flag (flag_i);
}

// =============================
bool ParseCommandLine(int argc, char** argv)
{
  return mng.parse_command_line(argc, argv); 
}

     
// =============================
bool RunDemo()
{
  return mng.get_demo_flag(); 
}

     
// =============================
void Measure()
{
  RunDemo() ? MeasureDemo() : MeasureIt();
}

     
// =============================
void Compare()
{
  RunDemo() ? CompareDemo() : CompareIt();
}



// #########################################
// #
// #  AuxClass-0 : GroupDetailedName
// #  	METHODS
// #
// #########################################
// =============================
bool operator< (
		const GroupDetailedName& ins1, 
		const GroupDetailedName& ins2
		)
{

  // DON'T CHANGE
  assert (!ins1.group_name_.empty());
  assert (!ins2.group_name_.empty());

  return (ins1.group_name_ < ins2.group_name_);

  // -------------------------------------------------------
  // IMPORTANT. file_lines_  must not take part in operator<
  // -------------------------------------------------------


}


// =============================
// Constructor-1
GroupDetailedName::GroupDetailedName(
		const string&	group_name_i,
		const string&	file_name_i,
		size_t		line_no_i
		)
		:
		group_name_ (group_name_i)
		// file_name_ (file_name_i),
		// line_no_ (line_no_i)
{
  if (file_lines_.count(file_name_i) == 0)
  {
    file_lines_[file_name_i] = vector<size_t> ();
  }
  assert (file_lines_.count(file_name_i) == 1);

  file_lines_[file_name_i].push_back (line_no_i);

  assert (file_lines_[file_name_i].size() == 1);

}


// =============================
// Constructor-2
GroupDetailedName::GroupDetailedName(
		const string&	group_name_i,
		const map<string, vector<size_t> >&	file_lines_i
		)
		:
		group_name_ (group_name_i),
		file_lines_ (file_lines_i)
{
}
  


// =============================
// Destructor
GroupDetailedName::~GroupDetailedName()
{
}

// =============================
bool GroupDetailedName::empty () const
{
  return group_name_.empty();
}


// =============================
string GroupDetailedName::get_group_name () const
{
  return group_name_;
}


// =============================
map<string, vector<size_t> > GroupDetailedName::get_file_lines () const
{
  return file_lines_;
}


// =============================
string GroupDetailedName::get_group_detailed_name (size_t setw_size_i) const
{
ostringstream oss;
  oss << ""
      << setw (setw_size_i)
      << left
      << group_name_.c_str()
      << "  [";

map<string, vector<size_t> >::const_iterator iter;
size_t id1;
  for (iter = file_lines_.begin(), id1 = 0;
       iter != file_lines_.end();
       iter++, id1++
      )
  {
    if (id1 > 0) oss << "; ";
    oss << iter->first;
    oss << " : ";
    
    for (size_t id2 = 0; id2 < iter->second.size(); id2++)
    {
      if (id2 > 0) oss << ", ";
      oss << iter->second[id2];
    }
  }
  oss << "]";

  return oss.str();   
}


// #########################################
// #
// #  Class-1 : ClassResultsFunc
// #  	METHODS
// #
// #########################################

// -----------------------------
size_t		ClassResultsFunc::max_prettyfuncname_len_s(0);
size_t		ClassResultsFunc::max_funcname_len_s(0);
size_t		ClassResultsFunc::max_filename_len_s(0);
size_t		ClassResultsFunc::max_descrmsg_len_s(0);
// -----------------------------

// =============================
bool operator< (
		const ClassResultsFunc&	ins1, 
		const ClassResultsFunc& ins2
		)
// =============================
{
  if (ins1.prettyfuncname_ < ins2.prettyfuncname_) return true;
  if (ins1.prettyfuncname_ > ins2.prettyfuncname_) return false;

  if (ins1.funcname_ < ins2.funcname_) return true;
  if (ins1.funcname_ > ins2.funcname_) return false;

  if (ins1.filename_ < ins2.filename_) return true;
  if (ins1.filename_ > ins2.filename_) return false;

  if (ins1.lineno_ < ins2.lineno_) return true;
  if (ins1.lineno_ > ins2.lineno_) return false;

  if (ins1.descrmsg_ < ins2.descrmsg_) return true;
  if (ins1.descrmsg_ > ins2.descrmsg_) return false;
 
  // ----------------------------------------------------
  // IMPORTANT. funcptr_ must not take part in operator<
  // ----------------------------------------------------


  return false;

} // operator<


// =============================
bool operator== (
		const ClassResultsFunc&	ins1, 
		const ClassResultsFunc& ins2
		)
// =============================
{
  if (!(ins1.prettyfuncname_ == ins2.prettyfuncname_))	return false;
  if (!(ins1.funcname_ == ins2.funcname_))		return false;
  if (!(ins1.filename_ == ins2.filename_))		return false;
  if (!(ins1.lineno_ == ins2.lineno_))			return false;
  if (!(ins1.descrmsg_ == ins2.descrmsg_))		return false;

  // -----------------------------------------------------
  // IMPORTANT. funcptr_ must not take part in operator==
  // -----------------------------------------------------

  return true;

} // operator==


// =============================
// Constructor-0
ClassResultsFunc::ClassResultsFunc() :
      		funcptr_ (NULL),		
      		prettyfuncname_ (""),		
      		funcname_ (""),		
      		filename_ (""),		
      		lineno_ (0),		
      		descrmsg_ ("")		
// =============================
{
  assert (0);
} // ClassResultsFunc


// =============================
// Constructor-1
ClassResultsFunc::ClassResultsFunc(
      	MeasuredFuncType	const	funcptr_i,		
	const string&			prettyfuncname_i,
	const string&			funcname_i,
	const string&			filename_i,
	size_t				lineno_i,
	const string&			descrmsg_i
	) :
      		funcptr_ (funcptr_i),		
      		prettyfuncname_ (prettyfuncname_i),		
      		funcname_ (funcname_i),		
      		filename_ (filename_i),		
      		lineno_ (lineno_i),		
      		descrmsg_ (descrmsg_i)		
// =============================
{

  max_prettyfuncname_len_s = MAX_VALUE (
				max_prettyfuncname_len_s, 
				prettyfuncname_.size()
				);

  max_funcname_len_s = MAX_VALUE (
				max_funcname_len_s, 
				funcname_.size()
				);

  max_filename_len_s = MAX_VALUE (
				max_filename_len_s, 
				filename_.size()
				);
  max_descrmsg_len_s = MAX_VALUE (
				max_descrmsg_len_s, 
				descrmsg_.size()
				);

} // ClassResultsFunc



// =============================
// Destructor
ClassResultsFunc::~ClassResultsFunc()
// =============================
{
} // ~ClassResultsFunc


// =============================
string ClassResultsFunc::get_prettyfuncname () const
// =============================
{
  return prettyfuncname_;
}


// =============================
string ClassResultsFunc::get_funcname () const
// =============================
{
  return funcname_;
}

// =============================
string ClassResultsFunc::get_filename () const
// =============================
{
  return filename_;
}


// =============================
size_t ClassResultsFunc::get_lineno () const
// =============================
{
  return lineno_;
}


// =============================
string ClassResultsFunc::get_descr () const
// =============================
{
  return descrmsg_;
}

// =============================
MeasuredFuncType ClassResultsFunc::get_funcptr () const
// =============================
{
  return funcptr_;
}

// =============================
string ClassResultsFunc::getstr_func_detailed_descr (
		vector<string>	resource_descr_i,
		count_t		func_serial_no_i, 
		count_t		total_funcs_i, 
		count_t		total_tests_i, 
		bool		detailed_report_flag_i, 
		const string&	msg_i
		) const

// =============================
{
ostringstream osstr;

  osstr	<< endl;

const string word_Resource ("Resource");

#ifdef OPTIMIZATION_LEVEL
  osstr << space3 << "OPTIMIZATION_LEVEL" << endl;
#endif

  osstr	<< space3
	<< string (size45, '#') 
	<< endl;

  osstr	<< space3
        << pref1
        << string (word_Resource.size(), '-')
	<< endl;

  osstr	<< space3
        << pref1
        << word_Resource
	<< endl;

  osstr	<< space3
        << pref1
        << string (word_Resource.size(), '-')
	<< endl;

  for (count_t i = 0; i < resource_descr_i.size(); i++)
  {
    osstr	<< space3
    		<< pref1
    		<< "\t"
    		<< resource_descr_i[i]
    		<< endl;
 }

  osstr	<< space3
	<< pref1
	<< string (size43, '=') 
	<< endl;

  if (!msg_i.empty())
  {
    osstr << space3 << msg_i << " ---> "; 
  }


  osstr	<< space3
	<< pref1 
	<< setw (setw16) 
	<< string ("Function-" + to_string (func_serial_no_i)).c_str()
	<< " : " 
	<< "of "
        << total_funcs_i
	<< endl;

  osstr	<< space3
	<< pref1 
	<< setw (setw16) 
	<< "PRETTY FUNCTION" 
	<< " : " 
	<< prettyfuncname_ 
	<< endl;

  osstr	<< space3
	<< pref1 
	<< setw (setw16) 
	<< "FUNCTION" 
	<< " : " 
	<< funcname_ 
	<< endl;

  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< "FILE" 
	<< " : " 
	<< filename_ 
	<< ", line#" 
	<< lineno_ 
	<< endl;

  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< "DESCRIPTION" 
	<< " : " 
	<< descrmsg_ 
	<< endl;


  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< "PER-CALLS" 
	<< " : " 
	<< mng.get_scale_per_calls() 
	<< endl;


  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< "TOTAL TESTS" 
	<< " : " 
	<< total_tests_i 
	<< endl;

  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< string (9, '-').c_str() 
	<< string (3, '-') 
	<< string (9, '-') 
	<< endl;

  osstr	<< space3 
	<< pref1 
	<< "Specific Performance is Runcost expended per " 
	<< mng.get_scale_per_calls() 
	<< " repetitions"
	<< endl;

  osstr	<< space3 
	<< pref1 
	<< setw (setw16) 
	<< string (9, '-').c_str() 
	<< string (3, '-') 
	<< string (9, '-') 
	<< endl;

  osstr	<< space3
	<< pref1
	<< space5;

  if (detailed_report_flag_i)
  { 
    osstr << "Detailed measurement report";
  }
  else
  {
    osstr << "Brief measurement report";
  }
  osstr	<< endl;

  osstr	<< space3
	<< string (size45, '#') 
	<< endl;

 return osstr.str();   

} // ClassResultsFunc::getstr_func_detailed_descr



// =============================
size_t ClassResultsFunc::Get_Max_PrettyFuncName_Len () 
// =============================
{
  return max_prettyfuncname_len_s;
}


// =============================
size_t ClassResultsFunc::Get_Max_FuncName_Len () 
// =============================
{
  return max_funcname_len_s;
}


// =============================
size_t ClassResultsFunc::Get_Max_FileName_Len () 
// =============================
{
  return max_filename_len_s;
}



// =============================
size_t ClassResultsFunc::Get_Max_LineNo_Len () 
// =============================
{
  return 3;
}

// =============================
size_t ClassResultsFunc::Get_Max_DescrMsg_Len () 
// =============================
{
  return max_descrmsg_len_s;
}



// #########################################
// #
// #  Class-7 : BasicClassMeasurementCollection
// #  	METHODS
// #
// #########################################
// -----------------------------
vector<BasicClassMeasurementCollection*>	BasicClassMeasurementCollection::all_measurement_collections_s;
// -----------------------------

// =============================
// Constructor-0
BasicClassMeasurementCollection::BasicClassMeasurementCollection () 
			:
			typeid_name_(string()),
			total_measured_funcs_(0),
    			resource_description_ (vector<string>())
// =============================
{
}

// =============================
// Copy Constructor
BasicClassMeasurementCollection::BasicClassMeasurementCollection (const BasicClassMeasurementCollection&) 
// =============================
{
  assert (0);
}

// =============================
// Destructor
BasicClassMeasurementCollection::~BasicClassMeasurementCollection () 
// =============================
{
}



// =============================
void BasicClassMeasurementCollection::add_resource_type_if_need () 
// =============================
{

  if (find_if(
		all_measurement_collections_s.begin(), 
		all_measurement_collections_s.end(), 
		bind1st(
			mem_fun(&BasicClassMeasurementCollection::typeid_predicat), 
			this
			)
		)
      ==
      all_measurement_collections_s.end()
     )
  {
    all_measurement_collections_s.push_back(this);
  }
  
  
  assert (count_if(
		all_measurement_collections_s.begin(), 
		all_measurement_collections_s.end(), 
		bind1st(
			mem_fun(&BasicClassMeasurementCollection::typeid_predicat), 
			this
			)
		)
          == 1
         );

} // BasicClassMeasurementCollection::add_resource_type_if_need



// =============================
bool BasicClassMeasurementCollection::typeid_predicat (
		const BasicClassMeasurementCollection* const p_inst_i
		) const 
// =============================
{
  assert (p_inst_i);  
  return (typeid_name_ == p_inst_i->typeid_name_);
}



// =============================
size_t BasicClassMeasurementCollection::Get_Total_Resource_Types ()
// =============================
{
  return all_measurement_collections_s.size();
}


// =============================
void BasicClassMeasurementCollection::Show_List_Of_Resources ()
// =============================
{

  // -----------------------------------------------
  if (all_measurement_collections_s.empty()) return;
  // -----------------------------------------------

const char fill0 = '%';
const char fill1 = '=';
ostringstream osstr;
  

  sout << endl;
  sout << endl;
  sout << endl;
  sout << "\t" << string (42, fill1) << endl;
  sout << "\t" << string (2, fill0) << string (40, fill1) << endl;
  sout << "\t" << string (2, fill0) << endl;

  sout << "\t" << string (2, fill0) << space2 
       << "Total Measured Resources : " 
       << all_measurement_collections_s.size() 
       << endl;

  for (count_t i = 0; i < all_measurement_collections_s.size(); i++)
  {

    sout << "\t" 
         << string (2, fill0) 
         << space1 
         << endl; 

    sout << "\t" 
         << string (2, fill0) 
         << space2 
         << string (10, '-')
         << space1 
         << "Resource#"
         << (i + 1)
         << space1 
         << string (10, '-')
         << endl; 

    for (
	count_t j = 0; 
	        j < all_measurement_collections_s[i]->resource_description_.size(); 
	        j++
	        )
    {
      sout << "\t" 
           << string (2, fill0) 
           << space1; 
      sout << all_measurement_collections_s[i]->resource_description_[j];
      sout << endl;

    }

    sout << "\t" 
         << string (2, fill0) 
         << space1
         << "---> Total measured functions : "
         << all_measurement_collections_s[i]->total_measured_funcs_
         << endl; 
  }
  sout << "\t" << string (2, fill0) << "\t" << string(osstr.str().size(), '-') << endl;

  sout << "\t" << string (2, fill0) << endl;
  sout << "\t" << string (2, fill0) << string (40, fill1) << endl;
  sout << "\t" << string (42, fill1) << endl;
  sout << endl;


} // BasicClassMeasurementCollection::Show_List_Of_Resources



// #########################################
// #
// #  Class-9 : ClassManager	
// #  	METHODS
// #
// #########################################

// -----------------------------
ostringstream		sout;
ClassManager		mng;
int			bogus = 0;
count_t			ClassManager::inst_counter_s(0);
MeasuredFuncType	ClassManager::cur_func_ptr_s(NULL);

// -----------------------------

// =============================
// Constructor-0
ClassManager::ClassManager () 
		: 
		all_groups_are_active_ (false),
		performing_flag_ (true), 
		rebuilding_setup_flag_ (false),
		reading_setup_file_content_ (true),
		demo_flag_ (false)
// =============================
{

  // ---------------------
  // show_logo ("");
  // ---------------------

  inst_counter_s++;
  assert (inst_counter_s == 1);
  // -----------------------
  set_measurement_report_flag ();
  set_detailed_measurement_report_flag ();
  set_tracing_report_flag ();

  set_discarding_threshold (); 
  set_tests (); 
  set_per_calls (); 	// Must be before  set_repetitions()
  set_repetitions (); 

  assert (scale_per_calls_ > 0);
  assert (total_repetitions_ > 0);
  assert (scale_per_calls_ <= total_repetitions_);

  assert (total_tests_ > 0);


} // ClassManager::ClassManager


// =============================
// Copy Constructor
ClassManager::ClassManager (const ClassManager&) 
// =============================
{
  assert (0);
} // ClassManager::ClassManager


// =============================
// Destructor
ClassManager::~ClassManager () 
// =============================
{

  if (reading_setup_file_content_ && (rebuilding_setup_flag_ || (!setup_file_is_availble ())))
  {
    if (rebuilding_setup_flag_)	selected_groups_.clear();

    create_primary_setup_file ();
    performing_flag_ = false;

    assert (selected_groups_.empty());

    read_setup_file_content ();

  }

  assert (!reading_setup_file_content_ || setup_file_is_availble ());


  // ------------------------
  if (reading_setup_file_content_) check_selected_groups ();
  assert (selected_groups_.size() <= active_comparison_groups_.size());

  // ------------------------
  if (performing_flag_)
  {
    show_comparison_groups (false);
    perform_measurement ();
    report_measurement (); 
  }
  else
  {
    show_comparison_groups (true);
    // MAYBE TBD : show_testcases ();
  }
  delete_it (); 

  // -------------------------
const string logfile_info ("Log file  " + (log_filename_.empty() ? string("has not been created") : log_filename_));
  show_logo ("FINISH : " + logfile_info);

  close_log ();

ostringstream oss;
  
  oss << logfile_info;

  cout << endl;
  cout << "\t" << string (oss.str().size(), '-') << endl;
  cout << "\t" << oss.str() << endl;
  cout << "\t" << string (oss.str().size(), '-') << endl;
  cout << endl;
  
}


// =============================
void ClassManager::open_log ()
// =============================
{
  assert (fout_);

  fout_.open (log_filename_.c_str());

  if ((!fout_) || (fout_ && !fout_.is_open()))
  {
    cerr << endl;
    cerr << "\t--- WARNING! Unable to open log-file " << log_filename_ << " ---" << endl;
    cerr << endl;
 
    log_filename_.clear();

    return;
  }
  assert (fout_);
  assert (fout_.is_open());

}


// =============================
void ClassManager::close_log ()
// =============================
{

  if (fout_ && fout_.is_open()) 
  {
    fout_ << sout.str();

    assert (fout_);
    assert (fout_.is_open());

    fout_.close();

  }
  else
  {
    cout << sout.str();
  }


  assert (!fout_ || (fout_ && !fout_.is_open()));

}

// =============================
void ClassManager::flush_log ()
// =============================
{

  assert (!(fout_ && fout_.is_open()));
  fout_ << sout.str();
  sout.str("");
}


// =============================
void ClassManager::set_log_filename (const string& log_filename_i)
// =============================
{
  log_filename_ = log_filename_i;  
  assert (!log_filename_.empty());
}


// =============================
void ClassManager::set_setup_filename (const string& setup_filename_i)
// =============================
{
  setup_filename_ = setup_filename_i;  
  assert (!setup_filename_.empty());
}


// =============================
void ClassManager::perform_measurement () const
// =============================
{

  for (count_t i = 0; i < total_tests_; i++)
  {
    curr_meas_exn = i + 1;
    perform_a_measurement (); 
  }

} // perform_measurement

// =============================
void ClassManager::perform_a_measurement () const
// =============================
{

  /*
  for_each(
	measured_funcs_.begin(), 
	measured_funcs_.end(), 
	self_caller<MeasuredFuncType>
	);
  */


  cur_func_ptr_s = NULL;
  for (count_t cur_func_no = 0; 
	cur_func_no < measured_funcs_.size(); 
	cur_func_no++
	)
  {
    cur_func_ptr_s = measured_funcs_[cur_func_no];
    assert (cur_func_ptr_s);

    // -----------------------------
    measured_funcs_[cur_func_no] ();
    // -----------------------------

    cur_func_ptr_s = NULL;
  }


} // perform_a_measurement


// =============================
void ClassManager::report_measurement () const
// =============================
{

vector<BasicClassMeasurementCollection*> &all_measurement_collections = 
		BasicClassMeasurementCollection::all_measurement_collections_s;

map<GroupDetailedName, vector<MeasuredFuncType>, less<GroupDetailedName> >::const_iterator	map_iter;

  BasicClassMeasurementCollection::Show_List_Of_Resources ();

  for (count_t i = 0;
               i < all_measurement_collections.size();
               i++
      )
  {
   
    if (mng.get_measurement_report_flag ())
    {
      // Measurement Report
      all_measurement_collections[i]->show_resource_detailed_descr(i + 1, "Performance Measurement", false);  
      all_measurement_collections[i]->show_measurement_collection();
    }


    // -----------------------
    // Comparison Report
    all_measurement_collections[i]->show_resource_detailed_descr(i + 1, "Performance Comparison", true);  

    count_t group_no = 0;
    for (
	  map_iter = active_comparison_groups_.begin();
	  map_iter != active_comparison_groups_.end();
	  map_iter++
	  )
    {
      group_no++;
      ostringstream osstr1;
      ostringstream osstr2;
      ostringstream osstr3;
      ostringstream osstr4;

      size_t max_len = 0;
      osstr1 << "Comparison Group#"
             << group_no
             << " of "
             << active_comparison_groups_.size()
             << " : "
             << map_iter->first.get_group_name();
      max_len = MAX_VALUE (max_len, osstr1.str().size() + 1);

      for (
	   count_t j = 0; 
	           j < all_measurement_collections[i]->resource_description_.size(); 
	           j++
	           )
      {
        osstr2 << "\t\t" 
               << "#" 
               << string (2, ' ') 
               << space1; 
        osstr2 << all_measurement_collections[i]->resource_description_[j];
        osstr2 << endl;
      }

      osstr3 << "" 
             << string (2, ' ') 
             << space1 
             << "Total repetitions   : " 
             << get_current_total_repetitions ();
      max_len = MAX_VALUE (max_len, osstr3.str().size());


      osstr4 << "" 
             << string (2, ' ') 
             << space1 
             << "Performance metrics : " 
             << all_measurement_collections[i]->get_resource_cost_unit_description ()
             << " / "
             << get_scale_per_calls ()
             << " repetition"
             << ((get_scale_per_calls () > 1) ? "s" : "");
      max_len = MAX_VALUE (max_len, osstr4.str().size());


      // -----------------------
      sout << endl;
      sout << endl;

#ifdef OPTIMIZATION_LEVEL
  sout << "\t\t" << "OPTIMIZATION_LEVEL" << endl;
#endif

      sout << "\t\t" 
	   << "#" 
	   << string (max_len, '=') 
	   << endl;

      sout << "\t\t" 
	   << "#" 
	   << " " 
	   << osstr1.str() 
	   << endl;

      sout << "\t\t" 
	   << "#" 
           << string (max_len, '-')
	   << endl;

      sout << osstr2.str(); 

      sout << "\t\t" 
	   << "#" 
           << string (max_len, '-')
	   << endl;

      sout << "\t\t" 
	   << "#" 
	   << osstr3.str() 
	   << endl;

      sout << "\t\t" 
	   << "#" 
	   << osstr4.str() 
	   << endl;

      sout << "\t\t" 
	   << "#" 
	   << string (max_len, '=') 
	   << endl;
      
      osstr1.str(string());
    
      osstr1 << string (3, ' ') 
             << ": " 
             << string (
		ClassResultsFunc::Get_Max_FuncName_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_FileName_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_LineNo_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_DescrMsg_Len() + 4
		+
		12
		, '-'); 

      sout << osstr1.str() << endl;
      all_measurement_collections[i]->show_comparison_collection(map_iter->second);
      sout << osstr1.str() << endl;
    }
    sout << endl;
    sout << endl;

  }  // for (count_t i = 0; ...

} // ClassManager::report_measurement


// =============================
void ClassManager::add_measured_func (MeasuredFuncType const func_i)
// =============================
{
  assert (func_i);
  if (find (
	     measured_funcs_.begin(), 
	     measured_funcs_.end(),
	     func_i
	     )
	     == measured_funcs_.end()
     )
  {
    measured_funcs_.push_back(func_i);
  }
} 


// =============================
void ClassManager::add_comparison_group (
			const string&	group_name_i,
			const string&	file_name_i,
			size_t		line_no_i
			)
// =============================
{
  if (group_name_i.empty())	
  {
    sout << "Unable to add active comparison group : group name is empty" << endl;
    return;
  }

  assert (!group_name_i.empty());
  assert (!file_name_i.empty());
  assert (line_no_i > 0);

  if (all_comparison_groups_.count(group_name_i) == 0)
  {
    all_comparison_groups_[group_name_i] = map<string, vector<size_t> > (); 
  }
  assert (all_comparison_groups_.count(group_name_i) == 1);

  if (all_comparison_groups_[group_name_i].count(file_name_i) == 0)
  {
    all_comparison_groups_[group_name_i][file_name_i] = vector<size_t> (); 
  }
  assert (all_comparison_groups_[group_name_i].count(file_name_i) == 1);

  all_comparison_groups_[group_name_i][file_name_i].push_back(line_no_i); 

}

// =============================
void ClassManager::add_active_comparison_group (
			const GroupDetailedName& group_detailed_name_i
			)
// =============================
{
  if (group_detailed_name_i.empty())	
  {
    sout << "Unable to add active comparison group : group name is empty" << endl;
    return;
  }

  assert (!group_detailed_name_i.empty());
  assert (
	  (active_comparison_groups_.count(group_detailed_name_i) == 0)
	  ||
	  (active_comparison_groups_.count(group_detailed_name_i) == 1)
	 );


  if (active_comparison_groups_.count(group_detailed_name_i) == 0)
  {
    active_comparison_groups_ [group_detailed_name_i] = vector<MeasuredFuncType> ();
  }
  else
  {
    GroupDetailedName tmp_group_detailed_name (group_detailed_name_i); // Default Copy Constructor

    map<GroupDetailedName, vector<MeasuredFuncType>, less<GroupDetailedName> >::iterator find_iter;
    find_iter = active_comparison_groups_.find (group_detailed_name_i);
    assert (find_iter != active_comparison_groups_.end());

    vector<MeasuredFuncType> tmp_vect (find_iter->second);

    map<string, vector<size_t> > map_to_be_updated (find_iter->first.get_file_lines());
    map<string, vector<size_t> > new_map (group_detailed_name_i.get_file_lines());
    assert (new_map.size() == 1);

    assert (new_map[new_map.begin()->first].size() == 1);
    const string group_new_file_name (new_map.begin()->first);
    const size_t group_new_line_no (new_map.begin()->second.front());

    assert (map_to_be_updated.size() > 0);

    map<string, vector<size_t> >::const_iterator find2_iter;

    find2_iter = map_to_be_updated.find(group_new_file_name);

    if (find2_iter == map_to_be_updated.end())
    {
      map_to_be_updated[group_new_file_name] = vector<size_t> ();
    }
    map_to_be_updated[group_new_file_name].push_back (group_new_line_no);

    assert (!active_comparison_groups_.empty());
    active_comparison_groups_.erase(find_iter);

    active_comparison_groups_[GroupDetailedName (
				group_detailed_name_i.get_group_name(), 
				map_to_be_updated
				)
		      ] = tmp_vect; 
  }
  assert (active_comparison_groups_.count(group_detailed_name_i) == 1);

} // add_active_comparison_group


// =============================
void ClassManager::add_compared_func (
			const GroupDetailedName& group_detailed_name_i,
			MeasuredFuncType	 const func_i
			)
// =============================
{
  // if (active_comparison_groups_.count(group_detailed_name_i) == 0)
  // {
  // -----------------------------------------
  add_active_comparison_group (group_detailed_name_i);
  // -----------------------------------------
  // }

  assert (!active_comparison_groups_.empty());
  assert (active_comparison_groups_.count(group_detailed_name_i) == 1);
  assert (func_i);


  active_comparison_groups_[group_detailed_name_i].push_back(func_i);

  if (find (
	     active_comparison_groups_[group_detailed_name_i].begin(), 
	     active_comparison_groups_[group_detailed_name_i].end(),
	     func_i
	     )
	     == active_comparison_groups_[group_detailed_name_i].end()
     )
  {
    active_comparison_groups_[group_detailed_name_i].push_back(func_i);
  }

  add_measured_func (func_i);
} 


// =============================
void ClassManager::show_comparison_groups (bool show_all_i) const
// =============================
{
  if (all_comparison_groups_.empty()) return;

size_t number_of_active_groups = 0;

map<string, map<string, vector<size_t> > >::const_iterator citer;
  for (citer = all_comparison_groups_.begin();
       citer != all_comparison_groups_.end();
       citer++
       )
  {
    if (is_active_comparison_group (citer->first)) number_of_active_groups++;
  }

  if ((!show_all_i) && (number_of_active_groups == 0)) return;

 // --------------------------
ostringstream oss_testsuites;
  oss_testsuites << "Testsuites-" << sw_testsuites_version;

ostringstream oss_what;
  if (get_demo_flag ())
  {
    oss_what << "Demo";
  }
  else
  {
    if (show_all_i) oss_what << "All";
    else            oss_what << "Active";      
  }

ostringstream oss_groups;
  oss_groups <<	oss_what.str() << " Comparison Groups";

ostringstream oss_notes;
  if (!get_demo_flag () && !show_all_i)
  {
    oss_notes << " (are set via Setup file " << setup_filename_ << ")";
  }

  // -------------
ostringstream oss;

  oss << endl;
  if (show_all_i)
  {  
    oss << endl
        << "\t " << oss_testsuites.str()
        << endl
        << "\t " << string (oss_testsuites.str().size(), '=')
        << endl;
  }

  oss_groups << oss_notes.str();
  oss << ""
      << "\t" << oss_groups.str()
      << endl
      << "\t" << string (oss_groups.str().size(), '-');
      
  oss << endl;


map<string, vector<size_t> >::const_iterator c2iter;

size_t setw_size = 0;
  for (citer = all_comparison_groups_.begin();
       citer != all_comparison_groups_.end();
       citer++
       )
  {
    setw_size = MAX_VALUE (setw_size, citer->first.size());
  }
  setw_size++;

  for (citer = all_comparison_groups_.begin();
       citer != all_comparison_groups_.end();
       citer++
       )
  {
    const string group_name (citer->first);
    if (!show_all_i && !is_active_comparison_group (group_name)) continue;

    // -------------
    oss << "\t";
    if (is_active_comparison_group (group_name))
    {
      oss << ACTIVE_GROUP_SIGN;
    }
    else
    {
      oss << PASSIVE_GROUP_SIGN;
    }
    oss << " " 
        << setw (setw_size) 
        << std::left
        << group_name; 

    oss << "[";
    
    bool flag = false;
    for (c2iter = citer->second.begin();
         c2iter != citer->second.end();
         c2iter++
         )
    {
      if (flag) oss << "; ";
      else      flag = true;

      oss << c2iter->first << " : "; 

      const vector<size_t> line_nos (c2iter->second);
      for (size_t i = 0; i < line_nos.size(); i++)
      {
        if (i > 0) oss << ", ";
        oss << line_nos[i];
      }
    }
    oss << "]";

    oss << endl;
  }
  oss << endl;
  oss << endl;

  // ----------
  sout << oss.str();
  cerr << oss.str();

}

// =============================
void ClassManager::show_testcases () const
// =============================
{
// --------------			
// TO BE CHANGED  
assert(0 && "not for use ---> perhaps to be changed");
// --------------			

vector<BasicClassMeasurementCollection*> &all_measurement_collections = 
		BasicClassMeasurementCollection::all_measurement_collections_s;

map<GroupDetailedName, vector<MeasuredFuncType>, less<GroupDetailedName> >::const_iterator	map_iter;

  BasicClassMeasurementCollection::Show_List_Of_Resources ();

  for (count_t i = 0;
               i < all_measurement_collections.size();
               i++
      )
  {
   

    // -----------------------
    // Comparison Report
    all_measurement_collections[i]->show_resource_detailed_descr(i + 1, "Performance Comparison", true);  

    count_t group_no = 0;
    for (
	  map_iter = active_comparison_groups_.begin();
	  map_iter != active_comparison_groups_.end();
	  map_iter++
	  )
    {
      group_no++;
      ostringstream osstr1;
      ostringstream osstr2;
      osstr1 << "Comparison Group#"
             << group_no
             << " of "
             << active_comparison_groups_.size()
             << " : "
             << map_iter->first.get_group_name();

      for (
	   count_t j = 0; 
	           j < all_measurement_collections[i]->resource_description_.size(); 
	           j++
	           )
      {
        osstr2 << "\t\t" 
               << "#" 
               << string (2, ' ') 
               << space1; 
        osstr2 << all_measurement_collections[i]->resource_description_[j];
        osstr2 << endl;
      }

      // -----------------------
      sout << endl;
      sout << endl;

#ifdef OPTIMIZATION_LEVEL
  sout << "\t\t" << "OPTIMIZATION_LEVEL" << endl;
#endif

      sout << "\t\t" 
	   << "#" 
	   << string (osstr1.str().size() + 1, '=') 
	   << endl;

      sout << "\t\t" 
	   << "#" 
	   << " " 
	   << osstr1.str() 
	   << endl;

      sout << "\t\t" 
	   << "#" 
           << string (osstr1.str().size() + 1, '-')
	   << endl;

      sout << osstr2.str(); 

      sout << "\t\t" 
	   << "#" 
	   << string (osstr1.str().size() + 1, '=') 
	   << endl;
      
      osstr1.str(string());
    
      osstr1 << string (3, ' ') 
             << ": " 
             << string (
		ClassResultsFunc::Get_Max_FuncName_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_FileName_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_LineNo_Len() + 3
		+ 
		ClassResultsFunc::Get_Max_DescrMsg_Len() + 4
		+
		12
		, '-'); 

      sout << osstr1.str() << endl;
      all_measurement_collections[i]->show_comparison_collection(map_iter->second);
      sout << osstr1.str() << endl;
    }
    sout << endl;
    sout << endl;

  }  // for (count_t i = 0; ...

}



// =============================
void ClassManager::set_tests (ulong tests_i) 
// =============================
{
  total_tests_ = tests_i;
}


// =============================
void ClassManager::set_discarding_threshold (double discarding_threshold_i) 
// =============================
{
  discarding_threshold_ = discarding_threshold_i;
}

// =============================
void ClassManager::set_detailed_measurement_report_flag (bool type_i) 
// =============================
{
  detailed_measurement_report_flag_ = type_i;
}


// =============================
void ClassManager::set_measurement_report_flag (bool type_i) 
// =============================
{
  measurement_report_flag_ = type_i;
} 

// =============================
void ClassManager::set_repetitions (ulong repetitions_i) 
// =============================
{
  total_repetitions_ = repetitions_i;
  if (total_repetitions_ == 0)
  {
    total_repetitions_ = TOTAL_REPETITIONS_PER_TEST_default;
  }

  while (scale_per_calls_ > total_repetitions_)
  {
    total_repetitions_ *= 10;
  }

  assert (scale_per_calls_ <= total_repetitions_);


  total_repetitions_ = (total_repetitions_/scale_per_calls_)*scale_per_calls_;

  if (repetitions_i != total_repetitions_)
  {
     string pref0 = "\t ";
     string pref1 = pref0 + ": ";
     size_t the_size = 52;
     ostringstream oss;
     oss << pref0 << string (the_size, '-') << endl;
     oss << pref1 << "\t--- WARNING! ---" << endl;
     oss << pref1 << "Requested number-of-repetitions   = " << repetitions_i << endl;
     oss << pref1 << "---> Actual number-of-repetitions = " << total_repetitions_ << endl;
     oss << pref1 << "   Info! scale-per-calls         = " << scale_per_calls_ << endl;
     oss << pref1 << "   Note! Must be : scale-per-calls <= number-of-repetitions"<< endl;
     oss << pref1 << "                 : total_repetitions_%scale_per_calls_ == 0"<< endl;
     oss << pref0 << string (the_size, '-') << endl;
     notes_.push_back (oss.str());
  }

} // ClassManager::set_repetitions



// =============================
void ClassManager::set_per_calls (ulong per_calls_i) 
// =============================
{
  scale_per_calls_ = per_calls_i;
  if (scale_per_calls_ == 0)
  {
    scale_per_calls_ = PER_CALLS_default;
  }
    
} // ClassManager::set_per_calls



// =============================
ulong ClassManager::get_scale_per_calls () const
// =============================
{
  return scale_per_calls_;
} 


// =============================
ulong ClassManager::get_total_tests () const
// =============================
{
  return total_tests_;
} 

// =============================
double ClassManager::get_discarding_threshold ()	const
// =============================
{
  return discarding_threshold_;
}


// ============================= 
double ClassManager::Get_Discarding_Threshold ()
// =============================
{
  assert (inst_counter_s == 1);
  return mng.get_discarding_threshold();
}


// =============================
bool ClassManager::Get_Tracing_Report_Flag ()
// =============================
{
  assert (inst_counter_s == 1);
  return mng.get_tracing_report_flag();
}


// =============================
bool ClassManager::Get_Demo_Flag ()
// =============================
{
  assert (inst_counter_s == 1);
  return mng.get_demo_flag();
}



// =============================
ulong ClassManager::get_current_total_repetitions () const
// =============================
{
  return total_repetitions_;
} 

// =============================
bool ClassManager::get_detailed_measurement_report_flag () const
// =============================
{
  return detailed_measurement_report_flag_;
} 


// =============================
bool ClassManager::get_measurement_report_flag () const
// =============================
{
  return measurement_report_flag_;
} 

// =============================
MeasuredFuncType ClassManager::get_cur_func_ptr () const
// =============================
{
  return cur_func_ptr_s;
} 


// -----------------------------------------
// -
// -  Constants for Parsing command line	
// -
// -----------------------------------------
static const char	ch_minus_CNS	= '-';	
static const char	ch_plus_CNS	= '+';	

static const string	log_filename_option_CNS		("-l");	
static const string	getting_groups_option_CNS	("-g");	
static const string	re_create_setup_option_CNS	("-b");	
static const string	skipping_actual_run_option_CNS	("-k");	
static const string	demo_option_CNS			("-x");	
static const string	help_option_CNS			("-h");	
static const string	settings_option_CNS		("-s");	
static const string	version_option_CNS		("-v");	

static const string	report_option_enabled_CNS	("+");	
static const string	report_option_disabled_CNS	("-");	

static const string	full_measuring_report_option_CNS	("m");	
static const string	detailed_measuring_report_option_CNS	("d");	
static const string	tracing_report_option_CNS		("t");	

static const string	set_repetitions_option_CNS		("-i");	
static const string	set_tests_option_CNS			("-e");	
static const string	set_per_calls_option_CNS		("-p");	
static const string	set_confidence_threshold_option_CNS	("-f");	


// =============================
void ClassManager::show_brief_man_description() const
// =============================
{
ostringstream oss;
const string proj_name ("--- C/C++ Program Perfometer ---");

  oss << endl
      << endl
      << space2 << string (proj_name.size(), '=') << endl
      << space2 << proj_name << endl
      << space2 << string (proj_name.size(), '=') << endl
      << endl
      << space2 << "BRIEF DESCRIPTION" << endl
      << endl
      << space4 << string (proj_name.size() - (space4.size() - space2.size()), '-')  << endl
      << space4 << "The program enables to get performance" << endl
      << space4 << "of C/C++ program and separated pieces of code for any metrics" << endl
      << space4 << "(for instance : clocks, uclocks, rusage-metrics, metrics defined by user etc.)." << endl
      << space4 << endl
      << space4 << "The measurement results are represented in detailed/summary reports." << endl
      << space4 << "The detailed report holds results of individual tests," << endl
      << space4 << "the summary report holds average cost and its analysis." << endl
      << space4 << "The comparison results are represented in comparative tables" << endl
      << space4 << "for individual comparison groups." << endl
      << space4 << endl
      << space4 << "The user may set various parameters" << endl
      << space4 << "in order to control the measurement/comparison process :" << endl
      << space4 << "* measurement report and detailed measurement report flags," << endl
      << space4 << "* total tests," << endl
      << space4 << "* total repetitions," << endl
      << space4 << "* measurement scale," << endl
      << space4 << "* confidence threshold." << endl
      << endl
      << endl

      << space4 << "User-Defined Program Files." << endl
      << space4 << "* Metrics related program files (adapt.h, adapt.cpp) :" << endl
      << space4 << "  The files contain definition and implementation of specific performance metrics (IF ANY)." << endl
      << space4 << "* Testsuite related program files (tests.h, tests.cpp, t_*.cpp) :" << endl
      << space4 << "  The files contain declaration and implementation of measured/compared functions." << endl
      << endl
      << endl
      << endl

      << space4 << "Web page :" << endl
      << space4 << "- http://alexvn.freeservers.com/s1/perfometer.html" << endl
      << endl
      << endl

      << space4 << "Download :" << endl
      << space4 << "- http://sourceforge.net/projects/cpp-perfometer" << endl
      << space4 << "- http://alexvn.freeservers.com/s1/perfometer.zip" << endl
      << space4 << "  (via http://alexvn.freeservers.com/s1/perfometer.html)" << endl
      << endl
      << endl

      << space4 << "Mailing List :" << endl
      << space4 << "- Web Page -> http://lists.sourceforge.net/lists/listinfo/cpp-perfometer-users" << endl
      << space4 << "- Posting  -> mailto:cpp-perfometer-users@lists.sourceforge.net" << endl
      << space4 << "- Archives -> http://sourceforge.net/mailarchive/forum.php?forum=cpp-perfometer-users" << endl
      << endl
      << endl

      << space4 << "Newsgroup :" << endl
      << space4 << "- News     -> news://news.gmane.org/gmane.comp.lang.c++.perfometer" << endl
      << space4 << "- Archives -> http://news.gmane.org/gmane.comp.lang.c++.perfometer" << endl
      << endl
      << endl

      << space4 << "Tests' Raw Logs :" << endl
      << space4 << "- http://groups.google.com/groups?th=fb0e11d9e7107414" << endl
      << endl
      << endl

      << space4 << "Some C/C++ Performance Tests performed using the Perfometer :" << endl
      << space4 << "- http://groups.google.com/groups?th=466338d8028d2fc6" << endl
      << space4 << "- http://article.gmane.org/gmane.comp.gcc.g++.general/101" << endl
      << space4 << "- http://mail.gnu.org/archive/html/help-gplusplus/2003-06/msg00003.html" << endl
      << endl
      << endl


      << space2 << string (proj_name.size(), '=') << endl
      << endl
      << endl
      << endl;

  sout << oss.str();
  cerr << oss.str();
}


// =============================
void ClassManager::show_help_usage(const string& exe_name_i) const
// =============================
{
ostringstream osstr;

  osstr << ""
        << "--- "
        << "USAGE : "
        << exe_name_i

        << " ["
        << log_filename_option_CNS
        << " "
        << "<log-file-name>"

        << "]"
        << " ["
        << help_option_CNS
        << "]"

        << " ["
        << version_option_CNS
        << "]"

        << " ["
        << demo_option_CNS
        << "]"

        << " ["
        << getting_groups_option_CNS
        << "]"

        << " ["
        << re_create_setup_option_CNS
        << "]"

        << " ["
        << skipping_actual_run_option_CNS
        << "]"

        << " ["
        << settings_option_CNS
        << "]"

        << " ["
        << (report_option_disabled_CNS + full_measuring_report_option_CNS)
        << "]"

        << " ["
        << (report_option_enabled_CNS + full_measuring_report_option_CNS)
        << "]"

        << " ["
        << (report_option_disabled_CNS + detailed_measuring_report_option_CNS)
        << "]"

        << " ["
        << (report_option_enabled_CNS + detailed_measuring_report_option_CNS)
        << "]"

        << " ["
        << (report_option_disabled_CNS + tracing_report_option_CNS)
        << "]"

        << " ["
        << (report_option_enabled_CNS + tracing_report_option_CNS)
        << "]"

        << " ["
        << set_repetitions_option_CNS
        << " "
        << "<number>"
        << "]"

        << " ["
        << set_tests_option_CNS
        << " "
        << "<number>"
        << "]"

        << " ["
        << set_per_calls_option_CNS
        << " "
        << "<number>"
        << "]"

        << " ["
        << set_confidence_threshold_option_CNS
        << " "
        << "<double-number>"
        << "]";

const string help_msg = osstr.str();

ostringstream oss;

  oss  << endl
       << string (help_msg.size(), '-')
       << endl
       << help_msg
       << endl
       << string (help_msg.size(), '-')
       << endl;


  sout << oss.str();
  cerr << oss.str();

} // show_help_usage



// =============================
void ClassManager::show_help_example(const string& exe_name_i) const
// =============================
{
#define	HELP_EXAMPLE_LINES	15
ostringstream	osstr [HELP_EXAMPLE_LINES];
size_t	pos = 0;

const string pref1 (4, ' ');
const string str_examples ("USAGE EXAMPLES");

  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << str_examples;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << string (str_examples.size(), '^');



  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << help_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << version_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << demo_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << getting_groups_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << re_create_setup_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << skipping_actual_run_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << settings_option_CNS;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i;


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << (report_option_disabled_CNS + full_measuring_report_option_CNS);


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << (report_option_enabled_CNS + detailed_measuring_report_option_CNS);


  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << (report_option_enabled_CNS + tracing_report_option_CNS);


const int set_repetitions_example = 75000;
const int set_tests_example = 15;
const int set_per_calls_example = 7500;
const double set_confidence_threshold_example = 0.17;
  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << set_repetitions_option_CNS
               << " "
               << set_repetitions_example
               << " "
               << set_tests_option_CNS
               << " "
               << set_tests_example
               << " "
               << set_per_calls_option_CNS
               << " "
               << set_per_calls_example
               << " "
               << set_confidence_threshold_option_CNS
               << " "
               << set_confidence_threshold_example;



const string log_filename_example ("my" + log_file_suffix_CNS);
  assert (pos < HELP_EXAMPLE_LINES);
  osstr[pos++] << ""
               << pref1
               << exe_name_i
               << " "
               << log_filename_option_CNS
               << " "
               << log_filename_example;


size_t wsize = 0;

  for (size_t i = 0; i < pos; i++)
  {
    wsize = MAX_VALUE (wsize, osstr[i].str().size());
  }

ostringstream oss;
  oss  << endl
       << string (wsize, '-')
       << endl;

  for (size_t i = 0; i < pos; i++)
  {
    oss << osstr[i].str() << endl;
  }

  oss << ""
       << string (wsize, '-')
       << endl;


  sout << oss.str();
  cerr << oss.str();

} // show_help_example



// =============================
void ClassManager::show_help_info(const string& exe_name_i) const
// =============================
{
ostringstream	osstr;

const string pref1 ("\t===> ");
const string str_infos1 ("HELP : ");
const string str_infos2 ("DEMO : ");


  osstr << ""
        << pref1
        << str_infos1
        << exe_name_i
        << " "
        << help_option_CNS;

  osstr << endl;

  osstr << ""
        << pref1
        << str_infos2
        << exe_name_i
        << " "
        << demo_option_CNS;


ostringstream oss;

  oss << osstr.str() << endl;

  sout << oss.str();
  cerr << oss.str();

} // show_help_info



// =============================
void ClassManager::show_help_options() const
// =============================
{
ostringstream osstr;
const string shift1 = "   #   ";
const string delim1 = "   : ";
const string text1 = "OPTIONS DESCRIPTION";
const string text2 = " <number>";
const string text3 = " <name>";
const string text_default = "Default";
const string text_run_time_default = "Run-time Defined Default";
const string show_delim_CNS = "/";
const string text_enabled = "ENABLED";
const string text_disabled = "DISABLED";

const size_t line1_len = 70;

  osstr.setf (ios::left, ios::adjustfield);

  osstr << ""
        << shift1
        << space9
        << text1
        << endl
        << shift1
        << space9
        << string (text1.size(), '-')
        << endl

        << shift1
        << setw (size12)
        << help_option_CNS.c_str()
        << delim1
        << " get help information"
        << endl

        << shift1
        << setw (size12)
        << version_option_CNS.c_str()
        << delim1
        << " get version information"
        << endl


        << shift1
        << setw (size12)
        << demo_option_CNS.c_str()
        << delim1
        << " run demo tests"
        << endl

        << shift1
        << setw (size12)
        << settings_option_CNS.c_str()
        << delim1
        << " get environment variables values"
        << endl


        << shift1
        << setw (size12)
        << getting_groups_option_CNS.c_str()
        << delim1
        << " get list of compared groups"
        << endl


        << shift1
        << setw (size12)
        << re_create_setup_option_CNS.c_str()
        << delim1
        << " rebuild primary setup file (list of comparison groups)"
        << endl


        << shift1
        << setw (size12)
        << skipping_actual_run_option_CNS.c_str()
        << delim1
        << " skip an actual run (get list of test cases)"
        << endl


        << shift1
        << setw (size12)
        << (report_option_disabled_CNS + show_delim_CNS + report_option_enabled_CNS + full_measuring_report_option_CNS).c_str()
        << delim1
        << " measurement report "  
        << text_disabled
        << show_delim_CNS
        << text_enabled 
        << " ("
        << text_default
        << " - "
        << text_enabled 
        << ")"
        << endl


        << shift1
        << setw (size12)
        << (report_option_disabled_CNS + show_delim_CNS + report_option_enabled_CNS + detailed_measuring_report_option_CNS).c_str()
        << delim1
        << " detailed measurement report "  
        << text_disabled
        << show_delim_CNS
        << text_enabled 
        << " ("
        << text_default
        << " - "
        << text_disabled 
        << ")"
        << endl

        << shift1
        << setw (size12)
        << ""
        << delim1
        << "   Note! If measurement report " 
        <<  text_disabled 
        << " this option ignored"
        << endl

        << shift1
        << setw (size12)
        << (report_option_disabled_CNS + show_delim_CNS + report_option_enabled_CNS + tracing_report_option_CNS).c_str()
        << delim1
        << " tracing report "  
        << text_disabled
        << show_delim_CNS
        << text_enabled 
        << " ("
        << text_default
        << " - "
        << text_disabled 
        << ")"
        << endl


        << shift1
        << setw (size12)
        << (set_repetitions_option_CNS + text2).c_str()
        << delim1
        << " set number of repetitions"
        << " ("
        << text_default
        << " = "
        << TOTAL_REPETITIONS_PER_TEST_default
        << ")"
        << endl


        << shift1
        << setw (size12)
        << (set_confidence_threshold_option_CNS + text2).c_str()
        << delim1
        << " set confidence threshold"
        << " ("
        << text_default
        << " = "
        << DISCARDING_THRESHOLD_default
        << ")"
        << endl


        << shift1
        << setw (size12)
        << (set_tests_option_CNS + text2).c_str()
        << delim1
        << " set number of tests"
        << " ("
        << text_default
        << " = "
        << TOTAL_TESTS_default
        << ")"
        << endl

        << shift1
        << setw (size12)
        << (set_per_calls_option_CNS + text2).c_str()
        << delim1
        << " set scale per calls"
        << " ("
        << text_default
        << " = "
        << PER_CALLS_default
        << ")"
        << endl


        << shift1
        << setw (size12)
        << (log_filename_option_CNS + text3).c_str()
        << delim1
        << " set log file name"
        << " ("
        << text_run_time_default
        << " = "
        << (get_exe_basename() + log_file_suffix_CNS)
        << ")"
        << endl;


  osstr << ""
        << shift1
        << string (line1_len - shift1.size(), '-')
        << endl
        << shift1
        << setw (size12)
        << "NOTE!"
        << delim1
        << " Illegal options will be ignored"
        << endl;

const string help_msg = osstr.str();

ostringstream ossaux;

  ossaux  << endl
          << string (line1_len, '=')
          << endl
          << help_msg
          << string (line1_len, '=')
          << endl;

  sout << ossaux.str();
  cerr << ossaux.str();

} // show_help_options


// =============================
void ClassManager::show_settings(string msg_i) const
// =============================
{
ostringstream osstr;

const string shift_1 = "   *   ";
const string delim_1 = "   : ";
string text_1 = "ENVIRONMENT VARIABLES";
  text_1 += " ";  
  text_1 += "(";  
  text_1 += msg_i;  
  text_1 += ")";  

const string msg_info1 = "Measurement Report";
const string msg_info2 = "Detailed Measurement Report";
const string msg_info3 = "Tracing Report";

const string msg_info4 = "Total Repetitions";
const string msg_info5 = "Total Tests";
const string msg_info6 = "Scale per Calls Value";
const string msg_info7 = "Confidence Threshold";

const string msg_enable  = "Enable";
const string msg_disable = "Disable";

const size_t line_1_len = 70;

  osstr.setf (ios::left, ios::adjustfield);

  osstr << ""
        << shift_1
        << space9
        << text_1
        << endl
        << shift_1
        << space9
        << string (text_1.size(), '-')
        << endl

        << shift_1
        << setw (size27)
        << msg_info1.c_str()
        << delim_1
        << (measurement_report_flag_ ? msg_enable : msg_disable)
        << endl;

        
        if (measurement_report_flag_)
        {
          osstr << ""
                << shift_1
                << setw (size27)
                << msg_info2.c_str()
                << delim_1
                << (detailed_measurement_report_flag_ ? msg_enable : msg_disable)
                << endl;
        }

  osstr << ""
        << shift_1
        << setw (size27)
        << msg_info3.c_str()
        << delim_1
        << (tracing_report_flag_ ? msg_enable : msg_disable)
        << endl

        << shift_1
        << setw (size27)
        << msg_info4.c_str()
        << delim_1
        << total_repetitions_
        << endl

        << shift_1
        << setw (size27)
        << msg_info5.c_str()
        << delim_1
        << total_tests_
        << endl

        << shift_1
        << setw (size27)
        << msg_info6.c_str()
        << delim_1
        << scale_per_calls_
        << endl

        << shift_1
        << setw (size27)
        << msg_info7.c_str()
        << delim_1
        << discarding_threshold_
        << endl;

const string help_msg = osstr.str();
ostringstream oss;

  oss  << endl
       << string (line_1_len, '=')
       << endl
       << help_msg
       << string (line_1_len, '=')
       << endl
       << endl
       << endl;

  sout << oss.str();
  cerr << oss.str();


} // show_settings



// =============================
void ClassManager::show_no_warranty() const
// =============================
{
ostringstream oss;

  oss << endl;
  oss << "Copyright (C) 2002-2004  Alex Vinokur" << endl;
  oss << "This is free software; see the source for copying conditions." << endl;
  oss << "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
  oss << endl;


  sout << oss.str();
  cerr << oss.str();
}


// =============================
void ClassManager::show_notes(string msg_i) const
// =============================
{
  sout << endl;
  if (!msg_i.empty())
  {
     sout << "\t" << space3 << msg_i << endl;
     sout << "\t" << string (space3.size() + msg_i.size(), '-') << endl;
  }

  for (count_t i = 0; i < notes_.size(); i++)
  {
    sout << notes_[i] << endl;
  }
  sout << endl;

} // show_notes



// =============================
bool ClassManager::parse_command_line (int argc, char** argv)
// =============================
{
bool ret_value = false;

  assert (command_line_.empty());
  read_command_line (argc, argv);

  // -----------------------------
  parse0_command_line();

  // -----------------
  open_log ();
const string logfile_info ("Log file  " + (log_filename_.empty() ? string("has not been created") : log_filename_));
  show_logo ("START : " + logfile_info);
  // -----------------

  assert (!command_line_.empty());
  show_help_info (command_line_[0]);


  // -----------------------------
  ret_value |= parse1_command_line();
const bool rc5 = parse5_command_line();
  ret_value |= rc5;
  ret_value |= parse4_command_line();

  // -----------------------------
  //show_settings("Default Values");
  // -----------------------------

  sout << endl;
  ret_value |= parse2_command_line();
  ret_value |= parse3_command_line();
  parse6_command_line();

  // -----------------------------
  show_command_line();
  show_notes ();
  if (!rc5) show_settings ("Actual Values");
  // -----------------------------

  // -----------------------------
  if (reading_setup_file_content_ && !rebuilding_setup_flag_) read_setup_file_content ();

  // -----------------------------
  if (rc5) show_no_warranty();

  // -----------------------------
  return ret_value;

} // ClassManager::parse_command_line()


// =============================
string ClassManager::get_exe_basename () const
// =============================
{
  assert (!command_line_[0].empty());

string name (command_line_[0]);

const size_t ind1 = name.find_last_of('/');
  if (ind1 != string::npos) name = name.substr (ind1 + 1);

const size_t ind2 = name.find_last_of('.');
  if (ind2 != string::npos) name = name.substr (0, ind2);

  return name;
}


// =============================
void ClassManager::parse0_command_line ()
// =============================
{
bool flag = false;
const string pref1 ("\t---> ");
const string str1_logfile ("Missing Log File Name");
const string str2_logfile ("Default Log File Name Used");

  for (count_t i = 0; i < command_line_.size(); i++)
  {
    // --------------------------------
    if (command_line_[i] == log_filename_option_CNS)
    {
      if (command_line_.size() > (i + 1))
      {
        if (!(
             (command_line_[i + 1][0] == ch_minus_CNS) 
             || 
             (command_line_[i + 1][0] == ch_plus_CNS) 
           ))
        {
          set_log_filename (command_line_[i + 1]);
          flag = true;
        }
      }
      
      if (!flag)
      {
        sout << endl;
        sout << endl
             << pref1
             << str1_logfile
             << endl
             << pref1
             << str2_logfile
             << endl;
      }

      break;

    }  // if (command_line_[i] == log_filename_option_CNS)

  } // for (count_t i = 0; i < command_line_.size(); i++)

  // --------------------
  if (!flag)
  {
    set_log_filename (get_exe_basename() + log_file_suffix_CNS);
  }

  set_setup_filename (get_exe_basename() + setup_file_suffix_CNS);

} // ClassManager::parse0_command_line()


// =============================
bool ClassManager::parse1_command_line ()
// =============================
{
bool	ret_value = (find (
			command_line_.begin(), 
			command_line_.end(), 
			help_option_CNS
			) 
			!= command_line_.end()
			);

  if (ret_value)
  {
    reading_setup_file_content_ = false;;

    show_brief_man_description();
    show_help_usage(command_line_[0]);
    show_help_example(command_line_[0]);
    show_help_options();
  }

  return ret_value;

} // ClassManager::parse1_command_line()



// =============================
bool ClassManager::parse2_command_line ()
// =============================
{

  for (count_t i = 0; i < command_line_.size(); i++)
  {
    // --------------------------------
    if (command_line_[i] == (report_option_disabled_CNS + full_measuring_report_option_CNS))
    {
      set_measurement_report_flag (false);
    }

    // --------------------------------
    if (command_line_[i] == (report_option_enabled_CNS + full_measuring_report_option_CNS))
    {
      set_measurement_report_flag (true);
    }


    // --------------------------------
    if (command_line_[i] == (report_option_disabled_CNS + detailed_measuring_report_option_CNS))
    {
      set_detailed_measurement_report_flag (false);
    }

    // --------------------------------
    if (command_line_[i] == (report_option_enabled_CNS + detailed_measuring_report_option_CNS))
    {
      set_detailed_measurement_report_flag (true);
    }


    // --------------------------------
    if (command_line_[i] == (report_option_disabled_CNS + tracing_report_option_CNS))
    {
      set_tracing_report_flag (false);
    }

    // --------------------------------
    if (command_line_[i] == (report_option_enabled_CNS + tracing_report_option_CNS))
    {
      set_tracing_report_flag (true);
    }


    // --------------------------------
    if (command_line_[i] == set_per_calls_option_CNS)
    {
      bool flag = false;
      if (command_line_.size() > (i + 1))
      {
        ulong entered_value;
	if (string_to (command_line_[i + 1], entered_value))
        {
          set_per_calls (entered_value);
          flag = true;
        }
      }
      
      if (!flag)
      {
        sout << "---> Desired Number of Scale-Per-Calls Value is Illegal or Not Defined "
             << endl
             << "---> Option "
             << set_per_calls_option_CNS
             << " irnored"
             << endl
             << "---> Used Default Value =  "
             << get_scale_per_calls () 
             << endl;
      }
    }  // if (command_line_[i] == set_per_calls_option_CNS)



    // --------------------------------
    if (command_line_[i] == set_tests_option_CNS)
    {
      bool flag = false;
      if (command_line_.size() > (i + 1))
      {
        ulong entered_value;
	if (string_to (command_line_[i + 1], entered_value))
        {
          set_tests (entered_value);
          flag = true;
        }
      }
      
      if (!flag)
      {
        sout << "---> Desired Number of Tests is Illegal or Not Defined "
             << endl
             << "---> Option "
             << set_tests_option_CNS
             << " irnored"
             << endl
             << "---> Used Default Value =  "
             << get_total_tests ()
             << endl;
      }

    } // if (command_line_[i] == set_tests_option_CNS)

    // --------------------------------
    if (command_line_[i] == set_confidence_threshold_option_CNS)
    {
      bool flag = false;
      if (command_line_.size() > (i + 1))
      {
        double entered_value;
	if (string_to (command_line_[i + 1], entered_value))
        {
          set_discarding_threshold (entered_value);
          flag = true;
        }
      }
      
      if (!flag)
      {
        sout << "---> Desired Number of Tests is Illegal or Not Defined "
             << endl
             << "---> Option "
             << set_confidence_threshold_option_CNS
             << " irnored"
             << endl
             << "---> Used Default Value =  "
             << get_discarding_threshold ()
             << endl;
      }

 
    } // if (command_line_[i] == set_confidence_threshold_option_CNS)


  } // for (count_t i = 0; i < command_line_.size(); i++)


  return false;

} // ClassManager::parse2_command_line()



// =============================
bool ClassManager::parse3_command_line ()
// =============================
{
  
  for (count_t i = 0; i < command_line_.size(); i++)
  {
    // --------------------------------
    if (command_line_[i] == set_repetitions_option_CNS)
    {
      bool flag = false;
      if (command_line_.size() > (i + 1))
      {
        ulong entered_value;
	if (string_to (command_line_[i + 1], entered_value))
        {
          set_repetitions (entered_value);
          flag = true;
        }
      }
      
      if (!flag)
      {
        sout << "---> Desired Number of Repetitions is Illegal or Not Defined "
             << endl
             << "---> Option "
             << set_repetitions_option_CNS
             << " irnored"
             << endl
             << "---> Used Default Value =  "
             << get_current_total_repetitions ()
             << endl;
      }
    } // if (command_line_[i] == set_repetitions_option_CNS)

  } // for (count_t i = 0; i < command_line_.size(); i++)


  return false;

} // ClassManager::parse3_command_line()


// =============================
bool ClassManager::parse4_command_line ()
// =============================
{
bool	ret_value = (find (
			command_line_.begin(), 
			command_line_.end(), 
			settings_option_CNS
			) 
			!= command_line_.end()
			);

  if (ret_value)
  {
    reading_setup_file_content_ = false;
  }

  return ret_value;

} // ClassManager::parse4_command_line()


// =============================
bool ClassManager::parse5_command_line ()
// =============================
{
bool	ret_value = (find (
			command_line_.begin(), 
			command_line_.end(), 
			version_option_CNS
			) 
			!= command_line_.end()
			);

  if (ret_value)
  {
    reading_setup_file_content_ = false;;

    show_version();
  }

  return ret_value;

} // ClassManager::parse5_command_line()



// =============================
void ClassManager::parse6_command_line ()
// =============================
{
bool flag;

  // -----------------------------
  flag = (find (
		command_line_.begin(), 
		command_line_.end(), 
		demo_option_CNS
		) 
		!= command_line_.end()
	 );

  if (flag) 
  {
    demo_flag_ = true;
    reading_setup_file_content_ = false;
    all_groups_are_active_ = true;

  }

  // -----------------------------
  flag = (find (
		command_line_.begin(), 
		command_line_.end(), 
		getting_groups_option_CNS
		) 
		!= command_line_.end()
	 );

  if (flag) performing_flag_ = false;

  // -----------------------------
  flag = (find (
		command_line_.begin(), 
		command_line_.end(), 
		re_create_setup_option_CNS
		) 
		!= command_line_.end()
	 );

  if (flag) rebuilding_setup_flag_ = true;


  // -----------------------------
  flag = (find (
		command_line_.begin(), 
		command_line_.end(), 
		skipping_actual_run_option_CNS
		) 
		!= command_line_.end()
	 );

  if (flag)
  {
    set_per_calls (1);	// Must be before set_repetitions()
    set_repetitions (1);
    set_tests (1);
    set_measurement_report_flag (false);
  }


} // ClassManager::parse6_command_line()


// =============================
void ClassManager::show_command_line()
// =============================
{
  sout << endl;
  sout << endl;

#ifdef OPTIMIZATION_LEVEL
  sout << "\t---> COMPILATION       : " << "OPTIMIZATION_LEVEL" << endl;
#endif

  sout << "\t---> YOUR COMMAND LINE : ";
  copy (
	command_line_.begin(), 
	command_line_.end(), 
	ostream_iterator<string> (sout, " ")
	);

  sout << endl;
  sout << endl;
} // ClassManager::show_command_line



// =============================
void ClassManager::set_tracing_report_flag (bool format_i)
// =============================
{
  tracing_report_flag_ = format_i;
} 

// =============================
bool ClassManager::get_tracing_report_flag () const
// =============================
{
  return tracing_report_flag_;
} 


// =============================
bool ClassManager::get_demo_flag () const
// =============================
{
  return demo_flag_;
} 



// =============================
void ClassManager::read_command_line (int argc, char** argv)
// =============================
{
  assert (command_line_.empty());
  for (int i = 0; i < argc; i++)
  {
    command_line_.push_back (argv[i]);
  }

const string	slash ("/\\");
const string	the_exe_name (command_line_[0]);
string::size_type	pos = the_exe_name.find_last_of (slash);

  if (!(pos == string::npos))
  {
    command_line_[0] = the_exe_name.substr(pos + 1);
  }


} 

#if (defined UNIX_ENV) 
#include <sys/utsname.h>
#endif

// =============================
void ClassManager::show_logo (string msg_i)
// =============================
{
const time_t timer = time(NULL);

const string pref1 ("\t#");
const string pref2 (pref1 + " ");

const string sw_version (string ("Version ") + string (sw_tool_version) + "-" + string(sw_testsuites_version));

vector<string> product_urls;
  product_urls.push_back (product_url_1); 
  product_urls.push_back (product_url_2); 
  product_urls.push_back (product_url_3); 
  product_urls.push_back (product_url_4); 

const size_t url_no = timer%product_urls.size();
  assert (url_no < product_urls.size());


vector<string>	product_info;
  product_info.push_back (product_name);
  product_info.push_back (string (2, ' ') + product_urls[url_no]);
  product_info.push_back (string (2, ' ') + sw_version);
  product_info.push_back (string (2, ' ') + string (sw_version.size(), '-'));
  product_info.push_back (string (2, ' ') + author_name);
  product_info.push_back (string (2, ' ') + author_url);
  product_info.push_back (string (2, ' ') + author_email);

vector<vector<string> >	system_info;

const string system_name (get_system_name());
  if (!system_name.empty())
  {
    system_info.push_back(vector<string>());
    system_info.rbegin()->push_back (pref2);
    system_info.rbegin()->push_back (system_name);
  }

#if (defined UNIX_ENV) 
struct utsname inst_utsname;
  uname (&inst_utsname);
  system_info.push_back(vector<string>());
  system_info.rbegin()->push_back (pref2);
  system_info.rbegin()->push_back (to_string(inst_utsname.sysname));
  system_info.rbegin()->push_back (" ");
  system_info.rbegin()->push_back (to_string(inst_utsname.release));
  system_info.rbegin()->push_back (" ");
  system_info.rbegin()->push_back (to_string(inst_utsname.machine));
#endif  


const string compiler_name (get_compiler_name());
  if (!compiler_name.empty())
  {
    system_info.push_back(vector<string>());
    system_info.rbegin()->push_back (pref2);
    system_info.rbegin()->push_back (compiler_name);
  }

// -------------------
size_t the_width = 0;
  for (size_t i = 0; i < product_info.size(); i++)
  {
    the_width = MAX_VALUE (the_width, product_info[i].size());
  }
  
  sout << endl;
  sout << pref1 << string (the_width + 1, '=') << endl;
  for (size_t i = 0; i < product_info.size(); i++)
  {
    sout << pref2 << product_info[i] << endl;
  }

  if (!system_info.empty())
  {
    sout << pref1 << string (the_width + 1, '-') << endl;
  }
  for (size_t i = 0; i < system_info.size(); i++)
  {
    copy (system_info[i].begin(), system_info[i].end(), ostream_iterator<string> (sout, ""));
    sout << endl;
  }
  sout << pref1 << string (the_width + 1, '-') << endl;

  sout << pref2 << msg_i << endl;
  sout << pref2 << asctime (localtime(&timer));
  sout << pref1 << string (the_width + 1, '=') << endl;
  sout << endl;

} // show_logo



// =============================
void ClassManager::show_version(string msg_i) const
// =============================
{
ostringstream oss;

const string	pref1 ("\t===> ");
size_t		size1 = 0;		
  oss << endl;
  if (!msg_i.empty())
  {
     oss << "\t" << space3 << msg_i << endl;
     oss << "\t" << string (space3.size() + msg_i.size(), '-') << endl;
  }
  

const string sw_version (string ("Version ") + string (sw_tool_version) + "-" + string(sw_testsuites_version));
  size1 = MAX_VALUE (size1, sw_version.size());
  oss << pref1 << string (size1, '#') << endl;
  oss << pref1 << sw_version << endl;
  oss << pref1 << string (size1, '#') << endl;

  oss << endl;
  oss << endl;


  sout << oss.str();
  cerr << oss.str();

} // show_version




// =============================
void ClassManager::delete_it ()
// =============================
{
vector<BasicClassMeasurementCollection*> &all_that = 
		BasicClassMeasurementCollection::all_measurement_collections_s;

  for (count_t i = 0; i < all_that.size(); i++)
  {
    delete all_that[i];
  }
  all_that.clear();
  assert (all_that.empty());
}



// =============================
bool ClassManager::Read_File_Content (const string& file_name_i, vector<string>& vect_o)
// =============================
{
  vect_o.clear();

ifstream fin (file_name_i.c_str ());

const string pref1 ("\t===> ");
ostringstream oss;

  // ------------------------------------
  if (!fin)
  {
    oss << ""
        << pref1
        << "Cannot open file "
        << file_name_i
        << " for reading"
        << endl;

    sout << oss.str();
    cerr << oss.str();

    return false;
  }
  assert (fin);
  assert (fin.is_open());

  // --------------------------
char	ch;
string	line;
  while ( fin.get (ch) ) 
  {
    if (ch == '\n')
    {
      vect_o.push_back (line);
      line.erase();
    }
    else
    {
      line += ch;
    }
  }


  // ----------------------
  if (vect_o.empty())
  {
    oss << ""
        << pref1
        << "File  "
        << file_name_i
        << "  is empty"
        << endl;

    sout << oss.str();
    cerr << oss.str();
  }

  fin.close();
  assert (!fin.is_open());

  return true;

} // Read_File_Content


// =============================
void ClassManager::read_setup_file_content ()
// =============================
{
  assert (selected_groups_.empty());

vector<string> vect;

  if (!Read_File_Content (setup_filename_, vect)) return;

  for (size_t i = 0; i < vect.size(); i++) 
  {
    if (vect[i].empty()) continue;
    if (vect[i][0] == SETUP_FILE_COMMENT_SIGN) continue;

    selected_groups_.push_back (CutName (vect[i]));
  }

  // --------------------

  if (selected_groups_.empty())
  {
    ostringstream oss;
    oss << endl
        << "\t--- WARNING! No comparison groups selected to be tested "; 
    if (!setup_filename_.empty())
    {
      oss << endl
          << "\t--- -------- You should set active comparison groups" 
          << endl
          << "\t--- -------- in Active Group Setup file  " 
          << setup_filename_;
    }

    oss << endl
        << endl;

    cerr << oss.str();
    sout << oss.str();

  }

} // read_setup_file_content


// =============================
bool ClassManager::is_comparison_group_name(const string& name_i) const
// =============================
{
  return  (active_comparison_groups_.find(
  		GroupDetailedName (
  			name_i,
  			string(),  // never mind because of operator< for GroupDetailedName
  			0	   // never mind because of operator< for GroupDetailedName
  			)
  		) != active_comparison_groups_.end()
  		);
}


// =============================
bool ClassManager::is_active_comparison_group(const string& name_i) const
// =============================
{
  if (all_groups_are_active_) return true;

  return (find (
		selected_groups_.begin(),
		selected_groups_.end(),
		name_i
		)
		!= selected_groups_.end()
	);
}


// =============================
void ClassManager::check_selected_groups ()
// =============================
{
vector<string>::iterator iter;

bool	check_next_group = true;

  while	(check_next_group)
  {
    check_next_group = false;
    for (iter = selected_groups_.begin(); 
         iter != selected_groups_.end();
         iter++)
    {
  
      if (!is_comparison_group_name(*iter))
      {
  
        ostringstream oss;
        oss << endl
      
            << "\t--- WARNING! Group name " 
            << "<"
            << *iter
            << ">"
            << endl
   
            << "\t             contained in Active Group Setup file "
            << setup_filename_
            << endl
  
            << "\t             is illegal"
            << endl
      
            << "\t--- -------- The selected group rejected" 
            << endl
            << endl;
      
         cerr << oss.str();
         sout << oss.str();
  

        // -------------------------
        selected_groups_.erase(iter);
        check_next_group = true;
        break;
        // -------------------------
      }

    } // for (iter = ...


  } // while



} // check_selected_groups



// =============================
bool ClassManager::setup_file_is_availble () const
// =============================
{
  assert (!setup_filename_.empty());

ifstream fin (setup_filename_.c_str());
bool ret_value = (fin);
  fin.close();
  return ret_value;
}


// =============================
void ClassManager::create_primary_setup_file () const
// =============================
{
  assert (selected_groups_.empty());

ofstream fout (setup_filename_.c_str());

  if ((!fout) || (fout && !fout.is_open()))
  {
    cerr << endl;
    cerr << "\t--- FATAL ERROR! Unable to open Active Group Setup file " << setup_filename_ << endl;
    cerr << endl;
 
    return;
  }


  assert (fout);  
  assert (fout.is_open());  


size_t max_len = 0;
const string head_line0 (
		string (product_name) 
		+ " (Version " + 
		string (sw_tool_version)
		+
		"-"
		+
		string (sw_testsuites_version)
		+
		")"
		);

const string head_line1 (setup_filename_ + " : Setup file of Active Comparison Groups");
const string head_line2 ("To make some group be active");
const string head_line3 ("   one should remove the \'#\' symbol in relevant row");

  max_len = MAX_VALUE (max_len, head_line0.size());
  max_len = MAX_VALUE (max_len, head_line1.size());
  max_len = MAX_VALUE (max_len, head_line2.size());
  max_len = MAX_VALUE (max_len, head_line3.size());


const string shift1 (7, ' ');
  fout << ""

       << SETUP_FILE_COMMENT_SIGN
       << " "
       << string (max_len + shift1.size() - 1, '#')
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << head_line0
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << string (head_line0.size(), '=')
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << head_line1
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << string (head_line1.size(), '-')
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << head_line2
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << shift1
       << head_line3
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << endl

       << SETUP_FILE_COMMENT_SIGN
       << " "
       << string (max_len + shift1.size() - 1, '#')
       << endl;

  fout << endl;
  fout << endl;


  // -------------------------------
map<string, map<string, vector<size_t> > >::const_iterator citer;

  for (citer = all_comparison_groups_.begin();
       citer != all_comparison_groups_.end();
       citer++
       )
  {
    fout << SETUP_FILE_COMMENT_SIGN
         << " "
         << citer->first 
         << endl;
  }

  // --------
  fout.close();

  // --------

const string pref1 ("\t===> ");
const string pref2 ("\t     ");
ostringstream oss;
  oss << ""
      << pref1
      << "Primary Active Group Setup file  "
      << setup_filename_
      << "  has been created"
      << endl;
  oss << ""
      << pref1
      << "Now you should set active comparison groups" 
      << endl
      << pref2
      << "in the Active Group Setup file" 
      << endl;

  sout << oss.str();
  cerr << oss.str();

} // create_primary_setup_file

