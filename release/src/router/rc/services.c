/*

	Copyright 2003, CyberTAN  Inc.  All Rights Reserved

	This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
	the contents of this file may not be disclosed to third parties,
	copied or duplicated in any form without the prior written
	permission of CyberTAN Inc.

	This software should be used as a reference only, and it not
	intended for production use!

	THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE

*/
/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/
/*

	Modified for Tomato Firmware
	Portions, Copyright (C) 2006-2009 Jonathan Zarate

*/
#include "rc.h"

#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

// !!TB
#include <sys/mount.h>
#include <mntent.h>
#include <dirent.h>

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

// Pop an alarm to recheck pids in 500 msec.
static const struct itimerval pop_tv = { {0,0}, {0, 500 * 1000} };

// Pop an alarm to reap zombies. 
static const struct itimerval zombie_tv = { {0,0}, {307, 0} };

// -----------------------------------------------------------------------------

static const char dmhosts[] = "/etc/hosts.dnsmasq";
static const char dmresolv[] = "/etc/resolv.dnsmasq";

static pid_t pid_dnsmasq = -1;

static int is_wet(int idx, int unit, int subunit, void *param)
{
	return nvram_match(wl_nvname("mode", unit, subunit), "wet");
}

void start_dnsmasq()
{
	FILE *f;
	const char *nv;
	char buf[512];
	char lan[24];
	const char *router_ip;
	const char *lan_ifname;
	char sdhcp_lease[32];
	char *e;
	int n;
	char *mac, *ip, *name;
	char *p;
	int ipn;
	char ipbuf[32];
	FILE *hf;
	int dhcp_start;
	int dhcp_count;
	int dhcp_lease;
	int do_dhcpd;
	int do_dns;

	TRACE_PT("begin\n");

	if (getpid() != 1) {
		start_service("dnsmasq");
		return;
	}

	stop_dnsmasq();

	if (foreach_wif(1, NULL, is_wet)) return;

	if ((f = fopen("/etc/dnsmasq.conf", "w")) == NULL) return;

	lan_ifname = nvram_safe_get("lan_ifname");
	router_ip = nvram_safe_get("lan_ipaddr");
	strlcpy(lan, router_ip, sizeof(lan));
	if ((p = strrchr(lan, '.')) != NULL) *(p + 1) = 0;

	fprintf(f,
		"pid-file=/var/run/dnsmasq.pid\n"
		"interface=%s\n",
		lan_ifname);
	if (((nv = nvram_get("wan_domain")) != NULL) || ((nv = nvram_get("wan_get_domain")) != NULL)) {
		if (*nv) fprintf(f, "domain=%s\n", nv);
	}

	// dns
	const dns_list_t *dns = get_dns();	// this always points to a static buffer

	if (((nv = nvram_get("dns_minport")) != NULL) && (*nv)) n = atoi(nv);
		else n = 4096;
	fprintf(f,
		"resolv-file=%s\n"		// the real stuff is here
		"addn-hosts=%s\n"		// "
		"expand-hosts\n"		// expand hostnames in hosts file
		"min-port=%u\n", 		// min port used for random src port
		dmresolv, dmhosts, n);
	do_dns = nvram_match("dhcpd_dmdns", "1");

	// DNS rebinding protection, will discard upstream RFC1918 responses
	if (nvram_get_int("dns_norebind")) {
		fprintf(f,
			"stop-dns-rebind\n"
			"rebind-localhost-ok\n");
		// allow RFC1918 responses for server domain
		switch (get_wan_proto()) {
		case WP_PPTP:
			nv = nvram_get("pptp_server_ip");
			break;
		case WP_L2TP:
			nv = nvram_get("l2tp_server_ip");
			break;
		default:
			nv = NULL;
			break;
		}
		if (nv && *nv) fprintf(f, "rebind-domain-ok=%s\n", nv);
	}

	for (n = 0 ; n < dns->count; ++n) {
		if (dns->dns[n].port != 53) {
			fprintf(f, "server=%s#%u\n", inet_ntoa(dns->dns[n].addr), dns->dns[n].port);
		}
	}

	// dhcp
	do_dhcpd = nvram_match("lan_proto", "dhcp");
	if (do_dhcpd) {
		dhcp_lease = nvram_get_int("dhcp_lease");
		if (dhcp_lease <= 0) dhcp_lease = 1440;

		if ((e = nvram_get("dhcpd_slt")) != NULL) n = atoi(e); else n = 0;
		if (n < 0) strcpy(sdhcp_lease, "infinite");
			else sprintf(sdhcp_lease, "%dm", (n > 0) ? n : dhcp_lease);

		if (!do_dns) {
			// if not using dnsmasq for dns

			if ((dns->count == 0) && (nvram_get_int("dhcpd_llndns"))) {
				// no DNS might be temporary. use a low lease time to force clients to update.
				dhcp_lease = 2;
				strcpy(sdhcp_lease, "2m");
				do_dns = 1;
			}
			else {
				// pass the dns directly
				buf[0] = 0;
				for (n = 0 ; n < dns->count; ++n) {
					if (dns->dns[n].port == 53) {	// check: option 6 doesn't seem to support other ports
						sprintf(buf + strlen(buf), ",%s", inet_ntoa(dns->dns[n].addr));
					}
				}
				fprintf(f, "dhcp-option=6%s\n", buf);
			}
		}

		if ((p = nvram_get("dhcpd_startip")) && (*p) && (e = nvram_get("dhcpd_endip")) && (*e)) {
			fprintf(f, "dhcp-range=%s,%s,%s,%dm\n", p, e, nvram_safe_get("lan_netmask"), dhcp_lease);
		}
		else {
			// for compatibility
			dhcp_start = nvram_get_int("dhcp_start");
			dhcp_count = nvram_get_int("dhcp_num");
			fprintf(f, "dhcp-range=%s%d,%s%d,%s,%dm\n",
				lan, dhcp_start, lan, dhcp_start + dhcp_count - 1, nvram_safe_get("lan_netmask"), dhcp_lease);
		}

		nv = router_ip;
		if ((nvram_get_int("dhcpd_gwmode") == 1) && (get_wan_proto() == WP_DISABLED)) {
			p = nvram_safe_get("lan_gateway");
			if ((*p) && (strcmp(p, "0.0.0.0") != 0)) nv = p;
		}

		n = nvram_get_int("dhcpd_lmax");
		fprintf(f,
			"dhcp-option=3,%s\n"	// gateway
			"dhcp-lease-max=%d\n",
			nv,
			(n > 0) ? n : 255);

		if (nvram_get_int("dhcpd_auth") >= 0) {
			fprintf(f, "dhcp-authoritative\n");
		}

		if (((nv = nvram_get("wan_wins")) != NULL) && (*nv) && (strcmp(nv, "0.0.0.0") != 0)) {
			fprintf(f, "dhcp-option=44,%s\n", nv);
		}
#ifdef TCONFIG_SAMBASRV
		else if (nvram_get_int("smbd_enable") && nvram_invmatch("lan_hostname", "") && nvram_get_int("smbd_wins")) {
			if ((nv == NULL) || (*nv == 0) || (strcmp(nv, "0.0.0.0") == 0)) {
				// Samba will serve as a WINS server
				fprintf(f, "dhcp-option=44,0.0.0.0\n");
			}
		}
#endif
	}
	else {
		fprintf(f, "no-dhcp-interface=%s\n", lan_ifname);
	}

	// write static lease entries & create hosts file

	if ((hf = fopen(dmhosts, "w")) != NULL) {
		if (((nv = nvram_get("wan_hostname")) != NULL) && (*nv))
			fprintf(hf, "%s %s\n", router_ip, nv);
#ifdef TCONFIG_SAMBASRV
		else if (((nv = nvram_get("lan_hostname")) != NULL) && (*nv))
			fprintf(hf, "%s %s\n", router_ip, nv);
#endif
		p = (char *)get_wanip();
		if ((*p == 0) || strcmp(p, "0.0.0.0") == 0)
			p = "127.0.0.1";
		fprintf(hf, "%s wan-ip\n", p);
		if (nv && (*nv))
			fprintf(hf, "%s %s-wan\n", p, nv);
	}

	// 00:aa:bb:cc:dd:ee<123<xxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 53 w/ delim
	// 00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 85 w/ delim
	// 00:aa:bb:cc:dd:ee,00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 106 w/ delim
	p = nvram_safe_get("dhcpd_static");
	while ((e = strchr(p, '>')) != NULL) {
		n = (e - p);
		if (n > 105) {
			p = e + 1;
			continue;
		}

		strncpy(buf, p, n);
		buf[n] = 0;
		p = e + 1;

		if ((e = strchr(buf, '<')) == NULL) continue;
		*e = 0;
		mac = buf;

		ip = e + 1;
		if ((e = strchr(ip, '<')) == NULL) continue;
		*e = 0;
		if (strchr(ip, '.') == NULL) {
			ipn = atoi(ip);
			if ((ipn <= 0) || (ipn > 255)) continue;
			sprintf(ipbuf, "%s%d", lan, ipn);
			ip = ipbuf;
		}
		else {
			if (inet_addr(ip) == INADDR_NONE) continue;
		}

		name = e + 1;

		if ((hf) && (*name != 0)) {
			fprintf(hf, "%s %s\n", ip, name);
		}

		if ((do_dhcpd) && (*mac != 0) && (strcmp(mac, "00:00:00:00:00:00") != 0)) {
			fprintf(f, "dhcp-host=%s,%s,%s\n", mac, ip, sdhcp_lease);
		}
	}

	if (hf) fclose(hf);

	//

#ifdef TCONFIG_OPENVPN
	write_vpn_dnsmasq_config(f);
#endif

	fprintf(f, "%s\n\n", nvram_safe_get("dnsmasq_custom"));

	fappend(f, "/etc/dnsmasq.custom");

	//

	fclose(f);

	if (do_dns) {
		unlink("/etc/resolv.conf");
		symlink("/rom/etc/resolv.conf", "/etc/resolv.conf");	// nameserver 127.0.0.1
	}

	TRACE_PT("run dnsmasq\n");

	// Default to some values we like, but allow the user to override them.
	eval("dnsmasq", "-c", "1500", "--log-async");

	if (!nvram_contains_word("debug_norestart", "dnsmasq")) {
		pid_dnsmasq = -2;
	}

	TRACE_PT("end\n");
}

