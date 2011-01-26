/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ne_io.c,v 3.64 2002/12/11 16:39:50 dillema Exp $>
 */

#include "totd.h"

static G_List *NI_head = NULL;	/* lists are to be allocated */
static G_List *NI_wildcard = NULL;	/* linked element of wildcard socket */
static G_List *NI_wildcard6 = NULL;	/* wildcard 6 */

static int net_ifc_cmp (struct ifconf *, struct ifconf *);
static Nia *nia_alloc (struct sockaddr *, int, int);
static void nia_list_destroy (G_List *);
static int nia_is_in_totd_iflist(char *);
static int net_get_ifaddrs (G_List *, int);
static int nia_set_wildsock (G_List *, int);

/*
 * nia_find_by_sock    : find Nia by socket id
 * nia_fds_set         : set all bits in the fd_set for all socks
 * nia_fds_isset       : check Nia available for fd_set
 *
 * net_init_socketlist
 * net_reinit_socketlist
 * net_bind_socketlist
 * net_delete_socketlist
 *
 * net_stream_socket: open stream socket, bind it and listen
 * net_mesg_socket: open dgram binded socket on a interface
 *
 * net_mesg_send  : send message from specific interface
 *
 */

static int nia_is_in_totd_iflist(char *ifname) {
	char **ifp;

	ifp = T.iflist;
	while (*ifp) {
		if (!strcmp("any", *ifp))
			return 1;
		if (!strcmp("all", *ifp))
			return 1;
		if (!strcmp(ifname, *ifp))
			return 1;
		ifp++;
	}
	return 0;
}

static int net_ifc_cmp (struct ifconf *ifc1, struct ifconf *ifc2) {
	struct ifreq *ifr_p1;
	struct ifreq *ifr_p2;
	u_char *cp1;
	u_char *cp2;

	if (ifc1->ifc_len != ifc2->ifc_len) {
		if (T.debug > 4) 
			syslog (LOG_DEBUG, "net_ifc_cmp(): lengths differ");
		return 1;
	}

	cp1 = ifc1->ifc_buf;
	cp2 = ifc2->ifc_buf;

	while (*cp1 && *cp2) {
		ifr_p1 = (struct ifreq *) cp1;
		ifr_p2 = (struct ifreq *) cp2;

		cp1 = cp1 + IFNAMSIZ + SOCKADDR_SIZEOF (ifr_p1->ifr_addr);
		cp2 = cp2 + IFNAMSIZ + SOCKADDR_SIZEOF (ifr_p2->ifr_addr);

		if (T.debug > 4) {
			char astr[MAX_DNAME], bstr[MAX_DNAME];

			sprint_inet(&ifr_p1->ifr_addr, astr);
			sprint_inet(&ifr_p2->ifr_addr, bstr);
			
			syslog (LOG_DEBUG, "net_ifc_cmp(): if1 %s, if2 %s, \
af1 %d, af2 %d, addr1 %s, addr2 %s", ifr_p1->ifr_name, ifr_p2->ifr_name, \
ifr_p1->ifr_addr.sa_family, ifr_p1->ifr_addr.sa_family, astr, bstr);
		}

#ifdef USE_INET4
		if (ifr_p1->ifr_addr.sa_family != AF_INET &&
		    ifr_p2->ifr_addr.sa_family != AF_INET)
#endif
#ifdef USE_INET6
			if (ifr_p1->ifr_addr.sa_family != AF_INET6 &&
		    	    ifr_p2->ifr_addr.sa_family != AF_INET6)
#endif
				continue; /* skip */

		/* comparison only AF_INET and AF_INET6 */
		if (ifr_p1->ifr_addr.sa_family != ifr_p2->ifr_addr.sa_family)
			return 1;

		if (strcmp (ifr_p1->ifr_name, ifr_p2->ifr_name))
			return 1;

		if (memcmp (&(ifr_p1->ifr_addr), &(ifr_p2->ifr_addr),
			    SOCKADDR_SIZEOF (ifr_p1->ifr_addr)))
			return 1;
	}

	return 0; /* equal! */
}

