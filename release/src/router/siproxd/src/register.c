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
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: register.c 419 2009-02-28 23:06:22Z hb9xar $";

/* configuration storage */
extern struct siproxd_config configuration;

/* URL mapping table     */
struct urlmap_s urlmap[URLMAP_SIZE];

/* time of last save     */
static time_t last_save=0;

extern int errno;


/*
 * initialize the URL mapping table
 */
void register_init(void) {
   FILE *stream;
   int sts, i;
   size_t len;
   char buff[128];
   char *c;

   memset(urlmap, 0, sizeof(urlmap));

   if (configuration.registrationfile) {
      stream = fopen(configuration.registrationfile, "r");

      if (!stream) {
         /*
          * the file does not exist, or the size was incorrect,
          * delete it and start from scratch
          */
         unlink(configuration.registrationfile);
         WARN("registration file not found, starting with empty table");
      } else {
         /* read the url table from file */
         for (i=0;i < URLMAP_SIZE; i++) {
            c=fgets(buff, sizeof(buff), stream);
            sts=sscanf(buff, "***:%i:%i", &urlmap[i].active, &urlmap[i].expires);
            if (sts == 0) break; /* format error */
            if (urlmap[i].active) {
               osip_uri_init(&urlmap[i].true_url);
               osip_uri_init(&urlmap[i].masq_url);
               osip_uri_init(&urlmap[i].reg_url);

               #define R(X) {\
               c=fgets(buff, sizeof(buff), stream);\
               buff[sizeof(buff)-1]='\0';\
               if (strchr(buff, 10)) *strchr(buff, 10)='\0';\
               if (strchr(buff, 13)) *strchr(buff, 13)='\0';\
               if (strlen(buff) > 0) {\
                  len  = strlen(buff);\
                  X    =(char*)malloc(len+1);\
                  sts=sscanf(buff,"%s",X);\
                  if (sts == 0) break;\
               } else {\
                  X = NULL;\
               }\
               }

               R(urlmap[i].true_url->scheme);
               R(urlmap[i].true_url->username);
               R(urlmap[i].true_url->host);
               R(urlmap[i].true_url->port);
               R(urlmap[i].masq_url->scheme);
               R(urlmap[i].masq_url->username);
               R(urlmap[i].masq_url->host);
               R(urlmap[i].masq_url->port);
               R(urlmap[i].reg_url->scheme);
               R(urlmap[i].reg_url->username);
               R(urlmap[i].reg_url->host);
               R(urlmap[i].reg_url->port);
            }
         }
         fclose(stream);

         /* check for premature abort of reading the registration file */
         if (i < URLMAP_SIZE) {
            /* clean up and delete it */
            WARN("registration file corrupt, starting with empty table");
            memset(urlmap, 0, sizeof(urlmap));
            unlink(configuration.registrationfile);
         }
      }
   }
   /* initialize save-timer */
   time(&last_save);
   return;
}


/*
 * shut down the URL mapping table
 */
void register_save(void) {
   int i;
   FILE *stream;

   if (configuration.registrationfile) {
      DEBUGC(DBCLASS_REG,"saving registration table");
      /* write urlmap back to file */
      stream = fopen(configuration.registrationfile, "w+");
      if (!stream) {
         /* try to unlink it and open again */
         unlink(configuration.registrationfile);
         stream = fopen(configuration.registrationfile, "w+");

         /* open file for write failed, complain */
         if (!stream) {
            ERROR("unable to write registration file");
            return;
         }
      }

      for (i=0;i < URLMAP_SIZE; i++) {
         fprintf(stream, "***:%i:%i\n", urlmap[i].active, urlmap[i].expires);
         if (urlmap[i].active) {
            #define W(X) fprintf(stream, "%s\n", (X)? X:"");

            W(urlmap[i].true_url->scheme);
            W(urlmap[i].true_url->username);
            W(urlmap[i].true_url->host);
            W(urlmap[i].true_url->port);
            W(urlmap[i].masq_url->scheme);
            W(urlmap[i].masq_url->username);
            W(urlmap[i].masq_url->host);
            W(urlmap[i].masq_url->port);
            W(urlmap[i].reg_url->scheme);
            W(urlmap[i].reg_url->username);
            W(urlmap[i].reg_url->host);
            W(urlmap[i].reg_url->port);
         }
      }
      fclose(stream);
   }
   return;
}


