/* ----------------------------------------------------------------------------- 
 * fio.c
 *
 *     This file implements a number of standard I/O operations included
 *     formatted output, readline, and splitting.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

static char cvsroot[] = "$Header: /home/pastacvs/cvs-rep/vermicelli/tot/src/SWILL-0.1/Source/Objects/fio.c,v 1.1 2005/01/27 08:46:25 dillema Exp $";

#include "dohint.h"

#define OBUFLEN  512

static DOH *encodings = 0;              /* Encoding hash */

/* -----------------------------------------------------------------------------
 * Writen()
 *
 * Write's N characters of output and retries until all characters are
 * written.  This is useful should a write operation encounter a spurious signal.
 * ----------------------------------------------------------------------------- */

static int Writen(DOH *out, void *buffer, int len) {
  int nw = len, ret;
  char *cb = (char *) buffer;
  while (nw) {
    ret = Write(out,cb,nw);
    if (ret < 0) return -1;
    nw = nw - ret;
    cb += ret;
  }
  return len;
}

/* -----------------------------------------------------------------------------
 * DohEncoding()
 *
 * Registers a new printf encoding method.  An encoder function should accept
 * two file-like objects and operate as a filter.
 * ----------------------------------------------------------------------------- */

void 
DohEncoding(char *name, DOH *(*fn)(DOH *s)) {
  if (!encodings) encodings = NewHash();
  Setattr(encodings,(void *) name, NewVoid((void *)fn,0));
}

/* internal function for processing an encoding */
static DOH *encode(char *name,  DOH *s) {
  DOH *handle, *ns;
  DOH *(*fn)(DOH *);
  long  pos;
  if (!encodings || !(handle = Getattr(encodings,name))) {
    return Copy(s);
  }
  pos = Tell(s);
  Seek(s,0,SEEK_SET);
  fn = (DOH *(*)(DOH *)) Data(handle);
  ns = (*fn)(s);
  Seek(s,pos,SEEK_SET);
  return ns;
}

/* -----------------------------------------------------------------------------
 * DohvPrintf()
 *
 * DOH implementation of printf.  Output can be directed to any file-like object
 * including bare FILE * objects.  The same formatting codes as printf are
 * recognized with two extensions:
 *
 *       %s          - Prints a "char *" or the string representation of any
 *                     DOH object.  This will implicitly result in a call to
 *                     Str(obj).
 *
 *       %(encoder)* - Filters the output through an encoding function registered
 *                     with DohEncoder().
 *
 * Note: This function is not particularly memory efficient with large strings.
 * It's better to use Dump() or some other method instead.
 * ----------------------------------------------------------------------------- */

