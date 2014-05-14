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
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <osipparser2/osip_parser.h>
#include <osipparser2/sdp_message.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: proxy.c 456 2010-03-29 17:28:12Z hb9xar $";

/* configuration storage */
extern struct siproxd_config configuration;	/* defined in siproxd.c */

extern struct urlmap_s urlmap[];		/* URL mapping table     */
extern struct lcl_if_s local_addresses;


/*
 * PROXY_REQUEST
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 *
 * RFC3261
 *    Section 16.3: Proxy Behavior - Request Validation
 *    1. Reasonable Syntax
 *    2. URI scheme
 *    3. Max-Forwards
 *    4. (Optional) Loop Detection
 *    5. Proxy-Require
 *    6. Proxy-Authorization
 *
 *    Section 16.6: Proxy Behavior - Request Forwarding
 *    1.  Make a copy of the received request
 *    2.  Update the Request-URI
 *    3.  Update the Max-Forwards header field
 *    4.  Optionally add a Record-route header field value
 *    5.  Optionally add additional header fields
 *    6.  Postprocess routing information
 *    7.  Determine the next-hop address, port, and transport
 *    8.  Add a Via header field value
 *    9.  Add a Content-Length header field if necessary
 *    10. Forward the new request
 *    11. Set timer C
 */
int proxy_request (sip_ticket_t *ticket) {
   int i;
   int sts;
   int type;
   struct in_addr sendto_addr;
   osip_uri_t *url;
   int port;
   char *buffer;
   size_t buflen;
   osip_message_t *request;

   DEBUGC(DBCLASS_PROXY,"proxy_request");

   if (ticket==NULL) {
      ERROR("proxy_request: called with NULL ticket");
      return STS_FAILURE;
   }

   request=ticket->sipmsg;

   /*
    * RFC&&&&
    * add a received= parameter to the topmost Via header. Used for TCP
    * connections - send answer within the existing TCP connection back
    * to client.
    */
   if (ticket->protocol == PROTO_TCP) {
      sip_add_received_param(ticket);
   }

   /*
    * RFC 3261, Section 16.4
    * Proxy Behavior - Route Information Preprocessing
    * (process Route header)
    */
   route_preprocess(ticket);

   /*
    * figure out whether this is an incoming or outgoing request
    * by doing a lookup in the registration table.
    */
   sip_find_direction(ticket, &i);
   type = ticket->direction;

   /* Call Plugins for stage: PLUGIN_PRE_PROXY */
   sts = call_plugins(PLUGIN_PRE_PROXY, ticket);

   /*
    * RFC 3261, Section 16.6 step 1
    * Proxy Behavior - Request Forwarding - Make a copy
    */
   /* nothing to do here, copy is ready in 'request'*/

   /* get destination address */
   url=osip_message_get_uri(request);

   switch (type) {
  /*
   * from an external host to the internal masqueraded host
   */
   case REQTYP_INCOMING:
      DEBUGC(DBCLASS_PROXY,"incoming request from %s@%s from outbound",
	request->from->url->username? request->from->url->username:"*NULL*",
        request->from->url->host? request->from->url->host: "*NULL*");

      /*
       * RFC 3261, Section 16.6 step 2
       * Proxy Behavior - Request Forwarding - Request-URI
       * (rewrite request URI to point to the real host)
       */
      /* 'i' still holds the valid index into the URLMAP table */
      proxy_rewrite_request_uri(request, i);

      /* if this is CANCEL/BYE request, stop RTP proxying */
      if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
         /* stop the RTP proxying stream(s) */
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_INCOMING);
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_OUTGOING);

      /* check for incoming request */
      } else if (MSG_IS_INVITE(request)) {
         /* Rewrite the body */
         sts = proxy_rewrite_invitation_body(ticket, DIR_INCOMING);

      } else if (MSG_IS_ACK(request) || MSG_IS_PRACK(request)) {
         /* Rewrite the body */
         sts = proxy_rewrite_invitation_body(ticket, DIR_INCOMING);

      } else if (MSG_IS_UPDATE(request)) {
         /* Rewrite the body */
         sts = proxy_rewrite_invitation_body(ticket, DIR_INCOMING);

      }
sts=sip_obscure_callid(ticket);
      break;
   
  /*
   * from the internal masqueraded host to an external host
   */
   case REQTYP_OUTGOING:
      DEBUGC(DBCLASS_PROXY,"outgoing request from %s@%s from inbound",
	request->from->url->username? request->from->url->username:"*NULL*",
        request->from->url->host? request->from->url->host: "*NULL*");

      /*
       * RFC 3261, Section 16.6 step 2
       * Proxy Behavior - Request Forwarding - Request-URI
       */
      /* nothing to do for an outgoing request */


      /* if it is addressed to myself, then it must be some request
       * method that I as a proxy do not support. Reject */
#if 0
/* careful - an internal UA might send an request to another internal UA.
   This would be caught here, so don't do this. This situation should be
   caught in the default part of the CASE statement below */
      if (is_sipuri_local(ticket) == STS_TRUE) {
         WARN("unsupported request [%s] directed to proxy from %s@%s -> %s@%s",
	    request->sip_method? request->sip_method:"*NULL*",
	    request->from->url->username? request->from->url->username:"*NULL*",
	    request->from->url->host? request->from->url->host : "*NULL*",
	    url->username? url->username : "*NULL*",
	    url->host? url->host : "*NULL*");

         sip_gen_response(ticket, 403 /*forbidden*/);

         return STS_FAILURE;
      }
