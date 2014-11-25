#include <string.h>
#include <stdio.h>
#include <math.h>

#include "time_class.h"
#include "peteys_tmpl_lib.h"
#include "simple_temp.h"
#include "dependent_temp.h"
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
//model1 vfile qfile outfile
//  where:
//  	vfile is the binary file containing the velocity field
//	n is the number of integrated values
//	tstep is the time step in fractions of the grid spacing
//  initial conditions are read from standard in,
//  final conditions are written to standard out.

int main(int argc, char *argv[]) {
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

  long nx, ny;				//dimensions of tracer field
  simple<float> *x_qgrid;		//tracer x grid
  simple<float> *y_qgrid;		//tracer y grid
  simple<time> *t_qgrid;		//tracer time value
  simple<float> *qlev;			//tracer level
  dependent<float> *qinit;		//initial tracer field (first variable)
  dependent<float> *tracer1;		//working tracer field
  dependent<float> *tracer2;
  dependent<float> *exch;

  if (argc != 5) return 1;

  //read in the velocity field:
  read_field_file_all(argv[1], uandv, nvar, x_grid, y_grid, level, time_grid);
  u=uandv[0];
  v=uandv[1];

  //read in the initial tracer field:
  read_field_file(argv[2], "q", qinit, x_qgrid, y_qgrid, qlev, t_qgrid);

  //read in the number of time steps:
  sscanf(argv[4], "%d\n", &n);

  //figure out the initial time index:
  tind=new double[n];
  //get the date of the initial tracer field:
  t_qgrid->get(date1, 0);
  //figure out its location in relation to the velocity field:
  tind[0]=time_grid->interp(date1);
  tstep=1;
  nfine=4;
  tstep_fine=-tstep/nfine;

  nx=x_qgrid->nel();
  ny=y_qgrid->nel();

  for (long i=1; i<n; i++) tind[i]=tind[i-1]+tstep;

  //initialize the tracer concentrations:
  tracer1=new dependent<float>(x_qgrid, y_qgrid);
  tracer2=new dependent<float>(x_qgrid, y_qgrid);

  for (long i=0; i<nx; i++) {
    for (long j=0; j<ny; j++) {
      float val;
      qinit->get(val, i, j, 0, 0);
      tracer1->cel(val, i, j);
      tracer2->cel(val, i, j);
    }
  }

  //don't need this stuff any more, get rid of it:
  delete qinit;
  delete qlev;
  delete t_qgrid;

  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  outfun=fopen(argv[3], "w");

  for (long it=0; it<n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    float val;		//interpolated value
    double xind, yind;	//interpolation index

    for (long i=0; i<nx; i++) {
      for (long j=0; j<ny; j++) {
        //use the current grid point as the initial condition:
        x_qgrid->get(x0[0], i);
        y_qgrid->get(x0[1], j);
//	printf("(%f, %f)\n", x0[0], x0[1]);
	//do a Runge-Kutta integration:
	rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs);
	//get the final position:
	xf=result[nfine][0];
	yf=result[nfine][1];
	//find the interpolated value in the previous tracer field:
        xind=x_qgrid->interp(xf);
	yind=y_qgrid->interp(yf);
	tracer1->interpol(val, xind, yind);
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

  delete u;
  delete v;
  delete uandv;
  delete x_grid;
  delete y_grid;
  delete level;
  delete time_grid;

  delete tracer1;
  delete tracer2;
  delete x_qgrid;
  delete y_qgrid;

}
