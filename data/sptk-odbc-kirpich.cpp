#include <sptk3/sptk-config.h>
#include <sptk3/CODBCDatabase.h>

#if !HAVE_ODBC
#error ODBC support is not compiled in SPTK
#endif

using namespace sptk;

CODBCDatabase db("DSN=");

int main(int, char**)
{
}
