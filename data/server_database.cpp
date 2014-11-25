#include "server_database.hpp"


/*!
	This is the collect implementation.

	@param collection A reference to a collection
	@param anti_tuple A reference to an anti-tuple to create a collection
	@return method returns the number of matched tuples

*/
const size_t ServerDataBase::collect(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	return makeCollection(collection,      // collection object for tuples
			      anti_tuple,      // anti-tuple to find a tuple   
	                      true);           // true,therefore, we must remove all tuples from the table
}
/*!
	This is the copy-collect implementation.

	@param collection A reference to a collection
	@param anti_tuple A reference to an anti-tuple 
	@return methode returns the number of machted tuples 

*/
const size_t ServerDataBase::copycollect(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	return makeCollection(collection,      // collection object for tuples
			      anti_tuple,      // anti-tuple to find a tuple 
	                      false);          // false, therfore, we don't remove the tuples from the table
}
/*!
	This is the query implementation.The boost::threads that calls this function will suspend ( boost::condition ) until we have 
	enough tuple.

	@param collection A reference to a collection
	@param anti_tuple A reference to an anti-tuple
	@sa    queryp()
	@sa    copyquery()	

*/
void ServerDataBase::query(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	while(makeQueryCollection(collection,anti_tuple,true) != anti_tuple.getQuerySize()){
			_condi.wait(lock);
	}
}
/*!
	This is the queryp implementation, a none-suspending version of the query implementation.

	@param collection A reference to a collection 
	@param anti_tuple A reference to an anti-tuple 
	@return true or false, whethere the queryp call was successfull or not
	@sa query()

*/
const bool ServerDataBase::queryp(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	return (makeQueryCollection(collection,anti_tuple,true) > 0);
}
/*!
	This is the copy-query implementation., a non destructively implementation of the query implementation.The boost::thread that calls this function
 	will suspend, until we have enough tuple.

	@param collection A reference to a collection
	@param anti_tuple A reference to an anti-tuple
	@sa    copyqueryp()

*/
void ServerDataBase::copyquery(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	while(makeQueryCollection(collection,anti_tuple,false) != anti_tuple.getQuerySize()){
		_condi.wait(lock);
	}
}
/*!
	This is the copy-query-p implementation.

	@param	collection A reference to a collection
	@param  anti_tuple A reference to an anti tuple 

	@return true or false, whether we have enough machted tuple or not

*/
const bool ServerDataBase::copyqueryp(Collection &collection,const Tuple &anti_tuple){
	boost::mutex::scoped_lock lock(_mutex);
	return (makeQueryCollection(collection,anti_tuple,false) > 0);
}
/*!
	Function to create a query/queryp or copyquery/copyqueryp collection.

	@param collection A reference to a collection
	@param anti_tuple A reference to a collection
	@param remove     true, if we use this function in combination with query/queryp or false if we use the function in combination with copyquery/copyqueryp

	@sa	query()
	@sa	queryp()
	@sa	copyquery()
	@sa	copyqueryp()

	@return returns the number of machted tuple or 0, whether, we have found enough tuple or not

*/
const size_t ServerDataBase::makeQueryCollection(Collection &collection,const Tuple &anti_tuple,const bool remove){
	it = db.find(anti_tuple.getTableString());
	if(it != db.end()){
		Table &table = it->second;
		return table.query(collection,anti_tuple,remove);
	}
	return 0;
}
/*!

	Function to create a  collect or copy-collect collection

	@param collection A reference to a collection
	@param anti_tuple A reference to an anti-tuple 
	@param remove     true if we use this function in combination with collect or false if we use this function in combination with copycollect 

	@sa	collect()
	@sa	copycollect()

 */
const size_t ServerDataBase::makeCollection(Collection &collection,const Tuple &anti_tuple,const bool remove){
	it = db.find(anti_tuple.getTableString());
	if(it != db.end()){
		Table &table = it->second;
		table.collect(collection,anti_tuple,remove);
		return collection.size();
	}
	return 0;
}

