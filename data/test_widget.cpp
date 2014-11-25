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

#include <glib.h>
#include <gtk/gtk.h>

#include <utility>

#include <c++-gtk-utils/widget.h>
#include <c++-gtk-utils/gobj_handle.h>

using namespace Cgu;

extern "C" {
static void test_widget() {

  GobjWeakHandle<GtkWidget> wh;

  {
    MainWidgetBase w1{gtk_button_new()};
    MainWidgetBase w2{gtk_button_new()};
    g_assert(!g_object_is_floating(G_OBJECT(w1.get_main_widget())));
    g_assert(!g_object_is_floating(G_OBJECT(w2.get_main_widget())));

    wh.reset(w1.get_main_widget());
    g_assert(w1.get_main_widget() == wh.get());
    
    w1 = std::move(w2);
    g_assert(!w2.get_main_widget());
    g_assert(!wh.valid());
    wh.reset(w1.get_main_widget());
    g_assert(w1.get_main_widget() == wh.get());
  }

  g_assert(!wh.valid());
}
} // extern "C"


int main (int argc, char* argv[]) {
  gtk_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/widget/main_widget_base", test_widget); 

  return g_test_run();
}
