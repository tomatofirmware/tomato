<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Tomato VLAN GUI
	Copyright (C) 2011 Augusto Bott
	http://code.google.com/p/tomato-sdhc-vlan/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Status: Przegląd</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript' src='interfaces.js'></script>

<!-- / / / -->

<style type='text/css'>
.controls {
	width: 90px;
	margin-top: 5px;
	margin-bottom: 10px;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvstat(); %>

wmo = {'ap':'Punkt dostępu','sta':'Klient WiFi','wet':'Most WiFi (bridge)','wds':'WDS'};
auth = {'disabled':'-','wep':'WEP','wpa_personal':'WPA Personal (PSK)','wpa_enterprise':'WPA Enterprise','wpa2_personal':'WPA2 Personal (PSK)','wpa2_enterprise':'WPA2 Enterprise','wpaX_personal':'WPA / WPA2 Personal','wpaX_enterprise':'WPA / WPA2 Enterprise','radius':'Radius'};
enc = {'tkip':'TKIP','aes':'AES','tkip+aes':'TKIP / AES'};
bgmo = {'disabled':'-','mixed':'Auto','b-only':'Tylko B','g-only':'Tylko G','bg-mixed':'Mieszane B/G','lrs':'LRS','n-only':'Tylko N'};
</script>

<script type='text/javascript' src='wireless.jsx?_http_id=<% nv(http_id); %>'></script>
<script type='text/javascript' src='status-data.jsx?_http_id=<% nv(http_id); %>'></script>

<script type='text/javascript'>
show_dhcpc = ((nvram.wan_proto == 'dhcp') || (((nvram.wan_proto == 'l2tp') || (nvram.wan_proto == 'pptp')) && (nvram.pptp_dhcp == '1')));
show_codi = ((nvram.wan_proto == 'pppoe') || (nvram.wan_proto == 'l2tp') || (nvram.wan_proto == 'pptp') || (nvram.wan_proto == 'ppp3g'));

show_radio = [];
for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
/* REMOVE-BEGIN
//	show_radio.push((nvram['wl'+wl_unit(uidx)+'_radio'] == '1'));
REMOVE-END */
	if (wl_sunit(uidx)<0)
		show_radio.push((nvram['wl'+wl_fface(uidx)+'_radio'] == '1'));
}

nphy = features('11n');

function dhcpc(what)
{
	form.submitHidden('dhcpc.cgi', { exec: what, _redirect: 'status-overview.asp' });
}

function serv(service, sleep)
{
	form.submitHidden('service.cgi', { _service: service, _redirect: 'status-overview.asp', _sleep: sleep });
}

function wan_connect()
{
	serv('wan-restart', 5);
}

function wan_disconnect()
{
	serv('wan-stop', 2);
}

function wlenable(uidx, n)
{
	form.submitHidden('wlradio.cgi', { enable: '' + n, _nextpage: 'status-overview.asp', _nextwait: n ? 6 : 3, _wl_unit: wl_unit(uidx) });
}

var ref = new TomatoRefresh('status-data.jsx', '', 0, 'status_overview_refresh');

ref.refresh = function(text)
{
	stats = {};
	try {
		eval(text);
	}
	catch (ex) {
		stats = {};
	}
	show();
}


function c(id, htm)
{
	E(id).cells[1].innerHTML = htm;
}

function show()
{
	c('cpu', stats.cpuload);
	c('uptime', stats.uptime);
	c('time', stats.time);
	c('wanip', stats.wanip);
	c('wannetmask', stats.wannetmask);
	c('wangateway', stats.wangateway);
	c('dns', stats.dns);
	c('memory', stats.memory);
	c('swap', stats.swap);
	elem.display('swap', stats.swap != '');

/* IPV6-BEGIN */
	c('ip6_wan', stats.ip6_wan);
	elem.display('ip6_wan', stats.ip6_wan != '');
	c('ip6_lan', stats.ip6_lan);
	elem.display('ip6_lan', stats.ip6_lan != '');
	c('ip6_lan_ll', stats.ip6_lan_ll);
	elem.display('ip6_lan_ll', stats.ip6_lan_ll != '');
/* IPV6-END */

	c('wanstatus', stats.wanstatus);
	c('wanuptime', stats.wanuptime);
	if (show_dhcpc) c('wanlease', stats.wanlease);
	if (show_codi) {
		E('b_connect').disabled = stats.wanup;
		E('b_disconnect').disabled = !stats.wanup;
	}

	for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
		if (wl_sunit(uidx)<0) {
			c('radio'+uidx, wlstats[uidx].radio ? 'Włączone' : '<b>Wyłączone</b>');
			c('rate'+uidx, wlstats[uidx].rate);
			if (show_radio[uidx]) {
				E('b_wl'+uidx+'_enable').disabled = wlstats[uidx].radio;
				E('b_wl'+uidx+'_disable').disabled = !wlstats[uidx].radio;
			}
			c('channel'+uidx, stats.channel[uidx]);
			if (nphy) {
				c('nbw'+uidx, wlstats[uidx].nbw);
			}
			c('interference'+uidx, stats.interference[uidx]);
			elem.display('interference'+uidx, stats.interference[uidx] != '');

			if (wlstats[uidx].client) {
				c('rssi'+uidx, wlstats[uidx].rssi || '');
				c('noise'+uidx, wlstats[uidx].noise || '');
				c('qual'+uidx, stats.qual[uidx] || '');
			}
		}
		c('ifstatus'+uidx, wlstats[uidx].ifstatus || '');
	}
}

