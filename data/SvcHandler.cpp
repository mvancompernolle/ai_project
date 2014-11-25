/**
 * Client.cpp,v 1.3 2004/01/07 22:40:16 shuston Exp
 *
 * A simple client program using ACE_Svc_Handler and ACE_Connector.
 */

#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_sys_time.h"

#include "SvcHandler.h"
#include "errlog.h"

#define ASTBUF_SIZE 1025

namespace CAST 
{
  int SvcHandler::open (void *p)
  {
    cast_checkpoint();
    //this open set up the notifier for the message queue
    //so the queue can be used to stack up message to be sent.
    ACE_Time_Value iter_delay (2);   
    ACE_Time_Value initial_delay (5);   
    // Two seconds, the ping message doens't need to be invoke right away
    
    if (super::open (p) == -1)
      return -1;
    
    this->notifier_.reactor (this->reactor ());
    
    this->msg_queue ()->notification_strategy (&this->notifier_);
    
    return this->reactor ()->schedule_timer(this, 0, initial_delay, iter_delay);
  }
  
  int SvcHandler::handle_input (ACE_HANDLE)
  {
    cast_checkpoint();
    char buf[ASTBUF_SIZE];
    ssize_t recv_cnt = this->peer ().recv (buf, sizeof (buf) - 1);

    buf[recv_cnt]='\0'; //make it a null terminated string 
    
    if (recv_cnt > 0)
      cast_debug(0,"%s",buf); //echo the received string;
    
    if (recv_cnt == 0 && ACE_OS::last_error () != EWOULDBLOCK)
      {
		//this->reactor()->end_reactor_event_loop ();
	//return -1;
	 ACE_ERROR_RETURN ((LM_ERROR,
                    ACE_TEXT ("(%P|%t) %p\n"),
			    ACE_TEXT ("recv")),-1);
		return -1;
      }

    //cast_debug(0,"%s", buf);
    recv_buf_+=buf; //add the current buf to the cumulated recv string;
    
    std::string::size_type end_packet;
    
    while ((end_packet=recv_buf_.find("\r\n\r\n"))!=std::string::npos)
      {
	unsigned int recv_buf_len = recv_buf_.length();
	
	std::string packet=recv_buf_.substr(0,end_packet+4); 
	//packet has the full packet
	recv_buf_=recv_buf_.substr(end_packet+4, recv_buf_len - end_packet-4); 
	//put the remains back into the recv_buf
	
	ACE_Message_Block *mb;
	ACE_NEW_RETURN(mb, ACE_Message_Block(packet.length()), -1);
	mb->copy(packet.c_str(),packet.length());
	
	if (packet.find("Event:")!=std::string::npos)
	  {
	    assert(event_queue_); 
	    event_queue_->enqueue_tail(mb);
	    return 0;
	  }
	
	if (packet.find("Response:")!=std::string::npos)
	  {
	    if ((packet.find("Pong")!=std::string::npos) //a pong response
		&& (packet.find("ActionID")==std::string::npos)) //not trigger by user
	      return 0; //drop the automatic pong response, but user pong is kept
	    assert(reply_queue_);
	    reply_queue_->enqueue_tail(mb);
	    return 0;
	  }
      }
    return 0;
  }
  

