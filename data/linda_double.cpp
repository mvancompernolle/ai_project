#include "linda_double.hpp"

using namespace LINDA::TYPE;

Double::Double()
:BASE_TYPES(true),_value(0.0)
{}
Double::Double(const double &value)
:BASE_TYPES(false),_value(value)
{}
Double& Double::operator=(const BASE_TYPES *object){
	const Double *temp = dynamic_cast<const Double*>(object);
	this->_value = temp->_value;
	toActual();
	return *this;
}
Double& Double::operator=(const double &value){
	this->_value = value;
	toActual();
	return *this;
}
const bool Double::hasSameValue(const BASE_TYPES &object) const {
	const Double obj = dynamic_cast<const Double&>(object);
	return (obj() == this->_value);
}
const double Double::operator()() const{
	return _value;
}
const std::string Double::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<" "<<_value<<" ";
	}
	return stream.str();
}
const std::string Double::type(void) const{
	return "double";
}
void Double::reset(void){
	_value = 0.0;
}
BOOST_CLASS_EXPORT(Double)
