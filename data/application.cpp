/* Copyright (C) 2011 and 2013 Chris Vine

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

#include <algorithm>

#include <glib.h>

#include <c++-gtk-utils/application.h>
#include <c++-gtk-utils/shared_handle.h>
#include <c++-gtk-utils/thread.h>

#ifdef CGU_USE_GTK
#if GTK_CHECK_VERSION(2,99,0)

#define CGU_ID_PREFIX "org.cgu.applications"

// the GObject callback functions with both C linkage
// specification and internal linkage
extern "C" {
  static void cgu_application_activate(GApplication*, void* data) {
    Cgu::Application* app = static_cast<Cgu::Application*>(data);
    // provide a CancelBlock here to make this function NPTL friendly,
    // as we have a catch-all without rethrowing
    Cgu::Thread::CancelBlock b;
    try {
      app->activate.emit(app);
    }
    // we can't propagate exceptions from functions with C linkage.
    catch (...) {
      g_critical("Exception thrown in cgu_application_activate()\n");
    }
  }

  static void cgu_application_startup(GApplication*, void* data) {
    Cgu::Application* app = static_cast<Cgu::Application*>(data);
    // provide a CancelBlock here to make this function NPTL friendly,
    // as we have a catch-all without rethrowing
    Cgu::Thread::CancelBlock b;
    try {
      app->startup.emit(app);
    }
    // we can't propagate exceptions from functions with C linkage.
    catch (...) {
      g_critical("Exception thrown in cgu_application_startup()\n");
    }
  }

  static gint cgu_application_command_line(GApplication*,
					   GApplicationCommandLine* cl,
					   void* data) {
    gint ret = 0;
    Cgu::Application* app = static_cast<Cgu::Application*>(data);
    // provide a CancelBlock here to make this function NPTL friendly,
    // as we have a catch-all without rethrowing
    Cgu::Thread::CancelBlock b;
    try {
      app->command_line.emit(app, cl, ret);
    }
    // we can't propagate exceptions from functions with C linkage.
    catch (...) {
      g_critical("Exception thrown in cgu_application_command_line()\n");
      ret = -1;
    }
    return ret;
  }

  static void cgu_application_open(GApplication*,
				   GFile** files, gint n_files,
				   gchar* hint, void* data) {
    Cgu::Application* app = static_cast<Cgu::Application*>(data);
    // provide a CancelBlock here to make this function NPTL friendly,
    // as we have a catch-all without rethrowing
    Cgu::Thread::CancelBlock b;
    try {
      app->open.emit(app, std::pair<GFile**, gint>(files, n_files), hint);
    }
    // we can't propagate exceptions from functions with C linkage.
    catch (...) {
      g_critical("Exception thrown in cgu_application_open()\n");
    }
  }
} // extern "C"

namespace Cgu {

void Application::add(Cgu::WinBase* win) {
  // this is superstition: I think gtk_application_add_window() and
  // gtk_window_set_application() do the same thing, but the
  // documentation is too poor to be sure
  if (win_list.empty()) {
    win_list.push_back(win);
    win->set_application(this);
    gtk_window_set_application(win->get_win(), app);
  }
  else {
    win_list.push_back(win);
    win->set_application(this);
    gtk_application_add_window(app, win->get_win());
  }
}

bool Application::remove(WinBase* win) {

  bool ret = false;
  std::list<WinBase*>::iterator iter =
    std::find(win_list.begin(), win_list.end(), win);

  if (iter != win_list.end()) {
    win_list.erase(iter);
    win->unset_application();
    gtk_application_remove_window(app, win->get_win());
    ret = true;
  }
  return ret;
}

Application::Application(const char* prog_name, GApplicationFlags flags) {

  GcharScopedHandle id{g_strdup_printf("%s.%s", CGU_ID_PREFIX, prog_name)};
  if (!g_application_id_is_valid(id))
    throw ApplicationNameError();

  app.reset(gtk_application_new(id, flags));

  g_signal_connect((GObject*)app.get(), "activate",
                   G_CALLBACK(cgu_application_activate), this);
  g_signal_connect((GObject*)app.get(), "startup",
                   G_CALLBACK(cgu_application_startup), this);
  g_signal_connect((GObject*)app.get(), "command_line",
                   G_CALLBACK(cgu_application_command_line), this);
  g_signal_connect((GObject*)app.get(), "open",
                   G_CALLBACK(cgu_application_open), this);
}

} // namespace Cgu

#endif // GTK_CHECK_VERSION
#endif // CGU_USE_GTK
