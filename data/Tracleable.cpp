/***************************************************************************
 *   Copyright (C) 2004-2006 by Ilya A. Volynets-Evenbakh                  *
 *   ilya@total-knowledge.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <servlet/Traceable.h>
#include <execinfo.h>
#include <iostream>
#include <cxxabi.h>
#include <stdlib.h>

namespace servlet {

#define MAX_TRACE 40

/*!
 * \fn servlet::Traceable::Traceable(const std::string& msg="")
 */
Traceable::Traceable(const std::string& msg)
	: msg(msg)
{
	void * array[MAX_TRACE];
	int nSize = backtrace(array, MAX_TRACE);
	char ** symbols = backtrace_symbols(array, nSize);
	// Start with 1, in order to exclude this constructor itself
	// from the trace. We know we've been here anyways :)
	trace.resize(nSize-1);
	char* dmBuf=(char*)malloc(120);
	char* dmName;
	size_t sz=120;
	try {
		for(int i=1;i<nSize;i++){
			std::string &s(trace[nSize-1-i]);
			s.assign(symbols[i]);
			std::string::size_type p=s.find('('),p2=std::string::npos;
			if(p!=std::string::npos){
				p2=s.find(')',++p);
			}
			int st=-1;
			if(p!=std::string::npos && p2!=std::string::npos) {
				std::string::size_type plus=s.find('+', p);
				p2--;
				if(plus==std::string::npos)
					plus=p2;
				std::string s2=(s.substr(p,plus-p));
				dmName=abi::__cxa_demangle(s2.c_str(),dmBuf,&sz,&st);
			}
			if(st!=0) {
				switch(st) {
				case -3:
					s.insert(p,"cannot demangle: one of the arguments is invalid.");
					break;
				case -2:
					//s.insert(p,"cannot demangle: mangled name is invalid under C++ ABI rules.");
					//do nothing, since this is the case of C names...
					break;
				case -1:
					s.insert(p,"cannot demangle: memory allocation failure");
					break;
				default:
					s.insert(p,"unknown error while demangling ");
				}
			}else if(dmName){
				s.erase(p,p2-p+1);
				s.insert(p,dmName);
				dmBuf = dmName;
			}
		}
	} catch (...) {
		// Do nothing... We can't handle this problem
	}
	if(dmBuf) free(dmBuf);
	if(symbols) ::free(symbols);
}

/*!
 * \fn servlet::Traceable::~Traceable()
 */
Traceable::~Traceable() throw()
{
}

}


/*!
 * \fn void servlet::Traceable::printStackTrace(std::ostream &os)
 * @param os output stream to send stack trace to
 * Outputs stack trace of this exception to os
 */
void servlet::Traceable::printStackTrace(std::ostream& os) const
{
	os<<getMsg()<<":\n";
	for(trace_t::const_iterator i=trace.begin();i!=trace.end();i++){
		os<<*i<<std::endl;
	}
}
