#include "local_client.hpp"

LocalClient::LocalClient(const std::string &programm, const std::string &param)
:_programm(programm),_param(param)
{}
const char* LocalClient::getCMD(void){
	std::stringstream stream;
	stream.clear();
	stream<<_programm<<" "<<_param<<" & ";
	_cmd = stream.str();
	return _cmd.c_str();
}
