#include <more/math/spinor.h>
#include <more/math/cospinor.h>
#include <more/math/complex.h>
#include <iostream>

using namespace more::math;

using std::cout;
using std::endl;

#define test(x) cout << #x << " = " << x << endl

int main() {
    typedef spinor< complex<double> > spinor_t;
    typedef cospinor< complex<double> > cospinor_t;
    spinor_t spn1, spn2;

    spn1 = (.4+onei*0.)*alpha + onei*.5*beta;
    spn2 = (1.7+1.0*onei)*alpha + onei*1.*beta;

    test(spn1);
    test(spn2);
    test(spn1 + spn2);
    test(spn1 - spn2);
    test(adj(spn1));
    test(adj(spn1)*spn2);
    test(dot(spn1, spn2));
    test(real_dot(spn1, spn2));
    test(spn1*adj(spn2));
    test(outer_prod(spn1, spn2));

    test(10.0*spn1);
    test((10.0+10.0*onei)*spn1);
    test(spn1*100.0);
    test(spn1/100.0);

    typedef spinopr< complex<double> > op_t;
    op_t sigx = sigma_x;
    op_t sigy = sigma_y;
    op_t sigz = sigma_z;
    op_t sigx2z = sigma_x + 2.0*sigma_z;
    test(sigx);
    test(sigy);
    test(sigz);
    test(sigx*sigy);
    test(sigx*sigma_y);
    test(sigma_x*sigy);
    test(sigx + sigy);
    test(sigx - sigy);
    test(sigx*sigy - sigy*sigx);
    test(sigx*sigy - sigy*sigma_x);
    test(sigy*sigz - sigz*sigy);
    test((sigma_x*sigy - sigma_y*sigx)/(2.0*onei));
    test((sigy*sigma_z - sigma_z*sigy)/(2.0*onei));
    test((sigma_z*sigx - sigma_x*sigz)/(2.0*onei));
    test(sigx2z + 1.0);
    test(sigx2z - 1.0);
    test(1.0 - sigx2z);
    test(1.0 + sigx2z);

    test(spinor_t(alpha) * cospinor_t(cobeta));
    test(spinor_t(beta) * cospinor_t(cobeta));
//    test(adj(sigma_x));
//    test(adj(sigx));
}
