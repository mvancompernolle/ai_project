#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "coeffs.h"

#include "time_class.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

//#define ECMWFPATH "/talax/storage/ecmwf/"
#define ECMWFPATH "/equinox/ecmwf/"
#define NLON 240
#define DLON 1.5
#define NLAT 121
#define DLAT 1.5
#define LAT0 -90
#define NLEV 60

//syntax: ecmwf_sigma_to_vfield t1 t2 level outfile [rmax [ngrid]]
//where:
//  t1		Date to start import
//  t2		Date to finish
//  level	Sigma level (+ indicates N. hemisphere, - S.)
//  outfile	Output file
//  rmax	(Side-length)/2
//  ngrid	Number of grid points per side

int main(int argc, char *argv[]) {
  char *outfile;		//output file name
  FILE *outfs;			//output file stream
  char ecmwf_file[200];		//ecmwf file name
  FILE *ecmwfs;			//ecmwf file stream
  time_class t0, t1;			//start, finish time
  time_class tval;			//miscellaneous time value
  time_class tdiff;			//t1-t0
  time_class tint;			//6 hours
  long slev;			//sigma level
  long nt;			//number of time grids
  short year, month, day, hour, min;	//fields from time value
  float s;			//seconds field
  float rmax;			//(side-length)/2
  long ngrid;			//number of grid points per side
  float val, val1, val2;	//misc. floating point values
  float xval, yval, r;		//for holding x and y grid values
  float uval, vval;		//for holding velocity vectors
  float utval, vtval;		//for holding transformed v vectors
  double c1val, c2val;		//for holding interpolation coeffs.
  float lonp, latp;		//lon and lat coords of grid point in transformed system
  int hemi;			//hemisphere
  float vunit;			//unit conversion for velocity

  composite_dataset all;	//holds the data to write to the output file
  long loc;
  
  simple<float> xgrid;		//x grid
  simple<float> ygrid;		//y grid
  simple<float> zgrid;		//sigma level
  simple<time_class> tgrid;		//time grid

  simple<float> lon;		//ecmwf lon. grid
  simple<float> lat;		//ecmwf lat. grid
  simple<long> sigma;		//sigma level index

  dependent<float> *uu;		//ecmwf u-field
  dependent<float> *vv;		//ecmwf v-field

  dependent_swap<float> *u;	//transformed u-field
  dependent_swap<float> *v;	//transformed v-field

  dependent<double> *c1;	//interpolation coefficients (longitude)
  dependent<double> *c2;	//interpolation coefficients (latitude)

  if (argc < 5) {
    fprintf(stdout, "\n");
    fprintf(stdout, "Syntax:  ecmwf_sigma_to_vfield t1 t2 level outfile [rmax [ngrid]]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  where:\n");
    fprintf(stdout, "	t1	=	date to begin import\n");
    fprintf(stdout, "	t2	=	last date to import\n");
    fprintf(stdout, "	level	=	sigma level to import (sign determines hemisphere)\n");
    fprintf(stdout, "	outfile	=	name of output file\n");
    fprintf(stdout, "	rmax	=	(side-length)/2 [=12000]\n");
    fprintf(stdout, "	ngrid	=	number of grid points per side [=300]\n");
    return 1;
  }
  
  //parse the command line arguments:
  t0.read_string(argv[1]);
  t1.read_string(argv[2]);
  sscanf(argv[3], "%d", &slev);
  outfile=argv[4];
  ngrid=300;
  rmax=12000;
  if (argc>5) {
    sscanf(argv[5], "%f", &rmax);
    if (argc>6) {
      sscanf(argv[6], "%d", &ngrid);
    }
  }

  hemi=-abs(slev)/slev;
  slev=abs(slev)-1;
  if (slev < 0 || slev >= NLEV) {
    fprintf(stderr, "Vertical level index out of bounds\n");
    return 2;
  }
  
  //initialize the grids:
  for (long i=0; i<NLON; i++) lon.add_el(i*DLON);
  for (long i=0; i<NLAT; i++) lat.add_el(i*DLAT+LAT0);
  for (long i=0; i<NLEV; i++) sigma.add_el(i);
  //(note: latitude grid is inverted from the usual sense-- positive for S., -ve for N.)
  zgrid.add_el(slev);

  //time grid:
  tint.read_string("0/0/0-6");
  tdiff=t1-t0;
  nt=tdiff/tint;
  nt++;
  for (long i=0; i<nt; i++) {
    tval=tint*(double) i;
    tval=t0+tval;
    tgrid.add_el(tval);
  }

  //x- and y-grids:
  for (long i=0; i<ngrid; i++) {
    val=i*rmax*2/(ngrid-1)-rmax;
    xgrid.add_el(val);
    ygrid.add_el(val);
  }

  //calculate the interpolation coefficients:
  c1=new dependent<double>(&xgrid, &ygrid);
  c2=new dependent<double>(&xgrid, &ygrid);
  for (long i=0; i<ngrid; i++) {
    for (long j=0; j<ngrid; j++) {
      xgrid.get(xval, i);
      ygrid.get(yval, j);
      //transform to lon-lat coords:
      r=sqrt(xval*xval+yval*yval);
      lonp=atan(yval/xval)*RAD2DEG;
      if (xval < 0) lonp=180+lonp;
      if (lonp < 0) lonp=lonp+360;
      latp=hemi*(90-r/KMPERDEG);

      //get the interpolation coefficients:
      c1val=lon.interp(lonp);
      c2val=lat.interp(latp);
      c1->cel(c1val, i, j);
      c2->cel(c2val, i, j);
    }
  }

  //create the output data and group it into the "all" variable:
  loc=all.add_var("xgrid");
  all.cvar(loc, (dataset *) &xgrid);
  loc=all.add_var("ygrid");
  all.cvar(loc, (dataset *) &ygrid);
  loc=all.add_var("zgrid");
  all.cvar(loc, (dataset *) &zgrid);
  loc=all.add_var("time");
  all.cvar(loc, (dataset *) &tgrid);

  u=new dependent_swap<float>(&xgrid, &ygrid, &zgrid, &tgrid);
  v=new dependent_swap<float>(&xgrid, &ygrid, &zgrid, &tgrid);

  loc=all.add_var("u");
  all.cvar(loc, (dataset *) u);
  loc=all.add_var("v");
  all.cvar(loc, (dataset *) v);
	  
  //create the datasets to hold the ecmwf data:
  uu=new dependent<float>(&lon, &lat, &sigma);  
  vv=new dependent<float>(&lon, &lat, &sigma);

  //start to read and write the data:
  outfs=fopen(outfile, "w");
  all.write(outfs);
  for (long it=0; it<nt; it++) {
    //create the ecmwf file name and read the fields:
    tgrid.get(tval, it);
    //tdiff=tint*(double) it;
    //tval=t0+tdiff;
    tval.get_fields(year, month, day, hour, min, s);
    //u field:
    sprintf(ecmwf_file, "%s%4.4d/%2.2d/%2.2d/%2.2d/UU%4.4d%2.2d%2.2d%2.2d", 
		    ECMWFPATH, year, month, day, hour, year, month, day, hour);
    printf("Reading file: %s\n", ecmwf_file);
    ecmwfs=fopen(ecmwf_file, "r");
    uu->read(ecmwfs);
    fclose(ecmwfs);

    //v field:
    sprintf(ecmwf_file, "%s%4.4d/%2.2d/%2.2d/%2.2d/VV%4.4d%2.2d%2.2d%2.2d", 
		    ECMWFPATH, year, month, day, hour, year, month, day, hour);
    printf("Reading file: %s\n", ecmwf_file);
    ecmwfs=fopen(ecmwf_file, "r");
    vv->read(ecmwfs);
    fclose(ecmwfs);

    vunit=(double) tint*86.4;		//convert from m/s to km/(delta t=6 h)
    for (long i=0; i<ngrid; i++) {
      for (long j=0; j<ngrid; j++) {
        //do the interpolation:
        c1->get(c1val, i, j);
        c2->get(c2val, i, j);
	uu->interpol(uval, c1val, c2val, (double) slev);
	vv->interpol(vval, c1val, c2val, (double) slev);
	//transform the velocity:
        xgrid.get(xval, i);
        ygrid.get(yval, j);
	r=sqrt(xval*xval+yval*yval);
	val=REARTH*sin(r/REARTH);
	uval*=vunit;
	vval*=vunit;
	utval=-hemi*vval*xval/r-uval*yval/val;
	vtval=-hemi*vval*yval/r+uval*xval/val;
	u->cel(utval, i, j, 0, it);
	v->cel(vtval, i, j, 0, it);
      }
    }
  }

  //finish:
  delete u;
  delete v;

  fclose(outfs);

  delete c1;
  delete c2;

  delete uu;
  delete vv;
      

}

