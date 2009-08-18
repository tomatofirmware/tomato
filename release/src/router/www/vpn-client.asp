<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Portions Copyright (C) 2008-2009 Keith Moyer, tomatovpn@keithmoyer.com

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] VPN: Client</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript' src='vpn.js'></script>
<script type='text/javascript'>

//	<% nvram("vpn_client_eas,vpn_client1_if,vpn_client1_bridge,vpn_client1_nat,vpn_client1_proto,vpn_client1_addr,vpn_client1_port,vpn_client1_retry,vpn_client1_firewall,vpn_client1_crypt,vpn_client1_comp,vpn_client1_cipher,vpn_client1_local,vpn_client1_remote,vpn_client1_nm,vpn_client1_reneg,vpn_client1_hmac,vpn_client1_adns,vpn_client1_rgw,vpn_client1_gw,vpn_client1_custom,vpn_client1_static,vpn_client1_ca,vpn_client1_crt,vpn_client1_key,vpn_client2_if,vpn_client2_bridge,vpn_client2_nat,vpn_client2_proto,vpn_client2_addr,vpn_client2_port,vpn_client2_retry,vpn_client2_firewall,vpn_client2_crypt,vpn_client2_comp,vpn_client2_cipher,vpn_client2_local,vpn_client2_remote,vpn_client2_nm,vpn_client2_reneg,vpn_client2_hmac,vpn_client2_adns,vpn_client2_rgw,vpn_client2_gw,vpn_client2_custom,vpn_client2_static,vpn_client2_ca,vpn_client2_crt,vpn_client2_key"); %>

tabs = [['client1', 'Client 1'],['client2', 'Client 2']];
sections = [['basic', 'Basic'],['advanced', 'Advanced'],['keys','Keys'],['status','Status']];
statusUpdaters = [];
for (i = 0; i < tabs.length; ++i) statusUpdaters.push(new StatusUpdater());
ciphers = [['default','Use Default'],['none','None']];
for (i = 0; i < vpnciphers.length; ++i) ciphers.push([vpnciphers[i],vpnciphers[i]]);

changed = 0;
vpn1up = parseInt('<% psup("vpnclient1"); %>');
vpn2up = parseInt('<% psup("vpnclient2"); %>');

function updateStatus(num)
{
	var xob = new XmlHttp();
	xob.onCompleted = function(text, xml)
	{
		statusUpdaters[num].update(text);
		xob = null;
	}
	xob.onError = function(ex)
	{
		statusUpdaters[num].errors.innerHTML += 'ERROR! '+ex+'<br>';
		xob = null;
	}

	xob.post('/vpnstatus.cgi', 'client=' + (num+1));
}

function tabSelect(name)
{
	tgHideIcons();

	tabHigh(name);

	for (var i = 0; i < tabs.length; ++i)
	{
		var on = (name == tabs[i][0]);
		elem.display(tabs[i][0] + '-tab', on);
	}

	cookie.set('vpn_client_tab', name);
}

function sectSelect(tab, section)
{
	tgHideIcons();

	for (var i = 0; i < sections.length; ++i)
	{
		if (section == sections[i][0])
		{
			elem.addClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], true);
		}
		else
		{
			elem.removeClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], false);
		}
	}

	cookie.set('vpn_client'+tab+'_section', section);
}

function toggle(service, isup)
{
	if (changed && !confirm("Unsaved changes will be lost. Continue anyway?")) return;

	E('_' + service + '_button').disabled = true;
	form.submitHidden('service.cgi', {
		_redirect: 'vpn-client.asp',
		_sleep: '3',
		_service: service + (isup ? '-stop' : '-start')
	});
}

