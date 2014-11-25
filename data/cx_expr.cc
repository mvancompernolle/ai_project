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

#include <more/lang/cx_expr.h>
#include <sstream>
#include <stdarg.h>

namespace more {
namespace lang {

  //--------------------------------------------------------------------
  //			       cx_expr
  //--------------------------------------------------------------------

  cx_expr::~cx_expr() {}

  void
  cx_expr::print_as_expr(std::ostream& os) const
  {
      throw std::logic_error("more::lang::cx_expr::print_source: "
			     "This expression can not print it's source code "
			     "directly.");
  }

  void
  cx_expr::print_as_stmt(std::ostream& os) const
  {
      print_as_expr(os);
      os << ";\n";
  }


  //--------------------------------------------------------------------
  //			       cx_block
  //--------------------------------------------------------------------

  cx_identifier*
  cx_block::declare(ct_type const* rtt)
  {
#ifndef MORE_LANG_NDEBUG
      if (rtt == lang::ct_type_of<void>())
	  throw std::invalid_argument("more::lang::cx_block::declare: "
				      "Can not declare a variable of type "
				      "void.");
#endif
      cx_identifier* var = new(UseGC) cx_identifier(rtt);
      m_vars.push_back(var);
      return var;
  }

  cx_identifier*
  cx_block::append_definition(cx_expr* e)
  {
      cx_identifier* var = declare(e->type_of());
      m_vars.push_back(var);
      append_stmt(make_assignment(var, e));
      return var;
  }

  void
  cx_block::append_stmt(cx_expr* e)
  {
      e->append_ctors_to(this);
      m_stmts.push_back(e);
      e->append_dtors_to(this);
  }

  void
  cx_block::prepend_stmt(cx_expr* e)
  {
#ifndef MORE_LANG_NDEBUG
      cx_block* blk = new(UseGC) cx_block;
      e->append_ctors_to(blk);
      e->append_dtors_to(blk);
      if (!blk->empty())
	  throw std::logic_error("more::lang::cx_block::prepend_stmt: "
				 "Ctors and dtors are not supported here.");
#endif
      m_stmts.push_front(e);
  }

  void
  cx_block::append_stmts_from(cx_block const* blk)
  {
      std::copy(blk->m_stmts.begin(), blk->m_stmts.end(),
		std::back_inserter(m_stmts));
  }

  void
  cx_block::append_decls_from(cx_block const* blk)
  {
      std::copy(blk->m_vars.begin(), blk->m_vars.end(),
		std::back_inserter(m_vars));
  }

  void
  cx_block::print_as_expr(std::ostream& os) const
  {
      throw std::logic_error("more::lang::cx_block::print_source: "
			     "Can not print a block as an expression.");
  }

  void
  cx_block::print_as_stmt_seq(std::ostream& os) const
  {
      for (var_container::const_iterator it = m_vars.begin();
	   it != m_vars.end(); ++it) {
	  ct_type const* rtt = (*it)->type_of();
	  rtt->print_declaration_pre(os, ct_type::printopt_space_after_name);
	  (*it)->print_as_expr(os);
	  rtt->print_declaration_post(os, 0);
	  os << ";\n";
      }
      for (stmt_container::const_iterator it = m_stmts.begin();
	   it != m_stmts.end(); ++it) {
	  (*it)->print_as_expr(os);
	  os << ";\n";
      }
  }

  void
  cx_block::print_as_stmt(std::ostream& os) const
  {
      os << "{\n";
      print_as_stmt_seq(os);
      os << "}\n";
  }

  void cx_block::append_ctors_to(cx_block* blk) {}
  void cx_block::append_dtors_to(cx_block* blk) {}

  //--------------------------------------------------------------------
  //			cx_expr_with_ctor_dtor
  //--------------------------------------------------------------------

  void
  cx_expr_with_ctor_dtor::append_ctors_to(cx_block* blk)
  {
      if (m_ctor_dtor) {
	  blk->append_decls_from(&m_ctor_dtor->m_ctors);
	  blk->append_decls_from(&m_ctor_dtor->m_dtors);
	  blk->append_stmts_from(&m_ctor_dtor->m_ctors);
      }
  }

