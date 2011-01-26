/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: res_record.c,v 3.33 2002/03/06 14:56:02 dillema Exp $>
 */

#include "totd.h"

RR *rr_alloc (uint32_t ttl, int rd_len, u_char *rdata) {
	RR *ret;

	if (T.debug > 5)
		syslog (LOG_DEBUG, "rr_alloc(): start ttl = %d, rd_len = %d", ttl, rd_len);

	ret = (RR *) malloc (rd_len + RR_HEAD_LEN);
	if (!ret)
		return NULL;

	ret->ttl = ttl;
	ret->rd_len = rd_len;

	if (rdata) 
		memcpy (rr_rdata (ret), rdata, ret->rd_len);

	return ret;
}

RR_List *rr_list_alloc (void) {
	RR_List *ret;

	if (T.debug > 5)
		syslog (LOG_DEBUG, "start: rr_list_alloc(void)");

	ret = (RR_List *) malloc (sizeof (RR_List));
	if (!ret)
		return NULL;

	ret->next = NULL;
	ret->rrp = NULL;
	ret->cnt = -1;

	return ret;
}

void rr_list_free (RR_List *rrl) {
        char *fn = "rr_list_free()";
	RR_List *rrl_tmp;

	if (!rrl) {
		if (T.debug > 5)
			syslog (LOG_DEBUG, "%s start: NULL: immediate return", fn);
		return;
	}

	if (T.debug > 5)
		syslog (LOG_DEBUG, "%s start: (%p)", fn, rrl);

	for ( /* void */ ; rrl->next; rrl = rrl_tmp) {
		if (rrl->rrp) {
			if (T.debug > 5)
				syslog (LOG_DEBUG, "%s: free %p", fn, rrl->rrp);
			free (rrl->rrp);
		}
		rrl_tmp = rrl->next;
		free (rrl);
	}
	free (rrl);		/* last watchdog */

	return;
}

RR_List *rr_list_add (RR_List *rrl, uint32_t ttl, int rd_len, u_char *rdata) {
	RR_List *rrl_tmp;

	if (T.debug > 5)
		syslog (LOG_DEBUG, "start: rr_list_add(....,ttl=%d,rd_len=%d...)", ttl, rd_len);

	rrl_tmp = (RR_List *) malloc (sizeof (RR_List));
	if (!rrl_tmp)
		return NULL;

	rrl_tmp->rrp = rr_alloc (ttl, rd_len, rdata);
	if (!rrl_tmp->rrp) {
		free (rrl_tmp);
		return NULL;
	}
	rrl_tmp->next = rrl;
	rrl_tmp->cnt = rrl->cnt + 1;

	return rrl_tmp;
}

RR *rrset_get_rr_f (int n, RRset *rrset) {
	uint16_t offset;

	offset = *(uint16_t *) ((u_char *) rrset->data.p + DATADATA_HEAD_LEN +
				 +sizeof (uint16_t) * n);
	return (RR *) (((u_char *) rrset->data.p) + offset);
}

RR_List *rr_list_of_rrset (RRset *rrsp) {
	RR_List *rrl, *rrl_tmp;
	RR *rrp;
	int i;

	rrl = rr_list_alloc();
	if (!rrl) 
		return NULL;

	if (!rrsp->data.p) 
		return rrl;	/* no record -- empty list */

	for (i = 0; i < rrsp->data.d->data_cnt; i++) {
		rrp = (RR *) (rrsp->data.p + data_offset (i, rrsp->data.d));
		rrl_tmp = rr_list_add (rrl, rrp->ttl, rrp->rd_len, rr_rdata (rrp));
		if (!rrl_tmp) {
			rr_list_free (rrl);
			return NULL;
		}
		rrl = rrl_tmp;
	}

	return rrl;
}

RRset *rrset_alloc (void) {
	RRset *ret;

	ret = (RRset *) malloc (sizeof (RRset));
	if (!ret)
		return NULL;

	ret->key_len = 0;
	ret->key.p = NULL;

	ret->data_len = 0;
	ret->data.p = NULL;

	ret->links = 1;

	if (T.debug > 5)
		syslog (LOG_DEBUG, "rrset_alloc(): allocated %p", ret);

	return ret;
}

