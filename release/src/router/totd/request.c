/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: request.c,v 3.98 2005/01/29 18:51:36 dillema Exp $>
 */

#include "totd.h"

#ifdef STF
static void collect_ns_socks (Context *);
#endif
static void cname_without_crecord (G_List *, u_char *);

/*
 * This routine is called after a request has been received by the proxy,
 * from {udp|tcp}_response_start(). There a context was created for us, and
 * the request message received has been copied into it.
 *
 * Here we then parse the request received, and we do the forwarding of it
 * or of a slightly modifed question (depending on how the proxy is
 * configured). The actual forwarded request becomes a child context of
 * the original request we recevied.
 */
int request_start (Context *cont, int tcp) {
	const char *fn = "request_start()";
	uint16_t qclass, qtype;
	u_char *cp, qname[MAX_DNAME], str[MAX_DNAME];
	struct in6_addr a6;
	int len;

	syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * get q-stuff from original message we received
	 */
#define QNAME (cont->mesg.p + sizeof (Mesg_Hdr))
	cp = mesg_skip_dname (QNAME, cont->mesg.p + cont->mesg_len);
	if (!cp) {
		syslog(LOG_INFO, "%s: malformed question", fn);
		return (1);
	}

	len = cp - QNAME;
	if (len >= MAX_DNAME) {
		syslog (LOG_INFO, "%s: malformed question", fn);
		return (1);
	}

	GETSHORT (qtype, cp);
	cont->q_type = qtype;

	GETSHORT (qclass, cp);
	cont->q_class = qclass;

	memcpy (cont->qname, QNAME, len);
	memcpy (qname, QNAME, len);
	/* be sure to always keep it NULL terminated */
	qname[len] = '\0';

	dname_decompress (str, MAX_DNAME, qname, NULL, NULL, NULL);
	syslog (LOG_INFO, "Query name: %s type: %s", str, string_rtype(qtype));

        if (*qname == EDNS0_ELT_BITLABEL) {
		/* We will and may not use qname as string further */
		qname[0] = '\0';
	} else if (len != strlen(qname) + 1) {
		/* We will and may not use qname as string further */
		qname[0] = '\0';
	}

	/*
	 * Now we extracted all we need from the original
	 * message in order to decide what we need to do as proxy.
	 */

	cont->work_state = FORWARD_REQUEST;

	/*
	 * If the original request is a reverse name lookup for
	 * an address that was produced by us, i.e. a `tricked' fake address,
	 * or a scoped address that we need to convert to its
	 * global counterpart.
	 */
	if (qtype == RT_PTR && conv_trick_is_tot_newptr (QNAME, &a6)) {
		cont->work_state = NEWPTR_TRICK_REQUEST;
		conv_trick_newptr_rq(qname, &a6);
		if (T.debug) {
			dname_decompress (str, MAX_DNAME, qname, 0, 0, 0);
			syslog (LOG_DEBUG, "%s: converted new ptr name: %s",
				fn, str);
		}
	}

	if (qtype == RT_PTR && conv_trick_is_tot_ptr (qname)) {
		cont->work_state = PTR_TRICK_REQUEST;
		conv_trick_ptr_rq(qname);
		if (T.debug) {
			dname_decompress (str, MAX_DNAME, qname, 0, 0, 0);
			syslog (LOG_DEBUG, "%s: converted ptr name: %s",
				fn, str);
		}
	}

#ifdef SCOPED_REWRITE
	if (T.scoped_prefixes && conv_is_scoped_ptr (qname, 1) != -1) {
		cont->work_state = PTR_SCOPED_TRICK_REQUEST;
		conv_scoped_ptr_rq(qname);
		if (T.debug) {
			dname_decompress (str, MAX_DNAME, qname, 0, 0, 0);
			syslog (LOG_DEBUG, "%s: converted scoped qname: %s",
			fn, str);
		}
	}
#endif

#ifdef STF
	if (T.stf && (qtype == RT_NS || qtype == RT_PTR))
		if (conv_stf_is_stf_ptr (cont->qname)) {
			cont->work_state = STF_REQUEST;
			if (T.debug) {
				dname_decompress (str, MAX_DNAME, qname,0,0,0);
				syslog (LOG_DEBUG, "%s: 6to4 rev query", fn);
			}
	}
#endif

#undef QNAME

	/*
	 * initialize list of nameservers to current forwarder
	 */
	cont->nameservers = fwd_socketlist();
	cont->current_ns = cont->nameservers->next;
	if (!cont->current_ns || !cont->current_ns->list_data) {
		syslog (LOG_ERR, "%s: no forwarders available!", fn);
		return -1;
	}

       	syslog (LOG_INFO, "Selected forwarder: %s",
		sprint_inet((struct sockaddr *)
			(cont->current_ns->list_data), str));

	/* forward the actual (modified) query ww want to make */
	return (do_forward (cont, qname, qclass, qtype, tcp));
}


