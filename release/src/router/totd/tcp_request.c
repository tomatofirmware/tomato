/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: tcp_request.c,v 3.58 2005/01/27 11:50:29 dillema Exp $>
 */

#include "totd.h"

int tcp_request_start (struct context *cont) {
	const char *fn = "tcp_request_start()";
	struct sockaddr *sa;
	const int on = 1;
	int sock, timeout;

	syslog (LOG_DEBUG, "%s: start", fn);

	cont->process = tcp_request_wait_connect_process;
	cont->retry = tcp_request_wait_connect_retry;

	/*
	 * in case something goes wrong in setting up the TCP
	 * we will retry immediately with zero timeout (with next forwarder)
	 */
	timeout = 0;

	/*
	 * Note that this socket will be closed by context_destroy()
	 * which is called by request_abort() or request_finish()
	 */
	sa = (struct sockaddr *) cont->current_ns->list_data;
	sock = socket (sa->sa_family, SOCK_STREAM, 0);

	/* non-blocking IO, or die */
	if (sock == -1 || ioctl (sock, FIONBIO, (char *) &on) == -1) {
		syslog (LOG_WARNING, "%s: can't open socket: %m", fn);
		/* FAILURE */
		return (request_abort (cont, -1));
	}

	/* try to connect it */
	if (!connect (sock, sa, SOCKADDR_SIZEOF(*sa)) || errno == EINPROGRESS) {
		cont->conn_sock = sock;
		timeout = cont->timeout;

		if (ev_tcp_out_register (sock, cont) < 0)
			return (request_abort (cont, -1));
	} else {
		syslog (LOG_WARNING, "%s: can't connect: %m", fn);
                timeout = 0; /* force immediate retry */
	}

	if (context_timeout_register (cont, timeout) < 0)
		return (request_abort (cont, -1));

	syslog (LOG_DEBUG, "%s: Query will time out in %d seconds.",
			fn, cont->timeout);

	return 0;
}

int tcp_request_wait_connect_process (Context *cont) {
	char *fn = "tcp_request_wait_connect_process()";
	socklen_t errlen;
	int sockerr;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* disable old timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	/* and add a new fresh one */
	if (context_timeout_register (cont, cont->timeout))
		return (request_abort (cont, -1));

	/* check whether connect() actually succeeded or not. */
	errlen = sizeof (sockerr);
	if (getsockopt (cont->conn_sock, SOL_SOCKET, SO_ERROR,
			(void *) &sockerr, &errlen))
		return (request_abort (cont, -1));

	switch (sockerr) {
	case 0:
		syslog (LOG_DEBUG, "%s: TCP connect succeeded", fn);
		/* change state to writing */
		cont->process = tcp_request_writing_process;
		cont->retry = tcp_request_writing_retry;

		/* 
		 * Reset wp to make sure writing
		 * process begins from first byte
		 */
		cont->wp = NULL;
		return 0;
	case ETIMEDOUT:
		syslog (LOG_DEBUG, "%s: TCP forwarder connect timed out", fn);
		return tcp_request_wait_connect_retry(cont);
	case ECONNREFUSED:
		syslog (LOG_WARNING, "Forwarder refused TCP connection");
		return tcp_request_wait_connect_retry(cont);
	case ENETUNREACH:
		syslog (LOG_WARNING, "TCP forwarder unreachable");
		return tcp_request_wait_connect_retry(cont);
	default:
		/*
		 * All else is local server failure. Not worth a retry,
		 * not even to a different forwarder
		 */
		return (request_abort (cont, -1));
	}
}

int tcp_request_wait_connect_retry (Context *cont) {
	char *fn = "tcp_request_wait_connect_retry";
	struct sockaddr *sa;
        const int on = 1;
	int sock, timeout;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* disable old timeout, in case it is still there */
	if (cont->tout)
		cont->tout->handler = NULL;

	/* remove old I/O List */
	if (cont->conn_sock >= 0) {
		ev_tcp_out_remove (cont->conn_sock);
		/* stop current connection */
		close (cont->conn_sock);
		cont->conn_sock = -1;
	}

        if (request_retry (cont) < 0)
                return (request_abort (cont, -1));

	/*
	 * in case something goes wrong in setting up the TCP
	 * we will retry immediately with zero timeout (with next forwarder)
	 */
	timeout = 0;

	/* create new socket: non-blocking IO, or die */
	sa = (struct sockaddr *) cont->current_ns->list_data;
	sock = socket (sa->sa_family, SOCK_STREAM, 0);

	if (sock == -1 || ioctl (sock, FIONBIO, (char *) &on) == -1) {
		syslog (LOG_WARNING, "%s: can't open socket: %m", fn);
		return (request_abort (cont, -1));
	}

	/* try to connect it */
	if (!connect (sock, sa, SOCKADDR_SIZEOF(*sa))
	    || errno == EINPROGRESS) {

		cont->conn_sock = sock;
		timeout = cont->timeout;

		/* put me to input list */
		if (ev_tcp_out_register (cont->conn_sock, cont) < 0)
			return (request_abort (cont, -1));
	}

	if (!timeout) {
		syslog (LOG_WARNING, "%s: can't TCP connect: %m", fn);
                syslog (LOG_DEBUG, "%s: force retry at zero timeout", fn);
	}

	/* put me to timeout list */
	if (context_timeout_register (cont, timeout) < 0)
		return (request_abort (cont, -1));

	syslog (LOG_DEBUG, "%s: Query will time out in %d seconds.",
		fn, cont->timeout);

	/* SUCCESS */
	return 0;
}