#endif

      /* Rewrite Contact header to represent the masqued address */
      sip_rewrite_contact(ticket, DIR_OUTGOING);

      /* Masquerade the User-Agent if configured to do so */
      proxy_rewrite_useragent(ticket);

      /* if an INVITE, rewrite body */
      if (MSG_IS_INVITE(request)) {
         sts = proxy_rewrite_invitation_body(ticket, DIR_OUTGOING);
      } else if (MSG_IS_ACK(request) || MSG_IS_PRACK(request)) {
         sts = proxy_rewrite_invitation_body(ticket, DIR_OUTGOING);
      } else if (MSG_IS_UPDATE(request)) {
         sts = proxy_rewrite_invitation_body(ticket, DIR_OUTGOING);
      }

      /* if this is CANCEL/BYE request, stop RTP proxying */
      if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
         /* stop the RTP proxying stream(s) */
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_INCOMING);
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_OUTGOING);
      }

sts=sip_obscure_callid(ticket);

      break;
   
   default:
      DEBUGC(DBCLASS_PROXY, "request [%s] from/to unregistered UA "
           "(RQ: %s@%s -> %s@%s)",
           request->sip_method? request->sip_method:"*NULL*",
	   request->from->url->username? request->from->url->username:"*NULL*",
	   request->from->url->host? request->from->url->host : "*NULL*",
	   url->username? url->username : "*NULL*",
	   url->host? url->host : "*NULL*");

/*
 * we may end up here for two reasons:
 *  1) An incomming request (from outbound) that is directed to
 *     an unknown (not registered) local UA
 *  2) an outgoing request from a local UA that is not registered.
 *
 * Case 1) we should probably answer with "404 Not Found",
 * case 2) more likely a "403 Forbidden"
 * 
 * How about "408 Request Timeout" ?
 *
 */
      /* Call Plugins for stage: PLUGIN_PROXY_UNK */
      sts = call_plugins(PLUGIN_PROXY_UNK, ticket);
      /*&&& we might want have the possibility for a plugin to generate
            and send a response, similar an in stage PLUGIN_DETERMINE_TARGET */

      sip_gen_response(ticket, 408 /* Request Timeout */);

      return STS_FAILURE;
   }


   /*
    * RFC 3261, Section 16.6 step 3
    * Proxy Behavior - Request Forwarding - Max-Forwards
    * (if Max-Forwards header exists, decrement by one, if it does not
    * exist, add a new one with value SHOULD be 70)
    */
   {
   osip_header_t *max_forwards;
   int forwards_count = DEFAULT_MAXFWD;
   char mfwd[12]; /* 10 digits, +/- sign, termination */

   osip_message_get_max_forwards(request, 0, &max_forwards);
   if (max_forwards == NULL) {
      sprintf(mfwd, "%i", forwards_count);
      osip_message_set_max_forwards(request, mfwd);
   } else {
      if (max_forwards->hvalue) {
         forwards_count = atoi(max_forwards->hvalue);
         if ((forwards_count<0)||
             (forwards_count>255)) forwards_count=DEFAULT_MAXFWD;
         forwards_count -=1;
         osip_free (max_forwards->hvalue);
      }

      sprintf(mfwd, "%i", forwards_count);
      max_forwards->hvalue = osip_strdup(mfwd);
   }

   DEBUGC(DBCLASS_PROXY,"setting Max-Forwards=%s",mfwd);
   }

   /*
    * RFC 3261, Section 16.6 step 4
    * Proxy Behavior - Request Forwarding - Add a Record-route header
    */

   /*
    * for ALL incoming requests, include my Record-Route header.
    * The local UA will probably send its answer to the topmost 
    * Route Header (8.1.2 of RFC3261)
    */
   if (type == REQTYP_INCOMING) {
      DEBUGC(DBCLASS_PROXY,"Adding my Record-Route");
      route_add_recordroute(ticket);
   } else {
      /*
       * outgoing packets must not have my record route header, as
       * this likely will contain a private IP address (my inbound).
       */
      DEBUGC(DBCLASS_PROXY,"Purging Record-Routes (outgoing packet)");
      route_purge_recordroute(ticket);
   }

   /*
    * RFC 3261, Section 16.6 step 5
    * Proxy Behavior - Request Forwarding - Add Additional Header Fields
    */
   /* NOT IMPLEMENTED (optional) */


   /*
    * RFC 3261, Section 16.6 step 6
    * Proxy Behavior - Request Forwarding - Postprocess routing information
    *
    * If the copy contains a Route header field, the proxy MUST
    * inspect the URI in its first value.  If that URI does not
    * contain an lr parameter, the proxy MUST modify the copy as
    * follows:
    *
    * -  The proxy MUST place the Request-URI into the Route header
    *    field as the last value.
    *
    * -  The proxy MUST then place the first Route header field value
    *    into the Request-URI and remove that value from the Route
    *    header field.
    */
#if 0
   route_postprocess(ticket);
#endif

   /*
    * RFC 3261, Section 16.6 step 7
    * Proxy Behavior - Determine Next-Hop Address
    */
