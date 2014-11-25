#include <iostream>
#include <cassert>
#include <vector>
#include <utility>
#include <string>
#include <sstream>

static int pwr(int base, int exponent)
{
  int result = 1;
  while (exponent-- > 0)
    result *= base;
  return result;
}

// decode_group_structure
//
// Returns a string that is unique down to isomorphism.
//
// pt: An array with all primes that might be factors of the given orders.
// np: The size of the pt array.
// v : A list of (order, number) pairs, where 'order' is the order of a point
//     and 'number' is the total number of points with that order.
//
// The list must be complete, and all prime factors must be given
// or else the result will be undefined.
// This means for example that the sum of all 'number' values in the given
// pairs must be equal to the cardinality of the group and that each of the
// prime factors of that cardinality must be listed in pt.
//
// Warning: This code contains deep magic*).  This function does the inverse
// of what is described in reference-manual/group__theory__cyclic.html
// and its algorithm went over my capabilities to document.
//
// *) That means that you will never be able to understand why it works
//    (and I only hardly understood, once).
//
std::string decode_group_structure(int const* pt, int np, std::vector<std::pair<int, int> > const& v)
{
  std::ostringstream out;

#if 0
  std::cout << "Input (order, #): ";
  for (std::vector<std::pair<int, int> >::const_iterator iter = v.begin(); iter != v.end(); ++iter)
  {
    if (iter != v.begin())
      std::cout << ", ";
    std::cout << "(" << iter->first << ", " << iter->second << ")";
  }
  std::cout << std::endl;
#endif

  bool first_time = true;
  int large_cycle = 1;		// Accumulate as much as possible in one cyclic group, with this as cardinality.

  // Run over all (possible) primes.
  for (int pi = 0; pi < np; ++pi)
  {
    int p = pt[pi];

#if 0
    std::cout << "Trying prime " << p << std::endl;
#endif

    // Determine the sum of the number of points that have
    // an order not containing a factor p^x for any 0 <= x <= k
    // separately (thus, add the same number multiple times
    // when appropriate).  Write the results into u[k].
    std::vector<int> u;
    bool found = true;
    for (unsigned int pp = p, k = 0; found; pp *= p, ++k)
    {
      found = false;
      for (std::vector<std::pair<int, int> >::const_iterator iter = v.begin(); iter != v.end(); ++iter)
      {
	if (iter->first % pp == 0)
	  found = true;
	else
	{
	  assert(k <= u.size());
	  if (k == u.size())
	    u.push_back(0);
	  u[k] += iter->second;
        }
      }
    }
    int ma = u.size() - 1;

    // This prime is not a factor (everything went into u[0]).
    if (ma == 0)
      continue;

    // Devide out residu of other prime factors
    for (int i = ma; i >= 0; --i)
      u[i] /= u[0];

    // Now u[] contains powers of p.

    // Determine those powers, by taking the p-log of this.
    std::vector<int> t;
    for (int i = 0; i <= ma; ++i)
    {
      int ui = u[i];
      int ti = 0;
      while(ui != 1)
      {
        assert(ui % p == 0);
        ui /= p;
        ++ti;
      }
      t.push_back(ti); 
    }

#if 0
    std::cout << "t = ";
    for (int i = 0; i < ma; ++i)
      std::cout << t[i] << ", ";
    std::cout << t[ma] << std::endl;
#endif

    // Determine increment in the powers as function of k.
    std::vector<int> in;
    for (int k = 1; k <= ma; ++k)
      in.push_back(t[k] - t[k-1]);
    in.push_back(0);	// No more increments behond this point.

    // Determine a and m from that.
    std::vector<int> a;
    std::vector<int> m;
    int lf = in[0];
    int ai = 0;
    for (int i = 1; i <= ma; ++i)
    {
      ++ai;
      if (in[i] != lf)
      {
	a.push_back(ai);
	m.push_back(lf - in[i]);
	lf = in[i];
      }
    }

    // Print results.
    int n = a.size();
#if 0
    for (int i = 0; i < n; ++i)
      std::cout << "a[" << i << "] = " << a[i] << "; m[" << i << "] = " << m[i] << std::endl;
#endif

    for (int i = 0; i < n; ++i)
    {
      if (m[i] > 1)
      {
	if (!first_time)
	  out << " X ";
	else
	  first_time = false;
        if (m[i] > 2)
	  out << "(C_" << pwr(p, a[i]) << ")^" << (m[i] - 1);
        else
	  out << "C_" << pwr(p, a[i]);
        large_cycle *= pwr(p, a[i]);
      }
      else if (i != n - 1)
      {
	if (!first_time)
	  out << " X ";
	else
	  first_time = false;
	out << "C_" << pwr(p, a[i]);
      }
      else
        large_cycle *= pwr(p, a[i]);
    }
  }
  if (large_cycle > 1)
  {
    if (!first_time)
      out << " X ";
    out << "C_" << large_cycle;
  }

  return out.str();
}

