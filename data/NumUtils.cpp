//==============================================================================
// This file is part of the OpenTop C++ Library.
// (c) Copyright 2000-2002 ElCel Technology Limited
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding valid OpenTop Commercial licences may use this file
// in accordance with the OpenTop Commercial License Agreement included
// with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.elcel.com/products/opentop/pricing.html or email
// sales@elcel.com for information about OpenTop Commercial License Agreements.
//
//==============================================================================
//
// $Revision: 1.1.1.1 $
// $Date: 2002/12/14 00:24:39 $
//
//==============================================================================
/**
	@class ot::NumUtils
	
	@brief Class module containing functions to convert numbers into Strings
	       and vice versa.

	@sa StringUtils
*/
//==============================================================================

#include "NumUtils.h"
#include "Exception.h"
#include "StringUtils.h"

#include <stdio.h>
#include <stdlib.h>

OT_BASE_NAMESPACE_BEGIN

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the long value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(long x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%ld", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the unsigned long value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(unsigned long x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%lu", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the integer value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(int x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%d", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the unsigned integer value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(unsigned int x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%u", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the double-precision floating-point value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(double d)
{
	return StringUtils::FromLatin1(StringUtils::Format("%f", d));
}

//==============================================================================
// NumUtils::ToInt
//
/**
   Converts a String into an integer value.

   @param str the String to convert
   @param base the number base to use for the conversion (e.g. 10 for decimal,
          16 for hexadecimal)
   @returns the String converted into an integer.  @c 0 is returned if the
            String cannot be converted.
*/
//==============================================================================
int NumUtils::ToInt(const String& str, int base)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return (int)strtol(ascii.c_str(), 0, base);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0;
}

//=============================================================================
// NumUtils::ToLong
//
/**
   Converts a String into a long integer value.

   @param str the String to convert
   @param base the number base to use for the conversion (e.g. 10 for decimal,
          16 for hexadecimal)
   @returns the String converted into a long integer.  @c 0 is returned if the
            String cannot be converted.
*/
//=============================================================================
long NumUtils::ToLong(const String& str, int base)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return strtol(ascii.c_str(), 0, base);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0L;
}

//=============================================================================
// NumUtils::ToDouble
//
/**
   Converts a String into a double-precision floating-point value.

   @param str the String to convert
   @returns the String converted into an double.  @c 0.0 is returned if the
            String cannot be converted.
*/
//=============================================================================
double NumUtils::ToDouble(const String& str)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return ::strtod(ascii.c_str(), 0);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0.0;
}

OT_BASE_NAMESPACE_END
