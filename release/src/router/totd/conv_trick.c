/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: conv_trick.c,v 3.47 2002/12/11 16:39:49 dillema Exp $>
 */

#include "totd.h"

static RRset *conv_trick_rrset (RRset *, uint16_t, int);

static RRset *conv_trick_rrset (RRset *rrs_a, uint16_t qtype, int pref) {
        const char *fn = "conv_trick_rrset()";
	RR_List *rrl = NULL, *rrl_tmp;
	RRset *rrs_new = NULL;
	RR *rr_a = NULL, *rr_new = NULL;
	u_char *rd, *rd_new = NULL;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* parse A resource record set */
	rrl = rr_list_of_rrset (rrs_a);
	if (!rrl)
		return NULL;


	/* convert each RR_List data */
	for (rrl_tmp = rrl; rrl_tmp->next; rrl_tmp = rrl_tmp->next) {
		int rdlen;
		u_char *rd_addr;

		rr_a = rrl_tmp->rrp;
		rrl_tmp->rrp = NULL; /* kept in rr_a */
		rd = rr_rdata (rr_a);

		rdlen = sizeof(struct in6_addr);
		if (qtype == RT_A6)
			rdlen++; /* size of prefix length */

		rd_new = malloc (rdlen);
		if (!rd_new)
			break;

		rd_addr = (qtype == RT_AAAA) ? rd_new : (rd_new + 1);

		memcpy (rd_addr, T.prefix[pref], 12);
		memcpy (rd_addr + 12, rd, 4);

		if (qtype == RT_A6)
			*rd_new = 0; /* plen is 0 */

		rr_new = rr_alloc (rr_a->ttl, rdlen, rd_new);
		if (!rr_new)
			break;

		free (rd_new);
		rd_new = NULL;	/* kept in rr_new */
		rrl_tmp->rrp = rr_new;
		rr_new = NULL;	/* kept in rrl_tmp->rrp */

		free (rr_a);
		rr_a = NULL;
	}

	/* assemble A6/AAAA rrset */
	rrs_new = rrset_create (qtype, rrs_a->key.info->r_class,
				rrs_a->key.info->owner_len,
				rrset_owner (rrs_a), rrl);

	/* cleanup */
	if (rr_a)
		free (rr_a);
	if (rr_new)
		free (rr_new);
	if (rrl)
		rr_list_free (rrl);
	if (rd_new)
		free (rd_new);

	return rrs_new;
}

/* target_rtype should be either RT_A6 or RT_AAAA */
void conv_trick_list (G_List *rrsl, int target_rtype, int add) {
	const char *fn = "conv_trick_list()";
	RRset *rrsp, *rrsp_aaaa;
	u_char str[MAX_DNAME];
	G_List *gl;

	syslog (LOG_DEBUG, "%s: start", fn);

	rrsl->list_data = NULL;
	for (gl = rrsl->next; gl->list_data; gl = gl->next) {
		RRset *dup;
		char *name;
		int len;

		rrsp = (RRset *) gl->list_data;
		if (rrsp->key.info->r_type != RT_A)
			continue;

		name = rrset_owner(rrsp);
		len = rrsp->key.info->owner_len;

		dup = search_name(rrsl, name, len, target_rtype);
		if (dup) {
			if (T.debug > 3) {
				dname_decompress (str, MAX_DNAME, name, 0,0,0);
				syslog (LOG_DEBUG, "%s: duplicate %s", fn, str);
			}
			rrset_free(dup);
			continue;
		}

		/* convert A record into faked target_rtype record */
		if (T.debug > 3) {
			dname_decompress (str, MAX_DNAME, name, 0,0,0);
			syslog (LOG_DEBUG, "%s: converting: %s", fn, str);
		}
		rrsp_aaaa = conv_trick_rrset(rrsp, target_rtype,
					     T.current_prefix);
		if (!rrsp_aaaa) {
			syslog (LOG_ERR, "%s: Can't convert A to AAAA", fn);
			continue;
		}

		if (T.debug) {
			dname_decompress (str, MAX_DNAME, name, 0,0,0);
			syslog (LOG_DEBUG, "%s: %s %s", fn,
				add ? "add" : "replace by", str);
		}

		if (add) {
			list_add (rrsl, rrsp_aaaa);
		} else {
			rrset_free (rrsp);
			gl->list_data = rrsp_aaaa;
		}
	}

	return;
}