static Nia *nia_alloc (struct sockaddr *sa_p, int usock, int tsock) {
	Nia *ni;

	ni = (Nia *) malloc (sizeof (Nia));
	if (!ni)
		return NULL;

	if (sa_p) {
		ni->sa_p =
			(struct sockaddr *) malloc (SOCKADDR_SIZEOF (*sa_p));
		if (!ni->sa_p) {
			free (ni);
			return NULL;
		}
		memcpy (ni->sa_p, sa_p, SOCKADDR_SIZEOF (*sa_p));
	} else
		ni->sa_p = NULL;

	ni->udp_sock = usock;
	ni->tcp_sock = tsock;
	return ni;
}

static void nia_list_destroy (G_List *list_head) {

	list_destroy(list_head, nia_free_closev);
	return;
}

void nia_free_closev (void *ni) {
	nia_free((Nia *)ni, 1);
}

void nia_free (Nia *ni, int close_flag) {
	const char *fn = "nia_free()";

	if (ni->sa_p)
		free (ni->sa_p);

	if (ni->udp_sock >= 0 && close_flag) {
		if (T.debug > 4)
			syslog (LOG_DEBUG, "%s: socket close(fd = %d)",
				fn, ni->udp_sock);
		close (ni->udp_sock);
	}

	if (ni->tcp_sock >= 0 && close_flag) {
		if (T.debug > 4)
			syslog (LOG_DEBUG, "%s: socket close(fd = %d)",
				fn, ni->tcp_sock);
		close (ni->tcp_sock);
	}

	free (ni);
}

Nia *nia_copy (Nia *ni) {
	if (!ni)
		return NULL;

	return nia_alloc (ni->sa_p, ni->udp_sock, ni->tcp_sock);
}

Nia *nia_find_by_sock (int sock_id) {
	G_List *gl;
	Nia *ni;

	for (gl = NI_head->next; gl->list_data; gl = gl->next) {
		ni = (Nia *) gl->list_data;

		if (!ni->sa_p)
			continue;

		if (ni->udp_sock == sock_id)
			return ni;
		if (ni->tcp_sock == sock_id)
			return ni;
	}

	return NULL;
}

void nia_fds_set (fd_set *fds, int *max_fd) {
	const char *fn = "nia_fds_set()";
	G_List *gl;
	Nia *ni;

	if (!NI_head)
		return;

	for (gl = NI_head->next; gl->list_data; gl = gl->next) {
		ni = (Nia *) gl->list_data;

		if (!ni->sa_p)
			continue;

		if (ni->udp_sock >= 0) {
			if (T.debug > 4)
				syslog (LOG_DEBUG, "%s: FD_SET(%d)", fn,
					ni->udp_sock);
			FD_SET (ni->udp_sock, fds);
			if (max_fd)
				*max_fd = MAXNUM (*max_fd, ni->udp_sock);
		}

		if (ni->tcp_sock >= 0) {
			if (T.debug > 4)
				syslog (LOG_DEBUG, "%s: FD_SET(%d)", fn,
					ni->tcp_sock);
			FD_SET (ni->tcp_sock, fds);
			if (max_fd)
				*max_fd = MAXNUM (*max_fd, ni->tcp_sock);
		}
	}

	return;
}

int nia_fds_isset (fd_set *fds, int *sock) {
	const char *fn = "nia_fds_isset()";
	G_List *gl;
	Nia *ni;

	*sock = -1;

	if (!NI_head)
		return -1;

	for (gl = NI_head->next; gl->list_data; gl = gl->next) {
		ni = (Nia *) gl->list_data;

		if (!ni->sa_p)
			continue;

		if (ni->udp_sock >= 0 && FD_ISSET (ni->udp_sock, fds)) {
			if (T.debug > 4)
				syslog (LOG_DEBUG, "%s: %d FD_ISSET",
					fn, ni->udp_sock);
			FD_CLR (ni->udp_sock, fds);
			*sock = ni->udp_sock;
			return 0; /* UDP */
		}

		if (ni->tcp_sock >= 0 && FD_ISSET (ni->tcp_sock, fds)) {
			if (T.debug > 4)
				syslog (LOG_DEBUG, "%s: %d FD_ISSET",
					fn, ni->tcp_sock);
			FD_CLR (ni->tcp_sock, fds);
			*sock = ni->tcp_sock;
			return 1; /* TCP */
		}
	}

	return -1;
}

