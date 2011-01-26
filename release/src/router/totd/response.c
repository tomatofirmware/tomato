/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: response.c,v 3.8 2005/01/29 18:51:36 dillema Exp $>
 */

#include "totd.h"

int response_abort (Context *cont, int status) {
        const char *fn = "response_abort()";

	syslog (LOG_DEBUG, "%s", fn);

	context_destroy (cont);
	return status;
}

int assemble_response (Context *cont) {
	const char *fn = "assemble_answer()";
        static u_char buf[MAX_STREAM];     /* Buffer for TCP/UDP messages */
	int len;

	syslog (LOG_DEBUG, "%s: start", fn);

	if (cont->mesg.hdr->rcode == RC_OK) {
		/*
		 * We got an OK message cont->mesg and we have parsed it
		 * and processed it (into cont->??_lists). Now we
		 * assemble a message from the lists again.
		 */
		len = mesg_assemble (cont->an_list, cont->ns_list, cont->ar_list,
				     buf, sizeof (buf), cont->mesg.p, cont->mesg_len);
		if (len < 0)
			return (-1);

		/* free old buffer */
		if (cont->mesg.p)
			free (cont->mesg.p);

		/* allocate a new properly sized one */
		cont->mesg.p = malloc (len);
		if (!cont->mesg.p)
			return -1;

		memcpy (cont->mesg.p, buf, len);
		cont->mesg_len = len;

		/* copy child RCODE and AA as we did well ourselves */
		if (cont->child) {
			cont->mesg.hdr->rcode = cont->child->mesg.hdr->rcode;
			cont->mesg.hdr->aa = cont->child->mesg.hdr->aa;
			cont->mesg.hdr->ra = cont->child->mesg.hdr->ra;
		}
	} else {
		cont->mesg.hdr->ra = 1; /* pretend? */
		cont->mesg.hdr->qr = 1;
        	cont->mesg.hdr->ancnt = 0;
        	cont->mesg.hdr->nscnt = 0;
        	cont->mesg.hdr->arcnt = 0;
	}

	/* SUCCESS */
	return (cont->mesg_len);
}

