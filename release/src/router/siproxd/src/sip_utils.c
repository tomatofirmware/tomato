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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <pwd.h>

#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_md5.h>

#include "siproxd.h"
#include "digcalc.h"
#include "log.h"

static char const ident[]="$Id: sip_utils.c 467 2011-01-09 13:10:27Z hb9xar $";


/* configuration storage */
extern struct siproxd_config configuration;

extern int h_errno;

extern struct urlmap_s urlmap[];		/* URL mapping table     */


/*
 * create a reply template from an given SIP request
 *
 * RETURNS a pointer to osip_message_t
 */
osip_message_t *msg_make_template_reply (sip_ticket_t *ticket, int code) {
   osip_message_t *request=ticket->sipmsg;
   osip_message_t *response;
   int pos;

   if (request->to==NULL) {
      ERROR("msg_make_template_reply: empty To in request header");
      return NULL;
   }

   if (request->from==NULL) {
      ERROR("msg_make_template_reply: empty From in request header");
      return NULL;
   }

   osip_message_init (&response);
   if (response == NULL) {
      ERROR("msg_make_template_reply: osip_message_init() failed");
      return NULL;
   }
   response->message=NULL;
   osip_message_set_version (response, osip_strdup ("SIP/2.0"));
   osip_message_set_status_code (response, code);
   osip_message_set_reason_phrase (response, 
                                   osip_strdup(osip_message_get_reason (code)));


   osip_to_clone (request->to, &response->to);
   osip_from_clone (request->from, &response->from);

   /* if 3xx, also include 1st contact header */
   if ((code==200) || ((code>=300) && (code<400))) {
      osip_contact_t *req_contact = NULL;
      osip_contact_t *res_contact = NULL;
      osip_message_get_contact(request, 0, &req_contact);
      if (req_contact) osip_contact_clone (req_contact, &res_contact);
      if (res_contact) osip_list_add(&(response->contacts),res_contact,0);
   }

   /* via headers */
   pos = 0;
   while (!osip_list_eol (&(request->vias), pos)) {
      char *tmp;
      osip_via_t *via;
      via = (osip_via_t *) osip_list_get (&(request->vias), pos);
      osip_via_to_str (via, &tmp);

      osip_message_set_via (response, tmp);
      osip_free (tmp);
      pos++;
   }

   osip_call_id_clone(request->call_id,&response->call_id);
   
   osip_cseq_clone(request->cseq,&response->cseq);

   return response;
}


/*
 * check for a via loop.
 * It checks for the presense of a via entry that holds one of
 * my IP addresses and is *not* the topmost via.
 *
 * RETURNS
 *	STS_TRUE if loop detected
 *	STS_FALSE if no loop
 */
int check_vialoop (sip_ticket_t *ticket) {
/*
!!! actually this is a problematic one.
1) for requests, I must search the whole VIA list
   (topmost via is the previos station in the path)

2) for responses I must skip the topmost via, as this is mine
   (and will be removed later on)

3) What happens if we have 'clashes'  with private addresses??
   From that point of view, siproxd *should* not try to
   check against it's local IF addresses if they are private.
   this then of course again can lead to a endless loop...

   -> Might use a fixed unique part of branch parameter to identify
   that it is MY via
   
*/
   osip_message_t *my_msg=ticket->sipmsg;
   int sts;
   int pos;
   int found_own_via;

   found_own_via=0;
   pos = 1;	/* for detecting a loop, don't check the first entry 
   		   as this is my own VIA! */
   while (!osip_list_eol (&(my_msg->vias), pos)) {
      osip_via_t *via;
      via = (osip_via_t *) osip_list_get (&(my_msg->vias), pos);
      sts = is_via_local (via);
      if (sts == STS_TRUE) found_own_via+=1;
      pos++;
   }

   /*
    * what happens if a message is coming back to me legally?
    *  UA1 -->--\       /-->--\
    *            siproxd       Registrar
    *  UA2 --<--/       \--<--/
    *
    * This may also lead to a VIA loop - so I probably must take the branch
    * parameter into count (or a unique part of it) OR just allow at least 2
    * vias of my own.
    */
   return (found_own_via>2)? STS_TRUE : STS_FALSE;
}


/*
 * check if a given osip_via_t is local. I.e. its address is owned
 * by my inbound or outbound interface (or externally defined using
 * the host_outbound directive
 *
 * RETURNS
 *	STS_TRUE if the given VIA is one of my interfaces
 *	STS_FALSE otherwise
 */
int is_via_local (osip_via_t *via) {
   int sts, found;
   struct in_addr addr_via, addr_myself;
   int port;
   int i;

   if (via==NULL) {
      ERROR("called is_via_local with NULL via");
      return STS_FALSE;
   }

   DEBUGC(DBCLASS_BABBLE,"via name %s",via->host);
   if (utils_inet_aton(via->host,&addr_via) == 0) {
      /* need name resolution */
      sts=get_ip_by_host(via->host, &addr_via);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_DNS, "is_via_local: cannot resolve VIA [%s]",
                via->host);
         return STS_FAILURE;
      }
   }   

   if (via->port) {
      port=atoi(via->port);
      if ((port<=0) || (port>65535)) port=SIP_PORT;
   } else {
      port=SIP_PORT;
   }

   found=0;
   for (i=0; i<3; i++) {
      if (i < 2) {
         /* search my in/outbound interfaces */
         DEBUGC(DBCLASS_BABBLE,"resolving IP of interface %s",
                (i==IF_INBOUND)? "inbound":"outbound");
         if (get_interface_ip(i, &addr_myself) != STS_SUCCESS) {
            continue;
         }
      } else {
         /* check against a possible defined 'host_outbound' */
         /* defined? */
         if (configuration.outbound_host == NULL) continue;
         DEBUGC(DBCLASS_BABBLE,"resolving IP of interface 'host_outbound'");
         /* lookup */
         if (get_ip_by_host(configuration.outbound_host,
                            &addr_myself) != STS_SUCCESS) continue;
      }

      /* check the extracted VIA against my own host addresses */
      if ( (memcmp(&addr_myself, &addr_via, sizeof(addr_myself))==0) &&
           (port == configuration.sip_listen_port) ) {
         DEBUG("got address match [%s]", utils_inet_ntoa(addr_via));
         found=1;
	 break;
      }
   }

   return (found)? STS_TRUE : STS_FALSE;
}


