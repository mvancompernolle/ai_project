#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#define TESTSHIFT
#include "bitsetTestShiftRotate.h"

void bitsetTest::testShift_cpu4(void)
{
#define SHIFTROTATEBITS (2 * ECC_BITS + 23)
#include "bitsetTestShiftRotate.hcc"
}
