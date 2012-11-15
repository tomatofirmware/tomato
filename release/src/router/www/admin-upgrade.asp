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
<title>[<% ident(); %>] Administracja: Aktualizacja firmware</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->
<style type='text/css'>
#afu-progress {
	text-align: center;
	padding: 200px 0;
	width: 890px;
}
#afu-time {
	font-size: 26px;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

// <% nvram("jffs2_on"); %>

function clock()
{
	var t = ((new Date()).getTime() - startTime) / 1000;
	elem.setInnerHTML('afu-time', Math.floor(t / 60) + ':' + Number(Math.floor(t % 60)).pad(2));
}

function upgrade()
{
	var name;
	var i;
	var fom = document.form_upgrade;
	var ext;

	name = fixFile(fom.file.value);
	if (name.search(/\.(bin|trx|chk)$/i) == -1) {
		alert('Plik firmware musi być w formacie ".bin" lub ".trx".');
		return;
	}
	if (!confirm('Czy jesteś całkowicie pewien że chesz dokonać aktualizacji firmware używając ' + name + '?')) return;

	E('afu-upgrade-button').disabled = true;

	elem.display('afu-input', false);
	E('content').style.verticalAlign = 'middle';
	elem.display('afu-progress', true);
	elem.display('navi', false)
	elem.display('ident', false)

	startTime = (new Date()).getTime();
	setInterval('clock()', 800);

	fom.action += '?_reset=' + (E('f_reset').checked ? "1" : "0");
	form.addIdAction(fom);
	fom.submit();
}
</script>

</head>
<body>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<div id='afu-input'>
	<div class='section-title'>Aktualizacja oprogramowania routera</div>
	<div class='section'>
		<form name='form_upgrade' method='post' action='upgrade.cgi' encType='multipart/form-data'>
		<div id='box-input'>
			Wybierz plik z firmware:<br>
			<input type='file' name='file' size='50' style='height:20px'> <input type='button' value='Aktualizacja' id='afu-upgrade-button' onclick='upgrade()' style='height:20px'>
		</div>
		</form>
		<br><form name='form_reset' action='javascript:{}'>
		<div id='reset-input'>
			<input type='checkbox' id='f_reset'>&nbsp;&nbsp;Po zakończeniu procesu flashowania wyczyść pamięci NVRAM
		</div>
		</form>

		<br>
		<table border=0>
		<tr><td>Aktualna wersja:</td><td>&nbsp; <% version(1); %></td></tr>
		<script type='text/javascript'>
		//	<% sysinfo(); %>
		W('<tr><td>Dostępna pamięć:</td><td>&nbsp; ' + scaleSize(sysinfo.totalfreeram) + ' &nbsp; <small>(przybliżony rozmiar, który może być całkowicie zbuforowany w RAM)</small></td></tr>');
		</script>
		</table>

	</div>
</div>

<!-- /* JFFS2-BEGIN */ -->
<div class='note-disabledw' style='display:none' id='jwarn'>
<b>Nie można wykonać aktualizacji jeśli JFFS2 jest włączone..</b><br><br>
Aktualizacja może nadpisać aktualnie używaną partycję JFFS2. Przed rozpoczęciem aktualizacji
proszę wykonać kopię zapasową plików z partycji JFFS2, wyłączyć ją i zrestartować router.<br><br><br>
<a href='admin-jffs2.asp'>Wyłącz &raquo;</a>
</div>
<!--- /* JFFS2-END */ -->

<div id='afu-progress' style='display:none;margin:auto'>
	<img src='spin.gif' style='vertical-align:baseline'> <span id='afu-time'>0:00</span><br>
	Proszę <b>cierpliwie</b> czekać, aż ładowanie i fleszowanie nowego firmware zostanie zakończone.<br>
	<b>Ostrzeżenie:</b> Pod żadnym pozorem nie przerywaj tego procesu zarówno w przeglądarce jak i na routerze!<br>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>&nbsp;</td></tr>
</table>
/* JFFS2-BEGIN */
<script type='text/javascript'>
if (nvram.jffs2_on != '0') {
	E('jwarn').style.display = '';
	E('afu-input').style.display = 'none';
}
</script>
/* JFFS2-END */
</body>
</html>
