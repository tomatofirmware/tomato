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
//	<% nvram("ipv6_prefix,ipv6_prefix_length,ipv6_radvd,ipv6_nat64,tayga_prefix,tayga_prefixlen,tayga_server4,tayga_server6,tayga_client4,tayga_client6,ipv6_rtr_addr,ipv6_service,ipv6_dns,ipv6_tun_addr,ipv6_tun_addrlen,ipv6_ifname,ipv6_tun_v4end,ipv6_tun_mtu,ipv6_tun_ttl"); %>

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
		_ipv6_rtr_addr: 1,
		_f_ipv6_dns_1: 1,
		_f_ipv6_dns_2: 1,
		_f_ipv6_dns_3: 1,
		_f_ipv6_radvd: 1,
		_f_ipv6_nat64: 1,
		_ipv6_tun_v4end: 1,
		_ipv6_ifname: 1,
		_ipv6_tun_addr: 1,
		_ipv6_tun_addrlen: 1,
		_ipv6_tun_ttl: 1,
		_ipv6_tun_mtu: 1,
		_tayga_prefix: 1,
		_tayga_prefixlen: 1,
		_tayga_client4: 1,
		_tayga_client6: 1,
		_tayga_server4: 1,
		_tayga_server6: 1
	};

	switch(E('_ipv6_service').value) {
		case '':
			vis._ipv6_ifname = 0;
			vis._f_ipv6_dns_1 = 0;
			vis._f_ipv6_dns_2 = 0;
			vis._f_ipv6_dns_3 = 0;
			// fall through
		case 'other':
			vis._f_ipv6_radvd = 0;
			vis._ipv6_prefix = 0;
			vis._ipv6_prefix_length = 0;
			vis._ipv6_rtr_addr = 0;
			vis._ipv6_tun_v4end = 0;
			vis._ipv6_tun_addr = 0;
			vis._ipv6_tun_addrlen = 0;
			vis._ipv6_tun_ttl = 0;
			vis._ipv6_tun_mtu = 0;
			break;
		case 'native-pd':
			vis._ipv6_prefix = 0;
			vis._ipv6_rtr_addr = 0;
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

	if(!E('_f_ipv6_nat64').checked) {
		vis._tayga_prefix = 0;
		vis._tayga_prefixlen = 0;
		vis._tayga_client4 = 0;
		vis._tayga_client6 = 0;
		vis._tayga_server4 = 0;
		vis._tayga_server6 = 0;
	}
	
	for (a in vis) {
		b = E(a);
		c = vis[a];
		b.disabled = (c != 1);
		PR(b).style.display = c ? '' : 'none';
	}

	// --- verify ---

	// Length
	a = [['_ipv6_ifname', 2]];
	for (i = a.length - 1; i >= 0; --i) {
		v = a[i];
		if ((vis[v[0]]) && (!v_length(v[0], quiet || !ok, v[1]))) ok = 0;
	}

	// IP address
	a = ['_ipv6_tun_v4end', '_tayga_prefix', '_tayga_client4', '_tayga_server4'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]) && (!v_ip(a[i], quiet || !ok))) ok = 0;

	// range
	a = [['_ipv6_prefix_length', 3, 64], ['_ipv6_tun_addrlen', 3, 127], ['_ipv6_tun_ttl', 0, 255], ['_tayga_prefixlen', 3, 30]];
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
	a = ['_ipv6_prefix', '_ipv6_tun_addr', '_tayga_client6', '_tayga_server6'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]) && (!v_ipv6_addr(a[i], quiet || !ok))) ok = 0;

	// optional IPv6 address
	a = ['_ipv6_rtr_addr', '_f_ipv6_dns_1', '_f_ipv6_dns_2', '_f_ipv6_dns_3'];
	for (i = a.length - 1; i >= 0; --i)
		if ((vis[a[i]]) && (E(a[i]).value.length > 0) && (!v_ipv6_addr(a[i], quiet || !ok))) ok = 0;

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
	fom.ipv6_nat64.value = fom.f_ipv6_nat64.checked ? 1 : 0;

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
<input type='hidden' name='ipv6_nat64'>
<input type='hidden' name='ipv6_dns'>