function verifyFields(focused, quiet)
{
	tgHideIcons();

	var ret = 1;

	// When settings change, make sure we restart the right client
	if (focused)
	{
		changed = 1;

		var clientindex = focused.name.indexOf("client");
		if (clientindex >= 0)
		{
			var clientnumber = focused.name.substring(clientindex+6,clientindex+7);
			var stripped = focused.name.substring(0,clientindex+6)+focused.name.substring(clientindex+7);

			if (stripped == 'vpn_client_local')
				E('_f_vpn_client'+clientnumber+'_local').value = focused.value;
			else if (stripped == 'f_vpn_client_local')
				E('_vpn_client'+clientnumber+'_local').value = focused.value;

			var fom = E('_fom');
			if (eval('vpn'+clientnumber+'up') && fom._service.value.indexOf('client'+clientnumber) < 0)
			{
				if ( fom._service.value != "" ) fom._service.value += ",";
				fom._service.value += 'vpnclient'+clientnumber+'-restart';
			}
		}
	}

	// Element varification
	for (i = 0; i < tabs.length; ++i)
	{
		t = tabs[i][0];

		if (!v_ip('_vpn_'+t+'_addr', true) && !v_domain('_vpn_'+t+'_addr', true)) { ferror.set(E('_vpn_'+t+'_addr'), "Invalid server address.", quiet); ret = 0; }
		if (!v_port('_vpn_'+t+'_port', quiet)) ret = 0;
		if (!v_ip('_vpn_'+t+'_local', quiet, 1)) ret = 0;
		if (!v_ip('_f_vpn_'+t+'_local', true, 1)) ret = 0;
		if (!v_ip('_vpn_'+t+'_remote', quiet, 1)) ret = 0;
		if (!v_netmask('_vpn_'+t+'_nm', quiet)) ret = 0;
		if (!v_range('_vpn_'+t+'_retry', quiet, -1, 32767)) ret = 0;
		if (!v_range('_vpn_'+t+'_reneg', quiet, -1, 2147483647)) ret = 0;
		if (E('_vpn_'+t+'_gw').value.length > 0 && !v_ip('_vpn_'+t+'_gw', quiet, 1)) ret = 0;
	}

	// Visability changes
	for (i = 0; i < tabs.length; ++i)
	{
		t = tabs[i][0];

		fw = E('_vpn_'+t+'_firewall');
		auth = E('_vpn_'+t+'_crypt');
		iface = E('_vpn_'+t+'_if');
		bridge = E('_f_vpn_'+t+'_bridge');
		nat = E('_f_vpn_'+t+'_nat');
		hmac = E('_vpn_'+t+'_hmac');
		rgw = E('_f_vpn_'+t+'_rgw');

		elem.display(PR('_vpn_'+t+'_ca'), PR('_vpn_'+t+'_crt'), PR('_vpn_'+t+'_key'), PR('_vpn_'+t+'_hmac'),
		             PR('_vpn_'+t+'_adns'), PR('_vpn_'+t+'_reneg'), auth.value == "tls");
		elem.display(PR('_vpn_'+t+'_static'), auth.value == "secret" || (auth.value == "tls" && hmac.value >= 0));
		elem.display(E(t+'_custom_crypto_text'), auth.value == "custom");
		elem.display(PR('_f_vpn_'+t+'_bridge'), iface.value == "tap");
		elem.display(E(t+'_bridge_warn_text'), !bridge.checked);
		elem.display(PR('_f_vpn_'+t+'_nat'), fw.value != "custom" && (iface.value == "tun" || !bridge.checked));
		elem.display(E(t+'_nat_warn_text'), fw.value != "custom" && (!nat.checked || (auth.value == "secret" && iface.value == "tun")));
		elem.display(PR('_vpn_'+t+'_local'), auth.value == "secret" && iface.value == "tun");
		elem.display(PR('_f_vpn_'+t+'_local'), auth.value == "secret" && (iface.value == "tap" && !bridge.checked));
		elem.display(E(t+'_gateway'), iface.value == "tap" && E('_f_vpn_'+t+'_rgw').checked);

		keyHelp = E(t+'-keyhelp');
		switch (auth.value)
		{
		case "tls":
			keyHelp.href = helpURL['TLSKeys'];
			break;
		case "secret":
			keyHelp.href = helpURL['staticKeys'];
			break;
		default:
			keyHelp.href = helpURL['howto'];
			break;
		}
	}

	return ret;
}

function save()
{
	if (!verifyFields(null, false)) return;

	var fom = E('_fom');

	E('vpn_client_eas').value = '';

	for (i = 0; i < tabs.length; ++i)
	{
		t = tabs[i][0];

		if ( E('_f_vpn_'+t+'_eas').checked )
			E('vpn_client_eas').value += ''+(i+1)+',';

		E('vpn_'+t+'_bridge').value = E('_f_vpn_'+t+'_bridge').checked ? 1 : 0;
		E('vpn_'+t+'_nat').value = E('_f_vpn_'+t+'_nat').checked ? 1 : 0;
		E('vpn_'+t+'_rgw').value = E('_f_vpn_'+t+'_rgw').checked ? 1 : 0;
	}

	form.submit(fom, 1);

	changed = 0;
}

function init()
 {
	tabSelect(cookie.get('vpn_client_tab') || tabs[0][0]);

 	for (i = 0; i < tabs.length; ++i)
	{
		sectSelect(i, cookie.get('vpn_client'+i+'_section') || sections[i][0]);

		t = tabs[i][0];

		statusUpdaters[i].init(null,null,t+'-status-stats-table',t+'-status-time',t+'-status-content',t+'-no-status',t+'-status-errors');
		updateStatus(i);
	}

	verifyFields(null, true);
}
</script>

