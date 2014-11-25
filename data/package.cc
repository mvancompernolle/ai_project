//  Copyright (C) 2001--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <more/lang/package.h>
#include <more/io/filesys.h>
#include <more/lang/compilation.h>
#include <stack>


namespace more {
namespace lang {

  namespace {
    bool const opt_debug = true;
    bool const opt_transparent_c_names = true;
  }


  //--------------------------------------------------------------------
  //			   cx_package_variable
  //--------------------------------------------------------------------

  // IMPL NOTE. A dynamic (re)linking must update all its
  // cx_package_variable nodes accourding to dlsym.  Values which are
  // already bound must be copied to the new location.  This requires
  // that a copy constructor is available.
  //
  // WETHER TO COPY THE VALUE depends on the kind of data.  If the
  // value is constant it should not be copied.  In particular, if it
  // is a function, it can not be copied.

#define DEFAULTS				\
	m_old_ptr(0),				\
	m_client(0),				\
	m_version(0),				\
	m_has_version(0),			\
	m_is_dlsym(0),				\
	m_export_p(0)

  cx_package_variable::cx_package_variable(cx_expr* val)
      : cx_identifier(val->type_of()),
	m_val(val),
	m_pkg(0),
	DEFAULTS {}

  cx_package_variable::cx_package_variable(ct_type const* t)
      : cx_identifier(t),
	m_val(0),
	m_pkg(0),
	DEFAULTS {}

#undef DEFAULTS

  ct_type const*
  cx_package_variable::resolve_type_of() const
  {
      return type_of();
  }

  void
  cx_package_variable::print_definition(std::ostream& os)
  {
      if (!definiens())
	  return; // XXX or throw
      type_of()->print_declaration_pre(os, ct_type::printopt_space_after_name);
      cx_identifier::print_as_expr(os);
      if (cx_lambda* f = dynamic_cast<cx_lambda*>(m_val)) {
	  f->type_of()->print_declaration_post(
	      os, ct_type::printopt_space_before_name,
	      f->arg_begin());
	  f->body()->print_as_stmt(os);
      }
      else {
	  type_of()->print_declaration_post(
	      os, ct_type::printopt_space_before_name);
	  os << " = ";
	  m_val->print_as_expr(os);
	  os << ";\n";
      }
  }

  cx_expr*
  cx_package_variable::make_definiens()
  {
      throw std::logic_error("more::lang::cx_package_variable: "
			     "There is no way to obtain the definition of "
			     + std::string(name().c_str()) + ".");
  }

  cx_package_variable::~cx_package_variable()
  {
#ifndef MORE_LANG_NDEBUG
      if (m_pkg) {
	  std::cerr << "more::lang::cx_package_variable::~cx_package_variable: "
		    << "This object should be erased from its package "
		    << "before destruction! (Use GC.)" << std::endl;
	  abort();
      }
#endif
  }

  void
  cx_package_variable::append_ctors_to(cx_block* blk)
  {
      definiens()->append_ctors_to(blk);
      cx_identifier::append_ctors_to(blk);
  }

  void
  cx_package_variable::append_dtors_to(cx_block* blk)
  {
      cx_identifier::append_dtors_to(blk);
      definiens()->append_dtors_to(blk);
  }

  void*
  cx_package_variable::resolve()
  {
      if (m_pkg) {
	  assert(!client()->get_if_ready());
	  void* ptr = m_pkg->resolve_no_prefix(name().c_str());
	  if (!ptr)
	      throw std::runtime_error("more::lang::cx_package_variable::"
				       "resolve: "
				       "Undefined dlsym \""
				       + std::string(name().c_str())
				       + "\".");
// 				       + std::string(lt_dlerror()));
	  client()->set(ptr);
	  m_is_dlsym = 1;
	  if (m_old_ptr) {
	      type_of()->construct_copy(ptr, m_old_ptr);
	      m_old_ptr = 0;
	  }
	  return ptr;
      }
      else
	  throw std::logic_error("more::lang::cx_package_variable::resolve: "
				 "Variable is member of a package.");
  }

  bool
  cx_package_variable::equal_to(resolver* rhs)
  {
      return this == rhs;
  }



  //--------------------------------------------------------------------
  //			      package
  //--------------------------------------------------------------------

