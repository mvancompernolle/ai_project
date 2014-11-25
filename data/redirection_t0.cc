#include <more/io/redirection.h>
#include <iostream>
#include <more/diag/errno.h>

int
main()
{
    more::io::redirection redir;
    redir.set_out("redirection_t0.out");

    // Repeat to overflow the limit if there is a fd-leak.
    for (int i = 0; i < 1000; ++i) {
	if (!redir.activate()) {
	    std::cerr << "Activate #" << i << " failed "
		      << more::diag::strerror() << std::endl;
	    return 1;
	}
	std::cout << "Check this out!\n";
	redir.deactivate();
    }
    return 0;
}
