#include <iostream>
#include <iomanip>
#include <limits>
#include <more/math/math.h>
#include <more/diag/debug.h>

//  bitpow(x, n) with negtive n is nonsense unless x _exactly_
//  representible in a double.  In particular it can be used for
//  positive numbers, giving fractions as result, but we choose
//  to return NaN if n < 0 for now.

int main() {
    using more::math::bitpow;
    std::cerr.precision(std::numeric_limits<double>::digits);
    MORE_SHOW(bitpow(0.5, 2));
    MORE_SHOW(bitpow(0.5, -2));
    MORE_SHOW(bitpow(2.0, 2));
    MORE_SHOW(bitpow(2.0, -2));
    MORE_SHOW(bitpow(bitpow(12.25, 2), 3));
    MORE_SHOW(bitpow(bitpow(12.25, -2), -3));
    MORE_SHOW(bitpow(bitpow(12.33, 2), 3));
    MORE_SHOW(bitpow(bitpow(12.33, -2), -3));
    return 0;
}
