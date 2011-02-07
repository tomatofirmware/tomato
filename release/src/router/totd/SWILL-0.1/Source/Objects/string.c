/* -----------------------------------------------------------------------------
 * string.c
 *
 *     Implements a string object that supports both sequence operations and
 *     file semantics.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

static char cvsroot[] = "$Header: /home/pastacvs/cvs-rep/vermicelli/tot/src/SWILL-0.1/Source/Objects/string.c,v 1.1 2005/01/27 08:46:25 dillema Exp $";

#include "dohint.h"

extern DohObjInfo DohStringType;

typedef struct String {
  DOH           *file;
  int            line;
  int            maxsize;                   /* Max size allocated */
  int            len;                       /* Current length     */
  int            hashkey;                   /* Hash key value     */
  int            sp;                        /* Current position   */
  char          *str;                       /* String data        */
} String;

/* -----------------------------------------------------------------------------
 * void *String_data() - Return as a 'void *'
 * ----------------------------------------------------------------------------- */

static void *
String_data(DOH *so) {
  String *s = (String *) ObjData(so);
  s->str[s->len] = 0;
  return (void *) s->str;
}

/* -----------------------------------------------------------------------------
 * int String_dump() - Serialize a string onto out
 * ----------------------------------------------------------------------------- */

static int
String_dump(DOH *so, DOH *out) {
  int nsent;
  int ret;
  String *s = (String *) ObjData(so);
  nsent = 0;
  while (nsent < s->len) {
    ret = Write(out,s->str+nsent,(s->len-nsent));
    if (ret < 0) return ret;
    nsent += ret;
  }
  return nsent;
}

/* -----------------------------------------------------------------------------
 * CopyString() - Copy a string
 * ----------------------------------------------------------------------------- */

static DOH *
CopyString(DOH *so) {
  String *s = (String *) ObjData(so);
  int max;
  String *str;
  str = (String *) DohMalloc(sizeof(String));
  str->hashkey = -1;
  str->sp = 0;
  str->line = s->line;
  str->file = s->file;
  if (str->file) Incref(str->file);
  max = s->maxsize;
  str->str = (char *) DohMalloc(max);
  memmove(str->str, s->str, max);
  str->maxsize= max;
  str->len = s->len;
  str->str[str->len] = 0;
  return DohObjMalloc(&DohStringType,str);
}

/* -----------------------------------------------------------------------------
 * DelString() - Delete a string
 * ----------------------------------------------------------------------------- */

static void
DelString(DOH *so) {
  String *s = (String *) ObjData(so);
  DohFree(s->str);
  DohFree(s);
}

/* -----------------------------------------------------------------------------
 * String_len() - Length of a string
 * ----------------------------------------------------------------------------- */

static int
String_len(DOH *so) {
  String *s = (String *) ObjData(so);
  return s->len;
}


/* -----------------------------------------------------------------------------
 * int String_cmp() - Compare two strings
 * ----------------------------------------------------------------------------- */

static int
String_cmp(DOH *so1, DOH *so2)
{
  String *s1, *s2;
  char *c1, *c2;
  int  maxlen,i;
  s1 = (String *) ObjData(so1);
  s2 = (String *) ObjData(so2);
  maxlen = s1->len;
  if (s2->len < maxlen) maxlen = s2->len;
  c1 = s1->str;
  c2 = s2->str;
  for (i = 0; i < maxlen; i++,c1++,c2++) {
    if (*c1 != *c2) break;
  }
  if (i < maxlen) {
    if (*c1 < *c2) return -1;
    else return 1;
  }
  if (s1->len == s2->len) return 0;
  if (s1->len > s2->len) return 1;
  return -1;
}

/* -----------------------------------------------------------------------------
 * int String_hash() - Compute string hash value
 * ----------------------------------------------------------------------------- */

static int 
String_hash(DOH *so) {
  String *s = (String *) ObjData(so);
  char *c;
  int   i, h = 0, len;

  if (s->hashkey >= 0) return s->hashkey;
  c = s->str;
  len = s->len > 50 ? 50 : s->len;
  for (i = 0; i < len; i++) {
    h = (((h << 5) + *(c++)));
  }
  h = h & 0x7fffffff;
  s->hashkey = h;
  return h;
}  

