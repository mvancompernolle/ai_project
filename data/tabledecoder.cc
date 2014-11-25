// This program calculates the factorization of 2^n - 1 for n < 1200.
//
// For this it uses the Cunningham Project tables,
// see http://www.cerias.purdue.edu/homes/ssw/cun/
//
// There are four tables needed:

enum table_nt {
  none = -1,
  table_2minus = 0,	// "Factorizations of 2^n-1, n odd, n<1200"
  table_2plus_odd = 1,	// "Factorizations of 2^n+1, n odd, n<1200"
  table_2LM = 2,	// "Factorizations of 2^n+1, n=4k-2, n<2400"
  table_2plus_4k = 3	// "Factorizations of 2^n+1, n=4k, n<=1200"
};

// These tables are read from the file 'pmainDATE', which
// is a copy of http://www.cerias.purdue.edu/homes/ssw/cun/
// Main Tables, Table 2*.
//
// This file is considered to exist of:

enum lines_nt {
  white_space,		// A line existing entirely of whitespace (space, tab, formfeed and newline).
  title,		// A line starting with "Table 2" (preceeded by white space).
  header,		// A line, following the title, starting with 'n' (preceeded by white space).
  entry,		// A line starting with the value of 'n', such lines follow the header and start with a digit.
  entry_continuation,	// A line starting with a '.', such lines can follow an entry.
  footer		// A line at the end of a table, following an empty line, before a new title line.
};

// Depending on the 'state' of the decoder, only certain types of lines are expected.
enum state_nt {
  before_table,			// white_space, title and footer (in case we're actually after_table, see below).
  after_title,			// white_space and header.
  expecting_entry,		// entry or white_space.
  expecting_continuation,	// entry_continuation.
  after_table = before_table
};

// The entries exist of the following:
//
// <spaces><line>[.|\]<new_line>
//
// The end of a line can be used to determine if the next line is an entry_continuation.

enum last_nt {
  last_dot,		// The previous line did end on a '.', this line is an entry_continuation.
  last_star,		// The previous line did end on a '*', this line is (assumed) not (to be) an entry_continuation.
  last_brace,		// The previous line did end on a ')', this line is an entry_continuation.
  last_space,		// The previous line did end on a ' ', this line is an entry_continuation.
  last_backslash,	// The previous line did end on a '\\', this line is an entry_continuation.
  last_digit,		// The previous line did end on a digit, this line cannot be an entry_continuation.
  last_other		// The previous line did end on none of the above; this is not an entry or entry_continuation.
};

// A continuation entry exist of leading spaces followed by a part that
// should be catenated to the previous line, after deleting the last (non-digit)
// character of that line.  The resulting <line> exist of the following:
//
// <prefix><spaces>[(<reference list>)][<spaces>][<factors>]
//
// The <prefix> can be of three types:

enum prefix_nt {
  plain,		// A series of digits, representing the value of 'n'.
  L_prefix,		// A series of digits, representing the value of 'n', followed by a 'L'.
  M_prefix		// The character 'M'.
};

#include <iostream>
std::ostream& operator<<(std::ostream& os, prefix_nt p)
{
  switch(p)
  {
    case plain:
      os << "plain";
      break;
    case L_prefix:
      os << "L_prefix";
      break;
    case M_prefix:
      os << "M_prefix";
      break;
  }
  return os;
}

// The L_prefix and M_prefix only occur in table_2LM.
//
// The optional <reference list> exists of a comma seperated list of references.
// A reference exists of the value of 'n' optionally followed by an 'L' or 'M',
// they refer to a previous entry in the same table.  Therefore the tables need
// search keys that include both 'n' as prefix_nt:

class prefix_ct {
  private:
    unsigned int M_n;	// The exponent of the power.  See table_nt.
    prefix_nt M_kind;	// The kind of entry; should always be `plain' except for table_2LM.
  public:
    // Constructors.
    prefix_ct() { }
    prefix_ct(unsigned int n, prefix_nt kind = plain) : M_n(n), M_kind(kind) { }
    prefix_ct(prefix_ct const& prefix) : M_n(prefix.M_n), M_kind(prefix.M_kind) { }
    prefix_ct& operator=(prefix_ct const& prefix) { M_n = prefix.M_n; M_kind = prefix.M_kind; return *this; }
    // Operators.
    friend bool operator==(prefix_ct const& prefix1, prefix_ct const& prefix2)
	{ return (prefix1.M_n == prefix2.M_n) && (prefix1.M_kind == prefix2.M_kind); }
    friend bool operator!=(prefix_ct const& prefix1, prefix_ct const& prefix2)
	{ return (prefix1.M_n != prefix2.M_n) || (prefix1.M_kind != prefix2.M_kind); }
    friend bool operator<(prefix_ct const& prefix1, prefix_ct const& prefix2)
        { return (prefix1.M_n < prefix2.M_n) || (prefix1.M_n == prefix2.M_n && prefix1.M_kind < prefix2.M_kind); }
    friend bool operator>(prefix_ct const& prefix1, prefix_ct const& prefix2)
        { return (prefix1.M_n > prefix2.M_n) || (prefix1.M_n == prefix2.M_n && prefix1.M_kind > prefix2.M_kind); }
    friend bool operator<=(prefix_ct const& prefix1, prefix_ct const& prefix2)
        { return (prefix1.M_n < prefix2.M_n) || (prefix1.M_n == prefix2.M_n && prefix1.M_kind <= prefix2.M_kind); }
    friend bool operator>=(prefix_ct const& prefix1, prefix_ct const& prefix2)
        { return (prefix1.M_n > prefix2.M_n) || (prefix1.M_n == prefix2.M_n && prefix1.M_kind >= prefix2.M_kind); }
    // Accessors.
    unsigned int get_n(void) const { return M_n; }
    prefix_nt get_kind(void) const { return M_kind; }
    // Debugging
    friend std::ostream& operator<<(std::ostream& os, prefix_ct const& prefix)
    {
      os << prefix.M_n << ' ' << prefix.M_kind;
      return os;
    }
};

#include <vector>
#include "tabledecoder.h"

// The <reference list> is then a list of prefixes.
typedef std::vector<prefix_ct> reference_list_type;

// The <factors> list is then a list of these objects:
typedef std::vector<factor_ct> factor_list_type;

// Each table entry then has the following data:
struct data_st {
  reference_list_type reference_list;
  factor_list_type factor_list;
};

#include <map>
// The tables have the following type:
typedef std::map<prefix_ct, data_st> table_type;

// The tables are singletons that can only be accessed through this function:
inline table_type& table(table_nt t)
{
  static table_type table[4];
  return table[(int)t];
}

#include <fstream>
#include <cstring>
#include <cctype>
#include <string>
#include <cstdlib>

class Tokenizer {
  private:
    std::string const& M_input;
    char M_delimiter;
    int M_pos;
    bool M_done;
  public:
    Tokenizer(std::string const& input, char delimiter) : M_input(input), M_delimiter(delimiter), M_pos(0), M_done(false) { }
    std::string nextToken(void)
    {
      if (M_done)
	return "";
      std::string::size_type end = M_input.find_first_of(M_delimiter, M_pos);
      if (end == std::string::npos)
	M_done = true;
      std::string token(M_input, M_pos, end - M_pos);
      M_pos = end + 1;
      return token;
    }
};

#include <cassert>

