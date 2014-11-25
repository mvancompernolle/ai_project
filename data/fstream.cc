//  Copyright (C) 2001--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <more/io/fstream.h>
#include <more/io/ipstream.h>
#include <more/io/filesys.h>
#include <fstream>
#include <cwchar>
#include <more/bits/conf.h>
#if defined(MORE_CONF_HAVE_BZLIB_H)
#  include <bzlib.h>
#  define MORE_MORE_CONF_HAVE_BZLIB
#endif
#if defined(MORE_CONF_HAVE_ZLIB_H)
#  include <zlib.h>
#  define MORE_MORE_CONF_HAVE_ZLIB
#endif


namespace more {
namespace io {

#if defined(MORE_MORE_CONF_HAVE_BZLIB) || defined(MORE_MORE_CONF_HAVE_ZLIB)
  template<typename Char, typename Traits>
    struct basic_callbackbuf : std::basic_streambuf<Char, Traits>
    {
	typedef Char char_type;
	typedef Traits traits_type;
	typedef typename Traits::int_type int_type;
	typedef typename Traits::pos_type pos_type;
	typedef typename Traits::off_type off_type;
	typedef typename std::size_t size_type;

	basic_callbackbuf()
	{
	    this->setg(0, 0, 0);
	    this->setp(0, 0);
	}

	virtual int_type overflow(int_type = traits_type::eof());
	virtual int_type underflow();
	virtual ~basic_callbackbuf();
	virtual size_type read(void* ptr, size_type size) = 0;
	virtual size_type write(void* ptr, size_type size) = 0;
// 	virtual int sync() = 0;
      private:
	static const int s_size_put = 512;
	static const int s_size_get = 512;
    };

  template<typename Char, typename Traits>
    basic_callbackbuf<Char, Traits>::~basic_callbackbuf()
    {
	sync();
	delete this->pbase();
	delete this->eback();
    }

  template<typename CharT, typename Traits>
    typename basic_callbackbuf<CharT, Traits>::int_type
    basic_callbackbuf<CharT, Traits>::
    overflow(int_type c)
    {
	/* test for the empty sequence */
	if (Traits::eq_int_type(c, Traits::eof()) &&
	    (!this->pptr() || (this->pptr() == this->pbase())))
	    return Traits::not_eof(c);

	/* if no buffer, make one, otherwise empty it */
	if (!this->pptr()) {
	    CharT* buf = new CharT[s_size_put];
	    this->setp(buf, buf + s_size_put);
	}
	else if (this->pbase() != this->pptr()) {
	    int n_wish = this->pptr() - this->pbase();
	    int n = write(this->pbase(), n_wish*sizeof(CharT));
	    if (n != n_wish)
		return Traits::eof();
	    this->setp(this->pbase(), this->epptr());
	}

	if (Traits::eq_int_type(c, Traits::eof()))
	    return Traits::not_eof(c);
	else {
	    *this->pbase() = c;
	    this->pbump(1);
	    return c;
	}
    }

  template<typename CharT, typename Traits>
    typename basic_callbackbuf<CharT, Traits>::int_type
    basic_callbackbuf<CharT, Traits>::
    underflow()
    {
	if (!this->eback()) {
	    CharT* p = new CharT[s_size_get];
	    this->setg(p, p, p);
	}
	int nread = read(this->eback(), s_size_get*sizeof(CharT));
	if (nread <= 0)
	    return Traits::eof();
	else {
	    this->setg(this->eback(), this->eback(), this->eback() + nread);
	    return *this->eback();
	}
    }
#endif

#ifdef MORE_MORE_CONF_HAVE_BZLIB
  template<typename Char, typename Traits>
    struct basic_bzfilebuf : basic_callbackbuf<Char, Traits>
    {
    private:
	typedef basic_callbackbuf<Char, Traits> base;
    public:
	typedef typename base::size_type size_type;

	basic_bzfilebuf(BZFILE* file, FILE* cfile)
	    : m_bzfile(file),
	      m_cfile_to_close(cfile) {}

	virtual ~basic_bzfilebuf()
	{
	    if (m_cfile_to_close)
		fclose(m_cfile_to_close);
	    int err;
	    BZ2_bzReadClose(&err, m_bzfile);
	    if (err != BZ_OK)
		BZ2_bzWriteClose(&err, m_bzfile, 0, 0, 0);
	}