int do_forward(struct context *parent, u_char *qname, uint16_t qclass,
	       uint16_t qtype, int tcp) {
	const char *fn = "do_forward()";
        static u_char buf[MAX_STREAM];     /* Buffer for TCP/UDP messages */
	u_char str[MAX_DNAME];
	Context *cont;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* create new context */
	cont = context_create();
	if (!cont)
		return (request_abort (cont, -1));

	/* we'll hook it up as a child of the given (parent) context */
	cont->parent = parent;
	cont->parent->child = cont;
	cont->timeout = SEARCH_REMOTE_TIMEOUT;
	cont->current_ns = cont->parent->current_ns;

	strlcpy (cont->qname, qname, MAX_DNAME);
	cont->q_class = qclass;
	cont->q_type = qtype;
	cont->q_id = mesg_id ();

	syslog (LOG_DEBUG, "%s: constructing query", fn);

	/*
	 * if we have an empty query string in qname, get the qname
	 * straight from the received request such that we forward
	 * it unmodified (binary or not)
	 */
	if (*qname == '\0')
		qname = (parent->mesg.p + sizeof (Mesg_Hdr));

	cont->mesg_len = mesg_make_query (qname, qtype, qclass, cont->q_id, 1,
					  buf, sizeof(buf));

	if (cont->mesg_len < 0) {
                syslog (LOG_ERR, "%s: failed to keep query in %d bytes",
			fn, sizeof(buf));
		return (request_abort (cont, -1));
	}

        cont->mesg.p = malloc (cont->mesg_len);
	if (!cont->mesg.p) {
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
		return (request_abort (cont, -1));
	}

	memcpy (cont->mesg.p, buf, cont->mesg_len);
        cont->wp = cont->mesg.p;

	if (tcp == 0) {
		dname_decompress (str, MAX_DNAME, cont->qname, 0, 0, 0);
		syslog (LOG_INFO, "Forward %s query for %s over UDP",
			string_rtype(cont->q_type), str);
		tcp = udp_request_start (cont);
	}

	if (tcp == 1) {
		dname_decompress (str, MAX_DNAME, cont->qname, 0, 0, 0);
		syslog (LOG_INFO, "Forward %s query for %s over TCP",
			string_rtype(cont->q_type), str);
		tcp = tcp_request_start (cont);
	}

	return tcp;
}

int request_retry (Context *cont) {
	char astr[MAX_DNAME], bstr[MAX_DNAME];

	cont->retry_count++;
	cont->timeout = cont->timeout * 2;

	/* mark current forwarder as unresponsive */
	fwd_mark (cont->current_ns->list_data, 1);

	if (cont->retry_count > SEARCH_REMOTE_RETRY) {
		syslog (LOG_INFO, "Exceeded retry limit");
		return -2;
	}

	/* select next nameserver in the list, if any */
	if (!cont->current_ns->next || !cont->current_ns->next->list_data)
		syslog (LOG_DEBUG, "Cycled through list of nameservers");
	else
		cont->current_ns = cont->current_ns->next;

	dname_decompress (astr, MAX_DNAME, cont->qname, 0, 0, 0);
       	syslog (LOG_INFO, "Retrying query for %s to nameserver: %s", astr,
		sprint_inet((struct sockaddr *) (cont->current_ns->list_data),
			    bstr));

	/*
	 * Note that we keep timout time high, even when we
	 * switched to another forwarder/nameserver
	 */

	syslog (LOG_DEBUG, "Retry %d of %d+%d will time out in %d seconds",
		cont->retry_count, SEARCH_REMOTE_RETRY, SEARCH_REMOTE_RETRY,
		cont->timeout);

	return 0;
}

