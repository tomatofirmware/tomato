/*
    Copyright (C) 2003-2009  Thomas Ries <tries@gmx.net>

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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#ifdef HAVE_PTHREAD_SETSCHEDPARAM
   #include <sched.h>
#endif

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "rtpproxy.h"
#include "log.h"

#ifdef GPL
   #include "dejitter.h"
#endif

static char const ident[]="$Id: rtpproxy_relay.c 482 2011-06-12 18:45:17Z hb9xar $";

/* configuration storage */
extern struct siproxd_config configuration;

/*
 * table to remember all active rtp proxy streams
 */
rtp_proxytable_t rtp_proxytable[RTPPROXY_SIZE];

/*
 * Mutex for thread synchronization (locking when accessing common 
 * data structures -> rtp_proxytable[]).
 *
 * use a 'fast' mutex for synchronizing - as these are portable... 
 */
static pthread_mutex_t rtp_proxytable_mutex = PTHREAD_MUTEX_INITIALIZER;

/* thread id of RTP proxy */
static pthread_t rtpproxy_tid=0;

/* master fd_set */
static fd_set master_fdset;
static int    master_fd_max;

/*
 * forward declarations of internal functions
 */
static void sighdl_alm(int sig) {/* just wake up from select() */};
static void *rtpproxy_main(void *i);
static void rtpproxy_kill( void );
static int  rtp_recreate_fdset(void);
static int  match_socket (int rtp_proxytable_idx);
static void error_handler (int rtp_proxytable_idx, int socket_type);


/*
 * initialize and create rtp_relay proxy thread
 *
 * RETURNS
 *	STS_SUCCESS on success
 */
int rtp_relay_init( void ) {
   int sts;
   int arg=0;
   struct sigaction sigact;
   pthread_attr_t attr;
   size_t stacksize;

#ifdef USE_DEJITTER
   dejitter_init();
#endif

   atexit(rtpproxy_kill);  /* cancel RTP thread at exit */

   /* clean proxy table */
   memset (rtp_proxytable, 0, sizeof(rtp_proxytable));

   /* initialize fd set for RTP proxy thread */
   FD_ZERO(&master_fdset); /* start with an empty fdset */
   master_fd_max=-1;

   /* install signal handler for SIGALRM - used to wake up
      the rtpproxy thread from select() hibernation */
   sigact.sa_handler = sighdl_alm;
   sigemptyset(&sigact.sa_mask);
   sigact.sa_flags=0;
   sigaction(SIGALRM, &sigact, NULL);

   pthread_attr_init(&attr);
   pthread_attr_init(&attr);
   pthread_attr_getstacksize (&attr, &stacksize);
   INFO("Current thread stacksize is %i kB",stacksize/1024);   

   /* experimental feature:
    * reduce the thread stack size to reduce the overall
    * memory footprint of siproxd on embedded systems
    *
    * Use at your own risk! 
    * Too small stack size may lead to unexplainable crashes!
    * Improper use may make siproxd eat your dog and vandalize
    * your garden.
    */
   if (configuration.thread_stack_size > 0) {
      stacksize = configuration.thread_stack_size*1024;
      pthread_attr_setstacksize (&attr, stacksize);
      INFO("Setting new thread stacksize to %i kB",stacksize/1024);   
   }

   DEBUGC(DBCLASS_RTP,"create thread");
   sts=pthread_create(&rtpproxy_tid, &attr, rtpproxy_main, (void *)&arg);
   DEBUGC(DBCLASS_RTP,"created, sts=%i", sts);

   /* set realtime scheduling - if started by root */
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
   {
      int uid,euid;
      struct sched_param schedparam;

#ifndef _CYGWIN
      uid=getuid();
      euid=geteuid();
      DEBUGC(DBCLASS_RTP,"uid=%i, euid=%i", uid, euid);
      if (uid != euid) seteuid(0);

      if (geteuid()==0) {
#endif

#if defined(HAVE_SCHED_GET_PRIORITY_MAX) && defined(HAVE_SCHED_GET_PRIORITY_MIN)
         int pmin, pmax;
         /* place ourself at 1/3 of the available priority space */
         pmin=sched_get_priority_min(SCHED_RR);
         pmax=sched_get_priority_max(SCHED_RR);
         schedparam.sched_priority=pmin+(pmax-pmin)/3;
         DEBUGC(DBCLASS_RTP,"pmin=%i, pmax=%i, using p=%i", pmin, pmax,
                schedparam.sched_priority);
#else
         /* just taken a number out of thin air */
         schedparam.sched_priority=10;
         DEBUGC(DBCLASS_RTP,"using p=%i", schedparam.sched_priority);
#endif
         sts=pthread_setschedparam(rtpproxy_tid, SCHED_RR, &schedparam);
         if (sts != 0) {
            ERROR("pthread_setschedparam failed: %s", strerror(errno));
         }
#ifndef _CYGWIN
      } else {
         INFO("Unable to use realtime scheduling for RTP proxy");
         INFO("You may want to start siproxd as root and switch UID afterwards");
      }
      if (uid != euid)  seteuid(euid);
#endif
   }
#endif

   return STS_SUCCESS;
}


/*
 * main() of rtpproxy
 */
