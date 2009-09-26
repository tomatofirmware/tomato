/*

	Copyright 2003, CyberTAN  Inc.
	All Rights Reserved.

	This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
	the contents of this file may not be disclosed to third parties,
	copied or duplicated in any form without the prior written
	permission of CyberTAN Inc.

	This software should be used as a reference only, and it not
	intended for production use!

	THIS SOFTWARE IS OFFERED "AS IS",	AND CYBERTAN GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE

*/
/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS",	AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include <string.h>
#include <bcmnvram.h>

#include <tomato_config.h>	//!!TB
#include "tomato_profile.h"
#include "defaults.h"

//! = see restore_main()

const defaults_t defaults[] = {
	{ "restore_defaults",	"0"				},	// Set to 0 to not restore defaults on boot

	// LAN H/W parameters
//!	{ "lan_ifname",			""				},	// LAN interface name
//!	{ "lan_ifnames",		""				},	// Enslaved LAN interfaces
	{ "lan_hwnames",		""				},	// LAN driver names (e.g. et0)
	{ "lan_hwaddr",			""				},	// LAN interface MAC address

	// LAN TCP/IP parameters
	{ "lan_dhcp",			"0"				},	// DHCP client [static|dhcp]
    { "lan_proto",			"dhcp"			},	// DHCP server [static|dhcp]  //Barry add 2004 09 16
	{ "lan_ipaddr",			"192.168.1.1"	},	// LAN IP address
	{ "lan_netmask",		"255.255.255.0"	},	// LAN netmask
	{ "lan_wins",			""				},	// x.x.x.x x.x.x.x ...
	{ "lan_domain",			""				},	// LAN domain name
	{ "lan_lease",			"86400"			},	// LAN lease time in seconds
	{ "lan_stp",			"0"				},	// LAN spanning tree protocol
	{ "lan_route",			""				},	// Static routes (ipaddr:netmask:gateway:metric:ifname ...)

    { "lan_gateway",		"0.0.0.0"		},	// LAN Gateway
	{ "wds_enable",			"0"				},	// WDS Enable (0|1)

	// WAN H/W parameters
//!	{ "wan_ifname",			""				},	// WAN interface name
//!	{ "wan_ifnames",		""				},	// WAN interface names
	{ "wan_hwname",			""				},	// WAN driver name (e.g. et1)
	{ "wan_hwaddr",			""				},	// WAN interface MAC address
	{ "wan_ifnameX",		NULL			},	// real wan if; see wan.c:start_wan

	// WAN TCP/IP parameters
	{ "wan_proto",			"dhcp"			},	// [static|dhcp|pppoe|disabled]
	{ "wan_ipaddr",			"0.0.0.0"		},	// WAN IP address
	{ "wan_netmask",		"0.0.0.0"		},	// WAN netmask
	{ "wan_gateway",		"0.0.0.0"		},	// WAN gateway
	{ "wan_dns",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan_wins",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan_lease",			"86400"			},	// WAN lease time in seconds
	{ "wan_islan",			"0"				},

	{ "wan_primary",		"1"				},	// Primary wan connection
	{ "wan_unit",			"0"				},	// Last configured connection

	// Filters
	{ "filter_maclist",		""				},	// xx:xx:xx:xx:xx:xx ...
	{ "filter_macmode",		"deny"			},	// "allow" only, "deny" only, or "disabled" (allow all)
	{ "filter_client0",		""				},	// [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,proto,enable,day_start-day_end,sec_start-sec_end,desc

	{ "filter",				"on"			},	// [on | off] Firewall Protection

	// Port forwards
	{ "autofw_port0",		""				},	// out_proto:out_port,in_proto:in_port0-in_port1>to_port0-to_port1,enable,desc

	// DHCP server parameters
	{ "dhcp_start",			"100"			},	//
	{ "dhcp_num",			"50"			},	//
	{ "dhcpd_startip",		"" 				},	// if empty, tomato will use dhcp_start/dchp_num for better compatibility
	{ "dhcpd_endip",		"" 				},	// "
	{ "dhcp_lease",			"0"				},	// LAN lease time in minutes
	{ "dhcp_domain",		"wan"			},	// Use WAN domain name first if available (wan|lan)
	{ "wan_get_dns",		""				},	// DNS IP address which get by dhcpc // Add


	// PPPoE parameters
	{ "pppoe_ifname",		""				},	// PPPoE enslaved interface
	{ "ppp_username",		""				},	// PPP username
	{ "ppp_passwd",			""				},	// PPP password
	{ "ppp_idletime",		"5"				},	// Dial on demand max idle time (mins)
	{ "ppp_keepalive",		"0"				},	// Restore link automatically
	{ "ppp_demand",			"0"				},	// Dial on demand
	{ "ppp_redialperiod",	"30"			},	// Redial Period  (seconds)*/
	{ "ppp_mru",			"1500"			},	// Negotiate MRU to this value
	{ "ppp_mtu",			"1500"			},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "ppp_service",		""				},	// PPPoE service name
	{ "ppp_ac",				""				},	// PPPoE access concentrator name
	{ "ppp_static",			"0"				},	// Enable / Disable Static IP
	{ "ppp_static_ip",		""				},	// PPPoE Static IP
	{ "ppp_get_ac",			""				},	// PPPoE Server ac name
	{ "ppp_get_srv",		""				},	// PPPoE Server service name

	{ "pppoe_lei",			""				},
	{ "pppoe_lef",			""				},

	// Wireless parameters
	{ "wl_ifname",			""				},	// Interface name
	{ "wl_hwaddr",			""				},	// MAC address
	{ "wl_phytype",			"g"				},	// Current wireless band ("a" (5 GHz), "b" (2.4 GHz), or "g" (2.4 GHz))	// Modify
	{ "wl_corerev",			""				},	// Current core revision
	{ "wl_phytypes",		""				},	// List of supported wireless bands (e.g. "ga")
	{ "wl_radioids",		""				},	// List of radio IDs
	{ "wl_ssid",			"wireless"		},	// Service set ID (network name)
	{ "wl_country",			"JP"		},		// Country (default obtained from driver)
	{ "wl_country_code",		"JP"		},		// !!TB - Country (default to JP to allow all 14 channels)
	{ "wl_radio",			"1"				},	// Enable (1) or disable (0) radio
	{ "wl_closed",			"0"				},	// Closed (hidden) network
    { "wl_ap_isolate",		"0"				},	// AP isolate mode
	{ "wl_mode",			"ap"			},	// AP mode (ap|sta|wds)
	{ "wl_lazywds",			"1"				},	// Enable "lazy" WDS mode (0|1)
	{ "wl_wds",				""				},	// xx:xx:xx:xx:xx:xx ...
	{ "wl_wds_timeout",		"1"				},	// WDS link detection interval defualt 1 sec*/
	{ "wl_wep",				"disabled"		},	// WEP data encryption (enabled|disabled)
	{ "wl_auth",			"0"				},	// Shared key authentication optional (0) or required (1)
	{ "wl_key",				"1"				},	// Current WEP key
	{ "wl_key1",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key2",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key3",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key4",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_maclist",			""				},	// xx:xx:xx:xx:xx:xx ...
	{ "wl_channel",			"6"				},	// Channel number
	{ "wl_rate",			"0"				},	// Rate (bps, 0 for auto)
	{ "wl_mrate",			"0"				},	// Mcast Rate (bps, 0 for auto)
	{ "wl_rateset",			"default"		},	// "default" or "all" or "12"
	{ "wl_frag",			"2346"			},	// Fragmentation threshold
	{ "wl_rts",				"2347"			},	// RTS threshold
	{ "wl_dtim",			"1"				},	// DTIM period (3.11.5)*/	// It is best value for WiFi test
	{ "wl_bcn",				"100"			},	// Beacon interval
	{ "wl_plcphdr",			"long"			},	// 802.11b PLCP preamble type
	{ "wl_net_mode",		"mixed"			},	// Wireless mode (mixed|g-only|b-only|disable)
	{ "wl_gmode",			"1"				},	// 54g mode
	{ "wl_gmode_protection","off"			},	// 802.11g RTS/CTS protection (off|auto)
	{ "wl_afterburner",		"off"			},	// AfterBurner
	{ "wl_frameburst",		"off"			},	// BRCM Frambursting mode (off|on)
	{ "wl_wme",				"off"			},	// WME mode (off|on)
	{ "wl_antdiv",			"-1"			},	// Antenna Diversity (-1|0|1|3)
	{ "wl_infra",			"1"				},	// Network Type (BSS/IBSS)
	{ "wl_btc_mode",		"0"				},	// !!TB - BT Coexistence Mode

	{ "wl_passphrase",		""				},	// Passphrase	// Add
	{ "wl_wep_bit",			"128"			},	// WEP encryption [64 | 128] // Add
	{ "wl_wep_buf",			""				},	// save all settings for web // Add
	{ "wl_wep_gen",			""				},	// save all settings for generate button	// Add
	{ "wl_wep_last",		""				},	// Save last wl_wep mode	// Add
	{ "wl_active_mac",		""				},	// xx:xx:xx:xx:xx:xx ...	// Add

	// WPA parameters
	{ "security_mode2",		"disabled"		},	// WPA mode (disabled|radius|wpa_personal|wpa_enterprise|wep|wpa2_personal|wpa2_enterprise) for WEB	// Add
	{ "security_mode",		"disabled"		},	// WPA mode (disabled|radius|wpa|psk|wep|psk psk2|wpa wpa2) for WEB	// Add
	{ "security_mode_last",	""				},	// Save last WPA mode	// Add
	{ "wl_auth_mode",		"none"			},	// Network authentication mode (radius|none)
	{ "wl_wpa_psk",			""				},	// WPA pre-shared key
	{ "wl_wpa_gtk_rekey",	"3600"			},	// WPA GTK rekey interval	// Modify
	{ "wl_radius_ipaddr",	""				},	// RADIUS server IP address
	{ "wl_radius_key",		""				},	// RADIUS shared secret
	{ "wl_radius_port",		"1812"			},	// RADIUS server UDP port
	{ "wl_crypto",			"tkip"			},	// WPA data encryption
	{ "wl_net_reauth",		"36000"			},	// Network Re-auth/PMK caching duration
	{ "wl_akm",				""				},	// WPA akm list

	// WME parameters
	// EDCA parameters for STA
	{ "wl_wme_sta_bk",		"15 1023 7 0 0 off"		},	// WME STA AC_BK paramters
	{ "wl_wme_sta_be",		"15 1023 3 0 0 off"		},	// WME STA AC_BE paramters
	{ "wl_wme_sta_vi",		"7 15 2 6016 3008 off"	},	// WME STA AC_VI paramters
	{ "wl_wme_sta_vo",		"3 7 2 3264 1504 off"	},	// WME STA AC_VO paramters

	// EDCA parameters for AP
	{ "wl_wme_ap_bk",		"15 1023 7 0 0 off"		},	// WME AP AC_BK paramters
	{ "wl_wme_ap_be",		"15 63 3 0 0 off"		},	// WME AP AC_BE paramters
	{ "wl_wme_ap_vi",		"7 15 1 6016 3008 off"	},	// WME AP AC_VI paramters
	{ "wl_wme_ap_vo",		"3 7 1 3264 1504 off"	},	// WME AP AC_VO paramters

	{ "wl_wme_no_ack",		"off"			},	// WME No-Acknowledgmen mode

	{ "wl_unit",			"0"				},	// Last configured interface
	{ "wl_mac_deny",		""				},	// filter MAC	// Add

	{ "wl_leddc",			"0x640000"		},	// !!TB - 100% duty cycle for LED on router (WLAN LED fix for some routers)
	{ "wl_bss_enabled",		"1"				},	// !!TB - If not present the new versions of wlconf may not bring up wlan
	{ "wl_reg_mode",		"off"			},	// !!TB - Regulatory: 802.11H(h)/802.11D(d)/off(off)

	{ "pptp_server_ip",		""				},	// as same as WAN gateway
	{ "pptp_get_ip",		""				},	// IP Address assigned by PPTP server

	// for firewall
	{ "mtu_enable",			"0"				},	// WAN MTU [1|0]
	{ "wan_mtu",			"1500"			},	// Negotiate MTU to the smaller of this value or the peer MRU

	{ "l2tp_server_ip",		""				},	// L2TP auth server (IP Address)
	{ "l2tp_get_ip",		""				},	// IP Address assigned by L2TP server
	{ "wan_gateway_buf",	"0.0.0.0"		},	// save the default gateway for DHCP
