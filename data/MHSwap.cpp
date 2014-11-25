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
#include "MHSwap.h"

// initialize the MHSwap standard error string.
const string MHSwap::errorString("<ERROR>");

// Create a new, empty Swap.
MHSwap::MHSwap() {
	mySize = 0;
	myTo.resize( 0, "" );
	myFrom.resize( 0, "" );
	numberOfPointersToMe = new BYTE4(0);
}

// Add a word pair to the MHSwap.
void MHSwap::add_swap( const string &newFrom, const string &newTo ) {
	myFrom.push_back( newFrom );
	myTo.push_back( newTo );
	++mySize;
}

// Initialize the MHSwap from a file.
// The file must contain whitespace between the two words.
// The file must have one swappable pair per line.
// Returns zero on success or non-zero on failure.
int MHSwap::initialize_swap( const string &fileName ) {
	ifstream infile;
	char buffer[2048]; // buffer to use when reading infile
	string bufstr, tostr, fromstr;
	BYTE2 pos;
	infile.open(fileName.c_str(), ios::in);
	if( ! infile ) {
		return( 1 );	// unable to open file
	}
	while( infile.getline( buffer, 4096 ) ) {
		if( buffer[0] == '#' ) { continue; }
		bufstr = buffer;
		pos = (BYTE2)bufstr.find_first_of( "\t ", 0 );
		fromstr = bufstr.substr( 0, pos++ );
		while( ! isalnum( bufstr[pos] ) ) {
			++pos;
		}
		tostr = bufstr.substr( pos, bufstr.size() - pos );
		add_swap( fromstr, tostr );
	}
	infile.close();
	return( 0 );
}

// Print the MHSwap for debugging purposes.
void MHSwap::print_swap() {
	cout << "SWAP:\n";
	for( size_t i = 0; i < myFrom.size(); ++i ) {
		cout << "/" << myFrom[i] << "->" << myTo[i] << "/"<< endl;
	}
	cout << endl;
}

// end of MHSwap class.
