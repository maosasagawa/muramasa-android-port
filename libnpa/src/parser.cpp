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
#line 1 "/home/ubuntu/repos/libnpa/src/parser.y"

    #include "nsscompiler.hpp"
    #ifdef __ANDROID__
#include <android/log.h>
#endif
#include <cstdio>
    #include <cstdlib>
    #include <boost/algorithm/string/replace.hpp>

    Call* MakeCall(string Name, uint16_t Magic);
    void WriteSymbol(const string& Symbol);
    Block* WrapStmt(Statement* s) { Block* b = new Block(); b->Statements.push_back(s); return b; }

    Program* pRoot;
    extern int yylex();
    void yyerror(const char* s) { extern int g_nss_line; extern char* yytext; extern int yyleng;
    std::fprintf(stderr, "Error: %s at line %d near '%s'\n", s, g_nss_line, yytext);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_ERROR, "muramasa", "nss parse error: %s line %d near '%s'", s, g_nss_line, yytext);
#endif
    std::abort(); }

#line 87 "/home/ubuntu/repos/libnpa/src/parser.cpp"

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

#include "parser.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TIDENTIFIER = 3,                /* TIDENTIFIER  */
  YYSYMBOL_TFLOAT = 4,                     /* TFLOAT  */
  YYSYMBOL_TINTEGER = 5,                   /* TINTEGER  */
  YYSYMBOL_TXML = 6,                       /* TXML  */
  YYSYMBOL_TSTRING = 7,                    /* TSTRING  */
  YYSYMBOL_THEX = 8,                       /* THEX  */
  YYSYMBOL_TVARNAME = 9,                   /* TVARNAME  */
  YYSYMBOL_TLPAREN = 10,                   /* TLPAREN  */
  YYSYMBOL_TRPAREN = 11,                   /* TRPAREN  */
  YYSYMBOL_TLBRACE = 12,                   /* TLBRACE  */
  YYSYMBOL_TRBRACE = 13,                   /* TRBRACE  */
  YYSYMBOL_TFUNCTION = 14,                 /* TFUNCTION  */
  YYSYMBOL_TSEMICOLON = 15,                /* TSEMICOLON  */
  YYSYMBOL_TEQUAL = 16,                    /* TEQUAL  */
  YYSYMBOL_TCOMMA = 17,                    /* TCOMMA  */
  YYSYMBOL_TQUOTE = 18,                    /* TQUOTE  */
  YYSYMBOL_TCHAPTER = 19,                  /* TCHAPTER  */
  YYSYMBOL_TSCENE = 20,                    /* TSCENE  */
  YYSYMBOL_TINCLUDE = 21,                  /* TINCLUDE  */
  YYSYMBOL_TADD = 22,                      /* TADD  */
  YYSYMBOL_TSUB = 23,                      /* TSUB  */
  YYSYMBOL_TMUL = 24,                      /* TMUL  */
  YYSYMBOL_TDIV = 25,                      /* TDIV  */
  YYSYMBOL_TMOD = 26,                      /* TMOD  */
  YYSYMBOL_TIF = 27,                       /* TIF  */
  YYSYMBOL_TWHILE = 28,                    /* TWHILE  */
  YYSYMBOL_TLESS = 29,                     /* TLESS  */
  YYSYMBOL_TGREATER = 30,                  /* TGREATER  */
  YYSYMBOL_TEQUALEQUAL = 31,               /* TEQUALEQUAL  */
  YYSYMBOL_TNEQUAL = 32,                   /* TNEQUAL  */
  YYSYMBOL_TGEQUAL = 33,                   /* TGEQUAL  */
  YYSYMBOL_TLEQUAL = 34,                   /* TLEQUAL  */
  YYSYMBOL_TAND = 35,                      /* TAND  */
  YYSYMBOL_TOR = 36,                       /* TOR  */
  YYSYMBOL_TNOT = 37,                      /* TNOT  */
  YYSYMBOL_TRETURN = 38,                   /* TRETURN  */
  YYSYMBOL_TCALLCHAPTER = 39,              /* TCALLCHAPTER  */
  YYSYMBOL_TCALLSCENE = 40,                /* TCALLSCENE  */
  YYSYMBOL_TELSE = 41,                     /* TELSE  */
  YYSYMBOL_TSELECT = 42,                   /* TSELECT  */
  YYSYMBOL_TCASE = 43,                     /* TCASE  */
  YYSYMBOL_TCOLON = 44,                    /* TCOLON  */
  YYSYMBOL_TAT = 45,                       /* TAT  */
  YYSYMBOL_TDOLLAR = 46,                   /* TDOLLAR  */
  YYSYMBOL_THASH = 47,                     /* THASH  */
  YYSYMBOL_TBREAK = 48,                    /* TBREAK  */
  YYSYMBOL_TLABRACE = 49,                  /* TLABRACE  */
  YYSYMBOL_TRABRACE = 50,                  /* TRABRACE  */
  YYSYMBOL_TARROW = 51,                    /* TARROW  */
  YYSYMBOL_UNARY = 52,                     /* UNARY  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_start = 54,                     /* start  */
  YYSYMBOL_55_1 = 55,                      /* $@1  */
  YYSYMBOL_program = 56,                   /* program  */
  YYSYMBOL_include = 57,                   /* include  */
  YYSYMBOL_stmts = 58,                     /* stmts  */
  YYSYMBOL_stmt = 59,                      /* stmt  */
  YYSYMBOL_block = 60,                     /* block  */
  YYSYMBOL_func_decl = 61,                 /* func_decl  */
  YYSYMBOL_func_args = 62,                 /* func_args  */
  YYSYMBOL_func_exps = 63,                 /* func_exps  */
  YYSYMBOL_arg = 64,                       /* arg  */
  YYSYMBOL_array = 65,                     /* array  */
  YYSYMBOL_call = 66,                      /* call  */
  YYSYMBOL_expr = 67,                      /* expr  */
  YYSYMBOL_cond = 68                       /* cond  */
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
#define YYLAST   579

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  206

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   307


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
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    58,    58,    58,    61,    62,    63,    64,    67,    70,
      71,    74,    74,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    87,    88,    91,    92,    93,    96,
      97,    98,   101,   102,   103,   104,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   122,
     123,   124,   125,   127,   128,   134,   140,   156,   157,   158,
     159,   160,   161,   162,   163,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204
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
  "\"end of file\"", "error", "\"invalid token\"", "TIDENTIFIER",
  "TFLOAT", "TINTEGER", "TXML", "TSTRING", "THEX", "TVARNAME", "TLPAREN",
  "TRPAREN", "TLBRACE", "TRBRACE", "TFUNCTION", "TSEMICOLON", "TEQUAL",
  "TCOMMA", "TQUOTE", "TCHAPTER", "TSCENE", "TINCLUDE", "TADD", "TSUB",
  "TMUL", "TDIV", "TMOD", "TIF", "TWHILE", "TLESS", "TGREATER",
  "TEQUALEQUAL", "TNEQUAL", "TGEQUAL", "TLEQUAL", "TAND", "TOR", "TNOT",
  "TRETURN", "TCALLCHAPTER", "TCALLSCENE", "TELSE", "TSELECT", "TCASE",
  "TCOLON", "TAT", "TDOLLAR", "THASH", "TBREAK", "TLABRACE", "TRABRACE",
  "TARROW", "UNARY", "$accept", "start", "$@1", "program", "include",
  "stmts", "stmt", "block", "func_decl", "func_args", "func_exps", "arg",
  "array", "call", "expr", "cond", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-129)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -129,    21,    88,  -129,    85,    85,    85,    24,    88,  -129,
    -129,  -129,  -129,  -129,  -129,  -129,   -37,  -129,     6,    31,
      48,    42,    37,    37,  -129,  -129,  -129,    85,    47,    68,
      49,    85,    85,   158,  -129,  -129,    54,    85,  -129,    85,
      57,  -129,    56,    66,  -129,  -129,  -129,   111,   113,   105,
     328,   342,    37,     5,   112,   204,  -129,  -129,   118,  -129,
    -129,  -129,    75,    79,    37,    85,  -129,  -129,   318,   318,
    -129,   121,    86,     7,    91,   124,    92,     8,    94,  -129,
       1,   143,  -129,  -129,  -129,   318,   318,     0,    39,   131,
    -129,  -129,  -129,  -129,   318,   318,   318,   318,   318,   141,
     368,   384,  -129,    85,   -11,    85,  -129,    85,   -10,    85,
     149,    37,  -129,    12,   262,   503,   422,   318,    15,   318,
      22,   318,   400,  -129,  -129,  -129,  -129,   318,   110,   318,
     318,   318,   318,   318,   318,   318,   318,   318,   318,   318,
     318,   318,   110,   138,  -129,   139,   144,  -129,   145,    13,
    -129,    37,  -129,   153,   318,   503,  -129,   444,  -129,  -129,
     466,  -129,  -129,   488,  -129,   307,   128,   164,   134,    -6,
      -6,  -129,  -129,  -129,   154,   154,   532,   532,   154,   154,
     545,   518,  -129,  -129,  -129,  -129,  -129,  -129,    37,  -129,
    -129,  -129,   503,  -129,  -129,  -129,  -129,   250,   250,  -129,
    -129,  -129,  -129,  -129,  -129,  -129
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     0,     1,     0,     0,     0,     0,     3,     6,
       4,    39,    42,    41,    40,    38,    46,    48,    45,    44,
       0,    43,     0,     0,     8,     7,     5,     0,    36,     0,
      37,    29,     0,     0,    27,    28,     0,     0,    47,     0,
       0,    30,     0,    39,    56,    25,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     9,    13,     0,    11,
      12,    52,     0,     0,     0,     0,    51,    22,     0,     0,
      54,    39,     0,    45,     0,    39,     0,    45,     0,    93,
       0,     0,    55,    24,    10,    32,     0,     0,     0,     0,
      49,    50,    26,    31,     0,     0,     0,     0,    48,    65,
       0,     0,    57,     0,    36,     0,    61,     0,    36,     0,
       0,     0,    97,     0,     0,    33,     0,     0,     0,     0,
       0,     0,     0,    84,    80,    79,    81,    32,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    58,     0,     0,    62,     0,     0,
      94,     0,    99,     0,     0,    35,    14,     0,    18,    20,
       0,    19,    21,     0,    83,     0,    85,     0,    88,    69,
      70,    66,    67,    68,    71,    72,    73,    74,    75,    76,
      77,    78,   100,   101,    60,    59,    64,    63,     0,    95,
      98,    53,    34,    15,    16,    17,    82,     0,     0,    96,
      87,    89,    86,    92,    90,    91
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -129,  -129,  -129,  -129,   181,  -129,   135,   -16,   183,  -129,
      65,    -4,  -129,  -125,   -54,  -128
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     8,     9,    55,    56,    57,    10,    40,
     114,    99,    21,    59,   115,    60
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      20,    22,    23,   168,   144,   147,    34,    35,    80,    28,
     104,   108,    27,    33,   100,   101,   117,   183,   131,   132,
     133,     3,   118,    36,    33,    33,   110,    41,    42,    58,
     158,    24,   116,    62,    30,    63,    79,   161,    37,    37,
     122,   123,   124,   125,   126,   111,    74,    78,    92,    33,
      81,    58,    29,    29,    29,   119,   151,   188,    31,   159,
     155,    93,   120,   157,   112,   160,   162,   163,    64,   202,
     205,    38,   201,   204,    65,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,    11,    12,
      13,    32,    14,    15,    16,   150,    37,   152,    39,   143,
     192,   145,     4,   146,    61,   148,    66,     5,     6,     7,
      67,   155,   166,    11,    12,    13,    44,    14,    15,    16,
      70,    68,    33,    69,   167,    90,   182,    82,    85,    91,
      17,    18,    19,   189,    86,   190,   102,   103,   167,   106,
      87,    88,   105,   107,    89,   109,   113,   121,    49,    50,
      51,   127,   149,   184,   185,    17,    18,    19,    54,   186,
     187,    43,    12,    13,    44,    14,    15,    16,   191,   197,
      33,    45,   199,    46,    85,   198,   129,   130,   131,   132,
     133,   200,   203,    -1,    -1,    47,    48,    -1,    -1,    25,
      84,    26,   165,   167,   167,     0,    49,    50,    51,     0,
      52,    53,     0,    17,    18,    19,    54,    43,    12,    13,
      44,    14,    15,    16,     0,     0,    33,    83,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,    50,    51,     0,    52,    53,     0,    17,
      18,    19,    54,    11,    12,    13,    44,    14,    15,    16,
       0,     0,    33,     0,     0,    11,    12,    13,     0,    14,
      15,    16,    94,   153,     0,     0,     0,    47,    48,   154,
       0,     0,     0,     0,    95,    96,     0,     0,    49,    50,
      51,     0,    52,    53,     0,    17,    18,    19,    54,    97,
       0,     0,     0,     0,     0,     0,     0,    98,    18,    19,
      11,    12,    13,     0,    14,    15,    16,    94,   196,     0,
       0,    11,    12,    13,   154,    14,    15,    16,    94,    95,
      96,    71,    12,    13,     0,    14,    15,    16,     0,     0,
      95,    96,     0,     0,    97,    75,    12,    13,     0,    14,
      15,    16,    98,    18,    19,    97,     0,     0,     0,     0,
       0,     0,     0,    98,    18,    19,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    19,     0,     0,     0,   128,
       0,     0,     0,     0,     0,     0,     0,    76,    77,    19,
     129,   130,   131,   132,   133,   142,     0,   134,   135,   136,
     137,   138,   139,   140,   141,     0,   129,   130,   131,   132,
     133,   164,     0,   134,   135,   136,   137,   138,   139,   140,
     141,     0,   129,   130,   131,   132,   133,     0,     0,   134,
     135,   136,   137,   138,   139,   140,   141,   156,     0,     0,
       0,     0,     0,     0,   129,   130,   131,   132,   133,     0,
       0,   134,   135,   136,   137,   138,   139,   140,   141,   193,
       0,     0,     0,     0,     0,     0,   129,   130,   131,   132,
     133,     0,     0,   134,   135,   136,   137,   138,   139,   140,
     141,   194,     0,     0,     0,     0,     0,     0,   129,   130,
     131,   132,   133,     0,     0,   134,   135,   136,   137,   138,
     139,   140,   141,   195,     0,     0,     0,     0,     0,     0,
     129,   130,   131,   132,   133,     0,     0,   134,   135,   136,
     137,   138,   139,   140,   141,   129,   130,   131,   132,   133,
       0,     0,   134,   135,   136,   137,   138,   139,   140,   141,
     129,   130,   131,   132,   133,     0,     0,   134,   135,   136,
     137,   138,   139,   140,   129,   130,   131,   132,   133,     0,
       0,   134,   135,    -1,    -1,   138,   139,   129,   130,   131,
     132,   133,     0,     0,   134,   135,   136,   137,   138,   139
};

