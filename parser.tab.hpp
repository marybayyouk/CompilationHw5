/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     BYTE = 259,
     B = 260,
     BOOL = 261,
     TRUE = 262,
     FALSE = 263,
     IF = 264,
     RETURN = 265,
     WHILE = 266,
     BREAK = 267,
     CONTINUE = 268,
     SC = 269,
     ID = 270,
     STRING = 271,
     NUM = 272,
     ASSIGN = 273,
     OR = 274,
     AND = 275,
     GE = 276,
     GT = 277,
     LE = 278,
     LT = 279,
     EQ = 280,
     NE = 281,
     SUB = 282,
     ADD = 283,
     DIVIDE = 284,
     MULTIPLY = 285,
     NOT = 286,
     LPAREN = 287,
     RPAREN = 288,
     LBRACE = 289,
     RBRACE = 290,
     ELSE = 291
   };
#endif
/* Tokens.  */
#define INT 258
#define BYTE 259
#define B 260
#define BOOL 261
#define TRUE 262
#define FALSE 263
#define IF 264
#define RETURN 265
#define WHILE 266
#define BREAK 267
#define CONTINUE 268
#define SC 269
#define ID 270
#define STRING 271
#define NUM 272
#define ASSIGN 273
#define OR 274
#define AND 275
#define GE 276
#define GT 277
#define LE 278
#define LT 279
#define EQ 280
#define NE 281
#define SUB 282
#define ADD 283
#define DIVIDE 284
#define MULTIPLY 285
#define NOT 286
#define LPAREN 287
#define RPAREN 288
#define LBRACE 289
#define RBRACE 290
#define ELSE 291




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

