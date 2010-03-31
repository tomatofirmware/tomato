/* $Id: natpmp.c,v 1.13 2008/10/17 16:44:37 nanard Exp $ */
/* MiniUPnP project
 * (c) 2007 Thomas Bernard
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "natpmp.h"
#include "upnpglobalvars.h"
#include "getifaddr.h"
#include "upnpredirect.h"
#include "commonrdr.h"

#ifdef ENABLE_NATPMP

int OpenAndConfNATPMPSocket()
{
	int snatpmp;
	snatpmp = socket(PF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/);
	if(snatpmp<0)
	{
		syslog(LOG_ERR, "socket(natpmp): %m");
		return -1;
	}
	else
	{
		struct sockaddr_in natpmp_addr;
		memset(&natpmp_addr, 0, sizeof(natpmp_addr));
		natpmp_addr.sin_family = AF_INET;
		natpmp_addr.sin_port = htons(NATPMP_PORT);
		natpmp_addr.sin_addr.s_addr = INADDR_ANY;
		//natpmp_addr.sin_addr.s_addr = inet_addr("192.168.0.1");
		if(bind(snatpmp, (struct sockaddr *)&natpmp_addr, sizeof(natpmp_addr)) < 0)
		{
			syslog(LOG_ERR, "bind(natpmp): %m");
			close(snatpmp);
			return -1;
		}
	}
	return snatpmp;
}

void ProcessIncomingNATPMPPacket(int s)
{
	unsigned char req[32];	/* request udp packet */
	unsigned char resp[32];	/* response udp packet */
	int resplen;
	struct sockaddr_in senderaddr;
	socklen_t senderaddrlen = sizeof(senderaddr);
	int n;
	char senderaddrstr[16];
#ifndef MULTIPLE_EXTERNAL_IP
	char tmp[16];
#else
	int i;