/*
 * handles register requests and updates the URL mapping table
 *
 * RETURNS:
 *    STS_SUCCESS : successfully registered
 *    STS_FAILURE : registration failed
 *    STS_NEED_AUTH : authentication needed
 */
int register_client(sip_ticket_t *ticket, int force_lcl_masq) {
   int i, j, n, sts;
   int expires;
   time_t time_now;
   osip_contact_t *contact;
   osip_uri_t *url1_to, *url1_contact=NULL;
   osip_uri_t *url2_to;
   osip_header_t *expires_hdr;
   osip_uri_param_t *expires_param=NULL;
   
   /*
    * Authorization - do only if I'm not just acting as outbound proxy
    * but am ment to be the registrar
    */
   if (force_lcl_masq == 0) {
      /*
       * RFC 3261, Section 16.3 step 6
       * Proxy Behavior - Request Validation - Proxy-Authorization
       */
      sts = authenticate_proxy(ticket->sipmsg);
      if (sts == STS_FAILURE) {
         /* failed */
         WARN("proxy authentication failed for %s@%s",
              (ticket->sipmsg->to->url->username)? 
              ticket->sipmsg->to->url->username : "*NULL*",
              ticket->sipmsg->to->url->host);
         return STS_FAILURE;
      } else if (sts == STS_NEED_AUTH) {
         /* needed */
         DEBUGC(DBCLASS_REG,"proxy authentication needed for %s@%s",
                ticket->sipmsg->to->url->username,
                ticket->sipmsg->to->url->host);
         return STS_NEED_AUTH;
      }
   }

/*
   fetch 1st Via entry and remember this address. Incoming requests
   for the registered address have to be passed on to that host.

   To: -> address to be registered
   Contact: -> host is reachable there
               Note: in case of un-REGISTER, the contact header may
                     contain '*' only - which means "all registrations
                     made by this UA"
   
   => Mapping is
   To: <1--n> Contact
   
*/
   time(&time_now);

   DEBUGC(DBCLASS_BABBLE,"sip_register:");

   /*
    * First make sure, we have a proper Contact header:
    *  - url
    *  - url -> hostname
    *
    * Libosip parses an:
    * "Contact: *"
    * the following way (Note: Display name!! and URL is NULL)
    * (gdb) p *((osip_contact_t*)(sip->contacts.node->element))
    * $5 = {displayname = 0x8af8848 "*", url = 0x0, gen_params = 0x8af8838}
    */

   osip_message_get_contact(ticket->sipmsg, 0, &contact);
   if ((contact == NULL) ||
       (contact->url == NULL) ||
       (contact->url->host == NULL)) {
      /* Don't have required Contact fields.
         This may be a Registration query or unregistering all registered
         records for this UA. We should simply forward this request to its
         destination. However, if this is an unregistration from a client
         that is not registered (Grandstream "unregister at startup" option)
         -> How do I handle this one?
         Right now we do a direction lookup and if this fails we generate
         an OK message by ourself (fake) */
      DEBUGC(DBCLASS_REG, "empty Contact header - "
             "seems to be a registration query");
      sts = sip_find_direction(ticket, NULL);
      if (sts != STS_SUCCESS) {
         /* answer the request myself. Most likely this is an UNREGISTER
          * request when the client just booted */
         sts = register_response(ticket, STS_SUCCESS);
         return STS_SIP_SENT;
      }

      return STS_SUCCESS;
   }

   url1_contact=contact->url;

   /* evaluate Expires Header field */
   osip_message_get_expires(ticket->sipmsg, 0, &expires_hdr);

  /*
   * look for an Contact expires parameter - in case of REGISTER
   * these two are equal. The Contact expires has higher priority!
   */
   if (ticket->sipmsg->contacts.node &&
       ticket->sipmsg->contacts.node->element) {
      osip_contact_param_get_byname(
              (osip_contact_t*) ticket->sipmsg->contacts.node->element,
              EXPIRES, &expires_param);
   }

   if (expires_param && expires_param->gvalue) {
      /* get expires from contact Header */
      expires=atoi(expires_param->gvalue);
      if ((expires < 0) || (expires >= UINT_MAX ))
         expires=configuration.default_expires;
   } else if (expires_hdr && expires_hdr->hvalue) {
      /* get expires from expires Header */
      expires=atoi(expires_hdr->hvalue);
      if ((expires < 0) || (expires >= UINT_MAX ))
         expires=configuration.default_expires;
   } else {
      char tmp[16];
      /* it seems, the expires field is not present everywhere... */
      DEBUGC(DBCLASS_REG,"no 'expires' header found - set time to %i sec",
             configuration.default_expires);
      expires=configuration.default_expires;
      sprintf(tmp,"%i",expires);
      osip_message_set_expires(ticket->sipmsg, tmp);
   }

   url1_to=ticket->sipmsg->to->url;



   /*
    * REGISTER
    */
   if (expires > 0) {
      DEBUGC(DBCLASS_REG,"register: %s@%s expires=%i seconds",
          (url1_contact->username) ? url1_contact->username : "*NULL*",
          (url1_contact->host) ? url1_contact->host : "*NULL*",
          expires);

      /*
       * Update registration. There are two possibilities:
       * - already registered, then update the existing record
       * - not registered, then create a new record
       */

      j=-1;
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) {
            if (j < 0) j=i; /* remember first hole */
            continue;
         }

         url2_to=urlmap[i].reg_url;

         /* check address-of-record ("public address" of user) */
         if (compare_url(url1_to, url2_to)==STS_SUCCESS) {
            DEBUGC(DBCLASS_REG, "found entry for %s@%s <-> %s@%s at "
                   "slot=%i, exp=%li",
                   (url1_contact->username) ? url1_contact->username : "*NULL*",
                   (url1_contact->host) ? url1_contact->host : "*NULL*",
                   (url2_to->username) ? url2_to->username : "*NULL*",
                   (url2_to->host) ? url2_to->host : "*NULL*",
                   i, (long)urlmap[i].expires-time_now);
            break;
         }
      }

      if ( (j < 0) && (i >= URLMAP_SIZE) ) {
         /* oops, no free entries left... */
         ERROR("URLMAP is full - registration failed");
         return STS_FAILURE;
      }

      if (i >= URLMAP_SIZE) {
         /* entry not existing, create new one */
         i=j;

         /* write entry */
         urlmap[i].active=1;
         /* Contact: field */
         osip_uri_clone( ((osip_contact_t*)
                         (ticket->sipmsg->contacts.node->element))->url, 
                         &urlmap[i].true_url);
         /* To: field */
         osip_uri_clone( ticket->sipmsg->to->url, 
                    &urlmap[i].reg_url);

         DEBUGC(DBCLASS_REG,"create new entry for %s@%s <-> %s@%s at slot=%i",
                (url1_contact->username) ? url1_contact->username : "*NULL*",
                (url1_contact->host) ? url1_contact->host : "*NULL*",
                (urlmap[i].reg_url->username) ? urlmap[i].reg_url->username : "*NULL*",
                (urlmap[i].reg_url->host) ? urlmap[i].reg_url->host : "*NULL*",
                i);

         /*
          * try to figure out if we ought to do some masquerading
          */
         osip_uri_clone( ticket->sipmsg->to->url, 
                         &urlmap[i].masq_url);

         n=configuration.mask_host.used;
         if (n != configuration.masked_host.used) {
            ERROR("# of mask_host is not equal to # of masked_host in config!");
            n=0;
         }

         DEBUG("%i entries in MASK config table", n);
         for (j=0; j<n; j++) {
            DEBUG("compare [%s] <-> [%s]",configuration.mask_host.string[j],
                  ticket->sipmsg->to->url->host);
            if (strcmp(configuration.mask_host.string[j],
                ticket->sipmsg->to->url->host)==0)
               break;
         }
         if (j<n) { 
            /* we are masquerading this UA, replace the host part of the url */
            DEBUGC(DBCLASS_REG,"masquerading UA %s@%s as %s@%s",
                   (url1_contact->username) ? url1_contact->username : "*NULL*",
                   (url1_contact->host) ? url1_contact->host : "*NULL*",
                   (url1_contact->username) ? url1_contact->username : "*NULL*",
                   configuration.masked_host.string[j]);
            urlmap[i].masq_url->host=realloc(urlmap[i].masq_url->host,
                                    strlen(configuration.masked_host.string[j])+1);
            strcpy(urlmap[i].masq_url->host, configuration.masked_host.string[j]);
         }
      } else { /* if new entry */
         /* This is an existing entry */
         /*
          * Some phones (like BudgeTones *may* dynamically grab a SIP port
          * so we might want to update the true_url and reg_url each time
          * we get an REGISTER
          */

         /* Contact: field (true_url) */
         osip_uri_free(urlmap[i].true_url);
         osip_uri_clone( ((osip_contact_t*)
                         (ticket->sipmsg->contacts.node->element))->url, 
                         &urlmap[i].true_url);
         /* To: field (reg_url) */
         osip_uri_free(urlmap[i].reg_url);
         osip_uri_clone( ticket->sipmsg->to->url, 
                         &urlmap[i].reg_url);
      }

      /*
       * for proxying: force device to be masqueraded
       * as with the outbound IP (masq_url)
       */
      if (force_lcl_masq) {
         struct in_addr addr;
         char *addrstr;

         if (get_interface_ip(IF_OUTBOUND, &addr) != STS_SUCCESS) {
            return STS_FAILURE;
         }

         /* host part */
         addrstr = utils_inet_ntoa(addr);
         DEBUGC(DBCLASS_REG,"masquerading UA %s@%s local %s@%s",
                (url1_contact->username) ? url1_contact->username : "*NULL*",
                (url1_contact->host) ? url1_contact->host : "*NULL*",
                (url1_contact->username) ? url1_contact->username : "*NULL*",
                addrstr);
         urlmap[i].masq_url->host=realloc(urlmap[i].masq_url->host,
                                          strlen(addrstr)+1);
         strcpy(urlmap[i].masq_url->host, addrstr);

         /* port number if required */
         if (configuration.sip_listen_port != SIP_PORT) {
            urlmap[i].masq_url->port=realloc(urlmap[i].masq_url->port, 16);
            sprintf(urlmap[i].masq_url->port, "%i",
                    configuration.sip_listen_port);
         }
      }

      /* give some safety margin for the next update */
      if (expires > 0) expires+=30;

      /* update registration timeout */
      urlmap[i].expires=time_now+expires;

   /*
    * un-REGISTER
    */
   } else { /* expires > 0 */
      /*
       * Remove registration
       * Siproxd will ALWAYS remove ALL bindings for a given
       * address-of-record
       */
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) continue;

         url2_to=urlmap[i].reg_url;

         if (compare_url(url1_to, url2_to)==STS_SUCCESS) {
            DEBUGC(DBCLASS_REG, "removing registration for %s@%s at slot=%i",
                   (url2_to->username) ? url2_to->username : "*NULL*",
                   (url2_to->host) ? url2_to->host : "*NULL*", i);
            urlmap[i].expires=0;
            break;
         }
      }
   }

   return STS_SUCCESS;
}