  int SvcHandler::handle_timeout(const ACE_Time_Value &, const void *)
  {
    //Should call the Manager API's ping method here, however, we here
    //just put the send ping message code
    
    cast_checkpoint();
    std::string action_msg="ACTION: Ping\r\n";
    action_msg+="\r\n"; //the endian CRLF;
    ACE_Message_Block *mb;
    ACE_NEW_RETURN(mb, ACE_Message_Block(action_msg.length()), -1);
    mb->copy(action_msg.c_str(), action_msg.length());
    this->putq(mb);
    
    return 0;
  }
  
  
  //flow controled send
  int SvcHandler::handle_output (ACE_HANDLE)
  {
    cast_checkpoint();
    ACE_Message_Block *mb;
    char send_buf[4096];
    ACE_Time_Value timeout_time(ACE_OS::gettimeofday()); //get current time
    ACE_Time_Value one_sec(1);
    
    timeout_time+=one_sec; //timeout_time is 10 seconds from now

    while (-1 != this->getq (mb, &timeout_time)) //block for at most 1 sec 
      {
	strncpy(send_buf, mb->rd_ptr (), mb->length ());
	//cast_checkpoint();
	send_buf[mb->length ()]='\0';

	cast_debug(0,"sending : ");
	cast_debug(0,send_buf);
	cast_debug(0,"-----------\n");
	fflush(NULL);
		
	ssize_t send_cnt =
	  this->peer ().send (mb->rd_ptr (), mb->length ());

	if ((send_cnt == -1)&& ACE_OS::last_error () != EWOULDBLOCK)
	{
	  ACE_ERROR ((LM_ERROR,
		      ACE_TEXT ("(%P|%t) %p\n"),
		      ACE_TEXT ("send")));
	  return -1;
	}
	else
	  mb->rd_ptr (ACE_static_cast (size_t, send_cnt));
	if (mb->length () > 0)
	  {
	    this->ungetq (mb);
	    break;
	  }
	mb->release ();
      }
 
    if (this->msg_queue ()->is_empty ())
      {
	this->reactor ()->cancel_wakeup
	  (this, ACE_Event_Handler::WRITE_MASK);
	cast_debug(0,"Empty sending queue\n");
      }
    else
      this->reactor ()->schedule_wakeup
	(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
  }

  int SvcHandler::handle_close (ACE_HANDLE handle,
      				ACE_Reactor_Mask close_mask)
  {
    cast_checkpoint();
    
    if (close_mask == ACE_Event_Handler::WRITE_MASK)
      return 0;
	this->reactor()->cancel_timer(this);
    close_mask = ACE_Event_Handler::ALL_EVENTS_MASK |
      ACE_Event_Handler::DONT_CALL;
    this->reactor()->remove_handler (this, close_mask);
    this->peer().close ();
    this->msg_queue()->flush ();
    //delete this;
     
    return 0;
  }

  int SockHandler::init(const char address[])
  { 
    cast_checkpoint();
    ast_addr_=ACE_INET_Addr(address); 
   
    SvcHandler *pc = &client_handler_;
   
    client_handler_.set_msg_queues(this->msg_queue (), &event_queue_);
   
    if (connector_.connect (pc, ast_addr_) == -1)
      ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("%p\n"),
			 ACE_TEXT ("connect")), -1);
    return 0;
  };
  
  int SockHandler::svc()
  {
    cast_checkpoint();

    //only the thread owns the reactor can run the reactor event loop
    //ACE_Reactor::instance ()->run_reactor_event_loop ();
    
    //transfer the owner to be this thread
    ACE_Reactor::instance ()->owner(ACE_OS::thr_self());
    
    ACE_Reactor::instance ()->run_reactor_event_loop ();
  
    return (0);
  }
  
  /* this function put msg into the sending queue */
  int SockHandler::send_msg(const char* msg)
  {
    cast_checkpoint();
    cast_debug(0,msg);
    ACE_Message_Block *mb;
    ACE_NEW_RETURN(mb, ACE_Message_Block(strlen(msg)), -1);
    mb->copy(msg, strlen(msg));
    client_handler_.putq(mb);
    return 0;
  }
  
  char* SockHandler::get_reply()
  {
    cast_checkpoint();
    ACE_Message_Block *mb;
    ACE_Time_Value timeout_time(ACE_OS::gettimeofday()); //get current time
    ACE_Time_Value ten_sec(10);
    
    timeout_time+=ten_sec; //timeout_time is 10 seconds from now
    
    if  (-1==this->getq(mb, &timeout_time)) //reply queue timeout
      {
	ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"),
		    ACE_TEXT ("get_reply")));
	return NULL;
      }
    
    //we got something here
    int msg_len = mb->length();
    char * reply= new char[msg_len+1];
    strncpy(reply, mb->rd_ptr(), msg_len);
    reply[msg_len]= '\0';
    
    mb->release();
    
    return reply;
  }
  
  char* SockHandler::get_event()
  {
    cast_checkpoint();
    ACE_Message_Block *mb;
    ACE_Time_Value timeout_time(ACE_OS::gettimeofday()); //get current time
    ACE_Time_Value ten_sec(10);
    
    timeout_time+=ten_sec; //timeout_time is 10 seconds from now
    
    if  (-1==event_queue_.dequeue_head (mb, &timeout_time)) //reply queue timeout
      {
	return NULL;
      }
    //we got something here
    int msg_len = mb->length();
    char * event= new char[msg_len+1];
    strncpy(event, mb->rd_ptr(), msg_len);
    event[msg_len]= '\0';
    
    mb->release();
    
    return event;
  }
  
  void SockHandler::get_reply(char *reply)
  {
    cast_checkpoint();
    ACE_Message_Block *mb;
    ACE_Time_Value timeout_time(ACE_OS::gettimeofday()); //get current time
    ACE_Time_Value ten_sec(10);
    
    timeout_time+=ten_sec; //timeout_time is 10 seconds from now
    
    if  (-1==this->getq(mb, &timeout_time)) //reply queue timeout
      {
	strcpy(reply,"TIMEOUT");
	ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"),
		    ACE_TEXT ("get_reply")));
	return;
      }
    
    //we got something here
    int msg_len = mb->length();
    
    strncpy(reply, mb->rd_ptr(), msg_len);
    reply[msg_len]= '\0';
    
     mb->release();
    
    return;
  }
  
  void SockHandler::get_event(char *event)
  {
    cast_checkpoint();
    ACE_Message_Block *mb;
    ACE_Time_Value timeout_time(ACE_OS::gettimeofday()); //get current time
    ACE_Time_Value ten_sec(10);
    
    timeout_time+=ten_sec; //timeout_time is 10 seconds from now
    
    if  (-1==event_queue_.dequeue_head (mb, &timeout_time)) //event  queue timeout
      return; //not error, simply no event happened
          
    //we got something here
    int msg_len = mb->length();
    
    strncpy(event, mb->rd_ptr(), msg_len);
    event[msg_len]= '\0';
    
    mb->release();
    
    return;
  }
  
#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
  template class ACE_Connector<SvcHandler, ACE_SOCK_CONNECTOR>;
  template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>;
  template class ACE_TASK<ACE_MT_SYNCH>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
# pragma instantiate ACE_Connector<SvcHandler, ACE_SOCK_CONNECTOR>
# pragma instantiate ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
# pragma instantiate ACE_TASK<ACE_MT_SYNCH>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
 
}