static void *rtpproxy_main(void *arg) {
   fd_set fdset;
   int fd_max;
   int i, sts;
   int num_fd;
   static rtp_buff_t rtp_buff;
   int count;
   struct timeval last_tv ;
   struct timeval sleep_tv ;
   struct timeval current_tv ;
   struct timezone tz ;

   memcpy(&fdset, &master_fdset, sizeof(fdset));
   fd_max=master_fd_max;
   last_tv.tv_sec = 0;
   last_tv.tv_usec = 0;

   /* loop forever... */
   for (;;) {

#ifdef USE_DEJITTER
      if ((configuration.rtp_input_dejitter > 0) || 
          (configuration.rtp_output_dejitter > 0)) {
         /* calculate time until next packet to send from dejitter buffer */
         if (!dejitter_delay_of_next_tx(&sleep_tv, &current_tv)) {
            sleep_tv.tv_sec = 5;
            sleep_tv.tv_usec = 0;
         }
      } else {
         sleep_tv.tv_sec = 5;
         sleep_tv.tv_usec = 0;
      }
#else
      sleep_tv.tv_sec = 5;
      sleep_tv.tv_usec = 0;
#endif

      num_fd=select(fd_max+1, &fdset, NULL, NULL, &sleep_tv);
      gettimeofday(&current_tv, &tz);

#ifdef USE_DEJITTER
      /* Send delayed Packets that are timed to be send */
      if ((configuration.rtp_input_dejitter > 0) || 
          (configuration.rtp_output_dejitter > 0)) {
         dejitter_flush(&current_tv, LOCK_FDSET);
      }
#endif

      /* exit point for this thread in case of program terminaction */
      pthread_testcancel();
      if ((num_fd<0) && (errno==EINTR)) {
         /*
          * wakeup due to a change in the proxy table:
          * lock mutex, copy master FD set and unlock
          */
         pthread_mutex_lock(&rtp_proxytable_mutex);
         memcpy(&fdset, &master_fdset, sizeof(fdset));
         fd_max=master_fd_max;
         pthread_mutex_unlock(&rtp_proxytable_mutex);
         continue;
      }


      /*
       * LOCK the MUTEX
       */
      pthread_mutex_lock(&rtp_proxytable_mutex);

      /* check for data available and send to destination */
      for (i=0;(i<RTPPROXY_SIZE) && (num_fd>0);i++) {
         /*
          * RTCP control socket
          */
         if ( (rtp_proxytable[i].rtp_con_rx_sock != 0) &&
            FD_ISSET(rtp_proxytable[i].rtp_con_rx_sock, &fdset) ) {
            /* yup, have some data to send */
            num_fd--;

            /* read from sock rtp_proxytable[i].rtp_con_rx_sock */
            count=read(rtp_proxytable[i].rtp_con_rx_sock, rtp_buff, RTP_BUFFER_SIZE);

            /* check if something went banana */
            if (count < 0) error_handler(i,1) ;

            /* Buffer really full? This may indicate a too small buffer! */
            if (count == RTP_BUFFER_SIZE) {
               LIMIT_LOG_RATE(30) {
                  WARN("received an RTCP datagram bigger than buffer size");
               }
            }

            /*
             * forwarding an RTCP packet only makes sense if we really
             * have got some data in it (count > 0)
             */
            if (count > 0) {
               /* send only if I have the matching TX socket, otherwise throw away.
                * this requires a full 2-way communication to be set up for each
                * RTP stream... */
               if (rtp_proxytable[i].rtp_con_tx_sock != 0) {
                  struct sockaddr_in dst_addr;

                  /* write to dest via socket rtp_con_tx_sock */
                  dst_addr.sin_family = AF_INET;
                  memcpy(&dst_addr.sin_addr.s_addr,
                         &rtp_proxytable[i].remote_ipaddr,
                         sizeof(struct in_addr));
                  dst_addr.sin_port= htons(rtp_proxytable[i].remote_port+1);

                  /* Don't dejitter RTCP packets */
                  sts = sendto(rtp_proxytable[i].rtp_con_tx_sock, rtp_buff,
                               count, 0, (const struct sockaddr *)&dst_addr,
                               (socklen_t)sizeof(dst_addr));
                  /* ignore errors here. We don't know if the remote
                     site does receive RTCP messages at all (or reject
                     them with ICMP-whatever). If it fails, it is lost.
                     Basta, end of story. */
               }
            } /* count > 0 */
            /* RTCP does not wind up the keepalive timestamp. */
         } /* if */

         /*
          * RTP data stream
          */
         if ( (rtp_proxytable[i].rtp_rx_sock != 0) &&
            FD_ISSET(rtp_proxytable[i].rtp_rx_sock, &fdset) ) {
            /* yup, have some data to send */
            num_fd--;

            /* read from sock rtp_proxytable[i].rtp_rx_sock */
            count=read(rtp_proxytable[i].rtp_rx_sock, rtp_buff, RTP_BUFFER_SIZE);

            /* check if something went banana */
            if (count < 0) error_handler (i,0);

            /* Buffer really full? This may indicate a too small buffer! */
            if (count == RTP_BUFFER_SIZE) {
               LIMIT_LOG_RATE(30) {
                  WARN("received an RTP datagram bigger than buffer size");
               }
            }

            /*
             * forwarding an RTP packet only makes sense if we really
             * have got some data in it (count > 0)
             */
            if (count > 0) {
               /* send only if I have the matching TX socket, otherwise throw away.
                * this requires a full 2-way communication to be set up for each
                * RTP stream... */
               if (rtp_proxytable[i].rtp_tx_sock != 0) {
                  struct sockaddr_in dst_addr;
#ifdef USE_DEJITTER
                  struct timeval ttv;
#endif

                  /* write to dest via socket rtp_tx_sock */
                  dst_addr.sin_family = AF_INET;
                  memcpy(&dst_addr.sin_addr.s_addr,
                         &rtp_proxytable[i].remote_ipaddr,
                         sizeof(struct in_addr));
                  dst_addr.sin_port= htons(rtp_proxytable[i].remote_port);

#ifdef USE_DEJITTER
                  if ((configuration.rtp_input_dejitter > 0) || 
                      (configuration.rtp_output_dejitter > 0)) {
                     dejitter_calc_tx_time(&rtp_buff, &(rtp_proxytable[i].tc),
                                             &current_tv, &ttv);
                     dejitter_delayedsendto(rtp_proxytable[i].rtp_tx_sock,
                                            rtp_buff, count, 0, &dst_addr,
                                            &ttv, &current_tv,
                                            &rtp_proxytable[i], NOLOCK_FDSET);
                  } else {
                     /*&&& duplicated code - needs cleanup! */
                     sts = sendto(rtp_proxytable[i].rtp_tx_sock, rtp_buff,
                                  count, 0, (const struct sockaddr *)&dst_addr,
                                  (socklen_t)sizeof(dst_addr));
                     if (sts == -1) {
                        /* ECONNREFUSED: Got ICMP destination unreachable
                         * ENOBUFS: Full TX queue, packet dropped (FreeBSD for example)
                         */
                        if ((errno != ECONNREFUSED) && (errno != ENOBUFS)){
                           osip_call_id_t callid;

                           ERROR("sendto() [%s:%i size=%i] call failed: %s",
                           utils_inet_ntoa(rtp_proxytable[i].remote_ipaddr),
                           rtp_proxytable[i].remote_port, count, strerror(errno));

                           /* if sendto() fails with bad filedescriptor,
                            * this means that the opposite stream has been
                            * canceled or timed out.
                            * we should then cancel this stream as well.
                            * But only this specific media stream and not all
                            * active media streams in this ongoing call! */

                           WARN("stopping opposite stream");
                           callid.number=rtp_proxytable[i].callid_number;
                           callid.host=rtp_proxytable[i].callid_host;
                           /* don't lock the mutex, as we own the lock already */
                           sts = rtp_relay_stop_fwd(&callid,
                                                    rtp_proxytable[i].direction,
                                                    rtp_proxytable[i].media_stream_no,
                                                    NOLOCK_FDSET);
                           if (sts != STS_SUCCESS) {
                              /* force the streams to timeout on next occasion */
                              rtp_proxytable[i].timestamp=0;
                           }
                        }
                     }
                  }
#else
                  sts = sendto(rtp_proxytable[i].rtp_tx_sock, rtp_buff,
                               count, 0, (const struct sockaddr *)&dst_addr,
                               (socklen_t)sizeof(dst_addr));
                  if (sts == -1) {
                     /* ECONNREFUSED: Got ICMP destination unreachable
                      * ENOBUFS: Full TX queue, packet dropped (FreeBSD for example)
                      */
                     if ((errno != ECONNREFUSED) && (errno != ENOBUFS)){
                        osip_call_id_t callid;

                        ERROR("sendto() [%s:%i size=%i] call failed: %s",
                        utils_inet_ntoa(rtp_proxytable[i].remote_ipaddr),
                        rtp_proxytable[i].remote_port, count, strerror(errno));

                        /* if sendto() fails with bad filedescriptor,
                         * this means that the opposite stream has been
                         * canceled or timed out.
                         * we should then cancel this stream as well.*/

                        WARN("stopping opposite stream");
                        callid.number=rtp_proxytable[i].callid_number;
                        callid.host=rtp_proxytable[i].callid_host;
                        /* don't lock the mutex, as we own the lock already */
                        sts = rtp_relay_stop_fwd(&callid,
                                                 rtp_proxytable[i].direction,
                                                 rtp_proxytable[i].media_stream_no,
                                                 NOLOCK_FDSET);
                        if (sts != STS_SUCCESS) {
                           /* force the streams to timeout on next occasion */
                           rtp_proxytable[i].timestamp=0;
                        }
                     }
                  }
#endif
               }
            } /* count > 0 */

            /* update timestamp of last usage for both (RX and TX) entries.
             * This allows silence (no data) on one direction without breaking
             * the connection after the RTP timeout */
            rtp_proxytable[i].timestamp=current_tv.tv_sec;
            if (rtp_proxytable[i].opposite_entry > 0) {
               rtp_proxytable[rtp_proxytable[i].opposite_entry-1].timestamp=
                  current_tv.tv_sec;
            }
         } /* if */
      } /* for i */

      /*
       * age and clean rtp_proxytable (check every 10 seconds)
       */
      if (current_tv.tv_sec > last_tv.tv_sec) {
         last_tv.tv_sec = current_tv.tv_sec + 10 ;
         for (i=0;i<RTPPROXY_SIZE; i++) {
            if ( (rtp_proxytable[i].rtp_rx_sock != 0) &&
                 ((rtp_proxytable[i].timestamp+configuration.rtp_timeout) < 
                   current_tv.tv_sec)) {
               osip_call_id_t callid;

               /* this one has expired, clean it up */
               callid.number=rtp_proxytable[i].callid_number;
               callid.host=rtp_proxytable[i].callid_host;
#ifdef USE_DEJITTER
               if ((configuration.rtp_input_dejitter > 0) || 
                   (configuration.rtp_output_dejitter > 0)) {
                  dejitter_cancel(&rtp_proxytable[i]);
               }
#endif
               INFO("RTP stream %s@%s (media=%i) has expired",
                    callid.number, callid.host,
                    rtp_proxytable[i].media_stream_no);
               DEBUGC(DBCLASS_RTP,"RTP stream rx_sock=%i tx_sock=%i "
                      "%s@%s (idx=%i) has expired",
                      rtp_proxytable[i].rtp_rx_sock,
                      rtp_proxytable[i].rtp_tx_sock,
                      callid.number, callid.host, i);
               /* Don't lock the mutex, as we own the lock already here */
               /* Only stop the stream we caught is timeout and not everything.
                * This may be a multiple stream conversation (audio/video) and
                * just one (unused?) has timed out. Seen with VoIPEX PBX! */
               rtp_relay_stop_fwd(&callid, rtp_proxytable[i].direction,
                                  rtp_proxytable[i].media_stream_no,
                                  NOLOCK_FDSET);
            } /* if */
         } /* for i */
      } /* if (t>...) */

      /* copy master FD set */
      memcpy(&fdset, &master_fdset, sizeof(fdset));
      fd_max=master_fd_max;

      /*
       * UNLOCK the MUTEX
       */
      pthread_mutex_unlock(&rtp_proxytable_mutex);
   } /* for(;;) */

   return NULL;
}