void stop_dnsmasq(void)
{
	TRACE_PT("begin\n");

	if (getpid() != 1) {
		stop_service("dnsmasq");
		return;
	}

	pid_dnsmasq = -1;

	unlink("/etc/resolv.conf");
	symlink(dmresolv, "/etc/resolv.conf");

	killall_tk("dnsmasq");

	TRACE_PT("end\n");
}

void clear_resolv(void)
{
	f_write(dmresolv, NULL, 0, 0, 0);	// blank
}

#ifdef TCONFIG_IPV6
static int write_ipv6_dns_servers(FILE *f, const char *prefix, char *dns)
{
	char p[INET6_ADDRSTRLEN + 1], *next = NULL;
	struct in6_addr addr;
	int cnt = 0;

	foreach(p, dns, next) {
		// verify that this is a valid IPv6 address
		if (inet_pton(AF_INET6, p, &addr) == 1) {
			fprintf(f, "%s%s\n", prefix, p);
			++cnt;
		}
	}

	return cnt;
}
#endif

void dns_to_resolv(void)
{
	FILE *f;
	const dns_list_t *dns;
	int i;
	mode_t m;

	m = umask(022);	// 077 from pppoecd
	if ((f = fopen(dmresolv, "w")) != NULL) {
		// Check for VPN DNS entries
		if (!write_vpn_resolv(f)) {
#ifdef TCONFIG_IPV6
			if (write_ipv6_dns_servers(f, "nameserver ", nvram_safe_get("ipv6_dns")) == 0 || nvram_get_int("dns_addget"))
				write_ipv6_dns_servers(f, "nameserver ", nvram_safe_get("ipv6_get_dns"));
#endif
			dns = get_dns();	// static buffer
			if (dns->count == 0) {
				// Put a pseudo DNS IP to trigger Connect On Demand
				if (nvram_match("ppp_demand", "1")) {
					switch (get_wan_proto()) {
					case WP_PPPOE:
					case WP_PPTP:
					case WP_L2TP:
						fprintf(f, "nameserver 1.1.1.1\n");
						break;
					}
				}
			}
			else {
				for (i = 0; i < dns->count; i++) {
					if (dns->dns[i].port == 53) {	// resolv.conf doesn't allow for an alternate port
						fprintf(f, "nameserver %s\n", inet_ntoa(dns->dns[i].addr));
					}
				}
			}
		}
		fclose(f);
	}
	umask(m);
}

// -----------------------------------------------------------------------------

void start_httpd(void)
{
	stop_httpd();
	chdir("/www");
	xstart("httpd");
	chdir("/");
}

void stop_httpd(void)
{
	killall_tk("httpd");
}

// -----------------------------------------------------------------------------
#ifdef TCONFIG_IPV6

void start_ipv6_sit_tunnel(void)
{
	char *tun_dev = nvram_safe_get("ipv6_ifname");
	char ip[INET6_ADDRSTRLEN + 4];
	const char *wanip;

	if (get_ipv6_service() == IPV6_6IN4) {
		modprobe("sit");
		snprintf(ip, sizeof(ip), "%s/%d",
			nvram_safe_get("ipv6_tun_addr"),
			nvram_get_int("ipv6_tun_addrlen") ? : 64);
		wanip = get_wanip();

		eval("ip", "tunnel", "add", tun_dev, "mode", "sit",
			"remote", nvram_safe_get("ipv6_tun_v4end"),
			"local", (char *)wanip,
			"ttl", nvram_safe_get("ipv6_tun_ttl"));
		if (nvram_get_int("ipv6_tun_mtu") > 0)
			eval("ip", "link", "set", tun_dev, "mtu", nvram_safe_get("ipv6_tun_mtu"), "up");
		else
			eval("ip", "link", "set", tun_dev, "up");
		eval("ip", "addr", "add", ip, "dev", tun_dev);
		eval("ip", "route", "add", "::/0", "dev", tun_dev);
	}
}

void stop_ipv6_sit_tunnel(void)
{
	char *tun_dev = nvram_safe_get("ipv6_ifname");
	eval("ip", "tunnel", "del", tun_dev);
	modprobe_r("sit");
}

static char *tayga_dev = "nat64";
static char *tayga_ipv4 = "192.168.3.1";

void start_tayga(void)
{
	FILE *f;
	char *prefix;

	if (getpid() != 1) {
		start_service("tayga");
		return;
	}

	if (ipv6_enabled() && nvram_match("ipv6_nat64", "1")) {
		char *tayga_prefix = "192.168.3.0/24";
		char tayga_ipv6[100];

		prefix = nvram_safe_get("ipv6_prefix");
		sprintf(tayga_ipv6, "%s5", prefix);

		// Create tayga.conf
		if ((f = fopen("/etc/tayga.conf", "w")) == NULL) return;

		fprintf(f,
			"tun-device %s\n"
			"ipv4-addr %s\n"
			"ipv6-addr %s4\n"
			"prefix 64:ff9b::/96\n"
			"dynamic-pool %s\n",
			tayga_dev, tayga_ipv4,
			prefix, tayga_prefix);
		fclose(f);

		// Start radvd
                modprobe("tun");
		eval("tayga", "-c", "/etc/tayga.conf", "--mktun");
		eval("ip", "link", "set", tayga_dev, "up");
		eval("ip", "addr", "add", tayga_ipv6, "dev", tayga_dev);
		eval("ip", "addr", "add", tayga_ipv4, "dev", tayga_dev);
		eval("ip", "route", "add", tayga_prefix, "dev", tayga_dev);
		eval("ip", "route", "add", "64:ff9b::/96", "dev",
		     tayga_dev);
		eval("tayga", "-c", "/etc/tayga.conf");
	}
}

void stop_tayga(void)
{
	char tayga_ipv6[100];
	char *prefix;

	prefix = nvram_safe_get("ipv6_prefix");
	sprintf(tayga_ipv6, "%s5", prefix);

	if (getpid() != 1) {
		stop_service("tayga");
		return;
	}

	killall_tk("tayga");
	eval("ip", "addr", "del", tayga_ipv4, "dev", tayga_dev);
	eval("ip", "addr", "del", tayga_ipv6, "dev", tayga_dev);
	eval("tayga", "-c", "/etc/tayga.conf", "--rmtun");
}

static pid_t pid_radvd = -1;

