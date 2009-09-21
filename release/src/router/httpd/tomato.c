/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "tomato.h"

#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <time.h>


//	#define DEBUG_NOEXECSERVICE
//	#define DEBUG_NVRAMSET(k, v)	cprintf("nvram set %s=%s\n", k, v);
#define	DEBUG_NVRAMSET(k, v)	do { } while(0);


char *post_buf = NULL;
int rboot = 0;
extern int post;

void asp_resmsg(int argc, char **argv);

//
static void wo_tomato(char *url);
static void wo_update(char *url);
static void wo_service(char *url);
static void wo_shutdown(char *url);
static void wo_nvcommit(char *url);
//	static void wo_logout(char *url);


// ----------------------------------------------------------------------------


void exec_service(const char *action)
{
	int i;

	_dprintf("exec_service: %s\n", action);

	i = 10;
	while ((!nvram_match("action_service", "")) && (i-- > 0))  {
		_dprintf("%s: waiting before %d\n", __FUNCTION__, i);
		sleep(1);
	}

	nvram_set("action_service", action);
	kill(1, SIGUSR1);

	i = 3;
	while ((nvram_match("action_service", (char *)action)) && (i-- > 0))  {
		_dprintf("%s: waiting after %d\n", __FUNCTION__, i);
		sleep(1);
	}

/*
	if (atoi(webcgi_safeget("_service_wait", ""))) {
		i = 10;
		while ((nvram_match("action_service", (char *)action)) && (i-- > 0))  {
			_dprintf("%s: waiting after %d\n", __FUNCTION__, i);
			sleep(1);
		}
	}
*/
}

static void wi_generic_noid(char *url, int len, char *boundary)
{
	if (post == 1) {
		if (len >= (32 * 1024)) {
//			syslog(LOG_WARNING, "POST max");
			exit(1);
		}

		if (!post_buf) free(post_buf);
		if ((post_buf = malloc(len + 1)) == NULL) {
//			syslog(LOG_CRIT, "Unable to allocate post buffer");
			exit(1);
		}

		if (web_read_x(post_buf, len) != len) {
			exit(1);
		}
		post_buf[len] = 0;
		webcgi_init(post_buf);
	}
}

void wi_generic(char *url, int len, char *boundary)
{
	wi_generic_noid(url, len, boundary);
	check_id(url);
}

static void wo_blank(char *url)
{
	web_puts("\n\n\n\n");
}

static void wo_favicon(char *url)
{
	send_header(200, NULL, "image/vnd.microsoft.icon", 0);
	do_file(url);
/*
	if (nvram_match("web_favicon", "1")) {
		send_header(200, NULL, "image/vnd.microsoft.icon", 0);
		do_file(url);
	}
	else {
		send_error(404, NULL, NULL);
	}
*/
}

static void wo_cfe(char *url)
{
	do_file("/dev/mtd/0ro");
}

static void wo_nvram(char *url)
{
	web_pipecmd("nvram show", WOF_NONE);
}

static void wo_iptables(char *url)
{
	web_pipecmd("iptables -nvL; iptables -t nat -nvL; iptables -t mangle -nvL", WOF_NONE);
}

/*
static void wo_spin(char *url)
{
	char s[64];

	strlcpy(s, nvram_safe_get("web_css"), sizeof(s));
	strlcat(s, "_spin.gif", sizeof(s));
	if (f_exists(s)) do_file(s);
		else do_file("_spin.gif");
}
*/

void common_redirect(void)
{
	if (atoi(webcgi_safeget("_ajax", ""))) {
		send_header(200, NULL, mime_html, 0);
		web_puts("OK");
	}
	else {
		redirect(webcgi_safeget("_redirect", "/"));
	}
}

// ----------------------------------------------------------------------------

