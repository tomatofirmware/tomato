/*
    Copyright (C) 2009  Thomas Ries <tries@gmx.net>

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
#define PLUGIN_NAME	plugin_stun

#include "config.h"

#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_md5.h>

#include "siproxd.h"
#include "digcalc.h"
#include "plugins.h"
#include "log.h"

static char const ident[]="$Id: plugin_stun.c 479 2011-06-11 21:51:40Z hb9xar $";

/* Plug-in identification */
static char name[]="plugin_stun";
static char desc[]="Use an external STUN server to determine my public IP";

/* global configuration storage - required for config file location */
extern struct siproxd_config configuration;

/* plugin configuration storage */
static struct plugin_config {
   char *server;
   int  port;
   int period;
} plugin_cfg;

/* Instructions for config parser */
static cfgopts_t plugin_cfg_opts[] = {
   { "plugin_stun_server",      TYP_STRING, &plugin_cfg.server,	{0, NULL} },
   { "plugin_stun_port",        TYP_INT4, &plugin_cfg.port,	{3478, NULL} },
   { "plugin_stun_period",      TYP_INT4, &plugin_cfg.period,	{300, NULL} },
   {0, 0, 0}
};

/*
 * module-local function prototypes
 */
static int stun_validate_response(char *buffer, int len, char *tid);
static int stun_send_request(char *tid);
static int stun_new_transaction_id(char *tid);

/*
 * constants used in this module
 */
#define STUN_TID_SIZE	16	/* STUN transaction ID size: 16 bytes */


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
   plugin_def->exe_mask=PLUGIN_PROCESS_RAW|PLUGIN_TIMER;

   /* read the config file */
   if (read_config(configuration.configfile,
                   configuration.config_search,
                   plugin_cfg_opts, name) == STS_FAILURE) {
      ERROR("Plugin '%s': could not load config file", name);
      return STS_FAILURE;
   }

/*&&& resolve STUN server
store IP
*/

   INFO("plugin_stun is initialized, using %s:%i as STUN server",
        plugin_cfg.server, plugin_cfg.port);
   return STS_SUCCESS;
}

/*
 * Processing.
 * 
 */