void start_radvd(void)
{
	FILE *f;
	char *prefix, *ip;
	int do_dns;
	char *argv[] = { "radvd", NULL, NULL, NULL };
	int pid, argc;

	if (getpid() != 1) {
		start_service("radvd");
		return;
	}

	stop_radvd();

	if (ipv6_enabled() && nvram_match("ipv6_radvd", "1")) {

		switch (get_ipv6_service()) {
		case IPV6_NATIVE_DHCP:
			prefix = "::";	
			break;
		default:
			prefix = nvram_safe_get("ipv6_prefix");
			break;
		}
		if (!(*prefix)) return;

		// Create radvd.conf
		if ((f = fopen("/etc/radvd.conf", "w")) == NULL) return;

		ip = (char *)ipv6_router_address(NULL);
		do_dns = (*ip) && nvram_match("dhcpd_dmdns", "1");

		fprintf(f,
			"interface %s\n"
			"{\n"
			" AdvSendAdvert on;\n"
			" MaxRtrAdvInterval 60;\n"
			" AdvHomeAgentFlag off;\n"
			" AdvManagedFlag off;\n"
			" prefix %s/64 \n"
			" {\n"
			"  AdvOnLink on;\n"
			"  AdvAutonomous on;\n"
			" };\n"
			" %s%s%s\n"
			"};\n",
			nvram_safe_get("lan_ifname"), prefix,
			do_dns ? "RDNSS " : "", do_dns ? ip : "", do_dns ? " { };" : "");
		fclose(f);

		// Start radvd
		argc = 1;
		if (nvram_get_int("debug_ipv6")) {
			argv[argc++] = "-d";
			argv[argc++] = "10";
		}
		argv[argc] = NULL;
		_eval(argv, NULL, 0, &pid);

		if (!nvram_contains_word("debug_norestart", "radvd")) {
			pid_radvd = -2;
		}
	}
}

void stop_radvd(void)
{
	if (getpid() != 1) {
		stop_service("radvd");
		return;
	}

	pid_radvd = -1;
	killall_tk("radvd");
}

void start_ipv6(void)
{
	char *p;
	char ip[INET6_ADDRSTRLEN + 4];
	int service;

	service = get_ipv6_service();
	enable_ipv6(service != IPV6_DISABLED);
	enable_ip_forward();

	// Check if turned on
	switch (service) {
	case IPV6_NATIVE:
	case IPV6_6IN4:
		p = (char *)ipv6_router_address(NULL);
		if (*p) {
			snprintf(ip, sizeof(ip), "%s/%d", p, nvram_get_int("ipv6_prefix_length") ? : 64);
			eval("ip", "-6", "addr", "add", ip, "dev", nvram_safe_get("lan_ifname"));
		}
		break;
	}
}

void stop_ipv6(void)
{
	stop_ipv6_sit_tunnel();
	stop_dhcp6c();
	eval("ip", "-6", "addr", "flush", "scope", "global");
}

#endif

// -----------------------------------------------------------------------------

void start_upnp(void)
{
	if (getpid() != 1) {
		start_service("upnp");
		return;
	}

	if (get_wan_proto() == WP_DISABLED) return;

	int enable;
	FILE *f;
	int upnp_port;
	
	if (((enable = nvram_get_int("upnp_enable")) & 3) != 0) {
		mkdir("/etc/upnp", 0777);
		if (f_exists("/etc/upnp/config.alt")) {
			xstart("miniupnpd", "-f", "/etc/upnp/config.alt");
		}
		else {
			if ((f = fopen("/etc/upnp/config", "w")) != NULL) {
				upnp_port = nvram_get_int("upnp_port");
				if ((upnp_port < 0) || (upnp_port >= 0xFFFF)) upnp_port = 0;

				char *lanip = nvram_safe_get("lan_ipaddr");
				char *lanmask = nvram_safe_get("lan_netmask");
				
				fprintf(f,
					"ext_ifname=%s\n"
					"listening_ip=%s/%s\n"
					"port=%d\n"
					"enable_upnp=%s\n"
					"enable_natpmp=%s\n"
					"secure_mode=%s\n"
					"upnp_forward_chain=upnp\n"
					"upnp_nat_chain=upnp\n"
					"notify_interval=%d\n"
					"system_uptime=yes\n"
					"\n"
					,
					get_wanface(),
					lanip, lanmask,
					upnp_port,
					(enable & 1) ? "yes" : "no",						// upnp enable
					(enable & 2) ? "yes" : "no",						// natpmp enable
					nvram_get_int("upnp_secure") ? "yes" : "no",			// secure_mode (only forward to self)
					nvram_get_int("upnp_ssdp_interval")
				);

				if (nvram_get_int("upnp_clean")) {
					int interval = nvram_get_int("upnp_clean_interval");
					if (interval < 60) interval = 60;
					fprintf(f,
						"clean_ruleset_interval=%d\n"
						"clean_ruleset_threshold=%d\n",
						interval,
						nvram_get_int("upnp_clean_threshold")
					);
				}
				else
					fprintf(f,"clean_ruleset_interval=0\n");

				if (nvram_match("upnp_mnp", "1")) {
					int https = nvram_get_int("https_enable");
					fprintf(f, "presentation_url=http%s://%s:%s/forward-upnp.asp\n",
						https ? "s" : "", lanip,
						nvram_safe_get(https ? "https_lanport" : "http_lanport"));
				}
				else {
					// Empty parameters are not included into XML service description
					fprintf(f, "presentation_url=\n");
				}

				char uuid[45];
				f_read_string("/proc/sys/kernel/random/uuid", uuid, sizeof(uuid));
				fprintf(f, "uuid=%s\n", uuid);

				int ports[4];
				if ((ports[0] = nvram_get_int("upnp_min_port_int")) > 0 &&
				    (ports[1] = nvram_get_int("upnp_max_port_int")) > 0 &&
				    (ports[2] = nvram_get_int("upnp_min_port_ext")) > 0 &&
				    (ports[3] = nvram_get_int("upnp_max_port_ext")) > 0) {
					fprintf(f,
						"allow %d-%d %s/%s %d-%d\n",
						ports[0], ports[1],
						lanip, lanmask,
						ports[2], ports[3]
					);
				}
				else {
					// by default allow only redirection of ports above 1024
					fprintf(f, "allow 1024-65535 %s/%s 1024-65535\n", lanip, lanmask);
				}

				fappend(f, "/etc/upnp/config.custom");
				fprintf(f, "\ndeny 0-65535 0.0.0.0/0 0-65535\n");
				fclose(f);
				
				xstart("miniupnpd", "-f", "/etc/upnp/config");
			}
		}
	}
}

void stop_upnp(void)
{
	if (getpid() != 1) {
		stop_service("upnp");
		return;
	}

	killall_tk("miniupnpd");
}

// -----------------------------------------------------------------------------

static pid_t pid_crond = -1;

void start_cron(void)
{
	stop_cron();

	eval("crond", nvram_contains_word("log_events", "crond") ? NULL : "-l", "9");
	if (!nvram_contains_word("debug_norestart", "crond")) {
		pid_crond = -2;
	}
}

void stop_cron(void)
{
	pid_crond = -1;
	killall_tk("crond");
}

// -----------------------------------------------------------------------------
#ifdef LINUX26

static pid_t pid_hotplug2 = -1;

void start_hotplug2()
{
	stop_hotplug2();

	f_write_string("/proc/sys/kernel/hotplug", "", FW_NEWLINE, 0);
	xstart("hotplug2", "--persistent", "--no-coldplug");
	// FIXME: Don't remember exactly why I put "sleep" here -
	// but it was not for a race with check_services()... - TB
	sleep(1);

	if (!nvram_contains_word("debug_norestart", "hotplug2")) {
		pid_hotplug2 = -2;
	}
}

void stop_hotplug2(void)
{
	pid_hotplug2 = -1;
	killall_tk("hotplug2");
}

#endif	/* LINUX26 */
// -----------------------------------------------------------------------------

