#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTROTATE
#include "bitsetTestShiftRotate.h"

void bitsetTest::testRotate_cpu4(void)
{
#define SHIFTROTATEBITS (2 * ECC_BITS + 13)
#include "bitsetTestShiftRotate.hcc"
}
