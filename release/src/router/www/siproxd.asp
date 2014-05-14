<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Portions Copyright (C) 2011 Ofer Chen, oferchen@gmail.com
	Copyright K2.6 Build by Vicente Soriano (Victek), victek@gmail.com 

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->


<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Siproxd</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<style type='text/css'>
.controls {
	width: 90px;
	margin-top: 5px;
	margin-bottom: 10px;
}
</style>

<style type='text/css'>
#siproxd-trueurl-grid .co1 <!-- scheme -->
#siproxd-trueurl-grid .co2 <!-- username -->
#siproxd-trueurl-grid .co3 <!-- host -->
#siproxd-trueurl-grid .co4 <!-- port -->

#siproxd-masqurl-grid .co5 <!-- scheme -->
#siproxd-masqurl-grid .co6 <!-- username -->
#siproxd-masqurl-grid .co7 <!-- host -->
#siproxd-masqurl-grid .co8 <!-- port -->

#siproxd-regurl-grid .co9 <!-- scheme -->
#siproxd-regurl-grid .co10 <!-- username -->
#siproxd-regurl-grid .co11 <!-- host -->
#siproxd-regurl-grid .co12 <!-- port -->

</style>

<script type='text/javascript' src='debug.js'></script>
<script type='text/javascript'>

//	<% nvram("siproxd_enable,siproxd_reboot,siproxd_if_inbound,siproxd_if_outbound,siproxd_intcpt,siproxd_listen_port,siproxd_default_expires,siproxd_daemonize,siproxd_autosave_registrations,siproxd_rtp_proxy,siproxd_rtp_port_low,siproxd_rtp_port_high,siproxd_rtp_timeout,siproxd_rtp_dscp,siproxd_sip_dscp,siproxd_silence_log,siproxd_debug_level,siproxd_debug_port,siproxd_logcall,siproxd_shortdial,siproxd_pi_shortdial_akey,siproxd_pi_shortdial1,siproxd_pi_shortdial2,siproxd_pi_shortdial3,siproxd_pi_shortdial4,siproxd_pi_shortdial5,wan_ifname,lan_ifname,wl_ifname"); %>
// <% siproxd_stat(); %>

changed = 0;
siproxdup = parseInt ('<% psup("siproxd"); %>');

function toggle(service, isup)
{
	if (changed) {
		if (!confirm("Unsaved changes will be lost. Continue anyway?")) return;
	}
	E('_' + service + '_button').disabled = true;
	form.submitHidden('/service.cgi', {
		_redirect: 'siproxd.asp',
		_sleep: ((service == 'siproxdfp') && (!isup)) ? '10' : '5',
		_service: service + (isup ? '-stop' : '-start')
	});
}

/*REMOVE-BEGIN
True Url
siproxd-grid .co1 <!-- true_url scheme (sip or sips) -->
siproxd-grid .co2 <!-- true_url username -->
siproxd-grid .co3 <!-- true_url host -->
siproxd-grid .co4 <!-- true_url port -->

Masq Url
siproxd-grid .co5 <!-- masq_url scheme  (sip or sips) -->
siproxd-grid .co6 <!-- masq_url username -->
siproxd-grid .co7 <!-- masq_url host -->
siproxd-grid .co8 <!-- masq_url port -->

Reg Url
siproxd-grid .co9 <!-- reg_url scheme  (sip or sips) -->
siproxd-grid .co10 <!-- reg_url username -->
siproxd-grid .co11 <!-- reg_url host -->
siproxd-grid .co12 <!-- reg_url port -->
REMOVE-END*/

var strueurlg = new TomatoGrid();
var smasqurlg = new TomatoGrid();
var sregurlg = new TomatoGrid();

sgsetup = function()
{
	strueurlg.init('strueurlg', 'sort');
	strueurlg.headerSet(['Protocol', 'Username', 'Host', 'Port']);

	smasqurlg.init('smasqurlg', 'sort');
	smasqurlg.headerSet(['Protocol', 'Username', 'Host', 'Port']);

	sregurlg.init('sregurlg', 'sort');
	sregurlg.headerSet(['Protocol', 'Username', 'Host', 'Port']);

	sgpopulate();
}

sgpopulate = function() {
	var i, j, r, data;
	var trueurlrow, masqurlrow, regurlrow;

	if ((nvram.siproxd_enable != 0) && (nvram.siproxd_logcall != 0)) {
		var data = siproxd_registration_data.split('\n');
		for (i = 0; i < data.length; ++i) {
			r = data[i].match(/^(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))\s+(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))\s+(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))$/);
