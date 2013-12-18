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
<title>[<% ident(); %>] Advanced: GAE Proxy</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("gaeproxy_enable,gaeproxy_sleep,gaeproxy_check,gaeproxy_check_time,gaeproxy_binary,gaeproxy_binary_custom,gaeproxy_binary_download,gaeproxy_listen_ip,gaeproxy_listen_port,gaeproxy_listen_web_username,gaeproxy_listen_web_password,gaeproxy_listen_username,gaeproxy_listen_password,gaeproxy_listen_transparent_enable,gaeproxy_pac_enable,gaeproxy_pac_file_enable,gaeproxy_pac_file,gaeproxy_pac_https_mode,gaeproxy_gae_enable,gaeproxy_gae_appid,gaeproxy_gae_password,gaeproxy_gae_listen,gaeproxy_gae_profile,gaeproxy_gae_max_threads,gaeproxy_gae_fetch_mode,gaeproxy_hosts_enable,gaeproxy_hosts_dns,gaeproxy_hosts_resolve,gaeproxy_autorange_enable,gaeproxy_autorange_maxsize,gaeproxy_autorange_waitsize,gaeproxy_autorange_bufsize,gaeproxy_proxy_enable,gaeproxy_proxy_host,gaeproxy_proxy_port,gaeproxy_proxy_username,gaeproxy_proxy_password,gaeproxy_useragent_enable,gaeproxy_useragent_match,gaeproxy_useragent_rules,gaeproxy_useragent_string,gaeproxy_autoupload_enable,gaeproxy_autoupload_gmail,gaeproxy_autoupload_password,gaeproxy_paas_enable,gaeproxy_paas_password,gaeproxy_paas_listen,gaeproxy_paas_fetchserver,gaeproxy_paas_proxy_enable,gaeproxy_paas_proxy,http_lanport"); %>

var proxygui_link = '&nbsp;&nbsp;<a href="http://' + location.hostname +':<% nv('gaeproxy_listen_port'); %>/" target="_blank"><i>[Click here to open WallProxy Config. GUI]</i></a>';
var autoupload_log_link = '&nbsp;&nbsp;<a href="http://' + location.hostname + ':' + <% nv('http_lanport'); %> + '/ext/autoupload.txt" target="_blank"><i>[Click here to open autoupload log]</i></a>';


