#include <assert.h>
#include <math.h>

#include <stdint.h>

#include "peteys_tmpl_lib.h"

//#include "nr.h"
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"

#include "tcoord_defs.h"
#include "boundary_list_g.h"

/*
extern simple<float> *global_xgrid;
extern simple<float> *global_ygrid;
extern dependent_swap<float> *global_u;
extern dependent_swap<float> *global_v;
*/

extern int derivs(real tind, real x[], real dxdt[], void *param);

//default constructor:
boundary_list_g::boundary_list_g() {
  xgrid_N=NULL;
  ygrid_N=NULL;
  xgrid_S=NULL;
  ygrid_S=NULL;
  zgrid=NULL;
  tgrid=NULL;

  u_N=NULL;
  v_N=NULL;
  u_S=NULL;
  v_S=NULL;

  N_fs=NULL;
  S_fs=NULL;

  result=NULL;

  x=NULL;
  y=NULL;
  hemi=NULL;
  n=0;

  tind=0;
  tstep=0;
  nrk=0;
  min_spac=0;
  thresh_arc=0;
  wrap_flag=0;
}

boundary_list_g::boundary_list_g(char *nfile, char *sfile) {
  simple<float> *check;
  simple<time_class> *checkt;
  long loc, dum;
  time_class tel;
  char tstr[30];

  printf("Opening: %s\n", nfile);
  N_fs=fopen(nfile, "r");

  //get the N. hemisphere velocity field:
  ndata.read(N_fs);
  loc=ndata.search_var("xgrid", dum);
  xgrid_N=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("ygrid", dum);
  ygrid_N=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("zgrid", dum);
  zgrid=(simple<float> *) ndata.get_var(loc);
  loc=ndata.search_var("time", dum);
  tgrid=(simple<time_class> *) ndata.get_var(loc);
  loc=ndata.search_var("u", dum);
  u_N=(dependent_swap<float> *) ndata.get_var(loc);
  loc=ndata.search_var("v", dum);
  v_N=(dependent_swap<float> *) ndata.get_var(loc);

  param_N[0]=xgrid_N;
  param_N[1]=ygrid_N;
  param_N[2]=u_N;
  param_N[3]=v_N;

  //get the S. hemisphere velocity field:
  printf("Opening: %s\n", sfile);
  S_fs=fopen(sfile, "r");
  sdata.read(S_fs);
  loc=sdata.search_var("u", dum);
  u_S=(dependent_swap<float> *) sdata.get_var(loc);
  loc=sdata.search_var("v", dum);
  v_S=(dependent_swap<float> *) sdata.get_var(loc);

  //check the gridding of the two files to make sure that they agree:
  loc=sdata.search_var("xgrid", dum);
  xgrid_S=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("ygrid", dum);
  ygrid_S=(simple<float> *) sdata.get_var(loc);
  loc=sdata.search_var("zgrid", dum);
  check=(simple<float> *) sdata.get_var(loc);
  assert(*check == *zgrid);
  loc=sdata.search_var("time", dum);
  checkt=(simple<time_class> *) sdata.get_var(loc);
  assert(*checkt == *tgrid);

  param_S[0]=xgrid_S;
  param_S[1]=ygrid_S;
  param_S[2]=u_S;
  param_S[3]=v_S;

  x=NULL;
  y=NULL;
  hemi=NULL;
  n=0;

  result=NULL;
  nrk=0;
  tstep=0;

  wrap_flag=1;
/*
  for (ind_type i=0; i<tgrid->nel(); i++) {
    tgrid->get(tel, i);
    //tel.write_string(tstr);
    //printf("%s\n", tstr);
  }
  */

}

long boundary_list_g::init_circle(real x0, real y0, real r) {
  real ang;

  n=(long) (2*M_PI/thresh_arc);
  x=new real[n+1*wrap_flag];
  y=new real[n+1*wrap_flag];

  for (long i=0; i<n; i++) {
    ang=i*thresh_arc;
    //no attempt to apply metric corrections:
    x[i]=x0+r*cos(ang);
    y[i]=y0+r*sin(ang);
  }

  return n;
}