/*
 * cyclically called to do the aging of the URL mapping table entries
 * and throw out expired entries.
 * Also we do the cyclic saving here - if required.
 */
void register_agemap(void) {
   int i;
   time_t t;
   
   /* expire old entries */
   time(&t);
   DEBUGC(DBCLASS_BABBLE,"sip_agemap, t=%i",(int)t);
   for (i=0; i<URLMAP_SIZE; i++) {
      if ((urlmap[i].active == 1) && (urlmap[i].expires < t)) {
         DEBUGC(DBCLASS_REG,"cleaned entry:%i %s@%s", i,
                urlmap[i].masq_url->username,  urlmap[i].masq_url->host);
         urlmap[i].active=0;
         osip_uri_free(urlmap[i].true_url);
         osip_uri_free(urlmap[i].masq_url);
         osip_uri_free(urlmap[i].reg_url);
      }
   }

   /* auto-save of registration table */
   if ((configuration.autosave_registrations > 0) &&
       ((last_save + configuration.autosave_registrations) < t)) {
      register_save();
      last_save = t;
   }
   return;
}


/*
 * send answer to a registration request.
 *  flag = STS_SUCCESS    -> positive answer (200)
 *  flag = STS_FAILURE    -> negative answer (503)
 *  flag = STS_NEED_AUTH  -> proxy authentication needed (407)
 *
 * RETURNS
 *      STS_SUCCESS on success
 *      STS_FAILURE on error
 */
