#include <stdlib.h>
#include <stdarg.h>

#include "errlog.h"

static int preset_cast_error_level=0;

static FILE* preset_cast_error_stream=stderr;

void  __cast_printf(const char* file, int lineno, const char*func, 
			  int err_level, const char *str, ...)
{
  va_list arg;
  
  if (err_level > preset_cast_error_level)
    {
      switch(err_level)
	{
	case 0: 
	  fprintf(preset_cast_error_stream, "%s : Line %d : Func: %s :", file, lineno, func);
	  break;
	case 1:
	  fprintf(preset_cast_error_stream, "WARNING ! ");
	  fprintf(preset_cast_error_stream, "%s : Line %d : Func: %s :", file, lineno, func);
	  break;
	case 2:
	  fprintf(preset_cast_error_stream, "ERROR ! ");
	  fprintf(preset_cast_error_stream, "%s : Line %d : Func: %s :", file, lineno, func);
	  break;
	case 1000:
	  fprintf(preset_cast_error_stream, "CRITICAL ERROR ! ");
	  fprintf(preset_cast_error_stream, "%s : Line %d : Func: %s :", file, lineno, func);
	  break;
	}
      va_start(arg, str);
      vfprintf(preset_cast_error_stream, str, arg);
      va_end(arg);
      fflush(NULL);
    };

  
  if (err_level>preset_cast_error_level && err_level>=2)
    abort();
 
}

void cast_set_err_level(int lvl)
{
  if (lvl>=999)
    preset_cast_error_level=999; //max of the preset_cast_error_level
  else if (lvl < 0)
    preset_cast_error_level=-1;
  else
    preset_cast_error_level=lvl;
}

void cast_set_err_stream(FILE* stream)
{
  if (stream!=NULL)
    preset_cast_error_stream=stream;
  else
    stream=stderr;
}

void cast_debug(int level, const char *str, ...)
{
  va_list arg;
  
  if (level>preset_cast_error_level)
    {
      va_start(arg, str);
      vfprintf(preset_cast_error_stream, str, arg);
      va_end(arg);
    };
 
  return;
}
