#include <more/bits/conf.h>
#include <iostream>
#include <iomanip>
#include <limits>

template<typename T>
  void mkprecuint(const char* s, const char* s_uc) {
      bool found = true;
      if (std::numeric_limits<T>::digits <=
	  std::numeric_limits<unsigned short>::digits)
	  std::cout << "typedef unsigned short " << s << "_prec_uint;\n";
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<unsigned int>::digits)
	  std::cout << "typedef unsigned int " << s << "_prec_uint;\n";
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<unsigned long>::digits)
	  std::cout << "typedef unsigned long " << s << "_prec_uint;\n";
#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<unsigned long long>::digits)
	  std::cout << "typedef unsigned long long " << s << "_prec_uint;\n";
#endif
      else found = false;
      if (found)
	  std::cout << "#define MORE_HAVE_" << s_uc << "_PREC_UINT 1\n";
      else
	  std::cout << "/* #undef MORE_HAVE_" << s_uc << "_PREC_UINT */\n";
  }

template<typename T>
  void mkprecint(const char* s, const char* s_uc) {
      bool found = true;
      if (std::numeric_limits<T>::digits <=
	  std::numeric_limits<short>::digits)
	  std::cout << "typedef short " << s << "_prec_int;\n";
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<int>::digits)
	  std::cout << "typedef int " << s << "_prec_int;\n";
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<long>::digits)
	  std::cout << "typedef long " << s << "_prec_int;\n";
#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
      else if (std::numeric_limits<T>::digits <=
	       std::numeric_limits<long long>::digits)
	  std::cout << "typedef long long " << s << "_prec_int;\n";
#endif
      else found = false;
      if (found)
	  std::cout << "#define MORE_HAVE_" << s_uc << "_PREC_INT 1\n";
      else
	  std::cout << "/* #undef MORE_HAVE_" << s_uc << "_PREC_INT */\n";
  }


int
main()
{
    std::cout << "/* This is a generated file. */\n";
#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
    std::cout << "/* The compiler supports the long long type. */\n\n";
#endif
    std::cout << "namespace more {\n";
    mkprecuint<float>("float", "FLOAT");
    mkprecuint<double>("double", "DOUBLE");
    mkprecuint<long double>("long_double", "LONG_DOUBLE");
    mkprecint<float>("float", "FLOAT");
    mkprecint<double>("double", "DOUBLE");
    mkprecint<long double>("long_double", "LONG_DOUBLE");
    std::cout << "}\n";
    return 0;
}
