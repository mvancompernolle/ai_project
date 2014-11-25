#include "tcoord_defs.h"

#include "/home/home02/pmills/clib/peteys_tmpl_lib.h"
#include "/home/home02/pmills/clib/time.h"
#include "/home/home02/pmills/datasets/simple_temp.h"
#include "/home/home02/pmills/datasets/dependent_swap.h"
#include "/home/home02/pmills/datasets/composite_dataset.h"

//global datasets containing the velocity field:
simple<float> *x_grid;		//Gridding in the x direction
simple<float> *y_grid;		//Gridding in the y direction
simple<time> *time_grid;	//The time gridding

dependent_swap<float> *u;	//Velocity in the x direction.
dependent_swap<float> *v;	//Velocity in the y direction.

FILE *vfield_swap;

float **result;
long nres;

void traj_derivs(float tind, float xvec[], float dxdt[], long n)
// name: 	traj_derivs
//
// usage: 	derivs(tind, xvec, dxdt, n);
//
// purpose: 	Returns a set of derivatives for use in a Runge-Kutta or
// 		similar integration scheme.  Returns the interpolated values
// 		of a two-dimensional velocity field at the supplied time and
// 		location.
//
// parameters:
// 		tind: (in) The non-dimensional time as a floating point
// 		variable.  Must be normalized to the grid spacing of the
// 		velocity field.
//
// 		xvec: (in) The position.  Must be a two dimensional floating
// 		point array.
//
// 		dxdt: (out) A two-dimensional floating point array containing
// 		the velocity field at the particular time and location.
//
// 		n: (in) Ignored.  Should be two (2).
//
{
  double xint, yint;
  float nn;

  xint=x_grid->interp(xvec[0]);
  yint=y_grid->interp(xvec[1]);
  u->interpol(dxdt[0], xint, yint, 0., (double) tind);
  v->interpol(dxdt[1], xint, yint, 0., (double) tind);

}


void traj_int(vector &p0, vector &pf, float t0, float tstep, long nt) {
  if (nt > nres) {
    for (long i=0; i<=nres; i++) delete result[i];
    delete result;
    result=new float * [nt+1];
    for (long i=0; i<=nt; i++) result[i]=new float[2];
    nres=nt;
  }

  rk_dumb(t0, (float *) &p0, 2L, tstep, nt, result, &traj_derivs);

  pf=*(vector *) result[nt];
}

time traj_get_t(double tind) {
  time tlow, tdiff, result;
  long lind;
  double frac;

  lind=(long) tind;
  frac=tind-(double) lind;

  tlow=time_ind->get(lind);
  tdiff=time_ind->get(lind+1)-tlow;

  return tlow+tdiff*frac;

}

inline double traj_tind(time t) {
  return time_ind->interp(t);
}

void traj_int_init(char *filename) {
  long loc, dum;
  composite_dataset vdata;

  //read in the velocity field:
  vfield_swap=fopen(filename, "r");
  vdata.read(vfield_swap);

  //get the x grid:
  loc=vdata.search_var("xgrid", dum);
  x_grid=(simple<float> *) vdata.get_var(loc);
  //get the y grid:
  loc=vdata.search_var("ygrid", dum);
  y_grid=(simple<float> *) vdata.get_var(loc);
  //get the time grid:
  loc=vdata.search_var("time", dum);
  time_grid=(simple<time> *) vdata.get_var(loc);
  //get the velocity in the x direction:
  loc=vdata.search_var("u", dum);
  u=(dependent_swap<float> *) vdata.get_var(loc);
  //get the velocity in the y direction:
  loc=vdata.search_var("v", dum);
  v=(dependent_swap<float> *) vdata.get_var(loc);

  nres=1;
  result=new float *;
  result[0]=new float[2];

}

void traj_int_end() {

  for (long i=0; i<=nres; i++) delete result[i];
  delete result;

  delete u;
  delete v;
  delete x_grid;
  delete y_grid;
  delete time_grid;

  fclose(vfield_swap);
}
