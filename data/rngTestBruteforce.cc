// This program has been used to test the results of the Matrix program.

#include <inttypes.h>
#include <iostream>

unsigned int const number_of_bits = 17;
unsigned int const number_of_feedbackpoints = 2;
unsigned int const feedbackpoints[number_of_feedbackpoints] = { 4, 13 };

uint32_t sr[number_of_bits * 2];

int main(void)
{
  sr[0] = sr[number_of_bits] = 1;
  uint32_t* head = sr;

  int cnt = 0;
  for(;;)
  {
    uint32_t a = head[0];
    for(int fp = 0; fp < number_of_feedbackpoints; ++fp)
      a ^= head[number_of_bits - feedbackpoints[fp]];
    head[0] = head[number_of_bits] = a;
    if (++head == &sr[number_of_bits])
      head = sr;
    ++cnt;
    if (head[0] == 1)
    {
      int i;
      for (i = 1; i < number_of_bits; ++i)
	if (head[i])
	  break;
      if (i == number_of_bits)
	break;
    }
  }
  std::cout << "Period of " << number_of_bits;
  for (int i = 0; i < number_of_feedbackpoints; ++i)
    std::cout << '/' << feedbackpoints[i];
  std::cout << " is " << cnt << std::endl;
  return 0;
}
