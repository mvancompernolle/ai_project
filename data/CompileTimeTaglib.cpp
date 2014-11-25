#include <servlet/taglib/CompileTimeTaglib.h>
#include <servlet/taglib/Generator.h>
#include <stdexcept>
#include <iostream>
namespace servlet
{
namespace taglib
{

CompileTimeTaglib::CompileTimeTaglib(const std::string& prefix)
	: Taglib(prefix)
{
}

CompileTimeTaglib::TagFactoryBase::~TagFactoryBase()
{
}

Generator* CompileTimeTaglib::getTag(const std::string& name)
{
	tagfactorymap_t::iterator f = m_factoryMap.find(name);
	if(f == m_factoryMap.end())
		throw std::runtime_error("Generator for tag \""+name+"\" is not registered");
	std::string fullname = getPrefix()+":"+name;
	return f->second->makeTag(fullname);
}

void CompileTimeTaglib::releaseTag(Generator* tag)
{
	delete tag;
}

void CompileTimeTaglib::registerTag(const std::string& name, TagFactoryBase* factory)
{
	m_factoryMap[name] = factory;
}

} // namespace taglib
} // namespace servlet
