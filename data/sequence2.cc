#include "sys.h"
#include "debug.h"
#include <iostream>
#include <string>
#include <cmath>
#include <iomanip>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <gmpxx.h>
#include "MultiLoop.h"
#include "utils.h"

using std::cout;
using std::endl;
using std::flush;

typedef unsigned long elements_type;		// The type of an elements of GF(2^n) count.
typedef unsigned long fecs_type;		// The type of an FEC of GF(2^n) count.
typedef std::vector<unsigned long> input_type;	// The type of the vector with the input sequence.

std::string program_name;			// The name of the program (argv[0]).

// This functor translates comma's to spaces.
struct CommaToSpace {
  char operator()(char c) { if (c == ',') return ' '; return c; }
};

// This functor translates x to log2(x).
struct Log2 {
  double operator()(input_type::value_type x) { return (x == 0) ? 1e-12 : log2(x); }
};

// This functor returns every time one more.
struct PlusOne {
  int offset;
  PlusOne(int start) : offset(start) { }
  int operator()(void) { return offset++; }
};

// A line y = a * x + b.
struct Line {
  double a;
  double b;
};

// Fit's a line through the set of points (x, y) by minimizing \sum_{i} { (y_i - (a x_i + b))^2 w_i }.
template<typename InItX, typename InItY, typename InItW>
Line fit_line(InItX x_begin, InItX x_end, InItY y_begin, InItW w_begin)
{
  double w_sum = 0, xw_sum = 0, yw_sum = 0;
  for (InItX x = x_begin, y = y_begin, w = w_begin; x != x_end; ++x, ++y, ++w)
  {
    w_sum += *w; 
    xw_sum += *x * *w;
    yw_sum += *y * *w;
  }
  double x_mean = xw_sum / w_sum;
  double y_mean = yw_sum / w_sum;
  double n1 = 0, n2 = 0;
  for (InItX x = x_begin, y = y_begin, w = w_begin; x != x_end; ++x, ++y, ++w)
  {
    n1 += *x * *w * (*y - y_mean);
    n2 += *x * *w * (*x - x_mean);
  }
  Line result;
  result.a = n1 / n2;
  double b_sum = 0;
  for (InItX x = x_begin, y = y_begin, w = w_begin; x != x_end; ++x, ++y, ++w)
    b_sum += (*y - result.a * *x) * *w;
  result.b = b_sum / w_sum;
  return result;
}

// Pretty print a matrix object.
template<typename T>
void print(boost::numeric::ublas::matrix<T> const& m)
{
  cout << '\n';
  size_t width = 0;
  for (int row = 0; row < m.size1(); ++row)
  {
    for (int col = 0; col < m.size2(); ++col)
    {
      std::stringstream tmp;
      tmp << m(row, col);
      width = std::max(width, tmp.str().size());
    }
  }
  width += 2;
  for (int row = 0; row < m.size1(); ++row)
  {
    for (int col = 0; col < m.size2(); ++col)
      cout << std::setw(width) << m(row, col);
    cout << '\n';
  }
}

// This function returns the number of non-zero rows after 'wiping' the matrix 'm',
// or zero when the wipe was successful (no non-zero rows occurred).
// The matrix must be n x (n+1). The extra column is in fact the result u of
// the matrix equation M v = u. This function then solves v by wiping the lower-left
// triangle of the square nxn matrix M, while updating u in order to keep the
// equation valid.
template<typename T>
int wipe(boost::numeric::ublas::matrix<T>& m)
{
  // Run over the diagonal elements (k, k) of the matrix.
  for (unsigned int k = 0; k < m.size1(); ++k)
  {
    T f = m(k, k);
    for (unsigned int row = k + 1; row < m.size1(); ++row)
    {
      bool nonzero = false;
      for (unsigned int col = k + 1; col < m.size2(); ++col)
        if (((m(row, col) *= f) -= m(row, k) * m(k, col)) != 0)
          nonzero = true;
      m(row, k) = 0;
      if (!nonzero)
        return k + 1;
    }
  }
  return 0;
}

