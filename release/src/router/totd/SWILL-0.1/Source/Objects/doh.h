/* -----------------------------------------------------------------------------
 * doh.h
 *
 *     This file describes of the externally visible functions in DOH.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 *
 * $Header: /home/pastacvs/cvs-rep/vermicelli/tot/src/SWILL-0.1/Source/Objects/doh.h,v 1.1 2005/01/27 08:46:25 dillema Exp $
 * ----------------------------------------------------------------------------- */

#ifndef _DOH_H
#define _DOH_H

/* Set the namespace prefix for DOH API functions. This can be used to control
   visibility of the functions in libraries */

/* Set this macro if you want to change DOH linkage. You would do this if you
   wanted to hide DOH in a library using a different set of names.  Note: simply
   change "Doh" to a new name. */


/* All of these functions are private in SWILL.  Give 'em a really odd prefix. */

#define DOH_NAMESPACE(x) _swilL ## x

#ifdef DOH_NAMESPACE

/* Namespace control.  These macros define all of the public API names in DOH */

#define DohCheck           DOH_NAMESPACE(Check)
#define DohIntern          DOH_NAMESPACE(Intern)
#define DohDelete          DOH_NAMESPACE(Delete)
#define DohCopy            DOH_NAMESPACE(Copy)
#define DohClear           DOH_NAMESPACE(Clear)
#define DohStr             DOH_NAMESPACE(Str)
#define DohData            DOH_NAMESPACE(Data)
#define DohDump            DOH_NAMESPACE(Dump)
#define DohLen             DOH_NAMESPACE(Len)
#define DohHashval         DOH_NAMESPACE(Hashval)
#define DohCmp             DOH_NAMESPACE(Cmp)
#define DohIncref          DOH_NAMESPACE(Incref)
#define DohGetattr         DOH_NAMESPACE(Getattr)
#define DohSetattr         DOH_NAMESPACE(Setattr)
#define DohDelattr         DOH_NAMESPACE(Delattr)
#define DohFirstkey        DOH_NAMESPACE(Firstkey)
#define DohNextkey         DOH_NAMESPACE(Nextkey)
#define DohKeys            DOH_NAMESPACE(Keys)
#define DohGetInt          DOH_NAMESPACE(GetInt)
#define DohGetDouble       DOH_NAMESPACE(GetDouble)
#define DohGetChar         DOH_NAMESPACE(GetChar)
#define DohSetChar         DOH_NAMESPACE(SetChar)
#define DohSetInt          DOH_NAMESPACE(SetInt)
#define DohSetDouble       DOH_NAMESPACE(SetDouble)
#define DohSetVoid         DOH_NAMESPACE(SetVoid)
#define DohGetVoid         DOH_NAMESPACE(GetVoid)
#define DohGetitem         DOH_NAMESPACE(Getitem)
#define DohSetitem         DOH_NAMESPACE(Setitem)
#define DohDelitem         DOH_NAMESPACE(Delitem)
#define DohInsertitem      DOH_NAMESPACE(Insertitem)
#define DohFirstitem       DOH_NAMESPACE(Firstitem)
#define DohNextitem        DOH_NAMESPACE(Nextitem)
#define DohWrite           DOH_NAMESPACE(Write)
#define DohRead            DOH_NAMESPACE(Read)
#define DohSeek            DOH_NAMESPACE(Seek)
#define DohTell            DOH_NAMESPACE(Tell)
#define DohGetc            DOH_NAMESPACE(Getc)
#define DohPutc            DOH_NAMESPACE(Putc)
#define DohUngetc          DOH_NAMESPACE(Ungetc)
#define DohGetline         DOH_NAMESPACE(Getline)
#define DohSetline         DOH_NAMESPACE(Setline)
#define DohGetfile         DOH_NAMESPACE(Getfile)
#define DohSetfile         DOH_NAMESPACE(Setfile)
#define DohReplace         DOH_NAMESPACE(Replace)
#define DohChop            DOH_NAMESPACE(Chop)
#define DohGetmeta         DOH_NAMESPACE(Getmeta)
#define DohSetmeta         DOH_NAMESPACE(Setmeta)
#define DohDelmeta         DOH_NAMESPACE(Delmeta)
#define DohEncoding        DOH_NAMESPACE(Encoding)
#define DohPrintf          DOH_NAMESPACE(Printf)
#define DohvPrintf         DOH_NAMESPACE(vPrintf)
#define DohPrintv          DOH_NAMESPACE(Printv)
#define DohReadline        DOH_NAMESPACE(Readline)
#define DohIsMapping       DOH_NAMESPACE(IsMapping)
#define DohIsSequence      DOH_NAMESPACE(IsSequence)
#define DohIsString        DOH_NAMESPACE(IsString)
#define DohIsFile          DOH_NAMESPACE(IsFile)
#define DohNewString       DOH_NAMESPACE(NewString)
#define DohNewStringf      DOH_NAMESPACE(NewStringf)
#define DohStrcmp          DOH_NAMESPACE(Strcmp)
#define DohStrncmp         DOH_NAMESPACE(Strncmp)
#define DohStrstr          DOH_NAMESPACE(Strstr)
#define DohStrchr          DOH_NAMESPACE(Strchr)
#define DohNewFile         DOH_NAMESPACE(NewFile)
#define DohNewFileFromFile DOH_NAMESPACE(NewFileFromFile)
#define DohNewFileFromFd   DOH_NAMESPACE(NewFileFromFd)
#define DohClose           DOH_NAMESPACE(Close)
#define DohCopyto          DOH_NAMESPACE(Copyto)
#define DohNewList         DOH_NAMESPACE(NewList)
#define DohNewHash         DOH_NAMESPACE(NewHash)
#define DohNewVoid         DOH_NAMESPACE(NewVoid)
#define DohSplit           DOH_NAMESPACE(Split)
#define DohNone            DOH_NAMESPACE(None)

