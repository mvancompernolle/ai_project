#include <more/io/buffer.h>
#include <more/diag/errno.h>
#include <more/bits/conf.h>

#if defined(MORE_CONF_HAVE_SYS_TYPES_H) && defined(MORE_CONF_HAVE_SYS_STAT_H) \
    && defined(MORE_CONF_HAVE_FCNTL_H) && defined(MORE_CONF_HAVE_UNISTD_H) \
    && defined(MORE_CONF_HAVE_OPEN) && defined(MORE_CONF_HAVE_CLOSE) \
    && defined(MORE_CONF_HAVE_READ) && defined(MORE_CONF_HAVE_WRITE)
#  define MORE_CONF_HAVE_UNIX_IO
#endif

#ifdef MORE_CONF_HAVE_UNIX_IO
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#endif
#include <stdio.h>
#include <wchar.h>

#ifdef MORE_MORE_CONF_HAVE_READLINE_BUFFER
#  include <readline.h>
#  include <history.h>
#endif

#include <cstring>

namespace more {
namespace io {

  //
  //  String buffers
  //

  template<typename Char, typename Traits>
    struct input_string_driver : buffer<Char>::driver {
	~input_string_driver() {
	    delete[] m_buf;
	}
	explicit
	input_string_driver(std::basic_string<Char, Traits> const& str,
			    bool want_eof) {
	    size_t n = str.size() + (want_eof? 1 : 0);
	    m_buf = new Char[n];
	    std::copy(str.begin(), str.end(), m_buf);
	    if (want_eof) m_buf[str.size()] = Traits::eof();
#if 0 // XXX to simplify
	    set_fixed(m_buf, my_buf + n);
#else
	    m_cur = m_buf;
	    m_end = m_buf + n;
#endif
	}
	Char* read(Char* first, Char* last) {
#if 0 // XXX to simplify
	    return 0;
#else
//	    difference n = last - first;
	    std::ptrdiff_t n_dest = last - first;
	    std::ptrdiff_t n_src = m_end - m_cur;
	    if (n_dest < n_src) {
		std::copy(m_cur, m_cur + n_dest, first);
		m_cur += n_dest;
		return last;
	    }
	    else if (n_src == 0)
		return 0;
	    else {
		std::copy(m_cur, m_end, first);
		m_cur = m_end;
		return first + n_src;
	    }
#endif
	}
      private:
	Char* m_buf;
#if 1 // XXX to simplify
	Char* m_cur;
	Char* m_end;
#endif
    };

  template<typename Char, typename Traits>
    typename buffer<Char>::driver*
    new_isdriver(std::basic_string<Char, Traits> const& str, bool want_eof) {
	return new input_string_driver<Char, Traits>(str, want_eof);
    }

  template
    buffer<char>::driver*
    new_isdriver<char>(
	std::basic_string< char, std::char_traits<char> > const&, bool);
  template
    buffer<wchar_t>::driver*
    new_isdriver<wchar_t>(
	std::basic_string< wchar_t, std::char_traits<wchar_t> > const&, bool);

  template<typename Char, typename Traits>
    struct output_string_driver : buffer<Char>::driver::driver {
	explicit output_string_driver(std::basic_string<Char, Traits>& str)
	    : m_str(str) {}
	~output_string_driver() {}
	void write(Char* first, Char* last) {
	    m_str.append(first, last);
	}
      private:
	std::basic_string<Char, Traits>& m_str;
    };

  template<typename Char, typename Traits>
    typename buffer<Char>::driver*
    new_osdriver(std::basic_string<Char, Traits>& str) {
	return new output_string_driver<Char, Traits>(str);
    }

  template
    buffer<char>::driver*
    new_osdriver(std::basic_string< char, std::char_traits<char> >&);

  template
    buffer<wchar_t>::driver*
    new_osdriver(std::basic_string< wchar_t, std::char_traits<wchar_t> >&);


  //
  //  File buffers
  //

  template<typename Char>
    struct input_file_driver {};
  template<typename Char>
    struct output_file_driver {};

#ifdef MORE_CONF_HAVE_UNIX_IO

  // UNIX IO.  More efficient if available, since we avoid redundant
  // buffering.

  template<>
    struct input_file_driver<char> : buffer<char>::driver {

	input_file_driver(int fd_, bool want_eof)
	    : fd(fd_), m_want_eof(want_eof) {}
	virtual ~input_file_driver() {
	    if (fd != -1 && fd != STDIN_FILENO)
		close(fd);
	}