// Return true if this finds a recursive linear dependence in the sequence 'a'.
// a(n_begin), a(n_begin+1), ..., a(n_end-1) must be given.
// If the function returns true, 'solution' contains first a constant offset,
// followed by coefficients relative to a(n-i). Thus, a(n) = solution[0] + \sum_{i}{solution[i] * a(n-i)}.
bool solve(unsigned long* a, int n_begin, int n_end, std::vector<long>& solution)
{
  // We start with trying the maximum number of variables.
  unsigned int rows = (n_end - n_begin + 1) / 2;
  while (rows)
  {
    // Declare and fill the matrix.
    boost::numeric::ublas::matrix<mpz_class> m(rows, rows + 1);
    for (unsigned int row = 0; row < rows; ++row)
    {
      m(row, 0) = 1;
      for (unsigned int col = 1; col < rows; ++col)
        m(row, col) = a[n_begin + row + rows - 1 - col];
      m(row, rows) = a[n_begin + row + rows - 1];
    }
    // Solve the problem halfway. This returns 0 if it works,
    // otherwise the correct number of rows is returned and we try once more.
    if (!(rows = wipe(m)))
    {
      rows = m.size1();
      for (int row = rows - 1; row >= 0; --row)
      {
        // We should have exactly one non-zero value in the first 'rows' columns that devides the last column.
        mpz_class x = 0, y;
        int c;
        for (unsigned int col = 0; col < rows; ++col)
        {
          y = m(row, col);
          if (y != 0)
          {
            if (x != 0)
              return false;     // Failure.
            x = y;
            c = col;
          }
        }
        if (x == 0 || m(row, rows) % x != 0)
          return false;         // Failure.
        m(row, rows) /= x;
        m(row, c) = 1;
        for (int row2 = row - 1; row2 >= 0; --row2)
        {
          m(row2, rows) -= m(row, rows) * m(row2, c);
          m(row2, c) = 0;
        }
      }
      // We found a solution!
      for (unsigned int row = 0; row < rows; ++row)
      {
        assert(m(row, rows).fits_slong_p());
        solution.push_back(m(row, rows).get_si());
      }
      // Or did we?
      for (int n = n_begin + solution.size() - 1; n < n_end; ++n)
      {
        unsigned long an = solution[0];
	for (unsigned int row = 1; row < rows; ++row)
	  an += solution[row] * a[n - row];
        if (an != a[n])
	  return false;
      }
      break;
    }
  }
  return true;
}

// The format used in the table 'brute_force_sequences'.
struct BruteForceSequences {
  char code;
  char const* description;
  unsigned long terms[31];
};

