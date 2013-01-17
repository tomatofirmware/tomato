<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato RAF Transmission GUI
	Copyright (C) 2007-2011 Shibby
	http://openlinksys.info
	For use with Tomato RAF Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] NAS: Klient BitTorrent</title>
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
//	<% nvram("bt_enable,bt_binary,bt_binary_custom,bt_custom,bt_port,bt_dir,bt_settings,bt_settings_custom,bt_incomplete,bt_rpc_enable,bt_rpc_wan,bt_auth,bt_login,bt_password,bt_port_gui,bt_dl_enable,bt_dl,bt_ul_enable,bt_ul,bt_peer_limit_global,bt_peer_limit_per_torrent,bt_ul_slot_per_torrent,bt_ratio_enable,bt_ratio,bt_ratio_idle_enable,bt_ratio_idle,bt_dht,bt_pex,bt_lpd,bt_utp,bt_blocklist,bt_blocklist_url,bt_sleep,bt_check,bt_check_time,bt_dl_queue_enable,bt_dl_queue_size,bt_ul_queue_enable,bt_ul_queue_size,bt_message"); %>

var btgui_link = '&nbsp;&nbsp;<a href="http://' + location.hostname +':<% nv('bt_port_gui'); %>" target="_blank"><i>[Kliknij, żeby otworzyć GUI Transmission]</i></a>';

