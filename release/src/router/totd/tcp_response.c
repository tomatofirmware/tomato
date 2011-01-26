/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/


/*
 * <$Id: tcp_response.c,v 3.51 2005/01/27 15:29:33 dillema Exp $>
 */

#include "totd.h"

int tcp_response_start (int sock, struct sockaddr *sa_p) {
	const char *fn = "tcp_response_start()";
	Context *cont;

	syslog (LOG_DEBUG, "%s: start", fn);

	cont = context_create();
	if (!cont)
		return (response_abort (cont, -1));

	cont->process = tcp_response_readlen_process;
	cont->retry = tcp_response_readlen_retry;
	memcpy (cont->peer, sa_p, SOCKADDR_SIZEOF(*sa_p));

	cont->conn_sock = sock;
	cont->timeout = TCP_SRV_TIMEOUT;

	if (!context_timeout_register (cont, cont->timeout) &&
	    !ev_tcp_conn_in_register (cont->conn_sock, cont))
			return 0; /* SUCCESS */

	return (response_abort (cont, -1));
}

int tcp_response_readlen_process (Context *cont) {
	const char *fn = "tcp_response_readlen_process()";
	uint16_t length_buf;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (context_timeout_register (cont, cont->timeout))
		return (response_abort (cont, -1));

	/* still use old input list -- data following */

	/* read length buffer */
	if (read (cont->conn_sock, (u_char *) (&length_buf),
	    sizeof (uint16_t)) < sizeof (uint16_t)) {
		syslog (LOG_NOTICE, "%s: cannot read length TCP message", fn);
		return (response_abort (cont, -1));
	}

	length_buf = ntohs (length_buf);
	syslog (LOG_DEBUG, "%s: data length = %d", fn, length_buf);

	/* free old buffer */
	if (cont->mesg.p)
		free (cont->mesg.p);

	/* allocate new properly sized buffer */
	cont->mesg.p = malloc (length_buf);
	if (!cont->mesg.p)
		return (response_abort (cont, -1));

	cont->mesg_len = length_buf;
	cont->wp = cont->mesg.p;

	/* go next state */
	cont->process = tcp_response_reading_process;
	cont->retry = tcp_response_reading_retry;

	/* SUCCESS */
	return 0;
}

int tcp_response_readlen_retry (Context *cont) {
	const char *fn = "tcp_response_readlen_retry()";

	syslog (LOG_NOTICE, "%s: connection does not respond. closing.", fn);
	return (response_abort (cont, -1));
}

int tcp_response_reading_process (Context *cont) {
	const char *fn = "tcp_response_reading_process()";
	int len;

	syslog (LOG_DEBUG, "%s: start", fn);

	if (cont->tout)
		cont->tout->handler = NULL;

	if (context_timeout_register (cont, cont->timeout))
		return(response_abort (cont, -1));

	len = read (cont->conn_sock, cont->wp,
		    cont->mesg_len - (cont->wp - cont->mesg.p));

	if (len <= 0)
		return(response_abort (cont, -1));

	cont->wp += len;
	if (cont->wp < (cont->mesg.p + cont->mesg_len)) {
		syslog (LOG_DEBUG, "%s: left %d bytes -- continue.", fn,
			(cont->mesg.p + cont->mesg_len) - cont->wp);

		/* SUCCESS */
		return 0;	/* the processing continues ... */
	}

	/* all data has been read now */
	ev_tcp_conn_in_remove (cont->conn_sock);

	if (cont->mesg.hdr->opcode == OP_QUERY) {
		if (cont->tout)
			cont->tout->handler = NULL;

		/*
		 * don't register input list,
		 * child context takes input
		 */

		switch (request_start (cont, QUERY_TCP)) {
		case 0:
			/*
			 * We got a response
			 * goto recursive_processing state
			 * clear events, child will call me then
			 */
			if (cont->tout)
				cont->tout->handler = NULL;

			cont->process = tcp_response_recursive_process;
			cont->retry = tcp_response_recursive_retry;

			/* SUCCESS */
			return 0;
		case 1:
			/* Something wrong with the query */
			syslog (LOG_NOTICE, "%s: Request failed", fn);
			cont->mesg.hdr->rcode = RC_FMTERR;
		default:
			/* totd failed itself somehow */
			syslog (LOG_NOTICE, "%s: Totd failed", fn);
			cont->mesg.hdr->rcode = RC_SERVERERR;
		}
	} else
		cont->mesg.hdr->rcode = RC_NIMP;

	/* 
	 * Not OP_QUERY or Request failed brings us here
	 *
	 * Go to writing state. Timeout already done in this function
	 * Register new output event 
	 */

	if (!ev_tcp_out_register (cont->conn_sock, cont)) {
		assemble_response (cont);

		/* state change */
		cont->process = tcp_response_writing_process;
		cont->retry = tcp_response_writing_retry;
		cont->wp = NULL;

		syslog (LOG_DEBUG, "%s: end (writing:)", fn);
		/* SUCCESS */
		return 0;
	}

	/* FAILURE */
	return (response_abort (cont, -1));
}

