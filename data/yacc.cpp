
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 2 "yacc.yy"

/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 2007 Ewald Arnold <ulxmlrpcpp@ewald-arnold.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

//#include <config.h>

// Workaround for a bison issue:
// bison.simple concludes from _GNU_SOURCE that stpcpy is available,
// while GNU string.h only exposes it if __USE_GNU is set.
#ifdef _GNU_SOURCE
#define __USE_GNU 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mstring.h"

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern MString idl_lexFile;
extern int idl_line_no;
extern int function_mode;

static int ulxr_area = 0;
static int ulxr_methods_area = 0;
static int ulxr_constructor_area = 0;

static MString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
  fprintf(stderr, "In line %i : %s\n", idl_line_no, s );
        exit(1);
  //   theParser->parse_error( idl_lexFile, s, idl_line_no );
}



/* Line 189 of yacc.c  */
#line 140 "yacc.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_UNIMPORTANT = 258,
     T_CHARACTER_LITERAL = 259,
     T_DOUBLE_LITERAL = 260,
     T_IDENTIFIER = 261,
     T_INTEGER_LITERAL = 262,
     T_STRING_LITERAL = 263,
     T_INCLUDE = 264,
     T_CLASS = 265,
     T_STRUCT = 266,
     T_LEFT_CURLY_BRACKET = 267,
     T_LEFT_PARANTHESIS = 268,
     T_RIGHT_CURLY_BRACKET = 269,
     T_RIGHT_PARANTHESIS = 270,
     T_COLON = 271,
     T_SEMICOLON = 272,
     T_PUBLIC = 273,
     T_PROTECTED = 274,
     T_TRIPE_DOT = 275,
     T_PRIVATE = 276,
     T_VIRTUAL = 277,
     T_CONST = 278,
     T_THROW = 279,
     T_INLINE = 280,
     T_FRIEND = 281,
     T_RETURN = 282,
     T_SIGNAL = 283,
     T_SLOT = 284,
     T_TYPEDEF = 285,
     T_PLUS = 286,
     T_MINUS = 287,
     T_COMMA = 288,
     T_ASTERISK = 289,
     T_TILDE = 290,
     T_LESS = 291,
     T_GREATER = 292,
     T_AMPERSAND = 293,
     T_EXTERN = 294,
     T_EXTERN_C = 295,
     T_ACCESS = 296,
     T_ENUM = 297,
     T_NAMESPACE = 298,
     T_USING = 299,
     T_UNKNOWN = 300,
     T_TRIPLE_DOT = 301,
     T_TRUE = 302,
     T_FALSE = 303,
     T_STATIC = 304,
     T_MUTABLE = 305,
     T_EQUAL = 306,
     T_SCOPE = 307,
     T_NULL = 308,
     T_INT = 309,
     T_ARRAY_OPEN = 310,
     T_ARRAY_CLOSE = 311,
     T_CHAR = 312,
     T_ULXRMETHODS_AREA = 313,
     T_ULXRFUNCTION = 314,
     T_ULXRCTOR_AREA = 315,
     T_MOCKMETHODS_AREA = 316,
     T_MOCKCTOR_AREA = 317,
     T_SIGNED = 318,
     T_UNSIGNED = 319,
     T_LONG = 320,
     T_SHORT = 321,
     T_FUNOPERATOR = 322,
     T_MISCOPERATOR = 323,
     T_SHIFT = 324,
     T_DCOP = 325,
     T_DCOP_AREA = 326,
     T_DCOP_SIGNAL_AREA = 327,
     T_QOBJECT = 328
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 70 "yacc.yy"

  long                 _int;
  MString             *_str;
  unsigned short       _char;
  double               _float;



