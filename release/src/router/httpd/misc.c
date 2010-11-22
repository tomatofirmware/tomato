/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "tomato.h"

#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/statfs.h>
#include <netdb.h>
#include <net/route.h>

#include <wlioctl.h>
#include <wlutils.h>

// to javascript-safe string
char *js_string(const char *s)
{
	unsigned char c;
	char *buffer;
	char *b;

	if ((buffer = malloc((strlen(s) * 4) + 1)) != NULL) {
		b = buffer;
		while ((c = *s++) != 0) {
			if ((c == '"') || (c == '\'') || (c == '\\') || (!isprint(c))) {
				b += sprintf(b, "\\x%02x", c);
			}
			else {
				*b++ = c;
			}
		}
		*b = 0;
	}
	return buffer;
}

// to html-safe string
char *html_string(const char *s)
{
	unsigned char c;
	char *buffer;
	char *b;

	if ((buffer = malloc((strlen(s) * 6) + 1)) != NULL) {
		b = buffer;
		while ((c = *s++) != 0) {
			if ((c == '&') || (c == '<') || (c == '>') || (c == '"') || (c == '\'') || (!isprint(c))) {
				b += sprintf(b, "&#%d;", c);
			}
			else {
				*b++ = c;
			}
		}
		*b = 0;
	}
	return buffer;
}

// removes \r
char *unix_string(const char *s)
{
	char *buffer;
	char *b;
	char c;

	if ((buffer = malloc(strlen(s) + 1)) != NULL) {
		b = buffer;
		while ((c = *s++) != 0)
			if (c != '\r') *b++ = c;
		*b = 0;
	}
	return buffer;
}

// # days, ##:##:##
char *reltime(char *buf, time_t t)
{
	int days;
	int m;

	if (t < 0) t = 0;
	days = t / 86400;
	m = t / 60;
	sprintf(buf, "%d day%s, %02d:%02d:%02d", days, ((days==1) ? "" : "s"), ((m / 60) % 24), (m % 60), (int)(t % 60));
	return buf;
}

int get_client_info(char *mac, char *ifname)
{
	FILE *f;
	char s[256];
#ifdef TCONFIG_IPV6
	char ip[INET6_ADDRSTRLEN];
	int junk;
#else
	char ip[INET_ADDRSTRLEN];
#endif

/*
# ip neigh show fe80:0:0::201:02ff:fe03:0405
fe80::201:2ff:fe3:405 dev br0 lladdr 00:01:02:03:04:05 REACHABLE
*/
	if (clientsai.ss_family == AF_INET) {
		inet_ntop(clientsai.ss_family, &(((struct sockaddr_in*)&clientsai)->sin_addr), ip, sizeof(ip));
		sprintf(s, "ip neigh show %s", ip);
	}
#ifdef TCONFIG_IPV6
	else if (clientsai.ss_family == AF_INET6) {
		inet_ntop(clientsai.ss_family, &(((struct sockaddr_in6*)&clientsai)->sin6_addr), ip, sizeof(ip));
		if (IN6_IS_ADDR_V4MAPPED( &(((struct sockaddr_in6*)&clientsai)->sin6_addr) ))
			sprintf(s, "ip neigh show %s", ip + 7); // chop off the ::ffff: to get the ipv4 bit
		else
			sprintf(s, "ip neigh show %s", ip);
	}
#endif

	if ((f = popen(s, "r")) != NULL) {
		while (fgets(s, sizeof(s), f)) {
			if (sscanf(s, "%*s dev %16s lladdr %17s %*s", ifname, mac) == 2) {
				pclose(f);
				return 1;
			}
		}
		pclose(f);
	}
	return 0;
}


//	<% lanip(mode); %>
//	<mode>
//		1		return first 3 octets (192.168.1)
//		2		return last octet (1)
//		else	return full (192.168.1.1)

void asp_lanip(int argc, char **argv)
{
	char *nv, *p;
	char s[64];
	char mode;

	mode = argc ? *argv[0] : 0;

	if ((nv = nvram_get("lan_ipaddr")) != NULL) {
		strcpy(s, nv);
		if ((p = strrchr(s, '.')) != NULL) {
			*p = 0;
            web_puts((mode == '1') ? s : (mode == '2') ? (p + 1) : nv);
		}
	}
}