static int nia_set_wildsock (G_List *list_head, int port) {
	Nia *ni;

#ifdef USE_INET4
	if (!T.use_mapped) {
		struct sockaddr_in sin;

		memset (&sin, 0, sizeof (sin));
#ifdef HAVE_SA_LEN_FIELD
		sin.sin_len = sizeof (struct sockaddr_in);
#endif
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = INADDR_ANY;

		ni = nia_alloc((struct sockaddr *)&sin, -1, -1);
		if (!ni)
			return -1;

		if (list_add (list_head, ni) < 0) {
			nia_free (ni, 0);
			return -1;
		}
	
		NI_wildcard = list_head->next;
	}
#endif

#ifdef USE_INET6
	if (1) {
		struct sockaddr_in6 sin6;

		memset (&sin6, 0, sizeof (sin6));
#ifdef HAVE_SA_LEN_FIELD
		sin6.sin6_len = sizeof (struct sockaddr_in6);
#endif
		sin6.sin6_family = AF_INET6;
		sin6.sin6_port = htons(port);
		sin6.sin6_addr = in6addr_any;

		ni = nia_alloc((struct sockaddr *)&sin6, -1, -1);
		if (!ni)
			return -1;

		if (list_add (list_head, ni) < 0) {
			nia_free (ni, 0);
			return -1;
		}

		NI_wildcard6 = list_head->next;
	}
#endif

	/* SUCCESS */
	return 0;
}

int net_init_socketlist (int port) {

	NI_head = list_init();
	if (!NI_head)
		return -1;

	if (T.wildcard)
		nia_set_wildsock (NI_head, T.port);
	else {
		/* wildcard for sending requests only */
		nia_set_wildsock (NI_head, 0);
		if (net_get_ifaddrs (NI_head, port) != 1)
			return -1;
	}

	return 0;
}

int net_reinit_socketlist (int port, int force_update) {
	G_List *newlist;

	newlist = list_init();
	if (!newlist)
		return -1;

	if (T.wildcard) {
		if (force_update) {
			nia_list_destroy (NI_head);
			nia_set_wildsock (newlist, T.port);
			NI_head = newlist;
			net_bind_socketlist();
			return 1;
		} else {
			nia_list_destroy (newlist);
			return 0;
		}
	}

	switch (net_get_ifaddrs (newlist, port)) {
	case 0: /* no change */
		if (!force_update) {
			nia_list_destroy (newlist);
			return 0;
		}
		/* else fall through to next case */
	case 1: /* changed list */
		nia_list_destroy (NI_head);
		/* wildcard for sending requests only */
		nia_set_wildsock (newlist, 0);
		NI_head = newlist;
		net_bind_socketlist();
		return 1;
	default: /* error */
		nia_list_destroy (newlist);
		return -1;
	}
}

/* returns number of sockets opened successfully */
int net_bind_socketlist (void) {
	G_List *gl;
	Nia *ni;
	int s;

	if (!NI_head)
		return -1;

	s = 0; /* no socket opened yet */
	for (gl = NI_head->next; gl->list_data; gl = gl->next) {
		ni = (Nia *) gl->list_data;

		if (!ni->sa_p) 
			continue;

		if (net_mesg_socket (ni) >= 0)
			s++;
		if (net_stream_socket (ni) >= 0)
			s++;
	}

	return s;
}

int net_delete_socketlist (void) {

	/*  throw out old list if any */
	nia_list_destroy (NI_head);
	NI_head = NI_wildcard = NI_wildcard6 = NULL;

	return 0;
}

