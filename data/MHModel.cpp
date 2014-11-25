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
#include "MHModel.h"

void MHModel::new_model( BYTE1 newOrder, const string &newName ) {
	myName = newName.size() ? newName : "MegaHAL";
	myOrder = newOrder;
	myForward = new MHTree();
	myBackward = new MHTree();
	myContext.resize( myOrder + 2 );
	mySwap = NULL;
	myGreets = NULL;
	myAux = NULL;
	myBan = NULL;
	setSwap( new MHSwap() );
	setGreets( new MHDictionary() );
	setAux( new MHDictionary() );
	setBan( new MHDictionary() );
	initialize_context();
	myVocabulary = new MHDictionary();
	myVocabulary->add_word( "<ERROR>" );
	myVocabulary->add_word( "<FIN>" );
}

void MHModel::free() {
	if( myForward != NULL ) {
		delete myForward;
		myForward = NULL;
	}
	if( myBackward != NULL ) {
		delete myBackward;
		myBackward = NULL;
	}
	if( myVocabulary != NULL ) {
		delete myVocabulary;
		myVocabulary = NULL;
	}
	setSwap( NULL );
	setGreets( NULL );
	setAux( NULL );
	setBan( NULL );
	myContext.resize( 0 );
}


void MHModel::update_model( BYTE2 symbol ) {
	/*
	 *		Update all of the models in the current context with the specified
	 *		symbol.
	 */
	for( BYTE1 i = (myOrder+1); i > 0; --i ) {
		if( myContext[i-1] != NULL ) {
			myContext[i] = myContext[i-1]->add_symbol( symbol );
		}
	}
}

void MHModel::initialize_context() {
	for( BYTE1 i = 0; i <= myOrder; ++i ) {
		myContext[i] = NULL;
	}
}

void MHModel::update_context( BYTE2 symbol ) {
	register int i;
	/*
	 *		Update all of the models in the current context without
	 *		adding the specified symbol.
	 */
	for( i = (myOrder+1); i > 0; --i) {
		if( myContext[i-1] != NULL ) {
			myContext[i] = myContext[i-1]->find_symbol( symbol );
		}
	}
}

void MHModel::learn(MHDictionary *phrase) {
	BYTE2 i, symbol;

	//	We only learn from inputs which are long enough
	if( phrase->size() <= myOrder ) {
		return;
	}

	/*
	 *		Train the model in the forwards direction.  Start by initializing
	 *		the context of the model.
	 */
	// DEBUG
	//myVocabulary->show_dictionary("m:\\megahavcc\\TESTDICT.TXT");

	initialize_context();
	myContext[0] = myForward;
	for( i=0; i < phrase->size(); ++i ) {
		/*
		 *		Add the symbol to the model's dictionary if necessary, and then
		 *		update the forward model accordingly.
		 */
		symbol = myVocabulary->add_word( phrase->entry(i) );
		update_model( symbol );
	}
	//	Add the sentence-terminating symbol.
	update_model( 1 );

	/*
	 *		Train the model in the backwards direction.  Start by initializing
	 *		the context of the model.
	 */
	initialize_context();
	myContext[0] = myBackward;
	for( i = phrase->size(); i-- > 0; ) {
		/*
		 *		Find the symbol in the model's dictionary, and then update
		 *		the backward model accordingly.
		 */
		symbol = myVocabulary->find_word( phrase->entry(i) );
		update_model( symbol );
	}
	//	Add the sentence-terminating symbol.
	update_model( 1 );
}

MHDictionary *MHModel::make_greeting() {
	MHDictionary *newGreeting = new MHDictionary();
	if( (myGreets != NULL) && (myGreets->size() != 0) ) {
		newGreeting->push_word( myGreets->entry( (BYTE2)MHFuncs::rnd( myGreets->size() ) ) );
	}
	return( newGreeting );
}

/*
 *		Function:	Make_Keywords
 *
 *		Purpose:		Put all the interesting words from the user's input into
 *						a keywords dictionary, which will be used when generating
 *						a reply.
 */