  void
  cx_expr_with_ctor_dtor::append_dtors_to(cx_block* blk)
  {
      if (m_ctor_dtor)
	  blk->append_stmts_from(&m_ctor_dtor->m_dtors);
  }

  cx_identifier*
  cx_expr_with_ctor_dtor::ctor_declare(ct_type const* t)
  {
      if (m_ctor_dtor == 0)
	  m_ctor_dtor = new(UseGC) ctor_dtor_type;
      return m_ctor_dtor->m_ctors.declare(t);
  }

  void
  cx_expr_with_ctor_dtor::ctor_append_stmt(cx_expr* e)
  {
      if (m_ctor_dtor == 0)
	  m_ctor_dtor = new(UseGC) ctor_dtor_type;
      m_ctor_dtor->m_ctors.append_stmt(e);
  }

  void
  cx_expr_with_ctor_dtor::dtor_append_stmt(cx_expr* e)
  {
      if (m_ctor_dtor == 0)
	  m_ctor_dtor = new(UseGC) ctor_dtor_type;
      m_ctor_dtor->m_dtors.append_stmt(e);
  }


  //--------------------------------------------------------------------
  //			     cx_identifier
  //--------------------------------------------------------------------

  cx_identifier::~cx_identifier() {}

  void
  cx_identifier::print_as_expr(std::ostream& os) const
  {
      if (m_id.is_defined())
	  os << m_id;
      else {
	  lang::uintptr_t i = (lang::intptr_t)this;
	  os << "_x";
	  while (i) {
	      unsigned int j = i % 62;
	      i /= 62;
	      if (j < 10)
		  os << j;
	      else if (j < 36)
		  os << char(j - 10 + 'a');
	      else
		  os << char(j - 36 + 'A');
	  }
      }
  }

//   void
//   cx_identifier::print_definition(std::ostream&) const
//   {
//       throw std::logic_error("more::lang::cx_identifier::print_definition: "
// 			     "This variable is not part of a definition.");
//   }

  //--------------------------------------------------------------------
  //			      cx_literal
  //--------------------------------------------------------------------

  inline void
  putc_esc(std::ostream& os, char ch)
  {
      if (std::isprint(ch))
	  os << ch;
      else
	  switch (ch) {
	    case '"': os << "\\\""; break;
	    case '\'': os << "\\\'"; break;
	    case '\\': os << "\\\\"; break;
	    case '\n': os << "\\n"; break;
	    case '\t': os << "\\t"; break;
	    case '\v': os << "\\v"; break;
	    case '\b': os << "\\b"; break;
	    default:
	      os << "\\";
	      os.setf(std::ios_base::oct, std::ios_base::basefield);
	      os.width(3);
	      os.fill('0');
	      os << (int)ch;
	      os.fill(' ');
	      os.setf(std::ios_base::dec, std::ios_base::basefield);
	      break;
	  }
  }

  static void c_print(std::ostream& os, long double x) { os << x; }
  static void c_print(std::ostream& os, long x) { os << x << "L"; }
  static void c_print(std::ostream& os, unsigned long x) { os << x << "UL"; }
  static void c_print(std::ostream& os, int x) { os << x; }
  static void c_print(std::ostream& os, unsigned int x) { os << x << "U"; }
#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
  static void c_print(std::ostream& os, long long x) { os << x << "LL"; }
  static void c_print(std::ostream& os, unsigned long long x) {os<<x<<"ULL";}
#endif

  static void
  c_print(std::ostream& os, char x)
  {
      os << '\'';
      putc_esc(os, x);
      os << '\'';
  }

  static void
  c_print(std::ostream& os, char const* str)
  {
      os << '\"';
      while (*str)
	  putc_esc(os, *str++);
      os << '\"';
  }

