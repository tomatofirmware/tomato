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

/* must be defined before including <plugin.h> */
#define PLUGIN_NAME	plugin_shortdial

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: plugin_shortdial.c 466 2010-10-13 22:48:27Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_shortdial";
static char desc[]="Handles Dial shortcuts as defined in config file";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;


/* plugin configuration storage */
static struct plugin_config {
   char *shortdial_akey;
   stringa_t shortdial_entry;
} plugin_cfg;
/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_shortdial_akey",   TYP_STRING, &plugin_cfg.shortdial_akey,		{0, NULL} },
   { "plugin_shortdial_entry",  TYP_STRINGA,&plugin_cfg.shortdial_entry,	{0, NULL} },
   {0, 0, 0}
};


/* local prototypes */
static int plugin_shortdial_redirect(sip_ticket_t *ticket, int shortcut_no);
static int plugin_shortdial(sip_ticket_t *ticket);


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
   sts=plugin_shortdial(ticket);
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
static int plugin_shortdial(sip_ticket_t *ticket) {
   int sts=STS_SUCCESS;
   osip_uri_t *req_url;
   int  shortcut_no=0;

   /* plugin loaded and not configured, return with success */
   if (plugin_cfg.shortdial_akey==NULL) return STS_SUCCESS;
   if (plugin_cfg.shortdial_entry.used==0) return STS_SUCCESS;

   DEBUGC(DBCLASS_PLUGIN,"plugin entered");
   req_url=osip_message_get_uri(ticket->sipmsg);

   /* only outgoing direction is handled */
   sip_find_direction(ticket, NULL);
   if (ticket->direction != DIR_OUTGOING)
      return STS_SUCCESS;

   /* only INVITE and ACK are handled */
   if (!MSG_IS_INVITE(ticket->sipmsg) && !MSG_IS_ACK(ticket->sipmsg))
      return STS_SUCCESS;

   /* REQ URI with username must exist, length as defined in config,
    * shortdial must be enabled and short dial key must match */
   if (!req_url || !req_url->username ||
       !plugin_cfg.shortdial_akey ||
       (strlen(req_url->username) != strlen(plugin_cfg.shortdial_akey)) ||
       (req_url->username[0] != plugin_cfg.shortdial_akey[0]))
      return STS_SUCCESS; /* ignore */

   shortcut_no = atoi(&(req_url->username[1]));
   if ((shortcut_no <= 0) || (shortcut_no >= INT_MAX)) return STS_SUCCESS; /* not a number */

   /* requested number is not defined (out of range) */
   if (shortcut_no > plugin_cfg.shortdial_entry.used) {
      DEBUGC(DBCLASS_PLUGIN, "shortdial: shortcut %i > available shortcuts (%i)",
            shortcut_no, plugin_cfg.shortdial_entry.used);
      return STS_SUCCESS;
   }

   /* requested number is not defined (empty) */
   if (!plugin_cfg.shortdial_entry.string[shortcut_no-1]) {
      DEBUGC(DBCLASS_PLUGIN, "shortdial: shortcut %i empty", shortcut_no);
      return STS_SUCCESS;
   }

   /*
    * called number does match the short dial specification
    */

   /* outgoing INVITE request */
   if (MSG_IS_INVITE(ticket->sipmsg)) {
      DEBUGC(DBCLASS_PLUGIN,"processing INVITE");
      sts=plugin_shortdial_redirect(ticket, shortcut_no);
   }
   /* outgoing ACK request: is result of a local 3xx answer (moved...) */
   else if (MSG_IS_ACK(ticket->sipmsg)) {
      /* make sure we only catch ACKs caused by myself (**02 -> *02 legitime) */
      DEBUGC(DBCLASS_PLUGIN,"processing ACK");
      sts=STS_SIP_SENT; /* eat up the ACK that was directed to myself */
   }

   return sts;
}


/* private plugin code */
static int plugin_shortdial_redirect(sip_ticket_t *ticket, int shortcut_no) {
   osip_uri_t *to_url=ticket->sipmsg->to->url;
   char *to_user=to_url->username;
   char *new_to_user=NULL;
   char *new_to_host=NULL;
   int  i;
   size_t username_len;
   size_t host_len=0;
   osip_contact_t *contact = NULL;

   new_to_user=plugin_cfg.shortdial_entry.string[shortcut_no-1];
   if (!new_to_user) return STS_SUCCESS;

   DEBUGC(DBCLASS_PLUGIN,"redirect: redirecting [%s]->[%s]",
          to_user, new_to_user);

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

   /* get info about new target (user and optional host part) */
   username_len=strlen(new_to_user);	/* excluding \0 */

   /* check if there is an '@' in the shortdial entry */
   new_to_host = strstr(new_to_user, "@"); 
   if (new_to_host) {
      host_len=strlen(new_to_host)-1;	/* don't count '@' */
      /* only include the username length */
      username_len = (size_t)(new_to_host-new_to_user); 
      if (host_len == 0) {
         /* if '@' is the last character, then dont rewrite host */
         new_to_host = NULL;
      } else {
         /*  advance one character for the pointer to the host part (skip @) */
         new_to_host++;
      }
   }

   /* insert one new Contact header containing the new target address */
   osip_contact_init(&contact);
   osip_uri_clone(to_url, &contact->url);

   /* USER part is always present */
   osip_free(contact->url->username);
   contact->url->username=osip_malloc(username_len+1); /* *_len excluding \0 */

   /* only copy the part that really belongs to the username */
   strncpy(contact->url->username, new_to_user, username_len);
   /* strncpy does not terminate - do it manually */
   contact->url->username[username_len]='\0';

   /* HOST part is optional */
   if (new_to_host) {
      osip_free(contact->url->host);
      contact->url->host=osip_malloc(host_len+1); /* *_len excluding \0 */
      strcpy(contact->url->host, new_to_host);
   }

   osip_list_add(&(ticket->sipmsg->contacts),contact,0);

   /* sent redirect message back to local client */
   sip_gen_response(ticket, 302 /*Moved temporarily*/);

   return STS_SIP_SENT;
}
