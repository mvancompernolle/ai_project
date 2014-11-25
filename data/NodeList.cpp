/******************************************************************
*
*	CyberXML for C++
*
*	Copyright (C) Satoshi Konno 2002-2003
*
*	File: NodeList.cpp
*
*	Revision;
*
*	04/28/04
*		- first revision
*	06/30/04
*		- Added addNode(), insertNode() and removeNode();
*
******************************************************************/

#include <cybergarage/xml/NodeList.h>
#include <cybergarage/xml/Node.h>

using namespace CyberXML;
using namespace CyberUtil;

NodeList::NodeList() 
{
}

NodeList::~NodeList() 
{
	clear();
}

bool NodeList::addNode(Node *node) 
{
	if (node == NULL)
		return false;
	return add(node);
}

bool NodeList::insertNode(Node *node, int index) 
{
	if (node == NULL)
		return false;
	return insertAt(node, index);
}

bool NodeList::removeNode(Node *node) 
{
	if (node == NULL)
		return false;
	bool ret = remove(node);
	delete node;
	return ret;
}

Node *NodeList::getNode(int n)
{
	return (Node *)Vector::get(n);
}

Node *NodeList::getNode(const char *name) 
{
	if (name == NULL)
		return NULL;
		
	int nLists = size(); 
	for (int n=0; n<nLists; n++) {
		Node *node = getNode(n);
		const char *nodeName = node->getName();
		if (StringEquals(name, nodeName) == true)
			return node;
	}
	return NULL;
}

Node *NodeList::getEndsWith(const char *name) 
{
	if (name == NULL)
		return NULL;

	int nLists = size(); 
	for (int n=0; n<nLists; n++) {
		Node *node = getNode(n);
		const char *nodeName = node->getName();
		if (nodeName == NULL)
			continue;
		if (StringEndsWith(nodeName, name) == true)
			return node;
	}
	return NULL;
}

void NodeList::clear()
{
	int nLists = size(); 
	for (int n=0; n<nLists; n++) {
		Node *node = getNode(n);
		if (node != NULL)
			delete node;
	}
	Vector::clear();
}
