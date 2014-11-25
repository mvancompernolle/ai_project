#include "linda_short.hpp"

using namespace LINDA::TYPE;
Short::Short()
:BASE_TYPES(true),
_value(0){}
		    
Short::Short(const short value)
:BASE_TYPES(false),_value(value)
{}

Short& Short::operator=(const BASE_TYPES *object){
    const Short *temp = dynamic_cast<const Short*>(object);
    _value = temp->_value;
    toActual();
    return *this;
}
Short& Short::operator=(const short &anyShort){
    _value = anyShort;
    toActual();
    return *this;
}
const bool Short::hasSameValue(const BASE_TYPES &object) const{
	const Short &v = dynamic_cast<const Short&>(object);
	return ( v() == _value);
}
const short Short::operator()() const{
	return _value;
}
const std::string Short::getTypeOrValue(void) const{
	std::stringstream stream;
	if(isFormal()){
		stream<<" SHORT ";
	}
	else{
		stream<<" "<<_value<<" ";
	}
	return stream.str();
}
const std::string Short::type(void) const{
	return "signed short";
}
void Short::reset(void){
	_value = 0;
}
BOOST_CLASS_EXPORT(Short)