/*&&&& priority probably should be:
 * 1) Route header
 * 2) fixed outbound proxy
 * 3) SIP URI
 */
   /*
    * Route present?
    * If so, fetch address from topmost Route: header and remove it.
    */
   if ((type == REQTYP_OUTGOING) && 
              (!osip_list_eol(&(request->routes), 0))) {
      sts=route_determine_nexthop(ticket, &sendto_addr, &port);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: route_determine_nexthop failed");
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_request: have Route header to %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * fixed or domain outbound proxy defined ?
    */
   } else if ((type == REQTYP_OUTGOING) &&
       (sip_find_outbound_proxy(ticket, &sendto_addr, &port) == STS_SUCCESS)) {
      DEBUGC(DBCLASS_PROXY, "proxy_request: have outbound proxy %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * destination from SIP URI
    */
   } else {
      /* get the destination from the SIP URI */
/*&&&& Here, the SRV record lookup magic must go.
In a first implementation we may just try to get the lowest priority,
max weighted '_sip._udp.domain' entry and port number.
No load balancing and no failover are supported with this.
&&&*/
      sts = get_ip_by_host(url->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve URI [%s]",
                url->host);
         return STS_FAILURE;
      }

      if (url->port) {
         port=atoi(url->port);
         if ((port<=0) || (port>65535)) port=SIP_PORT;
      } else {
         port=SIP_PORT;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_request: have SIP URI to %s:%i",
             url->host, port);
   }

   /*
    * RFC 3261, Section 16.6 step 8
    * Proxy Behavior - Add a Via header field value
    */
   /* add my Via header line (outbound interface)*/
   if (type == REQTYP_INCOMING) {
      sts = sip_add_myvia(ticket, IF_INBOUND);
      if (sts == STS_FAILURE) {
         ERROR("adding my inbound via failed!");
      }
   } else {
      sts = sip_add_myvia(ticket, IF_OUTBOUND);
      if (sts == STS_FAILURE) {
         ERROR("adding my outbound via failed!");
         return STS_FAILURE;
      }
   }
  /*
   * RFC 3261, Section 16.6 step 9
   * Proxy Behavior - Add a Content-Length header field if necessary
   */
  /* not necessary, already in message and we do not support TCP */

   /* Call Plugins for stage: PLUGIN_POST_PROXY */
   sts = call_plugins(PLUGIN_POST_PROXY, ticket);

  /*
   * RFC 3261, Section 16.6 step 10
   * Proxy Behavior - Forward the new request
   */
   sts = sip_message_to_str(request, &buffer, &buflen);
   if (sts != 0) {
      ERROR("proxy_request: sip_message_to_str failed");
      return STS_FAILURE;
   }

   sipsock_send(sendto_addr, port, ticket->protocol, buffer, buflen);
   osip_free (buffer);

  /*
   * RFC 3261, Section 16.6 step 11
   * Proxy Behavior - Set timer C
   */
  /* NOT IMPLEMENTED - does this really apply for stateless proxies? */

   return STS_SUCCESS;
}


/*
 * PROXY_RESPONSE
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 * RFC3261
 *    Section 16.7: Proxy Behavior - Response Processing
 *    1.  Find the appropriate response context
 *    2.  Update timer C for provisional responses
 *    3.  Remove the topmost Via
 *    4.  Add the response to the response context
 *    5.  Check to see if this response should be forwarded immediately
 *    6.  When necessary, choose the best final response from the
 *        response context
 *    7.  Aggregate authorization header field values if necessary
 *    8.  Optionally rewrite Record-Route header field values
 *    9.  Forward the response
 *    10. Generate any necessary CANCEL requests 
 *
 */
