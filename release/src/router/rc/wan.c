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

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <bcmdevs.h>

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

static const char ppp_linkfile[] = "/tmp/ppp/link";
static const char ppp_optfile[]  = "/tmp/ppp/options";

static void make_secrets(void)
{
	FILE *f;
	char *user;
	char *pass;
	
	user = nvram_safe_get("ppp_username");
	pass = nvram_safe_get("ppp_passwd");
	if ((f = fopen("/tmp/ppp/pap-secrets", "w")) != NULL) {
		fprintf(f, "\"%s\" * \"%s\" *\n", user, pass);
		fclose(f);
	}
	chmod("/tmp/ppp/pap-secrets", 0600);

	if ((f = fopen("/tmp/ppp/chap-secrets", "w")) != NULL) {
		fprintf(f, "\"%s\" * \"%s\" *\n", user, pass);
		fclose(f);
	}
	chmod("/tmp/ppp/chap-secrets", 0600);
}

// -----------------------------------------------------------------------------

static int config_pppd(int wan_proto, int num)
{
	TRACE_PT("begin\n");

	FILE *fp;
	char *p;
	int debug, demand;

	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
#ifdef TCONFIG_IPV6
	symlink("/sbin/rc", "/tmp/ppp/ipv6-up");
	symlink("/sbin/rc", "/tmp/ppp/ipv6-down");
#endif
	symlink("/dev/null", "/tmp/ppp/connect-errors");

	demand = nvram_get_int("ppp_demand");
	debug = nvram_get_int("debug_ppp") ||
		(wan_proto == WP_PPPOE && nvram_contains_word("log_events", "pppoe"));

	// Generate options file
	if ((fp = fopen(ppp_optfile, "w")) == NULL) {
		perror(ppp_optfile);
		return -1;
	}

	fprintf(fp,
		"unit %d\n"
		"defaultroute\n"	// Add a default route to the system routing tables, using the peer as the gateway
		"usepeerdns\n"		// Ask the peer for up to 2 DNS server addresses
		"user '%s'\n"
		"default-asyncmap\n"	// Disable  asyncmap  negotiation
		"nopcomp\n"		// Disable protocol field compression
		"noaccomp\n"		// Disable Address/Control compression
		"novj\n"		// Disable Van Jacobson style TCP/IP header compression
		"nobsdcomp\n"		// Disable BSD-Compress  compression
		"nodeflate\n"		// Disable Deflate compression
		"noauth\n"		// Do not authenticate peer
		"refuse-eap\n"		// Do not use eap
		"maxfail 0\n"		// Never give up
		"lcp-echo-interval %d\n"// Interval between LCP echo-requests
		"lcp-echo-failure %d\n"	// Tolerance to unanswered echo-requests
		"lcp-echo-adaptive\n"	// Suppress LCP echo-requests if traffic was received
		"%s",			// Debug
		num,
		nvram_safe_get("ppp_username"),
		nvram_get_int("pppoe_lei") ? : 30,
		nvram_get_int("pppoe_lef") ? : 6,
		debug ? "debug\n" : "");

	if (wan_proto != WP_L2TP) {
		fprintf(fp,
			"persist\n"
			"holdoff %d\n",
			demand ? 30 : (nvram_get_int("ppp_redialperiod") ? : 30));
	}

	switch (wan_proto) {
	case WP_PPTP:
		fprintf(fp,
			"plugin pptp.so\n"
			"pptp_server %s\n"
			"nomppe-stateful\n"
			"mtu %d\n",
			nvram_safe_get("pptp_server_ip"),
			nvram_get_int("mtu_enable") ? nvram_get_int("wan_mtu") : 1400);
		break;
	case WP_PPPOE:
		if (((p = nvram_get("ppp_service")) != NULL) && (*p)) {
			fprintf(fp, "rp_pppoe_service '%s'\n", p);
		}
		if (((p = nvram_get("ppp_ac")) != NULL) && (*p)) {
			fprintf(fp, "rp_pppoe_ac '%s'\n", p);
		}
		fprintf(fp,
			"password '%s'\n"
			"plugin rp-pppoe.so\n"
			"nomppe nomppc\n"
			"nic-%s\n"
			"mru %d mtu %d\n",
			nvram_safe_get("ppp_passwd"),
			nvram_safe_get("wan_ifname"),
			nvram_get_int("wan_mtu"), nvram_get_int("wan_mtu"));
		break;
	case WP_L2TP:
		fprintf(fp, "nomppe nomppc\n");
		if (nvram_get_int("mtu_enable"))
			fprintf(fp, "mtu %d\n", nvram_get_int("wan_mtu"));
		break;
	}

	if (demand) {
		// demand mode
		fprintf(fp,
			"demand\n"		// Dial on demand
			"idle %d\n"
			"ipcp-accept-remote\n"
			"ipcp-accept-local\n"
			"noipdefault\n"		// Disables  the  default  behaviour when no local IP address is specified
			"ktune\n",		// Set /proc/sys/net/ipv4/ip_dynaddr to 1 in demand mode if the local address changes
			nvram_get_int("ppp_idletime") * 60);
	}

#ifdef TCONFIG_IPV6
	switch (get_ipv6_service()) {
	case IPV6_NATIVE:
	case IPV6_NATIVE_DHCP:
		fprintf(fp, "+ipv6\n");
		break;
	}
#endif
	// User specific options
	fprintf(fp, "%s\n", nvram_safe_get("ppp_custom"));

	fclose(fp);
	make_secrets();

	TRACE_PT("end\n");
	return 0;
}

