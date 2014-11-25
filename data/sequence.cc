#include "sys.h"
#include "debug.h"
#include <iostream>
#include <string>
#include <vector>
#include "MultiLoop.h"

using std::cout;
using std::endl;

#if 0
int main()
{
  for (int odd = 0; odd < 2; ++odd)
  {
    char const* divisors = (odd ? "odd_divisors" : "divisors");
    for (int n = 1; n <= 64; ++n)
    {
      std::vector<int> divs;
      for (int d = 1; d < n; ++d)
	if (n % d == 0 && (!odd || (n/d) % 2 == 1))
	  divs.push_back(d);
      cout << "int " << divisors << n << "[] = { " << divs.size();
      for (std::vector<int>::iterator iter = divs.begin(); iter != divs.end(); ++iter)
	cout << ", " << *iter;
      cout << " };\n";
    }
    cout << "\nint* " << divisors << "[65] = {\n  NULL";
    for (int n = 1; n <= 64; ++n)
    {
      if ((n - 1) % 8 == 0)
	cout << ",\n  ";
      cout << divisors << n;
      if (n % 8 != 0)
	cout << ", ";
    }
    cout << "\n};\n";
  }
}
#else

int divisors1[] = { 0 };
int divisors2[] = { 1, 1 };
int divisors3[] = { 1, 1 };
int divisors4[] = { 2, 1, 2 };
int divisors5[] = { 1, 1 };
int divisors6[] = { 3, 1, 2, 3 };
int divisors7[] = { 1, 1 };
int divisors8[] = { 3, 1, 2, 4 };
int divisors9[] = { 2, 1, 3 };
int divisors10[] = { 3, 1, 2, 5 };
int divisors11[] = { 1, 1 };
int divisors12[] = { 5, 1, 2, 3, 4, 6 };
int divisors13[] = { 1, 1 };
int divisors14[] = { 3, 1, 2, 7 };
int divisors15[] = { 3, 1, 3, 5 };
int divisors16[] = { 4, 1, 2, 4, 8 };
int divisors17[] = { 1, 1 };
int divisors18[] = { 5, 1, 2, 3, 6, 9 };
int divisors19[] = { 1, 1 };
int divisors20[] = { 5, 1, 2, 4, 5, 10 };
int divisors21[] = { 3, 1, 3, 7 };
int divisors22[] = { 3, 1, 2, 11 };
int divisors23[] = { 1, 1 };
int divisors24[] = { 7, 1, 2, 3, 4, 6, 8, 12 };
int divisors25[] = { 2, 1, 5 };
int divisors26[] = { 3, 1, 2, 13 };
int divisors27[] = { 3, 1, 3, 9 };
int divisors28[] = { 5, 1, 2, 4, 7, 14 };
int divisors29[] = { 1, 1 };
int divisors30[] = { 7, 1, 2, 3, 5, 6, 10, 15 };
int divisors31[] = { 1, 1 };
int divisors32[] = { 5, 1, 2, 4, 8, 16 };
int divisors33[] = { 3, 1, 3, 11 };
int divisors34[] = { 3, 1, 2, 17 };
int divisors35[] = { 3, 1, 5, 7 };
int divisors36[] = { 8, 1, 2, 3, 4, 6, 9, 12, 18 };
int divisors37[] = { 1, 1 };
int divisors38[] = { 3, 1, 2, 19 };
int divisors39[] = { 3, 1, 3, 13 };
int divisors40[] = { 7, 1, 2, 4, 5, 8, 10, 20 };
int divisors41[] = { 1, 1 };
int divisors42[] = { 7, 1, 2, 3, 6, 7, 14, 21 };
int divisors43[] = { 1, 1 };
int divisors44[] = { 5, 1, 2, 4, 11, 22 };
int divisors45[] = { 5, 1, 3, 5, 9, 15 };
int divisors46[] = { 3, 1, 2, 23 };
int divisors47[] = { 1, 1 };
int divisors48[] = { 9, 1, 2, 3, 4, 6, 8, 12, 16, 24 };
int divisors49[] = { 2, 1, 7 };
int divisors50[] = { 5, 1, 2, 5, 10, 25 };
int divisors51[] = { 3, 1, 3, 17 };
int divisors52[] = { 5, 1, 2, 4, 13, 26 };
int divisors53[] = { 1, 1 };
int divisors54[] = { 7, 1, 2, 3, 6, 9, 18, 27 };
int divisors55[] = { 3, 1, 5, 11 };
int divisors56[] = { 7, 1, 2, 4, 7, 8, 14, 28 };
int divisors57[] = { 3, 1, 3, 19 };
int divisors58[] = { 3, 1, 2, 29 };
int divisors59[] = { 1, 1 };
int divisors60[] = { 11, 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30 };
int divisors61[] = { 1, 1 };
int divisors62[] = { 3, 1, 2, 31 };
int divisors63[] = { 5, 1, 3, 7, 9, 21 };
int divisors64[] = { 6, 1, 2, 4, 8, 16, 32 };

