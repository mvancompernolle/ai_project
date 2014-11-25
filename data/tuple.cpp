#include "tuple.hpp"

using namespace LINDA::TYPE;
Tuple::Tuple()
:querySize(0),cmd(""){}

void Tuple::add(BASE_TYPES *object){
	tuple_vector.push_back(object);
}
const BASE_TYPES* Tuple::get(const size_t &index) const{
	return tuple_vector.at(index);
}
const unsigned int Tuple::getSize(void) const{
	return tuple_vector.size();
}
Tuple::~Tuple(){
	tuple_vector.clear();
}
const std::string Tuple::getCommand(void) const{
	return cmd;
}
void Tuple::setCommand(const std::string command){		
	cmd = command;
}
void Tuple::setEachFieldToActual(void){
	for(unsigned int i = 0; i < tuple_vector.size(); i++){
		tuple_vector.at(i)->toActual();
	}
}
void Tuple::setQuerySize(unsigned int size){
	querySize = size;
}
const unsigned int Tuple::getQuerySize(void) const{
	return querySize;
}
/*!
	It's not allowed allowed to use the out() operation if a tuple has formals.
	Therefore, this function checks the contents of a tuple before we place 
	the tuple into tuple space.

	@return true or false, if we have formals or not
*/
const bool Tuple::hasNoFormals(void) const{
	for(unsigned int index = 0; index < tuple_vector.size();index++){
		if(tuple_vector.at(index)->isFormal()){
			return false;
		}
	}
	return true;
}
const std::string Tuple::getTupleString(void) const{
	std::stringstream stream;
	stream<<"[";
	for(unsigned int i = 0; i < tuple_vector.size(); i++){
		stream<<tuple_vector.at(i)->getTypeOrValue();
	}
	stream<<"]";
	return stream.str();
}
const std::string Tuple::getTableString(void) const{
	std::stringstream stream;
	for(unsigned int i = 0; i < tuple_vector.size();i++){
		stream<<"|"<<tuple_vector.at(i)->type();
	}
	stream<<"|";
	return stream.str();
}
/*!
	@brief function to find actuals the tuple 
*/
void Tuple::setActuals(void){
	for(size_t index = 0; index < tuple_vector.size(); index++){
		if(tuple_vector.at(index)->isFormal() == false ) {
			actuals.push_back(index);
		}
	}
	setFormals();
}
/*!
	@brief function returns a std::vector with the index of each actuals in the tuple
*/
const std::vector<int> &Tuple::getActuals(void) const{
	return actuals;
}
void Tuple::setFormals(void){
	for(size_t index = 0; index < tuple_vector.size(); index++){
		if(tuple_vector.at(index)->isFormal()){
			formals.push_back(index);
		}
	}
}
const std::vector<size_t> Tuple::getFormals(void) const{
	return formals;
}
void Tuple::exchange_formal(const size_t index, const BASE_TYPES *object){
	tuple_vector.at(index) = const_cast<BASE_TYPES*>(object);
}
