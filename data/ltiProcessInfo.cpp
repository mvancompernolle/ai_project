/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib), except
 * that code from the psapi.lib.
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiProcessInfo.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.00
 * revisions ..: $Id: ltiProcessInfo.cpp,v 1.7 2010/04/10 01:36:03 alvarado Exp $
 */

#define _BSD_SOURCE 1
#include "ltiProcessInfo.h"
#include "ltiConfig.h"

#ifdef _LTI_WIN32
#  include <windows.h>
#  include <process.h>
#  include "ltiException.h"
#  include <Winbase.h>
#  include "ltiTimer.h"
#  include "ltiMath.h"
#  include <Pdh.h>

#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h

// this code is from the Win32 SDK, psapi.lib

/* BUILD Version: 0001    // Increment this if a change has global effects
  Copyright (c) 1994-1997  Microsoft Corporation
  Module Name:      psapi.h
  Abstract:      Include file for APIs provided by PSAPI.DLL
  Author:      Richard Shupak   [richards]  06-Jan-1994
  Revision History: */

extern "C" {
  typedef struct PROCESS_MEMORY_COUNTERS {
  DWORD cb;
  DWORD PageFaultCount;
  DWORD PeakWorkingSetSize;
  DWORD WorkingSetSize;
  DWORD QuotaPeakPagedPoolUsage;
  DWORD QuotaPagedPoolUsage;
  DWORD QuotaPeakNonPagedPoolUsage;
  DWORD QuotaNonPagedPoolUsage;
  DWORD PagefileUsage;
  DWORD PeakPagefileUsage;
  } PROCESS_MEMORY_COUNTERS;



BOOL WINAPI GetProcessMemoryInfo(HANDLE Process,
                 PROCESS_MEMORY_COUNTERS *ppsmemCounters,
                 DWORD cb     );

}

#else
#  include <unistd.h>
#  include <cstdio>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace lti {

  // returns the name of this type
  const char* processInfo::getTypeName() const {
    return "processInfo";
  }