/*
 * compares two URLs
 * (by now, only scheme, hostname and username are compared)
 *
 * RETURNS
 *	STS_SUCCESS if equal
 *	STS_FAILURE if non equal or error
 */
int compare_url(osip_uri_t *url1, osip_uri_t *url2) {
   int sts1, sts2;
   struct in_addr addr1, addr2;

   /* sanity checks */
   if ((url1 == NULL) || (url2 == NULL)) {
      ERROR("compare_url: NULL ptr: url1=0x%p, url2=0x%p",url1, url2);
      return STS_FAILURE;
   }

   /* sanity checks: host part is a MUST */
   if ((url1->host == NULL) || (url2->host == NULL)) {
      ERROR("compare_url: NULL ptr: url1->host=0x%p, url2->host=0x%p",
            url1->host, url2->host);
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_PROXY, "comparing urls: %s:%s@%s -> %s:%s@%s",
         (url1->scheme)   ? url1->scheme :   "(null)",
         (url1->username) ? url1->username : "(null)",
         (url1->host)     ? url1->host :     "(null)",
         (url2->scheme)   ? url2->scheme :   "(null)",
         (url2->username) ? url2->username : "(null)",
         (url2->host)     ? url2->host :     "(null)");

   /* compare SCHEME (if present) case INsensitive */
   if (url1->scheme && url2->scheme) {
      if (osip_strcasecmp(url1->scheme, url2->scheme) != 0) {
         DEBUGC(DBCLASS_PROXY, "compare_url: scheme mismatch");
         return STS_FAILURE;
      }
   } else {
      WARN("compare_url: NULL scheme - ignoring");
   }

   /* compare username (if present) case sensitive */
   if (url1->username && url2->username) {
      if (strcmp(url1->username, url2->username) != 0) {
         DEBUGC(DBCLASS_PROXY, "compare_url: username mismatch");
         return STS_FAILURE;
      }
   } else {
      DEBUGC(DBCLASS_PROXY, "compare_url: NULL username - ignoring");
   }


   /*
    * now, try to resolve the host. If resolveable, compare
    * IP addresses - if not resolveable, compare the host names
    * itselfes
    */

   /* get the IP addresses from the (possible) hostnames */
   sts1=get_ip_by_host(url1->host, &addr1);
   if (sts1 == STS_FAILURE) {
      DEBUGC(DBCLASS_PROXY, "compare_url: cannot resolve host [%s]",
             url1->host);
   }

   sts2=get_ip_by_host(url2->host, &addr2);
   if (sts2 == STS_FAILURE) {
      DEBUGC(DBCLASS_PROXY, "compare_url: cannot resolve host [%s]",
             url2->host);
   }

   if ((sts1 == STS_SUCCESS) && (sts2 == STS_SUCCESS)) {
      /* compare IP addresses */
      if (memcmp(&addr1, &addr2, sizeof(addr1))!=0) {
         DEBUGC(DBCLASS_PROXY, "compare_url: IP mismatch");
         return STS_FAILURE;
      }
   } else {
      /* compare hostname strings case INsensitive */
      if (osip_strcasecmp(url1->host, url2->host) != 0) {
         DEBUGC(DBCLASS_PROXY, "compare_url: host name mismatch");
         return STS_FAILURE;
      }
   }

   /* the two URLs did pass all tests successfully - MATCH */
   return STS_SUCCESS;
}


/*
 * compares two Call IDs
 * (by now, only hostname and username are compared)
 *
 * RETURNS
 *	STS_SUCCESS if equal
 *	STS_FAILURE if non equal or error
 */
int compare_callid(osip_call_id_t *cid1, osip_call_id_t *cid2) {

   if ((cid1==0) || (cid2==0)) {
      ERROR("compare_callid: NULL ptr: cid1=0x%p, cid2=0x%p",cid1, cid2);
      return STS_FAILURE;
   }

   /*
    * Check number part: if present must be equal, 
    * if not present, must be not present in both cids
    */
   if (cid1->number && cid2->number) {
      /* have both numbers */
      if (strcmp(cid1->number, cid2->number) != 0) goto mismatch;
   } else {
      /* at least one number missing, make sure that both are empty */
      if ( (cid1->number && (cid1->number[0]!='\0')) ||
           (cid2->number && (cid2->number[0]!='\0'))) {
         goto mismatch;
      }
   }

   /*
    * Check host part: if present must be equal, 
    * if not present, must be not present in both cids
    */
   if (cid1->host && cid2->host) {
      /* have both hosts */
      if (strcasecmp(cid1->host, cid2->host) != 0) goto mismatch;
   } else {
      /* at least one host missing, make sure that both are empty */
      if ( (cid1->host && (cid1->host[0]!='\0')) ||
           (cid2->host && (cid2->host[0]!='\0'))) {
         goto mismatch;
      }
   }

   DEBUGC(DBCLASS_BABBLE, "comparing callid - matched: "
          "%s@%s <-> %s@%s",
          cid1->number, cid1->host, cid2->number, cid2->host);
   return STS_SUCCESS;

mismatch:
   DEBUGC(DBCLASS_BABBLE, "comparing callid - mismatch: "
          "%s@%s <-> %s@%s",
          cid1->number, cid1->host, cid2->number, cid2->host);
   return STS_FAILURE;
}


/*
 * check if a given request is addressed to local. I.e. it is addressed
 * to the proxy itself (IP of my inbound or outbound interface, same port)
 *
 * RETURNS
 *	STS_TRUE if the request is addressed local
 *	STS_FALSE otherwise
 */