static void stop_ppp(void)
{
	TRACE_PT("begin\n");

	unlink(ppp_linkfile);

	killall_tk("ip-up");
	killall_tk("ip-down");
#ifdef TCONFIG_IPV6
	killall_tk("ipv6-up");
	killall_tk("ipv6-down");
#endif
	killall_tk("xl2tpd");
	killall_tk("pppd");
	killall_tk("listen");

	TRACE_PT("end\n");
}

static void run_pppd(void)
{
	eval("pppd");

	if (nvram_get_int("ppp_demand")) {
		// demand mode
		/*
		   Fixed issue id 7887(or 7787):
		   When DUT is PPTP Connect on Demand mode, it couldn't be trigger from LAN.
		*/
		stop_dnsmasq();
		dns_to_resolv();
		start_dnsmasq();

		// Trigger Connect On Demand if user ping pptp server
		eval("listen", nvram_safe_get("lan_ifname"));
	}
	else {
		// keepalive mode
		start_redial();
	}
}

// -----------------------------------------------------------------------------

inline void stop_pptp(void)
{
	stop_ppp();
}

void start_pptp(int mode)
{
	TRACE_PT("begin\n");

	if (!using_dhcpc()) stop_dhcpc();
	stop_pptp();

	if (config_pppd(WP_PPTP, 0) != 0)
		return;

	if (!using_dhcpc()) {
		// Bring up  WAN interface
		ifconfig(nvram_safe_get("wan_ifname"), IFUP,
			nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
	}

	run_pppd();

	TRACE_PT("end\n");
}

// -----------------------------------------------------------------------------

void preset_wan(char *ifname, char *gw, char *netmask)
{
	int i = 0;
	int metric = nvram_get_int("ppp_defgw") ? 2 : 0;

	/* Delete all default routes */
	while ((route_del(ifname, metric, NULL, NULL, NULL) == 0) || (i++ < 10));

	/* Set default route to gateway if specified */
	i = 5;
	while ((route_add(ifname, metric, "0.0.0.0", gw, "0.0.0.0") == 1) && (i--)) {
		sleep(1);
	}
	_dprintf("set default gateway=%s n=%d\n", gw, i);

	/* Add routes to dns servers as well for demand ppp to work */
	char word[100], *next;
	in_addr_t mask = inet_addr(netmask);
	foreach(word, nvram_safe_get("wan_get_dns"), next) {
		if ((inet_addr(word) & mask) != (inet_addr(nvram_safe_get("wan_ipaddr")) & mask))
			route_add(ifname, metric, word, gw, "255.255.255.255");
	}

	dns_to_resolv();
	start_dnsmasq();
	sleep(1);
	start_firewall();
}

// -----------------------------------------------------------------------------


// Get the IP, Subnetmask, Geteway from WAN interface and set nvram
static void start_tmp_ppp(int num, char *ifname)
{
	int timeout;
	struct ifreq ifr;
	int s;

	TRACE_PT("begin: num=%d\n", num);

	if (num != 0) return;

	// Wait for ppp0 to be created
	timeout = 15;
	while ((ifconfig(ifname, IFUP, NULL, NULL) != 0) && (timeout-- > 0)) {
		sleep(1);
		_dprintf("[%d] waiting for %s %d...\n", __LINE__, ifname, timeout);
	}

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) return;
	strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);

	// Set temporary IP address
	timeout = 3;
	while (ioctl(s, SIOCGIFADDR, &ifr) && timeout--){
		_dprintf("[%d] waiting for %s...\n", __LINE__, ifname);
		sleep(1);
	};
	nvram_set("wan_ipaddr", inet_ntoa(sin_addr(&(ifr.ifr_addr))));
	nvram_set("wan_netmask", "255.255.255.255");

	// Set temporary P-t-P address
	timeout = 3;
	while (ioctl(s, SIOCGIFDSTADDR, &ifr) && timeout--){
		_dprintf("[%d] waiting for %s...\n", __LINE__, ifname);
		sleep(1);
	}
	nvram_set("wan_gateway", inet_ntoa(sin_addr(&(ifr.ifr_dstaddr))));
	
	close(s);

	start_wan_done(ifname);
	TRACE_PT("end\n");
}