int proxy_response (sip_ticket_t *ticket) {
   int sts;
   int type;
   struct in_addr sendto_addr;
   osip_via_t *via;
   int port;
   char *buffer;
   size_t buflen;
   osip_message_t *response;

   DEBUGC(DBCLASS_PROXY,"proxy_response");

   if (ticket==NULL) {
      ERROR("proxy_response: called with NULL ticket");
      return STS_FAILURE;
   }

   response=ticket->sipmsg;

   /*
    * RFC 3261, Section 16.7 step 3
    * Proxy Behavior - Response Processing - Remove my Via header field value
    */
   /* remove my Via header line */
   sts = sip_del_myvia(ticket);
   if (sts == STS_FAILURE) {
      DEBUGC(DBCLASS_PROXY,"not addressed to my VIA, ignoring response");
      return STS_FAILURE;
   }

   /*
    * figure out if this is an request coming from the outside
    * world to one of our registered clients
    */
   sip_find_direction(ticket, NULL);
   type = ticket->direction;

   /* Call Plugins for stage: PLUGIN_PRE_PROXY */
   sts = call_plugins(PLUGIN_PRE_PROXY, ticket);

/*
 * ok, we got a response that we are allowed to process.
 */
   switch (type) {
  /*
   * from an external host to the internal masqueraded host
   */
   case RESTYP_INCOMING:
      DEBUGC(DBCLASS_PROXY,"incoming response for %s@%s from outbound",
	response->from->url->username? response->from->url->username:"*NULL*",
	response->from->url->host? response->from->url->host : "*NULL*");

sts=sip_obscure_callid(ticket);


      /*
       * Response for INVITE - deal with RTP data in body and
       *                       start RTP proxy stream(s). In case
       *                       of a negative answer, stop RTP stream
       */
      if ((MSG_IS_RESPONSE_FOR(response,"INVITE")) ||
          (MSG_IS_RESPONSE_FOR(response,"UPDATE"))) {
         /* positive response, start RTP stream */
         if ((MSG_IS_STATUS_1XX(response)) || 
              (MSG_IS_STATUS_2XX(response))) {
            if (configuration.rtp_proxy_enable == 1) {
               sts = proxy_rewrite_invitation_body(ticket, DIR_INCOMING);
            }
         /* negative - stop a possibly started RTP stream */
         } else if ((MSG_IS_STATUS_4XX(response))  ||
                     (MSG_IS_STATUS_5XX(response)) ||
                     (MSG_IS_STATUS_6XX(response))) {
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_INCOMING);
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_OUTGOING);
         }
      } /* if INVITE */

      /*
       * Response for REGISTER - special handling of Contact header
       */
      if (MSG_IS_RESPONSE_FOR(response,"REGISTER")) {
         /*
          * REGISTER returns *my* Contact header information.
          * Rewrite Contact header back to represent the true address.
          * Other responses do return the Contact header of the sender.
          * also change the expiration timeout to the value returned by the
          * server.
          */
         sts = register_set_expire(ticket);
         sip_rewrite_contact(ticket, DIR_INCOMING);
      }

      /* 
       * Response for SUBSCRIBE
       *
       * HACK for Grandstream SIP phones (with newer firmware like 1.0.4.40):
       *   They send a SUBSCRIBE request to the registration server. In
       *   case of beeing registering directly to siproxd, this request of
       *   course will eventually be forwarded back to the same UA.
       *   Grandstream then does reply with an '202' response (A 202
       *   response merely indicates that the subscription has been
       *   understood, and that authorization may or may not have been
       *   granted), which then of course is forwarded back to the phone.
       *   And it seems that the Grandstream can *not* *handle* this
       *   response, as it immediately sends another SUBSCRIBE request.
       *   And this games goes on and on and on...
       *
       *   As a workaround we will transform any 202 response to a
       *   '404 unknown destination'
       *   
       */
{
      osip_header_t *ua_hdr=NULL;
      osip_message_get_user_agent(response, 0, &ua_hdr);
      if (ua_hdr && ua_hdr->hvalue &&
          (osip_strncasecmp(ua_hdr->hvalue,"grandstream", 11)==0) &&
          (MSG_IS_RESPONSE_FOR(response,"SUBSCRIBE")) &&
          (MSG_TEST_CODE(response, 202))) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: Grandstream hack 202->404");
         response->status_code=404;
      }
}
      break;
   
  /*
   * from the internal masqueraded host to an external host
   */
   case RESTYP_OUTGOING:
      DEBUGC(DBCLASS_PROXY,"outgoing response for %s@%s from inbound",
	     response->from->url->username ?
                response->from->url->username : "*NULL*",
	     response->from->url->host ? 
                response->from->url->host : "*NULL*");

sts=sip_obscure_callid(ticket);

      /* Rewrite Contact header to represent the masqued address */
      sip_rewrite_contact(ticket, DIR_OUTGOING);

      /* Masquerade the User-Agent if configured to do so */
      proxy_rewrite_useragent(ticket);

      /*
       * If an 2xx OK or 1xx response, answer to an INVITE request,
       * rewrite body
       *
       * In case of a negative answer, stop RTP stream
       */
      if ((MSG_IS_RESPONSE_FOR(response,"INVITE")) ||
          (MSG_IS_RESPONSE_FOR(response,"UPDATE"))) {
         /* positive response, start RTP stream */
         if ((MSG_IS_STATUS_1XX(response)) || 
              (MSG_IS_STATUS_2XX(response))) {
            /* This is an outgoing response, therefore an outgoing stream */
            sts = proxy_rewrite_invitation_body(ticket, DIR_OUTGOING);
         /* megative - stop a possibly started RTP stream */
         } else if ((MSG_IS_STATUS_4XX(response))  ||
                     (MSG_IS_STATUS_5XX(response)) ||
                     (MSG_IS_STATUS_6XX(response))) {
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_INCOMING);
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_OUTGOING);
         }
      } /* if INVITE */

      break;
   
   default:
      DEBUGC(DBCLASS_PROXY, "response from/to unregistered UA (%s@%s)",
	   response->from->url->username? response->from->url->username:"*NULL*",
	   response->from->url->host? response->from->url->host : "*NULL*");

      /* Call Plugins for stage: PLUGIN_PROXY_UNK */
      sts = call_plugins(PLUGIN_PROXY_UNK, ticket);
      return STS_FAILURE;
   }

   /*
    * for ALL incoming response include my Record-Route header.
    * The local UA will probably send its answer to the topmost 
    * Route Header (8.1.2 of RFC3261)
    */
    if (type == RESTYP_INCOMING) {
       DEBUGC(DBCLASS_PROXY,"Adding my Record-Route");
       route_add_recordroute(ticket);
    } else {
       /*
        * outgoing packets must not have my record route header, as
        * this likely will contain a private IP address (my inbound).
        */
       DEBUGC(DBCLASS_PROXY,"Purging Record-Routes (outgoing packet)");
       route_purge_recordroute(ticket);
    }

   /*
    * Determine Next-Hop Address
    */
