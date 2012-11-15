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
<title>[<% ident(); %>] Administracja: Debugowanie</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("debug_nocommit,debug_cprintf,debug_cprintf_file,console_loglevel,t_cafree,t_hidelr,debug_ddns,debug_norestart"); %>

function nvramCommit()
{
	fields.disableAll('_fom', 1);
	form.submitHidden('nvcommit.cgi', { '_nextpage': myName() });
}

function verifyFields(focused, quiet)
{
	return 1;
}

function save()
{
	var fom = E('_fom');
	fom.debug_nocommit.value = fom.f_debug_nocommit.checked ? 1 : 0;
	fom.debug_cprintf.value = fom.f_debug_cprintf.checked ? 1 : 0;
	fom.debug_cprintf_file.value = fom.f_debug_cprintf_file.checked ? 1 : 0;
	fom.t_cafree.value = fom.f_cafree.checked ? 1 : 0;
	fom.t_hidelr.value = fom.f_hidelr.checked ? 1 : 0;
	fom.debug_ddns.value = fom.f_debug_ddns.checked ? 1 : 0;

	var a = [];
	if (fom.f_nr_crond.checked) a.push('crond');
	if (fom.f_nr_dnsmasq.checked) a.push('dnsmasq');
/* LINUX26-BEGIN */
	if (fom.f_nr_hotplug2.checked) a.push('hotplug2');
/* LINUX26-END */
/* IPV6-BEGIN */
	if (fom.f_nr_radvd.checked) a.push('radvd');
/* IPV6-END */
	if (fom.f_nr_igmprt.checked) a.push('igmprt');
	fom.debug_norestart.value = a.join(',');

	form.submit(fom, 1);
}
</script>

</head>
<body>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='admin-misc.asp'>

<input type='hidden' name='debug_nocommit'>
<input type='hidden' name='debug_cprintf'>
<input type='hidden' name='debug_cprintf_file'>
<input type='hidden' name='debug_ddns'>
<input type='hidden' name='debug_norestart'>
<input type='hidden' name='t_cafree'>
<input type='hidden' name='t_hidelr'>

<div class='section-title'>Debugowanie</div>
<div class='section'>
<script type='text/javascript'>
a = [];
for (i = 1; i <= 8; ++i) a.push([i, i]);
createFieldTable('', [
	{ title: 'Nie wykonuj zapisów do NVRAM', name: 'f_debug_nocommit', type: 'checkbox', value: nvram.debug_nocommit != '0' },
	{ title: 'Włącz wysyłanie komunikatów cprintf na konsolę', name: 'f_debug_cprintf', type: 'checkbox', value: nvram.debug_cprintf != '0' },
	{ title: 'Włącz wysyłanie komunikatów cprintf do pliku /tmp/cprintf', name: 'f_debug_cprintf_file', type: 'checkbox', value: nvram.debug_cprintf_file != '0' },
	{ title: 'Włącz wysyłanie komunikatów DDNS do plików /tmp/mdu-*', name: 'f_debug_ddns', type: 'checkbox', value: nvram.debug_ddns != '0' },
	{ title: 'Zalicz pamięć podręczną jako pamięć dostępną', name: 'f_cafree', type: 'checkbox', value: nvram.t_cafree == '1' },
	{ title: 'Nie pokazuj połączeń między LAN a routerem', name: 'f_hidelr', type: 'checkbox', value: nvram.t_hidelr == '1' },
	{ title: 'Poziom logowania na konsoli', name: 'console_loglevel', type: 'select', options: a, value: fixInt(nvram.console_loglevel, 1, 8, 1) },
	{ title: 'Nie uruchamiaj ponownie następujących procesów po ich zakończeniu', multi: [
		{ name: 'f_nr_crond', type: 'checkbox', suffix: ' crond<br>', value: (nvram.debug_norestart.indexOf('crond') != -1) },
		{ name: 'f_nr_dnsmasq', type: 'checkbox', suffix: ' dnsmasq<br>', value: (nvram.debug_norestart.indexOf('dnsmasq') != -1) },
/* LINUX26-BEGIN */
		{ name: 'f_nr_hotplug2', type: 'checkbox', suffix: ' hotplug2<br>', value: (nvram.debug_norestart.indexOf('hotplug2') != -1) },
/* LINUX26-END */
/* IPV6-BEGIN */
		{ name: 'f_nr_radvd', type: 'checkbox', suffix: ' radvd<br>', value: (nvram.debug_norestart.indexOf('radvd') != -1) },
/* IPV6-END */
		{ name: 'f_nr_igmprt', type: 'checkbox', suffix: ' igmprt<br>', value: (nvram.debug_norestart.indexOf('igmprt') != -1) }
	] }
]);
</script>
<br><br>

&raquo; <a href='clearcookies.asp?_http_id=<% nv(http_id); %>'>Wyczyść Cookies</a><br>
&raquo; <a href='javascript:nvramCommit()'>Utrwal NVRAM</a><br>
&raquo; <a href='javascript:reboot()'>Uruchom ponownie</a><br>
&raquo; <a href='javascript:shutdown()'>Wyłącz</a><br>
<br><br>

&raquo; <a href='/cfe/cfe.bin?_http_id=<% nv(http_id); %>'>Pobierz CFE</a><br>
&raquo; <a href='/ipt/iptables.txt?_http_id=<% nv(http_id); %>'>Pobierz zrzut Iptables</a><br>
<!-- IPV6-BEGIN -->
&raquo; <a href='/ip6t/ip6tables.txt?_http_id=<% nv(http_id); %>'>Pobierz zrzut Ip6tables</a><br>
<!-- IPV6-END -->
&raquo; <a href='/logs/syslog.txt?_http_id=<% nv(http_id); %>'>Pobierz Logs</a><br>
&raquo; <a href='/nvram/nvram.txt?_http_id=<% nv(http_id); %>'>Pobierz zrzut NVRAM</a><br>
<br>

<div style='width:80%'>
<b>Ostrzeżenie</b>: Plik ze zrzutem zawartości NVRAM może zawierać informacje takie jak: klucze szyfrujące Wi-Fi
oraz nazwy użytkowników i hasła dostępu do routera, konta u ISP oraz DDNS. Przed udostępnieniem go osobom trzecim należy go przejrzeć  
i usunąć poufną zawartość.<br>
</div>

</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Zapisz' id='save-button' onclick='save()'>
	<input type='button' value='Anuluj' id='cancel-button' onclick='reloadPage();'>
</td></tr>
</table>
</form>
</body>
</html>
