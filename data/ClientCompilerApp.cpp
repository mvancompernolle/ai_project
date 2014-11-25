/*

CLIENTCOMPILERAPP.CPP: COMPILER SAMPLE FOR CALCULUS-CPP

This application is an example of an off-line compiler using calculus-cpp
to do minimalist formal verifications.  It may also be used to analyze
the produced binaries for regression or performance-tweaking purposes.  The
output from this compiler is a set of binary files which may be read
by a dissassmbler - though I have no such dissassembler to recommend.

Use this application's source code to explore the Functional parsing
capabilities of calculus-cpp.

* calculus-cpp: Scientific "Functional" Library
*
* This software was developed at McGill University (Montreal, 2002) by
* Olivier Giroux in the course of his studies in Mechanical Engineering.
* It was presented, along with an accompanying paper, for credit in the fall
* of 2002.
*
* Calculus-cpp was not designed to prove a point or to serve as a formal
* framework within which exact solutions can be derived.  Instead it was
* created to fill the need for run-time Functional constructions and to
* accomplish very real and tangible goals.  It remains your responsibility
* to use it properly - as much more sophisticated <math.h>, which allows
* Functions to be treated as first-class objects.
*
* You are welcome to make any additions you feel are necessary.

COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2002, Olivier Giroux, <oliver@canada.com>.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without any restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
provided that the copyright notice(s) and this permission notice appear
in all copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.

THIS SOFTWARE INCLUDES THE NIST'S TNT PACKAGE FOR USE WITH THE EXAMPLES FURNISHED.

THE FOLLOWING NOTICE APPLIES SOLELY TO THE TNT-->
* Template Numerical Toolkit (TNT): Linear Algebra Module
*
* Mathematical and Computational Sciences Division
* National Institute of Technology,
* Gaithersburg, MD USA
*
*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
<--END NOTICE

THE FOLLOWING NOTICE APPLIES SOLELY TO LEMON-->
** Copyright (c) 1991, 1994, 1997, 1998 D. Richard Hipp
**
** This file contains all sources (including headers) to the LEMON
** LALR(1) parser generator.  The sources have been combined into a
** single file to make it easy to include LEMON as part of another
** program.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
** 
** You should have received a copy of the GNU General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
** Author contact information:
**   drh@acm.org
**   http://www.hwaci.com/drh/
<--END NOTICE

*/
//GET THE STANDARD IO SET UP
#include <stdio.h>
//TEST CONFLICTS WITH THE STANDARD MATH
#include "math.h"
//THE NUMERICAL PACKAGE
#include <calculus.h>

#ifdef _MSC_VER
    #undef UNREFERENCED_PARAMETER
    #include <crtdbg.h>
    #include <windows.h>
#endif

int EnumerateExplicitFiles(char * pscArg,unsigned int *puiNumSourceFiles,char *** pppscSourceFiles) {
	unsigned int uiNumSourceFiles = *puiNumSourceFiles+1;
	char * pscCurrentPos = pscArg;
	unsigned int uiFileNameSize;
	while(*pscCurrentPos) {
		if (*pscCurrentPos == ',' && *(pscCurrentPos-1) != ',')
			uiNumSourceFiles++;
		pscCurrentPos++;
	}
	char ** ppscSourceFiles = new char*[uiNumSourceFiles];
	for(unsigned int i = 0;i < *puiNumSourceFiles;i++) {
        ppscSourceFiles[i] = new char[strlen((*pppscSourceFiles)[i])];
		strcpy(ppscSourceFiles[i],(*pppscSourceFiles)[i]);
	}
	pscCurrentPos = pscArg;
	for(;i < uiNumSourceFiles;i++) {
		pscArg = pscCurrentPos;
		uiFileNameSize = 0;
		while(*(pscCurrentPos++) && *(pscCurrentPos++) != ',');
		while(*(pscCurrentPos++) && *pscCurrentPos == ',');
        ppscSourceFiles[i] = new char[uiFileNameSize+1];
		strncpy(ppscSourceFiles[i],pscArg,uiFileNameSize);
		ppscSourceFiles[i][uiFileNameSize] = 0;
	}
	if (*pppscSourceFiles)
		delete [] *pppscSourceFiles;
	*pppscSourceFiles = ppscSourceFiles;
	return *puiNumSourceFiles = uiNumSourceFiles;
}

int EnumerateImplicitFiles(unsigned int *puiNumSourceFiles,char *** pppscSourceFiles) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	printf ("\nAutomatically looking for files, target type is *.CAL\n");

	hFind = FindFirstFile("*.CAL", &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf ("There are no CAL files in this directory");
	}
	else
	{
		do
		{
			printf ("Found a file : %s\n", FindFileData.cFileName);
			(*puiNumSourceFiles)++;
			if (*pppscSourceFiles)
			{
				*pppscSourceFiles = (char**)realloc(*pppscSourceFiles,*puiNumSourceFiles*sizeof(char**));
			}
			else
			{
				*pppscSourceFiles = (char**)malloc(*puiNumSourceFiles*sizeof(char**));
			}
			(*pppscSourceFiles)[*puiNumSourceFiles-1] = new char[strlen(FindFileData.cFileName)+1];
			strcpy((*pppscSourceFiles)[*puiNumSourceFiles-1],FindFileData.cFileName);
			_strupr((*pppscSourceFiles)[*puiNumSourceFiles-1]);
		}
		while(FindNextFile(hFind,&FindFileData));
		FindClose(hFind);
	}
	printf("Now have %d files to compile\n\n",*puiNumSourceFiles);
	return *puiNumSourceFiles;
};

