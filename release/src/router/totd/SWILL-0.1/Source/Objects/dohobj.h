/* -----------------------------------------------------------------------------
 * dohobj.h
 *
 *     This file describes internally managed objects.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 *
 * $Header: /home/pastacvs/cvs-rep/vermicelli/tot/src/SWILL-0.1/Source/Objects/dohobj.h,v 1.1 2005/01/27 08:46:25 dillema Exp $
 * ----------------------------------------------------------------------------- */

#ifndef _DOHOBJ_H
#define _DOHOBJ_H

#include "doh.h"

/* Hash objects */
typedef struct {
  DOH    *(*doh_getattr)(DOH *obj, DOH *name);               /* Get attribute */
  int     (*doh_setattr)(DOH *obj, DOH *name, DOH *value);   /* Set attribute */
  int     (*doh_delattr)(DOH *obj, DOH *name);               /* Del attribute */
  DOH    *(*doh_firstkey)(DOH *obj);                         /* First key     */
  DOH    *(*doh_nextkey)(DOH *obj);                          /* Next key      */
  DOH    *(*doh_keys)(DOH *obj);                             /* All keys as a list */
} DohHashMethods;

/* List objects */
typedef struct {
  DOH      *(*doh_getitem)(DOH *obj, int index);             /* Get item      */
  int       (*doh_setitem)(DOH *obj, int index, DOH *value); /* Set item      */
  int       (*doh_delitem)(DOH *obj, int index);             /* Delete item   */
  int       (*doh_insitem)(DOH *obj, int index, DOH *value); /* Insert item   */
  DOH      *(*doh_firstitem)(DOH *obj);                      /* Iterators     */
  DOH      *(*doh_nextitem)(DOH *obj);
} DohListMethods;

/* File methods */
typedef struct {
  int       (*doh_read)(DOH *obj, void *buffer, int nbytes);  /* Read bytes */
  int       (*doh_write)(DOH *obj, void *buffer, int nbytes); /* Write bytes */
  int       (*doh_putc)(DOH *obj, int ch);                    /* Put character */
  int       (*doh_getc)(DOH *obj);                            /* Get character */
  int       (*doh_ungetc)(DOH *obj, int ch);                  /* Unget character */
  int       (*doh_seek)(DOH *obj, long offset, int whence);   /* Seek */
  long      (*doh_tell)(DOH *obj);                            /* Tell */
  int       (*doh_close)(DOH *obj);                           /* Close */
} DohFileMethods;

/* String methods */
typedef struct {
  int     (*doh_replace)(DOH *obj, DOH *old, DOH *rep, int flags);
  void    (*doh_chop)(DOH *obj);
} DohStringMethods;

/* -----------------------------------------------------------------------------
 * DohObjInfo
 * ----------------------------------------------------------------------------- */

typedef struct DohObjInfo {
  char       *objname;                         /* Object name        */

  /* Basic object methods */
  void      (*doh_del)(DOH *obj);              /* Delete object      */
  DOH      *(*doh_copy)(DOH *obj);             /* Copy and object    */
  void      (*doh_clear)(DOH *obj);            /* Clear an object    */

  /* I/O methods */
  DOH       *(*doh_str)(DOH *obj);             /* Make a full string */
  void      *(*doh_data)(DOH *obj);            /* Return raw data    */
  int        (*doh_dump)(DOH *obj, DOH *out);  /* Serialize on out   */

  /* Length and hash values */
  int        (*doh_len)(DOH *obj);
  int        (*doh_hashval)(DOH *obj);

  /* Compare */
  int        (*doh_cmp)(DOH *obj1, DOH *obj2);

  /* Positional */
  void       (*doh_setfile)(DOH *obj, DOHString_or_char *file);
  DOH       *(*doh_getfile)(DOH *obj);
  void       (*doh_setline)(DOH *obj, int line);
  int        (*doh_getline)(DOH *obj);

  DohHashMethods     *doh_hash;                /* Hash methods       */
  DohListMethods     *doh_list;                /* List methods       */
  DohFileMethods     *doh_file;                /* File methods       */
  DohStringMethods   *doh_string;              /* String methods     */
  void               *doh_reserved;            /* Reserved           */
  void               *clientdata;              /* User data          */
} DohObjInfo;

typedef struct {
  void       *data;                 /* Data pointer */
  DohObjInfo *type;             
  void       *meta;                 /* Meta data */
  int     flag_intern   : 1;        /* Interned object */
  int     flag_marked   : 1;        /* Mark flag. Used to avoid recursive loops in places */
  int     flag_user     : 1;        /* User flag */
  int     flag_reserved : 1;        /* Reserved flag */
  int     refcount      : 28;       /* Reference count (max 16 million) */
} DohBase;

/* Macros for decrefing and increfing (safe for null objects). */
#define Decref(a)         if (a) ((DohBase *) a)->refcount--
#define Incref(a)         if (a) ((DohBase *) a)->refcount++
#define Refcount(a)       ((DohBase *) a)->refcount

/* Macros for manipulating objects in a safe manner */
#define ObjData(a)        ((DohBase *)a)->data
#define ObjSetMark(a,x)   ((DohBase *)a)->flag_marked = x
#define ObjGetMark(a)     ((DohBase *)a)->flag_marked
#define ObjType(a)        ((DohBase *)a)->type

extern DOH     *DohObjMalloc(DohObjInfo *type, void *data); /* Allocate a DOH object */
extern void     DohObjFree(DOH *ptr);               /* Free a DOH object     */

#endif /* DOHOBJ_H */




