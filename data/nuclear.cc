//  Copyright (C) 2001--2009  Petter Urkedal
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


#include <more/math/math.h>
#include <more/phys/nuclear.h>
#include <more/phys/si.h>
#include <more/io/iomanip.h>
#include <iostream>
#include <sstream>


namespace more {
namespace phys {

  math::minus_half_tag  mt_neut;  // neutron iso-spin z-projection
  math::half_tag        mt_prot;  // proton iso-spin z-projection


  //  --[ nucleus, stream operators ]--

  std::istream&
  operator>>(std::istream& is, nucleus& x)
  {
      int A;
      char c0;
      is >> c0;
      if (isalpha(c0)) {		// Ca-42, ca-40, Ca40, ca40
	  std::string Zname;
	  Zname += std::toupper(c0);
	  if (std::isalpha(is.peek()))
	      Zname += is.get();
	  if (std::isalpha(is.peek()))
	      Zname += is.get();
	  int Z;
	  try {
	      Z = nucleus::n_prot(Zname);
	  } catch (std::invalid_argument const&) {
	      std::cerr << "Fail to get Z from " << Zname << std::endl;
	      is.setstate(std::ios_base::failbit);
	      return is;
	  }
	  if (is.peek() == '-')
	      is.get();
	  is >> A;
	  if (!is.fail()) {
	      x = nucleus(A-Z, Z);
	      if (is.fail()) {
		  std::cerr << "Input is fail 1" << std::endl;
	      }
	      return is;
	  }
      }
      else if (c0 == '(') {		// (22, 20)
	  char comma, rparen;
	  int N, Z;
	  is >> N >> comma >> Z >> rparen;
	  if (is.good() && comma == ',' && rparen == ')' && N >= 0 && Z >= 0) {
	      x = nucleus(N, Z);
	      if (is.fail()) {
		  std::cerr << "Input is fail 2" << std::endl;
	      }
	      return is;
	  }
      }
      else {
	  is.putback(c0);
	  char c0 = ' ', c1 = ' ';
	  int i0 = 0, i1 = 0;
	  is >> i0 >> c0;
	  if (!is.good()){
              std::cerr << "Fail to load one int + one char" << std::endl;
	      return is;
          }
	  if (c0 == 'n') {		// 22n20p
	      if (std::isdigit(is.peek()))
		  is >> i1 >> c1;
	      if (is.good() && c1 == 'p') {
		  x = nucleus(i0, i1);
		  if (is.fail()) {
		      std::cerr << "Input is fail 3" << std::endl;
		  }
		  return is;
	      }
	  } else if (c0 == 'p') {	// 20p22n
	      if (std::isdigit(is.peek()))
		  is >> i1 >> c1;
	      if (is.good() && c1 == 'n') {
		  x = nucleus(i1, i0);
                  if (is.fail()) {
                      std::cerr << "Input is fail 4" << std::endl;
                  }
		  return is;
	      }
	  } else {			// 40Ca, 40ca
	      std::string Zname;
	      Zname += std::toupper(c0);

	      if (!is.eof() && std::isalpha(is.peek()))
		  Zname += is.get();
	      if (!is.eof() && std::isalpha(is.peek()))
		  Zname += is.get();
	      int Z;
	      try {
		  Z = nucleus::n_prot(Zname);
	      } catch (std::invalid_argument const&) {
		  std::cerr << "Fail to get Z from " << Zname << std::endl;
		  is.setstate(std::ios_base::failbit);
		  return is;
	      }
	      x = nucleus(i0 - Z, Z);
	      return is;
	  }
      }
      {
	  std::string line;
	  is >> line;
	  std::cerr << "Failed to parse: '" << line << "'\n";
      }
      is.setstate(std::ios_base::failbit);
      return is;
  }

  std::ostream&
  operator<<(std::ostream& os, const nucleus& x)
  {
      if (io::tslang(os) == io::tslang_latex_text)
	  os << "$^{" << x.n_part() << "}_{" << x.n_prot() << "}$"
	     << x.chemical_symbol() << "$_{" << x.n_neut() << "}$";
      else if (io::tslang(os) == io::tslang_latex_math)
	  os << "^{" << x.n_part() << "}_{" << x.n_prot() << "}\\mbox{"
	     << x.chemical_symbol() << "}_{" << x.n_neut() << "}";
      else if (io::has_scripts(os))
	  os << io::beg_super << x.n_part() << io::end_super
	     << io::beg_sub << x.n_prot() << io::end_sub
	     << x.chemical_symbol()
	     << io::beg_sub << x.n_neut() << io::end_sub;
      else
	  os << x.name();
      return os;
  }