int* divisors[65] = {
  NULL,
  divisors1, divisors2, divisors3, divisors4, divisors5, divisors6, divisors7, divisors8,
  divisors9, divisors10, divisors11, divisors12, divisors13, divisors14, divisors15, divisors16,
  divisors17, divisors18, divisors19, divisors20, divisors21, divisors22, divisors23, divisors24,
  divisors25, divisors26, divisors27, divisors28, divisors29, divisors30, divisors31, divisors32,
  divisors33, divisors34, divisors35, divisors36, divisors37, divisors38, divisors39, divisors40,
  divisors41, divisors42, divisors43, divisors44, divisors45, divisors46, divisors47, divisors48,
  divisors49, divisors50, divisors51, divisors52, divisors53, divisors54, divisors55, divisors56,
  divisors57, divisors58, divisors59, divisors60, divisors61, divisors62, divisors63, divisors64
};
int odd_divisors1[] = { 0 };
int odd_divisors2[] = { 0 };
int odd_divisors3[] = { 1, 1 };
int odd_divisors4[] = { 0 };
int odd_divisors5[] = { 1, 1 };
int odd_divisors6[] = { 1, 2 };
int odd_divisors7[] = { 1, 1 };
int odd_divisors8[] = { 0 };
int odd_divisors9[] = { 2, 1, 3 };
int odd_divisors10[] = { 1, 2 };
int odd_divisors11[] = { 1, 1 };
int odd_divisors12[] = { 1, 4 };
int odd_divisors13[] = { 1, 1 };
int odd_divisors14[] = { 1, 2 };
int odd_divisors15[] = { 3, 1, 3, 5 };
int odd_divisors16[] = { 0 };
int odd_divisors17[] = { 1, 1 };
int odd_divisors18[] = { 2, 2, 6 };
int odd_divisors19[] = { 1, 1 };
int odd_divisors20[] = { 1, 4 };
int odd_divisors21[] = { 3, 1, 3, 7 };
int odd_divisors22[] = { 1, 2 };
int odd_divisors23[] = { 1, 1 };
int odd_divisors24[] = { 1, 8 };
int odd_divisors25[] = { 2, 1, 5 };
int odd_divisors26[] = { 1, 2 };
int odd_divisors27[] = { 3, 1, 3, 9 };
int odd_divisors28[] = { 1, 4 };
int odd_divisors29[] = { 1, 1 };
int odd_divisors30[] = { 3, 2, 6, 10 };
int odd_divisors31[] = { 1, 1 };
int odd_divisors32[] = { 0 };
int odd_divisors33[] = { 3, 1, 3, 11 };
int odd_divisors34[] = { 1, 2 };
int odd_divisors35[] = { 3, 1, 5, 7 };
int odd_divisors36[] = { 2, 4, 12 };
int odd_divisors37[] = { 1, 1 };
int odd_divisors38[] = { 1, 2 };
int odd_divisors39[] = { 3, 1, 3, 13 };
int odd_divisors40[] = { 1, 8 };
int odd_divisors41[] = { 1, 1 };
int odd_divisors42[] = { 3, 2, 6, 14 };
int odd_divisors43[] = { 1, 1 };
int odd_divisors44[] = { 1, 4 };
int odd_divisors45[] = { 5, 1, 3, 5, 9, 15 };
int odd_divisors46[] = { 1, 2 };
int odd_divisors47[] = { 1, 1 };
int odd_divisors48[] = { 1, 16 };
int odd_divisors49[] = { 2, 1, 7 };
int odd_divisors50[] = { 2, 2, 10 };
int odd_divisors51[] = { 3, 1, 3, 17 };
int odd_divisors52[] = { 1, 4 };
int odd_divisors53[] = { 1, 1 };
int odd_divisors54[] = { 3, 2, 6, 18 };
int odd_divisors55[] = { 3, 1, 5, 11 };
int odd_divisors56[] = { 1, 8 };
int odd_divisors57[] = { 3, 1, 3, 19 };
int odd_divisors58[] = { 1, 2 };
int odd_divisors59[] = { 1, 1 };
int odd_divisors60[] = { 3, 4, 12, 20 };
int odd_divisors61[] = { 1, 1 };
int odd_divisors62[] = { 1, 2 };
int odd_divisors63[] = { 5, 1, 3, 7, 9, 21 };
int odd_divisors64[] = { 0 };

