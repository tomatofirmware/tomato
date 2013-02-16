/* A Bison parser, made by GNU Bison 2.6.2.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.6.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 336 of yacc.c  */
#line 34 "parse.y"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <net/pfkeyv2.h>
#include PATH_IPSEC_H
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "libpfkey.h"
#include "vchar.h"
#include "extern.h"

#define DEFAULT_NATT_PORT	4500

#ifndef UDP_ENCAP_ESPINUDP
#define UDP_ENCAP_ESPINUDP	2
#endif

#define ATOX(c) \
  (isdigit((int)c) ? (c - '0') : \
    (isupper((int)c) ? (c - 'A' + 10) : (c - 'a' + 10)))

u_int32_t p_spi;
u_int p_ext, p_alg_enc, p_alg_auth, p_replay, p_mode;
u_int32_t p_reqid;
u_int p_key_enc_len, p_key_auth_len;
const char *p_key_enc;
const char *p_key_auth;
time_t p_lt_hard, p_lt_soft;
size_t p_lb_hard, p_lb_soft;

struct security_ctx {
	u_int8_t doi;
	u_int8_t alg;
	u_int16_t len;
	char *buf;
};

struct security_ctx sec_ctx;

static u_int p_natt_type;
static struct addrinfo * p_natt_oa = NULL;

static int p_aiflags = 0, p_aifamily = PF_UNSPEC;

static struct addrinfo *parse_addr __P((char *, char *));
static int fix_portstr __P((int, vchar_t *, vchar_t *, vchar_t *));
static int setvarbuf __P((char *, int *, struct sadb_ext *, int, 
    const void *, int));
void parse_init __P((void));
void free_buffer __P((void));

int setkeymsg0 __P((struct sadb_msg *, unsigned int, unsigned int, size_t));
static int setkeymsg_spdaddr __P((unsigned int, unsigned int, vchar_t *,
	struct addrinfo *, int, struct addrinfo *, int));
static int setkeymsg_spdaddr_tag __P((unsigned int, char *, vchar_t *));
static int setkeymsg_addr __P((unsigned int, unsigned int,
	struct addrinfo *, struct addrinfo *, int));
static int setkeymsg_add __P((unsigned int, unsigned int,
	struct addrinfo *, struct addrinfo *));

/* Line 336 of yacc.c  */
#line 144 "parse.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_Y_TAB_H
# define YY_Y_TAB_H
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     EOT = 258,
     SLASH = 259,
     BLCL = 260,
     ELCL = 261,
     ADD = 262,
     GET = 263,
     DELETE = 264,
     DELETEALL = 265,
     FLUSH = 266,
     DUMP = 267,
     EXIT = 268,
     PR_ESP = 269,
     PR_AH = 270,
     PR_IPCOMP = 271,
     PR_ESPUDP = 272,
     PR_TCP = 273,
     F_PROTOCOL = 274,
     F_AUTH = 275,
     F_ENC = 276,
     F_REPLAY = 277,
     F_COMP = 278,
     F_RAWCPI = 279,
     F_MODE = 280,
     MODE = 281,
     F_REQID = 282,
     F_EXT = 283,
     EXTENSION = 284,
     NOCYCLICSEQ = 285,
     ALG_AUTH = 286,
     ALG_AUTH_NOKEY = 287,
     ALG_ENC = 288,
     ALG_ENC_NOKEY = 289,
     ALG_ENC_DESDERIV = 290,
     ALG_ENC_DES32IV = 291,
     ALG_ENC_OLD = 292,
     ALG_COMP = 293,
     F_LIFETIME_HARD = 294,
     F_LIFETIME_SOFT = 295,
     F_LIFEBYTE_HARD = 296,
     F_LIFEBYTE_SOFT = 297,
     DECSTRING = 298,
     QUOTEDSTRING = 299,
     HEXSTRING = 300,
     STRING = 301,
     ANY = 302,
     SPDADD = 303,
     SPDUPDATE = 304,
     SPDDELETE = 305,
     SPDDUMP = 306,
     SPDFLUSH = 307,
     F_POLICY = 308,
     PL_REQUESTS = 309,
     F_AIFLAGS = 310,
     TAGGED = 311,
     SECURITY_CTX = 312
   };
#endif
/* Tokens.  */
#define EOT 258
#define SLASH 259
#define BLCL 260
#define ELCL 261
#define ADD 262
#define GET 263
#define DELETE 264
#define DELETEALL 265
#define FLUSH 266
#define DUMP 267
#define EXIT 268
#define PR_ESP 269
#define PR_AH 270
#define PR_IPCOMP 271
#define PR_ESPUDP 272
#define PR_TCP 273
#define F_PROTOCOL 274
#define F_AUTH 275
#define F_ENC 276
#define F_REPLAY 277
#define F_COMP 278
#define F_RAWCPI 279
#define F_MODE 280
#define MODE 281
#define F_REQID 282
#define F_EXT 283
#define EXTENSION 284
#define NOCYCLICSEQ 285
#define ALG_AUTH 286
#define ALG_AUTH_NOKEY 287
#define ALG_ENC 288
#define ALG_ENC_NOKEY 289
#define ALG_ENC_DESDERIV 290
#define ALG_ENC_DES32IV 291
#define ALG_ENC_OLD 292
#define ALG_COMP 293
#define F_LIFETIME_HARD 294
#define F_LIFETIME_SOFT 295
#define F_LIFEBYTE_HARD 296
#define F_LIFEBYTE_SOFT 297
#define DECSTRING 298
#define QUOTEDSTRING 299
#define HEXSTRING 300
#define STRING 301
#define ANY 302
#define SPDADD 303
#define SPDUPDATE 304
#define SPDDELETE 305
#define SPDDUMP 306
#define SPDFLUSH 307
#define F_POLICY 308
#define PL_REQUESTS 309
#define F_AIFLAGS 310
#define TAGGED 311
#define SECURITY_CTX 312



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 350 of yacc.c  */
#line 110 "parse.y"

	int num;
	unsigned long ulnum;
	vchar_t val;
	struct addrinfo *res;


/* Line 350 of yacc.c  */
#line 309 "parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

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

#endif /* !YY_Y_TAB_H  */

/* Copy the second part of user declarations.  */

