#include <more/lang/ct_type.h>
#include <more/lang/cx_expr.h>
#include <more/lang/package.h>
#include <more/diag/debug.h>

namespace lang = more::lang;

lang::ct_proto* ct_proto_int_int;
lang::ct_proto const* ct_proto_int_double_void;

struct test0_handler : lang::cx_package_variable
{
    test0_handler()
	: cx_package_variable(ct_proto_int_int) { set_version(0); }

    lang::cx_expr*
    make_definiens()
    {
	include("stdio.h");
	lang::cx_lambda* fn
	    = new(UseGC) lang::cx_lambda(ct_proto_int_int);
	lang::cx_expr* arg0 = fn->arg(0);
	ctor_append_stmt(
	    lang::make_stmt("printf(\"init test0_handler\\n\")"));
	fn->body()->append_stmt(lang::make_return(make_product(arg0, arg0)));
	return fn;
    }
};

struct test1_handler : lang::cx_package_variable
{
    test1_handler() : cx_package_variable(ct_proto_int_double_void) {}

    lang::cx_expr*
    make_definiens()
    {
	include("stdio.h");
	lang::cx_lambda* fn
	    = new(UseGC) lang::cx_lambda(ct_proto_int_double_void);
	fn->body()->append_stmt(
	    lang::make_stmt("printf(\"x=%d,y=%f\\n\", %!, %!)",
			    fn->arg(0), fn->arg(1)));
	ctor_append_stmt(
	    lang::make_stmt("printf(\"running test1 ctor\\n\");"));
	dtor_append_stmt(
	    lang::make_stmt("printf(\"running test1 dtor\\n\");"));
	return fn;
    }
};

int
main()
try {
    int arg_n = 767;
    double arg_f = 7.67;
    void* args[2] = { &arg_n, &arg_f };
    int res;

    ct_proto_int_int = new(UseGC)
	lang::ct_proto(1, lang::ct_type_of<int>());
    ct_proto_int_int->set_arg_type(0, lang::ct_type_of<int>());
    ct_proto_int_int->freeze();

    ct_proto_int_double_void
	= static_cast<lang::ct_proto const*>(
	    static_cast<lang::ct_pointer const*>(
		lang::ct_type_of<void (*)(int, double)>())->deref_type());

    for (int i_test = 0; i_test < 4; ++i_test) {
	static char const* pkgname[4] = {
	    "merged", "split", "merged_cpp", "split_cpp"
	};
	std::clog << "\nPACKAGE " << pkgname[i_test] << "\n";
	lang::package root(0, "root");
	lang::package sub(
	    &root, pkgname[i_test],
	    ((i_test & 1)? lang::package::flag_split : 0)
	    | ((i_test & 2)
	       ? lang::package::flag_lang_cxx
	       : lang::package::flag_lang_c),
	    lang::package::flag_split
	    | lang::package::flag_lang_mask);
	    
	lang::location t0(new(UseGC) test0_handler());
	sub.define("test_fun", t0, true);
	t0.call(&res, args);
	lang::location t1(new(UseGC) test1_handler);
	sub.define("test1", t1, true);
	t1.call(&res, args);
	MORE_CHECK_EQ(res, 767*767);
    }
    return 0;
}
catch (std::runtime_error const& xc) {
    std::cerr << "runtime_error: " << xc.what() << '\n';
    return 1;
}
// catch (std::logic_error const& xc) {
//     std::cerr << "logic_error: " << xc.what() << '\n';
//     return 1;
// }