void asp_lipp(int argc, char **argv)
{
	char *one = "1";
	asp_lanip(1, &one);
}

//	<% psup(process); %>
//	returns 1 if process is running

void asp_psup(int argc, char **argv)
{
	if (argc == 1) web_printf("%d", pidof(argv[0]) > 0);
}

void wo_vpn_status(char *url)
{
#ifdef TCONFIG_OPENVPN
	char buf[256];
	char *type;
	char *str;
	int num;
	FILE *fp;

	type = 0;
	if ( (str = webcgi_get("server")) )
		type = "server";
	else if ( (str = webcgi_get("client")) )
		type = "client";

	num = str? atoi(str): 0;
	if ( type && num > 0 )
	{
		// Trigger OpenVPN to update the status file
		snprintf(&buf[0], sizeof(buf), "vpn%s%d", type, num);
		killall(&buf[0], SIGUSR2);

		// Give it a chance to update the file
		sleep(1);

		// Read the status file and repeat it verbatim to the caller
		snprintf(&buf[0], sizeof(buf), "/etc/openvpn/%s%d/status", type, num);
		fp = fopen(&buf[0], "r");
		if( fp != NULL )
		{
			while (fgets(&buf[0], sizeof(buf), fp) != NULL)
				web_puts(&buf[0]);
			fclose(fp);
		}
	}
#endif
}

/*
# cat /proc/meminfo
        total:    used:    free:  shared: buffers:  cached:
Mem:  14872576 12877824  1994752        0  1236992  4837376
Swap:        0        0        0
MemTotal:        14524 kB
MemFree:          1948 kB
MemShared:           0 kB
Buffers:          1208 kB
Cached:           4724 kB
SwapCached:          0 kB
Active:           4364 kB
Inactive:         2952 kB
HighTotal:           0 kB
HighFree:            0 kB
LowTotal:        14524 kB
LowFree:          1948 kB
SwapTotal:           0 kB
SwapFree:            0 kB

*/

typedef struct {
	unsigned long total;
	unsigned long free;
	unsigned long shared;
	unsigned long buffers;
	unsigned long cached;
	unsigned long swaptotal;
	unsigned long swapfree;
	unsigned long maxfreeram;
} meminfo_t;

static int get_memory(meminfo_t *m)
{
	FILE *f;
	char s[128];
	int ok = 0;

	memset(m, 0, sizeof(*m));
	if ((f = fopen("/proc/meminfo", "r")) != NULL) {
		while (fgets(s, sizeof(s), f)) {
#ifdef LINUX26
			if (strncmp(s, "MemTotal:", 9) == 0) {
				m->total = strtoul(s + 12, NULL, 10) * 1024;
				++ok;
			}
			else if (strncmp(s, "MemFree:", 8) == 0) {
				m->free = strtoul(s + 12, NULL, 10) * 1024;
				++ok;
			}
			else if (strncmp(s, "Buffers:", 8) == 0) {
				m->buffers = strtoul(s + 12, NULL, 10) * 1024;
				++ok;
			}
			else if (strncmp(s, "Cached:", 7) == 0) {
				m->cached = strtoul(s + 12, NULL, 10) * 1024;
				++ok;
			}
#else
			if (strncmp(s, "Mem:", 4) == 0) {
				if (sscanf(s + 6, "%ld %*d %ld %ld %ld %ld", &m->total, &m->free, &m->shared, &m->buffers, &m->cached) == 5)
					++ok;
			}
#endif
			else if (strncmp(s, "SwapTotal:", 10) == 0) {
				m->swaptotal = strtoul(s + 12, NULL, 10) * 1024;
				++ok;
			}
			else if (strncmp(s, "SwapFree:", 9) == 0) {
				m->swapfree = strtoul(s + 11, NULL, 10) * 1024;
				++ok;
#ifndef LINUX26
				break;
#endif
			}
		}
		fclose(f);
	}
	if (ok == 0) {
		return 0;
	}
	m->maxfreeram = m->free;
	if (nvram_match("t_cafree", "1")) m->maxfreeram += (m->cached + m->buffers);
	return 1;
}

