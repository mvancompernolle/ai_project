#include "search_agent.hpp"
SearchAgent::SearchAgent()
{}
bool SearchAgent::lookUp(const Tuple &anti_tuple,const Tuple &tuple){
 	const std::vector<int> &actuals = anti_tuple.getActuals();
 	for(size_t index = 0; index < actuals.size(); index++){
 		const size_t field = actuals.at(index);
		if((*anti_tuple.get(field) == *tuple.get(field)) == false){
			return false;
		}
 	}
	return true;
}

