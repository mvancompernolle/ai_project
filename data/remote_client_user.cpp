#include <linda/network/remote_client_user.hpp>

RemoteUserClient::RemoteUserClient(const std::string &user,
		 		   const std::string &host,
				   const std::string &program,
                                   const std::string &param)
:_user(user),_host(host),_program(program),_param(param)
{}

const char* RemoteUserClient::getCMD(){
	std::stringstream stream;
	stream<<"ssh "<<_user<<"@"<<_host<<" "<<_program<<" "<<_param<<" & ";
	_cmd = stream.str();
	return _cmd.c_str();
}

