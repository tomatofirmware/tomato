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
 * This plugin adds a regular expression rewrite support
 * for SIP targets.
 */


/* must be defined before including <plugin.h> */
#define PLUGIN_NAME	plugin_regex

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "plugins.h"
#include "redirect_cache.h"
#include "log.h"

static char const ident[]="$Id: plugin_regex.c 471 2011-05-28 10:03:49Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_regex";
static char desc[]="Use regular expressions to rewrite SIP targets";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;

/* constants */
#define REDIRECTED_TAG "redirected"
#define REDIRECTED_VAL "regex"

/* plugin configuration storage */
static struct plugin_config {
   stringa_t regex_desc;
   stringa_t regex_pattern;
   stringa_t regex_replace;
} plugin_cfg;

/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_regex_desc",  TYP_STRINGA,&plugin_cfg.regex_desc,	{0, NULL} },
   { "plugin_regex_pattern",  TYP_STRINGA,&plugin_cfg.regex_pattern,	{0, NULL} },
   { "plugin_regex_replace",  TYP_STRINGA,&plugin_cfg.regex_replace,	{0, NULL} },
   {0, 0, 0}
};

/* local storage needed for regular expression handling */
static regex_t *re;
/* Redirect Cache: Queue Head is static */
static redirected_cache_element_t redirected_cache;


/* local prototypes */
static int plugin_regex_init(void);
static int plugin_regex_process(sip_ticket_t *ticket);
static int plugin_regex_redirect(sip_ticket_t *ticket);
regmatch_t * rmatch (char *buf, int size, regex_t *re);
int rreplace (char *buf, int size, regex_t *re, regmatch_t pmatch[], char *rp);


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
   
   return plugin_regex_init();
}

/* Processing */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   int sts;
   sts=plugin_regex_process(ticket);
   return sts;
}

/* De-Initialization */
int  PLUGIN_END(plugin_def_t *plugin_def){
   return STS_SUCCESS;
}


/*
 * Workload code
 */
int plugin_regex_init(void) {
   int i;
   int sts, retsts;
   int num_entries;
   char errbuf[256];

   retsts = STS_SUCCESS;

   /* check for equal entries of patterns and replacements */
   if (plugin_cfg.regex_pattern.used != plugin_cfg.regex_replace.used) {
      ERROR("Plugin '%s': number of search patterns (%i) and number of "
            "replacement patterns (%i) differ!", name,
            plugin_cfg.regex_pattern.used, plugin_cfg.regex_replace.used);
      return STS_FAILURE;
   }

   if (plugin_cfg.regex_pattern.used != plugin_cfg.regex_desc.used) {
      ERROR("Plugin '%s': number of search patterns (%i) and number of "
            "descriptions (%i) differ!", name,
            plugin_cfg.regex_pattern.used, plugin_cfg.regex_desc.used);
      return STS_FAILURE;
   }

   /* allocate space for regexes and compile them */
   num_entries = plugin_cfg.regex_pattern.used;
   re = malloc(num_entries*sizeof(re[0]));
   for (i=0; i < num_entries; i++) {
      sts = regcomp (&re[i], plugin_cfg.regex_pattern.string[i],
                     REG_ICASE|REG_EXTENDED);
      if (sts != 0) {
         regerror(sts, &re[i], errbuf, sizeof(errbuf));
         ERROR("Regular expression [%s] failed to compile: %s", 
               plugin_cfg.regex_pattern.string[i], errbuf);
         retsts = STS_FAILURE;
      }
   }
   
   return retsts;
}
/* returns STS_SIP_SENT if processing is to be terminated,
 * otherwise STS_SUCCESS (go on with processing) */
