/******************************************************************
*
*	CyberLink for C++
*
*	Copyright (C) Satoshi Konno 2002-2003
*
*	File: Device.cpp
*
*	Revision;
*
*	08/13/03
*		- first revision
*	10/21/03
*		- Updated a udn field by a original uuid.
*	10/22/03
*		- Added setActionListener().
*		- Added setQueryListener().
*	12/25/03
*		- Added advertiser functions.
*	01/05/04
*		- Added isExpired().
*	03/23/04
*		- Oliver Newell <newell@media-rush.com>
*		- Changed to update the UDN only when the field is null.
*	04/21/04
*		- Added isDeviceType().
*	05/19/04
*		- Changed the header include order for Cygwin.
*	06/18/04
*		- Added setNMPRMode() and isNMPRMode().
*		- Changed getDescriptionData() to update only when the NMPR mode is false.
*	06/21/04
*		- Changed start() to send a bye-bye before the announce.
*		- Changed annouce(), byebye() and deviceSearchReceived() to send the SSDP
*		  messsage four times when the NMPR and the Wireless mode are true.
*	07/02/04
*		- Fixed announce() and byebye() to send the upnp::rootdevice message despite embedded devices.
*		- Fixed getRootNode() to return the root node when the device is embedded.
*	07/24/04
*		- Thanks for Stefano Lenzi <kismet-sl@users.sourceforge.net>
*		- Added getParentDevice().
*	10/20/04 
*		- Brent Hills <bhills@openshores.com>
*		- Changed postSearchResponse() to add MYNAME header.
*	11/19/04
*		- Theo Beisch <theo.beisch@gmx.de>
*		- Added getStateVariable(String serviceType, String name).
*	03/23/05
*		- Changed httpPostRequestRecieved() to return the bad request when the post request isn't the soap action.
*	03/30/05
*		- Added a  exclusive access control to Device::getDescriptionData().
*		- Added Devuce::getServiceBySCPDURL().
*	03/31/05
*		- Changed httpGetRequestRecieved() to return the description stream using
*		  Device::getDescriptionData() and Service::getSCPDData() at first.
*	04/25/05
*		- Thanks for Mikael Hakman <mhakman@dkab.net>
*		- Changed announce() and byebye() to close the socket after the posting.
*	04/25/05
*		- Thanks for Mikael Hakman <mhakman@dkab.net>
*		- Changed deviceSearchResponse() answer with USN:UDN::<device-type> when request ST is device type.
* 	04/25/05
*		- Thanks for Mikael Hakman <mhakman@dkab.net>
* 		- Changed getDescriptionData() to add a XML declaration at first line.
* 	04/25/05
*		- Thanks for Mikael Hakman <mhakman@dkab.net>
*		- Added a new setActionListener() and serQueryListner() to include the sub devices. 
*	07/24/05
*		- Thanks for Stefano Lenzi <kismet-sl@users.sourceforge.net>
*		- Fixed a bug of getParentDevice() to return the parent device normally.
*
******************************************************************/


#include <cybergarage/net/HostInterface.h>
#include <cybergarage/net/URL.h>
#include <cybergarage/util/Debug.h>
#include <cybergarage/util/TimeUtil.h>
#include <cybergarage/http/HTTPDate.h>
#include <cybergarage/io/File.h>
#include <cybergarage/upnp/Device.h>
#include <cybergarage/upnp/Service.h>
#include <cybergarage/upnp/UPnPStatus.h>
#include <cybergarage/upnp/device/ST.h>
#include <cybergarage/upnp/ssdp/SSDPNotifySocket.h>
#include <cybergarage/upnp/ssdp/SSDPNotifyRequest.h>
#include <cybergarage/upnp/ssdp/SSDPSearchResponse.h>
#include <cybergarage/upnp/ssdp/SSDPSearchResponseSocket.h>
#include <cybergarage/upnp/event/SubscriptionResponse.h>
#include <cybergarage/xml/Parser.h>

#include <iostream>

using namespace std;
using namespace CyberUtil;
using namespace CyberLink;
using namespace CyberXML;
using namespace CyberHTTP;
using namespace CyberNet;
using namespace CyberSOAP;
using namespace CyberIO;

////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////

const char *Device::ELEM_NAME = "device";

const char *Device::UPNP_ROOTDEVICE = "upnp:rootdevice";
const int Device::DEFAULT_STARTUP_WAIT_TIME = 1000;
const int Device::DEFAULT_DISCOVERY_WAIT_TIME = 500;
const int Device::DEFAULT_LEASE_TIME = 30 * 60;
const int Device::HTTP_DEFAULT_PORT = 4004;
const char *Device::DEFAULT_DESCRIPTION_URI = "/description.xml";
const char *Device::URLBASE_NAME = "URLBase";
const char *Device::DEVICE_TYPE = "deviceType";
const char *Device::FRIENDLY_NAME = "friendlyName";
const char *Device::MANUFACTURE = "manufacture";
const char *Device::MANUFACTURE_URL = "manufactureURL";
const char *Device::MODEL_DESCRIPTION = "modelDescription";
const char *Device::MODEL_NAME = "modelName";
const char *Device::MODEL_NUMBER = "modelNumber";
const char *Device::MODEL_URL = "modelURL";
const char *Device::SERIAL_NUMBER = "serialNumber";
const char *Device::UDN = "UDN";
const char *Device::UPC = "UPC";
const char *Device::presentationURL = "presentationURL";

