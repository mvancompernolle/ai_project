/* ANSI-C code produced by gperf version 3.0.4 */
/* Command-line: gperf -o -i 7 -C -k '1-4,$' -L ANSI-C -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "./lexor_keyword.gperf"

/* Command-line: gperf -o -i 1 -C -k 1-3,$ -L C -H keyword_hash -N check_identifier -tT lexor_keyword.gperf */

#include "config.h"
#include "parse_misc.h"
#include "parse.h"
#include <cstring>
#include "lexor_keyword.h"
#include "compiler.h"

#line 12 "./lexor_keyword.gperf"
struct lexor_keyword { const char*name; int mask; int tokenType; };

#define TOTAL_KEYWORDS 175
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 31
#define MAX_HASH_VALUE 574
/* maximum key range = 544, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
keyword_hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 142, 102,
        7, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575,   7, 575,  22,  82,  77,
        7,   7, 132, 117,  27,  17,  22,  62,  27,  97,
        7, 117,  22,   7,  27,   7,  32, 162,  52, 137,
      226,  12,  12, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575, 575, 575, 575, 575,
      575, 575, 575, 575, 575, 575
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct lexor_keyword *
check_identifier (register const char *str, register unsigned int len)
{
  static const struct lexor_keyword wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 59 "./lexor_keyword.gperf"
      {"end",			GN_KEYWORDS_1364_1995,		K_end},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 149 "./lexor_keyword.gperf"
      {"sin",			GN_KEYWORDS_VAMS_2_3,		K_sin},
      {""},
#line 103 "./lexor_keyword.gperf"
      {"ln",			GN_KEYWORDS_VAMS_2_3,		K_ln},
#line 66 "./lexor_keyword.gperf"
      {"endnature",		GN_KEYWORDS_VAMS_2_3,		K_endnature},
      {""},
#line 24 "./lexor_keyword.gperf"
      {"and",			GN_KEYWORDS_1364_1995,		K_and},
      {""},
#line 62 "./lexor_keyword.gperf"
      {"enddiscipline",		GN_KEYWORDS_VAMS_2_3,		K_enddiscipline},
      {""},
#line 68 "./lexor_keyword.gperf"
      {"endspecify",		GN_KEYWORDS_1364_1995,		K_endspecify},
#line 52 "./lexor_keyword.gperf"
      {"design",			GN_KEYWORDS_1364_2001_CONFIG,	K_design},
      {""}, {""},
#line 112 "./lexor_keyword.gperf"
      {"nand",			GN_KEYWORDS_1364_1995,		K_nand},
      {""}, {""}, {""},
#line 49 "./lexor_keyword.gperf"
      {"deassign",		GN_KEYWORDS_1364_1995,		K_deassign},
#line 58 "./lexor_keyword.gperf"
      {"else",			GN_KEYWORDS_1364_1995,		K_else},
      {""}, {""},
#line 67 "./lexor_keyword.gperf"
      {"endprimitive",		GN_KEYWORDS_1364_1995,		K_endprimitive},
      {""},
#line 25 "./lexor_keyword.gperf"
      {"asin",			GN_KEYWORDS_VAMS_2_3,		K_asin},
      {""},
#line 28 "./lexor_keyword.gperf"
      {"assign",			GN_KEYWORDS_1364_1995,		K_assign},
#line 53 "./lexor_keyword.gperf"
      {"disable",		GN_KEYWORDS_1364_1995,		K_disable},
#line 69 "./lexor_keyword.gperf"
      {"endtable",		GN_KEYWORDS_1364_1995,		K_endtable},
      {""},
#line 48 "./lexor_keyword.gperf"
      {"ddt_nature",		GN_KEYWORDS_VAMS_2_3,		K_ddt_nature},
#line 161 "./lexor_keyword.gperf"
      {"tan",			GN_KEYWORDS_VAMS_2_3,		K_tan},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 97 "./lexor_keyword.gperf"
      {"instance",		GN_KEYWORDS_1364_2001_CONFIG,	K_instance},
      {""},
#line 88 "./lexor_keyword.gperf"
      {"idt_nature",		GN_KEYWORDS_VAMS_2_3,		K_idt_nature},
#line 27 "./lexor_keyword.gperf"
      {"assert",			GN_KEYWORDS_1800_2005,		K_assert},
#line 139 "./lexor_keyword.gperf"
      {"release",		GN_KEYWORDS_1364_1995,		K_release},
      {""}, {""},
#line 26 "./lexor_keyword.gperf"
      {"asinh",			GN_KEYWORDS_VAMS_2_3,		K_asinh},
      {""}, {""}, {""},
#line 150 "./lexor_keyword.gperf"
      {"sinh",			GN_KEYWORDS_VAMS_2_3,		K_sinh},
      {""}, {""}, {""}, {""},
#line 29 "./lexor_keyword.gperf"
      {"atan",			GN_KEYWORDS_VAMS_2_3,		K_atan},
#line 30 "./lexor_keyword.gperf"
      {"atan2",			GN_KEYWORDS_VAMS_2_3,		K_atan2},
#line 168 "./lexor_keyword.gperf"
      {"tri",			GN_KEYWORDS_1364_1995,		K_tri},
#line 98 "./lexor_keyword.gperf"
      {"integer",		GN_KEYWORDS_1364_1995,		K_integer},
#line 137 "./lexor_keyword.gperf"
      {"realtime",		GN_KEYWORDS_1364_1995,		K_realtime},
#line 165 "./lexor_keyword.gperf"
      {"tran",			GN_KEYWORDS_1364_1995,		K_tran},
      {""},
#line 140 "./lexor_keyword.gperf"
      {"repeat",			GN_KEYWORDS_1364_1995,		K_repeat},
      {""}, {""}, {""}, {""}, {""},
#line 94 "./lexor_keyword.gperf"
      {"initial",		GN_KEYWORDS_1364_1995,		K_initial},
      {""},
#line 154 "./lexor_keyword.gperf"
      {"sqrt",			GN_KEYWORDS_VAMS_2_3,		K_sqrt},
#line 71 "./lexor_keyword.gperf"
      {"event",			GN_KEYWORDS_1364_1995,		K_event},
#line 171 "./lexor_keyword.gperf"
      {"triand",			GN_KEYWORDS_1364_1995,		K_triand},
#line 60 "./lexor_keyword.gperf"
      {"endcase",		GN_KEYWORDS_1364_1995,		K_endcase},
      {""},
#line 136 "./lexor_keyword.gperf"
      {"real",			GN_KEYWORDS_1364_1995,		K_real},
#line 31 "./lexor_keyword.gperf"
      {"atanh",			GN_KEYWORDS_VAMS_2_3,		K_atanh},
      {""}, {""}, {""},
#line 162 "./lexor_keyword.gperf"
      {"tanh",			GN_KEYWORDS_VAMS_2_3,		K_tanh},
#line 143 "./lexor_keyword.gperf"
      {"rtran",			GN_KEYWORDS_1364_1995,		K_rtran},
#line 14 "./lexor_keyword.gperf"
      {"abs",			GN_KEYWORDS_VAMS_2_3,		K_abs},
#line 70 "./lexor_keyword.gperf"
      {"endtask",		GN_KEYWORDS_1364_1995,		K_endtask},
#line 55 "./lexor_keyword.gperf"
      {"discrete",		GN_KEYWORDS_VAMS_2_3,		K_discrete},
#line 38 "./lexor_keyword.gperf"
      {"case",			GN_KEYWORDS_1364_1995,		K_case},
#line 54 "./lexor_keyword.gperf"
      {"discipline",		GN_KEYWORDS_VAMS_2_3,		K_discipline},
      {""}, {""}, {""},
#line 123 "./lexor_keyword.gperf"
      {"parameter",		GN_KEYWORDS_1364_1995,		K_parameter},
#line 40 "./lexor_keyword.gperf"
      {"casez",			GN_KEYWORDS_1364_1995,		K_casez},
#line 110 "./lexor_keyword.gperf"
      {"min",			GN_KEYWORDS_VAMS_2_3,		K_min},
#line 152 "./lexor_keyword.gperf"
      {"specify",		GN_KEYWORDS_1364_1995,		K_specify},
      {""},
#line 65 "./lexor_keyword.gperf"
      {"endmodule",		GN_KEYWORDS_1364_1995,		K_endmodule},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 91 "./lexor_keyword.gperf"
      {"incdir",			GN_KEYWORDS_1364_2001_CONFIG,	K_incdir},
#line 92 "./lexor_keyword.gperf"
      {"include",		GN_KEYWORDS_1364_2001_CONFIG,	K_include},
      {""}, {""}, {""}, {""}, {""},
#line 146 "./lexor_keyword.gperf"
      {"scalared",		GN_KEYWORDS_1364_1995,		K_scalared},
#line 57 "./lexor_keyword.gperf"
      {"edge",			GN_KEYWORDS_1364_1995,		K_edge},
      {""}, {""},
#line 114 "./lexor_keyword.gperf"
      {"negedge",		GN_KEYWORDS_1364_1995,		K_negedge},
#line 82 "./lexor_keyword.gperf"
      {"generate",		GN_KEYWORDS_1364_2001,		K_generate},
      {""}, {""},
#line 64 "./lexor_keyword.gperf"
      {"endgenerate",		GN_KEYWORDS_1364_2001,		K_endgenerate},
      {""}, {""},
#line 41 "./lexor_keyword.gperf"
      {"ceil",			GN_KEYWORDS_VAMS_2_3,		K_ceil},
      {""},
#line 148 "./lexor_keyword.gperf"
      {"signed",			GN_KEYWORDS_1364_2001,		K_signed},
      {""}, {""},
#line 164 "./lexor_keyword.gperf"
      {"time",			GN_KEYWORDS_1364_1995,		K_time},
      {""}, {""},
#line 125 "./lexor_keyword.gperf"
      {"posedge",		GN_KEYWORDS_1364_1995,		K_posedge},
      {""},
#line 42 "./lexor_keyword.gperf"
      {"cell",			GN_KEYWORDS_1364_2001_CONFIG,	K_cell},
      {""},
#line 63 "./lexor_keyword.gperf"
      {"endfunction",		GN_KEYWORDS_1364_1995,		K_endfunction},
#line 102 "./lexor_keyword.gperf"
      {"library",		GN_KEYWORDS_1364_2001_CONFIG,	K_library},
#line 121 "./lexor_keyword.gperf"
      {"or",			GN_KEYWORDS_1364_1995,		K_or},
#line 99 "./lexor_keyword.gperf"
      {"join",			GN_KEYWORDS_1364_1995,		K_join},
#line 160 "./lexor_keyword.gperf"
      {"table",			GN_KEYWORDS_1364_1995,		K_table},
#line 15 "./lexor_keyword.gperf"
      {"abstol",			GN_KEYWORDS_VAMS_2_3,		K_abstol},
      {""}, {""},
#line 128 "./lexor_keyword.gperf"
      {"primitive",		GN_KEYWORDS_1364_1995,		K_primitive},
#line 117 "./lexor_keyword.gperf"
      {"noshowcancelled",	GN_KEYWORDS_1364_2001,		K_noshowcancelled},
#line 116 "./lexor_keyword.gperf"
      {"nor",			GN_KEYWORDS_1364_1995,		K_nor},
      {""},
#line 179 "./lexor_keyword.gperf"
      {"vectored",		GN_KEYWORDS_1364_1995,		K_vectored},
#line 181 "./lexor_keyword.gperf"
      {"wand",			GN_KEYWORDS_1364_1995,		K_wand},
#line 151 "./lexor_keyword.gperf"
      {"small",			GN_KEYWORDS_1364_1995,		K_small},
#line 177 "./lexor_keyword.gperf"
      {"use",			GN_KEYWORDS_1364_2001_CONFIG,	K_use},
      {""}, {""},
#line 163 "./lexor_keyword.gperf"
      {"task",			GN_KEYWORDS_1364_1995,		K_task},
      {""},
#line 118 "./lexor_keyword.gperf"
      {"not",			GN_KEYWORDS_1364_1995,		K_not},
#line 101 "./lexor_keyword.gperf"
      {"liblist",		GN_KEYWORDS_1364_2001_CONFIG,	K_liblist},
      {""}, {""}, {""},
#line 16 "./lexor_keyword.gperf"
      {"access",			GN_KEYWORDS_VAMS_2_3,		K_access},
#line 167 "./lexor_keyword.gperf"
      {"tranif1",		GN_KEYWORDS_1364_1995,		K_tranif1},
      {""},
#line 185 "./lexor_keyword.gperf"
      {"wire",			GN_KEYWORDS_1364_1995,		K_wire},
      {""},
#line 23 "./lexor_keyword.gperf"
      {"analog",			GN_KEYWORDS_VAMS_2_3,		K_analog},
      {""}, {""}, {""},
#line 100 "./lexor_keyword.gperf"
      {"large",			GN_KEYWORDS_1364_1995,		K_large},
#line 155 "./lexor_keyword.gperf"
      {"string",			GN_KEYWORDS_VAMS_2_3,		K_string},
#line 50 "./lexor_keyword.gperf"
      {"default",		GN_KEYWORDS_1364_1995,		K_default},
#line 176 "./lexor_keyword.gperf"
      {"unsigned",		GN_KEYWORDS_1364_2001,		K_unsigned},
      {""}, {""},
#line 46 "./lexor_keyword.gperf"
      {"cos",			GN_KEYWORDS_VAMS_2_3,		K_cos},
      {""}, {""},
#line 126 "./lexor_keyword.gperf"
      {"potential",		GN_KEYWORDS_VAMS_2_3,		K_potential},
#line 87 "./lexor_keyword.gperf"
      {"hypot",			GN_KEYWORDS_VAMS_2_3,		K_hypot},
#line 83 "./lexor_keyword.gperf"
      {"genvar",			GN_KEYWORDS_1364_2001,		K_genvar},
      {""},
#line 145 "./lexor_keyword.gperf"
      {"rtranif1",		GN_KEYWORDS_1364_1995,		K_rtranif1},
#line 153 "./lexor_keyword.gperf"
      {"specparam",		GN_KEYWORDS_1364_1995,		K_specparam},
#line 184 "./lexor_keyword.gperf"
      {"while",			GN_KEYWORDS_1364_1995,		K_while},
#line 19 "./lexor_keyword.gperf"
      {"always",			GN_KEYWORDS_1364_1995,		K_always},
      {""}, {""},
#line 61 "./lexor_keyword.gperf"
      {"endconfig",		GN_KEYWORDS_1364_2001_CONFIG,	K_endconfig},
#line 172 "./lexor_keyword.gperf"
      {"trior",			GN_KEYWORDS_1364_1995,		K_trior},
#line 173 "./lexor_keyword.gperf"
      {"trireg",			GN_KEYWORDS_1364_1995,		K_trireg},
      {""}, {""}, {""},
#line 174 "./lexor_keyword.gperf"
      {"units",			GN_KEYWORDS_VAMS_2_3,		K_units},
#line 109 "./lexor_keyword.gperf"
      {"medium",			GN_KEYWORDS_1364_1995,		K_medium},
      {""}, {""},
#line 17 "./lexor_keyword.gperf"
      {"acos",			GN_KEYWORDS_VAMS_2_3,		K_acos},
#line 33 "./lexor_keyword.gperf"
      {"begin",			GN_KEYWORDS_1364_1995,		K_begin},
#line 113 "./lexor_keyword.gperf"
      {"nature",			GN_KEYWORDS_VAMS_2_3,		K_nature},
#line 166 "./lexor_keyword.gperf"
      {"tranif0",		GN_KEYWORDS_1364_1995,		K_tranif0},
      {""},
#line 115 "./lexor_keyword.gperf"
      {"nmos",			GN_KEYWORDS_1364_1995,		K_nmos},
      {""},
#line 107 "./lexor_keyword.gperf"
      {"macromodule",		GN_KEYWORDS_1364_1995,		K_macromodule},
      {""}, {""},
#line 180 "./lexor_keyword.gperf"
      {"wait",			GN_KEYWORDS_1364_1995,		K_wait},
#line 96 "./lexor_keyword.gperf"
      {"input",			GN_KEYWORDS_1364_1995,		K_input},
      {""},
#line 22 "./lexor_keyword.gperf"
      {"always_latch",		GN_KEYWORDS_1800_2005,		K_always_latch},
      {""}, {""},
#line 45 "./lexor_keyword.gperf"
      {"continuous",		GN_KEYWORDS_VAMS_2_3,		K_continuous},
      {""}, {""},
#line 131 "./lexor_keyword.gperf"
      {"pulldown",		GN_KEYWORDS_1364_1995,		K_pulldown},
#line 124 "./lexor_keyword.gperf"
      {"pmos",			GN_KEYWORDS_1364_1995,		K_pmos},
#line 18 "./lexor_keyword.gperf"
      {"acosh",			GN_KEYWORDS_VAMS_2_3,		K_acosh},
#line 56 "./lexor_keyword.gperf"
      {"domain",			GN_KEYWORDS_VAMS_2_3,		K_domain},
      {""},
#line 144 "./lexor_keyword.gperf"
      {"rtranif0",		GN_KEYWORDS_1364_1995,		K_rtranif0},
#line 47 "./lexor_keyword.gperf"
      {"cosh",			GN_KEYWORDS_VAMS_2_3,		K_cosh},
#line 141 "./lexor_keyword.gperf"
      {"rnmos",			GN_KEYWORDS_1364_1995,		K_rnmos},
      {""}, {""}, {""}, {""}, {""},
#line 132 "./lexor_keyword.gperf"
      {"pullup",			GN_KEYWORDS_1364_1995,		K_pullup},
      {""},
#line 133 "./lexor_keyword.gperf"
      {"pulsestyle_onevent",	GN_KEYWORDS_1364_2001,		K_pulsestyle_onevent},
#line 134 "./lexor_keyword.gperf"
      {"pulsestyle_ondetect",	GN_KEYWORDS_1364_2001,		K_pulsestyle_ondetect},
      {""},
#line 138 "./lexor_keyword.gperf"
      {"reg",			GN_KEYWORDS_1364_1995,		K_reg},
      {""},
#line 51 "./lexor_keyword.gperf"
      {"defparam",		GN_KEYWORDS_1364_1995,		K_defparam},
      {""},
#line 142 "./lexor_keyword.gperf"
      {"rpmos",			GN_KEYWORDS_1364_1995,		K_rpmos},
      {""}, {""}, {""},
#line 187 "./lexor_keyword.gperf"
      {"wone",			GN_KEYWORDS_1364_2005,		K_wone},
#line 73 "./lexor_keyword.gperf"
      {"exp",			GN_KEYWORDS_VAMS_2_3,		K_exp},
#line 120 "./lexor_keyword.gperf"
      {"notif1",			GN_KEYWORDS_1364_1995,		K_notif1},
      {""},
#line 89 "./lexor_keyword.gperf"
      {"if",			GN_KEYWORDS_1364_1995,		K_if},
#line 170 "./lexor_keyword.gperf"
      {"tri1",			GN_KEYWORDS_1364_1995,		K_tri1},
      {""},
#line 90 "./lexor_keyword.gperf"
      {"ifnone",			GN_KEYWORDS_1364_1995,		K_ifnone},
      {""}, {""}, {""}, {""},
#line 93 "./lexor_keyword.gperf"
      {"inf",			GN_KEYWORDS_VAMS_2_3,		K_inf},
#line 157 "./lexor_keyword.gperf"
      {"strong1",		GN_KEYWORDS_1364_1995,		K_strong1},
      {""}, {""}, {""},
#line 86 "./lexor_keyword.gperf"
      {"highz1",			GN_KEYWORDS_1364_1995,		K_highz1},
      {""}, {""}, {""}, {""},
#line 20 "./lexor_keyword.gperf"
      {"always_comb",		GN_KEYWORDS_1800_2005,		K_always_comb},
      {""}, {""}, {""}, {""},
#line 76 "./lexor_keyword.gperf"
      {"for",			GN_KEYWORDS_1364_1995,		K_for},
      {""},
#line 147 "./lexor_keyword.gperf"
      {"showcancelled",		GN_KEYWORDS_1364_2001,		K_showcancelled},
#line 43 "./lexor_keyword.gperf"
      {"cmos",			GN_KEYWORDS_1364_1995,		K_cmos},
      {""},
#line 188 "./lexor_keyword.gperf"
      {"wor",			GN_KEYWORDS_1364_1995,		K_wor},
      {""}, {""}, {""}, {""}, {""},
#line 78 "./lexor_keyword.gperf"
      {"forever",		GN_KEYWORDS_1364_1995,		K_forever},
      {""}, {""}, {""},
#line 119 "./lexor_keyword.gperf"
      {"notif0",			GN_KEYWORDS_1364_1995,		K_notif0},
#line 159 "./lexor_keyword.gperf"
      {"supply1",		GN_KEYWORDS_1364_1995,		K_supply1},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 135 "./lexor_keyword.gperf"
      {"rcmos",			GN_KEYWORDS_1364_1995,		K_rcmos},
      {""},
#line 156 "./lexor_keyword.gperf"
      {"strong0",		GN_KEYWORDS_1364_1995,		K_strong0},
      {""}, {""},
#line 183 "./lexor_keyword.gperf"
      {"weak1",			GN_KEYWORDS_1364_1995,		K_weak1},
#line 85 "./lexor_keyword.gperf"
      {"highz0",			GN_KEYWORDS_1364_1995,		K_highz0},
      {""}, {""}, {""},
#line 95 "./lexor_keyword.gperf"
      {"inout",			GN_KEYWORDS_1364_1995,		K_inout},
      {""}, {""}, {""},
#line 39 "./lexor_keyword.gperf"
      {"casex",			GN_KEYWORDS_1364_1995,		K_casex},
#line 130 "./lexor_keyword.gperf"
      {"pull1",			GN_KEYWORDS_1364_1995,		K_pull1},
      {""}, {""}, {""},
#line 21 "./lexor_keyword.gperf"
      {"always_ff",		GN_KEYWORDS_1800_2005,		K_always_ff},
#line 104 "./lexor_keyword.gperf"
      {"localparam",		GN_KEYWORDS_1364_2001,		K_localparam},
#line 72 "./lexor_keyword.gperf"
      {"exclude",		GN_KEYWORDS_VAMS_2_3,		K_exclude},
      {""}, {""}, {""},
#line 178 "./lexor_keyword.gperf"
      {"uwire",			GN_KEYWORDS_1364_2005,		K_uwire},
      {""}, {""}, {""}, {""},
#line 106 "./lexor_keyword.gperf"
      {"logic",			GN_KEYWORDS_ICARUS,		K_logic},
      {""},
#line 158 "./lexor_keyword.gperf"
      {"supply0",		GN_KEYWORDS_1364_1995,		K_supply0},
      {""},
#line 169 "./lexor_keyword.gperf"
      {"tri0",			GN_KEYWORDS_1364_1995,		K_tri0},
#line 77 "./lexor_keyword.gperf"
      {"force",			GN_KEYWORDS_1364_1995,		K_force},
      {""}, {""}, {""}, {""}, {""},
#line 122 "./lexor_keyword.gperf"
      {"output",			GN_KEYWORDS_1364_1995,		K_output},
      {""}, {""},
#line 34 "./lexor_keyword.gperf"
      {"bool",			GN_KEYWORDS_ICARUS,		K_bool},
#line 182 "./lexor_keyword.gperf"
      {"weak0",			GN_KEYWORDS_1364_1995,		K_weak0},
      {""}, {""}, {""}, {""}, {""},
#line 105 "./lexor_keyword.gperf"
      {"log",			GN_KEYWORDS_VAMS_2_3,		K_log},
      {""}, {""}, {""},
#line 129 "./lexor_keyword.gperf"
      {"pull0",			GN_KEYWORDS_1364_1995,		K_pull0},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 81 "./lexor_keyword.gperf"
      {"function",		GN_KEYWORDS_1364_1995,		K_function},
      {""}, {""},
#line 111 "./lexor_keyword.gperf"
      {"module",			GN_KEYWORDS_1364_1995,		K_module},
      {""}, {""}, {""},
#line 190 "./lexor_keyword.gperf"
      {"xor",			GN_KEYWORDS_1364_1995,		K_xor},
      {""}, {""}, {""},
#line 79 "./lexor_keyword.gperf"
      {"fork",			GN_KEYWORDS_1364_1995,		K_fork},
      {""}, {""}, {""},
#line 189 "./lexor_keyword.gperf"
      {"xnor",			GN_KEYWORDS_1364_1995,		K_xnor},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 127 "./lexor_keyword.gperf"
      {"pow",			GN_KEYWORDS_VAMS_2_3,		K_pow},
      {""}, {""},
#line 32 "./lexor_keyword.gperf"
      {"automatic",		GN_KEYWORDS_1364_2001,		K_automatic},
      {""}, {""}, {""}, {""}, {""},
#line 74 "./lexor_keyword.gperf"
      {"floor",			GN_KEYWORDS_VAMS_2_3,		K_floor},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 84 "./lexor_keyword.gperf"
      {"ground",			GN_KEYWORDS_VAMS_2_3,		K_ground},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 44 "./lexor_keyword.gperf"
      {"config",			GN_KEYWORDS_1364_2001_CONFIG,	K_config},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 80 "./lexor_keyword.gperf"
      {"from",			GN_KEYWORDS_VAMS_2_3,		K_from},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 37 "./lexor_keyword.gperf"
      {"bufif1",			GN_KEYWORDS_1364_1995,		K_bufif1},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 35 "./lexor_keyword.gperf"
      {"buf",			GN_KEYWORDS_1364_1995,		K_buf},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 36 "./lexor_keyword.gperf"
      {"bufif0",			GN_KEYWORDS_1364_1995,		K_bufif0},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 75 "./lexor_keyword.gperf"
      {"flow",			GN_KEYWORDS_VAMS_2_3,		K_flow},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 108 "./lexor_keyword.gperf"
      {"max",			GN_KEYWORDS_VAMS_2_3,		K_max}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 191 "./lexor_keyword.gperf"


int lexor_keyword_mask = 0;

int lexor_keyword_code(const char*str, unsigned nstr)
{
      const struct lexor_keyword*rc = check_identifier(str, nstr);
      if (rc == 0)
	  return IDENTIFIER;
      else if ((rc->mask & lexor_keyword_mask) == 0)
          return IDENTIFIER;
      else
	  return rc->tokenType;
}
