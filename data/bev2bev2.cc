#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "error_codes.h"
#include "parse_command_opts.h"
#include "ctraj_defaults.h"

#include "time_class.h"
#include "tcoord_defs.h"

#define MAGIC -55

int main(int argc, char **argv) {
  FILE *fs;

  int32_t n1;
  int32_t npt1;
  int32_t magic;

  int32_t ind;
  short yr, mon, dy, hr, min;
  float sec;

  int64_t bs;
  long bs2;
  long n2;
  long npt2;
  long zero=0;

  float *lat, *lon;
  float x, y;
  short hemi;

  time_class *t;

  void *optarg[10];
  int flag[10];

  ind=0;
  bs=C3_BLOCK_SIZE;

  optarg[0]=&ind;
  optarg[1]=&bs;

  argc=parse_command_opts(argc, argv, "1B?", "%d%ld%", optarg, flag, 1);

  if (argc != 3 || flag[2]) {
    FILE *docfs;
    int err;
    if (flag[2]) {
      docfs=stdout;
      err=0;
    } else {
      docfs=stderr;
      err=INSUFFICIENT_COMMAND_ARGS;
    }
    fprintf(docfs, "bev2bev2 infile [-1 index] [-B blocksize] outfile\n");
    fprintf(docfs, "options:\n");
    ctraj_optargs(docfs, "1B");
    return err;
  }

  fs=fopen(argv[1], "r");

  fread(&magic, sizeof(magic), 1, fs);
  assert(magic == MAGIC);
  fread(&n1, sizeof(n1), 1, fs);

  for (int32_t i=0; i<ind; i++) {
    fread(&yr, sizeof(yr), 1, fs);
    fread(&mon, sizeof(mon), 1, fs);
    fread(&dy, sizeof(dy), 1, fs);
    fread(&hr, sizeof(hr), 1, fs);
    fread(&min, sizeof(min), 1, fs);
    fread(&sec, sizeof(sec), 1, fs);
    fread(&npt1, sizeof(npt1), 1, fs);
    fseek(fs, 8*npt1, SEEK_CUR);
  }

  fread(&yr, sizeof(yr), 1, fs);
  fread(&mon, sizeof(mon), 1, fs);
  fread(&dy, sizeof(dy), 1, fs);
  fread(&hr, sizeof(hr), 1, fs);
  fread(&min, sizeof(min), 1, fs);
  fread(&sec, sizeof(sec), 1, fs);
  fread(&npt1, sizeof(npt1), 1, fs);

  t=new time_class(yr, mon, dy, hr, min, sec);

  lon=new float[npt1];
  lat=new float[npt1];

  fread(lon, sizeof(float), npt1, fs);
  fread(lat, sizeof(float), npt1, fs);

  fclose(fs);

  fs=fopen(argv[2], "w");

  bs2=bs;
  npt2=npt1;

  fwrite(t, sizeof(*t), 1, fs);
  fwrite(&bs2, sizeof(bs2), 1, fs);
  fwrite(&npt2, sizeof(npt2), 1, fs);
  fwrite(&zero, sizeof(zero), 1, fs);

  printf("%d %d %d\n", bs2, npt2, zero);

  for (long i=0; i<npt2; i++) {
    if (lat[i] < 0) hemi=-1; else hemi=1;
    tcoord2_2lonlat(x, y, -1, hemi, lon[i], lat[i]);
    fwrite(&x, sizeof(float), 1, fs);
    fwrite(&y, sizeof(float), 1, fs);
    fwrite(&hemi, sizeof(short), 1, fs);
    printf("%f %f %d\n", x, y, hemi);
  }
  //write dummy data to the end of the block:
  for (long i=npt2; i<bs2; i++) {
    fwrite(&x, sizeof(float), 1, fs);
    fwrite(&y, sizeof(float), 1, fs);
    fwrite(&hemi, sizeof(short), 1, fs);
  }

  fclose(fs);

  delete t;

  delete [] lon;
  delete [] lat;

}


