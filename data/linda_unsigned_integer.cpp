#include "linda_unsigned_integer.hpp"

using namespace LINDA::TYPE;

UInteger::UInteger()
:BASE_TYPES(true),_uvalue(0)
{}
UInteger::UInteger(const unsigned int &uvalue)
:BASE_TYPES(false),_uvalue(uvalue)
{}
UInteger& UInteger::operator=(const BASE_TYPES *object){
	const UInteger *temp = dynamic_cast<const UInteger*>(object);
	this->_uvalue = temp->_uvalue;
	toActual();
	return *this;
}
UInteger& UInteger::operator=(const unsigned int &uvalue){
	this->_uvalue = uvalue;
	toActual();
	return *this;
}
const bool UInteger::hasSameValue(const BASE_TYPES &object) const {
	const UInteger obj = dynamic_cast<const UInteger&>(object);
	return (obj() == this->_uvalue);
}
const unsigned int UInteger::operator()() const{
	return _uvalue;
}
const std::string UInteger::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<" "<<_uvalue<<" ";
	}
	return stream.str();
}
const std::string UInteger::type(void) const{
	return "unsigned integer";
}
void UInteger::reset(void){
	_uvalue = 0;
}
BOOST_CLASS_EXPORT(UInteger)
