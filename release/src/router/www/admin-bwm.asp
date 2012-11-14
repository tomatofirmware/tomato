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
<title>[<% ident(); %>] Administracja: Monitorowanie przepustowości</title>
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

//	<% nvram("rstats_enable,rstats_path,rstats_stime,rstats_offset,rstats_exclude,rstats_sshut,et0macaddr,cifs1,cifs2,jffs2_on,rstats_bak"); %>

function backupNameChanged()
{
	if (location.href.match(/^(http.+?\/.+\/)/)) {
		E('backup-link').href = RegExp.$1 + 'bwm/' + fixFile(E('backup-name').value) + '.gz?_http_id=' + nvram.http_id;
	}
}

function backupButton()
{
	var name;

	name = fixFile(E('backup-name').value);
	if (name.length <= 1) {
		alert('Niewłaściwa nazwa pliku');
		return;
	}
	location.href = 'bwm/' + name + '.gz?_http_id=' + nvram.http_id;
}

function restoreButton()
{
	var fom;
	var name;
	var i;

	name = fixFile(E('restore-name').value);
	name = name.toLowerCase();
	if ((name.length <= 3) || (name.substring(name.length - 3, name.length).toLowerCase() != '.gz')) {
		alert('Niewłaściwa nazwa pliku. Oczekiwany plik typu ".gz".');
		return;
	}
	if (!confirm('Odtworzyć dane z ' + name + '?')) return;

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

	eLoc = E('_f_loc');
	eUser = E('_f_user');
	eTime = E('_rstats_stime');
	eOfs = E('_rstats_offset');

	b = !E('_f_rstats_enable').checked;
	eLoc.disabled = b;
	eUser.disabled = b;
	eTime.disabled = b;
	eOfs.disabled = b;
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

	path = getPath();
	E('newmsg').style.visibility = ((nvram.rstats_path != path) && (path != '*nvram') && (path != '')) ? 'visible' : 'hidden';

	bak = 0;
	v = eLoc.value;
	b = (v == '*user');
	elem.display(eUser, b);
	if (b) {
		if (!v_path(eUser, quiet, 1)) return 0;
	}
/* JFFS2-BEGIN */
	else if (v == '/jffs/') {
		if (nvram.jffs2_on != '1') {
			ferror.set(eLoc, 'JFFS2 nie jest włączony.', quiet);
			return 0;
		}
	}
/* JFFS2-END */
/* CIFS-BEGIN */
	else if (v.match(/^\/cifs(1|2)\/$/)) {
		if (nvram['cifs' + RegExp.$1].substr(0, 1) != '1') {
			ferror.set(eLoc, 'CIFS #' + RegExp.$1 + ' nie jest włączony.', quiet);
			return 0;
		}
	}
/* CIFS-END */
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
	en = E('_f_rstats_enable').checked;
	fom = E('_fom');
	fom._service.value = 'rstats-restart';
	if (en) {
		path = getPath();
		if (((E('_rstats_stime').value * 1) <= 48) &&
			((path == '*nvram') || (path == '/jffs/'))) {
			if (!confirm('Częste zapisywanie do NVRAM lub JFFS2 nie jest zalecane ze względu na trwałość pamięci flash. Czy mimo wszystko kontynuować?')) return;
		}
		if ((nvram.rstats_path != path) && (fom.rstats_path.value != path) && (path != '') && (path != '*nvram') &&
			(path.substr(path.length - 1, 1) != '/')) {
			if (!confirm('Notka: ' + path + ' będzie traktowany jak plik. Jeśli to katalog, proszę użyć znaku ukośnika /. Czy kontynuować?')) return;
		}
		fom.rstats_path.value = path;

		if (E('_f_new').checked) {
			fom._service.value = 'rstatsnew-restart';
			aj = 0;
		}
	}

	fom.rstats_path.disabled = !en;
	fom.rstats_enable.value = en ? 1 : 0;
	fom.rstats_sshut.value = E('_f_sshut').checked ? 1 : 0;
	fom.rstats_bak.value = E('_f_bak').checked ? 1 : 0;

	e = E('_rstats_exclude');
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

<div class='section-title'>Monitorowanie przepustowości</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='admin-bwm.asp'>
<input type='hidden' name='_service' value='rstats-restart'>
<input type='hidden' name='rstats_enable'>
<input type='hidden' name='rstats_path'>
<input type='hidden' name='rstats_sshut'>
<input type='hidden' name='rstats_bak'>

<script type='text/javascript'>
switch (nvram.rstats_path) {
case '':
case '*nvram':
case '/jffs/':
case '/cifs1/':
case '/cifs2/':
	loc = nvram.rstats_path;
	break;
default:
	loc = '*user';
	break;
}
createFieldTable('', [
	{ title: 'Włączony', name: 'f_rstats_enable', type: 'checkbox', value: nvram.rstats_enable == '1' },
	{ title: 'Miejsce zapisu historii', multi: [
		{ name: 'f_loc', type: 'select', options: [['','RAM (tymczasowo)'],['*nvram','NVRAM'],
/* JFFS2-BEGIN */
			['/jffs/','JFFS2'],
/* JFFS2-END */
/* CIFS-BEGIN */
			['/cifs1/','CIFS 1'],['/cifs2/','CIFS 2'],
/* CIFS-END */
			['*user','Ścieżka użytkownika']], value: loc },
		{ name: 'f_user', type: 'text', maxlen: 48, size: 50, value: nvram.rstats_path }
	] },
	{ title: 'Częstotliwość zapisu', indent: 2, name: 'rstats_stime', type: 'select', value: nvram.rstats_stime, options: [
		[1,'Co godzinę'],[2,'Co 2 godziny'],[3,'Co 3 godziny'],[4,'Co 4 godziny'],[5,'Co 5 godzin'],[6,'Co 6 godzin'],
		[9,'Co 9 godzin'],[12,'Co 12 godzin'],[24,'Co 24 godziny'],[48,'Co 2 dni'],[72,'Co 3 dni'],[96,'Co 4 dni'],
		[120,'Co 5 dni'],[144,'Co 6 dni'],[168,'Co tydzień']] },
	{ title: 'Zapisz przy zamknięciu', indent: 2, name: 'f_sshut', type: 'checkbox', value: nvram.rstats_sshut == '1' },
	{ title: 'Utwórz nowy plik<br><small>(zeruje dane)</small>', indent: 2, name: 'f_new', type: 'checkbox', value: 0,
		suffix: ' &nbsp; <b id="newmsg" style="visibility:hidden"><small>(notka: zaznacz jeśli to nowy plik)</small></b>' },
	{ title: 'Twórz kopie', indent: 2, name: 'f_bak', type: 'checkbox', value: nvram.rstats_bak == '1' },
	{ title: 'Pierwszy dzień miesiąca', name: 'rstats_offset', type: 'text', value: nvram.rstats_offset, maxlen: 2, size: 4 },
	{ title: 'Wyklucz interfejsy', name: 'rstats_exclude', type: 'text', value: nvram.rstats_exclude, maxlen: 64, size: 50, suffix: '&nbsp;<small>(lista oddzielona przecinkami)</small>' }
]);
</script>
</form>
</div>

<br>

<div class='section-title'>Kopia zapasowa</div>
<div class='section' id='backup-section'>
	<form>
	<script type='text/javascript'>
	W("<input type='text' size='40' maxlength='64' id='backup-name' name='backup_name' onchange='backupNameChanged()' value='tomato_rstats_" + nvram.et0macaddr.replace(/:/g, '').toLowerCase() + "'>");
	</script>
	.gz &nbsp;
	<input type='button' name='f_backup_button' id='backup-button' onclick='backupButton()' value='Zrób kopię'>
	</form>
	<a href='' id='backup-link'>Link</a>
</div>
<br>

<div class='section-title'>Przywracanie danych historycznych</div>
<div class='section' id='restore-section'>
	<form id='restore-form' method='post' action='bwm/restore.cgi?_http_id=<% nv(http_id); %>' encType='multipart/form-data'>
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