int is_sipuri_local (sip_ticket_t *ticket) {
   osip_message_t *sip=ticket->sipmsg;
   int found;
   struct in_addr addr_uri, addr_myself;
   int port;
   int i, sts;

   if (sip==NULL) {
      ERROR("called is_sipuri_local with NULL sip");
      return STS_FALSE;
   }

   if (!sip || !sip->req_uri) {
      ERROR("is_sipuri_local: no request URI present");
      return STS_FALSE;
   }

   DEBUGC(DBCLASS_DNS,"check for local SIP URI %s:%s",
          sip->req_uri->host? sip->req_uri->host : "*NULL*",
          sip->req_uri->port? sip->req_uri->port : "*NULL*");

   if (utils_inet_aton(sip->req_uri->host, &addr_uri) == 0) {
      /* need name resolution */
      sts=get_ip_by_host(sip->req_uri->host, &addr_uri);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "sip_gen_response: cannot resolve request uri [%s]",
                sip->req_uri->host);
         return STS_FALSE;
      }
   }   

   found=0;
   for (i=0; i<2; i++) {
      /*
       * search my in/outbound interfaces
       */
      DEBUGC(DBCLASS_BABBLE,"resolving IP of interface %s",
             (i==IF_INBOUND)? "inbound":"outbound");
      if (get_interface_ip(i, &addr_myself) != STS_SUCCESS) {
         continue;
      }

      /* check the extracted HOST against my own host addresses */
      if (sip->req_uri->port) {
         port=atoi(sip->req_uri->port);
         if ((port<=0) || (port>65535)) port=SIP_PORT;
      } else {
         port=SIP_PORT;
      }

      if ( (memcmp(&addr_myself, &addr_uri, sizeof(addr_myself))==0) &&
           (port == configuration.sip_listen_port) ) {
         DEBUG("address match [%s]", utils_inet_ntoa(addr_uri));
         found=1;
	 break;
      }
   }

   DEBUGC(DBCLASS_DNS, "SIP URI is %slocal", found? "":"not ");
   return (found)? STS_TRUE : STS_FALSE;
}


/*
 * SIP_GEN_RESPONSE
 *
 * send an proxy generated response back to the client.
 * Only errors are reported from the proxy itself.
 *  code =  SIP result code to deliver
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sip_gen_response(sip_ticket_t *ticket, int code) {
   osip_message_t *response;
   int sts;
   osip_via_t *via;
   int port;
   char *buffer;
   size_t buflen;
   struct in_addr addr;

   /* create the response template */
   if ((response=msg_make_template_reply(ticket, code))==NULL) {
      ERROR("sip_gen_response: error in msg_make_template_reply");
      return STS_FAILURE;
   }

   /* we must check if first via has x.x.x.x address. If not, we must resolve it */
   osip_message_get_via (response, 0, &via);
   if (via == NULL)
   {
      ERROR("sip_gen_response: Cannot send response - no via field");
      osip_message_free(response);
      return STS_FAILURE;
   }


   /* name resolution */
   if (utils_inet_aton(via->host, &addr) == 0)
   {
      /* need name resolution */
      DEBUGC(DBCLASS_DNS,"resolving name:%s",via->host);
      sts = get_ip_by_host(via->host, &addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "sip_gen_response: cannot resolve via [%s]",
                via->host);
         osip_message_free(response);
         return STS_FAILURE;
      }
   }   

   sts = sip_message_to_str(response, &buffer, &buflen);
   if (sts != 0) {
      ERROR("sip_gen_response: msg_2char failed");
      osip_message_free(response);
      return STS_FAILURE;
   }


   if (via->port) {
      port=atoi(via->port);
      if ((port<=0) || (port>65535)) port=SIP_PORT;
   } else {
      port=SIP_PORT;
   }

   /* send to destination */
   sipsock_send(addr, port, ticket->protocol, buffer, buflen);

   /* free the resources */
   osip_message_free(response);
   osip_free(buffer);
   return STS_SUCCESS;
}


/*
 * SIP_ADD_MYVIA
 *
 * interface == IF_OUTBOUND, IF_INBOUND
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sip_add_myvia (sip_ticket_t *ticket, int interface) {
   struct in_addr addr;
   char tmp[URL_STRING_SIZE];
   osip_via_t *via;
   int sts;
   char branch_id[VIA_BRANCH_SIZE];
   char *myaddr;
   int  add_rport=0;

   if (get_interface_ip(interface, &addr) != STS_SUCCESS) {
      return STS_FAILURE;
   }

   /* evaluate if we need to include an ;rport */
   /* incoming */
   if ((configuration.use_rport & 1) && 
       ((ticket->direction==REQTYP_INCOMING) || 
        (ticket->direction==RESTYP_INCOMING)) ) {
        add_rport=1;
   } else if ((configuration.use_rport & 2) && 
       ((ticket->direction==REQTYP_OUTGOING) || 
        (ticket->direction==RESTYP_OUTGOING)) ) {
        add_rport=1;
   }

   sts = sip_calculate_branch_id(ticket, branch_id);

   myaddr=utils_inet_ntoa(addr);
   if (ticket->protocol == PROTO_UDP) {
      sprintf(tmp, "SIP/2.0/UDP %s:%i;branch=%s%s",
              myaddr, configuration.sip_listen_port,
              branch_id,
              (add_rport)? ";rport":"");
   } else {
      sprintf(tmp, "SIP/2.0/TCP %s:%i;branch=%s%s",
              myaddr, configuration.sip_listen_port,
              branch_id,
              (add_rport)? ";rport":"");
   }

   DEBUGC(DBCLASS_BABBLE,"adding VIA:%s",tmp);

   sts = osip_via_init(&via);
   if (sts!=0) return STS_FAILURE; /* allocation failed */

   sts = osip_via_parse(via, tmp);
   if (sts!=0) return STS_FAILURE;

   osip_list_add(&(ticket->sipmsg->vias),via,0);

   return STS_SUCCESS;
}