// Decode <prefix><spaces>[(<reference list>)][<spaces>][<factors>]
void decode(table_nt table_index, std::string const& input)
{
  static unsigned int last_n;
  unsigned int n;
  prefix_nt prefix_kind = plain;

  int pos = 0;
  if (input[0] == 'M')
  {
    n = last_n;
    prefix_kind = M_prefix;
    ++pos;
  }
  else
  {
    n = std::atoi(input.data());
    last_n = n;
    while(std::isdigit(input[pos]))
      ++pos;
    if (input[pos] == 'L')
    {
      prefix_kind = L_prefix;
      ++pos;
    }
  }
  while(std::isspace(input[pos]))
    ++pos;
  prefix_ct prefix(n, prefix_kind);
  data_st data;
  if (input[pos] == '(')
  {
    int end = input.find_first_of(')', pos);
    std::string reference_list_string(input, pos + 1, end - pos - 1);
    Tokenizer tokenizer(reference_list_string, ',');
    for(;;)
    {
      std::string token(tokenizer.nextToken());
      if (token.size() == 0)
	break;
      char last_char = token[token.length() - 1];
      prefix_ct prefix(std::atoi(token.c_str()), (last_char == 'L') ? L_prefix : (last_char == 'M') ? M_prefix : plain);
      data.reference_list.push_back(prefix);
    }
    pos = end + 1;
    while(std::isspace(input[pos]))
      ++pos;
  }
  std::string factors_string(input, pos);
  Tokenizer tokenizer(factors_string, '.');
  for(;;)
  {
    std::string token(tokenizer.nextToken());
    if (token.size() == 0)
      break;
    char last_char = token[token.length() - 1];
    bool last_char_is_star = last_char == '*';
    if (last_char_is_star)
      token.erase(token.end() - 1);
    if (token[0] == ' ')
    {
      int pos = token.find_first_not_of(' ');
      token.erase(0, pos);
    }
    factor_type_nt factor_type = plain_factor;
    if (token[0] == 'P' || token[0] == 'C')
    {
      factor_type = (token[0] == 'P') ? large_prime : composite;
      mpz_class product(1);
      for (reference_list_type::iterator iter = data.reference_list.begin(); iter != data.reference_list.end(); ++iter)
      {
	prefix_ct& reference_prefix(*iter);
	table_type::iterator iter = table(table_index).find(reference_prefix);
	assert( iter != table(table_index).end() );
	for (factor_list_type::iterator i = iter->second.factor_list.begin(); i != iter->second.factor_list.end(); ++i)
	  product *= i->get_factor();
      }
      for (factor_list_type::iterator iter = data.factor_list.begin(); iter != data.factor_list.end(); ++iter)
	product *= iter->get_factor();
      mpz_class total, two(2);
      if (table_index != table_2LM)
      {
	mpz_pow_ui(total.get_mpz_t(), two.get_mpz_t(), n);
	total += ((table_index == table_2minus) ? -1 : 1);
      }
      else
      {
	// n=4k-2
	// 2^{2h}+1=L.M, L=2^h-2^k+1, M=2^h+2^k+1, h=2k-1.
	unsigned int k = (n + 2) / 4;
	unsigned int h = 2 * k - 1;
	mpz_class pwrh, pwrk;
	mpz_pow_ui(pwrh.get_mpz_t(), two.get_mpz_t(), h);
	mpz_pow_ui(pwrk.get_mpz_t(), two.get_mpz_t(), k);
	if (prefix_kind == L_prefix)
	  total = pwrh - pwrk + 1;
	else
	  total = pwrh + pwrk + 1;
      }
      mpz_class prime = total / product;
      factor_ct factor(prime, factor_type, last_char_is_star);
      data.factor_list.push_back(factor);
    }
    else
    {
      mpz_class number(token, 10);
      assert( number != 0 );
      factor_ct factor(number, factor_type, last_char_is_star);
      data.factor_list.push_back(factor);
    }
  }
  table(table_index).insert(table_type::value_type(prefix, data));
}

