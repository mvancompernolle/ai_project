#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTSHIFT
#include "bitsetTestShiftRotate.h"

void bitsetTest::testShift_cpu5(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS (3 * ECC_BITS + 15)
#include "bitsetTestShiftRotate.hcc"
#endif
}
