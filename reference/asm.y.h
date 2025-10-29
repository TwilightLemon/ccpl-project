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

#ifndef YY_YY_ASM_Y_H_INCLUDED
# define YY_YY_ASM_Y_H_INCLUDED
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
    ADD = 258,                     /* ADD  */
    SUB = 259,                     /* SUB  */
    MUL = 260,                     /* MUL  */
    DIV = 261,                     /* DIV  */
    TST = 262,                     /* TST  */
    STO = 263,                     /* STO  */
    STC = 264,                     /* STC  */
    LOD = 265,                     /* LOD  */
    LDC = 266,                     /* LDC  */
    JMP = 267,                     /* JMP  */
    JEZ = 268,                     /* JEZ  */
    JLZ = 269,                     /* JLZ  */
    JGZ = 270,                     /* JGZ  */
    DBN = 271,                     /* DBN  */
    DBS = 272,                     /* DBS  */
    ITC = 273,                     /* ITC  */
    ITI = 274,                     /* ITI  */
    OTC = 275,                     /* OTC  */
    OTI = 276,                     /* OTI  */
    OTS = 277,                     /* OTS  */
    NOP = 278,                     /* NOP  */
    END = 279,                     /* END  */
    INTEGER = 280,                 /* INTEGER  */
    REG = 281,                     /* REG  */
    LABEL = 282                    /* LABEL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define ADD 258
#define SUB 259
#define MUL 260
#define DIV 261
#define TST 262
#define STO 263
#define STC 264
#define LOD 265
#define LDC 266
#define JMP 267
#define JEZ 268
#define JLZ 269
#define JGZ 270
#define DBN 271
#define DBS 272
#define ITC 273
#define ITI 274
#define OTC 275
#define OTI 276
#define OTS 277
#define NOP 278
#define END 279
#define INTEGER 280
#define REG 281
#define LABEL 282

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 78 "asm.y"

	int number;
	char *string;

#line 126 "asm.y.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_ASM_Y_H_INCLUDED  */
