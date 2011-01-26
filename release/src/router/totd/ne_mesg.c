/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ne_mesg.c,v 3.48 2002/12/10 22:31:18 dillema Exp $>
 */

#include "totd.h"

static int write_dname (u_char *, u_char *, uint16_t *, int, u_char *, u_char *);
static int dname_copy (u_char *, u_char *, int);
static u_char *dname_redirect (u_char *, u_char *);
static u_char *mesg_read_sec (G_List *, u_char *, int, u_char *, int);

uint16_t mesg_id (void) {
	static uint16_t id = 0;

	if (!id) {
		srandom (time (NULL));
		id = random ();
	}
	id++;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "mesg_id() = %d", id);
	return id;
}

int mesg_make_query (u_char *qname, uint16_t qtype, uint16_t qclass,
		     uint32_t id, int rd, u_char *buf, int buflen) {
	char *fn = "mesg_make_query()";
	u_char *ucp;
	int i, written_len;
	Mesg_Hdr *hdr;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: (qtype: %s, id: %d): start", fn,
			string_rtype (qtype), id);

	hdr = (Mesg_Hdr *) buf;

	/* write header */
	hdr->id = id;
	hdr->opcode = OP_QUERY;
	hdr->rcode = RC_OK;
	hdr->rd = rd;
	hdr->qr = hdr->aa = hdr->tc = hdr->ra = hdr->zero = 0;
	hdr->qdcnt = ntohs (1);
	hdr->ancnt = hdr->nscnt = hdr->arcnt = ntohs (0);

	written_len = sizeof (Mesg_Hdr);
	ucp = (u_char *) (hdr + 1);

	/* write qname */
	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: qname offset = %d", fn, ucp - buf);

	i = dname_copy (qname, ucp, buflen - written_len);
	if (i < 0)
		return -1;

	written_len += i;
	ucp += i;

	/* write qtype / qclass */
	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: qtype/qclass offset = %d",
			fn, ucp - buf);

	written_len += sizeof (uint16_t) * 2;
	if (written_len > buflen)
		return -1;

	PUTSHORT (qtype, ucp);
	PUTSHORT (qclass, ucp);

	return written_len;
}

int labellen (const u_char *cp) {
	uint i;

	i = *cp;
	if ((i & DNCMP_MASK) == 0)
		return(i);
	else if ((i & DNCMP_MASK) == EDNS0_MASK) {
		uint bitlen;

		if (i != EDNS0_ELT_BITLABEL)
			return -1;

		bitlen = *(cp + 1);
		if (bitlen == 0)
			bitlen = 256;

		return (((bitlen + 7) / 8) + 1);
	} else
		return -1;
}

u_char *mesg_skip_dname (u_char *dname, u_char *end) {
	int l;

	if (dname >= end)
		return NULL;

	while(*dname) {
		if ((*dname & DNCMP_MASK) == DNCMP_MASK) {
			dname += 2;	/* redirection */
			return dname;
		}
		if (dname + 2 > end) /* we need at least 2 bytes */
			return NULL;

		l = labellen(dname);
		if (l < 0)
			return NULL;
		dname += l + 1;

		if (dname >= end)
			return NULL;
	}
	dname++;	/* push away null terminator */
	return dname;
}

int mesg_dname_cmp (u_char *msg, u_char *dname_mesg, u_char *dname) {

	/*
	 * compare dname_mesg and dname label by label, while doing
	 * decompression of the dname_mesg of the message msg.
	 */
	dname_mesg = dname_redirect (dname_mesg, msg);
	while (*dname_mesg != '\0' && (*dname == *dname_mesg)) {
		int len;

		len  = labellen(dname_mesg);
		if (len != labellen(dname))
			return -1;

		if (*dname == EDNS0_ELT_BITLABEL) {
			if (memcmp (dname_mesg + 1, dname + 1,  len))
				return -1;
		} else if (strncasecmp (dname_mesg + 1, dname + 1, len))
				return -1;

		dname += len + 1;
		dname_mesg += len + 1;
		dname_mesg = dname_redirect (dname_mesg, msg);
	}

	if (*dname != *dname_mesg)
		return -1;
	else
		return 0;
}

