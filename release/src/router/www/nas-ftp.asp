<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	FTP Server - !!TB

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] NAS: Serwer FTP</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->
<style tyle='text/css'>
#aft-grid {
	width: 99%;
}
#aft-grid .co1 {
	width: 20%;
}
#aft-grid .co2 {
	width: 20%;
}
#aft-grid .co3 {
	width: 15%;
}
#aft-grid .co4 {
	width: 45%;
}
</style>

<style type='text/css'>
textarea {
	width: 98%;
	height: 5em;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("ftp_enable,ftp_super,ftp_anonymous,ftp_dirlist,ftp_port,ftp_max,ftp_ipmax,ftp_staytimeout,ftp_rate,ftp_anonrate,ftp_anonroot,ftp_pubroot,ftp_pvtroot,ftp_custom,ftp_users,ftp_sip,ftp_limit,log_ftp"); %>

ftplimit = nvram.ftp_limit.split(',');
if (ftplimit.length != 3) ftplimit = [0,3,60];

var aftg = new TomatoGrid();

aftg.exist = function(f, v)
{
	var data = this.getAllData();
	for (var i = 0; i < data.length; ++i) {
		if (data[i][f] == v) return true;
	}
	return false;
}

aftg.existName = function(name)
{
	return this.exist(0, name);
}

aftg.sortCompare = function(a, b) {
	var da = a.getRowData();
	var db = b.getRowData();
	var r = cmpText(da[this.sortColumn], db[this.sortColumn]);
	return this.sortAscending ? r : -r;
}

aftg.verifyFields = function(row, quiet)
{
	var f, s;
	f = fields.getAll(row);

	ferror.clear(f[0]);
	ferror.clear(f[1]);
	ferror.clear(f[3]);

	if (!v_length(f[0], quiet, 1)) return 0;

	s = f[0].value.trim().replace(/\s+/g, ' ');
	if (s.length > 0) {
		if (s.search(/^[a-zA-Z0-9_\-]+$/) == -1) {
			ferror.set(f[0], 'Błędna nazwa użytkownika. Tylko znaki "A-Z 0-9 - _" są dopuszczalne.', quiet);
			return 0;
		}
		if (this.existName(s)) {
			ferror.set(f[0], 'Zdublowana nazwa użytkownika.', quiet);
			return 0;
		}
		if (s == 'root' || s == 'admin') {
			ferror.set(f[0], 'Nazwy użytkownika "root" i "admin" nie są dozwolone.', quiet);
			return 0;
		}
		f[0].value = s;
	}

	if (!v_length(f[1], quiet, 1)) return 0;
	if (!v_nodelim(f[1], quiet, 'Hasło', 1)) return 0;
	if (f[2].value == 'Prywatne') {
		f[3].value = '';
		f[3].disabled = true;
	}
	else {
		f[3].disabled = false;
		if (!v_nodelim(f[3], quiet, 'Katalog główny', 1) || !v_path(f[3], quiet, 0)) return 0;
	}

	return 1;
}

aftg.resetNewEditor = function() {
	var f;

	f = fields.getAll(this.newEditor);
	ferror.clearAll(f);

	f[0].value = '';
	f[1].value = '';
	f[2].selectedIndex = 0;
	f[3].value = '';
}

aftg.setup = function()
{
	this.init('aft-grid', 'sort', 50, [
		{ type: 'text', maxlen: 50 },
		{ type: 'password', maxlen: 50, peekaboo: 1 },
		{ type: 'select', options: [['Odczyt/Zapis', 'Odczyt/Zapis'],['Odczyt', 'Odczyt'],['Widok', 'Widok'],['Prywatne', 'Prywatne']] },
		{ type: 'text', maxlen: 128 }
	]);
	this.headerSet(['Użytkownik', 'Hasło', 'Uprawnienia', 'Katalog główny*']);

	var s = nvram.ftp_users.split('>');
	for (var i = 0; i < s.length; ++i) {
		var t = s[i].split('<');
		if (t.length == 3) {
			t.push('');
		}
		if (t.length == 4) {
			this.insertData(-1, t);
		}
	}

	this.showNewEditor();
	this.resetNewEditor();
}

function verifyFields(focused, quiet)
{
	var a, b;
	var ok = 1;

	a = E('_ftp_enable').value;	
	b = E('_ftp_port');
	elem.display(PR(b), (a != 0));
	b = E('_f_ftp_sip');
	elem.display(PR(b), (a == 1));

	E('_f_limit').disabled = (a != 1);
	b = E('_f_limit').checked;
	elem.display(PR('_f_limit_hit'), PR('_f_limit_sec'), (a == 1 && b));
	E('_ftp_anonymous').disabled = (a == 0);
	E('_f_ftp_super').disabled = (a == 0);
	E('_f_log_ftp').disabled = (a == 0);
	E('_ftp_pubroot').disabled = (a == 0);
	E('_ftp_pvtroot').disabled = (a == 0);
	E('_ftp_anonroot').disabled = (a == 0);
	E('_ftp_dirlist').disabled = (a == 0);
	E('_ftp_max').disabled = (a == 0);
	E('_ftp_ipmax').disabled = (a == 0);
	E('_ftp_rate').disabled = (a == 0);
	E('_ftp_anonrate').disabled = (a == 0);
	E('_ftp_staytimeout').disabled = (a == 0);
	E('_ftp_custom').disabled = (a == 0);

	if (a != 0) {
		if (!v_port('_ftp_port', quiet || !ok)) ok = 0;
		if (!v_range('_ftp_max', quiet || !ok, 0, 12)) ok = 0;
		if (!v_range('_ftp_ipmax', quiet || !ok, 0, 12)) ok = 0;
		if (!v_range('_ftp_rate', quiet || !ok, 0, 99999)) ok = 0;
		if (!v_range('_ftp_anonrate', quiet || !ok, 0, 99999)) ok = 0;
		if (!v_range('_ftp_staytimeout', quiet || !ok, 0, 65535)) ok = 0;
		if (!v_length('_ftp_custom', quiet || !ok, 0, 2048)) ok = 0;
		if (!v_path('_ftp_pubroot', quiet || !ok, 0)) ok = 0;
		if (!v_path('_ftp_pvtroot', quiet || !ok, 0)) ok = 0;
		if (!v_path('_ftp_anonroot', quiet || !ok, 0)) ok = 0;
		if (a == 1 && b) {
			if (!v_range('_f_limit_hit', quiet || !ok, 1, 100)) ok = 0;
			if (!v_range('_f_limit_sec', quiet || !ok, 3, 3600)) ok = 0;
		}
	}

	if (a == 1) {
		b = E('_f_ftp_sip');
		if ((b.value.length) && (!_v_iptaddr(b, quiet || !ok, 15, 1, 1))) ok = 0;
		else ferror.clear(b);
	}

	return ok;
}

function save()
{
	if (aftg.isEditing()) return;
	if (!verifyFields(null, 0)) return;

	var fom = E('_fom');

	var data = aftg.getAllData();
	var r = [];
	for (var i = 0; i < data.length; ++i) r.push(data[i].join('<'));
	fom.ftp_users.value = r.join('>');

	fom.ftp_sip.value = fom.f_ftp_sip.value.split(/\s*,\s*/).join(',');
	fom.ftp_super.value = E('_f_ftp_super').checked ? 1 : 0;
	fom.log_ftp.value = E('_f_log_ftp').checked ? 1 : 0;

	fom.ftp_limit.value = (E('_f_limit').checked ? 1 : 0) +
		',' + E('_f_limit_hit').value + ',' + E('_f_limit_sec').value;

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

<input type='hidden' name='_nextpage' value='nas-ftp.asp'>
<input type='hidden' name='_service' value='ftpd-restart'>

<input type='hidden' name='ftp_super'>
<input type='hidden' name='log_ftp'>
<input type='hidden' name='ftp_users'>
<input type='hidden' name='ftp_sip'>
<input type='hidden' name='ftp_limit'>

<div class='section-title'>Konfiguracja serwera FTP</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Włącz serwer FTP', name: 'ftp_enable', type: 'select',
		options: [['0', 'Nie'],['1', 'Tak, WAN i LAN'],['2', 'Tak, tylko LAN']],
		value: nvram.ftp_enable },
	{ title: 'Port FTP', indent: 2, name: 'ftp_port', type: 'text', maxlen: 5, size: 7, value: fixPort(nvram.ftp_port, 21) },
	{ title: 'Dozwolone zdalne<br>adres(y)', indent: 2, name: 'f_ftp_sip', type: 'text', maxlen: 512, size: 64, value: nvram.ftp_sip,
		suffix: '<br><small>(opcjonalnie; ex: "1.1.1.1", "1.1.1.0/24", "1.1.1.1 - 2.2.2.2" lub "moja.domena.com")</small>' },
	{ title: 'Dostęp anonimowy', name: 'ftp_anonymous', type: 'select',
		options: [['0', 'Wyłączony'],['1', 'Odczyt/Zapis'],['2', 'Tylko odczyt'],['3', 'Tylko zapis']],
		value: nvram.ftp_anonymous },
	{ title: 'Zezwól na logowanie Admina*', name: 'f_ftp_super', type: 'checkbox',
		suffix: ' <small>Pozwalaj na logowanie się z konta administracyjnego.</small>',
		value: nvram.ftp_super == 1 },
	{ title: 'Loguj ruch FTP', name: 'f_log_ftp', type: 'checkbox',
		value: nvram.log_ftp == 1 }
]);
</script>
<small><br>*&nbsp;Nie używaj tej opcji jeśli serwer FTP jest dostępny od strony WAN. TO POZWALA NA PEŁNY DOSTĘP DO SYSTEMU PLIKÓW NA ROUTERZE!</small>
</div>

