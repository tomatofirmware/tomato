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
<title>[<% ident(); %>] Zaawansowane: DHCP / DNS</title>
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

//	<% nvram("dhcpd_dmdns,dns_addget,dhcpd_gwmode,dns_intcpt,dhcpd_slt,dhcpc_minpkt,dnsmasq_custom,dnsmasq_norw,dhcpd_lmax,dhcpc_custom,dns_norebind,dhcpd_static_only"); %>

if ((isNaN(nvram.dhcpd_lmax)) || ((nvram.dhcpd_lmax *= 1) < 1)) nvram.dhcpd_lmax = 255;

function verifyFields(focused, quiet)
{
	var b = (E('_f_dhcpd_sltsel').value == 1);
	elem.display('_dhcpd_sltman', b);
	if ((b) && (!v_range('_f_dhcpd_slt', quiet, 1, 43200))) return 0;
	if (!v_length('_dnsmasq_custom', quiet, 0, 2048)) return 0;
	if (!v_range('_dhcpd_lmax', quiet, 1, 0xFFFF)) return 0;
	if (!v_length('_dhcpc_custom', quiet, 0, 80)) return 0;
	return 1;
}

function nval(a, b)
{
	return (a == null || (a + '').trim() == '') ? b : a;
}

function save()
{
	if (!verifyFields(null, false)) return;

	var a;
	var fom = E('_fom');

	fom.dhcpd_dmdns.value = E('_f_dhcpd_dmdns').checked ? 1 : 0;
	a = E('_f_dhcpd_sltsel').value;
	fom.dhcpd_slt.value = (a != 1) ? a : E('_f_dhcpd_slt').value;
	fom.dns_addget.value = E('_f_dns_addget').checked ? 1 : 0;
	fom.dns_norebind.value = E('_f_dns_norebind').checked ? 1 : 0;
	fom.dhcpd_gwmode.value = E('_f_dhcpd_gwmode').checked ? 1 : 0;
	fom.dns_intcpt.value = E('_f_dns_intcpt').checked ? 1 : 0;
	fom.dhcpc_minpkt.value = E('_f_dhcpc_minpkt').checked ? 1 : 0;
	fom.dhcpd_static_only.value = E('_f_dhcpd_static_only').checked ? '1' : '0';

	if (fom.dhcpc_minpkt.value != nvram.dhcpc_minpkt ||
	    fom.dhcpc_custom.value != nvram.dhcpc_custom) {
		nvram.dhcpc_minpkt = fom.dhcpc_minpkt.value;
		nvram.dhcpc_custom = fom.dhcpc_custom.value;
		fom._service.value = '*';
	}
	else {
		fom._service.value = 'dnsmasq-restart';
	}

	if (fom.dns_intcpt.value != nvram.dns_intcpt) {
		nvram.dns_intcpt = fom.dns_intcpt.value;
		if (fom._service.value != '*') fom._service.value += ',firewall-restart';
	}

/* IPV6-BEGIN */
	if (fom.dhcpd_dmdns.value != nvram.dhcpd_dmdns) {
		nvram.dhcpd_dmdns = fom.dhcpd_dmdns.value;
		if (fom._service.value != '*') fom._service.value += ',radvd-restart';
	}
/* IPV6-END */

	form.submit(fom, 1);
}

function toggleVisibility(whichone) {
	if(E('sesdiv' + whichone).style.display=='') {
		E('sesdiv' + whichone).style.display='none';
		E('sesdiv' + whichone + 'showhide').innerHTML='(Kliknij, żeby wyświetlić)';
		cookie.set('adv_dhcpdns_' + whichone + '_vis', 0);
	} else {
		E('sesdiv' + whichone).style.display='';
		E('sesdiv' + whichone + 'showhide').innerHTML='(Kliknij, żeby ukryć)';
		cookie.set('adv_dhcpdns_' + whichone + '_vis', 1);
	}
}

function init() {
	var c;
	if (((c = cookie.get('adv_dhcpdns_notes_vis')) != null) && (c == '1')) {
		toggleVisibility("notes");
	}
}
</script>

</head>
<body onload='init()'>
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

<input type='hidden' name='_nextpage' value='advanced-dhcpdns.asp'>
<input type='hidden' name='_service' value=''>

<input type='hidden' name='dhcpd_dmdns'>
<input type='hidden' name='dhcpd_slt'>
<input type='hidden' name='dns_addget'>
<input type='hidden' name='dns_norebind'>
<input type='hidden' name='dhcpd_gwmode'>
<input type='hidden' name='dns_intcpt'>
<input type='hidden' name='dhcpc_minpkt'>
<input type='hidden' name='dhcpd_static_only'>

