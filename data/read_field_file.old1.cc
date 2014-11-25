//#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "../datasets/simple_temp.h"
#include "../clib/time.h"
#include "../datasets/dependent_temp.h"
#include "read_field_file.h"

#define MAGIC_NO -49
#define NDIM_FIXED 4
#define MAX_N 100000
#define VERBOSE

typedef simple<float> simple_float;
typedef simple<time> simple_time;
typedef dependent<float> dependent_float;

//read a single variable of gridded data from a binary file
//as well as the dimension headers:
int read_field_file(	char *filename,
			char *fieldname,
			dependent_float *&fdata,
			simple_float *&lon,
			simple_float *&lat,
			simple_float *&lev,
			simple_time *&date) {

  //top most header data:
  long magic;			//"magic" number
  long ndim;			//number of dimensions (4)
  long nvar;			//number of variables
  long fieldsize;		//size of each field
  long offset;			//start of data section

  //dimension info:
  long dims[NDIM_FIXED];	//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  long dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  long * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  long * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  long varind;			//which variable to retrieve??
  long nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  long nfield, n;		//total number of data elements in field

  float * data;			//the data
  long byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time time_data;
  short int year;
  short int month;
  short int day;
  short int hour;
  short int minute;
  float second;

  fileptr=fopen(filename, "r");

  //in this section we read the header data:
  fread(&magic, 1, sizeof(magic), fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, 1, sizeof(ndim), fileptr);	//read # of dimensions
  fread(&nvar, 1, sizeof(ndim), fileptr);	//read # of variables
  fread(&fieldsize, 1, sizeof(ndim), fileptr);	//read in the field size
  fread(&offset, 1, sizeof(ndim), fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, 1, sizeof(long)*ndim, fileptr);
  fread(dim_type, 1, sizeof(char)*ndim, fileptr);
  fread(dimn_len, 1, sizeof(long)*ndim, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    long dim_len_i=dimn_len[i];
    dim_name[i]=new char[dim_len_i+1];
    fread(dim_name[i], 1, dim_len_i, fileptr);
    dim_name[i][dim_len_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following dimensions were found in the file %s\n", filename);
    for (long i=0; i<ndim; i++) printf("%s: %d\n", dim_name[i], dims[i]);
  #endif

  //allocate space for the types and name lengths:
  var_type=new char[nvar];
  varn_len=new long[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, 1, sizeof(char)*nvar, fileptr);
  fread(varn_len, 1, sizeof(long)*nvar, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    long vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';

    //while we're at it, we might as well search for the desired field:
    if (strcmp(var_name[i], fieldname)==0) varind=i;
  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  if (varind == -1) {
    printf("Field %s not found in file %s\n", fieldname, filename);
    fclose(fileptr);
    return 3;
  }

  //read in the lengths of the descriptors (which we do not use):
  desc_len=new long[nvar];
  fread(desc_len, 1, sizeof(long)*nvar, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, 1, sizeof(float)*nlon, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, 1, sizeof(float)*nlat, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, 1, sizeof(float)*nl, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time>;


  //start reading in the field:
  nfield=nlon*nlat*nl;
  n=nfield*nt;

  data=new float[n];
  for (long i=0; i<nt; i++) {
    fread(&year, 1, sizeof(year), fileptr);
    fread(&month, 1, sizeof(month), fileptr);
    fread(&day, 1, sizeof(day), fileptr);
    fread(&hour, 1, sizeof(hour), fileptr);
    fread(&minute, 1, sizeof(minute), fileptr);
    fread(&second, 1, sizeof(second), fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    date->add_el(time_data);

    //how do you move the file pointer in C??
    fseek(fileptr, varind*nfield*sizeof(float), SEEK_CUR);

    fread(data+i*nfield, 1, nfield*sizeof(float), fileptr);

    fseek(fileptr, (nvar-varind-1)*nfield*sizeof(float), SEEK_CUR);
//    fdata->preload(i*nfield, data);
  }

  //initialize the field dataset:
  fdata=new dependent<float>(lon, lat, lev, date);
  fdata->preload(data, n, 0);

  fclose(fileptr);
  return 0;

}

//read only the dimension headers from a binary file of gridded data:
int read_field_file_head(char *filename,
			char **&var_name,
			long nvar,
			simple_float *&lon,
			simple_float *&lat,
			simple_float *&lev,
			simple_time *&date) {

  //top most header data:
  long magic;			//"magic" number
  long ndim;			//number of dimensions (4)
  long fieldsize;		//size of each field
  long offset;			//start of data section

  //dimension info:
  long dims[NDIM_FIXED];	//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  long dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  long * varn_len;		//lengths of each of the variable names
  long * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  long varind;			//which variable to retrieve??
  long nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.

  float * data;			//the data
  long byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time time_data;
  short int year;
  short int month;
  short int day;
  short int hour;
  short int minute;
  float second;

  fileptr=fopen(filename, "r");

  //in this section we read the header data:
  fread(&magic, 1, sizeof(magic), fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, 1, sizeof(ndim), fileptr);	//read # of dimensions
  fread(&nvar, 1, sizeof(ndim), fileptr);	//read # of variables
  fread(&fieldsize, 1, sizeof(ndim), fileptr);	//read in the field size
  fread(&offset, 1, sizeof(ndim), fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, 1, sizeof(long)*ndim, fileptr);
  fread(dim_type, 1, sizeof(char)*ndim, fileptr);
  fread(dimn_len, 1, sizeof(long)*ndim, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    long dim_len_i=dimn_len[i];
    dim_name[i]=new char[dim_len_i+1];
    fread(dim_name[i], 1, dim_len_i, fileptr);
    dim_name[i][dim_len_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following dimensions were found in the file %s\n", filename);
    for (long i=0; i<ndim; i++) printf("%s: %d\n", dim_name[i], dims[i]);
  #endif

  //allocate space for the types and name lengths:
  var_type=new char[nvar];
  varn_len=new long[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, 1, sizeof(char)*nvar, fileptr);
  fread(varn_len, 1, sizeof(long)*nvar, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    long vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  //read in the lengths of the descriptors (which we do not use):
  desc_len=new long[nvar];
  fread(desc_len, 1, sizeof(long)*nvar, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, 1, sizeof(float)*nlon, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, 1, sizeof(float)*nlat, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, 1, sizeof(float)*nl, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time>;

  for (long i=0; i<nt; i++) {
    fread(&year, 1, sizeof(year), fileptr);
    fread(&month, 1, sizeof(month), fileptr);
    fread(&day, 1, sizeof(day), fileptr);
    fread(&hour, 1, sizeof(hour), fileptr);
    fread(&minute, 1, sizeof(minute), fileptr);
    fread(&second, 1, sizeof(second), fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    date->add_el(time_data);

    //how do you move the file pointer in C??
    fseek(fileptr, fieldsize*nvar, SEEK_CUR);
  }

  fclose(fileptr);
  return 0;

}

//read in only the data from a single variable out of a gridded binary data file:
int read_field_file_data(	char *filename,
			char *fieldname,
			dependent_float *fdata
			) {

  //top most header data:
  long magic;			//"magic" number
  long ndim;			//number of dimensions (4)
  long nvar;			//number of variables
  long fieldsize;		//size of each field
  long offset;			//start of data section

  //dimension info:
  long dims[NDIM_FIXED];	//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  long dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  long * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  long * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  long varind;			//which variable to retrieve??
  long nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  long nfield;			//total number of data elements in field

  long byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  short int year;
  short int month;
  short int day;
  short int hour;
  short int minute;
  float second;

  fileptr=fopen(filename, "r");

  //in this section we read the header data:
  fread(&magic, 1, sizeof(magic), fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, 1, sizeof(ndim), fileptr);	//read # of dimensions
  fread(&nvar, 1, sizeof(ndim), fileptr);	//read # of variables
  fread(&fieldsize, 1, sizeof(ndim), fileptr);	//read in the field size
  fread(&offset, 1, sizeof(ndim), fileptr);	//read in offset


  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, 1, sizeof(long)*ndim, fileptr);
  fread(dim_type, 1, sizeof(char)*ndim, fileptr);
  fread(dimn_len, 1, sizeof(long)*ndim, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    long dim_len_i=dimn_len[i];
    dim_name[i]=new char[dim_len_i+1];
    fread(dim_name[i], 1, dim_len_i, fileptr);
    dim_name[i][dim_len_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following dimensions were found in the file %s\n", filename);
    for (long i=0; i<ndim; i++) printf("%s: %d\n", dim_name[i], dims[i]);
  #endif

  //allocate space for the types and name lengths:
  var_type=new char[nvar];
  varn_len=new long[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, 1, sizeof(char)*nvar, fileptr);
  fread(varn_len, 1, sizeof(long)*nvar, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    long vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';

    //while we're at it, we might as well search for the desired field:
    if (strcmp(var_name[i], fieldname)==0) varind=i;
  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  if (varind == -1) {
    printf("Field %s not found in file %s\n", fieldname, filename);
    fclose(fileptr);
    return 3;
  }

  //seek to the beginning of the variable data section:
  fseek(fileptr, offset, SEEK_SET);

  //start reading in the field:
  nfield=nlon*nlat*nl;

  for (long i=0; i<nt; i++) {
    fread(&year, 1, sizeof(year), fileptr);
    fread(&month, 1, sizeof(month), fileptr);
    fread(&day, 1, sizeof(day), fileptr);
    fread(&hour, 1, sizeof(hour), fileptr);
    fread(&minute, 1, sizeof(minute), fileptr);
    fread(&second, 1, sizeof(second), fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);

    //how do you move the file pointer in C??
    fseek(fileptr, varind*nfield*sizeof(float), SEEK_CUR);

    fdata->read_chunk(fileptr, i*nfield, nfield);

    fseek(fileptr, (nvar-varind-1)*nfield*sizeof(float), SEEK_CUR);
//    fdata->preload(i*nfield, data);
  }

  fclose(fileptr);
  return 0;

}

//read in all the fields and all the dimensions of a gridded binary data
//file:
int read_field_file_all(	char *filename,
			dependent_float **&fdata,
			long &nvar,			//number of variables
			simple_float *&lon,
			simple_float *&lat,
			simple_float *&lev,
			simple_time *&date) {

  //top most header data:
  long magic;			//"magic" number
  long ndim;			//number of dimensions (4)
  long fieldsize;		//size of each field
  long offset;			//start of data section

  //dimension info:
  long dims[NDIM_FIXED];	//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  long dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  long * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  long * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  long nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  long nfield, n;		//total number of data elements in field

  float * data;			//the data for simple datasets
  float ** ddata;		//the data for dependent datasets
  long byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time time_data;
  short int year;
  short int month;
  short int day;
  short int hour;
  short int minute;
  float second;

  fileptr=fopen(filename, "r");

  //in this section we read the header data:
  fread(&magic, 1, sizeof(magic), fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, 1, sizeof(ndim), fileptr);	//read # of dimensions
  fread(&nvar, 1, sizeof(ndim), fileptr);	//read # of variables
  fread(&fieldsize, 1, sizeof(ndim), fileptr);	//read in the field size
  fread(&offset, 1, sizeof(ndim), fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, 1, sizeof(long)*ndim, fileptr);
  fread(dim_type, 1, sizeof(char)*ndim, fileptr);
  fread(dimn_len, 1, sizeof(long)*ndim, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    long dim_len_i=dimn_len[i];
    dim_name[i]=new char[dim_len_i+1];
    fread(dim_name[i], 1, dim_len_i, fileptr);
    dim_name[i][dim_len_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following dimensions were found in the file %s\n", filename);
    for (long i=0; i<ndim; i++) printf("%s: %d\n", dim_name[i], dims[i]);
  #endif

  //allocate space for the types and name lengths:
  var_type=new char[nvar];
  varn_len=new long[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, 1, sizeof(char)*nvar, fileptr);
  fread(varn_len, 1, sizeof(long)*nvar, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  for (long i=0; i<nvar; i++) {
    long vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';

  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  //read in the lengths of the descriptors (which we do not use):
  desc_len=new long[nvar];
  fread(desc_len, 1, sizeof(long)*nvar, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, 1, sizeof(float)*nlon, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, 1, sizeof(float)*nlat, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, 1, sizeof(float)*nl, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time>;


  //start reading in the field:
  nfield=nlon*nlat*nl;
  n=nfield*nt;

  ddata=new float * [nvar];
  for (long i=0; i<nvar; i++) ddata[i]=new float[n];

  for (long i=0; i<nt; i++) {
    fread(&year, 1, sizeof(year), fileptr);
    fread(&month, 1, sizeof(month), fileptr);
    fread(&day, 1, sizeof(day), fileptr);
    fread(&hour, 1, sizeof(hour), fileptr);
    fread(&minute, 1, sizeof(minute), fileptr);
    fread(&second, 1, sizeof(second), fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    date->add_el(time_data);

    for (long j=0; j<nvar; j++) {
      fread(ddata[j]+i*nfield, 1, nfield*sizeof(float), fileptr);
    }

  }

  //initialize the field dataset:
  fdata=new dependent<float> * [nvar];
  for (long i=0; i<nvar; i++) {
    fdata[i]=new dependent<float>(lon, lat, lev, date);
    fdata[i]->preload(ddata[i], n, 0);
  }

  fclose(fileptr);
  return 0;

}


