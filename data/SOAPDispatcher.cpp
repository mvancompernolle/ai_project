/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software

	26-Aug-04	G. Cerchio	add SOAP output and input logging
	24-Aug-04	G. Cerchio	use long message interface to logger
	21-Aug-04	G. Cerchio	added logging for SOAP responses

	$Log: SOAPDispatcher.cpp,v $
	Revision 1.1  2005/03/22 22:22:14  gjpc
	This is the intial check in of the Simple SOAP library.
	
	The code compiles and executes under MSVC .NET and GNU 3.3
	
	It has been run under Debian, SUSE, CYGWIN and WinXP
	
	Revision 1.3  2004/04/23 16:59:26  gjpc
	expanded the Simple SOAP package to allow RPC's within RPC's
	

*/
// SOAPDispatcher.cpp: implementation of the SOAPDispatcher class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning ( disable: 4786 )
#include "SOAPDispatcher.h"
#if !defined(SOAPENCODER_H)
    #include "SOAPEncoder.h"
#endif // SOAPENCODER_H
#if !defined(SOAPOBJECT_H)
    #include "SOAPObject.h"
#endif // !defined(SOAPOBJECT_H)
#if !defined(SOAPFAULT_H)
    #include "SOAPFault.h"
#endif // !defined(SOAPFAULT_H)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SOAPDispatcher::SOAPDispatcher()
{
	currentMethodList = NULL;
}

SOAPDispatcher::~SOAPDispatcher()
{
    for (CreatorContainer::iterator it = m_creatorContainer.begin();
		it != m_creatorContainer.end(); ++it)
	{
		delete it->second;
	}
}

SOAPDispatcher& SOAPDispatcher::Instance()
{
    return Singleton<SOAPDispatcher>::Instance();
}

bool SOAPDispatcher::registerObject( SOAPObjectCreator* pCreator )
{
    // Check to see if the item is in the map.
    CreatorContainer::iterator it = m_creatorContainer.find( pCreator->createdObjectName() );
    bool retval = false;
    if ( it == m_creatorContainer.end() )
    {
        // The item was not found.  Indicate success
        // and take ownership of the memory.
        retval = true;
		m_creatorContainer.insert( CreatorContainer::value_type( 
			pCreator->createdObjectName(), pCreator ) );
    }
    return retval;
}

SOAPMethod * SOAPDispatcher::GetMethod( const char* methodName )
{
// Iterate over the list of methods
	if ( currentMethodList )
		for ( SOAPObject::MethodList::iterator it = currentMethodList->begin(); it != currentMethodList->end(); ++it )
			if ( (*it)->methodName() == methodName )
			{
				return (*it);
			}

	return NULL;
}

std::string SOAPDispatcher::processMessage( 
    const std::string& KszObjectName, 
    const std::string& KszMethodName,
    SOAPElement& theCall,
    bool& bContainsFault)
{
    std::string retval;
    SOAPEncoder theEncoder;
    SOAPFault soapFault;
    CreatorContainer::iterator it = m_creatorContainer.find( KszObjectName );

    if ( it != m_creatorContainer.end() )
    {
        std::auto_ptr<SOAPObject> pObject = std::auto_ptr<SOAPObject>( it->second->newSOAPObject() );
        if ( NULL != pObject.get() )
        {
            // Iterate over the list of methods
            SOAPObject::MethodList& methodList = pObject->getMethodList();
			currentMethodList = &methodList;
            bool bFoundMethod = false;
            for ( SOAPObject::MethodList::iterator it = methodList.begin(); it != methodList.end(); ++it )
            {
                if ( (*it)->methodName() == KszMethodName )
                {
					SOAPElement theMethod;
					bFoundMethod = true;

//					theLogger.Log( LogXML, "Dispatching SOAP Method %s", KszMethodName.c_str() );

					if	( ( !(*it)->mustIUnderstand( theCall ) ) &&
						( (*it)->extractMethod( theCall, theMethod ) ) &&
						( (*it)->execute( theMethod ) ) )
                    {
                        retval = theEncoder.encodeMethodResponse( **it );
                    }
                    else
                    {
                        retval = theEncoder.encodeFault( *((*it)->getFault()) );
                        bContainsFault = true;
                    }
                    break;
                }
            }
            if ( !bFoundMethod )
            {
                // Return SOAP error.  Class: Client.
                soapFault.setSpecificFault( "CouldNotLocateMethod", SOAPFault::Client );
                soapFault.faultString() = 
                    std::string( "Requested object exists but does not "
                    "implement the requested method: " ) +
                    KszObjectName + std::string(".") + 
                    KszMethodName;
                retval = theEncoder.encodeFault( soapFault );
                bContainsFault = true;
            }
        }
        else
        {
            // Return SOAP error.  Class: Server.
            soapFault.setSpecificFault( "CouldNotCreateObject", SOAPFault::Server );
            soapFault.faultString() = 
                "Requested object exists but could not be created.";
            retval = theEncoder.encodeFault( soapFault );
            bContainsFault = true;
        }
    }
    else
    {
        // Return object not found error.  This would be a 
        // Client fault.
        soapFault.setSpecificFault( "ObjectNotFound" );
		soapFault.faultString() = "Object: " + KszObjectName + 
			" Method: " + KszObjectName + 
            " The requested server context object does not exist on this server.";
        retval = theEncoder.encodeFault( soapFault );
        bContainsFault = true;
    }
	currentMethodList = NULL;

//	theLogger.LogOutSoap( retval.c_str() );
    return retval;
}