//	hbobs	{ "hb_server_ip",		""				},	// heartbeat auth server (IP Address)
//	hbobs	{ "hb_server_domain",	""				},	// heartbeat auth server (domain name)

// misc
	{ "t_noise",			"-99"			},
	{ "led_override",		""				},
	{ "btn_override",		""				},
	{ "btn_reset",			""				},
	{ "env_path",			""				},
	{ "manual_boot_nv",		"0"				},
	{ "wlx_hpamp",			""				},
//	{ "wlx_hperx",			""				},	//	see init.c
	{ "t_fix1",				""				},

// basic-ddns
	{ "ddnsx0",				""				},
	{ "ddnsx1",				""				},
	{ "ddnsx0_cache",		""				},
	{ "ddnsx1_cache",		""				},
	{ "ddnsx_save",			"1"				},
	{ "ddnsx_refresh",		"28"			},

// basic-network
	{ "wds_save",			""				},

// basic-ident
	{ "router_name",		"tomato"		},
	{ "wan_hostname",		"unknown"		},
	{ "wan_domain",			""				},

// basic-time
	{ "tm_sel",				"PST8PDT,M3.2.0/2,M11.1.0/2"	},
	{ "tm_tz",				"PST8PDT,M3.2.0/2,M11.1.0/2"	},
	{ "tm_dst",				"1",							},
	{ "ntp_updates",		"4"								},
	{ "ntp_tdod",			"0"								},
	{ "ntp_server",			"0.pool.ntp.org 1.pool.ntp.org 2.pool.ntp.org" },
	{ "ntp_kiss",			""								},
	{ "ntp_kiss_ignore",	""								},