  template<typename T>
    void
    cx_literal<T>::print_as_expr(std::ostream& os) const
    {
	c_print(os, m_value);
    }

  template<typename T>
    void cx_literal<T>::append_ctors_to(cx_block* blk) {}
  template<typename T>
    void cx_literal<T>::append_dtors_to(cx_block* blk) {}



  //--------------------------------------------------------------------
  //			       cx_call
  //--------------------------------------------------------------------

  cx_call::cx_call(cx_expr* fn, va_list va)
      : cx_expr(static_cast<ct_proto const*>(fn->type_of())->result_type()),
	m_fn(fn)
  {
#ifndef MORE_LANG_NDEBUG
      if (!dynamic_cast<ct_proto const*>(fn->type_of()))
	  throw std::invalid_argument("more::lang::cx_call: "
				      "Argument 0 is not a function.");
#endif
      ct_proto const* proto = static_cast<ct_proto const*>(fn->type_of());
      int n = proto->arg_count();
      m_args = new(UseGC) cx_expr*[n];
      for (int i = 0; i < n; ++i) {
	  cx_expr* x = va_arg(va, cx_expr*);
#ifndef MORE_LANG_NDEBUG
	  if (x == 0)
	      throw std::logic_error("more::lang::cx_call::cx_call: "
				     "Too few arguments.");
#endif
	  m_args[i] = x;
      }
#ifndef MORE_LANG_NDEBUG
      if (va_arg(va, cx_expr*) != 0)
	  throw std::logic_error("more::lang::cx_call::cx_call: "
				 "Too many arguments.");
#endif
  }

  cx_call::cx_call(cx_expr* fn, cx_expr** args)
      : cx_expr(static_cast<ct_proto const*>(fn->type_of())->result_type()),
	m_fn(fn),
	m_args(args)
  {
      if (dynamic_cast<ct_pointer const*>(m_fn->type_of()))
	  m_fn = make_deref_expr(m_fn);
#ifndef MORE_LANG_NDEBUG
      if (!dynamic_cast<ct_proto const*>(m_fn->type_of()))
	  throw std::invalid_argument("more::lang::cx_call::cx_call: "
				      "First argument is neither a function "
				      "nor a pointer to a function.");
#endif
  }


  void
  cx_call::print_as_expr(std::ostream& os) const
  {
      m_fn->print_as_expr(os);
      os << '(';
      int n = static_cast<ct_proto const*>(m_fn->type_of())->arg_count();
      if (n != 0) {
	  m_args[0]->print_as_expr(os);
	  for (int i = 1; i < n; ++i) {
	      os << ',';
	      m_args[i]->print_as_expr(os);
	  }
      }
      os << ')';
  }

  void
  cx_call::append_ctors_to(cx_block* blk)
  {
      size_t n = static_cast<ct_proto const*>(m_fn->type_of())->arg_count();
      for (size_t i = 0; i < n; ++i)
	  m_args[i]->append_ctors_to(blk);
  }

  void
  cx_call::append_dtors_to(cx_block* blk)
  {
      size_t n = static_cast<ct_proto const*>(m_fn->type_of())->arg_count();
      for (size_t i = n; i != 0;) {
	  --i;
	  m_args[i]->append_dtors_to(blk);
      }
  }

  cx_call*
  make_call(cx_expr* fn, ...)
  {
      va_list va;
      va_start(va, fn);
      cx_call* call = new(UseGC) cx_call(fn, va);
      va_end(va);
      return call;
  }



  //--------------------------------------------------------------------
  //			      cx_switch
  //--------------------------------------------------------------------

  void
  cx_switch::print_as_stmt(std::ostream& os) const
  {
      m_cond->print_as_expr(os);
      cx_block::print_as_expr(os);
  }

  void
  cx_switch::append_ctors_to(cx_block* blk)
  {
      m_cond->append_ctors_to(blk);
  }

  void
  cx_switch::append_dtors_to(cx_block* blk)
  {
      m_cond->append_dtors_to(blk);
  }


