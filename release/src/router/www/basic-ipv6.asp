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
<title>[<% ident(); %>] Basic: IPv6</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>
//	<% nvram("ipv6_prefix,ipv6_prefix_length,ipv6_radvd,ipv6_accept_ra,ipv6_rtr_addr,ipv6_service,ipv6_dns,ipv6_tun_addr,ipv6_tun_addrlen,ipv6_ifname,ipv6_tun_v4end,ipv6_tun_mtu,ipv6_tun_ttl"); %>

nvram.ipv6_accept_ra = fixInt(nvram.ipv6_accept_ra, 0, 3, 0);

function verifyFields(focused, quiet)
{
	var i;
	var ok = 1;
	var a, b, c;

	// --- visibility ---

	var vis = {
		_ipv6_service: 1,
		_ipv6_prefix: 1,
		_ipv6_prefix_length: 1,
		_f_ipv6_rtr_addr_auto: 1,
		_f_ipv6_rtr_addr: 1,
		_f_ipv6_dns_1: 1,
		_f_ipv6_dns_2: 1,
		_f_ipv6_dns_3: 1,
		_f_ipv6_radvd: 1,
		_f_ipv6_accept_ra_wan: 1,
		_f_ipv6_accept_ra_lan: 1,
		_ipv6_tun_v4end: 1,
		_ipv6_ifname: 1,
		_ipv6_tun_addr: 1,
		_ipv6_tun_addrlen: 1,
		_ipv6_tun_ttl: 1,
		_ipv6_tun_mtu: 1
	};

	switch(E('_ipv6_service').value) {
		case '':
			vis._ipv6_ifname = 0;
			vis._f_ipv6_rtr_addr_auto = 0;
			vis._f_ipv6_rtr_addr = 0;
			vis._f_ipv6_dns_1 = 0;
			vis._f_ipv6_dns_2 = 0;
			vis._f_ipv6_dns_3 = 0;
			vis._f_ipv6_radvd = 0;
			vis._f_ipv6_accept_ra_wan = 0;
			vis._f_ipv6_accept_ra_lan = 0;
			// fall through
		case 'other':
			vis._ipv6_prefix = 0;
			vis._ipv6_prefix_length = 0;
			E('_f_ipv6_rtr_addr_auto').value = 1;
			vis._ipv6_tun_v4end = 0;
			vis._ipv6_tun_addr = 0;
			vis._ipv6_tun_addrlen = 0;
			vis._ipv6_tun_ttl = 0;
			vis._ipv6_tun_mtu = 0;
			break;
		case 'native-pd':
			vis._ipv6_prefix = 0;
			vis._f_ipv6_rtr_addr_auto = 0;
			vis._f_ipv6_rtr_addr = 0;
			// fall through
		case 'native':
			vis._ipv6_ifname = 0;
			vis._ipv6_tun_v4end = 0;
			vis._ipv6_tun_addr = 0;
			vis._ipv6_tun_addrlen = 0;
			vis._ipv6_tun_ttl = 0;
			vis._ipv6_tun_mtu = 0;
			break;
		case 'sit':
			break;
	}
	
	if (vis._f_ipv6_rtr_addr_auto && E('_f_ipv6_rtr_addr_auto').value == 0) {
		vis._f_ipv6_rtr_addr = 2;
	}
	
	for (a in vis) {
		b = E(a);
		c = vis[a];
		b.disabled = (c != 1);
		PR(b).style.display = c ? '' : 'none';
	}

	// --- verify ---

	if (vis._ipv6_ifname == 1) {
		if (E('_ipv6_service').value != 'other') {
			if (!v_length('_ipv6_ifname', quiet || !ok, 2)) ok = 0;
		}
		else ferror.clear('_ipv6_ifname');
	}

/* REMOVE-BEGIN
	// Length
	a = [['_ipv6_ifname', 2]];
	for (i = a.length - 1; i >= 0; --i) {
		v = a[i];
		if ((vis[v[0]]) && (!v_length(v[0], quiet || !ok, v[1]))) ok = 0;
	}
REMOVE-END */

	// IP address
	a = ['_ipv6_tun_v4end'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]) && (!v_ip(a[i], quiet || !ok))) ok = 0;

	// range
	a = [['_ipv6_prefix_length', 3, 64], ['_ipv6_tun_addrlen', 3, 127], ['_ipv6_tun_ttl', 0, 255]];
	for (i = a.length - 1; i >= 0; --i) {
		b = a[i];
		if ((vis[b[0]]) && (!v_range(b[0], quiet || !ok, b[1], b[2]))) ok = 0;
	}

	// mtu
	b = '_ipv6_tun_mtu';
	if (vis[b]) {
		if ((!v_range(b, 1, 0, 0)) && (!v_range(b, quiet || !ok, 1280, 1480))) ok = 0;
		else ferror.clear(E(b));
	}

	// IPv6 address
	a = ['_ipv6_prefix', '_ipv6_tun_addr'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]) && (!v_ipv6_addr(a[i], quiet || !ok))) ok = 0;
			
	if (vis._f_ipv6_rtr_addr == 2 && ok) {
		b = E('_ipv6_prefix');
		ip = ZeroIPv6PrefixBits(b.value, E('_ipv6_prefix_length').value);
		b.value = ip;
		E('_f_ipv6_rtr_addr').value = ip + '1';
	}

	// optional IPv6 address
	a = ['_f_ipv6_rtr_addr', '_f_ipv6_dns_1', '_f_ipv6_dns_2', '_f_ipv6_dns_3'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]==1) && (E(a[i]).value.length > 0) && (!v_ipv6_addr(a[i], quiet || !ok))) ok = 0;

	return ok;
}