/*
 * SIP_DEL_MYVIA
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sip_del_myvia (sip_ticket_t *ticket) {
   osip_via_t *via;
   int sts;

   DEBUGC(DBCLASS_PROXY,"deleting topmost VIA");
   via = osip_list_get (&(ticket->sipmsg->vias), 0);
   
   if ( via == NULL ) {
      ERROR("Got empty VIA list - is your UA configured properly?");
      return STS_FAILURE;
   }

   if ( is_via_local(via) == STS_FALSE ) {
      ERROR("I'm trying to delete a VIA but it's not mine! host=%s",via->host);
      return STS_FAILURE;
   }

   sts = osip_list_remove(&(ticket->sipmsg->vias), 0);
   osip_via_free (via);
   return STS_SUCCESS;
}


/*
 * SIP_REWRITE_CONTACT
 *
 * rewrite the Contact header
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sip_rewrite_contact (sip_ticket_t *ticket, int direction) {
   osip_message_t *sip_msg=ticket->sipmsg;
   osip_contact_t *contact;
   int i, j;
   int replaced=0;

   if (sip_msg == NULL) return STS_FAILURE;

   /* at least one contact header present? */
   osip_message_get_contact(sip_msg, 0, &contact);
   if (contact == NULL) return STS_FAILURE;

   /* loop for all existing contact headers in message */
   for (j=0; contact != NULL; j++) {
      osip_message_get_contact(sip_msg, j, &contact);
      if (contact == NULL) break;
      if (contact->url == NULL) continue;

      /* search for an entry */
      for (i=0;i<URLMAP_SIZE;i++){
         if (urlmap[i].active == 0) continue;
         if ((direction == DIR_OUTGOING) &&
             (compare_url(contact->url, urlmap[i].true_url)==STS_SUCCESS)) break;
         if ((direction == DIR_INCOMING) &&
             (compare_url(contact->url, urlmap[i].masq_url)==STS_SUCCESS)) break;
      }

      /* found a mapping entry */
      if (i<URLMAP_SIZE) {
         char *tmp;

         if (direction == DIR_OUTGOING) {
            DEBUGC(DBCLASS_PROXY, "rewriting Contact header %s@%s -> %s@%s",
                   (contact->url->username)? contact->url->username : "*NULL*",
                   (contact->url->host)? contact->url->host : "*NULL*",
                   urlmap[i].masq_url->username, urlmap[i].masq_url->host);
         } else {
            DEBUGC(DBCLASS_PROXY, "rewriting Contact header %s@%s -> %s@%s",
                   (contact->url->username)? contact->url->username : "*NULL*",
                   (contact->url->host)? contact->url->host : "*NULL*",
                   urlmap[i].true_url->username, urlmap[i].true_url->host);
         }

         /* remove old entry */
         osip_list_remove(&(sip_msg->contacts),j);
         osip_contact_to_str(contact, &tmp);
         osip_contact_free(contact);

         /* clone the url from urlmap*/
         osip_contact_init(&contact);
         osip_contact_parse(contact,tmp);
         osip_free(tmp);
         osip_uri_free(contact->url);
         if (direction == DIR_OUTGOING) {
            /* outgoing, use masqueraded url */
            osip_uri_clone(urlmap[i].masq_url, &contact->url);
         } else {
            /* incoming, use true url */
            osip_uri_clone(urlmap[i].true_url, &contact->url);
         }

         /* add transport=tcp parameter if TCP */
         if (ticket->protocol == PROTO_TCP) {
            osip_uri_set_transport_tcp(contact->url);
         }

         osip_list_add(&(sip_msg->contacts),contact,j);
         replaced=1;
      }

   } /* for j */

   if (replaced == 0) {
      DEBUGC(DBCLASS_PROXY, "no Contact header rewritten!");
      return STS_FAILURE;
   }

   return STS_SUCCESS;
}


