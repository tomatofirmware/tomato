/****************************************************************************
 * Copyright (C) 1998 WIDE Project. All rights reserved.
 * Copyright (C) 1999,2000,2001,2002 University of Tromso. All rights reserved.
 * Copyright (C) 2002 Invenia Innovation AS. All rights reserved.
 *
 * Author: Feike W. Dillema, feico@pasta.cs.uit.no.
 *         based on newbie code by Yusuke DOI, Keio Univ. Murai Lab.
 ****************************************************************************/

/*
 * <$Id: totd.c,v 3.74 2005/02/02 11:10:31 dillema Exp $>
 */

#include "totd.h"

struct ToT T;
char *version = "Trick or Treat Daemon (totd) version 1.5";

void usage () {
	printf ("%s\n\
Usage: totd [-6|-no6|-4|-no4|-64|-dn|-v|-q|-p <prefix>|-c <filename>]\n\
\n\
-[no]6      : enable[disable] IPv6 service functionality\n\
-[no]4      : enable[disable] IPv4 service functionality\n\
-64         : alias to -6 -4\n\
-dn         : debug mode (no fork / loglevel = n)\n\
-v          : verbose\n\
-q          : quiet\n\
-u <user>   : username or uid totd should run at, after startup\n\
-g <group>  : groupname or gid totd should run at, after startup\n\
-t <dir>    : put totd in <dir> chroot() cage\n\
-p <prefix> : a prefix to use for totd tricks; more than one allowed\n\
-http-port <port> : port we listen on for http requests (default = 6464)\n\
-c <file>   : specify alternative totd configfile, default=%s\n\
\n\
default   : IPv6 is %s and IPv4 is %s\n\n\
totd %s use IPv6 because it was compiled %s USE_INET6 option in config.h\n\
totd %s use IPv4 because it was compiled %s USE_INET4 option in config.h\n",
		version,
		TOTCONF,
		(T.ip6) ? "enabled" : "disabled",
		(T.ip4) ? "enabled" : "disabled",
		(V4 (1) + 0) ? "can" : "can not",
		(V4 (1) + 0) ? "with" : "without",
		(V6 (1) + 0) ? "can" : "can not",
		(V6 (1) + 0) ? "with" : "without"
	);

	totd_exit (EXIT_FAILURE);
}

const char *hex = "0123456789abcdef";

