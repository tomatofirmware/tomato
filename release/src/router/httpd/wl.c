/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "tomato.h"

#include <ctype.h>
#include <wlutils.h>

#ifndef WL_BSS_INFO_VERSION
#error WL_BSS_INFO_VERSION
#endif
#if WL_BSS_INFO_VERSION < 108
#error WL_BSS_INFO_VERSION < 108
#endif


static int unit = 0;
static int subunit = 0;

static void check_wl_unit(const char *unitarg)
{
	char ifname[12], *wlunit;
	unit = 0; subunit = 0;

	wlunit = (unitarg && *unitarg) ? (char *)unitarg :
		webcgi_safeget("_wl_unit", nvram_safe_get("wl_unit"));
	snprintf(ifname, sizeof(ifname), "wl%s", wlunit);
	get_ifname_unit(ifname, &unit, &subunit);

	_dprintf("check_wl_unit: unitarg: %s, _wl_unit: %s, ifname: %s, unit: %d, subunit: %d\n",
		unitarg, webcgi_safeget("_wl_unit", nvram_safe_get("wl_unit")), ifname, unit, subunit);
}

static void wl_restore(char *wif, int unit, int ap, int radio)
{
	if (ap > 0) {
		wl_ioctl(wif, WLC_SET_AP, &ap, sizeof(ap));

		if (!radio) set_radio(1, unit);
		eval("wl", "-i", wif, "up"); // without this the router may reboot
#if WL_BSS_INFO_VERSION >= 108
		// no idea why this voodoo sequence works to wake up wl	-- zzz
		eval("wl", "-i", wif, "ssid", "");
		eval("wl", "-i", wif, "ssid", nvram_safe_get(wl_nvname("ssid", unit, 0)));
#endif
	}
	set_radio(radio, unit);
}

// allow to scan using up to MAX_WLIF_SCAN wireless ifaces
#define MAX_WLIF_SCAN	3

typedef struct {
	int unit_filter;
	char comma;
	struct {
		int ap;
		int radio;
	} wif[MAX_WLIF_SCAN];
} scan_list_t;

static int start_scan(int idx, int unit, int subunit, void *param)
{
	scan_list_t *rp = param;
	wl_scan_params_t sp;
	char *wif;
	int zero = 0;
	int retry = 3;

	if ((idx >= MAX_WLIF_SCAN) || (rp->unit_filter >= 0 && rp->unit_filter != unit)) return 0;

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));
	memset(&sp, 0xff, sizeof(sp));		// most default to -1
	memset(&sp.bssid, 0xff, sizeof(sp.bssid));
	sp.ssid.SSID_len = 0;
	sp.bss_type = DOT11_BSSTYPE_ANY;	// =2
	sp.channel_num = 0;
#ifdef CONFIG_BCMWL5
	sp.scan_type = DOT11_SCANTYPE_ACTIVE;
#else
	// with older BCM wifi drivers, passive scan seems to provide better results
	sp.scan_type = DOT11_SCANTYPE_PASSIVE;
#endif

	if (wl_ioctl(wif, WLC_GET_AP, &(rp->wif[idx].ap), sizeof(rp->wif[idx].ap)) < 0) {
		// Unable to get AP mode
		return 0;
	}
	if (rp->wif[idx].ap > 0) {
		wl_ioctl(wif, WLC_SET_AP, &zero, sizeof(zero));
	}

	rp->wif[idx].radio = get_radio(unit);
	if (!(rp->wif[idx].radio)) set_radio(1, unit);

	while (retry--) {
		if (wl_ioctl(wif, WLC_SCAN, &sp, WL_SCAN_PARAMS_FIXED_SIZE) == 0)
			return 1;
		if (retry) sleep(1);
	}

	// Unable to start scan
	wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio);
	return 0;
}