#define DohObjMalloc       DOH_NAMESPACE(ObjMalloc)
#define DohObjFree         DOH_NAMESPACE(ObjFree)
#define DohStringType      DOH_NAMESPACE(StringType)
#define DohListType        DOH_NAMESPACE(ListType)
#define DohHashType        DOH_NAMESPACE(HashType)
#define DohFileType        DOH_NAMESPACE(FileType)
#define DohVoidType        DOH_NAMESPACE(VoidType)

#endif

#include <stdio.h>
#include <stdarg.h>

#define DOH_MAJOR_VERSION 0
#define DOH_MINOR_VERSION 1

typedef void DOH;

/*
 * With dynamic typing, all DOH objects are technically of type 'void *'.
 * However, to clarify the reading of source code, the following symbolic
 * names are used.
 */

#define DOHString          DOH
#define DOHList            DOH
#define DOHHash            DOH
#define DOHFile            DOH
#define DOHVoid            DOH
#define DOHString_or_char  DOH
#define DOHObj_or_char     DOH

#define DOH_BEGIN          -1
#define DOH_END            -2
#define DOH_CUR            -3
#define DOH_CURRENT        -3

/* Memory management */

#ifndef DohMalloc
#define DohMalloc malloc
#endif
#ifndef DohRealloc
#define DohRealloc realloc
#endif
#ifndef DohFree
#define DohFree free
#endif

extern int           DohCheck(const DOH *ptr);           /* Check if a DOH object */
extern void          DohIntern(DOH *);                   /* Intern an object      */

/* Basic object methods.  Common to most objects */

extern void          DohDelete(DOH *obj);                /* Delete an object      */
extern DOH          *DohCopy(const DOH *obj);
extern void          DohClear(DOH *obj);
extern DOHString    *DohStr(const DOH *obj);
extern void         *DohData(const DOH *obj);
extern int           DohDump(const DOH *obj, DOHFile *out);
extern int           DohLen(const DOH *obj);
extern int           DohHashval(const DOH *obj);
extern int           DohCmp(const DOH *obj1, const DOH *obj2);
extern void          DohIncref(DOH *obj);

