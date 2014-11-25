/***************************************************************************
 *   Copyright (C) 2006 by Ilya A. Volynets-Evenbakh                       *
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

#include <servlet/Util.h>
#include <stdio.h>
#include <stdlib.h>

namespace servlet
{
namespace util
{

std::string urlDecode(const std::string& s)
{
	std::string result(s);
	urlInPlaceDecode(result);
	return result;
}


/*!
    \fn urlDecode(const std::string& s)
 */
void urlInPlaceDecode(std::string& s)
{
	std::string::size_type w=0;
	for (std::string::size_type p=0; p < s.size(); p++,w++) {
		if (s[p] == '+'){
			s[w]= ' ';  // translate '+' to ' ':
		} else if ((s[p] == '%') && (p + 2 < s.size())) {  // check length
			char str[]={s[p+1],s[p+2],0};
			char *pp;
			unsigned char c=(unsigned char)strtoul(str,&pp,16);
			if(pp==str+2){
				s[w]=c;
				p+=2;
			} else {
				s[w]=s[p];
			}
		}else {
			s[w]=s[p];
		}
	}
	s.erase(w);
}

/*!
 * \fn urlEncode(const std::string& source, std::string& dest)
 * @param source String to encode
 * @param dest Where to put encoded string
 * @return dest
 */
std::string& urlEncode(const std::string& source, std::string& dest)
{
	dest.clear();
	for(std::string::size_type r=0; r< source.length(); r++) {
		const char c=source[r];
		if((c>'0' && c<'9')||(c>'a' && c<'z')
		   ||(c>'A' && c<'Z') || c=='_')
		{
			dest+=c;
		}
		else
		{
			char hex[4];
			sprintf(hex,"%%%x",(unsigned int)c);
			dest.append(hex);
		}
	}
	return dest;
}

}
}
