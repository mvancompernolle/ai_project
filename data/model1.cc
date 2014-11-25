//
// Copywrite 2004 Peter Mills.  All rights reserved.
//
// Implements a simple two-dimensional trajectory model.
//

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <peteys_tmpl_lib.h>
#include <time_class.h>
#include <simple_temp.h>
#include <dependent_swap.h>
#include <composite_dataset.h>

#include "error_codes.h"
#include "parse_command_opts.h"
#include "read_ascii_all.h"
#include "ctraj_defaults.h"

//global datasets containing the velocity field:
simple<float> *x_grid;		//Gridding in the x direction
simple<float> *y_grid;		//Gridding in the y direction
simple<float> *level;		//The vertical level.  Not used.
simple<time_class> *time_grid;	//The time gridding

dependent_swap<float> *u;	//Velocity in the x direction.
dependent_swap<float> *v;	//Velocity in the y direction.

void derivs(float tind, float xvec[], float dxdt[], long n)
// name: 	derivs
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
// 		xvec: (in) The position.  Must be a two element floating
// 		point array.
//
// 		dxdt: (out) A two element floating point array containing
// 		the velocity field at the particular time and location.
//
// 		n: (in) Ignored.  Should be two (2).
// 		
{
  interpol_index xint, yint;

  xint=x_grid->interp(xvec[0]);
  yint=y_grid->interp(xvec[1]);
  u->interpol(dxdt[0], xint, yint, 0., (interpol_index) tind);
  v->interpol(dxdt[1], xint, yint, 0., (interpol_index) tind);

/*
  if (isnan(dxdt[0]) || isnan(dxdt[1])) {
    u->interpol(dxdt[0], xint, yint, 0., (double) tind, 1);
    v->interpol(dxdt[1], xint, yint, 0., (double) tind, 1);

    printf("t= %f; r= (%f %f); v= (%f, %f); int= (%f, %f)\n", tind, xvec[0], xvec[1], dxdt[0], dxdt[1], xint, yint);
    printf("yint new: %f\n", y_grid->interp(xvec[1]));
    return;
  }
  if (isnan(xvec[0]) || isnan(xvec[1])) {
    printf("Position at t= %f: (%f, %f)\n", tind, xvec[0], xvec[1]);
  }
*/

}

#define MAX_LINE_LENGTH 200
#define DATE_WIDTH 23

// syntax:
// model1 vfile initfile outfile
//
// where:
//  	vfile is the binary file containing the velocity field.  Stored in
//  		the format read and written by the "composite_dataset" object.
//
//	initfile is an ascii file containing a list of initial conditions.
//		The initial conditions for each trajectory are stored on
//		a separate line.  The fields are:
//
//		date                    x0 y0 dt nt
//
//		The date is a fixed width field and has the following format:
//		year/month/day-hour:minute:second
//
//		x0 and y0 are floating point fields giving the initial
//		position.
//
//		dt is the non-dimensional time step as a fraction of the
//		time grid spacing of the velocity field.
//
//		nt is the number of time steps.
//
//	outfile is the output file.  The time and position at each time
//		step is simply written to a separate line in an ascii file.
//		The trajectories are written in order, meaning there
//		will be nt+1 consecutive lines for each trajectory.
//
// history:	2004-2-19 PM: formally documented.
// 		- changed file format for velocity fields
// 		- entire velocity field is no longer read in
// 		  (see dependent_swap.cpp)
// 		2004-5-27 PM: updated documentation to reflect changes
//
// bugs:	Entire velocity field is read in.  This may cause problems
// 		once it gets too big.  A better solution would be to only
// 		read in those time grids that are needed or read them in
// 		as they are needed.  The likely final "solution" will be
// 		to code the data structures used to store them so that they
// 		can swap from the disk.
//		--> done
//		However, there is still the problem of longer trajectories:
//		for each trajectory the field will be swapped in and out.
//		If all the trajectories start at the same date, it would be
//		better to run them concurrently.
//
// 		No syntax or range checking.  GIGO.