/* Line 214 of yacc.c  */
#line 258 "yacc.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 270 "yacc.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   654

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  205
/* YYNRULES -- Number of states.  */
#define YYNSTATES  428

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   328

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     8,    11,    16,    17,    20,    23,
      26,    29,    31,    33,    34,    41,    49,    53,    57,    63,
      64,    72,    77,    83,    86,    91,    99,   108,   111,   113,
     115,   117,   120,   121,   123,   125,   127,   129,   131,   133,
     135,   139,   142,   145,   148,   152,   155,   159,   161,   165,
     167,   172,   176,   178,   181,   185,   188,   190,   191,   193,
     195,   198,   202,   205,   208,   211,   214,   217,   220,   223,
     226,   229,   232,   235,   241,   246,   251,   256,   263,   268,
     275,   282,   290,   297,   304,   310,   314,   316,   320,   322,
     324,   326,   329,   331,   333,   335,   339,   343,   351,   361,
     362,   367,   368,   370,   372,   375,   377,   380,   383,   387,
     390,   394,   397,   401,   404,   408,   410,   412,   415,   417,
     420,   422,   425,   428,   431,   433,   435,   439,   440,   442,
     446,   448,   450,   453,   456,   461,   468,   472,   474,   477,
     479,   483,   487,   490,   493,   497,   500,   502,   506,   509,
     513,   515,   519,   522,   524,   525,   528,   534,   536,   538,
     540,   542,   547,   548,   550,   552,   554,   556,   558,   560,
     568,   577,   588,   597,   599,   603,   604,   609,   611,   615,
     618,   626,   635,   641,   649,   656,   660,   662,   664,   668,
     673,   676,   678,   679,   681,   684,   685,   687,   691,   694,
     697,   701,   707,   713,   719,   726
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      75,     0,    -1,    76,    82,    75,    -1,    -1,    76,     9,
      -1,    40,    12,    75,    14,    -1,    -1,    62,    16,    -1,
      61,    16,    -1,    71,    16,    -1,    72,    16,    -1,    70,
      -1,    73,    -1,    -1,    10,    92,    96,    81,    98,    17,
      -1,    10,     6,    92,    96,    81,    98,    17,    -1,    10,
      92,    17,    -1,    11,    92,    17,    -1,    11,    92,    96,
      98,    17,    -1,    -1,    43,     6,    12,    83,    75,    14,
      97,    -1,    44,    43,     6,    17,    -1,    44,     6,    52,
       6,    17,    -1,    39,    17,    -1,    30,   114,    92,    17,
      -1,    30,    11,    12,    84,    14,    92,    17,    -1,    30,
      11,    92,    12,    84,    14,    92,    17,    -1,    25,   125,
      -1,   125,    -1,   133,    -1,    99,    -1,   133,    84,    -1,
      -1,    47,    -1,    48,    -1,    21,    -1,    19,    -1,    18,
      -1,    28,    -1,    29,    -1,    86,    87,    16,    -1,    87,
      16,    -1,    86,    16,    -1,    58,    16,    -1,    86,    58,
      16,    -1,    60,    16,    -1,    86,    60,    16,    -1,     6,
      -1,     6,    52,    92,    -1,    92,    -1,    92,    36,   115,
      37,    -1,   119,    18,    93,    -1,    93,    -1,    94,    12,
      -1,    94,    33,    95,    -1,    16,    95,    -1,    12,    -1,
      -1,    17,    -1,    14,    -1,   104,    98,    -1,    25,   125,
      98,    -1,   125,    98,    -1,    90,    98,    -1,    91,    98,
      -1,    99,    98,    -1,    89,    98,    -1,    77,    98,    -1,
      78,    98,    -1,    79,    98,    -1,    88,    98,    -1,    80,
      98,    -1,   133,    98,    -1,    26,    10,    92,    17,    98,
      -1,    26,    92,    17,    98,    -1,    26,   121,    17,    98,
      -1,    10,    92,    17,    98,    -1,    10,    92,    96,    98,
      17,    98,    -1,    11,    92,    17,    98,    -1,    11,    92,
      96,    98,    17,    98,    -1,    44,     6,    52,     6,    17,
      98,    -1,    42,     6,    12,   100,    14,     6,    17,    -1,
      42,     6,    12,   100,    14,    17,    -1,    42,    12,   100,
      14,     6,    17,    -1,    42,    12,   100,    14,    17,    -1,
     101,    33,   100,    -1,   101,    -1,     6,    51,   103,    -1,
       6,    -1,     4,    -1,     7,    -1,    32,     7,    -1,    53,
      -1,    92,    -1,   102,    -1,   102,    31,   102,    -1,   102,
      69,   102,    -1,    30,    92,    36,   115,    37,    92,    17,
      -1,    30,    92,    36,   115,    37,    52,     6,    92,    17,
      -1,    -1,    24,    13,   110,    15,    -1,    -1,    23,    -1,
      63,    -1,    63,    54,    -1,    64,    -1,    64,    54,    -1,
      63,    66,    -1,    63,    66,    54,    -1,    63,    65,    -1,
      63,    65,    54,    -1,    64,    66,    -1,    64,    66,    54,
      -1,    64,    65,    -1,    64,    65,    54,    -1,    54,    -1,
      65,    -1,    65,    54,    -1,    66,    -1,    66,    54,    -1,
      57,    -1,    63,    57,    -1,    64,    57,    -1,    34,   108,
      -1,    34,    -1,   118,    -1,   109,    33,   118,    -1,    -1,
     116,    -1,   110,    33,   116,    -1,   107,    -1,    92,    -1,
      11,    92,    -1,    10,    92,    -1,    92,    36,   112,    37,
      -1,    92,    36,   112,    37,    52,    92,    -1,   113,    33,
     112,    -1,   113,    -1,   111,   108,    -1,   111,    -1,    23,
     111,   108,    -1,    23,   111,    38,    -1,    23,   111,    -1,
     111,    23,    -1,   111,    23,    38,    -1,   111,    38,    -1,
     111,    -1,   111,   108,    23,    -1,   111,   108,    -1,   114,
      33,   115,    -1,   114,    -1,   114,    92,   117,    -1,   114,
     117,    -1,    46,    -1,    -1,    51,   118,    -1,    51,    13,
     114,    15,   118,    -1,     8,    -1,   103,    -1,     5,    -1,
      85,    -1,    92,    13,   109,    15,    -1,    -1,    22,    -1,
      68,    -1,    69,    -1,    37,    -1,    36,    -1,    51,    -1,
     114,    92,    13,   110,    15,   106,   105,    -1,    22,   114,
      92,    13,   110,    15,   106,   105,    -1,    22,   114,    92,
      13,   110,    15,   106,   105,    51,    53,    -1,   114,    67,
     120,    13,   110,    15,   106,   105,    -1,   118,    -1,   118,
      33,   122,    -1,    -1,     6,    13,   122,    15,    -1,   123,
      -1,   123,    33,   124,    -1,   121,   127,    -1,    59,   114,
      92,    13,   110,    15,    17,    -1,    59,    25,   114,    92,
      13,   110,    15,    17,    -1,    92,    13,   110,    15,   127,
      -1,    92,    13,   110,    15,    16,   124,   127,    -1,   119,
      35,    92,    13,    15,   127,    -1,    49,   121,   127,    -1,
      12,    -1,    17,    -1,   126,   128,    14,    -1,   126,   128,
      14,    17,    -1,   129,   128,    -1,    34,    -1,    -1,    17,
      -1,    33,   132,    -1,    -1,     6,    -1,     6,    51,   118,
      -1,   108,     6,    -1,   131,   130,    -1,   114,   132,    17,
      -1,   114,    92,    16,     7,    17,    -1,    49,   114,     6,
     117,    17,    -1,    50,   114,     6,   117,    17,    -1,   114,
       6,    55,   103,    56,    17,    -1,    49,   114,     6,    55,
     103,    56,    17,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   177,   177,   180,   184,   188,   192,   197,   204,   211,
     218,   225,   226,   227,   231,   236,   242,   245,   248,   252,
     251,   263,   266,   269,   272,   275,   278,   281,   284,   287,
     290,   296,   297,   300,   300,   302,   302,   302,   308,   308,
     311,   315,   321,   328,   333,   341,   346,   354,   358,   367,
     373,   382,   386,   393,   397,   404,   408,   416,   418,   422,
     426,   430,   434,   438,   442,   446,   450,   454,   458,   462,
     466,   470,   474,   478,   482,   486,   490,   494,   498,   502,
     506,   513,   514,   515,   516,   520,   521,   525,   526,   530,
     531,   532,   533,   534,   538,   539,   540,   544,   556,   565,
     568,   576,   579,   586,   587,   588,   589,   590,   591,   592,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   609,   613,   620,   623,   631,   634,   635,
     643,   644,   645,   646,   647,   654,   666,   670,   678,   683,
     692,   702,   711,   717,   723,   732,   741,   747,   757,   770,
     774,   781,   793,   804,   814,   816,   819,   826,   829,   832,
     835,   838,   844,   845,   849,   849,   849,   849,   849,   853,
     881,   909,   938,   949,   950,   951,   956,   961,   962,   967,
     971,   986,  1011,  1039,  1066,  1073,  1079,  1086,  1087,  1088,
    1092,  1093,  1094,  1098,  1102,  1103,  1106,  1107,  1108,  1111,
    1115,  1116,  1117,  1118,  1119,  1120
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_UNIMPORTANT", "T_CHARACTER_LITERAL",
  "T_DOUBLE_LITERAL", "T_IDENTIFIER", "T_INTEGER_LITERAL",
  "T_STRING_LITERAL", "T_INCLUDE", "T_CLASS", "T_STRUCT",
  "T_LEFT_CURLY_BRACKET", "T_LEFT_PARANTHESIS", "T_RIGHT_CURLY_BRACKET",
  "T_RIGHT_PARANTHESIS", "T_COLON", "T_SEMICOLON", "T_PUBLIC",
  "T_PROTECTED", "T_TRIPE_DOT", "T_PRIVATE", "T_VIRTUAL", "T_CONST",
  "T_THROW", "T_INLINE", "T_FRIEND", "T_RETURN", "T_SIGNAL", "T_SLOT",
  "T_TYPEDEF", "T_PLUS", "T_MINUS", "T_COMMA", "T_ASTERISK", "T_TILDE",
  "T_LESS", "T_GREATER", "T_AMPERSAND", "T_EXTERN", "T_EXTERN_C",
  "T_ACCESS", "T_ENUM", "T_NAMESPACE", "T_USING", "T_UNKNOWN",
  "T_TRIPLE_DOT", "T_TRUE", "T_FALSE", "T_STATIC", "T_MUTABLE", "T_EQUAL",
  "T_SCOPE", "T_NULL", "T_INT", "T_ARRAY_OPEN", "T_ARRAY_CLOSE", "T_CHAR",
  "T_ULXRMETHODS_AREA", "T_ULXRFUNCTION", "T_ULXRCTOR_AREA",
  "T_MOCKMETHODS_AREA", "T_MOCKCTOR_AREA", "T_SIGNED", "T_UNSIGNED",
  "T_LONG", "T_SHORT", "T_FUNOPERATOR", "T_MISCOPERATOR", "T_SHIFT",
  "T_DCOP", "T_DCOP_AREA", "T_DCOP_SIGNAL_AREA", "T_QOBJECT", "$accept",
  "main", "includes", "mockmethod_area_begin", "mockctor_area_begin",
  "dcop_area_begin", "dcop_signal_area_begin", "platformtag",
  "declaration", "$@1", "member_list", "bool_value", "access_kw",
  "sigslot", "signal_area_begin", "access_kw_begin",
  "ulxr_methods_area_begin", "ulxr_constructor_area_begin", "Identifier",
  "super_class_name", "super_class", "super_classes", "class_header",
  "opt_semicolon", "body", "enum", "enum_list", "enum_item", "number",
  "int_expression", "typedef", "throw_qualifier", "const_qualifier",
  "int_type", "asterisks", "vparams", "params", "type_name",
  "templ_type_list", "templ_type", "type", "type_list", "param", "default",
  "value", "virtual_qualifier", "operator", "function_header", "values",
  "init_item", "init_list", "function", "function_begin", "function_body",
  "function_lines", "function_line", "Identifier_list_rest",
  "Identifier_list_entry", "Identifier_list", "member", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    76,    76,    76,    77,    78,    79,
      80,    81,    81,    81,    82,    82,    82,    82,    82,    83,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    84,    84,    85,    85,    86,    86,    86,    87,    87,
      88,    88,    89,    90,    90,    91,    91,    92,    92,    93,
      93,    94,    94,    95,    95,    96,    96,    97,    97,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    99,    99,    99,    99,   100,   100,   101,   101,   102,
     102,   102,   102,   102,   103,   103,   103,   104,   104,   105,
     105,   106,   106,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   108,   108,   109,   109,   110,   110,   110,
     111,   111,   111,   111,   111,   111,   112,   112,   113,   113,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   115,
     115,   116,   116,   116,   117,   117,   117,   118,   118,   118,
     118,   118,   119,   119,   120,   120,   120,   120,   120,   121,
     121,   121,   121,   122,   122,   122,   123,   124,   124,   125,
     125,   125,   125,   125,   125,   125,   126,   127,   127,   127,
     128,   128,   128,   129,   130,   130,   131,   131,   131,   132,
     133,   133,   133,   133,   133,   133
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     0,     2,     4,     0,     2,     2,     2,
       2,     1,     1,     0,     6,     7,     3,     3,     5,     0,
       7,     4,     5,     2,     4,     7,     8,     2,     1,     1,
       1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     2,     2,     3,     2,     3,     1,     3,     1,
       4,     3,     1,     2,     3,     2,     1,     0,     1,     1,
       2,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     5,     4,     4,     4,     6,     4,     6,
       6,     7,     6,     6,     5,     3,     1,     3,     1,     1,
       1,     2,     1,     1,     1,     3,     3,     7,     9,     0,
       4,     0,     1,     1,     2,     1,     2,     2,     3,     2,
       3,     2,     3,     2,     3,     1,     1,     2,     1,     2,
       1,     2,     2,     2,     1,     1,     3,     0,     1,     3,
       1,     1,     2,     2,     4,     6,     3,     1,     2,     1,
       3,     3,     2,     2,     3,     2,     1,     3,     2,     3,
       1,     3,     2,     1,     0,     2,     5,     1,     1,     1,
       1,     4,     0,     1,     1,     1,     1,     1,     1,     7,
       8,    10,     8,     1,     3,     0,     4,     1,     3,     2,
       7,     8,     5,     7,     6,     3,     1,     1,     3,     4,
       2,     1,     0,     1,     2,     0,     1,     3,     2,     2,
       3,     5,     5,     5,     6,     7
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     0,   162,     6,     1,    47,     4,     0,     0,
     163,     0,   162,     0,     0,     0,     0,     0,     0,     0,
     115,   120,     0,   103,   105,   116,   118,     6,   131,    30,
     130,   146,     0,     0,     0,    28,    29,     0,     0,    47,
     133,   132,     0,     0,   131,     0,   142,     0,     0,    27,
       0,     0,    23,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   104,   121,   109,   107,   106,   122,
     113,   111,   117,   119,     2,   127,     0,   143,   124,   145,
     148,    47,     0,     0,     0,   195,     0,     0,   186,   187,
     192,   179,     5,    48,     0,    56,   162,    16,    13,    17,
     162,   133,   132,     0,   141,   140,     0,    32,   132,     0,
       0,    88,     0,    86,    19,     0,     0,    47,   185,   154,
       0,     0,   110,   108,   114,   112,   153,     0,   154,   128,
     139,     0,   137,   144,   123,   147,     0,     0,   167,   166,
     168,   164,   165,     0,   127,     0,   198,     0,   199,   200,
       0,   193,   191,     0,   192,    13,   163,    49,    52,     0,
      55,     0,    11,    12,   162,     0,     0,    59,    37,    36,
      35,   162,     0,    38,    39,     0,     0,     0,     0,     0,
       0,     0,     0,   162,   162,   162,   162,     0,     0,   162,
     162,   162,   162,     0,   162,   162,   162,   162,   127,     0,
       0,     0,    32,    32,    24,     0,     0,     0,     0,     6,
       0,    21,     0,     0,     0,     0,     0,   127,     0,     0,
     154,   152,   138,   134,     0,    89,   159,    90,   157,     0,
      33,    34,    92,   160,    93,    94,   158,   197,    93,     0,
     127,     0,     0,   196,   194,     0,   188,   190,   162,     0,
      53,   162,     0,     0,   133,   132,   162,     0,   131,     0,
       0,     0,    43,    45,     8,     7,     9,    10,    67,    68,
      69,    71,    42,     0,     0,     0,    41,    70,    66,    63,
      64,    18,    65,    60,    62,    72,     0,     0,     0,     0,
      31,     0,     0,    87,     0,    84,    85,     0,    22,     0,
     155,     0,   202,   203,   127,     0,     0,   182,   129,   151,
       0,   136,    91,     0,     0,     0,     0,     0,   101,   201,
       0,   189,     0,   150,     0,    54,    51,    14,   162,   162,
     162,   162,    61,   133,   162,   162,     0,     0,    44,    46,
      40,   101,   154,     0,     0,     0,    82,    83,    57,     0,
       0,     0,     0,     0,   177,     0,   135,     0,   125,    95,
      96,   204,   101,   102,    99,   184,    15,     0,    50,    76,
       0,    78,     0,   162,    74,    75,     0,     0,    99,    25,
       0,    81,    58,    20,     0,   205,     0,   180,   175,     0,
     183,   161,     0,    99,     0,   169,   149,   162,   162,    73,
       0,   162,   170,    26,   156,   181,   173,     0,   178,   126,
     172,   127,    77,    79,     0,     0,    80,     0,   175,   176,
       0,     0,    97,   171,   174,   100,     0,    98
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   183,   184,   185,   186,   164,    27,   209,
     200,   233,   187,   188,   189,   190,   191,   192,    28,   158,
     159,   160,    98,   383,   193,   194,   112,   113,   235,   236,
     195,   395,   364,    30,    84,   357,   127,    31,   131,   132,
      32,   324,   129,   214,   406,    33,   143,    34,   407,   354,
     355,   196,    90,    91,   153,   154,   148,    85,    86,   197
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -275
static const yytype_int16 yypact[] =
{
      36,    26,    82,   443,     9,  -275,    50,  -275,   105,   127,
     538,   172,   307,   559,   139,   241,   180,    55,   461,   538,
    -275,  -275,   488,   238,   255,   134,   145,    60,    28,  -275,
    -275,   162,    19,   166,    92,  -275,  -275,   220,   127,    13,
     257,   281,   127,   127,   203,   127,   189,   461,    20,  -275,
     242,   127,  -275,   248,   258,   260,   262,   280,   538,    22,
      92,   288,   538,   127,  -275,  -275,   265,   268,  -275,  -275,
     269,   270,  -275,  -275,  -275,   510,   172,   287,   292,  -275,
     308,   108,   138,    31,   326,   301,   318,   127,  -275,  -275,
     211,  -275,  -275,  -275,   234,  -275,   149,  -275,   217,  -275,
     379,  -275,  -275,   331,  -275,  -275,   334,   202,   341,   329,
     258,   303,   343,   327,  -275,   356,   348,    75,  -275,   317,
     127,   361,  -275,  -275,  -275,  -275,  -275,    37,    40,  -275,
     292,   332,   342,  -275,  -275,  -275,   601,    52,  -275,  -275,
    -275,  -275,  -275,   364,   510,   371,  -275,    97,  -275,  -275,
     366,  -275,  -275,   367,   211,   217,  -275,   350,  -275,    83,
    -275,   369,  -275,  -275,   379,   127,   127,  -275,  -275,  -275,
    -275,   307,   524,  -275,  -275,   127,   385,   378,   380,   383,
     390,   395,   396,   379,   379,   379,   379,    94,   398,   379,
     379,   379,   379,   399,   379,   379,   379,   379,   510,   538,
     401,   101,   202,   202,  -275,   403,    52,   136,   258,     9,
     402,  -275,   335,    52,   405,   407,   412,   510,   299,   510,
     317,  -275,  -275,   368,   172,  -275,  -275,  -275,  -275,   411,
    -275,  -275,  -275,  -275,   414,    32,  -275,  -275,  -275,   374,
     510,    81,   415,   384,  -275,   416,   417,  -275,   379,   538,
    -275,   149,   127,   429,   321,   333,   379,   127,    77,   430,
     419,   404,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,   432,   441,   442,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,   147,   453,   127,   444,
    -275,   447,   159,  -275,   445,  -275,  -275,   449,  -275,   538,
    -275,   408,  -275,  -275,   510,   154,   463,  -275,  -275,  -275,
     127,  -275,  -275,   601,    52,    52,   457,   164,   452,  -275,
      92,  -275,   459,   446,   440,  -275,  -275,  -275,   379,   379,
     379,   379,  -275,   464,   379,   379,   538,   472,  -275,  -275,
    -275,   452,   226,   471,   127,   473,  -275,  -275,   474,   465,
     478,   200,   479,   476,   468,    92,  -275,   207,  -275,  -275,
    -275,  -275,   452,  -275,   480,  -275,  -275,   538,  -275,  -275,
     486,  -275,   493,   379,  -275,  -275,   475,   497,   480,  -275,
     500,  -275,  -275,  -275,   601,  -275,   502,  -275,   601,   463,
    -275,  -275,   601,   480,   492,  -275,  -275,   379,   379,  -275,
      23,   379,   477,  -275,  -275,  -275,   489,   508,  -275,  -275,
    -275,   510,  -275,  -275,   523,   514,  -275,   483,   601,  -275,
     209,   127,  -275,  -275,  -275,  -275,   515,  -275
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -275,     5,  -275,  -275,  -275,  -275,  -275,   382,  -275,  -275,
      68,  -275,  -275,   351,  -275,  -275,  -275,  -275,    -8,   289,
    -275,   304,   -37,  -275,   -46,   536,   -93,  -275,   -14,  -116,
    -275,  -201,  -194,  -275,     2,  -275,  -132,    -3,   316,  -275,
      59,  -274,   324,  -101,  -129,   -90,  -275,    -2,   132,  -275,
     168,    10,  -275,   -45,   406,  -275,  -275,  -275,   421,    17
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -197
static const yytype_int16 yytable[] =
{
      40,    41,    44,    44,   100,    44,   161,   237,    46,    37,
      44,    44,   241,    35,    44,   118,    60,   205,   215,     6,
      36,   239,    49,    -3,    83,    81,     6,   221,   117,     6,
      93,    94,    74,    80,   101,   102,    -3,   103,     4,    44,
     106,    75,   108,   109,   144,    60,     6,   145,   105,     1,
      44,   106,   218,    78,    44,   121,   225,   155,     6,   227,
      -3,    56,   376,   314,    76,    38,   286,    44,    44,    45,
     219,    48,    51,   130,    -3,   414,     1,    59,    61,   150,
     134,    63,     5,   300,   229,   305,    82,    82,   157,    82,
     293,   212,  -154,   396,   334,   250,   318,   301,    57,    44,
       1,   315,    38,   243,    88,   232,    48,    81,   317,    89,
     272,    39,   216,    76,   219,   296,   251,    45,   253,   309,
     220,   120,   173,   174,   202,  -196,   212,    38,   234,   238,
     213,    78,   222,     6,   128,    78,    44,   268,   269,   270,
     271,  -196,   294,   277,   278,   279,   280,   378,   282,   283,
     284,   285,   273,   295,   274,     6,    52,   254,   255,   136,
      38,   161,   341,   137,   258,   345,   201,   260,   393,   352,
     259,   156,   351,   307,   138,   139,   346,   402,     6,   362,
     219,   256,    42,    43,   358,    77,    55,   219,    72,   140,
      44,    44,   410,   289,    44,    44,    78,   219,   238,    73,
      79,    87,   322,   128,   234,   238,   141,   142,     6,    44,
     332,    44,    42,    43,   297,   386,    44,   329,   331,   202,
     202,   130,   391,    78,   425,    11,    20,   104,   151,    21,
      48,    48,    44,   219,    92,    23,    24,    25,    26,    76,
     392,    44,   219,   157,   157,   152,    95,    53,     6,   333,
      96,   199,    19,    54,   107,   404,    20,   128,   287,    21,
     110,   201,   201,   409,   111,    23,    24,    25,    26,    95,
     290,   291,   114,    96,    97,   365,   128,   212,   128,   420,
     343,   213,   369,   370,   371,   372,   116,   162,   374,   375,
     163,    44,    64,    95,   119,    65,    44,    96,    99,   128,
     359,   360,   356,    66,    67,   234,   238,   238,   323,    68,
     390,    88,    69,     6,   115,   306,    89,    42,    43,   122,
      70,    71,   123,   124,   125,   133,    78,   399,    44,    10,
      11,   135,   146,    95,   147,   149,   380,    96,   328,   225,
     226,     6,   227,   228,   198,    95,   204,   144,   299,    96,
     330,   412,   413,   203,   206,   416,    47,   207,   349,    44,
     208,    20,   210,   128,    21,   211,    22,   229,   212,   223,
      23,    24,    25,    26,   217,   224,   234,   240,   242,   245,
     234,   246,   230,   231,   234,     6,   249,   252,   232,   165,
     166,   261,   415,   167,   262,   323,   263,   168,   169,   264,
     170,    10,    11,    44,   171,   172,   265,   173,   174,   175,
     234,   266,   267,   426,   276,   288,   281,   292,   312,   298,
     310,    15,   302,   176,   303,   304,   323,   313,    18,    19,
     316,   320,   319,    20,   321,   136,    21,   177,    22,   178,
     179,   180,    23,    24,    25,    26,   327,   335,   338,     6,
     181,   182,     7,     8,     9,   336,   337,   339,   340,   342,
     145,   344,   347,   348,   350,    10,    11,     6,    12,   353,
     128,    42,    43,    13,   361,   363,   366,   368,   377,   367,
     384,   373,    14,    58,    11,    15,    16,    17,   379,   388,
     381,   382,    18,    19,     6,   385,   387,    20,    42,    43,
      21,   389,    22,   397,   394,   411,    23,    24,    25,    26,
     398,    11,   400,    62,   401,    20,     6,   403,    21,   405,
      42,    43,   418,   419,    23,    24,    25,    26,   417,   421,
       6,   422,   427,    11,   257,    43,   423,   248,   275,    29,
     311,   326,    20,   308,     6,    21,    58,    11,    42,    43,
     424,    23,    24,    25,    26,   325,   126,   408,     0,     0,
     247,    11,     0,     0,    20,     6,     0,    21,   244,    42,
      50,     0,     0,    23,    24,    25,    26,     0,    20,     0,
       0,    21,    11,     0,     0,     0,     0,    23,    24,    25,
      26,     0,    20,     0,     0,    21,     0,     0,     0,     0,
       0,    23,    24,    25,    26,   225,   226,     6,   227,   228,
       0,     0,     0,    20,     0,     0,    21,     0,     0,     0,
       0,     0,    23,    24,    25,    26,     0,     0,     0,     0,
       0,     0,     0,   229,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   230,   231,
       0,     0,     0,     0,   232
};

static const yytype_int16 yycheck[] =
{
       8,     9,    10,    11,    41,    13,    96,   136,    11,     4,
      18,    19,   144,     3,    22,    60,    18,   110,   119,     6,
       3,   137,    12,    14,    32,     6,     6,   128,     6,     6,
      38,    39,    27,    31,    42,    43,     0,    45,    12,    47,
      48,    13,    50,    51,    13,    47,     6,    16,    46,    40,
      58,    59,    15,    34,    62,    63,     4,    94,     6,     7,
       0,     6,   336,    31,    36,    52,   198,    75,    76,    10,
      33,    12,    13,    76,    14,    52,    40,    18,    19,    87,
      78,    22,     0,   212,    32,   217,    67,    67,    96,    67,
     206,    51,    17,   367,    17,    12,    15,   213,    43,   107,
      40,    69,    52,     6,    12,    53,    47,     6,   240,    17,
      16,     6,   120,    36,    33,   208,    33,    58,   164,   220,
     128,    62,    28,    29,   107,    17,    51,    52,   136,   137,
      55,    34,   130,     6,    75,    34,   144,   183,   184,   185,
     186,    33,     6,   189,   190,   191,   192,   341,   194,   195,
     196,   197,    58,    17,    60,     6,    17,   165,   166,    51,
      52,   251,    15,    55,   172,     6,   107,   175,   362,    15,
     172,    22,   304,   218,    36,    37,    17,   378,     6,    15,
      33,   171,    10,    11,   313,    23,     6,    33,    54,    51,
     198,   199,   393,   201,   202,   203,    34,    33,   206,    54,
      38,    35,   248,   144,   212,   213,    68,    69,     6,   217,
     256,   219,    10,    11,   209,    15,   224,   254,   255,   202,
     203,   224,    15,    34,    15,    23,    54,    38,    17,    57,
     171,   172,   240,    33,    14,    63,    64,    65,    66,    36,
      33,   249,    33,   251,   252,    34,    12,     6,     6,   257,
      16,    49,    50,    12,    12,   384,    54,   198,   199,    57,
      12,   202,   203,   392,     6,    63,    64,    65,    66,    12,
     202,   203,    12,    16,    17,   320,   217,    51,   219,   411,
     288,    55,   328,   329,   330,   331,     6,    70,   334,   335,
      73,   299,    54,    12,     6,    57,   304,    16,    17,   240,
     314,   315,   310,    65,    66,   313,   314,   315,   249,    54,
     355,    12,    57,     6,    52,    16,    17,    10,    11,    54,
      65,    66,    54,    54,    54,    38,    34,   373,   336,    22,
      23,    23,     6,    12,    33,    17,   344,    16,    17,     4,
       5,     6,     7,     8,    13,    12,    17,    13,    13,    16,
      17,   397,   398,    12,    51,   401,    49,    14,   299,   367,
      33,    54,     6,   304,    57,    17,    59,    32,    51,    37,
      63,    64,    65,    66,    13,    33,   384,    13,     7,    13,
     388,    14,    47,    48,   392,     6,    36,    18,    53,    10,
      11,     6,   400,    14,    16,   336,    16,    18,    19,    16,
      21,    22,    23,   411,    25,    26,    16,    28,    29,    30,
     418,    16,    16,   421,    16,    14,    17,    14,     7,    17,
      52,    42,    17,    44,    17,    13,   367,    13,    49,    50,
      56,    15,    17,    54,    17,    51,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    17,    17,    16,     6,
      71,    72,     9,    10,    11,    36,    52,    16,    16,     6,
      16,    14,    17,    14,    56,    22,    23,     6,    25,     6,
     411,    10,    11,    30,    17,    23,    17,    37,     6,    33,
      15,    17,    39,    22,    23,    42,    43,    44,    17,    13,
      17,    17,    49,    50,     6,    17,    17,    54,    10,    11,
      57,    33,    59,    17,    24,    13,    63,    64,    65,    66,
      17,    23,    37,    25,    17,    54,     6,    17,    57,    17,
      10,    11,    33,    15,    63,    64,    65,    66,    51,     6,
       6,    17,    17,    23,    10,    11,    53,   155,   187,     3,
     224,   252,    54,   219,     6,    57,    22,    23,    10,    11,
     418,    63,    64,    65,    66,   251,    46,   389,    -1,    -1,
     154,    23,    -1,    -1,    54,     6,    -1,    57,   147,    10,
      11,    -1,    -1,    63,    64,    65,    66,    -1,    54,    -1,
      -1,    57,    23,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    -1,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    63,    64,    65,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    -1,    -1,    53
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    40,    75,    76,    12,     0,     6,     9,    10,    11,
      22,    23,    25,    30,    39,    42,    43,    44,    49,    50,
      54,    57,    59,    63,    64,    65,    66,    82,    92,    99,
     107,   111,   114,   119,   121,   125,   133,    75,    52,     6,
      92,    92,    10,    11,    92,   114,   111,    49,   114,   125,
      11,   114,    17,     6,    12,     6,     6,    43,    22,   114,
     121,   114,    25,   114,    54,    57,    65,    66,    54,    57,
      65,    66,    54,    54,    75,    13,    36,    23,    34,    38,
     108,     6,    67,    92,   108,   131,   132,    35,    12,    17,
     126,   127,    14,    92,    92,    12,    16,    17,    96,    17,
      96,    92,    92,    92,    38,   108,    92,    12,    92,    92,
      12,     6,   100,   101,    12,    52,     6,     6,   127,     6,
     114,    92,    54,    54,    54,    54,    46,   110,   114,   116,
     111,   112,   113,    38,   108,    23,    51,    55,    36,    37,
      51,    68,    69,   120,    13,    16,     6,    33,   130,    17,
      92,    17,    34,   128,   129,    96,    22,    92,    93,    94,
      95,   119,    70,    73,    81,    10,    11,    14,    18,    19,
      21,    25,    26,    28,    29,    30,    44,    58,    60,    61,
      62,    71,    72,    77,    78,    79,    80,    86,    87,    88,
      89,    90,    91,    98,    99,   104,   125,   133,    13,    49,
      84,   114,   133,    12,    17,   100,    51,    14,    33,    83,
       6,    17,    51,    55,   117,   117,    92,    13,    15,    33,
      92,   117,   108,    37,    33,     4,     5,     7,     8,    32,
      47,    48,    53,    85,    92,   102,   103,   118,    92,   103,
      13,   110,     7,     6,   132,    13,    14,   128,    81,    36,
      12,    33,    18,    98,    92,    92,   125,    10,    92,   121,
      92,     6,    16,    16,    16,    16,    16,    16,    98,    98,
      98,    98,    16,    58,    60,    87,    16,    98,    98,    98,
      98,    17,    98,    98,    98,    98,   110,   114,    14,    92,
      84,    84,    14,   103,     6,    17,   100,    75,    17,    13,
     118,   103,    17,    17,    13,   110,    16,   127,   116,   117,
      52,   112,     7,    13,    31,    69,    56,   110,    15,    17,
      15,    17,    98,   114,   115,    95,    93,    17,    17,    96,
      17,    96,    98,    92,    17,    17,    36,    52,    16,    16,
      16,    15,     6,    92,    14,     6,    17,    17,    14,   114,
      56,   110,    15,     6,   123,   124,    92,   109,   118,   102,
     102,    17,    15,    23,   106,   127,    17,    33,    37,    98,
      98,    98,    98,    17,    98,    98,   115,     6,   106,    17,
      92,    17,    17,    97,    15,    17,    15,    17,    13,    33,
     127,    15,    33,   106,    24,   105,   115,    17,    17,    98,
      37,    17,   105,    17,   118,    17,   118,   122,   124,   118,
     105,    13,    98,    98,    52,    92,    98,    51,    33,    15,
     110,     6,    17,    53,   122,    15,    92,    17
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 178 "yacc.yy"
    {
    ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 185 "yacc.yy"
    {
      printf("<INCLUDE>%s</INCLUDE>\n", (yyvsp[(2) - (2)]._str)->latin1() );
    ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 189 "yacc.yy"
    {
    ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 192 "yacc.yy"
    {
    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 198 "yacc.yy"
    {
    /*empty*/
  ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 205 "yacc.yy"
    {
    /*empty*/
  ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 212 "yacc.yy"
    {
    /*empty*/
  ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 219 "yacc.yy"
    {
    /*empty*/
  ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 225 "yacc.yy"
    { (yyval._int) = 1; ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 226 "yacc.yy"
    { (yyval._int) = 2; ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 227 "yacc.yy"
    { (yyval._int) = 0; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 232 "yacc.yy"
    {
      printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", MString( in_namespace + *(yyvsp[(2) - (6)]._str) ).latin1(), (yyvsp[(3) - (6)]._str)->latin1(), (yyvsp[(5) - (6)]._str)->latin1() );
      ulxr_area = 0;
    ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 237 "yacc.yy"
    {
      printf("<CLASS>\n    <NAME>%s</NAME>\n    <LINK_SCOPE>%s</LINK_SCOPE>\n%s%s</CLASS>\n", MString( in_namespace + *(yyvsp[(3) - (7)]._str) ).latin1(), (yyvsp[(2) - (7)]._str)->latin1(),  (yyvsp[(4) - (7)]._str)->latin1(), (yyvsp[(6) - (7)]._str)->latin1() );
      ulxr_area = 0;

    ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 243 "yacc.yy"
    {
    ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 246 "yacc.yy"
    {
    ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 249 "yacc.yy"
    {
    ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 252 "yacc.yy"
    {
          in_namespace += *(yyvsp[(2) - (3)]._str); in_namespace += "::";
      ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 256 "yacc.yy"
    {
            std::size_t pos = in_namespace.findRev( "::", -3 );
            if( pos != MString::npos )
                in_namespace = in_namespace.left( pos + 2 );
            else
                in_namespace = "";
        ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 264 "yacc.yy"
    {
    ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 267 "yacc.yy"
    {
    ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 270 "yacc.yy"
    {
    ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 273 "yacc.yy"
    {
    ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 276 "yacc.yy"
    {
    ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 279 "yacc.yy"
    {
    ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 282 "yacc.yy"
    {
    ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 285 "yacc.yy"
    {
    ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 288 "yacc.yy"
    {
    ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 291 "yacc.yy"
    {
    ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 303 "yacc.yy"
    {
    ulxr_area = 0;
  ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 312 "yacc.yy"
    {
  ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 316 "yacc.yy"
    {
  ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 322 "yacc.yy"
    {
    ulxr_area = 0;
  ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 329 "yacc.yy"
    {
    ulxr_methods_area = 1;
    ulxr_area = 1;
  ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 334 "yacc.yy"
    {
    ulxr_methods_area = 1;
    ulxr_area = 1;
  ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 342 "yacc.yy"
    {
    ulxr_constructor_area = 1;
    ulxr_area = 1;
  ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 347 "yacc.yy"
    {
    ulxr_constructor_area = 1;
    ulxr_area = 1;
  ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 355 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(1) - (1)]._str);
    ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 359 "yacc.yy"
    {
      MString* tmp = new MString( "%1::%2" );
            *tmp = tmp->arg(*((yyvsp[(1) - (3)]._str))).arg(*((yyvsp[(3) - (3)]._str)));
            (yyval._str) = tmp;
    ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 368 "yacc.yy"
    {
      MString* tmp = new MString( "    <SUPER>%1</SUPER>\n" );
      *tmp = tmp->arg( *((yyvsp[(1) - (1)]._str)) );
      (yyval._str) = tmp;
    ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 374 "yacc.yy"
    {
      MString* tmp = new MString( "    <SUPER>%1</SUPER>\n" );
      *tmp = tmp->arg( *((yyvsp[(1) - (4)]._str)) + "&lt;" + *((yyvsp[(3) - (4)]._str)) + "&gt;" );
      (yyval._str) = tmp;
    ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 383 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(3) - (3)]._str);
    ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 387 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(1) - (1)]._str);
    ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 394 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(1) - (2)]._str);
    ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 398 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(1) - (3)]._str)) + *((yyvsp[(3) - (3)]._str)) );
    ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 405 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 409 "yacc.yy"
    {
      (yyval._str) = new MString( "" );
    ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 416 "yacc.yy"
    {
    ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 423 "yacc.yy"
    {
      (yyval._str) = new MString( "" );
    ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 427 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(1) - (2)]._str)) + *((yyvsp[(2) - (2)]._str)) );
    ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 431 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(2) - (3)]._str)) + *((yyvsp[(3) - (3)]._str)) );
    ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 435 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(1) - (2)]._str)) + *((yyvsp[(2) - (2)]._str)) );
    ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 439 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 443 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 447 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 451 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 455 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 459 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 463 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 467 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 471 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 475 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (2)]._str);
    ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 479 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(5) - (5)]._str);
    ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 483 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(4) - (4)]._str);
    ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 487 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(4) - (4)]._str);
    ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 491 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(4) - (4)]._str);
    ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 495 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(6) - (6)]._str);
    ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 499 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(4) - (4)]._str);
    ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 503 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(6) - (6)]._str);
    ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 507 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(6) - (6)]._str);
    ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 525 "yacc.yy"
    {;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 526 "yacc.yy"
    {;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 530 "yacc.yy"
    {;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 531 "yacc.yy"
    {;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 532 "yacc.yy"
    {;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 533 "yacc.yy"
    {;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 534 "yacc.yy"
    {;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 538 "yacc.yy"
    {;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 539 "yacc.yy"
    {;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 540 "yacc.yy"
    {;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 545 "yacc.yy"
    {
      if (ulxr_area)
      {
        MString* tmp = new MString("<TYPEDEF name=\"%1\" template=\"%2\">%3</TYPEDEF>\n");
        *tmp = tmp->arg( *((yyvsp[(6) - (7)]._str)) ).arg( *((yyvsp[(2) - (7)]._str)) ).arg( *((yyvsp[(4) - (7)]._str)) );
        (yyval._str) = tmp;
      } else
      {
        (yyval._str) = new MString("");
      }
    ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 557 "yacc.yy"
    {
      if (ulxr_area)
        yyerror("scoped template typedefs are not supported in ulxr methods!");
    ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 565 "yacc.yy"
    {
        (yyval._str) = new MString("");
    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 569 "yacc.yy"
    {
        (yyval._str) = new MString("/* todo */");
    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 576 "yacc.yy"
    {
      (yyval._int) = 0;
    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 580 "yacc.yy"
    {
      (yyval._int) = 1;
    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 586 "yacc.yy"
    { (yyval._str) = new MString("signed int"); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 587 "yacc.yy"
    { (yyval._str) = new MString("signed int"); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 588 "yacc.yy"
    { (yyval._str) = new MString("unsigned int"); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 589 "yacc.yy"
    { (yyval._str) = new MString("unsigned int"); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 590 "yacc.yy"
    { (yyval._str) = new MString("signed short int"); ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 591 "yacc.yy"
    { (yyval._str) = new MString("signed short int"); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 592 "yacc.yy"
    { (yyval._str) = new MString("signed long int"); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 593 "yacc.yy"
    { (yyval._str) = new MString("signed long int"); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 594 "yacc.yy"
    { (yyval._str) = new MString("unsigned short int"); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 595 "yacc.yy"
    { (yyval._str) = new MString("unsigned short int"); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 596 "yacc.yy"
    { (yyval._str) = new MString("unsigned long int"); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 597 "yacc.yy"
    { (yyval._str) = new MString("unsigned long int"); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 598 "yacc.yy"
    { (yyval._str) = new MString("int"); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 599 "yacc.yy"
    { (yyval._str) = new MString("long int"); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 600 "yacc.yy"
    { (yyval._str) = new MString("long int"); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 601 "yacc.yy"
    { (yyval._str) = new MString("short int"); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 602 "yacc.yy"
    { (yyval._str) = new MString("short int"); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 603 "yacc.yy"
    { (yyval._str) = new MString("char"); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 604 "yacc.yy"
    { (yyval._str) = new MString("signed char"); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 605 "yacc.yy"
    { (yyval._str) = new MString("unsigned char"); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 610 "yacc.yy"
    {
      (yyval._str) = new MString( MString("*") + *((yyvsp[(2) - (2)]._str)) );
    ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 614 "yacc.yy"
    {
      (yyval._str) = new MString("*");
    ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 621 "yacc.yy"
    {
    ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 624 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(1) - (3)]._str)) );
    ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 631 "yacc.yy"
    {
      (yyval._str) = new MString( "" );
    ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 636 "yacc.yy"
    {
      (yyval._str) = new MString( *((yyvsp[(1) - (3)]._str)) + *((yyvsp[(3) - (3)]._str)) );
    ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 644 "yacc.yy"
    { (yyval._str) = (yyvsp[(1) - (1)]._str); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 645 "yacc.yy"
    { (yyval._str) = (yyvsp[(2) - (2)]._str); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 646 "yacc.yy"
    { (yyval._str) = (yyvsp[(2) - (2)]._str); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 648 "yacc.yy"
    {
      MString *tmp = new MString("%1&lt;%2&gt;");
      *tmp = tmp->arg(*((yyvsp[(1) - (4)]._str)));
      *tmp = tmp->arg(*((yyvsp[(3) - (4)]._str)));
      (yyval._str) = tmp;
    ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 655 "yacc.yy"
    {
      MString *tmp = new MString("%1&lt;%2&gt;::%3");
      *tmp = tmp->arg(*((yyvsp[(1) - (6)]._str)));
      *tmp = tmp->arg(*((yyvsp[(3) - (6)]._str)));
      *tmp = tmp->arg(*((yyvsp[(6) - (6)]._str)));
      (yyval._str) = tmp;
    ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 667 "yacc.yy"
    {
      (yyval._str) = new MString(*((yyvsp[(1) - (3)]._str)) + "," + *((yyvsp[(3) - (3)]._str)));
    ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 671 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(1) - (1)]._str);
    ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 679 "yacc.yy"
    {
      if (ulxr_area)
        yyerror("in ulxr methods are no pointers allowed");
    ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 684 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(1) - (1)]._str);
    ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 693 "yacc.yy"
    {
//         if (ulxr_area)
         {
            MString* tmp = new MString("<TYPE  qleft=\"const\" qright=\"%2\">%1</TYPE>");
            *tmp = tmp->arg( *((yyvsp[(2) - (3)]._str)) );
            *tmp = tmp->arg( *((yyvsp[(3) - (3)]._str)) );
            (yyval._str) = tmp;
         }
      ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 703 "yacc.yy"
    {
//         if (ulxr_area)
         {
            MString* tmp = new MString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
            *tmp = tmp->arg( *((yyvsp[(2) - (3)]._str)) );
            (yyval._str) = tmp;
         }
      ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 712 "yacc.yy"
    {
        MString* tmp = new MString("<TYPE qleft=\"const\" >%1</TYPE>");
        *tmp = tmp->arg( *((yyvsp[(2) - (2)]._str)) );
        (yyval._str) = tmp;
      ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 718 "yacc.yy"
    {
        MString* tmp = new MString("<TYPE qleft=\"const\">%1</TYPE>");
        *tmp = tmp->arg( *((yyvsp[(1) - (2)]._str)) );
        (yyval._str) = tmp;
      ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 724 "yacc.yy"
    {
//        if (ulxr_area)
        {
           MString* tmp = new MString("<TYPE qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
           *tmp = tmp->arg( *((yyvsp[(1) - (3)]._str)) );
           (yyval._str) = tmp;
        }
      ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 733 "yacc.yy"
    {
//        if (ulxr_area)
        {
           MString* tmp = new MString("<TYPE qright=\"" AMP_ENTITY "\">%1</TYPE>");
           *tmp = tmp->arg( *((yyvsp[(1) - (2)]._str)) );
           (yyval._str) = tmp;
        }
      ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 742 "yacc.yy"
    {
        MString* tmp = new MString("<TYPE>%1</TYPE>");
        *tmp = tmp->arg( *((yyvsp[(1) - (1)]._str)) );
        (yyval._str) = tmp;
      ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 748 "yacc.yy"
    {
//        if (ulxr_area)
        {
           MString* tmp = new MString("<TYPE qright=\"%2 const\">%1</TYPE>");
           *tmp = tmp->arg( *((yyvsp[(1) - (3)]._str)) );
           *tmp = tmp->arg( *((yyvsp[(2) - (3)]._str)) );
           (yyval._str) = tmp;
        }
      ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 758 "yacc.yy"
    {
//        if (ulxr_area)
        {
           MString* tmp = new MString("<TYPE qright=\"%2\">%1</TYPE>");
           *tmp = tmp->arg( *((yyvsp[(1) - (2)]._str)) );
           *tmp = tmp->arg( *((yyvsp[(2) - (2)]._str)) );
           (yyval._str) = tmp;
        }
      ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 771 "yacc.yy"
    {
      (yyval._str) = new MString(*((yyvsp[(1) - (3)]._str)) + "," + *((yyvsp[(3) - (3)]._str)));
    ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 775 "yacc.yy"
    {
       (yyval._str) = (yyvsp[(1) - (1)]._str);
    ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 782 "yacc.yy"
    {
//    if (ulxr_area)
      {
        MString* tmp = new MString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
        *tmp = tmp->arg( *((yyvsp[(1) - (3)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(2) - (3)]._str)) );
        (yyval._str) = tmp;
      }
//    else
//      $$ = new MString();
    ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 794 "yacc.yy"
    {
//    if (ulxr_area)
      {
         MString* tmp = new MString("\n        <ARG>%1</ARG>");
         *tmp = tmp->arg( *((yyvsp[(1) - (2)]._str)) );
         (yyval._str) = tmp;
      }
//    else
//      $$ = new MString();
    ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 805 "yacc.yy"
    {
      if (ulxr_area)
        yyerror("variable arguments not supported in ulxr methods.");
      (yyval._str) = new MString("");
    ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 814 "yacc.yy"
    {
    ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 817 "yacc.yy"
    {
    ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 820 "yacc.yy"
    {
    ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 827 "yacc.yy"
    {
    ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 830 "yacc.yy"
    {
    ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 833 "yacc.yy"
    {
    ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 836 "yacc.yy"
    {
    ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 839 "yacc.yy"
    {
    ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 844 "yacc.yy"
    { (yyval._int) = 0;  ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 845 "yacc.yy"
    { (yyval._int) = 1; ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 854 "yacc.yy"
    {
      if (ulxr_area)
      {
        MString* tmp = 0;
        tmp = new MString(
                "    <%4>\n"
                "        %2\n"
                "        <NAME>%1</NAME>"
                "%3\n"
                "    </%5>\n");
        *tmp = tmp->arg( *((yyvsp[(2) - (7)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(1) - (7)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(4) - (7)]._str)) );

        MString tagname = "METHOD";

        MString attr = ((yyvsp[(6) - (7)]._int)) ? " qual=\"const\"" : "";
        MString thrower (*((yyvsp[(7) - (7)]._str)));
        if (thrower.length() != 0)
          attr += MString(" throw='%1'").arg(thrower);
        *tmp = tmp->arg( MString("%1%2").arg(tagname).arg(attr) );
        *tmp = tmp->arg( MString("%1").arg(tagname) );
        (yyval._str) = tmp;
      }
      else
        (yyval._str) = new MString("");
    ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 882 "yacc.yy"
    {
      if (ulxr_area)
      {
        MString* tmp = 0;
        tmp = new MString(
                "    <%4>\n"
                "        %2\n"
                "        <NAME>%1</NAME>"
                "%3\n"
                "    </%5>\n");
        *tmp = tmp->arg( *((yyvsp[(3) - (8)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(2) - (8)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(5) - (8)]._str)) );

        MString tagname = "METHOD";
        MString attr = ((yyvsp[(7) - (8)]._int)) ? " qual=\"const\"" : "";
        MString thrower (*((yyvsp[(8) - (8)]._str)));
        if (thrower.length() != 0)
          attr += MString(" throw='%1'").arg(thrower);
        attr += " virtual='1'";
        *tmp = tmp->arg( MString("%1%2").arg(tagname).arg(attr) );
        *tmp = tmp->arg( MString("%1").arg(tagname) );
        (yyval._str) = tmp;
      }
      else
        (yyval._str) = new MString("");
    ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 910 "yacc.yy"
    {
      if (ulxr_area)
      {
        MString* tmp = 0;
        tmp = new MString(
                "    <%4>\n"
                "        %2\n"
                "        <NAME>%1</NAME>"
                "%3\n"
                "    </%5>\n");
        *tmp = tmp->arg( *((yyvsp[(3) - (10)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(2) - (10)]._str)) );
        *tmp = tmp->arg( *((yyvsp[(5) - (10)]._str)) );

        MString tagname = "METHOD";

        MString attr = ((yyvsp[(7) - (10)]._int)) ? " qual=\"const\"" : "";
        attr += " virtual='1' pure='1'";
        MString thrower (*((yyvsp[(8) - (10)]._str)));
        if (thrower.length() != 0)
          attr += MString(" throw='%1'").arg(thrower);
        *tmp = tmp->arg( MString("%1%2").arg(tagname).arg(attr) );
        *tmp = tmp->arg( MString("%1").arg(tagname) );
        (yyval._str) = tmp;
      }
      else
       (yyval._str) = new MString("");
    ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 939 "yacc.yy"
    {
      if (ulxr_area)
        yyerror("operators aren't allowed in ulxr methods!");
       (yyval._str) = new MString("");
    ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 949 "yacc.yy"
    {;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 950 "yacc.yy"
    {;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 951 "yacc.yy"
    {;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 956 "yacc.yy"
    {;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 961 "yacc.yy"
    {;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 962 "yacc.yy"
    {;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 968 "yacc.yy"
    {
          (yyval._str) = (yyvsp[(1) - (2)]._str);
    ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 972 "yacc.yy"
    {
      MString tmp;
      tmp = MString(
              "<FUNCTION>\n"
              "        %2\n"
              "        <NAME>%1</NAME>"
              "%3\n"
              "</FUNCTION>\n");
      tmp = tmp.arg( in_namespace + *((yyvsp[(3) - (7)]._str)) );
      tmp = tmp.arg( *((yyvsp[(2) - (7)]._str)) );
      tmp = tmp.arg( *((yyvsp[(5) - (7)]._str)) );

      printf ("%s", tmp.c_str());
    ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 987 "yacc.yy"
    {
      MString tmp;
      tmp = MString(
              "<FUNCTION>\n"
              "        %2\n"
              "        <NAME>%1</NAME>"
              "%3\n"
              "</FUNCTION>\n");
      tmp = tmp.arg( in_namespace + *((yyvsp[(4) - (8)]._str)) );
      tmp = tmp.arg( *((yyvsp[(3) - (8)]._str)) );
      tmp = tmp.arg( *((yyvsp[(6) - (8)]._str)) );

      printf ("%s", tmp.c_str());
    ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1012 "yacc.yy"
    {
      /* The constructor */
//      assert(!ulxr_methods_area);
      if (ulxr_constructor_area)
      {
        MString* tmp = 0;
        tmp = new MString(
                "    <%4>"
                "%2\n"
                "        <NAME>%1</NAME>\n"
                "    </%3>\n");
        *tmp = tmp->arg( *((yyvsp[(1) - (5)]._str)) );
        if (((yyvsp[(3) - (5)]._str))->length() != 0)
          *tmp = tmp->arg( *((yyvsp[(3) - (5)]._str)) );
        else
          *tmp = tmp->arg("");

        MString tagname = "CONSTRUCTOR";
        ulxr_area = 0;
        ulxr_constructor_area = 0;  // only once

        *tmp = tmp->arg(tagname).arg(tagname);
        (yyval._str) = tmp;
      }
      else
        (yyval._str) = new MString("");
    ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1040 "yacc.yy"
    {
      if (ulxr_constructor_area)
      {
        MString* tmp = 0;
        tmp = new MString(
                "    <%4>"
                "%2\n"
                "        <NAME>%1</NAME>\n"
                "    </%3>\n");
        *tmp = tmp->arg( *((yyvsp[(1) - (7)]._str)) );
        if (((yyvsp[(3) - (7)]._str))->length() != 0)
          *tmp = tmp->arg( *((yyvsp[(3) - (7)]._str)));
        else
          *tmp = tmp->arg("");


        MString tagname = "CONSTRUCTOR";
        ulxr_area = 0;
        ulxr_constructor_area = 0;  // only once

        *tmp = tmp->arg(tagname).arg(tagname);
        (yyval._str) = tmp;
      }
      else
        (yyval._str) = new MString("");
    ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1067 "yacc.yy"
    {
        /* The destructor */
//        assert(!ulxr_methods_area);
//        assert(!ulxr_constructor_area);
        (yyval._str) = new MString("");
    ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1074 "yacc.yy"
    {
      (yyval._str) = (yyvsp[(2) - (3)]._str);
    ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1080 "yacc.yy"
    {
    function_mode = 1;
  ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1092 "yacc.yy"
    {;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1094 "yacc.yy"
    {;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1106 "yacc.yy"
    {;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1107 "yacc.yy"
    {;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1108 "yacc.yy"
    {;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1111 "yacc.yy"
    {;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1115 "yacc.yy"
    {;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1116 "yacc.yy"
    {;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1117 "yacc.yy"
    {;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1118 "yacc.yy"
    {;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1119 "yacc.yy"
    {;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1120 "yacc.yy"
    {;}
    break;



/* Line 1455 of yacc.c  */
#line 3571 "yacc.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1123 "yacc.yy"


void dcopidlParse( const char *_code )
{
    ulxr_area = 0;
    ulxr_methods_area = 0;
    ulxr_constructor_area = 0;
    dcopidlInitFlex( _code );
    yyparse();
}