const struct mime_handler mime_handlers[] = {
	{ "update.cgi",		mime_javascript,			0,	wi_generic,			wo_update,		1 },
	{ "tomato.cgi",		NULL,						0,	wi_generic,		    wo_tomato,		1 },

	{ "debug.js",		mime_javascript,			5,	wi_generic_noid,	wo_blank,		1 },	// while debugging
	{ "cfe/*.bin",		mime_binary,				0,	wi_generic,			wo_cfe,			1 },
	{ "nvram/*.txt",	mime_binary,				0,	wi_generic,			wo_nvram,		1 },
	{ "ipt/*.txt",		mime_binary,				0,	wi_generic,			wo_iptables,	1 },

	{ "cfg/*.cfg",			NULL,					0,	wi_generic,			wo_backup,		1 },
	{ "cfg/restore.cgi",	mime_html,				0,	wi_restore,			wo_restore,		1 },
	{ "cfg/defaults.cgi",	NULL,					0,	wi_generic,	 		wo_defaults,	1 },

	{ "bwm/*.gz",			NULL,					0,	wi_generic,			wo_bwmbackup,	1 },
	{ "bwm/restore.cgi",	NULL,					0,	wi_bwmrestore,		wo_bwmrestore,	1 },

	{ "logs/view.cgi",	NULL,						0,	wi_generic,			wo_viewlog,		1 },
	{ "logs/*.txt",		NULL,						0,	wi_generic,			wo_syslog,		1 },

	{ "logout.asp",			NULL,					0,	wi_generic,			wo_asp,			1 },
	{ "clearcookies.asp",	NULL,					0,	wi_generic,			wo_asp,			1 },

//	{ "spin.gif",		NULL,						0,	wi_generic_noid,	wo_spin,		1 },

	{ "**.asp",			NULL,						0,	wi_generic_noid,	wo_asp,			1 },
	{ "**.css",			"text/css",					2,	wi_generic_noid,	do_file,		1 },
	{ "**.htm",			mime_html,		  		  	2,	wi_generic_noid,	do_file,		1 },
	{ "**.gif",			"image/gif",				5,	wi_generic_noid,	do_file,		1 },
	{ "**.jpg",			"image/jpeg",				5,	wi_generic_noid,	do_file,		1 },
	{ "**.png",			"image/png",				5,	wi_generic_noid,	do_file,		1 },
	{ "**.js",			mime_javascript,			2,	wi_generic_noid,	do_file,		1 },
	{ "**.jsx",			mime_javascript,			0,	wi_generic,			wo_asp,			1 },
	{ "**.svg",			"image/svg+xml",			2,	wi_generic_noid,	do_file,		1 },
	{ "**.txt",			mime_plain,					2,	wi_generic_noid,	do_file,		1 },
	{ "**.bin",			mime_binary,				0,	wi_generic_noid,	do_file,		1 },
	{ "**.bino",		mime_octetstream,			0,	wi_generic_noid,	do_file,		1 },
	{ "favicon.ico",	NULL,						5,	wi_generic_noid,	wo_favicon,		1 },


	{ "dhcpc.cgi",		NULL,						0,	wi_generic,			wo_dhcpc,		1 },
	{ "dhcpd.cgi",		mime_javascript,			0,	wi_generic,			wo_dhcpd,		1 },
	{ "nvcommit.cgi",	NULL,						0,	wi_generic,			wo_nvcommit,	1 },
	{ "ping.cgi",		mime_javascript,			0,	wi_generic,			wo_ping,		1 },
	{ "trace.cgi",		mime_javascript,			0,	wi_generic,			wo_trace,		1 },
	{ "upgrade.cgi",	mime_html,					0,	wi_upgrade,			wo_flash,		1 },
	{ "upnp.cgi",		NULL,						0,	wi_generic,			wo_upnp,		1 },
	{ "wakeup.cgi",		NULL,						0,	wi_generic,			wo_wakeup,		1 },
	{ "wlmnoise.cgi",	mime_html,					0,	wi_generic,			wo_wlmnoise,	1 },
	{ "wlradio.cgi",	NULL,						0,	wi_generic,			wo_wlradio,		1 },
	{ "resolve.cgi",	mime_javascript,			0,	wi_generic,			wo_resolve,		1 },
	{ "expct.cgi",		mime_html,					0,	wi_generic,			wo_expct,		1 },
	{ "service.cgi",	NULL,						0,	wi_generic,			wo_service,		1 },
//	{ "logout.cgi",		NULL,	   		 			0,	wi_generic,			wo_logout,		0 },	// see httpd.c
	{ "shutdown.cgi",	mime_html,					0,	wi_generic,			wo_shutdown,	1 },
	{ "usbcmd.cgi",			mime_javascript,			0,	wi_generic,		wo_usbcommand,		1 },	//!!TB - USB
#ifdef BLACKHOLE
	{ "blackhole.cgi",	NULL,						0,	wi_blackhole,		NULL,			1 },
#endif
//	{ "test",			mime_html,					0,	wi_generic,			wo_test,		1 },
	{ NULL,				NULL,						0,	NULL,				NULL,			1 }
};

const aspapi_t aspapi[] = {
	{ "activeroutes",		asp_activeroutes	},
	{ "arplist",			asp_arplist			},
	{ "bandwidth",			asp_bandwidth		},
	{ "build_time",			asp_build_time		},
	{ "cgi_get",			asp_cgi_get			},
	{ "compmac",			asp_compmac			},
	{ "ctcount",			asp_ctcount			},
	{ "ctdump",				asp_ctdump			},
	{ "ddnsx",				asp_ddnsx			},
	{ "devlist",			asp_devlist			},
	{ "dhcpc_time",			asp_dhcpc_time		},
	{ "dns",				asp_dns				},
	{ "ident",				asp_ident			},
	{ "lanip",				asp_lanip			},
	{ "layer7",				asp_layer7			},
	{ "link_uptime",		asp_link_uptime		},
	{ "lipp",				asp_lipp			},
	{ "netdev",				asp_netdev			},
	{ "notice",				asp_notice			},
	{ "nv",					asp_nv				},
	{ "nvram",				asp_nvram 			},
	{ "nvramseq",			asp_nvramseq		},
	{ "psup",				asp_psup			},
	{ "qrate",				asp_qrate			},
	{ "resmsg",				asp_resmsg			},
	{ "rrule",				asp_rrule			},
	{ "statfs",				asp_statfs			},
	{ "sysinfo",			asp_sysinfo			},
	{ "time",				asp_time			},
	{ "upnpinfo",			asp_upnpinfo		},
	{ "version",			asp_version			},
	{ "wanstatus",			asp_wanstatus		},
	{ "wanup",				asp_wanup			},
	{ "wlchannel",			asp_wlchannel		},
	{ "wlclient",			asp_wlclient		},
	{ "wlcrssi",			asp_wlcrssi			},
	{ "wlnoise",			asp_wlnoise			},
	{ "wlradio",			asp_wlradio			},
	{ "wlscan",				asp_wlscan			},
	{ "wlchannels",			asp_wlchannels	},	//!!TB
#if TOMATO_SL
	{ "sharelist",			asp_sharelist		},
#endif
	{ "usbdevices",			asp_usbdevices	},	//!!TB - USB Support
	{ NULL,					NULL				}
};