/*
 * start an rtp stream on the proxy
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int rtp_relay_start_fwd (osip_call_id_t *callid, client_id_t client_id,
                         int rtp_direction, int call_direction,
                         int media_stream_no, struct in_addr local_ipaddr,
                         int *local_port, struct in_addr remote_ipaddr,
                         int remote_port, int dejitter) {
   static int prev_used_port = 0;
   int num_ports;
   int i2, i, j;
   int sock, port;
   int sock_con;
   int freeidx;
   int sts=STS_SUCCESS;
   int tos;
   osip_call_id_t cid;

   if (callid == NULL) {
      ERROR("rtp_relay_start_fwd: callid is NULL!");
      return STS_FAILURE;
   }

   /*
    * life insurance: check size of received call_id strings
    * I don't know what the maximum allowed size within SIP is,
    * so if this test fails maybe it's just necessary to increase
    * the constants CALLIDNUM_SIZE and/or CALLIDHOST_SIZE.
    */
   if (callid->number && (strlen(callid->number) >= CALLIDNUM_SIZE)) {
      ERROR("rtp_relay_start_fwd: received callid number [%s] "
            "has too many characters (%ld, max=%i)",
            callid->number, (long)strlen(callid->number),CALLIDNUM_SIZE);
      return STS_FAILURE;
   }
   if (callid->host && (strlen(callid->host) >= CALLIDHOST_SIZE)) {
      ERROR("rtp_relay_start_fwd: received callid host [%s] "
            "has too many characters (%ld, max=%i)",
            callid->host, (long)strlen(callid->host),CALLIDHOST_SIZE);
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_RTP,"rtp_relay_start_fwd: starting RTP proxy "
          "stream for: CallID=%s@%s [Client-ID=%s] (%s,%s) #=%i",
          callid->number, callid->host, client_id.idstring,
          ((rtp_direction == DIR_INCOMING) ? "incoming RTP" : "outgoing RTP"),
          ((call_direction == DIR_INCOMING) ? "incoming Call" : "outgoing Call"),
          media_stream_no);

   /* lock mutex */
   #define return is_forbidden_in_this_code_section
   pthread_mutex_lock(&rtp_proxytable_mutex);
   /*
    * !! We now have a locked MUTEX! It is forbidden to return() from
    * !! here up to the end of this funtion where the MUTEX is
    * !! unlocked again.
    * !! Per design, a mutex is locked (for one purpose) at *exactly one*
    * !! place in the code and unlocked also at *exactly one* place.
    * !! this minimizes the risk of deadlocks.
    */

   /*
    * figure out, if this is an request to start an RTP proxy stream
    * that is already existing (identified by SIP Call-ID, direction,
    * media_stream_no and some other client unique thing).
    * This can be due to UDP repetitions of the INVITE request...
    */
   for (i=0; i<RTPPROXY_SIZE; i++) {
      cid.number = rtp_proxytable[i].callid_number;
      cid.host   = rtp_proxytable[i].callid_host;
      if (rtp_proxytable[i].rtp_rx_sock &&
         (compare_callid(callid, &cid) == STS_SUCCESS) &&
         (rtp_proxytable[i].direction == rtp_direction) &&
         (rtp_proxytable[i].media_stream_no == media_stream_no) &&
         (compare_client_id(rtp_proxytable[i].client_id, client_id) == STS_SUCCESS)) {
         /*
          * The RTP port number reported by the UA MAY change
          * for a given media stream
          * (seen with KPhone during HOLD/unHOLD)
          * Also the destination IP may change during a re-Invite
          * (seen with Sipphone.com, re-Invites when using
          * the SIP - POTS gateway [SIP Minutes]
          */
         /* Port number */
         if (rtp_proxytable[i].remote_port != remote_port) {
            DEBUGC(DBCLASS_RTP,"RTP port number changed %i -> %i",
                   rtp_proxytable[i].remote_port, remote_port);
            rtp_proxytable[i].remote_port = remote_port;
         }
         /* IP address */
         if (memcmp(&rtp_proxytable[i].remote_ipaddr, &remote_ipaddr,
                    sizeof(remote_ipaddr))) {
            DEBUGC(DBCLASS_RTP,"RTP IP address changed to %s",
                   utils_inet_ntoa(remote_ipaddr));
            memcpy (&rtp_proxytable[i].remote_ipaddr, &remote_ipaddr,
                     sizeof(remote_ipaddr));
         }

#ifdef USE_DEJITTER
         /* Initialize up timecrontrol for dejitter function */
         if ((configuration.rtp_input_dejitter > 0) || 
             (configuration.rtp_output_dejitter > 0)) {
            dejitter_init_time(&rtp_proxytable[i].tc, dejitter);
         }
#endif


         /* return the already known local port number */
         DEBUGC(DBCLASS_RTP,"RTP stream already active idx=%i (remaddr=%s, "
                "remport=%i, lclport=%i, id=%s, #=%i)",
                i, utils_inet_ntoa(remote_ipaddr),
                rtp_proxytable[i].remote_port,
                rtp_proxytable[i].local_port,
                rtp_proxytable[i].callid_number,
                rtp_proxytable[i].media_stream_no);
         *local_port=rtp_proxytable[i].local_port;
         sts = STS_SUCCESS;
	 goto unlock_and_exit;
      } /* if already active */
   } /* for */


   /*
    * find first free slot in rtp_proxytable
    */
   freeidx=-1;
   for (j=0; j<RTPPROXY_SIZE; j++) {
      if (rtp_proxytable[j].rtp_rx_sock==0) {
         freeidx=j;
         break;
      }
   }

   /* rtp_proxytable port pool full? */
   if (freeidx == -1) {
      ERROR("rtp_relay_start_fwd: rtp_proxytable is full!");
      sts = STS_FAILURE;
      goto unlock_and_exit;
   }

   /* TODO: randomize the port allocation - start at a random offset to
         search in the allowed port range (so some modulo stuff w/
         random start offset
         - for i=x to (p1-p0)+x; p=p0+mod(x,p1-p0) */

   /* find a local port number to use and bind to it */
   sock=0;	/* RTP socket */
   sock_con=0;	/* RTCP socket */
   port=0;

   if ((prev_used_port < configuration.rtp_port_low) ||
       (prev_used_port > configuration.rtp_port_high)) {
      prev_used_port = configuration.rtp_port_high;
   }

   num_ports = configuration.rtp_port_high - configuration.rtp_port_low + 1;
   for (i2 = (prev_used_port - configuration.rtp_port_low + 1);
        i2 < (num_ports + prev_used_port - configuration.rtp_port_low + 1);
        i2++) {
      i = (i2%num_ports) + configuration.rtp_port_low;

      /* only allow even port numbers */
      if ((i % 2) != 0) continue;

      for (j=0; j<RTPPROXY_SIZE; j++) {
         /* check if port already in use */
         if (memcmp(&rtp_proxytable[j].local_ipaddr,
                     &local_ipaddr, sizeof(struct in_addr))== 0) {
            if (rtp_proxytable[j].local_port == i) break;
            if (rtp_proxytable[j].local_port == i + 1) break;
            if (rtp_proxytable[j].local_port + 1 == i) break;
            if (rtp_proxytable[j].local_port + 1 == i + 1) break;
          }
      }

      /* port is available, try to allocate */
      if (j == RTPPROXY_SIZE) {
         port=i;
         sock=sockbind(local_ipaddr, port, PROTO_UDP, 0);	/* RTP */

         if (sock) {
            sock_con=sockbind(local_ipaddr, port+1, PROTO_UDP, 0);	/* RTCP */
            /* if success break, else try further on */
            if (sock_con) break;
            sts = close(sock);
            DEBUGC(DBCLASS_RTP,"closed socket %i [%i] for RTP stream because "
                               "cant get pair sts=%i",
                               sock, i, sts);
         } /* if sock */
      } /* if j */

   } /* for i */
   prev_used_port = port+1;

   DEBUGC(DBCLASS_RTP,"rtp_relay_start_fwd: addr=%s, port=%i, sock=%i, "
          "freeidx=%i, input data dejitter buffer=%i usec", 
          utils_inet_ntoa(local_ipaddr), port, sock, freeidx, dejitter);

   /* found an unused port? No -> RTP port pool fully allocated */
   if ((port == 0) || (sock == 0) || (sock_con == 0)) {
      ERROR("rtp_relay_start_fwd: no RTP port available or bind() failed");
      sts = STS_FAILURE;
      goto unlock_and_exit;
   }

   /*&&&: do RTP and RTCP both set DSCP value? */
   /* set DSCP value, need to be ROOT */
   if (configuration.rtp_dscp) {
      int uid,euid;
      uid=getuid();
      euid=geteuid();
      DEBUGC(DBCLASS_RTP,"uid=%i, euid=%i", uid, euid);
      if (uid != euid) seteuid(0);
      if (geteuid()==0) {
         /* now I'm root */
         if (!(configuration.rtp_dscp & ~0x3f)) {
            tos = (configuration.rtp_dscp << 2) & 0xff;
            if(setsockopt(sock, SOL_IP, IP_TOS, &tos, sizeof(tos))) {
               ERROR("rtp_relay_start_fwd: setsockopt() failed while "
                     "setting DSCP value: %s", strerror(errno));
            }
         } else {
            ERROR("rtp_relay_start_fwd: Invalid DSCP value %d",
                  configuration.rtp_dscp);
            configuration.rtp_dscp = 0; /* inhibit further attempts */
         }
      } else {
         /* could not get root */
         WARN("siproxd not started as root - cannot set DSCP value");
         configuration.rtp_dscp = 0; /* inhibit further attempts */
      }
      /* drop privileges */
      if (uid != euid) seteuid(euid);
   }

   /* write entry into rtp_proxytable slot (freeidx) */
   rtp_proxytable[freeidx].rtp_rx_sock=sock;
   rtp_proxytable[freeidx].rtp_con_rx_sock = sock_con;

   if (callid->number) {
      strcpy(rtp_proxytable[freeidx].callid_number, callid->number);
   } else {
      rtp_proxytable[freeidx].callid_number[0]='\0';
   }

   if (callid->host) {
      strcpy(rtp_proxytable[freeidx].callid_host, callid->host);
   } else {
      rtp_proxytable[freeidx].callid_host[0]='\0';
   }

   /* store the passed Client-ID data */
   memcpy(&rtp_proxytable[freeidx].client_id, &client_id, sizeof(client_id_t));

   rtp_proxytable[freeidx].direction = rtp_direction;
   rtp_proxytable[freeidx].call_direction = call_direction;
   rtp_proxytable[freeidx].media_stream_no = media_stream_no;
   memcpy(&rtp_proxytable[freeidx].local_ipaddr,
          &local_ipaddr, sizeof(struct in_addr));
   rtp_proxytable[freeidx].local_port=port;
   memcpy(&rtp_proxytable[freeidx].remote_ipaddr,
          &remote_ipaddr, sizeof(struct in_addr));
   rtp_proxytable[freeidx].remote_port=remote_port;
   time(&rtp_proxytable[freeidx].timestamp);

