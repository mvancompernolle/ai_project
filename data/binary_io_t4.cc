#include <string>
#include <fstream>
#include <iomanip>
#include <more/io/binary_io.h>
#include <cmath>
#include <cstdlib>
#include <more/io/cmdline.h>

template<typename T>
  T mydist() {
      return ((T)drand48()-(T).5)*
	std::exp((T)20.0*(drand48()-.5));
//	std::exp((T)(drand48()-.5)*std::numeric_limits<T>::max_exponent);
  }

bool verb = false;

template<typename T>
  bool test_fp(int cnt) {
      bool status = true;
      long seed = lrand48();
      srand48(seed);
      {
	  std::ofstream os("tmp.binary_io_t4.bin");
	  if (verb) std::cout << "Values: ";
	  for (int i = 0; i < cnt; ++i) {
	      T x = mydist<T>();
	      more::io::binary_write(os, x);
	      if (verb) std::cout << x << ' ';
	  }
	  if (verb) std::cout << std::endl;
	  if (std::numeric_limits<T>::has_infinity) {
	      more::io::binary_write(os, std::numeric_limits<T>::infinity());
	      more::io::binary_write(os, -std::numeric_limits<T>::infinity());
	  }
	  if (std::numeric_limits<T>::has_quiet_NaN)
	      more::io::binary_write(os, std::numeric_limits<T>::quiet_NaN());
	  if (os.fail())
	      throw std::runtime_error("writing failed");
      }
      if (verb) {
	  std::ifstream is("tmp.binary_io_t4.bin");
	  std::cout << "Encoded as: ";
	  while (is.good())
	      std::cout << std::hex << std::setw(3)
			<< (unsigned int)(unsigned char)is.get();
	  std::cout << std::endl;
      }
      srand48(seed);
      {
	  std::ifstream is("tmp.binary_io_t4.bin");
	  for (int i = 0; i < cnt; ++i) {
	      T x;
	      more::io::binary_read(is, x);
	      T y = mydist<T>();
	      if (std::abs(x-y)/std::abs(x+y) >
		  std::numeric_limits<T>::epsilon()) {
		  std::cerr << "Mismatched value "
			    << x << " != " << y << std::endl;
		  status = false;
	      }
	      if (is.fail())
		  throw std::runtime_error("reading failed");
	  }
	  T x;
	  if (std::numeric_limits<T>::has_infinity) {
	      more::io::binary_read(is, x);
	      if (isinf(x) != 1) {
		  std::cerr << "Error decoding inf != " << x << std::endl;
		  status = false;
	      }
	      more::io::binary_read(is, x);
	      if (isinf(x) != -1) {
		  std::cerr << "Error decoding -inf != " << x << std::endl;
		  status = false;
	      }
	  }
	  if (std::numeric_limits<T>::has_quiet_NaN) {
	      more::io::binary_read(is, x);
	      if (!isnan(x)) {
		  std::cerr << "Error decoding NaN != " << x << std::endl;
		  status = false;
	      }
	  }
	  if (is.fail())
	      throw std::runtime_error("reading failed");
      }
      return status;
  }

int main(int argc, char** argv) try {
    more::io::cmdline cmd;
    int n = 100;
    cmd.insert_setter("-v", "Verbose output", verb, true);
    cmd.insert_reference("", "n#Number of floats to try", n);
    cmd.parse(argc, argv);
    bool status = true;
    std::cout << "Testing float" << std::endl;
    status &= test_fp<float>(n);
    std::cout << "\nTesting double" << std::endl;
    status &= test_fp<double>(n);
//  Long double may fail due to inprecise library functions
//     std::cout << "\nTesting long double" << std::endl;
//     status &= test_fp<long double>(n);
    if (status)
	return 0;
    else
	throw std::runtime_error("something failed");
}
catch (more::io::cmdline::relax) {
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}