// Written by Sparq in 2002/07/16
void start_zebra(void)
{
#ifdef TCONFIG_ZEBRA
	if (getpid() != 1) {
		start_service("zebra");
		return;
	}

	FILE *fp;

	char *lan_tx = nvram_safe_get("dr_lan_tx");
	char *lan_rx = nvram_safe_get("dr_lan_rx");
	char *wan_tx = nvram_safe_get("dr_wan_tx");
	char *wan_rx = nvram_safe_get("dr_wan_rx");

	if ((*lan_tx == '0') && (*lan_rx == '0') && (*wan_tx == '0') && (*wan_rx == '0')) {
		return;
	}

	// empty
	if ((fp = fopen("/etc/zebra.conf", "w")) != NULL) {
		fclose(fp);
	}

	//
	if ((fp = fopen("/etc/ripd.conf", "w")) != NULL) {
		char *lan_ifname = nvram_safe_get("lan_ifname");
		char *wan_ifname = nvram_safe_get("wan_ifname");

		fprintf(fp, "router rip\n");
		fprintf(fp, "network %s\n", lan_ifname);
		fprintf(fp, "network %s\n", wan_ifname);
		fprintf(fp, "redistribute connected\n");
		//fprintf(fp, "redistribute static\n");

		// 43011: modify by zg 2006.10.18 for cdrouter3.3 item 173(cdrouter_rip_30) bug
		// fprintf(fp, "redistribute kernel\n"); // 1.11: removed, redistributes indirect -- zzz

		fprintf(fp, "interface %s\n", lan_ifname);
		if (*lan_tx != '0') fprintf(fp, "ip rip send version %s\n", lan_tx);
		if (*lan_rx != '0') fprintf(fp, "ip rip receive version %s\n", lan_rx);

		fprintf(fp, "interface %s\n", wan_ifname);
		if (*wan_tx != '0') fprintf(fp, "ip rip send version %s\n", wan_tx);
		if (*wan_rx != '0') fprintf(fp, "ip rip receive version %s\n", wan_rx);

		fprintf(fp, "router rip\n");
		if (*lan_tx == '0') fprintf(fp, "distribute-list private out %s\n", lan_ifname);
		if (*lan_rx == '0') fprintf(fp, "distribute-list private in %s\n", lan_ifname);
		if (*wan_tx == '0') fprintf(fp, "distribute-list private out %s\n", wan_ifname);
		if (*wan_rx == '0') fprintf(fp, "distribute-list private in %s\n", wan_ifname);
		fprintf(fp, "access-list private deny any\n");

		//fprintf(fp, "debug rip events\n");
		//fprintf(fp, "log file /etc/ripd.log\n");
		fclose(fp);
	}

	xstart("zebra", "-d");
	xstart("ripd",  "-d");
#endif
}

void stop_zebra(void)
{
#ifdef TCONFIG_ZEBRA
	if (getpid() != 1) {
		stop_service("zebra");
		return;
	}

	killall("zebra", SIGTERM);
	killall("ripd", SIGTERM);

	unlink("/etc/zebra.conf");
	unlink("/etc/ripd.conf");
#endif
}

// -----------------------------------------------------------------------------

void start_syslog(void)
{
	char *argv[16];
	int argc;
	char *nv;
	char *b_opt = "";
	char rem[256];
	int n;
	char s[64];
	char cfg[256];
	char *rot_siz = "50";

	argv[0] = "syslogd";
	argc = 1;

	if (nvram_match("log_remote", "1")) {
		nv = nvram_safe_get("log_remoteip");
		if (*nv) {
			snprintf(rem, sizeof(rem), "%s:%s", nv, nvram_safe_get("log_remoteport"));
			argv[argc++] = "-R";
			argv[argc++] = rem;
		}
	}

	if (nvram_match("log_file", "1")) {
		argv[argc++] = "-L";

		/* Read options:    rotate_size(kb)    num_backups    logfilename.
		 * Ignore these settings and use defaults if the logfile cannot be written to.
		 */
		if (f_read_string("/etc/syslogd.cfg", cfg, sizeof(cfg)) > 0) {
			if ((nv = strchr(cfg, '\n')))
				*nv = 0;

			if ((nv = strtok(cfg, " \t"))) {
				if (isdigit(*nv))
					rot_siz = nv;
			}

			if ((nv = strtok(NULL, " \t")))
				b_opt = nv;

			if ((nv = strtok(NULL, " \t")) && *nv == '/') {
				if (f_write(nv, cfg, 0, FW_APPEND, 0) >= 0) {
					argv[argc++] = "-O";
					argv[argc++] = nv;
				}
				else {
					rot_siz = "50";
					b_opt = "";
				}
			}
		}

		argv[argc++] = "-s";
		argv[argc++] = rot_siz;

		if (isdigit(*b_opt)) {
			argv[argc++] = "-b";
			argv[argc++] = b_opt;
		}
	}

	if (argc > 1) {
		argv[argc] = NULL;
		_eval(argv, NULL, 0, NULL);

		argv[0] = "klogd";
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);

		// used to be available in syslogd -m
		n = nvram_get_int("log_mark");
		if (n > 0) {
			// n is in minutes
			if (n < 60)
				sprintf(rem, "*/%d * * * *", n);
			else if (n < 60 * 24)
				sprintf(rem, "0 */%d * * *", n / 60);
			else
				sprintf(rem, "0 0 */%d * *", n / (60 * 24));
			sprintf(s, "cru a syslogdmark \"%s logger -p syslog.info -- -- MARK --\"", rem);
			system(s);
		}
		else {
			system("cru d syslogdmark");
		}
	}
}

void stop_syslog(void)
{
	killall("klogd", SIGTERM);
	killall("syslogd", SIGTERM);
}

// -----------------------------------------------------------------------------

static pid_t pid_igmp = -1;

void start_igmp_proxy(void)
{
	FILE *fp;

	pid_igmp = -1;
	if (nvram_match("multicast_pass", "1")) {
		if (get_wan_proto() == WP_DISABLED)
			return;

		if (f_exists("/etc/igmp.alt")) {
			eval("igmpproxy", "/etc/igmp.alt");
		}
		else if ((fp = fopen("/etc/igmp.conf", "w")) != NULL) {
			fprintf(fp,
				"quickleave\n"
				"phyint %s upstream\n"
				"\taltnet %s\n"
				"phyint %s downstream ratelimit 0\n",
				get_wanface(),
				nvram_get("multicast_altnet") ? : "0.0.0.0/0",
				nvram_safe_get("lan_ifname"));
			fclose(fp);
			eval("igmpproxy", "/etc/igmp.conf");
		}
		else {
			return;
		}
		if (!nvram_contains_word("debug_norestart", "igmprt")) {
			pid_igmp = -2;
		}
	}
}

void stop_igmp_proxy(void)
{
	pid_igmp = -1;
	killall_tk("igmpproxy");
}


// -----------------------------------------------------------------------------

void set_tz(void)
{
	f_write_string("/etc/TZ", nvram_safe_get("tm_tz"), FW_CREATE|FW_NEWLINE, 0644);
}

void start_ntpc(void)
{
	set_tz();

	stop_ntpc();

	if (nvram_get_int("ntp_updates") >= 0) {
		xstart("ntpsync", "--init");
	}
}

void stop_ntpc(void)
{
	killall("ntpsync", SIGTERM);
}

// -----------------------------------------------------------------------------

static void stop_rstats(void)
{
	int n;
	int pid;

	n = 60;
	while ((n-- > 0) && ((pid = pidof("rstats")) > 0)) {
		if (kill(pid, SIGTERM) != 0) break;
		sleep(1);
	}
}

static void start_rstats(int new)
{
	if (nvram_match("rstats_enable", "1")) {
		stop_rstats();
		if (new) xstart("rstats", "--new");
			else xstart("rstats");
	}
}

// -----------------------------------------------------------------------------

// !!TB - FTP Server

#ifdef TCONFIG_FTP
static char *get_full_storage_path(char *val)
{
	static char buf[128];
	int len;

	if (val[0] == '/')
		len = sprintf(buf, "%s", val);
	else
		len = sprintf(buf, "%s/%s", MOUNT_ROOT, val);

	if (len > 1 && buf[len - 1] == '/')
		buf[len - 1] = 0;

	return buf;
}

static char *nvram_storage_path(char *var)
{
	char *val = nvram_safe_get(var);
	return get_full_storage_path(val);
}

char vsftpd_conf[] = "/etc/vsftpd.conf";
char vsftpd_users[] = "/etc/vsftpd.users";
char vsftpd_passwd[] = "/etc/vsftpd.passwd";

/* VSFTPD code mostly stolen from Oleg's ASUS Custom Firmware GPL sources */

