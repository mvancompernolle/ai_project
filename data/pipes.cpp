/* Copyright (C) 1999 to 2004 and 2009 to 2013 Chris Vine

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

#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

#include <c++-gtk-utils/pipes.h>

namespace Cgu {

PipeFifo::PipeFifo(Fifo_mode mode): write_blocking_mode(block) {
  int file_pipes[2];
  if (pipe(file_pipes) == -1) throw PipeError();

  read_fd = file_pipes[0];
  write_fd = file_pipes[1];
  if (mode == non_block) {
    int fdflags = fcntl(read_fd, F_GETFL);
    fdflags |= O_NONBLOCK;
    fcntl(read_fd, F_SETFL, fdflags);
    read_blocking_mode = non_block;
  }
  else read_blocking_mode = block;
}

PipeFifo::PipeFifo(): read_fd(-1), write_fd(-1),
		      read_blocking_mode(block), write_blocking_mode(block) {}

PipeFifo::PipeFifo(PipeFifo&& pf): read_fd(pf.read_fd),
				   write_fd(pf.write_fd),
				   read_blocking_mode(pf.read_blocking_mode),
				   write_blocking_mode(pf.write_blocking_mode) {
  // neutralize the PipeFifo object moved from
  pf.read_fd = -1;
  pf.write_fd = -1;
}

PipeFifo& PipeFifo::operator=(PipeFifo&& pf) {
  if (&pf != this) {
    // close any existing open file descriptors
    close();

    read_fd = pf.read_fd;
    write_fd = pf.write_fd;
    read_blocking_mode = pf.read_blocking_mode;
    write_blocking_mode = pf.write_blocking_mode;

    // neutralize the PipeFifo object moved from
    pf.read_fd = -1;
    pf.write_fd = -1;
  }
  return *this;
}

void PipeFifo::open(Fifo_mode mode) {

  // close any existing open file descriptors
  close();
  // now open new descriptors
  int file_pipes[2];
  if (pipe(file_pipes) == -1) throw PipeError();

  read_fd = file_pipes[0];
  write_fd = file_pipes[1];
  if (mode == non_block) {
    int fdflags = fcntl(read_fd, F_GETFL);
    fdflags |= O_NONBLOCK;
    fcntl(read_fd, F_SETFL, fdflags);
    read_blocking_mode = non_block;
  }
}

void PipeFifo::close() {
  make_writeonly();
  make_readonly();
  read_blocking_mode = block;
  write_blocking_mode = block;
}

ssize_t PipeFifo::read(char* buffer, size_t max_num) {
  if (read_fd == -1) return -2;
  ssize_t result;
  do {
    result = ::read(read_fd, buffer, max_num);     // call unix read() in file scope
  } while (result == -1  && errno == EINTR);
  return result;
}      

int PipeFifo::read() {
  if (read_fd == -1) return -2;
  unsigned char item;
  ssize_t result;
  do {
    result = ::read(read_fd, &item, 1);            // call unix read() in file scope
  } while (result == -1  && errno == EINTR);
  if (result > 0) return (int)item;
  return result;
}

ssize_t PipeFifo::write(const char* buffer) {
  if (write_fd == -1) return -2;
  return write(buffer, std::strlen(buffer));
}

ssize_t PipeFifo::write(const char* buffer, size_t length) {
  if (write_fd == -1) return -2;
  ssize_t written = 0;
  if (write_blocking_mode == block) {
    // if in blocking mode, make sure everything to be written is written
    ssize_t result;
    do {
      result = ::write(write_fd, buffer + written, length);// call unix write() in file scope
      if (result > 0) {
	written += result;
	length -= result;
      }
    } while (length && (result != -1 || errno == EINTR));

    if (result == -1) written = -1;
  }
  else {
    // if in non-blocking mode just return the value of unix write()
    do {
      written = ::write(write_fd, buffer, length);             // call unix write() in file scope
    } while (written == -1  && errno == EINTR);
  }
  return written;
}

void PipeFifo::make_writeonly() {
  if (read_fd != -1) {
    while (::close(read_fd) == -1 && errno == EINTR); // call unix close() in file scope
    read_fd = -1;
  }
}

void PipeFifo::make_readonly() {
  if (write_fd != -1) {
    while (::close(write_fd) == -1 && errno == EINTR);// call unix close() in file scope
    write_fd = -1;
  }
}

int PipeFifo::make_write_non_block() {
  if (write_fd >= 0) {
    int fdflags = fcntl(write_fd, F_GETFL);
    fdflags |= O_NONBLOCK;
    fcntl(write_fd, F_SETFL, fdflags);
    write_blocking_mode = non_block;
    return 0;
  }
  return -1;
}

int PipeFifo::connect_to_stdin() {

  // check preconditions
  if (read_fd == -1) return -2;

  int result;
  // first we need to duplicate the read file descriptor onto stdin
  while ((result = dup2(read_fd, 0)) == -1 && errno == EINTR);
  if (!result) {
    make_writeonly();    // this will close the original read file descriptor
                         // but leave stdin unaffected
    make_readonly();     // since the pipe is unidirectional, we can also close the write fd
  }
  return result;
}

int PipeFifo::connect_to_stdout() {

  // check preconditions
  if (write_fd == -1) return -2;

  int result;
  // first we need to duplicate the write file descriptor onto stdout
  while ((result = dup2(write_fd, 1)) == -1 && errno == EINTR); // stdout will now write to the pipe
  if (!result) {
    make_readonly();     // this will close the original write file descriptor
                         // but leave stdout unaffected
    make_writeonly();    // since the pipe is unidirectional, we can also close the read fd
  }
  return result;
}

int PipeFifo::connect_to_stderr() {

  // check preconditions
  if (write_fd == -1) return -2;

  int result;
  // first we need to duplicate the write file descriptor onto stderr
  while ((result = dup2(write_fd, 2)) == -1 && errno == EINTR); // stderr will now write to the pipe
  if (!result) {
    make_readonly();     // this will close the original write file descriptor
                         // but leave stderr unaffected
    make_writeonly();    // since the pipe is unidirectional, we can also close the read fd
  }
  return result;
}

void SyncPipe::wait() {
  pipe_fifo.make_readonly();
  char letter;
  while (::read(pipe_fifo.get_read_fd(), &letter, 1) == -1
	 && errno == EINTR);
  pipe_fifo.make_writeonly();
}

} // namespace Cgu
