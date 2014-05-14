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


/* $Id: fwapi.h 369 2008-01-19 16:07:14Z hb9xar $ */

/*
 * constants
 */
#define ACT_START_RTP	1	/* action: start RTP stream */
#define ACT_STOP_RTP	2	/* action: stop  RTP stream */

#define DIR_IN		1	/* direction: incoming	*/
#define DIR_OUT		2	/* direction: outgoing	*/


/*
 * structure passed to custom firewall control module
 */
typedef struct {
   int	action;
   int	direction;

   struct in_addr local_ipaddr;
   int		  local_port;

   struct in_addr remote_ipaddr;
   int		  remote_port;

} fw_ctl_t;


/*
 * Functions that must be present in custom firewall control module.
 * Siproxd will link against it.
 */ 
int custom_fw_control(fw_ctl_t fwdata);

