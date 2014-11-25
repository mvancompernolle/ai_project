#include "linda_integer.hpp"

using namespace LINDA::TYPE;
Integer::Integer()
:BASE_TYPES(true),
_value(0)
{}
Integer::Integer(const int value)
:BASE_TYPES(false),
_value(value)
{}
Integer& Integer::operator=(const BASE_TYPES *object){
	const Integer *temp = dynamic_cast<const Integer*>(object);
	this->_value = temp->_value;
	toActual();
	return *this;
}
Integer& Integer::operator=(const Integer &anyInteger){
	this->_value = anyInteger._value;
	toActual();
	return *this;
}
Integer& Integer::operator=(const int &anyInteger){
	_value = anyInteger;
	toActual();
	return *this;
}
const bool Integer::hasSameValue(const BASE_TYPES &object) const{
	Integer v = dynamic_cast<const Integer&>(object);
	return ( v() == _value);
}
const int Integer::operator()() const{
	return _value;
}
const std::string Integer::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		stream<<" INTEGER ";
	}
	else{
		stream<<" "<<_value<<" ";
	}
	return stream.str();
}
const std::string Integer::type(void) const{
	return "integer";
}
void Integer::reset(void){
	_value = 0;
}
BOOST_CLASS_EXPORT(Integer)
