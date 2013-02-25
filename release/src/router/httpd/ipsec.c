/*

	Tomato Firmware
	IPSec Server Support
	Copyright (C) 2013 Daniel Borca

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "tomato.h"
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef IPSEC_CONNECTED
#define IPSEC_CONNECTED	"/tmp/ipsec_connected"
#endif

#ifndef IF_SIZE
#define IF_SIZE 8
#endif

void asp_ipsec_altnames(int argc, char **argv)
{
	BIO *in;
	X509 *cert;
	int i, n;

	STACK_OF(GENERAL_NAME) *gens;

	int comma = ' ';
	char *buf = nvram_safe_get("ipsec_crt");
	const int len = strlen(buf);
	web_puts("\n\nipsec_altnames=[");

	in = BIO_new_mem_buf(buf, len);
	if (in == NULL) {
		goto done;
	}

	cert = PEM_read_bio_X509(in, NULL, NULL, NULL);
	if (cert == NULL) {
		goto err1;
	}

	gens = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
	if (gens == NULL) {
		goto err2;
	}

	n = sk_GENERAL_NAME_num(gens);
	for (i = 0; i < n; i++) {
		GENERAL_NAME *name = sk_GENERAL_NAME_value(gens, i);
		if (name) {
			switch (name->type) {
			case GEN_DNS: {
				unsigned char *dns = NULL;
				ASN1_STRING_to_UTF8(&dns, name->d.dNSName);
				if (dns) {
					web_printf("%c['%s']", comma, dns);
					comma = ',';
					OPENSSL_free(dns);
				}
				break;
			}
			case GEN_IPADD: {
				break;
			}
			default:
				break;
			}
			GENERAL_NAME_free(name);
		}
	}

	sk_GENERAL_NAME_free(gens);
    err2:
	X509_free(cert);
    err1:
	BIO_free(in);
    done:
	web_puts(" ];\n");
}

void asp_ipsec_userol(int argc, char **argv) {
	char comma;
	char line[256];
	FILE *fp;

	char clientusername[256+1];
	char clientlocalip[INET6_ADDRSTRLEN+1];
	char clientremoteip[INET6_ADDRSTRLEN+1];
	char interface[IF_SIZE+1];
	int pid = -1;
	int clientuptime;

	web_puts("\n\nipsec_online=[");
	comma = ' ';

	fp = fopen("/var/run/racoon.pid", "r");
	if (fp) {
		if (fgets(line, sizeof(line), fp) != NULL) {
			sscanf(line, "%d", &pid);
		}
		fclose(fp);
	}

	fp = fopen(IPSEC_CONNECTED, "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (sscanf(line, "%s %s %s %s %d", interface, clientlocalip, clientremoteip, clientusername, &clientuptime) != 5) continue;
			web_printf("%c['%d', '%s', '%s', '%s', '%s', '%d']", 
				comma, pid, interface, clientlocalip, clientremoteip, clientusername, clientuptime);
			comma = ',';
		}
		fclose(fp);
	}

	web_puts("];\n");
}

void wo_ipseccmd(char *url) {
#if 0
	char *p;
	int n = 10;
#endif
	// do we really need to output anything?
	web_puts("\nipsec_result = [\n");
#if 0
	if ((p = webcgi_get("disconnect")) != NULL) {
		while ((kill(atoi(p), SIGTERM) == 0) && (n > 1)) {
			sleep(1);
			n--;
		}
	}
#endif
	web_puts("];\n");
}
