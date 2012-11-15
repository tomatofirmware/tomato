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
<title>[<% ident(); %>] Administracja: Logowanie</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("log_remote,log_remoteip,log_remoteport,log_file,log_file_custom,log_file_path,log_limit,log_in,log_out,log_mark,log_events,log_wm,log_wmtype,log_wmip,log_wmdmax,log_wmsmax,log_file_size,log_file_keep"); %>

function verifyFields(focused, quiet)
{
	var a, b, c;

	a = E('_f_log_file').checked;
	b = E('_f_log_remote').checked;
	c = E('_f_log_file_custom').checked;

	a = !(a || b);
	E('_log_in').disabled = a;
	E('_log_out').disabled = a;
	E('_log_limit').disabled = a;
	E('_log_mark').disabled = a;
	E('_f_log_acre').disabled = a;
	E('_f_log_crond').disabled = a;
	E('_f_log_dhcpc').disabled = a;
	E('_f_log_ntp').disabled = a;
	E('_f_log_sched').disabled = a;

	elem.display(PR('_log_remoteip'), b);
	E('_log_remoteip').disabled = !b;
	E('_log_remoteport').disabled = !b;

	E('_f_log_file_custom').disabled = !E('_f_log_file').checked;
	E('_log_file_path').disabled = !c || !E('_f_log_file').checked;

	if (!a) {
		if (!v_range('_log_limit', quiet, 0, 2400)) return 0;
		if (!v_range('_log_mark', quiet, 0, 99999)) return 0;
		if (b) {
			c = E('_log_remoteip');
			if (!v_ip(c, 1) && !v_domain(c, 1)) {
				if (!quiet) ferror.show(c);
				return 0;
			}
			if (!v_port('_log_remoteport', quiet)) return 0;
		}
	}

	if (E('_f_log_file').checked) {
		E('_log_file_size').disabled = 0;
		if (!v_range('_log_file_size', quiet, 0, 99999)) return 0;
		if (parseInt(E('_log_file_size').value) > 0) {
			E('_log_file_keep').disabled = 0;
			if (!v_range('_log_file_keep', quiet, 0, 99)) return 0;
		} else {
			E('_log_file_keep').disabled = 1;
		}
	} else {
		E('_log_file_size').disabled = 1;
		E('_log_file_keep').disabled = 1;
	}

	a = E('_f_log_wm').checked;
	b = E('_log_wmtype').value != 0;
	E('_log_wmtype').disabled = !a;
	E('_f_log_wmip').disabled = !a;
	E('_log_wmdmax').disabled = !a;
	E('_log_wmsmax').disabled = !a;
	elem.display(PR('_f_log_wmip'), b);

	if (a) {
		if (b) {
			if (!_v_iptaddr('_f_log_wmip', quiet, 15, 1, 1)) return 0;
		}
		if (!v_range('_log_wmdmax', quiet, 0, 9999)) return 0;
		if (!v_range('_log_wmsmax', quiet, 0, 9999)) return 0;
	}

	return 1;
}

