#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTSHIFT
#include "bitsetTestShiftRotate.h"

void bitsetTest::testShift_cpu2(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS ECC_BITS
#include "bitsetTestShiftRotate.hcc"
#endif
}
