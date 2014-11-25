#include "serializing_agent.hpp"


void SerializingAgent::serializeTuple(const Tuple &t,std::stringstream &str){
	boost::archive::text_oarchive oa(str);
	try
	{
		oa<<t;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
		exit(EXIT_FAILURE);
	}
}
void SerializingAgent::serializeCollection(const Collection &c,std::stringstream &str){
	boost::archive::text_oarchive oa(str);
	try{
		oa<<c;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}
void SerializingAgent::serilizeClientInfo(const ProcessInfo &p,std::stringstream &str){
	boost::archive::text_oarchive oa(str);
	try{
		oa<<p;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}
void SerializingAgent::serializeScopeInfo(const ScopeInfo &object,std::stringstream &str){
	boost::archive::text_oarchive oa(str);
	try{
		oa<<object;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}
void SerializingAgent::deserializeTuple(Tuple &tuple,std::stringstream &str){
	boost::archive::text_iarchive ia(str);
	try{
		ia>>tuple;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}
void SerializingAgent::deserializeClientInfo(ProcessInfo &p, std::stringstream &str){
	boost::archive::text_iarchive ia(str);
	try{
		ia>>p;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}
void SerializingAgent::deserializeScopeInfo(ScopeInfo &object,std::stringstream &str){
	boost::archive::text_iarchive ia(str);
	try{
		ia>>object;
	}
	catch(std::exception &ex){
		std::cerr<<ex.what()<<std::endl;
	}
}

