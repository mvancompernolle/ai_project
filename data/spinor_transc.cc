#include <more/math/math.h>
#include <more/math/spinor.h>

#define FUNC sin
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC cos
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC tan
#include "spinor_transc_templ.cc"
#undef FUNC

#if 0
#define FUNC acos
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC asin
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC atan
#include "spinor_transc_templ.cc"
#undef FUNC
#endif

#define FUNC sinh
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC cosh
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC tanh
#include "spinor_transc_templ.cc"
#undef FUNC

#define FUNC exp
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC log
#include "spinor_transc_templ.cc"
#undef FUNC
#define FUNC sqrt
#include "spinor_transc_templ.cc"
#undef FUNC

// div, log10 skipped
// Non-analytic functions skipped: abs, fabs, floor, ceil
// Two argument functions skipped: pow, fmod, frexp, ldexp
