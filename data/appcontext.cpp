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
#include <requesthandler.h>
#include <servlet/IllegalArgumentException.h>
#include <sptk4/CThread.h>
#include <sptk4/CBuffer.h>
#include <sptk4/CBase64.h>
#include <sptk4/CGuard.h>
#include <dlfcn.h>
#include <fcntl.h>

using namespace container;
namespace container {
namespace serverconfig {

// Helper class to remove servlets from the servlet map
class RHServletRemover
{
public:
	void operator()(const std::string& path)
	{
		RequestHandler::removeServlet(path);
	}
};

// Helper class template to iterate over all servlets in the map
template<class T>
 class SMFunctor
{
private:
	T* m_inst;
	typedef void (T::*F)(const std::string&);
	F m_f;
public:
	SMFunctor(T* inst, F f): m_inst(inst), m_f(f){}
	void operator()(const std::string& path)
	{
		(m_inst->*m_f)(path);
	}
};

AppContext::AppContext(const ConfigNode& node, Context* parent)
	: Context(parent->getServerConfig(), parent)
	, servlet::ServletContext()
	, m_sessionTimeout(600) // five minutes
	, m_uploadDir("/tmp") //hardcoded default
	, m_cache(false) // Don't let proxies cache anything generated by CPPSERV by default
#ifdef HAVE_LIBMAGIC
	, m_mime_cookie(0)
#endif
{
	m_paramregistry.getParamList(getUnsetParams());
	util::param_t attrs = node.getAttrs();
	util::param_t::const_iterator cacheIt = attrs.find("allow_cacheing");
	if(cacheIt != attrs.end())
	{
		std::string cacheVal(cacheIt->second);
		std::transform(cacheVal.begin(), cacheVal.end(), cacheVal.begin(), ::tolower);
		if(cacheVal == "true")
			m_cache=true;
	}
}

bool AppContext::onSetParam(const ConfigNode& node)
{
	return m_paramregistry.setParam(node, this);
}

AppContext::~AppContext()
{
	if(m_cleaner) {
		m_cleaner->stop();
		delete m_cleaner;
	}
	killAllSessions();
	std::string empty;
	destroyServlets(); // Calls destroy() on all servlets, before unloading them
	RHServletRemover rh;
	SMFunctor<AppContext> unl(this, &AppContext::unloadServlet);
	// This will block till all requests being processed by this servlet
	// are done. Alternatively we may want to kill all processes that are
	// in this servlet. Maybe there should be some timeout
	m_maptop.forEachServletPath(rh, empty);
	m_maptop.forEachServletPath(unl, empty);
#ifdef HAVE_LIBMAGIC
	if(m_mime_cookie)
		::magic_close(m_mime_cookie);
#endif
}

void AppContext::registerContexts(ContextRegistry& reg)
{
	reg.registerContext("servlet",serverconfig::ServletContext::contextCreator, 0);
	reg.registerContext("csp",CSPContext::contextCreator, 0);
}

void AppContext::registerParams(ParamRegistry<AppContext>& reg)
{
	reg.registerParam("sessiontimeout",&AppContext::setSessionTimeout,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("upload_dir",&AppContext::setUploadDir,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("parameter",&AppContext::addInitParam,0);
	reg.registerParam("max_request_size", &Context::setIgnore,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("max_file_size", &Context::setIgnore,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("uri_base", &AppContext::setUriBase,PARAM_INHERITABLE|PARAM_REQUIRED|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("phys_base", &AppContext::setPhysBase,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("default_mime_type", &AppContext::setDefaultMimeType,PARAM_SINGLE_OF_TYPE);
	reg.registerParam("default_character_encoding", &AppContext::setDefaultCharacterEncoding,PARAM_SINGLE_OF_TYPE);
}

bool AppContext::setSessionTimeout(const ConfigNode& val)
{
	util::param_t attrs = val.getAttrs();
	util::param_t::const_iterator value = attrs.find("value");
	if(attrs.end() == value) {
		std::cerr<<"Value missing for session timeout paramter"<<std::endl;
		return false;
	}
	m_sessionTimeout=::atoi(value->second.c_str());
	return true;
}

bool AppContext::setUploadDir(const ConfigNode& val)
{
	return setString(val, m_uploadDir);
}

bool AppContext::setPhysBase(const ConfigNode& val)
{
	return setString(val, m_physbase);
}

bool AppContext::setUriBase(const ConfigNode& val)
{
	return setString(val, m_uribase);
}

bool AppContext::setDefaultMimeType(const ConfigNode& val)
{
	return setString(val, m_mime);
}

bool AppContext::setDefaultCharacterEncoding(const ConfigNode& val)
{
	return setString(val, m_enc);
}

bool AppContext::addInitParam(const ConfigNode& val)
{
	util::param_t::const_iterator name=val.getAttrs().find("name");
	util::param_t::const_iterator value=val.getAttrs().find("value");
	if(value == val.getAttrs().end() || name == val.getAttrs().end()) {
		std::cerr<<"Application paramter is missing name or value"<<std::endl;
		return false;
	}
	m_init_params.push_back(std::pair<std::string,std::string>(name->second,value->second));
	return true;
}

bool AppContext::onPreComplete()
{
	// URI base MUST end with '/'
	if(m_uribase.empty())
		m_uribase="/";
	else if(*m_uribase.rbegin()!='/')
		m_uribase+='/';
	m_sessionCookieName=makeSName(getName());
	if(m_mime.empty())
		m_mime="text/html";
	if(m_enc.empty())
		m_enc="utf8";
	for(util::pairlist_t::iterator it=m_init_params.begin(); it!=m_init_params.end(); it++) {
		addInitParam(it->first,it->second);
	}
	getGlobalContext()->registerApp(getName(), this);
	return true;
}

bool AppContext::onPostComplete()
{
	m_fileSaveTemplate=m_uploadDir+'/'+"cppserv.tmpXXXXXX";
	initServlets();
	m_cleaner=new SessionCleaner(this,m_sessionTimeout/2+1); // +1, so that it doesn't become 0...
	m_cleaner->run();
	return true;
}

ParamRegistry<AppContext> AppContext::m_paramregistry(AppContext::registerParams);
ContextRegistry AppContext::m_contextregistry(AppContext::registerContexts);

ServletConfigImpl* AppContext::addServlet(const std::string& path, const std::string& name, const std::string& dso, bool hidden, size_t maxRequestSize, size_t maxFileSize)
{
	std::string fullpath("/");
	if(!getServletContextName().empty()) { // avoid starting with "//"
		fullpath.append(getServletContextName());
		fullpath+='/';
	}
	fullpath.append(path);
	ServletDesc* desc = getDesc(fullpath);
	if(desc) { // Another servlet is already configured for the path in this app...
		std::cerr<<"Path "<<fullpath<<" is already used"<<std::endl;
		return 0;
	}
	void* dsoHandle=dlopen(dso.c_str(),RTLD_LAZY);
	if(!dsoHandle) {
		std::cerr<<"Error loading library \""<<dso<<"\": "<<dlerror()<<std::endl;
		return 0;
	}
	servletcreatefunc_t createFunc=(servletcreatefunc_t)dlsym(dsoHandle,(name + "_createServlet").c_str());
	if(!createFunc) {
		std::cerr<<"Could not locate servlet "<<name<<" in the library "<<dso<<": "<<dlerror()<<std::endl;
		return 0;
	}
	ServletConfigImpl* config=new ServletConfigImpl(this, name);
	desc=new ServletDesc(createFunc(), config, dsoHandle, fullpath, maxRequestSize, maxFileSize, m_mime, m_enc, m_cache);
	m_maptop[fullpath]=desc;
	if(!hidden) {
		if(!RequestHandler::addServlet(fullpath,getServletContainer(fullpath))) { // Another such URL exists globally
			unloadServlet(fullpath);
			delServlet(fullpath);
			return 0;
		}
	}
	return config;
}

AppContext::ServletDesc* AppContext::getDesc(const std::string& path)
{
	return m_maptop.getServletDesc(path);
}

void AppContext::delServlet(const std::string& path)
{
	m_maptop.removeDesc(path);
}

ServletContainer* AppContext::getServletContainer(const std::string& path)
{
	ServletDesc* desc = getDesc(path);
	if(!desc)
		return 0;
	return &(desc->m_cont);
}

void AppContext::splitServPath(const std::string& path, std::string& dir, std::string& name)
{
	std::string::size_type slash = path.rfind('/');
	if(slash == std::string::npos) {
		dir = "/";
		name = path;
	} else {
		dir.assign(path.substr(0, slash));
		name.assign(path.substr(slash + 1));
	}
}

void AppContext::initServlet(const std::string& path)
{
	ServletContainer* cont=getServletContainer(path);
	cont->init();
}

void AppContext::destroyServlet(const std::string& path)
{
	ServletContainer* cont=getServletContainer(path);
	cont->destroy();
}

/*!
 * \fn AppContext::unloadServlet(const std::string& name)
 * @param name Name of servlet to unload, as specified in name= parameter in config file
 * Stops servlet - that is removed it from mapper, and waits till all 
 * requests within this servlet are done.
 */
void AppContext::unloadServlet(const std::string& path)
{
	ServletContainer* cont=getServletContainer(path);
	servlet::ServletConfig* conf=cont->getConfig();
	ServletDesc* d=getDesc(path);
	void* dsoHandle=d->m_h;
	delete d; // this will delete ServletContainer, which will in turn delete servlet object
	delete conf;
	::dlclose(dsoHandle);
}

void AppContext::initServlets()
{
	std::string empty;
	SMFunctor<AppContext> init(this, &AppContext::initServlet);
	m_maptop.forEachServletPath(init, empty);
}

void AppContext::destroyServlets()
{
	std::string empty;
	SMFunctor<AppContext> destroy(this, &AppContext::destroyServlet);
	m_maptop.forEachServletPath(destroy, empty);
}


/*!
    \fn AppContext::getSession(const std::string& sid, bool create)
	@param sid Session ID. If empty string, new session is created
	@param create Whether to create session if it doesn't exist
	@return Pointer to the session.
	@note sets session state to not-new when sid is not empty
		This presumes session is only retrieved from Context once per request
 */
container::HttpSessionImpl* AppContext::getSession(const std::string& sid, bool create)
{
	sptk::CGuard guard(m_sessionLock);
	if(sid.empty()) {
		container::HttpSessionImpl *s = 0;
		if(create)
			s = newSession();
		return s;
	}
	sessionlist_t::iterator s=m_sessions.find(sid);
	if(s==m_sessions.end()||!s->second->validP()){
		HttpSessionImpl* s = 0;
		if(create)
			s = newSession();
		return s;
	}
	s->second->notNew();
	return s->second;
}


/*!
    \fn AppContext::newSession()
	This function must always be called with sessionLock locked.
 */
container::HttpSessionImpl* AppContext::newSession()
{
	std::string sid=container::util::getRandomString(10);
	HttpSessionImpl* s=new HttpSessionImpl(*this,sid,m_sessionTimeout);
	m_sessions[sid]=s;
	return s;
}


/*!
    \fn AppContext::killSession(const std::string& sid)
	@param sid ID of session to kill
	@note This function should always be called with sessionLock held.
 */
void AppContext::killSession(const std::string& sid)
{
	sessionlist_t::iterator s=m_sessions.find(sid);
	if(s==m_sessions.end()){
		return;
	}
	delete s->second;
	m_sessions.erase(sid);
}


/*!
    \fn AppContext::cleanSessions()
 */
void AppContext::cleanSessions()
{
	sptk::CGuard guard(m_sessionLock);
	sessionlist_t::iterator cur=m_sessions.begin();
	while(cur!=m_sessions.end()){
		if(!cur->second->validP()){
			std::string sid = cur->first;
			cur++;
			killSession(sid);
		} else {
			cur++;
		}
	}
}
/**
	\fn AppContext::SessionCleaner::SessionCleaner(AppContext* ctx,int freq)
	Constructs session cleaner object. Opens a session cleaner trigger pipe.
 */
AppContext::SessionCleaner::SessionCleaner(AppContext* ctx,int freq)
	: sptk::CThread("SessionCleaner",true)
	, m_running(true)
	, m_freq(freq)
	, m_ctx(ctx)
{
	if(::pipe(m_trigger)==-1) {
		char errbuf[1024];
		const char* err = ::strerror_r(errno, errbuf, sizeof(errbuf));
		std::cerr<<"Error setting up session cleaner trigger: "<<err<<std::endl;
		::exit(1);
	}
	if(::fcntl(m_trigger[0],F_SETFL,O_NONBLOCK)<0) {
		char errbuf[1024];
		const char* err = ::strerror_r(errno, errbuf, sizeof(errbuf));
		std::cerr<<"Error setting trigger reader to non-blocking mode: "
			<<err<<std::endl;
		::exit(1);
	}
}
/**
	\fn AppContext::SessionCleaner::~SessionCleaner()
 */
AppContext::SessionCleaner::~SessionCleaner()
{
	::close(m_trigger[0]);
	::close(m_trigger[1]);
}
void AppContext::SessionCleaner::threadFunction()
{
	while(m_running) {
		m_ctx->cleanSessions();
		struct timeval timeout={m_freq,0};
		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(m_trigger[0],&rfd);
		int r=::select(m_trigger[0]+1,&rfd,0,0,&timeout);
		if(r){
			char buf[4];
			while(::read(m_trigger[0],buf,sizeof(buf))>0){;}
		}
	}
}
/**
	\fn AppContext::SessionCleaner::stop()
	Politely stops session cleaner thread by telling it it should
	no longer run, and triggering it. If there is cleaning in progress
	it will not be interrupted.
 */
void AppContext::SessionCleaner::stop()
{
	m_running=false;
	this->wakeUp();
	this->terminate();
}

/*!
 * \fn AppContext::isSessionValid(const std::string& sid)
 * @param sid Session ID, as retrieved from session cookie.
 * Session is valid if it exists and did not expire.
 */
bool AppContext::isSessionValid(const std::string& sid)
{
	sessionlist_t::iterator s=m_sessions.find(sid);
	return s!=m_sessions.end()&&s->second->validP();
}


/*!
 * \fn AppContext::addInitParam(const std::string& name, const std::string& value)
 */
void AppContext::addInitParam(const std::string& name, const std::string& value)
{
	m_params[name]=value;
}

/*!
 * \fn AppContext::getServletContextName() const
 * @return Contest's name (which is same as application name)
 */
std::string AppContext::getServletContextName() const
{
	return getName();
}

void AppContext::log(const std::string& msg) const
{
	std::cerr<<msg<<std::endl;
}

void AppContext::log(const std::string& message, const std::exception& e) const 
{
	log(message);
	std::cerr<<e.what()<<std::endl;
}


/*!
 * \fn AppContext::SessionCleaner::wakeUp()
 * Wakes up session cleaning thread.
 */
void AppContext::SessionCleaner::wakeUp()
{
	char c='1';
	::write(m_trigger[1],&c,1);
}


/*!
 * \fn AppContext::killAllSessions()
 * Removes all sessions in this context.
 * It presumes no servlets in this context are going
 * to use us.
 */
void AppContext::killAllSessions()
{
	sptk::CGuard guard(m_sessionLock);
	sessionlist_t::iterator cur=m_sessions.begin();
	while(cur!=m_sessions.end()){
		delete cur->second;
		cur++;
	}
	m_sessions.clear();
}

std::string AppContext::makeSName(const std::string& name)
{
	sptk::CBuffer in(name.c_str());
	sptk::CBuffer out(name.length()+5);
	std::string ret="CPPSERV";
	::memset(out.data(), 0, out.size());
	sptk::CBase64::encode(out,in);
	ret+=out.data();
	std::transform(ret.begin(),ret.end(),ret.begin(),container::util::TrEq());
	return ret;
}


servlet::ServletContext* AppContext::getContext(const std::string& uripath)
{
	std::string uribase = getUriBase();
	if(uripath.find(uribase) != 0)
		return 0;
	std::string contextName = uripath.substr(uribase.length());
	return getServerConfig().getApp(contextName);
}
int AppContext::getMajorVersion()
{
	return VERSION_MAJOR;
}
int AppContext::getMinorVersion()
{
	return VERSION_MINOR;
}
std::string AppContext::getMimeType(const std::string& file)
{
	std::string ret;
#ifdef HAVE_LIBMAGIC
	if(!m_mime_cookie)
	{
		m_mime_cookie=::magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_ERROR);
		if(!m_mime_cookie)
		{
			std::cerr<<"Error initializing MIME magic library"<<std::endl;
			return ret;
		}
		if(::magic_load(m_mime_cookie, 0)!=0) // Fail!
		{
			std::cerr<<"Error loading MIME magic database: "<<::magic_error(m_mime_cookie)<<std::endl;
			return ret;
		}
	}
	const char* cstr_ret = ::magic_file(m_mime_cookie, file.c_str());
	if(!cstr_ret)
	{
		std::cerr<<"Error determining MIME type of file \""<<file<<"\": "<<::magic_error(m_mime_cookie)<<std::endl;
		return ret;
	}
	ret = cstr_ret;
#endif
	return ret;
}
std::auto_ptr< std::set<std::string> > AppContext::getResourcePaths(const std::string& path)
{
	std::auto_ptr< std::set<std::string> > ret(new std::set<std::string>);
	// For now we can only return list of available Servlets
	// Which means our implementation is not really complete
	servletmap_t* sp = m_maptop.getPath(path, false);
	if(!sp)
		return ret;
	std::insert_iterator<std::set<std::string> > ii(*ret, ret->begin());
	std::transform(sp->getSubpaths().begin(), sp->getSubpaths().end(), ii, __gnu_cxx::select1st<std::pair<std::string, ServletMap<ServletDesc>*> >());
	std::transform(sp->getServlets().begin(), sp->getServlets().end(), ii, __gnu_cxx::select1st<std::pair<std::string, ServletDesc*> >());
	return ret;
}
std::string AppContext::getResource(const std::string& /*path*/)
{
	//throw servlet::ServletException("Method unimplemented");
	return "";
}
std::istream& AppContext::getResourceAsStream(const std::string& /*path*/)
{
	throw servlet::ServletException("Method unimplemented");
}
servlet::RequestDispatcher* AppContext::getRequestDispatcher(const std::string& path)
{
	if(path[0]!='/')
		throw servlet::IllegalArgumentException();
	std::string name(path.substr(1));
	return getServletContainer(name);
}
servlet::RequestDispatcher* AppContext::getNamedDispatcher(const std::string& name)
{
	return getServletContainer(name);
}
std::string AppContext::getRealPath(const std::string& path)
{
	//TODO: use TR2 filesystem interface, when available
	//for now completely non-portable
	std::string ret = m_physbase;
	if(path.empty())
		return "";
	if(path[0]!='/')
		ret += '/';
	ret+=path;
	return ret;
}
std::string AppContext::getServerInfo()
{
	return "CPPSERV/" CPPSERV_VERSION;
}
std::string AppContext::getInitParameter(const std::string& name)
{
	param_t::const_iterator iRet = m_params.find(name);
	if(iRet == m_params.end())
	    return "";
	else
	    return iRet->second;
}
std::auto_ptr< std::vector<std::string> > AppContext::getInitParameterNames()
{
	return util::getMapKeyNames(m_params);
}
boost::shared_ptr<void> AppContext::getAttribute(const std::string& name)
{
	attr_t::const_iterator iRet = m_attrs.find(name);
	if(iRet == m_attrs.end())
	    return boost::shared_ptr<void>();
	else
	    return iRet->second;
}
std::auto_ptr< std::vector<std::string> > AppContext::getAttributeNames()
{
	return util::getMapKeyNames(m_attrs);
}
void AppContext::setAttribute(const std::string& name, boost::shared_ptr<void> object)
{
	m_attrs[name] = object;
}
void AppContext::removeAttribute(const std::string& name)
{
	attr_t::iterator it=m_attrs.find(name);
	if(it!=m_attrs.end())
	    m_attrs.erase(it);
}
bool AppContext::hasAttribute(const std::string& name) const
{
	attr_t::const_iterator it=m_attrs.find(name);
	return it!=m_attrs.end();
}

}
}