static void start_ftpd(void)
{
	char tmp[256];
	FILE *fp, *f;
	char *buf;
	char *p, *q;
	char *user, *pass, *rights;

	if (getpid() != 1) {
		start_service("ftpd");
		return;
	}

	if (!nvram_get_int("ftp_enable")) return;

	mkdir_if_none(vsftpd_users);
	mkdir_if_none("/var/run/vsftpd");

	if ((fp = fopen(vsftpd_conf, "w")) == NULL)
		return;

	if (nvram_get_int("ftp_super"))
	{
		/* rights */
		sprintf(tmp, "%s/%s", vsftpd_users, "admin");
		if ((f = fopen(tmp, "w")))
		{
			fprintf(f,
				"dirlist_enable=yes\n"
				"write_enable=yes\n"
				"download_enable=yes\n");
			fclose(f);
		}
	}

#ifdef TCONFIG_SAMBASRV
	if (nvram_match("smbd_cset", "utf8"))
		fprintf(fp, "utf8=yes\n");
#endif

	if (nvram_invmatch("ftp_anonymous", "0"))
	{
		fprintf(fp,
			"anon_allow_writable_root=yes\n"
			"anon_world_readable_only=no\n"
			"anon_umask=022\n");
		
		/* rights */
		sprintf(tmp, "%s/ftp", vsftpd_users);
		if ((f = fopen(tmp, "w")))
		{
			if (nvram_match("ftp_dirlist", "0"))
				fprintf(f, "dirlist_enable=yes\n");
			if (nvram_match("ftp_anonymous", "1") || 
			    nvram_match("ftp_anonymous", "3"))
				fprintf(f, "write_enable=yes\n");
			if (nvram_match("ftp_anonymous", "1") || 
			    nvram_match("ftp_anonymous", "2"))
				fprintf(f, "download_enable=yes\n");
			fclose(f);
		}
		if (nvram_match("ftp_anonymous", "1") || 
		    nvram_match("ftp_anonymous", "3"))
			fprintf(fp, 
				"anon_upload_enable=yes\n"
				"anon_mkdir_write_enable=yes\n"
				"anon_other_write_enable=yes\n");
	} else {
		fprintf(fp, "anonymous_enable=no\n");
	}
	
	fprintf(fp,
		"dirmessage_enable=yes\n"
		"download_enable=no\n"
		"dirlist_enable=no\n"
		"hide_ids=yes\n"
		"syslog_enable=yes\n"
		"local_enable=yes\n"
		"local_umask=022\n"
		"chmod_enable=no\n"
		"chroot_local_user=yes\n"
		"check_shell=no\n"
		"log_ftp_protocol=%s\n"
		"user_config_dir=%s\n"
		"passwd_file=%s\n"
		"listen%s=yes\n"
		"listen_port=%s\n"
		"background=yes\n"
		"isolate=no\n"
		"max_clients=%d\n"
		"max_per_ip=%d\n"
		"max_login_fails=1\n"
		"idle_session_timeout=%s\n"
		"use_sendfile=no\n"
		"anon_max_rate=%d\n"
		"local_max_rate=%d\n"
		"%s\n",
		nvram_get_int("log_ftp") ? "yes" : "no",
		vsftpd_users, vsftpd_passwd,
#ifdef TCONFIG_IPV6
		ipv6_enabled() ? "_ipv6" : "",
#else
		"",
#endif
		nvram_get("ftp_port") ? : "21",
		nvram_get_int("ftp_max"),
		nvram_get_int("ftp_ipmax"),
		nvram_get("ftp_staytimeout") ? : "300",
		nvram_get_int("ftp_anonrate") * 1024,
		nvram_get_int("ftp_rate") * 1024,
		nvram_safe_get("ftp_custom"));

	fclose(fp);

	/* prepare passwd file and default users */
	if ((fp = fopen(vsftpd_passwd, "w")) == NULL)
		return;

	if (((user = nvram_get("http_username")) == NULL) || (*user == 0)) user = "admin";
	if (((pass = nvram_get("http_passwd")) == NULL) || (*pass == 0)) pass = "admin";

	fprintf(fp, /* anonymous, admin, nobody */
		"ftp:x:0:0:ftp:%s:/sbin/nologin\n"
		"%s:%s:0:0:root:/:/sbin/nologin\n"
		"nobody:x:65534:65534:nobody:%s/:/sbin/nologin\n",
		nvram_storage_path("ftp_anonroot"), user,
		nvram_get_int("ftp_super") ? crypt(pass, "$1$") : "x",
		MOUNT_ROOT);

	if ((buf = strdup(nvram_safe_get("ftp_users"))) != NULL)
	{
		/*
		username<password<rights
		rights:
			Read/Write
			Read Only
			View Only
			Private
		*/
		p = buf;
		while ((q = strsep(&p, ">")) != NULL) {
			if (vstrsep(q, "<", &user, &pass, &rights) != 3) continue;
			if (!user || !pass) continue;

			/* directory */
			if (strncmp(rights, "Private", 7) == 0)
			{
				sprintf(tmp, "%s/%s", nvram_storage_path("ftp_pvtroot"), user);
				mkdir_if_none(tmp);
			}
			else
				sprintf(tmp, "%s", nvram_storage_path("ftp_pubroot"));

			fprintf(fp, "%s:%s:0:0:%s:%s:/sbin/nologin\n",
				user, crypt(pass, "$1$"), user, tmp);

			/* rights */
			sprintf(tmp, "%s/%s", vsftpd_users, user);
			if ((f = fopen(tmp, "w")))
			{
				tmp[0] = 0;
				if (nvram_invmatch("ftp_dirlist", "1"))
					strcat(tmp, "dirlist_enable=yes\n");
				if (strstr(rights, "Read") || !strcmp(rights, "Private"))
					strcat(tmp, "download_enable=yes\n");
				if (strstr(rights, "Write") || !strncmp(rights, "Private", 7))
					strcat(tmp, "write_enable=yes\n");
					
				fputs(tmp, f);
				fclose(f);
			}
		}
		free(buf);
	}

	fclose(fp);
	killall("vsftpd", SIGHUP);

	/* start vsftpd if it's not already running */
	if (pidof("vsftpd") <= 0)
		xstart("vsftpd");
}

static void stop_ftpd(void)
{
	if (getpid() != 1) {
		stop_service("ftpd");
		return;
	}

	killall_tk("vsftpd");
	unlink(vsftpd_passwd);
	unlink(vsftpd_conf);
	eval("rm", "-rf", vsftpd_users);
}
#endif	// TCONFIG_FTP

// -----------------------------------------------------------------------------

// !!TB - Samba

#ifdef TCONFIG_SAMBASRV
static void kill_samba(int sig)
{
	if (sig == SIGTERM) {
		killall_tk("smbd");
		killall_tk("nmbd");
	}
	else {
		killall("smbd", sig);
		killall("nmbd", sig);
	}
}