static int get_scan_results(int idx, int unit, int subunit, void *param)
{
	scan_list_t *rp = param;

	if ((idx >= MAX_WLIF_SCAN) || (rp->unit_filter >= 0 && rp->unit_filter != unit)) return 0;

	// get results

	char *wif;
	wl_scan_results_t *results;
	wl_bss_info_t *bssi;
	int r;

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));

	results = malloc(WLC_IOCTL_MAXLEN);
	if (!results) {
		// Not enough memory
		wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio);
		return 0;
	}
	results->buflen = WLC_IOCTL_MAXLEN - (sizeof(*results) - sizeof(results->bss_info));
	results->version = WL_BSS_INFO_VERSION;

	// Keep trying to obtain scan results for up to 3 more secs
	// Passive scan may require more time, although 1 extra sec is almost always enough.
	int t;
	for (t = 0; t < 3 * 10; t++) {
		r = wl_ioctl(wif, WLC_SCAN_RESULTS, results, WLC_IOCTL_MAXLEN);
		if (r >= 0)
			break;
		usleep(100000);
	}

	wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio);

	if (r < 0) {
		free(results);
		// Unable to obtain scan results
		return 0;
	}

	// format for javascript

	int i;
	int j;
	int k;
	char c;
	char ssid[64];
	char mac[32];
	char *ssidj;
	int channel;

	bssi = &results->bss_info[0];
	for (i = 0; i < results->count; ++i) {

		channel = CHSPEC_CHANNEL(bssi->chanspec);
		if (CHSPEC_IS40(bssi->chanspec))
			channel = channel + (CHSPEC_SB_LOWER(bssi->chanspec) ? -2 : 2);

		j = bssi->SSID_len;
		if (j < 0) j = 0;
		if (j > 32) j = 32;
		if (nvram_get_int("wlx_scrubssid")) {
			for (k = j - 1; k >= 0; --k) {
				c = bssi->SSID[k];
				if (!isprint(c)) c = '?';
				ssid[k] = c;
			}
		}
		else {
			memcpy(ssid, bssi->SSID, j);
		}
		ssid[j] = 0;
		ssidj = js_string(ssid);

		web_printf("%c['%s','%s',%u,%u,%d,%d,[", rp->comma,
			ether_etoa(bssi->BSSID.octet, mac), ssidj ? ssidj : "",
			channel,
			bssi->capability, bssi->RSSI, bssi->phy_noise);
		rp->comma = ',';
		free(ssidj);

		for (j = 0; j < bssi->rateset.count; ++j) {
			web_printf("%s%u", j ? "," : "", bssi->rateset.rates[j]);
		}
		web_printf("],%d,%d]", bssi->n_cap, bssi->nbss_cap);

		bssi = (wl_bss_info_t*)((uint8*)bssi + bssi->length);
	}
	free(results);

	return 1;
}

//	returns: ['bssid','ssid',channel,capabilities,rssi,noise,[rates,]],  or  [null,'error message']
void asp_wlscan(int argc, char **argv)
{
	scan_list_t rp;

	memset(&rp, 0, sizeof(rp));
	rp.comma = ' ';
	rp.unit_filter = (argc > 0) ? atoi(argv[0]) : (-1);

	web_puts("\nwlscandata = [");

	// scan

	if (foreach_wif(0, &rp, start_scan) == 0) {
		web_puts("[null,'Unable to start scan.']];\n");
		return;
	}
	sleep(2);

	// get results

	if (foreach_wif(0, &rp, get_scan_results) == 0) {
		web_puts("[null,'Unable to obtain scan results.']];\n");
		return;
	}

	web_puts("];\n");
}

void wo_wlradio(char *url)
{
	char *enable;
	char sunit[10];

	check_wl_unit(NULL);

	parse_asp("saved.asp");
	if (nvram_get_int(wl_nvname("radio", unit, 0))) {
		if ((enable = webcgi_get("enable")) != NULL) {
			web_close();
			sleep(2);
			sprintf(sunit, "%d", unit);
			eval("radio", atoi(enable) ? "on" : "off", sunit);
			return;
		}
	}
}

static int read_noise(int unit)
{
	int v;
	
	// WLC_GET_PHY_NOISE = 135
	if (wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, 0)), 135, &v, sizeof(v)) == 0) {
		char s[32];
		sprintf(s, "%d", v);
		nvram_set(wl_nvname("tnoise", unit, 0), s);
		return v;
	}
	return -99;
}

static int get_wlnoise(int client, int unit)
{
	int v;

	if (client) {
		v = read_noise(unit);
	}
	else {
		v = nvram_get_int(wl_nvname("tnoise", unit, 0));
		if ((v >= 0) || (v < -100)) v = -99;
	}
	return v;
}

static int print_wlnoise(int idx, int unit, int subunit, void *param)
{
	web_printf("%c%d", (idx == 0) ? ' ' : ',', get_wlnoise(wl_client(unit, 0), unit));
	return 0;
}

