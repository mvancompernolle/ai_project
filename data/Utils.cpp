#include "Utils.h"
//ACE includes
#include "ace/OS.h"
#include <ace/Time_Value.h> 
#include <ace/UUID.h> 

#include <assert.h>
#include <stdio.h>
#include "errlog.h"

namespace CAST
{
  //define static data member in file scope
  unsigned long Utils::unique_counter_; 

  std::string Utils::unique_seed_;

  unsigned int Utils::get_tokens(const char* buf, std::vector<std::string> &tokens, const char* delim)
  {
    unsigned int len = strlen(buf);
    unsigned int d_len = strlen(delim);
    char *temp_buf=NULL;
    
    cast_checkpoint();

    tokens.clear();
    if (len==0)
      return 0;

    if (d_len==0)
      {
	tokens.push_back(buf);
	return 1; //the whol string is the only deliminator
      }

    temp_buf = new char[len+1];
    assert(temp_buf);  //memory alloc failed
    
    unsigned i,j,k;
    bool is_delim;
    j=0;
    for (i=0; i<len; i++)
      {
	//now check if the current char is one of the deliminators
	is_delim=false;
	for (k=0; k<d_len; k++)
	  if (buf[i]==delim[k])
	    is_delim=true;

	if (is_delim)
	  {
	    if (j>0) //not leading deliminators
	      {
		temp_buf[j]='\0';
		tokens.push_back(temp_buf);
		j=0;
	      }
	    else
	      continue; //leading deliminators, just skip
	  }
	else
	  temp_buf[j++]=buf[i];
      };
    
    if (j>0)
      {
	temp_buf[j]='\0';
	tokens.push_back(temp_buf);
      }
    
    if (temp_buf!=NULL)
      delete [] temp_buf; //release memory

    return tokens.size();
  }

  
  bool Utils::match(const char* str1, const char* str2, bool caseSensitive)
  {
    char *cmp_str1=NULL;
    char *cmp_str2=NULL;
    bool result;
    unsigned int str1_len, str2_len;
    unsigned int i;

    cast_checkpoint();

    if (!caseSensitive)
      {
	str1_len = strlen(str1);
	str2_len = strlen(str2);

	if (str1_len!=str2_len)
	  return false;

	if (str1_len==0&&str2_len==0)
	  return true;
	
	//if gets here, str1_len==str2_len
	cmp_str1 = new char[str1_len+1];

	assert(cmp_str1);// memory alloc error, false by default
	
	cmp_str2 = new char[str2_len+1];
	assert(cmp_str2); //memory alloc error, false by default
	
	for (i=0; i<str1_len; i++)
	  {
	    cmp_str1[i]=(char) toupper(str1[i]);
	    cmp_str2[i]=(char) toupper(str2[i]);
	  }
	cmp_str1[i]=cmp_str2[i]='\0'; //terminate the c-string
	result = !((bool) (strcmp(str1, str2)));
	if (cmp_str1!=NULL)
	  delete [] cmp_str1; //release memory
	if (cmp_str2!=NULL)
	  delete [] cmp_str2; //release memory
      }
    else
      result = !((bool) (strcmp(str1, str2)));
	
    return result;
  }

  void Utils::seed_uniqueid()
  {
    //seed the unique id with three different parts;
    //garantee unique for the same machine,
    //may not be unique global, but the clash chance are small
    /*ACE_Time_Value tv;
    char part1[80];
    char part2[80];
    char part3[80];
    char buf[80];

    cast_checkpoint();

	tv=ACE_OS::gettimeofday();
 
    sprintf(part1, "%ld",tv.sec()); //part 1

    srandom(int(tv.usec())); 

    sprintf(part2, "%ld", random()); //part 2;

    sprintf(part3,"%d", getpid()); //part3;
    
    unique_seed_="";
    unique_seed_=unique_seed_+part1+"."
      +part2+"."
      +part3+".";
    */

    //here try the ACE's UUID 
	ACE_Utils::UUID_Generator uuid_gen;
	ACE_Utils::UUID seed_uuid;
	uuid_gen.generateUUID(seed_uuid); 
	unique_seed_=seed_uuid.to_string()->c_str();
	unique_seed_+=".";
    //unique_counter_=0;
    //sprintf(buf, "%ld", unique_counter_);
    //unique_seed_+=buf;

    cast_debug(0, "Unique seed is : %s", unique_seed_.c_str());
    
  }

  std::string Utils::get_uniqueid(const char* tag)
  {
    char buf[80];
    std::string result="";
    sprintf(buf, "%ld.",++unique_counter_);

    result=unique_seed_+buf+tag;

    return result;
  }

}//end of namespace 