  //--------------------------------------------------------------------
  //			     cx_lambda
  //--------------------------------------------------------------------

  cx_lambda::cx_lambda(ct_proto const* proto)
      : cx_expr(proto),
	m_args(new(UseGC) cx_identifier*[proto->arg_count()])
  {
      size_t i = 0;
      for (ct_proto::arg_type_iterator it = proto->arg_type_begin();
	   it != proto->arg_type_end(); ++it)
	  m_args[i++] = new(UseGC) cx_identifier(*it);
  }

  cx_identifier*
  cx_lambda::arg(size_t i)
  {
#ifndef MORE_LANG_NDEBUG
      if (i >= arg_count())
	  throw std::out_of_range("more::lang::rt_lambda: "
				  "Queried non-existing argument.");
#endif
      return m_args[i];
  }

  void cx_lambda::append_ctors_to(cx_block* blk) {}
  void cx_lambda::append_dtors_to(cx_block* blk) {}

  void
  cx_lambda::print_as_expr(std::ostream& os) const
  {
      throw std::logic_error("more::lang::cx_lambda::print_as_expr: "
			     "Lambda expressions can not be printed as "
			     "C code. (Bind it to an identifier.)");
  }


  //--------------------------------------------------------------------
  //			       cx_cond
  //--------------------------------------------------------------------

  void
  cx_cond::append(cx_expr* cond, cx_expr* conseq)
  {
      if (cond) {
#ifndef MORE_LANG_NDEBUG
	  if (m_counter_conseq)
	      throw std::logic_error("more::lang::cx_cond: "
				     "Counter-cosequence precedes a "
				     "(condition, consequence) pair.");
#endif
	  cond_conseq cc;
	  cc.m_cond = cond;
	  cc.m_conseq = conseq;
	  if (!m_cases.empty()) {
	      cc.m_cond_ctors = new(UseGC) cx_block;
	      cc.m_cond_dtors = new(UseGC) cx_block;
	      cond->append_ctors_to(cc.m_cond_ctors);
	      cond->append_dtors_to(cc.m_cond_dtors);
	      if (cc.m_cond_ctors->empty())
		  cc.m_cond_ctors = 0;
	      if (cc.m_cond_dtors->empty())
		  cc.m_cond_dtors = 0;
	  }
	  else {
	      cc.m_cond_ctors = 0;
	      cc.m_cond_dtors = 0;
	  }
	  m_cases.push_back(cc);
      }
      else
	  m_counter_conseq = conseq;
  }

  void
  cx_cond::print_as_expr(std::ostream& os) const
  {
      // XXX Well, we could use ...? ... : ... or generate a tmp var.
      throw std::logic_error("more::lang::cx_cond::print_source: "
			     "Can not print if/then as an expression.");
  }

  void
  cx_cond::print_as_stmt(std::ostream& os) const
  {
      if (!m_cases.empty()) {
	  case_container::const_iterator it = m_cases.begin();
	  os << "if (";
	  it->m_cond->print_as_expr(os);
	  os << ")\n";
	  bool is_cond = dynamic_cast<cx_cond*>(it->m_conseq) != 0;
	  if (is_cond)
	      os << "{\n";
	  it->m_conseq->print_as_stmt(os);
	  if (is_cond)
	      os << "}\n";
	  while (++it != m_cases.end()) {
	      if (it->m_cond_ctors || it->m_cond_dtors) {
		  os << "else {\n";
		  if (it->m_cond_ctors)
		      it->m_cond_ctors->print_as_stmt_seq(os);
		  os << "if (";
		  it->m_cond->print_as_expr(os);
		  os << ")\n";
		  bool is_cond = dynamic_cast<cx_cond*>(it->m_conseq) != 0;
		  if (is_cond)
		      os << "{\n";
		  it->m_conseq->print_as_stmt(os);
		  if (is_cond)
		      os << "}\n";
		  if (it->m_cond_dtors)
		      it->m_cond_dtors->print_as_stmt_seq(os);
		  os << "}\n";
	      }
	      else {
		  os << "else if (";
		  it->m_cond->print_as_expr(os);
		  os << ")\n";
		  bool is_cond = dynamic_cast<cx_cond*>(it->m_conseq) != 0;
		  if (is_cond)
		      os << "{\n";
		  it->m_conseq->print_as_stmt(os);
		  if (is_cond)
		      os << "}\n";
	      }
	  }
      }
      if (m_counter_conseq) {
	  os << "else\n";
	  // If the conter-conseq is an cx_cond, it will correctly
	  // continue as an else if.
	  m_counter_conseq->print_as_stmt(os);
      }
  }

