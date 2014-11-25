//
// Copywrite 2004 Peter Mills.  All rights reserved.
//

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include "time_class.h"
#include "peteys_tmpl_lib.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

#include "dependent_intc.h"

#include "tcoord_defs.h"
#include "default_grid.h"

#include "sparse.h"

//global datasets containing the velocity field:
simple<float> *x_grid_n;
simple<float> *y_grid_n;
simple<float> *x_grid_s;
simple<float> *y_grid_s;
//simple<float> *level;
simple<time_class> *time_grid;

dependent_swap<float> *u_n;
dependent_swap<float> *v_n;
dependent_swap<float> *u_s;
dependent_swap<float> *v_s;

//northern hemisphere derivatives:
void derivs_n(float tind, float xvec[], float dxdt[], long n) {
  interpol_index xint, yint;

  xint=x_grid_n->interp(xvec[0]);
  yint=y_grid_n->interp(xvec[1]);
  //printf("%f %f\n", xint, yint);
  u_n->interpol(dxdt[0], xint, yint, 0, (interpol_index) tind);
  v_n->interpol(dxdt[1], xint, yint, 0, (interpol_index) tind);

  //printf("Velocity at t= %f : (%f, %f)\n", tind, dxdt[0], dxdt[1]);

}

//southern hemisphere derivatives:
void derivs_s(float tind, float xvec[], float dxdt[], long n) {
  interpol_index xint, yint;

  xint=x_grid_s->interp(xvec[0]);
  yint=y_grid_s->interp(xvec[1]);
  //printf("%f %f\n", xint, yint);
  u_s->interpol(dxdt[0], xint, yint, 0, (interpol_index) tind);
  v_s->interpol(dxdt[1], xint, yint, 0, (interpol_index) tind);

  //printf("Velocity at t= %f : (%f, %f)\n", tind, dxdt[0], dxdt[1]);

}

#define MAXLL 200
#define DATE_WIDTH 23

//syntax:
//tracer4_step1 initfile
//
// where initfile is the initialization file
//
// format of initfile:
//
//nfile		Binary file containing velocity field in the N. hemi.
//sfile		Binary file containing the velocity field in the S. hemi.
//outfile	Output file
//mapfile	File containing the mapping from the two tracer fields to a vector
//t0		Date to start integration
//nt		Number of (Eulerian) time steps
//tstep		Non-dimensional time (Eulerian, i.e. coarse) step
//nfine		Number of Runge-Kutta steps between each time step
//np		Number of grid points per side