/* REMOVE-BEGIN
						TU           TU        TU        TU              MU           MU        MU        MU               RU           RU        RU        RU
					(/^(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))\s+(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))\s+(sip|sips)\s+(\d+-\d+)\s+(.+?)\s+(\d+|\(null\))$/);
REMOVE-END */
				if (r == null) continue;
				trueurlrow = strueurlg.insertData(-1, [r[1], r[2], r[3], r[4]]);
				masqurlrow = smasqurlg.insertData(-1, [r[5], r[6], r[7], r[8]]);
				regurlrow = sregurlg.insertData(-1, [r[9], r[10], r[11], r[12]]);
			if (!r[0]) {
				for (j = 0; j < 12; ++j) {
					if ( j >= 0 || j < 4 ) { elem.addClass(trueurlrow.cells[j], 'disabled'); }
					if ( j >= 4 || j < 8 ) { elem.addClass(masqurlrow.cells[j], 'disabled'); }
					if ( j >= 8 || j < 12) { elem.addClass(regurlrow.cells[j], 'disabled'); }
				}
			}
		}
		strueurlg.sort(2);
		smasqurlg.sort(6);
		sregurlg.sort(10);
	}
}

function verifyFields(focused, quiet)
{
	var ok = 1;
	var a, b, c;
	var d;
	var p, i;

	var vis = {
		_f_siproxd_enable: 1,  /* Checkbox */
		_f_siproxd_if_inbound: 1, /* Static Interface List */
		_f_siproxd_if_outbound: 1, /* Static Interface List */
		_f_siproxd_intcpt: 1, /* siproxd intercept checkbox */
		_f_siproxd_listen_port: 1, /* Port */
		_f_siproxd_default_expires: 1, /* Length 2-4 */

		_f_siproxd_rtp_proxy: 1, /* Checkbox */
		_f_siproxd_rtp_port_low: 1, /* Port */
		_f_siproxd_rtp_port_high: 1, /* Port */
		_f_siproxd_rtp_timeout: 1, /* Length 0-4 */
		_f_siproxd_rtp_dscp: 1, /* Length 0-4 */
		_f_siproxd_sip_dscp: 1, /* Length 0-4 */

		_f_siproxd_logcall: 1, /* Checkbox */

		_f_siproxd_shortdial: 1, /* Checkbox */
		_f_siproxd_pi_shortdial_akey: 1, /* Length 0-4 */
		_f_siproxd_pi_shortdial1: 1, /* Length 0-16 */
		_f_siproxd_pi_shortdial2: 1, /* Length 0-16 */
		_f_siproxd_pi_shortdial3: 1, /* Length 0-16 */
		_f_siproxd_pi_shortdial4: 1, /* Length 0-16 */
		_f_siproxd_pi_shortdial5: 1, /* Length 0-16 */

		_f_siproxd_silence_log: 1, /* Range 0-4 */
		_f_siproxd_debug_level: 1, /* 0-12 */
		_f_siproxd_debug_port: 1, /* TCP Port, 0 Disable */
	}

		if (!E('_f_siproxd_enable').checked) {
			for (i in vis) {
					vis[i] = 2;
			}
			vis._f_siproxd_enable = 1;
		}

		if (!E('_f_siproxd_rtp_proxy').checked) {
			vis._f_siproxd_rtp_port_low = 0;
			vis._f_siproxd_rtp_port_high = 0;
			vis._f_siproxd_rtp_timeout = 0;
			vis._f_siproxd_rtp_dscp = 0;
			vis._f_siproxd_sip_dscp = 0;
		};

		if (!E('_f_siproxd_shortdial').checked) {
			vis._f_siproxd_pi_shortdial_akey = 0;
			vis._f_siproxd_pi_shortdial1 = 0;
			vis._f_siproxd_pi_shortdial2 = 0;
			vis._f_siproxd_pi_shortdial3 = 0;
			vis._f_siproxd_pi_shortdial4 = 0;
			vis._f_siproxd_pi_shortdial5 = 0;
		}

	/* check debug port field when debug is active, remove field when debug is disabled */
	d = E('_f_siproxd_debug_level').value == 0;
	if (d) {
		vis._f_siproxd_debug_port = 0;
	}

	for (a in vis) {
		b = E(a);
		c = vis[a];
		b.disabled = (c != 1);
		PR(b).style.display = c ? '' : 'none';
	}

	E('logcall-section').style.visibility = ((!E('_f_siproxd_logcall').checked) || (!E('_f_siproxd_enable').checked)) ? 'hidden' : 'visible';

 /* prevent network device loopback */
		if ('_f_siproxd_if_inbound' == '_f_siproxd_if_outbound') {
			ok = 0;
			W('inbound interface can\'t be the same as outbound interface, we\'re not that smart yet');
		}

		if (!v_port('_f_siproxd_listen_port', quiet)) {
			ok =0;
			W('listen port value is out of bounds');
		}
		
		if (!v_length('_f_siproxd_default_expires',quiet, 2, 4)) {
			ok = 0;
			W('default expire value is out of bounds');
		}

		if ((!v_port('_f_siproxd_rtp_port_low', quiet)) || (!v_port('_f_siproxd_rtp_port_high', quiet))) {
			ok = 0;
			W('RTP ports are out of bounds');
		}
		
		if (!v_length('_f_siproxd_rtp_timeout',quiet, 0, 4)) {
			ok = 0;
			W('RTP timeout is out of bounds');
		}
		
		if (!v_length('_f_siproxd_rtp_dscp',quiet, 0, 4))  {
			ok = 0;
			W('RTP DSCP value is out of bounds');
		}
		
		if (!v_length('_f_siproxd_sip_dscp',quiet, 0, 4)) {
			ok = 0;
			W('SIP DSCP value is out of bounds');
		}
		
		
/* prevent port range overlap */
		if (_f_siproxd_rtp_port_low.value >= _f_siproxd_rtp_port_high.value) {
			ok = 0;
			W('RTP ports values are inconsistent');
		}

 /* reset variable to prevent error in config file */
		if ('_f_siproxd_rtp_dscp' == null) {
			W('RTP DSCP is null!');
		}
		
 /* reset variable to prevent error in config file */
		if ('_f_siproxd_sip_dscp' == null) {
			W('SIP DSCP is null!');
		}
		
		if (E('_f_siproxd_shortdial').checked) {
			for (i = 1; i <=5; i++) {
				p = E('_f_siproxd_pi_shortdial' + i);
				if (!v_length(p, quiet, 0, 16)) {
					ok = 0;
					W('Shortdial entries are too long');
				}
			}
			if (!v_length('_f_siproxd_pi_shortdial_akey', quiet, 0, 4)) {
				ok = 0;
				W('Shortdial akey value is too long');
			}
		}

/* prevent out of range debug port */
	if ((!d) && (!v_range('_f_siproxd_debug_port', quiet, 0, 65535))) {
		ok = 0;
		W('Debug port value is out of bounds');
	}

	changed |= ok;
	return ok;
}


