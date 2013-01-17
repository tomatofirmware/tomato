<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2012 Shibby
	http://openlinksys.info
	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Administracja: Projekt TomatoAnon</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript'>
//	<% nvram("tomatoanon_enable,tomatoanon_answer,tomatoanon_cru,tomatoanon_id"); %>

var anon_link = '&nbsp;&nbsp;<a href="http://tomato.groov.pl/tomatoanon.php?search=9&routerid=<% nv('tomatoanon_id'); %>" target="_blank"><i>[Sprawdź mój router]</i></a>';

function verifyFields(focused, quiet)
{
	var o = (E('_tomatoanon_answer').value == '1');
	E('_tomatoanon_enable').disabled = !o;

	var s = (E('_tomatoanon_enable').value == '1');
	E('_tomatoanon_cru').disabled = !o || !s;

	return 1;
}

function save()
{
	if (verifyFields(null, 0)==0) return;
	var fom = E('_fom');

	fom._service.value = 'tomatoanon-restart';
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
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='admin-tomatoanon.asp'>
<input type='hidden' name='_service' value='tomatoanon-restart'>
<div class='section-title'>Informacje o projekcie TomatoAnon</div>
<div class="fields"><div class="about">
<b>Witajcie,</b><br>
<br>
Chciałbym przedstawić nowy projekt nazwany TomatoAnon.<br>
TomatoAnon będzie wysyłał anonimowe lub niekompletne informacje odnośnie modelu routera oraz zainstalowanej wersji tomato.<br>
Te informacje wykorzystane zostaną wyłącznie w celach statystycznych.<br>
<b>Żadne prywatne informacje (np. adres MAC, adres IP itd) NIE BĘDĄ wysyłane!</b></br>
Skrypt jest otwarty, napisany w bash`u, tak więc każdy może przekonać się jakie dane tak naprawde są wysyłane do bazy.<br>
<br>
Wyniki można przeglądać na stronie <a href=http://tomato.groov.pl/tomatoanon.php target=_blanc><b>http://tomato.groov.pl/tomatoanon.php</b></a>.<br>
Te informacje mogą pomóc w dokonaniu wyboru najlepszego i najbardziej popularnego routera w waszym kraju.<br>
Można sprawdzić, która wersja tomato jest najbardziej popularna wśród użytkowników i która jest najbardziej stabilna.<br>
<br>
Jeżeli nie wyrażasz zgody by skrypt wysyłał jakiekolwiek dane, możesz go zwyczajnie wyłączyć.<br>
Pamiętaj, że możesz go aktywować kiedy tylko zechcesz.<br>
<br>
Wysyłane informacje to:<br>
 - Suma MD5 generowana z adresów MAC WAN+LAN - to identyfikator twojego routera. Np: 1c1dbd4202d794251ec1acf1211bb2c8<br>
 - Model routera. Np: Asus RT-N66U<br>
 - Wersja Tomato. Np: 102 K26 USB<br>
 - Typ wersji. Np: Mega-VPN-64K<br>
 - Kraj. Np: POLAND<br>
 - Kod kraju. Np: PL<br>
 - Czas działania. Np: 3 days<br>
To wszystko !!<br>
<br>
Dziękuję za zapoznanie się z tematem projektu i proszę o dokonanie włąściwego wyboru.<br>
<br>
<b>Pozdrawiam!</b></font>
</div></div>
<br>
<br>
<div class='section-title'>Ustawienia TomatoAnon <script>W(anon_link);</script></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Czy wiesz czym jest projekt TomatoAnon ?', name: 'tomatoanon_answer', type: 'select', options: [ ['0','Nie. Muszę przeczytać informacje zanim podejdę wybór'], ['1','Tak. Chcę dokonać wyboru'] ], value: nvram.tomatoanon_answer, suffix: ' '},
	{ title: 'Czy chcesz by TomatoAnon wysyłał anonimowe informacje? ?', name: 'tomatoanon_enable', type: 'select', options: [ ['-1','Nie jestem jeszcze zdecydowany'], ['1','Tak, zgadzam się'], ['0','Nie, nie zgadzam się'] ], value: nvram.tomatoanon_enable, suffix: ' '},
	{ title: 'Wyślij dane co', indent: 2, name: 'tomatoanon_cru', type: 'text', maxlen: 5, size: 7, value: nvram.tomatoanon_cru, suffix: ' <small>godzin(y) (zakres: 1 - 12; domyślnie: 6)</small>' }
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