// basic-static
	{ "dhcpd_static",		""				},

// basic-wfilter
	{ "wl_mac_list",		""				},
	{ "wl_macmode",			"disabled"		},
	{ "macnames",			""				},

// advanced-ctnf
	{ "ct_tcp_timeout",		""				},
	{ "ct_udp_timeout",		""				},
	{ "ct_max",				""				},
	{ "nf_ttl",				"0"				},
	{ "nf_l7in",			"1"				},
	{ "nf_rtsp",			"1"				},
	{ "nf_pptp",			"1"				},
	{ "nf_h323",			"1"				},
	{ "nf_ftp",				"1"				},

// advanced-mac
	{ "mac_wan",			""				},
	{ "mac_wl",				""				},

// advanced-misc
	{ "boot_wait",			"on"			},
	{ "wait_time",			"5"				},
	{ "wan_speed",			"4"				},	// 0=10 Mb Full, 1=10 Mb Half, 2=100 Mb Full, 3=100 Mb Half, 4=Auto

// advanced-dhcpdns
	{ "dhcpd_dmdns",		"1"				},
	{ "dhcpd_slt",			"0"				},
	{ "dhcpd_lmax",			""				},
	{ "dns_addget",			"0"				},
	{ "dns_intcpt",			"0"				},
	{ "dhcpc_minpkt",		"0"				},
	{ "dnsmasq_custom",		""				},