int main(int argc, char *argv[]) {
  char *initfile;
  FILE *initfs;
  char line[MAXLL];

  //main data file names:
  char nfile[MAXLL];
  char sfile[MAXLL];
  char outfile[MAXLL];

  //composite dataset containing velocity field:
  composite_dataset ndata;
  composite_dataset sdata;
  FILE *nfs;				//file stream for velocity field
  FILE *sfs;
  long loc, dum;			//location of the datasets (members of vdata)

  //time step info:
  long n;				//number of time steps
  long nfine;				//number of trajectory time steps
  float tstep_fine;			//time step for trajectory calc.
  float tstep;				//time step for tracer field

  //general date variables:
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string
  char ranstr[MAXLL];

  //time indices:
  double *tind;				//vector of time indices
  ind_type lt;				//time index as longword

  //stuff for integration:
  float x0[2];				//initial conditions
  float **result;			//integrated values

  FILE *outfun;				//output file unit

  //grid info:
  long np=0;				//number of x and y tracer grids

  float *qgrid;
  simple<float> *x_qgrid;		//tracer x grid
  simple<float> *y_qgrid;		//tracer y grid

  //dummy tracer field for calculating interpolation coefficients:
  dependent_intc *tracer;

  //sparse matrix class instance for holding the matrix of interpolation coeffs:
  sparse_matrix map;

  //for checking the consistancy of the two files:
  simple<float> *check1;
  simple<float> *check2;
  simple<time_class> *check3;

  //really this is a misnomer, but that's the name I've used
  //in all the other codes:
  float rmax=SIDELENGTH_Q;

  //intermediate values:
  float r;	//radius
  float dx;
  long ind, k;

  char c;
  long ncon;

  while ((c = getopt(argc, argv, "r:n:")) != -1) {
    switch (c) {
      case ('n'):
        ncon=sscanf(optarg, "%d", &np);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -n %s", optarg);
          exit(2);
        }
        break;
      case ('r'):
        ncon=sscanf(optarg, "%f", &rmax);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -r %s", optarg);
          exit(2);
        }
        break;
      case ('?'):
             fprintf(stderr, "Unknown option: -%c -- ignored\n", optopt);
             break;
      default:
             fprintf(stderr, "Error parsing command line\n");
             exit(2);
             break;
    }
  }
  argc-=optind;
  argv+=optind;

  if (argc < 1) {
    printf("Purpose: same as tracer4_matout, except the entire field for each\n");
    printf("hemisphere is advected and stored.\n");
    printf("\n");
    printf("syntax: tracer4a_matout [-r slen] [-n np] initfile\n");
    printf("\n");
    printf("initfile is an initialisation file\n");
    printf("\n");
    printf("The format of the initialisation file is the same as for tracer4_matout\n");
    printf("except that line 4 is ignored\n\n");
    printf("- slen  is the sidelength/2 of the azimuthal equidistant coords\n");
    printf("(default=%10.1f)\n\n", rmax);
    printf("- np    is the number of grids per side\n");
    return 1;
  }

  initfile=argv[0];

  //parse the initialisation file:
  initfs=fopen(initfile, "r");
  fgets(line, MAXLL, initfs);
  sscanf(line, "%s", nfile);
  fgets(line, MAXLL, initfs);
  sscanf(line, "%s", sfile);
  fgets(line, MAXLL, initfs);
  sscanf(line, "%s", outfile);
  fgets(line, MAXLL, initfs);		//ignore "mapfile" entry in init. file
  fgets(line, MAXLL, initfs);
  date1=line;
  strcpy(ranstr, line);
  fgets(line, MAXLL, initfs);
  sscanf(line, "%g", &tstep);
  fgets(line, MAXLL, initfs);
  sscanf(line, "%d", &n);
  fgets(line, MAXLL, initfs);
  sscanf(line, "%d", &nfine);
  fgets(line, MAXLL, initfs);
  if (np <= 0) sscanf(line, "%d", &np);

  printf("tstep: %f, number of time steps: %d, Runge-Kutta steps: %d, grid size: %d\n",
  		tstep, n, nfine, np);
  date1.write_string(date_str);
  printf("t0=%s; %s\n", date_str, ranstr);

  //get the N. hemi. velocity fields:
  nfs=fopen(nfile, "r");
  ndata.read(nfs);
  loc=ndata.search_var("xgrid", dum);
  x_grid_n=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("ygrid", dum);
  y_grid_n=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("time", dum);
  time_grid=(simple<time_class> *) ndata.get_var(loc);
  loc=ndata.search_var("u", dum);
  u_n=(dependent_swap<float> *) ndata.get_var(loc);
  loc=ndata.search_var("v", dum);
  v_n=(dependent_swap<float> *) ndata.get_var(loc);

  //get the S. hemi. velocity fields:
  sfs=fopen(sfile, "r");
  sdata.read(sfs);
  loc=sdata.search_var("xgrid", dum);
  x_grid_s=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("ygrid", dum);
  y_grid_s=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("u", dum);
  u_s=(dependent_swap<float> *) sdata.get_var(loc);
  loc=sdata.search_var("v", dum);
  v_s=(dependent_swap<float> *) sdata.get_var(loc);

  //check the consistency of the two fields:
  loc=ndata.search_var("zgrid", dum);
  check1=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("zgrid", dum);
  check2=(simple<float> *) sdata.get_var(loc);
  assert(*check1 == *check2);
  loc=sdata.search_var("time", dum);
  check3=(simple<time_class> *) sdata.get_var(loc);
  assert(*time_grid==*check3);

  //figure out the initial time index:
  tind=new double[n+1];
  //figure out its location in relation to the velocity field:
  tind[0]=time_grid->interp(date1);
  tstep_fine=-tstep/nfine;

  for (long i=1; i<=n; i++) tind[i]=tind[i-1]+tstep;

  //initialize the dummy tracer field:
  //tracer gridding:
  /*(have no idea what all this shit is for... )
  qgrid=new float [np];
  dx=2.0*rmax/(np-4.0);
  maxr=rmax+sqrt(2)*dx;
  for (long i=0; i<np; i++) {
    qgrid[i]=dx*(i-1.5)-rmax;
  }
  x_qgrid=new simple<float>(qgrid, np, 0);
  y_qgrid=new simple<float>(qgrid, np, 0);
  */
  //no reason to have any overlap whatsoever between the grids:
  x_qgrid=new simple<float>(-rmax, rmax, np);
  y_qgrid=new simple<float>(-rmax, rmax, np);

  tracer=new dependent_intc(x_qgrid, y_qgrid);

  //initialize the sparse matrix:
  map.extend(np*np*8);

  //initialize the vector of results for the Runge-Kutta integrations:
  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  //open the output file and write the headers:
  outfun=fopen(outfile, "w");
