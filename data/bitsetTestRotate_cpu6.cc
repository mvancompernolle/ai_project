#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTROTATE
#include "bitsetTestShiftRotate.h"

void bitsetTest::testRotate_cpu6(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS (3 * ECC_BITS + 13)
#include "bitsetTestShiftRotate.hcc"
#endif
}
