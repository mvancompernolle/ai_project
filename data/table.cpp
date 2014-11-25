#include "table.hpp"
/*!

*/
void Table::out(const Tuple &tuple){
	SerializingAgent  agent;
	std::stringstream stream;
	Tuple t;
	agent.serializeTuple(tuple,stream);
	agent.deserializeTuple(t,stream);
	tuple_vector.push_back(t);
}
/*!

*/
bool Table::rd(Tuple &anti_tuple){
	return  search(anti_tuple);
}
/*!

*/
bool Table::in(Tuple &anti_tuple){
	return search(anti_tuple,true);
}
/*!

*/
void Table::collect(Collection &collection,const Tuple &anti_tuple,const bool remove){
	SearchAgent agent;
	for(size_t index = 0; index < tuple_vector.size(); index++){
		if(agent.lookUp(anti_tuple,tuple_vector.at(index))){
			collection.addTuple(tuple_vector.at(index));
			if(remove){
				tuple_vector.erase(tuple_vector.begin() + index);
				index = index - 1;
			}
		}
	}
}
const unsigned int Table::query(Collection &collection,const Tuple &anti_tuple,const bool remove){
	SearchAgent agent;
	size_t      elements = anti_tuple.getQuerySize();
	size_t      counter  = 0;
	// check if we have enough tuples in this table
	if(tuple_vector.size() < anti_tuple.getQuerySize()){
		return 0;
	}
	for(size_t index = 0; index < tuple_vector.size();index++){
		if(agent.lookUp(anti_tuple,tuple_vector.at(index))){
			collection.addTuple(tuple_vector.at(index));
			counter++;
			if(remove){
				tuple_vector.erase(tuple_vector.begin() + index);
				index = index -1;
			}
			// if we have enough elements, return
			if(counter == elements){
				return counter;
			}
		}
	}
	// at this point, it's clear, that we don't find enough elements, therefore we must place all tuple back to the
	// table ( i.e. if we call this function with remove == true )
	if(remove){
		for(size_t index = 0; index < collection.getSize(); index++){
			tuple_vector.push_back(collection.getTuple(index));
		}
		collection.clear();
	}
	return NOT_ENOUGH_TUPLE;
}
const bool Table::search(Tuple &anti_tuple,const bool &remove){
	SearchAgent agent;
	for(size_t i = 0; i < tuple_vector.size(); i++){
		if(agent.lookUp(anti_tuple,tuple_vector.at(i))){
 			Tuple &tuple = tuple_vector.at(i);
 			std::vector<size_t> formals = anti_tuple.getFormals();
 			for(size_t j = 0; j < formals.size(); j++){
				anti_tuple.exchange_formal(formals.at(j),tuple.get(formals.at(j)));
 			}
			anti_tuple.setEachFieldToActual();
			anti_tuple.setQuerySize(0);
			anti_tuple.setCommand("");
			if(remove){
				tuple_vector.erase(tuple_vector.begin() + i);
			}
			return true;
		}
				
	}
	return false;
}
const size_t Table::size(void) const{
	return tuple_vector.size();
}