MHDictionary *MHModel::make_keywords( MHDictionary *phrase ) {
	BYTE2 c;
	MHDictionary *keys = new MHDictionary();
	for( BYTE2 i = 0; i < phrase->size(); ++i ) {
		c = 0;
		for( BYTE2 j = 0; j < mySwap->size(); ++j ) {
			if( mySwap->from(j) == phrase->entry(i) ) {
				add_key( keys, mySwap->to(j) );
				++c;
			}
		}
		if( c == 0 ) { add_key( keys, phrase->entry(i) ); }
	}
	if( keys->size() > 0 ) {
		for( BYTE2 i = 0; i < phrase->size(); ++i ) {
			c = 0;
			for( BYTE2 j = 0; j < mySwap->size(); ++j ) {
				if( mySwap->from(j) == phrase->entry(i) ) {
					add_aux( keys, mySwap->to(j) );
					++c;
				}
			}
			if( c == 0 ) { add_aux( keys, phrase->entry(i) ); }
		}
	}
	return(keys);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Key
 *
 *		Purpose:		Add a word to the keyword dictionary.
 */
void MHModel::add_key( MHDictionary *keys, const string &theWord ) {
	BYTE2 symbol = myVocabulary->find_word( theWord );
	if( symbol == 0 ) { return; }
	if( isalnum(theWord[0]) == 0 ) { return; }
	symbol = myBan->find_word( theWord );
	if( symbol != 0 ) { return; }
	symbol = myAux->find_word( theWord );
	if( symbol != 0 ) { return; }
	keys->add_word( theWord );
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Aux
 *
 *		Purpose:		Add an auxilliary keyword to the keyword dictionary.
 */
void MHModel::add_aux( MHDictionary *keys, const string &theWord ) {
	BYTE2 symbol = myVocabulary->find_word( theWord );
	if( symbol == 0 ) { return; }
	if( isalnum(theWord[0]) == 0 ) { return; }
	symbol = myAux->find_word( theWord );
	if( symbol == 0 ) { return; }
	keys->add_word( theWord );
}

/*---------------------------------------------------------------------------*/ 
/*
 *    Function:   Generate_Reply
 *
 *    Purpose:    Take a string of user input and return a string of output
 *                which may vaguely be construed as containing a reply to
 *                whatever is in the input string.
 */
string MHModel::generate_reply( MHDictionary *phrase ) {
	//	Create an array of keywords from the words in the user's input
	MHDictionary *keywords = make_keywords( phrase );
	MHDictionary *dummy = new MHDictionary();
	string output;
	float surprise;
	float max_surprise = (float)-1.0;
	size_t count = 0;
	size_t basetime;

	MHDictionary *replyphrase = reply( dummy );
	MHDictionary *chosenphrase = NULL;
	if( phrase->dissimilar(*replyphrase) ) {
		chosenphrase = replyphrase;
		replyphrase = NULL;
	}
	basetime = time(NULL);

#ifdef MHDEBUG_UNTIMED_REPLY
	int numloops = 0;
#endif

	do {
		replyphrase = reply( keywords );
		surprise = evaluate_reply( keywords, replyphrase );
		++count;
		if( surprise > max_surprise && phrase->dissimilar(*replyphrase) ) {
			if( chosenphrase != NULL ) {
				delete chosenphrase;
			}
			chosenphrase = replyphrase;
			replyphrase = NULL;
			max_surprise = surprise;
		}
		else {
			delete replyphrase;
			replyphrase = NULL;
		}
#ifdef MHDEBUG_UNTIMED_REPLY
	} while( numloops++ < MHDEBUG_TIMEOUT );
#else
	} while( time(NULL) - basetime < MHTIMEOUT );
#endif
	if( chosenphrase == NULL ) {
		output = "I don't know enough to answer you yet!";
	}
	else {
		output = chosenphrase->make_output();
		delete chosenphrase;
		if( replyphrase != NULL ) {
			delete replyphrase;
		}
	}
	delete keywords; keywords = NULL;
	delete dummy; dummy = NULL;
	return(output);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Reply
 *
 *		Purpose:		Generate a dictionary of reply words appropriate to the
 *						given dictionary of keywords.
 */
MHDictionary *MHModel::reply( MHDictionary *keys ) {
	MHDictionary *replyWords = new MHDictionary();
	initialize_context();
	myContext[0] = myForward;
	bool usedKey = false;
	BYTE2 symbol = seed( keys );

	while( symbol > 1 ) {
		replyWords->push_word( myVocabulary->entry(symbol) );
		update_context( symbol );
		symbol = babble( keys, replyWords, usedKey );
	}

	initialize_context();
	myContext[0] = myBackward;
	if( replyWords->size() > 0 ) {
		BYTE2 i = 1;
		for( i += MIN( replyWords->size() - 1, myOrder ); i-- > 0; ) {
			symbol = myVocabulary->find_word( replyWords->entry(i) );
			update_context( symbol );
		}
	}

	while( (symbol=babble(keys,replyWords,usedKey)) > 1 ) {
		replyWords->insert_word( 0, myVocabulary->entry(symbol) );
		update_context( symbol );
	}
	return( replyWords );
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Evaluate_Reply
 *
 *		Purpose:		Measure the average surprise of keywords relative to the
 *						language model.
 */
float MHModel::evaluate_reply( MHDictionary *keys, MHDictionary *words ) {
	if( words->size() < 1 ) { return( (float)0.0 ); }
	BYTE2 symbol, num;
	symbol = num = 0;
	BYTE1 count;
	MHTree *node = NULL;
	float probability = (float)0.0;
	float entropy = (float)0.0;
	initialize_context();
	myContext[0] = myForward;

	for( BYTE2 i = 0; i < words->size(); ++i ) {
		symbol = myVocabulary->find_word( words->entry(i) );
		if( keys->find_word( words->entry(i) ) != 0 ) {
			probability = (float)0.0;
			count = 0;
			++num;
			for( BYTE1 j = 0; j < myOrder; ++j ) {
				if( myContext[j] != NULL ) {
					node = myContext[j]->find_symbol( symbol );
					probability += (float)(node->count()) / (float)(myContext[j]->usage());
					++count;
				}
			}
			if( count > 0 ) {
				entropy -= (float)log(probability/(float)count);
			}
		}
		update_context( symbol );
	}

	initialize_context();
	myContext[0] = myBackward;
	for( BYTE2 i = words->size(); i-- > 0; ) {
		symbol = myVocabulary->find_word( words->entry(i) );
		if( keys->find_word( words->entry(i) ) != 0 ) {
			probability = (float)0.0;
			count = 0;
			++num;
			for( BYTE1 j = 0; j < myOrder; ++j ) {
				if( myContext[j] != NULL ) {
					node = myContext[j]->find_symbol( symbol );
					probability += (float)(node->count()) / (float)(myContext[j]->usage());
					++count;
				}
			}
			if( count > 0 ) {
				entropy -= (float)log(probability/(float)count);
			}
		}
		update_context( symbol );
	}

	if( num > 7 ) { entropy /= (float)sqrt((float)(num-1)); }
	if( num > 15 ) { entropy /= (float)num; }
	return(entropy);
}

/*
 *		Function:	Babble
 *
 *		Purpose:		Return a random symbol from the current context, or a
 *						zero symbol identifier if we've reached either the
 *						start or end of the sentence.  Select the symbol based
 *						on probabilities, favouring keywords.  In all cases,
 *						use the longest available context to choose the symbol.
 */
BYTE2 MHModel::babble( MHDictionary *keys, MHDictionary *words, bool &usedKey )
{
	MHTree *node = NULL;
	BYTE2 randomIndex, symbol;
	BYTE4 count;
	for( BYTE1 i = 0; i <= myOrder; ++i ) {
		if( myContext[i] != NULL ) {
			node = myContext[i];
		}
	}

	if( node->branch() == 0 ) { return 0; }

	randomIndex = (BYTE2)MHFuncs::rnd( node->branch() );
	count = (BYTE2)MHFuncs::rnd( node->usage() );

	while( true ) {
		symbol = node->child(randomIndex)->symbol();
		//DEBUG
		/*
		if( symbol == 201 ) {
			int abba = 0;
		}
		BYTE2 t1 = keys->find_word( myVocabulary->entry(symbol) );
		BYTE2 t2 = usedKey;
		BYTE2 t3 = myAux->find_word( myVocabulary->entry(symbol) );
		BYTE2 t4 = words->word_exists( myVocabulary->entry(symbol) );
		if( symbol == 201 ) {
			int abba = 0;
			cout << "WORD IN BABBLE: " << myVocabulary->entry(symbol) << endl;
			cout << "reply so far: ";
			words->print_dictionary();
		}
		*/
		if(
			( keys->find_word( myVocabulary->entry(symbol) ) != 0 )			&&
			(	( usedKey == true )											||
				( myAux->find_word( myVocabulary->entry(symbol) ) == 0 ) )	&&
			( words->word_exists( myVocabulary->entry(symbol) ) == false )
		) {
			usedKey = true;
			break;
		}
		if( count < node->child(randomIndex)->count() ) {
			break;
		}
		count -= node->child(randomIndex)->count();
		randomIndex = ( randomIndex >= (node->branch()-1) ) ? 0 : randomIndex + 1;
	}
	return( symbol );
}

/*
 *		Function:	Seed
 *
 *		Purpose:		Seed the reply by guaranteeing that it contains a
 *						keyword, if one exists.
 */
BYTE2 MHModel::seed( MHDictionary *keys ) {
	BYTE2 symbol, i, stop;
	if( myContext[0] == NULL || myContext[0]->branch() == 0 ) {
		symbol = 0;
	}
	else {
		symbol = myContext[0]->child( (BYTE2)MHFuncs::rnd(myContext[0]->branch()) )->symbol();
	}
	if( keys->size() > 0 ) {
		i = (BYTE2)MHFuncs::rnd( keys->size() );
		stop = i;
		while( true ) {
			if( ( myVocabulary->find_word( keys->entry(i) ) != 0 ) &&
				( myAux->find_word( keys->entry(i) ) == 0 ) ) {
					symbol = myVocabulary->find_word( keys->entry(i) );
					return( symbol );
			}
			++i;
			if( i == keys->size() ) { i = 0; }
			if( i == stop ) { return( symbol ); }
		}
	}
	return( symbol );
}


int MHModel::train( istream &in ) {
	char buffer[4096]; // buffer to use when reading fp_in
	MHDictionary *phrase = NULL;
	string bufstr;

	while( in.getline( buffer, 4096 ) ) {
		if( buffer[0] == '#' ) { continue; }
		bufstr = buffer;
		MHFuncs::upper(bufstr);
		phrase = new MHDictionary( bufstr );
		learn( phrase );
		delete phrase;
		phrase = NULL;
	}
	return( 0 );
}

void MHModel::setAux( MHDictionary *newAux ) {
	if( myAux != NULL ) {
		*(myAux->numberOfPointersToMe) -= 1;
		if( *(myAux->numberOfPointersToMe) == 0 ) {
			delete myAux; myAux = NULL;
			cout << "Deleting old Aux database." << endl;
		}
	}
	if( newAux != NULL ) {
		*(newAux->numberOfPointersToMe) += 1;
	}
	myAux = newAux;
}

void MHModel::setGreets( MHDictionary *newGreets ) {
	if( myGreets != NULL ) {
		*(myGreets->numberOfPointersToMe) -= 1;
		if( *(myGreets->numberOfPointersToMe) == 0 ) {
			delete myGreets; myGreets = NULL;
			cout << "Deleting old Grt database." << endl;
		}
	}
	if( newGreets != NULL ) {
		*(newGreets->numberOfPointersToMe) += 1;
	}
	myGreets = newGreets;
}

void MHModel::setBan( MHDictionary *newBan ) {
	if( myBan != NULL ) {
		*(myBan->numberOfPointersToMe) -= 1;
		if( *(myBan->numberOfPointersToMe) == 0 ) {
			delete myBan; myBan = NULL;
			cout << "Deleting old Ban database." << endl;
		}
	}
	if( newBan != NULL ) {
		*(newBan->numberOfPointersToMe) += 1;
	}
	myBan = newBan;
}

void MHModel::setSwap( MHSwap *newSwap ) {
	if( mySwap != NULL ) {
		*(mySwap->numberOfPointersToMe) -= 1;
		if( *(mySwap->numberOfPointersToMe) == 0 ) {
			delete mySwap; mySwap = NULL;
			cout << "Deleting old Swap database." << endl;
		}
	}
	if( newSwap != NULL ) {
		*(newSwap->numberOfPointersToMe) += 1;
	}
	mySwap = newSwap;
}

/*
 *		Function:	Save_Model
 *
 *		Purpose:		Save the current state to a MegaHAL brain file.
 */
bool MHModel::save_model( const string &fileName ) {
	ofstream outfile;
	bool status;
	if( fileName.size() != 0 ) {
		myBrainFile = fileName;
	}
	if( myBrainFile.size() == 0 ) {
		return( false );
	}
	outfile.open( myBrainFile.c_str(), ios::binary | ios::out );
	status = save_model( outfile );
	outfile.close();
	return( status );
}

bool MHModel::save_model(ostream &out) const {
	out << MHFuncs::cookie();
	out.write( (char*)&myOrder, 1 );
	myForward->save_tree( out );
	myBackward->save_tree( out );
	myVocabulary->save_dictionary( out );
	return( true );
}


/*
 *		Function:	Load_Model
 *
 *		Purpose:		Load a model into memory.
 */
bool MHModel::load_model( const string &fileName ) {
	ifstream infile;
	bool status;
	if( fileName.size() != 0 ) {
		myBrainFile = fileName;
	}
	if( myBrainFile.size() == 0 ) {
		return( false );
	}
	infile.open( myBrainFile.c_str(), ios::binary | ios::in );
	status = load_model( infile );
	infile.close();
	return( status );
}

bool MHModel::load_model( istream &in ) {
	char aCookie[16];
	for( BYTE1 i = 0; i < 16; ++i ) {
		aCookie[i] = '\0';
	}
	in.read(aCookie, sizeof(char) * (BYTE1)MHFuncs::cookie().size() );
	if( MHFuncs::cookie() != aCookie ) {
		return( false );
	}
	in.read( (char*)&myOrder, 1 );
	myForward->load_tree(in);
	myBackward->load_tree(in);
	myVocabulary->load_dictionary(in);
	return( true );
}

int MHModel::load_personality( const string &personalityPath ) {
	string myPath = personalityPath;
	if( myPath.size() != 0 ) {
		myPath.append( SEP );
	}
	string	brnPath, trnPath, errPath, swpPath,
			banPath, auxPath, grtPath, dicPath,
			logPath, txtPath;
	
	brnPath = trnPath = errPath = swpPath
			= banPath = auxPath = grtPath = dicPath
			= logPath = txtPath
			= myPath;

	brnPath.append( "megahal.brn" );
	trnPath.append( "megahal.trn" );
	grtPath.append( "megahal.grt" );
	banPath.append( "megahal.ban" );
	auxPath.append( "megahal.aux" );
	swpPath.append( "megahal.swp" );
	dicPath.append( "megahal.dic" );
	logPath.append( "megahal.log" );
	txtPath.append( "megahal.txt" );
	
	cout << "Loading " << brnPath << endl;
	ifstream brnfile;
	brnfile.open( brnPath.c_str(), ios::binary | ios::in );
	if( brnfile ) {
		load_model( brnfile );
		brnfile.close();
	}
	else {
		cout << brnPath << " not found. Loading " << trnPath << endl;
		ifstream trnfile;
		trnfile.open( trnPath.c_str(), ios::in );
		if( trnfile ) {
			train( trnfile );
		}
		else {
			cout << trnPath << " not found either. Starting fresh." << endl;
			return( 1 );
		}
	}
	initialize_context();

	myVocabOutputFile = dicPath;
	myErrorLogFile = logPath;
	myConversationLogFile = txtPath;
	myBrainFile = brnPath;
	
	if( myGreets != NULL ) {
		setGreets( new MHDictionary() );
		myGreets->initialize_list( grtPath );
	}
	if( myBan != NULL ) {
		setBan( new MHDictionary() );
		myBan->initialize_list( banPath );
	}
	if( mySwap != NULL ) {
		setSwap( new MHSwap() );
		mySwap->initialize_swap( swpPath );
	}
	if( myAux != NULL ) {
		setAux( new MHDictionary() );
		myAux->initialize_list( auxPath );
	}

	return( 0 );
}