/*
 * SIP_CALCULATE_BRANCH
 *
 * Calculates a branch parameter according to RFC3261 section 16.11
 *
 * The returned 'id' will be HASHHEXLEN + strlen(magic_cookie)
 * characters (32 + 7) long. The caller must supply at least this
 * amount of space in 'id'.
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int  sip_calculate_branch_id (sip_ticket_t *ticket, char *id) {
/* RFC3261 section 16.11 recommends the following procedure:
 *   The stateless proxy MAY use any technique it likes to guarantee
 *   uniqueness of its branch IDs across transactions.  However, the
 *   following procedure is RECOMMENDED.  The proxy examines the
 *   branch ID in the topmost Via header field of the received
 *   request.  If it begins with the magic cookie, the first
 *   component of the branch ID of the outgoing request is computed
 *   as a hash of the received branch ID.  Otherwise, the first
 *   component of the branch ID is computed as a hash of the topmost
 *   Via, the tag in the To header field, the tag in the From header
 *   field, the Call-ID header field, the CSeq number (but not
 *   method), and the Request-URI from the received request.  One of
 *   these fields will always vary across two different
 *   transactions.
 *
 * The branch value will consist of:
 * - magic cookie "z9hG4bK"
 * - 1st part (unique calculated ID
 * - 2nd part (value for loop detection) <<- not yet used by siproxd
 */
   osip_message_t *sip_msg=ticket->sipmsg;
   static char *magic_cookie="z9hG4bK";
   osip_via_t *via;
   osip_uri_param_t *param=NULL;
   osip_call_id_t *call_id=NULL;
   HASHHEX hashstring;

   hashstring[0]='\0';

   /*
    * Examine topmost via and look for a magic cookie.
    * If it is there, I use THIS branch parameter as input for
    * our hash calculation
    */
   via = osip_list_get (&(sip_msg->vias), 0);
   if (via == NULL) {
      ERROR("have a SIP message without any via header");
      return STS_FAILURE;
   }

   param=NULL;
   osip_via_param_get_byname(via, "branch", &param);
   if (param && param->gvalue) {
      DEBUGC(DBCLASS_BABBLE, "looking for magic cookie [%s]",param->gvalue);
      if (strncmp(param->gvalue, magic_cookie,
                  strlen(magic_cookie))==0) {
         /* calculate MD5 hash */
         osip_MD5_CTX Md5Ctx;
         HASH HA1;

         osip_MD5Init(&Md5Ctx);
         osip_MD5Update(&Md5Ctx, (unsigned char*)param->gvalue,
                        strlen(param->gvalue)); 
         osip_MD5Final(HA1, &Md5Ctx);
         CvtHex(HA1, hashstring);

         DEBUGC(DBCLASS_BABBLE, "existing branch -> branch hash [%s]",
                hashstring);
      }
   }

   /*
    * If I don't have a branch parameter in the existing topmost via,
    * then I need:
    *   - the topmost via
    *   - the tag in the To header field
    *   - the tag in the From header field
    *   - the Call-ID header field
    *   - the CSeq number (but not method)
    *   - the Request-URI from the received request
    */
   if (hashstring[0] == '\0') {
      /* calculate MD5 hash */
      osip_MD5_CTX Md5Ctx;
      HASH HA1;
      char *tmp;

      osip_MD5Init(&Md5Ctx);

      /* topmost via */
      osip_via_to_str(via, &tmp);
      if (tmp) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)tmp, strlen(tmp));
         osip_free(tmp);
      }
     
      /* Tag in To header */
      osip_to_get_tag(sip_msg->to, &param);
      if (param && param->gvalue) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)param->gvalue,
	                strlen(param->gvalue));
      }

      /* Tag in From header */
      osip_from_get_tag(sip_msg->from, &param);
      if (param && param->gvalue) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)param->gvalue,
	                strlen(param->gvalue));
      }

      /* Call-ID */
      call_id = osip_message_get_call_id(sip_msg);
      osip_call_id_to_str(call_id, &tmp);
      if (tmp) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)tmp, strlen(tmp));
         osip_free(tmp);
      }

      /* CSeq number (but not method) */
      tmp = osip_cseq_get_number(sip_msg->cseq);
      if (tmp) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)tmp, strlen(tmp));
      }
 
      /* Request URI */
      osip_uri_to_str(sip_msg->req_uri, &tmp);
      if (tmp) {
         osip_MD5Update(&Md5Ctx, (unsigned char*)tmp, strlen(tmp));
         osip_free(tmp);
      }

      osip_MD5Final(HA1, &Md5Ctx);
      CvtHex(HA1, hashstring);

      DEBUGC(DBCLASS_BABBLE, "non-existing branch -> branch hash [%s]",
             hashstring);
   }

   /* include the magic cookie */
   sprintf(id, "%s%s", magic_cookie, hashstring);

   return STS_SUCCESS;
}


/*
 * SIP_FIND_OUTBOUND_PROXY
 *
 * performs the lookup for an apropriate outbound proxy
 *
 * RETURNS
 *	STS_SUCCESS on successful lookup
 *	STS_FAILURE if no outbound proxy to be used
 */
int  sip_find_outbound_proxy(sip_ticket_t *ticket, struct in_addr *addr,
                             int *port) {
   int i, sts;
   char *domain=NULL;
   osip_message_t *sipmsg;

   sipmsg=ticket->sipmsg;

   if (!addr ||!port) {
      ERROR("sip_find_outbound_proxy called with NULL addr or port");
      return STS_FAILURE;
   }

   if (sipmsg && sipmsg->from && sipmsg->from->url) {
      domain=sipmsg->from->url->host;
   }

   if (domain == NULL) {
      WARN("sip_find_outbound_proxy called with NULL from->url->host");
      return STS_FAILURE;
   }

   /*
    * check consistency of configuration:
    * outbound_domain_name, outbound_domain_host, outbound_domain_port
    * must match up
    */
   if ((configuration.outbound_proxy_domain_name.used !=
        configuration.outbound_proxy_domain_host.used) ||
       (configuration.outbound_proxy_domain_name.used !=
        configuration.outbound_proxy_domain_port.used)) {
      ERROR("configuration of outbound_domain_ inconsistent, check config");
   } else {
      /*
       * perform the lookup for domain specific proxies
       * first match wins
       */
      for (i=0; i<configuration.outbound_proxy_domain_name.used; i++) {
         if (strcasecmp(configuration.outbound_proxy_domain_name.string[i],
             domain)==0) {
            sts = get_ip_by_host(configuration.outbound_proxy_domain_host.string[i],
                                 addr);
            if (sts == STS_FAILURE) {
               ERROR("sip_find_outbound_proxy: cannot resolve "
                     "outbound proxy host [%s], check config", 
                     configuration.outbound_proxy_domain_host.string[i]);
               return STS_FAILURE;
            }
            *port=atoi(configuration.outbound_proxy_domain_port.string[i]);
            if ((*port<=0) || (*port>65535)) *port=SIP_PORT;

            return STS_SUCCESS;

         } /* strcmp */
      } /* for i */
   }

   /*
    * now check for a global outbound proxy
    */
   if (configuration.outbound_proxy_host) {
      /* I have a global outbound proxy configured */
      sts = get_ip_by_host(configuration.outbound_proxy_host, addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve outbound "
                " proxy host [%s]", configuration.outbound_proxy_host);
         return STS_FAILURE;
      }

      if (configuration.outbound_proxy_port) {
         *port=configuration.outbound_proxy_port;
      } else {
         *port = SIP_PORT;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_request: have outbound proxy %s:%i",
             configuration.outbound_proxy_host, *port);

      return STS_SUCCESS;
   }

   return STS_FAILURE; /* no proxy */
}


/*
 * SIP_IS_OUTGOING
 *
 * Figures out if this is an outgoing or incoming request/response.
 * The direction is stored in the ticket->direction property.
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE if unable to determine
 */
