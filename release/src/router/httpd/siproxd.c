/*

	Tomato Firmware
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Portions Copyright (C) 2011 Ofer Chen, oferchen@gmail.com

	For use with Tomato Firmware only.
	No part of this file may be used without permission.

*/

#include "tomato.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <osipparser2/osip_headers.h>
#define max_url_size 128
static const char siproxdpid[] = "/tmp/var/run/siproxd.pid";
static const char siproxdregistrationfile[] =  "/var/run/siproxd_registration";
static const char siproxdconf[] = "/etc/siproxd.conf";
static const char siproxdplugindir[] = "/usr/lib/siproxd/";

typedef struct {
	int active;
	int expires;
	osip_uri_t *true_url;	/* inbound URL */
	osip_uri_t *masq_url;	/* outbound URL */
	osip_uri_t *reg_url;	/* masq URL */
} urlmap_s;

void asp_siproxd_stat(int argc, char **argv)
{
	FILE *f;
	char buf[max_url_size];
	int i, stat;
	char *c;
	urlmap_s urlmap[max_url_size];
	size_t len;


	if ((nvram_get_int("siproxd_enable"))&&(nvram_get_int("siproxd_logcall"))) {
	memset(urlmap, 0, sizeof(urlmap));
	web_puts("\nsiproxd_registration_data = '");
	if ((f =  fopen(siproxdregistrationfile,"r")) != NULL ) {
		for (i=0; i < max_url_size; i++) {
		c = fgets(buf, sizeof(buf), f);
		stat = sscanf(buf, "***:%i:%i", &urlmap[i].active, &urlmap[i].expires);
			/* check error in file format */
			if (stat == 0) { break; }
			/* process only active entries */
			if (urlmap[i].active) {
					osip_uri_init(&urlmap[i].true_url);
					osip_uri_init(&urlmap[i].masq_url);
					osip_uri_init(&urlmap[i].reg_url);
					#define R(X) {\
					c=fgets(buf, sizeof(buf), f);\
					buf[sizeof(buf)-1]='\0';\
					if (strchr(buf, 10)) *strchr(buf, 10)='\0';\
					if (strchr(buf, 13)) *strchr(buf, 13)='\0';\
					if (strlen(buf) > 0) {\
					len  = strlen(buf);\
					X =(char*)malloc(len+1);\
					stat=sscanf(buf,"%s",X);\
					if (stat == 0) break;\
					} else {\
					X = NULL;\
					}\
					}
					/* read file to buffer */
					R(urlmap[i].true_url->scheme);
					R(urlmap[i].true_url->username);
					R(urlmap[i].true_url->host);
					R(urlmap[i].true_url->port);
					R(urlmap[i].masq_url->scheme);
					R(urlmap[i].masq_url->username);
					R(urlmap[i].masq_url->host);
					R(urlmap[i].masq_url->port);
					R(urlmap[i].reg_url->scheme);
					R(urlmap[i].reg_url->username);
					R(urlmap[i].reg_url->host);
					R(urlmap[i].reg_url->port);
	/* output buffer to web variable */
	web_printf("%s %s %s %s %s %s %s %s %s %s %s %s\\x0a" , urlmap[i].true_url->scheme, urlmap[i].true_url->username, urlmap[i].true_url->host, urlmap[i].true_url->port, \
								urlmap[i].masq_url->scheme, urlmap[i].masq_url->username, urlmap[i].masq_url->host, urlmap[i].masq_url->port, \
								urlmap[i].reg_url->scheme, urlmap[i].reg_url->username, urlmap[i].reg_url->host, urlmap[i].reg_url->port);
			}
		}
	}
	fclose(f);
	web_puts("';\n");
	}
else
	{
	web_puts("\nsiproxd_registration_data = '';\n"); /* return empty buffer if registration file does not exist - perhaps not the best method*/
	return;
	}
}