int main (int argc, char **argv) {
	struct passwd *pwd_p;
	int i;

	/* initialize global totd structure */
	T.uid = getuid();
	T.gid = getgid();

	T.user = NULL;
	T.group = NULL;
	T.rootdir = NULL;

	/* fill in default config values */
	T.ip4 = V4 (1) + 0;	/* bit ugly, but short */
	T.ip6 = V6 (1) + 0;
	T.use_mapped = 0;
	T.quiet = 0;
	T.debug = 0;
	T.prefixnum = 0;
	T.rescan_iflist = 1;
	T.stf = 0;
	T.retry_interval = 300;
	T.port = PORT_SRV;
	T.http_port = 0;
	T.wildcard = 1;
	T.Fwd_list = NULL;
	T.pidfile = TOT_PID_FILE;
	T.configfile = TOTCONF;
	T.current_fwd = NULL;

	/* make sure these start out empty */
	for (i = 0; i < MAXPREFIXES; i++) {
		T.prefix[i][0] = '\0';
	}

	/* list of forwarders */
	T.Fwd_list = list_init ();
	if (!T.Fwd_list)
		exit(1);

	/* parse command line arguments */
	for (i = 1; i < argc; i++) {
		if (!strncmp (argv[i], "-d", 2) ) {
			/* debug option */
			T.debug = atoi(&argv[i][2]);
			syslog (LOG_INFO, "debug level %d enabled", T.debug);
		} else if (!strcmp (argv[i], "-6"))
			T.ip6 = 1;
		else if (!strcmp (argv[i], "-4"))
			T.ip4 = 1;
		else if (!strcmp (argv[i], "-46") || !strcmp (argv[i], "-64"))
			T.ip4 = 1, T.ip6 = 1;
		else if (!strcmp (argv[i], "-no4"))
			T.ip4 = 0;
		else if (!strcmp (argv[i], "-no6"))
			T.ip6 = 0;
		else if (!strcmp (argv[i], "-c"))
			T.configfile = strdup(argv[++i]);
		else if (!strcmp (argv[i], "-p")) {
			if (conv_trick_conf((u_char *)argv[++i])) {
			    syslog (LOG_ERR, "invalid prefix on command line: %s", argv[++i]);
			    usage(1);
			}
		}
		else if (!strcmp (argv[i], "-http-port")) {
			T.http_port = atoi(argv[++i]);
			if (!T.http_port) {
				syslog (LOG_ERR, "invalid portnumer: %s", argv[++i]);
				usage(1);
			}
		} else if (!strcmp (argv[i], "-u"))
			T.user = strdup(argv[++i]);
		else if (!strcmp (argv[i], "-g"))
			T.group = strdup(argv[++i]);
		else if (!strcmp (argv[i], "-t"))
			T.rootdir = strdup(argv[++i]);
		else if (!strcmp (argv[i], "-h"))
			usage (1);
		else if (!strcmp (argv[i], "-v"))
			T.quiet = -1;
		else if (!strcmp (argv[i], "-q"))
			T.quiet = 1;
		else {
			syslog (LOG_ERR, "unknown option %s", argv[i]);
			usage (1);
		}
	}

#ifndef LOG_PERROR
#define LOG_PERROR 0
#endif /* not LOG_PERROR */

	if (T.debug > 0)
	    openlog("totd", LOG_PID | LOG_NDELAY | LOG_CONS | LOG_PERROR, LOG_DAEMON);
	else
	    openlog ("totd", LOG_PID | LOG_NDELAY | LOG_CONS, LOG_DAEMON);

	syslog (LOG_NOTICE, "%s", version);

	if (T.debug) {
		if (T.quiet > 0)
			setlogmask (LOG_UPTO (LOG_ERR));
		else if (!T.quiet)
		setlogmask (LOG_UPTO (LOG_INFO));
		else
			setlogmask (LOG_UPTO (LOG_DEBUG));
	} else {
		if (T.quiet > 0)
			setlogmask (LOG_UPTO (LOG_ERR));
		else if (!T.quiet)
			setlogmask (LOG_UPTO (LOG_WARNING));
		else if (T.quiet < 0)
	       		setlogmask (LOG_UPTO (LOG_INFO));
	}

	if (T.user) {
                if (isdigit(T.user[0])) {
			T.uid = atoi(T.user);
			pwd_p = NULL;
		} else {
			pwd_p = getpwnam (T.user);
		}
	} else
		pwd_p = getpwuid (T.uid);

	if (pwd_p) {
		syslog (LOG_INFO, "Found user record of %s; uid: %d gid: %d",
			pwd_p->pw_name, pwd_p->pw_uid, pwd_p->pw_gid);
		if (T.uid && T.uid != pwd_p->pw_uid) {
			syslog (LOG_ERR, "Need root privileges to change user \
to: %s", T.user);
			totd_exit (EXIT_FAILURE);
		} else {
			T.uid = pwd_p->pw_uid;
		}
	} else {
		syslog (LOG_INFO, "can't find user record of %s", T.user);
	}

	if (T.group) {
		if (isdigit(T.group[0])) {
			T.gid = atoi(T.group);
		} else {
        		struct group *grp_p;

			grp_p = getgrnam(T.group);
			if (!grp_p) {
				syslog(LOG_ERR, "group `%s' unknown", T.group);
				totd_exit (EXIT_FAILURE);
			}
			T.gid = grp_p->gr_gid;
		}
	} else {
		if (pwd_p)
			T.gid = pwd_p->pw_gid;
		else
			T.gid = getgid();
	}

	/* close any open descriptors */
	endpwent();
	endgrent();

	/* chroot() cage totd if asked for */
	if (T.rootdir) {
                if (chroot(T.rootdir) < 0) {
                        syslog (LOG_ERR, "chroot %s failed: %m", T.rootdir);
                        totd_exit(EXIT_FAILURE);
                } 
                syslog (LOG_INFO, "chrooted to %s", T.rootdir);
                if (chdir("/") < 0) {
                        syslog (LOG_ERR, "chdir(\"/\") failed: %m");
                        totd_exit(EXIT_FAILURE); 
                }
	}

#ifdef SWILL
  	/* Initialize the SWILL server */
	if (T.http_port) {
  		swill_init(T.http_port);
		syslog (LOG_INFO, "Listening on [*]:%d for http requests", T.http_port);
		swill_deny("");
	}
#endif
	/*
	 * read in config file
	 * possibly overriding defaults and command line options
 	 */
	if (read_config (T.configfile)) {
	    syslog (LOG_ERR, "Configuration failure");
	    syslog (LOG_INFO, "Check log, or try -d option for debug mode...");
	    totd_exit (EXIT_FAILURE);
	}

	/* we cannot rebind to reserved ports if we are not root */
	if (T.rescan_iflist == 1 && T.uid && T.port < IPPORT_RESERVED) {
		syslog(LOG_WARNING, "Disabling rescanning of network interfaces");
		T.rescan_iflist = 0;
	}

	/* Check for pidfile conflicts */
	if (T.pidfile) {
		FILE *pid_fp;
		pid_t pid;

		/* check if pid file exists */
		pid_fp = fopen (T.pidfile, "r");
		if (pid_fp) {
			if (fscanf (pid_fp, "%d", &pid) != 1) {
				syslog (LOG_NOTICE, "Removing bogus lockfile");
				unlink (T.pidfile);
			} else if (kill (pid, 0) == -1 && !unlink (T.pidfile)) {
				syslog (LOG_NOTICE, "Removed stale lockfile");
			} else {
				syslog (LOG_ERR, "PID file %s already exists",
					T.pidfile);
				syslog (LOG_INFO, "There can be another totd \
running. Please make sure there's no totd already running. Then delete the \
file %s and try again.", T.pidfile);
				fclose (pid_fp);
				totd_exit (EXIT_FAILURE);
			}
			fclose (pid_fp);
		}
	}

	if (!T.ip4 && T.ip6) {
		/* If user specifies we should *not* accept IPv4
		 * requests on Linux, we will bail out with an error.
		 * totd does not filter incoming requests on their source
		 * address. It is not totd's task to do so IMHO, and I will
		 * probably never implement that.
		 */
#ifdef NEEDSV4MAPPED
		syslog(LOG_ERR, "Cannot disable IPv4 when IPv6 is enabled on\
this OS, due to IPv4 mapped addresses");
		syslog(LOG_INFO, "Will always accept IPv4 *and* IPv6 requests.\
Bailing out, so that you can explicitly tell me to do so");
		totd_exit (EXIT_FAILURE);
#endif
	}

	if (T.ip4 && T.ip6) {
#ifdef WILDCARDONLY
		if (!T.wildcard) {
			syslog(LOG_ERR, "On this OS we only support \
wildcard binding when IPv6 is enabled.");
			syslog(LOG_INFO, "Please remove `interfaces' \
specification in your config file.");
			totd_exit (EXIT_FAILURE);
		}
#endif
#ifdef NEEDSV4MAPPED
		syslog(LOG_DEBUG, "IPv6 wildcard socket with IPv4 mapped, \
will not bind to wildcard IPv4 socket.");
		T.use_mapped = 1;
#endif
	}

	if (T.ip4) {
#ifdef USE_INET4
		syslog (LOG_INFO, "IPv4 activated");
#else
		syslog (LOG_ERR, "IPv4 support is not compiled in");
		totd_exit (EXIT_FAILURE);
#endif
	}

	if (T.ip6) {
#ifdef USE_INET6
		syslog (LOG_INFO, "IPv6 activated");
#else
		syslog (LOG_ERR, "IPv6 support is not compiled in");
		totd_exit (EXIT_FAILURE);
#endif
	}

	if (!T.ip4 && !T.ip6) {
		syslog (LOG_ERR, "all supported protocols are deactivated; \
what do you want me to do then?");
		totd_exit (EXIT_FAILURE);
	}

#ifdef SCOPED_REWRITE
	if (T.wildcard && T.scoped_prefixes) {
		syslog (LOG_ERR, "Scoped address rewriting currently not \
implemented when wildcard sockets are used. Please use `interfaces' keyword \
in your config file or remove `scoped' keyword");
		totd_exit (EXIT_FAILURE);
	}
#endif

	fwd_init();
	fwd_select();
	if (!T.current_fwd) {
	    syslog (LOG_ERR, "no forwarder available, what should we do then?");
	    return -1;
	}

	/* initialize each event routine */
	ev_dup_init ();
	if (ev_signal_init () < 0) {
		syslog (LOG_ERR, "Signal event handling  initialize failed");
		totd_exit (EXIT_FAILURE);
	}

	if (ev_to_init () < 0) {
		syslog (LOG_ERR, "Timeout event handling initialize failed");
		totd_exit (EXIT_FAILURE);
	}

	if (ev_tcp_conn_in_init () < 0) {
		syslog (LOG_ERR, "TCP connection initialize failed");
		totd_exit (EXIT_FAILURE);
	}

	if (net_init_socketlist(T.port) < 0) {
		syslog (LOG_ERR, "Init list of sockets failed");
		totd_exit (EXIT_FAILURE);
	}

	if (net_bind_socketlist() <= 0) {
		if (!T.rescan_iflist) {
			syslog (LOG_ERR, "Could not open any sockets");
			totd_exit (EXIT_FAILURE);
		} else {
			syslog (LOG_WARNING, "Could not open any sockets");
			syslog (LOG_WARNING, "Maybe later??? Continuing");
		}
	}

	if (ev_udp_in_init () < 0) {
		syslog (LOG_ERR, "UDP initialize failed");
		totd_exit (EXIT_FAILURE);
	}
	if (ev_tcp_out_init () < 0) {
		syslog (LOG_ERR, "TCP output routine initialize failed");
		totd_exit (EXIT_FAILURE);
	}

	/* drop root privs */
	if (setgid(T.gid) < 0) {
		syslog (LOG_ERR, "setgid to %d failed", T.gid);
		totd_exit (EXIT_FAILURE);
	}
	if (setuid(T.uid) < 0) {
		syslog (LOG_ERR, "setuid to %d failed", T.uid);
		totd_exit (EXIT_FAILURE);
	}

	if (T.rescan_iflist) {
		if (ev_to_register_ifcheck () < 0) {
			syslog (LOG_ERR, "Registering Interface Check Event failed");
			totd_exit (EXIT_FAILURE);
		}
	}

	if (!T.debug) {
		if (daemon(0,0))
			totd_exit (EXIT_FAILURE);
		else
			syslog (LOG_INFO, "totd successfully daemonized");
	}

	/* as detached child, we can now write pid file */
	if (T.pidfile) {
		FILE *pid_fp;
		pid_fp = fopen (T.pidfile, "w");
		if (!pid_fp) {
			syslog (LOG_ERR, "can't open pid file \"%s\"", T.pidfile);
			totd_exit (EXIT_FAILURE);
		}
		fprintf (pid_fp, "%d", getpid ());
		syslog (LOG_INFO, "wrote pid %d to file %s", getpid (), T.pidfile);
		fclose (pid_fp);
	}

#ifdef SWILL
	if (T.http_port) {
		if (T.debug) swill_log(stderr);
		swill_title("Trick or Treat DNS Proxy");
        	swill_handle("index.html", print_stats, 0);
        	swill_handle("add-prefix.html", add_prefix, 0);
        	swill_handle("del-prefix.html", del_prefix, 0);
	}
#endif
	syslog (LOG_INFO, "totd started");
	totd_eventloop ();
	return (totd_exit (0));
}