/* -----------------------------------------------------------------------------
 * static add(String *s, const char *newstr) - Append to s
 * ----------------------------------------------------------------------------- */

static void
add(String *s, const char *newstr) {
  int   oldlen, newlen, newmaxsize, l, i, sp;
  char *tc;
  if (!newstr) return;
  s->hashkey = -1;
  l = (int) strlen(newstr);
  oldlen = s->len;
  newlen = oldlen+l + 1;
  if (newlen >= s->maxsize-1) {
    newmaxsize = 2*s->maxsize;
    if (newlen >= newmaxsize -1) newmaxsize = newlen + 1;
    s->str = (char *) DohRealloc(s->str,newmaxsize);
    assert(s->str);
    s->maxsize = newmaxsize;
  }
  tc = s->str;
  strcpy(tc+oldlen,newstr);
  sp = s->sp;
  if (sp >= oldlen) {
    tc += sp;
    for (i = sp; i < oldlen+l; i++,tc++) {
      if (*tc == '\n') s->line++;
    }
    s->sp = oldlen+l;
  }
  s->len += l;
}

/* Add a single character to s */

static void
String_addchar(String *s, char c) {
  register char *tc;
  register int   len = s->len;
  register int   maxsize = s->maxsize;
  s->hashkey = -1;
  if (len > (maxsize-2)) {
    s->str = (char *) DohRealloc(s->str,2*maxsize);
    assert(s->str);
    s->maxsize = 2*maxsize;
  }
  tc = s->str;
  tc[len] = c;
  if (s->sp >= len) {
    s->sp = len+1;
    tc[len+1] = 0;
    if (c == '\n') s->line++;
  }
  s->len++;
}

/* Expand a string to accomodate a write */

static void
String_expand(String *s, int width) {
  if ((s->len + width) > (s->maxsize-1)) {
    s->str = (char *) DohRealloc(s->str,(s->len + width)+1);
    assert(s->str);
    s->maxsize = s->len + width + 1;
  }
}

/* -----------------------------------------------------------------------------
 * void String_clear() - Clear a string
 * ----------------------------------------------------------------------------- */

static void
String_clear(DOH *so)
{
  String *s = (String *) ObjData(so);
  s->hashkey = -1;
  s->len = 0;
  *(s->str) = 0;
  s->sp = 0;
  s->line = 1;
}

/* -----------------------------------------------------------------------------
 * void String_insert() - Insert a string
 * ----------------------------------------------------------------------------- */

static int 
String_insert(DOH *so, int pos, DOH *str)
{
  String *s = (String *) ObjData(so);
  char *nstr;
  int   len;
  char *data;

  data = (char *) DohData(str);
  nstr = s->str;

  s->hashkey = -1;
  if (pos == DOH_END) {
    add(s, data);
    return 0;
  }

  if (pos < 0) pos = 0;
  else if (pos > s->len) pos = s->len;
  
  /* See if there is room to insert the new data */
  len = Len(str);  
  while (s->maxsize <= s->len+len) {
    s->str = (char *) DohRealloc(s->str,2*s->maxsize);
    assert(s->str);
    s->maxsize *= 2;
  }
  memmove(s->str+pos+len, s->str+pos, (s->len - pos));
  memcpy(s->str+pos,data,len);
  if (s->sp >= pos) {
    int i;
    
    for (i = 0; i < len; i++) {
      if (data[i] == '\n') s->line++;
    }
    s->sp+=len;
  }
  s->len += len;
  s->str[s->len] = 0;
  return 0;
}  

/* -----------------------------------------------------------------------------
 * int String_delitem() - Delete a character
 * ----------------------------------------------------------------------------- */

static int 
String_delitem(DOH *so, int pos)
{
  String *s = (String *) ObjData(so);
  s->hashkey = -1;
  if (pos == DOH_END) pos = s->len-1;
  if (pos == DOH_BEGIN) pos = 0;
  if (s->len == 0) return 0;

  if (s->sp > pos) {
    s->sp--;
    assert (s->sp >= 0);
    if (s->str[pos] == '\n') s->line--;
  }
  memmove(s->str+pos, s->str+pos+1, ((s->len-1) - pos));
  s->len--;
  s->str[s->len] = 0;
  return 0;
}