<style type='text/css'>
textarea {
	width: 98%;
	height: 10em;
}
p.keyhelp
{
	font-size: smaller;
	font-style: italic;
}
div.status-header p
{
	font-weight: bold;
	padding-bottom: 4px;
}
table.status-table
{
	width: auto;
	margin-left: auto;
	margin-right: auto;
	text-align: center;
}
</style>

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

<input type='hidden' name='_nextpage' value='vpn-client.asp'>
<input type='hidden' name='_nextwait' value='5'>
<input type='hidden' name='_service' value=''>
<input type='hidden' name='vpn_client_eas' id='vpn_client_eas' value=''>

<div class='section-title'>VPN Client Configuration</div>
<div class='section'>
<script type='text/javascript'>
tabCreate.apply(this, tabs);

for (i = 0; i < tabs.length; ++i)
{
	t = tabs[i][0];
	W('<div id=\''+t+'-tab\'>');
	W('<input type=\'hidden\' id=\'vpn_'+t+'_bridge\' name=\'vpn_'+t+'_bridge\'>');
	W('<input type=\'hidden\' id=\'vpn_'+t+'_nat\' name=\'vpn_'+t+'_nat\'>');
	W('<input type=\'hidden\' id=\'vpn_'+t+'_rgw\' name=\'vpn_'+t+'_rgw\'>');

	W('<ul class="tabs">');
	for (j = 0; j < sections.length; j++)
	{
		W('<li><a href="javascript:sectSelect('+i+', \''+sections[j][0]+'\')" id="'+t+'-'+sections[j][0]+'-tab">'+sections[j][1]+'</a></li>');
	}
	W('</ul><div class=\'tabs-bottom\'></div>');

	W('<div id=\''+t+'-basic\'>');
	createFieldTable('', [
		{ title: 'Start with Router', name: 'f_vpn_'+t+'_eas', type: 'checkbox', value: nvram.vpn_client_eas.indexOf(''+(i+1)) >= 0 },
		{ title: 'Interface Type', name: 'vpn_'+t+'_if', type: 'select', options: [ ['tap','TAP'], ['tun','TUN'] ], value: eval( 'nvram.vpn_'+t+'_if' ) },
		{ title: 'Protocol', name: 'vpn_'+t+'_proto', type: 'select', options: [ ['udp','UDP'], ['tcp-client','TCP'] ], value: eval( 'nvram.vpn_'+t+'_proto' ) },
		{ title: 'Server Address/Port', multi: [
			{ name: 'vpn_'+t+'_addr', type: 'text', size: 17, value: eval( 'nvram.vpn_'+t+'_addr' ) },
			{ name: 'vpn_'+t+'_port', type: 'text', maxlen: 5, size: 7, value: eval( 'nvram.vpn_'+t+'_port' ) } ] },
		{ title: 'Firewall', name: 'vpn_'+t+'_firewall', type: 'select', options: [ ['auto', 'Automatic'], ['custom', 'Custom'] ], value: eval( 'nvram.vpn_'+t+'_firewall' ) },
		{ title: 'Authorization Mode', name: 'vpn_'+t+'_crypt', type: 'select', options: [ ['tls', 'TLS'], ['secret', 'Static Key'], ['custom', 'Custom'] ], value: eval( 'nvram.vpn_'+t+'_crypt' ),
			suffix: '<span id=\''+t+'_custom_crypto_text\'>&nbsp;<small>(must configure manually...)</small></span>' },
		{ title: 'Extra HMAC authorization (tls-auth)', name: 'vpn_'+t+'_hmac', type: 'select', options: [ [-1, 'Disabled'], [2, 'Bi-directional'], [0, 'Incoming (0)'], [1, 'Outgoing (1)'] ], value: eval( 'nvram.vpn_'+t+'_hmac' ) },
		{ title: 'Server is on the same subnet', name: 'f_vpn_'+t+'_bridge', type: 'checkbox', value: eval( 'nvram.vpn_'+t+'_bridge' ) != 0,
			suffix: '<span style="color: red" id=\''+t+'_bridge_warn_text\'>&nbsp<small>Warning: Cannot bridge distinct subnets. Defaulting to routed mode.<small></span>' },
		{ title: 'Create NAT on tunnel', name: 'f_vpn_'+t+'_nat', type: 'checkbox', value: eval( 'nvram.vpn_'+t+'_nat' ) != 0,
			suffix: '<span style="font-style: italic" id=\''+t+'_nat_warn_text\'>&nbsp<small>Routes must be configured manually.<small></span>' },
		{ title: 'Local/remote endpoint addresses', multi: [
			{ name: 'vpn_'+t+'_local', type: 'text', maxlen: 15, size: 17, value: eval( 'nvram.vpn_'+t+'_local' ) },
			{ name: 'vpn_'+t+'_remote', type: 'text', maxlen: 15, size: 17, value: eval( 'nvram.vpn_'+t+'_remote' ) } ] },
		{ title: 'Tunnel address/netmask', multi: [
			{ name: 'f_vpn_'+t+'_local', type: 'text', maxlen: 15, size: 17, value: eval( 'nvram.vpn_'+t+'_local' ) },
			{ name: 'vpn_'+t+'_nm', type: 'text', maxlen: 15, size: 17, value: eval( 'nvram.vpn_'+t+'_nm' ) } ] }
	]);
	W('</div>');
	W('<div id=\''+t+'-advanced\'>');
	createFieldTable('', [
		{ title: 'Redirect Internet traffic', multi: [
			{ name: 'f_vpn_'+t+'_rgw', type: 'checkbox', value: eval( 'nvram.vpn_'+t+'_rgw' ) != 0 },
			{ name: 'vpn_'+t+'_gw', type: 'text', maxlen: 15, size: 17, value: eval( 'nvram.vpn_'+t+'_gw' ), prefix: '<span id=\''+t+'_gateway\'>Gateway:&nbsp', suffix: '</span>'} ] },
		{ title: 'Accept DNS configuration', name: 'vpn_'+t+'_adns', type: 'select', options: [[0, 'Disabled'],[1, 'Relaxed'],[2, 'Strict'],[3, 'Exclusive']], value: eval( 'nvram.vpn_'+t+'_adns' ) },
		{ title: 'Encryption cipher', name: 'vpn_'+t+'_cipher', type: 'select', options: ciphers, value: eval( 'nvram.vpn_'+t+'_cipher' ) },
		{ title: 'Compression', name: 'vpn_'+t+'_comp', type: 'select', options: [ ['-1', 'Disabled'], ['no', 'None'], ['yes', 'Enabled'], ['adaptive', 'Adaptive'] ], value: eval( 'nvram.vpn_'+t+'_comp' ) },
		{ title: 'TLS Renegotiation Time', name: 'vpn_'+t+'_reneg', type: 'text', maxlen: 10, size: 7, value: eval( 'nvram.vpn_'+t+'_reneg' ),
			suffix: '&nbsp;<small>(in seconds, -1 for default)</small>' },
		{ title: 'Connection retry', name: 'vpn_'+t+'_retry', type: 'text', maxlen: 5, size: 7, value: eval( 'nvram.vpn_'+t+'_retry' ),
			suffix: '&nbsp;<small>(in seconds; -1 for infinite)</small>' },
		{ title: 'Custom Configuration', name: 'vpn_'+t+'_custom', type: 'textarea', value: eval( 'nvram.vpn_'+t+'_custom' ) }
	]);
	W('</div>');
	W('<div id=\''+t+'-keys\'>');
	W('<p class=\'keyhelp\'>For help generating keys, refer to the OpenVPN <a id=\''+t+'-keyhelp\'>HOWTO</a>.</p>');
	createFieldTable('', [
		{ title: 'Static Key', name: 'vpn_'+t+'_static', type: 'textarea', value: eval( 'nvram.vpn_'+t+'_static' ) },
		{ title: 'Certificate Authority', name: 'vpn_'+t+'_ca', type: 'textarea', value: eval( 'nvram.vpn_'+t+'_ca' ) },
		{ title: 'Client Certificate', name: 'vpn_'+t+'_crt', type: 'textarea', value: eval( 'nvram.vpn_'+t+'_crt' ) },
		{ title: 'Client Key', name: 'vpn_'+t+'_key', type: 'textarea', value: eval( 'nvram.vpn_'+t+'_key' ) }
	]);
	W('</div>');
	W('<div id=\''+t+'-status\'>');
		W('<div id=\''+t+'-no-status\'><p>Client is not running or status could not be read.</p></div>');
		W('<div id=\''+t+'-status-content\' style=\'display:none\' class=\'status-content\'>');
			W('<div id=\''+t+'-status-header\' class=\'status-header\'><p>Data current as of <span id=\''+t+'-status-time\'></span>.</p></div>');
			W('<div id=\''+t+'-status-stats\'><div class=\'section-title\'>General Statistics</div><table class=\'tomato-grid status-table\' id=\''+t+'-status-stats-table\'></table><br></div>');
			W('<div id=\''+t+'-status-errors\' class=\'error\'></div>');
		W('</div>');
		W('<div style=\'text-align:right\'><a href=\'javascript:updateStatus('+i+')\'>Refresh Status</a></div>');
	W('</div>');
	W('<input type="button" value="' + (eval('vpn'+(i+1)+'up') ? 'Stop' : 'Start') + ' Now" onclick="toggle(\'vpn'+t+'\', vpn'+(i+1)+'up)" id="_vpn'+t+'_button">');
	W('</div>');
}

</script>
</div>

</td></tr>
	<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>init();</script>
</body>
</html>

