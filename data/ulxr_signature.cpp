/***************************************************************************
         ulxr_signature.cpp  -  handle method signatures
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_signature.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_value.h>

namespace ulxr {


ULXR_API_IMPL0 Signature::Signature()
{
}


ULXR_API_IMPL0 Signature::Signature(const Void & /* v */)
{
}


ULXR_API_IMPL0 Signature::Signature(const CppString &s)
{
  sig = s;
}


ULXR_API_IMPL0 Signature::Signature(const ValueBase &v)
{
  sig = v.getSignature();
}


ULXR_API_IMPL(Signature) & Signature::addParam(const Value &v)
{
  if (sig.length() != 0)
    sig += ULXR_PCHAR(",");
  sig += v.getSignature();
  return *this;
}


ULXR_API_IMPL(Signature) & Signature::addParam(const CppString &s)
{
  if (sig.length() != 0)
    sig += ULXR_PCHAR(",");
  sig += s;
  return *this;
}


ULXR_API_IMPL(CppString) Signature::getString() const
{
  return sig;
}


ULXR_API_IMPL(Signature) & Signature::operator<<(const Value &v)
{
  addParam(v);
  return *this;
}


ULXR_API_IMPL(Signature) & Signature::operator<<(const CppString &s)
{
  addParam(s);
  return *this;
}

}  // namespace ulxr