function save()
{
	var a, fom;

	if (!verifyFields(null, false)) return;

	fom = E('_fom');
	fom.log_remote.value = E('_f_log_remote').checked ? 1 : 0;
	fom.log_file.value = E('_f_log_file').checked ? 1 : 0;
	fom.log_file_custom.value = E('_f_log_file_custom').checked ? 1 : 0;

	a = [];
	if (E('_f_log_acre').checked) a.push('acre');
	if (E('_f_log_crond').checked) a.push('crond');
	if (E('_f_log_dhcpc').checked) a.push('dhcpc');
	if (E('_f_log_ntp').checked) a.push('ntp');
	if (E('_f_log_sched').checked) a.push('sched');
	fom.log_events.value = a.join(',');

	fom.log_wm.value = E('_f_log_wm').checked ? 1 : 0;
	fom.log_wmip.value = fom.f_log_wmip.value.split(/\s*,\s*/).join(',');

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

<input type='hidden' name='_nextpage' value='admin-log.asp'>
<input type='hidden' name='_service' value='logging-restart'>

<input type='hidden' name='log_remote'>
<input type='hidden' name='log_file'>
<input type='hidden' name='log_file_custom'>
<input type='hidden' name='log_events'>

<input type='hidden' name='log_wm'>
<input type='hidden' name='log_wmip'>

<script type='text/javascript'>
</script>

<div class='section-title'>Logi systemowe</div>
<div class='section'>
<script type='text/javascript'>

/* REMOVE-BEGIN
// adjust (>=1.22)
nvram.log_mark *= 1;
if (nvram.log_mark >= 120) nvram.log_mark = 120;
	else if (nvram.log_mark >= 60) nvram.log_mark = 60;
	else if (nvram.log_mark > 0) nvram.log_mark = 30;
	else nvram.log_mark = 0;
REMOVE-END */

createFieldTable('', [
	{ title: 'Loguj wewnętrznie', name: 'f_log_file', type: 'checkbox', value: nvram.log_file == 1 },
	{ title: 'Rotuj plik gdy osiągnie rozmiar', name: 'log_file_size', type: 'text', maxlen: 5, size: 6, value: nvram.log_file_size || 50, suffix: ' <small>KB</small>' },
	{ title: 'Ilość zrotowanym plików', name: 'log_file_keep', type: 'text', maxlen: 2, size: 3, value: nvram.log_file_keep || 1 },
	{ title: 'Własna ścieżka do pliku z logami', multi: [
		{ name: 'f_log_file_custom', type: 'checkbox', value: nvram.log_file_custom == 1, suffix: '  ' },
		{ name: 'log_file_path', type: 'text', maxlen: 50, size: 30, value: nvram.log_file_path, suffix: ' <small>(upewnij się że katalog istnieje i jest zapisywalny)</small>' }
		] },
	{ title: 'Loguj na zdalny system', name: 'f_log_remote', type: 'checkbox', value: nvram.log_remote == 1 },
	{ title: 'Nazwa hosta lub adres IP / Port', indent: 2, multi: [
		{ name: 'log_remoteip', type: 'text', maxlen: 15, size: 17, value: nvram.log_remoteip, suffix: ':' },
		{ name: 'log_remoteport', type: 'text', maxlen: 5, size: 7, value: nvram.log_remoteport } ]},
	{ title: 'Generuj znacznik', name: 'log_mark', type: 'select', options: [[0,'Wyłączone'],[30,'Co 30 minut'],[60,'Co godzinę'],[120,'Co 2 godziny'],[360,'Co 6 godzin'],[720,'Co 12 godzin'],[1440,'Codziennie'],[10080,'Co 7 dni']], value: nvram.log_mark },
	{ title: 'Logowane zdarzenia', text: '<small>(niektóre zmiany będą aktywne dopiero po restarcie)</small>' },
		{ title: 'Ograniczenia dostępu', indent: 2, name: 'f_log_acre', type: 'checkbox', value: (nvram.log_events.indexOf('acre') != -1) },
		{ title: 'Cron', indent: 2, name: 'f_log_crond', type: 'checkbox', value: (nvram.log_events.indexOf('crond') != -1) },
		{ title: 'Klienci DHCP', indent: 2, name: 'f_log_dhcpc', type: 'checkbox', value: (nvram.log_events.indexOf('dhcpc') != -1) },
		{ title: 'NTP', indent: 2, name: 'f_log_ntp', type: 'checkbox', value: (nvram.log_events.indexOf('ntp') != -1) },
		{ title: 'Harmonogram', indent: 2, name: 'f_log_sched', type: 'checkbox', value: (nvram.log_events.indexOf('sched') != -1) },
	{ title: 'Logowanie połączeń' },
		{ title: 'Wchodzących', indent: 2, name: 'log_in', type: 'select', options: [[0,'Wyłączone (zalecane)'],[1,'Jeśli zablokowane przez Firewall'],[2,'Jeśli przepuszczone przez Firewall'],[3,'Oba']], value: nvram.log_in },
		{ title: 'Wychodzących', indent: 2, name: 'log_out', type: 'select', options: [[0,'Wyłączone (zalecane)'],[1,'Jeśli zablokowane przez Firewall'],[2,'Jeśli przepuszczone przez Firewall'],[3,'Oba']], value: nvram.log_out },
		{ title: 'Limit', indent: 2, name: 'log_limit', type: 'text', maxlen: 4, size: 5, value: nvram.log_limit, suffix: ' <small>(zdarzeń na minutę / 0 - brak limitu)</small>' }
]);
</script>
</div>

<div class='section-title'>Monitor sieci Web</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Użyj monitora sieci Web', name: 'f_log_wm', type: 'checkbox', value: nvram.log_wm == 1 },
	{ title: 'Monitoruj', name: 'log_wmtype', type: 'select', options: [[0,'Wszystkie urządzenia'],[1,'Następujące...'],[2,'Wszystkie, z wyjątkiem...']], value: nvram.log_wmtype },
		{ title: 'Adres(y) IP', indent: 2,  name: 'f_log_wmip', type: 'text', maxlen: 512, size: 64, value: nvram.log_wmip,
		  suffix: '<br><small>(np: "1.1.1.1", "1.1.1.0/24" lub "1.1.1.1 - 2.2.2.2")</small>' },
	{ title: 'Ilość pozycji do zapamiętania' },
		{ title: 'Domeny', indent: 2,  name: 'log_wmdmax', type: 'text', maxlen: 4, size: 6, value: nvram.log_wmdmax, suffix: ' <small>(0 żeby wyłączyć)</small>' },
		{ title: 'Wyszukiwania', indent: 2, name: 'log_wmsmax', type: 'text', maxlen: 4, size: 6, value: nvram.log_wmsmax, suffix: ' <small>(0 żeby wyłączyć)</small>' }
]);
</script>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Zapisz' id='save-button' onclick='save()'>
	<input type='button' value='Anuluj' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>

