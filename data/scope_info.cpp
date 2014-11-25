#include "scope_info.hpp"

ScopeInfo::ScopeInfo(const ProcessInfo &process)
:_process(process)
{}
void ScopeInfo::setOperation(const std::string &operation){
	_operation = operation;
}
std::string ScopeInfo::getOperation(void) const{
	return _operation;
}
void ScopeInfo::setNew(const bool flag){
	_process.setToNewClient(flag);
}
const ProcessInfo ScopeInfo::getProcessObject(void) const{
	return _process;
}
void ScopeInfo::markAsDisconnected(){
	_process.markAsDisconnected();
}
void ScopeInfo::setTuple(const int &tuple){
	_tuple = tuple;
}
void ScopeInfo::setConnections(const int &connections){
	_connections = connections;
}
const int &ScopeInfo::getConnections(void) const{
	return _connections;
}
const int &ScopeInfo::getTuple(void) const{
	return _tuple;
}
const std::string ScopeInfo::getTupleString(void) const{
	return _tuplestring;
}
void ScopeInfo::setTupleString(const std::string &tupleString){
	_tuplestring = tupleString;
}
void ScopeInfo::showTupleString(const bool flag){
	if(flag == true){
		_show_tuplestring = true;
	}
	else{
		_show_tuplestring = false;
		_tuplestring      = "";
	}
}