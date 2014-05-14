/*
    Copyright (C) 2008  Thomas Ries <tries@gmx.net>

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

/* must be defined before including <plugin.h> */
#define PLUGIN_NAME	plugin_defaulttarget

#include "config.h"

#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: plugin_defaulttarget.c 440 2010-01-07 11:30:27Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_defaulttarget";
static char desc[]="Forwards all unknown calls to a default internal location";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;

/* plugin configuration storage */
static struct plugin_config {
   char *target;
   int   log;
} plugin_cfg;

/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_defaulttarget_target",      TYP_STRING, &plugin_cfg.target,	{0, NULL} },
   { "plugin_defaulttarget_log",         TYP_INT4,   &plugin_cfg.log,		{0, NULL} },
   {0, 0, 0}
};
/* local storage */
static osip_contact_t *default_target = NULL;


/* local prototypes */
static int plugin_defaulttarget_redirect(sip_ticket_t *ticket);


/* 
 * Initialization.
 * Called once suring siproxd startup.
 */
int  PLUGIN_INIT(plugin_def_t *plugin_def) {

   /* API version number of siproxd that this plugin is built against.
    * This constant will change whenever changes to the API are made
    * that require adaptions in the plugin. */
   plugin_def->api_version=SIPROXD_API_VERSION;

   /* Name and descriptive text of the plugin */
   plugin_def->name=name;
   plugin_def->desc=desc;

   /* Execution mask - during what stages of SIP processing shall
    * the plugin be called. */
   plugin_def->exe_mask=PLUGIN_DETERMINE_TARGET;

   /* read the config file */
   if (read_config(configuration.configfile,
                   configuration.config_search,
                   plugin_cfg_opts, name) == STS_FAILURE) {
      ERROR("Plugin '%s': could not load config file", name);
      return STS_FAILURE;
   }

   /* parse the default target URI testwise */
   osip_contact_init(&default_target);
   if (osip_contact_parse(default_target, plugin_cfg.target) != 0) {
      /* parsing failed, so the config file must contain CRAP */
      ERROR("%s: Illegal default target [%s] - cannot parse!",
            name, plugin_cfg.target);
      return STS_FAILURE;
   }


   return STS_SUCCESS;
}

/*
 * Processing.
 * 
 */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   /* stage contains the PLUGIN_* value - the stage of SIP processing. */
   int sts;

   sts = STS_SUCCESS;
   sip_find_direction(ticket, NULL);

   /* direction is unknown and SIP message is a REQUEST */
   if ((ticket->direction == DIRTYP_UNKNOWN) &&
       (MSG_IS_REQUEST(ticket->sipmsg)) &&
       (MSG_IS_INVITE(ticket->sipmsg))) {

      /* LOG if master wishes so */
      if (plugin_cfg.log) {
         osip_uri_t *to_url=ticket->sipmsg->to->url;
         osip_uri_t *from_url=ticket->sipmsg->from->url;
         INFO("Unknown Target (from: %s@%s), redirecting %s@%s -> %s",
              from_url->username? from_url->username:"*NULL*",
              from_url->host?     from_url->host:    "*NULL*",
              to_url->username?   to_url->username:"*NULL*",
              to_url->host?       to_url->host:    "*NULL*",
              plugin_cfg.target);
      }

      /* target defined in config file */
      if (plugin_cfg.target) {
         /* reply with a "302 Moved temporarily" back to the sender */
         sts = plugin_defaulttarget_redirect(ticket);
      }
      return sts;
   }

   /* Catch the ACK following the redirect */
   if ((ticket->direction == DIRTYP_UNKNOWN) &&
       (MSG_IS_REQUEST(ticket->sipmsg)) &&
       (MSG_IS_ACK(ticket->sipmsg))) {
      /* eat it up and don't react */
      return STS_SIP_SENT;
   }
   return STS_SUCCESS;
}

/*
 * De-Initialization.
 * Called during shutdown of siproxd. Gives the plugin the chance
 * to clean up its mess (e.g. dynamic memory allocation, database
 * connections, whatever the plugin messes around with)
 */
int  PLUGIN_END(plugin_def_t *plugin_def){
   return STS_SUCCESS;
}


/* private plugin code */
static int plugin_defaulttarget_redirect(sip_ticket_t *ticket) {
   int i;
   osip_contact_t *contact = NULL;

   /* remove all Contact headers in message */
   for (i=0; (contact != NULL) || (i == 0); i++) {
      osip_message_get_contact(ticket->sipmsg, 0, &contact);
      if (contact) {
         osip_list_remove(&(ticket->sipmsg->contacts),0);
         osip_contact_free(contact);
      }
   } /* for i */

   /* use a "302 Moved temporarily" response back to the client */
   /* new target is within the Contact Header */

   /* Clone the target contact header we initialized during plugin load */
   osip_contact_init(&contact);
   osip_contact_clone(default_target, &contact);

   osip_list_add(&(ticket->sipmsg->contacts),contact,0);

   /* sent redirect message back to local client */
   sip_gen_response(ticket, 302 /*Moved temporarily*/);

   return STS_SIP_SENT;
}
