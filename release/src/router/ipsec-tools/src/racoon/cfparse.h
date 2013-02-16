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

#ifndef YY_CFPARSE_H
# define YY_CFPARSE_H
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
     PRIVSEP = 258,
     USER = 259,
     GROUP = 260,
     CHROOT = 261,
     PATH = 262,
     PATHTYPE = 263,
     INCLUDE = 264,
     PFKEY_BUFFER = 265,
     LOGGING = 266,
     LOGLEV = 267,
     PADDING = 268,
     PAD_RANDOMIZE = 269,
     PAD_RANDOMIZELEN = 270,
     PAD_MAXLEN = 271,
     PAD_STRICT = 272,
     PAD_EXCLTAIL = 273,
     LISTEN = 274,
     X_ISAKMP = 275,
     X_ISAKMP_NATT = 276,
     X_ADMIN = 277,
     STRICT_ADDRESS = 278,
     ADMINSOCK = 279,
     DISABLED = 280,
     LDAPCFG = 281,
     LDAP_HOST = 282,
     LDAP_PORT = 283,
     LDAP_PVER = 284,
     LDAP_BASE = 285,
     LDAP_BIND_DN = 286,
     LDAP_BIND_PW = 287,
     LDAP_SUBTREE = 288,
     LDAP_ATTR_USER = 289,
     LDAP_ATTR_ADDR = 290,
     LDAP_ATTR_MASK = 291,
     LDAP_ATTR_GROUP = 292,
     LDAP_ATTR_MEMBER = 293,
     RADCFG = 294,
     RAD_AUTH = 295,
     RAD_ACCT = 296,
     RAD_TIMEOUT = 297,
     RAD_RETRIES = 298,
     MODECFG = 299,
     CFG_NET4 = 300,
     CFG_MASK4 = 301,
     CFG_DNS4 = 302,
     CFG_NBNS4 = 303,
     CFG_DEFAULT_DOMAIN = 304,
     CFG_AUTH_SOURCE = 305,
     CFG_AUTH_GROUPS = 306,
     CFG_SYSTEM = 307,
     CFG_RADIUS = 308,
     CFG_PAM = 309,
     CFG_LDAP = 310,
     CFG_LOCAL = 311,
     CFG_NONE = 312,
     CFG_GROUP_SOURCE = 313,
     CFG_ACCOUNTING = 314,
     CFG_CONF_SOURCE = 315,
     CFG_MOTD = 316,
     CFG_POOL_SIZE = 317,
     CFG_AUTH_THROTTLE = 318,
     CFG_SPLIT_NETWORK = 319,
     CFG_SPLIT_LOCAL = 320,
     CFG_SPLIT_INCLUDE = 321,
     CFG_SPLIT_DNS = 322,
     CFG_PFS_GROUP = 323,
     CFG_SAVE_PASSWD = 324,
     RETRY = 325,
     RETRY_COUNTER = 326,
     RETRY_INTERVAL = 327,
     RETRY_PERSEND = 328,
     RETRY_PHASE1 = 329,
     RETRY_PHASE2 = 330,
     NATT_KA = 331,
     ALGORITHM_CLASS = 332,
     ALGORITHMTYPE = 333,
     STRENGTHTYPE = 334,
     SAINFO = 335,
     FROM = 336,
     REMOTE = 337,
     ANONYMOUS = 338,
     CLIENTADDR = 339,
     INHERIT = 340,
     REMOTE_ADDRESS = 341,
     EXCHANGE_MODE = 342,
     EXCHANGETYPE = 343,
     DOI = 344,
     DOITYPE = 345,
     SITUATION = 346,
     SITUATIONTYPE = 347,
     CERTIFICATE_TYPE = 348,
     CERTTYPE = 349,
     PEERS_CERTFILE = 350,
     CA_TYPE = 351,
     VERIFY_CERT = 352,
     SEND_CERT = 353,
     SEND_CR = 354,
     MATCH_EMPTY_CR = 355,
     IDENTIFIERTYPE = 356,
     IDENTIFIERQUAL = 357,
     MY_IDENTIFIER = 358,
     PEERS_IDENTIFIER = 359,
     VERIFY_IDENTIFIER = 360,
     DNSSEC = 361,
     CERT_X509 = 362,
     CERT_PLAINRSA = 363,
     NONCE_SIZE = 364,
     DH_GROUP = 365,
     KEEPALIVE = 366,
     PASSIVE = 367,
     INITIAL_CONTACT = 368,
     NAT_TRAVERSAL = 369,
     REMOTE_FORCE_LEVEL = 370,
     PROPOSAL_CHECK = 371,
     PROPOSAL_CHECK_LEVEL = 372,
     GENERATE_POLICY = 373,
     GENERATE_LEVEL = 374,
     SUPPORT_PROXY = 375,
     PROPOSAL = 376,
     EXEC_PATH = 377,
     EXEC_COMMAND = 378,
     EXEC_SUCCESS = 379,
     EXEC_FAILURE = 380,
     GSS_ID = 381,
     GSS_ID_ENC = 382,
     GSS_ID_ENCTYPE = 383,
     COMPLEX_BUNDLE = 384,
     DPD = 385,
     DPD_DELAY = 386,
     DPD_RETRY = 387,
     DPD_MAXFAIL = 388,
     PH1ID = 389,
     XAUTH_LOGIN = 390,
     WEAK_PHASE1_CHECK = 391,
     REKEY = 392,
     PREFIX = 393,
     PORT = 394,
     PORTANY = 395,
     UL_PROTO = 396,
     ANY = 397,
     IKE_FRAG = 398,
     ESP_FRAG = 399,
     MODE_CFG = 400,
     PFS_GROUP = 401,
     LIFETIME = 402,
     LIFETYPE_TIME = 403,
     LIFETYPE_BYTE = 404,
     STRENGTH = 405,
     REMOTEID = 406,
     SCRIPT = 407,
     PHASE1_UP = 408,
     PHASE1_DOWN = 409,
     PHASE1_DEAD = 410,
     NUMBER = 411,
     SWITCH = 412,
     BOOLEAN = 413,
     HEXSTRING = 414,
     QUOTEDSTRING = 415,
     ADDRSTRING = 416,
     ADDRRANGE = 417,
     UNITTYPE_BYTE = 418,
     UNITTYPE_KBYTES = 419,
     UNITTYPE_MBYTES = 420,
     UNITTYPE_TBYTES = 421,
     UNITTYPE_SEC = 422,
     UNITTYPE_MIN = 423,
     UNITTYPE_HOUR = 424,
     EOS = 425,
     BOC = 426,
     EOC = 427,
     COMMA = 428
   };