static const yytype_int16 yycheck[] =
{
       4,     5,     6,   128,    15,    15,    22,    23,     3,     3,
       3,     3,    49,    12,    68,    69,    16,   142,    24,    25,
      26,     0,    22,    27,    12,    12,    25,    31,    32,    33,
      15,     7,    86,    37,     3,    39,    52,    15,    49,    49,
      94,    95,    96,    97,    98,    44,    50,    51,    64,    12,
      45,    55,    46,    46,    46,    16,    44,    44,    10,    44,
     114,    65,    23,   117,    80,   119,    44,   121,    11,   197,
     198,     3,   197,   198,    17,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,     3,     4,
       5,    49,     7,     8,     9,   111,    49,   113,    49,   103,
     154,   105,    14,   107,    50,   109,    50,    19,    20,    21,
      44,   165,   128,     3,     4,     5,     6,     7,     8,     9,
      15,    10,    12,    10,   128,    50,   142,    15,    10,    50,
      45,    46,    47,   149,    16,   151,    15,    51,   142,    15,
      22,    23,    51,    51,    26,    51,     3,    16,    38,    39,
      40,    10,     3,    15,    15,    45,    46,    47,    48,    15,
      15,     3,     4,     5,     6,     7,     8,     9,    15,    41,
      12,    13,   188,    15,    10,    41,    22,    23,    24,    25,
      26,   197,   198,    29,    30,    27,    28,    33,    34,     8,
      55,     8,   127,   197,   198,    -1,    38,    39,    40,    -1,
      42,    43,    -1,    45,    46,    47,    48,     3,     4,     5,
       6,     7,     8,     9,    -1,    -1,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    -1,    42,    43,    -1,    45,
      46,    47,    48,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    12,    -1,    -1,     3,     4,     5,    -1,     7,
       8,     9,    10,    11,    -1,    -1,    -1,    27,    28,    17,
      -1,    -1,    -1,    -1,    22,    23,    -1,    -1,    38,    39,
      40,    -1,    42,    43,    -1,    45,    46,    47,    48,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
       3,     4,     5,    -1,     7,     8,     9,    10,    11,    -1,
      -1,     3,     4,     5,    17,     7,     8,     9,    10,    22,
      23,     3,     4,     5,    -1,     7,     8,     9,    -1,    -1,
      22,    23,    -1,    -1,    37,     3,     4,     5,    -1,     7,
       8,     9,    45,    46,    47,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      22,    23,    24,    25,    26,    11,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    22,    23,    24,    25,
      26,    11,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    22,    23,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    15,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    22,    23,    24,    25,    26,
      -1,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      22,    23,    24,    25,    26,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    22,    23,    24,    25,    26,    -1,
      -1,    29,    30,    31,    32,    33,    34,    22,    23,    24,
      25,    26,    -1,    -1,    29,    30,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    54,    55,     0,    14,    19,    20,    21,    56,    57,
      61,     3,     4,     5,     7,     8,     9,    45,    46,    47,
      64,    65,    64,    64,     7,    57,    61,    49,     3,    46,
       3,    10,    49,    12,    60,    60,    64,    49,     3,    49,
      62,    64,    64,     3,     6,    13,    15,    27,    28,    38,
      39,    40,    42,    43,    48,    58,    59,    60,    64,    66,
      68,    50,    64,    64,    11,    17,    50,    44,    10,    10,
      15,     3,    45,    46,    64,     3,    45,    46,    64,    60,
       3,    45,    15,    13,    59,    10,    16,    22,    23,    26,
      50,    50,    60,    64,    10,    22,    23,    37,    45,    64,
      67,    67,    15,    51,     3,    51,    15,    51,     3,    51,
      25,    44,    60,     3,    63,    67,    67,    16,    22,    16,
      23,    16,    67,    67,    67,    67,    67,    10,    11,    22,
      23,    24,    25,    26,    29,    30,    31,    32,    33,    34,
      35,    36,    11,    64,    15,    64,    64,    15,    64,     3,
      60,    44,    60,    11,    17,    67,    15,    67,    15,    44,
      67,    15,    44,    67,    11,    63,    60,    64,    66,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    60,    66,    15,    15,    15,    15,    44,    60,
      60,    15,    67,    15,    15,    15,    11,    41,    41,    60,
      60,    66,    68,    60,    66,    68
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    53,    55,    54,    56,    56,    56,    56,    57,    58,
      58,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    60,    60,    61,    61,    61,    62,
      62,    62,    63,    63,    63,    63,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    65,
      65,    65,    65,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     2,     1,     2,     2,     1,
       2,     1,     1,     1,     4,     5,     5,     5,     4,     4,
       4,     4,     2,     1,     3,     2,     6,     3,     3,     0,
       1,     3,     0,     1,     3,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     5,
       5,     4,     4,     5,     2,     2,     1,     3,     4,     5,
       5,     3,     4,     5,     5,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     4,     3,     2,     5,     7,     7,     5,     7,
       7,     7,     7,     2,     4,     5,     6,     3,     5,     4,
       5,     5
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
  case 2: /* $@1: %empty  */
#line 58 "/home/ubuntu/repos/libnpa/src/parser.y"
        { pRoot = new Program; }
#line 1348 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 3: /* start: $@1 program  */
#line 58 "/home/ubuntu/repos/libnpa/src/parser.y"
                                         { }
#line 1354 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 4: /* program: func_decl  */
#line 61 "/home/ubuntu/repos/libnpa/src/parser.y"
                    { pRoot->Subroutines.push_back((yyvsp[0].subr)); }
#line 1360 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 5: /* program: program func_decl  */
#line 62 "/home/ubuntu/repos/libnpa/src/parser.y"
                            { pRoot->Subroutines.push_back((yyvsp[0].subr)); }
#line 1366 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 8: /* include: TINCLUDE TSTRING  */
#line 67 "/home/ubuntu/repos/libnpa/src/parser.y"
                           { WriteSymbol("include." + (yyvsp[0].string)->substr(1, (yyvsp[0].string)->size() - 2)); delete (yyvsp[0].string); }
#line 1372 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 9: /* stmts: stmt  */
#line 70 "/home/ubuntu/repos/libnpa/src/parser.y"
             { (yyval.block) = new Block(); (yyval.block)->Statements.push_back((yyvsp[0].stmt)); }
#line 1378 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 10: /* stmts: stmts stmt  */
#line 71 "/home/ubuntu/repos/libnpa/src/parser.y"
                   { (yyvsp[-1].block)->Statements.push_back((yyvsp[0].stmt)); }
#line 1384 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 14: /* stmt: arg TEQUAL expr TSEMICOLON  */
#line 75 "/home/ubuntu/repos/libnpa/src/parser.y"
                                  { (yyval.stmt) = new Assignment((yyvsp[-3].arg), (yyvsp[-1].expr), MAGIC_ASSIGN); }
#line 1390 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 15: /* stmt: arg TADD TEQUAL expr TSEMICOLON  */
#line 76 "/home/ubuntu/repos/libnpa/src/parser.y"
                                       { (yyval.stmt) = new Assignment((yyvsp[-4].arg), (yyvsp[-1].expr), MAGIC_ADD_ASSIGN); }
#line 1396 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 16: /* stmt: arg TSUB TEQUAL expr TSEMICOLON  */
#line 77 "/home/ubuntu/repos/libnpa/src/parser.y"
                                       { (yyval.stmt) = new Assignment((yyvsp[-4].arg), (yyvsp[-1].expr), MAGIC_SUB_ASSIGN); }
#line 1402 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 17: /* stmt: arg TMOD TEQUAL expr TSEMICOLON  */
#line 78 "/home/ubuntu/repos/libnpa/src/parser.y"
                                       { (yyval.stmt) = new Assignment((yyvsp[-4].arg), (yyvsp[-1].expr), MAGIC_MOD_ASSIGN); }
#line 1408 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 18: /* stmt: arg TADD TADD TSEMICOLON  */
#line 79 "/home/ubuntu/repos/libnpa/src/parser.y"
                                { (yyval.stmt) = new UnaryStatement(MAGIC_INCREMENT, (yyvsp[-3].arg)); }
#line 1414 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 19: /* stmt: arg TSUB TSUB TSEMICOLON  */
#line 80 "/home/ubuntu/repos/libnpa/src/parser.y"
                                { (yyval.stmt) = new UnaryStatement(MAGIC_DECREMENT, (yyvsp[-3].arg)); }
#line 1420 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 20: /* stmt: arg TADD TADD TCOLON  */
#line 81 "/home/ubuntu/repos/libnpa/src/parser.y"
                            { (yyval.stmt) = new UnaryStatement(MAGIC_INCREMENT, (yyvsp[-3].arg)); }
#line 1426 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 21: /* stmt: arg TSUB TSUB TCOLON  */
#line 82 "/home/ubuntu/repos/libnpa/src/parser.y"
                            { (yyval.stmt) = new UnaryStatement(MAGIC_DECREMENT, (yyvsp[-3].arg)); }
#line 1432 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 22: /* stmt: TIDENTIFIER TCOLON  */
#line 83 "/home/ubuntu/repos/libnpa/src/parser.y"
                          { (yyval.stmt) = new Label(*(yyvsp[-1].string)); delete (yyvsp[-1].string); }
#line 1438 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 23: /* stmt: TSEMICOLON  */
#line 84 "/home/ubuntu/repos/libnpa/src/parser.y"
                  { (yyval.stmt) = new Block(*new StatementList()); }
#line 1444 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 24: /* block: TLBRACE stmts TRBRACE  */
#line 87 "/home/ubuntu/repos/libnpa/src/parser.y"
                              { (yyval.block) = (yyvsp[-1].block); }
#line 1450 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 25: /* block: TLBRACE TRBRACE  */
#line 88 "/home/ubuntu/repos/libnpa/src/parser.y"
                        { (yyval.block) = new Block(); }
#line 1456 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 26: /* func_decl: TFUNCTION arg TLPAREN func_args TRPAREN block  */
#line 91 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                          { (yyvsp[-4].arg)->Data = string("function.") + (yyvsp[-4].arg)->Data; (yyval.subr) = new Function((yyvsp[-4].arg), *(yyvsp[-2].argvec), (yyvsp[0].block)); delete (yyvsp[-2].argvec); }
#line 1462 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 27: /* func_decl: TCHAPTER arg block  */
#line 92 "/home/ubuntu/repos/libnpa/src/parser.y"
                               { (yyvsp[-1].arg)->Data = string("chapter.") + (yyvsp[-1].arg)->Data; (yyval.subr) = new Chapter((yyvsp[-1].arg), (yyvsp[0].block)); }
#line 1468 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 28: /* func_decl: TSCENE arg block  */
#line 93 "/home/ubuntu/repos/libnpa/src/parser.y"
                             { (yyvsp[-1].arg)->Data = string("scene.") + (yyvsp[-1].arg)->Data; (yyval.subr) = new Scene((yyvsp[-1].arg), (yyvsp[0].block)); }
#line 1474 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 29: /* func_args: %empty  */
#line 96 "/home/ubuntu/repos/libnpa/src/parser.y"
            { (yyval.argvec) = new ArgumentList(); }
#line 1480 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 30: /* func_args: arg  */
#line 97 "/home/ubuntu/repos/libnpa/src/parser.y"
                { (yyval.argvec) = new ArgumentList(); (yyval.argvec)->push_back((yyvsp[0].arg)); }
#line 1486 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 31: /* func_args: func_args TCOMMA arg  */
#line 98 "/home/ubuntu/repos/libnpa/src/parser.y"
                                 { (yyvsp[-2].argvec)->push_back((yyvsp[0].arg)); }
#line 1492 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 32: /* func_exps: %empty  */
#line 101 "/home/ubuntu/repos/libnpa/src/parser.y"
            { (yyval.expvec) = new ExpressionList(); }
#line 1498 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 33: /* func_exps: expr  */
#line 102 "/home/ubuntu/repos/libnpa/src/parser.y"
                 { (yyval.expvec) = new ExpressionList(); (yyval.expvec)->push_back((yyvsp[0].expr)); }
#line 1504 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 34: /* func_exps: func_exps TCOMMA expr  */
#line 103 "/home/ubuntu/repos/libnpa/src/parser.y"
                                  { (yyvsp[-2].expvec)->push_back((yyvsp[0].expr)); }
#line 1510 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 35: /* func_exps: func_exps expr  */
#line 104 "/home/ubuntu/repos/libnpa/src/parser.y"
                           { (yyvsp[-1].expvec)->push_back((yyvsp[0].expr)); }
#line 1516 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 36: /* arg: TDOLLAR TIDENTIFIER  */
#line 107 "/home/ubuntu/repos/libnpa/src/parser.y"
                          { (yyval.arg) = new Argument('$' + *(yyvsp[0].string), ARG_VARIABLE); delete (yyvsp[0].string); }
#line 1522 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 37: /* arg: THASH TIDENTIFIER  */
#line 108 "/home/ubuntu/repos/libnpa/src/parser.y"
                          { (yyval.arg) = new Argument('#' + *(yyvsp[0].string), ARG_VARIABLE); delete (yyvsp[0].string); }
#line 1528 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 38: /* arg: THEX  */
#line 109 "/home/ubuntu/repos/libnpa/src/parser.y"
             { (yyval.arg) = new Argument(*(yyvsp[0].string), ARG_STRING); delete (yyvsp[0].string); }
#line 1534 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 39: /* arg: TIDENTIFIER  */
#line 110 "/home/ubuntu/repos/libnpa/src/parser.y"
                    { (yyval.arg) = new Argument(*(yyvsp[0].string), ARG_STRING); delete (yyvsp[0].string); }
#line 1540 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 40: /* arg: TSTRING  */
#line 111 "/home/ubuntu/repos/libnpa/src/parser.y"
                { std::string str = (yyvsp[0].string)->substr(1, (yyvsp[0].string)->size() - 2); boost::replace_all(str, "\\n", "\n"); (yyval.arg) = new Argument(str, ARG_STRING); delete (yyvsp[0].string); }
#line 1546 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 41: /* arg: TINTEGER  */
#line 112 "/home/ubuntu/repos/libnpa/src/parser.y"
                 { (yyval.arg) = new Argument(*(yyvsp[0].string), ARG_INT); delete (yyvsp[0].string); }
#line 1552 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 42: /* arg: TFLOAT  */
#line 113 "/home/ubuntu/repos/libnpa/src/parser.y"
               { (yyval.arg) = new Argument(*(yyvsp[0].string), ARG_FLOAT); delete (yyvsp[0].string); }
#line 1558 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 43: /* arg: array  */
#line 114 "/home/ubuntu/repos/libnpa/src/parser.y"
              { (yyval.arg) = (yyvsp[0].arg); }
#line 1564 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 44: /* arg: THASH  */
#line 115 "/home/ubuntu/repos/libnpa/src/parser.y"
              { (yyval.arg) = new Argument("#", ARG_STRING); }
#line 1570 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 45: /* arg: TDOLLAR  */
#line 116 "/home/ubuntu/repos/libnpa/src/parser.y"
                { (yyval.arg) = new Argument("$", ARG_STRING); }
#line 1576 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 46: /* arg: TVARNAME  */
#line 117 "/home/ubuntu/repos/libnpa/src/parser.y"
                 { (yyval.arg) = new Argument(*(yyvsp[0].string), ARG_VARIABLE); delete (yyvsp[0].string); }
#line 1582 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 47: /* arg: TDOLLAR TDOLLAR TIDENTIFIER  */
#line 118 "/home/ubuntu/repos/libnpa/src/parser.y"
                                    { (yyval.arg) = new Argument('$' + *(yyvsp[0].string), ARG_VARIABLE); delete (yyvsp[0].string); }
#line 1588 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 48: /* arg: TAT  */
#line 119 "/home/ubuntu/repos/libnpa/src/parser.y"
            { (yyval.arg) = new Argument("@", ARG_STRING); }
#line 1594 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 49: /* array: TDOLLAR TIDENTIFIER TLABRACE arg TRABRACE  */
#line 122 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                  { (yyval.arg) = new Array('$' + *(yyvsp[-3].string)); ((Array*)(yyval.arg))->Arguments.push_back((yyvsp[-1].arg)); delete (yyvsp[-3].string); }
#line 1600 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 50: /* array: THASH TIDENTIFIER TLABRACE arg TRABRACE  */
#line 123 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                { (yyval.arg) = new Array('#' + *(yyvsp[-3].string)); ((Array*)(yyval.arg))->Arguments.push_back((yyvsp[-1].arg)); delete (yyvsp[-3].string); }
#line 1606 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 51: /* array: array TLABRACE arg TRABRACE  */
#line 124 "/home/ubuntu/repos/libnpa/src/parser.y"
                                    { (yyval.arg) = (yyvsp[-3].arg); ((Array*)(yyval.arg))->Arguments.push_back((yyvsp[-1].arg)); }
#line 1612 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 52: /* array: TVARNAME TLABRACE arg TRABRACE  */
#line 125 "/home/ubuntu/repos/libnpa/src/parser.y"
                                       { (yyval.arg) = new Array(*(yyvsp[-3].string)); ((Array*)(yyval.arg))->Arguments.push_back((yyvsp[-1].arg)); delete (yyvsp[-3].string); }
#line 1618 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 53: /* call: arg TLPAREN func_exps TRPAREN TSEMICOLON  */
#line 127 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                { (yyval.stmt) = new CallStatement((yyvsp[-4].arg), *(yyvsp[-2].expvec), MAGIC_CALL_FUNCTION); delete (yyvsp[-2].expvec); }
#line 1624 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 54: /* call: TRETURN TSEMICOLON  */
#line 129 "/home/ubuntu/repos/libnpa/src/parser.y"
          {
               ExpressionList Args;
               Argument* Arg = new Argument("Return", ARG_FUNCTION);
               (yyval.stmt) = new Call(Arg, Args, 0);
          }
#line 1634 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 55: /* call: TBREAK TSEMICOLON  */
#line 135 "/home/ubuntu/repos/libnpa/src/parser.y"
          {
               ExpressionList Args;
               Argument* Arg = new Argument("Break", ARG_FUNCTION);
               (yyval.stmt) = new Call(Arg, Args, 0);
          }
#line 1644 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 56: /* call: TXML  */
#line 140 "/home/ubuntu/repos/libnpa/src/parser.y"
            {
               size_t i = (yyvsp[0].string)->find("<PRE ") + 5;
               size_t j = (yyvsp[0].string)->find(">", i);
               size_t k = (yyvsp[0].string)->find("[", j + 1);
               size_t l = k == std::string::npos ? std::string::npos : (yyvsp[0].string)->find("]", k + 1);
               ExpressionList Args;
               if (k != std::string::npos && l != std::string::npos)
                    Args.push_back(new Argument((yyvsp[0].string)->substr(k + 1, l - k - 1), ARG_STRING));
               else
                    Args.push_back(new Argument("", ARG_STRING));
               Args.push_back(new Argument(j != std::string::npos ? (yyvsp[0].string)->substr(i, j - i) : *(yyvsp[0].string), ARG_STRING));
               Args.push_back(new Argument(*(yyvsp[0].string), ARG_STRING));
               Argument* Arg = new Argument("ParseText", ARG_FUNCTION);
               (yyval.stmt) = new Call(Arg, Args, 0);
               delete (yyvsp[0].string);
            }
#line 1665 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 57: /* call: TCALLCHAPTER TIDENTIFIER TSEMICOLON  */
#line 156 "/home/ubuntu/repos/libnpa/src/parser.y"
                                           { (yyval.stmt) = MakeCall(*(yyvsp[-1].string), MAGIC_CALL_CHAPTER); delete (yyvsp[-1].string); }
#line 1671 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 58: /* call: TCALLCHAPTER TDOLLAR TIDENTIFIER TSEMICOLON  */
#line 157 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                   { (yyval.stmt) = MakeCall('$' + *(yyvsp[-1].string), MAGIC_CALL_CHAPTER); delete (yyvsp[-1].string); }
#line 1677 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 59: /* call: TCALLCHAPTER arg TARROW arg TSEMICOLON  */
#line 158 "/home/ubuntu/repos/libnpa/src/parser.y"
                                              { (yyval.stmt) = MakeCall((yyvsp[-3].arg)->Data + "->" + (yyvsp[-1].arg)->Data, MAGIC_CALL_CHAPTER); }
#line 1683 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 60: /* call: TCALLCHAPTER TAT TARROW arg TSEMICOLON  */
#line 159 "/home/ubuntu/repos/libnpa/src/parser.y"
                                              { (yyval.stmt) = MakeCall("@->" + (yyvsp[-1].arg)->Data, MAGIC_CALL_CHAPTER); }
#line 1689 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 61: /* call: TCALLSCENE TIDENTIFIER TSEMICOLON  */
#line 160 "/home/ubuntu/repos/libnpa/src/parser.y"
                                         { (yyval.stmt) = MakeCall(*(yyvsp[-1].string), MAGIC_CALL_SCENE); delete (yyvsp[-1].string); }
#line 1695 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 62: /* call: TCALLSCENE TDOLLAR TIDENTIFIER TSEMICOLON  */
#line 161 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                 { (yyval.stmt) = MakeCall('$' + *(yyvsp[-1].string), MAGIC_CALL_SCENE); delete (yyvsp[-1].string); }
#line 1701 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 63: /* call: TCALLSCENE arg TARROW arg TSEMICOLON  */
#line 162 "/home/ubuntu/repos/libnpa/src/parser.y"
                                            { (yyval.stmt) = MakeCall((yyvsp[-3].arg)->Data + "->" + (yyvsp[-1].arg)->Data, MAGIC_CALL_SCENE); }
#line 1707 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 64: /* call: TCALLSCENE TAT TARROW arg TSEMICOLON  */
#line 163 "/home/ubuntu/repos/libnpa/src/parser.y"
                                            { (yyval.stmt) = MakeCall("@->" + (yyvsp[-1].arg)->Data, MAGIC_CALL_SCENE); }
#line 1713 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 65: /* expr: arg  */
#line 166 "/home/ubuntu/repos/libnpa/src/parser.y"
           { (yyval.arg) = (yyvsp[0].arg); }
#line 1719 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 66: /* expr: expr TMUL expr  */
#line 167 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_MUL_EXPRESSION, (yyvsp[0].expr)); }
#line 1725 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 67: /* expr: expr TDIV expr  */
#line 168 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_DIV_EXPRESSION, (yyvsp[0].expr)); }
#line 1731 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 68: /* expr: expr TMOD expr  */
#line 169 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_MOD_EXPRESSION, (yyvsp[0].expr)); }
#line 1737 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 69: /* expr: expr TADD expr  */
#line 170 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_ADD_EXPRESSION, (yyvsp[0].expr)); }
#line 1743 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 70: /* expr: expr TSUB expr  */
#line 171 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_SUB_EXPRESSION, (yyvsp[0].expr)); }
#line 1749 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 71: /* expr: expr TLESS expr  */
#line 172 "/home/ubuntu/repos/libnpa/src/parser.y"
                       { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_LESS, (yyvsp[0].expr)); }
