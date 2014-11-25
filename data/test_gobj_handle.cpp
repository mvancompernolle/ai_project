/* Copyright (C) 2012 Chris Vine

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

#include <glib.h>
#include <gtk/gtk.h>

#include <c++-gtk-utils/gobj_handle.h>

using namespace Cgu;

extern "C" {
static void test_handles() {

  GobjHandle<GtkWidget> strong{gtk_button_new()};
  g_assert(!g_object_is_floating(G_OBJECT(strong.get())));

  GobjWeakHandle<GtkWidget> weak{strong};
  g_assert(strong.get() == weak.get());

  strong.reset();
  
  g_assert(!weak.valid());
  g_assert(!weak);

  try {
    weak.get();
    g_assert_not_reached();
  }
  catch (GobjWeakHandleError&) {}
}
} // extern "C"


int main (int argc, char* argv[]) {
  gtk_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/gobj_handle/handles", test_handles); 

  return g_test_run();
}