/* -----------------------------------------------------------------------------
 * DOH *String_str() - Returns a string (used by printing commands)
 * ----------------------------------------------------------------------------- */

static DOH *
String_str(DOH *so)
{
  String *s = (String *) ObjData(so);
  s->str[s->len] = 0;
  return NewString(s->str);
}

/* -----------------------------------------------------------------------------
 * int String_read() - Read data from a string
 * ----------------------------------------------------------------------------- */

static int
String_read(DOH *so, void *buffer, int len)
{
  int    reallen, retlen;
  char   *cb;
  String *s = (String *) ObjData(so);
  if ((s->sp + len) > s->len) reallen = (s->len - s->sp);
  else reallen = len;

  cb = (char *) buffer;
  retlen = reallen;

  if (reallen > 0) {
    memmove(cb, s->str+s->sp, reallen);
    s->sp += reallen;
  }
  return retlen;
}

/* -----------------------------------------------------------------------------
 * int String_write() - Write data to a string
 * ----------------------------------------------------------------------------- */
static int
String_write(DOH *so, void *buffer, int len)
{
  int    newlen;
  String *s = (String *) ObjData(so);
  s->hashkey = -1;
  if (s->sp > s->len) s->sp = s->len;
  newlen = s->sp + len+1;
  if (newlen > s->maxsize) {
    s->str = (char *) DohRealloc(s->str,newlen);
    assert(s->str);
    s->maxsize = newlen;
    s->len = s->sp + len;
  }
  if ((s->sp+len) > s->len) s->len = s->sp + len;
  memmove(s->str+s->sp,buffer,len);
  s->sp += len;
  s->str[s->len] = 0;
  return len;
}

/* -----------------------------------------------------------------------------
 * int String_seek() - Seek to a new position
 * ----------------------------------------------------------------------------- */

static int
String_seek(DOH *so, long offset, int whence)
{
  int    pos, nsp, inc;
  int prev;
  String *s = (String *) ObjData(so);
  if (whence == SEEK_SET) pos = 0;
  else if (whence == SEEK_CUR) pos = s->sp;
  else if (whence == SEEK_END) {
    pos = s->len;
    offset = -offset;
  } else pos = s->sp;

  nsp = pos + offset;
  if (nsp < 0)
    nsp = 0;
  if (s->len > 0 && nsp >= s->len)
    nsp = s->len-1;

  inc = (nsp > s->sp) ? 1 : -1;

  {
    register int sp = s->sp;
    register char *tc = s->str;
    register int len = s->len;
    while (sp != nsp) {
      prev = sp + inc;
      if (prev>=0 && prev<=len && tc[prev] == '\n')
	s->line += inc;
      sp+=inc;
    }
    s->sp = sp;
  }
  assert (s->sp >= 0);
  return 0;
}

/* -----------------------------------------------------------------------------
 * long String_tell() - Return current position
 * ----------------------------------------------------------------------------- */

static long
String_tell(DOH *so)
{
  String *s = (String *) ObjData(so);
  return (long) (s->sp);
}

/* -----------------------------------------------------------------------------
 * int String_putc()
 * ----------------------------------------------------------------------------- */

static int
String_putc(DOH *so, int ch)
{
  register int len, maxsize, sp;
  String *s = (String *) ObjData(so);
  s->hashkey = -1;
  len = s->len;
  sp = s->sp;
  if (sp >= len) {
    register char *tc;
    maxsize = s->maxsize;
    if (len > (maxsize-2)) {
      s->str = (char *) DohRealloc(s->str,2*maxsize);
      assert(s->str);
      s->maxsize = 2*maxsize;
    }
    tc = s->str + len;
    *(tc++) = ch;
    if (sp >= len) {
      s->sp = len+1;
      *tc = 0;
      if (ch == '\n') s->line++;
    }
    s->len = len+1;
  } else {
    s->str[s->sp++] = (char) ch;
    if (ch == '\n') s->line++;
  }
  return ch;
}

/* -----------------------------------------------------------------------------
 * int String_getc()
 * ----------------------------------------------------------------------------- */

static int 
String_getc(DOH *so)
{
  int c;
  String *s = (String *) ObjData(so);
  if (s->sp >= s->len)
    c = EOF;
  else
    c = (int) s->str[s->sp++];
  if (c == '\n') s->line++;
  return c;
}

