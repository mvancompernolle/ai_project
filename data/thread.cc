//  libmore.a(thread.o) -- a thread class: POSIX implementation
//  Copyright (C) 1998--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <map>
#include <more/gen/lambda.h>
#include <more/cf/thread.h>
#include <more/cf/signal.h>
#include <cerrno>
#include <cassert>
#include <pthread.h>


namespace more {
namespace cf {

  namespace {
    // A mutex for the file-scope static variables here.
    mutex static_mutex;
  }

  namespace bits {
    int thread_count = 1;
    bool other_threads = false;
  }

  void notify_foreign_thread()
  {
      static_mutex.lock();
      ++bits::thread_count;
      bits::other_threads = true;
      static_mutex.unlock();
  }

  void unnotyfy_foreign_thread()
  {
      static_mutex.lock();
      if (--bits::thread_count == 1)
	  bits::other_threads = false;
      static_mutex.unlock();
  }



  //--------------------------------------------------------------------
  //				mutex
  //--------------------------------------------------------------------

  struct mutex_data : gen::handle_data
  {
      friend class mutex;
      mutex_data() {
	  pthread_mutex_init(&var, NULL);
      }
      ~mutex_data() {
	  switch (pthread_mutex_destroy(&var)) {
	  case 0: return;
	  case EBUSY:
	      throw std::logic_error("more::cf::mutex::~mutex(): "
				     "A locked mutex is destructed.");
	  default:
	      throw std::logic_error("more::cf::mutex_data::~mutex_data(): "
				     "Internal error.");
	  }
      }
    private:
      pthread_mutex_t var;
  };


  mutex::mutex()
      : gen::handle<mutex_data>(new mutex_data) {}

  mutex::mutex(mutex const& x)
      : gen::handle<mutex_data>(x) {}

  mutex::~mutex() {}

  mutex&
  mutex::operator=(mutex const& x)
  {
      (gen::handle<mutex_data>&)(*this) = x; return *this;
  }

  void mutex::lock()
  {
      switch(pthread_mutex_lock(&data()->var)) {
      case 0: return;
      case EDEADLK:
	  throw std::logic_error(
	      "mutex::lock(): The mutex is already locked in this thread.");
      default:
	  throw std::logic_error("mutex::lock(): Internal error.");
      }
  }

  bool mutex::trylock()
  {
      switch(pthread_mutex_trylock(&data()->var)) {
      case 0: return true;
      case EBUSY: return false;
      default:
	  throw std::logic_error("mutex::trylock(): Internal error.");
      }
  }

  void mutex::unlock()
  {
      switch(pthread_mutex_unlock(&data()->var)) {
      case 0: return;
      case EPERM:
	  throw std::logic_error(
	      "mutex::unlock(): The mutex is locked by another thread.");
      default:
	  throw std::logic_error("mutex::unlock(): Internal error.");
      }
  }



  //--------------------------------------------------------------------
  //				thread
  //--------------------------------------------------------------------

  void _thread_checkpoint()
  {
      if (caller_is_canceled())
	  throw thread_canceled();
  }


  struct thread_data
      : gen::handle_data
  {
      typedef unsigned char state_type;  // must be atomic
      // Set by from the creators process:
      static const state_type undefined_state = 0;
      static const state_type defined_state = 1;
      static const state_type pending_state = 2;
      // Set by from the thread process:
      static const state_type running_state = 3;
      static const state_type finished_state = 4;

      typedef thread::closure_type closure_type;

      //  --- constructors ---

    public:
      thread_data(closure_type const& proc)
	  : m_proc(proc),
	    m_st(defined_state),
	    m_est(0),
	    m_attached(0)
      {
	  static_init();
      }

      ~thread_data()
      {
	  //assert(!(m_st & active_state));
	  if (!is_attached())
	      return;
	  switch (pthread_detach(m_pth)) {
	    case 0:
	      break;
	    case ESRCH:
	      throw std::logic_error("more::cf::thread_data::~thread_data(): "
				     "Internal error.  "
				     "pthread_detach retruned ESRCH.");
	    case EINVAL:
	      throw std::logic_error("more::cf::thread_data::~thread_data(): "
				     "Internal error.  "
				     "pthread_detach retruned EINVAL.");
	    default:
	      throw std::logic_error("more::cf::thread_data::~thread_data(): "
				     "Internal error. Unknown "
				     "exit code from pthread_detach.");
	  }
      }

      state_type	state(state_type st) const { return m_st; }
      void set_state(state_type st) { m_st = st; }
      bool is_finished() const { return m_st >= finished_state; }
      bool is_active() const
      {
	  return m_st == pending_state || m_st == running_state;
      }
      bool is_running() const { return m_st == running_state; }
      bool is_started() const { return m_st >= running_state; }
      bool is_attached() const { return m_attached; }
      void set_attached(bool st) { m_attached = st; }
      void set_canceled() { m_est |= 1; }
      void set_excepted() { m_est |= 2; }
      bool is_canceled() const { return m_est & 1; }
      bool is_excepted() const { return m_est & 2; }
      bool is_good() const { return m_est == 0; }

      void ref()
      {
	  ((handle_data*)this)->ref();
      }

      void set_call(closure_type const& proc)
      {
	  if (is_started())
	      throw std::logic_error("more::cf::thread::set_call: "
				     "The thread is already started.");
	  m_proc = proc;
      }

    private:
      void register_running()
      {
	  static_mutex.lock();
	  ++bits::thread_count;
	  bits::other_threads = true;
	  static_mutex.unlock();
      }

      void unregister_running()
      {
	  static_mutex.lock();
	  if (--bits::thread_count == 1)
	      bits::other_threads = false;
	  static_mutex.unlock();
      }

      pthread_t     m_pth;
      closure_type  m_thrower;	// Only set within thread and only
				// accessed after thread is finished.
      closure_type  m_proc;	// Set before thread starts and then
				// accessed only within thread.
      state_type    m_st;	// Assumed to be atomic.
      state_type    m_est;
      state_type    m_attached;

    public:
      static thread_data*
      current()
      {
	  return static_cast<thread_data*>
	      (pthread_getspecific(s_current_thread_key));
      }

    private:
      static void* sub_main(void*);

      static void static_init()
      {
	  if (!s_done_init) {
	      pthread_key_create(&s_current_thread_key, 0);
	      register_pointchecker(_thread_checkpoint);
	      s_done_init = true;
	  }
      }

      static bool s_done_init;
      static pthread_key_t s_current_thread_key;

      friend class thread;
  };

