/******************************************************************
*
*	MediaServer for CyberLink
*
*	Copyright (C) Satoshi Konno 2003
*
*	File : MediaServer.cpp
*
*	04/03/04
*		- first revision.
*	05/19/04
*		- Changed the header include order for Cygwin.
*
******************************************************************/

#include <cybergarage/upnp/media/server/MediaServer.h>

#include <string>

using namespace std;
using namespace CyberLink;
using namespace CyberNet;

////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////
	
const char *MediaServer::DEVICE_TYPE = "urn:schemas-upnp-org:device:MediaServer:1";

const int MediaServer::DEFAULT_HTTP_PORT = 38520;
	
////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////
	
const char *MediaServer::DESCRIPTION_FILE_NAME = "description/description.xml";
	
MediaServer::MediaServer(const char *descriptionFileName) : Device(descriptionFileName)
{
	// Netwroking initialization		
	UPnP::SetEnable(UPnP::USE_ONLY_IPV4_ADDR);
	string firstIf;
	GetHostAddress(0, firstIf);
	setInterfaceAddress(firstIf.c_str());
	setHTTPPort(DEFAULT_HTTP_PORT);
		
	conMan = new ConnectionManager(this);
	conDir = new ContentDirectory(this);
		
	Service *servConMan = getService(ConnectionManager::SERVICE_TYPE);
	servConMan->setActionListener(getConnectionManager());
	servConMan->setQueryListener(getConnectionManager());

	Service *servConDir = getService(ContentDirectory::SERVICE_TYPE);
	servConDir->setActionListener(getContentDirectory());
	servConDir->setQueryListener(getContentDirectory());
}

MediaServer::~MediaServer()
{
	delete conMan;
	delete conDir;
	stop();		
}

////////////////////////////////////////////////
// HttpRequestListner (Overridded)
////////////////////////////////////////////////
	
void MediaServer::httpRequestRecieved(CyberHTTP::HTTPRequest *httpReq)
{
	string uri;
	httpReq->getURI(uri);
	
	if (uri.find(ContentDirectory::CONTENT_EXPORT_URI) != string::npos) {
		getContentDirectory()->contentExportRequestRecieved(httpReq);
		return;
	}
			 
	Device::httpRequestRecieved(httpReq);
}
