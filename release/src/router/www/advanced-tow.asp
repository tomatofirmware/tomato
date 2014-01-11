<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2013 Hyzoom bwq518
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Advanced: Transparent over Wall</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->
<style type='text/css'>
textarea {
 width: 98%;
 height: 15em;
}
</style>
<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("tow_enable,tow_check,tow_check_time,tow_sleep,tow_mode,tow_iprange_all,tow_iprange_start,tow_iprange_end,tow_pdnsd_localdns_useisp,tow_pdnsd_localdns_ip,tow_pdnsd_localdns_port,tow_pdnsd_opendns_ip,tow_pdnsd_opendns_port,tow_pdnsd_reject_ip_add,tow_pdnsd_exclude_domain,tow_gfwlist_enable,tow_gfwlist_url,tow_gfwlist_add,tow_tunlr_custom_enable,tow_tunlr_custom,tow_tunlr_url,tow_whitelist_enable,tow_whitelist_url,tow_whitelist_add,tow_pandalist_url,tow_tunlr_domains_add,tow_ssh_server,tow_ssh_port,tow_ssh_username,tow_ssh_passwd,tow_ssh_obfcode,tow_ssh_listen_port,tow_ssh_argv,tow_ss_server,tow_ss_server_port,tow_ss_crypt_method,tow_ss_passwd,tow_ss_local_port,tow_ss_redir_local_port,tow_redsocks_gae_port,tow_redsocks_wp_port"); %>