//	{ "dnsmasq_norw",		"0"				},

// advanced-firewall
//		{ "block_loopback",		"0"				},	// nat loopback
	{ "nf_loopback",		"1"				},
	{ "block_wan",			"1"				},	// block inbound icmp
	{ "multicast_pass",		"0"				},	// enable multicast proxy
	{ "ne_syncookies",		"0"				},	// tcp_syncookies
	{ "ne_shlimit",			"0,3,60"		},

// advanced-routing
	{ "routes_static",		""				},
	{ "wk_mode",			"gateway"		},	// Network mode [gateway|router]
	{ "dr_setting",			"0"				},	// [ Disable | WAN | LAN | Both ]
	{ "dr_lan_tx",			"0"				},	// Dynamic-Routing LAN out
	{ "dr_lan_rx",			"0"				},	// Dynamic-Routing LAN in
	{ "dr_wan_tx",			"0"				},	// Dynamic-Routing WAN out
	{ "dr_wan_rx",			"0"				},	// Dynamic-Routing WAN in

// advanced-wireless
	{ "wl_txant",			"3"				},
	{ "wl_txpwr",			"42"			},
	{ "wl_maxassoc",		"128"			},	// Max associations driver could support
	{ "wl_distance",		""				},

// forward-*
	{ "portforward",		"0<3<1.1.1.0/24<1000:2000<<192.168.1.2<ex: 1000 to 2000, restricted>0<2<<1000,2000<<192.168.1.2<ex: 1000 and 2000>0<1<<1000<2000<192.168.1.2<ex: different internal port>0<3<<1000:2000,3000<<192.168.1.2<ex: 1000 to 2000, and 3000>" },
	{ "trigforward",		"0<1<3000:4000<5000:6000<ex: open 5000-6000 if 3000-4000>"	},
	{ "dmz_enable",			"0"				},
	{ "dmz_ipaddr",			"0"				},
	{ "dmz_sip",			""				},

