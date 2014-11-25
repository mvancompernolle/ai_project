#include "tcoord_defs.h"
#include "traj_int_obj.h"

int main(int argc, char *argv[]) {
  traj_int_obj *trobj;

  float lon, lat;

  float x0[2], xf[2];
  float x, y;
  short hemi;
  time_class t0;
  double tind0;
  long nt;
  double dt;
 
  time_class *tarr;
  time_class tgrid;
  time_class tstep;

  char tstring[30];

  //read in the parameters:
  trobj=new traj_int_obj(argv[1]);

  t0.read_string(argv[2]);
  sscanf(argv[3], "%f", &lon);
  sscanf(argv[4], "%f", &lat);

  tcoord_from_lonlat(lon, lat, hemi, x0[0], x0[1]);
  //tcoord_from_lonlat(lon, lat, hemi, x, y);
  printf("%f %f %f %f\n", lon, lat, x, y);

  sscanf(argv[5], "%lf", &dt);
  sscanf(argv[6], "%d", &nt);

  //make the time array:
  tarr=new time_class[nt*2+1];
  //assume velocity field has six-hour time-step:
  tgrid.read_string("0/0/0-6");
  tstep=tgrid*dt;
  tarr[nt]=t0;
  for (long i=1; i<=nt; i++) tarr[i+nt]=tarr[i+nt-1]+tstep;
  for (long i=nt-1; i>=0; i--) tarr[i]=tarr[i+1]-tstep;

  for (long i=0; i<+nt*2; i++) {
    tarr[i].write_string(tstring);
    printf("%s\n", tstring);
  }

  tind0=trobj->get_tind(t0);

  //integrate forwards:
  trobj->integrate(x0, xf, tind0, dt, nt);
  //print it out:
  trobj->print_result_N(stdout);
  //integrate backwards:
  trobj->integrate(x0, xf, tind0, -dt, nt);
  //print it out:
  trobj->print_result_N(stdout);

  //use the other interface:
  trobj->int_tarray(x0, t0, tarr, nt*2+1);
  trobj->print_result_N(stdout);

  delete [] tarr;
  delete trobj;

}