function verifyFields(focused, quiet)
{
	var ok = 1;

	var a = E('_f_bt_enable').checked;
	var c = E('_f_bt_rpc_enable').checked;
	var d = E('_f_bt_dl_enable').checked;
	var e = E('_f_bt_ul_enable').checked;
	var g = E('_f_bt_ratio_enable').checked;
	var h = E('_f_bt_auth').checked;
	var i = E('_f_bt_blocklist').checked;
	var k = E('_f_bt_dl_queue_enable').checked;
	var l = E('_f_bt_ul_queue_enable').checked;
	var m = E('_f_bt_check').checked;
	var n = E('_f_bt_ratio_idle_enable').checked;

	E('_bt_custom').disabled = !a;
	E('_bt_binary').disabled = !a;
	E('_bt_dir').disabled = !a;
	E('_bt_port').disabled = !a;
	E('_bt_sleep').disabled = !a;
	E('_f_bt_incomplete').disabled = !a;
	E('_f_bt_check').disabled = !a;
	E('_bt_check_time').disabled = !a || !m;
	E('_bt_settings').disabled = !a;
	E('_f_bt_rpc_enable').disabled = !a;
	E('_bt_port_gui').disabled = !a || !c;
	E('_f_bt_auth').disabled = !a || !c;
	E('_bt_login').disabled = !a || !c || !h;
	E('_bt_password').disabled = !a || !c | !h;
	E('_f_bt_rpc_wan').disabled = !a || !c || !h;
	E('_f_bt_dl_enable').disabled = !a;
	E('_bt_dl').disabled = !a || !d;
	E('_f_bt_ul_enable').disabled = !a;
	E('_bt_ul').disabled = !a || !e;
	E('_bt_peer_limit_global').disabled = !a;
	E('_bt_peer_limit_per_torrent').disabled = !a;
	E('_bt_ul_slot_per_torrent').disabled = !a;
	E('_f_bt_ratio_enable').disabled = !a;
	E('_bt_ratio').disabled = !a || !g;
	E('_f_bt_ratio_idle_enable').disabled = !a;
	E('_bt_ratio_idle').disabled = !a || !n;
	E('_f_bt_dht').disabled = !a;
	E('_f_bt_pex').disabled = !a;
	E('_f_bt_lpd').disabled = !a;
	E('_f_bt_utp').disabled = !a;
	E('_f_bt_blocklist').disabled = !a;
	E('_bt_blocklist_url').disabled = !a || !i;
	E('_f_bt_dl_queue_enable').disabled = !a;
	E('_bt_dl_queue_size').disabled = !a || !k;
	E('_f_bt_ul_queue_enable').disabled = !a;
	E('_bt_ul_queue_size').disabled = !a || !l;
	E('_bt_message').disabled = !a;

	var o = (E('_bt_settings').value == 'custom');
	elem.display('_bt_settings_custom', o && a);

	var p = (E('_bt_binary').value == 'custom');
	elem.display('_bt_binary_custom', p && a);

	if (!v_length('_bt_custom', quiet, 0, 2048)) ok = 0;

	var s = E('_bt_custom');
	if (s.value.search(/"rpc-enable":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-enable". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"peer-port":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "peer-port". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"speed-limit-down-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "speed-limit-down-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"speed-limit-up-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "speed-limit-up-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"speed-limit-down":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "speed-limit-down". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"speed-limit-up":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "speed-limit-up". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"rpc-port":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-port". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"rpc-whitelist-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-whitelist-enabled". "Biała lista" jest wyłączona na stałe.', quiet);
		ok = 0; }

	if (s.value.search(/"rpc-username":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-username". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"rpc-password":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-password". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"download-dir":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "download-dir". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"incomplete-dir-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "incomplete-dir-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"incomplete-dir":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "incomplete-dir". Jeśli ta opcja jest włączona, wszystkie częściowo pobrane pliki będą zapisywane w katalogu "/download_dir/.incomplete".', quiet);
		ok = 0; }

	if (s.value.search(/"peer-limit-global":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "peer-limit-global". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"peer-limit-per-torrent":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "peer-limit-per-torrent". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"upload-slots-per-torrent":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "upload-slots-per-torrent". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"dht-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "dht-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"pex-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "pex-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"lpd-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "lpd-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"utp-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "utp-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"ratio-limit-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "ratio-limit-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"ratio-limit":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "ratio-limit". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"rpc-authentication-required":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "rpc-authentication-required". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"blocklist-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "blocklist-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"blocklist-url":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "blocklist-url". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"download-queue-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "download-queue-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"download-queue-size":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "download-queue-size". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"seed-queue-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "seed-queue-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"seed-queue-size":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "seed-queue-size". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"idle-seeding-limit-enabled":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "idle-seeding-limit-enabled". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"idle-seeding-limit":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "idle-seeding-limit". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	if (s.value.search(/"message-level":/) == 0)  {
		ferror.set(s, 'Nie można tu ustawić opcji "message-level". Możesz to to zrobić w GUI Tomato', quiet);
		ok = 0; }

	return ok;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');
  fom.bt_enable.value = E('_f_bt_enable').checked ? 1 : 0;
  fom.bt_incomplete.value = E('_f_bt_incomplete').checked ? 1 : 0;
  fom.bt_check.value = E('_f_bt_check').checked ? 1 : 0;
  fom.bt_rpc_enable.value = E('_f_bt_rpc_enable').checked ? 1 : 0;
  fom.bt_auth.value = E('_f_bt_auth').checked ? 1 : 0;
  fom.bt_rpc_wan.value = E('_f_bt_rpc_wan').checked ? 1 : 0;
  fom.bt_dl_enable.value = E('_f_bt_dl_enable').checked ? 1 : 0;
  fom.bt_ul_enable.value = E('_f_bt_ul_enable').checked ? 1 : 0;
  fom.bt_ratio_enable.value = E('_f_bt_ratio_enable').checked ? 1 : 0;
  fom.bt_ratio_idle_enable.value = E('_f_bt_ratio_idle_enable').checked ? 1 : 0;
  fom.bt_dht.value = E('_f_bt_dht').checked ? 1 : 0;
  fom.bt_pex.value = E('_f_bt_pex').checked ? 1 : 0;
  fom.bt_lpd.value = E('_f_bt_lpd').checked ? 1 : 0;
  fom.bt_utp.value = E('_f_bt_utp').checked ? 1 : 0;
  fom.bt_blocklist.value = E('_f_bt_blocklist').checked ? 1 : 0;
  fom.bt_dl_queue_enable.value = E('_f_bt_dl_queue_enable').checked ? 1 : 0;
  fom.bt_ul_queue_enable.value = E('_f_bt_ul_queue_enable').checked ? 1 : 0;

  if (fom.bt_enable.value == 0) {
  	fom._service.value = 'bittorrent-stop';
  }
  else {
  	fom._service.value = 'bittorrent-restart'; 
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
<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<div class='section-title'>Ustawienia podstawowe</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='nas-bittorrent.asp'>
<input type='hidden' name='_service' value='bittorrent-restart'>
<input type='hidden' name='bt_enable'>
<input type='hidden' name='bt_incomplete'>
<input type='hidden' name='bt_check'>
<input type='hidden' name='bt_rpc_enable'>
<input type='hidden' name='bt_auth'>
<input type='hidden' name='bt_rpc_wan'>
<input type='hidden' name='bt_dl_enable'>
<input type='hidden' name='bt_ul_enable'>
<input type='hidden' name='bt_blocklist'>
<input type='hidden' name='bt_ratio_enable'>
<input type='hidden' name='bt_ratio_idle_enable'>
<input type='hidden' name='bt_dht'>
<input type='hidden' name='bt_pex'>
<input type='hidden' name='bt_lpd'>
<input type='hidden' name='bt_utp'>
<input type='hidden' name='bt_dl_queue_enable'>
<input type='hidden' name='bt_ul_queue_enable'>

<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Włącz klienta torrenta', name: 'f_bt_enable', type: 'checkbox', value: nvram.bt_enable == '1', suffix: ' <small>*</small>' },
	{ title: 'Ścieżka do binariów Transmission', multi: [
		{ name: 'bt_binary', type: 'select', options: [
/* BBT-BEGIN */
			['internal','Wbudowany (/usr/bin)'],
/* BBT-END */
			['optware','Optware (/opt/bin)'],
			['custom','Własny'] ], value: nvram.bt_binary, suffix: ' <small>*</small> ' },
		{ name: 'bt_binary_custom', type: 'text', maxlen: 40, size: 40, value: nvram.bt_binary_custom }
	] },
	{ title: 'Utrzymuj aktywność', name: 'f_bt_check', type: 'checkbox', value: nvram.bt_check == '1', suffix: ' <small>*</small>' },
	{ title: 'Sprawdzaj aktywność co', indent: 2, name: 'bt_check_time', type: 'text', maxlen: 5, size: 7, value: nvram.bt_check_time, suffix: ' <small>w minutach (zakres: 1 - 55; domyślnie: 15)</small>' },
	{ title: 'Opóźnienie na starcie', name: 'bt_sleep', type: 'text', maxlen: 5, size: 7, value: nvram.bt_sleep, suffix: ' <small>w sekundach (zakres: 1 - 60; domyślnie: 10)</small>' },
	{ title: 'Port', name: 'bt_port', type: 'text', maxlen: 5, size: 7, value: nvram.bt_port, suffix: ' <small>*</small>' },
	{ title: 'Katalog pobierania', name: 'bt_dir', type: 'text', maxlen: 40, size: 40, value: nvram.bt_dir },
	{ title: 'Użyj .incomplete/', indent: 2, name: 'f_bt_incomplete', type: 'checkbox', value: nvram.bt_incomplete == '1' }
]);
</script>
	<ul>
		<li><b>Włącz klienta torrenta</b> - Ostrzeżenie! - Jeśli Twój router ma tylko 32MB RAM, musisz użyć swapu.
		<li><b>Ścieżka do binariów Transmission</b> Ścieżka do katalogu z plikiem transmission-daemon itd.
		<li><b>Utrzymuj aktywność</b> - Jeśli włączone, demon transmission będzie sprawdzany co określany czas i ponownie uruchamiany w razie nieoczekiwanego zamknięcia.
		<li><b>Port</b> - Port dla klienta torrent. Upewnij się że żadna inna aplikacja go nie używa.
	</ul>
</div>
<div class='section-title'>Dostęp przez GUI<script>W(btgui_link);</script></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Włącz GUI', name: 'f_bt_rpc_enable', type: 'checkbox', value: nvram.bt_rpc_enable == '1' },
	{ title: 'Port GUI', indent: 2, name: 'bt_port_gui', type: 'text', maxlen: 32, size: 5, value: nvram.bt_port_gui, suffix: ' <small>*</small>' },
	{ title: 'Wymagane uwierzytelnienie', name: 'f_bt_auth', type: 'checkbox', value: nvram.bt_auth == '1', suffix: ' <small>*</small>' },
	{ title: 'Użytkownik', indent: 2, name: 'bt_login', type: 'text', maxlen: 32, size: 15, value: nvram.bt_login },
	{ title: 'Hasło', indent: 2, name: 'bt_password', type: 'password', maxlen: 32, size: 15, value: nvram.bt_password },
	{ title: 'Pozwól na zdalny dostęp', name: 'f_bt_rpc_wan', type: 'checkbox', value: nvram.bt_rpc_wan == '1', suffix: ' <small>*</small>' }
]);
</script>
	<ul>
		<li><b>Port GUI</b> - Port dla GUI Transmission. Upewnij się że nie jest zajęty.
		<li><b>Wymagane uwierzytelnianie</b> - Logowanie się do GUI jest <b><i>wysoce wskazane</i> ze względów bezpieczeństwa i nie tylko.</b>.
		<li><b>Pozwól na zdalny dostęp</b> - Ta opcja otwiera dostęp do GUI Transmission od strony WAN i pozwala używać GUI przez Internet. Jeśli włączona, <b>bezwzględnie</b> należy włączyć również <i>Wymagane uwierzytelnianie</i>.
	</ul>
</div>
<div class='section-title'>Ograniczenia</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Limit pobierania', multi: [
		{ name: 'f_bt_dl_enable', type: 'checkbox', value: nvram.bt_dl_enable == '1', suffix: '  ' },
		{ name: 'bt_dl', type: 'text', maxlen: 10, size: 7, value: nvram.bt_dl, suffix: ' <small>kB/s</small>' } ] },
	{ title: 'Limit wysyłania', multi: [
		{ name: 'f_bt_ul_enable', type: 'checkbox', value: nvram.bt_ul_enable == '1', suffix: '  ' },
		{ name: 'bt_ul', type: 'text', maxlen: 10, size: 7, value: nvram.bt_ul, suffix: ' <small>kB/s</small>' } ] },
	{ title: 'Zakończ udostępnianie po uzyskaniu ratio', multi: [
		{ name: 'f_bt_ratio_enable', type: 'checkbox', value: nvram.bt_ratio_enable == '1', suffix: '  ' },
		{ name: 'bt_ratio', type: 'select', options: [['0.0000','0.0'],['0.1000','0.1'],['0.2000','0.2'],['0.5000','0.5'],['1.0000','1.0'],['1.5000','1.5'],['2.0000','2.0'],['2.5000','2.5'],['3.0000','3.0']], value: nvram.bt_ratio } ] },
	{ title: 'Zakończ udostępnianie, jeśli nikt nie pobiera przez ', multi: [
		{ name: 'f_bt_ratio_idle_enable', type: 'checkbox', value: nvram.bt_ratio_idle_enable == '1', suffix: '  ' },
		{ name: 'bt_ratio_idle', type: 'text', maxlen: 10, size: 7, value: nvram.bt_ratio_idle, suffix: ' <small>minut (zakres: 1 - 55; domyślnie: 30)</small>' } ] },
	{ title: 'Całkowity limit peerów', name: 'bt_peer_limit_global', type: 'text', maxlen: 10, size: 7, value: nvram.bt_peer_limit_global, suffix: ' <small>(zakres: 10 - 1000; domyślnie: 150)</small>' },
	{ title: 'Limit peerów na torrent', name: 'bt_peer_limit_per_torrent', type: 'text', maxlen: 10, size: 7, value: nvram.bt_peer_limit_per_torrent, suffix: ' <small>(zakres: 1 - 200; domyślnie: 30)</small>' },
	{ title: 'Ilość slotów wysyłania na torrent', name: 'bt_ul_slot_per_torrent', type: 'text', maxlen: 10, size: 7, value: nvram.bt_ul_slot_per_torrent, suffix: ' <small>(zakres: 1 - 50; domyślnie: 10)</small>' }
]);
</script>
</div>
<div class='section-title'>Kolejkowanie torrentów</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Kolejkowanie pobierania', multi: [
		{ name: 'f_bt_dl_queue_enable', type: 'checkbox', value: nvram.bt_dl_queue_enable == '1', suffix: '  ' },
		{ name: 'bt_dl_queue_size', type: 'text', maxlen: 5, size: 7, value: nvram.bt_dl_queue_size, suffix: ' <small>(zakres: 1 - 30; domyślnie: 5) *</small>' }
		] },
	{ title: 'Kolejkowanie wysyłania', multi: [
		{ name: 'f_bt_ul_queue_enable', type: 'checkbox', value: nvram.bt_ul_queue_enable == '1', suffix: '  ' },
		{ name: 'bt_ul_queue_size', type: 'text', maxlen: 5, size: 7, value: nvram.bt_ul_queue_size, suffix: ' <small>(zakres: 1 - 30; domyślnie: 5) *</small>' }
		] }
]);
</script>
	<ul>
		<li><b>Kolejkowanie pobierania</b> - Jeśli włączone to oznacza liczbę torrentów pobieranych jednocześnie (ważne dla regulacji obciążenia routera)
		<li><b>Kolejkowanie wysyłania</b> - Jeśli włączone to oznacza limit torrentów wysyłanych jednocześnie (ważne dla regulacji obciążenia routera)
	</ul>
