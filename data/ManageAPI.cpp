#include "ManageAPI.h"
#include "Utils.h"
#include "errlog.h"

namespace CAST {

  void Response::dump()
  {
    std::vector<std::string> keys;
    int num_msg;
    unsigned int i;
    std::string val;
    keys = getListOfVars();
    printf("===================================\n");
    
    printf("total %d key pairs\n", keys.size());
    for (i=0; i<keys.size(); i++)
      {
	val = getVal(keys[i]);
	printf("<Key> %s <Val> %s \n", keys[i].c_str(), val.c_str());
      }
    num_msg= getNumMsgs();
    printf("Total %d messages:\n", num_msg);
  
    for (i=0; i<num_msg; i++)
      {
	val=getMsg(i);
	printf("<Msg %d> %s\n", i, val.c_str()); 
      }
    fflush(NULL);
  }
  const std::string  Response::getVal(const char* var_name)
  {
    if (var_name!=NULL)
      return getVal(std::string(var_name));
    else
      return std::string("");
  }

  const std::string Response::getVal(const std::string var_name)
  {
    std::map<std::string, std::string>::iterator iter;
    
    iter=var_vals_.find(var_name);
    
    if (iter!=var_vals_.end())
      return iter->second;
    else
      return std::string("");

  }

  const std::string Response::getMsg(unsigned int ind)
  {
    if (ind<msgs_.size())
      return msgs_[ind];
    else
      return std::string("");
  }

  const std::vector<std::string> Response::getListOfVars()
  {
    std::map<std::string, std::string>::iterator iter;
    std::vector<std::string> result;

    for (iter=var_vals_.begin(); iter!=var_vals_.end(); iter++)
      result.push_back(iter->first);
    
    return result;
  }

  unsigned int Response::getNumMsgs()
  {
    return msgs_.size();
  }

  Response::Response(const std::map<std::string, std::string> & keyvals, 
	     const std::vector<std::string> & msgs)
  {
    var_vals_=keyvals;
    msgs_=msgs;
  }

  Response::Response(const Response & response)
  {
    var_vals_=response.var_vals_;
    msgs_=response.msgs_;
  }

  Manager_API::~Manager_API() 
  { 
    cast_checkpoint();
    ACE_Reactor::instance ()->end_reactor_event_loop (); 
    ast_sock_handler_.wait();

  }
 
  Response Manager_API::proc_response(char* response)
  {
    cast_checkpoint();
    int res_len;
    char* buf;
    std::vector<std::string> tokens;
    int i,j;
    unsigned int k;
    std::string val;
    unsigned int num_toks;
    std::map<std::string, std::string> var_vals;
    std::vector<std::string> msgs;
    
    if (response==NULL)
      {
	return Response(var_vals, msgs);
	//something seriouse wrong or no response (like the events on command
      };
    
    res_len = strlen(response);
    if (res_len==0)
      {
	return Response(var_vals, msgs);
	//something seriouse wrong or no response (like the events on command
      }
    j=0;
    
    for (i=0; i<res_len-1; i++)
      if (response[i]=='\r' && response[i+1]=='\n')
	{
	  buf= new char[(i-j)+1];
	  strncpy(buf, &(response[j]), (i-j));
	  buf[(i-j)]='\0';
	  num_toks=Utils::get_tokens(buf, tokens,":");
	  
	  if (num_toks>=2)
	    {
	      val="";
	      for (k=1; k<num_toks; k++)
		val+=tokens[k];
	      val=val.substr(1,val.length()-1); //get rid of the leading whitespace after :
	      var_vals[tokens[0]]=val;
	    }
	  else if (num_toks==1)
	    msgs.push_back(tokens[0]);
	  else
	    ; //this is the package ending /r/n

	  j=(i+2); //if (i+2> res_len, it will be catched in the for loop)
	  i=i+1; //now response[i]='\n'the i++ in the for loop will move it to the next char
	  delete [] buf; //release buf memory;
	};
    
    return Response(var_vals, msgs);
  }
  
  Response Manager_API::absoluteTimeout(const char* channel, int timeout, const char* actionId, char* msg)
  {
    cast_checkpoint();
    char buf[256];
    
    std::string action_msg="ACTION: AbsoluteTimeout\r\n";
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    action_msg+="Timeout: ";
    sprintf(buf, "%d", timeout);
    action_msg+=buf;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;

    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(msg);
    
  }
  
  Response Manager_API::changeMonitor(const char* channel, const char* file, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ChangeMonitor\r\n";
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    action_msg+="File: ";
    action_msg+=file;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
      return proc_response(response_msg);
  }
  