#ifdef USE_DEJITTER
   /* Initialize up timecrontrol for dejitter function */
   if ((configuration.rtp_input_dejitter > 0) || 
       (configuration.rtp_output_dejitter > 0)) {
      dejitter_init_time(&rtp_proxytable[freeidx].tc, dejitter);
   }
#endif

   *local_port=port;

   /* call to firewall API: RTP port */
   fwapi_start_rtp(rtp_proxytable[freeidx].direction,
                   rtp_proxytable[freeidx].local_ipaddr,
                   rtp_proxytable[freeidx].local_port,
                   rtp_proxytable[freeidx].remote_ipaddr,
                   rtp_proxytable[freeidx].remote_port);
   /* call to firewall API: RTCP port */
   fwapi_start_rtp(rtp_proxytable[freeidx].direction,
                   rtp_proxytable[freeidx].local_ipaddr,
                   rtp_proxytable[freeidx].local_port + 1,
                   rtp_proxytable[freeidx].remote_ipaddr,
                   rtp_proxytable[freeidx].remote_port + 1);

   /* try to find the matching socket for return path. This has to be done for
    * both directions, the new socket and if one found, it must link back. */
   i=match_socket(freeidx);
   if (i>=0 && i<RTPPROXY_SIZE) j=match_socket(i);

   /* prepare FD set for next select operation */
   rtp_recreate_fdset();

   /* wakeup/signal rtp_proxythread from select() hibernation */
   if (!pthread_equal(rtpproxy_tid, pthread_self()))
      pthread_kill(rtpproxy_tid, SIGALRM);

