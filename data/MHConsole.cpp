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
#include "MHConsole.h"

MHConsole::MHConsole() {
}

string MHConsole::commandArgs( const string &wholeCommand ) const {
	BYTE1 i;
	BYTE1 cmdSize = (BYTE1)wholeCommand.size();
	if( cmdSize == 0 || wholeCommand[0] != '#' ) {
		return( string("") );
	}
	for( i = 1; i < cmdSize; ++i ) {
		if( wholeCommand[i] == ' ' || wholeCommand[i] == '\t' ) {
			break;
		}
	}
	for( ; i < cmdSize; ++i ) {
		if( wholeCommand[i] != ' ' && wholeCommand[i] != '\t' ) {
			break;
		}
	}
	return( wholeCommand.substr( i, cmdSize - i ) );
}

void MHConsole::main_loop() {
	string buffer, user_input, mhal_reply;
	MHModel *mhal = new MHModel();
	mhal->load_personality( "" );
	MHDictionary *input_words = NULL;
	bool typingDelay = false;
	cout
<< "+------------------------------------------------------------------------+\n"
<< "|                                                                        |\n"
<< "|  #    #  ######   ####     ##    #    #    ##    #                     |\n"
<< "|  ##  ##  #       #    #   #  #   #    #   #  #   #               ###   |\n"
<< "|  # ## #  #####   #       #    #  ######  #    #  #              #   #  |\n"
<< "|  #    #  #       #  ###  ######  #    #  ######  #       #   #   ####  |\n"
<< "|  #    #  #       #    #  #    #  #    #  #    #  #        # #       #  |\n"
<< "|  #    #  ######   ####   #    #  #    #  #    #  ######    #     ###r1 |\n"
<< "|                                                                        |\n"
<< "|           MegaHAL in C: Copyright(C) 1998 Jason Hutchens               |\n"
<< "|         MegaHAL in C++: Copyright(C) 2002 plushpuffin@yahoo.com        |\n"
<< "+------------------------------------------------------------------------+\n";

	input_words = mhal->make_greeting();
	mhal_reply = mhal->generate_reply( input_words );
	MHFuncs::middle(mhal_reply);
	cout << "MegaHAL: " << mhal_reply << endl << endl;

	while( true ) {
		if( input_words != NULL ) {
			delete input_words;
			input_words = NULL;
		}
		user_input = "";
		cout << "> ";
		getline( cin, buffer );
		while( buffer.size() != 0 ) {
			user_input.append( buffer );
			cout << "+ ";
			getline( cin, buffer );
			if( buffer.size() != 0 ) {
				buffer.insert( 0, " " );
			}
		}
		input_words = new MHDictionary( user_input );
		if( input_words->size() == 0 ) {
			cout << "MegaHAL doesn't respond well to the silent treatment.\n\n";
			continue;
		}
		if( input_words->entry(0)[0] == '#' ) {
			buffer = input_words->entry(1);
			delete input_words; input_words = NULL;
			cout << "command: " << buffer << endl;
			MHFuncs::upper(buffer);
			if( buffer == "EXIT" || buffer == "ABORT" ) {
				break;
			}
			else if( buffer == "QUIT" ) {
				mhal->save_model( "" );
				break;
			}
			else if( buffer == "SAVE" ) {
				mhal->save_model( commandArgs( user_input ) );
			}
			else if( buffer == "BRAIN" ) {
				buffer = commandArgs( user_input );
				cout << "Attempting to open personality " << buffer << "." << endl;
				// Create a new MegaHAL model
				// Load all new everything.
				MHModel *newmhal = new MHModel(buffer);
				if( ! newmhal->load_personality( buffer ) ) {
					delete mhal; mhal = NULL;
					mhal = newmhal;
				}
				else {
					cout << "#BRAIN command failure!" << endl;
					delete newmhal; newmhal = NULL;
				}
				input_words = mhal->make_greeting();
				mhal_reply = mhal->generate_reply( input_words );
				MHFuncs::middle(mhal_reply);
				cout << "MegaHAL: " << mhal_reply << endl << endl;
			}
			else if( buffer == "BRAWN" ) {
				buffer = commandArgs( user_input );
				cout << "Attempting to open only the brain file in " << buffer << "." << endl;
				// Create a new MegaHAL model
				// Share the old model's AUX/GREETS/BAN/SWAP list
				MHModel *newmhal = new MHModel(buffer);
				newmhal->setAux( NULL );
				newmhal->setGreets( NULL );
				newmhal->setBan( NULL );
				newmhal->setSwap( NULL );
				if( ! newmhal->load_personality( buffer ) ) {
					newmhal->setAux( mhal->getAux() );
					newmhal->setGreets( mhal->getGreets() );
					newmhal->setBan( mhal->getBan() );
					newmhal->setSwap( mhal->getSwap() );
					delete mhal; mhal = NULL;
					mhal = newmhal;
				}
				else {
					cout << "#BRAWN command failure!" << endl;
					delete newmhal; newmhal = NULL;
				}
				input_words = mhal->make_greeting();
				mhal_reply = mhal->generate_reply( input_words );
				MHFuncs::middle(mhal_reply);
				cout << "MegaHAL: " << mhal_reply << endl << endl;
			}
			else if( buffer == "DELAY" ) {
				typingDelay = ! typingDelay;
				cout << "MegaHAL: TYPING DELAY IS NOW " << ( typingDelay ? "ON" : "OFF" ) << ".\n";
			}
			else {
				cout << endl
<< "#QUIT   : quits the program and saves MegaHAL's brain." << endl
<< "#EXIT   : exits the program *without* saving MegaHAL's brain." << endl
<< "#SAVE   : saves the current MegaHAL brain." << endl
<< "#BRAIN  : loads an entire personality from the specified directory." << endl
<< "#BRAWN  : loads a megahal.brn file or megahal.trn file from the specified" << endl
<< "          directory. Uses the currently loaded AUX/BAN/GRT/SWP databases." << endl
<< "#DELAY  : toggles MegaHAL's typing delay (off by default)." << endl
<< "#HELP   : displays this message." << endl << endl;
			}
			continue;
		}
		MHFuncs::upper( user_input );
		mhal->learn( input_words );
		mhal_reply = mhal->generate_reply( input_words );
		MHFuncs::middle(mhal_reply);
		cout << endl << "MegaHAL: " << mhal_reply << endl << endl;
	}
	cout << endl << "Goodbye." << endl;
	if( input_words != NULL ) {
		delete input_words;
		input_words = NULL;
	}
	if( mhal != NULL ) {
		delete mhal;
		mhal = NULL;
	}
}