  void
  cx_cond::append_ctors_to(cx_block* blk)
  {
      if (!m_cases.empty())
	  m_cases.front().m_cond->append_ctors_to(blk);
  }

  void
  cx_cond::append_dtors_to(cx_block* blk)
  {
      if (!m_cases.empty())
	  m_cases.front().m_cond->append_dtors_to(blk);
  }


  //--------------------------------------------------------------------
  //			      cx_pattern
  //--------------------------------------------------------------------

  cx_pattern::cx_pattern(char const* pattern, ct_type const* rtt_res,
			 va_list va)
      : cx_expr(rtt_res),
	m_pattern(pattern)
  {
#ifndef MORE_LANG_NDEBUG
      if (!rtt_res)
	  throw std::invalid_argument("more::lang::cx_pattern::cx_pattern: "
				      "Result type is unspecified.");
#endif
      int arity = 0;
      while (*pattern) {
	  if (pattern[0] == '%' && pattern[1] == '!') {
	      ++arity;
	      pattern += 2;
	  }
	  else
	      ++pattern;
      }
      m_arity = arity;
      m_args = new(UseGC) cx_expr*[arity];
      for (int i = 0; i < arity; ++i)
	  m_args[i] = va_arg(va, cx_expr*);
  }

  void
  cx_pattern::print_as_expr(std::ostream& os) const
  {
      char const* s = m_pattern;
      int i = 0;
      while (*s) {
	  if (s[0] == '%' && s[1] == '!') {
	      m_args[i++]->print_as_expr(os);
	      s += 2;
	  }
	  else {
	      os << *s;
	      ++s;
	  }
      }
  }

  void
  cx_pattern::append_ctors_to(cx_block* blk)
  {
      for (int i = 0; i < m_arity; ++i)
	  m_args[i]->append_ctors_to(blk);
  }

  void
  cx_pattern::append_dtors_to(cx_block* blk)
  {
      for (int i = m_arity; i > 0;)
	  m_args[--i]->append_dtors_to(blk);
  }


  //--------------------------------------------------------------------
  //				cx_and
  //--------------------------------------------------------------------

  cx_and::cx_and(size_t n_arg, cx_expr** args)
      : cx_expr(ct_type_of<int>()),
	m_n(n_arg),
	m_argv(args)
  {
      if (!n_arg)
	  throw std::out_of_range("more::lang::cx_and::"
				  "cx_and(size_t, cx_expr**): "
				  "Need at least one argument.");
  }

  void
  cx_and::print_as_expr(std::ostream& os) const
  {
      os << '(';
      m_argv[0]->print_as_expr(os);
      for (size_t i = 1; i < m_n; ++i) {
	  os << "&&";
	  m_argv[i]->print_as_expr(os);
      }
      os << ')';
  }

  void
  cx_and::append_ctors_to(cx_block* blk)
  {
      m_argv[0]->append_ctors_to(blk);
      assert(!"Not implemented.");
  }

  void
  cx_and::append_dtors_to(cx_block* blk)
  {
      assert(!"Not implemented.");
      m_argv[0]->append_dtors_to(blk);
  }


  //--------------------------------------------------------------------
  //			Convenience Functions
  //--------------------------------------------------------------------

  cx_expr*
  make_stmt(char const* pattern, ...)
  {
      va_list va;
      va_start(va, pattern);
      cx_expr* res = new(UseGC) cx_pattern(pattern, ct_type_of<void>(), va);
      va_end(va);
      return res;
  }

