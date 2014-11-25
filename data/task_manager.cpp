/* Copyright (C) 2012 and 2013 Chris Vine

The library comprised in this file or of which this file is part is
distributed by Chris Vine under the GNU Lesser General Public
License as follows:

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License, version 2.1, for more details.

   You should have received a copy of the GNU Lesser General Public
   License, version 2.1, along with this library (see the file LGPL.TXT
   which came with this source code package in the c++-gtk-utils
   sub-directory); if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <c++-gtk-utils/lib_defs.h>

#include <algorithm>  // for std::min()
#include <cstdlib>    // for std::abs()

#include <glib.h>     // for g_critical()

#include <c++-gtk-utils/intrusive_ptr.h>
#include <c++-gtk-utils/task_manager.h>

namespace {

// this is an internal struct thrown by the TaskManager implemention
// in order to finish threads when stop_all() is called.  It is caught
// by the do_tasks() method (which is then recaught and consumed by
// the Thread::Thread implementation) - the purpose of this struct is
// to distinguish between a case where the task's user code throws
// Thread::Exit to end a task (which should not end the thread on
// which it is running) and a case where the internal TaskManager
// implemention needs to kill off threads when stop_all() is called.
struct KillThread {};
void throw_kill_thread() {throw KillThread();}

enum StopStatus {running, stop_requested, stopped};

}

namespace Cgu {

namespace Thread {

struct TaskManager::RefImpl: public IntrusiveLockCounter {

  mutable Mutex mutex;
  // stop_guard is provided so we don't have to hold the TaskManager
  // mutex when pushing to the task queue.  It is used when access is
  // made to RefImpl::stop_status (see the TaskManager::stop_all() and
  // TaskManager::add_task()).  If both have to be locked, 'mutex' is
  // locked before 'stop_guard'
  mutable Mutex stop_guard;
  Cond cond;

  AsyncQueueDispatch<QueueItemType, std::deque<QueueItemType>> task_queue;

  unsigned int max_threads;
  const unsigned int min_threads;
  unsigned int used_threads;
  unsigned int idle_time;
  unsigned int tasks;
  bool blocking;
  StopStatus stop_status;
  bool error;

  StopMode stop_mode;

  void do_tasks(bool);
  RefImpl(unsigned int max, unsigned int min,
	  unsigned int idle, bool block,
	  StopMode mode): max_threads(max), min_threads(min),
			  used_threads(0), idle_time(idle),
			  tasks(0), blocking(block),
			  stop_status(running), error(false),
			  stop_mode(mode) {}
/* Only has effect if --with-glib-memory-slices-compat or
 * --with-glib-memory-slices-no-compat option picked */
  CGU_GLIB_MEMORY_SLICES_FUNCS
};

void TaskManager::RefImpl::do_tasks(bool persistent) {

  // every worker thread in the pool must be uncancellable
  CancelBlock::block();

  for (;;) {

    QueueItemType task;                                             // won't throw
    if (persistent)
      task_queue.move_pop_dispatch(task);                           // won't throw
    else if (task_queue.move_pop_timed_dispatch(task, idle_time)) { // won't throw
      mutex.lock();
      // test in case between task_queue unblocking and the mutex
      // being acquired, add_task() has added a task expecting to find
      // this waiting thread
      if (tasks < used_threads) {
	--used_threads;
	if (stop_status == StopStatus::stopped && blocking) cond.broadcast();
	mutex.unlock();
	unref();
	return; // end thread if idle time elapsed without a task
      }
      else {
	mutex.unlock();
	continue;
      }
    }
    try {
      task.first->dispatch();
    }
    catch (KillThread&) {
      // we don't decrement 'tasks' here, as adding a KillThread
      // callback does not increment the number of tasks
      mutex.lock();
      --used_threads;
      if (stop_status == StopStatus::stopped && blocking) cond.broadcast();
      mutex.unlock();
      unref();
      return;
    }
    catch (Exit&) {
      try {
	if (task.second.get()) task.second->dispatch();
      }
      catch (...) {
	g_critical("fail callback has thrown in Cgu::Thread::TaskManager::do_tasks()");
      }
    }
    catch (...) {
      try {
	if (task.second.get()) task.second->dispatch();
	else g_critical("task has thrown in Cgu::Thread::TaskManager::do_tasks()");
      }
      catch (...) {
	g_critical("fail callback has thrown in Cgu::Thread::TaskManager::do_tasks()");
      }
    }

    mutex.lock();
    // 'used_threads' must be decremented atomically with 'tasks'
    --tasks;
    if (!persistent && used_threads > max_threads) {
      --used_threads;
      if (stop_status == StopStatus::stopped && blocking) cond.broadcast();
      mutex.unlock();
      unref();
      return;
    }
    else mutex.unlock();
  }
}

