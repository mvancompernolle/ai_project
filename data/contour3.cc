#include <stdio.h>
#include <string.h>

#include "error_codes.h"
#include "parse_command_opts.h"
#include "ctraj_defaults.h"

#include "boundary_swap.h"

#define MAGIC -55
#define PI 3.141592657

//syntax: contour2 initfile
//
//where:
//	initfile:	name of the initialisation file
//
//format of the initialisation file:
//	nfile:	Binary file containing N hemisphere velocity field
//	sfile:	Binary file containing S hemisphere velocity field
//	cfile:	Binary file containing initial contour
//	t0:	Date to start integration
//	tstep:	Time step (coarse)
//	n:	Number of steps
//	nrk:	Number of Runge-Kutta steps between each time step
//	thresh_arc:	Maximum radians of arc between each pair of points
//	min_spac:	Minimum spacing between each pair of points

#define MAXLL 200

int main(int argc, char *argv[]) {
  char *initfile;
  FILE *initfs;

  char line[MAXLL];
  char nfile[MAXLL];
  char sfile[MAXLL];
  char cfile[MAXLL];
  char ofile[MAXLL];
  float maxspac=DSMAX;		//maximum degrees of arc
  float minspac=DSMIN;		//minimum point spacing
  int64_t bs=C3_BLOCK_SIZE;	//64 megs should be a reasonable page size
  float ts=TSTEP_COARSE;
  int32_t n;			//number of coarse time steps
  int32_t nrk=TSTEP_NFINE;	//number of fine time steps
  int32_t npts;
  int32_t nc;			//number of contours in input file
  int32_t magic=MAGIC;
  time_class tcur, t0;
  time_class tf;
  char tstring[100];
  boundary_swap *contour;
  interpol_index ind1, dum;

  int flag[20];
  void *optarg[20];

  //defaults:
  ind1=0;
  n=-1;

  optarg[0]=&bs;
  optarg[1]=&minspac;
  optarg[2]=&maxspac;
  optarg[3]=&ts;
  optarg[4]=&nrk;
  optarg[7]=&ind1;
  optarg[8]=&n;

  argc=parse_command_opts(argc, argv, "Bmshkif0No?", "%ld%g%g%g%d%s%s%lg%d%d%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "contour3: error parsing command line");
  if (flag[5]) t0.read_string((char *) optarg[5]);
  if (flag[6]) tf.read_string((char *) optarg[6]);

  if ((argc != 2 && argc !=5) || flag[10]) {
    FILE *docfs;
    int err;
    if (flag[10]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }

    fprintf(docfs, "For simulating very large contours (several million points):\n");
    fprintf(docfs, "- contour is buffered to a file;\n");
    fprintf(docfs, "- uses a distance criterion for adding and removing points;\n");
    fprintf(docfs, "- original purpose was to check for space-filling of advected contours.\n"); 
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax: contour3 [options] [initfile | nfile sfile cfile ofile]\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "mshkif0NB", 1);
    fprintf(docfs, "\n");
    fprintf(docfs, "where:\n");
    fprintf(docfs, "  initfile is the name of the initialisation file\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "          format of the initialisation file:\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "          line 1:    Binary file containing N hemisphere velocity field\n");
    fprintf(docfs, "          line 2:    Binary file containing S hemisphere velocity field\n");
    fprintf(docfs, "          line 3:    Binary file containing contour\n");
    fprintf(docfs, "          line 4:    Minimum block size of file\n");
    fprintf(docfs, "          line 5:    Date to start integration\n");
    fprintf(docfs, "          line 6:    Time step (coarse)\n");
    fprintf(docfs, "          line 7:    Number of steps\n");
    fprintf(docfs, "          line 8:    Number of Runge-Kutta steps between each time step\n");
    fprintf(docfs, "          line 9:    Minimum spacing between each pair of points\n");
    fprintf(docfs, "          line 10:   Maximum spacing between each pair of points\n");
    fprintf(docfs, "          line 11:   Final output file\n");
    return err;
  }

  //parse the argument list:
  if (argc==2) {
    initfile=argv[1];
    //parse the initialisation file:
    initfs=fopen(initfile, "r");
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", nfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", sfile);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", cfile);
    fgets(line, MAXLL, initfs);
    if (flag[0]==0) sscanf(line, "%ld", &bs);
    fgets(line, MAXLL, initfs);
    if (flag[5]==0 && flag[7]==0) t0=line;
    fgets(line, MAXLL, initfs);
    if (flag[3]==0) sscanf(line, "%g", &ts);
    fgets(line, MAXLL, initfs);
    if (flag[6]==0 && flag[8]==0) sscanf(line, "%d", &n);
    fgets(line, MAXLL, initfs);
    if (flag[4]==0) sscanf(line, "%d", &nrk);
    fgets(line, MAXLL, initfs);
    if (flag[1]==0) sscanf(line, "%g", &minspac);
    fgets(line, MAXLL, initfs);
    if (flag[2]==0) sscanf(line, "%g", &maxspac);
    fgets(line, MAXLL, initfs);
    sscanf(line, "%s", ofile);
  } else {
    strcpy(nfile, argv[1]);
    strcpy(sfile, argv[2]);
    strcpy(cfile, argv[3]);
    strcpy(ofile, argv[4]);
  }

  printf("Time step: %f\n", ts);
  printf("Number of steps: %d\n", n);
  printf("Number of intermediate steps: %d\n", nrk);
  printf("Minimum spacing: %f\n", minspac);
  printf("Maximum spacing: %f\n", maxspac);

  //initialize the contour:
  contour=new boundary_swap(nfile, sfile, cfile, bs);
  if (maxspac <= minspac) maxspac=2*minspac;
  contour->set_parm(ts, nrk, minspac, maxspac);
  //if (oflag) contour->wrap_off();

  //unless set otherwise, we start at the date supplied with the initial
  //contour:
  if (flag[7]==0) ind1=contour->get_tind();

  //figure out integration interval:
  if (flag[5]) ind1=contour->set(t0);
  if (flag[6]) {
    n=(contour->set(tf)-ind1)/ts;
  } else if (n<0) {
    n=(contour->nt(dum)-ind1)/ts;
  }
  
  contour->set(ind1);

  for (long i=0; i<n; i++) {
    tcur=contour->advance();
    //contour->print_current();
    tcur.write_string(tstring);
    printf("%d %s\n", i, tstring);
  }

  printf("%d\n", contour->store(ofile));

  delete contour;

  return 0;

}