void boundary_list_g::wrap_on() {
  real *xnew;
  real *ynew;
  short *hnew;

  if (!wrap_flag) {
    wrap_flag=1;
    if (n != 0) {
      xnew=new real[n+1];
      ynew=new real[n+1];
      hnew=new short[n+1];
      for (long i=1; i<=n; i++) {
        xnew[i]=x[i];
        ynew[i]=y[i];
	hnew[i]=hemi[i];
      }
      delete [] x;
      delete [] y;
      delete [] hemi;
      x=xnew;
      y=ynew;
      hemi=hnew;
    }
  }
}

/*
void boundary_list_g::wrap_off() {
//  if (wrap_flag) {
    wrap_flag=0;
    if (n!=0) {
      for (long i=0; i<n; i++) {
        x[i]=x[i+1];
        y[i]=y[i+1];
      }
    }
  }
}
*/


boundary_list_g::~boundary_list_g() {
  if (x != NULL) delete x;
  if (y != NULL) delete y;
  if (hemi != NULL) delete [] hemi;

  delete u_N;
  delete v_N;
  delete u_S;
  delete v_S;
  delete xgrid_N;
  delete ygrid_N;
  delete xgrid_S;
  delete ygrid_S;
  //delete zgrid;
  //delete tgrid;

  if (N_fs != NULL) fclose(N_fs);
  if (S_fs != NULL) fclose(S_fs);
}

time_class boundary_list_g::advance() {
  short hold;
  real x0[2];
  real tstepfine=tstep/nrk;
  ind_type tindl;
  double frac;
  time_class t1;
  char tstr[30];
  void *param;

  //pass the grids to the global variables:
  param=(void *) param_N;
  hold=1;

  for (long i=0; i<n; i++) {
    tcoord_fix(x[i], y[i], hemi[i]);
    if (hemi[i] != hold) {
      hold=hemi[i];
      //pass the velocity fields to the global variables:
      if (hold == 1) {
        param=(void *) param_N;
      } else {
        param=(void *) param_S;
      }
    }
    x0[0]=x[i];
    x0[1]=y[i];
    rk_dumb((real) tind, x0, 2L, tstepfine, nrk, (real **) result, param, &derivs);
    x[i]=result[nrk][0];
    y[i]=result[nrk][1];
  }

  tind+=tstep;

  tgrid->get(t1, tind);

  return t1;
}