/*&&&& priority probably should be:
 * 0) rport=;received= header (TCP only for now)
 * 1) Route header
 * 2) fixed outbound proxy
 * 3) Via header
 */
   /*
    * IF TCP, check for rport=x;received=y parameters in VIA
    */
   if ((ticket->protocol == PROTO_TCP) && 
       (sip_get_received_param(ticket,  &sendto_addr, &port) == STS_SUCCESS)) {
      DEBUGC(DBCLASS_PROXY, "proxy_response: have received/rport to %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * Route present?
    * If so, fetch address from topmost Route: header and remove it.
    */
   } else if ((type == RESTYP_OUTGOING) && 
              (!osip_list_eol(&(response->routes), 0))) {
      sts=route_determine_nexthop(ticket, &sendto_addr, &port);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: route_determine_nexthop failed");
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_response: have Route header to %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * check if we need to send to an outbound proxy
    */
   } else if ((type == RESTYP_OUTGOING) &&
       (sip_find_outbound_proxy(ticket, &sendto_addr, &port) == STS_SUCCESS)) {
      DEBUGC(DBCLASS_PROXY, "proxy_response: have outbound proxy %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   } else {
      /* get target address and port from VIA header */
      via = (osip_via_t *) osip_list_get (&(response->vias), 0);
      if (via == NULL) {
         ERROR("proxy_response: list_get via failed");
         return STS_FAILURE;
      }

      sts = get_ip_by_host(via->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: cannot resolve VIA [%s]",
                via->host);
         return STS_FAILURE;
      }

      if (via->port) {
         port=atoi(via->port);
         if ((port<=0) || (port>65535)) port=SIP_PORT;
      } else {
         port=SIP_PORT;
      }
   }

   /* Call Plugins for stage: PLUGIN_POST_PROXY */
   sts = call_plugins(PLUGIN_POST_PROXY, ticket);

  /*
   * Proxy Behavior - Forward the response
   */
   sts = sip_message_to_str(response, &buffer, &buflen);
   if (sts != 0) {
      ERROR("proxy_response: sip_message_to_str failed");
      return STS_FAILURE;
   }

   sipsock_send(sendto_addr, port, ticket->protocol, buffer, buflen);
   osip_free (buffer);
   return STS_SUCCESS;
}


/*
 * PROXY_REWRITE_INVITATION_BODY
 *
 * rewrites the outgoing INVITATION request or response packet
 * 
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int proxy_rewrite_invitation_body(sip_ticket_t *ticket, int direction){
   osip_message_t *mymsg=ticket->sipmsg;
   osip_body_t *body;
   sdp_message_t  *sdp;
   struct in_addr map_addr, addr_sess, addr_media, outside_addr, inside_addr;
   int sts;
   char *buff;
   size_t buflen;
   char clen[8]; /* content length: probably never more than 7 digits !*/
   int map_port, msg_port;
   int media_stream_no;
   sdp_connection_t *sdp_conn;
   sdp_media_t *sdp_med;
   int rtp_direction=0;
   int call_direction=0;
   int have_c_media=0;
   int isrtp = 0 ;

   if (configuration.rtp_proxy_enable == 0) return STS_SUCCESS;

   /*
    * get SDP structure
    */
   sts = osip_message_get_body(mymsg, 0, &body);
   if (sts != 0) {
      DEBUGC(DBCLASS_PROXY, "rewrite_invitation_body: "
                            "no body found in message");
      return STS_SUCCESS;
   }

   sts = sip_body_to_str(body, &buff, &buflen);
   if (sts != 0) {
      ERROR("rewrite_invitation_body: unable to sip_body_to_str");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_PROXY, "rewrite_invitation_body: payload %ld bytes", 
          (long)buflen);
   DUMP_BUFFER(DBCLASS_PROXY, buff, buflen);

   sts = sdp_message_init(&sdp);
   sts = sdp_message_parse (sdp, buff);
   if (sts != 0) {
      ERROR("rewrite_invitation_body: unable to sdp_message_parse body");
      DUMP_BUFFER(-1, buff, buflen);
      osip_free(buff);
      sdp_message_free(sdp);
      return STS_FAILURE;
   }
   osip_free(buff);


