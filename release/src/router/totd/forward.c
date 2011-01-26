/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: forward.c,v 3.41 2005/01/28 12:24:34 dillema Exp $>
 */

#include "totd.h"

char *sprint_inet(struct sockaddr *sa, char *address_str) {
    char tmp[MAX_DNAME];

	address_str[0] = '\0';
#ifdef USE_INET4
    	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *sin_p = (struct sockaddr_in *) sa;
		inet_ntop (sin_p->sin_family, (void *) &sin_p->sin_addr, tmp,
			   MAX_DNAME);
		snprintf (address_str, MAX_DNAME, "[%s]:%d", tmp,
			  ntohs(sin_p->sin_port));
    	}
#endif
#ifdef USE_INET6
    	if (sa->sa_family == AF_INET6) {
		struct sockaddr_in6 *sin6_p = (struct sockaddr_in6 *) sa;
		inet_ntop(sin6_p->sin6_family, (void *) &sin6_p->sin6_addr,
			  tmp, MAX_DNAME);
		snprintf (address_str, MAX_DNAME, "[%s]:%d", tmp,
			  ntohs(sin6_p->sin6_port));
    	}
#endif
    	return address_str;
}

struct sockaddr *parse_and_alloc_addr (char *caddr, int port, int *sa_len_ret) {
	char address[MAX_DNAME] = "";
	struct sockaddr *sa_p;
	int sa_len, af = 0;
	char *colon = NULL;

#ifdef USE_INET4
	sa_len = sizeof (struct sockaddr_in);
	af = AF_INET;
#endif

#ifdef USE_INET6
	colon = strchr (caddr, ':');
	if (colon || T.use_mapped) {
		sa_len = sizeof (struct sockaddr_in6);
		af = AF_INET6;
	}
#endif

	if (!af)
		return NULL;

	sa_p = malloc (sa_len);
	if (!sa_p)
		return NULL;
	memset ((void *) sa_p, 0, sa_len);
#ifdef HAVE_SA_LEN_FIELD
	sa_p->sa_len = sa_len;
#endif
	sa_p->sa_family = af;

	if (!colon && T.use_mapped)
		strcpy(address, "::ffff:");

	if (strlcat (address, caddr, MAX_DNAME) >= sizeof(address))
		return NULL;

#ifdef USE_INET4
	if (af == AF_INET) {
		struct sockaddr_in *sin_p;

		sin_p = (struct sockaddr_in *) sa_p;
		sin_p->sin_port = htons (port);
		if (!inet_aton (address, &sin_p->sin_addr)) {
			free(sa_p);
			return NULL;
		}
	}
#endif

#ifdef USE_INET6
	if (af == AF_INET6) {
		struct addrinfo hints, *res;
		char portstr[NI_MAXSERV];
		int error;

		snprintf(portstr, NI_MAXSERV, "%d", port);

		memset(&hints, 0, sizeof(hints));
           	hints.ai_family = af;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_NUMERICHOST;
           	error = getaddrinfo(address, portstr, &hints, &res);
           	if (error) {
                	syslog(LOG_ERR, "%s", gai_strerror(error));
			if (res)
				freeaddrinfo(res);
			return NULL;
		}
		memcpy(sa_p, res->ai_addr, sa_len);

		if (res)
			freeaddrinfo(res);
	}
#endif

	if (sa_len_ret)
		*sa_len_ret = sa_len;

	return sa_p;
}

void fwd_free (Fwd *fwd_ptr) {
    if (fwd_ptr) {
	if (fwd_ptr->sa)
	    free (fwd_ptr->sa);
	
	free (fwd_ptr);
    }
    return;
}

void fwd_freev (void *fwd_p) {
	fwd_free ((Fwd *) fwd_p);
	return;
}

Fwd *fwd_alloc (void) {
	Fwd *fwd_p = NULL;	/* alloc'ed */
	char *fn = "fwd_alloc()";

	fwd_p = malloc (sizeof (Fwd));
	if (!fwd_p) {
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
		goto error;
	}
	fwd_p->sa = malloc (sizeof (struct sockaddr_storage));
	if (!fwd_p->sa) {
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
		goto error;
	}

	return fwd_p;

error:
	fwd_free (fwd_p);
	return NULL;
}