////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////

Device::Device(CyberXML::Node *root, CyberXML::Node *device)
{
	setLocalRootDeviceFlag(false);
	rootNode = root;
	deviceNode = device;
	initUUID();
	initDeviceData();
	initChildList();
	setWirelessMode(false);
}

Device::Device()
{
	setLocalRootDeviceFlag(false);
	rootNode = NULL;
	deviceNode = NULL;
	initUUID();
	initDeviceData();
	initChildList();
}
	
Device::Device(CyberXML::Node *device)
{
	setLocalRootDeviceFlag(false);
	rootNode = NULL;
	deviceNode = device;
	initUUID();
	initDeviceData();
	initChildList();
}

void Device::initUUID()
{
	string uuid;
	setUUID(UPnP::CreateUUID(uuid));
}

void Device::initDeviceData()
{
	rootDevice = NULL;
	parentDevice = NULL;
	DeviceData *data = getDeviceData();
	if (data != NULL)
		data->setDevice(this);
}

void Device::initChildList()
{
	initDeviceList();
	initServiceList();
	initIconList();
}

////////////////////////////////////////////////
//	Constructor (File)
////////////////////////////////////////////////

#if !defined(BTRON) && !defined(ITRON) && !defined(TENGINE) 

Device::Device(CyberIO::File *descriptionFile)
{
	setLocalRootDeviceFlag(true);
	rootNode = NULL;
	deviceNode = NULL;
	initUUID();
	bool ret = loadDescription(descriptionFile);
	if (ret == false)
		throw InvalidDescriptionException(INVALIDDESCRIPTIONEXCEPTION_FILENOTFOUND);
}

Device::Device(const char *descriptionFileName)
{
	setLocalRootDeviceFlag(true);
	rootNode = NULL;
	deviceNode = NULL;
	initUUID();
	CyberIO::File descriptionFile(descriptionFileName);
	bool ret = loadDescription(&descriptionFile);
	if (ret == false)
		throw InvalidDescriptionException(INVALIDDESCRIPTIONEXCEPTION_FILENOTFOUND);
}

#endif

////////////////////////////////////////////////
//	Member
////////////////////////////////////////////////

CyberXML::Node *Device::getRootNode()
{
	if (rootNode != NULL)
		return rootNode;
	if (deviceNode == NULL)
		return NULL;
	return deviceNode->getRootNode();
}

////////////////////////////////////////////////
//	NMPR
////////////////////////////////////////////////
	
void Device::setNMPRMode(bool flag)
{
	Node *devNode = getDeviceNode();
	if (devNode == NULL)
		return;
	if (flag == true) {
		devNode->setNode(UPnP::INMPR03, UPnP::INMPR03_VER);
		devNode->removeNode(Device::URLBASE_NAME);
	}
	else {
		devNode->removeNode(UPnP::INMPR03);
	}
}

bool Device::isNMPRMode()
{
	Node *devNode = getDeviceNode();
	if (devNode == NULL)
		return false;
	return (devNode->getNode(UPnP::INMPR03) != NULL) ? true : false;
}

////////////////////////////////////////////////
//	URL Base
////////////////////////////////////////////////

void Device::setURLBase(const char *value)
{
	if (isRootDevice() == true) {
		Node *node = getRootNode()->getNode(URLBASE_NAME);
		if (node != NULL) {
			node->setValue(value);
			return;
		}
		node = new Node(URLBASE_NAME);
		node->setValue(value);
		int index = 1;
		if (getRootNode()->hasNodes() == false)
			index = 1;
		getRootNode()->insertNode(node, index);
	}
}

void Device::updateURLBase(const char *host)
{
	string urlBase;
	GetHostURL(host, getHTTPPort(), "", urlBase);
	setURLBase(urlBase.c_str());
}

const char *Device::getURLBase()
{
	if (isRootDevice() == true)
		return getRootNode()->getNodeValue(URLBASE_NAME);
	return "";
}

////////////////////////////////////////////////
//	Destructor
////////////////////////////////////////////////

Device::~Device()
{
	if (isLocalRootDevice() == true) {
		stop();
		if (rootDevice != NULL)
			delete rootDevice;
		if (parentDevice != NULL)
			delete parentDevice;
	}
}

////////////////////////////////////////////////
//	isDeviceType
////////////////////////////////////////////////

bool Device::isDeviceType(const char *value)
{
	if (value == NULL)
		return false;
	string valueStr = value;
	return (valueStr.compare(getDeviceType()) == 0) ? true : false;
}

////////////////////////////////////////////////
//	LeaseTime 
////////////////////////////////////////////////

void Device::setLeaseTime(int value)
{
	getDeviceData()->setLeaseTime(value);
	Advertiser *adv = getAdvertiser();
	if (adv != NULL) {
		announce();
		adv->restart();
	}
}

int Device::getLeaseTime()
{
	SSDPPacket *packet = getSSDPPacket();
	if (packet != NULL)
		return packet->getLeaseTime();
	return getDeviceData()->getLeaseTime();
}

////////////////////////////////////////////////
//	TimeStamp 
////////////////////////////////////////////////

long Device::getTimeStamp()
{
	SSDPPacket *packet = getSSDPPacket();
	if (packet != NULL)
		return packet->getTimeStamp();		
	return 0;
}