TaskManager::TaskManager(unsigned int max, unsigned int min,
			 unsigned int idle, bool blocking,
			 StopMode mode) {
  if (!max) max = 1;
  if (max < min) max = min;

  ref_impl = new RefImpl{max, min, idle, blocking, mode};

  // take ownership in this constructor in case it throws - we add
  // another reference at the end of this method so that the fully
  // constructed TaskManager object reclaims ownership
  IntrusivePtr<RefImpl> temp{ref_impl}; // ref count is 1

  // 'kill_queue' is a clean-up queue which is pushed to at the
  // beginning so we can back out and release any started threads if
  // later threads fail to start correctly.  It doesn't matter if the
  // number of callbacks emplaced here is greater than the number of
  // threads actually started, as any unused callbacks will be
  // destroyed if all threads start correctly, or otherwise remain
  // unused until the TaskManager object is destroyed.
  AsyncQueueDispatch<QueueItemType, std::deque<QueueItemType>> kill_queue;
  for (unsigned int count = min; count; --count) {
    kill_queue.emplace(std::unique_ptr<const Callback::Callback>(Callback::make(&throw_kill_thread)),
                       std::unique_ptr<const Callback::Callback>());
  }

  Mutex::Lock lock{ref_impl->mutex};
  for (unsigned int count = 0; count < min; ++count) {
    std::unique_ptr<Thread> t;
    try {
      t = Thread::start(Callback::make(*ref_impl,
				       &TaskManager::RefImpl::do_tasks,
				       true),
			false);
    }
    catch (...) {
      ref_impl->stop_status = StopStatus::stopped;
      swap(ref_impl->task_queue, kill_queue);
      throw;
    }
    if (!t.get()) {
      ref_impl->stop_status = StopStatus::stopped;
      swap(ref_impl->task_queue, kill_queue);
      throw TaskError();
    }
    // to avoid data races, 'used_threads' and 'ref_impl' have to be
    // incremented by the thread starting a new thread, not in the new
    // thread itself
    ++ref_impl->used_threads;
    ref_impl->ref(); // give each thread a reference
  }
  ref_impl->ref();   // give the TaskManager object a reference: when
		     // this constructor finishes, the ref count is 1
		     // plus the number of min threads successfully
		     // started
}

TaskManager::~TaskManager() {
  ref_impl->mutex.lock();
  if (ref_impl->stop_status != StopStatus::stopped) {
    ref_impl->mutex.unlock();
    try {
      stop_all();
    }
    catch (std::exception&) {} // for std::bad_alloc or Cgu::Thread::TaskError
  }
  else if (ref_impl->blocking) {  // && StopStatus::stopped
    while (ref_impl->used_threads) ref_impl->cond.wait(ref_impl->mutex);
    ref_impl->mutex.unlock();
  }
  else ref_impl->mutex.unlock();

  ref_impl->unref(); // 'ref_impl' will remain in existence until all
		     // threads (if any) have also released their
		     // references
}

unsigned int TaskManager::get_max_threads() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->max_threads;
}

unsigned int TaskManager::get_min_threads() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->min_threads;
}

unsigned int TaskManager::get_used_threads() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->used_threads;
}

unsigned int TaskManager::get_tasks() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->tasks;
}