int tcp_request_writing_process (Context *cont) {
        char *fn = "tcp_request_writing_process";

	syslog (LOG_DEBUG, "%s: start", fn);

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (!context_timeout_register (cont, cont->timeout)) {
		switch (tcp_writemesg (cont, cont->conn_sock)) {
		case 0: /* continue this process */
			syslog (LOG_DEBUG, "%s: return, continue writing", fn);
			return 0;
		case 1: /* all data written */
			if (!ev_tcp_conn_in_register (cont->conn_sock, cont)) {
				/* delete old I/O list */
				ev_tcp_out_remove (cont->conn_sock);

				/* go to readlen context */
				cont->process = tcp_request_readlen_process;
				cont->retry = tcp_request_readlen_retry;

				/* SUCCESS */
				return 0;
			}
			/* FAILURE */
			break;
		default:
			/* FAILURE */
			break;
		}
	}

	return (request_abort (cont, -1));

}

int tcp_request_writing_retry (Context *cont) {
	syslog (LOG_NOTICE, "tcp connection not responding.... closed");
	return (request_abort (cont, -1));
}

int tcp_request_readlen_process (Context *cont) {
	char *fn = "tcp_request_readlen_process()";
	uint16_t length_buf;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (context_timeout_register (cont, cont->timeout) < 0)
		return (request_abort (cont, -1));

	/* read length buffer */
	if (read (cont->conn_sock, (u_char *) (&length_buf), sizeof (uint16_t))
	    < sizeof (uint16_t)) {
		syslog (LOG_NOTICE, "cannot read length on TCP connection.");
		return (request_abort (cont, -1));
	}
	length_buf = ntohs (length_buf);

	syslog (LOG_DEBUG, "%s: data length = %d", fn, length_buf);

	free (cont->mesg.p);
	cont->mesg_len = 0;

	cont->mesg.p = malloc (length_buf);
	if (!cont->mesg.p) {
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
		return (request_abort (cont, -1));
	}
	cont->mesg_len = length_buf;
	cont->wp = cont->mesg.p;

	/* next state */
	cont->process = tcp_request_reading_process;
	cont->retry = tcp_request_reading_retry;
	return 0;
}

int tcp_request_readlen_retry (Context *cont) {
	char *fn = "tcp_request_readlen_retry()";

	syslog (LOG_NOTICE, "TCP connection not respond.... closed");

	/* Fast GiveUp Hack */
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
		return 0; /* SUCCESS */
	}
	return (request_abort (cont, -1));
}

int tcp_request_reading_process (Context *cont) {
	const char *fn = "tcp_request_reading_process()";

	syslog (LOG_DEBUG, "%s: start", fn);

	/* renew timeout */
	if (cont->tout)
		cont->tout->handler = NULL;

	if (!context_timeout_register (cont, cont->timeout)) {
		int len;

		len = read (cont->conn_sock, cont->wp,
			    cont->mesg_len - (cont->wp - cont->mesg.p));

		if (len <= 0)
			syslog (LOG_NOTICE, "%s: Read failed on TCP: %m", fn);
		else {
			cont->wp += len;
			if (cont->wp < (cont->mesg.p + cont->mesg_len))
				return 0; /* continue reading */
			else
				return request_finish (cont);
		}
	}

	/* FAILURE */
	return (request_abort (cont, -1));
}

int tcp_request_reading_retry (Context *cont) {
	syslog (LOG_NOTICE, "connection not responding... closed");
	return (request_abort (cont, -1));
}

int tcp_writemesg (Context *cont, int sock) {
	char *fn = "tcp_writemesg()";
	uint16_t lenbuf_nbo;
	int len;

	if (!cont->wp) {
		/* first time: write length */
		lenbuf_nbo = htons (cont->mesg_len);
		len = write (sock, (u_char *)(&lenbuf_nbo), sizeof (uint16_t));
		if (len < 0) {
			syslog (LOG_INFO, "%s: write length failed: %m", fn);
			return -1;
		}
		cont->wp = cont->mesg.p;
		return 0;
	}

	/* been here before */
	len = write (sock, cont->wp,
		     cont->mesg_len - (cont->wp - cont->mesg.p));
	if (len < 0) {
		syslog (LOG_INFO, "%s: write failed: %m", fn);
		return -1;
	}
	cont->wp += len;

	if (cont->wp < cont->mesg.p + cont->mesg_len)
		return 0;
	else
		return 1;

	/* NOTREACHED */
	abort ();
}

