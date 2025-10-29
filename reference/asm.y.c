/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "asm.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"

#define LABNUM 100

extern int yylineno;

int pass, ip;

struct label
{
	int addr;
	char *name;
} label[LABNUM];

int yylex();
void yyerror(char* msg);
void byte1(int  n);
void byte2(int  n);
void byte4(int n);

int number(char * name)
{
	int index=0;

	while(label[index].name != NULL)
	{
		if(!strcmp(label[index].name, name)) 
			return index;
		index++;
	}

	if(index>=LABNUM)
	{
		fprintf(stderr, "error: too many label");
		exit(0);
	}

	label[index].name=name;
	return index;
}

void byte1(int  n)
{
	if(pass==2)
		putchar(n);
	ip++;
}

void byte2(int  n)
{
	if(pass==2)
	{
		putchar(n);
		putchar(n>>8);
	}
	ip+=2;
}	

void byte4(int n)
{
	if(pass==2)
	{
		putchar(n);
		putchar(n>>8);
		putchar(n>>16);
		putchar(n>>24);
	}
	ip+=4;
}


#line 147 "asm.y.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "asm.y.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ADD = 3,                        /* ADD  */
  YYSYMBOL_SUB = 4,                        /* SUB  */
  YYSYMBOL_MUL = 5,                        /* MUL  */
  YYSYMBOL_DIV = 6,                        /* DIV  */
  YYSYMBOL_TST = 7,                        /* TST  */
  YYSYMBOL_STO = 8,                        /* STO  */
  YYSYMBOL_STC = 9,                        /* STC  */
  YYSYMBOL_LOD = 10,                       /* LOD  */
  YYSYMBOL_LDC = 11,                       /* LDC  */
  YYSYMBOL_JMP = 12,                       /* JMP  */
  YYSYMBOL_JEZ = 13,                       /* JEZ  */
  YYSYMBOL_JLZ = 14,                       /* JLZ  */
  YYSYMBOL_JGZ = 15,                       /* JGZ  */
  YYSYMBOL_DBN = 16,                       /* DBN  */
  YYSYMBOL_DBS = 17,                       /* DBS  */
  YYSYMBOL_ITC = 18,                       /* ITC  */
  YYSYMBOL_ITI = 19,                       /* ITI  */
  YYSYMBOL_OTC = 20,                       /* OTC  */
  YYSYMBOL_OTI = 21,                       /* OTI  */
  YYSYMBOL_OTS = 22,                       /* OTS  */
  YYSYMBOL_NOP = 23,                       /* NOP  */
  YYSYMBOL_END = 24,                       /* END  */
  YYSYMBOL_INTEGER = 25,                   /* INTEGER  */
  YYSYMBOL_REG = 26,                       /* REG  */
  YYSYMBOL_LABEL = 27,                     /* LABEL  */
  YYSYMBOL_28_ = 28,                       /* ','  */
  YYSYMBOL_29_ = 29,                       /* ':'  */
  YYSYMBOL_30_ = 30,                       /* '+'  */
  YYSYMBOL_31_ = 31,                       /* '-'  */
  YYSYMBOL_32_ = 32,                       /* '('  */
  YYSYMBOL_33_ = 33,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 34,                  /* $accept  */
  YYSYMBOL_start = 35,                     /* start  */
  YYSYMBOL_program = 36,                   /* program  */
  YYSYMBOL_statement = 37,                 /* statement  */
  YYSYMBOL_nop_stmt = 38,                  /* nop_stmt  */
  YYSYMBOL_add_stmt = 39,                  /* add_stmt  */
  YYSYMBOL_sub_stmt = 40,                  /* sub_stmt  */
  YYSYMBOL_mul_stmt = 41,                  /* mul_stmt  */
  YYSYMBOL_div_stmt = 42,                  /* div_stmt  */
  YYSYMBOL_tst_stmt = 43,                  /* tst_stmt  */
  YYSYMBOL_lab_stmt = 44,                  /* lab_stmt  */
  YYSYMBOL_jmp_stmt = 45,                  /* jmp_stmt  */
  YYSYMBOL_jez_stmt = 46,                  /* jez_stmt  */
  YYSYMBOL_jlz_stmt = 47,                  /* jlz_stmt  */
  YYSYMBOL_jgz_stmt = 48,                  /* jgz_stmt  */
  YYSYMBOL_lod_stmt = 49,                  /* lod_stmt  */
  YYSYMBOL_sto_stmt = 50,                  /* sto_stmt  */
  YYSYMBOL_input_stmt = 51,                /* input_stmt  */
  YYSYMBOL_output_stmt = 52,               /* output_stmt  */
  YYSYMBOL_end_stmt = 53,                  /* end_stmt  */
  YYSYMBOL_dbn_stmt = 54,                  /* dbn_stmt  */
  YYSYMBOL_dbs_stmt = 55                   /* dbs_stmt  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   152

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  81
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  158

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   282


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      32,    33,     2,    30,    28,    31,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    29,     2,
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
      25,    26,    27
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    89,    89,    92,    93,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   116,   119,   126,   133,   142,   149,   156,
     165,   172,   179,   188,   195,   202,   211,   220,   237,   244,
     253,   260,   269,   276,   285,   292,   301,   308,   315,   322,
     329,   336,   343,   350,   357,   364,   371,   378,   385,   392,
     401,   408,   415,   422,   429,   436,   443,   450,   457,   464,
     471,   478,   485,   494,   495,   498,   499,   500,   503,   506,
     514,   515
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ADD", "SUB", "MUL",
  "DIV", "TST", "STO", "STC", "LOD", "LDC", "JMP", "JEZ", "JLZ", "JGZ",
  "DBN", "DBS", "ITC", "ITI", "OTC", "OTI", "OTS", "NOP", "END", "INTEGER",
  "REG", "LABEL", "','", "':'", "'+'", "'-'", "'('", "')'", "$accept",
  "start", "program", "statement", "nop_stmt", "add_stmt", "sub_stmt",
  "mul_stmt", "div_stmt", "tst_stmt", "lab_stmt", "jmp_stmt", "jez_stmt",
  "jlz_stmt", "jgz_stmt", "lod_stmt", "sto_stmt", "input_stmt",
  "output_stmt", "end_stmt", "dbn_stmt", "dbs_stmt", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-5)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -5,    30,    -3,    -5,     9,    13,    17,    43,    54,    50,
      51,    55,    58,    -4,    37,    39,    41,    60,    61,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    59,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    62,    63,    64,    65,    66,
      -5,    69,    70,    71,    72,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    73,    -5,    -5,    77,    20,    23,    26,
      29,    -2,     3,     0,    57,    78,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    79,
      80,    81,    82,    83,    84,    -5,    40,    -5,    32,    47,
      -5,    85,    86,    35,    87,    88,    49,    89,    90,    91,
       7,    92,    93,    11,    94,    95,    -5,    46,    -5,    99,
     100,    -5,    48,    -5,    -5,    -5,   104,   105,    -5,    -5,
      -5,   106,   107,    -5,   108,   109,   111,   112,   113,   114,
     116,   117,   110,   115,   118,   119,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     2,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    23,    78,     0,     3,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    19,    18,    20,    21,    22,     0,     0,     0,     0,
      36,     0,     0,     0,     0,    39,    38,    41,    40,    43,
      42,    45,    44,     0,    81,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,    24,    26,    25,
      27,    29,    28,    30,    32,    31,    33,    35,    34,     0,
       0,     0,     0,     0,     0,    46,    48,    47,     0,     0,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    60,    63,    62,     0,
       0,    61,    64,    49,    50,    51,     0,     0,    54,    53,
      52,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,    71,    65,    67,
      70,    72,    66,    68,    56,    58,    57,    59
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    55,    56,    26,    95,    96,    97,    89,    90,
       3,    91,    98,    92,    93,    46,    94,   126,   127,    47,
     128,   131,   132,    48,   133,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,   109,   110,   111,
     116,   117,   118,    57,    58,    59,    60,    61,    62,    49,
     107,   108,   112,   113,   121,   122,   136,   137,   140,   141,
      50,    53,    51,    52,    54,    63,    64,     0,    65,    99,
      66,    67,    68,    69,    70,    71,    72,     0,     0,    73,
      74,    75,    76,   100,   101,   102,     0,   104,   105,   103,
       0,     0,   106,     0,   123,   124,     0,     0,   114,   115,
     119,   120,   134,   135,   125,   129,   130,   138,   139,   142,
     143,   144,   145,     0,   146,   147,   148,   149,     0,   150,
     151,   152,   153,   154,     0,     0,     0,     0,   155,     0,
       0,   156,   157
};

static const yytype_int8 yycheck[] =
{
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    26,    27,    27,    25,    26,    27,    30,    31,
       0,    33,    32,    30,    31,    26,    33,    30,    31,    26,
      33,    30,    31,    26,    33,    25,    26,    27,    25,    26,
      27,    25,    26,    27,    25,    26,    27,    25,    26,    27,
      25,    26,    27,    26,    27,    26,    27,    26,    27,    26,
      30,    31,    25,    26,    25,    26,    30,    31,    30,    31,
      26,    26,    32,    32,    26,    25,    25,    -1,    29,    32,
      28,    28,    28,    28,    28,    26,    26,    -1,    -1,    28,
      28,    28,    25,    25,    25,    25,    -1,    25,    25,    28,
      -1,    -1,    28,    -1,    25,    25,    -1,    -1,    33,    33,
      33,    33,    28,    28,    33,    33,    33,    28,    28,    25,
      25,    25,    25,    -1,    26,    26,    25,    25,    -1,    26,
      26,    25,    25,    33,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    33,    33
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    35,    36,     0,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    27,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    26,    26,    26,    26,
      26,    32,    32,    26,    26,    26,    27,    26,    27,    26,
      27,    26,    27,    25,    25,    29,    28,    28,    28,    28,
      28,    26,    26,    28,    28,    28,    25,    25,    26,    27,
      25,    26,    27,    25,    26,    27,    25,    26,    27,    30,
      31,    33,    30,    31,    33,    25,    26,    27,    32,    32,
      25,    25,    25,    28,    25,    25,    28,    30,    31,    25,
      26,    27,    25,    26,    33,    33,    25,    26,    27,    33,
      33,    25,    26,    25,    25,    33,    30,    31,    33,    33,
      33,    30,    31,    33,    28,    28,    30,    31,    28,    28,
      30,    31,    25,    25,    25,    25,    26,    26,    25,    25,
      26,    26,    25,    25,    33,    33,    33,    33
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    34,    35,    36,    36,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    38,    39,    39,    39,    40,    40,    40,
      41,    41,    41,    42,    42,    42,    43,    44,    45,    45,
      46,    46,    47,    47,    48,    48,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    51,    51,    52,    52,    52,    53,    54,
      55,    55
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     4,     4,     4,     6,
       6,     6,     6,     6,     6,     6,     8,     8,     8,     8,
       6,     6,     6,     6,     6,     8,     8,     8,     8,     8,
       8,     8,     8,     1,     1,     1,     1,     1,     1,     4,
       3,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 23: /* nop_stmt: NOP  */