/*
 * After making and sending a request with request_start(), do_forward() and
 * {udp|tcp}_request_start(), we get back here to process the response
 * (only if we received one of course).
 *
 * Note that it is the child that calls us, before it cleans itself, i.e. we
 * are called by {tcp|udp}_request_finish(). The response message is in our
 * child context and has already been parsed. The resource records found in
 * it, are then also available as lists in the child.
 */
int recursive_process (Context *cont) {
        const char *fn = "recursive_process()";
	u_char qname[MAX_DNAME], astr[MAX_DNAME];
	uint16_t qclass, qtype;
	int switch_to_tcp;
	uint16_t rtype;

	syslog (LOG_DEBUG, "%s: start", fn);

	/*
	 * The child context contains the response message, which we look at
	 * here directly to decide what we need to do. Note that the child
	 * context keeps describing the original request we forwarded, not 
	 * the response! (I.e. only the actual message buffer got replaced).
	 */

	if (cont->child) {
		u_char *cp;

		if (cont->child->mesg.hdr->rcode != RC_OK &&
	    	    cont->child->mesg.hdr->rcode != RC_NXDOMAIN &&
		    cont->child->mesg.hdr->rcode != RC_SERVERERR) {
			/* RC_SERVERERR is handled below to allow hack */
			cont->mesg.hdr->rcode = cont->child->mesg.hdr->rcode;
			return 1;
		}

		cp = mesg_skip_dname (cont->child->mesg.p + sizeof (Mesg_Hdr),
			cont->child->mesg.p + cont->child->mesg_len);
		if (cp) {
			/*
		 	 * we need the resource type of the response message
		 	 */
			GETSHORT (rtype, cp);
		} else {
			syslog(LOG_INFO, "%s: malformed message", fn);
			return -1;
		}
	} else {
		syslog (LOG_DEBUG, "%s: child died", fn);
		return -1;
	}

	/* initialize vars for additional request we may make */
	qclass = cont->q_class;
	qtype = cont->q_type;
	strlcpy(qname, cont->qname, MAX_DNAME);

	syslog (LOG_DEBUG, "%s: work state: %d", fn, cont->work_state);

	/*
	 * In case of a truncated response, we'll try the same request 
	 * again using TCP.
	 */
	switch_to_tcp = 0;
	if (cont->child->mesg.hdr->tc == 1) {
		syslog (LOG_DEBUG, "%s: Truncated Message", fn);
		if (cont->child->conn_sock < 0) {
			switch_to_tcp = 1;
			syslog (LOG_DEBUG, "%s: Switch to TCP", fn);
			/* skip state machine, just redo current child query */
			goto do_work;
		} else {
			/* out of options, truncated TCP message */
			syslog (LOG_DEBUG, "%s: TCP message too big", fn);
			return -1;
		}
	}

	/*
	 * Some misbehaving old nameservers simply fail to process a 
	 * AAAA query and return a RC_SERVERERR (aka. SERVFAIL)
	 * error response. If such is the case, we hack around it
	 * and let totd pretend it got a proper NXDOMAIN response.
	 */
	if (cont->child->mesg.hdr->rcode == RC_SERVERERR) { 
	    	if (cont->work_state == FORWARD_REQUEST &&
		    (qtype == RT_AAAA || qtype == RT_A6)) {
			cont->child->mesg.hdr->rcode = RC_NXDOMAIN;
		} else {
			/* behave as normal in case of failure (see above) */
                        cont->mesg.hdr->rcode = cont->child->mesg.hdr->rcode;
                        return 1;
		}
	}

	/*
	 * we changed a AAAA or A6 query into a A query, now construct
	 * AAAA or A6 replies with prefix to match original request.
	 */
	if (cont->work_state == TRICK_REQUEST) {
		if (T.prefixnum && rtype == RT_A &&
		    (qtype == RT_AAAA || qtype == RT_A6)) {
			conv_trick_list (cont->child->an_list, qtype, 0);
			conv_trick_list (cont->child->ns_list, qtype, 0);
			/* circulate through prefixes */
                        T.current_prefix = (T.current_prefix + 1) % T.prefixnum;
			syslog (LOG_DEBUG, "%s: %s query changed into A query",
				fn, qtype == RT_AAAA ? "AAAA" : "A6");
		}
		cont->work_state = WORK_DONE;
	}

	if (cont->work_state == PTR_TRICK_REQUEST) {
		conv_trick_ptr (cont->child->an_list, cont->qname);
		if (T.debug) {
			dname_decompress (astr, MAX_DNAME, cont->qname, 0,0,0);
			syslog (LOG_DEBUG, "%s: converted PTR response %s",
				fn, astr);
		}
		cont->work_state = WORK_DONE;
	}

	if (cont->work_state == NEWPTR_TRICK_REQUEST) {
		conv_trick_newptr(cont->child->an_list, cont->qname);
		if (T.debug) {
			dname_decompress (astr, MAX_DNAME, cont->qname, 0,0,0);
			syslog (LOG_DEBUG, "%s: converted new PTR response: %s",
				fn, astr);
		}
		cont->work_state = WORK_DONE;
	}

	/* 
	 * If we got no answers to our query, we may need to do our trick. 
	 * I.e. we may need to send another query and use it to fabricate
	 * some answers...
	 */
	if (cont->work_state == FORWARD_REQUEST) {
		if (T.prefixnum && (cont->child->mesg.hdr->rcode == RC_NXDOMAIN ||
		    (cont->child->mesg.hdr->rcode == RC_OK && (!cont->child->mesg.hdr ||
                         cont->child->mesg.hdr->ancnt == htons(0))))) {
			/* We got no answers in the reply we got to our query. 
		 	 * We check here whether we need to modify the query
		 	 * to get answers we can transform into (pseudo-)
		 	 * answer records for the original query.
		 	 */
			if (rtype == RT_AAAA || rtype == RT_A6) {
				syslog (LOG_DEBUG, "Changed target to A");

				/* same query, but IPv4 address type */
				strlcpy(qname, cont->qname, MAX_DNAME);
				qtype = RT_A;

				cont->work_state = TRICK_REQUEST;
			}
		}
	}

	/* if we are just forwarding without tricks, then we are done */
	if (cont->work_state == FORWARD_REQUEST)
		cont->work_state = WORK_DONE;

#ifdef STF
	/* 
	 * State Machine for reverse lookup (PTR) of 6to4 address.
	 */

	if (cont->work_state == STF_REQUEST) {
		/*
	 	 * We forwarded 6to4 PTR request as normal, if we did not get
		 * answers from that we start our 6to4 PTR trick. First we need
		 * to find the nameservers for the IPv4 reverse zone that is
		 * embedded in the 6to4 address.
	 	 */
		if (cont->child->mesg.hdr->rcode == RC_NXDOMAIN) {
			/*
			 * 6to4 address is like 2002:AABB:CCDD:...
			 * PTR request is the like:
			 *	 ...d.d.c.c.b.b.a.a.2.0.0.2.ip6.int.
			 *
			 * Here, we are only interested in the final part of
			 * the PTR request that contains the embedded IPv4
			 * address.
			 * The last 48 bits of the address in the PTR record
			 * are 32 characters (digit and length bytes).
			 */
			int len = strlen(cont->qname) -
				(strlen(cont->qname) > 32) ? 32 : strlen(qname);
			strlcpy(cont->stfname, cont->qname + len , MAX_DNAME);
			cont->work_state = STF_NSLIST;
		} else
			cont->work_state = STF_DONE;
	}

	syslog (LOG_DEBUG, "%s: work state: %d", fn, cont->work_state);

	/*
	 * We forward our 6to4 request to each nameserver in the list untill we
	 * find one that gives us an answer.
	 */
	if (cont->work_state == STF_FORWARDING &&
	    cont->child->mesg.hdr->ancnt != htons(0))
		cont->work_state = STF_DONE;

	syslog (LOG_DEBUG, "%s: work state: %d", fn, cont->work_state);

	if (cont->work_state == STF_NSLIST) {
		/*
		 * we are trying to find the nameservers for the embedded IPv4
		 * zone.
		 * if we found these for IPv4 PTR zone, then start forwarding
	 	 * to those to find the 6to4 PTR record. if not, move up the PTR
	 	 * hierarchy, and query again for nameservers
	 	 */
		if (cont->child->mesg.hdr->ancnt == htons(0)) {
			/* move up pointer hierarchy (two nybbles == 4 chars) */
			memmove(cont->stfname, cont->stfname+4, MAX_DNAME-4);
			/* if no IPv4 address zone left, give up */
			if (strlen(cont->stfname) < 16) 
				cont->work_state = STF_DONE;
		} else if (cont->q_type == RT_PTR) {
			collect_ns_socks(cont);
			/* set current ns to first one in the list */
			cont->current_ns = cont->nameservers;
			cont->work_state = STF_FORWARDING;
		} else {
			/* RT_NS request for 6to4
		 	 * We transform it into a nameserver
			 * pseudo-record here for the 6to4 address.
	 	 	 * This pseudo-record is marked non-authorative. 
		 	 */

			/* mark non-authorative */
			cont->child->mesg.hdr->aa = 0;
			conv_stf_ns_list(cont->child->an_list);
			conv_stf_ns_list(cont->child->ns_list);
			conv_stf_ns_list(cont->child->ar_list);
			if (T.debug) {
				dname_decompress(astr, MAX_DNAME, qname, 0,0,0);
				syslog (LOG_DEBUG, "Converted NS record for \
6to4 NS query: %s", astr);
			}

			cont->work_state = STF_DONE;
		}
	}

	syslog (LOG_DEBUG, "%s: work state: %d", fn, cont->work_state);

	switch (cont->work_state) {
	case STF_NSLIST:
		/* get embedded IPv4 address in 6to4 address */
		strlcpy(qname, cont->stfname, MAX_DNAME);
		if (conv_stf_ptr (qname) < 0)
			return -1;

		if (T.debug) {
			dname_decompress(astr, MAX_DNAME, cont->qname, 0,0,0);
			syslog (LOG_DEBUG, "Converted 6to4 PTR: %s", astr);
		}
		qtype = RT_NS;
		break;
	case STF_FORWARDING:
		cont->current_ns = cont->current_ns->next;

		if (!cont->current_ns->list_data) {
			if (T.debug) {
				dname_decompress(astr, MAX_DNAME, cont->qname,
						 0,0,0);
				syslog (LOG_DEBUG, "%s: tried all nameservers \
found for 6to4 PTR name: %s", fn, astr);
			}
			cont->work_state = STF_DONE;
		}

		if (T.debug) {
			dname_decompress(astr, MAX_DNAME, cont->qname, 0,0,0);
			syslog (LOG_DEBUG, "%s: 6to4 PTR req to: %s", fn, astr);
		}

		strlcpy(qname, cont->qname, MAX_DNAME);
		qtype = RT_PTR;
		break;
	default:
		break;
	}
#endif

	syslog (LOG_DEBUG, "%s: current work state: %d", fn, cont->work_state);

	if (cont->work_state == WORK_DONE) {
		/*
		 * We should be finished then... unless we only got a CNAME
		 * record in our answers and no address record with it to
		 * resolve it. In that case, we need to do another query to
		 * our forwarder to resolve it properly.
		 *
		 * Note that we only follow CNAME's for the actual answer
		 * record (and assume there's only one), not for the ns or ar
		 * records.
		 */

		/* unresolved CNAME in our answer list? */
		cname_without_crecord (cont->child->an_list, qname);
		if (*qname) {
			/*
			 * if so, should we follow it? standard (RFC) only
			 * allows a single CNAME, but CNAME chaining is rather
			 * common in practice. We follow chains a bit, but
			 * avoid following a CNAME loop by limiting its length.
			 */
			if (cont->cname_links++ > SEARCH_CNAME_LEVEL) {
				syslog (LOG_DEBUG, "%s: exceeded max number \
of CNAME links: depth %d. A loop perhaps?", fn, cont->cname_links);
				return -1;
			}

			/* change target and start again */
			syslog (LOG_DEBUG, "Changed target to CNAME name");
			cont->work_state = FORWARD_REQUEST;
		}
	}

	/*
	 * if we decided somewhere above that we need to do another query to
	 * formulate our response, we do so here
	 */
do_work:
	if (cont->work_state != WORK_DONE) {
        	char astr[MAX_DNAME];

		/* Now just forget about old child. child will free itself */
		cont->child->parent = NULL;
		cont->child = NULL;

		if (cont->current_ns->list_data)
        		syslog (LOG_INFO, "Selected nameserver: %s",
                    		sprint_inet((struct sockaddr *)
					(cont->current_ns->list_data), astr));

		/*
		 * send out query in a new child context, use same
		 * transport as before.
		 */
		if (cont->conn_sock < 0 && !switch_to_tcp)
			return do_forward (cont, qname, qclass, qtype, 0);
		else 
			return do_forward (cont, qname, qclass, qtype, 1);
	}

	/*
	 * Now we are ready to formulate our answer to the original query.
	 * We do the final manipulations, if any, here.
	 */

	/* rewrite additional section if needed */
	if (T.prefixnum) {
		if (cont->q_type == RT_ALL) {
			syslog (LOG_DEBUG, "Add IPv6 addresses to answers");
			conv_trick_list(cont->child->an_list, RT_AAAA, 1);
		}

		syslog (LOG_DEBUG, "Add IPv6 addresses to additional section");
		conv_trick_list(cont->child->ar_list, RT_AAAA, 1);
		/* conv_trick_list(cont->child->ar_list, RT_A6, 1); */

		/* circulate through prefixes */
		T.current_prefix =
			(T.current_prefix + 1) % T.prefixnum;
	}

#ifdef SCOPED_REWRITE
	if (qtype == RT_AAAA && T.scoped_prefixes && conv_scoped_query(cont)) {
		conv_scoped_list(cont->child->an_list);
		conv_scoped_list(cont->child->ns_list);
		conv_scoped_list(cont->child->ar_list);
        	syslog (LOG_DEBUG, "%s: checked whether to rewrite global \
into scoped address", fn);
	}

	if (cont->work_state == PTR_SCOPED_TRICK_REQUEST) {
		if (conv_scoped_query(cont)) {
			conv_scoped_ptr(cont->child->an_list,
					cont->child->qname);
			if (T.debug) {
				dname_decompress(astr, MAX_DNAME,
						 cont->child->qname, 0,0,0);
				syslog (LOG_DEBUG, "Converted query name: %s",
					astr);
			}
		}
	}
#endif

	/*
	 * Ok, end of sleazy tricks. If and when we get here, everything is
	 * quite normal. We made request and the response to it has been parsed
	 * and available in our child context. We copy the resulting resource
	 * records to our (the parent) context, and return to the caller
	 * {udp|tcp}_recursive_process() which is responsible for
	 * using the acquired results to send a response to the client.
	 */

	list_destroy (cont->an_list, rrset_freev);
	cont->an_list = cont->child->an_list;
	cont->child->an_list = NULL;

	list_destroy (cont->ns_list, rrset_freev);
	cont->ns_list = cont->child->ns_list;
	cont->child->ns_list = NULL;

	list_destroy (cont->ar_list, rrset_freev);
	cont->ar_list = cont->child->ar_list;
	cont->child->ar_list = NULL;

	syslog (LOG_DEBUG, "%s: finish", fn);
	return 1;		/* recursive query finished */
}