  package::package(package* cxt, identifier name, flag_type fl, flag_type fm)
      : m_context(cxt),
	m_c_name(name),
	m_dlhandle(0),
	m_finalizer(0),
	m_extern_p(0),
	m_dropped_dep(0),
	m_dl_not_closed(0),
	m_flags((flags_intern_default & ~fm) | fl)
  {
      std::string dir = top_builddir();
      if (m_context) {
	  dir += '/';
	  dir += m_context->mk_path("/").c_str();
      }
      if (fl & flag_split) {
	  io::create_dir_rec(dir + "/" + m_c_name.c_str());
	  io::ifsyncstream isync((dir + '/'
				  + name.c_str() + "/init.bin").c_str());
	  if (isync.good()) {
	      io::sync_associative_container(isync, m_inits);
	      io::sync_associative_container(isync, m_finalizers);
	  }
      }
      else
	  io::create_dir_rec(dir);
      m_comp = new(UseGC) compilation(dir, dir);
      m_comp->set_target_library(name.c_str(), true);
      std::string rel_top_blddir;
      for (package* pkg = cxt; pkg != 0; pkg = pkg->m_context)
	  rel_top_blddir += "../";
      m_comp->append_cppflags("-I"+
			      io::directory_as_file_name(rel_top_blddir));
  }

  package::package(package* cxt, identifier name, lt_dlhandle h,
		   flag_type fl_set, flag_type fl_clear)
      : m_context(cxt),
	m_c_name(name),
	m_comp(0),
	m_dlhandle(h),
	m_finalizer(0),
	m_extern_p(1),
	m_dropped_dep(0),
	m_dl_not_closed(0),
	m_flags((flags_extern_default & ~fl_clear) | fl_set)
  {
      if (flags() & flag_compile)
	  throw std::logic_error("more::lang::package::compile: "
				 "flag_compile is invalid for external "
				 "package.");
      if (!m_dlhandle)
	  throw std::invalid_argument("more::lang::package::package: "
				      "Zero lt_dlhandle.");
      if (opt_debug)
	  std::clog << "package::package: Constructed with #[dlhandle @ "
		    << (void*)m_dlhandle << "]" << std::endl;
  }

  package::package(package* cxt, identifier name, char const* file,
		   flag_type fl_set, flag_type fl_clear)
      : m_context(cxt),
	m_c_name(name),
	m_comp(0),
	m_dlhandle(lt_dlopenext(file)),
	m_finalizer(0),
	m_extern_p(1),
	m_dropped_dep(0),
	m_dl_not_closed(0),
	m_flags((flags_extern_default & ~fl_clear) | fl_set)
  {
      if (flags() & flag_compile)
	  throw std::logic_error("more::lang::package::compile: "
				 "flag_compile is invalid for external "
				 "package.");
      if (!m_dlhandle)
	  throw std::runtime_error("more::lang::package::package: "
				   "Could not open library "
				   + std::string(file) + ".");
      if (opt_debug)
	  std::clog << "package::package: Opened #[dlhandle @ "
		    << (void*)m_dlhandle << "]" << std::endl;
  }


  package::~package()
  {
      // Remove dependencies of this package.
      for (requires_iterator it = m_requires.begin();
	   it != m_requires.end(); ++it)
	  (*it)->m_required_by.erase(this);

      // If the GC phases out two packagens in the same collection,
      // and especially if there are mutual dependencies, we may be
      // clean up before packages which depends on this one.  So, also
      // remove dependencies of other packages on this package.
      for (required_by_iterator it = m_required_by.begin();
	   it != m_required_by.end(); ++it) {
	  (*it)->m_requires.erase(this);
	  (*it)->m_dropped_dep = 1;
      }

      if (m_dlhandle) {
	  if (m_finalizer)
	      m_finalizer();
	  if (opt_debug)
	      std::clog << "package::~package: Closing #[dlhandle @ "
			<< (void*)m_dlhandle << "]" << std::endl;
	  lt_dlclose(m_dlhandle);
      }
  }

  void
  package::define(identifier idr_last, cx_package_variable* h, bool export_p)
  {
      if (h->m_pkg != 0)
	  throw std::logic_error("more::lang::package::define: "
				 "This dlsym_hander is already bound to "
				 "a package.");
      identifier idr = mk_c_name(idr_last.c_str()).c_str();
      h->set_name(idr);
      h->m_pkg = this;
      h->m_export_p = export_p;
      m_defs[idr_last] = h;
  }

  void
  package::define(identifier idr, location& locn, bool export_p)
  {
      cx_package_variable* h
	  = dynamic_cast<cx_package_variable*>(locn.get_resolver());
      if (!h)
	  throw std::invalid_argument("more::lang::package::define: "
				      "Location does not have a "
				      "cx_package_variable.");
      define(idr, h, export_p);
  }

  void
  package::undefine(identifier idr)
  {
      definition_iterator it = m_defs.find(idr);
      if (it != m_defs.end()) {
	  m_defs.erase(it);
	  it->second->m_pkg = 0;
      }
  }

