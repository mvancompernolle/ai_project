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
#include "MHDictionary.h"

// initialize the standard error string
const string MHDictionary::errorString("<ERROR>");

// Construct a new, empty Dictionary.
// Since it's empty, it's sorted, too.
MHDictionary::MHDictionary() {
	mySize = 0;
	myWords.resize(0,"");
	myIndex.resize(0,0);
	myIndexed = true;
	numberOfPointersToMe = new BYTE4(0);
}

// Construct a new, unsorted Dictionary from the input string.
// Allow duplicate words.
MHDictionary::MHDictionary( const string &inphrase ) {
	mySize = 0;
	myWords.resize(0,"");
	myIndex.resize(0,0);
	myIndexed = true;
	numberOfPointersToMe = new BYTE4(0);
	size_t offset = 0;
	string phrase( inphrase );
	/*
	 *		If the string is empty then do nothing, for it contains no words.
	 */
	if( phrase.size() == 0 ) { return; }

	/*
	 *		Loop forever.
	 */
	while( true ) {
		/*
		 *		If the current character is of the same type as the previous
		 *		character, then include it in the word.  Otherwise, terminate
		 *		the current word.
		 */
		if( MHFuncs::boundary(phrase, offset) ) {
			/*
			 *		Add the word to the dictionary
			 */
			push_word( phrase.substr(0,offset) );

			if( offset == phrase.size() ) { break; }
			phrase = phrase.substr( offset, phrase.size() - offset );
			offset = 0;
		} else {
			++offset;
		}
	}

	/*
	 *		If the last word isn't punctuation, then add punctuation.
	 */
	if(isalnum(myWords[mySize-1][0])) {
		push_word(".");
	}
	else if( phrase.find_first_of( "!?.", 0 ) == basic_string<char>::npos ) {
		myWords[mySize-1] = ".";
	}

   return;
}

// Set this Dictionary equal to another.
MHDictionary &MHDictionary::operator=( const MHDictionary &other ) {
	myWords.resize( 0 );
	mySize = 0;
	myIndexed = true;
	for( BYTE2 i = 0; i < other.mySize; ++i ) {
		add_word( other.myWords[i] );
	}
	return *this;
}

// Erase the dictionary.
// Since it's now empty, it's sorted too.
void MHDictionary::free() {
	myWords.resize( 0 );
	mySize = 0;
	myIndexed = true;
}

// Push a word onto the end of the dictionary.
// This makes it unsorted.
BYTE2 MHDictionary::push_word( const string &newWord ) {
	if( myIndexed && mySize == 2 ) {
		mySize = 0;
		myWords.resize( 0 );
		myIndex.resize( 0 );
	}
	myIndexed = false;
	myWords.push_back( newWord );
	MHFuncs::upper( myWords[mySize]);
	myIndex.push_back( 0 );
	++mySize;
	return( mySize - 1 );
}

// Insert a word at a specific place in the dictionary.
// This makes it unsorted.
BYTE2 MHDictionary::insert_word( BYTE2 pos, const string &newWord ) {
	if( myIndexed && mySize == 2 ) {
		mySize = 0;
		myWords.resize( 0 );
		myIndex.resize( 0 );
	}
	myIndexed = false;
	pos = pos < mySize ? pos : mySize;
	myWords.insert( myWords.begin() + pos, newWord );
	MHFuncs::upper( myWords[pos]);
	myIndex.push_back( 0 );
	++mySize;
	return( pos );
}

// Insert a word in the correct position in the dictionary.
// This only works if the dictionary is sorted.
// Otherwise, push_word will be called instead.
BYTE2 MHDictionary::add_word( const string &newWord ) {
	if( myIndexed == false ) { return( push_word( newWord ) ); }

	BYTE2 position;
	bool found;
	/* 
	 *		If the word's already in the dictionary, there is no need to add it
	 */
	position = search( newWord, &found );
	if( found==true ) { return( myIndex[position] ); }
	/* 
	 *		Increase the number of words in the dictionary
	 */
	myWords.push_back( newWord );
	MHFuncs::upper( myWords[mySize] );
	myIndex.insert( myIndex.begin() + position, mySize );
	++mySize;
	return( myIndex[position] );
}



// search for the specified word in the dictionary.
// this is a very efficient binary search which works only on a sorted dictionary.
// if the dictionary is unsorted, the unindexed_search simply examines every
// single word in the dictionary to find the specified word.
BYTE2 MHDictionary::search( const string &thisWord, bool *find) const {
	if( myIndexed == false ) { return( unindexed_search( thisWord, find ) ); }
	*find = false;
	BYTE2 min;
	BYTE2 max;
	BYTE2 middle;
	int compar;
	/*
	 *		If the dictionary is empty, then obviously the word won't be found
	 */
	if( mySize == 0 ) {
		return( 0 );
	}
	/*
	 *		Initialize the lower and upper bounds of the search
	 */
	min = 0;
	max = mySize - 1;
	/*
	 *		Search repeatedly, halving the search space each time, until either
	 *		the entry is found, or the search space becomes empty
	 */
	while( true ) {
		/*
		 *		See whether the middle element of the search space is greater
		 *		than, equal to, or less than the element being searched for.
		 */
		middle = ( min + max ) / 2;
		compar = MHFuncs::wordcmp( thisWord, myWords[myIndex[middle]] );
		//DEBUG
		//cout << "compare(" << thisWord << "," << myWords[myIndex[middle]] << ") => " << compar << endl;
		
		/*
		 *		If it is equal then we have found the element.  Otherwise we
		 *		can halve the search space accordingly.
		 */
		if( compar == 0 ) {
			*find = true;
			return( middle );
		}
		else if( compar > 0 ) {
			if( max == middle ) {
				return( middle + 1 );
			}
			min = middle + 1;
		} else {
			if( min == middle ) {
				return( middle );
			}
			max = middle - 1;
		}
	}
}


