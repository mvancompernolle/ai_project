#include "sys.h"
#include "debug.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cassert>
#include <math.h>
#include <libecc/point.h>
#include "pngwriter.h"
#include "utils.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using libecc::bitset_digit_t;
using libecc::bitset;

struct RGB {
 int red;
 int green;
 int blue;
 RGB(int r, int g, int b) : red(r), green(g), blue(b) { }
};

std::ostream& operator<<(std::ostream& os, RGB const& rgb)
{
  return os << '{' << rgb.red << ", " << rgb.green << ", " << rgb.blue << '}';
}

std::vector<RGB> colors;
std::vector<RGB>::iterator color_iter;

RGB& get_next_color(void)
{
  return *(color_iter++);
}

void reset_colors(void)
{
  color_iter = colors.begin();
}

RGB background(247 << 8, 252 << 8, 248 << 8);

void generate_colors(int number_of_colors)
{
  static int a2[] = { 1, 0 };
  static int a3[] = { 2, 0, 1 };
  static int a4[] = { 3, 0, 2, 1 };
  static int a5[] = { 4, 0, 2, 3, 1 };
  int* ra;
  int* ga;
  int* ba;
  int rs, gs, bs;

  if (number_of_colors <= 6)
  {
    rs = gs = bs = 2;
    ra = ga = ba = a2;
  }
  else if (number_of_colors <= 10)
  {
    rs = gs = 2;
    ra = ga = a2;
    bs = 3;
    ba = a3;
  }
  else if (number_of_colors <= 16)
  {
    rs = 2;
    ra = a2;
    gs = bs = 3;
    ga = ba = a3;
  }
  else if (number_of_colors <= 24)
  {
    rs = gs = bs = 3;
    ra = ga = ba = a3;
  }
  else if (number_of_colors <= 34)
  {
    rs = gs = 3;
    ra = ga = a3;
    bs = 4;
    ba = a4;
  }
  else if (number_of_colors <= 46)
  {
    rs = 3;
    ra = a3;
    gs = bs = 4;
    ga = ba = a4;
  }
  else if (number_of_colors <= 60)
  {
    rs = gs = bs = 4;
    ra = ga = ba = a4;
  }
  else if (number_of_colors <= 78)
  {
    rs = gs = 4;
    ra = ga = a4;
    bs = 5;
    ba = a5;
  }
  else if (number_of_colors <= 98)
  {
    rs = 4;
    ra = a4;
    gs = bs = 5;
    ga = ba = a5;
  }
  else if (number_of_colors <= 120)
  {
    rs = gs = bs = 5;
    ra = ga = ba = a5;
  }
  else
  {
    assert(number_of_colors <= 120);
  }
  for (int ri = 0; ri < rs; ++ri)
  {
    int r = (int)round(65535 * sqrt(1.0 * ra[ri] / (rs - 1)));
    for (int gi = 0; gi < gs; ++gi)
    {
      int g = (int)round(65535 * sqrt(1.0 * ga[gi] / (gs - 1)));
      for (int bi = 0; bi < bs; ++bi)
      {
	int b = (int)round(65535 * sqrt(1.0 * ba[bi] / (bs - 1)));
	if (ra[ri] == 0 && ga[gi] == 0 && ba[bi] == 0)
	  continue;	// Skip black.
	if (ra[ri] == rs - 1 && ga[gi] == gs - 1 && ba[bi] == bs - 1)
	  continue;	// Skip white.
	if (rs == gs && rs == bs && ra[ri] == ga[gi] && ra[ri] == ba[bi])
	  continue;	// Skip gray values.
	assert(r != 0 || g != 0 || b != 0);
        colors.push_back(RGB(r, g, b));
      }
    }
  }
  reset_colors();
}

inline int len(bitset_digit_t d)
{
  if (d < 10)
    return 1;
  if (d < 100)
    return 2;
  if (d < 1000)
    return 3;
  if (d < 10000)
    return 4;
  if (d < 100000)
    return 5;
  assert(false);
}