  void
  package::define_type(identifier idr, ct_struct* sct, bool export_p)
  {
      gen::gc_string c_name = mk_c_name(idr.c_str());
      sct->set_c_name(c_name.c_str());
      m_structs.insert(sct);
  }

  gen::gc_string
  package::mk_path(char const* infix) const
  {
      if (m_context) {
	  assert(m_c_name.c_str());
	  gen::gc_string name = m_context->mk_path(infix);
	  if (name.empty())
	      return m_c_name.c_str();
	  else
	      return name + infix + m_c_name.c_str();
      }
      else if (m_c_name.c_str())
	  return m_c_name.c_str();
      else
	  return "";
  }

  gen::gc_string
  package::mk_c_name(gen::gc_string name) const
  {
      gen::gc_string str;
      if (use_cxx_symbols())
	  str = mk_path("::");
      else if (opt_transparent_c_names)
	  str = mk_path("_");
      else
	  assert(!"Missing implementation.");
      str += '_';
      str += name;
      return str;
  }

  // Called if m_dlhandle is 0 or if lt_dlsym failed.  sym is a full
  // symbol name or 0.
  void*
  package::link(char const* sym)
  {
      if (lt_dlinit() != 0)
	  throw std::runtime_error("more::lang::package: "
				   "lt_dlinit failed: " +
				   std::string(lt_dlerror()));
      if (flags() & flag_compile)
	  compile();

      if (m_dlhandle == 0) {
	  assert(m_comp);
	  std::string ltname
	      = io::resolve_file_name(m_comp->library_file_name(),
				      m_comp->build_dir());
	  if (opt_debug)
	      std::clog << "Trying to open " << ltname << '.' << std::endl;
	  if (m_dl_not_closed)
	      std::cerr << "** warning: Last lt_dlclose failed for "
			<< ltname << ".  This may cause trouble.\n";
	  m_dlhandle = lt_dlopen(ltname.c_str());
	  if (m_dlhandle == 0)
	      throw std::runtime_error("more::lang::package: "
				       "Could not link in library: " +
				       std::string(lt_dlerror()));
	  if (opt_debug)
	      std::clog << "package::link: Opened #[dlhandle @ "
			<< (void*)m_dlhandle << "]\n"
			<< "Getting symbol " << m_name_fin << '.'
			<< std::endl;
	  m_finalizer
	      = (void (*)())lt_dlsym(m_dlhandle, m_name_fin.c_str());
	  if (opt_debug)
	      std::clog << "Getting symbol " << m_name_init
			<< '.' << std::endl;
	  if (void* ptr = lt_dlsym(m_dlhandle, m_name_init.c_str())) {
	      if (opt_debug)
		  std::clog << "Initializing module " << m_c_name << '.'
			    << std::endl;
	      ((void (*)())ptr)();
	  }
      }

      if (sym)
	  return lt_dlsym(m_dlhandle, sym);
      else
	  return 0;
  }

  void
  package::unlink()
  {
      if (m_dlhandle == 0)
	  return;
      for (definition_iterator it_dfn = m_defs.begin();
	   it_dfn != m_defs.end(); ++it_dfn) {
	  cx_package_variable* val = it_dfn->second;
	  if (val->m_is_dlsym) {
	      // If this is a non-const dlsym, copy it before we
	      // close the library, and schedule it for copying
	      // back into the new version of the library.
	      if (!val->type_of()->is_const()) {
		  val->m_old_ptr = GC_malloc(val->type_of()->size());
		  val->type_of()
		      ->construct_copy(val->m_old_ptr,
				       val->client()->get_if_ready());
	      }
	  }
	  else
	      // val is not yet a dlsym or has not yet been requested
	      val->m_old_ptr = val->client()->get_if_ready();
	  val->client()->set(0);
      }
      if (m_finalizer) {
	  if (opt_debug)
	      std::cerr << "Finalizing module " << m_c_name << '.'
			<< std::endl;
	  m_finalizer();
      }
      if (opt_debug)
	  std::clog << "package::unlink: Closing #[dlhandle @ "
		    << (void*)m_dlhandle
		    << "]." << std::endl;
      if (lt_dlclose(m_dlhandle) != 0)
	  m_dl_not_closed = 1;
      m_dlhandle = 0;
  }

