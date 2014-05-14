/*
    Copyright (C) 2002-2011  Thomas Ries <tries@gmx.net>

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

/*
 * plugin_prefix is based on plugin_shortdial
 * 
 * This plugin adds a configured prefix to outgoing calls.
 * Typical use is to add a dial-out code ('9' or '0') to 
 * cover user experience with POTS dialing.
 */


/* must be defined before including <plugin.h> */
#define PLUGIN_NAME	plugin_prefix

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "redirect_cache.h"
#include "log.h"

static char const ident[]="$Id: plugin_prefix.c 485 2011-06-18 08:23:54Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_prefix";
static char desc[]="Adds a dial-prefix as defined in config file";

/* constants */
#define REDIRECTED_TAG "redirected"
#define REDIRECTED_VAL "prefix"

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;


/* plugin configuration storage */
static struct plugin_config {
   char *prefix_akey;
} plugin_cfg;

/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_prefix_akey",   TYP_STRING, &plugin_cfg.prefix_akey,		{0, NULL} },
   {0, 0, 0}
};


/* local storage needed by plugin */
/* Redirect Cache: Queue Head is static */
static redirected_cache_element_t redirected_cache;


/* local prototypes */
static int plugin_prefix_redirect(sip_ticket_t *ticket);
static int plugin_prefix(sip_ticket_t *ticket);

/* 
 * Plugin API functions code
 */
/* Initialization */
int  PLUGIN_INIT(plugin_def_t *plugin_def) {
   plugin_def->api_version=SIPROXD_API_VERSION;
   plugin_def->name=name;
   plugin_def->desc=desc;
   plugin_def->exe_mask=PLUGIN_DETERMINE_TARGET;

   /* read the config file */
   if (read_config(configuration.configfile,
                   configuration.config_search,
                   plugin_cfg_opts, name) == STS_FAILURE) {
      ERROR("Plugin '%s': could not load config file", name);
      return STS_FAILURE;
   }

   return STS_SUCCESS;
}

/* Processing */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   int sts;
   sts=plugin_prefix(ticket);
   return sts;
}

/* De-Initialization */
int  PLUGIN_END(plugin_def_t *plugin_def){
   return STS_SUCCESS;
}


/*
 * Workload code
 */

/* returns STS_SIP_SENT if processing is to be terminated,
 * otherwise STS_SUCCESS (go on with processing) */
