#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "parse_command_opts.h"
#include "ctraj_defaults.h"

int main(int argc, char **argv) {
  char c;
  long ncon;
  long nlon, nlat;
  float q;

  nlon=NLON;
  nlat=NLAT;

  int pflag=0;

  void * optarg[10];
  int flag[10];

  optarg[0]=&nlon;
  optarg[1]=&nlat;

  argc=parse_command_opts(argc, argv, "xyE?", "%d%d%%", optarg, flag, OPT_WHITESPACE);
  if (argc < 0) {
             fprintf(stderr, "zonally_symmetric_tracer: Error parsing command line\n");
             exit(2);
  }
  pflag=flag[2];

  if (flag[3]) { 
	    fprintf(stdout, "\n");
            fprintf(stdout, "Syntax:  zonally_symmetric tracer [-x nlon] [-y nlat] [-?] [-E]\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Generates a zonally-symmetric tracer field and prints it to stdout.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "options:\n");
	    ctraj_optargs(stdout, "xyE?");
	    fprintf(stdout, "\n");
	    exit(0);
  }

  for (long i=0; i<nlat; i++) {
    for (long j=0; j<nlon; j++) {
      if (pflag) {
        if (j <= nlon/2.) q=2.*j/nlon; else q=2.*(j-nlon)/nlon;
      } else {
	q=2.*i/(nlat-1)-1;
      }
      printf("%g\n", q);
    }
  }

}

