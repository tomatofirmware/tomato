/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/
/*
 * <$Id: ev_udp_in.c,v 3.38 2002/03/06 14:56:02 dillema Exp $>
 */

#include "totd.h"

static G_List *UDPL_head;		/* UDP head */
static G_List *UDPL_tail;		/* tail */

int ev_udp_in_eq (void *arg1, void *arg2) {
	Ev_UDP_In_Data *e1;
	Ev_UDP_In_Data *e2;
	int len1, len2;

	e1 = (Ev_UDP_In_Data *) arg1;
	e2 = (Ev_UDP_In_Data *) arg2;

	if (!e1 || !e2 || !e1->sa_p || !e2->sa_p) 
		return -1;
	
	len1 = SOCKADDR_SIZEOF (*(e1->sa_p));
	len2 = SOCKADDR_SIZEOF (*(e2->sa_p));

	if (e1->id == e2->id && len1 == len2 &&
	    !memcmp (e1->sa_p, e2->sa_p, len1))
		return 0;	/* equality */
	else
		return 1;
}

void ev_udp_in_data_free (Ev_UDP_In_Data *euid_p) {

	if (euid_p) {
		if (euid_p->sa_p)
			free (euid_p->sa_p);
		free (euid_p);
	}
	return;
}

void ev_udp_in_data_free_v (void *euid_vp) {

	ev_udp_in_data_free ((Ev_UDP_In_Data *) euid_vp);
	return;
}

int ev_udp_in_init (void) {

	if (UDPL_head)
		return -1;

	UDPL_head = list_init ();
	if (!UDPL_head)
		return -1;

	UDPL_tail = UDPL_head;
	UDPL_head->list_data = NULL;

	return 0;
}

void ev_udp_in_finish (void) {
	net_delete_socketlist ();
	list_destroy (UDPL_head, ev_udp_in_data_free_v);
}

int ev_udp_in_register (Context *cptr, struct sockaddr *sa_p, int sa_len,
			uint16_t id) {
	const char *fn = "ev_udp_in_register()";
	Ev_UDP_In_Data *euid_new = NULL;

	if (T.debug > 3)
		syslog (LOG_DEBUG, "%s: id=%d", fn, id);

	euid_new = malloc (sizeof (Ev_UDP_In_Data));
	if (!euid_new)
		return -1;

	euid_new->sa_p = malloc (sa_len);
	if (!euid_new->sa_p) {
		free (euid_new);
		return -1;
	}

	memcpy (euid_new->sa_p, sa_p, sa_len);
	euid_new->cptr = cptr;
	euid_new->id = id;

	if (!list_add (UDPL_head, euid_new)) {
		if (T.debug > 3)
			syslog (LOG_DEBUG, "%s: %p", fn, euid_new);
		/* SUCCESS */
		return 0;
	} else {
		ev_udp_in_data_free (euid_new);
		return -1;
	}
}

int ev_udp_in_remove (struct sockaddr *sa_p, int id) {
	Ev_UDP_In_Data euid_tmp;
	Ev_UDP_In_Data *euid_p;
	G_List *gl_tmp;

	euid_tmp.sa_p = sa_p;
	euid_tmp.id = id;

	gl_tmp = list_search (UDPL_head, (void *) &euid_tmp, ev_udp_in_eq);
	if (!gl_tmp)
		return -1;

	euid_p = (Ev_UDP_In_Data *) list_delete (gl_tmp);
	ev_udp_in_data_free (euid_p);

	return 0;
}

/*
 * Receive a UDP message; either a request or response.
 *	==> a request message starts a new UDP transaction with root context.
 *	==> a response message overwrites a request message in the
 *	    existing context of the matching transaction whose `process'
 *	    routine is called.
 */
int ev_udp_in_read (int sock) {
	const char *fn = "ev_udp_in_read()";
	struct sockaddr_storage sa;
	u_char *newbuf;
	Mesg_Hdr *mesg;
	int mesg_len;

	unsigned int sa_len;
	G_List *gl_tmp;
	Ev_UDP_In_Data euid;
	Nia *ni_l;

	/* allocate buffer */
	newbuf = malloc (MAX_PACKET);
	if (!newbuf)
		return (-1);
	mesg = (Mesg_Hdr *) newbuf;

	/* read packet */
	sa_len = sizeof(sa);
	memset ((void *) &sa, 0, sa_len);
	mesg_len = recvfrom (sock, newbuf, MAX_PACKET, 0,
			     (struct sockaddr *) &sa, &sa_len);
	if (mesg_len < 0) {
		syslog (LOG_ERR, "recvfrom: %m");
		if (newbuf)
			free (newbuf);
		return (-1);
	}

	if (T.debug > 3)
		syslog (LOG_DEBUG, "%s: read from sockid %d, len %d.", fn,
			sock, mesg_len);

	if (mesg_len < sizeof (Mesg_Hdr)) {
		syslog(LOG_NOTICE, "ignoring too short message");
		if (newbuf)
			free (newbuf);
		return (-1);
	}

	/*
	 * Check for duplicate message id. If we already have a transaction
	 * going with the same id, we have to ignore it to avoid confusion.
	 * Note that this may be just a retransmission of a client, i.e.
	 * nothing to worry about.
	 */
	if (ev_dup ((struct sockaddr *) &sa, sa_len, mesg->id) < 0) {
		/* duplicate */
		if (T.debug > 3)
			syslog (LOG_DEBUG, "%s: duplicate request ignored", fn);
		if (newbuf)
			free (newbuf);
		return (0);
	}

	/* identify and match */
	euid.id = mesg->id;
	euid.sa_p = (struct sockaddr *) &sa;
	gl_tmp = list_search (UDPL_head, (void *) &euid, ev_udp_in_eq);

	if (!gl_tmp) {
		if (mesg->qr == 1) {
			if (T.debug > 3)
				syslog (LOG_DEBUG, "%s: unknown UDP response \
ignored", fn);
			if (newbuf)
				free (newbuf);
			return (0);
		}
		/* new request, start new transaction */
		if (T.debug > 3)
			syslog (LOG_DEBUG, "%s: Create new transaction", fn);

		ni_l = nia_find_by_sock (sock);
		if (!ni_l) {
			syslog (LOG_ERR, "no socket for interface");
			if (newbuf)
				free (newbuf);
			return (-1);
		}
		return udp_response_start (newbuf, mesg_len,
				(struct sockaddr *) &sa, ni_l);
	} else {
		Context *cont;

		if (T.debug > 3)
			syslog (LOG_DEBUG, "%s: resume transaction", fn);

		cont = ((Ev_UDP_In_Data *) gl_tmp->list_data)->cptr;

		/* delete old message buffer (with request message) */
		if (cont->mesg.p)
			free (cont->mesg.p);

		/* ... and replace it with the new response message */
		cont->mesg.p = newbuf;
		cont->mesg_len = mesg_len;
		/* 
		 * `wp'' has no meaning for UDP transactions,
		 * clear it in case we switch to TCP later
		 */
		cont->wp = NULL;

		/*
		 * process context; context must release the timeout event
		 */
		return (cont->process) (cont);
	}

	/* NOTREACHED */
	return -1;
}

