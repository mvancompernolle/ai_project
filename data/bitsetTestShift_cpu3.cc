#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTSHIFT
#include "bitsetTestShiftRotate.h"

void bitsetTest::testShift_cpu3(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS (2 * ECC_BITS)
#include "bitsetTestShiftRotate.hcc"
#endif
}