  cx_expr*
  make_expr(char const* pattern, ct_type const* rtt_res, ...)
  {
      va_list va;
      va_start(va, rtt_res);
      cx_expr* res = new(UseGC) cx_pattern(pattern, rtt_res, va);
      va_end(va);
      return res;
  }

  cx_expr*
  make_deref_expr(cx_expr* x)
  {
#ifndef MORE_LANG_NDEBUG
      if (!dynamic_cast<ct_pointer const*>(x->type_of()))
	  throw std::invalid_argument("more::lang::make_deref_expr: "
				      "Code fragment does not evaluate to "
				      "a pointer.");
#endif
      return make_expr("(*%!)",
		       static_cast<ct_pointer const*>(
			   x->type_of())->deref_type(), x);
  }

  cx_expr*
  make_element_expr(cx_expr* x, cx_expr* y)
  {
#ifndef MORE_LANG_NDEBUG
      if (!dynamic_cast<ct_pointer const*>(x->type_of()))
	  throw std::invalid_argument("more::lang::make_element_expr: "
				      "Arg 0 is not an array.");
#endif
      return make_expr("%![%!]",
		       static_cast<ct_array const*>(
			   x->type_of())->element_type(), x, y);
  }

  cx_expr*
  make_cast(ct_type const* t, cx_expr* x)
  {
      std::ostringstream oss;
      oss << "((";
      t->print_declaration(oss, 0);
      oss << ")%!)";
      gc_string str(oss.str().c_str());
      return make_expr(str.c_str(), t, x);
  }

  cx_expr*
  make_not(cx_expr* x)
  {
      return make_expr("(!%!)", ct_type_of<int>(), x);
  }

  cx_expr*
  make_equal(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!==%!)", ct_type_of<int>(), x, y);
  }

  cx_expr*
  make_less(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!<%!)", ct_type_of<int>(), x, y);
  }

  cx_expr*
  make_lesseq(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!<=%!)", ct_type_of<int>(), x, y);
  }

  cx_expr*
  make_assignment(cx_expr* dst, cx_expr* src)
  {
      return make_expr("(%!=%!)", dst->type_of(), dst, src);
  }


  // Arithmetic

  cx_expr*
  make_negation(cx_expr* x)
  {
      // Space avoids composition to --
      return make_expr("(- %!)", x->type_of(), x);
  }

  cx_expr*
  make_sum(cx_expr* x, cx_expr* y)
  {
      // Space avoids composition to ++
      return make_expr("(%! + %!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_difference(cx_expr* x, cx_expr* y)
  {
      // Space avoids composition to --
      return make_expr("(%! - %!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_product(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!*%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_quotient(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!/%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_modulus(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!%%%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }


  // Bitwise

  cx_expr*
  make_bitnot(cx_expr* x)
  {
      return make_expr("(~%!)", x->type_of(), x);
  }

  cx_expr*
  make_bitand(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!&%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_bitor(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!|%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_bitxor(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!^%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_bitlsh(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!<<%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }

  cx_expr*
  make_bitrsh(cx_expr* x, cx_expr* y)
  {
      return make_expr("(%!>>%!)",
		       arithmetic_result_type(x->type_of(), y->type_of()),
		       x, y);
  }


  // Control

  cx_expr*
  make_return(cx_expr* x)
  {
      return make_stmt("return %!", x);
  }

  cx_expr*
  make_return()
  {
      static cx_expr* e = make_stmt("return");
      return e;
  }

  cx_expr*
  make_break()
  {
      static cx_expr* e = make_stmt("break");
      return e;
  }

  cx_expr*
  make_do_while(cx_expr* blk, cx_expr* cond)
  {
      return make_stmt("do\n%!\nwhile(%!)", blk, cond);
  }

  cx_expr*
  make_while(cx_expr* cond, cx_expr* blk)
  {
      return make_stmt("while(%!)\n%!\n", cond, blk);
  }

}}
