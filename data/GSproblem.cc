#include <iostream>
#include <cstring>

int main()
{
  // Let G = Z/nZ
  for (int n = 2; n < 1200; ++n)
  {
    // Scratch array
    int S[1200];
    memset(&S, 0, sizeof(S));

    // Let S = { 2^i }
    int e = 1;		// i = 0

    while (S[e] == 0)
    {
      // Found a new element of S.
      S[e] = 1;
      e = (e * 2) % n;
    }

    // Can we pair up the elements of S to form a sum x+y=n ?
    if (S[0] || !S[n-1])
      continue;				// Nope we can't.
    bool failure = false;
    for (int i = 1; i < n; ++i)
    {
      if (!S[i])			// Find elements of S.
        continue;
      if (!S[n - i] || i == n - i)	// Check if they have a pair partner.
      {
        failure = true;
	break;
      }
    }
    if (failure)
    {
      std::cout << "We get here!\n";
      continue;
    }

    // Success!
    std::cout << "G = Z/" << n << "Z.";
#ifdef VERBOSE
    std::cout << " S = { 1";
    e = 2;
    while (e != 1)
    {
      std::cout << ", " << e;
      e = (e * 2) % n;
    }
    std::cout << " }";
#endif
    std::cout << std::endl;
  }
}