/* code (entry point) */
static int plugin_prefix(sip_ticket_t *ticket) {
   int sts=STS_SUCCESS;
   osip_uri_t *req_url;
   osip_uri_t *to_url;
   osip_generic_param_t *r=NULL;

   /* plugin loaded and not configured, return with success */
   if (plugin_cfg.prefix_akey == NULL) return STS_SUCCESS;

   DEBUGC(DBCLASS_PLUGIN,"plugin entered");
   req_url=osip_message_get_uri(ticket->sipmsg);
   to_url=osip_to_get_url(ticket->sipmsg);

   /* only outgoing direction is handled */
   sip_find_direction(ticket, NULL);
   if (ticket->direction != DIR_OUTGOING)
      return STS_SUCCESS;

   /* only INVITE and ACK are handled */
   if (!MSG_IS_INVITE(ticket->sipmsg) && !MSG_IS_ACK(ticket->sipmsg))
      return STS_SUCCESS;

   /* expire old cache entries */
   expire_redirected_cache(&redirected_cache);

   /* REQ URI with username must exist, prefix string must exist */
   if (!req_url || !req_url->username || !plugin_cfg.prefix_akey)
      return STS_SUCCESS; /* ignore */

   /* Loop avoidance:
    * If this INVITE has already been redirected by a prior 302
    * moved response a "REDIRECTED_TAG" parameter should be present in the
    * URI.
    * Hopefully all UAs (Clients) do honor RFC3261 and copy the
    * *full* URI form the contact header into the new request header
    * upon a 3xx response.
    */
   if (req_url) {
      osip_uri_param_get_byname(&(req_url->url_params), REDIRECTED_TAG, &r);
      if (r && r->gvalue && strcmp(r->gvalue,REDIRECTED_VAL)== 0) {
         DEBUGC(DBCLASS_PLUGIN,"Packet has already been redirected (ReqURI)");
         return STS_SUCCESS;
      }
   }
   if (to_url) {
      osip_uri_param_get_byname(&(to_url->url_params), REDIRECTED_TAG, &r);
      if (r && r->gvalue && strcmp(r->gvalue,REDIRECTED_VAL)== 0) {
         DEBUGC(DBCLASS_PLUGIN,"Packet has already been redirected (ToURI)");
         return STS_SUCCESS;
      }
   }

   /*
    * The called number is to be prefixed
    */

   /* outgoing INVITE request */
   if (MSG_IS_INVITE(ticket->sipmsg)) {
      DEBUGC(DBCLASS_PLUGIN,"processing INVITE");
      sts=plugin_prefix_redirect(ticket);
   }
   /* outgoing ACK request: is result of a local 3xx answer (moved...)
    *
    * Only consume that particular ACK that belongs to a sent 302 answer,
    * nothing else. Otherwise the ACK from the redirected call will get 
    * consumed as well and causes the call to be aborted (timeout).
    * We keep a cache with Call-Ids of such "302 moved" dialogs.
    * Only consume such ACKs that are part of such a dialog.
    */
   else if (MSG_IS_ACK(ticket->sipmsg)) {
      if (is_in_redirected_cache(&redirected_cache, ticket) == STS_TRUE) {
	 DEBUGC(DBCLASS_PLUGIN,"processing ACK (consume it)");
	 sts=STS_SIP_SENT; /* eat up the ACK that was directed to myself */
      }
   }

   return sts;
}


/* private plugin code */
static int plugin_prefix_redirect(sip_ticket_t *ticket) {
   osip_uri_t *to_url=ticket->sipmsg->to->url;
   char *to_user=to_url->username;
   char *new_to_user=NULL;
   int  i;
   size_t username_len;
   osip_contact_t *contact = NULL;

   /* including \0 + leading character(s) */
   username_len=strlen(to_user) + strlen(plugin_cfg.prefix_akey) + 1;

   new_to_user = osip_malloc(username_len); /* *_len excluding \0 */
   if (!new_to_user) return STS_SUCCESS;

   /* use a "302 Moved temporarily" response back to the client */
   /* new target is within the Contact Header */

   /* remove all Contact headers in message */
   for (i=0; (contact != NULL) || (i == 0); i++) {
      osip_message_get_contact(ticket->sipmsg, 0, &contact);
      if (contact) {
         osip_list_remove(&(ticket->sipmsg->contacts),0);
         osip_contact_free(contact);
      }
   } /* for i */

   /* insert one new Contact header containing the new target address */
   osip_contact_init(&contact);
   osip_uri_clone(to_url, &contact->url);
   
   /*
    * Add the 'REDIRECTED_TAG=REDIRECTED_VAL' parameter to URI. Required to figure out
    * if this INVITE has already been processed (redirected) and
    * does not need further attention by this plugin.
    * THIS IS REQUIRED TO AVOID A LOOP
    */
   osip_uri_param_add(&(contact->url->url_params), osip_strdup(REDIRECTED_TAG), 
                      osip_strdup(REDIRECTED_VAL));

   /* only copy the part that really belongs to the username */
   snprintf(new_to_user, username_len, "%s%s",
            plugin_cfg.prefix_akey,to_user );

   /* strncpy may not terminate - do it manually to be sure */
   new_to_user[username_len-1]='\0';
   osip_list_add(&(ticket->sipmsg->contacts),contact,0);

   INFO("redirecting %s -> %s", to_user, new_to_user);

   /* USER part is always present */
   osip_free(contact->url->username);
   contact->url->username=new_to_user;

   /* sent redirect message back to local client */
   add_to_redirected_cache(&redirected_cache, ticket);
   sip_gen_response(ticket, 302 /*Moved temporarily*/);

   return STS_SIP_SENT;
}
