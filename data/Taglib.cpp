#include <servlet/taglib/Taglib.h>

namespace servlet
{
namespace taglib
{
Taglib::Taglib(const std::string& prefix)
	: m_prefix(prefix)
{}
Taglib::~Taglib()
{}

const std::string& Taglib::getPrefix() const
{
	return m_prefix;
}

}
}