//  fwrite(&maxr, 1, sizeof(maxr), outfun);
//  fwrite(&np, 1, sizeof(np), outfun);
//  fwrite(&n, 1, sizeof(n), outfun);

  for (long it=0; it<n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    float val;			//interpolated value
    interpol_index xyind[2];	//interpolation indices
    ind_type row_sub;
    sub_1d_type sub[4];		//1d subscripts
    double weight[4];		//interpolation coeffs
    short hemi;			//hemisphere

    map.reset(np*np*2, np*np*2);

    for (ind_type i=0; i<np; i++) {
      for (ind_type j=0; j<np; j++) {
        //check to see that the point falls within the useable regions:
	ind=j*np+i;
        //use the current grid point as the initial condition:
        x_qgrid->get(x0[0], i);
        y_qgrid->get(x0[1], j);
//	  printf("(%f, %f)\n", x0[0], x0[1]);

        //Northern hemisphere:
        //do a Runge-Kutta integration:
        rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs_n);
        //get the final position:
        xf=result[nfine][0];
        yf=result[nfine][1];

        //"fix" it:
	hemi=1;
        tcoord_fix(xf, yf, hemi);

//      printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);
        //find the interpolated value in the previous tracer field:
        xyind[0]=x_qgrid->interp(xf);
        xyind[1]=y_qgrid->interp(yf);

        tracer->interpol_coeff(xyind, sub, weight);

        hemi=-(hemi-1)/2;

        row_sub=ind;

        map.add_el(weight[0], row_sub, sub[0]+hemi*np*np);
        map.add_el(weight[1], row_sub, sub[1]+hemi*np*np);
        map.add_el(weight[2], row_sub, sub[2]+hemi*np*np);
        map.add_el(weight[3], row_sub, sub[3]+hemi*np*np);

        //Southern hemisphere:
        //do a Runge-Kutta integration:
        rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs_s);
        //get the final position:
        xf=result[nfine][0];
        yf=result[nfine][1];

	//"fix" it:
	hemi=-1;
	tcoord_fix(xf, yf, hemi);

//	printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);
        //find the interpolated value in the previous tracer field:
        xyind[0]=x_qgrid->interp(xf);
        xyind[1]=y_qgrid->interp(yf);

        tracer->interpol_coeff(xyind, sub, weight);

        hemi=-(hemi-1)/2;

	row_sub=ind+np*np;

        map.add_el(weight[0], row_sub, sub[0]+hemi*np*np);
        map.add_el(weight[1], row_sub, sub[1]+hemi*np*np);
        map.add_el(weight[2], row_sub, sub[2]+hemi*np*np);
        map.add_el(weight[3], row_sub, sub[3]+hemi*np*np);
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
  fclose(nfs);
  fclose(sfs);

  //clean up:
  delete [] tind;
  for (long i=0; i<=nfine; i++) delete [] result[i];
  delete [] result;

  //delete u_n;
  //delete v_n;
  //delete x_grid_n;
  //delete y_grid_n;

  //delete u_s;
  //delete v_s;
  //delete x_grid_n;
  //delete y_grid_n;

  //delete time_grid;

  delete tracer;
  delete x_qgrid;
  delete y_qgrid;

}
