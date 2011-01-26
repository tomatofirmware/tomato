/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ev_tcp.c,v 3.7 2002/03/04 12:34:10 dillema Exp $>
 */

#include "totd.h"

/* TCP event lists */
static G_List *ConnL_head;
static G_List *ETOL_head;

/*
 * TCP service handling
 */

int ev_tcp_srv_accept (int srv_sock) {
	struct sockaddr_storage peer;
	unsigned int peer_len;
	int conn_sock;

	if (T.debug > 2)
		syslog (LOG_DEBUG, "ev_tcp_srv_accept(): sock = %d", srv_sock);

	peer_len = sizeof (struct sockaddr_storage);
	conn_sock = accept (srv_sock, (struct sockaddr *)&peer, &peer_len);
	if (conn_sock < 0) {
		syslog (LOG_ERR, "accept: %m");
		return -1;
	}

	return tcp_response_start (conn_sock, (struct sockaddr *)&peer);
}

/*
 * Generic TCP event handling routines
 */

int ev_tcp_common_eq (void *arg1, void *arg2) {
	Ev_TCP_Common_Data *etcd_arg1;
	Ev_TCP_Common_Data *etcd_arg2;

	etcd_arg1 = arg1;
	etcd_arg2 = arg2;

	if (etcd_arg1->sock == etcd_arg2->sock)
		return 0;
	else
		return 1;
}

G_List *ev_tcp_common_init (void) {
	G_List *gl;

	gl = list_init ();
	if (!gl) 
		return NULL;

	gl->list_data = NULL;
	return gl;
}

int ev_tcp_common_register (G_List *gl, int sock, Context *cptr) {
        const char *fn = "ev_tcp_common_register()";
	Ev_TCP_Common_Data *etcd_new;

	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: sock = %d", fn, sock);

	etcd_new = malloc (sizeof (Ev_TCP_Common_Data));
	if (!etcd_new)
		return -1;

	etcd_new->sock = sock;
	etcd_new->cptr = cptr;

	if (list_add (gl, etcd_new) < 0)
		return -1;

	return 0;
}

int ev_tcp_common_remove (G_List *gl, int sock) {
        const char *fn = "ev_tcp_common_remove()";
	Ev_TCP_Common_Data etcd_tmp;
	Ev_TCP_Common_Data *etcd_p;
	G_List *gl_tmp;

	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: sock = %d", fn, sock);

	etcd_tmp.sock = sock;
	etcd_tmp.cptr = NULL;

	gl_tmp = list_search (gl, (void *) &etcd_tmp, ev_tcp_common_eq);
	if (!gl_tmp)
		return -1;

	etcd_p = (Ev_TCP_Common_Data *) list_delete (gl_tmp);
	free (etcd_p);

	return 0;
}

int ev_tcp_common_fds (G_List *gl, fd_set *fds) {
	Ev_TCP_Common_Data *etcd_p;
	G_List *gl_tmp;
	int maxid = 0;

	gl->list_data = NULL;
	for (gl_tmp = gl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		etcd_p = ((Ev_TCP_Common_Data *) (gl_tmp->list_data));
		if (T.debug > 2)
			syslog (LOG_DEBUG, "ev_tcp_common_fds: FD_SET(%d)",
				etcd_p->sock);
		FD_SET (etcd_p->sock, fds);
		maxid = MAXNUM (etcd_p->sock, maxid);
	}

	return maxid;
}

int ev_tcp_common_fd_check (G_List *gl, fd_set *fds) {
	G_List *gl_tmp;
	Context *cont;
	Ev_TCP_Common_Data *etcd_p;
	int s_tmp;

	for (gl_tmp = gl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		etcd_p = (Ev_TCP_Common_Data *) (gl_tmp->list_data);
		s_tmp = etcd_p->sock;

		if (FD_ISSET (s_tmp, fds)) {
			FD_CLR (s_tmp, fds);

			/*
			 * etcd_p MAY be released within context, so keep
			 * context alone
			 */
			cont = etcd_p->cptr;

			/* resume context */
			return cont->process (cont);
		}
	}
	return 0;
}

/*
 * TCP incoming connection event list
 */

int ev_tcp_conn_in_init (void) {
	if (ConnL_head)
		return -1;

	ConnL_head = ev_tcp_common_init ();
	if (!ConnL_head)
		return -1;
	return 0;
}

void ev_tcp_conn_in_finish (void) {
	list_destroy (ConnL_head, free);
}

int ev_tcp_conn_in_register (int sock, Context *cptr) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_conn_in_register(): start");
	return ev_tcp_common_register (ConnL_head, sock, cptr);
}

int ev_tcp_conn_in_remove (int sock) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_conn_in_remove(): start");
	return ev_tcp_common_remove (ConnL_head, sock);
}

int ev_tcp_conn_in_fds (fd_set *fds) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_conn_in_fds(): start");
	return ev_tcp_common_fds (ConnL_head, fds);
}

int ev_tcp_conn_in_fd_check (fd_set *fds) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_conn_in_fd_check(): start");
	return ev_tcp_common_fd_check (ConnL_head, fds);
}

/*
 * TCP outgoing connection event list
 */

int ev_tcp_out_init (void) {

	if (ETOL_head)
		return -1;

	ETOL_head = ev_tcp_common_init ();
	if (!ETOL_head)
		return -1;

	return 0;
}

void ev_tcp_out_finish (void) {
	list_destroy (ETOL_head, free);
}

int ev_tcp_out_register (int sock, Context * cptr) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_out_register(): start");
	return ev_tcp_common_register (ETOL_head, sock, cptr);
}

int ev_tcp_out_remove (int sock) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_out_remove(): start");
	return ev_tcp_common_remove (ETOL_head, sock);
}

int ev_tcp_out_fds (fd_set * fds) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_out_fds(): start");
	return ev_tcp_common_fds (ETOL_head, fds);
}

int ev_tcp_out_fd_check (fd_set * fds) {
	if (T.debug > 3)
		syslog (LOG_DEBUG, "ev_tcp_out_fd_check(): start");
	return ev_tcp_common_fd_check (ETOL_head, fds);
}

