/* -----------------------------------------------------------------------------
 * base.c 
 *
 *     This file contains the function entry points for dispatching methods on
 *     DOH objects.  A number of small utility functions are also included.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 * ----------------------------------------------------------------------------- */

static char cvsroot[] = "$Header: /home/pastacvs/cvs-rep/vermicelli/tot/src/SWILL-0.1/Source/Objects/base.c,v 1.1 2005/01/27 08:46:24 dillema Exp $";

#include "dohint.h"

/* -----------------------------------------------------------------------------
 * DohDelete()
 * ----------------------------------------------------------------------------- */

void
DohDelete(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;

  if (!obj) return;
  if (!DohCheck(b)) {
    fputs("DOH: Fatal error. Attempt to delete a non-doh object.\n",stderr);
    abort();
  }
  if (b->flag_intern) return;
  assert(b->refcount > 0);
  b->refcount--;
  if (b->refcount <= 0) {
    objinfo = b->type;
    if (objinfo->doh_del) {
      (objinfo->doh_del)(b);
    } else {
      if (b->data) DohFree(b->data);
    }
    DohObjFree(b);
  }
}

/* -----------------------------------------------------------------------------
 * DohCopy()
 * ----------------------------------------------------------------------------- */

DOH *
DohCopy(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;

  if (!obj) return 0;
  objinfo = b->type;
  if (objinfo->doh_copy) {
    DohBase *bc = (DohBase *) (objinfo->doh_copy)(b);
    if ((bc) && b->meta) {
      bc->meta = Copy(b->meta);
    }
    return (DOH *) bc;
  }
  return 0;
}

void
DohIncref(DOH *obj) {
  Incref(obj);
}

/* -----------------------------------------------------------------------------
 * DohClear()
 * ----------------------------------------------------------------------------- */

void
DohClear(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_clear)
    (objinfo->doh_clear)(b);
}

/* -----------------------------------------------------------------------------
 * DohStr()
 * ----------------------------------------------------------------------------- */

DOH *
DohStr(const DOH *obj) {
  char buffer[512];
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(b)) {
    objinfo = b->type;
    if (objinfo->doh_str) {
      return (objinfo->doh_str)(b);
    }
    sprintf(buffer,"<Object '%s' at %x>", objinfo->objname, b);
    return NewString(buffer);
  } else {
    return NewString(obj);
  }
}

/* -----------------------------------------------------------------------------
 * DohDump()
 * ----------------------------------------------------------------------------- */

int
DohDump(const DOH *obj, DOH *out) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_dump) {
    return (objinfo->doh_dump)(b,out);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohLen() - Defaults to strlen() if not a DOH object
 * ----------------------------------------------------------------------------- */
int
DohLen(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (!b) return 0;
  if (DohCheck(b)) {
    objinfo = b->type;
    if (objinfo->doh_len) {
      return (objinfo->doh_len)(b);
    }
    return 0;
  } else {
    return strlen((char *) obj);
  }
}

/* -----------------------------------------------------------------------------
 * DohHashVal()
 * ----------------------------------------------------------------------------- */

int
DohHashval(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(b)) {
    objinfo = b->type;
    if (objinfo->doh_hashval) {
      return (objinfo->doh_hashval)(b);
    }
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohData()
 * ----------------------------------------------------------------------------- */

void *
DohData(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if (objinfo->doh_data) {
      return (objinfo->doh_data)(b);
    }
    return 0;
  }
  return (void *) obj;
}

/* -----------------------------------------------------------------------------
 * DohCmp()
 * ----------------------------------------------------------------------------- */

int
DohCmp(const DOH *obj1, const DOH *obj2) {
  DohBase *b1, *b2;
  DohObjInfo *b1info, *b2info;
  b1 = (DohBase *) obj1;
  b2 = (DohBase *) obj2;
  if ((!DohCheck(b1)) || (!DohCheck(b2))) {
    if ((b1 == 0) && (b2 == 0)) return 0;
    if (b1 && !b2) return 1;
    if (!b1 &&  b2) return -1;
    return strcmp((char *) DohData(b1),(char *) DohData(b2));
  }
  b1info = b1->type;
  b2info = b2->type;
  if ((b1info == b2info) && (b1info->doh_cmp)) 
    return (b1info->doh_cmp)(b1,b2);
  return 1;
}

/* -----------------------------------------------------------------------------
 * DohIsMapping()
 * ----------------------------------------------------------------------------- */
int
DohIsMapping(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (!DohCheck(b)) return 0;
  objinfo = b->type;
  if (objinfo->doh_hash) return 1;
  else return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetattr()
 * ----------------------------------------------------------------------------- */

DOH *
DohGetattr(DOH *obj, const DOH *name) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_hash && objinfo->doh_hash->doh_getattr) {
    return (objinfo->doh_hash->doh_getattr)(b,(DOH *) name);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohSetattr()
 * ----------------------------------------------------------------------------- */

int
DohSetattr(DOH *obj, const DOH *name, const DOH *value) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_hash && objinfo->doh_hash->doh_setattr) {
    return (objinfo->doh_hash->doh_setattr)(b,(DOH *) name,(DOH *) value);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohDelattr()
 * ----------------------------------------------------------------------------- */

int
DohDelattr(DOH *obj, const DOH *name) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_hash && objinfo->doh_hash->doh_delattr) {
    return (objinfo->doh_hash->doh_delattr)(b,(DOH *) name);
  }
}

