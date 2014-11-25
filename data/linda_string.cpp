#include "linda_string.hpp"

using namespace LINDA::TYPE;
String::String()
:BASE_TYPES(true),_string("")
{}
String::String(const std::string &value)
:BASE_TYPES(false),_string(value)
{}
String& String::operator=(const BASE_TYPES *object){
	const String *temp = dynamic_cast<const String*>(object);
	this->_string = temp->_string;
	toActual( );
	return *this;
}
String& String::operator=(const std::string &anyString){
	this->_string = anyString;
	toActual( );
	return *this;
}	
String& String::operator=(const char *c_ptr){
	this->_string = c_ptr;
	toActual();
	return *this;
}
const bool String::hasSameValue(const BASE_TYPES &object) const {
	const String str = dynamic_cast<const String&>(object);
	return ( strcmp(str().c_str(),this->_string.c_str()) == 0);
}
const std::string String::operator()() const{
	return _string;
}
const std::string String::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<" "<<_string<<" ";
	}
	return stream.str();
}
const std::string String::type(void) const{
	return "string";
}
void String::reset(void){
	_string = "";
}
BOOST_CLASS_EXPORT(String)
