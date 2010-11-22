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
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>


static void ctvbuf(FILE *f)
{
	int n;
	struct sysinfo si;
//	meminfo_t mem;

#if 1
	const char *p;

	if ((p = nvram_get("ct_max")) != NULL) {
		n = atoi(p);
		if (n == 0) n = 2048;
		else if (n < 1024) n = 1024;
		else if (n > 10240) n = 10240;
	}
	else {
		n = 2048;
	}
#else
	char s[64];

	if (f_read_string("/proc/sys/net/ipv4/ip_conntrack_max", s, sizeof(s)) > 0) n = atoi(s);
		else n = 1024;
	if (n < 1024) n = 1024;
		else if (n > 10240) n = 10240;
#endif

	n *= 170;	// avg tested

//	get_memory(&mem);
//	if (mem.maxfreeram < (n + (64 * 1024))) n = mem.maxfreeram - (64 * 1024);

	sysinfo(&si);
	if (si.freeram < (n + (64 * 1024))) n = si.freeram - (64 * 1024);

//	cprintf("free: %dK, buffer: %dK\n", si.freeram / 1024, n / 1024);

	if (n > 4096) {
//		n =
		setvbuf(f, NULL, _IOFBF, n);
//		cprintf("setvbuf = %d\n", n);
	}
}