static int net_get_ifaddrs (G_List *list_head, int port) {
	const char *fn = "net_get_ifaddrs()";
	static struct ifconf ifc_old;
	struct ifconf ifc;
	int dummy_sock = -1, status;
	u_char buf[8192];
	u_char *cp;

	memset (buf, 0, sizeof (buf));
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;

#ifdef USE_INET6
	if (dummy_sock < 0)
		dummy_sock = socket (AF_INET6, SOCK_STREAM, 0);
#endif
#ifdef USE_INET4
	if (dummy_sock < 0)
		dummy_sock = socket (AF_INET, SOCK_STREAM, 0);
#endif
	if (dummy_sock < 0)
		return -1;

	/*
	 * check whether list of interface addresses has changed
	 */
	if (ioctl (dummy_sock, SIOCGIFCONF, (char *) &ifc) < 0) {
		close(dummy_sock);
		syslog (LOG_ERR, "%s: get iflist error: %m", fn);
		return -1;
	}
	close(dummy_sock);

	status = 1;
	if (ifc_old.ifc_buf) {
		status = net_ifc_cmp (&ifc_old, &ifc);
		syslog (LOG_DEBUG, "%s: checked interface data %d", fn, status);
	}

	/*
	 * Interface address list changed or we got called for
	 * for the first time
	 */

	cp = buf;
	while (*cp) {
		int valid_address = 0;
		char astr[MAX_DNAME];
		struct ifreq *ifr_p;

		ifr_p = (struct ifreq *) cp;
		sprint_inet(&ifr_p->ifr_addr, astr);

#ifdef USE_INET6
		if (T.ip6 && ifr_p->ifr_addr.sa_family == AF_INET6)
			if (nia_is_in_totd_iflist(ifr_p->ifr_name))
				valid_address = 1;
#endif
#ifdef USE_INET4
		if (T.ip4 && ifr_p->ifr_addr.sa_family == AF_INET)
			if (T.ip4 && nia_is_in_totd_iflist(ifr_p->ifr_name))
				valid_address = 1;
#endif

		if (valid_address) {
			Nia *ni;

			ni = nia_alloc (&ifr_p->ifr_addr, -1, -1);
			if (!ni)
				return -1;

#ifdef USE_INET6
			if (T.ip6 && ni->sa_p->sa_family == AF_INET6) {
				struct sockaddr_in6 *sa_p;

				sa_p = (struct sockaddr_in6 *) ni->sa_p;
				sa_p->sin6_port = htons (port);
			}
#endif

#ifdef USE_INET6
			if (T.ip4 && ni->sa_p->sa_family == AF_INET) {
				struct sockaddr_in *sa_p;

				sa_p = (struct sockaddr_in *) ni->sa_p;
				sa_p->sin_port = htons (port);
			}
#endif

			if (*astr && T.debug > 3)
				syslog (LOG_DEBUG, "Found address %s on if %s",
					astr, ifr_p->ifr_name);

			/* ok! add to the list */
			if (list_add (list_head, ni) < 0)
                        	return -1;

		} else if (*astr && T.debug > 3)
			syslog (LOG_DEBUG, "Ignoring address %s on if %s",
				astr, ifr_p->ifr_name);

		/* point to next address entry in list */
		cp = cp + IFNAMSIZ + SOCKADDR_SIZEOF (ifr_p->ifr_addr);
	}

	/*
	 * preserve old buffer
	 */

	if (ifc_old.ifc_buf)
		free (ifc_old.ifc_buf);

	ifc_old.ifc_buf = malloc (ifc.ifc_len);
	if (ifc_old.ifc_buf) {
		memcpy (ifc_old.ifc_buf, buf, ifc.ifc_len);
		ifc_old.ifc_len = ifc.ifc_len;
	}

	return status;
}

