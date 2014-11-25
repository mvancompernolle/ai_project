#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "time_class.h"
#include "peteys_tmpl_lib.h"

#include "coordtran.h"
#include "tracer_anal.h"
#include "ctraj_defaults.h"
#include "av.h"

#define MAXN 10000
#define MAXLL 200
#define TLEN 30

int main(int argc, char **argv) {

  FILE *fs;
  char *tfile;
  char *datefile;
  char *measurement_file;
  char *outfile;
  char c;
  long ncon;

  long fsize;

  float **q;

  int32_t ngrid=NGRID_Q;
  int32_t np;
  int32_t nall;			//total number of tracer fields

  ind_type i0, N;		//start point, number of field to interpolate

  time_class *t;
  char tstring[TLEN];
  char line[MAXLL];

  //for reading and storing the samples:
  long nsamp;
  meas_data *samp;

  float *qint;		//interpolated vector at approximate lead time

  //for calculating lead times:
  float lead;
  float window;
  time_class tf;		//date at end of lead time
  time_class t1, t2;		//measurement time window
  double l2;			//for truncating matrix array
  long N2;			//size of matrix array to use
  int32_t dwid;			//width of date field

  //measurement data within window:
  meas_data *samp_w;
  long nsamp_w;

  int order=2;			//order of regression
  float *coeff;			//regression coefficients
  int lead_ind;			//index of field at lead time
  float qval_p;			//tracer value to various powers

  i0=0;
  N=-1;

  //parse the command line arguments:
  while ((c = getopt(argc, argv, "d:n:o:O:N:")) != -1) {
    switch (c) {
      case ('N'):
        ncon=sscanf(optarg, "%d", &N);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -N %s", optarg);
          exit(2);
        }
        break;
      case ('O'):
        ncon=sscanf(optarg, "%d", &i0);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -O %s", optarg);
          exit(2);
        }
        break;
      case ('o'):
        ncon=sscanf(optarg, "%d", &order);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -o %s", optarg);
          exit(2);
        }
        break;
      case ('n'):
        ncon=sscanf(optarg, "%d", &ngrid);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -n %s", optarg);
          exit(2);
        }
        break;
      case ('d'):
        ncon=sscanf(optarg, "%d", &dwid);
        if (ncon != 1) {
          fprintf(stderr, "Warning: garbled command option argument: -d %s", optarg);
          exit(2);
        }
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

  if (argc<5) {
    printf("\n");
    printf("usage: proxy_tracer [-O i0] [-N N] [-o order] [-n ngrid] \n");
    printf("                  tfile dates measurements window outfile\n");
    printf("\n");
    printf("where:\n");
    printf("  matfile      = binary file containing tracer field\n");
    printf("  dates        = ASCII file containing dates corresponding to each field\n");
    printf("  measurements = ASCII file containing measurements and locations\n");
    printf("  window       = measurement window in days\n");
    printf("  outfile      = binary file containing interpolated tracer field\n");
    printf("\n");
    printf("  i0           = index for start point [%d]\n", i0);
    printf("  N            = interpolate this number of fields\n");
    printf("  order        = order of fit [%d]\n", order);
    printf("  n            = grid points per side [%d]\n", ngrid);
    printf("\n");
    return -1;
  }

  tfile=argv[0];
  datefile=argv[1];
  measurement_file=argv[2];
  sscanf(argv[3], "%f", &window);
  outfile=argv[4];

  //read in the array of tracers:
  fprintf(stderr, "Reading file: %s\n", tfile);
  fs=fopen(tfile, "r");
  fseek(fs, 0, SEEK_END);
  fsize=ftell(fs);

  //calculate total number of grid points:
  np=calc_nmap(ngrid);

  if (fsize % (sizeof(float)*np) != 0) {
    fprintf(stderr, "Warning: not an even number of tracer fields in file, %s\n", tfile);
  }

  nall=fsize/sizeof(float)/np;
  q=new float*[nall];

  q[0]=new float[nall*np];
  for (int i=1; i<nall; i++) q[i]=q[0]+i*np*2;

  fseek(fs, 0, SEEK_SET);

  fread(q[0], sizeof(float), np*nall*2, fs);
  fprintf(stderr, "%d tracer fields read in\n", nall);

  fclose(fs);

  //read in the dates:
  t=new time_class[nall];

  fs=fopen(datefile, "r");
  //fgets(line, MAXLL, fs);		//current versions of "tracer"
					//do NOT print out a header...

  for (long i=0; i<nall; i++) {
    int ind;
    fgets(line, MAXLL, fs);
    sscanf(line, "%d %s", &ind, tstring);
    t[i].read_string(tstring);
  }
  fclose(fs);

  if (N == -1) {
    tf=t[nall-1];
    tf.add(-lead);
    N=bin_search(t, nall, tf, -1)-i0;
  }

  //read in measurements:
  printf("Reading in measurements from, %s\n", measurement_file);
  samp=read_meas(measurement_file, &nsamp, dwid);

  qint=new float[np];
  fs=fopen(outfile, "w");
  for (long i=i0; i<N+i0; i++) {
    //calculate lead times:
    tf=t[i];
    t1=tf;
    t1.add(-window);
    t2=tf;
    t2.add(window);
    l2=interpolate(t, nall, tf, -1);
    N2=ceil(l2);

    t1.write_string(tstring);
    fprintf(stderr, "%d: interpolating tracer between %s ", i, tstring);
    t2.write_string(tstring);
    fprintf(stderr, "and %s\n", tstring);

    //select measurements:
    samp_w=select_meas(t1, t2, samp, nsamp, &nsamp_w);

    coeff=proxy_tracer(q, ngrid, t, nall, samp_w, nsamp_w, order);

    //calculate interpolated tracer field:
    for (int j=0; j<np; j++) {
      qint[j]=0;
      qval_p=1;
      for (int k=0; k<=order; k++) {
        qint[j]+=coeff[k]*qval_p;
        qval_p*=q[N2][j];
      }
    }

    //output final, interpolated field:
    t[N2].write_string(tstring);
    printf("%d %s\n", i-i0, tstring);
    //printf("%d: writing vector of length %d\n", i, np);
    fwrite(qint, sizeof(float), np*2, fs);

    delete [] samp_w;
  }

  delete [] qint;


  //qvec=tracer_multiply(matall+i0, nall-i0, q0);

  //fwrite(qvec[0], sizeof(float), n*(nall-i0), fs);
  fclose(fs);


  delete [] q[0];
  delete [] q;
  delete [] t;
  delete [] samp;
}

