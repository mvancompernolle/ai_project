#include "linda_float.hpp"

using namespace LINDA::TYPE;
Float::Float()
:BASE_TYPES(true),_value(0.0)
{}
Float::Float(const float &value)
:BASE_TYPES(false),_value(value)
{}
Float& Float::operator=(const BASE_TYPES *object){
	const Float *temp = dynamic_cast<const Float*>(object);
	_value = temp->_value;
	toActual();
	return *this;
}
Float& Float::operator=(const float &f){
	_value = f;
	toActual();
	return *this;
}
const float Float::operator()() const{
	return _value;
}
const bool Float::hasSameValue(const BASE_TYPES &object) const{
	Float v = dynamic_cast<const Float&>(object);
	return ( v() == _value);
}
const std::string Float::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		stream<<" FLOAT ";
	}
	else{
		stream<<" "<<_value<<" ";
	}
	return stream.str();
}
const std::string Float::type(void) const{
	return "float";
}
void Float::reset(void){
	_value = 0.0;
}
BOOST_CLASS_EXPORT(Float)