  void
  package::create_sources()
  {
      bool const opt_cxx = (m_flags & flag_lang_mask) == flag_lang_cxx;
      bool const opt_cxx_symbols = m_flags & flag_cxx_symbols;
      lang_t const opt_lang = opt_cxx? lang_cxx : lang_c;
      if (m_extern_p)
	  throw std::logic_error("more::lang::package::create_sources(): "
				 "Can not create sources for an external "
				 "package.");
      // Don't mind if flag_compile is not set.  This function may be
      // called by client code independent of that flag.

      // Setup and start files.
      compilation& cc(*m_comp);
      std::string dir = cc.build_dir();
      std::string name_h;
      std::string name_c;
      name_h = cc.insert_header(m_c_name.c_str(), opt_lang);
      std::ofstream os_h(name_h.c_str());
      std::ofstream os_c;
      gc_string path_slash = mk_path("/");
      gc_string path_uscore = mk_path("_");
      if (!(flags() & flag_split)) {
	  name_c = cc.insert_source(m_c_name.c_str(), opt_lang);
	  os_c.open(name_c.c_str());
	  if (os_c.fail())
	      throw std::runtime_error(
		  "more::lang::package::create_sources:"
		  "Failed to create output source file " + name_c);
	  os_c << "#include <" << path_slash << ".h>\n";

	  // Add additional include's requested form make_definition
	  // calls.
	  std::set<std::string> incs;
	  for (definition_iterator it_def = m_defs.begin();
	       it_def != m_defs.end(); ++it_def) {
	      it_def->second->definiens();
	      for (std::set<std::string>::iterator it_inc
		       = it_def->second->m_includes.begin();
		   it_inc != it_def->second->m_includes.end(); ++it_inc)
		  if (incs.insert(*it_inc).second)
		      os_c << "#include <" << *it_inc << ">\n";
	  }

	  if (opt_cxx && !opt_cxx_symbols)
	      os_c << "extern \"C\" {\n";
      }

      os_h << "#ifndef INCLUDED_" << path_uscore << "_h\n";
      os_h << "#define INCLUDED_" << path_uscore << "_h\n";
      for (requires_iterator it = m_requires.begin(); it != m_requires.end();
	   ++it)
	  os_h << "#include <" << (*it)->mk_path("/") << ".h>\n";
      if (opt_cxx && !opt_cxx_symbols)
	  os_h << "extern \"C\" {\n";

      if (opt_debug)
	  std::clog << "Creating compilation for \""
		    << name_h << "\" and \"" << name_c << "\".\n";

      // Write prototypes.
      for (definition_iterator it = m_defs.begin();
	   it != m_defs.end(); ++it) {
	  ct_type const* rtt = it->second->type_of();
	  assert(rtt);
	  std::ostream* os;
	  if ((flags() & flag_split) || it->second->m_export_p) {
	      os_h << "extern ";
	      os = &os_h;
	  }
	  else {
	      os_c << "static ";
	      os = &os_c;
	  }
	  rtt->print_declaration_pre(*os, 0);
	  if (opt_cxx_symbols)
	      *os << it->first;
	  else
	      *os << it->second->name();
	  rtt->print_declaration_post(*os, 0);
	  *os << ";\n";
      }

      cx_block init;
      cx_block finalizer;
      bool init_changed_p = false; // used if split_p

      // Write function definitions.
      if (flags() & flag_split) {
	  // Then we are splitting definitions into separate
	  // files.
	  for (definition_iterator it = m_defs.begin();
	       it != m_defs.end(); ++it) {

	      if (it->second->m_has_version) {
		  name_c = cc.insert_source(std::string(m_c_name.c_str())
					    + "/" + it->first.c_str(),
					    opt_lang, it->second->m_version);
		  if (name_c.empty()) {
		      // No recompilation needed for this file.  Then
		      // add the file initializer and finalizer
		      // defined in that file, if any ...
		      if (m_inits.find(it->first) != m_inits.end())
			  init.append_stmt(
			      make_stmt(("initialize_" + path_uscore + "_"
					 + it->first.c_str() + "()").c_str()));
		      if (m_finalizers.find(it->first) != m_finalizers.end())
			  finalizer.prepend_stmt(
			      make_stmt(("finalize_" + path_uscore + "_"
					 + it->first.c_str() + "()").c_str()));

		      // ... and skip to the next definition.
		      continue;
		  }
	      }
	      else
		  name_c = cc.insert_source(std::string(m_c_name.c_str())
					    + "/" + it->first.c_str(),
					    opt_lang);
	      os_c.open(name_c.c_str());
	      if (os_c.fail())
		  throw std::runtime_error(
		      "more::lang::package::create_sources: "
		      "Failed to open output source file " + name_c);
	      os_c << "#include <" << path_slash << ".h>\n";
	      it->second->definiens();
	      for (std::set<std::string>::iterator it_inc
		       = it->second->m_includes.begin();
		   it_inc != it->second->m_includes.end(); ++it_inc)
		  os_c << "#include <" << *it_inc << ">\n";
	      if (opt_cxx && !opt_cxx_symbols)
		  os_c << "extern \"C\" {\n";
	      cx_block local_init;
	      cx_block local_finalizer;
	      it->second->append_ctors_to(&local_init);
	      it->second->append_dtors_to(&local_finalizer);
	      it->second->print_definition(os_c);

	      if (!local_init.empty()) {
		  gc_string local_init_name
		      = "initialize_" + path_uscore + "_" + it->first.c_str();
		  if (m_inits.insert(it->first).second)
		      init_changed_p = true;
		  init.append_stmt(make_stmt((local_init_name + "()").c_str()));
		  os_c << "void\n" << local_init_name << "(void)";
		  local_init.print_as_stmt(os_c);
		  os_c << "\n";
	      }
	      else if (m_inits.erase(it->first))
		  init_changed_p = true;

	      if (!local_finalizer.empty()) {
		  gc_string local_finalizer_name
		      = "finalize_" + path_uscore + "_" + it->first.c_str();
		  if (m_finalizers.insert(it->first).second)
		      init_changed_p = true;
		  finalizer.prepend_stmt(make_stmt((local_finalizer_name
						    + "()").c_str()));
		  os_c << "void\n" << local_finalizer_name << "(void)";
		  local_finalizer.print_as_stmt(os_c);
		  os_c << "\n";
	      }
	      else if (m_finalizers.erase(it->first))
		  init_changed_p = true;

	      if (opt_cxx && !opt_cxx_symbols)
		  os_c << "}\n";
	      os_c.close();
	  }
      }
      else {
	  for (definition_iterator it = m_defs.begin();
	       it != m_defs.end(); ++it) {
	      // Then we are putting all definitions into the same
	      // source file.

	      it->second->definiens();
	      it->second->append_ctors_to(&init);
	      it->second->append_dtors_to(&finalizer);
	      it->second->print_definition(os_c);
	      os_c << '\n';
	  }
      }

      // Write package initialization function.
      m_name_init = "initialize_" + path_uscore + "_module";
      m_name_fin = "finalize_" + path_uscore + "_module";

      if (flags() & flag_split) {
	  std::string name_init = std::string(m_c_name.c_str()) + "/init";
	  if (init_changed_p)
	      name_c = cc.insert_source(name_init, lang_c);
	  else
	      name_c = cc.insert_source(name_init, lang_c, 0);
	  if (!name_c.empty()) {
	      if (opt_debug)
		  std::clog << "Init file " << name_c << " is changed.\n";
	      os_c.open(name_c.c_str());
	      if (os_c.fail())
		  throw std::runtime_error(
		      "more::lang::package::create_sources: "
		      "Failed to open output source file " + name_c);
	  }
      }
      if (os_c.is_open()) {
	  os_c << "void\n" << m_name_fin << "(void)\n";
	  finalizer.print_as_stmt(os_c);
	  os_c << "void\n" << m_name_init << "(void)\n";
	  init.print_as_stmt(os_c);
	  if (flags() & flag_split) {
	      io::ofsyncstream osync((cc.build_dir() + '/' + m_c_name.c_str()
				      + "/init.bin").c_str());
	      io::sync_associative_container(osync, m_inits);
	      io::sync_associative_container(osync, m_finalizers);
	  }
      }
      os_h << "\n";
      if (opt_cxx && !opt_cxx_symbols) {
	  os_h << "}\n";
	  if (os_c.is_open() && !(flags() & flag_split))
	      os_c << "}\n";
      }

      os_h << "#endif\n";
      os_c.close();
      os_h.close();
  }

  void
  package::compile()
  {
      create_sources();
      if (!m_comp->make_all())
	  throw std::runtime_error("more::lang::package: "
				   "Compilation failed.");
      unlink();
  }

  package*
  bits_package()
  {
      static package* pkg = 0;
      if (!pkg)
	  pkg = new(UseGC) package(0, "bits");
      return pkg;
  }

  package*
  self_package()
  {
      static package* pkg = 0;
      if (!pkg) {
	  lt_dlhandle h = lt_dlopen(0);
	  if (!h)
	      throw std::runtime_error("more::lang::self_package: "
				       "The program can not dlopen itself. "
				       "Make sure to compile with "
				       "-export-dynamic or -dlopen self.");
	  pkg = new(UseGC) package(0, "", h);
      }
      return pkg;
  }
}}
