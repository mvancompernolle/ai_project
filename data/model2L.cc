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
//#include <simple_temp.h>
//#include <dependent_swap.h>
//#include <composite_dataset.h>

#include "tcoord_defs.h"
#include "traj_int_obj.h"

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
  char line[MAX_LINE_LENGTH];		//one line from input file
  char *fg_res;

  int32_t n;				//number of time steps
  double tstep;				//time step
  time_class date1, date2, date3;		//a date
  char date_str[DATE_WIDTH];		//a date as a string
  double tind;				//time index
  double tind0n, tind0s;
  int32_t lt;				//time index as longword

  float x0[2];				//initial conditions
  float xf[2];				//final conditions
  short hemi, hemi1;
  
  float checkint;			//interval (in indexed time coords)
				//to check for switching hemispheres
  int32_t checksteps;
  int32_t nt_coarse;			//number of coarse steps
  int32_t ss, srem;

  traj_int_obj *ntraj;			//nicely encapsulates the whole process
  traj_int_obj *straj;
  traj_int_obj *curtraj;

  FILE *initfun;			//initialization file unit
  FILE *outfun;				//output file unit
  FILE *outfun2;			//output file unit

  float lon, lat;

  double hmatrix[4];

  double cv1, cv2, cv3, cv4;
  double c, sqrt_det;
  double lyap1, lyap2;

  if (argc < 6 || argc > 7) {
    printf("Runs the trajectory model over one hemisphere for a series of initial conditions\n");
    printf("Outputs integrated tangent-model (H-matrix)\n");
    printf("\n");
    printf("syntax:\n");
    printf("	model2 vfile1 vfile2 infile outfile1 outfile2 [checkint]\n");
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
    printf("    outfile1 Ascii file containing final integrated trajectories\n");
    printf("\n");
    printf("             The time and position at each time\n");
    printf("             step is simply written to a separate line in an ascii file.\n");
    printf("             The trajectories are written in order, meaning there\n");
    printf("             will be nt+1 consecutive lines for each trajectory.\n");
    printf("\n");
    printf("    outfile2 Ascii file containing integrated H matrix\n");
    printf("\n");
    printf("    checkint Interval to check for switching hemispheres\n");
    printf("\n");
    return 0;
  }

  //read in the velocity field:
  ntraj=new traj_int_obj(argv[1]);
  straj=new traj_int_obj(argv[2]);

  if (argc == 7) sscanf(argv[6], "%f", &checkint); else checkint=4;

  initfun=fopen(argv[3], "r");
  outfun=fopen(argv[4], "w");
  outfun2=fopen(argv[5], "w");
  while (!feof(initfun)) {
    //read in the initial conditions from the input file:
    fg_res=fgets(line, MAX_LINE_LENGTH, initfun);
    if (fg_res==NULL) break;
    //printf("%s\n", line);
    line[DATE_WIDTH]='\0';
    date1.read_string(line);
    date1.write_string(date_str);
    printf("%s\n", date_str);
    sscanf(&line[DATE_WIDTH+1], "%f %f %lg %d", &lon, &lat, &tstep, &n);

    //convert to transformed coordinate system:
    tcoord2_2lonlat(x0[0], x0[1], -1, hemi, lon, lat);

    //number of "coarse" time steps:
    checksteps=checkint/tstep;
    nt_coarse=n/checksteps;

    //figure out the initial time index:
    tind0n=ntraj->get_tind(date1);
    tind0s=straj->get_tind(date1);

    //printf("T0: %s, tstep: %f, nt: %d, ti0: %f\n", date_str, tstep, n, tind0);
    //printf("Number of check steps=%d\n", checksteps);

    hmatrix[0]=1;
    hmatrix[1]=0;
    hmatrix[2]=0;
    hmatrix[3]=1;

    fprintf(outfun, "%s %10.6g %10.6g\n", date_str, lon, lat);
    fprintf(outfun2, "%s %g %g %g %g\n", date_str, hmatrix[0], hmatrix[1], hmatrix[2], hmatrix[3]);
    for (int32_t i=0; i<nt_coarse; i++) {
      if (hemi == 1) {
        //do the integration:
        ntraj->integrate(x0, xf, tind0n+i*checksteps*tstep, tstep, checksteps);
        ntraj->integrate_Hmatrix(hmatrix);
        //write it to the output file:
        ntraj->print_result(outfun, hemi);
        date1=ntraj->get_time(tind0n+(i+1)*checksteps*tstep);
        date1.write_string(date_str);
        fprintf(outfun2, "%s %g %g %g %g\n", date_str, 
		hmatrix[0], hmatrix[1], hmatrix[2], hmatrix[3]);
      } else {
        //do the integration:
        straj->integrate(x0, xf, tind0s+i*checksteps*tstep, tstep, checksteps);
        straj->integrate_Hmatrix(hmatrix);
        //write it to the output file:
        straj->print_result(outfun, hemi);
        date1=straj->get_time(tind0s+(i+1)*checksteps*tstep);
        date1.write_string(date_str);
        fprintf(outfun2, "%s %g %g %g %g\n", date_str, 
		hmatrix[0], hmatrix[1], hmatrix[2], hmatrix[3]);
      }
      tcoord_fixH(xf[0], xf[1], hemi, hmatrix);
      x0[0]=xf[0];
      x0[1]=xf[1];
    }
    //integrage the remaining steps:
    ss=nt_coarse*checksteps;
    srem=n-ss;
    if (srem != 0) {
      if (hemi == 1) {
        //do the integration:
        ntraj->integrate(x0, xf, tind0n+ss*tstep, tstep, srem);
        ntraj->integrate_Hmatrix(hmatrix);
        //write it to the output file:
        ntraj->print_result(outfun, hemi);
        date1=ntraj->get_time(tind0n+ss*tstep+srem*tstep);
        date1.write_string(date_str);
        fprintf(outfun2, "%s %g %g %g %g\n", date_str, 
		hmatrix[0], hmatrix[1], hmatrix[2], hmatrix[3]);
      } else {
        //do the integration:
        straj->integrate(x0, xf, tind0s+ss*tstep, tstep, srem);
        straj->integrate_Hmatrix(hmatrix);
        //write it to the output file:
        straj->print_result(outfun, hemi);
        date1=straj->get_time(tind0s+ss*tstep+srem*tstep);
        date1.write_string(date_str);
        fprintf(outfun2, "%s %g %g %g %g\n", date_str, 
		hmatrix[0], hmatrix[1], hmatrix[2], hmatrix[3]);
      }
    }
    //what the hell, let's calculate the Lyapunov exponents while
    //we're at it:
    cv1=hmatrix[0]*hmatrix[0]+hmatrix[1]*hmatrix[1];
    cv2=hmatrix[0]*hmatrix[2]+hmatrix[1]*hmatrix[3];
    cv4=hmatrix[2]*hmatrix[2]+hmatrix[3]*hmatrix[3];
    c=cv1+cv4;
    sqrt_det=sqrt(c*c-4*(cv1*cv4-cv2*cv2));
    lyap1=log((c+sqrt_det)/2.)/n/2.;
    lyap2=log(fabs(c-sqrt_det)/2.)/n/2.;
    fprintf(outfun2, "%g %g\n", lyap1, lyap2);

  }

  //printf("%f\n", nanf(""));

  //close the open file pointers:
  fclose(initfun);
  fclose(outfun);

  delete ntraj;
  delete straj;
}

