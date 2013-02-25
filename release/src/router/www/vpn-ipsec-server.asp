<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato IPSec GUI
	Copyright (C) 2013 Daniel Borca
	Copyright (C) 2012 Augusto Bott
	http://code.google.com/p/tomato-sdhc-vlan/

	Tomato GUI
	Copyright (C) 2006-2007 Jonathan Zarate
	http://www.polarcloud.com/tomato/
	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] VPN: IPSec Server</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>
<style type='text/css'>
#ul-grid .co2 {
  text-align: center;
}
textarea {
  width: 98%;
  height: 10em;
}
</style>
<script type='text/javascript' src='interfaces.js'></script>
<script type='text/javascript'>
//	<% nvram("ipsec_enable,ipsec_usecert,ipsec_remoteip,ipsec_users,ipsec_dns1,ipsec_psk,ipsec_name,ipsec_ca,ipsec_crt,ipsec_key,l2tpd_enable");%>
//	<% ipsec_altnames(); %>

if (nvram.ipsec_remoteip == '') nvram.ipsec_remoteip = '10.6.0.2-7';

var ul = new TomatoGrid();
ul.setup = function() {
	this.init('ul-grid', 'sort', 6, [
		{ type: 'text', maxlen: 32, size: 32 },
		{ type: 'text', maxlen: 32, size: 32 } ]);

	this.headerSet(['Username', 'Password']);

	var r = nvram.ipsec_users.split('>');
	for (var i = 0; i < r.length; ++i) {
		var l = r[i].split('<');
		if (l.length == 2)
			ul.insertData(-1, l);
	}

	ul.recolor();
	ul.showNewEditor();
	ul.resetNewEditor();
	ul.sort(0);
}

ul.exist = function(f, v) {
	var data = this.getAllData();
	for (var i = 0; i < data.length; ++i) {
		if (data[i][f] == v) return true;
	}
	return false;
}

ul.existUser = function(user) {
	return this.exist(0, user);
}

function v_ipsec_secret(e, quiet) {
	var s;
	if ((e = E(e)) == null) return 0;
	s = e.value.trim().replace(/\s+/g, '');
	if (s.length < 1) {
		ferror.set(e, "Username and password can not be empty.", quiet);
		return 0;
	}
	if (s.length > 32) {
		ferror.set(e, "Invalid entry: max 32 characters are allowed.", quiet);
		return 0;
	}
	if (s.search(/^[.a-zA-Z0-9_\- ]+$/) == -1) {
		ferror.set(e, "Invalid entry. Only characters \"A-Z 0-9 . - _\" are allowed.", quiet);
		return 0;
	}
	e.value = s;
	ferror.clear(e);
	return 1;
}

ul.verifyFields = function(row, quiet) {
	var f, s;
	f = fields.getAll(row);

	if (!v_ipsec_secret(f[0], quiet)) return 0;

	if (this.existUser(f[0].value)) {
		ferror.set(f[0], 'Duplicate User', quiet);
		return 0;
	}

	if (!v_ipsec_secret(f[1], quiet)) return 0;

	return 1;
}

ul.dataToView = function(data) {
	return [data[0], '<center><small><i>Secret</i></small></center>'];
}