void totd_eventloop (void) {
	const char *fn = "totd_eventloop()";
	struct timeval tv_out, *tvp;
	fd_set fd_read, fd_write;
	int max_fd, fdnum, i;
	time_t next_timeout;
#ifdef DBMALLOC
	unsigned long histid1, histid2, orig_size, current_size;

	/* Malloc debugging */
	if (T.debug) 
		malloc_dump (2);
	orig_size = malloc_inuse (&histid1);
	syslog (LOG_DEBUG, "Malloc Size: %ld", orig_size);
#endif

	while (1) {		/* main loop */
		if (T.debug > 2)
			syslog (LOG_DEBUG, "main loop: start");

		/* pick a proper forwarder */
		fwd_select ();

#ifdef DBMALLOC
		/* Malloc debugging */
		current_size = malloc_inuse (&histid2);

		if (current_size != orig_size) {
			syslog (LOG_DEBUG, "Malloc Size: %ld", current_size);
			if (T.debug) 
				malloc_list (2, histid1, histid2);
			orig_size = current_size;
			/* histid1 = histid2; */
		}
#endif

#ifdef SWILL
		if (T.http_port) swill_poll();
#endif
		/* signal event */
		ev_signal_process ();

		/* timeout event */
#ifdef SWILL
		if (T.http_port) {
			tv_out.tv_usec = 500;
			tv_out.tv_sec = 0;
			tvp = &tv_out;
		} else
#endif
		{
			next_timeout = ev_timeout_process ();
			if (!next_timeout) {
				if (T.debug > 2)
					syslog (LOG_DEBUG, "no timeouts at present");
				tvp = NULL;
			} else {
				tv_out.tv_usec = 0;
				tv_out.tv_sec = next_timeout - time (NULL);
				tvp = &tv_out;
				if (T.debug > 2)
					syslog (LOG_DEBUG, "next timeout after %ld s.",
						tv_out.tv_sec);
			}
		}

		/* get FD_SET for now */
		max_fd = 0;
		FD_ZERO (&fd_read);
		FD_ZERO (&fd_write);

		if (T.debug > 3)
			syslog (LOG_DEBUG, "check for UDP fds...");

		nia_fds_set (&fd_read, &max_fd);

		if (T.debug > 3)
			syslog (LOG_DEBUG, "check for TCP-in fds...");
		i = ev_tcp_conn_in_fds (&fd_read);
		max_fd = MAXNUM (i, max_fd);

		if (T.debug > 3)
			syslog (LOG_DEBUG, "check for TCP-out fds...");
		i = ev_tcp_out_fds (&fd_write);
		max_fd = MAXNUM (i, max_fd);

		if (T.debug > 3)
			syslog (LOG_DEBUG, "%s: max_fd = %d", fn, max_fd);

		/* select */
		if (T.debug > 2)
			syslog (LOG_DEBUG, "main loop: select");

		fdnum = select (max_fd + 1, &fd_read, &fd_write, NULL, tvp);
		if (fdnum < 0) {
			if (errno == EINTR) {
				syslog (LOG_DEBUG, "%s: select() interrupted",
					fn);
				continue;	/* while(1) */
			} else {
				syslog (LOG_ERR, "%s: select(): %m", fn);
				if (net_reinit_socketlist (T.port, 1) < 0)
					totd_exit (EXIT_FAILURE);
				sleep (1);
			}
		} else {
			int sock;

			switch (nia_fds_isset (&fd_read, &sock)) {
			case 0: /* UDP */
				if (ev_udp_in_read (sock) < 0)
					syslog (LOG_INFO, "udp service error");
				continue;
			case 1: /* TCP */
				if (ev_tcp_srv_accept (sock) < 0)
					syslog (LOG_INFO, "tcp service error");
				continue;
			default: /* not found */
				if (ev_tcp_out_fd_check (&fd_write) < 0)
					syslog (LOG_INFO, "tcp output failed");

				if (ev_tcp_conn_in_fd_check (&fd_read) < 0)
					syslog (LOG_INFO, "tcp input failed");
			}
		}		/* if(...select...) */
	}			/* while(1) */
}

int totd_exit (int status) {

	/* finish in reverse order of initialize */
	ev_tcp_out_finish ();
	ev_udp_in_finish ();
	ev_tcp_conn_in_finish ();
	/* ev_tcp_srv_in_finish() -- no such func */
	ev_to_finish ();
	ev_signal_finish ();
        if (T.Fwd_list)
                list_destroy (T.Fwd_list, fwd_freev);

	if (T.pidfile)
		unlink (T.pidfile);

	if (status != EXIT_SUCCESS) {
		syslog (LOG_ERR, "terminated with error");
		fprintf (stderr, "totd terminated with error, \
check system logs for details or run totd in debug mode.\n");
	}

	exit (status);
}
