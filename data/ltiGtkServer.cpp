/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiGtkServer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.11.2001
 * revisions ..: $Id: ltiGtkServer.cpp,v 1.5 2006/02/08 12:56:26 ltilib Exp $
 */

// debug levels: 1. everything
//               2. no protocoll
//               3. a few messages
//               4. nothing at all

#include "ltiGtkServer.h"

#ifdef HAVE_GTK

#include <cstdlib>

#ifdef _DEBUG
//#define _DEBUG_GUI_LEVEL 5
#endif

#include "ltiSemaphore.h"
#include "ltiMutex.h"

#include <iostream>
#include <cstring>

using std::cerr;
using std::endl;

namespace lti {

  // static member server instantiation:
  gtkServer::guiThread gtkServer::theThread;
  bool gtkServer::alreadyStarted = false;
  mutex gtkServer::lock;

  gtkServer::gtkServer() {
  }

  gtkServer::~gtkServer() {
  }

  bool gtkServer::start() {
    lock.lock();
    if (!alreadyStarted) {
      alreadyStarted = true;
      atexit(toolkitAbort);
      theThread.start();
    }
    lock.unlock();
    return true;
  }

  bool gtkServer::shutdown() {
    if (isAlive()) {
      lock.lock();
      alreadyStarted = false;
      theThread.stop();
      lock.unlock();
    }
    return true;
  }

  bool gtkServer::isAlive() {
    bool alive=false;
    lock.lock();
    alive=alreadyStarted && theThread.isAlive();
    lock.unlock();
    return alive;
  }

  void gtkServer::toolkitAbort() {

    if (theThread.isAlive()) {

#     if defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
      cerr << "toolkitAbort called!" << endl;
#     endif

      //theThread.stop();
      gtk_exit(0);

#     if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
      cerr << "toolkitAbort ready!" << endl;
#     endif

    }
  }

  // -------------------------------------------------------------------
  // The GUI-Server Thread
  // -------------------------------------------------------------------

  semaphore gtkServer::guiThread::initReady(0);

  //
  gtkServer::guiThread::guiThread()
    : thread() {
  }

  // destructor
  gtkServer::guiThread::~guiThread() {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 1)
    cerr << "gtkServer::guiThread::destructor called!" << endl;
#   endif

    /*
     * BUG: this should work, but it doesn't and nobody knows why!
     * The problem is that the GTK is somehow already partially
     * destroyed or cannot answer the termination requests, this
     * produces following error: Gdk-ERROR **: Fatal IO error 9 (Bad
     * file descriptor) on X server :0.0.  As workaround, the
     * guiThread will be forced to terminate, and the operating system
     * should destroy all GTK stuff.  This job is well done by Linux,
     * but maybe Windows cannot cope with it!  The user can call
     * gtkServer::server.stop() if he wants to enssure the proper
     * conclusion of the gui server
     */
    stop();

    // thread::stop();
  }

  void gtkServer::guiThread::run() {
    toolkitInit();
    toolkitMainLoop();

#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "guiThread::run() ready!" << endl;
#   endif
  }

  void gtkServer::guiThread::cleanUp() {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "guiThread::cleanUp() called." << endl;
#   endif

    gdk_threads_enter();
    gtk_main_quit();
# if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "  gtk_main_quit() called." << endl;
# endif
    gdk_threads_leave();

#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "guiThread::cleanUp() done." << endl;
#   endif
  }

  void gtkServer::guiThread::start() {
    if (!isAlive()) {
      // at the end of the main thread, try to clean up everything properly
      thread::start();
      waitUntilInitReady();
    }
  }

  void gtkServer::guiThread::stop() {
    if (isAlive()) {

#     if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
      cerr << "guiThread::stop() called" << endl;
#     endif

      gtk_main_quit();
      thread::stop();

#     if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
      cerr << "guiThread::stopped." << endl;
#     endif
    }
  }

  // initialize GTK
  void gtkServer::guiThread::toolkitInit() {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "toolkitInit called!" << endl;
#   endif

    int argc;
    char** argv = new char*[3];

#ifdef _DEBUG
    argc = 2;
    argv[0]=new char[strlen("lti::viewer")+1];
    argv[1]=new char[strlen("--debug")+1];
    argv[2]=0;
    strcpy(argv[0],"lti::viewer");
    strcpy(argv[1],"--debug");
#else
    argc = 1;
    argv[0]=new char[strlen("lti::viewer")+1];
    argv[1]=new char[1];
    argv[2]=0;
    strcpy(argv[0],"lti::viewer");
    strcpy(argv[1],"");
#endif

    if (!g_thread_supported()) {
      g_thread_init(NULL); // NULL means use the default vtable* functions
    }

    /*
     * Some problems with the Windows GTK version forced us to leave the locale
     * uninitialized here, otherwise parsing of floating numbers with 1.0
     * or 1,0 will fail.
     */
    // gtk_set_locale();

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application.
     */
    gtk_init(&argc, &argv);
    gdk_rgb_init();

    delete[] argv[0];
    delete[] argv[1];
    delete[] argv;
  }

  void gtkServer::guiThread::toolkitMainLoop() {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "toolkitMainLoop called!" << endl;
#   endif

    gdk_threads_enter();

    // caller thread wait for post() in a semaphore, which will
    // be set by the callback-function initializationTimeout()
    gtk_timeout_add(1,initializationTimeout,0);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event).
     */
    gtk_main ();

    gdk_threads_leave();

#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "end of toolkitMainLoop!" << endl;
#   endif
  }

  void gtkServer::guiThread::waitUntilInitReady() {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "waiting for GUI toolkit initialization..." << endl;
#   endif

    initReady.wait();

#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
    cerr << "GUI toolkit reports initialization completed!" << endl;
#   endif

  }

  gint gtkServer::guiThread::applicationEnd(gpointer data) {
# ifdef _DEBUG_GUI_LEVEL
    cerr << "->applicationEnd timer catched!" << endl;
# endif
    gdk_threads_enter();
    gtk_main_quit();
# ifdef _DEBUG_GUI_LEVEL
    cerr << "  gtk_main_quit() called." << endl;
# endif
    gdk_threads_leave();

    initReady.post();
    return 0;
  }


  // callback function
  gint gtkServer::guiThread::initializationTimeout(gpointer data) {
#   if  defined(_DEBUG_GUI_LEVEL) && (_DEBUG_GUI_LEVEL > 4)
      cerr << "GUI toolkit initialization completed!" << endl;
#   endif
    initReady.post();
    return 0; // destroy timer!
  }

}

#endif