      protected:
	virtual char*
	read(char* first, char* last)
	{
	    if (fd == -1)
		throw std::logic_error("more::buffer::read: File error.");
	    ssize_t st = ::read(fd, first, (last - first)*sizeof(char));
	    if (st == -1 && fd != STDIN_FILENO) {
		close(fd);
		fd = -1;
		throw diag::errno_exception();
	    }
	    else {
		if (st == 0 && !m_want_eof)
		    return 0;
		first += st/sizeof(char);
		if (first != last && m_want_eof) {
		    *first++ = EOF;
		    m_want_eof = false;
		}
		return first;
	    }
	}
	virtual bool is_error() { return fd == -1; }

      private:
	int fd;
	bool m_want_eof;
    };

  template<>
    buffer<char>::driver*
    new_ifdriver<char>(char const* name, bool want_eof) {
	if (name == 0)
	    return new input_file_driver<char>(STDIN_FILENO, want_eof);
	else if (int fd = open(name, O_RDONLY))
	    return new input_file_driver<char>(fd, want_eof);
	else
	    return 0;
    }

  template<>
    struct output_file_driver<char> : buffer<char>::driver {

	explicit output_file_driver(int fd_) : fd(fd_) {}
	virtual ~output_file_driver() {
	    if (fd != -1 && fd != STDOUT_FILENO)
		close(fd);
	}

      protected:
	virtual void write(char* first, char* last) {
	    if (fd == -1)
		throw std::logic_error("more::buffer::read: File error.");
	    ssize_t st = ::write(fd, first, (last - first)*sizeof(char));
	    if (st == -1 && fd != STDOUT_FILENO) {
		close(fd);
		fd = -1;
		throw diag::errno_exception();
	    }
	}
	virtual bool is_error() { return fd == -1; }

      private:
	int fd;
    };

  template<>
    buffer<char>::driver*
    new_ofdriver<char>(char const* name) {
	if (name == 0)
	    return new output_file_driver<char>(STDOUT_FILENO);
	if (int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644))
	    return new output_file_driver<char>(fd);
	else
	    return 0;
    }

#else

  // ANSI C based IO

  template<>
    struct input_file_driver<char> : buffer<char>::driver {

	input_file_driver(FILE* fp_, bool want_eof)
	    : fp(fp_), m_want_eof(want_eof) {}

	virtual ~input_file_driver() {
	    if (fp && fp != stdin)
		fclose(fp);
	}

      protected:
	virtual char* read(char* first, char* last) {
	    size_type n = fread(first, sizeof(char), last - first, fp);
	    if (n == 0 && !m_want_eof) return 0;
	    first += n;
	    if (first != last && m_want_eof) {
		*first++ = EOF;
		m_want_eof = false;
	    }
	    return first;
	}
	virtual bool is_error() { return ferror(fp); }

      private:
	FILE* fp;
	bool m_want_eof;
    };

  template<>
    buffer<char>::driver*
    new_ifdriver<char>(char const* name, bool want_eof) {
	if (name == 0)
	    return new input_file_dirver<char>(stdin, want_eof);
	else if (FILE* fp = fopen(name, "r"))
	    return new input_file_driver<char>(fp, want_eof);
	else
	    return 0;
    }

  template<>
    struct output_file_driver<char> : buffer<char>::driver {

	output_file_driver(FILE* fp_) : fp(fp_) {}

	virtual ~output_file_driver() {
	    if (fp && fp != stdout)
		fclose(fp);
	}

      protected:
	virtual void write(char* first, char* last) {
	    fwrite(first, sizeof(char), last - first, fp);
	}
	virtual bool is_error() { return ferror(fp); }

      private:
	FILE* fp;
    };

  template<>
    buffer<char>::driver*
    new_ofdriver<char>(char const* name) {
	if (name == 0)
	    return new output_file_driver<char>(stdout);
	else if (FILE* fp = fopen(name, "w"))
	    return new output_file_driver<char>(fp);
	else
	    return 0;
    }

#endif


  template<>
    struct input_file_driver<wchar_t> : buffer<wchar_t>::driver {

	input_file_driver(FILE* fp_, bool want_eof)
	    : fp(fp_), m_want_eof(want_eof) {}

	virtual ~input_file_driver() {
	    if (fp && fp != stdin)
		fclose(fp);
	}

