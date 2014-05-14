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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: security.c 443 2010-01-07 11:31:56Z hb9xar $";

/*
 * do security and integrity checks on the received packet
 * (raw buffer, \0 terminated)
 *
 * RETURNS
 *	STS_SUCCESS if ok 
 * 	STS_FAILURE if the packed did not pass the checks
 */
int security_check_raw(char *sip_buffer, size_t size) {
   char *p1=NULL, *p2=NULL;

   DEBUGC(DBCLASS_BABBLE,"security_check_raw: size=%ld", (long)size);
   /*
    * empiric: size must be >= 16 bytes
    *   2 byte <CR><LF> packets have been seen in the wild
    *   also 4 bytes with 0x00 (seem to be used to keep potential UDP
    *   masquerading tunnels open)
    */
   if (size<SEC_MINLEN) return STS_FAILURE;

   /*
    * make sure no line (up to the next CRLF) is longer than allowed
    * empiric: a line should not be longer than 256 characters
    * (libosip may die with "virtual memory exhausted" otherwise)
    * Ref: protos test suite c07-sip-r2.jar, test case 203
    * !! Contact records may all come in one single line, getting QUITE long...
    *    especially on TCP.
    */
   for (p1=sip_buffer; (p1+SEC_MAXLINELEN) < (sip_buffer+size); p1=p2+1) {
      p2=strchr(p1, 10);
      if ((p2 == 0) ||                  /* no CRLF found */
          (p2-p1) > SEC_MAXLINELEN) {   /* longer than allowed */
         DEBUGC(DBCLASS_SIP,"security_check_raw: line too long or no "
                            "CRLF found");
         return STS_FAILURE;
      }
   }


   /* As libosip2 is *VERY* sensitive to corrupt input data, we need to
      do more stuff here. For example, libosip2 can be crashed (with a
      "<port_malloc.c> virtual memory exhausted" error - God knows why)
      by sending the following few bytes. It will die in osip_message_parse()
      ---BUFFER DUMP follows---
        6e 74 2f 38 30 30 30 0d 0a 61 3d 66 6d 74 70 3a nt/8000..a=fmtp:
        31 30 31 20 30 2d 31 35 0d 0a                   101 0-15..      
      ---end of BUFFER DUMP---

      By looking at the code in osip_message_parse.c, I'd guess it is
      the 'only one space present' that leads to a faulty size
      calculation (VERY BIG NUMBER), which in turn then dies inside 
      osip_malloc.
      So, we need at least 2 spaces to survive that code part of libosip2.
    */
   p1 = strchr(sip_buffer, ' ');
   if (p1 && ((p1+1) < (sip_buffer+size))) {
      p2 = strchr(p1+1, ' ');
   } else {
         DEBUGC(DBCLASS_SIP,"security_check_raw: found no space");
         return STS_FAILURE;
   }
   if (p2==NULL) {
         DEBUGC(DBCLASS_SIP,"security_check_raw: found only one space");
         return STS_FAILURE;
   }

   /* libosip2 can be put into an endless loop by trying to parse:
      ---BUFFER DUMP follows---
        49 4e 56 49 54 45 20 20 53 49 50 2f 32 2e 30 0d INVITE  SIP/2.0.
        0a 56 69 61 3a 20 53 49 50 2f 32 2e 30 2f 55 44 .Via: SIP/2.0/UD
   Note, this is an INVITE with no valid SIP URI (INVITE  SIP/2.0)
   */
   /* clumsy... */
   if (size >20) {
   if      (strncmp(sip_buffer, "INVITE  SIP/2.0",  15)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "ACK  SIP/2.0",     12)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "BYE  SIP/2.0",     12)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "CANCEL  SIP/2.0",  15)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "REGISTER  SIP/2.0",17)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "OPTIONS  SIP/2.0", 16)==0) return STS_FAILURE;
   else if (strncmp(sip_buffer, "INFO  SIP/2.0",    13)==0) return STS_FAILURE;
   }

   /* TODO: still way to go here ... */
   return STS_SUCCESS;
}


/*
 * do security and integrity checks on the received packet
 * (parsed buffer)
 *
 * RETURNS
 *	STS_SUCCESS if ok 
 * 	STS_FAILURE if the packed did not pass the checks
 */