<div class='section-title'>IPv6 Configuration</div>
<div class='section'>
<script type='text/javascript'>
dns = nvram.ipv6_dns.split(/\s+/);

createFieldTable('', [
	{ title: 'IPv6 Service Type', name: 'ipv6_service', type: 'select', 
		options: [['', 'Disabled'],['native','Native IPv6 from ISP'],['native-pd','DHCPv6 with Prefix Delegation'],['sit','6in4 Static Tunnel'],['other','Other (manually configured)']],
		value: nvram.ipv6_service },
	{ title: 'IPv6 Interface Name', name: 'ipv6_ifname', type: 'text', maxlen: 8, size: 10, value: nvram.ipv6_ifname },
	null,
	{ title: 'Assigned IPv6 Prefix', name: 'ipv6_prefix', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_prefix },
	{ title: 'Prefix Length', name: 'ipv6_prefix_length', type: 'text', maxlen: 3, size: 5, value: nvram.ipv6_prefix_length },
	{ title: 'Router IPv6 Address', name: 'ipv6_rtr_addr', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_rtr_addr },
	{ title: 'Static DNS', name: 'f_ipv6_dns_1', type: 'text', maxlen: 46, size: 48, value: dns[0] || '' },
	{ title: '',           name: 'f_ipv6_dns_2', type: 'text', maxlen: 46, size: 48, value: dns[1] || '' },
	{ title: '',           name: 'f_ipv6_dns_3', type: 'text', maxlen: 46, size: 48, value: dns[2] || '' },
	{ title: 'Enable Router Advertisements', name: 'f_ipv6_radvd', type: 'checkbox', value: nvram.ipv6_radvd != '0' },
	null,
	{ title: 'Tunnel Remote Endpoint (IPv4 Address)', name: 'ipv6_tun_v4end', type: 'text', maxlen: 15, size: 17, value: nvram.ipv6_tun_v4end },
	{ title: 'Tunnel Client IPv6 Address', multi: [
		{ name: 'ipv6_tun_addr', type: 'text', maxlen: 46, size: 48, value: nvram.ipv6_tun_addr, suffix: ' / ' },
		{ name: 'ipv6_tun_addrlen', type: 'text', maxlen: 3, size: 5, value: nvram.ipv6_tun_addrlen }
	] },
	{ title: 'Tunnel MTU', name: 'ipv6_tun_mtu', type: 'text', maxlen: 4, size: 8, value: nvram.ipv6_tun_mtu, suffix: ' <small>(0 for default)</small>' },
	{ title: 'Tunnel TTL', name: 'ipv6_tun_ttl', type: 'text', maxlen: 3, size: 8, value: nvram.ipv6_tun_ttl },
        null,
	{ title: 'Enable NAT64', name: 'f_ipv6_nat64', type: 'checkbox', value: nvram.ipv6_nat64 != '0' },
	{ title: 'NAT64 IPv4 Prefix', name: 'tayga_prefix', type: 'text', value: nvram.tayga_prefix },
	{ title: 'NAT64 IPv4 Prefix Length', name: 'tayga_prefixlen', type: 'text', value: nvram.tayga_prefixlen },
	{ title: 'NAT64 Client IPv4 Address', name: 'tayga_client4', type: 'text', value: nvram.tayga_client4 },
	{ title: 'NAT64 Client IPv6 Address', name: 'tayga_client6', type: 'text', value: nvram.tayga_client6 },
	{ title: 'NAT64 Server IPv4 Address', name: 'tayga_server4', type: 'text', value: nvram.tayga_server4 },
	{ title: 'NAT64 Server IPv6 Address', name: 'tayga_server6', type: 'text', value: nvram.tayga_server6 }
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