// Default sequences.
BruteForceSequences brute_force_sequences[] = {
  { 'A', "Number of Frobenius equivalence classes (foc(m) / m = A001037)", { 2, 1, 2, 3, 6, 9, 18, 30, 56, 99, 186, 335, 630, 1161, 2182, 4080, 7710, 14532, 27594, 52377, 99858, 190557, 364722, 698870, 1342176, 2580795, 4971008, 9586395, 18512790, 35790267, 69273666 } },
  { 'B', "Number of Frobenius equivalence classes with trace 1 (blwt1(m) = A000048 (see also proposition 3))", { 1, 1, 1, 2, 3, 5, 9, 16, 28, 51, 93, 170, 315, 585, 1091, 2048, 3855, 7280, 13797, 26214, 49929, 95325, 182361, 349520, 671088, 1290555, 2485504, 4793490, 9256395, 17895679, 34636833 } },
  { 'C', "Number of Frobenius equivalence classes that are their own inverse (blwt1(m/2) (propositions 2 and 5))", { 1, 1, 0, 1, 0, 1, 0, 2, 0, 3, 0, 5, 0, 9, 0, 16, 0, 28, 0, 51, 0, 93, 0, 170, 0, 315, 0, 585, 0, 1091, 0 } },
  { 'D', "Number of Frobenius equivalence classes that are their own inverse with trace 1 (depends on w(m/2, 1)*)", { 1, 1, 0, 1, 0, 0, 0, 1, 0, 2, 0, 2, 0, 4, 0, 9, 0, 14, 0, 24, 0, 48, 0, 86, 0, 154, 0, 294, 0, 550, 0 } },
 // { 'E', "Number of generators (euler_phi(2^m - 1) / m = A011260)", { 1, 1, 2, 2, 6, 6, 18, 16, 48, 60, 176, 144, 630, 756, 1800, 2048, 7710, 7776, 27594, 24000, 84672, 120032, 356960, 276480, 1296000, 1719900, 4202496, 4741632, 18407808, 17820000, 69273666 } },
 // { 'F', "Number of generators with trace 1 (no formula)", { 1, 1, 1, 1, 3, 4, 9, 7, 25, 31, 87, 72, 315, 381, 901, 1017, 3855, 3890, 13797, 12000, 42344, 60043, 178431, 138224, 648031, 860059, 2101353, 2370715, 9203747, 8908940, 34636833 } },
 // { 'G', "Number of normal basis (num_normal(m) = A027362)", { 1, 1, 1, 2, 3, 4, 7, 16, 21, 48, 93, 128, 315, 448, 675, 2048, 3825, 5376, 13797, 24576, 27783, 95232, 182183, 262144, 629145, 1290240, 1835001, 3670016, 9256395, 11059200, 28629151 } },
 // { 'H', "Number of normal basis that exist of generators (no formula, A107222)", { 1, 1, 1, 1, 3, 3, 7, 7, 19, 29, 87, 52, 315, 291, 562, 1017, 3825, 2870, 13797, 11255, 23579, 59986, 178259, 103680, 607522, 859849, 1551227, 1815045, 9203747, 5505966, 28629151 } },
  { 'I', "Number of solutions, w(m, 1) (this is what we want to know) (A137503)", { 1, 1, 0, 1, 4, 3, 8, 16, 28, 45, 96, 167, 308, 579, 1100, 2018, 3852, 7280, 13776, 26133, 49996, 95223, 182248, 349474, 671176, 1289925, 2485644, 4793355, 9255700, 17894421, 34638296 } },
  { 'J', "Number of solutions with trace 1 (no formula)", { 1, 1, 0, 1, 2, 2, 4, 9, 14, 24, 48, 86, 154, 294, 550, 1017, 1926, 3654, 6888, 13092, 24998, 47658, 91124, 174822, 335588, 645120, 1242822, 2396970, 4627850, 8947756, 17319148 } },
 // { 'K', "Number of solutions that are generators (no formula)", { 1, 1, 0, 0, 4, 2, 8, 10, 26, 26, 94, 76, 308, 378, 902, 1014, 3852, 3908, 13776, 11944, 42460, 60014, 178414, 138360, 648042, 859642, 2101290, 2370858, 9202954, 8910288, 34638296 } },
 // { 'L', "Number of solutions that are generators with trace 1 (no formula)", { 1, 1, 0, 0, 2, 2, 4, 4, 14, 14, 46, 38, 154, 192, 452, 500, 1926, 1956, 6888, 5972, 21238, 30034, 89158, 69164, 324052, 429930, 1050750, 1185328, 4601320, 4454084, 17319148 } },
 // { 'M', "Number of solutions that are normal basis (no formula)", { 1, 1, 0, 1, 2, 2, 4, 9, 12, 22, 48, 62, 154, 224, 332, 1017, 1912, 2712, 6888, 12286, 13912, 47610, 91028, 131094, 314584, 644966, 917600, 1835144, 4627850, 5529896, 14314976 } },
 // { 'N', "Number of solutions that are normal basis and generators (no formula)", { 1, 1, 0, 0, 2, 2, 4, 4, 12, 13, 46, 25, 154, 145, 277, 500, 1912, 1459, 6888, 5603, 11876, 30005, 89062, 51888, 303761, 429829, 775566, 907684, 4601320, 2753361, 14314976 } }
};
unsigned int const number_of_brute_force_sequences = sizeof(brute_force_sequences) / sizeof(BruteForceSequences);

enum split_nt {
  not_split,
  odd_n,
  even_n
};

class Filter {
  public:
    virtual unsigned int loop_end(void) = 0;	// Returns one beyond the maximum loop counter value.

    virtual bool split_apply(unsigned long* terms, int n_begin, int n_end, int loop_counter, split_nt& split);	// Returns true if successful.
    virtual void split_print_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split);
    virtual void split_print_inverse_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split);

    virtual bool apply(unsigned long* terms, int n_begin, int n_end, int loop_counter) = 0;
    virtual void print_formula_on(std::ostream& os, char sn, int loop_counter) = 0;
    virtual void print_inverse_formula_on(std::ostream& os, char sn, int loop_counter) = 0;
};

