#include "operation_collect.hpp"

operation_collect::~operation_collect() {}

const unsigned int operation_collect::getCollectionSize(LocalSpace *ls,Tuple &tuple){
	sendTupleToTS(tuple);
	Collection collection;
	recvCollection(collection);
	for(unsigned int index = 0; index < collection.getSize(); index++){
		ls->outTuple(collection.getTuple(index));
	}
	return collection.getSize();
}
