<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Copyright (C) 2011 Ofer Chen (Roadkill), Vicente Soriano (Victek)
	Adapted & Modified from Dual WAN Tomato Firmware.

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Captive Portal</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='<% nv('web_css'); %>.css'>
<script type='text/javascript' src='tomato.js'></script>
<style type='text/css'>
textarea {
 width: 98%;
 height: 15em;
}
</style>
<script type='text/javascript'>
//	<% nvram("NC_enable,NC_Verbosity,NC_GatewayName,NC_GatewayPort,NC_ForcedRedirect,NC_HomePage,NC_DocumentRoot,NC_LoginTimeout,NC_IdleTimeout,NC_MaxMissedARP,NC_ExcludePorts,NC_IncludePorts,NC_AllowedWebHosts,NC_MACWhiteList,NC_BridgeLAN,lan_ifname,lan1_ifname,lan2_ifname,lan3_ifname"); %>
function fix(name)
{
 var i;
 if (((i = name.lastIndexOf('/')) > 0) || ((i = name.lastIndexOf('\\')) > 0))
 name = name.substring(i + 1, name.length);
 return name;
}

function uploadButton()
{
 var fom;
 var name;
 var i;
 name = fix(E('upload-name').value);
 name = name.toLowerCase();
 if ((name.length <= 5) || (name.substring(name.length - 5, name.length).toLowerCase() != '.html')) {
 alert('Niewłaściwy plik, plik musi mieć rozszerzenie ".html".');
 return;
 }
 if (!confirm('Czy jesteś pewien, że plik' + name + 'ma zostać wysłany do urządzenia?')) return;
 E('upload-button').disabled = 1;
 fields.disableAll(E('config-section'), 1);
 fields.disableAll(E('footer'), 1);
 E('upload-form').submit();
}