// -----------------------------------------------------------------------------

const char *resmsg_get(void)
{
	return webcgi_safeget("resmsg", "");
}

void resmsg_set(const char *msg)
{
	webcgi_set("resmsg", strdup(msg));	// m ok
}

int resmsg_fread(const char *fname)
{
	char s[256];
	char *p;

	f_read_string(fname, s, sizeof(s));
	if ((p = strchr(s, '\n')) != NULL) *p = 0;
	if (s[0]) {
		resmsg_set(s);
		return 1;
	}
	return 0;
}

void asp_resmsg(int argc, char **argv)
{
	char *p;

	if ((p = js_string(webcgi_safeget("resmsg", (argc > 0) ? argv[0] : ""))) == NULL) return;
	web_printf("\nresmsg='%s';\n", p);
	free(p);
}

// ----------------------------------------------------------------------------

// verification... simple sanity checks. UI should verify all fields.

// todo: move and re-use for filtering	- zzz

typedef union {
	int i;
	long l;
	const char *s;
} nvset_varg_t;

typedef struct {
	const char *name;
	enum {
		VT_NONE,		// no checking
		VT_LENGTH,		// check length of string
		VT_TEXT,		// strip \r, check length of string
		VT_RANGE,		// expect an integer, check range
		VT_IP,			// expect an ip address
		VT_MAC,			// expect a mac address
		VT_TEMP			// no checks, no commit
	} vtype;
	nvset_varg_t va;
	nvset_varg_t vb;
} nvset_t;


#define	V_NONE				VT_NONE,	{ }, 			{ }
#define V_01				VT_RANGE,	{ .l = 0 },		{ .l = 1 }
#define V_PORT				VT_RANGE,	{ .l = 2 },		{ .l = 65535 }
#define V_ONOFF				VT_LENGTH,	{ .i = 2 },		{ .i = 3 }
#define V_WORD				VT_LENGTH,	{ .i = 1 },		{ .i = 16 }
#define V_LENGTH(min, max)	VT_LENGTH,	{ .i = min },	{ .i = max }
#define V_TEXT(min, max)	VT_TEXT,	{ .i = min },	{ .i = max }
#define V_RANGE(min, max)	VT_RANGE,	{ .l = min },	{ .l = max }
#define V_IP				VT_IP,		{ },			{ }
#define	V_OCTET				VT_RANGE,	{ .l = 0 },		{ .l = 255 }
#define V_NUM				VT_RANGE,	{ .l = 0 },		{ .l = 0x7FFFFFFF }
#define	V_TEMP				VT_TEMP,	{ }, 			{ }

