#include <string.h>
#include <stdio.h>
#include <math.h>

#include "/home/home02/pmills/clib/time.h"
#include "/home/home02/pmills/clib/peteys_tmpl_lib.h"
#include "/home/home02/pmills/datasets/simple_temp.h"
#include "/home/home02/pmills/datasets/dependent_temp.h"
#include "read_field_file.h"

//global datasets containing the velocity field:
simple<float> *x_grid;
simple<float> *y_grid;
simple<float> *level;
simple<time> *time_grid;

dependent<float> *u;
dependent<float> *v;

void derivs(float tind, float xvec[], float dxdt[], long n) {
  double xint, yint;

  xint=x_grid->interp(xvec[0]);
  yint=y_grid->interp(xvec[1]);
  //printf("%f %f\n", xint, yint);
  u->interpol(dxdt[0], xint, yint, 0, (double) tind);
  v->interpol(dxdt[1], xint, yint, 0, (double) tind);

  //printf("Velocity at t= %f : (%f, %f)\n", tind, dxdt[0], dxdt[1]);

}

#define MAX_LINE_LENGTH 200
#define DATE_WIDTH 23

#define RMAX 10000

//syntax:
//model1 vfile n tstep
//  where:
//  	vfile is the binary file containing the velocity field
//	n is the number of integrated values
//	tstep is the time step in fractions of the grid spacing
//  initial conditions are read from standard in,
//  final conditions are written to standard out.

int main(int argc, char *argv[]) {
  char *infile="vfields/ecmwf_jan1999_400K.grd";	//binary file containing velocity field
  char *outfile="tracer.dat";
  char line[MAX_LINE_LENGTH];		//one line from output file
  long n;				//number of time steps
  long nfine;				//number of trajectory time steps
  float tstep_fine;			//time step for trajectory calc.
  float tstep;				//time step for tracer field
  time date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string
  double *tind;				//vector of time indices
  long lt;				//time index as longword
  float x0[2];				//initial conditions
  float **result;			//integrated values
  long nvar;				//number of variables in file
  dependent<float> **uandv;

  FILE *outfun;				//output file unit

  long nx, ny;
  dependent<float> *tracer1;
  dependent<float> *tracer2;
  dependent<float> *exch;

  //read in the velocity field:
  read_field_file_all(infile, uandv, nvar, x_grid, y_grid, level, time_grid);
  u=uandv[0];
  v=uandv[1];

  //figure out the initial time index:
  n=time_grid->nel()-3;
  tind=new double[n];
  tind[0]=2;
  tstep=1;
  nfine=4;
  tstep_fine=-tstep/nfine;

  nx=x_grid->nel();
  ny=y_grid->nel();

  for (long i=1; i<n; i++) tind[i]=tind[i-1]+tstep;

  //initialize the tracer concentrations:
  tracer1=new dependent<float>(x_grid, y_grid);
  tracer2=new dependent<float>(x_grid, y_grid);

  for (long i=0; i<nx; i++) {
    for (long j=0; j<ny; j++) {
      float xval, yval, r;
      x_grid->get(xval, i);
      y_grid->get(yval, j);
      r=sqrt(xval*xval+yval*yval);
      if (r < RMAX) {
        tracer1->cel((RMAX-r)/RMAX, i, j);
      } else {
        tracer1->cel(0, i, j);
      }
      tracer2->cel(0, i, j);
    }
  }

  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  outfun=fopen(outfile, "w");

  for (long it=0; it<n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    float val;		//interpolated value
    double xind, yind;	//interpolation index

    for (long i=20; i<nx-20; i++) {
      for (long j=20; j<ny-20; j++) {
        //use the current grid point as the initial condition:
        x_grid->get(x0[0], i);
        y_grid->get(x0[1], j);
//	printf("(%f, %f)\n", x0[0], x0[1]);
	//do a Runge-Kutta integration:
	rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs);
	//get the final position:
	xf=result[nfine][0];
	yf=result[nfine][1];
	//find the interpolated value in the previous tracer field:
        xind=x_grid->interp(xf);
	yind=y_grid->interp(yf);
	tracer1->interpol(val, xind, yind);
	if (val > 1) {
	  printf("(%f, %f): %f\n", xf, yf, val);
	}
	//change it in the new tracer field:
	tracer2->cel(val, i, j);

      }
    }

    //get the date:
    lt=(long) tind[it];
    time_grid->get(date1, lt);
    time_grid->get(date2, lt+1);
    date3=date2-date1;
    date2=date3*(tind[it]-(float) lt);
    date3=date1+date2;
    date3.write_string(date_str);

    printf("%d %s\n", it, date_str);

    //write the date and the tracer field to a file:
    fwrite(date_str, 1, sizeof(char)*DATE_WIDTH, outfun);
    tracer1->write(outfun);

    //exchange the old tracer field with the new one:
    exch=tracer1;
    tracer1=tracer2;
    tracer2=exch;
  }

  //get the date:
  lt=(long) tind[n-1];
  time_grid->get(date1, lt);
  time_grid->get(date2, lt+1);
  date3=date2-date1;
  date2=date3*(tind[n-1]-(float) lt);
  date3=date1+date2;
  date3.write_string(date_str);

  //write the date and the tracer field to a file:
  fwrite(date_str, 1, sizeof(char)*DATE_WIDTH, outfun);
  tracer1->write(outfun);

  fclose(outfun);

  //clean up:
  delete tind;
  for (long i=0; i<=nfine; i++) delete result[i];
  delete result;
  delete x_grid;
  delete y_grid;
  delete level;
  delete time_grid;
  delete u;
  delete v;
  delete uandv;

}