if (configuration.debuglevel)
{ /* just dump the buffer */
   char *tmp, *tmp2;
   size_t tmplen;
   sts = osip_message_get_body(mymsg, 0, &body);
   sts = sip_body_to_str(body, &tmp, &tmplen);
   if (sts == 0) {
      osip_content_length_to_str(mymsg->content_length, &tmp2);
      DEBUG("Body before rewrite (may be truncated) - (clen=%s, strlen=%ld):\n%s\n----",
            tmp2, (long)tmplen, tmp);
      osip_free(tmp);
      osip_free(tmp2);
   } else {
      DEBUG("Body before rewrite: failed to decode!");
   }
}

   /*
    * RTP proxy: get ready and start forwarding
    * start forwarding for each media stream ('m=' item in SIP message)
    */

   /* get outbound address */
   if (get_interface_ip(IF_OUTBOUND, &outside_addr) != STS_SUCCESS) {
      sdp_message_free(sdp);
      return STS_FAILURE;
   }

   /* get inbound address */
   if (get_interface_ip(IF_INBOUND, &inside_addr) != STS_SUCCESS) {
      sdp_message_free(sdp);
      return STS_FAILURE;
   }

   /* figure out what address to use for RTP masquerading */
   if (MSG_IS_REQUEST(mymsg)) {
      if (direction == DIR_INCOMING) {
         memcpy(&map_addr, &inside_addr, sizeof (map_addr));
         rtp_direction  = DIR_OUTGOING;
         call_direction = DIR_INCOMING;
      } else {
         memcpy(&map_addr, &outside_addr, sizeof (map_addr));
         rtp_direction  = DIR_INCOMING;
         call_direction = DIR_OUTGOING;
      }
   } else /* MSG_IS_REPONSE(mymsg) */ {
      if (direction == DIR_INCOMING) {
         memcpy(&map_addr, &inside_addr, sizeof (map_addr));
         rtp_direction  = DIR_OUTGOING;
         call_direction = DIR_OUTGOING;
      } else {
         memcpy(&map_addr, &outside_addr, sizeof (map_addr));
         rtp_direction  = DIR_INCOMING;
         call_direction = DIR_INCOMING;
      }
   }

   DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: SIP[%s %s] RTP[%s %s]",
          MSG_IS_REQUEST(mymsg)? "RQ" : "RS",
          (direction==DIR_INCOMING)? "IN" : "OUT",
          (rtp_direction==DIR_INCOMING)? "IN" : "OUT",
          utils_inet_ntoa(map_addr));


   /*
    * first, check presence of a 'c=' item on session level
    */
   if (sdp->c_connection==NULL || sdp->c_connection->c_addr==NULL) {
      /*
       * No 'c=' on session level, search on media level now
       *
       * According to RFC2327, ALL media description must
       * include a 'c=' item now:
       */
      media_stream_no=0;
      while (!sdp_message_endof_media(sdp, media_stream_no)) {
         /* check if n'th media stream is present */
         if (sdp_message_c_addr_get(sdp, media_stream_no, 0) == NULL) {
            ERROR("SDP: have no 'c=' on session level and neither "
                  "on media level (media=%i)",media_stream_no);
            sdp_message_free(sdp);
            return STS_FAILURE;
         }
         media_stream_no++;
      } /* while */
   }

   /* Required 'c=' items ARE present */


   /*
    * rewrite 'c=' item on session level if present and not yet done.
    * remember the original address in addr_sess
    */
   memset(&addr_sess, 0, sizeof(addr_sess));
   if (sdp->c_connection && sdp->c_connection->c_addr) {
      sts = get_ip_by_host(sdp->c_connection->c_addr, &addr_sess);
      if (sts == STS_FAILURE) {
         ERROR("SDP: cannot resolve session 'c=' host [%s]",
               sdp->c_connection->c_addr);
         sdp_message_free(sdp);
         return STS_FAILURE;
      }
      /*
       * Rewrite
       * an IP address of 0.0.0.0 means *MUTE*, don't rewrite such
       */
      if (strcmp(sdp->c_connection->c_addr, "0.0.0.0") != 0) {
         osip_free(sdp->c_connection->c_addr);
         sdp->c_connection->c_addr=osip_malloc(HOSTNAME_SIZE);
         sprintf(sdp->c_connection->c_addr, "%s", utils_inet_ntoa(map_addr));
      } else {
         /* 0.0.0.0 - don't rewrite */
         DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: "
                "got a MUTE c= record (on session level - legal?)");
      }
   }


   /*
    * rewrite 'o=' item (originator) on session level if present.
    */
   if (sdp->o_addrtype && sdp->o_addr) {
      if (strcmp(sdp->o_addrtype, "IP4") != 0) {
         ERROR("got IP6 in SDP originator - not yet suported by siproxd");
         sdp_message_free(sdp);
         return STS_FAILURE;
      }

      osip_free(sdp->o_addr);
      sdp->o_addr=osip_malloc(HOSTNAME_SIZE);
      sprintf(sdp->o_addr, "%s", utils_inet_ntoa(map_addr));
   }


   /*
    * loop through all media descritions,
    * start RTP proxy and rewrite them
    */
   for (media_stream_no=0;;media_stream_no++) {
      /* check if n'th media stream is present */
      if (sdp_message_m_port_get(sdp, media_stream_no) == NULL) break;

      /*
       * check if a 'c=' item is present in this media description,
       * if so -> rewrite it
       */
      memset(&addr_media, 0, sizeof(addr_media));
      have_c_media=0;
      sdp_conn=sdp_message_connection_get(sdp, media_stream_no, 0);
      if (sdp_conn && sdp_conn->c_addr) {
         if (strcmp(sdp_conn->c_addr, "0.0.0.0") != 0) {
            sts = get_ip_by_host(sdp_conn->c_addr, &addr_media);
            have_c_media=1;
            /* have a valid address */
            osip_free(sdp_conn->c_addr);
            sdp_conn->c_addr=osip_malloc(HOSTNAME_SIZE);
            sprintf(sdp_conn->c_addr, "%s", utils_inet_ntoa(map_addr));
         } else {
            /* 0.0.0.0 - don't rewrite */
            DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: got a "
                   "MUTE c= record (media level)");
         }
      }

      /* start an RTP proxying stream */
      if (sdp_message_m_port_get(sdp, media_stream_no)) {
         msg_port=atoi(sdp_message_m_port_get(sdp, media_stream_no));
         if ((msg_port > 0) && (msg_port <= 65535)) {
            client_id_t client_id;
            osip_contact_t *contact = NULL;
            char *protocol=NULL;

            /* try to get some additional UA specific unique ID.
             * This Client-ID should be guaranteed persistent
             * and not depend on if a UA/Server does include a
             * particular Header (Contact) or not.
             */
            /* we will use - if present the from/to fields.
              * Outgoing call (RQ out, RS in  => use the "from" field to identify local client
              * Incoming call (RQ in,  RS out => use the "to" field to identify local client
              *
              * According to RFC3261, the From and To headers MUST NOT change
              * withing an ongoing dialog:
              *
              * 8.2.6.2 Headers and Tags
              *
              *    The From field of the response MUST equal the From header field of
              *    the request.  The Call-ID header field of the response MUST equal the
              *    Call-ID header field of the request.  The CSeq header field of the
              *    response MUST equal the CSeq field of the request.  The Via header
              *    field values in the response MUST equal the Via header field values
              *    in the request and MUST maintain the same ordering.
              *
              *    If a request contained a To tag in the request, the To header field
              *    in the response MUST equal that of the request.  However, if the To
              *    header field in the request did not contain a tag, the URI in the To
              *    header field in the response MUST equal the URI in the To header
              *    field; additionally, the UAS MUST add a tag to the To header field in
              *    the response (with the exception of the 100 (Trying) response, in
              *    [...]
              */

             /* If no proper TO/FROM headers are present, fall back to use Contact header... */

            memset(&client_id, 0, sizeof(client_id));

             /* Outgoing call (RQ out, RS in  => use the "from" field to identify local client */
            if ((MSG_IS_REQUEST(mymsg) && direction == DIR_OUTGOING) ||
                (!MSG_IS_REQUEST(mymsg) && direction == DIR_INCOMING)) {

                /* I have a full FROM SIP URI 'user@host' */
                if (mymsg->from && mymsg->from->url && 
                    mymsg->from->url->username && mymsg->from->url->host) {
                   snprintf(client_id.idstring, CLIENT_ID_SIZE-1, "%s@%s", 
                            mymsg->from->url->username, mymsg->from->url->host);
                } else {
                   char *tmp=NULL;
                   /* get the Contact Header if present */
                   osip_message_get_contact(mymsg, 0, &contact);
                   if (contact) osip_contact_to_str(contact, &tmp);
                   if (tmp) strncpy(client_id.idstring, tmp, CLIENT_ID_SIZE-1);
                } /* if from header */

            /* Incoming call (RQ in,  RS out => use the "to" field to identify local client */
            } else { /*(MSG_IS_REQUEST(mymsg) && direction == DIR_INCOMING) ||
                       (!MSG_IS_REQUEST(mymsg) && direction == DIR_OUTGOING)) */

                /* I have a full TO SIP URI 'user@host' */
                if (mymsg->to && mymsg->to->url && 
                    mymsg->to->url->username && mymsg->to->url->host) {
                   snprintf(client_id.idstring, CLIENT_ID_SIZE-1, "%s@%s", 
                            mymsg->to->url->username, mymsg->to->url->host);
                } else {
                   char *tmp=NULL;
                   /* get the Contact Header if present */
                   osip_message_get_contact(mymsg, 0, &contact);
                   if (contact) osip_contact_to_str(contact, &tmp);
                   if (tmp) strncpy(client_id.idstring, tmp, CLIENT_ID_SIZE-1);
                } /* if to header */
            }

            /* store the IP address of the sender */
            memcpy(&client_id.from_ip, &ticket->from.sin_addr,
                   sizeof(client_id.from_ip));


            /*
             * is this an RTP stream ? If yes, set 'isrtp=1'
             */
            protocol = sdp_message_m_proto_get (sdp, media_stream_no);
            if (protocol == NULL) {
               DEBUGC(DBCLASS_PROXY, "no protocol definition found!");
            } else {
               char *check;
               char *cmp;
               isrtp = 1;
               check = protocol ;
               cmp = "RTP/" ;
               while (*cmp && (isrtp = isrtp && *check) && 
                      (isrtp = isrtp && (*cmp++ == toupper(*check++))) ) {} ;
               if (isrtp) {
                  DEBUGC(DBCLASS_PROXY, "found RTP protocol [%s]!", protocol);
               } else {
                  DEBUGC(DBCLASS_PROXY, "found non RTP protocol [%s]!", protocol);
               }
            }

            /*
             * do we have a 'c=' item on media level?
             * if not, use the same as on session level
             */
            if (have_c_media == 0) {
               memcpy(&addr_media, &addr_sess, sizeof(addr_sess));
            }

            /*
             * Am I running in front of the routing device? Then I cannot
             * use the external IP to bind a listen socket to, but should
             * use my real IP on the outbound interface (which may legally
             * be the same as the inbound interface if only one interface
             * is used).
             */
            if ((rtp_direction == DIR_INCOMING) &&
                (configuration.outbound_host) &&
                (strcmp(configuration.outbound_host, "")!=0)) {
               DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: "
                      "in-front-of-NAT-Router, use real outboud IP");
               if (get_interface_real_ip(IF_OUTBOUND, &map_addr)
                   != STS_SUCCESS) {
                  ERROR("cannot get my real outbound interface address");
                  /* as we do not know better, take the internal address */
                  memcpy(&map_addr, &inside_addr, sizeof (map_addr));
               }
            }

            /*
             * Start the RTP stream
             */
            sts = rtp_start_fwd(osip_message_get_call_id(mymsg),
                                client_id,
                                rtp_direction, call_direction,
                                media_stream_no,
                                map_addr, &map_port,
                                addr_media, msg_port,
                                isrtp);

            if (sts == STS_SUCCESS) {
               /* and rewrite the port */
               sdp_med=osip_list_get(&(sdp->m_medias), media_stream_no);
               if (sdp_med && sdp_med->m_port) {
                  osip_free(sdp_med->m_port);
                  sdp_med->m_port=osip_malloc(8); /* 5 digits, \0 + align */
                  sprintf(sdp_med->m_port, "%i", map_port);
                  DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: "
                         "m= rewrote port to [%i]",map_port);
               } else {
                  ERROR("rewriting port in m= failed sdp_med=%p, "
                        "m_number_of_port=%p", sdp_med, sdp_med->m_port);
               }
            } /* sts == success */
         } /* if msg_port > 0 */
      } else {
         /* no port defined - skip entry */
         WARN("no port defined in m=(media) stream_no=%i", media_stream_no);
         continue;
      }
   } /* for media_stream_no */

   /* remove old body */
   sts = osip_list_remove(&(mymsg->bodies), 0);
   osip_body_free(body);

   /* dump new body */
   sdp_message_to_str(sdp, &buff);
   buflen=strlen(buff);

   /* free sdp structure */
   sdp_message_free(sdp);

   /* include new body */
   sip_message_set_body(mymsg, buff, buflen);
   if (sts != 0) {
      ERROR("rewrite_invitation_body: unable to sip_message_set_body body");
   }

   /* free content length resource and include new one*/
   osip_content_length_free(mymsg->content_length);
   mymsg->content_length=NULL;
   sprintf(clen,"%ld",(long)buflen);
   sts = osip_message_set_content_length(mymsg, clen);

   /* free old body */
   osip_free(buff);