void start_pppoe(int num)
{
	char ifname[8];

	TRACE_PT("begin pppoe_num=%d\n", num);
	
	if (num != 0) return;

	stop_pppoe();

	snprintf(ifname, sizeof(ifname), "ppp%d", num);

	if (config_pppd(WP_PPPOE, num) != 0)
		return;

	run_pppd();

	if (nvram_get_int("ppp_demand"))
		start_tmp_ppp(num, ifname);
	else
		ifconfig(ifname, IFUP, NULL, NULL);

	TRACE_PT("end\n");
}

void stop_pppoe(void)
{
	stop_ppp();
}

#if 0
void stop_singe_pppoe(int num)
{
	_dprintf("%s pppoe_num=%d\n", __FUNCTION__, num);

	int i;

	if (num != 0) return;
	
	i = nvram_get_int("pppoe_pid0");
	if ((i > 1) && (kill(i, SIGTERM) == 0)) {
		do {
			sleep(2);
		} while (kill(i, SIGKILL) == 0);
	}

	unlink(ppp_linkfile);
	nvram_unset("pppoe_ifname0");

	nvram_set("wan_get_dns", "");
	clear_resolv();
}
#endif

// -----------------------------------------------------------------------------

inline void stop_l2tp(void)
{
	stop_ppp();
}