static void start_samba(void)
{
	FILE *fp;
	DIR *dir = NULL;
	struct dirent *dp;
	char nlsmod[15];
	int mode;
	char *nv;

	if (getpid() != 1) {
		start_service("smbd");
		return;
	}

	mode = nvram_get_int("smbd_enable");
	if (!mode || !nvram_invmatch("lan_hostname", ""))
		return;

	if ((fp = fopen("/etc/smb.conf", "w")) == NULL)
		return;

	fprintf(fp, "[global]\n"
		" interfaces = %s\n"
		" bind interfaces only = yes\n"
		" workgroup = %s\n"
		" netbios name = %s\n"
		" server string = %s\n"
		" guest account = nobody\n"
		" security = user\n"
		" %s\n"
		" guest ok = %s\n"
		" guest only = no\n"
		" browseable = yes\n"
		" syslog only = yes\n"
		" timestamp logs = no\n"
		" syslog = 1\n"
		" encrypt passwords = yes\n"
		" preserve case = yes\n"
		" short preserve case = yes\n",
		nvram_safe_get("lan_ifname"),
		nvram_get("smbd_wgroup") ? : "WORKGROUP",
		nvram_safe_get("lan_hostname"),
		nvram_get("router_name") ? : "Tomato",
		mode == 2 ? "" : "map to guest = Bad User",
		mode == 2 ? "no" : "yes"	// guest ok
	);

	if (nvram_get_int("smbd_wins")) {
		nv = nvram_safe_get("wan_wins");
		if ((*nv == 0) || (strcmp(nv, "0.0.0.0") == 0)) {
			fprintf(fp, " wins support = yes\n");
		}
	}

	if (nvram_get_int("smbd_master")) {
		fprintf(fp,
			" domain master = yes\n"
			" local master = yes\n"
			" preferred master = yes\n"
			" os level = 65\n");
	}

	nv = nvram_safe_get("smbd_cpage");
	if (*nv) {
#ifndef TCONFIG_SAMBA3
		fprintf(fp, " client code page = %s\n", nv);
#endif
		sprintf(nlsmod, "nls_cp%s", nv);

		nv = nvram_safe_get("smbd_nlsmod");
		if ((*nv) && (strcmp(nv, nlsmod) != 0))
			modprobe_r(nv);

		modprobe(nlsmod);
		nvram_set("smbd_nlsmod", nlsmod);
	}

#ifndef TCONFIG_SAMBA3
	if (nvram_match("smbd_cset", "utf8"))
		fprintf(fp, " coding system = utf8\n");
	else if (nvram_invmatch("smbd_cset", ""))
		fprintf(fp, " character set = %s\n", nvram_safe_get("smbd_cset"));
#endif

	nv = nvram_safe_get("smbd_custom");
	/* add socket options unless overriden by the user */
	if (strstr(nv, "socket options") == NULL) {
		fprintf(fp, " socket options = TCP_NODELAY SO_KEEPALIVE IPTOS_LOWDELAY SO_RCVBUF=65536 SO_SNDBUF=65536\n");
	}
	fprintf(fp, "%s\n\n", nv);

	/* configure shares */

	char *buf;
	char *p, *q;
	char *name, *path, *comment, *writeable, *hidden;
	int cnt = 0;

	if ((buf = strdup(nvram_safe_get("smbd_shares"))) != NULL)
	{
		/* sharename<path<comment<writeable[0|1]<hidden[0|1] */

		p = buf;
		while ((q = strsep(&p, ">")) != NULL) {
			if (vstrsep(q, "<", &name, &path, &comment, &writeable, &hidden) != 5) continue;
			if (!path || !name) continue;

			/* share name */
			fprintf(fp, "\n[%s]\n", name);

			/* path */
			fprintf(fp, " path = %s\n", path);

			/* access level */
			if (!strcmp(writeable, "1"))
				fprintf(fp, " writable = yes\n delete readonly = yes\n force user = root\n");
			if (!strcmp(hidden, "1"))
				fprintf(fp, " browseable = no\n");

			/* comment */
			if (comment)
				fprintf(fp, " comment = %s\n", comment);

			cnt++;
		}
		free(buf);
	}

	/* Share every mountpoint below MOUNT_ROOT */
	if (nvram_get_int("smbd_autoshare") && (dir = opendir(MOUNT_ROOT))) {
		while ((dp = readdir(dir))) {
			if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {

				char path[256];
				struct stat sb;
				int thisdev;

				/* Only if is a directory and is mounted */
				sprintf(path, "%s/%s", MOUNT_ROOT, dp->d_name);
				sb.st_mode = S_IFDIR;	/* failsafe */
				stat(path, &sb);
				if (!S_ISDIR(sb.st_mode))
					continue;

				/* If this dir & its parent dir are on the same device, it is not a mountpoint */
				strcat(path, "/.");
				stat(path, &sb);
				thisdev = sb.st_dev;
				strcat(path, ".");
				++sb.st_dev;	/* failsafe */
				stat(path, &sb);
				if (thisdev == sb.st_dev)
					continue;

				/* smbd_autoshare: 0 - disable, 1 - read-only, 2 - writable, 3 - hidden writable */
				fprintf(fp, "\n[%s]\n path = %s/%s\n comment = %s\n",
					dp->d_name, MOUNT_ROOT, dp->d_name, dp->d_name);
				if (nvram_match("smbd_autoshare", "3"))	// Hidden
					fprintf(fp, "\n[%s$]\n path = %s/%s\n browseable = no\n",
						dp->d_name, MOUNT_ROOT, dp->d_name);
				if (nvram_match("smbd_autoshare", "2") || nvram_match("smbd_autoshare", "3"))	// RW
					fprintf(fp, " writable = yes\n delete readonly = yes\n force user = root\n");

				cnt++;
			}
		}
	}
	if (dir) closedir(dir);

	if (cnt == 0) {
		/* by default share MOUNT_ROOT as read-only */
		fprintf(fp, "\n[share]\n"
			" path = %s\n"
			" writable = no\n",
			MOUNT_ROOT);
	}

	fclose(fp);

	mkdir_if_none("/var/run/samba");
	mkdir_if_none("/etc/samba");

	/* write smbpasswd */
#ifdef TCONFIG_SAMBA3
	eval("smbpasswd", "nobody", "\"\"");
#else
	eval("smbpasswd", "-a", "nobody", "\"\"");
#endif
	if (mode == 2) {
		char *smbd_user;
		if (((smbd_user = nvram_get("smbd_user")) == NULL) || (*smbd_user == 0) || !strcmp(smbd_user, "root"))
			smbd_user = "nas";
#ifdef TCONFIG_SAMBA3
		eval("smbpasswd", smbd_user, nvram_safe_get("smbd_passwd"));
#else
		eval("smbpasswd", "-a", smbd_user, nvram_safe_get("smbd_passwd"));
#endif
	}

	kill_samba(SIGHUP);
	int ret1 = 0, ret2 = 0;
	/* start samba if it's not already running */
	if (pidof("nmbd") <= 0)
		ret1 = xstart("nmbd", "-D");
	if (pidof("smbd") <= 0)
		ret2 = xstart("smbd", "-D");

	if (ret1 || ret2) kill_samba(SIGTERM);
}

static void stop_samba(void)
{
	if (getpid() != 1) {
		stop_service("smbd");
		return;
	}

	kill_samba(SIGTERM);
	/* clean up */
	unlink("/var/log/smb");
	unlink("/var/log/nmb");
	eval("rm", "-rf", "/var/run/samba");
}
#endif	// TCONFIG_SAMBASRV

#ifdef TCONFIG_MEDIA_SERVER
#define MEDIA_SERVER_APP	"minidlna"

static void start_media_server(void)
{
	FILE *f;
	int port, pid, https;
	char *dbdir;
	char *argv[] = { MEDIA_SERVER_APP, "-f", "/etc/"MEDIA_SERVER_APP".conf", "-R", NULL };
	static int once = 1;

	if (getpid() != 1) {
		start_service("media");
		return;
	}

	if (nvram_get_int("ms_sas") == 0)
		once = 0;

	if (nvram_get_int("ms_enable") != 0) {
		if ((!once) && (nvram_get_int("ms_rescan") == 0)) {
			// no forced rescan
			argv[3] = NULL;
		}
		nvram_unset("ms_rescan");

		if (f_exists("/etc/"MEDIA_SERVER_APP".alt")) {
			argv[2] = "/etc/"MEDIA_SERVER_APP".alt";
		}
		else {
			if ((f = fopen(argv[2], "w")) != NULL) {
				port = nvram_get_int("ms_port");
				https = nvram_get_int("https_enable");
				dbdir = nvram_safe_get("ms_dbdir");
				if (!(*dbdir)) dbdir = NULL;
				mkdir_if_none(dbdir ? : "/var/run/"MEDIA_SERVER_APP);

				fprintf(f,
					"network_interface=%s\n"
					"port=%d\n"
					"friendly_name=%s\n"
					"db_dir=%s/.db\n"
					"enable_tivo=%s\n"
					"strict_dlna=%s\n"
					"presentation_url=http%s://%s:%s/nas-media.asp\n"
					"inotify=yes\n"
					"notify_interval=600\n"
					"album_art_names=Cover.jpg/cover.jpg/Thumb.jpg/thumb.jpg\n"
					"\n",
					nvram_safe_get("lan_ifname"),
					(port < 0) || (port >= 0xffff) ? 0 : port,
					nvram_get("router_name") ? : "Tomato",
					dbdir ? : "/var/run/"MEDIA_SERVER_APP,
					nvram_get_int("ms_tivo") ? "yes" : "no",
					nvram_get_int("ms_stdlna") ? "yes" : "no",
					https ? "s" : "", nvram_safe_get("lan_ipaddr"), nvram_safe_get(https ? "https_lanport" : "http_lanport")
				);

				// media directories
				char *buf, *p, *q;
				char *path, *restrict;

				if ((buf = strdup(nvram_safe_get("ms_dirs"))) != NULL) {
					/* path<restrict[A|V|P|] */

					p = buf;
					while ((q = strsep(&p, ">")) != NULL) {
						if (vstrsep(q, "<", &path, &restrict) < 1 || !path || !(*path))
							continue;
						fprintf(f, "media_dir=%s%s%s\n",
							restrict ? : "", (restrict && *restrict) ? "," : "", path);
					}
					free(buf);
				}

				fclose(f);
			}
		}

		/* start media server if it's not already running */
		if (pidof(MEDIA_SERVER_APP) <= 0) {
			if ((_eval(argv, NULL, 0, &pid) == 0) && (once)) {
				/* If we started the media server successfully, wait 1 sec
				 * to let it die if it can't open the database file.
				 * If it's still alive after that, assume it's running and
				 * disable forced once-after-reboot rescan.
				 */
				sleep(1);
				if (pidof(MEDIA_SERVER_APP) > 0)
					once = 0;
			}
		}
	}
}