/* Line 353 of yacc.c  */
#line 337 "parse.c"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   174

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  90
/* YYNRULES -- Number of states.  */
#define YYNSTATES  188

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

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
      55,    56,    57
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    41,    50,    57,
      66,    70,    74,    75,    77,    79,    81,    83,    86,    88,
      90,    92,    94,    96,    98,   103,   106,   109,   112,   116,
     118,   121,   123,   126,   129,   132,   134,   136,   138,   139,
     142,   145,   148,   151,   154,   157,   160,   163,   166,   169,
     172,   177,   191,   197,   211,   217,   231,   234,   237,   238,
     241,   243,   245,   247,   250,   251,   254,   255,   259,   263,
     267,   269,   271,   273,   275,   276,   278,   279,   284,   287,
     289
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      59,     0,    -1,    -1,    59,    60,    -1,    61,    -1,    64,
      -1,    62,    -1,    63,    -1,    65,    -1,    66,    -1,    94,
      -1,    78,    -1,    79,    -1,    80,    -1,    81,    -1,    82,
      -1,     7,    83,    86,    86,    67,    68,    76,    69,     3,
      -1,     9,    83,    86,    86,    67,    68,    76,     3,    -1,
      10,    83,    85,    85,    67,     3,    -1,     8,    83,    86,
      86,    67,    68,    76,     3,    -1,    11,    67,     3,    -1,
      12,    67,     3,    -1,    -1,    14,    -1,    15,    -1,    16,
      -1,    17,    -1,    17,    85,    -1,    18,    -1,    43,    -1,
      45,    -1,    70,    -1,    71,    -1,    72,    -1,    21,    73,
      20,    74,    -1,    21,    73,    -1,    20,    74,    -1,    23,
      38,    -1,    23,    38,    24,    -1,    34,    -1,    33,    75,
      -1,    37,    -1,    35,    75,    -1,    36,    75,    -1,    31,
      75,    -1,    32,    -1,    44,    -1,    45,    -1,    -1,    76,
      77,    -1,    28,    29,    -1,    28,    30,    -1,    25,    26,
      -1,    25,    47,    -1,    27,    43,    -1,    22,    43,    -1,
      39,    43,    -1,    40,    43,    -1,    41,    43,    -1,    42,
      43,    -1,    57,    43,    43,    44,    -1,    48,    83,    46,
      87,    88,    46,    87,    88,    89,    90,    91,    92,     3,
      -1,    48,    56,    44,    92,     3,    -1,    49,    83,    46,
      87,    88,    46,    87,    88,    89,    90,    91,    92,     3,
      -1,    49,    56,    44,    92,     3,    -1,    50,    83,    46,
      87,    88,    46,    87,    88,    89,    90,    91,    92,     3,
      -1,    51,     3,    -1,    52,     3,    -1,    -1,    83,    84,
      -1,    55,    -1,    46,    -1,    46,    -1,    46,    88,    -1,
      -1,     4,    43,    -1,    -1,     5,    47,     6,    -1,     5,
      43,     6,    -1,     5,    46,     6,    -1,    43,    -1,    47,
      -1,    18,    -1,    46,    -1,    -1,    46,    -1,    -1,    57,
      43,    43,    44,    -1,    53,    93,    -1,    54,    -1,    13,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   151,   151,   153,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   178,   190,   209,   237,
     252,   262,   272,   275,   283,   291,   295,   302,   309,   318,
     319,   340,   341,   342,   346,   347,   351,   355,   363,   375,
     390,   405,   421,   442,   466,   491,   504,   508,   537,   539,
     543,   544,   545,   546,   547,   548,   557,   558,   559,   560,
     561,   573,   606,   619,   652,   664,   696,   707,   716,   718,
     722,   747,   758,   766,   777,   778,   783,   791,   800,   811,
     818,   819,   820,   823,   846,   850,   861,   863,   872,   896,
     901
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EOT", "SLASH", "BLCL", "ELCL", "ADD",
  "GET", "DELETE", "DELETEALL", "FLUSH", "DUMP", "EXIT", "PR_ESP", "PR_AH",
  "PR_IPCOMP", "PR_ESPUDP", "PR_TCP", "F_PROTOCOL", "F_AUTH", "F_ENC",
  "F_REPLAY", "F_COMP", "F_RAWCPI", "F_MODE", "MODE", "F_REQID", "F_EXT",
  "EXTENSION", "NOCYCLICSEQ", "ALG_AUTH", "ALG_AUTH_NOKEY", "ALG_ENC",
  "ALG_ENC_NOKEY", "ALG_ENC_DESDERIV", "ALG_ENC_DES32IV", "ALG_ENC_OLD",
  "ALG_COMP", "F_LIFETIME_HARD", "F_LIFETIME_SOFT", "F_LIFEBYTE_HARD",
  "F_LIFEBYTE_SOFT", "DECSTRING", "QUOTEDSTRING", "HEXSTRING", "STRING",
  "ANY", "SPDADD", "SPDUPDATE", "SPDDELETE", "SPDDUMP", "SPDFLUSH",
  "F_POLICY", "PL_REQUESTS", "F_AIFLAGS", "TAGGED", "SECURITY_CTX",
  "$accept", "commands", "command", "add_command", "delete_command",
  "deleteall_command", "get_command", "flush_command", "dump_command",
  "protocol_spec", "spi", "algorithm_spec", "esp_spec", "ah_spec",
  "ipcomp_spec", "enc_alg", "auth_alg", "key_string", "extension_spec",
  "extension", "spdadd_command", "spdupdate_command", "spddelete_command",
  "spddump_command", "spdflush_command", "ipaddropts", "ipaddropt",
  "ipaddr", "ipandport", "prefix", "portstr", "upper_spec",
  "upper_misc_spec", "context_spec", "policy_spec", "policy_requests",
  "exit_command", YY_NULL
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
     305,   306,   307,   308,   309,   310,   311,   312
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    61,    62,    63,    64,
      65,    66,    67,    67,    67,    67,    67,    67,    67,    68,
      68,    69,    69,    69,    70,    70,    71,    72,    72,    73,
      73,    73,    73,    73,    74,    74,    75,    75,    76,    76,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    78,    78,    79,    79,    80,    81,    82,    83,    83,
      84,    85,    86,    86,    87,    87,    88,    88,    88,    88,
      89,    89,    89,    89,    90,    90,    91,    91,    92,    93,
      94
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     9,     8,     6,     8,
       3,     3,     0,     1,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     4,     2,     2,     2,     3,     1,
       2,     1,     2,     2,     2,     1,     1,     1,     0,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,    13,     5,    13,     5,    13,     2,     2,     0,     2,
       1,     1,     1,     2,     0,     2,     0,     3,     3,     3,
       1,     1,     1,     1,     0,     1,     0,     4,     2,     1,
       2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,    68,    68,    68,    68,    22,    22,     0,
      68,    68,    68,     0,     0,     3,     4,     6,     7,     5,
       8,     9,    11,    12,    13,    14,    15,    10,     0,     0,
       0,     0,    23,    24,    25,    26,    28,     0,     0,    90,
       0,     0,     0,     0,     0,    66,    67,    72,    70,    69,
       0,     0,     0,    71,     0,    27,    20,    21,     0,    74,
       0,    74,    74,     0,    73,    22,    22,    22,    22,     0,
       0,     0,    76,     0,    76,    76,     0,     0,     0,     0,
       0,     0,     0,    89,    88,    62,    75,     0,    64,     0,
       0,    78,    79,    77,    29,    30,    48,    48,    48,    18,
      74,    74,    74,     0,     0,     0,    76,    76,    76,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    31,    32,    33,    49,    19,    17,     0,     0,
       0,     0,    45,    36,     0,    39,     0,     0,    41,    35,
      55,    37,    52,    53,    54,    50,    51,    56,    57,    58,
      59,     0,    16,    82,    80,    83,    81,    84,    84,    84,
      46,    47,    44,    40,    42,    43,     0,    38,     0,    85,
      86,    86,    86,    34,    60,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    63,    65,    87
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    15,    16,    17,    18,    19,    20,    21,    37,
      96,   121,   122,   123,   124,   139,   133,   162,   103,   125,
      22,    23,    24,    25,    26,    28,    49,    54,    50,    72,
      64,   157,   170,   176,    70,    84,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -61
