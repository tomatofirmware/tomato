<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] <% _("Advanced"); %>: <% _("Wireless"); %></title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript' src='wireless.jsx?_http_id=<% nv(http_id); %>'></script>
<script type='text/javascript'>

//	<% nvram("wl_security_mode,wl_afterburner,wl_antdiv,wl_ap_isolate,wl_auth,wl_bcn,wl_dtim,wl_frag,wl_frameburst,wl_gmode_protection,wl_plcphdr,wl_rate,wl_rateset,wl_rts,wl_txant,wl_wme,wl_wme_no_ack,wl_wme_apsd,wl_txpwr,wl_mrate,t_features,wl_distance,wl_maxassoc,wlx_hpamp,wlx_hperx,wl_reg_mode,wl_country_code,wl_country,wl_btc_mode,wl_mimo_preamble,wl_obss_coex,wl_mitigation,wl_wmf_bss_enable"); %>
//	<% wlcountries(); %>

hp = features('hpamp');
nphy = features('11n');

function verifyFields(focused, quiet)
{
	for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
//		if(wl_ifaces[uidx][0].indexOf('.') < 0) {
		if (wl_sunit(uidx)<0) {
			var u = wl_unit(uidx);

			if (!v_range('_f_wl'+u+'_distance', quiet, 0, 99999)) return 0;
			if (!v_range('_wl'+u+'_maxassoc', quiet, 0, 255)) return 0;
			if (!v_range('_wl'+u+'_bcn', quiet, 1, 65535)) return 0;
			if (!v_range('_wl'+u+'_dtim', quiet, 1, 255)) return 0;
			if (!v_range('_wl'+u+'_frag', quiet, 256, 2346)) return 0;
			if (!v_range('_wl'+u+'_rts', quiet, 0, 2347)) return 0;
			if (!v_range(E('_wl'+u+'_txpwr'), quiet, hp ? 1 : 0, hp ? 251 : 400)) return 0;

			var b = E('_wl'+u+'_wme').value == 'off';
			E('_wl'+u+'_wme_no_ack').disabled = b;
			E('_wl'+u+'_wme_apsd').disabled = b;
		}
	}

	return 1;
}

function save()
{
	var fom;
	var n;

	if (!verifyFields(null, false)) return;

	fom = E('_fom');

	for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
		if (wl_sunit(uidx)<0) {
			var u = wl_unit(uidx);

			n = E('_f_wl'+u+'_distance').value * 1;
			E('_wl'+u+'_distance').value = n ? n : '';

			E('_wl'+u+'_country').value = E('_wl'+u+'_country_code').value;
			E('_wl'+u+'_nmode_protection').value = E('_wl'+u+'_gmode_protection').value;
		}
	}

	if (hp) {
		if ((E('_wlx_hpamp').value != nvram.wlx_hpamp) || (E('_wlx_hperx').value != nvram.wlx_hperx)) {
			fom._service.disabled = 1;
			fom._reboot.value = 1;
			form.submit(fom, 0);
			return;
		}
	}
	else {
		E('_wlx_hpamp').disabled = 1;
		E('_wlx_hperx').disabled = 1;
	}

	form.submit(fom, 1);
}
</script>

</head>
<body>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'><% _("Tomato"); %></div>
	<div class='version'><% _("Version"); %> <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='advanced-wireless.asp'>
<input type='hidden' name='_nextwait' value='10'>
<input type='hidden' name='_service' value='*'>
<input type='hidden' name='_reboot' value='0'>

<script type='text/javascript'>