//&&&
   DEBUGC(DBCLASS_RTP,"rtp_relay_start_fwd: started RTP proxy "
          "stream for: CallID=%s@%s [Client-ID=%s] %s #=%i idx=%i",
          rtp_proxytable[freeidx].callid_number,
          rtp_proxytable[freeidx].callid_host,
          rtp_proxytable[freeidx].client_id.idstring,
          ((rtp_proxytable[freeidx].direction == DIR_INCOMING) ? "incoming RTP" : "outgoing RTP"),
          rtp_proxytable[freeidx].media_stream_no, freeidx);

unlock_and_exit:
   /* unlock mutex */
   pthread_mutex_unlock(&rtp_proxytable_mutex);
   #undef return

   return sts;
}


/*
 * stop a rtp stream on the proxy
 *
 * if media_stream_no == -1, all media streams will be stopped,
 * otherwise only the specified one.
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int rtp_relay_stop_fwd (osip_call_id_t *callid,
                        int rtp_direction,
                        int media_stream_no, int nolock) {
   int i, sts;
   int retsts=STS_SUCCESS;
   int got_match=0;
   osip_call_id_t cid;
 
   if (callid == NULL) {
      ERROR("rtp_relay_stop_fwd: callid is NULL!");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_RTP,"rtp_relay_stop_fwd: stopping RTP proxy "
          "stream for: %s@%s (%s) (nolock=%i)",
          callid->number, callid->host,
          ((rtp_direction == DIR_INCOMING) ? "incoming" : "outgoing"),
          nolock);

   /*
    * lock mutex - only if not requested to skip the lock.
    * this is needed as we are also called from within
    * the RTP thread itself - and there we already own the lock.
    */
   #define return is_forbidden_in_this_code_section
   if (nolock == 0) {
      pthread_mutex_lock(&rtp_proxytable_mutex);
      /*
       * !! We now have a locked MUTEX! It is forbidden to return() from
       * !! here up to the end of this funtion where the MUTEX is
       * !! unlocked again.
       * !! Per design, a mutex is locked (for one purpose) at *exactly one*
       * !! place in the code and unlocked also at *exactly one* place.
       * !! this minimizes the risk of deadlocks.
       */
   }
   /* 
   * wakeup/signal rtp_proxythread from select() hibernation.
   * This must be done here before we close the socket, otherwise
   * we may get an select() error later from the proxy thread that
   * is still hibernating in select() now.
   */
   if (!pthread_equal(rtpproxy_tid, pthread_self()))
      pthread_kill(rtpproxy_tid, SIGALRM);

   /*
    * find the proper entry in rtp_proxytable
    * we need to loop the whole table, as there might be multiple
    * media streams active for the same callid (audio + video stream)
    * if media_stream_no == -1, all streams are stoppen, otherwise
    * if media_stream_no > 0 only the specified stream is stopped.
    */
   for (i=0; i<RTPPROXY_SIZE; i++) {
      cid.number = rtp_proxytable[i].callid_number;
      cid.host   = rtp_proxytable[i].callid_host;
      if (rtp_proxytable[i].rtp_rx_sock &&
         (compare_callid(callid, &cid) == STS_SUCCESS) &&
         (rtp_proxytable[i].direction == rtp_direction) &&
         ((media_stream_no < 0) ||
          (media_stream_no == rtp_proxytable[i].media_stream_no))) {
         /* close RTP sockets */
         sts = close(rtp_proxytable[i].rtp_rx_sock);
         DEBUGC(DBCLASS_RTP,"closed socket %i for RTP stream "
                "%s:%s == %s:%s  (idx=%i) sts=%i",
                rtp_proxytable[i].rtp_rx_sock,
                rtp_proxytable[i].callid_number,
                rtp_proxytable[i].callid_host,
                callid->number, callid->host, i, sts);
         if (sts < 0) {
            ERROR("Error in close(%i): %s nolock=%i %s:%s\n",
                  rtp_proxytable[i].rtp_rx_sock,
                  strerror(errno), nolock,
                  callid->number, callid->host);
         }
         /* call to firewall API (RTP port) */
         fwapi_stop_rtp(rtp_proxytable[i].direction,
                   rtp_proxytable[i].local_ipaddr,
                   rtp_proxytable[i].local_port,
                   rtp_proxytable[i].remote_ipaddr,
                   rtp_proxytable[i].remote_port);
         /* close RTCP socket */
         sts = close(rtp_proxytable[i].rtp_con_rx_sock);
         DEBUGC(DBCLASS_RTP,"closed socket %i for RTCP stream sts=%i",
                rtp_proxytable[i].rtp_con_rx_sock, sts);
         if (sts < 0) {
            ERROR("Error in close(%i): %s nolock=%i %s:%s\n",
                  rtp_proxytable[i].rtp_con_rx_sock,
                  strerror(errno), nolock,
                  callid->number, callid->host);
         }
         /* call to firewall API (RTCP port) */
         fwapi_stop_rtp(rtp_proxytable[i].direction,
                   rtp_proxytable[i].local_ipaddr,
                   rtp_proxytable[i].local_port + 1,
                   rtp_proxytable[i].remote_ipaddr,
                   rtp_proxytable[i].remote_port + 1);
         /* clean up */
         if (rtp_proxytable[i].opposite_entry) {
            rtp_proxytable[rtp_proxytable[i].opposite_entry-1].opposite_entry=0;
         }
         memset(&rtp_proxytable[i], 0, sizeof(rtp_proxytable[0]));
         got_match=1;
      }
   }

   /* did not find an active stream... */
   if (!got_match) {
      DEBUGC(DBCLASS_RTP,
             "rtp_relay_stop_fwd: can't find active stream for %s@%s (%s)",
             callid->number, callid->host,
             ((rtp_direction == DIR_INCOMING) ? "incoming RTP" : "outgoing RTP"));
      retsts = STS_FAILURE;
      goto unlock_and_exit;
   }


   /* prepare FD set for next select operation */
   rtp_recreate_fdset();
   

