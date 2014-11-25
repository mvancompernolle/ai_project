//some simple class definitions for dealing with sparse matrices:

#include "/home/home02/pmills/clib/heapsort_tmpl.cc"
#include "/home/home02/pmills/clib/bin_search.cc"

#include "sparse.h"

template long bin_search<sparse_matrix_element>
		(sparse_matrix_element *, long, sparse_matrix_element, long);
template void heapsort_inplace<sparse_matrix_element>
		(sparse_matrix_element *, long);


sparse_matrix::sparse_matrix() {
  m=0;
  n=0;
  nel=0;
  array_size=1;
  last_search=-1;
  update_flag=1;
  matrix=new sparse_matrix_element;
}

sparse_matrix::~sparse_matrix() {
  delete matrix;
}

sparse_matrix::sparse_matrix(long min, long nin) {
  m=min;
  n=nin;
  nel=0;
  array_size=1;
  last_search=-1;
  update_flag=1;
  matrix=new sparse_matrix_element;

}

sparse_matrix::sparse_matrix(float **non, long min, long nin) {
  float val;
  long nel;
  m=min;
  n=nin;

  matrix=new sparse_matrix_element[m];
  for (long i=0; i<m; i++) {
    for (long j=0; j<n; j++) {
      val=non[i][j];
      if (val<EPS) {
        add_el(val, i, j);
      }
    }
  }
}

void sparse_matrix::extend(long size) {
  long new_size;

  new_size=size+nel;
  if (array_size < new_size) {
    sparse_matrix_element *new_matrix;

    array_size=new_size;
    new_matrix=new sparse_matrix_element[array_size];
    for (long k=0; k<nel; k++) new_matrix[k]=matrix[k];
    delete matrix;
    matrix=new_matrix;
  }
}

long sparse_matrix::add_el(float value, long i, long j) {
  update_flag=0;

  if (i>m) m=i;
  if (j>n) n=j;

  if (nel >= array_size) {
    sparse_matrix_element *new_matrix;

    array_size*=2;
    new_matrix=new sparse_matrix_element[array_size];
    for (long k=0; k<nel; k++) new_matrix[k]=matrix[k];
    delete matrix;
    matrix=new_matrix;
  }

  matrix[nel].i=i;
  matrix[nel].j=j;
  matrix[nel].value=value;
  return nel++;
}

long sparse_matrix::cel(float value, long i, long j) {
  sparse_matrix_element search_blank(i, j, 0);

  if (update_flag == 0) update();

  last_search=bin_search(matrix, nel, search_blank, last_search);
  if (last_search==-1) return add_el(value, i, j);
  matrix[last_search].i=i;
  matrix[last_search].j=j;
  matrix[last_search].value=value;

  return last_search;
}

float sparse_matrix::operator ( ) (long i, long j) {
  sparse_matrix_element search_blank(i, j, 0);

  if (nel == 0) return 0;
  if (update_flag==0) update();
  last_search=bin_search(matrix, nel, search_blank, last_search);
  if (last_search == -1) return 0;

  return matrix[last_search].value;
}

void sparse_matrix::update() {
  long i;
  //sort the elements:
  heapsort_inplace(matrix, nel);
  for (i=0; matrix[i].value<EPS; i++);

  //remove zero elements:
  if (i==nel) {
    nel=0;
  } else {
    for (long j=i; j<nel; j++) matrix[j-i]=matrix[j];
  }
  update_flag=1;
}

//perform a vector multiplication:
void sparse_matrix::vect_mult(float *cand, float *result) {
  for (long i=0; i<m; i++) result[i]=0;
  for (long i=0; i<nel; i++) {
    result[matrix[i].i]+=matrix[i].value*cand[matrix[i].j];
  }
}