for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
if (wl_sunit(uidx)<0) {
	var u = wl_unit(uidx);

	W('<input type=\'hidden\' id=\'_wl'+u+'_distance\' name=\'wl'+u+'_distance\'>');
	W('<input type=\'hidden\' id=\'_wl'+u+'_country\' name=\'wl'+u+'_country\'>');
	W('<input type=\'hidden\' id=\'_wl'+u+'_nmode_protection\' name=\'wl'+u+'_nmode_protection\'>');

	W('<div class=\'section-title\'>Wireless Settings ');
	//if (wl_ifaces.length > 1)
		W('(' + wl_display_ifname(uidx) + ') ');
	//W('');
	W('</div><div class=\'section\'>');

	at = ((nvram['wl'+u+'_security_mode'] != "wep") && (nvram['wl'+u+'_security_mode'] != "radius") && (nvram['wl'+u+'_security_mode'] != "disabled"));
	createFieldTable('', [
		{ title: '<% _("Afterburner"); %>', name: 'wl'+u+'_afterburner', type: 'select', options: [['auto','Auto'],['on','Enable'],['off','Disable *']],
			value: nvram['wl'+u+'_afterburner'] },
		{ title: '<% _("AP Isolation"); %>', name: 'wl'+u+'_ap_isolate', type: 'select', options: [['0','Disable *'],['1','Enable']],
			value: nvram['wl'+u+'_ap_isolate'] },
		{ title: '<% _("Authentication Type"); %>', name: 'wl'+u+'_auth', type: 'select',
			options: [['0','<% _("Auto"); %> *'],['1','<% _("Shared Key"); %>']], attrib: at ? 'disabled' : '',
			value: at ? 0 : nvram['wl'+u+'_auth'] },
		{ title: '<% _("Basic Rate"); %>', name: 'wl'+u+'_rateset', type: 'select', options: [['default','<% _("Default"); %> *'],['12','1-2 Mbps'],['all','All']],
			value: nvram['wl'+u+'_rateset'] },
		{ title: '<% _("Beacon Interval"); %>', name: 'wl'+u+'_bcn', type: 'text', maxlen: 5, size: 7,
			suffix: ' <small>(<% _("range"); %>: 1 - 65535; <% _("default"); %>: 100)</small>', value: nvram['wl'+u+'_bcn'] },
		{ title: '<% _("CTS Protection Mode"); %>', name: 'wl'+u+'_gmode_protection', type: 'select', options: [['off','<% _("Disable"); %> *'],['auto','<% _("Auto"); %>']],
			value: nvram['wl'+u+'_gmode_protection'] },
		{ title: '<% _("Regulatory Mode"); %>', name: 'wl'+u+'_reg_mode', type: 'select',
			options: [['off', '<% _("Off"); %> *'],['d', '802.11d'],['h', '802.11h']],
			value: nvram['wl'+u+'_reg_mode'] },
		{ title: '<% _("Country"); %> / <% _("Region"); %>', name: 'wl'+u+'_country_code', type: 'select',
			options: wl_countries, value: nvram['wl'+u+'_country_code'] },
		{ title: '<% _("Bluetooth Coexistence"); %>', name: 'wl'+u+'_btc_mode', type: 'select',
			options: [['0', '<% _("Disable"); %> *'],['1', '<% _("Enable"); %>'],['2', '<% _("Preemption"); %>']],
			value: nvram['wl'+u+'_btc_mode'] },
		{ title: '<% _("Distance"); %> / <% _("ACK"); %> <% _("Timing"); %>', name: 'f_wl'+u+'_distance', type: 'text', maxlen: 5, size: 7,
			suffix: ' <small><% _("meters"); %></small>&nbsp;&nbsp;<small>(<% _("range"); %>: 0 - 99999; 0 = <% _("use default"); %>)</small>',
				value: (nvram['wl'+u+'_distance'] == '') ? '0' : nvram['wl'+u+'_distance'] },
		{ title: '<% _("DTIM Interval"); %>', name: 'wl'+u+'_dtim', type: 'text', maxlen: 3, size: 5,
			suffix: ' <small>(<% _("range"); %>: 1 - 255; <% _("default"); %>: 1)</small>', value: nvram['wl'+u+'_dtim'] },
		{ title: '<% _("Fragmentation Threshold"); %>', name: 'wl'+u+'_frag', type: 'text', maxlen: 4, size: 6,
			suffix: ' <small>(<% _("range"); %>: 256 - 2346; <% _("default"); %>: 2346)</small>', value: nvram['wl'+u+'_frag'] },
		{ title: '<% _("Frame Burst"); %>', name: 'wl'+u+'_frameburst', type: 'select', options: [['off','<% _("Disable"); %> *'],['on','<% _("Enable"); %>']],
			value: nvram['wl'+u+'_frameburst'] },
		{ title: '<% _("HP"); %>', hidden: !hp || (uidx > 0) },
			{ title: '<% _("Amplifier"); %>', indent: 2, name: 'wlx_hpamp' + (uidx > 0 ? uidx + '' : ''), type: 'select', options: [['0','<% _("Disable"); %>'],['1','<% _("Enable"); %> *']],
				value: nvram.wlx_hpamp != '0', hidden: !hp || (uidx > 0) },
			{ title: '<% _("Enhanced RX Sensitivity"); %>', indent: 2, name: 'wlx_hperx' + (uidx > 0 ? uidx + '' : ''), type: 'select', options: [['0','<% _("Disable"); %> *'],['1','<% _("Enable"); %>']],
				value: nvram.wlx_hperx != '0', hidden: !hp || (uidx > 0) },
		{ title: '<% _("Maximum Clients"); %>', name: 'wl'+u+'_maxassoc', type: 'text', maxlen: 3, size: 5,
			suffix: ' <small>(<% _("range"); %>: 1 - 255; <% _("default"); %>: 128)</small>', value: nvram['wl'+u+'_maxassoc'] },
		{ title: '<% _("Multicast Rate"); %>', name: 'wl'+u+'_mrate', type: 'select',
			options: [['0','<% _("Auto"); %> *'],['1000000','1 Mbps'],['2000000','2 Mbps'],['5500000','5.5 Mbps'],['6000000','6 Mbps'],['9000000','9 Mbps'],['11000000','11 Mbps'],['12000000','12 Mbps'],['18000000','18 Mbps'],['24000000','24 Mbps'],['36000000','36 Mbps'],['48000000','48 Mbps'],['54000000','54 Mbps']],
			value: nvram['wl'+u+'_mrate'] },
		{ title: '<% _("Preamble"); %>', name: 'wl'+u+'_plcphdr', type: 'select', options: [['long','Long *'],['short','Short']],
			value: nvram['wl'+u+'_plcphdr'] },
		{ title: '<% _("802.11n Preamble"); %>', name: 'wl'+u+'_mimo_preamble', type: 'select', options: [['auto','<% _("Auto"); %>'],['mm','<% _("Mixed Mode"); %> *'],['gf','<% _("Green Field"); %>'],['gfbcm','<% _("GF-BRCM"); %>']],
			value: nvram['wl'+u+'_mimo_preamble'], hidden: !nphy },
		{ title: '<% _("Overlapping BSS Coexistence"); %>', name: 'wl'+u+'_obss_coex', type: 'select', options: [['0','<% _("Off"); %> *'],['1','<% _("On"); %>']],
			value: nvram['wl'+u+'_obss_coex'], hidden: !nphy },
		{ title: '<% _("RTS Threshold"); %>', name: 'wl'+u+'_rts', type: 'text', maxlen: 4, size: 6,
			suffix: ' <small>(<% _("range"); %>: 0 - 2347; <% _("default"); %>: 2347)</small>', value: nvram['wl'+u+'_rts'] },
		{ title: '<% _("Receive Antenna"); %>', name: 'wl'+u+'_antdiv', type: 'select', options: [['3','<% _("Auto"); %> *'],['1','A'],['0','B']],
			value: nvram['wl'+u+'_antdiv'] },
		{ title: '<% _("Transmit Antenna"); %>', name: 'wl'+u+'_txant', type: 'select', options: [['3','<% _("Auto"); %> *'],['1','A'],['0','B']],
			value: nvram['wl'+u+'_txant'] },
		{ title: '<% _("Transmit Power"); %>', name: 'wl'+u+'_txpwr', type: 'text', maxlen: 3, size: 5,
			suffix: hp ?
				' <small>mW (<% _("before amplification"); %>)</small>&nbsp;&nbsp;<small>(<% _("range"); %>: 1 - 251; <% _("default"); %>: 10)</small>' :
				' <small>mW</small>&nbsp;&nbsp;<small>(<% _("range"); %>: 0 - 400, <% _("actual max depends on Country selected"); %>; <% _("use 0 for hardware default"); %>)</small>',
				value: nvram['wl'+u+'_txpwr'] },
		{ title: '<% _("Transmission Rate"); %>', name: 'wl'+u+'_rate', type: 'select',
			options: [['0','<% _("Auto"); %> *'],['1000000','1 Mbps'],['2000000','2 Mbps'],['5500000','5.5 Mbps'],['6000000','6 Mbps'],['9000000','9 Mbps'],['11000000','11 Mbps'],['12000000','12 Mbps'],['18000000','18 Mbps'],['24000000','24 Mbps'],['36000000','36 Mbps'],['48000000','48 Mbps'],['54000000','54 Mbps']],
			value: nvram['wl'+u+'_rate'] },
	{ title: '<% _("Interference Mitigation"); %>', name: 'wl'+u+'_mitigation', type: 'select',
		options: [['0','<% _("None"); %> *'],['1','<% _("Non-WLAN"); %>'],['2','<% _("WLAN Manual"); %>'],['3','<% _("WLAN Auto"); %>'],['4','<% _("WLAN Auto with Noise Reduction"); %>']],
		value: nvram['wl'+u+'_mitigation'] },
	{ title: '<% _("WMM"); %>', name: 'wl'+u+'_wme', type: 'select', options: [['auto','<% _("Auto"); %> *'],['off','<% _("Disable"); %>'],['on','<% _("Enable"); %>']], value: nvram['wl'+u+'_wme'] },
	{ title: '<% _("No ACK"); %>', name: 'wl'+u+'_wme_no_ack', indent: 2, type: 'select', options: [['off','<% _("Disable"); %> *'],['on','<% _("Enable"); %>']],
		value: nvram['wl'+u+'_wme_no_ack'] },
	{ title: '<% _("APSD Mode"); %>', name: 'wl'+u+'_wme_apsd', indent: 2, type: 'select', options: [['off','<% _("Disable"); %>'],['on','<% _("Enable"); %> *']],
		value: nvram['wl'+u+'_wme_apsd'] },
	{ title: '<% _("Wireless Multicast Forwarding"); %>', name: 'wl'+u+'_wmf_bss_enable', type: 'select', options: [['0','<% _("Disable"); %> *'],['1','<% _("Enable"); %>']],
		value: nvram['wl'+u+'_wmf_bss_enable'] }
	]);
	W('</div>');
	}
}
</script>

<small><% _("The default settings are indicated with an asterisk <b style='font-size: 1.5em'>*</b> symbol"); %>.</small>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='<% _("Save"); %>' id='save-button' onclick='save()'>
	<input type='button' value='<% _("Cancel"); %>' id='cancel-button' onclick='reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
