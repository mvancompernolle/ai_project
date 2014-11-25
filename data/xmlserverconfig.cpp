/***************************************************************************
 *   Copyright (C) 2004-2006 by Ilya A. Volynets-Evenbakh                  *
 *   ilya@total-knowledge.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <serverconfig/xmlserverconfig.h>
#include <sptk4/CBuffer.h>
#include <sptk4/cxml>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

using namespace sptk;
namespace container {
namespace serverconfig {

XMLServerConfig::XMLServerConfig()
	: ServerConfig()
	, m_path("/etc/engine.xml")
{
}


XMLServerConfig::~XMLServerConfig()
{
	if(m_root)
		delete m_root;
}


ConfigNode::childlist_t XMLServerConfig::getNodes(const ConfigNode* parent)
{
	XMLConfigNode* pparent=(XMLConfigNode*)parent;
	ConfigNode::childlist_t ret;
	for(ConfigNode::childlist_t::iterator i=pparent->getChildren().begin();i!=pparent->getChildren().end();i++){
		ret.push_back(*i);
	}
	return ret;
}
ConfigNode::childlist_t XMLServerConfig::getNodes(const ConfigNode* parent, const std::string& type)
{
	XMLConfigNode* pparent=(XMLConfigNode*)parent;
	ConfigNode::childlist_t ret;
	for(ConfigNode::childlist_t::iterator i=pparent->getChildren().begin();i!=pparent->getChildren().end();i++){
		if((*i)->getType()==type)
			ret.push_back(*i);
	}
	return ret;
}

/*!
	\fn container::serverconfig::XMLServerConfig::addKids(sptk::CXmlNode* n, container::serverconfig::XMLServerConfig::XMLConfigNode* parent)
	@param n XML node from parsed tree.
	@param parent CXmlConfigNode to add children to.
	Adds children nodes to parent. Presumes parent is always passed in (i.e. root node is
	created explicitly elsewhere)
 */
void XMLServerConfig::addKids(CXmlNode* n, XMLServerConfig::XMLConfigNode* parent)
{
	for(CXmlNode::iterator node=n->begin();node!=n->end();node++){
		switch((*node)->type()){
		case CXmlNode::DOM_ELEMENT:
			{
				XMLServerConfig::XMLConfigNode* cnode =
					 new XMLServerConfig::XMLConfigNode(std::string((*node)->name()),parent);
				if ((*node)->hasAttributes()) {
#ifdef XML_ATTRIBUTE_IS_NODE
					const CXmlAttributes &attributes = (*node)->attributes();
					for(CXmlAttributes::const_iterator it=attributes.begin();
						it!=attributes.end(); it++) {
						const CXmlNode* attr = *it;
						cnode->addAttr(attr->name(),attr->value());
					}
#else
					const CXmlAttributes &attr_map = (*node)->attributes;
					for(CXmlAttributes::const_iterator it=attr_map.begin();
						it!=attr_map.end(); it++) {
						cnode->addAttr(it->first,it->second);
					}
#endif
				}
				if(!(*node)->empty())
					addKids(*node,cnode);
				parent->addNode(cnode);//Presumes parent is always set
			}
			break;
		default:
			break;
		}
	}
}

/*!
    \fn XMLServerConfig::load()
 */
void XMLServerConfig::load()
{
	XMLServerConfig::XMLConfigNode *r=new XMLServerConfig::XMLConfigNode("root","root",0);
	m_root=r;
	try {
		sptk::CBuffer buf;
		buf.loadFromFile(m_path.c_str());
		CXmlDoc doc;
		doc.load(buf);
		addKids(&doc,r);
	} catch (CXmlException& exp) {
		std::cerr<<"XML error: "<<exp.what()<<std::endl;
	} catch (CException& e) {
		std::cerr<<"error: "<<e.message()<<std::endl;
	} catch (...) {
		std::cerr<<"Error reading config\n";
	}
}


void XMLServerConfig::XMLConfigNode::addAttr(const std::string& name, const std::string& val)
{
	if(name=="name")
		this->m_name=val;
	m_attrs[name]=val;
}

void XMLServerConfig::XMLConfigNode::addNode(XMLConfigNode* node)
{
	m_children.push_back(node);
}

}
}