function save() {
	if (ul.isEditing()) return;

	if ((E('_f_ipsec_enable').checked) && (!verifyFields(null, 0))) return;

	if (E('_f_ipsec_enable').checked) {
		var nouser = (ul.getDataCount() < 1);
		if (nouser || nvram.l2tpd_enable == '1') {
			var e = E('footer-msg');
			e.innerHTML = nouser ? 'Cannot proceed: at least one user must be defined.' : 'L2TP daemon enabled.';
			e.style.visibility = 'visible';
			setTimeout(
				function() {
					e.innerHTML = '';
					e.style.visibility = 'hidden';
				}, 5000);
			return;
		}
	}

	ul.resetNewEditor();

	var fom = E('_fom');
	var uldata = ul.getAllData();

	var s = '';
	for (var i = 0; i < uldata.length; ++i) {
		s += uldata[i].join('<') + '>';
	}
	fom.ipsec_users.value = s;

	fom.ipsec_enable.value = E('_f_ipsec_enable').checked ? 1 : 0;
	fom.ipsec_usecert.value = E('_f_ipsec_usecert').checked ? 1 : 0;

	var a = E('_f_ipsec_startip').value;
	var b = E('_f_ipsec_endip').value;
	if ((fixIP(a) != null) && (fixIP(b) != null)) {
		var c = b.split('.').splice(3, 1);
		fom.ipsec_remoteip.value = a + '-' + c;
	}

	if (fom.ipsec_dns1.value == '0.0.0.0') fom.ipsec_dns1.value = '';

	form.submit(fom, 1);
}

function submit_complete() {
/* REMOVE-BEGIN */
//	reloadPage();
/* REMOVE-END */
	verifyFields(null, 1);
}

function verifyFields(focused, quiet) {
	var c = !E('_f_ipsec_enable').checked;
	var u = !E('_f_ipsec_usecert').checked;
	E('_f_ipsec_usecert').disabled = c;
	E('_f_ipsec_startip').disabled = c;
	E('_f_ipsec_endip').disabled = c;
	E('_ipsec_dns1').disabled = c;
	E('_ipsec_psk').disabled = c || !u;
	E('_f_ipsec_psk_random').disabled = c || !u;
	E('_ipsec_name').disabled = c || u;
	E('_ipsec_ca').disabled = c || u;
	E('_ipsec_crt').disabled = c || u;
	E('_ipsec_key').disabled = c || u;

	elem.display(PR('_ipsec_psk'), u);
	elem.display(PR('_ipsec_name'), PR('_ipsec_ca'), PR('_ipsec_crt'), PR('_ipsec_key'), !u);

	if (u) {
		var r = E('_ipsec_psk');
		if (r.value.length < 8) {
			ferror.set(r, 'Invalid pre-shared key', quiet);
			return 0;
		} else {
			ferror.clear(r);
		}
	} else {
		var r1 = E('_ipsec_ca');
		var r2 = E('_ipsec_crt');
		var r3 = E('_ipsec_key');
		if (!r1.value.length || !r2.value.length || !r3.value.length) {
			if (!r1.value.length) ferror.set(r1, 'Certificate Authority', quiet);
			if (!r2.value.length) ferror.set(r2, 'Server Certificate', quiet);
			if (!r3.value.length) ferror.set(r3, 'Server Key', quiet);
			return 0;
		} else {
			ferror.clear(r1);
			ferror.clear(r2);
			ferror.clear(r3);
		}
	}

/* REMOVE-BEGIN */
// XXX TODO ipsec_name must be in ipsec_altnames
// we need a way to refresh altnames
// form.submitHidden('/ipsec.cgi', { refresh_altnames: nvram.ipsec_crt });
// xob.post('/ipsec.cgi', 'refresh_altnames=' + nvram.ipsec_crt);
/* REMOVE-END */

	var a = E('_f_ipsec_startip');
/* REMOVE-BEGIN */
/*
	if ((a.value == '') || (a.value == '0.0.0.0')) {
		var l;
		var m = aton(nvram.lan_ipaddr) & aton(nvram.lan_netmask);
		var o = (m) ^ (~ aton(nvram.lan_netmask))
		var n = o - m;
		do {
			if (--n < 0) {
				a.value = '';
				return;
			}
			m++;
		} while (((l = fixIP(ntoa(m), 1)) == null) || (l == nvram.lan_ipaddr) );
		a.value = l;
	}
*/
/* REMOVE-END */
	var b = E('_f_ipsec_endip');
/* REMOVE-BEGIN */
/*
	if ((b.value == '') || (b.value == '0.0.0.0')) {
		var l;
		var m = aton(nvram.lan_ipaddr) & aton(nvram.lan_netmask);
		var o = (m) ^ (~ aton(nvram.lan_netmask));
		var n = o - m;
		do {
			if (--n < 0) {
				b.value = '';
				return;
			}
			o--;
		} while (((l = fixIP(ntoa(o), 1)) == null) || (l == nvram.lan_ipaddr) || (Math.abs((aton(a.value) - (aton(l)))) > 5) );
		b.value = l;
	}

	var net = getNetworkAddress(nvram.lan_ipaddr, nvram.lan_netmask);
	var brd = getBroadcastAddress(net, nvram.lan_netmask);

	if ((aton(a.value) >= aton(brd)) || (aton(a.value) <= aton(net))) {
		ferror.set(a, 'Invalid starting IP address (outside valid range).', quiet);
		return 0;
	} else {
		ferror.clear(a);
	}

	if ((aton(b.value) >= aton(brd)) || (aton(b.value) <= aton(net))) {
		ferror.set(b, 'Invalid final IP address (outside valid range)', quiet);
		return 0;
	} else {
		ferror.clear(b);
	}
*/
/* REMOVE-END */
	if (Math.abs((aton(a.value) - (aton(b.value)))) > 5) {
		ferror.set(a, 'Invalid range (max 6 IPs)', quiet);
		ferror.set(b, 'Invalid range (max 6 IPs)', quiet);
		elem.setInnerHTML('ipsec_count', '(?)');
		return 0;
	} else {
		ferror.clear(a);
		ferror.clear(b);
	}

	if (aton(a.value) > aton(b.value)) {
		var d = a.value;
		a.value = b.value;
		b.value = d;
	}

	elem.setInnerHTML('ipsec_count', '(' + ((aton(b.value) - aton(a.value)) + 1) + ')');
/* REMOVE-BEGIN */
// AB TODO - move to ul.onOk, onAdd,onDelete?
//	elem.setInnerHTML('user_count', '(total ' + (ul.getDataCount()) + ')');
/* REMOVE-END */
	if (!v_ipz('_ipsec_dns1', quiet)) return 0;

	if (!v_ip('_f_ipsec_startip', quiet)) return 0;
	if (!v_ip('_f_ipsec_endip', quiet)) return 0;

	return 1;
}