#line 1755 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 72: /* expr: expr TGREATER expr  */
#line 173 "/home/ubuntu/repos/libnpa/src/parser.y"
                          { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_GREATER, (yyvsp[0].expr)); }
#line 1761 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 73: /* expr: expr TEQUALEQUAL expr  */
#line 174 "/home/ubuntu/repos/libnpa/src/parser.y"
                             { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_EQUAL, (yyvsp[0].expr)); }
#line 1767 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 74: /* expr: expr TNEQUAL expr  */
#line 175 "/home/ubuntu/repos/libnpa/src/parser.y"
                         { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_NE, (yyvsp[0].expr)); }
#line 1773 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 75: /* expr: expr TGEQUAL expr  */
#line 176 "/home/ubuntu/repos/libnpa/src/parser.y"
                         { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_GE, (yyvsp[0].expr)); }
#line 1779 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 76: /* expr: expr TLEQUAL expr  */
#line 177 "/home/ubuntu/repos/libnpa/src/parser.y"
                         { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_LE, (yyvsp[0].expr)); }
#line 1785 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 77: /* expr: expr TAND expr  */
#line 178 "/home/ubuntu/repos/libnpa/src/parser.y"
                      { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_LOGICAL_AND, (yyvsp[0].expr)); }
#line 1791 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 78: /* expr: expr TOR expr  */
#line 179 "/home/ubuntu/repos/libnpa/src/parser.y"
                     { (yyval.expr) = new BinaryOperator((yyvsp[-2].expr), MAGIC_CMP_LOGICAL_OR, (yyvsp[0].expr)); }