#endif
	n = recvfrom(s, req, sizeof(req), 0,
	             (struct sockaddr *)&senderaddr, &senderaddrlen);
	if(n<0) {
		syslog(LOG_ERR, "recvfrom(natpmp): %m");
		return;
	}
	if(!inet_ntop(AF_INET, &senderaddr.sin_addr,
	              senderaddrstr, sizeof(senderaddrstr))) {
		syslog(LOG_ERR, "inet_ntop(natpmp): %m");
	}
	syslog(LOG_INFO, "NAT-PMP request received from %s:%hu %dbytes",
           senderaddrstr, ntohs(senderaddr.sin_port), n);
	if(n<2 || ((((req[1]-1)&~1)==0) && n<12)) {
		syslog(LOG_WARNING, "discarding NAT-PMP request (too short) %dBytes",
		       n);
		return;
	}
	if(req[1] & 128) {
		/* discarding NAT-PMP responses silently */
		return;
	}
	memset(resp, 0, sizeof(resp));
	resplen = 8;
	resp[1] = 128 + req[1];	/* response OPCODE is request OPCODE + 128 */
	/* setting response TIME STAMP */
	*((uint32_t *)(resp+4)) = htonl(time(NULL) - startup_time);
	if(req[0] > 0) {
		/* invalid version */
		syslog(LOG_WARNING, "unsupported NAT-PMP version : %u",
		       (unsigned)req[0]);
		resp[3] = 1;	/* unsupported version */
	} else switch(req[1]) {
	case 0:	/* Public address request */
		syslog(LOG_INFO, "NAT-PMP public address request");
#ifndef MULTIPLE_EXTERNAL_IP
		if(use_ext_ip_addr) {
               inet_pton(AF_INET, use_ext_ip_addr, resp+8);
		} else {
			if(!ext_if_name || ext_if_name[0]=='\0') {
				resp[3] = 3;	/* Network Failure (e.g. NAT box itself
				                 * has not obtained a DHCP lease) */
			} else if(getifaddr(ext_if_name, tmp, INET_ADDRSTRLEN) < 0) {
				syslog(LOG_ERR, "Failed to get IP for interface %s", ext_if_name);
				resp[3] = 3;	/* Network Failure (e.g. NAT box itself
				                 * has not obtained a DHCP lease) */
			} else {
				inet_pton(AF_INET, tmp, resp+8);
			}
		}
#else
		for(i = 0; i<n_lan_addr; i++) {
			if( (senderaddr.sin_addr.s_addr & lan_addr[i].mask.s_addr)
			   == (lan_addr[i].addr.s_addr & lan_addr[i].mask.s_addr)) {
				memcpy(resp+8, &lan_addr[i].ext_ip_addr,
				       sizeof(lan_addr[i].ext_ip_addr));
				break;
			}
		}
#endif
		resplen = 12;
		break;
	case 1:	/* UDP port mapping request */
	case 2:	/* TCP port mapping request */
		{
			unsigned short iport;	/* private port */
			unsigned short eport;	/* public port */
			uint32_t lifetime; 		/* lifetime=0 => remove port mapping */
			int r;
			int proto;
			char iaddr_old[16];
			unsigned short iport_old;
			iport = ntohs(*((uint16_t *)(req+4)));
			eport = ntohs(*((uint16_t *)(req+6)));
			lifetime = ntohl(*((uint32_t *)(req+8)));
			proto = (req[1]==1)?IPPROTO_UDP:IPPROTO_TCP;
			syslog(LOG_INFO, "NAT-PMP port mapping request : "
			                 "%hu->%s:%hu %s lifetime=%us",
			                 eport, senderaddrstr, iport,
			                 (req[1]==1)?"udp":"tcp", lifetime);
			if(eport==0)
				eport = iport;
			/* TODO: accept port mapping if iport ok but eport not ok
			 * (and set eport correctly) */
			if(lifetime == 0) {
				/* remove the mapping */
				if(iport == 0) {
					/* remove all the mappings for this client */
					int index = 0;
					unsigned short eport2, iport2;
					char iaddr2[16];
					int proto2;
					char desc[64];
					while(get_redirect_rule_by_index(index, 0,
					          &eport2, iaddr2, sizeof(iaddr2),
							  &iport2, &proto2,
							  desc, sizeof(desc), 0, 0) >= 0) {
						syslog(LOG_DEBUG, "%d %d %hu->'%s':%hu '%s'",
						       index, proto2, eport2, iaddr2, iport2, desc);
						if(0 == strcmp(iaddr2, senderaddrstr)
						  && 0 == memcmp(desc, "NAT-PMP ", 8)) {
							r = _upnp_delete_redir(eport2, proto2);
							/* TODO : check return value */
							if(r<0) {
								syslog(LOG_ERR, "failed to remove port mapping");
								index++;
							} else {
								syslog(LOG_INFO, "NAT-PMP %s port %hu mapping removed", proto2==IPPROTO_TCP?"TCP":"UDP", eport2);
							}
						} else {
							index++;
						}
					}
				} else {
					/* To improve the interworking between nat-pmp and
					 * UPnP, we should check that we remove only NAT-PMP 
					 * mappings */
					//!!TB - begin
					// only attempt to delete rule if it exists
					r = get_redirect_rule(ext_if_name, eport, proto,
						iaddr_old, sizeof(iaddr_old),
						&iport_old, 0, 0, 0, 0);
					if (r >= 0) {
					//!!TB - end
						r = _upnp_delete_redir(eport, proto);
						/*syslog(LOG_DEBUG, "%hu %d r=%d", eport, proto, r);*/
						if(r<0) {
							syslog(LOG_ERR, "Failed to remove NAT-PMP mapping eport %hu, protocol %s", eport, (proto==IPPROTO_TCP)?"TCP":"UDP");
							resp[3] = 2;	/* Not Authorized/Refused */
						}
					}	//!!TB
				}
				eport = 0; /* to indicate correct removing of port mapping */
			} else if(iport==0
			   || !check_upnp_rule_against_permissions(upnppermlist, num_upnpperm, eport, senderaddr.sin_addr, iport)) {
				resp[3] = 2;	/* Not Authorized/Refused */
			} else do {
				r = get_redirect_rule(ext_if_name, eport, proto,
				                      iaddr_old, sizeof(iaddr_old),
				                      &iport_old, 0, 0, 0, 0);
				if(r==0) {
					if(strcmp(senderaddrstr, iaddr_old)==0
				       && iport==iport_old) {
						/* redirection allready existing */
						syslog(LOG_INFO, "port %hu %s already redirected to %s:%hu, replacing", eport, (proto==IPPROTO_TCP)?"tcp":"udp", iaddr_old, iport_old);
						/* remove and then add again */
						if(_upnp_delete_redir(eport, proto) < 0) {
							syslog(LOG_ERR, "failed to remove port mapping");
							break;
						}
					} else {
						eport++;
						continue;
					}
				}
				{ /* do the redirection */
					char desc[64];
					unsigned timestamp = (unsigned)(time(NULL) - startup_time)
					                      + lifetime;
					snprintf(desc, sizeof(desc), "NAT-PMP %u", timestamp);
					/* TODO : check return code */
					if(upnp_redirect_internal(eport, senderaddrstr,
					                          iport, proto, desc) < 0) {
						syslog(LOG_ERR, "Failed to add NAT-PMP %hu %s->%s:%hu '%s'",
						       eport, (proto==IPPROTO_TCP)?"tcp":"udp", senderaddrstr, iport, desc);
						resp[3] = 3;  /* Failure */
					} else if( !nextnatpmptoclean_eport
					         || timestamp < nextnatpmptoclean_timestamp) {
						nextnatpmptoclean_timestamp = timestamp;
						nextnatpmptoclean_eport = eport;
						nextnatpmptoclean_proto = proto;
					}
					break;
				}
			} while(r==0);
			*((uint16_t *)(resp+8)) = htons(iport);	/* private port */
			*((uint16_t *)(resp+10)) = htons(eport);	/* public port */
			*((uint32_t *)(resp+12)) = htonl(lifetime);
		}
		resplen = 16;
		break;
	default:
		resp[3] = 5;	/* Unsupported OPCODE */
	}
	n = sendto(s, resp, resplen, 0,
	           (struct sockaddr *)&senderaddr, sizeof(senderaddr));
	if(n<0) {
		syslog(LOG_ERR, "sendto(natpmp): %m");
	} else if(n<resplen) {
		syslog(LOG_ERR, "sendto(natpmp): sent only %d bytes out of %d",
		       n, resplen);
	}
}