int* odd_divisors[65] = {
  NULL,
  odd_divisors1, odd_divisors2, odd_divisors3, odd_divisors4, odd_divisors5, odd_divisors6, odd_divisors7, odd_divisors8,
  odd_divisors9, odd_divisors10, odd_divisors11, odd_divisors12, odd_divisors13, odd_divisors14, odd_divisors15, odd_divisors16,
  odd_divisors17, odd_divisors18, odd_divisors19, odd_divisors20, odd_divisors21, odd_divisors22, odd_divisors23, odd_divisors24,
  odd_divisors25, odd_divisors26, odd_divisors27, odd_divisors28, odd_divisors29, odd_divisors30, odd_divisors31, odd_divisors32,
  odd_divisors33, odd_divisors34, odd_divisors35, odd_divisors36, odd_divisors37, odd_divisors38, odd_divisors39, odd_divisors40,
  odd_divisors41, odd_divisors42, odd_divisors43, odd_divisors44, odd_divisors45, odd_divisors46, odd_divisors47, odd_divisors48,
  odd_divisors49, odd_divisors50, odd_divisors51, odd_divisors52, odd_divisors53, odd_divisors54, odd_divisors55, odd_divisors56,
  odd_divisors57, odd_divisors58, odd_divisors59, odd_divisors60, odd_divisors61, odd_divisors62, odd_divisors63, odd_divisors64
};

class Constant {
  private:
    long M_constant;

  public:
    Constant(void) : M_constant(0) { }

  int complexity(void) const { return M_constant ? ((M_constant == 1L || M_constant == -1L) ? 1 : 2) : 0; }
  void next(void) { if (M_constant == 0) M_constant = 1; else if (M_constant < 0) M_constant = -M_constant + 1; else M_constant = -M_constant; }
  void reset(void) { M_constant = 0; }
  unsigned long index(void) const
  {
    return (M_constant == 0) ? 0 :
        ((M_constant < 0) ? ((unsigned long)(-M_constant) << 1)
	                  : ((unsigned long)(M_constant) << 1 - 1));
  }
  void set_index(unsigned long index)
  {
    // Index: 0   1   2   3   4   5   6 ...
    // Value: 0   1  -1   2  -2   3  -3 ...
    M_constant = (index + 1) >> 1;
    if ((index & 1) == 0)
      M_constant = -M_constant;
  }
  long value(void) const { return M_constant; }

  friend std::ostream& operator<<(std::ostream& os, Constant const& c)
  {
    os << c.M_constant;
    return os;
  }
};

struct FixedSequence {
  int M_offset;
  std::vector<long> M_sequence;
  std::string M_name;

  FixedSequence(int offset, std::string const& name) : M_offset(offset), M_name(name) { }

  long operator[](int index) const { assert(index >= M_offset && index <= M_offset + (int)M_sequence.size()); return M_sequence[index - M_offset]; }

  int min_index(void) const { return M_offset; }
  int max_index(void) const { return M_offset + M_sequence.size(); }
};

enum subtract_divisors_t {
  subtract_all_divisors,
  subtract_odd_divisors
};

struct Sequence {
  Sequence* M_base;				// Base sequence.

  Constant M_factor1;				// - M_factor1 * a(n-1)
  Constant M_factor2;				// - M_factor2 * a(n-2)
  subtract_divisors_t M_subtract_divisors;	// If M_factor1 and M_factor2 are both zero, then this determines which a(d) are subtracted.

  long M_offset;				// Base offset.
  int M_value0_offset;				// The value of n corresponding to M_value0.
  long M_value0;				// The value of a(M_value0_offset), valid if M_factor1 or M_factor2 is non-zero.
  long M_value1;				// The value of a(M_value0_offset + 1), valid if M_factor2 is non-zero.

  MultiLoop* M_ml;		// Loop over M_factor1 and M_factor2.
  int M_number_of_loops;
  int M_last_valid_loop;
  int M_maxed_out;		// The first loop that reached it maximum.

  FixedSequence const* M_fs;

  long cache[64];		// Cache.

  void clear_cache(void)
  {
    std::memset(cache, 0xaa, sizeof(cache));
  }

