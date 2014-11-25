/* Copyright (C) 2009 and 2011 Chris Vine

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

#include <c++-gtk-utils/lib_defs.h>  // for config.h, HAVE_GIO_DBUS and HAVE_DBUS_GLIB
#include <c++-gtk-utils/prog_present.h>

/* CGU_PROG_PRESENTER_DBUS_INTERFACE needs to match the interface name
 in prog-presenter.xml used to generate prog-presenter-glue.h and
 prog-presenter-bindings.h because we might be compiling against dbus-glib.
 The introspection data for gio's gdbus will also use this interface name */
#define CGU_PROG_PRESENTER_DBUS_INTERFACE "org.cgu.CguProgPresenterInterface"

/* REGISTER_SERVICE_PREFIX can be anything sensible  */
#define REGISTER_SERVICE_PREFIX "org.cgu.progs"

/* REGISTER_PATH_PREFIX is the namespace for the dbus object path
   (name) and can be anything sensible */
#define REGISTER_PATH_PREFIX "/org/cgu/progs"

#ifdef HAVE_GIO_DBUS

#include <gio/gio.h>
#include <glib-object.h>

#include <c++-gtk-utils/gobj_handle.h>
#include <c++-gtk-utils/gvar_handle.h>

/* since gio does are not use libdbus, provide our own defines */
#define CGU_DBUS_SERVICE              "org.freedesktop.DBus"
#define CGU_DBUS_PATH                 "/org/freedesktop/DBus"
#define CGU_DBUS_INTERFACE            "org.freedesktop.DBus"
#define CGU_DBUS_BECOME_PRIMARY_OWNER 1

namespace Cgu {

/* with gio we have to provide our introspection data in the code */
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='org.cgu.CguProgPresenterInterface'>"
  "    <method name='Present'>"
  "      <arg type='v' name='args' direction='in'/>"
  "    </method>"
  "  </interface>"
  "</node>";

/* for error reporting on CguProgPresenterInterface */
typedef enum {CGU_PROG_PRESENTER_FALSE = 0} CguProgPresenterError;

static GDBusErrorEntry cgu_prog_presenter_errors[] = {
  {CGU_PROG_PRESENTER_FALSE, "org.cgu.CguProgPresenterInterface.FalseReturnValue"}
};

static GQuark cgu_prog_presenter_error_get_quark() {
  static volatile gsize error_quark = 0;
  if (!error_quark) {
    g_dbus_error_register_error_domain("CguProgPresenterErrorDomain",
				       &error_quark,
				       cgu_prog_presenter_errors,
				       1);
  }
  return (GQuark)error_quark;  
}

/* this struct contains the function pointer and persistent
   data executed by cgu_present_cb() */
static struct {
  Cgu::PresentFunc func;
  void* data;
} cgu_present_items = {0, 0};

/* this is the dbus object callback */
extern "C" {
static void cgu_present_cb(GDBusConnection*,
			   const gchar*,
			   const gchar*,
			   const gchar*,
			   const gchar* method_name,
			   GVariant* parameters,
			   GDBusMethodInvocation* invocation,
			   void*) {

  if (!g_strcmp0(method_name, "Present")) {

    gboolean ret;
    
    if (!cgu_present_items.func) // we shouldn't ever get a false value
      ret = FALSE;
    else {
      // parameters is a variant holding a tuple containing a variant
      // holding either a bytestring-array or a boolean: now decompose

      GvarHandle decomposed{g_variant_get_child_value(parameters, 0)};
      // decomposed is a variant holding a variant holding either
      // a bytestring-array or a boolean

      decomposed.reset(g_variant_get_variant(decomposed));
      // decomposed is now a variant holding either a bytestring-array or
      // a boolean: the variant will be of boolean type if there are no
      // instance arguments

      if (!g_variant_type_equal(g_variant_get_type(decomposed), G_VARIANT_TYPE_BOOLEAN)) {
	const gchar** ar = g_variant_get_bytestring_array(decomposed, 0);
	ret = cgu_present_items.func(cgu_present_items.data, (const char**)ar);
	g_free(ar);
      }
      else
	ret = cgu_present_items.func(cgu_present_items.data, 0);
    }

    if (!ret) {
      g_dbus_method_invocation_return_error(invocation,
					    cgu_prog_presenter_error_get_quark(),
					    CGU_PROG_PRESENTER_FALSE,
					    "%s",
					    "Cgu::PresentFunc function returned FALSE");
    }
    else
      g_dbus_method_invocation_return_value(invocation, 0);
  }
  else
    g_warning("ProgPresenter call to invalid method name\n");
}
} // extern "C"

static const GDBusInterfaceVTable cgu_prog_presenter_interface_vtable = {
  cgu_present_cb,
  0,
  0,
};

/* register_service and register_path are allocated on the heap in
   register_prog() as they may need to be visible also in
   present_instance().  They are freed in register_prog() or
   present_instance(), as appropriate */
static gchar* register_service = 0;
static gchar* register_path = 0;

static void clean_up_dbus_variables() {
  g_free(register_service);
  g_free(register_path);
  register_service = 0;
  register_path = 0;
}

// making session_connection static will shut valgrind up
static GDBusConnection* session_connection;

bool register_prog(const char* prog_name, PresentFunc func, void* object_data) {

  // we only call register_prog() once in any one program invocation -
  // once it has returned FALSE go on to call present_instance()
  static bool initialised = false;
  // because of that also store previous return value so we can supply
  // it again rather than just abort
  static bool return_val = false;

  if (initialised) {
    g_warning("Only call register_prog() once in any one program instance - ignoring this call\n");
    return return_val;
  }
  initialised = true;

  GError* error = 0;

  // use the session bus
  // don't keep the connection object in a GobjHandle, as it needs to survive
  // the call to this method and last throughout the running of this process
  // instead it has been made a static object 
  session_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, 0, &error);
  if (!session_connection) {
    // problem in obtaining a connection - bail out
    g_critical("Failed to open connection to bus: %s\n", error->message);
    g_error_free(error);
    return return_val;
  }