int main(int argc, char *argv[]) {
  char **line;		//the whole bloody thing from input file

  long ntraj;				//number of trajectories
  ind_type *nt;				//number of time steps
  ind_type nt1;				//to read it in
  float *tstep;
  float tstep1;				//time step
  time_class date1, date2, date3;		//a date
  int32_t twid=TFIELD_WIDTH;
  char date_str[30];		//a date as a string
  double *tind0;			//initial time index
  double *tind;				//time index
  double tind0min, tind0max;		//we use a rather convoluted method
  					//to do the integration...
  ind_type lt;				//time index as longword

  float **x0;			//initial conditions
  float ***result;			//integrated values

  FILE *vfun;				//velocity field file stream
  composite_dataset vdata;		//all of the velocity field data
  long loc, dum;			//used for searching on variables

  FILE *outfun;				//output file unit

  //for command-line arguments:
  time_class t0, tf;
  double ind1;
  float tstep_coarse=TSTEP_COARSE;
  ind_type n;			//# of t-steps read from the command line
  //int32_t nrk=TSTEP_NFINE;
  //

  int flag[20];
  void *optarg[20];

  //defaults:
  ind1=0;
  n=-1;

  optarg[0]=&tstep_coarse;
  optarg[3]=&ind1;
  optarg[4]=&n;
  optarg[5]=&twid;

  argc=parse_command_opts(argc, argv, "hif0Nd?", "%g%s%s%lg%d%d%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "model1: error parsing command line");
  if (flag[1]) t0.read_string((char *) optarg[1]);
  if (flag[2]) tf.read_string((char *) optarg[2]);

  if (argc != 4 || flag[6]) {
    FILE *docfs;
    int err;
    if (flag[7]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }

    fprintf(docfs, "Runs the trajectory model over one hemisphere for a series of initial conditions\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax:\n");
    fprintf(docfs, "	model1 vfile1 infile outfile\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "where:\n");
    fprintf(docfs, "    vfile   Binary file containing velocity field\n");
    fprintf(docfs, "    infile  Ascii file containing initial conditions\n");
    fprintf(docfs, "            The format of each one-column record is as follows:\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "             date                    x0 y0 dt nt\n\n");
    fprintf(docfs, "             date is a fixed width (%d) field and has the following format:\n", DATE_WIDTH);
    fprintf(docfs, "                  year/month/day-hour:minute:second\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "             x0   floating point fields giving the initial position\n");
    fprintf(docfs, "             y0   \n");
    fprintf(docfs, "\n");
    fprintf(docfs, "             dt   is the non-dimensional time step as a fraction of the\n");
    fprintf(docfs, "                  time grid spacing of the velocity field.\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "             nt   is the number of time steps.");
    fprintf(docfs, "\n");
    fprintf(docfs, "    outfile  Ascii file containing final, integrated trajectories.\n");
    fprintf(docfs, "             The time and position at each time\n");
    fprintf(docfs, "             step is simply written to a separate line in an ascii file.\n");
    fprintf(docfs, "             The trajectories are written in order, meaning there\n");
    fprintf(docfs, "             will be nt+1 consecutive lines for each trajectory.\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "    checkint Interval to check for switching hemispheres\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "hif0Nd", 1);
    return err;
  }

  //read in the velocity field:
  vfun=fopen(argv[1], "r");
  vdata.read(vfun);

  //get the x grid:
  loc=vdata.search_var("xgrid", dum);
  x_grid=(simple<float> *) vdata.get_var(loc);
  //get the y grid:
  loc=vdata.search_var("ygrid", dum);
  y_grid=(simple<float> *) vdata.get_var(loc);
  //get the time grid:
  loc=vdata.search_var("time", dum);
  time_grid=(simple<time_class> *) vdata.get_var(loc);
  //get the velocity in the x direction:
  loc=vdata.search_var("u", dum);
  u=(dependent_swap<float> *) vdata.get_var(loc);
  //get the velocity in the y direction:
  loc=vdata.search_var("v", dum);
  v=(dependent_swap<float> *) vdata.get_var(loc);

  if (flag[1]) ind1=time_grid->interp(t0);

  //read the initialization file:
  line=read_ascii_all(argv[2], &ntraj);

  nt=new ind_type[ntraj];
  tind0=new double[ntraj];
  tind=new double[ntraj];
  tstep=new float[ntraj];
  x0=new float*[ntraj];
  x0[0]=new float[ntraj*2];

  result=new float **[ntraj];

  //collect initial conditions:
  tind0max=0;
  for (long i=0; i<ntraj; i++) {
    x0[i]=x0[0]+i*2;
    line[i][twid]='\0';
    date1.read_string(line[i]);
    date1.write_string(date_str);
    sscanf(line[i]+twid+1, "%f %f %f %d", x0[i]+0, x0[i]+1, &tstep1, &nt1);
    if (flag[0]==0) tstep[i]=tstep1; else tstep[i]=tstep_coarse;
    if (flag[1]==0 && flag[3]==0) {
      tind0[i]=time_grid->interp(date1);
    } else {
      tind0[i]=ind1;
    }
    if (flag[2]==0 && flag[4]==0) {
      nt[i]=nt1;
    } else if (flag[2]) {
      nt[i]=(time_grid->interp(tf)-tind0[i])/tstep[i]+1;
    } else {
      nt[i]=n;
    }
    result[i]=new float *[nt[i]+1];
    result[i][0]=new float[(nt[i]+1)*2];
    result[i][0][0]=x0[i][0];
    result[i][0][1]=x0[i][1];
    for (ind_type j=1; j<=nt[i]; j++) result[i][j]=result[i][0]+2*j;
    if (tind0[i] < tind0min || i==0) tind0min=tind0[i];
    if (tind0[i]+tstep[i]*nt[i] > tind0max) tind0max=tind0[i]+tstep[i]*nt[i];
    tind[i]=tind0[i];
  }

  //perform the integration:
  for (ind_type i=floor(tind0min); i<=ceil(tind0max); i++) {
    printf("%d:", i);
    for (long j=0; j<ntraj; j++) {
      time_grid->get(date2, tind[j]);
      date2.write_string(date_str);
      printf(" %s", date_str);
      n=(tind[j]-tind0[j])/tstep[j];
      if (i>tind[j] && n<nt[j]) {
        if (i > tind0[j]+nt[j]*tstep[j]) {
          nt1=ceil((tind0[j]+nt[j]*tstep[j]-tind[j])/tstep[j]);
        } else {
          nt1=ceil((i-tind[j])/tstep[j]);
	}
        //do the integration:
        rk_dumb((float) tind[j], result[j][n], 2L, tstep[j], nt1, 
			result[j]+n, &derivs);
	tind[j]+=tstep[j]*nt1;
        printf("x");
      }
    }
    printf("\n");
  }

  outfun=fopen(argv[3], "w");
  for (ind_type i=0; i<ntraj; i++) {
    //write it to the output file:
    for (long j=0; j<=nt[i]; j++) {
      //compute the date and transform to a pretty ascii format:
      ind1=tind0[i]+j*tstep[i];
      time_grid->get(date1, ind1);
      date1.write_string(date_str);
      fprintf(outfun, "%23s %12.4f %12.4f\n", date_str, result[i][j][0], result[i][j][1]);
    }
    delete [] result[i];
  }

  delete [] result;
  delete [] tind0;
  delete [] tind;
  delete [] tstep;
  delete [] nt;

  //printf("%f\n", nanf(""));

  //close the open file pointers:
  fclose(outfun);
  fclose(vfun);

  //clean up:
  /*
  delete u;
  delete v;
  delete x_grid;
  delete y_grid;
  delete level;
  delete time_grid;
  */

}