function init() {
	var c;
	if (((c = cookie.get('vpn_ipsec_notes_vis')) != null) && (c == '1')) toggleVisibility("notes");

	if (nvram.ipsec_remoteip.indexOf('-') != -1) {
		var tmp = nvram.ipsec_remoteip.split('-');
		E('_f_ipsec_startip').value = tmp[0];
		E('_f_ipsec_endip').value = tmp[0].split('.').splice(0,3).join('.') + '.' + tmp[1];
	}

	ul.setup();

	verifyFields(null, 1);
}

function toggleVisibility(whichone) {
	if (E('sesdiv_' + whichone).style.display == '') {
		E('sesdiv_' + whichone).style.display = 'none';
		E('sesdiv_' + whichone + '_showhide').innerHTML = '(Click here to show)';
		cookie.set('vpn_ipsec_' + whichone + '_vis', 0);
	} else {
		E('sesdiv_' + whichone).style.display='';
		E('sesdiv_' + whichone + '_showhide').innerHTML = '(Click here to hide)';
		cookie.set('vpn_ipsec_' + whichone + '_vis', 1);
	}
}

function random_x(max)
{
	var c = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
	var s = '';
	while (max-- > 0) s += c.substr(Math.floor(c.length * Math.random()), 1);
	return s;
}

function random_psk(id)
{
	var e = E(id);
	e.value = random_x(64);
	verifyFields(null, 1);
}

</script>
</head>
<body onload='init()'>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
  <div class='title'>Tomato</div>
  <div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<input type='hidden' name='_nextpage' value='vpn-ipsec.asp'>