bool Filter::split_apply(unsigned long* terms, int n_begin, int n_end, int loop_counter, split_nt& split)
{
  if (split == not_split)
    return apply(terms, n_begin, n_end, loop_counter);
  bool success;
  if ((n_begin & 1) == 0)	// n_begin is even?
  {
    unsigned int number_of_even_terms = (n_end - n_begin + 1) / 2;
    unsigned int number_of_odd_terms = (n_end - n_begin) / 2;
    n_begin /= 2;
    success = apply(terms, n_begin, n_begin + number_of_even_terms, loop_counter);
    terms += n_begin + number_of_even_terms;
    success = apply(terms, n_begin, n_begin + number_of_odd_terms, loop_counter) && success;
  }
  else				// n_begin is odd.
  {
    unsigned int number_of_even_terms = (n_end - n_begin) / 2;
    unsigned int number_of_odd_terms = (n_end - n_begin + 1) / 2;
    n_begin = (n_begin - 1) / 2;
    if (n_begin == 0)
    {
      n_begin = 1;
      --number_of_odd_terms;
    }
    success = apply(terms, n_begin, n_begin + number_of_odd_terms, loop_counter);
    terms += n_begin + number_of_odd_terms;
    success = apply(terms, n_begin, n_begin + number_of_even_terms, loop_counter) && success;
  }
  return success;
}

void Filter::split_print_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split)
{
  if (split == even_n)
    print_formula_on(os, sn + 13, loop_counter);
  else
    print_formula_on(os, sn, loop_counter);
}

void Filter::split_print_inverse_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split)
{
  if (split == even_n)
    print_inverse_formula_on(os, sn + 13, loop_counter);
  else
    print_inverse_formula_on(os, sn, loop_counter);
}

class Split : public Filter {
  public:
    virtual unsigned int loop_end(void) { return 2; }
    virtual bool split_apply(unsigned long* terms, int n_begin, int n_end, int loop_counter, split_nt& split);
    virtual void split_print_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split);
    virtual void split_print_inverse_formula_on(std::ostream& os, char sn, int loop_counter, split_nt split);
    virtual bool apply(unsigned long*, int, int, int) { return false; /* dummy */ }
    virtual void print_formula_on(std::ostream&, char, int) { /* dummy */ }
    virtual void print_inverse_formula_on(std::ostream&, char, int) { /* dummy */ }
};

class multiply_with_n : public Filter {
  public:
    virtual unsigned int loop_end(void) { return 2; }
    virtual bool apply(unsigned long* terms, int n_begin, int n_end, int loop_counter);		// Returns true if successful.
    virtual void print_formula_on(std::ostream& os, char sn, int loop_counter);
    virtual void print_inverse_formula_on(std::ostream& os, char sn, int loop_counter);
};

class add_divisors : public Filter {
  private:
    bool no_n;
  public:
    virtual unsigned int loop_end(void) { return 4; }
    virtual bool apply(unsigned long* terms, int n_begin, int n_end, int loop_counter);		// Returns true if successful.
    virtual void print_formula_on(std::ostream& os, char sn, int loop_counter);
    virtual void print_inverse_formula_on(std::ostream& os, char sn, int loop_counter);
};

class phi_inv : public Filter {
  private:
    unsigned int M_loop_end;
  public:
    phi_inv(void) : M_loop_end(2) { }	// The real value is the first time apply is called (with loop_counter == 1).
    virtual unsigned int loop_end(void) { return M_loop_end; }
    virtual bool apply(unsigned long* terms, int n_begin, int n_end, int loop_counter);		// Returns true if successful.
    virtual void print_formula_on(std::ostream& os, char sn, int loop_counter);
    virtual void print_inverse_formula_on(std::ostream& os, char sn, int loop_counter);
};

std::vector<Filter*> filters;

