/*
 * tow.c
 *
 * Copyright (C) 2014 bwq518, Hyzoom
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <rc.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <sys/stat.h>

#define MAX_PORTS 64
#define PORT_SIZE 16

#define SCRIPT_INIT_DIR 	"/etc/tow"
#define SCRIPT_INIT_DNSMASQ 	"/etc/tow/tow1_dnsmasqc"
#define SCRIPT_INIT_SHADOWSOCKS "/etc/tow/tow2_shadowsocks"
#define SCRIPT_INIT_SHADOWSOCKS_REDIR "/etc/tow/tow2_shadowsocks_redir"
#define SCRIPT_INIT_REDSOCKS 	"/etc/tow/tow3_redsocks2"
#define SCRIPT_INIT_PDNSD 	"/etc/tow/tow4_pdnsd"
#define SCRIPT_INIT_OBSSH 	"/etc/tow/tow5_obssh"
#define SCRIPT_WANUP 		"/etc/config/tow-hyzoom.wanup"
#define SCRIPT_FIRE 		"/etc/config/tow-hyzoom.fire"
#define DNSMASQ_CUSTOM_DIR	"/etc/dnsmasq/custom"
#define DNSMASQ_CUSTOM_MYCFG	"/etc/dnsmasq/custom/my.cfg"
#define DNSMASQ_CUSTOM_GFWLIST	"/etc/dnsmasq/custom/gfwlist.cfg"
#define DNSMASQ_CUSTOM_TUNLR	"/etc/dnsmasq/custom/tunlr.cfg"
#define DNSMASQ_CUSTOM_GFWLIST_ADD	"/etc/dnsmasq/custom/gfwlist_add.cfg"
#define DNSMASQ_CUSTOM_TUNLR	"/etc/dnsmasq/custom/tunlr.cfg"
#define DNSMASQ_CUSTOM_WHITELIST	"/etc/dnsmasq/custom/whitelist.cfg"
#define DNSMASQ_CUSTOM_WHITELIST_ADD	"/etc/dnsmasq/custom/whitelist_add.cfg"
#define DNSMASQ_TUNLR_DIR 	"/etc/dnsmasq/tunlr"
#define DNSMASQ_TUNLR_DOMAIN 	"/etc/dnsmasq/tunlr/domains.txt"
#define CONF_PDNSD 		"/etc/pdnsd.conf"
#define CONF_REDSOCKS 		"/etc/redsocks.conf"
#define CONF_SHADOWSOCKS 	"/etc/shadowsocks.json"
#define CONF_SHADOWSOCKS_REDIR 	"/etc/shadowsocks.json"
#define TOWCHECK		"/usr/bin/towcheck"

#define WGET "/usr/bin/wgetpro  -q --no-check-certificate --dns-timeout=60 --connect-timeout=60 --read-timeout=60 --tries=3 --wait=5"
#define ELOG "logger -t DNSMASQ_RENEW -s"

void stop_dnsmasqc(void)
{
	mkdir_if_none(SCRIPT_INIT_DIR);
	//remove my config of dnsmasq
	xstart(TOWCHECK,"delcru","dnsmasqc");
	unlink(DNSMASQ_CUSTOM_MYCFG);
	unlink(DNSMASQ_CUSTOM_GFWLIST);
	unlink(DNSMASQ_CUSTOM_GFWLIST_ADD);
	unlink(DNSMASQ_CUSTOM_TUNLR);
	unlink(DNSMASQ_CUSTOM_WHITELIST);
	unlink(DNSMASQ_CUSTOM_WHITELIST_ADD);
	unlink(DNSMASQ_TUNLR_DOMAIN);
	xstart("service", "dnsmasq", "restart");
	
	//Waiting for dnsmasq done
	char sleep_str[16];
	int sleep_sec;
	strcpy(sleep_str, nvram_safe_get("tow_sleep"));
	sleep_sec = atoi(sleep_str);
	if (sleep_sec <=1 || sleep_sec >= 60) sleep_sec = 10;
	sleep(sleep_sec);
}

void start_dnsmasqc(void)
{
	FILE *fps;
	char url[512], s1[4096], s2[4096];
	int i, j;

	mkdir_if_none(DNSMASQ_CUSTOM_DIR);
	mkdir_if_none(DNSMASQ_TUNLR_DIR);
	mkdir_if_none(SCRIPT_INIT_DIR);
	
	if (!(fps = fopen(DNSMASQ_CUSTOM_MYCFG, "wt")))
	{
		perror(DNSMASQ_CUSTOM_MYCFG);
		return;
	}
	fprintf(fps, "#Author: punkdm@gmail.com\n");
	fprintf(fps, "#Description: generate dnsmasq conf from smarthosts file\n");
	fprintf(fps, "#UPDATE:13-04-05 23:14\n");
	fprintf(fps, "## Some Tweak\n");
	fprintf(fps, "#cache-size=8192\n");
	fprintf(fps, "#stop-dns-rebind\n");
	fprintf(fps, "#log-async=5\n");
	fprintf(fps, "## Use pdnsd Only\n");
	fprintf(fps, "no-resolv\n");
	//5454 is the port between dnsmasq and pdnsd
	fprintf(fps, "server=/#/127.0.0.1#5454\n");
	fprintf(fps, "bogus-priv\n");
	fprintf(fps, "domain-needed\n");
	fprintf(fps, "filterwin2k\n");
	fprintf(fps, "no-hosts\n");
	fprintf(fps, "neg-ttl=3600\n");
	fclose(fps);

	//create /etc/dnsmasq/tunlr/domains.txt
	if (!(fps = fopen(DNSMASQ_TUNLR_DOMAIN, "wt")))
	{
		perror(DNSMASQ_TUNLR_DOMAIN);
		return;
	}
	fprintf(fps, "abc.com\n");
	fprintf(fps, "adnxs.com\n");
	fprintf(fps, "adultswim.com\n");
	fprintf(fps, "aertv.ie\n");
	fprintf(fps, "atdmt.com\n");
	fprintf(fps, "au.tv.yahoo.com\n");
	fprintf(fps, "bbc.co.uk\n");
	fprintf(fps, "blinkbox.com\n");
	fprintf(fps, "blockbuster.com\n");
	fprintf(fps, "brightcove.com\n");
	fprintf(fps, "cbs.com\n");
	fprintf(fps, "channel4.com\n");
	fprintf(fps, "channel5.com\n");
	fprintf(fps, "chartbeat.com\n");
	fprintf(fps, "cinemanow.com\n");
	fprintf(fps, "cpxadroit.com\n");
	fprintf(fps, "cpxinteractive.com\n");
	fprintf(fps, "crackle.com\n");
	fprintf(fps, "ctv.ca\n");
	fprintf(fps, "cwtv.com\n");
	fprintf(fps, "demdex.net\n");
	fprintf(fps, "diynetwork.com\n");
	fprintf(fps, "elogua.com\n");
	fprintf(fps, "foodnetwork.com\n");
	fprintf(fps, "fox.com\n");
	fprintf(fps, "fwmrm.net\n");
	fprintf(fps, "nhl.com\n");
	fprintf(fps, "go.com\n");
	fprintf(fps, "hgtv.com\n");
	fprintf(fps, "history.com\n");
	fprintf(fps, "hulu.com\n");
	fprintf(fps, "iheart.com\n");
	fprintf(fps, "imrworldwide.com\n");
	fprintf(fps, "inskinmedia.com\n");
	fprintf(fps, "ip2location.com\n");
	fprintf(fps, "itv.com\n");
	fprintf(fps, "js.revsci.net\n");
	fprintf(fps, "kidlet.tv\n");
	fprintf(fps, "last.fm\n");
	fprintf(fps, "lastfm.es\n");
	fprintf(fps, "livepass.conviva.com\n");
	fprintf(fps, "liverail.com\n");
	fprintf(fps, "local.yahooapis.com\n");
	fprintf(fps, "mlb.tv\n");
	fprintf(fps, "mog.com\n");
	fprintf(fps, "mtv.com\n");
	fprintf(fps, "mtvnservices.com\n");
	fprintf(fps, "mylifetime.com\n");
	fprintf(fps, "mytv.taiseng.com\n");
	fprintf(fps, "nbc.com\n");
	fprintf(fps, "nbcsports.msnbc.com\n");
	fprintf(fps, "nbcuni.com\n");
	fprintf(fps, "netflix.com\n");
	fprintf(fps, "netflix.net\n");
	fprintf(fps, "nflxext.com\n");
	fprintf(fps, "nflximg.com\n");
	fprintf(fps, "nhl.com\n");
	fprintf(fps, "pandora.com\n");
	fprintf(fps, "pbs.com\n");
	fprintf(fps, "pbs.org\n");
	fprintf(fps, "quantserve.com\n");
	fprintf(fps, "rdio.com\n");
	fprintf(fps, "recaptcha.net\n");
	fprintf(fps, "rhapsody.com\n");
	fprintf(fps, "roku.com\n");
	fprintf(fps, "rokulabs.com\n");
	fprintf(fps, "rte.ie\n");
	fprintf(fps, "rubiconproject.com\n");
	fprintf(fps, "scorecardresearch.com\n");
	fprintf(fps, "scrippscontroller.com\n");
	fprintf(fps, "scrippsnetworks.com\n");
	fprintf(fps, "sharethis.com\n");
	fprintf(fps, "southparkstudios.com\n");
	fprintf(fps, "sportsnet.ca\n");
	fprintf(fps, "sony.net\n");
	fprintf(fps, "tbs.com\n");
	fprintf(fps, "theplatform.com\n");
	fprintf(fps, "thewb.com\n");
	fprintf(fps, "tnt.tv\n");
	fprintf(fps, "trutv.com\n");
	fprintf(fps, "turntable.fm\n");
	fprintf(fps, "tv3.ie\n");
	fprintf(fps, "tv.com\n");
	fprintf(fps, "universalsports.com\n");
	fprintf(fps, "usanetwork.com\n");
	fprintf(fps, "vevo.com\n");
	fprintf(fps, "vh1.com\n");
	fprintf(fps, "video.uk.msn.com\n");
	fprintf(fps, "vudu.com\n");
	fprintf(fps, "zattoo.com\n");
	//add domains from nvram "tow_tunlr_domains_add"
	strcpy(s2, nvram_safe_get("tow_tunlr_domains_add"));
	filter_space(s2);
	trimstr(s2);
	bzero(s1, sizeof(s1));
	for (i = 0; i < strlen(s2) ; i ++)
		if ( s2[i] == ';' || s2[i] == ',') s2[i] = ' ';
	shrink_space(s1, s2, sizeof(s2));
	for (i = 0; i < strlen(s1) ; i ++)
		if ( s1[i] == ' ') s1[i] = '\n';
	nvram_set("tow_tunlr_domains_add",s1);
	fprintf(fps, s1);
	fclose(fps);

	//Create a text file for display tunrl domains list through Web UI
	if (!(fps = fopen("/www/ext/tunlr_domains.htm", "wt")))
	{
		perror("/www/ext/tunlr_domains.htm");
		return;
	}
	fprintf(fps, "<html>\n<head>\n");
	fprintf(fps, "<meta http-equiv='content-type' content='text/html;charset=utf-8'>\n");
	fprintf(fps, "<style type='text/css'>\n");
	fprintf(fps, "body {\n");
	fprintf(fps, "background-color: #1C3845;\n");
	fprintf(fps, "background-repeat: repeat-y;\n");
	fprintf(fps, "background-position: center;\n");
	fprintf(fps, "font-family: Tahoma, Arial, sans-serif, 微软雅黑, 宋体;\n");
	fprintf(fps, "font-size: 14px;\n");
	fprintf(fps, "color: #ffffff;\n}\n");
	fprintf(fps, "</style>\n</head>\n<body>\n<br>");
	fprintf(fps,"The following domains are already in tunlr domains list.<br>\n");
	fprintf(fps,"以下域名已经存在于tunlr的列表中.<br><br>\n\n");
	fprintf(fps, "abc.com<br>\n");
	fprintf(fps, "adnxs.com<br>\n");
	fprintf(fps, "adultswim.com<br>\n");
	fprintf(fps, "aertv.ie<br>\n");
	fprintf(fps, "atdmt.com<br>\n");
	fprintf(fps, "au.tv.yahoo.com<br>\n");
	fprintf(fps, "bbc.co.uk<br>\n");
	fprintf(fps, "blinkbox.com<br>\n");
	fprintf(fps, "blockbuster.com<br>\n");
	fprintf(fps, "brightcove.com<br>\n");
	fprintf(fps, "cbs.com<br>\n");
	fprintf(fps, "channel4.com<br>\n");
	fprintf(fps, "channel5.com<br>\n");
	fprintf(fps, "chartbeat.com<br>\n");
	fprintf(fps, "cinemanow.com<br>\n");
	fprintf(fps, "cpxadroit.com<br>\n");
	fprintf(fps, "cpxinteractive.com<br>\n");
	fprintf(fps, "crackle.com<br>\n");
	fprintf(fps, "ctv.ca<br>\n");
	fprintf(fps, "cwtv.com<br>\n");
	fprintf(fps, "demdex.net<br>\n");
	fprintf(fps, "diynetwork.com<br>\n");
	fprintf(fps, "elogua.com<br>\n");
	fprintf(fps, "foodnetwork.com<br>\n");
	fprintf(fps, "fox.com<br>\n");
	fprintf(fps, "fwmrm.net<br>\n");
	fprintf(fps, "nhl.com<br>\n");
	fprintf(fps, "go.com<br>\n");
	fprintf(fps, "hgtv.com<br>\n");
	fprintf(fps, "history.com<br>\n");
	fprintf(fps, "hulu.com<br>\n");
	fprintf(fps, "iheart.com<br>\n");
	fprintf(fps, "imrworldwide.com<br>\n");
	fprintf(fps, "inskinmedia.com<br>\n");
	fprintf(fps, "ip2location.com<br>\n");
	fprintf(fps, "itv.com<br>\n");
	fprintf(fps, "js.revsci.net<br>\n");
	fprintf(fps, "kidlet.tv<br>\n");
	fprintf(fps, "last.fm<br>\n");
	fprintf(fps, "lastfm.es<br>\n");
	fprintf(fps, "livepass.conviva.com<br>\n");
	fprintf(fps, "liverail.com<br>\n");
	fprintf(fps, "local.yahooapis.com<br>\n");
	fprintf(fps, "mlb.tv<br>\n");
	fprintf(fps, "mog.com<br>\n");
	fprintf(fps, "mtv.com<br>\n");
	fprintf(fps, "mtvnservices.com<br>\n");
	fprintf(fps, "mylifetime.com<br>\n");
	fprintf(fps, "mytv.taiseng.com<br>\n");
	fprintf(fps, "nbc.com<br>\n");
	fprintf(fps, "nbcsports.msnbc.com<br>\n");
	fprintf(fps, "nbcuni.com<br>\n");
	fprintf(fps, "netflix.com<br>\n");
	fprintf(fps, "netflix.net<br>\n");
	fprintf(fps, "nflxext.com<br>\n");
	fprintf(fps, "nflximg.com<br>\n");
	fprintf(fps, "nhl.com<br>\n");
	fprintf(fps, "pandora.com<br>\n");
	fprintf(fps, "pbs.com<br>\n");
	fprintf(fps, "pbs.org<br>\n");
	fprintf(fps, "quantserve.com<br>\n");
	fprintf(fps, "rdio.com<br>\n");
	fprintf(fps, "recaptcha.net<br>\n");
	fprintf(fps, "rhapsody.com<br>\n");
	fprintf(fps, "roku.com<br>\n");
	fprintf(fps, "rokulabs.com<br>\n");
	fprintf(fps, "rte.ie<br>\n");
	fprintf(fps, "rubiconproject.com<br>\n");
	fprintf(fps, "scorecardresearch.com<br>\n");
	fprintf(fps, "scrippscontroller.com<br>\n");
	fprintf(fps, "scrippsnetworks.com<br>\n");
	fprintf(fps, "sharethis.com<br>\n");
	fprintf(fps, "southparkstudios.com<br>\n");
	fprintf(fps, "sportsnet.ca<br>\n");
	fprintf(fps, "sony.net<br>\n");
	fprintf(fps, "tbs.com<br>\n");
	fprintf(fps, "theplatform.com<br>\n");
	fprintf(fps, "thewb.com<br>\n");
	fprintf(fps, "tnt.tv<br>\n");
	fprintf(fps, "trutv.com<br>\n");
	fprintf(fps, "turntable.fm<br>\n");
	fprintf(fps, "tv3.ie<br>\n");
	fprintf(fps, "tv.com<br>\n");
	fprintf(fps, "universalsports.com<br>\n");
	fprintf(fps, "usanetwork.com<br>\n");
	fprintf(fps, "vevo.com<br>\n");
	fprintf(fps, "vh1.com<br>\n");
	fprintf(fps, "video.uk.msn.com<br>\n");
	fprintf(fps, "vudu.com<br>\n");
	fprintf(fps, "zattoo.com<br>\n");
	fprintf(fps, "</body>\n</html>");
	fclose(fps);

	//Create tunlr, gfwlist, whitelist for dnsmasq
	if (!(fps = fopen(SCRIPT_INIT_DNSMASQ, "wt")))
	{
		perror(SCRIPT_INIT_DNSMASQ);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	fprintf(fps, "# Script for initializing dnsmasq configurations.\n");
	fprintf(fps, "# Credit by PunkDM. Created by bwq518.\n# bwq518@gmail.com\n\n");
	fprintf(fps, "alias wget=\"%s\"\n", WGET);
	fprintf(fps, "alias base64=/bin/base64\n");
	fprintf(fps, "alias elog=\"logger -t DNSMASQ_RENEW -s\"\n\n");
	fprintf(fps, "SHR=0\n\n");
	fprintf(fps, "# Tunlr DNS updater for dnsmasq. 2013, Alexander Ryzhov\n");
	fprintf(fps, "# Adapted For asuswrt-merlin firmware.\n");
	fprintf(fps, "tunlr() {\n");
	fprintf(fps, "    echo \"Tunlr DNS updater started\"\n\n");
	fprintf(fps, "    touch \"/etc/dnsmasq/custom/tunlr.cfg\"\n");
	fprintf(fps, "    touch \"/etc/dnsmasq/tunlr/domains.txt\"\n");
	fprintf(fps, "    local DNSMASQ_CONF='/etc/dnsmasq/custom/tunlr.cfg'\n");
	fprintf(fps, "    local DOMAINS=$(cat /etc/dnsmasq/tunlr/domains.txt)\n");
	if (nvram_match("tow_tunlr_custom_enable","1"))
	{
		strcpy(s1, nvram_safe_get("tow_tunlr_custom"));
		for (i = 0; i < strlen(s1) ; i ++)
			if ( s1[i] == ';' || s1[i] == ',') s1[i] = ' ';
		trimstr(s1);
		shrink_space(s2, s1, sizeof(s1));
		nvram_set("tow_tunlr_custom",s2);
		fprintf(fps, "    local IPS=\"%s\"\n", s2);
	}
	else
	{
		fprintf(fps, "    local IPS=$(wget -O - \\\n");
		strcpy(url, nvram_safe_get("tow_tunlr_url"));
		trimstr(url);
		fprintf(fps, "        \"%s\" \\\n", url);
		fprintf(fps, "        | sed \"s/\\\"dns.\\\"://g\" | sed \"s/[{}]//g\" | sed \"s/,/\\ /g\" |sed \"s/\\\"//g\")\n");
		fprintf(fps, "    if [ -z \"$IPS\" ] || [ -n \"$(echo $IPS | sed 's/[0-9\\.\\ ]//g')\" ] ; then\n");
		fprintf(fps, "        echo \"Tunlr DNS addresses not retrieved. default will be used.\"\n");
		strcpy(s1, nvram_safe_get("tow_tunlr_custom"));
		for (i = 0; i < strlen(s1) ; i ++)
			if ( s1[i] == ';' || s1[i] == ',') s1[i] = ' ';
		trimstr(s1);
		shrink_space(s2, s1, sizeof(s1));
		nvram_set("tow_tunlr_custom",s2);
		fprintf(fps, "        local IPS=\"%s\"\n", s2);
		fprintf(fps, "    fi\n");
	}
	fprintf(fps, "    if [ X\"$IPS\" = X ]; then\n");
	fprintf(fps, "        return\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    echo -n > $DNSMASQ_CONF\n");
	fprintf(fps, "    echo \"# Tunlr DNS updater for dnsmasq\" >> $DNSMASQ_CONF\n");
	fprintf(fps, "    for domain in $DOMAINS\n");
	fprintf(fps, "    do\n");
	fprintf(fps, "        for dns in $IPS\n");
	fprintf(fps, "        do\n");
	fprintf(fps, "            echo \"server=/${domain}/${dns}\" >> $DNSMASQ_CONF\n");
	fprintf(fps, "        done\n");
	fprintf(fps, "    done\n");
	fprintf(fps, "    SHR=1\n");
	fprintf(fps, "    FILESIZE=$(wc -c $DNSMASQ_CONF | awk '{print $1}')\n");
	fprintf(fps, "    if [ ! -s $DNSMASQ_CONF ] || [ $FILESIZE -lt 4000 ]; then\n");
	fprintf(fps, "        cd /etc/dnsmasq/custom\n");
	fprintf(fps, "        rm -f tunlr.cfg\n");
	fprintf(fps, "        cp -f /usr/local/tow/tunlr.cfg.gz .\n");
	fprintf(fps, "        gzip -d tunlr.cfg.gz\n");
	fprintf(fps, "        elog \"tunlr.cfg is too small. default file will be used.\"\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    echo 'done.'\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "gfwlist() {\n");
	fprintf(fps, "    local GLF=\"/etc/dnsmasq/custom/gfwlist.cfg\"\n");
	fprintf(fps, "    cd /tmp\n");
	strcpy(url, nvram_safe_get("tow_gfwlist_url"));
	trimstr(url);
	fprintf(fps, "    wget %s -O gfwlist.txt\n", url);
	fprintf(fps, "    if [ \"$?\" != \"0\" ] || [ ! -s gfwlist.txt ] || [ $(cat gfwlist.txt | grep \"http-equiv\" | wc -l) -ge 1 ]; then\n");
	fprintf(fps, "        elog \"gfwlist file not retrieved. Default gfwlist.cfg will be used.\"\n");
	fprintf(fps, "        T_PWD=`pwd -P`\n");
	fprintf(fps, "        cd /etc/dnsmasq/custom\n");
	fprintf(fps, "        if [ ! -s gfwlist.cfg ]; then\n");
	fprintf(fps, "            rm -f gfwlist.cfg\n");
	fprintf(fps, "            cp -f /usr/local/tow/gfwlist.cfg.gz .\n");
	fprintf(fps, "            gzip -d gfwlist.cfg.gz\n");
	fprintf(fps, "        fi\n");
	fprintf(fps, "        cd $T_PWD\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        base64 -d gfwlist.txt > gfwlist.dec     #Base64 decode\n");
	fprintf(fps, "        sed -i '1,20d' gfwlist.dec              #remove Restricted site\n");
	fprintf(fps, "        grep '^@@' gfwlist.dec > gfwlist.direct #direct connect\n");
	fprintf(fps, "        grep '^||' gfwlist.dec > gfwlist.p1     #pattern 1:start with \"||\"\n");
	fprintf(fps, "        grep '^\\.' gfwlist.dec > gfwlist.p2     #pattern 2:start with \".\"\n");
	fprintf(fps, "        grep '^|[^|]' gfwlist.dec > gfwlist.p3  #pattern 3:start with single \"|\"\n");
	fprintf(fps, "        grep -v '^|\\|^$\\|^@\\|^\\.\\|^\\[\\|^!' gfwlist.dec > gfwlist.p4 #pattern 4:others\n");
	fprintf(fps, "        echo 'converting gfwlist to domain list...'\n");
	fprintf(fps, "        sed -i 's/^||//g' gfwlist.p1    #remove the leading \"||\"\n");
	fprintf(fps, "        sed -i 's/\\/.*$//g' gfwlist.p1  #remove anything after \"/\", including \"/\"\n");
	fprintf(fps, "        sed -i 's/\\^//g' gfwlist.p1     #remove \"^\" if any, due to gfwlist's flawed lines\n");
	fprintf(fps, "        sed -i 's/^\\.//g' gfwlist.p2    #remove the leading \".\"\n");
	fprintf(fps, "        sed -i 's/^google[\\.\\*].*//g' gfwlist.p2        #remove lines start with google. or google*\n");
	fprintf(fps, "        sed -i 's/\\/.*//g' gfwlist.p2   #remove anything after \"/\", including \"/\"\n");
	fprintf(fps, "        sed -i 's/|http:\\/\\///g' gfwlist.p3     #remove prefix\n");
	fprintf(fps, "        sed -i 's/|https:\\/\\///g' gfwlist.p3    #remove prefix\n");
	fprintf(fps, "        sed -i 's/\\/.*$//g' gfwlist.p3  #remove .....\n");
	fprintf(fps, "        sed -i 's/^\\*\\.//g' gfwlist.p3\n");
	fprintf(fps, "        grep '\\.' gfwlist.p4 > gfwlist.tmp      #remove lines contain no domain\n");
	fprintf(fps, "        mv gfwlist.tmp gfwlist.p4\n");
	fprintf(fps, "        sed -i 's/\\/.*$//g' gfwlist.p4  #remove....\n");
	fprintf(fps, "        sed -i 's/^google.*$//g' gfwlist.p4     #remove lines start with google\n");
	fprintf(fps, "        grep -v '\\.wikipedia\\.org.*' gfwlist.p4 > gfwlist.tmp   #remove wikipedia lines\n");
	fprintf(fps, "        mv gfwlist.tmp gfwlist.p4\n");
	fprintf(fps, "        cp gfwlist.p1 domainlist.tmp\n");
	fprintf(fps, "        echo '\n");
	fprintf(fps, "        '>> domainlist.tmp\n");
	fprintf(fps, "        sed 's/^[[:space:]]*//g' gfwlist.p2 | cat | sed '/^$/d' >> domainlist.tmp\n");
	fprintf(fps, "        echo '\n");
	fprintf(fps, "        '>> domainlist.tmp\n");
	fprintf(fps, "        sed 's/^[[:space:]]*//g' gfwlist.p3 | cat | sed '/^$/d' >> domainlist.tmp\n");
	fprintf(fps, "        echo '\n");
	fprintf(fps, "        '>> domainlist.tmp\n");
	fprintf(fps, "        sed 's/^[[:space:]]*//g' gfwlist.p4 | cat | sed '/^$/d' >> domainlist.tmp\n");
	fprintf(fps, "        sort domainlist.tmp | uniq > domainlist.txt\n");
	fprintf(fps, "        sed -i '/^[[:space:]]*$/d' domainlist.txt       #delete blank line\n");
	fprintf(fps, "        grep '\\*' domainlist.txt > domainlist.special\n");
	fprintf(fps, "        grep '^|' domainlist.txt >> domainlist.special\n");
	fprintf(fps, "        sed -i '/\\*\\|^|/d' domainlist.txt\n");
	fprintf(fps, "        sed -i -e \"s/.*/ipset=\\/&\\/gfwlist/\" domainlist.txt\n");
	fprintf(fps, "        rm domainlist.tmp\n");
	fprintf(fps, "        rm domainlist.special\n");
	fprintf(fps, "        rm gfwlist.*\n");
	fprintf(fps, "        mv -f /tmp/domainlist.txt $GLF\n");
	//fprintf(fps, "        #lanternlist\n");
	//fprintf(fps, "        local TMPLISTFILE=\"/tmp/lanternlist.txt\"\n");
	//strcpy(url, nvram_safe_get("tow_lanternlist_url"));
	//trimstr(url);
	//fprintf(fps, "        wget %s -O $TMPLISTFILE\n", url);
	//fprintf(fps, "        echo 'Converting lanternlist to domain list...'\n");
	//fprintf(fps, "        sed -i '1,10d' $TMPLISTFILE\n");
	//fprintf(fps, "        sed -i \"s/.*/ipset=\\/&\\/gfwlist/\" $TMPLISTFILE\n");
	//fprintf(fps, "        [ -e $GLF ] && cat $TMPLISTFILE >> $GLF\n");
	//fprintf(fps, "        sort $GLF| uniq > $TMPLISTFILE\n");
	//fprintf(fps, "        mv -f $TMPLISTFILE $GLF\n");
	fprintf(fps, "        sed -i '1s/^.*$/#### GFWList Domain for IPSET ####/' $GLF\n");
	fprintf(fps, "        FILESIZE=$(wc -c $GLF | awk '{print $1}')\n");
	fprintf(fps, "        if [ ! -s $GLF ] || [ $FILESIZE -lt 60000 ]; then\n");
	fprintf(fps, "            cd /etc/dnsmasq/custom\n");
	fprintf(fps, "            rm -f gfwlist.cfg\n");
	fprintf(fps, "            cp -f /usr/local/tow/gfwlist.cfg.gz .\n");
	fprintf(fps, "            gzip -d gfwlist.cfg.gz\n");
	fprintf(fps, "            elog \"gfwlist.cfg is too small. default file will be used.\"\n");
	fprintf(fps, "        fi\n");
	fprintf(fps, "        echo 'done.'\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "whitelist() {\n");
	fprintf(fps, "    local WLF=\"/etc/dnsmasq/custom/whitelist.cfg\"\n");
	fprintf(fps, "    cd /tmp\n");
	strcpy(url, nvram_safe_get("tow_whitelist_url"));
	trimstr(url);
	fprintf(fps, "    wget %s -O whitelist.txt\n", url);
	fprintf(fps, "    if [ \"$?\" != \"0\" ] || [ ! -s whitelist.txt ] || [ $(cat whitelist.txt | grep \"http-equiv\" | wc -l) -ge 1 ]; then\n");
	fprintf(fps, "        elog \"whitelist file not retrieved. Default whitelist.cfg will be used.\"\n");
	fprintf(fps, "        T_PWD=`pwd -P`\n");
	fprintf(fps, "        cd /etc/dnsmasq/custom\n");
	fprintf(fps, "        if [ ! -s whitelist.cfg ]; then\n");
	fprintf(fps, "            rm -f whitelist.cfg\n");
	fprintf(fps, "            cp -f /usr/local/tow/whitelist.cfg.gz .\n");
	fprintf(fps, "            gzip -d whitelist.cfg.gz\n");
	fprintf(fps, "        fi\n");
	fprintf(fps, "        cd $T_PWD\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        sed -i 's/ //g' whitelist.txt        #remove all space\n");
	fprintf(fps, "        grep '^\"\\.' whitelist.txt > whitelist.tmp #get all domain line\n");
	fprintf(fps, "        echo 'converting whitelist to domain list...'\n");
	fprintf(fps, "        sed -i 's/^\"\\.//g' whitelist.tmp   #remove the leading \"\".\"\n");
	fprintf(fps, "        sed -i 's/\\/.*$//g' whitelist.tmp  #remove anything after \"/\", including \"/\"\n");
	fprintf(fps, "        sed -i 's/\\\"//g' whitelist.tmp     #remove \"\"\" if any, due to gfwlist's flawed lines\n");
	fprintf(fps, "        sed -i 's/\\,//g' whitelist.tmp     #remove \",\" if any, due to gfwlist's flawed lines\n");
	fprintf(fps, "        sort whitelist.tmp | uniq > dnswhitelist.txt\n");
	fprintf(fps, "        sed -i -e \"s/.*/ipset=\\/&\\/whitelist/\" dnswhitelist.txt\n");
	fprintf(fps, "        rm whitelist.*\n");
	fprintf(fps, "        mv -f /tmp/dnswhitelist.txt $WLF\n");
	fprintf(fps, "        #pandalist\n");
	fprintf(fps, "        local TMPLISTFILE=\"/tmp/panda_list.txt\"\n");
	strcpy(url, nvram_safe_get("tow_pandalist_url"));
	trimstr(url);
	fprintf(fps, "        wget -O $TMPLISTFILE %s\n", url);
	fprintf(fps, "        if [ \"$?\" != \"0\" ]; then\n");
	fprintf(fps, "            elog \"pandalist file not retrieved, skip it.\"\n");
	fprintf(fps, "        else\n");
	fprintf(fps, "            echo 'Converting pandalist to domain list...'\n");
	fprintf(fps, "            sed -i 's/\\[//g' $TMPLISTFILE\n");
	fprintf(fps, "            sed -i 's/\\]//g' $TMPLISTFILE\n");
	fprintf(fps, "            sed -i 's/, /\\n/g' $TMPLISTFILE\n");
	fprintf(fps, "            sed -i \"s/'//g\" $TMPLISTFILE\n");
	fprintf(fps, "            sed -i \"s/.*/ipset=\\/&\\/whitelist/\" $TMPLISTFILE\n");
	fprintf(fps, "            [ -e $WLF ] && cat $TMPLISTFILE >> $WLF\n");
	fprintf(fps, "            sort $WLF | uniq > $TMPLISTFILE\n");
	fprintf(fps, "            mv -f $TMPLISTFILE $WLF\n");
	fprintf(fps, "            sed -i '1 i\\###China Domain WhiteList for IPSET###' $WLF\n");
	fprintf(fps, "            [ -e $TMPLISTFILE ] && rm -f $TMPLISTFILE\n");
	fprintf(fps, "            echo 'done.'\n");
	fprintf(fps, "        fi\n");
	fprintf(fps, "        FILESIZE=$(wc -c $WLF | awk '{print $1}')\n");
	fprintf(fps, "        if [ ! -s $WLF ] || [ $FILESIZE -lt 120000 ]; then\n");
	fprintf(fps, "            cd /etc/dnsmasq/custom\n");
	fprintf(fps, "            rm -f $WLF\n");
	fprintf(fps, "            cp -f /usr/local/tow/whitelist.cfg.gz .\n");
	fprintf(fps, "            gzip -d whitelist.cfg.gz\n");
	fprintf(fps, "            elog \"whitelist.cfg is too small. default file will be used.\"\n");
	fprintf(fps, "        fi\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "tunlr\n");
	fprintf(fps, "gfwlist\n");
	fprintf(fps, "whitelist\n\n");
	fprintf(fps, "if [ $SHR -eq 1 ]\n");
	fprintf(fps, "    then\n");
	fprintf(fps, "        service dnsmasq restart\n");
	fprintf(fps, "        elog \"DNSMASQ restart success, new Configs and Hosts working now...\"\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        elog \"DNSMASQ update failed, old Configs and Hosts working now...\"\n");
	fprintf(fps, "fi\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru dnsmasqc\n", TOWCHECK);
	fclose(fps);

	//gfwlist_add.cfg
	if (!(fps = fopen(DNSMASQ_CUSTOM_GFWLIST_ADD, "wt")))
	{
		perror(DNSMASQ_CUSTOM_GFWLIST_ADD);
		return;
	}
	strcpy(s1, nvram_safe_get("tow_gfwlist_add"));
	for (i = 0; i < strlen(s1) ; i ++)
		if ( s1[i] == ';' || s1[i] == ',') s1[i] = ' ';
	trimstr(s1);
	shrink_space(s2, s1, sizeof(s1));
	for (i = 0; i < strlen(s2) ; i ++)
		if ( s2[i] == ' ') s2[i] = '\n';
	nvram_set("tow_gfwlist_add", s2);
	j = 0;
	bzero(s1, sizeof(s1));
	for (i = 0; i < strlen(s2); i ++)
	{
		if (s2[i] == '\n')
		{
			s1[j] = '\0';
			trimstr(s1);
			if(strlen(s1) > 0) fprintf(fps, "ipset=/%s/gfwlist\n", s1);
			j = 0;
			bzero(s1, sizeof(s1));
		}
		else
		{
			s1[j] = s2[i];
			j ++;
		}
	}
	//process the last line
	s2[j] = '\0';
	trimstr(s1);
	if(strlen(s1) > 0) fprintf(fps, "ipset=/%s/gfwlist\n", s1);
	fclose(fps);
	
	//whitelist_add.cfg
	if (!(fps = fopen(DNSMASQ_CUSTOM_WHITELIST_ADD, "wt")))
	{
		perror(DNSMASQ_CUSTOM_WHITELIST_ADD);
		return;
	}
	strcpy(s1, nvram_safe_get("tow_whitelist_add"));
	for (i = 0; i < strlen(s1) ; i ++)
		if ( s1[i] == ';' || s1[i] == ',') s1[i] = ' ';
	trimstr(s1);
	shrink_space(s2, s1, sizeof(s1));
	for (i = 0; i < strlen(s2) ; i ++)
		if ( s2[i] == ' ') s2[i] = '\n';
	nvram_set("tow_whitelist_add", s2);
	j = 0;
	bzero(s1, sizeof(s1));
	for (i = 0; i < strlen(s2); i ++)
	{
		if (s2[i] == '\n')
		{
			s1[j] = '\0';
			trimstr(s1);
			if(strlen(s1) > 0) fprintf(fps, "ipset=/%s/whitelist\n", s1);
			j = 0;
			bzero(s1, sizeof(s1));
		}
		else
		{
			s1[j] = s2[i];
			j ++;
		}
	}
	//process the last line
	s2[j] = '\0';
	trimstr(s1);
	if(strlen(s1) > 0) fprintf(fps, "ipset=/%s/whitelist\n", s1);
	fclose(fps);

	// restart dnsmasq
	chmod(SCRIPT_INIT_DNSMASQ, 0755);
	xstart(SCRIPT_INIT_DNSMASQ);

	//Waiting for dnsmasq done
	char sleep_str[16];
	int sleep_sec;
	strcpy(sleep_str, nvram_safe_get("tow_sleep"));
	sleep_sec = atoi(sleep_str);
	if (sleep_sec <=1 || sleep_sec >= 60) sleep_sec = 10;
	sleep(sleep_sec);
	eval("ln", "-sf","/etc/dnsmasq/custom/gfwlist.cfg", "/www/ext/gfwlist.cfg.txt");
	eval("ln", "-sf","/etc/dnsmasq/custom/whitelist.cfg", "/www/ext/whitelist.cfg.txt");

	return;
}

void stop_pdnsd(void)
{
	FILE *fp;

	//stop file
	mkdir_if_none(SCRIPT_INIT_DIR);
	if( !( fp = fopen( "/etc/tow/stop_pdnsd.sh", "w" ) ) )
	{
		perror( "/etc/tow/stop_pdnsd.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL pdnsd\n");
	fprintf( fp, "rm -f /var/run/pdnsd.pid\n");
	fprintf( fp, "logger \"pdnsd successfully stoped\" \n");
	fprintf( fp, "sleep 1\n");
	fprintf( fp, "%s delcru pdnsd\n", TOWCHECK);
	fclose( fp );
	chmod( "/etc/tow/stop_pdnsd.sh", 0755 );
	eval( "/etc/tow/stop_pdnsd.sh" );
	return;
}
void start_pdnsd(void)
{
	FILE *fps;
	char s1[256], s2[4096];
	int i, j, len;

	//ensure pdnsd stopped
	mkdir_if_none(SCRIPT_INIT_DIR);
	stop_pdnsd();

	//create pdnsd configuration file
	if (!(fps = fopen(CONF_PDNSD, "wt")))
	{
		perror(CONF_PDNSD);
		return;
	}
	fprintf(fps, "// pdnsd configuration file.\n\n");
	fprintf(fps, "global {\n");
	fprintf(fps, "	daemon = on;\n");
	fprintf(fps, "	perm_cache = 2048;\n");
	fprintf(fps, "	ctl_perms = 0666;\n");
	fprintf(fps, "	cache_dir = \"/var/cache/pdnsd\";\n");
	fprintf(fps, "	pid_file = \"/var/run/pdnsd.pid\";\n");
	fprintf(fps, "	run_as = \"root\";\n");
	fprintf(fps, "	server_ip = 127.0.0.1;\n");
	//5454 is a listening port for dnsmasq connection.
	fprintf(fps, "	server_port = 5454;\n");
	fprintf(fps, "	status_ctl = on;\n");
	fprintf(fps, "	paranoid = on;\n");
	fprintf(fps, "	query_method = udp_only;\n");
	fprintf(fps, "	min_ttl = 8h;\n");
	fprintf(fps, "	max_ttl = 1d;\n");
	fprintf(fps, "	timeout = 10;\n");
	fprintf(fps, "	neg_domain_pol = on;\n");
	fprintf(fps, "	udpbufsize = 1024;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "\n");
	fprintf(fps, "server {\n");
	fprintf(fps, "	label = \"ISP\";\n");
	//if use ISP dns from wan
	if (nvram_match("tow_pdnsd_localdns_useisp", "1")) 
	{
		//If have wan_dns, use it first and ignore wan_get_dns
		//if have not wan_dns, then use wan_get_dns
		strcpy(s2, nvram_safe_get("wan_dns"));
		if (strlen(trimstr(s2)) == 0) strcpy(s2, nvram_safe_get("wan_get_dns"));
		shrink_space(s1, s2, sizeof(s2));
		trimstr(s1);
		for (i = 0; i < strlen(s1); i ++) if (s1[i] == ' ') s1[i] = ',';
	}
	//else use specified dns on Web UI
	else strcpy(s1, nvram_safe_get("tow_pdnsd_localdns_ip"));
	if(strlen(trimstr(s1)) == 0) strcpy(s1, "114.114.114.114,114.114.115.115");
	trimstr(s1);
	fprintf(fps, "	ip = %s;\n", s1);
	fprintf(fps, "	port = 53;\n");
	fprintf(fps, "	timeout = 5;\n");
	fprintf(fps, "	reject = 118.5.49.6,\n");
	fprintf(fps, "            128.121.126.139,\n");
	fprintf(fps, "            141.101.114.4,\n");
	fprintf(fps, "            141.101.115.4,\n");
	fprintf(fps, "            159.106.121.75,\n");
	fprintf(fps, "            169.132.13.103,\n");
	fprintf(fps, "            188.5.4.96,\n");
	fprintf(fps, "            189.163.17.5,\n");
	fprintf(fps, "            190.93.244.4,\n");
	fprintf(fps, "            190.93.245.4,\n");
	fprintf(fps, "            190.93.246.4,\n");
	fprintf(fps, "            190.93.247.4,\n");
	fprintf(fps, "            192.67.198.6,\n");
	fprintf(fps, "            197.4.4.12,\n");
	fprintf(fps, "            202.106.1.2,\n");
	fprintf(fps, "            202.181.7.85,\n");
	fprintf(fps, "            203.161.230.171,\n");
	fprintf(fps, "            203.98.7.65,\n");
	fprintf(fps, "            207.12.88.98,\n");
	fprintf(fps, "            208.56.31.43,\n");
	fprintf(fps, "            209.145.54.50,\n");
	fprintf(fps, "            209.220.30.174,\n");
	fprintf(fps, "            209.36.73.33,\n");
	fprintf(fps, "            209.85.229.138,\n");
	fprintf(fps, "            211.94.66.147,\n");
	fprintf(fps, "            213.169.251.35,\n");
	fprintf(fps, "            216.221.188.182,\n");
	fprintf(fps, "            216.234.179.13,\n");
	fprintf(fps, "            23.89.5.60,\n");
	fprintf(fps, "            243.185.187.39,\n");
	fprintf(fps, "            249.129.46.48,\n");
	fprintf(fps, "            253.157.14.165,\n");
	fprintf(fps, "            37.61.54.158,\n");
	fprintf(fps, "            4.36.66.178,\n");
	fprintf(fps, "            46.82.174.68,\n");
	fprintf(fps, "            49.2.123.56,\n");
	fprintf(fps, "            54.76.135.1,\n");
	fprintf(fps, "            59.24.3.173,\n");
	fprintf(fps, "            64.33.88.161,\n");
	fprintf(fps, "            64.33.99.47,\n");
	fprintf(fps, "            64.66.163.251,\n");
	fprintf(fps, "            65.104.202.252,\n");
	fprintf(fps, "            65.160.219.113,\n");
	fprintf(fps, "            66.45.252.237,\n");
	fprintf(fps, "            69.55.52.253,\n");
	fprintf(fps, "            72.14.205.104,\n");
	fprintf(fps, "            72.14.205.99,\n");
	fprintf(fps, "            74.125.127.102,\n");
	fprintf(fps, "            74.125.155.102,\n");
	fprintf(fps, "            74.125.39.102,\n");
	fprintf(fps, "            74.125.39.113,\n");
	fprintf(fps, "            77.4.7.92,\n");
	fprintf(fps, "            78.16.49.15,\n");
	fprintf(fps, "            8.7.198.45,\n");
	fprintf(fps, "            93.46.8.89");
	//add IPs from nvram "tow_pdnsd_reject_ip_add"
	strcpy(s1, nvram_safe_get("tow_pdnsd_reject_ip_add"));
	for (i = 0; i < strlen(s1) ; i ++)
		if ( s1[i] == ';' || s1[i] == ',') s1[i] = ' ';
	trimstr(s1);
	shrink_space(s2, s1, sizeof(s1));
	for (i = 0; i < strlen(s2) ; i ++)
		if ( s2[i] == ' ') s2[i] = '\n';
	nvram_set("tow_pdnsd_reject_ip_add", s2);
	len = strlen(s2);
	bzero(s1, sizeof(s1));
	j = 0;
	for (i = 0; i < len ; i ++)
	{
		if ( s2[i] == '\n')
		{
			s1[j] = '\0';
			if (strlen(s1) >= 7) // should longer than the minimum length of 1.1.1.1
			{
				trimstr(s1);
				if( s1[strlen(s1) -1 ] == ',' || s1[strlen(s1) -1 ] == ';') s1[strlen(s1) -1 ] = '\0';
				fprintf(fps, ",\n              %s",s1);
				j = 0;
				bzero(s1, sizeof(s1));
			}
		}
		else
		{
			s1[j] = s2[i];
			j ++;
		}
	}
	//process the last line
	s1[j] = '\0';
	if (strlen(s1) >= 7) fprintf(fps, ",\n              %s",s1);
	fprintf(fps, ";\n");
	fprintf(fps, "	reject_policy = fail;\n");
	fprintf(fps, "  timeout = 3;\n");
	fprintf(fps, "  uptest=query;\n");
	fprintf(fps, "  interval=10m;\n");
	fprintf(fps, "  purge_cache=on;\n");
	fprintf(fps, "  edns_query=yes;\n");
	fprintf(fps, "  caching=on;\n");
	fprintf(fps, "  proxy_only=on;\n");
	fprintf(fps, "  lean_query=on;\n");
	strcpy(s2, nvram_safe_get("tow_pdnsd_exclude_domain"));
	filter_space(s2);
	trimstr(s2);
	fprintf(fps, "	exclude = %s;\n", s2);
	fprintf(fps, "}\n\n");
	fprintf(fps, "server {\n");
	fprintf(fps, "	label = \"OpenDNS\";\n");
	strcpy(s2, nvram_safe_get("tow_pdnsd_opendns_ip"));
	trimstr(s2);
	if(strlen(s2) == 0) strcpy(s2, "208.67.222.222,208.67.220.220");
	fprintf(fps, "	ip = %s;\n",s2);
	strcpy(s2, nvram_safe_get("tow_pdnsd_opendns_port"));
	trimstr(s2);
	fprintf(fps, "	port = %s;\n",s2);
	fprintf(fps, "  timeout = 5;\n");
	fprintf(fps, "  uptest=query;\n");
	fprintf(fps, "  interval=10m;\n");
	fprintf(fps, "  purge_cache=on;\n");
	fprintf(fps, "  edns_query=yes;\n");
	fprintf(fps, "  caching=on;\n");
	fprintf(fps, "  proxy_only=on;\n");
	fprintf(fps, "  lean_query=on;\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "source {\n");
	fprintf(fps, "	owner=localhost;\n");
	fprintf(fps, "	file=\"/etc/hosts\";\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "rr {\n");
	fprintf(fps, "	name=localhost;\n");
	fprintf(fps, "	reverse=on;\n");
	fprintf(fps, "	a=127.0.0.1;\n");
	fprintf(fps, "	owner=localhost;\n");
	fprintf(fps, "	soa=localhost,root.localhost,42,86400,900,86400,86400;\n");
	fprintf(fps, "}\n\n");
	fclose(fps);

	//Create a text file for display reject IP list through Web UI
	if (!(fps = fopen("/www/ext/pdnsd_reject_ip.htm", "wt")))
	{
		perror("/www/ext/pdnsd_reject_ip.htm");
		return;
	}
	fprintf(fps, "<html>\n<head>\n");
	fprintf(fps, "<meta http-equiv='content-type' content='text/html;charset=utf-8'>\n");
	fprintf(fps, "<style type='text/css'>\n");
	fprintf(fps, "body {\n");
	fprintf(fps, "background-color: #1C3845;\n");
	fprintf(fps, "background-repeat: repeat-y;\n");
	fprintf(fps, "background-position: center;\n");
	fprintf(fps, "font-family: Tahoma, Arial, sans-serif, 微软雅黑, 宋体;\n");
	fprintf(fps, "font-size: 14px;\n");
	fprintf(fps, "color: #ffffff;\n}\n");
	fprintf(fps, "</style>\n</head>\n<body>\n<br>");
	fprintf(fps,"The following IPs are already in rejested list.<br>\n");
	fprintf(fps,"以下IP地址已经存在于pdnsd的阻止列表中.<br><br>\n\n");
	fprintf(fps, "118.5.49.6<br>\n");
	fprintf(fps, "128.121.126.139<br>\n");
	fprintf(fps, "141.101.114.4<br>\n");
	fprintf(fps, "141.101.115.4<br>\n");
	fprintf(fps, "159.106.121.75<br>\n");
	fprintf(fps, "169.132.13.103<br>\n");
	fprintf(fps, "188.5.4.96<br>\n");
	fprintf(fps, "189.163.17.5<br>\n");
	fprintf(fps, "190.93.244.4<br>\n");
	fprintf(fps, "190.93.245.4<br>\n");
	fprintf(fps, "190.93.246.4<br>\n");
	fprintf(fps, "190.93.247.4<br>\n");
	fprintf(fps, "192.67.198.6<br>\n");
	fprintf(fps, "197.4.4.12<br>\n");
	fprintf(fps, "202.106.1.2<br>\n");
	fprintf(fps, "202.181.7.85<br>\n");
	fprintf(fps, "203.161.230.171<br>\n");
	fprintf(fps, "203.98.7.65<br>\n");
	fprintf(fps, "207.12.88.98<br>\n");
	fprintf(fps, "208.56.31.43<br>\n");
	fprintf(fps, "209.145.54.50<br>\n");
	fprintf(fps, "209.220.30.174<br>\n");
	fprintf(fps, "209.36.73.33<br>\n");
	fprintf(fps, "209.85.229.138<br>\n");
	fprintf(fps, "211.94.66.147<br>\n");
	fprintf(fps, "213.169.251.35<br>\n");
	fprintf(fps, "216.221.188.182<br>\n");
	fprintf(fps, "216.234.179.13<br>\n");
	fprintf(fps, "23.89.5.60<br>\n");
	fprintf(fps, "243.185.187.39<br>\n");
	fprintf(fps, "249.129.46.48<br>\n");
	fprintf(fps, "253.157.14.165<br>\n");
	fprintf(fps, "37.61.54.158<br>\n");
	fprintf(fps, "4.36.66.178<br>\n");
	fprintf(fps, "46.82.174.68<br>\n");
	fprintf(fps, "49.2.123.56<br>\n");
	fprintf(fps, "54.76.135.1<br>\n");
	fprintf(fps, "59.24.3.173<br>\n");
	fprintf(fps, "64.33.88.161<br>\n");
	fprintf(fps, "64.33.99.47<br>\n");
	fprintf(fps, "64.66.163.251<br>\n");
	fprintf(fps, "65.104.202.252<br>\n");
	fprintf(fps, "65.160.219.113<br>\n");
	fprintf(fps, "66.45.252.237<br>\n");
	fprintf(fps, "69.55.52.253<br>\n");
	fprintf(fps, "72.14.205.104<br>\n");
	fprintf(fps, "72.14.205.99<br>\n");
	fprintf(fps, "74.125.127.102<br>\n");
	fprintf(fps, "74.125.155.102<br>\n");
	fprintf(fps, "74.125.39.102<br>\n");
	fprintf(fps, "74.125.39.113<br>\n");
	fprintf(fps, "77.4.7.92<br>\n");
	fprintf(fps, "78.16.49.15<br>\n");
	fprintf(fps, "8.7.198.45<br>\n");
	fprintf(fps, "93.46.8.89<br>\n");
	fprintf(fps, "</body>\n</html>");
	fclose(fps);

	//Create init script for starting pdnsd
	if (!(fps = fopen(SCRIPT_INIT_PDNSD, "wt")))
	{
		perror(SCRIPT_INIT_PDNSD);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	fprintf(fps, "BIN=pdnsd\n");
	fprintf(fps, "BINPATH=\"/usr/sbin/\"\n");
	fprintf(fps, "CONF=\"-c %s\"\n", CONF_PDNSD);
	fprintf(fps, "CACHEPATH=\"/var/cache/pdnsd\"\n");
	fprintf(fps, "CACHEFILE=\"${CACHEPATH}/pdnsd.cache\"\n");
	fprintf(fps, "alias elog=\"logger -t $BIN -s\"\n");
	fprintf(fps, "COND=$1\n");
	fprintf(fps, "[ $# -eq 0 ] && COND=\"start\"\n");
	fprintf(fps, "\n");
	fprintf(fps, "case $COND in\n");
	fprintf(fps, "stop)\n");
	fprintf(fps, "    elog \"Stopping $BIN... \"\n");
	fprintf(fps, "    [ -n \"`pidof $BIN`\" ] && kill -15 `pidof $BIN`\n");
	fprintf(fps, "    elog \"$BIN Stopped. \"\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "start)\n");
	fprintf(fps, "    elog \"Starting $BIN... \"\n");
	fprintf(fps, "    if [ -n \"`pidof $BIN`\" ]\n");
	fprintf(fps, "    then\n");
	fprintf(fps, "        elog \"$BIN already running.\"\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        [ ! -d $CACHEPATH ] && { mkdir -p $CACHEPATH; chmod 777 $CACHEPATH; }\n");
	fprintf(fps, "        touch $CACHEFILE\n");
	fprintf(fps, "        ${BINPATH}${BIN} ${CONF}\n");
	fprintf(fps, "        elog \"$BIN started.\"\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "restart)\n");
	fprintf(fps, "    elog \"Restarting $BIN... \"\n");
	fprintf(fps, "    ct=0\n");
	fprintf(fps, "    while [ $ct -ne 2 ]\n");
	fprintf(fps, "    do\n");
	fprintf(fps, "        [ -n \"`pidof $BIN`\" ] && kill -15 `pidof $BIN`\n");
	fprintf(fps, "        [ ! -d $CACHEPATH ] && { mkdir -p $CACHEPATH; chmod 777 $CACHEPATH; }\n");
	fprintf(fps, "        touch $CACHEFILE\n");
	fprintf(fps, "        sleep 1\n");
	fprintf(fps, "        ${BINPATH}${BIN} ${CONF}\n");
	fprintf(fps, "        sleep 1\n");
	fprintf(fps, "        ct=`netstat -ln | grep \"5454\" | wc -l`\n");
	fprintf(fps, "    done\n");
	fprintf(fps, "    [ -n \"`pidof $BIN`\" ] && { elog \"$BIN Restart Success. \"; echo \\\n");
	fprintf(fps, "    \"$BIN Restart Success. \"; } || elog \"$BIN Restart failed\"\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "*)\n");
	fprintf(fps, "    elog \"Usage: $0 (start|stop|restart)\"\n");
	fprintf(fps, "    exit 1\n");
	fprintf(fps, "esac\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru pdnsd\n", TOWCHECK);
	fclose(fps);

	chmod(SCRIPT_INIT_PDNSD, 0755);
	xstart(SCRIPT_INIT_PDNSD, "start");
}

void stop_ssh_ofc(void)
{
	FILE *fp;

	//stop file
	mkdir_if_none(SCRIPT_INIT_DIR);
	if( !( fp = fopen( "/etc/tow/stop_ssh_ofc.sh", "w" ) ) )
	{
		perror( "/etc/tow/stop_ssh_ofc.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL ssh_ofc\n");
	fprintf( fp, "rm -f /var/run/ssh_ofc.pid\n");
	fprintf( fp, "logger \"ssh_ofc successfully stoped\" \n");
	fprintf( fp, "sleep 1\n");
	fprintf( fp, "%s delcru ssh_ofc\n", TOWCHECK);
	fclose( fp );
	chmod( "/etc/tow/stop_ssh_ofc.sh", 0755 );
	eval( "/etc/tow/stop_ssh_ofc.sh" );
	return;
}
void start_ssh_ofc(void)
{
	FILE *fps;
	char s1[256], s2[256];

	//ensure ssh_ofc stopped
	stop_ssh_ofc();

	//Create init script for starting ssh_ofc
	if (!(fps = fopen(SCRIPT_INIT_OBSSH, "wt")))
	{
		perror(SCRIPT_INIT_OBSSH);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	strcpy(s1, nvram_safe_get("tow_ssh_server"));
	trimstr(s1);
	strcpy(s2, nvram_safe_get("tow_ssh_username"));
	trimstr(s2);
	fprintf(fps, "UDOMAIN=\"%s@%s\"\n", s2, s1);
	strcpy(s1, nvram_safe_get("tow_ssh_port"));
	trimstr(s1);
	fprintf(fps, "UPORT=\"%s\"\n", s1);
	strcpy(s1, nvram_safe_get("tow_ssh_passwd"));
	trimstr(s1);
	fprintf(fps, "UPASSWD=\"%s\"\n", s1);
	strcpy(s1, nvram_safe_get("tow_ssh_obfcode"));
	trimstr(s1);
	fprintf(fps, "OBKEY=\"%s\"\n\n", s1);
	fprintf(fps, "BIN=ssh_ofc\n");
	fprintf(fps, "BINPATH=\"/usr/bin/\"\n");
	strcpy(s1, nvram_safe_get("tow_ssh_listen_port"));
	trimstr(s1);
	fprintf(fps, "LIPPORT=\"127.0.0.1:%s\"\n", s1);
	fprintf(fps, "alias elog=\"logger -t $BIN -s\"\n\n");
	fprintf(fps, "if [ -n \"$OBKEY\" ]; then\n");
	strcpy(s1, nvram_safe_get("tow_ssh_argv"));
	trimstr(s1);
	fprintf(fps, "    CONF=\"%s -Z ${OBKEY} -p ${UPORT} ${UDOMAIN} -D \\\n", s1);
	fprintf(fps, "    ${LIPPORT}\"\n");
	fprintf(fps, "else\n");
	fprintf(fps, "    CONF=\"%s -p ${UPORT} ${UDOMAIN} -D ${LIPPORT}\"\n", s1);
	fprintf(fps, "fi\n\n\n");
	fprintf(fps, "start_os() {\n");
	fprintf(fps, "    elog \"Starting $BIN... \"\n");
	strcpy(s1, nvram_safe_get("tow_ssh_listen_port"));
	trimstr(s1);
	fprintf(fps, "    ON=`netstat -ln | grep \"%s\" | wc -l`\n", s1);
	fprintf(fps, "    if [ \"$ON\" != \"1\" ]; then\n");
	fprintf(fps, "        sshpass -p ${UPASSWD} ${BINPATH}${BIN} $CONF > /dev/null 2>&1 &\n");
	fprintf(fps, "        elog \"$BIN Started. \"\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        elog \"${BIN} already running!\"\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "stop_os() {\n");
	fprintf(fps, "    elog \"Stopping $BIN... \"\n");
	fprintf(fps, "    killall -KILL $BIN\n");
	fprintf(fps, "    elog \"$BIN Stopped. \"\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "restart_os() {\n");
	fprintf(fps, "    elog \"Restarting $BIN... \"\n");
	fprintf(fps, "    stop_os\n");
	fprintf(fps, "    sleep 1\n");
	fprintf(fps, "    start_os\n");
	fprintf(fps, "    elog \"$BIN Restart Success. \"\n");
	fprintf(fps, "}\n\n");
	fprintf(fps, "COND=$1\n");
	fprintf(fps, "[ $# -eq 0 ] && COND=\"start\"\n");
	fprintf(fps, "case $COND in\n");
	fprintf(fps, "stop)\n");
	fprintf(fps, "    stop_os\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "start)\n");
	fprintf(fps, "    start_os\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "restart)\n");
	fprintf(fps, "    restart_os\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "watchdog)\n");
	strcpy(s1, nvram_safe_get("tow_ssh_listen_port"));
	trimstr(s1);
	fprintf(fps, "    ct=`netstat -ln | grep \"%s\" | wc -l`\n", s1);
	fprintf(fps, "    while [ \"$ct\" -ne \"1\" ]\n");
	fprintf(fps, "    do\n");
	fprintf(fps, "        elog \"$BIN stopped abnormal, try to restart it...\"\n");
	fprintf(fps, "        restart_os\n");
	fprintf(fps, "        sleep 5\n");
	fprintf(fps, "        ct=`netstat -ln | grep \"%s\" | wc -l`\n", s1);
	fprintf(fps, "    done\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "*)\n");
	fprintf(fps, "  elog \"Usage: $0 (start|stop|restart|watchdog)\"\n");
	fprintf(fps, "  exit 1\n");
	fprintf(fps, "esac\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru ssh_ofc\n", TOWCHECK);
	fclose(fps);

	chmod(SCRIPT_INIT_OBSSH, 0755);
	xstart(SCRIPT_INIT_OBSSH, "start");
	return;
}

void stop_redsocks2(void)
{
	FILE *fp;

	//stop file
	mkdir_if_none(SCRIPT_INIT_DIR);
	if( !( fp = fopen( "/etc/tow/stop_redsocks2.sh", "w" ) ) )
	{
		perror( "/etc/tow/stop_redsocks2.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL redsocks2\n");
	fprintf( fp, "rm -f /var/run/redsocks2.pid\n");
	fprintf( fp, "logger \"redsocks successfully stoped\" \n");
	fprintf( fp, "sleep 1\n");
	fprintf( fp, "%s delcru redsocks2\n", TOWCHECK);
	fclose( fp );
	chmod( "/etc/tow/stop_redsocks2.sh", 0755 );
	eval( "/etc/tow/stop_redsocks2.sh" );
	return;
}
void start_redsocks2(void)
{
	FILE *fps;
	char s1[256];

	//ensure redsocks stopped
	stop_redsocks2();

	//Create conf for redsocks
	if (!(fps = fopen(CONF_REDSOCKS, "wt")))
	{
		perror(CONF_REDSOCKS);
		return;
	}
	fprintf(fps, "base {\n");
	fprintf(fps, "    log_debug = off;\n");
	fprintf(fps, "    log_info = off;\n");
	fprintf(fps, "    daemon = on;\n");
	fprintf(fps, "    redirector= iptables;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "\n");
	fprintf(fps, "//Work with GoAgent\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8096; //HTTP should be redirect to this port.\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	strcpy(s1, nvram_safe_get("tow_redsocks_gae_port"));
	trimstr(s1);
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    type = http-relay; // Must be 'htt-relay' for HTTP traffic.\n");
	fprintf(fps, "    autoproxy = 0; // I want autoproxy feature enabled on this section.\n");
	fprintf(fps, "    // timeout is meaningful when 'autoproxy' is non-zero.\n");
	fprintf(fps, "    // It specified timeout value when trying to connect to destination\n");
	fprintf(fps, "    // directly. Default is 10 seconds. When it is set to 0, default\n");
	fprintf(fps, "    // timeout value will be used.\n");
	fprintf(fps, "    timeout = 10;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8097; // HTTPS should be redirect to this port.\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    type = http-connect; // Must be 'htt-connect' for HTTPS traffic.\n");
	fprintf(fps, "    autoproxy = 0; // I want autoproxy feature enabled on this section.\n");
	fprintf(fps, "    // timeout is meaningful when 'autoproxy' is non-zero.\n");
	fprintf(fps, "    // It specified timeout value when trying to connect to destination\n");
	fprintf(fps, "    // directly. Default is 10 seconds. When it is set to 0, default\n");
	fprintf(fps, "    // timeout value will be used.\n");
	fprintf(fps, "    timeout = 10;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "//Work with WallProxy\n");
	fprintf(fps, "// If WallProxy, you must use GoAgent port 8087.\n");
	fprintf(fps, "// Can not use WallProxy's smart port 8086\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8094; //HTTP should be redirect to this port.\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	strcpy(s1, nvram_safe_get("tow_redsocks_wp_port"));
	trimstr(s1);
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    type = http-relay; // Must be 'htt-relay' for HTTP traffic.\n");
	fprintf(fps, "    autoproxy = 0; // I want autoproxy feature enabled on this section.\n");
	fprintf(fps, "    // timeout is meaningful when 'autoproxy' is non-zero.\n");
	fprintf(fps, "    // It specified timeout value when trying to connect to destination\n");
	fprintf(fps, "    // directly. Default is 10 seconds. When it is set to 0, default\n");
	fprintf(fps, "    // timeout value will be used.\n");
	fprintf(fps, "    timeout = 10;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8095; // HTTPS should be redirect to this port.\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    type = http-connect; // Must be 'htt-connect' for HTTPS traffic.\n");
	fprintf(fps, "    autoproxy = 0; // I want autoproxy feature enabled on this section.\n");
	fprintf(fps, "    // timeout is meaningful when 'autoproxy' is non-zero.\n");
	fprintf(fps, "    // It specified timeout value when trying to connect to destination\n");
	fprintf(fps, "    // directly. Default is 10 seconds. When it is set to 0, default\n");
	fprintf(fps, "    // timeout value will be used.\n");
	fprintf(fps, "    timeout = 10;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "//Work with SSH\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8098;\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	strcpy(s1, nvram_safe_get("tow_ssh_listen_port"));
	trimstr(s1);
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    type = socks5;\n");
	fprintf(fps, "    autoproxy = 0;\n");
	fprintf(fps, "    timeout = 5;\n");
	fprintf(fps, "}\n");
	fprintf(fps, "\n");
	fprintf(fps, "//Work with ShadowSocks\n");
	fprintf(fps, "redsocks {\n");
	fprintf(fps, "    local_ip = 0.0.0.0;\n");
	fprintf(fps, "    local_port = 8099;\n");
	fprintf(fps, "    ip = 127.0.0.1;\n");
	strcpy(s1, nvram_safe_get("tow_ss_local_port"));
	trimstr(s1);
	fprintf(fps, "    port = %s;\n", s1);
	fprintf(fps, "    // known types: socks4, socks5, http-connect, http-relay\n");
	fprintf(fps, "    // New types: direct\n");
	fprintf(fps, "    type = socks5;\n");
	fprintf(fps, "    // Change this parameter to 1 if you want auto proxy feature.\n");
	fprintf(fps, "    // When autoproxy is set to non-zero, the connection to target\n");
	fprintf(fps, "    // will be made directly first. If direct connection to target\n");
	fprintf(fps, "    // fails for timeout/connection refuse, redsocks will try to\n");
	fprintf(fps, "    // connect to target via the proxy.\n");
	fprintf(fps, "    autoproxy = 0;\n");
	fprintf(fps, "    // timeout is meaningful when 'autoproxy' is non-zero.\n");
	fprintf(fps, "    // It specified timeout value when trying to connect to destination\n");
	fprintf(fps, "    // directly. Default is 10 seconds. When it is set to 0, default\n");
	fprintf(fps, "    // timeout value will be used.\n");
	fprintf(fps, "    timeout = 5;\n");
	fprintf(fps, "}\n");
	fclose(fps);

	//Create init script for starting redsocks
	if (!(fps = fopen(SCRIPT_INIT_REDSOCKS, "wt")))
	{
		perror(SCRIPT_INIT_REDSOCKS);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	fprintf(fps, "BIN=redsocks2\n");
	fprintf(fps, "BINPATH=\"/usr/bin/\"\n");
	fprintf(fps, "RUN_D=\"/var/run\"\n");
	fprintf(fps, "PID_F=\"${RUN_D}/${BIN}.pid\"\n");
	fprintf(fps, "CONF=\"-c %s -p ${PID_F}\"\n", CONF_REDSOCKS);
	fprintf(fps, "alias elog=\"logger -t $BIN -s\"\n");
	fprintf(fps, "COND=$1\n");
	fprintf(fps, "[ $# -eq 0 ] && COND=\"start\"\n");
	fprintf(fps, "\n");
	fprintf(fps, "case $COND in\n");
	fprintf(fps, "stop)\n");
	fprintf(fps, "  elog \"Stopping $BIN... \"\n");
	fprintf(fps, "  [ -n \"`pidof $BIN`\" ] && killall $BIN\n");
	fprintf(fps, "  elog \"$BIN Stopped. \"\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "start)\n");
	fprintf(fps, "  elog \"Starting $BIN... \"\n");
	fprintf(fps, "  [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "  [ -n \"`pidof $BIN`\" ] && elog \"${BIN} already running!\" \\\n");
	fprintf(fps, "  || ${BINPATH}${BIN} $CONF &\n");
	fprintf(fps, "  elog \"$BIN Started. \"\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "restart)\n");
	fprintf(fps, "  elog \"Restarting $BIN... \"\n");
	fprintf(fps, "  [ -n \"`pidof $BIN`\" ] && killall $BIN\n");
	fprintf(fps, "  [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "  ${BINPATH}${BIN} $CONF &\n");
	fprintf(fps, "  elog \"$BIN Restart Success. \"\n");
	fprintf(fps, "  ;;\n");
	fprintf(fps, "*)\n");
	fprintf(fps, "  elog \"Usage: $0 (start|stop|restart)\"\n");
	fprintf(fps, "  exit 1\n");
	fprintf(fps, "esac\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru redsocks2\n", TOWCHECK);
	fclose(fps);

	chmod(SCRIPT_INIT_REDSOCKS, 0755);
	xstart(SCRIPT_INIT_REDSOCKS, "start");

	return;
}

void stop_shadowsocks(void)
{
	FILE *fp;

	//stop file
	mkdir_if_none(SCRIPT_INIT_DIR);
	if( !( fp = fopen( "/etc/tow/stop_shadowsocks.sh", "w" ) ) )
	{
		perror( "/etc/tow/stop_shadowsocks.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL ss-local\n");
	fprintf( fp, "rm -f /var/run/shadowsocks.pid\n");
	fprintf( fp, "logger \"shadowsocks successfully stoped\" \n");
	fprintf( fp, "sleep 1\n");
	fprintf( fp, "%s delcru shadowsocks\n", TOWCHECK);
	fclose( fp );
	chmod( "/etc/tow/stop_shadowsocks.sh", 0755 );
	eval( "/etc/tow/stop_shadowsocks.sh" );
	return;
}
void start_shadowsocks(void)
{
	FILE *fps;
	char s1[256];

	//ensure shadowsocks stopped
	stop_shadowsocks();

	//Create conf for shadowsocks
	if (!(fps = fopen(CONF_SHADOWSOCKS, "wt")))
	{
		perror(CONF_SHADOWSOCKS);
		return;
	}
	fprintf(fps, "{\n");
	strcpy(s1, nvram_safe_get("tow_ss_server"));
	trimstr(s1);
	fprintf(fps, "\"server\":\"%s\",\n", s1);
	strcpy(s1, nvram_safe_get("tow_ss_server_port"));
	trimstr(s1);
	fprintf(fps, "\"server_port\":%s,\n", s1);
	strcpy(s1, nvram_safe_get("tow_ss_passwd"));
	trimstr(s1);
	fprintf(fps, "\"password\":\"%s\",\n", s1);
	fprintf(fps, "\"timeout\":600,\n");
	strcpy(s1, nvram_safe_get("tow_ss_crypt_method"));
	trimstr(s1);
	fprintf(fps, "\"method\":\"%s\"\n", s1);
	fprintf(fps, "}\n");
	fclose(fps);

	//Create init script for starting shadowsocks
	if (!(fps = fopen(SCRIPT_INIT_SHADOWSOCKS, "wt")))
	{
		perror(SCRIPT_INIT_SHADOWSOCKS);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	fprintf(fps, "BIN=shadowsocks\n");
	fprintf(fps, "BINPATH=\"/usr/bin/\"\n");
	fprintf(fps, "RUN_D=\"/var/run\"\n");
	fprintf(fps, "PID_F=\"${RUN_D}/${BIN}.pid\"\n");
	fprintf(fps, "CONF=\"-c %s -l %s -f ${PID_F}\"\n", CONF_SHADOWSOCKS, nvram_safe_get("tow_ss_local_port"));
	fprintf(fps, "alias elog=\"logger -t $BIN -s\"\n");
	fprintf(fps, "COND=$1\n");
	fprintf(fps, "[ $# -eq 0 ] && COND=\"start\"\n");
	fprintf(fps, "\n");
	fprintf(fps, "case $COND in\n");
	fprintf(fps, "stop)\n");
	fprintf(fps, "    elog \"Stopping $BIN... \"\n");
	fprintf(fps, "    [ -n \"`pidof ss-local`\" ] && { killall ss-local; elog \"ss-local Stopped. \"; }\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "start)\n");
	fprintf(fps, "    elog \"Starting ss-local... \"\n");
	fprintf(fps, "    [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "    if [ -n \"`pidof ss-local`\" ]; then\n");
	fprintf(fps, "        elog \"ss-local already running!\"\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        ${BINPATH}ss-local $CONF &\n");
	fprintf(fps, "        elog \"ss-local Started. \"\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "restart)\n");
	fprintf(fps, "    elog \"Restarting $BIN... \"\n");
	fprintf(fps, "    [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "    if [ -n \"`pidof ss-local`\" ]; then\n");
	fprintf(fps, "            killall ss-local\n");
	fprintf(fps, "        ${BINPATH}ss-local $CONF &\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        elog \"ss-local is not running, start...\"\n");
	fprintf(fps, "        ${BINPATH}ss-local $CONF &\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    elog \"$BIN Restart Success. \"\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "*)\n");
	fprintf(fps, "    elog \"Usage: $0 (start|stop|restart)\"\n");
	fprintf(fps, "    exit 1\n");
	fprintf(fps, "esac\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru shadowsocks\n", TOWCHECK);
	fclose(fps);

	chmod(SCRIPT_INIT_SHADOWSOCKS, 0755);
	xstart(SCRIPT_INIT_SHADOWSOCKS, "start");
	return;
}

void stop_redir(void)
{
	FILE *fp;

	//stop file
	mkdir_if_none(SCRIPT_INIT_DIR);
	if( !( fp = fopen( "/etc/tow/stop_shadowsocks_redir.sh", "w" ) ) )
	{
		perror( "/etc/tow/stop_shadowsocks_redir.sh" );
		return;
	}
	fprintf( fp, "#!/bin/sh\n\n" );
	fprintf( fp, "killall -KILL ss-redir\n");
	fprintf( fp, "rm -f /var/run/shadowsocks_redir.pid\n");
	fprintf( fp, "logger \"shadowsocks_redir successfully stoped\" \n");
	fprintf( fp, "sleep 1\n");
	fprintf( fp, "%s delcru redir\n", TOWCHECK);
	fclose( fp );
	chmod( "/etc/tow/stop_shadowsocks_redir.sh", 0755 );
	eval( "/etc/tow/stop_shadowsocks_redir.sh" );
	return;
}
void start_redir(void)
{
	FILE *fps;
	char s1[256];

	//ensure shadowsocks stopped
	stop_shadowsocks();

	//Create conf for shadowsocks
	if (!(fps = fopen(CONF_SHADOWSOCKS_REDIR, "wt")))
	{
		perror(CONF_SHADOWSOCKS_REDIR);
		return;
	}
	fprintf(fps, "{\n");
	strcpy(s1, nvram_safe_get("tow_ss_server"));
	trimstr(s1);
	fprintf(fps, "\"server\":\"%s\",\n", s1);
	strcpy(s1, nvram_safe_get("tow_ss_server_port"));
	trimstr(s1);
	fprintf(fps, "\"server_port\":%s,\n", s1);
	strcpy(s1, nvram_safe_get("tow_ss_passwd"));
	trimstr(s1);
	fprintf(fps, "\"password\":\"%s\",\n", s1);
	fprintf(fps, "\"timeout\":600,\n");
	strcpy(s1, nvram_safe_get("tow_ss_crypt_method"));
	trimstr(s1);
	fprintf(fps, "\"method\":\"%s\"\n", s1);
	fprintf(fps, "}\n");
	fclose(fps);

	//Create init script for starting shadowsocks
	if (!(fps = fopen(SCRIPT_INIT_SHADOWSOCKS_REDIR, "wt")))
	{
		perror(SCRIPT_INIT_SHADOWSOCKS_REDIR);
		return;
	}
	fprintf(fps, "#!/bin/sh\n");
	fprintf(fps, "BIN=shadowsocks\n");
	fprintf(fps, "BINPATH=\"/usr/bin/\"\n");
	fprintf(fps, "RUN_D=\"/var/run\"\n");
	fprintf(fps, "PID_REDIR=\"${RUN_D}/${BIN}_redir.pid\"\n");
	fprintf(fps, "CONF_REDIR=\"-c %s -l %s -f ${PID_REDIR}\"\n", CONF_SHADOWSOCKS_REDIR, nvram_safe_get("tow_ss_redir_local_port"));
	fprintf(fps, "alias elog=\"logger -t $BIN -s\"\n");
	fprintf(fps, "COND=$1\n");
	fprintf(fps, "[ $# -eq 0 ] && COND=\"start\"\n");
	fprintf(fps, "\n");
	fprintf(fps, "case $COND in\n");
	fprintf(fps, "stop)\n");
	fprintf(fps, "    elog \"Stopping $BIN... \"\n");
	fprintf(fps, "    [ -n \"`pidof ss-redir`\" ] && { killall ss-redir; elog \"ss-redir Stopped. \"; }\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "start)\n");
	fprintf(fps, "    elog \"$BIN redir mode! \"\n");
	fprintf(fps, "    [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "    if [ -n \"`pidof ss-redir`\" ]; then\n");
	fprintf(fps, "        elog \"ss-redir already running!\"\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        ${BINPATH}ss-redir $CONF_REDIR &\n");
	fprintf(fps, "        elog \"ss-redir Started. \"\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "restart)\n");
	fprintf(fps, "    elog \"Restarting $BIN redir... \"\n");
	fprintf(fps, "    [ ! -d $RUN_D ] && mkdir -p $RUN_D\n");
	fprintf(fps, "    if [ -n \"`pidof ss-redir`\" ]; then\n");
	fprintf(fps, "        killall ss-redir\n");
	fprintf(fps, "        ${BINPATH}ss-redir $CONF_REDIR &\n");
	fprintf(fps, "    else\n");
	fprintf(fps, "        elog \"ss-redir is not running! starting...\"\n");
	fprintf(fps, "        ${BINPATH}ss-redir $CONF_REDIR &\n");
	fprintf(fps, "    fi\n");
	fprintf(fps, "    elog \"$BIN redir Restart Success. \"\n");
	fprintf(fps, "    ;;\n");
	fprintf(fps, "*)\n");
	fprintf(fps, "    elog \"Usage: $0 (start|stop|restart)\"\n");
	fprintf(fps, "    exit 1\n");
	fprintf(fps, "esac\n\n");
	fprintf(fps, "# Add check into cru list\n");
	fprintf(fps, "%s addcru redir\n", TOWCHECK);
	fclose(fps);

	chmod(SCRIPT_INIT_SHADOWSOCKS_REDIR, 0755);
	xstart(SCRIPT_INIT_SHADOWSOCKS_REDIR, "start");
	return;
}

void stop_tow(void)
{
	mkdir_if_none(SCRIPT_INIT_DIR);
	stop_pdnsd();
	stop_ssh_ofc();
	stop_shadowsocks();
	stop_redir();
	stop_redsocks2();
	stop_dnsmasqc();
	syslog(LOG_INFO, "Transparent over Wall Proxy has stopped.");
	return;
}
void start_tow(void)
{
	char s1[4096];

	if (! nvram_match("tow_enable", "1")) return;

	strcpy(s1, nvram_safe_get("dnsmasq_custom"));
	filter_space(s1);
	trimstr(s1);
	if (strstr(s1,"conf-dir=/etc/dnsmasq/custom") == NULL)
	{
		if((s1[strlen(s1) -1] != '\n') && (strlen(s1) > 0)) strcat(s1, "\nconf-dir=/etc/dnsmasq/custom");
		else strcat(s1, "conf-dir=/etc/dnsmasq/custom");
		nvram_set("dnsmasq_custom", s1);
	}
	start_dnsmasqc();
	start_pdnsd();
	strcpy(s1, nvram_safe_get("tow_mode"));
	trimstr(s1);
	if (strcmp(s1, "ss") == 0)
	{
		start_redsocks2();
		start_shadowsocks();
	}
	else if (strcmp(s1,"redir") == 0)
	{
		//do not need to startup redsocks2
		start_redir();
	}
	else if (strcmp(s1, "ssh") == 0)
	{
		start_redsocks2();
		start_ssh_ofc();
	}
	else if (strcmp(s1, "gae") == 0)
	{
		strcpy(s1, eval_return("ps w | grep python | grep proxy.py | grep -v grep | wc -l"));
		if (atoi (s1) == 0)
		{
			syslog(LOG_WARNING, "GoAgent is not running? Pls. be sure to start it.");
		}
		start_redsocks2();
	}
	else if (strcmp(s1, "wp") == 0)
	{
		strcpy(s1, eval_return("ps w | grep python | grep startup.py | grep -v grep | wc -l"));
		if (atoi (s1) == 0)
		{
			syslog(LOG_WARNING, "WallProxy is not running? Pls. be sure to start it.");
		}
	}
	else
	{
		syslog(LOG_ERR, "Invalid nvram value of tow_mode. Starting maybe unsuccessful.");
	}
	syslog(LOG_INFO, "Transparent over Wall Proxy has started.");
	
	return;
}

