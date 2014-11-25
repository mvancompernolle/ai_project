#include "operation_copy_queryp.hpp"

operation_copy_query_p::~operation_copy_query_p() {}

bool operation_copy_query_p::createCollection(LocalSpace *ls,Tuple &tuple){
	sendTupleToTS(tuple);
	Collection collection;
	recvCollection(collection);
			// this is just important for the predicated forms of query and copy-query
	if(collection.getSize() == 0)
		return false;
	for(unsigned int index = 0; index < collection.getSize(); index++){
		ls->outTuple(collection.getTuple(index));
	}
	return true;
}