int main(int argc, char* argv[])
{
  program_name = argv[0];

  initialize_utils();

  // Default values for the table.
  int n_begin = 1;	
  int number_of_terms = 31;
  bool use_table = true;

  // Vector used to buffer a possible commandline sequence.
  input_type input;

  // There must be at least two terms.
  if (argc == 2)
  {
    cout << "argc = " << argc << endl;
    cout << "Usage: " << program_name << " <n_begin> <term>,<term>,<term>,<term>[,<term>...]" << endl;
    return 1;
  }
  else if (argc > 1)
  {
    use_table = false;

    // The first argument must be the index of the first term.
    n_begin = atoi(argv[1]);

    // Catenate the rest.
    std::string sequence_str;
    for (int i = 2; i < argc; ++i)
    {
      sequence_str += ' ';
      sequence_str += argv[i];
    }
    // Replace all comma's with spaces.
    std::transform(sequence_str.begin(), sequence_str.end(), sequence_str.begin(), CommaToSpace());
    // Read the sequence into a vector.
    std::stringstream buf;
    buf << sequence_str;
    input_type::value_type term;
    while (buf >> term)
      input.push_back(term);
    number_of_terms = input.size();

    cout << "Input: ";
    unsigned int n = n_begin;
    for (input_type::const_iterator iter = input.begin(); iter != input.end(); ++iter, ++n)
      cout << "a(" << n << ") = " << *iter << "; ";
    cout << '\n';
  }

  filters.push_back(new multiply_with_n);
  filters.push_back(new add_divisors);
  filters.push_back(new Split);
//  filters.push_back(new phi_inv);

  unsigned int const number_of_filters = filters.size();

  // If there is a sequence given on the command line try that, otherwise try every sequence given in the table.
  for (unsigned int sequence = 0; sequence < (use_table ? number_of_brute_force_sequences : 1); ++sequence)
  {
    if (use_table)
    {
      // Print a little header before each sequence from the table.
      if (sequence != 0)
        cout << endl;
      cout << brute_force_sequences[sequence].code << ": " << brute_force_sequences[sequence].description << ":\n";
    }

    // Make a copy.
    unsigned long terms[n_begin + number_of_terms];
    for (int n = n_begin; n < n_begin + number_of_terms; ++n)
      terms[n] = use_table ? brute_force_sequences[sequence].terms[n - n_begin] : input[n - n_begin];
    split_nt split = not_split;

    char sn = 'a';
    for (MultiLoop ml(number_of_filters); !ml.finished(); ml.next_loop())
    {
      for(; ml() < filters[*ml]->loop_end(); ++ml)
      {
        if (*ml == number_of_filters - 1)		// Inner most loop.
	{
	  // Make a list of all filters that are currently turned on.
	  std::vector<Filter*> current_filters;
	  std::vector<int> current_loop_counters;
	  int filter_index = 0;
	  for (std::vector<Filter*>::iterator filter = filters.begin(); filter != filters.end(); ++filter, ++filter_index)
	    if (ml[filter_index] != 0)
	    {
	      current_filters.push_back(*filter);
	      current_loop_counters.push_back(ml[filter_index]);
	    }
	  // Generate the first permutation of the order in which to apply these filters.
	  std::vector<int> permutation(current_filters.size());
	  for (unsigned int i = 0; i < permutation.size(); ++i)
	    permutation[i] = i;
	  // Run over all permutations.
	  do
	  {
	    if (sn != 'a')	// Applied filters before?
	    {
	      // Put back the original sequence.
	      for (int n = n_begin; n < n_begin + number_of_terms; ++n)
		terms[n] = use_table ? brute_force_sequences[sequence].terms[n - n_begin] : input[n - n_begin];
	      split = not_split;

	      // Apply the current filters in the order given by permutation.
	      bool success = true;
	      sn = 'a';
	      for (std::vector<int>::const_iterator perm = permutation.begin(); perm != permutation.end(); ++perm)
	      {
	        ++sn;
		if (!current_filters[*perm]->split_apply(terms, n_begin, n_begin + number_of_terms, current_loop_counters[*perm], split))
		{
		  success = false;
		  break;
	        }
	        cout << "Let ";
		current_filters[*perm]->split_print_formula_on(cout, sn, current_loop_counters[*perm], split);
		cout << '\n';
	      }
	      if (!success)
	        continue;	// Try next permutation.
	    }

	    unsigned long* split_terms = terms;
	    int split_n_begin = (split == not_split) ? n_begin : (n_begin / 2);
	    unsigned int split_number_of_terms = (split == not_split) ? number_of_terms : ((number_of_terms + 1) / 2);	// The first batch of terms.
	    unsigned int split_number_of_terms_next = number_of_terms / 2;						// The second batch of terms.

	    for (int batch = 0; batch < 2; ++batch)
	    {
	      if (batch == 1)
	      {
	        split_n_begin = (n_begin + 1) / 2;
		split_terms += split_number_of_terms - ((split == even_n) ? 0 : 1);
		split_number_of_terms = split_number_of_terms_next;
		split = (((n_begin + 1) & 1) ? odd_n : even_n);
	      }

	      int split_n_end = split_n_begin + split_number_of_terms;
	      char split_sn = (split == even_n) ? (sn + 13) : sn;

	      // Print out the sequence that we're going to try and find a linear dependency for.
	      cout << "Trying to solve " << split_sn << "(n = " << split_n_begin << "...) = ";
	      for (int n = split_n_begin; n < split_n_end; ++n)
		cout << split_terms[n] << ", ";
	      cout << "...\n";

#if 0
	      // Guess if this is a sequence of elements (2^(m - c0) + epsilon), or of FEC (2^(m - c0) / m + epsilon).
	      // We do this by taking the logarithm.
	      std::vector<double> input_log2(split_n_end - split_n_begin);
	      std::transform(split_terms + split_n_begin, split_terms + split_n_end, input_log2.begin(), Log2());
	      // The 'x-axis' values.
	      std::vector<double> x(split_n_end - split_n_begin);
	      std::generate(x.begin(), x.end(), PlusOne(split_n_begin));
	      // Then fit a straight line through this, weighting the points with the original terms (larger values are more important).
	      std::vector<double> weight;
	      for (int n = split_n_begin; n < split_n_end; ++n)
	      {
		if (split_terms[n] <= 2)
		  weight.push_back(0);
		else
		{
		  double delta = log2((split_terms[n] + 1.0 + sqrt(split_terms[n])) / (split_terms[n] - 1.0 - sqrt(split_terms[n])));
		  weight.push_back(1.0 / (delta * delta));
		}
	      }
	      // Fit a weighted line.
	      Line line = fit_line(x.begin(), x.end(), input_log2.begin(), weight.begin());
	      cout << "Line fit: " << sn << "(n) = " << exp2(line.a) << "^n * " << exp2(line.b) << endl;
#endif

	      std::vector<long> solution;
	      if (solve(split_terms, split_n_begin, split_n_end, solution))
	      {
	        if (split == not_split)
		  cout << "* Found!\n* ";
		else if (split == odd_n)
		  cout << "* Found for odd n!\n* ";
		else
		  cout << "* Found for even n!\n* ";
		for (unsigned int n = split_n_begin; n < split_n_begin + solution.size() - 1; ++n)
		  cout << split_sn << "(" << n << ") = " << split_terms[n] << "; ";
		cout << split_sn << "(n) = ";
		for (unsigned int i = 0; i < solution.size(); ++i)
		{
		  if (i == 0)
		    cout << solution[0];
		  else
		  {
		    long f = solution[i];
		    if (f == 0)
		      continue;
		    if (f < 0)
		    {
		      cout << " - ";
		      f = -f;
		    }
		    else
		      cout << " + ";
		    if (f > 1)
		      cout << f << ' ';
		    cout << split_sn << "(n - " << i << ')';
		  }
		}
		cout << endl;
		for (std::vector<int>::const_reverse_iterator perm = permutation.rbegin(); perm != permutation.rend(); ++perm)
		{
		  cout << "* ";
		  current_filters[*perm]->split_print_inverse_formula_on(cout, sn, current_loop_counters[*perm], split);
		  --split_sn;
		  cout << '\n';
		}
		//goto solved;
	      }
	      if (split == not_split)
		break;
	    }
	    sn = 'b';	// Make sure we apply filters the next loop.
	  }
	  while (std::next_permutation(permutation.begin(), permutation.end()));
	}
      }
    }
//solved:
    while(0);
    // Next sequence.
  }
}