int mesg_write_rrset_list (G_List *rrls, u_char *msg, u_char *msg_tail,
			   uint16_t *dnames, int dnames_len, u_char **wp,
			   uint16_t *cnt) {
	char *fn = "mesg_write_rrset_list()";
	u_char *wp_start, *wp_period;
	Mesg_Hdr *hdr;
	uint16_t us;
	uint32_t ul;
	RRset *rrsp;
	RR *rrp;
	int i, ret;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: start.", fn);

	if (!rrls)
		return 0;

	wp_start = *wp;
	hdr = (Mesg_Hdr *) msg;

	for (rrls = rrls->next; rrls->list_data; rrls = rrls->next) {
		if (T.debug > 4)
			syslog (LOG_DEBUG, "%s: write a record", fn);

		rrsp = (RRset *) rrls->list_data;
		for (i = 0; i < rrsp->data.d->data_cnt; i++) {
			wp_period = *wp;

			/* write the owner name */
			ret = write_dname (msg, msg_tail, dnames,
				dnames_len, rrset_owner (rrsp), *wp);
			if (ret < 0) {
				syslog (LOG_DEBUG, "write ownername failed");
				*wp = wp_period;
				return wp_period - wp_start;
			}
			*wp += ret;

			/* write RR field and data */
			rrp = (RR *) (rrsp->data.p +
				      data_offset (i, rrsp->data.p));
			if (*wp + sizeof (uint16_t) * 3 + sizeof (uint32_t)
			    + rrp->rd_len > msg_tail) {
				syslog (LOG_DEBUG, "write rdata failed");
				*wp = wp_period;
				return wp_period - wp_start;
			}

			PUTSHORT (rrsp->key.info->r_type, *wp);
			PUTSHORT (rrsp->key.info->r_class, *wp);
			ul = rrp->ttl;
			PUTLONG (ul, *wp);

			/* XXX RDATA COMPRESSION NOT IMPLEMENTED */
			PUTSHORT (rrp->rd_len, *wp);
			memcpy (*wp, rr_rdata (rrp), rrp->rd_len);
			*wp += rrp->rd_len;

			/* update header */
			us = ntohs (*cnt) + 1;
			/* and caller's counter */
			*cnt = htons (us);

			if (T.debug > 4)
				syslog (LOG_DEBUG, "%s: now counter = %d",
					fn, us);
		}
	}

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: return %d", fn, *wp - wp_start);

	return (*wp - wp_start);
}

#define MESG_ASSEMBLE_OFFSET_LEN 64

