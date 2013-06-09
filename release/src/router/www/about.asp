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
<title>[<% ident(); %>] About</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript'>
//	<% nvram(''); %>	// http_id

var clicks = 0;
var tux = null;
var t = 0;
var r = 3;
var rd = 1;

function moo()
{
	if ((r <= 2) || (r >= 25)) rd = -rd;
	r += rd;
	t += (Math.PI / 10);
	if (t > (2 * Math.PI)) t = 0;

	var x = tux.origX + (r * Math.sin(t));
	var y = tux.origY + (r * Math.cos(t));

	tux.style.left = x + 'px';
	tux.style.top = y + 'px';

	if (clicks > 0) setTimeout(moo, 33);
}

function onClick()
{
	try {
		++clicks;
		if (clicks < 10) moo();
			else clicks = 0;
	}
	catch (ex) {
	}
}

function init()
{
	try {
		tux = E('tux');

		var o = elem.getOffset(tux);
		tux.origX = o.x;
		tux.origY = o.y;

		tux.style.position = 'absolute';
		tux.style.left = o.x + 'px';
		tux.style.top = o.y + 'px';

		tux.addEventListener('click', onClick, false);
	}
	catch (ex) {
	}
}
</script>

<!-- / / / -->

</head>
<body onload='init()'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<div style='float:right;margin:20px 20px;text-align:center'>
<img src='tux.png' alt='Linux &amp; Tomato' id='tux'>
</div>
<div style='margin:30px 30px;font-size:14px;color:#555;'>
<b>Tomato RAF Firmware v<% version(1); %></b><br>
- Linux kernel <% version(2); %> and Broadcom Wireless Driver <% version(3); %> updates<br>
- Support for additional router models, dual-band and Wireless-N mode.<br>
<!-- USB-BEGIN -->
- USB support integration and GUI<br>
<!-- USB-END -->
<!-- IPV6-BEGIN -->
- IPv6 support<br>
<!-- IPV6-END -->
- Extended System and Hardware Info<br>
- HFS/HFS+ MAC OS x read support<br>
<!-- NOCAT-BEGIN -->
- Captive Portal. (NocatSplash). Copyright (C) 2011 Ofer Chen & Vicente Soriano<br>
<!-- NOCAT-END -->
<!-- NGINX-BEGIN -->
- Web Server. (NGinX). Copyright (C) 2013 Ofer Chen & Vicente Soriano<br>
<!-- NGINX-END -->
Tomato RAF. Copyright (C) 2007-2013 Vicente Soriano. <br>
<a href='http://www.tomatoraf.com' target='_new'>http://victek.is-a-geek.com</a><br>
<br>
<br>
<b>Based on TomatoUSB and Original Tomato by Jonathan Zarate.</b><br>
Copyright (C) 2008-2011 Fedor Kozhevnikov, Ray Van Tassle, Wes Campaigne<br>
<a href='http://www.tomatousb.org/' target='_new'>http://www.tomatousb.org</a><br>
Copyright (C) 2006-2010 Jonathan Zarate<br>
<a href='http://www.polarcloud.com/tomato/' target='_new'>http://www.polarcloud.com/tomato/</a><br>
<br>
<b>This version may include other features:</b><br>
<br>
<!-- OPENVPN-BEGIN -->
<b>OpenVPN integration and GUI</b><br>
Copyright (C) 2010 Keith Moyer<br>
<a href='mailto:tomatovpn@keithmoyer.com'>tomatovpn@keithmoyer.com</a><br>
<br>
<!-- OPENVPN-END -->
<!-- JYAVENARD-BEGIN -->
<b>"JYAvenard" Features:</b><br>
<!-- OPENVPN-BEGIN -->
- OpenVPN enhancements &amp; username/password only authentication<br>
<!-- OPENVPN-END -->
<!-- USERPPTP-BEGIN -->
- PPTP VPN Client integration and GUI<br>
<!-- USERPPTP-END -->
Copyright (C) 2010-2011 Jean-Yves Avenard<br>
<a href='mailto:jean-yves@avenard.org'>jean-yves@avenard.org</a><br>
<br>
<!-- JYAVENARD-END -->
<b>"Teaman" Features:</b><br>
- QOS-detailed & ctrate improved filters<br>
- Per-IP bandwidth monitoring of LAN clients [cstats v2]<br>
- IPTraffic conn/BW ratios graphs<br>
- Static ARP binding<br>
- CPU % usage<br>  
- Udpxy v1.0-Chipmunk-build 21<br>
<!-- VLAN-BEGIN -->
- VLAN support integration and GUI<br>
- VSSID support (experimental)<br>
<!-- VLAN-END -->
<!-- PPTPD-BEGIN -->
- PPTP VPN Server integration and GUI</a><br>
<!-- PPTPD-END -->
- Real-time bandwidth monitoring of LAN clients<br>
- Static ARP binding<br>
Copyright (C) 2011-2012 Augusto Bott<br>
<a href='http://code.google.com/p/tomato-sdhc-vlan/' target='_new'>http://code.google.com/p/tomato-sdhc-vlan/</a><br>
<br>
<b>"KDB" Features:</b><br>
- IPv6 Full Support (Beta)<br>
Copyright (C) 2013 Kevin Darbyshire-Bryant<br>
<br>
<b>"Shibby20" Features:</b><br>
<!-- DNSCRYPT-BEGIN -->
- DNScrypt-proxy 0.9.3 integration and GUI<br>
<!-- DNSCRYPT-END -->
- Ethernet Status Monitor<br>
Copyright (C) 2013 shibby20<br>
<br>
<b>"Toastman" Features:</b><br>
- 250 entry limit in Static DHCP  & Wireless Filter<br>
- 500 entry limit in Access Restriction rules<br>
- Up to 80 QOS rules<br>
- IMQ based QOS/Bandwidth Limiter<br>
- Configurable QOS class names<br>
- Comprehensive QOS rule examples set by default<br>
- TC-ATM overhead calculation - patch by tvlz<br>
- GPT support for HDD by Yaniv Hamo<br>
Copyright (C) 2010-2012 Toastman<br>
<a href='http://www.toastmanfirmware.yolasite.com'>http://www.toastmanfirmware.yolasite.com</a><br>
<br>
Built on <% build_time(); %><br>
<br><br>
<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
<input type="hidden" name="cmd" value="_s-xclick">
<input type="hidden" name="hosted_button_id" value="8099400">
<input type="hidden" name="charset" value="US-ASCII">
<input type="image" src="https://www.paypal.com/en_US/i/btn/btn_donate_LG.gif" border="0" align="center" name="submit" alt="PayPal - The safer, easier way to pay online!">
<img alt="" border="0" src="https://www.paypal.com/es_ES/i/scr/pixel.gif" width="1" height="1">
</form></p></center>
<div style='border-top:1px solid #e7e7e7;margin:4em 0;padding:2em 0;font-size:12px'>
<b>Thanks to everyone who risked their routers, tested, reported bugs, made
suggestions and contributed to this project. ^ _ ^</b><br>
</div>

</div>


</td></tr>
	<tr><td id='footer' colspan=2>&nbsp;</td></tr>
</table>
</body>
</html>
