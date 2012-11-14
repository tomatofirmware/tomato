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
<title>[<% ident(); %>] Administracja: Monitoring IP Traffic</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
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

//	<% nvram("cstats_enable,cstats_path,cstats_stime,cstats_offset,cstats_exclude,cstats_include,cstats_sshut,et0macaddr,cifs1,cifs2,jffs2_on,cstats_bak,cstats_all,cstats_labels"); %>

function fix(name)
{
	var i;
	if (((i = name.lastIndexOf('/')) > 0) || ((i = name.lastIndexOf('\\')) > 0))
		name = name.substring(i + 1, name.length);
	return name;
}

function backupNameChanged()
{
	if (location.href.match(/^(http.+?\/.+\/)/)) {
		E('backup-link').href = RegExp.$1 + 'ipt/' + fix(E('backup-name').value) + '.gz?_http_id=' + nvram.http_id;
	}
}

function backupButton()
{
	var name;

	name = fix(E('backup-name').value);
	if (name.length <= 1) {
		alert('Błędna nazwa pliku');
		return;
	}
	location.href = 'ipt/' + name + '.gz?_http_id=' + nvram.http_id;
}

function restoreButton()
{
	var fom;
	var name;
	var i;

	name = fix(E('restore-name').value);
	name = name.toLowerCase();
	if ((name.length <= 3) || (name.substring(name.length - 3, name.length).toLowerCase() != '.gz')) {
		alert('Błedna nazwa pliku. Oczekiwany plik z rozszerzeniem ".gz".');
		return;
	}
	if (!confirm('Restore data from ' + name + '?')) return;

	E('restore-button').disabled = 1;
	fields.disableAll(E('config-section'), 1);
	fields.disableAll(E('backup-section'), 1);
	fields.disableAll(E('footer'), 1);

	E('restore-form').submit();
}

function getPath()
{
	var s = E('_f_loc').value;
	return (s == '*user') ? E('_f_user').value : s;
}

function verifyFields(focused, quiet)
{
	var b, v;
	var path;
	var eLoc, eUser, eTime, eOfs;
	var bak;
	var eInc, eExc, eAll, eBak, eLab;

	eLoc = E('_f_loc');
	eUser = E('_f_user');
	eTime = E('_cstats_stime');
	eOfs = E('_cstats_offset');

	eInc = E('_cstats_include');
	eExc = E('_cstats_exclude');
	eAll = E('_f_all');
	eBak = E('_f_bak');

	eLab = E('_cstats_labels');

	b = !E('_f_cstats_enable').checked;
	eLoc.disabled = b;
	eUser.disabled = b;
	eTime.disabled = b;
	eOfs.disabled = b;
	eInc.disabled = b;
	eExc.disabled = b;
	eAll.disabled = b;
	eBak.disabled = b;
	eLab.disabled = b;
	E('_f_new').disabled = b;
	E('_f_sshut').disabled = b;
	E('backup-button').disabled = b;
	E('backup-name').disabled = b;
	E('restore-button').disabled = b;
	E('restore-name').disabled = b;
	ferror.clear(eLoc);
	ferror.clear(eUser);
	ferror.clear(eOfs);
	if (b) return 1;

	eInc.disabled = eAll.checked;

	path = getPath();
	E('newmsg').style.visibility = ((nvram.cstats_path != path) && (path != '*nvram') && (path != '')) ? 'visible' : 'hidden';

	bak = 0;
	v = eLoc.value;
	b = (v == '*user');
	elem.display(eUser, b);
	if (b) {
		if (!v_length(eUser, quiet, 2)) return 0;
		if (path.substr(0, 1) != '/') {
			ferror.set(eUser, 'Proszę rozpocząć od katalogu / root.', quiet);
			return 0;
		}
	}
	else if (v == '/jffs/') {
		if (nvram.jffs2_on != '1') {
			ferror.set(eLoc, 'JFFS2 nie jest włączony.', quiet);
			return 0;
		}
	}
	else if (v.match(/^\/cifs(1|2)\/$/)) {
		if (nvram['cifs' + RegExp.$1].substr(0, 1) != '1') {
			ferror.set(eLoc, 'CIFS #' + RegExp.$1 + ' nie jest włączony.', quiet);
			return 0;
		}
	}
	else {
		bak = 1;
	}

	E('_f_bak').disabled = bak;

	return v_range(eOfs, quiet, 1, 31);
}

