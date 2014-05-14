/*
    Copyright (C) 2006-2011  Hans Carlos Hofmann <labtop-carlos@hchs.de>,
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
#include "config.h"

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <osipparser2/osip_parser.h>
#include "siproxd.h"
#include "rtpproxy.h"
#include "log.h"

#include "dejitter.h"

/*!!Roadkill - missing time struct in compilation*/
#include <sys/time.h>

static char const ident[]="$Id: dejitter.c 469 2011-01-09 14:44:23Z hb9xar $";

#ifdef GPL

/*
 * static forward declarations
 */
static void   add_time_values(const struct timeval *a,
                              const struct timeval *b, struct timeval *r);
static void   sub_time_values(const struct timeval *a, const struct timeval *b,
                              struct timeval *r);
static int    cmp_time_values(const struct timeval *a, const struct timeval *b);
static double make_double_time(const struct timeval *tv);
static void   send_top_of_que(int nolock);
static void   split_double_time(double d, struct timeval *tv);
static int    fetch_missalign_long_network_oder(char *where);


/*
 * RTP buffers for dejitter
 */

/*
 * table to buffer date for dejitter function
 */
#define NUMBER_OF_BUFFER (10*RTPPROXY_SIZE)
static rtp_delayed_message rtp_buffer_area[NUMBER_OF_BUFFER];

static rtp_delayed_message *free_memory;
static rtp_delayed_message *msg_que;

static struct timeval minstep;



/*
 * Initialize RTP dejitter
 */
void dejitter_init(void) {
   int i;
   rtp_delayed_message *m;

   memset (&rtp_buffer_area, 0, sizeof(rtp_buffer_area));
   free_memory = NULL;
   msg_que = NULL;
   for (i=0,m=&rtp_buffer_area[0];i<NUMBER_OF_BUFFER;i++,m++) {
      m->next = free_memory;
      free_memory = m;
   }
}

/*
 * Delayed send
 */
void dejitter_delayedsendto(int s, const void *msg, size_t len, int flags,
                            const struct sockaddr_in *to,
                            const struct timeval *tv,
                            const struct timeval *current_tv,
                            rtp_proxytable_t *errret, int nolock) {
   rtp_delayed_message *m;
   rtp_delayed_message *linkin;

   if (!free_memory) send_top_of_que(nolock);

   m = free_memory;

   m->socked = s;
   memcpy(&(m->rtp_buff), msg, m->message_len = len);
   m->flags = flags;
   m->dst_addr = *to;
   m->transm_time = *tv;
   m->errret = errret;

   free_memory = m->next;

   if (cmp_time_values(current_tv,tv) >= 0) {
      m->next = msg_que;
      msg_que = m;
      send_top_of_que(nolock);
   } else {
      linkin = msg_que;
      while ((linkin != NULL) &&
             (cmp_time_values(&(linkin->transm_time),tv) < 0)) {
         linkin = linkin->next;
      }
      m->next = linkin;
      linkin = m;
   }
}

/*
 * Cancel a message
 */
void dejitter_cancel(rtp_proxytable_t *dropentry) {
   rtp_delayed_message *linkout;
   rtp_delayed_message *m;

   linkout = msg_que;

   while (linkout != NULL) {
      if (linkout->errret == dropentry) {
         m = linkout;
         linkout = m->next;
         m->next = free_memory;
         free_memory = m;
      } else {
         linkout = linkout->next;
      }
   }
}

/*
 * Flush buffers
 */
void dejitter_flush(struct timeval *current_tv, int nolock) {
   struct timezone tz;

   while (msg_que &&
          (cmp_time_values(&(msg_que->transm_time),current_tv)<=0)) {
      send_top_of_que(nolock);
      gettimeofday(current_tv,&tz);
   }
}

/*
 * Delay of next transmission
 */
int dejitter_delay_of_next_tx(struct timeval *tv,struct timeval *current_tv) {
   struct timezone tz ;

   if (msg_que) {
      gettimeofday(current_tv,&tz);
      sub_time_values(&(msg_que->transm_time),current_tv,tv);
      if (cmp_time_values(tv,&minstep)<=0) {
         *tv = minstep ;
      }
      return -1;
   }
   return 0;
}

/*
 * Initialize calculation of transmit the frame
 */
void dejitter_init_time(timecontrol_t *tc, int dejitter) {
   struct timezone tz;

   minstep.tv_sec = 0;
   minstep.tv_usec = 6000;
   memset(tc, 0, sizeof(*tc));
   if (dejitter>0) {
      gettimeofday(&(tc->starttime),&tz);

      tc->dejitter = dejitter;
      tc->dejitter_d = dejitter;
      split_double_time(tc->dejitter_d, &(tc->dejitter_tv));
   }
}