int  sip_find_direction(sip_ticket_t *ticket, int *urlidx) {
   int type;
   int i, sts;
   struct sockaddr_in *from;
   osip_message_t *request;
   osip_message_t *response;
   struct in_addr tmp_addr, tmp_addr2;

   from=&ticket->from;
   request=ticket->sipmsg;
   response=ticket->sipmsg;
   type = DIRTYP_UNKNOWN;

   ticket->direction = DIRTYP_UNKNOWN;

   /* Search order is as follows:
    * - "from" IP address one of our registered local UAs
    * - "To:" SIP header (Request) directed to internal UA
    *   or
    *   "From:" SIP header (Response) coming from internal UA
    * - SIP URI matches one of the registered local UAs
    * - for Responses: check for bottommost "Via:" header to be
    *   one of our registered local UA IPs
    * - "from" IP == 127.0.0.1 || inbound_IP || outbound IP 
    *
    * The first successful match is taken.
    */

   /*
    * did I receive the telegram from a REGISTERED host?
    * -> it must be an OUTGOING request/response
    */
   for (i=0; i<URLMAP_SIZE; i++) {
      if (urlmap[i].active == 0) continue;
      if (get_ip_by_host(urlmap[i].true_url->host, &tmp_addr) == STS_FAILURE) {
         DEBUGC(DBCLASS_SIP, "sip_find_direction: cannot resolve host [%s]",
             urlmap[i].true_url->host);
      } else {
         DEBUGC(DBCLASS_SIP, "sip_find_direction: reghost:%s ip:%s",
                urlmap[i].true_url->host, utils_inet_ntoa(from->sin_addr));
         if (memcmp(&tmp_addr, &from->sin_addr, sizeof(tmp_addr)) == 0) {
            if (MSG_IS_REQUEST(ticket->sipmsg)) {
               type=REQTYP_OUTGOING;
            } else {
               type=RESTYP_OUTGOING;
            }
            break;
         }
      }
   }


   /*
    * is the telegram directed to an internally registered host?
    * -> likely to be an INCOMING request/response
    * check for a match on the To: header  first
    */
   if (type == DIRTYP_UNKNOWN) {
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) continue;
         /* RFC3261:
          * 'To' contains a display name (Bob) and a SIP or SIPS URI
          * (sip:bob@biloxi.com) towards which the request was originally
          * directed.  Display names are described in RFC 2822 [3].
          */

         /* So this means, that we must check the SIP URI supplied with the
          * INVITE method, as this points to the real wanted target.
          * First we will try to match on the To: and From: headers
          * If nothing is found here, we try again with SIP URI futher down.
          */

         if (MSG_IS_REQUEST(ticket->sipmsg)) {
            /* REQUEST */
            /* incoming request ('to' == 'masq') || (('to' == 'reg') && !REGISTER)*/
            if ((compare_url(request->to->url, urlmap[i].masq_url)==STS_SUCCESS) ||
                (!MSG_IS_REGISTER(request) &&
                 (compare_url(request->to->url, urlmap[i].reg_url)==STS_SUCCESS))) {
               type=REQTYP_INCOMING;
               break;
            }
         } else { 
            /* RESPONSE */
            /* incoming response ('from' == 'masq') || ('from' == 'reg') */
            if ((compare_url(response->from->url, urlmap[i].reg_url)==STS_SUCCESS) ||
                (compare_url(response->from->url, urlmap[i].masq_url)==STS_SUCCESS)) {
               type=RESTYP_INCOMING;
               break;
            }
         } /* is request */
      } /* for i */
   } /* if type == DIRTYP_UNKNOWN */


   /*
    * nothing found yet? 
    * -> likely to be an INCOMING request/response
    * check for a match on the SIP URI (requests only)
    */
   if ((type == DIRTYP_UNKNOWN) && (MSG_IS_REQUEST(ticket->sipmsg))) {
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) continue;
         /* incoming request (SIP URI == 'masq') || ((SIP URI == 'reg') && !REGISTER)*/
         if ((compare_url(request->req_uri, urlmap[i].masq_url)==STS_SUCCESS) ||
             (!MSG_IS_REGISTER(request) &&
              (compare_url(request->req_uri, urlmap[i].reg_url)==STS_SUCCESS))) {
            type=REQTYP_INCOMING;
            break;
         }
      } /* for i */
   } /* if type == DIRTYP_UNKNOWN */


   /* &&&& Open Issue &&&&
    * it has been seen with cross-provider calls that the FROM may be 'garbled'
    * (e.g 1393xxx@proxy01.sipphone.com for calls made sipphone -> FWD)
    * How can we deal with this? Should I take into consideration the 'Via'
    * headers? This is the only clue I have, pointing to the *real* UA.
    * Maybe I should put in a 'siproxd' ftag value to recognize it as a header
    * inserted by myself
    */
   if ((type == DIRTYP_UNKNOWN) && 
       (!osip_list_eol(&(response->vias), 0))) {
      if (MSG_IS_RESPONSE(ticket->sipmsg)) {
         osip_via_t *via;
         struct in_addr addr_via, addr_myself;
         int port_via, port_ua;

         /* get the via address */
         via = (osip_via_t *) osip_list_get (&(response->vias), 0);
         DEBUGC(DBCLASS_SIP, "sip_find_direction: check via [%s] for "
                "registered UA",via->host);
         sts=get_ip_by_host(via->host, &addr_via);
         if (sts == STS_FAILURE) {
            DEBUGC(DBCLASS_SIP, "sip_find_direction: cannot resolve VIA [%s]",
                   via->host);
         } else {

            for (i=0; i<URLMAP_SIZE; i++) {
               if (urlmap[i].active == 0) continue;
               /* incoming response (1st via in list points to a registered UA) */
               sts=get_ip_by_host(urlmap[i].true_url->host, &addr_myself);
               if (sts == STS_FAILURE) {
                  DEBUGC(DBCLASS_SIP, "sip_find_direction: cannot resolve "
                         "true_url [%s]", via->host);
                  continue;
               }

               port_via=0;
               if (via->port) port_via=atoi(via->port);
               if ((port_via<=0) || (port_via>65535)) port_via=SIP_PORT;

               port_ua=0;
               if (urlmap[i].true_url->port)
                  port_ua=atoi(urlmap[i].true_url->port);
               if ((port_ua<=0) || (port_ua>65535)) port_ua=SIP_PORT;

               DEBUGC(DBCLASS_SIP, "sip_find_direction: checking for registered "
                      "host [%s:%i] <-> [%s:%i]",
                      urlmap[i].true_url->host, port_ua,
                      via->host, port_via);

               if ((memcmp(&addr_myself, &addr_via, sizeof(addr_myself))==0) &&
                   (port_via == port_ua)) {
                  type=RESTYP_INCOMING;
                  break;
               }
            } /* for i */
         }
      } /* is response */
   } /* if type == DIRTYP_UNKNOWN */


   /*
    * if the telegram is received from 127.0.0.1 use my inbound IP as sender,
    * this likely is a locally REDIRECTED/DNATed (by iptables) packet.
    * So it is a local UA.
    * Also, my own outbound address is considered to be redirected traffic
    * Example Scenario:
    * Softphone(or PBX) running on the same host as siproxd is running.
    * Using iptables, you do a REDIRECT of outgoing SIP traffic of the
    * PBX to be passed to siproxd.
    */
   if (type == DIRTYP_UNKNOWN) {
      sts=get_interface_ip(IF_INBOUND, &tmp_addr);
      sts=get_interface_ip(IF_OUTBOUND, &tmp_addr2);
      if ((htonl(from->sin_addr.s_addr) == INADDR_LOOPBACK) ||
	  (from->sin_addr.s_addr == tmp_addr.s_addr) ||
	  (from->sin_addr.s_addr == tmp_addr2.s_addr)) {
	 if (MSG_IS_REQUEST(ticket->sipmsg)) {
            type=REQTYP_OUTGOING;
	 } else {
            type=RESTYP_OUTGOING;
	 }
      }
   } /* if type == DIRTYP_UNKNOWN */


   if (type == DIRTYP_UNKNOWN) {
      DEBUGC(DBCLASS_SIP, "sip_find_direction: unable to determine "
                          "direction of SIP packet");
      return STS_FAILURE;
   }

   ticket->direction=type;
   if (urlidx) *urlidx=i;
   return STS_SUCCESS;
}


