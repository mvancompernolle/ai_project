#include <iostream>
#include <string>

extern void xmem_main_return();

using namespace std;

void subfunction()
{
  //cout << "\nenter subfunction()";
  std::string s = "HELLO ";
  //cout << "\nleave subfunction()";
}

int main(int argc, char** argv)
{
  cout << "\n>>>> calling subfunction() <<<<";
  subfunction();
  cout << "\n>>>> return from subfunction() <<<<";
  xmem_main_return();
  return 0;
}
