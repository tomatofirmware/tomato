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
<title>[<% ident(); %>] Nas:<% _("Shairport"); %></title>
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
//	<% nvram("shairport_enable,shairport_name,shairport_waitframes,shairport_dmabuffer,shairport_audiodevice"); %>

function verifyFields(focused, quiet)
{
	var ok = 1;

	var a = E('_f_shairport_enable').checked;
	E('_shairport_name').disabled = !a;
	E('_shairport_waitframes').disabled = !a;
	E('_shairport_dmabuffer').disabled = !a;
	E('_shairport_audiodevice').disabled = !a;
	return ok;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');
  fom.shairport_enable.value = E('_f_shairport_enable').checked ? 1 : 0;
  if (fom.shairport_enable.value == 0) {
  	fom._service.value = 'shairport-stop';
  }
  else {
  	fom._service.value = 'shairport-restart'; 
  }
	form.submit('_fom', 1);
}

function init()
{
}
</script>
</head>

<body onLoad="init()">
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
<div class='title'><% _("Tomato"); %></div>
<div class='version'><% _("Version"); %> <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<div class='section-title'><% _("Airplay Settings"); %></div>
<div class='section' id='config-section'>
<input type='hidden' name='_nextpage' value='nas-shairport.asp'>
<input type='hidden' name='_service' value='shairport-restart'>
<input type='hidden' name='shairport_enable'>

<script type='text/javascript'>
createFieldTable('', [
	{ title: '<% _("Enable Airplay service"); %>', name: 'f_shairport_enable', type: 'checkbox', value: nvram.shairport_enable == '1', suffix: ' <small>*</small>' },
	{ title: '<% _("Advertised Name"); %>', name: 'shairport_name', type: 'text', maxlen: 15, size: 20, value: nvram.shairport_name, suffix: ' <small>*</small>' },
    { title: '<% _("Wait Frames before Play"); %>', name: 'shairport_waitframes', type: 'text', maxlen: 4, size: 5, value: nvram.shairport_waitframes == null || nvram.shairport_waitframes == "" ? '0' : nvram.shairport_waitframes, suffix: ' <small>*</small>' },
    { title: '<% _("DMA Buffer"); %>', name: 'shairport_dmabuffer', type: 'text', maxlen: 5, size: 7, value: nvram.shairport_dmabuffer == null || nvram.shairport_dmabuffer == "" ? '2571':nvram.shairport_dmabuffer, suffix: ' <small>*</small>' },
    { title: '<% _("Audio Device"); %>', name: 'shairport_audiodevice', type: 'select', options: [['/dev/audio','/dev/audio'],
['/dev/dsp','/dev/dsp']], value: nvram.shairport_audiodevice, suffix: ' <small>*</small>' }
]);
</script>
<div>
<ul>
<br>
<br>
<b>NOTES</b><br>
<br>
<small>
* Wait Frames before Play: frames to buffer before play. (0-512)<br>
* DMA Buffer: SNDCTL_DSP_SETFRAGMENT to set on audio device 0 for default. see <a href="http://manuals.opensound.com/developer/SNDCTL_DSP_SETFRAGMENT.html">http://manuals.opensound.com/developer/SNDCTL_DSP_SETFRAGMENT.html</a>for detail information.<br>
</small>
</ul>
</div>
</div>
</td></tr>
<tr><td id='footer' colspan=2>
 <span id='footer-msg'></span>
 <input type='button' value='<% _("Save"); %>' id='save-button' onclick='save()'>
 <input type='button' value='<% _("Cancel"); %>' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