// this method should only be entered when holding ref_impl's mutex
void TaskManager::set_max_threads_impl(unsigned int max, Mutex::TrackLock& lock) {

  if (ref_impl->error || ref_impl->stop_status != StopStatus::running) return;

  if (!max) max = 1;
  if (max < ref_impl->min_threads) max = ref_impl->min_threads;

  if (max > ref_impl->max_threads) {
    
    ref_impl->max_threads = max;
    const unsigned int new_used_threads = std::min(ref_impl->tasks, max);
    unsigned int new_thread_count = 0;
    if (new_used_threads > ref_impl->used_threads) {
      new_thread_count = new_used_threads - ref_impl->used_threads;
      ref_impl->used_threads = new_used_threads;
    }
    // we need to give the threads a reference before we release the
    // mutex so we don't race with thread killing in stop_all()
    for (unsigned int refs = 0; refs < new_thread_count; ++refs) ref_impl->ref();
    lock.unlock();

    for (; new_thread_count; --new_thread_count) {
      std::unique_ptr<Thread> t;
      try {
	t = Thread::start(Callback::make(*ref_impl,
					 &TaskManager::RefImpl::do_tasks,
					 false),
			  false);
      }
      catch (...) { // try block might throw std::bad_alloc
	// roll back for any unstarted threads
	lock.lock();
	ref_impl->error = true;
	ref_impl->used_threads -= new_thread_count;
	if (ref_impl->stop_status == StopStatus::stopped && ref_impl->blocking)
	  ref_impl->cond.broadcast();
	lock.unlock();
	for (unsigned int unrefs = 0; unrefs < new_thread_count; ++unrefs) ref_impl->unref();
	throw;
      }
      if (!t.get()) {
	// roll back for any unstarted threads
	lock.lock();
	ref_impl->error = true;
	ref_impl->used_threads -= new_thread_count;
	if (ref_impl->stop_status == StopStatus::stopped && ref_impl->blocking)
	  ref_impl->cond.broadcast();
	lock.unlock();
	for (unsigned int unrefs = 0; unrefs < new_thread_count; ++unrefs) ref_impl->unref();
	throw TaskError();
      }
    }
  }
  else ref_impl->max_threads = max;
}

void TaskManager::set_max_threads(unsigned int max) {
  Mutex::TrackLock l{ref_impl->mutex};
  set_max_threads_impl(max, l);
}

void TaskManager::change_max_threads(int delta) {
  Mutex::TrackLock l{ref_impl->mutex};
  unsigned int new_max = (delta < 0
			  && std::abs(delta) >= ref_impl->max_threads) ? 1 :
                                                                         ref_impl->max_threads + delta;
  set_max_threads_impl(new_max, l);
}

unsigned int TaskManager::get_idle_time() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->idle_time;
}

void TaskManager::set_idle_time(unsigned int idle) {
  Mutex::Lock l{ref_impl->mutex};
  ref_impl->idle_time = idle;
}

bool TaskManager::get_blocking() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->blocking;
}

void TaskManager::set_blocking(bool block) {
  Mutex::Lock l{ref_impl->mutex};
  if (ref_impl->stop_status == StopStatus::stopped) throw TaskError();
  ref_impl->blocking = block;
}

TaskManager::StopMode TaskManager::get_stop_mode() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->stop_mode;
}

void TaskManager::set_stop_mode(StopMode mode) {
  Mutex::Lock l{ref_impl->mutex};
  ref_impl->stop_mode = mode;
}

bool TaskManager::is_error() const {
  Mutex::Lock l{ref_impl->mutex};
  return ref_impl->error;
} 

void TaskManager::stop_all() {

  Mutex::TrackLock lock{ref_impl->mutex};
  if (ref_impl->stop_status == StopStatus::stopped) throw TaskError();

  { // stop_guard scope block.  stop_guard is only referenced in
    // add_task() , and is included to enable
    // StopMode::wait_for_running to be implemented scaleably
    Mutex::Lock stop_lock{ref_impl->stop_guard};
    ref_impl->stop_status = StopStatus::stop_requested;
    // there is no race here once 'stop_status' is set to
    // stop_requested
    if (ref_impl->stop_mode == StopMode::wait_for_running)
      while (!ref_impl->task_queue.empty()) ref_impl->task_queue.pop();

    // we could be adding more KillThread callbacks than necessary
    // here, because as we are doing this a timeout on a thread might
    // expire leading to its demise in that way.  However, that
    // doesn't matter - we just get left with a redundant callback in
    // 'task_queue' which never gets used
    for (unsigned int count = ref_impl->used_threads; count; --count) {
      try {
	ref_impl->task_queue.emplace(
	  std::unique_ptr<const Callback::Callback>(Callback::make(&throw_kill_thread)),
	  std::unique_ptr<const Callback::Callback>()
	);
      }
      catch (...) { // if the push throws, it is guaranteed that the
                    // item concerned is not inserted in the queue,
                    // because std::unique_ptr's move constructor and
                    // move assignment operator do not throw, so we
                    // can rethrow here without issues
	ref_impl->error = true;
	throw;
      }
    }
    ref_impl->stop_status = StopStatus::stopped;
  }

  if (ref_impl->blocking) {
    // in case the destructor is also blocking and unwaits first, we
    // need to take a reference to the RefImpl object so the last test
    // of the while variable is valid, and also take a pointer to its
    // address so we can still address it
    RefImpl* tmp = ref_impl;
    tmp->ref();
    while (tmp->used_threads) tmp->cond.wait(tmp->mutex);
    lock.unlock(); // we cannot do a final unreference of RefImpl
		   // still holding its mutex
    tmp->unref();
  }
}