int  PLUGIN_PROCESS(int stage, sip_ticket_t *ticket){
   static time_t next_stun_send=0;
   static int rq_pending=0; /* !=0 if waiting for response (ongoing dialog) */
   
   static union {
      char stun_transaction_id[STUN_TID_SIZE];
      uint16_t  port;
   } u;

   time_t now;
   int sts;

   /* stage contains the PLUGIN_* value - the stage of SIP processing. */
   DEBUGC(DBCLASS_BABBLE, "called in stage %i, rq_pending=%i",
          stage, rq_pending);

   time(&now);

   switch (stage) {
      case PLUGIN_TIMER:
         if (next_stun_send <= now) {
            /* compose and send new STUN request */
            DEBUGC(DBCLASS_BABBLE, "preparing to send STUN request");

            /* allocate transaction ID if a new STUN transaction */
            if (rq_pending == 0) {
               sts = stun_new_transaction_id(u.stun_transaction_id);
            }

            /* send STUN BIND request */
            sts = stun_send_request(u.stun_transaction_id);

            rq_pending=1;
            /* 10 seconds T/O to receive answer until retrying */
            next_stun_send = now + 10;
         }
         break;

      case PLUGIN_PROCESS_RAW:
         /* raw UDP packet: ticket->raw_buffer, len: ticket->raw_buffer_len */
         sts = stun_validate_response(ticket->raw_buffer, 
                                      ticket->raw_buffer_len,
                                      u.stun_transaction_id);
         if (sts == STS_SUCCESS) {
            /* is a valid response to our last STUN request */
            /* This is about what we get:
            +-----------------+-----------------+
            |........¦..type..|........¦...len..|   [0]..[4]
            +-----------------+-----------------+
            |      Transaction ID               |   [5]..
            |                                   |
            |                                   |
            |                                   |      ..[19]
            +-----------------+-----------------+
            |........¦.type...|........¦...len..|  [20]..     Attribute
            +-----------------+-----------------+
            |  len bytes                        |
            |                                   |
            +-----------------+-----------------+
            |........¦.type...|........¦...len..|            Attribute
            +-----------------+-----------------+
            |                                   |
            ...
            */
{ /* clean up & move to function! */
            int i, j;
            int len=ticket->raw_buffer_len;
            char *buffer=ticket->raw_buffer;
            int iptype;
            uint16_t port;
            unsigned char ip[4];
            char ipstring[IPSTRING_SIZE];
            int got_address=0;

            i=20;		// 1st attribute position

            while (i+4 <= len) {
               int attr_typ = ntohs(*((int *)&buffer[i]) & 0x0000ffff);
               int attr_len = ntohs(*((int *)&buffer[i+2]) & 0x0000ffff);

               DEBUGC(DBCLASS_BABBLE,"STUN response: i=%i, type=%i, len=%i",
                      i, attr_typ, attr_len);

               /* check if attribute is fully contained in message */
               if ((i+4+attr_len) > len) {
                  DEBUGC(DBCLASS_BABBLE,"corrupt STUN response");
                  break;
               }

               /* advance to start of attribute */
               i = i+4;

               switch (attr_typ) {
                  /* 0x0001 Mapped Address */
                  case 0x0001:
                     DEBUGC(DBCLASS_BABBLE,"Mapped Addr, len=%i", attr_len);
                     iptype=*((int*)&buffer[i+1]) & 0x000000ff;
                     if (iptype != 0x0001) {
                        DEBUGC(DBCLASS_BABBLE,
                               "Mapped Addr, wrong proto family [%i]", iptype);
                        break;
                     }

                     port=htons(*((int*)&buffer[i+2]) & 0x0000ffff);
                     memcpy(ip,&buffer[i+4], 4);

                     DEBUGC(DBCLASS_BABBLE,"STUN: public IP %u.%u.%u.%u:%i",
                            ip[0], ip[1], ip[2], ip[3], port);
                     /* remember normal IP address only if not yet known */
                     if (got_address == 0) {
                        snprintf(ipstring, IPSTRING_SIZE-1, "%u.%u.%u.%u",
                                 ip[0], ip[1], ip[2], ip[3]);
                        ipstring[IPSTRING_SIZE-1]='\0';
                        got_address=1;
                     }
                     break;

                  /* 0x8020 XOR Mapped Address */
                  case 0x8020:
                     DEBUGC(DBCLASS_BABBLE,"XOR Mapped Addr, len=%i", attr_len);
                     iptype=*((int*)&buffer[i+1]) & 0x000000ff;

                     if (iptype != 0x0001) {
                        DEBUGC(DBCLASS_BABBLE,
                               "Mapped Addr, wrong proto family [%i]", iptype);
                        break;
                     }

                     port=*((int*)&buffer[i+2]) & 0x0000ffff;
                     /* XOR the port with start of TID */
//                     port = port ^ *((short int*)stun_transaction_id);
                     port = port ^ u.port;
                     port = htons(port);
                     memcpy(ip,&buffer[i+4], 4);
                     /* XOR the IP with start of TID */
                     for (j=0; j<4; j++) { 
                        ip[j]=ip[j] ^ u.stun_transaction_id[j];
                     }
                     DEBUGC(DBCLASS_BABBLE,"STUN: public IP %u.%u.%u.%u:%i",
                            ip[0], ip[1], ip[2], ip[3], port);

                     /* remember XORed IP address always (preferred) */
                     snprintf(ipstring, IPSTRING_SIZE-1, "%u.%u.%u.%u",
                              ip[0], ip[1], ip[2], ip[3]);
                     ipstring[IPSTRING_SIZE-1]='\0';
                     got_address=1;
                     break;

                  default:
                     /* don't care... */
                     break;
               }
               /* advance to next attribute */
               i=i+attr_len;
            }

            if (got_address && (
                (configuration.outbound_host == NULL) ||
                (strcmp(configuration.outbound_host, ipstring) != 0) ) ) {

               INFO("STUN: public IP has changed %s -> %s",
                    (configuration.outbound_host) ?
                    configuration.outbound_host:"NULL" ,
                    ipstring);

               if (configuration.outbound_host) {
                  free(configuration.outbound_host);
               }
               configuration.outbound_host=malloc(IPSTRING_SIZE);

               strcpy(configuration.outbound_host, ipstring);
            }

}

/*&&&
if XOR address and normal address do not match I may have a
serious problem (IP provider not passing public IPs to clients).
Some linksys routers seem to alter the "normal" IP to be
the WAN IP, that may make such a situation visible.

How can we deal with such a thing? We cannot change the NAT on
the provider side, can we? This will be WAY out os scope of siproxds
capabilities.

I should issue an WARNING on such a detected situation.
*/


            rq_pending=0; /* received answer, good */
            next_stun_send = now + plugin_cfg.period;

            DEBUGC(DBCLASS_BABBLE,"next STUN request in %i sec at %i",
                   plugin_cfg.period, (int)next_stun_send);

            return STS_FAILURE; /* done, do not further process as SIP */
         }
         break;

      default:
         break;
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


/*
 * module-local functions
 */
static int stun_validate_response(char *buffer, int len, char *tid){

   /* expect min. STUN header + 1 attribute */
   if (len < 24) {
      DEBUGC(DBCLASS_BABBLE,"stun_validate_response: no STUN response (too short)");
      return STS_FAILURE;
   }

   if (ntohs(*((int*)&buffer[0]) & 0x0000ffff) != 0x0101) {
      DEBUGC(DBCLASS_BABBLE,"stun_validate_response: no STUN response (type)");
      return STS_FAILURE;
   }

   if (memcmp(&buffer[4], tid, STUN_TID_SIZE) != 0) {
      DEBUGC(DBCLASS_BABBLE,"stun_validate_response: wrong STUN response (TID)");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_BABBLE,"valid STUN response");
   return STS_SUCCESS;
}

static int stun_send_request(char *tid){
   struct in_addr addr;
   int sts;
   char stun_rq[28];	/*&&& testing */
   size_t size=28;

   /* name resolution */
   if (utils_inet_aton(plugin_cfg.server, &addr) == 0)
   {
      /* need name resolution */
      DEBUGC(DBCLASS_DNS,"resolving name:%s", plugin_cfg.server);
      sts = get_ip_by_host(plugin_cfg.server, &addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_DNS, "stun_send_request: cannot resolve STUN server [%s]",
                plugin_cfg.server);
         return STS_FAILURE;
      }
   }   

   /* compose STUN BIND request - poor mans way */
   stun_rq[0]=0x00;	// type
   stun_rq[1]=0x01;
   stun_rq[2]=0x00;	// length
   stun_rq[3]=0x08;
   memcpy (&stun_rq[4], tid, STUN_TID_SIZE);
   stun_rq[20]=0x00;	// ATTR change request
   stun_rq[21]=0x03;
   stun_rq[22]=0x00;	// ATTR len 4
   stun_rq[23]=0x04;
   stun_rq[24]=0x00;	// Change IP not set, Change port not set
   stun_rq[25]=0x00;
   stun_rq[26]=0x00;
   stun_rq[27]=0x00;

   /* and send via the SIP UDP port */
   sts = sipsock_send(addr, plugin_cfg.port, PROTO_UDP, stun_rq, size);

   return STS_SUCCESS;
}

static int stun_new_transaction_id(char *tid) {
   time_t now;
   /* OSIP MD5 hash lenght is 16 bytes (32 hex digits),
    * so this does fit for STUN transaction ID... */
   osip_MD5_CTX Md5Ctx;
   HASH HA1;

   time(&now);

   /* calc MD5 from servername + timestamp */
   osip_MD5Init(&Md5Ctx);
   if (plugin_cfg.server) osip_MD5Update(&Md5Ctx, 
                                         (unsigned char*)plugin_cfg.server,
                                         strlen(plugin_cfg.server));
   osip_MD5Update(&Md5Ctx, (unsigned char*)&now, sizeof(now));
   osip_MD5Final(HA1, &Md5Ctx);

   memcpy(tid, HA1, (STUN_TID_SIZE<HASHLEN)? STUN_TID_SIZE:HASHLEN);

   return STS_SUCCESS;
}
