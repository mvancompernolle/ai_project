#include <more/lang/ct_struct.h>
#include <more/diag/debug.h>
#include <assert.h>

struct test_type
{
    char ch;
    int i;
    double x;
    char chp;
};

int
main()
{
    using more::lang::ct_struct;
    using more::lang::ct_type_of;
    ct_struct* rt_test = new ct_struct;

    rt_test->append(ct_type_of<char>());
    rt_test->append(ct_type_of<int>());
    rt_test->append(ct_type_of<double>());
    rt_test->append(ct_type_of<char>());
    rt_test->freeze();

    ct_struct::member_iterator it_ch = rt_test->member_begin(), it_i, it_x;
    it_i = it_ch, ++it_i;
    it_x = it_i, ++it_x;

    test_type test;
    MORE_CHECK_EQ(&test.ch, it_ch->apply(&test));
    MORE_CHECK_EQ(&test.i, it_i->apply(&test));
    MORE_CHECK_EQ(&test.x, it_x->apply(&test));
    MORE_CHECK_EQ(sizeof(test_type), rt_test->size());
    struct at { char pad; test_type tt; };
    MORE_CHECK_EQ(offsetof(at, tt), rt_test->alignment());

    rt_test->print_definition(std::cout, 0);

    delete rt_test;

    return 0;
}