function verifyFields(focused, quiet)
{
	var ok = 1;
	var a = E('_f_gaeproxy_enable').checked;
	var b = E('_f_gaeproxy_check').checked;
	var c = E('_f_gaeproxy_pac_enable').checked;
	var d = E('_f_gaeproxy_gae_enable').checked;
	var e = E('_f_gaeproxy_hosts_enable').checked;
	var g = E('_f_gaeproxy_autorange_enable').checked;
	var h = E('_f_gaeproxy_proxy_enable').checked;
	var i = E('_f_gaeproxy_autoupload_enable').checked;
	var j = E('_f_gaeproxy_useragent_enable').checked;
	var k = E('_f_gaeproxy_pac_file_enable').checked;
	var l = E('_f_gaeproxy_paas_enable').checked;
	var m = E('_f_gaeproxy_paas_proxy_enable').checked;
	
	E('_f_gaeproxy_check').disabled = !a;
	E('_gaeproxy_check_time').disabled = !a || !b;
	E('_gaeproxy_binary').disabled = !a;
	E('_gaeproxy_binary_custom').disabled = !a;
	E('_gaeproxy_binary_download').disabled = !a;
	E('_gaeproxy_sleep').disabled = !a;
	E('_gaeproxy_listen_ip').disabled = !a;
	E('_gaeproxy_listen_port').disabled = !a;
	E('_gaeproxy_listen_web_username').disabled = !a;
	E('_gaeproxy_listen_web_password').disabled = !a;
	E('_gaeproxy_listen_username').disabled = !a;
	E('_gaeproxy_listen_password').disabled = !a;
	E('_f_gaeproxy_listen_transparent_enable').disabled = 1;
	E('_f_gaeproxy_pac_enable').disabled = !a;
	E('_f_gaeproxy_pac_file_enable').disabled = !a;
	E('_gaeproxy_pac_file').disabled = !a || !c || !k;
	E('_gaeproxy_pac_https_mode').disabled = !a || !c;
	E('_f_gaeproxy_gae_enable').disabled = !a;
	E('_gaeproxy_gae_appid').disabled = !a || !d;
	E('_gaeproxy_gae_password').disabled = !a || !d;
	E('_gaeproxy_gae_listen').disabled = !a || !d;
	E('_gaeproxy_gae_profile').disabled = !a || !d;
	E('_gaeproxy_gae_max_threads').disabled = !a || !d;
	E('_gaeproxy_gae_fetch_mode').disabled = !a || !d;
	E('_f_gaeproxy_hosts_enable').disabled = !a;
	E('_gaeproxy_hosts_dns').disabled = !a || !e;
	E('_gaeproxy_hosts_resolve').disabled = !a || !e;
	E('_f_gaeproxy_autorange_enable').disabled = !a;
	E('_gaeproxy_autorange_maxsize').disabled = !a || !g;
	E('_gaeproxy_autorange_waitsize').disabled = !a || !g;
	E('_gaeproxy_autorange_bufsize').disabled = !a || !g;
	E('_f_gaeproxy_proxy_enable').disabled = !a;
	E('_gaeproxy_proxy_host').disabled = !a || !h;
	E('_gaeproxy_proxy_port').disabled = !a || !h;
	E('_gaeproxy_proxy_username').disabled = !a || !h;
	E('_gaeproxy_proxy_password').disabled = !a || !h;
	E('_f_gaeproxy_useragent_enable').disabled = !a;
	E('_gaeproxy_useragent_match').disabled = !a || !j
	E('_gaeproxy_useragent_rules').disabled = !a || !j
	E('_gaeproxy_useragent_string').disabled = !a || !j
	E('_f_gaeproxy_autoupload_enable').disabled = !a;
	E('_gaeproxy_autoupload_gmail').disabled = !a || !i;
	E('_gaeproxy_autoupload_password').disabled = !a || !i;
	E('_f_gaeproxy_paas_enable').disabled = !a;
	E('_gaeproxy_paas_password').disabled = !a || !l;
	E('_gaeproxy_paas_listen').disabled = !a || !l;
	E('_gaeproxy_paas_fetchserver').disabled = !a || !l;
	E('_f_gaeproxy_paas_proxy_enable').disabled = !a || !l;
	E('_gaeproxy_paas_proxy').disabled = !a || !l || !m;
	
	if (!v_port('_gaeproxy_listen_port', quiet)) ok = 0;
	if (!v_port('_gaeproxy_gae_listen', quiet)) ok = 0;
	if (!v_port('_gaeproxy_paas_listen', quiet)) ok = 0;

	var p = (E('_gaeproxy_binary').value == 'custom');
	elem.display('_gaeproxy_binary_custom', p && a);

	var q = (E('_gaeproxy_binary').value == 'download');
	elem.display('_gaeproxy_binary_download', q && a);

	return ok;
}

function save()
{
	if (verifyFields(null, 0)==0) return;
	var fom = E('_fom');
	fom.gaeproxy_enable.value =          E('_f_gaeproxy_enable').checked ? 1 : 0;
	fom.gaeproxy_check.value =           E('_f_gaeproxy_check').checked ? 1 : 0;
	fom.gaeproxy_pac_enable.value =      E('_f_gaeproxy_pac_enable').checked ? 1 : 0;
	fom.gaeproxy_pac_file_enable.value = E('_f_gaeproxy_pac_file_enable').checked ? 1 : 0;
	fom.gaeproxy_gae_enable.value =      E('_f_gaeproxy_gae_enable').checked ? 1 : 0;
	fom.gaeproxy_hosts_enable.value =    E('_f_gaeproxy_hosts_enable').checked ? 1 : 0;
	fom.gaeproxy_autorange_enable.value =E('_f_gaeproxy_autorange_enable').checked ? 1 : 0;
	fom.gaeproxy_proxy_enable.value =    E('_f_gaeproxy_proxy_enable').checked ? 1 : 0;
	fom.gaeproxy_autoupload_enable.value=E('_f_gaeproxy_autoupload_enable').checked ? 1 : 0;
	fom.gaeproxy_useragent_enable.value =E('_f_gaeproxy_useragent_enable').checked ? 1 : 0;
	fom.gaeproxy_listen_transparent_enable.value =E('_f_gaeproxy_listen_transparent_enable').checked ? 1 : 0;
	fom.gaeproxy_paas_enable.value =     E('_f_gaeproxy_paas_enable').checked ? 1 : 0;
	fom.gaeproxy_paas_proxy_enable.value=E('_f_gaeproxy_paas_proxy_enable').checked ? 1 : 0;
	
	if (fom.gaeproxy_enable.value == 0) {
		fom._service.value = 'gaeproxy-stop';
	}
	else {
		fom._service.value = 'gaeproxy-restart'; 
	}
	form.submit('_fom', 1);
}