static void stop_media_server(void)
{
	if (getpid() != 1) {
		stop_service("media");
		return;
	}

	killall_tk(MEDIA_SERVER_APP);
}
#endif	// TCONFIG_MEDIA_SERVER

#ifdef TCONFIG_USB
static void start_nas_services(void)
{
	if (getpid() != 1) {
		start_service("usbapps");
		return;
	}

#ifdef TCONFIG_SAMBASRV
	start_samba();
#endif
#ifdef TCONFIG_FTP
	start_ftpd();
#endif
#ifdef TCONFIG_MEDIA_SERVER
	start_media_server();
#endif
}

static void stop_nas_services(void)
{
	if (getpid() != 1) {
		stop_service("usbapps");
		return;
	}

#ifdef TCONFIG_MEDIA_SERVER
	stop_media_server();
#endif
#ifdef TCONFIG_FTP
	stop_ftpd();
#endif
#ifdef TCONFIG_SAMBASRV
	stop_samba();
#endif
}

void restart_nas_services(int stop, int start)
{
	int fd = file_lock("usb");
	/* restart all NAS applications */
	if (stop)
		stop_nas_services();
	if (start)
		start_nas_services();
	file_unlock(fd);
}
#endif // TCONFIG_USB

// -----------------------------------------------------------------------------

/* -1 = Don't check for this program, it is not expected to be running.
 * Other = This program has been started and should be kept running.  If no
 * process with the name is running, call func to restart it.
 * Note: At startup, dnsmasq forks a short-lived child which forks a
 * long-lived (grand)child.  The parents terminate.
 * Many daemons use this technique.
 */
static void _check(pid_t pid, const char *name, void (*func)(void))
{
	if (pid == -1) return;

	if (pidof(name) > 0) return;

	syslog(LOG_DEBUG, "%s terminated unexpectedly, restarting.\n", name);
	func();

	// Force recheck in 500 msec
	setitimer(ITIMER_REAL, &pop_tv, NULL);
}

void check_services(void)
{
	TRACE_PT("keep alive\n");

	// Periodically reap any zombies
	setitimer(ITIMER_REAL, &zombie_tv, NULL);

#ifdef LINUX26
	_check(pid_hotplug2, "hotplug2", start_hotplug2);
#endif
	_check(pid_dnsmasq, "dnsmasq", start_dnsmasq);
	_check(pid_crond, "crond", start_cron);
	_check(pid_igmp, "igmpproxy", start_igmp_proxy);
#ifdef TCONFIG_IPV6
	_check(pid_radvd, "radvd", start_radvd);
#endif
}

// -----------------------------------------------------------------------------

void start_services(void)
{
	static int once = 1;

	if (once) {
		once = 0;

		if (nvram_get_int("telnetd_eas")) start_telnetd();
		if (nvram_get_int("sshd_eas")) start_sshd();
	}

//	start_syslog();
	start_nas();
	start_zebra();
	start_dnsmasq();
	start_cifs();
	start_httpd();
	start_cron();
//	start_upnp();
	start_rstats(0);
	start_sched();
#ifdef TCONFIG_IPV6
	/* note: starting radvd here might be too early in case of
	 * DHCPv6 because we won't have received a prefix and so it
	 * will disable advertisements, but the SIGHUP sent from
	 * dhcp6c-state will restart them.
	 */
	start_radvd();
	start_tayga();
#endif
	restart_nas_services(1, 1);	// !!TB - Samba, FTP and Media Server
}

void stop_services(void)
{
	clear_resolv();

	restart_nas_services(1, 0);	// stop Samba, FTP and Media Server
#ifdef TCONFIG_IPV6
	stop_radvd();
	stop_tayga();
#endif
	stop_sched();
	stop_rstats();
//	stop_upnp();
	stop_cron();
	stop_httpd();
	stop_cifs();
	stop_dnsmasq();
	stop_zebra();
	stop_nas();
//	stop_syslog();
}

// -----------------------------------------------------------------------------

/* nvram "action_service" is: "service-action[-modifier]"
 * action is something like "stop" or "start" or "restart"
 * optional modifier is "c" for the "service" command-line command
 */
