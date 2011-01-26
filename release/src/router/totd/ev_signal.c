/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ev_signal.c,v 3.20 2002/03/04 12:34:10 dillema Exp $>
 */

#include "totd.h"

static void ev_signal_initiator_generic (int sig);
static void ev_handler_sigusr1 (void);
static void ev_handler_sigusr2 (void);
static void ev_handler_cleanup (void);

/* event initiator (== signal handler) matrix */
const struct {
	int signal_id;		/* signal id */
	void (*initiator) (int);/* event initiator */
	void (*handler) (void);	/* event handler */
}   Ev_matrix[] = {
	{
		SIGINT, ev_signal_initiator_generic, ev_handler_cleanup
	},
	{
		SIGHUP, ev_signal_initiator_generic, ev_handler_cleanup
	},
	{
		SIGTERM, ev_signal_initiator_generic, ev_handler_cleanup
	},
	{
		SIGUSR1, ev_signal_initiator_generic, ev_handler_sigusr1
	},
	{
		SIGUSR2, ev_signal_initiator_generic, ev_handler_sigusr2
	},
	{
		-1, NULL, NULL
	}			/* <- terminator -- don't forget */
};

/* signal event queue */
static Q_Set *Ev_signal_queue = NULL;

/*
 * (private) generic event initiator(== signal handler)
 */
static void ev_signal_initiator_generic (int sig) {
	int i;
	Ev_Sig_Data *sd_tmp;

	if (T.debug > 2) {
		syslog (LOG_DEBUG, "ev_signal_initiator_generic(): start.");
	}

	if (!Ev_signal_queue) {
		syslog (LOG_ERR, "ev_signal_initiator_generic(): no queue to process.");
		return;
	}
	for (i = 0; Ev_matrix[i].signal_id > 0 && Ev_matrix[i].signal_id != sig; i++);

	sd_tmp = malloc (sizeof (Ev_Sig_Data));
	if (!sd_tmp) {
		syslog (LOG_WARNING, "ev_signal_initiator_generic(): memory exhausted");
		return;
	}
	sd_tmp->handler = Ev_matrix[i].handler;

	if (enqueue (Ev_signal_queue, (void *) sd_tmp) < 0)
		syslog (LOG_WARNING, "ev_signal_initiator_generic(): signal queue full?");

	return;
}

/*
 * event handlers
 */
void ev_handler_sigusr1 (void) {
	syslog (LOG_INFO, "ev_handler_sigusr1(): processing SIGUSR1");
}

void ev_handler_sigusr2 (void) {
	syslog (LOG_INFO, "ev_handler_sigusr2(): processing SIGUSR2");
}

void ev_handler_cleanup (void) {
	totd_exit (0);
}

/* == end of private section == */

/*
 * event initializer
 */
int ev_signal_init (void) {
	int i;

	/* set event initiator */
	for (i = 0; Ev_matrix[i].signal_id > 0; i++)
		signal (Ev_matrix[i].signal_id, Ev_matrix[i].initiator);

	/* initialize queue */
	Ev_signal_queue = queue_create ();
	if (!Ev_signal_queue)
		return -1;

	return 0;
}

/*
 * event dequeue/process
 */
void ev_signal_process (void) {
	Ev_Sig_Data *sd_tmp;

	if (Ev_signal_queue)
		while ((sd_tmp = (Ev_Sig_Data *) dequeue (Ev_signal_queue))) {
			(sd_tmp->handler) ();	/* process event */
			free (sd_tmp);
		}
	else
		syslog (LOG_ERR, "ev_signal_process(): no queue to process.");
}

/*
 * finish signal routine
 */
void ev_signal_finish (void) {
	/* don't process signal. It may cause event loop */
	queue_destroy (Ev_signal_queue, free);
	Ev_signal_queue = NULL;
}