unlock_and_exit:
   /*
    * unlock mutex - only if not requested to skip the lock.
    * this is needed as we are also called from within
    * the RTP thread itself - and there we already own the lock.
    */
   if (nolock == 0) {
      pthread_mutex_unlock(&rtp_proxytable_mutex);
   }
   #undef return

   return retsts;
}


/*
 * some sockets have been newly created or removed -
 * recreate the FD set for next select operation
 *
 * RETURNS
 *	STS_SUCCESS on success (always)
 */
static int rtp_recreate_fdset(void) {
   int i;

   FD_ZERO(&master_fdset);
   master_fd_max=-1;
   for (i=0;i<RTPPROXY_SIZE;i++) {
      if (rtp_proxytable[i].rtp_rx_sock != 0) {
         /* RTP */
         FD_SET(rtp_proxytable[i].rtp_rx_sock, &master_fdset);
         if (rtp_proxytable[i].rtp_rx_sock > master_fd_max) {
            master_fd_max=rtp_proxytable[i].rtp_rx_sock;
         }
         /* RTPCP */
         FD_SET(rtp_proxytable[i].rtp_con_rx_sock, &master_fdset);
         if (rtp_proxytable[i].rtp_con_rx_sock > master_fd_max) {
            master_fd_max=rtp_proxytable[i].rtp_con_rx_sock;
         }
      }
   } /* for i */
   return STS_SUCCESS;
}