# ifdef HAVE_PROC

  // --------------------------------------------------------
  //          UNIX/LINUX with /proc directory
  // --------------------------------------------------------

  const int processInfo::procStatus::MAX_SYSTEM_CPUS = 10;

  processInfo::procStatus::procStatus() {
    // init and set error values
    numberofcpus=0;
    processorload=-1.0;
    clockrate = new int[MAX_SYSTEM_CPUS];
  }

  processInfo::procStatus::~procStatus() {
    delete[] clockrate;
  }


  // destructor
  processInfo::~processInfo() {
    delete[] commBuffer;
    delete[] state.comm;
  }


  // default constructor
  processInfo::processInfo() {
    sprintf(procPath,"/proc/%d/stat",getpid());
    bufLength=1024;
    kEquals1024();
    commBuffer=new char[bufLength];
    state.comm=new char[bufLength];
    getCurrentCPUStatus();
    getCurrentStatus();
    getCurrentSystemStatus();
  }

  void processInfo::readBuffer() {
    // read process info buffer, and make sure that nothing is ever
    // overloaded
    while (true) {
      FILE *f=fopen(procPath,"r");
      int really=fread(commBuffer, 1, bufLength-1, f);
      fclose(f);
      if (really < bufLength-1) {
        break;
      }
      bufLength*=2;
      delete[] commBuffer;
      commBuffer=new char[bufLength];
      delete[] state.comm;
      state.comm=new char[bufLength];
    }
  }

  void processInfo::getCurrentCPUStatus() {
    int numberOfCPUs=0;
    float cpuInfo=0.0;
    char* workBuffer;

    // deleting array with old information
    for (int i=0;i<procStatus::MAX_SYSTEM_CPUS;i++) {

     state.clockrate[i]=0;
    }

    // get information from kernel-file
    workBuffer = new char[bufLength];
    FILE *f=fopen("/proc/cpuinfo","r");
    while (!feof(f)) {
      if (fgets(workBuffer,bufLength,f) != 0) {
        // read line (<=bufLength characters)
        return;
      }
      if (sscanf(workBuffer, "cpu MHz : %f ",&cpuInfo)!=0){
        state.clockrate[numberOfCPUs]=(int)(cpuInfo+0.5);
        numberOfCPUs++;
      }
    }
    fclose(f);
    state.numberofcpus=numberOfCPUs;

    // get information from kernel-file
    float f_dummy; int i_dummy;
    f=fopen("/proc/loadavg","r");
    if (f!=NULL) {
      // file contains one line of information
      if (fgets(workBuffer,bufLength,f) != 0) {
        // read line
        return;
      }
      sscanf(workBuffer, "%f %f %f %i/%i %i",&state.processorload,&f_dummy,
             &f_dummy,&i_dummy,&i_dummy,&i_dummy);
    }

    delete[] workBuffer;
    fclose(f);
  }

  void processInfo::getCurrentStatus() {
    unsigned int dummu;
    int dummi;
    readBuffer();
    // get the info from the proc file
    // order is:
    //
    // pid comm state ppid pgrp session tty tpgid flags
    // minflt cminflt majflt cmajflt
    // utime stime cutime cstime counter
    // priority timeout itrealvalue starttime
    // vsize rss rlim
    // startcode endcode startstack kstkesp kstkeip
    // signal blocked sigignore sigcatch wchan
    sscanf(commBuffer,
           "%d %s %c %d %d " \
           "%d %d %d %u %u %u" \
           "%u %u " \
           "%d %d %d %d %d " \
           "%d %u %u %d " \
           "%d %d %u " \
           "%u %u %u %u %u " \
           "%d %d %d %d %u",
           &state.pid, state.comm, &state.state, &state.ppid, &state.pgrp,
           &dummi, &dummi, &dummi, &dummu, &dummu, &dummu,
           &state.majflt, &state.cmajflt,
           &dummi, &dummi, &dummi, &dummi, &dummi,
           &state.priority,  &dummu, &dummu, &dummi,
           &state.vsize, &state.rss, &dummu,
           &dummu, &dummu, &dummu, &dummu, &dummu,
           &dummi, &dummi, &dummi, &dummi, &dummu);
  }

  void processInfo::getCurrentSystemStatus() {

    int dummy;
    char* workBuffer;

    static const int bufSize=1024;

    // init vars
    workBuffer = new char[bufSize];
    state.coremem=-1; state.freecoremem=-1;
    state.swapmem=-1; state.freeswapmem=-1;
    state.buffers=-1; state.cached=-1;

    // get the info from the /proc/memstat file
    // order is (example):
    //        total:    used:    free:  shared: buffers:  cached:
    //Mem:  133734400 104443904 29290496        0 12038144 62570496
    //Swap: 224493568        0 224493568
    //MemTotal: ...
    FILE *f=fopen("/proc/meminfo","r");
    if (fgets(workBuffer,bufSize,f) != 0) {
      // drop first line
      return;
    }

    if (fgets(workBuffer,bufSize,f) != 0) {
      return;
    }

    // terminate string in case of buffer overflow
    workBuffer[bufSize-1]='\0';
    sscanf(workBuffer, "Mem: %d %d %d %d %d %d", 
           &state.coremem, 
           &dummy,
           &state.freecoremem,
           &dummy,
           &state.buffers,
           &state.cached);
    if (fgets(workBuffer,bufSize,f) != 0) {
      return;
    }

    // terminate string in case of buffer overflow
    workBuffer[bufSize-1]='\0';
    sscanf(workBuffer, "Swap: %d %d %d", &state.swapmem, &dummy,
           &state.freeswapmem);
    delete[] workBuffer;
    fclose(f);
  }

  int processInfo::getUsedVirtualMemory() {
    getCurrentStatus();
    return state.vsize/sizeOfK;
  }

  int processInfo::getUsedCoreMemory() {
    getCurrentStatus();
    return (state.rss*4096)/sizeOfK;
  }

  int processInfo::getCoreMemory() {
    getCurrentSystemStatus();
    return state.coremem/sizeOfK;
  }

  int processInfo::getVirtualMemory() {
    getCurrentSystemStatus();
    return (state.coremem+state.swapmem)/sizeOfK;
  }

  int processInfo::getFreeCoreMemory() {
    getCurrentSystemStatus();
    return state.freecoremem/sizeOfK;
  }

  int processInfo::getBuffers() {
    getCurrentSystemStatus();
    return state.buffers/sizeOfK;
  }

  int processInfo::getCachedMemory() {
    getCurrentSystemStatus();
    return state.cached/sizeOfK;
  }

  int processInfo::getFreeSwapMemory() {
    getCurrentSystemStatus();
    return state.freeswapmem/sizeOfK;
  }

  int processInfo::getFreeVirtualMemory() {
    getCurrentSystemStatus();
    return (state.freecoremem+state.freeswapmem)/sizeOfK;
  }

  int processInfo::getNumberOfCPUs() {
    // set during init, number cannot change in running system
    // getCurrentCPUStatus();
    return state.numberofcpus;
  }

  int* processInfo::getProcessorClockRate() {
    // already set during init, update here
    getCurrentCPUStatus();
    return state.clockrate;
  }

  float processInfo::getProcessorLoad() {
    // already set during init, update here
    getCurrentCPUStatus();
    return state.processorload;
  }

  std::string processInfo::getHostname() const {
    char buffer[256];
    gethostname(buffer,255);
    buffer[255]='\0';
    return buffer;
  }

  std::string processInfo::getUsername() const {
    struct passwd* pwbuf=getpwuid(getuid());
    return pwbuf->pw_name;
  }

  long processInfo::getPid() const {
    return long(getpid());
  }