<div class='section-title'>Katalogi</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Główny katalog anonimowy*', name: 'ftp_anonroot', type: 'text', maxlen: 256, size: 32, 
		suffix: ' <small>(dla połączeń anonimowych)</small>',
		value: nvram.ftp_anonroot },
	{ title: 'Główny katalog publiczny*', name: 'ftp_pubroot', type: 'text', maxlen: 256, size: 32,
		suffix: ' <small>(dla zalogowanych użytkowników, jeśli nie mają przypisanego własnego)</small>',
		value: nvram.ftp_pubroot },
	{ title: 'Główny katalog prywatny**', name: 'ftp_pvtroot', type: 'text', maxlen: 256, size: 32,
		suffix: ' <small>(dla zalogowanych użytkowników w trybie prywatnym)</small>',
		value: nvram.ftp_pvtroot },
	{ title: 'Przeglądanie katalogów', name: 'ftp_dirlist', type: 'select',
		options: [['0', 'Włączone'],['1', 'Wyłączone'],['2', 'Wyłączone dla Anonimowych']],
		suffix: ' <small>(zawsze dostępne dla Admina)</small>',
		value: nvram.ftp_dirlist }
]);
</script>
<small>
<br>*&nbsp;&nbsp;W przypadku niezdefiniowania katalogu głównego, zostanie użyty katalog /mnt. .
<br>**&nbsp; W trybie "Prywatnym" katalogiem głównym uzytkownika jest katalog z nazwą użytkownika pod "Głównym katalogiem prywatnym".
</small>
</div>