/* -----------------------------------------------------------------------------
 * int String_ungetc()
 * ----------------------------------------------------------------------------- */

static int 
String_ungetc(DOH *so, int ch)
{
  String *s = (String *) ObjData(so);
  if (ch == EOF) return ch;
  if (s->sp <= 0) return EOF;
  s->sp--;
  if (ch == '\n') s->line--;
  return ch;
}


/* -----------------------------------------------------------------------------
 * replace_simple(String *str, char *token, char *rep, int flags, int count)
 *
 * Replaces count non-overlapping occurrences of token with rep in a string.   
 * ----------------------------------------------------------------------------- */

static char *
end_quote(char *s)
{
  char  qc;
  char  *q;
  qc = *s;
  while (1) {
    q = strpbrk(s+1,"\"\'");
    if (!q) return 0;
    if ((*q == qc) && (*(q-1) != '\\')) return q;
    s = q;
  }
}

static char *
match_simple(char *base, char *s, char *token, int tokenlen)
{
  return strstr(s,token);
}

static char *
match_identifier(char *base, char *s, char *token, int tokenlen)
{
  while (s) {
    s = strstr(s,token);
    if (!s) return 0;
    if ((s > base) && (isalnum(*(s-1)) || (*(s-1) == '_'))) {
      s += tokenlen;
      continue;
    }
    if (isalnum(*(s+tokenlen)) || (*(s+tokenlen) == '_')) {
      s += tokenlen;
      continue;
    }
    return s;
  }
  return 0;
}

