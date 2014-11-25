#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "error_codes.h"
#include "parse_command_opts.h"
#include "ctraj_defaults.h"

#include "boundary_list_g.h"

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
//	infile:	Binary file containing initial contour
//	outfile:	Binary file to hold results
//	t0:	Date to start integration
//	tstep:	Time step (coarse)
//	n:	Number of steps
//	nrk:	Number of Runge-Kutta steps between each time step
//	thresh_arc:	Maximum radians of arc between each pair of points
//	min_spac:	Minimum spacing between each pair of points
//	max_spac:	Maximum spacing between each pair of points

#define MAXLL 200

int main(int argc, char *argv[]) {
  char *initfile;
  FILE *initfs;

  char line[MAXLL];
  char nfile[MAXLL];
  char sfile[MAXLL];
  char infile[MAXLL];
  FILE *infs;
  char outfile[MAXLL];
  FILE *outfs;
  float x, y, r;
  float tarc=MAXARC;		//maximum degrees of arc
  float minspac=DSMIN;		//minimum point spacing
  float maxspac=DSMAX;		//maximum point spacing
  float ts=TSTEP_COARSE;
  int32_t n;			//number of coarse time steps
  int32_t nrec;			//number of records in output file
  int32_t nrk=TSTEP_NFINE;	//number of fine time steps
  int32_t npts;
  int32_t nc;			//number of contours in input file
  int32_t magic=MAGIC;
  time_class tcur, t0;
  time_class tf;
  char tstring[100];
  boundary_list_g *contour;
  size_t nread;
  interpol_index ind1;
  int oflag;
  int32_t maxnpt=-1;
  int32_t writeint=WRITE_INT;

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
  optarg[9]=&maxnpt;
  optarg[10]=&writeint;

  argc=parse_command_opts(argc, argv, "cmshkif0NMOo?", "%g%g%g%g%d%s%s%lg%d%d%d%%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) fprintf(stderr, "contour2: error parsing command line");
  if (flag[5]) t0.read_string((char *) optarg[5]);
  if (flag[6]) tf.read_string((char *) optarg[6]);
  oflag=flag[11];

  if ((argc != 2 && argc != 4) || flag[12]) {
    FILE *docfs;
    int err;
    if (flag[12]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "Runs the contour advection program.\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "syntax: contour2 [-h tstep] [-k nfine] [-c arc] [-m dsmin] [-s dsmax]\n");
    fprintf(docfs, "                 [-i t0] [-f tf] [vfieldN vfieldS cfile | initfile]\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "where:\n");
    fprintf(docfs, "  vfieldN is the velocity field covering the N. hemisphere\n");
    fprintf(docfs, "  vfieldS is the velocity field covering the S. hemisphere\n");
    fprintf(docfs, "  cfile   contains both initial contour and the simulation results\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "cmshk0NifOoM?", 1);
    fprintf(docfs, "\n");
    fprintf(docfs, "  initfile is the name of the initialisation file\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "          format of the initialisation file:\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "          line 1:     Binary file containing N hemisphere velocity field\n");
    fprintf(docfs, "          line 2:     Binary file containing S hemisphere velocity field\n");
    fprintf(docfs, "          line 3:     Binary file containing initial contour\n");
    fprintf(docfs, "          line 4:     Binary file to hold results\n");
    fprintf(docfs, "          line 5:     Date to start integration\n");
    fprintf(docfs, "          line 6:     Time step (coarse)\n");
    fprintf(docfs, "          line 7:     Number of steps\n");
    fprintf(docfs, "          line 8:     Number of Runge-Kutta steps between each time step\n");
    fprintf(docfs, "          line 9:     Maximum radians of arc between each pair of points\n");
    fprintf(docfs, "          line 10:    Minimum spacing between each pair of points\n");
    fprintf(docfs, "          line 11:    Maximum spacing between each pair of points\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "Format of binary file:\n");
    fprintf(docfs, "  - 4 byte 'magic' number\n");
    fprintf(docfs, "  - 4 byte integer header containing number of records\n");
    fprintf(docfs, "  Each record has the following layout:\n");
    fprintf(docfs, "    - 2 byte integer year\n");
    fprintf(docfs, "    - 2 byte integer month\n");
    fprintf(docfs, "    - 2 byte integer day\n");
    fprintf(docfs, "    - 2 byte integer hour\n");
    fprintf(docfs, "    - 2 byte integer minute\n");
    fprintf(docfs, "    - 4 byte float second\n");
    fprintf(docfs, "    - 4 byte integer number of points in contour\n");
    fprintf(docfs, "      all longitudes are stored contiguously as floats\n");
    fprintf(docfs, "      followed by latitudes, NOT as lon-lat pairs\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "  Contour advection files may be queried using:\n");
    fprintf(docfs, "  > bev2xy\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "  and created using:\n");
    fprintf(docfs, "  > c0\n");
    fprintf(docfs, "  > xy2bev\n");
    fprintf(docfs, "  e.g.:\n");
    fprintf(docfs, "  > c0 45 | xy2bev c45deg.bev\n");
    return err;
  }

  //parse the argument list:
  if (argc==4) {
    strcpy(nfile, argv[1]);
    strcpy(sfile, argv[2]);
    strcpy(infile, argv[3]);
    strcpy(outfile, argv[3]);
  } else if (argc==2) {
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
    if (flag[5]==0 && flag[7]==0) t0=line;
    fgets(line, MAXLL, initfs);
    if (flag[3]==0) sscanf(line, "%g", &ts);
    fgets(line, MAXLL, initfs);
    if (flag[6]==0 && flag[8]==0) sscanf(line, "%d", &n);
    fgets(line, MAXLL, initfs);
    if (flag[4]==0) sscanf(line, "%d", &nrk);
    fgets(line, MAXLL, initfs);
    if (flag[0]==0) sscanf(line, "%g", &tarc);
    fgets(line, MAXLL, initfs);
    if (flag[1]==0) sscanf(line, "%g", &minspac);
    fgets(line, MAXLL, initfs);
    if (flag[2]==0) nread=sscanf(line, "%g", &maxspac);
    if (nread != 1 && flag[2]==0) maxspac=10*minspac;
  }

  //convert degrees to radians:
  tarc=tarc*PI/180;

  //initialize the contour:
  contour=new boundary_list_g(nfile, sfile);
  if (maxspac <= minspac) maxspac=10*minspac;
  contour->set_parm(ts, nrk, tarc, minspac, maxspac);

  //get the initial contour:
  infs=fopen(infile, "r");
  fread(&magic, sizeof(magic), 1, infs);
  if (magic != MAGIC) {
    printf("Wrong file type: %s\n", infile);
    return 2;
  }
  fread(&nc, sizeof(nc), 1, infs);
  contour->read(infs);
  fclose(infs);

  //unless set otherwise, we start at the date supplied with the initial
  //contour:
  if (flag[7]==0) ind1=contour->get_tind();

  //figure out integration interval:
  if (flag[5]) ind1=contour->set(t0);
  if (flag[6]) {
    n=(contour->set(tf)-ind1)/ts;
  } else if (n<0) {
    n=(contour->nt()-ind1)/ts;
  }

  printf("Time step: %f\n", ts);
  printf("Number of steps: %d\n", n);
  printf("Number of intermediate steps: %d\n", nrk);
  printf("Threshold arc: %f\n", tarc);
  printf("Minimum spacing: %f\n", minspac);
  printf("Maximum spacing: %f\n", maxspac);

  if (oflag) contour->wrap_off();

  //set the start time:
  contour->set(ind1);

  //open the output file and write the headers:
  outfs=fopen(outfile, "w");
  fwrite(&magic, sizeof(magic), 1, outfs);
  nrec=n/writeint+1;
  n++;
  fwrite(&nrec, sizeof(nrec), 1, outfs);
//  contour->print(stdout);
  npts=contour->fix();
  contour->write(outfs);

  for (int32_t i=1; i<n; i++) {
    tcur=contour->advance();
    tcur.write_string(tstring);
    printf("%4d %s %d\n", i, tstring, npts);
//    contour->print(stdout);
    if (i % writeint == 0) contour->write(outfs);
    if (maxnpt > 0 && npts >= maxnpt) {
      nrec=i/writeint+1;
      break;
    }
    fflush(outfs);
    npts=contour->fix();
  }

  if (maxnpt > 0) {
    fseek(outfs, sizeof(magic), SEEK_SET);
    fwrite(&nrec, sizeof(nrec), 1, outfs);
  }

  fclose(outfs);

  //delete contour;

  return 0;

}
