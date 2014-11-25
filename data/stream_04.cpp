/**
 *  @example mirror/example/stream_04.cpp
 *  This example shows the registering macros, container reflection
 *  and wrapper classes which allow to write instances of arbitrary
 *  reflectible class to standard output streams in the XML format
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/list.hpp>
#include <mirror/pre_registered/class/boost/optional.hpp>
#include <mirror/pre_registered/class/boost/variant.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <mirror/stream/xml.hpp>
#include <iostream>

namespace inet {

enum class protocol
{
	http, https, ftp
};

struct ip_v4_address
{
	unsigned char a,b,c,d;
};

struct parameter
{
	std::string name;
	std::string value;
};

struct url
{
	protocol scheme;

	boost::variant<
		ip_v4_address,
		std::string
	> host;
	boost::optional<unsigned short> port;

	std::string path;
	std::list<parameter> parameters;
};

struct bookmark
{
	std::string title;
	url location;
};

} // namespace inet

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(inet)

MIRROR_QREG_ENUM(
	inet, protocol,
	(http)(https)(ftp)
)

MIRROR_QREG_POD_CLASS(
	struct, inet, ip_v4_address,
	(a)(b)(c)(d)
)

MIRROR_QREG_POD_CLASS(
	struct, inet, parameter,
	(name)(value)
)

MIRROR_QREG_POD_CLASS(
	struct, inet, url,
	(scheme)(host)(port)(path)(parameters)
)

MIRROR_QREG_POD_CLASS(
	struct, inet, bookmark,
	(title)(location)
)

MIRROR_REG_END

int main(void)
{
	using namespace mirror;
	//
	boost::none_t none = boost::none_t();
	inet::ip_v4_address localhost = {127,0,0,1};
	inet::ip_v4_address router_ip = {192,168,1,254};
	//
	std::list<inet::bookmark> bookmarks = {
	{
		"Boost C++ libraries",
		{
			inet::protocol::http,
			std::string("www.boost.org"), none,
			{}, {}
		}
	},
	{
		"Google search: C++ reflection",
		{
			inet::protocol::http,
			std::string("www.google.com"), none,
			{}, {{"hl", "sk"},{"source", "hp"},{"q","C%2B%2B+reflection"}}
		}
	},
	{
		"Facebook Login",
		{
			inet::protocol::https,
			std::string("www.facebook.com"), none,
			"login.php", {}
		}
	},
	{
		"Tomcat admin interface",
		{
			inet::protocol::http, localhost, 8080,
			"manager/html/list", {}
		}
	},
	{
		"Router admin interface",
		{
			inet::protocol::https, router_ip, 8443,
			"main.php", {{"login","administrator"}}
		}
	}
	};
	//
	auto name_maker = [](std::ostream& out){out << "bookmarks";};
	// write in XML format
	std::cout << stream::to_xml::from(bookmarks, name_maker) << std::endl;
	//
	return 0;
}

/* Example of output
 | <bookmarks>
 |     <bookmark>
 |         <title>Boost C++ libraries<title/>
 |         <location>
 |             <scheme>http<scheme/>
 |             <host>www.boost.org<host/>
 |             <port><port/>
 |             <path><path/>
 |             <parameters><parameters/>
 |         <location/>
 |     <bookmark/>
 |     <bookmark>
 |         <title>Google search: C++ reflection<title/>
 |         <location>
 |             <scheme>http<scheme/>
 |             <host>www.google.com<host/>
 |             <port><port/>
 |             <path><path/>
 |             <parameters>
 |                 <parameter>
 |                     <name>hl<name/>
 |                     <value>sk<value/>
 |                 <parameter/>
 |                 <parameter>
 |                     <name>source<name/>
 |                     <value>hp<value/>
 |                 <parameter/>
 |                 <parameter>
 |                     <name>q<name/>
 |                     <value>C%2B%2B+reflection<value/>
 |                 <parameter/>
 |             <parameters/>
 |         <location/>
 |     <bookmark/>
 |     <bookmark>
 |         <title>Facebook Login<title/>
 |         <location>
 |             <scheme>https<scheme/>
 |             <host>www.facebook.com<host/>
 |             <port><port/>
 |             <path>login.php<path/>
 |             <parameters><parameters/>
 |         <location/>
 |     <bookmark/>
 |     <bookmark>
 |         <title>Tomcat admin interface<title/>
 |         <location>
 |             <scheme>http<scheme/>
 |             <host>
 |                     <a>127<a/>
 |                     <b>0<b/>
 |                     <c>0<c/>
 |                     <d>1<d/>
 |                 <host/>
 |             <port>8080<port/>
 |             <path>manager/html/list<path/>
 |             <parameters><parameters/>
 |         <location/>
 |     <bookmark/>
 |     <bookmark>
 |         <title>Router admin interface<title/>
 |         <location>
 |             <scheme>https<scheme/>
 |             <host>
 |                     <a>192<a/>
 |                     <b>168<b/>
 |                     <c>1<c/>
 |                     <d>254<d/>
 |                 <host/>
 |             <port>8443<port/>
 |             <path>main.php<path/>
 |             <parameters>
 |                 <parameter>
 |                     <name>login<name/>
 |                     <value>administrator<value/>
 |                 <parameter/>
 |             <parameters/>
 |         <location/>
 |     <bookmark/>
 | <bookmarks/>
 */