/* -----------------------------------------------------------------------------
 * DohFirstkey()
 * ----------------------------------------------------------------------------- */

DOH *
DohFirstkey(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_hash && objinfo->doh_hash->doh_firstkey) {
    return (objinfo->doh_hash->doh_firstkey)(b);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohNextkey()
 * ----------------------------------------------------------------------------- */

DOH *
DohNextkey(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo && objinfo->doh_hash->doh_nextkey) {
    return (objinfo->doh_hash->doh_nextkey)(b);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohNextkey()
 * ----------------------------------------------------------------------------- */

DOH *
DohKeys(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo && objinfo->doh_hash->doh_keys) {
    return (objinfo->doh_hash->doh_keys)(b);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetInt()
 * ----------------------------------------------------------------------------- */

int
DohGetInt(DOH *obj, const DOH *name) {
  DOH *val;
  val = Getattr(obj,(DOH *) name);
  if (!val) return 0;
  if (DohIsString(val)) {
    return atoi((char *) Data(val));
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetDouble()
 * ----------------------------------------------------------------------------- */

double
DohGetDouble(DOH *obj, const DOH *name) {
  DOH *val;
  val = Getattr(obj,(DOH *) name);
  if (!val) return 0;
  if (DohIsString(val)) {
    return atof((char *) Data(val));
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetChar()
 * ----------------------------------------------------------------------------- */

char *
DohGetChar(DOH *obj, const DOH *name) {
  DOH *val;
  val = Getattr(obj,(DOH *) name);
  if (!val) return 0;
  if (DohIsString(val)) {
    return (char *) Data(val);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetVoid()
 * ----------------------------------------------------------------------------- */

void *
DohGetVoid(DOH *obj, const DOH *name) {
  DOH *val;
  val = Getattr(obj,(DOH *) name);
  if (!val) return 0;
  return (void *) Data(val);
}

/* -----------------------------------------------------------------------------
 * DohSetInt()
 * ----------------------------------------------------------------------------- */

void
DohSetInt(DOH *obj, const DOH *name, int value) {
  DOH *temp;
  temp = NewString("");
  Printf(temp,"%d",value);
  Setattr(obj,(DOH *) name,temp);
}

/* -----------------------------------------------------------------------------
 * DohSetDouble()
 * ----------------------------------------------------------------------------- */

void
DohSetDouble(DOH *obj, const DOH *name, double value) {
  DOH *temp;
  temp = NewString("");
  Printf(temp,"%0.17f",value);
  Setattr(obj,(DOH *) name,temp);
}

/* -----------------------------------------------------------------------------
 * DohSetChar()
 * ----------------------------------------------------------------------------- */

void
DohSetChar(DOH *obj, const DOH *name, char *value) {
  Setattr(obj,(DOH *) name,NewString(value));
}

/* -----------------------------------------------------------------------------
 * DohSetVoid()
 * ----------------------------------------------------------------------------- */

void
DohSetVoid(DOH *obj, const DOH *name, void *value) {
  Setattr(obj,(DOH *) name,NewVoid(value,0));
}

/* -----------------------------------------------------------------------------
 * DohIsSequence()
 * ----------------------------------------------------------------------------- */

int
DohIsSequence(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (!DohCheck(b)) return 0;
  objinfo = b->type;
  if (objinfo->doh_list) return 1;
  else return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetitem()
 * ----------------------------------------------------------------------------- */

DOH *
DohGetitem(DOH *obj, int index) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_getitem) {
    return (objinfo->doh_list->doh_getitem)(b,index);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohSetitem()
 * ----------------------------------------------------------------------------- */

int
DohSetitem(DOH *obj, int index, const DOH *value) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_setitem) {
    return (objinfo->doh_list->doh_setitem)(b,index,(DOH *) value);
  }
  return -1;
}

/* -----------------------------------------------------------------------------
 * DohDelitem()
 * ----------------------------------------------------------------------------- */

int
DohDelitem(DOH *obj, int index) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_delitem) {
    return (objinfo->doh_list->doh_delitem)(b,index);
  }
  return -1;
}

/* -----------------------------------------------------------------------------
 * DohInsertitem()
 * ----------------------------------------------------------------------------- */

int
DohInsertitem(DOH *obj, int index, const DOH *value) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_insitem) {
    return (objinfo->doh_list->doh_insitem)(b,index,(DOH *) value);
  }
  return -1;
}

/* -----------------------------------------------------------------------------
 * DohFirstitem()
 * ----------------------------------------------------------------------------- */

DOH *
DohFirstitem(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_firstitem) {
    return (objinfo->doh_list->doh_firstitem)(b);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohNextitem()
 * ----------------------------------------------------------------------------- */

DOH *
DohNextitem(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo = b->type;
  if (objinfo->doh_list && objinfo->doh_list->doh_nextitem) {
    return (objinfo->doh_list->doh_nextitem)(b);
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohIsFile()
 * ----------------------------------------------------------------------------- */

int
DohIsFile(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (!DohCheck(b)) return 0;
  objinfo = b->type;
  if (objinfo->doh_file) return 1;
  else return 0;
}

/* -----------------------------------------------------------------------------
 * DohRead()
 * ----------------------------------------------------------------------------- */

int
DohRead(DOH *obj, void *buffer, int length) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if ((objinfo->doh_file) && (objinfo->doh_file->doh_read)) {
      return (objinfo->doh_file->doh_read)(b,buffer,length);
    }
    return -1;
  }
  /* Hmmm.  Not a file.  Maybe it's a real FILE */
  return fread(buffer,1,length,(FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohWrite()
 * ----------------------------------------------------------------------------- */

int
DohWrite(DOH *obj, void *buffer, int length) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if ((objinfo->doh_file) && (objinfo->doh_file->doh_write)) {
      return (objinfo->doh_file->doh_write)(b,buffer,length);
    }
    return -1;
  }
  /* Hmmm.  Not a file.  Maybe it's a real FILE */
  return fwrite(buffer,1,length,(FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohSeek()
 * ----------------------------------------------------------------------------- */

int
DohSeek(DOH *obj, long offset, int whence) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if ((objinfo->doh_file) && (objinfo->doh_file->doh_seek)) {
      return (objinfo->doh_file->doh_seek)(b,offset,whence);
    }
    return -1;
  }
  return fseek((FILE *) b, offset, whence);
}

/* -----------------------------------------------------------------------------
 * DohTell()
 * ----------------------------------------------------------------------------- */

long
DohTell(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if ((objinfo->doh_file) && (objinfo->doh_file->doh_tell)) {
      return (objinfo->doh_file->doh_tell)(b);
    }
    return -1;
  }
  return ftell((FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohGetc()
 * ----------------------------------------------------------------------------- */

int
DohGetc(DOH *obj) {
  static DOH *lastdoh = 0;
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (obj == lastdoh) {
    objinfo = b->type;
    return (objinfo->doh_file->doh_getc)(b);
  }
  if (DohCheck(obj)) {
    objinfo = b->type;
    if (objinfo->doh_file->doh_getc) {
      lastdoh = obj;
      return (objinfo->doh_file->doh_getc)(b);
    }
    return EOF;
  }
  return fgetc((FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohPutc()
 * ----------------------------------------------------------------------------- */

int
DohPutc(int ch, DOH *obj) {
  static DOH *lastdoh = 0;
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;

  if (obj == lastdoh) {
    objinfo = b->type;
    return (objinfo->doh_file->doh_putc)(b,ch);
  }
  if (DohCheck(obj)) {
    objinfo = b->type;
    if (objinfo->doh_file->doh_putc) {
      lastdoh = obj;
      return (objinfo->doh_file->doh_putc)(b,ch);
    }
    return EOF;
  }
  return fputc(ch,(FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohUngetc()
 * ----------------------------------------------------------------------------- */

int
DohUngetc(int ch, DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if (objinfo->doh_file->doh_ungetc) {
      return (objinfo->doh_file->doh_ungetc)(b,ch);
    }
    return EOF;
  }
  return ungetc(ch,(FILE *) b);
}

/* -----------------------------------------------------------------------------
 * DohClose()
 * ----------------------------------------------------------------------------- */

int
DohClose(DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (DohCheck(obj)) {
    objinfo = b->type;
    if (objinfo->doh_file->doh_close) {
      return (objinfo->doh_file->doh_close)(b);
    }
    return 0;
  }
  return fclose((FILE *) obj);
}

/* -----------------------------------------------------------------------------
 * DohIsString()
 * ----------------------------------------------------------------------------- */

int
DohIsString(const DOH *obj) {
  DohBase *b = (DohBase *) obj;
  DohObjInfo *objinfo;
  if (!DohCheck(b)) return 0;
  objinfo = b->type;
  if (objinfo->doh_string) return 1;
  else return 0;
}

/* -----------------------------------------------------------------------------
 * DohReplace()
 * ----------------------------------------------------------------------------- */

int
DohReplace(DOH *src, const DOH *token, const DOH *rep, int flags) {
  DohBase *b = (DohBase *) src;
  DohObjInfo *objinfo;
  if (!token) return 0;
  if (!rep) rep = "";
  if (DohIsString(src)) {
    objinfo = b->type;
    if (objinfo->doh_string->doh_replace) {
      return (objinfo->doh_string->doh_replace)(b,(DOH *) token, (DOH *) rep,flags);
    }
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohChop()
 * ----------------------------------------------------------------------------- */

void
DohChop(DOH *src) {
  DohBase *b = (DohBase *) src;
  DohObjInfo *objinfo;
  if (DohIsString(src)) {
    objinfo = b->type;
    if (objinfo->doh_string->doh_chop) {
      (objinfo->doh_string->doh_chop)(b);
    }
  }
}

/* -----------------------------------------------------------------------------
 * DohSetFile()
 * ----------------------------------------------------------------------------- */
void
DohSetfile(DOH *ho, DOH *file) {
  DohBase *h = (DohBase *) ho;
  DohObjInfo *objinfo;
  if (!h) return;
  objinfo = h->type;
  if (objinfo->doh_setfile)
    (objinfo->doh_setfile)(h,file);
}

/* -----------------------------------------------------------------------------
 * DohGetFile()
 * ----------------------------------------------------------------------------- */
DOH *
DohGetfile(DOH *ho) {
  DohBase *h = (DohBase *) ho;
  DohObjInfo *objinfo;
  if (!h) return 0;
  objinfo = h->type;
  if (objinfo->doh_getfile) 
    return (objinfo->doh_getfile)(h);
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohSetLine()
 * ----------------------------------------------------------------------------- */
void
DohSetline(DOH *ho, int l) {
  DohBase *h = (DohBase *) ho;
  DohObjInfo *objinfo;
  if (!h) return;
  objinfo = h->type;
  if (objinfo->doh_setline) 
    (objinfo->doh_setline)(h,l);
}

/* -----------------------------------------------------------------------------
 * DohGetLine()
 * ----------------------------------------------------------------------------- */
int
DohGetline(DOH *ho) {
  DohBase *h = (DohBase *) ho;
  DohObjInfo *objinfo;
  if (!h) return 0;
  objinfo = h->type;
  if (objinfo->doh_getline) 
    return (objinfo->doh_getline)(h);
  return 0;
}

/* -----------------------------------------------------------------------------
 * DohGetmeta()
 * ----------------------------------------------------------------------------- */

DOH *
DohGetmeta(DOH *ho, const DOH *name) {
  DohBase *h = (DohBase *) ho;
  if (!DohCheck(ho)) return 0;
  if (!h->meta) return 0;
  return DohGetattr(h->meta,name);
}

/* -----------------------------------------------------------------------------
 * DohGetmeta()
 * ----------------------------------------------------------------------------- */

int
DohSetmeta(DOH *ho, const DOH *name, const DOH *value) {
  DohBase *h = (DohBase *) ho;
  if (!DohCheck(ho)) return 0;
  if (!h->meta) h->meta = NewHash();
  return DohSetattr(h->meta, name, value);
}

/* -----------------------------------------------------------------------------
 * DohDelmeta()
 * ----------------------------------------------------------------------------- */  

int
DohDelmeta(DOH *ho, const DOH *name) {
  DohBase *h = (DohBase *) ho;
  if (!DohCheck(ho)) return 0;
  if (!h->meta) return 0;
  return DohDelattr(h->meta, name);
}