int register_response(sip_ticket_t *ticket, int flag) {
   osip_message_t *response;
   int code;
   int sts;
   osip_via_t *via;
   int port;
   char *buffer;
   size_t buflen;
   struct in_addr addr;
   osip_header_t *expires_hdr;

   /* ok -> 200, fail -> 503 */
   switch (flag) {
   case STS_SUCCESS:
      code = 200;       /* OK */
      break;
   case STS_FAILURE:
      code = 503;       /* failed */
      break;
   case STS_NEED_AUTH:
      code = 407;       /* proxy authentication needed */
      break;
   default:
      code = 503;       /* failed */
      break;
   }

   /* create the response template */
   if ((response=msg_make_template_reply(ticket, code))==NULL) {
      ERROR("register_response: error in msg_make_template_reply");
      return STS_FAILURE;
   }

   /* insert the expiration header */
   osip_message_get_expires(ticket->sipmsg, 0, &expires_hdr);
   if (expires_hdr) {
      osip_message_set_expires(response, expires_hdr->hvalue);
   }

   /* if we send back an proxy authentication needed, 
      include the Proxy-Authenticate field */
   if (code == 407) {
      auth_include_authrq(response);
   }

   /* get the IP address from existing VIA header */
   osip_message_get_via (response, 0, &via);
   if (via == NULL) {
      ERROR("register_response: Cannot send response - no via field");
      return STS_FAILURE;
   }

   /* name resolution needed? */
   if (utils_inet_aton(via->host,&addr) == 0) {
      /* yes, get IP address */
      sts = get_ip_by_host(via->host, &addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_REG, "register_response: cannot resolve VIA [%s]",
                via->host);
         return STS_FAILURE;
      }
   }   

   sts = sip_message_to_str(response, &buffer, &buflen);
   if (sts != 0) {
      ERROR("register_response: msg_2char failed");
      return STS_FAILURE;
   }

   /* send answer back */
   if (via->port) {
      port=atoi(via->port);
      if ((port<=0) || (port>65535)) port=SIP_PORT;
   } else {
      port=configuration.sip_listen_port;
   }

   sipsock_send(addr, port, ticket->protocol, buffer, buflen);

   /* free the resources */
   osip_message_free(response);
   free(buffer);
   return STS_SUCCESS;
}