int request_abort (Context *cont, int status) {

	if (cont && status == -2) {
		/* we timed out, do not send error response */
		cont->parent->child = NULL;
		response_abort(cont->parent, -2);
		cont->parent = NULL;
	}

	/* if parent exists, process parent */
	if (cont && cont->parent) {
		cont->parent->child = NULL;	/* indicates child failure */
		cont->parent->process (cont->parent);
		cont->parent = NULL;
	}
	/* free all resources */
	context_destroy (cont);

	return (status);
}

int request_finish (Context *cont) {
	char *fn = "request_finish()";

	/*
	 * We have successfully sent a request and have received a
	 * response to it. Here we finish the request by deciding
	 * whether the response is ok or not.
	 */

	syslog (LOG_DEBUG, "%s: start", fn);

	/* re-initialize answer, nameserver and additional record lists */
	list_destroy (cont->an_list, rrset_freev);
	list_destroy (cont->ns_list, rrset_freev);
	list_destroy (cont->ar_list, rrset_freev);
	cont->an_list = list_init ();
	cont->ns_list = list_init ();
	cont->ar_list = list_init ();
	if (!cont->an_list || !cont->ns_list || !cont->ar_list)
		return request_abort (cont, -1);

	/* Try to parse the response message */
	if (cont->mesg.hdr->tc == 0) {
		if (mesg_parse (cont->mesg.p, cont->mesg_len, cont->an_list,
			     cont->ns_list, cont->ar_list)) {
			syslog (LOG_WARNING, "%s: can't parse answer data", fn);
			return request_abort (cont, -1);
		}
	}

	/* mark success to forwarder */
	fwd_mark (cont->current_ns->list_data, -1);

	/*
	 * if we have a parent, finish processing the parent
	 *
	 * Note: the parent will not cleanup its children...
	 */
	if (cont->parent) {
		syslog (LOG_DEBUG, "%s: process parent context", fn);
		cont->parent->process (cont->parent);
	}

	/* ... so we cleanup ourselves in any case */
	context_destroy (cont);

	syslog (LOG_DEBUG, "%s: return success", fn);
	return 0; /* SUCCESS */
}