/* Mapping methods */

extern DOH          *DohGetattr(DOH *obj, const DOHString_or_char *name);
extern int           DohSetattr(DOH *obj, const DOHString_or_char *name, const DOHObj_or_char *value);
extern int           DohDelattr(DOH *obj, const DOHString_or_char *name);
extern DOH          *DohFirstkey(DOH *obj);
extern DOH          *DohNextkey(DOH *obj);
extern DOH          *DohKeys(DOH *obj);
extern int           DohGetInt(DOH *obj, const DOHString_or_char *name);
extern double        DohGetDouble(DOH *obj, const DOHString_or_char *name);
extern char         *DohGetChar(DOH *obj, const DOHString_or_char *name);
extern void          DohSetInt(DOH *obj, const DOHString_or_char *name, int);
extern void          DohSetDouble(DOH *obj, const DOHString_or_char *name, double);
extern void         *DohGetVoid(DOH *obj, const DOHString_or_char *name);
extern void          DohSetVoid(DOH *obj, const DOHString_or_char *name, void *value);

/* Sequence methods */

extern DOH          *DohGetitem(DOH *obj, int index);
extern int           DohSetitem(DOH *obj, int index, const DOHObj_or_char *value);
extern int           DohDelitem(DOH *obj, int index);
extern int           DohInsertitem(DOH *obj, int index, const DOHObj_or_char *value);
extern DOH          *DohFirstitem(DOH *obj);
extern DOH          *DohNextitem(DOH *obj);

/* File methods */

extern int           DohWrite(DOHFile *obj, void *buffer, int length);
extern int           DohRead(DOHFile *obj, void *buffer, int length);
extern int           DohSeek(DOHFile *obj, long offset, int whence);
extern long          DohTell(DOHFile *obj);
extern int           DohGetc(DOHFile *obj);
extern int           DohPutc(int ch, DOHFile *obj);
extern int           DohUngetc(int ch, DOHFile *obj);

  /* Positional */

extern int           DohGetline(DOH *obj);
extern void          DohSetline(DOH *obj, int line);
extern DOH          *DohGetfile(DOH *obj);
extern void          DohSetfile(DOH *obj, DOH *file);

  /* String Methods */

extern int           DohReplace(DOHString *src, const DOHString_or_char *token, const DOHString_or_char *rep, int flags);
extern void          DohChop(DOHString *src);

/* Meta-variables */
extern DOH          *DohGetmeta(DOH *, const DOH *);
extern int           DohSetmeta(DOH *, const DOH *, const DOH *value);
extern int           DohDelmeta(DOH *, const DOH *);

  /* Utility functions */

extern void          DohEncoding(char *name, DOH *(*fn)(DOH *s));
extern int           DohPrintf(DOHFile *obj, const char *format, ...);
extern int           DohvPrintf(DOHFile *obj, const char *format, va_list ap);
extern int           DohPrintv(DOHFile *obj, ...);
extern DOH          *DohReadline(DOHFile *in);

  /* Miscellaneous */

extern int           DohIsMapping(const DOH *obj);
extern int           DohIsSequence(const DOH *obj);
extern int           DohIsString(const DOH *obj);
extern int           DohIsFile(const DOH *obj);


/* -----------------------------------------------------------------------------
 * Strings.
 * ----------------------------------------------------------------------------- */

extern DOHString    *DohNewString(const DOH *c);
extern DOHString    *DohNewStringf(const DOH *fmt, ...);

extern int   DohStrcmp(const DOHString_or_char *s1, const DOHString_or_char *s2);
extern int   DohStrncmp(const DOHString_or_char *s1, const DOHString_or_char *s2, int n);
extern char *DohStrstr(const DOHString_or_char *s1, const DOHString_or_char *s2);
extern char *DohStrchr(const DOHString_or_char *s1, int ch);

/* String replacement flags */