bool Device::isExpired()
{
	long elipsedTime = getElapsedTime();
	long leaseTime = getLeaseTime() + UPnP::DEFAULT_EXPIRED_DEVICE_EXTRA_TIME;
	if (leaseTime < elipsedTime)
		return true;
	return false;
}

 ////////////////////////////////////////////////
//	Root Device
////////////////////////////////////////////////

Device *Device::getRootDevice()
{
	if (rootDevice != NULL)
		return rootDevice;
	CyberXML::Node *rootNode = getRootNode();
	if (rootNode == NULL)
		return NULL;
	CyberXML::Node *devNode = rootNode->getNode(Device::ELEM_NAME);
	if (devNode == NULL)
		return NULL;
	rootDevice = new Device();
	rootDevice->setDeviceNode(devNode);
	rootDevice->setRootNode(rootNode);
	return rootDevice;
}

 ////////////////////////////////////////////////
//	Root Device
////////////////////////////////////////////////

// Thanks for Stefano Lenzi (07/24/04)

Device *Device::getParentDevice()
{
	if (parentDevice != NULL)
		return parentDevice;
	if(isRootDevice() == true)
		return NULL;
	CyberXML::Node *rootNode = getRootNode();
	if (rootNode == NULL)
		return NULL;
	Node *devNode = getDeviceNode();
	//<device><deviceList><device>
	devNode = devNode->getParentNode()->getParentNode();
	parentDevice = new Device();
	parentDevice->setDeviceNode(devNode);
	parentDevice->setRootNode(rootNode);
	return parentDevice;
}

////////////////////////////////////////////////
//	UserData
////////////////////////////////////////////////

DeviceData *Device::getDeviceData()
{
	CyberXML::Node *node = getDeviceNode();
	if (node == NULL)
		return NULL;
	DeviceData *userData = (DeviceData *)node->getUserData();
	if (userData == NULL) {
		userData = new DeviceData();
		node->setUserData(userData);
	}
	return userData;
}

////////////////////////////////////////////////
//	Description
////////////////////////////////////////////////

bool Device::loadDescription(const char *description)
{
	try {
		Parser parser;
		rootNode = parser.parse(description);
	}
	catch (ParserException e) {
		string msg;
		msg = "Couldn't load description";
		Debug::warning(msg);
		Debug::warning(e);
		//throw InvalidDescriptionException(msg.c_str()); 
		return false;
	}

	if (rootNode == NULL)
		return false;

	deviceNode = rootNode->getNode(Device::ELEM_NAME);
	if (deviceNode == NULL)
		return false;
		
	setDescriptionFile("");
	setDescriptionURI(DEFAULT_DESCRIPTION_URI);
	setLeaseTime(DEFAULT_LEASE_TIME);
	setHTTPPort(HTTP_DEFAULT_PORT);
	
	// Thanks for Oliver Newell (03/23/04)
	if (hasUDN() == false)
		updateUDN();

	initDeviceData();
	initChildList();

	return true;
}

////////////////////////////////////////////////
//	Description (File)
////////////////////////////////////////////////

#if !defined(BTRON) && !defined(ITRON) && !defined(TENGINE) 

bool Device::loadDescription(CyberIO::File *file)
{
	string description;

	if (file->load(description) == false) {
		string msg;
		msg += "Couldn't load description file (";
		msg += file->getName();
		msg += ")";
		Debug::warning(msg);
		//throw InvalidDescriptionException(msg.c_str()); 
		return false;
	}

	if (loadDescription(description.c_str()) == false)
		return false;

	setDescriptionFile(file->getName());

	// initialize childlist again
	initDeviceData();
	initChildList();

	return true;
}

#endif

////////////////////////////////////////////////
//	deviceList
////////////////////////////////////////////////

void Device::initDeviceList()
{
	deviceList.clear();
	Node *devNode = getDeviceNode();
	if (devNode == NULL)
		return;
	CyberXML::Node *devListNode = devNode->getNode(DeviceList::ELEM_NAME);
	if (devListNode == NULL)
		return;
	int nNode = devListNode->getNNodes();
	for (int n=0; n<nNode; n++) {
		CyberXML::Node *node = devListNode->getNode(n);
		if (Device::isDeviceNode(node) == false)
			continue;
		Device *dev = new Device(node);
		deviceList.add(dev);
	} 
}

bool Device::isDevice(const char *name)
{
	if (name == NULL)
		return false;
	String nameStr = name;
	if (nameStr.endsWith(getUDN()) == true)
		return true;
	if (nameStr.equals(getFriendlyName()) == true)
		return true;
	if (nameStr.endsWith(getDeviceType()) == true)
		return true;
	return false;
}
	
Device *Device::getDevice(const char *name)
{
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (int n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		if (dev->isDevice(name) == true)
			return dev;
		Device *cdev = dev->getDevice(name);
		if (cdev != NULL)
			return cdev;
	}
	return NULL;
}

Device *Device::getDeviceByDescriptionURI(const char *uri)
{
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (int n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		if (dev->isDescriptionURI(uri) == true)
			return dev;
		Device *cdev = dev->getDeviceByDescriptionURI(uri);
		if (cdev != NULL)
			return cdev;
	}
	return NULL;
}

////////////////////////////////////////////////
//	serviceList
////////////////////////////////////////////////

