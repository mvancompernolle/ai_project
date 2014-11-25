//lon-lat ascii to azimuthal-equidistant binary...
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//much better than getopt (reduces size of source by an order-of-magnitude...):
#include "error_codes.h"
#include "parse_command_opts.h"

#include "simple_temp.h"
#include "dependent_temp.h"

#include "coordtran.h"
#include "ctraj_defaults.h"

int main(int argc, char **argv) {

  char *infile;
  char *outfile;
  ind_type index;

  FILE *fs;

  size_t ncon;

  int aflag=0;		//include corners
  int zflag=0;		//print out record size

  float *dum;

  float rmax;		//side length/2
  ind_type ngrid;		//grid points per side
  sub_1d_type ntot;		//total number of points/2
  ind_type nlon, nlat;	//lon-lat grid
  ind_type nlatn, nlats;
  char c;
  float val;
  double c1val, c2val;

  //datasets:
  simple<float> *xgrid;
  simple<float> *ygrid;
  simple<float> *lon;
  simple<float> *lat;
  dependent<float> *qn;
  dependent<float> *qs;
  float *readq_n;
  float *readq_s;
  //interpolation coefficients:
  dependent<interpol_index> *c1;
  dependent<interpol_index> *c2;

  dependent<sub_1d_type> *map;
  sub_1d_type mapval;

  size_t fsize;		//file size
  sub_1d_type recsize;	//record size
  int32_t nvar;	//size of each vector
  int32_t nrec;	//number of records

  void *optarg[10];
  int flags[10];

  int err;

  ngrid=NGRID_Q;
  rmax=SIDELENGTH_Q;

  nlon=NLON;
  nlat=NLAT;

  optarg[1]=&ngrid;
  optarg[2]=&rmax;
  optarg[3]=&nlon;
  optarg[4]=&nlat;
  argc=parse_command_opts(argc, argv, "anrxy?", "%%d%g%d%d%", optarg, flags, OPT_WHITESPACE);
  if (argc<0) exit(21);
  aflag=flags[0];

  if ((argc < 2 && zflag != 1) || flags[5]) {
    FILE *docfs;
    int err;
    if (flags[5]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "\n");
    fprintf(docfs, "Syntax:  extract_field [-a] [-r rmax] [-n ngrid] [-x nlon] [-y nlat]\n");
    fprintf(docfs, "                             infile [index]\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "Converts fields in azimuthal equidistant gridding, binary format to\n");
    fprintf(docfs, "lon-lat gridding, ASCII format (GMT compatible)\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "where:\n");
    fprintf(docfs, "  infile  =  input file name, binary dump of an array of vectors\n");
    fprintf(docfs, "  index   =  0-based index of field to extract\n");
    fprintf(docfs, "             (- if argument is absent, returns number of records;\n");
    fprintf(docfs, "              - gridding is specified by -x and -y options.)\n\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "anrxy?", 1);
    fprintf(docfs, "\n");
    return err;
  }

  infile=argv[1];
  if (argc>2) sscanf(argv[2], "%d", &index); else index=-1;
  fs=fopen(infile, "r");

  fread(&nvar, sizeof(nvar), 1, fs);

  fseek(fs, 0, SEEK_END);
  fsize=ftell(fs);
  nrec=(fsize-sizeof(nvar))/nvar/sizeof(float);

  if (flags[1]==0) {
    if (aflag) {
      ngrid=sqrt(nvar/2);
    } else {
      ngrid=2*(ind_type) sqrt(nvar/M_PI/2);
    }
  }
  //printf("ngrid: %d\n", ngrid);

  //create the input grids and interpolation coefficients:
  //xgrid=create_grid(ngrid, rmax);
  //ygrid=create_grid(ngrid, rmax);
  xgrid=new simple<float>(-rmax, rmax, ngrid);
  ygrid=new simple<float>(-rmax, rmax, ngrid);

  //create the dataset to hold the input data and read it in:
  qn=new dependent<float>(xgrid, ygrid);
  qs=new dependent<float>(xgrid, ygrid);

  //create mapping from cornerless to cornered grid:
  if (aflag != 1) {
    map=new dependent<sub_1d_type>(xgrid, ygrid);
    nocorner_map(xgrid, ygrid, map, ntot);
    readq_n=new float[ntot];
    readq_s=new float[ntot];
  } else {
    ntot=ngrid*ngrid;
  }

  if (nvar != 2*ntot) {
    if (flags[0]==0) {
      fprintf(stderr, "extract_field: Forward and reverse calculations between grid size\n");
      fprintf(stderr, "and vector length do not match: %d vs %d.  Use -n option.\n", nvar, 2*ntot);
      fprintf(stderr, "Please report this bug: petey@peteysoft.org\n");
      err=901;
    } else {
      fprintf(stderr, "extract_field: -n specification does not match file header\n");
      err=411;
    }
  }

  if (zflag) {
    printf("%d\n", ntot*2);
    exit(0);
  }

  recsize=2*ntot*sizeof(float);
  if ((fsize-sizeof(nvar))%recsize!=0) {
    fprintf(stderr, "Not an even number of records:\n");
    fprintf(stderr, "     %ld mod %ld = %ld\n", fsize, recsize, fsize%recsize);
  }

  if (argc==2) {
    printf("%d\n", nrec);
    exit(0);
  }

  if (index >= nrec || index < 0) {
    fprintf(stderr, "Index out of range: %d (%d records)\n", index, nrec);
    exit(-3);
  }

  fseek(fs, index*recsize+sizeof(nvar), SEEK_SET); //should do some range-checking here...

  if (aflag != 1) {
    fread(readq_s, sizeof(float), ntot, fs);
    fread(readq_n, sizeof(float), ntot, fs);
    //map this onto a rectangular grid:
    for (ind_type i=0; i<ngrid; i++) {
      for (ind_type j=0; j<ngrid; j++) {
        map->get(mapval, i, j);
	//(this is not the most efficient way of doing things...)
	if (mapval > -1) {
          qn->cel(readq_n[mapval], i, j);
	  qs->cel(readq_s[mapval], i, j);
	  //printf("%d %f\n", mapval, readq_s[mapval]);
	}
      }
    }
  } else {
    qs->read(fs);
    qn->read(fs);
  }
  fclose(fs);

  //create the output grids:
  nlats=nlat/2;

  lon=new simple<float>(0., 360.-360./nlon, nlon);
  lat=new simple<float>(-90., 90., nlat);

  //lon->print();
  //lat->print();

  c1=new dependent<double>(lon, lat);
  c2=new dependent<double>(lon, lat);
  intcoeff2(lon, lat, xgrid, ygrid, c1, c2, -1);

  //Southern Hemisphere:
  //perform the interpolation and write the result to the output file:
  for (ind_type j=0; j<nlats; j++) {
    for (ind_type i=0; i<nlon; i++) {
      c1->get(c1val, i, j);
      c2->get(c2val, i, j);
      qs->interpol(val, c1val, c2val);
      //printf("%d %d\n", i, j);
      //printf("%lg %lg\n", c1val, c2val);
      //printf("%f\n", val);
      printf("%g\n", val);
    }
  }

  //Northern Hemisphere:
  //create the output grids and interpolation coefficients:
  intcoeff2(lon, lat, xgrid, ygrid, c1, c2, 1);

  //perform the interpolation and write the result to the output file:
  for (ind_type j=nlats; j<nlat; j++) {
    for (ind_type i=0; i<nlon; i++) {
      //printf("%d %d\n", i, j);
      c1->get(c1val, i, j);
      c2->get(c2val, i, j);
      qn->interpol(val, c1val, c2val);
      printf("%g\n", val);
    }
  }

  delete c1;
  delete c2;

  delete qn;
  delete qs;

  delete lon;
  delete lat;

  if (aflag != 1) {
    delete map;
    delete [] readq_n;
    delete [] readq_s;
  }

  delete xgrid;
  delete ygrid;

  return err;

}
