//
// Copywrite 2004 Peter Mills.  All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "time_class.h"
#include "peteys_tmpl_lib.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

#include "dependent_intc.h"

#include "sparse.h"

//flags for the different boundary conditions:
#define BC_FLOAT 'f'
#define BC_FIXED 'x'
#define BC_ZERO 'o'
#define BC_EXTRAP 'e'

//global datasets containing the velocity field:
simple<float> *x_grid;
simple<float> *y_grid;
//simple<float> *level;
simple<time_class> *time_grid;

dependent_swap<float> *u;
dependent_swap<float> *v;

void derivs(float tind, float xvec[], float dxdt[], long n) {
  interpol_index xint, yint;

  xint=x_grid->interp(xvec[0]);
  yint=y_grid->interp(xvec[1]);
  //printf("%f %f\n", xint, yint);
  u->interpol(dxdt[0], xint, yint, 0, (interpol_index) tind);
  v->interpol(dxdt[1], xint, yint, 0, (interpol_index) tind);

  //printf("Velocity at t= %f : (%f, %f)\n", tind, dxdt[0], dxdt[1]);

}

#define MAX_LINE_LENGTH 200
#define DATE_WIDTH 23

#define RMAX 10000

//syntax:
//tracer3_step1 vfile outfile t0 nt [tstep [nfine [np [rmax]]]]
//  where:
//	vfile		Binary file containing velocity field
//	outfile		Output file
//	t0		Date to start integration
//	nt		Number of (Eulerian) time steps
//	tstep		Non-dimensional time (Eulerian, i.e. coarse) step
//	nfine		Number of Runge-Kutta steps between each time step
//	np		Number of grid points per side of tracer
//	rmax		Size of tracer field i.e. side length divided by 2