</script>
</head>

<body onLoad="init()">
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<input type='hidden' name='_nextpage' value='advanced-gaeproxy.asp'>
<input type='hidden' name='_service' value='gaeproxy-restart'>
<input type='hidden' name='gaeproxy_enable'>
<input type='hidden' name='gaeproxy_check'>
<input type='hidden' name='gaeproxy_pac_enable'>
<input type='hidden' name='gaeproxy_gae_enable'>
<input type='hidden' name='gaeproxy_hosts_enable'>
<input type='hidden' name='gaeproxy_autorange_enable'>
<input type='hidden' name='gaeproxy_proxy_enable'>
<input type='hidden' name='gaeproxy_autoupload_enable'>
<input type='hidden' name='gaeproxy_useragent_enable'>
<input type='hidden' name='gaeproxy_listen_transparent_enable'>
<input type='hidden' name='gaeproxy_pac_file_enable'>
<input type='hidden' name='gaeproxy_paas_enable'>
<input type='hidden' name='gaeproxy_paas_proxy_enable'>

<!-- Menu Section Begin -->
<div class='section-title'>Basic Settings<script>W(proxygui_link);</script></div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable WallProxy', name: 'f_gaeproxy_enable', type: 'checkbox', value: nvram.gaeproxy_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Startup.py Location', multi: [
		{ name: 'gaeproxy_binary', type: 'select', options: [
			['internal','Internal (.../gaeproxy/local/startup.py)'],
			['optware','Optware (/mnt/opt/wallproxy/local/startup.py)'],
			['download','Download'],
			['custom','Custom'] ], value: nvram.gaeproxy_binary, suffix: ' <small>*</small> ' },
		{ name: 'gaeproxy_binary_download', type: 'text', maxlen: 128, size: 50, value: nvram.gaeproxy_binary_download },
		{ name: 'gaeproxy_binary_custom', type: 'text', maxlen: 50, size: 50, value: nvram.gaeproxy_binary_custom }
	] },
	{ title: 'Keep alive', name: 'f_gaeproxy_check', type: 'checkbox', value: nvram.gaeproxy_check == 1, suffix: ' <small>*</small>' },
	{ title: 'Check alive every', indent: 2, name: 'gaeproxy_check_time', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_check_time, suffix: ' <small>minutes (range: 1 - 55; default: 5)</small>' },
	{ title: 'Delay at startup', name: 'gaeproxy_sleep', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_sleep, suffix: ' <small>seconds (range: 1 - 60; default: 10)</small>' },
	{ title: 'WallProxy Listening IP', name: 'gaeproxy_listen_ip', type: 'text', maxlen: 15, size: 17, value: nvram.gaeproxy_listen_ip, suffix: ' <small>*</small>' },
	{ title: 'WallProxy Listening port', name: 'gaeproxy_listen_port', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_listen_port, suffix: ' <small>*</small>' },
	{ title: 'WallProxy Username', name: 'gaeproxy_listen_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_username },
	{ title: 'WallProxy Password', name: 'gaeproxy_listen_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_password },
	{ title: 'Web Username', name: 'gaeproxy_listen_web_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_web_username },
	{ title: 'Web Password', name: 'gaeproxy_listen_web_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_web_password },
	{ title: 'Enable Transparent Proxy', name: 'f_gaeproxy_listen_transparent_enable', type: 'checkbox', value: nvram.gaeproxy_listen_transparent_enable == 1 }
	]);  
</script>
<ul>
		<li><b>Enable WallProxy</b> - Caution! - If your router only has 32MB of RAM, you'll have to use swap.
		<li><b>Startup.py Location</b> - if selected 'Custom', pls. input full path including startup.py. If selected 'Download',input HTTP or FTP URL.
		<li><b>Enable WallProxy</b> - Caution! - If your router only has 32MB of RAM, you'll have to use swap.
		<li><b>Keep alive</b> - If enabled, proxy daemon will be checked at the specified interval and will re-launch after a crash.
		<li><b>WallProxy Username/Password</b> - To avoid stealth, clients must be setup username and password when access proxy server.
		<li><b>Web Username/Password</b> - The username/password when you access the web configuration through http://listen_ip:listen_port/web.
		<li><b>Enable Transparent Proxy</b> - Catch data from 80 and forward to port of wallproxy like a transparent proxy activity.
</ul>
</div>

<div class='section-title'>GAE Settings</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable GAE', name: 'f_gaeproxy_gae_enable', type: 'checkbox', value: nvram.gaeproxy_gae_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'GAE APPIDs', name: 'gaeproxy_gae_appid', type: 'text', maxlen: 512, size: 64, value: nvram.gaeproxy_gae_appid, suffix: ' <small>Max number of appid is 10.</small>' },
	{ title: 'GAE Listening port', name: 'gaeproxy_gae_listen', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_gae_listen, suffix: ' <small>*</small>' },
	{ title: 'GAE Password', name: 'gaeproxy_gae_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_gae_password },
	{ title: 'GAE Profile', name: 'gaeproxy_gae_profile', type: 'select', options: [ ['google_hk','google_hk*'], ['google_cn','google_cn'], ['google_ipv6','google_ipv6'] ], value: nvram.gaeproxy_gae_profile, suffix: ' ' },
	{ title: 'Max threads', name: 'gaeproxy_gae_max_threads', type: 'text', maxlen: 10, size: 7, value: nvram.gaeproxy_gae_max_threads, suffix: ' <small>(range: 0 - 10; default: 3)</small>' },
	{ title: 'Fetch Mode', name: 'gaeproxy_gae_fetch_mode', type: 'select', options: [ ['0','Smooth Cached'], ['1','Partial Cached*'], ['2','No Cached'] ], value: nvram.gaeproxy_gae_fetch_mode, suffix: ' ' }
	]);  
</script>
<ul>
		<li><b>Max Threads</b> - Max threads of each resource when to download by multi. parts.
		<li><b>Fetch Mode</b> - 0: Maybe not stable; 1 and 2: stable but no smooth cached. Select 0 for stable network and 1 for non-stable network.
</ul>
</div>

<div class='section-title'>Proxy PAC</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable PAC', name: 'f_gaeproxy_pac_enable', type: 'checkbox', value: nvram.gaeproxy_pac_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Enable PAC File', multi: [
		{ name: 'f_gaeproxy_pac_file_enable', type: 'checkbox', value: nvram.gaeproxy_pac_file_enable == 1, suffix: '  ' },
		{ name: 'gaeproxy_pac_file', type: 'text', maxlen: 256, size: 64, value: nvram.gaeproxy_pac_file } ] },
	{ title: 'HTTPS Mode', name: 'gaeproxy_pac_https_mode', type: 'select', options: [ ['0','Direct All, No Faked CA'], ['1','HTTPS All, Faked CA*'], ['2','Partial Direct&Facked CA'] ], value: nvram.gaeproxy_pac_https_mode, suffix: ' ' }
	]);  
