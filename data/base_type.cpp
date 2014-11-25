#include "base_type.hpp"

using namespace LINDA::TYPE;

BASE_TYPES::BASE_TYPES(const bool anyValue)
:_anyValue(anyValue)
{}
/*!
	Operator to compare a tuple with a "template"
*/
const bool BASE_TYPES::operator==(const BASE_TYPES &object) const{
	// continue, if a tuple value has the same data type like the BASE_TYPE object
	if(_anyValue)
		return true;
	return hasSameValue(object);

// 	if(isOfType(object)){
// 		// if the template value can have any value from the same data type, return true
// 		if(_anyValue){
// 			return true;
// 		}
// 		// else, compare the values
// 		else{
// 			return hasSameValue(object);
// 		}
// 	}
// 	return false;
}
// virtual class to compare the typeid from deriverd classes
const bool BASE_TYPES::isOfType(const BASE_TYPES &object) const{
	return ( typeid(*this) == typeid(object));
}
BASE_TYPES::~ BASE_TYPES()
{}
// function, to set a value into a formal
void BASE_TYPES::toFormal(void){
	reset();
	_anyValue = true;
}
// function, to set a value into an actual
void BASE_TYPES::toActual(void){
	_anyValue = false;
}
const bool BASE_TYPES::isFormal(void) const{
	return _anyValue;
}
const bool BASE_TYPES::isActual(void) const{
	return (_anyValue == false);
}