//perform a matrix multiplication on a non-sparse matrix:
void sparse_matrix::mat_mult(float **cand, float **result) {
  for (long i=0; i<m; i++) {
    for (long j=0; j<n; j++) {
      result[i][j]=0;
    }
  }

  for (long j=0; j<m; j++) {
    for (long i=0; i<nel; i++) {
      result[matrix[i].i][j]+=matrix[i].value*cand[matrix[i].j][j];
    }
  }
}

void sparse_matrix::mat_mult(sparse_matrix &cand, sparse_matrix &result) {
  sparse_matrix tran_cand;
  float new_val;
  long q, r;
  long qold;
  long this_i_old, tran_cand_i_old;
  long this_j, tran_cand_j;

  update();

  //delete everthing in result:
  delete result.matrix;
  result.m=m;
  result.n=cand.n;
  result.nel=0;
  result.array_size=array_size;
  result.matrix=new sparse_matrix_element[result.array_size];

  //transpose the multiplicand:
  cand.transpose(tran_cand);
  tran_cand.update();

  q=0;
  while (q<nel) {
    this_i_old=matrix[q].i;
    qold=q;
    while (matrix[q].i == this_i_old) {
      r=0;
      while (r<tran_cand.nel) {
        tran_cand_i_old=tran_cand.matrix[r].i;
	new_val=0;
	q=qold;
        while (tran_cand.matrix[r].i == tran_cand_i_old) {
	  this_j=matrix[r].j;
	  tran_cand_j=tran_cand.matrix[r].j;
	  if (this_j == tran_cand_j) {
	    new_val+=matrix[q].value*tran_cand.matrix[r].value;
	    q++;
	    r++;
	  } else if (this_j < tran_cand_j) {
	    q++;
	  } else {
	    r++;
	  }
	}
	result.add_el(new_val, this_i_old, tran_cand_i_old);
      }
    }
  }
}


//find the transpose:
void sparse_matrix::transpose(sparse_matrix &tran) {

  delete tran.matrix;

  tran.m=n;
  tran.n=m;
  tran.nel=nel;
  array_size=nel;
  last_search=-1;
  tran.matrix=new sparse_matrix_element[nel];

  for (long i=0; i<nel; i++) {
    tran.add_el(matrix[i].value, matrix[i].j, matrix[i].i);
  }
}

void sparse_matrix::transpose(float **tran) {

  for (long i=0; i<n; i++) {
    for (long j=0; j<m; j++) {
      tran[i][j]=0;
    }
  }
  for (long i=0; i<nel; i++) {
    tran[matrix[i].j][matrix[i].i]=matrix[i].value;
  }
}

sparse_matrix::operator float ** () {
  float **non;

  non=new float *[m];
  for (long i=0; i<m; i++) {
    non[i]=new float[n];
    for (long j=0; j<n; j++) {
      non[i][j]=0;
    }
  }
  for (long i=0; i<nel; i++) {
    non[matrix[i].i][matrix[i].j]=matrix[i].value;
  }

  return non;
}

long sparse_matrix::read(FILE *fptr) {
  long nread;
  nread=fread(&m, 1, sizeof(m), fptr);
  nread+=fread(&n, 1, sizeof(n), fptr);
  nread+=fread(&nel, 1, sizeof(nel), fptr);
  delete matrix;
  array_size=nel;
  matrix=new sparse_matrix_element[nel];
  nread+=fread(matrix, 1, sizeof(sparse_matrix_element)*nel, fptr);
}

long sparse_matrix::write(FILE *fptr) {
  long nwritten;

  update();
  nwritten+=fwrite(&m, 1, sizeof(m), fptr);
  nwritten+=fwrite(&n, 1, sizeof(n), fptr);
  nwritten+=fwrite(&nel, 1, sizeof(nel), fptr);
  nwritten+=fwrite(matrix, 1, sizeof(sparse_matrix_element)*nel, fptr);

  return nwritten;
}

void sparse_matrix::print(FILE *fptr) {
  update();
  
  for (long i=0; i<nel; i++) {
    fprintf("%d %d %f\n", matrix[i].i, matrix[j].j, matrix[i].value);
  }
}