void exec_service(void)
{
	const int A_START = 1;
	const int A_STOP = 2;
	const int A_RESTART = 1|2;
	char buffer[128];
	char *service;
	char *act;
	char *next;
	char *modifier;
	int action, user;
	int i;

	strlcpy(buffer, nvram_safe_get("action_service"), sizeof(buffer));
	next = buffer;

TOP:
	act = strsep(&next, ",");
	service = strsep(&act, "-");
	if (act == NULL) {
		next = NULL;
		goto CLEAR;
	}
	modifier = act;
	strsep(&modifier, "-");

	TRACE_PT("service=%s action=%s modifier=%s\n", service, act, modifier ? : "");

	if (strcmp(act, "start") == 0) action = A_START;
		else if (strcmp(act, "stop") == 0) action = A_STOP;
		else if (strcmp(act, "restart") == 0) action = A_RESTART;
		else action = 0;
	user = (modifier != NULL && *modifier == 'c');

	if (strcmp(service, "dhcpc") == 0) {
		if (action & A_STOP) stop_dhcpc();
		if (action & A_START) start_dhcpc();
		goto CLEAR;
	}

	if ((strcmp(service, "dhcpd") == 0) || (strcmp(service, "dns") == 0) || (strcmp(service, "dnsmasq") == 0)) {
		if (action & A_STOP) stop_dnsmasq();
		if (action & A_START) {
			dns_to_resolv();
			start_dnsmasq();
		}
		goto CLEAR;
	}

	if (strcmp(service, "firewall") == 0) {
		if (action & A_STOP) {
			stop_firewall();
			stop_igmp_proxy();
		}
		if (action & A_START) {
			start_firewall();
			start_igmp_proxy();
		}
		goto CLEAR;
	}

	if (strcmp(service, "restrict") == 0) {
		if (action & A_STOP) {
			stop_firewall();
		}
		if (action & A_START) {
			i = nvram_get_int("rrules_radio");	// -1 = not used, 0 = enabled by rule, 1 = disabled by rule

			start_firewall();

			// if radio was disabled by access restriction, but no rule is handling it now, enable it
			if (i == 1) {
				if (nvram_get_int("rrules_radio") < 0) {
					eval("radio", "on");
				}
			}
		}
		goto CLEAR;
	}

	if (strcmp(service, "qos") == 0) {
		if (action & A_STOP) {
			stop_qos();
		}
		stop_firewall(); start_firewall();		// always restarted
		if (action & A_START) {
			start_qos();
			if (nvram_match("qos_reset", "1")) f_write_string("/proc/net/clear_marks", "1", 0, 0);
		}
		goto CLEAR;
	}

	if (strcmp(service, "upnp") == 0) {
		if (action & A_STOP) {
			stop_upnp();
		}
		stop_firewall(); start_firewall();		// always restarted
		if (action & A_START) {
			start_upnp();
		}
		goto CLEAR;
	}

	if (strcmp(service, "telnetd") == 0) {
		if (action & A_STOP) stop_telnetd();
		if (action & A_START) start_telnetd();
		goto CLEAR;
	}

	if (strcmp(service, "sshd") == 0) {
		if (action & A_STOP) stop_sshd();
		if (action & A_START) start_sshd();
		goto CLEAR;
	}

	if (strcmp(service, "httpd") == 0) {
		if (action & A_STOP) stop_httpd();
		if (action & A_START) start_httpd();
		goto CLEAR;
	}
	
#ifdef TCONFIG_IPV6
	if (strcmp(service, "ipv6") == 0) {
		if (action & A_STOP) {
			stop_radvd();
			stop_ipv6();
		}
		if (action & A_START) {
			start_ipv6();
			start_radvd();
		}
		goto CLEAR;
	}
	
	if (strcmp(service, "radvd") == 0) {
		if (action & A_STOP) {
			stop_radvd();
		}
		if (action & A_START) {
			start_radvd();
		}
		goto CLEAR;
	}

	if (strcmp(service, "tayga") == 0) {
		if (action & A_STOP) {
			stop_tayga();
		}
		if (action & A_START) {
			start_tayga();
		}
		goto CLEAR;
	}

	if (strncmp(service, "dhcp6", 5) == 0) {
		if (action & A_STOP) {
			stop_dhcp6c();
		}
		if (action & A_START) {
			start_dhcp6c();
		}
		goto CLEAR;
	}
#endif
	
	if (strcmp(service, "admin") == 0) {
		if (action & A_STOP) {
			stop_sshd();
			stop_telnetd();
			stop_httpd();
		}
		stop_firewall(); start_firewall();		// always restarted
		if (action & A_START) {
			start_httpd();
			create_passwd();
			if (nvram_match("telnetd_eas", "1")) start_telnetd();
			if (nvram_match("sshd_eas", "1")) start_sshd();
		}
		goto CLEAR;
	}

	if (strcmp(service, "ddns") == 0) {
		if (action & A_STOP) stop_ddns();
		if (action & A_START) start_ddns();
		goto CLEAR;
	}

	if (strcmp(service, "ntpc") == 0) {
		if (action & A_STOP) stop_ntpc();
		if (action & A_START) start_ntpc();
		goto CLEAR;
	}

	if (strcmp(service, "logging") == 0) {
		if (action & A_STOP) {
			stop_syslog();
		}
		if (!user) {
			// always restarted except from "service" command
			stop_cron(); start_cron();
			stop_firewall(); start_firewall();
		}
		if (action & A_START) {
			start_syslog();
		}
		goto CLEAR;
	}

	if (strcmp(service, "crond") == 0) {
		if (action & A_STOP) {
			stop_cron();
		}
		if (action & A_START) {
			start_cron();
		}
		goto CLEAR;
	}

#ifdef LINUX26
	if (strncmp(service, "hotplug", 7) == 0) {
		if (action & A_STOP) {
			stop_hotplug2();
		}
		if (action & A_START) {
			start_hotplug2(1);
		}
		goto CLEAR;
	}
#endif

	if (strcmp(service, "upgrade") == 0) {
		if (action & A_START) {
#if TOMATO_SL
			stop_usbevent();
			stop_smbd();
#endif
			restart_nas_services(1, 0);	// stop Samba, FTP and Media Server
			stop_jffs2();
//			stop_cifs();
			stop_zebra();
			stop_cron();
			stop_ntpc();
			stop_upnp();
//			stop_dhcpc();
			killall("rstats", SIGTERM);
			killall("buttons", SIGTERM);
			stop_syslog();
			remove_storage_main(1);	// !!TB - USB Support
			stop_usb();		// !!TB - USB Support
		}
		goto CLEAR;
	}

#ifdef TCONFIG_CIFS
	if (strcmp(service, "cifs") == 0) {
		if (action & A_STOP) stop_cifs();
		if (action & A_START) start_cifs();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_JFFS2
	if (strncmp(service, "jffs", 4) == 0) {
		if (action & A_STOP) stop_jffs2();
		if (action & A_START) start_jffs2();
		goto CLEAR;
	}
#endif

	if (strcmp(service, "zebra") == 0) {
		if (action & A_STOP) stop_zebra();
		if (action & A_START) start_zebra();
		goto CLEAR;
	}

	if (strcmp(service, "routing") == 0) {
		if (action & A_STOP) {
			stop_zebra();
			do_static_routes(0);	// remove old '_saved'
			eval("brctl", "stp", nvram_safe_get("lan_ifname"), "0");
		}
		stop_firewall();
		start_firewall();
		if (action & A_START) {
			do_static_routes(1);	// add new
			start_zebra();
			eval("brctl", "stp", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_stp"));
		}
		goto CLEAR;
	}

	if (strcmp(service, "ctnf") == 0) {
		if (action & A_START) {
			setup_conntrack();
			stop_firewall();
			start_firewall();
		}
		goto CLEAR;
	}

	if (strcmp(service, "wan") == 0) {
		if (action & A_STOP) {
			stop_wan();
		}

		if (action & A_START) {
			rename("/tmp/ppp/log", "/tmp/ppp/log.~");
			start_wan(BOOT);
			sleep(2);
			force_to_dial();
		}
		goto CLEAR;
	}

	if (strcmp(service, "net") == 0) {
		if (action & A_STOP) {
#ifdef TCONFIG_IPV6
			stop_radvd();
#endif
			stop_httpd();
			stop_dnsmasq();
			stop_nas();
			stop_wan();
			stop_lan();
			stop_vlan();
		}
		if (action & A_START) {
			start_vlan();
			start_lan();
			start_wan(BOOT);
			start_nas();
			start_dnsmasq();
			start_httpd();
#ifdef TCONFIG_IPV6
			start_radvd();
#endif
			start_wl();
		}
		goto CLEAR;
	}

	if (strcmp(service, "nas") == 0) {
		if (action & A_STOP) {
			stop_nas();
		}
		if (action & A_START) {
			start_nas();
			start_wl();
		}
		goto CLEAR;
	}

	if (strcmp(service, "rstats") == 0) {
		if (action & A_STOP) stop_rstats();
		if (action & A_START) start_rstats(0);
		goto CLEAR;
	}

	if (strcmp(service, "rstatsnew") == 0) {
		if (action & A_STOP) stop_rstats();
		if (action & A_START) start_rstats(1);
		goto CLEAR;
	}

	if (strcmp(service, "sched") == 0) {
		if (action & A_STOP) stop_sched();
		if (action & A_START) start_sched();
		goto CLEAR;
	}

#ifdef TCONFIG_USB
	// !!TB - USB Support
	if (strcmp(service, "usb") == 0) {
		if (action & A_STOP) stop_usb();
		if (action & A_START) {
			start_usb();
			// restart Samba and ftp since they may be killed by stop_usb()
			restart_nas_services(0, 1);
			// remount all partitions by simulating hotplug event
			add_remove_usbhost("-1", 1);
		}
		goto CLEAR;
	}

	if (strcmp(service, "usbapps") == 0) {
		if (action & A_STOP) stop_nas_services();
		if (action & A_START) start_nas_services();
		goto CLEAR;
	}
#endif
	
#ifdef TCONFIG_FTP
	// !!TB - FTP Server
	if (strcmp(service, "ftpd") == 0) {
		if (action & A_STOP) stop_ftpd();
		setup_conntrack();
		stop_firewall();
		start_firewall();
		if (action & A_START) start_ftpd();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_MEDIA_SERVER
	if (strcmp(service, "media") == 0 || strcmp(service, "dlna") == 0) {
		if (action & A_STOP) stop_media_server();
		if (action & A_START) start_media_server();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_SAMBASRV
	// !!TB - Samba
	if (strcmp(service, "samba") == 0 || strcmp(service, "smbd") == 0) {
		if (action & A_STOP) stop_samba();
		if (action & A_START) {
			create_passwd();
			stop_dnsmasq();
			start_dnsmasq();
			start_samba();
		}
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_OPENVPN
	if (strncmp(service, "vpnclient", 9) == 0) {
		if (action & A_STOP) stop_vpnclient(atoi(&service[9]));
		if (action & A_START) start_vpnclient(atoi(&service[9]));
		goto CLEAR;
	}

	if (strncmp(service, "vpnserver", 9) == 0) {
		if (action & A_STOP) stop_vpnserver(atoi(&service[9]));
		if (action & A_START) start_vpnserver(atoi(&service[9]));
		goto CLEAR;
	}
#endif

CLEAR:
	if (next) goto TOP;

	// some functions check action_service and must be cleared at end	-- zzz
	nvram_set("action_service", "");

	// Force recheck in 500 msec
	setitimer(ITIMER_REAL, &pop_tv, NULL);
}

static void do_service(const char *name, const char *action, int user)
{
	int n;
	char s[64];

	n = 150;
	while (!nvram_match("action_service", "")) {
		if (user) {
			putchar('*');
			fflush(stdout);
		}
		else if (--n < 0) break;
		usleep(100 * 1000);
	}

	snprintf(s, sizeof(s), "%s-%s%s", name, action, (user ? "-c" : ""));
	nvram_set("action_service", s);
	kill(1, SIGUSR1);

	n = 150;
	while (nvram_match("action_service", s)) {
		if (user) {
			putchar('.');
			fflush(stdout);
		}
		else if (--n < 0) {
			break;
		}
		usleep(100 * 1000);
	}
}

int service_main(int argc, char *argv[])
{
	if (argc != 3) usage_exit(argv[0], "<service> <action>");
	do_service(argv[1], argv[2], 1);
	printf("\nDone.\n");
	return 0;
}

void start_service(const char *name)
{
	do_service(name, "start", 0);
}

void stop_service(const char *name)
{
	do_service(name, "stop", 0);
}

/*
void restart_service(const char *name)
{
	do_service(name, "restart", 0);
}
*/