int mesg_assemble (G_List *an_list, G_List *ns_list, G_List *ar_list,
		   u_char *buf, uint16_t buflen, u_char *mesg, int mesg_len) {
	uint16_t dnames[MESG_ASSEMBLE_OFFSET_LEN];
	u_char *ucp, *ucp_tmp;
	int written_len, ret;
	Mesg_Hdr *hdr;

	/* check if header is already present */
	if (mesg)
		memcpy (buf, mesg, mesg_len);
	else
		memset (buf, 0, buflen);

	hdr = (Mesg_Hdr *) buf;

	written_len = 0;

	/* check and reset header */
	hdr->qr = 1;
	hdr->ancnt = 0;
	hdr->nscnt = 0;
	hdr->arcnt = 0;

	if (hdr->qdcnt) {
		int qdcnt = ntohs(hdr->qdcnt);

		/*
		 * Register question name to compression name list
		 * Note that for simplicity we only register
		 * the first, and ignore multiple question
		 */
		dnames[0] = (uint16_t) (sizeof (Mesg_Hdr));
		dnames[1] = 0;	/* terminator */

		/*
		 * Skip the question section.
		 */
		ucp = buf + sizeof(Mesg_Hdr);
		while (qdcnt--) {
			/* skip QNAME */
			if (!(ucp = mesg_skip_dname(ucp, buf + mesg_len)) ||
			    ucp + 2 * sizeof(uint16_t) > buf + mesg_len) {
				syslog (LOG_NOTICE, "query message overrun");
				return -1;
			}
			/* skip QTYPE and QCLASS */
			ucp += (2 * sizeof(uint16_t));
		}
		written_len = ucp - buf;
	} else {
		/* there mustnot be any question */
		written_len = sizeof (Mesg_Hdr);
		ucp = buf + written_len;
		dnames[0] = 0;
	}

	/* write answers */
	ucp_tmp = ucp;
	ret = mesg_write_rrset_list (an_list, buf, buf + buflen, dnames,
				     MESG_ASSEMBLE_OFFSET_LEN, &ucp,
				     &(hdr->ancnt));
	if (ret < 0) {
		/* truncated message */
		hdr->tc = 1;
		return ucp_tmp - buf;
	}
	written_len += ret;

	/* write ns */
	ucp_tmp = ucp;
	ret = mesg_write_rrset_list (ns_list, buf, buf + buflen, dnames,
				     MESG_ASSEMBLE_OFFSET_LEN, &ucp,
				     &(hdr->nscnt));
	if (ret < 0) {
		/* truncated message */
		hdr->tc = 1;
		return ucp_tmp - buf;
	}
	written_len += ret;

	/* write additonal records */
	ucp_tmp = ucp;
	ret = mesg_write_rrset_list (ar_list, buf, buf + buflen, dnames,
				     MESG_ASSEMBLE_OFFSET_LEN, &ucp,
				     &(hdr->arcnt));
	/*
	 * ignore if error -- we may leave out additional
	 * records if we do not have room for them
	 */
	if (ret < 0)
		return ucp_tmp - buf;
	else
		return written_len + ret;
}

int mesg_extract_rr (u_char *mesg, u_char *msg_end, uint16_t r_type,
	uint16_t r_class, u_char *rrp, u_char *buf, int buflen) {
	int i, written_len;
	u_char *rp, *wp;

	written_len = 0;

	switch (r_type) {
	case RT_NS:
	case RT_CNAME:
	case RT_PTR:
		/* just extract domain name */
		if (!dname_decompress (buf, buflen, rrp, mesg, msg_end,
					    &written_len)) {
			syslog (LOG_INFO, "record invalid -- %s",
				string_rtype (r_type));
			return -1;
		}
		break;
	case RT_SOA:
		rp = rrp;
		wp = buf;
		rp = dname_decompress (wp, buflen, rp, mesg, msg_end, &i);
		if (!rp) {
			syslog (LOG_INFO, "record invalid -- SOA MNAME");
			return -1;
		}

		wp += i;
		rp = dname_decompress (wp, buflen - (wp - buf), rp,
					    mesg, msg_end, &i);
		if (!rp) {
			syslog (LOG_INFO, "record invalid -- SOA RNAME");
			return -1;
		}

		wp += i;
		memcpy (wp, rp, (i = sizeof (uint32_t) * 5));
		wp += i;
		written_len = wp - buf;
		break;
	case RT_MX:
		rp = rrp;
		wp = buf;
		memcpy (wp, rp, (i = sizeof (uint16_t) * 1));	/* PREFERENCE */
		wp += i;
		rp += i;
		if (!dname_decompress (wp, buflen - (wp - buf), rp,
					    mesg, msg_end, &i)) {
			syslog (LOG_INFO, "record invalid -- MX EXCHANGE");
			return -1;
		}
		wp += i;
		written_len = wp - buf;
		break;
	case RT_RP:
		/* two domain names */
		rp = rrp;
		wp = buf;
		rp = dname_decompress (wp, buflen, rp, mesg, msg_end, &i);
		if (!rp) {
			syslog (LOG_INFO, "record invalid -- RP MBOX-DNAME");
			return -1;
		}
		wp += i;
		rp = dname_decompress (wp, buflen - (wp - buf), rp,
					    mesg, msg_end, &i);
		if (!rp) {
			syslog (LOG_INFO, "record invalid -- RP TXT-DNAME");
			return -1;
		}
		wp += i;
		written_len = wp - buf;
		break;
	case RT_A:
	case RT_HINFO:
	case RT_AAAA:
	case RT_A6:
	case RT_SRV:
	case RT_TXT:
		/* no modification */
		return 0;
	default:
		syslog (LOG_INFO, "unknown resource type %d", r_type);
		return 0;
	}

	return written_len;
}

