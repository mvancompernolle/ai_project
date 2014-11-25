//
// Copywrite 2004 Peter Mills.  All rights reserved.
//

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "time_class.h"
#include "peteys_tmpl_lib.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

#include "error_codes.h"
#include "parse_command_opts.h"
#include "ctraj_defaults.h"

#include "tcoord_defs.h"

//global datasets containing the velocity field:
simple<float> *x_grid_n;
simple<float> *y_grid_n;
simple<float> *x_grid_s;
simple<float> *y_grid_s;
//simple<float> *level;

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

#define MAXLL 1000
#define DATE_WIDTH 23

#define RMAX 10000

//syntax:
//tracer4_step1 initfile
//
// where initfile is the initialization file
//
// format of initfile:
//
//nfile		Binary file containing velocity field in the N. hemi.
//sfile		Binary file containing the velocity field in the S. hemi.
//infile	Input file containing initial tracer concentration
//outfile	Output file containing integrated tracer field
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
  char infile[MAXLL];

  //composite dataset containing velocity field:
  composite_dataset ndata;
  composite_dataset sdata;
  FILE *nfs;				//file stream for velocity field
  FILE *sfs;
  long loc, dum;			//location of the datasets (members of vdata)

  //northern and Southern hemisphere time grids:
  simple<time_class> *time_grid_n;
  simple<time_class> *time_grid_s;

  //time step info:
  int32_t n;				//number of time steps
  int32_t nfine=TSTEP_NFINE;		//number of trajectory time steps
  float tstep_fine;			//time step for trajectory calc.
  float tstep=TSTEP_COARSE;		//time step for tracer field

  //general date variables:
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string

  //time indices:
  double *tind_n;			//vector of time indices
  double *tind_s;
  ind_type lt;				//time index as integer 

  //stuff for integration:
  float x0[2];				//initial conditions
  float **result;			//integrated values

  FILE *infun;				//input file unit
  FILE *outfun;				//output file unit

  //tracer field:
  composite_dataset tracer_init;
  composite_dataset tracer_f(NO_DELETE_VARS);
  dependent<float> *q0n, *q0s;		//old tracer field
  //dependent<float> *q1n, *q1s;		//current tracer field
  //dependent<float> *qswap;
  dependent_swap<float> *qn, *qs;	//integrated tracer field
  simple<float> *xqgridn, *xqgrids;	//tracer x grid
  simple<float> *yqgridn, *yqgrids;	//tracer y grid
  simple<time_class> tqgrid;			//time grid for the tracer field
  long qsizen, qsizes;			//number of data elements in one tracer field
  long nxn, nxs, nyn, nys;		//number of grids
  float val;				//interpolated value

  //for checking the consistancy of the two files:
  simple<float> *zgrid;
  simple<float> *check2;

  //intermediate values:
  long ind, k;

  double tind1=0;
  int64_t page_size;

  int flag[20];
  void *optarg[20];

  optarg[0]=&tstep;
  optarg[1]=&nfine;
  optarg[4]=&page_size;
  optarg[5]=&tind1;
  optarg[6]=&n;

  argc=parse_command_opts(argc, argv, "hkifB0N?", "%g%d%s%s%ld%lg%d%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "tracer5: error parsing command line");
  if (flag[2]) date1.read_string((char *) optarg[2]);
  if (flag[3]) date2.read_string((char *) optarg[3]);
//  qflag=flag[0];

  if ((argc != 2 && argc != 5) || flag[7]) {
    FILE *docfs;
    int err;
    if (flag[7]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "Purpose: two-dimensional, 'semi-Lagrangian' tracer simulation driven \n");
    fprintf(docfs, "by globally gridded wind fields.\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax:\n");
    fprintf(docfs, "tracer5 [initfile | vfield_N vfield_S infile outfile]\n");
    fprintf(docfs, "\n");
    fprintf(docfs, " where initfile is the initialization file\n");
    fprintf(docfs, "\n");
    fprintf(docfs, " format of initfile:\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "line 1:	Binary file containing velocity field in the N. hemi.\n");
    fprintf(docfs, "line 2:	Binary file containing the velocity field in the S. hemi.\n");
    fprintf(docfs, "line 3:	Input file containing initial tracer concentration\n");
    fprintf(docfs, "line 4:	Output file for storing integrated tracer field\n");
    fprintf(docfs, "line 5:	Date to start integration\n");
    fprintf(docfs, "line 6:	Number of (Eulerian) time steps\n");
    fprintf(docfs, "line 7:	Non-dimensional time (Eulerian, i.e. coarse) step\n");
    fprintf(docfs, "line 8:	Number of Runge-Kutta steps between each time step\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "hkifB0N?");
    return err;
  }

  if (argc == 2) {
    initfile=argv[1];

    //parse the initialisation file:
    initfs=fopen(initfile, "r");
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", nfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", sfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", infile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", outfile);
    fgets(line, MAXLL, initfs);
    if (flag[2]==0 && flag[5]==0) date1=line;
    fgets(line, MAXLL, initfs);
    if (flag[0]==0) sscanf(line, "%g", &tstep);
    fgets(line, MAXLL, initfs);
    if (flag[3]==0 && flag[6]==0) sscanf(line, "%d", &n);
    fgets(line, MAXLL, initfs);
    if (flag[1]==0) sscanf(line, "%d", &nfine);
  } else {
    strcpy(nfile, argv[1]);
    strcpy(sfile, argv[2]);
    strcpy(infile, argv[3]);
    strcpy(outfile, argv[3]);
  }

  printf("tstep: %f, number of time steps: %d, Runge-Kutta steps: %d\n", tstep, n, nfine);

  //get the N. hemi. velocity fields:
  nfs=fopen(nfile, "r");
  ndata.read(nfs);
  loc=ndata.search_var("xgrid", dum);
  x_grid_n=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("ygrid", dum);
  y_grid_n=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("time", dum);
  time_grid_n=(simple<time_class> *) ndata.get_var(loc);
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
  loc=sdata.search_var("time", dum);
  time_grid_s=(simple<time_class> *) sdata.get_var(loc);
  loc=sdata.search_var("u", dum);
  u_s=(dependent_swap<float> *) sdata.get_var(loc);
  loc=sdata.search_var("v", dum);
  v_s=(dependent_swap<float> *) sdata.get_var(loc);

  //check the consistency of the two fields:
  loc=ndata.search_var("zgrid", dum);
  zgrid=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("zgrid", dum);
  check2=(simple<float> *) sdata.get_var(loc);
  assert(*zgrid == *check2);

  if (flag[4]) {
    u_n->set_page_size(page_size, 1);
    v_n->set_page_size(page_size, 1);
    u_s->set_page_size(page_size, 1);
    v_s->set_page_size(page_size, 1);
  }

  //figure out the initial time index:
  tind_n=new double[n+1];
  tind_s=new double[n+1];
  //figure out its location in relation to the velocity field:
  if (flag[5]==0 || (flag[2]==0 && argc==5)) {
    time_grid_n->get(date1, tind1);
  } 

  tind_n[0]=time_grid_n->interp(date1);
  tind_s[0]=time_grid_s->interp(date1);

  if (flag[3]) {
    n=(time_grid_n->interp(date2)-time_grid_n->interp(date1))/tstep+1;
  } else if (flag[3]==0 && flag[6]==0 && argc==5) {
    ind_type ntemp;
    n=(time_grid_n->nel()-tind_n[0])/tstep+1;
    ntemp=(time_grid_s->nel()-tind_s[0])/tstep+1;
    if (ntemp < n) n=ntemp;
  }

  tstep_fine=-tstep/nfine;

  tqgrid.add_el(date1);

  for (long i=1; i<=n; i++) {
    tind_n[i]=tind_n[i-1]+tstep;
    tind_s[i]=tind_s[i-1]+tstep;

    time_grid_n->get(date2, tind_n[i]);
    tqgrid.add_el(date2);

  }

  //open the input file and get the initial tracer concentration
  //and the tracer gridding:
  infun=fopen(infile, "r");
  tracer_init.read(infun);
  loc=tracer_init.search_var("xn", dum);
  xqgridn=(simple<float> *) tracer_init.get_var(loc);
  loc=tracer_init.search_var("yn", dum);
  yqgridn=(simple<float> *) tracer_init.get_var(loc);
  loc=tracer_init.search_var("xs", dum);
  xqgrids=(simple<float> *) tracer_init.get_var(loc);
  loc=tracer_init.search_var("ys", dum);
  yqgrids=(simple<float> *) tracer_init.get_var(loc);
  loc=tracer_init.search_var("qn", dum);
  q0n=(dependent<float> *) tracer_init.get_var(loc);
  loc=tracer_init.search_var("qs", dum);
  q0s=(dependent<float> *) tracer_init.get_var(loc);

  //close the input file
  fclose(infun);

  //get the size of each of the tracer fields (North and South):
  qsizen=q0n->nel();
  qsizes=q0s->nel();

  //initialize the output tracer fields:
  qn=new dependent_swap<float>((simple_dataset *) xqgridn, (simple_dataset *) yqgridn, 
		  (simple_dataset *) zgrid, (simple_dataset *) &tqgrid);
  qs=new dependent_swap<float>((simple_dataset *) xqgrids, (simple_dataset *) yqgrids, 
		  (simple_dataset *) zgrid, (simple_dataset *) &tqgrid);
  if (flag[4]) {
    qn->set_page_size(page_size, 1);
    qs->set_page_size(page_size, 1);
  }

  loc=tracer_f.add_var("xn");
  tracer_f.cvar(loc, (dataset *) xqgridn);
  loc=tracer_f.add_var("yn");
  tracer_f.cvar(loc, (dataset *) yqgridn);
  loc=tracer_f.add_var("xs");
  tracer_f.cvar(loc, (dataset *) xqgrids);
  loc=tracer_f.add_var("ys");
  tracer_f.cvar(loc, (dataset *) yqgrids);
  loc=tracer_f.add_var("z");
  tracer_f.cvar(loc, (dataset *) zgrid);
  loc=tracer_f.add_var("t");
  tracer_f.cvar(loc, (dataset *) & tqgrid);
  loc=tracer_f.add_var("qn");
  tracer_f.cvar(loc, (dataset *) qn);
  loc=tracer_f.add_var("qs");
  tracer_f.cvar(loc, (dataset *) qs);
  
  //open up the output file and write the results:
  outfun=fopen(outfile, "w+");
  tracer_f.write(outfun);

  //initialize the vector of results for the Runge-Kutta integrations:
  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  //get the number of grids for each tracer field:
  nxn=xqgridn->nel();
  nyn=yqgridn->nel();
  nxs=xqgrids->nel();
  nys=yqgrids->nel();

  //load the initial tracer field:
  //printf("Loading up initial tracer field\n");
  for (ind_type i=0; i<nxn; i++) {
    for (ind_type j=0; j<nyn; j++) {
      q0n->get(val, i, j);
      //printf("%f ", val);
      qn->cel(val, i, j, 0, 0);
    }
    //printf("\n");
  }
  for (ind_type i=0; i<nxs; i++) {
    for (ind_type j=0; j<nys; j++) {
      q0s->get(val, i, j);
      qs->cel(val, i, j, 0, 0);
    }
  }
  //printf("Initial tracer field loaded\n");

  for (ind_type it=1; it<=n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    interpol_index xind, yind;	//interpolation indices
    short hemi;			//hemisphere

    for (ind_type i=0; i<nxn; i++) {
      for (ind_type j=0; j<nyn; j++) {
        //use the current grid point as the initial condition:
        xqgridn->get(x0[0], i);
        yqgridn->get(x0[1], j);
//	printf("(%f, %f)\n", x0[0], x0[1]);

        //Northern hemisphere:
        //do a Runge-Kutta integration:
        rk_dumb((float) tind_n[it], x0, 2L, tstep_fine, nfine, result, &derivs_n);
        //get the final position:
        xf=result[nfine][0];
        yf=result[nfine][1];

	//"fix" it:
	hemi=1;
	tcoord_fix(xf, yf, hemi);

	//printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);

	if (hemi == 1) {
          //find the interpolated value in the previous tracer field:
          xind=xqgridn->interp(xf);
          yind=yqgridn->interp(yf);
          qn->interpol(val, xind, yind, 0, it-1);
	  qn->cel(val, i, j, 0, it);
	} else {
          //find the interpolated value in the previous tracer field:
          xind=xqgrids->interp(xf);
          yind=yqgrids->interp(yf);
          qs->interpol(val, xind, yind, 0, it-1);
	  qn->cel(val, i, j, 0, it);
	}
	//printf("%f ", val);
      }
      //printf("\n");
    }

    for (ind_type i=0; i<nxs; i++) {
      for (ind_type j=0; j<nys; j++) {
        //use the current grid point as the initial condition:
        xqgrids->get(x0[0], i);
        yqgrids->get(x0[1], j);

        //Southern hemisphere:
        //do a Runge-Kutta integration:
        rk_dumb((float) tind_s[it], x0, 2L, tstep_fine, nfine, result, &derivs_s);
        //get the final position:
        xf=result[nfine][0];
        yf=result[nfine][1];

	//"fix" it:
	hemi=-1;
	tcoord_fix(xf, yf, hemi);

//	printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);

	if (hemi == 1) {
          //find the interpolated value in the previous tracer field:
          xind=xqgridn->interp(xf);
          yind=yqgridn->interp(yf);
          qn->interpol(val, xind, yind, 0, it-1);
          qs->cel(val, i, j, 0, it);
        } else {
          //find the interpolated value in the previous tracer field:
          xind=xqgrids->interp(xf);
          yind=yqgrids->interp(yf);
          qs->interpol(val, xind, yind, 0, it-1);
          qs->cel(val, i, j, 0, it);
        }

      }
    }

    //get the date:
    tqgrid.get(date2, it);
    date2.write_string(date_str);

    printf("%d %s\n", it, date_str);

  }

  //clean up:
  delete qn;
  delete qs;

  fclose(outfun);
  fclose(nfs);
  fclose(sfs);

  //clean up:
  delete [] tind_n;
  delete [] tind_s;
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

  //delete x_qgrid;
  //delete y_qgrid;

}
