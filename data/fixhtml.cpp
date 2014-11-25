/*

  Glt OpenGL C++ Toolkit (Glt)
  Copyright (C) 2000-2002 Nigel Stewart
  Email: nigels@nigels.com   WWW: http://www.nigels.com/glt/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <misc/string.h>

#include <string>
#include <list>
#include <fstream>
#include <iostream>
using namespace std;

// "Fix" HTML tags by matching all opening tags with closing
//  tags.  Do this based on tracking the current stack of
//  open tags.  Eg: <html><body><ul><li>... should have
//  matching </li></ul></body></html> tags....

void fixHTML(const string &str);

bool GlutMain(const std::vector<std::string> &arg)
{
    string in;
    readStream(cin,in);
    fixHTML(in);

    return true;
}


void fixHTML(const string &str)
{
    const char *begin = str.c_str();
    const char *end   = str.c_str() + str.size();

    list<string> stack;

    for (const char *i=begin; i<end;)
    {
        if (*i=='<')
        {
            const char *j = ++i;

            // Skip comments

            if (j<end && *j=='!')
            {
                while (j<end && *j!='>')
                    j++;

                // TODO - Output comment

                i = j+1;
            }
            else
            {
                // Get the tag name

                while (j<end && (*j=='/' || *j>='a' && *j<='z' || *j>='A' && *j<='Z'))
                    j++;

                string tag(i,j-i);

                if (tag.size()>0)
                {
                    if (tag[0]=='/')
                    {
                        // Pop until matching tag has been popped
                        while (stack.size())
                        {
                            string top = stack.front();
                            stack.pop_front();
                            if (top==tag.substr(1))
                                break;
                            else
                                cout << "</" << top << ">";
                        }
                    }
                    else
                    {
                        stack.push_front(tag);
                    }

                    cout << "<" << tag;
                }

                i = j;
            }
        }
        else
        {
            cout << *i;
            i++;
        }

    }
}