int main(int argc, char *argv[]) {
  //main input arguments:
  char *vfile;
  char *outfile;

  //composite dataset containing velocity field:
  composite_dataset vdata;
  FILE *vfs;				//file stream for velocity field
  long loc, dum;			//location of the datasets (members of vdata)

  //time step info:
  int32_t n;				//number of time steps
  int32_t nfine;			//number of trajectory time steps
  float tstep_fine;			//time step for trajectory calc.
  float tstep;				//time step for tracer field

  //general date variables:
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string

  //time indices:
  double *tind;				//vector of time indices
  ind_type lt;				//time index as integer

  //stuff for integration:
  float x0[2];				//initial conditions
  float **result;			//integrated values

  FILE *outfun;				//output file unit

  //grid info:
  ind_type np;				//number of x and y tracer grids
  float maxr;				//physical size of tracer field=side_length/2

  float *qgrid;
  simple<float> *x_qgrid;		//tracer x grid
  simple<float> *y_qgrid;		//tracer y grid

  //dummy tracer field for calculating interpolation coefficients:
  dependent_intc *tracer;

  //sparse matrix class instance for holding the matrix of interpolation coeffs:
  sparse_matrix map;

  //boundary conditions:
  char bc;		//type of bc--defined above
  int buffer=0;		//size of buffer zone (for fixed bc's)

  if (argc < 5 || argc > 10) return 1;

  //parse the input arguments:
  //search for bc flag:
  if (argv[1][0]=='-') {
    bc=argv[1][1];
    argc--;
    argv++;
  } else {
    bc='f';		//default floating bc's
  }
  
  vfile=argv[1];		//file containing velocity field
  outfile=argv[2];		//output file

  date1=argv[3];		//date to start the integration

  //read in the number of time steps:
  sscanf(argv[4], "%d", &n);

  //read in the velocity field:
  /*
  read_field_file(vfile, "u", u, x_grid, y_grid, level, time_grid);
  v=new dependent<float>(x_grid, y_grid, level, time_grid);
  read_field_file_data(vfile, "v", v);
  */

  vfs=fopen(vfile, "r");
  vdata.read(vfs);
  loc=vdata.search_var("xgrid", dum);
  x_grid=(simple<float> *) vdata.get_var(loc);
  loc=vdata.search_var("ygrid", dum);
  y_grid=(simple<float> *) vdata.get_var(loc);
  loc=vdata.search_var("time", dum);
  time_grid=(simple<time_class> *) vdata.get_var(loc);
  loc=vdata.search_var("u", dum);
  u=(dependent_swap<float> *) vdata.get_var(loc);
  loc=vdata.search_var("v", dum);
  v=(dependent_swap<float> *) vdata.get_var(loc);

  //scan the optional input arguments:
  //defaults:
  tstep=1;
  nfine=4;
  np=x_grid->nel();
  x_grid->get(maxr, np-1);
//  printf("max r= %f\n", maxr);
  maxr=fabs(maxr);

  switch (argc) {
    case 9: sscanf(argv[8], "%f", &maxr);
    case 8: sscanf(argv[7], "%d", &np);
    case 7: sscanf(argv[6], "%d", &nfine);
    case 6: sscanf(argv[5], "%f", &tstep);
  }

  printf("tstep: %f, Runge-Kutta steps: %d, grid size: %d, max r: %f\n",
  		tstep, nfine, np, maxr);

  //figure out the initial time index:
  tind=new double[n+1];
  //figure out its location in relation to the velocity field:
  tind[0]=time_grid->interp(date1);
  tstep_fine=-tstep/nfine;

  for (long i=1; i<=n; i++) tind[i]=tind[i-1]+tstep;

  //initialize the dummy tracer field:
  //tracer gridding:
  qgrid=new float [np];
  for (long i=0; i<np; i++) {
    qgrid[i]=2*maxr*i/np-maxr;
  }
  x_qgrid=new simple<float>(qgrid, np, 0);
  y_qgrid=new simple<float>(qgrid, np, 0);
  delete qgrid;
  tracer=new dependent_intc(x_qgrid, y_qgrid);

  //initialize the sparse matrix:
  map.extend(np*np*4);

  //initialize the vector of results for the Runge-Kutta integrations:
  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  //open the output file and write the headers:
  outfun=fopen(outfile, "w");
//  fwrite(&maxr, 1, sizeof(maxr), outfun);
//  fwrite(&np, 1, sizeof(np), outfun);
//  fwrite(&n, 1, sizeof(n), outfun);

  if (bc == BC_FIXED) {
    //fixed boundary cond's: all elements around outside remain fixed
    buffer=1;
    //otherwise equivalent to floating:
    bc=BC_FLOAT;
  }

  for (long it=0; it<n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    float val;			//interpolated value
    interpol_index xyind[2];	//interpolation indices
    sub_1d_type row_sub;
    sub_1d_type sub[4];		//1d subscripts
    double weight[4];		//interpolation coeffs

    map.reset(np*np, np*np);

    for (ind_type i=buffer; i<np-buffer; i++) {
      for (ind_type j=buffer; j<np-buffer; j++) {
        //use the current grid point as the initial condition:
        x_qgrid->get(x0[0], i);
        y_qgrid->get(x0[1], j);
//	printf("(%f, %f)\n", x0[0], x0[1]);
	//do a Runge-Kutta integration:
	rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs);
	//get the final position:
	xf=result[nfine][0];
	yf=result[nfine][1];

//	printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);
	//find the interpolated value in the previous tracer field:
        xyind[0]=x_qgrid->interp(xf);
	xyind[1]=y_qgrid->interp(yf);

        //apply floating boundary cond's (if applicable):
	if (xyind[0]>=np && bc==BC_FLOAT) xyind[0]=np-1;
	if (xyind[1]>=np && bc==BC_FLOAT) xyind[1]=np-1;
	if (xyind[0]<0 && bc==BC_FLOAT) xyind[0]=0;
	if (xyind[1]<0 && bc==BC_FLOAT) xyind[1]=0;
	  
	tracer->interpol_coeff(xyind, sub, weight);

	row_sub=(ind_type) (j*np+i);

	//apply zero boundary conditions:
	if (bc==BC_ZERO && 
			(xyind[0]>=np || xyind[1]>=np || 
			 xyind[0] < 0 || xyind[1] < 0))
			for (long ii=0; i<4; i++) weight[i]=0;

	map.add_el(weight[0], row_sub, (ind_type) sub[0]);
	map.add_el(weight[1], row_sub, (ind_type) sub[1]);
	map.add_el(weight[2], row_sub, (ind_type) sub[2]);
	map.add_el(weight[3], row_sub, (ind_type) sub[3]);
      }
    }

    //get the date:
    lt=(ind_type) tind[it];
    time_grid->get(date1, lt);
    time_grid->get(date2, lt+1);
    date3=date2-date1;
    date2=date3*(tind[it]-(float) lt);
    date3=date1+date2;
    date3.write_string(date_str);

    printf("%d %s\n", it, date_str);

    //write the date and the tracer field to a file:
//    fwrite(date_str, 1, sizeof(char)*DATE_WIDTH, outfun);
    map.write(outfun);
//    map.print(stdout);

  }

  fclose(outfun);
  fclose(vfs);

  //clean up:
  delete tind;
  for (long i=0; i<=nfine; i++) delete result[i];
  delete result;

  delete u;
  delete v;
  delete x_grid;
  delete y_grid;
  delete time_grid;

  delete tracer;
  delete x_qgrid;
  delete y_qgrid;

}