int mesg_parse (u_char *msg, int msg_len, G_List *an_list, G_List *ns_list,
		G_List *ar_list) {
	char *fn = "mesg_parse()";
	Mesg_Hdr *hdr;
	u_char *rp;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: start", fn);

	if (msg_len < sizeof (*hdr))
		return -1;

	hdr = (Mesg_Hdr *) msg;
	rp = (u_char *) (hdr + 1);	/* read next of the header */

	if (hdr->qdcnt) {
		/* skip question section */
		rp = mesg_skip_dname (rp, msg + msg_len);
		rp += 4;	/* sizeof qtype + qclass */
		if (rp > msg + msg_len)
			return -1;
	}

	rp = mesg_read_sec (an_list, rp, ntohs(hdr->ancnt), msg, msg_len);
	if (!rp)
		return -1;

	rp = mesg_read_sec (ns_list, rp, ntohs(hdr->nscnt), msg, msg_len);
	if (!rp)
		return -1;

	rp = mesg_read_sec (ar_list, rp, ntohs(hdr->arcnt), msg, msg_len);
	if (!rp)
		return -1;

	return 0;
}

u_char *dname_decompress (u_char *buf, int buflen, u_char *dname,
			  u_char *m_head, u_char *m_tail, int *written) {
	int token_len, written_len, iter;
	u_char *cp, *next;
	int pktsiz = m_tail - m_head;

	next = NULL;
	written_len = token_len = 0;
	for (cp = dname; *cp; cp += token_len) {
		iter = 0;
	  top:
		if ((*cp & DNCMP_MASK) == DNCMP_MASK) {
			uint16_t ui;

			if (iter++ >= pktsiz) /* we're probably in a loop. */
				return NULL;

			if (!m_head || !m_tail) /* irregular redirect */
				return NULL;

			/* redirect */
			next = cp + 2;
			GETSHORT (ui, cp);
			ui = ui & ~DNCMP_MASK_INT16T;

			cp = m_head + ui;
			if (cp < m_head || m_tail < cp)
				return NULL;

			goto top;
		}

		token_len = labellen(cp);
		if (token_len < 0)
			return NULL;
		else
			token_len++;

		if (T.debug > 4)
			syslog (LOG_DEBUG, "token_len: %d", token_len);

		if (written_len + token_len >= buflen)
			return NULL; /* buffer overrun */
		if (cp + token_len > m_tail)
			return NULL; /* out of bounds */

		if (written) {
			/* non-printable dname string */
			memcpy (buf, cp, token_len);
			written_len += token_len;
			buf += token_len;
		} else {
			/* write printable string */
			if ((*cp & DNCMP_MASK) != EDNS0_MASK) {
				memcpy (buf, cp + 1, token_len - 1);
				*(buf + (token_len - 1)) = DNAME_DELIM;
				written_len += token_len;
				buf += token_len;
			} else if (*cp == EDNS0_ELT_BITLABEL) {
				int bitlength, i;
				u_char *wp;

				/* a bit conservative test, but simple */
				if (written_len + token_len*2 + 7 >= buflen)
					return NULL; /* buffer overrun */

				wp = buf;
				wp += sprintf(wp, "\\[x");
				for (i = 1; i < token_len-1; i++) {
					u_char d1, d2;
					uint b;

					b = (int) *(cp + 1 + i);
					d1 = hex[(b >> 4) & 0x0f];
					d2 = hex[b & 0x0f];
					wp += sprintf(wp, "%c%c", d1, d2);
				}
				bitlength = *(cp + 1) ? *(cp + 1) : 256;
				wp += sprintf(wp, "/%u].", bitlength);

				written_len += (wp - buf);
				buf += written_len;
			}
		}
	}

	*buf = '\0';
	if (written)
		*written = written_len + 1;

	if (!next)
		next = cp+1;

	return next;
}