function verifyFields(focused, quiet)
{
	var a = E('_f_NC_enable').checked;

	E('_NC_Verbosity').disabled = !a;
	E('_NC_GatewayName').disabled = !a;
	E('_NC_GatewayPort').disabled = !a;
	E('_f_NC_ForcedRedirect').disabled = !a;
	E('_NC_HomePage').disabled = !a;
	E('_NC_DocumentRoot').disabled = !a;
	E('_NC_LoginTimeout').disabled = !a;
	E('_NC_IdleTimeout').disabled = !a;
	E('_NC_MaxMissedARP').disabled = !a;
	E('_NC_ExcludePorts').disabled = !a;
	E('_NC_IncludePorts').disabled = !a;
	E('_NC_AllowedWebHosts').disabled = !a;
	E('_NC_MACWhiteList').disabled = !a;
	E('_NC_BridgeLAN').disabled = !a;

	var bridge = E('_NC_BridgeLAN');
	if(nvram.lan_ifname.length < 1)
		bridge.options[0].disabled=true;
	if(nvram.lan1_ifname.length < 1)
		bridge.options[1].disabled=true;
	if(nvram.lan2_ifname.length < 1)
		bridge.options[2].disabled=true;
	if(nvram.lan3_ifname.length < 1)
		bridge.options[3].disabled=true;

	if ( (E('_f_NC_ForcedRedirect').checked) && (!v_length('_NC_HomePage', quiet, 1, 255))) return 0;
	if (!v_length('_NC_GatewayName', quiet, 1, 255)) return 0;	
	if ( (E('_NC_IdleTimeout').value != '0') && (!v_range('_NC_IdleTimeout', quiet, 300))) return 0;
	return 1;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');
  fom.NC_enable.value = E('_f_NC_enable').checked ? 1 : 0;
  fom.NC_ForcedRedirect.value = E('_f_NC_ForcedRedirect').checked ? 1 : 0;

  // blank spaces with commas
  e = E('_NC_ExcludePorts');
  e.value = e.value.replace(/\,+/g, ' ');

  e = E('_NC_IncludePorts');
  e.value = e.value.replace(/\,+/g, ' ');

  e = E('_NC_AllowedWebHosts');
  e.value = e.value.replace(/\,+/g, ' ');
  
  e = E('_NC_MACWhiteList');
  e.value = e.value.replace(/\,+/g, ' ');

  fields.disableAll(E('upload-section'), 1);
  if (fom.NC_enable.value == 0) {
	fom._service.value = 'splashd-stop';
  }
	else {
	fom._service.value = 'splashd-restart';
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
<div class='section-title'>Zarządzanie Captive Portal</div>
<div class='section' id='config-section'>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='splashd.asp'>
<input type='hidden' name='_service' value='splashd-restart'>
<input type='hidden' name='NC_enable'>
<input type='hidden' name='NC_ForcedRedirect'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Włącz', name: 'f_NC_enable', type: 'checkbox', value: nvram.NC_enable == '1' },
	{ title: 'Interfejs', multi: [
		{ name: 'NC_BridgeLAN', type: 'select', options: [
			['br0','LAN (br0)*'],
			['br1','LAN1 (br1)'],
			['br2','LAN2 (br2)'],
			['br3','LAN3 (br3)']
			], value: nvram.NC_BridgeLAN, suffix: ' <small>* default</small> ' } ] },
	{ title: 'Nazwa bramy', name: 'NC_GatewayName', type: 'text', maxlen: 255, size: 34, value: nvram.NC_GatewayName },
	{ title: 'Przekierowanie do Captive Site', name: 'f_NC_ForcedRedirect', type: 'checkbox', value: (nvram.NC_ForcedRedirect == '1') },
	{ title: 'Strona domowa', name: 'NC_HomePage', type: 'text', maxlen: 255, size: 34, value: nvram.NC_HomePage },
	{ title: 'Ścieżka do pliku powitalnego html', name: 'NC_DocumentRoot', type: 'text', maxlen: 255, size: 20, value: nvram.NC_DocumentRoot, suffix: '<span>&nbsp;/splash.html</span>' },
	{ title: 'Limit czasu sesji', name: 'NC_LoginTimeout', type: 'text', maxlen: 8, size: 4, value: nvram.NC_LoginTimeout, suffix: ' <small>sekund</small>' },
	{ title: 'Limit bezczynności', name: 'NC_IdleTimeout', type: 'text', maxlen: 8, size: 4, value: nvram.NC_IdleTimeout, suffix: ' <small>sekund (0 - brak limitu)</small>' },
	{ title: 'Maksymalna ilość straconych ARP', name: 'NC_MaxMissedARP', type: 'text', maxlen: 10, size: 2, value: nvram.NC_MaxMissedARP },
	null,
	{ title: 'Poziom logowania', name: 'NC_Verbosity', type: 'text', maxlen: 10, size: 2, value: nvram.NC_Verbosity },
	{ title: 'Port bramy', name: 'NC_GatewayPort', type: 'text', maxlen: 10, size: 7, value: fixPort(nvram.NC_GatewayPort, 5280) },
	{ title: 'Porty wyłączone z przekierowania', name: 'NC_ExcludePorts', type: 'text', maxlen: 255, size: 34, value: nvram.NC_ExcludePorts },
	{ title: 'Porty włączone do przekierowania', name: 'NC_IncludePorts', type: 'text', maxlen: 255, size: 34, value: nvram.NC_IncludePorts },
	{ title: 'Lista adresów URL ignorowanych przez Captive Portal', name: 'NC_AllowedWebHosts', type: 'text', maxlen: 255, size: 34, value: nvram.NC_AllowedWebHosts },
	{ title: '"Biała lista" adresów MAC', name: 'NC_MACWhiteList', type: 'text', maxlen: 255, size: 34, value: nvram.NC_MACWhiteList }
]);
</script>
</form>
</div>
<br>
<div class='section-title'>Ścieżka do własnego pliku Splash</div>
<div class='section' id='upload-section'>
 <form id='upload-form' method='post' action='uploadsplash.cgi?_http_id=<% nv(http_id); %>' encType='multipart/form-data'>
 <input type='file' size='40' id='upload-name' name='upload_name'>
 <input type='button' name='f_upload_button' id='upload-button' value='Prześlij' onclick='uploadButton()'>
 <br>
 </form>
