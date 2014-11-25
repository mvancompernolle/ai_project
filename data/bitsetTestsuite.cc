#include "sys.h"
#include "debug.h"
#include <libecc/bitset.h>
#include <cppunit/extensions/HelperMacros.h>
#include "bitsetTest.h"

#if ECC_BITS != 32 && ECC_BITS != 64
#error Missing tests.
#endif

size_t const bitsetTest::digit_size;
size_t const bitsetTest::digit_bits;

char const* const bitsetTest::b7v3_str		=                                                         "05";
char const* const bitsetTest::b7v3_bare		=                                                          "5";
char const* const bitsetTest::b7v3_fit		=                                                         "85";
char const* const bitsetTest::b7v3_excess	=                                                        "F85";

#if ECC_BITS == 32
char const* const bitsetTest::bDSv3_str		=                                                   "00000005";
char const* const bitsetTest::bDSv3_bare	=                                                          "5";
char const* const bitsetTest::bDSv3_excess1	=                                                 " F00000005";
char const* const bitsetTest::bDSv3_excess2	=                                                 "F 00000005";
char const* const bitsetTest::bDSv3_excess3	=                                                "F F00000005";
char const* const bitsetTest::bDSv3_excess4	=                                     "FFFFFFFFFFFFF 00000005";
#elif ECC_BITS == 64
char const* const bitsetTest::bDSv3_str		=                                           "0000000000000005";
char const* const bitsetTest::bDSv3_bare	=                                                          "5";
char const* const bitsetTest::bDSv3_excess1	=                                         " F0000000000000005";
char const* const bitsetTest::bDSv3_excess2	=                                         "F 0000000000000005";
char const* const bitsetTest::bDSv3_excess3	=                                        "F F0000000000000005";
char const* const bitsetTest::bDSv3_excess4	=                     "FFFFFFFFFFFFFFFFFFFFF 0000000000000005";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::bDSvDS_str	=                                                   "D23DAE05";
char const* const bitsetTest::bDSvDS_excess1	=                                                  "1D23DAE05";
char const* const bitsetTest::bDSvDS_excess2	=                                                 "1 D23DAE05";
char const* const bitsetTest::bDSvDS_excess3	=                                     "FFFFFFFFFFFFF D23DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::bDSvDS_str	=                                           "C5F190E7D23DAE05";
char const* const bitsetTest::bDSvDS_excess1	=                                          "1C5F190E7D23DAE05";
char const* const bitsetTest::bDSvDS_excess2	=                                         "1 C5F190E7D23DAE05";
char const* const bitsetTest::bDSvDS_excess3	=                     "FFFFFFFFFFFFFFFFFFFFF C5F190E7D23DAE05";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::bDSp7vDS_str	=                                                "00 D23DAE05";
char const* const bitsetTest::bDSp7vDS_bare	=                                                   "D23DAE05";
char const* const bitsetTest::bDSp7vDS_fit	=                                                "80 D23DAE05";
char const* const bitsetTest::bDSp7vDS_excess1	=                                               "180 D23DAE05";
char const* const bitsetTest::bDSp7vDS_excess2	=                                 "FFFFFFFF FFFFF180 D23DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::bDSp7vDS_str	=                                        "00 C5F190E7D23DAE05";
char const* const bitsetTest::bDSp7vDS_bare	=                                           "C5F190E7D23DAE05";
char const* const bitsetTest::bDSp7vDS_fit	=                                        "80 C5F190E7D23DAE05";
char const* const bitsetTest::bDSp7vDS_excess1	=                                       "180 C5F190E7D23DAE05";
char const* const bitsetTest::bDSp7vDS_excess2	=         "FFFFFFFFFFFFFFFF FFFFFFFFFFFFF180 C5F190E7D23DAE05";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b2DSv2DSm1_str		=                                          "423DAE05 F8A013C7";
char const* const bitsetTest::b2DSv2DSm1_excess1	=               "0000FFFF FFFFFFFF FFFFFFFF 423DAE05 F8A013C7";
char const* const bitsetTest::b2DSv2DSm1_excess2	=      "      0000FFFFFFFFFFFF  FFFFFFFF  423DAE  05F8A01 3C7";
#elif ECC_BITS == 64
char const* const bitsetTest::b2DSv2DSm1_str		=                                          "423DAE05CC2189E2 F8A013C76B03A127";
char const* const bitsetTest::b2DSv2DSm1_excess1	=               "0000FFFF FFFFFFFF FFFFFFFF 423DAE05CC2189E2 F8A013C76B03A127";
char const* const bitsetTest::b2DSv2DSm1_excess2	=      "      0000FFFFFFFFFFFF  FFFFFFFF  423DAE  05CC2189E2F8A01 3C76B03A127";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b2DSv2DS_str		=                                          "F8A013C7 423DAE05";
char const* const bitsetTest::b2DSv2DS_excess		=                                        "1 F8A013C7 423DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::b2DSv2DS_str		=                                          "F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b2DSv2DS_excess		=                                        "1 F8A013C76B03A127 423DAE05CC2189E2";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b2DSp1v2DSp1_str		=                                        "1 F8A013C7 423DAE05";
char const* const bitsetTest::b2DSp1v2DSp1_fit		=                                        "3 F8A013C7 423DAE05";
char const* const bitsetTest::b2DSp1v2DSp1_excess	=                        "0000FFFF FFFFFFFF F8A013C7 423DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::b2DSp1v2DSp1_str		=                                        "1 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b2DSp1v2DSp1_fit		=                                        "3 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b2DSp1v2DSp1_excess	=        "00000000FFFFFFFF FFFFFFFFFFFFFFFF F8A013C76B03A127 423DAE05CC2189E2";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b6DSp3v6DSp3_str	=    "5 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
char const* const bitsetTest::b6DSp3v6DSp3_fit	=    "D FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::b6DSp3v6DSp3_str	=    "5 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b6DSp3v6DSp3_fit	=    "D FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b6DSp13v6DSp3_str		= "0005 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
char const* const bitsetTest::b6DSp13v6DSp3_bare	=    "5 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
char const* const bitsetTest::b6DSp13v6DSp3_fit		= "2005 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::b6DSp13v6DSp3_str		= "0005 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b6DSp13v6DSp3_bare	=    "5 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b6DSp13v6DSp3_fit		= "2005 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
#endif