function save()
{
	var fom, path, en, e, aj;

	if (!verifyFields(null, false)) return;

	aj = 1;
	en = E('_f_cstats_enable').checked;
	fom = E('_fom');
	fom._service.value = 'cstats-restart';
	if (en) {
		path = getPath();
		if (((E('_cstats_stime').value * 1) <= 48) &&
			((path == '*nvram') || (path == '/jffs/'))) {
			if (!confirm('Częste zapisywanie do NVRAM lub JFFS2 nie jest zalecane ze względu na trwałość pamięci flash. Czy mimo wszystko kontynuować?')) return;
		}
		if ((nvram.cstats_path != path) && (fom.cstats_path.value != path) && (path != '') && (path != '*nvram') &&
			(path.substr(path.length - 1, 1) != '/')) {
			if (!confirm('Notka: ' + path + ' będzie traktowany jak plik. Jeśli to katalog, proszę użyć znaku ukośnika /. Czy kontynuować?')) return;
		}
		fom.cstats_path.value = path;

		if (E('_f_new').checked) {
			fom._service.value = 'cstatsnew-restart';
			aj = 0;
		}
	}

	fom.cstats_path.disabled = !en;
	fom.cstats_enable.value = en ? 1 : 0;
	fom.cstats_sshut.value = E('_f_sshut').checked ? 1 : 0;
	fom.cstats_bak.value = E('_f_bak').checked ? 1 : 0;
	fom.cstats_all.value = E('_f_all').checked ? 1 : 0;

	e = E('_cstats_exclude');
	e.value = e.value.replace(/\s+/g, ',').replace(/,+/g, ',');

	e = E('_cstats_include');
	e.value = e.value.replace(/\s+/g, ',').replace(/,+/g, ',');

	fields.disableAll(E('backup-section'), 1);
	fields.disableAll(E('restore-section'), 1);
	form.submit(fom, aj);
	if (en) {
		fields.disableAll(E('backup-section'), 0);
		fields.disableAll(E('restore-section'), 0);
	}
}

function init()
{
	backupNameChanged();
}
</script>

</head>
<body onload="init()">
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<div class='section-title'>Monitorowanie IP Traffic</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='admin-iptraffic.asp'>
<input type='hidden' name='_service' value='cstats-restart'>
<input type='hidden' name='cstats_enable'>
<input type='hidden' name='cstats_path'>
<input type='hidden' name='cstats_sshut'>
<input type='hidden' name='cstats_bak'>
<input type='hidden' name='cstats_all'>