int security_check_sip(sip_ticket_t *ticket){
   osip_message_t *sip=ticket->sipmsg;
   if (MSG_IS_REQUEST(sip)) {
      /* check for existing SIP URI in request */
      if ((sip->req_uri == NULL) || (sip->req_uri->scheme == NULL)) {
         ERROR("security check failed: NULL SIP URI");
         return STS_FAILURE;
      }

      /* check SIP URI scheme */
      if (osip_strcasecmp(sip->req_uri->scheme, "sip")) {
         ERROR("security check failed: unknown scheme: %s",
               sip->req_uri->scheme);
         return STS_FAILURE;
      }
   }

   /*
    * Check existence of mandatory headers
    *

Rosenberg, et. al.          Standards Track                   [Page 161]

RFC 3261            SIP: Session Initiation Protocol           June 2002

      Header field          where   proxy ACK BYE CAN INV OPT REG
      ___________________________________________________________
      Accept                  R            -   o   -   o   m*  o
      Accept                 2xx           -   -   -   o   m*  o
      Accept                 415           -   c   -   c   c   c
      Accept-Encoding         R            -   o   -   o   o   o
      Accept-Encoding        2xx           -   -   -   o   m*  o
      Accept-Encoding        415           -   c   -   c   c   c
      Accept-Language         R            -   o   -   o   o   o
      Accept-Language        2xx           -   -   -   o   m*  o
      Accept-Language        415           -   c   -   c   c   c
      Alert-Info              R      ar    -   -   -   o   -   -
      Alert-Info             180     ar    -   -   -   o   -   -
      Allow                   R            -   o   -   o   o   o
      Allow                  2xx           -   o   -   m*  m*  o
      Allow                   r            -   o   -   o   o   o
      Allow                  405           -   m   -   m   m   m
      Authentication-Info    2xx           -   o   -   o   o   o
      Authorization           R            o   o   o   o   o   o
      Call-ID                 c       r    m   m   m   m   m   m
      Call-Info                      ar    -   -   -   o   o   o
      Contact                 R            o   -   -   m   o   o
      Contact                1xx           -   -   -   o   -   -
      Contact                2xx           -   -   -   m   o   o
      Contact                3xx      d    -   o   -   o   o   o
      Contact                485           -   o   -   o   o   o
      Content-Disposition                  o   o   -   o   o   o
      Content-Encoding                     o   o   -   o   o   o
      Content-Language                     o   o   -   o   o   o
      Content-Length                 ar    t   t   t   t   t   t
      Content-Type                         *   *   -   *   *   *
      CSeq                    c       r    m   m   m   m   m   m
      Date                            a    o   o   o   o   o   o
      Error-Info           300-699    a    -   o   o   o   o   o
      Expires                              -   -   -   o   -   o
      From                    c       r    m   m   m   m   m   m
      In-Reply-To             R            -   -   -   o   -   -
      Max-Forwards            R      amr   m   m   m   m   m   m
      Min-Expires            423           -   -   -   -   -   m
      MIME-Version                         o   o   -   o   o   o
      Organization                   ar    -   -   -   o   o   o

             Table 2: Summary of header fields, A--O






Rosenberg, et. al.          Standards Track                   [Page 162]

RFC 3261            SIP: Session Initiation Protocol           June 2002


   Header field              where       proxy ACK BYE CAN INV OPT REG
   ___________________________________________________________________
   Priority                    R          ar    -   -   -   o   -   -
   Proxy-Authenticate         407         ar    -   m   -   m   m   m
   Proxy-Authenticate         401         ar    -   o   o   o   o   o
   Proxy-Authorization         R          dr    o   o   -   o   o   o
   Proxy-Require               R          ar    -   o   -   o   o   o
   Record-Route                R          ar    o   o   o   o   o   -
   Record-Route             2xx,18x       mr    -   o   o   o   o   -
   Reply-To                                     -   -   -   o   -   -
   Require                                ar    -   c   -   c   c   c
   Retry-After          404,413,480,486         -   o   o   o   o   o
                            500,503             -   o   o   o   o   o
                            600,603             -   o   o   o   o   o
   Route                       R          adr   c   c   c   c   c   c
   Server                      r                -   o   o   o   o   o
   Subject                     R                -   -   -   o   -   -
   Supported                   R                -   o   o   m*  o   o
   Supported                  2xx               -   o   o   m*  m*  o
   Timestamp                                    o   o   o   o   o   o
   To                        c(1)          r    m   m   m   m   m   m
   Unsupported                420               -   m   -   m   m   m
   User-Agent                                   o   o   o   o   o   o
   Via                         R          amr   m   m   m   m   m   m
   Via                        rc          dr    m   m   m   m   m   m
   Warning                     r                -   o   o   o   o   o
   WWW-Authenticate           401         ar    -   m   -   m   m   m
   WWW-Authenticate           407         ar    -   o   -   o   o   o

*/


  /*
   * => Mandatory for ALL requests and responses
   * Call-ID                 c       r    m   m   m   m   m   m
   * CSeq                    c       r    m   m   m   m   m   m
   * From                    c       r    m   m   m   m   m   m
   * To                      c(1)    r    m   m   m   m   m   m
   * Via                     R      amr   m   m   m   m   m   m
   */

  /* check for existing Call-ID header */
   if ((sip->call_id==NULL)||
       ((sip->call_id->number==NULL)&&(sip->call_id->host==NULL))) {
      ERROR("security check failed: NULL Call-Id Header");
      return STS_FAILURE;
   }

  /* check for existing CSeq header */
   if ((sip->cseq==NULL)||
       (sip->cseq->method==NULL)||(sip->cseq->number==NULL)) {
      ERROR("security check failed: NULL CSeq Header");
      return STS_FAILURE;
   }

   /* check for existing To: header */
   if ((sip->to==NULL)||
       (sip->to->url==NULL)||(sip->to->url->host==NULL)) {
      ERROR("security check failed: NULL To Header");
      return STS_FAILURE;
   }

  /* check for existing From: header */
   if ((sip->from==NULL)||
       (sip->from->url==NULL)||(sip->from->url->host==NULL)) {
      ERROR("security check failed: NULL From Header");
      return STS_FAILURE;
   }



   /* TODO: still way to go here ... */
   return STS_SUCCESS;
}
