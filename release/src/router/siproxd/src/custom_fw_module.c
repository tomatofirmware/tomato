/*
    Copyright (C) 2004-2008  Thomas Ries <tries@gmx.net>

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
 * This is just an example of how to build your custom
 * interface between siproxd and your firewall.
 *
 * Take this as a starting point for your own code.
 *
 * To build siproxd with you own firewall control module:
 * 1) compile your interface module (e.g. this example code) 
 *    and make an static library out of it.
 * 2) configure siproxd with:
 *    ./configure --with-custom-fwmodule=<path>/<library>.a
 *    (for example: --with-custom-fwmodule=`pwd`/src/libcustom_fw_module.a)
 *
 *
 * The START_RTP action will be called BEFORE the RTP stream 
 * actually is started. The STOP_RTP action will be called after
 * the RTP stream has been stopped.
 * START_RTP will only be called once for an starting RTP stream,
 * in case of repetitions (SIP INVITE sequence) it will not
 * be called multiple times.
 *
 * The code here is called synchroneously, means the time you spend
 * in here doing things, siproxd will not do anything else, so
 * try to do thins as fast as possible and don't wait for something
 * to happen.
 *
 */

#include <stdio.h>		/* sprintf */
#include <string.h>		/* strcat  */


#include <sys/types.h>
#include <netinet/in.h>
#include "fwapi.h"
#include "log.h"

static char const ident[]="$Id: custom_fw_module.c 369 2008-01-19 16:07:14Z hb9xar $";

/*
 * some prototypes of util.c - so I don't have to suck in the
 * whole bunch of include files. You probably will not use this
 * in your code anyway - or then should make it in a proper way.
 */
char *utils_inet_ntoa(struct in_addr in);

/*
 * Should return with 0 on success.
 * If return status is != 0, siproxd will complain with an
 * an ERROR() but continue.
 */
int custom_fw_control(fw_ctl_t fwdata) {
   static char tmp[256];

   tmp[0]='\0';
   switch (fwdata.action) {
     case ACT_START_RTP:
       strcat(tmp, "ACT_START_RTP: ");
       break;
     case ACT_STOP_RTP:
       strcat(tmp, "ACT_STOP_RTP: ");
       break;
     default:
       strcat(tmp, "ACT_unknown: ");
       break;
   }

   switch (fwdata.direction) {
     case DIR_IN:
       strcat(tmp, "DIR_IN ");
       break;
     case DIR_OUT:
       strcat(tmp, "DIR_OUT ");
       break;
     default:
       strcat(tmp, "DIR_unknown ");
       break;
   }

   sprintf(&tmp[strlen(tmp)],"[lcl %s:%i] ",
           utils_inet_ntoa(fwdata.local_ipaddr),
           fwdata.local_port);

   sprintf(&tmp[strlen(tmp)],"[rem %s:%i] ",
           utils_inet_ntoa(fwdata.remote_ipaddr),
           fwdata.remote_port);

   INFO("CUSTOM: %s", tmp);

   return 0;
}
