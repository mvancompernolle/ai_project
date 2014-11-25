#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "netcdfcpp.h"

#include "coeffs.h"

#include "time_class.h"
#include "simple_temp.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

#include "default_grid.h"
#include "coordtran.h"

#define TOFFS -2.
#define HOURSPERDAY 24.
#define MPERKM 1000.
#define SECPERDAY 86400.
#define KAPPA 0.2857
#define P0 1000.
#define FNAMELEN 20

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

  float lev;			//sigma level

  ind_type nt;			//number of time grids
  ind_type nlat;
  ind_type nlon;
  ind_type nz;

  float rmax;			//(side-length)/2
  long ngrid;			//number of grid points per side

  float val;
  float xval, yval;		//for holding x and y grid values
  double r;
  double uval, vval, wval;	//for holding velocity vectors
  float tval;			//intermediate temperature value
  double utval, vtval, wtval;	//for holding transformed v vectors
  double c1val, c2val;		//for holding interpolation coeffs.
  double lonp, latp;		//lon and lat coords of grid point in transformed system
  float lonp1, latp1;		//lon and lat coords of grid point in transformed system
  int hemi;			//hemisphere
  double tint;
  float vunit;			//unit conversion for velocity

  char wflag=0;			//import vertical velocity??

  //variables for input files:
  int year;

  char *path;
  char *ufile;
  char *vfile;
  char *wfile;
  char fname[FNAMELEN];

  NcFile *nc_u;
  NcFile *nc_v;
  NcFile *nc_w;

  NcVar *ncv;
  NcAtt *nca;
  float *data;
  short *ncdata_u, *ncdata_v, *ncdata_w;

  NcVar *ncv_u;
  NcVar *ncv_v;
  NcVar *ncv_w;

  double u0, du;
  double v0, dv;
  double w0, dw;

  composite_dataset all(1);	//holds the data to write to the output file
  long loc;

  double zint;
  long zloc;			//location in array of desired level
  double frac;			//interpolation coeff for desired level

  double *traw;
  time_class *time;
  double tconv;			//conversion from NCEP units to days
  
  simple<float> *xgrid;		//x grid
  simple<float> *ygrid;		//y grid
  simple<float> zgrid;		//pressure level
  simple<time_class> *tgrid;		//time grid

  simple<float> *lon;		//ncep lon. grid
  simple<float> *lat;		//ncep lat. grid
  float *pres;
  simple<float> pressure;	//ncep pressure grid

  dependent<double> *uu;	//ncep u-field
  dependent<double> *vv;	//ncep v-field
  dependent<double> *ww;	//ncep v-field
  simple<float> *pdum;

  dependent_swap<float> *u;	//transformed u-field
  dependent_swap<float> *v;	//transformed v-field
  dependent_swap<float> *w;	//transformed w-field

  dependent<double> *c1;	//interpolation coefficients (longitude)
  dependent<double> *c2;	//interpolation coefficients (latitude)

  char c;
  int ncon;
  char tstring[30];
  long nlen;			//for measuring the length of strings

  FILE *fstest;			//debugging purposes

  //test interpolation on bogus dataset:
  dependent<float> *testds;
  float testval;

  ngrid=NGRID;
  rmax=SIDELENGTH;
  path=new char [3];
  strcpy(path, "./");

  if (argc < 4) {
    fprintf(stdout, "\n");
    fprintf(stdout, "Syntax:  ncep_plev_to_vfield [-r rmax] [-n ngrid] [-p path] [-w]\n");
    fprintf(stdout, "                             year level outfile\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "where:\n");
    fprintf(stdout, "  year    =       year to import\n");
    //fprintf(stdout, "	t1      =       date to begin import\n");
    //fprintf(stdout, "	t2      =	last date to import\n");
    fprintf(stdout, "  level   =       pressure level to import (sign determines hemisphere)\n");
    fprintf(stdout, "  outfile =       name of output file\n\n");
    fprintf(stdout, "  rmax    =       (side-length)/2 [=%7.0f]\n", rmax);
    fprintf(stdout, "  ngrid   =       number of grid points per side [=%d]\n", ngrid);
    fprintf(stdout, "  path    =       directory containing input files [%s]\n\n", path);
    fprintf(stdout, "flags:\n");
    fprintf(stdout, "  -w      =       import vertical velocities\n", path);
    fprintf(stdout, "  -s      =       Southern hemisphere\n", path);
    return 1;
  }

  hemi=1;

  while ((c = getopt(argc, argv, "r:n:p:ws")) != -1) {
    switch (c) {
      case ('n'):
        ncon=sscanf(optarg, "%d", &ngrid);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -n %s", optarg);
          exit(2);
        }
        break;
      case ('r'):
        ncon=sscanf(optarg, "%f", &rmax);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -n %s", optarg);
          exit(2);
        }
        break;
      case ('p'):
	nlen=strlen(optarg);
	delete [] path;
        path=new char[nlen+2];
	strcpy(path, optarg);
	if (path[nlen] != '/') strcat(path, "/");
        break;
      case ('w'):
	wflag=1;
	break;
      case ('s'):
	hemi=-1;
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
  
  //parse the command line arguments:
  //t0.read_string(argv[1]);
  //t1.read_string(argv[2]);
  sscanf(argv[0], "%d", &year);
  sscanf(argv[1], "%f", &lev);
  //it's kind of a weird way of doing it, 
  //but it makes sense if you think about it...
  lev=hemi*lev;

  outfile=argv[2];
  //ngrid=300;
  //rmax=12000;
  if (argc>3) {
    sscanf(argv[3], "%f", &rmax);
    if (argc>4) {
      sscanf(argv[4], "%d", &ngrid);
    }
  }

  hemi=fabs(lev)/lev;
  lev=fabs(lev);

  //create the file names:
  ufile=new char[strlen(path)+FNAMELEN];
  strcpy(ufile, path);
  sprintf(fname, "uwnd.%4.4d.nc", year);
  strcat(ufile, fname);
  vfile=new char[strlen(path)+FNAMELEN];
  strcpy(vfile, path);
  sprintf(fname, "vwnd.%4.4d.nc", year);
  strcat(vfile, fname);
  if (wflag) {
    wfile=new char[strlen(path)+FNAMELEN];
    strcpy(wfile, path);
    sprintf(fname, "omega.%4.4d.nc", year);
    strcat(wfile, fname);

    printf("Opening file, %s...\n", wfile);
    nc_w=new NcFile(wfile);

    ncv_w=nc_w->get_var("omega");

    nca=ncv_w->get_att("scale_factor");
    dw=nca->as_double(0);
    nca=ncv_w->get_att("add_offset");
    w0=nca->as_double(0);
  }

  //open all the files:
  printf("Opening file, %s...\n", ufile);
  nc_u=new NcFile(ufile);
  printf("Opening file, %s...\n", vfile);
  nc_v=new NcFile(vfile);

  ncv_u=nc_u->get_var("uwnd");
  ncv_v=nc_v->get_var("vwnd");

  nca=ncv_u->get_att("scale_factor");
  du=nca->as_double(0);
  nca=ncv_u->get_att("add_offset");
  u0=nca->as_double(0);

  nca=ncv_v->get_att("scale_factor");
  dv=nca->as_double(0);
  nca=ncv_v->get_att("add_offset");
  v0=nca->as_double(0);

  //read in the grids for the NCEP data:
  printf("Reading ncep grids...\n");
  //ncep longitude grid:
  ncv=nc_u->get_var("lon");
  nlon=ncv->num_vals();
  data=new float[nlon];
  ncv->get(data, nlon);
  lon=new simple<float>(data, nlon);
  //for (long i=0;i<nlon;i++) printf("%f ", data[i]);
  //printf("\n");
  delete [] data;
  //add 360 degree longitude grid which wraps 0 degree grid:
  lon->add_el(360);
  
  //ncep latitude grid:
  ncv=nc_u->get_var("lat");		//some rather unfortunate variable names here...
  nlat=ncv->num_vals();
  data=new float[nlat];
  ncv->get(data, nlat);
  //lat=new simple<float>(data, nlat);
  lat=new simple<float>;
  for (ind_type i=0;i<nlat;i++) {
    //printf("%f ", data[i]);
    //reverse latitude grids:
    lat->add_el(data[i]);
  }
  //printf("\n");
  delete [] data;

  //time grid:
  ncv=nc_u->get_var("time");
  nt=ncv->num_vals();
  traw=new double[nt];
  ncv->get(traw, nt);

  if (nt > 366) tconv=HOURSPERDAY; else tconv=1;
  
  //convert time to our format:
  printf("Converting time grids...\n");
  time=new time_class[nt];
  for (long i=0; i<nt; i++) {
    //convert raw times to units of days:
    traw[i]/=tconv;
    time[i].init(1, 1, 1, 0, 0, 0);
    time[i].add((traw[i])+TOFFS);
  }
  tgrid=new simple<time_class>(time, nt);

  //ncep pressure grid:
  ncv=nc_u->get_var("level");
  nz=ncv->num_vals();
  pres=new float[nz];
  ncv->get(pres, nz);

  printf("Pressure grid: ");
  for (long i=0; i<nz; i++) {
    printf("%6.0", pres[i]);
    //this is sooo inefficient:
    pressure.add_el(pres[i]);
  }
  printf("\n");

  printf("%d longitude x %d latitude x %d level x %d time grids\n", 
		  nlon, nlat, nz, nt);

  //z-grid:
  zgrid.add_el(lev*100);	//(convert to units compatible with vertical velocity)

  //interpolate the pressure level:
  zint=nz-pressure.interp(lev)-1;
  zloc=long(zint);
  frac=zint-zloc;

  //x- and y-grids:
  printf("Generating interpolation coefficients\n");
  xgrid=new simple<float>(-rmax, rmax, ngrid);
  ygrid=new simple<float>(-rmax, rmax, ngrid);

  //calculate the interpolation coefficients:
  c1=new dependent<double>(xgrid, ygrid);
  c2=new dependent<double>(xgrid, ygrid);
 
  intcoeff(lon, lat, xgrid, ygrid, c1, c2, hemi);

  //create the output data and group it into the "all" variable:
  printf("Creating data structure to hold output data...\n");
  loc=all.add_var("xgrid");
  all.cvar(loc, (dataset *) xgrid);
  loc=all.add_var("ygrid");
  all.cvar(loc, (dataset *) ygrid);
  loc=all.add_var("zgrid");
  all.cvar(loc, (dataset *) &zgrid);
  loc=all.add_var("time");
  all.cvar(loc, (dataset *) tgrid);

  u=new dependent_swap<float>(xgrid, ygrid, &zgrid, tgrid);
  v=new dependent_swap<float>(xgrid, ygrid, &zgrid, tgrid);

  loc=all.add_var("u");
  all.cvar(loc, (dataset *) u);
  loc=all.add_var("v");
  all.cvar(loc, (dataset *) v);

  if (wflag) {
    w=new dependent_swap<float>(xgrid, ygrid, &zgrid, tgrid);
    loc=all.add_var("w");
    all.cvar(loc, (dataset *) w);
    ww=new dependent<double>(lon, lat, pdum);
    ncdata_w=new short[nlon*nlat*nz];
  }
	  
  //create the datasets to hold the ecmwf data:
  ncdata_u=new short[nlon*nlat*nz];
  ncdata_v=new short[nlon*nlat*nz];
  pdum=new simple<float>(pres+zloc, 2);
  uu=new dependent<double>(lon, lat, pdum);
  vv=new dependent<double>(lon, lat, pdum);

  //testds=new dependent<float>(lon, lat);

  /*
  //populate our test dataset with zonally symmetric field:
  //printf("%f\n", latp1);
  for (long i=0; i<=nlon; i++) {
    lon->get(lonp1, i);
    printf("%f\n", lonp1);
    for (long j=0; j<nlat; j++) {
      //lat->get(latp1, j);
      testds->cel(lonp1, i, j);
    }
  }
  */

  //note: this whole process is rather awkward
  //...there must be a better way of doing it...

  //start to read and write the data:
  outfs=fopen(outfile, "w");
  all.write(outfs);
  for (ind_type it=0; it<nt; it++) {
    time[it].write_string(tstring);
    printf("Reading in grid %s...\n", tstring);

    //time interval should always be the same value, 
    //but we calculate it anyways:
    if (it == 0) {
      tint=traw[1]-traw[0];
    } else if (it == nt-1) {
      tint=traw[nt-1]-traw[nt-2];
    } else {
      tint=(traw[it+1]-traw[it-1])/2;
    }

    //read in the data from the ncep files:
    ncv_u->set_cur(it, zloc, 0, 0);
    ncv_u->get(ncdata_u, 1, 2, nlat, nlon);
    ncv_v->set_cur(it, zloc, 0, 0);
    ncv_v->get(ncdata_v, 1, 2, nlat, nlon);

    printf("Storing two adjacent levels...\n");

    //if (it == 0) fstest=fopen("check1.txt", "w");

    for (ind_type j=0; j<nlat; j++) {
      for (ind_type i=0; i<nlon; i++) {
        uval=ncdata_u[i+nlon*j]*du+u0;
        vval=ncdata_v[i+nlon*j]*dv+v0;
	//reverse latitude grids:
	uu->cel(uval, i, nlat-j-1, 0);
	vv->cel(vval, i, nlat-j-1, 0);
	//printf(" %f", uval);

	//if (it == 0) fprintf(fstest, "%f\n", uval);

        uval=ncdata_u[i+nlon*(j+nlat)]*du+u0;
        vval=ncdata_v[i+nlon*(j+nlat)]*dv+v0;
	uu->cel(uval, i, nlat-j-1, 1);
	vv->cel(vval, i, nlat-j-1, 1);
      }
    }
    //printf("\n");

    //wrap the zero-degree latitude grid:
    for (ind_type j=0; j<nlat; j++) {
      uu->get(uval, 0, j, 0);
      uu->cel(uval, nlon, j, 0);
      vv->get(vval, 0, j, 0);
      vv->cel(vval, nlon, j, 0);
      uu->get(uval, 0, j, 1);
      uu->cel(uval, nlon, j, 1);
      vv->get(vval, 0, j, 1);
      vv->cel(vval, nlon, j, 1);
    }
    //if (it == 0) fclose(fstest);

    if (wflag) {
      ncv_w->set_cur(it, zloc, 0, 0);
      ncv_w->get(ncdata_w, 1, 2, nlat, nlon);
      for (long i=0; i<nlon; i++) {
        for (long j=0; j<nlat; j++) {
          wval=ncdata_w[i+nlon*j]*dw+w0;
          //reverse latitude grids:
          ww->cel(wval, i, nlat-j-1, 0);

          //if (it == 0) fprintf(fstest, "%f\n", uval);

          wval=ncdata_w[i+nlon*(j+nlat)]*dw+w0;
          ww->cel(wval, i, nlat-j-1, 1);
        }
      }
      for (long j=0; j<nlat; j++) {
        ww->get(wval, 0, j, 0);
        ww->get(wval, 0, j, 1);
        ww->cel(wval, nlon, j, 0);
        ww->cel(wval, nlon, j, 1);
      }
    }

    //if (it == 0) fstest=fopen("check2.txt", "w");
    printf("Interpolating to azimuthal grid...\n");
    vunit=(double) tint*SECPERDAY/MPERKM;//convert from m/s to km/(delta t=6 h)
    printf("velocity conversion: %f\n", vunit);
    for (ind_type j=0; j<ngrid; j++) {
      for (ind_type i=0; i<ngrid; i++) {
        //do the interpolation:
        c1->get(c1val, i, j);
        c2->get(c2val, i, j);
	uu->interpol(uval, c1val, c2val, frac);
	vv->interpol(vval, c1val, c2val, frac);
	//printf("(%8.2f,%8.2f) ", uval, vval);

	/*
	//if (it == 0) fprintf(fstest, "%f\n", uval);
	if (it == 0) {
	  if (testds->interpol(testval, c1val, c2val) != 0) exit(-1);
	  lon->get(lonp1, c1val);
	  lat->get(latp1, c2val);
          fprintf(fstest, "%f %f %f %f %f %f\n", c1val, c2val, lonp1, latp1, uval, testval);
        }
	*/

	//transform the velocity:
        xgrid->get(xval, i);
        ygrid->get(yval, j);
	r=sqrt(xval*xval+yval*yval);
	val=REARTH*sin(r/REARTH);
	uval*=vunit;
	vval*=vunit;
	utval=-hemi*vval*xval/r-uval*yval/val;
	vtval=-hemi*vval*yval/r+uval*xval/val;
	//printf("(%8.2f,%8.2f) ", utval, vtval);
	u->cel(utval, i, j, 0, it);
	v->cel(vtval, i, j, 0, it);

	if (wflag) {
	  ww->interpol(wval, c1val, c2val, frac);
	  w->cel(wtval, i, j, 0, it);
	}
      }
    }
    //printf("\n");
    //if (it == 0) fclose(fstest);
  }

  /*
  for (long i=0; i<=nlon; i++) {
    //lon->get(lonp1, i);
    for (long j=0; j<nlat; j++) {
      //lat->get(latp1, j);
      testds->get(lonp1, i, j);
      printf("%f\n", lonp1);
    }
  }
  */

  //finish:
  delete u;
  delete v;

  delete tgrid;

  fclose(outfs);

  delete c1;
  delete c2;

  delete uu;
  delete vv;

  delete nc_u;
  delete nc_v;
}

