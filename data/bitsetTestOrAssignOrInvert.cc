#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include "bitsetTestTriop.h"

void bitsetTest::testOrAssignOrInvert(void)
{
  TESTTRIOPLEVEL2(|=, |, , ~);
}
