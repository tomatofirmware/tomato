/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard 
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __GETIFSTATS_H__
#define __GETIFSTATS_H__

struct ifdata {
	unsigned long opackets;
	unsigned long ipackets;
	unsigned long obytes;
	unsigned long ibytes;
	unsigned long baudrate;
};

int
getifstats(const char * ifname, struct ifdata * data);

#endif

