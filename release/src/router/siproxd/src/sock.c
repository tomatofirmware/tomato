/*
    Copyright (C) 2002-2009  Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.
    
    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: sock.c 465 2010-10-13 22:46:37Z hb9xar $";


/* configuration storage */
extern struct siproxd_config configuration;


/* static functions */
static void tcp_expire(void);
static int tcp_add(struct sockaddr_in addr, int fd);
static int tcp_connect(struct sockaddr_in dst_addr);
static int tcp_remove(int idx);

/* module local variables */

/* UDP socket used for SIP datagrams */
int sip_udp_socket=0;

/* TCP listen socket used for SIP */
int sip_tcp_socket=0;

/* TCP sockets used for SIP connections (twice the max number of clients) */
struct {
   int fd;				/* file descriptor, 0=unused */
   struct sockaddr_in dst_addr;		/* remote target of TCP connection */
   time_t traffic_ts;			/* last 'alive' TS (real SIP traffic) */
   time_t keepalive_ts;			/* last 'alive' TS */
   int    rxbuf_size;
   int    rxbuf_len;
   char   *rx_buffer;
} sip_tcp_cache[2*URLMAP_SIZE];


/*
 * binds to SIP UDP and TCP sockets for listening to incoming packets
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_listen (void) {
   struct in_addr ipaddr;

   /* listen on UDP port */
   memset(&ipaddr, 0, sizeof(ipaddr));
   sip_udp_socket=sockbind(ipaddr, configuration.sip_listen_port,
                           PROTO_UDP, 1);
   if (sip_udp_socket == 0) return STS_FAILURE; /* failure */

   /* set DSCP value, need to be ROOT */
   if (configuration.sip_dscp) {
      int tos;
      int uid,euid;
      uid=getuid();
      euid=geteuid();
      DEBUGC(DBCLASS_SIP,"uid=%i, euid=%i", uid, euid);
      if (uid != euid) seteuid(0);
      if (geteuid()==0) {
         /* now I'm root */
         if (!(configuration.sip_dscp & ~0x3f)) {
            tos = (configuration.sip_dscp << 2) & 0xff;
            if(setsockopt(sip_udp_socket, SOL_IP, IP_TOS, &tos, sizeof(tos))) {
               ERROR("sipsock_listen: setsockopt() failed while "
                     "setting DSCP value: %s", strerror(errno));
            }
         } else {
            ERROR("sipsock_listen: Invalid DSCP value %d",
                  configuration.sip_dscp);
            configuration.sip_dscp = 0; /* inhibit further attempts */
         }
      } else {
         /* could not get root */
         WARN("siproxd not started as root - cannot set DSCP value");
         configuration.rtp_dscp = 0; /* inhibit further attempts */
      }
      /* drop privileges */
      if (uid != euid) seteuid(euid);
   }

   /* listen on TCP port */
   memset(&ipaddr, 0, sizeof(ipaddr));
   sip_tcp_socket=sockbind(ipaddr, configuration.sip_listen_port,
                           PROTO_TCP, 1);
   if (sip_tcp_socket == 0) return STS_FAILURE; /* failure */
   if (listen(sip_tcp_socket, 10)) {
      ERROR("TCP listen() failed: %s", strerror(errno));
      return STS_FAILURE;
   }

   INFO("bound to port %i", configuration.sip_listen_port);
   DEBUGC(DBCLASS_NET,"bound UDP socket=%i, TCP socket=%i",
          sip_udp_socket, sip_tcp_socket);

   /* initialize the TCP connection cache array */
   memset(&sip_tcp_cache, 0, sizeof(sip_tcp_cache));

   return STS_SUCCESS;
}


/*
 * read a message from SIP listen socket (UDP datagram)
 *
 * RETURNS number of bytes read (=0 if nothing read, <0 timeout)
 *         from is modified to return the sockaddr_in of the sender
 */
