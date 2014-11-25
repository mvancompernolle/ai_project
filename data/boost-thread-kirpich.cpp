#include <boost/thread.hpp>
#include <memory>
void f(){}
int main()
{
    std::auto_ptr<boost::thread> t(new boost::thread(&f));
    return 0;
}
