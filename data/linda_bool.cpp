#include "linda_bool.hpp"

using namespace LINDA::TYPE;

Bool::Bool()
:BASE_TYPES(true),_value(false)
{}
Bool::Bool(const bool value)
:BASE_TYPES(false),_value(value)
{}

Bool& Bool::operator=(const BASE_TYPES *object){
	const Bool *temp = dynamic_cast<const Bool*>(object);
	_value =  temp->_value;		
	toActual();
	return *this;
}
Bool& Bool::operator=(const bool &any){
	_value = any;
	toActual();
	return *this;
}
const bool Bool::operator()() const{
	return _value;
}
const bool Bool::hasSameValue(const BASE_TYPES &object) const{
	Bool v = dynamic_cast<const Bool&>(object);
	return ( v() == _value);
}
const std::string Bool::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<(_value == true ? " TRUE " : " FALSE ");
	}
	return stream.str();
}
const std::string Bool::type(void) const{
	return "boolean";
}
void Bool::reset(void){
	_value = false;
}
BOOST_CLASS_EXPORT(Bool)