int
DohvPrintf(DOH *so, const char *format, va_list ap)
{
  static char *fmt_codes = "dioxXucsSfeEgGpn";
  int state = 0;
  const char *p = format;
  char  newformat[256];
  char  obuffer[OBUFLEN];
  char *fmt = 0;
  char  temp[64];
  int   widthval = 0;
  int   precval = 0;
  int   maxwidth;
  char  *w = 0;
  int   ivalue;
  double dvalue;
  void  *pvalue;
  char  *stemp;
  int   nbytes = 0;
  char  encoder[128], *ec = 0;

  memset (newformat, 0, sizeof (char *));

  while (*p) {
    switch(state) {
    case 0:  /* Ordinary text */
      if (*p != '%') {
	Putc(*p,so);
	nbytes++;
      } else{
	fmt = newformat;
	widthval = 0;
	precval = 0;
	*(fmt++) = *p;
	encoder[0] = 0;
	state = 10;
      }
      break;
    case 10: /* Look for a width and precision */
      if (isdigit(*p) && (*p != '0')) {
	w = temp;
	*(w++) = *p;
	*(fmt++) = *p;
	state = 20;
      } else if (strchr(fmt_codes,*p)) {
	/* Got one of the formatting codes */
	p--;
	state = 100;
      } else if (*p == '*') {
	/* Width field is specified in the format list */
	widthval = va_arg(ap,int);
	sprintf(temp,"%d",widthval);
	for (w = temp; *w; w++) {
	  *(fmt++) = *w;
	}
	state = 30;
      } else if (*p == '%') {
	Putc(*p,so);
	fmt = newformat;
	nbytes++;
	state = 0;
      } else if (*p == '(') {
	ec = encoder;
	state = 60;
      } else {
	*(fmt++) = *p;
      }
      break;
      
    case 20: /* Hmmm. At the start of a width field */
      if (isdigit(*p)) {
	*(w++) = *p;
	*(fmt++) = *p;
      } else if (strchr(fmt_codes,*p)) {
	/* Got one of the formatting codes */
	/* Figure out width */
	*w = 0;
	widthval = atoi(temp);
	p--;
	state = 100;
      } else if (*p == '.') {
	*w = 0;
	widthval = atoi(temp);
	w = temp;
	*(fmt++) = *p;
	state = 40;
      } else {
	/* ??? */
	*w = 0;
	widthval = atoi(temp);
	state = 50;
      }
      break;

    case 30:   /* Parsed a width from an argument.  Look for a . */
      if (*p == '.') {
	w = temp;
	*(fmt++) = *p;
	state = 40;
      } else if (strchr(fmt_codes,*p)) {
	/* Got one of the formatting codes */
	/* Figure out width */
	p--;
	state = 100;
      } else {
	/* hmmm. Something else. */
	state = 50;
      }
      break;

    case 40:
      /* Start of precision expected */
      if (isdigit(*p) && (*p != '0')) {
	*(fmt++) = *p;
	*(w++) = *p;
	state = 41;
      } else if (*p == '*') {
	/* Precision field is specified in the format list */
	precval = va_arg(ap,int);
	sprintf(temp,"%d",precval);
	for (w = temp; *w; w++) {
	  *(fmt++) = *w;
	}
	state = 50;
      } else if (strchr(fmt_codes,*p)) {
	p--;
	state = 100;
      } else {
	*(fmt++) = *p;
	state = 50;
      }
      break;
    case 41:
      if (isdigit(*p)) {
	*(fmt++) = *p;
	*(w++) = *p;
      } else if (strchr(fmt_codes,*p)) {
	/* Got one of the formatting codes */
	/* Figure out width */
	*w = 0;
	precval = atoi(temp);
	p--;
	state = 100;
      } else {
	*w = 0;
	precval = atoi(temp);
	*(fmt++) = *p;
	state = 50;
      }
      break;
      /* Hang out, wait for format specifier */
    case 50:
      if (strchr(fmt_codes,*p)) {
	p--;
	state = 100;
      } else {
	*(fmt++) = *p;
      }
      break;

      /* Got an encoding header */
    case 60:
      if (*p == ')') {
	*ec = 0;
	state = 10;
      } else {
	*ec = *p;
	ec++;
      }
      break;
    case 100:
      /* Got a formatting code */
      if (widthval < precval) maxwidth = precval;
      else maxwidth = widthval;
      if ((*p == 's') || (*p == 'S')) {       /* Null-Terminated string */
	DOH    *doh;
	DOH    *Sval;
	DOH    *enc = 0;
	doh = va_arg(ap, DOH *);
	if (DohCheck(doh)) {
	  /* Is a DOH object. */
 	  if (DohIsString(doh)) {
	    Sval = doh;
	  } else {
	    Sval = Str(doh);
	  }
	  if (strlen(encoder)) {
	    enc = encode(encoder,Sval);
	    maxwidth = maxwidth+strlen(newformat)+Len(enc);
	  } else {
	    maxwidth = maxwidth+strlen(newformat)+Len(Sval);
	  }
	  *(fmt++) = 's';
	  *fmt = 0;
	  if ((maxwidth + 1) < OBUFLEN) {
	    stemp = obuffer;
	  } else {
	    stemp = (char *) DohMalloc(maxwidth+1);
	  }
	  if (enc) {
	    nbytes+=sprintf(stemp,newformat,Data(enc));
	  } else {
	    nbytes+=sprintf(stemp,newformat,Data(Sval));
	  }
	  if (Writen(so,stemp,strlen(stemp)) < 0) return -1;
	  if ((DOH *) Sval != doh) {
	    Delete(Sval);
	  }
	  if (enc) Delete(enc);
	  if (*p == 'S') {
	    Delete(doh);
	  }
	  if (stemp != obuffer) {
	    DohFree(stemp);
	  }
	} else {
	  if (!doh) doh = "";
	  
	  if (strlen(encoder)) {
	    DOH *s = NewString(doh);
	    Seek(s,0, SEEK_SET);
	    enc = encode(encoder,s);
	    Delete(s);
	    doh = Char(enc);
	  } else {
	    enc = 0;
	  }
	  maxwidth = maxwidth+strlen(newformat)+strlen((char *) doh);
	  *(fmt++) = 's';
	  *fmt = 0;
	  if ((maxwidth+1) < OBUFLEN) {
	    stemp = obuffer;
	  } else {
	    stemp = (char *) DohMalloc(maxwidth + 1);
	  }
	  nbytes+=sprintf(stemp,newformat,doh);
	  if (Writen(so,stemp,strlen(stemp)) < 0) return -1;
	  if (stemp != obuffer) {
	    DohFree(stemp);
	  }
	  if (enc) Delete(enc);
	}
      } else {
	*(fmt++) = *p;
	*fmt = 0;
	maxwidth = maxwidth+strlen(newformat)+64;
	
	/* Only allocate a buffer if it is too big to fit.  Shouldn't have to do
           this very often */

	if (maxwidth < OBUFLEN)
	  stemp = obuffer;
	else 
	  stemp = (char *) DohMalloc(maxwidth+1);
	switch(*p) {
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
	case 'c':
	  ivalue = va_arg(ap,int);
	  nbytes+=sprintf(stemp,newformat,ivalue);
	  break;
	case 'f':
	case 'g':
	case 'e':
	case 'E':
	case 'G':
	  dvalue = va_arg(ap,double);
	  nbytes+=sprintf(stemp,newformat,dvalue);
	  break;
	case 'p':
	  pvalue = va_arg(ap,void *);
	  nbytes+=sprintf(stemp,newformat,pvalue);	  
	  break;
	default:
	  break;
	}
	if (Writen(so,stemp,strlen(stemp)) < 0) return -1;
	if (stemp != obuffer) DohFree(stemp);
      }
      state = 0;
      break;
    }
    p++;
  }
  if (state) {
    int r;
    *fmt = 0;
    r = Writen(so,fmt,strlen(fmt));
    if (r < 0) return -1;
    nbytes += r;
  }
  return nbytes;
}

