#include <basic.h>
#include <bstdlib.h>
#include <bstdio.h>
#include <bstring.h>
#include <errcode.h>
#include <tstring.h>
#include <tcode.h>
#include <btron/btron.h>
#include <btron/dp.h>
#include <btron/hmi.h>
#include <btron/vobj.h>
#include <btron/libapp.h>
#include <bsyslog.h>
#include <typedef.h>
#include <net/sock_com.h>
#include <btron/bsocket.h>

#include <cybergarage/util/Debug.h>
#include <cybergarage/upnp/ssdp/SSDP.h>
#include <cybergarage/net/HostInterface.h>
#include <cybergarage/net/Socket.h>
#include <cybergarage/net/SocketUtil.h>
#include <cybergarage/net/DatagramSocket.h>
#include <cybergarage/net/MulticastSocket.h>
#include <cybergarage/http/HTTPServer.h>
#include <cybergarage/xml/Parser.h>

using namespace std;
using namespace CyberNet;
using namespace CyberLink;
using namespace CyberHTTP;
using namespace CyberXML;
using namespace CyberUtil;

//#define ASSERT(e)       ((e) ? (void)0 : syslog(LOG_DEBUG, "%s(%d) %s"__FILE__, __LINE__, #e))
#define ASSERT(e)       ((e) ? (void)0 : syslog(LOG_DEBUG, "%s(%d)"__FILE__, __LINE__))

//////////////////////////////////////////////////
// testHello
//////////////////////////////////////////////////

void testHello()
{
	printf("Hello BTRON World !!\n");
}

//////////////////////////////////////////////////
// testHostInterface
//////////////////////////////////////////////////

void testHostInterface()
{
	NetworkInterfaceList netIfList;
	GetHostAddresses(netIfList);
	int netIfCnt = netIfList.size();
	for (int n=0; n<netIfCnt; n++) {
			NetworkInterface *netif = netIfList.getNetworkInterface(n);
			printf("[%d] = %s, %s\n", n, netif->getName(), netif->getAddress());
	}
	ASSERT(netIfCnt == 1);
}

//////////////////////////////////////////////////
// testSocket
//////////////////////////////////////////////////

void testSocket()
{
	//std::cout << "" << "";
	struct sockaddr_in sockaddr;
	bool ret = toSocketAddrIn("192.168.1.1", 80, &sockaddr);
	printf("toSocketAddrIn = %d\n", ret);
	ASSERT(ret == true);
}

//////////////////////////////////////////////////
// testMulticast
//////////////////////////////////////////////////

void testMulticast()
{
	MulticastSocket msock;
	const char ssdpmsg[] = "NOTIFY * HTTP1.1\r\nHOST: UNKOWN\r\nSERVER: BTRON\r\n\r\n";
	printf(ssdpmsg);
	int nSend = msock.send(SSDP::ADDRESS, SSDP::PORT, ssdpmsg, strlen(ssdpmsg));
	printf("nSend = %d\n", nSend);
	ASSERT(nSend == (int)strlen(ssdpmsg));
}

//////////////////////////////////////////////////
// testSSDPDump
//////////////////////////////////////////////////

void testSSDPDump()
{
	MulticastSocket msock;
	msock.bind(SSDP::PORT, "");
	DatagramPacket dgmPkt;
	while (0 < msock.receive(dgmPkt)) {
		printf("%s\n", dgmPkt.getData());
	}
}

//////////////////////////////////////////////////
// testServerName
//////////////////////////////////////////////////

void testServerName()
{
	string buf;
	GetServerName(buf);
	printf("%s\n", buf.c_str());
}

//////////////////////////////////////////////////
// testExpat
//////////////////////////////////////////////////

void testExpat()
{
	string xml = "<root><name>Satoshi Konno</name></root>";
	Parser expat;

	Node *node = expat.parse(xml.c_str(), xml.length());
	if (node != NULL) {
		node->print();
		delete node;
	}
	else
		Debug::warning("Expat parse is fail");

}

//////////////////////////////////////////////////
// TestHTTPServer
//////////////////////////////////////////////////

class TestHTTPServer : public CyberHTTP::HTTPServer, public CyberHTTP::HTTPRequestListener
{
public:

	TestHTTPServer()
	{
		addRequestListener(this);
	}

	////////////////////////////////////////////////
	// HttpRequestListner
	////////////////////////////////////////////////

	void httpRequestRecieved(CyberHTTP::HTTPRequest *httpReq)
	{
		HTTPResponse httpRes;
		httpRes.print();
		httpRes.setStatusCode(HTTP::OK_REQUEST);
		httpRes.setContent("<html><body>HELLO BTRON WORLD</body></html>");
		httpReq->post(&httpRes);
	}

	////////////////////////////////////////////////
	// update
	////////////////////////////////////////////////

	void update()
	{
	}
};

void testHTTPServer()
{
	NetworkInterfaceList netIfList;
	GetHostAddresses(netIfList);
	int netIfCnt = netIfList.size();
	if (netIfCnt <= 0)
		return;

	NetworkInterface *netif = netIfList.getNetworkInterface(0);

	bool ret;
	TestHTTPServer httpServer;
	//ret = httpServer.open("127.0.0.1", 80);
	//ret = httpServer.open("localhost", 80);
	ret = httpServer.open(netif->getAddress(), 80);
	printf("open = %d\n", (int)ret);
	ret = httpServer.start();
	printf("start = %d\n", (int)ret);
/*
	TestHTTPServer httpServer2;
	ret = httpServer2.open("127.0.0.1", 81);
	printf("open = %d\n", (int)ret);
	ret = httpServer2.start();
	printf("start = %d\n", (int)ret);
*/
}

//////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////

extern "C" {
EXPORT W MAIN(MESSAGE *msg);
}

EXPORT W MAIN(MESSAGE *msg)
{
	setlogmask(LOG_UPTO(LOG_DEBUG));

	testHello();
	testServerName();
	testHostInterface();
	testExpat();
	testHTTPServer();
	/*
	testSocket();
	testMulticast();
	testSSDPDump();
	*/

	while (true) {
		slp_tsk(1000);
	}

	return 0;
}
