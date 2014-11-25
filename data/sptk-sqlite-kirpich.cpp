#include <sptk3/sptk-config.h>
#include <sptk3/CSQLite3Database.h>

#if !HAVE_SQLITE3
#error SQLite3 support is not compiled in SPTK
#endif

using namespace sptk;

CSQLite3Database db("path");

int main(int, char**)
{
}
