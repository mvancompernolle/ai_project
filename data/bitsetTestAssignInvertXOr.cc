#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include "bitsetTestTriop.h"

void bitsetTest::testAssignInvertXOr(void)
{
  TESTTRIOPLEVEL2(=, ^, ~, );
}
