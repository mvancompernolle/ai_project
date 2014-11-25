//#include <stdio.h>
#include <cstring>

#include "../clib/time.h"
#include "../datasets/simple_temp.h"
#include "../datasets/dependent_temp.h"

#define LINE_LENGTH 100

extern int read_field_file(char *, char *, dependent<float> *&, simple<float> *&,
		simple<float> *&, simple<float> *&, simple<time> *&);

int main() {

//  char *filename="/rinax/storage/users/pmills/gridded_data/sh1999_400K.grd2";
  char *filename="/rinax/storage/users/pmills/velocity_fields/ecmwf_jan1999_400K.grd";
  simple<float> *lon;
  simple<float> *lat;
  simple<float> *lev;
  simple<time> *date;
  dependent<float> *data;
  int err;
  long i, j;
  time tvalue;
  char *tstring;
  float value;
  double lonval, latval, tind;
  char line[LINE_LENGTH];

  err=read_field_file(filename, "u", data, lon, lat, lev, date);

  for (i=0; i<lon->nel(); i++) {
    lon->get(value, i);
    printf("Longitude # %d: %f\n", i, value);
  }

  for (i=0; i<date->nel(); i++) {
    date->get(tvalue, i);
    tstring=(char *) tvalue;
    printf("Time # %d: %s\n", i, tstring);
    delete tstring;
  }

  do {
    fgets(line, LINE_LENGTH, stdin);
    printf("%s\n", line);
    sscanf(line, "%lf %lf %lf", &lonval, &latval, &tind);
//    sscanf("%f", &lonval);
//    scanf("%f", &latval);
//    scanf("%f", &tind);
    printf("Longitude: %f; latitude: %f; time index: %f\n", lonval, latval, tind);
    data->interpol(value, lon->interp(lonval), lat->interp(latval), 0, tind);
    printf("%g\n", value);
  } while (1);

  delete lon;
  delete lat;
  delete lev;
  delete date;

  return 0;

}