#ifdef STF
/*
 * Expects list of NS records in AN section of given context.
 * It searches for address records (A or AAAA) in the AR section of the
 * context matching these name servers. Each address thus found is put in
 * an appropriate sockaddr structure. These are then returned as
 * an array of sockaddr structs. (Caller's responsibility to free them).
 */
void collect_ns_socks (Context *cont) {
        const char *fn = "collect_ns_socks()";
	G_List* gl;

	syslog (LOG_DEBUG, "%s: start", fn);

	/* `Steal' nameservers from child */
	list_destroy (cont->an_list, rrset_freev);
	list_destroy (cont->ns_list, rrset_freev);
	list_destroy (cont->ar_list, rrset_freev);
	cont->an_list = cont->child->an_list;
	cont->ns_list = cont->child->ns_list;
	cont->ar_list = cont->child->ar_list;

	/*
 	 * We, the parent, take over responsibility for
 	 * cleaning/freeing these lists we stole from the
 	 * child. 
 	 */
	cont->child->an_list = NULL;
	cont->child->ns_list = NULL;
	cont->child->ar_list = NULL;

	/* kill/free any old list */
	if (cont->nameservers)
		list_destroy(cont->nameservers, free);

	/* init the list of sockaddr structs */
	cont->nameservers = list_init();

	/*
	 * We cycle through the list of answers, which are
	 * NS (nameserver) records.
	 */
	for (gl = cont->an_list->next; gl->list_data; gl = gl->next) {
        	RRset *glue_record, *ns_record;
		struct sockaddr* sa_p;
		int ns, glue;

		ns_record = gl->list_data;

		for (ns = 0; ns < ns_record->data.d->data_cnt; ns++) {
			char *name;
			int len;
			RR* rr;

			rr = (RR*) (ns_record->data.p +
			 	data_offset(ns, ns_record->data.d));
			/*
			 * Extract the domainname and its length from the
			 * given record.
			 */
			len = rr->rd_len;
			name = rr_rdata(rr);

#ifdef USE_INET6
			/* try to find glue within AR record list */
			glue_record = search_name(cont->ar_list, name, len,
						  RT_AAAA);
			if (glue_record) {
				for (glue = 0; glue < glue_record->data.d->data_cnt; glue++) {
        				struct sockaddr_in6 *sin6_p;

					rr = (RR*) (glue_record->data.p
			   		 	 + data_offset(glue, glue_record->data.d));

					sa_p = (struct sockaddr *) malloc (sizeof(struct sockaddr_in6));
					if (!sa_p)
						return;

					memset ((void *) sa_p, 0, sizeof(struct sockaddr_in6));
        				sa_p->sa_family = AF_INET6;
#ifdef HAVE_SA_LEN_FIELD
        				sa_p->sa_len = sizeof (struct sockaddr_in6);
#endif
                			sin6_p = (struct sockaddr_in6 *) sa_p;
                			sin6_p->sin6_port = htons (PORT_TO);
                			sin6_p->sin6_flowinfo = htonl (0);
                			memcpy (sin6_p->sin6_addr.s6_addr, rr_rdata(rr),
						sizeof(sin6_p->sin6_addr.s6_addr));

					/* add sa_p to list */
					syslog (LOG_DEBUG, "%s: .adding nameserver address", fn);
					list_add_tail (cont->nameservers, sa_p);
				}
				rrset_free(glue_record);
			}
#endif /* USE_INET6 */

#ifdef USE_INET4
			glue_record = search_name(cont->ar_list, name, len, RT_A);
			if (glue_record) {
				for (glue = 0; glue < glue_record->data.d->data_cnt; glue++) {

					rr = (RR*)(glue_record->data.p
			   		 	 + data_offset(glue, glue_record->data.d));

#ifdef USE_INET6
					if (T.use_mapped) {
       						struct sockaddr_in6 *sin6_p;

						sa_p = (struct sockaddr *) malloc (sizeof(struct sockaddr_in6));
						if (!sa_p)
							return;
						memset ((void *) sa_p, 0, sizeof(struct sockaddr_in6));

       						sa_p->sa_family = AF_INET6;
#ifdef HAVE_SA_LEN_FIELD
        					sa_p->sa_len = sizeof (struct sockaddr_in6);
#endif
						sin6_p = (struct sockaddr_in6 *) sa_p;
       		       		  		sin6_p->sin6_port = htons (PORT_TO);
       		         			sin6_p->sin6_flowinfo = htonl (0);

						/* make IPv4 MAPPED IPv6 address */
						*(uint32_t *)(void *)(&sin6_p->sin6_addr.s6_addr[8]) = ntohl(0x0000ffff);
               					memcpy (&sin6_p->sin6_addr.s6_addr[12], rr_rdata(rr),
							sizeof(struct in_addr));
						/* add sa_p to list */
						syslog (LOG_DEBUG, "%s: ..adding nameserver mapped address", fn);
						list_add_tail (cont->nameservers, sa_p);
					}
#endif

					if (!T.use_mapped) {
       						struct sockaddr_in *sin_p;

						sa_p = (struct sockaddr *) malloc (sizeof(struct sockaddr_in));
						if (!sa_p)
							return;
						memset ((void *) sa_p, 0, sizeof(struct sockaddr_in));

       						sa_p->sa_family = AF_INET;
#ifdef HAVE_SA_LEN_FIELD
       						sa_p->sa_len = sizeof (struct sockaddr_in);
#endif
               					sin_p = (struct sockaddr_in *) sa_p;
               					sin_p->sin_port = htons (PORT_TO);
               					memcpy (&sin_p->sin_addr.s_addr, rr_rdata(rr),
							sizeof(sin_p->sin_addr.s_addr));
						/* add sa_p to list */
						syslog (LOG_DEBUG, "%s: ...adding nameserver address", fn);
						list_add_tail (cont->nameservers, sa_p);
					}
				} /* for */
				rrset_free(glue_record);
			} /* if (glue_record) */
#endif /* USE_INET4 */
		}
	}

	syslog (LOG_DEBUG, "%s: return", fn);

  	return;
}
#endif