void start_l2tp(void)
{
	TRACE_PT("begin\n");

	FILE *fp;
	int demand;

	stop_l2tp();

	if (config_pppd(WP_L2TP, 0) != 0)
		return;

	demand = nvram_get_int("ppp_demand");

	/* Generate XL2TPD configuration file */
	if ((fp = fopen("/etc/xl2tpd.conf", "w")) == NULL)
		return;
	fprintf(fp,
		"[global]\n"
		"access control = no\n"
		"port = 1701\n"
		"[lac l2tp]\n"
		"lns = %s\n"
		"pppoptfile = %s\n"
		"redial = yes\n"
		"max redials = 32767\n"
		"redial timeout = %d\n"
		"ppp debug = %s\n",
		nvram_safe_get("l2tp_server_ip"),
		ppp_optfile,
		demand ? 30 : (nvram_get_int("ppp_redialperiod") ? : 30),
		nvram_get_int("debug_ppp") ? "yes" : "no");
	fclose(fp);

	enable_ip_forward();

	eval("xl2tpd");

	if (demand) {
		eval("listen", nvram_safe_get("lan_ifname"));
	}
	else {
		force_to_dial();
		start_redial();
	}

	TRACE_PT("end\n");
}

// -----------------------------------------------------------------------------

static char *wan_gateway(void)
{
	char *gw = nvram_safe_get("wan_gateway_get");
	if ((*gw == 0) || (strcmp(gw, "0.0.0.0") == 0))
		gw = nvram_safe_get("wan_gateway");
	return gw;
}

// -----------------------------------------------------------------------------

// trigger connect on demand
void force_to_dial(void)
{
	TRACE_PT("begin\n");

	sleep(1);
	switch (get_wan_proto()) {
	case WP_L2TP:
		f_write_string("/var/run/l2tp-control", "c l2tp", 0, 0);
		break;
	case WP_PPTP:
		eval("ping", "-c", "2", "10.112.112.112");
		break;
	case WP_DISABLED:
	case WP_STATIC:
		break;
	default:
		eval("ping", "-c", "2", wan_gateway());
		break;
	}
	
	TRACE_PT("end\n");
}

// -----------------------------------------------------------------------------