#line 1797 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 79: /* expr: TNOT expr  */
#line 180 "/home/ubuntu/repos/libnpa/src/parser.y"
                             { (yyval.expr) = new UnaryOperator(MAGIC_NOT_EXPRESSION, (yyvsp[0].expr)); }
#line 1803 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 80: /* expr: TSUB expr  */
#line 181 "/home/ubuntu/repos/libnpa/src/parser.y"
                             { (yyval.expr) = new UnaryOperator(MAGIC_NEGA_EXPRESSION, (yyvsp[0].expr)); }
#line 1809 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 81: /* expr: TAT expr  */
#line 182 "/home/ubuntu/repos/libnpa/src/parser.y"
                            { (yyval.expr) = new UnaryOperator(MAGIC_AT_EXPRESSION, (yyvsp[0].expr)); }
#line 1815 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 82: /* expr: arg TLPAREN func_exps TRPAREN  */
#line 183 "/home/ubuntu/repos/libnpa/src/parser.y"
                                     { (yyval.expr) = new Call((yyvsp[-3].arg), *(yyvsp[-1].expvec), MAGIC_CALL_FUNCTION); delete (yyvsp[-1].expvec); }
#line 1821 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 83: /* expr: TLPAREN expr TRPAREN  */
#line 184 "/home/ubuntu/repos/libnpa/src/parser.y"
                            { (yyval.expr) = (yyvsp[-1].expr); }
