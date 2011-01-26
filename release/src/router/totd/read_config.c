/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: read_config.c,v 3.38 2005/01/29 18:51:36 dillema Exp $>
 */

#include "totd.h"

int read_config (char *config_file) {
	char buf[512];
	char *cp;
	FILE *config_fp;
	char *args[MAXARGS];
	int argcnt, errcnt, linenum;

	if (T.debug > 0)
		syslog (LOG_DEBUG, "config file: %s", config_file);

	config_fp = fopen (config_file, "r");
	if (!config_fp) {
		syslog (LOG_ERR, "can't open config file: %s", config_file);
		return -1;
	}
	linenum = errcnt = 0;
	while (fgets (buf, sizeof (buf), config_fp)) {
		linenum++;

		cp = strchr (buf, '\n');
		if (cp)
			*cp = '\0';
		cp = strchr (buf, '\n');
		if (cp)
			*cp = '\0';
		cp = strchr (buf, '#'); /* strip comment */
		if (cp)
			*cp = '\0';
		cp = strchr (buf, ';'); /* strip comment */
		if (cp)
			*cp = '\0';

		/* parse config line in words */
		args[argcnt = 0] = strtok (buf, " \t");
		while (args[argcnt] && ++argcnt < MAXARGS)
			args[argcnt] = strtok (NULL, " \t");

		if (!args[0]) /* empty line (or only comment) */
			continue;

		if (!strcasecmp (args[0], "forwarder")) {
			if (argcnt < 2 || argcnt > 4 ) {
				syslog (LOG_ERR, "line %d: invalid format: forwarder <IPaddr> [port <port>]", linenum);
				errcnt++;
			} else {
				int port = PORT_TO, i;

				i = 2;
				if (argcnt > i) {
					if (!strcasecmp (args[i], "port")) {
						if (argcnt >= ++i) {
							port = atoi (args[i++]);
						} else {
							syslog (LOG_ERR, "line %d: invalid format, missing <port> after port attribute", linenum);
							errcnt++;
							continue;
						}
					} else {
						syslog (LOG_ERR, "line %d: invalid format, cannot parse unknown attribute: %s", linenum, args[i]);
						errcnt++;
						continue;
					}	
				}
				if (fwd_add (args[1], port) == -1)
					return -1; /* serious trouble */
			}
		} else if (!strcasecmp (args[0], "prefix")) {
			if (argcnt != 2) {
				syslog (LOG_ERR, "line %d: invalid format: prefix <IPv6prefix>", linenum);
				errcnt++;
			} else {
				if (conv_trick_conf (args[1])) {
					syslog (LOG_INFO, "can not add prefix %d: %s", T.prefixnum, args[1]);
					errcnt++;
				} else
					syslog (LOG_INFO, "prefix %d added: %s", T.prefixnum, args[1]);
			}
		} else if (!strcasecmp (args[0], "allow")) {
			if (argcnt != 2) {
				syslog (LOG_ERR, "line %d: invalid format: allow <IP address>", linenum);
				errcnt++;
			} else {
#ifdef SWILL
				if (T.http_port) {
					swill_allow(args[1]);
					syslog (LOG_INFO, "allow http connects from %s", args[1]);
				} else
#endif
					syslog (LOG_INFO, "NOTE: http support not enabled!!!");
			}
		} else if (!strcasecmp (args[0], "retry")) {
			if (argcnt != 2) {
				syslog (LOG_ERR, "line %d: invalid format: retry <seconds>", linenum);
				errcnt++;
			} else {
				T.retry_interval = atoi(args[1]);
			}
		} else if (!strcasecmp (args[0], "pidfile")) {
			if (argcnt != 2) {
				syslog (LOG_ERR, "line %d: invalid format: pidfile <filename>", linenum);
				errcnt++;
			} else {
				T.pidfile = strdup(args[1]);
			}
		} else if (!strcasecmp (args[0], "interfaces")) {
			int i;

			if (argcnt < 2) {
				syslog (LOG_ERR, "line %d: invalid format: interfaces <ifa> <ifb> ...", linenum);
				errcnt++;
			}

			for (i = 0; T.iflist[i] && i < MAXINTERFACES; i++);

		 	if (i + argcnt-1 > MAXINTERFACES) {
				syslog (LOG_ERR, "line %d: to many interfaces, more than %d", linenum, MAXINTERFACES);
				errcnt++;
			} else {
				syslog (LOG_DEBUG, "line %d: %d interfaces listed, no wildcard socket", linenum, argcnt-1);

				T.iflist[i + argcnt--] = NULL;
				while (argcnt) {
					if (!strcmp(args[argcnt], "*")) {
						/* wildcard interface, ignore other ifn */
						T.iflist[0] = NULL;
						T.wildcard = 1;
						break;
					}
					T.iflist[i+argcnt-1] = strdup(args[argcnt]);
					argcnt--;
				}
				if (T.iflist[0])
					T.wildcard = 0;
			}
		} else if (!strcasecmp (args[0], "port")) {
			if (argcnt != 2) {
				syslog (LOG_ERR, "line %d: invalid format: port <portnum>", linenum);
				errcnt++;
			} else {
				T.port = atoi(args[1]);
			}
		}
#ifdef SCOPED_REWRITE
		else if (!strcasecmp(args[0], "scoped")) {
			if (argcnt != 4) {
				syslog (LOG_ERR, "line %d: invalid format: scoped <from> <to> <plen>", linenum);
				errcnt++;
			} else {
				if (conv_scoped_conf(args[1], args[2], atoi(args[3]))) {
					syslog (LOG_INFO, "can not add scoped %d: %s %s %s",
						T.scoped_prefixes, args[1], args[2], args[3]);
					errcnt++;
				} else {
					syslog(LOG_INFO, "scoped %d added: %s %s %d", 
						T.scoped_prefixes, args[1], args[2], atoi(args[3]));
				}
			}
		}
#endif
#ifdef STF
		else if  (!strcasecmp(args[0], "stf")) {
			if (argcnt != 1) {
				syslog (LOG_ERR, "line %d: invalid format: stf", linenum);
				errcnt++;
			} else {
				T.stf = 1;
			}
		}
#endif
		else {
			syslog (LOG_WARNING, "line %d: unknown keyword in config file: %s",
				linenum, args[0]);
			errcnt++;
		}
	}

	fclose (config_fp);

	if (errcnt) {
	    syslog (LOG_ERR, "errors found in config file. errcnt = %d", errcnt);
	    return -1;
	}
	else
	    syslog (LOG_INFO, "configuration file loaded.");

	return 0;
}