#endif
/* Tokens.  */
#define PRIVSEP 258
#define USER 259
#define GROUP 260
#define CHROOT 261
#define PATH 262
#define PATHTYPE 263
#define INCLUDE 264
#define PFKEY_BUFFER 265
#define LOGGING 266
#define LOGLEV 267
#define PADDING 268
#define PAD_RANDOMIZE 269
#define PAD_RANDOMIZELEN 270
#define PAD_MAXLEN 271
#define PAD_STRICT 272
#define PAD_EXCLTAIL 273
#define LISTEN 274
#define X_ISAKMP 275
#define X_ISAKMP_NATT 276
#define X_ADMIN 277
#define STRICT_ADDRESS 278
#define ADMINSOCK 279
#define DISABLED 280
#define LDAPCFG 281
#define LDAP_HOST 282
#define LDAP_PORT 283
#define LDAP_PVER 284
#define LDAP_BASE 285
#define LDAP_BIND_DN 286
#define LDAP_BIND_PW 287
#define LDAP_SUBTREE 288
#define LDAP_ATTR_USER 289
#define LDAP_ATTR_ADDR 290
#define LDAP_ATTR_MASK 291
#define LDAP_ATTR_GROUP 292
#define LDAP_ATTR_MEMBER 293
#define RADCFG 294
#define RAD_AUTH 295
#define RAD_ACCT 296
#define RAD_TIMEOUT 297
#define RAD_RETRIES 298
#define MODECFG 299
#define CFG_NET4 300
#define CFG_MASK4 301
#define CFG_DNS4 302
#define CFG_NBNS4 303
#define CFG_DEFAULT_DOMAIN 304
#define CFG_AUTH_SOURCE 305
#define CFG_AUTH_GROUPS 306
#define CFG_SYSTEM 307
#define CFG_RADIUS 308
#define CFG_PAM 309
#define CFG_LDAP 310
#define CFG_LOCAL 311
#define CFG_NONE 312
#define CFG_GROUP_SOURCE 313
#define CFG_ACCOUNTING 314
#define CFG_CONF_SOURCE 315
#define CFG_MOTD 316
#define CFG_POOL_SIZE 317
#define CFG_AUTH_THROTTLE 318
#define CFG_SPLIT_NETWORK 319
#define CFG_SPLIT_LOCAL 320
#define CFG_SPLIT_INCLUDE 321
#define CFG_SPLIT_DNS 322
#define CFG_PFS_GROUP 323
#define CFG_SAVE_PASSWD 324
#define RETRY 325
#define RETRY_COUNTER 326
#define RETRY_INTERVAL 327
#define RETRY_PERSEND 328
#define RETRY_PHASE1 329
#define RETRY_PHASE2 330
#define NATT_KA 331
#define ALGORITHM_CLASS 332
#define ALGORITHMTYPE 333
#define STRENGTHTYPE 334
#define SAINFO 335
#define FROM 336
#define REMOTE 337
#define ANONYMOUS 338
#define CLIENTADDR 339
#define INHERIT 340
#define REMOTE_ADDRESS 341
#define EXCHANGE_MODE 342
#define EXCHANGETYPE 343
#define DOI 344
#define DOITYPE 345
#define SITUATION 346
#define SITUATIONTYPE 347
#define CERTIFICATE_TYPE 348
#define CERTTYPE 349
#define PEERS_CERTFILE 350
#define CA_TYPE 351
#define VERIFY_CERT 352
#define SEND_CERT 353
#define SEND_CR 354
#define MATCH_EMPTY_CR 355
#define IDENTIFIERTYPE 356
#define IDENTIFIERQUAL 357
#define MY_IDENTIFIER 358
#define PEERS_IDENTIFIER 359
#define VERIFY_IDENTIFIER 360
#define DNSSEC 361
#define CERT_X509 362
#define CERT_PLAINRSA 363
#define NONCE_SIZE 364
#define DH_GROUP 365
#define KEEPALIVE 366
#define PASSIVE 367
#define INITIAL_CONTACT 368
#define NAT_TRAVERSAL 369
#define REMOTE_FORCE_LEVEL 370
#define PROPOSAL_CHECK 371
#define PROPOSAL_CHECK_LEVEL 372
#define GENERATE_POLICY 373
#define GENERATE_LEVEL 374
#define SUPPORT_PROXY 375
#define PROPOSAL 376
#define EXEC_PATH 377
#define EXEC_COMMAND 378
#define EXEC_SUCCESS 379
#define EXEC_FAILURE 380
#define GSS_ID 381
#define GSS_ID_ENC 382
#define GSS_ID_ENCTYPE 383
#define COMPLEX_BUNDLE 384
#define DPD 385
#define DPD_DELAY 386
#define DPD_RETRY 387
#define DPD_MAXFAIL 388
#define PH1ID 389
#define XAUTH_LOGIN 390
#define WEAK_PHASE1_CHECK 391
#define REKEY 392
#define PREFIX 393
#define PORT 394
#define PORTANY 395
#define UL_PROTO 396
#define ANY 397
#define IKE_FRAG 398
#define ESP_FRAG 399
#define MODE_CFG 400
#define PFS_GROUP 401
#define LIFETIME 402
#define LIFETYPE_TIME 403
#define LIFETYPE_BYTE 404
#define STRENGTH 405
#define REMOTEID 406
#define SCRIPT 407
#define PHASE1_UP 408
#define PHASE1_DOWN 409
#define PHASE1_DEAD 410
#define NUMBER 411
#define SWITCH 412
#define BOOLEAN 413
#define HEXSTRING 414
#define QUOTEDSTRING 415
#define ADDRSTRING 416
#define ADDRRANGE 417
#define UNITTYPE_BYTE 418
#define UNITTYPE_KBYTES 419
#define UNITTYPE_MBYTES 420
#define UNITTYPE_TBYTES 421
#define UNITTYPE_SEC 422
#define UNITTYPE_MIN 423
#define UNITTYPE_HOUR 424
#define EOS 425
#define BOC 426
#define EOC 427
#define COMMA 428



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2049 of yacc.c  */
#line 247 "cfparse.y"

	unsigned long num;
	vchar_t *val;
	struct remoteconf *rmconf;
	struct sockaddr *saddr;
	struct sainfoalg *alg;


/* Line 2049 of yacc.c  */
#line 412 "cfparse.h"
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

#endif /* !YY_CFPARSE_H  */