static int 
replace_simple(String *str, char *token, char *rep, int flags, int count, char *(*match)(char *, char *, char *, int))
{
  int tokenlen;           /* Length of the token */
  int replen;             /* Length of the replacement */
  int delta, expand = 0;
  int ic;
  int rcount = 0;
  int noquote = 0;
  char *c, *s, *t, *first;
  char *q, *q2;
  register char *base;
  int i;


  /* Figure out if anything gets replaced */
  if (!strlen(token)) return 0;

  base = str->str;
  tokenlen = strlen(token);
  s = (*match)(base,base,token,tokenlen);

  if (!s) return 0;    /* No matches.  Who cares */

  if (flags & DOH_REPLACE_NOQUOTE) noquote = 1;

  /* If we are not replacing inside quotes, we need to do a little extra work */
  if (noquote) {
    q = strpbrk(base,"\"\'");
    if (!q) {
      noquote = 0;         /* Well, no quotes to worry about. Oh well */
    } else {
      while (q && (q < s)) {
	/* First match was found inside a quote.  Try to find another match */
	q2 = end_quote(q);
	if (!q2) {
	  return 0;
	}
	if (q2 > s)
	  s = (*match)(base,q2+1,token,tokenlen);
	if (!s) return 0;         /* Oh well, no matches */
	q = strpbrk(q2+1,"\"\'");
	if (!q) noquote = 0;      /* No more quotes */
      }
    }
  }

  first = s;
  replen = strlen(rep);

  delta = (replen - tokenlen);

  if (delta <= 0) {
    /* String is either shrinking or staying the same size */
    /* In this case, we do the replacement in place without memory reallocation */
    ic = count;
    t = s;         /* Target of memory copies */
    while (ic && s) {
      if (replen) {
	memcpy(t,rep,replen);
	t += replen;
      }
      rcount++;
      expand += delta;
      /* Find the next location */
      s += tokenlen;
      if (ic == 1) break;
      c = (*match)(base,s,token,tokenlen);
      
      if (noquote) {
	q = strpbrk(s,"\"\'");
	if (!q) {
	  noquote = 0;
	} else {
	  while (q && (q < c)) {
	    /* First match was found inside a quote.  Try to find another match */
	    q2 = end_quote(q);
	    if (!q2) {
	      c = 0;
	      break;
	    }
	    if (q2 > c)
	      c = (*match)(base,q2+1,token,tokenlen);
	    if (!c) break;
	    q = strpbrk(q2+1,"\"\'");
	    if (!q) noquote = 0;      /* No more quotes */
	  }
	}
      }
      if (delta) {
	if (c) {
	  memmove(t,s,c-s);
	  t += (c-s);
	} else {
	  memmove(t,s,(str->str + str->len) - s + 1);
	}
      } else {
	t += (c-s);
      }
      s = c;
      ic--;
    }
    if (s && delta) {
      memmove(t,s,(str->str + str->len) - s + 1);
    }
    str->len += expand;
    str->str[str->len] = 0;
    if (str->sp >= str->len) str->sp += expand;  /* Fix the end of file pointer */
    return rcount;
  }
  /* The string is expanding as a result of the replacement */
  /* Figure out how much expansion is going to occur and allocate a new string */
  {
    char *ns;
    int   newsize;

    rcount++;
    ic = count -1;
    s += tokenlen;
    while (ic && (c = (*match)(base,s,token,tokenlen))) {
      if (noquote) {
	q = strpbrk(s,"\"\'");
	if (!q) {
	  break;
	} else {
	  while (q && (q < c)) {
	    /* First match was found inside a quote.  Try to find another match */
	    q2 = end_quote(q);
	    if (!q2) {
	      c = 0;
	      break;
	    }
	    if (q2 > c)
	      c = (*match)(base,q2+1,token,tokenlen);
	    if (!c) break;
	    q = strpbrk(q2+1,"\"\'");
	  }
	}
      }
      if (c) {
	rcount++;
	ic--;
	s = c + tokenlen;
      } else {
	break;
      }
    }
    expand = delta*rcount;     /* Total amount of expansion for the replacement */
    newsize = str->maxsize;
    while ((str->len + expand) >= newsize) newsize *= 2;

    ns = (char *) DohMalloc(newsize);
    assert(ns);
    t = ns;
    s = first;

    /* Copy the first part of the string */
    if (first > str->str) {
      memcpy(t,str->str,(first - str->str));
      t += (first-str->str);
    }
    for (i = 0; i < rcount; i++) {
      memcpy(t,rep,replen);
      t += replen;
      s += tokenlen;
      c = (*match)(base,s,token,tokenlen);
      if (noquote) {
	q = strpbrk(s,"\"\'");
	if (!q) {
	  noquote = 0;
	} else {
	  while (q && (q < c)) {
	    /* First match was found inside a quote.  Try to find another match */
	    q2 = end_quote(q);
	    if (!q2) {
	      c = 0;
	      break;
	    }
	    if (q2 > c)
	      c = (*match)(base,q2+1,token,tokenlen);
	    if (!c) break;
	    q = strpbrk(q2+1,"\"\'");
	    if (!q) noquote = 0;      /* No more quotes */
	  }
	}
      }
      if (i < (rcount - 1)) {
	memcpy(t,s,c-s);
	t += (c-s);
      } else {
	memcpy(t,s,(str->str + str->len) - s + 1);
      }
      s = c;
    }
    c = str->str;
    str->str = ns;
    if (str->sp >= str->len) str->sp += expand;
    str->len += expand;
    str->str[str->len] = 0;
    str->maxsize = newsize;
    DohFree(c);
    return rcount;
  }
}

/* -----------------------------------------------------------------------------
 * int String_replace()
 * ----------------------------------------------------------------------------- */

static int
String_replace(DOH *stro, DOH *token, DOH *rep, int flags)
{
    int count = -1;
    String *str = (String *) ObjData(stro);

    if (flags & DOH_REPLACE_FIRST) count = 1;

    if (flags & DOH_REPLACE_ID) {
      return replace_simple(str,Char(token),Char(rep),flags, count, match_identifier);
    } else {
      return replace_simple(str,Char(token), Char(rep), flags, count, match_simple);
    }
}

/* -----------------------------------------------------------------------------
 * void String_chop(DOH *str)
 * ----------------------------------------------------------------------------- */

static void
String_chop(DOH *so)
{
  char *c;
  String *str = (String *) ObjData(so);
  /* Replace trailing whitespace */
  c = str->str + str->len - 1;
  while ((str->len > 0) && (isspace(*c))) {
    if (str->sp >= str->len) {
      str->sp--;
      if (*c == '\n') str->line--;
    }
    str->len--;
    c--;
  }
  str->str[str->len] = 0; 
  assert (str->sp >= 0);
  str->hashkey = -1;
}