/*
 * kills the rtp_proxy thread
 *
 * RETURNS
 *	-
 */
static void rtpproxy_kill( void ) {
   void *thread_status;
   osip_call_id_t cid;
   int i, sts;

   /* stop any active RTP stream */
   for (i=0;i<RTPPROXY_SIZE;i++) {
      if (rtp_proxytable[i].rtp_rx_sock != 0) {
         cid.number = rtp_proxytable[i].callid_number;
         cid.host   = rtp_proxytable[i].callid_host;
         sts = rtp_relay_stop_fwd(&cid, rtp_proxytable[i].direction,
                                  rtp_proxytable[i].media_stream_no,
                                  LOCK_FDSET);
      }
   }
   

   /* kill the thread */
   if (rtpproxy_tid) {
      pthread_cancel(rtpproxy_tid);
      pthread_kill(rtpproxy_tid, SIGALRM);
      pthread_join(rtpproxy_tid, &thread_status);
   }

   DEBUGC(DBCLASS_RTP,"killed RTP proxy thread");
   return;
}


/*
 * match_socket
 * matches and cross connects two rtp_proxytable entries
 * (corresponds to the two data directions of one RTP stream
 * within one call).
 * returns the matching rtp_proxytable index of -1 if not found.
 */
static int match_socket (int rtp_proxytable_idx) {
   int j;
   int rtp_direction = rtp_proxytable[rtp_proxytable_idx].direction;
   int call_direction = rtp_proxytable[rtp_proxytable_idx].call_direction;
   int media_stream_no = rtp_proxytable[rtp_proxytable_idx].media_stream_no;
   osip_call_id_t callid;

   callid.number = rtp_proxytable[rtp_proxytable_idx].callid_number;
   callid.host = rtp_proxytable[rtp_proxytable_idx].callid_host;

   for (j=0;(j<RTPPROXY_SIZE);j++) {
      osip_call_id_t cid;
      cid.number = rtp_proxytable[j].callid_number;
      cid.host = rtp_proxytable[j].callid_host;

      /* match on:
       * - same call ID
       * - same media stream
       * - opposite direction
       * - different client ID
       */
      if ( (rtp_proxytable[j].rtp_rx_sock != 0) &&
           (compare_callid(&callid, &cid) == STS_SUCCESS) &&		// same Call-ID
           (call_direction == rtp_proxytable[j].call_direction) &&	// same Call direction
           (media_stream_no == rtp_proxytable[j].media_stream_no) &&	// same stream
           (rtp_direction != rtp_proxytable[j].direction) ) {		// opposite RTP dir
         char remip1[16], remip2[16];
         char lclip1[16], lclip2[16];
         
         /* connect the two sockets */
         rtp_proxytable[rtp_proxytable_idx].rtp_tx_sock = rtp_proxytable[j].rtp_rx_sock;
         rtp_proxytable[rtp_proxytable_idx].rtp_con_tx_sock = rtp_proxytable[j].rtp_con_rx_sock;

         strcpy(remip1, utils_inet_ntoa(rtp_proxytable[j].remote_ipaddr));
         strcpy(lclip1, utils_inet_ntoa(rtp_proxytable[j].local_ipaddr));	
         strcpy(remip2, utils_inet_ntoa(rtp_proxytable[rtp_proxytable_idx].remote_ipaddr));
         strcpy(lclip2, utils_inet_ntoa(rtp_proxytable[rtp_proxytable_idx].local_ipaddr));

         rtp_proxytable[rtp_proxytable_idx].opposite_entry=j+1;
         rtp_proxytable[j].opposite_entry=rtp_proxytable_idx+1;

         DEBUGC(DBCLASS_RTP, "connected entry %i (fd=%i, %s:%i->%s:%i) <-> entry %i (fd=%i, %s:%i->%s:%i)",
                             j, rtp_proxytable[j].rtp_rx_sock,
                             lclip1, rtp_proxytable[j].local_port,
                             remip1, rtp_proxytable[j].remote_port,
                             rtp_proxytable_idx,
                             rtp_proxytable[rtp_proxytable_idx].rtp_rx_sock,
                             lclip2, rtp_proxytable[rtp_proxytable_idx].local_port,
                             remip2, rtp_proxytable[rtp_proxytable_idx].remote_port
                             );
         break;
      }
   }
   if (j >= RTPPROXY_SIZE) j= -1;
   return j;
}


