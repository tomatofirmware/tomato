<html>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Pomiar poziomu szumów...</title>
<script language='javascript'>
function tick()
{
	t.innerHTML = tock + ' sek.' + ((tock == 1) ? '' : '');
	if (--tock >= 0) setTimeout(tick, 1000);
		else history.go(-1);
}
function init()
{
	t = document.getElementById('time');
	tock = 15;
	tick();
}
</script></head>
<body onload='init()' style='background-color:#fff' onclick='go()'>
<table style='width:100%;height:100%'>
<tr><td style='text-align:center;vertical-align:middle;font:13px sans-serif;width:100%;height:100%'>
<span style='font-size: 25px'>Pomiar poziomu szumów tła...</span>
<br>
Dostęp bezprzewodowy będzie chwilowo wyłączony przez <span id='time'>15 sekund</span>.
</td></tr>
</table>
</body></html>
