#include <servlet/taglib/CompileTimeTaglib.h>
#include <servlet/taglib/Generator.h>
#include <string>
#include <sstream>
using namespace servlet::taglib;
COMPILE_TIME_TAGLIB(test)

class HeadTag: public Generator
{
public:
	HeadTag(const std::string& name)
		: Generator(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void HeadTag::doStartTag(const Generator::attribs_t& attribs)
{
	std::string title;
	attribs_t::const_iterator it = attribs.find("title");
	if(it!=attribs.end())
		title=it->second;
	std::stringstream s;
	s<<"\"<HTML><HEAD><TITLE>"<<title<<"</TITLE></HEAD><BODY>\\n\"";
	*body<<"out.write("<<s.str()<<", "<<s.str().length()-2<<");\n";
}
void HeadTag::doEndTag()
{
}

EXPORT_COMPILE_TIME_TAG(test, head, HeadTag)

class FootTag: public Generator
{
public:
	FootTag(const std::string& name)
		: Generator(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void FootTag::doStartTag(const Generator::attribs_t& attribs)
{
	std::stringstream s;
	s<<"\"</BODY></HTML>\\n\"";
	*body<<"out.write("<<s.str()<<", "<<s.str().length()-2<<");\n";
}
void FootTag::doEndTag()
{
}

EXPORT_COMPILE_TIME_TAG(test, foot, FootTag)

class BlockTag: public Generator
{
public:
	BlockTag(const std::string& name)
		: Generator(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void BlockTag::doStartTag(const Generator::attribs_t& attribs)
{
	std::stringstream s;
	s<<"\"<br>body tag start<br><hr>\\n\"";
	*body<<"out.write("<<s.str()<<", "<<s.str().length()-2<<");\n";
}
void BlockTag::doEndTag()
{
	std::stringstream s;
	s<<"\"<hr><br>body tag end<br>\\n\"";
	*body<<"out.write("<<s.str()<<", "<<s.str().length()-2<<");\n";
}


EXPORT_COMPILE_TIME_TAG(test, block, BlockTag)