function earlyInit()
{
	if ((stats.anon_enable == '-1') || (stats.anon_answer == '0'))
		E('notice1').style.display = '';

	elem.display('b_dhcpc', show_dhcpc);
	elem.display('b_connect', 'b_disconnect', show_codi);
	if (nvram.wan_proto == 'disabled')
		elem.display('wan-title', 'sesdiv_wan', 0);
	for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
		if (wl_sunit(uidx)<0)
			elem.display('b_wl'+uidx+'_enable', 'b_wl'+uidx+'_disable', show_radio[uidx]);
	}
	show();
}

function init()
{
	var c;
	if (((c = cookie.get('status_overview_system_vis')) != null) && (c != '1')) toggleVisibility("system");
	if (((c = cookie.get('status_overview_wan_vis')) != null) && (c != '1')) toggleVisibility("wan");
	if (((c = cookie.get('status_overview_lan_vis')) != null) && (c != '1')) toggleVisibility("lan");
	for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
		u = wl_unit(uidx);
		if (((c = cookie.get('status_overview_wl_'+u+'_vis')) != null) && (c != '1')) toggleVisibility("wl_"+u);
	}
	ref.initPage(3000, 3);
}

function toggleVisibility(whichone) {
	if (E('sesdiv_' + whichone).style.display == '') {
		E('sesdiv_' + whichone).style.display = 'none';
		E('sesdiv_' + whichone + '_showhide').innerHTML = '(pokaż)';
		cookie.set('status_overview_' + whichone + '_vis', 0);
	} else {
		E('sesdiv_' + whichone).style.display='';
		E('sesdiv_' + whichone + '_showhide').innerHTML = '(ukryj)';
		cookie.set('status_overview_' + whichone + '_vis', 1);
	}
}

</script>

</head>
<body onload='init()'>
<form>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->
<div style='display:none' id='notice1'>
<div class='section-title'><font color="red"><center>!! Attention !!</center></font></div>
<div class='section'><center><font color="black">You did not configure <b>TomatoAnon project</b> setting.
<br>Please go to <a style="color: black;" href='admin-tomatoanon.asp'><b>TomatoAnon configuration page</b></a> and make a choice.</font></center></div>
</div>

<div class='section-title'>System <small><i><a href='javascript:toggleVisibility("system");'><span id='sesdiv_system_showhide'>(ukryj)</span></a></i></small></div>
<div class='section' id='sesdiv_system'>
<script type='text/javascript'>
var a = nvstat.free / nvstat.size * 100.0;
createFieldTable('', [
	{ title: 'Nazwa', text: nvram.router_name },
	{ title: 'Model', text: nvram.t_model_name },
	{ title: 'Chipset', text: stats.systemtype },
	{ title: 'Zegar CPU', text: stats.cpumhz },
	{ title: 'Rozmiar pamięci Flash', text: stats.flashsize },
	null,
	{ title: 'Czas systemowy', rid: 'time', text: stats.time },
	{ title: 'Uruchomiony od', rid: 'uptime', text: stats.uptime },
	{ title: 'Obciążenie CPU <br><small>(1 / 5 / 15 min.)</small>', rid: 'cpu', text: stats.cpuload },
	{ title: 'Całkowita/Wolna pamięć', rid: 'memory', text: stats.memory },
	{ title: 'Całkowity/Wolny SWAP', rid: 'swap', text: stats.swap, hidden: (stats.swap == '') },
	{ title: 'Całkowity/Wolny NVRAM', text: scaleSize(nvstat.size) + ' / ' + scaleSize(nvstat.free) + ' <small>(' + (a).toFixed(2) + '%)</small>' }
]);
</script>
</div>

<div class='section-title' id='wan-title'>WAN <small><i><a href='javascript:toggleVisibility("wan");'><span id='sesdiv_wan_showhide'>(ukryj)</span></a></i></small></div>
<div class='section' id='sesdiv_wan'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Adres MAC', text: nvram.wan_hwaddr },
	{ title: 'Typ połączenia', text: { 'dhcp':'DHCP', 'static':'Statyczny IP', 'pppoe':'PPPoE', 'pptp':'PPTP', 'l2tp':'L2TP', 'ppp3g':'Modem 3G' }[nvram.wan_proto] || '-' },
	{ title: 'Adres IP', rid: 'wanip', text: stats.wanip },
	{ title: 'Maska podsieci', rid: 'wannetmask', text: stats.wannetmask },
	{ title: 'Brama', rid: 'wangateway', text: stats.wangateway },