/*
 * SIP_FIXUP_ALERT_INFO
 *
 * Asterisk Hack.
 * Asterisk seems to include broken Alert-Info headers (without <>)
 * that cause parsing to fail (libosip2).
 * This function does try to guess if we have such a broken SIP
 * message and does simply remove the offending Alert-Info header.
 *
 * RETURNS
 *	STS_SUCCESS on success
 */
int  sip_fixup_asterisk(char *buff, size_t *buflen) {
   char *alert_info_ptr=NULL;
   /*
    * Check for Asterisk UA string
    * User-Agent: Asterisk PBX
    */
   if (strstr(buff, "\r\nUser-Agent: Asterisk PBX\r\n")==NULL)
      return STS_SUCCESS;

   DEBUGC(DBCLASS_SIP, "sip_fixup_alert_info: SIP message is from Asterisk");
   /*
    * Look form Alert-Info: header
    */
   alert_info_ptr=strstr(buff, "\r\nAlert-Info: ");
   if (alert_info_ptr) {
      char *eol_ptr=NULL;
      DEBUGC(DBCLASS_SIP, "sip_fixup_alert_info: Asterisk message "
             "with Alert-Info found");
      eol_ptr=strstr((alert_info_ptr+2), "\r\n");
      if (eol_ptr) {
         int i;
         // cut the Alert-Info header from the message
         // (actually move the rest of the message up)
         DEBUGC(DBCLASS_SIP, "sip_fixup_alert_info: removed malformed "
                "Alert-Info header");
         for (i=0; i<(*buflen - (eol_ptr - buff)); i++) {
            alert_info_ptr[i]=eol_ptr[i];
         } /* for */

         *buflen=strlen(buff);
      } /* if */
   }
   return STS_SUCCESS;
}


/*
 * SIP_OBSCURE_CALLID
 *
  27-May-2007:  - new feature: "Obscure Loops" does modify the Call-IDs in
                  outgoing requests and thus allows incoming calls forked
                  off such an outgoing call (redirect, transfer, ...) back
                  to the same UA where the initial call did originate.
                  This even seems to fix some issues with Asterisks
                  Loop detection... :-)
Needs more thinking. The logic about call termination is quite tricky -
calls can be terminated from either side... It is not stratigh forward
when and how to modify CIDs for call cancellation...
 * ...
 *
 * RETURNS
 *	STS_SUCCESS on success
 */
