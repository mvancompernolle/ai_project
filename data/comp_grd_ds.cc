#include <math.h>

#include "read_field_file.h"
#include "/home/home02/pmills/datasets/dependent_swap.h"
#include "/home/home02/pmills/datasets/composite_dataset.h"

//compares a "field file" to a "dataset file":

int main(int argc, char *argv[]) {
  char *infile;
  char *outfile;
  char *var;
  char *swap_file="/rinax/storage/users/pmills/dataset.swp";
  FILE *optr;
  FILE *swaptr;

  simple<float> *xgrid;
  simple<float> *ygrid;
  simple<float> *zgrid;
  simple<time> *time;
  dependent_swap<float> *fdata1;
  dependent_swap<float> *fdata2;
  char **var_all;
  long nvar;
  long nel;

  composite_dataset all;
//  composite_dataset comp;
  long loc, dum;
  float value1, value2;

  if (argc!=4) return 1;

  infile=argv[1];
  outfile=argv[2];
  var=argv[3];

  read_field_file_head(infile, var_all, nvar, xgrid, ygrid, zgrid, time);

  fdata1=new dependent_swap<float>(xgrid, ygrid, zgrid, time);
  //open the swap file:
  swaptr=fopen(swap_file, "w+");

  fdata1->read(swaptr);
  read_field_file_data(infile, var, fdata1);

//  all.print();

  optr=fopen(outfile, "r");
  all.read(optr);

  //test file to make sure it works:

//  comp.print();

  loc=all.search_var(var, dum);
  fdata2=(dependent_swap<float> *) all.get_var(loc);

  nel=fdata2->nel();
  printf("%d data elements found in original v field\n", fdata1->nel());
  printf("%d data elements found in re-read v field\n", nel);
  for (long i=0; i<nel; i++) {
    float diff;
    fdata1->get_1d(value1, i);
    fdata2->get_1d(value2, i);
    diff=fabs(value1-value2);
    if (diff != 0) printf("Elements at location %d differ by %g\n", i, diff);
  }

  fclose(optr);
  fclose(swaptr);

  //this should be done by the composite dataset class, but since it
  //doesn't work:
  delete fdata1;
  delete fdata2;
  delete xgrid;
  delete ygrid;
  delete zgrid;
  delete time;

  return 0;
}
