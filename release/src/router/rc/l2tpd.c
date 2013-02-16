/*
 * l2tp.c
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

#define PHASE1_SCRIPT 1 // with this disabled, we will use a generic policy (which needs to be recreated each time wan-ip changes)

void start_l2tpd(void)
{
	int ret = 0, mss = 0, manual_dns = 0;
//	char *lpTemp;
	FILE *fp;

//	int pid = getpid();
//	_dprintf("start_l2tpd: getpid= %d\n", pid);

//	if(getpid() != 1) {
//		notify_rc("start_l2tpd");
//		return;
//	}

	if (!nvram_match("l2tpd_enable", "1")) {
		return;
	}
	// cprintf("stop vpn modules\n");
	// stop_vpn_modules ();

	// Create directory for use by l2tpd daemon and its supporting files
	mkdir("/tmp/l2tpd", 0744);
	cprintf("open options file\n");
	// Create options file that will be unique to l2tpd to avoid interference 
	// with pppoe and l2tp
	fp = fopen("/tmp/l2tpd/options.l2tpd", "w");
//	fprintf(fp, "logfile /var/log/l2tpd-pppd.log\n");

	cprintf("write config\n");
	fprintf(fp, "lock\n"
		"name \"l2tp-server\"\n"
//		"proxyarp\n"			// Better disable this
//		"dump\n"			// DUMP may crash the daemon
//		"debug\n"
		"minunit 4\n"			// AB !! - we leave ppp0-ppp3 for WAN and/or other ppp connections (PPTP client, ADSL, etc... perhaps)?
		"noccp\n"			// CCP seems to confuse Android clients, better turn it off
		"lcp-echo-interval 120\n"
		"lcp-echo-failure 10\n"
		"lcp-echo-adaptive\n"
		"idle 1800\n"
		"connect-delay 5000\n"
		"nodefaultroute\n"
		"noipdefault\n"
		"auth\n"
		"require-mschap-v2\n");

	fprintf(fp,
		"chap-secrets /tmp/l2tpd/chap-secrets\n"
		"ip-up-script /tmp/l2tpd/ip-up\n"
		"ip-down-script /tmp/l2tpd/ip-down\n"
		"mtu %s\n" "mru %s\n",
		nvram_get("l2tpd_mtu") ? nvram_get("l2tpd_mtu") : "1400",
		nvram_get("l2tpd_mru") ? nvram_get("l2tpd_mru") : "1400");
	//DNS Server
	if (strlen(nvram_safe_get("l2tpd_dns1"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("l2tpd_dns1"));
		manual_dns=1;
	}
	if (strlen(nvram_safe_get("l2tpd_dns2"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("l2tpd_dns2"));
		manual_dns=1;
	}
	if(!manual_dns && !nvram_match("lan_ipaddr", ""))
                fprintf(fp, "ms-dns %s\n", nvram_safe_get("lan_ipaddr"));

	fprintf(fp, "\n%s\n", nvram_safe_get("l2tpd_custom"));

//	fprintf(fp, "plugin pppol2tp.so\npppol2tp_debug_mask 15\n");
	fclose(fp);

	// Create l2tpd.conf options file for l2tpd daemon
	fp = fopen("/tmp/l2tpd/l2tpd.conf", "w");
	fprintf(fp,
		"[global]\n"
		"port = 1701\n"
//		"debug avp = yes\n"
//		"debug network = yes\n"
//		"debug packet = yes\n"
//		"debug tunnel = yes\n"
//		"debug state = yes\n"
//		"force userspace = yes\n"
		"access control = no\n");
	if (nvram_match("l2tpd_ipsec_saref", "1"))
		fprintf(fp, "ipsec saref = yes\n");	// will probably disable kernel mode
	fprintf(fp,
		"[lns default]\n"
		"exclusive = yes\n"
		"ip range = %s\n"
		"local ip = %s\n"
//		"hidden bit = no\n"
		"length bit = yes\n"
		"name = VPNServer\n"
		"ppp debug = yes\n"
		"require authentication = yes\n"
		"unix authentication = no\n"
		"require chap = yes\n"
		"refuse pap = yes\n"
		"pppoptfile = /tmp/l2tpd/options.l2tpd\n",
		nvram_safe_get("l2tpd_remoteip"),
		nvram_safe_get("lan_ipaddr"));
	fclose(fp);

	// Create ip-up and ip-down scripts that are unique to l2tpd to avoid
	// interference with pppoe and l2tp
	/*
	 * adjust for tunneling overhead (mtu - 40 byte IP - 108 byte tunnel
	 * overhead) 
	 */
	if (nvram_match("mtu_enable", "1"))
		mss = atoi(nvram_safe_get("wan_mtu")) - 40 - 108;
	else
		mss = 1500 - 40 - 108;
	char bcast[32];

	strcpy(bcast, nvram_safe_get("lan_ipaddr"));
	get_broadcast(bcast, nvram_safe_get("lan_netmask"));

	fp = fopen("/tmp/l2tpd/ip-up", "w");