void fwd_init (void) {
	G_List *gl_tmp;

	if (!T.Fwd_list)
		return;

	for (gl_tmp = T.Fwd_list->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
        	struct sockaddr *sa_p;
        	int sa_p_len;
		Fwd *fwd_tmp;

		fwd_tmp = (Fwd *) (gl_tmp->list_data);
		fwd_tmp->went_down_at = 0;
		fwd_tmp->ticks = 0;

		sa_p = parse_and_alloc_addr (fwd_tmp->hostname, fwd_tmp->port, &sa_p_len);
		if (sa_p) {
			syslog (LOG_INFO, "Forwarder %s configured, port %d", fwd_tmp->hostname, fwd_tmp->port);
			memcpy (fwd_tmp->sa, sa_p, sa_p_len);
			fwd_tmp->sa_len = sa_p_len;
		} else {
			syslog (LOG_ERR, "Can't configure forwarder %s, port %d", fwd_tmp->hostname, fwd_tmp->port);
			if (fwd_tmp->sa)
	    			free (fwd_tmp->sa);
			fwd_tmp->sa = NULL;
			fwd_tmp->sa_len = 0;
		}
	}
}

int fwd_add (char *hostname, int port) {
	Fwd *fwd_p;

	syslog (LOG_DEBUG, "fwd_add(): start");

	if (!T.Fwd_list)
		return -1;

	fwd_p = fwd_alloc ();
	if (!fwd_p)
		return -1;

	strlcpy(fwd_p->hostname, hostname, MAX_DNAME);
	fwd_p->port = port;

	if (list_add_tail (T.Fwd_list, fwd_p) < 0) {
		fwd_free (fwd_p);
		return -1;
	}
	return 0;
}

/*
 * Selects nameserver to initially forward incoming requests to.
 */
void fwd_select (void) {
	char *fn = "fwd_select";
    	char astr[MAX_DNAME];
	G_List *list_tmp;
	Fwd *fwd_tmp;

	syslog (LOG_DEBUG, "%s: start()", fn);

	if (!T.current_fwd) {
	    /* No forwarder selected yet, just pick first valid one */
	    if (!T.Fwd_list)
		return;

	    T.current_fwd = T.Fwd_list->next;
	    fwd_tmp = (Fwd *)T.current_fwd->list_data;
	    while (fwd_tmp && !fwd_tmp->sa) {
	    	T.current_fwd = T.current_fwd->next;
	        fwd_tmp = (Fwd *)T.current_fwd->list_data;
	    }

	    if (!fwd_tmp || !fwd_tmp->sa) {
		/* we didn't find a valid forwarder at all */
		T.current_fwd = NULL;
	    	syslog (LOG_ERR, "No forwarder configured!");
		return;
	    }
	    syslog (LOG_DEBUG, "Use initial forwarder %s",
		    sprint_inet(fwd_tmp->sa, astr));
	} else if (T.current_fwd->prev->list_data) {
	    /* 
	     * We're not using the first nameserver listed, i.e.
	     * we are using a backup server. After a while we should
	     * try to go back to an earlier nameserver.
	     */
	    time_t waittime, downtime, current_time;

	    current_time = time(NULL);
	    list_tmp = T.current_fwd->prev;
	    fwd_tmp = (Fwd *)list_tmp->list_data;
	    while (fwd_tmp) {
		waittime = (current_time - fwd_tmp->went_down_at);
		downtime = (time_t) (T.retry_interval);

		if (fwd_tmp->sa && waittime > downtime) {
		    /* waited long enough, let's try again! */
		    syslog (LOG_NOTICE, "Enable forwarder %s again",
			    sprint_inet(fwd_tmp->sa, astr));
		    /* for the occasion, we mark it up again */
		    fwd_tmp->went_down_at = 0;
		    if (fwd_tmp->ticks > 0)
		    	fwd_tmp->ticks--;
		    T.current_fwd = list_tmp;
		    /* only one at a time, the rest will follow */
		    break;
		}
		list_tmp = list_tmp->prev;
	    	fwd_tmp = (Fwd *)list_tmp->list_data;
	    }
	}

	syslog (LOG_DEBUG, "Current forwarder %s",
		    sprint_inet(((Fwd *)T.current_fwd->list_data)->sa, astr));
	syslog (LOG_DEBUG, "%s: end()", fn);
	return;
}

/*
 * The fact that this routines gets called is a first hint that
 * the current forwarder/nameserver is down at this point in time.
 * Actually, it may just be slow, be overloaded, or the network may
 * be congested. In any way, from our point of view it is slow or
 * down and thus we may gain by trying a configured backup forwarder.
 * 
 * This routines marks the forwarder with the given address (if any)
 * with a `minus' point. If a forwarder has gathered `enough' minus points
 * it will be marked down, such that it will not be used for a while.
 *
 * Note that the current forwarder (T.current_fwd) is never marked down!
 */