</div>
<hr>
<span style='color:blue'>
<b>Captive Portal. Podręcznik użytkownika.</b><br>
<br>
<b>*- Włączenie funkcji:</b> Po zaznaczeniu i zapisaniu router będzie pokazywać Baner Powitalny (Welcome Banner) gdy komputer będzie się próbować łączyć do Internetu.<br>
<b>*- Interfejs:</b> Interfejs (bridge), na którym Captive Portal ma pracować.<br>
<b>*- Nazwa bramy:</b> Nazwa bramy pojawiająca się na banerze powitalnym.<br>
<b>*- Przekierowanie do Captive Site:</b> Gdy włączone, to gdy zostanie naciśnięta zgoda (Agree) na Banerze Powitalnym, zostanie wyświetlona 'Strona domowa' (opis w następnej linijce).<br>
<b>*- Strona domowa</b> URL który się pojawi po naciśnięciu zgody (Agree) na Banerze Powitalnym.<br>
<b>*- Ścieżka do pliku html z Banerem Powitalnym:</b> Lokalizacja w której znajduje się Baner Powitalny <br>
<b>*- Limit czasu sesji:</b> Przez ten okres czasu, gdy użytkownik korzysta z urządzenia, nie będzie się pojawiać Baner Powitalny . Domyślnie=3600 sek.(1 godzina).<br>
<b>*- Limit oczekiwania:</b> Czas, jaki należy odczekać przed dalszym korzystaniem z urządzenia po wyczerpaniu Limitu czasu sesji. Domyślnie=0.<br>
<b>*- Maksymalna ilość straconych ARP:</b>Liczba straconych ARP przed uznaniem, że klient zakończył połączenie. Domyślnie = 5<br>
<b>*- Poziom logowania:</b> Informacje z tego modułu są gromadzone wewnętrznie. Poziomy 0=Cisza, 10=Papuga, 2=Domyślnie.<br>
<b>*- Port bramy:</b> Port używany przez mechanizm Captive Portal do przekierowywania stron. Port od 1 do 65534. Domyślnie=5280.<br>
<b>*- Porty włączone/wyłączone z przekierowania:</b> Ustawiając taki port (włączony lub wyłączony) pozostaw spację pomiędzy każdym portem, np.: 25 110 4662 4672. Użyj tylko jednej z tych opcji aby zapobiec konfliktom konfiguracji.<br>
<b>*- Lista adresów URL ignorowanych przez Captive Portal:</b> Lista zawiera adresy URL które mają być dostępne bez pokazywania Banera Powitalnego. Wprowadzając kolejne adresy URL również pozostaw pomiędzy nimi spacje. np.; http://www.google.com http://www.google.es<br>
<b>*- "Biała lista" adresów MAC :</b> Lista adresów MAC niepodlegających mechanizmowi Captive Portal. Poszczególne adresy MAC należy oddzielić spacjami , np.; 11:22:33:44:55:66 11:22:33:44:55:67<br>
<b>*- Ścieżka do własnego pliku Splash:</b> Tu możesz umieścić swój własny Baner Powitalny który zostąpi baner domyślny.<br><br></span>
<br>
<span style='color:red'>
<b> Uwaga: Jeśli minie okres czasu określony w opcji Limit czasu sesji należy ponownie odwiedzić stronę z banerem żeby otrzymać nowy przydział czasu. Miej świadomość, że informacja o kończącym się Limicie czasu sesji nie jest wyświetlana, więc użytkownik może nagle utracić łączność z Internetem.</b><br>
</span>
<br>
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