int sipsock_waitfordata(char *buf, size_t bufsize,
                        struct sockaddr_in *from, int *protocol) {
   int i, fd;
   fd_set fdset;
   int highest_fd, num_fd_active;
   static struct timeval timeout={0,0};
   int length;
   socklen_t fromlen;

   DEBUGC(DBCLASS_BABBLE,"entered sipsock_waitfordata");

   /* we keep the select() timeout running acrosse multiple calls to
    * select(). This avoids missing select() timeouts if the system
    * is busy with a lot of SIP traffic, causing NOT doing some
    * cyclic tasks. Like this we ensure that every 'N' (N=5) seconds
    * sipsock_waitfordata will return a timeout condition.
    * Note: there is still the remote possibility that SIP packet
    * arrive so fast that select() always return data available - 
    * but in this case YOU have some seroious other issues...
    */
   if ((timeout.tv_sec== 0) && (timeout.tv_usec == 0)) {
      DEBUGC(DBCLASS_BABBLE,"winding up select() timeout");
      timeout.tv_sec=5;
      timeout.tv_usec=0;
   }

   /* prepare FD set: UDP, TCP listen */
   FD_ZERO(&fdset);
   FD_SET (sip_udp_socket, &fdset);
   FD_SET (sip_tcp_socket, &fdset);
   if (sip_udp_socket > sip_tcp_socket) {
      highest_fd = sip_udp_socket;
   } else {
      highest_fd = sip_tcp_socket;
   }

   /* prepare FD set: TCP connections */
   for (i=0; i<(sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++) {
      /* active TCP conenction? */
      if (sip_tcp_cache[i].fd) {
         /* add to FD set */
         FD_SET(sip_tcp_cache[i].fd, &fdset);
         if (sip_tcp_cache[i].fd > highest_fd) {
            highest_fd = sip_tcp_cache[i].fd;
         }
      } /* if fd > 0 */
   }

   /* select() on all FD's with timeout */
   num_fd_active=select (highest_fd+1, &fdset, NULL, NULL, &timeout);

   /* WARN on failures */
   if (num_fd_active < 0) {
      /* WARN on failure, except if it is an "interrupted system call"
         as it will result by SIGINT, SIGTERM */
      if (errno != EINTR) {
         WARN("select() returned error [%i:%s]",errno, strerror(errno));
      } else {
         DEBUGC(DBCLASS_NET,"select() returned error [%i:%s]",
                errno, strerror(errno));
      }
   }

   /* nothing here = timeout condition */
   if (num_fd_active <= 0) {
      /* process the active TCP connection list - expire old entries */
      tcp_expire();
      return -1;
   }


for (i=0; i< highest_fd; i++) {
   if (FD_ISSET(i, &fdset)) DEBUGC(DBCLASS_BABBLE, "FD %i = active", i);
}

   /*
    * Some FD's have signalled that data is available (fdset)
    * Process them:
    * - UDP socket: read data and return
    * - TCP listen socket: Accept connection, update TCP cache and return
    * - TCP connection socket: read data, update alive timestamp & return
    *   In case of disconnected socket (recv error [all but EAGAIN, EINTR])
    *   close connection
    */

   /* Strategy to get get data from the FD's:
    *  1) check TCP listen socket, if connection pending ACCEPT
    *  2) check UDP socket. If data available, process that & return
    *  3) check TCP sockets, take first in table with data & return
    */


   /*
    * Check TCP listen socket
    */
   if (FD_ISSET(sip_tcp_socket, &fdset)) {
      fromlen=sizeof(struct sockaddr_in);
      fd = accept(sip_tcp_socket, (struct sockaddr *)from, &fromlen);
      if (fd < 0) {
         WARN("accept() returned error [%i:%s]",errno, strerror(errno));
         return 0;
      }

      i=tcp_add(*from, fd);
      if (i < 0) {
         ERROR("out of space in TCP connection cache - rejecting");
         close(fd);
         return 0;
      }

      DEBUGC(DBCLASS_NET, "accepted TCP connection from [%s] fd=%i",
             utils_inet_ntoa(from->sin_addr), fd);

      num_fd_active--;
      if (num_fd_active <=0) return 0;
   }

   /*
    * Check UDP socket
    */
   if (FD_ISSET(sip_udp_socket, &fdset)) {
      *protocol = PROTO_UDP;

      fromlen=sizeof(struct sockaddr_in);
      length=recvfrom(sip_udp_socket, buf, bufsize, 0,
                      (struct sockaddr *)from, &fromlen);

      if (length < 0) {
         WARN("recvfrom() returned error [%s]",strerror(errno));
         length=0;
      }

      DEBUGC(DBCLASS_NET,"received UDP packet from [%s:%i] count=%i",
             utils_inet_ntoa(from->sin_addr), ntohs(from->sin_port), length);
      DUMP_BUFFER(DBCLASS_NETTRAF, buf, length);

      return length;
   }


   /*
    * Check active TCP sockets
    */
   for (i=0; i<(sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++) {
      if (sip_tcp_cache[i].fd == 0) continue;

      /* no more active FD's to be expected, exit the loop */
      if (num_fd_active <= 0) break;

      if (FD_ISSET(sip_tcp_cache[i].fd, &fdset)) {
         /* found a match */
         DEBUGC(DBCLASS_BABBLE,"matched active TCP fd=%i idx=%i",
                sip_tcp_cache[i].fd, i);

         num_fd_active--;
         *protocol = PROTO_TCP;
         memcpy(from, &sip_tcp_cache[i].dst_addr, sizeof(struct sockaddr_in));

         length = recv(sip_tcp_cache[i].fd, buf, bufsize, 0);
         if (length < 0) {
            WARN("recv() returned error [%s], disconnecting TCP [%s] fd=%i",
                 strerror(errno), utils_inet_ntoa(from->sin_addr),
                 sip_tcp_cache[i].fd);
            length=0;
            tcp_remove(i);
         }
         if (length == 0) {
            /* length=0 indicates a disconnect from remote side */
            DEBUGC(DBCLASS_NET, "received TCP disconnect [%s:%i] fd=%i",
                   utils_inet_ntoa(from->sin_addr), ntohs(from->sin_port),
                   sip_tcp_cache[i].fd);
            tcp_remove(i);
            continue;
         }

         /* prematurely check for <CR><LF> keepalives, no need to do any
            work on them... Set length = 0 and done. */
         if (length == 2 && (memcmp(buf, "\x0d\x0a", 2) == 0)) {
            DEBUGC(DBCLASS_NET, "got a SIP TCP keepalive from [%s:%i] fd=%i",
                   utils_inet_ntoa(from->sin_addr), ntohs(from->sin_port),
                   sip_tcp_cache[i].fd);
            return 0;
         }

         DEBUGC(DBCLASS_NET,"received TCP packet from [%s:%i] count=%i fd=%i",
                utils_inet_ntoa(from->sin_addr), ntohs(from->sin_port),
                length, sip_tcp_cache[i].fd);
         DUMP_BUFFER(DBCLASS_NETTRAF, buf, length);

         /* check for <CR><LF> termination of TCP RX buffer */
         if ((length > 2) && 
             (memcmp(&buf[length-2], "\x0d\x0a", 2) != 0)) {
            /* not terminated */
            DEBUGC(DBCLASS_NET, "received incomplete fragment, buffering...");
            /* append to RX buffer of his connection */
            if (sip_tcp_cache[i].rxbuf_len+length < sip_tcp_cache[i].rxbuf_size) {
               memcpy(&sip_tcp_cache[i].rx_buffer[sip_tcp_cache[i].rxbuf_len],
                      buf, length);
               sip_tcp_cache[i].rxbuf_len+=length;
            } else {
               /* out of RX buffer space, discard this SIP frame */
               DEBUGC(DBCLASS_NET, "RX buffer too small, discarding this frame");
               sip_tcp_cache[i].rxbuf_len=0;
            }

            return 0;

         } else {
            /* terminated by <CR><LF> */
            if (sip_tcp_cache[i].rxbuf_len != 0) {
               /* have already buffered data waiting. Copy new fragment to end
                * of RX buffer and then copy all back... */

               if (sip_tcp_cache[i].rxbuf_len+length < sip_tcp_cache[i].rxbuf_size) {
                  DEBUGC(DBCLASS_NET, "received last fragment, assembling...");
                  memcpy(&sip_tcp_cache[i].rx_buffer[sip_tcp_cache[i].rxbuf_len],
                         buf, length);
                  sip_tcp_cache[i].rxbuf_len+=length;
               } else {
                  /* out of RX buffer space, discard this SIP frame */
                  DEBUGC(DBCLASS_NET, "RX buffer too small, discarding this frame");
                  sip_tcp_cache[i].rxbuf_len=0;
                  return 0;
               }

               /* copy whole RX buffer to the callers buffer */
               if (sip_tcp_cache[i].rxbuf_len <= bufsize) {
                  memcpy (buf, sip_tcp_cache[i].rx_buffer, sip_tcp_cache[i].rxbuf_len);
                  length = sip_tcp_cache[i].rxbuf_len;
               } else {
                  /* TCP RX buffer bigger than callers buffer... */
                  DEBUGC(DBCLASS_NET, "buffer passed to sipsock_waitfordata is too small");
                  sip_tcp_cache[i].rxbuf_len=0;
                  length =0;
               }
            }

            /* update activity timestamp */
            if (length > 0) {
               time(&sip_tcp_cache[i].traffic_ts);
               sip_tcp_cache[i].keepalive_ts=sip_tcp_cache[i].traffic_ts;
            }

            return length;
         }

      } /* FD_ISSET(sip_tcp_cache[i].fd, &fdset */
   } /* for i */

   /* no data found to be processed */
   return 0;
}


/*
 * sends an SIP datagram (UDP or TCP) to the specified destination
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_send(struct in_addr addr, int port, int protocol,
                 char *buffer, size_t size) {
   struct sockaddr_in dst_addr;
   int sts;
   int i;

   /* first time: allocate a socket for sending */
   if (sip_udp_socket == 0) {
      ERROR("SIP socket not allocated");
      return STS_FAILURE;
   }

   if (buffer == NULL) {
      ERROR("sipsock_send got NULL buffer");
      return STS_FAILURE;
   }

   if (protocol == PROTO_UDP) {
      /*
       * UDP target
       */
      dst_addr.sin_family = AF_INET;
      memcpy(&dst_addr.sin_addr, &addr, sizeof(struct in_addr));
      dst_addr.sin_port= htons(port);

      DEBUGC(DBCLASS_NET,"send UDP packet to %s: %i", utils_inet_ntoa(addr),port);
      DUMP_BUFFER(DBCLASS_NETTRAF, buffer, size);

      sts = sendto(sip_udp_socket, buffer, size, 0,
                   (const struct sockaddr *)&dst_addr,
                   (socklen_t)sizeof(dst_addr));

      if (sts == -1) {
         if (errno != ECONNREFUSED) {
            ERROR("sendto() [%s:%i size=%ld] call failed: %s",
                  utils_inet_ntoa(addr),
                  port, (long)size, strerror(errno));
            return STS_FAILURE;
         }
         DEBUGC(DBCLASS_BABBLE,"sendto() [%s:%i] call failed: %s",
                utils_inet_ntoa(addr), port, strerror(errno));
      }
   } else if (protocol == PROTO_TCP) {
      /*
       * TCP target
       */
      dst_addr.sin_family = AF_INET;
      memcpy(&dst_addr.sin_addr, &addr, sizeof(struct in_addr));
      dst_addr.sin_port= htons(port);

      /* check connection cache for an existing TCP connection */
      i=tcp_find(dst_addr);

      /* if no TCP connection found, do a connect (non blocking) and add to list */
      if (i < 0) {
         DEBUGC(DBCLASS_NET,"no TCP connection found to %s:%i - connecting",
                utils_inet_ntoa(addr), port);

         i=tcp_connect(dst_addr);
         if (i < 0) {
            ERROR("tcp_connect() failed");
            return STS_FAILURE;
         }

      } /* if i */

      /* send data and update alive timestamp */
      DEBUGC(DBCLASS_NET,"send TCP packet to %s:%i", utils_inet_ntoa(addr), port);
      DUMP_BUFFER(DBCLASS_NETTRAF, buffer, size);

      time(&sip_tcp_cache[i].traffic_ts);
      sip_tcp_cache[i].keepalive_ts=sip_tcp_cache[i].traffic_ts;

      sts = send(sip_tcp_cache[i].fd, buffer, size, 0);

      if (sts == -1) {
         ERROR("send() [%s:%i size=%ld] call failed: %s",
               utils_inet_ntoa(addr),
               port, (long)size, strerror(errno));
         return STS_FAILURE;
      }

   } else {
      /*
       * unknown/unsupported protocol
       */
      ERROR("sipsock_send: only UDP and TCP supported by now");
      return STS_FAILURE;
   }

   return STS_SUCCESS;
}



/*
 * generic routine to allocate and bind a socket to a specified
 * local address and port (UDP)
 * errflg !=0 log errors, ==0 don't
 *
 * RETURNS socket number on success, zero on failure
 */
int sockbind(struct in_addr ipaddr, int localport, int protocol, int errflg) {
   struct sockaddr_in my_addr;
   int sts, on=1;
   int sock;
   int flags;

   memset(&my_addr, 0, sizeof(my_addr));

   my_addr.sin_family = AF_INET;
   memcpy(&my_addr.sin_addr, &ipaddr, sizeof(struct in_addr));
   my_addr.sin_port = htons(localport);

   if (protocol == PROTO_UDP) {
      sock=socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   } else if (protocol == PROTO_TCP) {
      sock=socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
   } else {
      if (errflg) ERROR("invalig protocol: %i", protocol);
      return 0;
   }
   if (sock < 0) {
      ERROR("socket call failed: %s",strerror(errno));
      return 0;
   }

   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on , sizeof(on)) < 0) {
      ERROR("setsockopt returned error [%i:%s]",errno, strerror(errno));
      return 0;
   }

   sts=bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
   if (sts != 0) {
      if (errflg) ERROR("bind failed: %s",strerror(errno));
      close(sock);
      return 0;
   }

   /*
    * It has been seen on linux 2.2.x systems that for some
    * reason (bug?) inside the RTP relay, select()
    * claims that a certain file descriptor has data available to
    * read, a subsequent call to read() or recv() then does block!!
    * So lets make the FD's we are going to use non-blocking, so
    * we will at least survive and not run into a deadlock.
    *
    * There is a way to (more or less) reproduce this effect:
    * Make a local UA to local UA call and then very quickly do
    * HOLD/unHOLD, several times.
    */
   flags = fcntl(sock, F_GETFL);
   if (flags < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }
   if (fcntl(sock, F_SETFL, (long) flags | O_NONBLOCK) < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }

   return sock;
}


