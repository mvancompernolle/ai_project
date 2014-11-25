#include <stdio.h>
#include "error_codes.h"
#include "parse_command_opts.h"
#include "ctraj_defaults.h"
#include "boundary_list.h"

#define MAGIC -55
#define PI 3.141592657

//syntax: contour1 vfile outfile x y r t0 tstep n nrk thresh_arc min_spac
//
//where:
//	vfile:	Binary file containing velocity field
//	outfile:	Binary file to hold results
//	x:	X centre of contour
//	y:	Y centre of contour
//	r:	Radius of contour
//	t0:	Date to start integration
//	tstep:	Time step (coarse)
//	n:	Number of steps
//	nrk:	Number of Runge-Kutta steps between each time step
//	thresh_arc:	Maximum radians of arc between each pair of points
//	min_spac:	Minimum spacing between each pair of points

int main(int argc, char *argv[]) {
  char *vfile;
  char *outfile;
  FILE *outfs;
  float x, y, r;
  float tarc, minspac, maxspac;
  float ts;
  int32_t n, nrk;
  int32_t npts;
  int32_t magic=MAGIC;
  time_class tcur, t0;
  time_class tf;
  char tstring[100];
  boundary_list *contour;

  interpol_index ind1;
  int oflag;

  int flag[20];
  void *optarg[20];

  //defaults:
  ind1=0;
  n=-1;

  optarg[0]=&tarc;
  optarg[1]=&minspac;
  optarg[2]=&maxspac;
  optarg[3]=&ts;
  optarg[4]=&nrk;
  optarg[7]=&ind1;
  optarg[8]=&n;

  argc=parse_command_opts(argc, argv, "cmshkif0No?", "%g%g%g%g%d%s%s%lg%d%%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "contour2: error parsing command line");
  if (flag[5]) t0.read_string((char *) optarg[5]);
  if (flag[6]) tf.read_string((char *) optarg[6]);
  oflag=flag[9];

  if (argc != 3 || flag[10]) {
    FILE *docfs;
    int err;
    if (flag[10]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "Runs the contour advection program over a single hemisphere\n");
    fprintf(docfs, "or on a cartesian plane.\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax: contour1 vfile cfile\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "where:\n");
    fprintf(docfs, "	vfile:		Binary file containing velocity field\n");
    fprintf(docfs, "	cfile:		Binary file containing contours\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "cmshkif0No?", 1);
    return err;
  }

  //parse the argument list:
  vfile=argv[1];
  outfile=argv[2];

  //convert degrees to radians:
  tarc=tarc*PI/180;

  //initialize the contour:
  printf("Opening: %s\n", vfile);
  contour=new boundary_list(vfile);
 
  if (maxspac <= minspac) maxspac=10*minspac;
  contour->set_parm(ts, nrk, tarc, minspac, maxspac);

  outfs=fopen(outfile, "r");
  fread(&magic, sizeof(magic), 1, outfs);
  if (magic!=MAGIC) {
    fprintf(stderr, "contour1: file, %s, is wrong type\n", outfile);
    return FILE_READ_ERROR;
  }
  contour->read(outfs);

  if (oflag) contour->wrap_off();

  //unless set otherwise, we start at the date supplied with the initial
  //contour:
  if (flag[7]==0) ind1=contour->get_tind();

  //figure out integration interval:
  //dates always trump indices...
  if (flag[5]) ind1=contour->set(t0);
  if (flag[6]) {
    n=(contour->set(tf)-ind1)/ts;
  } else if (n<0) {
    n=(contour->nt()-ind1)/ts;
  }

  //set the start time:
  contour->set(ind1);

  //open the output file and write the headers:
  outfs=fopen(outfile, "w");
  fwrite(&magic, sizeof(magic), 1, outfs);
  n++;
  fwrite(&n, sizeof(n), 1, outfs);
//  contour->print(stdout);
  npts=contour->fix();
  contour->write(outfs);

  for (long i=1; i<n; i++) {
    tcur=contour->advance();
    tcur.write_string(tstring);
    printf("%4d %s %d\n", i, tstring, npts);
//    contour->print(stdout);
    contour->write(outfs);
    npts=contour->fix();
  }

  fclose(outfs);

  delete contour;

  return 0;

}
