#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>

#include "default_grid.h"
#include "coordtran.h"

int main(int argc, char **argv) {
  char * infile;
  FILE * fs;

  simple<float> *xgrid;
  simple<float> *ygrid;
  dependent<long> *map;

  int aflag=0;
  long n;
  float area=10000*10000*M_PI;
  long ngrid=NGRID_Q;
  float slen=SIDELENGTH_Q;
  long fsize;		//size of file in bytes
  long mod;

  long ncon;
  char c;

  while ((c = getopt(argc, argv, "r:n:a")) != -1) {
    switch (c) {
      case ('n'):
        ncon=sscanf(optarg, "%d", &ngrid);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -n %s", optarg);
          exit(2);
        }
        break;
      case ('r'):
        ncon=sscanf(optarg, "%f", &slen);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -r %s", optarg);
          exit(2);
        }
        break;
      case ('a'):
	aflag=1;
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
    printf("Usage: query_tracer_out [-a] [-r slen] [-n ngrid] file\n\n");
    printf("Returns the number of records in a passive tracer simulation\n\n");
    printf("Where:\n");
    printf("  file     File containing binary dump of tracer fields as vectors\n");
    printf("  -a       Simulation includes (redundant) corners\n");
    printf("  slen     Sidelength/2\n");
    printf("  ngrid    Number of gridpoints per side\n");
    exit(-1);
  }

  infile=argv[0];

  fs=fopen(infile, "r");
  fseek(fs, 0, SEEK_END);
  fsize=ftell(fs);
  fclose(fs);

  if (aflag) {
    n=ngrid*ngrid*2;
  } else {
    xgrid=new simple<float>(-slen, slen, ngrid);
    ygrid=new simple<float>(-slen, slen, ngrid);
    map=new dependent<long>(xgrid, ygrid);
    nocorner_map(xgrid, ygrid, map, n);
    n=n*2;
  }

  mod = fsize%n;
  if (mod != 0) {
    fprintf(stderr, "Warning: not an even number of records;");
    fprintf(stderr, "  (%d mod %d = %d)\n", fsize, n, mod);
  }

  printf("%d\n", fsize/n/sizeof(float));

}

