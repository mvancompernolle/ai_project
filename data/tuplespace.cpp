#include "tuplespace.hpp"

using namespace LINDA;
TUPLESPACE::InstanzHandler TUPLESPACE::instanceAgent;
unsigned int TUPLESPACE::instanceID = 0;
boost::mutex TUPLESPACE::instance_mutex;


void TUPLESPACE::error_cleanup(){
	MessageAgent agent;
	MultiCastSender multi_send(multicast_port);
	multi_send.init();
	if(!stopped){
		stopped = true;
		multi_send.send_broadcast();
	}
}
void TUPLESPACE::cleanup(void){
	instanceAgent.cleanup();
}
const unsigned int TUPLESPACE::getSpaceID(void) const{
	return myID;
}
TUPLESPACE::~TUPLESPACE(){
	MessageAgent agent;
	instanceAgent.remove(instanceID);
}
/*!
	
	@param port Port for tuple space server
	@sa    sigpipe_handler()
	@sa    sigint_handler()
	
	@brief Contructor for tuple space server
	
*/
TUPLESPACE::TUPLESPACE(const int communication_port,const int multicast_port)
:SpaceNetwork(communication_port),server_port(communication_port),clients_running(false),connections(0),eachClientConnected(false),totalConnections(0),multicast_port(multicast_port),stopped(false),server_initialized(false)
{
	MessageAgent agent;
	
	 
	// The function register_instance() is static.
	// If different threads would like to create a tuple space 
	// object, we would get some problems.Therfore, we need a
	// static booost::mutex.
	{
		boost::mutex::scoped_lock lock(instance_mutex);
		// the first instance must
		// => call the function atexit() with the member function cleanup as its argument
		// => initialize the signal handler
		if(instanceID == 0){
 			atexit(cleanup);
			
			// signal handler for signal SIGPIPE.
			// We need this signal handler, otherwise, the program 
			// will abort, if a client is not available.
			// The signal will be ignored.	
			if(signal(SIGPIPE,SIG_IGN) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGPIPE");
				exit(EXIT_FAILURE);
			}
			
			// Signal handler for SIGINT.
			// The function sigint_handler will shutdown the server.
			if(signal(SIGINT,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGINT");
				exit(EXIT_FAILURE);
			}
		
			if(signal(SIGTERM,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGTERM");
				exit(EXIT_FAILURE);
			}
			if(signal(SIGABRT,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGABRT");
				exit(EXIT_FAILURE);
			}
			if(signal(SIGFPE,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGABRT");
				exit(EXIT_FAILURE);
			}
			if(signal(SIGSEGV,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGABRT");
				exit(EXIT_FAILURE);
			}
			if(signal(SIGTERM,sigint_handler) == SIG_ERR){
				agent.print_error("signal() failed for signal SIGABRT");
				exit(EXIT_FAILURE);
			}
		}
		myID = instanceID++;
	}
	instanceAgent.register_instance(this);
}
/*!
	Signal handler for SIGINT.This function will shutdown the server ( i.e shutdown all local and remote processes ).

	@param signr unused
*/
void TUPLESPACE::sigint_handler(int signr){
	instanceAgent.cleanup();
	exit(EXIT_FAILURE);
}
/*!
	
	Function to initialize the tuple space, i.e, network start up and a new boost::thread
	to accept the client requests ( TUPLESPACE::fetch_requests() ).

	@sa	fetch_requests()
	@sa	init_server_network()

	@brief Network start up and new boost::thread 
*/
void TUPLESPACE::init(void){
	if(server_initialized == false){
		server_initialized = true;
		init_server_network();
		channel.start_thread();
		std::auto_ptr<boost::thread>(new boost::thread(boost::bind(&TUPLESPACE::fetch_requests,this)));
	}
}
/*!
	This functions stops each connected linda client as follows.If the tuple space accepts a connection,
	we get the process id from the remote process. With these pids we can create a ssh connection
	to each remote client and kill the process.
*/
void TUPLESPACE::stop(void){
	MessageAgent agent;
	MultiCastSender multi_send(multicast_port);
	multi_send.init();
	if(!stopped){
		stopped = true;
		multi_send.send_broadcast();
		instanceAgent.remove(instanceID);
	}
}
/*!
	Function, to add a local client ( running on the same machine like the tuple space server )
*/
void TUPLESPACE::addLocal(const std::string &programm, const std::string &param){
	if(clients_running == false){
		std::ifstream  file;
		file.open(programm.c_str());
		if(file.good() == false){
			std::cerr<<"open linda error : Program "<<programm<<" not available in current working directory\n";
			exit(EXIT_FAILURE);
		}
		std::stringstream modifed;
		modifed<<param<<" "<<getIP()<<" "<<server_port<<" "<<multicast_port;
		file.close();
		LocalClient lc(programm,modifed.str());
 		local_clients.push_back(lc);
	}
}
void TUPLESPACE::addRemote(const std::string &host,const std::string &program,const std::string &param){
	if(clients_running == false){
		std::stringstream modified;
		modified<<param<<" "<<getIP()<<" "<<server_port<<" "<<multicast_port;
		RemoteClient rc(host,program,modified.str());
  		remote_clients.push_back(rc);
	}
}
void TUPLESPACE::addRemoteWithUser(const std::string &user,const std::string &host,
				   const std::string &program,const std::string &param){
	if(clients_running == false){
		std::stringstream modified;
		modified<<param<<" "<<getIP()<<" "<<server_port<<" "<<multicast_port;
		RemoteUserClient rcu(user,host,program,modified.str());
		remote_client_user.push_back(rcu);
	}
}
/*!
	Function to suspend, until all clients are connected
 */
void TUPLESPACE::wait(void){
	boost::mutex::scoped_lock lock(connection_mutex);
	unsigned int space_clients = local_clients.size() + remote_clients.size();
	while(space_clients != connections){
		connection_condi.wait(lock);
	}
}
/*!
	Wait, until all clients closed their connections
*/
void TUPLESPACE::await(void){
	boost::mutex::scoped_lock lock(connection_mutex);
	while(connections > 0){
		connection_condi.wait(lock);
	}
}
/*!
	Function to ensure, that we received a connection request from each client
*/
void TUPLESPACE::ensure(void) {
	boost::mutex::scoped_lock lock(total_connection_mutex);
	while(eachClientConnected != true){
		total_condi.wait(lock);
	}
}
unsigned int TUPLESPACE::getConnections(void){
	boost::mutex::scoped_lock lock(total_connection_mutex);
	return connections;
}
/*!
	Public function to start all local and remote client.A local client is startet by a simple system call whereas
	a remote client is also started by a system call, but in combination with a ssh connection.

	@brief function to all start clients
 */
void TUPLESPACE::start(void){
	MessageAgent agent;
	if(clients_running == false){
		clients_running = true;
 		for(size_t i = 0; i < local_clients.size(); ++i){
			system(local_clients.at(i).getCMD());
 		}
 		for(size_t i = 0; i < remote_clients.size(); ++i){
			system(remote_clients.at(i).getCMD());
 		}
		for(size_t i = 0; i < remote_client_user.size(); ++i){
			system(remote_client_user.at(i).getCMD());
		}
	}
}
void TUPLESPACE::getRemoteInfo(ProcessInfo &remote,const int &_socket){
	// receive the size of the stream
	char size[32];
	int  r;
	if((r = recv(_socket,size,32-1,0)) > 0){
		size[r] ='\0';
		const int stream_size = atoi(size);
		char  buffer[stream_size];
		send(_socket,"OK",strlen("OK"),0);
		// receive the stream
		if((r = recv(_socket,buffer,stream_size,0)) > 0){
			std::stringstream stream;
			// deserialize the stream
			SerializingAgent agent;
			stream<<buffer;
			agent.deserializeClientInfo(remote,stream);
			send(_socket,"OK",strlen("OK"),0);
		}
	}
}
/*!
	Function, to accept connection requests, running as a thread
 */
void TUPLESPACE::fetch_requests(void){
	bool flag = true;
	unsigned int client_ids = 1;
	while(flag){
		client_socket = accept(server_socket,(struct sockaddr*)&srv_addr,&addrlength);	
		if(client_socket < 0){
			if(errno == EINTR)
				continue;
			else{
// 				std::cerr<<"accept() failed !"<<std::endl;
			}
		}
		else{
			ProcessInfo remote;
			getRemoteInfo(remote,client_socket);
			if(remote.isClient()){
				remote.setIP(inet_ntoa(srv_addr.sin_addr));
				remote.setID(client_ids++);
				remote.setSocket(client_socket);
				boost::thread p(boost::bind(&TUPLESPACE::client_handler,this,remote));
			}
			else if(remote.isTupleScope()){
				channel.addSocket(client_socket);
			}
		}
	}
}
/*!
	Member function to handle a client.This function is running in a boost::thread and created by the
	function TUPLESPACE::fetch_requests().
	
	@param clientSocket accepted socket for network communication
	@param clientID     client id
	@param addr         IP from the client

	@sa		    fetch_requests()
	@sa		    getTupleFromClient()
	@sa		    remotecopycollect()
	@sa		    remoteCollect()
	@sa		    copyQuery()
	@sa		    query()
	@sa		    sendcollection()
	
 */
void TUPLESPACE::client_handler(const ProcessInfo remote){
	ScopeInfo scope(remote);
	MessageAgent message;
	char buffer[128];
	int  r;
	{
		boost::mutex::scoped_lock lock(total_connection_mutex);
		totalConnections = totalConnections + 1;
		unsigned int space_clients = local_clients.size() + remote_clients.size();
		// now, we can check if all clients connected to tuple space
		// it's not important, if there are still connected, we just check
		// if we have enough connections. With connections, we count the number
		// of CURRENT connections and with totelConenctions we count the
		// the number of CONNECTED clients.We need the "totalConnections"
		// for the function TUPLESPACE::ensure().With this function we can
		// check, if each clients is or was connected to tuple space.
		// The program will suspend, until we receive a connection request
		// from ALL remote and local processes.
 		//
		// EXAMPLE:
		//
		// We have 3 remote clients.
		// client #1 : we accept the connection request and the closed his connection immediately
		// totalConenctions = 1   connections = 0
		// client #2 : we accept the connection request and the closed his connection immediately
		// totalConnections = 2   connections = 0
		// client #3 : we accept the connection request and the closed his connection immediately
		// totalConnections = 3   connections = 0
		//
		// With the function TUPLESPACE::wait() we have now a deadlock
		// -> We add 3 clients, and wait() waits until the 3 clients are connected
		// With the function TUPLESPACE::erase() we don't have a deadlock
		// -> This function just ensure, that the tuple space received 3 connection requests
		//
		// THIS IS A SMALL BUT IMPORTANT DIFFERENCE
		if(totalConnections == space_clients){
			eachClientConnected = true;
			total_condi.notify_all();
		}
	}
	{
		boost::mutex::scoped_lock lock(connection_mutex);
		connections = connections + 1; // new connection, therefore, increment
		connection_condi.notify_all();
	}
	scope.setNew(true);
	scope.showTupleString(false);
	{
		/*!
			@brief ich muss die tuple noch zählen.
		*/
// 		boost::mutex::scoped_lock lock(tuple_mutex);
// 		scope.setTuple(tuple_container.size());
	}
	{
		boost::mutex::scoped_lock lock(connection_mutex);
		scope.setConnections(connections);
	}
	channel.channel_send(scope);
	scope.setNew(false);
	while(( r = recv(remote.getSocket(),buffer,127,0)) > 0){
		buffer[r] = '\0';  // tuple size
		send(remote.getSocket(),"OK",strlen("OK"),0);
		// if a clients send quit, we must remove this client to avoid
		// that we create a ssh connection to the remote host.Sure, why should we
		// kill a client, that is not alive
		if(strcmp(buffer,"quit") == 0){
			break; // we receive quit from a client and not the tuple size
		}
		const int tupleSize = atoi(buffer);
		Tuple clientTuple;
		if(!getTupleFromClient(remote.getSocket(),tupleSize,clientTuple)){
			// TODO : BETA DIRECTORY
		}
		else{
// 			
			// out : place a tuple into tuple space
			if(strcmp(clientTuple.getCommand().c_str(),"out") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("out");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				db.out(clientTuple);
				send(remote.getSocket(),"placed",strlen("placed"),0);
			}
			// in : withdraw a tuple from tuple space
			else if(strcmp(clientTuple.getCommand().c_str(),"in") == 0){
				
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("in");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				db.in(clientTuple);
				sendTupleToClient(clientTuple,remote.getSocket());
			}
			// rd : read a tuple from tuple space
			else if(strcmp(clientTuple.getCommand().c_str(),"rd") == 0){
				
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("rd");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				db.rd(clientTuple);
				sendTupleToClient(clientTuple,remote.getSocket());
			}
			// inp : check once, if the requested tuple is available ( Tuple will be withdrawn )
			else if(strcmp(clientTuple.getCommand().c_str(),"inp") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setTuple(db.size());
				scope.setOperation("inp");
				channel.channel_send(scope);
				
				
				if(db.inp(clientTuple)){
					sendTupleToClient(clientTuple,remote.getSocket());
				}
				else{
					send(remote.getSocket(),"0",strlen("0"),0);
				}
				
			}
			// rdp : check once, if the requested tuple is available 
			else if(strcmp(clientTuple.getCommand().c_str(),"rdp") == 0){
				
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("rdp");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				
				if(db.rdp(clientTuple)){
					sendTupleToClient(clientTuple,remote.getSocket());
				}
				else{
					send(remote.getSocket(),"0",strlen("0"),0);
				}
			}
			/*#################### COLLECT #################################*/
			else if(strcmp(clientTuple.getCommand().c_str(),"collect") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("collect");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.collect(collection,clientTuple); 
				sendCollection(collection,remote.getSocket());
				scope.setOperation("collect");
			}
			/*#################### COPY-COLLECT #################################*/
			else if(strcmp(clientTuple.getCommand().c_str(),"copycollect") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("copy-collect");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.copycollect(collection,clientTuple);
				sendCollection(collection,remote.getSocket());
				scope.setOperation("copy-collect");
			}
			/*########################## QUERY #################################*/
			else if(strcmp(clientTuple.getCommand().c_str(),"query") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("query");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.query(collection,clientTuple);
				sendCollection(collection,remote.getSocket());
				scope.setOperation("query");
			}
			else if(strcmp(clientTuple.getCommand().c_str(),"queryp") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("query-p");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.queryp(collection,clientTuple);
				sendCollection(collection,remote.getSocket());
				scope.setOperation("query-p");
			}
			else if(strcmp(clientTuple.getCommand().c_str(),"copyquery") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("copy-query");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.copyquery(collection,clientTuple);
				sendCollection(collection,remote.getSocket());
				scope.setOperation("copy-query");
			}
			else if(strcmp(clientTuple.getCommand().c_str(),"copyquery-p") == 0){
				scope.setTupleString(clientTuple.getTupleString());
				scope.showTupleString(true);
				scope.setOperation("copy-query-p");
				scope.setTuple(db.size());
				channel.channel_send(scope);
				Collection collection;
				db.copyqueryp(collection,clientTuple);
				sendCollection(collection,remote.getSocket());
				scope.setOperation("copy-queryp-p");
			}
		}
		{
			boost::mutex::scoped_lock lock(connection_mutex);
			scope.setConnections(connections);
		}
// 		scope.setTuple(db.size());
		
	}
	{
		boost::mutex::scoped_lock lock(connection_mutex);
		connections = connections - 1; // connection closed
		connection_condi.notify_all();
	}
	scope.markAsDisconnected();
	{
// 		scope.setTuple(db.size());
	}
	{
		boost::mutex::scoped_lock lock(connection_mutex);
		scope.setConnections(connections);
	}
	scope.setTuple(db.size());
	channel.channel_send(scope);
	close(remote.getSocket());
}
void TUPLESPACE::sendCollection(const Collection &collection,const socket_t &_socket){
	std::stringstream stream;
	SerializingAgent agent;
	agent.serializeCollection(collection,stream);
	char sizeBuffer[32];
	int size = strlen(stream.str().c_str());
	sprintf(sizeBuffer,"%d",size);
	send(_socket,sizeBuffer,strlen(sizeBuffer),0);
	recv(_socket,sizeBuffer,31,0);
	send(_socket,stream.str().c_str(),size,0);
}
/*!
	Function to receive a tuple from tuple space
*/
const bool TUPLESPACE::getTupleFromClient(const socket_t &socket,const int &tupleSize,Tuple &tuple){
	char tuple_buffer[tupleSize];
	SerializingAgent agent;
	int  r;
	if((r = recv(socket,tuple_buffer,tupleSize,0)) > 0){
		tuple_buffer[r] = '\0';
		std::stringstream stream;
		stream<<tuple_buffer;
		agent.deserializeTuple(tuple,stream);
	}
	return true;
}
/* ############################### OPERATIONS FOR TUPLE SPACE ###################################################*/
void TUPLESPACE::copy_collection_to_localspace(const Collection &collection,LocalSpace *ls){
	for(size_t index = 0; index < collection.getSize(); index++){
		ls->outTuple(collection.getTuple(index));
	}
}
void TUPLESPACE::outTuple(const Tuple &tuple){
	db.out(tuple);
}
/*!
	This function is from the class ts_in.In ts_in, this function is just a pure virtual function !.With this function, the tuple space is able to get and remove a tuple.If a tuple ist not available, the process with will suspend until
	a matched tuple is found, therefore, the functionality of this function is equivalent to the in() function
	for the linda client processes.The argument tuple is an anti-tuple, if a machted tuple is found, the anti tuple
	will be changed into a normal tuple and each value will be automatically an actual.

	@param tuple anti-tuple 
	@brief in implementation for tuple space
*/
void TUPLESPACE::inTuple(Tuple &tuple){
	db.in(tuple);
}
/*!
	This function is from the class td_rd.In ts_rs, this function is just a pure virtual function !.The function
	is equivalent to the function rd implementet for the linda client processes, therefore, it is a suspending and 
	non-destructivly operation.

	@param tuple anti-tuple 
	@brief  rd implementation for tuple space
*/
void TUPLESPACE::rdTuple(Tuple &tuple){
	db.rd(tuple);
}
/*!
	This function is from the class ts_rdp.In ts_rdp, this function is a pure virtual function !.The functionality
	is equivalent to the rdp implementation for the linda processes, therefore, this is a non-suspending and non-destructivly operation. We check once if a tuple is available or not.Please note, that, if a tuple is
	not found, any formals in the tuple are ( of course ) still formals.

	@param	tuple anti-tuple
	@brief  rdp implementation for tuple space
*/
bool TUPLESPACE::rdpTuple(Tuple &tuple){
	return db.rdp(tuple);
}
/*!
	This function is from the class ts_inp.In ts_inp, this function is a pure virtual function.The functionality is 
	equivalent to the inp implementation for the linda processes.We check once, if a tuple is available or not.Please note, that, if a tuple is not found, any formals in the tuple are ( of course ) still formals.

	@param	tuple anti-tuple
	@brief  inp implementation for tuple space
*/
bool TUPLESPACE::inpTuple(Tuple &tuple){
	return db.inp(tuple);
}
/*!
	This function is from the class ts_collect.In ts_collect, this function is implemented as a pure
	virtual function. The functionality is equivalent to the collection implementation for the linda processes.This
	function was proposed by Paul Butcher,Alan Wood nad Martin Atkins from the University of York ( paper : global synchronization in linda ) and is implemented in open linda.The functionality of collect is comparable with a 
	repeated call of inp().This function is a destructivly and non suspending function.The mutex for the tuple container is set in the function createCollection().All matched tuples are placed ( for the time being ) in a collection
	and afterwards in the local space specified by the first argument.This function calls collectIMPL() to collect the tuples. 

	@param ls Local Space to place all matched tuples
	@param tuple  ant-tuple
	@return number of machted tuples
	@brief  collect implementation for tuple space
*/
const size_t TUPLESPACE::collectTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	db.collect(collection,tuple);
	copy_collection_to_localspace(collection,ls);
	return collection.getSize();
}
/*!
	Function for tuple space for copy-collect.In the class ts_copycollect, this function is just a pure virtual function.The function is equivalent to the copycollect implementation for the remote processes ( function remotecopycollect() ).This is a non-suspending and non-destructivly function.This function solves the multiple rd problem.The matched tuples will be placed in a localspace ( specified by the first argument ).

	@param ls    localspace to place tuples 
	@param tuple anti-tuple
	@brief copy-collect implementation for tuple space
*/
const size_t TUPLESPACE::copycollectTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	db.copycollect(collection,tuple);
	copy_collection_to_localspace(collection,ls);
	return collection.getSize();
}
/*!

	@param tuple anti-tuple to found tuples;
	@param ls Local space to place the machted tuples
	
	@brief query implementation for tuple space
*/
const void TUPLESPACE::queryTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	db.query(collection,tuple);
	copy_collection_to_localspace(collection,ls);
}
/*!
	queryp implementation, this is the non-suspending version of query.

	@brief query-p implementation
*/
const bool TUPLESPACE::querypTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	const bool retcode = db.queryp(collection,tuple);
	copy_collection_to_localspace(collection,ls);
	return retcode;
}
/*!
	@brief copy-query implementation
*/
const void TUPLESPACE::copy_queryTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	db.copyquery(collection,tuple);
	copy_collection_to_localspace(collection,ls);
}