/*
 * set expiration timeout as received with SIP response
 *
 * RETURNS
 *      STS_SUCCESS on success
 *      STS_FAILURE on error
 */
int register_set_expire(sip_ticket_t *ticket) {
   int i, j;
   int expires=-1;
   osip_contact_t *contact=NULL;
   time_t time_now;
   osip_header_t *expires_hdr=NULL;
   osip_uri_param_t *expires_param=NULL;

   if (ticket->direction != RESTYP_INCOMING) {
      WARN("register_set_expire called with != incoming response");
      return STS_FAILURE;
   }

   time(&time_now);

   DEBUGC(DBCLASS_REG,"REGISTER response, looking for 'Expire' information");

   /* evaluate Expires Header field */
   osip_message_get_expires(ticket->sipmsg, 0, &expires_hdr);

   /* loop for all existing contact headers in message */
   for (j=0; (contact != NULL) || (j==0); j++) {
      osip_message_get_contact(ticket->sipmsg, j, &contact);

     /*
      * look for an Contact expires parameter - in case of REGISTER
      * these two are equal. The Contact expires has higher priority!
      */
      if (contact==NULL) continue;

      osip_contact_param_get_byname(contact, EXPIRES, &expires_param);

      if (expires_param && expires_param->gvalue) {
         /* get expires from contact Header */
         expires=atoi(expires_param->gvalue);
         if ((expires < 0) || (expires >= UINT_MAX ))
            expires=configuration.default_expires;
      } else if (expires_hdr && expires_hdr->hvalue) {
         /* get expires from expires Header */
         expires=atoi(expires_hdr->hvalue);
         if ((expires < 0) || (expires >= UINT_MAX ))
            expires=configuration.default_expires;
      }

      DEBUGC(DBCLASS_REG,"Expires=%i, expires_param=%p, expires_hdr=%p",
             expires, expires_param, expires_hdr);
      if (expires > 0) {

         /* search for an entry */
         for (i=0;i<URLMAP_SIZE;i++){
            if (urlmap[i].active == 0) continue;
            if ((compare_url(contact->url, urlmap[i].masq_url)==STS_SUCCESS)) break;
         }

         /* found a mapping entry */
         if (i<URLMAP_SIZE) {
            /* update registration timeout */
            DEBUGC(DBCLASS_REG,"changing registration timeout to %i"
                               " entry [%i]", expires, i);
            urlmap[i].expires=time_now+expires;
         } else {
            DEBUGC(DBCLASS_REG,"no urlmap entry found");
         }
      }
   } /* for j */
   return STS_SUCCESS;
}
