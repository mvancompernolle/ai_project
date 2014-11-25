#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include "bitsetTestTriop.h"

void bitsetTest::testAndAssignInvertOrInvert(void)
{
  TESTTRIOPLEVEL2(&=, |, ~, ~);
}