void Device::initServiceList()
{
	serviceList.clear();
	Node *devNode = getDeviceNode();
	if (devNode == NULL)
		return;
	CyberXML::Node *serviceListNode = devNode->getNode(ServiceList::ELEM_NAME);
	if (serviceListNode == NULL)
		return;
	int nNode = serviceListNode->getNNodes();
	for (int n=0; n<nNode; n++) {
		CyberXML::Node *node = serviceListNode->getNode(n);
		if (Service::isServiceNode(node) == false)
			continue;
		Service *service = new Service(node);
		serviceList.add(service);
	} 
}

Service *Device::getService(const char *name)
{
	int n;

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		if (service->isService(name) == true)
			return service;
	}
		
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Service *service = dev->getService(name);
		if (service != NULL)
			return service;
	}
		
	return NULL;
}

Service *Device::getServiceBySCPDURL(const char *searchUrl)
{
	int n;
	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		if (service->isSCPDURL(searchUrl) == true)
			return service;
	}
	
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Service *service = dev->getServiceBySCPDURL(searchUrl);
		if (service != NULL)
			return service;
	}
	
	return NULL;
}

Service *Device::getServiceByControlURL(const char *searchUrl)
{
	int n;
	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		if (service->isControlURL(searchUrl) == true)
			return service;
	}
	
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Service *service = dev->getServiceByControlURL(searchUrl);
		if (service != NULL)
			return service;
	}
	
	return NULL;
}

Service *Device::getServiceByEventSubURL(const char *searchUrl)
{
	int n;
	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		if (service->isEventSubURL(searchUrl) == true)
			return service;
	}
		
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Service *service = dev->getServiceByEventSubURL(searchUrl);
		if (service != NULL)
			return service;
	}
		
	return NULL;
}

Service *Device::getSubscriberService(const char *uuid)
{
	int n;
	String uuidStr = uuid;
	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		const char *sid = service->getSID();
		if (uuidStr.equals(sid) == true)
			return service;
	}
		
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Service *service = dev->getSubscriberService(uuid);
		if (service != NULL)
			return service;
	}
	
	return NULL;
}

////////////////////////////////////////////////
//	StateVariable
////////////////////////////////////////////////

StateVariable *Device::getStateVariable(const char* serviceType, const char *name)
{
	int n;
	string serviceTypeStr;
	if (serviceType != NULL)
		serviceTypeStr = serviceType;

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		// Thanks for Theo Beisch (11/09/04)
		if (serviceType != NULL) {
			if (serviceTypeStr.compare(service->getServiceType()) != 0)
				continue;
		}
		StateVariable *stateVar = service->getStateVariable(name);
		if (stateVar != NULL)
			return stateVar;
	}
	
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		StateVariable *stateVar = dev->getStateVariable(name);
		if (stateVar != NULL)
			return stateVar;
	}
		
	return NULL;
}

StateVariable *Device::getStateVariable(const char *name)
{
	return getStateVariable(NULL, name);
}

////////////////////////////////////////////////
//	Action
////////////////////////////////////////////////

CyberLink::Action *Device::getAction(const char *name)
{
	int n;
	CyberUtil::String nameStr= name;

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		ActionList *actionList = service->getActionList();
		int actionCnt = actionList->size();
		for (int i=0; i<actionCnt; i++) {
			Action *action = actionList->getAction(i);
			const char *actionName = action->getName();
			if (actionName == NULL)
				continue;
			if (nameStr.equals(actionName) == true)
				return action;
		}
	}
		
	DeviceList *devList = getDeviceList();
	int devCnt = devList->size();
	for (n=0; n<devCnt; n++) {
		Device *dev = devList->getDevice(n);
		Action *action = dev->getAction(name);
		if (action != NULL)
			return action;
	}
		
	return NULL;
}

////////////////////////////////////////////////
//	iconList
////////////////////////////////////////////////

void Device::initIconList()
{
	iconList.clear();
	Node *devNode = getDeviceNode();
	if (devNode == NULL)
		return;
	Node *iconListNode = devNode->getNode(IconList::ELEM_NAME);
	if (iconListNode == NULL)
		return;
	int nNode = iconListNode->getNNodes();
	for (int n=0; n<nNode; n++) {
		Node *node = iconListNode->getNode(n);
		if (Icon::isIconNode(node) == false)
			continue;
		Icon *icon = new Icon(node);
		iconList.add(icon);
	} 
}
	
////////////////////////////////////////////////
//	Notify
////////////////////////////////////////////////

void Device::notifyWait()
{
	CyberUtil::WaitRandom(DEFAULT_DISCOVERY_WAIT_TIME);
}

const char *Device::getLocationURL(const char *host, std::string &buf)
{
	return CyberNet::GetHostURL(host, getHTTPPort(), getDescriptionURI(), buf);
}

const char *Device::getNotifyDeviceNT(std::string &buf)
{
	if (isRootDevice() == false)
		buf = getUDN();			
	buf = UPNP_ROOTDEVICE;
	return buf.c_str();
}

const char *Device::getNotifyDeviceUSN(std::string &buf)
{
	buf = getUDN();
	if (isRootDevice() == true) {
		buf.append("::");
		buf.append(UPNP_ROOTDEVICE);
	}
	return buf.c_str();
}