int EnumerateOutputFilesFromInputFiles(unsigned int uiNumFiles,char ** ppscSourceFiles,unsigned int * puiNumOutputFiles,char *** pppscDestinationFiles,bool toBin = true)
{
	if (*puiNumOutputFiles > uiNumFiles)
		_ASSERT(false);
	if (*pppscDestinationFiles)
	{
        *pppscDestinationFiles = (char**)realloc(*pppscDestinationFiles,uiNumFiles*sizeof(char*));
	}
	else
	{
		*pppscDestinationFiles = (char**)malloc(uiNumFiles*sizeof(char*));
	}
	for(unsigned int i = *puiNumOutputFiles;i < uiNumFiles;i++)
	{
		char * pCurrentPos = ppscSourceFiles[i];
		pCurrentPos += strlen(ppscSourceFiles[i]);
		while(*pCurrentPos != '.' && pCurrentPos > ppscSourceFiles[i])
		{
			pCurrentPos--;
		}
		(*pppscDestinationFiles)[i] = new char[pCurrentPos-ppscSourceFiles[i]+5];
		strncpy((*pppscDestinationFiles)[i],ppscSourceFiles[i],pCurrentPos-ppscSourceFiles[i]);
		(*pppscDestinationFiles)[i][pCurrentPos-ppscSourceFiles[i]] = 0;
		if (toBin)
		{
			strcat((*pppscDestinationFiles)[i],".BIN");
		}
		else
			strcat((*pppscDestinationFiles)[i],".OBJ");
	}
	return *puiNumOutputFiles = uiNumFiles;
};

void FreeFiles(unsigned int uiNumFiles,char ** ppscSourceFiles,char ** ppscDestinationFiles)
{
	for(unsigned int i = 0;i < uiNumFiles;i++)
	{
		if (ppscSourceFiles[i])
			delete ppscSourceFiles[i];
		ppscSourceFiles[i] = NULL;
		if (ppscDestinationFiles[i])
			delete ppscDestinationFiles[i];
		ppscDestinationFiles[i] = NULL;
	}
	delete ppscDestinationFiles;
	delete ppscSourceFiles;
};

