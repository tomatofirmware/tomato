/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

#define CVSID(string) \
        static const char cvsid[] __attribute__((__unused__)) = string

/*
  macro MAXNUM(x,y);
  return maximum number of x and y.
  */
#define MAXNUM(x,y) (((x)>(y)) ? (x) : (y))

/*
 * Inline versions of get/put short/long;
 * they are copied from BIND destribution
 *
 * Pointer is advanced; we assume that both arguments
 * are lvalues and will already be in registers.
 * cp MUST be u_char *.
 */
#define GETSHORT(us, ucp) { \
	(us) = *(ucp)++ << 8; \
	(us) |= *(ucp)++; \
}

#define GETLONG(ul, ucp) { \
	(ul) = *(ucp)++ << 8; \
	(ul) |= *(ucp)++; (ul) <<= 8; \
	(ul) |= *(ucp)++; (ul) <<= 8; \
	(ul) |= *(ucp)++; \
}

#define PUTSHORT(us, ucp) { \
	*(ucp)++ = (us) >> 8; \
	*(ucp)++ = (us); \
}

/*
 * Warning: PUTLONG destroys its first argument.
 */
#define PUTLONG(ul, ucp) { \
	(ucp)[3] = ul; \
	(ucp)[2] = (ul >>= 8); \
	(ucp)[1] = (ul >>= 8); \
	(ucp)[0] = ul >> 8; \
	(ucp) += sizeof(uint32_t); \
}

/*
 * get size of sockaddr structure
 */
#ifdef HAVE_SA_LEN_FIELD
#define SOCKADDR_SIZEOF(sa) (MAXNUM((sa).sa_len, sizeof(sa)))
#else
#ifdef SA_LEN
#define SOCKADDR_SIZEOF(sa) SA_LEN(&(sa))
#else
/* Use ugly hack from USAGI project */
#define SOCKADDR_SIZEOF(sa) (((sa).sa_family == AF_INET6) \
	? sizeof(struct sockaddr_in6) \
	: (((sa).sa_family == AF_INET) \
		? sizeof(struct sockaddr_in) \
		: sizeof(struct sockaddr)))           
#endif
#endif

#ifdef USE_INET6
#define V6(x) x
#else
#define V6(x)
#endif

#ifdef USE_INET4
#define V4(x) x
#else
#define V4(x)
#endif