// forward-upnp
	{ "upnp_enable",		"1"				},
	{ "upnp_secure",		"1"				},
	{ "upnp_port",			"5000"			},
#if 0	// disabled for miniupnpd
	{ "upnp_ssdp_interval",	"60"			},	// SSDP interval
	{ "upnp_max_age",		"180"			},	// Max age
	{ "upnp_mnp",			"0"				},
	{ "upnp_config",		"0"				},
#endif

// qos
	{ "qos_enable",			"0"				},
	{ "qos_ack",			"1"				},
	{ "qos_syn",			"0"				},
	{ "qos_fin",			"0"				},
	{ "qos_rst",			"0"				},
	{ "qos_icmp",			"0"				},
	{ "qos_reset",			"0"				},
	{ "qos_obw",			"230"			},
	{ "qos_ibw",			"1000"			},
	{ "qos_orules",			"0<<6<d<80,443<0<<0:512<1<WWW>0<<6<d<80,443<0<<512:<3<WWW (512K+)>0<<-1<d<53<0<<0:2<0<DNS>0<<-1<d<53<0<<2:<4<DNS (2K+)>0<<-1<d<1024:65535<0<<<4<Bulk Traffic" },
	{ "qos_burst0",			""				},
	{ "qos_burst1",			""				},

	{ "qos_default",		"3"				},
	{ "qos_orates",			"80-100,10-100,5-100,3-100,2-95,1-50,1-40,1-30,1-20,1-10"	},

	{ "ne_vegas",			"0"				},	// TCP Vegas
	{ "ne_valpha",			"2"				},	// "
	{ "ne_vbeta",			"6"				},	// "
	{ "ne_vgamma",			"2"				},	// "

// access restrictions
	{ "rruleN",				"0"				},
	{ "rrule0",				"0|1320|300|31|||word text\n^begins-with.domain.\n.ends-with.net$\n^www.exact-domain.net$|0|example" },
//*	{ "rrule##",			""				},
	{ "rrulewp",			"80,8080"		},

#if TOMATO_SL
// samba
	{ "smbd_on",			"0"				},
	{ "nmbd_on",			"0"				},
	{ "smbd_wgroup",		"WORKGROUP"		},
	{ "smbd_nbname",		"TOMATO"		},
	{ "smbd_adminpass",		"admin"			},
#endif

// admin-access
	{ "http_username",		""				},	// Username
	{ "http_passwd",		"admin"			},	// Password
	{ "remote_management",	"0"				},	// Remote Management [1|0]
	{ "remote_mgt_https",	"0"				},	// Remote Management use https [1|0]
	{ "http_wanport",		"8080"			},	// WAN port to listen on
	{ "http_lanport",		"80"			},	// LAN port to listen on
	{ "https_lanport",		"443"			},	// LAN port to listen on
	{ "http_enable",		"1"				},	// HTTP server enable/disable
	{ "https_enable",		"0"				},	// HTTPS server enable/disable
	{ "https_crt_save",		"0"				},
	{ "https_crt_cn",		""				},
	{ "https_crt_file",		""				},
	{ "https_crt",			""				},
	{ "web_wl_filter",		"0"				},	// Allow/Deny Wireless Access Web
//	{ "web_favicon",		"0"				},
	{ "web_css",			"tomato"		},
	{ "web_svg",			"1"				},
	{ "telnetd_eas",		"1"				},
	{ "telnetd_port",		"23"			},
	{ "sshd_eas",			"0"				},
	{ "sshd_pass",			"1"				},
	{ "sshd_port",			"22"			},
	{ "sshd_remote",		"0"				},
	{ "sshd_rport",			"2222"			},
	{ "sshd_authkeys",		""				},
	{ "sshd_hostkey",		""				},
	{ "sshd_forwarding",		"1"				},
	{ "rmgt_sip",			""				},	// remote management: source ip address

	{ "http_id",			""				},

