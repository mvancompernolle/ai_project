#include <more/math/hash.h>


namespace more {
namespace math {

  namespace bits {

    // 64 bits mixing function by Bob Jenkins, 1996--1997
    // See http://burtleburtle.net/bob/hash/
    inline void
    mix64(uint_least64_t& a, uint_least64_t& b, uint_least64_t& c)
    {
	// 64 bits, from 1997
	a -= b; a -= c; a ^= (c>>43);
	b -= c; b -= a; b ^= (a<<9);
	c -= a; c -= b; c ^= (b>>8);
	a -= b; a -= c; a ^= (c>>38);
	b -= c; b -= a; b ^= (a<<23);
	c -= a; c -= b; c ^= (b>>5);
	a -= b; a -= c; a ^= (c>>35);
	b -= c; b -= a; b ^= (a<<49);
	c -= a; c -= b; c ^= (b>>11);
	a -= b; a -= c; a ^= (c>>12);
	b -= c; b -= a; b ^= (a<<18);
	c -= a; c -= b; c ^= (b>>22);
    }
    // 32 bits mixing function by Bob Jenkins, 1996--1997
    // See http://burtleburtle.net/bob/hash/
    inline void
    mix32(uint_least32_t& a, uint_least32_t& b, uint_least32_t& c)
    {
	if (sizeof(uint_least32_t) == 4) {
	    a -= b; a -= c; a ^= (c>>13);
	    b -= c; b -= a; b ^= (a<<8);
	    c -= a; c -= b; c ^= (b>>13);
	    a -= b; a -= c; a ^= (c>>12);
	    b -= c; b -= a; b ^= (a<<16);
	    c -= a; c -= b; c ^= (b>>5);
	    a -= b; a -= c; a ^= (c>>3);
	    b -= c; b -= a; b ^= (a<<10);
	    c -= a; c -= b; c ^= (b>>15);
	} else {
	    a -= b; a -= c; a ^= (c>>13);
	    b -= c; b -= a; b ^= (a<< 8);
	    c -= a; c -= b; c ^= ((b&0xffffffff)>>13);
	    a -= b; a -= c; a ^= ((c&0xffffffff)>>12);
	    b -= c; b -= a; b = (b ^ (a<<16)) & 0xffffffff;
	    c -= a; c -= b; c = (c ^ (b>> 5)) & 0xffffffff;
	    a -= b; a -= c; a = (a ^ (c>> 3)) & 0xffffffff;
	    b -= c; b -= a; b = (b ^ (a<<10)) & 0xffffffff;
	    c -= a; c -= b; c = (c ^ (b>>15)) & 0xffffffff;
	}
    }