/*
 * error_handler
 *
 * rtp_proxytable_idx:	index into the rtp_proxytable array
 * socket_type: 	1 - RTCP, 0 - RTP
 */
static void error_handler (int rtp_proxytable_idx, int socket_type) {
   /*
    * It has been seen on linux 2.2.x systems that for some
    * reason (ICMP issue? -> below) inside the RTP relay, select()
    * claims that a certain file descriptor has data available to
    * read, a subsequent call to read() or recv() then does block!!
    * So lets make the FD's we are going to use non-blocking, so
    * we will at least survive and not run into a deadlock.
    *
    * We catch this here with this workaround (pronounce "HACK")
    * and hope that next time we pass by it will be ok again.
    */
   if (errno == EAGAIN) {
      /* I may want to remove this WARNing */
      WARN("read() [fd=%i, %s:%i] would block, but select() "
           "claimed to be readable!",
           socket_type ? rtp_proxytable[rtp_proxytable_idx].rtp_rx_sock : 
                         rtp_proxytable[rtp_proxytable_idx].rtp_con_rx_sock,
           utils_inet_ntoa(rtp_proxytable[rtp_proxytable_idx].local_ipaddr),
           rtp_proxytable[rtp_proxytable_idx].local_port + socket_type);
   }

   /*
    * I *MAY* receive ICMP destination unreachable messages when I
    * try to send RTP traffic to a destination that is in HOLD
    * (better: is not listening on the UDP port where I send
    * my RTP data to).
    * So I should *not* do this - or ignore errors originating
    * by this -> ECONNREFUSED
    *
    * Note: This error is originating from a previous send() on the
    *       same socket and has nothing to do with the read() we have
    *       done above!
    */
   if (errno != ECONNREFUSED) {
      /* some other error that I probably want to know about */
      int j;
      WARN("read() [fd=%i, %s:%i] returned error [%i:%s]",
          socket_type ? rtp_proxytable[rtp_proxytable_idx].rtp_rx_sock : 
                        rtp_proxytable[rtp_proxytable_idx].rtp_con_rx_sock,
          utils_inet_ntoa(rtp_proxytable[rtp_proxytable_idx].local_ipaddr),
          rtp_proxytable[rtp_proxytable_idx].local_port + socket_type,
          errno, strerror(errno));
      for (j=0; j<RTPPROXY_SIZE;j++) {
         DEBUGC(DBCLASS_RTP, "%i - rx:%i tx:%i %s@%s dir:%i "
                "lp:%i, rp:%i rip:%s",
                j,
                socket_type ? rtp_proxytable[rtp_proxytable_idx].rtp_rx_sock : 
                              rtp_proxytable[rtp_proxytable_idx].rtp_con_rx_sock,
                socket_type ? rtp_proxytable[rtp_proxytable_idx].rtp_tx_sock : 
                              rtp_proxytable[rtp_proxytable_idx].rtp_con_tx_sock,
                rtp_proxytable[j].callid_number,
                rtp_proxytable[j].callid_host,
                rtp_proxytable[j].direction,
                rtp_proxytable[j].local_port,
                rtp_proxytable[j].remote_port,
                utils_inet_ntoa(rtp_proxytable[j].remote_ipaddr));
      } /* for j */
   } /* if errno != ECONNREFUSED */
}

