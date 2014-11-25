#include "read_field_file.h"
#include "dependent_swap.h"
#include "composite_dataset.h"

//converts a "field file" to a "dataset file":

int main(int argc, char *argv[]) {
  char *infile;
  char *outfile;
//  char *swap_file="/rinax/storage/users/pmills/dataset.swp";
  FILE *optr;
  FILE *swaptr;

  simple<float> *xgrid;
  simple<float> *ygrid;
  simple<float> *zgrid;
  simple<time_class> *time;
  dependent_swap<float> **fdata;
  dependent_swap<float> *v1;
  dependent_swap<float> *v2;
  char **var;
  num nvar;
  long nel;

  composite_dataset all;
//  composite_dataset comp;
  long loc, dum;
//  float value1, value2;

  if (anumrgc!=3) return 1;

  infile=argv[1];
  outfile=argv[2];

  read_field_file_head(infile, var, nvar, xgrid, ygrid, zgrid, time);

  loc=all.add_var("xgrid");
  all.cvar(loc, (dataset *) xgrid);
  loc=all.add_var("ygrid");
  all.cvar(loc, (dataset *) ygrid);
  loc=all.add_var("zgrid");
  all.cvar(loc, (dataset *) zgrid);
  loc=all.add_var("time");
  all.cvar(loc, (dataset *) time);

  //create the dependent datasets to hold the fields and add them in to the pot:
  fdata=new dependent_swap<float> *[nvar];
  for (long i=0; i<nvar; i++) {
    fdata[i]=new dependent_swap<float>(xgrid, ygrid, zgrid, time);
    loc=all.add_var(var[i]);
    all.cvar(loc, (dataset *) fdata[i]);
  }

  all.print();

  //write the data to the output file (the fields will be left blank):
  //open the output file:
  optr=fopen(outfile, "w");
  all.write(optr);

  //transfer the field data from the input file to the output file:
  for (long i=0; i<nvar; i++) {
    read_field_file_data(infile, var[i], fdata[i]);
  }

//  all.print();
  fclose(optr);

/*  //test file to make sure it works:
  optr=fopen(outfile, "r");
  comp.read(optr);
  fclose(optr);

//  comp.print();

  loc=comp.search_var("v", dum);
  v2=(dependent<float> *) comp.get_var(loc);
  v1=fdata[1];

  nel=v2->nel();
  printf("%d data elements found in original v field\n", v1->nel());
  printf("%d data elements found in re-read v field\n", nel);
  for (long i=0; i<nel; i++) {
    v1->get_1d(value1, i);
    v2->get_1d(value2, i);
    if (value1 != value2) printf("Elements at location %d differ\n", i);
  }
*/

  //this should be done by the composite dataset class, but since it
  //doesn't work:
  //clean up:
  for (long i=0; i<nvar; i++) delete fdata[i];
  delete fdata;
  delete xgrid;
  delete ygrid;
  delete zgrid;
  delete time;

  return 0;
}