long boundary_list_g::fix() {
  long nn=n+wrap_flag;
  real s[nn];		//the parametric distance along the curve
  real dx2;		//2nd derivative of x wrt s
  real dy2;		//2nd derivative of y wrt s
  real dels;		//the distance between a pair of points
  real dstotal;
  real dangle;		//the radians of arc (^2) between a pair of points
  real curv2;		//the curvature squared
  short nnew[nn-1];	//number of new points to insert between each pair of points
  long totaln, dn;	//final total number of points
  real * xnew;
  real * ynew;
  short * hnew;
  long j, k;
  long offset;		//for removing redundant nodes

  gsl_interp *xinterp;
  gsl_interp *yinterp;
  gsl_interp_accel *xaccel;
  gsl_interp_accel *yaccel;

  gsl_interp_type *spltype;

  if (wrap_flag) {
    x[n]=x[0];
    y[n]=y[0];
    hemi[n]=hemi[0];
  }

  s[0]=0;
  //we work in the N. hemisphere:
  if (hemi[0] == -1) tcoord_N2S(x[0], y[0]);
  offset=0;
  //printf("nn=%d\n", nn);
  for (long i=1; i<nn; i++) {
    if (offset != 0) {
      //because of any redundant nodes, all the nodes must be shifted:
      j=i-offset;
      x[j]=x[i];
      y[j]=y[i];
      hemi[j]=hemi[i];
    } else {
      j=i;
    }
    if (hemi[j] == -1) tcoord_N2S(x[j], y[j]);
    dels=tcoord_ds2(x[j-1], y[j-1], x[j], y[j]);
    //printf("%f\n", dels);
    assert(dels>=0);
    s[j]=s[j-1]+sqrt(dels);
    //printf("%g %g\n", 
    //		sqrt((x[j-1]-x[j])*(x[j-1]-x[j])+(y[j-1]-y[j])*(y[j-1]-y[j])), 
    //		sqrt(dels));
    //check for nodes made redundant by round-off:
    if (s[j]-s[j-1] <= 0) {
      printf("Warning: removing %dth node made redundant by round-off\n", i);
      offset++;
    }
  }
  n-=offset;
  nn-=offset;

  //do a cubic spline interpolation on both variables:
  if (wrap_flag) {
    //closed contour implies periodic boundary conditions:
    spltype=(gsl_interp_type *) gsl_interp_cspline_periodic;
//    printf("dx0ds=%g; dy0ds=%g\n", dx0, dy0);
  } else {
    spltype=(gsl_interp_type *) gsl_interp_cspline;
  }

  //initialize our cubic spline fitters:
  xinterp=gsl_interp_alloc(spltype, nn);
  yinterp=gsl_interp_alloc(spltype, nn);

  xaccel=gsl_interp_accel_alloc();
  yaccel=gsl_interp_accel_alloc();

  //start the cubic spline interpolation:
  gsl_interp_init(xinterp, s, x, nn);
  gsl_interp_init(yinterp, s, y, nn);

  //calculate the angle traced out by each pair of points:
  //and use to figure out how many new points to add...
  totaln=1;
  dangle=0;
  dstotal=0;
//  printf("i, s, dx2ds2, dy2ds2, dangle, #new\n");
  for (long i=1; i<nn; i++) {
    dels=s[i]-s[i-1];
    dstotal+=dels;
    assert(dels>=0);
    //if the point spacing is less than the threshold, either
    //remove next point or add no new:
    if (dstotal < min_spac) {
      if (2*dstotal < min_spac) {
        nnew[i-1]=-1;		//remove point
      } else {
        nnew[i-1]=0;
      }
    } else {
      //get second order derivatives:
      gsl_interp_eval_deriv2_e(xinterp, s, x, (s[i-1]+s[i])/2, xaccel, &dx2);
      gsl_interp_eval_deriv2_e(xinterp, s, y, (s[i-1]+s[i])/2, yaccel, &dy2);
      //use that to calculate the sweep of arc:
      dangle+=sqrt(dx2*dx2+dy2*dy2)*dels;

      //printf("r=%g\n", dels/dangle);

      //if the accumulated angle of arc is greater than the threshold,
      //add new points, otherwise, remove the next point...
      if (dangle*2 < thresh_arc) {
        nnew[i-1]=-1;
      } else {
        nnew[i-1]=(short) (dangle/thresh_arc-0.5);
        //dangle=dangle-(nnew[i-1]+0.5)*thresh_arc;
	dangle=0;
      }

      //to correct for instabilities caused by 
      //points being too widely separated:
      if (dstotal > max_spac) {
        long checkn=(long) (dstotal/max_spac);
        if (checkn > nnew[i-1]) nnew[i-1]=checkn;
      }
      dstotal=0;
    }
    totaln+=nnew[i-1]+1;
    //printf("%d, %g, %g, %g, %g, %d\n", i, s[i], dx2[i], dy2[i], dangle, nnew[i-1]);
  }

  xnew=new real[totaln];
  ynew=new real[totaln];

  j=1;			//index of new nodes
  xnew[0]=x[0];		//always keeps the first point
  ynew[0]=y[0];

  k=0;			//start interpolating from this node
  for (long i=1; i<nn; i++) {
    if (nnew[i-1] > 0) {
      dstotal=s[i]-s[k];
      dels=dstotal/(nnew[i-1]+1);
      if (2*dels < min_spac) {
        dn=nnew[i-1]-(short) (dstotal/min_spac*2);
        nnew[i-1]-=dn;
	totaln-=dn;
        dels=dstotal/(nnew[i-1]+1);
      }
      //add in new points interpolated along the spline:
      for (long m=1; m <= nnew[i-1]; m++) {
	gsl_interp_eval_e(xinterp, s, x, s[k]+m*dels, xaccel, xnew+j);
	gsl_interp_eval_e(yinterp, s, y, s[k]+m*dels, yaccel, ynew+j);
        //printf("%g %g\n", xnew[j], ynew[j]);
	j++;
      }
      xnew[j]=x[i];
      ynew[j]=y[i];
      k=i;
      j++;
    } else if (nnew[i-1] == 0) {
      //no new points to add:
      xnew[j]=x[i];
      ynew[j]=y[i];
      k=i;
      j++;
    }
  }

  assert(j==totaln);

  delete [] x;
  delete [] y;
  x=xnew;
  y=ynew;

  delete [] hemi;
  hemi=new short[totaln];
  for (long i=0; i<totaln; i++) hemi[i]=1;

  n=totaln-wrap_flag;

  //clean up:
  gsl_interp_free(xinterp);
  gsl_interp_free(yinterp);
  gsl_interp_accel_free(xaccel);
  gsl_interp_accel_free(yaccel);

  return n;

}