// admin-bwm
	{ "rstats_enable",		"1"				},
	{ "rstats_path",		""				},
	{ "rstats_stime",		"48"			},
	{ "rstats_offset",		"1"				},
	{ "rstats_data",		""				},
	{ "rstats_colors",		""				},
	{ "rstats_exclude",		""				},
	{ "rstats_sshut",		"1"				},
	{ "rstats_bak",			"0"				},

// advanced-buttons
	{ "sesx_b0",			"1"				},
	{ "sesx_b1",			"4"				},
	{ "sesx_b2",			"4"				},
	{ "sesx_b3",			"4"				},
	{ "sesx_script",
		"[ $1 -ge 20 ] && telnetd -p 233 -l /bin/sh\n"
	},
	{ "script_brau",
		"if [ ! -e /tmp/switch-start ]; then\n"
		"  # do something at startup\n"
		"  echo position at startup was $1 >/tmp/switch-start\n"
		"  exit\n"
		"fi\n"
		"if [ $1 = \"bridge\" ]; then\n"
		"  # do something\n"
		"  led bridge on\n"
		"elif [ $1 = \"auto\" ]; then\n"
		"  # do something\n"
		"  led bridge off\n"
		"fi\n"
	},

// admin-log
	{ "log_remote",			"0"				},
	{ "log_remoteip",		""				},
	{ "log_remoteport",		"514"			},
	{ "log_file",			"1"				},
	{ "log_limit",			"60"			},
	{ "log_in",				"0"				},
	{ "log_out",			"0"				},
	{ "log_mark",			"60"			},
	{ "log_events",			""				},

// admin-debugging
	{ "debug_nocommit",		"0"				},
	{ "debug_cprintf",		"0"				},
	{ "debug_cprintf_file",	"0"				},
//	{ "debug_keepfiles",	"0"				},
	{ "console_loglevel",	"1"				},
	{ "t_cafree",			"0"				},
	{ "t_hidelr",			"0"				},
	{ "debug_clkfix",		"1"				},
	{ "debug_ddns",			"0"				},

// admin-cifs
	{ "cifs1",				""				},
	{ "cifs2",				""				},

// admin-jffs2
	{ "jffs2_on",			"0"				},
	{ "jffs2_exec",			""				},

// nas-usb - !!TB
	{ "usb_enable",			"0"				},
	{ "usb_uhci",			"0"				},
	{ "usb_ohci",			"0"				},
	{ "usb_usb2",			"0"				},
	{ "usb_storage",		"0"				},
	{ "usb_printer",		"0"				},
	{ "usb_printer_bidirect",	"1"				},
	{ "usb_fs_ext3",		"0"				},
	{ "usb_fs_fat",			"0"				},
#ifdef TCONFIG_NTFS
	{ "usb_fs_ntfs",		"0"				},
#endif
	{ "usb_automount",		"0"				},
#if 0
	{ "usb_bdflush",		"30 500 0 0 100 100 60 0 0"	},
#endif
	{ "script_usbhotplug", 		""				},
	{ "script_usbmount", 		""				},
	{ "script_usbumount", 		""				},

#ifdef TCONFIG_FTP
// nas-ftp - !!TB
	{ "ftp_enable",			"0"				},
	{ "ftp_super",			"0"				},
	{ "ftp_anonymous",		"0"				},
	{ "ftp_dirlist",		"0"				},
	{ "ftp_port",			"21"				},
	{ "ftp_max",			"0"				},
	{ "ftp_ipmax",			"0"				},
	{ "ftp_staytimeout",		"300"				},
	{ "ftp_rate",			"0"				},
	{ "ftp_anonrate",		"0"				},
	{ "ftp_anonroot",		""				},
	{ "ftp_pubroot",		""				},
	{ "ftp_pvtroot",		""				},
	{ "ftp_users",			""				},
	{ "ftp_custom",			""				},
	{ "ftp_sip",			""				},	// wan ftp access: source ip address(es)
	{ "ftp_limit",			"0,3,60"			},
	{ "log_ftp",			"0"				},
#endif

