#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTSHIFT
#include "bitsetTestShiftRotate.h"

void bitsetTest::testShift_cpu1(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS 13
#include "bitsetTestShiftRotate.hcc"
#endif
}