const char *Device::getNotifyDeviceTypeNT(std::string &buf)
{
	buf = getDeviceType();
	return buf.c_str();
}

const char *Device::getNotifyDeviceTypeUSN(std::string &buf)
{
	buf = getUDN();
	buf.append("::");
	buf.append(getDeviceType());
	return buf.c_str();
}

void Device::announce(const char *bindAddr)
{
	string devLocationBuf;
	const char *devLocation = getLocationURL(bindAddr, devLocationBuf);
		
	SSDPNotifySocket ssdpSock(bindAddr);

	SSDPNotifyRequest ssdpReq;
	string serverName;
	ssdpReq.setServer(UPnP::GetServerName(serverName));
	ssdpReq.setLeaseTime(getLeaseTime());
	ssdpReq.setLocation(devLocation);
	ssdpReq.setNTS(NTS::ALIVE);
		
	// uuid:device-UUID(::upnp:rootdevice)* 
	if (isRootDevice() == true) {
		string devNT, devUSN;
		getNotifyDeviceNT(devNT);
		getNotifyDeviceUSN(devUSN);
		ssdpReq.setNT(devNT.c_str());
		ssdpReq.setUSN(devUSN.c_str());
		ssdpSock.post(&ssdpReq);
	}

	// uuid:device-UUID::urn:schemas-upnp-org:device:deviceType:v 
	string devNT, devUSN;
	getNotifyDeviceTypeNT(devNT);
	getNotifyDeviceTypeUSN(devUSN);
	ssdpReq.setNT(devNT.c_str());
	ssdpReq.setUSN(devUSN.c_str());
	ssdpSock.post(&ssdpReq);

	// Thanks for Mikael Hakman (04/25/05)
	ssdpSock.close();

	int n;

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		service->announce(bindAddr);
	}

	DeviceList *childDeviceList = getDeviceList();
	int childDeviceCnt = childDeviceList->size();
	for (n=0; n<childDeviceCnt; n++) {
		Device *childDevice = childDeviceList->getDevice(n);
		childDevice->announce(bindAddr);
	}
}

void Device::announce()
{
	notifyWait();

	int nHostAddrs = CyberNet::GetNHostAddresses();
	for (int n=0; n<nHostAddrs; n++) {
		std::string bindAddrBuf;
		const char *bindAddr = CyberNet::GetHostAddress(n, bindAddrBuf);
		if (CyberUtil::StringHasData(bindAddr) == false)
			continue;
		int ssdpCount = getSSDPAnnounceCount();
		for (int i=0; i<ssdpCount; i++)
			announce(bindAddr);
	}
}
	
void Device::byebye(const char *bindAddr)
{
	int n;
	SSDPNotifySocket ssdpSock(bindAddr);
		
	SSDPNotifyRequest ssdpReq;
	ssdpReq.setNTS(NTS::BYEBYE);
		
	// uuid:device-UUID(::upnp:rootdevice)* 
	if (isRootDevice() == true) {
		string devNT, devUSN;
		getNotifyDeviceNT(devNT);
		getNotifyDeviceUSN(devUSN);
		ssdpReq.setNT(devNT.c_str());
		ssdpReq.setUSN(devUSN.c_str());
		ssdpSock.post(&ssdpReq);
	}

	// uuid:device-UUID::urn:schemas-upnp-org:device:deviceType:v 
	string devNT, devUSN;
	getNotifyDeviceTypeNT(devNT);
	getNotifyDeviceTypeUSN(devUSN);
	ssdpReq.setNT(devNT.c_str());
	ssdpReq.setUSN(devUSN.c_str());
	ssdpSock.post(&ssdpReq);

	// Thanks for Mikael Hakman (04/25/05)
	ssdpSock.close();

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		service->byebye(bindAddr);
	}

	DeviceList *childDeviceList = getDeviceList();
	int childDeviceCnt = childDeviceList->size();
	for (n=0; n<childDeviceCnt; n++) {
		Device *childDevice = childDeviceList->getDevice(n);
		childDevice->byebye(bindAddr);
	}
}

void Device::byebye()
{
	int nHostAddrs = CyberNet::GetNHostAddresses();
	for (int n=0; n<nHostAddrs; n++) {
		std::string bindAddrBuf;
		const char *bindAddr = CyberNet::GetHostAddress(n, bindAddrBuf);
		if (CyberUtil::StringHasData(bindAddr) == false)
			continue;
		int ssdpCount = getSSDPAnnounceCount();
		for (int i=0; i<ssdpCount; i++)
			byebye(bindAddr);
	}
}

////////////////////////////////////////////////
//	Search
////////////////////////////////////////////////

bool Device::postSearchResponse(SSDPPacket *ssdpPacket, const char *st, const char *usn)
{
	const char *localAddr = ssdpPacket->getLocalAddress();
	Device *rootDev = getRootDevice();
	string rootDevLocation;
	rootDev->getLocationURL(localAddr, rootDevLocation);
		
	SSDPSearchResponse ssdpRes;
	HTTPDate currDate;
	ssdpRes.setLeaseTime(getLeaseTime());
	ssdpRes.setDate(&currDate);
	ssdpRes.setST(st);
	ssdpRes.setUSN(usn);
	ssdpRes.setLocation(rootDevLocation.c_str());
	// Thanks for Brent Hills (10/20/04)
	ssdpRes.setMYNAME(getFriendlyName());

	int mx = ssdpPacket->getMX();
	CyberUtil::WaitRandom(mx * 1000);
		
	const char *remoteAddr = ssdpPacket->getRemoteAddress();
	int remotePort = ssdpPacket->getRemotePort();
	SSDPSearchResponseSocket ssdpResSock;
	if (Debug::isOn() == true)
		ssdpRes.print();
	int ssdpCount = getSSDPAnnounceCount();
	for (int i=0; i<ssdpCount; i++)
		ssdpResSock.post(remoteAddr, remotePort, &ssdpRes);

	return true;
}