#line 1827 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 84: /* expr: TADD expr  */
#line 185 "/home/ubuntu/repos/libnpa/src/parser.y"
                             { (yyval.expr) = (yyvsp[0].expr); }
#line 1833 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 85: /* cond: TIF TLPAREN expr TRPAREN block  */
#line 188 "/home/ubuntu/repos/libnpa/src/parser.y"
                                      { (yyval.stmt) = new If((yyvsp[0].block), (yyvsp[-2].expr)); }
#line 1839 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 86: /* cond: TIF TLPAREN expr TRPAREN block TELSE cond  */
#line 189 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                 { (yyval.stmt) = new If((yyvsp[-2].block), (yyvsp[-4].expr), (yyvsp[0].stmt)); }
#line 1845 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 87: /* cond: TIF TLPAREN expr TRPAREN block TELSE block  */
#line 190 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                  { (yyval.stmt) = new If((yyvsp[-2].block), (yyvsp[-4].expr), (yyvsp[0].block)); }
#line 1851 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 88: /* cond: TIF TLPAREN expr TRPAREN call  */
#line 191 "/home/ubuntu/repos/libnpa/src/parser.y"
                                     { (yyval.stmt) = new If(WrapStmt((yyvsp[0].stmt)), (yyvsp[-2].expr)); }
