#include "remote_client.hpp"

RemoteClient::RemoteClient(const std::string &host,const std::string &programm, const std::string &param)
:_host(host),_programm(programm),_param(param)
{}
const char* RemoteClient::getCMD(void){
	std::stringstream stream;
	stream<<"ssh "<<_host<<"  "<<_programm<<" "<<_param<<" &";
	_cmd = stream.str();
	return _cmd.c_str();
}