/*
 * Calculate transmit time
 */
void dejitter_calc_tx_time(rtp_buff_t *rtp_buff, timecontrol_t *tc,
                           struct timeval *input_tv,
                           struct timeval *ttv) {
   int    packet_time_code;
   double currenttime;
   double calculatedtime = 0;
   double calculatedtime2 = 0;
   struct timeval input_r_tv;
   struct timeval output_r_tv;

   if (!tc || !tc->dejitter) {
      *ttv = *input_tv;
      return;
   }


   /* I hate this computer language ... :-/ quite confuse ! look modula */
   packet_time_code = fetch_missalign_long_network_oder(&((*rtp_buff)[4]));

/*&&&& beware, it seems that when sending RTP events (payload type
telephone-event) the timestamp does not increment and stays the same.
The sequence number however DOES increment. This could lead to confusion when
transmitting RTP events (like DTMF). How can we handle this? Check for RTP event
and then do an "educated guess" for the to-be timestamp?
*/
   if (tc->calccount == 0) {
      DEBUGC(DBCLASS_RTP, "initialise time calculatin");
      tc->starttime = *input_tv;
      tc->time_code_a = packet_time_code;
   }

   sub_time_values(input_tv,&(tc->starttime),&input_r_tv);

   calculatedtime = currenttime = make_double_time(&input_r_tv);
   if (tc->calccount < 10) {
      DEBUGC(DBCLASS_RTP, "initial data stage 1 %f usec", currenttime);
      tc->received_a = currenttime / (packet_time_code - tc->time_code_a);
   } else if (tc->calccount < 20) {
      tc->received_a = 0.95 * tc->received_a + 0.05 * currenttime /
                      (packet_time_code - tc->time_code_a);
   } else {
      tc->received_a = 0.99 * tc->received_a + 0.01 * currenttime /
                      (packet_time_code - tc->time_code_a);
   }
   if (tc->calccount > 20) {
      if (!tc->time_code_b) {
         tc->time_code_b = packet_time_code;
         tc->received_b = currenttime;
      } else if (tc->time_code_b < packet_time_code) {
         calculatedtime = tc->received_b = tc->received_b + 
                          (packet_time_code - tc->time_code_b) * tc->received_a;
         tc->time_code_b = packet_time_code;
         if (tc->calccount < 28) {
            tc->received_b = 0.90 * tc->received_b + 0.1 * currenttime;
         } else if (tc->calccount < 300) {
            tc->received_b = 0.95 * tc->received_b + 0.05 * currenttime;
         } else {
            tc->received_b = 0.99 * tc->received_b + 0.01 * currenttime;
         }
      } else {
         calculatedtime = tc->received_b + 
                          (packet_time_code - tc->time_code_b) * tc->received_a;
      }
   }
   tc->received_c = currenttime;
   tc->time_code_c = packet_time_code;

   if (tc->calccount < 30) {
      /*
       * But in the start phase,
       * we asume every packet as not delayed.
       */
      calculatedtime = currenttime;
   }

   /*
   ** theoretical value for F1000 Phone
   */
   //calculatedtime = (tc->received_a = 125.) * packet_time_code;

   tc->calccount ++;
   calculatedtime += tc->dejitter_d;

   if (calculatedtime < currenttime) {
      calculatedtime = currenttime;
   } else if (calculatedtime > currenttime + 2.* tc->dejitter_d) {
      calculatedtime = currenttime + 2.* tc->dejitter_d;
   }

   /* every 500 counts show statistics */
   if (tc->calccount % 500 == 0) {
      DEBUGC(DBCLASS_RTPBABL, "currenttime = %f", currenttime);
      DEBUGC(DBCLASS_RTPBABL, "packetcode  = %i", packet_time_code);
      DEBUGC(DBCLASS_RTPBABL, "timecodes %i, %i, %i",
             tc->time_code_a, tc->time_code_b, tc->time_code_c);
      DEBUGC(DBCLASS_RTPBABL, "measuredtimes %f usec, %f usec, %f usec",
             tc->received_a, tc->received_b, tc->received_c);
      DEBUGC(DBCLASS_RTPBABL, "p2 - p1 = (%i,%f usec)",
             tc->time_code_b - tc->time_code_a,
             tc->received_b - tc->received_a);
      if (tc->time_code_c) {
         DEBUGC(DBCLASS_RTPBABL, "p3 - p2 = (%i,%f usec)",
                tc->time_code_c - tc->time_code_b,
                tc->received_c - tc->received_b);
      }
      DEBUGC(DBCLASS_RTPBABL, "calculatedtime = %f", calculatedtime);
      if (calculatedtime2) {
         DEBUGC(DBCLASS_RTPBABL, "calculatedtime2 = %f", calculatedtime2);
      }
      DEBUGC(DBCLASS_RTPBABL, "transmtime = %f (%f)", calculatedtime / 
             (160. * tc->received_a) - packet_time_code / 160,
             currenttime / (160. * tc->received_a) - 
             packet_time_code / 160);
      DEBUGC(DBCLASS_RTPBABL, "synthetic latency = %f, %f, %f, %i, %i",
             calculatedtime-currenttime, calculatedtime,
             currenttime, packet_time_code, 
             packet_time_code / 160);
   }

   split_double_time(calculatedtime, &output_r_tv);
   add_time_values(&output_r_tv,&(tc->starttime),ttv);
}