	virtual size_type read(void* buf, size_type size)
	{
#if 0 // not official libbzip2
	    return BZ2_bzread(m_bzfile, buf, size);
#else
	    int err;
	    return BZ2_bzRead(&err, m_bzfile, buf, size);
#endif
	}

	virtual size_type write(void* buf, size_type size)
	{
#if 0 // not official libbzip2
	    return BZ2_bzwrite(m_bzfile, buf, size);
#else
	    int err;
	    BZ2_bzWrite(&err, m_bzfile, buf, size);
	    if (err == BZ_OK)
		return size;
	    else
		return 0;
#endif
	}

	virtual int sync() { this->overflow(); return 0; }

    private:
	BZFILE* m_bzfile;
	FILE* m_cfile_to_close;
    };
#endif

#ifdef MORE_MORE_CONF_HAVE_ZLIB
  template<typename Char, typename Traits>
    struct basic_gzfilebuf : basic_callbackbuf<Char, Traits>
    {
    private:
	typedef basic_callbackbuf<Char, Traits> base;
    public:
	typedef typename base::size_type size_type;

	basic_gzfilebuf(gzFile file) : m_file(file) {}
	~basic_gzfilebuf() { gzclose(m_file); }

	virtual size_type read(void* buf, size_type size)
	{
	    return gzread(m_file, buf, size);
	}

	virtual size_type write(void* buf, size_type size)
	{
	    return gzwrite(m_file, buf, size);
	}

	virtual int sync() { this->overflow(); gzflush(m_file, 1); return 0; }

    private:
	gzFile m_file;
    };
#endif


  template<typename Char, typename Traits>
    typename basic_ifstream<Char, Traits>::streambuf_type*
    basic_ifstream<Char, Traits>::
    open(char const* c_name, std::ios_base::openmode mode = std::ios_base::in)
    {
	typedef basic_ipstreambuf<Char, Traits> ipstreambuf_type;
	typedef std::basic_filebuf<Char, Traits> filebuf_type;
	close();

	enum { plain, gz, bz2 } format = plain;
	std::string name = c_name;

	int n = name.length();
	if (n >= 3 && name.substr(n - 3) == ".gz")
	    format = gz;
	else if (n >= 4 && name.substr(n - 4) == ".bz2")
	    format = bz2;
	else if (!file_status(name).exists()) {
	    if (file_status(name + ".gz").exists()) {
		name += ".gz";
		format = gz;
	    }
	    else if (file_status(name + ".bz2").exists()) {
		name += ".bz2";
		format = bz2;
	    }
	    else
		return 0;
	}

	switch (format) {
	case plain:
	    m_buf = new filebuf_type();
	    static_cast<filebuf_type*>(m_buf)->open(name.c_str(), mode);
	    break;
	case gz:
#ifdef MORE_MORE_CONF_HAVE_ZLIB
	    {
		gzFile file = gzopen(name.c_str(), "r");
		if (!file)
		    return 0;
		m_buf = new basic_gzfilebuf<Char, Traits>(file);
	    }
#else
	    m_buf = new ipstreambuf_type(("gunzip -c "+name).c_str());
#endif
	    break;
	case bz2:
#ifdef MORE_MORE_CONF_HAVE_BZLIB
	    {
#if 0		// not official libbzip2
		BZFILE* file = BZ2_bzopen(name.c_str(), "r");
		if (!file)
		    return 0;
		m_buf = new basic_bzfilebuf<Char, Traits>(file, cfile);
#else
		FILE* cfile = fopen(name.c_str(), "r");
		if (cfile == 0)
		    return 0;
		int err;
		BZFILE* file = BZ2_bzReadOpen(&err, cfile,
					      /* small = */0,
					      /* verbosity = */1, 0, 0);
		if (!file)
		    return 0;
		m_buf = new basic_bzfilebuf<Char, Traits>(file, cfile);
#endif
	    }
#else
	    m_buf = new ipstreambuf_type(("bunzip2 -c "+name).c_str());
#endif
	    break;
	}
	init(m_buf);
	return m_buf;
    }

  template class basic_ifstream<char, std::char_traits<char> >;
  template class basic_ifstream<wchar_t, std::char_traits<wchar_t> >;

}
}
