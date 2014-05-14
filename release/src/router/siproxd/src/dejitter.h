/*
    Copyright (C) 2006-2008  Hans Carlos Hofmann <labtop-carlos@hchs.de>,
                             Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.

    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warrantry of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#ifdef GPL
#define USE_DEJITTER

typedef struct {
   void *next;				/* next free or next que element */
   int socked;				/* socket number */
   size_t message_len;			/* length of message */
   int flags;				/* flags */
   struct sockaddr_in dst_addr;		/* where shall i send */
   struct timeval transm_time;		/* when shall i send */
   rtp_proxytable_t *errret;		/* deliver error status */
   rtp_buff_t rtp_buff;			/* Data storage */
} rtp_delayed_message;


/* dejitter */
void dejitter_init(void);
void dejitter_delayedsendto(int s, const void *msg, size_t len, int flags,
                            const struct sockaddr_in *to,
                            const struct timeval *tv,
                            const struct timeval *current_tv,
                            rtp_proxytable_t *errret, int nolock);
void dejitter_cancel(rtp_proxytable_t *dropentry);
void dejitter_flush(struct timeval *current_tv, int nolock);
int  dejitter_delay_of_next_tx(struct timeval *tv, struct timeval *current_tv);
void dejitter_init_time(timecontrol_t *tc, int dejitter);
void dejitter_calc_tx_time(rtp_buff_t *rtp_buff, timecontrol_t *tc,
                           struct timeval *input_tv,
                           struct timeval *ttv);

#endif
