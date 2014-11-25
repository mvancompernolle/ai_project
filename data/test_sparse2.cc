#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>

#include "/rinax/storage/users/pmills/numerical_recipes/ran2.c"
#include "sparse.h"

float ** allocate_matrix(long m, long n) {
  float *temp, **mat;

  temp=new float[m*n];
  mat=new float *[m];
  for (long i=0; i<m; i++) {
    mat[i]=&temp[n*i];
  }

  return mat;
}

void delete_matrix(float ** mat) {
  delete mat[0];
}

float ** mat_mult(float **plier, float **cand, long m, long p, long n) {
  float **result;

  result=allocate_matrix(m, p);

  for (long i=0; i<m; i++) {
    for (long j=0; j<n; j++) {
      result[i][j]=0;
      for (long k=0; k<p; k++) {
        result[i][j]+=plier[i][k]*cand[k][j];
      }
    }
  }

  return result;
}

void mat_mult(float **plier, float **cand, float **result, long m, long p, long n) {

  for (long i=0; i<m; i++) {
    for (long j=0; j<n; j++) {
      result[i][j]=0;
      for (long k=0; k<p; k++) {
        result[i][j]+=plier[i][k]*cand[k][j];
      }
    }
  }

}


float ** read_matrix(FILE *fptr, long &m, long &n) {
  char line[200];
  float **mat;
  int line_pos;

  fgets(line, 200, fptr);
  sscanf(line, "%d %d\n", m, n);
  mat=allocate_matrix(m, n);

  for (long i=0; i<m; i++) {
    fgets(line, 200, fptr);
    for (long j=0; j<n; j++) {
      sscanf(&line[line_pos], "%f%n\n", &mat[i][j], &line_pos);
    }
  }

  return mat;
}

void print_matrix(FILE *fptr, float **mat, long m, long n) {

  for (long i=0; i<m; i++) {
    for (long j=0; j<n; j++) {
      fprintf(fptr, "%f ", mat[i][j]);
    }
    fprintf(fptr, "\n");
  }
}

int main(int argc, char *argv[]) {
  float data;
  long idum;
  sparse_matrix *a;
  sparse_matrix *f;
  sparse_matrix g;
  float **b;
  float **c;
  float **d;
  float **e;
  long size=50;

  double t1, t2;
  float dt1, dt2;
  timeb t;

  if (argc == 2) sscanf(argv[1], "%d", &idum);

  for (long s=0; s<15; s++) {

    b=allocate_matrix(size, size);
    c=allocate_matrix(size, size);
    d=allocate_matrix(size, size);

    for (long i=0; i<size; i++) {
      for (long j=0; j<size; j++) {
        data=ran2(&idum)*10;
        if (data < 7) data=0;
        b[i][j]=data;
        data=ran2(&idum)*10;
        if (data < 7) data=0;
        c[i][j]=data;
      }
    }

//    print_matrix(stdout, b, size, size);

    a=new sparse_matrix(b, size, size);

//    a->print(stdout);

    f=new sparse_matrix(c, size, size);

//    f->print(stdout);

    ftime(&t);
    t1=t.time+t.millitm/1000.;

    a->mat_mult(*f, g);

    ftime(&t);
    t2=t.time+t.millitm/1000.;

    dt1=t2-t1;

    e=(float **) g;

//    print_matrix(stdout, e, size, size);
    ftime(&t);
    t1=t.time+t.millitm/1000.;

    mat_mult(b, c, d, size, size, size);

    ftime(&t);
    t2=t.time+t.millitm/1000.;

    dt2=t2-t1;

    for (long j=0; j<size*size; j++) {
      float err;
      err=fabs(d[0][j]-e[0][j]);
      if (err > EPS) printf("error=%f\n", err);
    }

    delete a;
    delete f;
    delete_matrix(b);
    delete_matrix(c);
    delete_matrix(d);
    delete_matrix(e);

    printf("%d %f %f\n", size, dt1, dt2);
//    print_matrix(stdout, d, size, size);
    size=size+50;
  }


}

