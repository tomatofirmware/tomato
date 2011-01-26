/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/


/*
 * <$Id: udp_request.c,v 3.53 2002/03/04 12:34:11 dillema Exp $>
 */

#include "totd.h"

/*
 * A request message is sent over the given socket, and a
 * timeout event registered:
 *	- if no response within timeout, retry routine is called. 
 * 	- if response is received, it replaces the request message in the
 *	  (child) context and the process routine is called.
 */

int udp_request_start (struct context *cont) {
	char *fn = "udp_request_start()";
	struct sockaddr *sa;
	int timeout;

	syslog (LOG_DEBUG, "%s: start", fn);

        cont->process = udp_request_process;
        cont->retry = udp_request_retry;

	if (cont->mesg_len > MAX_PACKET) {
		syslog (LOG_NOTICE, "Query to big for UDP datagram.");
		return (request_abort (cont, 1)); /* try TCP instead? */
	}

	sa = (struct sockaddr *) cont->current_ns->list_data;
	if (net_mesg_send (NULL, cont->mesg.p, cont->mesg_len, sa) < 0) {
		syslog (LOG_NOTICE, "send failed: %m");

		/* force immediate retry */
		timeout = 0;
		syslog (LOG_DEBUG, "%s: force retry at zero timeout", fn);
	} else {
		/* put me to input list */
		if (ev_udp_in_register (cont, sa,
					SOCKADDR_SIZEOF (*sa), cont->q_id) < 0)
			return (request_abort (cont, -1));

		timeout = cont->timeout;
		syslog (LOG_DEBUG, "Query times out in %d seconds", timeout);
	}

	/* put me on timeout event queue */
	if (context_timeout_register (cont, timeout) < 0) 
		return (request_abort (cont, -1));

	syslog (LOG_DEBUG, "%s: end", fn);
	/* SUCCESS */
	return 0;
}

int udp_request_process (Context *cont) {
	char *fn = "udp_request_process()";
	struct sockaddr *sa;

	syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * ev_udp_in() called us to process an incoming response
	 * to the request we sent. The response message has been
	 * put in cont->mesg.p
	 */

	/*
	 * Timeout will still go off later, so we render
	 * it harmless here by removing the handler.
	 */
	if (cont->tout)
		cont->tout->handler = NULL;

	/* remove me from I/O list */
	sa = (struct sockaddr *) cont->current_ns->list_data;
	if (ev_udp_in_remove (sa, cont->q_id) < 0)
		return (request_abort (cont, -1));
	else
		return request_finish (cont);
}

int udp_request_retry (Context *cont) {
	char *fn = "udp_request_retry()";
	struct sockaddr *sa;
	int len, timeout;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* remove old I/O List (if it is still there) */
	sa = (struct sockaddr *) cont->current_ns->list_data;
	ev_udp_in_remove (sa, cont->q_id);

	/*
	 * Fast GiveUp Hack.
	 *
	 * decisions like this should only be made in request.c,
	 * so this hack is ugly too.
	 */
	if (cont->q_type == RT_AAAA || cont->q_type == RT_A6) {
		/*
		 * Some nameservers just do not like IPv6
		 * address records. We guess that is the case
		 * here and tell the parent to continue if it
		 * can.
		 *
		 * We perform the code of request_finish()
		 * here, but without the parsing of the
		 * response (we have none).
		 *
		 * Note that this means that a timeout for a IPv6
		 * address record request never causes a shift to
		 * the next forwarder! Seems reasonable in the
		 * for the majority of cases.
		 */

		syslog (LOG_DEBUG, "Giving up quickly on IPv6 address record");

		/* re-initialize answer, nameserver, additional record lists */
		list_destroy (cont->an_list, rrset_freev);
		list_destroy (cont->ns_list, rrset_freev);
		list_destroy (cont->ar_list, rrset_freev);
		cont->an_list = list_init ();
		cont->ns_list = list_init ();
		cont->ar_list = list_init ();
		if (!cont->an_list || !cont->ns_list || !cont->ar_list)
			return request_abort (cont, -1);

		if (cont->parent) {
			syslog (LOG_DEBUG, "%s: process parent context", fn);
			cont->parent->process (cont->parent);
		}

		/* ... so we cleanup ourselves in any case */
		context_destroy (cont);

		syslog (LOG_DEBUG, "%s: return success", fn);
		return 0; /* SUCCESS */
	}

	if (request_retry (cont) < 0)
		return (request_abort (cont, -2));

	/* send/forward the message/request again */
	sa = (struct sockaddr *) cont->current_ns->list_data;
	len = net_mesg_send (NULL, cont->mesg.p, cont->mesg_len, sa);
	if (len < cont->mesg_len) {
		if (len < 0)
			syslog (LOG_NOTICE, "retry failed(default socket): %m");
		else
			syslog (LOG_NOTICE, "can't send whole datagram");
		/* forcing immediate retry */
		timeout = 0;
	} else {
		/* we retried the request, now wait for response or timeout */
		timeout = cont->timeout;
		/* put me to input list */
		if (ev_udp_in_register (cont, sa,
		     SOCKADDR_SIZEOF (*sa), cont->q_id) < 0)
			return (request_abort (cont, -1));
	}

	/* put me to timeout list */
	if (context_timeout_register (cont, timeout) < 0)
		return (request_abort (cont, -1));

	/* no state change... */
	syslog (LOG_DEBUG, "%s: end", fn);
	return 0;
}