<div class='section-title'>Ograniczenia</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Maksymalna liczba zalogowanych użytkowników', name: 'ftp_max', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>(0 - bez limitu)</small>',
		value: nvram.ftp_max },
	{ title: 'Maksymalna ilość połączeń z tego samego IP', name: 'ftp_ipmax', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>(0 - bez limitu)</small>',
		value: nvram.ftp_ipmax },
	{ title: 'Maksymalna przepustowość dla anonimowych użytkowników', name: 'ftp_anonrate', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>KBajtów/sek (0 - bez limitu)</small>',
		value: nvram.ftp_anonrate },
	{ title: 'Maksymalna przepustowość dla uwierzytelnionych użytkowników', name: 'ftp_rate', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>KBajtów/sek (0 - bez limitu)</small>',
		value: nvram.ftp_rate },
	{ title: 'Rozłącz jeśli bezczynny', name: 'ftp_staytimeout', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>sekund (0 - nie rozłączaj)</small>',
		value: nvram.ftp_staytimeout },
	{ title: 'Limit prób połączeń', name: 'f_limit', type: 'checkbox',
		value: ftplimit[0] != 0 },
	{ title: '', indent: 2, multi: [
		{ name: 'f_limit_hit', type: 'text', maxlen: 4, size: 6, suffix: '&nbsp; <small>co</small> &nbsp;', value: ftplimit[1] },
		{ name: 'f_limit_sec', type: 'text', maxlen: 4, size: 6, suffix: '&nbsp; <small>sekund</small>', value: ftplimit[2] }
	] }
]);
</script>
</div>

<div class='section-title'>Własna konfiguracja</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: '<a href="http://vsftpd.beasts.org/vsftpd_conf.html" target="_new">Vsftpd</a><br>Własna konfiguracja', name: 'ftp_custom', type: 'textarea', value: nvram.ftp_custom }
]);
</script>
</div>

<div class='section-title'>Konta użytkowników</div>
<div class='section'>
	<table class='tomato-grid' cellspacing=1 id='aft-grid'></table>
	<script type='text/javascript'>aftg.setup();</script>
<small>
*&nbsp;&nbsp;Jeśli nie wskazano głównego katalogu dla użytkownika to używany jest "Główny Katalog Publiczny".
</small>
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
