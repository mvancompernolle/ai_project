#include <stdio.h>
#include "time_class.h"

int main(int argc, char **argv){

  FILE *fs;
  float lon;
  float lat;
  time_class t;
  short yr, mon, dy, hr, min;
  float sec;
  long n=1;
  long nlon=360;
  long magic=-55;

  fs=fopen(argv[1], "w");
  
  sscanf(argv[2], "%g", &lat);

  t.read_string(argv[3]);
  t.get_fields(yr, mon, dy, hr, min, sec);

  fwrite(&magic, sizeof(long), 1, fs);
  fwrite(&n, sizeof(long), 1, fs);

  fwrite(&yr, sizeof(yr), 1, fs);
  fwrite(&mon, sizeof(mon), 1, fs);
  fwrite(&dy, sizeof(dy), 1, fs);
  fwrite(&hr, sizeof(hr), 1, fs);
  fwrite(&min, sizeof(min), 1, fs);
  fwrite(&sec, sizeof(sec), 1, fs);

  fwrite(&nlon, sizeof(long), 1, fs);

  for (long i=0; i<nlon; i++) {
    lon=i*360./nlon;
    fwrite(&lon, sizeof(lon), 1, fs);
  }
  for (long i=0; i<nlon; i++) fwrite(&lat, sizeof(lat), 1, fs);

  fclose(fs);

}