void Split::split_print_formula_on(std::ostream& os, char b, int, split_nt split)
{
  char a = b - 1;
  if (split == even_n)
    b += 13;
  if (split == odd_n)
    os << b << "(n) = " << a << "(2n + 1)";
  else
    os << b << "(n) = " << a << "(2n)";
}

void Split::split_print_inverse_formula_on(std::ostream& os, char b, int, split_nt split)
{
  char a = b - 1;
  if (split == even_n)
    b += 13;
  if (split == odd_n)
    os << a << "(n) = " << b << "((n - 1) / 2), n odd";
  else
    os << a << "(n) = " << b << "(n / 2), n even";
}

bool Split::split_apply(unsigned long* terms, int n_begin, int n_end, int, split_nt& split)
{
  int first_n_begin = n_begin / 2;
  int first_number_of_terms = (n_end - n_begin + 1) / 2;
  int second_n_begin = first_n_begin + first_number_of_terms;
  int second_number_of_terms = (n_end - n_begin) / 2;
  unsigned long second_entries[second_number_of_terms];
  for (int i = 0; i < second_number_of_terms; ++i)
    second_entries[i] = terms[n_begin + 1 + 2 * i];
  for (int i = 0; i < first_number_of_terms; ++i)
    terms[first_n_begin + i] = terms[n_begin + 2 * i]; 
  for (int i = 0; i < second_number_of_terms; ++i)
    terms[second_n_begin + i] = second_entries[i];
  split = (n_begin & 1) ? odd_n : even_n;
  return true;
}