void asp_sysinfo(int argc, char **argv)
{
	struct sysinfo si;
	char s[64];
	meminfo_t mem;

	web_puts("\nsysinfo = {\n");
	sysinfo(&si);
	get_memory(&mem);
	web_printf(
		"\tuptime: %ld,\n"
		"\tuptime_s: '%s',\n"
		"\tloads: [%ld, %ld, %ld],\n"
		"\ttotalram: %ld,\n"
		"\tfreeram: %ld,\n"
		"\tshareram: %ld,\n"
		"\tbufferram: %ld,\n"
		"\tcached: %ld,\n"
		"\ttotalswap: %ld,\n"
		"\tfreeswap: %ld,\n"
		"\ttotalfreeram: %ld,\n"
		"\tprocs: %d\n",
			si.uptime,
			reltime(s, si.uptime),
			si.loads[0], si.loads[1], si.loads[2],
			mem.total, mem.free,
			mem.shared, mem.buffers, mem.cached,
			mem.swaptotal, mem.swapfree,
			mem.maxfreeram,
			si.procs);
	web_puts("};\n");
}

void asp_activeroutes(int argc, char **argv)
{
	FILE *f;
	char s[512];
	char dev[17];
	unsigned long dest;
	unsigned long gateway;
	unsigned long flags;
	unsigned long mask;
	unsigned metric;
	struct in_addr ia;
	char s_dest[16];
	char s_gateway[16];
	char s_mask[16];
	int n;

	web_puts("\nactiveroutes = [");
	n = 0;
	if ((f = fopen("/proc/net/route", "r")) != NULL) {
		while (fgets(s, sizeof(s), f)) {
			if (sscanf(s, "%16s%lx%lx%lx%*s%*s%u%lx", dev, &dest, &gateway, &flags, &metric, &mask) != 6) continue;
			if ((flags & RTF_UP) == 0) continue;
			if (dest != 0) {
				ia.s_addr = dest;
				strcpy(s_dest, inet_ntoa(ia));
			}
			else {
				strcpy(s_dest, "default");
			}
			if (gateway != 0) {
				ia.s_addr = gateway;
				strcpy(s_gateway, inet_ntoa(ia));
			}
			else {
				strcpy(s_gateway, "*");
			}
			ia.s_addr = mask;
			strcpy(s_mask, inet_ntoa(ia));
			web_printf("%s['%s','%s','%s','%s',%u]", n ? "," : "", dev, s_dest, s_gateway, s_mask, metric);
			++n;
		}
		fclose(f);
	}
	web_puts("];\n");
}

void asp_cgi_get(int argc, char **argv)
{
	const char *v;
	int i;

	for (i = 0; i < argc; ++i) {
		v = webcgi_get(argv[i]);
		if (v) web_puts(v);
	}
}

void asp_time(int argc, char **argv)
{
	time_t t;
	char s[64];

	t = time(NULL);
	if (t < Y2K) {
		web_puts("Not Available");
	}
	else {
		strftime(s, sizeof(s), "%a, %d %b %Y %H:%M:%S %z", localtime(&t));
		web_puts(s);
	}
}

void asp_wanup(int argc, char **argv)
{
	web_puts(check_wanup() ? "1" : "0");
}

void asp_wanstatus(int argc, char **argv)
{
	const char *p;

	if ((using_dhcpc()) && (f_exists("/var/lib/misc/dhcpc.renewing"))) {
		p = "Renewing...";
	}
	else if (check_wanup()) {
		p = "Connected";
	}
	else if (f_exists("/var/lib/misc/wan.connecting")) {
		p = "Connecting...";
	}
	else {
		p = "Disconnected";
	}
	web_puts(p);
}

void asp_link_uptime(int argc, char **argv)
{
	struct sysinfo si;
	char buf[64];
	long uptime;

	buf[0] = '-';
	buf[1] = 0;
	if (check_wanup()) {
		sysinfo(&si);
		if (f_read("/var/lib/misc/wantime", &uptime, sizeof(uptime)) == sizeof(uptime)) {
			reltime(buf, si.uptime - uptime);
		}
	}
	web_puts(buf);
}

