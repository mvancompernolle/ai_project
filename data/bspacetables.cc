#include <iostream>

unsigned long cardinality[] = {
  8, 4, 16, 44, 56, 116, 288, 508, 968, 2116, 4144, 8012, 16472,
  33044, 65088, 130972, 263144, 523492, 1047376, 2099948, 4193912,
  8383412, 16783200, 33558844, 67092488, 134225284, 268460656,
  536830604, 1073731736, 2147574356
};

bool is_prime(int p)
{
  for (int d = 2; d < p; ++d)
    if (p % d == 0)
      return false;
  return true;
}

int main()
{
  std::cout << "<table>\n<tr><th>m</th><th>#<b></b>E</th></tr>\n";
  for (int i = 0; i < sizeof(cardinality)/sizeof(unsigned long); ++i)
  {
    int m = i + 2;
    std::cout << "<tr><td>" << m << "</td><td>" << cardinality[i] << "</td></tr>\n";
  }
  std::cout << "</table>\n" << std::endl;
  std::cout << "<table>\n<tr><th>m</th><th>(#<b></b>E - 2)/2</th></tr>\n";
  for (int i = 0; i < sizeof(cardinality)/sizeof(unsigned long); ++i)
  {
    int m = i + 2;
    std::cout << "<tr><td>" << m << "</td><td>" << (cardinality[i] - 2)/2 << "</td></tr>\n";
  }
  std::cout << "</table>\n" << std::endl;
  std::cout << "<table>\n<tr><th>m</th><th>(#<b></b>E - 2)/2 - 1</th><th>((#<b></b>E - 2)/2 - 1)/m</th></tr>\n";
  for (int i = 0; i < sizeof(cardinality)/sizeof(unsigned long); ++i)
  {
    int m = i + 2;
    std::cout << "<tr><td>" << m << "</td><td>" << (cardinality[i] - 2)/2 - 1 << "</td><td>";
    if (is_prime(m))
      std::cout << ((cardinality[i] - 2)/2 - 1) / m;
    std::cout << "</td></tr>\n";
  }
  std::cout << "</table>\n" << std::endl;
}
