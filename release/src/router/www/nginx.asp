<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	NGINX Web Server Management Control
	Ofer Chen (roadkill AT tomatoraf dot com)
	Vicente Soriano (victek AT tomatoraf dot com)
	Copyright (C) 2013 http://www.tomatoraf.com
	
	For use with Tomato Firmware only.
	No part of this file can be used or modified without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Web Server Menu</title>
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

<script type='text/javascript' src='debug.js'></script>
<script type='text/javascript'>

//	<% nvram("nginx_enable,nginx_port,nginx_fqdn,nginx_docroot,nginx_priority,nginx_custom"); %>

changed = 0;
nginxup = parseInt ('<% psup("nginx"); %>');

function toggle(service, isup)
{
	if (changed) {
		if (!confirm("Unsaved changes will be lost. Continue anyway?")) return;
	}
	E('_' + service + '_button').disabled = true;
	form.submitHidden('/service.cgi', {
		_redirect: 'nginx.asp',
		_sleep: ((service == 'nginxfp') && (!isup)) ? '10' : '5',
		_service: service + (isup ? '-stop' : '-start')
	});
}

function verifyFields(focused, quiet)
{
	var ok = 1;
	var a, b, c;
	var i;
	var vis = {
			_f_nginx_port : 1,
			_f_nginx_fqdn : 1,
			_f_nginx_docroot : 1,
			_f_nginx_priority : 1,
			_f_nginx_custom : 1,
	}

	if (!E('_f_nginx_enable').checked) {
		for (i in vis) {
			vis[i] = 2;
		}
	}

	for (a in vis) {
		b = E(a);
		c = vis[a];
		b.disabled = (c != 1);
		PR(b).style.display = c ? '' : 'none';
	}

	if (!v_port('_f_nginx_port', quiet))
	{
		ok = 0;
		W('port');
	}

	if ((!v_length('_f_nginx_docroot', quiet, 1, 255)) || (!v_length('_f_nginx_fqdn', quiet, 1, 255)) || (!v_length('_f_nginx_custom', quiet, 1, 4096)))
	{
	ok = 0;
	W('others');
	}

	changed |= ok;
	return ok;	
}

function save()
{
  	var fom = E('_fom');
	if (!verifyFields(null, false)) return;

	var en = fom.f_nginx_enable.checked;
	fom.nginx_enable.value = en ? 1 : 0;

	if (en) {
	fom.nginx_port.value = fom.f_nginx_port.value;
	fom.nginx_fqdn.value = fom.f_nginx_fqdn.value;
	fom.nginx_docroot.value = fom.f_nginx_docroot.value;
	fom.nginx_priority.value = fom.f_nginx_priority.value;
	fom.nginx_custom.value = fom.f_nginx_custom.value;
	}

	if (!en) {
	fom._service.value = 'enginex-stop';
	} else {
	fom._service.value = 'enginex-restart';
	}
	
	form.submit(fom, 1);
}
</script>
</head>
<body>
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
	W('NGINX is currently '+(!nginxup ? 'stopped' : 'running')+' ');
	W('<input type="button" value="' + (nginxup ? 'Stop' : 'Start') + ' Now" onclick="toggle(\'nginxfp\', nginxup)" id="_nginxfp_button">');
</script>
<br>
</div>
</div>

<div class='section-title'>WEB Server Management</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='nginx.asp'>
<input type='hidden' name='_service' value='enginex-restart'>
<input type='hidden' name='_nextwait' value='10'>
<input type='hidden' name='_reboot' value='0'>

<input type='hidden' name='nginx_enable'>
<input type='hidden' name='nginx_port'>
<input type='hidden' name='nginx_fqdn'>
<input type='hidden' name='nginx_docroot'>
<input type='hidden' name='nginx_priority'>
<input type='hidden' name='nginx_custom'>

<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable Server on Start', name: 'f_nginx_enable', type: 'checkbox', value: (nvram.nginx_enable != '0') },
	{ title: 'Web Server Port', name: 'f_nginx_port', type: 'text', maxlen: 5, size: 7, value: fixPort(nvram.nginx_port, 85), suffix: '<small> default: 85</small>' },
	{ title: 'Web Server Name', name: 'f_nginx_fqdn', type: 'text', maxlen: 255, size: 20, value: nvram.nginx_fqdn },
	{ title: 'Server Root Path', name: 'f_nginx_docroot', type: 'text', maxlen: 255, size: 40, value: nvram.nginx_docroot, suffix: '<span>&nbsp;/index.html / index.htm / index.php</span>' },
	{ title: 'Server Priority', name: 'f_nginx_priority', type: 'text', maxlen: 8, size:3, value: nvram.nginx_priority, suffix:'<small> Max. Perfor: -20, Min.Perfor: 19, default: 10</small>' },
	{ title: '<a href="http://wiki.nginx.org/Configuration" target="_new">NGINX</a><br>Custom configuration', name: 'f_nginx_custom', type: 'textarea', value: nvram.nginx_custom }
]);
</script>
</form>
</div>
<br>

<ul>
<span style='color:blue'>
<b>User Manual.</b><br>
</span>
<li><b>Note: This Feature is in Beta test, please test and report any bug you may find.</b>
<br> 
<br>
<li><b> Status Button:</b> Start-Stop service for test. Enable Web Server must be checked<br>
to change the settings, then save and Toggle the upper button.<br>
<li><b> Enable Server on Start:</b> To activate the Web Server tick and save this screen.<br>
<li><b> Web Server Port:</b> The Port used by the Web Server to be accessed.<br>
<li><b> Web Server Name:</b> Name that will appear on top of your Internet Browser.<br>
<li><b> Document Root Path:</b> The path in your router where documents are stored.<br>
<li><b> Examples:<br></b>
/tmp/mnt/HDD/www/ as you can find in USB mount path.<br>
<li><b> NGINX Custom Configuration:</b> You can add other values to nginx.conf to suit your needs.<br>
</span>
<span style='color:blue'>
<li><b> Server Priority:</b> Sets the service priority over other processes running on the router.<br><br>
The operating system kernel has priority -5.<br>
Never select a lower value than the kernel uses. Do not use the service test page to adjust the<br>
server performance, it's performance is lower than the definitive media where files will be <br>
located, i.e; USB Stick, Hard Drive or SSD.<br>
</span>
</ul>
<br>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
