#include <assert.h>

#include <stdint.h>

//#include "nr.h"
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"

#include "peteys_tmpl_lib.h"

#include "tcoord_defs.h"
#include "boundary_list.h"

/*
simple<float> *global_xgrid;
simple<float> *global_ygrid;
dependent_swap<float> *global_u;
dependent_swap<float> *global_v;
*/

int derivs(real tind, real x[], real dxdt[], void *param) {
  double xind, yind;
  float dx1dt, dx2dt;
  simple<float> *xgrid;
  simple<float> *ygrid;
  dependent_swap<float> *u;
  dependent_swap<float> *v;

  dataset **param2=(dataset **) param;
  xgrid=(simple<float> *) param2[0];
  ygrid=(simple<float> *) param2[1];
  u=(dependent_swap<float> *) param2[2];
  v=(dependent_swap<float> *) param2[3];

  xind=xgrid->interp(x[0]);
  yind=ygrid->interp(x[1]);
  u->interpol(dx1dt, xind, yind, 0., (double) tind);
  v->interpol(dx2dt, xind, yind, 0., (double) tind);
  dxdt[0]=dx1dt;
  dxdt[1]=dx2dt;
  //printf("%lf %f %f\n", tind, dxdt[0], dxdt[1]);

}

//default constructor:
boundary_list::boundary_list() {
  xgrid=NULL;
  ygrid=NULL;
  zgrid=NULL;
  tgrid=NULL;
  u=NULL;
  v=NULL;
  vfs=NULL;
  result=NULL;
  x=NULL;
  y=NULL;
  n=0;
  tind=0;
  tstep=0;
  nrk=0;
  min_spac=0;
  thresh_arc=0;
  wrap_flag=0;
}

boundary_list::boundary_list(char *filename) {
  long loc, dum;

  printf("Opening: %s\n", filename);
  vfs=fopen(filename, "r");

  all.read(vfs);
  loc=all.search_var("xgrid", dum);
  xgrid=(simple<float> *) all.get_var(loc);
  loc=all.search_var("ygrid", dum);
  ygrid=(simple<float> *) all.get_var(loc);
  loc=all.search_var("zgrid", dum);
  zgrid=(simple<float> *) all.get_var(loc);
  loc=all.search_var("time", dum);
  tgrid=(simple<time_class> *) all.get_var(loc);
  loc=all.search_var("u", dum);
  u=(dependent_swap<float> *) all.get_var(loc);
  loc=all.search_var("v", dum);
  v=(dependent_swap<float> *) all.get_var(loc);

  param[0]=xgrid;
  param[1]=ygrid;
  param[2]=u;
  param[3]=v;

  x=NULL;
  y=NULL;
  n=0;

  result=NULL;
  nrk=0;
  tstep=0;

  wrap_flag=1;

}

void boundary_list::set_parm(real tcoarse, int32_t nfine, real tarc,
			real minspc, real maxspc) {

  tstep=tcoarse;
  nrk=nfine;
  thresh_arc=tarc;
  min_spac=minspc;
  max_spac=maxspc;

  if (result != NULL) {
    delete [] result[0];
    delete [] result;
  }

  result=new real *[nrk+1];
  result[0]=new real[(nrk+1)*2];
  for (int32_t i=0; i<=nrk; i++) result[i]=result[0]+2*i;
}

time_class boundary_list::set(interpol_index ind) {
  time_class t0;
  tind=ind;
  tgrid->get(t0, tind);
  return t0;
}

interpol_index boundary_list::set(time_class t0) {
  tind=tgrid->interp(t0);
  return tind;
}

interpol_index boundary_list::get_tind() {
  return tind;
}

time_class boundary_list::get_t() {
  time_class t0;
  tgrid->get(t0, tind);
  return t0;
}

ind_type boundary_list::nt() {
	return tgrid->nel();
}