#line 1857 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 89: /* cond: TIF TLPAREN expr TRPAREN block TELSE call  */
#line 192 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                 { (yyval.stmt) = new If((yyvsp[-2].block), (yyvsp[-4].expr), WrapStmt((yyvsp[0].stmt))); }
#line 1863 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 90: /* cond: TIF TLPAREN expr TRPAREN call TELSE call  */
#line 193 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                { (yyval.stmt) = new If(WrapStmt((yyvsp[-2].stmt)), (yyvsp[-4].expr), WrapStmt((yyvsp[0].stmt))); }
#line 1869 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 91: /* cond: TIF TLPAREN expr TRPAREN call TELSE cond  */
#line 194 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                { (yyval.stmt) = new If(WrapStmt((yyvsp[-2].stmt)), (yyvsp[-4].expr), (yyvsp[0].stmt)); }
#line 1875 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 92: /* cond: TIF TLPAREN expr TRPAREN call TELSE block  */
#line 195 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                 { (yyval.stmt) = new If(WrapStmt((yyvsp[-2].stmt)), (yyvsp[-4].expr), (yyvsp[0].block)); }
#line 1881 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 93: /* cond: TSELECT block  */
#line 196 "/home/ubuntu/repos/libnpa/src/parser.y"
                     { (yyval.stmt) = new Select((yyvsp[0].block)); }