void asp_wlnoise(int argc, char **argv)
{
	web_puts("\nwlnoise = [");
	foreach_wif(0, NULL, print_wlnoise);
	web_puts(" ];\n");
}

void wo_wlmnoise(char *url)
{
	int ap;
	int i;
	char *wif;

	check_wl_unit(NULL);

	parse_asp("mnoise.asp");
	web_close();
	sleep(3);

	int radio = get_radio(unit);

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));
	if (wl_ioctl(wif, WLC_GET_AP, &ap, sizeof(ap)) < 0) return;

	i = 0;
	wl_ioctl(wif, WLC_SET_AP, &i, sizeof(i));

	for (i = 10; i > 0; --i) {
		sleep(1);
		read_noise(unit);
	}

	wl_restore(wif, unit, ap, radio);
}

static int wl_chanfreq(uint ch, int band)
{
	if ((band == WLC_BAND_2G && (ch < 1 || ch > 14)) || (ch > 200))
		return -1;
	else if ((band == WLC_BAND_2G) && (ch == 14))
		return 2484;
	else
		return ch * 5 + ((band == WLC_BAND_2G) ? 4814 : 10000) / 2;
}

static int not_wlclient(int idx, int unit, int subunit, void *param)
{
	return (!wl_client(unit, subunit));
}

// returns '1' if all wireless interfaces are in client mode, '0' otherwise
void asp_wlclient(int argc, char **argv)
{
	web_puts(foreach_wif(1, NULL, not_wlclient) ? "0" : "1");
}

static int print_wlstats(int idx, int unit, int subunit, void *param)
{
	int phytype;
	int rate, client, nbw;
	int chanspec, channel, mhz, band, scan;
	int chanim_enab;
	int interference;
	char retbuf[WLC_IOCTL_SMLEN];
	scb_val_t rssi;
	char *ifname, *ctrlsb;

	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	client = wl_client(unit, 0);

	/* Get configured phy type */
	wl_ioctl(ifname, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));

	if (wl_ioctl(ifname, WLC_GET_RATE, &rate, sizeof(rate)) < 0)
		rate = 0;

	if (wl_ioctl(ifname, WLC_GET_BAND, &band, sizeof(band)) < 0)
		band = nvram_get_int(wl_nvname("nband", unit, 0));

	channel = nvram_get_int(wl_nvname("channel", unit, 0));
	scan = 0;
	interference = -1;

	if (wl_phytype_n(phytype)) {
		if (wl_iovar_getint(ifname, "chanspec", &chanspec) != 0) {
			ctrlsb = nvram_safe_get(wl_nvname("nctrlsb", unit, 0));
			nbw = nvram_get_int(wl_nvname("nbw", unit, 0));
		}
		else {
			channel = CHSPEC_CHANNEL(chanspec);
			if (CHSPEC_IS40(chanspec))
				channel = channel + (CHSPEC_SB_LOWER(chanspec) ? -2 : 2);
			ctrlsb = CHSPEC_SB_LOWER(chanspec) ? "lower" : (CHSPEC_SB_UPPER(chanspec) ? "upper" : "none");
			nbw = CHSPEC_IS40(chanspec) ? 40 : 20;
		}
	}
	else {
		channel_info_t ch;
		if (wl_ioctl(ifname, WLC_GET_CHANNEL, &ch, sizeof(ch)) == 0) {
			scan = (ch.scan_channel > 0);
			channel = (scan) ? ch.scan_channel : ch.hw_channel;
		}
		ctrlsb = "";
		nbw = 20;
	}

	mhz = (channel) ? wl_chanfreq(channel, band) : 0;
	if (wl_iovar_getint(ifname, "chanim_enab", (int*)(void*)&chanim_enab) != 0)
		chanim_enab = 0;
	if (chanim_enab) {
		if (wl_iovar_getbuf(ifname, "chanim_state", &chanspec, sizeof(chanspec), retbuf, WLC_IOCTL_SMLEN) == 0)
			interference = *(int*)retbuf;
	}

	memset(&rssi, 0, sizeof(rssi));
	if (client) {
		if (wl_ioctl(ifname, WLC_GET_RSSI, &rssi, sizeof(rssi)) != 0)
			rssi.val = -100;
	}

	// [ radio, is_client, channel, freq (mhz), rate, nctrlsb, nbw, rssi, noise, interference ]
	web_printf("%c{ radio: %d, client: %d, channel: %c%d, mhz: %d, rate: %d, ctrlsb: '%s', nbw: %d, rssi: %d, noise: %d, intf: %d }\n",
		(idx == 0) ? ' ' : ',',
		get_radio(unit), client, (scan ? '-' : ' '), channel, mhz, rate, ctrlsb, nbw, rssi.val, get_wlnoise(client, unit), interference);

	return 0;
}