long boundary_list::init_circle(real x0, real y0, real r) {
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


void boundary_list::wrap_on() {
  real *xnew;
  real *ynew;

  if (!wrap_flag) {
    wrap_flag=1;
    if (n != 0) {
      xnew=new real[n+1];
      ynew=new real[n+1];
      for (long i=0; i<n; i++) {
        xnew[i]=x[i];
        ynew[i]=y[i];
      }
      delete [] x;
      delete [] y;
      x=xnew;
      y=ynew;
    }
  }
}

void boundary_list::wrap_off() {
//  if (wrap_flag) {
    wrap_flag=0;
/*    if (n!=0) {
      for (long i=0; i<n; i++) {
        x[i]=x[i+1];
        y[i]=y[i+1];
      }
    }
  }*/
}


boundary_list::~boundary_list() {
  delete [] x;
  delete [] y;

  delete u;
  delete v;
  delete xgrid;
  delete ygrid;
  delete zgrid;
  delete tgrid;

  if (vfs != NULL ) fclose(vfs);
}

time_class boundary_list::advance() {
  real x0[2];
  real tstepfine=tstep/nrk;
  ind_type tindl;
  double frac;
  time_class t0, t1;

/*
  //pass the grids and the velocity fields to the global variabls:
  global_xgrid=xgrid;
  global_ygrid=ygrid;
  global_u=u;
  global_v=v;
*/

  for (long i=0; i<n; i++) {
    x0[0]=x[i];
    x0[1]=y[i];
    rk_dumb((real) tind, x0, 2L, tstepfine, nrk, (real **) result, (void *) param, &derivs);
    x[i]=result[nrk][0];
    y[i]=result[nrk][1];
  }

  tind+=tstep;

  tindl=(ind_type) tind;
  frac=tind-(double) tindl;

  tgrid->get(t0, tindl);
  tgrid->get(t1, tindl+1);

  t1=t1-t0;
  t1=t1*frac;
  t1=t0+t1;

  return t1;
}

long boundary_list::fix() {
  long nn=n+1*wrap_flag;
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
  }

  s[0]=0;
  offset=0;
  for (long i=1; i<nn; i++) {
    if (offset != 0) {
      //because of any redundant nodes, all the nodes must be shifted:
      j=i-offset;
      x[j]=x[i];
      y[j]=y[i];
    } else {
      j=i;
    }
    s[j]=s[j-1]+sqrt(tcoord_ds2(x[j-1], y[j-1], x[j], y[j]));
    //check for nodes made redundant by round-off:
    if (s[j]-s[j-1] <= 0) {
      printf("Warning: removing %ldth node made redundant by round-off\n", i);
      offset++;
    }
  }
  n-=offset;
  nn-=offset;

  //do a cubic spline interpolation on both variables:
  if (wrap_flag) {
    //if the contour is closed, use periodic interpolation:
    spltype=(gsl_interp_type *) gsl_interp_cspline_periodic;
//    printf("dx0ds=%g; dy0ds=%g\n", dx0, dy0);
  } else {
    //otherwise, use a "natural" spline:
    spltype=(gsl_interp_type *)gsl_interp_cspline;
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
    assert(dels>0);
    //if the point spacing is less than the threshold, either
    //remove next point or add no new:
    if (dstotal < min_spac) {
      if (2*dstotal < min_spac) {
        nnew[i-1]=-1;		//remove point
      } else {
        nnew[i-1]=0;
      }
    } else {
      dstotal=0;
      gsl_interp_eval_deriv2_e(xinterp, s, x, (s[i-1]+s[i])/2, xaccel, &dx2);
      gsl_interp_eval_deriv2_e(xinterp, s, y, (s[i-1]+s[i])/2, yaccel, &dy2);
      //dx2av=(dx2[i-1]+dx2[i])/2;
      //dy2av=(dy2[i-1]+dy2[i])/2;
      dangle+=sqrt(dx2*dx2+dy2*dy2)*dels;

//      printf("r=%g\n", dels/dangle);
    
      //if the accumulated angle of arc is greater than the threshold,
      //add new points, otherwise, remove the next point...
      if (dangle*2 < thresh_arc) {
        nnew[i-1]=-1;
      } else {
        nnew[i-1]=(short) (dangle/thresh_arc-0.5);
        //dangle=dangle-(nnew[i-1]+0.5)*thresh_arc;
	dangle=0;
      }
    }
    totaln+=nnew[i-1]+1;
//    printf("%d, %g, %g, %g, %g, %d\n", i, s[i], dx2[i], dy2[i], dangle, nnew[i-1]);
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
      for (long m=1; m <= nnew[i-1]; m++) {
        //splint(s-1, x-1, dx2-1, nn, s[k]+m*dels, &xnew[j]);
        //splint(s-1, y-1, dy2-1, nn, s[k]+m*dels, &ynew[j]);
	gsl_interp_eval_e(xinterp, s, x, s[k]+m*dels, xaccel, xnew+j);
	gsl_interp_eval_e(yinterp, s, y, s[k]+m*dels, yaccel, ynew+j);
	j++;
      }
      xnew[j]=x[i];
      ynew[j]=y[i];
      k=i;
      j++;
    } else if (nnew[i-1] == 0) {
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
  n=totaln-wrap_flag;

  //clean up:
  gsl_interp_free(xinterp);
  gsl_interp_free(yinterp);
  gsl_interp_accel_free(xaccel);
  gsl_interp_accel_free(yaccel);

  return n;

}

typedef float read_real;

size_t boundary_list::read(FILE *fs) {
  int16_t yy, mon, dd, hh, min;
  float sec;
  time_class start;
  size_t nread;
  int32_t readn;

  read_real *x1, *y1;

  if (x != NULL) delete [] x;
  if (y != NULL) delete [] y;

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

  x1=new read_real[n+1*wrap_flag];
  y1=new read_real[n+1*wrap_flag];

  nread+=fread(x1, sizeof(read_real), n, fs);
  nread+=fread(y1, sizeof(read_real), n, fs);

  x=new real[n+1*wrap_flag];
  y=new real[n+1*wrap_flag];

  for (long i=0; i<n; i++) {
    x[i]=(real) x1[i];
    y[i]=(real) y1[i];
  }

  delete [] x1;
  delete [] y1;

  return nread;

}

size_t boundary_list::write(FILE *fs) {
  int16_t yy, mon, dd, hh, min;
  float sec;
  time_class tcur, t0, t1;
  size_t nwrit;
  ind_type tindl;
  double frac;
  int32_t readn;

  read_real x1[n];
  read_real y1[n];

  //copy contour into floating arrays:
  for (long i=0; i<n; i++) {
    x1[i]=(read_real) x[i];
    y1[i]=(read_real) y[i];
  }

  tindl=(long) tind;
  frac=tind-(double) tindl;

  tgrid->get(t0, tindl);
  tgrid->get(t1, tindl+1);

  t1=(t1-t0);
  t1=t1*frac;
  tcur=t0+t1;
  tcur.get_fields(yy, mon, dd, hh, min, sec);

  nwrit=fwrite(&yy, sizeof(yy), 1, fs);
  nwrit+=fwrite(&mon, sizeof(mon), 1, fs);
  nwrit+=fwrite(&dd, sizeof(dd), 1, fs);
  nwrit+=fwrite(&hh, sizeof(hh), 1, fs);
  nwrit+=fwrite(&min, sizeof(min), 1, fs);
  nwrit+=fwrite(&sec, sizeof(sec), 1, fs);

  readn=n;
  nwrit+=fwrite(&readn, sizeof(readn), 1, fs);

  nwrit+=fwrite(x1, sizeof(read_real), n, fs);
  nwrit+=fwrite(y1, sizeof(read_real), n, fs);

  return nwrit;

}

long boundary_list::print(FILE *fs) {
  for (long i=0; i<n; i++) {
    printf("%10.6g %10.6g\n", x[i], y[i]);
  }
  return n;
}

