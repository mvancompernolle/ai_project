#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include "bitsetTestTriop.h"

void bitsetTest::testAssignXOrInvert(void)
{
  TESTTRIOPLEVEL2(=, ^, , ~);
}
