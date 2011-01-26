/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: ev_dup.c,v 3.18 2002/03/06 14:56:02 dillema Exp $>
 */

#include "totd.h"

/* Event hash table for duplicate detection */
static struct {
	union {
		u_char d[16];
		struct sockaddr sa;
	}   data;
	uint16_t data_len;
	uint16_t id;
	time_t expire;
}   Ev_dup_table[EV_DUP_TABLE_SIZE];

/* returns index entry in table for argument combo */
static int ev_dup_hashval (struct sockaddr *sa, uint16_t sa_len, uint16_t id) {
	int hashval;
	u_char *cp;
	u_char *cp_tail;

	hashval = 0;
	for (cp = (u_char *) sa, cp_tail = cp + sa_len; cp < cp_tail; cp++)
		hashval += *cp;

	hashval += id;
	return hashval % EV_DUP_TABLE_SIZE;
}

/* 
 * Initializes Event Duplicate Table
 */
void ev_dup_init (void) {
	int i;

	for (i = 0; i < EV_DUP_TABLE_SIZE; i++) {
		memset (Ev_dup_table[i].data.d, 0, EV_DUP_CMP_LEN);
		Ev_dup_table[i].data_len = 0;
		Ev_dup_table[i].id = 0;
	}
	return;
}

/* 
 * Returns -1 when it finds a fresh duplicate, zero otherwise
 */
int ev_dup (struct sockaddr *sa, uint16_t sa_len, uint16_t id) {
	int index;
	time_t now;

	index = ev_dup_hashval (sa, sa_len, id);
	if (T.debug > 2) {
		syslog (LOG_DEBUG, "ev_dup(): index = %d", index);
	}
	now = time (NULL);
	/* use minimum of sa_len and EV_DUP_CMP_LEN */
	sa_len = sa_len > EV_DUP_CMP_LEN ? EV_DUP_CMP_LEN : sa_len;

	if (Ev_dup_table[index].id == id &&
	    Ev_dup_table[index].data_len == sa_len &&
	    !memcmp (&(Ev_dup_table[index].data.d), (u_char *) sa, sa_len) &&
	    now < Ev_dup_table[index].expire) {
		syslog (LOG_DEBUG, "ev_dup(): duplicate detected");
		return -1;
	} else {
		/* (over-)write table entry */
		Ev_dup_table[index].id = id;
		Ev_dup_table[index].data_len = sa_len;
		memcpy (Ev_dup_table[index].data.d, (u_char *) sa, sa_len);
		Ev_dup_table[index].expire = time (NULL) + EVENT_LIFETIME;
		return 0;
	}
}
