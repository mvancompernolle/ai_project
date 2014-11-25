#include "channel.hpp"

/*!
	Function to allow a remote process, to add his informations ( ID, Process ID, IP Address ) to a channel.This informations
	will be send to all connected tuple scopes.

	@note  function is proctected by a boost::mutex
	@param object ScopeInfo object, that will be placed into a channel by any connected process
*/
void Channel::channel_send(const ScopeInfo object){
	boost::mutex::scoped_lock lock(_mutex);
	_queue.push(object);
	_condi.notify_one();
}
/*!
	Function to start a new boost::thread for distribute informations to connected tuple scopes

	@sa		dispatch()
*/
void Channel::start_thread(void){
	boost::thread t(boost::bind(&Channel::dispatch,this));
}
/*!
	Function runs in a boost::thread and sends ScopeInfo objects to connected tuple scopes

	@sa		receive()
	@sa		distribute()
*/
void Channel::dispatch(void){
	ScopeInfo object;
	// NO. This is not polling, receive will block, if the _queue is empty
	for(;;){		
		object = receive();
		distribute(object);
	}
}
/*!
	Function to get a new ScopeInfo object from the _queue.

	@return new object that will be send to connected tuple scopes
*/
ScopeInfo Channel::receive(void){
	boost::mutex::scoped_lock lock(_mutex);
	while(_queue.empty())
		_condi.wait(lock);
	ScopeInfo object = _queue.front();
	_queue.pop();
	return object;
}
/*!
	This function sends one ScopeInfo object to each connected tuple scope

	@sa		sendObject()
	@sa		serialize_object()
	@note		tuple scope sockets are protected by a boost::mutex
*/
void Channel::distribute(const ScopeInfo &object){
	std::stringstream stream;
	serialize_object(object,stream);
	boost::mutex::scoped_lock lock(_socket_mutex);
	try{
		for(unsigned int index = 0;index < _sockets.size(); index++){
			if(sendObject(stream,_sockets.at(index)) == false){
				// remove socket
				_sockets.erase(_sockets.begin() + index);
				index = index - 1;
			}
		}
	}catch(std::exception &ex){
		std::cerr<<"Channel::distribute "<<ex.what()<<std::endl;
		exit(EXIT_FAILURE);
	}
}
/*!
	Function to serialize a ScopeInfo object 
*/
void Channel::serialize_object(const ScopeInfo &object,std::stringstream &stream){
	SerializingAgent agent;
	agent.serializeScopeInfo(object,stream);
}
/*!
	Function to send the serialized ScopeInfo object 

	@return false if a tuple scope is not available ( i.e. distribute() will remove the socket )
	@sa		distribute()
*/
const bool Channel::sendObject(const std::stringstream &stream,const int &_socket) const{
	const int _size = stream.str().size();
	if(send_size(_size,_socket)){
		if(send_stream(stream,_socket,_size)){
			return true;
		}
		else{
			return false;
		}
	}
	return false;
}
/*!
	Function to send the ScopeInfo size ( i.e. the size of the serialized object )

	@param _size     size of the stream
	@param  _socket  socket of a tuple scope 
*/
const bool Channel::send_size(const int &_size,const int &_socket) const{
	char buffer[32];
	sprintf(buffer,"%d",_size);
	send(_socket,buffer,strlen(buffer),0);
	const int r = recv(_socket,buffer,31,0);
	if(r > 0) {
		return true;
	}
	return false;
}
/*!
	Function to send the stream.
	
	@param	stream  the serialized ScopeInfo object 
	@param  _socket socket of a tuple scope 
	@param  _size   the length of the serialied ScopeInfo object 
*/
const bool Channel::send_stream(const std::stringstream &stream,const int &_socket,const int &_size) const{
	char buffer[32];
	send(_socket,stream.str().c_str(),_size,0);
	const int r = recv(_socket,buffer,31,0);
	if(r > 0) {
		return true;
	}
	return false;
}
/*!
	Function to allow the tuple scope, to add the socket of a new connected tuple scope.
	
	@param socket the socket of the new connected tuple scope
*/
void Channel::addSocket(const int socket){
	boost::mutex::scoped_lock lock(_socket_mutex);
	try{
		_sockets.push_back(socket);
	}catch(std::exception &ex){
		std::cerr<<"Exception in Channel "<<ex.what()<<std::endl;
		exit(EXIT_FAILURE);
	}
}
