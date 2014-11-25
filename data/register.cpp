#include "register.hpp"

void Register::addClient(const char *ip,const int pid){
	Client c(ip,pid);
	pool.push_back(c);
}
void Register::removeAt(const unsigned int index){
	pool.erase(pool.begin() + index);
}
const bool Register::hasMore(void) const{
	return (pool.size() > 0);
}
bool Register::removeClient(const char *ip, const int pid){
	Client temp(ip,pid);
	for(unsigned int i = 0; i < pool.size(); i++){
		if(pool.at(i).equals(temp)){
			pool.erase(pool.begin() + i);
			return true;
		}
	}
	return false;
}
const unsigned int Register::getSize(void) const{
	return pool.size();
}
const int &Register::getClientPID(unsigned int index) const {
	return pool.at(index).getClientPID();
}
const char *Register::getAddr(unsigned int index) const{
	return pool.at(index).getAddr();
}
Register::Client::Client(std::string ip, const int pid)
:ip(ip),pid(pid){}
const bool Register::Client::equals(Client &c){
	return (this->ip == c.ip && this->pid == c.pid);
}	
const char *Register::Client::getAddr(void) const{
	return ip.c_str();
}
const int &Register::Client::getClientPID(void) const{
	return pid;
}