void TaskManager::add_task(std::unique_ptr<const Callback::Callback> task,
			   std::unique_ptr<const Callback::Callback> fail) {


  { // scope block for mutex lock
    Mutex::TrackLock lock{ref_impl->mutex};

    if (ref_impl->error || ref_impl->stop_status != StopStatus::running)
      throw TaskError();

    // check if we need to start a new thread
    if (ref_impl->tasks >= ref_impl->used_threads
	&& ref_impl->used_threads < ref_impl->max_threads) {

      // by incrementing 'tasks' and 'used_threads' here (and backing
      // out later below if something goes wrong), there is no race
      // from starting the thread before adding the task, even if the
      // thread has a very short minimum idle time, and we can also
      // release the mutex before calling Thread::start() or emplacing
      // in the queue to reduce contention on the mutex: this is
      // because, after coming out of a timeout, RefImpl::do_tasks()
      // will test again whether tasks >= used_threads and if so the
      // new thread will not exit.  Doing it this way also means that
      // we are guaranteed that if an exception is thrown no task has
      // been added.
      ++ref_impl->tasks;
      ++ref_impl->used_threads;
      ref_impl->ref(); // give each thread a reference
      lock.unlock();

      std::unique_ptr<Thread> t;
      try {
	t = Thread::start(Callback::make(*ref_impl,
					 &TaskManager::RefImpl::do_tasks,
					 false),
			  false);
      }
      catch (...) { // try block might throw std::bad_alloc
	// roll back for the unstarted thread
	lock.lock();
	ref_impl->error = true;
	--ref_impl->tasks;
	--ref_impl->used_threads;
	if (ref_impl->stop_status == StopStatus::stopped && ref_impl->blocking)
	  ref_impl->cond.broadcast();
	lock.unlock();
	ref_impl->unref();
	throw;
      }
      if (!t.get()) {
	// roll back for the unstarted thread
	lock.lock();
	ref_impl->error = true;
	--ref_impl->tasks;
	--ref_impl->used_threads;
	if (ref_impl->stop_status == StopStatus::stopped && ref_impl->blocking)
	  ref_impl->cond.broadcast();
	lock.unlock();
	ref_impl->unref();
	throw TaskError();
      }
    }
    else {
      // if we are in this block the mutex must still be locked
      ++ref_impl->tasks;
    }
  }

  // we use stop_guard and stop_status to detect whether stop_all()
  // has been called between us releasing the TaskManager mutex above
  // and reaching here - stop_guard is included so that we can push
  // onto the queue below without holding the TaskManager mutex.  The
  // only other place where stop_guard is locked is in stop_all():
  // stop_all() is normally only called once.  stop_guard will
  // therefore not give rise to any significant additional contention,
  // because AsyncQueueDispatch::emplace() is itself ordered (and only
  // moves two std::unique_ptr objects onto the queue).
  // ref_impl->stop_status is set in stop_all() holding both the
  // TaskManager mutex and stop_guard, so we can lock either in order
  // to make a safe read.  stop_guard would be a candidate for being a
  // read-write lock, except this is pointless because as mentioned
  // AsyncQueueDispatch::emplace() is ordered.
  Mutex::TrackLock stop_lock{ref_impl->stop_guard};
  if (ref_impl->stop_status == StopStatus::running) {
    try {
      ref_impl->task_queue.emplace(std::move(task), std::move(fail));
    }
    catch (...) {
      // roll back for the unadded task
      // release 'stop_guard' - 'mutex' cannot be locked after
      // 'stop_guard' is locked
      stop_lock.unlock();
      Mutex::Lock lock{ref_impl->mutex};
      ref_impl->error = true;
      --ref_impl->tasks;
      throw;
    }
  }
  else {
    // roll back for the unadded task
    // release 'stop_guard' - 'mutex' cannot be locked after
    // 'stop_guard' is locked
    stop_lock.unlock();
    Mutex::Lock lock{ref_impl->mutex};
    --ref_impl->tasks;
    throw TaskError();
  }
}

} // namespace Thread

} // namespace Cgu
