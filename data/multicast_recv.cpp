#include "multicast_recv.hpp"

MultiCastReceiver::MultiCastReceiver()
:port(0),p(0){}

MultiCastReceiver::MultiCastReceiver(const int port)
:port(port),p(1){}

void MultiCastReceiver::wait(){
	if((server_host_name = gethostbyname("224.0.0.1")) == 0){
		perror("gethostbyname()");
		exit(EXIT_FAILURE);
	}
	socket_d  = get_multicast_socket();
	sin_len = sizeof(sin);
	if(recvfrom(socket_d,message,256,0,(struct sockaddr*)&sin,&sin_len) == -1){
		perror("recvfrom()");
	}
	if (setsockopt ( socket_d,IPPROTO_IP,IP_DROP_MEMBERSHIP,&mreq, sizeof (mreq)) < 0 ){
		perror ("setsockopt:IP_DROP_MEMBERSHIP");
	}
	close (socket_d);
}
int MultiCastReceiver::get_multicast_socket(void){
	memset (&sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl (INADDR_ANY);
	sin.sin_port = htons (port);
	if((socket_d=socket (PF_INET, SOCK_DGRAM, 0)) == -1) {
		perror ("socket()");
		exit (EXIT_FAILURE);
	}
	if (setsockopt ( socket_d,
	    SOL_SOCKET,
	    SO_REUSEADDR,
	    &p, sizeof (p)) < 0) {
		    perror ("setsockopt:SO_REUSEADDR");
		    exit (EXIT_FAILURE);
	    }
	    if(bind( socket_d,
	       (struct sockaddr *)&sin,
	       sizeof(sin)) < 0) {
		       perror ("bind");
		       exit (EXIT_FAILURE);
	       }
	       /* Broadcast auf dieser Maschine zulassen */
	       if (setsockopt ( socket_d,
		   IPPROTO_IP,
		   IP_MULTICAST_LOOP,
		   &p, sizeof (p)) < 0) {
			   perror ("setsockopt:IP_MULTICAST_LOOP");
			   exit (EXIT_FAILURE);
		   }
		   /* Join the broadcast group: */
		   mreq.imr_multiaddr.s_addr = inet_addr ("224.0.0.1");
		   mreq.imr_interface.s_addr = htonl (INADDR_ANY);
		   if (mreq.imr_multiaddr.s_addr == -1) {
			   perror ("224.0.0.1 ist keine Multicast-Adresse\n");
			   exit (EXIT_FAILURE);
		   }
		   if (setsockopt ( socket_d,
		       IPPROTO_IP,
		       IP_ADD_MEMBERSHIP,
		       &mreq, sizeof (mreq)) < 0) {
			       perror ("setsockopt:IP_ADD_MEMBERSHIP");
		       }
		       return socket_d;
}