  void reset(void)
  {
    if (M_base)
      M_base->reset();
    M_offset = 0;
    clear_cache();
    M_factor1.reset();
    M_factor2.reset();
    M_subtract_divisors = subtract_all_divisors;
    M_value0_offset = 1;
    M_value0 = 0;
    M_value1 = 0;
    if (M_ml)
      delete M_ml;
    M_ml = NULL;
    initialize_multi_loop();
    M_maxed_out = 0;
    M_last_valid_loop = M_number_of_loops - 2;
  }

  Sequence(void) : M_base(NULL), M_ml(NULL), M_number_of_loops(6), M_fs(NULL) { reset(); }
  Sequence(FixedSequence const& fs) : M_base(NULL), M_ml(NULL), M_number_of_loops(4), M_fs(&fs) { reset(); }
  ~Sequence() { if (M_ml) delete M_ml; }

  unsigned int loop_end(MultiLoop const& ml)
  {
    if ((int)*ml == M_number_of_loops - 1)
      return 2;
    else if (*ml == 0)
      return std::numeric_limits<int>::max();
    return ml[0] + 1;
  }

  void initialize_multi_loop(void)
  {
    M_ml = new MultiLoop(M_number_of_loops);
    MultiLoop& ml(*M_ml);
    // Enter center loop.
    for(;; ml.next_loop())
    {
      for(; ml() < loop_end(ml); ++ml)
      {
	if ((int)*ml == M_number_of_loops - 1)
	{
	  if (M_fs && !finish_init())
	    next();
	  return;
        }
      }
    }
  }

  void next(void)
  {
    clear_cache();
    MultiLoop& ml(*M_ml);
    ++ml;
    for(;;)
    {
      while (ml() < loop_end(ml))
      {
        if ((int)*ml < M_maxed_out)
	  M_maxed_out = 0;
        if ((int)*ml == M_number_of_loops - 1)
	{
	  if (M_factor1.value() == 0 && M_factor2.value() == 0)
	    M_subtract_divisors = ml[*ml] ? subtract_odd_divisors : subtract_all_divisors;
	  else
	    ml[*ml] = 1;
	  if (!M_fs || finish_init())
	    return;
	}
	if (*ml != 0 && (int)*ml != M_number_of_loops - 1)
	{
	  if (!M_maxed_out && ml[*ml] == ml[0])
	    M_maxed_out = *ml;
	  if (*ml == 1)
	    M_factor2.set_index(ml[1]);
	  else if (!M_fs && *ml == 2)
	  {
	    if (ml[1] != 0)
	      M_last_valid_loop = 4;
	    else
	    {
	      M_last_valid_loop = 3;
	      if (ml[2] == 0)
	        ml[2] = 1;
	    }
	    if (!M_maxed_out && ml[*ml] == ml[0])
	      M_maxed_out = *ml;
	  }
	  if ((int)*ml == M_last_valid_loop)
	  {
	    if (!M_maxed_out)
	      ml[*ml] = ml[0];
	  }
	  if (*ml == 2)
	    M_factor1.set_index(ml[2]);
	  else if (*ml == 3)
	  {
	    if (M_factor1.value() != 0 || M_factor2.value() != 0)
	    {
	      M_value0 = (ml[3] + 1) >> 1;
	      if ((ml[3] & 1) == 0)
	        M_value0 = -M_value0;
            }
	    else
	      ml[3] = ml[0];
	  }
	  else if (*ml == 4)
	  {
	    if (M_factor2.value() != 0)
	    {
	      M_value1 = (ml[4] + 1) >> 1;
	      if ((ml[4] & 1) == 0)
	        M_value1 = -M_value1;
	    }
	    else
	      ml[4] = ml[0];
	  }
	}
        ++ml;
      }
      ml.next_loop();
    }
  }

  bool finish_init(void)
  {
    // This is the inner loop. Returning false skips this sequence.

    // Determine M_value0 and M_value1.
    int n  = M_value0_offset = M_fs->min_index();
    if (M_factor2.value() != 0)
    {
      M_value0 = (*M_fs)[M_value0_offset];
      M_value1 = (*M_fs)[M_value0_offset + 1];
      n += 2;
    }
    else if (M_factor1.value() != 0)
    {
      M_value0 = (*M_fs)[M_value0_offset];
      ++n;
    }
    long old_cache = cache[n];
    M_offset = 0;
    M_offset = (*M_fs)[n] - value(n);
    cache[n] = old_cache;

    return true;
  }

