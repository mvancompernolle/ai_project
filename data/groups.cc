#include "sys.h"
#include "debug.h"
#include <iostream>
#include <set>
#include <vector>
#include <cassert>
#include <libecc/fieldmath.h>

// 0 : A group
// 1 : The order
// 2 : Subgroups
// 3 : Normal Subgroup
#ifndef TEST
#define TEST 3
#endif

// 0 : Print matrices as (1 0 | 0 1)
// 1 : Print matrices LaTeX ready
#define LATEX 0

unsigned int gcd(unsigned int x, unsigned int y)
{
  if (y <= x)
  {
    if (y == 0)
    {
      assert(x);
      return x;
    }
    x = x % y;
  }
  for(;;)
  {
    if (x == 0)
      return y;
    y %= x;
    if (y == 0)
      return x;
    x %= y;
  }
}

unsigned int const cardinality_c = 96; //48;
int order_count[cardinality_c + 1];
int cyclic_count[cardinality_c + 1];

using std::cout;
using std::endl;

template<unsigned int modulo>
  struct Z {
    static int const n = modulo;
    int v;
    Z(void) { }
    Z(int _v) : v(_v) { }
    bool operator==(int r) const { return v == r; }
    bool operator==(Z const& r) const { return v == r.v; }
  };

template<unsigned int modulo>
  Z<modulo> operator+(Z<modulo> const& z1, Z<modulo> const& z2)
  {
    Z<modulo> result;
    result.v = (z1.v + z2.v) % modulo;
    return result;
  }

template<unsigned int modulo>
  Z<modulo> operator*(Z<modulo> const& z1, Z<modulo> const& z2)
  {
    Z<modulo> result;
    result.v = (z1.v * z2.v) % modulo;
    return result;
  }

template<unsigned int modulo>
  bool operator<(Z<modulo> const& z1, Z<modulo> const& z2)
  {
    return z1.v < z2.v;
  }

template<unsigned int modulo>
  std::ostream& operator<<(std::ostream& os, Z<modulo> const& z)
  {
    return os << z.v;
  }

class Matrix {
public:
  typedef Z<4> Zn;
private:
  Zn a, b;
  Zn c, d;
  int M_order;
  union inverse_union_type {
    int tmp_power;					// First a temporary 'power' is stored.
    Matrix* tmp_inverse;				// From that the order and an inverse is calculated and stored.
    char iter[sizeof(std::set<Matrix>::const_iterator)];// At the end the inverses are replaced with iterators into G.
  } M_inverse;

public:
  Matrix(void) : M_order(-1) { }
  Matrix(Zn _a, Zn _b, Zn _c, Zn _d) : a(_a), b(_b), c(_c), d(_d), M_order(-1) { }
  Matrix(Matrix const& m) : a(m.a), b(m.b), c(m.c), d(m.d), M_order(m.M_order), M_inverse(m.M_inverse) { }
  bool identity(void) const { return a == 1 && b == 0 && c == 0 && d == 1; }
  bool zero(void) const { return a == 0 && b == 0 && c == 0 && d == 0; }
  int get_order(void) const { return M_order; }
  void set_order(int order);
  void set_order(int order, std::set<Matrix>::const_iterator const& iter);
  void set_power(int n) { M_inverse.tmp_power = n; }
  int get_power(void) const { return M_inverse.tmp_power; }
  void set_inverse(std::set<Matrix> const& G);
  Matrix const& inverse(void) const;
  Matrix& operator*=(Matrix const& m2);
  friend bool operator==(Matrix const& m1, Matrix const& m2);
  friend bool operator<(Matrix const& m1, Matrix const& m2);
  friend Matrix operator*(Matrix const& m1, Matrix const& m2);
  friend std::ostream& operator<<(std::ostream& os, Matrix const& m);
  static Matrix const& unity(void);
};

void Matrix::set_inverse(std::set<Matrix> const& G)
{
  Dout(dc::groups, "Matrix::set_inverse(): Setting inverse of " << *this << " which has M_order = " << M_order);
  // Replace the pointer in the union with an iterator, initialize the iterator 'in place'.
  Matrix* tmp_inverse = M_inverse.tmp_inverse;
  new (&M_inverse.iter) std::set<Matrix>::const_iterator(G.find(*M_inverse.tmp_inverse));
  delete tmp_inverse;
  assert(*this * inverse() == Matrix::unity());
}