#define   DOH_REPLACE_ANY         0x01
#define   DOH_REPLACE_NOQUOTE     0x02
#define   DOH_REPLACE_ID          0x04
#define   DOH_REPLACE_FIRST       0x08

#define Replaceall(s,t,r)  DohReplace(s,t,r,DOH_REPLACE_ANY)
#define Replaceid(s,t,r)   DohReplace(s,t,r,DOH_REPLACE_ID)

/* -----------------------------------------------------------------------------
 * Files
 * ----------------------------------------------------------------------------- */

extern DOHFile *DohNewFile(DOH *file, const char *mode);
extern DOHFile *DohNewFileFromFile(FILE *f);
extern DOHFile *DohNewFileFromFd(int fd);
extern int      DohClose(DOH *file);
extern int      DohCopyto(DOHFile *input, DOHFile *output);


/* -----------------------------------------------------------------------------
 * List
 * ----------------------------------------------------------------------------- */

extern DOHList  *DohNewList();

/* -----------------------------------------------------------------------------
 * Hash
 * ----------------------------------------------------------------------------- */

extern DOHHash   *DohNewHash();

/* -----------------------------------------------------------------------------
 * Void
 * ----------------------------------------------------------------------------- */

extern DOHVoid  *DohNewVoid(void *ptr, void (*del)(void *));
extern DOHList  *DohSplit(DOHFile *input, const char *chs, int nsplits);
extern DOH      *DohNone;

#ifndef DOH_LONG_NAMES
/* Macros to invoke the above functions.  Includes the location of
   the caller to simplify debugging if something goes wrong */

#define Delete             DohDelete
#define Copy               DohCopy
#define Clear              DohClear
#define Str                DohStr
#define Dump               DohDump
#define Getattr            DohGetattr
#define Setattr            DohSetattr
#define Delattr            DohDelattr
#define Hashval            DohHashval
#define Getitem            DohGetitem
#define Setitem            DohSetitem
#define Delitem            DohDelitem
#define Insert             DohInsertitem
#define Append(s,x)        DohInsertitem(s,DOH_END,x)
#define Push(s,x)          DohInsertitem(s,DOH_BEGIN,x)
#define Len                DohLen
#define Firstkey           DohFirstkey
#define Nextkey            DohNextkey
#define Data               DohData
#define Char               (char *) Data
#define Cmp                DohCmp
#define Setline            DohSetline
#define Getline            DohGetline
#define Setfile            DohSetfile
#define Getfile            DohGetfile
#define Write              DohWrite
#define Read               DohRead
#define Seek               DohSeek
#define Tell               DohTell
#define Printf             DohPrintf
#define Printv             DohPrintv
#define Getc               DohGetc
#define Putc               DohPutc
#define Ungetc             DohUngetc
#define Close              DohClose
#define vPrintf            DohvPrintf
#define GetInt             DohGetInt
#define GetDouble          DohGetDouble
#define GetChar            DohGetChar
#define GetVoid            DohGetVoid
#define SetInt             DohSetInt
#define SetDouble          DohSetDouble
#define SetChar            DohSetattr
#define SetVoid            DohSetVoid
#define Firstitem          DohFirstitem
#define Nextitem           DohNextitem
#define Readline           DohReadline
#define Replace            DohReplace
#define Chop               DohChop
#define Getmeta            DohGetmeta
#define Setmeta            DohSetmeta
#define Delmeta            DohDelmeta
#define NewString          DohNewString
#define NewStringf         DohNewStringf
#define NewHash            DohNewHash
#define NewList            DohNewList
#define NewFile            DohNewFile
#define NewFileFromFile    DohNewFileFromFile
#define NewFileFromFd      DohNewFileFromFd
#define Close              DohClose
#define NewVoid            DohNewVoid
#define Keys               DohKeys
#define Strcmp             DohStrcmp
#define Strncmp            DohStrncmp
#define Strstr             DohStrstr
#define Strchr             DohStrchr
#define Copyto             DohCopyto
#define Split              DohSplit

#endif


#endif /* DOH_H */