<input type='hidden' name='_nextwait' value='5'>
<input type='hidden' name='_service' value='firewall-restart,ipsec-restart,dnsmasq-restart'>
<input type='hidden' name='ipsec_users'>
<input type='hidden' name='ipsec_enable'>
<input type='hidden' name='ipsec_usecert'>
<input type='hidden' name='ipsec_remoteip'>

<div class='section-title'>IPSec Server Configuration</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable', name: 'f_ipsec_enable', type: 'checkbox', value: nvram.ipsec_enable == '1' },
	{ title: 'Use certs', name: 'f_ipsec_usecert', type: 'checkbox', value: nvram.ipsec_usecert == '1' },
	{ title: 'Remote IP Address Range', multi: [
		{ name: 'f_ipsec_startip', type: 'text', maxlen: 15, size: 17, value: nvram.dhcpd_startip, suffix: '&nbsp;-&nbsp;' },
		{ name: 'f_ipsec_endip', type: 'text', maxlen: 15, size: 17, value: nvram.dhcpd_endip, suffix: ' <i id="ipsec_count"></i>' }
	] },
	{ title: 'DNS Server', name: 'ipsec_dns1', type: 'text', maxlen: 15, size: 17, value: nvram.ipsec_dns1 },
	{ title: 'Group PSK (nobody)', name: 'ipsec_psk', type: 'password', maxlen: 64, size: 60, peekaboo: 1, value: eval('nvram.ipsec_psk'),
		suffix: ' <input type="button" id="_f_ipsec_psk_random" value="Random" onclick="random_psk(\'_ipsec_psk\')">' },
	{ title: 'Server name', name: 'ipsec_name', type: 'select', options: ipsec_altnames.length ? ipsec_altnames : [['invalid']], value: eval('nvram.ipsec_name') },
	{ title: 'Certificate Authority', name: 'ipsec_ca', type: 'textarea', value: nvram.ipsec_ca },
	{ title: 'Server certificate', name: 'ipsec_crt', type: 'textarea', value: nvram.ipsec_crt },
	{ title: 'Server key', name: 'ipsec_key', type: 'textarea', value: nvram.ipsec_key }
]);
</script>
</div>

<div class='section-title'>IPSec User List</div>
<div class='section'>
  <table class='tomato-grid' cellspacing=1 id='ul-grid'></table>
</div>

<div class='section-title'>Notes <small><i><a href='javascript:toggleVisibility("notes");'><span id='sesdiv_notes_showhide'>(Click here to show)</span></a></i></small></div>
<div class='section' id='sesdiv_notes' style='display:none'>
<ul>
<li><b>Remote IP Address Range</b> - Remote IP addresses to be used on the tunnelled IPSec links (max 6).</li>
<li><b>DNS Server</b> - Allows defining DNS server manually (if none are set, the router/local IP address should be used by VPN clients).</li>
<li><b>Group PSK (nobody)</b> - Pre-shared Key used by the IPSec daemon to authenticate group <b>nobody</b>.</li>
<li><b>Server Name</b> - This server FQDN.  Must be included in certificate <b>subjectAltName</b>.</li>
<li><b>Server certificate</b> - Certificate must contain this server name in <b>subjectAltName</b>.</li>
<li><b>IPSec users</b> - Non-shell system users belonging to group <b>nobody</b>.</li>
</ul>

<small>
<ul>
<li><b>Other relevant notes/hints:</b>
<ul>
<li>Try to avoid any conflicts and/or overlaps between the address ranges configured/available for DHCP and VPN clients on your local networks.</li>
</ul>
</small>
</div>

<br>
<div style="float:right;text-align:right">
&raquo; <a href="vpn-ipsec-online.asp">IPSec Online</a>
</div>

</td></tr>
<tr><td id='footer' colspan=2>
 <span id='footer-msg'></span>
 <input type='button' value='Save' id='save-button' onclick='save()'>
 <input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
</body>
</html>
