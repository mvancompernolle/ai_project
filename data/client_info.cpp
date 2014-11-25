#include "client_info.hpp"

ProcessInfo::ProcessInfo()
:process_pid(0),ip_addr(""),_client_id(0),client(false),_socket(-1),new_process(false),connection_closed(false)
{}
const pid_t ProcessInfo::getPID(void) const{
	return process_pid;
}
const char *ProcessInfo::getPID_as_char_ptr(void) const{
	std::stringstream stream;
	stream<<process_pid;
	return stream.str().c_str();
}
const std::string ProcessInfo::getIP(void) const{
	return ip_addr;
}
const unsigned int ProcessInfo::getID(void) const{
	return _client_id;
}
const bool ProcessInfo::isNewClient(void) const{
	return new_process;
}
void ProcessInfo::setPID(const pid_t pid){
	process_pid = pid;
}
void ProcessInfo::setIP(const std::string ip){
	ip_addr = ip;
}
void ProcessInfo::setID(const unsigned int client_id){
	_client_id = client_id;
}
void ProcessInfo::setToClient(void){
	client = true;
}
const bool ProcessInfo::isClient(void) const{
	return client;
}
const int ProcessInfo::getSocket(void) const{
	return _socket;
}
void ProcessInfo::setSocket(const int socket){
	_socket = socket;
}
void ProcessInfo::setToNewClient(const bool &flag){
	new_process = flag;
}
const bool ProcessInfo::isDisconnected(void) const{
	return (connection_closed == true);
}
const int ProcessInfo::isTupleScope(void) const{
	return ( client == false);
}
const std::string ProcessInfo::getProcessString(void) const{
	std::stringstream stream;
	stream<<"Process "<<process_pid<<"@"<<ip_addr<<" with ID "<<_client_id<<" : ";
	return stream.str();
}
void ProcessInfo::markAsDisconnected(void){
	connection_closed = true;
}
void ProcessInfo::setProgName(const std::string &prog){
	_progname = prog;
}
const std::string &ProcessInfo::getProgName(void) const{
	return _progname;
}
void ProcessInfo::setUser(const char *user){
	_username = user;
}
const std::string &ProcessInfo::getUser(void) const{
	return _username;
}
