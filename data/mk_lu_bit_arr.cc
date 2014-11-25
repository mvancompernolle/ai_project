#include <more/math/math.h>
#include <iostream>

int
main()
{
    std::cout << "namespace more {\nnamespace math{\n"
	      << "int lower_bit_arr[] = {\n    0";
    for (int i = 1; i < 0x10000; ++i) {
	if (i % 16 == 0)
	    std::cout << ",\n    ";
	else
	    std::cout << ", ";
	std::cout << more::math::lower_bit(i);
    }
    std::cout << "\n};\n\nint upper_bit_arr[] = {\n    0";
    for (int i = 1; i < 0x10000; ++i) {
	if (i % 16 == 0)
	    std::cout << ",\n    ";
	else
	    std::cout << ", ";
	std::cout << more::math::upper_bit(i);
    }
    std::cout << "\n};\n}}\n";
}
