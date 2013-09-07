<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato RAF Aria2 GUI
	Copyright (C) 2007-2011 TomatoEgg
	http://openlinksys.info
	For use with Tomato RAF Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Nas: Aria2 Client</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>
<style type='text/css'>
textarea {
 width: 98%;
 height: 15em;
}
</style>
<script type='text/javascript'>
//	<% nvram("aria2_enable,aria2_custom,aria2_port,aria2_dir,aria2_settings,aria2_port_rpc,aria2_ssl_rpc"); %>

function verifyFields(focused, quiet)
{
	var ok = 1;

	var a = E('_f_aria2_enable').checked;

	E('_aria2_custom').disabled = !a;
	E('_aria2_ssl_rpc').disabled = !a;
	E('_aria2_dir').disabled = !a;
	E('_aria2_port').disabled = !a;
	E('_aria2_settings').disabled = !a;
	E('_aria2_port_rpc').disabled = !a;
	if (!v_length('_aria2_custom', quiet, 0, 2048)) ok = 0;

	var s = E('_aria2_custom');
	if (s.value.search(/"rpc-enable":/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-enable" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	return ok;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');
  fom.aria2_enable.value = E('_f_aria2_enable').checked ? 1 : 0;
  if (fom.aria2_enable.value == 0) {
  	fom._service.value = 'aria2-stop';
  }
  else {
  	fom._service.value = 'aria2-restart'; 
  }
	form.submit('_fom', 1);
}

function init()
{
}
</script>
</head>

<body onLoad="init()">
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
<div class='title'>Tomato</div>
<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<div class='section-title'>Basic Settings</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='nas-aria2.asp'>
<input type='hidden' name='_service' value='aria2-restart'>
<input type='hidden' name='aria2_enable'>

<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable torrent client', name: 'f_aria2_enable', type: 'checkbox', value: nvram.aria2_enable == '1', suffix: ' <small>*</small>' },
	{ title: 'Listening port', name: 'aria2_port', type: 'text', maxlen: 5, size: 7, value: nvram.aria2_port > 0 ? nvram.aria2_port : 6881, suffix: ' <small>*</small>' },
	{ title: 'Download directory', name: 'aria2_dir', type: 'text', maxlen: 40, size: 40, value: nvram.aria2_dir }
]);
</script>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Listening RPC port', indent: 2, name: 'aria2_port_rpc', type: 'text', maxlen: 32, size: 5, value: nvram.aria2_port_rpc > 0 ? nvram.aria2_port_rpc : 6800, suffix: ' <small>*</small>' },
	{ title: 'Use SSL to encrypt RPC traffic', indent: 2, name: 'aria2_ssl_rpc', type: 'checkout', value: nvram.aria2_ssl_rpc , suffix: ' <small>*</small>' },
	{ title: 'Save settings location',name: 'aria2_settings', type: 'select', options: [
			['down_dir','In the Download directory (Recommended)'],
/* JFFS2-BEGIN */
			['/jffs','JFFS2'],
/* JFFS2-END */
/* CIFS-BEGIN */
			['/cifs1','CIFS 1'],['/cifs2','CIFS 2'],
/* CIFS-END */
			['/tmp','RAM (Temporary)']], value: nvram.aria2_settings, suffix: ' ' },
    null,
    { title: '<a href="http://aria2.sourceforge.net/manual/en/html/aria2c.html" target="_new">Aria2</a><br>Custom configuration', name: 'aria2_custom', type: 'textarea', value: nvram.aria2_custom }
]);
</script>
	<ul>
		<li><b>Enable torrent client</b> - Caution! - If your router only has 32MB of RAM, you'll have to use swap.
		<li><b>Listening port</b> - Port used for torrent client. Make sure this port is not in use.
        <li><b>Listening RPC port</b> - Port used for Aria2 RPC. Make sure this port is not in use.
		<li><b>Use SSL to encrypt RPC traffic</b> - Use ssl to encrypt RPC traffic. A self signed certificate will automaticaly generated.
		<li><b>Allow remote access</b> - To open the Aria2 RPC port from the WAN side and allow the RPC to be accessed from the internet.
				you have to goto <a href="/advanced-firewall.asp">Firewall</a> settings to allow incoming traffic to RPC port of Aria2.
        <li><b>RPC access in lan is automatically enabled. RPC auth is also enabled, user name is "admin" and password is your password to login Tomato. you can change this option for security reason.</b>
	</ul>
</div>
</form>
</div>
</td></tr>
<tr><td id='footer' colspan=2>
 <form>
 <span id='footer-msg'></span>
 <input type='button' value='Save' id='save-button' onclick='save()'>
 <input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
 </form>
</div>
</td></tr>
</table>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
