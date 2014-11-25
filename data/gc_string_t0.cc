
#include <more/diag/debug.h>
#include <iostream>
#include <more/gen/gc_string.h>

int main()
{
#ifdef MORE_MORE_CONF_HAVE_BOEHM_GC
    using more::gen::gc_string;
    gc_string s0("alpha"), s1("beta"), s2;
    MORE_SHOW(s0);
    MORE_SHOW("zin"+s0+s1+"gamma");
#endif
    return 0;
}