Matrix const& Matrix::unity(void)
{
  static Matrix S_unity(1, 0, 0, 1);
  return S_unity;
}

class Group : public std::set<Matrix> {
private:
  int M_rank;
public:
  Group(void) : M_rank(-1) { }
  Group(int rank) : M_rank(rank) { }
  bool check(void) const;
  bool is_element(Matrix const& m) const;
  int get_rank(void) const { assert(M_rank != -1); return M_rank; }
  void set_rank(int rank) { assert(M_rank == -1); M_rank = rank; }
};

// The group with all 48 elements.
Group G;

void Matrix::set_order(int order, std::set<Matrix>::const_iterator const& iter)
{
  M_order = order;
  *reinterpret_cast<std::set<Matrix>::const_iterator*>(M_inverse.iter) = iter;
}

void Matrix::set_order(int order)
{
  if (M_order > 0)
  {
    assert(order == M_order);
    return;
  }
  M_order = order;
  // Also calculate the inverse here and allocate a temporary object for it.
  mpz_class exponent(order - 1);
  // Its not a field, but when it compiles, it works, right?
  M_inverse.tmp_inverse =
      new Matrix(libecc::exponentiation<Matrix, libecc::multiplicative_square<Matrix> >(*this, exponent));
  assert(*this * *M_inverse.tmp_inverse == Matrix::unity());
}

Matrix const& Matrix::inverse(void) const
{
#ifdef CWDEBUG
  if (M_order == -1)
  {
    Dout(dc::groups, *this << " has inverse " <<
        **reinterpret_cast<std::set<Matrix>::const_iterator const*>(&M_inverse.iter) <<
	" with product " <<
	(*this * **reinterpret_cast<std::set<Matrix>::const_iterator const*>(&M_inverse.iter)) <<
	"; however, M_order is still -1!");
  }
#endif
  assert(M_order != -1);
  return **reinterpret_cast<std::set<Matrix>::const_iterator const*>(&M_inverse.iter);
}

bool operator==(Matrix const& m1, Matrix const& m2)
{
  return m1.a == m2.a && m1.b == m2.b && m1.c == m2.c && m1.d == m2.d;
}

bool operator<(Matrix const& m1, Matrix const& m2)
{
  return m1.a < m2.a || (m1.a == m2.a && (m1.b < m2.b || (m1.b == m2.b && (m1.c < m2.c || (m1.c == m2.c && m1.d < m2.d)))));
}

Matrix& Matrix::operator*=(Matrix const& m2)
{
  // We need the copies because it is possible that &m2 == this.
  Zn na = a * m2.a + b * m2.c;
  Zn nb = a * m2.b + b * m2.d;
  Zn nc = c * m2.a + d * m2.c;
  d     = c * m2.b + d * m2.d;
  c     = nc;
  b     = nb;
  a     = na;
  M_order = -1;		// Unknown again.
  return *this;
}

Matrix operator*(Matrix const& m1, Matrix const& m2)
{
  Matrix result;
  result.a = m1.a * m2.a + m1.b * m2.c;
  result.b = m1.a * m2.b + m1.b * m2.d;
  result.c = m1.c * m2.a + m1.d * m2.c;
  result.d = m1.c * m2.b + m1.d * m2.d;
  return result;
}

std::ostream& operator<<(std::ostream& os, Matrix const& m)
{
#if LATEX
  os << "\\begin{pmatrix}\n";
  os << m.a << " & " << m.b << "\\\\\n";
  os << m.c << " & " << m.d << "\n";
  os << "\\end{pmatrix},";
#else
  os << '(' << m.a << ' ' << m.b << " | " << m.c << ' ' << m.d << ')';
#endif
  return os;
}

bool Group::is_element(Matrix const& m) const
{
  return this->count(m) > 0;
}

bool Group::check(void) const
{
  for (const_iterator iter1 = begin(); iter1 != end(); ++iter1)
    for (const_iterator iter2 = begin(); iter2 != end(); ++iter2)
      if (!is_element(*iter1 * iter2->inverse()))
        return false;
  return true;
}

