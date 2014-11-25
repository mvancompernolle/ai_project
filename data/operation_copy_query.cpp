#include "operation_copy_query.hpp"

operation_copy_query::~operation_copy_query(){
}

void operation_copy_query::createCollection(LocalSpace *ls,Tuple &tuple){
	sendTupleToTS(tuple);
	Collection collection;
	recvCollection(collection);
	for(unsigned int index = 0; index < collection.getSize(); index++){
		ls->outTuple(collection.getTuple(index));
	}
}
