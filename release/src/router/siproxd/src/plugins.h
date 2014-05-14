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

/* $Id: plugins.h 463 2010-06-19 20:39:11Z hb9xar $ */

#ifdef LTDL_CONVLIB
   /* fetch local version of ltdl */
   #include "../libltdl/ltdl.h"
#else
   /* fetch system version of libltdl */
   #include <ltdl.h>
#endif


/* Plugins must return STS_SUCCESS / SUCCESS_FAILURE */


/*
 * Processing stages for Plugins
 */
/* get cyclic trigger */
/* NO ticket is present (ticket = NULL pointer) */
#define PLUGIN_TIMER		0x00000001

/* Process RAW data received */
/* may end the current SIP processing in siproxd by returning STS_FALSE *
 * may be used to intercept other traffic on SIP port */
/* ticket with NO sipmsg is present (ticket.sipmsg = NULL pointer) */
#define PLUGIN_PROCESS_RAW	0x00000005

/*--------- below here a valid sip message (ticket->sipmsg) is present ---*/

/* Validation of SIP packet */
/* may end the current SIP processing in siproxd by returning STS_FALSE *
 * may be used to intercept other traffic on SIP port */
#define PLUGIN_VALIDATE		0x00000010	

/* Determining Request Targets */
/* may end the current SIP processing in siproxd by returning STS_SIP_SENT
 * see plugin_shortcut that sends a redirect back to the client */
#define PLUGIN_DETERMINE_TARGET	0x00000020	/* Determining Request Targets */

/* SIP package before siproxd starts the proxying process */
#define PLUGIN_PRE_PROXY	0x00000040	/* before MASQuerading */

/* to/from unregistered UA */
#define PLUGIN_PROXY_UNK	0x00000080	/* e.g. incoming call to unknown UA */

/* before sending the SIP message */
#define PLUGIN_POST_PROXY	0x00000100	/* after MASQuerading */


/* Plugin "database" */
typedef struct {
   void *next;		/* link to next plugin element, NULL if last */
   int  api_version;	/* API version that PLUGIN uses */
   char *name;		/* Plugin name */
   char *desc;		/* Description */
   int  exe_mask;	/* bitmask for activation of different processing
   			   stages during SIP processing that a plugin wants
   			   to be called */
   lt_ptr plugin_process;/* Plugin processing entry point */
   lt_ptr plugin_end;	/* de-initialization function */
   lt_ptr dlhandle;	/* handle returned by dlopen() */
} plugin_def_t;

#define SIPROXD_API_VERSION	0x0101


/* The plugin must provide the following entry points */
/* Plugin is responsable for its dynamic memory management.
   - Storage allocated in _init must be released in _end.
   - manipulation of SIP messages (sip_ticket structure) must
     be made in a way to ensure that no memleaks do exist.
     If you want to change a field, first osip_malloc() new space,
     move the pointer in the osip structure to the new place and then
     osip_free() the old area.
*/
/* plugin_init must define the following fields of the plugin_def_t structure:
   - api_version	(= SIPROXD_API_VERSION)
   - name
   - desc
   - exe_mask
   The rest will be initialized by siproxd and must not be fumbled with.
*/
int  plugin_init(plugin_def_t *plugin_def);
int  plugin_process(int stage, sip_ticket_t *ticket);
int  plugin_end(plugin_def_t *plugin_def);

/* libltdl symbol name magic...
   convert plugin_init into <module>_LTX_plugin_init			*/

#if defined (PLUGIN_NAME)
#define JOIN(x, y) JOIN_AGAIN(x, y)
#define JOIN_AGAIN(x, y) x ## y
#define PLUGIN_INIT	JOIN(PLUGIN_NAME, _LTX_plugin_init)
#define PLUGIN_PROCESS	JOIN(PLUGIN_NAME, _LTX_plugin_process)
#define PLUGIN_END	JOIN(PLUGIN_NAME, _LTX_plugin_end)
#endif
