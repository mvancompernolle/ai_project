//
// Copywrite Peter Mills 2004.  All rights reserved.
//
// A set of subroutines used to read customized binary files in the same
// format as those accessed by the IDL routines in "file_lib.pro"
// 

//#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "read_field_file.h"

#define MAGIC_NO -49
#define NDIM_FIXED 4
#define MAX_N 100000
#define VERBOSE

typedef simple<float> simple_float;
typedef simple<time_class> simple_time;
typedef dependent<float> dependent_float;

//read a single variable of gridded data from a binary file
//as well as the dimension headers:
int read_field_file(	char *filename,
			char *fieldname,
			dependent_float *&fdata,
			simple_float *&lon,
			simple_float *&lat,
			simple_float *&lev,
			simple_time *&date) 
//
// name:	read_field_file
//
// usage:	err=read_field_file(filename, fieldname, 
// 			fielddata, xgrid, ygrid, zgrid, timegrid);
//
// purpose:	Reads customized binary files for holding gridded data of
// 		a format compatible with the routines contained in
// 		"time_lib.pro"  Returns the data in a set of special
// 		classes called "datasets."
//
// parameters:
// 	filename: (in)	The name of the file to read from.
//
// 	fieldname: (in) The name of the field to retrieve.
//
// 	fielddata: (out) The gridded field data.  Should be passed a null
// 		or uninitialized pointer to a variable of type
// 		dependent<float>.  Depends upon the four "simple" datasets,
// 		below.	See the file dependent_temp.cpp.
//
// 	xgrid: (out) The horizontal gridding in the x-direction.  
// 		Should be passed a null	or unitialized pointer to a variable 
// 		of type simple<float>.
//
// 	ygrid: (out) The horizontal gridding in the y-direction.  Same type
// 		as xgrid.
//
// 	zgrid: (out) The vertical gridding.  Same type as ygrid.
//
// 	timegrid (out) The time gridding.  Should be a null or unitialized
// 		pointer of type simple<time>.
//
// 	history:
// 		2004-2-19 PM: formally documented
//
{

  //top most header data:
  num magic;			//"magic" number
  num ndim;			//number of dimensions (4)
  num nvar;			//number of variables
  num fieldsize;		//size of each field
  num offset;			//start of data section

  //dimension info:
  num dims[NDIM_FIXED];		//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  num dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  num * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  num * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  num varind;			//which variable to retrieve??
  num nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  num nfield, n;		//total number of data elements in field

  float * data;			//the data
  num byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time_class time_data;
  short int year;
  short int month;
  short int day;
  short int hour;
  short int minute;
  float second;

  fileptr=fopen(filename, "r");

  //in this section we read the header data:
  fread(&magic, sizeof(magic), 1, fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, sizeof(ndim), 1, fileptr);	//read # of dimensions
  fread(&nvar, sizeof(ndim), 1, fileptr);	//read # of variables
  fread(&fieldsize, sizeof(ndim), 1, fileptr);	//read in the field size
  fread(&offset, sizeof(ndim), 1, fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, sizeof(long)*ndim, 1, fileptr);
  fread(dim_type, sizeof(char)*ndim, 1, fileptr);
  fread(dimn_len, sizeof(long)*ndim, 1, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    num dim_len_i=dimn_len[i];
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
  varn_len=new num[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, sizeof(char)*nvar, 1, fileptr);
  fread(varn_len, sizeof(num)*nvar, 1, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    num vnlen_i=varn_len[i];
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
  desc_len=new num[nvar];
  fread(desc_len, sizeof(long)*nvar, 1, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, sizeof(float)*nlon, 1, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, sizeof(float)*nlat, 1, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, sizeof(float)*nl, 1, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time_class>;


  //start reading in the field:
  nfield=nlon*nlat*nl;
  n=nfield*nt;

  data=new float[n];
  for (long i=0; i<nt; i++) {
    fread(&year, sizeof(year), 1, fileptr);
    fread(&month, sizeof(month), 1, fileptr);
    fread(&day, sizeof(day), 1, fileptr);
    fread(&hour, sizeof(hour), 1, fileptr);
    fread(&minute, sizeof(minute), 1, fileptr);
    fread(&second, sizeof(second), 1, fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    date->add_el(time_data);

//    printf("%d\n", ftell(fileptr));

    //how do you move the file pointer in C??
    fseek(fileptr, varind*nfield*sizeof(float), SEEK_CUR);

    fread(data+i*nfield, nfield*sizeof(float), 1, fileptr);

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
//use is essentially equivalent to read_field_file, above
int read_field_file_head(char *filename,	//the name of the file
			char **&var_name,	//returns all the variable names
			num &nvar,		//returns the number of variables
			simple_float *&lon,	//x-gridding
			simple_float *&lat,	//y-gridding
			simple_float *&lev,	//vertical gridding
			simple_time *&date) {	//time gridding

  //top most header data:
  num magic;			//"magic" number
  num ndim;			//number of dimensions (4)
  num fieldsize;		//size of each field
  num offset;			//start of data section

  //dimension info:
  num dims[NDIM_FIXED];		//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  num dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  num * varn_len;		//lengths of each of the variable names
  num * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  num varind;			//which variable to retrieve??
  num nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.

  float * data;			//the data
  num byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time_class time_data;
  char date_str[30];
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

  fread(&ndim, sizeof(ndim), 1, fileptr);	//read # of dimensions
  fread(&nvar, sizeof(ndim), 1, fileptr);	//read # of variables
  fread(&fieldsize, sizeof(ndim), 1, fileptr);	//read in the field size
  fread(&offset, sizeof(ndim), 1, fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, sizeof(num)*ndim, 1, fileptr);
  fread(dim_type, sizeof(char)*ndim, 1, fileptr);
  fread(dimn_len, sizeof(num)*ndim, 1, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    num dim_len_i=dimn_len[i];
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
  varn_len=new num[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, sizeof(char)*nvar, 1, fileptr);
  fread(varn_len, sizeof(num)*nvar, 1, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    num vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';
  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  //read in the lengths of the descriptors (which we do not use):
  desc_len=new num[nvar];
  fread(desc_len, sizeof(num)*nvar, 1, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, sizeof(float)*nlon, 1, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, sizeof(float)*nlat, 1, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, sizeof(float)*nl, 1, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time_class>;

  for (long i=0; i<nt; i++) {
    fread(&year, sizeof(year), 1, fileptr);
    fread(&month, sizeof(month), 1, fileptr);
    fread(&day, sizeof(day), 1, fileptr);
    fread(&hour, sizeof(hour), 1, fileptr);
    fread(&minute, sizeof(minute), 1, fileptr);
    fread(&second, sizeof(second), 1, fileptr);
//    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    time_data.write_string(date_str);
    printf("%s\n", date_str);
    date->add_el(time_data);

    //how do you move the file pointer in C??
    fseek(fileptr, fieldsize*nvar, SEEK_CUR);
  }

  fclose(fileptr);
  return 0;

}

//read in only the data from a single variable out of a gridded binary data file:
//use is similar to read_field_file, but fdata must be pre-initialized,
//i.e.: fdata=new dependent<float>(lon, lat, lev, time);
int read_field_file_data(	char *filename,
			char *fieldname,
			dependent_float *fdata
			) {

  //top most header data:
  num magic;			//"magic" number
  num ndim;			//number of dimensions (4)
  num nvar;			//number of variables
  num fieldsize;		//size of each field
  num offset;			//start of data section

  //dimension info:
  num dims[NDIM_FIXED];		//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  num dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  num * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  num * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  num varind;			//which variable to retrieve??
  num nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  num nfield;			//total number of data elements in field

  num byte_counter;

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
  fread(&magic, sizeof(magic), 1, fileptr);

  if (magic != MAGIC_NO) {
    printf("File %s of wrong type\n", filename);
    fclose(fileptr);
    return 1;
  }

  fread(&ndim, sizeof(ndim), 1, fileptr);	//read # of dimensions
  fread(&nvar, sizeof(ndim), 1, fileptr);	//read # of variables
  fread(&fieldsize, sizeof(ndim), 1, fileptr);	//read in the field size
  fread(&offset, sizeof(ndim), 1, fileptr);	//read in offset


  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, sizeof(num)*ndim, 1, fileptr);
  fread(dim_type, sizeof(char)*ndim, 1, fileptr);
  fread(dimn_len, sizeof(num)*ndim, 1, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    num dim_len_i=dimn_len[i];
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
  varn_len=new num[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, sizeof(char)*nvar, 1, fileptr);
  fread(varn_len, sizeof(num)*nvar, 1, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  varind=-1;
  for (long i=0; i<nvar; i++) {
    num vnlen_i=varn_len[i];
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
  nt=dims[3];
  nfield=dims[0]*dims[1]*dims[2];

  for (long i=0; i<nt; i++) {
    fread(&year, sizeof(year), 1, fileptr);
    fread(&month, sizeof(month), 1, fileptr);
    fread(&day, sizeof(day), 1, fileptr);
    fread(&hour, sizeof(hour), 1, fileptr);
    fread(&minute, sizeof(minute), 1, fileptr);
    fread(&second, sizeof(second), 1, fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
//    printf("%d\n", i);
//
//    fseek(fileptr, 14, SEEK_CUR);

//    printf("%d\n", ftell(fileptr));

    //how do you move the file pointer in C??
    fseek(fileptr, varind*nfield*sizeof(float), SEEK_CUR);

    //printf("Data offset: %d\n", i*nfield);
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
			dependent_float **&fdata,	//returns the data as an array
							//of dependent<float> datasets
			num &nvar,			//number of variables
			simple_float *&lon,		//x gridding
			simple_float *&lat,		//y gridding
			simple_float *&lev,		//vertical gridding
			simple_time *&date) {		//time gridding

  //top most header data:
  num magic;			//"magic" number
  num ndim;			//number of dimensions (4)
  num fieldsize;		//size of each field
  num offset;			//start of data section

  //dimension info:
  num dims[NDIM_FIXED];		//the size of each dimension
  char dim_type[NDIM_FIXED];	//type of each dimension
  num dimn_len[NDIM_FIXED];	//lengths of each of the dimension names
  char *dim_name[NDIM_FIXED];	//names of the dimensions

  //variable info:
  char * var_type;		//type of each variable
  num * varn_len;		//lengths of each of the variable names
  char **var_name;		//names of the variables
  num * desc_len;		//lengths of the descriptors (currently not used)

  //indices and sizes:
  num nlon, nlat, nl, nt;	//number of grids for lon, lat, lev, and time resp.
  num nfield, n;		//total number of data elements in field

  float * data;			//the data for simple datasets
  float ** ddata;		//the data for dependent datasets
  num byte_counter;

  //very important, our file pointer!:
  FILE *fileptr;

  //dates and times:
  time_class time_data;
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

  fread(&ndim, sizeof(ndim), 1, fileptr);	//read # of dimensions
  fread(&nvar, sizeof(ndim), 1, fileptr);	//read # of variables
  fread(&fieldsize, sizeof(ndim), 1, fileptr);	//read in the field size
  fread(&offset, sizeof(ndim), 1, fileptr);	//read in offset

  if (ndim != NDIM_FIXED) {
    printf("Wrong number of dimensions in file %s\n", filename);
    fclose(fileptr);
    return 2;
  }

  fread(dims, sizeof(num)*ndim, 1, fileptr);
  fread(dim_type, sizeof(char)*ndim, 1, fileptr);
  fread(dimn_len, sizeof(num)*ndim, 1, fileptr);

  //allocate space for the dimension names and read them in:
  for (long i=0; i<ndim; i++) {
    num dim_len_i=dimn_len[i];
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
  varn_len=new num[nvar];

  //read in the variable types (which we don't use):
  fread(var_type, sizeof(char)*nvar, 1, fileptr);
  fread(varn_len, sizeof(num)*nvar, 1, fileptr);

  //allocate space for the variable names and read them in:
  var_name=new char *[nvar];
  for (long i=0; i<nvar; i++) {
    num vnlen_i=varn_len[i];
    var_name[i]=new char[vnlen_i+1];
    fread(var_name[i], 1, vnlen_i, fileptr);
    var_name[i][vnlen_i]='\0';

  }

  #ifdef VERBOSE
    printf("The following variables were found in the file %s\n", filename);
    for (long i=0; i<nvar; i++) printf("%s\n", var_name[i]);
  #endif

  //read in the lengths of the descriptors (which we do not use):
  desc_len=new num[nvar];
  fread(desc_len,  sizeof(num)*nvar, 1, fileptr);

  //end of the top header section
  //now to the dimension variables:

  //read in the longitude grids:
  nlon=dims[0];
  data=new float[nlon];

  fread(data, sizeof(float)*nlon, 1, fileptr);

  //initialize the longitude dataset:
  lon=new simple<float>(data, nlon, 1);

  //read in the latitude grids:
  nlat=dims[1];
  data=new float[nlat];

  fread(data, sizeof(float)*nlat, 1, fileptr);

  //initialize the latitude dataset:
  lat=new simple<float>(data, nlat, 0);

  //read in the level grids:
  nl=dims[2];
  data=new float[nl];

  fread(data, sizeof(float)*nl, 1, fileptr);

  //initialize the latitude dataset:
  lev=new simple<float>(data, nl, 1);

  //now to start reading in the main data:
  nt=dims[3];

  //initialize the time dataset:
  date=new simple<time_class>;

  //start reading in the field:
  nfield=nlon*nlat*nl;
  n=nfield*nt;

  ddata=new float * [nvar];
  for (long i=0; i<nvar; i++) ddata[i]=new float[n];

  for (long i=0; i<nt; i++) {
    fread(&year, sizeof(year), 1, fileptr);
    fread(&month, sizeof(month), 1, fileptr);
    fread(&day, sizeof(day), 1, fileptr);
    fread(&hour, sizeof(hour), 1, fileptr);
    fread(&minute, sizeof(minute), 1, fileptr);
    fread(&second, sizeof(second), 1, fileptr);
    printf("%i.%i.%i %i:%i:%f\n", year, month, day, hour, minute, second);
    time_data.init(year, month, day, hour, minute, second);
    date->add_el(time_data);

    for (long j=0; j<nvar; j++) {
      fread(ddata[j]+i*nfield, nfield*sizeof(float), 1, fileptr);
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