static const nvset_t nvset_list[] = {

// basic-ident
	{ "router_name",		V_LENGTH(0, 32)		},
	{ "wan_hostname",		V_LENGTH(0, 32)		},
	{ "wan_domain",			V_LENGTH(0, 32)		},

// basic-time
	{ "tm_tz",				V_LENGTH(1, 64)		},	// PST8PDT
	{ "tm_sel",				V_LENGTH(1, 64)		},	// PST8PDT
	{ "tm_dst",				V_01				},
	{ "ntp_updates",		V_RANGE(-1, 24)		},
	{ "ntp_tdod",			V_01				},
	{ "ntp_server",			V_LENGTH(1, 150)	},	// x y z
	{ "ntp_kiss",			V_LENGTH(0, 255)	},

// basic-static
	{ "dhcpd_static",		V_LENGTH(0, 85*101)	},	// 85 (max chars per entry) x 100 entries

// basic-ddns
	{ "ddnsx0",				V_LENGTH(0, 2048)	},
	{ "ddnsx1",				V_LENGTH(0, 2048)	},
	{ "ddnsx0_cache",		V_LENGTH(0, 1)		},	// only to clear
	{ "ddnsx1_cache",		V_LENGTH(0, 1)		},
	{ "ddnsx_ip",			V_LENGTH(0, 32)		},
	{ "ddnsx_save",			V_01				},
	{ "ddnsx_refresh",		V_RANGE(0, 365)		},

// basic-network
	// WAN
	{ "wan_proto",			V_LENGTH(1, 16)		},	// disabled, dhcp, static, pppoe, pptp, l2tp
	{ "wan_ipaddr",			V_IP				},
	{ "wan_netmask",		V_IP				},
	{ "wan_gateway",		V_IP				},
	{ "hb_server_ip",		V_LENGTH(0, 32)		},
	{ "l2tp_server_ip",		V_IP				},
	{ "pptp_server_ip",		V_IP				},
	{ "ppp_username",		V_LENGTH(0, 50)		},
	{ "ppp_passwd",			V_LENGTH(0, 50)		},
	{ "ppp_service",		V_LENGTH(0, 50)		},
	{ "ppp_demand",			V_01				},
	{ "ppp_idletime",		V_RANGE(0, 1440)	},
	{ "ppp_redialperiod",	V_RANGE(1, 86400)	},
	{ "mtu_enable",			V_01				},
	{ "wan_mtu",			V_RANGE(576, 1500)	},
	{ "wan_islan",			V_01				},

	// LAN
	{ "lan_ipaddr",			V_IP				},
	{ "lan_netmask",		V_IP				},
	{ "lan_gateway",		V_IP				},
	{ "wan_dns",			V_LENGTH(0, 50)		},	// ip ip ip
	{ "lan_proto",			V_WORD				},	// static, dhcp
	{ "dhcp_start",			V_RANGE(1, 254)		},	// remove !
	{ "dhcp_num",			V_RANGE(1, 255)		},	// remove !
	{ "dhcpd_startip",		V_IP				},
	{ "dhcpd_endip",		V_IP				},
	{ "dhcp_lease",			V_RANGE(1, 10080)	},
	{ "wan_wins",			V_IP				},

	// wireless
	{ "wl_radio",			V_01				},
	{ "wl_mode",			V_LENGTH(2, 3)		},	// ap, sta, wet, wds
	{ "wl_net_mode",		V_LENGTH(5, 8)		},  // disabled, mixed, b-only, g-only, bg-mixed, n-only [speedbooster]
	{ "wl_ssid",			V_LENGTH(1, 32)		},
	{ "wl_closed",			V_01				},
	{ "wl_channel",			V_RANGE(0, 14)		},	//!!TB - 0=Auto
#if TOMATO_N
	// ! update
#endif

	{ "security_mode2",		V_LENGTH(1, 32)		},	// disabled, radius, wep, wpa_personal, wpa_enterprise, wpa2_personal, wpa2_enterprise
	{ "wl_radius_ipaddr",	V_IP				},
	{ "wl_radius_port",		V_PORT				},
	{ "wl_radius_key",		V_LENGTH(1, 64)		},
	{ "wl_wep_bit",			V_RANGE(64, 128)	},	// 64 or 128
	{ "wl_passphrase",		V_LENGTH(0, 20)		},
	{ "wl_key",				V_RANGE(1, 4)		},
	{ "wl_key1",			V_LENGTH(0, 26)		},
	{ "wl_key2",			V_LENGTH(0, 26)		},
	{ "wl_key3",			V_LENGTH(0, 26)		},
	{ "wl_key4",			V_LENGTH(0, 26)		},
	{ "wl_crypto",			V_LENGTH(3, 8)		},	// tkip, aes, tkip+aes
	{ "wl_wpa_psk",			V_LENGTH(8, 64)		},
	{ "wl_wpa_gtk_rekey",	V_RANGE(60, 7200)	},

	{ "wl_lazywds",			V_01				},
	{ "wl_wds",				V_LENGTH(0, 180)	},	// mac mac mac (x 10)

	{ "security_mode",		V_LENGTH(1, 32)		},	//  disabled, radius, wpa, psk,wep, wpa2, psk2, wpa wpa2, psk psk2
	{ "wds_enable",			V_01				},
	{ "wl_gmode",			V_RANGE(-1, 6)		},
	{ "wl_wep",				V_LENGTH(1, 32)		},	//  off, on, restricted,tkip,aes,tkip+aes
	{ "wl_akm",				V_LENGTH(0, 32)		},	//  wpa, wpa2, psk, psk2, wpa wpa2, psk psk2, ""
	{ "wl_auth_mode",	   	V_LENGTH(4, 6)		},	//  none, radius

#if TOMATO_N
	{ "wl_nmode",			V_NONE				},
	{ "wl_nreqd",			V_NONE				},
#endif

// basic-wfilter
	{ "wl_macmode",			V_NONE				},	// allow, deny, disabled
	{ "wl_maclist",			V_LENGTH(0, 18*201)	},	// 18 x 200		(11:22:33:44:55:66 ...)
	{ "macnames",			V_LENGTH(0, 62*201)	},	// 62 (12+1+48+1) x 50	(112233445566<..>)		todo: re-use -- zzz

// advanced-ctnf
	{ "ct_max",				V_RANGE(128, 10240)	},
	{ "ct_tcp_timeout",		V_LENGTH(20, 70)	},
	{ "ct_udp_timeout",		V_LENGTH(5, 15)		},
	{ "nf_ttl",				V_RANGE(-10, 10)	},
	{ "nf_l7in",			V_01				},
	{ "nf_rtsp",			V_01				},
	{ "nf_pptp",			V_01				},
	{ "nf_h323",			V_01				},
	{ "nf_ftp",				V_01				},

// advanced-dhcpdns
	{ "dhcpd_slt",			V_RANGE(-1, 43200)	},	// -1=infinite, 0=follow normal lease time, >=1 custom
	{ "dhcpd_dmdns",		V_01				},
	{ "dhcpd_lmax",			V_NUM				},
	{ "dns_addget",			V_01				},
	{ "dns_intcpt",			V_01				},
	{ "dhcpc_minpkt",		V_01				},
	{ "dnsmasq_custom",		V_TEXT(0, 2048)		},
//	{ "dnsmasq_norw",		V_01				},

// advanced-firewall
	{ "block_wan",			V_01				},
	{ "multicast_pass",		V_01				},
	{ "block_loopback",		V_01				},
	{ "nf_loopback",		V_NUM				},
	{ "ne_syncookies",		V_01				},

// advanced-misc
	{ "wait_time",			V_RANGE(3, 20)		},
	{ "wan_speed",			V_RANGE(0, 4)		},

// advanced-mac
	{ "mac_wan",			V_LENGTH(0, 17)		},
	{ "mac_wl",				V_LENGTH(0, 17)		},

// advanced-routing
	{ "routes_static",		V_LENGTH(0, 2048)	},
	{ "lan_stp",			V_RANGE(0, 1)		},
	{ "wk_mode",			V_LENGTH(1, 32)		},	// gateway, router
	{ "dr_setting",			V_RANGE(0, 3)		},
	{ "dr_lan_tx",			V_LENGTH(0, 32)		},
	{ "dr_lan_rx",			V_LENGTH(0, 32)		},
	{ "dr_wan_tx",			V_LENGTH(0, 32)		},
	{ "dr_wan_rx",			V_LENGTH(0, 32)		},

// advanced-wireless
	{ "wl_country",			V_LENGTH(0, 64)		},	// !!TB - Country code
	{ "wl_country_code",		V_LENGTH(0, 4)		},	// !!TB - Country code
	{ "wl_btc_mode",		V_RANGE(0, 2)		},	// !!TB - BT Coexistence Mode: 0 (disable), 1 (enable), 2 (preemption)
	{ "wl_afterburner",		V_LENGTH(2, 4)		},	// off, on, auto
	{ "wl_auth",			V_01				},
	{ "wl_rateset",			V_LENGTH(2, 7)		},	// all, default, 12
	{ "wl_rate",			V_RANGE(0, 54 * 1000 * 1000)	},
	{ "wl_mrate",			V_RANGE(0, 54 * 1000 * 1000)	},
	{ "wl_gmode_protection",V_LENGTH(3, 4)		},	// off, auto
	{ "wl_frameburst",		V_ONOFF				},	// off, on
	{ "wl_bcn",				V_RANGE(1, 65535)	},
	{ "wl_dtim",			V_RANGE(1, 255)		},
	{ "wl_frag",			V_RANGE(256, 2346)	},
	{ "wl_rts",				V_RANGE(0, 2347)	},
	{ "wl_ap_isolate",		V_01				},
	{ "wl_plcphdr",			V_LENGTH(4, 5)		},	// long, short
	{ "wl_antdiv",			V_RANGE(0, 3)		},
	{ "wl_txant",			V_RANGE(0, 3)		},
	{ "wl_txpwr",			V_RANGE(0, 255)		},
	{ "wl_wme",				V_ONOFF				},	// off, on
	{ "wl_wme_no_ack",		V_ONOFF				},	// off, on
	{ "wl_maxassoc",		V_RANGE(0, 255)	},
	{ "wl_distance",		V_LENGTH(0, 5)		},	// "", 1-99999
	{ "wlx_hpamp",			V_01				},
	{ "wlx_hperx",			V_01				},
	{ "wl_reg_mode",		V_LENGTH(1, 3)			},	// !!TB - Regulatory: off, h, d

#if TOMATO_N
	{ "wl_nmode_protection",V_WORD,				},	// off, auto
	{ "wl_nmcsidx",			V_RANGE(-2, 15),	},	// -2 - 15
#endif

// forward-dmz
	{ "dmz_enable",			V_01				},
	{ "dmz_ipaddr",			V_LENGTH(0, 15)		},
	{ "dmz_sip",			V_LENGTH(0, 512)	},

// forward-upnp
	{ "upnp_enable",		V_NUM				},
#ifndef USE_MINIUPNPD
	{ "upnp_mnp",			V_01				},
//	{ "upnp_config",		V_01				},
	{ "upnp_ssdp_interval", V_RANGE(10, 9999)	},
	{ "upnp_max_age",		V_RANGE(5, 9999)	},
#endif

// forward-basic
	{ "portforward",		V_LENGTH(0, 4096)	},

// forward-triggered
	{ "trigforward",		V_LENGTH(0, 4096)	},


// access restriction
	{ "rruleN",				V_RANGE(0, 49)		},
//	{ "rrule##",			V_LENGTH(0, 2048)	},	// in save_variables()

// admin-access
	{ "http_enable",		V_01				},
	{ "https_enable",		V_01				},
	{ "https_crt_save",		V_01				},
	{ "https_crt_cn",		V_LENGTH(0, 64)		},
	{ "https_crt_gen",		V_TEMP				},
	{ "remote_management",	V_01				},
	{ "remote_mgt_https",	V_01				},
	{ "http_lanport",		V_PORT				},
	{ "https_lanport",		V_PORT				},
	{ "web_wl_filter",		V_01				},
//	{ "web_favicon",		V_01				},
	{ "web_css",			V_LENGTH(1, 32)		},
	{ "http_wanport",		V_PORT				},
	{ "telnetd_eas",		V_01				},
	{ "telnetd_port",		V_PORT				},
	{ "sshd_eas",			V_01				},
	{ "sshd_pass",			V_01				},
	{ "sshd_port",			V_PORT				},
	{ "sshd_remote",		V_01				},
	{ "sshd_forwarding",		V_01				},
	{ "sshd_rport", 		V_PORT				},
	{ "sshd_authkeys",		V_TEXT(0, 4096)		},
	{ "rmgt_sip",			V_LENGTH(0, 512)	},
	{ "ne_shlimit",			V_TEXT(1, 50)		},

// admin-bwm
	{ "rstats_enable",		V_01				},
	{ "rstats_path",		V_LENGTH(0, 48)		},
	{ "rstats_stime",		V_RANGE(1, 168)		},
	{ "rstats_offset",		V_RANGE(1, 31)		},
	{ "rstats_exclude",		V_LENGTH(0, 64)		},
	{ "rstats_sshut",		V_01				},
	{ "rstats_bak",			V_01				},

// admin-buttons
	{ "sesx_b0",			V_RANGE(0, 4)		},	// 0-4: toggle wireless, reboot, shutdown, script
	{ "sesx_b1",			V_RANGE(0, 4)		},	// "
	{ "sesx_b2",			V_RANGE(0, 4)		},	// "
	{ "sesx_b3",			V_RANGE(0, 4)		},	// "
	{ "sesx_script",		V_TEXT(0, 1024)		},	//

// admin-debug
	{ "debug_nocommit",		V_01				},
	{ "debug_cprintf",		V_01				},
	{ "debug_cprintf_file",	V_01				},
//	{ "debug_keepfiles",	V_01				},
	{ "debug_ddns",			V_01				},
	{ "debug_norestart",	V_TEXT(0, 128)		},
	{ "console_loglevel",	V_RANGE(1, 8)		},
	{ "t_cafree",			V_01				},
	{ "t_hidelr",			V_01				},

// admin-sched
	{ "sch_rboot", 			V_TEXT(0, 64)		},
	{ "sch_rcon", 			V_TEXT(0, 64)		},
	{ "sch_c1",				V_TEXT(0, 64)		},
	{ "sch_c1_cmd",			V_TEXT(0, 2048)		},
	{ "sch_c2",				V_TEXT(0, 64)		},
	{ "sch_c2_cmd",			V_TEXT(0, 2048)		},
	{ "sch_c3",				V_TEXT(0, 64)		},
	{ "sch_c3_cmd",			V_TEXT(0, 2048)		},

// admin-scripts
	{ "script_init", 		V_TEXT(0, 4096)		},
	{ "script_shut", 		V_TEXT(0, 4096)		},
	{ "script_fire", 		V_TEXT(0, 8192)		},
	{ "script_wanup", 		V_TEXT(0, 4096)		},

// admin-log
	{ "log_remote",			V_01				},
	{ "log_remoteip",		V_IP				},
	{ "log_remoteport",		V_PORT				},
	{ "log_file",			V_01				},
	{ "log_limit",			V_RANGE(0, 2400)	},
	{ "log_in",				V_RANGE(0, 3)		},
	{ "log_out",			V_RANGE(0, 3)		},
	{ "log_mark",			V_RANGE(0, 1440)	},
	{ "log_events",			V_TEXT(0, 32)		},	// "acre,crond,ntp"

// admin-cifs
	{ "cifs1",				V_LENGTH(1, 1024)	},
	{ "cifs2",				V_LENGTH(1, 1024)	},

// admin-jffs2
	{ "jffs2_on",			V_01				},
	{ "jffs2_exec",			V_LENGTH(0, 64)		},
	{ "jffs2_format",		V_01				},

// nas-usb - !!TB
	{ "usb_enable",			V_01				},
	{ "usb_uhci",			V_01				},
	{ "usb_ohci",			V_01				},
	{ "usb_usb2",			V_01				},
	{ "usb_storage",		V_01				},
	{ "usb_printer",		V_01				},
	{ "usb_printer_bidirect",	V_01				},
	{ "usb_fs_ext3",		V_01				},
	{ "usb_fs_fat",			V_01				},
#ifdef TCONFIG_NTFS
	{ "usb_fs_ntfs",		V_01				},
#endif
	{ "usb_automount",		V_01				},
	{ "script_usbhotplug", 		V_TEXT(0, 2048)			},
	{ "script_usbmount", 		V_TEXT(0, 2048)			},
	{ "script_usbumount", 		V_TEXT(0, 2048)			},

// nas-ftp - !!TB
#ifdef TCONFIG_FTP
	{ "ftp_enable",			V_RANGE(0, 2)			},
	{ "ftp_super",			V_01				},
	{ "ftp_anonymous",		V_RANGE(0, 3)			},
	{ "ftp_dirlist",		V_RANGE(0, 2)			},
	{ "ftp_port",			V_PORT				},
	{ "ftp_max",			V_RANGE(0, 12)			},
	{ "ftp_ipmax",			V_RANGE(0, 12)			},
	{ "ftp_staytimeout",		V_RANGE(0, 65535)		},
	{ "ftp_rate",			V_RANGE(0, 99999)		},
	{ "ftp_anonrate",		V_RANGE(0, 99999)		},
	{ "ftp_anonroot",		V_LENGTH(0, 256)		},
	{ "ftp_pubroot",		V_LENGTH(0, 256)		},
	{ "ftp_pvtroot",		V_LENGTH(0, 256)		},
	{ "ftp_users",			V_LENGTH(0, 4096)		},
	{ "ftp_custom",			V_TEXT(0, 2048)			},
	{ "ftp_sip",			V_LENGTH(0, 512)		},
	{ "ftp_limit",			V_TEXT(1, 50)			},
	{ "log_ftp",			V_01				},
#endif

#ifdef TCONFIG_SAMBASRV
// nas-samba - !!TB
	{ "smbd_enable",		V_RANGE(0, 2)			},
	{ "smbd_wgroup",		V_LENGTH(0, 20)			},
	{ "smbd_cpage",			V_LENGTH(0, 4)			},
	{ "smbd_cset",			V_LENGTH(0, 20)			},
	{ "smbd_loglevel",		V_RANGE(0, 100)			},
	{ "smbd_custom",		V_TEXT(0, 2048)			},
	{ "smbd_autoshare",		V_RANGE(0, 3)			},
	{ "smbd_shares",		V_LENGTH(0, 4096)		},
	{ "smbd_user",			V_LENGTH(0, 50)			},
	{ "smbd_passwd",		V_LENGTH(0, 50)			},
#endif

//	qos
	{ "qos_enable",			V_01				},
	{ "qos_ack",			V_01				},
	{ "qos_syn",			V_01				},
	{ "qos_fin",			V_01				},
	{ "qos_rst",			V_01				},
	{ "qos_icmp",			V_01				},
	{ "qos_reset",			V_01				},
	{ "qos_pfifo",			V_01				}, // !!TB
	{ "qos_obw",			V_RANGE(10, 999999)	},
	{ "qos_ibw",			V_RANGE(10, 999999)	},
	{ "qos_orules",			V_LENGTH(0, 4096)	},
	{ "qos_default",		V_RANGE(0, 9)		},
	{ "qos_irates",			V_LENGTH(0, 128)	},
	{ "qos_orates",			V_LENGTH(0, 128)	},

	{ "ne_vegas",			V_01				},
	{ "ne_valpha",			V_NUM				},
	{ "ne_vbeta",			V_NUM				},
	{ "ne_vgamma",			V_NUM				},


/*
ppp_static			0/1
ppp_static_ip		IP
wl_enable			0/1
wl_wds_timeout
wl_maxassoc			1-256
wl_phytype			a,b,g
wl_net_reauth
wl_preauth
wl_wme_ap_bk
wl_wme_ap_be
wl_wme_ap_vi
wl_wme_ap_vo
wl_wme_sta_bk
wl_wme_sta_be
wl_wme_sta_vi
wl_wme_sta_vo
QoS
port_priority_1		0-2
port_flow_control_1	0,1
port_rate_limit_1	0-8
port_priority_2		0-2
port_flow_control_2	0,1
port_rate_limit_2	0-8
port_priority_3		0-2
port_flow_control_3	0,1
port_rate_limit_3	0-8
port_priority_4		0-2
port_flow_control_4	0,1
port_rate_limit_4	0-8
wl_ap_ip
wl_ap_ssid
*/

	{ NULL }
};

