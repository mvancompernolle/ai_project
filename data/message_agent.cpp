#include "message_agent.hpp"


bool MessageAgent::firstInstance = true;

boost::mutex MessageAgent::message_mutex;
boost::mutex MessageAgent::instance_mutex;
std::string MessageAgent::hostname;

MessageAgent::MessageAgent(){
	boost::mutex::scoped_lock lock(instance_mutex);
	if(MessageAgent::firstInstance){
		char host[256];
		gethostname(host,255);
		hostname = host;
		firstInstance = false;
	} 
}

void MessageAgent::print_info(const std::string message_1,const std::string message_2,const std::string message_3){
	boost::mutex::scoped_lock lock(message_mutex);
	std::clog<<"["<<std::setw(9)<<getpid()<<"@"<<hostname<<"] open linda message : "<<message_1<<" "<<message_2<<" "<<message_3<<std::endl;
}

void MessageAgent::print_error(const std::string message_1,const std::string message_2,const std::string message_3){
	boost::mutex::scoped_lock lock(message_mutex);
	std::cerr<<"["<<std::setw(9)<<getpid()<<"@"<<hostname<<"] open linda error   : "<<message_1<<" "<<message_2<<" "<<message_3<<std::endl;
}