int  sip_obscure_callid(sip_ticket_t *ticket) {
   static char myident[]="-siproxd";
   int myidentlen=sizeof(myident)-1; /* a static strlen() of myident */

   osip_message_t *sipmsg;
   osip_from_t *from;
   osip_to_t *to;
   osip_uri_param_t *fromtag, *totag; /* gname, gvalue */

   osip_call_id_t *CID=NULL;

   char *tmp, *tmp2;


   /* feature enabled? */
   if (!configuration.obscure_loops) return STS_SUCCESS;

   sipmsg=ticket->sipmsg;


      if (MSG_IS_REQUEST(sipmsg) &&
          MSG_IS_REGISTER(sipmsg)) return STS_SUCCESS;
      if (MSG_IS_RESPONSE(sipmsg) &&
          MSG_IS_RESPONSE_FOR(sipmsg,"REGISTER")) return STS_SUCCESS;

   from=sipmsg->from;
   osip_from_get_tag(from, &fromtag);

   to=sipmsg->to;
   osip_to_get_tag(to, &totag);

   /* at lest the From Tag must be present, otherwise out Logic does
      not work. Is MANDATORY according to RFC3261 */
   if (!fromtag || !fromtag->gvalue || strlen(fromtag->gvalue)==0) {
      WARN("sip_obscure_callid: no From-Tag, not RFC3261 conform!");
      return STS_FAILURE;
   }

   CID=osip_message_get_call_id(sipmsg);

   /* does CID->number exist? According to RFC3261, this part is mandatory */
   if (!CID || !CID->number) {
      WARN("sip_obscure_callid: invalid Call-ID received, not RFC3261 conform!");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_PROXY, "sip_obscure_callid: current Callid#=%s Direction=%i",
                         CID->number, ticket->direction);

   switch (ticket->direction) {

   /* Outgoing Request */
   case REQTYP_OUTGOING:
   /* also need testing for "normal" incoming call that is present and
      now we SEND a cancel for this call */
      tmp=strstr(CID->number, myident);
      if (tmp==NULL) {
         /* no obscuring present yet, must be a new call, modify it */
         tmp=osip_malloc(strlen(CID->number) + myidentlen + 
                         strlen(fromtag->gvalue) + 1);
         sprintf(tmp,"%s%s%s",CID->number,myident,fromtag->gvalue);
         osip_free(CID->number);
         CID->number=tmp;
      } else {
         /* Obscuring is present, do nothing. */
      }
      break;

   /* Incoming Response */
   case RESTYP_INCOMING:
      tmp=strstr(CID->number,myident);
      /* modify it back if existing */
      if (tmp) {
         /* make sure to cut only the last marker - in case
            of multiple siproxd instances...
            I dont know if this actually would work (multiple instances...)*/
         for (;(tmp2=strstr(tmp+1, myident)) != NULL;) {
            tmp=tmp2;
         }
         tmp[0]='\0';
      }
      break;

   /* Incoming Request */
   case REQTYP_INCOMING:
      tmp=strstr(CID->number, myident);
      if (tmp==NULL) {
      /* no obscuring present yet, must be a new incoming call, do nothing */
      } else {
         /* if BYE or CANCEL, check if obscuring present.
          * if From-Tag different than in CID stored, modify back CID */
DEBUGC(DBCLASS_PROXY, "tmp+myidentlen=[%s], FromTag=[%s]",
                         tmp+myidentlen, fromtag->gvalue);

         if (strcmp(tmp+myidentlen, fromtag->gvalue) != 0) {
            tmp[0]='\0';
         } else { 
            /* if From-Tag equal to in CID stored, do nothing */
         }
      }
      break;

   /* Outgoing Response */
   case RESTYP_OUTGOING:
      if (MSG_IS_RESPONSE_FOR(sipmsg,"BYE")) {
      tmp=strstr(CID->number, myident);
      if ((tmp==NULL)&& totag && totag->gvalue) {
         /* no obscuring present yet, must be a new call, modify it */
         tmp=osip_malloc(strlen(CID->number) + myidentlen + 
                         strlen(fromtag->gvalue) + 1);
         sprintf(tmp,"%s%s%s",CID->number,myident,totag->gvalue);
         osip_free(CID->number);
         CID->number=tmp;
      } else {
         /* Obscuring is present, do nothing. */
      }
}
//      tmp=strstr(CID->number, myident);
//      /* modify it back if existing*/
//      if (tmp) {
//         /* make sure to cut only the last marker - in case
//            of multiple siproxd instances... */
//         for (;(tmp2=strstr(tmp+1, myident)) != NULL;) {
//            tmp=tmp2;
//         }
//         tmp[0]='\0';
//      }
      break;

   }

   DEBUGC(DBCLASS_PROXY, "sip_obscure_callid: new Callid#=%s",CID->number);

   return STS_SUCCESS;
}


/*
 * SIP_ADD_RECEIVED_PARAM
 *
 * Add a received parameter to the topmost VIA header (IP and port)
 * 
 * RETURNS
 *	STS_SUCCESS on success
 */
int sip_add_received_param(sip_ticket_t *ticket){
   osip_via_t *via;
   char tmp[6];

   DEBUGC(DBCLASS_PROXY,"adding received= param to topmost via");
   via = osip_list_get (&(ticket->sipmsg->vias), 0);
   
   /* set rport=xxx;received=1.2.3.4 */
   snprintf(tmp, sizeof(tmp), "%i", ntohs(ticket->from.sin_port));
   osip_via_param_add(via,osip_strdup("rport"),osip_strdup(tmp));

   osip_via_param_add(via,osip_strdup("received"),
                      osip_strdup(utils_inet_ntoa(ticket->from.sin_addr)));

   return STS_SUCCESS;
}


/*
 * SIP_GET_RECEIVED_PARAM
 *
 * Get a received parameter from the topmost VIA header (IP and port)
 * 
 * RETURNS
 *	STS_SUCCESS on success
 */
int  sip_get_received_param(sip_ticket_t *ticket,
                            struct in_addr *dest, int *port) {
   osip_via_t *via;
   osip_generic_param_t *received=NULL;
   osip_generic_param_t *rport=NULL;
   int sts;

   DEBUGC(DBCLASS_PROXY,"searching received= param in topmost via");
   via = osip_list_get (&(ticket->sipmsg->vias), 0);

   osip_via_param_get_byname (via, "received", &received);
   osip_via_param_get_byname (via, "rport",    &rport);

   if (received && rport && received->gvalue && rport->gvalue) {
      /* fetch the IP */
      sts = get_ip_by_host(received->gvalue, dest);
      if (sts != STS_SUCCESS) return STS_FAILURE;
      
      /* fetch the port number */
      *port = atoi(rport->gvalue);
      if ((*port <=0) || (*port >=65536)) return STS_FAILURE;

      /* If TCP, then validate first if an existing connection is in the cache.
       * If not, do not use this - a new conection must be established! */
      if (ticket->protocol == PROTO_TCP) {
         struct sockaddr_in addr;
         addr.sin_family = AF_INET;
         memcpy(&addr.sin_addr, dest, sizeof(struct in_addr));
         addr.sin_port= htons(*port);
         sts = tcp_find(addr);
         if (sts < 0) {
            DEBUGC(DBCLASS_BABBLE, "IP: %s, port: %i not found in cache",
                   utils_inet_ntoa(*dest), *port);
            return STS_FAILURE;
         }
      }

      /* found, return */
      DEBUGC(DBCLASS_BABBLE, "IP:%s, port:%i is ok to be reused",
             utils_inet_ntoa(*dest), *port);
      return STS_SUCCESS;
   }

   /* not found */
   return STS_FAILURE;
}