void asp_wlstats(int argc, char **argv)
{
	web_puts("\nwlstats = [");
	foreach_wif(0, NULL, print_wlstats);
	web_puts("];\n");
}

static void web_print_wlchan(uint chan, int band)
{
	int mhz;
	if ((mhz = wl_chanfreq(chan, band)) > 0)
		web_printf(",[%d, %d]", chan, mhz);
	else
		web_printf(",[%d, 0]", chan);
}

static int _wlchanspecs(char *ifname, char *country, int band, int bw, int ctrlsb)
{
	chanspec_t c = 0, *chanspec;
	int buflen;
	wl_uint32_list_t *list;
	int count, i = 0;

	char *buf = (char *)malloc(WLC_IOCTL_MAXLEN);
	if (!buf)
		return 0;

	strcpy(buf, "chanspecs");
	buflen = strlen(buf) + 1;

	c |= (band == WLC_BAND_5G) ? WL_CHANSPEC_BAND_5G : WL_CHANSPEC_BAND_2G;
	c |= (bw == 20) ? WL_CHANSPEC_BW_20 : WL_CHANSPEC_BW_40;

	chanspec = (chanspec_t *)(buf + buflen);
	*chanspec = c;
	buflen += (sizeof(chanspec_t));
	strncpy(buf + buflen, country, WLC_CNTRY_BUF_SZ);
	buflen += WLC_CNTRY_BUF_SZ;

	/* Add list */
	list = (wl_uint32_list_t *)(buf + buflen);
	list->count = WL_NUMCHANSPECS;
	buflen += sizeof(uint32)*(WL_NUMCHANSPECS + 1);

	if (wl_ioctl(ifname, WLC_GET_VAR, buf, buflen) < 0) {
		free((void *)buf);
		return 0;
	}

	count = 0;
	list = (wl_uint32_list_t *)buf;
	for (i = 0; i < list->count; i++) {
		c = (chanspec_t)list->element[i];
		/* Skip upper.. (take only one of the lower or upper) */
		if (bw == 40 && (CHSPEC_CTL_SB(c) != ctrlsb))
			continue;
		/* Create the actual control channel number from sideband */
		int chan = CHSPEC_CHANNEL(c);
		if (bw == 40)
			chan += ((ctrlsb == WL_CHANSPEC_CTL_SB_UPPER) ? 2 : -2);
		web_print_wlchan(chan, band);
		count++;
	}

	free((void *)buf);
	return count;
}

static void _wlchannels(char *ifname, char *country, int band)
{
	int i;
	wl_channels_in_country_t *cic;

	cic = (wl_channels_in_country_t *)malloc(WLC_IOCTL_MAXLEN);
	if (cic) {
		cic->buflen = WLC_IOCTL_MAXLEN;
		strcpy(cic->country_abbrev, country);
		cic->band = band;

		if (wl_ioctl(ifname, WLC_GET_CHANNELS_IN_COUNTRY, cic, cic->buflen) == 0) {
			for (i = 0; i < cic->count; i++) {
				web_print_wlchan(cic->channel[i], band);
			}
		}
		free((void *)cic);
	}
}