void multiply_with_n::print_formula_on(std::ostream& os, char b, int)
{
  char a = b - 1;
  os << b << "(n) = n * " << a << "(n)";
}

void multiply_with_n::print_inverse_formula_on(std::ostream& os, char b, int)
{
  char a = b - 1;
  os << a << "(n) = " << b << "(n) / n";
}

bool multiply_with_n::apply(unsigned long* terms, int n_begin, int n_end, int)
{
  for (int n = n_begin; n < n_end; ++n)
    terms[n] *= n;
  return true;
}

void add_divisors::print_formula_on(std::ostream& os, char b, int loop_counter)
{
  char a = b - 1;
  os << b << "(n) = " << a << "(n) + \\sum_{";
  if (no_n)
    os << "1<d<n";
  else
   os << "1<d<=n";
  if (loop_counter == 1)
    os << ",odd d";
  if (loop_counter == 2)
    os << ",even d";
  os << ",d|n} { " << a << "(n/d) }";
}

void add_divisors::print_inverse_formula_on(std::ostream& os, char b, int loop_counter)
{
  char a = b - 1;
  os << a << "(n) = " << b << "(n) - \\sum_{";
  if (no_n)
    os << "1<d<n";
  else
   os << "1<d<=n";
  if (loop_counter == 1)
    os << ",odd d";
  if (loop_counter == 2)
    os << ",even d";
  os << ",d|n} { " << a << "(n/d) }";
}

bool add_divisors::apply(unsigned long* terms, int n_begin, int n_end, int loop_counter)
{
  // loop_counter
  // 1:	odd divisors.
  // 2: even divisors.
  // 3: both.
  if (n_begin > 2)
    return false;
  no_n = (n_begin == 1);
  for (int n = n_end - 1; n >= n_begin; --n)
  {
    for (int d = 2; d <= (no_n ? n - 1 : n); ++d)
    {
      int eo = 2 - (d & 1);
      if ((eo & loop_counter) && n % d == 0)
	terms[n] += terms[n/d];
    }
  }
  return true;
}

void phi_inv::print_formula_on(std::ostream& os, char b, int loop_counter)
{
  char a = b - 1;
  os << b << "(n) = euler_phi_inv(" << a << "(n)) #" << loop_counter;
}

void phi_inv::print_inverse_formula_on(std::ostream& os, char b, int)
{
  char a = b - 1;
  os << a << "(n) = euler_phi(" << b << "(n))";
}

bool phi_inv::apply(unsigned long* terms, int n_begin, int n_end, int loop_counter)
{
  for (int n = n_begin; n < n_end; ++n)
  {
    unsigned long term = terms[n];
    if ((term & 1) == 1 && term != 1)
      return false;
  }
  if (loop_counter == 1)
  {
    M_loop_end = 1;
    for (int n = n_begin; n < n_end; ++n)
    {
      std::vector<unsigned long> result;
      cout << "Calculating euler_phi_inv(" << terms[n] << ");";
      euler_phi_inv(terms[n], result);
      cout << " number of solutions: " << result.size() << ": ";
      unsigned int count = 1;
      for (std::vector<unsigned long>::iterator iter = result.begin(); iter != result.end(); ++iter, ++count)
      {
        cout << *iter << ", ";
	if (count == 9)
	{
	  cout << "...";
	  break;
	}
      }
      cout << '\n';
      M_loop_end *= result.size();
      if (M_loop_end == 0)
      {
        M_loop_end = 2;
        return false;
      }
    }
    ++M_loop_end;
  }
  else if (M_loop_end == 0)
    return false;
  cout << "WE GET HERE with M_loop_end = " << M_loop_end << endl;
  return true;
}
