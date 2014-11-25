#include <string>
#include <iostream>
#include <list>
#include <more/gen/lambda.h>
#include <algorithm>

int main() {
    using more::gen::lambda;
    using more::gen::refer;

    std::string sl[] = { "test ", "of ", "stream ", "operators\n" };
    more::gen::placeholder<1, std::string> ph;

    std::for_each(sl+0, sl+4,
		  lambda(ph, refer<std::ostream>(std::cout) << ph));
    return 0;
}