/* REMOVE-BEGIN
siproxd_enable
siproxd_if_inbound
siproxd_if_outbound
siproxd_intcpt
siproxd_listen_port
siproxd_default_expires

siproxd_rtp_proxy
siproxd_rtp_port_low
siproxd_rtp_port_high
siproxd_rtp_timeout
siproxd_rtp_dscp
siproxd_sip_dscp

siproxd_logcall
siproxd_shortdial
siproxd_pi_shortdial_akey
siproxd_pi_shortdial1
siproxd_pi_shortdial2
siproxd_pi_shortdial3
siproxd_pi_shortdial4
siproxd_pi_shortdial5

siproxd_silence_log
siproxd_debug_level
siproxd_debug_port
REMOVE-END */

function save()
{
/*	var en,rtp,sdpi;  en - siproxd enable, rtp - rtp proxy, sdpi - shortdial plugin */
	var fom = E('_fom');

	if (!verifyFields(null, 0)) {
		W('config failed');
		return;
	}

	fom.siproxd_enable.value = E('_f_siproxd_enable').checked ? 1 : 0;
	if (fom.siproxd_enable.value) {
		fom.siproxd_if_inbound.value = fom.f_siproxd_if_inbound.value;
		fom.siproxd_if_outbound.value = fom.f_siproxd_if_outbound.value;
		fom.siproxd_intcpt.value = E('_f_siproxd_intcpt').checked ? 1 : 0;
		fom.siproxd_listen_port.value = fom.f_siproxd_listen_port.value;
		fom.siproxd_default_expires.value = fom.f_siproxd_default_expires.value;
		fom._service.value = 'siproxd-restart';
	} else {
		fom._service.value = 'siproxd-stop';
	}

	fom.siproxd_rtp_proxy.value = E('_f_siproxd_rtp_proxy').checked ? 1 : 0;
	if (fom.siproxd_rtp_proxy.value) {
		fom.siproxd_rtp_port_low.value = fom.f_siproxd_rtp_port_low.value;
		fom.siproxd_rtp_port_high.value = fom.f_siproxd_rtp_port_high.value;
		fom.siproxd_rtp_timeout.value = fom.f_siproxd_rtp_timeout.value;
		fom.siproxd_rtp_dscp.value = fom.f_siproxd_rtp_dscp.value;
		fom.siproxd_sip_dscp.value = fom.f_siproxd_sip_dscp.value;
	}

	fom.siproxd_logcall.value = E('_f_siproxd_logcall').checked ? 1 : 0;

	fom.siproxd_shortdial.value = E('_f_siproxd_shortdial').checked ? 1 : 0;
	if (fom.siproxd_shortdial.value) {
		fom.siproxd_pi_shortdial_akey.value = fom.f_siproxd_pi_shortdial_akey.value;
		fom.siproxd_pi_shortdial1.value = fom.f_siproxd_pi_shortdial1.value;
		fom.siproxd_pi_shortdial2.value = fom.f_siproxd_pi_shortdial2.value;
		fom.siproxd_pi_shortdial3.value = fom.f_siproxd_pi_shortdial3.value;
		fom.siproxd_pi_shortdial4.value = fom.f_siproxd_pi_shortdial4.value;
		fom.siproxd_pi_shortdial5.value = fom.f_siproxd_pi_shortdial5.value;
	}

		fom.siproxd_debug_level.value = fom.f_siproxd_debug_level.value;
		fom.siproxd_debug_port.value = fom.f_siproxd_debug_port.value;

	fom.siproxd_silence_log.value = fom.f_siproxd_silence_log.value;

/* reboot mode - do not try to stop,start services just reboot the unit */
if (nvram.siproxd_reboot == '0') { 
		fom._reboot.value = 0;
		form.submit(fom, 1);
	} else {
		fom._reboot.value = 1;
		form.submit(fom, 0);
	}
	changed = 0;
}

