#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <utility>
#include <cassert>
#include <cstdlib>
#include <algorithm>

using std::cout;
using std::cerr;
using std::endl;

// This file generates the header file mk.h which automatically defines
// the constants m, k, k1 and k2 from the macro LIBECC_M.

struct poly {
  unsigned int m;
  unsigned int k;
  unsigned int k1;
  unsigned int k2;
  poly(unsigned int _m, unsigned int _k, unsigned int _k1 = 0, unsigned int _k2 = 0) : m(_m), k(_k), k1(_k1), k2(_k2) { }
};

bool operator<(poly const& p1, poly const& p2) { return p1.m < p2.m; }

int main(int argc, char* argv[])
{
  std::ifstream trinomials;
  if (argc == 2)
    trinomials.open((std::string(argv[1]) + "/TRINOMIALS").c_str());   
 
  if (argc != 2 || !trinomials)
  {
    cerr << "Usage: " << argv[0] << " pathto/TRINOMIALS" << endl;
    return 1;
  }

  unsigned int m = 0;
  unsigned int k = 0;	// Initialize to avoid compiler warning.

  std::vector<poly> mk;

  std::string line;
  while(getline(trinomials, line))
  {
    int pos = 0;
    while(std::isdigit(line[pos]))
      ++pos;
    if (pos == 0 || !std::isspace(line[pos]))
      continue;
    unsigned int next_m = atoi(line.data());
    while(std::isspace(line[pos]))
      ++pos;
    if (!std::isdigit(line[pos]))
      continue;
    unsigned int next_k = atoi(line.data() + pos);
    if (next_m != m)
    {
      if (m >= 2)
	mk.push_back(poly(m, k));
      k = 0;
    }
    m = next_m;
    if (k == 0 || (k < 8 * sizeof(unsigned long) && next_k > 8 * sizeof(unsigned long)))
      k = next_k;
  }
  mk.push_back(poly(m, k));
  trinomials.close();

  std::ifstream pentanomials;
  if (argc == 2)
    pentanomials.open((std::string(argv[1]) + "/PENTANOMIALS").c_str());   

  unsigned int k1, k2;

  while(getline(pentanomials, line))
  {
    if (!std::isdigit(line[0]))
      continue;
    int pos = 0;
    for (;;)
    {
      m = atoi(line.data() + pos);
      while(std::isdigit(line[pos]))
	++pos;
      assert(line[pos] == ',');
      ++pos;
      k = atoi(line.data() + pos);
      k1 = k2 = 0;
      while(std::isdigit(line[pos]))
	++pos;
      if (line[pos] == ',')
      {
        ++pos;
	k1 = atoi(line.data() + pos);
	while(std::isdigit(line[pos]))
	  ++pos;
	assert(line[pos] == ',');
        ++pos;
	k2 = atoi(line.data() + pos);
	while(std::isdigit(line[pos]))
	  ++pos;
        mk.push_back(poly(m, k, k1, k2));
      }
      else if (m >= 1200)
        mk.push_back(poly(m, k));
      if (line[pos] == 0)
	break;
      assert(line[pos] == ' ');
      ++pos;
    }
  }
  pentanomials.close();

  std::sort(mk.begin(), mk.end());
  // Sanity check.
  unsigned int next = 2;
  for (std::vector<poly>::iterator iter = mk.begin(); iter != mk.end(); ++iter)
  {
    if (iter->m != next)
    {
      cout << "Unexpected: " << iter->m << endl;
      exit(1);
    }
    ++next;
  }
  assert(mk.size() == 9999);
 
  std::ofstream mkheader;
  mkheader.open("mk.h");

  mkheader << "#ifndef MK_H\n"
              "#define MK_H\n\n"
              "// Automatically define m, k, k1 and k2 as function of LIBECC_M.\n"
              "// This header was automatically generated with ./genmk\n\n"
	      "namespace libecc {\n\n"
	      "template<unsigned int m>\n"
	      "  struct reduction_polynomial_exponents {\n"
	      "    static unsigned int const k = 0;\n"
	      "    static unsigned int const k1 = 0;\n"
	      "    static unsigned int const k2 = 0;\n"
	      "  };\n\n";

  for (std::vector<poly>::iterator iter = mk.begin(); iter != mk.end(); ++iter)
  {
    mkheader << "#if LIBECC_M == " << iter->m << "\n"
    		"static unsigned int const field_extension_degree = " << iter->m << ";\n"
                "template<>\n"
                "  struct reduction_polynomial_exponents<" << iter->m << "> {\n"
                "    static unsigned int const k = " << iter->k << ";\n"
                "    static unsigned int const k1 = " << iter->k1 << ";\n"
                "    static unsigned int const k2 = " << iter->k2 << ";\n"
		"  };\n"
		"#endif\n";
  }

  mkheader << "\n} // namespace libecc\n\n#endif\t// MK_H\n";
  mkheader.close();

  return 0;
}
