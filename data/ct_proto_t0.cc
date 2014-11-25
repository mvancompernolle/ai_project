#include <more/lang/ct_proto.h>

int
f(char ch0, int i, double x, double* xx)
{
    *xx = -x;
    return ch0*100 + i;
}

int
main()
{
    using more::lang::ct_proto;
    using more::lang::ct_type_of;
    ct_proto rtt_f(4, ct_type_of<int>());
    rtt_f.set_arg_type(0, ct_type_of<char>());
    rtt_f.set_arg_type(1, ct_type_of<int>());
    rtt_f.set_arg_type(2, ct_type_of<double>());
    rtt_f.set_arg_type(3, ct_type_of<double*>());
    rtt_f.freeze();

    int i_ret;
    char ch_in = 'a';
    int i_in = 11;
    double x_in = 3.779;
    double x_out;
    double* x_out_ptr = &x_out;
    void* args[] = { &ch_in, &i_in, &x_in, &x_out_ptr };
    rtt_f.call(more::lang::fn_ptr_t(&f), &i_ret, args);
    assert(i_ret == 'a'*100 + 11);
    assert(x_out == -3.779);

    return 0;
}
