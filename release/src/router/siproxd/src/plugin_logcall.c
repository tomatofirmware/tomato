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
#define PLUGIN_NAME	plugin_logcall

#include "config.h"

#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: plugin_logcall.c 477 2011-06-01 08:58:31Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_logcall";
static char desc[]="Logs calls to syslog";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;


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
   plugin_def->exe_mask=PLUGIN_PRE_PROXY;

   return STS_SUCCESS;
}

/*
 * Processing.
 * 
 */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   osip_message_t *request;
   osip_uri_t *from_url = NULL;
   osip_uri_t *to_url   = NULL;
   osip_uri_t *req_uri = NULL;
   char *to_username =NULL;
   char *to_host = NULL;
   char *from_username =NULL;
   char *from_host = NULL;
   char *call_type = NULL;

   request=ticket->sipmsg;
   req_uri = request->req_uri;

   /* From: 1st preference is From header, then try contact header */
   if (request->from->url) {
      from_url = request->from->url;
   } else {
      from_url = (osip_uri_t *)osip_list_get(&(request->contacts), 0);
   }

   to_url = request->to->url;

   if (to_url) {
      to_username = to_url->username;
      to_host = to_url->host;
   }

   if (from_url) {
      from_username = from_url->username;
      from_host = from_url->host;
   }

   /* INVITE */
   if (MSG_IS_INVITE(request)) {
      if (ticket->direction==REQTYP_INCOMING) call_type="Incoming";
      else call_type="Outgoing";
   /* BYE / CANCEL */
   } else if (MSG_IS_ACK(request)) {
      call_type="ACK";
   } else if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
      call_type="Ending";
   }

   if (call_type) {
      INFO("%s Call: %s@%s -> %s@%s [Req: %s@%s]",
           call_type,
           from_username ? from_username: "*NULL*",
           from_host     ? from_host    : "*NULL*",
           to_username   ? to_username  : "*NULL*",
           to_host       ? to_host      : "*NULL*",
           (req_uri && req_uri->username) ? req_uri->username : "*NULL*",
           (req_uri && req_uri->host)     ? req_uri->host     : "*NULL*"
           );
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