void fwd_mark (struct sockaddr *sa, int up) {
	char *fn = "fwd_mark";
    	char astr[MAX_DNAME];
    	char bstr[MAX_DNAME];
	G_List *gl;
	Fwd *fwd;

	syslog (LOG_DEBUG, "%s: start()", fn);

	if (!T.Fwd_list || !T.current_fwd)
		return;

	fwd = NULL;
        for (gl = T.Fwd_list->next; gl->list_data; gl = gl->next) {
	        fwd = (Fwd *)gl->list_data;

		if (sa->sa_family != fwd->sa->sa_family)
			continue;

#ifdef USE_INET6
		if (sa->sa_family == AF_INET6) {
			struct sockaddr_in6 *sina, *sinb;

			sina = (struct sockaddr_in6 *) (fwd->sa);
			sinb = (struct sockaddr_in6 *) sa;
			if (IN6_ARE_ADDR_EQUAL(&sina->sin6_addr, &sinb->sin6_addr)
			    && sina->sin6_port == sinb->sin6_port) {
				fwd->ticks += up;
				break;
			}
		}
#endif
#ifdef USE_INET4
		if (sa->sa_family == AF_INET) {
			struct sockaddr_in *sina, *sinb;

			sina = (struct sockaddr_in *) (fwd->sa);
			sinb = (struct sockaddr_in *) sa;
			if (sina->sin_addr.s_addr == sinb->sin_addr.s_addr &&
			    sina->sin_port == sinb->sin_port) {
				fwd->ticks += up;
				break;
			}
		}
#endif
	}

	if (!fwd)
		return;

	if (fwd->ticks < 0)
		fwd->ticks = 0;

	if (gl->list_data)
		syslog (LOG_DEBUG, "Mark forwarder with %d: %s ", fwd->ticks,
		    sprint_inet(sa, astr));

	if (fwd->ticks < FORWARDER_DEATH_MARK)
		return;
	else
		fwd->went_down_at = time(NULL);

	if (((Fwd *)(T.current_fwd->list_data))->went_down_at) {
		G_List *new_fwd;
		Fwd *fwd_tmp;

		/*
		 * we marked current forwarder down, so
		 * select new next valid forwarder
		 */
		new_fwd = T.current_fwd->next;
		fwd_tmp = (Fwd *)new_fwd->list_data;
		while (fwd_tmp) {
			if (fwd_tmp->sa && !fwd_tmp->went_down_at) 
				break;
			new_fwd = new_fwd->next;
	        	fwd_tmp = (Fwd *)new_fwd->list_data;
		}

		if (!fwd_tmp || !fwd_tmp->sa) {
			/*
			 * we didn't find a next valid forwarder, game over!
			 * Note that we do not mark current forwarder down
			 * (the current one never is) nor do we change it.
			 *
			 * Actually, we mark all forwarders up again! No use
			 * discriminating between things that seem to behave
			 * the same ;)
			 */
			new_fwd = T.Fwd_list->next;
			fwd_tmp = (Fwd *)new_fwd->list_data;
			while (fwd_tmp) {
				/* mark 'em `up' again */
				fwd_tmp->ticks = 0;
				fwd_tmp->went_down_at = 0;
				new_fwd = new_fwd->next;
	        		fwd_tmp = (Fwd *)new_fwd->list_data;
			}
			return;
		}

		syslog (LOG_NOTICE, "Disabling forwarder %s (next %s)", 
			sprint_inet(((Fwd *)T.current_fwd->list_data)->sa, astr), 
			sprint_inet(fwd_tmp->sa, bstr));

		T.current_fwd = new_fwd;
	}
}

G_List *fwd_socketlist (void) {
	char *fn = "fwd_socketlist";
	G_List *socklist, *gl;
	Fwd *fwd;

	syslog (LOG_DEBUG, "%s: start()", fn);

	if (!T.Fwd_list || !T.current_fwd)
		return NULL;

	socklist = list_init();

	/*
	 * We cycle through all forwarders, starting with the `current' one.
	 * We skip all those that are currently marked down or without proper
	 * socket address.
	 */
        for (gl = T.current_fwd; gl->next != T.current_fwd; gl = gl->next) {
		if (!gl->list_data)
			continue;

	        fwd = (Fwd *)gl->list_data;
		if (fwd->sa && !fwd->went_down_at)  {
			struct sockaddr *sa;

			sa = malloc (sizeof(struct sockaddr_storage));
			if (!sa)
				return NULL;

			memcpy(sa, fwd->sa, SOCKADDR_SIZEOF(*fwd->sa));
			list_add_tail(socklist, sa);
		}
	}

	return socklist;
}