int ScanNATPMPforExpiration()
{
	char desc[64];
	unsigned short iport, eport;
	int proto;
	int r, i;
	unsigned timestamp;
	nextnatpmptoclean_eport = 0;
	nextnatpmptoclean_timestamp = 0;
	for(i = 0; ; i++) {
		r = get_redirect_rule_by_index(i, 0, &eport, 0, 0,
		                               &iport, &proto, desc, sizeof(desc),
		                               0, 0);
		if(r<0)
			break;
		if(sscanf(desc, "NAT-PMP %u", &timestamp) == 1) {
			if( !nextnatpmptoclean_eport
			  || timestamp < nextnatpmptoclean_timestamp) {
				nextnatpmptoclean_eport = eport;
				nextnatpmptoclean_proto = proto;
				nextnatpmptoclean_timestamp = timestamp;
				syslog(LOG_DEBUG, "set nextnatpmptoclean_timestamp to %u", timestamp);
			}
		}
	}
	return 0;
}

int CleanExpiredNATPMP()
{
	char desc[64];
	unsigned short iport;
	if(get_redirect_rule(ext_if_name, nextnatpmptoclean_eport,
	                     nextnatpmptoclean_proto,
	                     0, 0,
	                     &iport, desc, sizeof(desc), 0, 0) < 0)
		return ScanNATPMPforExpiration();
	/* TODO: check desc */
	/* remove redirection then search for next one:) */
	if(_upnp_delete_redir(nextnatpmptoclean_eport, nextnatpmptoclean_proto)<0)
		return -1;
	syslog(LOG_INFO, "Expired NAT-PMP mapping port %hu %s removed",
	       nextnatpmptoclean_eport,
	       nextnatpmptoclean_proto==IPPROTO_TCP?"TCP":"UDP");
	return ScanNATPMPforExpiration();
}

#endif