</script>
<ul>
		<li><b>PAC Filename</b> - If config, pac file will be auto updated and smart proxy will be invalid.
</ul>
</div>

<div class='section-title'>AutoRange Download</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable AutoRange', name: 'f_gaeproxy_autorange_enable', type: 'checkbox', value: nvram.gaeproxy_autorange_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Max Size', name: 'gaeproxy_autorange_maxsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_maxsize, suffix: ' <small>Byte (range: 0 - 99999999; default: 1000000)</small>' },
	{ title: 'Wait Size', name: 'gaeproxy_autorange_waitsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_waitsize, suffix: ' <small>Byte (range: 0 - 99999999; default: 1000000)</small>' },
	{ title: 'Buffered Size', name: 'gaeproxy_autorange_bufsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_bufsize, suffix: ' <small>Byte (range: 0 - 1024000; default: 8192)</small>' }
	]);  
</script>
</div>

<div class='section-title'>PAAS Server</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable PAAS Server', name: 'f_gaeproxy_paas_enable', type: 'checkbox', value: nvram.gaeproxy_paas_enable == 1, suffix: ' <small>Default: 0</small>' },
	{ title: 'PAAS Password', name: 'gaeproxy_paas_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_paas_password },
	{ title: 'PAAS Listen Port', name: 'gaeproxy_paas_listen', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_paas_listen },
	{ title: 'Fetch Server', name: 'gaeproxy_paas_fetchserver', type: 'text', maxlen: 512, size: 64, value: nvram.gaeproxy_paas_fetchserver },
	{ title: 'Enable Higher Proxy', multi: [
		{ name: 'f_gaeproxy_paas_proxy_enable', type: 'checkbox', value: nvram.gaeproxy_paas_proxy_enable == 1, suffix: '  ' },
		{ name: 'gaeproxy_paas_proxy', type: 'text', maxlen: 128, size: 64, value: nvram.gaeproxy_paas_proxy } ] }
	]);  