static void 
String_setfile(DOH *so, DOH *file)
{
  DOH *fo;
  String *str = (String *) ObjData(so);

  if (!DohCheck(file)) {
    fo = NewString(file);
    Decref(fo);
  } else fo = file;
  Incref(fo);
  Delete(str->file);
  str->file = fo;
}

static DOH *
String_getfile(DOH *so)
{
  String *str = (String *) ObjData(so);
  return str->file;
}

static void 
String_setline(DOH *so, int line)
{
  String *str = (String *) ObjData(so);
  str->line = line;
}

static int 
String_getline(DOH *so)
{
  String *str = (String *) ObjData(so);
  return str->line;
}

static DohListMethods StringListMethods = {
  0,                      /* doh_getitem */
  0,                      /* doh_setitem */
  String_delitem,         /* doh_delitem */
  String_insert,          /* doh_insitem */
  0,                      /* doh_first   */
  0,                      /* doh_next    */
};

static DohFileMethods StringFileMethods = {
  String_read,
  String_write,
  String_putc,
  String_getc,
  String_ungetc,
  String_seek,
  String_tell,
  0,              /* close */
};

static DohStringMethods StringStringMethods = {
  String_replace,
  String_chop,
};

DohObjInfo DohStringType = {
    "String",          /* objname */
    DelString,         /* doh_del */
    CopyString,        /* doh_copy */
    String_clear,      /* doh_clear */
    String_str,        /* doh_str */
    String_data,       /* doh_data */
    String_dump,       /* doh_dump */
    String_len,        /* doh_len */
    String_hash,       /* doh_hash    */
    String_cmp,        /* doh_cmp */
    String_setfile,    /* doh_setfile */
    String_getfile,    /* doh_getfile */
    String_setline,    /* doh_setline */
    String_getline,    /* doh_getline */
    0,                    /* doh_mapping */
    &StringListMethods,   /* doh_sequence */
    &StringFileMethods,   /* doh_file */
    &StringStringMethods, /* doh_string */
    0,                    /* doh_position */
    0,
};


#define INIT_MAXSIZE  16

/* -----------------------------------------------------------------------------
 * NewString(const char *c) - Create a new string
 * ----------------------------------------------------------------------------- */

DOHString *
DohNewString(const DOH *so)
{
    int l = 0, max;
    String *str;
    char *s;
    if (DohCheck(so)) s = Char(so);
    else s = (char *) so;
    str = (String *) DohMalloc(sizeof(String));
    str->hashkey = -1;
    str->sp = 0;
    str->line = 1;
    str->file = 0;
    max = INIT_MAXSIZE;
    if (s) {
      l = (int) strlen(s);
      if ((l+1) > max) max = l+1;
    }
    str->str = (char *) DohMalloc(max);
    str->maxsize = max;
    if (s) {
	strcpy(str->str,s);
	str->len = l;
	str->sp = l;
    } else {
	str->str[0] = 0;
	str->len = 0;
    }
    return DohObjMalloc(&DohStringType,str);
}

/* -----------------------------------------------------------------------------
 * NewStringf(DOH *fmt, ...)
 *
 * Create a new string from a list of objects.
 * ----------------------------------------------------------------------------- */

DOHString *
DohNewStringf(const DOH *fmt, ...)
{
  va_list ap;
  DOH *r;
  va_start(ap,fmt);
  r = NewString("");
  DohvPrintf(r,Char(fmt),ap);
  va_end(ap);
  return (DOHString *) r;
}

/* -----------------------------------------------------------------------------
 * Strcmp()
 * Strncmp()
 * Strstr()
 * Strchr()
 *
 * Some utility functions.
 * ----------------------------------------------------------------------------- */

int DohStrcmp(const DOHString_or_char *s1, const DOHString_or_char *s2) {
  return strcmp(Char(s1),Char(s2));
}

int DohStrncmp(const DOHString_or_char *s1, const DOHString_or_char *s2, int n) {
  return strncmp(Char(s1),Char(s2),n);
}

char *DohStrstr(const DOHString_or_char *s1, const DOHString_or_char *s2) {
  return strstr(Char(s1),Char(s2));
}

char *DohStrchr(const DOHString_or_char *s1, int ch) {
  return strchr(Char(s1),ch);
}