void cname_without_crecord (G_List *an_list, u_char *cname) {
        char *fn = "cname_without_crecord()";
	u_char *canonical_domain, *domain;
	int canonical_domain_len;
	RRset *rrs_tmp;
	RR *rr_tmp;
	G_List *gl;

	syslog (LOG_DEBUG, "%s: Searching for CNAME in answers", fn);

	/* start with nothing */
	*cname = '\0';
	canonical_domain = NULL;
	canonical_domain_len = 0;

	an_list->list_data = NULL; /* termination safety pin */
	for (gl = an_list->next; gl->list_data; gl = gl->next) {
		rrs_tmp = (RRset *) gl->list_data;
		if (rrs_tmp->key.info->r_type == RT_CNAME) {
			rr_tmp = (RR *) (rrs_tmp->data.p +
				 data_offset (0, rrs_tmp->data.p));
			canonical_domain = rr_rdata (rr_tmp);
			canonical_domain_len = rr_tmp->rd_len;
		}
	}

	if (!canonical_domain) 
		return;	/* no problem -- no cname record */

	an_list->list_data = NULL;
	for (gl = an_list->next; gl->list_data; gl = gl->next) {
		rrs_tmp = (RRset *) gl->list_data;
		domain = rrset_owner (rrs_tmp);

		if ((rrs_tmp->key.info->owner_len == canonical_domain_len)
		    && !mesg_dname_cmp (NULL, domain, canonical_domain))
			return;
	}

	syslog (LOG_DEBUG, "%s: Problem: CNAME without canonical record", fn);

	if (canonical_domain_len >= MAX_DNAME) {
		syslog (LOG_DEBUG, "%s: malformed message", fn);
		return;
	}

	memcpy(cname, canonical_domain, canonical_domain_len);
	cname[canonical_domain_len] = '\0'; /* terminate it as a string */

	return;
}