void conv_trick_ptr (G_List *rrsl, u_char *qname) {
        const char *fn = "conv_trick_ptr()";
	U_Key rk, rk_new;
	G_List *gl;

	syslog (LOG_DEBUG, "%s: start", fn);

	if (*qname == '\0')
		return;

	rrsl->list_data = NULL;
	for (gl = rrsl->next; gl->list_data; gl = gl->next) {
		rk = ((RRset *) gl->list_data)->key;
		if (rk.info->r_type == RT_PTR) {
			/* convert PTR record from in-addr.arpa to ip6.int */
			rk_new.p = malloc (KEYINFO_HEAD_LEN +
					   strlen((char *)qname) + 1);
			if (rk_new.p) {
				rk_new.info->r_type = RT_PTR;
				rk_new.info->r_class = C_IN;
				strcpy((char *)(rk_new.p + KEYINFO_HEAD_LEN),
				       (char *)qname);
				free (rk.p);
				((RRset *) gl->list_data)->key = rk_new;
			} else
				syslog (LOG_ERR, "Cannot allocate memory");
		}
	}

	return;
}

void conv_trick_newptr (G_List *rrsl, u_char *qname) {
        const char *fn = "conv_trick_newptr()";
	G_List *gl;
	U_Key rk, rk_new;
	size_t qlen;

	syslog (LOG_DEBUG, "%s: start", fn);

	if (*qname == '\0')
		return;

	rrsl->list_data = NULL;
	for (gl = rrsl->next; gl->list_data;
	     gl = gl->next) {
		rk = ((RRset *) gl->list_data)->key;
		if (rk.info->r_type == RT_PTR) {
			/*
			 * convert PTR record from in-addr.arpa to
			 * ip6.arpa.
			 */
			qlen = strlen(qname) + 1;
			rk_new.p = malloc (KEYINFO_HEAD_LEN + qlen);
			if (rk_new.p) {
				rk_new.info->r_type = RT_PTR;
				rk_new.info->r_class = C_IN;
				memcpy(rk_new.p+KEYINFO_HEAD_LEN, qname, qlen);
				free (rk.p);
				((RRset *) gl->list_data)->key = rk_new;
			} else
				syslog (LOG_ERR, "Out of memory");
		}
	}

	return;
}

int conv_trick_conf (u_char *v6addr) {
        const char *fn = "conv_trick_conf()";

	if (T.prefixnum >= MAXPREFIXES)
		syslog (LOG_ERR, "%s: max number of %d prefixes exceeded",
			fn, MAXPREFIXES);

	if (inet_pton (AF_INET6, (char *)v6addr,
		       (void *)T.prefix[T.prefixnum]) == 1) {
		syslog (LOG_DEBUG, "%s: %s", fn, v6addr);

		/* SUCCESS */
		T.prefixnum++;
		return (0);
	} else
        	syslog (LOG_ERR, "%s: invalid IPv6 prefix: %s", fn, v6addr);

	/* FAILURE */
	return (-1);
}

/* 
 * returns 1 if netprefix of qname equals one of tot prefixes, else returns 0
 */
int conv_trick_is_tot_ptr (u_char *qname) {
	struct in6_addr a6;
	u_char *qname6, *q6;
	u_char *p, buf[3];
	unsigned int val;
	int i;
	const int cmpsiz = sizeof(struct in6_addr) - sizeof(struct in_addr);

	if (!T.prefixnum || *qname == '\0')
		return 0;

	if (!(strstr((char *)qname, "INT") || strstr((char *)qname, "int")))
		return 0;

	qname6 = (u_char *) strstr((char *)qname, "IP6");
	if (!qname6)
		qname6 = (u_char *) strstr((char *)qname, "ip6");
	if (!qname6)
		return 0;

	memset(&a6, 0, sizeof(a6));
	qname6--;
	if (qname6 - qname != 64)
		return 0;
	qname6--;
	q6 = qname6;
	p = &a6.s6_addr[0];
	while (q6 - 4 >= qname) {
		buf[0] = *q6--;
		if (*q6-- != 1)
			return 0;
		buf[1] = *q6--;
		if (*q6-- != 1)
			return 0;
		buf[2] = '\0';
		sscanf((char *)buf, "%x", &val);
		*p++ = val & 0xff;
	}

	for (i = 0; i < T.prefixnum; i++) {
		if (memcmp(&a6, &T.prefix[i], cmpsiz) == 0)
			return 1; /* got a match */
	}
	return 0;
}