RRset *rrset_create (uint16_t r_type, uint16_t r_class, uint16_t owner_len,
		     u_char *owner, RR_List *rrl) {
	RR_List *rrl_tmp;
	int len;
	RRset *rrs;

	if (T.debug > 5)
		syslog (LOG_DEBUG, "start: rrset_create(%s,%s,....)",
			    string_rtype (r_type), string_rclass (r_class));

	rrs = rrset_alloc();
	if (!rrs)
		return NULL;

	/* make key */
	len = owner_len + KEYINFO_HEAD_LEN;
	rrs->key.p = malloc (len);
	if (!rrs->key.p) {
		rrset_free (rrs);
		return NULL;
	}

	rrs->key_len = len;
	rrs->key.info->r_type = r_type;
	rrs->key.info->r_class = r_class;
	rrs->key.info->owner_len = owner_len;
	memcpy (rrset_owner (rrs), owner, owner_len);

	/* return, if no data */
	if (!rrl)
		return rrs;

	/* make data */
	len = ((rrl->cnt + 1) * sizeof(uint16_t) + DATADATA_HEAD_LEN);
	len += (len % 4 == 0) ? 0 : (4 - len % 4); /* padding */
	for (rrl_tmp = rrl; rrl_tmp->next; rrl_tmp = rrl_tmp->next) {
		rrl_tmp->offset = len;	/* store current data offset */

		len += RR_HEAD_LEN + rrl_tmp->rrp->rd_len;
		len += (len % 4 == 0) ? 0 : (4 - len % 4);
	}

	if (T.debug > 5)
		syslog (LOG_DEBUG, "rrset_create: final length %d", len);

	rrs->data.p = malloc (len);
	if (!rrs->data.p) {
		rrset_free (rrs);
		return NULL;
	}
	rrs->data_len = len;

	/* write RRs */
	rrs->data.d->data_cnt = rrl->cnt + 1;	/* because zero origin */
	for (rrl_tmp = rrl; rrl_tmp->next; rrl_tmp = rrl_tmp->next) {
		data_offset (rrl_tmp->cnt, rrs->data.p) = rrl_tmp->offset;
		memcpy (rrs->data.p + rrl_tmp->offset, (u_char *) rrl_tmp->rrp, rrl_tmp->rrp->rd_len + RR_HEAD_LEN);
	}

	return rrs;
}

RRset *rrset_create_single (u_char *dname, int dname_len, uint16_t rtype,
		            uint16_t rclass, uint32_t ttl, uint16_t rd_len,
		            u_char *rd) {
	RR_List *rrlp;
	RR_List *rrlp_tmp;
	RRset *rrsp;

	rrlp = rr_list_alloc ();
	if (!rrlp)
		return NULL;

	rrlp_tmp = rr_list_add (rrlp, ttl, rd_len, rd);
	if (!rrlp_tmp) {
		rr_list_free (rrlp);
		return NULL;
	}
	rrlp = rrlp_tmp;
	rrsp = rrset_create (rtype, rclass, dname_len, dname, rrlp);
	if (!rrsp) {
		rr_list_free (rrlp);
		return NULL;
	}
	rr_list_free (rrlp);

	return rrsp;
}

void rrset_freev (void *p) {
	rrset_free ((RRset *) p);
	return;
}

void rrset_free (RRset *rrset) {

	if (rrset->links <= 1) {
		if (T.debug > 5)
			syslog (LOG_DEBUG, "rrset_free(%p): link %d -> destroyed",
				rrset, rrset->links);

		if (rrset->key.p)
			free (rrset->key.p);

		if (rrset->data.p)
			free (rrset->data.p);

		free (rrset);
	} else {
		if (T.debug > 5)
			syslog (LOG_DEBUG, "rrset_free(%p): link %d -> %d",
				rrset, rrset->links, rrset->links - 1);
		rrset->links--;
	}
}

RRset *rrset_copy (RRset *rrset) {
	if (T.debug > 5)
		syslog (LOG_DEBUG, "rrset_copy(%p): link %d -> %d", rrset,
			    rrset->links, rrset->links + 1);

	rrset->links++;
	return rrset;
}

void *rrset_copyv (void *rrsetv) {
	return (void *) rrset_copy ((RRset *) rrsetv);
}

void *rrset_dupv (void *rrsetv) {
	return (void *) rrset_dup ((RRset *) rrsetv);
}

RRset *rrset_dup (RRset *rrset) {
	RRset *rrsp_new = NULL;

	rrsp_new = rrset_alloc ();
	if (!rrsp_new)
		return NULL;

	rrsp_new->key.p = NULL;
	rrsp_new->data.p = NULL;

	if (rrset->key.p) {
		rrsp_new->key.p = malloc (rrset->key_len);
		if (!rrsp_new->key.p) {
			free (rrsp_new);
			return NULL;
		}

		memcpy (rrsp_new->key.p, rrset->key.p, rrset->key_len);
		rrsp_new->key_len = rrset->key_len;
	}
	if (rrset->data.p) {
		rrsp_new->data.p = malloc (rrset->data_len);
		if (!rrsp_new->data.p) {
			if (rrsp_new->key.p)
				free (rrsp_new->key.p);
			free (rrsp_new);
			return NULL;
		}

		memcpy (rrsp_new->data.p, rrset->data.p, rrset->data_len);
		rrsp_new->data_len = rrset->data_len;
	}
	return rrsp_new;
}

void rrset_couple_free (RRset_Couple * rc) {

	if (rc->rrs)
		rrset_free (rc->rrs);
	if (rc->rrl)
		rr_list_free (rc->rrl);

	free (rc);
}

void rrset_couple_freev (void *rcv) {
	rrset_couple_free ((RRset_Couple *) rcv);
}