static int save_variables(int write)
{
	const nvset_t *v;
	char *p, *e;
	int n;
	long l;
	unsigned u[6];
	int ok;
	char s[256];
	int dirty;
	static const char *msgf = "The field \"%s\" is invalid. Please report this problem.";

	dirty = 0;
	for (v = nvset_list; v->name; ++v) {
//		_dprintf("[%s] %p\n", v->name, webcgi_get((char*)v->name));
		if ((p = webcgi_get((char*)v->name)) == NULL) continue;
		ok = 1;
		switch (v->vtype) {
		case VT_TEXT:
			p = unix_string(p);	// NOTE: p = malloc'd
			// drop
		case VT_LENGTH:
			n = strlen(p);
			if ((n < v->va.i) || (n > v->vb.i)) ok = 0;
			break;
		case VT_RANGE:
			l = strtol(p, &e, 10);
			if ((p == e) || (*e) || (l < v->va.l) || (l > v->vb.l)) ok = 0;
			break;
		case VT_IP:
			if ((sscanf(p, "%3u.%3u.%3u.%3u", &u[0], &u[1], &u[2], &u[3]) != 4) ||
				(u[0] > 255) || (u[1] > 255) || (u[2] > 255) || (u[3] > 255)) ok = 0;
			break;
		case VT_MAC:
			if ((sscanf(p, "%2x:%2x:%2x:%2x:%2x:%2x", &u[0], &u[1], &u[2], &u[3], &u[4], &u[5]) != 6) ||
				(u[0] > 255) || (u[1] > 255) || (u[2] > 255) || (u[3] > 255) || (u[4] > 255) || (u[5] > 255)) ok = 0;
			break;
		default:
			// shutup gcc
			break;
		}
		if (!ok) {
			if (v->vtype == VT_TEXT) free(p);

			sprintf(s, msgf, v->name);
			resmsg_set(s);
			return 0;
		}
		if (write) {
			if (!nvram_match((char *)v->name, p)) {
				if (v->vtype != VT_TEMP) dirty = 1;
				nvram_set(v->name, p);
			}
		}
		if (v->vtype == VT_TEXT) free(p);
	}


	// special cases

	char *p1, *p2;
	if (((p1 = webcgi_get("set_password_1")) != NULL) && (strcmp(p1, "**********") != 0)) {
		if (((p2 = webcgi_get("set_password_2")) != NULL) && (strcmp(p1, p2) == 0)) {
			if ((write) && (!nvram_match("http_passwd", p1))) {
				dirty = 1;
				nvram_set("http_passwd", p1);
			}
  		}
  		else {
			sprintf(s, msgf, "password");
			resmsg_set(s);
			return 0;
  		}
	}

	for (n = 0; n < 50; ++n) {
	    sprintf(s, "rrule%d", n);
	    if ((p = webcgi_get(s)) != NULL) {
	        if (strlen(p) > 2048) {
				sprintf(s, msgf, s);
				resmsg_set(s);
				return 0;
	        }
			if ((write) && (!nvram_match(s, p))) {
				dirty = 1;
				nvram_set(s, p);
			}
	    }
	}

	return (write) ? dirty : 1;
}

