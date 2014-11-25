#include "linda.hpp"

using namespace LINDA;

/*!
	ctor
	@param argc argument argc from main()
	@param argv argument argv from main()
*/
CLIENT::CLIENT(const int argc,char *argv[])
:operation_out(_socket){
	// we need a thread to ensure, that we receive a message from
	// the tuple space to shut down the server
	boost::thread func(boost::bind(&CLIENT::multicast_thread,this));
	// we need at least 3 argument ( program ip port )
	if(argc < 4){
 		std::cerr<<"Invalid initialization \n";
		std::cerr<<"Arguments : ";
		for(int i = 0; i < argc; i++){
			std::cerr<<argv[i]<<" ";
		}
		std::cerr<<"\n";
 		exit(EXIT_FAILURE);
	}
	// server port is the last argument 
	broadcast_port = atoi(argv[(argc-1)]);
 	_serverPort    = atoi(argv[(argc-2)]);
	host           = argv[(argc-3)];
	_progname      = argv[0];
}
/*!
	Function receives a multicast socket message. We need this to kill the remote clients.
*/
void CLIENT::multicast_thread(void){
	MultiCastReceiver mcr(broadcast_port);
	mcr.wait();
	exit(EXIT_SUCCESS);
}
/*!
	Just a signal handler for SIGPIPE
*/
void CLIENT::signal_handler(int signr){
	signal(SIGPIPE,signal_handler);
}
/*!
	Function to detect the IP Adress from a host
*/
const char *CLIENT::get_addr(const char *host){
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
/*!
	Function to verify, if a tuple was corretly places
*/
const bool CLIENT::tuplePlaced(void){
	const char *msg = "Deserilizing error";
	const int size = 128;
	char buffer[size];
	int r;
	if(( r = recv(_socket,buffer,128-1,0)) > 0){
		buffer[r] = '\0';
		if(strcmp(buffer,msg) == 0){
			return false;
		}
	}
	return true;
}
/*!
	Function to init the client
*/
const bool CLIENT::init(void){
	init_socket();
	inet_aton(get_addr(host),&client_addr.sin_addr);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port   = htons(_serverPort);
	int connection_counter = 0;
	do{
		if(connect_to_ts())
			connection_counter = 10;
		else
			++connection_counter;
	}while(connection_counter != 10);
	if(connection_counter == 10){
		ProcessInfo process;
		process.setPID(getpid());
		process.setToClient();
		process.setProgName(_progname);
		process.setUser(getenv("USER"));
		sendProcessInfo(process);
		return true;
	}
	return false;
}
/*!
	Function to send a process info object to tuple space.
	@param p reference to a process infor object
*/
void CLIENT::sendProcessInfo(const ProcessInfo &p){
	SerializingAgent agent;
	std::stringstream stream;
	char temp[32];
	agent.serilizeClientInfo(p,stream);
	// send size
	{
		const size_t length = stream.str().size();
		sprintf(temp,"%d",length);
		send(_socket,temp,strlen(temp),0);
		recv(_socket,temp,32-1,0);
		send(_socket,stream.str().c_str(),length,0);
		recv(_socket,temp,32-1,0);
	}
}
/*!
	Function to disconnect from tuple space
*/
void CLIENT::stop(void){
	send(_socket,"quit",strlen("quit"),0);
	char buffer[32];
	recv(_socket,buffer,31,0);
	close(_socket);
}
/*************************** FUNCTIONS TO GET A TUPLE ************************************/
/*!
	Function to send a tuple to tuple space

	@param reference to a tuple
*/
void CLIENT::sendTupleToTS(Tuple &tuple){
 	tuple.setActuals();
	std::stringstream net_stream;
	serialize_tuple(tuple,net_stream); // serialize tuple 
	send_tuple_size(net_stream);       // send tuple size to tuple space
	sendTuple(net_stream);		   // send tuple to tuple space
}
/*!
	Function to recv a tuple from tuple space
*/
const bool CLIENT::recvTupleFromTS(Tuple &tuple){
	int  tupleSize = getTupleSize();
	if(tupleSize > 0){
		send(_socket,"OK",strlen("OK"),0);
		getTupleFromTS(tuple,tupleSize);
		return true;
	}
	return false;
}
/*!
	Function to get the tuple size from tuple space
*/
const int CLIENT::getTupleSize(void){
	char temp[128];
	int r;
	if(( r = recv(_socket,temp,127,0)) > 0)
		temp[r] = '\0';
	return atoi(temp);
}
/*!
	This function finally receives the tuple
*/
void CLIENT::getTupleFromTS(Tuple &tuple,const int &size){
	char buffer[256];
	int  r = size;
	std::stringstream stream;
	do{
		int p = recv(_socket,buffer,255,0);
		r     = r - p;
		buffer[p] = '\0';
		stream<<buffer;
	}while(r > 0);
	SerializingAgent agent;
	agent.deserializeTuple(tuple,stream);
	
}
/*!
	Function to get a collection from tuple space
	@param collection reference to a collection
*/
void CLIENT::recvCollection(Collection &collection){
	char sizeBuffer[32];
	std::stringstream stream;
	int r = recv(_socket,sizeBuffer,31,0);
	send(_socket,"OK",strlen("OK"),0);
	if(r > 0){
		sizeBuffer[r] = '\0';
		int size = atoi(sizeBuffer);
		char buffer[512];
		do{
			int p = recv(_socket,buffer,511,0);
			size = size - p;
			buffer[p] = '\0';
			stream<<buffer;
		}while(size > 0);
		if(size == 0){
			try{
				boost::archive::text_iarchive ia(stream);
				ia>>collection;
			}
			catch(std::exception &ex){
				std::cerr<<ex.what()<<std::endl;
				exit(EXIT_FAILURE);
			}
		}
		else{
			exit(EXIT_FAILURE);
		}		
	}
}
/*!
	Function to serialize a tuple
	
	@param tuple reference to a tuple
	@param str   reference to a stringstream
*/
void CLIENT::serialize_tuple(const Tuple &tuple, std::stringstream &str){
	str.clear();
	SerializingAgent agent;
	agent.serializeTuple( tuple, str );
}
/*!
	Function to send the size of the serializd tuple

	@param tuple_stream reference to a stringstream
*/
void CLIENT::send_tuple_size(std::stringstream &tuple_stream){
	const int tupleSize = tuple_stream.str().size();
	char buffer[tupleSize];
	sprintf(buffer,"%d",tupleSize);
	send(_socket,buffer,strlen(buffer),0);
 	recv(_socket,buffer,tupleSize-1,0);
}
/*!
	Function to send the tuple 
*/
void CLIENT::sendTuple(std::stringstream &tuple_stream){
	const std::string stream  = tuple_stream.str();
	send(_socket,stream.c_str(),stream.size(),0);
}
void CLIENT::init_socket(void){
	if((_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		std::cerr<<"socket() failed !"<<std::endl;
		exit(EXIT_FAILURE);
	}
}
bool CLIENT::connect_to_ts(void){
	if((connect(_socket,(struct sockaddr*) &client_addr,sizeof(client_addr))) < 0){
		return false;
	}
	return true;
}
void CLIENT::identify_client(void){
	const char *msg = "client";
	char buffer[32];
	send(_socket,msg,strlen(msg),0);
	recv(_socket,buffer,32-1,0);
}
void CLIENT::sendPID(void){
	char buffer[32];
	int r;
	snprintf(buffer,32,"%d",getpid());   
	send(_socket,buffer,strlen(buffer),0);
	r = recv(_socket,buffer,32-1,0);
	if(r > 0){
	}
}