char *string_rclass (uint16_t rclass) {
	switch (rclass) {
		case C_IN:
		return "IN";
	case C_NONE:
		return "NONE";
	case C_ANY:
		return "ANY";
	default:
		syslog (LOG_NOTICE, "Unknown resource class(%d)", rclass);
		return "UNKNOWN";
	}
}

char *string_rtype (uint16_t rtype) {
	switch (rtype) {
		case RT_VOID:
		return "(void)";
	case RT_A:
		return "A";
	case RT_NS:
		return "NS";
	case RT_MD:
		return "MD";
	case RT_MF:
		return "MF";
	case RT_CNAME:
		return "CNAME";
	case RT_SOA:
		return "SOA";
	case RT_MB:
		return "MB";
	case RT_MG:
		return "MG";
	case RT_MR:
		return "MR";
	case RT_NULL:
		return "NULL";
	case RT_WKS:
		return "WKS";
	case RT_PTR:
		return "PTR";
	case RT_HINFO:
		return "HINFO";
	case RT_MINFO:
		return "MINFO";
	case RT_MX:
		return "MX";
	case RT_TXT:
		return "TXT";
	case RT_RP:
		return "RP";
	case RT_AAAA:
		return "AAAA";
	case RT_SRV:
		return "SRV";
	case RT_A6:
		return "A6";
	case RT_UINFO:
		return "UINFO";
	case RT_TSIG:
		return "TSIG";
	case RT_IXFR:
		return "IXFR";
	case RT_AXFR:
		return "AXFR";
	case RT_ALL:
		return "ANY";
	default:
		syslog (LOG_NOTICE, "Unknown resource type(%d)", rtype);
		return "UNKNOWN";
	}
}

static int dname_copy (u_char *from, u_char *to, int tolen) {
	int skip, written_len;

	written_len = 0;
	while (*from) {
		skip = labellen(from) + 1;
		written_len += skip;
		if (written_len >= tolen)
			return -1;

		memcpy (to, from, skip);
		from += skip;
		to += skip;
	}
	*to = '\0';
	written_len++;

	return written_len;
}

static u_char *dname_redirect (u_char *label, u_char *msg) {
	uint16_t us;

	if (msg && (*label & DNCMP_MASK) == DNCMP_MASK) {
		GETSHORT (us, label);
		us = us & (~DNCMP_MASK_INT16T);
		label = msg + us;
	}
	return label;
}