/* IPV6-BEGIN */
	{ title: 'Adres IPv6', rid: 'ip6_wan', text: stats.ip6_wan, hidden: (stats.ip6_wan == '') },
/* IPV6-END */
	{ title: 'DNS', rid: 'dns', text: stats.dns },
	{ title: 'MTU', text: nvram.wan_run_mtu },
	null,
	{ title: 'Status', rid: 'wanstatus', text: stats.wanstatus },
	{ title: 'Podłączony od', rid: 'wanuptime', text: stats.wanuptime },
	{ title: 'Pozostały czas dzierżawy', rid: 'wanlease', text: stats.wanlease, ignore: !show_dhcpc }
]);
</script>
<span id='b_dhcpc' style='display:none'>
	<input type='button' class='controls' onclick='dhcpc("renew")' value='Odnów IP'>
	<input type='button' class='controls' onclick='dhcpc("release")' value='Zwolnij IP'> &nbsp;
</span>
<input type='button' class='controls' onclick='wan_connect()' value='Połącz' id='b_connect' style='display:none'>
<input type='button' class='controls' onclick='wan_disconnect()' value='Rozłącz' id='b_disconnect' style='display:none'>
</div>


<div class='section-title'>LAN <small><i><a href='javascript:toggleVisibility("lan");'><span id='sesdiv_lan_showhide'>(ukryj)</span></a></i></small></div>
<div class='section' id='sesdiv_lan'>
<script type='text/javascript'>

function h_countbitsfromleft(num) {
	if (num == 255 ){
		return(8);
	}
	var i = 0;
	var bitpat=0xff00; 
	while (i < 8){
		if (num == (bitpat & 0xff)){
			return(i);
		}
		bitpat=bitpat >> 1;
		i++;
	}
	return(Number.NaN);
}

function numberOfBitsOnNetMask(netmask) {
	var total = 0;
	var t = netmask.split('.');
	for (var i = 0; i<= 3 ; i++) {
		total += h_countbitsfromleft(t[i]);
	}
	return total;
}

var s='';
var t='';
for (var i = 0 ; i <= MAX_BRIDGE_ID ; i++) {
	var j = (i == 0) ? '' : i.toString();
	if (nvram['lan' + j + '_ifname'].length > 0) {
		if (nvram['lan' + j + '_proto'] == 'dhcp') {
			if ((!fixIP(nvram.dhcpd_startip)) || (!fixIP(nvram.dhcpd_endip))) {
				var x = nvram['lan' + j + '_ipaddr'].split('.').splice(0, 3).join('.') + '.';
				nvram['dhcpd' + j + '_startip'] = x + nvram['dhcp' + j + '_start'];
				nvram['dhcpd' + j + '_endip'] = x + ((nvram['dhcp' + j + '_start'] * 1) + (nvram['dhcp' + j + '_num'] * 1) - 1);
			}
			s += ((s.length>0)&&(s.charAt(s.length-1) != ' ')) ? '<br>' : '';
			s += '<b>br' + i + '</b> (LAN' + j + ') - ' + nvram['dhcpd' + j + '_startip'] + ' - ' + nvram['dhcpd' + j + '_endip'];
		} else {
			s += ((s.length>0)&&(s.charAt(s.length-1) != ' ')) ? '<br>' : '';
			s += '<b>br' + i + '</b> (LAN' + j + ') - wyłączony';
		}
		t += ((t.length>0)&&(t.charAt(t.length-1) != ' ')) ? '<br>' : '';
		t += '<b>br' + i + '</b> (LAN' + j + ') - ' + nvram['lan' + j + '_ipaddr'] + '/' + numberOfBitsOnNetMask(nvram['lan' + j + '_netmask']);
	}
}

createFieldTable('', [
	{ title: 'Adres MAC routera', text: nvram.et0macaddr },
	{ title: 'Adres IP routera', text: t },
	{ title: 'Brama', text: nvram.lan_gateway, ignore: nvram.wan_proto != 'disabled' },
/* IPV6-BEGIN */
	{ title: 'Adres IPv6 routera', rid: 'ip6_lan', text: stats.ip6_lan, hidden: (stats.ip6_lan == '') },
	{ title: 'Adres IPv6 Link-local', rid: 'ip6_lan_ll', text: stats.ip6_lan_ll, hidden: (stats.ip6_lan_ll == '') },
/* IPV6-END */
	{ title: 'DNS', rid: 'dns', text: stats.dns, ignore: nvram.wan_proto != 'disabled' },
	{ title: 'DHCP', text: s }
]);