void asp_wlchannels(int argc, char **argv)
{
	char buf[WLC_CNTRY_BUF_SZ];
	int band, phytype, nphy;
	int bw, ctrlsb, chanspec;
	char *ifname;

	// args: unit, nphy[1|0], bw, band, ctrlsb

	check_wl_unit(argc > 0 ? argv[0] : NULL);

	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	wl_ioctl(ifname, WLC_GET_COUNTRY, buf, sizeof(buf));
	if (wl_ioctl(ifname, WLC_GET_BAND, &band, sizeof(band)) != 0)
		band = nvram_get_int(wl_nvname("nband", unit, 0));
	wl_iovar_getint(ifname, "chanspec", &chanspec);

	if (argc > 1)
		nphy = atoi(argv[1]);
	else {
		wl_ioctl(ifname, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
		nphy = wl_phytype_n(phytype);
	}

	bw = (argc > 2) ? atoi(argv[2]) : 0;
	bw = bw ? : CHSPEC_IS40(chanspec) ? 40 : 20;

	if (argc > 3) band = atoi(argv[3]) ? : band;

	if (argc > 4) {
		if (strcmp(argv[4], "upper") == 0)
			ctrlsb = WL_CHANSPEC_CTL_SB_UPPER;
		else
			ctrlsb = WL_CHANSPEC_CTL_SB_LOWER;
	}
	else
		ctrlsb = CHSPEC_CTL_SB(chanspec);

	web_puts("\nwl_channels = [\n[0, 0]");
	if (nphy) {
		if (!_wlchanspecs(ifname, buf, band, bw, ctrlsb) && band == WLC_BAND_2G && bw == 40)
			_wlchanspecs(ifname, buf, band, 20, ctrlsb);
	}
	else
		_wlchannels(ifname, buf, band);
	web_puts("];\n");
}

static int print_wlbands(int idx, int unit, int subunit, void *param)
{
	char *phytype, *phylist, *ifname;
	char comma = ' ';
	int list[WLC_BAND_ALL];
	int i;
	
	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	phytype = nvram_safe_get(wl_nvname("phytype", unit, 0));

	web_printf("%c[", (idx == 0) ? ' ' : ',');

	if (phytype[0] == 'n' || phytype[0] == 'l' || phytype[0] == 's' || phytype[0] == 'c') {
		/* Get band list. Assume both the bands in case of error */
		if (wl_ioctl(ifname, WLC_GET_BANDLIST, list, sizeof(list)) < 0) {
			for (i = WLC_BAND_5G; i <= WLC_BAND_2G; i++) {
				web_printf("%c'%d'", comma, i);
				comma = ',';
			}
		}
		else {
			if (list[0] > 2)
				list[0] = 2;
			for (i = 1; i <= list[0]; i++) {
				web_printf("%c'%d'", comma, list[i]);
				comma = ',';
			}
		}
	}
	else {
		/* Get available phy types of the currently selected wireless interface */
		phylist = nvram_safe_get(wl_nvname("phytypes", unit, 0));
		for (i = 0; i < strlen(phylist); i++) {
			web_printf("%c'%d'", comma, phylist[i] == 'a' ? WLC_BAND_5G : WLC_BAND_2G);
			comma = ',';
		}
	}

	web_puts("]");

	return 0;
}

void asp_wlbands(int argc, char **argv)
{
	web_puts("\nwl_bands = [");
	foreach_wif(0, NULL, print_wlbands);
	web_puts(" ];\n");
}

static int print_wif(int idx, int unit, int subunit, void *param)
{
	char unit_str[] = "000000";
	char *ssidj;

	if (subunit > 0)
		snprintf(unit_str, sizeof(unit_str), "%d.%d", unit, subunit);
	else
		snprintf(unit_str, sizeof(unit_str), "%d", unit);

	// [ifname, unitstr, unit, subunit, ssid, hwaddr]
	ssidj = js_string(nvram_safe_get(wl_nvname("ssid", unit, subunit)));
	web_printf("%c['%s','%s',%d,%d,'%s','%s']", (idx == 0) ? ' ' : ',',
		nvram_safe_get(wl_nvname("ifname", unit, subunit)),
		unit_str, unit, subunit, ssidj,
		// assume the slave inteface MAC address is the same as the primary interface
		nvram_safe_get(wl_nvname("hwaddr", unit, 0))
	);
	free(ssidj);

	return 0;
}

void asp_wlifaces(int argc, char **argv)
{
	int include_vifs = (argc > 0) ? atoi(argv[0]) : 0;

	web_puts("\nwl_ifaces = [");
	foreach_wif(include_vifs, NULL, print_wif);
	web_puts("];\n");
}

void asp_wlcountries(int argc, char **argv)
{
	char s[128], *p, *code, *country;
	FILE *f;
	int i = 0;

	web_puts("\nwl_countries = [");
	if ((f = popen("wl country list", "r")) != NULL) {
		// skip the header line
		fgets(s, sizeof(s), f);
		while (fgets(s, sizeof(s), f)) {
			p = s;
			if ((code = strsep(&p, " \t\n")) && p) {
				country = strsep(&p, "\n");
				if (country && *country && strcmp(code, country) != 0) {
					p = js_string(country);
					web_printf("%c['%s', '%s']", i++ ? ',' : ' ', code, p);
					free(p);
				}
			}
		}
		fclose(f);
	}
	web_puts("];\n");
}