class IsomorphicCurves {
private:
  std::string M_structure;
  int M_cardinality;
  std::vector<std::pair<bitset_digit_t, bitset_digit_t> > M_ab;

public:
  IsomorphicCurves(std::string const& structure, int cardinality) :
      M_structure(structure), M_cardinality(cardinality) { }
  std::string const& structure(void) const { return M_structure; }
  int cardinality(void) const { return M_cardinality; }
  std::vector<std::pair<bitset_digit_t, bitset_digit_t> > const& get_ab(void) const { return M_ab; }
  void add(bitset_digit_t a, bitset_digit_t b) { M_ab.push_back(std::make_pair(a, b)); }
  void check(void) const;
};

static bitset_digit_t S_normal;

void IsomorphicCurves::check(void) const
{
  cout << "Checking " << M_structure << endl;

  // There should be a multiple of q/2 curves.
  assert(M_ab.size() % (q / 2) == 0);

  // Make a square of all values of a against b.
  std::vector<bitset<q> > matrix(q, bitset<q>(0));
  for (std::vector<std::pair<bitset_digit_t, bitset_digit_t> >::const_iterator iter = M_ab.begin(); iter != M_ab.end(); ++iter)
    matrix[iter->first].set(iter->second);

  // Check that a and b are totally independent.
  bitset<q> b_OR_pattern, b_AND_pattern;
  b_OR_pattern.reset();
  b_AND_pattern.setall();
  int a_count = 0;
  for (unsigned int a = 0; a < q; ++a)
  {
    if (matrix[a].any())
    {
      ++a_count;
      b_OR_pattern |= matrix[a];
      b_AND_pattern &= matrix[a];
    }
  }
  assert(a_count == q / 2);
  assert(b_OR_pattern == b_AND_pattern);

  // Also determine the bits of a that are correlated.
  // Consider the m-dimensional space spanned by GF(2^m) in
  // which each value of a is a vector.
  // We are going to determine a vector called 'normal'
  // that is orthogonal to the difference between any
  // two pair of a's.  However, we already "know" that all
  // a are spanning a hyper-plane (because they are independent
  // of b and split the space precisely in two parts: that
  // means we lose one dimension (one 'bit')).  Therefore
  // we can subtract any of the values of a from all the
  // others and then find a vector whose inproduct with each
  // of those differences is 0.  Lets call this orthogonal
  // vector 'normal'.
  // Now we have to realize that this means the following.
  // It means that for any two values of a, a1 and a2 say,
  // where a1 != a2, that normal * (a1 - a2) = 0, where the
  // '*' is the inproduct.  Because we work with a field
  // with characteristic 2, the subtraction is equivalent
  // with exclusive-OR; the bitwise multiplication of
  // the inproduct is the AND function, and the addition of
  // each of those products is again an exclusive-OR.
  // In other words: normal & (a1 ^ a2) has an even number
  // bits set for any a1 and a2.  Because we already know
  // that this is true, we can suffice with just using one
  // fixed value of a1 (for example the smallest one).
  // Then, this smallest one is either 0, or not 0.
  // (Because the value 0 is either part of the set or it
  // is not).  If 0 is not part of the solution, then
  // apparently normal & (a1 ^ a2) does not have an even number
  // of 1's for a1=0; in other words, normal & a2 = has an odd number
  // of 1's for any value of a2, including the smallest value
  // of a2 that is part of the set. In order to save a little
  // cpu we refrain from subtracting any a1 at all and
  // just look at (normal & a) for all a, which will now
  // either have an odd number or even number of 1's
  // (corresponding with the inproduct being 1 or 0 respectively).
  bool inproduct_is_zero = matrix[0].any();	// If a == 0 is part of this set, then the inproduct must be 0.

  // Next we have to realize that if normal has an odd number of
  // 1's and a=0 is NOT part of the set then a=(2^m - 1) (all bits
  // set) IS part of the set and visa versa.  This is a logical
  // result from the fact that (normal & 0) = 0 which has an even
  // number of 1's (namely 0), and (normal & (2^m - 1)) = normal
  // which will then have an odd number of 1's.
  bool normal_has_even_number_of_ones = (inproduct_is_zero == matrix[q - 1].any());

  // Now we can separate the following cases.
  //
  // case A (inproduct is not zero):
  //
  // If a=0 is not part of the set, then the solution(s) with
  // exactly 1 bit set reveals each of the 1's of normal.
  //
  // case B (inproduct is zero, and normal has an even number of 1's):
  //
  // If a=0 and a=2^m - 1 are not part of the set, then there must
  // exist a value of 'a' with just one bit set that is not part
  // of the set (namely, any bit in normal); once we found that we
  // can convert the solutions by toggling this bit with each
  // value of a and treating it like in case A.
  //
  // case C (inproduct is zero, and normal has an odd number of 1's):
  //
  // If a=0 is part of the set, but 2^m - 1 is not, then we
  // can look for all values of 'a' with m-1 bits set, the zeroes
  // of which will reveal the 1's of 'normal'.
  //
  // We implement this by treating all cases like case A after
  // toggling any bits in 'toggle' and setting 'toggle' to:
  bitset_digit_t toggle = inproduct_is_zero ? ((bitset_digit_t)1 << m) - 1 : 0;	// Case C : A.
  if (inproduct_is_zero && normal_has_even_number_of_ones)
  {
    // Case B.
    // This mean thus that both, a=0 (all zeroes) and a=q-1 (all ones) are part of the hyperplane.
    // Find a value of 'a' with exactly one bit set that is not part of the set.
    toggle = 0;
    for (bitset_digit_t a = 1; a < q; a <<= 1)
      if (!matrix[a].any())
      {
        toggle = a;
	break;
      }
    assert(toggle != 0);
  }

  // Finally find all values of 'a' (after toggling any bits)
  // with exactly one bit set and collect them to form 'normal'.
  bitset_digit_t normal = 0;
  for (bitset_digit_t single_bit = 1; single_bit < q; single_bit <<= 1)
    if (matrix[single_bit ^ toggle].any())
      normal |= single_bit;
  assert(normal != 0);

  // Next make sure that this is a dependence on m only, and therefore
  // constant for for every group structure.
  if (S_normal == 0)
    S_normal = normal;
  assert(normal == S_normal);
}

