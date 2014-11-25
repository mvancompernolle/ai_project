/*===========================================================================*/
/*
 *  Copyright (C) 2002 plushpuffin@yahoo.com
 *  Copyright (C) 1998 Jason Hutchens
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the Gnu Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*===========================================================================*/

#include "stdafx.h"
using namespace std;

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
	MHConsole con;
	con.main_loop();
	return 0;
}
#else
int main(int argc, char* argv[])
{
	MHConsole con;
	con.main_loop();
	return 0;
}
#endif
