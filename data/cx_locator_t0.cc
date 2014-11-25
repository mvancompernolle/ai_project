#include <more/lang/cx_locator.h>
#include <more/lang/ct_struct.h>
#include <more/diag/debug.h>
#include <sstream>

struct st0_t {
    int i;
    double x;
};

struct st1_t {
    int j;
    st0_t st0a;
    double y;
    st0_t* st0b_ptr;
};

std::string
c_name(more::lang::cx_locator* loc)
{
    std::ostringstream oss;
    loc->print_operation_pre(oss);
    oss << "var";
    loc->print_operation_post(oss);
    return oss.str();
}


int
main()
try {
    using more::lang::cx_locator;
    using more::lang::ct_struct;
    using more::lang::ct_type_of;
    using more::lang::ct_pointer;
    using more::lang::cx_member_locator;
    using more::lang::cx_deref_locator;

#if 0

    ct_struct st0_desc;
    st0_desc.append(ct_type_of<int>(), "i");
    st0_desc.append(ct_type_of<double>(), "x");
    st0_desc.set_c_name("st0_t");
    st0_desc.freeze();

    ct_struct st1_desc;
    st1_desc.append(ct_type_of<int>(), "j");
    st1_desc.append(&st0_desc, "st0a");
    st1_desc.append(ct_type_of<double>(), "y");
    st1_desc.append(new(UseGC) ct_pointer(&st0_desc), "st0b_ptr");
    st1_desc.freeze();

    ct_struct::member_const_iterator it = st1_desc.member_begin();
    cx_member_locator loc_j(&st1_desc, it++);
    cx_member_locator loc_st0a(&st1_desc, it++);
    cx_member_locator loc_y(&st1_desc, it++);
    cx_member_locator loc_st0b_ptr(&st1_desc, it++);
    assert(it == st1_desc.member_end());
    cx_deref_locator loc_st0b(&loc_st0b_ptr);
    it = st0_desc.member_begin();
    ++it;
    cx_member_locator loc_st0a_x(&loc_st0a, it);
    cx_member_locator loc_st0b_x(&loc_st0b, it);

#else

    typedef ct_struct::member_const_iterator iter_t;
    ct_struct st0_desc;
    iter_t it_i = st0_desc.append(ct_type_of<int>(), "i");
    iter_t it_x = st0_desc.append(ct_type_of<double>(), "x");
    st0_desc.set_c_name("st0_t");
    st0_desc.freeze();

    ct_struct st1_desc;
    iter_t it_j = st1_desc.append(ct_type_of<int>(), "j");
    iter_t it_st0a = st1_desc.append(&st0_desc, "st0a");
    iter_t it_y = st1_desc.append(ct_type_of<double>(), "y");
    iter_t it_st0b_ptr
	= st1_desc.append(new(UseGC) ct_pointer(&st0_desc), "st0b_ptr");
    st1_desc.freeze();

    cx_member_locator loc_j(&st1_desc, it_j);
    cx_member_locator loc_st0a(&st1_desc, it_st0a);
    cx_member_locator loc_y(&st1_desc, it_y);
    cx_member_locator loc_st0b_ptr(&st1_desc, it_st0b_ptr);
    cx_deref_locator loc_st0b(&loc_st0b_ptr);
    cx_member_locator loc_st0a_x(&loc_st0a, it_x);
    cx_member_locator loc_st0b_x(&loc_st0b, it_x);

    cx_member_locator loc_x(&st0_desc, it_x);
    cx_member_locator loc_st0a_x_alt(&loc_st0a, &loc_x);
    cx_member_locator loc_st0b_x_alt(&loc_st0b, &loc_x);

#endif

    st0_t st0;
    st0.i = 10;
    st0.x = 10.1;

    st1_t st1;
    st1.j = 11;
    st1.y = 11.11;
    st1.st0a.i = 1000;
    st1.st0a.x = 1000.1;
    st1.st0b_ptr = &st0;

    MORE_CHECK_EQ(loc_j.apply(&st1), &st1.j);
    MORE_SHOW(c_name(&loc_y));
    MORE_CHECK_EQ(loc_y.apply(&st1), &st1.y);

    MORE_SHOW(&st1);
    MORE_SHOW(offsetof(st1_t, st0a));

    MORE_SHOW(c_name(&loc_st0a));
    std::clog << "loc_st0a = ";
    loc_st0a.debug_dump(std::clog);
    std::clog << '\n';
    MORE_CHECK_EQ(loc_st0a.apply(&st1), &st1.st0a);

    MORE_SHOW(c_name(&loc_st0b_ptr));
    MORE_CHECK_EQ(loc_st0b_ptr.apply(&st1), &st1.st0b_ptr);

    MORE_SHOW(c_name(&loc_st0b));
    MORE_CHECK_EQ(loc_st0b.apply(&st1), st1.st0b_ptr);
    std::clog << "loc_st0b = ";
    loc_st0b.debug_dump(std::clog);
    std::clog << '\n';

    MORE_SHOW(c_name(&loc_st0a_x));
    MORE_CHECK_EQ(loc_st0a_x.apply(&st1), &st1.st0a.x);

    MORE_SHOW(c_name(&loc_st0b_x));
    MORE_CHECK_EQ(loc_st0b_x.apply(&st1), &st1.st0b_ptr->x);

    MORE_SHOW(c_name(&loc_st0a_x_alt));
    MORE_CHECK_EQ(loc_st0a_x_alt.apply(&st1), &st1.st0a.x);

    MORE_SHOW(c_name(&loc_st0b_x_alt));
    MORE_CHECK_EQ(loc_st0b_x_alt.apply(&st1), &st1.st0b_ptr->x);

    return more::diag::check_exit_status();
}
catch (std::exception const& xc) {
    std::cerr << xc.what() << std::endl;
    return 1;
}