<div class='section-title'>Serwer DHCP / DNS (LAN)</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Użyj wewnętrznego DNS', name: 'f_dhcpd_dmdns', type: 'checkbox', value: nvram.dhcpd_dmdns == '1' },
	{ title: 'Użyj otrzymanego DNS razem z DNS użytkownika', name: 'f_dns_addget', type: 'checkbox', value: nvram.dns_addget == '1' },
	{ title: 'Zapobiegaj atakom DNS-rebind', name: 'f_dns_norebind', type: 'checkbox', value: nvram.dns_norebind == '1' },
	{ title: 'Przechwyć port DNS<br>(UDP 53)', name: 'f_dns_intcpt', type: 'checkbox', value: nvram.dns_intcpt == '1' },
	{ title: 'Użyj bramy wprowadzonej przez użytkownika jeśli WAN jest wyłączony', name: 'f_dhcpd_gwmode', type: 'checkbox', value: nvram.dhcpd_gwmode == '1' },
	{ title: 'Ignoruj zapytania DHCP od nieznanych urządzeń', name: 'f_dhcpd_static_only', type: 'checkbox', value: nvram.dhcpd_static_only == '1' },
	{ title: 'Maksymalna liczba aktywnych klientów DHCP', name: 'dhcpd_lmax', type: 'text', maxlen: 5, size: 8, value: nvram.dhcpd_lmax },
	{ title: 'Czas dzierżawy statycznej', multi: [
		{ name: 'f_dhcpd_sltsel', type: 'select', options: [[0,'Taki sam jak normalny czas dzierżawy'],[-1,'"Nieskończony"'],[1,'Własny']],
			value: (nvram.dhcpd_slt < 1) ? nvram.dhcpd_slt : 1 },
		{ name: 'f_dhcpd_slt', type: 'text', maxlen: 5, size: 8, prefix: '<span id="_dhcpd_sltman"> ', suffix: ' <i>(minut)</i></span>',
			value: (nvram.dhcpd_slt >= 1) ? nvram.dhcpd_slt : 3600 } ] },
	{ title: '<a href="http://www.thekelleys.org.uk/" target="_new">Dnsmasq</a><br>Własna konfiguracja', name: 'dnsmasq_custom', type: 'textarea', value: nvram.dnsmasq_custom }
]);
</script>

<!-- / / / -->

<div class='section-title'>Klient DHCP (WAN)</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Opcje DHCPC', name: 'dhcpc_custom', type: 'text', maxlen: 80, size: 34, value: nvram.dhcpc_custom },
	{ title: 'Redukuj rozmar pakietu', name: 'f_dhcpc_minpkt', type: 'checkbox', value: nvram.dhcpc_minpkt == '1' }
]);
</script>
</div>

<!-- / / / -->

<div class='section-title'>Opis opcji <small><i><a href='javascript:toggleVisibility("notes");'><span id='sesdivnotesshowhide'>(Kliknij, żeby wyświetlić)</span></a></i></small></div>
<div class='section' id='sesdivnotes' style='display:none'>

<i>Serwer DHCP / DNS (LAN):</i><br>
<ul>
<li><b>Użyj wewnętrznego DNS</b> - Pozwala dnsmasq na bycie serwerem DNS dla LAN.</li>
<li><b>Użyj otrzymanego DNS razem z DNS użytkownika</b> - Dodaje otrzymane od Twojego ISP(z WAN) serwery DNS do statycznej listy serwerów DNS(zobacz <a href='basic-network.asp'>Konfiguracja sieciowa</a>).</li>
<li><b>Zapobiegaj atakom DNS-rebind</b> - Włącza ochronę DNS rebinding w Dnsmasq.</li>
<li><b>Przechwyć port DNS</b> - Każde zapytanie/pakiet DNS wysłany do portu 53 UDP zostanie przekierowane do wewnętrznego serwera DNS.</li>
<li><b>Użyj bramy wprowadzonej przez użytkownika jeśli WAN jest wyłączony</b> - DHCP będzie używać adresu IP routera jako domyślną bramę dla każdego w LAN.</li>
<li><b>Ignoruj zapytania DNS (...)</b> - Dnsmasq będzie ignorować zapytania DHCP od urządzeń innych niż mające swoje adresy MAC wpisane w <a href='basic-static.asp'>Statyczne DHCP/ARP</a>. Urządzenia nie uwzględnione w tej tablicy nie otrzymają adresu IP.</li>
<li><b>Czas dzierżawy statycznej</b> - Absolutnie maksymalny dopuszczalny czas dzierżawy DHCP.</li>
<li><b>Własna konfiguracja</b> - Dodatkowe opcje które zostaną dodane do pliku konfiguracyjnego Dnsmasq.</li>
</ul>

<i>Klient DHCP (WAN):</i><br>
<ul>
<li><b>Opcje DHCPC</b> - Dodatkowe opcjie klienta DHCP.</li>
</ul>

<i>Inne powiązane uwagi/podpowiedzi:</i><br>
<ul>
<li>Zawartość pliku /etc/dnsmasq.custom (jeśli takowy istnieje) jest dodawana na końcu pliku konfiguracyjnego Dnsmasq.</li>
</ul>

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
<script type='text/javascript'>verifyFields(null, true);</script>
</body>
</html>

