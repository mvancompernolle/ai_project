#include "space_network.hpp"

SpaceNetwork::SpaceNetwork(const int port)
:server_port(port),addrlength(sizeof(struct sockaddr_in))
{}
SpaceNetwork::~SpaceNetwork()
{
	close(server_socket);
}
const char *SpaceNetwork::getIP(void){
	int tempSocket = socket( AF_INET , SOCK_DGRAM , 0 );
	if( tempSocket == -1 ){ 
		std::cerr<<"SpaceNetwork::getIP() : function socket() failed : "<<__FILE__<<" "<<__LINE__<<std::endl;
		exit(EXIT_FAILURE); 
	}
	struct ifreq ifr;
	strcpy( ifr.ifr_name , "eth0" );
	if( ioctl( tempSocket , SIOCGIFADDR , &ifr ) == -1 ){ 
		std::cerr<<"SpaceNetwork::getIP() : function ioctl() failed : "<<__FILE__<<" "<<__LINE__<<std::endl;
		exit(EXIT_FAILURE); 
	}
	close( tempSocket );
	return ( inet_ntoa( ((struct sockaddr_in *) (&ifr.ifr_addr))->sin_addr ));
}
void SpaceNetwork::init_server_network(void)
{
	const int i = 1;
	init_server_socket( );
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port   = htons(server_port);
	memset(&srv_addr.sin_addr,0,sizeof(srv_addr.sin_addr));
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(int));
	init_server_bind();
	if(listen(server_socket,0))
	{
		std::cerr<<"listen() failed !"<<std::endl;
		exit(EXIT_FAILURE);
	}
}
void SpaceNetwork::init_server_socket(void)
{
	if((server_socket = socket(PF_INET,SOCK_STREAM,0)) < 0)
	{
		std::cerr<<"socket() failed !"<<std::endl;
		exit(EXIT_FAILURE);
	}
}
void SpaceNetwork::init_server_bind(void)
{
	const int retcode = bind(server_socket,(struct sockaddr *)&srv_addr,sizeof(srv_addr));;
	if(retcode != 0)
	{
		std::cerr<<"bind("<<server_port<<") failed !"<<std::endl;
		exit(EXIT_FAILURE);
	}
}
