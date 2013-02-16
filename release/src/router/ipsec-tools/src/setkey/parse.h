/* A Bison parser, made by GNU Bison 2.6.2.  */

/* Bison interface for Yacc-like parsers in C
   
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

#ifndef YY_PARSE_H
# define YY_PARSE_H
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
/* Line 2049 of yacc.c  */
#line 110 "parse.y"

	int num;
	unsigned long ulnum;
	vchar_t val;
	struct addrinfo *res;


/* Line 2049 of yacc.c  */
#line 179 "parse.h"
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

#endif /* !YY_PARSE_H  */
