//lon-lat ascii to azimuthal-equidistant binary...
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "error_codes.h"
#include "linked.h"
#include "parse_command_opts.h"

#include "simple_temp.h"
#include "dependent_temp.h"

#include "coordtran.h"
#include "ctraj_defaults.h"

#define MAXLL 250

int main(int argc, char **argv) {

  char *infile;
  char *outfile;

  char *mapfile=NULL;

  FILE *fs;
  int32_t nvar;

  size_t ncon;

  float *dum;

  float rmax;
  ind_type ngrid;
  ind_type nlon, nlat;
  char c;
  float val;
  interpol_index c1val, c2val;

  float xval, yval;
  ind_type xind, yind;

  //datasets:
  simple<float> *xgrid;
  simple<float> *ygrid;
  simple<float> *lon;
  simple<float> *lat;
  dependent<float> *q1;
  //interpolation coefficients:
  dependent<interpol_index> *c1;
  dependent<interpol_index> *c2;

  //mapping to cancel corners:
  sub_1d_type nmap;
  long nmap2;
  dependent<sub_1d_type> *map;
  sub_1d_type mapval;

  //to read in the mapfile:
  linked_list<char *> mapin;
  char line1[MAXLL];
  char *line2;
  char **maptext;

  void *optargs[6];
  int flags[6];

  short aflag=0;	//use mapping to eliminate "corners" (0=yes)

  //the tracer vector (what we are after after all this work...)
  //long k;
  float *qvec;

  //defaults:
  ngrid=NGRID_Q;
  rmax=SIDELENGTH_Q;

  nlon=NLON;
  nlat=NLAT;

  optargs[1]=&ngrid;
  optargs[2]=&rmax;
  optargs[3]=&nlon;
  optargs[4]=&nlat;
  optargs[5]=NULL;
  argc=parse_command_opts(argc, argv, "anrxym?", "%%d%g%d%d%s%", 
		  optargs, flags, OPT_WHITESPACE);
  if (argc < 0) exit(21);
  aflag=flags[0];
  mapfile=(char *) optargs[5];
  //printf("%d %g %d %d %s\n", ngrid, rmax, nlon, nlat, mapfile);

  if (argc < 2 || flags[6]) {
    FILE *docfs;
    int err;
    if (flags[6]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "\n");
    fprintf(docfs, "Syntax:  lla2aeb [-a] [-r rmax] [-n ngrid] [-x nlon] [-y nlat]\n");
    fprintf(docfs, "                             [infile] outfile\n");
    fprintf(docfs, "\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "anrxy?", 1);
    fprintf(docfs, "\n");
    return err;
  } else if (argc == 2) {
    infile=NULL;
    outfile=argv[1];
  } else if (argc > 2) {
    infile=argv[1];
    outfile=argv[2];
  }

  //Northern Hemisphere:
  //create the output grids and interpolation coefficients:
  //xgrid=create_grid(ngrid, rmax);
  //ygrid=create_grid(ngrid, rmax);
  xgrid=new simple<float>(-rmax, rmax, ngrid);
  ygrid=new simple<float>(-rmax, rmax, ngrid);
  c1=new dependent<interpol_index>(xgrid, ygrid);
  //c1->print_meta();
  c2=new dependent<interpol_index>(xgrid, ygrid);
  //c2->print_meta();

  map=new dependent<sub_1d_type>(xgrid, ygrid);
  //map->print_meta();

  //read in the map (if applicable):
  if (mapfile != NULL) {
    fs=fopen(mapfile, "r");
    while (fgets(line1, MAXLL, fs) != NULL) {
      ncon=strlen(line1);
      line2=new char[ncon+1];
      strcpy(line2, line1);
      mapin.push(line2);
    }
    fclose(fs);
    maptext=mapin.make_array(nmap2);
    nmap=nmap2;
    if (nmap != ngrid*ngrid) {
      fprintf(stderr, "Map must have same size as grid:%d, %d\n", nmap, ngrid*ngrid);
    }
    for (sub_1d_type i=0; i<nmap; i++) {
      sscanf(maptext[i], "%d %d %d", &xind, &yind, mapval);
      map->cel(mapval, xind, yind);
      //printf("%d\n", mapval);
      delete [] maptext[i];   
    }

    delete [] maptext;
  } else if (aflag) {
    nmap=ngrid*ngrid;
    for (sub_1d_type i=0; i<nmap; i++) map->cel_1d(i, i);
  } else {
    //create the mapping from scratch:
    nocorner_map(xgrid, ygrid, map, nmap);
  }
  qvec=new float[nmap];

  //create the input grids:
  lon=new simple<float>(0., 360., nlon+1);
  lat=new simple<float>(-90., 90., nlat);

  //create the dataset to hold the input data and read it in:
  q1=new dependent<float>(lon, lat);
  if (infile != NULL) fs=fopen(infile, "r"); else fs=stdin;
  for (ind_type j=0; j<nlat; j++) {
    for (ind_type i=0; i<nlon; i++) {
      fscanf(fs, "%g", &val);
      q1->cel(val, i, j);
    }
    //"wrap" input grids for interpolation:
    q1->get(val, 0, j);
    q1->cel(val, nlon, j);
  }
  fclose(fs);

  //Southern Hemisphere:
  //interpolation coefficients:
  intcoeff(lon, lat, xgrid, ygrid, c1, c2, -1);

  //perform the interpolation and write the result to the output file:
  for (ind_type i=0; i<ngrid; i++) {
    for (ind_type j=0; j<ngrid; j++) {
      c1->get(c1val, i, j);
      c2->get(c2val, i, j);
      q1->interpol(val, c1val, c2val);
      //printf("%d %d\n", i, j);
      //printf("%lg %lg\n", c1val, c2val);
      //printf("%f\n", val);
      //fwrite(&val, 1, sizeof(val), fs);
      map->get(mapval, i, j);
      //printf("%d %d %d %d\n", i, j, mapval, nmap);
      if (mapval >= 0) {
        qvec[mapval]=val;
	//printf("%f\n", val);
      }
    }
  }

  //printf("%d\n", k);

  fs=fopen(outfile, "w");
  nvar=nmap*2;
  //write size of each record for easier parsing and compatibility with libagf:
  fwrite(&nvar, sizeof(nvar), 1, fs);
  fwrite(qvec, sizeof(float), nmap, fs);

  //Northern Hemisphere:
  //create the interpolation coefficients:
  intcoeff(lon, lat, xgrid, ygrid, c1, c2, 1);

  //perform the interpolation and write the result to the output file:
  for (ind_type i=0; i<ngrid; i++) {
    for (ind_type j=0; j<ngrid; j++) {
      c1->get(c1val, i, j);
      c2->get(c2val, i, j);
      //printf("%lf %lf\n", c1val, c2val);
      q1->interpol(val, c1val, c2val);
      //fwrite(&val, 1, sizeof(val), fs);
      map->get(mapval, i, j);
      if (mapval >= 0) {
        qvec[mapval]=val;
	//printf("%f\n", val);
      }
    }
  }
  fwrite(qvec, sizeof(float), nmap, fs);
  fclose(fs);

  delete c1;
  delete c2;
  if (aflag != 1) delete map;
  delete xgrid;
  delete ygrid;

  delete q1;
  delete lon;
  delete lat;

  delete [] qvec;

}
