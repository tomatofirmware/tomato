/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: list.c,v 3.19 2002/03/04 12:34:10 dillema Exp $>
 */

#include "totd.h"

G_List *list_init (void) {
        const char *fn = "list_init()";
	G_List *gl_new = NULL;

	gl_new = malloc (sizeof (G_List));
	if (gl_new) {
		gl_new->next = gl_new;
		gl_new->prev = gl_new;
		gl_new->list_data = NULL;
	}

	if (T.debug > 4)
		syslog (LOG_DEBUG, "%s: new list: %p", fn, (void *)gl_new);

	return gl_new;
}

int list_add (G_List *gl_head, void *list_data) {
	G_List *gl_new;

	gl_new = malloc (sizeof (G_List));
	if (gl_new) {
		gl_new->list_data = list_data;
		gl_new->next = gl_head->next;
		gl_new->prev = gl_head;
		gl_head->next->prev = gl_new;
		gl_head->next = gl_new;
		/* SUCCESS */
		return 0;
	}

	/* FAILURE */
	return -1;
}

int list_add_tail (G_List *gl_head, void *list_data) {
	G_List *gl_new;

	gl_new = malloc (sizeof (G_List));
	if (gl_new) {
		gl_new->list_data = list_data;
		gl_new->next = gl_head;
		gl_new->prev = gl_head->prev;
		gl_head->prev->next = gl_new;
		gl_head->prev = gl_new;
		/* SUCCESS */
		return 0;
	}

	/* FAILURE */
	return -1;
}

G_List *list_search (G_List *list_head, void *data,
		     int (*fcmp) (void *, void *)) {
	G_List *gl;

	list_head->list_data = data;
	for (gl = list_head->next; (*fcmp)(data, gl->list_data); gl = gl->next);

	list_head->list_data = NULL;
	if (gl == list_head)
		return NULL;
	else 
		return gl;
}

void *list_delete (G_List *gl) {
	void *p;

	gl->next->prev = gl->prev;
	gl->prev->next = gl->next;

	p = gl->list_data;
	free (gl);

	return p;
}

void list_destroy (G_List *gl, void (*freefnc) (void *)) {

	G_List *gl_tmp;
	G_List *gl_next;

	if (!gl)
		return;

	for (gl_tmp = gl->next; gl_tmp != gl; gl_tmp = gl_next) {
		gl_next = gl_tmp->next;
		if (gl_tmp->list_data)
			(*freefnc) (gl_tmp->list_data);

		free (gl_tmp);
	}

	/* head record */
	free (gl);

	return;
}


G_List *list_dup (G_List *gl, void (freefunc) (void *),
		  void *(dupfunc) (void *)) {
	G_List *gl_new;
	G_List *gl_tmp;
	void *p;

	gl_new = list_init ();
	if (!gl_new)
		return NULL;

	gl->list_data = NULL;
	for (gl_tmp = gl->next; gl_tmp->list_data; gl_tmp = gl_tmp->next) {
		p = (*dupfunc) (gl_tmp->list_data);
		if (!p) {
			list_destroy (gl_new, freefunc);
			return NULL;
		}
		if (list_add_tail (gl_new, p) < 0) {
			list_destroy (gl_new, freefunc);
			return NULL;
		}
	}

	return gl_new;
}

void list_cat (G_List *list, G_List *toadd) {

	toadd->list_data = NULL;
	if (toadd->next->list_data) {
		list->prev->next = toadd->next;
		toadd->next->prev = list->prev;
		list->prev = toadd->prev;
		toadd->prev->next = list;
		toadd->prev = toadd;
		toadd->next = toadd;
	}
}