void Device::deviceSearchResponse(SSDPPacket *ssdpPacket)
{
	string ssdpST;
	ssdpPacket->getST(ssdpST);

	if (ssdpST.length() <= 0)
		return;

	bool isRootDev = isRootDevice();
		
	string devUSN = getUDN();
	if (isRootDev == true) {
		devUSN.append("::");
		devUSN.append(USN::ROOTDEVICE);
	}
			
	if (ST::IsAllDevice(ssdpST.c_str()) == true) {
		string devNT;
		getNotifyDeviceNT(devNT);			
		int repeatCnt = (isRootDev == true) ? 3 : 2;
		for (int n=0; n<repeatCnt; n++)
			postSearchResponse(ssdpPacket, devNT.c_str(), devUSN.c_str());
	}
	else if (ST::IsRootDevice(ssdpST.c_str()) == true) {
		if (isRootDev == true)
			postSearchResponse(ssdpPacket, ST::ROOT_DEVICE, devUSN.c_str());
	}
	else if (ST::IsUUIDDevice(ssdpST.c_str()) == true) {
		const char *devUDN = getUDN();
		if (ssdpST.compare(devUDN) == 0)
			postSearchResponse(ssdpPacket, devUDN, devUSN.c_str());
	}
	else if (ST::IsURNDevice(ssdpST.c_str()) == true) {
		const char *devType= getDeviceType();
		if (ssdpST.compare(devType) == 0) {
			// Thanks for Mikael Hakman (04/25/05)
			devUSN = getUDN();
			devUSN += "::";
			devUSN += devType;
			postSearchResponse(ssdpPacket, devType, devUSN.c_str());
		}
	}
		
	int n;

	ServiceList *serviceList = getServiceList();
	int serviceCnt = serviceList->size();
	for (n=0; n<serviceCnt; n++) {
		Service *service = serviceList->getService(n);
		service->serviceSearchResponse(ssdpPacket);
	}
		
	DeviceList *childDeviceList = getDeviceList();
	int childDeviceCnt = childDeviceList->size();
	for (n=0; n<childDeviceCnt; n++) {
		Device *childDevice = childDeviceList->getDevice(n);
		childDevice->deviceSearchResponse(ssdpPacket);
	}
}


void Device::deviceSearchReceived(SSDPPacket *ssdpPacket)
{
	deviceSearchResponse(ssdpPacket);
}

////////////////////////////////////////////////
//	HTTP Server	
////////////////////////////////////////////////

void Device::httpRequestRecieved(CyberHTTP::HTTPRequest *httpReq)
{
	if (Debug::isOn() == true)
		httpReq->print();

	/*
	string buf;
	httpReq->toString(buf);
	Debug::on();
	Debug::message(buf.c_str(), "httpRequestRecieved.log");
	*/
	
	if (httpReq->isGetRequest() == true) {
		httpGetRequestRecieved(httpReq);
		return;
	}

	if (httpReq->isPostRequest() == true) {
		httpPostRequestRecieved(httpReq);
		return;
	}

	if (httpReq->isSubscribeRequest() == true || httpReq->isUnsubscribeRequest() == true) {
		SubscriptionRequest subReq(httpReq);
		deviceEventSubscriptionRecieved(&subReq);
		return;
	}

	httpReq->returnBadRequest();
}

////////////////////////////////////////////////
//	HTTP Server	
////////////////////////////////////////////////

const char *Device::getDescriptionData(const char *host, string &buf)
{
	lock();
	if (isNMPRMode() == false)
		updateURLBase(host);
	Node *rootNode = getRootNode();
	buf = "";
	if (rootNode != NULL) {
		string nodeBuf;
		// Thanks for Mikael Hakman (04/25/05)
		buf = UPnP::XML_DECLARATION;
		buf += "\n";
		buf += rootNode->toString(nodeBuf);
	}
	unlock();
	return buf.c_str();
}
	
void Device::httpGetRequestRecieved(HTTPRequest *httpReq)
{
	string uri;
	httpReq->getURI(uri);
	if (uri.length() <= 0) {
		httpReq->returnBadRequest();
		return;
	}
			
	string rootPathBuf;
	const char *rootPath = getDescriptionFilePath(rootPathBuf);
	
	string fileByteBuf;
	const char *fileByte = "";

	Device *embDev;
	Service *embService;

	if (isDescriptionURI(uri.c_str()) == true) {
		const char *localAddr = httpReq->getLocalAddress();
		fileByte = getDescriptionData(localAddr, fileByteBuf);
	}
	else if ((embDev = getDeviceByDescriptionURI(uri.c_str())) != NULL) {
		const char *localAddr = httpReq->getLocalAddress();
		fileByte = embDev->getDescriptionData(localAddr, fileByteBuf);
	}
	else if ((embService = getServiceBySCPDURL(uri.c_str())) != NULL) {
		fileByte = embService->getSCPDData(fileByteBuf);
	}
	else {
		httpReq->returnBadRequest();
		return;
	}

	HTTPResponse httpRes;
	if (File::isXMLFileName(uri.c_str()) == true)
		httpRes.setContentType(XML::CONTENT_TYPE);
	httpRes.setStatusCode(HTTP::OK_REQUEST);
	httpRes.setContent(fileByte);
	httpReq->post(&httpRes);
}