/* -----------------------------------------------------------------------------
 * DohPrintf()
 *
 * Variable length argument entry point to Printf
 * ----------------------------------------------------------------------------- */

int
DohPrintf(DOH *obj, const char *format, ...) {
  va_list ap;
  int ret;
  va_start(ap,format);
  ret = DohvPrintf(obj,format,ap);
  va_end(ap);
  return ret;
}

/* -----------------------------------------------------------------------------
 * DohPrintv()
 * 
 * Print a null-terminated variable length list of DOH objects
 * ----------------------------------------------------------------------------- */

int DohPrintv(DOHFile *f, ...) {
  va_list ap;
  int ret = 0;
  DOH *obj;
  va_start(ap,f);
  while(1) {
    obj = va_arg(ap,void *);
    if (!obj) break;
    if (DohCheck(obj)) {
      ret += DohDump(obj,f);
    } else {
      ret += DohWrite(f,obj,strlen((char *) obj));
    }
  }
  va_end(ap);
  return ret;
}

/* ----------------------------------------------------------------------------- 
 * DohCopyto()
 *
 * Copies all of the input from an input stream to an output stream. Returns the
 * number of bytes copied.
 * ----------------------------------------------------------------------------- */

int
DohCopyto(DOH *in, DOH *out) {
  int nbytes = 0, ret;
  int nwrite = 0, wret;
  char *cw;
  char buffer[16384];

  if ((!in) || (!out)) return 0;
  while (1) {
    ret = Read(in,buffer,16384);
    if (ret > 0) {
      nwrite = ret;
      cw = buffer;
      while (nwrite) {
	wret = Write(out,cw,nwrite);
	if (wret < 0) return -1;
	nwrite = nwrite - wret;
	cw += wret;
      }
      nbytes += ret;
    } else {
      return nbytes;
    }
  }
}


/* -----------------------------------------------------------------------------
 * DohSplit()
 *
 * Split an input stream into a list of strings delimeted by characters in a 
 * string.  Optionally accepts a maximum number of splits to perform.
 * ----------------------------------------------------------------------------- */

DOH *
DohSplit(DOH *in, const char *chs, int nsplits) {
  DOH *list;
  DOH *str;
  int c;
  
  list = NewList();

  if (DohIsString(in)) {
    Seek(in,0,SEEK_SET);
  }

  while (1) {
    str = NewString("");
    do {
      c = Getc(in);
    } while ((c != EOF) && (c == *chs));
    if (c != EOF) {
      Putc(c,str);
      while (1) {
	c = Getc(in);
	if ((c == EOF) || ((c == *chs) && (nsplits != 0))) break;
	Putc(c,str);
      }
      nsplits--;
    }
    Append(list,str);
    Delete(str);
    if (c == EOF) break;
  }
  return list;
}

/* -----------------------------------------------------------------------------
 * DohReadline()
 *
 * Read a single input line and return it as a string.
 * ----------------------------------------------------------------------------- */

DOH *
DohReadline(DOH *in) {
  char c;
  int n = 0;
  DOH *s = NewString("");
  while (1) {
    if (Read(in,&c,1) < 0) {
      if (n == 0) {
	Delete(s);
	return 0;
      }
      return s;
    }
    if (c == '\n') return s;
    if (c == '\r') continue;
    Putc(c,s);
    n++;
  }
}

  
