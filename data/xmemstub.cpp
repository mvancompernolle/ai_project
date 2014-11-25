/*****************************************************************************/
/*
    XMEMSTUB.CPP - Extended C/C++ Dynamic Memory Control And Debug Library

    Copyright (C) Juergen Mueller (J.M.) 1987-2010
    All rights reserved.

    You are expressly prohibited from selling this software in any form
    or removing this notice.

    THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE
    IMPLIED WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
    PURPOSE, OR NON-INFRINGEMENT. THE AUTHOR SHALL NOT BE LIABLE FOR
    ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
    OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES. THE ENTIRE RISK
    AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM AND DOCUMENTATION
    IS WITH YOU.

    Permission to modify the code and to distribute modified code is granted,
    provided the above notices are retained, and a notice that the code was
    modified is included with the above copyright notice.

    written by: Juergen Mueller, D-70806 Kornwestheim, GERMANY

    FILE       : XMEMSTUB.CPP
    REVISION   : 31-May-2010
                 11:20:57
 */
/*****************************************************************************/

#ifdef __cplusplus

#ifndef __XMEM_H_INCLUDED__
#include "xmem.h"
#ifdef new
 #undef new
#endif
#ifdef delete
 #undef delete
#endif
#endif              /* __XMEM_H_INCLUDED__ */

#if XMEM

/*****************************************************************************/
/* C++ operator new */
/*****************************************************************************/
#if XMEM_CRITICAL_OPERATOR
void *operator new(size_t size)
{ /* call C++ helper function */
#if XMEM_GET_LOCATION
  unsigned long  line = 0;
  char          *func = "operator new";
  char          *fname = "[xmem_lib/detours]";

  if (xmem_get_memtrace())
  {
    xmem_get_call_location(1, &func, &fname, &line);
  }

  return(xmem_operator_new_loc(size, func, fname, line));
#else
  return(xmem_operator_new(size));
#endif
}
#endif
/*****************************************************************************/
/* C++ operator new */
/*****************************************************************************/
void *operator new(size_t size, const char *callname, const char *fname, unsigned long line)
{ /* call C++ helper function */
  return(xmem_operator_new_loc(size, callname, fname, line));
}
/*****************************************************************************/
/* C++ operator new[] */
/*****************************************************************************/
#if XMEM_CRITICAL_OPERATOR
#if XMEM_NEWARRAY
void *operator new[] (size_t size)
{ /* call C++ helper function */
#if XMEM_GET_LOCATION
  unsigned long  line = 0;
  char          *func = "operator new[]";
  char          *fname = "[xmem_lib/detours]";

  if (xmem_get_memtrace())
  {
    xmem_get_call_location(1, &func, &fname, &line);
  }

  return(xmem_operator_new_array_loc(size, func, fname, line));
#else
  return(xmem_operator_new_array(size));
#endif
}
#endif
#endif
/*****************************************************************************/
/* C++ operator new[] */
/*****************************************************************************/
#if XMEM_NEWARRAY
void *operator new[] (size_t size, const char *callname, const char *fname, unsigned long line)
{ /* call C++ helper function */
  return(xmem_operator_new_array_loc(size, callname, fname, line));
}
#endif
/*****************************************************************************/
/* C++ operator delete */
/*****************************************************************************/
void operator delete(void *ptr)
{ /* call C++ helper function */
#if XMEM_GET_LOCATION
  unsigned long  line = 0;
  char          *func = "operator delete";
  char          *fname = "[xmem_lib/detours]";

  if (xmem_get_memtrace())
  {
    xmem_get_call_location(1, &func, &fname, &line);
  }

  return(xmem_operator_delete_loc(ptr, func, fname, line));
#else
  xmem_operator_delete(ptr);
#endif
}
/*****************************************************************************/
/* C++ operator delete */
/*****************************************************************************/
#if !(_MSC_VER == 800)
void operator delete(void *ptr, const char *callname, const char *fname, unsigned long line)
{ /* call C++ helper function */
  xmem_operator_delete_loc(ptr, callname, fname, line);
}
#endif
/*****************************************************************************/
/* C++ operator delete[] */
/*****************************************************************************/
#if XMEM_DELETEARRAY
void operator delete[] (void *ptr)
{ /* call C++ helper function */
#if XMEM_GET_LOCATION
  unsigned long  line = 0;
  char          *func = "operator delete[]";
  char          *fname = "[xmem_lib/detours]";

  if (xmem_get_memtrace())
  {
    xmem_get_call_location(1, &func, &fname, &line);
  }

  return(xmem_operator_delete_array_loc(ptr, func, fname, line));
#else
  xmem_operator_delete_array(ptr);
#endif
}
#endif
/*****************************************************************************/
/* C++ operator delete[] */
/*****************************************************************************/
#if XMEM_DELETEARRAY
void operator delete[] (void *ptr, const char *callname, const char *fname, unsigned long line)
{ /* call C++ helper function */
  xmem_operator_delete_array_loc(ptr, callname, fname, line);
}
#endif
/*****************************************************************************/

#endif              /* XMEM */

#endif              /* __cplusplus */
