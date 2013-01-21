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
<title>[<% ident(); %>] Edycja ograniczeń dostępu</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript' src='protocols.js'></script>

<!-- / / / -->

<style type='text/css'>
#res-comp-grid {
	width: 60%;
}
#res-bp-grid .box1, #res-bp-grid .box2 {
	width: 20%;
	float: left;
}
#res-bp-grid .box3 {
	width: 60%;
	float: left;
}
#res-bp-grid .box4 {
	width: 30%;
	float: left;
	clear: left;
	padding-top: 2px;
}
#res-bp-grid .box5 {
	width: 70%;
	float: left;
	padding-top: 2px;
}
#res-bp-grid .box6 {
	width: 30%;
	float: left;
	padding-top: 2px;
}
#res-bp-grid .box7 {
	width: 70%;
	float: left;
	padding-top: 2px;
}

textarea {
	width: 99%;
	height: 20em;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>
//	<% nvram(''); %>	// http_id

// {enable}|{begin_mins}|{end_mins}|{dow}|{comp[<comp]}|{rules<rules[...]>}|{http[ ...]}|{http_file}|{desc}
//	<% rrule(); %>
if ((rule = rrule.match(/^(\d+)\|(-?\d+)\|(-?\d+)\|(\d+)\|(.*?)\|(.*?)\|([^|]*?)\|(\d+)\|(.*)$/m)) == null) {
	rule = ['', 1, 1380, 240, 31, '', '', '', 0, 'Nowa reguła ' + (rruleN + 1)];
}
rule[2] *= 1;
rule[3] *= 1;
rule[4] *= 1;
rule[8] *= 1;

// <% layer7(); %>
layer7.sort();
for (i = 0; i < layer7.length; ++i)
	layer7[i] = [layer7[i],layer7[i]];
layer7.unshift(['', 'Layer 7 (wyłączone)']);

var ipp2p = [
	[0,'IPP2P (wyłączone)'],[0xFFFF,'Wszystkie filtry IPP2P'],[1,'AppleJuice'],[2,'Ares'],[4,'BitTorrent'],[8,'Direct Connect'],
	[16,'eDonkey'],[32,'Gnutella'],[64,'Kazaa'],[128,'Mute'],
/* LINUX26-BEGIN */
	[4096,'PPLive/UUSee'],
/* LINUX26-END */
	[256,'SoulSeek'],[512,'Waste'],[1024,'WinMX'],[2048,'XDCC']
/* LINUX26-BEGIN */
	,[8192,'Xunlei/QQCyclone']
/* LINUX26-END */
	];

var dowNames = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

//

var cg = new TomatoGrid();

cg.verifyFields = function(row, quiet) {
	var f = fields.getAll(row)[0];
	if (v_mac(f, true)) return true;
	if (_v_iptaddr(f, true, false, true, true)) return true;

	ferror.set(f, 'Niewłaściwy adres MAC lub zakres adresów IP', quiet);
	return false;
}

cg.setup = function() {
	var a, i, count, ex;

	this.init('res-comp-grid', 'sort', 140, [ { type: 'text', maxlen: 32 } ] );
	this.headerSet(['MAC / Adres IP']);
	this.showNewEditor();
	this.resetNewEditor();

	if (rule[5] == '~') return;	// wireless disable rule

	ex = 0;
	count = 0;
	a = rule[5].split('>');
	for (i = 0; i < a.length; ++i) {
		if (!a[i].length) continue;
		if (a[i] == '!') {
			ex = 1;
		}
		else {
			cg.insertData(-1, [a[i]]);
			++count;
		}
	}

	a = E('_f_comp_all')
	if (count) {
		a.value = ex ? 2 : 1;
	}
	else {
		a.value = 0;
	}
}


var bpg = new TomatoGrid();

bpg.verifyFields = function(row, quiet) {
	var f = fields.getAll(row);
	ferror.clearAll(f);
	this.enDiFields(row);

	if ((f[5].selectedIndex != 0) && ((!v_length(f[6], quiet, 1)) || (!_v_iptaddr(f[6], quiet, false, true, true)))) return 0;
	if ((f[1].selectedIndex != 0) && (!v_iptport(f[2], quiet))) return 0;

	if ((f[1].selectedIndex == 0) && (f[3].selectedIndex == 0) && (f[4].selectedIndex == 0) && (f[5].selectedIndex == 0)) {
		var m = 'Proszę wprowadzić numer portu lub wybrać odpowiednią aplikację';
		ferror.set(f[3], m, 1);
		ferror.set(f[4], m, 1);
		ferror.set(f[5], m, 1);
		ferror.set(f[1], m, quiet);
		return 0;
	}

	ferror.clear(f[1]);
	ferror.clear(f[3]);
	ferror.clear(f[4]);
	ferror.clear(f[5]);
	ferror.clear(f[6]);
	return 1;
}

bpg.dataToView = function(data) {
	var s, i;

	s = '';
	if (data[5] != 0) s = ((data[5] == 1) ? 'Do ' : 'Z ') + data[6] + ', ';

	if (data[0] <= -2) s += (s.length ? 'a' : 'A') + 'ny protocol';
	else if (data[0] == -1) s += 'TCP/UDP';
	else if (data[0] >= 0) s += protocols[data[0]] || data[0];

	if (data[0] >= -1) {
		if (data[1] == 'd') s += ', dst port ';
		else if (data[1] == 's') s += ', src port ';
		else if (data[1] == 'x') s += ', port ';
		else s += ', all ports';
		if (data[1] != 'a') s += data[2].replace(/:/g, '-');
	}

	if (data[3] != 0) {
		for (i = 0; i < ipp2p.length; ++i) {
			if (data[3] == ipp2p[i][0]) {
				s += ', IPP2P: ' + ipp2p[i][1];
				break;
			}
		}
	}
	else if (data[4] != '') {
		s += ', L7: ' + data[4];
	}

	return [s];
}

bpg.fieldValuesToData = function(row) {
	var f = fields.getAll(row);
	return [f[0].value, f[1].value, (f[1].selectedIndex == 0) ? '' : f[2].value, f[3].value, f[4].value, f[5].value, (f[5].selectedIndex == 0) ? '' : f[6].value];
},

bpg.resetNewEditor = function() {
	var f = fields.getAll(this.newEditor);
	f[0].selectedIndex = 0;
	f[1].selectedIndex = 0;
	f[2].value = '';
	f[3].selectedIndex = 0;
	f[4].selectedIndex = 0;
	f[5].selectedIndex = 0;
	f[6].value = '';
	this.enDiFields(this.newEditor);
	ferror.clearAll(fields.getAll(this.newEditor));
}

bpg._createEditor = bpg.createEditor;
bpg.createEditor = function(which, rowIndex, source) {
	var row = this._createEditor(which, rowIndex, source);
	if (which == 'edit') this.enDiFields(row);
	return row;
}

bpg.enDiFields = function(row) {
	var x;
	var f = fields.getAll(row);

	x = f[0].value;
	x = ((x != -1) && (x != 6) && (x != 17));
	f[1].disabled = x;
	if (f[1].selectedIndex == 0) x = 1;
	f[2].disabled = x;
	f[3].disabled = (f[4].selectedIndex != 0);
	f[4].disabled = (f[3].selectedIndex != 0);
	f[6].disabled = (f[5].selectedIndex == 0);
}

bpg.setup = function() {
	var a, i, r, count, protos;

	protos = [[-2, 'Dowolny protokół'],[-1,'TCP/UDP'],[6,'TCP'],[17,'UDP']];
	for (i = 0; i < 256; ++i) {
		if ((i != 6) && (i != 17)) protos.push([i, protocols[i] || i]);
	}

	this.init('res-bp-grid', 'sort', 140, [ { multi: [
		{ type: 'select', prefix: '<div class="box1">', suffix: '</div>', options: protos },
		{ type: 'select', prefix: '<div class="box2">', suffix: '</div>',
			options: [['a','Dowolny port'],['d','Port docelowy'],['s','Port źródłowy'],['x','Źródłowy lub Docelowy']] },
		{ type: 'text', prefix: '<div class="box3">', suffix: '</div>', maxlen: 32 },
		{ type: 'select', prefix: '<div class="box4">', suffix: '</div>', options: ipp2p },
		{ type: 'select', prefix: '<div class="box5">', suffix: '</div>', options: layer7 },
		{ type: 'select', prefix: '<div class="box6">', suffix: '</div>',
			options: [[0,'Dowolny adres'],[1,'Docel. IP'],[2,'Źródł. IP']] },
		{ type: 'text', prefix: '<div class="box7">', suffix: '</div>', maxlen: 64 }
		] } ] );
	this.headerSet(['Reguły']);
	this.showNewEditor();
	this.resetNewEditor();
	count = 0;

	// ---- proto<dir<port<ipp2p<layer7[<addr_type<addr]

	a = rule[6].split('>');
	for (i = 0; i < a.length; ++i) {
		r = a[i].split('<');
		if (r.length == 5) {
			// ---- fixup for backward compatibility
			r.push('0');
			r.push('');
		}
		if (r.length == 7) {
			r[2] = r[2].replace(/:/g, '-');
			this.insertData(-1, r);
			++count;
		}
	}
	return count;
}

//

function verifyFields(focused, quiet)
{
	var b, e;

	tgHideIcons();

	elem.display(PR('_f_sched_begin'), !E('_f_sched_allday').checked);
	elem.display(PR('_f_sched_sun'), !E('_f_sched_everyday').checked);

	b = E('rt_norm').checked;
	elem.display(PR('_f_comp_all'), PR('_f_block_all'), b);

	elem.display(PR('res-comp-grid'), b && E('_f_comp_all').value != 0);
	elem.display(PR('res-bp-grid'), PR('_f_block_http'), PR('_f_activex'), b && !E('_f_block_all').checked);

	ferror.clear('_f_comp_all');

	e = E('_f_block_http');
	e.value = e.value.replace(/[|"']/g, ' ');
	if (!v_length(e, quiet, 0, 2048 - 16)) return 0;

	e = E('_f_desc');
	e.value = e.value.replace(/\|/g, '_');
	if (!v_length(e, quiet, 1)) return 0;

	return 1;
}

function cancel()
{
	document.location = 'restrict.asp';
}

function remove()
{
	if (!confirm('Usunąć tę regułę?')) return;

	E('delete-button').disabled = 1;

	e = E('_rrule');
	e.name = 'rrule' + rruleN;
	e.value = '';
	form.submit('_fom');
}

function save()
{
	if (!verifyFields(null, false)) return;
	if ((cg.isEditing()) || (bpg.isEditing())) return;

	var a, b, e, s, n, data;

	data = [];
	data.push(E('_f_enabled').checked ? '1' : '0');
	if (E('_f_sched_allday').checked) data.push(-1, -1);
		else data.push(E('_f_sched_begin').value, E('_f_sched_end').value);

	if (E('_f_sched_everyday').checked) {
		n = 0x7F;
	}
	else {
		n = 0;
		for (i = 0; i < 7; ++i) {
			if (E('_f_sched_' + dowNames[i].toLowerCase()).checked) n |= (1 << i);
		}
		if (n == 0) n = 0x7F;
	}
	data.push(n);

	if (E('rt_norm').checked) {
		e = E('_f_comp_all');
		if (e.value != 0) {
			a = cg.getAllData();
			if (a.length == 0) {
				ferror.set(e, 'Nie podano adresu MAC ani IP', 0);
				return;
			}
			if (e.value == 2) a.unshift('!');
			data.push(a.join('>'));
		}
		else {
			data.push('');
		}

		if (E('_f_block_all').checked) {
			data.push('', '', '0');
		}
		else {
			var check = 0;
			a = bpg.getAllData();
			check += a.length;
			b = [];
			for (i = 0; i < a.length; ++i) {
				a[i][2] = a[i][2].replace(/-/g, ':');
				b.push(a[i].join('<'));
			}
			data.push(b.join('>'));

			a = E('_f_block_http').value.replace(/\r+/g, ' ').replace(/\n+/g, '\n').replace(/ +/g, ' ').replace(/^\s+|\s+$/g, '');
			check += a.length;
			data.push(a);

			n = 0;
			if (E('_f_activex').checked) n = 1;
			if (E('_f_flash').checked) n |= 2;
			if (E('_f_java').checked) n |= 4;
			data.push(n);
			
			if (((check + n) == 0) && (data[0] == 1)) {
				alert('Proszę określić, co ma być blokowane.');
				return;
			}
		}
	}
	else {
		data.push('~');
		data.push('', '', '', '0');
	}

	data.push(E('_f_desc').value);
	data = data.join('|');

	if (data.length >= 2048) {
		alert('Reguła zbyt długa. Proszę ją ograniczyć o ' + (data.length - 2048) + ' znaków.');
		return;
	}

	e = E('_rrule');
	e.name = 'rrule' + rruleN;
	e.value = data;

	E('delete-button').disabled = 1;
	form.submit('_fom');
}

function init()
{
	cg.recolor();
	bpg.recolor();
}

function earlyInit()
{
	var count;

	cg.setup();

	count = bpg.setup();
	E('_f_block_all').checked = (count == 0) && (rule[7].search(/[^\s\r\n]/) == -1) && (rule[8] == 0);
	verifyFields(null, 1);
}
</script>
</head>
<body onload='init()'>
<form name='_fom' id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Wersja <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='restrict.asp'>
<input type='hidden' name='_service' value='restrict-restart'>
<input type='hidden' name='rruleNN' id='_rrule' value=''>

<div class='section-title'>Ograniczenia dostępu</div>
<div class='section'>
<script type='text/javascript'>
W('<div style="float:right"><small>'+ 'ID: ' + rruleN.pad(2) + '</small>&nbsp;</div><br>');
tm = [];
for (i = 0; i < 1440; i += 15) tm.push([i, timeString(i)]);

createFieldTable('', [
	{ title: 'Włączone', name: 'f_enabled', type: 'checkbox', value: rule[1] == '1' },
	{ title: 'Opis', name: 'f_desc', type: 'text', maxlen: 32, size: 35, value: rule[9] },
	{ title: 'Harmonogram', multi: [
		{ name: 'f_sched_allday', type: 'checkbox', suffix: ' Cała doba &nbsp; ', value: (rule[2] < 0) || (rule[3] < 0) },
		{ name: 'f_sched_everyday', type: 'checkbox', suffix: ' Codziennie', value: (rule[4] & 0x7F) == 0x7F } ] },
	{ title: 'Czas', indent: 2, multi: [
		{ name: 'f_sched_begin', type: 'select', options: tm, value: (rule[2] < 0) ? 0 : rule[2], suffix: ' - ' },
		{ name: 'f_sched_end', type: 'select', options: tm, value: (rule[3] < 0) ? 0 : rule[3] } ] },
	{ title: 'Dni', indent: 2, multi: [
		{ name: 'f_sched_sun', type: 'checkbox', suffix: ' Nie &nbsp; ', value: (rule[4] & 1) },
		{ name: 'f_sched_mon', type: 'checkbox', suffix: ' Pon &nbsp; ', value: (rule[4] & (1 << 1)) },
		{ name: 'f_sched_tue', type: 'checkbox', suffix: ' Wto &nbsp; ', value: (rule[4] & (1 << 2)) },
		{ name: 'f_sched_wed', type: 'checkbox', suffix: ' Śro &nbsp; ', value: (rule[4] & (1 << 3)) },
		{ name: 'f_sched_thu', type: 'checkbox', suffix: ' Czw &nbsp; ', value: (rule[4] & (1 << 4)) },
		{ name: 'f_sched_fri', type: 'checkbox', suffix: ' Pią &nbsp; ', value: (rule[4] & (1 << 5)) },
		{ name: 'f_sched_sat', type: 'checkbox', suffix: ' Sob', value: (rule[4] & (1 << 6)) } ] },
	{ title: 'Typ', name: 'f_type', id: 'rt_norm', type: 'radio', suffix: ' Standardowe ograniczenie dostępu', value: (rule[5] != '~') },
	{ title: '', name: 'f_type', id: 'rt_wl', type: 'radio', suffix: ' Wyłącz Wi-Fi', value: (rule[5] == '~') },
	{ title: 'Zastosuj do', name: 'f_comp_all', type: 'select', options: [[0,'Wszystkich urządzeń'],[1,'Następujących...'],[2,'Wszystkich, oprócz...']], value: 0 },
	{ title: '&nbsp;', text: '<table class="tomato-grid" cellspacing=1 id="res-comp-grid"></table>' },
	{ title: 'Zablokowane zasoby', name: 'f_block_all', type: 'checkbox', suffix: ' Blokuj cały dostęp do Internetu', value: 0 },
	{ title: 'Port /<br>Aplikacja', indent: 2, text: '<table class="tomato-grid" cellspacing=1 id="res-bp-grid"></table>' },
	{ title: 'Zapytania HTTP', indent: 2, name: 'f_block_http', type: 'textarea', value: rule[7] },
	{ title: 'Zapytania o pliki typu', indent: 2, multi: [
		{ name: 'f_activex', type: 'checkbox', suffix: ' ActiveX (ocx, cab) &nbsp;&nbsp;', value: (rule[8] & 1) },
		{ name: 'f_flash', type: 'checkbox', suffix: ' Flash (swf) &nbsp;&nbsp;', value: (rule[8] & 2) },
		{ name: 'f_java', type: 'checkbox', suffix: ' Java (class, jar) &nbsp;&nbsp;', value: (rule[8] & 4) } ] }
]);
</script>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Usuń...' id='delete-button' onclick='remove()'>
	&nbsp;
	<input type='button' value='Zapisz' id='save-button' onclick='save()'>
	<input type='button' value='Anuluj' id='cancel-button' onclick='cancel()'>
</td></tr>
</table>
<br><br>
</form>
<script type='text/javascript'>earlyInit();</script>
</body>
</html>
