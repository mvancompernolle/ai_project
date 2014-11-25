#include "stdio.h"
#include "math.h"

#include "coordtran.h"

int main(int argc, char **argv) {
  sub_1d_type n;
  simple<float> *xgrid, *ygrid;
  dependent<sub_1d_type> *map;

  for (long i=1; i<200; i++) {
    //forward model:
    xgrid=new simple<float>(-10000, 10000, i);
    ygrid=new simple<float>(-10000, 10000, i);
    //xgrid->print();

    map=new dependent<sub_1d_type>(xgrid, ygrid);
    nocorner_map(xgrid, ygrid, map, n);
    printf("%d %f %d %f\n", n, M_PI*(i+1)*(i+1)/4., i, 2.*(sqrt(n/M_PI)));

    delete map;
    delete xgrid;
    delete ygrid;
  }

}

