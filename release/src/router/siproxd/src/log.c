/*
    Copyright (C) 2002-2008  Thomas Ries <tries@gmx.net>

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
#include "log.h"

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <syslog.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <signal.h>

static char const ident[]="$Id: log.c 415 2009-02-28 22:02:11Z hb9xar $";

/* static functions */
static void output_to_stderr(const char *label, va_list ap, char *file,
                             int line, const char *format);
static void output_to_syslog(const char *label, int level, va_list ap,
                             char *file, int line, const char *format);
static void output_to_TCP(const char *label, va_list ap, char *file,
                          int line, const char *format);


/* module local variables */
static int log_to_stderr=0;
static unsigned int debug_pattern=0;

static int debug_listen_port=0;
static int debug_listen_fd=0;
static int debug_fd=0;

/*
 * What shall I log to syslog?
 *   0 - DEBUGs, INFOs, WARNINGs and ERRORs
 *   1 - INFOs, WARNINGs and ERRORs (this is the default)
 *   2 - WARNINGs and ERRORs
 *   3 - only ERRORs
 *   4 - absolutely nothing
 */
static int silence_level=1;

/*
 * Mutex for thread synchronization when writing log data
 *
 * use a 'fast' mutex for synchronizing - as these are portable... 
 */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_init(void) {
   openlog(NULL,LOG_NDELAY|LOG_PID,LOG_DAEMON);
}

void log_end(void) {
   closelog();
}

void log_set_pattern(unsigned int pattern) {
   debug_pattern=pattern;
   return;
}

unsigned int  log_get_pattern(void) {
   return debug_pattern;
}

void log_set_stderr(int tostdout) {
   log_to_stderr=tostdout;
   return;
}

void log_set_silence(int level) {
   silence_level=level;
   return;
}

/*
 * TCP logging
 */
void log_set_listen_port(int port){
   debug_listen_port = port;
   log_tcp_listen();
   return;
}