</script>
</div>

<div class='section-title'>Hosts Rule</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable Hosts', name: 'f_gaeproxy_hosts_enable', type: 'checkbox', value: nvram.gaeproxy_hosts_enable == 1, suffix: ' <small>IPv6 should be disabled.*</small>' },
	{ title: 'DNS', name: 'gaeproxy_hosts_dns', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_hosts_dns },
	{ title: 'Resolve by DNS', name: 'gaeproxy_hosts_resolve', type: 'text', maxlen: 1024, size: 100, value: nvram.gaeproxy_hosts_resolve }
	]);  
</script>
</div>

<div class='section-title'>Higher Proxy</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable Higher Proxy', name: 'f_gaeproxy_proxy_enable', type: 'checkbox', value: nvram.gaeproxy_proxy_enable == 1, suffix: ' <small> * Enable it if you through other higher proxy.</small>' },
	{ title: 'Proxy IP', name: 'gaeproxy_proxy_host', type: 'text', maxlen: 15, size: 17, value: nvram.gaeproxy_proxy_host, suffix: ' <small>*</small>' },
	{ title: 'Proxy Port', name: 'gaeproxy_proxy_port', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_proxy_port, suffix: ' <small>*</small>' },
	{ title: 'Proxy Username', name: 'gaeproxy_proxy_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_proxy_username },
	{ title: 'Proxy Password', name: 'gaeproxy_proxy_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_proxy_password }
	]);  
</script>
</div>

<div class='section-title'>User Agent</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable UserAgent Rewrite', name: 'f_gaeproxy_useragent_enable', type: 'checkbox', value: nvram.gaeproxy_useragent_enable == 1 },
	{ title: 'Matched Regex', name: 'gaeproxy_useragent_match', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_useragent_match },
	{ title: 'Matched Rules', name: 'gaeproxy_useragent_rules', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_useragent_rules },
	{ title: 'New UserAgent', name: 'gaeproxy_useragent_string', type: 'text', maxlen: 256, size: 100, value: nvram.gaeproxy_useragent_string }
	]);  
</script>
</div>

<div class='section-title'>Autoupload to GAE Server<script>W(autoupload_log_link);</script></div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable Autoupload', name: 'f_gaeproxy_autoupload_enable', type: 'checkbox', value: nvram.gaeproxy_autoupload_enable == 1 },
	{ title: 'GMail Address', name: 'gaeproxy_autoupload_gmail', type: 'text', maxlen: 64, size: 64, value: nvram.gaeproxy_autoupload_gmail },
	{ title: 'GAE Password', name: 'gaeproxy_autoupload_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_autoupload_password }
	]);  
</script>
<ul>
		<li><b>Enable Autoupload</b> - If enable, it will auto upload config. files to GAE servers.
		<li><b>Note:</b> - Pls. wait 5 minutes to finish upload, you can review log file to check status.
</ul>
</div>

<!-- Menu Section End -->

</td></tr>
</form>

<tr><td id='footer' colspan=2>
<form>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='reloadPage();'>
</form>
</td></tr>
</table>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