</script>
</div>

<script type='text/javascript'>
for (var uidx = 0; uidx < wl_ifaces.length; ++uidx) {
/* REMOVE-BEGIN
//	u = wl_unit(uidx);
REMOVE-END */
	u = wl_fface(uidx);
	W('<div class=\'section-title\' id=\'wl'+u+'-title\'>Sieć bezprzewodowa');
	if (wl_ifaces.length > 0)
		W(' (' + wl_display_ifname(uidx) + ')');
	W(' <small><i><a href=\'javascript:toggleVisibility("wl_' + u + '");\'><span id=\'sesdiv_wl_' +u + '_showhide\'>(ukryj)</span></a></i></small>');
	W('</div>');
	W('<div class=\'section\' id=\'sesdiv_wl_'+u+'\'>');
	sec = auth[nvram['wl'+u+'_security_mode']] + '';
	if (sec.indexOf('WPA') != -1) sec += ' + ' + enc[nvram['wl'+u+'_crypto']];

	wmode = wmo[nvram['wl'+u+'_mode']] + '';
	if ((nvram['wl'+u+'_mode'] == 'ap') && (nvram['wl'+u+'_wds_enable'] * 1)) wmode += ' + WDS';

	createFieldTable('', [
		{ title: 'Adres MAC', text: nvram['wl'+u+'_hwaddr'] },
		{ title: 'Tryb pracy', text: wmode },
		{ title: 'Standard Wi-Fi', text: bgmo[nvram['wl'+u+'_net_mode']], ignore: (wl_sunit(uidx)>=0) },
		{ title: 'Status interfejsu', rid: 'ifstatus'+uidx, text: wlstats[uidx].ifstatus },
		{ title: 'Status radia', rid: 'radio'+uidx, text: (wlstats[uidx].radio == 0) ? '<b>Wyłączone</b>' : 'Włączone', ignore: (wl_sunit(uidx)>=0) },
/* REMOVE-BEGIN */
//	{ title: 'SSID', text: (nvram['wl'+u+'_ssid'] + ' <small><i>' + ((nvram['wl'+u+'_mode'] != 'ap') ? '' : ((nvram['wl'+u+'_closed'] == 0) ? '(Rozgłaszanie włączone)' : '(Rozgłaszanie wyłączone)')) + '</i></small>') },
/* REMOVE-END */
		{ title: 'SSID', text: nvram['wl'+u+'_ssid'] },
		{ title: 'Rozgłaszanie SSID', text: (nvram['wl'+u+'_closed'] == 0) ? 'Włączone' : '<b>Wyłączone</b>', ignore: (nvram['wl'+u+'_mode'] != 'ap') },
		{ title: 'Zabezpieczenie', text: sec },
		{ title: 'Kanał', rid: 'channel'+uidx, text: stats.channel[uidx], ignore: (wl_sunit(uidx)>=0) },
		{ title: 'Szerokość kanału', rid: 'nbw'+uidx, text: wlstats[uidx].nbw, ignore: ((!nphy) || (wl_sunit(uidx)>=0)) },
		{ title: 'Poziom zakłóceń', rid: 'interference'+uidx, text: stats.interference[uidx], hidden: ((stats.interference[uidx] == '') || (wl_sunit(uidx)>=0)) },
		{ title: 'Prędkość', rid: 'rate'+uidx, text: wlstats[uidx].rate, ignore: (wl_sunit(uidx)>=0) },
		{ title: 'RSSI', rid: 'rssi'+uidx, text: wlstats[uidx].rssi || '', ignore: ((!wlstats[uidx].client) || (wl_sunit(uidx)>=0)) },
		{ title: 'Poziom szumów', rid: 'noise'+uidx, text: wlstats[uidx].noise || '', ignore: ((!wlstats[uidx].client) || (wl_sunit(uidx)>=0)) },
		{ title: 'Jakość sygnału', rid: 'qual'+uidx, text: stats.qual[uidx] || '', ignore: ((!wlstats[uidx].client) || (wl_sunit(uidx)>=0)) }
	]);

	W('<input type=\'button\' class=\'controls\' onclick=\'wlenable('+uidx+', 1)\' id=\'b_wl'+uidx+'_enable\' value=\'Włącz\' style=\'display:none\'>');
	W('<input type=\'button\' class=\'controls\' onclick=\'wlenable('+uidx+', 0)\' id=\'b_wl'+uidx+'_disable\' value=\'Wyłącz\' style=\'display:none\'>');
	W('</div>');
}
</script>


<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<script type='text/javascript'>genStdRefresh(1,0,'ref.toggle()');</script>
</td></tr>
</table>
</form>
<script type='text/javascript'>earlyInit()</script>
</body>
</html>