void initialize_tables(void)
{
  static last_nt last_was = last_other;
  std::fstream input;
  input.open((std::string(SRCDIR) + "/pmainDATE").c_str());
  if (!input)
  {
    std::cerr << "Cannot open " << (std::string(SRCDIR) + "pmainDATE") << std::endl;
    exit(1);
  }
  char buf[1024];
  std::string line;
  state_nt state = before_table;
  table_nt table = none;
  bool success;
  do
  {
    last_nt prev_last = last_was;
    success = input.getline(buf, sizeof(buf));
    if (success)
    {
      char* line_start = buf;
      if (*line_start == '\f')
	state = after_table;
      // Eat all leading white space.
      while(std::isspace(*line_start))
	++line_start;
      size_t line_length = std::strlen(line_start);
      if (line_length == 0)
        continue;
      char last_char = line_start[line_length - 1];
      switch (last_char)
      {
	case '.':
	  last_was = last_dot;
	  break;
	case '*':
	  last_was = last_star;
	  break;
	case ')':
	  last_was = last_brace;
	  break;
	case ' ':
	  last_was = last_space;
	  break;
	case '\\':
	  last_was = last_backslash;
	  break;
	default:
	  last_was = std::isdigit(last_char) ? last_digit : last_other;
	  break;
      }
      switch(state)
      {
	case before_table:
	  if (*line_start == 'T')
	  {
	    state = after_title;
	    if (table == none)
	      table = table_2minus;
	    else if (table == table_2minus)
	      table = table_2plus_odd;
	    else if (table == table_2plus_odd)
	      table = table_2LM;
	    else if (table == table_2LM)
	      table = table_2plus_4k;
	    else if (table == table_2plus_4k)
	    {
	      success = false;
	      break;
	    }
          }
	  break;
	case after_title:
	  if (*line_start == 'n')
	    state = expecting_entry;
	  break;
	case expecting_entry:
	  line = line_start;
	  if (last_was == last_dot || last_was == last_backslash)
	  {
	    state = expecting_continuation;
	    line.erase(line.end() - 1);
	  }
	  if (last_was == last_brace || last_was == last_space)
	    state = expecting_continuation;
	  if (state != expecting_continuation)
	    decode(table, line);
	  break;
	case expecting_continuation:
	  assert( prev_last != last_dot || line_start[0] == '.');
	  assert( prev_last != last_backslash || std::isdigit(line_start[0]) );
	  assert( prev_last != last_space || std::isdigit(line_start[0]) );
	  assert( prev_last != last_brace || std::isdigit(line_start[0]) );
	  assert( prev_last != last_star && prev_last != last_digit && prev_last != last_other );
	  line += line_start;
	  if (last_was == last_dot || last_was == last_backslash)
	    line.erase(line.end() - 1);
	  else
	  {
	    decode(table, line);
	    state = expecting_entry;
	  }
	  break;
      }
    }
  }
  while(success);
  input.close();
}

void add_factors(std::vector<factor_ct>& output, table_nt table_index, data_st const& data)
{
  for (reference_list_type::const_iterator iter = data.reference_list.begin(); iter != data.reference_list.end(); ++iter)
  {
    prefix_ct const& reference_prefix(*iter);
    table_type::const_iterator iter = table(table_index).find(reference_prefix);
    assert( iter != table(table_index).end() );
    for (factor_list_type::const_iterator i = iter->second.factor_list.begin(); i != iter->second.factor_list.end(); ++i)
      output.push_back(*i);
  }
  for (factor_list_type::const_iterator iter = data.factor_list.begin(); iter != data.factor_list.end(); ++iter)
    output.push_back(*iter);
}

void list_factors(std::vector<factor_ct>& output, table_nt table_index, unsigned int n)
{
  table_type& t(table(table_index));
  prefix_ct prefix(n);
  table_type::iterator iter = t.find(prefix);
  if (iter != t.end())
    add_factors(output, table_index, iter->second);
  else
  {
    assert( table_index = table_2LM );
    iter = t.find(prefix_ct(n, L_prefix));
    assert( iter != t.end() );
    add_factors(output, table_index, iter->second);
    iter = t.find(prefix_ct(n, M_prefix));
    assert( iter != t.end() );
    add_factors(output, table_index, iter->second);
  }
}

void list_factors_plus(std::vector<factor_ct>& output, unsigned int n)
{
  if ((n & 1) == 1)
    list_factors(output, table_2plus_odd, n);
  else
  {
    if ((n & 2) == 2)
      list_factors(output, table_2LM, n);
    else
      list_factors(output, table_2plus_4k, n);
  }
}

void list_factors_minus(std::vector<factor_ct>& output, unsigned int n)
{
  if (n == 2)
    output.push_back(factor_ct(3, plain_factor, false));
  else if ((n & 1) == 1)
    list_factors(output, table_2minus, n);
  else
  {
    list_factors_minus(output, n / 2);
    list_factors_plus(output, n / 2);
  }
}

