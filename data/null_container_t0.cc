#include <more/gen/null_container.h>
#include <algorithm>

// Utilize the concept checks of the STL library, if available.
int main() {
    more::gen::null_container<int> c;
    more::gen::null_container<int> const& c_const(c);
    std::copy(c.begin(), c.end(), c.begin());
    std::copy(c_const.begin(), c_const.end(), c.begin());
    return 0;
}
