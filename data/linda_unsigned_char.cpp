#include "linda_unsigned_char.hpp"

using namespace LINDA::TYPE;
UChar::UChar()
:BASE_TYPES(true),_uc(' ')
{}
UChar::UChar(const unsigned char &uc)
:BASE_TYPES(false),_uc(uc)
{}
UChar& UChar::operator=(const BASE_TYPES *object){
	const UChar *temp = dynamic_cast<const UChar*>(object);
	this->_uc = temp->_uc;
	toActual();
	return *this;
}
UChar& UChar::operator=(const unsigned char &_uc){
	this->_uc = _uc;
	toActual();
	return *this;
}
const bool UChar::hasSameValue(const BASE_TYPES &object) const {
	const UChar obj = dynamic_cast<const UChar&>(object);
	return (obj() == this->_uc);
}
const unsigned char UChar::operator()() const{
	return _uc;
}
const std::string UChar::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<" "<<_uc<<" ";
	}
	return stream.str();
}
const std::string UChar::type(void) const{
	return "unsigned char";
}
void UChar::reset(void){
	_uc = 0;
}
BOOST_CLASS_EXPORT(UChar)

