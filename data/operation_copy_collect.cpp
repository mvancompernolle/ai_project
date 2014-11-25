#include "operation_copy_collect.hpp"

operation_copy_collect::~operation_copy_collect(){}

const unsigned int operation_copy_collect::getCollectionSize(LocalSpace *ls,Tuple &tuple){
	sendTupleToTS(tuple);
	Collection collection;
	recvCollection(collection);
	for(unsigned int index = 0; index < collection.getSize(); index++){
		ls->outTuple(collection.getTuple(index));
	}
	return collection.getSize();
}

