#include "RunTimeTaglib.h"

#include <stdexcept>

namespace servlet
{
namespace taglib
{

RunTimeTaglib::RunTimeTaglib(const std::string& prefix, const attribs_t& attribs)
	: Taglib(prefix)
{
	attribs_t::const_iterator it = attribs.find("prefix");
	if(it == attribs.end())
		throw std::runtime_error("No prefix specified for run-time taglib");
	setPrefix(it->second);
	it = attribs.find("uri");
	if(it == attribs.end())
		throw std::runtime_error("No URI specified for run-time taglib");
	m_uri=it->second;
}

Generator* RunTimeTaglib::getTag(const std::string& name)
{
	throw std::runtime_error("TODO: implement RunTimeTaglib::getTag");
	return 0;
}
void RunTimeTaglib::releaseTag(Generator*)
{
	throw std::runtime_error(std::string("TODO: implement ")+__PRETTY_FUNCTION__);
}
RunTimeTaglib::RunTimeTagGenerator::RunTimeTagGenerator(const std::string& prefix)
	: Generator(prefix)
	, m_name(prefix)
{
}
RunTimeTaglib::RunTimeTagGenerator::~RunTimeTagGenerator()
{
}
void RunTimeTaglib::RunTimeTagGenerator::doStartTag(const attribs_t& attribs)
{
	throw std::runtime_error("TODO: implement RunTimeTaglib::RunTimeTagGenerator::doStartTag");
}
void RunTimeTaglib::RunTimeTagGenerator::doEndTag()
{
	throw std::runtime_error("TODO: implement RunTimeTaglib::RunTimeTagGenerator::doEndTag");
}

} // namespace taglib
} // namespace servlet

EXPORT_TAGLIB(cxxsp_run_time_taglib,servlet::taglib::RunTimeTaglib)