//to match the different sizes of types encountered on different
//architectures:
typedef float read_real;

size_t boundary_list_g::read(FILE *fs) {
  int16_t yy, mon, dd, hh, min;
  float sec;
  time_class start;
  size_t nread;
  int32_t readn;

  real lond, latd;

  read_real *lon, *lat;

  if (x != NULL) delete [] x;
  if (y != NULL) delete [] y;
  if (hemi != NULL) delete [] hemi;

  nread=fread(&yy, sizeof(yy), 1, fs);
  nread+=fread(&mon, sizeof(mon), 1, fs);
  nread+=fread(&dd, sizeof(dd), 1, fs);
  nread+=fread(&hh, sizeof(hh), 1, fs);
  nread+=fread(&min, sizeof(min), 1, fs);
  nread+=fread(&sec, sizeof(sec), 1, fs);

  start.init(yy, mon, dd, hh, min, sec);

  tind=tgrid->interp(start);

  nread+=fread(&readn, sizeof(readn), 1, fs);
  n=readn;

  lon=new read_real[n+wrap_flag];
  lat=new read_real[n+wrap_flag];
  hemi=new short[n+wrap_flag];

  nread+=fread(lon, sizeof(read_real), n, fs);
  nread+=fread(lat, sizeof(read_real), n, fs);
/*
  for (long i=0; i<n; i++) {
    printf("%f %f\n", lon[i], lat[i]);
  }
*/

  x=new real[n+wrap_flag];
  y=new real[n+wrap_flag];

  //convert from lon-lat:
  for (long i=0; i<n; i++) {
    lond=(real) lon[i];
    latd=(real) lat[i];
    hemi[i]=0;
    tcoord2_2lonlat(x[i], y[i], -1, hemi[i], lond, latd);
  }

/*
  for (long i=0; i<n; i++) {
    printf("%f %f %d\n", x[i], y[i], hemi[i]);
  }
*/

  delete [] lon;
  delete [] lat;

  return nread;

}

size_t boundary_list_g::write(FILE *fs) {
  read_real lon[n], lat[n];
  int16_t yy, mon, dd, hh, min;
  float sec;
  time_class tcur;
  size_t nwrit;
  double frac;
  double lon1, lat1;
  int32_t readn;

  tgrid->get(tcur, tind);

  tcur.get_fields(yy, mon, dd, hh, min, sec);

  nwrit=fwrite(&yy, sizeof(yy), 1, fs);
  nwrit+=fwrite(&mon, sizeof(mon), 1, fs);
  nwrit+=fwrite(&dd, sizeof(dd), 1, fs);
  nwrit+=fwrite(&hh, sizeof(hh), 1, fs);
  nwrit+=fwrite(&min, sizeof(min), 1, fs);
  nwrit+=fwrite(&sec, sizeof(sec), 1, fs);

  readn=n;
  nwrit+=fwrite(&readn, sizeof(readn), 1, fs);

  //convert to lon-lat:
  for (long i=0; i<n; i++) {
    tcoord2_2lonlat(x[i], y[i], 1, hemi[i], lon1, lat1);
    lon[i]=lon1;
    lat[i]=lat1;
    //printf("%g %g %d %g %g\n", x[i], y[i], hemi[i], lon[i], lat[i]);
  }

  nwrit+=fwrite(lon, sizeof(read_real), n, fs);
  nwrit+=fwrite(lat, sizeof(read_real), n, fs);

  return nwrit;

}

long boundary_list_g::print(FILE *fs) {
  for (long i=0; i<n; i++) {
    printf("%10.6g %10.6g %d\n", x[i], y[i], hemi[i]);
  }
  return n;
}