  //  --[ nucleus, members ]--

  nucleus::nucleus(int N, int Z)
      : n_p(Z), n_n(N)
  {
      if (N < 0 || Z < 0) {
	  std::ostringstream oss;
	  oss << "more::phys::nucleus::nucleus(int, int): Arguments "
	      << "(N, Z) = " << N << ", " << Z << " out of range.\"";
	  throw std::logic_error(oss.str());
      }
  }

  void
  nucleus::sync(more::io::syncstream& sio)
  {
      sio | n_p | n_n;
  }

  std::string
  nucleus::name() const
  {
      std::ostringstream oss;
      oss << chemical_name() << '-' << n_part();
      return oss.str();
  }

  std::string
  nucleus::name_AAASy() const
  {
      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(3) << n_part() << chemical_name();
      return oss.str();
  }


  //  --[ nucleus, static members ]--

  static const int Z_unnamed = 119;

  static const char*
  chemical_symbols[Z_unnamed+1] =
  {
       "Nn", "H",  "He", "Li", "Be",   "B",  "C",  "N",  "O",  "F", // 0--9
       "Ne", "Na", "Mg", "Al", "Si",   "P",  "S", "Cl", "Ar",  "K", // 10--19
       "Ca", "Sc", "Ti",  "V", "Cr",  "Mn", "Fe", "Co", "Ni", "Cu", // 20--29
       "Zn", "Ga", "Ge", "As", "Se",  "Br", "Kr", "Rb", "Sr",  "Y", // 30--39
       "Zr", "Nb", "Mo", "Tc", "Ru",  "Rh", "Pd", "Ag", "Cd", "In", // 40--49
       "Sn", "Sb", "Te",  "I", "Xe",  "Cs", "Ba", "La", "Ce", "Pr", // 50--59
       "Nd", "Pm", "Sm", "Eu", "Gd",  "Tb", "Dy", "Ho", "Er", "Tm", // 60--69
       "Yb", "Lu", "Hf", "Ta",  "W",  "Re", "Os", "Ir", "Pt", "Au", // 70--79
       "Hg", "Tl", "Pb", "Bi", "Po",  "At", "Rn", "Fr", "Ra", "Ac", // 80--89
       "Th", "Pa",  "U", "Np", "Pu",  "Am", "Cm", "Bk", "Cf", "Es", // 90--99
       "Fm", "Md", "No", "Lr", "Rf",  "Db", "Sg", "Bh", "Hs", "Mt", // 100--
      "Uun","Uuu","Uub","Uut","Uuq", "Uup","Uuh","Uus","Uuo",
      NULL
  };

  static const char*
  chemical_names[Z_unnamed+1] =
  {
      "", "hydrogen", "helium", "lithium",
      "beryllium", "boron", "carbon", "nitrogen",
      "oxygen", "flourine", "neon", "sodium",
      "magnesium", "aluminium", "silicon", "phosphorus",
      "sulfur", "chlorine", "argon", "potassium",
      "calcium", "scandium", "titatium", "vanadium",
      "chromium", "manganese", "iron", "cobalt",
      "nickel", "copper", "zinc", "gallium",
      "germanium", "arsenic", "selenium", "bromine",
      "krypton", "rubidium", "strontium", "yttrium",
      "zirconium", "niobium", "molybdenum", "technetium",
      "ruthenium", "rhodium", "palladium", "silver",
      "cadmium", "indium", "tin", "antimony",
      "tellurium", "iodine", "xenon", "cesium",
      "barium", "lanthanum", "cerium", "praseodymium",
      "neodinium", "promethium", "samarium", "europium",
      "gadolinium", "terbium", "dysprosium", "holonium",
      "erbium", "thulium", "ytterbium", "lutetium",
      "hafnium", "tantalum", "tungsten", "rhenium",
      "osmium", "iridium", "platinum", "gold",
      "mercury", "thallium", "lead", "bismuth",
      "polonium", "astatine", "radon", "francium",
      "radium", "actinium", "thorium", "protactinium",
      "uranium", "neptunium", "plutonium", "americum",
      "curium", "berkelium", "californium", "einsteinium",
      "fermium", "mendelevium", "nobelium", "lawrencium",
      "rutherfordium", "dubnium", "seaborgium", "bohrium",
      "hassium", "meitnerium", "ununnilium", "unununium",
      "ununbium", "ununtrium", "ununquadium", "ununpentium",
      "ununhexium", "ununseptium", "ununoctium",
      NULL
  };

