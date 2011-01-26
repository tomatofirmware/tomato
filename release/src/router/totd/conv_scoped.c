/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/


/*
 * <$Id: conv_scoped.c,v 1.31 2003/01/27 16:22:46 dillema Exp $>
 */

#include "totd.h"

#ifdef SCOPED_REWRITE

static RRset *conv_scoped_rrset (RRset *);

int conv_scoped_query(Context *context) {
	struct sockaddr_in6 *me;
	struct sockaddr_in6 *peer;
	static int warned = 0;
	Nia *ni;

	syslog(LOG_DEBUG, "conv_scoped_query(): start");

	ni = context->netifaddr;
	if (!ni || !ni->sa_p)
		return 0;

	me =(struct sockaddr_in6 *)ni->sa_p;
	if (me->sin6_family != AF_INET6)
		return 0;

	if (IN6_ARE_ADDR_EQUAL(&me->sin6_addr, &in6addr_any)) {
		/* me is wildcard, so lookup our address */
		if (!warned)
			syslog(LOG_WARNING, "Scoped rewriting not \
implemented for wildcard sockets");
		warned = 1;
		return 0;
	}

	peer = (struct sockaddr_in6 *)context->peer;
	if (peer->sin6_family != AF_INET6)
		return 0;

	if (IN6_IS_ADDR_LINKLOCAL(&me->sin6_addr) &&
	    IN6_IS_ADDR_LINKLOCAL(&peer->sin6_addr)) {
#ifdef HAVE_SIN6_SCOPE_ID
	    	if (me->sin6_scope_id == peer->sin6_scope_id)
			return 1; /* good */
		if (me->sin6_scope_id)
			return 0;
		/*
		 * XXX this is horrible hack to handle case where
		 * scope_id is in sockaddr, and not in sin6_scope_id
		 * due to problem with SIOCGIFCONF ioctl. I have this
		 * problem on my test machine, so I added this code
		 * to be able to test it all. XXX This code should
		 * be removed again in the future.
		 */
		if (!warned)
			syslog(LOG_WARNING, "Need hack around your \
sin6_scope_id for SIOCGIFCONF!");
		warned = 1;

	    	if ((&me->sin6_addr)->s6_addr[3] == peer->sin6_scope_id)
			return 1; /* good */
		return 0;
#endif
		/* when we don not know, do rewrite */
		return 1;
	}

	if (IN6_IS_ADDR_SITELOCAL(&me->sin6_addr) &&
	    IN6_IS_ADDR_SITELOCAL(&peer->sin6_addr)) {
#ifdef HAVE_SIN6_SCOPE_ID
	    	if (me->sin6_scope_id == peer->sin6_scope_id)
			return 1; /* good */
		if (me->sin6_scope_id)
			return 0;
		/*
		 * XXX this is horrible hack to handle case where
		 * scope_id is in sockaddr, and not in sin6_scope_id
		 * due to problem with SIOCGIFCONF ioctl. I have this
		 * problem on my test machine, so I added this code
		 * to be able to test it all. XXX This code should
		 * be removed again in the future.
		 */
		if (!warned)
			syslog(LOG_WARNING, "Need hack around your \
sin6_scope_id for SIOCGIFCONF!");
		warned = 1;

	    	if ((&me->sin6_addr)->s6_addr[3] == peer->sin6_scope_id)
			return 1; /* good */
		return 0;
#endif
		/* when we don not know, do rewrite */
		return 1;
	}

	return 0;
}

