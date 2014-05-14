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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: accessctl.c 424 2009-03-28 09:37:47Z hb9xar $";

/* configuration storage */
struct siproxd_config configuration;


/*
 * verifies the from address agains the access lists
 * defined in the configuration file.
 *
 * returns a bitmask with ACCESSCTL_SIP, ACCESSCTL_REG
 */
int accesslist_check (struct sockaddr_in from) {
   int access = 0;

   DEBUGC(DBCLASS_ACCESS,"deny  list (SIP):%s",
      configuration.hosts_deny_sip? configuration.hosts_deny_sip : "*NULL*");
   DEBUGC(DBCLASS_ACCESS,"allow list (SIP):%s",
      configuration.hosts_allow_sip? configuration.hosts_allow_sip : "*NULL*");
   DEBUGC(DBCLASS_ACCESS,"allow list (REG):%s",
      configuration.hosts_allow_reg? configuration.hosts_allow_reg : "*NULL*");

/*
 * check DENY list
 */
   if ( (configuration.hosts_deny_sip !=NULL) &&
        (strcmp(configuration.hosts_deny_sip,"")!=0) ) {
      /* non-empty list -> check agains it */
      if (process_aclist(configuration.hosts_deny_sip, from)== STS_SUCCESS) {
         /* yup - this one is blacklisted */
         DEBUGC(DBCLASS_ACCESS,"caught by deny list");
         return 0;
      }
   }

/*
 * check SIP allow list
 */
   if ( (configuration.hosts_allow_sip !=NULL) &&
        (strcmp(configuration.hosts_allow_sip,"")!=0) ) {
      /* non-empty list -> check agains it */
      if (process_aclist(configuration.hosts_allow_sip, from)==STS_SUCCESS) {
         /* SIP access granted */
         DEBUGC(DBCLASS_ACCESS,"granted SIP access");
         access |= ACCESSCTL_SIP;
      }
   } else {
      access |= ACCESSCTL_SIP;
   }

/*
 * check SIP registration allow list
 */
   if ( (configuration.hosts_allow_reg !=NULL) &&
        (strcmp(configuration.hosts_allow_reg,"")!=0) ) {
      /* non-empty list -> check against it */
      if (process_aclist(configuration.hosts_allow_reg, from)==STS_SUCCESS) {
         /* SIP registration access granted */
         DEBUGC(DBCLASS_ACCESS,"granted REG/SIP access");
         access |= ACCESSCTL_REG | ACCESSCTL_SIP;
      }
   } else {
      access |= ACCESSCTL_REG;
   }

   DEBUGC(DBCLASS_ACCESS,"access check =%i", access);
   return access;
}


/*
 * checks for a match of the 'from' address with the supplied
 * access list.
 *
 * RETURNS
 *	STS_SUCCESS for a match
 *	STS_FAILURE for no match
 */
int process_aclist (char *aclist, struct sockaddr_in from) {
   int i, sts;
   int lastentry;
   char *p1, *p2;
   char address[32]; /* dotted decimal IP - max 15 chars*/
   char mask[8];     /* mask - max 2 digits */
   int  mask_int;
   struct in_addr inaddr;
   unsigned int bitmask;


   for (i=0, p1=aclist, lastentry=0;
        !lastentry; i++) {

/*
 * extract one entry from the access list
 */
      /* address */
      p2=strchr(p1,'/');
      if (!p2) {
         ERROR("CONFIG: accesslist [%s]- no mask separator found", aclist);
	 return STS_FAILURE;
      }
      memset(address,0,sizeof(address));
      memcpy(address,p1,p2-p1);

      /* mask */
      p1=p2+1;
      p2=strchr(p1,',');
      if (!p2) { /* then this must be the last entry in the list */
         p2=strchr(p1,'\0');
	 lastentry=1;
      }
      memset(mask,0,sizeof(mask));
      memcpy(mask,p1,p2-p1);
      p1=p2+1;

      DEBUGC(DBCLASS_ACCESS,"[%i] extracted address=%s", i, address);
      DEBUGC(DBCLASS_ACCESS,"[%i] extracted mask   =%s", i, mask);

/*
 * check for a match
 */
      sts=get_ip_by_host(address, &inaddr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_ACCESS, "process_aclist: cannot resolve address [%s]",
                address);
         return STS_FAILURE;
      }

      mask_int=atoi(mask);
      if ((mask_int < 0) || (mask_int > 32)) mask_int=32;
      bitmask= (mask_int)? (0xffffffff<<(32-mask_int)) : 0;

      DEBUGC(DBCLASS_ACCESS,"check match: entry=%i, filter=%lx, from=%lx", i,
                            (long)ntohl(inaddr.s_addr) & bitmask,
			    (long)ntohl(from.sin_addr.s_addr) & bitmask);

      if ( (ntohl(inaddr.s_addr) & bitmask) == 
           (ntohl(from.sin_addr.s_addr) & bitmask) ) return STS_SUCCESS;
   }

   return STS_FAILURE;
}