    void
    hash_base<192>::insert_impl(unsigned char const* it,
				unsigned char const* it_end)
    {
	if (std::size_t(it_end - it) >= m_n_free)
	    switch (m_n_free) {
	      case 24: m_st[0] += (word_type)*it++;
	      case 23: m_st[0] += (word_type)*it++ << 8;
	      case 22: m_st[0] += (word_type)*it++ << 16;
	      case 21: m_st[0] += (word_type)*it++ << 24;
	      case 20: m_st[0] += (word_type)*it++ << 32;
	      case 19: m_st[0] += (word_type)*it++ << 40;
	      case 18: m_st[0] += (word_type)*it++ << 48;
	      case 17: m_st[0] += (word_type)*it++ << 56;
	      case 16: m_st[1] += (word_type)*it++;
	      case 15: m_st[1] += (word_type)*it++ << 8;
	      case 14: m_st[1] += (word_type)*it++ << 16;
	      case 13: m_st[1] += (word_type)*it++ << 24;
	      case 12: m_st[1] += (word_type)*it++ << 32;
	      case 11: m_st[1] += (word_type)*it++ << 40;
	      case 10: m_st[1] += (word_type)*it++ << 48;
	      case  9: m_st[1] += (word_type)*it++ << 56;
	      case  8: m_st[2] += (word_type)*it++;
	      case  7: m_st[2] += (word_type)*it++ << 8;
	      case  6: m_st[2] += (word_type)*it++ << 16;
	      case  5: m_st[2] += (word_type)*it++ << 24;
	      case  4: m_st[2] += (word_type)*it++ << 32;
	      case  3: m_st[2] += (word_type)*it++ << 40;
	      case  2: m_st[2] += (word_type)*it++ << 48;
	      case  1: m_st[2] += (word_type)*it++ << 56;
	      default:
		break;
	    }
	else {
	    std::size_t save = m_n_free;
	    m_n_free -= it_end - it;
	    switch (save) {
	      case 24:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++;
	      case 23:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 8;
	      case 22:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 16;
	      case 21:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 24;
	      case 20:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 32;
	      case 19:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 40;
	      case 18:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 48;
	      case 17:
		if (it == it_end) return;
		m_st[0] += (word_type)(word_type)*it++ << 56;
	      case 16:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++;
	      case 15:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 8;
	      case 14:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 16;
	      case 13:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 24;
	      case 12:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 32;
	      case 11:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 40;
	      case 10:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 48;
	      case  9:
		if (it == it_end) return;
		m_st[1] += (word_type)(word_type)*it++ << 56;
	      case  8:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++;
	      case  7:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 8;
	      case  6:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 16;
	      case  5:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 24;
	      case  4:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 32;
	      case  3:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 40;
	      case  2:
		if (it == it_end) return;
		m_st[2] += (word_type)(word_type)*it++ << 48;
	      default:
		return;
	    }
	}
	bits::mix64(m_st[0], m_st[1], m_st[2]);
	while (it_end - it >= 24) {
	    m_st[0] += (word_type)*it++;
	    m_st[0] += (word_type)*it++ << 8;
	    m_st[0] += (word_type)*it++ << 16;
	    m_st[0] += (word_type)*it++ << 24;
	    m_st[0] += (word_type)*it++ << 32;
	    m_st[0] += (word_type)*it++ << 40;
	    m_st[0] += (word_type)*it++ << 48;
	    m_st[0] += (word_type)*it++ << 56;
	    m_st[1] += (word_type)*it++;
	    m_st[1] += (word_type)*it++ << 8;
	    m_st[1] += (word_type)*it++ << 16;
	    m_st[1] += (word_type)*it++ << 24;
	    m_st[1] += (word_type)*it++ << 32;
	    m_st[1] += (word_type)*it++ << 40;
	    m_st[1] += (word_type)*it++ << 48;
	    m_st[1] += (word_type)*it++ << 56;
	    m_st[2] += (word_type)*it++;
	    m_st[2] += (word_type)*it++ << 8;
	    m_st[2] += (word_type)*it++ << 16;
	    m_st[2] += (word_type)*it++ << 24;
	    m_st[2] += (word_type)*it++ << 32;
	    m_st[2] += (word_type)*it++ << 40;
	    m_st[2] += (word_type)*it++ << 48;
	    m_st[2] += (word_type)*it++ << 56;
	    bits::mix64(m_st[0], m_st[1], m_st[2]);
	}
	m_n_free = 24 - (it_end - it);
	switch (it_end - it) {
	  case 23: m_st[2] += (word_type)*--it_end << 48;
	  case 22: m_st[2] += (word_type)*--it_end << 40;
	  case 21: m_st[2] += (word_type)*--it_end << 32;
	  case 20: m_st[2] += (word_type)*--it_end << 24;
	  case 19: m_st[2] += (word_type)*--it_end << 16;
	  case 18: m_st[2] += (word_type)*--it_end << 8;
	  case 17: m_st[2] += (word_type)*--it_end;
	  case 16: m_st[1] += (word_type)*--it_end << 56;
	  case 15: m_st[1] += (word_type)*--it_end << 48;
	  case 14: m_st[1] += (word_type)*--it_end << 40;
	  case 13: m_st[1] += (word_type)*--it_end << 32;
	  case 12: m_st[1] += (word_type)*--it_end << 24;
	  case 11: m_st[1] += (word_type)*--it_end << 16;
	  case 10: m_st[1] += (word_type)*--it_end << 8;
	  case  9: m_st[1] += (word_type)*--it_end;
	  case  8: m_st[0] += (word_type)*--it_end << 56;
	  case  7: m_st[0] += (word_type)*--it_end << 48;
	  case  6: m_st[0] += (word_type)*--it_end << 40;
	  case  5: m_st[0] += (word_type)*--it_end << 32;
	  case  4: m_st[0] += (word_type)*--it_end << 24;
	  case  3: m_st[0] += (word_type)*--it_end << 16;
	  case  2: m_st[0] += (word_type)*--it_end << 8;
	  case  1: m_st[0] += (word_type)*--it_end;
	  default: break;
	}
    }