static RRset *conv_scoped_rrset(RRset *rrs_old) {
	RR_List *rrl = NULL, *rrl_tmp = NULL;
	RRset *ret_val = NULL, *rrs_new = NULL;
	u_char *rd = NULL, *rd_new = NULL;
	RR_List sentinel, *cur;
	int i;

	syslog(LOG_DEBUG, "conv_scoped_rrset: start");

	if (!T.scoped_prefixes) {
		ret_val = NULL;
		goto finish;
	}

	memset(&sentinel, 0, sizeof(sentinel));
	cur = &sentinel;

	if (rrs_old->key.info->r_type != RT_AAAA) {
		ret_val = NULL;
		goto finish;
	}
	/* parse A rrset */
	rrl = rr_list_of_rrset(rrs_old);
	if (!rrl) {
		ret_val = NULL;
		goto finish;
	}
	/* convert each RR_List data */
	for (rrl_tmp = rrl; rrl_tmp->next; rrl_tmp = rrl_tmp->next) {
		rd = rr_rdata(rrl_tmp->rrp);

		for (i = 0; i < T.scoped_prefixes; i++) {
			if (!memcmp(rd, &T.scoped_from[i], T.scoped_plen[i]/8))
				break;
		}
		if (i >= T.scoped_prefixes)
			continue;

		/* make a duplicate */
		cur->next = rr_list_alloc();
		*(cur->next) = *rrl_tmp;
		cur->next->rrp = NULL;
		cur->next->next = NULL;

		rd_new = malloc(sizeof(struct in6_addr));
		if (!rd_new) {
			rr_list_free(cur->next);
			cur->next = NULL;
			continue;
		}

		memcpy(rd_new, &T.scoped_to[i], T.scoped_plen[i] / 8);
		memcpy(rd_new + T.scoped_plen[i] / 8, rd + T.scoped_plen[i] / 8,
		       sizeof(struct in6_addr) - T.scoped_plen[i] / 8);

		cur->next->rrp = rr_alloc(rrl_tmp->rrp->ttl,
					  sizeof(struct in6_addr), rd_new);

		if (rd_new)
			free (rd_new);
		rd_new = NULL;

		if (!cur->next->rrp) {
			rr_list_free(cur->next);
			cur->next = NULL;
			continue;
		}

		while (cur && cur->next)
			cur = cur->next;
	}

	cur->next = rrl;

	/* why do we need to count like this? */
	i = 0;
	for (cur = sentinel.next; cur; cur = cur->next)
		i++;
	i -= 2;
	for (cur = sentinel.next; cur; cur = cur->next)
		cur->cnt = i--;

	/* assemble AAAA rrset */
	rrs_new = rrset_create(RT_AAAA, rrs_old->key.info->r_class,
				 rrs_old->key.info->owner_len,
				 rrset_owner(rrs_old), sentinel.next);
	if (!rrs_new) {
		ret_val = NULL;
		goto finish;
	}

	/* set return value */
	ret_val = rrs_new;
	rrs_new = NULL;

finish:
	if (rrs_new)
		rrset_free(rrs_new);
	if (sentinel.next)
		rr_list_free(sentinel.next);
	if (rd_new)
		free(rd_new);

	syslog(LOG_DEBUG, "conv_scoped_rrset: return");
	return ret_val;
}

void conv_scoped_list(G_List *rrsl) {
	const char *fn = "conv_scoped_list()";
        RRset *rrsp, *rrsp_new;
	G_List *gl_tmp;

	syslog(LOG_DEBUG, "%s: start", fn);

	rrsl->list_data = NULL;
	for (gl_tmp = rrsl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		rrsp =(RRset *)gl_tmp->list_data;
		if (rrsp->key.info->r_type != RT_AAAA)
			continue;

		/* add scoped address if necessary */
		rrsp_new = conv_scoped_rrset(rrsp);
		if (!rrsp_new) {
			syslog(LOG_ERR, "Can't add scoped address");
		} else {
			rrset_free(rrsp);
			gl_tmp->list_data = rrsp_new;
			rrsp_new = NULL;
		}
	}

	syslog(LOG_DEBUG, "%s: return", fn);
	return;
}

