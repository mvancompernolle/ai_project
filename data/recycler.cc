#include <more/gen/recycler.h>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace more {
namespace gen {

  bool opt_recycler_do_log = false;

  recycler_group::~recycler_group()
  {
      if (recyclers.size()) {
	  throw std::logic_error("more::recycler_group: There are "
				 "still recyclers in this group.");
      }
  }

  void
  recycler_group::collect()
  {
//       if (opt_recycler_do_log)
// 	  std::clog << "more::recycler: Collecting." << std::endl;
      std::for_each(recyclers.begin(), recyclers.end(),
		    adapt_with_ptr(&recycler_base::clear_marks));
//       if (opt_recycler_do_log)
// 	  std::clog << "more::recycler: Calling markers." << std::endl;
      mark_hook.run();
  }

  recycler_base::~recycler_base()
  {
      group->recyclers.erase(std::find(group->recyclers.begin(),
				       group->recyclers.end(), this));
  }

}}
