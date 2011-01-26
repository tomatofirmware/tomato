/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/


/*
 * <$Id: context.c,v 3.32 2002/03/04 12:34:10 dillema Exp $>
 */

#include "totd.h"

int context_timeout_handler (Ev_TO_Data *td) {

	if (td->type != EV_TIMEOUT_CONTEXT)
		return -1;

	td->data.cont->tout = NULL;
	/* call and return from retry routine */
	return td->data.cont->retry (td->data.cont);
}

int context_timeout_register (Context *cont, int timeout) {
	const char *fn = "context_timeout_register()";
	Ev_TO_Data *edtp;

	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: start", fn);

	if (cont->tout && cont->tout->handler) {
		syslog (LOG_INFO, "%s: duplicate timeout. ignoring", fn);
		return -1;
	}

	edtp = malloc (sizeof (Ev_TO_Data));
	if (!edtp)
		return -1;

	edtp->at = time (NULL) + timeout;
	edtp->handler = context_timeout_handler;
	edtp->type = EV_TIMEOUT_CONTEXT;
	edtp->data.cont = cont;
	if (ev_to_register (edtp) < 0) {
		free (edtp);
		return (-1);
	}

	cont->tout = edtp;
	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: scheduled in %d", fn, (int) edtp->at);

	/* SUCCESS */
	return 0;
}

Context *context_create (void) {
	const char *fn = "context_create()";
	Context *cont;

	cont = malloc (sizeof (Context));
	if (!cont)
		return NULL;

	syslog (LOG_DEBUG, "%s: %p", fn, (void *) cont);

	memset (cont, 0, sizeof (Context));
	cont->current_ns = NULL;
	cont->conn_sock = -1;

	cont->an_list = list_init ();
	cont->ns_list = list_init ();
	cont->ar_list = list_init ();

	if (!cont->an_list || !cont->ns_list || !cont->ar_list)
		return context_destroy(cont);

	cont->peer = malloc (sizeof (struct sockaddr_storage));
	if (!cont->peer)
		return context_destroy(cont);

	memset (cont->peer, 0, sizeof (struct sockaddr_storage));
	return cont;
}

void *context_destroy (Context *cont) {
	const char *fn = "context_destroy()";

	syslog (LOG_DEBUG, "%s: %p", fn, (void *) cont);

	if (!cont)
		return NULL;

	/* anybody referencing us? */
	if (cont->parent)
		cont->parent->child = NULL;
	if (cont->child)
		cont->child->parent = NULL;

	if (cont->an_list)
		list_destroy (cont->an_list, rrset_freev);
	if (cont->ns_list)
		list_destroy (cont->ns_list, rrset_freev);
	if (cont->ar_list)
		list_destroy (cont->ar_list, rrset_freev);

	if (cont->nameservers)
		list_destroy(cont->nameservers, free);

	if (cont->mesg.p)
		free (cont->mesg.p);

	if (cont->tout)
		cont->tout->handler = NULL;

	/* in case of TCP */
	if (cont->conn_sock >= 0) {
		/* first close the connection */
		shutdown (cont->conn_sock, SHUT_RDWR);
		close (cont->conn_sock);

		/* then cleanup events and free memory */
		ev_tcp_out_remove (cont->conn_sock);
		ev_tcp_conn_in_remove (cont->conn_sock);
        }

	/* in case of UDP */
	if (cont->conn_sock < 0 && cont->peer && cont->peer->sa_family)
		ev_udp_in_remove (cont->peer, cont->q_id);
	if (cont->peer)
		free (cont->peer);

	/* in case of UDP response */
	if (cont->netifaddr)
		nia_free (cont->netifaddr, 0);

	free (cont);
	return NULL;
}
