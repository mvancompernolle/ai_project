#include "database.hpp"

void DataBase::in(Tuple &anti_tuple){
	// lock the mutex
	boost::mutex::scoped_lock lock(_mutex);
	while(tryToFind(anti_tuple,true) == false){
		_condi.wait(lock);
	}
}
const bool DataBase::inp(Tuple &anti_tuple){
	// lock the mutex
	boost::mutex::scoped_lock lock(_mutex);
	return tryToFind(anti_tuple,true);
}
void DataBase::rd(Tuple &anti_tuple){
	// lock the mutex
	boost::mutex::scoped_lock lock(_mutex);
	while(tryToFind(anti_tuple,false) == false){
		_condi.wait(lock);
	}
}
const bool DataBase::rdp(Tuple &anti_tuple){
	// lock the mutex
	boost::mutex::scoped_lock lock(_mutex);
	return tryToFind(anti_tuple,false);
}
void DataBase::out(const Tuple &tuple){
	boost::mutex::scoped_lock lock(_mutex);
	it = db.find(tuple.getTableString());
	// if we still have such a table, add the tuple to this table
	if(it != db.end()){
		Table &table =  it->second;
		table.out(tuple);
	}
	// else, create such a table
	else{
		Table table;
		// add the tuple
		table.out(tuple);
		// add the table 
		db[tuple.getTableString()] = table;
	}
	_condi.notify_all();
}
const bool DataBase::tryToFind(Tuple &anti_tuple,const bool remove){
	it = db.find(anti_tuple.getTableString());
	if(it != db.end()){
		Table &table = it->second;
		// if remove is true, we use the in operation
		if(remove){
			return table.in(anti_tuple);
		}
		// else, we use the rd operation
		else{
			return table.rd(anti_tuple);
		}
	}
	return false;
			
}
const size_t DataBase::size(void){
	boost::mutex::scoped_lock lock(_mutex);
	int size = 0;
	for(it = db.begin(); it != db.end(); ++it){
		size += it->second.size();
	}
	return size;
}