function init()
{
	strueurlg.recolor();
	smasqurlg.recolor();
	sregurlg.recolor();
}

</script>

</head>
<body onload='init()'>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato RAF</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->


<div class='section-title'>Status</div>
 <div class='section' id='status-section'>
   <script type='text/javascript'>
W('Siproxd is currently '+(!siproxdup ? 'stopped' : 'running')+' ');
W('<input type="button" value="' + (siproxdup ? 'Stop' : 'Start') + ' Now" onclick="toggle(\'siproxdfp\', siproxdup)" id="_siproxdfp_button">');
  </script>
  <br>
 </div>
</div>
<input type='hidden' name='_nextpage' value='siproxd.asp'>
<input type='hidden' name='_nextwait' value='10'>
<input type='hidden' name='_service' value='siproxd-restart'>
<input type='hidden' name='_reboot' value='0'>

<input type='hidden' id='siproxd_enable' name='siproxd_enable'>
<input type='hidden' id='siproxd_if_inbound' name='siproxd_if_inbound'>
<input type='hidden' id='siproxd_if_outbound' name='siproxd_if_outbound'>
<input type='hidden' id='siproxd_intcpt' name='siproxd_intcpt'>
<input type='hidden' id='siproxd_listen_port' name='siproxd_listen_port'>
<input type='hidden' id='siproxd_default_expires' name='siproxd_default_expires'>

<input type='hidden' id='siproxd_rtp_proxy' name='siproxd_rtp_proxy'>
<input type='hidden' id='siproxd_rtp_port_low' name='siproxd_rtp_port_low'>
<input type='hidden' id='siproxd_rtp_port_high' name='siproxd_rtp_port_high'>
<input type='hidden' id='siproxd_rtp_timeout' name='siproxd_rtp_timeout'>
<input type='hidden' id='siproxd_rtp_dscp' name='siproxd_rtp_dscp'>
<input type='hidden' id='siproxd_sip_dscp' name='siproxd_sip_dscp'>