/* code (entry point) */
static int plugin_regex_process(sip_ticket_t *ticket) {
   int sts=STS_SUCCESS;
   osip_uri_t *req_url;
   osip_uri_t *to_url;
   osip_generic_param_t *r=NULL;

   /* plugin loaded and not configured, return with success */
   if (plugin_cfg.regex_pattern.used==0) return STS_SUCCESS;
   if (plugin_cfg.regex_replace.used==0) return STS_SUCCESS;

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
   if (!req_url || !req_url->username)
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
         DEBUGC(DBCLASS_PLUGIN,"Packet has already been processed (ReqURI)");
         return STS_SUCCESS;
      }
   }
   if (to_url) {
      osip_uri_param_get_byname(&(to_url->url_params), REDIRECTED_TAG, &r);
      if (r && r->gvalue && strcmp(r->gvalue,REDIRECTED_VAL)== 0) {
         DEBUGC(DBCLASS_PLUGIN,"Packet has already been processed (ToURI)");
         return STS_SUCCESS;
      }
   }

   /*
    * The SIP message is to be processed
    */

   /* outgoing INVITE request */
   if (MSG_IS_INVITE(ticket->sipmsg)) {
      DEBUGC(DBCLASS_PLUGIN,"processing INVITE");
      sts=plugin_regex_redirect(ticket);
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
static int plugin_regex_redirect(sip_ticket_t *ticket) {
   osip_uri_t *to_url=ticket->sipmsg->to->url;
   char *url_string=NULL;
   osip_uri_t *new_to_url;
   int  i, sts;
   osip_contact_t *contact = NULL;
   /* character workspaces for regex */
   #define WORKSPACE_SIZE 128
   static char in[WORKSPACE_SIZE+1], rp[WORKSPACE_SIZE+1];

   /* do apply to full To URI... */
   sts = osip_uri_to_str(to_url, &url_string);
   if (sts != 0) {
      ERROR("osip_uri_to_str() failed");
      return STS_FAILURE;
   }
   DEBUGC(DBCLASS_BABBLE, "To URI string: [%s]", url_string);

   /* perform search and replace of the regexes, first match hits */
   for (i = 0; i < plugin_cfg.regex_pattern.used; i++) {
      regmatch_t *pmatch = NULL;
      pmatch = rmatch(url_string, WORKSPACE_SIZE, &re[i]);
      if (pmatch == NULL) continue; /* no match, next */

      /* have a match, do the replacement */
      INFO("Matched rexec rule: %s",plugin_cfg.regex_desc.string[i] );
      strncpy (in, url_string, WORKSPACE_SIZE);
      in[WORKSPACE_SIZE]='\0';
      strncpy (rp, plugin_cfg.regex_replace.string[i], WORKSPACE_SIZE);
      rp[WORKSPACE_SIZE]='\0';

      sts = rreplace(in, WORKSPACE_SIZE, &re[i], pmatch, rp);
      if (sts != STS_SUCCESS) {
         ERROR("regex replace failed: pattern:[%s] replace:[%s]",
               plugin_cfg.regex_pattern.string[i],
               plugin_cfg.regex_replace.string[i]);
         osip_free(url_string);
         return STS_FAILURE;
      }
      /* only do first match */
      break;
   }
   if (i >= plugin_cfg.regex_pattern.used) {
      /* no match */
      osip_free(url_string);
      return STS_SUCCESS;
   }
   /* in: contains the new string */

   sts = osip_uri_init(&new_to_url);
   if (sts != 0) {
      ERROR("Unable to initialize URI");
      osip_free(url_string);
      return STS_FAILURE;
   }

   sts = osip_uri_parse(new_to_url, in);
   if (sts != 0) {
      ERROR("Unable to parse To URI: %s", in);
      osip_uri_free(new_to_url);
      osip_free(url_string);
      return STS_FAILURE;
   }

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
   osip_list_add(&(ticket->sipmsg->contacts),contact,0);
   
   /* link the new_to_url into the Contact list */
   contact->url = new_to_url;
   new_to_url = NULL;

   /*
    * Add the 'REDIRECTED_TAG=REDIRECTED_VAL' parameter to URI. Required to figure out
    * if this INVITE has already been processed (redirected) and
    * does not need further attention by this plugin.
    * THIS IS REQUIRED TO AVOID A LOOP
    */
   osip_uri_param_add(&(contact->url->url_params), osip_strdup(REDIRECTED_TAG), 
                      osip_strdup(REDIRECTED_VAL));

   INFO("redirecting %s -> %s", url_string, in);

   /* sent redirect message back to local client */
   add_to_redirected_cache(&redirected_cache, ticket);
   sip_gen_response(ticket, 302 /*Moved temporarily*/);

   /* release resources and return */
   osip_free(url_string);
   return STS_SIP_SENT;
}

/*
 * This regex replacement code has been proudly borrowed from
 * http://www.daniweb.com/software-development/c/code/216955#
 *
 * buf: input string + output result
 * rp: replacement string, will be destroyed during processing!
 * size: size of buf and rp
 * re: regex to process
 *
 * rmatch() performs the initial regexec match, and if a match is found
 * it returns a pointer to the regmatch array which contains the result
 * of the match.
 * Afterwards rreplace() is to be called, providing this regmatch array.
 *
 * This eliminates the need to copy the 'rp' string before knowing
 * if a match is actually there.
 */
#define NMATCHES 10
regmatch_t * rmatch (char *buf, int size, regex_t *re) {
   static regmatch_t pm[NMATCHES]; /* regoff_t is int so size is int */

   /* perform the match */
   if (regexec (re, buf, NMATCHES, pm, 0)) {
      return NULL;
   }
   return &pm[0];
}

int rreplace (char *buf, int size, regex_t *re, regmatch_t pmatch[], char *rp) {
   char *pos;
   int sub, so, n;

   /* match(es) found: */
   for (pos = rp; *pos; pos++) {
      /* back references \1 ... \9: expand them in 'rp' */
      if (*pos == '\\' && *(pos + 1) > '0' && *(pos + 1) <= '9') {
         so = pmatch[*(pos + 1) - 48].rm_so;	/* pmatch[1..9] */
         n = pmatch[*(pos + 1) - 48].rm_eo - so;
         if (so < 0 || strlen (rp) + n - 1 > size) return STS_FAILURE;
         memmove (pos + n, pos + 2, strlen (pos) - 1);
         memmove (pos, buf + so, n);
         pos = pos + n - 2;
      }
   }

   sub = pmatch[1].rm_so; /* no repeated replace when sub >= 0 */
   /* and replace rp in the input buffer */
   for (pos = buf; !regexec (re, pos, 1, pmatch, 0); ) {
      n = pmatch[0].rm_eo - pmatch[0].rm_so;
      pos += pmatch[0].rm_so;
      if (strlen (buf) - n + strlen (rp) > size) {
         return STS_FAILURE;
      }
      memmove (pos + strlen (rp), pos + n, strlen (pos) - n + 1);
      memmove (pos, rp, strlen (rp));
      pos += strlen (rp);
      if (sub >= 0) break;
   }
   return STS_SUCCESS;
}
