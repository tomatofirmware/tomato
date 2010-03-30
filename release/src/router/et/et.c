/*
 * et driver ioctl swiss army knife command.
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: et.c,v 1.1.1.1 2007/03/20 12:22:00 roly Exp $
 */

#include <stdio.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <typedefs.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <etioctl.h>
#include <proto/ethernet.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>

static void usage(char *av0);
static void syserr(char *s);
static void et_find(int s, struct ifreq *ifr);
static int et_check(int s, struct ifreq *ifr);



char buf[16 * 1024];

#define VECLEN		2

static int optind;

int
main(int ac, char *av[])
{
	char *interface = NULL;
	struct ifreq ifr;
	char *endptr;
	int arg;
	int vecarg[VECLEN];
	int s;

	if (ac < 2)
		usage(av[0]);

	optind = 1;

	if (av[1][0] == '-') {
		if ((av[1][1] != 'a') && (av[1][1] != 'i'))
			usage(av[0]);
		if (ac < 4)
			usage(av[0]);
		interface = av[2];
		optind += 2;
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	if (interface)
		strncpy(ifr.ifr_name, interface, sizeof (ifr.ifr_name));
	else
		et_find(s, &ifr);

	if (!*ifr.ifr_name) {
		fprintf(stderr, "et interface not found\n");
		exit(1);
	}

	if (strcmp(av[optind], "up") == 0) {
		if (ioctl(s, SIOCSETCUP, (caddr_t)&ifr) < 0)
			syserr("etcup");
	} else if (strcmp(av[optind], "down") == 0) {
		if (ioctl(s, SIOCSETCDOWN, (caddr_t)&ifr) < 0)
			syserr("etcdown");
	} else if (strcmp(av[optind], "loop") == 0) {
		if (optind >= (ac -1))
			usage(av[0]);
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCLOOP, (caddr_t)&ifr) < 0)
			syserr("etcloop");
	} else if (strcmp(av[optind], "dump") == 0) {
		ifr.ifr_data = buf;
		if (ioctl(s, SIOCGETCDUMP, (caddr_t)&ifr) < 0)
			syserr("etcdump");
		printf("%s\n", buf);
	} else if (strcmp(av[optind], "msglevel") == 0) {
		if (optind >= (ac -1))
			usage(av[0]);
		arg = strtol(av[optind + 1], &endptr, 0);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCSETMSGLEVEL, (caddr_t)&ifr) < 0)
			syserr("etcsetmsglevel");
	} else if (strcmp(av[optind], "promisc") == 0) {
		if (optind >= (ac -1))
			usage(av[0]);
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCPROMISC, (caddr_t)&ifr) < 0)
			syserr("etcpromisc");
	} else if (strcmp(av[optind], "qos") == 0) {
		if (optind >= (ac -1))
			usage(av[0]);
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCQOS, (caddr_t)&ifr) < 0)
			syserr("etcqos");
	} else if (strcmp(av[optind], "txdown") == 0) {
		if (ioctl(s, SIOCSETCTXDOWN, (caddr_t)&ifr) < 0)
			syserr("etctxdown");
	} else if (strcmp(av[optind], "speed") == 0) {
		if (optind >= (ac -1))
			usage(av[0]);
		if (strcmp(av[optind+1], "auto") == 0)
			arg = -1;
		else if (strcmp(av[optind+1], "10half") == 0)
			arg = 0;
		else if (strcmp(av[optind+1], "10full") == 0)
			arg = 1;
		else if (strcmp(av[optind+1], "100half") == 0)
			arg = 2;
		else if (strcmp(av[optind+1], "100full") == 0)
			arg = 3;
		else
			usage(av[0]);

		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCSPEED, (caddr_t)&ifr) < 0)
			syserr("etcspeed");
	}
	else if (strcmp(av[optind], "phyrd") == 0) {
		int cmd = -1;

		if ((ac < (optind + 2)) || (ac > (optind + 3))) {
			usage(av[0]);
		} else if (ac == (optind + 3)) {
			/* PHY address provided */
			vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
			vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
			cmd = SIOCGETCPHYRD2;
		} else {
			/* "My" PHY address implied */
			vecarg[0] = strtoul(av[optind + 1], NULL, 0);
			cmd = SIOCGETCPHYRD;
		}
		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, cmd, (caddr_t)&ifr) < 0)
			syserr("etcphyrd");

		printf("0x%04x\n", vecarg[1]);
	} else if (strcmp(av[optind], "phywr") == 0) {
		int cmd = -1;

		if ((ac < (optind + 3)) || (ac > (optind + 4))) {
			usage(av[0]);
		} else if (ac == (optind + 4)) {
			vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
			vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
			vecarg[1] = strtoul(av[optind + 3], NULL, 0);
			cmd = SIOCSETCPHYWR2;
		} else {
			vecarg[0] = strtoul(av[optind + 1], NULL, 0);
			vecarg[1] = strtoul(av[optind + 2], NULL, 0);
			cmd = SIOCSETCPHYWR;
		}
		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, cmd, (caddr_t)&ifr) < 0)
			syserr("etcphywr");
	} else if (strcmp(av[optind], "robord") == 0) {
		if (ac != (optind + 3))
			usage(av[0]);

		vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
		vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;

		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, SIOCGETCROBORD, (caddr_t)&ifr) < 0)
			syserr("etcrobord");

		printf("0x%04x\n", vecarg[1]);
	} else if (strcmp(av[optind], "robowr") == 0) {
		if (ac != (optind + 4))
			usage(av[0]);

		vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
		vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
		vecarg[1] = strtoul(av[optind + 3], NULL, 0);

		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0)
			syserr("etcrobowr");
	} else
		usage(av[0]);

	return (0);
}

static void
usage(char *av0)
{
	fprintf(stderr, "usage: %s [ [ -a | -i ] interface ] and one of:\n"
		"\tup\n"
		"\tdown\n"
		"\tloop <0 or 1>\n"
		"\tdump\n"
		"\tmsglevel <bitvec> (error=1, trace=2, prhdr=4, prpkt=8)\n"
		"\tpromisc <0 or 1>\n"
		"\tqos <0 or 1>\n"
		"\ttxdown\n"
		"\tspeed <auto, 10half, 10full, 100half, 100full>\n"
		"\tphyrd [<phyaddr>] <reg>\n"
		"\tphywr [<phyaddr>] <reg> <val>\n"
		"\trobord <page> <reg>\n"
		"\trobowr <page> <reg> <val>\n"
		,
		av0);
	exit(1);
}

static void
et_find(int s, struct ifreq *ifr)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[512], *c, *name;

	ifr->ifr_name[0] = '\0';

	/* eat first two lines */
        if (!(fp = fopen(proc_net_dev, "r")) ||
            !fgets(buf, sizeof(buf), fp) ||
            !fgets(buf, sizeof(buf), fp))
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (et_check(s, ifr) == 0)
			break;
		ifr->ifr_name[0] = '\0';
	}

	fclose(fp);
}

static int
et_check(int s, struct ifreq *ifr)
{
	struct ethtool_drvinfo info;

	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr->ifr_data = (caddr_t)&info;
	if (ioctl(s, SIOCETHTOOL, (caddr_t)ifr) < 0) {
		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("siocethtool");
		return (-1);
	}

	if (!strncmp(info.driver, "et", 2))
		return (0);
	else if (!strncmp(info.driver, "bcm57", 5))
		return (0);

	return (-1);
}

static void
syserr(char *s)
{
	perror(s);
	exit(1);
}

