#include <more/diag/errno.h>
#include <more/io/buffer.h>
#include <iostream>
#include <cstdio>

namespace io = more::io;
namespace diag = more::diag;

int _errcnt = 0;
#define ASSERT(x) \
    ((x)? (void)0 \
        : (void)(std::cout << "** assertion failed: "#x << std::endl, \
		 ++_errcnt))

template<typename Char>
void exercise(int N, bool want_eof = false)
{
    std::string str;
    std::cout << "Writing " << N << " chars." << std::endl;
    {
	io::buffer<Char>
	    obuf_file(io::new_ofdriver<Char>("tmp.buffer_t0.out"));
	io::buffer<char>
	    obuf_str(io::new_osdriver<char>(str));
	if (obuf_file.is_error())
	    throw diag::errno_exception();
	for (int i = 0; i < N; ++i) {
	    obuf_file.get() = (Char)'a' + i % 26;
	    obuf_str.get() = 'a' + i % 26;
	}
    }
    std::cout << "Reading " << N << " chars." << std::endl;
    {
	io::buffer<Char>
	    ibuf_file(io::new_ifdriver<Char>
		      ("tmp.buffer_t0.out", want_eof));
	io::buffer<char>
	    ibuf_str(io::new_isdriver<char>(str, want_eof));
	if (ibuf_file.is_error()) {
	    std::cout << "Could not open input file." << std::endl;
	    abort();
	}
	for (int i = 0; i < N; ++i) {
	    ASSERT(!ibuf_file.is_eof());
	    ASSERT(!ibuf_str.is_eof());
	    ASSERT(ibuf_file.get() == (Char)'a' + i % 26);
	    ASSERT(ibuf_str.get() == 'a' + i % 26);
	}
	if (want_eof) {
// 	    ASSERT(ibuf_file.get() == std::char_traits<Char>::eof());
// 	    ASSERT(std::char_traits<Char>::to_int_type(ibuf_file.get())
// 		   == std::char_traits<Char>::eof());
	    std::cout << std::char_traits<Char>::to_int_type(ibuf_file.get())
		      << " == " << std::char_traits<Char>::eof()
		      << "?" << std::endl;
	    ASSERT(ibuf_str.get() == EOF);
	}
	ASSERT(ibuf_file.is_eof());
	ASSERT(ibuf_str.is_eof());
	if (ibuf_file.is_error())
	    throw diag::errno_exception();
    }
}

int main()
try {
    exercise<char>(0);
    exercise<char>(0, true);
    exercise<char>(10);
    exercise<char>(511);
    exercise<char>(511, true);
    exercise<char>(512);
    exercise<char>(513);
    exercise<char>(800);
    exercise<wchar_t>(0);
    exercise<wchar_t>(10);
    exercise<wchar_t>(10, true);
    exercise<wchar_t>(511);
    exercise<wchar_t>(512);
    exercise<wchar_t>(512, true);
    exercise<wchar_t>(513);
    return _errcnt? 1 : 0;
}
catch (std::exception& xc) {
    std::cout << "exception: " << xc.what() << std::endl;
    return 1;
}
