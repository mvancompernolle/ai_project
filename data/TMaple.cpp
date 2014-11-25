/** TDSP *******************************************************************
                          TMaple.cpp  -  description
                             -------------------
    begin                : Mit Mär 5 2003
    copyright            : (C) 2003 by Peter Jung
    email                : jung@hhi.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TMaple.h"


#ifndef WIN32

# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
# include <wait.h>
# include <string.h>
#endif

#define BUFFER_SIZE 1000

ClassImpQ(TMaple)

TMaple::TMaple(){
  
#ifndef WIN32
  
  int read_pipe[2];    // for reading the output of maple
  int write_pipe[2];   // for writing to the input of maple
  int pid;

  /* Create the read-pipe - */
  if (pipe(read_pipe) < 0) {
    Error("TMaple","pipe() for read-pipe failed");
    return;
  }

  if (pipe(write_pipe) < 0) {
    Error("TMaple","pipe() for write-pipe failed");
    return;
  }

  /* Fork a child process */
  if ((pid = fork()) == (pid_t)-1) {
    Error("TMaple","fork(): failed");
  }
    
  /* Are we the child? */
  
  if (pid == 0) {
    
    /* Copy the output of the write-pipe to the stdin */
    if (dup2(write_pipe[0], STDIN_FILENO) < 0) {
      Error("TMaple","dup2() of the write_pipe[1] failed");
      exit(1);
    }
      
   //  /* Copy the input of the read-pipe to the stdout */
//     if (dup2(read_pipe[1], STDOUT_FILENO) < 0) {
//       Error("TMaple","dup2() of the read_pipe[0] failed");
//       exit(1);
//     }
      
//     /* Copy the input of the read-pipe to the stderr */
//     if (dup2(read_pipe[1], STDERR_FILENO) < 0) {
//       Error("TMaple","dup2() of the read_pipe[0] failed");
//       exit(1);
//     }
      
    /* Closing  */
    if (close(write_pipe[1]) < 0) {
      Error("TMaple","close(): failed");
      exit(1);
    }
    if (close(write_pipe[0]) < 0) {
      Error("TMaple","close(): failed");
      exit(1);
    }
    if (close(read_pipe[1]) < 0) {
      Error("TMaple","close(): failed");
      exit(1);
    }
    if (close(read_pipe[0]) < 0) {
      Error("TMaple","close(): failed");
      exit(1);
    }
    /* Execute the other process */
    //if (execvp(argv[1], argv + 1) < 0) {
    
    if (execvp("maple", NULL) < 0) {
      Error("TMaple","execl(): failed");
      exit(1);
    }  
    /* Should never get here */
    abort();
  } else {
      
    fd_in  = write_pipe[1];
    fd_out = read_pipe[0];
    fPid   = pid;

    //   int oldflags = fcntl (fd_out, F_GETFL, 0);
//       if (oldflags<0) {
// 	Error("TMaple","Error reading the flags - cant set nonblock !");
//       } else {
// 	fcntl(fd_out,F_SETFL,oldflags | O_NONBLOCK);
//       }

      // Initialisation

    // Put("interface( prettyprint=0 );\n"
	// 	  "interface( echo=0        );\n"
	// 	  "interface( quiet=true    );\n"
	// "1;\n");
      
     


    if (!fBuffer) fBuffer=(char*)malloc(sizeof(char)*BUFFER_SIZE);
  }
#endif
}
TMaple::~TMaple(){
#ifndef WIN32
	if (fBuffer) free(fBuffer);
#endif
}

char *TMaple::Get(char *out) {

#ifndef WIN32
  
  Int_t length = read(fd_out, fBuffer, BUFFER_SIZE-10);
  if (length < 0) {
    Error("Get","read(): failed");
    return NULL;
  }

 //  if (length == 0) {
//     int status;

//     if (close(fd_in) < 0) {
//       Error("Get","close(): failed");
//       return NULL;
//     }

//     if (waitpid(fPid, &status, 0) < 0) {
//       Error("Get","waitpid(): failed");
//       return NULL;
//     }

//     fprintf(stderr, "child exited: %d\n", WEXITSTATUS(status));
//     return NULL;
//   }

//  fwrite(fBuffer, length, 1, stdout);

#else 
  return NULL;
#endif

}

char *TMaple::Put(char *msg) {
#ifndef WIN32
	write(fd_in,msg,strlen(msg)+1);
#endif
  return NULL;
}