# else
# ifdef _LTI_WIN32

  // --------------------------------------------------------
  //                        WINDOWS
  // --------------------------------------------------------


 const int processInfo::procStatus::MAX_SYSTEM_CPUS = 10;
  // default constructor
  processInfo::processInfo() {
    state.pid=GetCurrentProcess();
    commBuffer=new PROCESS_MEMORY_COUNTERS;
    kEquals1024();
        getCpuStatus();
  }

  processInfo::~processInfo() {
    delete commBuffer;
  }


  void processInfo::getCpuStatus(){

        //determine cpu clockrate
        long RDTSC_start,RDTSC_end,RDTSC_elapsed;
        lti::timer myTimer;
        double sumClockrate=0;

        //count cpu cycles via RDTSC-Timer during measured time period
        //and average values
        for(int i=0;i<10;i++){
                myTimer.start();
                RDTSC_start = ReadRDTSCTimer();
                passiveWait(1000);
                myTimer.stop();
                RDTSC_end = ReadRDTSCTimer();
                double elapsedTime=myTimer.getTime();
                RDTSC_elapsed = RDTSC_end - RDTSC_start;
                sumClockrate+=double(RDTSC_elapsed/elapsedTime);
        }
        state.clockrate[0]=iround(sumClockrate/10);

        //determine number of CPUs
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
    state.numberOfCPUs=sysInfo.dwNumberOfProcessors;
  }

  processInfo::procStatus::procStatus() {
    // init and set error values
    clockrate = new int[MAX_SYSTEM_CPUS];
  }

  processInfo::procStatus::~procStatus() {
    delete[] clockrate;
  }



  void processInfo::getCurrentStatus() {
    if (GetProcessMemoryInfo((void*)state.pid,commBuffer,
                             sizeof(*commBuffer))) {
      state.wss=commBuffer->WorkingSetSize;
      state.pfu=commBuffer->PagefileUsage;
    } else {
      state.wss=0;
      state.pfu=0;
    }
  }

  int processInfo::getUsedVirtualMemory() {
    getCurrentStatus();
    if (state.pfu > state.wss) {
      return state.pfu/sizeOfK+1;
    } else {
      return state.wss/sizeOfK+1;
    }
  }

  int processInfo::getUsedCoreMemory() {
    getCurrentStatus();
    return state.wss/sizeOfK+1;
  }

  int processInfo::getCoreMemory() {
        MEMORYSTATUS memStatus;
        GlobalMemoryStatus(&memStatus);
        return memStatus.dwTotalPhys/sizeOfK;
  }

  int processInfo::getVirtualMemory() {
        MEMORYSTATUS memStatus;
        GlobalMemoryStatus(&memStatus);
        return memStatus.dwTotalVirtual/sizeOfK;
  }

  int processInfo::getFreeCoreMemory() {
        MEMORYSTATUS memStatus;
        GlobalMemoryStatus(&memStatus);
        return memStatus.dwAvailPhys/sizeOfK;
  }

  int processInfo::getFreeSwapMemory() {
        MEMORYSTATUS memStatus;
        GlobalMemoryStatus(&memStatus);
        return memStatus.dwAvailPageFile/sizeOfK;
  }

  int processInfo::getFreeVirtualMemory() {
        MEMORYSTATUS memStatus;
        GlobalMemoryStatus(&memStatus);
        return memStatus.dwAvailVirtual/sizeOfK;
  }

  int processInfo::getNumberOfCPUs() {
        return state.numberOfCPUs;
  }

  int* processInfo::getProcessorClockRate() {
        return state.clockrate;
  }


  std::string processInfo::getHostname() const {
    char buffer[256];
    DWORD nsize=255;
    if (GetComputerName(buffer,&nsize) != 0) {
      return std::string(buffer);
    } else {
      return "localhost";
    }
  }

  std::string processInfo::getUsername() const {
    char buffer[256];
    DWORD nsize=255;
    if (GetUserName(buffer,&nsize) != 0) {
      return std::string(buffer);
    } else {
      return "unknown";
    }
  }

  float processInfo::getProcessorLoad(){

    // queriing system registry using performance data helper Pdh
    HQUERY hQuery ;
    HCOUNTER hCounter;
    PDH_FMT_COUNTERVALUE data;

    if(PdhOpenQuery(NULL,0,&hQuery)!=ERROR_SUCCESS)return -1;
    if(PdhAddCounter(hQuery,"\\Prozessor(0)\\% Prozessorzeit",0,&hCounter)!=
      ERROR_SUCCESS)return -1;

    if(PdhCollectQueryData(hQuery)!=ERROR_SUCCESS)return -1;

    if(PdhGetFormattedCounterValue(hCounter,PDH_FMT_DOUBLE,NULL,&data)!=
      ERROR_SUCCESS)return -1;

    if((data.doubleValue>100)||(data.doubleValue<0))return -1;

    return static_cast<float>(data.doubleValue/100);
  }


  long processInfo::getPid() const {
    return long(_getpid());
  }


  long processInfo::ReadRDTSCTimer(){

        long cycles = 0;
# if defined(_LTI_MSC_VER)        
        __asm {
                cpuid
                rdtsc
                mov cycles,eax;
        }
# endif
# if defined(_LTI_GNUC)
        asm("cpuid\n\t"
            "rdtsc\n\t"
            "movl %eax, cycles\n\t");
# endif        
        return cycles;
  }


