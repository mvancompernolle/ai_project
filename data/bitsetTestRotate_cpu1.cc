#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTROTATE
#include "bitsetTestShiftRotate.h"

void bitsetTest::testRotate_cpu1(void)
{
#ifndef FASTTEST
#define SHIFTROTATEBITS 13
#include "bitsetTestShiftRotate.hcc"
#endif
}
