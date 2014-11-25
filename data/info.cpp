#include "info.h"

/*! \file
    \ingroup GLT
    \todo i2string

    $Id: info.cpp,v 1.17 2003/08/25 06:16:04 nigels Exp $

    $Log: info.cpp,v $
    Revision 1.17  2003/08/25 06:16:04  nigels
    *** empty log message ***

    Revision 1.15  2003/08/05 08:04:02  nigels
    *** empty log message ***

    Revision 1.13  2003/05/10 16:58:11  nigels
    Last tweaks for 0.8

    Revision 1.11  2003/03/06 11:48:30  nigels
    *** empty log message ***

    Revision 1.10  2003/03/06 08:32:11  nigels
    Experimenting with memory use information

    Revision 1.9  2002/11/27 00:57:28  nigels
    expand

    Revision 1.8  2002/11/07 15:40:45  nigels
    *** empty log message ***

    Revision 1.7  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <glt/gl.h>
#include <glt/config.h>
#include <glt/error.h>
#include <misc/string.h>
#include <misc/config.h>

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstring>
using namespace std;

#ifdef GLT_WIN32
#include <winbase.h>
#include <lmcons.h>
#endif

#ifdef GLT_UNIX
#include <cstdlib>
#include <sys/utsname.h>
#endif

/// Utility function for converting integers to strings
static string i2string(const int i)
{
    char buffer[100];
    sprintf(buffer,"%d",i);
    return buffer;
}

string
GltInfo()
{
    string str;

    GLERROR;

    str += "          Vendor : ";
    str += (char *) glGetString(GL_VENDOR);
    str += '\n';

    str += "        Renderer : ";
    str += (char *) glGetString(GL_RENDERER);
    str += '\n';

    str += "         Version : ";
    str += (char *) glGetString(GL_VERSION);
    str += '\n';

    GLint doubleBuffering;
    GLint rgbaMode;
    GLint zDepth;
    GLint stencilDepth;

    glGetIntegerv(GL_DOUBLEBUFFER,&doubleBuffering);
    glGetIntegerv(GL_RGBA_MODE,&rgbaMode);
    glGetIntegerv(GL_DEPTH_BITS,&zDepth);
    glGetIntegerv(GL_STENCIL_BITS,&stencilDepth);

    GLERROR

    str += "Double-buffering : ";
    str += (doubleBuffering ? "" : "not " );
    str += "supported.\n";

    str += "       RGBA mode : ";
    str += (rgbaMode ? "" : "not " );
    str += "enabled.\n";

    str += "    Depth buffer : ";
    str += i2string(zDepth);
    str += " bit-planes.\n";

    str += "  Stencil buffer : ";
    str += i2string(stencilDepth);
    str += " bit-planes.";

    return str;
}

string
GltDate()
{
    time_t clock;
    time(&clock);
    return asctime(localtime(&clock));
}

string
GltSystemInfo()
{
    string tmp;

    #ifdef GLT_WIN32

    // Computer Name

    char  cName       [MAX_COMPUTERNAME_LENGTH+1];
    DWORD cNameSize =  MAX_COMPUTERNAME_LENGTH+1;

    cName[0] = '\0';
    ::GetComputerName(cName,&cNameSize);

    // User Name

    char  uName       [UNLEN + 11];
    DWORD uNameSize =  UNLEN + 1;

    uName[0] = '\0';
    ::GetUserName(uName,&uNameSize);

    // System Name and Version

    OSVERSIONINFOEX osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osInfo));

    // Processors

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    string cpuFeatures;
    if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))    cpuFeatures += " MMX";
    if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))   cpuFeatures += " SSE";

    #if MSC_VER>6
    if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) cpuFeatures += " SSE2";
    if (IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE))  cpuFeatures += " 3DNow";
    #endif

    // Memory

    MEMORYSTATUS memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&memInfo);

    sprintf
    (
        tmp,
        "        Computer : %s\n"
        "            User : %s\n"
        "          System : %s %d.%d Build %d (%s)\n"
        "             CPU : %s%s\n"
        "          Memory : %d MB Physical, %d MB Swap",
         cName,
         uName,
        (osInfo.dwPlatformId==VER_PLATFORM_WIN32_NT ? "Windows NT" : "Windows 95/98"),
         osInfo.dwMajorVersion,
         osInfo.dwMinorVersion,
         osInfo.dwBuildNumber,
         osInfo.szCSDVersion,
        (sysInfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL ? "Intel" : "Unknown"),
         cpuFeatures.c_str(),
         (memInfo.dwTotalPhys     + ((1<<20) - 1))>>20,
         (memInfo.dwTotalPageFile + ((1<<20) - 1))>>20
    );

    return tmp;
    #endif

    #ifdef GLT_UNIX
    struct utsname info;

    if (!uname(&info))
    {
        sprintf
        (
            tmp,
            "        Computer : %s\n"
            "            User : %s\n"
            "          System : %s\n"
            "             CPU : %s",
             info.nodename,
             getenv("USER"),
             info.sysname,
             info.machine
        );
    }

    return tmp;
    #endif

    return tmp;
}

string GltVersion()
{
    return GLT_VERSION_STRING;
}

#ifdef GLT_ZLIB
#include <zlib.h>
#endif

#ifdef GLT_PNG
#include <png.h>
#endif

#ifdef GLT_JPEG

#ifdef FAR
#undef FAR
#endif

namespace jpeg
{
extern "C"
{
#include <jpeg/jpeglib.h>
}
}
#endif

string GltVersionInformation()
{
    string str;

    str += "GLT    " + GltVersion() + "\n";

    #ifdef GLT_ZLIB
    str += "zLib   " + string(ZLIB_VERSION) + "\n";
    #endif

    #ifdef GLT_PNG
    str += "libPNG " + string(PNG_LIBPNG_VER_STRING) + "\n";
    #endif

    #ifdef GLT_JPEG
    string jpeg;
    sprintf(jpeg,"libJPG %d%c\n",JPEG_LIB_VERSION/10,(JPEG_LIB_VERSION%10)+'a'-1);
    str += jpeg;
    #endif

    return str;
}

/*

#ifdef GLT_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

string GltMemoryInformation()
{
    string str;

    #ifdef GLT_UNIX

    // TODO: This doesn't actually work due to limitations
    //       in the Linux kernal implementation...

    struct rusage usage;
    if (!getrusage(RUSAGE_SELF,&usage))
    {
        sprintf(str,"Memory: %ld",usage.ru_maxrss);
    }
    #endif

    return str;
}
*/