void asp_ctcount(int argc, char **argv)
{
	static const char *states[10] = {
		"NONE", "ESTABLISHED", "SYN_SENT", "SYN_RECV", "FIN_WAIT",
		"TIME_WAIT", "CLOSE", "CLOSE_WAIT", "LAST_ACK", "LISTEN" };
	int count[13];	// tcp(10) + udp(2) + total(1) = 13 / max classes = 10
	FILE *f;
	char s[512];
	char *p;
	char *t;
	int i;
	int n;
	int mode;
	unsigned long rip;
	unsigned long lan;
	unsigned long mask;

	if (argc != 1) return;
	mode = atoi(argv[0]);

	memset(count, 0, sizeof(count));

#ifdef TCONFIG_IPV6
	if ((f = fopen("/proc/net/nf_conntrack", "r")) != NULL) {
#else
	if ((f = fopen("/proc/net/ip_conntrack", "r")) != NULL) {
#endif
		ctvbuf(f);	// if possible, read in one go

		if (nvram_match("t_hidelr", "1")) {
			mask = inet_addr(nvram_safe_get("lan_netmask"));
			rip = inet_addr(nvram_safe_get("lan_ipaddr"));
			lan = rip & mask;
		}
		else {
			rip = lan = mask = 0;
		}

		while (fgets(s, sizeof(s), f)) {
#ifdef TCONFIG_IPV6
			if (strncmp(s, "ipv4", 4) == 0) {
				t = s + 11;
#else
				t = s;
#endif
				if (rip != 0) {

					// src=x.x.x.x dst=x.x.x.x	// DIR_ORIGINAL
					if ((p = strstr(t + 14, "src=")) == NULL) continue;
					if ((inet_addr(p + 4) & mask) == lan) {
						if ((p = strstr(p + 13, "dst=")) == NULL) continue;
						if (inet_addr(p + 4) == rip) continue;
					}
				}
#ifdef TCONFIG_IPV6
			}
			else if (strncmp(s, "ipv6", 4) == 0) {
				t = s + 12;
			}
			else {
				continue; // another proto family?!
			}
#endif

			if (mode == 0) {
				// count connections per state
				if (strncmp(t, "tcp", 3) == 0) {
					for (i = 9; i >= 0; --i) {
						if (strstr(s, states[i]) != NULL) {
							count[i]++;
							break;
						}
					}
				}
				else if (strncmp(t, "udp", 3) == 0) {
					if (strstr(s, "[ASSURED]") != NULL) {
						count[11]++;
					}
					else {
						count[10]++;
					}
				}
				count[12]++;
			}
			else {
				// count connections per mark
				if ((p = strstr(s, " mark=")) != NULL) {
					n = atoi(p + 6) & 0xFF;
					if (n <= 10) count[n]++;
				}
			}
		}

		fclose(f);
	}

	if (mode == 0) {
		p = s;
		for (i = 0; i < 12; ++i) {
			p += sprintf(p, ",%d", count[i]);
		}
		web_printf("\nconntrack = [%d%s];\n", count[12], s);
	}
	else {
		p = s;
		for (i = 1; i < 11; ++i) {
			p += sprintf(p, ",%d", count[i]);
		}
		web_printf("\nnfmarks = [%d%s];\n", count[0], s);
	}
}

void asp_ctdump(int argc, char **argv)
{
	FILE *f;
	char s[512];
	char *p, *q;
	int mark;
	int findmark;
	unsigned int proto;
	unsigned int time;
	unsigned int family;
	char src[INET6_ADDRSTRLEN];
	char dst[INET6_ADDRSTRLEN];
	char sport[16];
	char dport[16];
	char byteso[16];
	char bytesi[16];
	int dir_reply;
#ifdef TCONFIG_IPV6
	struct in6_addr rip6;
	struct in6_addr lan6;
	int lan6_prefix_bytes;
	int lan6_prefix_bits;
	struct in6_addr in6;
#endif
	unsigned long rip;
	unsigned long lan;
	unsigned long mask;
	char comma;

	if (argc != 1) return;

	findmark = atoi(argv[0]);

	mask = inet_addr(nvram_safe_get("lan_netmask"));
	rip = inet_addr(nvram_safe_get("lan_ipaddr"));
	lan = rip & mask;
	
#ifdef TCONFIG_IPV6
	if (!nvram_match("ipv6_enable", "0")) {
		inet_pton(AF_INET6, nvram_safe_get("ipv6_rtr_addr"), &rip6);
		inet_pton(AF_INET6, nvram_safe_get("ipv6_prefix"), &lan6);
		int lan6_prefix_len = nvram_get_int("ipv6_prefix_length");
		lan6_prefix_bytes = lan6_prefix_len / 8;
		lan6_prefix_bits  = lan6_prefix_len % 8;
	}
#endif
	
	if (nvram_match("t_hidelr", "0")) rip = 0;	// hide lan -> router?

/*

/proc/net/nf_conntrack prefix (compared to ip_conntrack):
"ipvx" + 5 spaces + "2" or "10" + 1 space

add bytes out/in to table

*/

	web_puts("\nctdump = [");
	comma = ' ';
#ifdef TCONFIG_IPV6
	if ((f = fopen("/proc/net/nf_conntrack", "r")) != NULL) {
#else
	if ((f = fopen("/proc/net/ip_conntrack", "r")) != NULL) {
#endif
		ctvbuf(f);
		while (fgets(s, sizeof(s), f)) {
			dir_reply = 0;
			if ((p = strstr(s, " mark=")) == NULL) continue;
			if ((mark = (atoi(p + 6) & 0xFF)) > 10) mark = 0;
			if ((findmark != -1) && (mark != findmark)) continue;
#ifdef TCONFIG_IPV6
			if (sscanf(s, "%*s %u %*s %u %u", &family, &proto, &time) != 3) continue;
			if ((p = strstr(s + 25, "src=")) == NULL) continue;		// DIR_ORIGINAL
#else
			if (sscanf(s, "%*s %u %u", &proto, &time) != 2) continue;
			if ((p = strstr(s + 14, "src=")) == NULL) continue;		// DIR_ORIGINAL
#endif

#ifdef TCONFIG_IPV6
			switch (family) {
			case 2:
#endif
				if ((inet_addr(p + 4) & mask) != lan) {
					dir_reply = 1;
				}
				else if (rip != 0) {
					if ((q = strstr(p + 13, "dst=")) == NULL) continue;
//					cprintf("%lx=%lx\n", inet_addr(q + 4), rip);
					if (inet_addr(q + 4) == rip) continue;
				}
#ifdef TCONFIG_IPV6
				break;
			case 10:
				if (sscanf(p + 4, "%s ", src) != 1) continue;
				if (inet_pton(AF_INET6, src, &in6) <= 0) continue;
				if (rip != 0 && (IN6_ARE_ADDR_EQUAL(&rip6, &in6))) continue;
				else if (memcmp(&lan6.s6_addr[0], &in6.s6_addr[0], lan6_prefix_bytes) != 0 || ( lan6_prefix_bits && 
					(lan6.s6_addr[lan6_prefix_bytes] ^ in6.s6_addr[lan6_prefix_bytes]) >> (8-lan6_prefix_bits) )) {
					dir_reply = 1;
				}
				break;
			}
#endif
			if (dir_reply) {
				// make sure we're seeing int---ext if possible
				if ((q = strstr(p + 13, "bytes=")) == NULL) continue;
				if (sscanf(q + 6, "%s", bytesi) != 1) continue;
				if ((p = strstr(p + 41, "src=")) == NULL) continue;
			}
			else {
				if ((q = strstr(p + 41, "src=")) == NULL) continue;
				if ((q = strstr(q + 13, "bytes=")) == NULL) continue;
				if (sscanf(q + 6, "%s", bytesi) != 1) continue;
			}


			if ((proto == 6) || (proto == 17)) {
				if (sscanf(p + 4, "%s dst=%s sport=%s dport=%s %*s bytes=%s", src, dst, sport, dport, byteso) != 5) continue;
			}
			else {
				if (sscanf(p + 4, "%s dst=%s", src, dst) != 2) continue;
				if ((q = strstr(p + 13, "bytes=")) == NULL) continue;
				if (sscanf(q + 6, "%s", byteso) != 1) continue;
				sport[0] = 0;
				dport[0] = 0;
			}

			web_printf("%c[%u,%u,'%s','%s','%s','%s','%s','%s',%d]", comma, proto, time, src, dst, sport, dport, byteso, bytesi, mark );
			comma = ',';
		}
	}
	web_puts("];\n");
}

void asp_qrate(int argc, char **argv)
{
	FILE *f;
	char s[256];
	unsigned long rates[10];
	unsigned long u;
	char *e;
	int n;
	char comma;
	char *a[1];

	a[0] = "1";
	asp_ctcount(1, a);

	memset(rates, 0, sizeof(rates));
	sprintf(s, "tc -s class ls dev %s", nvram_safe_get("wan_iface"));
	if ((f = popen(s, "r")) != NULL) {
		n = 1;
		while (fgets(s, sizeof(s), f)) {
			if (strncmp(s, "class htb 1:", 12) == 0) {
				n = atoi(s + 12);
			}
			else if (strncmp(s, " rate ", 6) == 0) {
				if ((n % 10) == 0) {
					n /= 10;
					if ((n >= 1) && (n <= 10)) {
						u = strtoul(s + 6, &e, 10);
						if (*e == 'K') u *= 1000;
							else if (*e == 'M') u *= 1000 * 1000;
						rates[n - 1] = u;
						n = 1;
					}
				}
			}
		}
		pclose(f);
	}

	comma = ' ';
	web_puts("\nqrates = [0,");
	for (n = 0; n < 10; ++n) {
		web_printf("%c%lu", comma, rates[n]);
		comma = ',';
	}
	web_puts("];");
}

static void layer7_list(const char *path, int *first)
{
	DIR *dir;
	struct dirent *de;
	char *p;
	char name[NAME_MAX];

	if ((dir = opendir(path)) != NULL) {
		while ((de = readdir(dir)) != NULL) {
			strlcpy(name, de->d_name, sizeof(name));
			if ((p = strstr(name, ".pat")) == NULL) continue;
			*p = 0;
			web_printf("%s'%s'", *first ? "" : ",", name);
			*first = 0;
		}
		closedir(dir);
	}
}

void asp_layer7(int argc, char **argv)
{
	int first = 1;
	web_puts("\nlayer7 = [");
	layer7_list("/etc/l7-extra", &first);
	layer7_list("/etc/l7-protocols", &first);
	web_puts("];\n");
}

void wo_expct(char *url)
{
	f_write_string("/proc/net/expire_early", "15", 0, 0);
}
