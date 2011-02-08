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
<title>[<% ident(); %>] Advanced: NAT64</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>
//	<% nvram("ipv6_nat64,nat64_static"); %>

var fog = new TomatoGrid();

fog.dataToView = function(data) {
	return [data[0],data[1]];
}

fog.fieldValuesToData = function(row) {
	var f = fields.getAll(row);
	return [f[0].value, f[1].value];
}

fog.verifyFields = function(row, quiet) {
	// FIXME
	return 1;
}

fog.resetNewEditor = function() {
	var f = fields.getAll(this.newEditor);
	f[0].value = '';
	f[1].value = '';
	ferror.clearAll(fields.getAll(this.newEditor));
}

fog.setup = function() {
	this.init('fo-grid', 'sort', 50, [
		{ type: 'text', maxlen: 15 },
		{ type: 'text', maxlen: 40 }]);
	this.headerSet(['IPv4 Address', 'IPv6 Address']);
	var nv = nvram.nat64_static.split('>');
	for (var i = 0; i < nv.length; i++) {
		r = nv[i].split('<');
		if (r.length == 2)
			fog.insertData(-1, [r[0], r[1]]);
	}
	fog.showNewEditor();
}

function save()
{
	if (fog.isEditing()) return;

	var data = fog.getAllData();
	var s = '';

	for (var i = 0; i < data.length; i++) {
		s += data[i].join('<') + '>';
	}

	var fom = E('_fom');
	fom.nat64_static.value = s;
	form.submit(fom, 0, 'tomato.cgi');
}

function init()
{
	fog.recolor();
	fog.resetNewEditor();
}

</script>

</head>
<body>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='advanced-nat64.asp'>
<input type='hidden' name='_nextwait' value='5'>
<input type='hidden' name='_service' value='tayga-restart'>

<input type='hidden' name='nat64_static'>

<div class='section-title'>NAT64 Configuration</div>
<div class='section'>
	<table class='tomato-grid' cellspacing=1 id='fo-grid'></table>
	<script type='text/javascript'>fog.setup();</script>
</div>

<div>
<ul>
<li><b>IPv4 Address</b> - IPv4 Address for this static mapping
<li><b>IPv6 Address</b> - IPv6 Address for this static mapping
</ul>
</div>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='reloadPage();'>
</td></tr>
</table>
</form>
<div style='height:100px'></div>
</body>
</html>