  bool		thread_data::s_done_init = false;
  pthread_key_t	thread_data::s_current_thread_key;

  bool caller_is_canceled()
  {
      thread_data* thd = thread_data::current();
      return thd && thd->is_canceled();
  }

  thread thread_of_caller()
  {
      if (thread_data* thd = thread_data::current())
	  return thread(thd);
      else
	  return thread();
  }

  void* thread_data::sub_main(void* data)
  {
      thread_data* thd = static_cast<thread_data*>(data);

      // Prevent thread_data from being destroyed while the thread is
      // running, as it is accessible to the thread.
      thread th(thd);
      pthread_setspecific(s_current_thread_key, data);

      thd->set_state(thread_data::running_state);
      try {
	  thd->m_proc();
      }
      catch (const thread_canceled) {
	  thd->set_canceled();
      }
      catch (const std::logic_error& e) {
	  thd->m_thrower = apply(gen::thrower<std::logic_error>(), e);
	  thd->set_excepted();
      }
      catch (const std::runtime_error& e) {
	  thd->m_thrower = apply(gen::thrower<std::runtime_error>(), e);
	  thd->set_excepted();
      }
      catch (const std::exception& e) {
	  thd->m_thrower = apply(gen::thrower<std::exception>(), e);
	  thd->set_excepted();
      }
      catch (...) {
	  thd->m_thrower =
	      apply(gen::thrower<std::runtime_error>(),
		    std::runtime_error(
			"An unknown exception occured in a thread\n."));
	  thd->set_excepted();
      }
      thd->unregister_running();
      thd->set_state(thread_data::finished_state);
      return data;
  }


  //  --- thread ---


  thread::thread() {}

  // fixme: need mutex in the following methods
  thread::thread(thread const& x)
      : gen::handle<thread_data>(x) {}

  thread::thread(thread_data *p)
      : gen::handle<thread_data>(p) {}

  thread::~thread() {}

  thread::thread(closure_type const& f)
      : gen::handle<thread_data>(new thread_data(f)) {}

  thread&
  thread::operator=(thread const& x)
  {
      (gen::handle<thread_data>&)(*this) = x;
      return *this;
  }

  void thread::set_call(closure_type const& f)
  {
      if (!data())
	  set_data(new thread_data(f));
      data()->set_call(f);
  }

  bool thread::is_defined() const
  {
      return data() != 0;
  }
  bool thread::is_active() const
  {
      return data() != 0 && data()->is_active();
  }
  bool thread::is_running() const
  {
      return data() != 0 && data()->is_running();
  }
  bool thread::is_started() const
  {
      return data() != 0 && data()->is_started();
  }
  bool thread::is_finished() const
  {
      return data() != 0 && data()->is_finished();
  }
  bool thread::is_canceled() const { return datachk()->is_canceled(); }
  bool thread::is_excepted() const { return datachk()->is_excepted(); }

  void thread::start()
  {
      if (!is_defined())
	  throw std::logic_error("more::cf::thread::start: "
				 "Missing closure to execute.");
      if (data()->is_attached())
	  throw std::logic_error("more::cf::thread::start: "
				 "Thread is already started.");

      // Set the state variables _before_ starting the thread.  When
      // the thread is running only set_canceled() is allowed.
      data()->set_state(thread_data::pending_state);
      data()->set_attached(true);
      data()->register_running();

      switch (pthread_create(&data()->m_pth, NULL,
			     thread_data::sub_main, data())) {
	case 0:
	  break;
	case EAGAIN:
	  data()->set_attached(false);
	  data()->unregister_running();
	  throw std::runtime_error("more::cf::thread::start(): Out of "
				   "system resources in pthread_create.");
	default:
	  data()->set_attached(false);
	  data()->unregister_running();
	  throw std::logic_error("more::cf::thread::start(): "
				 " Unknown exit code from pthread_create.");
      }
  }

  void thread::cancel()
  {
      datachk()->set_canceled();
  }

  bool thread::join()
  {
      if (is_active()) {
	  switch (pthread_join(data()->m_pth, NULL)) {
	    case 0:
	      data()->set_attached(false);
	      break;
	    case EINVAL:
	      throw std::logic_error("more::cf::thread::join(): "
				     "Another thread has already joined.");
	    case EDEADLK:
	      throw std::logic_error("more::cf::thread::join(): "
				     "This thread is trying to join itself.");
	    default:
	    case ESRCH:
	      throw std::logic_error("more::cf::thread::join(): "
				     "Internal error.");
	  }
      }
      if (is_excepted())
	  data()->m_thrower();
      return is_finished();
  }

}} // more::cf