int main( int argc, char *argv[ ], char *envp[ ] )
{
	UNREFERENCED_PARAMETER(envp);
	unsigned int uiNumSourceFiles = 0;
	unsigned int uiNumDestinationFiles = 0;
	char ** ppscSourceFiles = NULL;
	char ** ppscDestinationFiles = NULL;
	bool bUsingExplicitMode = false;
	bool bPerformDifferentiationTest = false;
	bool bOutputToCalulusBinaries = true;
	bool bOutputToScreen = true;
	bool bVerboseMode = false;
	printf("\n\n");
    
    //_CrtSetBreakAlloc(242);

	for(int i = 1;i < argc;i++)
	{
		_strupr(argv[i]);
		switch(argv[i][0])
		{
		case '-' :
		case '/' :
			switch(argv[i][1])
			{
			case 'T' :
				bPerformDifferentiationTest = true;
				break;
			case 'S' :
				//	SUPPLY (A) SOURCE FILE(S), IF NOT THEN COMPILE ALL .CAL FILES
				bUsingExplicitMode = true;
				i += EnumerateExplicitFiles(argv[i]+3,&uiNumSourceFiles,&ppscSourceFiles);
				break;
			case 'D' :
				//	SUPPLY (A) DESTINATION FILE(S), IF NOT THEN DERIVE FROM ORIGINAL FILE NAME(S)
				EnumerateExplicitFiles(argv[i]+3,&uiNumDestinationFiles,&ppscDestinationFiles);
				break;
			case 'N' :
				//	OUTPUT ALL TO FILE
				if (strcmp(argv[i]+1,"DISK"))
					break;
				bOutputToScreen = true;
				break;
			case 'B' :
				//	USE THE CALCULUS BINARY FILE FORMAT (DEFAULT)
				if (strcmp(argv[i]+1,"BIN"))
					break;
				bOutputToCalulusBinaries = true;
				break;
			case 'V' :
				//	USE THE CALCULUS BINARY FILE FORMAT (DEFAULT)
				bVerboseMode = true;
				break;
			case '?' :
				//	OUTPUT HELP INFO TO CONSOLE
				printf("\nThese are the command line arguments that are supported\n");
				printf("\n\t/S[File_1[,File_N]]\tSupply one or more explicit file(s) to compile.\n\t\t\t\tDefault uses all files with the extension found in the currentdirectory are compiled.");
				printf("\n\t/D[File_1[,File_N]]\tSupply one or more explicit destination file(s) to compile to.\n\t\t\t\tDefault derives names from the source file names.");
				printf("\n\t/N\t\t\tForce output to screen instead of disk. (default is output to disk files.)");
				printf("\n\t/B\t\t\tUse the calculus binary file format (default).");
				printf("\n\t/T\t\t\tPerform an exhaustive differentiation test.");
				break;
			default :
				printf("Invalid command-line argument : %s",argv[i]);
				break;
			}
			break;
		default :
			printf("Invalid command-line argument : %s",argv[i]);
			break;
		}
	}
	if (!uiNumSourceFiles)
		EnumerateImplicitFiles(&uiNumSourceFiles,&ppscSourceFiles);

	EnumerateOutputFilesFromInputFiles(uiNumSourceFiles,ppscSourceFiles,&uiNumDestinationFiles,&ppscDestinationFiles);

	initialize_calculus(0);
	{

		for(unsigned int i = 0;i < uiNumSourceFiles;i++)
		{
			HANDLE hSourceFile = CreateFile(ppscSourceFiles[i],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

			if (hSourceFile != INVALID_HANDLE_VALUE)
			{
				HANDLE hDestinationFile = CreateFile(ppscDestinationFiles[i],GENERIC_WRITE,NULL,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);		

				printf("\nCompiling file #%d : \t%s \t-> \t%s\n",i,ppscSourceFiles[i],ppscDestinationFiles[i]);

				DWORD dwBytesToRead = GetFileSize(hSourceFile,NULL);
				DWORD dwBytesRead;
				printf("File size is %u\n",dwBytesToRead);
				char * pFunctionString = new char[dwBytesToRead+1];
				pFunctionString[dwBytesToRead] = 0;
				if (ReadFile(hSourceFile,pFunctionString,dwBytesToRead,(LPDWORD)&dwBytesRead,NULL))
				{
					printf("The file contained the instructions ---->   %s   <----\n",pFunctionString);

                    Function Alg = calculus::algebra_parser::get_service()->parse_to_algebra(pFunctionString);
					if (Alg != NULL)
					{
                        char * scBuffer = new char[Alg->to_string(NULL)+1];

						Alg->to_string(scBuffer);

						printf("Function was BUILT as %s\n",scBuffer);

                        Function Proof = calculus::algebra_parser::get_service()->parse_to_algebra(scBuffer);

						if (Proof != NULL)
						{
							char * scProofBuffer = new char[Proof->to_string(NULL)+1];
                            Proof->to_string(scProofBuffer);
							printf("Function was REBUILT as %s\n",scProofBuffer);

							if (bPerformDifferentiationTest)
							{
                                Function * pDerivatives = new Function[7];
                                pDerivatives[0] = Alg;
								for(unsigned int i = 1;i < 7;i++)
								{
                                    pDerivatives[i] = pDerivatives[i-1]->get_partial_derivative(calculus::_var("x"));
									unsigned int uiAlloc = 0;
                                    char * pscDerivativeBuffer = new char[uiAlloc = (pDerivatives[i]->to_string(NULL)+1)];
                                    pDerivatives[i]->to_string(pscDerivativeBuffer);
									uiAlloc -= strlen(pscDerivativeBuffer)+1;
									if (uiAlloc)
										__asm int 3
									printf("i-th (i=%u) derivative of the Function wrt x is : %s\n",i,pscDerivativeBuffer);
									delete [] pscDerivativeBuffer;
								}
                                delete [] pDerivatives;
							}
							delete [] scProofBuffer;
						}
						else
						{
							printf("Failed to REBUILD the Function\n");
						}

	/*					FUNCTION f = Alg;
						if (f)
						{
							DWORD dwBytesWritten,dwBytesToWrite;
							char scFileBuffer[4096];
							
							sprintf(scFileBuffer,"%u%s",strlen(scBuffer),scBuffer);
							dwBytesToWrite = (DWORD)strlen(scFileBuffer);
							WriteFile(hDestinationFile,scFileBuffer,dwBytesToWrite,(LPDWORD)&dwBytesWritten,NULL);
							if (dwBytesWritten != dwBytesToWrite)
								printf("CopyToDiskFailed");
							WriteFile(hDestinationFile,GetFunctionHeader(f),dwBytesToWrite = GetFunctionHeader(f)->dwMemSize,(LPDWORD)&dwBytesWritten,NULL);
							if (dwBytesWritten != dwBytesToWrite)
								printf("CopyToDiskFailed");
						}
						else
						{
							printf("Failed to compile the built tree");
						}
	*/
						delete [] scBuffer;
					}
					else
					{
						printf("Failed to BUILD the Function\n");
					}
				}
				delete [] pFunctionString;
				CloseHandle(hSourceFile); 
				CloseHandle(hDestinationFile); 
			}
			else
				printf("error CALCC1001: The file %s could not be located.",ppscSourceFiles[i]);

		}

		FreeFiles(uiNumSourceFiles,ppscSourceFiles,ppscDestinationFiles);
		printf("\n\n");
	}
	uninitialize_calculus();

	return 0;
}