#line 116 "asm.y"
                { byte2(I_NOP); byte1(0); byte1(0); byte4(0);}
#line 1287 "asm.y.c"
    break;

  case 24: /* add_stmt: ADD REG ',' INTEGER  */
#line 120 "asm.y"
{
	byte2(I_ADD_0) ;
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1298 "asm.y.c"
    break;

  case 25: /* add_stmt: ADD REG ',' LABEL  */
#line 127 "asm.y"
{
	byte2(I_ADD_0);
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1309 "asm.y.c"
    break;

  case 26: /* add_stmt: ADD REG ',' REG  */
#line 134 "asm.y"
{
	byte2(I_ADD_1);
	byte1((yyvsp[-2].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1320 "asm.y.c"
    break;

  case 27: /* sub_stmt: SUB REG ',' INTEGER  */
#line 143 "asm.y"
{
	byte2(I_SUB_0) ;
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1331 "asm.y.c"
    break;

  case 28: /* sub_stmt: SUB REG ',' LABEL  */
#line 150 "asm.y"
{
	byte2(I_SUB_0);
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1342 "asm.y.c"
    break;

  case 29: /* sub_stmt: SUB REG ',' REG  */
#line 157 "asm.y"
{
	byte2(I_SUB_1);
	byte1((yyvsp[-2].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1353 "asm.y.c"
    break;

  case 30: /* mul_stmt: MUL REG ',' INTEGER  */
#line 166 "asm.y"
{
	byte2(I_MUL_0) ;
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1364 "asm.y.c"
    break;

  case 31: /* mul_stmt: MUL REG ',' LABEL  */
#line 173 "asm.y"
{
	byte2(I_MUL_0);
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1375 "asm.y.c"
    break;

  case 32: /* mul_stmt: MUL REG ',' REG  */
#line 180 "asm.y"
{
	byte2(I_MUL_1);
	byte1((yyvsp[-2].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1386 "asm.y.c"
    break;

  case 33: /* div_stmt: DIV REG ',' INTEGER  */
#line 189 "asm.y"
{
	byte2(I_DIV_0) ;
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1397 "asm.y.c"
    break;

  case 34: /* div_stmt: DIV REG ',' LABEL  */
#line 196 "asm.y"
{
	byte2(I_DIV_0);
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1408 "asm.y.c"
    break;

  case 35: /* div_stmt: DIV REG ',' REG  */
#line 203 "asm.y"
{
	byte2(I_DIV_1);
	byte1((yyvsp[-2].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1419 "asm.y.c"
    break;

  case 36: /* tst_stmt: TST REG  */
#line 212 "asm.y"
{
	byte2(I_TST_0) ;
	byte1((yyvsp[0].number));
	byte1(0);
	byte4(0);
}
#line 1430 "asm.y.c"
    break;

  case 37: /* lab_stmt: LABEL ':'  */
#line 221 "asm.y"
{
	if(pass==1)
	{
		if(label[number((yyvsp[-1].string))].addr==0)
		{
			label[number((yyvsp[-1].string))].addr=ip;
		}
		else
		{
			fprintf(stderr, "error: label %s already exist\n", label[number((yyvsp[-1].string))].name);
			exit(0);
		}
	}
}
#line 1449 "asm.y.c"
    break;

  case 38: /* jmp_stmt: JMP LABEL  */
#line 238 "asm.y"
{
	byte2(I_JMP_0);
	byte1(0);
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1460 "asm.y.c"
    break;

  case 39: /* jmp_stmt: JMP REG  */
#line 245 "asm.y"
{
	byte2(I_JMP_1);
	byte1((yyvsp[0].number));		
	byte1(0);	
	byte4(0);				
}
#line 1471 "asm.y.c"
    break;

  case 40: /* jez_stmt: JEZ LABEL  */
#line 254 "asm.y"
{
	byte2(I_JEZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1482 "asm.y.c"
    break;

  case 41: /* jez_stmt: JEZ REG  */
#line 261 "asm.y"
{
	byte2(I_JEZ_1) ;
	byte1((yyvsp[0].number));		
	byte1(0);	
	byte4(0);	
}
#line 1493 "asm.y.c"
    break;

  case 42: /* jlz_stmt: JLZ LABEL  */
#line 270 "asm.y"
{
	byte2(I_JLZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1504 "asm.y.c"
    break;

  case 43: /* jlz_stmt: JLZ REG  */
#line 277 "asm.y"
{
	byte2(I_JLZ_1) ;
	byte1( (yyvsp[0].number) ) ;		
	byte1(0);	
	byte4(0);	
}
#line 1515 "asm.y.c"
    break;

  case 44: /* jgz_stmt: JGZ LABEL  */
#line 286 "asm.y"
{
	byte2(I_JGZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1526 "asm.y.c"
    break;

  case 45: /* jgz_stmt: JGZ REG  */
#line 293 "asm.y"
{
	byte2(I_JGZ_1) ;
	byte1( (yyvsp[0].number) ) ;		
	byte1(0);	
	byte4(0);	
}
#line 1537 "asm.y.c"
    break;

  case 46: /* lod_stmt: LOD REG ',' INTEGER  */
#line 302 "asm.y"
{
	byte2(I_LOD_0) ;
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1548 "asm.y.c"
    break;

  case 47: /* lod_stmt: LOD REG ',' LABEL  */
#line 309 "asm.y"
{
	byte2(I_LOD_0);
	byte1((yyvsp[-2].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1559 "asm.y.c"
    break;

  case 48: /* lod_stmt: LOD REG ',' REG  */
#line 316 "asm.y"
{
	byte2(I_LOD_1);
	byte1((yyvsp[-2].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1570 "asm.y.c"
    break;

  case 49: /* lod_stmt: LOD REG ',' REG '+' INTEGER  */
#line 323 "asm.y"
{
	byte2(I_LOD_2);
	byte1((yyvsp[-4].number));
	byte1((yyvsp[-2].number));
	byte4((yyvsp[0].number));
}
#line 1581 "asm.y.c"
    break;

  case 50: /* lod_stmt: LOD REG ',' REG '-' INTEGER  */
#line 330 "asm.y"
{
	byte2(I_LOD_2);
	byte1((yyvsp[-4].number));
	byte1((yyvsp[-2].number));
	byte4(-((yyvsp[0].number)));
}
#line 1592 "asm.y.c"
    break;

  case 51: /* lod_stmt: LOD REG ',' '(' INTEGER ')'  */
#line 337 "asm.y"
{
	byte2(I_LOD_3);
	byte1((yyvsp[-4].number));
	byte1(0);
	byte4((yyvsp[-1].number));
}
#line 1603 "asm.y.c"
    break;

  case 52: /* lod_stmt: LDC REG ',' '(' INTEGER ')'  */
#line 344 "asm.y"
{
	byte2(I_LDC_3);
	byte1((yyvsp[-4].number));
	byte1(0);
	byte4((yyvsp[-1].number));
}
#line 1614 "asm.y.c"
    break;

  case 53: /* lod_stmt: LOD REG ',' '(' LABEL ')'  */
#line 351 "asm.y"
{
	byte2(I_LOD_3);
	byte1((yyvsp[-4].number));
	byte1(0);
	byte4(label[number((yyvsp[-1].string))].addr);
}
#line 1625 "asm.y.c"
    break;

  case 54: /* lod_stmt: LOD REG ',' '(' REG ')'  */
#line 358 "asm.y"
{
	byte2(I_LOD_4);
	byte1((yyvsp[-4].number));
	byte1((yyvsp[-1].number));
	byte4(0);
}
#line 1636 "asm.y.c"
    break;

  case 55: /* lod_stmt: LDC REG ',' '(' REG ')'  */
#line 365 "asm.y"
{
	byte2(I_LDC_4);
	byte1((yyvsp[-4].number));
	byte1((yyvsp[-1].number));
	byte4(0);
}
#line 1647 "asm.y.c"
    break;

  case 56: /* lod_stmt: LOD REG ',' '(' REG '+' INTEGER ')'  */
#line 372 "asm.y"
{
	byte2(I_LOD_5);
	byte1((yyvsp[-6].number));
	byte1((yyvsp[-3].number));
	byte4((yyvsp[-1].number));
}
#line 1658 "asm.y.c"
    break;

  case 57: /* lod_stmt: LDC REG ',' '(' REG '+' INTEGER ')'  */
#line 379 "asm.y"
{
	byte2(I_LDC_5);
	byte1((yyvsp[-6].number));
	byte1((yyvsp[-3].number));
	byte4((yyvsp[-1].number));
}
#line 1669 "asm.y.c"
    break;

  case 58: /* lod_stmt: LOD REG ',' '(' REG '-' INTEGER ')'  */
#line 386 "asm.y"
{
	byte2(I_LOD_5);
	byte1((yyvsp[-6].number));
	byte1((yyvsp[-3].number));
	byte4(-((yyvsp[-1].number)));
}
#line 1680 "asm.y.c"
    break;

  case 59: /* lod_stmt: LDC REG ',' '(' REG '-' INTEGER ')'  */
#line 393 "asm.y"
{
	byte2(I_LDC_5);
	byte1((yyvsp[-6].number));
	byte1((yyvsp[-3].number));
	byte4(-((yyvsp[-1].number)));
}
#line 1691 "asm.y.c"
    break;

  case 60: /* sto_stmt: STO '(' REG ')' ',' INTEGER  */
#line 402 "asm.y"
{
	byte2(I_STO_0);
	byte1((yyvsp[-3].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1702 "asm.y.c"
    break;

  case 61: /* sto_stmt: STC '(' REG ')' ',' INTEGER  */
#line 409 "asm.y"
{
	byte2(I_STC_0);
	byte1((yyvsp[-3].number));
	byte1(0);
	byte4((yyvsp[0].number));
}
#line 1713 "asm.y.c"
    break;

  case 62: /* sto_stmt: STO '(' REG ')' ',' LABEL  */
#line 416 "asm.y"
{
	byte2(I_STO_0);
	byte1((yyvsp[-3].number));
	byte1(0);
	byte4(label[number((yyvsp[0].string))].addr);
}
#line 1724 "asm.y.c"
    break;

  case 63: /* sto_stmt: STO '(' REG ')' ',' REG  */
#line 423 "asm.y"
{
	byte2( I_STO_1 ) ;
	byte1((yyvsp[-3].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1735 "asm.y.c"
    break;

  case 64: /* sto_stmt: STC '(' REG ')' ',' REG  */
#line 430 "asm.y"
{
	byte2( I_STC_1 ) ;
	byte1((yyvsp[-3].number));
	byte1((yyvsp[0].number));
	byte4(0);
}
#line 1746 "asm.y.c"
    break;

  case 65: /* sto_stmt: STO '(' REG ')' ',' REG '+' INTEGER  */
#line 437 "asm.y"
{
	byte2( I_STO_2 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[-2].number));
	byte4((yyvsp[0].number));
}
#line 1757 "asm.y.c"
    break;

  case 66: /* sto_stmt: STC '(' REG ')' ',' REG '+' INTEGER  */
#line 444 "asm.y"
{
	byte2( I_STC_2 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[-2].number));
	byte4((yyvsp[0].number));
}
#line 1768 "asm.y.c"
    break;

  case 67: /* sto_stmt: STO '(' REG ')' ',' REG '-' INTEGER  */
#line 451 "asm.y"
{
	byte2( I_STO_2 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[-2].number));
	byte4(-((yyvsp[0].number)));
}
#line 1779 "asm.y.c"
    break;

  case 68: /* sto_stmt: STC '(' REG ')' ',' REG '-' INTEGER  */
#line 458 "asm.y"
{
	byte2( I_STC_2 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[-2].number));
	byte4(-((yyvsp[0].number)));
}
#line 1790 "asm.y.c"
    break;

  case 69: /* sto_stmt: STO '(' REG '+' INTEGER ')' ',' REG  */
#line 465 "asm.y"
{
	byte2( I_STO_3 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[0].number));
	byte4((yyvsp[-3].number));
}
#line 1801 "asm.y.c"
    break;

  case 70: /* sto_stmt: STC '(' REG '+' INTEGER ')' ',' REG  */
#line 472 "asm.y"
{
	byte2( I_STC_3 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[0].number));
	byte4((yyvsp[-3].number));
}
#line 1812 "asm.y.c"
    break;

  case 71: /* sto_stmt: STO '(' REG '-' INTEGER ')' ',' REG  */
#line 479 "asm.y"
{
	byte2( I_STO_3 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[0].number));
	byte4(-((yyvsp[-3].number)));
}
#line 1823 "asm.y.c"
    break;

  case 72: /* sto_stmt: STC '(' REG '-' INTEGER ')' ',' REG  */
#line 486 "asm.y"
{
	byte2( I_STC_3 ) ;
	byte1((yyvsp[-5].number));
	byte1((yyvsp[0].number));
	byte4(-((yyvsp[-3].number)));
}
#line 1834 "asm.y.c"
    break;

  case 73: /* input_stmt: ITC  */
#line 494 "asm.y"
                 { byte2(I_ITC); byte1(0); byte1(0); byte4(0);}
#line 1840 "asm.y.c"
    break;

  case 74: /* input_stmt: ITI  */
#line 495 "asm.y"
      { byte2(I_ITI); byte1(0); byte1(0); byte4(0);}
#line 1846 "asm.y.c"
    break;

  case 75: /* output_stmt: OTC  */
#line 498 "asm.y"
                  { byte2(I_OTC); byte1(0); byte1(0); byte4(0);}
#line 1852 "asm.y.c"
    break;

  case 76: /* output_stmt: OTI  */
#line 499 "asm.y"
      { byte2(I_OTI); byte1(0); byte1(0); byte4(0);}
#line 1858 "asm.y.c"
    break;

  case 77: /* output_stmt: OTS  */
#line 500 "asm.y"
      { byte2(I_OTS); byte1(0); byte1(0); byte4(0);}
#line 1864 "asm.y.c"
    break;

  case 78: /* end_stmt: END  */
#line 503 "asm.y"
               { byte2(I_END); byte1(0); byte1(0); byte4(0);}
#line 1870 "asm.y.c"
    break;

  case 79: /* dbn_stmt: DBN INTEGER ',' INTEGER  */
#line 507 "asm.y"
{
	int n = (yyvsp[0].number);
	while(n-- > 0)
		byte1((yyvsp[-2].number));
}
#line 1880 "asm.y.c"
    break;

  case 80: /* dbs_stmt: dbs_stmt ',' INTEGER  */
#line 514 "asm.y"
                                { byte1((yyvsp[0].number)); }
#line 1886 "asm.y.c"
    break;

  case 81: /* dbs_stmt: DBS INTEGER  */
#line 515 "asm.y"
              { byte1((yyvsp[0].number)); }
#line 1892 "asm.y.c"
    break;


#line 1896 "asm.y.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 518 "asm.y"


void yyerror(char* msg) 
{
	fprintf(stderr, "%s: line %d\n", msg, yylineno);
	exit(0);
}

int main(int argc,   char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(0);
	}
	
	char *input, *output;

	input = argv[1];
	if(freopen(input, "r", stdin)==NULL)
	{
		fprintf(stderr, "error: open %s failed\n", input);
		return 0;
	}

	output=strdup(input);
	output[strlen(output)-1]='o';

	if(freopen(output, "w", stdout)==NULL)
	{
		fprintf(stderr, "error: open %s failed\n", output);
		return 0;
	}

	int i=sizeof(label);
	bzero(label, i);

	/* First pass, set up labels */
	pass=1;
	ip=0;
	yyparse();

	/* Second pass, generate code */
	pass=2;
	ip=0;
	rewind(stdin) ;
	yyparse();

	return 0;
}