void asp_rrule(int argc, char **argv)
{
	char s[32];
	int i;

	i = nvram_get_int("rruleN");
	sprintf(s, "rrule%d", i);
	web_puts("\nrrule = '");
	web_putj(nvram_safe_get(s));
	web_printf("';\nrruleN = %d;\n", i);
}

void asp_compmac(int argc, char **argv)
{
	char mac[32];
	char ifname[32];

	if (get_client_info(mac, ifname)) {
		web_puts(mac);
	}
}

void asp_ident(int argc, char **argv)
{
	web_puth(nvram_safe_get("router_name"));
}

void asp_statfs(int argc, char **argv)
{
	struct statfs sf;
	int mnt;

	if (argc != 2) return;

	// used for /cifs/, /jffs/... if it returns squashfs type, assume it's not mounted
	if ((statfs(argv[0], &sf) != 0) || (sf.f_type == 0x73717368)) {
		mnt = 0;
		memset(&sf, 0, sizeof(sf));
#ifdef TCONFIG_JFFS2
		// for jffs, try to get total size from mtd partition
		if (strncmp(argv[1], "jffs", 4) == 0) {
			int part;

			if (mtd_getinfo(argv[1], &part, (int *)&sf.f_blocks)) {
				sf.f_bsize = 1;
			}
		}
#endif
	}
	else {
		mnt = 1;
	}

	web_printf(
			"\n%s = {\n"
			"\tmnt: %d,\n"
			"\tsize: %llu,\n"
			"\tfree: %llu\n"
			"};\n",
			argv[1], mnt,
			((uint64_t)sf.f_bsize * sf.f_blocks),
			((uint64_t)sf.f_bsize * sf.f_bfree));
}

void asp_notice(int argc, char **argv)
{
	char s[256];
	char buf[2048];

	if (argc != 1) return;
	snprintf(s, sizeof(s), "/var/notice/%s", argv[0]);
	if (f_read_string(s, buf, sizeof(buf)) <= 0) return;
	web_putj(buf);
}

void wo_wakeup(char *url)
{
	char *mac;
	char *p;
	char *end;

	if ((mac = webcgi_get("mac")) != NULL) {
		end = mac + strlen(mac);
		while (mac < end) {
			while ((*mac == ' ') || (*mac == '\t') || (*mac == '\r') || (*mac == '\n')) ++mac;
			if (*mac == 0) break;

			p = mac;
			while ((*p != 0) && (*p != ' ') && (*p != '\r') && (*p != '\n')) ++p;
			*p = 0;

			eval("ether-wake", "-b", "-i", nvram_safe_get("lan_ifname"), mac);
			mac = p + 1;
		}
	}
	common_redirect();
}

void asp_dns(int argc, char **argv)
{
	char s[128];
	int i;
	const dns_list_t *dns;

	dns = get_dns();	// static buffer
	strcpy(s, "\ndns = [");
	for (i = 0 ; i < dns->count; ++i) {
		sprintf(s + strlen(s), "%s'%s:%u'", i ? "," : "", inet_ntoa(dns->dns[i].addr), dns->dns[i].port);
	}
	strcat(s, "];\n");
	web_puts(s);
}

void wo_resolve(char *url)
{
	char *p;
	char *ip;
	
	char host[NI_MAXHOST];
	struct addrinfo hints;
	struct addrinfo *res;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	char comma;
	char *js;

	comma = ' ';
	web_puts("\nresolve_data = [\n");
	if ((p = webcgi_get("ip")) != NULL) {
		while ((ip = strsep(&p, ",")) != NULL) {
			if (getaddrinfo(ip, NULL, &hints, &res) != 0) {
				//error
				continue;
			}
			if (getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), NULL, 0, 0) != 0) continue;
			js = js_string(host);
			web_printf("%c['%s','%s']", comma, ip, js);
			free(js);
			freeaddrinfo(res);
			comma = ',';
		}
	}
	web_puts("];\n");
}
