#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "time_class.h"

#define MAGIC -55

int main(int argc, char **argv) {
  FILE *fs;

  int32_t n;
  int32_t npt;
  int32_t magic;
  time_class t;
  short yr, mon, dy, hr, min;
  float sec;

  char tstring[30];

  if (argc < 2) {
    printf("Usage: bevindex file\n");
    printf("\nLists all records contained in a binary file of contour advection results\n");
    printf("in columnar format: index, date and number of points\n");
    printf("\n");
    exit(-1);
  }

  fs=fopen(argv[1], "r");

  fread(&magic, sizeof(magic), 1, fs);
  if (magic != MAGIC) {
    fprintf(stderr, "File is of wrong type\n");
    exit(-1);
  }
  fread(&n, sizeof(n), 1, fs);

  for (int i=0; i<n; i++) {
    fread(&yr, sizeof(yr), 1, fs);
    fread(&mon, sizeof(mon), 1, fs);
    fread(&dy, sizeof(dy), 1, fs);
    fread(&hr, sizeof(hr), 1, fs);
    fread(&min, sizeof(min), 1, fs);
    fread(&sec, sizeof(sec), 1, fs);
    fread(&npt, sizeof(npt), 1, fs);
    t.init(yr, mon, dy, hr, min, sec);
    t.write_string(tstring);
    printf("%d %s %d\n", i, tstring, npt);
    fseek(fs, 8*npt, SEEK_CUR);
  }

}


