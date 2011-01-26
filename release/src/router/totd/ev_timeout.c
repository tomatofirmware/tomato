/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ev_timeout.c,v 3.27 2002/12/11 16:39:49 dillema Exp $>
 */

#include "totd.h"

static Q_Set *ev_to_queue;

int ev_to_init (void) {

	if (ev_to_queue)
		return -1;

	ev_to_queue = queue_create ();
	if (!ev_to_queue)
		return -1;

	return 0;
}

int ev_to_register (Ev_TO_Data *td) {
	char *fn = "ev_to_register()";
	G_Queue *gq_new, *gq_tmp, *gq_prev;

	gq_new = malloc (sizeof (G_Queue));
	if (!gq_new)
        	syslog (LOG_ERR, "%s: Cannot allocate memory", fn);
	else {
		gq_prev = NULL;
		/* find place insertion sort style */
		for (gq_tmp = ev_to_queue->head; gq_tmp->next && 
		     ((Ev_TO_Data *) (gq_tmp->p))->at < td->at; 
		     gq_tmp = gq_tmp->next)
			gq_prev = gq_tmp;	/* just skip */

		if (!gq_prev) {
			if (T.debug > 3)
				syslog (LOG_DEBUG, "%s: add event to head", fn);
			gq_new->p = (void *) td;
			gq_new->next = ev_to_queue->head;
			ev_to_queue->head = gq_new;
		} else {
			while (gq_tmp->p &&
			       (((Ev_TO_Data *) (gq_tmp->p))->at == td->at)) {
				gq_prev = gq_tmp;
				gq_tmp = gq_tmp->next;
			}

			if (!gq_tmp->p) {
				if (T.debug > 3)
					syslog (LOG_DEBUG, "%s: add to tail",
						fn);
				ev_to_queue->tail->p = (void *) td;
				ev_to_queue->tail->next = gq_new;
				gq_new->p = NULL;
				gq_new->next = NULL;
				ev_to_queue->tail = gq_new;
			} else {
				if (T.debug > 3)
					syslog (LOG_DEBUG, "%s: add to middle",
						fn);
				gq_new->p = (void *) td;
				gq_prev->next = gq_new;
				gq_new->next = gq_tmp;
			}
		}
		/* SUCCESS */
		return 0;
	}

	/* FAILURE */
	return -1;
}

time_t ev_timeout_process (void) {
	char *fn = "ev_timeout_process()";
	Ev_TO_Data *td_tmp;

	while ((td_tmp = (Ev_TO_Data *) queue_peek (ev_to_queue))
	       && (td_tmp->at <= time (NULL))) {
		/* process the event */
		td_tmp = dequeue (ev_to_queue);
		/* call timeout handler, if any */
		if (td_tmp->handler) {
			if (T.debug > 3)
				syslog (LOG_DEBUG, "%s: call timeout handler",
					fn);
			(td_tmp->handler) (td_tmp);
		} else
			if (T.debug > 3)
				syslog (LOG_DEBUG, "%s: cancel timeout event",
					fn);
		ev_to_data_free (td_tmp);
	}

	td_tmp = (Ev_TO_Data *) queue_peek (ev_to_queue);
	if (td_tmp)
		return td_tmp->at;	/* next event scheduled time */
	else
		return 0;	/* no event scheduled */
}

void ev_to_data_free (Ev_TO_Data *td) {
	char *fn = "ev_to_data_free()";

	switch (td->type) {
	case EV_TIMEOUT_NULL:
		if (td->data.p)
			free (td->data.p);
		break;
	case EV_TIMEOUT_CONTEXT:
		/*
		 * don't free td->data.cont because transaction itself will
		 * free its context
		 */
		break;
	default:
		syslog (LOG_CRIT, "%s: unknown type for timeout event", fn);
		break;
	}

	free (td);
}

void ev_to_data_free_v (void *td_v) {
	ev_to_data_free ((Ev_TO_Data *) td_v);
}

void ev_to_finish (void) {
	queue_destroy (ev_to_queue, ev_to_data_free_v);
}

void ev_to_flush (int type) {
	G_Queue *gqp;
	Ev_TO_Data *etdp;

	/* check one by one */
	for (gqp = ev_to_queue->head; gqp->next; gqp = gqp->next) {
		if (gqp->p) {
			etdp = (Ev_TO_Data *) (gqp->p);
			if (etdp->type == type) {
				if (etdp->handler)
					(etdp->handler) (etdp);
				etdp->handler = NULL;	/* disable the queue */
			}
		}
	}
	return;
}

int ev_to_register_ifcheck (void) {
	char *fn = "ev_to_register_ifcheck()";
	Ev_TO_Data *td_new;

	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: start", fn);

	if (!ev_to_queue)
		return -1;

	td_new = malloc (sizeof (Ev_TO_Data));
	if (!td_new) {
		syslog (LOG_ERR, "%s: can't allocate new event", fn);
		return -1;
	}
	td_new->at = time (NULL) + IF_CHECK_INTERVAL;
	td_new->handler = ev_to_handler_ifcheck;
	td_new->type = EV_TIMEOUT_NULL;
	td_new->data.p = NULL;

	ev_to_register (td_new);
	return 0;
}

int ev_to_handler_ifcheck (Ev_TO_Data *td) {
	char *fn = "ev_to_handler_ifcheck()";

	if (T.debug > 2)
		syslog (LOG_DEBUG, "%s: start", fn);

	if (td) {
		switch (net_reinit_socketlist (T.port, 0)) {
		case 0:
			/* no change */
			syslog (LOG_DEBUG, "%s: Socket List still the same", fn);
			break;
		case 1:
			/* change, bound new sockets */
			syslog (LOG_DEBUG, "%s: Socket List updated", fn);
			break;
		default:
			syslog (LOG_ERR, "%s: Can't get new socket list!", fn);
		}
	}

	/* (re-) register my event */
	return (ev_to_register_ifcheck());
}
