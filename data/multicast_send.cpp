#include "multicast_send.hpp"

MultiCastSender::MultiCastSender()
:port(0),socket_d(0)
{}
MultiCastSender::MultiCastSender(const int port)
:port(port)
{}
bool MultiCastSender::init(void){
	socket_d = socket(AF_INET,SOCK_DGRAM,0);
	if(socket_d == -1){
		return false;
	}
	memset(&adress,0,sizeof(adress));
	adress.sin_family      = AF_INET;
	adress.sin_addr.s_addr = inet_addr("224.0.0.1");
	adress.sin_port        = htons(port);
	return true;
}
bool MultiCastSender::send_broadcast(void){
	if(sendto( socket_d,"linda_broadcast_quit",sizeof("linda_broadcast_quit"),0,(struct sockaddr*)&adress,sizeof(adress)) < 0){
		return false;
	}
	return true;
}
