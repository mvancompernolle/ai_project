#include "linda_char.hpp"

using namespace LINDA::TYPE;
Char::Char()
:BASE_TYPES(true)
{}
Char::Char(const char c)
:BASE_TYPES(false),_c(c)
{}
Char& Char::operator=(const BASE_TYPES *object){
	const Char *temp = dynamic_cast<const Char*>(object);
	this->_c = temp->_c;
	toActual();
	return *this;
}
Char& Char::operator=(const char c){
	this->_c = c;
	toActual();
	return *this;
}
const bool Char::hasSameValue(const BASE_TYPES &object) const {
	const Char obj = dynamic_cast<const Char&>(object);
	return (obj() == this->_c);
}
const char Char::operator()() const{
	return _c;
}
const std::string Char::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		return type();
	}
	else{
		stream<<" "<<_c<<" ";
	}
	return stream.str();
}
const std::string Char::type(void) const{
	return "char";
}
void Char::reset(void){
	_c = 0;
}
BOOST_CLASS_EXPORT(Char)