int net_mesg_send (Nia *ni, u_char *mesg, int mesg_len,
		   struct sockaddr *sa_p) {
	const char *fn = "net_mesg_send()";

	if (!ni) {
#ifdef USE_INET4
		if (sa_p->sa_family == AF_INET)
			ni = (Nia *) NI_wildcard->list_data;
#endif
#ifdef USE_INET6
		if (sa_p->sa_family == AF_INET6)
			ni = (Nia *) NI_wildcard6->list_data;
#endif
	}

	if (!ni || ni->udp_sock < 0) {
		syslog (LOG_WARNING, "%s: no socket to send message over", fn);
		return -1;
	}

#ifdef USE_INET4
	if (ni->sa_p->sa_family == AF_INET && sa_p->sa_family == AF_INET)
		return sendto (ni->udp_sock, mesg, mesg_len, 0, sa_p,
			       SOCKADDR_SIZEOF (*sa_p));
#endif

#ifdef USE_INET6
	if (ni->sa_p->sa_family == AF_INET6 && sa_p->sa_family == AF_INET6)
		return sendto (ni->udp_sock, mesg, mesg_len, 0, sa_p,
			       SOCKADDR_SIZEOF (*sa_p));
#endif

	return -1;
}

int net_mesg_socket (Nia *ni) {
	const char *fn = "net_mesg_socket()";
	char astr[MAX_DNAME];
	const int on = 1;
	int sock;

	ni->udp_sock = -1;

	sock = socket (ni->sa_p->sa_family, SOCK_DGRAM, 0);
	if (sock < 0) {
		syslog (LOG_ERR, "%s: socket open failed: %m", fn);
		return -1;
	}
	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: socket open(fd = %d)", fn, sock);

	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
			(char *) &on, sizeof (on)))
		syslog (LOG_WARNING, "setsockopt: %m");


	if (bind (sock, ni->sa_p, SOCKADDR_SIZEOF (*ni->sa_p)) < 0) {
		syslog (LOG_ERR, "Can not bind datagram socket: %m");
		close (sock);
		return -1;
	}

	/* if no port set, then it is for outgoing messages only */
	if (((struct sockaddr_in *)ni->sa_p)->sin_port) {
		sprint_inet(ni->sa_p, astr);
		syslog (LOG_NOTICE, "Listening on %s for UDP", astr);
	}

#ifdef USE_INET6
#ifdef IPV6_USE_MIN_MTU
	if (ni->sa_p->sa_family == AF_INET6) {
		const int on = 1;

		/* ignore error */
		(void)setsockopt(sock, IPPROTO_IPV6,
			IPV6_USE_MIN_MTU, &on, sizeof(on));
	}
#endif
#endif

	ni->udp_sock = sock;
	return sock;
}

int net_stream_socket (Nia *ni) {
	const char *fn = "net_stream_socket()";
	char astr[MAX_DNAME];
	const int on = 1;
	int sock;

	ni->tcp_sock = -1;

	/* if no port set, do nothing for TCP */
	if (!((struct sockaddr_in *)ni->sa_p)->sin_port)
		return -1;

	sock = socket (ni->sa_p->sa_family, SOCK_STREAM, 0);
	if (sock < 0) {
		syslog (LOG_ERR, "%s: socket open failed: %m", fn);
		return -1;
	}
	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: socket open(fd = %d)", fn, sock);

	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
			(char *) &on, sizeof (on)))
		syslog (LOG_WARNING, "setsockopt: %m");

	if (bind (sock, ni->sa_p, SOCKADDR_SIZEOF (*ni->sa_p)) != 0) {
		syslog (LOG_ERR, "Can't bind TCP socket: %m");
		close (sock);
		return -1;
	}

	if (ioctl (sock, FIONBIO, (char *) &on) < 0) {
		syslog (LOG_ERR, "Can't ioctl on service socket: %m");
		return -1;
	}

	if (listen (sock, 5) != 0) {
		syslog (LOG_ERR, "Listen failed: %m");
		close (sock);
		return -1;
	}

	sprint_inet(ni->sa_p, astr);
	syslog (LOG_NOTICE, "Listening on %s for TCP", astr);

	ni->tcp_sock = sock;
	return sock;
}