int main(void)
{
  Debug(libcw_do.on());
  Debug(debug::init());

  // This set is going to store all subgroups of G.
  std::set<Group> sg;

  // Generate G by running over all possible 2x2 matrices and
  // calculating all powers of them until they either become
  // 0, e or themselfs before reaching e.  Only those that
  // first reach e have an inverse.  Add those to G.

  Dout(dc::groups, "Entering main loop for generating G.");
  unsigned int cardinality = 0;
  for (int b = 0; b < Matrix::Zn::n; ++b)  
    for (int c = 0; c < Matrix::Zn::n; ++c)  
      for (int a = 0; a < Matrix::Zn::n; ++a)  
        for (int d = 0; d < Matrix::Zn::n; ++d)  
	{
	  Matrix m1(a, b, c, d);		// The matrix under test.
	  Dout(dc::groups, "Trying matrix " << m1 << '.');
	  Debug(libcw_do.inc_indent(4));
	  int order = 1;
	  Group cg(1);				// This set is temporarily used to store all cyclic subgroups.
	  Matrix m2(m1);			// m2 = m1^order
	  m2.set_power(order);
	  cg.insert(m2);			// Store m1^1 in cg.
	  Dout(dc::groups, "Added to cg: " << m2 << "; size of cg is now " << cg.size() << '.');
	  while(!m2.identity())
	  {
	    // Calculate the next power of m1 and store it in cg.
	    m2 = m1 * m2;
	    m2.set_power(++order);
	    // Keep the subgroup that exists of all powers of m1 up to date.
	    if (!cg.insert(m2).second)
	    {
	      order = -1;			// Not an element of G, goto the next matrix.
	      break;
	    }
	    Dout(dc::groups, "Added to cg: " << m2 << "; size of cg is now " << cg.size() << '.');
	  }
	  Debug(libcw_do.dec_indent(4));
	  if (order == -1)
	  {
	    Dout(dc::groups, "Skipping " << m1 << " because it has no inverse.");
	    continue;
	  }
	  Dout(dc::groups, "Found inverse for " << m1 << ".  Adding it.  Looping over all powers of it to set order.");
	  Debug(libcw_do.inc_indent(4));
	  for (Group::iterator iter = cg.begin(); iter != cg.end(); ++iter)
	  {
	    Dout(dc::groups, "Setting the order of " << *iter << " to " << order <<
	        " / gcd(" << order << ", " << iter->get_power() << ") = " <<
		(order / gcd(order, iter->get_power())) << '.');
	    const_cast<Matrix&>(*iter).set_order(order / gcd(order, iter->get_power()));
	  }
	  Debug(libcw_do.dec_indent(4));
#ifdef CWDEBUG
	  Dout(dc::groups, "The values in cg are now:");
	  Debug(libcw_do.inc_indent(4));
	  for (Group::iterator iter = cg.begin(); iter != cg.end(); ++iter)
	    Dout(dc::groups, *iter);
#endif
	  Debug(libcw_do.dec_indent(4));
	  // This was a valid element of G.  Store it.
	  std::pair<Group::iterator, bool> res = G.insert(m1);
	  assert(res.second);
	  const_cast<Matrix&>(*res.first).set_order(order, res.first);
	  ++cardinality;

	  // Maintain some administration about the number of elements with a given order.
	  order_count[order]++;
	  // Store each different cyclic subgroup in sg.
	  bool new_cyclic_group = sg.insert(cg).second;
	  // Also maintain some administration about the number of subgroups with a given cardinality.
          if (new_cyclic_group)
	    cyclic_count[cg.size()]++;

	  // Optionally print information about this new element.
#if TEST == 0
#if !LATEX
	  cout << m1 << endl;
#else
	  cout << m1 << "\t\t";
          if (cardinality % 8 == 0)
	  {
	    if (cardinality != cardinality_c)
	      cout << "\\\\\n";
	    else
	      cout << endl;
	  }
          else
	    cout << "&\n";
#endif
#endif
#if TEST == 1
	  cout << m1 << " has order " << order << endl;
#elif TEST == 2
          if (new_cyclic_group)
	  {
	    cout << "Found a new cyclic subgroup with cardinality " << cg.size() << ":\n";
	    for (Group::const_iterator iter = cg.begin(); iter != cg.end(); ++iter)
	       cout << *iter << '\n';
	  }
#endif
	}	// Next matrix.
  Dout(dc::groups, "Exiting main loop for generating G");

  // All of G has been generated now.
  // Do some sanity checks and optionally print some information about G.
  assert(G.size() == cardinality);
#if TEST == 0 && !LATEX
  cout << "\nThe cardinality is " << cardinality << endl;
#endif
  assert(cardinality == cardinality_c);

  // Find and set the iterator of G to the inverse of every element of every subgroup in G.
  Dout(dc::groups, "Looping over all cyclic subgroups and all elements to set the inverse iterator.");
  Debug(libcw_do.inc_indent(4));
  for (std::set<Group>::iterator iter1 = sg.begin(); iter1 != sg.end(); ++iter1)
  {
#ifdef CWDEBUG
    Dout(dc::groups|continued_cf, "Processing subgroup { ");
    for (Group::const_iterator iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
      Dout(dc::continued, *iter2 << " ");
    Dout(dc::finish, "}.");
#endif
    for (Group::iterator iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
      const_cast<Matrix&>(*iter2).set_inverse(G);
  }
  Debug(libcw_do.dec_indent(4));

  Dout(dc::groups, "Looping over all cyclic subgroups, checking if they are really groups.");
  Debug(libcw_do.inc_indent(4));
  for (std::set<Group>::const_iterator iter = sg.begin(); iter != sg.end(); ++iter)
  {
#ifdef CWDEBUG
    Dout(dc::groups|continued_cf, "Checking subgroup with elements: {");
    for (Group::const_iterator iter2 = iter->begin(); iter2 != iter->end(); ++iter2)
      Dout(dc::continued, *iter2 << " ");
    Dout(dc::finish, "}.");
#endif
    assert(iter->check());
  }
  Debug(libcw_do.dec_indent(4));

  // Do more sanity checks and optionally print some more information about G.
#if TEST == 1
  for (int i = 0; i < cardinality; ++i)
  {
    if (order_count[i] != 0)
    {
      if (order_count[i] > 1)
	cout << "There are " << order_count[i] << " elements with order " << i << endl;
      else
	cout << "There is 1 element with order " << i << endl;
    }
  }
#elif TEST == 2
  for (int i = 0; i < cardinality; ++i)
  {
    if (cyclic_count[i] != 0)
    {
      if (cyclic_count[i] > 1)
	cout << "There are " << cyclic_count[i] << " cyclic subgroups with cardinality " << i << endl;
      else
	cout << "There is 1 cyclic subgroup with cardinality " << i << endl;
    }
  }
#endif
  if (cyclic_count[cardinality] != 0)
    cout << "The group G itself is cyclic!" << endl;
  assert(cyclic_count[cardinality] == 0);	// We don't want that.

#if TEST >= 2
  // Find all subgroups by running over all perviously found subgroups
  // of rank 'rank - 1' and lower, and then composing a new subgroups
  // by adding all elements that one can possibly make from those twp
  // subgroups.  The rank of this new subgroup is at most the sum
  // of those previous two subgroups.  Therefore, if a newly composed
  // subgroup is new - then it has to be of rank 'rank'.
  // We stop as soon as we found all subgroups with the same rank as G.
  int max_rank = 100;				// This is set to the rank of G as soon as we know it.
  for (int rank = 2; rank <= max_rank; ++rank)
  {
    std::set<Group> new_subgroups;		// Store newly found subgroups of rank 'rank' in here.
    for (std::set<Group>::const_iterator iter1 = sg.begin(); iter1 != sg.end(); ++iter1)
    {
      Group const& group1(*iter1);
      for (std::set<Group>::const_iterator iter2 = sg.begin(); iter2 != sg.end(); ++iter2)
      {
	if (iter1 == iter2)
	  continue;
        if (iter1->get_rank() + iter2->get_rank() != rank)
	  continue;
	Group const& group2(*iter2);
#ifdef CWDEBUG
	Dout(dc::groups, "Merging two groups:");
	Dout(dc::groups|continued_cf, "group1: { ");
	for (Group::iterator iter5 = group1.begin(); iter5 != group1.end(); ++iter5)
	  Dout(dc::continued, *iter5 << " ");
	Dout(dc::finish, "}[" << group1.get_rank() << "].");
	Dout(dc::groups|continued_cf, "group2: { ");
	for (Group::iterator iter5 = group2.begin(); iter5 != group2.end(); ++iter5)
	  Dout(dc::continued, *iter5 << " ");
	Dout(dc::finish, "}[" << group2.get_rank() << "].");
#endif
	Group new_subgroup(rank);
	bool is_G = false;
	for (Group::const_iterator iter3 = group1.begin(); iter3 != group1.end() && !is_G; ++iter3)
	{
	  for (Group::const_iterator iter4 = group2.begin(); iter4 != group2.end(); ++iter4)
	  {
	    Matrix product = *iter3 * *iter4;
	    std::pair<Group::iterator, bool> res = new_subgroup.insert(product);
	    if (new_subgroup.size() > cardinality / 2)
	    {
	      is_G = true;
	      if (max_rank == 100)
	      {
	        Dout(dc::groups, "This leads to G; skipping.");
	        max_rank = rank;
		G.set_rank(rank);
	      }
	      break;
	    }
	    Group::iterator piter = G.find(product);
	    assert(piter != G.end());
	    Dout(dc::groups, "Calculated product " << *iter3 << " * " << *iter4 << " = " << *piter);
	    const_cast<Matrix&>(*res.first).set_order(piter->get_order(), piter);
#ifdef CWDEBUG
	    Dout(dc::groups|continued_cf, "new_subgroup now contains " << new_subgroup.size() << " elements: { ");
	    for (Group::iterator iter5 = new_subgroup.begin(); iter5 != new_subgroup.end(); ++iter5)
	      Dout(dc::continued, *iter5 << " ");
	    Dout(dc::finish, "}.");
#endif
	  }
	}
	if (is_G)
	  continue;
	do
	{
	  // Add all a/b of all elements a and b, until this is a group.
	  Group new_subgroup_copy(new_subgroup);
	  for (Group::const_iterator iter1 = new_subgroup_copy.begin(); iter1 != new_subgroup_copy.end(); ++iter1)
	    for (Group::const_iterator iter2 = new_subgroup_copy.begin(); iter2 != new_subgroup_copy.end(); ++iter2)
	      new_subgroup.insert(*G.find(*iter1 * iter2->inverse()));
	}
	while (!new_subgroup.check());
	if (new_subgroup.size() == cardinality || sg.find(new_subgroup) != sg.end())
	  continue;
	if (new_subgroups.insert(new_subgroup).second)
	{
#if TEST == 2
	  cout << "Found new subgroup, rank " << new_subgroup.get_rank() <<
	      ", with cardinality " << new_subgroup.size() << ":\n";
	  for (Group::const_iterator iter = new_subgroup.begin(); iter != new_subgroup.end(); ++iter)
	     cout << *iter << '\n';
#endif
	}
      }
    }
    if (new_subgroups.empty())
      break;
    sg.insert(new_subgroups.begin(), new_subgroups.end());
  }
  cout << "The rank of G is " << G.get_rank() << '.' << endl;
#if TEST == 2
  for (int rank = 1; rank <= G.get_rank(); ++rank)
  {
    int order_count[cardinality + 1];
    std::memset(order_count, 0, (cardinality + 1) * sizeof(int));
    int count = 0;
    for (std::set<Group>::iterator iter = sg.begin(); iter != sg.end(); ++iter)
      if (iter->get_rank() == rank)
      {
        ++count;
	order_count[iter->size()]++;
      }
    cout << "There are " << count << " subgroups with rank " << rank << " in G." << endl;
    for (int n = 1; n <= cardinality; ++n)
    {
      if (order_count[n] > 0)
        cout << order_count[n] << " of those have " << n << " elements." << endl;
    }
  }
#endif
#endif
#if TEST == 3
  // Run over all subgroups.
  for (std::set<Group>::iterator iter1 = sg.begin(); iter1 != sg.end(); ++iter1)
  {
    bool is_normal_subgroup = true;
    // Run over each element.
    for (Group::iterator iter2 = iter1->begin(); is_normal_subgroup && iter2 != iter1->end(); ++iter2)
    {
      // Run over every element in G.
      for (Group::iterator iter3 = G.begin(); iter3 != G.end(); ++iter3)
      {
        Matrix tmp = *iter3 * *iter2;
	tmp *= iter3->inverse();
	if (!iter1->is_element(tmp))
	{
	  is_normal_subgroup = false;
	  break;
	}
      }
    }
    if (is_normal_subgroup)
    {
      cout << "Found new normal subgroup: { ";
#if LATEX
      int count = 0;
      cout << '\n';
#endif
      for (Group::iterator iter2 = iter1->begin(); is_normal_subgroup && iter2 != iter1->end(); ++iter2)
      {
#if !LATEX
        cout << *iter2 << " ";
#else
        ++count;
	cout << *iter2 << "\t\t";
	if (count % 8 == 0)
	{
	  if (count != iter1->size())
	    cout << "\\\\\n";
	  else
	    cout << endl;
	}
	else
	  cout << "&\n";
#endif
      }
      cout << "}" << endl;
    }
  }
#endif
}