/*
 *		Function:	Find_Word
 *
 *		Purpose:		Return the symbol corresponding to the word specified.
 *						We assume that the word with index zero is equal to a
 *						NULL word, indicating an error condition.
 */
BYTE2 MHDictionary::find_word( const string &thisWord ) const {
	BYTE2 position;
	bool found;
	position = search( thisWord, &found );
	return( found == true ? myIndex[position] : 0 );
}

// return true if the word was found in the dictionary.
bool MHDictionary::word_exists( const string &thisWord ) const {
	bool found;
	search( thisWord, &found );
	return found;
}

// search for the specified word in the dictionary.
// if the dictionary is sorted, use the more efficient search() function.
BYTE2 MHDictionary::unindexed_search( const string &thisWord, bool *find ) const {
	if( myIndexed == true ) { return( search( thisWord, find ) ); }
	*find = false;
	for( BYTE2 i = 0; i < mySize; ++i ) {
		if( MHFuncs::wordcmp( myWords[i], thisWord ) == 0 ) {
			*find = true;
			return( i );
		}
	}
	return( 0 );
}


/*
 *		Function:	similar
 *
 *		Purpose:		Return TRUE or FALSE depending on whether the dictionaries
 *						are the same or not.
 */
bool MHDictionary::similar( const MHDictionary &other ) const {
	if( mySize != other.mySize ) return(false);
	for( BYTE2 i = 0; i < mySize; ++i ) {
		if( MHFuncs::wordcmp( myWords[i], other.myWords[i] ) != 0 ) {
			return(false);
		}
	}
	return(true);
}


/*
 *		Function:	Make_Output
 *
 *		Purpose:		Generate a string from the dictionary of reply words.
 */
string MHDictionary::make_output() const {
	string output;
	if( mySize == 0 ) {
		output = "I am utterly speechless!";
		return(output);
	}
	for( BYTE2 i = 0; i < mySize; ++i ) {
		output.append( myWords[i] );
	}
	return(output);
}


/*
 *		Function:	Save_Dictionary
 *
 *		Purpose:		Save a dictionary to the specified file.
 */
void MHDictionary::save_dictionary( ostream &out ) const {
	BYTE4 saveSize = (BYTE4)mySize;
	BYTE1 myWordSize;
	out.write( (char*)&saveSize, sizeof( BYTE4 ) );
	for( BYTE2 i = 0; i < mySize; ++i ) {
		myWordSize = (BYTE1)myWords[i].size();
		out.write( (char*)&(myWordSize), sizeof( BYTE1 ) );
		out << myWords[i];
	}
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Dictionary
 *
 *		Purpose:		Load a dictionary from the specified file.
 */
void MHDictionary::load_dictionary( istream &in ) {
	free();
	BYTE4 newSize;
	BYTE1 newWordSize;
	char tc[2048];
	string bufstr;
	in.read( (char*)&newSize,  sizeof( BYTE4 ) );
	for( BYTE2 i = 0; i < newSize; ++i ) {
		in.read( (char*)&(newWordSize), sizeof( BYTE1 ) );
		in.read( tc, sizeof( char ) * newWordSize );
		tc[newWordSize] = '\0';
		bufstr = tc;
		add_word( bufstr );
	}
}


/*
 *		Function:	Initialize_List
 *
 *		Purpose:		Read a dictionary from a file.
 */
int MHDictionary::initialize_list( const string &fileName ) {
	ifstream infile;
	char buffer[2048]; // buffer to use when reading infile
	string bufstr;
	BYTE2 pos;
	infile.open(fileName.c_str(), ios::in);
	if( ! infile ) {
		return( 1 );	// unable to open file
	}
	while( infile.getline( buffer, 4096 ) ) {
		if( buffer[0] == '#' ) { continue; }
		bufstr = buffer;
		pos = (BYTE2)bufstr.find_first_of( "\t #\n", 0 );
		bufstr = bufstr.substr( 0, pos );
		add_word( bufstr );
	}
	infile.close();
	return( 0 );
}


/*
 *		Function:	Show_Dictionary
 *
 *		Purpose:		Display the dictionary for training purposes.
 */
int MHDictionary::show_dictionary( const string &filename ) const {
	ofstream outfile;
	outfile.open(filename.c_str(), ios::out);
	if( ! outfile ) {
		return( 1 );	// unable to open file
	}
	for( BYTE2 i = 0; i < mySize; ++i ) {
		outfile << i << ":" << myWords[i] << "\t\t:" << myIndex[i] << endl;
	}
	outfile.close();
	return( 0 );
}

// print the dictionary for debugging purposes.
void MHDictionary::print_dictionary() const {
	cout << "\n\n/";
	for( BYTE2 i = 0; i < mySize; ++i ) {
		cout << myWords[i] << "/";
	}
	cout << "\n";
}