      protected:
	virtual wchar_t* read(wchar_t* first, wchar_t* last) {
	    // XXX Buffering.
	    if (!m_want_eof && first != last &&
		std::char_traits<wchar_t>::to_int_type(*first++ = fgetwc(fp))
		== std::char_traits<wchar_t>::eof())
		return 0;
	    while (first != last) {
		if ((wint_t)(*first = fgetwc(fp)) == WEOF) {
		    if (m_want_eof) {
			m_want_eof = false;
			++first;
		    }
		    break;
		}
		++first;
	    }
	    return first;
	}
	virtual bool is_error() { return ferror(fp); }

      private:
	FILE* fp;
	bool m_want_eof;
    };

  template<>
    buffer<wchar_t>::driver*
    new_ifdriver<wchar_t>(char const* name, bool want_eof) {
	if (name == 0)
	    return new input_file_driver<wchar_t>(stdin, want_eof);
	else if (FILE* fp = fopen(name, "r"))
	    return new input_file_driver<wchar_t>(fp, want_eof);
	else
	    return 0;
    }

  template<>
    struct output_file_driver<wchar_t> : buffer<wchar_t>::driver {

	output_file_driver(FILE* fp_) : fp(fp_) {}

	virtual ~output_file_driver() {
	    if (fp && fp != stdout)
		fclose(fp);
	}

      protected:
	virtual void write(wchar_t* first, wchar_t* last) {
	    // XXX Buffering.
	    while (first != last) {
		if (fputwc(*first, fp) == WEOF) break;
		++first;
	    }
	}
	virtual bool is_error() { return ferror(fp); }

      private:
	FILE* fp;
    };

  template<>
    buffer<wchar_t>::driver*
    new_ofdriver<wchar_t>(char const* name)
    {
	if (name == 0)
	    return new output_file_driver<wchar_t>(stdout);
	else if (FILE* fp = fopen(name, "w"))
	    return new output_file_driver<wchar_t>(fp);
	else
	    return 0;
    }

  //
  //  Readline buffers
  //

  readline_driver<char>::
  readline_driver(char const* prompt, bool want_eof, char const* sep)
      : m_prompt(prompt), m_pending(sep),
	m_want_eof(want_eof), m_sep(sep), m_toggle(false) {}

  readline_driver<char>::
  ~readline_driver() {}

  char*
  readline_driver<char>::
  read(char* first, char* last)
  {
      if (m_pending && !*m_pending) {
	  if ((m_toggle = !m_toggle))
	      m_pending = readline(const_cast<char *>(m_prompt));
	  else
	      m_pending = m_sep;
      }
      if (m_pending) {
	  size_type n = strlen(m_pending);
	  if (n > (size_type)(last - first))
	      n = (last - first);
	  std::copy(m_pending, m_pending + n, first);
	  m_pending += n;
	  first += n;
	  return first;
      }
      else if (m_want_eof) {
	  *first++ = std::char_traits<char>::eof();
	  return first;
      }
      else
	  return 0;
  }

  readline_driver<wchar_t>::
  readline_driver(char const* prompt, bool want_eof, wchar_t const* sep)
      : m_prompt(prompt), m_pending(sep), m_alloc(0),
	m_want_eof(want_eof), m_sep(sep) {}

  readline_driver<wchar_t>::
  ~readline_driver() { delete[] m_alloc; }

  wchar_t*
  readline_driver<wchar_t>::
  read(wchar_t* first, wchar_t* last)
  {
      if (m_pending && !*m_pending) {
	  if ((m_toggle = !m_toggle)) {
	      delete[] m_alloc;
	      char* ln = readline(const_cast<char *>(m_prompt));
	      if (ln == 0) {
		  m_pending = 0;
		  if (m_want_eof) {
		      *first++ = std::char_traits<wchar_t>::eof();
		      return first;
		  }
		  else
		      return 0;
	      }
	      add_history(ln);
	      size_type n = mbstowcs(0, ln, 0) + 1;
	      m_pending = m_alloc = new wchar_t[n];
	      mbstowcs(m_alloc, ln, n);
	  }
	  else
	      m_pending = m_sep;
      }
      if (m_pending) {
	  size_type n = wcslen(m_pending);
	  if (n > (size_type)(last - first))
	      n = (last - first);
	  std::copy(m_pending, m_pending + n, first);
	  m_pending += n;
	  first += n;
	  return first;
      }
      else
	  return 0;
  }

}
}
