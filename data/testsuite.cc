#include "sys.h"
#include "debug.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(void)
{
  Debug( libcw_do.set_ostream(&std::cout) );
  Debug( libcw_do.on() );
  Debug( read_rcfile() );
  std::ios::sync_with_stdio(false);
  Debug( check_configuration() );
#if CWDEBUG_ALLOC
  libcwd::make_all_allocations_invisible_except(NULL);
#endif
  //Debug( dc::notice.on() );
  //Debug( dc::bitsetshift.on() );
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  runner.run();
  return 0;
}

