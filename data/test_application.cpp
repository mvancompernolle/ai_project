/* Copyright (C) 2012 to 2014 Chris Vine

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

#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_0

#include <glib.h>
#include <gtk/gtk.h>
#include <utility>

#if GTK_CHECK_VERSION(2,99,0)
#include <c++-gtk-utils/application.h>
#endif

#include <c++-gtk-utils/window.h>
#include <c++-gtk-utils/callback.h>

using namespace Cgu;

class Message: public Cgu::WinBase {
public:
  void pub_close() {close();}
  Message(const char* text);
};

Message::Message(const char* text): WinBase{"Message", 0, false} {
#if GTK_CHECK_VERSION(3,4,0)
  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_set_homogeneous(GTK_BOX(box), false);
  gtk_container_add(GTK_CONTAINER(get_win()), box);
  GtkWidget* label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(box), label,
		     true, false, 0);
  GtkWidget* button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(box), button_box,
		     false, false, 0);
#else
  GtkWidget* box = gtk_vbox_new(false, 2);
  gtk_container_add(GTK_CONTAINER(get_win()), box);
  GtkWidget* label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(box), label,
                     true, false, 0);
  GtkWidget* button_box = gtk_hbutton_box_new();
  gtk_box_pack_start(GTK_BOX(box), button_box,
                     false, false, 0);
#endif
  GtkWidget* button = gtk_button_new_with_label("OK");
  gtk_container_add(GTK_CONTAINER(button_box), button);
  gtk_widget_set_can_default(button, true);
}

#if GTK_CHECK_VERSION(2,99,0)
void app_activate(bool& success, Application* app) {
  Message* dialog = new Message("Test");
  app->add(dialog);
  gtk_widget_realize(GTK_WIDGET(dialog->get_win()));
  g_assert_cmpuint(app->get_win_count(), ==, 1U);
  success = true;

  // make Cgu::Application::run() unblock
  Callback::post(Callback::make(*dialog, &Message::pub_close));
}

void app_command(bool& success, Application* app, GApplicationCommandLine* cl, gint&) {
  Message* dialog = new Message("Test");
  app->add(dialog);
  gtk_widget_realize(GTK_WIDGET(dialog->get_win()));
  g_assert_cmpuint(app->get_win_count(), ==, 1U);
  success = true;

  // make Cgu::Application::run() unblock
  Callback::post(Callback::make(*dialog, &Message::pub_close));
}

void app_open(bool& success, Application* app, std::pair<GFile**, gint> files, gchar*) {
  Message* dialog = new Message("Test");
  app->add(dialog);
  gtk_widget_realize(GTK_WIDGET(dialog->get_win()));
  g_assert_cmpuint(app->get_win_count(), ==, 1U);
  success = true;

  // make Cgu::Application::run() unblock
  Callback::post(Callback::make(*dialog, &Message::pub_close));
}
#endif // GTK_CHECK_VERSION

extern "C" {

static void test_application_window() {
  Message dialog("Test");
  GtkWidget* w = GTK_WIDGET(dialog.get_win());
  gtk_widget_realize(w);
#if GTK_CHECK_VERSION(2,20,0)
  g_assert_cmpuint((unsigned int)gtk_widget_get_realized(w), !=, 0);
#else
  g_assert_cmpuint(((unsigned int)GTK_WIDGET_REALIZED(w)), !=, 0);
#endif

  // make Cgu::Application::run() unblock
  Callback::post(Callback::make(dialog, &Message::pub_close));
  dialog.exec();
}

#if GTK_CHECK_VERSION(2,99,0)
static void test_application_activate() {
  Application app{"test_prog1", G_APPLICATION_FLAGS_NONE};
  bool success = false;
  app.activate.connect(Callback::make<bool&>(app_activate, success));
  app.run(0, 0);
  g_assert_cmpuint((unsigned int)success, !=, 0);
}

static void test_application_command() {
  Application app{"test_prog2", G_APPLICATION_HANDLES_COMMAND_LINE};
  bool success = false;
  app.command_line.connect(Callback::make<bool&>(app_command, success));
  app.run(0, 0);
  g_assert_cmpuint((unsigned int)success, !=, 0);
}

static void test_application_open() {
  Application app{"test_prog3", G_APPLICATION_HANDLES_OPEN};
  bool success = false;
  app.open.connect(Callback::make<bool&>(app_open, success));
  char* args[] = {(char*)"test_application", (char*)"test"};
  app.run(2, args);
  g_assert_cmpuint((unsigned int)success, !=, 0);
}
#endif // GTK_CHECK_VERSION
} // extern "C"


int main (int argc, char* argv[]) {

  gtk_test_init(&argc, &argv, static_cast<void*>(0));

  // with GTK+2 just test Cgu::WinBase
  g_test_add_func("/application/window", test_application_window); 

#if GTK_CHECK_VERSION(2,99,0)
  g_test_add_func("/application/activate", test_application_activate); 
  g_test_add_func("/application/command", test_application_command); 
  g_test_add_func("/application/open", test_application_open);
#endif // GTK_CHECK_VERSION

  return g_test_run();  
}