function earlyInit()
{
	verifyFields(null, 1);
}

function joinIPv6Addr(a) {
	var r, i, s;

	r = [];
	for (i = 0; i < a.length; ++i) {
		s = CompressIPv6Address(a[i]);
		if ((s) && (s != '')) r.push(s);
	}
	return r.join(' ');
}

function save()
{
	var a, b, c;
	var i;

	if (!verifyFields(null, false)) return;

	var fom = E('_fom');

	fom.ipv6_dns.value = joinIPv6Addr([fom.f_ipv6_dns_1.value, fom.f_ipv6_dns_2.value, fom.f_ipv6_dns_3.value]);
	fom.ipv6_radvd.value = fom.f_ipv6_radvd.checked ? 1 : 0;
	
	fom.ipv6_accept_ra.value = 0;
	if (fom.f_ipv6_accept_ra_wan.checked) fom.ipv6_accept_ra.value |= 1;
	if (fom.f_ipv6_accept_ra_lan.checked) fom.ipv6_accept_ra.value |= 2;

	switch(E('_ipv6_service').value) {
		case 'other':
			fom.ipv6_prefix_length.value = 64;
			fom.ipv6_prefix.value = '';
			fom.ipv6_rtr_addr.value = fom.f_ipv6_rtr_addr.value;
			break;
		case 'native-pd':
			fom.ipv6_prefix.value = '';
			fom.ipv6_rtr_addr.value = '';
			break;
		default:
			fom.ipv6_rtr_addr.disabled = fom.f_ipv6_rtr_addr_auto.disabled;
			if (fom.f_ipv6_rtr_addr_auto.value == 1)
				fom.ipv6_rtr_addr.value = fom.f_ipv6_rtr_addr.value;
			else
				fom.ipv6_rtr_addr.value = '';
	}
	

	form.submit(fom, 1);
}

</script>

</head>
<body>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='basic-ipv6.asp'>
<input type='hidden' name='_nextwait' value='10'>
<input type='hidden' name='_service' value='*'>

<input type='hidden' name='ipv6_radvd'>
<input type='hidden' name='ipv6_dns'>
<input type='hidden' name='ipv6_rtr_addr'>
<input type='hidden' name='ipv6_accept_ra'>

<div class='section-title'>IPv6 Configuration</div>
<div class='section'>
<script type='text/javascript'>
dns = nvram.ipv6_dns.split(/\s+/);

createFieldTable('', [
	{ title: 'IPv6 Service Type', name: 'ipv6_service', type: 'select', 
		options: [['', 'Disabled'],['native','Native IPv6 from ISP'],['native-pd','DHCPv6 with Prefix Delegation'],['sit','6in4 Static Tunnel'],['other','Other (Manual Configuration)']],
		value: nvram.ipv6_service },
	{ title: 'IPv6 WAN Interface', name: 'ipv6_ifname', type: 'text', maxlen: 8, size: 10, value: nvram.ipv6_ifname },
	null,
	{ title: 'Assigned IPv6 Prefix', name: 'ipv6_prefix', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_prefix },
	{ title: 'Prefix Length', name: 'ipv6_prefix_length', type: 'text', maxlen: 3, size: 5, value: nvram.ipv6_prefix_length },
	{ title: 'Router IPv6 Address', multi: [
		{ name: 'f_ipv6_rtr_addr_auto', type: 'select', options: [['0', 'Default'],['1','Manual']], value: (nvram.ipv6_rtr_addr != '') },
		{ name: 'f_ipv6_rtr_addr', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_rtr_addr }
	] },
	{ title: 'Static DNS', name: 'f_ipv6_dns_1', type: 'text', maxlen: 46, size: 48, value: dns[0] || '' },
	{ title: '',           name: 'f_ipv6_dns_2', type: 'text', maxlen: 46, size: 48, value: dns[1] || '' },
	{ title: '',           name: 'f_ipv6_dns_3', type: 'text', maxlen: 46, size: 48, value: dns[2] || '' },
	{ title: 'Enable Router Advertisements', name: 'f_ipv6_radvd', type: 'checkbox', value: nvram.ipv6_radvd != '0' },
	{ title: 'Accept RA from', multi: [
		{ suffix: '&nbsp; WAN &nbsp;&nbsp;&nbsp;', name: 'f_ipv6_accept_ra_wan', type: 'checkbox', value: (nvram.ipv6_accept_ra & 1) },
		{ suffix: '&nbsp; LAN &nbsp;',	name: 'f_ipv6_accept_ra_lan', type: 'checkbox', value: (nvram.ipv6_accept_ra & 2) }
	] },
	null,
	{ title: 'Tunnel Remote Endpoint (IPv4 Address)', name: 'ipv6_tun_v4end', type: 'text', maxlen: 15, size: 17, value: nvram.ipv6_tun_v4end },
	{ title: 'Tunnel Client IPv6 Address', multi: [
		{ name: 'ipv6_tun_addr', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_tun_addr, suffix: ' / ' },
		{ name: 'ipv6_tun_addrlen', type: 'text', maxlen: 3, size: 5, value: nvram.ipv6_tun_addrlen }
	] },
	{ title: 'Tunnel MTU', name: 'ipv6_tun_mtu', type: 'text', maxlen: 4, size: 8, value: nvram.ipv6_tun_mtu, suffix: ' <small>(0 for default)</small>' },
	{ title: 'Tunnel TTL', name: 'ipv6_tun_ttl', type: 'text', maxlen: 3, size: 8, value: nvram.ipv6_tun_ttl }
]);
</script>
</div>

<br>
<script type='text/javascript'>show_notice1('<% notice("ip6tables"); %>');</script>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>earlyInit()</script>
<div style='height:100px'></div>
</body>
</html>