void Device::httpPostRequestRecieved(HTTPRequest *httpReq)
{
	if (httpReq->isSOAPAction() == true) {
		//SOAPRequest soapReq = new SOAPRequest(httpReq);
		soapActionRecieved(httpReq);
		return;
	}
	httpReq->returnBadRequest();
}

////////////////////////////////////////////////
//	SOAP
////////////////////////////////////////////////

void Device::soapBadActionRecieved(HTTPRequest *soapReq)
{
	SOAPResponse soapRes;
	soapRes.setStatusCode(HTTP::BAD_REQUEST);
	soapReq->post(&soapRes);
}

void Device::soapActionRecieved(HTTPRequest *soapReq)
{
	string uri;
	soapReq->getURI(uri);
	Service *ctlService = getServiceByControlURL(uri.c_str());
	if (ctlService != NULL)  {
		ActionRequest crlReq(soapReq);
		deviceControlRequestRecieved(&crlReq, ctlService);
		return;
	}
	soapBadActionRecieved(soapReq);
}

////////////////////////////////////////////////
//	controlAction
////////////////////////////////////////////////

void Device::invalidActionControlRecieved(ControlRequest *ctlReq)
{
	ControlResponse actRes;
	actRes.setFaultResponse(UPnP::INVALID_ACTION);
	ctlReq->post(&actRes);
}

void Device::deviceControlRequestRecieved(ControlRequest *ctlReq, Service *service)
{
	if (ctlReq->isQueryControl() == true) {
		QueryRequest queryRes(ctlReq);
		deviceQueryControlRecieved(&queryRes, service);
	}
	else {
		ActionRequest actRes(ctlReq);
		deviceActionControlRecieved(&actRes, service);
	}
}

void Device::deviceActionControlRecieved(ActionRequest *ctlReq, Service *service)
{
	if (Debug::isOn() == true)
		ctlReq->print();
			
	string actionNameBuf;
	const char *actionName = ctlReq->getActionName(actionNameBuf);
	Action *action = service->getAction(actionName);
	if (action == NULL) {
		invalidActionControlRecieved(ctlReq);
		return;
	}
	ArgumentList *actionArgList = action->getArgumentList();
	ArgumentList *reqArgList = ctlReq->getArgumentList();
	actionArgList->set(reqArgList);
	if (action->performActionListener(ctlReq) == false)
		invalidActionControlRecieved(ctlReq);
}

void Device::deviceQueryControlRecieved(QueryRequest *ctlReq, Service *service)
{
	if (Debug::isOn() == true)
		ctlReq->print();
	const char *varName = ctlReq->getVarName();
	if (service->hasStateVariable(varName) == false) {
		invalidActionControlRecieved(ctlReq);
		return;
	}
	StateVariable *stateVar = getStateVariable(varName);
	if (stateVar->performQueryListener(ctlReq) == false)
		invalidActionControlRecieved(ctlReq);
}

////////////////////////////////////////////////
//	eventSubscribe
////////////////////////////////////////////////

void Device::upnpBadSubscriptionRecieved(SubscriptionRequest *subReq, int code)
{
	SubscriptionResponse subRes;
	subRes.setErrorResponse(code);
	subReq->post(&subRes);
}

void Device::deviceEventSubscriptionRecieved(SubscriptionRequest *subReq)
{
	if (Debug::isOn() == true)
		subReq->print();
	string uri;
	subReq->getURI(uri);
	Service *service = getServiceByEventSubURL(uri.c_str());
	if (service == NULL) {
		subReq->returnBadRequest();
		return;
	}
	if (subReq->hasCallback() == false && subReq->hasSID() == false) {
		upnpBadSubscriptionRecieved(subReq, CyberHTTP::HTTP::PRECONDITION_FAILED);
		return;
	}

	// UNSUBSCRIBE
	if (subReq->isUnsubscribeRequest() == true) {
		deviceEventUnsubscriptionRecieved(service, subReq);
		return;
	}

	// SUBSCRIBE (NEW)
	if (subReq->hasCallback() == true) {
		deviceEventNewSubscriptionRecieved(service, subReq);
		return;
	}
		
	// SUBSCRIBE (RENEW)
	if (subReq->hasSID() == true) {
		deviceEventRenewSubscriptionRecieved(service, subReq);
		return;
	}
		
	upnpBadSubscriptionRecieved(subReq, CyberHTTP::HTTP::PRECONDITION_FAILED);
}

void Device::deviceEventNewSubscriptionRecieved(Service *service, SubscriptionRequest *subReq)
{
	string callback;
	subReq->getCallback(callback);

	URL url(callback.c_str());

	long timeOut = subReq->getTimeout();
	string sid;
	Subscription::CreateSID(sid);
			
	Subscriber *sub = new Subscriber();
	sub->setDeliveryURL(callback.c_str());
	sub->setTimeOut(timeOut);
	sub->setSID(sid.c_str());
	service->addSubscriber(sub);
			
	SubscriptionResponse subRes;
	subRes.setStatusCode(HTTP::OK_REQUEST);
	subRes.setSID(sid.c_str());
	subRes.setTimeout(timeOut);
	subReq->post(&subRes);

	service->notifyAllStateVariables();
}

