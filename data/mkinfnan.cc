//  mkinfnan.cc -- create architecture dependent is_inf and is_nan.
//  Copyright (C) 2002--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.

//  If any of std::isinf, std::isnan and std::finite are missing (they
//  are not standard C++), create these functions using comparison
//  with reinterpret_cast.

#include <iostream>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <limits>
#include <more/bits/conf.h>

#ifdef MORE_CONF_CXX_HAVE_STD_ISINF
    bool have_std_isinf = true;
#else
    bool have_std_isinf = false;
#endif

#ifdef MORE_CONF_CXX_HAVE_STD_ISNAN
    bool have_std_isnan = true;
#else
    bool have_std_isnan = false;
#endif

#ifdef MORE_CONF_CXX_HAVE_STD_FINITE
    bool have_std_finite = true;
#else
    bool have_std_finite = false;
#endif

char const* str_inline = "inline ";

template <typename Real>
void
make_function(Real x, char const* tname, char const* fname)
{
    typedef unsigned int word_t;
    char const* word_t_str = "unsigned int";
    std::cout << str_inline
	      << "bool\n" << fname << "(" << tname << " const& x)\n";
    std::cout << "{\n    return ";
    for (std::size_t i = 0; i < sizeof(Real)/sizeof(word_t); ++i) {
	if (i != 0)
	    std::cout << "\n        && ";
	std::cout << "((" << word_t_str << "*)&x)[" << i << "] == "
		  << ((word_t*)&x)[i] << "u";
    }
    std::cout << ";\n}\n\n";
}

template <typename Real>
bool
eq(Real const& x, Real const& y)
{
    typedef unsigned int word_t;
    for (std::size_t i = 0; i < sizeof(Real)/sizeof(word_t); ++i)
	if (((word_t*)&x)[i] != ((word_t*)&y)[i])
	    return false;
    return true;
}

template <typename Real>
void
make_for_type(char const* tname)
{
    Real zero;
    std::istringstream iss("0");
    iss >> zero;

    Real x_pl_inf = 1;
    x_pl_inf /= zero;
    Real x_mi_inf = -1;
    x_mi_inf /= zero;
    Real x_nan = 0;
    x_nan /= zero;

    if (have_std_isinf)
	std::cout << "inline bool is_inf(" << tname << " const& x)\n{\n"
		  << "    return std::isinf(x);\n}\n\n";
    else if (x_pl_inf <= std::numeric_limits<Real>::max() ||
	     x_mi_inf >= std::numeric_limits<Real>::min())
	std::cout << "inline bool is_inf(" << tname << " const& x)\n{\n"
		  << "    return false;\n}\n\n";
    else if (have_std_finite && have_std_isnan)
	std::cout << "inline bool is_inf(" << tname << " const& x)\n{\n"
		  << "    return !std::isnan(x) && !std::finite(x);\n}\n\n";
    else {
	if (eq(x_pl_inf, x_mi_inf)) {
// 	    std::cout << "// #undef MORE_PM_INF_" << tsym << "_ARE_DISTINCT\n";
	    make_function(x_pl_inf, tname, "is_inf");
// 	std::cout << str_inline << "bool\nis_plus_inf(" << tname << ") "
// 		  << "{ return false; }\n\n";
// 	std::cout << str_inline << "bool\nis_minus_inf(" << tname << ") "
// 		  << "{ return false; }\n\n";
	}
	else {
// 	    std::cout << "#define MORE_PM_INF_" << tsym << "_ARE_DISTINCT 1\n";
	    make_function(x_pl_inf, tname, "is_plus_inf");
	    make_function(x_mi_inf, tname, "is_minus_inf");
	    std::cout << str_inline << "bool\nis_inf(" << tname << " const& x) "
		      << "{ return is_plus_inf(x) || is_minus_inf(x); }\n\n";
	}
    }
    if (have_std_isnan)
	std::cout << "inline bool is_nan(" << tname << " const& x)\n{\n"
		  << "    return std::isnan(x);\n}\n\n";
    else if (x_nan <= std::numeric_limits<Real>::max() ||
	     x_nan >= std::numeric_limits<Real>::min())
	std::cout << "inline bool is_nan(" << tname << " const& x)\n{\n"
		  << "    return false;\n}\n\n";
    else
	make_function(x_nan, tname, "is_nan");

    std::cout << "inline bool is_finite(" << tname << " const& x)\n{\n";
    if (have_std_finite)
	std::cout << "    return std::finite(x);\n}\n\n";
    else
	std::cout << "    return !is_inf(x) && !is_nan(x);\n}\n\n";
}

int
main()
{
    std::signal(SIGFPE, SIG_IGN);
    std::cout << "// This is a generated file.\n\n"
	      << "#ifndef MORE_BITS_INFNAN_H\n"
	      << "#define MORE_BITS_INFNAN_H\n\n"
	      << "#include <cmath>\n\n"
	      << "namespace more {\nnamespace math {\n\n";
    make_for_type<float>("float");
    make_for_type<double>("double");
    make_for_type<long double>("long double");
    std::cout << "}} // namespace more::math\n"
	      << "#endif // defined(MORE_BITS_INFNAN_H)\n";
    return 0;
}
