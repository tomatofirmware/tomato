/*
 * xxtp.c
 *
 * Copyright (C) 2013 Daniel Borca <dborca@yahoo.com>
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <rc.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>

void get_broadcast(char *ipaddr, char *netmask)
{
        int ip2[4], mask2[4];
        unsigned char ip[4], mask[4];

        if (!ipaddr || !netmask)
                return;

        sscanf(ipaddr, "%d.%d.%d.%d", &ip2[0], &ip2[1], &ip2[2], &ip2[3]);
        sscanf(netmask, "%d.%d.%d.%d", &mask2[0], &mask2[1], &mask2[2],
               &mask2[3]);
        int i = 0;

        for (i = 0; i < 4; i++) {
                ip[i] = ip2[i];
                mask[i] = mask2[i];
                ip[i] = (ip[i] & mask[i]) | (0xff & ~mask[i]);
        }

        sprintf(ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

        //fprintf(stderr, "get_broadcast return %s\n", value);
}

void write_chap_secret(const char *users, const char *server, char *file)
{
        FILE *fp;
        char *nv, *nvp, *b;
        char *username, *passwd;
//        char buf[64];

        fp=fopen(file, "w");

        if (fp==NULL) return;

//        nv = nvp = strdup(nvram_safe_get("pptpd_clientlist"));
        nv = nvp = strdup(nvram_safe_get(users));

        if(nv) {
            	while ((b = strsep(&nvp, ">")) != NULL) {
                	if((vstrsep(b, "<", &username, &passwd)!=2)) continue;
	                if(strlen(username)==0||strlen(passwd)==0) continue;
        	        fprintf(fp, "%s %s %s *\n", username, server, passwd);
            	}
            	free(nv);
        }
        fclose(fp);
}

void write_xxtpd_dnsmasq_config(FILE* f) {
	int i;
	if (nvram_match("pptpd_enable", "1") || nvram_match("l2tpd_enable", "1")) {
		fprintf(f, "interface=");
		for (i = 4; i <= 9 ; i++) {
			fprintf(f, "ppp%d%c", i, ((i < 9)? ',' : '\n'));
		}
		fprintf(f, "no-dhcp-interface=");
		for (i = 4; i <= 9 ; i++) {
			fprintf(f, "ppp%d%c", i, ((i < 9)? ',' : '\n'));
		}
	}
}