#line 1887 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 94: /* cond: TCASE TIDENTIFIER TCOLON block  */
#line 197 "/home/ubuntu/repos/libnpa/src/parser.y"
                                      { (yyval.stmt) = new Case(*(yyvsp[-2].string), (yyvsp[0].block)); delete (yyvsp[-2].string); }
#line 1893 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 95: /* cond: TCASE TIDENTIFIER TDIV TIDENTIFIER block  */
#line 198 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                { (yyval.stmt) = new Case(*(yyvsp[-3].string) + "/" + *(yyvsp[-1].string), (yyvsp[0].block)); delete (yyvsp[-3].string); delete (yyvsp[-1].string); }
#line 1899 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 96: /* cond: TCASE TIDENTIFIER TDIV TIDENTIFIER TCOLON block  */
#line 199 "/home/ubuntu/repos/libnpa/src/parser.y"
                                                       { (yyval.stmt) = new Case(*(yyvsp[-4].string) + "/" + *(yyvsp[-2].string), (yyvsp[0].block)); delete (yyvsp[-4].string); delete (yyvsp[-2].string); }
#line 1905 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 97: /* cond: TCASE TIDENTIFIER block  */
#line 200 "/home/ubuntu/repos/libnpa/src/parser.y"
                               { (yyval.stmt) = new Case(*(yyvsp[-1].string), (yyvsp[0].block)); delete (yyvsp[-1].string); }