<script type='text/javascript'>
switch (nvram.cstats_path) {
case '':
case '*nvram':
case '/jffs/':
case '/cifs1/':
case '/cifs2/':
	loc = nvram.cstats_path;
	break;
default:
	loc = '*user';
	break;
}
createFieldTable('', [
	{ title: 'Włączony', name: 'f_cstats_enable', type: 'checkbox', value: nvram.cstats_enable == '1' },
	{ title: 'Miejsce zapisu historii', multi: [
/* REMOVE-BEGIN
//		{ name: 'f_loc', type: 'select', options: [['','RAM (Temporary)'],['*nvram','NVRAM'],['/jffs/','JFFS2'],['/cifs1/','CIFS 1'],['/cifs2/','CIFS 2'],['*user','Custom Path']], value: loc },
REMOVE-END */
		{ name: 'f_loc', type: 'select', options: [['','RAM (tymczasowo)'],['/jffs/','JFFS2'],['/cifs1/','CIFS 1'],['/cifs2/','CIFS 2'],['*user','Ścieżka użytkownika']], value: loc },
		{ name: 'f_user', type: 'text', maxlen: 48, size: 50, value: nvram.cstats_path }
	] },
	{ title: 'Częstotliwość zapisu', indent: 2, name: 'cstats_stime', type: 'select', value: nvram.cstats_stime, options: [
		[1,'Co godzinę'],[2,'Co 2 godziny'],[3,'Co 3 godziny'],[4,'Co 4 godziny'],[5,'Co 5 godzin'],[6,'Co 6 godzin'],
		[9,'Co 9 godzin'],[12,'Co 12 godzin'],[24,'Co 24 godziny'],[48,'Co 2 dni'],[72,'Co 3 dni'],[96,'Co 4 dni'],
		[120,'Co 5 dni'],[144,'Co 6 dni'],[168,'Co tydzień']] },
	{ title: 'Zapisz przy zamknięciu', indent: 2, name: 'f_sshut', type: 'checkbox', value: nvram.cstats_sshut == '1' },
	{ title: 'Utwórz nowy plik<br><small>(zeruje dane)</small>', indent: 2, name: 'f_new', type: 'checkbox', value: 0,
		suffix: ' &nbsp; <b id="newmsg" style="visibility:hidden"><small>(zaznaczyć w przypadku nowego pliku)</small></b>' },
	{ title: 'Twórz kopie', indent: 2, name: 'f_bak', type: 'checkbox', value: nvram.cstats_bak == '1' },
	{ title: 'Pierwszy dzień miesiąca', name: 'cstats_offset', type: 'text', value: nvram.cstats_offset, maxlen: 2, size: 4 },
	{ title: 'Wyklucz adresy IP', name: 'cstats_exclude', type: 'text', value: nvram.cstats_exclude, maxlen: 512, size: 50, suffix: '&nbsp;<small>(lista oddzielona przecinkami)</small>' },
	{ title: 'Uwzględnij adresy IP', name: 'cstats_include', type: 'text', value: nvram.cstats_include, maxlen: 2048, size: 50, suffix: '&nbsp;<small>(lista oddzielona przecinkami)</small>' },
	{ title: 'Włącz Auto-wykrywanie', name: 'f_all', type: 'checkbox', value: nvram.cstats_all == '1', suffix: '&nbsp;<small>(automatycznie uwzględnia nowe adresy IP w monitoringu gdy tylko wykryje ruch)</small>' },
	{ title: 'Etykiety na wykresach', name: 'cstats_labels', type: 'select', value: nvram.cstats_stime, options: [[0,'Pokazuj znane nazwy hostów i ich IP'],[1,'Preferuj pokazywanie znanych nazw hostów, gdy brak pokaż IP'],[2,'Pokazuj tylko adresy IP']], value: nvram.cstats_labels }
]);
</script>
</form>
</div>

<br>

<div class='section-title'>Kopia zapasowa</div>
<div class='section' id='backup-section'>
	<form>
	<script type='text/javascript'>
	W("<input type='text' size='40' maxlength='64' id='backup-name' name='backup_name' onchange='backupNameChanged()' value='tomato_cstats_" + nvram.et0macaddr.replace(/:/g, '').toLowerCase() + "'>");
	</script>
	.gz &nbsp;
	<input type='button' name='f_backup_button' id='backup-button' onclick='backupButton()' value='Zrób kopię'>
	</form>
	<a href='' id='backup-link'>Link</a>
</div>
<br>

<div class='section-title'>Przywracanie danych historycznych</div>
<div class='section' id='restore-section'>
	<form id='restore-form' method='post' action='ipt/restore.cgi?_http_id=<% nv(http_id); %>' encType='multipart/form-data'>
		<input type='file' size='40' id='restore-name' name='restore_name'>
		<input type='button' name='f_restore_button' id='restore-button' value='Odtwórz' onclick='restoreButton()'>
		<br>
	</form>
</div>

<!-- / / / -->

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