#ifdef TCONFIG_SAMBASRV
// nas-samba - !!TB
	{ "smbd_enable",		"0"				},
	{ "smbd_wgroup",		"WORKGROUP"			},
	{ "smbd_cpage",			""				},
	{ "smbd_cset",			"utf8"				},
	{ "smbd_loglevel",		"0"				},
	{ "smbd_custom",		""				},
	{ "smbd_autoshare",		"1"				},
	{ "smbd_shares",
		"share</mnt<Default Share<0<0>root$</<Hidden Root<0<1"
	},
	{ "smbd_user",			"nas"				},
	{ "smbd_passwd",		""				},
#endif

// admin-sch
	{ "sch_rboot",			""				},
	{ "sch_rcon",			""				},
	{ "sch_c1",				""				},
	{ "sch_c2",				""				},
	{ "sch_c3",				""				},
	{ "sch_c1_cmd",			""				},
	{ "sch_c2_cmd",			""				},
	{ "sch_c3_cmd",			""				},

// admin-script
	{ "script_init",		""				},
	{ "script_shut",		""				},
	{ "script_fire",		""				},
	{ "script_wanup",		""				},

#if 0
// safe to remove?
	{ "QoS",					"0"			},

	{ "ses_enable",			"0"				},	// enable ses
	{ "ses_event",			"2"				},	// initial ses event
	{ "ses_led_assertlvl",	"0"				},	// For SES II
	{ "ses_client_join",	"0"				},	// For SES II
	{ "ses_sw_btn_status",	"DEFAULTS"		},	// Barry Adds 20050309 for SW SES BTN
	{ "ses_count",			"0"				},
	{ "eou_configured",		"0"				},

	{ "port_priority_1",		"0"			},	// port 1 priority; 1:high, 0:low
	{ "port_flow_control_1",	"1"			},	// port 1 flow control; 1:enable, 0:disable
	{ "port_rate_limit_1",		"0"			},	// port 1 rate limit: 256k,512k,1M,2M,5M,10M,20M,50M
	{ "port_priority_2",		"0"			},	// port 2 priority; 1:high, 0:low
	{ "port_flow_control_2",	"1"			},	// port 2 flow control; 1:enable, 0:disable
	{ "port_rate_limit_2",		"0" 		},	// port 2 rate limit: 256k,512k,1M,2M,5M,10M,20M,50M
	{ "port_priority_3",		"0"			},	// port 3 priority; 1:high, 0:low
	{ "port_flow_control_3",	"1"			},	// port 3 flow control; 1:enable, 0:disable
	{ "port_rate_limit_3",		"0"			},	// port 3 rate limit: 256k,512k,1M,2M,5M,10M,20M,50M
	{ "port_priority_4",		"0"			},	// port 4 priority; 1:high, 0:low
	{ "port_flow_control_4",	"1"			},	// port 4 flow control; 1:enable, 0:disable
	{ "port_rate_limit_4",		"0"			},	// port 4 rate limit: 256k,512k,1M,2M,5M,10M,20M,50M

//obs	zzz	{ "http_method",		"post"	},	// HTTP method

//	{ "wl_macmode1",		"disabled"		},

/* obsolete
	{ "filter",				"on"	},	// Firewall Protection [on|off]
	{ "ipsec_pass",			"1"	},	// IPSec Pass Through [1|0]
	{ "pptp_pass",			"1"	},	// PPTP Pass Through [1|0]
	{ "l2tp_pass",			"1"	},	// L2TP Pass Through [1|0]
	{ "block_cookie",		"0"	},	// Block Cookie [1|0]
	{ "ident_pass",			"0"	},	// IDENT passthrough [1|0]
	{ "block_proxy",		"0"	},	// Block Proxy [1|0]
*/

/* --- obsolete ---
	{ "forward_port",		""	},	// name:[on|off]:[tcp|udp|both]:wan_port>lan_ipaddr:lan_port0
	{ "port_trigger",		""	},	// name:[on|off]:[tcp|udp|both]:wan_port0-wan_port1>lan_port0-lan_port1

	// for mac clone
	{ "mac_clone_enable",	"0"	},	// User define WAN interface MAC address
	{ "def_hwaddr",	"00:00:00:00:00:00"	},	// User define WAN interface MAC address

	{ "public_ip",			""	},	// public ip
*/

