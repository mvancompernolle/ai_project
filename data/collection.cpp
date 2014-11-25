#include "collection.hpp"

/*!
	Function to add a tuple to the collection
*/
void Collection::addTuple(const Tuple &t){
	tupleVector.push_back(t);
}
/*!
	Function to get the size of the collection

	@return collection size
*/
const unsigned int Collection::getSize(void) const{
	return tupleVector.size();
}
/*!
	Function to get a tuple from the collection

	@return reference to a tuple
*/
const Tuple &Collection::getTuple(const unsigned int index) const{
	assert(index <= tupleVector.size());
	return tupleVector.at(index);
}
/*!
	Function to remove all elements
*/
void Collection::clear(void){
	tupleVector.clear();
}

const size_t Collection::size(void) const{
	return tupleVector.size();
}
