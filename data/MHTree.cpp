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
#include "MHTree.h"

// Construct a new, empty MHTree.
MHTree::MHTree() {
	mySymbol = 0;
	myUsage = 0;
	myCount = 0;
	myBranch = 0;
	myChildren.resize( 0 );
#ifdef MHTREE_MAINTAINS_COUNTER
	numberOfPointersToMe = new BYTE4(0);
#endif
} // Constructor()

// Free all memory for this tree and its children
// and its children's children, etc.
void MHTree::free() {
	for( BYTE2 i = 0; i < myBranch; ++i ) {
		if( myChildren[i] != NULL ) {
			delete myChildren[i];
		}
	}
	mySymbol = 0;
	myUsage = 0;
	myCount = 0;
	myBranch = 0;
	myChildren.resize( 0 );
}

// Add a symbol to this Tree.
MHTree *MHTree::add_symbol(BYTE2 symbol) {
	MHTree *node = NULL;

	/*
	 *		Search for the symbol in the subtree of the tree node.
	 */
	node = find_symbol_add(symbol);

	/*
	 *		Increment the symbol counts
	 */
	if( node->myCount < 65535 ) {
		node->myCount += 1;
		myUsage += 1;
	}

	return(node);
}

// Return the node associated with the symbol,
// or NULL if not found.
MHTree *MHTree::find_symbol(BYTE2 symbol) const
{
	BYTE2 i;
	MHTree *found = NULL;
	bool found_symbol = false;

	/* 
	 *		Perform a binary search for the symbol.
	 */
	i = search_node( symbol, &found_symbol );
	if( found_symbol == true ) {
		found = myChildren[i];
	}
	return(found);
}

// Return the node associated with the symbol.
// Create the node if not found.
MHTree *MHTree::find_symbol_add(BYTE2 symbol)
{
	BYTE2 i;
	MHTree *found = NULL;
	bool found_symbol = false;

	/* 
	 *		Perform a binary search for the symbol.  If the symbol isn't found,
	 *		attach a new sub-node to the tree node so that it remains sorted.
	 */
	i = search_node(symbol, &found_symbol);
	if( found_symbol == true ) {
		found = myChildren[i];
	}
	else {
		found = new MHTree();
		found->mySymbol = symbol;
		add_child(found, i);
	}

	return(found);
}

// Add a child to the current node.
void MHTree::add_child(MHTree *node, BYTE2 position)
{
	myChildren.insert(myChildren.begin() + position, node);
	myBranch += 1;
}

// Look for the symbol in this Tree, return its position
// or its expected position, and modify the referenced boolean for
// the result of the search. (was it found? true/false)
BYTE2 MHTree::search_node(BYTE2 symbol, bool *found_symbol) const {
	BYTE2 position;
	BYTE2 min;
	BYTE2 max;
	BYTE2 middle;
	int compar;

	/*
	 *		Handle the special case where the subtree is empty.
	 */ 
	if( myBranch == 0 ) {
		position = 0;
		*found_symbol = false;
		return position;
	}

	/*
	 *		Perform a binary search on the subtree.
	 */
	min = 0;
	max = myBranch - 1;
	while( true ) {
		middle = (min + max) / 2;
		compar = symbol - myChildren[middle]->mySymbol;
		if( compar == 0 ) {
				position = middle;
				*found_symbol = true;
				return position;
		} else if( compar > 0 ) {
			if( max == middle ) {
				position = middle + 1;
				*found_symbol = false;
				return position;
			}
			min = middle + 1;
		} else {
			if( min == middle ) {
				position = middle;
				*found_symbol = false;
				return position;
			}
			max = middle - 1;
		}
	}
}

// save the tree to the output stream.
void MHTree::save_tree(ostream &out) const {
	out.write( (char*)&mySymbol, sizeof( BYTE2 ) );
	out.write( (char*)&myUsage,  sizeof( BYTE4 ) );
	out.write( (char*)&myCount,  sizeof( BYTE2 ) );
	out.write( (char*)&myBranch, sizeof( BYTE2 ) );

	for( BYTE2 i = 0; i < myBranch; ++i ) {
		myChildren[i]->save_tree( out );
	}
}

// print the tree to console for debugging.
void MHTree::print_tree( ostream &out ) const {
	out <<	mySymbol << " "
		 <<	myUsage << " "
		 <<	myCount << " "
		 << myBranch << "\n";
	for( BYTE2 i = 0; i < myBranch; ++i ) {
		myChildren[i]->print_tree( out );
	}
}

// load the tree from the input stream.
void MHTree::load_tree(istream &in) {
	free();
	in.read( (char*)&mySymbol, sizeof( BYTE2 ) );
	in.read( (char*)&myUsage,  sizeof( BYTE4 ) );
	in.read( (char*)&myCount,  sizeof( BYTE2 ) );
	in.read( (char*)&myBranch, sizeof( BYTE2 ) );

	if( myBranch == 0 ) { return; }

	myChildren.resize( myBranch );

	for( BYTE2 i = 0; i < myBranch; ++i ) {
		myChildren[i] = new MHTree();
		myChildren[i]->load_tree( in );
	}
}