# else

  // --------------------------------------------------------
  //             Other unknown systems
  // --------------------------------------------------------

  // default constructor
  processInfo::processInfo() {
  }

  processInfo::~processInfo() {
  }

  void processInfo::getCurrentStatus() {
  }

  int processInfo::getUsedVirtualMemory() {
    return (-1);
  }

  int processInfo::getUsedCoreMemory() {
    return (-1);
  }

  int processInfo::getCoreMemory() {
    return (-1);
  }

  int processInfo::getVirtualMemory() {
    return (-1);
  }

  int processInfo::getFreeCoreMemory() {
    return (-1);
  }

  int processInfo::getFreeSwapMemory() {
    return (-1);
  }

  int processInfo::getFreeVirtualMemory() {
    return (-1);
  }

  int processInfo::getNumberOfCPUs() {
    return (0);
  }

  int* processInfo::getProcessorClockRate() {
    return NULL;
  }

  float processInfo::getProcessorLoad() {
    return (-1.0);
  }


  std::string processInfo::getHostname() const {
    char buffer[256];
    gethostname(buffer,255);
    buffer[255]='\0';
    return buffer;
  }

  std::string processInfo::getUsername() const {
    struct passwd* pwbuf=getpwuid(getuid());
    return pwbuf->pw_name;
  }

  long processInfo::getPid() const {
    return (-1.0);
  }



# endif
#endif
}