static int write_dname (u_char *msg, u_char *msg_tail, uint16_t *dnames,
		      int dnames_len, u_char *dname, u_char *wp) {
	char *fn = "write_dname()";
	u_char *bestmatch_rpd = NULL;
	u_char *bestmatch_rpm = NULL;
	int bestmatch_len;
	u_char *rpd, *rpm;
	int written_len;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * Check if some (part of) dname has already appeared in message
	 * 
	 * Start with full name, and chop first name label off for each
	 * iteration.
	 */
	bestmatch_len = 0;
	for (rpd = dname; *rpd && !bestmatch_len; rpd += labellen(rpd) + 1) {
		int i;

		/*
		 * The dnames array contains a list of pointers to domainnames
		 * in the message that we will try to match our name against.
		 */
		for (i = 0; dnames[i] != 0 && i < dnames_len; i++) {
			/*
	 		 * Again, start with full name, and chop first name label
			 * off for each iteration. Meanwhile, we follow
			 * redirections.
			 */
			for (rpm = dname_redirect (msg + dnames[i], msg); *rpm;
			     rpm = dname_redirect (labellen(rpm)+rpm+1, msg)) {
				u_char *cpd, *cpm;
				int match_len;

				if (rpm < msg || msg_tail < rpm)
					return -1; /* out of bounds */

				/* comparison pointers */
				cpd = rpd;
				cpm = rpm;

				match_len = 0;
				while (*cpm && *cpm == *cpd) {
					int mlen;

					mlen = labellen(cpm);
					if (mlen != labellen(cpd))
						break;

					/* binary comparison */
					if (*cpm == EDNS0_ELT_BITLABEL &&
					    memcmp (cpm+1, cpd+1, mlen))
							break;

					/* case-insensitive comparison */
					if (*cpm != EDNS0_ELT_BITLABEL &&
					    strncasecmp (cpm+1, cpd+1, *cpm))
						break;
					
					/* a label matched, move to next one */
					cpm += mlen + 1;
					cpd += mlen + 1;

					/* check redirection */
					cpm = dname_redirect (cpm, msg);
					if (cpm < msg || msg_tail < cpm)
						return -1; /* out of bounds */

					match_len++;
				}

				/*
				 * matched parts have to be postfixes, i.e.
				 * both need to be NULL terminated. We record
				 * the match if it is better than the best one
				 * we found so far.
				 */
				if (*cpm == '\0' && *cpd == '\0' &&
				    match_len > bestmatch_len) {
					bestmatch_rpd = rpd;
					bestmatch_rpm = rpm;
					bestmatch_len = match_len;
				}
			}
		}
	}

	/* register this name if not complete match */
	if (bestmatch_rpd != dname) {
		int i;
		/* we didn't find ourselves */
		for (i = 0; dnames[i] != 0; i++);

		if (i + 1 < dnames_len) {
			/* we still have room for one more in the table */
			if (((uint16_t) (bestmatch_rpm - msg)
			    < DNCMP_REDIRECT_LIMIT)) {
				/* It is within range */

				dnames[i] = (uint16_t) (wp - msg);
				dnames[i + 1] = 0;
			}
		}
	}

	/* write dname */
	written_len = 0;
	rpd = dname;

	/* write first unique part */
	while (*rpd && rpd != bestmatch_rpd) {
		int i;

		i = labellen(rpd) + 1;
		if (wp + i > msg_tail)
			return -1; /* overflow! */

		memcpy (wp, rpd, i);
		written_len += i;
		rpd += i;
		wp += i;
	}

	/* write second redirected part, if any found, or terminate */
	if (rpd == bestmatch_rpd) {
		uint16_t us;

		/* write redirection pointer */
		if (wp + sizeof (uint16_t) > msg_tail)
			return -1; /* overflow */

		us = (uint16_t) (bestmatch_rpm - msg) | DNCMP_MASK_INT16T;
		PUTSHORT (us, wp);
		written_len += sizeof (uint16_t);
	} else {
		*wp = '\0';
		written_len++;
	}

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: return (written_len = %d)",
			fn, written_len);

	return written_len;
}

