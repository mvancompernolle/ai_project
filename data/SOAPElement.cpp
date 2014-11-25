/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

History:

	20-Oct-11	A. Salamon 	clear any old child elements

This library is public domain software
*/
// SOAPElement.cpp: implementation of the SOAPElement class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER 
// Microsoft only extension to the compiler
// Turns off noise about debug symbols being "too long".
    #pragma warning( disable : 4786 )
#endif // _MSC_VER 
#include <stdexcept>
#include "SOAPElement.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool splitNamespaceAndName( const std::string& KszOriginal,
    std::string& szNamespace, std::string& szAccessor )
{
    bool retval = KszOriginal.length() > 0;

    if ( retval )
    {
        long nColonPos = KszOriginal.find( ":" );
        if ( nColonPos != std::string::npos )
        {
            szNamespace = KszOriginal.substr( 0, nColonPos );
            szAccessor = KszOriginal.substr( nColonPos + 1, KszOriginal.length() - nColonPos - 1 );
        }
        else 
        {
            szNamespace = "";
            szAccessor = KszOriginal;
        }
    }

    return retval;
}

SOAPElement::SOAPElement()
{

}

SOAPElement::~SOAPElement()
{
	for(int i=0;i<numElements();i++) 
	{
		delete m_internalElements[i];
	}
}


SOAPElement::SOAPElement( const SOAPElement& rhs )
{
    *this = rhs;
}

SOAPElement& SOAPElement::operator=( const SOAPElement& rhs )
{
    m_szAccessorName = rhs.m_szAccessorName;
    m_szNamespace = rhs.m_szNamespace;
    m_szValue = rhs.m_szValue;
    m_attributes = rhs.m_attributes;

    // clear any old child elements
    m_internalElements.clear();

    //Change on 07/03/2001 by SM
    //Correct copy operator
    for(int i=0;i<rhs.numElements();i++) 
    {
	SOAPElement* pElement = new SOAPElement( *rhs.m_internalElements[i] );
	addElement( pElement );
    }

    return *this;
}

std::string& SOAPElement::accessorName()
{
    return m_szAccessorName;
}

bool SOAPElement::addAttribute( SOAPAttribute theAttribute )
{
    bool retval = true;
    m_attributes.push_back( theAttribute );
    return retval;
}

bool SOAPElement::getAttribute( const std::string& szAttributeName, SOAPAttribute& szValue )
{
    bool retval = false;

    // I don't expect most SOAP messages to have large numbers of elements. If I did,
    // I would have used a map as well as a vector.
    std::string szAccessor;
    std::string szNamespace;
    splitNamespaceAndName( szAttributeName, szNamespace, szAccessor );
    for( AttributeContainer::iterator itElement = m_attributes.begin();
         itElement != m_attributes.end();
         ++itElement )
    {
        if ( itElement->accessor() == szAttributeName )
        {
            if ( szNamespace.empty() || ( itElement->namespaceName() == szNamespace ) )
            {
                // We found the element.
                szValue = *itElement;
                retval = true;
                break;
            }
        }
    }

    return retval;
}

bool SOAPElement::addElement( SOAPElement* pElement )
{
    m_internalElements.push_back( pElement );
    return true;
}

bool SOAPElement::getElement( const std::string& szElementName,
    SOAPElement** pValue )
{
    bool retval = false;
    *pValue = NULL;

    std::string szAccessor;
    std::string szNamespace;
    splitNamespaceAndName( szElementName, szNamespace, szAccessor );

    // I don't expect most SOAP messages to have large numbers of 
    // elements. If I did, I would have used a map as well as a
    // vector.  Map for name lookup, vector for positional lookup.  
	// Probably would store the element in a vector and map 
	// accessors to indices.
    for( ElementContainer::iterator itElement = 
            m_internalElements.begin();
         itElement != m_internalElements.end();
         ++itElement )
    {
        if ( (*itElement)->accessorName() == szElementName )
        {
            if ( szNamespace.empty() || 
                 ( (*itElement)->namespaceName() == szNamespace ) )
            {
                // We found the element.
                *pValue = *itElement;
                retval = true;
                break;
            }
        }
    }

    return retval;
}

std::string& SOAPElement::value()
{
    return m_szValue;
}

long SOAPElement::numElements() const
{
    return m_internalElements.size();
}

long SOAPElement::numAttributes()
{
    return m_attributes.size();
}

SOAPElement& SOAPElement::elementAt( long index )
{
    if ( ( numElements() <= index ) || ( 0 > index ) )
    {
        throw std::out_of_range( std::string( "Invalid index passed to SOAPElement::elementAt" ) );
    }
    return *m_internalElements[index];
}

SOAPAttribute& SOAPElement::attributeAt( long index )
{
    if ( ( numAttributes() <= index ) || ( 0 > index ) )
    {
        throw std::out_of_range( std::string( "Invalid index passed to SOAPElement::attributeAt" ) );
    }
    return m_attributes[index];
}

std::string& SOAPElement::namespaceName()
{
    return m_szNamespace;
}

void SOAPElement::serialize(std::ostringstream& stream)
{
	serialize(stream, *this);
}

void SOAPElement::serialize(std::ostringstream& stream, SOAPElement& element)
{
	// encode the element name

	stream << "<";
	if (element.m_szNamespace.size() > 0) {
		stream << element.m_szNamespace << ":";
	}
	stream << element.m_szAccessorName << " ";

	// encode the element's attributes
    for (AttributeContainer::iterator itElement = m_attributes.begin(); itElement != m_attributes.end(); ++itElement) {
		if (itElement->namespaceName().size() > 0) {
			stream << itElement->namespaceName() << ":";
		}
		stream << itElement->accessor() << "=" << "\"" << itElement->value() << "\" ";
    }

	// terminate the element declaration
	stream << ">";

	// encode the element's children OR the value
	if (element.m_internalElements.size() > 0) {
		stream << std::endl;
	    for (ElementContainer::iterator itElement = m_internalElements.begin(); 
				itElement != m_internalElements.end(); 
				++itElement) {
			SOAPElement * e = *itElement;
			e->serialize(stream);
		}
	} else {
		stream << element.value();
	}

	// terminate the element
	stream << "</";
	if (element.m_szNamespace.size() > 0) {
		stream << element.m_szNamespace << ":";
	}
	stream << element.m_szAccessorName << ">";
}