//	fprintf(fp, "#!/bin/sh\n" "startservice set_routes\n"	// reinitialize 
	fprintf(fp, "#!/bin/sh\n" //"startservice set_routes\n"	// reinitialize 
		"echo $PPPD_PID $1 $5 $6 $PEERNAME `date +%%s`>> /tmp/l2tp_connected\n" 
		"iptables -I FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n" 
		"iptables -I INPUT -i $1 -j ACCEPT\n"
		"iptables -I FORWARD -i $1 -j ACCEPT\n"
		"iptables -I FORWARD -o $1 -j ACCEPT\n" // AB!!
		"iptables -t nat -I PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s "	// rule for wake on lan over l2tp tunnel
		"%s\n", bcast,
		nvram_get("l2tpd_ipup_script") ? nvram_get("l2tpd_ipup_script") : "");
	fclose(fp);
	fp = fopen("/tmp/l2tpd/ip-down", "w");
	fprintf(fp, "#!/bin/sh\n" "grep -v $1  /tmp/l2tp_connected > /tmp/l2tp_connected.new\n" 
		"mv /tmp/l2tp_connected.new /tmp/l2tp_connected\n" 
		"iptables -D FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n" 
		"iptables -D INPUT -i $1 -j ACCEPT\n" 
		"iptables -D FORWARD -i $1 -j ACCEPT\n" 
		"iptables -D FORWARD -o $1 -j ACCEPT\n" // AB!!
		"iptables -t nat -D PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s "	// rule for wake on lan over l2tp tunnel
		"%s\n", bcast,
		nvram_get("l2tpd_ipdown_script") ? nvram_get("l2tpd_ipdown_script") : "");
	fclose(fp);
	chmod("/tmp/l2tpd/ip-up", 0744);
	chmod("/tmp/l2tpd/ip-down", 0744);

	// Exctract chap-secrets from nvram
	write_chap_secret("l2tpd_users", "l2tp-server", "/tmp/l2tpd/chap-secrets");

	chmod("/tmp/l2tpd/chap-secrets", 0600);

	// racoon begin
	modprobe("af_key");
	modprobe("esp4");
	modprobe("xfrm4_mode_transport");

	// XXX use scripts?
	modprobe("xfrm_user");
#if !PHASE1_SCRIPT
	const char *wan_ipaddr = nvram_safe_get("wan_ipaddr");
	ret = eval("ip", "xfrm", "policy", "add", "src", wan_ipaddr, "dst", "0.0.0.0/0", "proto", "udp", "sport", "1701", "dir", "out", "tmpl", "proto", "esp", "mode", "transport", "level", "required");
	ret = eval("ip", "xfrm", "policy", "add", "src", "0.0.0.0/0", "dst", wan_ipaddr, "proto", "udp", "dport", "1701", "dir", "in", "tmpl", "proto", "esp", "mode", "transport", "level", "required");
#else
	fp = fopen("/tmp/l2tpd/ph1_script", "w");
	fprintf(fp, 
		"#!/bin/sh\n\n"
		"case $1 in\n"
		"    phase1_up)\n"
		"        ip xfrm policy add src $LOCAL_ADDR dst $REMOTE_ADDR proto udp sport 1701 dir out tmpl proto esp mode transport level required\n"
		"        ip xfrm policy add src $REMOTE_ADDR dst $LOCAL_ADDR proto udp dport 1701 dir in tmpl proto esp mode transport level required\n"
		"        ;;\n"
		"    phase1_down | phase1_dead)\n"
		"        ip xfrm policy delete src $LOCAL_ADDR dst $REMOTE_ADDR proto udp sport 1701 dir out\n"
		"        ip xfrm policy delete src $REMOTE_ADDR dst $LOCAL_ADDR proto udp dport 1701 dir in\n"
		"        ;;\n"
		"esac\n");
	fclose(fp);
	chmod("/tmp/l2tpd/ph1_script", 0744);
