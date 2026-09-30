/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_HOME_UBUNTU_REPOS_LIBNPA_SRC_PARSER_HPP_INCLUDED
# define YY_YY_HOME_UBUNTU_REPOS_LIBNPA_SRC_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TIDENTIFIER = 258,             /* TIDENTIFIER  */
    TFLOAT = 259,                  /* TFLOAT  */
    TINTEGER = 260,                /* TINTEGER  */
    TXML = 261,                    /* TXML  */
    TSTRING = 262,                 /* TSTRING  */
    THEX = 263,                    /* THEX  */
    TVARNAME = 264,                /* TVARNAME  */
    TLPAREN = 265,                 /* TLPAREN  */
    TRPAREN = 266,                 /* TRPAREN  */
    TLBRACE = 267,                 /* TLBRACE  */
    TRBRACE = 268,                 /* TRBRACE  */
    TFUNCTION = 269,               /* TFUNCTION  */
    TSEMICOLON = 270,              /* TSEMICOLON  */
    TEQUAL = 271,                  /* TEQUAL  */
    TCOMMA = 272,                  /* TCOMMA  */
    TQUOTE = 273,                  /* TQUOTE  */
    TCHAPTER = 274,                /* TCHAPTER  */
    TSCENE = 275,                  /* TSCENE  */
    TINCLUDE = 276,                /* TINCLUDE  */
    TADD = 277,                    /* TADD  */
    TSUB = 278,                    /* TSUB  */
    TMUL = 279,                    /* TMUL  */
    TDIV = 280,                    /* TDIV  */
    TMOD = 281,                    /* TMOD  */
    TIF = 282,                     /* TIF  */
    TWHILE = 283,                  /* TWHILE  */
    TLESS = 284,                   /* TLESS  */
    TGREATER = 285,                /* TGREATER  */
    TEQUALEQUAL = 286,             /* TEQUALEQUAL  */
    TNEQUAL = 287,                 /* TNEQUAL  */
    TGEQUAL = 288,                 /* TGEQUAL  */
    TLEQUAL = 289,                 /* TLEQUAL  */
    TAND = 290,                    /* TAND  */
    TOR = 291,                     /* TOR  */
    TNOT = 292,                    /* TNOT  */
    TRETURN = 293,                 /* TRETURN  */
    TCALLCHAPTER = 294,            /* TCALLCHAPTER  */
    TCALLSCENE = 295,              /* TCALLSCENE  */
    TELSE = 296,                   /* TELSE  */
    TSELECT = 297,                 /* TSELECT  */
    TCASE = 298,                   /* TCASE  */
    TCOLON = 299,                  /* TCOLON  */
    TAT = 300,                     /* TAT  */
    TDOLLAR = 301,                 /* TDOLLAR  */
    THASH = 302,                   /* THASH  */
    TBREAK = 303,                  /* TBREAK  */
    TLABRACE = 304,                /* TLABRACE  */
    TRABRACE = 305,                /* TRABRACE  */
    TARROW = 306,                  /* TARROW  */
    UNARY = 307                    /* UNARY  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "/home/ubuntu/repos/libnpa/src/parser.y"

    Node* node;
    Program* program;
    Block* block;
    Statement* stmt;
    Argument* arg;
    Expression* expr;
    Subroutine* subr;
    std::vector<Argument*>* argvec;
    std::vector<Expression*>* expvec;
    std::string* string;
    int token;

#line 130 "/home/ubuntu/repos/libnpa/src/parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_HOME_UBUNTU_REPOS_LIBNPA_SRC_PARSER_HPP_INCLUDED  */