if (configuration.debuglevel)
{ /* just dump the buffer */
   char *tmp, *tmp2;
   size_t tmplen;
   sts = osip_message_get_body(mymsg, 0, &body);
   sts = sip_body_to_str(body, &tmp, &tmplen);
   if (sts == 0) {
      osip_content_length_to_str(mymsg->content_length, &tmp2);
      DEBUG("Body after rewrite (may be truncated) - (clen=%s, strlen=%ld):\n%s\n----",
            tmp2, (long)tmplen, tmp);
      osip_free(tmp);
      osip_free(tmp2);
   } else {
      DEBUG("Body after rewrite: failed to decode!");
   }
}
   return STS_SUCCESS;
}


/*
 * PROXY_REWRITE_REQUEST_URI
 *
 * rewrites the incoming Request URI
 * 
 * RETURNS
 *	STS_SUCCESS on success
 */
int proxy_rewrite_request_uri(osip_message_t *mymsg, int idx){
   char *scheme;
   char *username;
   char *host;
   char *port;
   osip_uri_t *url;

   if ((idx >= URLMAP_SIZE) || (idx < 0)) {
      WARN("proxy_rewrite_request_uri: called with invalid index");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_PROXY,"rewriting incoming Request URI");
   url=osip_message_get_uri(mymsg);

   /* set the true scheme */
   if (url->scheme) {osip_free(url->scheme);url->scheme=NULL;}
   if (urlmap[idx].true_url->scheme) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: scheme=%s",
             urlmap[idx].true_url->scheme);
      scheme = (char *)osip_malloc(strlen(urlmap[idx].true_url->scheme)+1);
      memcpy(scheme, urlmap[idx].true_url->scheme,
             strlen(urlmap[idx].true_url->scheme));
      scheme[strlen(urlmap[idx].true_url->scheme)]='\0';
      osip_uri_set_scheme(url, scheme);
   }

   /* set the true username */
   if (url->username) {osip_free(url->username);url->username=NULL;}
   if (urlmap[idx].true_url->username) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: username=%s",
             urlmap[idx].true_url->username);
      username = (char*)osip_malloc(strlen(urlmap[idx].true_url->username)+1);
      memcpy(username, urlmap[idx].true_url->username,
             strlen(urlmap[idx].true_url->username));
      username[strlen(urlmap[idx].true_url->username)]='\0';
      osip_uri_set_username(url, username);
   }

   /* set the true host */
   if (url->host) {osip_free(url->host);url->host=NULL;}
   if (urlmap[idx].true_url->host) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: host=%s",
             urlmap[idx].true_url->host);
      host = (char *)osip_malloc(strlen(urlmap[idx].true_url->host)+1);
      memcpy(host, urlmap[idx].true_url->host, strlen(urlmap[idx].true_url->host));
      host[strlen(urlmap[idx].true_url->host)]='\0';
      osip_uri_set_host(url, host);
   }

   /* set the true port */
   if (url->port) {osip_free(url->port);url->port=NULL;}
   if (urlmap[idx].true_url->port) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: port=%s",
             urlmap[idx].true_url->port);
      port = (char *)osip_malloc(strlen(urlmap[idx].true_url->port)+1);
      memcpy(port, urlmap[idx].true_url->port, strlen(urlmap[idx].true_url->port));
      port[strlen(urlmap[idx].true_url->port)]='\0';
      osip_uri_set_port(url, port);
   }
   return STS_SUCCESS;
}


/*
 * PROXY_REWRITE_USERAGENT
 *
 * rewrites the User Agent String
 * 
 * RETURNS
 *	STS_SUCCESS on success
 */
int proxy_rewrite_useragent(sip_ticket_t *ticket){
   osip_header_t *ua_hdr=NULL;

   osip_message_get_user_agent(ticket->sipmsg, 0, &ua_hdr);

   /* Configured? & Does User-Agent header exist? */
   if ((configuration.ua_string) && (ua_hdr && ua_hdr->hvalue)) {
      DEBUGC(DBCLASS_PROXY,"proxy_rewrite_useragent: [%s] -> [%s]",
             ua_hdr->hvalue, configuration.ua_string);
      osip_free(ua_hdr->hvalue);
      ua_hdr->hvalue=osip_malloc(strlen(configuration.ua_string)+1);
      strcpy(ua_hdr->hvalue, configuration.ua_string);
   }
   return STS_SUCCESS;
}
