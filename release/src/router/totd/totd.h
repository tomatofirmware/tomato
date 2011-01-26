/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: totd.h,v 3.41 2005/01/29 18:51:36 dillema Exp $>
 */

#ifndef TOT_H
#define TOT_H

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>
#include <signal.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#include <string.h>
#include <assert.h>
#include <netdb.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <sys/socket.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#include <sys/termios.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <setjmp.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif 

#ifdef HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifdef __FreeBSD__
#include <sys/param.h>
#define socklen_t int
#endif

#ifdef DBMALLOC
#include <malloc.h>
#endif

#ifdef SWILL
#include <swill.h>
#endif

#include "macros.h"
#include "tot_constants.h"
#include "tot_types.h"

/*
 * TOTD global config and state variables
 */
struct ToT {
	/* filenames of config and pid file */
	char *configfile;	
	char *pidfile;
	/* when we do not run as root all the time, we run as: */
	char *user;		/* username */
	char *group;		/* groupname */
	char *rootdir;		/* dir to chroot to */
	uid_t uid;		/* user id  */
	gid_t gid;		/* group id */
	/* numeric values */
	int quiet; 		/* how much to (sys-)log */
	int debug; 		/* debugging level, 0 == no debugging output */
	int port;		/* the port we listen at for requests */
	int retry_interval;	/* how long to wait til retry forwarder */
	int http_port;		/* the port we listen at for http requests */
	/* boolean flags */
	int ip4;		/* do we do IPv4 */
	int ip6;		/* do we do IPv6 */
	int use_mapped;		/* do we do IPv4 over IPv6-AF socket */
	int wildcard;		/* do we open wildcard (UDP) socket or one per address? */
	int stf;		/* 6to4 reverse lookup support enable/disable */
	int tcp;		/* TCP fallover from UDP enable/disable */
	int rescan_iflist;	/* do we rescan for new addresses/interfaces??? */
	/* list of prefixes to cycle through (round-robin) */
	int prefixnum;		/* number of configured prefixes */
	u_char prefix[MAXPREFIXES][TOTPREFIXLEN + 1];
	/* list of configured forwarders (`normal' recursive nameservers) */
	G_List *Fwd_list;
	/* current state */
	G_List *current_fwd;		/* nameserver we currently forward our requests to */
	int current_prefix;		/* index into tot_prefix above */
	char *iflist[MAXINTERFACES+1];	/* null terminated list of interface names */
#ifdef SCOPED_REWRITE
	/* list of configured scoped prefixes */
	int scoped_prefixes;
	struct in6_addr scoped_from[MAXPREFIXES];
	struct in6_addr scoped_to[MAXPREFIXES];
	int scoped_plen[MAXPREFIXES];
#endif
};

extern struct ToT T;

#include "protos.h"

#endif				/* TOT_H */