  // we have a connection on the session bus
  g_object_set(session_connection,
	       "exit-on-close", false,
	       static_cast<void*>(0));

  gchar* delimited_prog_name = g_strdup(prog_name);
  g_strdelimit(delimited_prog_name, "-|<>./+", '_');
  register_service = g_strdup_printf("%s.%s", REGISTER_SERVICE_PREFIX, delimited_prog_name);
  register_path = g_strdup_printf("%s/%s/%s", REGISTER_PATH_PREFIX, delimited_prog_name, "presenter");
  g_free(delimited_prog_name);

  GobjHandle<GDBusProxy> proxy{g_dbus_proxy_new_sync(session_connection,
						     G_DBUS_PROXY_FLAGS_NONE,
						     0,
						     CGU_DBUS_SERVICE,
						     CGU_DBUS_PATH,
						     CGU_DBUS_INTERFACE,
						     0,
						     &error)};

  if (!proxy.get()) {
    // problem in creating proxy object - bail out
    g_critical("Failed to create proxy object: %s\n", error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return return_val;
  }

  GvarHandle result{g_dbus_proxy_call_sync(proxy,
					   "RequestName",
					   g_variant_new("(su)", register_service, 0),
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   0,
					   &error)};
  if (!result.get()) {
    // problem in obtaining the service name - bail out
    if (g_dbus_error_is_remote_error(error)) {
      gchar* dbus_msg = g_dbus_error_get_remote_error(error);
      if (dbus_msg) {
	g_critical("Dbus error: %s", dbus_msg);
	g_dbus_error_strip_remote_error(error);
	g_free(dbus_msg);
      }
    }
    g_critical("Failed to obtain name %s on session bus: %s", register_service, error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return return_val;
  }
  if (g_variant_get_uint32(GvarHandle{g_variant_get_child_value(result, 0)})
      != CGU_DBUS_BECOME_PRIMARY_OWNER) {
    // the program is already running - return FALSE
    return return_val;
  }

  // first invocation of the program - this process now owns the
  // service name, so register a dbus object for cgu_present_cb()
  GDBusNodeInfo* introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, 0);
  if (!introspection_data) { // this can't really happen
    g_critical("Failed to create introspection data for GDBus\n");
    clean_up_dbus_variables();
    return return_val;
  }
  cgu_present_items.func = func;
  cgu_present_items.data = object_data;

  guint id = g_dbus_connection_register_object(session_connection,
					       register_path,
					       introspection_data->interfaces[0],
					       &cgu_prog_presenter_interface_vtable,
					       0,
					       0,
					       &error);
  if (!id) {
    // problem in registering dbus object - bail out
    g_critical("Failed to register dbus object at path %s on session bus: %s",
	       register_path, error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    g_dbus_node_info_unref(introspection_data);
    return return_val;
  }

  // we don't need register_service or register_path any more as
  // this is the first invocation of the program and we will not
  // call present_instance()
  clean_up_dbus_variables();
  g_dbus_node_info_unref(introspection_data);

  return_val = true;
  return return_val;
}

int present_instance(const char** instance_args) {

  if (!register_service) {
    g_warning("Error in present_instance(): either register_prog() has not been called,\n"
	      "there was an error in obtaining a connection or service name in register_prog(),\n"
	      "present_instance() has been called in the first invocation of the program\n"
	      "or present_instance() has already been called once before in this invocation of\n"
              "the program.  Ignoring this call() to present_instance\n");
    return 1;
  }
  
  GError* error = 0;

  // use the session bus
  GobjHandle<GDBusConnection> connection{g_bus_get_sync(G_BUS_TYPE_SESSION, 0, &error)};
  if (!connection.get()) {
    // problem in obtaining a connection - bail out
    g_critical("Failed to open connection to bus: %s\n", error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return 1;
  }

  // we have a connection on the session bus
  g_object_set(connection.get(),
	       "exit-on-close", false,
	       static_cast<void*>(0));

  GobjHandle<GDBusProxy> proxy{g_dbus_proxy_new_sync(connection,
						     G_DBUS_PROXY_FLAGS_NONE,
						     0,
						     register_service,
						     register_path,
						     CGU_PROG_PRESENTER_DBUS_INTERFACE,
						     0,
						     &error)};
  if (!proxy.get()) {
    // problem in creating proxy object - bail out
    g_critical("Failed to create proxy object: %s\n", error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return 1;
  }

  GvarHandle args;
  if (!instance_args)
    args.reset(g_variant_new_boolean(FALSE));
  else
    args.reset(g_variant_new_bytestring_array((const gchar* const*)instance_args,
					      -1));

  GvarHandle result{g_dbus_proxy_call_sync(proxy,
					   "Present",
					   g_variant_new("(v)", args.get()),
					   G_DBUS_CALL_FLAGS_NONE,
					   -1,
					   0,
					   &error)};
  if (!result.get()) {
    int return_val;
    gchar* dbus_msg = 0;
    if (g_dbus_error_is_remote_error(error)) {
      dbus_msg = g_dbus_error_get_remote_error(error);
      return_val = 2;
    }
    else
      return_val = 1;

    if (dbus_msg) {
      g_dbus_error_strip_remote_error(error);
      g_critical("Failed to present instance: %s", error->message);
      g_critical("Dbus error name is %s", dbus_msg);
      g_free(dbus_msg);
    }
    else
      g_critical("Failed to present instance: %s", error->message);

    g_error_free(error);
    clean_up_dbus_variables();
    return return_val;
  }

  clean_up_dbus_variables();
  return 0;
}

} // namespace Cgu

#elif defined(HAVE_DBUS_GLIB)

#include <glib-object.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

/* GObject stuff */

/* CguProgPresenter is the dbus method part of the functionality in
   this file.  A CguProgPresenter object is created on the first
   occasion that register_prog() is called by the program in question.
   It will listen on the dbus session bus for any messages that are
   sent to it that the program has been started again (the client -
   the new program instance - does this by calling
   present_instance()).

   CguProgPresenter is private to the implementation in this file.
   None of it is exported to the program at large (all its methods are
   static).  The public interface to the functionality in this file
   only consists of register_prog() and present_instance() */

extern "C" {

#define CGU_PROG_PRESENTER_TYPE (cgu_prog_presenter_get_type())
#define CGU_PROG_PRESENTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CGU_PROG_PRESENTER_TYPE, CguProgPresenter))
#define IS_CGU_PROG_PRESENTER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CGU_PROG_PRESENTER_TYPE))

typedef struct _CguProgPresenter {
  GObject object;
  Cgu::PresentFunc func;
  void* data;
} CguProgPresenter;

typedef struct _CguProgPresenterClass {
  GObjectClass object_class;
} CguProgPresenterClass;

/* this is the Present method for interface CguProgPresenterInterface
   required by and registered in prog-presenter-glue.h. It is the
   callback executed by the CguProgPresenter object via dbus after
   present_instance() has been called.  As declared in
   prog-presenter.xml (and registered via make_bindings.sh in
   prog-presenter-glue.h) it is also a CguProgPresenter class
   method */
static gboolean cgu_prog_presenter_present(CguProgPresenter* presenter,
					   const char** args,
					   GError** error);

G_DEFINE_TYPE(CguProgPresenter, cgu_prog_presenter, G_TYPE_OBJECT);

static void cgu_prog_presenter_class_init(CguProgPresenterClass* presenter_class);

static void cgu_prog_presenter_init(CguProgPresenter* presenter);

static void cgu_prog_presenter_set(CguProgPresenter* presenter,
				   Cgu::PresentFunc func,
				   void* object_data);

static CguProgPresenter* cgu_prog_presenter_new();

// any dbus error domains have to be enums registered as GTypes
// but we only have one error code within the domain, namely
// CGU_PROG_PRESENTER_FALSE
typedef enum {CGU_PROG_PRESENTER_FALSE = 0} CguProgPresenterError;

static GType cgu_prog_presenter_error_get_type();

static GQuark cgu_prog_presenter_error_get_quark();

} // extern "C"

#include <c++-gtk-utils/prog-presenter/prog-presenter-glue.h>
#include <c++-gtk-utils/prog-presenter/prog-presenter-bindings.h>

/* the callback executed by dbus when present_instance() is called
   which is also a CguProgPresenter class method */
// this function is declared as static and with C calling specification
gboolean cgu_prog_presenter_present(CguProgPresenter* presenter,
				    const char** args,
				    GError** error) {
  // dbus-glib requires this method to return TRUE if successful

  // just return FALSE if the presenter object is invalid or the
  // cgu_prog_presenter_set() has not been called: that is a
  // programmer error (or a very weird dbus error)
  g_return_val_if_fail(IS_CGU_PROG_PRESENTER(presenter), FALSE);
  g_return_val_if_fail(presenter->func != 0, FALSE);

  gboolean ret = presenter->func(presenter->data, args);
  if (!ret) {
    // let dbus map this error into -
    // domain: DBUS_GERROR
    // code:   DBUS_GERROR_REMOTE_EXCEPTION
    // name:   org.cgu.CguProgPresenterInterface.FalseReturnValue
    g_set_error(error,
		cgu_prog_presenter_error_get_quark(),
		CGU_PROG_PRESENTER_FALSE,
		"%s",
		"Cgu::PresentFunc function returned FALSE");
  }
  return ret;
}

// this function is declared as static and with C calling specification
void cgu_prog_presenter_class_init(CguProgPresenterClass* presenter_class) {
  dbus_g_object_type_install_info(G_TYPE_FROM_CLASS(presenter_class),
				  &dbus_glib_cgu_prog_presenter_object_info);
  dbus_g_error_domain_register(cgu_prog_presenter_error_get_quark(),
			       CGU_PROG_PRESENTER_DBUS_INTERFACE,
			       cgu_prog_presenter_error_get_type());
}

// this function is declared as static and with C calling specification
void cgu_prog_presenter_init(CguProgPresenter* presenter) {
  presenter->func = 0;
  presenter->data = 0;
}

// this function is declared as static and with C calling specification
void cgu_prog_presenter_set(CguProgPresenter* presenter,
			    Cgu::PresentFunc func,
			    void* object_data) {
  presenter->func = func;
  presenter->data = object_data;
}

// this function is declared as static and with C calling specification
CguProgPresenter* cgu_prog_presenter_new() {
  return static_cast<CguProgPresenter*>(g_object_new(CGU_PROG_PRESENTER_TYPE,
						     static_cast<gchar*>(0)));
}

// this function is declared as static and with C calling specification
GType cgu_prog_presenter_error_get_type() {

  // this very cumbersome function just registers the CguProgPresenterError
  // enumeration as a GType with its enumerators
  static GType enum_type = 0;
  if (!enum_type) {
    static const GEnumValue values[] = {
      {CGU_PROG_PRESENTER_FALSE, "CGU_PROG_PRESENTER_FALSE", "FalseReturnValue"},
      {0, 0, 0}
    };
    enum_type = g_enum_register_static("CguProgPresenterError", values);
  }
  return enum_type;
}

// this function is declared as static and with C calling specification
GQuark cgu_prog_presenter_error_get_quark() {
  // provide a GQuark representing the domain name of the error codes
  // in the CguProgPresenterError enumeration
  static volatile GQuark error_quark = 0;
  if (!error_quark) error_quark = g_quark_from_string("CguProgPresenterErrorDomain");
  return error_quark;
}

/* End of GObject stuff */

namespace Cgu {

/* register_service and register_path are allocated on the heap in
   register_prog() as they may need to be visible also in
   present_instance().  They are freed in register_prog() or
   present_instance(), as appropriate */
static gchar* register_service = 0;
static gchar* register_path = 0;

static void clean_up_dbus_variables() {
  g_free(register_service);
  g_free(register_path);
  register_service = 0;
  register_path = 0;
}

// making presenter_instance static will shut valgrind up
CguProgPresenter* presenter_instance;

bool register_prog(const char* prog_name, PresentFunc func, void* object_data) {

  // we only call register_prog() once in any one program invocation -
  // once it has returned FALSE go on to call present_instance()
  static bool initialised = false;
  // because of that also store previous return value so we can supply
  // it again rather than just abort
  static bool return_val = false;

  if (initialised) {
    g_warning("Only call register_prog() once in any one program instance - ignoring this call\n");
    return return_val;
  }
  initialised = true;

  GError* error = 0;

  // use the session bus
  DBusGConnection* connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
  if (!connection) {
    // problem in obtaining a connection - bail out
    g_critical("Failed to open connection to bus: %s\n", error->message);
    g_error_free(error);
    return return_val;
  }

  // we have a connection on the session bus
  gchar* delimited_prog_name = g_strdup(prog_name);
  g_strdelimit(delimited_prog_name, "-|<>./+", '_');
  register_service = g_strdup_printf("%s.%s", REGISTER_SERVICE_PREFIX, delimited_prog_name);
  register_path = g_strdup_printf("%s/%s/%s", REGISTER_PATH_PREFIX, delimited_prog_name, "presenter");
  g_free(delimited_prog_name);

  DBusGProxy* proxy = dbus_g_proxy_new_for_name(connection,
						DBUS_SERVICE_DBUS,
						DBUS_PATH_DBUS,
						DBUS_INTERFACE_DBUS);
  guint32 dbus_ret_val;
  if (!org_freedesktop_DBus_request_name(proxy, register_service,
					 0, &dbus_ret_val, &error)) {
    // problem in obtaining the service name - bail out
    g_critical("Failed to obtain name %s on session bus: %s", register_service, error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return return_val;
  }

  if (dbus_ret_val != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
    // the program is already running - return FALSE
    return return_val;
  }

  // first invocation of the program - this process now owns the
  // service name, so create a CguProgPresenter object for dbus
  presenter_instance = cgu_prog_presenter_new();
  cgu_prog_presenter_set(presenter_instance, func, object_data);
  dbus_g_connection_register_g_object(connection,
				      register_path, 
				      G_OBJECT(presenter_instance));

  // we don't need register_service or register_path any more as
  // this is the first invocation of the program and we will not
  // call present_instance()
  clean_up_dbus_variables();

  return_val = true;
  return return_val;
}

int present_instance(const char** instance_args) {

  if (!register_service) {
    g_warning("Error in present_instance(): either register_prog() has not been called,\n"
	      "there was an error in obtaining a connection or service name in register_prog(),\n"
	      "present_instance() has been called in the first invocation of the program\n"
	      "or present_instance() has already been called once before in this invocation of\n"
              "the program.  Ignoring this call() to present_instance\n");
    return 1;
  }
  
  GError* error = 0;

  // use the session bus
  DBusGConnection* connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
  if (!connection) {
    // problem in obtaining a connection - bail out
    g_critical("Failed to open connection to bus: %s\n", error->message);
    g_error_free(error);
    clean_up_dbus_variables();
    return 1;
  }

  // we have a connection on the session bus
  DBusGProxy* proxy = dbus_g_proxy_new_for_name(connection,
						register_service,
						register_path,
						CGU_PROG_PRESENTER_DBUS_INTERFACE);


  /* org_cgu_CguProgPresenterInterface_present() is defined in
     prog-presenter-bindings.h and represents the client proxy version
     of the Present method for interface CguProgPresenterInterface.
     It is a blocking call which will wait for the callback
     (cgu_prog_presenter_present()) to complete and then report the
     outcome */
  if (!org_cgu_CguProgPresenterInterface_present(proxy, instance_args, &error)) {
    int return_val;
    g_critical("Failed to present instance: %s", error->message);
    if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION) {
      g_critical("Dbus error name is %s", dbus_g_error_get_name(error));
      return_val = 2;
    }
    else
      return_val = 1;

    g_error_free(error);
    clean_up_dbus_variables();
    return return_val;
  }
    
  clean_up_dbus_variables();
  return 0;
}

} // namespace Cgu

#endif /* HAVE_GIO_DBUS || HAVE_DBUS_GLIB */
