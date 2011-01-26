/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/
/*
 * <$Id: queue.c,v 3.16 2002/12/11 16:39:50 dillema Exp $>
 */

#include "totd.h"

Q_Set *queue_create (void) {
        char *fn = "queue_create()";
	Q_Set *qs_tmp;

	qs_tmp = malloc (sizeof (Q_Set));
	if (!qs_tmp)
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
	else if (!(qs_tmp->head = qs_tmp->tail = malloc (sizeof (G_Queue)))) {
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
		free (qs_tmp);
	} else {
		qs_tmp->head->next = NULL;
		qs_tmp->head->p = NULL;
		/* SUCCESS */
		return qs_tmp;
	}

	/* FAILURE */
	return (NULL);
}

int enqueue (Q_Set *qs, void *p) {
        char *fn = "enqueue()";
	G_Queue *gq_tmp;

	gq_tmp = malloc (sizeof (G_Queue));
	if (!gq_tmp)
		syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
	else {
		gq_tmp->next = NULL;
		gq_tmp->p = NULL;

		qs->tail->p = p;
		qs->tail->next = gq_tmp;
		qs->tail = gq_tmp;

		/* SUCCESS */
		return 0;
	}
	/* FAILURE */
	return (-1);
}

void *dequeue (Q_Set *qs) {
	void *p;
	G_Queue *gq_tmp;

	if (!qs->head->next)
		return NULL;

	do {
		p = qs->head->p;
		gq_tmp = qs->head;
		qs->head = qs->head->next;
		free (gq_tmp);
	} while (!p && qs->head != qs->tail);

	return p;
}

void *queue_peek (Q_Set *qs) {

	if (!qs->head->next)
		return NULL;

	return qs->head->p;
}

void *queue_disable (G_Queue *gq) {
	void *p;

	p = gq->p;
	gq->p = NULL;
	return p;
}

void queue_destroy (Q_Set *qs, void (*freefnc) (void *)) {
	void *p;

	if (!qs)
		return;

	while ((p = dequeue (qs)))
		(*freefnc) (p);

	if (qs->head) {
		/* watchdog */
		free (qs->head);
	}
	free (qs);
}
