#include "localspace.hpp"

void LocalSpace::outTuple(const Tuple &tuple){
	db.out(tuple);
}
void LocalSpace::inTuple(Tuple &anti_tuple){
	db.in(anti_tuple);
	anti_tuple.setEachFieldToActual();
}
const bool LocalSpace::inpTuple(Tuple &anti_tuple){
	if(db.inp(anti_tuple)){
		anti_tuple.setEachFieldToActual();
		return true;
	}
	return false;
}
const bool LocalSpace::rdpTuple(Tuple &anti_tuple){
	if(db.rdp(anti_tuple)){
		anti_tuple.setEachFieldToActual();
		return true;
	}
	return false;
}
void LocalSpace::rdTuple(Tuple &anti_tuple){
	db.rd(anti_tuple);
}
