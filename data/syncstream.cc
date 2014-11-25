#include <more/io/syncstream.h>
#include <more/math/math.h>
#include <cstring>

namespace more {
namespace io {

  syncstream::syncstream(std::streambuf* buf)
      : m_ios(buf),
	m_dig(std::numeric_limits<double>::digits) {}
  syncstream::syncstream(std::streambuf* buf, std::ios_base::openmode mode_)
      : m_ios(buf),
	m_mode(mode_),
	m_dig(std::numeric_limits<double>::digits)
  {
      prolog(m_mode);
  }

  void
  syncstream::set_digits10(int dig10)
  {
      m_dig = math::iceil(dig10*math::numbers::log2_10);
  }

  int
  syncstream::digits10()
  {
      return math::ifloor(m_dig*math::numbers::log10_2);
  }

  void
  syncstream::prolog(std::ios_base::openmode mode_)
  {
      static const int n_magic = 17;
      static char const magic[n_magic] = "more::syncstream";

      // The three number were meant to indicate format, and major and
      // minor version.  But for now, simply reserve them.

      static const int i0 = 0; // format
      static const int i1 = 0;
      m_version = 1;
      m_mode = mode_;
      if (m_mode & std::ios_base::out) {
	  m_ios.write(magic, n_magic);
	  binary_write(m_ios, i0);
	  binary_write(m_ios, i1);
	  binary_write(m_ios, m_version);
      }
      if (m_mode & std::ios_base::in) {
	  int i0p, i1p;
	  char buf[n_magic];
	  m_ios.read(buf, n_magic);
	  if (std::strncmp(buf, magic, n_magic))
	      m_ios.setstate(std::ios_base::failbit);
	  else {
	      binary_read(m_ios, i0p);
	      binary_read(m_ios, i1p);
	      binary_read(m_ios, m_version);
	  }
      }
  }

  void
  syncstream::sync_raw_data(void* p, size_t n)
  {
      if (is_output())
	  m_ios.write((char*)p, n);
      else
	  m_ios.read((char*)p, n);
  }

}
}