#line 1911 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 98: /* cond: TCASE TAT TIDENTIFIER TCOLON block  */
#line 201 "/home/ubuntu/repos/libnpa/src/parser.y"
                                          { (yyval.stmt) = new Case("@" + *(yyvsp[-2].string), (yyvsp[0].block)); delete (yyvsp[-2].string); }
#line 1917 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 99: /* cond: TCASE TAT TIDENTIFIER block  */
#line 202 "/home/ubuntu/repos/libnpa/src/parser.y"
                                   { (yyval.stmt) = new Case("@" + *(yyvsp[-1].string), (yyvsp[0].block)); delete (yyvsp[-1].string); }
#line 1923 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 100: /* cond: TWHILE TLPAREN expr TRPAREN block  */
#line 203 "/home/ubuntu/repos/libnpa/src/parser.y"
                                         { (yyval.stmt) = new While((yyvsp[0].block), (yyvsp[-2].expr)); }
#line 1929 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;

  case 101: /* cond: TWHILE TLPAREN expr TRPAREN call  */
#line 204 "/home/ubuntu/repos/libnpa/src/parser.y"
                                        { (yyval.stmt) = new While(WrapStmt((yyvsp[0].stmt)), (yyvsp[-2].expr)); }
#line 1935 "/home/ubuntu/repos/libnpa/src/parser.cpp"
    break;


#line 1939 "/home/ubuntu/repos/libnpa/src/parser.cpp"

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

#line 207 "/home/ubuntu/repos/libnpa/src/parser.y"

