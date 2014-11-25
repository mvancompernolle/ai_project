#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include "bitsetTestTriop.h"

void bitsetTest::testXorAssignOrInvert(void)
{
  TESTTRIOPLEVEL2(^=, |, , ~);
}
