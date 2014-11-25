/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

rewritten Mar 28, 2006 by Gerard J. Cerchio

This library is public domain software
*/
// Base64Encoder.cpp: implementation of the Base64Encoder class.
//
//////////////////////////////////////////////////////////////////////

#include "Base64Encoder.h"
#include <sstream>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const unsigned char Base64Encoder::lookupTable[65] = 

										"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz"
										"0123456789+/";

const unsigned char Base64Encoder::lookdownTable[256] = {

						0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 62,0, 0, 0, 63,52,53,
						54,55,56,57,58,59,60,61,0, 0, 
						0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
						5, 6, 7, 8, 9,10,11,12,13,14,
						15,16,17,18,19,20,21,22,23,24,
						25,0, 0, 0, 0, 0, 0, 26,27,28,
						29,30,31,32,33,34,35,36,37,38,
						39,40,41,42,43,44,45,46,47,48,
						49,50,51,0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0             
};

const int Base64Encoder::g_KsizeofBase64Buffer = 3;

Base64Encoder::Base64Encoder()
{

}

Base64Encoder::~Base64Encoder()
{

}

std::string Base64Encoder::encodeValue( void* value, unsigned long sizeofValue )
{
    std::ostringstream retVal;
    unsigned char* data = reinterpret_cast<unsigned char*>(value);

	if ( sizeofValue > 2 )
		for ( unsigned int i = 0; i < sizeofValue-2; i += 3 )
		{
			retVal << lookupTable[ *data >> 2 & 0x3f ];
			retVal << lookupTable[ ((*data << 4) & 0x3f) | ((*(data+1) >> 4) & 0xf) ]; data++;
			retVal << lookupTable[ ((*data << 2) & 0x3f) | ((*(data+1) >> 6) & 0x3) ]; data++;
			retVal << lookupTable[ *data++ & 0x3f ];
		}

	int leftOver = sizeofValue % 3;
	if ( leftOver > 0 )
	{
		retVal << lookupTable[ *data >> 2 & 0x3f ];

		if ( leftOver > 1 )
		{
			retVal << lookupTable[ ((*data << 4) & 0x3f) | ((*(data+1) >> 4) & 0xf) ];
			retVal << lookupTable[ ((*++data << 2) & 0x3f) ];
		}
		else
		{
			retVal << lookupTable[ (*data << 4) & 0x3f ];
			retVal << '=';
		}
		retVal << '=';
	}
    return retVal.str();
}



unsigned char* Base64Encoder::decodeValue( const std::string& value, unsigned long& sizeofValue )
{
    register int k = 0;
	register int strSize = value.length();
    register unsigned char*retval = new unsigned char[ sizeof(unsigned char) * strSize ];
	int i;

	union {
			unsigned char bytes[4];
			unsigned long data;
	} triad;
	triad.data = 0;

    for ( i = 0; i < strSize ; i++ )
    {
		triad.data <<= 6;
		triad.data |= lookdownTable[value[i]] ;

		if ( ( i != 0 ) && ( ( ( i + 1 ) % 4 ) == 0 ) )
		{
			retval[k++] = triad.bytes[2];
			retval[k++] = triad.bytes[1];
			retval[k++] = triad.bytes[0];
		}
    }
	i--;
	while ( value[i--] == '=' )
		k--;

	sizeofValue = k;
    return retval;
}
