/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: conv_stf.c,v 3.20 2002/12/11 16:39:49 dillema Exp $>
 */

#include "totd.h"

#define STF_SUFFIX "\0012\0010\0010\0012\003ip6\003int"
#define STF_PREFIX "2002"
#define STF_BYTES 2

RRset *conv_stf_owner_rrset (RRset *rrs) {
        const char *fn = "conv_stf_owner_rrset()";
	u_char owner[MAX_DNAME], str[MAX_DNAME];
	u_char *own, *ostop, *op;
	int owner_len, i, val;
	RRset *rrs_new = NULL;
	RR_List *rrl;
	char buf[4];

	syslog (LOG_DEBUG, "%s: start", fn);

	if (rrs->key.info->r_type != RT_SOA ||
	    rrs->key.info->r_type != RT_NS)
		return NULL;
	
	/* parse resource record set */
	rrl = rr_list_of_rrset (rrs);
	if (!rrl)
		return NULL;

	own = rrset_owner (rrs);

	/* XXX need a strdname() routine for this */
	ostop = (u_char *) strstr((char *)own, "\007IN-ADDR\004ARPA");
	if (!ostop)
		ostop = (u_char *)strstr((char *) own, "\007in-addr\004arpa");

	if (!ostop) {
		/*
		 * not something we can handle here,
		 * so don't modify just copy
		 */
		owner_len = rrs->key.info->owner_len;
		memcpy(owner, own, owner_len);
		if (T.debug) {
			dname_decompress (str, MAX_DNAME, owner,0,0,0);
			syslog (LOG_DEBUG, "%s: not a v4 PTR name %s", fn, str);
		}
	} else {
		op = owner;
		while (own < ostop) {
			for (i = 0; i < *own; i++)
				buf[i] = *(own + i + 1);

			buf[i] = '\0';
			sscanf(buf, "%u", &val);
			snprintf(buf, sizeof(buf), "%02x", val & 0xff);

			/* length: one nybble is one character */
			*op = *(op+2) = '\1';

			/* least significant nybble */
			*(op+1) = buf[1];
			/* most significant nybble */
			*(op+3) = buf[0];

			op += 4;
			own += *own + 1;
		}

		sprintf((char *)op, "%s", STF_SUFFIX);
		/* point to end (after null char) */
		op += strlen(STF_SUFFIX) + 1;
		owner_len = op - owner;

		if (T.debug) {
			dname_decompress (str, MAX_DNAME, owner, 0,0,0);
			syslog (LOG_DEBUG, "%s: converted PTR %s", fn, owner);
		}
	}

	/* assemble new rrset */
	rrs_new = rrset_create (rrs->key.info->r_type,
				rrs->key.info->r_class,
				owner_len, owner, rrl);
	rr_list_free (rrl);

	syslog (LOG_DEBUG, "%s: rrs_new %x", fn, (unsigned int) rrs_new);
	return (rrs_new);
}

void conv_stf_ns_list (G_List *rrsl) {
        const char *fn = "conv_stf_ns_list()";
        RRset *rrsp, *rrsp_tmp;
	G_List *gl_tmp;

	syslog (LOG_DEBUG, "%s: start", fn);

	rrsp_tmp = NULL;
	rrsl->list_data = NULL;
	for (gl_tmp = rrsl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		rrsp = (RRset *) gl_tmp->list_data;
		syslog (LOG_DEBUG, "%s: type %s", fn,
			string_rtype(rrsp->key.info->r_type));

		if (rrsp->key.info->r_type == RT_SOA)
			rrsp_tmp = conv_stf_owner_rrset (rrsp);
		if (rrsp->key.info->r_type == RT_NS)
			rrsp_tmp = conv_stf_owner_rrset (rrsp);
		if (rrsp_tmp) {
			rrset_free (rrsp);
			gl_tmp->list_data = rrsp_tmp;
			rrsp_tmp = NULL;
		}
	}

	return;
}

int conv_stf_ptr (u_char *qname) {
        const char *fn = "conv_stf_ptr()";
        const int off = sizeof(struct in_addr) + STF_BYTES;
	u_char qname4[MAX_DNAME], str[MAX_DNAME];
	u_char *qn6, *qn4;
	int val, qname_len;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* to small to convert? */
	if (strlen((char *)qname) < 8)
		return -1;

	qn4 = qname4;
	qname_len = strlen((char *)qname) + 1;
	qn6 = qname + qname_len - off*4 - 8;
	if (qn6 < qname)
		qn6 = qname + 1;

	while (qn6 < qname + qname_len - STF_BYTES*4 - 8) {
		val = 0;
                if (isdigit(*qn6))
			val = *qn6 - '0';
                else if (isalpha(*qn6))
                        val = *qn6 - (isupper(*qn6) ? 'A' - 10 : 'a' - 10);
		qn6++;

		if (*qn6 != 1) return -1;
		qn6++;

                if (isdigit(*qn6))
			val += 16 * (*qn6 - '0');
                else if (isalpha(*qn6))
                        val += 16 * (*qn6 - (isupper(*qn6) ? 'A'-10 : 'a'-10));
		qn6++;

		if (*qn6 != 1) return -1;
		qn6++;
 
		*qn4 = snprintf((char *)qn4 + 1, 4, "%u", val & 0xff);
		qn4 = qn4 + *qn4 + 1;
	}
	sprintf((char *)qn4, "%s", "\007in-addr\004arpa");
	strlcpy((char *)qname, (char *)qname4, MAX_DNAME);

	if (T.debug) {
		dname_decompress (str, MAX_DNAME, qname, NULL, NULL, NULL);
		syslog (LOG_DEBUG, "%s: converted name %s", fn, str);
	}

	return 0;
}

/*
 * returns 1 if netprefix of qname equals stf prefix, else returns 0
 */
int conv_stf_is_stf_ptr (u_char *qname) {
	u_char *qname6;

	/* first check it's a PTR name, if not return 0 */
	if (!(strstr((char *)qname, "INT") || strstr((char *)qname, "int")))
		return 0;

	qname6 = (u_char *) strstr((char *)qname, "IP6");
	if (!qname6)
		qname6 = (u_char *) strstr((char *)qname, "ip6");
	if (!qname6)
		return 0;

	qname6 = qname6 - 2;

	if (*qname6-- != '2')
		return 0;
	if (*qname6-- != 1)
		return 0;
	if (*qname6-- != '0')
		return 0;
	if (*qname6-- != 1)
		return 0;
	if (*qname6-- != '0')
		return 0;
	if (*qname6-- != 1)
		return 0;
	if (*qname6-- != '2')
		return 0;
	if (*qname6-- != 1)
		return 0;
	return 1; /* we matched 0x2002 */
}