/*!
	@brief  copy-queryp implementation for tuple space
*/
const bool TUPLESPACE::copy_querypTuple(LocalSpace *ls,const Tuple &tuple){
	Collection collection;
	const bool retcode = db.copyqueryp(collection,tuple);
	copy_collection_to_localspace(collection,ls);
	return retcode;
}
void TUPLESPACE::sendTupleToClient(const Tuple &tuple, const int _socket){
	char buffer[128];
	char temp[32];
	SerializingAgent agent;
	std::stringstream stream;
	agent.serializeTuple(tuple,stream);
	{
		std::stringstream convert;
		convert<<stream.str().size();
		convert>>buffer;
	}
	// send tuple size
	send(_socket,buffer,strlen(buffer),0);
	recv(_socket,temp,32,0);
	send(_socket,stream.str().c_str(),atoi(buffer),0);
	
}

void TUPLESPACE::InstanzHandler::register_instance(TUPLESPACE *object){
	boost::mutex::scoped_lock lock(_mutex);
	space_vector.push_back(object);
}
const bool TUPLESPACE::InstanzHandler::remove(const unsigned int &spaceID){
	boost::mutex::scoped_lock lock(_mutex);
	for(size_t index = 0; index < space_vector.size(); index++){
		if(space_vector.at(index)->getSpaceID() == spaceID){
			space_vector.erase(space_vector.begin() + index);
			return true;
		}
	}
	return false;
}
void TUPLESPACE::InstanzHandler::cleanup(void){
	boost::mutex::scoped_lock lock(_mutex);
	for(size_t index = 0; index < space_vector.size(); index++){
		space_vector.at(index)->error_cleanup();
	}
	space_vector.clear();
}
