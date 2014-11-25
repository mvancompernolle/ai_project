//some simple class definitions for dealing with sparse matrices:

struct col_element {
  long j;
  float * value;
};

struct row_array{
  long nel;
  col_element *row;
};

class sparse_matrix {
  protected:
    row_array *matrix;
    long m;
    long n;

  public:
    sparse_matrix();
    sparse_matrix(long min, long nin);
    sparse_matrix(float **non, long min, long nin);

    long add_el(float val, long i, long j);
    float operator [ ] (long i, long j);

    vect_mult(float *cand, float *result);	//multiply with a vector
    mat_mult(float **cand, float **result);	//multiply with a non-sparse matrix

    float ** operator (float **) ();		//convert to non-sparse

}

sparse_matrix() {
  m=0;
  n=0;
}

sparse_matrix(long min, long nin) {
  m=min;
  n=nin;

  matrix=new row_array[m];
  for (long i=0; i<m; i++) {
    matrix[i].nel=0;
    matrix[i].row=NULL;
  }
}

sparse_matrix(float **non, long min, long nin) {
  float **row;
  long nel;
  m=min;
  n=nin;

  matrix=new row_array[m];
  row=new float[n];
  for (i=0; i<m; i++) {
    nel=0;
    for (j=0; j<n; j++) {
      if (non[i][j]<EPS) {
        row[j]=non[i][j];
	nel++;
      }
    }
    matrix[i].row=new col_element[nel]
    matrix.nel=nel;
    for (j=0; j<nel; j++) matrix[i].row[j].value=row[j];
  }
}