/*
 * age and expire TCP connections
 *
 * RETURNS: -
 */
static void tcp_expire(void) {
   time_t now;
   time_t to_limit;
   int i;
   int sts;

   time(&now);
   to_limit = now - configuration.tcp_timeout;
   
   for (i=0; i<(sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++) {
      if (sip_tcp_cache[i].fd == 0) continue;

      if (sip_tcp_cache[i].traffic_ts < to_limit) {
         /* TCP has expired, close & cleanup */
         DEBUGC(DBCLASS_NET, "TCP inactivity T/O, disconnecting: [%s] fd=%i",
                utils_inet_ntoa((&sip_tcp_cache[i].dst_addr)->sin_addr),
                sip_tcp_cache[i].fd);
         tcp_remove(i);
      } else 

      /* TCP keepalive handling */
      if ((sip_tcp_cache[i].keepalive_ts + configuration.tcp_keepalive) <= now) {
         DEBUGC(DBCLASS_NET, "sending TCP keepalive [%s:%i] fd=%i idx=%i",
                utils_inet_ntoa(sip_tcp_cache[i].dst_addr.sin_addr),
                ntohs(sip_tcp_cache[i].dst_addr.sin_port), sip_tcp_cache[i].fd, i);

         sip_tcp_cache[i].keepalive_ts = now;

         sts = send(sip_tcp_cache[i].fd, "\x0d\x0a", 2, 0);

         if (sts == -1) {
            WARN("keepalive send() failed: %s", strerror(errno));
         }
      }

   } /* for */
}


/*
 * find a TCP connection in cache
 *
 * RETURNS: index into TCP cache or -1 on not found
 */
int tcp_find(struct sockaddr_in dst_addr) {
   int i;

   /* check connection cache for an existing TCP connection */
   for (i=0; i<(sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++) {
      /* occupied entry? */
      if (sip_tcp_cache[i].fd == 0) continue;

      /* address & port match */
      if ((memcmp(&dst_addr.sin_addr, &sip_tcp_cache[i].dst_addr.sin_addr,
                    sizeof(struct in_addr)) ==0) &&
          (dst_addr.sin_port==sip_tcp_cache[i].dst_addr.sin_port)) break;
   } /* for */

   /* if no TCP connection found return -1 */
   if (i >= (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0]))) return -1;

   return i;
}


