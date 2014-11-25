#include <more/net/ldap.h>

namespace net = more::net;

int
main()
{
    net::ldap_dn dn("ou = people, dc=example, dc=com ", net::ldap_dn_syntax_rfc);
    std::cout << dn;
    return 0;
}