int tcp_response_reading_retry (Context *cont) {
	return (response_abort (cont, -1));
}

int tcp_response_recursive_process (Context *cont) {
	const char *fn = "tcp_response_recursive_process()";

	syslog (LOG_DEBUG, "%s: start", fn);

	switch (recursive_process (cont)) {
	case 0:
		syslog (LOG_DEBUG, "%s: return, continue", fn);

		/* SUCCESS */
		return 0;
	case 1:
		assemble_response (cont);

		if (cont->mesg_len < 0 || cont->mesg_len > MAX_STREAM) {
			syslog (LOG_WARNING, "response message too big");
			return (response_abort (cont, 1));
		}

		if (cont->tout)
			cont->tout->handler = NULL;

		if (context_timeout_register (cont, cont->timeout))
			return (response_abort (cont, -1));

		/* register output */
		if (ev_tcp_out_register (cont->conn_sock, cont))
			return (response_abort (cont, -1));

		/* change the state */
		cont->process = tcp_response_writing_process;
		cont->retry = tcp_response_writing_retry;
		cont->wp = NULL;

		syslog (LOG_DEBUG, "%s: return, finish", fn);

		/* SUCCESS */
		return 0;
	default:
		return (response_abort (cont, -1));
	}
}

int tcp_response_recursive_retry (Context *cont) {
	return (response_abort (cont, -1));
}

int tcp_response_writing_process (Context *cont) {
	const char *fn = "tcp_response_writing_process()";

	syslog (LOG_DEBUG, "%s: start", fn);

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (context_timeout_register (cont, cont->timeout))
		return (response_abort (cont, -1));

	switch (tcp_writemesg (cont, cont->conn_sock)) {
	case 0:
		/* continue this process */
		syslog (LOG_DEBUG, "%s: return, continue", fn);

		/* SUCCESS */
		return 0;
	case 1:
		/* all data written */
		cont->process = tcp_response_waiting_client_close_process;
		cont->retry = tcp_response_waiting_client_close_retry;

		/* stop output and waiting for input */
		ev_tcp_out_remove (cont->conn_sock);
		ev_tcp_conn_in_register (cont->conn_sock, cont);

		syslog (LOG_DEBUG, "%s: return, finish", fn);

		/* SUCCESS */
		return 0;
	default:
		/* FAILURE */
		return (response_abort (cont, -1));
	}
}

int tcp_response_writing_retry (Context *cont) {
	return (response_abort (cont, -1));
}

int tcp_response_waiting_client_close_process (Context *cont) {
	const char *fn = "tcp_response_waiting_client_close_process()";
	uint16_t length_buf;
	int i;

	syslog (LOG_DEBUG, "%s: start", fn);

	i = read (cont->conn_sock, (u_char *) (&length_buf),
		  sizeof (uint16_t));
	if (!i) /* SUCCESS */
		return tcp_response_finish (cont);

	if (i < sizeof (uint16_t)) {
		/* read error */
		syslog (LOG_INFO, "%s: read(): %m", fn);
		return (response_abort (cont, -1));
	}

	/* redo query processing */
	length_buf = ntohs (length_buf);
	syslog (LOG_DEBUG, "%s: incoming length %d", fn, length_buf);

	/* free old buffer */
	if (cont->mesg.p)
		free (cont->mesg.p);

	/* allocate new buffer */
	cont->mesg.p = malloc (length_buf);
	if (!cont->mesg.p) {
		cont->mesg_len = 0;
		return (response_abort (cont, -1));
	}

	cont->mesg_len = length_buf;
	cont->wp = cont->mesg.p;
	cont->timeout = TCP_SRV_TIMEOUT;

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (context_timeout_register (cont, cont->timeout))
		return (response_abort (cont, -1));

	/*
	 * I/O event is not changed -- tcp_conn_in,
	 * go back to reading
	 */

	cont->process = tcp_response_reading_process;
	cont->retry = tcp_response_reading_retry;

	syslog (LOG_DEBUG, "%s: return, continue reading", fn);

	/* SUCCESS, but not FINISHED yet */
	return 0;
}

int tcp_response_waiting_client_close_retry (Context *cont) {
	return (response_abort (cont, 0));
}

int tcp_response_finish (Context *cont) {

	syslog (LOG_DEBUG, "tcp_response_finish()");

	context_destroy (cont);
	return 0;
}

