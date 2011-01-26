/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: udp_response.c,v 3.57 2002/03/04 12:34:11 dillema Exp $>
 */

#include "totd.h"

/*
 * When a new query is received over a UDP socket from a client, the
 * udp_response state machine is started. See also ev_udp_in_read(),
 * which is the only place from where udp_response_start() is called.
 *
 * udp_response_start() starts a new transaction. First creates a new
 * root context for it, and starts the forwarding of the original query.
 *
 *	forwarding is performed by starting a new child request, which
 *	if all goes well result in some resource records in our context.
 *
 * when the forwarded request finished successfully, udp_response_finish()
 * is responsible for interpreting it and sending a response back to the client.
 *
 */
int udp_response_start (u_char *mesg_buf, int mesg_len, struct sockaddr *sa_p,
			Nia *inif) {
	const char *fn = "udp_response_start()";
	Context *cont;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* create context */
	cont = context_create();
	if (!cont)
		return (response_abort (cont, -1));

	cont->mesg.p = mesg_buf;
	cont->mesg_len = mesg_len;
	cont->wp = mesg_buf + mesg_len;	/* just after the answer section */
	cont->q_id = cont->mesg.hdr->id;
	cont->netifaddr = nia_copy (inif);
	memcpy (cont->peer, sa_p, SOCKADDR_SIZEOF(*sa_p));

	if (cont->mesg.hdr->opcode == OP_QUERY) {
		syslog (LOG_DEBUG, "%s: OPCODE = OP_QUERY", fn);

		/* query-response specific variables */
		cont->process = udp_response_recursive_process;
		cont->retry = udp_response_recursive_retry;

		/* do the forwarding, send request to forwarder */
		switch (request_start (cont, QUERY_TCP)) {
		case 0:
			/* We got a response */
			return (0);
		case 1:
			/* Something wrong with the query */
			cont->mesg.hdr->rcode = RC_FMTERR;
			return (udp_response_finish (cont));
		default:
			/* We failed ourselves somehow */
			cont->mesg.hdr->rcode = RC_SERVERERR;
			return (udp_response_finish (cont));
		}
	} else {
		syslog (LOG_NOTICE, "%s: OPCODE unknown(%d)", fn,
			cont->mesg.hdr->opcode);
		cont->mesg.hdr->rcode = RC_NIMP;
		return udp_response_finish (cont);
	}

	/* NOTREACHED */
	return 0;
}

int udp_response_recursive_process (Context *cont) {
	switch (recursive_process (cont)) {
	case 0:
		/* continue, not ready yet */
		return 0;
	case 1:
		/* finished with success */
		return udp_response_finish (cont);
	default:
		/* we failed ourselves */
		cont->mesg.hdr->rcode = RC_SERVERERR;
		return udp_response_finish (cont);
	}
}

int udp_response_recursive_retry (Context *cont) {
	syslog (LOG_ERR, "udp_response_recursive_retry should not be called.");
	return response_abort (cont, 0);
}

int udp_response_finish (Context *cont) {
        const char *fn = "udp_response_finish()";
	int len;

	syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * We trim the response down if it doesn't fit in a default UDP
	 * data in a somewhat crude way. Could be smarter, like trim
	 * A records from response for pure IPv6 queries and so. See, if
	 * it is needed often. Maybe better to support EDNS0 option instead.
	 *
	 * RFC1035 says we should truncate the message to 512 bytes and set the
	 * tr bit in the header, but it is really vague on when a response does
	 * not fit. RFC2181 clarifies the use of the tr bit in section 9.
	 *
	 * Note that similar logic is still present in mesg_assemble() in
	 * ne_mesg.c (may disappear if we add the logic to assemble_response()
	 * instead).
	 */
	assemble_response (cont);
	if (cont->mesg_len < 0 || cont->mesg_len > MAX_PACKET) {
		/* put the message on a diet */
		list_destroy (cont->ar_list, rrset_freev);
		cont->ar_list = NULL;
		syslog (LOG_DEBUG, "Overweight, dropping additional section");
		assemble_response (cont);
	}
	if (cont->mesg_len < 0 || cont->mesg_len > MAX_PACKET) {
		/*
		 * ok, water adn bread then then. shuold check
		 * RFCs, it may be better to return error msg
		 * instead of slimmed answer.
		 */
		list_destroy (cont->ns_list, rrset_freev);
		cont->ns_list = NULL;
		syslog (LOG_DEBUG, "Overweight, dropping authority section");
		assemble_response (cont);
	}
	if (cont->mesg_len < 0 || cont->mesg_len > MAX_PACKET) {
		/* ok, nothing viable left, so die */
		list_destroy (cont->an_list, rrset_freev);
		cont->an_list = NULL;
		assemble_response (cont);
		cont->mesg.hdr->tc = 1;
		syslog (LOG_WARNING, "Obese, answers too big for UDP");
	}
	if (cont->mesg_len < 0 || cont->mesg_len > MAX_PACKET) {
		syslog (LOG_ERR, "Even error msg is too big for UDP");
		return (response_abort (cont, 1));
	}

	/* send the answer */
	len = net_mesg_send (cont->netifaddr, cont->mesg.p, cont->mesg_len,
			     cont->peer);

	if (len < cont->mesg_len) {
		syslog (LOG_NOTICE, "failed to send message.");
		return (response_abort (cont, -1));
	}

        context_destroy (cont);
	return 0;
}
