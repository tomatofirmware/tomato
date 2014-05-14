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
#include "redirect_cache.h"
#include "log.h"

static char const ident[]="$Id: plugin_prefix.c 484 2011-06-17 16:15:41Z hb9xar $";

#define CACHE_TIMEOUT  20

/*
 * Helper for siproxd plugins that operate with 302 redirection
 * like the plugin_shortdial, plugin_prefix or plugin_regex .
 *
 * The plugin must consume the ACK from the local UA caused by the
 * 302 Moved response. No other ACKs must be dropped by the plugin.
 *
 * The recommended way is that upon sending a "302 Moved" response
 * towards the local UA, the plugin does put the Call-Id of this
 * dialog into a cache. Any ACK with matching Call-Id will be dropped
 * (consumed by the plugin) but nothing more.
 *
 * Below is a set of 3 functions to implement this functionality
 * inside a plugin. The whole SIP ticket will be passed, so 
 * modifying the matching criteria can be easily done, transparent
 * to the plugins.
 *
 * This is the "redirected_cache". Each plugin needs its own private
 * cache. A static Queue header for the cache must be created and
 * passed to the cache handling functions.
 *
 * static redirected_cache_element_t redirected_cache;
 * [...]
 * add_to_redirected_cache(&redirected_cache, ticket);
 *
 */

int add_to_redirected_cache(redirected_cache_element_t *redirected_cache, sip_ticket_t *ticket) {
   redirected_cache_element_t *e;
   DEBUGC(DBCLASS_PLUGIN, "entered add_to_redirected_cache()");
   
   /* allocate */
   e=malloc(sizeof(redirected_cache_element_t));
   if (e == NULL) {
       ERROR("out of memory");
       return  STS_FAILURE;
   }

   /* populate element */
   e->next = NULL;
   e->ts   = time(NULL);
   osip_call_id_clone(ticket->sipmsg->call_id, &(e->call_id));

   /* add to head of queue */
   e->next = redirected_cache->next;
   redirected_cache->next = e;

   DEBUGC(DBCLASS_PLUGIN, "left add_to_redirected_cache()");
   return STS_SUCCESS;
}

int is_in_redirected_cache(redirected_cache_element_t *redirected_cache, sip_ticket_t *ticket) {
   redirected_cache_element_t *p, *p_prev;

   DEBUGC(DBCLASS_BABBLE, "entered is_in_redirected_cache");
   /* iterate through queue */
   p_prev=NULL;
   for (p=redirected_cache; p; p=p->next) {
      DEBUGC(DBCLASS_BABBLE, "l: p=%p, p->next=%p", p, p->next);
      if ( (p != redirected_cache) && (p_prev != NULL) ) {
         if (compare_callid(ticket->sipmsg->call_id, p->call_id) == STS_SUCCESS) {
            DEBUGC(DBCLASS_BABBLE, "remove p=%p", p);
            /* remove from queue */
            p_prev->next = p->next;
            osip_call_id_free (p->call_id);
            free(p);
            DEBUGC(DBCLASS_BABBLE, "left is_in_redirected_cache - FOUND");
            return STS_TRUE;
         } /* if compare_callid */
      }
      p_prev = p;
   } /* for */
   DEBUGC(DBCLASS_BABBLE, "left is_in_redirected_cache - NOT FOUND");
   return STS_FALSE;
}

/*
 * Run through the whole Call-Id cache and remove
 * expired elements.
 */
int expire_redirected_cache(redirected_cache_element_t *redirected_cache) {
   redirected_cache_element_t *p, *p_prev;
   time_t now;

   DEBUGC(DBCLASS_BABBLE, "entered expire_redirected_cache");
   now = time(NULL);

   /* iterate through queue */
   p_prev=NULL;
   for (p=redirected_cache; p; p=p->next) {
      DEBUGC(DBCLASS_BABBLE, "1: p=%p, p->next=%p", p, p->next);
      if ( (p != redirected_cache) && (p_prev != NULL) ) {
         DEBUGC(DBCLASS_BABBLE,"ts:%i, now:%i", (int)p->ts, (int)now);
         if ((p->ts + CACHE_TIMEOUT) < now) {
            DEBUGC(DBCLASS_BABBLE, "remove p=%p", p);
            /* remove from queue */
            p_prev->next = p->next;
            osip_call_id_free (p->call_id);
            free(p);
            /* the current element is being removed and invalidated,
             * set the iteration pointer to a valid element. */
            p = p_prev;
         } /* if timeout */
         DEBUGC(DBCLASS_BABBLE, "2: p=%p, p->next=%p", p, p->next);
      }
      p_prev = p;
   } /* for */
   DEBUGC(DBCLASS_BABBLE, "left expire_redirected_cache");
   return STS_FALSE;
}
