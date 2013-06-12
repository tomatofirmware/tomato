/*
 * ipsec.c
 *
 * Copyright (C) 2013 Daniel Borca <dborca@yahoo.com>
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

void start_ipsec(void)
{
	int with_cert;
	char *p, *network4;
	const char *dns4;
	const char *xauth;
	const char *wanface;
	const char *wanaddr;
	const char *lanaddr;
	const char *lanmask;
	FILE *fp;
	int ret;

	if (!nvram_match("ipsec_enable", "1") || nvram_match("l2tpd_enable", "1")) {
		return;
	}

	wanface = nvram_safe_get("wan_iface");
	wanaddr = nvram_safe_get("wan_ipaddr");
	lanaddr = nvram_safe_get("lan_ipaddr");
	lanmask = nvram_safe_get("lan_netmask");

	with_cert = nvram_match("ipsec_usecert", "1");
	xauth = "xauth_psk_server";
	if (with_cert)
		xauth = "xauth_rsa_server";

	network4 = strdup(nvram_safe_get("ipsec_remoteip"));
	p = strchr(network4, '-');
	if (p)
		*p = '\0'; // XXX pool size, blabla

	dns4 = nvram_safe_get("ipsec_dns1");
	if (strlen(dns4) == 0)
		dns4 = lanaddr;

	mkdir("/tmp/ipsec", 0744);

	modprobe("xfrm_user");
	modprobe("af_key");
	modprobe("esp4");
	modprobe("xfrm4_mode_tunnel");

	fp = fopen("/tmp/ipsec/ph1_script", "w");
	fprintf(fp, 
		"#!/bin/sh\n\n"
		"test -n \"$INTERNAL_ADDR4\" || exit\n\n"
		"getid() {\n"
		"    if [ -n \"`echo -n $1 | grep '\\bCN='`\" ]; then\n"
		"        echo -n $1 | grep -o \"\\bCN=[^, ]\\+\" | cut -d '=' -f 2-\n"
		"    else\n"
		"        echo -n $1 | tr -d '<>' | cut -d ' ' -f 1\n"
		"    fi\n"
		"}\n\n"
		"case $1 in\n"
		"    phase1_up)\n"
		"        iptables -I FORWARD -i %s -s $INTERNAL_ADDR4 -j ACCEPT\n"
		"        iptables -t nat -I PREROUTING -i %s -s $INTERNAL_ADDR4 -d %s/%s -j ACCEPT\n"
		"        echo %s $INTERNAL_ADDR4 $REMOTE_ADDR `getid \"$REMOTE_ID\"` `date +%%s` >> /tmp/ipsec_connected\n"
		"        ;;\n"
		"    phase1_down | phase1_dead)\n"
		"        iptables -D FORWARD -i %s -s $INTERNAL_ADDR4 -j ACCEPT\n"
		"        iptables -t nat -D PREROUTING -i %s -s $INTERNAL_ADDR4 -d %s/%s -j ACCEPT\n"
		"        grep -v $INTERNAL_ADDR4 /tmp/ipsec_connected > /tmp/ipsec_connected.new\n"
		"        mv /tmp/ipsec_connected.new /tmp/ipsec_connected\n"
		"        ;;\n"
		"esac\n",
		wanface, wanface, lanaddr, lanmask, wanface,
		wanface, wanface, lanaddr, lanmask);
	fclose(fp);
	chmod("/tmp/ipsec/ph1_script", 0744);

	if (with_cert) {
		fp = fopen("/tmp/ipsec/ca.crt", "w");
		fprintf(fp, "%s", nvram_safe_get("ipsec_ca"));
		fclose(fp);
		chmod("/tmp/ipsec/ca.crt", 0600);

		fp = fopen("/tmp/ipsec/server.crt", "w");
		fprintf(fp, "%s", nvram_safe_get("ipsec_crt"));
		fclose(fp);
		chmod("/tmp/ipsec/server.crt", 0600);

		fp = fopen("/tmp/ipsec/server.key", "w");
		fprintf(fp, "%s", nvram_safe_get("ipsec_key"));
		fclose(fp);
		chmod("/tmp/ipsec/server.key", 0600);
	}

	fp = fopen("/tmp/ipsec/racoon.conf", "w");
	fprintf(fp, "path pre_shared_key \"/tmp/ipsec/psk.txt\";\n");
	if (with_cert)
		fprintf(fp, "path certificate \"/tmp/ipsec\";\n");
	fprintf(fp,
		"listen {\n"
		"	isakmp %s;\n"
		"	isakmp_natt %s;\n"
		"}\n\n",
		wanaddr, wanaddr);
	fprintf(fp, 
		"path script \"/tmp/ipsec\";\n"
		"\n"
		"remote anonymous {\n"
		"        script \"ph1_script\" phase1_up;\n"
		"        script \"ph1_script\" phase1_down;\n"
		"        script \"ph1_script\" phase1_dead;\n"
		"        exchange_mode main;\n");
	if (with_cert) {
		fprintf(fp, 
			"        certificate_type x509 \"server.crt\" \"server.key\";\n"
			"        ca_type x509 \"ca.crt\";\n"
			"        verify_cert on;\n"
			"        my_identifier fqdn \"%s\";\n"
			"        peers_identifier asn1dn;\n",
			nvram_safe_get("ipsec_name"));
	}
	fprintf(fp, 
		"        passive on;\n"
		"        nat_traversal on;\n"
		"        generate_policy on;\n"
		"        proposal_check obey;\n"
		"        proposal {\n"
		"                encryption_algorithm aes 256;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method %s;\n"
		"                dh_group 2;\n"
		"        }\n"
		"        proposal {\n"
		"                encryption_algorithm aes;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method %s;\n"
		"                dh_group 2;\n"
		"        }\n"
		"        proposal {\n"
		"                encryption_algorithm 3des;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method %s;\n"
		"                dh_group 2;\n"
		"        }\n"
		"}\n\n", xauth, xauth, xauth);
	fprintf(fp, 
		"mode_cfg {\n"
		"        auth_source system;\n"
		"        pool_size 6;\n"
		"        network4 %s;\n"
		"        netmask4 255.255.255.0;\n"
		"        dns4 %s;\n"
		"        save_passwd on;\n"
		"        pfs_group 2;\n"
		"}\n\n", network4, dns4);
	fprintf(fp, 
		"sainfo anonymous {\n"
		"        encryption_algorithm aes 256, aes, 3des;\n"
		"        authentication_algorithm hmac_sha1;\n"
		"        compression_algorithm deflate;\n"
		"        pfs_group 2;\n"
		"}\n");
	fclose(fp);

	fp = fopen("/tmp/ipsec/psk.txt", "w");
	fprintf(fp, "nobody %s\n", nvram_safe_get("ipsec_psk"));
	fclose(fp);

	chmod("/tmp/ipsec/psk.txt", 0600);

	free(network4);

	ret = eval("racoon", "-f", "/tmp/ipsec/racoon.conf");

	_dprintf("start_ipsec: ret= %d\n", ret);
	return;
}

void stop_ipsec(void)
{
	kill_pidfile("/var/run/racoon.pid");

	return;
}

int ipsec_passwd(void)
{
	const char *shadow_file = "/etc/shadow.ipsec";
	const char *passwd_file = "/etc/passwd.ipsec";
	const char *users = "ipsec_users";
	int uid = 1000;

	char *nv, *nvp;
	FILE *sf, *pf;

	sf = fopen(shadow_file, "w");
	if (!sf) {
		return -1;
	}
	pf = fopen(passwd_file, "w");
	if (!pf) {
		fclose(sf);
		unlink(shadow_file);
		return -1;
	}

	nv = nvp = strdup(nvram_safe_get(users));
	if (nv) {
		const int gid = 65534; // nobody
		char *p, *username, *passwd, *b;
		char s[512], salt[32];

		strcpy(salt, "$1$");
		f_read("/dev/urandom", s, 6);
		base64_encode(s, salt + 3, 6);
		salt[3 + 8] = 0;
		for (p = salt; *p; p++) {
			if (*p == '+') *p = '.';
		}

		while ((b = strsep(&nvp, ">")) != NULL) {
			if (vstrsep(b, "<", &username, &passwd) == 2 && strlen(username) && strlen(passwd)) {
				p = crypt(passwd, salt);
				fprintf(sf, "%s:%s:0:0:99999:7:0:0:\n", username, p);
				fprintf(pf, "%s:x:%d:%d::/dev/null:/dev/null\n", username, uid, gid);
				uid++;
			}
		}
		free(nv);
	}

	fclose(pf);
	fclose(sf);

	chmod(shadow_file, 0600);
	chmod(passwd_file, 0644);
	return 0;
}
