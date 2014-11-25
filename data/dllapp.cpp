// PROJECT1.C.
// The myPuts function writes a null-terminated string to  
// the standard output device.

#include <windows.h> 

void myPuts(LPTSTR lpszMsg)
{ 
   DWORD cchWritten; 
   HANDLE hStdout; 
 
   // Get a handle to the standard output device.
 
   hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
 
   // Write a null-terminated string to the standard output device.
 
   while (*lpszMsg) 
      WriteFile(hStdout, lpszMsg++, 1, &cchWritten, NULL); 
} 