void log_tcp_listen(void) {
   struct sockaddr_in my_addr;
   int sts, on=1;
   int flags;

   /* disabled in configuration? */
   if (debug_listen_port == 0) {
      debug_listen_fd=-1;
      return;
   }

   /* ignore SIGPIPE of lost TCP connection */
   signal (SIGPIPE, SIG_IGN);

   memset(&my_addr, 0, sizeof(my_addr));
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(debug_listen_port);

   debug_listen_fd=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   INFO("DEBUG listener on TCP port %i",debug_listen_port);
   if (debug_listen_fd < 0) {
      ERROR("socket returned error [%i:%s]",errno, strerror(errno));
      return;
   }

   if (setsockopt(debug_listen_fd, SOL_SOCKET, SO_REUSEADDR, &on , sizeof(on)) < 0) {
      ERROR("setsockopt returned error [%i:%s]",errno, strerror(errno));
      return;
   }


   sts=bind(debug_listen_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
   if (sts != 0) {
      ERROR("bind returned error [%i:%s]",errno, strerror(errno));
      close(debug_listen_fd);
      debug_listen_fd=-1;
      return;
   }

   /* set non-blocking */
   flags = fcntl(debug_listen_fd, F_GETFL);
   if (flags < 0) {
      ERROR("fcntl returned error [%i:%s]",errno, strerror(errno));
      close(debug_listen_fd);
      debug_listen_fd=-1;
      return;
   }
   if (fcntl(debug_listen_fd, F_SETFL, (long) flags | O_NONBLOCK) < 0) {
      ERROR("fcntl returned error [%i:%s]",errno, strerror(errno));
      close(debug_listen_fd);
      debug_listen_fd=-1;
      return;
   }

   listen (debug_listen_fd, 1);
   return;
}

void log_tcp_connect(void) {
   int sts;
   fd_set fdset;
   struct timeval timeout;
   int tmpfd;

   if (debug_listen_fd <= 0) return;

   timeout.tv_sec=0;
   timeout.tv_usec=0;

   FD_ZERO(&fdset);
   FD_SET (debug_listen_fd, &fdset);

   sts=select(debug_listen_fd+1, &fdset, NULL, NULL, &timeout);
   if (sts > 0) {
      if (debug_fd != 0) {
         tmpfd=accept(debug_listen_fd, NULL, NULL);
         close(tmpfd);
         INFO("Rejected DEBUG TCP connection");
      } else {
         debug_fd=accept(debug_listen_fd, NULL, NULL);
         INFO("Accepted DEBUG TCP connection [fd=%i], debugpattern=%i",
              debug_fd, debug_pattern);
         INFO(PACKAGE"-"VERSION"-"BUILDSTR" "UNAME);
      }
   }

   /* check the TCP connection */
   if (debug_fd > 0) {
      timeout.tv_sec=0;
      timeout.tv_usec=0;

      FD_ZERO(&fdset);
      FD_SET (debug_fd, &fdset);

      sts=select(debug_fd+1, &fdset, NULL, NULL, &timeout);
      if (sts > 0) {
         char buf[32];
         sts = recv(debug_fd, buf, sizeof(buf), 0);
         /* got disconnected? */
         if (sts == 0) {
            close(debug_fd);
            INFO("Disconnected DEBUG TCP connection [fd=%i]", debug_fd);
            debug_fd=0;
         }
      }
   }
   return;
}


static void output_to_stderr(const char *label, va_list ap, char *file,
                             int line, const char *format) {
   va_list ap_copy;
   time_t t;
   struct tm *tim;

   if (!log_to_stderr) return;

   time(&t);
   tim = localtime(&t);
   fprintf(stderr, "%2.2i:%2.2i:%2.2i %s%s:%i ", tim->tm_hour,
           tim->tm_min, tim->tm_sec, label, file, line);
   va_copy(ap_copy, ap);
   vfprintf(stderr, format, ap_copy);
   va_end(ap_copy);
   fprintf(stderr, "\n");
   fflush(stderr);
   return;
}

static void output_to_syslog(const char *label, int level, va_list ap,
                             char *file, int line, const char *format) {
   va_list ap_copy;
   char outbuf[256];

   va_copy(ap_copy, ap);
   vsnprintf(outbuf, sizeof(outbuf), format, ap_copy);
   va_end(ap_copy);
   syslog(LOG_USER|level, "%s:%i %s%s", file, line, label, outbuf);
   return;
}

static void output_to_TCP(const char *label, va_list ap, char *file,
                          int line, const char *format) {
   va_list ap_copy;
   time_t t;
   struct tm *tim;
   char outbuf[256];
   int sts;

   if (debug_fd <= 0) return;

   time(&t);
   tim=localtime(&t);
   snprintf(outbuf, sizeof(outbuf), "%2.2i:%2.2i:%2.2i %s%s:%i ",
            tim->tm_hour, tim->tm_min, tim->tm_sec, label, file, line);
   sts=write(debug_fd, outbuf, strlen(outbuf));
   va_copy(ap_copy, ap);
   vsnprintf(outbuf, sizeof(outbuf), format, ap_copy);
   va_end(ap_copy);
   sts=write(debug_fd, outbuf, strlen(outbuf));
   snprintf(outbuf, sizeof(outbuf), "\n");
   sts=write(debug_fd, outbuf, strlen(outbuf));
   return;
}


void log_debug(unsigned int class, char *file, int line, const char *format, ...) {
   va_list ap;

   if ((debug_pattern & class) == 0) return;

   va_start(ap, format);
   pthread_mutex_lock(&log_mutex);

   output_to_stderr("", ap, file, line, format);

   if (!log_to_stderr && silence_level < 1) {
      output_to_syslog("", LOG_DEBUG, ap, file, line, format);
   }

   output_to_TCP("", ap, file, line, format);

   pthread_mutex_unlock(&log_mutex);
   va_end(ap);
   return;
}

void log_error(char *file, int line, const char *format, ...) {
   va_list ap;

   va_start(ap, format);
   pthread_mutex_lock(&log_mutex);

   output_to_stderr("ERROR:", ap, file, line, format);

   if (silence_level < 4) {
      output_to_syslog("ERROR:", LOG_ERR, ap, file, line, format);
   }

   output_to_TCP("ERROR:", ap, file, line, format);

   pthread_mutex_unlock(&log_mutex);
   va_end(ap);
   return;
}

void log_warn(char *file, int line, const char *format, ...) {
   va_list ap;

   va_start(ap, format);
   pthread_mutex_lock(&log_mutex);

   output_to_stderr("WARNING:", ap, file, line, format);

   if (silence_level < 3) {
      output_to_syslog("WARNING:", LOG_NOTICE, ap, file, line, format);
   }

   output_to_TCP("WARNING:", ap, file, line, format);

   pthread_mutex_unlock(&log_mutex);
   va_end(ap);
   return;
}

void log_info(char *file, int line, const char *format, ...) {
   va_list ap;

   va_start(ap, format);
   pthread_mutex_lock(&log_mutex);

   output_to_stderr("INFO:", ap, file, line, format);

   if (silence_level < 2) {
      output_to_syslog("INFO:", LOG_NOTICE, ap, file, line, format);
   }

   output_to_TCP("INFO:", ap, file, line, format);

   pthread_mutex_unlock(&log_mutex);
   va_end(ap);
   return;
 }
void log_dump_buffer(unsigned int class, char *file, int line,
                     char *buffer, int length) {
   int i, j;
   char tmp[8], tmplin1[80], tmplin2[80];
   char outbuf[256];
   int sts;

   if ((debug_pattern & class) == 0) return;
   if ((!log_to_stderr) && (debug_fd <= 0)) return;

   pthread_mutex_lock(&log_mutex);
   if (log_to_stderr) fprintf(stderr,  "---BUFFER DUMP follows---\n");
   if (debug_fd > 0) {
      snprintf(outbuf, sizeof(outbuf) ,"---BUFFER DUMP follows---\n");
      sts=write(debug_fd, outbuf, strlen(outbuf));
   }

   for (i=0; i<length; i+=16) {
      strcpy(tmplin1,"");
      strcpy(tmplin2,"");
      for (j=0;(j<16) && (i+j)<length ;j++) {
         sprintf(tmp,"%2.2x ",(unsigned char)buffer[i+j]);
         strcat(tmplin1, tmp);
         sprintf(tmp, "%c",(isprint((int)buffer[i+j]))? buffer[i+j]: '.');
         strcat(tmplin2, tmp);
      }
      if (log_to_stderr) {
         fprintf(stderr, "  %-47.47s %-16.16s\n",tmplin1, tmplin2);
      }
      if (debug_fd > 0) {
         snprintf(outbuf, sizeof(outbuf) ,"  %-47.47s %-16.16s\n",
                  tmplin1, tmplin2);
         sts=write(debug_fd, outbuf, strlen(outbuf));
      }
   }

   if (log_to_stderr) {
      fprintf(stderr,"\n---end of BUFFER DUMP---\n");
      fflush(stderr);
   }
   if (debug_fd > 0) {
      snprintf(outbuf, sizeof(outbuf) ,"---end of BUFFER DUMP---\n");
      sts=write(debug_fd, outbuf, strlen(outbuf));
   }
   pthread_mutex_unlock(&log_mutex);

   return;
}