static void wo_tomato(char *url)
{
	char *v;
	int i;
	int ajax;
	int nvset;
	const char *red;
	int commit;

//	_dprintf("tomato.cgi\n");

	red = webcgi_safeget("_redirect", "");
	if (!*red) send_header(200, NULL, mime_html, 0);

	commit = atoi(webcgi_safeget("_commit", "1"));
	ajax = atoi(webcgi_safeget("_ajax", "0"));

	nvset = atoi(webcgi_safeget("_nvset", "1"));
	if (nvset) {
		if (!save_variables(0)) {
			if (ajax) {
				web_printf("@msg:%s", resmsg_get());
			}
			else {
				parse_asp("error.asp");
			}
			return;
		}
		commit = save_variables(1) && commit;

		resmsg_set("Settings saved.");
	}

	rboot = atoi(webcgi_safeget("_reboot", "0"));
	if (rboot) {
		parse_asp("reboot.asp");
	}
	else {
		if (ajax) {
			web_printf("@msg:%s", resmsg_get());
		}
		else if (atoi(webcgi_safeget("_moveip", "0"))) {
			parse_asp("saved-moved.asp");
		}
		else if (!*red) {
			parse_asp("saved.asp");
		}
	}

	if (commit) {
		_dprintf("commit from tomato.cgi\n");
		nvram_commit_x();
	}

	if ((v = webcgi_get("_service")) != NULL) {
		if (!*red) {
			if (ajax) web_printf(" Some services are being restarted...");
			web_close();
		}
		sleep(1);

		if (*v == '*') {
			kill(1, SIGHUP);
		}
		else if (*v != 0) {
			exec_service(v);
		}
	}

	for (i = atoi(webcgi_safeget("_sleep", "0")); i > 0; --i) sleep(1);

	if (*red) redirect(red);

	if (rboot) {
		web_close();
		sleep(1);
		kill(1, SIGTERM);
	}
}


