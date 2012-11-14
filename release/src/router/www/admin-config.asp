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
<title>[<% ident(); %>] Administracja: Konfiguracja</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("et0macaddr,t_features,t_model_name"); %>
//	<% nvstat(); %>

function backupNameChanged()
{
	var name = fixFile(E('backup-name').value);
	if (name.length > 1) {
		E('backup-link').href = 'cfg/' + name + '.cfg?_http_id=' + nvram.http_id;
	}
	else {
		E('backup-link').href = '?';
	}
}

function backupButton()
{
	var name = fixFile(E('backup-name').value);
	if (name.length <= 1) {
		alert('Invalid filename');
		return;
	}
	location.href = 'cfg/' + name + '.cfg?_http_id=' + nvram.http_id;
}

function restoreButton()
{
	var name, i, f;

	name = fixFile(E('restore-name').value);
	name = name.toLowerCase();
	if ((name.indexOf('.cfg') != (name.length - 4)) && (name.indexOf('.cfg.gz') != (name.length - 7))) {
		alert('Niewłaściwy plik. Spodziewany plik z rozszerzeniem ".cfg".');
		return;
	}
	if (!confirm('Czy jesteś pewien?')) return;
	E('restore-button').disabled = 1;

	f = E('restore-form');
	form.addIdAction(f);
	f.submit();
}

function resetButton()
{
	var i;

	i = E('restore-mode').value;
	if (i == 0) return;
	if ((i == 2) && (features('!nve'))) {
		if (!confirm('OSTRZEŻENIE: Kasowanie zawartości NVRAM na routerze ' + nvram.t_model_name + ' może go uszkodzić. Może nie być możliwości ponownego poprawnego ustawienia NVRAM po jej całkowitym wykasowaniu. Czy mimo wszystko kontunuować?')) return;
	}
	if (!confirm('Czy jesteś CAŁKOWICIE pewien?')) return;
	E('reset-button').disabled = 1;
	form.submit('aco-reset-form');
}
</script>
</head>
<body onload='backupNameChanged()'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<div class='section-title'>Kopia zapasowa konfiguracji</div>
<div class='section'>
	<form>
		<script type='text/javascript'>
		W("<input type='text' size='40' maxlength='64' id='backup-name' onchange='backupNameChanged()' value='tomato_v" + ('<% version(); %>'.replace(/\./g, '')) + "_m" + nvram.et0macaddr.replace(/:/g, '').substring(6, 12) + "'>");
		</script>
		.cfg &nbsp;
		<input type='button' name='f_backup_button' onclick='backupButton()' value='Zapisz kopię'><br>
		<a href='' id='backup-link'>Link</a>
	</form>
</div>

<br><br>

<div class='section-title'>Przywracanie konfiguracji</div>
<div class='section'>
	<form id='restore-form' method='post' action='cfg/restore.cgi' encType='multipart/form-data'>
		Wybierz plik z konfiguracją do przywrócenia:<br>
		<input type='file' size='40' id='restore-name' name='filename'> <input type='button' name='f_restore_button' id='restore-button' value='Odtwórz' onclick='restoreButton()'>
		<br>
	</form>
	Nie jest zalecane odtwarzanie konfiguracji z innej wersji Tomato niż aktualnie zainstalowana.
</div>

<br><br>

<div class='section-title'>Przywracanie ustawień standardowych</div>
<div class='section'>
	<form id='aco-reset-form' method='post' action='cfg/defaults.cgi'>
	<select name='mode' id='restore-mode'>
		<option value=0>Wybierz...</option>
		<option value=1>Przywróć domyślne ustawienia routera (tryb normalny)</option>
		<option value=2>Usuń wszystkie dane z pamięci NVRAM (niebezpieczne)</option>
	</select>
	<input type='button' value='OK' onclick='resetButton()' id='reset-button'>
	</form>
</div>

<br>

<div class='section-title'></div>
<div class='section'>
<script type='text/javascript'>
var a = nvstat.free / nvstat.size * 100.0;
createFieldTable('', [
	{ title: 'Całkowita / Dostępna NVRAM:', text: scaleSize(nvstat.size) + ' / ' + scaleSize(nvstat.free) + ' <small>(' + (a).toFixed(2) + '%)</small>' }
]);

if (a <= 5) {
	document.write('<br><div id="notice1">' +
		'Wolny obszar NVRAM jest bardzo mały. Jest wysoce wskazane, żeby ' +
		'usunąc wszystkie dane z pamięci NVRAM i skonfigurować router ręcznie ' +
		'Pozwoli to usunąć niepotrzebne i przestarzałe dane.' +
		'</div><br style="clear:both">');
}
</script>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>&nbsp;</td></tr>
</table>
<br><br>
</body>
</html>
