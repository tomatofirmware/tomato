/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: tot_constants.h,v 3.17 2003/11/26 09:46:53 dillema Exp $>
 */

#ifndef TOT_CONSTANTS
#define TOT_CONSTANTS

extern const char *hex;

/*
 * TOTD Constants and Default Values
 */
#define TOTPREFIXLEN 24
#define MAXARGS 30

#ifndef MAXINTERFACES
#define MAXINTERFACES MAXARGS
#endif

#ifndef MAXPREFIXES
#define MAXPREFIXES MAXARGS
#endif

#define IF_CHECK_INTERVAL 10

#ifdef DBTCP
#define QUERY_TCP 1
#else
#define QUERY_TCP 0
#endif

/* default pid file -- if NULL no default pid file.*/
#define TOT_PID_FILE "/var/run/totd.pid"

#define SEARCH_CNAME_LEVEL   6	/* maximum levels of cname links we allow */

#define TCP_SRV_TIMEOUT 60      /* Useful value? */
#define SEARCH_REMOTE_TIMEOUT 2	/* timeout in seconds of query to forwarder */
#define SEARCH_REMOTE_RETRY   1	/* number of retransmits */
#define FORWARDER_DEATH_MARK 3

#define PORT_TO 53		/* default port number to forward to */
#define PORT_SRV 53		/* default port number to listen on for requests */
#define MAX_PACKET 512		/* max UDP packet size */
#define MAX_STREAM 65535	/* max TCP stream size */

/*
 * there is no good reason to select these value
 */
#define EV_DUP_TABLE_SIZE 1000
#define EV_DUP_CMP_LEN 16
#define EVENT_LIFETIME 60

/*
 * TOTD Transaction States
 */
#define WORD_NONE 0
#define WORK_DONE 0
#define FORWARD_REQUEST 1
#define TRICK_REQUEST 2
#define PTR_TRICK_REQUEST 3
#define NEWPTR_TRICK_REQUEST 4
#define PTR_SCOPED_TRICK_REQUEST 5
#ifdef STF
#define STF_DONE 0
#define STF_REQUEST 6
#define STF_NSLIST 7
#define STF_FORWARDING 8
#endif

/* Timeout Event Types */
#define EV_TIMEOUT_NULL 0
#define EV_TIMEOUT_CONTEXT 1

/* 
 * DNS constants and RFC defined values
 */
#define DNAME_DELIM '.'		/* domain name delimiter */
#define MAX_DNAME 256		/* max domain name length */
#define MAX_LABEL 63		/* max label length */

/* OPCODE in the requests */
#define OP_QUERY 0		/* standard query */

/* RCODE in the message */
#define RC_OK 0			/* no error */
#define RC_FMTERR 1		/* format error */
#define RC_SERVERERR 2		/* server error */
#define RC_NAMEERR 3		/* name does not exist error */
#define RC_NXDOMAIN 3		/* alias for NAMEERR */
#define RC_NIMP 4		/* not implimented error */
#define RC_REF 5		/* refused error */
#define RC_YXDOMAIN 6		/* <- rfc2136: request domain exists */
#define RC_YXRRSET 7		/* <- rfc2136: request rrset exists */
#define RC_NXRRSET 8		/* <- rfc2136: request rrset not exist */
#define RC_NOTAUTH 9		/* <- rfc2136: non-authority zone */
#define RC_NOTZONE 10		/* <- rfc2136: not-zone error */

/*
 * RR type and query type 
 *    RR means Resource Record
 *    RT means Resource Type
 */ 
#define RT_VOID 0
#define RT_A 1
#define RT_NS 2
#define RT_MD 3
#define RT_MF 4
#define RT_CNAME 5
#define RT_SOA 6
#define RT_MB 7
#define RT_MG 8
#define RT_MR 9
#define RT_NULL 10
#define RT_WKS 11
#define RT_PTR 12
#define RT_HINFO 13
#define RT_MINFO 14
#define RT_MX 15
#define RT_TXT 16
#define RT_RP 17		/* rfc1183: responsible person */
#define RT_AAAA 28		/* rfc1886: for IPv6 */
#define RT_SRV 33		/* rfc2052: SRV */
#define RT_A6 38		/* rfc2874: for IPv6 */
#define RT_DNAME 39
/* non standard */
#define RT_UINFO 100
/* these are only for query */
#define RT_TSIG 250
#define RT_IXFR 251
#define RT_AXFR 252
#define RT_ALL 255

/* Class values */
#define C_IN 1
#define C_NONE 254		/* <- rfc2136 */
/* this is only for query */
#define C_ANY 255

/* Bit mask for domain name compression */
#define DNCMP_MASK 0xc0
#define DNCMP_MASK_INT16T 0xc000
#define DNCMP_REDIRECT_LIMIT (0x3000-1)

/* for bit string label */
#define EDNS0_MASK 0x40
#define EDNS0_ELT_BITLABEL 0x41

#endif