  void print_loop_variables(void) const
  {
    for (int i = 0; i < M_number_of_loops; ++i)
      cout << (*M_ml)[i] << " ";
    cout << endl;
  }

  void set_base(Sequence& base)
  {
    M_base = &base;
  }

  long value(int n)
  {
    if ((unsigned long)cache[n] != 0xaaaaaaaaaaaaaaaaUL)
      return cache[n];
    long result = M_base ? M_base->value(n) : 0;
    result += M_offset;
    if (M_factor1.value() == 0 && M_factor2.value() == 0)
    {
      if (M_subtract_divisors == subtract_all_divisors)
      {
	for (int i = 1; i <= divisors[n][0]; ++i)
	  result -= value(divisors[n][i]);
      }
      else
      {
	for (int i = 1; i <= odd_divisors[n][0]; ++i)
	  result -= value(odd_divisors[n][i]);
      }
    }
    else if (M_factor2.value() != 0)
    {
      assert(n >= M_value0_offset);
      if (n == M_value0_offset)
        result = M_value0;
      else if (n == M_value0_offset + 1)
        result = M_value1;
      else
      {
	result -= M_factor1.value() * value(n - 1);
	result -= M_factor2.value() * value(n - 2);
      }
    }
    else if (M_factor1.value() != 0)
    {
      assert(n >= M_value0_offset);
      if (n == M_value0_offset)
        result = M_value0;
      else
        result -= M_factor1.value() * value(n - 1);
    }
    cache[n] = result;
    return result;
  }

  friend std::ostream& operator<<(std::ostream& os, Sequence const& sequence)
  {
    long factor[3];
    factor[1] = sequence.M_factor1.value();
    factor[2] = sequence.M_factor2.value();
    if (factor[1] || factor[2])
    {
      if (factor[1] != 0 || factor[2] != 0)
        os << "a(" << sequence.M_value0_offset << ") = " << sequence.M_value0 << ", ";
      if (factor[2] != 0)
        os << "a("<< (sequence.M_value0_offset + 1) << ") = " << sequence.M_value1 << ", ";
    }
    os << "a(n) = ";
    bool first = true;
    if (sequence.M_base)
    {
      os << *sequence.M_base;
      first = false;
    }
    if (sequence.M_offset != 0)
    {
      if (!first)
        os << " + ";
      os << sequence.M_offset;
      first = false;
    } 
    if (factor[1] == 0 && factor[2] == 0)
    {
      if (first)
        os << '-';
      else
        os << " - ";
      if (sequence.M_subtract_divisors == subtract_all_divisors)
	os << "\\sum_{1<d<=n,d|n}{a(n/d)}";
      else
        os << "\\sum_{1<d<=n,d odd,d|n}{a(n/d)}";
      first = false;
    }
    else
    {
      for (int f = 1; f <= 2; ++f)
      {
        char const* sign;
        if (factor[f] < 0)
	{
	  sign = first ? "" : " + ";
	  factor[f] = -factor[f];
	}
	else
	  sign = first ? "-" : " - ";
	if (factor[f] > 1)
	{
	  os << sign << factor[f] << " * a(n - " << f << ")";
	  first = false;
	}
	else if (factor[f] == 1)
	{
	  os << sign << "a(n - " << f << ")";
	  first = false;
        }
      }
    }
    if (first)
      cout << 0;
    return os;
  }
};

int main(int argc, char* argv[])
{
  int number_elements = argc - 2;

  if (number_elements < 1)
  {
    cout << "Not enough elements.\n";
    return 1;
  }

  int offset = atoi(argv[1]);

  FixedSequence input(offset, "input");
  for (int i = 0; i < number_elements; ++i)
    input.M_sequence.push_back(atoi(argv[i + 2]));

  if (number_elements == 1)
  {
    cout << "f(n) = " << input[offset] << '\n';
    return 0;
  }

  Sequence s(input);
  for (int i = 0; i < 700000000; ++i)
  {
    bool success = true;
    for (int n = offset; n < offset + number_elements; ++n)
    {
      if (s.value(n) != input[n])
      {
        success = false;
	break;
      }
    }
    //s.print_loop_variables();
    if (success)
    {
      cout << s << ":\n";
      for (int n = offset; n < offset + number_elements + 4; ++n)
	cout << "a(" << n << ") = " << s.value(n) << ", ";
      cout << '\n';
      cout << '\n';
      return 0;
    }
    s.next();
  }

  return 0;
}

#endif
