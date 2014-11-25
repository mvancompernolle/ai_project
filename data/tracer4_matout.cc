//
// Copywrite 2004 Peter Mills.  All rights reserved.
//

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
//#include <getopt.h>
#include <stdlib.h>

#include "error_codes.h"

#include "time_class.h"
#include "peteys_tmpl_lib.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"
#include "parse_command_opts.h"

#include "dependent_intc.h"

#include "tcoord_defs.h"
#include "coordtran.h"
#include "ctraj_defaults.h"

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

  printf("1 %g (%g, %g): (%g, %g)\n", tind, xvec[0], xvec[1], dxdt[0], dxdt[1]);

}

//southern hemisphere derivatives:
void derivs_s(float tind, float xvec[], float dxdt[], long n) {
  interpol_index xint, yint;

  xint=x_grid_s->interp(xvec[0]);
  yint=y_grid_s->interp(xvec[1]);
  //printf("%f %f\n", xint, yint);
  u_s->interpol(dxdt[0], xint, yint, 0, (interpol_index) tind);
  v_s->interpol(dxdt[1], xint, yint, 0, (interpol_index) tind);

  printf("0 %g (%g, %g): (%g, %g)\n", tind, xvec[0], xvec[1], dxdt[0], dxdt[1]);

}

#define MAXLL 200
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
//outfile	Output file
//mapfile	File containing the mapping from the two tracer fields to a vector
//t0		Date to start integration
//nt		Number of (Eulerian) time steps
//tstep		Non-dimensional time (Eulerian, i.e. coarse) step
//nfine		Number of Runge-Kutta steps between each time step
//np		Number of grid points per side