/*
 * add a TCP connection into cache
 *
 * RETURNS: index into TCP cache or -1 on failure (out of space)
 */
static int tcp_add(struct sockaddr_in addr, int fd) {
   int i;

   /* find free entry in TCP cache */
   for (i=0; i<(sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++) {
      if (sip_tcp_cache[i].fd == 0) break;
   }
   if (i >= (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0]))) {
      DEBUGC(DBCLASS_NET, "out of space in TCP cache [%s] fd=%i",
          utils_inet_ntoa(addr.sin_addr), fd);
      return -1;
   }

   /* store connection data in TCP cache */
   sip_tcp_cache[i].fd = fd;
   memcpy(&sip_tcp_cache[i].dst_addr, &addr, sizeof(struct sockaddr_in));
   time(&sip_tcp_cache[i].traffic_ts);
   sip_tcp_cache[i].keepalive_ts=sip_tcp_cache[i].traffic_ts;

   /* sanity check: must be unallocated */
   if (sip_tcp_cache[i].rx_buffer != NULL) {
      WARN("sip_tcp_cache[%i].rx_buffer was not freed! Potential memleak.", i);
      free(sip_tcp_cache[i].rx_buffer);
      sip_tcp_cache[i].rx_buffer = NULL;
   }

   /* allocate RX buffer */
   sip_tcp_cache[i].rx_buffer=malloc(BUFFER_SIZE);
   if (sip_tcp_cache[i].rx_buffer == NULL) {
      DEBUGC(DBCLASS_NET, "malloc() of %i bytes failed", BUFFER_SIZE);
      return -1;
   }
   sip_tcp_cache[i].rxbuf_size=BUFFER_SIZE;
   sip_tcp_cache[i].rxbuf_len=0;


   DEBUGC(DBCLASS_NET, "added TCP connection [%s] fd=%i to cache idx=%i",
          utils_inet_ntoa(addr.sin_addr), fd, i);

   return i;
}