int conv_scoped_conf(const char *from, const char *to, int plen) {
	const char *fn = "conv_scoped_conf()";
	const int max = sizeof(T.scoped_from)/sizeof(T.scoped_from[0]);

	if (T.scoped_prefixes >= max) {
		syslog(LOG_ERR, "%s: max number of %d prefixes exceeded",
		       fn, max);
		return(-1);
	}
	if (plen % 8) {
		syslog(LOG_ERR, "%s: plen needs to be multiple of 8", fn);
		return(-1);
	}

	if (inet_pton(AF_INET6, from, &T.scoped_from[T.scoped_prefixes]) != 1) {
		syslog(LOG_ERR, "invalid format: from %s", from);
		return(-1);
	}
	if (inet_pton(AF_INET6, to, &T.scoped_to[T.scoped_prefixes]) != 1) {
		syslog(LOG_ERR, "invalid format: to %s", to);
		return(-1);
	}
	T.scoped_plen[T.scoped_prefixes] = plen;

	syslog(LOG_DEBUG, "%s: %s %s", fn, from, to);

	T.scoped_prefixes++;
	return(0);
}

void conv_scoped_ptr(G_List *rrsl, u_char *qname) {
	G_List *gl_tmp;
	U_Key rk;
	int idx;
	u_char *p, *q;
	int off;

	syslog(LOG_DEBUG, "conv_scoped_ptr: start.");

	if (!T.scoped_prefixes) {
		syslog (LOG_ERR, "No scoped prefix configured!");
		return;
	}

	idx = conv_is_scoped_ptr(qname, 1);
	if (idx == -1)
		return;

	rrsl->list_data = NULL;
	for (gl_tmp = rrsl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		rk = ((RRset *) gl_tmp->list_data)->key;
		if (rk.info->r_type != RT_PTR)
			continue;

		/* convert PTR record from in-addr.arpa to ip6.int */
		p = rk.p + KEYINFO_HEAD_LEN;
		q = &T.scoped_to[idx].s6_addr[0];
		for (off = 0; off < T.scoped_plen[idx] / 8; off++) {
			p[64 - 4 - off * 4 + 3] = hex[(q[off] >> 4) & 0x0f];
			p[64 - 4 - off * 4 + 1] = hex[q[off] & 0x0f];
		}
	}

	syslog (LOG_DEBUG, "conv_scoped_ptr: return");
	return;
}

/* 
 * returns index of scoped rewrite, if netprefix of qname equals one of scoped
 * prefixes.  returns -1 if not.
 *
 * to argument: 0: from 1: to
 */
int conv_is_scoped_ptr(u_char *qname, int to) {
	u_char *qname6, *q6;
	struct in6_addr a6;
	u_char *p;
	unsigned int val;
	char buf[3];
	int i;

	if (!T.scoped_prefixes || *qname == '\0')
		return -1;

	if (!(strstr((char *)qname, "INT") || strstr((char *)qname, "int")))
		return -1;
	qname6 = (u_char *)strstr((char *)qname, "IP6");
	if (!qname6)
		qname6 = (u_char *)strstr((char *)qname, "ip6");
	if (!qname6)
		return -1;

	memset(&a6, 0, sizeof(a6));
	qname6--;
	if (qname6 - qname != 64)
		return -1;
	qname6--;
	q6 = qname6;
	p = &a6.s6_addr[0];
	while (q6 - 3 >= qname) {
		buf[0] = *q6--;
		if (*q6-- != 1)
			return -1;
		buf[1] = *q6--;
		if (*q6-- != 1)
			return -1;
		buf[2] = '\0';
		sscanf(buf, "%x", &val);
		*p = val & 0xff;

		p++;
	}

	for (i = 0; i < T.scoped_prefixes; i++) {
		if (!memcmp(&a6, to ? &T.scoped_to[i] : &T.scoped_from[i],
				T.scoped_plen[i] / 8))
			return i;
	}
	return -1;
}

void conv_scoped_ptr_rq(u_char *qname) {
	u_char *p, *q;
	int off, idx;

	idx = conv_is_scoped_ptr(qname, 1);
	if (idx == -1)
		return;

	/* convert PTR record from in-addr.arpa to ip6.int */
	p = qname;
	q = &T.scoped_from[idx].s6_addr[0];
	for (off = 0; off < T.scoped_plen[idx] / 8; off++) {
		p[64 - 4 - off * 4 + 3] = hex[(q[off] >> 4) & 0x0f];
		p[64 - 4 - off * 4 + 1] = hex[q[off] & 0x0f];
	}
}
#endif
