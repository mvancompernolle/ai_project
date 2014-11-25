#include "operation_query.hpp"

operation_query::~operation_query() {}

bool operation_query::createCollection(LocalSpace *ls,Tuple &tuple){
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
