/***************************************************************************
 *   Copyright (C) 2007 by Ilya A. Volynets-Evenbakh                       *
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

#include <serverconfig/serverconfig.h>

using namespace container;
namespace container {
namespace serverconfig {

GlobalContext::GlobalContext(ServerConfig& cfg)
	: Context(cfg, 0)
	, m_nThreads(-1)
	, m_queueDepth(-1)
	, m_listener(0)
	, m_pool(0)
{
	m_paramregistry.getParamList(getUnsetParams());
}

GlobalContext::~GlobalContext()
{
	for(std::list<GlobalConfigNode*>::iterator it=m_globalNodes.begin();
	    it!=m_globalNodes.end(); it++)
	{
		delete *it;
	}
	if(m_listener)
		delete m_listener;
	if(m_pool)
		delete m_pool;
	for(std::list<RequestListener::Acceptor*>::iterator it=m_acceptors.begin();
	    it!=m_acceptors.end(); it++)
	    delete *it;
}

void GlobalContext::registerContexts(ContextRegistry& reg)
{
	reg.registerContext("app",AppContext::contextCreator, PARAM_SINGLE_OF_NAME);
}

bool GlobalContext::onSetParam(const ConfigNode& node)
{
	return m_paramregistry.setParam(node, this);
}

void GlobalContext::registerParams(ParamRegistry<GlobalContext>& reg)
{
	reg.registerParam("sessiontimeout",&Context::setIgnore,PARAM_SINGLE_OF_TYPE);
	reg.registerParam("upload_dir",&Context::setIgnore,PARAM_SINGLE_OF_TYPE);
	reg.registerParam("listener",&GlobalContext::setListener,PARAM_NONE);
	reg.registerParam("num_threads", &GlobalContext::setNumThreads,PARAM_NONE);
	reg.registerParam("queue_depth", &GlobalContext::setQueueDepth,PARAM_NONE);
	reg.registerParam("uri_base", &Context::setIgnore,PARAM_REQUIRED|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("max_request_size", &Context::setIgnore,PARAM_SINGLE_OF_TYPE);
	reg.registerParam("max_file_size", &Context::setIgnore,PARAM_SINGLE_OF_TYPE);
	reg.registerParam("phys_base", &Context::setIgnore,PARAM_NONE);
}

bool GlobalContext::setGlobalParam(const std::string& type, const std::string& value)
{
	GlobalConfigNode* node = new GlobalConfigNode(type, value);
	m_globalNodes.push_back(node);
	return setParam(*node);
}

/*!
 * \fn container::GlobalContext::setListener(const ConfigNode& node)
 * Creates and registers an acceptor.
 * configNode should have protocol param (with value "unix" or "tcp"),
 * and either host & port params for tcp, or path param for unix protocols.
 */
bool GlobalContext::setListener(const ConfigNode& node)
{
	util::param_t::const_iterator param = node.getAttrs().find("protocol");
	if (param == node.getAttrs().end()) {
		//Ugh.. No protocol specified?
		std::cerr<<"Listener node specified without protocol specification"<<std::endl;
		return false;
	}
	std::string host, port, path, proto;
	try {
		RequestListener::Acceptor *acc;
		proto=param->second;
		if(proto=="tcp") {
			param = node.getAttrs().find("host");
			if(param == node.getAttrs().end()) {
				std::cerr<<"No host specified for tcp listener"<<std::endl;
				return false;
			}
			host=param->second;
			param = node.getAttrs().find("port");
			if(param == node.getAttrs().end()) {
				std::cerr<<"No port specified for tcp listener"<<std::endl;
				return false;
			}
			port = param->second;
			unsigned short uport=atoi(port.c_str());
			acc=new RequestListener::TCPAcceptor(host,uport);
		} else if(proto=="unix") {
			param = node.getAttrs().find("path");
			if(param == node.getAttrs().end()) {
				std::cerr<<"No path specifed for UNIX-domain listener"<<std::endl;
				return false;
			}
			path=param->second;
			acc=new RequestListener::UnixAcceptor(path);
		} else {
			//Ugh.. Not tcp, not unix..  What is it?
			std::cerr<<"Unknown listener type specified"<<std::endl;
			return false;
		}
		m_acceptors.push_back(acc);
	} catch (servlet::Traceable& e) {
		std::cerr<<"Unable to create listener for socket \""<<proto<<"://";
		if(proto == "tcp")
			std::cerr<<host<<':'<<port;
		else // For now we know that if it isn't tcp, it's Unix
			std::cerr<<path;
		std::cerr<<std::endl;
		e.printStackTrace(std::cerr);
		return false;
	} catch (std::exception& e) {
		std::cerr<<"Unable to create listener for socket \""<<proto<<"://";
		if(proto == "tcp")
			std::cerr<<host<<':'<<port;
		else // For now we know that if it isn't tcp, it's Unix
			std::cerr<<path;
		std::cerr<<std::endl;
		std::cerr<<"\tReason: "<<e.what()<<std::endl;
		return false;
	}
	return true;
}
/*!
 * \fn container::GlobalContext::setNumThreads(const ConfigNode& node)
 */
bool GlobalContext::setNumThreads(const ConfigNode& node)
{
	if(m_nThreads==-1) {
		m_nThreads=atoi(node.getAttrs().find("value")->second.c_str());
	}
	return true;
}

/*!
 * \fn container::GlobalContext::setQueueDepth(const ConfigNode& node)
 */
bool GlobalContext::setQueueDepth(const ConfigNode& node)
{
	if(m_queueDepth==-1) {
		m_queueDepth=atoi(node.getAttrs().find("value")->second.c_str());
	}
	return true;
}

class DoAccept: public std::unary_function<RequestListener::Acceptor*, void>
{
	RequestListener* m_listener;
public:
	DoAccept(RequestListener * listener)
		: m_listener(listener){}
	void operator()(RequestListener::Acceptor* acc)
	{
		m_listener->addAcceptor(acc);
	}
};

bool GlobalContext::onPostComplete()
{
	if(m_nThreads==-1)
		m_nThreads=15;
	if(m_queueDepth==-1)
		m_queueDepth=15;
	m_pool=new sptk::CThreadPool(m_nThreads, m_queueDepth);
	m_listener=new RequestListener(*m_pool);
	for_each(m_acceptors.begin(), m_acceptors.end(), DoAccept(m_listener));
	m_pool->start();
	return true;
}

ParamRegistry<GlobalContext> GlobalContext::m_paramregistry(GlobalContext::registerParams);
ContextRegistry GlobalContext::m_contextregistry(GlobalContext::registerContexts);

}
}
