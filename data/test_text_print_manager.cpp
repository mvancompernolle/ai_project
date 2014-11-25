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

#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_0

#include <gtk/gtk.h>
#include <memory>
#include <string>

#include <c++-gtk-utils/cgu_config.h>
#include <c++-gtk-utils/window.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/text_print_manager.h>

using namespace Cgu;

extern "C" void message_button_clicked(GtkWidget*, void*);

class ProgWin: public WinBase {
  GtkWidget* print_button;
  GtkWidget* view_button;
  GtkWidget* close_button;
  void print_page();
  void view_page();
public:
  friend void message_button_clicked(GtkWidget*, void*);
  ProgWin();
};

void message_button_clicked(GtkWidget* w, void* data) {
  ProgWin* instance = static_cast<ProgWin*>(data);
  if (w == instance->print_button) instance->print_page();
  else if (w == instance->view_button) instance->view_page();
  else instance->close();
}

ProgWin::ProgWin(): WinBase{"Print test", 0, false} {
  gtk_container_set_border_width(GTK_CONTAINER(get_win()), 5);
  GtkWidget* box = gtk_vbox_new(false, 2);
  gtk_container_add(GTK_CONTAINER(get_win()), box);
  GtkWidget* label = gtk_label_new("Print test page?");
  gtk_box_pack_start(GTK_BOX(box), label,
                     true, false, 0);
  GtkWidget* button_box = gtk_hbutton_box_new();
  gtk_box_pack_start(GTK_BOX(box), button_box,
                     false, false, 0);
  close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
  gtk_container_add(GTK_CONTAINER(button_box), close_button);
  gtk_widget_set_can_default(close_button, true);
  g_signal_connect(G_OBJECT(close_button), "clicked",
		   G_CALLBACK(message_button_clicked), this);
  view_button = gtk_button_new_with_label("View");
  gtk_container_add(GTK_CONTAINER(button_box), view_button);
  gtk_widget_set_can_default(view_button, true);
  g_signal_connect(G_OBJECT(view_button), "clicked",
		   G_CALLBACK(message_button_clicked), this);
  print_button = gtk_button_new_with_label("Print");
  gtk_container_add(GTK_CONTAINER(button_box), print_button);
  gtk_widget_set_can_default(print_button, true);
  g_signal_connect(G_OBJECT(print_button), "clicked",
		   G_CALLBACK(message_button_clicked), this);
}

void ProgWin::print_page() {
  std::unique_ptr<std::string> text(new std::string("TextPrintManger\n\n"
						    "Test page"));
  auto pm = Cgu::TextPrintManager::create_manager();
  pm->set_text(text);
  pm->print();
}

void ProgWin::view_page() {
  std::unique_ptr<std::string> text(new std::string("TextPrintManger\n\n"
						    "Test page"));
  auto pm = Cgu::TextPrintManager::create_manager();
  pm->set_text(text);
  pm->view();
}

int main (int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  ProgWin prog_win;

  prog_win.show_all();
  prog_win.exec();
}