</div>
<div class='section-title'>Ustawienia zaawansowane</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Znajdź więcej peerów używając', multi: [
		{ suffix: '&nbsp; DHT &nbsp;&nbsp;&nbsp;', name: 'f_bt_dht', type: 'checkbox', value: nvram.bt_dht == '1' },
		{ suffix: '&nbsp; PEX &nbsp;&nbsp;&nbsp;', name: 'f_bt_pex', type: 'checkbox', value: nvram.bt_pex == '1' },
		{ suffix: '&nbsp; LPD &nbsp;&nbsp;&nbsp;', name: 'f_bt_lpd', type: 'checkbox', value: nvram.bt_lpd == '1' },
		{ suffix: '&nbsp; uTP &nbsp;&nbsp;&nbsp;', name: 'f_bt_utp', type: 'checkbox', value: nvram.bt_utp == '1' }
		] },
	{ title: 'Poziom logowania', name: 'bt_message', type: 'select', options: [ ['0','Brak'], ['1','Błędy'], ['2','Info'], ['3','Debug'] ], value: nvram.bt_message, suffix: ' ' },
	{ title: 'Ścieżka zapisu ustawień', multi: [
		{ name: 'bt_settings', type: 'select', options: [
			['down_dir','W katalogu pobierania (zalecane)'],
/* JFFS2-BEGIN */
			['/jffs','JFFS2'],
/* JFFS2-END */
/* CIFS-BEGIN */
			['/cifs1','CIFS 1'],['/cifs2','CIFS 2'],
/* CIFS-END */
			['/tmp','RAM (tymczasowe)'], ['custom','Własna'] ], value: nvram.bt_settings, suffix: ' ' },
		{ name: 'bt_settings_custom', type: 'text', maxlen: 60, size: 40, value: nvram.bt_settings_custom }
		] },
	{ title: '"Czarna lista"', multi: [
		{ name: 'f_bt_blocklist', type: 'checkbox', value: nvram.bt_blocklist == '1', suffix: '  ' },
		{ name: 'bt_blocklist_url', type: 'text', maxlen: 80, size: 80, value: nvram.bt_blocklist_url }
		] },
	null,
	{ title: '<a href="https://trac.transmissionbt.com/wiki/EditConfigFiles" target="_new">Transmission</a><br>Własna konfiguracja', name: 'bt_custom', type: 'textarea', value: nvram.bt_custom }
]);
</script>
</div>
</form>
</div>
</td></tr>
<tr><td id='footer' colspan=2>
 <form>
 <span id='footer-msg'></span>
 <input type='button' value='Zapisz' id='save-button' onclick='save()'>
 <input type='button' value='Anuluj' id='cancel-button' onclick='javascript:reloadPage();'>
 </form>
</div>
</td></tr>
</table>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
