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

#include <sys/types.h>
#include <netinet/in.h>
#include <osipparser2/osip_parser.h>
#include "siproxd.h"
#include "fwapi.h"
#include "log.h"

static char const ident[]="$Id: fwapi.c 369 2008-01-19 16:07:14Z hb9xar $";

int fwapi_start_rtp(int rtp_direction,
                    struct in_addr local_ipaddr, int local_port,
                    struct in_addr remote_ipaddr, int remote_port) {
#ifdef CUSTOM_FWMODULE
   int sts;
   fw_ctl_t fwdata;
   fwdata.action    = ACT_START_RTP;
   fwdata.direction = (rtp_direction == DIR_INCOMING)? DIR_IN: DIR_OUT;
   memcpy(&fwdata.local_ipaddr, &local_ipaddr, sizeof(fwdata.local_ipaddr));
   fwdata.local_port = local_port;
   memcpy(&fwdata.remote_ipaddr, &remote_ipaddr, sizeof(fwdata.remote_ipaddr));
   fwdata.remote_port = remote_port;

   sts=custom_fw_control(fwdata);
   if (sts != STS_SUCCESS) {
      ERROR("Custom firewall module returned error [START, sts=%i]",sts);
   }
#endif
   return STS_SUCCESS;
}

int fwapi_stop_rtp(int rtp_direction,
                   struct in_addr local_ipaddr, int local_port,
                   struct in_addr remote_ipaddr, int remote_port) {
#ifdef CUSTOM_FWMODULE
   int sts;
   fw_ctl_t fwdata;
   fwdata.action    = ACT_STOP_RTP;
   fwdata.direction = (rtp_direction == DIR_INCOMING)? DIR_IN: DIR_OUT;
   memcpy(&fwdata.local_ipaddr, &local_ipaddr, sizeof(fwdata.local_ipaddr));
   fwdata.local_port = local_port;
   memcpy(&fwdata.remote_ipaddr, &remote_ipaddr, sizeof(fwdata.remote_ipaddr));
   fwdata.remote_port = remote_port;

   sts=custom_fw_control(fwdata);
   if (sts != STS_SUCCESS) {
      ERROR("Custom firewall module returned error [STOP, sts=%i]",sts);
   }
#endif
   return STS_SUCCESS;
}
