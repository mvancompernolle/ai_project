#include <more/diag/debug.h>
#include <more/lang/ct_type.h>
#include <more/lang/ct_struct.h>
#include <more/lang/ct_proto.h>
#include <iostream>

int
main()
try {
    using more::lang::ct_proto;
    using more::lang::ct_struct;
    using more::lang::ct_pointer;
    using more::lang::ct_type;
    using more::lang::ct_type_of;

    ct_proto pt(3, ct_type_of<void>());
    pt.set_arg_type(0, ct_type_of<int>());
    pt.set_arg_type(1, ct_type_of<int**>());
    pt.set_arg_type(2, ct_type_of<int (*)(char)>());
    pt.freeze();

    ct_pointer ptrdbl(ct_type_of<double>());

    ct_struct st;
    st.append(ct_type_of<int>(), "an_int");
    st.append(ct_type_of<unsigned int>());
    st.append(&ptrdbl, "a_ptr_to_double");
    st.append(ct_type_of<void**>(), "a_ptr_to_ptr_to_void");
    st.append(ct_type_of<int(*)[10]>(), "a_ptr_to_array_of_int");
    st.append(ct_type_of<int*[10]>(), "an_array_of_ptr_to_int");
    st.append(new(UseGC) ct_pointer(&pt), "a_fun_ptr");
    typedef void (*(*signal_proto)(int, void(*)(int)))(int);
    st.append(ct_type_of<signal_proto>(), "signal");
    st.freeze();

    st.print_forward(std::cout);
    st.print_definition(std::cout);
    ct_type_of<int const>()
	->print_declaration(std::cout, 0, "a_const_int");
    std::cout << '\n';
    ct_type_of<int* const>()
	->print_declaration(std::cout, 0, "a_const_ptr_to_int");
    std::cout << '\n';
    ct_type_of<int const*>()
	->print_declaration(std::cout, 0, "a_ptr_to_const_int");
    std::cout << '\n';
    ct_type_of<int const[10]>()
	->print_declaration(std::cout, 0, "a_array_of_const_int");
    std::cout << '\n';

    std::cout << "\nType promotions (does this seem right?):\n";
#define M(type)								\
    if (ct_type const* pro = ct_type_of<type>()->promoted()) {		\
	std::cout << "    "#type << " --> ";				\
	pro->print_declaration(std::cout, 0, "");			\
	std::cout << '\n';						\
    }									\
    else								\
	MORE_CHECK_FAILED("Builtin type "#type" lacks promotion type.");
    M(unsigned char);
    M(signed char);
    M(char);
    M(short);
    M(unsigned short);
    M(int);
    M(unsigned int);
    M(long);
    M(unsigned long);
    M(float);
    M(double);
    M(long double);
#undef M

    std::cout << "\nResult types:\n";
#define M(t0, t1)							\
    if (ct_type const* rt = arithmetic_result_type(ct_type_of<t0>(),	\
						   ct_type_of<t1>())) {	\
	std::cout << "    "#t0" + "#t1" --> ";				\
	rt->print_declaration(std::cout, 0, "");			\
	std::cout << '\n';						\
    }									\
    else								\
	MORE_CHECK_FAILED("No result type for "#t0" + "#t1".");
    M(int, int);
    M(short, int);
    M(unsigned int, int);
    M(long double, long double);
    M(float, int);
    M(double, float);
#undef M

    std::cout << "\nChecknig type sizes.\n";
#define M(type) \
    MORE_CHECK_EQ(ct_type_of<type>()->size(), sizeof(type))
    M(unsigned char);
    M(signed char);
    M(char);
    M(short);
    M(unsigned short);
    M(int);
    M(unsigned int);
    M(long);
    M(unsigned long);
    M(float);
    M(double);
    M(long double);
#undef M

    std::cout << std::endl;
    return more::diag::check_exit_status();
}
catch (std::runtime_error const& xc) {
    std::cerr << "runtime_error: " << xc.what() << '\n';
    return 1;
}