#if ECC_BITS == 32
char const* const bitsetTest::b6DSp13v6DSp13_str	= "19C5 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
char const* const bitsetTest::b6DSp13v6DSp13_fit	= "39C5 FFFFAAAA 5555FFFF 00005555 FFFF0000 F8A013C7 423DAE05";
#elif ECC_BITS == 64
char const* const bitsetTest::b6DSp13v6DSp13_str	= "19C5 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
char const* const bitsetTest::b6DSp13v6DSp13_fit	= "39C5 FFFFFFFFAAAAAAAA 55555555FFFFFFFF 0000000055555555 FFFFFFFF00000000 F8A013C76B03A127 423DAE05CC2189E2";
#endif

char const* const bitsetTest::b_str[] = {
    b7v3_str, bDSv3_str, bDSvDS_str, bDSp7vDS_str, b2DSv2DS_str, b2DSv2DSm1_str, b2DSp1v2DSp1_str, b6DSp3v6DSp3_str, b6DSp13v6DSp3_str, b6DSp13v6DSp13_str
};

// 0110010110011010 = 659A
// 0011001100111100 = 333C
// -----------------------------
// 0010000100011000 = 2118   AND
// 0111011110111110 =        OR
// 0101011010100110 =        XOR
// -----------------------------
// 1111010000001011 = F40B
char const* const bitsetTest::x_str = "659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A659A";
char const* const bitsetTest::y_str = "333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C333C";
char const* const bitsetTest::z_str = "F40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40BF40B";
char const* const bitsetTest::x_AND_y_13_str = "118";

#if ECC_BITS == 32
char const* const bitsetTest::x_AND_y_DS_str = "21182118";
char const* const bitsetTest::x_AND_y_DSp13_str = "11821182118";
char const* const bitsetTest::x_AND_y_2DS_str = "2118211821182118";
char const* const bitsetTest::x_AND_y_2DSp13_str = "1182118211821182118";
char const* const bitsetTest::x_AND_y_3DS_str = "211821182118211821182118";
char const* const bitsetTest::x_AND_y_3DSp13_str = "118211821182118211821182118";
char const* const bitsetTest::x_AND_y_4DS_str = "21182118211821182118211821182118";
#elif ECC_BITS == 64
char const* const bitsetTest::x_AND_y_DS_str = "2118211821182118";
char const* const bitsetTest::x_AND_y_DSp13_str = "1182118211821182118";
char const* const bitsetTest::x_AND_y_2DS_str = "21182118211821182118211821182118";
char const* const bitsetTest::x_AND_y_2DSp13_str = "11821182118211821182118211821182118";
char const* const bitsetTest::x_AND_y_3DS_str = "211821182118211821182118211821182118211821182118";
char const* const bitsetTest::x_AND_y_3DSp13_str = "118211821182118211821182118211821182118211821182118";
char const* const bitsetTest::x_AND_y_4DS_str = "2118211821182118211821182118211821182118211821182118211821182118";
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( bitsetTest );