/*
 * Add timeval times
 */
static void add_time_values(const struct timeval *a,
                            const struct timeval *b, struct timeval *r) {
   r->tv_sec = a->tv_sec + b->tv_sec;
   r->tv_usec = a->tv_usec + b->tv_usec;
   if (r->tv_usec >= 1000000) {
      r->tv_usec -= 1000000;
      r->tv_sec++;
   }
}

/*
 * Subtract timeval values
 */
static void sub_time_values(const struct timeval *a,
                            const struct timeval *b, struct timeval *r) {
   if ((a->tv_sec < b->tv_sec) ||
       ((a->tv_sec == b->tv_sec) && (a->tv_usec < b->tv_usec))) {
      r->tv_usec = 0;
      r->tv_sec = 0;
      return;
   }
   if (a->tv_usec < b->tv_usec) {
      r->tv_sec = a->tv_sec - b->tv_sec - 1;
      r->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
   } else {
      r->tv_sec = a->tv_sec - b->tv_sec;
      r->tv_usec = a->tv_usec - b->tv_usec;
   }
}
/*
 * Compare timeval values
 */
static int cmp_time_values(const struct timeval *a, const struct timeval *b) {
   if (a->tv_sec < b->tv_sec) return -1;
   if (a->tv_sec > b->tv_sec) return 1;
   if (a->tv_usec < b->tv_usec) return -1;
   if (a->tv_usec > b->tv_usec) return 1;
   return 0;
}

/*
 * Convert TIMEVAL to DOUBLE 
 */
static double make_double_time(const struct timeval *tv) {
   return 1000000.0 * tv->tv_sec + tv->tv_usec;
}

/*
 * Send Top of queue
 *
 *    nolock: 1 - do not lock the mutex - lock already owned!
 */
static void send_top_of_que(int nolock) {
   rtp_delayed_message *m;
   int sts;

   if (msg_que) {
      m = msg_que;
      msg_que = m->next;
      m->next = free_memory;
      free_memory = m;

      if ((m->errret != NULL) && (m->errret->rtp_tx_sock)) {
         sts = sendto(m->socked, &(m->rtp_buff), m->message_len,
                      m->flags, (const struct sockaddr *)&(m->dst_addr),
                     (socklen_t)sizeof(m->dst_addr));
         if ((sts == -1) && (m->errret != NULL) && (errno != ECONNREFUSED)) {
            osip_call_id_t callid;

            ERROR("sendto() [%s:%i size=%zd] delayed call failed: %s",
                  utils_inet_ntoa(m->errret->remote_ipaddr),
                  m->errret->remote_port, m->message_len, strerror(errno));

            /* if sendto() fails with bad filedescriptor,
             * this means that the opposite stream has been
             * canceled or timed out.
             * we should then cancel this stream as well.*/

            WARN("stopping opposite stream");

            callid.number=m->errret->callid_number;
            callid.host=m->errret->callid_host;

            /* caller tells us if we must lock the fdset mutex */
            sts = rtp_relay_stop_fwd(&callid, 
                                     m->errret->direction,
                                     m->errret->media_stream_no,
                                     nolock);
            if (sts != STS_SUCCESS) {
               /* force the streams to timeout on next occasion */
               m->errret->timestamp=0;
            }
         } /* if sendto fails */
      }
   } /* if (msg_que) */
}

/*
 * Convert DOUBLE time into TIMEVAL
 */
static void split_double_time(double d, struct timeval *tv) {
   tv->tv_sec = d / 1000000.0;
   tv->tv_usec = d - 1000000.0 * tv->tv_sec;
}

/*
 *
 */
static int fetch_missalign_long_network_oder(char *where) {
   int i = 0;
   int k;
   int j;

   for (j=0;j<4;j++) {
      k = *where;
      i = (i<<8) | (0xFF & k);
      where ++;
   }
   return i;
}

#endif