int main(int argc, char *argv[]) {
  char *initfile=NULL;
  FILE *initfs;
  char line[MAXLL];

  //main data file names:
  char nfile[MAXLL];
  char sfile[MAXLL];
  char outfile[MAXLL];

  char c;
  size_t ncon;

  //composite dataset containing velocity field:
  composite_dataset ndata;
  composite_dataset sdata;
  FILE *nfs;				//file stream for velocity field
  FILE *sfs;
  long loc, dum;			//location of the datasets (members of vdata)

  //time step info:
  ind_type n;				//number of time steps
  int32_t nfine=TSTEP_NFINE;		//number of trajectory time steps
  float tstep_fine;			//time step for trajectory calc.
  float tstep=TSTEP_COARSE;		//time step for tracer field

  //general date variables:
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string

  //time indices:
  double tind1, tind2;
  double *tind;				//vector of time indices
  ind_type lt;				//time index as integer

  //stuff for integration:
  float x0[2];				//initial conditions
  float **result;			//integrated values

  FILE *outfun;				//output file unit

  //grid info:
  ind_type np=NGRID_Q;			//number of x and y tracer grids
  float sidelength=SIDELENGTH_Q;	//actually sidelength/2...

  float *qgrid;
  simple<float> *x_qgrid;		//tracer x grid
  simple<float> *y_qgrid;		//tracer y grid

  //dummy tracer field for calculating interpolation coefficients:
  dependent_intc *tracer;

  //sparse matrix class instance for holding the matrix of interpolation coeffs:
  sparse_matrix map;

  //mapping from tracer field to vector to be held in sparse matrix:
  sub_1d_type nmap;
  dependent<sub_1d_type> *map_map;
  char mapfile[MAXLL]="mapout.txt";
  FILE *mfs;

  //for checking the consistancy of the two files:
  simple<float> *check1;
  simple<float> *check2;
  simple<time_class> *check3;

  //intermediate values:
  float r;	//radius
  float dx;
  sub_1d_type ind, k;
  sub_1d_type ind0, ind1, ind2, ind3;

  int64_t page_size;		//page size for velocity fields

  int qflag=0;		//just print out the dates...

  int flag[20];
  void *optarg[20];

  tind1=0;

  optarg[1]=&sidelength;
  optarg[2]=&np;
  optarg[3]=&tstep;
  optarg[4]=&nfine;
  optarg[7]=&page_size;
  optarg[8]=&tind1;
  optarg[9]=&n;

  argc=parse_command_opts(argc, argv, "QrnhkifB0N?", "%%g%d%g%d%s%s%ld%lg%d%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "tracer4_matout: error parsing command line");
  if (flag[5]) date1.read_string((char *) optarg[5]);
  if (flag[6]) date2.read_string((char *) optarg[6]);
  qflag=flag[0];

  if ((argc != 2 && argc != 4 && argc != 3) || flag[10]) {
    FILE *docfs;
    int err;
    if (flag[10]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }

    fprintf(docfs, "Purpose: two-dimensional, 'semi-Lagrangian' tracer simulation driven \n");
    fprintf(docfs, "by globally gridded wind fields.  For each time step, outputs a sparse \n");
    fprintf(docfs, "matrix defining the mapping from one tracer field to the next\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax:\n");
    fprintf(docfs, "tracer4_matout [-Q] [-r sidelengh/2] [-n ngrid] [-h dt] [-k nfine]\n");
    fprintf(docfs, "               {initfile | vfield_N vfield_S t0 tf outfile}\n");
    fprintf(docfs, "\n");
    fprintf(docfs, " where:\n");
    fprintf(docfs, "   vfield_N  is the Northern hemisphere velocity field file\n");
    fprintf(docfs, "   vfield_S  is the Southern hemisphere velocity field file\n");
    fprintf(docfs, "   outfile   is the binary output file\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "Qrnhkif0NB?", 1);
    fprintf(docfs, "\n");
    fprintf(docfs, " format of initfile:\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "line 1:	Binary file containing velocity field in the N. hemi.\n");
    fprintf(docfs, "line 2:	Binary file containing the velocity field in the S. hemi.\n");
    fprintf(docfs, "line 3:	Output file\n");
    fprintf(docfs, "line 4:	File in which to write the mapping from the two tracer fields to a vector\n");
    fprintf(docfs, "line 5:	Date to start integration\n");
    fprintf(docfs, "line 6:	Number of (Eulerian) time steps\n");
    fprintf(docfs, "line 7:	Non-dimensional time (Eulerian, i.e. coarse) step\n");
    fprintf(docfs, "line 8:	Number of Runge-Kutta steps between each time step\n");
    fprintf(docfs, "line 9:	Number of grid points per side\n");
    return err;
  }

  if (argc == 3) {
    strcpy(nfile, argv[1]);
    strcpy(sfile, argv[2]);
  } else if (argc == 4) {
    strcpy(nfile, argv[1]);
    strcpy(sfile, argv[2]);
    strcpy(outfile, argv[3]);
  } else {
    initfile=argv[1];
    //parse the initialisation file:
    initfs=fopen(initfile, "r");
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", nfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", sfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", outfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", mapfile);
    fgets(line, MAXLL, initfs);
    if (flag[5]==0 && flag[8]==0) date1=line;
    fgets(line, MAXLL, initfs);
    if (flag[3]==0) sscanf(line, "%g", &tstep);
    fgets(line, MAXLL, initfs);
    if (flag[6]==0 && flag[9]==0) sscanf(line, "%d", &n);
    fgets(line, MAXLL, initfs);
    if (flag[4]==0) sscanf(line, "%d", &nfine);
    fgets(line, MAXLL, initfs);
    if (flag[2]==0) sscanf(line, "%d", &np);
  }

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

  if (flag[7]) {
    u_n->set_page_size(page_size, 1);
    v_n->set_page_size(page_size, 1);
    u_s->set_page_size(page_size, 1);
    v_s->set_page_size(page_size, 1);
  }

  //figure out the initial time index:
  if (flag[5] || argc==2) {
    tind1=time_grid->interp(date1);
  }
  if (flag[6]) {
    tind2=time_grid->interp(date2);
    n=(tind2-tind1)/tstep+1;
  } else if (flag[9]==0 && argc!=2) {
    n=(time_grid->nel()-tind1)/tstep+1;
  }

  //printf("tstep: %f, number of time steps: %d, Runge-Kutta steps: %d, grid size: %d\n",
  //		tstep, n, nfine, np);

  tind=new double[n+1];
  //figure out its location in relation to the velocity field:
  tind[0]=tind1;
  tstep_fine=-tstep/nfine;

  for (long i=1; i<=n; i++) tind[i]=tind[i-1]+tstep;

  //initialize the dummy tracer field:
  //tracer gridding:
  /* really, what the fuck IS this shit???
  qgrid=new float [np];
  dx=2.0*SIDELENGTH_Q/(np-4.0);
  maxr=SIDELENGTH_Q+sqrt(2)*dx;
  for (long i=0; i<np; i++) {
    qgrid[i]=dx*(i-1.5)-RMAX;
  }
  x_qgrid=new simple<float>(qgrid, np, 0);
  y_qgrid=new simple<float>(qgrid, np, 0);*/

  x_qgrid=new simple<float>(-sidelength, sidelength, np);
  y_qgrid=new simple<float>(-sidelength, sidelength, np);
  //x_qgrid->print();

  tracer=new dependent_intc(x_qgrid, y_qgrid);

  map_map=new dependent<sub_1d_type>(x_qgrid, y_qgrid);

  nocorner_map(x_qgrid, y_qgrid, map_map, nmap);

  //generate the mapping of a hemispherical tracer field to a vector
  //(omitting everything below the equator):
  mfs=fopen(mapfile, "w");
  for (ind_type i=0; i<np; i++) {
    for (ind_type j=0; j<np; j++) {
      float x, y;
      map_map->get(ind, i, j);
      x_qgrid->get(x, i);
      y_qgrid->get(y, j);
      fprintf(mfs, "%8d %8d %8d %12.4f %12.4f\n", i, j, ind, x, y);
    }
  }
  fclose(mfs);

  //initialize the sparse matrix:
  map.extend(nmap*8);

  //initialize the vector of results for the Runge-Kutta integrations:
  result=new float * [nfine+1];
  for (long i=0; i<=nfine; i++) result[i]=new float[2];

  //open the output file and write the headers:
  if (qflag==0) outfun=fopen(outfile, "w");
//  fwrite(&maxr, 1, sizeof(maxr), outfun);
//  fwrite(&np, 1, sizeof(np), outfun);
//  fwrite(&n, 1, sizeof(n), outfun);

  for (ind_type it=0; it<n; it++) {
    float x0[2];		//initial cond. for traj.
    float xf, yf;		//final cond.
    float val;			//interpolated value
    interpol_index xyind[2];	//interpolation indices
    sub_1d_type row_sub;
    sub_1d_type sub[4];		//1d subscripts
    double weight[4];		//interpolation coeffs
    short hemi;			//hemisphere

    //get the date:
    lt=(ind_type) tind[it];
    time_grid->get(date1, lt);
    time_grid->get(date2, lt+1);
    date3=date2-date1;
    date2=date3*(tind[it]-(float) lt);
    date3=date1+date2;
    date3.write_string(date_str);

    printf("%d %s\n", it, date_str);

    if (qflag) continue;

    map.reset(nmap*2, nmap*2);

    for (ind_type j=0; j<np; j++) {
      for (ind_type i=0; i<np; i++) {
        //check to see that the point falls within the useable regions:
	map_map->get(row_sub, i, j);
	if (row_sub != -1) {
          //printf("%d\n", row_sub);
          //use the current grid point as the initial condition:
          x_qgrid->get(x0[0], i);
          y_qgrid->get(x0[1], j);
//	  printf("(%f, %f)\n", x0[0], x0[1]);

          //Southern hemisphere:
          //do a Runge-Kutta integration:
          rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs_s);
          //get the final position:
          xf=result[nfine][0];
          yf=result[nfine][1];

	  //"fix" it:
	  hemi=-1;
	  tcoord_fix(xf, yf, hemi);
          hemi=(hemi+1)/2;

	  //printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);
          //find the interpolated value in the previous tracer field:
          xyind[0]=x_qgrid->interp(xf);
          xyind[1]=y_qgrid->interp(yf);
	  //printf("(%f, %f)\n", xyind[0], xyind[1]);

          tracer->interpol_coeff(xyind, sub, weight);
	  /*printf("%d %d %d %d\n", sub[0], sub[1], sub[2], sub[3]);
	  printf("(%d %d)\n", sub[0]/np, sub[0] % np);
	  printf("(%d %d)\n", sub[1]/np, sub[1] % np);
	  printf("(%d %d)\n", sub[2]/np, sub[2] % np);
	  printf("(%d %d)\n", sub[3]/np, sub[3] % np);*/
	  //map_map->get(ind0, sub[0]%np, sub[0]/np);
	  //printf("%d\n", ind0);

	  map_map->get_1d(ind0, sub[0]);
	  //printf("%d\n", ind0);
	  map_map->get_1d(ind1, sub[1]);
	  map_map->get_1d(ind2, sub[2]);
	  map_map->get_1d(ind3, sub[3]);

	  assert(ind0 != -1);
	  assert(ind1 != -1);
	  assert(ind2 != -1);
	  assert(ind3 != -1);

          map.add_el(weight[0], row_sub, ind0+hemi*nmap);
          map.add_el(weight[1], row_sub, ind1+hemi*nmap);
          map.add_el(weight[2], row_sub, ind2+hemi*nmap);
          map.add_el(weight[3], row_sub, ind3+hemi*nmap);

          //Northern hemisphere:
          //do a Runge-Kutta integration:
          rk_dumb((float) tind[it+1], x0, 2L, tstep_fine, nfine, result, &derivs_n);
          //get the final position:
          xf=result[nfine][0];
          yf=result[nfine][1];

	  //"fix" it:
	  hemi=1;
	  tcoord_fix(xf, yf, hemi);
	  hemi=(hemi+1)/2;

//	  printf("Initial: (%f, %f); final: (%f, %f)\n", x0[0], x0[1], xf, yf);
          //find the interpolated value in the previous tracer field:
          xyind[0]=x_qgrid->interp(xf);
          xyind[1]=y_qgrid->interp(yf);
	  //printf("(%f, %f)\n", xyind[0], xyind[1]);

          tracer->interpol_coeff(xyind, sub, weight);
	  /*printf("%d %d %d %d\n", sub[0], sub[1], sub[2], sub[3]);
	  printf("(%d %d)\n", sub[0]/np, sub[0] % np);
	  printf("(%d %d)\n", sub[1]/np, sub[1] % np);
	  printf("(%d %d)\n", sub[2]/np, sub[2] % np);
	  printf("(%d %d)\n", sub[3]/np, sub[3] % np);*/

	  row_sub+=nmap;
	  map_map->get_1d(ind0, sub[0]);
	  map_map->get_1d(ind1, sub[1]);
	  map_map->get_1d(ind2, sub[2]);
	  map_map->get_1d(ind3, sub[3]);

	  assert(ind0 != -1);
	  assert(ind1 != -1);
	  assert(ind2 != -1);
	  assert(ind3 != -1);

          map.add_el(weight[0], row_sub, ind0+hemi*nmap);
          map.add_el(weight[1], row_sub, ind1+hemi*nmap);
          map.add_el(weight[2], row_sub, ind2+hemi*nmap);
          map.add_el(weight[3], row_sub, ind3+hemi*nmap);
	}
      }
    }

    //write the date and the tracer field to a file:
//    fwrite(date_str, 1, sizeof(char)*DATE_WIDTH, outfun);
    map.write(outfun);
    //map.print(stdout);

  }

  if (qflag==0) fclose(outfun);
  fclose(nfs);
  fclose(sfs);

  //clean up:
  delete [] tind;
  for (int32_t i=0; i<=nfine; i++) delete [] result[i];
  delete [] result;

  /*
  printf("u_n\n");
  delete u_n;
  printf("v_n\n");
  delete v_n;
  printf("x_grid_n\n");
  delete x_grid_n;
  printf("y_grid_n\n");
  delete y_grid_n;

  printf("u_s\n");
  delete u_s;
  printf("v_s\n");
  delete v_s;
  printf("x_grid_s\n");
  delete x_grid_s;
  printf("y_grid_s\n");
  delete y_grid_s;

  printf("time_grid\n");
  delete time_grid;
  */

  //printf("tracer\n");
  delete tracer;
  //printf("map_map\n");
  delete map_map;

  //printf("x_qgrid\n");
  delete x_qgrid;
  //printf("y_qgrid\n");
  delete y_qgrid;

}