  int Z_first_unnamed() { return Z_unnamed; }

  std::string
  chemical_symbol(int Z)
  {
      if (Z >= Z_unnamed)
	  throw std::runtime_error("more::phys::nucleus: "
				   "No chemical name for this Z.");
      else
	  return chemical_symbols[Z];
  }

  std::string
  chemical_name(int Z)
  {
      if (Z >= Z_unnamed)
	  throw std::runtime_error("more::phys::nucleus: "
				   "No chemical name for this Z.");
      else
	  return chemical_names[Z];
  }

  int
  Z_of_chemical(std::string s)
  {
      for (std::string::size_type i = 1; i < s.size(); ++i)
	  s[i] = std::tolower(s[i]);
      if (s.size() > 0) {
	  s[0] = std::toupper(s[0]);
	  if (s.size() <= 3) {
	      for (int Z = 0; chemical_symbols[Z] != NULL; ++Z)
		  if (s == std::string(chemical_symbols[Z]))
		      return Z;
	  }
	  s[0] = std::tolower(s[0]);
	  if (s.size() >= 3) {
	      for (int Z = 0; chemical_names[Z] != NULL; ++Z)
		  if (s == std::string(chemical_names[Z]))
		      return Z;
	  }
      }
      throw std::invalid_argument("no such nucleus");
  }

  double
  semiempirical_binding(nucleus const& nucl)
  {
      using math::pow2;
      using std::pow;
      static double const b_vol = 15.56;
      static double const b_surf = 17.23;
      static double const b_sym = 46.57;
      static double const b_coul = .6*1.439976/1.24;
      return (b_vol*nucl.n_part()
	      - b_surf*pow((double)nucl.n_part(), .666667)
	      - .5*b_sym*pow2(nucl.n_neut() - nucl.n_prot())/nucl.n_part()
	      - b_coul*pow2(nucl.n_prot())*pow((double)nucl.n_part(), -.333333)
	      - 12.0*(nucl.n_neut() % 2 + nucl.n_prot() % 2)
	      /sqrt(double(nucl.n_part())))*SI::MeV;
  }

  bool
  is_reasonably_bound(nucleus const& nucl)
  {
      if (nucl.n_prot() <= 2 && nucl.n_neut() <= 2)
	  return true;
      double B = semiempirical_binding(nucl);
      double S_neut = B - semiempirical_binding(neighbor(nucl, -1, 0));
      double S_prot = B - semiempirical_binding(neighbor(nucl, 0, -1));
      return S_neut >= -1.0*SI::MeV && S_prot >= -3.0*SI::MeV;
  }

  std::pair<double, double>
  semiempirical_separation_energies(nucleus const& nucl)
  {
      double B = semiempirical_binding(nucl);
      double S_neut = B - semiempirical_binding(neighbor(nucl, -1, 0));
      double S_prot = B - semiempirical_binding(neighbor(nucl, 0, -1));
      return std::make_pair(S_neut, S_prot);
  }

  int nuclear_magic_number_array[] = { 2, 8, 20, 28, 50, 82, 126 };
  std::size_t nuclear_magic_number_count
  = sizeof(nuclear_magic_number_array)/sizeof(nuclear_magic_number_array[0]);

  bool
  is_nuclear_magic_number(int i)
  {
      switch (i) { // may be faster than running in a loop on the above array
      case 2:
      case 8:
      case 20:
      case 28:
      case 50:
      case 82:
      case 126:
	  return true;
      default:
	  return false;
      }
  }

}
}


// Local Variables:
// coding: utf-8
// indent-tabs-mode: t
// End:
