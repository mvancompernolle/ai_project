/* Copyright (C) 2006, 2007, 2011 and 2013 Chris Vine

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

#include <c++-gtk-utils/lib_defs.h>      // for ENABLE_NLS

#include <unistd.h>

#include <new> // for std::bad_alloc

#include <glib.h>

#include <c++-gtk-utils/file_print_manager.h>
#include <c++-gtk-utils/thread.h>
#include <c++-gtk-utils/gerror_handle.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

#ifdef CGU_USE_GTK

Cgu::GobjHandle<GtkPrintSettings> Cgu::FilePrintManager::print_settings_h;

namespace Cgu {
class FilePrintDialog::CB {
public:
  static void selected(int id, void* data) {
    Cgu::FilePrintDialog* instance_p = static_cast<Cgu::FilePrintDialog*>(data);
    // provide a CancelBlock here to make this function NPTL friendly,
    // as we have a catch-all without rethrowing
    {
      Cgu::Thread::CancelBlock b;
      try {
	if (id == GTK_RESPONSE_OK) instance_p->accepted();
	else instance_p->rejected();
      }
      catch (...) {
	g_critical("Exception thrown in FilePrintDialogCB::selected()\n");
      }
    } // end of CancelBlock scope block
    instance_p->close();
  }
};
} // namespace Cgu

// the GObject callback function with both C linkage
// specification and internal linkage
extern "C" {
  static void cgu_fpd_selected(GtkDialog*, int id, void* data) {
    Cgu::FilePrintDialog::CB::selected(id, data);
  }
} // extern "C"


namespace Cgu {

FilePrintDialog::FilePrintDialog(GtkWindow* parent_p, GtkPrintSettings* print_settings_p,
				 const char* caption, GdkPixbuf* window_icon_p):
                                    WinBase(caption, window_icon_p,
					    true, parent_p,
					    (GtkWindow*)gtk_print_unix_dialog_new(0, 0)) {

  gtk_window_set_type_hint(get_win(), GDK_WINDOW_TYPE_HINT_DIALOG);

  g_signal_connect((GObject*)get_win(), "response",
		   G_CALLBACK(cgu_fpd_selected), this);

  gtk_window_set_position(get_win(), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_resizable(get_win(), false);

  if (print_settings_p) {
    gtk_print_unix_dialog_set_settings((GtkPrintUnixDialog*)get_win(), print_settings_p);
  }

  GtkPrintCapabilities capabilities = GtkPrintCapabilities(0);
  capabilities = GtkPrintCapabilities(GTK_PRINT_CAPABILITY_GENERATE_PS);

  gtk_print_unix_dialog_set_manual_capabilities((GtkPrintUnixDialog*)get_win(),
						capabilities);

  gtk_widget_show_all((GtkWidget*)get_win());
}

void FilePrintDialog::on_delete_event() {
  // we have connected to the response signal, so just return
  return;
}

GtkPrinter* FilePrintDialog::get_printer() const {
  return gtk_print_unix_dialog_get_selected_printer(GTK_PRINT_UNIX_DIALOG(get_win()));
}

GobjHandle<GtkPrintSettings> FilePrintDialog::get_settings() const {
  return GobjHandle<GtkPrintSettings>{gtk_print_unix_dialog_get_settings(GTK_PRINT_UNIX_DIALOG(get_win()))};
}

GtkPageSetup* FilePrintDialog::get_page_setup() const {
  return gtk_print_unix_dialog_get_page_setup(GTK_PRINT_UNIX_DIALOG(get_win()));
}

} // namespace Cgu


#ifndef DOXYGEN_PARSING
namespace Cgu {
class FilePrintManager::CB {
public:
  static void job_complete(void* data, const GError* error) {
    Cgu::FilePrintManager* instance_p = static_cast<Cgu::FilePrintManager*>(data);
    if (error) {
      g_warning("CguFilePrintManagerCB::file_print_job_complete(): %s\n", error->message);
      // we shouldn't free the GError object - the print system will do
      // that (see gtk_print_backend_cups_print_stream(), which calls
      // cups_request_execute() with this callback as part of a user
      // data struct (ps argument) in a call to cups_print_cb() which
      // calls ps->callback and thus this callback function). I imagine
      // the lpr backend does something similar (the interface shouldn't
      // be different)
    }
    try {
      instance_p->clean_up();
    }
    catch (std::bad_alloc&) {
      g_critical("std::bad_alloc exception thrown in FilePrintManagerCB::job_complete()\n");
    }
    instance_p->unref();
  }
};
} // namespace Cgu

// the GObject callback function with both C linkage
// specification and internal linkage
extern "C" {
  static void cgu_fpm_job_complete(GtkPrintJob*, void* data,
#if GTK_CHECK_VERSION(2,91,0)
				   const GError* error
#else
				   GError* error
#endif
				   ) {
    Cgu::FilePrintManager::CB::job_complete(data, error);
  }
} // extern "C"

#endif // DOXYGEN_PARSING

namespace Cgu {

FilePrintManager::~FilePrintManager() {

  // empty the filename through a mutex to synchronise memory
  Thread::Mutex::Lock lock{mutex};
  try {
    filename = "";
  }
  catch (std::bad_alloc&) {}
}

IntrusivePtr<FilePrintManager> FilePrintManager::create_manager(GtkWindow* parent,
								const std::string& caption_text,
								const GobjHandle<GdkPixbuf>& icon_h) {

  IntrusivePtr<FilePrintManager> instance_i{new FilePrintManager};
  instance_i->print_notifier.connect(Callback::make(*instance_i, &FilePrintManager::show_dialog));

  Thread::Mutex::Lock lock{instance_i->mutex};
  instance_i->parent_p = parent;
  instance_i->caption = caption_text;
  instance_i->window_icon_h = icon_h;
  instance_i->ready = true;

  return instance_i;
}

bool FilePrintManager::set_filename(const char* filename_, bool manage_file) {
  Thread::Mutex::Lock lock{mutex};
  if (!ready) return false;
  filename = filename_;
  manage = manage_file;
  return true;
}

bool FilePrintManager::print() {

  { // scope block for mutex lock
    Thread::Mutex::Lock lock{mutex};
    if (!ready) return false;
    // protect our innards
    ready = false;
  }

  // take ownership of ourselves
  ref();
  print_notifier();
  return true;
}

// this method is only called by emitting the print_notifier Notifier object
// in FilePrintManager::print()
void FilePrintManager::show_dialog() {

  // hand back ownership to local scope so that if there is a problem
  // or an exception is thrown this method cleans itself up (we will
  // ref() again at the end of this method if all has gone well)
  IntrusivePtr<FilePrintManager> temp{this};
  unref();

  mutex.lock();
  
  if (filename.empty()) {
    ready = true;
    mutex.unlock();
    g_warning("FilePrintManager::show_dialog(): %s\n",
	      gettext("No file has been specified for printing"));
    return;
  }
  mutex.unlock();

  // this method is called via the Notifier object, so it must be executing
  // in the main GUI thread - it is therefore safe to call any GTK+ functions
  // in this method and in FilePrintManager::print_file()
  dialog_p = new FilePrintDialog{parent_p, print_settings_h.get(), caption.c_str(), window_icon_h};
  try {
    dialog_p->accepted.connect(Callback::make(*this, &FilePrintManager::print_file));
    dialog_p->rejected.connect(Callback::make(*this, &FilePrintManager::print_cancel));
    // we don't need to worry about cleaning up the first callback if the second fails
    // to allocate memory - the 'accepted' emitter, and prior to that a Callback::Functor
    // object, takes ownership
  }
  catch (...) {
    delete dialog_p;
    throw;   // catch it in io_watch dispatcher for the Notifier pipe
  }

  // there is no memory leak -- the memory will be deleted when the FilePrintDialog object closes
 
  // regain ownership of ourselves
  ref();
}

// this could throw because clean_up() could throw std::bad_alloc
// that doesn't matter as this method is only executed through the
// FilePrintDialog::accepted Emitter object and exceptions thrown
// when emitting are caught by the FilePrintDialog implementation
void FilePrintManager::print_file() {

  // hand back ownership to local scope so that if there is a problem
  // this method cleans itself up (we will ref() again at the end of 
  // this method if all has gone well)
  IntrusivePtr<FilePrintManager> temp{this};
  unref();

  GtkPrinter* printer_p = dialog_p->get_printer();
  if (!printer_p) {
    g_warning("FilePrintManager::print_file(): %s\n",
	      gettext("No valid printer selected"));
    clean_up();
  }
  else {
    print_settings_h = dialog_p->get_settings();
    GtkPageSetup* page_setup_p = dialog_p->get_page_setup();
    GobjHandle<GtkPrintJob> print_job_h(gtk_print_job_new("efax-gtk print job",
							  printer_p,
							  print_settings_h,
							  page_setup_p));
    GError* error_p = 0;
    bool result;
    { // scope block for mutex lock
      Thread::Mutex::Lock lock{mutex};
      result = gtk_print_job_set_source_file(print_job_h, filename.c_str(), &error_p);
    }
    if (!result) {
      if (error_p) {
	GerrorScopedHandle handle_h{error_p};
	g_warning("FilePrintManager::print_file(): %s\n", error_p->message);
      }
      clean_up();
    }
    else {
      // regain ownership of ourselves and print the job (the
      // print system will do the final unreference in the
      // FilePrintManagerCB::file_print_job_complete() function)
      ref();

      // gtk_print_job_send() will cause the print system to
      // acquire ownership of the GtkPrintJob object (so our
      // GobjHandle object can go out of scope)
      gtk_print_job_send(print_job_h,
			 cgu_fpm_job_complete,
			 this, 0);
    }
  }
}

// this could throw because clean_up() could throw std::bad_alloc
// that doesn't matter as thsi method is only executed through the
// FilePrintDialog::rejected Emitter object, and exceptions thrown
// when emitting are caught by the FilePrintDialog implementation
void FilePrintManager::print_cancel() {
  clean_up();
  unref();
}

// this could throw std::bad_alloc
void FilePrintManager::clean_up() {
  Thread::Mutex::Lock lock{mutex};
  if (manage && !filename.empty()) {
    unlink(filename.c_str());
  }
  ready = true;
}

} // namespace Cgu

#endif // CGU_USE_GTK