function verifyFields(focused, quiet)
{
	var ok = 1;

	var a = E('_f_tow_enable').checked;
	var b = E('_f_tow_check').checked;
	var c = E('_f_tow_iprange_all').checked;
	var d = E('_f_tow_pdnsd_localdns_useisp').checked;
	var e = E('_f_tow_gfwlist_enable').checked;
	var f = E('_f_tow_whitelist_enable').checked;
	var l = E('_f_tow_tunlr_custom_enable').checked;
	var s = (E('_tow_mode').value == 'ss');
	var r = (E('_tow_mode').value == 'redir');
	var o = (E('_tow_mode').value == 'ssh');
	var g = (E('_tow_mode').value == 'gae');
	var w = (E('_tow_mode').value == 'wp');
	
	E('_f_tow_check').disabled = !a;
	E('_tow_check_time').disabled = !a || !b;
	E('_tow_sleep').disabled = !a;
	E('_tow_mode').disabled = !a;
	E('_f_tow_iprange_all').disabled = !a;
	E('_tow_iprange_start').disabled = !a || c;
	E('_tow_iprange_end').disabled = !a || c;
	E('_f_tow_pdnsd_localdns_useisp').disabled = !a;
	E('_tow_pdnsd_localdns_ip').disabled = !a || d;
	E('_tow_pdnsd_localdns_port').disabled = !a;
	E('_tow_pdnsd_opendns_ip').disabled = !a;
	E('_tow_pdnsd_opendns_port').disabled = !a;
	E('_tow_pdnsd_reject_ip_add').disabled = !a;
	E('_tow_pdnsd_exclude_domain').disabled = !a;
	E('_f_tow_gfwlist_enable').disabled = !a;
	E('_tow_gfwlist_url').disabled = !a || !e;
	E('_tow_gfwlist_add').disabled = !a || !e;
//	E('_tow_lanternlist_url').disabled = !a || !e;
	E('_f_tow_tunlr_custom_enable').disabled = !a;
	E('_tow_tunlr_custom').disabled = !a || !l;
	E('_tow_tunlr_url').disabled = !a || l;
	E('_f_tow_whitelist_enable').disabled = !a;
	E('_tow_whitelist_url').disabled = !a || !f;
	E('_tow_whitelist_add').disabled = !a || !f;
	E('_tow_pandalist_url').disabled = !a || !f;
	E('_tow_tunlr_domains_add').disabled = !a;
	E('_tow_ssh_server').disabled = !a;
	E('_tow_ssh_port').disabled = !a;
	E('_tow_ssh_username').disabled = !a;
	E('_tow_ssh_passwd').disabled = !a;
	E('_tow_ssh_obfcode').disabled = !a;
//	E('_tow_ssh_listen_port').disabled = !a;
	E('_tow_ssh_argv').disabled = !a;
	E('_tow_ss_server').disabled = !a;
	E('_tow_ss_server_port').disabled = !a;
	E('_tow_ss_crypt_method').disabled = !a;
	E('_tow_ss_passwd').disabled = !a;
//	E('_tow_ss_local_port').disabled = !a;
//	E('_tow_ss_redir_local_port').disabled = !a;
	E('_tow_redsocks_gae_port').disabled = !a;
	E('_tow_redsocks_wp_port').disabled = !a;	
	
	var x;
	if ( s  || r ) x = '';
	else x = 'none';
	PR(E('_tow_ss_server')).style.display = x;
	PR(E('_tow_ss_server')).style.display = x;
	PR(E('_tow_ss_server_port')).style.display = x;
	PR(E('_tow_ss_crypt_method')).style.display = x;
	PR(E('_tow_ss_passwd')).style.display = x;
//	PR(E('_tow_ss_local_port')).style.display = x;
//	PR(E('_tow_ss_redir_local_port')).style.display = x;

	if ( o ) x = '';
	else x = 'none';
	PR(E('_tow_ssh_server')).style.display = x;
	PR(E('_tow_ssh_port')).style.display = x;
	PR(E('_tow_ssh_username')).style.display = x;
	PR(E('_tow_ssh_passwd')).style.display = x;
	PR(E('_tow_ssh_obfcode')).style.display = x;
//	PR(E('_tow_ssh_listen_port')).style.display = x;
	PR(E('_tow_ssh_argv')).style.display = x;

	if ( g ) x = '';
	else x = 'none';
	PR(E('_tow_redsocks_gae_port')).style.display = x;
	if ( w ) x = '';
	else x = 'none';
	PR(E('_tow_redsocks_wp_port')).style.display = x;

	if (!v_port('_tow_pdnsd_localdns_port', quiet)) return 0;
	if (!v_port('_tow_ssh_port', quiet)) return 0;
//	if (!v_port('_tow_ssh_listen_port', quiet)) return 0;
	if (!v_port('_tow_ss_server_port', quiet)) return 0;
//	if (!v_port('_tow_ss_local_port', quiet)) return 0;
//	if (!v_port('_tow_ss_redir_local_port', quiet)) return 0;
	if (!v_port('_tow_redsocks_gae_port', quiet)) return 0;
	if (!v_port('_tow_redsocks_wp_port', quiet)) return 0;

	if (!v_length('_tow_pdnsd_reject_ip_add', quiet, 0, 1024)) ok = 0;
	if (!v_length('_tow_gfwlist_add', quiet, 0, 4096)) ok = 0;
	if (!v_length('_tow_whitelist_add', quiet, 0, 4096)) ok = 0;

	return ok;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');

  fom.tow_enable.value			= E('_f_tow_enable').checked ? 1 : 0;
  fom.tow_check.value			= E('_f_tow_check').checked ? 1 : 0;
  fom.tow_iprange_all.value		= E('_f_tow_iprange_all').checked ? 1 : 0;
  fom.tow_pdnsd_localdns_useisp.value	= E('_f_tow_pdnsd_localdns_useisp').checked ? 1 : 0;
  fom.tow_gfwlist_enable.value		= E('_f_tow_gfwlist_enable').checked ? 1 : 0;
  fom.tow_tunlr_custom_enable.value	= E('_f_tow_tunlr_custom_enable').checked ? 1 : 0;
  fom.tow_whitelist_enable.value	= E('_f_tow_whitelist_enable').checked ? 1 : 0;
	
  if (fom.tow_enable.value == 0) {
  	fom._service.value = 'tow-stop';
  }
  else {
  	fom._service.value = 'tow-restart'; 
  }
  form.submit('_fom', 1);

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
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='advanced-tow.asp'>
<input type='hidden' name='_service' value='tow-restart'>
<input type='hidden' name='tow_enable'>
<input type='hidden' name='tow_check'>
<input type='hidden' name='tow_iprange_all'>
<input type='hidden' name='tow_pdnsd_localdns_useisp'>
<input type='hidden' name='tow_gfwlist_enable'>
<input type='hidden' name='tow_whitelist_enable'>
<input type='hidden' name='tow_tunlr_custom_enable'>

<div class='section-title'>Basic Settings</div>
<div class='section' id='config-section'>
<script type='text/javascript'>	
createFieldTable('', [
	{ title: 'Enable ToW', name: 'f_tow_enable', type: 'checkbox', value: nvram.tow_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Keep alive', name: 'f_tow_check', type: 'checkbox', value: nvram.tow_check == 1, suffix: ' <small>*</small>' },
	{ title: 'Check alive every', indent: 2, name: 'tow_check_time', type: 'text', maxlen: 5, size: 7, value: nvram.tow_check_time, suffix: ' <small>minutes (range: 1 - 55; default: 1)</small>' },
	{ title: 'Delay at startup', name: 'tow_sleep', type: 'text', maxlen: 5, size: 7, value: nvram.tow_sleep, suffix: ' <small>seconds (range: 1 - 60; default: 10)</small>' },
	{ title: 'Apply to Global IP', name: 'f_tow_iprange_all', type: 'checkbox', value: nvram.tow_iprange_all == 1, suffix: ' <small>*</small>' },
	{ title: 'IP Range from', name: 'tow_iprange_start', type: 'text', maxlen: 5, size: 5, value: nvram.tow_iprange_start, suffix: ' <small>*</small>' },
	{ title: 'IP range to', name: 'tow_iprange_end', type: 'text', maxlen: 5, size: 5, value: nvram.tow_iprange_end, suffix: ' <small>*</small>' },
	{ title: 'ToW Mode', multi: [
		{ name: 'tow_mode', type: 'select', options: [
			['ss','Shadowsocks Local'],
			['redir','Shadowsocks Redir'],
			['ssh','Obfuscated SSH'],
			['gae','GoAgent'],
			['wp','WallProxy'] ], value: nvram.tow_mode, suffix: ' <small>*</small> ' }	] },
	
	{ title: 'Shadowsocks Server', indent: 2, name: 'tow_ss_server', type: 'text', maxlen: 64, size: 50, value: nvram.tow_ss_server, suffix: ' <small>*</small>' },
	{ title: 'Shadowsocks Remote Port', indent: 2, name: 'tow_ss_server_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_ss_server_port, suffix: ' <small>*</small>' },
	{ title: 'Shadowsocks Server Password', indent: 2, name: 'tow_ss_passwd', type: 'password', maxlen: 64, size: 50, value: nvram.tow_ss_passwd, suffix: ' <small>*</small>' },
//	{ title: 'Shadowsocks Listen Port', indent: 2, name: 'tow_ss_local_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_ss_local_port, suffix: ' <small>If do not understand, keep it unchanged !</small>' },
//	{ title: 'Shadowsocks Redir Listen Port', indent: 2, name: 'tow_ss_redir_local_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_ss_redir_local_port, suffix: ' <small>If do not understand, keep it unchanged !</small>' },
	{ title: 'Shadowsocks Crypt Method', indent: 2, multi: [
		{ name: 'tow_ss_crypt_method', type: 'select', options: [ 
		['aes-128-cfb','aes-128-cfb'], 
		['aes-192-cfb','aes-192-cfb'], 
		['aes-256-cfb','aes-256-cfb'], 
		['bf-cfb','bf-cfb'], 
		['camellia-128-cfb','camellia-128-cfb'], 
		['camellia-192-cfb','camellia-192-cfb'], 
		['camellia-256-cfb','camellia-256-cfb'], 
		['cast5-cfb','cast5-cfb'], 
		['des-cfb','des-cfb'], 
		['idea-cfb','idea-cfb'], 
		['rc2-cfb','rc2-cfb'], 
		['rc4','rc4'], 
		['seed-cfb','seed-cfb'], 
		['table','table'] ], value: nvram.tow_ss_crypt_method, suffix: ' <small>*</small> ' }	] },

	{ title: 'SSH Server', indent: 2, name: 'tow_ssh_server', type: 'text', maxlen: 64, size: 50, value: nvram.tow_ssh_server, suffix: ' <small>*</small>' },
	{ title: 'SSH Remote Port', indent: 2, name: 'tow_ssh_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_ssh_port, suffix: ' <small>*</small>' },
	{ title: 'SSH Server Username', indent: 2, name: 'tow_ssh_username', type: 'text', maxlen: 64, size: 50, value: nvram.tow_ssh_username, suffix: ' <small>*</small>' },
	{ title: 'SSH Server Password', indent: 2, name: 'tow_ssh_passwd', type: 'password', maxlen: 64, size: 50, value: nvram.tow_ssh_passwd, suffix: ' <small>*</small>' },
	{ title: 'SSH Obfuscated Code', indent: 2, name: 'tow_ssh_obfcode', type: 'text', maxlen: 64, size: 20, value: nvram.tow_ssh_obfcode, suffix: ' <small>*Leave blank for normal SSH without obfuscation</small>' },
	{ title: 'SSH Start Arguments', indent: 2, name: 'tow_ssh_argv', type: 'text', maxlen: 256, size: 80, value: nvram.tow_ssh_argv, suffix: ' <small>*</small>' },
//	{ title: 'SSH Listen Port', indent: 2, name: 'tow_ssh_listen_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_ssh_listen_port, suffix: ' <small>If do not understand, keep it unchanged !</small>' },

	{ title: 'GoAgent Port', indent: 2, name: 'tow_redsocks_gae_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_redsocks_gae_port, suffix: ' <small>*</small>' },
	{ title: 'WallProxy Port', indent: 2, name: 'tow_redsocks_wp_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_redsocks_wp_port, suffix: ' <small>*</small>' },
	{ title: 'Enable gfwlist', name: 'f_tow_gfwlist_enable', type: 'checkbox', value: nvram.tow_gfwlist_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'URL of gfwlist', indent: 2, name: 'tow_gfwlist_url', type: 'text', maxlen: 512, size: 80, value: nvram.tow_gfwlist_url },
//	{ title: 'URL of lanternlist', indent: 2, name: 'tow_lanternlist_url', type: 'text', maxlen: 512, size: 50, value: nvram.tow_lanternlist_url, suffix: ' <small>URL for downloading lanternlist</small>' },
	{ title: 'Add domains to gfwlist<br>(Each line consists of one domain)', indent: 2, name: 'tow_gfwlist_add', type: 'textarea', suffix: '<input type="button" id="btn_gfwlist" value="Current gfwlist for IPSet" onclick="javascript:window.open(\'/ext/gfwlist.cfg.txt\')">', value: nvram.tow_gfwlist_add },

	{ title: 'Enable whitelist', name: 'f_tow_whitelist_enable', type: 'checkbox', value: nvram.tow_whitelist_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'URL of whitelist 1', indent: 2, name: 'tow_whitelist_url', type: 'text', maxlen: 512, size: 80, value: nvram.tow_whitelist_url },
	{ title: 'URL of whitelist 2', indent: 2, name: 'tow_pandalist_url', type: 'text', maxlen: 512, size: 80, value: nvram.tow_pandalist_url },
	{ title: 'Add domains to whitelist<br>(Each line consists of one domain)',indent: 2,  name: 'tow_whitelist_add', type: 'textarea', suffix: '<input type="button" id="btn_whitelist" value="Current White List for IPSet" onclick="javascript:window.open(\'/ext/whitelist.cfg.txt\')">', value: nvram.tow_whitelist_add },
	{ title: 'Enable custom Tunlr DNS', name: 'f_tow_tunlr_custom_enable', type: 'checkbox', value: nvram.tow_tunlr_custom_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Custom Tunlr DNS IPs', indent: 2, name: 'tow_tunlr_custom', type: 'text', maxlen: 128, size: 80, value: nvram.tow_tunlr_custom },
	{ title: 'URL of Tunlr DNS', indent: 2, name: 'tow_tunlr_url', type: 'text', maxlen: 512, size: 60, value: nvram.tow_tunlr_url, suffix: ' <small>Breakthrough websites with area limitation</small>' },
	{ title: 'Add domains to Tunlr DNS list<br>(Each line consists of one domain)', name: 'tow_tunlr_domains_add', type: 'textarea', suffix: '<input type="button" id="btn_tunlr_domains" value="List of exist tunlr domains" onclick="javascript:window.open(\'/ext/tunlr_domains.htm\')">', value: nvram.tow_tunlr_domains_add }
]);
</script>
	<ul>
		<li><b>gfwlist and whitelist</b> - Rules as follows
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(1) If enabled gfwlist only, all sites will be pass-through except for those in gfwlist which will be accessed through proxy. 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(2) If enabled whitelist only, all sites will be pass-through except for those out of whitelist which will be accessed through proxy. 
		<li><b>Suggestions</b>
			<li>&nbsp;&nbsp;&nbsp;&nbsp;- Most time you access domestic or several famous blocked sites(e.g. YouTube, Twitter, Facebook, etc.), only enable gfwlist. 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;- Most time you access oversea's sites and do not care internet flow, only enable whitelist. 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;- It it not recommended to enable gfwlist and whitelist at the same time. 
		<li><b>Attention !</b> - If enabled ToW, port 5454, 7070, 7171, 7272, 8094-8099 will be used.
		<li><b>Attention !</b> - Be sure router's IP is the only DNS server of your client. Ensure to flush DNS cache (e.g. ipconfig /flushdns under windows).
		<li><b>Enable ToW</b> - If enabled, the following steps will be done.
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(1) Processes pdnsd, ss-local, ss-redir, redsocks, ssh_ofc will be launched. 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(2) Firewall chains will be modified.
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(3) Service dnsmasq will be restarted according to custom config.
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(4) All of scripts are saved in /etc/tow
		<li><b>ToW Mode</b> - Mode of ToW, should be Shadowsocks (local or redir), obfuscated SSH, GoAgent or WallProxy.
		<li><b>IP Range from/to</b> - Only IP addr. between 192.168.1.<b>[from]</b> and 192.168.1.<b>[to]</b> will be valid for ToW proxy
		<li><b>Keep alive</b> - If enabled, all processes will be checked at the specified interval and will re-launch after crash.
	</ul>
</div>
<div class='section-title'>PDNSD Settings</script></div>
<div class='section' id='config-section'>
<script type='text/javascript'>	
createFieldTable('', [
	{ title: 'Use DNS of ISP', name: 'f_tow_pdnsd_localdns_useisp', type: 'checkbox', value: nvram.tow_pdnsd_localdns_useisp == 1, suffix: ' <small>*The first priority DNS server.</small>' },
	{ title: 'Custom DNS', indent: 2, name: 'tow_pdnsd_localdns_ip', type: 'text', maxlen: 64, size: 50, value: nvram.tow_pdnsd_localdns_ip },
	{ title: 'DNS Port', indent: 2, name: 'tow_pdnsd_localdns_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_pdnsd_localdns_port, suffix: ' <small>*</small>' },
	{ title: 'Add domains to exclude list', indent: 2, name: 'tow_pdnsd_exclude_domain', type: 'text', maxlen: 512, size: 50, value: nvram.tow_pdnsd_exclude_domain, suffix: ' <small>These domains are never resolved by the first DNS server.</small>' },
	{ title: 'Add IPs to reject list<br>[If IP resolved by the first DNS server is in this list, then force to discard and resolve again through the backup DNS server.]<br>(Each line consists of one IP)', indent: 2, name: 'tow_pdnsd_reject_ip_add', type: 'textarea', suffix: '<input type="button" id="btn_pdnsd_reject_ip" value="List of exist rejected IPs" onclick="javascript:window.open(\'/ext/pdnsd_reject_ip.htm\')">', value: nvram.tow_pdnsd_reject_ip_add },
	{ title: 'OpenDNS IP', name: 'tow_pdnsd_opendns_ip', type: 'text', maxlen: 64, size: 50, value: nvram.tow_pdnsd_opendns_ip, suffix: ' <small>*The backup DNS server.</small>' },
	{ title: 'OpenDNS Port', indent: 2, name: 'tow_pdnsd_opendns_port', type: 'text', maxlen: 8, size: 10, value: nvram.tow_pdnsd_opendns_port, suffix: ' <small>*</small>' }
]);
</script>
	<ul>
		<li><b>Use DNS of ISP</b> - If selected, will use DNS servers as the following rule.
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(1) Use Static DNS, if not set then 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(2) Use DNS got from WAN, if not set then 
			<li>&nbsp;&nbsp;&nbsp;&nbsp;(3) Use Custom DNS set in this page 
	</ul>
</div>
</form>
</div>
<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
<form>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