static const yytype_int16 yypact[] =
{
     -61,     3,   -61,   -61,   -61,   -61,   -61,    96,    96,    17,
     -12,     1,   -61,    78,   106,   -61,   -61,   -61,   -61,   -61,
     -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -38,   -38,
     -38,    29,   -61,   -61,   -61,    69,   -61,   122,   130,   -61,
      92,    31,    93,    39,    49,   -61,   -61,   133,   -61,   -61,
      94,    94,    94,   -61,    69,   -61,   -61,   -61,    88,   135,
      88,   135,   135,   -24,   -61,    96,    96,    96,    96,    89,
     139,   101,   133,   142,   133,   133,   140,   141,   143,    81,
      81,    81,   145,   -61,   -61,   -61,   -61,   104,   -61,   105,
     107,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,
     135,   135,   135,    51,    -1,     6,   133,   133,   133,   -26,
      86,   109,   116,   -22,   112,    13,   113,   114,   115,   117,
     118,   156,   -61,   -61,   -61,   -61,   -61,   -61,   -11,   -11,
     -11,     5,   -61,   -61,     5,   -61,     5,     5,   -61,   144,
     -61,   138,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,
     -61,   120,   -61,   -61,   -61,   -61,   -61,   119,   119,   119,
     -61,   -61,   -61,   -61,   -61,   -61,   -26,   -61,   123,   -61,
     111,   111,   111,   -61,   -61,   126,    88,    88,    88,   127,
     163,   168,   169,   129,   -61,   -61,   -61,   -61
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,    -7,
      22,   -61,   -61,   -61,   -61,   -61,     8,   -54,    30,   -61,
     -61,   -61,   -61,   -61,   -61,    95,   -61,   -17,    37,   -32,
     -10,     0,   -27,   -37,   -60,   -61,   -61
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      73,    38,   126,     2,   142,   131,   132,   153,    47,   127,
       3,     4,     5,     6,     7,     8,     9,    48,    55,    76,
      39,   111,    77,    78,   113,   143,   114,   115,   111,    74,
      75,   113,   154,   114,   115,   155,   156,    68,   116,   117,
     118,   119,   145,   146,    40,   116,   117,   118,   119,   160,
     161,    10,    11,    12,    13,    14,   120,    42,    79,    80,
      81,    82,    87,   120,    89,    90,    51,    52,   106,   107,
     108,   109,   110,   111,   112,    53,   113,    59,   114,   115,
     163,    45,   164,   165,    48,    61,    48,    65,    66,    67,
     116,   117,   118,   119,    48,    62,   128,   129,   130,    29,
      30,    31,    97,    98,    48,    41,    43,    44,   120,    46,
      32,    33,    34,    35,    36,    53,   180,   181,   182,   134,
     135,   136,   137,   138,    94,    56,    95,   104,   105,   158,
     159,   171,   172,    57,   177,   178,    58,    60,    63,    71,
      47,    69,    85,    83,    86,    88,    91,    92,    99,    93,
     100,   101,   140,   102,   141,   144,   147,   148,   149,   152,
     150,   151,   167,   168,   166,   169,   184,   174,   175,   179,
     183,   185,   186,   187,   173
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-61))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_uint8 yycheck[] =
{
      60,     8,     3,     0,    26,    31,    32,    18,    46,     3,
       7,     8,     9,    10,    11,    12,    13,    55,    35,    43,
       3,    22,    46,    47,    25,    47,    27,    28,    22,    61,
      62,    25,    43,    27,    28,    46,    47,    54,    39,    40,
      41,    42,    29,    30,    56,    39,    40,    41,    42,    44,
      45,    48,    49,    50,    51,    52,    57,    56,    65,    66,
      67,    68,    72,    57,    74,    75,    29,    30,   100,   101,
     102,    20,    21,    22,    23,    46,    25,    46,    27,    28,
     134,     3,   136,   137,    55,    46,    55,    50,    51,    52,
      39,    40,    41,    42,    55,    46,   106,   107,   108,     4,
       5,     6,    80,    81,    55,    10,    11,    12,    57,     3,
      14,    15,    16,    17,    18,    46,   176,   177,   178,    33,
      34,    35,    36,    37,    43,     3,    45,    97,    98,   129,
     130,   158,   159,     3,   171,   172,    44,    44,     5,     4,
      46,    53,     3,    54,    43,     3,     6,     6,     3,     6,
      46,    46,    43,    46,    38,    43,    43,    43,    43,     3,
      43,    43,    24,    43,    20,    46,     3,    44,    57,    43,
      43,     3,     3,    44,   166
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    59,     0,     7,     8,     9,    10,    11,    12,    13,
      48,    49,    50,    51,    52,    60,    61,    62,    63,    64,
      65,    66,    78,    79,    80,    81,    82,    94,    83,    83,
      83,    83,    14,    15,    16,    17,    18,    67,    67,     3,
      56,    83,    56,    83,    83,     3,     3,    46,    55,    84,
      86,    86,    86,    46,    85,    85,     3,     3,    44,    46,
      44,    46,    46,     5,    88,    86,    86,    86,    85,    53,
      92,     4,    87,    92,    87,    87,    43,    46,    47,    67,
      67,    67,    67,    54,    93,     3,    43,    88,     3,    88,
      88,     6,     6,     6,    43,    45,    68,    68,    68,     3,
      46,    46,    46,    76,    76,    76,    87,    87,    87,    20,
      21,    22,    23,    25,    27,    28,    39,    40,    41,    42,
      57,    69,    70,    71,    72,    77,     3,     3,    88,    88,
      88,    31,    32,    74,    33,    34,    35,    36,    37,    73,
      43,    38,    26,    47,    43,    29,    30,    43,    43,    43,
      43,    43,     3,    18,    43,    46,    47,    89,    89,    89,
      44,    45,    75,    75,    75,    75,    20,    24,    43,    46,
      90,    90,    90,    74,    44,    57,    91,    91,    91,    43,
      92,    92,    92,    43,     3,     3,     3,    44
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                     \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])



/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

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

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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
        case 3:
/* Line 1787 of yacc.c  */
#line 154 "parse.y"
    {
			free_buffer();
			parse_init();
		}
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 179 "parse.y"
    {
			int status;

			status = setkeymsg_add(SADB_ADD, (yyvsp[(5) - (9)].num), (yyvsp[(3) - (9)].res), (yyvsp[(4) - (9)].res));
			if (status < 0)
				return -1;
		}
    break;

  case 17:
/* Line 1787 of yacc.c  */
#line 191 "parse.y"
    {
			int status;

			if ((yyvsp[(3) - (8)].res)->ai_next || (yyvsp[(4) - (8)].res)->ai_next) {
				yyerror("multiple address specified");
				return -1;
			}
			if (p_mode != IPSEC_MODE_ANY)
				yyerror("WARNING: mode is obsolete");

			status = setkeymsg_addr(SADB_DELETE, (yyvsp[(5) - (8)].num), (yyvsp[(3) - (8)].res), (yyvsp[(4) - (8)].res), 0);
			if (status < 0)
				return -1;
		}
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 210 "parse.y"
    {
#ifndef __linux__
			if (setkeymsg_addr(SADB_DELETE, (yyvsp[(5) - (6)].num), (yyvsp[(3) - (6)].res), (yyvsp[(4) - (6)].res), 1) < 0)
				return -1;
#else /* __linux__ */
			/* linux strictly adheres to RFC2367, and returns
			 * an error if we send an SADB_DELETE request without
			 * an SPI. Therefore, we must first retrieve a list
			 * of SPIs for all matching SADB entries, and then
			 * delete each one separately. */
			u_int32_t *spi;
			int i, n;

			spi = sendkeymsg_spigrep((yyvsp[(5) - (6)].num), (yyvsp[(3) - (6)].res), (yyvsp[(4) - (6)].res), &n);
			for (i = 0; i < n; i++) {
				p_spi = spi[i];
				if (setkeymsg_addr(SADB_DELETE,
							(yyvsp[(5) - (6)].num), (yyvsp[(3) - (6)].res), (yyvsp[(4) - (6)].res), 0) < 0)
					return -1;
			}
			free(spi);
#endif /* __linux__ */
		}
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 238 "parse.y"
    {
			int status;

			if (p_mode != IPSEC_MODE_ANY)
				yyerror("WARNING: mode is obsolete");

			status = setkeymsg_addr(SADB_GET, (yyvsp[(5) - (8)].num), (yyvsp[(3) - (8)].res), (yyvsp[(4) - (8)].res), 0);
			if (status < 0)
				return -1;
		}
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 253 "parse.y"
    {
			struct sadb_msg msg;
			setkeymsg0(&msg, SADB_FLUSH, (yyvsp[(2) - (3)].num), sizeof(msg));
			sendkeymsg((char *)&msg, sizeof(msg));
		}
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 263 "parse.y"
    {
			struct sadb_msg msg;
			setkeymsg0(&msg, SADB_DUMP, (yyvsp[(2) - (3)].num), sizeof(msg));
			sendkeymsg((char *)&msg, sizeof(msg));
		}
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 272 "parse.y"
    {
			(yyval.num) = SADB_SATYPE_UNSPEC;
		}
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 276 "parse.y"
    {
			(yyval.num) = SADB_SATYPE_ESP;
			if ((yyvsp[(1) - (1)].num) == 1)
				p_ext |= SADB_X_EXT_OLD;
			else
				p_ext &= ~SADB_X_EXT_OLD;
		}
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 284 "parse.y"
    {
			(yyval.num) = SADB_SATYPE_AH;
			if ((yyvsp[(1) - (1)].num) == 1)
				p_ext |= SADB_X_EXT_OLD;
			else
				p_ext &= ~SADB_X_EXT_OLD;
		}
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 292 "parse.y"
    {
			(yyval.num) = SADB_X_SATYPE_IPCOMP;
		}
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 296 "parse.y"
    {
			(yyval.num) = SADB_SATYPE_ESP;
			p_ext &= ~SADB_X_EXT_OLD;
			p_natt_oa = 0;
			p_natt_type = UDP_ENCAP_ESPINUDP;
		}
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 303 "parse.y"
    {
			(yyval.num) = SADB_SATYPE_ESP;
			p_ext &= ~SADB_X_EXT_OLD;
			p_natt_oa = (yyvsp[(2) - (2)].res);
			p_natt_type = UDP_ENCAP_ESPINUDP;
		}
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 310 "parse.y"
    {
#ifdef SADB_X_SATYPE_TCPSIGNATURE
			(yyval.num) = SADB_X_SATYPE_TCPSIGNATURE;
#endif
		}
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 318 "parse.y"
    { p_spi = (yyvsp[(1) - (1)].ulnum); }
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 320 "parse.y"
    {
			char *ep;
			unsigned long v;

			ep = NULL;
			v = strtoul((yyvsp[(1) - (1)].val).buf, &ep, 16);
			if (!ep || *ep) {
				yyerror("invalid SPI");
				return -1;
			}
			if (v & ~0xffffffff) {
				yyerror("SPI too big.");
				return -1;
			}

			p_spi = v;
		}
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 356 "parse.y"
    {
			if ((yyvsp[(2) - (2)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(2) - (2)].num);
		}
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 364 "parse.y"
    {
			if ((yyvsp[(2) - (3)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(2) - (3)].num);
			p_ext |= SADB_X_EXT_RAWCPI;
		}
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 375 "parse.y"
    {
			if ((yyvsp[(1) - (1)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(1) - (1)].num);

			p_key_enc_len = 0;
			p_key_enc = "";
			if (ipsec_check_keylen(SADB_EXT_SUPPORTED_ENCRYPT,
			    p_alg_enc, PFKEY_UNUNIT64(p_key_enc_len)) < 0) {
				yyerror(ipsec_strerror());
				return -1;
			}
		}
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 390 "parse.y"
    {
			if ((yyvsp[(1) - (2)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(1) - (2)].num);

			p_key_enc_len = (yyvsp[(2) - (2)].val).len;
			p_key_enc = (yyvsp[(2) - (2)].val).buf;
			if (ipsec_check_keylen(SADB_EXT_SUPPORTED_ENCRYPT,
			    p_alg_enc, PFKEY_UNUNIT64(p_key_enc_len)) < 0) {
				yyerror(ipsec_strerror());
				return -1;
			}
		}
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 405 "parse.y"
    {
			if ((yyvsp[(1) - (1)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			yyerror("WARNING: obsolete algorithm");
			p_alg_enc = (yyvsp[(1) - (1)].num);

			p_key_enc_len = 0;
			p_key_enc = "";
			if (ipsec_check_keylen(SADB_EXT_SUPPORTED_ENCRYPT,
			    p_alg_enc, PFKEY_UNUNIT64(p_key_enc_len)) < 0) {
				yyerror(ipsec_strerror());
				return -1;
			}
		}
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 422 "parse.y"
    {
			if ((yyvsp[(1) - (2)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(1) - (2)].num);
			if (p_ext & SADB_X_EXT_OLD) {
				yyerror("algorithm mismatched");
				return -1;
			}
			p_ext |= SADB_X_EXT_DERIV;

			p_key_enc_len = (yyvsp[(2) - (2)].val).len;
			p_key_enc = (yyvsp[(2) - (2)].val).buf;
			if (ipsec_check_keylen(SADB_EXT_SUPPORTED_ENCRYPT,
			    p_alg_enc, PFKEY_UNUNIT64(p_key_enc_len)) < 0) {
				yyerror(ipsec_strerror());
				return -1;
			}
		}
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 443 "parse.y"
    {
			if ((yyvsp[(1) - (2)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_enc = (yyvsp[(1) - (2)].num);
			if (!(p_ext & SADB_X_EXT_OLD)) {
				yyerror("algorithm mismatched");
				return -1;
			}
			p_ext |= SADB_X_EXT_IV4B;

			p_key_enc_len = (yyvsp[(2) - (2)].val).len;
			p_key_enc = (yyvsp[(2) - (2)].val).buf;
			if (ipsec_check_keylen(SADB_EXT_SUPPORTED_ENCRYPT,
			    p_alg_enc, PFKEY_UNUNIT64(p_key_enc_len)) < 0) {
				yyerror(ipsec_strerror());
				return -1;
			}
		}
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 466 "parse.y"
    {
			if ((yyvsp[(1) - (2)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_auth = (yyvsp[(1) - (2)].num);

			p_key_auth_len = (yyvsp[(2) - (2)].val).len;
			p_key_auth = (yyvsp[(2) - (2)].val).buf;
#ifdef SADB_X_AALG_TCP_MD5
			if (p_alg_auth == SADB_X_AALG_TCP_MD5) {
				if ((p_key_auth_len < 1) || 
				    (p_key_auth_len > 80))
					return -1;
			} else 
#endif
			{
				if (ipsec_check_keylen(SADB_EXT_SUPPORTED_AUTH,
				    p_alg_auth, 
				    PFKEY_UNUNIT64(p_key_auth_len)) < 0) {
					yyerror(ipsec_strerror());
					return -1;
				}
			}
		}
    break;

  case 45:
/* Line 1787 of yacc.c  */
#line 491 "parse.y"
    {
			if ((yyvsp[(1) - (1)].num) < 0) {
				yyerror("unsupported algorithm");
				return -1;
			}
			p_alg_auth = (yyvsp[(1) - (1)].num);

			p_key_auth_len = 0;
			p_key_auth = NULL;
		}
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 505 "parse.y"
    {
			(yyval.val) = (yyvsp[(1) - (1)].val);
		}
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 509 "parse.y"
    {
			caddr_t pp_key;
			caddr_t bp;
			caddr_t yp = (yyvsp[(1) - (1)].val).buf;
			int l;

			l = strlen(yp) % 2 + strlen(yp) / 2;
			if ((pp_key = malloc(l)) == 0) {
				yyerror("not enough core");
				return -1;
			}
			memset(pp_key, 0, l);

			bp = pp_key;
			if (strlen(yp) % 2) {
				*bp = ATOX(yp[0]);
				yp++, bp++;
			}
			while (*yp) {
				*bp = (ATOX(yp[0]) << 4) | ATOX(yp[1]);
				yp += 2, bp++;
			}

			(yyval.val).len = l;
			(yyval.val).buf = pp_key;
		}
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 543 "parse.y"
    { p_ext |= (yyvsp[(2) - (2)].num); }
    break;

  case 51:
/* Line 1787 of yacc.c  */
#line 544 "parse.y"
    { p_ext &= ~SADB_X_EXT_CYCSEQ; }
    break;

  case 52:
/* Line 1787 of yacc.c  */
#line 545 "parse.y"
    { p_mode = (yyvsp[(2) - (2)].num); }
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 546 "parse.y"
    { p_mode = IPSEC_MODE_ANY; }
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 547 "parse.y"
    { p_reqid = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 549 "parse.y"
    {
			if ((p_ext & SADB_X_EXT_OLD) != 0) {
				yyerror("replay prevention cannot be used with "
				    "ah/esp-old");
				return -1;
			}
			p_replay = (yyvsp[(2) - (2)].ulnum);
		}
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 557 "parse.y"
    { p_lt_hard = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 558 "parse.y"
    { p_lt_soft = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 559 "parse.y"
    { p_lb_hard = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 560 "parse.y"
    { p_lb_soft = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 561 "parse.y"
    {
		sec_ctx.doi = (yyvsp[(2) - (4)].ulnum);
		sec_ctx.alg = (yyvsp[(3) - (4)].ulnum);
		sec_ctx.len = (yyvsp[(4) - (4)].val).len+1;
		sec_ctx.buf = (yyvsp[(4) - (4)].val).buf;
	}
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 574 "parse.y"
    {
			int status;
			struct addrinfo *src, *dst;

#ifdef HAVE_PFKEY_POLICY_PRIORITY
			last_msg_type = SADB_X_SPDADD;
#endif

			/* fixed port fields if ulp is icmp */
			if (fix_portstr((yyvsp[(9) - (13)].num), &(yyvsp[(10) - (13)].val), &(yyvsp[(5) - (13)].val), &(yyvsp[(8) - (13)].val)))
				return -1;

			src = parse_addr((yyvsp[(3) - (13)].val).buf, (yyvsp[(5) - (13)].val).buf);
			dst = parse_addr((yyvsp[(6) - (13)].val).buf, (yyvsp[(8) - (13)].val).buf);
			if (!src || !dst) {
				/* yyerror is already called */
				return -1;
			}
			if (src->ai_next || dst->ai_next) {
				yyerror("multiple address specified");
				freeaddrinfo(src);
				freeaddrinfo(dst);
				return -1;
			}

			status = setkeymsg_spdaddr(SADB_X_SPDADD, (yyvsp[(9) - (13)].num), &(yyvsp[(12) - (13)].val),
			    src, (yyvsp[(4) - (13)].num), dst, (yyvsp[(7) - (13)].num));
			freeaddrinfo(src);
			freeaddrinfo(dst);
			if (status < 0)
				return -1;
		}
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 607 "parse.y"
    {
			int status;

			status = setkeymsg_spdaddr_tag(SADB_X_SPDADD,
			    (yyvsp[(3) - (5)].val).buf, &(yyvsp[(4) - (5)].val));
			if (status < 0)
				return -1;
		}
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 620 "parse.y"
    {
			int status;
			struct addrinfo *src, *dst;

#ifdef HAVE_PFKEY_POLICY_PRIORITY
			last_msg_type = SADB_X_SPDUPDATE;
#endif

			/* fixed port fields if ulp is icmp */
			if (fix_portstr((yyvsp[(9) - (13)].num), &(yyvsp[(10) - (13)].val), &(yyvsp[(5) - (13)].val), &(yyvsp[(8) - (13)].val)))
				return -1;

			src = parse_addr((yyvsp[(3) - (13)].val).buf, (yyvsp[(5) - (13)].val).buf);
			dst = parse_addr((yyvsp[(6) - (13)].val).buf, (yyvsp[(8) - (13)].val).buf);
			if (!src || !dst) {
				/* yyerror is already called */
				return -1;
			}
			if (src->ai_next || dst->ai_next) {
				yyerror("multiple address specified");
				freeaddrinfo(src);
				freeaddrinfo(dst);
				return -1;
			}

			status = setkeymsg_spdaddr(SADB_X_SPDUPDATE, (yyvsp[(9) - (13)].num), &(yyvsp[(12) - (13)].val),
			    src, (yyvsp[(4) - (13)].num), dst, (yyvsp[(7) - (13)].num));
			freeaddrinfo(src);
			freeaddrinfo(dst);
			if (status < 0)
				return -1;
		}
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 653 "parse.y"
    {
			int status;

			status = setkeymsg_spdaddr_tag(SADB_X_SPDUPDATE,
			    (yyvsp[(3) - (5)].val).buf, &(yyvsp[(4) - (5)].val));
			if (status < 0)
				return -1;
		}
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 665 "parse.y"
    {
			int status;
			struct addrinfo *src, *dst;

			/* fixed port fields if ulp is icmp */
			if (fix_portstr((yyvsp[(9) - (13)].num), &(yyvsp[(10) - (13)].val), &(yyvsp[(5) - (13)].val), &(yyvsp[(8) - (13)].val)))
				return -1;

			src = parse_addr((yyvsp[(3) - (13)].val).buf, (yyvsp[(5) - (13)].val).buf);
			dst = parse_addr((yyvsp[(6) - (13)].val).buf, (yyvsp[(8) - (13)].val).buf);
			if (!src || !dst) {
				/* yyerror is already called */
				return -1;
			}
			if (src->ai_next || dst->ai_next) {
				yyerror("multiple address specified");
				freeaddrinfo(src);
				freeaddrinfo(dst);
				return -1;
			}

			status = setkeymsg_spdaddr(SADB_X_SPDDELETE, (yyvsp[(9) - (13)].num), &(yyvsp[(12) - (13)].val),
			    src, (yyvsp[(4) - (13)].num), dst, (yyvsp[(7) - (13)].num));
			freeaddrinfo(src);
			freeaddrinfo(dst);
			if (status < 0)
				return -1;
		}
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 697 "parse.y"
    {
			struct sadb_msg msg;
			setkeymsg0(&msg, SADB_X_SPDDUMP, SADB_SATYPE_UNSPEC,
			    sizeof(msg));
			sendkeymsg((char *)&msg, sizeof(msg));
		}
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 708 "parse.y"
    {
			struct sadb_msg msg;
			setkeymsg0(&msg, SADB_X_SPDFLUSH, SADB_SATYPE_UNSPEC,
			    sizeof(msg));
			sendkeymsg((char *)&msg, sizeof(msg));
		}
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 723 "parse.y"
    {
			char *p;

			for (p = (yyvsp[(1) - (1)].val).buf + 1; *p; p++)
				switch (*p) {
				case '4':
					p_aifamily = AF_INET;
					break;
#ifdef INET6
				case '6':
					p_aifamily = AF_INET6;
					break;
#endif
				case 'n':
					p_aiflags = AI_NUMERICHOST;
					break;
				default:
					yyerror("invalid flag");
					return -1;
				}
		}
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 748 "parse.y"
    {
			(yyval.res) = parse_addr((yyvsp[(1) - (1)].val).buf, NULL);
			if ((yyval.res) == NULL) {
				/* yyerror already called by parse_addr */
				return -1;
			}
		}
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 759 "parse.y"
    {
			(yyval.res) = parse_addr((yyvsp[(1) - (1)].val).buf, NULL);
			if ((yyval.res) == NULL) {
				/* yyerror already called by parse_addr */
				return -1;
			}
		}
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 767 "parse.y"
    {
			(yyval.res) = parse_addr((yyvsp[(1) - (2)].val).buf, (yyvsp[(2) - (2)].val).buf);
			if ((yyval.res) == NULL) {
				/* yyerror already called by parse_addr */
				return -1;
			}
		}
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 777 "parse.y"
    { (yyval.num) = -1; }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 778 "parse.y"
    { (yyval.num) = (yyvsp[(2) - (2)].ulnum); }
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 783 "parse.y"
    {
			(yyval.val).buf = strdup("0");
			if (!(yyval.val).buf) {
				yyerror("insufficient memory");
				return -1;
			}
			(yyval.val).len = strlen((yyval.val).buf);
		}
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 792 "parse.y"
    {
			(yyval.val).buf = strdup("0");
			if (!(yyval.val).buf) {
				yyerror("insufficient memory");
				return -1;
			}
			(yyval.val).len = strlen((yyval.val).buf);
		}
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 801 "parse.y"
    {
			char buf[20];
			snprintf(buf, sizeof(buf), "%lu", (yyvsp[(2) - (3)].ulnum));
			(yyval.val).buf = strdup(buf);
			if (!(yyval.val).buf) {
				yyerror("insufficient memory");
				return -1;
			}
			(yyval.val).len = strlen((yyval.val).buf);
		}
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 812 "parse.y"
    {
			(yyval.val) = (yyvsp[(2) - (3)].val);
		}
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 818 "parse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].ulnum); }
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 819 "parse.y"
    { (yyval.num) = IPSEC_ULPROTO_ANY; }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 820 "parse.y"
    { 
				(yyval.num) = IPPROTO_TCP; 
			}
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 824 "parse.y"
    {
			struct protoent *ent;

			ent = getprotobyname((yyvsp[(1) - (1)].val).buf);
			if (ent)
				(yyval.num) = ent->p_proto;
			else {
				if (strcmp("icmp6", (yyvsp[(1) - (1)].val).buf) == 0) {
					(yyval.num) = IPPROTO_ICMPV6;
				} else if(strcmp("ip4", (yyvsp[(1) - (1)].val).buf) == 0) {
					(yyval.num) = IPPROTO_IPV4;
				} else {
					yyerror("invalid upper layer protocol");
					return -1;
				}
			}
			endprotoent();
		}
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 846 "parse.y"
    {
			(yyval.val).buf = NULL;
			(yyval.val).len = 0;
		}
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 851 "parse.y"
    {
			(yyval.val).buf = strdup((yyvsp[(1) - (1)].val).buf);
			if (!(yyval.val).buf) {
				yyerror("insufficient memory");
				return -1;
			}
			(yyval.val).len = strlen((yyval.val).buf);
		}
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 863 "parse.y"
    {
			sec_ctx.doi = (yyvsp[(2) - (4)].ulnum);
			sec_ctx.alg = (yyvsp[(3) - (4)].ulnum);
			sec_ctx.len = (yyvsp[(4) - (4)].val).len+1;
			sec_ctx.buf = (yyvsp[(4) - (4)].val).buf;
		}
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 873 "parse.y"
    {
			char *policy;
#ifdef HAVE_PFKEY_POLICY_PRIORITY
			struct sadb_x_policy *xpl;
#endif

			policy = ipsec_set_policy((yyvsp[(2) - (2)].val).buf, (yyvsp[(2) - (2)].val).len);
			if (policy == NULL) {
				yyerror(ipsec_strerror());
				return -1;
			}

			(yyval.val).buf = policy;
			(yyval.val).len = ipsec_get_policylen(policy);

#ifdef HAVE_PFKEY_POLICY_PRIORITY
			xpl = (struct sadb_x_policy *) (yyval.val).buf;
			last_priority = xpl->sadb_x_policy_priority;
#endif
		}
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 896 "parse.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 902 "parse.y"
    {
			exit_now = 1;
			YYACCEPT;
		}
    break;


/* Line 1787 of yacc.c  */
#line 2617 "parse.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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
      if (!yypact_value_is_default (yyn))
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

#if !defined yyoverflow || YYERROR_VERBOSE
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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


/* Line 2048 of yacc.c  */
#line 907 "parse.y"


int
setkeymsg0(msg, type, satype, l)
	struct sadb_msg *msg;
	unsigned int type;
	unsigned int satype;
	size_t l;
{

	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = type;
	msg->sadb_msg_errno = 0;
	msg->sadb_msg_satype = satype;
	msg->sadb_msg_reserved = 0;
	msg->sadb_msg_seq = 0;
	msg->sadb_msg_pid = getpid();
	msg->sadb_msg_len = PFKEY_UNIT64(l);
	return 0;
}

/* XXX NO BUFFER OVERRUN CHECK! BAD BAD! */
static int
setkeymsg_spdaddr(type, upper, policy, srcs, splen, dsts, dplen)
	unsigned int type;
	unsigned int upper;
	vchar_t *policy;
	struct addrinfo *srcs;
	int splen;
	struct addrinfo *dsts;
	int dplen;
{
	struct sadb_msg *msg;
	char buf[BUFSIZ];
	int l, l0;
	struct sadb_address m_addr;
	struct addrinfo *s, *d;
	int n;
	int plen;
	struct sockaddr *sa;
	int salen;
	struct sadb_x_policy *sp;
#ifdef HAVE_POLICY_FWD
	struct sadb_x_ipsecrequest *ps = NULL;
	int saved_level, saved_id = 0;
#endif

	msg = (struct sadb_msg *)buf;

	if (!srcs || !dsts)
		return -1;

	/* fix up length afterwards */
	setkeymsg0(msg, type, SADB_SATYPE_UNSPEC, 0);
	l = sizeof(struct sadb_msg);

	sp = (struct sadb_x_policy*) (buf + l);
	memcpy(buf + l, policy->buf, policy->len);
	l += policy->len;

	l0 = l;
	n = 0;

	/* do it for all src/dst pairs */
	for (s = srcs; s; s = s->ai_next) {
		for (d = dsts; d; d = d->ai_next) {
			/* rewind pointer */
			l = l0;

			if (s->ai_addr->sa_family != d->ai_addr->sa_family)
				continue;
			switch (s->ai_addr->sa_family) {
			case AF_INET:
				plen = sizeof(struct in_addr) << 3;
				break;
#ifdef INET6
			case AF_INET6:
				plen = sizeof(struct in6_addr) << 3;
				break;
#endif
			default:
				continue;
			}

			/* set src */
			sa = s->ai_addr;
			salen = sysdep_sa_len(s->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
			m_addr.sadb_address_proto = upper;
			m_addr.sadb_address_prefixlen =
			    (splen >= 0 ? splen : plen);
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), (caddr_t)sa, salen);

			/* set dst */
			sa = d->ai_addr;
			salen = sysdep_sa_len(d->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_DST;
			m_addr.sadb_address_proto = upper;
			m_addr.sadb_address_prefixlen =
			    (dplen >= 0 ? dplen : plen);
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);
#ifdef SADB_X_EXT_SEC_CTX
			/* Add security context label */
			if (sec_ctx.doi) {
				struct sadb_x_sec_ctx m_sec_ctx;
				u_int slen = sizeof(struct sadb_x_sec_ctx);

				memset(&m_sec_ctx, 0, slen);

				m_sec_ctx.sadb_x_sec_len =
				PFKEY_UNIT64(slen + PFKEY_ALIGN8(sec_ctx.len));

				m_sec_ctx.sadb_x_sec_exttype = 
					SADB_X_EXT_SEC_CTX;
				m_sec_ctx.sadb_x_ctx_len = sec_ctx.len;/*bytes*/
				m_sec_ctx.sadb_x_ctx_doi = sec_ctx.doi;
				m_sec_ctx.sadb_x_ctx_alg = sec_ctx.alg;
				setvarbuf(buf, &l, 
					  (struct sadb_ext *)&m_sec_ctx, slen, 
					  (caddr_t)sec_ctx.buf, sec_ctx.len);
			}
#endif
			msg->sadb_msg_len = PFKEY_UNIT64(l);

			sendkeymsg(buf, l);

#ifdef HAVE_POLICY_FWD
			/* create extra call for FWD policy */
			if (f_rfcmode && sp->sadb_x_policy_dir == IPSEC_DIR_INBOUND) {
				sp->sadb_x_policy_dir = IPSEC_DIR_FWD;
				ps = (struct sadb_x_ipsecrequest*) (sp+1);

				/* if request level is unique, change it to
				 * require for fwd policy */
				/* XXX: currently, only first policy is updated
				 * only. Update following too... */
				saved_level = ps->sadb_x_ipsecrequest_level;
				if (saved_level == IPSEC_LEVEL_UNIQUE) {
					saved_id = ps->sadb_x_ipsecrequest_reqid;
					ps->sadb_x_ipsecrequest_reqid=0;
					ps->sadb_x_ipsecrequest_level=IPSEC_LEVEL_REQUIRE;
				}

				sendkeymsg(buf, l);
				/* restoring for next message */
				sp->sadb_x_policy_dir = IPSEC_DIR_INBOUND;
				if (saved_level == IPSEC_LEVEL_UNIQUE) {
					ps->sadb_x_ipsecrequest_reqid = saved_id;
					ps->sadb_x_ipsecrequest_level = saved_level;
				}
			}
#endif

			n++;
		}
	}

	if (n == 0)
		return -1;
	else
		return 0;
}

static int
setkeymsg_spdaddr_tag(type, tag, policy)
	unsigned int type;
	char *tag;
	vchar_t *policy;
{
	struct sadb_msg *msg;
	char buf[BUFSIZ];
	int l, l0;
#ifdef SADB_X_EXT_TAG
	struct sadb_x_tag m_tag;
#endif
	int n;

	msg = (struct sadb_msg *)buf;

	/* fix up length afterwards */
	setkeymsg0(msg, type, SADB_SATYPE_UNSPEC, 0);
	l = sizeof(struct sadb_msg);

	memcpy(buf + l, policy->buf, policy->len);
	l += policy->len;

	l0 = l;
	n = 0;

#ifdef SADB_X_EXT_TAG
	memset(&m_tag, 0, sizeof(m_tag));
	m_tag.sadb_x_tag_len = PFKEY_UNIT64(sizeof(m_tag));
	m_tag.sadb_x_tag_exttype = SADB_X_EXT_TAG;
	if (strlcpy(m_tag.sadb_x_tag_name, tag,
	    sizeof(m_tag.sadb_x_tag_name)) >= sizeof(m_tag.sadb_x_tag_name))
		return -1;
	memcpy(buf + l, &m_tag, sizeof(m_tag));
	l += sizeof(m_tag);
#endif

	msg->sadb_msg_len = PFKEY_UNIT64(l);

	sendkeymsg(buf, l);

	return 0;
}

/* XXX NO BUFFER OVERRUN CHECK! BAD BAD! */
static int
setkeymsg_addr(type, satype, srcs, dsts, no_spi)
	unsigned int type;
	unsigned int satype;
	struct addrinfo *srcs;
	struct addrinfo *dsts;
	int no_spi;
{
	struct sadb_msg *msg;
	char buf[BUFSIZ];
	int l, l0, len;
	struct sadb_sa m_sa;
	struct sadb_x_sa2 m_sa2;
	struct sadb_address m_addr;
	struct addrinfo *s, *d;
	int n;
	int plen;
	struct sockaddr *sa;
	int salen;

	msg = (struct sadb_msg *)buf;

	if (!srcs || !dsts)
		return -1;

	/* fix up length afterwards */
	setkeymsg0(msg, type, satype, 0);
	l = sizeof(struct sadb_msg);

	if (!no_spi) {
		len = sizeof(struct sadb_sa);
		m_sa.sadb_sa_len = PFKEY_UNIT64(len);
		m_sa.sadb_sa_exttype = SADB_EXT_SA;
		m_sa.sadb_sa_spi = htonl(p_spi);
		m_sa.sadb_sa_replay = p_replay;
		m_sa.sadb_sa_state = 0;
		m_sa.sadb_sa_auth = p_alg_auth;
		m_sa.sadb_sa_encrypt = p_alg_enc;
		m_sa.sadb_sa_flags = p_ext;

		memcpy(buf + l, &m_sa, len);
		l += len;

		len = sizeof(struct sadb_x_sa2);
		m_sa2.sadb_x_sa2_len = PFKEY_UNIT64(len);
		m_sa2.sadb_x_sa2_exttype = SADB_X_EXT_SA2;
		m_sa2.sadb_x_sa2_mode = p_mode;
		m_sa2.sadb_x_sa2_reqid = p_reqid;

		memcpy(buf + l, &m_sa2, len);
		l += len;
	}

	l0 = l;
	n = 0;

	/* do it for all src/dst pairs */
	for (s = srcs; s; s = s->ai_next) {
		for (d = dsts; d; d = d->ai_next) {
			/* rewind pointer */
			l = l0;

			if (s->ai_addr->sa_family != d->ai_addr->sa_family)
				continue;
			switch (s->ai_addr->sa_family) {
			case AF_INET:
				plen = sizeof(struct in_addr) << 3;
				break;
#ifdef INET6
			case AF_INET6:
				plen = sizeof(struct in6_addr) << 3;
				break;
#endif
			default:
				continue;
			}

			/* set src */
			sa = s->ai_addr;
			salen = sysdep_sa_len(s->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
			m_addr.sadb_address_proto = IPSEC_ULPROTO_ANY;
			m_addr.sadb_address_prefixlen = plen;
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);

			/* set dst */
			sa = d->ai_addr;
			salen = sysdep_sa_len(d->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_DST;
			m_addr.sadb_address_proto = IPSEC_ULPROTO_ANY;
			m_addr.sadb_address_prefixlen = plen;
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);

			msg->sadb_msg_len = PFKEY_UNIT64(l);

			sendkeymsg(buf, l);

			n++;
		}
	}

	if (n == 0)
		return -1;
	else
		return 0;
}

#ifdef SADB_X_EXT_NAT_T_TYPE
static u_int16_t get_port (struct addrinfo *addr)
{
	struct sockaddr *s = addr->ai_addr;
	u_int16_t port = 0;

	switch (s->sa_family) {
	case AF_INET:
	  {
		struct sockaddr_in *sin4 = (struct sockaddr_in *)s;
		port = ntohs(sin4->sin_port);
		break;
	  }
	case AF_INET6:
	  {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)s;
		port = ntohs(sin6->sin6_port);
		break;
	  }
	}

	if (port == 0)
		port = DEFAULT_NATT_PORT;

	return port;
}
#endif

/* XXX NO BUFFER OVERRUN CHECK! BAD BAD! */
static int
setkeymsg_add(type, satype, srcs, dsts)
	unsigned int type;
	unsigned int satype;
	struct addrinfo *srcs;
	struct addrinfo *dsts;
{
	struct sadb_msg *msg;
	char buf[BUFSIZ];
	int l, l0, len;
	struct sadb_sa m_sa;
	struct sadb_x_sa2 m_sa2;
	struct sadb_address m_addr;
	struct addrinfo *s, *d;
	int n;
	int plen;
	struct sockaddr *sa;
	int salen;

	msg = (struct sadb_msg *)buf;

	if (!srcs || !dsts)
		return -1;

	/* fix up length afterwards */
	setkeymsg0(msg, type, satype, 0);
	l = sizeof(struct sadb_msg);

	/* set encryption algorithm, if present. */
	if (satype != SADB_X_SATYPE_IPCOMP && p_key_enc) {
		union {
			struct sadb_key key;
			struct sadb_ext ext;
		} m;

		m.key.sadb_key_len =
			PFKEY_UNIT64(sizeof(m.key)
				   + PFKEY_ALIGN8(p_key_enc_len));
		m.key.sadb_key_exttype = SADB_EXT_KEY_ENCRYPT;
		m.key.sadb_key_bits = p_key_enc_len * 8;
		m.key.sadb_key_reserved = 0;

		setvarbuf(buf, &l, &m.ext, sizeof(m.key),
			p_key_enc, p_key_enc_len);
	}

	/* set authentication algorithm, if present. */
	if (p_key_auth) {
		union {
			struct sadb_key key;
			struct sadb_ext ext;
		} m;

		m.key.sadb_key_len =
			PFKEY_UNIT64(sizeof(m.key)
				   + PFKEY_ALIGN8(p_key_auth_len));
		m.key.sadb_key_exttype = SADB_EXT_KEY_AUTH;
		m.key.sadb_key_bits = p_key_auth_len * 8;
		m.key.sadb_key_reserved = 0;

		setvarbuf(buf, &l, &m.ext, sizeof(m.key),
			p_key_auth, p_key_auth_len);
	}

	/* set lifetime for HARD */
	if (p_lt_hard != 0 || p_lb_hard != 0) {
		struct sadb_lifetime m_lt;
		u_int slen = sizeof(struct sadb_lifetime);

		m_lt.sadb_lifetime_len = PFKEY_UNIT64(slen);
		m_lt.sadb_lifetime_exttype = SADB_EXT_LIFETIME_HARD;
		m_lt.sadb_lifetime_allocations = 0;
		m_lt.sadb_lifetime_bytes = p_lb_hard;
		m_lt.sadb_lifetime_addtime = p_lt_hard;
		m_lt.sadb_lifetime_usetime = 0;

		memcpy(buf + l, &m_lt, slen);
		l += slen;
	}

	/* set lifetime for SOFT */
	if (p_lt_soft != 0 || p_lb_soft != 0) {
		struct sadb_lifetime m_lt;
		u_int slen = sizeof(struct sadb_lifetime);

		m_lt.sadb_lifetime_len = PFKEY_UNIT64(slen);
		m_lt.sadb_lifetime_exttype = SADB_EXT_LIFETIME_SOFT;
		m_lt.sadb_lifetime_allocations = 0;
		m_lt.sadb_lifetime_bytes = p_lb_soft;
		m_lt.sadb_lifetime_addtime = p_lt_soft;
		m_lt.sadb_lifetime_usetime = 0;

		memcpy(buf + l, &m_lt, slen);
		l += slen;
	}

#ifdef SADB_X_EXT_SEC_CTX
	/* Add security context label */
	if (sec_ctx.doi) {
		struct sadb_x_sec_ctx m_sec_ctx;
		u_int slen = sizeof(struct sadb_x_sec_ctx);

		memset(&m_sec_ctx, 0, slen);

		m_sec_ctx.sadb_x_sec_len = PFKEY_UNIT64(slen +
					PFKEY_ALIGN8(sec_ctx.len));
		m_sec_ctx.sadb_x_sec_exttype = SADB_X_EXT_SEC_CTX;
		m_sec_ctx.sadb_x_ctx_len = sec_ctx.len; /* bytes */
		m_sec_ctx.sadb_x_ctx_doi = sec_ctx.doi;
		m_sec_ctx.sadb_x_ctx_alg = sec_ctx.alg;
		setvarbuf(buf, &l, (struct sadb_ext *)&m_sec_ctx, slen,
			  (caddr_t)sec_ctx.buf, sec_ctx.len); 
	}
#endif

	len = sizeof(struct sadb_sa);
	m_sa.sadb_sa_len = PFKEY_UNIT64(len);
	m_sa.sadb_sa_exttype = SADB_EXT_SA;
	m_sa.sadb_sa_spi = htonl(p_spi);
	m_sa.sadb_sa_replay = p_replay;
	m_sa.sadb_sa_state = 0;
	m_sa.sadb_sa_auth = p_alg_auth;
	m_sa.sadb_sa_encrypt = p_alg_enc;
	m_sa.sadb_sa_flags = p_ext;

	memcpy(buf + l, &m_sa, len);
	l += len;

	len = sizeof(struct sadb_x_sa2);
	m_sa2.sadb_x_sa2_len = PFKEY_UNIT64(len);
	m_sa2.sadb_x_sa2_exttype = SADB_X_EXT_SA2;
	m_sa2.sadb_x_sa2_mode = p_mode;
	m_sa2.sadb_x_sa2_reqid = p_reqid;

	memcpy(buf + l, &m_sa2, len);
	l += len;

#ifdef SADB_X_EXT_NAT_T_TYPE
	if (p_natt_type) {
		struct sadb_x_nat_t_type natt_type;

		len = sizeof(struct sadb_x_nat_t_type);
		memset(&natt_type, 0, len);
		natt_type.sadb_x_nat_t_type_len = PFKEY_UNIT64(len);
		natt_type.sadb_x_nat_t_type_exttype = SADB_X_EXT_NAT_T_TYPE;
		natt_type.sadb_x_nat_t_type_type = p_natt_type;

		memcpy(buf + l, &natt_type, len);
		l += len;

		if (p_natt_oa) {
			sa = p_natt_oa->ai_addr;
			switch (sa->sa_family) {
			case AF_INET:
				plen = sizeof(struct in_addr) << 3;
				break;
#ifdef INET6
			case AF_INET6:
				plen = sizeof(struct in6_addr) << 3;
				break;
#endif
			default:
				return -1;
			}
			salen = sysdep_sa_len(sa);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_X_EXT_NAT_T_OA;
			m_addr.sadb_address_proto = IPSEC_ULPROTO_ANY;
			m_addr.sadb_address_prefixlen = plen;
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);
		}
	}
#endif

	l0 = l;
	n = 0;

	/* do it for all src/dst pairs */
	for (s = srcs; s; s = s->ai_next) {
		for (d = dsts; d; d = d->ai_next) {
			/* rewind pointer */
			l = l0;

			if (s->ai_addr->sa_family != d->ai_addr->sa_family)
				continue;
			switch (s->ai_addr->sa_family) {
			case AF_INET:
				plen = sizeof(struct in_addr) << 3;
				break;
#ifdef INET6
			case AF_INET6:
				plen = sizeof(struct in6_addr) << 3;
				break;
#endif
			default:
				continue;
			}

			/* set src */
			sa = s->ai_addr;
			salen = sysdep_sa_len(s->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
			m_addr.sadb_address_proto = IPSEC_ULPROTO_ANY;
			m_addr.sadb_address_prefixlen = plen;
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);

			/* set dst */
			sa = d->ai_addr;
			salen = sysdep_sa_len(d->ai_addr);
			m_addr.sadb_address_len = PFKEY_UNIT64(sizeof(m_addr) +
			    PFKEY_ALIGN8(salen));
			m_addr.sadb_address_exttype = SADB_EXT_ADDRESS_DST;
			m_addr.sadb_address_proto = IPSEC_ULPROTO_ANY;
			m_addr.sadb_address_prefixlen = plen;
			m_addr.sadb_address_reserved = 0;

			setvarbuf(buf, &l, (struct sadb_ext *)&m_addr,
			    sizeof(m_addr), sa, salen);

#ifdef SADB_X_EXT_NAT_T_TYPE
			if (p_natt_type) {
				struct sadb_x_nat_t_port natt_port;

				/* NATT_SPORT */
				len = sizeof(struct sadb_x_nat_t_port);
				memset(&natt_port, 0, len);
				natt_port.sadb_x_nat_t_port_len = PFKEY_UNIT64(len);
				natt_port.sadb_x_nat_t_port_exttype =
					SADB_X_EXT_NAT_T_SPORT;
				natt_port.sadb_x_nat_t_port_port = htons(get_port(s));
				
				memcpy(buf + l, &natt_port, len);
				l += len;

				/* NATT_DPORT */
				natt_port.sadb_x_nat_t_port_exttype =
					SADB_X_EXT_NAT_T_DPORT;
				natt_port.sadb_x_nat_t_port_port = htons(get_port(d));
				
				memcpy(buf + l, &natt_port, len);
				l += len;
			}
#endif
			msg->sadb_msg_len = PFKEY_UNIT64(l);

			sendkeymsg(buf, l);

			n++;
		}
	}

	if (n == 0)
		return -1;
	else
		return 0;
}

static struct addrinfo *
parse_addr(host, port)
	char *host;
	char *port;
{
	struct addrinfo hints, *res = NULL;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = p_aifamily;
	hints.ai_socktype = SOCK_DGRAM;		/*dummy*/
	hints.ai_protocol = IPPROTO_UDP;	/*dummy*/
	hints.ai_flags = p_aiflags;
	error = getaddrinfo(host, port, &hints, &res);
	if (error != 0) {
		yyerror(gai_strerror(error));
		return NULL;
	}
	return res;
}

static int
fix_portstr(ulproto, spec, sport, dport)
	int ulproto;
	vchar_t *spec, *sport, *dport;
{
	char sp[16], dp[16];
	int a, b, c, d;
	unsigned long u;

	if (spec->buf == NULL)
		return 0;

	switch (ulproto) {
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
	case IPPROTO_MH:
		if (sscanf(spec->buf, "%d,%d", &a, &b) == 2) {
			sprintf(sp, "%d", a);
			sprintf(dp, "%d", b);
		} else if (sscanf(spec->buf, "%d", &a) == 1) {
			sprintf(sp, "%d", a);
		} else {
			yyerror("invalid an upper layer protocol spec");
			return -1;
		}
		break;
	case IPPROTO_GRE:
		if (sscanf(spec->buf, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
			sprintf(sp, "%d", (a << 8) + b);
			sprintf(dp, "%d", (c << 8) + d);
		} else if (sscanf(spec->buf, "%lu", &u) == 1) {
			sprintf(sp, "%d", (int) (u >> 16));
			sprintf(dp, "%d", (int) (u & 0xffff));
		} else {
			yyerror("invalid an upper layer protocol spec");
			return -1;
		}
		break;
	}

	free(sport->buf);
	sport->buf = strdup(sp);
	if (!sport->buf) {
		yyerror("insufficient memory");
		return -1;
	}
	sport->len = strlen(sport->buf);

	free(dport->buf);
	dport->buf = strdup(dp);
	if (!dport->buf) {
		yyerror("insufficient memory");
		return -1;
	}
	dport->len = strlen(dport->buf);

	return 0;
}

static int
setvarbuf(buf, off, ebuf, elen, vbuf, vlen)
	char *buf;
	int *off;
	struct sadb_ext *ebuf;
	int elen;
	const void *vbuf;
	int vlen;
{
	memset(buf + *off, 0, PFKEY_UNUNIT64(ebuf->sadb_ext_len));
	memcpy(buf + *off, (caddr_t)ebuf, elen);
	memcpy(buf + *off + elen, vbuf, vlen);
	(*off) += PFKEY_ALIGN8(elen + vlen);

	return 0;
}

void
parse_init()
{
	p_spi = 0;

	p_ext = SADB_X_EXT_CYCSEQ;
	p_alg_enc = SADB_EALG_NONE;
	p_alg_auth = SADB_AALG_NONE;
	p_mode = IPSEC_MODE_ANY;
	p_reqid = 0;
	p_replay = 0;
	p_key_enc_len = p_key_auth_len = 0;
	p_key_enc = p_key_auth = 0;
	p_lt_hard = p_lt_soft = 0;
	p_lb_hard = p_lb_soft = 0;

	memset(&sec_ctx, 0, sizeof(struct security_ctx));

	p_aiflags = 0;
	p_aifamily = PF_UNSPEC;

	/* Clear out any natt OA information */
	if (p_natt_oa)
		freeaddrinfo (p_natt_oa);
	p_natt_oa = NULL;
	p_natt_type = 0;

	return;
}

void
free_buffer()
{
	/* we got tons of memory leaks in the parser anyways, leave them */

	return;
}