/*
 * connect to a remote TCP target
 *
 * RETURNS: index into TCP cache or -1 on failure
 */
static int tcp_connect(struct sockaddr_in dst_addr) {
   int sock;
   int flags;
   int sts;
   int i;
   struct timeval timeout={0,0};
   fd_set fdset;

   /* get socket and connect to remote site */
   sock=socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (sock < 0) {
      ERROR("socket() call failed: %s",strerror(errno));
      return -1;
   }

   /* non blocking */
   flags = fcntl(sock, F_GETFL);
   if (flags < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return -1;
   }
   if (fcntl(sock, F_SETFL, (long) flags | O_NONBLOCK) < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return -1;
   }

   sts=connect(sock, (struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in));
   if ((sts == -1 ) && (errno == EINPROGRESS)) {
      /* if non-blocking connect(), wait until connection 
         successful, discarded or timeout */
      DEBUGC(DBCLASS_NET, "connection in progress, waiting %i msec to succeed",
             configuration.tcp_connect_timeout);

      /* timeout for connect */
      timeout.tv_sec  = (configuration.tcp_connect_timeout/1000);
      timeout.tv_usec = (configuration.tcp_connect_timeout%1000)*1000;
      
      do {
         /* prepare fd set */
         FD_ZERO(&fdset);
         FD_SET(sock, &fdset);
         sts = select(sock+1, NULL, &fdset, NULL, &timeout);
         if ((sts < 0) && (errno == EINTR)) {
            /* select() has been interrupted, do it again */
            continue;
         } else if (sts < 0) {
            ERROR("waiting for TCP connect failed: %s",strerror(errno));
            close(sock);
            return -1;
         } else if (sts > 0) {
            /* fd available for write */
            int valopt;
            socklen_t optlen=sizeof(valopt);

            /* get error status from delayed connect() */
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, 
                           &valopt, &optlen) < 0) {
               ERROR("getsockopt(SO_ERROR) failed: %s",strerror(errno));
               close(sock);
               return -1;
            }

            if (valopt == EINPROGRESS) {
               ERROR("connect() returned: %s",strerror(valopt));
               continue;
            }

            DEBUGC(DBCLASS_NET, "connect() completed after %i msec",
                   (int)(configuration.tcp_connect_timeout 
                   - (timeout.tv_sec*1000) 
                   - (timeout.tv_usec/1000)));
            
            /* check the returned error value from connect() */
            if (valopt) {
               ERROR("delayed TCP connect() failed : %s",strerror(errno));
               close(sock);
               return -1;
            }
            /* all went fine, continue */
            break;

         } else {
            DEBUGC(DBCLASS_NET, "tcp_connect() timeout");
            close(sock);
            return -1;
         }
      } while (1);

   } else if (sts == -1 ) {
      if ((errno != ECONNREFUSED) && (errno != ETIMEDOUT)) {
         ERROR("connect() [%s:%i] call failed: %s",
               utils_inet_ntoa(dst_addr.sin_addr),
               ntohs(dst_addr.sin_port), strerror(errno));
         close(sock);
         return -1;
      }
      DEBUGC(DBCLASS_BABBLE,"connect() [%s:%i] call failed: %s",
             utils_inet_ntoa(dst_addr.sin_addr),
             ntohs(dst_addr.sin_port), strerror(errno));
   }

   i=tcp_add(dst_addr, sock);
   if (i < 0) {
      ERROR("out of space in TCP connection cache - rejecting");
      close(sock);
      return -1;
   }

   DEBUGC(DBCLASS_NET, "connected TCP connection to [%s:%i] fd=%i",
          utils_inet_ntoa(dst_addr.sin_addr),
          ntohs(dst_addr.sin_port), sock);

   return i;
}


/*
 * clean up resources occupied by a TCP entry
 *
 * RETURNS: 0
 */
static int tcp_remove(int idx) {
   close(sip_tcp_cache[idx].fd);
   sip_tcp_cache[idx].fd=0;
   free(sip_tcp_cache[idx].rx_buffer);
   sip_tcp_cache[idx].rx_buffer=NULL;
   sip_tcp_cache[idx].rxbuf_size=0;
   sip_tcp_cache[idx].rxbuf_len=0;
   return 0;
}
