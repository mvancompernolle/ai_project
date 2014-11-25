#include "network_basic.hpp"

using namespace LINDA::NETWORK;

const char *NetworkBasic::get_addr(const char *host){
	struct hostent *rechner;
	struct in_addr ip_addr;
	struct in_addr **addr_ptr;
	if(inet_aton(host,&ip_addr) != 0)
		rechner = gethostbyaddr((char*)&ip_addr,sizeof(ip_addr),AF_INET);
	else
		rechner = gethostbyname(host);
	if(rechner == NULL){
		herror("Unknown tuple space server\n");
		exit(EXIT_FAILURE);
	}
	addr_ptr = (struct in_addr**) rechner->h_addr_list;
	return (inet_ntoa(**addr_ptr));
}