#endif

	fp = fopen("/tmp/l2tpd/racoon.conf", "w");
	fprintf(fp, 
		"path pre_shared_key \"/tmp/l2tpd/psk.txt\";\n"
#if PHASE1_SCRIPT
		"path script \"/tmp/l2tpd/\";\n"
#endif
		"\n"
		"remote anonymous {\n"
#if PHASE1_SCRIPT
		"        script \"ph1_script\" phase1_up;\n"
		"        script \"ph1_script\" phase1_down;\n"
		"        script \"ph1_script\" phase1_dead;\n"
#endif
		"        exchange_mode main;\n"
		"        nat_traversal on;\n"
		"        generate_policy on;\n"
		"        proposal_check obey;\n"
		"        proposal {\n"
		"                encryption_algorithm aes 256;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method pre_shared_key;\n"
		"                dh_group 2;\n"
		"        }\n"
		"        proposal {\n"
		"                encryption_algorithm aes;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method pre_shared_key;\n"
		"                dh_group 2;\n"
		"        }\n"
		"        proposal {\n"
		"                encryption_algorithm 3des;\n"
		"                hash_algorithm sha1;\n"
		"                authentication_method pre_shared_key;\n"
		"                dh_group 2;\n"
		"        }\n"
		"}\n"
		"\n"
		"sainfo anonymous {\n"
		"        encryption_algorithm aes 256, aes, 3des;\n"
		"        authentication_algorithm hmac_sha1;\n"
		"        compression_algorithm deflate;\n"
		"        pfs_group 2;\n"
		"}\n");
	fclose(fp);

	fp = fopen("/tmp/l2tpd/psk.txt", "w");
	fprintf(fp, "* %s\n", nvram_safe_get("ipsec_psk"));
	fclose(fp);

	chmod("/tmp/l2tpd/psk.txt", 0600);

	ret = eval("racoon", "-f", "/tmp/l2tpd/racoon.conf");
	// racoon end

	// Execute l2tpd daemon // XXX dBorca: use another exe name to avoid being killed by stop_ppp()
	eval("ln", "-s", "/usr/sbin/xl2tpd", "/tmp/l2tpd/xl2tpd-vpn");
	ret =
	    eval("/tmp/l2tpd/xl2tpd-vpn", "-c", "/tmp/l2tpd/l2tpd.conf",
		 "-p", "/var/run/xl2tpd-vpn.pid");

	_dprintf("start_l2tpd: ret= %d\n", ret);
	//dd_syslog(LOG_INFO, "l2tpd : l2tp daemon successfully started\n");
	return;
}

void stop_l2tpd(void)
{
	FILE *fp;
	int ppppid;
	char line[128];

	eval("cp", "/tmp/l2tp_connected", "/tmp/l2tp_shutdown");

	fp = fopen("/tmp/l2tp_shutdown", "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (sscanf(line, "%d %*s %*s %*s %*s %*d", &ppppid) != 1) continue;
			int n = 10;
			while ((kill(ppppid, SIGTERM) == 0) && (n > 1)) {
				sleep(1);
				n--;
			}
		}
		fclose(fp);
	}
	unlink("/tmp/l2tp_shutdown");

//	if (getpid() != 1) {
//		notify_rc("stop_l2tpd");
//	}

	kill_pidfile("/var/run/xl2tpd-vpn.pid");
	kill_pidfile("/var/run/racoon.pid");

#if !PHASE1_SCRIPT
	const char *wan_ipaddr = nvram_safe_get("wan_ipaddr");
	eval("ip", "xfrm", "policy", "delete", "src", wan_ipaddr, "dst", "0.0.0.0/0", "proto", "udp", "sport", "1701", "dir", "out");
	eval("ip", "xfrm", "policy", "delete", "src", "0.0.0.0/0", "dst", wan_ipaddr, "proto", "udp", "dport", "1701", "dir", "in");
#endif
	return;
}