void conv_trick_ptr_rq (u_char *qname) {
    u_char *qname6, tmpaddr[8];
    int len, i, tmp;

    if (*qname == '\0')
	return;

    len = 0;
    qname6 = qname;
    while (len < 8) {
	strncpy ((char *) tmpaddr + len, (char *) qname6 + 1, (int) *qname6);
	len += (int) *qname6;
	qname6 += ((int) *qname6) + 1;
    }
    for (i = 0; i < 8; i++) {
	if (isdigit (tmpaddr[i]))
	    tmpaddr[i] -= '0';
	else if (isalpha (tmpaddr[i]))
	    tmpaddr[i] -= isupper (tmpaddr[i]) ? 'A' - 10 : 'a' - 10;
    }
    for (qname6 = qname, i = 0; i < 8; i += 2) {
	tmp = tmpaddr[i] + 16 * tmpaddr[i + 1];
	len = snprintf ((char *) qname6 + 1, 4, "%d", tmp);
	*qname6 = len;
	qname6 += len + 1;
    }
    *qname6 = '\0'; /* null terminate */
    strlcat ((char *) qname, "\007in-addr\004arpa", MAX_DNAME);

    return;
}

/*
 * Similar routines as above, but for bitstring label + ip6.arpa cases.
 * We assume qname has already been validated.
 */
int conv_trick_is_tot_newptr (u_char *qname, struct in6_addr *a6p) {
	const int cmpsiz = sizeof(struct in6_addr) - sizeof(struct in_addr);
	int ip6len, arpalen, i;

	if (!T.prefixnum || *qname == '\0')
		return 0;

	ip6len = strlen("IP6");
	arpalen = strlen("ARPA");

	/* we consider "\[x.../128\].ip6.arpa." only */
	if (*qname != EDNS0_ELT_BITLABEL || *(qname + 1) != 128)
		return 0;

	/*
	 * Copy the 128 bits (16 bytes) ipv6 binary address label for
	 * later use by caller.
	 */
	memcpy(a6p, qname + 2, sizeof(*a6p));

	/* check the `tail', does it end in IP6.ARPA? */
	qname += (2 + sizeof(struct in6_addr)); /* ELT + bitlen + address */

	if (*qname != ip6len || (strncasecmp((char *)qname + 1, "IP6", ip6len)
	    && strncasecmp((char *)qname + 1, "ip6", ip6len)))
		return 0;
	else
		qname += ip6len + 1;

	if (*qname != arpalen ||
	    (strncasecmp((char *)qname + 1, "ARPA", arpalen) &&
	     strncasecmp((char *)qname + 1, "arpa", arpalen)))
		return 0;

	qname += arpalen + 1;
	if (*qname != '\0')
		return 0;

	/* name is OK, now check our list of prefixes */
	for (i = 0; i < T.prefixnum; i++) {
		if (!memcmp(a6p, &T.prefix[i], cmpsiz))
			return 1; /* got a match */
	}

	return 0;
}

void conv_trick_newptr_rq (u_char *qname, struct in6_addr *a6) {
	int len, i;

	if (*qname == '\0')
		return;

	/*
	 * we take the lower 32 bits (4 bytes) of the ipv6 address as
	 * ipv4 address, and construct a IPv4 PTR record from it
	 */
	for (i = 0; i < 4; i++) {
		len = snprintf((char *)qname + 1, 4, "%d", a6->s6_addr[15 - i]);
		*qname = len;
		qname += len + 1;
	}
	strcpy ((char *)qname, "\007in-addr\004arpa\0");

	return;
}

/*
 * Search name record in specified record list for given NS record,
 * i.e. find the address record, if any, for the given nameserver.
 */
RRset *search_name (G_List *rr_list, char *name, int len, int r_type_search) {
        const char *fn = "search_name()";
	u_char *tmp_domain;
	G_List* gl;
	RRset *rrs;

	syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * search for address record (A or AAAA) of the nameserver
	 * (of NS record) in AR list
	 */
	rr_list->list_data = NULL; /* extra termination insurance */
	for (gl = rr_list->next; gl->list_data; gl = gl->next) {
		rrs = (RRset*)gl->list_data;
		tmp_domain =  rrset_owner(rrs);
		if (rrs->key.info->r_type == r_type_search &&
		    rrs->key.info->owner_len == len &&
		    !mesg_dname_cmp(NULL, tmp_domain, name))
			return rrset_copy(rrs);
	}

	/* no record found */
	return NULL;
}