//forced in rc.c	{ "os_name",			""	},	// OS name string
//forced in rc.c	{ "os_version",			EPI_VERSION_STR	},	// OS revision
//forced in rc.c	{ "os_date",			__DATE__	},	// OS date
//not used	{ "ct_modules",			""	},	// CyberTAN kernel modules
//obs	{ "timer_interval",		"3600"	},	// Timer interval in seconds
//obs	{ "ezc_enable",			"1"	},	// Enable EZConfig updates
//obs	{ "ezc_version",		EZC_VERSION_STR	},	// EZConfig version
//obs	{ "is_default",			"1"	},	// is it default setting: 1:yes 0:no*/
//obs	{ "os_server",			""	},	// URL for getting upgrades
//obs	{ "stats_server",		""	},	// URL for posting stats	-- used by httpd/stats.c
//obs	{ "router_disable",		"0"	},	// lan_proto=static lan_stp=0 wan_proto=disabled
//obs	{ "fw_disable",			"0"	},	// Disable firewall (allow new connections from the WAN)
//obs	{ "static_route",		""	},	// Static routes (ipaddr:netmask:gateway:metric:ifname ...)
//obs	{ "static_route_name",	""	},	// Static routes name ($NAME:name)
//	{ "filter_port",		""				},	// [lan_ipaddr|*]:lan_port0-lan_port1
	//{ "dhcp_end",			"150"			},	// Last assignable DHCP address	// Remove
//zzz	not used	{ "dhcp_wins",			"wan"	},	// Use WAN WINS first if available (wan|lan)
	//{ "eou_device_id",	""				},
	//{ "eou_public_key",	""	},
	//{ "eou_private_key",	""	},
	//{ "eou_public",		"b49b5ec6866f5b166cc058110b20551d4fe7a5c96a9b5f01a3929f40015e4248359732b7467bae4948d6bb62f96996a7122c6834311c1ea276b35d12c37895501c0f5bd215499cf443d580b999830ac620ac2bf3b7f912741f54fea17627d13a92f44d014030d5c8d3249df385f500ffc90311563e89aa290e7c6f06ef9a6ec311"	},
	//{ "eou_private",		"1fdf2ed7bd5ef1f4e603d34e4d41f0e70e19d1f65e1b6b1e6828eeed2d6afca354c0543e75d9973a1be9a898fed665e13f713f90bd5f50b3421fa7034fabde1ce63c44d01a5489765dc4dc3486521163bf6288db6c5e99c44bbb0ad7494fef20148ad862662dabcbff8dae7b466fad087d9f4754e9a6c84bc9adcbda7bc22e59"	},
 	{ "eou_expired_hour",	"72"	},     //The expired time is 72 hours, and this value = 72 * 10*/
//	{ "ntp_enable",			"1"	},	// replaced with ntp_updates
//	{ "ntp_mode",			"auto"	},	// auto, manual


	// for AOL
	{ "aol_block_traffic",	"0"				},	// 0:Disable 1:Enable for global
	{ "aol_block_traffic1",	"0"				},	// 0:Disable 1:Enable for "ppp_username"
	{ "aol_block_traffic2",	"0"				},	// 0:Disable 1:Enable for "Parental control"
	{ "skip_amd_check",		"0"				},	// 0:Disable 1:Enable
	{ "skip_intel_check",	"0"				},	// 0:Disable 1:Enable

// advanced-watchdog
	{ "wd_en",				""				},
	{ "wd_atp0",			""				},
	{ "wd_atp1",			""				},
	{ "wd_atp2",			""				},
	{ "wd_atp3",			""				},
	{ "wd_atp4",			""				},
	{ "wd_mxr",				"3"				},
	{ "wd_rdy",				"15"			},
	{ "wd_cki",				"300"			},
	{ "wd_fdm",				""				},
	{ "wd_aof",				""				},

#endif	// 0

	{ NULL, NULL	}
};

const defaults_t if_generic[] = {
	{ "lan_ifname",		"br0"					},
	{ "lan_ifnames",	"eth0 eth2 eth3 eth4"	},
	{ "wan_ifname",		"eth1"					},
	{ "wan_ifnames",	"eth1"					},
	{ NULL, NULL }
};

const defaults_t if_vlan[] = {
	{ "lan_ifname",		"br0"					},
	{ "lan_ifnames",	"vlan0 eth1 eth2 eth3"	},
	{ "wan_ifname",		"vlan1"					},
	{ "wan_ifnames",	"vlan1"					},
	{ NULL, NULL }
};