static u_char *mesg_read_sec (G_List *target_list, u_char *section, int count,
			      u_char *mesg, int mesg_len) {
	char *fn = "mesg_read_sec()";
	G_List *rc_list, *gl;
	u_char buf[MAX_PACKET];
	u_char *msg_end, *rp;
	int i;

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: start", fn);

	/* initialize */
	rc_list = list_init ();
	if (!rc_list)
		return NULL;

	rp = section;
	msg_end = mesg + mesg_len;
	for (i = 0; i < count; i++) {
		u_char *rname, *rp_ex, *rdp;
		uint16_t r_type, r_class;
		uint16_t rdlen, rdlen_ex;
		uint32_t r_ttl;
		RR_List *rrl;

		/* parse header */
		rname = rp;
		rp = mesg_skip_dname (rp, msg_end);
		if (!rp)
			goto error;

		if (rp + sizeof(uint16_t)*3 + sizeof(uint32_t) > msg_end)
			goto error;

		GETSHORT (r_type, rp);
		GETSHORT (r_class, rp);
		GETLONG (r_ttl, rp);
		GETSHORT (rdlen, rp);

		rdp = rp;
		rp += rdlen;
		if (rp > msg_end)
			goto error;

		/* seek for matching RRset_Couple */
		for (gl = rc_list->next; gl->list_data; gl = gl->next) {
			RRset_Couple *rc;

			rc = (RRset_Couple *) (gl->list_data);
			if ((rc->rrs->key.info->r_type == r_type) &&
			    (rc->rrs->key.info->r_class == r_class) &&
			    !mesg_dname_cmp (mesg, rname, rrset_owner (rc->rrs))) {
				if (T.debug > 4)
					syslog (LOG_DEBUG, "%s: matching record \
found rrs->dname = %s / rname = %s", fn, rrset_owner (rc->rrs), rname);
				break;
			}
		}

		/* if no match, create a new RRset_Couple */
		if (!gl->list_data) {
			RRset_Couple *rc;
			int dname_len;

			if (!dname_decompress (buf, sizeof (buf), rname, mesg,
					       msg_end, &dname_len))
				goto error;

			rc = malloc (sizeof (RRset_Couple));
			if (!rc)
				goto error;

			rc->rrl = rr_list_alloc ();
			rc->rrs = rrset_create (r_type, r_class, dname_len, buf, NULL);

			/* if ok, add it to RRset_Couple list */
			if (!rc->rrl || !rc->rrs || list_add (rc_list, rc)) {
				rrset_couple_free(rc);
				goto error;
			}

			/* point it to the one we just added */
			gl = rc_list->next;
		}

		/* extract resource record */
		if (rdlen) {
			int ret;

			ret = mesg_extract_rr (mesg, msg_end, r_type, r_class,
					       rdp, buf, sizeof (buf));
			if (ret < 0)
				goto error;

			if (!ret) {
				rp_ex = rdp;
				rdlen_ex = rdlen;
			} else {
				rp_ex = buf;
				rdlen_ex = ret;
			}
		} else {
			rp_ex = NULL;
			rdlen_ex = 0;
		}

		/* add the extracted RR to matching RR_List */
		rrl = rr_list_add (((RRset_Couple *) (gl->list_data))->rrl,
				   r_ttl, rdlen_ex, rp_ex);
		if (!rrl)
			goto error;

		((RRset_Couple *) (gl->list_data))->rrl = rrl;
	}

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: make each RRset from list.", fn);

	rc_list->list_data = NULL;
	for (gl = rc_list->next; gl->list_data; gl = gl->next) {
		RRset_Couple *rc;
		RRset *rrs;

		/* create complete RRset */
		rc = (RRset_Couple *) (gl->list_data);
		rrs = rrset_create (rc->rrs->key.info->r_type,
				    rc->rrs->key.info->r_class,
				    rc->rrs->key.info->owner_len,
				    rrset_owner (rc->rrs), rc->rrl);
		if (!rrs)
			goto error;

		if (target_list) {
			if (list_add (target_list, rrset_copy (rrs)) < 0) {
				rrset_free (rrs);
				goto error;
			}
		}

		rrset_free (rrs);
	}

	/* free unused resources */
	list_destroy (rc_list, rrset_couple_freev);

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: end", fn);

	return rp;  /* byte after of the section we just processed */

error:
	syslog (LOG_INFO, "%s: message extraction failed", fn);
	list_destroy (rc_list, rrset_couple_freev);
	return NULL;
}