<input type='hidden' id='siproxd_logcall' name='siproxd_logcall'>
<input type='hidden' id='siproxd_shortdial' name='siproxd_shortdial'>
<input type='hidden' id='siproxd_pi_shortdial_akey' name='siproxd_pi_shortdial_akey'>
<input type='hidden' id='siproxd_pi_shortdial1' name='siproxd_pi_shortdial1'>
<input type='hidden' id='siproxd_pi_shortdial2' name='siproxd_pi_shortdial2'>
<input type='hidden' id='siproxd_pi_shortdial3' name='siproxd_pi_shortdial3'>
<input type='hidden' id='siproxd_pi_shortdial4' name='siproxd_pi_shortdial4'>
<input type='hidden' id='siproxd_pi_shortdial5' name='siproxd_pi_shortdial5'>

<input type='hidden' id='siproxd_silence_log' name='siproxd_silence_log'>
<input type='hidden' id='siproxd_debug_level' name='siproxd_debug_level'>
<input type='hidden' id='siproxd_debug_port' name='siproxd_debug_port'>

<div class='section-title'>Configuration</div>
 <div class='section' id='config-section'>
 <script type='text/javascript'>
   createFieldTable('', [
   	{ title: 'Siproxd on Startup', name: 'f_siproxd_enable', type: 'checkbox', value: (nvram.siproxd_enable == '1'), suffix: ' <small> default: on</small>' },
   	{ title: 'Inbound Interface', name: 'f_siproxd_if_inbound', type: 'select', options: [[nvram.wan_ifname,'WAN - '+nvram.wan_ifname],[nvram.lan_ifname,'LAN - '+nvram.lan_ifname],[nvram.wl_ifname,'WiFi - '+nvram.wl_ifname]], value: (nvram.siproxd_if_inbound), suffix: ' <small> default: WAN - '+nvram.wan_ifname+'</small>' },
   	{ title: 'Outbound Interface', name: 'f_siproxd_if_outbound', type: 'select', options: [[nvram.wan_ifname,'WAN - '+nvram.wan_ifname],[nvram.lan_ifname,'LAN - '+nvram.lan_ifname],[nvram.wl_ifname,'WiFi - '+nvram.wl_ifname]], value: (nvram.siproxd_if_outbound), suffix: ' <small> default: LAN - '+nvram.lan_ifname+'</small>' },
   	{ title: 'Intercept outgoing SIP traffic to Siproxd', name: 'f_siproxd_intcpt', type: 'checkbox', value: (nvram.siproxd_intcpt =='1'), suffix:' <small> default: off, intercepted port is listen port tcp/udp</small>' },
   	{ title: 'SIP Listen Port', name: 'f_siproxd_listen_port', type: 'text', maxlen: 5, size: 7, value: (fixPort(nvram.siproxd_listen_port)), suffix: ' <small> default: 5060</small>' },
   	{ title: 'Default Expires', name: 'f_siproxd_default_expires', type:'text', maxlen: 4, size: 6, value: (nvram.siproxd_default_expires), suffix: ' <small> default: 600</small>' },
   	null,
   	{ title: 'RTP Proxy', name: 'f_siproxd_rtp_proxy', type: 'checkbox', value: (nvram.siproxd_rtp_proxy == '1') , suffix: ' <small> default: on</small>' },
   	{ title: 'RTP Proxy Port Range', multi: [
   		{ name: 'f_siproxd_rtp_port_low', type: 'text', maxlen: 5, size: 7, value: (fixPort(nvram.siproxd_rtp_port_low)), suffix: ' - ' },
   		{ name: 'f_siproxd_rtp_port_high', type: 'text', maxlen: 5, size: 7, value: (fixPort(nvram.siproxd_rtp_port_high)) , suffix:' <small> default port range: 10000-10100</small>' },
   		] },
   	{ title: 'RTP timeout', name: 'f_siproxd_rtp_timeout', type: 'text', maxlen: 4, size:6, value: (nvram.siproxd_rtp_timeout), suffix: ' <small> default: 300</small>' },
   	{ title: 'RTP DSCP', name: 'f_siproxd_rtp_dscp', type: 'text', maxlen: 4, size: 6, value: (nvram.siproxd_rtp_dscp), suffix: ' <small> default: 46</small>' },
   	{ title: 'SIP DSCP', name: 'f_siproxd_sip_dscp', type: 'text', maxlen: 4, size: 6, value: (nvram.siproxd_sip_dscp), suffix: ' <small> default: 0</small>' },
   	null,
   	{ title: 'Logcall Plugin', name: 'f_siproxd_logcall', type: 'checkbox', value: (nvram.siproxd_logcall == '1'), suffix: ' <small> default: on</small>' },
   	{ title: 'Shortdial Plugin', name: 'f_siproxd_shortdial', type: 'checkbox', value: (nvram.siproxd_shortdial == '1'), suffix: ' <small> default: on</small>' },
   	{ title: 'Shortdial A Key', name: 'f_siproxd_pi_shortdial_akey', type: 'text', maxlen: 4, size: 6, value: (nvram.siproxd_pi_shortdial_akey), ignore: (nvram.shortdial == '1') },
   	{ title: 'Shortdial Entry #1', name: 'f_siproxd_pi_shortdial1', type: 'text', maxlen: 16, size: 18, value: (nvram.siproxd_pi_shortdial1), ignore: (nvram.shortdial == '1') },
   	{ title: 'Shortdial Entry #2', name: 'f_siproxd_pi_shortdial2', type: 'text', maxlen: 16, size: 18, value: (nvram.siproxd_pi_shortdial2), ignore: (nvram.shortdial == '1') },
   	{ title: 'Shortdial Entry #3', name: 'f_siproxd_pi_shortdial3', type: 'text', maxlen: 16, size: 18, value: (nvram.siproxd_pi_shortdial3), ignore: (nvram.shortdial == '1') },
   	{ title: 'Shortdial Entry #4', name: 'f_siproxd_pi_shortdial4', type: 'text', maxlen: 16, size: 18, value: (nvram.siproxd_pi_shortdial4), ignore: (nvram.shortdial == '1') },
   	{ title: 'Shortdial Entry #5', name: 'f_siproxd_pi_shortdial5', type: 'text', maxlen: 16, size: 18, value: (nvram.siproxd_pi_shortdial5), ignore: (nvram.shortdial == '1') },
   	null,
   	{ title: 'Log Verbosity', name: 'f_siproxd_silence_log', type: 'select',
   		 options: [['4', 'absolutely nothing'],['3', 'only errors'],['2', 'warnings and errors'],['1', 'info, warnings and errors'],['0', 'debugs, info, warnings and errors']],
   		value: (nvram.siproxd_silence_log), suffix: ' <small> default: only errors</small>' },
   	{ title: 'Debug Level', name: 'f_siproxd_debug_level', type: 'select', options: [[0,'<b>Disabled</b>'],[1,'Babble'],[2,'Network'],[3,'SIP Manipulations'],[4,'Client Registration'],[5,'Non Specific Class'],[6,'Proxy'],[7,'DNS Stuff'],
   	[8,'Network Traffic'],[9,'Configuration'],[10,'RTP Proxy'],[11,'Access List Evaluation'],[12,'Authentication']], value: (fixInt (nvram.siproxd_debug_level, 0)), suffix: ' <small> default: disabled</small>' },
   	{ title: 'Debug Port', hidden: 1, name: 'f_siproxd_debug_port', type: 'text', maxlen: 5, size: 6, value: (fixInt(nvram.siproxd_debug_port)), hidden: 1, suffix: ' <small> default: 0 - disabled </small>' },
   ]);
 /* REMOVE-BEGIN 
 0 0x00000000 debug disabled
 1 0x00000001 babble (like entering/leaving func)
 2 0x00000002 network
 3 0x00000004 SIP manipulations
 4 0x00000008 Client registration
 5 0x00000010 non specified class
 6 0x00000020 proxy
 7 0x00000040 DNS stuff
 8 0x00000080 network traffic
 9 0x00000100 configuration
 10 0x00000200 RTP proxy
 11 0x00000400 Access list evaluation
 12 0x00000800 Authentication
 REMOVE-END */
  </script>
  <br><br>
 </div>
</div>

<div class='section' id='logcall-section'>
 <div class='section-title'>SIP Registrations</div>
  <div class='section'>
  <h3>True URL</h3>
	<table id='strueurlg' class='tomato-grid'></table>
	<div style='width: 100%; text-align: right'> </div>
	<script type='text/javascript'>
	</script>
	<br><br>
  </div>
  <div class='section'>
    <h3>Masquerade URL</h3>
	<table id='smasqurlg' class='tomato-grid'></table>
	<div style='width: 100%; text-align: right'></div>
	<script type='text/javascript'>
	</script>
	<br><br>
  </div>
  <div class='section'>
    <h3>Registered URL</h3>
	<table id='sregurlg' class='tomato-grid'></table>
	<script type='text/javascript'>
	</script>
	<br><br>
  </div>
 </div>
</div>

<script>
</script>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
	<input type='button' value='Refresh' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>sgsetup();</script>
</body>
</html>