static void _do_wan_routes(char *ifname, char *nvname, int metric, int add)
{
	char *routes, *tmp;
	int bits;
	struct in_addr mask;
	char netmask[16];

	// IP[/MASK] ROUTER IP2[/MASK2] ROUTER2 ...
	tmp = routes = strdup(nvram_safe_get(nvname));
	while (tmp && *tmp) {
		char *ipaddr, *gateway, *nmask;

		ipaddr = nmask = strsep(&tmp, " ");
		strcpy(netmask, "255.255.255.255");

		if (nmask) {
			ipaddr = strsep(&nmask, "/");
			if (nmask && *nmask) {
				bits = strtol(nmask, &nmask, 10);
				if (bits >= 1 && bits <= 32) {
					mask.s_addr = htonl(0xffffffff << (32 - bits));
					strcpy(netmask, inet_ntoa(mask));
				}
			}
		}
		gateway = strsep(&tmp, " ");

		if (gateway && *gateway) {
			if (add) route_add(ifname, metric + 1, ipaddr, gateway, netmask);
			else route_del(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);
}

void do_wan_routes(char *ifname, int metric, int add)
{
	if (nvram_get_int("dhcp_routes")) {
		// Static Routes:		IP ROUTER IP2 ROUTER2 ...
		// Classless Static Routes:	IP/MASK ROUTER IP2/MASK2 ROUTER2 ...
		_do_wan_routes(ifname, "wan_routes1", metric, add);
		_do_wan_routes(ifname, "wan_routes2", metric, add);
	}
}

// -----------------------------------------------------------------------------

const char wan_connecting[] = "/var/lib/misc/wan.connecting";

static int is_sta(int idx, int unit, int subunit, void *param)
{
	char **p = param;

	if (nvram_match(wl_nvname("mode", unit, subunit), "sta")) {
		*p = nvram_safe_get(wl_nvname("ifname", unit, subunit));
		return 1;
	}
	return 0;	
}

void start_wan(int mode)
{
	int wan_proto;
	char *wan_ifname;
	char *p = NULL;
	struct ifreq ifr;
	int sd;
	int max;
	int mtu;
	char buf[128];

	TRACE_PT("begin\n");

	f_write(wan_connecting, NULL, 0, 0, 0);
	
	//

	if (!foreach_wif(1, &p, is_sta)) {
		p = nvram_safe_get("wan_ifnameX");
		set_mac(p, "mac_wan", 1);
	}
	nvram_set("wan_ifname", p);
	nvram_set("wan_ifnames", p);

	//

	wan_ifname = nvram_safe_get("wan_ifname");
	if (wan_ifname[0] == 0) {
		wan_ifname = "none";
		nvram_set("wan_ifname", wan_ifname);
	}

	if (strcmp(wan_ifname, "none") == 0) {
		nvram_set("wan_proto", "disabled");
		syslog(LOG_INFO, "No WAN");
	}
	
	//
	
	wan_proto = get_wan_proto();

	// set the default gateway for WAN interface
	nvram_set("wan_gateway_get", nvram_safe_get("wan_gateway"));

	if (wan_proto == WP_DISABLED) {
		start_wan_done(wan_ifname);
		return;
	}

	if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket");
		return;
	}
	
	// MTU

	switch (wan_proto) {
	case WP_PPPOE:
		max = 1492;
		break;
	case WP_PPTP:
	case WP_L2TP:
		max = 1460;
		break;
	default:
		max = 1500;
		break;
	}
	if (nvram_match("mtu_enable", "0")) {
		mtu = max;
	}
	else {
		mtu = nvram_get_int("wan_mtu");
		if (mtu > max) mtu = max;
			else if (mtu < 576) mtu = 576;
	}
	sprintf(buf, "%d", mtu);
	nvram_set("wan_mtu", buf);
	nvram_set("wan_run_mtu", buf);

	// 43011: zhijian 2006-12-25 for CD-Router v3.4 mtu bug of PPTP connection mode
/*	if (wan_proto == WP_PPTP) {
		mtu += 40;
	} */	// commented out; checkme -- zzz
	
	if (wan_proto != WP_PPTP && wan_proto != WP_L2TP && wan_proto != WP_PPPOE) {
		// Don't set the MTU on the port for PPP connections, it will be set on the link instead
		ifr.ifr_mtu =  mtu;
		strcpy(ifr.ifr_name, wan_ifname);
		ioctl(sd, SIOCSIFMTU, &ifr);
	}

	//
	
	ifconfig(wan_ifname, IFUP, NULL, NULL);
	
	set_host_domain_name();

	switch (wan_proto) {
	case WP_PPPOE:
		start_pppoe(PPPOE0);
		break;
	case WP_DHCP:
	case WP_L2TP:
	case WP_PPTP:
		if (using_dhcpc()) {
			stop_dhcpc();
			start_dhcpc();
		}
		else if (wan_proto != WP_DHCP) {
			ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
			p = nvram_safe_get("wan_gateway");
			if ((*p != 0) && (strcmp(p, "0.0.0.0") != 0))
				preset_wan(wan_ifname, p, nvram_safe_get("wan_netmask"));

			switch (wan_proto) {
			case WP_PPTP:
				start_pptp(mode);
				break;
			case WP_L2TP:
				start_l2tp();
				break;
			}
		}
		break;
	default:	// static
		nvram_set("wan_iface", wan_ifname);
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		
		int r = 10;
		while ((!check_wanup()) && (r-- > 0)) {
			sleep(1);
		}
		
		start_wan_done(wan_ifname);
		break;
	}

	// Get current WAN hardware address
	strlcpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	if (ioctl(sd, SIOCGIFHWADDR, &ifr) == 0) {
		nvram_set("wan_hwaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, buf));
	}

	/* Set initial QoS mode again now that WAN port is ready. */
	set_et_qos_mode(sd);

	close(sd);

	enable_ip_forward();

	led(LED_DIAG, 0);	// for 4712, 5325E (?)
	led(LED_DMZ, nvram_match("dmz_enable", "1"));

	TRACE_PT("end\n");
}

#ifdef TCONFIG_IPV6
void start_wan6_done(char *wan_ifname)
{
	switch (get_ipv6_service()) {
	case IPV6_NATIVE:
		eval("ip", "route", "add", "::/0", "dev", wan_ifname);
		break;
	case IPV6_NATIVE_DHCP:
		eval("ip", "route", "add", "::/0", "dev", wan_ifname);
		stop_dhcp6c();
		stop_radvd();
		/* note: starting radvd here is really too early because we won't have
		 * received a prefix and so it will disable advertisements,
		 * but the SIGHUP sent from dhcp6c-state will restart them.
		 */
		start_radvd();
		start_dhcp6c();
		break;
	case IPV6_6IN4:
		stop_ipv6_sit_tunnel();
		start_ipv6_sit_tunnel();
		break;
	}
}
#endif

//	ppp_demand: 0=keep alive, 1=connect on demand (run 'listen')
//	wan_ifname: vlan1
//	wan_iface:	ppp# (PPPOE, PPTP, L2TP), vlan1 (DHCP, HB, Static)

void start_wan_done(char *wan_ifname)
{
	int proto;
	int n;
	char *gw;
	int dod;
	struct sysinfo si;
	int wanup;
	int metric;
		
	TRACE_PT("begin wan_ifname=%s\n", wan_ifname);
	
	sysinfo(&si);
	f_write("/var/lib/misc/wantime", &si.uptime, sizeof(si.uptime), 0, 0);
	
	proto = get_wan_proto();
	dod = nvram_get_int("ppp_demand");

#if 0
	if (using_dhcpc()) {
		while (route_del(nvram_safe_get("wan_ifname"), 0, NULL, NULL, NULL) == 0) {
			//
		}
	}
#endif

	// delete all default routes
	while (route_del(wan_ifname, 0, NULL, NULL, NULL) == 0) {
		//
	}

	if (proto != WP_DISABLED) {
		// set default route to gateway if specified
		gw = wan_gateway();
		if (proto == WP_PPTP && !using_dhcpc()) {
			// For PPTP protocol, we must use ppp_get_ip as gateway, not pptp_server_ip (??)
			if (*gw == 0 || strcmp(gw, "0.0.0.0") == 0) gw = nvram_safe_get("ppp_get_ip");
		}
		if ((*gw != 0) && (strcmp(gw, "0.0.0.0") != 0)) {
			if (proto == WP_DHCP || proto == WP_STATIC) {
				// possibly gateway is over the bridge, try adding a route to gateway first
				route_add(wan_ifname, 0, gw, NULL, "255.255.255.255");
			}

			metric = 0;
			if (proto == WP_PPTP || proto == WP_L2TP) {
				if (nvram_get_int("ppp_defgw") || !using_dhcpc())
					metric = 0;
				else {
					metric = 2;

					// we are not using default gateway on remote network,
					// add route to the vpn subnet
					char *netmask = nvram_safe_get("wan_netmask");
					struct in_addr net, mask;
					if (strcmp(netmask, "0.0.0.0") == 0 || !inet_aton(netmask, &mask)) {
						netmask = "255.255.255.0";
						inet_aton(netmask, &mask);
					}
					if (inet_aton(gw, &net)) {
						net.s_addr &= mask.s_addr;
						route_add(wan_ifname, 0, inet_ntoa(net), gw, netmask);

						// add routes to dns servers
						char word[100], *next;
						foreach(word, nvram_safe_get("wan_get_dns"), next) {
							route_add(wan_ifname, 0, word, gw, "255.255.255.255");
						}
					}
				}
			}

			n = 5;
			while ((route_add(wan_ifname, metric, "0.0.0.0", gw, "0.0.0.0") == 1) && (n--)) {
				sleep(1);
			}
			_dprintf("set default gateway=%s n=%d\n", gw, n);

			// hack: avoid routing cycles, when both peer and server have the same IP
			if (proto == WP_PPTP || proto == WP_L2TP) {
				// delete gateway route as it's no longer needed
				route_del(wan_ifname, 0, gw, "0.0.0.0", "255.255.255.255");
			}
		}

#ifdef THREE_ARP_GRATUATOUS_SUPPORT	// from 43011; checkme; commented-out	-- zzz
/*
		// 43011: Alpha add to send Gratuitous ARP when wan_proto is Static IP 2007-04-09
		if (proto == WP_STATIC)
		{
			int ifindex;
			u_int32_t wan_ip;
			unsigned char wan_mac[6];

			if (read_iface(nvram_safe_get("wan_iface"), &ifindex, &wan_ip, wan_mac) >= 0)
				arpping(wan_ip, wan_ip, wan_mac, nvram_safe_get("wan_iface"));
		}
*/
#endif

		if (proto == WP_PPTP || proto == WP_L2TP) {
			route_del(nvram_safe_get("wan_iface"), 0, nvram_safe_get("wan_gateway_get"), NULL, "255.255.255.255");
			route_add(nvram_safe_get("wan_iface"), 0, nvram_safe_get("ppp_get_ip"), NULL, "255.255.255.255");
		}
		if (proto == WP_L2TP) {
			route_add(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("wan_gateway"), "255.255.255.255"); // fixed routing problem in Israel by kanki
		}
	}

	dns_to_resolv();
	start_dnsmasq();

	start_firewall();
	start_qos();
	new_qoslimit_start();
	new_arpbind_start();

	do_static_routes(1);
	// and routes supplied via DHCP
	do_wan_routes(using_dhcpc() ? nvram_safe_get("wan_ifname") : wan_ifname, 0, 1);

	stop_zebra();
	start_zebra();

	wanup = check_wanup();
	
	if ((wanup) || (time(0) < Y2K)) {
		stop_ntpc();
		start_ntpc();
	}

	if ((wanup) || (proto == WP_DISABLED)) {
		stop_ddns();
		start_ddns();
		stop_igmp_proxy();
		start_igmp_proxy();
	}
	
#ifdef TCONFIG_IPV6
	if (wanup) start_wan6_done(wan_ifname);
#endif

	stop_upnp();
	start_upnp();

	if (wanup) {
		SET_LED(GOT_IP);
		notice_set("wan", "");
		
		run_nvscript("script_wanup", NULL, 0);
	}

	// We don't need STP after wireless led is lighted		//	no idea why... toggling it if necessary	-- zzz
	if (check_hw_type() == HW_BCM4702) {
		eval("brctl", "stp", nvram_safe_get("lan_ifname"), "0");
		if (nvram_match("lan_stp", "1")) eval("brctl", "stp", nvram_safe_get("lan_ifname"), "1");
	}

	if (wanup)
		start_vpn_eas();

	unlink(wan_connecting);

	TRACE_PT("end\n");
}

void stop_wan(void)
{
	char name[80];
	char *next;
	
	TRACE_PT("begin\n");

	new_arpbind_stop();
	new_qoslimit_stop();
	stop_qos();
	stop_upnp();	//!!TB - moved from stop_services()
	stop_firewall();
	stop_igmp_proxy();
	stop_ntpc();

#ifdef TCONFIG_IPV6
	stop_ipv6_sit_tunnel();
	stop_dhcp6c();
	nvram_set("ipv6_get_dns", "");
#endif

	/* Kill any WAN client daemons or callbacks */
	stop_redial();
	stop_pppoe();
	stop_ppp();
	stop_dhcpc();
	clear_resolv();
	nvram_set("wan_get_dns", "");

	/* Bring down WAN interfaces */
	foreach(name, nvram_safe_get("wan_ifnames"), next)
		ifconfig(name, 0, "0.0.0.0", NULL);

	SET_LED(RELEASE_IP);
	//notice_set("wan", "");
	unlink("/var/notice/wan");
	unlink(wan_connecting);

	TRACE_PT("end\n");
}
