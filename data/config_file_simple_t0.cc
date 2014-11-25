#include <more/io/config_file.h>
#include <iostream>
#include <more/io/iomanip.h>

namespace io = more::io;

void
print_tree(std::ostream& os, io::config_tree const& t, int indent = 0)
{
    os << io::osfill(indent) << "\"" << t.value() << "\"\n";
    for (io::config_tree::branch_const_iterator
	     it = t.branches().begin();
	 it != t.branches().end(); ++it)
	print_tree(os, *it, indent + 4);
}

int
main()
{
//    io::config_tree t(io::read_config_ksvn("/etc/openldap/ldap.conf"));
    io::config_tree t;
    io::read_config_ksvn(t, "/etc/syslog.conf");
    print_tree(std::cout, t);

    t.clear();
    io::read_config_kevn(t, "/etc/wgetrc");
    print_tree(std::cout, t);

    return 0;
}