int main(int, char* [])
{
#if 0
  // Alternative color generator.
  // Although slightly nicer looking colors, these are less distinguishable.

  int c = 2; // 1 = 7, 2 = 21, 3 = 43, 4 = 73, 5 = 111 colors.
  int sz = (c + 1) * 52;
  pngwriter png(sz * 3, sz * 3, 0.5, "pngwritertest.png");
  for (int i = -c; i <= 2 * c; ++i)
  {
    for (int j = -c; j <= 2 * c; ++j)
    {
      for (int ud = 0; ud < 2; ++ud)
      {
        bool down = ud;
	int x_offset = down ? 26 : 0;
	int y_offset = down ? 45 : 0;
	int top_offset = down ? -45 : 45;

	int x = sz + 26 * i + 52 * j + x_offset;
	int y = sz + 45 * i + (sz - (c + 1) * 45) / 2 + y_offset;

	double r = (j >= 0) ? sqrt((j + ud / 3.0) / c) : 0;
	double g = (i + j < c) ? sqrt(((c - (i + j)) - 2 * ud / 3.0) / c) : 0;
	double b = (i >= 0) ? sqrt((i + ud / 3.0) / c) : 0;

	if (r <= 1.0001 && g <= 1.0001 && b <= 1.0001)
	  png.filledtriangle(x, y, x + 52, y, x + 26, y + top_offset,
	      (int)(r * 65535), (int)(g * 65535 * 0.9), (int)(b * 65535));
      }
    }
  }
  png.close();

#else

  std::ostringstream out_filename;
  out_filename << "ab_map" << m << ".png";

  int const z = std::min(std::max(512 >> m, 1), 32);
  pngwriter png((z << m) - 1, (z << m) - 1, 0.0, out_filename.str().c_str());

  std::ostringstream in_filename;
  in_filename << "testPoint" << m << ".out";

  std::ifstream in;
  in.open(in_filename.str().c_str());

  std::string line;
  if (!in || !getline(in, line))
  {
    cerr << "Could not open file " << in_filename.str() << ".  First run: ./testPoint" << endl;
    return 1;
  }

  std::vector<IsomorphicCurves> hash_table[2 * q];	// Index is hash (which is cardinality of curve).
  int number_of_isomorphic_curves = 0;

  while(getline(in, line))
  {
    // We expect lines with exactly this format:
    // z = 1; #E = 8; (a, b) = (0, 2); Structure: C_8
    if (line[0] != 'z')
      continue;
    char const* l = line.c_str();
    int cardinality = atoi(l + 12);
    int offset = len(cardinality);
    bitset_digit_t a = atoi(l + 24 + offset);
    offset += len(a);
    bitset_digit_t b = atoi(l + 26 + offset);
    offset += len(b);
    offset += 40;
    std::string structure = l + offset;
    std::vector<IsomorphicCurves>& curves = hash_table[cardinality];
    std::vector<IsomorphicCurves>::iterator iter;
    for (iter = curves.begin(); iter != curves.end(); ++iter)
      if (iter->structure() == structure)
        break;
    if (iter == curves.end())
    {
      ++number_of_isomorphic_curves;
      iter = curves.insert(curves.end(), IsomorphicCurves(structure, cardinality));
    }
    assert(iter->cardinality() == cardinality);
    iter->add(a, b);
  }

  generate_colors(number_of_isomorphic_curves);

  for (unsigned int c = 0; c < 2 * q; ++ c)
  {
    std::vector<IsomorphicCurves> const& curves = hash_table[c];
    if (curves.size() == 0)
      continue;
    for (std::vector<IsomorphicCurves>::const_iterator iter = curves.begin(); iter != curves.end(); ++iter)
    {
      iter->check();
      RGB& color = get_next_color();
      for (unsigned int c = 0; c < iter->get_ab().size(); ++c)
      {
        int x = z * iter->get_ab()[c].first;
	int y = z * iter->get_ab()[c].second;
	png.filledsquare(x, y, x + z - 1, y + z - 1, color.red, color.green, color.blue);
      }
    }
  }

  png.close();
  cout << "a-space hyperplane normal is " << S_normal << endl;
  cout << "Output written to " << out_filename.str() << endl;

  std::ostringstream out_filename2;
  out_filename2 << "ab_legend" << m << ".png";

  int const h = std::max(z, 16);
  pngwriter png2(100, number_of_isomorphic_curves * h + 2, 1.0, out_filename2.str().c_str());
  png2.filledsquare(0, 0, 100, number_of_isomorphic_curves * h + 2, background.red, background.green, background.blue);
  png2.filledsquare(0, 0, h, number_of_isomorphic_curves * h + 1, 0, 0, 0);

  reset_colors();
  int sn = 0;
  for (unsigned int c = 0; c < 2 * q; ++ c)
  {
    std::vector<IsomorphicCurves> const& curves = hash_table[c];
    if (curves.size() == 0)
      continue;
    for (std::vector<IsomorphicCurves>::const_iterator iter = curves.begin(); iter != curves.end(); ++iter)
    {
      RGB& color = get_next_color();
      png2.filledsquare(2, h * sn + 2, h - 1, h * sn + h, color.red, color.green, color.blue);
      // FreeMonoBold.ttf is part of package ttf-freefont.
      png2.plot_text(const_cast<char*>("/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf"), 8,
          8 + h, h / 2 - 2 + h * sn, 0.0, const_cast<char*>(iter->structure().c_str()), 0, 0, 0);
      ++sn;
    }
  }

  png2.close();
  cout << "Output written to " << out_filename2.str() << endl;
#endif

  return 0;
}