// ----------------------------------------------------------------------------


static void wo_update(char *url)
{
	const aspapi_t *api;
	const char *name;
	int argc;
	char *argv[16];
	char s[32];

	if ((name = webcgi_get("exec")) != NULL) {
		for (api = aspapi; api->name; ++api) {
			if (strcmp(api->name, name) == 0) {
				for (argc = 0; argc < 16; ++argc) {
					sprintf(s, "arg%d", argc);
					if ((argv[argc] = (char *)webcgi_get(s)) == NULL) break;
				}
				api->exec(argc, argv);
				break;
			}
		}
	}
}

static void wo_service(char *url)
{
	int n;

	exec_service(webcgi_safeget("_service", ""));

	if ((n = atoi(webcgi_safeget("_sleep", "2"))) <= 0) n = 2;
	sleep(n);

	common_redirect();
}

/*
static void wo_logout(char *url)
{
	char s[256];

	// doesn't work with all browsers...

	if (((user_agent) && (strstr(user_agent, "Opera") != NULL))) {
		sprintf(s, "%llx", (unsigned long long)time(NULL) * rand());
		send_authenticate(s);
	}
	else {
		send_authenticate(NULL);
	}
}
*/

static void wo_shutdown(char *url)
{
	parse_asp("shutdown.asp");
	web_close();
	sleep(1);

	kill(1, SIGQUIT);
}

static void wo_nvcommit(char *url)
{
	parse_asp("saved.asp");
	web_close();
	nvram_commit();
}


