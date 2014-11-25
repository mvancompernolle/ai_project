//
// Copywrite 2004 Peter Mills.  All rights reserved.
//
// Implements a simple two-dimensional trajectory model.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "tcoord_defs.h"
#include "traj_int_obj.h"

#define MAX_LINE_LENGTH 200
#define DATE_WIDTH 30

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
//		2005 sometime PM: upgraded to model2...
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
  char **line;		//one line from input file
  char *fg_res;

  int32_t n;				//number of time steps
  long *nt;
  long ninit;
  double *tstep;				//time step
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string
  int32_t *tind;			//**number of t-steps completed
  double *tind0;
  double tind0min, tind0max;
  double toffs;
  int32_t lt;				//time index as longword

  float ***result;			//final conditions
  short **hemi;
  
  double checkint=TSTEP_COARSE;		//interval (in indexed time coords)
				//to check for switching hemispheres
  int32_t checksteps;
  int32_t nt_coarse;			//number of coarse steps
  int32_t ss, srem;

  traj_int_obj *Ntraj;			//nicely encapsulates the whole process
  traj_int_obj *Straj;
  traj_int_obj *curtraj;

  FILE *initfun;			//initialization file unit
  FILE *outfun;				//output file unit

  float lon, lat;

  //read from the command line:
  int32_t twid=TFIELD_WIDTH;
  int32_t nrk=TSTEP_NFINE;
  double ind1;
  time_class t0, tf;

  //read from the initialization file:
  float tstep1;
  int32_t nt1;

  int flag[20];
  void *optarg[20];

  //defaults:
  ind1=0;
  n=-1;

  optarg[0]=&checkint;
  optarg[3]=&ind1;
  optarg[4]=&n;
  optarg[5]=&twid;
  optarg[6]=&nrk;

  argc=parse_command_opts(argc, argv, "hif0Ndk?", "%lg%s%s%lg%d%d%d%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "model2: error parsing command line");
  if (flag[1]) t0.read_string((char *) optarg[1]);
  if (flag[2]) tf.read_string((char *) optarg[2]);

  if ((argc < 5 || argc > 6) || flag[7]) {
    FILE *docfs;
    int err;
    if (flag[7]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }

    printf("Runs the trajectory model over one hemisphere for a series of initial conditions\n");
    printf("\n");
    printf("syntax:\n");
    printf("	model2 vfile1 vfile2 infile outfile [checkint]\n");
    printf("\n");
    printf("where:\n");
    printf("    vfile1  Binary file containing N. hemi. velocity field\n");
    printf("    vfile2  Binary file containing S. hemi. velocity field\n");
    printf("    infile  Ascii file containing initial conditions\n");
    printf("            The format of each one-column record is as follows:\n");
    printf("\n");
    printf("             date                    x0 y0 dt nt\n\n");
    printf("             date is a fixed width (%d) field and has the following format:\n", DATE_WIDTH);
    printf("                  year/month/day-hour:minute:second\n");
    printf("\n");
    printf("             x0   floating point fields giving the initial position\n");
    printf("             y0   \n");
    printf("\n");
    printf("             dt   is the non-dimensional time step as a fraction of the\n");
    printf("                  time grid spacing of the velocity field.\n");
    printf("\n");
    printf("             nt   is the number of time steps.");
    printf("\n");
    printf("    outfile  Ascii file containing final, integrated trajectories.\n");
    printf("             The time and position at each time\n");
    printf("             step is simply written to a separate line in an ascii file.\n");
    printf("             The trajectories are written in order, meaning there\n");
    printf("             will be nt+1 consecutive lines for each trajectory.\n");
    printf("\n");
    printf("    checkint Interval to check for switching hemispheres\n");
    printf("\n");
    printf("options:\n");
    ctraj_optargs(stdout, "hif0Ndk", 1);
    return 0;
  }

  //read in the velocity field:
  Ntraj=new traj_int_obj(argv[1]);
  Straj=new traj_int_obj(argv[2]);

  //two fields can differ in the location of the time grids, but not
  //spacing:
  date1=Ntraj->get_time(0);
  date2=Straj->get_time(0);
  if (date2>date1) date1=date2;
  toffs=Ntraj->get_tind(date1)-Straj->get_tind(date1);

  if (argc == 6) sscanf(argv[5], "%f", &checkint);

  if (flag[1]) ind1=Ntraj->get_tind(t0);

  //read the initialization file:
  line=read_ascii_all(argv[3], &ninit);

  nt=new long[ninit];
  tind0=new double[ninit];
  tind=new int32_t[ninit];
  tstep=new double[ninit];
  hemi=new short*[ninit];

  result=new float **[ninit];

  //collect initial conditions:
  tind0max=0;
  for (long i=0; i<ninit; i++) {
    line[i][twid]='\0';
    date1.read_string(line[i]);
    date1.write_string(date_str);
    sscanf(line[i]+twid+1, "%f %f %f %d", &lon, &lat, &tstep1, &nt1);
    if (flag[0]==0) tstep[i]=tstep1; else tstep[i]=checkint/nrk;
    if (flag[1]==0 && flag[3]==0) {
      tind0[i]=Ntraj->get_tind(date1);
    } else {
      tind0[i]=ind1;
    }
    if (flag[2]==0 && flag[4]==0) {
      nt[i]=nt1;
    } else if (flag[2]) {
      nt[i]=(Ntraj->get_tind(tf)-tind0[i])/tstep[i]+1;
    } else {
      nt[i]=n*checkint/tstep[i];
      printf("nt[%d]=%d\n", i, nt[i]);
    }
    result[i]=new float *[nt[i]+1];
    result[i][0]=new float[(nt[i]+1)*2];
    for (ind_type j=1; j<=nt[i]; j++) result[i][j]=result[i][0]+2*j;

    hemi[i]=new short[nt[i]+1];
    hemi[i][0]=lat/fabs(lat);
    tcoord2_2lonlat(result[i][0][0], result[i][0][1], -1, hemi[i][0], lon, lat);

    if (tind0[i] < tind0min || i==0) tind0min=tind0[i];
    if (tind0[i]+tstep[i]*nt[i] > tind0max) tind0max=tind0[i]+tstep[i]*nt[i];
    tind[i]=0;
    delete [] line[i];
  }
  delete [] line;

  //perform the integration:
  for (double i=tind0min; i<=tind0max; ) {
    i+=checkint;
    printf("%lg:", i);
    for (long j=0; j<ninit; j++) {
      date2=Ntraj->get_time(tind[j]);
      date2.write_string(date_str);
      //printf(" %s", date_str);
      if (i>tind[j]*tstep[j]+tind0[j] && tind[j]<nt[j]) {
        nt1=ceil((i-tind0[j])/tstep[j])-tind[j];
        if (nt1+tind[j]>nt[j]) nt1=nt[j]-tind[j];

        if (hemi[j][tind[j]] > 0) {
          //do the integration:
          Ntraj->integrate(result[j][tind[j]], result[j]+tind[j], 
			tind0[j]+tstep[j]*tind[j], tstep[j], (long) nt1);
        } else {
          //do the integration:
          Straj->integrate(result[j][tind[j]], result[j]+tind[j], 
			tind0[j]+tstep[j]*tind[j]+toffs, tstep[j], (long) nt1);
        }
	for (int k=1; k<=nt1; k++) hemi[j][tind[j]+k]=hemi[j][tind[j]];
        tcoord_fix(result[j][tind[j]+nt1][0], result[j][tind[j]+nt1][1], 
			hemi[j][tind[j]+nt1]);
        //do the integration:
	tind[j]+=nt1;
        //printf("x");
      }
    }
    printf("\n");
  }

  outfun=fopen(argv[4], "w");

  for (ind_type i=0; i<ninit; i++) {
    //write it to the output file:
    for (long j=0; j<=nt[i]; j++) {
      //compute the date and transform to a pretty ascii format:
      ind1=tind0[i]+j*tstep[i];
      date1=Ntraj->get_time(ind1);
      date1.write_string(date_str);
      tcoord2_2lonlat(result[i][j][0], result[i][j][1], 1, hemi[i][j], lon, lat);
      fprintf(outfun, "%23s %12.4f %12.4f\n", date_str, lon, lat);
    }
    delete [] result[i];
  }

  delete [] result;
  

  //printf("%f\n", nanf(""));

  //close the open file pointers:
  fclose(outfun);

  delete Ntraj;
  delete Straj;
}

