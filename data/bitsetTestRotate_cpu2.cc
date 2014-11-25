#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTROTATE
#include "bitsetTestShiftRotate.h"

void bitsetTest::testRotate_cpu2(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS ECC_BITS
#include "bitsetTestShiftRotate.hcc"
#endif
}