    void
    hash_base<96>::insert_impl(unsigned char const* it,
			       unsigned char const* it_end)
    {
	if (it_end - it >= m_n_free)
	    switch (m_n_free) {
	      case 12: m_st[0] += (word_type)*it++;
	      case 11: m_st[0] += (word_type)*it++ << 8;
	      case 10: m_st[0] += (word_type)*it++ << 16;
	      case  9: m_st[0] += (word_type)*it++ << 24;
	      case  8: m_st[1] += (word_type)*it++;
	      case  7: m_st[1] += (word_type)*it++ << 8;
	      case  6: m_st[1] += (word_type)*it++ << 16;
	      case  5: m_st[1] += (word_type)*it++ << 24;
	      case  4: m_st[2] += (word_type)*it++;
	      case  3: m_st[2] += (word_type)*it++ << 8;
	      case  2: m_st[2] += (word_type)*it++ << 16;
	      case  1: m_st[2] += (word_type)*it++ << 24;
	      default:
		break;
	    }
	else {
	    std::size_t save = m_n_free;
	    m_n_free -= it_end - it;
	    switch (save) {
	      case 12:
		if (it == it_end) return;
		m_st[0] += (word_type)*it++;
	      case 11:
		if (it == it_end) return;
		m_st[0] += (word_type)*it++ << 8;
	      case 10:
		if (it == it_end) return;
		m_st[0] += (word_type)*it++ << 16;
	      case  9:
		if (it == it_end) return;
		m_st[0] += (word_type)*it++ << 24;
	      case  8:
		if (it == it_end) return;
		m_st[1] += (word_type)*it++;
	      case  7:
		if (it == it_end) return;
		m_st[1] += (word_type)*it++ << 8;
	      case  6:
		if (it == it_end) return;
		m_st[1] += (word_type)*it++ << 16;
	      case  5:
		if (it == it_end) return;
		m_st[1] += (word_type)*it++ << 24;
	      case  4:
		if (it == it_end) return;
		m_st[2] += (word_type)*it++;
	      case  3:
		if (it == it_end) return;
		m_st[2] += (word_type)*it++ << 8;
	      case  2:
		if (it == it_end) return;
		m_st[2] += (word_type)*it++ << 16;
	      default:
		return;
	    }
	}
	bits::mix32(m_st[0], m_st[1], m_st[2]);
	while (it_end - it >= 12) {
	    m_st[0] += (word_type)*it++;
	    m_st[0] += (word_type)*it++ << 8;
	    m_st[0] += (word_type)*it++ << 16;
	    m_st[0] += (word_type)*it++ << 24;
	    m_st[1] += (word_type)*it++;
	    m_st[1] += (word_type)*it++ << 8;
	    m_st[1] += (word_type)*it++ << 16;
	    m_st[1] += (word_type)*it++ << 24;
	    m_st[2] += (word_type)*it++;
	    m_st[2] += (word_type)*it++ << 8;
	    m_st[2] += (word_type)*it++ << 16;
	    m_st[2] += (word_type)*it++ << 24;
	    bits::mix32(m_st[0], m_st[1], m_st[2]);
	}
	m_n_free = 12 - (it_end - it);
	switch (it_end - it) {
	  case 11: m_st[2] += (word_type)*--it_end << 16;
	  case 10: m_st[2] += (word_type)*--it_end << 8;
	  case  9: m_st[2] += (word_type)*--it_end;
	  case  8: m_st[1] += (word_type)*--it_end << 24;
	  case  7: m_st[1] += (word_type)*--it_end << 16;
	  case  6: m_st[1] += (word_type)*--it_end << 8;
	  case  5: m_st[1] += (word_type)*--it_end;
	  case  4: m_st[0] += (word_type)*--it_end << 24;
	  case  3: m_st[0] += (word_type)*--it_end << 16;
	  case  2: m_st[0] += (word_type)*--it_end << 8;
	  case  1: m_st[0] += (word_type)*--it_end;
	  default: break;
	}
    }
  }

  void
  encode64_alnum(lang::uint_least64_t x, char* s)
  {
      *s++ = encode_alpha(x % 52);
      x /= 52;
      for (int i = 0; i < 9; ++i) {
	  *s++ = encode_alnum(x % 62);
	  x /= 62;
      }
      *s++ = encode_alnum(x);
  }

  lang::uint_least64_t
  decode64_alnum(char const* s)
  {
      s += 10;
      lang::uint_least64_t x = decode_alnum(*s);
      for (int i = 0; i < 9; ++i) {
	  x *= 62;
	  x += decode_alnum(*--s);
      }
      x *= 52;
      x += decode_alpha(*--s);
      return x;
  }


}}