  Response Manager_API::command(const char* command, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Command\r\n";
    action_msg+="command: ";
    action_msg+=command;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::events(bool eventMask, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Events\r\n";
    action_msg+="EVENTMASK: ";
    if (eventMask)
      action_msg+="ON";
    else
      action_msg+="OFF";
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    //entMask ON does not return any immediate response, in this example below, 
    //ere was nothing returned until the device unregistered (Asterisk 1.0.9): 
    if (eventMask)
      exec_action(action_msg.c_str(), response_msg, false); //no response will be expected
    else
      exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::extensionState(const char* exten, const char* context,const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ExtensionState\r\n";
    action_msg+="Context: ";
    action_msg+=context;
    action_msg+="\r\n";
    
    action_msg+="Exten: ";
    action_msg+=exten;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }

  Response Manager_API::getVar(const char* channel, const char* variable, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: GetVar\r\n";
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    action_msg+="Variable: ";
    action_msg+=variable;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::hangUp(const char* channel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Hangup\r\n";
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
        
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::iaxPeers(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: IaxPeers\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back, or timeout in case of no peers are existing 
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::listCommands(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ListCommands\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;
    
    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg_);
    
  }
  
  Response Manager_API::login(const char* username, const char* password, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Login\r\n";
    
    action_msg+="Username: ";
    
    if (username!=NULL)
      action_msg+=username;
    action_msg+="\r\n";
    
    action_msg+="Secret: ";
    action_msg+=password;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::logOff(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Logoff\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
//    ACE_Reactor::instance ()->end_reactor_event_loop (); 
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::mailboxCount(const char* mailbox, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: MailboxCount\r\n";
    
    action_msg+="Mailbox: ";
    action_msg+=mailbox;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::mailboxStatus(const char* mailbox, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: MailboxStatus\r\n";
    
    action_msg+="Mailbox: ";
    action_msg+=mailbox;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }

  Response Manager_API::monitor(const char* channel, const char* file, const char* format,
			    bool mix, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Monitor\r\n";
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    if (file!=NULL)
      {
	action_msg+="File: ";
	action_msg+=file;
	action_msg+="\r\n";
      }
    
    if (format!=NULL && (!strcmp(format,"wav")||!strcmp(format,"gsm")||!strcmp(format,"mp3")))
      {
	action_msg+="Format: ";
	action_msg+=format;
	action_msg+="\r\n";
      }
    
    if (mix)
      {
	action_msg+="Mix: ";
	action_msg+="1";
	action_msg+="\r\n";
      }
    else
      {
	action_msg+="Mix: ";
	action_msg+="0";
	action_msg+="\r\n";
      }
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;
    
    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
        
  }
  
  Response Manager_API::originate(const char* channel, const char* exten,
			      const char* context, const char* priority, const char* application, 
			      const char* data,long timeout, const char* callerID, const char* variable, 
			      const char* account, bool async, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Originate\r\n";
    char buf[256];
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    if (exten!=NULL&&context!=NULL&&priority!=NULL)
      {
	action_msg+="Exten: ";
	action_msg+=exten;
	action_msg+="\r\n";
	
	action_msg+="Context: ";
	action_msg+=context;
	action_msg+="\r\n";
	
	action_msg+="Priority: ";
	action_msg+=priority;
	action_msg+="\r\n";
	
      }
    
    if (application!=NULL)
      {
	action_msg+="Application: ";
	action_msg+=application;
	action_msg+="\r\n";
	
	if (data!=NULL)
	  {
	    action_msg+="Data: ";
	    action_msg+=data;
	    action_msg+="\r\n";
	  }
      }
    
    if (timeout>0) 
      {
	action_msg+="Timeout: ";
	sprintf(buf,"%ld", timeout);
	action_msg+=buf;
	action_msg+="\r\n";
      }
    
    if (callerID!=NULL)
      {
	action_msg+="CallerID: ";
	action_msg+=callerID;
	action_msg+="\r\n";
      }
    
    if (variable!=NULL)
      {
	action_msg+="Variable: ";
	action_msg+=variable;
	action_msg+="\r\n";
      }
    
    if (account!=NULL)
      {
	action_msg+="Account: ";
	action_msg+=account;
	action_msg+="\r\n";
      }
    
    if (async)
      {
	action_msg+="Async: ";
	action_msg+="1";
	action_msg+="\r\n";
      }
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;
    
    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::parkedCalls(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ParkedCalls\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::ping(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Ping\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;
    
    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    //the returned message should be "PONG"
    return proc_response(response_msg);
  }
  
  Response Manager_API::queueAdd(const char* queue, const char* interfaceId,
			     unsigned int penalty, bool isPaused, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: QueueAdd\r\n";
    char buf[256];
    
    action_msg+="Queue: ";
    action_msg+=queue;
    action_msg+="\r\n";
    
    action_msg+="Interface: ";
    action_msg+=interfaceId;
    action_msg+="\r\n";
    
    if (penalty>0)
      {
	action_msg+="Penalty: ";
	sprintf(buf,"%u",penalty);
	action_msg+=buf;
	action_msg+="\r\n";
      }
    
    if (isPaused)
      {
	action_msg+="Paused: ";
	action_msg+="1";
	action_msg+="\r\n";
      }
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::queueRemove(const char* queue, const char* interfaceId, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: QueueRemove\r\n";
    
    action_msg+="Queue: ";
    action_msg+=queue;
    action_msg+="\r\n";
    
    action_msg+="Interface: ";
    action_msg+=interfaceId;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
   
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::queues(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Queues\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::queueStatus(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: QueueStatus\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::redirect(const char* channel, const char* extraChannel,
			     const char* exten, const char* context, unsigned int priority,
			     const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Redirect\r\n";
    char buf[256];
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    if (extraChannel!=NULL)
      {
	action_msg+="ExtraChannel: ";
	action_msg+=extraChannel;
	action_msg+="\r\n";
      };
    
    action_msg+="Exten: ";
    action_msg+=exten;
    action_msg+="\r\n";
    
    action_msg+="Context: ";
    action_msg+=context;
    action_msg+="\r\n";
    
    if (priority>0)
      sprintf(buf, "%u", priority);
    else
      strcpy(buf,"1");
    
    action_msg+="Priority: ";
    action_msg+=buf;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg_);
    
  }
  
  Response Manager_API::setCDRUserField(const char* userField, const char* channel, const char* append,
				    const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: SetCDRUserField\r\n";
    
    action_msg+="UserField: ";
    action_msg+=userField;
    action_msg+="\r\n";
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    if (append!=NULL)
      {
	action_msg+="Append: ";
	action_msg+=append;
	action_msg+="\r\n";
      }
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::setVar(const char* channel, const char* variable, const char* value, 
			   const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: SetVar\r\n";
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    action_msg+="Variable: ";
    action_msg+=variable;
    action_msg+="\r\n";
    
    action_msg+="Value: ";
    action_msg+=value;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::status(const char* channel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: Status\r\n";
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::stopMonitor(const char* channel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: StopMonitor\r\n";
    
    action_msg+="Channel: ";
    action_msg+=channel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;
    
    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::zapDialOffHook(const char* zapChannel, const char* number, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapDialOffHook\r\n";
    
    action_msg+="ZapChannel: ";
    action_msg+=zapChannel;
    action_msg+="\r\n";
    
    action_msg+="Number: ";
    action_msg+=number;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg_);
  }
  
  Response Manager_API::zapDNDOff(const char* zapChannel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapDNDoff\r\n";
    
    action_msg+="ZapChannel: ";
    action_msg+=zapChannel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  Response Manager_API::zapDNDOn(const char* zapChannel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapDNDon\r\n";
    
    action_msg+="ZapChannel: ";
    action_msg+=zapChannel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
  
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::zapHangUp(const char* zapChannel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapHangUp\r\n";
    
    action_msg+="ZapChannel: ";
    action_msg+=zapChannel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);

  }
  
  Response Manager_API::zapShowChannels(const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapShowChannels\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;

    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
    
  }
  
  Response Manager_API::zapTransfer(const char* zapChannel, const char* actionId, char* msg)
  {
    cast_checkpoint();
    std::string action_msg="ACTION: ZapTransfer\r\n";
    
    action_msg+="ZapChannel: ";
    action_msg+=zapChannel;
    action_msg+="\r\n";
    
    add_action_id(action_msg, actionId);
    
    action_msg+="\r\n"; //the endian CRLF;
    
    char* response_msg;
    if (msg!=NULL)
      response_msg=msg;
    else
      response_msg=response_msg_;

    exec_action(action_msg.c_str(), response_msg); //this is a block call, it returns untill it got something back
    
    return proc_response(response_msg);
  }
  
  void Manager_API::add_action_id(std::string& action_msg, const char* actionId)
  {
    std::string my_id="ActionID: ";
    cast_checkpoint();
    //this function will try to generate an unique id if the passed in actionId is NULL
        
    if (actionId!=NULL && std::string(actionId)!="")
      my_id+=std::string(actionId);
    else
      my_id+= Utils::get_uniqueid("Ast");
    
    action_msg+=my_id;
    action_msg+="\r\n";
    
  }
  
  void Manager_API::exec_action(const char* action, char* response, bool is_response)
  {
    cast_debug(2, action);
    cast_checkpoint();
    ast_sock_handler_.send_msg(action);
    if (is_response) //does expecting a response for this action
      ast_sock_handler_.get_reply(response);
    else
      response=NULL;
  }
  
  void Manager_API::open(const char* addr)
  {
    cast_checkpoint();
    ast_sock_handler_.activate(); //activate the ast sock handler's thread
    Utils::seed_uniqueid();
    fflush(NULL);
    if (ast_sock_handler_.init(addr)<0) //set the asterisk server address and start connnection
      cast_critic_err(("Open Ast Server at %s FAILED!\n" ,addr));

    
  }
} //end of namespace CAST
