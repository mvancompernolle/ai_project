#include "dep_float_int_c.h"

dependent_float_int_coeff::dependent_float_int_coeff(stype *s1, stype *s2) {
	dependent<float>::dependent<float>();

	s1->add_dependent(this, 0);
	s2->add_dependent(this, 1);

	rank=2;
	dim=new ind_type[rank];
	dependencies=new stype * [rank];

	dim[0]=s1->nel();
	dim[1]=s2->nel();
	dependencies[0]=s1;
	dependencies[1]=s2;

	n_data=dim[0]*dim[1];
//	if (n_data==0) return;
//	data=new dtype[n_data];
//	for (long i=0; i<n_data; i++) data[i]=missing;
}

errtype dependent_float_int_coeff::interpol_coeff(interpol_index *indices,
		sub_1d_type *subscripts, double *coeffs) {

  ind_type lind[rank], hind[rank];
  ind_type l, h;
  sub_1d_type sub, mult;
  long n=1 << rank;
  long order;
  interpol_index frac;
  long norm;
  double weight;

  if (n_data == 0) return NO_DATA;

//  printf("(%f, %f)\n", indices[0], indices[1]);

  //determine the upper and lower values for the indices:
  norm=1;
  for (rank_type i=0; i<rank; i++) {
    if (dim[i]==1) {
//      l=0;
//      h=0;
      norm*=2;		//also determine the normalization coeff.
    } else {
      l=(ind_type) indices[i];
      if (l>=dim[i]-1) l=dim[i]-2; else if (l<0) l=0;
      lind[i]=l;
      hind[i]=l+1;
    }
  }

  //calculate the weights and apply them:
  for (long i=0; i<n; i++) {
    weight=1;
    sub=0;
    mult=1;
    for (rank_type j=0; j<rank; j++) {
      if (dim[j] == 1) continue;
      order= 1 << j;
      if ((i & order) == 0) {
        frac=(interpol_index) hind[j]-indices[j];
        sub=sub+mult*lind[j];
      } else {
        frac=indices[j]-(interpol_index) lind[j];
        sub=sub+mult*hind[j];
      }
      weight*=frac;
      mult=mult*dim[j];
   }
   subscripts[i]=sub;
   coeffs[i]=(float) (weight/(double) norm);
//   printf("Weight %d= %f\n", i, weight/(double) norm);
  }

//  printf("Normalization coef: %f\n", norm);

  return NO_PROBLEM;
}