void Device::deviceEventRenewSubscriptionRecieved(Service *service, SubscriptionRequest *subReq)
{
	string sidBuf;
	const char *sid = subReq->getSID(sidBuf);
	Subscriber *sub = service->getSubscriberBySID(sid);

	if (sub == NULL) {
		upnpBadSubscriptionRecieved(subReq, CyberHTTP::HTTP::PRECONDITION_FAILED);
		return;
	}

	long timeOut = subReq->getTimeout();
	sub->setTimeOut(timeOut);
	sub->renew();
				
	SubscriptionResponse subRes;
	subRes.setStatusCode(HTTP::OK_REQUEST);
	subRes.setSID(sid);
	subRes.setTimeout(timeOut);
	subReq->post(&subRes);
}		

void Device::deviceEventUnsubscriptionRecieved(Service *service, SubscriptionRequest *subReq)
{
	string sidBuf;
	const char *sid = subReq->getSID(sidBuf);

	Subscriber *sub = service->getSubscriberBySID(sid);
	if (sub == NULL) {
		upnpBadSubscriptionRecieved(subReq, CyberHTTP::HTTP::PRECONDITION_FAILED);
		return;
	}

	service->removeSubscriber(sub);
						
	SubscriptionResponse subRes;
	subRes.setStatusCode(HTTP::OK_REQUEST);
	subReq->post(&subRes);
}		

////////////////////////////////////////////////
//	Thread	
////////////////////////////////////////////////

bool Device::start()
{
	stop(true);

	////////////////////////////////////////
	// HTTP Server
	////////////////////////////////////////
		
	int retryCnt = 0;
	int bindPort = getHTTPPort();
	HTTPServerList *httpServerList = getHTTPServerList();
	while (httpServerList->open(bindPort) == false) {
		retryCnt++;
		if (UPnP::SERVER_RETRY_COUNT < retryCnt)
			return false;
		setHTTPPort(bindPort + 1);
		bindPort = getHTTPPort();
	}
	httpServerList->addRequestListener(this);
	httpServerList->start();

	////////////////////////////////////////
	// SSDP Seach Socket
	////////////////////////////////////////
	
	SSDPSearchSocketList *ssdpSearchSockList = getSSDPSearchSocketList();
	if (ssdpSearchSockList->open() == false)
		return false;
	ssdpSearchSockList->addSearchListener(this);
	ssdpSearchSockList->start();

	////////////////////////////////////////
	// Announce
	////////////////////////////////////////

	announce();

	////////////////////////////////////////
	// Advertiser
	////////////////////////////////////////

	Advertiser *adv = new Advertiser(this);
	setAdvertiser(adv);
	adv->start();
	
	return true;
}
	
bool Device::stop(bool doByeBye)
{
	if (doByeBye == true)
		byebye();
		
	HTTPServerList *httpServerList = getHTTPServerList();
	httpServerList->stop();
	httpServerList->close();
	httpServerList->clear();

	SSDPSearchSocketList *ssdpSearchSockList = getSSDPSearchSocketList();
	ssdpSearchSockList->stop();
	ssdpSearchSockList->close();
	ssdpSearchSockList->clear();
		
	Advertiser *adv = getAdvertiser();
	if (adv != NULL) {
		adv->stop();
		setAdvertiser(NULL);
	}

	return true;
}
	
////////////////////////////////////////////////
//	Action/QueryListener
////////////////////////////////////////////////

void Device::setActionListener(ActionListener *listener)
{
	ServiceList *ServiceList = getServiceList();
	int nServices = ServiceList->size();
	for (int n=0; n<nServices; n++) {
		Service *Service = ServiceList->getService(n);
		Service->setActionListener(listener);
	}
}

void Device::setQueryListener(QueryListener *listener)
{
	ServiceList *ServiceList = getServiceList();
	int nServices = ServiceList->size();
	for (int n=0; n<nServices; n++) {
		Service *Service = ServiceList->getService(n);
		Service->setQueryListener(listener);
	}
}

////////////////////////////////////////////////
//	Action/QueryListener
////////////////////////////////////////////////

// Thanks for Mikael Hakman (04/25/05)
void Device::setActionListener(ActionListener *listener, bool includeSubDevices)
{
	setActionListener(listener);
	if (includeSubDevices == true) {
		DeviceList *devList = getDeviceList();
		int devCnt = devList->size();
		for (int n=0; n<devCnt; n++) {
			Device *dev = devList->getDevice(n);
			dev->setActionListener(listener, true);
		}
	}
}

// Thanks for Mikael Hakman (04/25/05)
void Device::setQueryListener(QueryListener *listener, bool includeSubDevices)
{
	setQueryListener(listener);
	if (includeSubDevices == true) {
		DeviceList *devList = getDeviceList();
		int devCnt = devList->size();
		for (int n=0; n<devCnt; n++) {
			Device *dev = devList->getDevice(n);
			dev->setQueryListener(listener, true);
		}
	}
}
